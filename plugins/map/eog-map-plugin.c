#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#define HAVE_EXIF 1

#include "eog-map-plugin.h"

#include <gmodule.h>
#include <glib/gi18n-lib.h>

#include <eog/eog-exif-util.h>
#include <eog/eog-debug.h>
#include <eog/eog-thumb-view.h>
#include <eog/eog-image.h>
#include <eog/eog-sidebar.h>
#include <eog/eog-window.h>
#include <eog/eog-window-activatable.h>

#include <libpeas/peas.h>

#include <math.h>
#include <string.h>
#include <champlain/champlain.h>
#include <champlain-gtk/champlain-gtk.h>
#include <clutter-gtk/clutter-gtk.h>
#include <libexif/exif-data.h>
#include <libexif/exif-tag.h>

enum {
        PROP_0,
        PROP_WINDOW
};

static void
eog_window_activatable_iface_init (EogWindowActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (EogMapPlugin, eog_map_plugin,
                PEAS_TYPE_EXTENSION_BASE, 0,
                G_IMPLEMENT_INTERFACE_DYNAMIC (EOG_TYPE_WINDOW_ACTIVATABLE,
                                        eog_window_activatable_iface_init))

#define FACTOR 2.0

static void
eog_map_plugin_init (EogMapPlugin *plugin)
{
	eog_debug_message (DEBUG_PLUGINS, "EogMapPlugin initializing");
	gtk_clutter_init (NULL, NULL);
}

static void
eog_map_plugin_finalize (GObject *object)
{
	eog_debug_message (DEBUG_PLUGINS, "EogMapPlugin finalizing");

	G_OBJECT_CLASS (eog_map_plugin_parent_class)->finalize (object);
}

static void
update_marker_image (ChamplainLabel *marker,
		     GtkIconSize size)
{
	GtkWidget *widget;
	ClutterActor *thumb;

	widget = gtk_button_new ();
	thumb = gtk_clutter_texture_new ();
	gtk_clutter_texture_set_from_icon_name (GTK_CLUTTER_TEXTURE (thumb),
						widget,
						"gnome-mime-image",
						size, NULL);
	/* don't need to unref widget because it is floating */

	champlain_label_set_image (marker, thumb);
}

#define MAP_EXIF_ENTRY_IS_GPS_RATIONAL(e) ( e && \
	 (e->format == EXIF_FORMAT_RATIONAL) && \
	 (e->components == 3) && \
	 (exif_entry_get_ifd(e) == EXIF_IFD_GPS) )

static gboolean
parse_exif_gps_coordinate (ExifEntry *entry,
			   gdouble *co,
			   ExifByteOrder byte_order)
{
	gsize val_size;
	ExifRational val;
	gdouble hour = 0, min = 0, sec = 0;

	if (G_UNLIKELY (!MAP_EXIF_ENTRY_IS_GPS_RATIONAL (entry)))
		return FALSE;

	val_size = exif_format_get_size (EXIF_FORMAT_RATIONAL);

	val = exif_get_rational (entry->data, byte_order);
	if (val.denominator != 0)
		hour = (gdouble) val.numerator /
		       (gdouble) val.denominator;

	val = exif_get_rational (entry->data + val_size, byte_order);
	if (val.denominator != 0)
		min = (gdouble) val.numerator /
		      (gdouble) val.denominator;

	val = exif_get_rational (entry->data + (2 * val_size), byte_order);
	if (val.denominator != 0)
		sec = (gdouble) val.numerator /
		      (gdouble) val.denominator;

	if (G_LIKELY (co != NULL)) {
		*co = hour + (min / 60.0) + (sec / 3600.0);
	}

	return TRUE;
}

