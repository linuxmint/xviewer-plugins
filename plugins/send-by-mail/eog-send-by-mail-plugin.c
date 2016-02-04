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
#include <eog/eog-image.h>
#include <eog/eog-thumb-view.h>
#include <eog/eog-window.h>
#include <eog/eog-window-activatable.h>

#include "eog-send-by-mail-plugin.h"


static void
eog_window_activatable_iface_init (EogWindowActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (EogSendByMailPlugin, eog_send_by_mail_plugin,
		PEAS_TYPE_EXTENSION_BASE, 0,
		G_IMPLEMENT_INTERFACE_DYNAMIC (EOG_TYPE_WINDOW_ACTIVATABLE,
					eog_window_activatable_iface_init))

enum {
	PROP_0,
	PROP_WINDOW
};

static void send_by_mail_cb (GtkAction *action, EogWindow *window);

static const gchar * const ui_definition =
	"<ui><menubar name=\"MainMenu\">"
	"<menu name=\"ToolsMenu\" action=\"Tools\">"
	"<menuitem action=\"EogPluginSendByMail\"/>"
	"</menu></menubar>"
	"<popup name=\"ViewPopup\"><separator/>"
        "<menuitem action=\"EogPluginSendByMail\"/><separator/>"
        "</popup></ui>";

static const GtkActionEntry action_entries[] =
{
	{ "EogPluginSendByMail",
	  "mail-message-new",
	  N_("Send by Mail"),
	  NULL,
	  N_("Send the selected images by mail"),
	  G_CALLBACK (send_by_mail_cb) }
};

static void
eog_send_by_mail_plugin_init (EogSendByMailPlugin *plugin)
{
	plugin->ui_action_group = NULL;
	plugin->ui_menuitem_id = 0;
}

static void
impl_activate (EogWindowActivatable *activatable)
{
	EogSendByMailPlugin *plugin = EOG_SEND_BY_MAIL_PLUGIN (activatable);
	GtkUIManager *manager;

	manager = eog_window_get_ui_manager (plugin->window);

	plugin->ui_action_group = gtk_action_group_new ("EogSendByMailPluginActions");

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
impl_deactivate	(EogWindowActivatable *activatable)
{
	EogSendByMailPlugin *plugin = EOG_SEND_BY_MAIL_PLUGIN (activatable);
	GtkUIManager *manager;

	manager = eog_window_get_ui_manager (plugin->window);

	gtk_ui_manager_remove_ui (manager,
				  plugin->ui_menuitem_id);

	gtk_ui_manager_remove_action_group (manager,
					    plugin->ui_action_group);
	plugin->ui_action_group = NULL;
	plugin->ui_menuitem_id = 0;
}

static void
eog_send_by_mail_plugin_dispose (GObject *object)
{
	EogSendByMailPlugin *plugin = EOG_SEND_BY_MAIL_PLUGIN (object);

	if (plugin->window != NULL) {
		g_object_unref (plugin->window);
		plugin->window = NULL;
	}

	G_OBJECT_CLASS (eog_send_by_mail_plugin_parent_class)->dispose (object);
}

static void
eog_send_by_mail_plugin_get_property (GObject    *object,
				      guint       prop_id,
				      GValue     *value,
				      GParamSpec *pspec)
{
	EogSendByMailPlugin *plugin = EOG_SEND_BY_MAIL_PLUGIN (object);

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
eog_send_by_mail_plugin_set_property (GObject      *object,
				      guint         prop_id,
				      const GValue *value,
				      GParamSpec   *pspec)
{
	EogSendByMailPlugin *plugin = EOG_SEND_BY_MAIL_PLUGIN (object);

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
eog_send_by_mail_plugin_class_init (EogSendByMailPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = eog_send_by_mail_plugin_dispose;
	object_class->set_property = eog_send_by_mail_plugin_set_property;
	object_class->get_property = eog_send_by_mail_plugin_get_property;

	g_object_class_override_property (object_class, PROP_WINDOW, "window");
}

static void
eog_send_by_mail_plugin_class_finalize (EogSendByMailPluginClass *klass)
{
	/* Dummy needed for G_DEFINE_DYNAMIC_TYPE_EXTENDED */
}

static void
eog_window_activatable_iface_init (EogWindowActivatableInterface *iface)
{
	iface->activate = impl_activate;
	iface->deactivate = impl_deactivate;
}

static void
send_by_mail_cb (GtkAction *action, EogWindow *window)
{
	GdkScreen *screen = NULL;
	GtkWidget *tview = NULL;
	GList *images = NULL, *image = NULL;
	GString *uri = NULL;
	gboolean first = TRUE;

	g_return_if_fail (EOG_IS_WINDOW (window));

	if (gtk_widget_has_screen (GTK_WIDGET (window)))
		screen = gtk_widget_get_screen (GTK_WIDGET (window));

	tview = eog_window_get_thumb_view (window);
	images = eog_thumb_view_get_selected_images (EOG_THUMB_VIEW (tview));
	uri = g_string_new ("mailto:?attach=");

	for (image = images; image != NULL; image = image->next) {
		EogImage *img = EOG_IMAGE (image->data);
		GFile *file;
		gchar *path;

		file = eog_image_get_file (img);
		if (!file) {
			g_object_unref (img);
			continue;
		}

		path = g_file_get_path (file);
		if (first) {
			uri = g_string_append (uri, path);
			first = FALSE;
		} else {
			g_string_append_printf (uri, "&attach=%s", path);
		}
		g_free (path);
		g_object_unref (file);
		g_object_unref (img);
	}
	g_list_free (images);

	/* TODO: Error handling/reporting? */
	gtk_show_uri (screen, uri->str, gtk_get_current_event_time (), NULL);

	g_string_free (uri, TRUE);
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
	eog_send_by_mail_plugin_register_type (G_TYPE_MODULE (module));
	peas_object_module_register_extension_type (module,
						    EOG_TYPE_WINDOW_ACTIVATABLE,
						    EOG_TYPE_SEND_BY_MAIL_PLUGIN);
}
