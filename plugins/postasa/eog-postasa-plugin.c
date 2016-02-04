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
 **/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "eog-postasa-plugin.h"

#include <gmodule.h>
#include <glib/gi18n-lib.h>

#include <eog/eog-debug.h>
#include <eog/eog-thumb-view.h>
#include <eog/eog-image.h>
#include <eog/eog-window-activatable.h>

#include <gdata/gdata.h>

#define EOG_POSTASA_RESOURCE_PREFIX "/org/gnome/eog/plugins/postasa"
#define GTKBUILDER_CONFIG_FILE EOG_POSTASA_RESOURCE_PREFIX"/postasa-config.xml"
#define GTKBUILDER_UPLOAD_FILE EOG_POSTASA_RESOURCE_PREFIX"/postasa-uploads.xml"

enum {
	PROP_O,
	PROP_WINDOW
};

static void
eog_window_activatable_iface_init (EogWindowActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (EogPostasaPlugin, eog_postasa_plugin,
		PEAS_TYPE_EXTENSION_BASE, 0,
		G_IMPLEMENT_INTERFACE_DYNAMIC(EOG_TYPE_WINDOW_ACTIVATABLE,
					eog_window_activatable_iface_init))

/**
 * _EogPostasaPluginPrivate:
 *
 * Private data for the Postasa EoG plugin
 **/
struct _EogPostasaPluginPrivate
{
	EogWindow    *eog_window;
	GtkActionGroup *ui_action_group;
	guint ui_id;


#ifdef HAVE_LIBGDATA_0_9
	GDataClientLoginAuthorizer *authorizer;
#endif
	GDataPicasaWebService *service;
	GCancellable *login_cancellable;

	/* TODO: consider using GConf to store the username in; perhaps not the password */
	GtkDialog    *login_dialog;
	GtkEntry     *username_entry;
	GtkEntry     *password_entry;
	GtkLabel     *login_message;
	GtkButton    *login_button;
	GtkButton    *cancel_button;
	gboolean      uploads_pending;

	GtkWindow    *uploads_window;
	GtkTreeView  *uploads_view;
	GtkListStore *uploads_store;
};

/**
 * PicasaWebUploadFileAsyncData:
 *
 * Small chunk of data for use by our asynchronous PicasaWeb upload
 * API.  It describes the position in the upload window's tree view, for
 * cancellation purposes, and the the image file that we want to
 * upload.
 *
 * TODO: remove this and the async API when we get a proper upload
 * async method into libgdata.
 **/
typedef struct
{
	GtkTreeIter *iter;
	GFile *imgfile;
} PicasaWebUploadFileAsyncData;

static void picasaweb_upload_cb (GtkAction *action, EogPostasaPlugin *plugin);
static GtkWidget *login_get_dialog (EogPostasaPlugin *plugin);
static gboolean login_dialog_close (EogPostasaPlugin *plugin);

static const gchar * const ui_definition = 
	"<ui><menubar name=\"MainMenu\">"
	"<menu name=\"ToolsMenu\" action=\"Tools\"><separator/>"
	"<menuitem name=\"EogPluginPostasa\" action=\"EogPluginRunPostasa\"/>"
	"<separator/></menu></menubar></ui>";

/**
 * action_entries:
 *
 * Describes the #GtkActionEntry representing the Postasa upload menu
 * item and it's callback.
 **/
static const GtkActionEntry action_entries[] =
{
	{ "EogPluginRunPostasa",
	  "postasa",
	  N_("Upload to PicasaWeb"),
	  NULL,
	  N_("Upload your pictures to PicasaWeb"),
	  G_CALLBACK (picasaweb_upload_cb) }
};

/*** Uploads Dialog ***/

/**
 * uploads_cancel_row:
 *
 * Function on the upload list store to cancel a specified upload.
 * This is intended to be called by foreach functions.
 *
 * There's a small chance that the #GCancellable cancellation might not
 * occur before the upload is completed, in which case we do not
 * change the row's status to "Cancelled".
 **/
static gboolean
uploads_cancel_row (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, EogPostasaPlugin *plugin)
{
	GCancellable *cancellable;

	gtk_tree_model_get (model, iter, 4, &cancellable, -1);
	g_cancellable_cancel (cancellable);
	/* we let picasaweb_upload_async_cb() handle setting a Cancelled message */

	return FALSE;
}

/**
 * uploads_cancel_cb:
 *
 * Obtains the current selection and calls an API to cancel each one.
 **/
static void
uploads_cancel_cb (GtkWidget *cancel_button, EogPostasaPlugin *plugin)
{
	GtkTreeSelection *selection;
	selection = gtk_tree_view_get_selection (plugin->priv->uploads_view); /* don't unref */

	gtk_tree_selection_selected_foreach (selection, (GtkTreeSelectionForeachFunc)uploads_cancel_row, plugin);
}

/**
 * uploads_cancel_all_cb:
 *
 * Calls an API on every row to cancel its upload.
 **/
static void
uploads_cancel_all_cb (GtkWidget *cancel_all_button, EogPostasaPlugin *plugin)
{
	gtk_tree_model_foreach (GTK_TREE_MODEL (plugin->priv->uploads_store), (GtkTreeModelForeachFunc)uploads_cancel_row, plugin);
}

/**
 * uploads_get_dialog:
 *
 * Returns the a #GtkWindow representing the Uploads window.  If it
 * has not already been created, it creates it.  The Uploads window is
 * set to be hidden instead of destroyed when closed, to avoid having
 * to recreate it and re-parse the UI file, etc.
 **/
static GtkWindow *
uploads_get_dialog (EogPostasaPlugin *plugin)
{
	GtkBuilder *builder;
	GError *error = NULL;
	GtkButton *cancel_button;
	GtkButton *cancel_all_button;

	if (plugin->priv->uploads_window == NULL) {
		builder = gtk_builder_new ();
		gtk_builder_set_translation_domain (builder, GETTEXT_PACKAGE);
		gtk_builder_add_from_resource (builder, GTKBUILDER_UPLOAD_FILE,
		                               &error);
		if (error != NULL) {
			g_warning ("Couldn't load Postasa uploads UI file:%d:%s", error->code, error->message);
			g_error_free (error);
			return NULL;
		}

		/* note: do not unref gtk_builder_get_object() returns */
		plugin->priv->uploads_window = GTK_WINDOW     (gtk_builder_get_object (builder, "uploads_window"));
		plugin->priv->uploads_view   = GTK_TREE_VIEW  (gtk_builder_get_object (builder, "uploads_view"));
		plugin->priv->uploads_store  = GTK_LIST_STORE (gtk_builder_get_object (builder, "uploads_store"));

		cancel_button     = GTK_BUTTON (gtk_builder_get_object (builder, "cancel_button"));
		cancel_all_button = GTK_BUTTON (gtk_builder_get_object (builder, "cancel_all_button"));

		/* TODO: can't set expand = TRUE when packing cells into columns via glade-3/GtkBuilder apparently?
		   bgo #602152  So for now, we take them, clear them out, and remap them.  Ugh.  Better solutions welcome.  */
		GtkTreeViewColumn *file_col       = GTK_TREE_VIEW_COLUMN (gtk_builder_get_object (builder, "file_col"));
		GtkCellRenderer   *thumbnail_cell = GTK_CELL_RENDERER    (gtk_builder_get_object (builder, "thumbnail_cell"));
		GtkCellRenderer   *filepath_cell  = GTK_CELL_RENDERER    (gtk_builder_get_object (builder, "filepath_cell"));
		gtk_tree_view_column_clear (file_col);
		gtk_tree_view_column_pack_start (file_col, thumbnail_cell, FALSE);
		gtk_tree_view_column_pack_end (file_col, filepath_cell, TRUE);
		gtk_tree_view_column_add_attribute (file_col, thumbnail_cell, "pixbuf", 0);
		gtk_tree_view_column_add_attribute (file_col, filepath_cell, "text", 1);
		GtkTreeViewColumn *progress_col   = GTK_TREE_VIEW_COLUMN (gtk_builder_get_object (builder, "progress_col"));
		GtkCellRenderer   *progress_cell  = GTK_CELL_RENDERER    (gtk_builder_get_object (builder, "progress_cell"));
		gtk_tree_view_column_clear (progress_col);
		gtk_tree_view_column_pack_end (progress_col, progress_cell, TRUE);
		gtk_tree_view_column_add_attribute (progress_col, progress_cell, "pulse", 3);
		gtk_tree_view_column_add_attribute (progress_col, progress_cell, "text", 5);

		g_object_unref (builder);

		g_signal_connect (G_OBJECT (cancel_button),     "clicked", G_CALLBACK (uploads_cancel_cb), plugin);
		g_signal_connect (G_OBJECT (cancel_all_button), "clicked", G_CALLBACK (uploads_cancel_all_cb), plugin);
		g_signal_connect (G_OBJECT (plugin->priv->uploads_window), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), plugin);
	}

	return plugin->priv->uploads_window;
}

