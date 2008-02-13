#include <libebook/e-book.h>
#include "util.h"

int
main (int argc, char **argv)
{
  GError *error = NULL;
  GOptionContext *context;
  EBook *book;
  char *uri = NULL;
  
  const GOptionEntry options[] =  {
    { "uri", 'u', 0, G_OPTION_ARG_STRING, &uri, "URI of book to open (default: system addressbook)." },
    { NULL }
  };

  g_type_init ();

  context = g_option_context_new ("<UID> - remove a contact from the address book");
  g_option_context_add_main_entries (context, options, NULL);
  if (!g_option_context_parse (context, &argc, &argv, &error)) {
    g_error ("Cannot parse arguments: %s", error->message);
  }

  if (argc != 2) {
    g_print ("Usage: eds-remove <UID>\n");
    return 1;
  }

  if (uri) {
    book = e_book_new_from_uri (uri, &error);
    g_free (uri);
  } else {
    book = e_book_new_system_addressbook (&error);
  }
  if (!book)
    die ("Cannot get book", error);
  
  if (!e_book_open (book, TRUE, &error))
    die ("Cannot open book", error);

  if (!e_book_remove_contact (book, argv[1], &error))
    die ("Cannot remove contact", error);

  g_object_unref (book);

  return 0;
}
