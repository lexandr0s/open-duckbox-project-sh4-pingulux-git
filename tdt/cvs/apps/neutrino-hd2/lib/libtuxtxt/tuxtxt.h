#ifndef __tuxtxt_h__
#define __tuxtxt_h__

/******************************************************************************
 *                      <<< TuxTxt - Teletext Plugin >>>                      *
 *                                                                            *
 *             (c) Thomas "LazyT" Loewe 2002-2003 (LazyT@gmx.net)             *
 *                                                                            *
 *    continued 2004-2005 by Roland Meier <RolandMeier@Siemens.com>           *
 *                       and DBLuelle <dbluelle@blau-weissoedingen.de>        *
 *                                                                            *
 *              ported 2006 to Dreambox 7025 / 32Bit framebuffer              *
 *                   by Seddi <seddi@i-have-a-dreambox.com>                   *
 *                                                                            *
 ******************************************************************************/

#define TUXTXT_CFG_STANDALONE 0  // 1:plugin only 0:use library
#define TUXTXT_DEBUG 0


#include <config.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>

#include <linux/fb.h>

#include <linux/input.h>
#include <linux/videodev.h>

#include <sys/ioctl.h>
#include <sys/mman.h>

#include "tuxtxt_def.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CACHE_H
#include FT_CACHE_SMALL_BITMAPS_H

/* devices */

#if TUXTXT_CFG_STANDALONE
#include "tuxtxt_common.h"
#else
/* variables and functions from libtuxtxt */
extern tuxtxt_cache_struct tuxtxt_cache;
extern int tuxtxt_init();
extern void tuxtxt_close();
extern void tuxtxt_start(int tpid, int source );  // Start caching
extern int  tuxtxt_stop(); // Stop caching
extern void tuxtxt_next_dec(int *i); /* skip to next decimal */
extern void tuxtxt_prev_dec(int *i); /* counting down */
extern int tuxtxt_is_dec(int i);
extern int tuxtxt_next_hex(int i);
extern void tuxtxt_decode_btt();
extern void tuxtxt_decode_adip(); /* additional information table */
extern void tuxtxt_compress_page(int p, int sp, unsigned char* buffer);
extern void tuxtxt_decompress_page(int p, int sp, unsigned char* buffer);
#if TUXTXT_DEBUG
extern int tuxtxt_get_zipsize(int p, int sp);
#endif
#endif

#define TUXTXTCONF CONFIGDIR "/tuxtxt/tuxtxt2.conf"

/* fonts */
#define TUXTXTTTF FONTDIR "/tuxtxt.ttf"
#define TUXTXTOTB FONTDIR "/tuxtxt.otb"
/* alternative fontdir */
#define TUXTXTTTFVAR "/var/tuxtxt/tuxtxt.ttf"
#define TUXTXTOTBVAR "/var/tuxtxt/tuxtxt.otb"

int TTFWidthFactor16, TTFHeightFactor16, TTFShiftX, TTFShiftY; /* parameters for adapting to various TTF fonts */
int fontheight, fontwidth, fontwidth_normal, fontwidth_small, fontwidth_topmenumain, fontwidth_topmenusmall, ascender;
int ymosaic[4];
int displaywidth;

#define fontwidth_small_lcd 8

#define TV43STARTX (ex - 146) 		//(StartX + 2 + (40-nofirst)*fontwidth_topmenumain + (40*fontwidth_topmenumain/abx))
#define TV169FULLSTARTX /*(sx+ 8*40)*/ 	(sx +(ex +1 - sx)/2)
#define TVENDX ex
#define TVENDY (StartY + 25*fontheight)
#define TV43WIDTH 144 			/* 120 */
#define TV43HEIGHT 116 			/* 96 */
#define TV43STARTY (TVENDY - TV43HEIGHT)
#define TV169FULLSTARTY (720 - 360)/2 //sy
#define TV169FULLWIDTH  (ex - sx)/2
#define TV169FULLHEIGHT 360 //(ey - sy)

#define TOPMENUSTARTX TV43STARTX+2
#define TOPMENUENDX TVENDX
#define TOPMENUSTARTY StartY
#define TOPMENUENDY TV43STARTY

#define TOPMENULINEWIDTH ((TOPMENUENDX-TOPMENU43STARTX+fontwidth_topmenusmall-1)/fontwidth_topmenusmall)
#define TOPMENUINDENTBLK 0
#define TOPMENUINDENTGRP 1
#define TOPMENUINDENTDEF 2
#define TOPMENUSPC 0
#define TOPMENUCHARS (TOPMENUINDENTDEF+12+TOPMENUSPC+4)

#define FLOFSIZE 4

/* spacing attributes */
#define alpha_black         0x00
#define alpha_red           0x01
#define alpha_green         0x02
#define alpha_yellow        0x03
#define alpha_blue          0x04
#define alpha_magenta       0x05
#define alpha_cyan          0x06
#define alpha_white         0x07
#define flash               0x08
#define steady              0x09
#define end_box             0x0A
#define start_box           0x0B
#define normal_size         0x0C
#define double_height       0x0D
#define double_width        0x0E
#define double_size         0x0F
#define mosaic_black        0x10
#define mosaic_red          0x11
#define mosaic_green        0x12
#define mosaic_yellow       0x13
#define mosaic_blue         0x14
#define mosaic_magenta      0x15
#define mosaic_cyan         0x16
#define mosaic_white        0x17
#define conceal             0x18
#define contiguous_mosaic   0x19
#define separated_mosaic    0x1A
#define esc                 0x1B
#define black_background    0x1C
#define new_background      0x1D
#define hold_mosaic         0x1E
#define release_mosaic      0x1F

/* rc codes */
#define RC_0        0x00
#define RC_1        0x01
#define RC_2        0x02
#define RC_3        0x03
#define RC_4        0x04
#define RC_5        0x05
#define RC_6        0x06
#define RC_7        0x07
#define RC_8        0x08
#define RC_9        0x09

#define RC_RIGHT    0x0A
#define RC_LEFT     0x0B
#define RC_UP       0x0C
#define RC_DOWN     0x0D

#define RC_OK       0x0E
#define RC_MUTE     0x0F
#define RC_STANDBY  0x10
#define RC_GREEN    0x11
#define RC_YELLOW   0x12
#define RC_RED      0x13
#define RC_BLUE     0x14
#define RC_PLUS     0x15
#define RC_MINUS    0x16
#define RC_HELP     0x17
#define RC_DBOX     0x18
#define RC_TEXT     0x19
#define RC_HOME     0x1F

typedef enum /* object type */
{
	OBJ_PASSIVE,
	OBJ_ACTIVE,
	OBJ_ADAPTIVE
} tObjType;

const char *ObjectSource[] =
{
	"(illegal)",
	"Local",
	"POP",
	"GPOP"
};

const char *ObjectType[] =
{
	"Passive",
	"Active",
	"Adaptive",
	"Passive"
};

/* messages */
#define ShowInfoBar     0
//#define PageNotFound    1
#define ShowServiceName 2
#define NoServicesFound 3

/* framebuffer stuff */
static unsigned char *lfb = 0;

/* freetype stuff */
FT_Library      library;
FTC_Manager     manager;
static FTC_SBitCache   cache;
FTC_SBit        sbit;

#define FONTTYPE FTC_ImageTypeRec

FT_Face			face;
FONTTYPE typettf;

