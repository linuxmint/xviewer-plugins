#ifndef __XVIEWER_POSTR_PLUGIN_H__
#define __XVIEWER_POSTR_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <xviewer/xviewer-window.h>
#include <libpeas/peas-extension-base.h>
#include <libpeas/peas-object-module.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XVIEWER_TYPE_POSTR_PLUGIN		(xviewer_postr_plugin_get_type ())
#define XVIEWER_POSTR_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), XVIEWER_TYPE_POSTR_PLUGIN, XviewerPostrPlugin))
#define XVIEWER_POSTR_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k),     XVIEWER_TYPE_POSTR_PLUGIN, XviewerPostrPluginClass))
#define XVIEWER_IS_POSTR_PLUGIN(o)	        (G_TYPE_CHECK_INSTANCE_TYPE ((o), XVIEWER_TYPE_POSTR_PLUGIN))
#define XVIEWER_IS_POSTR_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k),    XVIEWER_TYPE_POSTR_PLUGIN))
#define XVIEWER_POSTR_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o),  XVIEWER_TYPE_POSTR_PLUGIN, XviewerPostrPluginClass))

/* Private structure type */
typedef struct _XviewerPostrPluginPrivate	XviewerPostrPluginPrivate;

/*
 * Main object structure
 */
typedef struct _XviewerPostrPlugin		XviewerPostrPlugin;

struct _XviewerPostrPlugin
{
	PeasExtensionBase parent_instance;

	XviewerWindow *window;
	GtkActionGroup *ui_action_group;
	guint ui_id;
};

/*
 * Class definition
 */
typedef struct _XviewerPostrPluginClass	XviewerPostrPluginClass;

struct _XviewerPostrPluginClass
{
	PeasExtensionBaseClass parent_class;
};

/*
 * Public methods
 */
GType	xviewer_postr_plugin_get_type		(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_END_DECLS

#endif /* __XVIEWER_POSTR_PLUGIN_H__ */
