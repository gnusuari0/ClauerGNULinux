
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

#include "block.h"
#include "smem.h"
#include <blocktypes.h>
#include <stdlib.h>

#include <CRYPTOWrapper/CRYPTOWrap.h>
#include <stdlib.h>

#include "log.h"

#ifdef WIN32
#define snprintf _snprintf
#endif


/*! \brief Allocates a new object block in a secure fashion.
 *
 * Allocates a new object block in a secure fashion. Use this function
 * with caution. Only for sensible objects.
 *
 * \param bo
 *        The buffer to be allocated.
 *
 * \retval ERR_INVALID_PARAMETER
 *         bo is NULL
 *
 * \retval SMEM_New()
 *         The same retur values of this function.
 *
 * \retval CLOS_SUCCESS
 *         Ok
 */

int BLOCK_OBJECT_New ( block_object_t **bo )
{
    int ret;

    if ( ! bo )
		return ERR_INVALID_PARAMETER;

    ret = SMEM_New ( (void **) bo, sizeof(block_object_t) );

    if ( ret != CLOS_SUCCESS ) 
		return ret;

    memset(*bo, 0, sizeof(block_object_t));

    return CLOS_SUCCESS;

}

/*! \brief Deallocates a block object previusly allocated with
 *         BLOCK_OBJECT_New().
 *
 * Deallocates a block object previously allocated with BLOCK_OBJECT_New().
 *
 * \param bo
 *        The block object to be allocated.
 *
 * \retval ERR_INVALID_PARAMTER
 *         bo is NULL
 *
 * \retval SMEM_Free()
 *         The same return values of this function
 *
 * \retval CLOS_SUCCESS
 *         Ok
 */

int BLOCK_OBJECT_Free ( block_object_t *bo )
{
    int ret;

    if ( ! bo )
		return ERR_INVALID_PARAMETER;

    ret = SMEM_Free(bo, sizeof(block_object_t));

    return ret;
}

/*! \brief Destroys the contents of a block object.
 *
 * Destroys the contents of a block object.
 *
 * \param bo
 *        The block object.
 *
 * \retval ERR_INVALID_PARAMETER
 *         bo is NULL.
 *
 * \retval SMEM_Destroy()
 *         The return values of this function
 *
 * \retval CLOS_SUCCESS
 *         Ok
 */

int BLOCK_OBJECT_Delete ( block_object_t *bo )
{
    if ( ! bo )
		return ERR_INVALID_PARAMETER;

    return SMEM_Destroy((void *)bo, sizeof(block_object_t));
}

/*! \brief Ciphers a block object with password pwd.
 *
 * Ciphers a block object with password pwd. It uses
 * PBES2 encryption scheme defined at PKCS#5 v2.0.
 *
 * \param ob
 *        The block object to be ciphered.
 *
 * \param pwd
 *        The password from wich to derive the key.
 *
 * \param ib
 *        The information block from wich to extract
 *        the salt information.
 *
 * \retval CLOS_SUCCESS
 *         Ok
 *
 * \retval ERR_INVALID_PARAMTER
 *         One or more of the parameters are invalid.
 * 
 * \retval ERR_CLOS
 *         Error
 *
 * \remarks Each time the function is called, the key
 *          derivation function is called. So this function
 *          is quite evil when you want to cipher a considerable
 *          amount of blocks. But this is something quite
 *          improbable.
 */

int BLOCK_OBJECT_Cipher   ( block_object_t *ob, char *pwd, block_info_t *ib )
{
    int size;
    unsigned char * aux= NULL;
	char auxMsg[512];

    if ( ! ob )
		return ERR_INVALID_PARAMETER;

    if ( ! pwd )
		return ERR_INVALID_PARAMETER;

    if ( ! ib ) 
		return ERR_INVALID_PARAMETER;

    aux = malloc(BLOCK_SIZE - 8);
    if ( ! aux ){
      return ERR_CLOS;
    }
    
    if ( CRYPTO_PBE_Cifrar ( pwd, ib->id, 20, 1000, 1, CRYPTO_CIPHER_DES_EDE3_CBC,
		ob->info, BLOCK_SIZE-8-8, aux, &size) != 0 ) {
      
	free( aux );
	return ERR_CLOS;
    }
    
    snprintf((char *)auxMsg, 512, "Tamano= %d ob->info= %d", size, sizeof(ob->info));
	LOG_Debug(1,"%s", auxMsg);

    memcpy((void *) ob->info, aux, BLOCK_SIZE-8);

    free(aux);
    ob->mode = 170;

    return CLOS_SUCCESS;
}

