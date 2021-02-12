
/*

                        LICENCIA

1. Este programa puede ser ejecutado sin ninguna restricci�n
   por parte del usuario final del mismo.

2. La  Universitat Jaume I autoriza la copia y  distribuci�n
   del programa con cualquier fin y por cualquier medio  con
   la  �nica limitaci�n de que, de forma  apropiada, se haga
   constar  en  cada  una  de las copias la  autor�a de esta  
   Universidad  y  una reproducci�n  exacta de las presentes 
   condiciones   y   de   la   declaraci�n  de  exenci�n  de 
   responsabilidad.

3. La  Universitat  Jaume  I autoriza  la  modificaci�n  del
   software  y  su  redistribuci�n  siempre que en el cambio
   del  c�digo  conste la autor�a de la Universidad respecto  
   al  software  original  y  la  url de descarga del c�digo
   fuente  original. Adem�s, su denominaci�n no debe inducir 
   a  error  o  confusi�n con el original. Cualquier persona
   o  entidad  que  modifique  y  redistribuya  el  software 
   modificado deber�  informar de tal circunstancia mediante
   el  env�o  de  un  mensaje  de  correo  electr�nico  a la 
   direcci�n  clauer@uji.es  y  remitir una copia del c�digo 
   fuente modificado.

4. El  c�digo  fuente  de todos los programas amparados bajo 
   esta licencia  est�  disponible para su descarga gratuita
   desde la p�gina web http//:clauer.uji.es.

5. El hecho en s� del uso, copia o distribuci�n del presente 
   programa implica la aceptaci�n de estas condiciones.

6. La  copia y distribuci�n del programa supone la extensi�n 
   de las presentes condiciones al destinatario.
   El  distribuidor no puede imponer condiciones adicionales
   que limiten las aqu� establecidas.

       DECLARACI�N DE EXENCI�N DE RESPONSABILIDAD

Este  programa  se  distribuye  gratuitamente. La Universitat 
Jaume  I  no  ofrece  ning�n  tipo de garant�a sobre el mismo
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

// Funci�n de windows, equivalente para Linux en 
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

    /* Descripci�n: Importa certificado, llave privada y certificados ra�z desde un 
                    fichero pPKCS#12 al Clauer. 
    
       Par�metros: fileName: Nombre del fichero pkcs#12 a importar.
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
