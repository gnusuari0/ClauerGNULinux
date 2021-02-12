
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

#ifndef __BLOCK_H__
#define __BLOCK_H__

#include <stdio.h>
#include <blocktypes.h>

#include "err.h"


enum object_mode {
	MODE_CIPHERED,
	MODE_CLEAR,
	MODE_EMPTY,
	MODE_ERR
};
typedef enum object_mode object_mode_t;

/* A generic block */

int BLOCK_OBJECT_New       ( block_object_t **block );
int BLOCK_OBJECT_Free      ( block_object_t *block );
int BLOCK_OBJECT_Delete    ( block_object_t *block );
int BLOCK_OBJECT_New_Empty ( block_object_t *ob );

object_mode_t BLOCK_OBJECT_Get_Mode ( block_object_t *ob );

int BLOCK_OBJECT_Cipher   ( block_object_t *ob, char *pwd, block_info_t *ib );
int BLOCK_OBJECT_Decipher ( block_object_t *ob, char *pwd, block_info_t *ib );

void BLOCK_PRINT_InfoBlock   ( block_info_t *block, FILE *fp );
void BLOCK_PRINT_ObjectBlock ( block_object_t *block, FILE *fp);

#endif
