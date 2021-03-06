/*
	volume bar - Neutrino-GUI

	Copyright (C) 2001 Steffen Hehn 'McClean'
	and some other guys
	Homepage: http://dbox.cyberphoria.org/

	Copyright (C) 2011-2012 M. Liebmann (micha-bbg)
	Copyright (C) 2012 Stefan Seyfried

	License: GPL

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the
	Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
	Boston, MA  02110-1301, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <global.h>
#include <neutrino.h>
#include <gui/infoclock.h>
#include <gui/keybind_setup.h>
#include <system/debug.h>
/* compat header from zapit/include */
#include <audio.h>
#include <system/settings.h>
#include <daemonc/remotecontrol.h>
#include <driver/framebuffer.h>
#include <driver/volume.h>

#if HAVE_COOL_HARDWARE
#include <gui/widget/progressbar.h>
#endif

CFrameBuffer * frameBuffer;
extern CRemoteControl * g_RemoteControl;
extern cAudio * audioDecoder;
static CProgressBar *g_volscale = NULL;

CVolume::CVolume()
{
	frameBuffer	= CFrameBuffer::getInstance();
#if 0
	g_Zapit		= new CZapitClient;
	g_RCInput	= new CRCInput;
	v_RemoteControl	= new CRemoteControl;
#endif
	VolumeFont	= SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO;
	paintDigits	= true;  // For future On/Off digits
	paintShadow	= false; // For future On/Off switch shadow
	MuteIconFrame	= false; // For future On/Off switch IconFrame
	ShadowOffset	= 4;
	mute_ay		= 0;
	m_mode = CNeutrinoApp::getInstance()->getMode();

	Init();
}

CVolume::~CVolume()
{
	if (g_volscale)
		delete g_volscale;
	g_volscale = NULL;
}

void CVolume::Init()
{
	mute_ay_old	= mute_ay;
	int faktor_h	= 18; // scale * 10
	int clock_height= 0;
	int clock_width = 0;
	int x_corr 	= 0;
	pB		= 2; // progress border
	spacer		= 8;

	colBar		= COL_MENUCONTENT_PLUS_0;
	colFrame	= COL_MENUCONTENT_PLUS_3;
	colContent	= COL_MENUCONTENT;
	colShadow	= COL_MENUCONTENTDARK_PLUS_0;

	x		= frameBuffer->getScreenX();
	y = sy		= frameBuffer->getScreenY() + spacer / 2;
	sw		= g_settings.screen_EndX - spacer;
	sh		= frameBuffer->getScreenHeight();

	frameBuffer->getIconSize(NEUTRINO_ICON_VOLUME, &icon_w, &icon_h);
	vbar_h		= std::max((icon_h * faktor_h) / 10, digit_h+digit_offset);
	progress_h	= icon_h - 2*pB;
	progress_w	= 200;
	vbar_w		= spacer + icon_w + spacer + progress_w + spacer;
	if (paintDigits) {
		digit_w		= g_Font[VolumeFont]->getRenderWidth("100");
		digit_offset	= g_Font[VolumeFont]->getDigitOffset();
		digit_h		= g_Font[VolumeFont]->getDigitHeight();
		progress_h	= std::max(icon_h, digit_h) - 2*pB;
		vbar_w 		+= digit_w;
	}

	if (g_volscale)
		delete g_volscale;
	g_volscale 	= new CProgressBar(true, progress_w, progress_h, 50, 100, 80, true);

	// mute icon
	mute_icon_dx 	= 0;
	mute_icon_dy 	= 0;
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_MUTE, &mute_icon_dx, &mute_icon_dy);
	mute_dx 	= mute_icon_dx;
	mute_dy 	= mute_icon_dy;
	mute_dx 	+= mute_icon_dx / 4;
	mute_dy 	+= mute_icon_dx / 4;
	mute_ax 	= sw - mute_dx;
	mute_ay 	= y;
	
	CNeutrinoApp* neutrino = CNeutrinoApp::getInstance();

	if ((g_settings.mode_clock) && (g_settings.volume_pos == 0)) {
		// Clock and MuteIcon in a line.
		clock_height = CInfoClock::getInstance(true)->time_height;
		clock_width = CInfoClock::getInstance(true)->time_width;
		mute_ay += (clock_height - mute_dy) / 2;
	}
	else {
		// Volume level and MuteIcon in a line.
		if (mute_dy > vbar_h)
			y += (mute_dy - vbar_h) / 2;
		else
			mute_ay += (vbar_h - mute_dy) / 2;
	}

	if ((g_settings.mode_clock) && (!neutrino->isMuted()))
		frameBuffer->paintBackgroundBoxRel(sw - clock_width, y, clock_width, clock_height);
