/*
	Neutrino-GUI  -   DBoxII-Project


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


#ifndef __streaminfo2__
#define __streaminfo2__

#include <gui/widget/menue.h>

#include <driver/framebuffer.h>
#include <driver/pig.h>
#include <gui/scale.h>


class CStreamInfo2 : public CMenuTarget
{
	private:

		CFrameBuffer	*frameBuffer;
		int x;
		int y;
		int width;
		int height;
		int hheight,iheight,sheight; 	// head/info/small font height
		int xd;

		int  max_height;	// Frambuffer 0.. max
		int  max_width;	

		int yypos;
		int  paint_mode;

		int  font_head;
		int  font_info;
		int  font_small;

		int  fgcolor;
		int  fgcolor_head;
		int  bgcolor;

		int   sigBox_x;
		int   sigBox_y;
		int   sigBox_w;
		int   sigBox_h;
		int   sigBox_pos;
		int   sig_text_y;
		int   sig_text_ber_x;
		int   sig_text_sig_x;
		int   sig_text_snr_x;
		int   sig_text_rate_x;

		#define FESIG_BER 0
		#define FESIG_SIG 1
		#define FESIG_SNR 2
		#define FESIG_RATE 3
		#define FESIG_MAX 4

		struct feSignal {
			#define FESIG_VAL_MAX 0
			#define FESIG_VAL_CUR 1
			#define FESIG_VAL_MIN 2
			long long val[3];
			long long old;
			long long limit;
			int x;
			int yd_old;
			int oldpercent;
			int color;
			const char *title;
		} fesig[4];
		
		int  doSignalStrengthLoop();

		int dvrfd, dmxfd;
		struct timeval tv, last_tv, first_tv;
		unsigned long long bit_s;
		unsigned long long abit_s;
		unsigned long long b_total;

		int update_rate();
		int ts_setup();
		int ts_close();

		void paint(int mode);
		void paint_techinfo(int x, int y);
		void paint_techinfo_line(int xpos, int ypos, const neutrino_locale_t loc, const char *txt, int xpos2, const char *txt2);
		void paint_signal_fe_box(int x, int y, int w, int h);
		void paint_signal_fe();
		int  y_signal_fe(long long value, long long max_range, int max_y);
		void SignalRenderStr (long long value, int x, int y);
		CScale *sigscale;
		CScale *snrscale;
		void showSNR ();

	public:

		CStreamInfo2();
		~CStreamInfo2();
		int exec();

		void hide();
		int exec(CMenuTarget* parent, const std::string & actionKey);

};
class CStreamInfo2Handler : public CMenuTarget
{
        public:
                int exec( CMenuTarget* parent,  const std::string &actionKey);
};
#endif
