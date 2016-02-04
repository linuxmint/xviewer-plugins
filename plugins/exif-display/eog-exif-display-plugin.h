/* Exif-display -- display information about digital pictures
 *
 * Copyright (C) 2009 The Free Software Foundation
 *
 * Author: Emmanuel Touzery  <emmanuel.touzery@free.fr>
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

#ifndef __EOG_EXIF_DISPLAY_PLUGIN_H__
#define __EOG_EXIF_DISPLAY_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <eog/eog-thumb-view.h>
#include <eog/eog-window.h>
#include <libpeas/peas-extension-base.h>
#include <libpeas/peas-object-module.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define EOG_TYPE_EXIF_DISPLAY_PLUGIN		(eog_exif_display_plugin_get_type ())
#define EOG_EXIF_DISPLAY_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), EOG_TYPE_EXIF_DISPLAY_PLUGIN, EogExifDisplayPlugin))
#define EOG_EXIF_DISPLAY_PLUGIN_CLASS(k)	G_TYPE_CHECK_CLASS_CAST((k),      EOG_TYPE_EXIF_DISPLAY_PLUGIN, EogExifDisplayPluginClass))
#define EOG_IS_EXIF_DISPLAY_PLUGIN(o)	        (G_TYPE_CHECK_INSTANCE_TYPE ((o), EOG_TYPE_EXIF_DISPLAY_PLUGIN))
#define EOG_IS_EXIF_DISPLAY_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k),    EOG_TYPE_EXIF_DISPLAY_PLUGIN))
#define EOG_EXIF_DISPLAY_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o),  EOG_TYPE_EXIF_DISPLAY_PLUGIN, EogExifDisplayPluginClass))

/* Private structure type */
typedef struct _EogExifDisplayPluginPrivate	EogExifDisplayPluginPrivate;

/*
 * Main object structure
 */
typedef struct _EogExifDisplayPlugin		EogExifDisplayPlugin;

struct _EogExifDisplayPlugin
{
	PeasExtensionBase parent_instance;

	EogThumbView *thumbview;
	EogWindow *window;

	GtkWidget *statusbar_exif;

	GtkBuilder *sidebar_builder;
	GtkWidget *gtkbuilder_widget;
	GtkDrawingArea *drawing_area;

	int *histogram_values_red;
	int *histogram_values_green;
	int *histogram_values_blue;

	int *histogram_values_rgb;

	int max_of_array_sums;
	int max_of_array_sums_rgb;

	/* Handlers ids */
	guint selection_changed_id;

	/* Settings */
	gboolean enable_statusbar;
	gboolean draw_chan_histogram;
	gboolean draw_rgb_histogram;
};

/*
 * Class definition
 */
typedef struct _EogExifDisplayPluginClass	EogExifDisplayPluginClass;

struct _EogExifDisplayPluginClass
{
	PeasExtensionBaseClass parent_class;
};

/*
 * Public methods
 */
GType	eog_exif_display_plugin_get_type		(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_END_DECLS

#endif /* __EOG_EXIF_DISPLAY_PLUGIN_H__ */
