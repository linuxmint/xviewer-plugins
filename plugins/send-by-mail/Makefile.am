# SendByMail plugin
plugindir = $(libdir)/xviewer/plugins
appstreamdir = $(datadir)/appdata

AM_CPPFLAGS = \
	-I$(top_srcdir)/src 				\
	$(XVIEWER_CFLAGS) 					\
	$(WARN_CFLAGS)					\
	-DXVIEWER_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

plugin_LTLIBRARIES = libsend-by-mail.la

libsend_by_mail_la_SOURCES = \
	xviewer-send-by-mail-plugin.h			\
	xviewer-send-by-mail-plugin.c

libsend_by_mail_la_LDFLAGS = \
	-avoid-version -module

libsend_by_mail_la_LIBADD = $(XVIEWER_LIBS)

# Plugin Info

plugin_in_files = send-by-mail.plugin.desktop.in

%.plugin: %.plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(AM_V_GEN)$(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.plugin.desktop.in=.plugin)

appstream_in_files = xviewer-send-by-mail.metainfo.xml.in

appstream_DATA = $(appstream_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

EXTRA_DIST = $(plugin_in_files) $(appstream_in_files)

CLEANFILES = $(plugin_DATA) $(appstream_DATA)
DISTCLEANFILES = $(plugin_DATA) $(appstream_DATA)

-include $(top_srcdir)/git.mk
