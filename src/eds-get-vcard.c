#include <libebook/e-book.h>
#include "util.h"

int
main (int argc, char **argv)
{
  GError *error = NULL;
  EBook *book;
  EContact *contact = NULL;

  g_type_init ();

  if (argc != 2) {
    g_print ("Usage: eds-get-vcard <UID>");
    return 1;
  }

  book = e_book_new_system_addressbook (&error);
  if (!book)
    die ("Cannot get book: %s", error);
  
  if (!e_book_open (book, TRUE, &error))
    die ("Cannot open book", error);

  if (!e_book_get_contact (book, argv[1], &contact, &error))
    die ("Cannot get contact", error);

  g_print (e_vcard_to_string (E_VCARD (contact), EVC_FORMAT_VCARD_30));
  g_print ("\n");

  g_object_unref (book);

  return 0;
}
