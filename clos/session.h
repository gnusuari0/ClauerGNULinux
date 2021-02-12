
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
