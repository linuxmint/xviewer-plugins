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


#ifndef __EOG_EXIF_DISPLAY_PLUGIN_SETUP_H__
#define __EOG_EXIF_DISPLAY_PLUGIN_SETUP_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <eog/eog-thumb-view.h>
#include <eog/eog-window.h>
#include <libpeas/peas-extension-base.h>
#include <libpeas/peas-object-module.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define EOG_TYPE_EXIF_DISPLAY_PLUGIN_SETUP	(eog_exif_display_plugin_setup_get_type ())
#define EOG_EXIF_DISPLAY_PLUGIN_SETUP(o)	(G_TYPE_CHECK_INSTANCE_CAST ((o), EOG_TYPE_EXIF_DISPLAY_PLUGIN_SETUP, EogExifDisplayPluginSetup))
#define EOG_EXIF_DISPLAY_PLUGIN_SETUP_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k),      EOG_TYPE_EXIF_DISPLAY_PLUGIN_SETUP, EogExifDisplayPluginSetupClass))
#define EOG_IS_EXIF_DISPLAY_PLUGIN_SETUP(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), EOG_TYPE_EXIF_DISPLAY_PLUGIN_SETUP))
#define EOG_IS_EXIF_DISPLAY_PLUGIN_SETUP_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), EOG_TYPE_EXIF_DISPLAY_PLUGIN_SETUP))
#define EOG_EXIF_DISPLAY_PLUGIN_SETUP_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o),  EOG_TYPE_EXIF_DISPLAY_PLUGIN_SETUP, EogExifDisplayPluginSetupClass))

/* Private structure type */
typedef struct _EogExifDisplayPluginSetupPrivate EogExifDisplayPluginSetupPrivate;

/*
 * Main object structure
 */
typedef struct _EogExifDisplayPluginSetup	EogExifDisplayPluginSetup;

struct _EogExifDisplayPluginSetup
{
	PeasExtensionBase parent_instance;
};

/*
 * Class definition
 */
typedef struct _EogExifDisplayPluginSetupClass	EogExifDisplayPluginSetupClass;

struct _EogExifDisplayPluginSetupClass
{
	PeasExtensionBaseClass parent_class;
};

/*
 * Public methods
 */
GType	eog_exif_display_plugin_setup_get_type		(void) G_GNUC_CONST;

G_GNUC_INTERNAL
void	eog_exif_display_plugin_setup_register_types	(PeasObjectModule *module);


G_END_DECLS

#endif /* __EOG_EXIF_DISPLAY_PLUGIN_SETUP_H__ */
