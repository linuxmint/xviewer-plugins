 /* Slideshow Shuffle Plugin for xviewer
 *
 * Copyright (c) 2008  Johannes Marbach <jm@rapidrabbit.de>
 * C port Copyright (c) 2026 ItsZariep
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

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <gmodule.h>
#include <libpeas/peas.h>
#include <xviewer/xviewer-window.h>
#include <xviewer/xviewer-window-activatable.h>
#include <xviewer/xviewer-list-store.h>
#include <xviewer/xviewer-image.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>

typedef struct _SlideshowShufflePlugin SlideshowShufflePlugin;
typedef struct _SlideshowShufflePluginClass SlideshowShufflePluginClass;

struct _SlideshowShufflePlugin
{
	GObject parent_instance;
	XviewerWindow *window;
	gulong state_handler_id;
	gboolean slideshow;
	GHashTable *map;
};

struct _SlideshowShufflePluginClass
{
	GObjectClass parent_class;
};

GType slideshow_shuffle_plugin_get_type (void) G_GNUC_CONST;

#define SLIDESHOW_SHUFFLE_TYPE_PLUGIN (slideshow_shuffle_plugin_get_type())
#define SLIDESHOW_SHUFFLE_PLUGIN(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SLIDESHOW_SHUFFLE_TYPE_PLUGIN, SlideshowShufflePlugin))

static void xviewer_window_activatable_iface_init (XviewerWindowActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (SlideshowShufflePlugin, slideshow_shuffle_plugin, G_TYPE_OBJECT, 0,
	G_IMPLEMENT_INTERFACE_DYNAMIC (XVIEWER_TYPE_WINDOW_ACTIVATABLE,
		xviewer_window_activatable_iface_init))

static gint
alphabetic_sort_function (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data)
{
	XviewerImage *img1, *img2;
	gtk_tree_model_get(model, a, XVIEWER_LIST_STORE_XVIEWER_IMAGE, &img1, -1);
	gtk_tree_model_get(model, b, XVIEWER_LIST_STORE_XVIEWER_IMAGE, &img2, -1);
	const char *uri1 = xviewer_image_get_uri_for_display(img1);
	const char *uri2 = xviewer_image_get_uri_for_display(img2);
	return g_strcmp0(uri1, uri2);
}

static gint
random_sort_function (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data)
{
	SlideshowShufflePlugin *plugin = SLIDESHOW_SHUFFLE_PLUGIN(data);
	XviewerImage *img1, *img2;
	gtk_tree_model_get(model, a, XVIEWER_LIST_STORE_XVIEWER_IMAGE, &img1, -1);
	gtk_tree_model_get(model, b, XVIEWER_LIST_STORE_XVIEWER_IMAGE, &img2, -1);
	const char *uri1 = xviewer_image_get_uri_for_display(img1);
	const char *uri2 = xviewer_image_get_uri_for_display(img2);
	gint pos1 = GPOINTER_TO_INT(g_hash_table_lookup(plugin->map, uri1));
	gint pos2 = GPOINTER_TO_INT(g_hash_table_lookup(plugin->map, uri2));
	if (pos1 > pos2) return 1;
	if (pos1 < pos2) return -1;
	return 0;
}

static void
state_changed_cb (GtkWidget *widget, GdkEventWindowState *event, gpointer user_data)
{
	SlideshowShufflePlugin *plugin = SLIDESHOW_SHUFFLE_PLUGIN(user_data);
	XviewerWindow *window = XVIEWER_WINDOW(widget);
	XviewerWindowMode mode = xviewer_window_get_mode(window);

	if (mode == XVIEWER_WINDOW_MODE_SLIDESHOW && !plugin->slideshow)
	{
		/* Slideshow starts */
		plugin->slideshow = TRUE;
		XviewerListStore *store = xviewer_window_get_store(window);
		if (!store) return;

		XviewerImage *current_image = xviewer_window_get_image(window);
		gint pos = 0;
		if (current_image)
		{
			pos = xviewer_list_store_get_pos_by_image(store, current_image);
		}

		/* Generate URI list and remove current */
		GList *uri_list = NULL;
		const char *current_uri = NULL;
		if (current_image)
		{
			current_uri = xviewer_image_get_uri_for_display(current_image);
		}

		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter))
		{
			do
			{
				XviewerImage *img;
				gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, XVIEWER_LIST_STORE_XVIEWER_IMAGE, &img, -1);
				const char *uri = xviewer_image_get_uri_for_display(img);
				if (!current_uri || g_strcmp0(uri, current_uri) != 0)
				{
					uri_list = g_list_append(uri_list, (gpointer)uri);
				}
			} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter));
		}

		plugin->map = g_hash_table_new(g_str_hash, g_str_equal);
		g_hash_table_insert(plugin->map, (gpointer)current_uri, GINT_TO_POINTER(0));

		/* Generate random positions for remaining URIs */
		GList *supply = NULL;
		for (gint i = 1; i <= g_list_length(uri_list); i++)
		{
			supply = g_list_append(supply, GINT_TO_POINTER(i));
		}
		
		GList *remaining = g_list_copy(uri_list);
		for (GList *l = remaining; l; l = l->next)
		{
			gint idx = g_random_int_range(0, g_list_length(supply));
			gint rand_pos = GPOINTER_TO_INT(g_list_nth_data(supply, idx));
			supply = g_list_remove(supply, GINT_TO_POINTER(rand_pos));
			g_hash_table_insert(plugin->map, l->data, GINT_TO_POINTER(rand_pos));
		}
		g_list_free(uri_list);
		g_list_free(remaining);
		g_list_free(supply);

		gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store), random_sort_function, plugin, NULL);
	}
	else if (mode == XVIEWER_WINDOW_MODE_NORMAL && plugin->slideshow)
	{
		/* Slideshow ends */
		plugin->slideshow = FALSE;
		XviewerListStore *store = xviewer_window_get_store(window);
		if (store)
		{
			gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store), alphabetic_sort_function, NULL, NULL);
		}
		if (plugin->map)
		{
			g_hash_table_destroy(plugin->map);
			plugin->map = NULL;
		}
	}
}