//printf("\n##### [volume.cpp Zeile %d] mute_ax %d, mute_dx %d\n \n", __LINE__, mute_ax, mute_dx);
	switch (g_settings.volume_pos)
	{
		case 0:// upper right
			if (( neutrino->getMode() != CNeutrinoApp::mode_scart ) && ( neutrino->getMode() != CNeutrinoApp::mode_audio) && ( neutrino->getMode() != CNeutrinoApp::mode_pic)) {
				if ((neutrino->isMuted()) && (!g_settings.mode_clock))
					x_corr = mute_dx + spacer;
				if (g_settings.mode_clock)
					y += clock_height + spacer / 2;
			}
			x = sw - vbar_w - x_corr;
			break;
		case 1:// upper left
			break;
		case 2:// bottom left
			y = sh - vbar_h;
			break;
		case 3:// bottom right
			x = sw - vbar_w;
			y = sh - vbar_h;
			break;
		case 4:// center default
			x = ((sw - vbar_w) / 2) + x;
			break;
		case 5:// center higher
			x = ((sw - vbar_w) / 2) + x;
			y = sh - sh/15;
			break;
#ifdef EVOLUX
		case 6:// off
			break;
#endif
	}

	icon_x		= x + spacer;
	icon_y		= y + ((vbar_h - icon_h) / 2);
	progress_x	= icon_x + icon_w + spacer;
	progress_y	= y + ((vbar_h - progress_h) / 2);
	if (paintDigits) {
		digit_x		= progress_x + progress_w + spacer/2;
		digit_y		= y + digit_h + digit_offset + ((vbar_h - digit_h) / 2);
		digit_b_x	= digit_x - spacer/4;
		digit_b_w	= vbar_w - (digit_b_x - x);
	}
}

CVolume* CVolume::getInstance()
{
	static CVolume* Volume = NULL;
	if(!Volume)
		Volume = new CVolume();
	return Volume;
}

void CVolume::AudioMute(int newValue, bool isEvent)
{
	if (!g_Zapit)	/* don't die... */
		return;
	CNeutrinoApp* neutrino = CNeutrinoApp::getInstance();
	bool doInit = newValue != (int) neutrino->isMuted();

	CVFD::getInstance()->setMuted(newValue);
	neutrino->setCurrentMuted(newValue);
	g_Zapit->muteAudio(newValue);

	if( isEvent && ( neutrino->getMode() != CNeutrinoApp::mode_scart ) && ( neutrino->getMode() != CNeutrinoApp::mode_audio) && ( neutrino->getMode() != CNeutrinoApp::mode_pic))
	{
		if ((mute_ay_old != mute_ay) && (mute_ay_old > 0))
			frameBuffer->paintBackgroundBoxRel(mute_ax, mute_ay_old, mute_dx, mute_dy);
		if ((g_settings.mode_clock) && (doInit))
			CInfoClock::getInstance(true)->ClearDisplay();
		frameBuffer->paintMuteIcon(newValue, mute_ax, mute_ay, mute_dx, mute_dy, MuteIconFrame);
		if (doInit) {
			Init();
			CInfoClock::getInstance(true)->Init(true);
		}
	}
}

void CVolume::setvol(int vol)
{
	audioDecoder->setVolume(vol, vol);
}

