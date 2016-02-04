/* Fit-to-width -- Fit zoom to the image width
 *
 * Copyright (C) 2009 The Free Software Foundation
 *
 * Author: Javier Sánchez  <jsanchez@deskblue.com>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __XVIEWER_FIT_TO_WIDTH_PLUGIN_H__
#define __XVIEWER_FIT_TO_WIDTH_PLUGIN_H__

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
#define XVIEWER_TYPE_FIT_TO_WIDTH_PLUGIN		(xviewer_fit_to_width_plugin_get_type ())
#define XVIEWER_FIT_TO_WIDTH_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), XVIEWER_TYPE_FIT_TO_WIDTH_PLUGIN, XviewerFitToWidthPlugin))
#define XVIEWER_FIT_TO_WIDTH_PLUGIN_CLASS(k)	G_TYPE_CHECK_CLASS_CAST((k),      XVIEWER_TYPE_FIT_TO_WIDTH_PLUGIN, XviewerFitToWidthPluginClass))
#define XVIEWER_IS_FIT_TO_WIDTH_PLUGIN(o)	        (G_TYPE_CHECK_INSTANCE_TYPE ((o), XVIEWER_TYPE_FIT_TO_WIDTH_PLUGIN))
#define XVIEWER_IS_FIT_TO_WIDTH_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k),    XVIEWER_TYPE_FIT_TO_WIDTH_PLUGIN))
#define XVIEWER_FIT_TO_WIDTH_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o),  XVIEWER_TYPE_FIT_TO_WIDTH_PLUGIN, XviewerFitToWidthPluginClass))

/* Private structure type */
typedef struct _XviewerFitToWidthPluginPrivate	XviewerFitToWidthPluginPrivate;

/*
 * Main object structure
 */
typedef struct _XviewerFitToWidthPlugin		XviewerFitToWidthPlugin;

struct _XviewerFitToWidthPlugin
{
	PeasExtensionBase parent_instance;

	XviewerWindow *window;

	GtkActionGroup *ui_action_group;
	guint           ui_menuitem_id;

};

/*
 * Class definition
 */
typedef struct _XviewerFitToWidthPluginClass	XviewerFitToWidthPluginClass;

struct _XviewerFitToWidthPluginClass
{
	PeasExtensionBaseClass parent_class;
};

/*
 * Public methods
 */
GType	xviewer_fit_to_width_plugin_get_type		(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_END_DECLS

#endif /* __XVIEWER_FIT_TO_WIDTH_PLUGIN_H__ */
