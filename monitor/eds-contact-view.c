#include "eds-contact-view.h"
#include <gtk/gtk.h>
#include <libebook/e-book.h>

G_DEFINE_TYPE (EdsContactView, eds_contact_view, GTK_TYPE_HPANED);

#define CONTACT_VIEW_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EDS_TYPE_CONTACT_VIEW, EdsContactViewPrivate))

struct _EdsContactViewPrivate
{
  EBook *book;
  EBookView *bookview;
  GtkListStore *store;
  GtkTreeModel *sorter;
  GtkTextBuffer *details;
};

enum {
  COL_NAME, /* The contact's full name (cached) */
  COL_CONTACT, /* The EContact */
};

static void
eds_contact_view_dispose (GObject *object)
{
  EdsContactViewPrivate *priv = EDS_CONTACT_VIEW (object)->priv;

  if (priv->book) {
    g_object_unref (priv->book);
    priv->book = NULL;
  }

  if (priv->bookview) {
    g_object_unref (priv->bookview);
    priv->bookview = NULL;
  }

  if (priv->store) {
    g_object_unref (priv->store);
    priv->store = NULL;
  }

  if (priv->sorter) {
    g_object_unref (priv->sorter);
    priv->sorter = NULL;
  }

  if (G_OBJECT_CLASS (eds_contact_view_parent_class)->dispose)
    G_OBJECT_CLASS (eds_contact_view_parent_class)->dispose (object);
}

static void
eds_contact_view_class_init (EdsContactViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (EdsContactViewPrivate));

  object_class->dispose = eds_contact_view_dispose;
}

static void
on_selection_changed (GtkTreeSelection *selection, EdsContactView *view)
{
  GtkTreeModel *model;
  GtkTreeIter iter;

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
    EContact *contact;
    char *string;

    gtk_tree_model_get (model, &iter, COL_CONTACT, &contact, -1);
    string = e_vcard_to_string (E_VCARD (contact), EVC_FORMAT_VCARD_30);
    gtk_text_buffer_set_text (view->priv->details, string, -1);
    g_free (string);
  } else {
    gtk_text_buffer_set_text (view->priv->details, "", -1);
  }
}

static void
eds_contact_view_init (EdsContactView *view)
{
  GtkWidget *scrolled, *treeview, *textview;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  view->priv = CONTACT_VIEW_PRIVATE (view);

  /* Create the contact store and the sorting filter */
  view->priv->store = gtk_list_store_new (2, G_TYPE_STRING, E_TYPE_CONTACT);
  view->priv->sorter = gtk_tree_model_sort_new_with_model (GTK_TREE_MODEL (view->priv->store));
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (view->priv->sorter),
                                        COL_NAME, GTK_SORT_ASCENDING);

  /* Create the widgets for the contact list on the left */
  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled),
                                       GTK_SHADOW_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

  treeview = gtk_tree_view_new_with_model (view->priv->sorter);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Name", renderer,
                                                     "text", COL_NAME,
                                                     NULL);
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

GtkWidget*
eds_contact_view_new (void)
{
  return g_object_new (EDS_TYPE_CONTACT_VIEW, NULL);
}

void
eds_contact_view_set_book (EdsContactView *view, EBook *book)
{
  g_return_if_fail (EDS_IS_CONTACT_VIEW (view));
  g_return_if_fail (E_IS_BOOK (book));

  if (view->priv->book) {
    g_object_unref (view->priv->book);
    view->priv->book = NULL;
  }

  view->priv->book = g_object_ref (book);
}

static void
on_contacts_added (EBookView *bookview, GList *list, gpointer user_data)
{
  EdsContactView *view = user_data;

  g_return_if_fail (EDS_IS_CONTACT_VIEW (view));

  for (; list; list = g_list_next (list)) {
    EContact *contact = list->data;
    GtkTreeIter iter;

    gtk_list_store_append (view->priv->store, &iter);
    gtk_list_store_set (view->priv->store, &iter,
                        COL_NAME, e_contact_get_const (contact, E_CONTACT_FULL_NAME),
                        COL_CONTACT, contact,
                        -1);
  }}

void
eds_contact_view_show_query_sexp (EdsContactView *view, const char *query)
{
  EBookQuery *equery;

  g_return_if_fail (EDS_IS_CONTACT_VIEW (view));
  g_return_if_fail (query != NULL);

  equery = e_book_query_from_string (query);

  eds_contact_view_show_query (view, equery);

  e_book_query_unref (equery);
}

void
eds_contact_view_show_query (EdsContactView *view, EBookQuery *query)
{
  GError *error = NULL;
  EBookView *bookview = NULL;

  g_return_if_fail (EDS_IS_CONTACT_VIEW (view));
  g_return_if_fail (query != NULL);

  if (!view->priv->book) {
    g_warning ("The EBook has not been set");
    return;
  }

  /* TODO: do async */
  e_book_get_book_view (view->priv->book, query, NULL, 0, &bookview, &error);

  eds_contact_view_set_book_view (view, bookview);

  e_book_view_start (bookview);
  g_object_unref (bookview);
}

void
eds_contact_view_set_book_view (EdsContactView *view, EBookView *bookview)
{
  g_return_if_fail (EDS_IS_CONTACT_VIEW (view));
  g_return_if_fail (E_IS_BOOK_VIEW (bookview));

  if (view->priv->bookview) {
    g_object_unref (view->priv->bookview);
    view->priv->bookview = NULL;
  }

  view->priv->bookview = g_object_ref (bookview);
  g_signal_connect (view->priv->bookview, "contacts-added", G_CALLBACK (on_contacts_added), view);

  gtk_list_store_clear (view->priv->store);
}
