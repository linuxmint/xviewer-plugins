# Fit-to-width plugin
plugindir = $(libdir)/xviewer/plugins
appstreamdir = $(datadir)/appdata

AM_CPPFLAGS = \
	-I$(top_srcdir)/src 				\
	$(XVIEWER_CFLAGS) 					\
	$(WARN_CFLAGS)					\
	-DXVIEWER_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

plugin_LTLIBRARIES = libfit-to-width.la

libfit_to_width_la_SOURCES = \
	xviewer-fit-to-width-plugin.h			\
	xviewer-fit-to-width-plugin.c

libfit_to_width_la_LDFLAGS = \
	-avoid-version -module

libfit_to_width_la_LIBADD = $(XVIEWER_LIBS)

# Plugin Info

plugin_in_files = fit-to-width.plugin.desktop.in

%.plugin: %.plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(AM_V_GEN)$(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.plugin.desktop.in=.plugin)

appstream_in_files = xviewer-fit-to-width.metainfo.xml.in

appstream_DATA = $(appstream_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

EXTRA_DIST = $(plugin_in_files) $(appstream_in_files)

CLEANFILES = $(plugin_DATA) $(appstream_DATA)
DISTCLEANFILES = $(plugin_DATA) $(appstream_DATA)

-include $(top_srcdir)/git.mk
