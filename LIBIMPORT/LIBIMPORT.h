
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

#ifndef __LIBIMPORT_H__
#define __LIBIMPORT_H__


#define PRIVKEY_DEFAULT_CIPHER   0
#define PRIVKEY_PASSWORD_CIPHER  1
#define PRIVKEY_DOUBLE_CIPHER    2 
#define PRIVKEY_NO_CIPHER        3 

#define LIBIMPORT_MAX_PASSPHRASE 127

#if defined(WIN32)

#include <windows.h>

#endif

#if defined(LINUX)

#define BOOL unsigned char
#define TRUE  1
#define FALSE 0 
#define BYTE unsigned char
#define LPTSTR char *
#define DWORD unsigned int

// Función de windows, equivalente para Linux en 
// CryptoWrapper
//
#define SecureZeroMemory CRYPTO_SecureZeroMemory

// Defines de windows para compatibilidad en 
// Linux.

// Obtenido de windows.h 
#define MAX_PATH 260

// Obtenidos de wincrypt.h
#define ALG_CLASS_KEY_EXCHANGE  (5 << 13)
#define ALG_CLASS_SIGNATURE     (1 << 13)
#define ALG_TYPE_RSA            (2 << 9)
#define ALG_SID_RSA_ANY          0

#define CALG_RSA_KEYX  (ALG_CLASS_KEY_EXCHANGE|ALG_TYPE_RSA|ALG_SID_RSA_ANY) // 0x0000a400
#define CALG_RSA_SIGN  (ALG_CLASS_SIGNATURE|ALG_TYPE_RSA|ALG_SID_RSA_ANY)    // 0x00002400 

#endif


#include <LIBRT/libRT.h>

#ifdef __cplusplus
extern "C" {
#endif

    /* Descripción: Importa certificado, llave privada y certificados raíz desde un 
                    fichero pPKCS#12 al Clauer. 
    
       Parámetros: fileName: Nombre del fichero pkcs#12 a importar.
                   pwd:      Password del fichero pkcs12.
		   hClauer:  Handle del Clauer.
		   mode:     Modo en el que queremos que se cifren las claves:
		                    PRIVKEY_DEFAULT_CIPHER.
				    PRIVKEY_DOUBLE_CIPHER. 
				    PRIVKEY_PASSWORD_CIPHER.
				    PRIVKEY_NO_CIPHER.
		   
    */
    
    BOOL LIBIMPORT_ImportarPKCS12 (const char *fileName, const char *pwd, USBCERTS_HANDLE *hClauer, int mode, char *extra_pass );

    BOOL LIBIMPORT_ImportarPKCS12_internal (const char *fileName, const char *pwd, USBCERTS_HANDLE *hClauer, int mode, char *extra_pwd, BOOL importarCAs);
  
    BOOL LIBIMPORT_ImportarPKCS12deBuffer (BYTE * pkcs12, DWORD tam, const char *pwd, USBCERTS_HANDLE *hClauer, int mode, char *extra_pwd, BOOL importarCAs);


    BOOL LIBIMPORT_ExportarPKCS12 (long blockNum, const char *pwd, USBCERTS_HANDLE *hClauer, const char *fileName); 
    BOOL LIBIMPORT_ExportarPKCS12enBuffer (long blockNum, const char *pwd, USBCERTS_HANDLE *hClauer, BYTE ** destBuf, unsigned long * destTam);

  

#ifdef __cplusplus
}
#endif

#endif
