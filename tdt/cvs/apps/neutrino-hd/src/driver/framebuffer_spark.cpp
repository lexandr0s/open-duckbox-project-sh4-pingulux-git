/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
                      2003 thegoodguy
	Copyright (C) 2012 Stefan Seyfried

	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <driver/framebuffer.h>

#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <memory.h>
#include <math.h>

#include <linux/kd.h>

#include <stdint.h>

#include <gui/color.h>
#include <gui/pictureviewer.h>
#include <global.h>
#include <video.h>
#include <cs_api.h>

#include <linux/stmfb.h>
#include <bpamem.h>

#include <pthread.h>

#ifdef EVOLUX
#include <png.h>
#endif
#define DEFAULT_XRES 1280
#define DEFAULT_YRES 720

extern cVideo * videoDecoder;

extern CPictureViewer * g_PicViewer;
#define ICON_CACHE_SIZE 1024*1024*2 // 2mb

#define BACKGROUNDIMAGEWIDTH 720

static int bpafd = -1;
static fb_pixel_t *backbuffer = NULL;		/* buffer in BPA mem, mostly for tuxtxt or icons */
static fb_pixel_t *lbb;				/* "shadow buffer", will be scale-blitted to the FB */
static size_t lbb_sz = 1920 * 1080;			/* offset from fb start in 'pixels' */
static size_t lbb_off = lbb_sz * sizeof(fb_pixel_t);	/* offset from fb start in bytes    */
static int backbuf_sz = 0;

/* #define PARTIAL_BLIT to use the theoretically faster blitter code
 * which updates only the region on screen which is really drawn upon
 * in practice, it seems to cause artifacts on some machines...
 */
#undef PARTIAL_BLIT
#ifdef PARTIAL_BLIT
static unsigned int last_xres = 0;

struct dirty_region {
	int xs;
	int ys;
	int xe;
	int ye;
};

static struct dirty_region to_blit = { INT_MAX, INT_MAX, 0, 0 };
#endif
static pthread_mutex_t blit_mutex;

static inline void blit_lock()
{
	pthread_mutex_lock(&blit_mutex);
}

static inline void blit_unlock()
{
	pthread_mutex_unlock(&blit_mutex);
}

#ifdef PARTIAL_BLIT
static inline void update_dirty(int xs, int ys, int xe, int ye)
{
	if (xs < to_blit.xs)
		to_blit.xs = xs;
	if (xe > to_blit.xe)
		to_blit.xe = xe;
	if (ys < to_blit.ys)
		to_blit.ys = ys;
	if (ye > to_blit.ye)
		to_blit.ye = ye;
}
#else
#define update_dirty(a, b, c, d)
#endif

void CFrameBuffer::waitForIdle(void)
{
#if 0
	struct timeval ts, te;
	gettimeofday(&ts, NULL);
#endif
	blit_lock();
	ioctl(fd, STMFBIO_SYNC_BLITTER);
	blit_unlock();
#if 0
	gettimeofday(&te, NULL);
	printf("STMFBIO_SYNC_BLITTER took %lld us\n", (te.tv_sec * 1000000LL + te.tv_usec) - (ts.tv_sec * 1000000LL + ts.tv_usec));
#endif
}

/*******************************************************************************/

static uint8_t * virtual_fb = NULL;
inline unsigned int make16color(uint16_t r, uint16_t g, uint16_t b, uint16_t t,
                                  uint32_t  /*rl*/ = 0, uint32_t  /*ro*/ = 0,
                                  uint32_t  /*gl*/ = 0, uint32_t  /*go*/ = 0,
                                  uint32_t  /*bl*/ = 0, uint32_t  /*bo*/ = 0,
                                  uint32_t  /*tl*/ = 0, uint32_t  /*to*/ = 0)
{
        return ((t << 24) & 0xFF000000) | ((r << 8) & 0xFF0000) | ((g << 0) & 0xFF00) | (b >> 8 & 0xFF);
}

CFrameBuffer::CFrameBuffer()
: active ( true )
{
	iconBasePath = "";
	available  = 0;
	cmap.start = 0;
	cmap.len = 256;
	cmap.red = red;
	cmap.green = green;
	cmap.blue  = blue;
	cmap.transp = trans;
	backgroundColor = 0;
	useBackgroundPaint = false;
	background = NULL;
	backupBackground = NULL;
	backgroundFilename = "";
	fd  = 0;
	tty = 0;
	bpp = 0;
	locked = false;
	m_transparent_default = CFrameBuffer::TM_BLACK; // TM_BLACK: Transparency when black content ('pseudo' transparency)
							// TM_NONE:  No 'pseudo' transparency
							// TM_INI:   Transparency depends on g_settings.infobar_alpha ???
	m_transparent         = m_transparent_default;
//FIXME: test
	memset(red, 0, 256*sizeof(__u16));
	memset(green, 0, 256*sizeof(__u16));
	memset(blue, 0, 256*sizeof(__u16));
	memset(trans, 0, 256*sizeof(__u16));
	pthread_mutex_init(&blit_mutex, NULL);
}

CFrameBuffer* CFrameBuffer::getInstance()
{
	static CFrameBuffer* frameBuffer = NULL;

	if(!frameBuffer) {
		frameBuffer = new CFrameBuffer();
		printf("[neutrino] frameBuffer Instance created\n");
	} else {
		//printf("[neutrino] frameBuffer Instace requested\n");
	}
	return frameBuffer;
}

void CFrameBuffer::init(const char * const fbDevice)
{
        int tr = 0xFF;

	fd = open(fbDevice, O_RDWR);
	if(!fd) fd = open(fbDevice, O_RDWR);

	if (fd<0) {
		perror(fbDevice);
		goto nolfb;
	}

	if (ioctl(fd, FBIOGET_VSCREENINFO, &screeninfo)<0) {
		perror("FBIOGET_VSCREENINFO");
		goto nolfb;
	}

	xRes = screeninfo.xres;
	yRes = screeninfo.yres;
	bpp = 32;
	stride = xRes * bpp / 8;
printf("FB: %dx%dx%d line length %d.\n", xRes, yRes, bpp, stride);

	xRes = DEFAULT_XRES;
	yRes = DEFAULT_YRES;
	screeninfo.xres = DEFAULT_XRES;
	screeninfo.yres = DEFAULT_YRES;
	screeninfo.xres_virtual = DEFAULT_XRES;
	screeninfo.yres_virtual = DEFAULT_YRES;

	screeninfo.bits_per_pixel = 32;
	stride = xRes * bpp / 8;
	backbuf_sz = stride * yRes;

	memmove(&oldscreen, &screeninfo, sizeof(screeninfo));

	if (ioctl(fd, FBIOGET_FSCREENINFO, &fix)<0) {
		perror("FBIOGET_FSCREENINFO");
		goto nolfb;
	}

	available = fix.smem_len;
	printf("%dk video mem\n", available/1024);
	if (available < 12*1024*1024)
	{
		/* for old installations that did not upgrade their module config
		 * it will still work good enough to display the message below */
		fprintf(stderr, "[neutrino] WARNING: not enough framebuffer memory available!\n");
		fprintf(stderr, "[neutrino]          I need at least 12MB.\n");
		FILE *f = fopen("/tmp/infobar.txt", "w");
		if (f) {
			fprintf(f, "NOT ENOUGH FRAMEBUFFER MEMORY!");
			fclose(f);
		}
		lbb_sz = 0;
		lbb_off = 0;
	}

	lfb = (fb_pixel_t*)mmap(0, available, PROT_WRITE|PROT_READ, MAP_SHARED, fd, 0);

	if (! lfb) {
		perror("mmap");
		goto nolfb;
	}
	memset(lfb, 0, available);
	lbb = lfb + lbb_sz;
	bpafd = open("/dev/bpamem0", O_RDWR);
	if (bpafd < 0)
	{
		fprintf(stderr, "[neutrino] FB: cannot open /dev/bpamem0: %m\n");
		return;
	}
	BPAMemAllocMemData bpa_data;
	bpa_data.bpa_part = (char *)"LMI_VID";
	bpa_data.mem_size = backbuf_sz;
	int res;
	res = ioctl(bpafd, BPAMEMIO_ALLOCMEM, &bpa_data);
	if (res)
	{
		fprintf(stderr, "[neutrino] FB: cannot allocate from bpamem: %m\n");
		close(bpafd);
		bpafd = -1;
		return;
	}
	close(bpafd);

	char bpa_mem_device[30];
	sprintf(bpa_mem_device, "/dev/bpamem%d", bpa_data.device_num);
	bpafd = open(bpa_mem_device, O_RDWR);
	if (bpafd < 0)
	{
		fprintf(stderr, "[neutrino] FB: cannot open secondary %s: %m\n", bpa_mem_device);
		return;
	}

	backbuffer = (fb_pixel_t *)mmap(0, bpa_data.mem_size, PROT_WRITE|PROT_READ, MAP_SHARED, bpafd, 0);
	if (backbuffer == MAP_FAILED)
	{
		fprintf(stderr, "[neutrino] FB: cannot map from bpamem: %m\n");
		ioctl(bpafd, BPAMEMIO_FREEMEM);
		close(bpafd);
		bpafd = -1;
		return;
	}

	memset(backbuffer, 0, backbuf_sz);
	cache_size = 0;

	/* Windows Colors */
	paletteSetColor(0x1, 0x010101, tr);
	paletteSetColor(0x2, 0x800000, tr);
	paletteSetColor(0x3, 0x008000, tr);
	paletteSetColor(0x4, 0x808000, tr);
	paletteSetColor(0x5, 0x000080, tr);
	paletteSetColor(0x6, 0x800080, tr);
	paletteSetColor(0x7, 0x008080, tr);
	paletteSetColor(0x8, 0xA0A0A0, tr);
	paletteSetColor(0x9, 0x505050, tr);
	paletteSetColor(0xA, 0xFF0000, tr);
	paletteSetColor(0xB, 0x00FF00, tr);
	paletteSetColor(0xC, 0xFFFF00, tr);
	paletteSetColor(0xD, 0x0000FF, tr);
	paletteSetColor(0xE, 0xFF00FF, tr);
	paletteSetColor(0xF, 0x00FFFF, tr);
	paletteSetColor(0x10, 0xFFFFFF, tr);
	paletteSetColor(0x11, 0x000000, tr);
	paletteSetColor(COL_BACKGROUND, 0x000000, 0xffff);

	paletteSet();

	useBackground(false);

	return;

nolfb:
	printf("framebuffer not available.\n");
	lfb = NULL;
}


