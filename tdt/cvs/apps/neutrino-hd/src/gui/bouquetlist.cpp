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

#include <string>
#include <algorithm>

#include <gui/bouquetlist.h>

#include <gui/color.h>
#include <gui/eventlist.h>
#include <gui/infoviewer.h>

#include <gui/widget/menue.h>
#include <gui/widget/buttons.h>
#include <gui/widget/icons.h>

#include <driver/fontrenderer.h>
#include <driver/screen_max.h>
#include <driver/rcinput.h>
#include <driver/fade.h>
#include <daemonc/remotecontrol.h>
#include <system/settings.h>
#ifdef EVOLUX
#include <system/localize_bouquetnames.h>
#endif

#include <global.h>
#include <neutrino.h>

extern CBouquetManager *g_bouquetManager;

CBouquetList::CBouquetList(const char * const Name)
{
	frameBuffer = CFrameBuffer::getInstance();
	selected    = 0;
	liststart   = 0;
	if(Name == NULL)
		name = g_Locale->getText(LOCALE_BOUQUETLIST_HEAD);
	else
		name = Name;

}

CBouquetList::~CBouquetList()
{
        for (std::vector<CBouquet *>::iterator it = Bouquets.begin(); it != Bouquets.end(); it++) {
               	delete (*it);
        }
	Bouquets.clear();
}

CBouquet* CBouquetList::addBouquet(CZapitBouquet * zapitBouquet)
{
	int BouquetKey= Bouquets.size();//FIXME not used ?
#ifdef EVOLUX
	localizeBouquetNames();
	CBouquet* tmp = new CBouquet(BouquetKey, zapitBouquet->lName.c_str(), zapitBouquet->bLocked);
#else
	CBouquet* tmp = new CBouquet(BouquetKey, zapitBouquet->bFav ? g_Locale->getText(LOCALE_FAVORITES_BOUQUETNAME) : zapitBouquet->Name.c_str(), zapitBouquet->bLocked);
#endif
	tmp->zapitBouquet = zapitBouquet;
	Bouquets.push_back(tmp);
	return tmp;
}

CBouquet* CBouquetList::addBouquet(const char * const pname, int BouquetKey, bool locked)
{
	if ( BouquetKey==-1 )
		BouquetKey= Bouquets.size();

	CBouquet* tmp = new CBouquet( BouquetKey, pname, locked, true);
	Bouquets.push_back(tmp);
	return(tmp);
}

void CBouquetList::deleteBouquet(CBouquet*bouquet)
{
	if (bouquet != NULL) {
		std::vector<CBouquet *>::iterator it = find(Bouquets.begin(), Bouquets.end(), bouquet);

		if (it != Bouquets.end()) {
			Bouquets.erase(it);
			delete bouquet;
		}
	}
}

int CBouquetList::getActiveBouquetNumber()
{
	return selected;
}

#if 0
void CBouquetList::adjustToChannel( int nChannelNr)
{
	for (uint32_t i=0; i<Bouquets.size(); i++) {
		int nChannelPos = Bouquets[i]->channelList->hasChannel(nChannelNr);
		if (nChannelPos > -1) {
			selected = i;
			Bouquets[i]->channelList->setSelected(nChannelPos);
			return;
		}
	}
}
#endif
bool CBouquetList::hasChannelID(t_channel_id channel_id)
{
	for (uint32_t i = 0; i < Bouquets.size(); i++) {
		int nChannelPos = Bouquets[i]->channelList->hasChannelID(channel_id);
		if (nChannelPos > -1)
			return true;
	}
	return false;
}

