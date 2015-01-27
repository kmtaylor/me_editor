include $(top_srcdir)/common/common.am

bin_PROGRAMS = me_explorer

me_explorer_SOURCES = sysex_explorer.c
me_explorer_LDADD = $(top_srcdir)/me_editor/libme_editor.la \
		  $(top_srcdir)/common/libcommon.la \
		  $(NCURSES_LIBS) $(PANEL_LIBS) $(FORM_LIBS) $(MENU_LIBS)
me_explorer_CFLAGS = $(NCURSES_CFLAGS)

EXTRA_DIST = sysex_explorer.h
