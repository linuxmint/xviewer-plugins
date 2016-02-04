#ifndef __EOG_MAP_PLUGIN_H__
#define __EOG_MAP_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <champlain/champlain.h>
#include <eog/eog-list-store.h>
#include <eog/eog-window.h>
#include <libpeas/peas-extension-base.h>
#include <libpeas/peas-object-module.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define EOG_TYPE_MAP_PLUGIN		(eog_map_plugin_get_type ())
#define EOG_MAP_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), EOG_TYPE_MAP_PLUGIN, EogMapPlugin))
#define EOG_MAP_PLUGIN_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k),     EOG_TYPE_MAP_PLUGIN, EogMapPluginClass))
#define EOG_IS_MAP_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), EOG_TYPE_MAP_PLUGIN))
#define EOG_IS_MAP_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k),    EOG_TYPE_MAP_PLUGIN))
#define EOG_MAP_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o),  EOG_TYPE_MAP_PLUGIN, EogMapPluginClass))

/* Private structure type */
typedef struct _EogMapPluginPrivate	EogMapPluginPrivate;

/*
 * Main object structure
 */
typedef struct _EogMapPlugin		EogMapPlugin;

struct _EogMapPlugin
{
	PeasExtensionBase parent_instance;

	EogWindow *window;

	/* Window Data */
	/* TODO: Make this a private struct! */
	/* Handlers ids */
	gulong selection_changed_id;
	gulong win_prepared_id;

	GtkWidget *thumbview;
	GtkWidget *viewport;
	ChamplainView *map;

	GtkWidget *jump_to_button;

	ChamplainMarkerLayer *layer;

	EogListStore *store;

	/* The current selected position */
	ChamplainLabel *marker;
};

/*
 * Class definition
 */
typedef struct _EogMapPluginClass	EogMapPluginClass;

struct _EogMapPluginClass
{
	PeasExtensionBaseClass parent_class;
};

/*
 * Public methods
 */
GType	eog_map_plugin_get_type		(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_END_DECLS

#endif /* __EOG_MAP_PLUGIN_H__ */