/*! \brief Deciphers a block object using password pwd.
 *
 * Deciphers a block object using password pwd and PBES2 defined
 * at PKCS#5 v2.0.
 *
 * \param ob
 *        The block object to be deciphered.
 *
 * \param pwd
 *        The password to be used to decipher the block object.
 *
 * \param ib
 *        The information block.
 *
 * \retval CLOS_SUCCESS
 *        Ok
 *
 * \retval ERR_CLOS
 *         Error
 *
 */

int BLOCK_OBJECT_Decipher ( block_object_t *ob, char *pwd, block_info_t *ib )
{
    block_object_t *nc_ob = NULL;
    int size;


    if ( ! ob )
		return ERR_INVALID_PARAMETER;

    if ( ! pwd ) 
		return ERR_INVALID_PARAMETER;

    if ( ! ib ) 
		return ERR_INVALID_PARAMETER;

    if ( BLOCK_OBJECT_New ( &nc_ob ) != CLOS_SUCCESS )
		return ERR_CLOS;

	/*nc_ob = (block_object_t * ) malloc ( sizeof(block_object_t) );*/

    memcpy((void *) nc_ob, ob, BLOCK_SIZE);

    if ( CRYPTO_PBE_Descifrar( pwd, ib->id, 20, 1000, 1, CRYPTO_CIPHER_DES_EDE3_CBC, 
		ob->info, BLOCK_SIZE-8, (unsigned char *) nc_ob->info, &size) != 0 ) {
		BLOCK_OBJECT_Free( nc_ob );
			/*free(nc_ob);*/
		return ERR_CLOS;
    }

    memcpy((void *) ob, nc_ob, BLOCK_SIZE);

    if ( BLOCK_OBJECT_Free( nc_ob ) != CLOS_SUCCESS )
		return ERR_CLOS;
	/*free(nc_ob);*/

    return CLOS_SUCCESS;
}

/*! \brief Returns the mode of a block object.
 *
 * Returns the mode of the block object.
 *
 * \param ob
 *        The object block.
 *
 * \retval MODE_EMPTY
 *         The block is empty.
 *
 * \retval MODE_CLEAR
 *         The block has an object inside and isn't ciphered.
 *
 * \retval MODE_CIPHERED
 *         The block has an object inside and is ciphered.
 *
 * \retval MODE_ERR
 *         The object has a mode that isn't recognized.
 */

object_mode_t BLOCK_OBJECT_Get_Mode ( block_object_t *ob )
{

    if ( ( ob->mode >= 0 ) && ( ob->mode <= 84 ) ) 
	return MODE_EMPTY;
    else if ( ( ob->mode >= 85 ) && ( ob->mode <= 169 ) )
	return MODE_CLEAR;
    else if ( ( ob->mode >= 170 ) && ( ob->mode <= 254 ) )
	return MODE_CIPHERED;	

    return MODE_ERR;
}

/*! \brief Creates a randomly generated empty block 
 *
 * Crates a randomly generated empty block.
 *
 * \param ob
 *        The object block to be returned.
 *
 * \retval CLOS_SUCCESS
 *         Ok
 *
 * \retval ERR_CLOS
 *         Error
 */

int BLOCK_OBJECT_New_Empty ( block_object_t *ob )
{
    if ( CRYPTO_Random(sizeof(*ob), (void *)ob) != 0 )
		return ERR_CLOS;

    ob->mode = 0;

    return CLOS_SUCCESS;
}

