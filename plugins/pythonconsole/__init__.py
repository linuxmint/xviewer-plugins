# -*- coding: utf-8 -*-

# __init__.py -- plugin object
#
# Copyright (C) 2007 - Diego Escalante Urrelo
# Copyright (C) 2006 - Steve Frécinaux
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

# Parts from "Interactive Python-GTK Console" (stolen from gedit's python
# console.py which was actually stolen from epiphany's console.py)
#     Copyright (C), 1998 James Henstridge <james@daa.com.au>
#     Copyright (C), 2005 Adam Hooper <adamh@densi.com>
#     Copyrignt (C), 2005 Raphaël Slinckx

from gi.repository import GObject, Gtk, Xviewer, PeasGtk

from .console import PythonConsole
from .config import PythonConsoleConfigWidget

import gettext
_ = gettext.translation('xviewer-plugins', fallback=True).gettext

_UI_STR = """
    <ui>
        <menubar name="MainMenu">
            <menu name="ToolsMenu" action="Tools">
                <separator/>
                <menuitem name="PythonConsole" action="PythonConsole"/>
                <separator/>
            </menu>
        </menubar>
    </ui>
    """


class PythonConsolePlugin(GObject.Object, Xviewer.WindowActivatable, PeasGtk.Configurable):

    # Override XviewerWindowActivatable's window property
    window = GObject.property(type=Xviewer.Window)
    action_group = None

    def __init__(self):
        GObject.Object.__init__(self)
        self.console_window = None
        self.ui_id = 0

    def do_activate(self):
        ui_manager = self.window.get_ui_manager()
        self.action_group = Gtk.ActionGroup(name='PythonConsole')
        self.action_group.add_actions([('PythonConsole', None, \
            _('P_ython Console'), None, None, self.console_cb)], self.window)
        ui_manager.insert_action_group(self.action_group, 0)
        self.ui_id = ui_manager.add_ui_from_string(_UI_STR)

    def do_deactivate(self):
        ui_manager = self.window.get_ui_manager()
        ui_manager.remove_ui(self.ui_id)
        self.ui_id = 0
        ui_manager.remove_action_group(self.action_group)
        self.action_group = None
        ui_manager.ensure_update()
        if self.console_window is not None:
            self.console_window.destroy()

    def console_cb(self, action, window):
        if not self.console_window:
            self.console_window = Gtk.Window()
            console = PythonConsole(namespace = {'__builtins__' : __builtins__,
                                                 'Xviewer' : Xviewer,
                                                 'window' : window})
            console.set_size_request(600, 400)
            console.eval('print("You can access the main window through ' \
                         '\'window\' :\\n%s" % window)', False)
            self.console_window.set_title(_('Python Console'))
            self.console_window.add(console)

            self.console_window.connect('delete-event', self.on_delete_cb)
            self.console_window.show_all()
            self.console_window.set_transient_for(window)
            self.console_window.set_destroy_with_parent(True)
        else:
            self.console_window.show_all()
        self.console_window.grab_focus()

    def on_delete_cb(self, window, event):
        window.destroy()
        self.console_window = None

    def do_create_configure_widget(self):
        config_widget = PythonConsoleConfigWidget(self.plugin_info.get_data_dir())

        return config_widget.configure_widget()