static gboolean
get_coordinates (EogImage *image,
		 gdouble *latitude,
		 gdouble *longitude)
{
	ExifData *exif_data;
	gchar buffer[32];
	gdouble lon, lat;

	exif_data = (ExifData *) eog_image_get_exif_info (image);

	if (exif_data) {
		ExifEntry *entry;
		ExifByteOrder byte_order;

		byte_order = exif_data_get_byte_order (exif_data);
		entry = exif_data_get_entry (exif_data,
					     EXIF_TAG_GPS_LONGITUDE);

		if (!parse_exif_gps_coordinate (entry, &lon, byte_order)
		    || (lon > 180.0)) {
			exif_data_unref (exif_data);
			return FALSE;
		}


		eog_exif_data_get_value (exif_data,
					 EXIF_TAG_GPS_LONGITUDE_REF,
					 buffer,
					 32);
		if (strcmp (buffer, "W") == 0)
			lon *= -1;

		entry = exif_data_get_entry (exif_data,
					     EXIF_TAG_GPS_LATITUDE);

		if (!parse_exif_gps_coordinate (entry, &lat, byte_order)
		    || (lat > 90.0)) {
			exif_data_unref (exif_data);
			return FALSE;
		}

		eog_exif_data_get_value (exif_data,
					 EXIF_TAG_GPS_LATITUDE_REF,
					 buffer,
					 32);
		if (strcmp (buffer, "S") == 0)
			lat *= -1;

		*longitude = lon;
		*latitude = lat;

		exif_data_unref (exif_data);
		return TRUE;
	}
	return FALSE;
}

static gboolean
change_image (ChamplainLabel *marker,
	      ClutterEvent *event,
	      EogMapPlugin *plugin)
{
	EogImage *image;

	image = g_object_get_data (G_OBJECT (marker), "image");

	if (!image)
		return FALSE;

	eog_thumb_view_set_current_image (EOG_THUMB_VIEW (plugin->thumbview),
					  image, TRUE);

	return FALSE;
}

static void
create_marker (EogImage *image,
	       EogMapPlugin *plugin)
{
	gdouble lon, lat;

	if (!image)
		return;

	if (!eog_image_has_data (image, EOG_IMAGE_DATA_EXIF) &&
	    !eog_image_load (image, EOG_IMAGE_DATA_EXIF, NULL, NULL))
		return;

	if (get_coordinates (image, &lat, &lon)) {
		ChamplainLabel *marker;

		marker = CHAMPLAIN_LABEL (champlain_label_new ());
		champlain_label_set_draw_background (CHAMPLAIN_LABEL (marker), FALSE);
		update_marker_image (marker, GTK_ICON_SIZE_MENU);

		g_object_set_data_full (G_OBJECT (image), "marker", marker, (GDestroyNotify) clutter_actor_destroy);
		g_object_set_data (G_OBJECT (marker), "image", image);

		champlain_location_set_location (CHAMPLAIN_LOCATION (marker),
						    lat,
						    lon);
		champlain_marker_layer_add_marker (plugin->layer, CHAMPLAIN_MARKER (marker));

		g_signal_connect (marker,
				  "button-release-event",
				  G_CALLBACK (change_image),
				  plugin);
	}

}

static void
selection_changed_cb (EogThumbView *view,
		      EogMapPlugin *plugin)
{
	EogImage *image;
	ChamplainLabel *marker;

	if (!eog_thumb_view_get_n_selected (view))
		return;

	image = eog_thumb_view_get_first_selected_image (view);

	g_return_if_fail (image != NULL);

	marker = g_object_get_data (G_OBJECT (image), "marker");

	if (marker) {
		gdouble lat, lon;

		g_object_get (marker,
			      "latitude", &lat,
			      "longitude", &lon,
			      NULL);

		champlain_view_center_on (CHAMPLAIN_VIEW (plugin->map),
					  lat,
					  lon);

		/* Reset the previous selection */
		if (plugin->marker)
			update_marker_image (plugin->marker, GTK_ICON_SIZE_MENU);

		plugin->marker = marker;
		update_marker_image (plugin->marker, GTK_ICON_SIZE_LARGE_TOOLBAR);
		gtk_widget_set_sensitive (plugin->jump_to_button, TRUE);
	}
	else {
		gtk_widget_set_sensitive (plugin->jump_to_button, FALSE);

		/* Reset the previous selection */
		if (plugin->marker)
			update_marker_image (plugin->marker, GTK_ICON_SIZE_MENU);

		plugin->marker = NULL;
	}

	g_object_unref (image);
}

static void
jump_to (GtkWidget *widget,
	 EogMapPlugin *plugin)
{
	if (!plugin->marker)
		return;

	gdouble lat, lon;

	g_object_get (plugin->marker,
		      "latitude", &lat,
		      "longitude", &lon,
		      NULL);

	champlain_view_center_on (CHAMPLAIN_VIEW (plugin->map),
				  lat,
				  lon);
}