CFrameBuffer::~CFrameBuffer()
{
	std::map<std::string, rawIcon>::iterator it;

	for(it = icon_cache.begin(); it != icon_cache.end(); it++) {
		/* printf("FB: delete cached icon %s: %x\n", it->first.c_str(), (int) it->second.data); */
		cs_free_uncached(it->second.data);
	}
	icon_cache.clear();

	if (background) {
		delete[] background;
		background = NULL;
	}

	if (backupBackground) {
		delete[] backupBackground;
		backupBackground = NULL;
	}

	useBackground(false);
	m_transparent = m_transparent_default;
#if 0
#ifdef RETURN_FROM_GRAPHICS_MODE
	if (-1 == ioctl(tty,KDSETMODE, kd_mode))
		perror("ioctl KDSETMODE");
#endif

	if (-1 == ioctl(tty,VT_SETMODE, &vt_mode))
		perror("ioctl VT_SETMODE");

	if (available)
		ioctl(fd, FBIOPUT_VSCREENINFO, &oldscreen);
#endif
	if (lfb)
		munmap(lfb, available);

	if (virtual_fb){
		delete[] virtual_fb;
		virtual_fb = NULL;
	}

	if (backbuffer)
	{
		fprintf(stderr, "CFrameBuffer: unmap backbuffer\n");
		munmap(backbuffer, backbuf_sz);
	}
	if (bpafd != -1)
	{
		fprintf(stderr, "CFrameBuffer: BPAMEMIO_FREEMEM\n");
		ioctl(bpafd, BPAMEMIO_FREEMEM);
		close(bpafd);
	}
	close(fd);
	close(tty);
}

int CFrameBuffer::getFileHandle() const
{
	return fd;
}

unsigned int CFrameBuffer::getStride() const
{
	return stride;
}

unsigned int CFrameBuffer::getScreenWidth(bool real)
{
	if(real)
		return xRes;
	else
		return g_settings.screen_EndX - g_settings.screen_StartX;
}

unsigned int CFrameBuffer::getScreenHeight(bool real)
{
	if(real)
		return yRes;
	else
		return g_settings.screen_EndY - g_settings.screen_StartY;
}

unsigned int CFrameBuffer::getScreenX()
{
	return g_settings.screen_StartX;
}

unsigned int CFrameBuffer::getScreenY()
{
	return g_settings.screen_StartY;
}

fb_pixel_t * CFrameBuffer::getFrameBufferPointer() const
{
	if (active || (virtual_fb == NULL))
		return lbb;
	else
		return (fb_pixel_t *) virtual_fb;
}

fb_pixel_t * CFrameBuffer::getBackBufferPointer() const
{
	return backbuffer;
}

bool CFrameBuffer::getActive() const
{
	return (active || (virtual_fb != NULL));
}

void CFrameBuffer::setActive(bool enable)
{
	active = enable;
}

t_fb_var_screeninfo *CFrameBuffer::getScreenInfo()
{
	return &screeninfo;
}

int CFrameBuffer::setMode(unsigned int /*nxRes*/, unsigned int /*nyRes*/, unsigned int /*nbpp*/)
{
	if (!available&&!active)
		return -1;

	return 0;
}

void CFrameBuffer::setTransparency( int /*tr*/ )
{
}

void CFrameBuffer::setBlendMode(uint8_t /*mode*/)
{
#if 0
	//g.use_global_alpha = (mode == 2); /* 1 == pixel alpha, 2 == global alpha */
	if (ioctl(fd, FBIO_SETBLENDMODE, mode))
		printf("FBIO_SETBLENDMODE failed.\n");
#endif
}

void CFrameBuffer::setBlendLevel(int /*level*/)
{
#if 0
	//printf("CFrameBuffer::setBlendLevel %d\n", level);
	unsigned char value = 0xFF;
	if((level >= 0) && (level <= 100))
		value = convertSetupAlpha2Alpha(level);

	if (ioctl(fd, FBIO_SETOPACITY, value))
		printf("FBIO_SETOPACITY failed.\n");
#if 1
       if(level == 100) // TODO: sucks.
               usleep(20000);
#endif
#endif
}

void CFrameBuffer::setAlphaFade(int in, int num, int tr)
{
	for (int i=0; i<num; i++) {
		cmap.transp[in+i]=tr;
	}
}

void CFrameBuffer::paletteFade(int i, __u32 rgb1, __u32 rgb2, int level)
{
	__u16 *r = cmap.red+i;
	__u16 *g = cmap.green+i;
	__u16 *b = cmap.blue+i;
	*r= ((rgb2&0xFF0000)>>16)*level;
	*g= ((rgb2&0x00FF00)>>8 )*level;
	*b= ((rgb2&0x0000FF)    )*level;
	*r+=((rgb1&0xFF0000)>>16)*(255-level);
	*g+=((rgb1&0x00FF00)>>8 )*(255-level);
	*b+=((rgb1&0x0000FF)    )*(255-level);
}

void CFrameBuffer::paletteGenFade(int in, __u32 rgb1, __u32 rgb2, int num, int tr)
{
	for (int i=0; i<num; i++) {
		paletteFade(in+i, rgb1, rgb2, i*(255/(num-1)));
		cmap.transp[in+i]=tr;
		tr--; //FIXME
	}
}

void CFrameBuffer::paletteSetColor(int i, __u32 rgb, int tr)
{
	cmap.red[i]    =(rgb&0xFF0000)>>8;
	cmap.green[i]  =(rgb&0x00FF00)   ;
	cmap.blue[i]   =(rgb&0x0000FF)<<8;
	cmap.transp[i] = tr;
}

void CFrameBuffer::paletteSet(struct fb_cmap *map)
{
	if (!active)
		return;

	if(map == NULL)
		map = &cmap;

	if(bpp == 8) {
//printf("Set palette for %dbit\n", bpp);
		ioctl(fd, FBIOPUTCMAP, map);
	}
	uint32_t  rl, ro, gl, go, bl, bo, tl, to;
	rl = screeninfo.red.length;
	ro = screeninfo.red.offset;
	gl = screeninfo.green.length;
	go = screeninfo.green.offset;
	bl = screeninfo.blue.length;
	bo = screeninfo.blue.offset;
	tl = screeninfo.transp.length;
	to = screeninfo.transp.offset;
	for (int i = 0; i < 256; i++) {
		realcolor[i] = make16color(cmap.red[i], cmap.green[i], cmap.blue[i], cmap.transp[i],
					   rl, ro, gl, go, bl, bo, tl, to);
	}
}

