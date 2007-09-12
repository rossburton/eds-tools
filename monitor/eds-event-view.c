#include <gtk/gtk.h>
#include <libebook/e-book.h>
#include "eds-event-view.h"

G_DEFINE_TYPE (EdsEventView, eds_event_view, GTK_TYPE_HPANED);

enum {
  COL_TIME,
  COL_TYPE,
  COL_DATA, /* create a box or object for this */
};

typedef enum {
  TYPE_STATUS,
  TYPE_ADDED,
  TYPE_CHANGED,
  TYPE_REMOVED,
  TYPE_COMPLETE,
} EventType;

typedef struct {
  EContact *before;
  EContact *after;
} ContactChange;

#define EVENT_VIEW_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EDS_TYPE_EVENT_VIEW, EdsEventViewPrivate))

struct _EdsEventViewPrivate
{
  EBookView *bookview;
  /* Hash of UID strings to EContact objects.  Dup the UID on insertion, on
     removal they get free'd automatically. */
  GHashTable *contacts;
  /* COL_TIME, COL_TYPE, COL_DATA */
  GtkListStore *events;
  GtkTextBuffer *details;
};

static glong
get_seconds (void)
{
  GTimeVal time = {0, 0};
  g_get_current_time (&time);
  return time.tv_sec;
}

static void
on_status_message (EBookView *bookview, char *message, EdsEventView *view)
{
  GtkTreeIter iter;
  gtk_list_store_append (view->priv->events, &iter);
  gtk_list_store_set (view->priv->events, &iter,
                      COL_TIME, get_seconds(),
                      COL_TYPE, TYPE_STATUS,
                      COL_DATA, g_strdup (message),
                      -1);
}

static void
on_contacts_added (EBookView *bookview, GList *list, EdsEventView *view)
{
  for (; list; list = g_list_next (list)) {
    EContact *contact = list->data;
    const char *uid;
    ContactChange *change;
    GtkTreeIter iter;

    uid = e_contact_get_const (contact, E_CONTACT_UID);
    g_hash_table_insert (view->priv->contacts, g_strdup (uid), contact);

    change = g_slice_new0 (ContactChange);
    change->before = NULL;
    change->after = g_object_ref (contact);

    gtk_list_store_append (view->priv->events, &iter);
    gtk_list_store_set (view->priv->events, &iter,
                        COL_TIME, get_seconds(),
                        COL_TYPE, TYPE_ADDED,
                        COL_DATA, change,
                        -1);
  }
}

static void
on_contacts_changed (EBookView *bookview, GList *list, EdsEventView *view)
{
  for (; list; list = g_list_next (list)) {
    EContact *contact = list->data;
    const char *uid;
    ContactChange *change;
    GtkTreeIter iter;

    uid = e_contact_get_const (contact, E_CONTACT_UID);

    change = g_slice_new0 (ContactChange);
    change->before = g_object_ref (g_hash_table_lookup (view->priv->contacts, uid));
    change->after = g_object_ref (contact);

    gtk_list_store_append (view->priv->events, &iter);
    gtk_list_store_set (view->priv->events, &iter,
                        COL_TIME, get_seconds(),
                        COL_TYPE, TYPE_CHANGED,
                        COL_DATA, change,
                        -1);

    g_hash_table_insert (view->priv->contacts, g_strdup (uid), contact);
  }
}

static void
on_contacts_removed (EBookView *bookview, GList *list, EdsEventView *view)
{
  for (; list; list = g_list_next (list)) {
    char *uid = list->data;
    EContact *contact;
    ContactChange *change;
    GtkTreeIter iter;

    contact = g_hash_table_lookup (view->priv->contacts, uid);
    
    change = g_slice_new0 (ContactChange);
    change->before = g_object_ref (contact);
    change->after = NULL;

    gtk_list_store_append (view->priv->events, &iter);
    gtk_list_store_set (view->priv->events, &iter,
                        COL_TIME, get_seconds(),
                        COL_TYPE, TYPE_REMOVED,
                        COL_DATA, change,
                        -1);

    g_hash_table_remove (view->priv->contacts, uid);
  }
}

static void
on_sequence_complete (EBookView *bookview, EBookStatus status, EdsEventView *view)
{
  GtkTreeIter iter;
  gtk_list_store_append (view->priv->events, &iter);
  gtk_list_store_set (view->priv->events, &iter,
                      COL_TIME, get_seconds(),
                      COL_TYPE, TYPE_COMPLETE,
                      COL_DATA, status,
                      -1);
}

/* Function to set the text of the cell in the Events list */
static void
event_data_func (GtkTreeViewColumn *column, GtkCellRenderer *cell,
                 GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
  EventType type = -1;
  glong seconds;
  char *detail = NULL;

  gtk_tree_model_get (model, iter,
                      COL_TIME, &seconds,
                      COL_TYPE, &type,
                      -1);
  
  switch (type) {
    /* TODO: display more information in the cell, like status code or contact name */
  case TYPE_STATUS:
    detail = g_strdup ("Status");
    break;
  case TYPE_ADDED:
    detail = g_strdup ("Added");
    break;
  case TYPE_CHANGED:
    detail = g_strdup ("Changed");
    break;
  case TYPE_REMOVED:
    detail = g_strdup ("Removed");
    break;
  case TYPE_COMPLETE:
    detail = g_strdup ("Complete");
    break;
  default:
    g_warning ("Unhandled event type %d", type);
  }

  if (detail) {
    char t[11], *s;
    struct tm *tm;
    tm = localtime(&seconds);
    strftime (t, sizeof(t), "[%T]", tm);
    s = g_strconcat (t, " ", detail, NULL);
    g_object_set (cell, "text", s, NULL);
    g_free (detail);
    g_free (s);
  }
}

