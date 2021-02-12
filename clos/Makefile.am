sbin_PROGRAMS= clos
bin_SCRIPTS= makeclos unmakeclos

clos_SOURCES= format.c common.c auth.c block.c session.c smem.c stub.c transport.c func.c log.c main.c format.h session.h smem.h stub.h transport.h err.h func.h log.h auth.h block.h common.h

INCLUDES= -O4 -DDEBUG_CRYF -DLINUX -I../ -I../clio/
LDADD= ../CRYPTOWrapper/libCRYPTOWrap.la ../clio/libclio.la

install-exec-hook:
		    ./makeclos $(sbindir)
		     /etc/init.d/clos start


uninstall-local:
		    ./unmakeclos
		     killall clos


