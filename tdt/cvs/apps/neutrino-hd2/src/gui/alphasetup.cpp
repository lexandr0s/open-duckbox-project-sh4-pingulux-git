/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

	Kommentar:

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


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

#include <gui/alphasetup.h>

#include <driver/fontrenderer.h>
#include <driver/rcinput.h>

#include <gui/color.h>

#include <gui/widget/messagebox.h>
#include <driver/screen_max.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>

#include <global.h>
#include <neutrino.h>

#define ALPHA_SETUP_ICON_DESELECTED      "volumeslider2"
#define ALPHA_SETUP_ICON_ALPHA1_SELECTED "volumeslider2red"


CAlphaSetup::CAlphaSetup(const neutrino_locale_t Name, unsigned char* Alpha, CChangeObserver* Observer)
{
	frameBuffer = CFrameBuffer::getInstance();
	observer = Observer;
	name = Name;

	alpha = Alpha;

	frameBuffer->setBlendLevel(*alpha);
}

int CAlphaSetup::exec(CMenuTarget* parent, const std::string &)
{
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	width = w_max(360, 0);

	hheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	mheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();

	height = hheight+ mheight*2;

	x = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth()-width) >> 1);
	y = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight()-height) >> 1);

	int res = menu_return::RETURN_REPAINT;
	if (parent)
	{
		parent->hide();
	}
	unsigned char alpha_alt= *alpha;

	frameBuffer->setBlendLevel(*alpha);
	
	paint();
	
#ifdef FB_BLIT
	frameBuffer->blit();
#endif	

	int selected = 0;
	int max = 1;

	unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_MENU]);

	bool loop=true;
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd, true );

		if ( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_MENU]);

		switch ( msg )
		{
			case CRCInput::RC_down:
			case CRCInput::RC_vfddown:
				{
					if(selected<max)
					{
						paintSlider(x + 10, y + hheight, alpha, LOCALE_GTXALPHA_ALPHA1, ALPHA_SETUP_ICON_DESELECTED, false);
						
						selected++;

						if(selected == 0)
							paintSlider(x + 10, y + hheight, alpha, LOCALE_GTXALPHA_ALPHA1, ALPHA_SETUP_ICON_ALPHA1_SELECTED, true );
					}
					break;
				}
				
			case CRCInput::RC_up:
			case CRCInput::RC_vfdup:
				{
					if (selected > 0)
					{
						paintSlider(x + 10, y + hheight, alpha, LOCALE_GTXALPHA_ALPHA1, ALPHA_SETUP_ICON_DESELECTED, false);

						selected--;

						if(selected == 0)
							paintSlider(x + 10, y + hheight, alpha, LOCALE_GTXALPHA_ALPHA1, ALPHA_SETUP_ICON_ALPHA1_SELECTED, true );
					}
					break;
				}

			case CRCInput::RC_right:
			case CRCInput::RC_vfdright:
				{
					if(selected == 0)
					{
						if (*alpha < 255) 
						{
							*alpha += 0x10;
							paintSlider(x + 10, y + hheight, alpha, LOCALE_GTXALPHA_ALPHA1, ALPHA_SETUP_ICON_ALPHA1_SELECTED, true );
							frameBuffer->setBlendLevel(*alpha);
						}
					
					}
					break;
				}
				
			case CRCInput::RC_left:
			case CRCInput::RC_vfdleft:
				{
					if(selected == 0)
					{
						if (*alpha >= 0x10) 
						{
							*alpha -= 0x10;
							paintSlider(x + 10, y + hheight, alpha, LOCALE_GTXALPHA_ALPHA1, ALPHA_SETUP_ICON_ALPHA1_SELECTED, true );
							frameBuffer->setBlendLevel(*alpha);
						}
					}
					break;
				}
					
			case CRCInput::RC_home:
			case CRCInput::RC_vfdexit:
				if ( *alpha != alpha_alt)
				{
					if (ShowLocalizedMessage(name, LOCALE_MESSAGEBOX_DISCARD, CMessageBox::mbrYes, CMessageBox::mbYes | CMessageBox::mbCancel) == CMessageBox::mbrCancel)
					{
						break;
					}
				}

				// sonst abbruch...
				*alpha = alpha_alt;

			case CRCInput::RC_timeout:
			case CRCInput::RC_ok:
			case CRCInput::RC_vfdok:
				loop = false;
				break;

			default:
				if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all )
				{
					loop = false;
					res = menu_return::RETURN_EXIT_ALL;
				}
		}
		
#ifdef FB_BLIT
		frameBuffer->blit();
#endif		
	}

	hide();

	if(observer)
		observer->changeNotify(name, NULL);

	return res;
}

void CAlphaSetup::hide()
{
	frameBuffer->paintBackgroundBoxRel(x, y, width, height);
	
#ifdef FB_BLIT
	frameBuffer->blit();
#endif	
}

void CAlphaSetup::paint()
{
	// top
	frameBuffer->paintBoxRel(x, y, width,hheight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP);
	
	// head
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(x+10,y+hheight, width, g_Locale->getText(name), COL_MENUHEAD, 0, true); // UTF-8
	
	// bottom
	frameBuffer->paintBoxRel(x, y + hheight, width, height -hheight, COL_MENUCONTENT_PLUS_0, RADIUS_MID, CORNER_BOTTOM);

	// slider
	paintSlider(x + 10, y + hheight, alpha, LOCALE_GTXALPHA_ALPHA1, ALPHA_SETUP_ICON_ALPHA1_SELECTED, true );
}

void CAlphaSetup::paintSlider(const int x, const int y, const unsigned char * const spos, const neutrino_locale_t text, const char * const iconname, const bool selected) // UTF-8
{
	if (!spos)
		return;

	int sspos = (*spos)*100/255;
	char wert[5];

	frameBuffer->paintBoxRel(x+70,y,120,mheight, COL_MENUCONTENT_PLUS_0);

	frameBuffer->paintIcon(NEUTRINO_ICON_VOLUMEBODY, x + 70, y + 2 + mheight / 4);
	frameBuffer->paintIcon(iconname, x + 73 + sspos, y + mheight / 4);

	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x, y + mheight, width, g_Locale->getText(text), COL_MENUCONTENT, 0, true); // UTF-8
	
	sprintf(wert, "%3d", sspos); // UTF-8 encoded

	frameBuffer->paintBoxRel(x + 70 + 120 + 10, y, 50, mheight, COL_MENUCONTENT_PLUS_0);
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x + 70 + 120 + 10, y+mheight, width, wert, COL_MENUCONTENT, 0, true); // UTF-8
}
