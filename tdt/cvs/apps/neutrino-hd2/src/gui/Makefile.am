AM_CPPFLAGS = -fno-rtti -fno-exceptions

SUBDIRS = widget bedit

INCLUDES = \
	-I$(top_srcdir)/daemons \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/zapit/include \
	-I$(top_srcdir)/lib/libeventserver \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/lib/libnet \
	-I$(top_srcdir)/lib/libconfigfile \
	-I$(top_srcdir)/lib/libxmltree \
	-I$(top_srcdir)/lib/libupnpclient \
	-I$(top_srcdir)/lib/libcoolstream \
	@CURL_CFLAGS@ \
	@FREETYPE_CFLAGS@


noinst_LIBRARIES = libtimerlist.a libneutrino_gui.a libneutrino_gui2.a

libneutrino_gui_a_SOURCES = \
	bouquetlist.cpp channellist.cpp eventlist.cpp pluginlist.cpp \
	favorites.cpp sleeptimer.cpp screensetup.cpp \
	epgview.cpp infoviewer.cpp scan.cpp update.cpp \
	filebrowser.cpp audioplayer.cpp nfs.cpp pictureviewer.cpp \
	movieplayer.cpp rc_lock.cpp bookmarkmanager.cpp \
	timeosd.cpp epgplus.cpp epg_menu.cpp \
	streaminfo2.cpp dboxinfo.cpp \
	plugins.cpp imageinfo.cpp audio_select.cpp moviebrowser.cpp movieinfo.cpp \
	scan_setup.cpp zapit_setup.cpp

libneutrino_gui2_a_SOURCES = \
	color.cpp motorcontrol.cpp scale.cpp hdd_menu.cpp \
	alphasetup.cpp colorgainsetup.cpp dvbsub_select.cpp upnpbrowser.cpp

if BOXTYPE_CUBEREVO
libneutrino_gui2_a_SOURCES += cam_menu.cpp
endif

if BOXTYPE_CUBEREVO_MINI
libneutrino_gui2_a_SOURCES += cam_menu.cpp
endif

if BOXTYPE_CUBEREVO_MINI2
libneutrino_gui2_a_SOURCES += cam_menu.cpp
endif

if BOXTYPE_CUBEREVO_9500HD
libneutrino_gui2_a_SOURCES += cam_menu.cpp
endif

if BOXTYPE_DUCKBOX
libneutrino_gui2_a_SOURCES += cam_menu.cpp
endif

if BOXTYPE_GIGABLUE
libneutrino_gui2_a_SOURCES += cam_menu.cpp
endif

if BOXTYPE_DREAMBOX
libneutrino_gui2_a_SOURCES += cam_menu.cpp
endif

if BOXTYPE_XTREND
libneutrino_gui2_a_SOURCES += cam_menu.cpp
endif

if ENABLE_TEST_MENU
libneutrino_gui2_a_SOURCES += test_menu.cpp
endif

libtimerlist_a_SOURCES = timerlist.cpp

