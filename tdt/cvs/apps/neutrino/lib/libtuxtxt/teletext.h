#ifndef __teletext_h__
#define __teletext_h__

int tuxtxt_init();
void tuxtxt_close();
void tuxtxt_start(int tpid, int source = 0);  // Start caching
int  tuxtxt_stop(); // Stop caching
void tuxtxt_check_death();
int tuxtx_main(int _rc, int pid, int page = 0, int source = 0);
void tuxtx_stop_subtitle(bool clr = true);
int tuxtx_subtitle_running(int *pid, int *page, int *running);
void tuxtx_pause_subtitle(bool pause = true, int delayed = -1);
void tuxtx_set_pid(int pid, int page, const char * cc);

#endif
