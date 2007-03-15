#include <libebook/e-book.h>
#include <dbus/dbus-glib.h>

static GMainLoop *loop = NULL;

static char *uri = NULL;
static char *query_str = NULL;
static int count = 1;
static gboolean silent = FALSE;
static gboolean verbose = FALSE;

static const GOptionEntry options[] =  {
  { "uri", 'u', 0, G_OPTION_ARG_STRING, &uri, "URI of book to open (default: system addressbook)." },
  { "query", 'q', 0, G_OPTION_ARG_STRING, &query_str, "Query to use (default: none)" },
  { "repetition", 'r', 0, G_OPTION_ARG_INT, &count, "Number of repetitions (default: 1)" },
  { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Verbose output" },
  { "silent", 's', 0, G_OPTION_ARG_NONE, &silent, "No output" },
  { NULL }
};

/* Convenience function to print an error and exit */
static void
die (const char *prefix, GError *error) 
{
  if (error) {
    if (error->code == DBUS_GERROR_REMOTE_EXCEPTION) {
      g_error ("%s: %s (%s)", prefix, error->message, dbus_g_error_get_name (error));
    } else {
      g_error ("%s: %s (%d)", prefix, error->message, error->code);
    }
    g_error_free (error);
  } else {
    g_error (prefix);
  }
  exit (1);
}

static void view_status_message (EBookView *book_view, const char*message, gpointer userdata)
{
  if (!silent)
    g_print ("Status: %s\n", message);
}

static void view_contacts_added (EBookView *book_view, GList *contacts, gpointer userdata)
{
  if (!silent) {
    while (contacts) {
      g_print ("Got contact %s\n",  (char*)e_contact_get_const (contacts->data, E_CONTACT_FILE_AS));
      contacts = g_list_next (contacts);
    }
  }
}

static void view_complete (EBookView *book_view, EBookViewStatus status, gpointer userdata)
{
  if (!silent)
    g_print ("Book view completed (status %d)\n", status);
  e_book_view_stop (book_view);
  g_object_unref (book_view);
  g_main_loop_quit (loop);
}

int main(int argc, char **argv) {
  GError *error = NULL;
  GOptionContext *context;
  EBook *book;
  EBookQuery *query;
  EBookView *view;
  
  g_type_init();

  context = g_option_context_new ("- EBookView stress test");
  g_option_context_add_main_entries (context, options, NULL);
  if (!g_option_context_parse (context, &argc, &argv, &error)) {
    g_error ("Cannot parse arguments: %s", error->message);
  }

  /* Ensure the user doesn't do something silly like --verbose --silent */
  if (silent)
    verbose = FALSE;

  loop = g_main_loop_new (NULL, FALSE);

  if (verbose) g_print ("Getting EBook...\n");

  if (uri) {
    book = e_book_new_from_uri (uri, &error);
    g_free (uri);
  } else {
    book = e_book_new_system_addressbook (&error);
  }
  
  if (!book)
    die ("Cannot get book", error);

  if (verbose) g_print ("Got EBook\nOpening book...\n");

  if (!e_book_open (book, TRUE, &error))
    die ("Cannot open book", error);

  if (verbose) g_print ("Opened book\n");

  if (query_str) {
    query = e_book_query_from_string (query_str);
    if (!query) {
      g_error ("Cannot parse query: '%s'", query_str);
    }
  } else {
    query = e_book_query_any_field_contains ("");
  }

  view = NULL;
  while (count--) {
    if (verbose) g_print ("Getting EBookView...\n");
    if (!e_book_get_book_view (book, query, NULL, 0, &view, &error))
      die ("Call to get_book_view failed", error);
    if (verbose) g_print ("Got EBookView\n");
    g_object_connect (view,
                      "signal::contacts-added", G_CALLBACK (view_contacts_added), NULL,
                      "signal::status-message", G_CALLBACK (view_status_message), NULL,
                      "signal::sequence-complete", G_CALLBACK (view_complete), NULL,
                      NULL);
    if (verbose) g_print ("Starting EBookView...\n");
    e_book_view_start (view);
    if (verbose) g_print ("Entering main loop...\n");
    g_main_loop_run (loop);
    /* At this point the view should be destroyed, so NULL the pointer */
    view = NULL;
  }
  g_object_unref (book);
  e_book_query_unref (query);
    
  if (verbose)
    g_print ("Closed EBook\n");

  g_main_loop_unref (loop);
  return 0;
}
