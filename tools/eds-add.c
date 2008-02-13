#include <libebook/e-book.h>
#include "util.h"

int
main (int argc, char **argv)
{
  GError *error = NULL;
  GOptionContext *context;
  EBook *book;
  char *uri = NULL, *vcard = NULL;
  EContact *contact = NULL;
  const GOptionEntry options[] =  {
    { "uri", 'u', 0, G_OPTION_ARG_STRING, &uri, "URI of book to open (default: system addressbook)." },
    { NULL }
  };

  g_type_init ();

  context = g_option_context_new ("<filename> - add vCard to address book");
  g_option_context_add_main_entries (context, options, NULL);
  if (!g_option_context_parse (context, &argc, &argv, &error)) {
    g_error ("Cannot parse arguments: %s", error->message);
  }

  if (argc != 2) {
    g_print ("Usage: eds-add <filename>\n");
    return 1;
  }

  if (!g_file_get_contents (argv[1], &vcard, NULL, &error))
    die ("Cannot open file", error);
  contact = e_contact_new_from_vcard (vcard);
  
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

  if (!e_book_add_contact (book, contact, &error))
    die ("Cannot add contact", error);

  g_print ("Added contact with UID %s\n",
           (char*)e_contact_get_const (contact, E_CONTACT_UID));
  
  g_object_unref (contact);
  g_object_unref (book);

  return 0;
}