// G2 Charset (0 = Latin, 1 = Cyrillic, 2 = Greek)
const unsigned short int G2table[3][6*16] =
{
	{ ' ' ,'�' ,'�' ,'�' ,'$' ,'�' ,'#' ,'�' ,'�' ,'\'','\"','�' ,8592,8594,8595,8593,
	  '�' ,'�' ,'�' ,'�' ,'x' ,'�' ,'�' ,'�' ,'�' ,'\'','\"','�' ,'�' ,'�' ,'�' ,'�' ,
	  ' ' ,'`' ,'�' ,710 ,732 ,'�' ,728 ,729 ,733 ,716 ,730 ,719 ,'_' ,698 ,718 ,711 ,
	  '�' ,'�' ,'�' ,'�' ,8482,9834,8364,8240,945 ,' ' ,' ' ,' ' ,8539,8540,8541,8542,
	  937 ,'�' ,272 ,'�' ,294 ,' ' ,306 ,319 ,321 ,'�' ,338 ,'�' ,'�' ,358 ,330 ,329 ,
	  1082,'�' ,273 ,'�' ,295 ,305 ,307 ,320 ,322 ,'�' ,339 ,'�' ,'�' ,359 ,951 ,0x7F},
	  
	{ ' ' ,'�' ,'�' ,'�' ,'$' ,'�' ,' ' ,'�' ,' ' ,'\'','\"','�' ,8592,8594,8595,8593,
	  '�' ,'�' ,'�' ,'�' ,'x' ,'�' ,'�' ,'�' ,'�' ,'\'','\"','�' ,'�' ,'�' ,'�' ,'�' ,
	  ' ' ,'`' ,'�' ,710 ,732 ,'�' ,728 ,729 ,733 ,716 ,730 ,719 ,'_' ,698 ,718 ,711 ,
	  '�' ,'�' ,'�' ,'�' ,8482,9834,8364,8240,945 ,321 ,322 ,'�' ,8539,8540,8541,8542,
	  'D' ,'E' ,'F' ,'G' ,'I' ,'J' ,'K' ,'L' ,'N' ,'Q' ,'R' ,'S' ,'U' ,'V' ,'W' ,'Z' ,
	  'd' ,'e' ,'f' ,'g' ,'i' ,'j' ,'k' ,'l' ,'n' ,'q' ,'r' ,'s' ,'u' ,'v' ,'w' ,'z' },
	  
	{ ' ' ,'a' ,'b' ,'�' ,'e' ,'h' ,'i' ,'�' ,':' ,'\'','\"','k' ,8592,8594,8595,8593,
	  '�' ,'�' ,'�' ,'�' ,'x' ,'m' ,'n' ,'p' ,'�' ,'\'','\"','t' ,'�' ,'�' ,'�' ,'x' ,
	  ' ' ,'`' ,'�' ,710 ,732 ,'�' ,728 ,729 ,733 ,716 ,730 ,719 ,'_' ,698 ,718 ,711 ,
	  '?' ,'�' ,'�' ,'�' ,8482,9834,8364,8240,945 ,906 ,910 ,911 ,8539,8540,8541,8542,
	  'C' ,'D' ,'F' ,'G' ,'J' ,'L' ,'Q' ,'R' ,'S' ,'U' ,'V' ,'W' ,'Y' ,'Z' ,902 ,905 ,
	  'c' ,'d' ,'f' ,'g' ,'j' ,'l' ,'q' ,'r' ,'s' ,'u' ,'v' ,'w' ,'y' ,'z' ,904 ,0x7F}
};

// cyrillic G0 Charset
// TODO: different maps for serbian/russian/ukrainian
const unsigned short int G0tablecyrillic[6*16] =
{
	  ' ' ,'!' ,'\"','#' ,'$' ,'%' ,'&' ,'\'','(' ,')' ,'*' ,'+' ,',' ,'-' ,'.' ,'/' ,
	  '0' ,'1' ,'2' ,'3' ,'4' ,'5' ,'6' ,'7' ,'8' ,'9' ,':' ,';' ,'<' ,'=' ,'>' ,'?' ,
	  1063,1040,1041,1062,1044,1045,1060,1043,1061,1048,1032,1050,1051,1052,1053,1054,
	  1055,1036,1056,1057,1058,1059,1042,1027,1033,1034,1047,1035,1046,1026,1064,1119,
	  1095,1072,1073,1094,1076,1077,1092,1075,1093,1080,1112,1082,1083,1084,1085,1086,
	  1087,1116,1088,1089,1090,1091,1074,1107,1113,1114,1079,1115,1078,1106,1096,0x7F
};

const unsigned short int nationaltable23[14][2] =
{
	{ '#', '�' }, /* 0          */
	{ '#', 367 }, /* 1  CS/SK   */
	{ '�', '$' }, /* 2    EN    */
	{ '#', '�' }, /* 3    ET    */
	{ '�', '�' }, /* 4    FR    */
	{ '#', '$' }, /* 5    DE    */
	{ '�', '$' }, /* 6    IT    */
	{ '#', '$' }, /* 7  LV/LT   */
	{ '#', 329 }, /* 8    PL    */
	{ '�', '$' }, /* 9  PT/ES   */
	{ '#', '�' }, /* A    RO    */
	{ '#', '�' }, /* B SR/HR/SL */
	{ '#', '�' }, /* C SV/FI/HU */
	{ '�', 287 }, /* D    TR   ? */
};

const unsigned short int nationaltable40[14] =
{
	'@', /* 0          */
	269, /* 1  CS/SK   */
	'@', /* 2    EN    */
	352, /* 3    ET    */
	'�', /* 4    FR    */
	'�', /* 5    DE    */
	'�', /* 6    IT    */
	352, /* 7  LV/LT   */
	261, /* 8    PL    */
	'�', /* 9  PT/ES   */
	354, /* A    RO    */
	268, /* B SR/HR/SL */
	'�', /* C SV/FI/HU */
	304, /* D    TR    */
};

const unsigned short int nationaltable5b[14][6] =
{
	{ '[','\\', ']', '^', '_', '`' }, /* 0          */
	{ 357, 382, '�', '�', 345, '�' }, /* 1  CS/SK   */
	{8592, '�',8594,8593, '#', 822 }, /* 2    EN    */
	{ '�', '�', 381, '�', '�', 353 }, /* 3    ET    */
	{ '�', '�', '�', '�', '#', '�' }, /* 4    FR    */
	{ '�', '�', '�', '^', '_', '�' }, /* 5    DE    */
	{ '�', '�',8594,8593, '#', '�' }, /* 6    IT    */
	{ '�', 553, 381, 269, 363, 353 }, /* 7  LV/LT   */
	{ 437, 346, 321, 263, '�', 281 }, /* 8    PL    */
	{ '�', '�', '�', '�', '�', '�' }, /* 9  PT/ES   */
	{ '�', 350, 461, '�', 305, 355 }, /* A    RO    */
	{ 262, 381, 272, 352, '�', 269 }, /* B SR/HR/SL */
	{ '�', '�', '�', '�', '_', '�' }, /* C SV/FI/HU */
	{ 350, '�', '�', '�', 486, 305 }, /* D    TR    */
};

const unsigned short int nationaltable7b[14][4] =
{
	{ '{', '|', '}', '~' }, /* 0          */
	{ '�', 283, '�', 353 }, /* 1  CS/SK   */
	{ '�',8214, '�', '�' }, /* 2    EN    */
	{ '�', '�', 382, '�' }, /* 3    ET    */
	{ '�', '�', '�', '�' }, /* 4    FR    */
	{ '�', '�', '�', '�' }, /* 5    DE    */
	{ '�', '�', '�', '�' }, /* 6    IT    */
	{ 261, 371, 382, 303 }, /* 7  LV/LT   */
	{ 380, 347, 322, 378 }, /* 8    PL    */
	{ '�', '�', '�', '�' }, /* 9  PT/ES   */
	{ '�', 351, 462, '�' }, /* A    RO    */
	{ 263, 382, 273, 353 }, /* B SR/HR/SL */
	{ '�', '�', '�', '�' }, /* C SV/FI/HU */
	{ 351, '�', 231, '�' }, /* D    TR    */
};

const unsigned short int arrowtable[] =
{
	8592, 8594, 8593, 8595, 'O', 'K', 8592, 8592
};

/* national subsets */
const char countrystring[] =
"          (#$@[\\]^_`{|}~) "   /*  0 no subset specified */
"  CS/SK   (#$@[\\]^_`{|}~) "   /*  1 czech, slovak */
"    EN    (#$@[\\]^_`{|}~) "   /*  2 english */
"    ET    (#$@[\\]^_`{|}~) "   /*  3 estonian */
"    FR    (#$@[\\]^_`{|}~) "   /*  4 french */
"    DE    (#$@[\\]^_`{|}~) "   /*  5 german */
"    IT    (#$@[\\]^_`{|}~) "   /*  6 italian */
"  LV/LT   (#$@[\\]^_`{|}~) "   /*  7 latvian, lithuanian */
"    PL    (#$@[\\]^_`{|}~) "   /*  8 polish */
"  PT/ES   (#$@[\\]^_`{|}~) "   /*  9 portuguese, spanish */
"    RO    (#$@[\\]^_`{|}~) "   /* 10 romanian */
" SR/HR/SL (#$@[\\]^_`{|}~) "   /* 11 serbian, croatian, slovenian */
" SV/FI/HU (#$@[\\]^_`{|}~) "   /* 12 swedish, finnish, hungarian */
"    TR    (#$@[\\]^_`{|}~) "   /* 13 turkish */
" RU/BUL/SER/CRO/UKR (cyr) "   /* 14 cyrillic */
"    EK                    "   /* 15 greek */
;

#define COUNTRYSTRING_WIDTH 26
#define MAX_NATIONAL_SUBSET (sizeof(countrystring) / COUNTRYSTRING_WIDTH - 1)

enum
{
	NAT_DEFAULT = 0,
	NAT_CZ = 1,
	NAT_UK = 2,
	NAT_ET = 3,
	NAT_FR = 4,
	NAT_DE = 5,
	NAT_IT = 6,
	NAT_LV = 7,
	NAT_PL = 8,
	NAT_SP = 9,
	NAT_RO = 10,
	NAT_SR = 11,
	NAT_SW = 12,
	NAT_TR = 13,
	NAT_MAX_FROM_HEADER = 13,
	NAT_RU = 14,
	NAT_GR = 15
};

const unsigned char countryconversiontable[] = { NAT_UK, NAT_DE, NAT_SW, NAT_IT, NAT_FR, NAT_SP, NAT_CZ, NAT_RO};

/* some data */
char versioninfo[16];
int hotlist[10];
int maxhotlist;

