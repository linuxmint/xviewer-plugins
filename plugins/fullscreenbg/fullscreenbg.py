# -*- coding: utf-8 -*-

# Fullscreen Background Plugin for Eye of GNOME
# Copyright (C) 2012 Adrian Zgorzałek <a.zgorzalek@gmail.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

from gi.repository import GObject, Gdk, Gio, Gtk, Eog, PeasGtk
from os.path import join


class BackgroundChanger(GObject.Object, Eog.WindowActivatable, \
    PeasGtk.Configurable):
    """Class implementing custom background in fullscreen mode"""
    BASE_KEY = 'org.gnome.eog.plugins.fullscreenbg'
    window = GObject.property(type=Eog.Window)
    preferences_dialog_created = False

    def __init__(self):
        GObject.Object.__init__(self)
        self.settings = Gio.Settings.new(self.BASE_KEY)

    def do_activate(self):
        self.state_handler_id = \
            self.window.connect('window-state-event', self.state_changed_cb)

    def do_deactivate(self):
        self.window.disconnect(self.state_handler_id)

    def state_changed_cb(self, window, event):
        scroll_view = self.window.get_view()
        mode = self.window.get_mode()
        if self.settings.get_boolean('use-custom'):
            color = Gdk.RGBA()
            color.parse(self.settings.get_string('background-color'))
        else:
            color = scroll_view.get_property('background-color')

        if mode == Eog.WindowMode(Eog.WindowMode.FULLSCREEN)\
            or mode == Eog.WindowMode(Eog.WindowMode.SLIDESHOW):
            scroll_view.override_bg_color(color)

    def do_create_configure_widget(self):
        # Create preference dialog
        if not self.preferences_dialog_created:
            signals = {'use_global_settings_cb': self.use_global_settings_cb,
                       'hide_colorbutton_cb': self.hide_colorbutton_cb}
            builder = Gtk.Builder()
            builder.set_translation_domain('eog-plugins')
            builder.add_from_file(join(self.plugin_info.get_data_dir(), \
                                       'preferences_dialog.ui'))
            builder.connect_signals(signals)

            self.use_global_checkbutton = builder.get_object( \
                                              'use_global_settings_checkbutton')
            self.choose_color = builder.get_object('background_color_colorbutton')
            self.preferences_dialog = builder.get_object('preferences_box')
            # Restore values
            self.use_global_checkbutton.set_active( \
                self.settings.get_boolean('use-custom'))
            color = Gdk.RGBA()
            color.parse(self.settings.get_string('background-color'))
            self.choose_color.set_rgba(color)
            self.preferences_dialog_created = True

        return self.preferences_dialog

    def use_global_settings_cb(self, data):
        self.settings.set_boolean('use-custom', \
                                  self.use_global_checkbutton.get_active())

    def hide_colorbutton_cb(self, data):
        self.settings.set_string('background-color', \
                                 self.choose_color.get_rgba().to_string())
