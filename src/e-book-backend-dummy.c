/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* e-book-backend-dummy.c - Dummy contact backend.
 *
 * Copyright (C) 2007 Ross Burton
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of version 2 of the GNU General Public License as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Author: Ross Burton <ross@openedhand.com>
 */

#include <config.h> 

#include <glib.h>

#include <libedataserver/e-data-server-util.h>
#include <libebook/e-contact.h>
#include <libedata-book/e-data-book.h>
#include <libedata-book/e-data-book-view.h>

#include "e-book-backend-dummy.h"

static const char* vcard_template = 
	"BEGIN:VCARD\n"
	"VERSION:3.0\n"
	"UID:%s\n"
	"FN:Test Contact\n"
	"N:Contact;Test\n"
	"EMAIL;TYPE=WORK:test@example.com\n"
	"EMAIL;TYPE=HOME:test2@example.com\n"
	"X-JABBER:test@jabber.example.com\n"
	"END:VCARD\n";


G_DEFINE_TYPE (EBookBackendDummy, e_book_backend_dummy, E_TYPE_BOOK_BACKEND_SYNC);

static GQuark view_data;

static EBookBackendSyncStatus
e_book_backend_dummy_create_contact (EBookBackendSync *backend,
				    EDataBook *book,
				    guint32 opid,
				    const char *vcard,
				    EContact **contact)
{
	return GNOME_Evolution_Addressbook_PermissionDenied;
}

static EBookBackendSyncStatus
e_book_backend_dummy_remove_contacts (EBookBackendSync *backend,
				     EDataBook *book,
				     guint32 opid,
				     GList *id_list,
				     GList **ids)
{
	return GNOME_Evolution_Addressbook_PermissionDenied;
}

static EBookBackendSyncStatus
e_book_backend_dummy_modify_contact (EBookBackendSync *backend,
				    EDataBook *book,
				    guint32 opid,
				    const char *vcard,
				    EContact **contact)
{
	return GNOME_Evolution_Addressbook_PermissionDenied;
}

static EBookBackendSyncStatus
e_book_backend_dummy_get_contact (EBookBackendSync *backend,
				 EDataBook *book,
				 guint32 opid,
				 const char *id,
				 char **vcard)
{
	*vcard = g_strdup_printf (vcard_template, id);
	return GNOME_Evolution_Addressbook_Success;
}

static EBookBackendSyncStatus
e_book_backend_dummy_get_contact_list (EBookBackendSync *backend,
				      EDataBook *book,
				      guint32 opid,
				      const char *query,
				      GList **contacts)
{
	GList *list = NULL;
	int i;
	
	/* TODO: replace 10 with an argument from the source somehow */
	for (i = 0; i < 10; i++) {
		char *id;
		id = g_strdup_printf ("dummy-%d", i);
		list = g_list_prepend (list, g_strdup_printf (vcard_template, id));
		g_free (id);
	}

	*contacts = list;

	return GNOME_Evolution_Addressbook_Success;
}

typedef struct {
	EBookBackendDummy *backend;
	GMutex *mutex;
	GCond *cond;
	gboolean stop;
} ViewData;

static void
view_data_destroy (ViewData *data)
{
	g_mutex_free (data->mutex);
	g_cond_free (data->cond);
	g_slice_free (ViewData, data);
}

static gpointer
book_view_thread (gpointer user_data)
{
	EDataBookView *book_view;
	ViewData *data;
	int i;

	book_view = E_DATA_BOOK_VIEW (user_data);
	g_assert (book_view);

	data = g_object_get_qdata (G_OBJECT (book_view), view_data);
	g_assert (data);

	g_object_ref (book_view);

	e_data_book_view_notify_status_message (book_view, "Loading...");

	g_mutex_lock (data->mutex);
	g_cond_signal (data->cond);
	g_mutex_unlock (data->mutex);

	for (i = 0; i < 100000; i++) {
		char *id, *vcard;
		id = g_strdup_printf ("dummy-%d", i);
		vcard = g_strdup_printf (vcard_template, id);
		e_data_book_view_notify_update_prefiltered_vcard (book_view, id, vcard);
	}
	e_data_book_view_notify_complete (book_view, GNOME_Evolution_Addressbook_Success);
	
	g_object_unref (book_view);

	return NULL;
}

static void
e_book_backend_dummy_start_book_view (EBookBackend  *backend,
				     EDataBookView *book_view)
{
	ViewData *data;
	GThread *thread;

	g_return_if_fail (E_IS_BOOK_BACKEND_DUMMY (backend));
	g_return_if_fail (E_IS_DATA_BOOK_VIEW (book_view));

	data = g_slice_new (ViewData);
	data->backend = E_BOOK_BACKEND_DUMMY (backend);
	data->mutex = g_mutex_new ();
	data->cond = g_cond_new ();
	data->stop = FALSE;

	g_object_set_qdata_full (G_OBJECT (book_view), view_data,
				 data, (GDestroyNotify)view_data_destroy);
	
	g_mutex_lock (data->mutex);
	thread = g_thread_create (book_view_thread, book_view, FALSE, NULL);
	g_cond_wait (data->cond, data->mutex);
	g_mutex_unlock (data->mutex);
}