void CFrameBuffer::paintBoxRel(const int x, const int y, const int dx, const int dy, const fb_pixel_t col, int radius, int type)
{
	/* draw a filled rectangle (with additional round corners) */
	if (!getActive())
		return;

	int corner_tl = (type & CORNER_TOP_LEFT)     ? 1 : 0;
	int corner_tr = (type & CORNER_TOP_RIGHT)    ? 1 : 0;
	int corner_bl = (type & CORNER_BOTTOM_LEFT)  ? 1 : 0;
	int corner_br = (type & CORNER_BOTTOM_RIGHT) ? 1 : 0;

	/* this table contains the x coordinates for a quarter circle (the bottom right quarter) with fixed
	   radius of 540 px which is the half of the max HD graphics size of 1080 px. So with that table we
	   ca draw boxes with round corners and als circles by just setting dx = dy = radius (max 540). */
	static const int q_circle[541] = {
	540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540,
	540, 540, 540, 540, 539, 539, 539, 539, 539, 539, 539, 539, 539, 539, 539, 539, 539, 539, 539, 539,
	539, 538, 538, 538, 538, 538, 538, 538, 538, 538, 538, 538, 538, 537, 537, 537, 537, 537, 537, 537,
	537, 537, 536, 536, 536, 536, 536, 536, 536, 536, 535, 535, 535, 535, 535, 535, 535, 535, 534, 534,
	534, 534, 534, 534, 533, 533, 533, 533, 533, 533, 532, 532, 532, 532, 532, 532, 531, 531, 531, 531,
	531, 531, 530, 530, 530, 530, 529, 529, 529, 529, 529, 529, 528, 528, 528, 528, 527, 527, 527, 527,
	527, 526, 526, 526, 526, 525, 525, 525, 525, 524, 524, 524, 524, 523, 523, 523, 523, 522, 522, 522,
	522, 521, 521, 521, 521, 520, 520, 520, 519, 519, 519, 518, 518, 518, 518, 517, 517, 517, 516, 516,
	516, 515, 515, 515, 515, 514, 514, 514, 513, 513, 513, 512, 512, 512, 511, 511, 511, 510, 510, 510,
	509, 509, 508, 508, 508, 507, 507, 507, 506, 506, 506, 505, 505, 504, 504, 504, 503, 503, 502, 502,
	502, 501, 501, 500, 500, 499, 499, 499, 498, 498, 498, 497, 497, 496, 496, 496, 495, 495, 494, 494,
	493, 493, 492, 492, 491, 491, 490, 490, 490, 489, 489, 488, 488, 487, 487, 486, 486, 485, 485, 484,
	484, 483, 483, 482, 482, 481, 481, 480, 480, 479, 479, 478, 478, 477, 477, 476, 476, 475, 475, 474,
	473, 473, 472, 472, 471, 471, 470, 470, 469, 468, 468, 467, 466, 466, 465, 465, 464, 464, 463, 462,
	462, 461, 460, 460, 459, 459, 458, 458, 457, 456, 455, 455, 454, 454, 453, 452, 452, 451, 450, 450,
	449, 449, 448, 447, 446, 446, 445, 445, 444, 443, 442, 441, 441, 440, 440, 439, 438, 437, 436, 436,
	435, 435, 434, 433, 432, 431, 431, 430, 429, 428, 427, 427, 426, 425, 425, 424, 423, 422, 421, 421,
	420, 419, 418, 417, 416, 416, 415, 414, 413, 412, 412, 411, 410, 409, 408, 407, 406, 405, 404, 403,
	403, 402, 401, 400, 399, 398, 397, 397, 395, 394, 393, 393, 392, 391, 390, 389, 388, 387, 386, 385,
	384, 383, 382, 381, 380, 379, 378, 377, 376, 375, 374, 373, 372, 371, 369, 368, 367, 367, 365, 364,
	363, 362, 361, 360, 358, 357, 356, 355, 354, 353, 352, 351, 350, 348, 347, 346, 345, 343, 342, 341,
	340, 339, 337, 336, 335, 334, 332, 331, 329, 328, 327, 326, 324, 323, 322, 321, 319, 317, 316, 315,
	314, 312, 310, 309, 308, 307, 305, 303, 302, 301, 299, 297, 296, 294, 293, 291, 289, 288, 287, 285,
	283, 281, 280, 278, 277, 275, 273, 271, 270, 268, 267, 265, 263, 261, 259, 258, 256, 254, 252, 250,
	248, 246, 244, 242, 240, 238, 236, 234, 232, 230, 228, 225, 223, 221, 219, 217, 215, 212, 210, 207,
	204, 202, 200, 197, 195, 192, 190, 187, 184, 181, 179, 176, 173, 170, 167, 164, 160, 157, 154, 150,
	147, 144, 140, 136, 132, 128, 124, 120, 115, 111, 105, 101,  95,  89,  83,  77,  69,  61,  52,  40,
	 23};

	int line = 0;

	if (! (type && radius))
	{
		blitRect(x, y + line, dx, dy - line, col);
		return;
	}
	#define MUL 32768	/* just an multiplicator for all math to reduce rounding errors */
	int ofs, scf, scl, ofl, ofr;

	/* limit the radius */
	if (radius > dx)
		radius = dx;
	if (radius > dy)
		radius = dy;
	if (radius > 540)
		radius = 540;
	if (radius < 1)		/* dx or dy = 0... */
		radius = 1;	/* avoid div by zero below */

	scf = (540 * MUL) / radius;

	while (line < dy)
	{
		ofl = ofr = 0;

		if (line < radius && (type & CORNER_TOP)) /* one of the top corners */
		{
			/* uper round corners */
			scl = scf * (radius - line) / MUL;
			if ((scf * (radius - line) % MUL) >= (MUL / 2))	/* round up */
				scl++;
			ofs = radius - (q_circle[scl] * MUL / scf);
			// ofl = corner_tl * ofs; // might depend on the arch if multiply is faster or not
			ofl = corner_tl ? ofs : 0;
			ofr = corner_tr ? ofs : 0;
		}
		else if ((line >= dy - radius) && (type & CORNER_BOTTOM)) /* one of the bottom corners */
		{
			/* lower round corners */
			scl = scf * (radius - (dy - (line + 1))) / MUL;
			if ((scf * (radius - (dy - (line + 1))) % MUL) >= (MUL / 2))	/* round up */
				scl++;
			ofs = radius - (q_circle[scl] * MUL / scf);
			ofl = corner_bl ? ofs : 0;
			ofr = corner_br ? ofs : 0;
		}
		else
		{
			int height = dy - ((corner_tl|corner_tr) + (corner_bl|corner_br)) * radius;
			blitRect(x, y + line, dx, height, col);
			line += height;
			continue;
		}
		blitRect(x + ofl, y + line, dx - ofl - ofr - 1, 1, col);
		line++;
	}
	update_dirty(x, y, x + dx, y + dy);
}


void CFrameBuffer::paintLine(int xa, int ya, int xb, int yb, const fb_pixel_t col)
{
	if (!getActive())
		return;

	int dx = abs (xa - xb);
	int dy = abs (ya - yb);
	int x;
	int y;
	int End;
	int step;

	if ( dx > dy )
	{
		int p = 2 * dy - dx;
		int twoDy = 2 * dy;
		int twoDyDx = 2 * (dy-dx);

		if (xa > xb)
		{
			x = xb;
			y = yb;
			End = xa;
			step = ya < yb ? -1 : 1;
		}
		else
		{
			x = xa;
			y = ya;
			End = xb;
			step = yb < ya ? -1 : 1;
		}

		paintPixel(x, y, col);

		while (x < End)
		{
			x++;
			if (p < 0)
				p += twoDy;
			else
			{
				y += step;
				p += twoDyDx;
			}
			paintPixel(x, y, col);
		}
	}
	else
	{
		int p = 2 * dx - dy;
		int twoDx = 2 * dx;
		int twoDxDy = 2 * (dx-dy);

		if (ya > yb)
		{
			x = xb;
			y = yb;
			End = ya;
			step = xa < xb ? -1 : 1;
		}
		else
		{
			x = xa;
			y = ya;
			End = yb;
			step = xb < xa ? -1 : 1;
		}

		paintPixel(x, y, col);

		while (y < End)
		{
			y++;
			if (p < 0)
				p += twoDx;
			else
			{
				x += step;
				p += twoDxDy;
			}
			paintPixel(x, y, col);
		}
	}
	update_dirty(xa, ya, xb, yb);
}

void CFrameBuffer::paintVLine(int x, int ya, int yb, const fb_pixel_t col)
{
	paintVLineRel(x, ya, yb - ya, col);
}

void CFrameBuffer::paintVLineRel(int x, int y, int dy, const fb_pixel_t col)
{
	if (!getActive())
		return;
	blitRect(x, y, 1, dy, col);
}

