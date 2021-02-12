
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

#ifndef __LIBRT_H__
#define __LIBRT_H__

#if defined(WIN32)

#include <windows.h>
#include <winsock.h>

#endif

#include <stdio.h>
#include <stdlib.h>

#include "UtilBloques.h"
#include "LIBRTMutex.h"
#ifdef WIN32
#include "libMSG.h"
#else
#include "libMSG.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif
    
#define IP		"127.0.0.1"
    /*#define IP              "150.128.49.221"*/
#define PORTNUMBER	969
    
    /*
     * Max Sizes 
     */
    
#define MAX_DEVICES  128 // By the time no more than 128 devices
#define MAX_PATH_LEN 512
#define MAX_PASS_LEN 127
#define HW_ID_LEN     16  // It will be an md5?
    
    /*
     * Tipos de datos
     */
    
typedef unsigned char TIPO_CERT;
typedef unsigned long HANDLE_OBJETO;
    
struct USBCERTS_HANDLE;

/* -1 Error
 * 0 Aceptar
 * 1 Cancelar
 */

typedef int (* LIBRT_PIN_cb) (struct USBCERTS_HANDLE *hDispositivo,	
                             #ifdef NO_CRYF_SUPPORT
				  unsigned char *dispositivos,	
                             #else
				  unsigned char **dispositivos,	
                             #endif
				  int nDispositivos,	
				  void *appData, 
				  char pin[100], 
				  int *dispSel);

typedef struct USBCERTS_HANDLE {
  int nDevice;                 /* El n�mero de dispositivo a utilizar */
  char path[MAX_PATH_LEN+1];   /* Path del dispositivo */ 
  SOCKET sock;		       /* El socket con el runtime */
  LIBRT_MUTEX mutex;	       /* M�tex que garantiza el acceso exclusivo al socket */
  unsigned char idDispositivo[20];      /* El identificador del dispositivo */
  int enumDispositivos;       /* Indica si se est� enumerando un dispositivo de un tipo determinado */
  TIPO_BLOQUE enumTipo;	       /* Indica el tipo de bloque que se est� enumrando */
  LIBRT_PIN_cb pin_cb;	       /* Funci�n para pedir el pin en las reconexiones debidas a timeout */
  void *appData;	       /* Datos que puede pasar la aplicaci�n */
  int auth;		       /* Indica si el handle se adquiri� en modo autenticado o no */
} USBCERTS_HANDLE;


typedef struct _clauer_partition_info{
 //All in bytes 
 long long size;
 int type; 
}CLAUER_PARTITION_INFO;

typedef struct _clauer_layout_info{
 int npartitions;
 CLAUER_PARTITION_INFO cpi[4];
}CLAUER_LAYOUT_INFO;

/*
 * Prototipos de funciones
 */

int LIBRT_Ini (void);
int LIBRT_Fin (void);

int LIBRT_HayRuntime (void);

int LIBRT_Set_PIN_Callback (USBCERTS_HANDLE *handle, LIBRT_PIN_cb pin_cb);
int LIBRT_Set_AppData      (USBCERTS_HANDLE *handle, void *app_data);

int LIBRT_ListarDispositivos        ( int *nDispositivos, unsigned char **dispositivos);

int _LIBRT_IniciarDispositivo ( unsigned char * device, char * pwd, USBCERTS_HANDLE *handle );
int LIBRT_IniciarDispositivo ( unsigned char * device, char * pwd, USBCERTS_HANDLE *handle );


int LIBRT_IniciarDispositivoVentana ( USBCERTS_HANDLE *handle);
int LIBRT_FinalizarDispositivo      ( USBCERTS_HANDLE *handle );

int LIBRT_LeerZonaReservada         ( USBCERTS_HANDLE *handle, unsigned char *buffer, unsigned long *bytesBuffer);
int LIBRT_EscribirZonaReservada     ( USBCERTS_HANDLE *handle, unsigned char *buffer, unsigned long bytesBuffer);

int LIBRT_CambiarPassword           ( USBCERTS_HANDLE *handle, char * nuevaPassword);

int LIBRT_LeerBloqueIdentificativo  ( USBCERTS_HANDLE *handle, unsigned char *bloque);

int LIBRT_LeerBloqueCrypto          ( USBCERTS_HANDLE *handle, int numBloque, unsigned char *bloque);
int LIBRT_EscribirBloqueCrypto      ( USBCERTS_HANDLE *handle, unsigned long numBloque, unsigned char *bloque);
int LIBRT_InsertarBloqueCrypto      ( USBCERTS_HANDLE *handle, unsigned char *buffer, long *numBloque);
int LIBRT_BorrarBloqueCrypto        ( USBCERTS_HANDLE *handle, unsigned long numBloque);

int LIBRT_LeerTipoBloqueCrypto      ( USBCERTS_HANDLE *handle, TIPO_BLOQUE tipo, int primero, unsigned char *bloque, long *numBloque);
int LIBRT_LeerTodosBloquesTipo      ( USBCERTS_HANDLE *handle, TIPO_BLOQUE tipo, long *handleBloques, unsigned char *bloques, long *numBloques);

int LIBRT_LeerTodosBloquesOcupados  ( USBCERTS_HANDLE *handle, long *handleBloques, unsigned char *bloques, unsigned long *numBloques);
int LIBRT_ObtenerHardwareId     ( USBCERTS_HANDLE *handle, unsigned char * hwIdDispositivo, unsigned char * hwIdSistema );
int LIBRT_ObtenerVersion     ( char ** version );
int LIBRT_RegenerarCache        ( );


/* Admin funcs */

int LIBRT_ListarUSBs  ( int *nDispositivos, unsigned char **dispositivos);
int LIBRT_CrearClauer ( char * dispositivo, char percent );
int LIBRT_EliminarClauer( char * dispositivo );
int LIBRT_FormatearClauerCrypto( char * dispositivo, char * pwd ); 
int LIBRT_FormatearClauerDatos( char * dispositivo );
int LIBRT_ObtenerLayout( char * dispositivo, CLAUER_LAYOUT_INFO * cli);

/**/



/* C�digos de error
 */

#define ERR_LIBRT_NO                              0
#define ERR_LIBRT_SI                              1
#define ERR_LIBRT_DISPOSITIVO_NO_UNICO            2
#define ERR_LIBRT_NO_USBCERTS                     3
#define ERR_LIBRT_NO_CERT_DISPONIBLE              4
#define ERR_LIBRT_FORMATO_INCORRECTO              5
#define ERR_LIBRT_NO_LLAVE_PRIVADA                6
#define ERR_LIBRT_SIN_BLOQUES_BLANCOS             7
#define ERR_LIBRT_PASSWORD_INCORRECTA             8
#define ERR_LIBRT_PARAMETRO_INCORRECTO			  9
#define ERR_LIBRT_NO_ADMIN                       10

#ifdef __cplusplus
}
#endif

#endif

