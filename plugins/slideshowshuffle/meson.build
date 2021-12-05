plugin_name = 'slideshowshuffle'

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

install_data(
    'slideshowshuffle.py',
    install_dir: plugin_libdir
)