int pig, rc, fb, lcd;
int sx, ex, sy, ey;
int PosX, PosY, StartX, StartY;
int lastpage;
int inputcounter;
int zoommode, screenmode, transpmode, hintmode, boxed, nofirst, savedscreenmode, showflof, show39, showl25, prevscreenmode;
char dumpl25;
int catch_row, catch_col, catched_page, pagecatching;
int prev_100, prev_10, next_10, next_100;
int screen_mode1, screen_mode2, color_mode, trans_mode, national_subset, national_subset_secondary, auto_national, swapupdown, showhex, menulanguage;
int pids_found, current_service, getpidsdone;
int SDT_ready;
int pc_old_row, pc_old_col;     /* for page catching */
int temp_page;	/* for page input */
char saveconfig, hotlistchanged;
signed char clearbbcolor = -1;
int usettf;
short pop, gpop, drcs, gdrcs;
unsigned char tAPx, tAPy;	/* temporary offset to Active Position for objects */
unsigned char axdrcs[12+1+10+1];
#define aydrcs (axdrcs + 12+1)
unsigned char FullRowColor[25];
unsigned char FullScrColor;
tstPageinfo *pageinfo = 0;/* pointer to cached info of last decoded page */
const char * fncmodes[] = {"12", "6"};
const char * saamodes[] = {"4:3_full_format", "16:9_full_format"};

struct timeval tv_delay;
int  subtitledelay, delaystarted;
FILE *conf;

unsigned short RCCode;

struct _pid_table
{
	int  vtxt_pid;
	int  service_id;
	int  service_name_len;
	char service_name[24];
	int  national_subset;
}pid_table[128];

unsigned char restoreaudio = 0;
/* 0 Nokia, 1 Philips, 2 Sagem */
/* typ_vcr/dvb: 	v1 a1 v2 a2 v3 a3 (vcr_only: fblk) */

/* language dependent texts */
#define MAXMENULANGUAGE 8 /* 0 deutsch, 1 englisch, 2 franz?sisch, 3 niederl?ndisch, 4 griechisch, 5 italienisch, 6 polnisch, 7 schwedisch, 8 suomi */
const int menusubset[] =   { NAT_DE   , NAT_UK    , NAT_FR       , NAT_UK          , NAT_GR      , NAT_IT       , NAT_PL    , NAT_SW, NAT_SW };


#define Menu_StartX (StartX + fontwidth*9/2)
#define Menu_StartY (StartY + fontheight)
#define Menu_Height 24
#define Menu_Width 31

const char MenuLine[] =
{
	3,8,11,12,15,17,19,20,21
};

enum
{
	M_HOT=0,
	M_PID,
	M_SC1,
	M_SC2,
	M_COL,
	M_TRA,
	M_AUN,
	M_NAT,
	M_LNG,
	M_Number
};

#define M_Start M_HOT
#define M_MaxDirect M_AUN

const char hotlistpagecolumn[] =	/* last(!) column of page to show in each language */
{
	22, 26, 28, 27, 28, 27, 28, 21, 20
};
const char hotlisttextcolumn[] =
{
	24, 14, 14, 15, 14, 15, 14, 23, 22
};
const char hotlisttext[][2*6] =
{
	{ "dazu entf." },
	{ " add rem. " },
	{ "ajoutenlev" },
	{ "toev.verw." },
	{ "pq|shava_q" },
	{ "agg. elim." },
	{ "dodajkasuj" },
	{ "ny   bort " },
	{ "lis{{pois " }
};

const char configonoff[][2*4] =
{
	{ "ausein" },
	{ "offon " },
	{ "desact" },
	{ "uitaan" },
	{ "emeape" },
	{ "offon " },
	{ "wy}w} " },
	{ "p} av " },
	{ "EI ON " }
};

const char menuatr[Menu_Height*(Menu_Width+1)] =
{
	"0000000000000000000000000000002"
	"0111111111111111111111111111102"
	"0000000000000000000000000000002"
	"3144444444444444444444444444432"
	"3556655555555555555555555555532"
	"3555555555555555555555555555532"
	"3333333333333333333333333333332"
	"3144444444444444444444444444432"
	"3555555555555555555555555555532"
	"3333333333333333333333333333332"
	"3444444444444444444444444444432"
	"3155555555555555555555555555532"
	"3155555555555555555555555555532"
	"3333333333333333333333333333332"
	"3144444444444444444444444444432"
	"3555555555555555555555555555532"
	"3144444444444444444444444444432"
	"3555555555555555555555555555532"
	"3144444444444444444444444444432"
	"3555555555555555555555555555532"
	"3555555555555555555555555555532"
	"3555555555555555555555555555532"
	"3334444444444444444444444443332"
	"2222222222222222222222222222222"
};

