
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

#ifndef __STUB_H__
#define __STUB_H__

#include "transport.h"
#include "common.h"


#define FUNC_ENUMERATE_DEVICES                 0
#define FUNC_START_SESSION                     1
#define FUNC_READ_RESERVED_ZONE		       2
#define FUNC_WRITE_RESERVED_ZONE               3
#define FUNC_CHANGE_PASSPHRASE                 4
#define FUNC_READ_INFO_ZONE                    5
#define FUNC_READ_OBJECT_ZONE_BLOCK            6 
#define FUNC_READ_ENUM_FIRST_OBJECT_TYPE       7
#define FUNC_READ_ENUM_OBJECT_TYPE             8
#define FUNC_READ_ALL_TYPE_OBJECTS             9
#define FUNC_READ_ALL_OCCUPED_BLOCKS          10
#define FUNC_WRITE_OBJECT_ZONE_BLOCK          11
#define FUNC_INSERT_OBJECT_ZONE_BLOCK         12
#define FUNC_ERASE_OBJECT_BLOCK               13
#define FUNC_GET_HARDWARE_ID                  14
#define FUNC_REGENERATE_CACHE                 15
#define FUNC_GET_CLOS_VERSION                 16 // The versión is hardcoded on the code must be increased 
                                                 // on each release 
#define FUNC_GET_LOGICAL_UNIT                 17

#define FUNC_CLOSE_SESSION                    20 
#define FUNC_ENUMERATE_USBS                  128
#define FUNC_CREATE_CLAUER                   129
#define FUNC_GET_LAYOUT                      130
#define FUNC_FORMAT_DATA_PARTITION           131
#define FUNC_FORMAT_CRYPTO_PARTITION         132
	
#define FUNC_LOCK_CLAUER                      21 /* 14 */
#define FUNC_UNLOCK_CLAUER                    22 /* 15 */


int stub ( trans_object_t tr );
void timeout_handler ( int signal );






#endif
