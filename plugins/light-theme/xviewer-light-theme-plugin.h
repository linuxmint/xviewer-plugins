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

#ifndef __XVIEWER_LIGHT_THEME_PLUGIN_H__
#define __XVIEWER_LIGHT_THEME_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <xviewer/xviewer-application.h>
#include <libpeas/peas-extension-base.h>
#include <libpeas/peas-object-module.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XVIEWER_TYPE_LIGHT_THEME_PLUGIN		(xviewer_light_theme_plugin_get_type ())
#define XVIEWER_LIGHT_THEME_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), XVIEWER_TYPE_LIGHT_THEME_PLUGIN, XviewerLightThemePlugin))
#define XVIEWER_LIGHT_THEME_PLUGIN_CLASS(k)	G_TYPE_CHECK_CLASS_CAST((k),      XVIEWER_TYPE_LIGHT_THEME_PLUGIN, XviewerLightThemePluginClass))
#define XVIEWER_IS_LIGHT_THEME_PLUGIN(o)	        (G_TYPE_CHECK_INSTANCE_TYPE ((o), XVIEWER_TYPE_LIGHT_THEME_PLUGIN))
#define XVIEWER_IS_LIGHT_THEME_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k),    XVIEWER_TYPE_LIGHT_THEME_PLUGIN))
#define XVIEWER_LIGHT_THEME_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o),  XVIEWER_TYPE_LIGHT_THEME_PLUGIN, XviewerLightThemePluginClass))

/* Private structure type */
typedef struct _XviewerLightThemePluginPrivate	XviewerLightThemePluginPrivate;

/*
 * Main object structure
 */
typedef struct _XviewerLightThemePlugin		XviewerLightThemePlugin;

struct _XviewerLightThemePlugin
{
	PeasExtensionBase parent_instance;

	XviewerApplication *app;
};

/*
 * Class definition
 */
typedef struct _XviewerLightThemePluginClass	XviewerLightThemePluginClass;

struct _XviewerLightThemePluginClass
{
	PeasExtensionBaseClass parent_class;
};

/*
 * Public methods
 */
GType	xviewer_light_theme_plugin_get_type		(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_END_DECLS

#endif /* __XVIEWER_LIGHT_THEME_PLUGIN_H__ */
