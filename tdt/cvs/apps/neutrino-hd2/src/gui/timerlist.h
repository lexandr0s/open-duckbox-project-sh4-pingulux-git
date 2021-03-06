/*
	Neutrino-GUI  -   DBoxII-Project

	Timerliste by Zwen
	
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

#ifndef __timerlist__
#define __timerlist__

#include <timerdclient/timerdclient.h>

#include <gui/widget/menue.h>

#include <driver/framebuffer.h>

#include <string>


class CTimerList : public CMenuTarget
{
	private:
		CFrameBuffer		*frameBuffer;
		unsigned int		selected;
		unsigned int		liststart;
		unsigned int		listmaxshow;
		int			fheight; // Fonthoehe Timerlist-Inhalt
		int			theight; // Fonthoehe Timerlist-Titel
		int               	buttonHeight;
		bool			visible;			

		CTimerdClient *Timer;
		CTimerd::TimerList timerlist;             // List of timers		
		CTimerd::responseGetTimer timerNew;
		int timerNew_standby_on;
		char timerNew_channel_name[30];
		char m_weekdaysStr[8];
		
		int timer_apids_dflt;
		int timer_apids_std;
		int timer_apids_ac3;
		int timer_apids_alt;

		int 			width;
		int 			height;
		int 			x;
		int 			y;

		int skipEventID;

		void paintItem(int pos);
		void paint();
		void paintHead();
		void paintFoot();
		void hide();
		int modifyTimer();
		int newTimer();

	public:
		CTimerList();
		~CTimerList();
		void updateEvents(void);
		int  show();
		int  exec(CMenuTarget* parent, const std::string & actionKey);
		static const char * convertTimerType2String(const CTimerd::CTimerEventTypes type); // UTF-8
		static std::string convertTimerRepeat2String(const CTimerd::CTimerEventRepeat rep); // UTF-8
		static std::string convertChannelId2String(const t_channel_id id); // UTF-8
};

bool askUserOnTimerConflict(time_t announceTime, time_t stopTime);


#endif
