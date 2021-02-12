
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

#include "session.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "err.h"


struct session_pool {
  session_t *first;
  session_t *last;
  unsigned long size;
};
typedef struct session_pool session_pool_t;


/*! \brief Allocates a new session object initilized with the parameters.
 *
 * Allocates a new session object initialized with the parameters.
 *
 * \param type
 *        The session type. It can be SESS_RDONLY or SESS_RDWR.
 *
 * \param tr
 *        The transport object to be used to communicate with the user.
 * 
 * \param hClauer
 *        The handleof the device
 * 
 * \param pwd
 *        The clauer's password. This buffer MUST BE ALLOCATED using
 *        SMEM_New().
 *
 * \param ib
 *        The information block of the device.
 *
 * \param s
 *        The session object that will be allocated.
 *
 * \retval CLOS_SUCCESS
 *         Ok
 *
 * \retval ERR_INVALID_PARAMETER
 *         One or more of the parameters are invalid.
 *
 * \retval ERR_OUT_OF_MEMORY
 *         Cannot allocate memory
 *
 * \remarks The buffers are not copied, so don't free them once the
 *          SESSION_New() has been called.
 *          Rememember that pwd must be allocated with SMEM_New()
 */

int SESSION_New ( session_type_t type,
		  trans_object_t tr,
		  clauer_handle_t *hClauer,
		  char *pwd,
		  block_info_t *ib,
		  options_t * opt,
		  session_t **s)
{

  if ( ! s )
    return ERR_INVALID_PARAMETER;

  *s = NULL;

  *s = ( session_t * ) malloc ( sizeof(session_t) );

  if ( ! *s ) 
    return ERR_OUT_OF_MEMORY;
  
  (*s)->type       = type;
  (*s)->tr         = tr;
  (*s)->hClauer    = hClauer;
  (*s)->pwd        = pwd;
  (*s)->ib         = ib;
  
  if ( opt ){
      (*s)->opt_cryf   = opt->cryf;
      (*s)->opt_floppy = opt->floppy;
  } 
  else {
      (*s)->opt_cryf   = 1;
      (*s)->opt_floppy = 1; /* Floppy is not longer working */
  }
 
  return CLOS_SUCCESS;

}




int SESSION_Free ( session_t *s )
{
	int ret;

	if ( s->pwd ) {
	    ret = SMEM_Free(s->pwd,(unsigned long) (strlen(s->pwd)));
		if ( ret != CLOS_SUCCESS )
			return ERR_CLOS;	
	}

	if ( s->ib )
		free(s->ib);

	return CLOS_SUCCESS;
}


