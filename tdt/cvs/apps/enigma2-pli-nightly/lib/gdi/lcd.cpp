#include <lib/gdi/lcd.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#if defined(HAVE_DBOX_FP_H) && defined(HAVE_DBOX_LCD_KS0713_H)
#include <dbox/fp.h>
#include <dbox/lcd-ks0713.h>
#else
#define NO_LCD 1
#endif

#include <lib/gdi/esize.h>
#include <lib/base/init.h>
#include <lib/base/init_num.h>
#ifdef HAVE_TEXTLCD
	#include <lib/base/estring.h>
#endif
#include <lib/gdi/glcddc.h>

eDBoxLCD *eDBoxLCD::instance;

eLCD::eLCD()
{
	lcdfd = -1;
	locked=0;
}

void eLCD::setSize(int xres, int yres, int bpp)
{
	res = eSize(xres, yres);
	_buffer=new unsigned char[xres * yres * bpp/8];
	memset(_buffer, 0, res.height()*res.width()*bpp/8);
	_stride=res.width()*bpp/8;
	eDebug("lcd buffer %p %d bytes, stride %d", _buffer, xres*yres*bpp/8, _stride);
}

eLCD::~eLCD()
{
	delete [] _buffer;
}

int eLCD::lock()
{
	if (locked)
		return -1;

	locked=1;
	return lcdfd;
}

void eLCD::unlock()
{
	locked=0;
}

#ifdef HAVE_TEXTLCD
void eLCD::renderText(ePoint start, const char *text)
{
	if (lcdfd >= 0 && start.y() < 5)
	{
		std::string message = text;
		message = replace_all(message, "\n", " ");
		::write(lcdfd, message.c_str(), message.size());
	}
}
#endif

#ifndef __sh__    

eDBoxLCD::eDBoxLCD()
{
	int xres=132, yres=64, bpp=8;
	is_oled = 0;
#ifndef NO_LCD
	lcdfd = open("/dev/dbox/oled0", O_RDWR);
	if (lcdfd < 0)
	{
		FILE *f=fopen("/proc/stb/lcd/oled_brightness", "w");
		if (!f)
			f = fopen("/proc/stb/fp/oled_brightness", "w");
		if (f)
		{
			is_oled = 2;
			fclose(f);
		}
		lcdfd = open("/dev/dbox/lcd0", O_RDWR);
	} else
	{
		eDebug("found OLED display!");
		is_oled = 1;
	}
#else
	lcdfd = -1;
#endif
	instance=this;

	if (lcdfd<0)
		eDebug("couldn't open LCD - load lcd.o!");
	else
	{
		int i=LCD_MODE_BIN;
		ioctl(lcdfd, LCD_IOCTL_ASC_MODE, &i);
		inverted=0;
		FILE *f = fopen("/proc/stb/lcd/xres", "r");
		if (f)
		{
			int tmp;
			if (fscanf(f, "%x", &tmp) == 1)
				xres = tmp;
			fclose(f);
			f = fopen("/proc/stb/lcd/yres", "r");
			if (f)
			{
				if (fscanf(f, "%x", &tmp) == 1)
					yres = tmp;
				fclose(f);
				f = fopen("/proc/stb/lcd/bpp", "r");
				if (f)
				{
					if (fscanf(f, "%x", &tmp) == 1)
						bpp = tmp;
					fclose(f);
				}
			}
			is_oled = 3;
		}
	}
	setSize(xres, yres, bpp);
}

void eDBoxLCD::setInverted(unsigned char inv)
{
	inverted=inv;
	update();
}

int eDBoxLCD::setLCDContrast(int contrast)
{
	int fp;
	if((fp=open("/dev/dbox/fp0", O_RDWR))<=0)
	{
		eDebug("[LCD] can't open /dev/dbox/fp0");
		return(-1);
	}

	if(ioctl(lcdfd, LCD_IOCTL_SRV, &contrast))
	{
		eDebug("[LCD] can't set lcd contrast");
	}
	close(fp);
	return(0);
}

int eDBoxLCD::setLCDBrightness(int brightness)
{
	eDebug("setLCDBrightness %d", brightness);
	FILE *f=fopen("/proc/stb/lcd/oled_brightness", "w");
	if (!f)
		f = fopen("/proc/stb/fp/oled_brightness", "w");
	if (f)
	{
		if (fprintf(f, "%d", brightness) == 0)
			eDebug("write /proc/stb/lcd/oled_brightness failed!! (%m)");
		fclose(f);
	}
	else
	{
		int fp;
		if((fp=open("/dev/dbox/fp0", O_RDWR))<=0)
		{
			eDebug("[LCD] can't open /dev/dbox/fp0");
			return(-1);
		}

		if(ioctl(fp, FP_IOCTL_LCD_DIMM, &brightness)<=0)
			eDebug("[LCD] can't set lcd brightness (%m)");
		close(fp);
	}
	return(0);
}

eDBoxLCD::~eDBoxLCD()
{
	if (lcdfd>=0)
	{
		close(lcdfd);
		lcdfd=-1;
	}
}

eDBoxLCD *eDBoxLCD::getInstance()
{
	return instance;
}