void CFrameBuffer::paintHLine(int xa, int xb, int y, const fb_pixel_t col)
{
	paintHLineRel(xa, xb - xa, y, col);
}

void CFrameBuffer::paintHLineRel(int x, int dx, int y, const fb_pixel_t col)
{
	if (!getActive())
		return;
	blitRect(x, y, dx, 1, col);
}

void CFrameBuffer::setIconBasePath(const std::string & iconPath)
{
	iconBasePath = iconPath;
}

void CFrameBuffer::getIconSize(const char * const filename, int* width, int *height)
{
	*width = 0;
	*height = 0;

	if(filename == NULL)
		return;

	std::map<std::string, rawIcon>::iterator it;


	/* if code ask for size, lets cache it. assume we have enough ram for cache */
	/* FIXME offset seems never used in code, always default = 1 ? */

	it = icon_cache.find(filename);
	if(it == icon_cache.end()) {
		if(paintIcon(filename, 0, 0, 0, 1, false)) {
			it = icon_cache.find(filename);
		}
	}
	if(it != icon_cache.end()) {
		*width = it->second.width;
		*height = it->second.height;
	}
}

bool CFrameBuffer::paintIcon8(const std::string & filename, const int x, const int y, const unsigned char offset)
{
	if (!getActive())
		return false;

//printf("%s(file, %d, %d, %d)\n", __FUNCTION__, x, y, offset);

	struct rawHeader header;
	uint16_t         width, height;
	int              lfd;

	lfd = open((iconBasePath + filename).c_str(), O_RDONLY);

	if (lfd == -1) {
		printf("paintIcon8: error while loading icon: %s%s\n", iconBasePath.c_str(), filename.c_str());
		return false;
	}

	read(lfd, &header, sizeof(struct rawHeader));

	width  = (header.width_hi  << 8) | header.width_lo;
	height = (header.height_hi << 8) | header.height_lo;

	unsigned char pixbuf[768];

	uint8_t * d = ((uint8_t *)getFrameBufferPointer()) + x * sizeof(fb_pixel_t) + stride * y;
	fb_pixel_t * d2;
	for (int count=0; count<height; count ++ ) {
		read(lfd, &pixbuf[0], width );
		unsigned char *pixpos = &pixbuf[0];
		d2 = (fb_pixel_t *) d;
		for (int count2=0; count2<width; count2 ++ ) {
			unsigned char color = *pixpos;
			if (color != header.transp) {
//printf("icon8: col %d transp %d real %08X\n", color+offset, header.transp, realcolor[color+offset]);
				paintPixel(d2, color + offset);
			}
			d2++;
			pixpos++;
		}
		d += stride;
	}
	close(lfd);
	update_dirty(x, y, x + width, y + height);
	blit();
	return true;
}

bool CFrameBuffer::blitToPrimary(unsigned int *, int, int, int, int)
{
	return false;
}

/* paint icon at position x/y,
   if height h is given, center vertically between y and y+h
   offset is a color offset (probably only useful with palette) */
bool CFrameBuffer::paintIcon(const std::string & filename, const int x, const int y,
			     const int h, const unsigned char offset, bool paint, bool paintBg, const fb_pixel_t colBg)
{
	struct rawHeader header;
	int         width, height;
	int              lfd;
	fb_pixel_t * data;
	struct rawIcon tmpIcon;
	std::map<std::string, rawIcon>::iterator it;
	int dsize;

	if (!getActive())
		return false;

	int  yy = y;
	//printf("CFrameBuffer::paintIcon: load %s\n", filename.c_str());fflush(stdout);

	/* we cache and check original name */
	it = icon_cache.find(filename);
	if(it == icon_cache.end()) {
		std::string newname = iconBasePath + filename.c_str() + ".png";
		//printf("CFrameBuffer::paintIcon: check for %s\n", newname.c_str());fflush(stdout);

		data = g_PicViewer->getIcon(newname, &width, &height);

		if(data) {
			dsize = width*height*sizeof(fb_pixel_t);
			//printf("CFrameBuffer::paintIcon: %s found, data %x size %d x %d\n", newname.c_str(), data, width, height);fflush(stdout);
			if(cache_size+dsize < ICON_CACHE_SIZE) {
				cache_size += dsize;
				tmpIcon.width = width;
				tmpIcon.height = height;
				tmpIcon.data = data;
				icon_cache.insert(std::pair <std::string, rawIcon> (filename, tmpIcon));
				//printf("Cached %s, cache size %d\n", newname.c_str(), cache_size);
			}
			goto _display;
		}

		newname = iconBasePath + filename.c_str() + ".raw";

		lfd = open(newname.c_str(), O_RDONLY);

		if (lfd == -1) {
			//printf("paintIcon: error while loading icon: %s\n", newname.c_str());
			return false;
		}
		read(lfd, &header, sizeof(struct rawHeader));

		tmpIcon.width = width  = (header.width_hi  << 8) | header.width_lo;
		tmpIcon.height = height = (header.height_hi << 8) | header.height_lo;

		dsize = width*height*sizeof(fb_pixel_t);

		tmpIcon.data = (fb_pixel_t*) cs_malloc_uncached(dsize);
		data = tmpIcon.data;

		unsigned char pixbuf[768];
		for (int count = 0; count < height; count ++ ) {
			read(lfd, &pixbuf[0], width >> 1 );
			unsigned char *pixpos = &pixbuf[0];
			for (int count2 = 0; count2 < width >> 1; count2 ++ ) {
				unsigned char compressed = *pixpos;
				unsigned char pix1 = (compressed & 0xf0) >> 4;
				unsigned char pix2 = (compressed & 0x0f);
				if (pix1 != header.transp)
					*data++ = realcolor[pix1+offset];
				else
					*data++ = 0;
				if (pix2 != header.transp)
					*data++ = realcolor[pix2+offset];
				else
					*data++ = 0;
				pixpos++;
			}
		}
		close(lfd);

		data = tmpIcon.data;

		if(cache_size+dsize < ICON_CACHE_SIZE) {
			cache_size += dsize;
			icon_cache.insert(std::pair <std::string, rawIcon> (filename, tmpIcon));
			//printf("Cached %s, cache size %d\n", newname.c_str(), cache_size);
		}
	} else {
		data = it->second.data;
		width = it->second.width;
		height = it->second.height;
		//printf("paintIcon: already cached %s %d x %d\n", newname.c_str(), width, height);
	}
_display:
	if(!paint)
		return true;

	if (h != 0)
		yy += (h - height) / 2;

	if (paintBg)
		paintBoxRel(x, yy, width, height, colBg);
	/* to make sure backbuffer is already blitted completely... */
	waitForIdle();
	blit2FB(data, width, height, x, yy, 0, 0);
	return true;
}

void CFrameBuffer::loadPal(const std::string & filename, const unsigned char offset, const unsigned char endidx)
{
	if (!getActive())
		return;

//printf("%s()\n", __FUNCTION__);

	struct rgbData rgbdata;
	int            lfd;

	lfd = open((iconBasePath + filename).c_str(), O_RDONLY);

	if (lfd == -1) {
		printf("error while loading palette: %s%s\n", iconBasePath.c_str(), filename.c_str());
		return;
	}

	int pos = 0;
	int readb = read(lfd, &rgbdata,  sizeof(rgbdata) );
	while(readb) {
		__u32 rgb = (rgbdata.r<<16) | (rgbdata.g<<8) | (rgbdata.b);
		int colpos = offset+pos;
		if( colpos>endidx)
			break;

		paletteSetColor(colpos, rgb, 0xFF);
		readb = read(lfd, &rgbdata,  sizeof(rgbdata) );
		pos++;
	}
	paletteSet(&cmap);
	close(lfd);
}

void CFrameBuffer::paintPixel(const int x, const int y, const fb_pixel_t col)
{
	if (!getActive())
		return;

	fb_pixel_t * pos = getFrameBufferPointer();
	pos += (stride / sizeof(fb_pixel_t)) * y;
	pos += x;

	*pos = col;
}

