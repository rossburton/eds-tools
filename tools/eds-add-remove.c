#include <libebook/e-book.h>
#include "util.h"

static const char* vcard = 
	"BEGIN:VCARD\n"
	"VERSION:3.0\n"
	"FN:Test Contact\n"
	"N:Contact;Test\n"
	"EMAIL;TYPE=WORK:test@example.com\n"
	"EMAIL;TYPE=HOME:test2@example.com\n"
	"X-JABBER:test@jabber.example.com\n"
	"END:VCARD\n";

int
main (int argc, char **argv)
{
  GError *error = NULL;
  int count;
  EBook *book;

  g_type_init ();

  if (argc == 1) {
    count = 1;
  } else if (argc == 2) {
    count = atoi (argv[1]);
  } else {
    g_print ("Usage: eds-add-remove <iterations>");
    return 1;
  }

  book = e_book_new_system_addressbook (&error);
  if (!book)
    die ("Cannot get book: %s", error);
  
  if (!e_book_open (book, FALSE, &error))
    die ("Cannot open book", error);

  while (count--) {
    EContact *contact;

    contact = e_contact_new_from_vcard (vcard);
    
    if (!e_book_add_contact (book, contact, &error))
      die ("Cannot add contact", error);

    if (!e_book_remove_contact (book, e_contact_get_const (contact, E_CONTACT_UID), &error))
      die ("Cannot remove contact", error);

    g_object_unref (contact);

    g_print (".");
  }

  g_print ("\nComplete\n");

  g_object_unref (book);

  return 0;
}