static void
zoom_in (GtkWidget *widget,
	 ChamplainView *view)
{
	champlain_view_zoom_in (view);
}

static void
zoom_out (GtkWidget *widget,
	  ChamplainView *view)
{
	champlain_view_zoom_out (view);
}

static gboolean
for_each_thumb (GtkTreeModel *model,
		GtkTreePath *path,
		GtkTreeIter *iter,
		EogMapPlugin *plugin)
{
	EogImage *image = NULL;

	gtk_tree_model_get (model, iter,
			    EOG_LIST_STORE_EOG_IMAGE, &image,
			    -1);

	if (!image) {
		return FALSE;
	}

	create_marker (image, plugin);

	g_object_unref (image);
	return FALSE;
}

static void
prepared_cb (EogWindow *window,
	     EogMapPlugin *plugin)
{

	plugin->store = eog_window_get_store (plugin->window);

	if (!plugin->store)
		return;

	if (plugin->win_prepared_id > 0) {
		g_signal_handler_disconnect (window, plugin->win_prepared_id);
		plugin->win_prepared_id = 0;
	}

	/* At this point, the collection has been filled */
	gtk_tree_model_foreach (GTK_TREE_MODEL (plugin->store),
				(GtkTreeModelForeachFunc) for_each_thumb,
				plugin);

	plugin->thumbview = eog_window_get_thumb_view (window);
	plugin->selection_changed_id = g_signal_connect (G_OBJECT (plugin->thumbview),
						       "selection-changed",
						       G_CALLBACK (selection_changed_cb),
						       plugin);

	/* Call the callback because if the plugin is activated after
	 *  the image is loaded, selection_changed isn't emited
	 */
	selection_changed_cb (EOG_THUMB_VIEW (plugin->thumbview), plugin);

}

static void
impl_activate (EogWindowActivatable *activatable)
{
	EogMapPlugin *plugin = EOG_MAP_PLUGIN (activatable);
	GtkWidget *sidebar, *vbox, *bbox, *button, *viewport;
	GtkWidget *embed;
	ClutterActor *scale;

	eog_debug (DEBUG_PLUGINS);

	/* This is a workaround until bug 590692 is fixed. */
	viewport = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (viewport), GTK_SHADOW_ETCHED_IN);
	/*viewport = gtk_viewport_new (NULL, NULL);
	gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport),
				      GTK_SHADOW_ETCHED_IN);*/

	embed = gtk_champlain_embed_new ();
	plugin->map = gtk_champlain_embed_get_view (GTK_CHAMPLAIN_EMBED (embed));
	g_object_set (G_OBJECT (plugin->map),
		"zoom-level", 3,
		"kinetic-mode", TRUE,
		NULL);
	scale = champlain_scale_new ();
	champlain_scale_connect_view (CHAMPLAIN_SCALE (scale), plugin->map);
	/* align to the bottom left */
	champlain_view_bin_layout_add (plugin->map, scale,
		CLUTTER_BIN_ALIGNMENT_START,
		CLUTTER_BIN_ALIGNMENT_END);

	gtk_container_add (GTK_CONTAINER (viewport), embed);

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	bbox = gtk_toolbar_new ();

	button = GTK_WIDGET (gtk_tool_button_new_from_stock (GTK_STOCK_JUMP_TO));
	gtk_widget_set_tooltip_text (button, _("Jump to current image's location"));
	g_signal_connect (button,
			  "clicked",
			  G_CALLBACK (jump_to),
			  plugin);
	gtk_container_add (GTK_CONTAINER (bbox), button);
	plugin->jump_to_button = button;

	button = GTK_WIDGET (gtk_separator_tool_item_new ());
	gtk_container_add (GTK_CONTAINER (bbox), button);

	button = GTK_WIDGET (gtk_tool_button_new_from_stock (GTK_STOCK_ZOOM_IN));
	gtk_widget_set_tooltip_text (button, _("Zoom in"));
	g_signal_connect (button,
			  "clicked",
			  G_CALLBACK (zoom_in),
			  plugin->map);
	gtk_container_add (GTK_CONTAINER (bbox), button);

	button = GTK_WIDGET (gtk_tool_button_new_from_stock (GTK_STOCK_ZOOM_OUT));
	gtk_widget_set_tooltip_text (button, _("Zoom out"));
	g_signal_connect (button,
			  "clicked",
			  G_CALLBACK (zoom_out),
			  plugin->map);
	gtk_container_add (GTK_CONTAINER (bbox), button);

	plugin->layer = champlain_marker_layer_new_full (CHAMPLAIN_SELECTION_SINGLE);
	champlain_view_add_layer (CHAMPLAIN_VIEW (plugin->map), CHAMPLAIN_LAYER (plugin->layer));

	sidebar = eog_window_get_sidebar (plugin->window);
	plugin->viewport = vbox;
	gtk_box_pack_start (GTK_BOX (vbox), bbox, FALSE, FALSE, 0);
	gtk_widget_set_vexpand (viewport, TRUE);
	gtk_container_add (GTK_CONTAINER (vbox), viewport);
	eog_sidebar_add_page (EOG_SIDEBAR (sidebar), _("Map"), vbox);
	gtk_widget_show_all (vbox);

	plugin->win_prepared_id = g_signal_connect (G_OBJECT (plugin->window),
						    "prepared",
						    G_CALLBACK (prepared_cb),
						    plugin);
	/* Call the callback once in case the window is already ready */
	prepared_cb (plugin->window, plugin);
}