/* Property handling */
enum
{
	PROP_0,
	PROP_WINDOW
};

static void
slideshow_shuffle_plugin_set_property (GObject *object, guint prop_id, 
	const GValue *value, GParamSpec *pspec)
{
	SlideshowShufflePlugin *plugin = SLIDESHOW_SHUFFLE_PLUGIN(object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			plugin->window = XVIEWER_WINDOW(g_value_dup_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
slideshow_shuffle_plugin_get_property (GObject *object, guint prop_id, 
	GValue *value, GParamSpec *pspec)
{
	SlideshowShufflePlugin *plugin = SLIDESHOW_SHUFFLE_PLUGIN(object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			g_value_set_object(value, plugin->window);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
slideshow_shuffle_plugin_init (SlideshowShufflePlugin *plugin)
{
	plugin->state_handler_id = 0;
	plugin->slideshow = FALSE;
	plugin->map = NULL;
}

static void
slideshow_shuffle_plugin_dispose (GObject *object)
{
	SlideshowShufflePlugin *plugin = SLIDESHOW_SHUFFLE_PLUGIN(object);

	if (plugin->state_handler_id != 0 && plugin->window)
	{
		g_signal_handler_disconnect(plugin->window, plugin->state_handler_id);
		plugin->state_handler_id = 0;
	}

	if (plugin->map)
	{
		g_hash_table_destroy(plugin->map);
		plugin->map = NULL;
	}

	if (plugin->window)
	{
		g_object_unref(plugin->window);
		plugin->window = NULL;
	}

	G_OBJECT_CLASS(slideshow_shuffle_plugin_parent_class)->dispose(object);
}

static void
slideshow_shuffle_plugin_activate (XviewerWindowActivatable *activatable)
{
	SlideshowShufflePlugin *plugin = SLIDESHOW_SHUFFLE_PLUGIN(activatable);

	/* Initialize random seed */
	g_random_set_seed(time(NULL));

	/* Connect to window state events */
	plugin->state_handler_id = g_signal_connect(plugin->window, 
		"window-state-event",
		G_CALLBACK(state_changed_cb),
		plugin);
}

static void
slideshow_shuffle_plugin_deactivate (XviewerWindowActivatable *activatable)
{
	SlideshowShufflePlugin *plugin = SLIDESHOW_SHUFFLE_PLUGIN(activatable);

	if (plugin->state_handler_id != 0)
	{
		g_signal_handler_disconnect(plugin->window, plugin->state_handler_id);
		plugin->state_handler_id = 0;
	}

	if (plugin->map)
	{
		g_hash_table_destroy(plugin->map);
		plugin->map = NULL;
	}
}

static void
slideshow_shuffle_plugin_class_init (SlideshowShufflePluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->dispose = slideshow_shuffle_plugin_dispose;
	object_class->set_property = slideshow_shuffle_plugin_set_property;
	object_class->get_property = slideshow_shuffle_plugin_get_property;

	g_object_class_override_property(object_class, PROP_WINDOW, "window");
}

static void
slideshow_shuffle_plugin_class_finalize (SlideshowShufflePluginClass *klass)
{
}

static void
xviewer_window_activatable_iface_init (XviewerWindowActivatableInterface *iface)
{
	iface->activate = slideshow_shuffle_plugin_activate;
	iface->deactivate = slideshow_shuffle_plugin_deactivate;
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
	slideshow_shuffle_plugin_register_type(G_TYPE_MODULE(module));

	peas_object_module_register_extension_type(module,
		XVIEWER_TYPE_WINDOW_ACTIVATABLE,
		SLIDESHOW_SHUFFLE_TYPE_PLUGIN);
}