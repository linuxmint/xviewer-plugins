plugin_name = 'export-to-folder'

i18n.merge_file(
    input: '@0@.plugin.desktop.in'.format(plugin_name),
    output: '@0@.plugin'.format(plugin_name),
    type: 'desktop',
    args: ['--keyword=Name', '--keyword=Description'],
    po_dir: po_dir,
    install: true,
    install_dir: plugin_libdir,
)

i18n.merge_file(
    input: 'xviewer-@0@.metainfo.xml.in'.format(plugin_name),
    output: 'xviewer-@0@.metainfo.xml'.format(plugin_name),
    type: 'xml',
    po_dir: po_dir,
    install: true,
    install_dir: metainfo_dir
)

configure_file(
    input: 'org.x.viewer.plugins.export-to-folder.gschema.xml.in',
    output: 'org.x.viewer.plugins.export-to-folder.gschema.xml',
    configuration: configuration_data({'GETTEXT_PACKAGE': meson.project_name()}),
    install_dir: gschemas_dir
)

install_data(
    'preferences_dialog.ui',
    install_dir: plugin_datadir / plugin_name
)

install_data(
    'export-to-folder.py',
    install_dir: plugin_libdir
)
