# -*- coding: utf-8 -*-
#
# rename-file.py -- rename file plugin for xviewer
#
# Copyright (c) 2016  Stephane Groleau (stefgroleau@gmail.com)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

import os
import shutil

from gi.repository import GObject, Xviewer, Gio, Gtk, PeasGtk


ui_str = """
<ui>
    <menubar name="MainMenu">
        <menu name="ToolsMenu" action="Tools">
            <separator/>
            <menuitem name="Rename file" action="Rename"/>
            <separator/>
        </menu>
    </menubar>
</ui>
"""

BASE_KEY = 'org.x.viewer.plugins.rename-file'

class RenamePlugin(GObject.Object, Xviewer.WindowActivatable):
    window = GObject.property(type=Xviewer.Window)

    def __init__(self):
        GObject.Object.__init__(self)
        #self.settings = Gio.Settings.new(BASE_KEY)

    def do_activate(self):
        ui_manager = self.window.get_ui_manager()
        self.action_group = Gtk.ActionGroup(name='Rename')
        self.action_group.add_actions([('Rename', None,
                        _('_Rename'), "F2", None, self.rename_cb)], self.window)
        ui_manager.insert_action_group(self.action_group, 0)
        self.ui_id = ui_manager.add_ui_from_string(ui_str)

    def do_deactivate(self):
        ui_manager = self.window.get_ui_manager().remove_ui(self.ui_id);

    def rename_cb(self, action, window):
        def get_new_name(message, name):
            # Returns user input as a string or None
            # If user does not input text it returns None, NOT AN EMPTY STRING.
            dialogWindow = Gtk.MessageDialog(window, Gtk.DialogFlags.MODAL | Gtk.DialogFlags.DESTROY_WITH_PARENT, Gtk.MessageType.QUESTION, Gtk.ButtonsType.OK_CANCEL, message)

            dialogWindow.set_title('Rename file')

            dialogBox = dialogWindow.get_content_area()
            userEntry = Gtk.Entry()
            userEntry.set_visibility(True)
            userEntry.set_size_request(250,0)
            userEntry.set_text(name)
            dialogBox.pack_end(userEntry, False, False, 0)

            dialogWindow.show_all()
            response = dialogWindow.run()
            text = userEntry.get_text()
            dialogWindow.destroy()
            if (response == Gtk.ResponseType.OK) and (text != ''):
                return text
            else:
                return None


        # Get path to current image.
        image = window.get_image()
        if not image:
            print('No image can be renamed')
            return
        src = image.get_file().get_path()
        name = os.path.basename(src)

        new_name = get_new_name('Enter the new name', name)
        if new_name is None:
            return

        folder = os.path.dirname(src)
        dest = os.path.join(folder, new_name)
        try:
            os.rename(src, dest)
        except OSError:
            pass
        print('Renamed %s into %s' % (name, new_name))

