
/*

                        LICENCIA

1. Este programa puede ser ejecutado sin ninguna restricción
   por parte del usuario final del mismo.

2. La  Universitat Jaume I autoriza la copia y  distribución
   del programa con cualquier fin y por cualquier medio  con
   la  única limitación de que, de forma  apropiada, se haga
   constar  en  cada  una  de las copias la  autoría de esta  
   Universidad  y  una reproducción  exacta de las presentes 
   condiciones   y   de   la   declaración  de  exención  de 
   responsabilidad.

3. La  Universitat  Jaume  I autoriza  la  modificación  del
   software  y  su  redistribución  siempre que en el cambio
   del  código  conste la autoría de la Universidad respecto  
   al  software  original  y  la  url de descarga del código
   fuente  original. Además, su denominación no debe inducir 
   a  error  o  confusión con el original. Cualquier persona
   o  entidad  que  modifique  y  redistribuya  el  software 
   modificado deberá  informar de tal circunstancia mediante
   el  envío  de  un  mensaje  de  correo  electrónico  a la 
   dirección  clauer@uji.es  y  remitir una copia del código 
   fuente modificado.

4. El  código  fuente  de todos los programas amparados bajo 
   esta licencia  está  disponible para su descarga gratuita
   desde la página web http//:clauer.uji.es.

5. El hecho en sí del uso, copia o distribución del presente 
   programa implica la aceptación de estas condiciones.

6. La  copia y distribución del programa supone la extensión 
   de las presentes condiciones al destinatario.
   El  distribuidor no puede imponer condiciones adicionales
   que limiten las aquí establecidas.

       DECLARACIÓN DE EXENCIÓN DE RESPONSABILIDAD

Este  programa  se  distribuye  gratuitamente. La Universitat 
Jaume  I  no  ofrece  ningún  tipo de garantía sobre el mismo
ni acepta ninguna responsabilidad por su uso o  imposibilidad
de uso.

*/

#ifndef __COMMON_H__
#define __COMMON_H__

#include <string.h>
#include"pkcs11.h"
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#endif

#ifdef LINUX
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#ifndef DWORD
#define DWORD unsigned int
#endif
#endif 
 
#define BUFF_SIZE 1024 

#define DEFAULT_FILE "/etc/clos.conf"
#define BUFF_SIZE 1024

// Language 
enum {ES, ES_IDCAT, ES_ACCV, ES_COITAV,	 // Language
	       CA, CA_IDCAT, CA_ACCV, CA_COITAV,	
	       EN, EN_IDCAT, EN_ACCV, EN_COITAV};

// Clauer Token specific info.
// LANGUAGE 
// UJIAN VERSION 
#define LABEL_CA "Clauer criptografic"
#define MANUFACTURER_ID_CA "Universitat Jaume I"
#define MODEL_CA "Clauer UJI"

#define LABEL_ES "Clauer criptografico"
#define MANUFACTURER_ID_ES "Universidad Jaume I"
#define MODEL_ES "Clauer UJI"

#define LABEL_EN "Cryptographic Clauer"
#define MANUFACTURER_ID_EN "Jaume I University"
#define MODEL_EN "UJI Clauer"

// IDCAT VERSION
#define LABEL_IDCAT_CA "Clauer idCAT"
#define MANUFACTURER_ID_IDCAT_CA "UJI per a CATCert"
#define MODEL_IDCAT_CA "Clauer idCAT"

#define LABEL_IDCAT_ES "Clauer criptografico"
#define MANUFACTURER_ID_IDCAT_ES "UJI para CATCert"
#define MODEL_IDCAT_ES "Clauer idCAT"

#define LABEL_IDCAT_EN "idCAT Clauer"
#define MANUFACTURER_ID_IDCAT_EN "UJI for CATCert"
#define MODEL_IDCAT_EN "idCAT Clauer"
///// 


// ACCV VERSION
#define LABEL_COITAVC_CA "Clauer Criptografic"
#define MANUFACTURER_ID_COITAVC_CA "UJI per a COITAVC"
#define MODEL_COITAVC_CA "Clauer Criptografic"

#define LABEL_COITAVC_ES "Clauer Criptografico"
#define MANUFACTURER_ID_COITAVC_ES "UJI para COITAVC"
#define MODEL_COITAVC_ES "Clauer Criptografico"

#define LABEL_COITAVC_EN "Criptographic Clauer"
#define MANUFACTURER_ID_COITAVC_EN "UJI for COITAVC"
#define MODEL_COITAVC_EN "Criptographic Clauer"
///// 


// COITAVC VERSION 
#define LABEL_ACCV_CA "ClauACCV"
#define MANUFACTURER_ID_ACCV_CA "UJI per a ACCV"
#define MODEL_ACCV_CA "ClauACCV"

#define LABEL_ACCV_ES "ClauACCV"
#define MANUFACTURER_ID_ACCV_ES "UJI para ACCV"
#define MODEL_ACCV_ES "ClauACCV"

