#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xviewer-postr-plugin.h"

#include <gmodule.h>
#include <glib/gi18n-lib.h>

#include <xviewer/xviewer-debug.h>
#include <xviewer/xviewer-thumb-view.h>
#include <xviewer/xviewer-image.h>
#include <xviewer/xviewer-window-activatable.h>

#define MENU_PATH "/MainMenu/ToolsMenu/ToolsOps_2"

enum {
	PROP_O,
	PROP_WINDOW
};

static void
xviewer_window_activatable_iface_init (XviewerWindowActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (XviewerPostrPlugin, xviewer_postr_plugin,
		PEAS_TYPE_EXTENSION_BASE, 0,
		G_IMPLEMENT_INTERFACE_DYNAMIC(XVIEWER_TYPE_WINDOW_ACTIVATABLE,
					xviewer_window_activatable_iface_init))

static void
postr_cb (GtkAction	*action,
	  XviewerWindow *window)
{
	GtkWidget *thumbview = xviewer_window_get_thumb_view (window);
	GList *images, *i;
	gchar *cmd = g_strdup ("postr ");

	images = xviewer_thumb_view_get_selected_images (XVIEWER_THUMB_VIEW (thumbview));

	for (i = g_list_first (images); i; i = i->next) {
		XviewerImage *image = (XviewerImage *) i->data;
		GFile *imgfile;
		gchar *imgpath;

		imgfile = xviewer_image_get_file (image);
		imgpath = g_file_get_path (imgfile);

		if (G_LIKELY (imgpath != NULL))
			cmd = g_strconcat (cmd, "\"", imgpath, "\"", " ", NULL);

		g_free (imgpath);
		g_object_unref (imgfile);
	}

	g_spawn_command_line_async (cmd, NULL);
}

static const GtkActionEntry action_entries[] =
{
	{ "RunPostr",
	  "postr",
	  N_("Upload to Flickr"),
	  NULL,
	  N_("Upload your pictures to Flickr"),
	  G_CALLBACK (postr_cb) }
};

static void
xviewer_postr_plugin_init (XviewerPostrPlugin *plugin)
{
	xviewer_debug_message (DEBUG_PLUGINS, "XviewerPostrPlugin initializing");

	plugin->ui_action_group = NULL;
	plugin->ui_id = 0;
}


static void
xviewer_postr_plugin_dispose (GObject *object)
{
	XviewerPostrPlugin *plugin = XVIEWER_POSTR_PLUGIN (object);

	xviewer_debug_message (DEBUG_PLUGINS, "XviewerPostrPlugin disposing");

	if (plugin->window != NULL) {
		g_object_unref (plugin->window);
		plugin->window = NULL;
	}

	G_OBJECT_CLASS (xviewer_postr_plugin_parent_class)->dispose (object);
}

static void
impl_activate (XviewerWindowActivatable *activatable)
{
	XviewerPostrPlugin *plugin = XVIEWER_POSTR_PLUGIN (activatable);
	GtkUIManager *manager;

	xviewer_debug (DEBUG_PLUGINS);

	g_return_if_fail (plugin->window != NULL);

	manager = xviewer_window_get_ui_manager (plugin->window);

	plugin->ui_action_group = gtk_action_group_new ("XviewerPostrPluginActions");

	gtk_action_group_set_translation_domain (plugin->ui_action_group,
						 GETTEXT_PACKAGE);
	gtk_action_group_add_actions (plugin->ui_action_group,
				      action_entries,
				      G_N_ELEMENTS (action_entries),
				      plugin->window);

	gtk_ui_manager_insert_action_group (manager,
					    plugin->ui_action_group,
					    -1);

	plugin->ui_id = gtk_ui_manager_new_merge_id (manager);

	gtk_ui_manager_add_ui (manager,
			       plugin->ui_id,
			       MENU_PATH,
			       "RunPostr",
			       "RunPostr",
			       GTK_UI_MANAGER_MENUITEM,
			       FALSE);
}

static void
impl_deactivate	(XviewerWindowActivatable *activatable)
{
	XviewerPostrPlugin *plugin = XVIEWER_POSTR_PLUGIN (activatable);
	GtkUIManager *manager;

	xviewer_debug (DEBUG_PLUGINS);

	manager = xviewer_window_get_ui_manager (plugin->window);

	gtk_ui_manager_remove_ui (manager,
				  plugin->ui_id);

	gtk_ui_manager_remove_action_group (manager,
					    plugin->ui_action_group);
	plugin->ui_action_group = NULL;
	plugin->ui_id = 0;
}

static void
xviewer_postr_plugin_get_property (GObject    *object,
			       guint       prop_id,
			       GValue     *value,
			       GParamSpec *pspec)
{
	XviewerPostrPlugin *plugin = XVIEWER_POSTR_PLUGIN (object);

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
xviewer_postr_plugin_set_property (GObject      *object,
			       guint         prop_id,
			       const GValue *value,
			       GParamSpec   *pspec)
{
	XviewerPostrPlugin *plugin = XVIEWER_POSTR_PLUGIN (object);

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
xviewer_postr_plugin_class_init (XviewerPostrPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = xviewer_postr_plugin_dispose;
	object_class->set_property = xviewer_postr_plugin_set_property;
	object_class->get_property = xviewer_postr_plugin_get_property;

	g_object_class_override_property (object_class, PROP_WINDOW, "window");
}

static void
xviewer_postr_plugin_class_finalize (XviewerPostrPluginClass *klass)
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
	xviewer_postr_plugin_register_type (G_TYPE_MODULE (module));
	peas_object_module_register_extension_type (module,
						    XVIEWER_TYPE_WINDOW_ACTIVATABLE,
						    XVIEWER_TYPE_POSTR_PLUGIN);
}
