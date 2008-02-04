#include <libebook/e-book.h>
#include "util.h"

static char *query_str = NULL;
static int count = 1;
static gboolean silent = FALSE;
static gboolean verbose = FALSE;

static const GOptionEntry options[] =  {
  { "query", 'q', 0, G_OPTION_ARG_STRING, &query_str, "Query to use (default: none)" },
  { "repetition", 'r', 0, G_OPTION_ARG_INT, &count, "Number of repetitions (default: 1)" },
  { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Verbose output" },
  { "silent", 's', 0, G_OPTION_ARG_NONE, &silent, "No output" },
  { NULL }
};

int
main(int argc, char **argv) {
  GError *error = NULL;
  GOptionContext *context;
  EBook *book;
  EBookQuery *query;
  GList *list;
  
  g_type_init ();

  context = g_option_context_new ("- EBookView stress test");
  g_option_context_add_main_entries (context, options, NULL);
  if (!g_option_context_parse (context, &argc, &argv, &error)) {
    g_error ("Cannot parse arguments: %s", error->message);
  }

  /* Ensure the user doesn't do something silly like --verbose --silent */
  if (silent)
    verbose = FALSE;

  if (verbose) g_print ("Getting EBook...\n");

  book = e_book_new_system_addressbook (&error);
  
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

  while (count--) {
    if (verbose) g_print ("Getting Contacts...\n");
    if (!e_book_get_contacts (book, query, &list, &error))
      die ("Call to get_contacts", error);
    while (list) {
      EContact *contact = list->data;
      if (verbose) {
        char *s;
        s = e_vcard_to_string (E_VCARD (contact), EVC_FORMAT_VCARD_30);
        g_print ("Got contact:\n%s\n\n", s);
        g_free (s);
      } else if (!silent) {
        g_print ("Got contact %s\n",
                 (char*)e_contact_get_const (contact, E_CONTACT_FILE_AS));
      }
      g_object_unref (contact);
      list = g_list_delete_link (list, list);
    }
  }
  g_object_unref (book);
  e_book_query_unref (query);
    
  if (verbose)
    g_print ("Closed EBook\n");

  return 0;
}