const char configmenu[][Menu_Height*(Menu_Width + 1)] =
{
	{
	  /*     0000000000111111111122222222223 */
	  /*     0123456789012345678901234567890 */
		"???????????????????????????????"
		"?     Konfigurationsmen}     ??"
		"???????????????????????????????"
		"?1 Favoriten: Seite 111 dazu ??"
		"?????                        ??"
		"?+-?                         ??"
		"?                            ??"
		"?2     Teletext-Auswahl      ??"
		"??          suchen          ???"
		"?                            ??"
		"?      Bildschirmformat      ??"
		"?3  Standard-Modus 16:9      ??"
		"?4  TextBild-Modus 16:9      ??"
		"?                            ??"
		"?5        Helligkeit         ??"
		"??                          ???"
		"?6       Transparenz         ??"
		"??                          ???"
		"?7  nationaler Zeichensatz   ??"
		"?automatische Erkennung      ??"
		"??    DE    (#$@[\\]^_`{|}~)???"
		"?? Sprache/Language deutsch ???"
		"??   www.tuxtxt.net  x.xx   ???"
		"???????????????????????????????"
	},
	  /*     0000000000111111111122222222223 */
	  /*     0123456789012345678901234567890 */
	{
		"???????????????????????????????"
		"?     Configuration menu     ??"
		"???????????????????????????????"
		"?1 Favorites:  add page 111  ??"
		"?????                        ??"
		"?+-?                         ??"
		"?                            ??"
		"?2     Teletext selection    ??"
		"??          search          ???"
		"?                            ??"
		"?        Screen format       ??"
		"?3 Standard mode 16:9        ??"
		"?4 Text/TV mode  16:9        ??"
		"?                            ??"
		"?5        Brightness         ??"
		"??                          ???"
		"?6       Transparency        ??"
		"??                          ???"
		"?7   national characterset   ??"
		"? automatic recognition      ??"
		"??    DE    (#$@[\\]^_`{|}~)???"
		"?? Sprache/language english ???"
		"??   www.tuxtxt.net  x.xx   ???"
		"???????????????????????????????"
	},
	  /*     0000000000111111111122222222223 */
	  /*     0123456789012345678901234567890 */
	{
		"???????????????????????????????"
		"?    Menu de configuration   ??"
		"???????????????????????????????"
		"?1 Favorites: ajout. page 111??"
		"?????                        ??"
		"?+-?                         ??"
		"?                            ??"
		"?2  Selection de teletext    ??"
		"??        recherche         ???"
		"?                            ??"
		"?      Format de l'#cran     ??"
		"?3 Mode standard 16:9        ??"
		"?4 Texte/TV      16:9        ??"
		"?                            ??"
		"?5          Clarte           ??"
		"??                          ???"
		"?6       Transparence        ??"
		"??                          ???"
		"?7     police nationale      ??"
		"?reconn. automatique         ??"
		"??    DE    (#$@[\\]^_`{|}~)???"
		"?? Sprache/language francais???"
		"??   www.tuxtxt.net  x.xx   ???"
		"???????????????????????????????"
	},
	  /*     0000000000111111111122222222223 */
	  /*     0123456789012345678901234567890 */
	{
		"???????????????????????????????"
		"?      Configuratiemenu      ??"
		"???????????????????????????????"
		"?1 Favorieten: toev. pag 111 ??"
		"?????                        ??"
		"?+-?                         ??"
		"?                            ??"
		"?2     Teletekst-selectie    ??"
		"??          zoeken          ???"
		"?                            ??"
		"?     Beeldschermformaat     ??"
		"?3   Standaardmode 16:9      ??"
		"?4   Tekst/TV mode 16:9      ??"
		"?                            ??"
		"?5        Helderheid         ??"
		"??                          ???"
		"?6       Transparantie       ??"
		"??                          ???"
		"?7    nationale tekenset     ??"
		"?automatische herkenning     ??"
		"??    DE    (#$@[\\]^_`{|}~)???"
		"?? Sprache/Language nederl. ???"
		"??   www.tuxtxt.net  x.xx   ???"
		"???????????????????????????????"
	},
	  /*     0000000000111111111122222222223 */
	  /*     0123456789012345678901234567890 */
	{
		"???????????????????????????????"
		"?      Lemo} quhl_seym       ??"
		"???????????????????????????????"
		"?1 Vaboq_:    pqo_h. sek. 111??"
		"?????                        ??"
		"?+-?                         ??"
		"?                            ??"
		"?2     Epikoc^ Teket]nt      ??"
		"??        Amaf^tgsg         ???"
		"?                            ??"
		"?       Loqv^ oh|mgr         ??"
		"?3 Tq|por pq|tupor   16:9    ??"
		"?4 Tq|por eij. jeil. 16:9    ??"
		"?                            ??"
		"?5       Kalpq|tgta          ??"
		"??                          ???"
		"?6       Diav\\meia          ??"
		"??                          ???"
		"?7    Ehmij^ tuposeiq\\      ??"
		"?aut|latg amacm~qisg         ??"
		"??    DE    (#$@[\\]^_`{|}~)???"
		"?? Ck~ssa/Language ekkgmij\\???"
		"??   www.tuxtxt.net  x.xx   ???"
		"???????????????????????????????"
	},
	  /*     0000000000111111111122222222223 */
	  /*     0123456789012345678901234567890 */
	{
		"???????????????????????????????"
		"?   Menu di configurazione   ??"
		"???????????????????????????????"
		"?1  Preferiti:  agg. pag.111 ??"
		"?????                        ??"
		"?+-?                         ??"
		"?                            ??"
		"?2   Selezione televideo     ??"
		"??         ricerca          ???"
		"?                            ??"
		"?      Formato schermo       ??"
		"?3  Modo standard 16:9       ??"
		"?4  Text/Mod.TV 16:9         ??"
		"?                            ??"
		"?5        Luminosit{         ??"
		"??                          ???"
		"?6        Trasparenza        ??"
		"??                          ???"
		"?7   nazionalita'caratteri   ??"
		"? riconoscimento automatico  ??"
		"??    DE    (#$@[\\]^_`{|}~)???"
		"?? Lingua/Language Italiana ???"
		"??   www.tuxtxt.net  x.xx   ???"
		"???????????????????????????????"
	},
	  /*     0000000000111111111122222222223 */
	  /*     0123456789012345678901234567890 */
	{
		"???????????????????????????????"
		"?        Konfiguracja        ??"
		"???????????????????????????????"
		"?1 Ulubione : kasuj  str. 111??"
		"?????                        ??"
		"?+-?                         ??"
		"?                            ??"
		"?2     Wyb_r telegazety      ??"
		"??          szukaj          ???"
		"?                            ??"
		"?       Format obrazu        ??"
		"?3 Tryb standard 16:9        ??"
		"?4 Telegazeta/TV 16:9        ??"
		"?                            ??"
		"?5          Jasno|^          ??"
		"??                          ???"
		"?6      Prze~roczysto|^      ??"
		"??                          ???"
		"?7 Znaki charakterystyczne   ??"
		"? automatyczne rozpozn.      ??"
		"??    DE    (#$@[\\]^_`{|}~)???"
		"??  J`zyk/Language   polski ???"
		"??   www.tuxtxt.net  x.xx   ???"
		"???????????????????????????????"
	},
	  /*     0000000000111111111122222222223 */
	  /*     0123456789012345678901234567890 */
	{
		"???????????????????????????????"
		"?     Konfigurationsmeny     ??"
		"???????????????????????????????"
		"?1 Favoriter: sida 111 ny    ??"
		"?????                        ??"
		"?+-?                         ??"
		"?                            ??"
		"?2      TextTV v{ljaren      ??"
		"??            s|k           ???"
		"?                            ??"
		"?        TV- format          ??"
		"?3 Standard l{ge 16:9        ??"
		"?4 Text/Bild l{ge  16:9      ??"
		"?                            ??"
		"?5        Ljusstyrka         ??"
		"??                          ???"
		"?6     Genomskinlighet       ??"
		"??                          ???"
		"?7nationell teckenupps{ttning??"
		"? automatisk igenk{nning     ??"
		"??    DE    (#$@[\\]^_`{|}~)???"
		"?? Sprache/language svenska ???"
		"??   www.tuxtxt.net  x.xx   ???"
		"???????????????????????????????"
	},
	  /*     0000000000111111111122222222223 */
	  /*     0123456789012345678901234567890 */
	{
		"???????????????????????????????"
		"?        Asetusvalikko       ??"
		"???????????????????????????????"
		"?1 Suosikit: sivu 111 lis{{  ??"
		"?????                        ??"
		"?+-?                         ??"
		"?                            ??"
		"?2   Tekstikanavan valinta   ??"
		"??          search          ???"
		"?                            ??"
		"?         N{ytt|tila         ??"
		"?3 Vakiotila     16:9        ??"
		"?4 Teksti/TV     16:9        ??"
		"?                            ??"
		"?5         Kirkkaus          ??"
		"??                          ???"
		"?6       L{pin{kyvyys        ??"
		"??                          ???"
		"?7   kansallinen merkist|    ??"
		"? automaattinen tunnistus    ??"
		"??    DE    (#$@[\\]^_`{|}~)???"
		"?? Kieli            suomi   ???"
		"??   www.tuxtxt.net  x.xx   ???"
		"???????????????????????????????"
	}
};

const char catchmenutext[][81] =
{
	{ "        ???? w{hlen   ?? anzeigen       "
	  "0000000011110000000000110000000000000000" },
	{ "        ???? select   ?? show           "
	  "0000000011110000000000110000000000000000" },
	{ "  ???? selectionner   ?? montrer        "
	  "0011110000000000000000110000000000000000" },
	{ "        ???? kiezen   ?? tonen          "
	  "0000000011110000000000110000000000000000" },
	{ "        ???? epikoc^  ?? pqobok^        "
	  "0000000011110000000000110000000000000000" },
	{ "        ????seleziona ?? mostra         "
	  "0000000011110000000000110000000000000000" },
	{ "        ???? wybiez   ?? wyswietl       "
	  "0000000011110000000000110000000000000000" },
	{ "        ???? v{lj     ?? visa           "
     "0000000011110000000000110000000000000000" },
	{ "        ???? valitse  ?? n{yt{          "
	  "0000000011110000000000110000000000000000" }
};

const char message_3[][39] =
{
	{ "?   suche nach Teletext-Anbietern   ??" },
	{ "?  searching for teletext Services  ??" },
	{ "?  recherche des services teletext  ??" },
	{ "? zoeken naar teletekst aanbieders  ??" },
	{ "?     amaf^tgsg voq]ym Teket]nt     ??" },
	{ "?     attesa opzioni televideo      ??" },
	{ "?  poszukiwanie sygna}u telegazety  ??" },
	{ "?    s|ker efter TextTV tj{nster    ??" },
	{ "?   etsit{{n Teksti-TV -palvelua    ??" }
};

const char message_3_blank[] = "?                                   ??";

const char message_7[][39] =
{
	{ "? kein Teletext auf dem Transponder ??" },
	{ "?   no teletext on the transponder  ??" },
	{ "? pas de teletext sur le transponder??" },
	{ "? geen teletekst op de transponder  ??" },
	{ "? jal]la Teket]nt ston amaletadot^  ??" },
	{ "? nessun televideo sul trasponder   ??" },
	{ "?   brak sygna}u na transponderze   ??" },
	{ "? ingen TextTV p} denna transponder ??" },
	{ "?    Ei Teksti-TV:t{ l{hettimell{   ??" }
};

const char message_8[][39] =
{
     /*    00000000001111111111222222222233333333334 */
     /*    01234567890123456789012345678901234567890 */
	{ "?  warte auf Empfang von Seite 100  ??" },
	{ "? waiting for reception of page 100 ??" },
	{ "? attentre la r?ception de page 100 ??" },
	{ "?wachten op ontvangst van pagina 100??" },
	{ "?     amal]my k^xg sek_dar 100      ??" },
	{ "?   attesa ricezione pagina 100     ??" },
	{ "?     oczekiwanie na stron` 100     ??" },
	{ "?  v{ntar p} mottagning av sida 100 ??" },
	{ "?        Odotetaan sivua 100        ??" }
};

const char message8pagecolumn[] = /* last(!) column of page to show in each language */
{
	33, 34, 34, 35, 29, 30, 30, 34, 34
};

enum /* options for charset */
{
	C_G0P = 0, /* primary G0 */
	C_G0S, /* secondary G0 */
	C_G1C, /* G1 contiguous */
	C_G1S, /* G1 separate */
	C_G2,
	C_G3,
	C_OFFSET_DRCS = 32
	/* 32..47: 32+subpage# GDRCS (offset/20 in page_char) */
	/* 48..63: 48+subpage#  DRCS (offset/20 in page_char) */
};