#define LABEL_ACCV_EN "ClauACCV"
#define MANUFACTURER_ID_ACCV_EN "UJI for ACCV"
#define MODEL_ACCV_EN "ClauACCV"
/////

#define SERIAL_NUMBER "123456789"
#define TOKEN_FLAGS /*(CKF_WRITE_PROTECTED |*/( CKF_LOGIN_REQUIRED | CKF_USER_PIN_INITIALIZED)
#define MAX_SESSIONS 1
#define MAX_PIN_LEN 127
#define MIN_PIN_LEN 8
#define MEM_PUBLICA  65535
#define MEM_PRIVADA  65535
#define UTC_TIME  "2006022312000000" // Maybe it is interesting get it from the system.


// Module specific info.
#define CRIPTOKI_VERSION_MAJOR 2
#define CRIPTOKI_VERSION_MINOR 0
#define FLAGS 0

#define LIBRARY_VERSION_MAJOR 1
#define LIBRARY_VERSION_MINOR 0

//LANGUAGE 
//UJIAN VERSION
#define LIBRARY_DESCRIPTION_ES "Modulo criptografico PKCS11"
#define LIBRARY_DESCRIPTION_CA "Modul criptografic PKCS11"
#define LIBRARY_DESCRIPTION_EN "Cryptographic Module PKCS11"

//IDCAT Version
#define LIBRARY_DESCRIPTION_IDCAT_ES "Modulo criptografico PKCS11"
#define LIBRARY_DESCRIPTION_IDCAT_CA "Modul criptografic PKCS11"
#define LIBRARY_DESCRIPTION_IDCAT_EN "Cryptographic Module PKCS11"

//ACCV Version
#define LIBRARY_DESCRIPTION_ACCV_ES "Modulo criptografico PKCS11"
#define LIBRARY_DESCRIPTION_ACCV_CA "Modul criptografic PKCS11"
#define LIBRARY_DESCRIPTION_ACCV_EN "Cryptographic Module PKCS11"

//COITAVC Version
#define LIBRARY_DESCRIPTION_COITAVC_ES "Modulo criptografico PKCS11"
#define LIBRARY_DESCRIPTION_COITAVC_CA "Modul criptografic PKCS11"
#define LIBRARY_DESCRIPTION_COITAVC_EN "Cryptographic Module PKCS11"

// Slot specific info.
//LANGUAGE
//UJIAN VERSION
#define SLOT_DESCRIPTION_ES "RSA con claves privada mediante Clauer"
#define SLOT_DESCRIPTION_CA "RSA amb claus privades mitjancant el Clauer"
#define SLOT_DESCRIPTION_EN "RSA with private keys on the Clauer"

//IDCAT VERSION 
#define SLOT_DESCRIPTION_IDCAT_ES "RSA con claves privada mediante Clauer"
#define SLOT_DESCRIPTION_IDCAT_CA "RSA amb claus privades mitjancant el Clauer"
#define SLOT_DESCRIPTION_IDCAT_EN "RSA with private keys on the Clauer"

//ACCV VERSION 
#define SLOT_DESCRIPTION_ACCV_ES "RSA con claves privada mediante ClauACCV"
#define SLOT_DESCRIPTION_ACCV_CA "RSA amb claus privades mitjancant la ClauACCV"
#define SLOT_DESCRIPTION_ACCV_EN "RSA with private keys on the ClauACCV"


//COITAVC VERSION 
#define SLOT_DESCRIPTION_COITAVC_ES "RSA con claves privada mediante Clauer Criptografic"
#define SLOT_DESCRIPTION_COITAVC_CA "RSA amb claus privades mitjancant el Clauer Criptografic"
#define SLOT_DESCRIPTION_COITAVC_EN "RSA with private keys on the Criptographic Clauer"

#define SLOT_FLAGS  CKF_REMOVABLE_DEVICE 
#define HARDWARE_VERSION_MAJOR 1
#define HARDWARE_VERSION_MINOR 0
#define FIRMWARE_VERSION_MAJOR 1
#define FIRMWARE_VERSION_MINOR 0


// Other generic defines
#define MAX_PRIVATE_KEY 10240
#define MAX_DEVICES 128
#define KEY_ID_LEN 20
#define CERT_ID_LEN 20
#define CLAUER_ID_LEN 20

// how long to wait to look for new clauers  
#define CHECK_FOR_CLAUER_INTERVAL  6

// Auxiliar Functions 
char * padding(char *destino, const char *origen, unsigned int longitudConRelleno);
CK_ATTRIBUTE_PTR buscarEnPlantilla(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount, CK_ATTRIBUTE_TYPE type);
void destruirPlantilla(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount);
void SMEM_Destroy ( void *buf, unsigned long size );

void parse_config_file ( const char * file, char ** r_version, int * r_type );
DWORD IDIOMA_Obtener (void);

//Windows specific 
#ifdef WIN32
#define snprintf _snprintf 
#endif

#endif
