AC_PREREQ(2.53)
AC_INIT(eds-tools, 0.1, http://www.openedhand.com/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(dummy-ebook/e-book-backend-dummy.c)
AM_CONFIG_HEADER(config.h)

AM_DISABLE_STATIC
AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

dnl TODO: enable this again when it works in Poky
dnl PKG_CHECK_EXISTS([evolution-data-server-1.2],
dnl                 [extensiondir=$($PKG_CONFIG --variable=extensiondir evolution-data-server-1.2 2>/dev/null)],
dnl                 [AC_MSG_ERROR([Package requirements were not met: evolution-data-server-1.2.pc not found])])
dnl AC_SUBST(EXTENSIONDIR, $extensiondir)

PKG_CHECK_MODULES(DBUS, dbus-glib-1)
PKG_CHECK_MODULES(EBOOK, libebook-1.2)
PKG_CHECK_MODULES(EDATABOOK, libedata-book-1.2)
PKG_CHECK_MODULES(EBACKEND, libebackend-1.2)
PKG_CHECK_MODULES(GTK, gtk+-2.0)

AC_OUTPUT([
Makefile
dummy-ebook/Makefile
monitor/Makefile
tools/Makefile
])
