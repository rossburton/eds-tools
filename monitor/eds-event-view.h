#ifndef _EDS_EVENT_VIEW
#define _EDS_EVENT_VIEW

#include <gtk/gtkhpaned.h>

G_BEGIN_DECLS

#define EDS_TYPE_EVENT_VIEW eds_event_view_get_type()

#define EDS_EVENT_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  EDS_TYPE_EVENT_VIEW, EdsEventView))

#define EDS_EVENT_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  EDS_TYPE_EVENT_VIEW, EdsEventViewClass))

#define EDS_IS_EVENT_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  EDS_TYPE_EVENT_VIEW))

#define EDS_IS_EVENT_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  EDS_TYPE_EVENT_VIEW))

#define EDS_EVENT_VIEW_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  EDS_TYPE_EVENT_VIEW, EdsEventViewClass))

typedef struct _EdsEventViewPrivate EdsEventViewPrivate;

typedef struct {
  GtkHPaned parent;
  EdsEventViewPrivate *priv;
} EdsEventView;

typedef struct {
  GtkHPanedClass parent_class;
} EdsEventViewClass;

GType eds_event_view_get_type (void);

GtkWidget* eds_event_view_new (void);

void eds_event_view_set_book_view (EdsEventView *view, EBookView *bookview);

G_END_DECLS

#endif /* _EDS_EVENT_VIEW */