/* struct for page attributes */
typedef struct
{
	unsigned char fg      :6; /* foreground color */
	unsigned char bg      :6; /* background color */
	unsigned char charset :6; /* see enum above */
	unsigned char doubleh :1; /* double height */
	unsigned char doublew :1; /* double width */
	/* ignore at Black Background Color Substitution */
	/* black background set by New Background ($1d) instead of start-of-row default or Black Backgr. ($1c) */
	/* or black background set by level 2.5 extensions */
	unsigned char IgnoreAtBlackBgSubst:1;
	unsigned char concealed:1; /* concealed information */
	unsigned char inverted :1; /* colors inverted */
	unsigned char flashing :5; /* flash mode */
	unsigned char diacrit  :4; /* diacritical mark */
	unsigned char underline:1; /* Text underlined */
	unsigned char boxwin   :1; /* Text boxed/windowed */
	unsigned char setX26   :1; /* Char is set by packet X/26 (no national subset used) */
	unsigned char setG0G2  :7; /* G0+G2 set designation  */
} tstPageAttr;

enum /* indices in atrtable */
{
	ATR_WB, /* white on black */
	ATR_PassiveDefault, /* Default for passive objects: white on black, ignore at Black Background Color Substitution */
	ATR_L250, /* line25 */
	ATR_L251, /* line25 */
	ATR_L252, /* line25 */
	ATR_L253, /* line25 */
	ATR_TOPMENU0, /* topmenu */
	ATR_TOPMENU1, /* topmenu */
	ATR_TOPMENU2, /* topmenu */
	ATR_TOPMENU3, /* topmenu */
	ATR_MSG0, /* message */
	ATR_MSG1, /* message */
	ATR_MSG2, /* message */
	ATR_MSG3, /* message */
	ATR_MSGDRM0, /* message */
	ATR_MSGDRM1, /* message */
	ATR_MSGDRM2, /* message */
	ATR_MSGDRM3, /* message */
	ATR_MENUHIL0, /* hilit menu line */
	ATR_MENUHIL1, /* hilit menu line */
	ATR_MENUHIL2, /* hilit menu line */
	ATR_MENU0, /* menu line */
	ATR_MENU1, /* menu line */
	ATR_MENU2, /* menu line */
	ATR_MENU3, /* menu line */
	ATR_MENU4, /* menu line */
	ATR_MENU5, /* menu line */
	ATR_MENU6, /* menu line */
	ATR_CATCHMENU0, /* catch menu line */
	ATR_CATCHMENU1 /* catch menu line */
};

/* define color names */
enum
{
	black = 0,
	red, /* 1 */
	green, /* 2 */
	yellow, /* 3 */
	blue,	/* 4 */
	magenta,	/* 5 */
	cyan,	/* 6 */
	white, /* 7 */
	menu1 = (4*8),
	menu2,
	menu3,
	transp,
	transp2,
	SIZECOLTABLE
};

