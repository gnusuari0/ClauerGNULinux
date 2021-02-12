
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
#define FUNC_GET_CLOS_VERSION                 16 // The versi�n is hardcoded on the code must be increased 
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
