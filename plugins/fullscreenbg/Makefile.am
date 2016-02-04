# Fullscreen Background plugin
plugindir = $(XVIEWER_PLUGINS_LIBS_DIR)
appstreamdir = $(datadir)/appdata
plugin_in_files = fullscreenbg.plugin.desktop.in
appstream_in_files = xviewer-fullscreenbg.metainfo.xml.in

gsettings_SCHEMAS = org.x.viewer.plugins.fullscreenbg.gschema.xml
gsettings_SCHEMAS_in_files = org.x.viewer.plugins.fullscreenbg.gschema.xml.in


ui_DATA = preferences_dialog.ui
uidir = $(XVIEWER_PLUGINS_DATA_DIR)/fullscreenbg

plugin_PYTHON = \
	fullscreenbg.py

%.plugin: %.plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(AM_V_GEN)$(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.plugin.desktop.in=.plugin)

appstream_DATA = $(appstream_in_files:.xml.in=.xml)

@INTLTOOL_XML_RULE@
@GSETTINGS_RULES@

EXTRA_DIST = $(plugin_in_files)\
	     $(appstream_in_files)\
	     $(ui_DATA)\
	     $(gsettings_SCHEMAS_in_files)

CLEANFILES = $(plugin_DATA) $(appstream_DATA) $(gsettings_SCHEMAS)
DISTCLEANFILES = $(plugin_DATA) $(appstream_DATA) $(gsettings_SCHEMAS)
MAINTAINERCLEANFILES = $(gsettings_SCHEMAS:.xml=.valid)

-include $(top_srcdir)/git.mk
