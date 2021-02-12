
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

#ifndef __SESSION_H__
#define __SESSION_H__

#include "transport.h"
#ifdef DEBUG
#include "log.h"
#endif
#include "common.h"
#include "block.h"
#include <clio.h>

enum session_type { SESS_RDONLY,
		    SESS_RDWR };

typedef enum session_type session_type_t;

struct session {
    session_type_t type;       // The type of the session
    trans_object_t tr;         // The transport object to the client
    clauer_handle_t hClauer;   // The handle to the clauer object
    block_info_t *ib;           // The info block of the clauer
    char *pwd;            // The clauer's passphrase
    int opt_cryf;         // List cryf files on mounted system. ( floppy devices are not include ) 
    int opt_floppy;       // List cryf files on floppy devices.
	int admin;            // Indicates whether the client is admin or not. 
};

typedef struct session session_t;


int SESSION_New ( session_type_t type,
		  trans_object_t tr,
		  clauer_handle_t *hClauer,
		  char *pwd,
		  block_info_t *ib,
		  options_t * opt,
		  session_t **s);


#define LOG_TO 1  /* 1 level */
                  /* 2  */

#endif