void CFrameBuffer::paintBoxFrame(const int sx, const int sy, const int dx, const int dy, const int px, const fb_pixel_t col, const int radius)
{
	if (!getActive())
		return;

	int c_radius = radius << 1;

	paintBoxRel(sx + radius , sy          , dx - c_radius, px,            col); // upper horizontal
	paintBoxRel(sx + radius , sy + dy - px, dx - c_radius, px,            col); // lower horizontal
	paintBoxRel(sx          , sy + radius , px,            dy - c_radius, col); // left vertical
	paintBoxRel(sx + dx - px, sy + radius , px,            dy - c_radius, col); // right vertical

	if (!radius)
		return;

	int x1 = sx + radius;
	int y1 = sy + radius;
	int x2 = sx + dx - radius -1;
	int y2 = sy + dy - radius -1;

	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = - c_radius;
	int x = 0;
	int y = radius;

	while (x < y)
	{
		// ddF_x == 2 * x + 1;
		// ddF_y == -2 * y;
		// f == x*x + y*y - radius*radius + 2*x - y + 1;
		if (f >= 0)
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		int width = 0;
		while (width <= px)
		{
			paintPixel(x2 + x        , y1 - y + width, col); // 1. oct
			paintPixel(x2 + y - width, y1 - x        , col); // 2. oct
			paintPixel(x2 + y - width, y2 + x        , col); // 3. oct
			paintPixel(x2 + x        , y2 + y - width, col); // 4. oct
			paintPixel(x1 - x        , y2 + y - width, col); // 5. oct
			paintPixel(x1 - y + width, y2 + x        , col); // 6. oct
			paintPixel(x1 - y + width, y1 - x        , col); // 7. oct
			paintPixel(x1 - x        , y1 - y + width, col); // 8. oct
			width++;
		}
	}
}


void CFrameBuffer::setBackgroundColor(const fb_pixel_t color)
{
	backgroundColor = color;
}

bool CFrameBuffer::loadPictureToMem(const std::string & filename, const uint16_t width, const uint16_t height, const uint16_t pstride, fb_pixel_t * memp)
{
	struct rawHeader header;
	int              lfd;

//printf("%s(%d, %d, memp)\n", __FUNCTION__, width, height);

	lfd = open((iconBasePath + filename).c_str(), O_RDONLY );

	if (lfd == -1)
	{
		printf("error while loading icon: %s%s\n", iconBasePath.c_str(), filename.c_str());
		return false;
	}

	read(lfd, &header, sizeof(struct rawHeader));

	if ((width  != ((header.width_hi  << 8) | header.width_lo)) ||
	    (height != ((header.height_hi << 8) | header.height_lo)))
	{
		printf("error while loading icon: %s - invalid resolution = %hux%hu\n", filename.c_str(), width, height);
		close(lfd);
		return false;
	}

	if ((pstride == 0) || (pstride == width * sizeof(fb_pixel_t)))
		read(lfd, memp, height * width * sizeof(fb_pixel_t));
	else
		for (int i = 0; i < height; i++)
			read(lfd, ((uint8_t *)memp) + i * pstride, width * sizeof(fb_pixel_t));

	close(lfd);
	return true;
}

bool CFrameBuffer::loadPicture2Mem(const std::string & filename, fb_pixel_t * memp)
{
	return loadPictureToMem(filename, BACKGROUNDIMAGEWIDTH, 576, 0, memp);
}

bool CFrameBuffer::loadPicture2FrameBuffer(const std::string & filename)
{
	if (!getActive())
		return false;

	return loadPictureToMem(filename, BACKGROUNDIMAGEWIDTH, 576, getStride(), getFrameBufferPointer());
}

bool CFrameBuffer::savePictureFromMem(const std::string & filename, const fb_pixel_t * const memp)
{
	struct rawHeader header;
	uint16_t         width, height;
	int              lfd;

	width = BACKGROUNDIMAGEWIDTH;
	height = 576;

	header.width_lo  = width  &  0xFF;
	header.width_hi  = width  >>    8;
	header.height_lo = height &  0xFF;
	header.height_hi = height >>    8;
	header.transp    =              0;

	lfd = open((iconBasePath + filename).c_str(), O_WRONLY | O_CREAT, 0644);

	if (lfd==-1)
	{
		printf("error while saving icon: %s%s", iconBasePath.c_str(), filename.c_str() );
		return false;
	}

	write(lfd, &header, sizeof(struct rawHeader));

	write(lfd, memp, width * height * sizeof(fb_pixel_t));

	close(lfd);
	return true;
}

bool CFrameBuffer::loadBackground(const std::string & filename, const unsigned char offset)
{
	if ((backgroundFilename == filename) && (background))
		return true;

	if (background)
		delete[] background;

	background = new fb_pixel_t[BACKGROUNDIMAGEWIDTH * 576];

	if (!loadPictureToMem(filename, BACKGROUNDIMAGEWIDTH, 576, 0, background))
	{
		delete[] background;
		background=0;
		return false;
	}

	if (offset != 0)//pic-offset
	{
		fb_pixel_t * bpos = background;
		int pos = BACKGROUNDIMAGEWIDTH * 576;
		while (pos > 0)
		{
			*bpos += offset;
			bpos++;
			pos--;
		}
	}

	fb_pixel_t * dest = background + BACKGROUNDIMAGEWIDTH * 576;
	uint8_t    * src  = ((uint8_t * )background)+ BACKGROUNDIMAGEWIDTH * 576;
	for (int i = 576 - 1; i >= 0; i--)
		for (int j = BACKGROUNDIMAGEWIDTH - 1; j >= 0; j--)
		{
			dest--;
			src--;
			paintPixel(dest, *src);
		}
	backgroundFilename = filename;

	return true;
}

bool CFrameBuffer::loadBackgroundPic(const std::string & filename, bool show)
{
	if ((backgroundFilename == filename) && (background))
		return true;

//printf("loadBackgroundPic: %s\n", filename.c_str());
	if (background){
		delete[] background;
		background = NULL;
	}
	background = g_PicViewer->getImage(iconBasePath + filename, BACKGROUNDIMAGEWIDTH, 576);

	if (background == NULL) {
		background=0;
		return false;
	}

	backgroundFilename = filename;
	if(show) {
		useBackgroundPaint = true;
		paintBackground();
	}
	return true;
}

void CFrameBuffer::useBackground(bool ub)
{
	useBackgroundPaint = ub;
	if(!useBackgroundPaint) {
		delete[] background;
		background=0;
	}
}

bool CFrameBuffer::getuseBackground(void)
{
	return useBackgroundPaint;
}

void CFrameBuffer::saveBackgroundImage(void)
{
	if (backupBackground != NULL){
		delete[] backupBackground;
		backupBackground = NULL;
	}
	backupBackground = background;
	//useBackground(false); // <- necessary since no background is available
	useBackgroundPaint = false;
	background = NULL;
}

void CFrameBuffer::restoreBackgroundImage(void)
{
	fb_pixel_t * tmp = background;

	if (backupBackground != NULL)
	{
		background = backupBackground;
		backupBackground = NULL;
	}
	else
		useBackground(false); // <- necessary since no background is available

	if (tmp != NULL){
		delete[] tmp;
		tmp = NULL;
	}
}

void CFrameBuffer::paintBackgroundBoxRel(int x, int y, int dx, int dy)
{
	if (!getActive())
		return;

	if(!useBackgroundPaint)
	{
		paintBoxRel(x, y, dx, dy, backgroundColor);
	}
	else
	{
		uint8_t * fbpos = ((uint8_t *)getFrameBufferPointer()) + x * sizeof(fb_pixel_t) + stride * y;
		fb_pixel_t * bkpos = background + x + BACKGROUNDIMAGEWIDTH * y;
		for(int count = 0;count < dy; count++)
		{
			memmove(fbpos, bkpos, dx * sizeof(fb_pixel_t));
			fbpos += stride;
			bkpos += BACKGROUNDIMAGEWIDTH;
		}
	}
}

void CFrameBuffer::paintBackground()
{
	if (!getActive())
		return;

	if (useBackgroundPaint && (background != NULL))
	{
		for (int i = 0; i < 576; i++)
			memmove(((uint8_t *)getFrameBufferPointer()) + i * stride, (background + i * BACKGROUNDIMAGEWIDTH), BACKGROUNDIMAGEWIDTH * sizeof(fb_pixel_t));
	}
	else
	{
		paintBoxRel(0, 0, xRes, yRes, backgroundColor);
	}
	blit();
}

void CFrameBuffer::SaveScreen(int x, int y, int dx, int dy, fb_pixel_t * const memp)
{
	if (!getActive())
		return;

	uint8_t * pos = ((uint8_t *)getFrameBufferPointer()) + x * sizeof(fb_pixel_t) + stride * y;
	fb_pixel_t * bkpos = memp;
	for (int count = 0; count < dy; count++) {
		fb_pixel_t * dest = (fb_pixel_t *)pos;
		for (int i = 0; i < dx; i++)
			//*(dest++) = col;
			*(bkpos++) = *(dest++);
		pos += stride;
	}
#if 0 //FIXME test to flush cache
        if (ioctl(fd, 1, FB_BLANK_UNBLANK) < 0);
#endif
	//RestoreScreen(x, y, dx, dy, memp); //FIXME
#if 0
	uint8_t * fbpos = ((uint8_t *)getFrameBufferPointer()) + x * sizeof(fb_pixel_t) + stride * y;
	fb_pixel_t * bkpos = memp;
	for (int count = 0; count < dy; count++)
	{
		memmove(bkpos, fbpos, dx * sizeof(fb_pixel_t));
		fbpos += stride;
		bkpos += dx;
	}
#endif

}

