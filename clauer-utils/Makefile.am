bin_PROGRAMS= clexport  climport  clls  clpasswd  clview  clwblock  clmakefs cldel 
sbin_SCRIPTS= fclauer
EXTRA_DIST= clauer-utils_es.mo clauer-utils_ca.mo  

clexport_SOURCES= clexport.c misc.c misc.h nls.h nls.c
climport_SOURCES= climport.c misc.c nls.h nls.c
clls_SOURCES= clls.c misc.c nls.h nls.c
clpasswd_SOURCES= clpasswd.c misc.c nls.h nls.c
clview_SOURCES= clview.c misc.c nls.h nls.c 
clwblock_SOURCES= clwblock.c misc.c 
clmakefs_SOURCES= clmakefs.c misc.c nls.c nls.h
cldel_SOURCES= cldel.c misc.c nls.c nls.h

INCLUDES= -O4 -DLINUX -I../
LDADD= ../CRYPTOWrapper/libCRYPTOWrap.la ../LIBRT/libRT.la ../clio/libclio.la -lcrypto
climport_LDADD= ../CRYPTOWrapper/libCRYPTOWrap.la ../LIBRT/libRT.la ../LIBIMPORT/libIMPORT.la -lcrypto

install-exec-hook:
	if [  -d /usr/share/locale/es/LC_MESSAGES/  ]; then cp clauer-utils_es.mo /usr/share/locale/es/LC_MESSAGES/clauer-utils.mo; fi
	if [  -d /usr/share/locale/ca/LC_MESSAGES/  ]; then cp clauer-utils_ca.mo /usr/share/locale/ca/LC_MESSAGES/clauer-utils.mo; fi             
