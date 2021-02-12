
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

/* Clauer utils miscellaneous functions */

#ifndef __CLAUERUTILS_MISC_H__
#define __CLAUERUTILS_MISC_H__



#include <stdio.h>
#include <stdlib.h>


#include <LIBRT/libRT.h>

#define MAX_PASSPHRASE   128
#define MAX_CLAUER_OWNER 256

int  CLUTILS_Connect       ( USBCERTS_HANDLE *hClauer, int auth );
int  CLUTILS_ConnectEx     ( USBCERTS_HANDLE *hClauer, char *passphrase );
int  CLUTILS_ConnectEx2    ( USBCERTS_HANDLE *hClauer, char *passphrase, char *device );

int  CLUTILS_AskPassphrase ( char *prompt, char passphrase[MAX_PASSPHRASE] );

int  CLUTILS_PrintClauers   ( void );
void CLUTILS_Destroy        ( void *buf, unsigned long size );
int  CLUTILS_Clauer_Is_File ( char deviceName[MAX_PATH_LEN+1] );

void CLUTILS_PrintCert      ( unsigned long bNumber, unsigned char *bloque, unsigned char type );
void CLUTILS_PrintLlave     ( long bNumber, unsigned char block[TAM_BLOQUE] );
void CLUTILS_PrintBlob      ( USBCERTS_HANDLE *h, long bNumber, unsigned char block[TAM_BLOQUE]);
void CLUTILS_PrintContainer ( long bNumber, unsigned char block[TAM_BLOQUE] );
void CLUTILS_PrintWallet    ( long bNumber, unsigned char block[TAM_BLOQUE] );

int CLUTILS_ListarDispositivos ( int * nDev, unsigned char ** dev );

int CLUTILS_Get_Clauer_Owner ( char *device, char owner[MAX_CLAUER_OWNER+1] );

#endif
