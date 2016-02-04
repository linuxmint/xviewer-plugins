/* HideTitlebar  -- Hide XviewerWindow's titlebar when maximized
 *
 * Copyright (C) 2012 The Free Software Foundation
 *
 * Author: Felix Riemann    <friemann@gnome.org>
 *         Claudio Saavedra <csaavedra@igalia.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <xviewer/xviewer-window.h>
#include <xviewer/xviewer-window-activatable.h>

#include "xviewer-hide-titlebar-plugin.h"


static void
xviewer_window_activatable_iface_init (XviewerWindowActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (XviewerHideTitlebarPlugin, xviewer_hide_titlebar_plugin,
                                PEAS_TYPE_EXTENSION_BASE, 0,
                     G_IMPLEMENT_INTERFACE_DYNAMIC (XVIEWER_TYPE_WINDOW_ACTIVATABLE,
                                             xviewer_window_activatable_iface_init))

enum {
	PROP_0,
	PROP_WINDOW
};


static void
xviewer_hide_titlebar_plugin_init (XviewerHideTitlebarPlugin *plugin)
{
}

static void
impl_activate (XviewerWindowActivatable *activatable)
{
	XviewerHideTitlebarPlugin *plugin = XVIEWER_HIDE_TITLEBAR_PLUGIN (activatable);
	GtkWindow *window = GTK_WINDOW (plugin->window);

	plugin->previous_state = 
		gtk_window_get_hide_titlebar_when_maximized (window);

	if (!plugin->previous_state)
		gtk_window_set_hide_titlebar_when_maximized (window, TRUE);
}

static void
impl_deactivate	(XviewerWindowActivatable *activatable)
{
	XviewerHideTitlebarPlugin *plugin = XVIEWER_HIDE_TITLEBAR_PLUGIN (activatable);
	GtkWindow *window = GTK_WINDOW (plugin->window);

	gtk_window_set_hide_titlebar_when_maximized (window,
	                                             plugin->previous_state);
}

static void
xviewer_hide_titlebar_plugin_get_property (GObject    *object,
                                       guint       prop_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
	XviewerHideTitlebarPlugin *plugin = XVIEWER_HIDE_TITLEBAR_PLUGIN (object);

	switch (prop_id)
	{
	case PROP_WINDOW:
		g_value_set_object (value, plugin->window);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
xviewer_hide_titlebar_plugin_set_property (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
	XviewerHideTitlebarPlugin *plugin = XVIEWER_HIDE_TITLEBAR_PLUGIN (object);

	switch (prop_id)
	{
	case PROP_WINDOW:
		plugin->window = XVIEWER_WINDOW (g_value_dup_object (value));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
xviewer_hide_titlebar_plugin_class_init (XviewerHideTitlebarPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = xviewer_hide_titlebar_plugin_set_property;
	object_class->get_property = xviewer_hide_titlebar_plugin_get_property;

	g_object_class_override_property (object_class, PROP_WINDOW, "window");
}

static void
xviewer_hide_titlebar_plugin_class_finalize (XviewerHideTitlebarPluginClass *klass)
{
	/* Dummy needed for G_DEFINE_DYNAMIC_TYPE_EXTENDED */
}

static void
xviewer_window_activatable_iface_init (XviewerWindowActivatableInterface *iface)
{
	iface->activate = impl_activate;
	iface->deactivate = impl_deactivate;
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
	xviewer_hide_titlebar_plugin_register_type (G_TYPE_MODULE (module));
	peas_object_module_register_extension_type (module,
	                                       XVIEWER_TYPE_WINDOW_ACTIVATABLE,
	                                       XVIEWER_TYPE_HIDE_TITLEBAR_PLUGIN);
}
