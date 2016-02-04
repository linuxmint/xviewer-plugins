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

#ifndef __EOG_FIT_TO_WIDTH_PLUGIN_H__
#define __EOG_FIT_TO_WIDTH_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <eog/eog-window.h>
#include <libpeas/peas-extension-base.h>
#include <libpeas/peas-object-module.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define EOG_TYPE_FIT_TO_WIDTH_PLUGIN		(eog_fit_to_width_plugin_get_type ())
#define EOG_FIT_TO_WIDTH_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), EOG_TYPE_FIT_TO_WIDTH_PLUGIN, EogFitToWidthPlugin))
#define EOG_FIT_TO_WIDTH_PLUGIN_CLASS(k)	G_TYPE_CHECK_CLASS_CAST((k),      EOG_TYPE_FIT_TO_WIDTH_PLUGIN, EogFitToWidthPluginClass))
#define EOG_IS_FIT_TO_WIDTH_PLUGIN(o)	        (G_TYPE_CHECK_INSTANCE_TYPE ((o), EOG_TYPE_FIT_TO_WIDTH_PLUGIN))
#define EOG_IS_FIT_TO_WIDTH_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k),    EOG_TYPE_FIT_TO_WIDTH_PLUGIN))
#define EOG_FIT_TO_WIDTH_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o),  EOG_TYPE_FIT_TO_WIDTH_PLUGIN, EogFitToWidthPluginClass))

/* Private structure type */
typedef struct _EogFitToWidthPluginPrivate	EogFitToWidthPluginPrivate;

/*
 * Main object structure
 */
typedef struct _EogFitToWidthPlugin		EogFitToWidthPlugin;

struct _EogFitToWidthPlugin
{
	PeasExtensionBase parent_instance;

	EogWindow *window;

	GtkActionGroup *ui_action_group;
	guint           ui_menuitem_id;

};

/*
 * Class definition
 */
typedef struct _EogFitToWidthPluginClass	EogFitToWidthPluginClass;

struct _EogFitToWidthPluginClass
{
	PeasExtensionBaseClass parent_class;
};

/*
 * Public methods
 */
GType	eog_fit_to_width_plugin_get_type		(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_END_DECLS

#endif /* __EOG_FIT_TO_WIDTH_PLUGIN_H__ */
