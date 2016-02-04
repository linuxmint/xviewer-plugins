/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* Postasa -- PicasaWeb Uploader plugin
 *
 * Copyright (C) 2009 The Free Software Foundation
 *
 * Author: Richard Schwarting <aquarichy@gmail.com>
 * Initially based on Postr code by: Lucas Rocha
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

#ifndef __EOG_POSTASA_PLUGIN_H__
#define __EOG_POSTASA_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <libpeas/peas-extension-base.h>
#include <libpeas/peas-object-module.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define EOG_TYPE_POSTASA_PLUGIN		(eog_postasa_plugin_get_type ())
#define EOG_POSTASA_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), EOG_TYPE_POSTASA_PLUGIN, EogPostasaPlugin))
#define EOG_POSTASA_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k),     EOG_TYPE_POSTASA_PLUGIN, EogPostasaPluginClass))
#define EOG_IS_POSTASA_PLUGIN(o)	        (G_TYPE_CHECK_INSTANCE_TYPE ((o), EOG_TYPE_POSTASA_PLUGIN))
#define EOG_IS_POSTASA_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k),    EOG_TYPE_POSTASA_PLUGIN))
#define EOG_POSTASA_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o),  EOG_TYPE_POSTASA_PLUGIN, EogPostasaPluginClass))

/* Private structure type */
typedef struct _EogPostasaPluginPrivate	EogPostasaPluginPrivate;

/*
 * Main object structure
 */
typedef struct _EogPostasaPlugin		EogPostasaPlugin;

struct _EogPostasaPlugin
{
	PeasExtensionBase parent_instance;
	EogPostasaPluginPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _EogPostasaPluginClass	EogPostasaPluginClass;

struct _EogPostasaPluginClass
{
	PeasExtensionBaseClass parent_class;
};

/*
 * Public methods
 */
GType	eog_postasa_plugin_get_type		(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_END_DECLS

#endif /* __EOG_POSTASA_PLUGIN_H__ */
