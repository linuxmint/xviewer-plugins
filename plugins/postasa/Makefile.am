# Postasa plugin
plugindir = $(XVIEWER_PLUGINS_LIBS_DIR)
appstreamdir = $(datadir)/appdata

AM_CPPFLAGS = \
	-I$(top_srcdir) 				\
	$(XVIEWER_CFLAGS) 					\
	$(WARN_CFLAGS)					\
	$(POSTASA_CFLAGS) 				\
	-DXVIEWER_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DXVIEWER_POSTASA_PLUGIN_DIR=\""$(plugindir)"\"		\
	-DXVIEWER_POSTASA_DATA_DIR=\""$(XVIEWER_PLUGINS_DATA_DIR)/postasa"\"


plugin_LTLIBRARIES = libpostasa.la

BUILT_SOURCES = \
	xviewer-postasa-plugin-resources.c

libpostasa_la_SOURCES = \
	${BUILT_SOURCES}				\
	xviewer-postasa-plugin.h				\
	xviewer-postasa-plugin.c

libpostasa_la_LDFLAGS = \
	$(PLUGIN_LIBTOOL_FLAGS)				\
	$(XVIEWER_LIBS)

libpostasa_la_LIBADD = $(XVIEWER_LIBS)			\
	$(POSTASA_LIBS)

xviewer-postasa-plugin-resources.c: xviewer-postasa-plugin.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/xviewer-postasa-plugin.gresource.xml)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --internal --target=$@ --sourcedir=$(srcdir) --generate-source $(srcdir)/xviewer-postasa-plugin.gresource.xml

UI_FILES = postasa-config.xml postasa-uploads.xml

# Plugin Info

plugin_in_files = postasa.plugin.desktop.in

%.plugin: %.plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(AM_V_GEN)$(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.plugin.desktop.in=.plugin)

appstream_in_files = xviewer-postasa.metainfo.xml.in

appstream_DATA = $(appstream_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

EXTRA_DIST = \
	$(plugin_in_files)	\
	$(appstream_in_files)	\
	$(UI_FILES)		\
	xviewer-postasa-plugin.gresource.xml

CLEANFILES = $(plugin_DATA) $(appstream_DATA) $(BUILT_SOURCES)
DISTCLEANFILES = $(plugin_DATA) $(appstream_DATA) $(BUILT_SOURCES)

-include $(top_srcdir)/git.mk
