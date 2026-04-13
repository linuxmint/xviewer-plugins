#ifndef __XVIEWER_MAP_PLUGIN_H__
#define __XVIEWER_MAP_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <osm-gps-map.h>
#include <xviewer/xviewer-list-store.h>
#include <xviewer/xviewer-window.h>
#include <libpeas/peas-extension-base.h>
#include <libpeas/peas-object-module.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XVIEWER_TYPE_MAP_PLUGIN		(xviewer_map_plugin_get_type ())
#define XVIEWER_MAP_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), XVIEWER_TYPE_MAP_PLUGIN, XviewerMapPlugin))
#define XVIEWER_MAP_PLUGIN_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k),     XVIEWER_TYPE_MAP_PLUGIN, XviewerMapPluginClass))
#define XVIEWER_IS_MAP_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), XVIEWER_TYPE_MAP_PLUGIN))
#define XVIEWER_IS_MAP_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k),    XVIEWER_TYPE_MAP_PLUGIN))
#define XVIEWER_MAP_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o),  XVIEWER_TYPE_MAP_PLUGIN, XviewerMapPluginClass))

/* Private structure type */
typedef struct _XviewerMapPluginPrivate	XviewerMapPluginPrivate;

/*
 * Main object structure
 */
typedef struct _XviewerMapPlugin		XviewerMapPlugin;

struct _XviewerMapPlugin
{
	PeasExtensionBase parent_instance;

	XviewerWindow *window;

	/* Window Data */
	/* TODO: Make this a private struct! */
	/* Handlers ids */
	gulong selection_changed_id;
	gulong win_prepared_id;
	gulong button_press_id;

	GtkWidget *thumbview;
	GtkWidget *viewport;
	OsmGpsMap *map;

	GtkWidget *jump_to_button;

	/* Marker pixbufs */
	GdkPixbuf *icon_small;
	GdkPixbuf *icon_large;

	/* All markers (OsmGpsMapImage*) so we can hit-test them */
	GList *markers;

	XviewerListStore *store;

	/* The current selected marker */
	OsmGpsMapImage *marker;
};

/*
 * Class definition
 */
typedef struct _XviewerMapPluginClass	XviewerMapPluginClass;

struct _XviewerMapPluginClass
{
	PeasExtensionBaseClass parent_class;
};

/*
 * Public methods
 */
GType	xviewer_map_plugin_get_type		(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_END_DECLS

#endif /* __XVIEWER_MAP_PLUGIN_H__ */
