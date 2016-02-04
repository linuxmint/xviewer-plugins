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

#ifndef __XVIEWER_EXIF_DISPLAY_PLUGIN_H__
#define __XVIEWER_EXIF_DISPLAY_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <xviewer/xviewer-thumb-view.h>
#include <xviewer/xviewer-window.h>
#include <libpeas/peas-extension-base.h>
#include <libpeas/peas-object-module.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XVIEWER_TYPE_EXIF_DISPLAY_PLUGIN		(xviewer_exif_display_plugin_get_type ())
#define XVIEWER_EXIF_DISPLAY_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), XVIEWER_TYPE_EXIF_DISPLAY_PLUGIN, XviewerExifDisplayPlugin))
#define XVIEWER_EXIF_DISPLAY_PLUGIN_CLASS(k)	G_TYPE_CHECK_CLASS_CAST((k),      XVIEWER_TYPE_EXIF_DISPLAY_PLUGIN, XviewerExifDisplayPluginClass))
#define XVIEWER_IS_EXIF_DISPLAY_PLUGIN(o)	        (G_TYPE_CHECK_INSTANCE_TYPE ((o), XVIEWER_TYPE_EXIF_DISPLAY_PLUGIN))
#define XVIEWER_IS_EXIF_DISPLAY_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k),    XVIEWER_TYPE_EXIF_DISPLAY_PLUGIN))
#define XVIEWER_EXIF_DISPLAY_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o),  XVIEWER_TYPE_EXIF_DISPLAY_PLUGIN, XviewerExifDisplayPluginClass))

/* Private structure type */
typedef struct _XviewerExifDisplayPluginPrivate	XviewerExifDisplayPluginPrivate;

/*
 * Main object structure
 */
typedef struct _XviewerExifDisplayPlugin		XviewerExifDisplayPlugin;

struct _XviewerExifDisplayPlugin
{
	PeasExtensionBase parent_instance;

	XviewerThumbView *thumbview;
	XviewerWindow *window;

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
typedef struct _XviewerExifDisplayPluginClass	XviewerExifDisplayPluginClass;

struct _XviewerExifDisplayPluginClass
{
	PeasExtensionBaseClass parent_class;
};

/*
 * Public methods
 */
GType	xviewer_exif_display_plugin_get_type		(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_END_DECLS

#endif /* __XVIEWER_EXIF_DISPLAY_PLUGIN_H__ */
