#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#define HAVE_EXIF 1

#include "xviewer-map-plugin.h"

#include <gmodule.h>
#include <glib/gi18n-lib.h>

#include <xviewer/xviewer-exif-util.h>
#include <xviewer/xviewer-debug.h>
#include <xviewer/xviewer-thumb-view.h>
#include <xviewer/xviewer-image.h>
#include <xviewer/xviewer-sidebar.h>
#include <xviewer/xviewer-window.h>
#include <xviewer/xviewer-window-activatable.h>

#include <libpeas/peas.h>

#include <math.h>
#include <string.h>
#include <osm-gps-map.h>
#include <libexif/exif-data.h>
#include <libexif/exif-tag.h>

enum {
        PROP_0,
        PROP_WINDOW
};

static void
xviewer_window_activatable_iface_init (XviewerWindowActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (XviewerMapPlugin, xviewer_map_plugin,
                PEAS_TYPE_EXTENSION_BASE, 0,
                G_IMPLEMENT_INTERFACE_DYNAMIC (XVIEWER_TYPE_WINDOW_ACTIVATABLE,
                                        xviewer_window_activatable_iface_init))

#define MARKER_ICON_SMALL 16
#define MARKER_ICON_LARGE 24
#define MARKER_HIT_RADIUS_PX 16

static void
xviewer_map_plugin_init (XviewerMapPlugin *plugin)
{
	xviewer_debug_message (DEBUG_PLUGINS, "XviewerMapPlugin initializing");
}

static void
xviewer_map_plugin_finalize (GObject *object)
{
	xviewer_debug_message (DEBUG_PLUGINS, "XviewerMapPlugin finalizing");

	G_OBJECT_CLASS (xviewer_map_plugin_parent_class)->finalize (object);
}

static GdkPixbuf *
load_marker_pixbuf (gint size)
{
	GtkIconTheme *theme = gtk_icon_theme_get_default ();
	GdkPixbuf *pixbuf = NULL;

	pixbuf = gtk_icon_theme_load_icon (theme, "mark-location", size,
	                                   GTK_ICON_LOOKUP_FORCE_SIZE, NULL);
	if (pixbuf == NULL) {
		pixbuf = gtk_icon_theme_load_icon (theme, "image-x-generic",
		                                   size,
		                                   GTK_ICON_LOOKUP_FORCE_SIZE,
		                                   NULL);
	}

	if (pixbuf == NULL) {
		g_warning ("Could not load icon for map marker. "
		           "Please install a suitable icon theme!");
	}

	return pixbuf;
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
get_coordinates (XviewerImage *image,
		 gdouble *latitude,
		 gdouble *longitude)
{
	ExifData *exif_data;
	gchar buffer[32];
	gdouble lon, lat;

	exif_data = (ExifData *) xviewer_image_get_exif_info (image);

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


		xviewer_exif_data_get_value (exif_data,
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

		xviewer_exif_data_get_value (exif_data,
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

static void
get_marker_coords (OsmGpsMapImage *marker,
                   gdouble *lat,
                   gdouble *lon)
{
	const OsmGpsMapPoint *pt;
	float flat = 0.0f, flon = 0.0f;

	pt = osm_gps_map_image_get_point (marker);
	if (pt) {
		osm_gps_map_point_get_degrees ((OsmGpsMapPoint *) pt,
		                               &flat, &flon);
	}
	*lat = (gdouble) flat;
	*lon = (gdouble) flon;
}

static OsmGpsMapImage *
replace_marker_pixbuf (XviewerMapPlugin *plugin,
                       OsmGpsMapImage   *marker,
                       GdkPixbuf        *pixbuf)
{
	XviewerImage *image;
	OsmGpsMapImage *new_marker;
	gdouble lat, lon;

	if (!marker || !pixbuf)
		return marker;

	image = g_object_get_data (G_OBJECT (marker), "image");
	get_marker_coords (marker, &lat, &lon);

	plugin->markers = g_list_remove (plugin->markers, marker);
	if (image)
		g_object_set_data (G_OBJECT (image), "marker", NULL);
	osm_gps_map_image_remove (plugin->map, marker);

	new_marker = osm_gps_map_image_add_with_alignment (plugin->map,
	                                                   (float) lat,
	                                                   (float) lon,
	                                                   pixbuf,
	                                                   0.5f, 1.0f);
	if (image) {
		g_object_set_data (G_OBJECT (new_marker), "image", image);
		g_object_set_data_full (G_OBJECT (image), "marker",
		                        new_marker, NULL);
	}
	plugin->markers = g_list_prepend (plugin->markers, new_marker);

	return new_marker;
}

static OsmGpsMapImage *
find_marker_at_event (XviewerMapPlugin *plugin,
                      GdkEventButton   *event)
{
	GList *l;
	gint best_dist2 = MARKER_HIT_RADIUS_PX * MARKER_HIT_RADIUS_PX;
	OsmGpsMapImage *best = NULL;

	for (l = plugin->markers; l != NULL; l = l->next) {
		OsmGpsMapImage *m = l->data;
		const OsmGpsMapPoint *pt;
		OsmGpsMapPoint tmp;
		gint x = 0, y = 0, dx, dy, d2;
		float flat = 0.0f, flon = 0.0f;

		pt = osm_gps_map_image_get_point (m);
		if (!pt)
			continue;

		osm_gps_map_point_get_degrees ((OsmGpsMapPoint *) pt,
		                               &flat, &flon);
		osm_gps_map_point_set_degrees (&tmp, flat, flon);
		osm_gps_map_convert_geographic_to_screen (plugin->map,
		                                          &tmp, &x, &y);

		dx = (gint) event->x - x;
		dy = (gint) event->y - y;
		d2 = dx * dx + dy * dy;
		if (d2 < best_dist2) {
			best_dist2 = d2;
			best = m;
		}
	}

	return best;
}

static gboolean
map_button_press_cb (GtkWidget        *widget,
                     GdkEventButton   *event,
                     XviewerMapPlugin *plugin)
{
	OsmGpsMapImage *marker;
	XviewerImage *image;

	if (event->button != GDK_BUTTON_PRIMARY ||
	    event->type != GDK_BUTTON_PRESS)
		return FALSE;

	marker = find_marker_at_event (plugin, event);
	if (!marker)
		return FALSE;

	image = g_object_get_data (G_OBJECT (marker), "image");
	if (!image)
		return FALSE;

	xviewer_thumb_view_set_current_image (XVIEWER_THUMB_VIEW (plugin->thumbview),
	                                  image, TRUE);
	return TRUE;
}

static void
create_marker (XviewerImage *image,
	       XviewerMapPlugin *plugin)
{
	gdouble lon, lat;

	if (!image)
		return;

	if (!xviewer_image_has_data (image, XVIEWER_IMAGE_DATA_EXIF) &&
	    !xviewer_image_load (image, XVIEWER_IMAGE_DATA_EXIF, NULL, NULL))
		return;

	if (get_coordinates (image, &lat, &lon)) {
		OsmGpsMapImage *marker;

		if (!plugin->icon_small)
			return;

		marker = osm_gps_map_image_add_with_alignment (plugin->map,
		                                               (float) lat,
		                                               (float) lon,
		                                               plugin->icon_small,
		                                               0.5f, 1.0f);

		g_object_set_data (G_OBJECT (marker), "image", image);
		g_object_set_data_full (G_OBJECT (image), "marker",
		                        marker, NULL);
		plugin->markers = g_list_prepend (plugin->markers, marker);
	}

}

static void
selection_changed_cb (XviewerThumbView *view,
		      XviewerMapPlugin *plugin)
{
	XviewerImage *image;
	OsmGpsMapImage *marker;

	if (!xviewer_thumb_view_get_n_selected (view))
		return;

	image = xviewer_thumb_view_get_first_selected_image (view);

	g_return_if_fail (image != NULL);

	marker = g_object_get_data (G_OBJECT (image), "marker");

	if (marker) {
		gdouble lat, lon;

		get_marker_coords (marker, &lat, &lon);
		osm_gps_map_set_center (plugin->map,
		                        (float) lat, (float) lon);

		/* Reset the previous selection */
		if (plugin->marker && plugin->marker != marker) {
			replace_marker_pixbuf (plugin, plugin->marker,
			                       plugin->icon_small);
		}

		/* Highlight new selection */
		plugin->marker = replace_marker_pixbuf (plugin, marker,
		                                        plugin->icon_large);
		gtk_widget_set_sensitive (plugin->jump_to_button, TRUE);
	}
	else {
		gtk_widget_set_sensitive (plugin->jump_to_button, FALSE);

		/* Reset the previous selection */
		if (plugin->marker) {
			replace_marker_pixbuf (plugin, plugin->marker,
			                       plugin->icon_small);
		}

		plugin->marker = NULL;
	}

	g_object_unref (image);
}

static void
jump_to (GtkWidget *widget,
	 XviewerMapPlugin *plugin)
{
	gdouble lat, lon;

	if (!plugin->marker)
		return;

	get_marker_coords (plugin->marker, &lat, &lon);
	osm_gps_map_set_center (plugin->map, (float) lat, (float) lon);
}

static void
zoom_in (GtkWidget *widget,
	 XviewerMapPlugin *plugin)
{
	osm_gps_map_zoom_in (plugin->map);
}

static void
zoom_out (GtkWidget *widget,
	  XviewerMapPlugin *plugin)
{
	osm_gps_map_zoom_out (plugin->map);
}

static gboolean
for_each_thumb (GtkTreeModel *model,
		GtkTreePath *path,
		GtkTreeIter *iter,
		XviewerMapPlugin *plugin)
{
	XviewerImage *image = NULL;

	gtk_tree_model_get (model, iter,
			    XVIEWER_LIST_STORE_XVIEWER_IMAGE, &image,
			    -1);

	if (!image) {
		return FALSE;
	}

	create_marker (image, plugin);

	g_object_unref (image);
	return FALSE;
}

static void
fit_all_markers (XviewerMapPlugin *plugin)
{
	GList *l;
	gdouble min_lat = 90.0, max_lat = -90.0;
	gdouble min_lon = 180.0, max_lon = -180.0;
	gboolean any = FALSE;

	for (l = plugin->markers; l != NULL; l = l->next) {
		gdouble lat, lon;
		get_marker_coords (l->data, &lat, &lon);
		if (lat < min_lat) min_lat = lat;
		if (lat > max_lat) max_lat = lat;
		if (lon < min_lon) min_lon = lon;
		if (lon > max_lon) max_lon = lon;
		any = TRUE;
	}

	if (!any)
		return;

	if (min_lat == max_lat && min_lon == max_lon) {
		osm_gps_map_set_center_and_zoom (plugin->map,
		                                 (float) min_lat,
		                                 (float) min_lon, 15);
	} else {
		osm_gps_map_zoom_fit_bbox (plugin->map,
		                           (float) min_lat, (float) max_lat,
		                           (float) min_lon, (float) max_lon);
	}
}

static void
prepared_cb (XviewerWindow *window,
	     XviewerMapPlugin *plugin)
{
	plugin->store = xviewer_window_get_store (plugin->window);

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

	plugin->thumbview = xviewer_window_get_thumb_view (window);
	plugin->selection_changed_id = g_signal_connect (G_OBJECT (plugin->thumbview),
						       "selection-changed",
						       G_CALLBACK (selection_changed_cb),
						       plugin);

	/* Call the callback because if the plugin is activated after
	 *  the image is loaded, selection_changed isn't emited
	 */
	selection_changed_cb (XVIEWER_THUMB_VIEW (plugin->thumbview), plugin);

	fit_all_markers (plugin);
}

static void
impl_activate (XviewerWindowActivatable *activatable)
{
	XviewerMapPlugin *plugin = XVIEWER_MAP_PLUGIN (activatable);
	GtkWidget *sidebar, *vbox, *bbox, *button, *viewport;

	xviewer_debug (DEBUG_PLUGINS);

	plugin->icon_small = load_marker_pixbuf (MARKER_ICON_SMALL);
	plugin->icon_large = load_marker_pixbuf (MARKER_ICON_LARGE);

	viewport = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (viewport), GTK_SHADOW_ETCHED_IN);

	plugin->map = OSM_GPS_MAP (osm_gps_map_new ());
	g_object_set (G_OBJECT (plugin->map),
	              "map-source", OSM_GPS_MAP_SOURCE_OPENSTREETMAP,
	              NULL);
	osm_gps_map_set_zoom (plugin->map, 3);

	plugin->button_press_id = g_signal_connect (plugin->map,
	                                            "button-press-event",
	                                            G_CALLBACK (map_button_press_cb),
	                                            plugin);

	gtk_container_add (GTK_CONTAINER (viewport), GTK_WIDGET (plugin->map));

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	bbox = gtk_toolbar_new ();

	button = GTK_WIDGET (gtk_tool_button_new (NULL, NULL));
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (button), "xsi-go-jump-symbolic");
	gtk_widget_set_tooltip_text (button, _("Jump to current image's location"));
	g_signal_connect (button,
			  "clicked",
			  G_CALLBACK (jump_to),
			  plugin);
	gtk_container_add (GTK_CONTAINER (bbox), button);
	plugin->jump_to_button = button;

	button = GTK_WIDGET (gtk_separator_tool_item_new ());
	gtk_container_add (GTK_CONTAINER (bbox), button);

	button = GTK_WIDGET (gtk_tool_button_new (NULL, NULL));
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (button), "xsi-zoom-in-symbolic");
	gtk_widget_set_tooltip_text (button, _("Zoom in"));
	g_signal_connect (button,
			  "clicked",
			  G_CALLBACK (zoom_in),
			  plugin);
	gtk_container_add (GTK_CONTAINER (bbox), button);

	button = GTK_WIDGET (gtk_tool_button_new (NULL, NULL));
	gtk_tool_button_set_icon_name (GTK_TOOL_BUTTON (button), "xsi-zoom-out-symbolic");
	gtk_widget_set_tooltip_text (button, _("Zoom out"));
	g_signal_connect (button,
			  "clicked",
			  G_CALLBACK (zoom_out),
			  plugin);
	gtk_container_add (GTK_CONTAINER (bbox), button);

	sidebar = xviewer_window_get_sidebar (plugin->window);
	plugin->viewport = vbox;
	gtk_box_pack_start (GTK_BOX (vbox), bbox, FALSE, FALSE, 0);
	gtk_widget_set_vexpand (viewport, TRUE);
	gtk_container_add (GTK_CONTAINER (vbox), viewport);
	xviewer_sidebar_add_page (XVIEWER_SIDEBAR (sidebar), _("Map"), vbox);
	gtk_widget_show_all (vbox);

	plugin->win_prepared_id = g_signal_connect (G_OBJECT (plugin->window),
						    "prepared",
						    G_CALLBACK (prepared_cb),
						    plugin);
	/* Call the callback once in case the window is already ready */
	prepared_cb (plugin->window, plugin);
}

static void
impl_deactivate (XviewerWindowActivatable *activatable)
{
	XviewerMapPlugin *plugin = XVIEWER_MAP_PLUGIN (activatable);
	GtkWidget *sidebar, *thumbview;

	xviewer_debug (DEBUG_PLUGINS);

	sidebar = xviewer_window_get_sidebar (plugin->window);
	xviewer_sidebar_remove_page (XVIEWER_SIDEBAR (sidebar), plugin->viewport);

	thumbview = xviewer_window_get_thumb_view (plugin->window);
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

	g_list_free (plugin->markers);
	plugin->markers = NULL;
	plugin->marker = NULL;

	g_clear_object (&plugin->icon_small);
	g_clear_object (&plugin->icon_large);
}

static void
xviewer_map_plugin_dispose (GObject *object)
{
        XviewerMapPlugin *plugin = XVIEWER_MAP_PLUGIN (object);

        if (plugin->window != NULL) {
                g_object_unref (plugin->window);
                plugin->window = NULL;
        }

        G_OBJECT_CLASS (xviewer_map_plugin_parent_class)->dispose (object);
}

static void
xviewer_map_plugin_get_property (GObject    *object,
			     guint       prop_id,
			     GValue     *value,
			     GParamSpec *pspec)
{
        XviewerMapPlugin *plugin = XVIEWER_MAP_PLUGIN (object);

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
xviewer_map_plugin_set_property (GObject      *object,
			     guint         prop_id,
			     const GValue *value,
			     GParamSpec   *pspec)
{
        XviewerMapPlugin *plugin = XVIEWER_MAP_PLUGIN (object);

        switch (prop_id)
        {
        case PROP_WINDOW:
                plugin->window = XVIEWER_WINDOW (g_value_dup_object (value));
                break;

        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                break;
        }
}

static void
xviewer_map_plugin_class_init (XviewerMapPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = xviewer_map_plugin_finalize;
        object_class->dispose = xviewer_map_plugin_dispose;
        object_class->set_property = xviewer_map_plugin_set_property;
        object_class->get_property = xviewer_map_plugin_get_property;

        g_object_class_override_property (object_class, PROP_WINDOW, "window");
}

static void
xviewer_map_plugin_class_finalize (XviewerMapPluginClass *klass)
{
        /* Dummy needed for G_DEFINE_DYNAMIC_TYPE_EXTENDED */
}

static void
xviewer_window_activatable_iface_init (XviewerWindowActivatableInterface *iface)
{
        iface->activate = impl_activate;
        iface->deactivate = impl_deactivate;
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
        xviewer_map_plugin_register_type (G_TYPE_MODULE (module));
        peas_object_module_register_extension_type (module,
                                                    XVIEWER_TYPE_WINDOW_ACTIVATABLE,
                                                    XVIEWER_TYPE_MAP_PLUGIN);
}
