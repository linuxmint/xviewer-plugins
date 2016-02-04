#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "eog-postr-plugin.h"

#include <gmodule.h>
#include <glib/gi18n-lib.h>

#include <eog/eog-debug.h>
#include <eog/eog-thumb-view.h>
#include <eog/eog-image.h>
#include <eog/eog-window-activatable.h>

#define MENU_PATH "/MainMenu/ToolsMenu/ToolsOps_2"

enum {
	PROP_O,
	PROP_WINDOW
};

static void
eog_window_activatable_iface_init (EogWindowActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (EogPostrPlugin, eog_postr_plugin,
		PEAS_TYPE_EXTENSION_BASE, 0,
		G_IMPLEMENT_INTERFACE_DYNAMIC(EOG_TYPE_WINDOW_ACTIVATABLE,
					eog_window_activatable_iface_init))

static void
postr_cb (GtkAction	*action,
	  EogWindow *window)
{
	GtkWidget *thumbview = eog_window_get_thumb_view (window);
	GList *images, *i;
	gchar *cmd = g_strdup ("postr ");

	images = eog_thumb_view_get_selected_images (EOG_THUMB_VIEW (thumbview));

	for (i = g_list_first (images); i; i = i->next) {
		EogImage *image = (EogImage *) i->data;
		GFile *imgfile;
		gchar *imgpath;

		imgfile = eog_image_get_file (image);
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
eog_postr_plugin_init (EogPostrPlugin *plugin)
{
	eog_debug_message (DEBUG_PLUGINS, "EogPostrPlugin initializing");

	plugin->ui_action_group = NULL;
	plugin->ui_id = 0;
}


static void
eog_postr_plugin_dispose (GObject *object)
{
	EogPostrPlugin *plugin = EOG_POSTR_PLUGIN (object);

	eog_debug_message (DEBUG_PLUGINS, "EogPostrPlugin disposing");

	if (plugin->window != NULL) {
		g_object_unref (plugin->window);
		plugin->window = NULL;
	}

	G_OBJECT_CLASS (eog_postr_plugin_parent_class)->dispose (object);
}

static void
impl_activate (EogWindowActivatable *activatable)
{
	EogPostrPlugin *plugin = EOG_POSTR_PLUGIN (activatable);
	GtkUIManager *manager;

	eog_debug (DEBUG_PLUGINS);

	g_return_if_fail (plugin->window != NULL);

	manager = eog_window_get_ui_manager (plugin->window);

	plugin->ui_action_group = gtk_action_group_new ("EogPostrPluginActions");

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
impl_deactivate	(EogWindowActivatable *activatable)
{
	EogPostrPlugin *plugin = EOG_POSTR_PLUGIN (activatable);
	GtkUIManager *manager;

	eog_debug (DEBUG_PLUGINS);

	manager = eog_window_get_ui_manager (plugin->window);

	gtk_ui_manager_remove_ui (manager,
				  plugin->ui_id);

	gtk_ui_manager_remove_action_group (manager,
					    plugin->ui_action_group);
	plugin->ui_action_group = NULL;
	plugin->ui_id = 0;
}

static void
eog_postr_plugin_get_property (GObject    *object,
			       guint       prop_id,
			       GValue     *value,
			       GParamSpec *pspec)
{
	EogPostrPlugin *plugin = EOG_POSTR_PLUGIN (object);

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
eog_postr_plugin_set_property (GObject      *object,
			       guint         prop_id,
			       const GValue *value,
			       GParamSpec   *pspec)
{
	EogPostrPlugin *plugin = EOG_POSTR_PLUGIN (object);

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
eog_postr_plugin_class_init (EogPostrPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = eog_postr_plugin_dispose;
	object_class->set_property = eog_postr_plugin_set_property;
	object_class->get_property = eog_postr_plugin_get_property;

	g_object_class_override_property (object_class, PROP_WINDOW, "window");
}

static void
eog_postr_plugin_class_finalize (EogPostrPluginClass *klass)
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
	eog_postr_plugin_register_type (G_TYPE_MODULE (module));
	peas_object_module_register_extension_type (module,
						    EOG_TYPE_WINDOW_ACTIVATABLE,
						    EOG_TYPE_POSTR_PLUGIN);
}