static void
impl_deactivate (EogWindowActivatable *activatable)
{
	EogMapPlugin *plugin = EOG_MAP_PLUGIN (activatable);
	GtkWidget *sidebar, *thumbview;

	eog_debug (DEBUG_PLUGINS);

	sidebar = eog_window_get_sidebar (plugin->window);
	eog_sidebar_remove_page (EOG_SIDEBAR (sidebar), plugin->viewport);

	thumbview = eog_window_get_thumb_view (plugin->window);
	if (thumbview && plugin->selection_changed_id > 0) {
		g_signal_handler_disconnect (thumbview,
					     plugin->selection_changed_id);
		plugin->selection_changed_id = 0;
	}

	if (plugin->window && plugin->win_prepared_id > 0) {
		g_signal_handler_disconnect (plugin->window,
					     plugin->win_prepared_id);
		plugin->win_prepared_id = 0;
	}

}

static void
eog_map_plugin_dispose (GObject *object)
{
        EogMapPlugin *plugin = EOG_MAP_PLUGIN (object);

        if (plugin->window != NULL) {
                g_object_unref (plugin->window);
                plugin->window = NULL;
        }

        G_OBJECT_CLASS (eog_map_plugin_parent_class)->dispose (object);
}

static void
eog_map_plugin_get_property (GObject    *object,
			     guint       prop_id,
			     GValue     *value,
			     GParamSpec *pspec)
{
        EogMapPlugin *plugin = EOG_MAP_PLUGIN (object);

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
eog_map_plugin_set_property (GObject      *object,
			     guint         prop_id,
			     const GValue *value,
			     GParamSpec   *pspec)
{
        EogMapPlugin *plugin = EOG_MAP_PLUGIN (object);

        switch (prop_id)
        {
        case PROP_WINDOW:
                plugin->window = EOG_WINDOW (g_value_dup_object (value));
                break;

        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                break;
        }
}

static void
eog_map_plugin_class_init (EogMapPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = eog_map_plugin_finalize;
        object_class->dispose = eog_map_plugin_dispose;
        object_class->set_property = eog_map_plugin_set_property;
        object_class->get_property = eog_map_plugin_get_property;

        g_object_class_override_property (object_class, PROP_WINDOW, "window");
}

static void
eog_map_plugin_class_finalize (EogMapPluginClass *klass)
{
        /* Dummy needed for G_DEFINE_DYNAMIC_TYPE_EXTENDED */
}

static void
eog_window_activatable_iface_init (EogWindowActivatableInterface *iface)
{
        iface->activate = impl_activate;
        iface->deactivate = impl_deactivate;
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
        eog_map_plugin_register_type (G_TYPE_MODULE (module));
        peas_object_module_register_extension_type (module,
                                                    EOG_TYPE_WINDOW_ACTIVATABLE,
                                                    EOG_TYPE_MAP_PLUGIN);
}
