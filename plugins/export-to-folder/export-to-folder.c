 /*  export-to-folder.c -- export plugin for xviewer
 *
 * Copyright (c) 2012  Jendrik Seipp (jendrikseipp@web.de)
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
#include <xviewer/xviewer-application.h>
#include <xviewer/xviewer-window.h>
#include <xviewer/xviewer-window-activatable.h>
#include <xviewer/xviewer-image.h>

#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>

#define BASE_KEY "org.x.viewer.plugins.export-to-folder"

static const gchar *ui_str = 
"<ui>"
"    <menubar name=\"MainMenu\">"
"        <menu name=\"ToolsMenu\" action=\"Tools\">"
"            <separator/>"
"            <menuitem name=\"Export\" action=\"Export\"/>"
"            <separator/>"
"        </menu>"
"    </menubar>"
"</ui>";

/* ExportPlugin type */
typedef struct _ExportPlugin ExportPlugin;
typedef struct _ExportPluginClass ExportPluginClass;

struct _ExportPlugin
{
	GObject parent_instance;

	XviewerWindow *window;
	GtkActionGroup *action_group;
	guint ui_id;
	GSettings *settings;
};

struct _ExportPluginClass
{
	GObjectClass parent_class;
};

GType export_plugin_get_type (void) G_GNUC_CONST;

#define EXPORT_TYPE_PLUGIN (export_plugin_get_type())
#define EXPORT_PLUGIN(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), EXPORT_TYPE_PLUGIN, ExportPlugin))

static void xviewer_window_activatable_iface_init (XviewerWindowActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (ExportPlugin, export_plugin, G_TYPE_OBJECT, 0,
	G_IMPLEMENT_INTERFACE_DYNAMIC (XVIEWER_TYPE_WINDOW_ACTIVATABLE,
		xviewer_window_activatable_iface_init))

/* Helper functions */
static gchar *
get_default_export_dir (void)
{
	const gchar *home = g_get_home_dir();
	return g_build_filename(home, "exported-images", NULL);
}

static gchar *
get_export_dir (ExportPlugin *plugin)
{
	gchar *target_dir = g_settings_get_string(plugin->settings, "target-folder");

	if (target_dir == NULL || target_dir[0] == '\0')
	{
		g_free(target_dir);
		return get_default_export_dir();
	}

	return target_dir;
}

static gboolean
create_directory_if_needed (const gchar *path)
{
	if (g_file_test(path, G_FILE_TEST_EXISTS))
	{
		return TRUE;
	}

	if (g_mkdir_with_parents(path, 0755) != 0)
	{
		g_warning("Failed to create directory %s: %s", path, g_strerror(errno));
		return FALSE;
	}

	return TRUE;
}

static void
export_cb (GtkAction *action, ExportPlugin *plugin)
{
	XviewerImage *image;
	GFile *file;
	gchar *src, *name, *export_dir, *dest;
	GError *error = NULL;
	GtkWidget *dialog;
	gint response;

	/* Get path to current image */
	image = xviewer_window_get_image(plugin->window);
	if (!image)
	{
		g_print("No image can be exported\n");
		return;
	}

	file = xviewer_image_get_file(image);
	if (!file)
	{
		g_print("No image file\n");
		return;
	}

	src = g_file_get_path(file);
	if (!src)
	{
		g_object_unref(file);
		return;
	}

	name = g_path_get_basename(src);

	/* Create file chooser dialog */
	dialog = gtk_file_chooser_dialog_new(_("Export Image"),
		GTK_WINDOW(plugin->window),
		GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		_("_Cancel"), GTK_RESPONSE_CANCEL,
		_("_Export"), GTK_RESPONSE_ACCEPT,
		NULL);

	/* Set default folder from settings */
	export_dir = get_export_dir(plugin);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), export_dir);
	g_free(export_dir);

	/* Show dialog and get response */
	response = gtk_dialog_run(GTK_DIALOG(dialog));

	if (response == GTK_RESPONSE_ACCEPT)
	{
		export_dir = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog));

		if (export_dir)
		{
			dest = g_build_filename(export_dir, name, NULL);

			/* Create directory if it doesn't exist */
			create_directory_if_needed(export_dir);

			/* Copy file */
			GFile *src_file = g_file_new_for_path(src);
			GFile *dest_file = g_file_new_for_path(dest);

			if (g_file_copy(src_file, dest_file, G_FILE_COPY_OVERWRITE, 
				NULL, NULL, NULL, &error))
			{
				g_print("Copied %s into %s\n", name, export_dir);

				/* Update settings with last used folder */
				g_settings_set_string(plugin->settings, "target-folder", export_dir);
			}
			else
			{
				g_warning("Failed to copy file: %s", error->message);
				g_error_free(error);
			}

			g_object_unref(src_file);
			g_object_unref(dest_file);
			g_free(dest);
			g_free(export_dir);
		}
	}

	gtk_widget_destroy(dialog);
	g_object_unref(file);
	g_free(src);
	g_free(name);
}

