plugin_name = 'postasa'

custom_target(
    '@0@-plugin-info'.format(plugin_name),
    input: '@0@.plugin.desktop.in'.format(plugin_name),
    output: '@0@.plugin'.format(plugin_name),
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: plugin_libdir,
)

custom_target(
    '@0@-metadata'.format(plugin_name),
    input: 'xviewer-@0@.metainfo.xml.in'.format(plugin_name),
    output: 'xviewer-@0@.metainfo.xml'.format(plugin_name),
    command: [intltool_merge, '-x', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
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