void eDBoxLCD::update()
{
	if (lcdfd >= 0)
	{
		if (!is_oled || is_oled == 2)
		{
			unsigned char raw[132*8];
			int x, y, yy;
			for (y=0; y<8; y++)
			{
				for (x=0; x<132; x++)
				{
					int pix=0;
					for (yy=0; yy<8; yy++)
					{
						pix|=(_buffer[(y*8+yy)*132+x]>=108)<<yy;
					}
					raw[y*132+x]=(pix^inverted);
				}
			}
			write(lcdfd, raw, 132*8);
		}
		else if (is_oled == 3)
			write(lcdfd, _buffer, _stride * res.height());
		else
		{
			unsigned char raw[64*64];
			int x, y;
			memset(raw, 0, 64*64);
			for (y=0; y<64; y++)
			{
				int pix=0;
				for (x=0; x<128 / 2; x++)
				{
					pix = (_buffer[y*132 + x * 2 + 2] & 0xF0) |(_buffer[y*132 + x * 2 + 1 + 2] >> 4);
					if (inverted)
						pix = 0xFF - pix;
					raw[y*64+x] = pix;
				}
			}
			write(lcdfd, raw, 64*64);
		}
	}
}

#else

void eDBoxLCD::setFlipped(bool onoff)
{
	flipped = onoff;
	update();
}

/* **************************************************************** */
/* Pearl LCD */

eDBoxLCD::eDBoxLCD()
{
	eDebug("eDBoxLCD::eDBoxLCD >");

	displayNumber = 0;
	is_oled = 1;
    
	instance=this;

	if (GLCD::Config.Load("/etc/graphlcd.conf") == false)
	{
		eDebug("Error loading config file!\n");
		return;
	}
	if (GLCD::Config.driverConfigs.size() <= 0)
	{
		eDebug("ERROR: No displays specified in config file!\n");
	}

	GLCD::Config.driverConfigs[displayNumber].upsideDown ^= 0;
	GLCD::Config.driverConfigs[displayNumber].invert ^= 0;

	lcd = GLCD::CreateDriver(GLCD::Config.driverConfigs[displayNumber].id, &GLCD::Config.driverConfigs[displayNumber]);
	
    if (!lcd)
	{
		eDebug("ERROR: Failed creating display object\n");
        return;
	}
	if (lcd->Init() != 0)
	{
#if 0
	// Returning an error here will break the code at various other places
		eDebug("ERROR: Failed initializing display\n");
		delete lcd;
		lcd = NULL;
		return;
#endif
	}
	lcd->SetBrightness(GLCD::Config.driverConfigs[displayNumber].brightness);

	lcd->GetFeature((std::string) "depth", depth);
	width = GLCD::Config.driverConfigs[displayNumber].width;
	height = GLCD::Config.driverConfigs[displayNumber].height;

	eDebug("config -> (w %d, h %d)", width, height);

	bitmap = new GLCD::cBitmap(width, height);
	bitmap->Clear();

	lcd->SetScreen(bitmap->Data(), bitmap->Width(), bitmap->Height());
	lcd->Refresh(true);

	lcdfd = 1; //needed for detected()
	setSize(width, height, depth);

	eDebug("eDBoxLCD::eDBoxLCD (w %d, h %d, depth %d)<", width, height, depth);
}

void eDBoxLCD::setInverted(unsigned char inv)
{
	eDebug("eDBoxLCD::setInverted");
	inverted = inv;
	update();
}

int eDBoxLCD::setLCDContrast(int contrast)
{
	eDebug("[LCD] setLCDContrast not supported");
	return(0);
}

int eDBoxLCD::setLCDBrightness(int brightness)
{
	eDebug("eDBoxLCD::setLCDBrightness");
/* fixme range check */
	lcd->SetBrightness(brightness);
	return(0);
}

eDBoxLCD::~eDBoxLCD()
{
	eDebug("eDBoxLCD::~eDBoxLCD");

/* konfetti: there is an error in graphlcd (I reported it alread)
 * ->InitSingleDisplay returns 0 if dpf_open fails, which leads
 * to a crash here!
    if (lcd)
    {
	   lcd->DeInit();
       delete lcd;
    }
    if (bitmap)
      delete bitmap;
*/
}

eDBoxLCD *eDBoxLCD::getInstance()
{
	eDebug("eDBoxLCD::getInstance");
	return instance;
}

void eDBoxLCD::update()
{
    if (lcdfd == 1)
    {

       bitmap->Clear();

       for (int x = 0; x < width; x++)
          for (int y = 0; y < height; y++)
          { 
             __u16 *buf16  = (__u16*) _buffer;

#if BYTE_ORDER == LITTLE_ENDIAN
             __u16 col16   = bswap_16(*((__u16*)(((__u16*)buf16) + y * width + x)));
#else
             __u16 col16   = *((__u16*)(((__u16*)buf16) + y  * width + x));
#endif
             __u8 red, green, blue, alpha; 
             __u32 color32;

             /* BBBBB GGGGGG RRRRR */
 		     blue  = ((col16 & 0xF800) >> 11) * ( 255 / 31);
             green = ((col16 & 0x7E0) >> 5) * (255 / 63);
             red   = (col16 & 0x1f) * (255 / 31);
             alpha = 255;

             color32 = alpha << 24 | red << 16 | green << 8 | blue;

		     if (inverted)
			     color32 = 0xFFFFFF - color32;

		     bitmap->DrawPixel(x, y, color32);
          }
	   lcd->SetScreen(bitmap->Data(), bitmap->Width(), bitmap->Height());
       lcd->Refresh(false); /* partial update */
   }
}

#endif    
