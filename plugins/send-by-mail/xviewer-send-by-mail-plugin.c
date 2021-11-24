/* SendByMail  -- Send images per email
 *
 * Copyright (C) 2009 The Free Software Foundation
 *
 * Author: Felix Riemann  <friemann@gnome.org>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n-lib.h>
#include <xviewer/xviewer-image.h>
#include <xviewer/xviewer-thumb-view.h>
#include <xviewer/xviewer-window.h>
#include <xviewer/xviewer-window-activatable.h>

#include "xviewer-send-by-mail-plugin.h"


static void
xviewer_window_activatable_iface_init (XviewerWindowActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (XviewerSendByMailPlugin, xviewer_send_by_mail_plugin,
		PEAS_TYPE_EXTENSION_BASE, 0,
		G_IMPLEMENT_INTERFACE_DYNAMIC (XVIEWER_TYPE_WINDOW_ACTIVATABLE,
					xviewer_window_activatable_iface_init))

enum {
	PROP_0,
	PROP_WINDOW
};

static void send_by_mail_cb (GtkAction *action, XviewerWindow *window);

static const gchar * const ui_definition =
	"<ui><menubar name=\"MainMenu\">"
	"<menu name=\"ToolsMenu\" action=\"Tools\">"
	"<menuitem action=\"XviewerPluginSendByMail\"/>"
	"</menu></menubar>"
	"<popup name=\"ViewPopup\"><separator/>"
        "<menuitem action=\"XviewerPluginSendByMail\"/><separator/>"
        "</popup></ui>";

static const GtkActionEntry action_entries[] =
{
	{ "XviewerPluginSendByMail",
	  "mail-message-new-symbolic",
	  N_("Send by Mail"),
	  NULL,
	  N_("Send the selected images by mail"),
	  G_CALLBACK (send_by_mail_cb) }
};

static void
xviewer_send_by_mail_plugin_init (XviewerSendByMailPlugin *plugin)
{
	plugin->ui_action_group = NULL;
	plugin->ui_menuitem_id = 0;
}

static void
impl_activate (XviewerWindowActivatable *activatable)
{
	XviewerSendByMailPlugin *plugin = XVIEWER_SEND_BY_MAIL_PLUGIN (activatable);
	GtkUIManager *manager;

	manager = xviewer_window_get_ui_manager (plugin->window);

	plugin->ui_action_group = gtk_action_group_new ("XviewerSendByMailPluginActions");

	gtk_action_group_set_translation_domain (plugin->ui_action_group,
						 GETTEXT_PACKAGE);

	gtk_action_group_add_actions (plugin->ui_action_group,
				      action_entries,
				      G_N_ELEMENTS (action_entries),
				      plugin->window);

	gtk_ui_manager_insert_action_group (manager,
					    plugin->ui_action_group,
					    -1);

	plugin->ui_menuitem_id = gtk_ui_manager_add_ui_from_string (manager,
								  ui_definition,
								  -1, NULL);
}

static void
impl_deactivate	(XviewerWindowActivatable *activatable)
{
	XviewerSendByMailPlugin *plugin = XVIEWER_SEND_BY_MAIL_PLUGIN (activatable);
	GtkUIManager *manager;

	manager = xviewer_window_get_ui_manager (plugin->window);

	gtk_ui_manager_remove_ui (manager,
				  plugin->ui_menuitem_id);

	gtk_ui_manager_remove_action_group (manager,
					    plugin->ui_action_group);
	plugin->ui_action_group = NULL;
	plugin->ui_menuitem_id = 0;
}

static void
xviewer_send_by_mail_plugin_dispose (GObject *object)
{
	XviewerSendByMailPlugin *plugin = XVIEWER_SEND_BY_MAIL_PLUGIN (object);

	if (plugin->window != NULL) {
		g_object_unref (plugin->window);
		plugin->window = NULL;
	}

	G_OBJECT_CLASS (xviewer_send_by_mail_plugin_parent_class)->dispose (object);
}

static void
xviewer_send_by_mail_plugin_get_property (GObject    *object,
				      guint       prop_id,
				      GValue     *value,
				      GParamSpec *pspec)
{
	XviewerSendByMailPlugin *plugin = XVIEWER_SEND_BY_MAIL_PLUGIN (object);

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
xviewer_send_by_mail_plugin_set_property (GObject      *object,
				      guint         prop_id,
				      const GValue *value,
				      GParamSpec   *pspec)
{
	XviewerSendByMailPlugin *plugin = XVIEWER_SEND_BY_MAIL_PLUGIN (object);

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
xviewer_send_by_mail_plugin_class_init (XviewerSendByMailPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = xviewer_send_by_mail_plugin_dispose;
	object_class->set_property = xviewer_send_by_mail_plugin_set_property;
	object_class->get_property = xviewer_send_by_mail_plugin_get_property;

	g_object_class_override_property (object_class, PROP_WINDOW, "window");
}

static void
xviewer_send_by_mail_plugin_class_finalize (XviewerSendByMailPluginClass *klass)
{
	/* Dummy needed for G_DEFINE_DYNAMIC_TYPE_EXTENDED */
}

static void
xviewer_window_activatable_iface_init (XviewerWindowActivatableInterface *iface)
{
	iface->activate = impl_activate;
	iface->deactivate = impl_deactivate;
}

static void
send_by_mail_cb (GtkAction *action, XviewerWindow *window)
{
	GdkScreen *screen = NULL;
	GtkWidget *tview = NULL;
	GList *images = NULL, *image = NULL;
	gboolean first = TRUE;
	GString *attachment_str = NULL;
	gchar *attachment = NULL;
	gchar *argv[] = { "thunderbird", "-compose", NULL, NULL };

	g_return_if_fail (XVIEWER_IS_WINDOW (window));

	if (gtk_widget_has_screen (GTK_WIDGET (window)))
		screen = gtk_widget_get_screen (GTK_WIDGET (window));

	tview = xviewer_window_get_thumb_view (window);
	images = xviewer_thumb_view_get_selected_images (XVIEWER_THUMB_VIEW (tview));

	attachment_str = g_string_new ("attachment='");

	for (image = images; image != NULL; image = image->next) {
		XviewerImage *img = XVIEWER_IMAGE (image->data);
		GFile *file;
		gchar *file_uri;

		file = xviewer_image_get_file (img);
		if (!file) {
			g_object_unref (img);
			continue;
		}

		file_uri = g_file_get_uri (file);
		if (first) {
			g_string_append (attachment_str, file_uri);
			first = FALSE;
		} else {
			g_string_append_c (attachment_str, ',');
			g_string_append (attachment_str, file_uri);
		}
		g_free (file_uri);
		g_object_unref (file);
		g_object_unref (img);
	}

	g_string_append_c (attachment_str, '\'');
	attachment = g_string_free (attachment_str, FALSE);
	argv[2] = attachment;

	g_spawn_async (NULL, argv, NULL, G_SPAWN_DEFAULT, NULL, NULL, NULL, NULL);

	g_free (attachment);
	g_list_free (images);
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
	xviewer_send_by_mail_plugin_register_type (G_TYPE_MODULE (module));
	peas_object_module_register_extension_type (module,
						    XVIEWER_TYPE_WINDOW_ACTIVATABLE,
						    XVIEWER_TYPE_SEND_BY_MAIL_PLUGIN);
}
