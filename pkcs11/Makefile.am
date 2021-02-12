#bin_SCRIPTS = firefox-install-pkcs11.sh clauerPK11inst.xpi
lib_LTLIBRARIES = libclauerpkcs11.la
libclauerpkcs11_la_SOURCES = certif.cpp certx509.cpp clauer.cpp clave.cpp clvprvda.cpp clvrsa.cpp common.cpp lobjeto.cpp log.cpp mechanism.cpp objeto.cpp pkcs11.cpp  slot.cpp threads.cpp certif.h  certx509.h  clauer.h  clave.h  clvprvda.h  clvrsa.h  common.h  lobjeto.h  log.h  mechanism.h  objeto.h  pkcs11f.h  pkcs11.h  pkcs11t.h  slot.h threads.h

INCLUDES = -O4 -DLINUX -D_UNIX  -I../ -I../CRYTPOWrapper -DLABEL_PURPOSE   # -DDEBUG
libclauerpkcs11_la_LIBADD =  -lssl -lcrypto ../CRYPTOWrapper/libCRYPTOWrap.la ../LIBRT/libRT.la

install-exec-hook:
	if [ -L /usr/lib/libclauerpkcs11.so ];then unlink /usr/lib/libclauerpkcs11.so; fi 
	if [ ! -L /usr/lib/libclauerpkcs11.so ];then ln -s  $(libdir)\/libclauerpkcs11.so /usr/lib/libclauerpkcs11.so; fi 

uninstall-hook:
	if [ -L /usr/lib/libclauerpkcs11.so ];then unlink /usr/lib/libclauerpkcs11.so; fi 

