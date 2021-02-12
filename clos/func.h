
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

#ifndef __FUNC_H__
#define __FUNC_H__

#ifdef DEBUG
#include "log.h"
#endif
#include "transport.h"
#include <clio/clio.h>
#include "block.h"
#include "session.h"

#define MAX_CLAUERS 1024
#define MAX_PATH_LEN 128
#define MAX_PASS_LEN 127
#define HW_ID_LEN    16  // It will be an md5

#define ERROR_NO_ADMIN 10 // Take care with libRT when changing this.

int FUNC_EnumerateDevices      ( session_t *s );
int FUNC_StartSession          ( session_t *s );
int FUNC_ReadReservedZone      ( session_t *s );
int FUNC_WriteReservedZone     ( session_t *s );
int FUNC_ChangePassphrase      ( session_t *s );
int FUNC_ReadInfoZone          ( session_t *s );
int FUNC_ReadObjectZoneBlock   ( session_t *s );
int FUNC_ReadEnumFirstObjectType   ( session_t *s );
int FUNC_ReadEnumObjectType        ( session_t *s );
int FUNC_ReadAllTypeObjects    ( session_t *s );
int FUNC_ReadAllOccupedBlocks  ( session_t *s );
int FUNC_WriteObjectZoneBlock  ( session_t *s );
int FUNC_InsertObjectZoneBlock ( session_t *s );
int FUNC_EraseObjectBlock      ( session_t *s );
int FUNC_GetHardwareId         ( session_t *s );
int FUNC_GetClosVersion        ( session_t *s );
int FUNC_GetLogicalUnitFromPhysicalDrive ( session_t *s );
int FUNC_CloseSession          ( session_t *s );
int FUNC_EnumerateDevices      ( session_t *s );
int FUNC_EnumerateUSBs         ( session_t *s );
int FUNC_CreateClauer          ( session_t *s );
int FUNC_FormatData            ( session_t *s );
int FUNC_FormatCrypto          ( session_t *s );
int FUNC_GetClauerLayout       ( session_t *s);
int FUNC_LockClauer            ( session_t *s );
int FUNC_UnlockClauer          ( session_t *s );

#endif

