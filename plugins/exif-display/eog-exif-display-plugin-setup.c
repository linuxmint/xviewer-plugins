/* Exif-display Plugin - Configuration Interface
 *
 * Copyright (C) 2009-2011 The Free Software Foundation
 *
 * Author: Felix Riemann  <friemann@gnome.org>
 * Based on code by Emmanuel Touzery  <emmanuel.touzery@free.fr>
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
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include <glib/gi18n-lib.h>
#include <eog/eog-debug.h>

#include <libpeas-gtk/peas-gtk-configurable.h>

#include "eog-exif-display-plugin-settings.h"

#include "eog-exif-display-plugin-setup.h"

#define GRESOURCE_PATH "/org/gnome/eog/plugins/exif-display/exif-display-config.ui"


static void
peas_gtk_configurable_iface_init (PeasGtkConfigurableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (EogExifDisplayPluginSetup,
		eog_exif_display_plugin_setup, PEAS_TYPE_EXTENSION_BASE, 0,
		G_IMPLEMENT_INTERFACE_DYNAMIC(PEAS_GTK_TYPE_CONFIGURABLE,
					peas_gtk_configurable_iface_init))


static void
eog_exif_display_plugin_setup_init (EogExifDisplayPluginSetup *setup)
{

}

static GtkWidget *
impl_create_config_widget (PeasGtkConfigurable *configurable)
{
	GSettings *settings;
	GtkBuilder *config_builder;
	GError *error = NULL;
	GtkWidget *display_channels_histogram_widget;
	GtkWidget *display_rgb_histogram_widget;
	GtkWidget *display_camera_settings_in_statusbar;
	GObject *result;
	gchar *object_ids[] = {"vbox1", NULL};

	settings = g_settings_new (EOG_EXIF_DISPLAY_CONF_SCHEMA_ID);

	config_builder = gtk_builder_new ();
	gtk_builder_set_translation_domain (config_builder, GETTEXT_PACKAGE);
	if (!gtk_builder_add_objects_from_resource (config_builder, GRESOURCE_PATH, object_ids, &error))
	{
		g_warning ("Couldn't load UI resource: %s", error->message);
		g_error_free (error);
	}

	// Add a reference to keep the box alive after the builder is gone
	result = g_object_ref (gtk_builder_get_object (config_builder, "vbox1"));
	display_channels_histogram_widget = GTK_WIDGET (
			gtk_builder_get_object (config_builder, "display_per_channel_histogram"));
	display_rgb_histogram_widget = GTK_WIDGET (
			gtk_builder_get_object (config_builder, "display_rgb_histogram"));
	display_camera_settings_in_statusbar = GTK_WIDGET (
			gtk_builder_get_object (config_builder, "display_camerasettings_statusbar"));

	g_settings_bind (settings, EOG_EXIF_DISPLAY_CONF_CHANNELS_HISTOGRAM,
			 display_channels_histogram_widget,
			 "active", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind (settings, EOG_EXIF_DISPLAY_CONF_RGB_HISTOGRAM,
			 display_rgb_histogram_widget,
			 "active", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind (settings, EOG_EXIF_DISPLAY_CONF_EXIF_IN_STATUSBAR,
			 display_camera_settings_in_statusbar,
			 "active", G_SETTINGS_BIND_DEFAULT);

	g_object_unref (config_builder);
	g_object_unref (settings);

	return GTK_WIDGET(result);
}


static void
eog_exif_display_plugin_setup_dispose (GObject *object)
{
	eog_debug_message (DEBUG_PLUGINS,
			   "EogExifDisplayPluginSetup disposing");

	G_OBJECT_CLASS (eog_exif_display_plugin_setup_parent_class)->dispose (object);
}

static void
eog_exif_display_plugin_setup_class_init (EogExifDisplayPluginSetupClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = eog_exif_display_plugin_setup_dispose;
}

static void
peas_gtk_configurable_iface_init (PeasGtkConfigurableInterface *iface)
{
	iface->create_configure_widget = impl_create_config_widget;
}

static void
eog_exif_display_plugin_setup_class_finalize (EogExifDisplayPluginSetupClass *klass)
{
	/* Dummy needed for G_DEFINE_DYNAMIC_TYPE_EXTENDED */
}

void
eog_exif_display_plugin_setup_register_types (PeasObjectModule *module)
{
	eog_exif_display_plugin_setup_register_type (G_TYPE_MODULE (module));
	peas_object_module_register_extension_type (module,
						    PEAS_GTK_TYPE_CONFIGURABLE,
						    EOG_TYPE_EXIF_DISPLAY_PLUGIN_SETUP);
}
