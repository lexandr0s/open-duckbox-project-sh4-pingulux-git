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


#include <global.h>
#include <neutrino.h>
#include <gui/widget/icons.h>
#include <gui/widget/menue.h>
#include <libtuxtxt/teletext.h>

extern CRemoteControl		* g_RemoteControl; /* neutrino.cpp */
extern CAPIDChangeExec		* APIDChanger;
extern CAudioSetupNotifier	* audioSetupNotifier;
int dvbsub_getpid();
int dvbsub_stop();

#include <gui/audio_select.h>

//
//  -- AUDIO Selector Menue Handler Class
//  -- to be used for calls from Menue
//  -- (2005-08-31 rasc)
// 

// -- this is a copy from neutrino.cpp!!
#define AUDIOMENU_ANALOGOUT_OPTION_COUNT 3
const CMenuOptionChooser::keyval AUDIOMENU_ANALOGOUT_OPTIONS[AUDIOMENU_ANALOGOUT_OPTION_COUNT] =
{
	{ 0, LOCALE_AUDIOMENU_STEREO    },
	{ 1, LOCALE_AUDIOMENU_MONOLEFT  },
	{ 2, LOCALE_AUDIOMENU_MONORIGHT }
};

int CAudioSelectMenuHandler::exec(CMenuTarget* parent, const std::string &actionkey)
{
	int           res = menu_return::RETURN_EXIT_ALL;


	if (parent) {
		parent->hide();
	}

	doMenu ();
	return res;
}

int CAudioSelectMenuHandler::doMenu ()
{
	CMenuWidget AudioSelector(LOCALE_APIDSELECTOR_HEAD, "audio", 360);
	unsigned int count;
	CSubtitleChangeExec SubtitleChanger;

	for(count=0; count < g_RemoteControl->current_PIDs.APIDs.size(); count++ ) {
		char apid[5];
		sprintf(apid, "%d", count);
		AudioSelector.addItem(new CMenuForwarderNonLocalized(
					g_RemoteControl->current_PIDs.APIDs[count].desc, true, NULL,
					APIDChanger, apid, CRCInput::convertDigitToKey(count + 1)),
				(count == g_RemoteControl->current_PIDs.PIDs.selected_apid));
	}

	// -- setup menue for to Dual Channel Stereo
	AudioSelector.addItem(GenericMenuSeparatorLine);

	CMenuOptionChooser* oj = new CMenuOptionChooser(LOCALE_AUDIOMENU_ANALOGOUT,
				&g_settings.audio_AnalogMode,
				AUDIOMENU_ANALOGOUT_OPTIONS, AUDIOMENU_ANALOGOUT_OPTION_COUNT,
				true, audioSetupNotifier, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED);

	AudioSelector.addItem( oj );

	CChannelList *channelList = CNeutrinoApp::getInstance ()->channelList;
	int curnum = channelList->getActiveChannelNumber();
	CZapitChannel * cc = channelList->getChannel(curnum);

	bool sep_added = false;
	if(cc) 
	{
		bool subs_running = false;
		for (int i = 0 ; i < (int)cc->getSubtitleCount() ; ++i) 
		{
			CZapitAbsSub* s = cc->getChannelSub(i);
			if (s->thisSubType == CZapitAbsSub::DVB) {
				CZapitDVBSub* sd = reinterpret_cast<CZapitDVBSub*>(s);
				fprintf(stderr, "[neutrino] adding DVB subtitle %s pid 0x%x\n", sd->ISO639_language_code.c_str(), sd->pId);
				if(!sep_added) {
					sep_added = true;
					AudioSelector.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_SUBTITLES_HEAD));
				}
				char spid[10];
				snprintf(spid,sizeof(spid), "DVB:%d", sd->pId);
				char item[64];
				snprintf(item,sizeof(item), "DVB: %s (pid %03X)", sd->ISO639_language_code.c_str(), sd->pId);
				bool this_sub_running = (sd->pId == dvbsub_getpid());
				subs_running |= this_sub_running;
				AudioSelector.addItem(new CMenuForwarderNonLocalized(item /*sd->ISO639_language_code.c_str()*/,
							!this_sub_running, NULL, &SubtitleChanger, spid, CRCInput::convertDigitToKey(++count)));
			} else if (s->thisSubType == CZapitAbsSub::TTX) {
				CZapitTTXSub* sd = reinterpret_cast<CZapitTTXSub*>(s);
				fprintf(stderr, "[neutrino] adding TTX subtitle %s pid %X mag %d page %d\n",
					sd->ISO639_language_code.c_str(), sd->pId, sd->teletext_magazine_number, sd->teletext_page_number);
				if(!sep_added) {
					sep_added = true;
					AudioSelector.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_SUBTITLES_HEAD));
				}
				char spid[64];
				int page = ((sd->teletext_magazine_number & 0xFF) << 8) | sd->teletext_page_number;
				int pid = sd->pId;
				snprintf(spid,sizeof(spid), "TTX:%d:%03X:%s", sd->pId, page, sd->ISO639_language_code.c_str()); 
				char item[64];
				snprintf(item,sizeof(item), "TTX: %s (pid %X page %03X)", sd->ISO639_language_code.c_str(), sd->pId, page);
				bool this_sub_running = tuxtx_subtitle_running(&pid, &page, NULL);
				subs_running |= this_sub_running;
				AudioSelector.addItem(new CMenuForwarderNonLocalized(item /*sd->ISO639_language_code.c_str()*/,
							!this_sub_running, NULL, &SubtitleChanger, spid, CRCInput::convertDigitToKey(++count)));
			}
		}
		
		if(sep_added) {
			if (subs_running)
				AudioSelector.addItem(new CMenuForwarder(LOCALE_SUBTITLES_STOP, true, NULL, &SubtitleChanger, "off", CRCInput::RC_stop));
		} else {
			// subtitles might not be available, but started by zapit anyway, as it remembers pids
			dvbsub_stop();
			tuxtx_stop_subtitle();
		}
	}
	AudioSelector.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_VOLUME_ADJUSTMENT));

	int percent[g_RemoteControl->current_PIDs.APIDs.size()];
	CZapitClient zapit;
	for(count = 0; count < g_RemoteControl->current_PIDs.APIDs.size(); count++ ) {
		zapit.getVolumePercent((unsigned int *) &percent[count], g_RemoteControl->current_PIDs.APIDs[count].pid);
		AudioSelector.addItem(new CMenuOptionNumberChooser(NONEXISTANT_LOCALE, &percent[count],
			count == g_RemoteControl->current_PIDs.PIDs.selected_apid,
			0, 999, audioSetupNotifier, 0, 0, NONEXISTANT_LOCALE,
			g_RemoteControl->current_PIDs.APIDs[count].desc));
	}

	return AudioSelector.exec(NULL, "");
}
