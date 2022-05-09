plugin_name = 'exif-display'

# plugin info
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

