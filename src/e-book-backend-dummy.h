/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* e-book-backend-dummy.h - dummy contact backend.
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

#ifndef __E_BOOK_BACKEND_DUMMY_H__
#define __E_BOOK_BACKEND_DUMMY_H__

#include <libedata-book/e-book-backend-sync.h>

#define E_TYPE_BOOK_BACKEND_DUMMY        (e_book_backend_dummy_get_type ())
#define E_BOOK_BACKEND_DUMMY(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), E_TYPE_BOOK_BACKEND_DUMMY, EBookBackendDummy))
#define E_BOOK_BACKEND_DUMMY_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), E_TYPE_BOOK_BACKEND_DUMMY, EBookBackendDummyClass))
#define E_IS_BOOK_BACKEND_DUMMY(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), E_TYPE_BOOK_BACKEND_DUMMY))
#define E_IS_BOOK_BACKEND_DUMMY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), E_TYPE_BOOK_BACKEND_DUMMY))
#define E_BOOK_BACKEND_DUMMY_GET_CLASS(k) (G_TYPE_INSTANCE_GET_CLASS ((obj), E_TYPE_BOOK_BACKEND_DUMMY, EBookBackendDummyClass))

typedef struct {
	EBookBackendSync         parent_object;
} EBookBackendDummy;

typedef struct {
	EBookBackendSyncClass parent_class;
} EBookBackendDummyClass;

EBookBackend *e_book_backend_dummy_new      (void);
GType         e_book_backend_dummy_get_type (void);

#endif /* __E_BOOK_BACKEND_DUMMY_H__ */
