plugin_name = 'exif-display'

# plugin info
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

configure_file(
    input: 'org.x.viewer.plugins.exif-display.gschema.xml.in',
    output: 'org.x.viewer.plugins.exif-display.gschema.xml',
    configuration: configuration_data({'GETTEXT_PACKAGE': meson.project_name()}),
    install_dir: gschemas_dir
)

plugin_exif_resources = gnome.compile_resources(
    'xviewer-exif-display-plugin-resources',
    'xviewer-exif-display-plugin.gresource.xml',
    source_dir: '.'
)

plugin_exif_headers = [
    'xviewer-exif-display-plugin.h',
    'xviewer-exif-display-plugin-settings.h',
    'xviewer-exif-display-plugin-setup.h'
]

plugin_exif_sources = [
    'xviewer-exif-display-plugin.c',
    'xviewer-exif-display-plugin-setup.c',
    plugin_exif_resources
]

shared_library(
    plugin_name,
    plugin_exif_sources,
    dependencies: [config_h, glib, gtk, exif, libpeas, libpeas_gtk, xviewer],
    install: true,
    install_dir: plugin_libdir,
)