extern CBouquetList   * TVfavList;
bool CBouquetList::adjustToChannelID(t_channel_id channel_id)
{
//printf("CBouquetList::adjustToChannelID [%s] to %llx, selected %d size %d\n", name.c_str(), channel_id, selected, Bouquets.size());
	if(selected < Bouquets.size()) {
		int nChannelPos = Bouquets[selected]->channelList->hasChannelID(channel_id);
		if(nChannelPos > -1) {
//printf("CBouquetList::adjustToChannelID [%s] to %llx -> not needed\n", name.c_str(), channel_id);
			Bouquets[selected]->channelList->setSelected(nChannelPos);
			return true;
		}
	}
//printf("CBouquetList::adjustToChannelID [%s] to %llx\n", name.c_str(), channel_id);
	for (uint32_t i=0; i < Bouquets.size(); i++) {
		if(i == selected)
			continue;
		int nChannelPos = Bouquets[i]->channelList->hasChannelID(channel_id);
		if (nChannelPos > -1) {
			selected = i;
			Bouquets[i]->channelList->setSelected(nChannelPos);
			return true;
		}
	}
	return false;
}
/* used in channellist to switch bouquets up/down */
int CBouquetList::showChannelList( int nBouquet)
{
	if (nBouquet == -1)
		nBouquet = selected;

	int nNewChannel = Bouquets[nBouquet]->channelList->exec();
	if (nNewChannel > -1) {
		selected = nBouquet;
		nNewChannel = -2;
	}
	return nNewChannel;
}
/* bShowChannelList default to false , return seems not checked anywhere */
int CBouquetList::activateBouquet( int id, bool bShowChannelList)
{
	int res = -1;

	if(id < (int) Bouquets.size())
		selected = id;

	if (bShowChannelList) {
		res = Bouquets[selected]->channelList->exec();
		if(res > -1)
			res = -2;
	}
	return res;
}

int CBouquetList::exec( bool bShowChannelList)
{
	/* select bouquet to show */
	int res = show(bShowChannelList);
//printf("Bouquet-exec: res %d bShowChannelList %d\n", res, bShowChannelList); fflush(stdout);

	if(!bShowChannelList)
		return res;
	/* if >= 0, call activateBouquet to show channel list */
	if ( res > -1) {
		return activateBouquet(selected, bShowChannelList);
	}
	return res;
}

int CBouquetList::doMenu()
{
	int i = 0;
	int select = -1;
	static int old_selected = 0;
	signed int bouquet_id;
	char cnt[5];
	CZapitBouquet * tmp, * zapitBouquet;
	ZapitChannelList* channels;

	if(!Bouquets.size() || g_settings.minimode)
		return 0;

	zapitBouquet = Bouquets[selected]->zapitBouquet;
	/* zapitBouquet not NULL only on real bouquets, not on virtual SAT or HD */
	if(!zapitBouquet)
		return 0;

	CMenuWidget* menu = new CMenuWidget(LOCALE_CHANNELLIST_EDIT, NEUTRINO_ICON_SETTINGS);
	menu->enableFade(false);
	CMenuSelectorTarget * selector = new CMenuSelectorTarget(&select);

	sprintf(cnt, "%d", i);
	if(!zapitBouquet->bUser) {
		menu->addItem(new CMenuForwarder(LOCALE_FAVORITES_COPY, true, NULL, selector, cnt, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE), old_selected == i ++);
		menu->exec(NULL, "");
		delete menu;
		delete selector;
		printf("CBouquetList::doMenu: %d selected\n", select);
		if(select >= 0) {
			old_selected = select;
			switch(select) {
				case 0:
					hide();
					bouquet_id = g_bouquetManager->existsUBouquet(Bouquets[selected]->channelList->getName());
					if(bouquet_id < 0) {
						tmp = g_bouquetManager->addBouquet(Bouquets[selected]->channelList->getName(), true);
					} else
						tmp = g_bouquetManager->Bouquets[bouquet_id];

					channels = &zapitBouquet->tvChannels;
					for(int li = 0; li < (int) channels->size(); li++)
						tmp->addService((*channels)[li]);
					channels = &zapitBouquet->radioChannels;
					for(int li = 0; li < (int) channels->size(); li++)
						tmp->addService((*channels)[li]);
					return 1;
					break;
				default:
					break;
			}
		}
		return -1;
	} else {
		menu->addItem(new CMenuForwarder(LOCALE_BOUQUETEDITOR_DELETE, true, NULL, selector, cnt, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED), old_selected == i ++);
		menu->exec(NULL, "");
		delete menu;
		delete selector;
		printf("CBouquetList::doMenu: %d selected\n", select);
		if(select >= 0) {
			old_selected = select;
			switch(select) {
				case 0:
					hide();
					bouquet_id = g_bouquetManager->existsUBouquet(Bouquets[selected]->channelList->getName());
					if(bouquet_id >= 0) {
						g_bouquetManager->deleteBouquet(bouquet_id);
						return 1;
					}
					break;
				default:
					break;
			}
		}
		return -1;
	}
	return 0;
}

