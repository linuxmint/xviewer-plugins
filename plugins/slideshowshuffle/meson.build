plugin_name = 'slideshowshuffle'

# Plugin info
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

# Plugin sources
plugin_slideshow_sources = [
    'slideshowshuffle.c'
]

# Build shared library
shared_library(
    plugin_name,
    plugin_slideshow_sources,
    dependencies: [config_h, glib, gtk, libpeas, xviewer],
    install: true,
    install_dir: plugin_libdir,
)