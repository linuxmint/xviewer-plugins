# Slideshow Shuffle Plugin for xviewer
# Copyright (C) 2008  Johannes Marbach <jm@rapidrabbit.de>
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

from gi.repository import GObject, Gtk, Xviewer
from gi.module import get_introspection_module
import random

class SlideshowShufflePlugin(GObject.Object, Xviewer.WindowActivatable):

    # Override XviewerWindowActivatable's window property
    window = GObject.property(type=Xviewer.Window)

    def __init__(self):
        GObject.Object.__init__(self)

    def do_activate(self):
        self.store = None
        self.handlers = []
        for signal in ('window-state-event', 'prepared'):
            self.handlers.append((self.window, self.window.connect_after(
                signal, self.state_changed_cb, self)))

        # Starting from fullscreen does not emit a window-state event.
        for group in self.window.get_ui_manager().get_action_groups():
            for name in ('ViewSlideshow', 'PauseSlideshow', 'ViewFullscreen'):
                action = group.get_action(name)
                if action is not None:
                    self.handlers.append((action, action.connect_after(
                        'toggled', self.state_changed_cb, self)))
        self.state_changed_cb(self)

    def do_deactivate(self):
        for obj, handler in self.handlers:
            obj.disconnect(handler)
        self.handlers = []
        self.restore_order()

    # The callback functions are done statically to avoid causing additional
    # references on the window property causing xviewer to not quit correctly.
    @staticmethod
    def state_changed_cb(*args):
        self = args[-1]
        window = self.window
        mode = self.window.get_mode()
        store = window.get_store()
        if self.store is not None and store != self.store:
            self.restore_order()

        if mode == Xviewer.WindowMode.SLIDESHOW and self.store is None:
            if store is None or len(store) == 0:
                return False
            uris = [row[2].get_uri_for_display() for row in store]
            self.original_order = dict(zip(uris, range(len(uris))))
            # PyGObject's convenience wrapper hides DEFAULT/UNSORTED IDs.
            _, self.sort_column, self.sort_order = get_introspection_module(
                'Gtk').TreeSortable.get_sort_column_id(store)
            current = window.get_image()
            current_uri = current.get_uri_for_display() if current else uris[0]
            if current_uri in uris:
                uris.remove(current_uri)
                random.shuffle(uris)
                uris.insert(0, current_uri)
            else:
                random.shuffle(uris)
            ranks = dict(zip(uris, range(len(uris))))
            self.store = store

            # Nemo supplies an explicitly ordered list, disabling GTK sorting.
            # Select our comparator explicitly, leaving the native default intact.
            store.set_sort_func(2, self.random_sort_function, ranks)
            store.set_sort_column_id(2, Gtk.SortType.ASCENDING)
        elif mode == Xviewer.WindowMode.NORMAL:
            self.restore_order()
        return False

    def restore_order(self):
        store = self.store
        if store is None:
            return
        self.store = None
        store.set_sort_column_id(self.sort_column, self.sort_order)
        if self.sort_column == Gtk.TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID:
            # Unsorted means preserving Nemo's order, not alphabetizing it.
            rows = list(store)
            order = sorted(range(len(rows)), key=lambda i: self.original_order.get(
                rows[i][2].get_uri_for_display(), len(self.original_order)))
            store.reorder(order)
        self.original_order = {}

    @staticmethod
    def random_sort_function(store, iter1, iter2, ranks):
        positions = []
        for tree_iter in (iter1, iter2):
            image = store[tree_iter][2]
            if image is None:
                positions.append(-1)
            else:
                uri = image.get_uri_for_display()
                # Folder monitors can add images during a slideshow.
                positions.append(ranks.setdefault(uri, len(ranks)))
        return (positions[0] > positions[1]) - (positions[0] < positions[1])