#ifdef EVOLUX
void CVolume::setVolume(const neutrino_msg_t key, const bool _bDoPaint, bool nowait)
#else
void CVolume::setVolume(const neutrino_msg_t key, const bool bDoPaint, bool nowait)
#endif
{
	if (!g_RCInput) /* don't die... */
		return;
	neutrino_msg_t msg	= key;
	int mode = CNeutrinoApp::getInstance()->getMode();
	
	if (msg <= CRCInput::RC_MaxRC) {
		if(m_mode != mode) {
			m_mode = mode;
			Init();
			setVolume(msg);
			return;
		}
	}
	
	int vol			= g_settings.current_volume;
	fb_pixel_t * pixbuf	= NULL;

#ifdef EVOLUX
	bool bDoPaint = _bDoPaint;
	if (g_settings.volume_pos == 6)
		bDoPaint = false;
#endif

	if(bDoPaint) {
		pixbuf = new fb_pixel_t[(vbar_w+ShadowOffset) * (vbar_h+ShadowOffset)];
		if(pixbuf!= NULL)
			frameBuffer->SaveScreen(x, y, vbar_w+ShadowOffset, vbar_h+ShadowOffset, pixbuf);

		// volumebar shadow
		if (paintShadow)
			frameBuffer->paintBoxRel(x+ShadowOffset , y+ShadowOffset , (paintDigits) ? vbar_w - vbar_h : vbar_w, vbar_h, colShadow, ROUNDED, CORNER_TOP_LEFT | CORNER_BOTTOM_LEFT);
		// volumebar
		frameBuffer->paintBoxRel(x , y , (paintDigits) ? vbar_w - vbar_h : vbar_w, vbar_h, colBar, ROUNDED, CORNER_TOP_LEFT | CORNER_BOTTOM_LEFT);
		// frame for progress
		frameBuffer->paintBoxRel(progress_x-pB, progress_y-pB, progress_w+pB*1, progress_h+pB*2, colFrame);
		// volume icon
		frameBuffer->paintIcon(NEUTRINO_ICON_VOLUME, icon_x, icon_y, 0, colBar);

		g_volscale->reset();
		refreshVolumebar(vol);
		frameBuffer->blit();
	}

	neutrino_msg_data_t data;
	uint64_t timeoutEnd;

	do {
		if (msg <= CRCInput::RC_MaxRC) 
		{
			int sub_chan_keybind = 0;
			if (g_settings.mode_left_right_key_tv == SNeutrinoSettings::VOLUME && g_RemoteControl && g_RemoteControl->subChannels.size() < 1)
 			     sub_chan_keybind = 1;
			
			if ((msg == CRCInput::RC_plus) || (sub_chan_keybind == 1 && (msg == CRCInput::RC_right))) {
				if(CNeutrinoApp::getInstance()->isMuted()) {
					if ((bDoPaint) && (pixbuf!= NULL)) {
						frameBuffer->RestoreScreen(x, y, vbar_w+ShadowOffset, vbar_h+ShadowOffset, pixbuf);
						delete [] pixbuf;
					}
					AudioMute(false, true);
					Init();
					setVolume(msg);
					return;
				}
				
				if(!CNeutrinoApp::getInstance()->isMuted()) {
					if (g_settings.current_volume < 100 - g_settings.current_volume_step)
						g_settings.current_volume += g_settings.current_volume_step;
					else
						g_settings.current_volume = 100;
#ifdef ENABLE_GRAPHLCD
				nGLCD::ShowVolume(true);
#endif
				}				
			}
			else if ((msg == CRCInput::RC_minus) || (sub_chan_keybind == 1 && (msg == CRCInput::RC_left))) {
				if(CNeutrinoApp::getInstance()->isMuted() && g_settings.current_volume > 0) {
					if ((bDoPaint) && (pixbuf!= NULL)) {
						frameBuffer->RestoreScreen(x, y, vbar_w+ShadowOffset, vbar_h+ShadowOffset, pixbuf);
						delete [] pixbuf;
					}
					AudioMute(false, true);
					Init();
					setVolume(msg);
					return;
				}
				
				if(!CNeutrinoApp::getInstance()->isMuted()) {
					if (g_settings.current_volume > g_settings.current_volume_step)
						g_settings.current_volume -= g_settings.current_volume_step;
						
					else if  (g_settings.show_mute_icon == 1) {
						if ((bDoPaint) && (pixbuf!= NULL)) {
							frameBuffer->RestoreScreen(x, y, vbar_w+ShadowOffset, vbar_h+ShadowOffset, pixbuf);
							delete [] pixbuf;
						}
						g_settings.current_volume = 0;
						AudioMute( true, true);
						Init();
						setVolume(msg);
						return;						
					}
					
					else if (g_settings.show_mute_icon == 0)
						g_settings.current_volume = 0;
#ifdef ENABLE_GRAPHLCD
				nGLCD::ShowVolume(true);
#endif
				}				
			}

			else if (msg == CRCInput::RC_home)
				break;
			else {
				g_RCInput->postMsg(msg, data);
				break;
			}

			setvol(g_settings.current_volume);
			timeoutEnd = CRCInput::calcTimeoutEnd(nowait ? 1 : 3);
		}
		else if (msg == NeutrinoMessages::EVT_VOLCHANGED) {
			timeoutEnd = CRCInput::calcTimeoutEnd(3);
		}
		else if (CNeutrinoApp::getInstance()->handleMsg(msg, data) & messages_return::unhandled) {
			g_RCInput->postMsg(msg, data);
			break;
		}

		if (bDoPaint) {
			if(vol != g_settings.current_volume) {
				vol = g_settings.current_volume;
				refreshVolumebar(g_settings.current_volume);
			}
		}

		if (bDoPaint)
			frameBuffer->blit();

		CVFD::getInstance()->showVolume(g_settings.current_volume);
		if (msg != CRCInput::RC_timeout) {
			g_RCInput->getMsgAbsoluteTimeout(&msg, &data, &timeoutEnd, true );
		}
	} while (msg != CRCInput::RC_timeout);

#ifdef ENABLE_GRAPHLCD
	nGLCD::ShowVolume(false);
#endif
	if( (bDoPaint) && (pixbuf!= NULL) ) {
		frameBuffer->RestoreScreen(x, y, vbar_w+ShadowOffset, vbar_h+ShadowOffset, pixbuf);
		delete [] pixbuf;
	}
	if (bDoPaint)
		frameBuffer->blit();
}

void CVolume::refreshVolumebar(int current_volume)
{
	// progressbar
	g_volscale->paintProgressBar2(progress_x, progress_y, current_volume);
	if (paintDigits) {
		// shadow for erase digits
		if (paintShadow)
			frameBuffer->paintBoxRel(digit_b_x+ShadowOffset, y+ShadowOffset, digit_b_w, vbar_h, colShadow, ROUNDED, CORNER_TOP_RIGHT | CORNER_BOTTOM_RIGHT);
		// erase digits
		frameBuffer->paintBoxRel(digit_b_x, y, digit_b_w, vbar_h, colBar, ROUNDED, CORNER_TOP_RIGHT | CORNER_BOTTOM_RIGHT);
		// digits
		char buff[4];
		snprintf(buff, 4, "%3d", current_volume);
		g_Font[VolumeFont]->RenderString(digit_x, digit_y, digit_w, buff, colContent);	
	}
}
