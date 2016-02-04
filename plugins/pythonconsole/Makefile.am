# Python Console plugin
plugindir = $(EOG_PLUGINS_LIBS_DIR)
appstreamdir = $(datadir)/appdata
pythonconsoledir = $(plugindir)/pythonconsole
uidir = $(EOG_PLUGINS_DATA_DIR)/pythonconsole
plugin_in_files = pythonconsole.plugin.desktop.in
appstream_in_files = eog-pythonconsole.metainfo.xml.in

pythonconsole_PYTHON =	\
	__init__.py	\
	config.py	\
	console.py

UI_FILES = config.ui
ui_DATA = $(UI_FILES)

%.plugin: %.plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(AM_V_GEN)$(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.plugin.desktop.in=.plugin)

appstream_DATA = $(appstream_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

gsettings_SCHEMAS = org.gnome.eog.plugins.pythonconsole.gschema.xml
@GSETTINGS_RULES@

EXTRA_DIST = \
	$(plugin_in_files)	\
	$(appstream_in_files)	\
	$(UI_FILES)		\
	org.gnome.eog.plugins.pythonconsole.gschema.xml.in


CLEANFILES = $(plugin_DATA) $(appstream_DATA) $(gsettings_SCHEMAS)
DISTCLEANFILES = $(plugin_DATA) $(appstream_DATA) $(gsettings_SCHEMAS)
MAINTAINERCLEANFILES = $(gsettings_SCHEMAS:.xml=.valid)

-include $(top_srcdir)/git.mk