typedef struct {
	EogPostasaPlugin *plugin;
	GtkTreeIter iter;
} PulseData;

static gboolean
pulse (PulseData *data)
{
	gint status;
	GCancellable *cancellable;

	gtk_tree_model_get (GTK_TREE_MODEL (data->plugin->priv->uploads_store), &(data->iter), 3, &status, 4, &cancellable, -1);

	if (0 <= status && status < G_MAXINT && g_cancellable_is_cancelled (cancellable) == FALSE) {
		/* TODO: consider potential for races and how g_timeout_add works wrt threading; none seen in testing, though */
		gtk_list_store_set (data->plugin->priv->uploads_store, &(data->iter), 3, status+1, -1);
		return TRUE;
	} else {
		/* either we've failed, <0, or we're done, G_MAX_INT */
		g_slice_free (PulseData, data);
		return FALSE;
	}
}

/**
 * uploads_add_entry:
 *
 * Adds a new row to the Uploads tree view for an #EogImage to upload.
 * The row stores the upload's #GCancellable and returns.
 *
 * Returns: a #GtkTreeIter that should be freed with g_slice_free()
 **/
static GtkTreeIter *
uploads_add_entry (EogPostasaPlugin *plugin, EogImage *image, GCancellable *cancellable)
{
	GtkWindow *uploads_window;
	GdkPixbuf *thumbnail_pixbuf;
	GdkPixbuf *scaled_pixbuf;
	gchar *size, *uri;
	GtkTreeIter *iter;

	/* display the Uploads window got from the plugin */
	uploads_window = uploads_get_dialog (plugin);
	gtk_widget_show_all (GTK_WIDGET (uploads_window));

	/* obtain the data describing the upload */
	/* TODO: submit patch with documentaiton for eog_image_get_*,
	   particularly what needs unrefing */
	uri = eog_image_get_uri_for_display (image);
	thumbnail_pixbuf = eog_image_get_thumbnail (image);
	if (thumbnail_pixbuf && GDK_IS_PIXBUF (thumbnail_pixbuf)) {
		scaled_pixbuf = gdk_pixbuf_scale_simple (thumbnail_pixbuf, 32, 32, GDK_INTERP_BILINEAR);
		g_object_unref (thumbnail_pixbuf);
	} else {
		/* This is currently a workaround due to limitations in eog's
		 * eog's thumbnailing mechanism */
		GError *error = NULL;
		GtkIconTheme *icon_theme;

		icon_theme = gtk_icon_theme_get_default ();

		scaled_pixbuf = gtk_icon_theme_load_icon (icon_theme,
							  "image-x-generic",
							  32, 0, &error);

		if (!scaled_pixbuf) {
			g_warning ("Couldn't load icon: %s", error->message);
			g_error_free (error);
		}
	}
	size = g_strdup_printf ("%" G_GOFFSET_FORMAT "KB",
				eog_image_get_bytes (image) / 1024);
	iter = g_slice_new0 (GtkTreeIter);

	/* insert the data into the upload's list store */
	gtk_list_store_insert_with_values (plugin->priv->uploads_store, iter, 0,
					   0, scaled_pixbuf,
					   1, uri,
					   2, size,
					   3, 50, /* upload status: set to G_MAXINT when done, to 0 to not start */
					   4, cancellable,
					   5, _("Uploading..."),
					   -1); /* TODO: where should cancellabe, scaled_pixbuf be unref'd? don't worry about it since
						   they'll exist until EoG exits anyway? or in eog_postasa_plugin_dispose()? */
	g_free (uri);
	g_free (size);
	g_object_unref (scaled_pixbuf);

	/* Set the progress bar to pulse every 50ms; freed in pulse() when upload is no longer in progress */
	PulseData *data; /* just needs to be freed with g_slice_free() */
	data = g_slice_new0 (PulseData);
	data->plugin = plugin;
	data->iter = *iter;
	g_timeout_add (50, (GSourceFunc)pulse, data);

	return iter;
}