void CFrameBuffer::RestoreScreen(int x, int y, int dx, int dy, fb_pixel_t * const memp)
{
	if (!getActive())
		return;

	uint8_t * fbpos = ((uint8_t *)getFrameBufferPointer()) + x * sizeof(fb_pixel_t) + stride * y;
	fb_pixel_t * bkpos = memp;
	for (int count = 0; count < dy; count++)
	{
		memmove(fbpos, bkpos, dx * sizeof(fb_pixel_t));
		fbpos += stride;
		bkpos += dx;
	}
	update_dirty(x, y, x + dx, y + dy);
}

void CFrameBuffer::switch_signal(int)
{
}

void CFrameBuffer::Clear()
{
	paintBackground();
	//memset(getFrameBufferPointer(), 0, stride * yRes);
}

void CFrameBuffer::showFrame(const std::string & filename)
{
	std::string varpath = "/var/tuxbox/config/neutrino/icons/";
	if(!access((varpath + filename).c_str(), F_OK))
		videoDecoder->ShowPicture((varpath + filename).c_str());
	else
		videoDecoder->ShowPicture((iconBasePath + filename).c_str());
}

bool CFrameBuffer::Lock()
{
	if(locked)
		return false;
	locked = true;
	return true;
}

void CFrameBuffer::Unlock()
{
	locked = false;
}

void * CFrameBuffer::int_convertRGB2FB(unsigned char *rgbbuff, unsigned long x, unsigned long y, int transp, bool alpha)
{
	unsigned long i;
	unsigned int *fbbuff;
	unsigned long count = x * y;

	fbbuff = (unsigned int *) cs_malloc_uncached(count * sizeof(unsigned int));
	if(fbbuff == NULL) {
		printf("convertRGB2FB%s: Error: cs_malloc_uncached\n", ((alpha) ? " (Alpha)" : ""));
		return NULL;
	}

	if (alpha) {
		for(i = 0; i < count ; i++)
			fbbuff[i] = ((rgbbuff[i*4+3] << 24) & 0xFF000000) | 
			            ((rgbbuff[i*4]   << 16) & 0x00FF0000) | 
		        	    ((rgbbuff[i*4+1] <<  8) & 0x0000FF00) | 
			            ((rgbbuff[i*4+2])       & 0x000000FF);
	} else {
		switch (m_transparent) {
			case CFrameBuffer::TM_BLACK:
				for(i = 0; i < count ; i++) {
					transp = 0;
					if(rgbbuff[i*3] || rgbbuff[i*3+1] || rgbbuff[i*3+2])
						transp = 0xFF;
					fbbuff[i] = (transp << 24) | ((rgbbuff[i*3] << 16) & 0xFF0000) | ((rgbbuff[i*3+1] << 8) & 0xFF00) | (rgbbuff[i*3+2] & 0xFF);
				}
				break;
			case CFrameBuffer::TM_INI:
				for(i = 0; i < count ; i++)
					fbbuff[i] = (transp << 24) | ((rgbbuff[i*3] << 16) & 0xFF0000) | ((rgbbuff[i*3+1] << 8) & 0xFF00) | (rgbbuff[i*3+2] & 0xFF);
				break;
			case CFrameBuffer::TM_NONE:
			default:
				for(i = 0; i < count ; i++)
					fbbuff[i] = 0xFF000000 | ((rgbbuff[i*3] << 16) & 0xFF0000) | ((rgbbuff[i*3+1] << 8) & 0xFF00) | (rgbbuff[i*3+2] & 0xFF);
				break;
		}
	}
	return (void *) fbbuff;
}

void * CFrameBuffer::convertRGB2FB(unsigned char *rgbbuff, unsigned long x, unsigned long y, int transp)
{
	return int_convertRGB2FB(rgbbuff, x, y, transp, false);
}

void * CFrameBuffer::convertRGBA2FB(unsigned char *rgbbuff, unsigned long x, unsigned long y)
{
	return int_convertRGB2FB(rgbbuff, x, y, 0, true);
}

void CFrameBuffer::blit2FB(void *fbbuff, uint32_t width, uint32_t height, uint32_t xoff, uint32_t yoff, uint32_t xp, uint32_t yp, bool transp)
{
	int x, y, dw, dh;
	x = xoff;
	y = yoff;
	dw = width - xp;
	dh = height - yp;

	size_t mem_sz = width * height * sizeof(fb_pixel_t);
	unsigned long ulFlags = 0;
	if (!transp) /* transp == false (default) means: color "0x0" is transparent (??) */
		ulFlags = BLT_OP_FLAGS_BLEND_SRC_ALPHA|BLT_OP_FLAGS_BLEND_DST_MEMORY; // we need alpha blending

	if (fbbuff != backbuffer)
		memmove(backbuffer, fbbuff, mem_sz);

	STMFBIO_BLT_EXTERN_DATA blt_data;
	memset(&blt_data, 0, sizeof(STMFBIO_BLT_EXTERN_DATA));
	blt_data.operation  = BLT_OP_COPY;
	blt_data.ulFlags    = ulFlags;
	blt_data.srcOffset  = 0;
	blt_data.srcPitch   = width * 4;
	blt_data.dstOffset  = lbb_off;
	blt_data.dstPitch   = stride;
	blt_data.src_left   = xp;
	blt_data.src_top    = yp;
	blt_data.src_right  = width;
	blt_data.src_bottom = height;
	blt_data.dst_left   = x;
	blt_data.dst_top    = y;
	blt_data.dst_right  = x + dw;
	blt_data.dst_bottom = y + dh;
	blt_data.srcFormat  = SURF_ARGB8888;
	blt_data.dstFormat  = SURF_ARGB8888;
	blt_data.srcMemBase = (char *)backbuffer;
	blt_data.dstMemBase = (char *)lfb;
	blt_data.srcMemSize = mem_sz;
	blt_data.dstMemSize = stride * yRes + lbb_off;

	blit_lock();
	if (fbbuff != backbuffer)
		memmove(backbuffer, fbbuff, mem_sz);
	// icons are so small that they will still be in cache
	msync(backbuffer, backbuf_sz, MS_SYNC);

	if(ioctl(fd, STMFBIO_BLT_EXTERN, &blt_data) < 0)
		perror("blit2FB FBIO_BLIT");
	update_dirty(x, y, blt_data.dst_right, blt_data.dst_bottom);
	blit_unlock();
	return;
}

void CFrameBuffer::displayRGB(unsigned char *rgbbuff, int x_size, int y_size, int x_pan, int y_pan, int x_offs, int y_offs, bool clearfb, int transp)
{
	void *fbbuff = NULL;

	if (rgbbuff == NULL)
		return;

	/* correct panning */
	if(x_pan > x_size - (int)xRes) x_pan = 0;
	if(y_pan > y_size - (int)yRes) y_pan = 0;

	/* correct offset */
	if(x_offs + x_size > (int)xRes) x_offs = 0;
	if(y_offs + y_size > (int)yRes) y_offs = 0;

	/* blit buffer 2 fb */
	fbbuff = convertRGB2FB(rgbbuff, x_size, y_size, transp);
	if (fbbuff==NULL)
		return;

	/* ClearFB if image is smaller */
	/* if(x_size < (int)xRes || y_size < (int)yRes) */
	if(clearfb)
		CFrameBuffer::getInstance()->Clear();

	blit2FB(fbbuff, x_size, y_size, x_offs, y_offs, x_pan, y_pan);
	cs_free_uncached(fbbuff);
}

