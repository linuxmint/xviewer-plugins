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

#ifndef __EOG_LIGHT_THEME_PLUGIN_H__
#define __EOG_LIGHT_THEME_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <eog/eog-application.h>
#include <libpeas/peas-extension-base.h>
#include <libpeas/peas-object-module.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define EOG_TYPE_LIGHT_THEME_PLUGIN		(eog_light_theme_plugin_get_type ())
#define EOG_LIGHT_THEME_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), EOG_TYPE_LIGHT_THEME_PLUGIN, EogLightThemePlugin))
#define EOG_LIGHT_THEME_PLUGIN_CLASS(k)	G_TYPE_CHECK_CLASS_CAST((k),      EOG_TYPE_LIGHT_THEME_PLUGIN, EogLightThemePluginClass))
#define EOG_IS_LIGHT_THEME_PLUGIN(o)	        (G_TYPE_CHECK_INSTANCE_TYPE ((o), EOG_TYPE_LIGHT_THEME_PLUGIN))
#define EOG_IS_LIGHT_THEME_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k),    EOG_TYPE_LIGHT_THEME_PLUGIN))
#define EOG_LIGHT_THEME_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o),  EOG_TYPE_LIGHT_THEME_PLUGIN, EogLightThemePluginClass))

/* Private structure type */
typedef struct _EogLightThemePluginPrivate	EogLightThemePluginPrivate;

/*
 * Main object structure
 */
typedef struct _EogLightThemePlugin		EogLightThemePlugin;

struct _EogLightThemePlugin
{
	PeasExtensionBase parent_instance;

	EogApplication *app;
};

/*
 * Class definition
 */
typedef struct _EogLightThemePluginClass	EogLightThemePluginClass;

struct _EogLightThemePluginClass
{
	PeasExtensionBaseClass parent_class;
};

/*
 * Public methods
 */
GType	eog_light_theme_plugin_get_type		(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_END_DECLS

#endif /* __EOG_LIGHT_THEME_PLUGIN_H__ */