static void
free_picasaweb_upload_file_async_data (PicasaWebUploadFileAsyncData *data)
{
	g_object_unref (data->imgfile);
	g_slice_free (GtkTreeIter, data->iter);
	g_slice_free (PicasaWebUploadFileAsyncData, data);
}

/**
 * picasaweb_upload_async_cb:
 *
 * Handles completion of the image's asynchronous upload to PicasaWeb.
 *
 * If the #GAsyncResults indicates success, we'll update the
 * treeview's row for the given upload indicating this.  Elsewise, if
 * it wasn't cancelled, then we report an error.
 *
 * NOTE: we also clean up the #PicasaWebUploadFileAsyncData here.
 *
 * TODO: we don't yet make the progress bar throb, how do we do that?
 *
 **/
static void
picasaweb_upload_async_cb (EogPostasaPlugin *plugin, GAsyncResult *res, PicasaWebUploadFileAsyncData *data)
{
	GCancellable* cancellable;
	GError *error = NULL; /* TODO: make sure to clear all set errors */

	if (g_simple_async_result_get_op_res_gboolean (G_SIMPLE_ASYNC_RESULT (res)) == TRUE) {
		gtk_list_store_set (plugin->priv->uploads_store, data->iter, 3, G_MAXINT, 5, _("Uploaded"), -1);
	} else {
		gtk_tree_model_get (GTK_TREE_MODEL (plugin->priv->uploads_store), data->iter, 4, &cancellable, -1);
		if (g_cancellable_is_cancelled (cancellable) == TRUE) {
			gtk_list_store_set (plugin->priv->uploads_store, data->iter, 3, -1, 5, _("Cancelled"), -1);
		} else {
			g_simple_async_result_propagate_error (G_SIMPLE_ASYNC_RESULT (res), &error);
			gtk_list_store_set (plugin->priv->uploads_store, data->iter, 3, -1, 5, error ? error->message : _("Failed"), -1);
			g_clear_error (&error);
		}
	}

	free_picasaweb_upload_file_async_data (data);
}

