backenddir = ${libdir}/evolution-data-server-1.2/extensions/
backend_LTLIBRARIES = libebookbackenddummy.la

libebookbackenddummy_la_SOURCES =	\
	e-book-backend-dummy-factory.c	\
	e-book-backend-dummy.h		\
	e-book-backend-dummy.c

libebookbackenddummy_la_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"libebookbackend\" \
	$(EDATABOOK_CFLAGS) -Wall

libebookbackenddummy_la_LIBADD = \
	$(EDATABOOK_LIBS)

libebookbackenddummy_la_LDFLAGS =	\
	-module -avoid-version
