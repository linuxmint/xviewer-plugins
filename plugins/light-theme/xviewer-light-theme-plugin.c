/* LightTheme  -- Disable preference of dark theme variants
 *
 * Copyright (C) 2012 Felix Riemann
 *
 * Author: Felix Riemann <friemann@gnome.org>
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

#include <gtk/gtk.h>
#include <xviewer/xviewer-application.h>
#include <xviewer/xviewer-application-activatable.h>

#include "xviewer-light-theme-plugin.h"


static void
xviewer_application_activatable_iface_init (XviewerApplicationActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (XviewerLightThemePlugin, xviewer_light_theme_plugin,
                                PEAS_TYPE_EXTENSION_BASE, 0,
                G_IMPLEMENT_INTERFACE_DYNAMIC (XVIEWER_TYPE_APPLICATION_ACTIVATABLE,
                                        xviewer_application_activatable_iface_init))

enum {
	PROP_0,
	PROP_APPLICATION
};


static void
xviewer_light_theme_plugin_init (XviewerLightThemePlugin *plugin)
{
}

static void
_set_theme_setting (gboolean state)
{
	GtkSettings *settings = gtk_settings_get_default ();
	g_object_set (G_OBJECT (settings),
	              "gtk-application-prefer-dark-theme", state,
	              NULL);

}

static void
impl_activate (XviewerApplicationActivatable *activatable)
{
	_set_theme_setting (FALSE);
}

static void
impl_deactivate (XviewerApplicationActivatable *activatable)
{
	_set_theme_setting (TRUE);
}


static void
xviewer_light_theme_plugin_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
	XviewerLightThemePlugin *plugin = XVIEWER_LIGHT_THEME_PLUGIN (object);

	switch (prop_id)
	{
	case PROP_APPLICATION:
		g_value_set_object (value, plugin->app);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
xviewer_light_theme_plugin_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
	XviewerLightThemePlugin *plugin = XVIEWER_LIGHT_THEME_PLUGIN (object);

	switch (prop_id)
	{
	case PROP_APPLICATION:
		plugin->app = XVIEWER_APPLICATION (g_value_dup_object (value));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
xviewer_light_theme_plugin_class_init (XviewerLightThemePluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = xviewer_light_theme_plugin_set_property;
	object_class->get_property = xviewer_light_theme_plugin_get_property;

	g_object_class_override_property (object_class, PROP_APPLICATION,
                                          "app");
}

static void
xviewer_light_theme_plugin_class_finalize (XviewerLightThemePluginClass *klass)
{
	/* Dummy needed for G_DEFINE_DYNAMIC_TYPE_EXTENDED */
}

static void
xviewer_application_activatable_iface_init (XviewerApplicationActivatableInterface *iface)
{
	iface->activate = impl_activate;
	iface->deactivate = impl_deactivate;
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
	xviewer_light_theme_plugin_register_type (G_TYPE_MODULE (module));
	peas_object_module_register_extension_type (module,
	                                       XVIEWER_TYPE_APPLICATION_ACTIVATABLE,
	                                       XVIEWER_TYPE_LIGHT_THEME_PLUGIN);
}