//const (avoid warnings :<)
tstPageAttr atrtable[] =
{
	{ white  , black , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_WB */
	{ white  , black , C_G0P, 0, 0, 1 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_PassiveDefault */
	{ white  , red   , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_L250 */
	{ black  , green , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_L251 */
	{ black  , yellow, C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_L252 */
	{ white  , blue  , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_L253 */
	{ magenta, black , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_TOPMENU0 */
	{ green  , black , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_TOPMENU1 */
	{ yellow , black , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_TOPMENU2 */
	{ cyan   , black , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_TOPMENU3 */
	{ menu2  , menu3 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MSG0 */
	{ yellow , menu3 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MSG1 */
	{ menu2  , transp, C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MSG2 */
	{ white  , menu3 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MSG3 */
	{ menu2  , menu1 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MSGDRM0 */
	{ yellow , menu1 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MSGDRM1 */
	{ menu2  , black , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MSGDRM2 */
	{ white  , menu1 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MSGDRM3 */
	{ menu1  , blue  , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MENUHIL0 5a Z */
	{ white  , blue  , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MENUHIL1 58 X */
	{ menu2  , transp, C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MENUHIL2 9b ? */
	{ menu2  , menu1 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MENU0 ab ? */
	{ yellow , menu1 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MENU1 a4 ? */
	{ menu2  , transp, C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MENU2 9b ? */
	{ menu2  , menu3 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MENU3 cb ? */
	{ cyan   , menu3 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MENU4 c7 ? */
	{ white  , menu3 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MENU5 c8 ? */
	{ white  , menu1 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MENU6 a8 ? */
	{ yellow , menu1 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_CATCHMENU0 a4 ? */
	{ white  , menu1 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}  /* ATR_CATCHMENU1 a8 ? */
};
/* buffers */
unsigned char  lcd_backbuffer[120*64 / 8];
unsigned char  page_char[40 * 25];
tstPageAttr page_atrb[40 * 25];

//unsigned short page_atrb[40 * 25]; /*  ?????:h:cc:bbbb:ffff -> ?=reserved, h=double height, c=charset (0:G0 / 1:G1c / 2:G1s), b=background, f=foreground */

/* colormap */
const unsigned short defaultcolors[] =	/* 0x0bgr */
{
	0x000, 0x00f, 0x0f0, 0x0ff, 0xf00, 0xf0f, 0xff0, 0xfff,
	0x000, 0x007, 0x070, 0x077, 0x700, 0x707, 0x770, 0x777,
	0x50f, 0x07f, 0x7f0, 0xbff, 0xac0, 0x005, 0x256, 0x77c,
	0x333, 0x77f, 0x7f7, 0x7ff, 0xf77, 0xf7f, 0xff7, 0xddd,
	0x420, 0x210, 0x420, 0x000, 0x000
};

/* 32bit colortable */
unsigned char bgra[][5] = { 
"\0\0\0\xFF", "\0\0\0\xFF", "\0\0\0\xFF", "\0\0\0\xFF",
"\0\0\0\xFF", "\0\0\0\xFF", "\0\0\0\xFF", "\0\0\0\xFF",
"\0\0\0\xFF", "\0\0\0\xFF", "\0\0\0\xFF", "\0\0\0\xFF",
"\0\0\0\xFF", "\0\0\0\xFF", "\0\0\0\xFF", "\0\0\0\xFF",
"\0\0\0\xFF", "\0\0\0\xFF", "\0\0\0\xFF", "\0\0\0\xFF",
"\0\0\0\xFF", "\0\0\0\xFF", "\0\0\0\xFF", "\0\0\0\xFF",
"\0\0\0\xFF", "\0\0\0\xFF", "\0\0\0\xFF", "\0\0\0\xFF",
"\0\0\0\xFF", "\0\0\0\xFF", "\0\0\0\xFF", "\0\0\0\xFF",
"\0\0\0\xFF", "\0\0\0\xFF", "\0\0\0\xC0", "\0\0\0\x00",
"\0\0\0\x33" };

/* old 8bit color table */
unsigned short rd0[] = {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0x00<<8, 0x00<<8, 0x00<<8, 0,      0      };
unsigned short gn0[] = {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0x20<<8, 0x10<<8, 0x20<<8, 0,      0      };
unsigned short bl0[] = {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0x40<<8, 0x20<<8, 0x40<<8, 0,      0      };
unsigned short tr0[] = {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0x0000 , 0x0000 , 0x0A00 , 0xFFFF, 0x3000 };

struct fb_cmap colormap_0 = {0, SIZECOLTABLE, rd0, gn0, bl0, tr0};

/* tables for color table remapping, first entry (no remapping) skipped, offsets for color index */
const unsigned char MapTblFG[] = {  0,  0,  8,  8, 16, 16, 16 };
const unsigned char MapTblBG[] = {  8, 16,  8, 16,  8, 16, 24 };


/* shapes */
enum
{
	S_END = 0,
	S_FHL, /* full horizontal line: y-offset */
	S_FVL, /* full vertical line: x-offset */
	S_BOX, /* rectangle: x-offset, y-offset, width, height */
	S_TRA, /* trapez: x0, y0, l0, x1, y1, l1 */
	S_BTR, /* trapez in bgcolor: x0, y0, l0, x1, y1, l1 */
	S_INV, /* invert */
	S_LNK, /* call other shape: shapenumber */
	S_CHR, /* Character from freetype hibyte, lowbyte */
	S_ADT, /* Character 2F alternating raster */
	S_FLH, /* flip horizontal */
	S_FLV  /* flip vertical */
};

/* shape coordinates */
enum
{
	S_W13 = 5, /* width*1/3 */
	S_W12, /* width*1/2 */
	S_W23, /* width*2/3 */
	S_W11, /* width */
	S_WM3, /* width-3 */
	S_H13, /* height*1/3 */
	S_H12, /* height*1/2 */
	S_H23, /* height*2/3 */
	S_H11, /* height */
	S_NrShCoord
};

/* G3 characters */
unsigned char aG3_20[] = { S_TRA, 0, S_H23, 1, 0, S_H11, S_W12, S_END };
unsigned char aG3_21[] = { S_TRA, 0, S_H23, 1, 0, S_H11, S_W11, S_END };
unsigned char aG3_22[] = { S_TRA, 0, S_H12, 1, 0, S_H11, S_W12, S_END };
unsigned char aG3_23[] = { S_TRA, 0, S_H12, 1, 0, S_H11, S_W11, S_END };
unsigned char aG3_24[] = { S_TRA, 0, 0, 1, 0, S_H11, S_W12, S_END };
unsigned char aG3_25[] = { S_TRA, 0, 0, 1, 0, S_H11, S_W11, S_END };
unsigned char aG3_26[] = { S_INV, S_LNK, 0x66, S_END };
unsigned char aG3_27[] = { S_INV, S_LNK, 0x67, S_END };
unsigned char aG3_28[] = { S_INV, S_LNK, 0x68, S_END };
unsigned char aG3_29[] = { S_INV, S_LNK, 0x69, S_END };
unsigned char aG3_2a[] = { S_INV, S_LNK, 0x6a, S_END };
unsigned char aG3_2b[] = { S_INV, S_LNK, 0x6b, S_END };
unsigned char aG3_2c[] = { S_INV, S_LNK, 0x6c, S_END };
unsigned char aG3_2d[] = { S_INV, S_LNK, 0x6d, S_END };
unsigned char aG3_2e[] = { S_BOX, 2, 0, 3, S_H11, S_END };
unsigned char aG3_2f[] = { S_ADT };
unsigned char aG3_30[] = { S_LNK, 0x20, S_FLH, S_END };
unsigned char aG3_31[] = { S_LNK, 0x21, S_FLH, S_END };
unsigned char aG3_32[] = { S_LNK, 0x22, S_FLH, S_END };
unsigned char aG3_33[] = { S_LNK, 0x23, S_FLH, S_END };
unsigned char aG3_34[] = { S_LNK, 0x24, S_FLH, S_END };
unsigned char aG3_35[] = { S_LNK, 0x25, S_FLH, S_END };
unsigned char aG3_36[] = { S_INV, S_LNK, 0x76, S_END };
unsigned char aG3_37[] = { S_INV, S_LNK, 0x77, S_END };
unsigned char aG3_38[] = { S_INV, S_LNK, 0x78, S_END };
unsigned char aG3_39[] = { S_INV, S_LNK, 0x79, S_END };
unsigned char aG3_3a[] = { S_INV, S_LNK, 0x7a, S_END };
unsigned char aG3_3b[] = { S_INV, S_LNK, 0x7b, S_END };
unsigned char aG3_3c[] = { S_INV, S_LNK, 0x7c, S_END };
unsigned char aG3_3d[] = { S_INV, S_LNK, 0x7d, S_END };
unsigned char aG3_3e[] = { S_LNK, 0x2e, S_FLH, S_END };
unsigned char aG3_3f[] = { S_BOX, 0, 0, S_W11, S_H11, S_END };
unsigned char aG3_40[] = { S_BOX, 0, S_H13, S_W11, S_H13, S_LNK, 0x7e, S_END };
unsigned char aG3_41[] = { S_BOX, 0, S_H13, S_W11, S_H13, S_LNK, 0x7e, S_FLV, S_END };
unsigned char aG3_42[] = { S_LNK, 0x50, S_BOX, S_W12, S_H13, S_W12, S_H13, S_END };
unsigned char aG3_43[] = { S_LNK, 0x50, S_BOX, 0, S_H13, S_W12, S_H13, S_END };
unsigned char aG3_44[] = { S_LNK, 0x48, S_FLV, S_LNK, 0x48, S_END };
unsigned char aG3_45[] = { S_LNK, 0x44, S_FLH, S_END };
unsigned char aG3_46[] = { S_LNK, 0x47, S_FLV, S_END };
unsigned char aG3_47[] = { S_LNK, 0x48, S_FLH, S_LNK, 0x48, S_END };
unsigned char aG3_48[] = { S_TRA, 0, 0, S_W23, 0, S_H23, 0, S_BTR, 0, 0, S_W13, 0, S_H13, 0, S_END };
unsigned char aG3_49[] = { S_LNK, 0x48, S_FLH, S_END };
unsigned char aG3_4a[] = { S_LNK, 0x48, S_FLV, S_END };
unsigned char aG3_4b[] = { S_LNK, 0x48, S_FLH, S_FLV, S_END };
unsigned char aG3_4c[] = { S_LNK, 0x50, S_BOX, 0, S_H13, S_W11, S_H13, S_END };
unsigned char aG3_4d[] = { S_CHR, 0x25, 0xE6 };
unsigned char aG3_4e[] = { S_CHR, 0x25, 0xCF };
unsigned char aG3_4f[] = { S_CHR, 0x25, 0xCB };
unsigned char aG3_50[] = { S_BOX, S_W12, 0, 2, S_H11, S_FLH, S_BOX, S_W12, 0, 2, S_H11,S_END };
unsigned char aG3_51[] = { S_BOX, 0, S_H12, S_W11, 2, S_FLV, S_BOX, 0, S_H12, S_W11, 2,S_END };
unsigned char aG3_52[] = { S_LNK, 0x55, S_FLH, S_FLV, S_END };
unsigned char aG3_53[] = { S_LNK, 0x55, S_FLV, S_END };
unsigned char aG3_54[] = { S_LNK, 0x55, S_FLH, S_END };
unsigned char aG3_55[] = { S_LNK, 0x7e, S_FLV, S_BOX, 0, S_H12, S_W12, 2, S_FLV, S_BOX, 0, S_H12, S_W12, 2, S_END };
unsigned char aG3_56[] = { S_LNK, 0x57, S_FLH, S_END};
unsigned char aG3_57[] = { S_LNK, 0x55, S_LNK, 0x50 , S_END};
unsigned char aG3_58[] = { S_LNK, 0x59, S_FLV, S_END};
unsigned char aG3_59[] = { S_LNK, 0x7e, S_LNK, 0x51 , S_END};
unsigned char aG3_5a[] = { S_LNK, 0x50, S_LNK, 0x51 , S_END};
unsigned char aG3_5b[] = { S_CHR, 0x21, 0x92};
unsigned char aG3_5c[] = { S_CHR, 0x21, 0x90};
unsigned char aG3_5d[] = { S_CHR, 0x21, 0x91};
unsigned char aG3_5e[] = { S_CHR, 0x21, 0x93};
unsigned char aG3_5f[] = { S_CHR, 0x00, 0x20};
unsigned char aG3_60[] = { S_INV, S_LNK, 0x20, S_END };
unsigned char aG3_61[] = { S_INV, S_LNK, 0x21, S_END };
unsigned char aG3_62[] = { S_INV, S_LNK, 0x22, S_END };
unsigned char aG3_63[] = { S_INV, S_LNK, 0x23, S_END };
unsigned char aG3_64[] = { S_INV, S_LNK, 0x24, S_END };
unsigned char aG3_65[] = { S_INV, S_LNK, 0x25, S_END };
unsigned char aG3_66[] = { S_LNK, 0x20, S_FLV, S_END };
unsigned char aG3_67[] = { S_LNK, 0x21, S_FLV, S_END };
unsigned char aG3_68[] = { S_LNK, 0x22, S_FLV, S_END };
unsigned char aG3_69[] = { S_LNK, 0x23, S_FLV, S_END };
unsigned char aG3_6a[] = { S_LNK, 0x24, S_FLV, S_END };
unsigned char aG3_6b[] = { S_BOX, 0, 0, S_W11, S_H13, S_TRA, 0, S_H13, S_W11, 0, S_H23, 1, S_END };
unsigned char aG3_6c[] = { S_TRA, 0, 0, 1, 0, S_H12, S_W12, S_FLV, S_TRA, 0, 0, 1, 0, S_H12, S_W12, S_BOX, 0, S_H12, S_W12,1, S_END };
unsigned char aG3_6d[] = { S_TRA, 0, 0, S_W12, S_W12, S_H12, 0, S_FLH, S_TRA, 0, 0, S_W12, S_W12, S_H12, 0, S_END };
unsigned char aG3_6e[] = { S_CHR, 0x00, 0x20};
unsigned char aG3_6f[] = { S_CHR, 0x00, 0x20};
unsigned char aG3_70[] = { S_INV, S_LNK, 0x30, S_END };
unsigned char aG3_71[] = { S_INV, S_LNK, 0x31, S_END };
unsigned char aG3_72[] = { S_INV, S_LNK, 0x32, S_END };
unsigned char aG3_73[] = { S_INV, S_LNK, 0x33, S_END };
unsigned char aG3_74[] = { S_INV, S_LNK, 0x34, S_END };
unsigned char aG3_75[] = { S_INV, S_LNK, 0x35, S_END };
unsigned char aG3_76[] = { S_LNK, 0x66, S_FLH, S_END };
unsigned char aG3_77[] = { S_LNK, 0x67, S_FLH, S_END };
unsigned char aG3_78[] = { S_LNK, 0x68, S_FLH, S_END };
unsigned char aG3_79[] = { S_LNK, 0x69, S_FLH, S_END };
unsigned char aG3_7a[] = { S_LNK, 0x6a, S_FLH, S_END };
unsigned char aG3_7b[] = { S_LNK, 0x6b, S_FLH, S_END };
unsigned char aG3_7c[] = { S_LNK, 0x6c, S_FLH, S_END };
unsigned char aG3_7d[] = { S_LNK, 0x6d, S_FLV, S_END };
unsigned char aG3_7e[] = { S_BOX, S_W12, 0, 2, S_H12, S_FLH, S_BOX, S_W12, 0, 2, S_H12, S_END };// help char, not printed directly (only by S_LNK)

unsigned char *aShapes[] =
{
	aG3_20, aG3_21, aG3_22, aG3_23, aG3_24, aG3_25, aG3_26, aG3_27, aG3_28, aG3_29, aG3_2a, aG3_2b, aG3_2c, aG3_2d, aG3_2e, aG3_2f,
	aG3_30, aG3_31, aG3_32, aG3_33, aG3_34, aG3_35, aG3_36, aG3_37, aG3_38, aG3_39, aG3_3a, aG3_3b, aG3_3c, aG3_3d, aG3_3e, aG3_3f,
	aG3_40, aG3_41, aG3_42, aG3_43, aG3_44, aG3_45, aG3_46, aG3_47, aG3_48, aG3_49, aG3_4a, aG3_4b, aG3_4c, aG3_4d, aG3_4e, aG3_4f,
	aG3_50, aG3_51, aG3_52, aG3_53, aG3_54, aG3_55, aG3_56, aG3_57, aG3_58, aG3_59, aG3_5a, aG3_5b, aG3_5c, aG3_5d, aG3_5e, aG3_5f,
	aG3_60, aG3_61, aG3_62, aG3_63, aG3_64, aG3_65, aG3_66, aG3_67, aG3_68, aG3_69, aG3_6a, aG3_6b, aG3_6c, aG3_6d, aG3_6e, aG3_6f,
	aG3_70, aG3_71, aG3_72, aG3_73, aG3_74, aG3_75, aG3_76, aG3_77, aG3_78, aG3_79, aG3_7a, aG3_7b, aG3_7c, aG3_7d, aG3_7e
};




/* lcd layout */
const char lcd_layout[] =
{
#define ____ 0x0
#define ___X 0x1
#define __X_ 0x2
#define __XX 0x3
#define _X__ 0x4
#define _X_X 0x5
#define _XX_ 0x6
#define _XXX 0x7
#define X___ 0x8
#define X__X 0x9
#define X_X_ 0xA
#define X_XX 0xB
#define XX__ 0xC
#define XX_X 0xD
#define XXX_ 0xE
#define XXXX 0xF

#define i <<4|

	____ i _XXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXX_ i ____,
	___X i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i X___,
	__XX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XX__ i ____,XXX_ i _X__,_XXX i __X_,__XX i X___,___X i XX__,X___ i XXX_,____ i _XXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XX__,
	_XXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,X_XX i XXXX,_X_X i X_XX,X_X_ i XX_X,XX_X i _XXX,XXX_ i X_XX,_XXX i _X_X,XXXX i X_XX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXX_,
	_XXX i XXXX,X___ i ____,____ i ____,____ i __XX,X_XX i XXXX,_X_X i X_XX,X_X_ i XX_X,XX_X i _XXX,XXX_ i X_XX,_XXX i _X_X,XXXX i X_XX,X___ i ____,____ i ____,____ i ___X,XXXX i XXX_,
	XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,X_XX i XXXX,_X_X i X_XX,X_X_ i XX_X,XX_X i _XXX,XXX_ i X_XX,_XXX i _X_X,XXXX i X_XX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,
	XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XX__ i XX__,XX_X i X_XX,X_X_ i XXXX,XX_X i X__X,X__X i X_XX,XXXX i _XX_,_XX_ i _XXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,
	XXXX i XX__,____ i ____,____ i ____,____ i __XX,XXX_ i XX_X,XX_X i X_XX,X_XX i _XXX,__XX i XX_X,X_XX i XX_X,XX__ i XXXX,_XX_ i XXXX,X___ i ____,____ i ____,____ i ____,__XX i XXXX,
	XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXX_ i XX_X,XX_X i X_XX,X_X_ i XXXX,XX_X i XX_X,X_XX i X_XX,XXXX i _XXX,_XX_ i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,
	XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXX_ i XX_X,XX_X i X_XX,X_X_ i XX_X,XX_X i XX_X,X_XX i X_XX,_XXX i _XXX,_XX_ i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,
	XXX_ i ____,____ i ____,____ i ____,____ i __XX,XXX_ i XX_X,XX_X i XXXX,X_X_ i XX_X,XX_X i XX_X,X_XX i X_XX,_XXX i _XXX,_XX_ i XXXX,X___ i ____,____ i ____,____ i ____,____ i _XXX,
	XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXX_ i XX_X,XX_X i XXXX,X_X_ i XX_X,XX_X i XX_X,X_XX i X_XX,_XXX i _XXX,_XX_ i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,
	XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i __XX,XXX_ i ____,_XXX i __X_,__XX i XXX_,_XXX i XX__,X___ i XXXX,X__X i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,
	XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,
	XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i XX__,_XXX i XXX_,__XX i ___X,XXXX i X___,XX__ i _XXX,XXX_ i __XX,____ i ___X,X___ i XXXX,XX__ i _XX_,__XX i XXXX,___X i XXXX,X___ i XX_X,XX__ i _XXX,XXX_ i __XX,XXXX i ___X,
	X__X i __X_,X___ i ___X,_X__ i X_X_,____ i _X_X,__X_ i X___,___X i _X__,X___ i __X_,_X_X i ____,__X_ i X__X,_X__ i ____,X_X_ i ____,_X_X i __X_,__X_ i X___,___X i _X__,____ i X__X,
	X__X i __X_,X___ i ___X,_X__ i X_X_,____ i _X_X,__X_ i X___,___X i _X__,X___ i __X_,_X_X i ____,__X_ i X__X,_X__ i ____,X_X_ i ____,_X_X i __X_,__X_ i X___,___X i _X__,____ i X__X,
	X__X i __X_,X___ i ___X,_X__ i X_X_,____ i _X_X,__X_ i X___,___X i _X__,X___ i __X_,_X_X i ____,__X_ i X__X,_X__ i ____,X_X_ i ____,_X_X i __X_,__X_ i X___,___X i _X__,____ i X__X,
	X__X i __X_,X__X i ___X,_X__ i X__X,X__X i X__X,__X_ i X__X,___X i _X__,X___ i __X_,_X_X i __XX,XX__ i X__X,_X__ i XXXX,__X_ i _X__,_X_X i __X_,__X_ i X__X,___X i _X__,XXXX i ___X,
	X__X i __X_,X__X i ___X,_X__ i X___,X__X i ___X,__X_ i X___,___X i _X__,X___ i __X_,_X_X i ____,__X_ i X__X,_X__ i ____,X_X_ i ___X,X__X i __X_,__X_ i X__X,___X i _X__,X___ i X__X,
	X__X i __X_,X__X i ___X,_X__ i X___,X__X i ___X,__X_ i X___,___X i _X__,X___ i __X_,_X_X i ____,__X_ i X__X,_X__ i ____,X_X_ i ____,_X_X i __X_,__X_ i X__X,___X i _X__,X___ i X__X,
	X__X i __X_,X__X i ___X,_X__ i X___,X__X i ___X,__X_ i X__X,___X i _X__,XXXX i __X_,_X__ i XXX_,__X_ i X__X,_X__ i XXXX,__X_ i _X__,_X_X i __X_,__X_ i X__X,___X i _X__,X___ i X__X,
	X__X i __X_,X__X i ___X,_X__ i X___,X__X i ___X,__X_ i X__X,___X i _X__,____ i X_X_,_X_X i ____,__X_ i X__X,_X__ i ____,X_X_ i _X__,_X_X i ____,__X_ i X__X,___X i _X__,____ i X__X,
	X__X i __X_,X__X i ___X,_X__ i X___,X__X i ___X,__X_ i X__X,___X i _X__,____ i X_X_,_X_X i ____,__X_ i X__X,_X__ i ____,X_X_ i _X__,_X_X i ____,__X_ i X__X,___X i _X__,____ i X__X,
	X___ i XX__,XXX_ i XXXX,__XX i ____,_XX_ i ____,XX__ i _XX_,XXX_ i __XX,XXXX i ___X,X___ i XXXX,XX__ i _XX_,__XX i XXXX,___X i X_XX,X___ i XXXX,XX__ i _XX_,XXX_ i __XX,XXXX i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,XXXX i XXXX,X___ i XXXX,XX__ i _XXX,XXXX i XX__,_XXX i XXX_,__XX i XXXX,___X i XXXX,X___ i ____,__XX i XXXX,___X i X___,XXXX i XX__,_XXX i XXX_,__XX i XXXX,____ i ___X,
	X___ i ___X,____ i ____,_X_X i ____,__X_ i X___,____ i __X_,X___ i ___X,_X__ i ____,X_X_ i ____,_X__ i ____,_X__ i ____,X_X_ i _X_X,____ i __X_,X___ i ___X,_X__ i ____,X___ i ___X,
	X___ i ___X,____ i ____,_X_X i ____,__X_ i X___,____ i __X_,X___ i ___X,_X__ i ____,X_X_ i ____,_X__ i ____,_X__ i ____,X_X_ i _X_X,____ i __X_,X___ i ___X,_X__ i ____,X___ i ___X,
	X___ i ___X,____ i ____,_X_X i ____,__X_ i X___,____ i __X_,X___ i ___X,_X__ i ____,X_X_ i ____,_X__ i ____,_X__ i XX__,X_X_ i _X_X,____ i __X_,X___ i ___X,_X__ i ____,X___ i ___X,
	X___ i ___X,__X_ i __X_,_X_X i __X_,__X_ i X__X,___X i __X_,X__X i XXX_,_X__ i X___,X__X i X__X,X___ i ____,_X__ i ____,X_X_ i _X__,XX__ i XX__,_XX_ i _XX_,_X__ i XXXX,____ i ___X,
	X___ i ___X,__X_ i __X_,_X_X i __X_,__X_ i X__X,___X i __X_,X___ i ___X,_X__ i X___,X___ i X__X,____ i ____,_X__ i __XX,__X_ i _X__,_X__ i X___,__X_ i _X__,_X__ i ____,X___ i ___X,
	X___ i ___X,__X_ i __X_,_X_X i __X_,__X_ i X__X,___X i __X_,X___ i ___X,_X__ i X___,X___ i X__X,____ i ____,_X__ i ____,X_X_ i _X__,_X__ i X___,__X_ i _X__,_X__ i ____,X___ i ___X,
	X___ i ___X,__X_ i __X_,_X_X i __X_,__X_ i X__X,___X i __X_,X__X i XXX_,_X__ i X___,X___ i X__X,____ i ____,_X__ i XX__,X_X_ i _X__,_X__ i X___,__X_ i _X__,_X__ i XXXX,____ i ___X,
	X___ i ___X,__X_ i __X_,_X_X i ____,__X_ i X__X,___X i __X_,X___ i ___X,_X__ i X___,X___ i X__X,____ i ____,_X__ i ____,X_X_ i _X__,_X__ i X___,__X_ i _X__,_X__ i ____,X___ i ___X,
	X___ i ___X,__X_ i __X_,_X_X i ____,__X_ i X__X,___X i __X_,X___ i ___X,_X__ i X___,X___ i X__X,____ i ____,_X__ i ____,X_X_ i _X__,_X__ i X___,__X_ i _X__,_X__ i ____,X___ i ___X,
	X___ i ____,XX_X i XX_X,X___ i XXXX,XX__ i _XX_,XXX_ i XX__,_XXX i XXX_,__XX i _XXX,____ i _XX_,____ i ____,__XX i XXXX,___X i X___,__XX i ____,___X i X___,__XX i XXXX,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	_X__ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i __X_,
	_X__ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i __X_,
	__X_ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i _X__,
	___X i X___,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,___X i X___,
	____ i _XXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXX_ i ____

#undef i
};

/* lcd digits */
const char lcd_digits[] =
{
	0,1,1,1,1,1,1,1,1,0,
	1,1,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,1,1,0,0,0,1,
	1,0,0,0,1,1,0,0,0,1,
	1,0,0,0,1,1,0,0,0,1,
	1,0,0,0,1,1,0,0,0,1,
	1,0,0,0,1,1,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,1,1,
	0,1,1,1,1,1,1,1,1,0,

	0,0,0,1,1,1,1,0,0,0,
	0,0,1,1,0,0,1,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,1,0,0,1,1,0,0,
	0,0,0,1,1,1,1,0,0,0,

	0,1,1,1,1,1,1,1,1,0,
	1,1,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,
	0,1,1,1,1,1,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,1,1,
	1,0,0,0,0,1,1,1,1,0,
	1,0,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,1,1,
	0,1,1,1,1,1,1,1,1,0,

	0,1,1,1,1,1,1,1,1,0,
	1,1,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,
	0,1,1,1,1,1,0,0,0,1,
	1,1,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,1,1,0,
	1,1,0,0,0,0,0,0,1,1,
	0,1,1,1,1,1,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,1,1,
	0,1,1,1,1,1,1,1,1,0,

	0,1,1,1,0,1,1,1,1,0,
	1,1,0,1,1,1,0,0,1,1,
	1,0,0,0,1,0,0,0,0,1,
	1,0,0,0,1,0,0,0,0,1,
	1,0,0,0,1,0,0,0,0,1,
	1,0,0,0,1,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,
	0,1,1,1,1,0,0,0,0,1,
	0,0,0,0,1,0,0,0,0,1,
	0,0,0,0,1,0,0,0,0,1,
	0,0,0,0,1,0,0,0,0,1,
	0,0,0,0,1,1,0,0,1,1,
	0,0,0,0,0,1,1,1,1,0,

	0,1,1,1,1,1,1,1,1,0,
	1,1,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,1,1,
	1,0,0,0,0,1,1,1,1,0,
	1,0,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,
	0,1,1,1,1,1,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,1,1,
	0,1,1,1,1,1,1,1,1,0,

	0,1,1,1,1,1,1,1,1,0,
	1,1,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,1,1,
	1,0,0,0,0,1,1,1,1,0,
	1,0,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,1,1,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,1,1,
	0,1,1,1,1,1,1,1,1,0,

	0,1,1,1,1,1,1,1,1,0,
	1,1,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,
	0,1,1,1,1,0,0,0,0,1,
	0,0,0,0,1,0,0,0,0,1,
	0,0,0,0,1,0,0,0,0,1,
	0,0,0,0,1,0,0,0,0,1,
	0,0,0,0,1,0,0,0,0,1,
	0,0,0,0,1,0,0,0,0,1,
	0,0,0,0,1,0,0,0,0,1,
	0,0,0,0,1,0,0,0,0,1,
	0,0,0,0,1,1,0,0,1,1,
	0,0,0,0,0,1,1,1,1,0,

	0,1,1,1,1,1,1,1,1,0,
	1,1,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,1,1,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,1,1,
	0,1,1,0,0,0,0,1,1,0,
	1,1,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,1,1,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,1,1,
	0,1,1,1,1,1,1,1,1,0,

	0,1,1,1,1,1,1,1,1,0,
	1,1,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,1,1,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,
	0,1,1,1,1,0,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,1,1,
	0,1,1,1,1,1,1,1,1,0,

	/* 10: - */
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,1,1,1,1,1,1,0,0,
	0,1,0,0,0,0,0,0,1,0,
	0,1,0,0,0,0,0,0,1,0,
	0,1,0,0,0,0,0,0,1,0,
	0,0,1,1,1,1,1,1,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,

	/* 11: / */
	0,0,0,0,1,1,1,1,0,0,
	0,0,0,1,0,0,0,0,1,0,
	0,0,0,1,0,0,0,0,1,0,
	0,0,0,1,0,0,0,0,1,0,
	0,0,0,1,0,0,0,0,1,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,1,0,0,0,0,1,0,0,0,
	0,1,0,0,0,0,1,0,0,0,
	0,1,0,0,0,0,1,0,0,0,
	0,1,0,0,0,0,1,0,0,0,
	0,0,1,1,1,1,0,0,0,0,

	/* 12: ? */
	0,1,1,1,1,1,1,1,1,0,
	1,1,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,
	0,1,1,1,1,1,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,1,1,
	1,1,1,1,1,1,1,1,1,0,
	1,0,0,0,0,1,0,0,0,0,
	1,0,0,0,0,1,0,0,0,0,
	1,0,0,0,0,1,0,0,0,0,
	1,1,0,0,1,1,0,0,0,0,
	0,1,1,1,1,0,0,0,0,0,

	/* 13: " " */
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0
};

/* functions */
void ConfigMenu(int Init, int source);
void CleanUp();
void PageInput(int Number);
void ColorKey(int);
void PageCatching();
void CatchNextPage(int, int);
void GetNextPageOne(int up);
void GetNextSubPage(int offset);
void SwitchZoomMode();
void SwitchScreenMode(int newscreenmode);
void SwitchTranspMode();
void SwitchHintMode();
void CreateLine25();
void CopyBB2FB();
void RenderCatchedPage();
void RenderCharFB(int Char, tstPageAttr *Attribute);
void RenderCharBB(int Char, tstPageAttr *Attribute);
void RenderCharLCD(int Digit, int XPos, int YPos);
void RenderMessage(int Message);
void RenderPage();
void DecodePage();
void UpdateLCD();
int  Init(int source);
int  GetNationalSubset(const char *country_code);
int  GetTeletextPIDs(int source);
int  GetRCCode();
int  eval_triplet(int iOData, tstCachedPage *pstCachedPage,
						unsigned char *pAPx, unsigned char *pAPy,
						unsigned char *pAPx0, unsigned char *pAPy0,
						unsigned char *drcssubp, unsigned char *gdrcssubp,
						signed char *endcol, tstPageAttr *attrPassive, unsigned char* pagedata);
void eval_object(int iONr, tstCachedPage *pstCachedPage,
					  unsigned char *pAPx, unsigned char *pAPy,
					  unsigned char *pAPx0, unsigned char *pAPy0,
					  tObjType ObjType, unsigned char* pagedata);


#endif