/*** PicasaWeb ***/

/**
 * tmp_picasaweb_upload_async:
 *
 * Temporary solution to provide asynchronous uploading and stop
 * blocking the UI until gdata_picasaweb_service_upload_file_async()
 * becomes available (bgo #600262).  This method does the synchronous
 * file upload, but is run asynchronously via
 * g_simple_async_result_run_in_thread().
 *
 * This sets up a minimal #GDataPicasaWebFile entry, using the
 * basename of the filepath for the file's title (which is not the
 * caption, but might be something we would consider doing).  The
 * image file and the minimal entry are then uploaded to PicasaWeb's
 * default album of "Drop Box".  In the future, we might consider
 * adding an Album Chooser to the Preferences/login window, but only
 * if there's demand.
 **/
static void
tmp_picasaweb_upload_async (GSimpleAsyncResult *result, GObject *source_object, GCancellable *cancellable)
{
	GDataPicasaWebFile *new_file = NULL;
	EogPostasaPlugin *plugin = EOG_POSTASA_PLUGIN (source_object);
	GDataPicasaWebService *service = plugin->priv->service;
	GDataPicasaWebFile *file_entry;
	PicasaWebUploadFileAsyncData *data;
#ifdef HAVE_LIBGDATA_0_8
	GDataUploadStream *upload_stream;
	GFileInputStream *in_stream;
	GFileInfo *file_info;
#endif
	gchar *filename;
	GError *error = NULL;

	data = (PicasaWebUploadFileAsyncData*)g_async_result_get_user_data (G_ASYNC_RESULT (result));

	/* get filename to set image title */
	file_entry = gdata_picasaweb_file_new (NULL);
	filename = g_file_get_basename (data->imgfile);
	gdata_entry_set_title (GDATA_ENTRY (file_entry), filename);
	g_free (filename);

#ifdef HAVE_LIBGDATA_0_8
	file_info = g_file_query_info (data->imgfile,
				      G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME ","
				      G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
				      G_FILE_QUERY_INFO_NONE, cancellable,
				      &error);

	if (file_info == NULL)
		goto got_err;

	upload_stream = gdata_picasaweb_service_upload_file (service,
				      NULL /* Upload to Dropbox */, file_entry,
				      g_file_info_get_display_name (file_info),
				      g_file_info_get_content_type (file_info),
				      cancellable, &error);
	g_object_unref (file_info);

	if (upload_stream == NULL)
		goto got_err;

	in_stream = g_file_read (data->imgfile, cancellable, &error);

	if (in_stream == NULL) {
		g_object_unref (upload_stream);
		goto got_err;
	}

	if (g_output_stream_splice (G_OUTPUT_STREAM (upload_stream),
				    G_INPUT_STREAM (in_stream),
				    G_OUTPUT_STREAM_SPLICE_CLOSE_SOURCE |
				    G_OUTPUT_STREAM_SPLICE_CLOSE_TARGET,
				    cancellable, &error) == -1)
	{
		g_object_unref (upload_stream);
		g_object_unref (in_stream);
		goto got_err;
	}


	new_file = gdata_picasaweb_service_finish_file_upload (service,
							       upload_stream,
							       &error);

	g_object_unref (upload_stream);
	g_object_unref (in_stream);
got_err:
	/* Jump here if any GIO/GData call doesn't return successfully.
	 * Error handling happens below. */

#else
	/* libgdata-0.6 */
	new_file = gdata_picasaweb_service_upload_file (service,
					       NULL /* Uploading to Drop Box */,
					       file_entry, data->imgfile,
					       cancellable, &error);
#endif
	g_object_unref (file_entry);

	if (new_file == NULL || error) {
		if (g_cancellable_is_cancelled (cancellable) == FALSE) {
			g_simple_async_result_set_from_error (result, error);
		}
		/* Clear errors always as cancelling creates errors too */
		g_clear_error (&error);
	} else {
		g_simple_async_result_set_op_res_gboolean (result, TRUE);
	}

	if (new_file != NULL)
		g_object_unref (new_file);
}


