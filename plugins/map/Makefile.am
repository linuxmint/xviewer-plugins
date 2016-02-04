# Champlain plugin
plugindir = $(libdir)/xviewer/plugins
appstreamdir = $(datadir)/appdata

AM_CPPFLAGS = \
	-I$(top_srcdir) 				\
	$(XVIEWER_CFLAGS) 					\
	$(CHAMPLAIN_CFLAGS) 		\
	$(WARN_CFLAGS)					\
	-DXVIEWER_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

plugin_LTLIBRARIES = libmap.la

libmap_la_SOURCES = \
	xviewer-map-plugin.h				\
	xviewer-map-plugin.c

libmap_la_LDFLAGS = \
	$(PLUGIN_LIBTOOL_FLAGS)				\
	$(XVIEWER_LIBS) \
	$(CHAMPLAIN_LIBS)

# Plugin Info

plugin_in_files = map.plugin.desktop.in

%.plugin: %.plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(AM_V_GEN)$(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.plugin.desktop.in=.plugin)

appstream_in_files = xviewer-map.metainfo.xml.in

appstream_DATA = $(appstream_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

EXTRA_DIST = $(plugin_in_files) $(appstream_in_files)

CLEANFILES = $(plugin_DATA) $(appstream_DATA)
DISTCLEANFILES = $(plugin_DATA) $(appstream_DATA)

-include $(top_srcdir)/git.mk
