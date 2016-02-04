# Postr plugin
plugindir = $(libdir)/eog/plugins
appstreamdir = $(datadir)/appdata

AM_CPPFLAGS = \
	-I$(top_srcdir) 				\
	$(EOG_CFLAGS) 					\
	$(WARN_CFLAGS)					\
	-DEOG_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

plugin_LTLIBRARIES = libpostr.la

libpostr_la_SOURCES = \
	eog-postr-plugin.h				\
	eog-postr-plugin.c

libpostr_la_LDFLAGS = \
	$(PLUGIN_LIBTOOL_FLAGS)

libpostr_la_LIBADD = $(EOG_LIBS)


# Plugin Info

plugin_in_files = postr.plugin.desktop.in

%.plugin: %.plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(AM_V_GEN)$(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.plugin.desktop.in=.plugin)

appstream_in_files = eog-postr.metainfo.xml.in

appstream_DATA = $(appstream_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

EXTRA_DIST = $(plugin_in_files) $(appstream_in_files)

CLEANFILES = $(plugin_DATA) $(appstream_DATA)
DISTCLEANFILES = $(plugin_DATA) $(appstream_DATA)

-include $(top_srcdir)/git.mk