/* bShowChannelList default to true, returns new bouquet or -1/-2 */
int CBouquetList::show(bool bShowChannelList)
{
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;
	int res = -1;

	//if(Bouquets.size()==0)
	//	return res;

	CVFD::getInstance()->setMode(CVFD::MODE_MENU_UTF8, "");
	fheight     = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight();

	width  = w_max (g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getWidth()*52, 20);//500
	height = h_max (16 * fheight, 40);

	/* assuming all color icons must have same size */
	int icol_w, icol_h;
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_RED, &icol_w, &icol_h);

	footerHeight = std::max(icol_h+8, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight()+8); //TODO get footerHeight from buttons
	theight     = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	listmaxshow = (height - theight - footerHeight)/fheight;
	height      = theight + footerHeight + listmaxshow * fheight; // recalc height

	x = frameBuffer->getScreenX() + (frameBuffer->getScreenWidth() - width) / 2;
	y = frameBuffer->getScreenY() + (frameBuffer->getScreenHeight() - height) / 2;

	int lmaxpos= 1;
	int i= Bouquets.size();
	while ((i= i/10)!=0)
		lmaxpos++;

	COSDFader fader(g_settings.menu_Content_alpha);
	fader.StartFadeIn();

	paintHead();
	paint();
	frameBuffer->blit();

	int oldselected = selected;
	int firstselected = selected+ 1;
	int zapOnExit = false;

	unsigned int chn= 0;
	int pos= lmaxpos;

	uint64_t timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_CHANLIST]);

	bool loop=true;
	while (loop) {
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		if ( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_CHANLIST]);

		if((msg == NeutrinoMessages::EVT_TIMER) && (data == fader.GetTimer())) {
			if(fader.Fade())
				loop = false;
		}
		else if ((msg == CRCInput::RC_timeout                             ) ||
				(msg == (neutrino_msg_t)g_settings.key_channelList_cancel))
		{
			selected = oldselected;
			if(fader.StartFadeOut()) {
				timeoutEnd = CRCInput::calcTimeoutEnd( 1 );
				msg = 0;
			} else
				loop=false;
		}
		else if(msg == CRCInput::RC_red || msg == CRCInput::RC_favorites) {
			CNeutrinoApp::getInstance()->SetChannelMode(LIST_MODE_FAV);
			hide();
			return -3;
		} else if(msg == CRCInput::RC_green) {
			CNeutrinoApp::getInstance()->SetChannelMode(LIST_MODE_PROV);
			hide();
			return -3;
		} else if(msg == CRCInput::RC_yellow || msg == CRCInput::RC_sat) {
			if(bShowChannelList) {
				CNeutrinoApp::getInstance()->SetChannelMode(LIST_MODE_SAT);
				hide();
				return -3;
			}
		} else if(msg == CRCInput::RC_blue) {
			if(bShowChannelList) {
				CNeutrinoApp::getInstance()->SetChannelMode(LIST_MODE_ALL);
				hide();
				return -3;
			}
		}
		else if(Bouquets.size() == 0)
			continue; //FIXME msgs not forwarded to neutrino !!
		else if ( msg == CRCInput::RC_setup) {
			int ret = doMenu();
			if(ret > 0) {
				CNeutrinoApp::getInstance ()->g_channel_list_changed = true;
				res = -4;
				loop = false;
			} else if(ret < 0)
				paint();
		}
		else if ( msg == (neutrino_msg_t) g_settings.key_list_start ) {
			selected=0;
			liststart = (selected/listmaxshow)*listmaxshow;
			paint();
		}
		else if ( msg == (neutrino_msg_t) g_settings.key_list_end ) {
			selected=Bouquets.size()-1;
			liststart = (selected/listmaxshow)*listmaxshow;
			paint();
		}
		else if (msg == CRCInput::RC_up || (int) msg == g_settings.key_channelList_pageup)
		{
			int step = 0;
			int prev_selected = selected;

			step = ((int) msg == g_settings.key_channelList_pageup) ? listmaxshow : 1;  // browse or step 1
			selected -= step;
#if 0
			if((prev_selected-step) < 0)            // because of uint
				selected = Bouquets.size()-1;
#endif
			if((prev_selected-step) < 0) {
				if(prev_selected != 0 && step != 1)
					selected = 0;
				else
					selected = Bouquets.size() - 1;
			}

			paintItem(prev_selected - liststart);
			unsigned int oldliststart = liststart;
			liststart = (selected/listmaxshow)*listmaxshow;
			if(oldliststart!=liststart)
				paint();
			else
				paintItem(selected - liststart);
		}
		else if (msg == CRCInput::RC_down || (int) msg == g_settings.key_channelList_pagedown)
		{
			unsigned int step = 0;
			unsigned int prev_selected = selected;

			step = ((int) msg == g_settings.key_channelList_pagedown) ? listmaxshow : 1;  // browse or step 1
			selected += step;
#if 0
			if(selected >= Bouquets.size()) {
				if (((Bouquets.size() / listmaxshow) + 1) * listmaxshow == Bouquets.size() + listmaxshow) // last page has full entries
					selected = 0;
				else
					selected = ((step == listmaxshow) && (selected < (((Bouquets.size() / listmaxshow) + 1) * listmaxshow))) ? (Bouquets.size() - 1) : 0;
			}
#endif
			if(selected >= Bouquets.size()) {
				if((Bouquets.size() - listmaxshow -1 < prev_selected) && (prev_selected != (Bouquets.size() - 1)) && (step != 1))
					selected = Bouquets.size() - 1;
				else if (((Bouquets.size() / listmaxshow) + 1) * listmaxshow == Bouquets.size() + listmaxshow) // last page has full entries
					selected = 0;
				else
					selected = ((step == listmaxshow) && (selected < (((Bouquets.size() / listmaxshow)+1) * listmaxshow))) ? (Bouquets.size() - 1) : 0;
			}

			paintItem(prev_selected - liststart);
			unsigned int oldliststart = liststart;
			liststart = (selected/listmaxshow)*listmaxshow;
			if(oldliststart!=liststart)
				paint();
			else
				paintItem(selected - liststart);
		}
		else if(msg == (neutrino_msg_t)g_settings.key_bouquet_up || msg == (neutrino_msg_t)g_settings.key_bouquet_down) {
			if(bShowChannelList) {
				int mode = CNeutrinoApp::getInstance()->GetChannelMode();
				mode += (msg == (neutrino_msg_t)g_settings.key_bouquet_down) ? -1 : 1;
				if(mode < 0)
					mode = LIST_MODE_LAST - 1;
				else if(mode >= LIST_MODE_LAST)
					mode = 0;
				CNeutrinoApp::getInstance()->SetChannelMode(mode);
				hide();
				return -3;
			}

		}

		else if ( msg == CRCInput::RC_ok ) {
			if(!bShowChannelList || Bouquets[selected]->channelList->getSize() > 0) {
				zapOnExit = true;
				loop=false;
			}
		}
		else if (CRCInput::isNumeric(msg)) {
			if (pos == lmaxpos) {
				if (msg == CRCInput::RC_0) {
					chn = firstselected;
					pos = lmaxpos;
				} else {
					chn = CRCInput::getNumericValue(msg);
					pos = 1;
				}
			} else {
				chn = chn * 10 + CRCInput::getNumericValue(msg);
				pos++;
			}

			if (chn > Bouquets.size()) {
				chn = firstselected;
				pos = lmaxpos;
			}

			int prevselected=selected;
			selected = (chn - 1) % Bouquets.size(); // is % necessary (i.e. can firstselected be > Bouquets.size()) ?
			paintItem(prevselected - liststart);
			unsigned int oldliststart = liststart;
			liststart = (selected/listmaxshow)*listmaxshow;
			if(oldliststart!=liststart) {
				paint();
			} else {
				paintItem(selected - liststart);
			}

		} else {
			if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all ) {
				loop = false;
				res = -2;
			}
		};
		frameBuffer->blit();
	}
	hide();

	fader.Stop();

	CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);
	if(zapOnExit) {
		return (selected);
	} else {
		return (res);
	}
}

