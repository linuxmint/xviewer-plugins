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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include <glib/gi18n-lib.h>
#include <eog/eog-debug.h>

// TODO: This is not cool. We need to find a better way to organize the API.
#ifndef HAVE_EXIF
#define HAVE_EXIF 1
#endif
#include <eog/eog-image.h>

#include <eog/eog-thumb-view.h>
#include <eog/eog-job-scheduler.h>
#include <eog/eog-exif-util.h>
#include <eog/eog-sidebar.h>
#include <eog/eog-window-activatable.h>

#include "eog-exif-display-plugin-settings.h"
#include "eog-exif-display-plugin-setup.h"
#include "eog-exif-display-plugin.h"

#define GRESOURCE_PATH "/org/gnome/eog/plugins/exif-display/exif-display.ui"

enum {
	PROP_O,
	PROP_DRAW_CHAN_HISTOGRAM,
	PROP_DRAW_RGB_HISTOGRAM,
	PROP_ENABLE_STATUSBAR,
	PROP_WINDOW
};

static void
eog_window_activatable_iface_init (EogWindowActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (EogExifDisplayPlugin, eog_exif_display_plugin,
                PEAS_TYPE_EXTENSION_BASE, 0,
                G_IMPLEMENT_INTERFACE_DYNAMIC(EOG_TYPE_WINDOW_ACTIVATABLE,
                                        eog_window_activatable_iface_init))

static void
eog_exif_display_plugin_init (EogExifDisplayPlugin *plugin)
{
}

/* eog_util_make_valid_utf8 is not exported so it's duped here */
gchar *
_eog_util_make_valid_utf8 (const gchar *str)
{
	GString *string;
	const char *remainder, *invalid;
	int remaining_bytes, valid_bytes;

	string = NULL;
	remainder = str;
	remaining_bytes = strlen (str);

	while (remaining_bytes != 0) {
		if (g_utf8_validate (remainder, remaining_bytes, &invalid)) {
			break;
		}

		valid_bytes = invalid - remainder;

		if (string == NULL) {
			string = g_string_sized_new (remaining_bytes);
		}

		g_string_append_len (string, remainder, valid_bytes);
		g_string_append_c (string, '?');

		remaining_bytes -= valid_bytes + 1;
		remainder = invalid + 1;
	}

	if (string == NULL) {
		return g_strdup (str);
	}

	g_string_append (string, remainder);
	g_string_append (string, _(" (invalid Unicode)"));

	g_assert (g_utf8_validate (string->str, -1, NULL));

	return g_string_free (string, FALSE);
}

/* stolen from eog-properties-dialog.c*/
static void
eog_exif_set_label (GtkWidget *w, ExifData *exif_data, gint tag_id)
{
	gchar exif_buffer[512];
	const gchar *buf_ptr;
	gchar *label_text = NULL;

	if (exif_data) {
		buf_ptr = eog_exif_data_get_value (exif_data, tag_id,
						   exif_buffer, 512);

		if (tag_id == EXIF_TAG_DATE_TIME_ORIGINAL && buf_ptr)
			label_text = eog_exif_util_format_date (buf_ptr);
		else
			label_text = _eog_util_make_valid_utf8 (buf_ptr);
	}

	gtk_label_set_text (GTK_LABEL (w), label_text);
	g_free (label_text);
}

static void set_exif_label (ExifData *exif_data, int exif_tag,
			    GtkBuilder *gtk_builder,
			    const gchar *gtk_builder_label_name,
			    gboolean tooltip)
{
	GtkWidget *widget = GTK_WIDGET (gtk_builder_get_object (
					gtk_builder, gtk_builder_label_name));
	eog_exif_set_label (widget, exif_data, exif_tag);

	if (tooltip) {
		gtk_widget_set_tooltip_text (widget, gtk_label_get_label (GTK_LABEL (widget)));
	}
}

/* stolen from eog-properties-dialog and slightly modified
 * you must g_free () the gchar* that I return!
 * */
static gchar*
eog_exif_get_focal_length_desc (ExifData *exif_data)
{
	ExifEntry *entry = NULL, *entry35mm = NULL;
	ExifByteOrder byte_order;
	gfloat f_val = 0.0;
	gchar *fl_text = NULL,*fl35_text = NULL;
	gchar *result;

	/* If no ExifData is supplied the label will be
	 * cleared later as fl35_text is NULL. */
	if (exif_data != NULL) {
		entry = exif_data_get_entry (exif_data, EXIF_TAG_FOCAL_LENGTH);
		entry35mm = exif_data_get_entry (exif_data,
					    EXIF_TAG_FOCAL_LENGTH_IN_35MM_FILM);
		byte_order = exif_data_get_byte_order (exif_data);
	}

	if (entry && G_LIKELY (entry->format == EXIF_FORMAT_RATIONAL)) {
		ExifRational value;

		/* Decode value by hand as libexif is not necessarily returning
		 * it in the format we want it to be.
		 */
		value = exif_get_rational (entry->data, byte_order);
		/* Guard against div by zero */
		if (G_LIKELY(value.denominator != 0))
			f_val = (gfloat)value.numerator/
				(gfloat)value.denominator;

		/* TRANSLATORS: This is the actual focal length used when
		   the image was taken.*/
		fl_text = g_strdup_printf (_("%.1fmm (lens)"), f_val);

	}
	if (entry35mm && G_LIKELY (entry35mm->format == EXIF_FORMAT_SHORT)) {
		ExifShort s_val;

		s_val = exif_get_short (entry35mm->data, byte_order);

		/* Print as float to get a similar look as above. */
		/* TRANSLATORS: This is the equivalent focal length assuming
		   a 35mm film camera. */
		fl35_text = g_strdup_printf(_("%.1fmm (35mm film)"),(float)s_val);
	}
	if (fl_text) {
		if (fl35_text) {
			result = g_strconcat (fl35_text,", ", fl_text, NULL);

			g_free (fl35_text);
			g_free (fl_text);

		} else {
			result = fl_text;
			g_free (fl35_text);
		}
	} else {
		result = fl35_text;
		g_free (fl_text);
	}

	return result;
}

/* stolen from eog-properties-dialog and modified */
static void
eog_exif_set_focal_length_label (GtkWidget *w, ExifData *exif_data)
{
	gchar *focal_length_desc = eog_exif_get_focal_length_desc (exif_data);
	gtk_label_set_text (GTK_LABEL (w), focal_length_desc);

	g_free (focal_length_desc);
}

static void manage_exif_data (EogExifDisplayPlugin *plugin)
{
	EogImage *image;
	ExifData *exif_data;

	image = eog_thumb_view_get_first_selected_image (plugin->thumbview);
	g_return_if_fail (image != NULL);

	exif_data = (ExifData *)eog_image_get_exif_info (image);

	set_exif_label (exif_data, EXIF_TAG_DATE_TIME_ORIGINAL, plugin->sidebar_builder, "takenon_label", TRUE);

	eog_exif_set_focal_length_label (GTK_WIDGET (gtk_builder_get_object (
			plugin->sidebar_builder, "focal_length_label")), exif_data);

	set_exif_label (exif_data, EXIF_TAG_EXPOSURE_BIAS_VALUE,
			plugin->sidebar_builder, "exposure_bias_label", FALSE);

	set_exif_label (exif_data, EXIF_TAG_EXPOSURE_TIME,
			plugin->sidebar_builder, "exposure_time_label", FALSE);

	set_exif_label (exif_data, EXIF_TAG_MODEL,
			plugin->sidebar_builder, "camera_model_label", FALSE);

	set_exif_label (exif_data, EXIF_TAG_FNUMBER,
			plugin->sidebar_builder, "aperture_label", FALSE);

	set_exif_label (exif_data, EXIF_TAG_ISO_SPEED_RATINGS,
			plugin->sidebar_builder, "iso_label", FALSE);

	set_exif_label (exif_data, EXIF_TAG_FLASH,
			plugin->sidebar_builder, "flash_label", TRUE);

	set_exif_label (exif_data, EXIF_TAG_METERING_MODE,
			plugin->sidebar_builder, "metering_mode_label", TRUE);
			
	set_exif_label (exif_data, EXIF_TAG_USER_COMMENT,
			plugin->sidebar_builder, "desc_label", TRUE);

	set_exif_label (exif_data, EXIF_TAG_EXPOSURE_BIAS_VALUE,
			plugin->sidebar_builder, "exposure_bias_label", FALSE);

	exif_data_unref (exif_data);

	g_object_unref (image);
}

static void manage_exif_data_cb (EogJob *job, gpointer data)
{
	if (!job->error) {
		manage_exif_data (EOG_EXIF_DISPLAY_PLUGIN(data));
	}
}

static gboolean
calculate_histogram (EogExifDisplayPlugin *plugin, EogImage *eog_image)
{
	int rowstride;
	int width, height;
	int row, col;
	GdkPixbuf *image_pixbuf;
	guchar *pixels;
	int array_sums_elt = 0;

	/* for the red when we calculate we store
	 * the values in a temporary array.
	 * only when everything is calculated
	 * we copy the pointers to the real
	 * plugin->histogram_values_red.
	 * That way we'll try to display
	 * the histogram only once it's fully
	 * calculated.*/
	int *histogram_values_red_temp;

	if (eog_image == NULL) {
		return FALSE;
	}

	g_free (plugin->histogram_values_red);
	plugin->histogram_values_red = NULL;

	g_free (plugin->histogram_values_green);
	plugin->histogram_values_green = NULL;

	g_free (plugin->histogram_values_blue);
	plugin->histogram_values_blue = NULL;

	g_free (plugin->histogram_values_rgb);
	plugin->histogram_values_rgb = NULL;

	image_pixbuf = eog_image_get_pixbuf (eog_image);
	if (image_pixbuf == NULL) {
		return FALSE;
	}

	if ((gdk_pixbuf_get_colorspace (image_pixbuf) != GDK_COLORSPACE_RGB)
		|| (gdk_pixbuf_get_bits_per_sample (image_pixbuf) > 8)) {
		g_object_unref (image_pixbuf);
		return FALSE;
	}

	rowstride = gdk_pixbuf_get_rowstride (image_pixbuf);

	width = gdk_pixbuf_get_width (image_pixbuf);
	height = gdk_pixbuf_get_height (image_pixbuf);

	pixels = gdk_pixbuf_get_pixels (image_pixbuf);

	histogram_values_red_temp = g_new0 (int, 256);

	plugin->histogram_values_green = g_new0 (int, 256);
	plugin->histogram_values_blue = g_new0 (int, 256);
	plugin->max_of_array_sums = 0;

	plugin->histogram_values_rgb = g_new0 (int, 256);
	plugin->max_of_array_sums_rgb = 0;

	for (row = 0; row < height; row++) {
		guchar *row_cur_idx = pixels + row*rowstride;
		for (col = 0; col < width; col++) {
			guchar red = *row_cur_idx++;
			guchar green = *row_cur_idx++;
			guchar blue = *row_cur_idx++;

			histogram_values_red_temp[red] += 1;
			plugin->histogram_values_green[green] += 1;
			plugin->histogram_values_blue[blue] += 1;
			plugin->histogram_values_rgb[MAX (red, MAX (green, blue))] += 1;
		}
	}
	for (array_sums_elt=0;array_sums_elt<256;array_sums_elt++) {
		if (histogram_values_red_temp[array_sums_elt] > plugin->max_of_array_sums) {
			plugin->max_of_array_sums = histogram_values_red_temp[array_sums_elt];
		}
		if (plugin->histogram_values_green[array_sums_elt] > plugin->max_of_array_sums) {
			plugin->max_of_array_sums = plugin->histogram_values_green[array_sums_elt];
		}
		if (plugin->histogram_values_blue[array_sums_elt] > plugin->max_of_array_sums) {
			plugin->max_of_array_sums = plugin->histogram_values_blue[array_sums_elt];
		}
	}

	for (array_sums_elt=0;array_sums_elt<256;array_sums_elt++) {
		if (plugin->histogram_values_rgb[array_sums_elt] > plugin->max_of_array_sums_rgb) {
			plugin->max_of_array_sums_rgb = plugin->histogram_values_rgb[array_sums_elt];
		}
	}

	plugin->histogram_values_red = histogram_values_red_temp;

	g_object_unref (image_pixbuf);

	return TRUE;
}

static void
draw_histogram_graph (cairo_t *cr, int *histogram_values, int max_of_array_sums)
{
	int i;

	cairo_move_to (cr, 0, 1);
	for (i = 0; i < 256; i++) {
		cairo_line_to (cr, ((float)i)/(256.0), 1.0 - ((float)histogram_values[i])/max_of_array_sums);
	}
	cairo_line_to (cr, 1, 1);
	cairo_close_path (cr);
	cairo_fill (cr);
}

static void
drawing_area_draw_cb (GtkDrawingArea *drawing_area, cairo_t *cr,
		     EogExifDisplayPlugin *plugin)
{
	gboolean draw_channels_histogram, draw_rgb_histogram;
	EogImage *eog_image;
	gint drawing_area_width, drawing_area_height;
	int scale_factor_y;
	GtkStyleContext *style_ctx;

	if (!gtk_widget_get_realized (GTK_WIDGET (drawing_area)))
		return;

	draw_channels_histogram = plugin->draw_chan_histogram;
	draw_rgb_histogram = plugin->draw_rgb_histogram;

	eog_image = eog_thumb_view_get_first_selected_image (plugin->thumbview);
	g_return_if_fail (eog_image != NULL);

	if (plugin->histogram_values_red == NULL) {
		/* when calculate_histogram was called previously,
		 * the picture was not loaded yet.
		 * Now it's loaded, let's ask to calculate the
		 * histogram again... */
		calculate_histogram (plugin, eog_image);
	}

	drawing_area_width = gtk_widget_get_allocated_width (GTK_WIDGET (drawing_area));
	drawing_area_height = gtk_widget_get_allocated_height (GTK_WIDGET (drawing_area));

	scale_factor_y = drawing_area_height;
	if (scale_factor_y > drawing_area_width/2) {
		/* histogram taller than it is wide looks ugly.
		 * it must be wider than it is tall for aesthetics.
		 */
		scale_factor_y = drawing_area_width/2;
	}
	cairo_scale (cr, drawing_area_width, scale_factor_y);

	/* clear the display */
	style_ctx = gtk_widget_get_style_context (GTK_WIDGET (drawing_area));
	gtk_render_background (style_ctx, cr, 0, 0,
			       drawing_area_width, drawing_area_height);

	if (plugin->histogram_values_red == NULL) {
		/* it's possible, if the image
		 * is not loaded and histogram
		 * can't be calculated, we go this
		 * far to clear the display.
		 * now exit, we won't draw any
		 * histogram without the data.
		 */
		return;
	}

	if (draw_channels_histogram) {
		cairo_set_source_rgba (cr, 1, 0, 0, 0.5);
		draw_histogram_graph (cr, plugin->histogram_values_red,
				      plugin->max_of_array_sums);

		cairo_set_source_rgba (cr, 0, 1, 0, 0.5);
		draw_histogram_graph (cr, plugin->histogram_values_green,
				      plugin->max_of_array_sums);

		cairo_set_source_rgba (cr, 0, 0, 1, 0.5);
		draw_histogram_graph (cr, plugin->histogram_values_blue,
				      plugin->max_of_array_sums);
	}
	if (draw_rgb_histogram) {
		cairo_set_source_rgba (cr, 0, 0, 0, 0.5);
		draw_histogram_graph (cr, plugin->histogram_values_rgb,
				      plugin->max_of_array_sums_rgb);
	}

	g_object_unref (eog_image);
}

static void calculate_histogram_cb (EogJob *job, gpointer data)
{
	EogExifDisplayPlugin *plugin = EOG_EXIF_DISPLAY_PLUGIN (data);

	if (!job->error) {
		EogImage *eog_image =
			eog_thumb_view_get_first_selected_image (plugin->thumbview);
		calculate_histogram (plugin, eog_image);
		g_object_unref (eog_image);
		gtk_widget_queue_draw (GTK_WIDGET (plugin->drawing_area));
	}
}

static void
statusbar_update_exif_data (GtkStatusbar *statusbar, EogThumbView *view)
{
	EogImage *image;
	ExifData *exif_data;
	gchar *exif_desc = NULL;

	if (eog_thumb_view_get_n_selected (view) == 0)
		return;

	image = eog_thumb_view_get_first_selected_image (view);

	gtk_statusbar_pop (statusbar, 0);

	if (!eog_image_has_data (image, EOG_IMAGE_DATA_EXIF)) {
		if (!eog_image_load (image, EOG_IMAGE_DATA_EXIF, NULL, NULL)) {
			gtk_widget_hide (GTK_WIDGET (statusbar));
		}
	}

	exif_data = (ExifData *) eog_image_get_exif_info (image);
	if (exif_data) {
		ExifEntry *exif_entry;
		gchar exposition_time[512];
		gchar aperture[512];
		gchar iso[512];
		gchar *focal_length;

		exposition_time[0] = 0;
		exif_entry = exif_data_get_entry (exif_data, EXIF_TAG_EXPOSURE_TIME);
		exif_entry_get_value (exif_entry, exposition_time, sizeof(exposition_time));

		aperture[0] = 0;
		exif_entry = exif_data_get_entry (exif_data, EXIF_TAG_FNUMBER);
		exif_entry_get_value (exif_entry, aperture, sizeof(aperture));

		iso[0] = 0;
		exif_entry = exif_data_get_entry (exif_data, EXIF_TAG_ISO_SPEED_RATINGS);
		exif_entry_get_value (exif_entry, iso, sizeof(iso));

		focal_length = eog_exif_get_focal_length_desc (exif_data);

		exif_desc = g_strdup_printf ("ISO%s  %s  %s  %s",
				iso, exposition_time, aperture, focal_length);

		g_free (focal_length);

		exif_data_unref (exif_data);
	}
	g_object_unref (image);

	if (exif_desc) {
		gtk_statusbar_push (statusbar, 0, exif_desc);
		gtk_widget_show (GTK_WIDGET (statusbar));
		g_free (exif_desc);
	} else {
		gtk_widget_hide (GTK_WIDGET (statusbar));
	}
}


static void
selection_changed_cb (EogThumbView *view, EogExifDisplayPlugin *plugin)
{
	EogImage *image;

	if (!eog_thumb_view_get_n_selected (view)) {
		return;
	}

	image = eog_thumb_view_get_first_selected_image (view);
	g_return_if_fail (image != NULL);

	if (plugin->enable_statusbar) {
		statusbar_update_exif_data (GTK_STATUSBAR (plugin->statusbar_exif), view);
	}

	if (!eog_image_has_data (image, EOG_IMAGE_DATA_EXIF)) {
		EogJob *job;

		job = eog_job_load_new (image, EOG_IMAGE_DATA_EXIF);
		g_signal_connect (G_OBJECT (job), "finished",
				  G_CALLBACK (manage_exif_data_cb),
				  plugin);
		eog_job_scheduler_add_job (job);
		g_object_unref (job);
	} else {
		manage_exif_data (plugin);
	}

	/* the selected image changed, the histogram must
	 * be recalculated. */
	if (!eog_image_has_data (image, EOG_IMAGE_DATA_IMAGE)) {
		EogJob *job;

		job = eog_job_load_new (image, EOG_IMAGE_DATA_IMAGE);
		g_signal_connect (G_OBJECT (job), "finished",
				  G_CALLBACK (calculate_histogram_cb),
				  plugin);
		eog_job_scheduler_add_job (job);
		g_object_unref (job);
	}

	g_object_unref (image);
}

static void
remove_statusbar_entry (EogExifDisplayPlugin *plugin)
{
	GtkWidget *statusbar = eog_window_get_statusbar (plugin->window);

	if (plugin->statusbar_exif == NULL) {
		return;
	}
	gtk_container_remove (GTK_CONTAINER (statusbar),
			      plugin->statusbar_exif);
	plugin->statusbar_exif = NULL;
}

static void
setup_statusbar_exif (EogExifDisplayPlugin *plugin)
{
	GtkWidget *statusbar = eog_window_get_statusbar (plugin->window);

	if (plugin->enable_statusbar) {
		plugin->statusbar_exif = gtk_statusbar_new ();
		gtk_widget_set_size_request (plugin->statusbar_exif, 280, 10);
		gtk_box_pack_end (GTK_BOX (statusbar),
				  plugin->statusbar_exif,
				  FALSE, FALSE, 0);

		statusbar_update_exif_data (GTK_STATUSBAR (plugin->statusbar_exif), plugin->thumbview);
	}
	else {
		remove_statusbar_entry (plugin);
	}
}

static void
impl_activate (EogWindowActivatable *activatable)
{
	EogExifDisplayPlugin *plugin = EOG_EXIF_DISPLAY_PLUGIN (activatable);
	EogWindow *window = plugin->window;
	GSettings *settings;
	GtkWidget *thumbview;
	GtkWidget *sidebar;
	GtkWidget *drawing_area;
	GError* error = NULL;

	settings = g_settings_new (EOG_EXIF_DISPLAY_CONF_SCHEMA_ID);

	thumbview = eog_window_get_thumb_view (window);
	plugin->thumbview = EOG_THUMB_VIEW (thumbview);

	plugin->histogram_values_red = NULL;
	plugin->histogram_values_green = NULL;
	plugin->histogram_values_blue = NULL;
	plugin->histogram_values_rgb = NULL;

	plugin->statusbar_exif = NULL;

	plugin->selection_changed_id = g_signal_connect (G_OBJECT (thumbview),
					"selection-changed",
					G_CALLBACK (selection_changed_cb),
					plugin);

	sidebar = eog_window_get_sidebar (window);

	plugin->sidebar_builder = gtk_builder_new ();
	gtk_builder_set_translation_domain (plugin->sidebar_builder,
					    GETTEXT_PACKAGE);
	if (!gtk_builder_add_from_resource (plugin->sidebar_builder,
					    GRESOURCE_PATH, &error))
	{
		g_warning ("Couldn't load UI resource: %s", error->message);
		g_error_free (error);
	}
	plugin->gtkbuilder_widget = GTK_WIDGET (gtk_builder_get_object (plugin->sidebar_builder, "viewport1"));

	drawing_area = GTK_WIDGET (gtk_builder_get_object (plugin->sidebar_builder, "drawingarea1"));
	g_signal_connect (drawing_area, "draw",
			  G_CALLBACK (drawing_area_draw_cb), plugin);
	plugin->drawing_area = GTK_DRAWING_AREA (drawing_area);

	eog_sidebar_add_page (EOG_SIDEBAR (sidebar), "Details",
			      plugin->gtkbuilder_widget);
	gtk_widget_show_all (plugin->gtkbuilder_widget);

	g_settings_bind (settings, EOG_EXIF_DISPLAY_CONF_CHANNELS_HISTOGRAM,
			 plugin, "draw-chan-histogram", G_SETTINGS_BIND_GET);
	g_settings_bind (settings, EOG_EXIF_DISPLAY_CONF_RGB_HISTOGRAM,
			 plugin, "draw-rgb-histogram", G_SETTINGS_BIND_GET);
	g_settings_bind (settings, EOG_EXIF_DISPLAY_CONF_EXIF_IN_STATUSBAR,
			 plugin, "enable-statusbar", G_SETTINGS_BIND_GET);

	setup_statusbar_exif (plugin);

	/* force display of data now */
	selection_changed_cb (plugin->thumbview, plugin);
	if (plugin->enable_statusbar)
	{
		statusbar_update_exif_data (GTK_STATUSBAR (plugin->statusbar_exif),
					    EOG_THUMB_VIEW (thumbview));
	}

	g_object_unref (settings);
}

static void
impl_deactivate	(EogWindowActivatable *activatable)
{
	EogExifDisplayPlugin *plugin = EOG_EXIF_DISPLAY_PLUGIN (activatable);
	GtkWidget *sidebar, *thumbview;

	remove_statusbar_entry (plugin);

	sidebar = eog_window_get_sidebar (plugin->window);
	eog_sidebar_remove_page(EOG_SIDEBAR (sidebar),
				plugin->gtkbuilder_widget);

	thumbview = eog_window_get_thumb_view (plugin->window);
	g_signal_handler_disconnect (thumbview, plugin->selection_changed_id);

	g_free (plugin->histogram_values_red);
	plugin->histogram_values_red = NULL;
	g_free (plugin->histogram_values_green);
	plugin->histogram_values_green = NULL;
	g_free (plugin->histogram_values_blue);
	plugin->histogram_values_blue = NULL;
	g_free (plugin->histogram_values_rgb);
	plugin->histogram_values_rgb = NULL;

	g_object_unref (plugin->sidebar_builder);
	plugin->sidebar_builder = NULL;
}


static void
eog_exif_display_plugin_set_draw_chan_histogram (EogExifDisplayPlugin *plugin,
						 gboolean value)
{
	if (plugin->draw_chan_histogram == value)
		return;

	plugin->draw_chan_histogram = value;

	gtk_widget_queue_draw (GTK_WIDGET (plugin->drawing_area));

	g_object_notify (G_OBJECT (plugin), "draw-chan-histogram");
}

static void
eog_exif_display_plugin_set_draw_rgb_histogram (EogExifDisplayPlugin *plugin,
						gboolean value)
{
	if (plugin->draw_rgb_histogram == value)
		return;

	plugin->draw_rgb_histogram = value;

	gtk_widget_queue_draw (GTK_WIDGET (plugin->drawing_area));

	g_object_notify (G_OBJECT (plugin), "draw-rgb-histogram");
}

static void
eog_exif_display_plugin_enable_statusbar (EogExifDisplayPlugin *plugin,
					  gboolean value)
{
	if (plugin->enable_statusbar == value)
		return;

	plugin->enable_statusbar = value;

	setup_statusbar_exif (plugin);

	g_object_notify (G_OBJECT (plugin), "enable-statusbar");
}

static void
eog_exif_display_plugin_get_property (GObject    *object,
				      guint       prop_id,
				      GValue     *value,
				      GParamSpec *pspec)
{
	EogExifDisplayPlugin *plugin = EOG_EXIF_DISPLAY_PLUGIN (object);

	switch (prop_id)
	{
	case PROP_DRAW_CHAN_HISTOGRAM:
		g_value_set_boolean (value, plugin->draw_chan_histogram);
		break;
	case PROP_DRAW_RGB_HISTOGRAM:
		g_value_set_boolean (value, plugin->draw_rgb_histogram);
		break;
	case PROP_ENABLE_STATUSBAR:
		g_value_set_boolean (value, plugin->enable_statusbar);
		break;
	case PROP_WINDOW:
		g_value_set_object (value, plugin->window);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
eog_exif_display_plugin_set_property (GObject      *object,
				      guint         prop_id,
				      const GValue *value,
				      GParamSpec   *pspec)
{
	EogExifDisplayPlugin *plugin = EOG_EXIF_DISPLAY_PLUGIN (object);

	switch (prop_id)
	{
	case PROP_DRAW_CHAN_HISTOGRAM:
		eog_exif_display_plugin_set_draw_chan_histogram (plugin,
						g_value_get_boolean (value));	
		break;
	case PROP_DRAW_RGB_HISTOGRAM:
		eog_exif_display_plugin_set_draw_rgb_histogram (plugin,
						g_value_get_boolean (value));
		break;
	case PROP_ENABLE_STATUSBAR:
		eog_exif_display_plugin_enable_statusbar (plugin,
						g_value_get_boolean (value));
		break;
	case PROP_WINDOW:
		plugin->window = EOG_WINDOW (g_value_dup_object (value));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
eog_exif_display_plugin_dispose (GObject *object)
{
	EogExifDisplayPlugin *plugin = EOG_EXIF_DISPLAY_PLUGIN (object);

	eog_debug_message (DEBUG_PLUGINS, "EogPostrPlugin disposing");

	if (plugin->window != NULL) {
		g_object_unref (plugin->window);
		plugin->window = NULL;
	}

	G_OBJECT_CLASS (eog_exif_display_plugin_parent_class)->dispose (object);
}
static void
eog_exif_display_plugin_class_init (EogExifDisplayPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = eog_exif_display_plugin_dispose;
	object_class->set_property = eog_exif_display_plugin_set_property;
	object_class->get_property = eog_exif_display_plugin_get_property;

	g_object_class_install_property (object_class, PROP_DRAW_CHAN_HISTOGRAM,
		g_param_spec_boolean ("draw-chan-histogram", NULL, NULL, FALSE,
				      G_PARAM_READWRITE | G_PARAM_STATIC_NAME));

	g_object_class_install_property (object_class, PROP_DRAW_RGB_HISTOGRAM,
		g_param_spec_boolean ("draw-rgb-histogram", NULL, NULL, FALSE,
				      G_PARAM_READWRITE | G_PARAM_STATIC_NAME));

	g_object_class_install_property (object_class, PROP_ENABLE_STATUSBAR,
		g_param_spec_boolean ("enable-statusbar", NULL, NULL, FALSE,
				      G_PARAM_READWRITE | G_PARAM_STATIC_NAME));

	g_object_class_override_property (object_class, PROP_WINDOW, "window");
}

static void
eog_window_activatable_iface_init (EogWindowActivatableInterface *iface)
{
        iface->activate = impl_activate;
        iface->deactivate = impl_deactivate;
}

static void
eog_exif_display_plugin_class_finalize (EogExifDisplayPluginClass *klass)
{
	/* Dummy needed for G_DEFINE_DYNAMIC_TYPE_EXTENDED */
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
        eog_exif_display_plugin_register_type (G_TYPE_MODULE (module));
        peas_object_module_register_extension_type (module,
                                                    EOG_TYPE_WINDOW_ACTIVATABLE,
                                                    EOG_TYPE_EXIF_DISPLAY_PLUGIN);

	eog_exif_display_plugin_setup_register_types (module);
}
