include $(top_srcdir)/common/common.am

lib_LTLIBRARIES = libme_editor.la
BUILT_SOURCES = midi_addresses.c

libme_editor_la_SOURCES = me_editor.c sysex.c
libme_editor_la_LDFLAGS = -version-info 0:1:0 @create_shared_lib@
libme_editor_la_LIBADD = $(top_srcdir)/common/libcommon.la \
			$(top_srcdir)/common/libmidi.la \
		       $(JACK_LIBS) $(GLIB_LIBS)
nodist_libme_editor_la_SOURCES = midi_addresses.c
libme_editor_la_CFLAGS = $(GLIB_CFLAGS) $(JACK_CFLAGS)

midi_addresses.c: $(top_srcdir)/manual_parse/manual_parse
	$(top_srcdir)/manual_parse/manual_parse > $@ \
		2> $(top_srcdir)/include/midi_addresses.h

EXTRA_DIST = libme_editor.pc.in sysex.h
pkgconfigdir = @PKGCONF_DIR@
pkgconfig_DATA = me_editor.pc

DISTCLEANFILES = midi_addresses.c