static void
e_book_backend_dummy_stop_book_view (EBookBackend  *backend,
				     EDataBookView *book_view)
{
	ViewData *data = g_object_get_qdata (G_OBJECT (book_view), view_data);
	g_assert (data);

	g_mutex_lock (data->mutex);
	data->stop = TRUE;
	g_mutex_unlock (data->mutex);
}

static EBookBackendSyncStatus
e_book_backend_dummy_get_changes (EBookBackendSync *backend,
				 EDataBook *book,
				 guint32 opid,
				 const char *change_id,
				 GList **changes_out)
{
	return GNOME_Evolution_Addressbook_OtherError;
}

static EBookBackendSyncStatus
e_book_backend_dummy_authenticate_user (EBookBackendSync *backend,
				       EDataBook *book,
				       guint32 opid,
				       const char *user,
				       const char *passwd,
				       const char *auth_method)
{
	return GNOME_Evolution_Addressbook_Success;
}

static EBookBackendSyncStatus
e_book_backend_dummy_get_required_fields (EBookBackendSync *backend,
					  EDataBook *book,
					  guint32 opid,
					  GList **fields_out)
{
	return GNOME_Evolution_Addressbook_OtherError;
}

static EBookBackendSyncStatus
e_book_backend_dummy_get_supported_fields (EBookBackendSync *backend,
					  EDataBook *book,
					  guint32 opid,
					  GList **fields_out)
{
	return GNOME_Evolution_Addressbook_OtherError;
}

static GNOME_Evolution_Addressbook_CallStatus
e_book_backend_dummy_load_source (EBookBackend           *backend,
				 ESource                *source,
				 gboolean                only_if_exists)
{
	e_book_backend_set_is_loaded (backend, TRUE);
	e_book_backend_set_is_writable (backend, FALSE);
	return GNOME_Evolution_Addressbook_Success;
}

static EBookBackendSyncStatus
e_book_backend_dummy_remove (EBookBackendSync *backend,
			    EDataBook        *book,
			    guint32           opid)
{
	return GNOME_Evolution_Addressbook_PermissionDenied;
}

static char *
e_book_backend_dummy_get_static_capabilities (EBookBackend *backend)
{
	return g_strdup ("local");
}

static GNOME_Evolution_Addressbook_CallStatus
e_book_backend_dummy_cancel_operation (EBookBackend *backend, EDataBook *book)
{
	return GNOME_Evolution_Addressbook_CouldNotCancel;
}

static void 
e_book_backend_dummy_set_mode (EBookBackend *backend, GNOME_Evolution_Addressbook_BookMode mode)
{
	if (e_book_backend_is_loaded (backend)) {
		e_book_backend_notify_writable (backend, FALSE);
		e_book_backend_notify_connection_status (backend, TRUE);
	}
}

static void
e_book_backend_dummy_dispose (GObject *object)
{
	G_OBJECT_CLASS (e_book_backend_dummy_parent_class)->dispose (object);	
}

static void
e_book_backend_dummy_finalize (GObject *object)
{
	G_OBJECT_CLASS (e_book_backend_dummy_parent_class)->finalize (object);
}

static void
e_book_backend_dummy_class_init (EBookBackendDummyClass *klass)
{
	GObjectClass    *object_class = G_OBJECT_CLASS (klass);
	EBookBackendSyncClass *sync_class;
	EBookBackendClass *backend_class;

	sync_class = E_BOOK_BACKEND_SYNC_CLASS (klass);
	backend_class = E_BOOK_BACKEND_CLASS (klass);

	/* Set the virtual methods. */
	backend_class->load_source             = e_book_backend_dummy_load_source;
	backend_class->get_static_capabilities = e_book_backend_dummy_get_static_capabilities;
	backend_class->start_book_view         = e_book_backend_dummy_start_book_view;
	backend_class->stop_book_view          = e_book_backend_dummy_stop_book_view;
	backend_class->cancel_operation        = e_book_backend_dummy_cancel_operation;
	backend_class->set_mode                = e_book_backend_dummy_set_mode;
	sync_class->remove_sync                = e_book_backend_dummy_remove;
	sync_class->create_contact_sync        = e_book_backend_dummy_create_contact;
	sync_class->remove_contacts_sync       = e_book_backend_dummy_remove_contacts;
	sync_class->modify_contact_sync        = e_book_backend_dummy_modify_contact;
	sync_class->get_contact_sync           = e_book_backend_dummy_get_contact;
	sync_class->get_contact_list_sync      = e_book_backend_dummy_get_contact_list;
	sync_class->get_changes_sync           = e_book_backend_dummy_get_changes;
	sync_class->authenticate_user_sync     = e_book_backend_dummy_authenticate_user;
	sync_class->get_supported_fields_sync  = e_book_backend_dummy_get_supported_fields;
	sync_class->get_required_fields_sync   = e_book_backend_dummy_get_required_fields;
	
	object_class->dispose = e_book_backend_dummy_dispose;
	object_class->finalize = e_book_backend_dummy_finalize;

	view_data = g_quark_from_static_string ("EBookBackendDummy:ViewData");
}

static void
e_book_backend_dummy_init (EBookBackendDummy *backend)
{
}

EBookBackend *
e_book_backend_dummy_new (void)
{
	EBookBackendDummy *backend;

	backend = g_object_new (E_TYPE_BOOK_BACKEND_DUMMY, NULL);

	return E_BOOK_BACKEND (backend);
}
