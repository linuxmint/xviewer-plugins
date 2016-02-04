# ExifDisplay plugin
plugindir = $(XVIEWER_PLUGINS_LIBS_DIR)
appstreamdir = $(datadir)/appdata
uidir = $(XVIEWER_PLUGINS_DATA_DIR)/exif-display

AM_CPPFLAGS = \
	-I$(top_srcdir)/src 				\
	$(XVIEWER_CFLAGS) 					\
	$(WARN_CFLAGS)					\
	$(EXIFDISPLAY_CFLAGS)				\
	-DXVIEWER_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DXVIEWER_EXIF_DISPLAY_DATA_DIR=\""$(uidir)"\"

plugin_LTLIBRARIES = libexif-display.la

BUILT_SOURCES = \
	xviewer-exif-display-plugin-resources.c

libexif_display_la_SOURCES = \
	${BUILT_SOURCES}				\
	xviewer-exif-display-plugin.h			\
	xviewer-exif-display-plugin.c			\
	xviewer-exif-display-plugin-settings.h		\
	xviewer-exif-display-plugin-setup.c			\
	xviewer-exif-display-plugin-setup.h

libexif_display_la_LDFLAGS = \
	-avoid-version -module

libexif_display_la_LIBADD = \
	$(XVIEWER_LIBS)		\
	$(EXIFDISPLAY_LIBS)

xviewer-exif-display-plugin-resources.c: xviewer-exif-display-plugin.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/xviewer-exif-display-plugin.gresource.xml)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --internal --target=$@ --sourcedir=$(srcdir) --generate-source $(srcdir)/xviewer-exif-display-plugin.gresource.xml

# Plugin Info

plugin_in_files = exif-display.plugin.desktop.in

%.plugin: %.plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(AM_V_GEN)$(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.plugin.desktop.in=.plugin)

appstream_in_files = xviewer-exif-display.metainfo.xml.in

appstream_DATA = $(appstream_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

gsettings_SCHEMAS = org.x.viewer.plugins.exif-display.gschema.xml
@GSETTINGS_RULES@

EXTRA_DIST = \
	$(plugin_in_files)	\
	$(appstream_in_files)	\
	exif-display.ui		\
	exif-display-config.ui	\
	xviewer-exif-display-plugin.gresource.xml	\
	org.x.viewer.plugins.exif-display.gschema.xml.in

CLEANFILES = \
	${BUILT_SOURCES}	\
	$(plugin_DATA)		\
	$(appstream_DATA)	\
	$(gsettings_SCHEMAS)

DISTCLEANFILES = \
	${BUILT_SOURCES}	\
	$(plugin_DATA)		\
	$(appstream_DATA)	\
	$(gsettings_SCHEMAS)

MAINTAINERCLEANFILES = $(gsettings_SCHEMAS:.xml=.valid)

-include $(top_srcdir)/git.mk
