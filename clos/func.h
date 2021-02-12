
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

