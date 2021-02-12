
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