void CFrameBuffer::blitRect(int x, int y, int width, int height, unsigned long color)
{
//printf ("[fb - blitRect]: x=%d, y=%d, width=%d, height=%d\n", x, y, width, height);
	if (width <= 0 || height <= 0)
		return;

	/* maybe we should just return instead of fixing this up... */
	if (x < 0) {
		fprintf(stderr, "[neutrino] fb::%s: x < 0 (%d)\n", __func__, x);
		width += x;
		if (width <= 0)
			return;
		x = 0;
	}

	if (y < 0) {
		fprintf(stderr, "[neutrino] fb::%s: y < 0 (%d)\n", __func__, y);
		height += y;
		if (height <= 0)
			return;
		y = 0;
	}

	int right = x + width;
	int bottom = y + height;

	if (right > (int)xRes) {
		if (x >= (int)xRes) {
			fprintf(stderr, "[neutrino] fb::%s: x >= xRes (%d > %d)\n", __func__, x, xRes);
			return;
		}
		fprintf(stderr, "[neutrino] fb::%s: x + w > xRes! (%d+%d > %d)\n", __func__, x, width, xRes);
		width = xRes - x;
	}
	if (bottom > (int)yRes) {
		if (y >= (int)yRes) {
			fprintf(stderr, "[neutrino] fb::%s: y >= yRes (%d > %d)\n", __func__, y, yRes);
			return;
		}
		fprintf(stderr, "[neutrino] fb::%s: y + h > yRes! (%d+%d > %d)\n", __func__, y, height, yRes);
		height = yRes - y;
	}

	STMFBIO_BLT_DATA bltData;
	memset(&bltData, 0, sizeof(STMFBIO_BLT_DATA));

	bltData.operation  = BLT_OP_FILL;
	bltData.dstOffset  = lbb_off;
	bltData.dstPitch   = stride;

	bltData.dst_left   = x;
	bltData.dst_top    = y;
	bltData.dst_right  = x + width;
	bltData.dst_bottom = y + height;
	bltData.dst_right = right;
	bltData.dst_bottom = bottom;

	bltData.dstFormat  = SURF_ARGB8888;
	bltData.srcFormat  = SURF_ARGB8888;
	bltData.dstMemBase = STMFBGP_FRAMEBUFFER;
	bltData.srcMemBase = STMFBGP_FRAMEBUFFER;
	bltData.colour     = color;

	blit_lock();
	if (ioctl(fd, STMFBIO_BLT, &bltData ) < 0)
		fprintf(stderr, "blitRect FBIO_BLIT: %m x:%d y:%d w:%d h:%d s:%d\n", x,y,width,height,stride);
	update_dirty(x, y, bltData.dst_right, bltData.dst_bottom);
	blit_unlock();
}

void CFrameBuffer::blitIcon(int src_width, int src_height, int fb_x, int fb_y, int width, int height)
{
#ifdef EVOLUX
	if (!src_width || !src_height)
		return;
#endif
	STMFBIO_BLT_EXTERN_DATA blt_data;
	memset(&blt_data, 0, sizeof(STMFBIO_BLT_EXTERN_DATA));
	blt_data.operation  = BLT_OP_COPY;
	blt_data.ulFlags    = BLT_OP_FLAGS_BLEND_SRC_ALPHA | BLT_OP_FLAGS_BLEND_DST_MEMORY;	// we need alpha blending
	blt_data.srcOffset  = 0;
	blt_data.srcPitch   = src_width * 4;
	blt_data.dstOffset  = lbb_off;
	blt_data.dstPitch   = stride;
	blt_data.src_top    = 0;
	blt_data.src_left   = 0;
	blt_data.src_right  = src_width;
	blt_data.src_bottom = src_height;
	blt_data.dst_left   = fb_x;
	blt_data.dst_top    = fb_y;
	blt_data.dst_right  = fb_x + width;
	blt_data.dst_bottom = fb_y + height;
	blt_data.srcFormat  = SURF_ARGB8888;
	blt_data.dstFormat  = SURF_ARGB8888;
	blt_data.srcMemBase = (char *)backbuffer;
	blt_data.dstMemBase = (char *)lfb;
	blt_data.srcMemSize = backbuf_sz;
#ifdef EVOLUX
	blt_data.dstMemSize = stride * yRes + lbb_off;
#else
	blt_data.dstMemSize = stride * yRes;
#endif

	blit_lock();
	msync(backbuffer, blt_data.srcPitch * src_height, MS_SYNC);

	if(ioctl(fd, STMFBIO_BLT_EXTERN, &blt_data) < 0)
		perror("blit_icon FBIO_BLIT");
	update_dirty(fb_x, fb_y, blt_data.dst_right, blt_data.dst_bottom);
	blit_unlock();
}

#ifdef PARTIAL_BLIT
void CFrameBuffer::mark(int xs, int ys, int xe, int ye)
{
	update_dirty(xs, ys, xe, ye);
}
#else
void CFrameBuffer::mark(int, int, int, int)
{
}
#endif

