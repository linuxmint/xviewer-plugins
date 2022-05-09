plugin_name = 'postasa'

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

plugin_postasa_resources = gnome.compile_resources(
    'xviewer-postasa-plugin-resources',
    'xviewer-postasa-plugin.gresource.xml',
    source_dir: '.',
)

plugin_postasa_sources = [
    'xviewer-postasa-plugin.c',
    plugin_postasa_resources
]

shared_library(
    plugin_name,
    plugin_postasa_sources,
    dependencies: [config_h, gdata, glib, gtk, libpeas, xviewer],
    install: true,
    install_dir: plugin_libdir,
)
