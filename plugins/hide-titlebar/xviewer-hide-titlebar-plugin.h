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

#ifndef __XVIEWER_HIDE_TITLEBAR_PLUGIN_H__
#define __XVIEWER_HIDE_TITLEBAR_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <xviewer/xviewer-window.h>
#include <libpeas/peas-extension-base.h>
#include <libpeas/peas-object-module.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XVIEWER_TYPE_HIDE_TITLEBAR_PLUGIN		(xviewer_hide_titlebar_plugin_get_type ())
#define XVIEWER_HIDE_TITLEBAR_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), XVIEWER_TYPE_HIDE_TITLEBAR_PLUGIN, XviewerHideTitlebarPlugin))
#define XVIEWER_HIDE_TITLEBAR_PLUGIN_CLASS(k)	G_TYPE_CHECK_CLASS_CAST((k),      XVIEWER_TYPE_HIDE_TITLEBAR_PLUGIN, XviewerHideTitlebarPluginClass))
#define XVIEWER_IS_HIDE_TITLEBAR_PLUGIN(o)	        (G_TYPE_CHECK_INSTANCE_TYPE ((o), XVIEWER_TYPE_HIDE_TITLEBAR_PLUGIN))
#define XVIEWER_IS_HIDE_TITLEBAR_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k),    XVIEWER_TYPE_HIDE_TITLEBAR_PLUGIN))
#define XVIEWER_HIDE_TITLEBAR_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o),  XVIEWER_TYPE_HIDE_TITLEBAR_PLUGIN, XviewerHideTitlebarPluginClass))

/* Private structure type */
typedef struct _XviewerHideTitlebarPluginPrivate	XviewerHideTitlebarPluginPrivate;

/*
 * Main object structure
 */
typedef struct _XviewerHideTitlebarPlugin		XviewerHideTitlebarPlugin;

struct _XviewerHideTitlebarPlugin
{
	PeasExtensionBase parent_instance;

	XviewerWindow *window;
	gboolean   previous_state;
};

/*
 * Class definition
 */
typedef struct _XviewerHideTitlebarPluginClass	XviewerHideTitlebarPluginClass;

struct _XviewerHideTitlebarPluginClass
{
	PeasExtensionBaseClass parent_class;
};

/*
 * Public methods
 */
GType	xviewer_hide_titlebar_plugin_get_type		(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_END_DECLS

#endif /* __XVIEWER_HIDE_TITLEBAR_PLUGIN_H__ */
