#include <string.h>
#include <libebook/e-book.h>
#include "util.h"

static gboolean
do_add (EVCard *vcard, int argc, char **argv)
{
  EVCardAttribute *attr;
  
  attr = e_vcard_attribute_new (NULL, argv[0]);
  e_vcard_attribute_add_value (attr, argv[1]);
  
  e_vcard_add_attribute (vcard, attr);

  return TRUE;
}

static gboolean
do_remove (EVCard *vcard, int argc, char **argv)
{
  EVCardAttribute *attr;

  attr = e_vcard_get_attribute (vcard, argv[0]);

  if (attr == NULL) {
    g_print ("No attribute %s found", argv[0]);
    return FALSE;
  }

  e_vcard_remove_attribute (vcard, attr);

  return TRUE;
}

static gboolean
do_clean (EVCard *vcard, int argc, char **argv)
{
  GList *attrs, *values;
  EVCardAttribute *attr;
  gboolean ret = FALSE;

  attrs = e_vcard_get_attributes (vcard);
  while (attrs) {
    attr = attrs->data;
    /* Move on now, before we manipulate the list */
    attrs = attrs->next;

    values = e_vcard_attribute_get_values (attr);
    /* If there are no values, or the value is the empty string, remove it */
    if (values == NULL ||
        (values->next == NULL && ((char*)(values->data))[0] == '\0')) {
      e_vcard_remove_attribute (vcard, attr);
      ret = TRUE;
    }
  }

  return ret;
}

typedef  gboolean (*Operation) (EVCard *vcard, int argc, char **argv);
typedef struct {
  const char *name;
  int arguments;
  Operation func;
} Command;

static const Command commands[] = {
  { "add" , 2, do_add },
  { "remove" , 1, do_remove },
  { "clean" , 0, do_clean },
};

int
main (int argc, char **argv)
{
  GError *error = NULL;
  EBook *book;
  EContact *contact = NULL;
  int i;
  Operation operation = NULL;

  g_type_init ();

  for (i = 0; i < G_N_ELEMENTS (commands); i++) {
    if (argc == commands[i].arguments + 3 &&
        strcmp (argv[2], commands[i].name) == 0) {
      operation = commands[i].func;
      break;
    }
  }

  if (!operation) {
    g_print ("Usage:\n"
             "$ eds-edit-vcard <UID> add <FIELD> <VALUE>\n"
             "$ eds-edit-vcard <UID> remove <FIELD>\n"
             "$ eds-edit-vcard <UID> clean\n"
             );
    return 1;
  }
  
  book = e_book_new_system_addressbook (&error);
  if (!book)
    die ("Cannot get book: %s", error);
  
  if (!e_book_open (book, TRUE, &error))
    die ("Cannot open book", error);

  if (!e_book_get_contact (book, argv[1], &contact, &error))
    die ("Cannot get contact", error);

  if (operation (E_VCARD (contact), argc - 3, argv + 3))
    if (!e_book_commit_contact (book, contact, &error))
      die ("Cannot commit contact", error);

  g_object_unref (book);
  
  return 0;
}