/* ExportPlugin implementation */
enum
{
	PROP_0,
	PROP_WINDOW
};

static void
export_plugin_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	ExportPlugin *plugin = EXPORT_PLUGIN(object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			plugin->window = XVIEWER_WINDOW(g_value_dup_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static void
export_plugin_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	ExportPlugin *plugin = EXPORT_PLUGIN(object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			g_value_set_object(value, plugin->window);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static void
export_plugin_init (ExportPlugin *plugin)
{
	plugin->settings = g_settings_new(BASE_KEY);
}

static void
export_plugin_dispose (GObject *object)
{
	ExportPlugin *plugin = EXPORT_PLUGIN(object);

	if (plugin->window)
	{
		g_object_unref(plugin->window);
		plugin->window = NULL;
	}

	if (plugin->settings)
	{
		g_object_unref(plugin->settings);
		plugin->settings = NULL;
	}

	G_OBJECT_CLASS(export_plugin_parent_class)->dispose(object);
}

static void
export_plugin_activate (XviewerWindowActivatable *activatable)
{
	ExportPlugin *plugin = EXPORT_PLUGIN(activatable);
	GtkUIManager *ui_manager;
	GError *error = NULL;

	static const GtkActionEntry action_entries[] =
	{
		{ "Export", NULL, N_("_Export"), "E", NULL, G_CALLBACK(export_cb) }
	};

	ui_manager = xviewer_window_get_ui_manager(plugin->window);

	plugin->action_group = gtk_action_group_new("Export");
	gtk_action_group_set_translation_domain(plugin->action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions(plugin->action_group, action_entries,
		G_N_ELEMENTS(action_entries), plugin);

	gtk_ui_manager_insert_action_group(ui_manager, plugin->action_group, 0);

	plugin->ui_id = gtk_ui_manager_add_ui_from_string(ui_manager, ui_str, -1, &error);
	if (error)
	{
		g_warning("Failed to add UI: %s", error->message);
		g_error_free(error);
	}
}

static void
export_plugin_deactivate (XviewerWindowActivatable *activatable)
{
	ExportPlugin *plugin = EXPORT_PLUGIN(activatable);
	GtkUIManager *ui_manager;

	ui_manager = xviewer_window_get_ui_manager(plugin->window);

	gtk_ui_manager_remove_ui(ui_manager, plugin->ui_id);
	gtk_ui_manager_remove_action_group(ui_manager, plugin->action_group);

	g_object_unref(plugin->action_group);
	plugin->action_group = NULL;
}

static void
export_plugin_class_init (ExportPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->dispose = export_plugin_dispose;
	object_class->set_property = export_plugin_set_property;
	object_class->get_property = export_plugin_get_property;

	g_object_class_override_property(object_class, PROP_WINDOW, "window");
}

static void
export_plugin_class_finalize (ExportPluginClass *klass)
{
	/* dummy function used by G_DEFINE_DYNAMIC_TYPE */
}

static void
xviewer_window_activatable_iface_init (XviewerWindowActivatableInterface *iface)
{
	iface->activate = export_plugin_activate;
	iface->deactivate = export_plugin_deactivate;
}

/* Module registration */
G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
	export_plugin_register_type(G_TYPE_MODULE(module));

	peas_object_module_register_extension_type(module,
		XVIEWER_TYPE_WINDOW_ACTIVATABLE,
		EXPORT_TYPE_PLUGIN);
}