void CBouquetList::hide()
{
	frameBuffer->paintBackgroundBoxRel(x,y, width,height+10);
	frameBuffer->blit();
}

void CBouquetList::paintItem(int pos)
{
	int ypos = y+ theight+0 + pos*fheight;
	uint8_t    color;
	fb_pixel_t bgcolor;
	bool iscurrent = true;
	int npos = liststart + pos;
	const char * lname = NULL;

	if(npos < (int) Bouquets.size())
		lname = (Bouquets[npos]->zapitBouquet && Bouquets[npos]->zapitBouquet->bFav) ? g_Locale->getText(LOCALE_FAVORITES_BOUQUETNAME) : Bouquets[npos]->channelList->getName();

	if (npos == (int) selected) {
		color   = COL_MENUCONTENTSELECTED;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
		frameBuffer->paintBoxRel(x, ypos, width- 15, fheight, bgcolor, RADIUS_LARGE);
		if(npos < (int) Bouquets.size())
			CVFD::getInstance()->showMenuText(0, lname, -1, true);
	} else {
		if(npos < (int) Bouquets.size())
			iscurrent = Bouquets[npos]->channelList->getSize() > 0;
                color = iscurrent ? COL_MENUCONTENT : COL_MENUCONTENTINACTIVE;
                bgcolor = iscurrent ? COL_MENUCONTENT_PLUS_0 : COL_MENUCONTENTINACTIVE_PLUS_0;
		frameBuffer->paintBoxRel(x, ypos, width- 15, fheight, bgcolor);
	}

	if(npos < (int) Bouquets.size()) {
		char tmp[10];
		sprintf((char*) tmp, "%d", npos+ 1);

		int numpos = x+5+numwidth- g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth(tmp);
		g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(numpos,ypos+fheight, numwidth+5, tmp, color, fheight);

		g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(x+ 5+ numwidth+ 10, ypos+ fheight, width- numwidth- 20- 15, lname, color, 0, true); // UTF-8
		//CVFD::getInstance()->showMenuText(0, bouq->channelList->getName(), -1, true);
	}
}