static void
on_selection_changed (GtkTreeSelection *selection, EdsEventView *view)
{
  GtkTreeModel *model;
  GtkTreeIter iter;

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
    EventType type;
    gpointer p;

    gtk_tree_model_get (model, &iter, COL_TYPE, &type, COL_DATA, &p, -1);
    switch (type) {
    case TYPE_STATUS:
      gtk_text_buffer_set_text (view->priv->details, g_strdup_printf ("Status message '%s'", (char*)p), -1);
      break;
    case TYPE_ADDED:
      {
        ContactChange *change = p;
        gtk_text_buffer_set_text (view->priv->details, g_strdup_printf ("Contact added\n\n%s", e_vcard_to_string (E_VCARD (change->after), EVC_FORMAT_VCARD_30)), -1);
      }
      break;
    case TYPE_CHANGED:
      {
        ContactChange *change = p;
        gtk_text_buffer_set_text (view->priv->details,
                                  g_strdup_printf ("Contact changed\n\n%s\n\n%s",
                                                   e_vcard_to_string (E_VCARD (change->before), EVC_FORMAT_VCARD_30),
                                                   e_vcard_to_string (E_VCARD (change->after), EVC_FORMAT_VCARD_30)),
                                  -1);
      }
      break;
    case TYPE_REMOVED:
      {
        ContactChange *change = p;
        gtk_text_buffer_set_text (view->priv->details, g_strdup_printf ("Contact removed\n\n%s", e_vcard_to_string (E_VCARD (change->before), EVC_FORMAT_VCARD_30)), -1);
      }
      break;
    case TYPE_COMPLETE:
      gtk_text_buffer_set_text (view->priv->details, g_strdup_printf ("Sequence Complete (code %d)", (int)p), -1);
      break;
    default:
      gtk_text_buffer_set_text (view->priv->details, g_strdup_printf ("Unhandled event type %d", type), -1);
      break;
    }
  } else {
    gtk_text_buffer_set_text (view->priv->details, "", -1);
  }
}

static void
eds_event_view_dispose (GObject *object)
{
  if (G_OBJECT_CLASS (eds_event_view_parent_class)->dispose)
    G_OBJECT_CLASS (eds_event_view_parent_class)->dispose (object);
}

static void
eds_event_view_class_init (EdsEventViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (EdsEventViewPrivate));

  object_class->dispose = eds_event_view_dispose;
}

static void
eds_event_view_init (EdsEventView *view)
{
  GtkWidget *scrolled, *treeview, *textview;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  view->priv = EVENT_VIEW_PRIVATE (view);

  /* Create the event store */  
  view->priv->contacts = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
  view->priv->events = gtk_list_store_new (3, G_TYPE_LONG, G_TYPE_INT, G_TYPE_POINTER);

  /* Create the widgets for the contact list on the left */
  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled),
                                       GTK_SHADOW_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

  treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (view->priv->events));
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Name", renderer, NULL);
  gtk_tree_view_column_set_cell_data_func (column, renderer, event_data_func, NULL, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
  g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)),
                    "changed", G_CALLBACK (on_selection_changed), view);
  
  gtk_container_add (GTK_CONTAINER (scrolled), treeview);
  gtk_paned_add1 (GTK_PANED (view), scrolled);

  /* Create the widgets for the contact details on the right */
  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled),
                                       GTK_SHADOW_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  textview = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (textview), FALSE);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (textview), FALSE);
  view->priv->details = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
  gtk_container_add (GTK_CONTAINER (scrolled), textview);
  gtk_paned_add2 (GTK_PANED (view), scrolled);

  /* Finally set the width of the pane, and show everything */
  gtk_paned_set_position (GTK_PANED (view), 200);
  gtk_widget_show_all (GTK_WIDGET (view));
}

GtkWidget *
eds_event_view_new (void)
{
  return g_object_new (EDS_TYPE_EVENT_VIEW, NULL);
}

void
eds_event_view_set_book_view (EdsEventView *view, EBookView *bookview)
{
  g_return_if_fail (EDS_IS_EVENT_VIEW (view));
  g_return_if_fail (E_IS_BOOK_VIEW (bookview));

  if (view->priv->bookview) {
    g_object_unref (view->priv->bookview);
    view->priv->bookview = NULL;
  }

  view->priv->bookview = g_object_ref (bookview);
  g_signal_connect (view->priv->bookview, "status-message", G_CALLBACK (on_status_message), view);
  g_signal_connect (view->priv->bookview, "contacts-added", G_CALLBACK (on_contacts_added), view);
  g_signal_connect (view->priv->bookview, "contacts-changed", G_CALLBACK (on_contacts_changed), view);
  g_signal_connect (view->priv->bookview, "contacts-removed", G_CALLBACK (on_contacts_removed), view);
  g_signal_connect (view->priv->bookview, "sequence-complete", G_CALLBACK (on_sequence_complete), view);

  gtk_list_store_clear (view->priv->events);
}
