#include <dbus/dbus-glib.h>

void
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
  g_assert_not_reached ();
}
