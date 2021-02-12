
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

#include "smem.h"

#include <stdlib.h>

#ifdef LINUX
#include <sys/mman.h>
#elif defined(WIN32)
#include <windows.h>
#include <tchar.h>
#endif

#include "log.h"



/*! \brief Allocates a new buffer (b) of size bytes and deactivates 
 *         pagination.
 *
 * Allocates a new buffer (b) of size bytes and deactivates pagination.
 *
 * \param b
 *        The buffer to wich return the allocated memory.
 *
 * \param size
 *        The bytes to be allocated
 *
 * \retval ERR_INVALID_PARAMETER
 *         b is NULL or size is zero
 *
 * \retval ERR_OUT_OF_MEMORY
 *         The buffer cannot be allocated.
 *
 * \retval ERR_SMEM_CANNOT_LOCK
 *         The buffer cannot be locked into memory.
 *
 * \retval CLOS_SUCCESS
 *         Ok
 *
 * \remarks Use this kind of memory allocation when you want to
 *          protect sensible information like password or private
 *          keys. Don't abuse due to performance consecuences.
 */
 
int SMEM_New ( void **b, unsigned long size )
{ 

#ifdef WIN32
  SYSTEM_INFO sSysInfo; 
  DWORD dwPageSize;

  GetSystemInfo(&sSysInfo);
  dwPageSize = sSysInfo.dwPageSize;

  /* En windows para poder hacer un lock en condiciones
   * tenemos que reservar memoria para un múltiplo del
   * tamaño de página
   */

  if ( (size % dwPageSize) != 0 ) 
	size += dwPageSize - size % dwPageSize;

  LOG_Debug(1, "size = %ld", size);

#endif

    if ( ! b )
		return ERR_INVALID_PARAMETER;

    if ( ! size )
		return ERR_INVALID_PARAMETER;

#ifdef LINUX
    *b = malloc ( size );
#elif defined(WIN32)
	*b = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
#endif

    if ( ! *b ) 
		return ERR_OUT_OF_MEMORY;

#ifdef LINUX
    if ( mlock (*b, size) == -1 ) {
		free(*b);
		return ERR_SMEM_CANNOT_LOCK;
    }
#elif defined(WIN32)
	LOG_Debug(1, "Antes del lock. size: %d", size);

    if ( ! VirtualLock(*b, size) ) {
		VirtualFree(*b, 0, MEM_RELEASE);
		return ERR_SMEM_CANNOT_LOCK;
	}

	LOG_Msg(1, "Después del lock");

#endif

    return CLOS_SUCCESS;
}



/*! \brief Destroys, frees and unlocks the memory at b.
 *
 * Destroys, frees and unlocks the memory at b. b has to be allocated
 * with SMEM_New() function.
 *
 * \param b
 *        The buffer to be freed.
 *
 * \param size
 *        The size of b.
 *
 * \retval ERR_INVALID_PARAMETER
 *         Either b or size are zero.
 *
 * \retval ERR_SMEM_CANNOT_UNLOCK
 *         The memory cannot be unlocked.
 *
 * \retval CLOS_SUCCESS
 *         Ok
 */

int SMEM_Free (void *b, unsigned long size)
{

#ifdef WIN32
  SYSTEM_INFO sSysInfo; 
  DWORD dwPageSize;

  GetSystemInfo(&sSysInfo);
  dwPageSize = sSysInfo.dwPageSize;

  /* En windows para poder hacer un lock en condiciones
   * tenemos que reservar memoria para un múltiplo del
   * tamaño de página
   */

  if ( (size % dwPageSize) != 0 ) 
	size += dwPageSize - size % dwPageSize;

#endif

    if ( ! b )
		return ERR_INVALID_PARAMETER;

    if ( ! size )
		return ERR_INVALID_PARAMETER;

    SMEM_Destroy(b, size);

#ifdef LINUX
  if ( munlock( (const void *) b, size ) == -1 ) 
    return ERR_SMEM_CANNOT_UNLOCK;

  free(b);
#elif defined(WIN32)

	/* Para cuando alguien se atreva a desentrañar el intrincado
	 * mundo de los locks de páginas... 
	 */

	if ( ! VirtualUnlock((LPVOID) b, size) ) {
		LOG_Debug(1, "Cannot unlock: %ld", GetLastError());
		return ERR_SMEM_CANNOT_UNLOCK;
	}

	VirtualFree(b, 0, MEM_RELEASE);

#endif



    return CLOS_SUCCESS;
}



/*! \brief Destroys the contents of a buffer.
 *
 * Destroys the contents of a buffer.
 *
 * \param b
 *        The buffer to be "destroyed".
 *
 * \param size
 *        The size in bytes of the buffer.
 *
 * \retval ERR_INVALID_PARAMETER
 *         Either b or size are zero.
 *
 * \retval CLOS_SUCCESS
 *         Ok
 *
 */

int SMEM_Destroy ( void *b, unsigned long size )
{
    volatile unsigned char *aux = ( volatile unsigned char *) b;

    if ( ! b )
        return ERR_INVALID_PARAMETER;

    while ( size-- ) {
	*aux = 0;
	*aux = 0x55;
	*aux = 0xaa;
	*(aux++) = 0;
    }

    return CLOS_SUCCESS;

}


