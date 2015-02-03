include $(top_srcdir)/common/common.am

AM_YFLAGS = -d

noinst_PROGRAMS = manual_parse

manual_parse_CPPFLAGS = $(AM_CPPFLAGS) -DUSE_LOG=1 \
	-DMIDI_ADDRESSES=$(top_srcdir)/manual_parse/midi_addresses.txt
manual_parse_SOURCES = dump_tree.c manual_parser.y manual_scanner.l \
		       class_map.c
manual_parse_LDADD = $(top_srcdir)/common/libcommon.la

EXTRA_DIST = midi_tree.h midi_addresses.txt

DISTCLEANFILES = manual_parser.c manual_parser.h manual_scanner.c midi.log
