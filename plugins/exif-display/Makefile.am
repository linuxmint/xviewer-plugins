# ExifDisplay plugin
plugindir = $(EOG_PLUGINS_LIBS_DIR)
appstreamdir = $(datadir)/appdata
uidir = $(EOG_PLUGINS_DATA_DIR)/exif-display

AM_CPPFLAGS = \
	-I$(top_srcdir)/src 				\
	$(EOG_CFLAGS) 					\
	$(WARN_CFLAGS)					\
	$(EXIFDISPLAY_CFLAGS)				\
	-DEOG_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DEOG_EXIF_DISPLAY_DATA_DIR=\""$(uidir)"\"

plugin_LTLIBRARIES = libexif-display.la

BUILT_SOURCES = \
	eog-exif-display-plugin-resources.c

libexif_display_la_SOURCES = \
	${BUILT_SOURCES}				\
	eog-exif-display-plugin.h			\
	eog-exif-display-plugin.c			\
	eog-exif-display-plugin-settings.h		\
	eog-exif-display-plugin-setup.c			\
	eog-exif-display-plugin-setup.h

libexif_display_la_LDFLAGS = \
	-avoid-version -module

libexif_display_la_LIBADD = \
	$(EOG_LIBS)		\
	$(EXIFDISPLAY_LIBS)

eog-exif-display-plugin-resources.c: eog-exif-display-plugin.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/eog-exif-display-plugin.gresource.xml)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --internal --target=$@ --sourcedir=$(srcdir) --generate-source $(srcdir)/eog-exif-display-plugin.gresource.xml

# Plugin Info

plugin_in_files = exif-display.plugin.desktop.in

%.plugin: %.plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(AM_V_GEN)$(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.plugin.desktop.in=.plugin)

appstream_in_files = eog-exif-display.metainfo.xml.in

appstream_DATA = $(appstream_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

gsettings_SCHEMAS = org.gnome.eog.plugins.exif-display.gschema.xml
@GSETTINGS_RULES@

EXTRA_DIST = \
	$(plugin_in_files)	\
	$(appstream_in_files)	\
	exif-display.ui		\
	exif-display-config.ui	\
	eog-exif-display-plugin.gresource.xml	\
	org.gnome.eog.plugins.exif-display.gschema.xml.in

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
