#include <gtk/gtk.h>
#include <libebook/e-book.h>
#include "eds-contact-view.h"
#include "eds-event-view.h"

static GtkWidget *contacts_view, *event_view, *query_view;

static void
on_query_button_clicked (GtkButton *button, GtkEntry *entry)
{
  const char *s;
  EBookQuery *query;

  g_return_if_fail (GTK_ENTRY (entry));

  s = gtk_entry_get_text (entry);
  
  if (s[0] == '\0') {
    query = e_book_query_any_field_contains ("");
  } else {
    query = e_book_query_from_string (s);
  }

  if (!query) {
    /* TODO: display error dialog */
    return;
  }

  eds_contact_view_show_query (EDS_CONTACT_VIEW (query_view), query);
  e_book_query_unref (query);
}

int
main (int argc, char **argv) {
  GError *error = NULL;
  GtkWidget *window;
  GtkWidget *tabs, *hbox, *vbox, *query_entry, *query_button;
  EBook *book;
  EBookView *view = NULL;
  EBookQuery *query;

  gtk_init (&argc, &argv);
  
  book = e_book_new_system_addressbook (&error);
  if (!book) {
    g_warning ("Cannot get book: %s", error->message);
    g_error_free (error);
    return 1;
  }
  
  if (!e_book_open (book, FALSE, &error)) {
    g_warning ("Cannot get book: %s", error->message);
    g_error_free (error);
    return 1;
  }

  query = e_book_query_any_field_contains ("");
  if (!e_book_get_book_view (book, query, NULL, 0, &view, &error)) {
    g_warning ("Cannot get book view: %s", error->message);
    g_error_free (error);
    return 1;
  }
  e_book_query_unref (query);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Contact Trace");
  gtk_window_set_default_size (GTK_WINDOW (window), 600, 400);
  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);
  gtk_widget_show (window);

  tabs = gtk_notebook_new ();
  gtk_container_add (GTK_CONTAINER (window), tabs);

  /* The Contacts tab */
  contacts_view = eds_contact_view_new ();
  eds_contact_view_set_book (EDS_CONTACT_VIEW (contacts_view), book);
  eds_contact_view_set_book_view (EDS_CONTACT_VIEW (contacts_view), view);
  gtk_notebook_append_page (GTK_NOTEBOOK (tabs),
                            contacts_view,
                            gtk_label_new_with_mnemonic ("_Contacts"));

  /* Events Tab */
  event_view = eds_event_view_new ();
  eds_event_view_set_book_view (EDS_EVENT_VIEW (event_view), view);
  gtk_notebook_append_page (GTK_NOTEBOOK (tabs),
                            event_view,
                            gtk_label_new_with_mnemonic ("_Events"));

  /* The Query tab */
  vbox = gtk_vbox_new (FALSE, 8);

  query_view = eds_contact_view_new ();
  eds_contact_view_set_book (EDS_CONTACT_VIEW (query_view), book);

  hbox = gtk_hbox_new (FALSE, 8);
  query_entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox), query_entry, TRUE, TRUE, 0);
  query_button = gtk_button_new_with_label ("Run");
  g_signal_connect (query_button, "clicked", G_CALLBACK (on_query_button_clicked), query_entry);
  gtk_box_pack_start (GTK_BOX (hbox), query_button, FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), query_view, TRUE, TRUE, 0);
  gtk_notebook_append_page (GTK_NOTEBOOK (tabs),
                            vbox,
                            gtk_label_new_with_mnemonic ("_Queries"));
  
  e_book_view_start (view);

  gtk_widget_show_all (window);
  gtk_main ();

  return 0;
}