const struct button_label CBouquetListButtons[4] =
{
        { NEUTRINO_ICON_BUTTON_RED, LOCALE_CHANNELLIST_FAVS},
        { NEUTRINO_ICON_BUTTON_GREEN, LOCALE_CHANNELLIST_PROVS},
        { NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_CHANNELLIST_SATS},
        { NEUTRINO_ICON_BUTTON_BLUE, LOCALE_CHANNELLIST_HEAD}
};

void CBouquetList::paintHead()
{
	frameBuffer->paintBoxRel(x,y, width,theight+0, COL_MENUHEAD_PLUS_0, RADIUS_LARGE, CORNER_TOP);
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(x+10,y+theight+0, width, name, COL_MENUHEAD, 0, true); // UTF-8
}

void CBouquetList::paint()
{
	liststart = (selected/listmaxshow)*listmaxshow;
	int lastnum =  liststart + listmaxshow;
	int bsize = Bouquets.size() > 0 ? Bouquets.size() : 1;

	if(lastnum<10)
		numwidth = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth("0");
	else if(lastnum<100)
		numwidth = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth("00");
	else if(lastnum<1000)
		numwidth = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth("000");
	else if(lastnum<10000)
		numwidth = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth("0000");
	else // if(lastnum<100000)
		numwidth = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth("00000");

	frameBuffer->paintBoxRel(x, y+theight, width, height - theight - footerHeight, COL_MENUCONTENT_PLUS_0);

	::paintButtons(x, y + (height - footerHeight), width, sizeof(CBouquetListButtons)/sizeof(CBouquetListButtons[0]), CBouquetListButtons, width, footerHeight);

	if(Bouquets.size())
	{
		for(unsigned int count=0;count<listmaxshow;count++) {
			paintItem(count);
		}
	}

	int ypos = y+ theight;
	int sb = fheight* listmaxshow;
	frameBuffer->paintBoxRel(x+ width- 15,ypos, 15, sb,  COL_MENUCONTENT_PLUS_1);

	int sbc= ((bsize - 1)/ listmaxshow)+ 1;
	int sbs= (selected/listmaxshow);

	frameBuffer->paintBoxRel(x+ width- 13, ypos+ 2+ sbs * (sb-4)/sbc, 11, (sb-4)/sbc, COL_MENUCONTENT_PLUS_3);
}
