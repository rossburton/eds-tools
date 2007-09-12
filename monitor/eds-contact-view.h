#ifndef _EDS_CONTACT_VIEW
#define _EDS_CONTACT_VIEW

#include <gtk/gtkhpaned.h>
#include <libebook/e-book.h>

G_BEGIN_DECLS

#define EDS_TYPE_CONTACT_VIEW eds_contact_view_get_type()

#define EDS_CONTACT_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  EDS_TYPE_CONTACT_VIEW, EdsContactView))

#define EDS_CONTACT_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  EDS_TYPE_CONTACT_VIEW, EdsContactViewClass))

#define EDS_IS_CONTACT_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  EDS_TYPE_CONTACT_VIEW))

#define EDS_IS_CONTACT_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  EDS_TYPE_CONTACT_VIEW))

#define EDS_CONTACT_VIEW_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  EDS_TYPE_CONTACT_VIEW, EdsContactViewClass))

typedef struct _EdsContactViewPrivate EdsContactViewPrivate;

typedef struct {
  GtkHPaned parent;
  EdsContactViewPrivate *priv;
} EdsContactView;

typedef struct {
  GtkHPanedClass parent_class;
} EdsContactViewClass;

GType eds_contact_view_get_type (void);

GtkWidget* eds_contact_view_new (void);

void eds_contact_view_set_book (EdsContactView *view, EBook *book);

void eds_contact_view_show_query (EdsContactView *view, EBookQuery *query);

void eds_contact_view_show_query_sexp (EdsContactView *view, const char *query);

void eds_contact_view_set_book_view (EdsContactView *view, EBookView *bookview);

G_END_DECLS

#endif /* _EDS_CONTACT_VIEW */