/**
 * picasaweb_upload_files:
 *
 * This obtains the list of selected images in EoG (selected in the
 * thumbview), sets up asynchronous uploads through
 * tmp_picasaweb_upload_async() in their own thread and instigates
 * them.

 * This attempts to upload the selected files.  It provides a message
 * near the end indicating the number successfully uploaded, and any
 * error messages encountered along the way.
 *
 * TODO: once gdata_picasaweb_service_upload_file_async() is available
 * from libgdata, simplify this as possible.
 **/
static void
picasaweb_upload_files (EogPostasaPlugin *plugin)
{
	EogWindow *window;
	GtkWidget *thumbview;
	GList *images, *node;
	EogImage *image;
	GFile *imgfile;
	GCancellable *cancellable;
	GSimpleAsyncResult *result;
	PicasaWebUploadFileAsyncData *data;

#ifdef HAVE_LIBGDATA_0_9
	if (gdata_service_is_authorized (GDATA_SERVICE (plugin->priv->service)) == FALSE) {
#else
	if (gdata_service_is_authenticated (GDATA_SERVICE (plugin->priv->service)) == FALSE) {
#endif
		g_warning ("PicasaWeb could not be authenticated.  Aborting upload.");
		return;
	}

	window = plugin->priv->eog_window;
	thumbview = eog_window_get_thumb_view (window); /* do not unref */
	images = eog_thumb_view_get_selected_images (EOG_THUMB_VIEW (thumbview)); /* need to use g_list_free() */

	for (node = g_list_first (images); node != NULL; node = node->next) {
		image = (EogImage *) node->data;
		cancellable = g_cancellable_new (); /* TODO: this gets attached to the image's list row; free with row */

		imgfile = eog_image_get_file (image); /* unref this */

		data = g_slice_new0(PicasaWebUploadFileAsyncData); /* freed by picasaweb_upload_async_cb() or below */
		data->imgfile = g_file_dup (imgfile); /* unref'd in free_picasaweb_upload_file_async_data() */
		data->iter = uploads_add_entry (plugin, image, cancellable); /* freed with data */

		if (g_file_query_exists (imgfile, cancellable)) {
			/* TODO: want to replace much of this with gdata_picasaweb_service_upload_file_async when that's avail */
			result = g_simple_async_result_new (G_OBJECT (plugin), (GAsyncReadyCallback)picasaweb_upload_async_cb,
							    data, tmp_picasaweb_upload_async); /* TODO: should this be freed? where? */
			g_simple_async_result_run_in_thread (result, tmp_picasaweb_upload_async, 0, cancellable);
		} else {
			/* TODO: consider setting a proper error and passing it in the data through GSimpleAsyncResult's thread */
			gtk_list_store_set (plugin->priv->uploads_store, data->iter, 3, -1, 5, "File not found", -1);
			free_picasaweb_upload_file_async_data (data);
		}
		g_object_unref (imgfile);
	}
	g_list_free (images);
}

/**
 * picasaweb_login_async_cb:
 *
 * Handles the result of the asynchronous
 * gdata_service_authenticate_async() operation, called by our
 * picasaweb_login_cb().  Upon success, it switches "Cancel" to
 * "Close".  Regardless of the response, it re-enables the Login
 * button (which is desensitised during the login attempt).
 **/
#ifdef HAVE_LIBGDATA_0_9
static void
picasaweb_login_async_cb (GDataClientLoginAuthorizer *authorizer, GAsyncResult *result, EogPostasaPlugin *plugin)
#else
static void
picasaweb_login_async_cb (GDataPicasaWebService *service, GAsyncResult *result, EogPostasaPlugin *plugin)
#endif
{
	GError *error = NULL;
	gchar *message;
	gboolean success = FALSE;

#ifdef HAVE_LIBGDATA_0_9
	success = gdata_client_login_authorizer_authenticate_finish (authorizer,
								     result,
								     &error);
#else
	success = gdata_service_authenticate_finish (GDATA_SERVICE (service), result, &error);
#endif

	gtk_widget_set_sensitive (GTK_WIDGET (plugin->priv->login_button), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (plugin->priv->username_entry), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (plugin->priv->password_entry), TRUE);

	if (success == FALSE || error != NULL) {
		message = g_strdup_printf (_("Login failed. %s"), error->message);
		gtk_label_set_text (plugin->priv->login_message, message);
		g_free (message);
	} else {
		gtk_label_set_text (plugin->priv->login_message, _("Logged in successully."));
		gtk_button_set_label (plugin->priv->cancel_button, _("Close"));
		login_dialog_close (plugin);
	}
}

/**
 * picasaweb_login_cb:
 *
 * Handles "clicked" for the Login button.  Attempts to use input from
 * the username and password entries to authenticate.  It does this
 * asynchronously, leaving it to @picasaweb_login_async_cb to handle
 * the result.  It disables the Login button while authenticating
 * (re-enabled when done) and ensures the Close/Cancel button says
 * Cancel (which, incidentally, will call g_cancellable_cancel on the
 * provided #GCancellable.
 **/
static void
picasaweb_login_cb (GtkWidget *login_button, gpointer _plugin)
{
	EogPostasaPlugin *plugin = EOG_POSTASA_PLUGIN (_plugin);

	gtk_button_set_label (plugin->priv->cancel_button, _("Cancel"));
	gtk_widget_set_sensitive (login_button, FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (plugin->priv->username_entry), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (plugin->priv->password_entry), FALSE);

	/* TODO: want to handle passwords more securely */
	gtk_label_set_text (plugin->priv->login_message, _("Logging in..."));
	g_cancellable_reset (plugin->priv->login_cancellable);

#ifdef HAVE_LIBGDATA_0_9
	gdata_client_login_authorizer_authenticate_async (
					  plugin->priv->authorizer,
					  gtk_entry_get_text (plugin->priv->username_entry),
					  gtk_entry_get_text (plugin->priv->password_entry),
					  plugin->priv->login_cancellable, (GAsyncReadyCallback)picasaweb_login_async_cb, plugin);
#else
	gdata_service_authenticate_async (GDATA_SERVICE (plugin->priv->service),
					  gtk_entry_get_text (plugin->priv->username_entry),
					  gtk_entry_get_text (plugin->priv->password_entry),
					  plugin->priv->login_cancellable, (GAsyncReadyCallback)picasaweb_login_async_cb, plugin);
#endif
}

/**
 * picasaweb_upload_cb:
 *
 * This checks that we are authenticated (popping up the login window
 * if we're not) and, if we are, moves on to upload the files.
 **/
static void
picasaweb_upload_cb (GtkAction	*action,
		     EogPostasaPlugin *plugin)
{
	EogPostasaPluginPrivate *priv;

	g_return_if_fail (EOG_IS_POSTASA_PLUGIN (plugin));

	priv = plugin->priv;

#ifdef HAVE_LIBGDATA_0_9
	if (gdata_service_is_authorized (GDATA_SERVICE (priv->service)) == TRUE)
#else
	if (gdata_service_is_authenticated (GDATA_SERVICE (priv->service)) == TRUE)
#endif
	{
		picasaweb_upload_files (plugin);
	} else {
		/* when the dialog closes, it checks if this is set to see if it should upload anything */
		priv->uploads_pending = TRUE;

		login_get_dialog (plugin);
		gtk_label_set_text (priv->login_message, _("Please log in to continue upload."));
		gtk_window_present (GTK_WINDOW (priv->login_dialog));
	}
}


/*** Login Dialog ***/

/**
 * login_dialog_close:
 *
 * Closes the login dialog, used for closing the window or cancelling
 * login.  This will also cancel any authentication in progress.  If
 * the login dialog was prompted by an upload attempt, it will resume
 * the upload attempt.
 **/
static gboolean
login_dialog_close (EogPostasaPlugin *plugin)
{
	/* abort the authentication attempt if in progress and we're cancelling */
	g_cancellable_cancel (plugin->priv->login_cancellable);
	gtk_widget_hide (GTK_WIDGET (plugin->priv->login_dialog));

	if (plugin->priv->uploads_pending == TRUE) {
		plugin->priv->uploads_pending = FALSE;
		picasaweb_upload_files (plugin);
	}

	return TRUE;
}

/**
 * login_dialog_cancel_button_cb:
 *
 * Handles clicks on the Cancel/Close button.
 **/
static gboolean
login_dialog_cancel_button_cb (GtkWidget *cancel_button, EogPostasaPlugin *plugin)
{
	/* Make sure we don't resume uploads */
	plugin->priv->uploads_pending = FALSE;

	return login_dialog_close (plugin);
}

/**
 * login_dialog_delete_event_cb:
 *
 * Handles other attempts to close the dialog. (e.g. window manager)
 **/
static gboolean
login_dialog_delete_event_cb (GtkWidget *widget, GdkEvent *event, gpointer *_plugin)
{
	/* Make sure we don't resume uploads */
	EOG_POSTASA_PLUGIN (_plugin)->priv->uploads_pending = FALSE;

	return login_dialog_close (EOG_POSTASA_PLUGIN (_plugin));
}

/**
 * login_get_dialog:
 *
 * Retrieves the login dialog.  If it has not yet been constructed, it
 * does so.  If the user is already authenticated, it populates the
 * username and password boxes with the relevant values.
 **/
static GtkWidget *
login_get_dialog (EogPostasaPlugin *plugin)
{
	GtkBuilder *builder;
	GError *error = NULL;

	if (plugin->priv->login_dialog == NULL) {
		builder = gtk_builder_new ();
		gtk_builder_set_translation_domain (builder, GETTEXT_PACKAGE);
		gtk_builder_add_from_resource (builder, GTKBUILDER_CONFIG_FILE,
		                               &error);
		if (error != NULL) {
			g_warning ("Couldn't load Postasa configuration UI file:%d:%s", error->code, error->message);
			g_error_free (error);
		}

		/* do not unref gtk_builder_get_object() returns */
		plugin->priv->username_entry = GTK_ENTRY  (gtk_builder_get_object (builder, "username_entry"));
		plugin->priv->password_entry = GTK_ENTRY  (gtk_builder_get_object (builder, "password_entry"));
		plugin->priv->login_dialog   = GTK_DIALOG (gtk_builder_get_object (builder, "postasa_login_dialog"));
		plugin->priv->cancel_button  = GTK_BUTTON (gtk_builder_get_object (builder, "cancel_button"));
		plugin->priv->login_button   = GTK_BUTTON (gtk_builder_get_object (builder, "login_button"));
		plugin->priv->login_message  = GTK_LABEL  (gtk_builder_get_object (builder, "login_message"));

		g_object_unref (builder);

		g_signal_connect (G_OBJECT (plugin->priv->login_button),  "clicked", G_CALLBACK (picasaweb_login_cb),     plugin);
		g_signal_connect (G_OBJECT (plugin->priv->cancel_button), "clicked", G_CALLBACK (login_dialog_cancel_button_cb), plugin);
		g_signal_connect (G_OBJECT (plugin->priv->login_dialog), "delete-event", G_CALLBACK (login_dialog_delete_event_cb), plugin);

#ifdef HAVE_LIBGDATA_0_9
		if (gdata_service_is_authorized (GDATA_SERVICE (plugin->priv->service))) {
			gtk_entry_set_text (plugin->priv->username_entry, gdata_client_login_authorizer_get_username (plugin->priv->authorizer));
			gtk_entry_set_text (plugin->priv->password_entry, gdata_client_login_authorizer_get_password (plugin->priv->authorizer));
#else
		if (gdata_service_is_authenticated (GDATA_SERVICE (plugin->priv->service))) {
			gtk_entry_set_text (plugin->priv->username_entry, gdata_service_get_username (GDATA_SERVICE (plugin->priv->service)));
			gtk_entry_set_text (plugin->priv->password_entry, gdata_service_get_password (GDATA_SERVICE (plugin->priv->service)));
#endif
		}
	}

	return GTK_WIDGET (plugin->priv->login_dialog);
}


/*** EogPlugin Functions ***/

/**
 * impl_activate:
 *
 * Plugin hook for plugin activation.  Creates #WindowData for the
 * #EogPostasaPlugin that gets associated with the window and defines
 * some UI.
 **/
static void
impl_activate (EogWindowActivatable *activatable)
{
	EogPostasaPlugin *plugin = EOG_POSTASA_PLUGIN (activatable);
	EogPostasaPluginPrivate *priv = plugin->priv;
	GtkUIManager *manager;
	EogWindow *window;

	eog_debug (DEBUG_PLUGINS);

	window = priv->eog_window;

	priv->ui_action_group = gtk_action_group_new ("EogPostasaPluginActions");
	gtk_action_group_set_translation_domain (priv->ui_action_group,
						 GETTEXT_PACKAGE);
	gtk_action_group_add_actions (priv->ui_action_group,
				      action_entries,
				      G_N_ELEMENTS (action_entries), plugin);

	manager = eog_window_get_ui_manager (window); /* do not unref */
	gtk_ui_manager_insert_action_group (manager, priv->ui_action_group, -1);
	priv->ui_id = gtk_ui_manager_add_ui_from_string (manager,
							 ui_definition,
							 -1, NULL);
	g_warn_if_fail (priv->ui_id != 0);
}

/**
 * impl_deactivate:
 *
 * Plugin hook for plugin deactivation. Removes UI and #WindowData
 **/
static void
impl_deactivate	(EogWindowActivatable *activatable)
{
	EogPostasaPlugin *plugin = EOG_POSTASA_PLUGIN (activatable);
	EogPostasaPluginPrivate *priv = plugin->priv;
	GtkUIManager *manager;

	eog_debug (DEBUG_PLUGINS);

	manager = eog_window_get_ui_manager (priv->eog_window);

	gtk_ui_manager_remove_ui (manager, priv->ui_id);
	gtk_ui_manager_remove_action_group (manager, priv->ui_action_group);

	priv->ui_action_group = NULL;
	priv->ui_id = 0;
}

/*** GObject Functions ***/

/**
 * eog_postasa_plugin_init:
 *
 * Object initialisation method.  Sets up the (unauthenticated)
 * PicasaWeb service, a #GCancellable for login, and sets the
 * uploads_pending flag to %FALSE.
 **/
static void
eog_postasa_plugin_init (EogPostasaPlugin *plugin)
{
	eog_debug_message (DEBUG_PLUGINS, "EogPostasaPlugin initializing");

	plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin, EOG_TYPE_POSTASA_PLUGIN, EogPostasaPluginPrivate);

#ifdef HAVE_LIBGDATA_0_9
	plugin->priv->authorizer = gdata_client_login_authorizer_new ("EogPostasa", GDATA_TYPE_PICASAWEB_SERVICE);
	plugin->priv->service = gdata_picasaweb_service_new (GDATA_AUTHORIZER (plugin->priv->authorizer)); /* unref'd in eog_postasa_plugin_dispose() */
#else
	plugin->priv->service = gdata_picasaweb_service_new ("EogPostasa"); /* unref'd in eog_postasa_plugin_dispose() */
#endif
	plugin->priv->login_cancellable = g_cancellable_new (); /* unref'd in eog_postasa_plugin_dispose() */
	plugin->priv->uploads_pending = FALSE;
}

/**
 * eog_postasa_plugin_dispose:
 *
 * Cleans up the #EogPostasaPlugin object, unref'ing its #GDataPicasaWebService and #GCancellable.
 **/
static void
eog_postasa_plugin_dispose (GObject *_plugin)
{
	EogPostasaPlugin *plugin = EOG_POSTASA_PLUGIN (_plugin);

	eog_debug_message (DEBUG_PLUGINS, "EogPostasaPlugin disposing");

#ifdef HAVE_LIBGDATA_0_9
	if (plugin->priv->authorizer) {
		g_object_unref (plugin->priv->authorizer);
		plugin->priv->authorizer = NULL;
	}
#endif

	if (plugin->priv->service) {
		g_object_unref (plugin->priv->service);
		plugin->priv->service = NULL;
	}
	if (plugin->priv->login_cancellable) {
		g_object_unref (plugin->priv->login_cancellable);
		plugin->priv->login_cancellable = NULL;
	}
	if (G_IS_OBJECT (plugin->priv->uploads_store)) {
		/* we check in case the upload window was never created */
		g_object_unref (plugin->priv->uploads_store);
		plugin->priv->uploads_store = NULL;
	}

	if (plugin->priv->eog_window) {
		g_object_unref (plugin->priv->eog_window);
		plugin->priv->eog_window = NULL;
	}

	G_OBJECT_CLASS (eog_postasa_plugin_parent_class)->dispose (_plugin);
}

static void
eog_postasa_plugin_get_property (GObject    *object,
				 guint       prop_id,
				 GValue     *value,
				 GParamSpec *pspec)
{
	EogPostasaPlugin *plugin = EOG_POSTASA_PLUGIN (object);

	switch (prop_id)
	{
	case PROP_WINDOW:
		g_value_set_object (value, plugin->priv->eog_window);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
eog_postasa_plugin_set_property (GObject      *object,
				 guint         prop_id,
				 const GValue *value,
				 GParamSpec   *pspec)
{
	EogPostasaPlugin *plugin = EOG_POSTASA_PLUGIN (object);

	switch (prop_id)
	{
	case PROP_WINDOW:
		plugin->priv->eog_window = EOG_WINDOW (g_value_dup_object (value));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

/**
 * eog_postasa_plugin_class_init:
 *
 * Plugin class initialisation.  Binds class hooks to actual implementations.
 **/
static void
eog_postasa_plugin_class_init (EogPostasaPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (EogPostasaPluginPrivate));

	object_class->dispose = eog_postasa_plugin_dispose;
	object_class->set_property = eog_postasa_plugin_set_property;
	object_class->get_property = eog_postasa_plugin_get_property;

	g_object_class_override_property (object_class, PROP_WINDOW, "window");
}

static void
eog_postasa_plugin_class_finalize (EogPostasaPluginClass *klass)
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
	eog_postasa_plugin_register_type (G_TYPE_MODULE (module));
	peas_object_module_register_extension_type (module,
						    EOG_TYPE_WINDOW_ACTIVATABLE,
						    EOG_TYPE_POSTASA_PLUGIN);
}
