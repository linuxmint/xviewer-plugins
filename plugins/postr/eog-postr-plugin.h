#ifndef __EOG_POSTR_PLUGIN_H__
#define __EOG_POSTR_PLUGIN_H__

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
#define EOG_TYPE_POSTR_PLUGIN		(eog_postr_plugin_get_type ())
#define EOG_POSTR_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), EOG_TYPE_POSTR_PLUGIN, EogPostrPlugin))
#define EOG_POSTR_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k),     EOG_TYPE_POSTR_PLUGIN, EogPostrPluginClass))
#define EOG_IS_POSTR_PLUGIN(o)	        (G_TYPE_CHECK_INSTANCE_TYPE ((o), EOG_TYPE_POSTR_PLUGIN))
#define EOG_IS_POSTR_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k),    EOG_TYPE_POSTR_PLUGIN))
#define EOG_POSTR_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o),  EOG_TYPE_POSTR_PLUGIN, EogPostrPluginClass))

/* Private structure type */
typedef struct _EogPostrPluginPrivate	EogPostrPluginPrivate;

/*
 * Main object structure
 */
typedef struct _EogPostrPlugin		EogPostrPlugin;

struct _EogPostrPlugin
{
	PeasExtensionBase parent_instance;

	EogWindow *window;
	GtkActionGroup *ui_action_group;
	guint ui_id;
};

/*
 * Class definition
 */
typedef struct _EogPostrPluginClass	EogPostrPluginClass;

struct _EogPostrPluginClass
{
	PeasExtensionBaseClass parent_class;
};

/*
 * Public methods
 */
GType	eog_postr_plugin_get_type		(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_END_DECLS

#endif /* __EOG_POSTR_PLUGIN_H__ */
