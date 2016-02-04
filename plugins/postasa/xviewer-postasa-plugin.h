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

#ifndef __XVIEWER_POSTASA_PLUGIN_H__
#define __XVIEWER_POSTASA_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <libpeas/peas-extension-base.h>
#include <libpeas/peas-object-module.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define XVIEWER_TYPE_POSTASA_PLUGIN		(xviewer_postasa_plugin_get_type ())
#define XVIEWER_POSTASA_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), XVIEWER_TYPE_POSTASA_PLUGIN, XviewerPostasaPlugin))
#define XVIEWER_POSTASA_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k),     XVIEWER_TYPE_POSTASA_PLUGIN, XviewerPostasaPluginClass))
#define XVIEWER_IS_POSTASA_PLUGIN(o)	        (G_TYPE_CHECK_INSTANCE_TYPE ((o), XVIEWER_TYPE_POSTASA_PLUGIN))
#define XVIEWER_IS_POSTASA_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k),    XVIEWER_TYPE_POSTASA_PLUGIN))
#define XVIEWER_POSTASA_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o),  XVIEWER_TYPE_POSTASA_PLUGIN, XviewerPostasaPluginClass))

/* Private structure type */
typedef struct _XviewerPostasaPluginPrivate	XviewerPostasaPluginPrivate;

/*
 * Main object structure
 */
typedef struct _XviewerPostasaPlugin		XviewerPostasaPlugin;

struct _XviewerPostasaPlugin
{
	PeasExtensionBase parent_instance;
	XviewerPostasaPluginPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _XviewerPostasaPluginClass	XviewerPostasaPluginClass;

struct _XviewerPostasaPluginClass
{
	PeasExtensionBaseClass parent_class;
};

/*
 * Public methods
 */
GType	xviewer_postasa_plugin_get_type		(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_END_DECLS

#endif /* __XVIEWER_POSTASA_PLUGIN_H__ */