#ifdef EVOLUX
void CFrameBuffer::blit()
{
	STMFBIO_BLT_DATA  bltData;
	memset(&bltData, 0, sizeof(STMFBIO_BLT_DATA));

	bltData.operation  = BLT_OP_COPY;

	// src
	bltData.srcOffset  = lbb_off;
	bltData.srcPitch   = stride;

	bltData.src_left   = 0;
	bltData.src_top    = 0;
	bltData.src_right  = xRes - 1;
	bltData.src_bottom = yRes - 1;

	bltData.srcFormat = SURF_BGRA8888;
	bltData.srcMemBase = STMFBGP_FRAMEBUFFER;

	/* calculate dst/blit factor */
	fb_var_screeninfo s;
	if (ioctl(fd, FBIOGET_VSCREENINFO, &s) == -1)
		perror("frameBuffer <FBIOGET_VSCREENINFO>");

	//bltData.dstOffset  = 0;
	bltData.dstPitch   = s.xres * 4;
	bltData.dstFormat = SURF_BGRA8888;
	bltData.dstMemBase = STMFBGP_FRAMEBUFFER;

	blit_lock();
	if(ioctl(fd, STMFBIO_SYNC_BLITTER) < 0)
		perror("CFrameBuffer::blit ioctl STMFBIO_SYNC_BLITTER 1");
	msync(lbb, xRes * 4 * yRes, MS_SYNC);

	switch(mode3D) {
	case Mode3D_off:
		bltData.dst_left   = 0;
		bltData.dst_top    = 0;
		bltData.dst_right  = s.xres;
		bltData.dst_bottom = s.yres;
		if (ioctl(fd, STMFBIO_BLT, &bltData ) < 0)
			perror("STMFBIO_BLT");
		break;
	case Mode3D_SideBySide:
		bltData.dst_left   = 0;
		bltData.dst_top    = 0;
		bltData.dst_right  = s.xres/2 - 1;
		bltData.dst_bottom = s.yres;
		if (ioctl(fd, STMFBIO_BLT, &bltData ) < 0)
			perror("STMFBIO_BLT");

		bltData.dst_left   = s.xres/2;
		bltData.dst_top    = 0;
		bltData.dst_right  = s.xres;
		bltData.dst_bottom = s.yres;
		if (ioctl(fd, STMFBIO_BLT, &bltData ) < 0)
			perror("STMFBIO_BLT");
		break;
	case Mode3D_TopAndBottom:
		bltData.dst_left   = 0;
		bltData.dst_top    = 0;
		bltData.dst_right  = s.xres - 1;
		bltData.dst_bottom = s.yres/2 - 1;
		if (ioctl(fd, STMFBIO_BLT, &bltData ) < 0)
			perror("STMFBIO_BLT");
		bltData.dst_left   = 0;
		bltData.dst_top    = s.yres/2;
		bltData.dst_right  = s.xres - 1;
		bltData.dst_bottom = s.yres - 1;
		if (ioctl(fd, STMFBIO_BLT, &bltData ) < 0)
			perror("STMFBIO_BLT");
		break;
	case Mode3D_Tile:
		bltData.dst_left   = 0;
		bltData.dst_top    = 0;
		bltData.dst_right  = (s.xres * 2) / 3 - 1;
		bltData.dst_bottom = (s.yres * 2) / 3;
		if (ioctl(fd, STMFBIO_BLT, &bltData ) < 0)
			perror("STMFBIO_BLT");
		bltData.src_left   = 0;
		bltData.src_top    = 0;
		bltData.src_right  = xRes/2 - 1;
		bltData.src_bottom = yRes - 1;
		bltData.dst_left   = (s.xres * 2) / 3;
		bltData.dst_top    = 0;
		bltData.dst_right  = s.xres - 1;
		bltData.dst_bottom = (s.yres * 2)/3 - 1;
		if (ioctl(fd, STMFBIO_BLT, &bltData ) < 0)
			perror("STMFBIO_BLT");

		bltData.src_left   = xRes/2;
		bltData.src_top    = 0;
		bltData.src_right  = xRes - 1;
		bltData.src_bottom = yRes/2 - 1;
		bltData.dst_left   = 0;
		bltData.dst_top    = (s.yres * 2)/3;
		bltData.dst_right  = (s.xres * 1)/3 - 1;
		bltData.dst_bottom = s.yres - 1;
		if (ioctl(fd, STMFBIO_BLT, &bltData ) < 0)
			perror("STMFBIO_BLT");
		bltData.src_left   = xRes/2;
		bltData.src_top    = yRes/2;
		bltData.src_right  = xRes - 1;
		bltData.src_bottom = yRes - 1;
		bltData.dst_left   = (s.xres * 1)/3;
		bltData.dst_top    = (s.yres * 2)/3;
		bltData.dst_right  = (s.xres * 2)/3 - 1;
		bltData.dst_bottom = s.yres - 1;
		if (ioctl(fd, STMFBIO_BLT, &bltData ) < 0)
			perror("STMFBIO_BLT");
#if 0
		// Should clear remaining block. This is just cosmetic, as
		// it won't be visible on a 3D-capable device. Besides, set3DMode()
		// did already clear the whose screen, so just forget about it.

		bltData.operation  = BLT_OP_FILL;
		bltData.dst_left   = (s.xres * 2)/3;
		bltData.dst_top    = (s.yres * 2)/3;
		bltData.dst_right  = s.xres - 1;
		bltData.dst_bottom = s.yres - 1;
		bltData.colour	   = 0;
		if (ioctl(fd, STMFBIO_BLT, &bltData ) < 0)
			perror("STMFBIO_BLT");
#endif
		break;
	}
	if(ioctl(fd, STMFBIO_SYNC_BLITTER) < 0)
		perror("CFrameBuffer::blit ioctl STMFBIO_SYNC_BLITTER 2");
	blit_unlock();
		
}
#else
void CFrameBuffer::blit()
{
#ifdef PARTIAL_BLIT
	if (to_blit.xs == INT_MAX)
		return;

	int srcXa = to_blit.xs;
	int srcYa = to_blit.ys;
	int srcXb = to_blit.xe;
	int srcYb = to_blit.ye;
#else
	const int srcXa = 0;
	const int srcYa = 0;
	int srcXb = xRes;
	int srcYb = yRes;
#endif
	STMFBIO_BLT_DATA  bltData;
	memset(&bltData, 0, sizeof(STMFBIO_BLT_DATA));

	bltData.operation  = BLT_OP_COPY;
	//bltData.ulFlags  = BLT_OP_FLAGS_BLEND_SRC_ALPHA | BLT_OP_FLAGS_BLEND_DST_MEMORY; // we need alpha blending
	// src
	bltData.srcOffset  = lbb_off;
	bltData.srcPitch   = stride;

	bltData.src_left   = srcXa;
	bltData.src_top    = srcYa;
	bltData.src_right  = srcXb;
	bltData.src_bottom = srcYb;

	bltData.srcFormat = SURF_BGRA8888;
	bltData.srcMemBase = STMFBGP_FRAMEBUFFER;

	/* calculate dst/blit factor */
	fb_var_screeninfo s;
	if (ioctl(fd, FBIOGET_VSCREENINFO, &s) == -1)
		perror("frameBuffer <FBIOGET_VSCREENINFO>");

#ifdef PARTIAL_BLIT
	if (s.xres != last_xres) /* fb resolution has changed -> clear artifacts */
	{
		last_xres = s.xres;
		bltData.src_left   = 0;
		bltData.src_top    = 0;
		bltData.src_right  = xRes;
		bltData.src_bottom = yRes;
	}

	double xFactor = (double)s.xres/(double)xRes;
	double yFactor = (double)s.yres/(double)yRes;

	int desXa = xFactor * bltData.src_left;
	int desYa = yFactor * bltData.src_top;
	int desXb = xFactor * bltData.src_right;
	int desYb = yFactor * bltData.src_bottom;
#else
	const int desXa = 0;
	const int desYa = 0;
	int desXb = s.xres;
	int desYb = s.yres;
#endif

	/* dst */
	bltData.dstOffset  = 0;
	bltData.dstPitch   = s.xres * 4;

	bltData.dst_left   = desXa;
	bltData.dst_top    = desYa;
	bltData.dst_right  = desXb;
	bltData.dst_bottom = desYb;

	bltData.dstFormat = SURF_BGRA8888;
	bltData.dstMemBase = STMFBGP_FRAMEBUFFER;

	//printf("CFrameBuffer::blit: sx:%d sy:%d sxe:%d sye: %d dx:%d dy:%d dxe:%d dye:%d\n", srcXa, srcYa, srcXb, srcYb, desXa, desYa, desXb, desYb);
	if ((bltData.dst_right > s.xres) || (bltData.dst_bottom > s.yres))
		printf("CFrameBuffer::blit: values out of range desXb:%d desYb:%d\n",
			bltData.dst_right, bltData.dst_bottom);

	blit_lock();
	if(ioctl(fd, STMFBIO_SYNC_BLITTER) < 0)
		perror("CFrameBuffer::blit ioctl STMFBIO_SYNC_BLITTER 1");
	msync(lbb, xRes * 4 * yRes, MS_SYNC);
	if (ioctl(fd, STMFBIO_BLT, &bltData ) < 0)
		perror("STMFBIO_BLT");
	if(ioctl(fd, STMFBIO_SYNC_BLITTER) < 0)
		perror("CFrameBuffer::blit ioctl STMFBIO_SYNC_BLITTER 2");

#ifdef PARTIAL_BLIT
	to_blit.xs = to_blit.ys = INT_MAX;
	to_blit.xe = to_blit.ye = 0;
#endif
	blit_unlock();
}
#endif

void CFrameBuffer::paintMuteIcon(bool paint, int ax, int ay, int dx, int dy, bool paintFrame)
{
	if(paint) {
		if (paintFrame) {
			paintBackgroundBoxRel(ax, ay, dx, dy);
			paintBoxRel(ax, ay, dx, dy, COL_MENUCONTENT_PLUS_0, RADIUS_SMALL);
		}
		int icon_dx=0, icon_dy=0;
		getIconSize(NEUTRINO_ICON_BUTTON_MUTE, &icon_dx, &icon_dy);
		paintIcon(NEUTRINO_ICON_BUTTON_MUTE, ax+((dx-icon_dx)/2), ay+((dy-icon_dy)/2));
	}
	else
		paintBackgroundBoxRel(ax, ay, dx, dy);
	blit();
}

#ifdef EVOLUX
CFrameBuffer::Mode3D CFrameBuffer::get3DMode() {
	return mode3D;
}

void CFrameBuffer::set3DMode(Mode3D m) {
	if (mode3D != m) {
		fb_var_screeninfo s;
		if (ioctl(fd, FBIOGET_VSCREENINFO, &s) == -1)
			perror("frameBuffer <FBIOGET_VSCREENINFO>");
		STMFBIO_BLT_DATA  bltData;
		memset(&bltData, 0, sizeof(STMFBIO_BLT_DATA));
		bltData.srcFormat = SURF_BGRA8888;
		bltData.srcMemBase = STMFBGP_FRAMEBUFFER;
		bltData.dstPitch   = s.xres * 4;
		bltData.dstFormat = SURF_BGRA8888;
		bltData.dstMemBase = STMFBGP_FRAMEBUFFER;
		bltData.operation  = BLT_OP_FILL;
		bltData.dst_right  = s.xres - 1;
		bltData.dst_bottom = s.yres - 1;
		blit_lock();
		if (ioctl(fd, STMFBIO_BLT, &bltData ) < 0)
			perror("STMFBIO_BLT");
		blit_unlock();
		mode3D = m;
		blit();
	}
}

bool CFrameBuffer::OSDShot(const std::string &name) {
	struct timeval ts, te;
	gettimeofday(&ts, NULL);

	size_t l = name.find_last_of(".");
	if(l == std::string::npos)
		return false;
	if (name.substr(l) != ".png")
		return false;
	FILE *out = fopen(name.c_str(), "w");
	if (!out)
		return false;

	png_bytep row_pointers[DEFAULT_YRES];
	png_structp png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING,
		(png_voidp) NULL, (png_error_ptr) NULL, (png_error_ptr) NULL);
	png_infop info_ptr = png_create_info_struct(png_ptr);

	png_init_io(png_ptr, out);

	for (unsigned int y = 0; y < DEFAULT_YRES; y++)
		row_pointers[y] = (png_bytep) ((fb_pixel_t *)lbb + y * DEFAULT_XRES);

	png_set_compression_level(png_ptr, 1);
	png_set_bgr(png_ptr);
	png_set_IHDR(png_ptr, info_ptr, DEFAULT_XRES, DEFAULT_YRES, 8, PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_write_info(png_ptr, info_ptr);
	png_write_image(png_ptr, row_pointers);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	fclose(out);

	gettimeofday(&te, NULL);
	fprintf(stderr, "%s took %lld us\n", __func__, (te.tv_sec * 1000000LL + te.tv_usec) - (ts.tv_sec * 1000000LL + ts.tv_usec));
	return true;
}
#endif
