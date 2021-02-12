
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
uente  original. Además, su denominación no debe inducir 
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

#include "func.h"

#include "block.h"
#include "session.h"
#include "clio.h"
#include "auth.h"
#include "common.h"
#include "stub.h"

#ifdef WIN32
#include "format.h"
#endif 

#include <CRYPTOWrapper/CRYPTOWrap.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


#ifdef LINUX
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#elif defined(WIN32)
#define snprintf _snprintf
#endif



enum { FUNC_OK,
       FUNC_ERROR,
       FUNC_ERROR_IO,
       FUNC_ERROR_PASS_LENGTH,
       FUNC_ERROR_SMEM,
       FUNC_ERROR_PASS_VERIFY
};



/*! \brief Sends a result to the client.
*
* Sends a result to the client.
*
* \param tr
*        The transport object.
*
* \param result
*        The result to be sent to the client. It can be
*        FUNC_OK or FUNC_ERROR
*
* \retval CLOS_SUCCESS
*         Ok
* 
* \retval TRANS_Send()
*         The same return values as TRANS_Send() function.
*/

int FUNC_SendResult ( trans_object_t tr, unsigned char result )
{
	return TRANS_Send(tr, &result, 1);
}


/***********************************************************
* Function 0. Enumerate Devices
* type  Size    Name         Sematics
*  <-    1      func_id      Function. Always take 0 value.
*  ->    1      num_disp     Number of inserted devices.
*  ->    1      disp_num_1   First device number.
*  ->    1      disp_num_2   Second device number.
*  ->    1      disp_num_3   Third device number
*  ...
*  ->    1      disp_num_n   N device number.
***********************************************************
*/

int FUNC_EnumerateDevices ( session_t *s )
{
	int  i;
	//int ret; 
	//unsigned char nclauers;
	//char * clauerPath[MAX_CLAUERS];
	int mode = IO_ENUM_ALL;

#ifdef LINUX
	mode= IO_ENUM_USB;
	if ( s->opt_cryf && s->opt_floppy )
		mode= IO_ENUM_ALL;
	else if ( s->opt_cryf )
		mode= IO_ENUM_FILES;
#endif


	LOG_Msg(LOG_TO,"Pasando por antes de TRANS_Send");
	if ( TRANS_Send(s->tr, (void *) &(child_info.nDevs), 1) != CLOS_SUCCESS ) 
		return ERR_CLOS;

	// New version implementing complete path return

	LOG_Debug(LOG_TO,"Antes de child_info.nDevs= %d",child_info.nDevs);

	for( i=0 ; i < child_info.nDevs ; i++ ){
		size_t aux;
		aux= strlen(child_info.devices[i]);

		LOG_Debug(LOG_TO,"Enviando  tam= %d",aux);
		if ( TRANS_Send(s->tr, (void *) &aux , 4) != CLOS_SUCCESS ) {
			LOG_Msg(LOG_TO,"[ERROR]Enviando tamaño");
			return ERR_CLOS;
		}
		LOG_Msg(LOG_TO,"OK");

		LOG_Debug(LOG_TO,"Enviando  child_info.devices = %s",child_info.devices[i]);
		if ( TRANS_Send(s->tr, (void *) child_info.devices[i] , (unsigned long) aux) != CLOS_SUCCESS ) {
			LOG_Debug(LOG_TO, "[ERROR]Enviando cadena %s", child_info.devices[i]);
			return ERR_CLOS;
		}	
		LOG_Msg(LOG_TO,"OK");
	}

	TRANS_Close_Ex(s->tr);
	LOG_Msg(LOG_TO,"Saliendo OK");
	return CLOS_SUCCESS;
}



/**************************************************************************************
* Function 1. Start Session
* type  Size     Name         Sematics
*  <-    1       func_id      Function. Always take 1 value.
*  <-    1       dev          Selected device.
*  <-    1       pass_len     Password Size, 0 claims for an unauthenticated session.
*  <-  pass_len  password     Password. Only if pass_len != 0
*  ->    1       error        Byte Error
*  ->   20       dev_id       Device identifier
**************************************************************************************
*/


int FUNC_StartSession ( session_t *s )
{
	//int deviceSel; 
	unsigned char passLen;
	void * aux = NULL;
	int ret, i;
	int lenDeviceSel;
	char deviceName[MAX_DEVICE_LEN+1]; /*, *pwd = NULL;*/
	block_info_t *ib = NULL;

	char auxMsg[512];

	LOG_BeginFunc(LOG_TO);

#ifdef DEBUG_CRYF

	ret = TRANS_Receive( s->tr, &lenDeviceSel, 4 );
	if ( ret != CLOS_SUCCESS ) {

		LOG_Error(LOG_TO, "Function 1. Receiving size of device selected. Returned value: %d", ret);
		return ERR_CLOS;
	}


	LOG_Debug(LOG_TO,"Received size of path = %d",lenDeviceSel);

	if ( lenDeviceSel > MAX_DEVICE_LEN  ){

		LOG_MsgError(LOG_TO, "Function 1. Receiving device selected too long.");
		return ERR_INVALID_PARAMETER;
	}

	//MARK
	ret = TRANS_Receive( s->tr, deviceName, lenDeviceSel );
	if ( ret != CLOS_SUCCESS ) {

		LOG_Error(LOG_TO, "Function 1. Receiving device selected. Returned value: %d", ret);
		return ERR_CLOS;
	}
	deviceName[lenDeviceSel]= '\0';


	LOG_Debug(LOG_TO,"Received device = %s",deviceName);

#else
	ret = TRANS_Receive( s->tr, &deviceSel, 1 );
	if ( ret != CLOS_SUCCESS ) {

		LOG_Error(LOG_TO, "Function 1. Receiving device selected. Returned value: %d", ret);
		return ERR_CLOS;
	}
#endif

	// printf("Device selected is %c\n",deviceSel);

	ret = TRANS_Receive( s->tr, &passLen, 1 );
	if ( ret != CLOS_SUCCESS ) {

		LOG_Error(LOG_TO, "Function 1. Receiving device selected. Returned value: %d", ret);
		return ERR_CLOS;
	}

	/*
	* Try to open the device
	*/


	// TODO: Is ok to try to open a device if LIBRT_ListarDispositivos is not called? 
	//if ( nclauers != 0 )
	//  deviceSel= clauers[deviceSel];
	//else
	//deviceSel= 'a';

#ifndef DEBUG_CRYF
	memset(deviceName, 0, MAX_DEVICE_LEN);
	snprintf(deviceName, MAX_DEVICE_LEN, "/dev/sd%c", (char) deviceSel);
#endif


	LOG_Debug(LOG_TO, "Opening device %s", deviceName);

	if ( passLen == 0 )
		s->type= SESS_RDONLY;
	else 
		s->type= SESS_RDWR;

	
	LOG_Debug(1,"Session type= %d ",s->type);
	ret = IO_Open(deviceName, &(s->hClauer), s->type, 0);

	if ( ret != IO_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR_IO);
		snprintf(auxMsg, 512, "Function 1. Trying to open device %s. Returned value: %d", deviceName, ret);
		LOG_Error(LOG_TO, "%s", auxMsg);
		return ERR_CLOS;
	}

	/*
	* Verifiy the parameters to determine if an authenticated
	* session is to be initiated
	*/

	if ( passLen > 0 ) {

		/* If the passphrase is greater than 127 -> ERROR */

		if ( passLen > 127 ) {	    
			LOG_Error(LOG_TO, "Passphrase sent by the client greater than 127 characters: %d", passLen);
			FUNC_SendResult(s->tr, FUNC_ERROR_PASS_LENGTH);
			return ERR_CLOS;
		}

		ret = SMEM_New ( &aux, passLen + 1 );
		if ( ret != CLOS_SUCCESS ) {
			FUNC_SendResult(s->tr, FUNC_ERROR_SMEM);

			LOG_Error(LOG_TO, "Function 1 : %d", ret);
			return ERR_CLOS;
		}


		ret = TRANS_Receive(s->tr, aux, passLen);
		if ( ret != CLOS_SUCCESS ) {
			SMEM_Free(aux, passLen+1);
			FUNC_SendResult(s->tr, FUNC_ERROR);

			LOG_Error(LOG_TO, "Function 1. Receiving password: %d", ret);
			return ERR_CLOS;
		}

		*((unsigned char *)aux + passLen) = 0;

		/*
		* Now verify the password
		*/
		// 
		// #ifdef DEBUG
		// LOG_Debug(LOG_TO,"[0] La password es %s ",aux);
		// #endif

		ret = AUTH_VerifyClauerPassphrase(s->hClauer, (char *) aux);

		// fflush(stdout);

		// 
		// #ifdef DEBUG
		// LOG_Debug(LOG_TO,"[1] La password es %s ",s->pwd);
		// #endif

		switch ( ret ) {
	case ERR_AUTH_INVALID_PASSPHRASE:
		SMEM_Free(aux, passLen+1);
		FUNC_SendResult(s->tr, FUNC_ERROR_PASS_VERIFY);

		LOG_MsgError(LOG_TO, "Function 1. Invalid passphrase");
		return ERR_CLOS;

	case CLOS_SUCCESS:
		//SMEM_New(s->pwd,passLen+1);
		//memcpy(s->pwd,aux,passLen+1);
		break;

	default:
		SMEM_Free(aux, passLen+1);
		FUNC_SendResult(s->tr, FUNC_ERROR_PASS_VERIFY);

		LOG_MsgError(LOG_TO, "Function 1. Verifying clauer's passphrase");
		return ERR_CLOS;
		}

	}

	/* 
	* Read the information block
	*/

	s->ib= NULL;
	LOG_Debug(LOG_TO,"NDEVS= %d", child_info.nDevs );
	for ( i=0 ; i < child_info.nDevs ; i++ ){
		LOG_Debug(LOG_TO,"Paso i= %d ",i);
		if ( strncmp( child_info.devices[i], deviceName, MAX_DEVICE_LEN ) == 0 ){
			LOG_Debug(LOG_TO,"Entro 1 i= %d ",i);
			s->ib = child_info.ibs[i];
			LOG_Debug(LOG_TO,"ib.totalBlocks= %d ", child_info.ibs[i]->totalBlocks);
			ret= IO_SetInfoBlock(s->hClauer, child_info.ibs[i]);  
			LOG_Debug(LOG_TO,"Entro 2 i= %d ",i);
			LOG_Debug(LOG_TO,"Entro 3 i= %d ",i);
			if ( ret != IO_SUCCESS ) {
				SMEM_Free(aux, passLen+1);
				free(ib);
				FUNC_SendResult(s->tr, FUNC_ERROR);
				LOG_Error(LOG_TO, "Function 1. Reading information block: %d", ret);
				return ERR_CLOS;
			}
			LOG_Debug(LOG_TO,"Entro 4 i= %d ",i);
		}
		LOG_Debug(LOG_TO,"Entro 5 i= %d ",i);
	}

	LOG_Msg(LOG_TO,"Salimos !!!!!!");

	if ( ! s->ib ) {
		SMEM_Free(aux, passLen + 1);
		FUNC_SendResult(s->tr, FUNC_ERROR);
		LOG_Error(LOG_TO, "Function 1 : %d", ret);
		return ERR_CLOS;
	}

	// TODO: that's temporaly, the hwId must be obtained from the device 
	//       by the time 16 0's are writen.

	memset(s->ib->hwId, 0, HW_ID_LEN);

	/*
	* Everything is ok. Inform the client
	*/

	if ( FUNC_SendResult(s->tr, FUNC_OK) != CLOS_SUCCESS ) {
		SMEM_Free(aux, passLen+1);
		free(ib);

		LOG_MsgError(LOG_TO, "Function 1. Sending ok");
		return ERR_CLOS;
	}

	/*
	* We send the clauer's id : 20 bytes
	*/

	if ( TRANS_Send(s->tr, (void *) s->ib->id, 20) != CLOS_SUCCESS ) {
		SMEM_Free(aux, passLen + 1);
		free(ib);

		LOG_MsgError(LOG_TO, "Function 1. Sending device id");
		return ERR_CLOS;
	}

	s->type = ( passLen > 0 ) ? SESS_RDWR : SESS_RDONLY;

	s->pwd  = aux;

	// 
	// #ifdef DEBUG
	// LOG_Debug(LOG_TO,"[3] La password es %s ",s->pwd);
	// #endif

	return CLOS_SUCCESS;
}






/* *****************************************************************
* Function 2. Read Reserved Zone.
* type  Size     Name         Sematics
*  <-    1       func_id      Function. Always take 2 value.
*  ->    1       error        Byte Error.
*  ->    4       res_size     Tamaño de la zone reservada en bytes.
*  -> res_size   res_zone     Bloques con la zona reservada.
*******************************************************************
*/

int FUNC_ReadReservedZone ( session_t *s )
{
	unsigned char *rZone = NULL;
	int ret, rzSize;
	unsigned int i=0;
	char aux[512];


	LOG_BeginFunc(LOG_TO);
	rzSize = BLOCK_SIZE * (s->ib->rzSize);


	LOG_Debug(LOG_TO,"El Tamanyo de la zona reservada es %d\n", rzSize);

	rZone = ( unsigned char * ) malloc ( rzSize );
	if ( ! rZone ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);

		LOG_MsgError(LOG_TO, "Out of memory error");
		goto err_FUNC_ReadReservedZone;
	}

	ret = IO_Seek(s->hClauer, 0, IO_SEEK_RESERVED);
	if ( ret != IO_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);

		LOG_Error(LOG_TO, "Positioning file pointer in reserved zone: %d", ret);
		goto err_FUNC_ReadReservedZone;
	}

	for ( i = 0 ; i < s->ib->rzSize ; i++ ) {
		/* 
		This has to be improved. But reading the reserved zone
		is highly improbable
		*/

		snprintf(aux, 512, "hClauer= 0x%x IO_SEEK_RESERVED=%d ",s->hClauer,IO_SEEK_RESERVED);
		LOG_Debug(LOG_TO, "%s", aux);
		ret = IO_Read(s->hClauer, rZone + i * BLOCK_SIZE);
		if ( ret != IO_SUCCESS ) {
			FUNC_SendResult(s->tr, FUNC_ERROR);

			LOG_Error(LOG_TO, "Read error: %d", ret);
			goto err_FUNC_ReadReservedZone;
		}

	}

	if ( FUNC_SendResult(s->tr, FUNC_OK) != CLOS_SUCCESS ) {

		LOG_MsgError(LOG_TO, "Sending OK to the client");
		goto err_FUNC_ReadReservedZone;
	}

	ret = TRANS_Send(s->tr, &rzSize, 4);
	if ( ret != CLOS_SUCCESS ) {

		LOG_MsgError(LOG_TO, "Sending reserved zone size");
		goto err_FUNC_ReadReservedZone;
	}

	ret = TRANS_Send(s->tr, (void *) rZone, BLOCK_SIZE * (s->ib->rzSize));
	if ( ret != CLOS_SUCCESS ) {

		LOG_Error(LOG_TO, "Sending data: %d", ret);
		goto err_FUNC_ReadReservedZone;
	}


	if ( SMEM_Destroy((void *) rZone, rzSize) != CLOS_SUCCESS ) {

		LOG_MsgError(LOG_TO, "Error destroying reserved zone from memory");
		goto err_FUNC_ReadReservedZone;
	}

	free(rZone);
	rZone = NULL;


	LOG_EndFunc(LOG_TO, CLOS_SUCCESS);

	return CLOS_SUCCESS;

err_FUNC_ReadReservedZone:

	if ( rZone ) {
		SMEM_Destroy((void *) rZone, rzSize);
		free(rZone);
		rZone = NULL;
	}


	LOG_EndFunc(LOG_TO, ret);

	return ret;

}




/*
* ***********************************************************
* Function 3. Write Reserved Zone.
* type  Size     Name         Sematics
*  <-    1       func_id      Function. Always take 3 value.
*  <-    4       buf_len      Size of data to be writen.
*  <- buf_len  res_zone_buf   Data to be writen.
*  ->    1       error        Byte Error.
*************************************************************
*/


/* 
*  TODO: This function always overwrites the firs reserved block,
*        the correct behaviour will be to look for the first free 
*        block and write the data there. 
*/

int FUNC_WriteReservedZone ( session_t *s )
{
	int ret, err = FUNC_OK;
	unsigned int rzSize=0;
	unsigned char block[BLOCK_SIZE];


	LOG_BeginFunc(LOG_TO);

	/* If session is readonly, receive the parameters and then
	* inform the client of this error.
	*/

	if ( s->type == SESS_RDONLY ) {
		err = FUNC_ERROR;

		LOG_MsgError(LOG_TO, "Session is read-only");
		// goto err_FUNC_WriteReservedZone;
	}

	if ( TRANS_Receive(s->tr, (void *) &rzSize, 4) != CLOS_SUCCESS ) {	
		LOG_MsgError(LOG_TO, "Error receiving buffer size");
		ret = ERR_FUNC_ABORT;
		goto err_FUNC_WriteReservedZone;
	}

	if ( rzSize > (s->ib->rzSize * BLOCK_SIZE) ) {
		ret = ERR_FUNC_ABORT;

		LOG_MsgError(LOG_TO, "Buffer is larger than reserved zone");
		goto err_FUNC_WriteReservedZone;
	}


	if ( (ret = IO_Seek(s->hClauer, 0, IO_SEEK_RESERVED)) != IO_SUCCESS ) {

		LOG_Error(LOG_TO, "Error positioning file pointer in the reserved zone: %d", ret);
		ret = ERR_FUNC_ABORT;
		goto err_FUNC_WriteReservedZone;
	}

	memset(block, 0, sizeof(block));

	while ( rzSize >= BLOCK_SIZE ) {

		if ( TRANS_Receive ( s->tr, (void *) block, BLOCK_SIZE ) != CLOS_SUCCESS ) {
			ret = ERR_FUNC_ABORT;

			LOG_MsgError(LOG_TO, "Error receiving reserved zone block");
			goto err_FUNC_WriteReservedZone;
		}

		if ( err == FUNC_OK ) {

			LOG_Msg(LOG_TO, "I'm writing");
			ret = IO_Write ( s->hClauer, block );
			if ( ret != IO_SUCCESS ) {
				/* If we've a write error, we don't return. Receive all
				* the data from client and send and error to him
				*/
				err = FUNC_ERROR;

				LOG_Error(LOG_TO, "Error writing reserved zone: %d", ret);
			}
		}

		rzSize -= BLOCK_SIZE;
	}

	if ( rzSize > 0 ) {

		if ( TRANS_Receive ( s->tr, (void *) block, rzSize ) != CLOS_SUCCESS) {
			ret = ERR_FUNC_ABORT;

			LOG_MsgError(LOG_TO, "Error receiving reserved zone block");
			goto err_FUNC_WriteReservedZone;
		}

		if ( ! err ) {
			ret = IO_Write ( s->hClauer, block );
			if ( ret != IO_SUCCESS ) {
				err = FUNC_ERROR;

				LOG_Error(LOG_TO, "Error receiving reserved zone block: %d", ret);
			}
		}

	}

	if ( SMEM_Destroy((void *) block, BLOCK_SIZE) != CLOS_SUCCESS ) {
		ret = ERR_FUNC_ABORT;

		LOG_MsgError(LOG_TO, "Error destroying memory");
		goto err_FUNC_WriteReservedZone;
	}

	ret = FUNC_SendResult(s->tr, err);
	if ( ret != CLOS_SUCCESS ) {

		LOG_Error(LOG_TO, "Error sending OK to the client : %d", ret);
		goto err_FUNC_WriteReservedZone;
	}

	return CLOS_SUCCESS;

err_FUNC_WriteReservedZone:

	SMEM_Destroy((void *) block, BLOCK_SIZE);


	LOG_EndFunc(LOG_TO, ret);

	return ret;
}



/*
************************************************************
* Function 4. Change clauer password.
* type  Size      Name         Sematics
*  <-    1        func_id      Function. Always take 4 value.
*  <-    1        bytes_pass   Size of the new password.
*  <- bytes_pass  password     New password.
*  ->    1        error        Byte Error.
*************************************************************
*/


int FUNC_ChangePassphrase ( session_t *s )
{
	unsigned char passSize, newIden[40];
	char pass[128];
	int ret = 0, aux, err = FUNC_OK;
	block_info_t ib;
	long b, to;
	block_object_t ob;
	void * auxpass;
	char auxMsg[512];

	LOG_Msg(LOG_TO,"FUNC_ChangePassphrase");

	if ( s->type == SESS_RDONLY ) {
		err = FUNC_ERROR;


		LOG_MsgError(LOG_TO, "Session is readonly");
	}

	ret = TRANS_Receive (s->tr, &passSize, 1);
	if ( ret != CLOS_SUCCESS ) {


		LOG_Error(LOG_TO, "Error receiving passphrase size: %d", ret);

		goto err_FUNC_ChangePassphrase;
	}

	if ( passSize > 127 ) {


		LOG_MsgError(LOG_TO, "Password size too large");

		ret = ERR_FUNC_ABORT;
		goto err_FUNC_ChangePassphrase;
	}

	ret = TRANS_Receive (s->tr, pass, passSize );
	if ( ret != CLOS_SUCCESS ) {


		LOG_MsgError(LOG_TO, "Error receiving passphrase");

		goto err_FUNC_ChangePassphrase;
	}
	pass[passSize]='\0';

	if ( err == FUNC_OK ) {
		/*
		Now change the passphrase
		*/

		ret = IO_ReadInfoBlock ( s->hClauer, &ib );


		if ( ret != IO_SUCCESS ) {
			FUNC_SendResult(s->tr, FUNC_ERROR);
			LOG_Error(LOG_TO, "Error reading information block: %d", ret);
			goto err_FUNC_ChangePassphrase;
		}

		if ( CRYPTO_PBE_Cifrar ( pass, ib.id, 20, 1000, 1, CRYPTO_CIPHER_DES_EDE3_CBC, (unsigned char *)"UJI - Clauer PKI storage system", 32, newIden, &aux ) != 0 ) {
			FUNC_SendResult(s->tr, FUNC_ERROR);
			LOG_MsgError(LOG_TO, "Error ciphering iden string");
			goto err_FUNC_ChangePassphrase;
		}

		memcpy(ib.idenString, newIden, 40);

		ret = IO_Seek ( s->hClauer, 0, IO_SEEK_INFO);
		if ( ret != IO_SUCCESS ) {
			FUNC_SendResult(s->tr, FUNC_ERROR);
			LOG_MsgError(LOG_TO, "Error positioning over the info zone");
			goto err_FUNC_ChangePassphrase;
		}

		ret = IO_Write(s->hClauer, (unsigned char *) &ib);
		if ( ret != IO_SUCCESS ) {
			FUNC_SendResult(s->tr, FUNC_ERROR);
			LOG_MsgError(LOG_TO, "Error writing new info block");
			goto err_FUNC_ChangePassphrase;
		}


		ret = IO_Seek ( s->hClauer, 0, IO_SEEK_OBJECT);
		if ( ret != IO_SUCCESS ) {
			FUNC_SendResult(s->tr, FUNC_ERROR);

			LOG_MsgError(LOG_TO, "Error positioning over the object zone");
			goto err_FUNC_ChangePassphrase;
		}

		to = ( ib.cb == -1 ) ? ib.totalBlocks-1 : ib.cb;
		for ( b = 0 ; b < to ; b++ ) {

			ret = IO_Read(s->hClauer, (unsigned char *) &ob);
			if ( ret != IO_SUCCESS ) {
				FUNC_SendResult(s->tr, FUNC_ERROR);

				LOG_MsgError(LOG_TO, "Error reading block object");
				goto err_FUNC_ChangePassphrase;
			}


			if ( BLOCK_OBJECT_Get_Mode(&ob) == MODE_CIPHERED ) {

				ret = BLOCK_OBJECT_Decipher(&ob, s->pwd, &ib);
				if ( ret != CLOS_SUCCESS ) {
					FUNC_SendResult(s->tr, FUNC_ERROR);
					snprintf(auxMsg, 512, "Error deciphering block object %d type= %d", b, ob.type);
					LOG_MsgError(LOG_TO, aux);
					goto err_FUNC_ChangePassphrase;
				}


				ret = BLOCK_OBJECT_Cipher(&ob, pass, &ib);
				if ( ret != CLOS_SUCCESS ) {
					FUNC_SendResult(s->tr, FUNC_ERROR);

					LOG_Error(LOG_TO, "Error ciphering block object %d", b);
					goto err_FUNC_ChangePassphrase;
				}

				ret = IO_Seek(s->hClauer, b, IO_SEEK_OBJECT);
				if ( ret != IO_SUCCESS ) {
					FUNC_SendResult(s->tr, FUNC_ERROR);

					LOG_MsgError(LOG_TO, "Error positioning file pointer");
					goto err_FUNC_ChangePassphrase;
				}

				ret = IO_Write(s->hClauer, (unsigned char *) &ob);
				if ( ret != IO_SUCCESS ) {
					FUNC_SendResult(s->tr, FUNC_ERROR);

					LOG_MsgError(LOG_TO, "Error writing ciphered block");
					goto err_FUNC_ChangePassphrase;
				}
			}
		}

		SMEM_Free(s->pwd,(unsigned long) (strlen(s->pwd)+1));
		ret = SMEM_New ( &auxpass, (unsigned long) (strlen(pass) + 1) );
		if ( ret != CLOS_SUCCESS ) {
			FUNC_SendResult(s->tr, FUNC_ERROR);

			LOG_Error(LOG_TO, "Function 4 : %d", ret);
			return ERR_CLOS;
		}

		pass[strlen(pass)+1]='\0';
		memcpy(auxpass, pass, strlen(pass)+1);
		s->pwd= auxpass;

		if ( SMEM_Destroy((void *) pass, 128) != CLOS_SUCCESS ) {
			FUNC_SendResult(s->tr, FUNC_ERROR);

			LOG_MsgError(LOG_TO, "Error destroying password");
			goto err_FUNC_ChangePassphrase;
		}

	}

	ret = FUNC_SendResult(s->tr, FUNC_OK);
	if ( ret != CLOS_SUCCESS ) {

		LOG_Error(LOG_TO, "Error sending OK to the client : %d", ret);
		goto err_FUNC_ChangePassphrase;
	}


	LOG_EndFunc(LOG_TO, CLOS_SUCCESS);
	return CLOS_SUCCESS;

err_FUNC_ChangePassphrase:

	SMEM_Destroy((void *) pass, 128);


	LOG_EndFunc(LOG_TO, ret);
	return ret;
}




/*
************************************************************
* Function 5. Read Info block.
* type  Size      Name         Sematics
*  <-    1        func_id      Function. Always take 5 value.
*  ->    1        error        Byte Error.     
*  ->  10240      info_block   Information block.       
*************************************************************
*/

int FUNC_ReadInfoZone ( session_t *s )
{
	block_info_t ib;
	int ret;


	LOG_BeginFunc(LOG_TO);

	ret = IO_Seek(s->hClauer, 0, IO_SEEK_INFO);
	if ( ret != IO_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);
		goto err_FUNC_ReadInfoZone;
	}

	ret = IO_Read(s->hClauer, (unsigned char *) &ib);
	if ( ret != IO_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);
		goto err_FUNC_ReadInfoZone;
	}

	ret = FUNC_SendResult(s->tr, FUNC_OK);
	if ( ret != CLOS_SUCCESS ) {

		LOG_Error(LOG_TO, "Error sending OK to the client : %d", ret);
		ret = ERR_FUNC_ABORT;
		goto err_FUNC_ReadInfoZone;
	}

	ret = TRANS_Send(s->tr, (void *) &ib, BLOCK_SIZE);
	if ( ret != CLOS_SUCCESS ) {

		LOG_Error(LOG_TO, "Error sending information block to client: %d", ret);
		ret = ERR_FUNC_ABORT;
		goto err_FUNC_ReadInfoZone;
	}


	LOG_EndFunc(LOG_TO, CLOS_SUCCESS);

	return CLOS_SUCCESS;

err_FUNC_ReadInfoZone:


	LOG_EndFunc(LOG_TO, ret);

	return ret;
}


/*
************************************************************
* Function 6. Read object zone block.
* type  Size      Name         Sematics
*  <-    1        func_id      Function. Always take 6 value.
*  <-    4        block_num    Block number to be read.
*  ->    1        error        Byte Error.     
*  ->  10240      bloque       The block itself.       
*************************************************************
*/

int FUNC_ReadObjectZoneBlock ( session_t *s )
{
	block_object_t *ob = NULL;
	int ret;
	unsigned long nBlock = 0;
	char aux[512];

	LOG_BeginFunc(6);

	LOG_Msg(LOG_TO, "1");
	ret = TRANS_Receive(s->tr, (void *) &nBlock, 4);
	if ( ret != CLOS_SUCCESS ) {
		LOG_Error(LOG_TO, "Error receiving block number : %d", ret);
		goto err_FUNC_ReadObjectZoneBlock;
	}
	LOG_Msg(LOG_TO, "2");
	ret = BLOCK_OBJECT_New ( &ob );
	if ( ret != CLOS_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);
		LOG_Error(LOG_TO, "Error allocating memory : %d", ret);
		goto err_FUNC_ReadObjectZoneBlock;
	}
	LOG_Msg(LOG_TO, "3");
	if ( nBlock >= s->ib->totalBlocks ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);
		snprintf(aux, 512, "Block requested out of available blocks. Block requested: %d. Total blocks: %d", nBlock, s->ib->totalBlocks);
		LOG_MsgError(LOG_TO, aux);
		goto err_FUNC_ReadObjectZoneBlock;
	}
	LOG_Msg(LOG_TO, "4");
	ret = IO_Seek(s->hClauer, nBlock, IO_SEEK_OBJECT);
	if ( ret != IO_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);
		LOG_Error(LOG_TO, "Error positioning device pointer: %d", ret);
		goto err_FUNC_ReadObjectZoneBlock;
	}
	LOG_Msg(LOG_TO, "5");
	ret = IO_Read(s->hClauer, (unsigned char *) ob);
	if ( ret != IO_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);	
		LOG_Error(LOG_TO, "Error reading block : %d", ret);
		goto err_FUNC_ReadObjectZoneBlock;
	}
	LOG_Msg(LOG_TO, "6");
	if ( BLOCK_OBJECT_Get_Mode ( ob ) == MODE_CIPHERED ) {
		ret = BLOCK_OBJECT_Decipher(ob, s->pwd, s->ib);
		if ( ret != CLOS_SUCCESS ) {
			FUNC_SendResult(s->tr, FUNC_ERROR);		    
			LOG_Error(LOG_TO, "Error deciphering block : %d", ret);
			goto err_FUNC_ReadObjectZoneBlock;
		}
	}	
	LOG_Msg(LOG_TO, "7");
	ret = FUNC_SendResult(s->tr, FUNC_OK);
	if ( ret != CLOS_SUCCESS ) {
		LOG_Error(LOG_TO, "Error sending result to the client : %d", ret);
		goto err_FUNC_ReadObjectZoneBlock;
	}
	LOG_Msg(LOG_TO, "8");
	ret = TRANS_Send(s->tr, (void *) ob, BLOCK_SIZE);
	if ( ret != CLOS_SUCCESS ) {
		LOG_Error(LOG_TO, "Error sending object block : %d", ret);
		goto err_FUNC_ReadObjectZoneBlock;
	}

	LOG_Msg(LOG_TO, "Apunto de hacer el free");
	ret = BLOCK_OBJECT_Free(ob);
	if ( ret != CLOS_SUCCESS ) {
		LOG_Error(LOG_TO, "Error freing memory : %d", ret);
		goto err_FUNC_ReadObjectZoneBlock;
	}
	LOG_Msg(LOG_TO, "9");
	LOG_EndFunc(LOG_TO, ret);

	return CLOS_SUCCESS;

err_FUNC_ReadObjectZoneBlock:

	if ( ob ) {
		LOG_Msg(LOG_TO, "Desde error");
		BLOCK_OBJECT_Free(ob);
		LOG_Msg(LOG_TO, "Después desde error");
	}

	LOG_EndFunc(LOG_TO, ret);

	return ret;
}


/*
************************************************************
* Function 7. Read first object zone block of given kind.
* type  Size      Name         Sematics
*  <-    1        func_id      Function. Always take 7 value.
*  <-    1        type         Block type to be read.
*  ->    1        error        Byte Error.     
*  ->    4        block_num    Position of the block.       
*  ->  10240      bloque       The block itself.       
*************************************************************
*/

int FUNC_ReadEnumFirstObjectType ( session_t *s )
{
	block_object_t ob;
	long b, to;
	unsigned char type;
	int ret, found = 0;
	char aux[512];

	LOG_BeginFunc(LOG_TO);

	ret = TRANS_Receive (s->tr, &type, 1);
	if ( ret != CLOS_SUCCESS ) {
		LOG_MsgError(LOG_TO, "Error receiving block type");
		goto err_FUNC_ReadEnumFirstObjectType;
	}

	LOG_Debug(LOG_TO,"Looking for type: 0x%02x",type); 
	ret = IO_Seek(s->hClauer, 0, IO_SEEK_OBJECT);
	if ( ret != IO_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);
		LOG_Error(LOG_TO, "Error positioning device pointer : %d", ret);
		goto err_FUNC_ReadEnumFirstObjectType;
	}

	to = ( s->ib->cb == -1 ) ? s->ib->totalBlocks : s->ib->cb;

	snprintf(aux, 512, "s->ib->cb= %d,  s->ib->totalBlocks= %d, to= %d",s->ib->cb,s->ib->totalBlocks,to);
	LOG_Debug(LOG_TO,"%s", aux);

	/* 
	* to == 0 means an empty crypto zone so, we must return nblock=-1 and
	* no error because CLOS have done its work properly  
	*/

	/* if ( to == 0 ){

	#ifdef DEBUG
	LOG_Debug(LOG_TO,"Return an error and exit: %d",FUNC_ERROR);
	#endif
	FUNC_SendResult(s->tr, FUNC_ERROR);
	return CLOS_SUCCESS;
	}*/

	for ( b = 0 ; ! found && (b < to ) ; b++ ) {

		ret = IO_Read(s->hClauer, (unsigned char *) &ob);
		if ( ret != IO_SUCCESS ) {
			FUNC_SendResult(s->tr, FUNC_ERROR);

			snprintf(aux, 512, "Error reading block %ld : %d", b, ret);
			LOG_MsgError(LOG_TO, aux);
			goto err_FUNC_ReadEnumFirstObjectType;
		}

		if ( ob.type == type && BLOCK_OBJECT_Get_Mode(&ob) != MODE_EMPTY ) {

			snprintf(aux, 512, "            MODE = %d BLOCK= %d ", ob.mode, b);
			LOG_Debug(LOG_TO, "%s", aux);
			/*
			if the block is ciphered and we're in a non authenticated
			session, we haven't found it yet
			*/

			if ( ( s->type == SESS_RDWR ) || 
				( BLOCK_OBJECT_Get_Mode(&ob) != MODE_CIPHERED ) ) 
				found = 1; 
		}
	}

	if ( found ) {

		--b;

		if ( BLOCK_OBJECT_Get_Mode(&ob) == MODE_CIPHERED ) {

			ret = BLOCK_OBJECT_Decipher(&ob, s->pwd, s->ib);
			if ( ret != CLOS_SUCCESS ) {
				FUNC_SendResult(s->tr, FUNC_ERROR);
				snprintf(aux, 512, "Error deciphering block %d : %d", b, ret);
				LOG_MsgError(LOG_TO, aux);
				goto err_FUNC_ReadEnumFirstObjectType;
			}

		}

		ret = FUNC_SendResult(s->tr, FUNC_OK);
		if ( ret != CLOS_SUCCESS ) {

			LOG_Error(LOG_TO, "Error sending OK result to the client: %d", ret);
			goto err_FUNC_ReadEnumFirstObjectType;
		}

		ret = TRANS_Send(s->tr, (void *) &b, 4);
		if ( ret != CLOS_SUCCESS ) {

			LOG_Error(LOG_TO, "Error sending block number : %d", ret);
			goto err_FUNC_ReadEnumFirstObjectType;
		}

		ret = TRANS_Send(s->tr, (void *) &ob, 10240);
		if ( ret != CLOS_SUCCESS ) {

			LOG_MsgError(LOG_TO, "Error sending block.");
			goto err_FUNC_ReadEnumFirstObjectType;
		}

	} else {

		b = -1;

		ret = FUNC_SendResult(s->tr, FUNC_OK);
		if ( ret != CLOS_SUCCESS ) {
			LOG_Error(LOG_TO, "Error sending OK result to the client: %d"
				, ret);
			goto err_FUNC_ReadEnumFirstObjectType;
		}

		ret = TRANS_Send(s->tr, (void *) &b, 4);
		if ( ret != CLOS_SUCCESS ) {

			LOG_Error(LOG_TO, "Error sending block number : %d", ret);
			goto err_FUNC_ReadEnumFirstObjectType;
		}

	}

	ret = BLOCK_OBJECT_Delete ( &ob );
	if ( ret != CLOS_SUCCESS ) {	
		LOG_Error(LOG_TO, "Error destroying object block : %d", ret);
		goto err_FUNC_ReadEnumFirstObjectType;
	}


	LOG_EndFunc(LOG_TO, CLOS_SUCCESS);

	return CLOS_SUCCESS;

err_FUNC_ReadEnumFirstObjectType:

	BLOCK_OBJECT_Delete(&ob);


	LOG_EndFunc(LOG_TO, ret);

	return ret;
}




/*
********************************************************************
* Function 8. Read next object zone block of given kind.
* type  Size      Name           Sematics
*  <-    1        func_id        Function. Always take 8 value.
*  <-    4        block_num_ini  Start position to find next block.
*  ->    1        error          Byte Error.     
*  ->    4        block_num      Position of the block.       
*  ->  10240      bloque         The block itself.       
*******************************************************************
*/

int FUNC_ReadEnumObjectType ( session_t *s )
{
	unsigned char type;
	long nBlock, to, b;
	block_object_t ob;
	int ret, found= 0;
	char aux[512];


	LOG_BeginFunc(LOG_TO);

	ret = TRANS_Receive(s->tr, &type, 1);

	LOG_Debug(LOG_TO,"Received type 0x%02x",type);	

	if ( ret != CLOS_SUCCESS ) {

		LOG_Error(LOG_TO, "Error receiving parameter type : %d", ret);
		goto err_FUNC_ReadEnumObjectType;
	}

	ret = TRANS_Receive(s->tr, &nBlock, 4);

	LOG_Debug(LOG_TO,"nBlock received nBlock=%d ",nBlock);

	/* The beginning block is not considered */
	nBlock += 1;

	if ( ret != CLOS_SUCCESS ) {

		LOG_Error(LOG_TO, "Error receiving parameter number of block: %d", ret);
		goto err_FUNC_ReadEnumObjectType;
	}

	to = ( s->ib->cb == -1 ) ? s->ib->totalBlocks : s->ib->cb;
	LOG_Debug( LOG_TO,"HANDER TO =  %d", to );
	if ( nBlock >= to ) {

		LOG_Debug( LOG_TO,"HANDER ENTRANDO  nBLOCK =  %d", nBlock );
		//
		// Here we must return nBlock=-1 because we can't found it
		//
		b = -1;
		ret = FUNC_SendResult(s->tr, FUNC_OK);
		if ( ret != CLOS_SUCCESS ) {
			LOG_Error(LOG_TO, "Error sending OK result to the client: %d", ret);
			goto err_FUNC_ReadEnumObjectType;
		}

		ret = TRANS_Send(s->tr, (void *) &b, 4);

		LOG_Debug( LOG_TO,"DEVUELTO b= %d", b );
		if ( ret != CLOS_SUCCESS ) {

			LOG_Error(LOG_TO, "Error sending block number : %d", ret);
			goto err_FUNC_ReadEnumObjectType;
		}

		return CLOS_SUCCESS;

		/*FUNC_SendResult(s->tr, FUNC_ERROR);

		LOG_Error(LOG_TO, "Block requested out of bounds: %d", nBlock);
		goto err_FUNC_ReadEnumObjectType;
		*/
	}
	LOG_Debug( LOG_TO,"SEGUIMOS POR BAJO  %d", 2 );
	snprintf(aux, 512, "nBlock=%d found= %d, to= %d ",nBlock,found,to);
	LOG_Debug(LOG_TO,"%s", aux);

	ret = IO_Seek(s->hClauer, nBlock, IO_SEEK_OBJECT);
	if ( ret != IO_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);

		LOG_Error(LOG_TO, "Error positioning device pointer : %d", ret);
		goto err_FUNC_ReadEnumObjectType;
	}


	for ( b = nBlock ; ! found && (b < to ) ; b++ ) {


		ret = IO_Read(s->hClauer, (unsigned char *) &ob);

		if ( ret != IO_SUCCESS ) {
			FUNC_SendResult(s->tr, FUNC_ERROR);
			snprintf(aux, 512, "Error reading block %ld : %d", b, ret);
			LOG_MsgError(LOG_TO, aux);
			goto err_FUNC_ReadEnumObjectType;
		}
		LOG_Debug(LOG_TO,"Found type: 0x%02x", ob.type);
		if ( ob.type == type && BLOCK_OBJECT_Get_Mode(&ob) != MODE_EMPTY ) {

			LOG_Debug(LOG_TO, "            MODE = %d", ob.mode);
			/*
			if the block is ciphered and we're in a non authenticated
			session, we haven't found it yet
			*/

			if ( ( s->type == SESS_RDWR ) ||
				( BLOCK_OBJECT_Get_Mode(&ob) != MODE_CIPHERED ) ){
					found = 1;
					//b++;

					LOG_Debug(LOG_TO,"FOUND BLOCK = %d",b);
			}
		}
	}

	if ( found ) {

		--b;


		LOG_Debug(LOG_TO,"Found block %d",b);

		if ( BLOCK_OBJECT_Get_Mode(&ob) == MODE_CIPHERED ) {

			ret = BLOCK_OBJECT_Decipher(&ob, s->pwd, s->ib);
			if ( ret != CLOS_SUCCESS ) {
				FUNC_SendResult(s->tr, FUNC_ERROR);

#ifdef DEBUG
				snprintf(aux, 512, "Error deciphering block %d : %d", b, ret);
				LOG_MsgError(LOG_TO, aux);
#endif
				goto err_FUNC_ReadEnumObjectType;
			}

		}


		ret = FUNC_SendResult(s->tr, FUNC_OK);
		if ( ret != CLOS_SUCCESS ) {

			LOG_Error(LOG_TO, "Error sending OK result to the client: %d", ret);
			goto err_FUNC_ReadEnumObjectType;
		}

		ret = TRANS_Send(s->tr, (void *) &b, 4);

		LOG_Debug(LOG_TO,"Devuelto el bloque %d",b);
		if ( ret != CLOS_SUCCESS ) {

			LOG_Error(LOG_TO, "Error sending block number : %d", ret);
			goto err_FUNC_ReadEnumObjectType;
		}

		ret = TRANS_Send(s->tr, (void *) &ob, 10240);
		if ( ret != CLOS_SUCCESS ) {

			LOG_Error(LOG_TO, "Error sending block number : %d", ret);
			goto err_FUNC_ReadEnumObjectType;
		}

	} else {

		b = -1;

		ret = FUNC_SendResult(s->tr, FUNC_OK);
		if ( ret != CLOS_SUCCESS ) {
			LOG_Error(LOG_TO, "Error sending OK result to the client: %d", ret);
			goto err_FUNC_ReadEnumObjectType;
		}

		ret = TRANS_Send(s->tr, (void *) &b, 4);
		if ( ret != CLOS_SUCCESS ) {

			LOG_Error(LOG_TO, "Error sending block number : %d", ret);
			goto err_FUNC_ReadEnumObjectType;
		}

	}

	ret = BLOCK_OBJECT_Delete ( &ob );
	if ( ret != CLOS_SUCCESS ) {

		LOG_Error(LOG_TO, "Error destroying object block : %d", ret);
		goto err_FUNC_ReadEnumObjectType;
	}



	LOG_EndFunc(LOG_TO, CLOS_SUCCESS);


	return CLOS_SUCCESS;

err_FUNC_ReadEnumObjectType:

	BLOCK_OBJECT_Delete ( &ob );
	LOG_EndFunc(LOG_TO, ret);
	return ret;
}




/*
***************************************************************************
* Function 9. Read all blocks of given kind.
* type  Size               Name           Sematics
*  <-    1                 func_id        Function. Always take 9 value.
*  ->    1                 error          Byte error.
*  ->    4                 total_blocks   Number of blocks.
*  ->    4                 pos_block_1    First block position.
*  ->    4                 pos_block_2    Second block position.
*  ->    4                 pos_block_3    Third block position.
*  ...
*  ->    4                 pos_block_n    N block position.
*  -> 10240*total_blocks   blocks         The blocks themselves.
***************************************************************************
*/

int FUNC_ReadAllTypeObjects ( session_t *s )
{
	unsigned char type;
	block_object_t *blocks = NULL;
	long b, to, bCount, *bNumber = NULL;
	int ret;
	char aux[512];

	LOG_BeginFunc(LOG_TO);

	/* Receive the parameters */

	//
	LOG_Msg(LOG_TO,"Receiving type");
	ret = TRANS_Receive(s->tr, &type, 1);
	if ( ret != CLOS_SUCCESS ) {

		LOG_Error(LOG_TO, "Error receiving type parameter : %d", ret);
		goto err_FUNC_ReadAllTypeObjects;
	}	
	//
	LOG_Debug(LOG_TO,"type received %d",type);
	ret = IO_Seek(s->hClauer, 0, IO_SEEK_OBJECT);
	if ( ret != IO_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);

		LOG_Error(LOG_TO, "Error positioning device pointer : %d", ret);
		goto err_FUNC_ReadAllTypeObjects;
	}
	//
	LOG_Msg(LOG_TO,"Pasado seek");
	to = ( s->ib->cb == -1 ) ? s->ib->totalBlocks : s->ib->cb;

	blocks = ( block_object_t *) malloc ( sizeof(block_object_t) * 100);
	if ( ! blocks ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);

		LOG_MsgError(LOG_TO, "Out of memory");
		goto err_FUNC_ReadAllTypeObjects;
	}

	bNumber = ( long * ) malloc ( sizeof(long) * 100);
	if ( ! bNumber ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);

		LOG_MsgError(LOG_TO, "Out of memory");
		goto err_FUNC_ReadAllTypeObjects;
	}

	bCount = 0;
	//
	LOG_Msg(LOG_TO,"Llegamos a for");
	for ( b = 0 ; b < to ; b++ ) {
		ret = IO_Read(s->hClauer, (unsigned char *) (blocks + bCount));
		if ( ret != IO_SUCCESS ) {
			FUNC_SendResult(s->tr, FUNC_ERROR);

			snprintf(aux, 512,"Error reading block %d : %d", b, ret);
			LOG_MsgError(LOG_TO, aux);

			goto err_FUNC_ReadAllTypeObjects;
		}

		bNumber[bCount] = b;	

		if ( (blocks+bCount)->type == type ) {

			if ( ( s->type == SESS_RDWR ) ||
				( BLOCK_OBJECT_Get_Mode(blocks+bCount) != MODE_CIPHERED ) ) {

					bCount++;
					if ( ((bCount+1) % 100) == 0 ) {

						blocks = ( block_object_t *) realloc ( blocks, sizeof(block_object_t) * (bCount + 101));

						if ( ! blocks ) {
							FUNC_SendResult(s->tr, FUNC_ERROR);

							LOG_Error(LOG_TO, "Out of memory: %d", ret);
							goto err_FUNC_ReadAllTypeObjects;
						}

						bNumber = ( long * ) realloc ( blocks, sizeof(long) * (bCount + 101));
						if ( ! bNumber ) {
							FUNC_SendResult(s->tr, FUNC_ERROR);

							LOG_Error(LOG_TO, "Out of memory: %d", ret);
							goto err_FUNC_ReadAllTypeObjects;
						}

					}
			}
		}
	}

	//
	LOG_Msg(LOG_TO,"Salimos del for");
	/* now decipher the blocks if necessary */

	if ( s->type == SESS_RDWR ) {
		//
		LOG_Msg(LOG_TO,"Entramos en SESS_RDWR");
		for ( b = 0 ; b < bCount ; b++ ) {
			//
			LOG_Debug(LOG_TO,"Entramos en segundo for b=%d",b);
			if ( BLOCK_OBJECT_Get_Mode(blocks+b) == MODE_CIPHERED ) {
				ret = BLOCK_OBJECT_Decipher((block_object_t *) (blocks+b), s->pwd, s->ib);
				if ( ret != CLOS_SUCCESS ) {

					LOG_Msg(LOG_TO,"Enviamos error ");
					FUNC_SendResult(s->tr, FUNC_ERROR);

#ifdef DEBUG
					snprintf(aux, 512, "Error deciphering block %ld : %d", bNumber[b], ret);
					LOG_MsgError(LOG_TO, aux);
#endif
					goto err_FUNC_ReadAllTypeObjects;
				}
			}
			//
			LOG_Debug(LOG_TO,"Salimos segundo for b=%d",b);
		}
		//
		LOG_Msg( 1,"Saliendo de for" );
	}

	//
	LOG_Msg(LOG_TO,"Enviamos el resultado");
	ret = FUNC_SendResult(s->tr, FUNC_OK);
	if ( ret != CLOS_SUCCESS ) {

		LOG_MsgError(LOG_TO, "Error sending OK to the client");
		goto err_FUNC_ReadAllTypeObjects;
	}

	//
	LOG_Debug(LOG_TO,"Sending num blocks = %d",bCount);    
	ret = TRANS_Send(s->tr, &bCount, 4);
	if ( ret != CLOS_SUCCESS ) {

		LOG_MsgError(LOG_TO, "Error sending block count");
		goto err_FUNC_ReadAllTypeObjects;
	}

	ret = TRANS_Send(s->tr, bNumber, sizeof(long)*bCount);
	if ( ret != CLOS_SUCCESS ) {

		LOG_MsgError(LOG_TO, "Error sending block numbers");
		goto err_FUNC_ReadAllTypeObjects;
	}

	free(bNumber);
	bNumber = NULL;

	//
	LOG_Msg(LOG_TO,"Sending blocks");
	ret = TRANS_Send(s->tr, blocks, sizeof(block_object_t)*bCount);
	if ( ret != CLOS_SUCCESS ) {

		LOG_MsgError(LOG_TO, "Error sending blocks ");
		goto err_FUNC_ReadAllTypeObjects;
	}

	ret = SMEM_Destroy((void *) blocks, sizeof(block_object_t)*bCount);
	if ( ret != CLOS_SUCCESS ) {

		LOG_MsgError(LOG_TO, "Error destroying memory");
		goto err_FUNC_ReadAllTypeObjects;
	}

	free(blocks);
	blocks = NULL;

	LOG_EndFunc(LOG_TO, CLOS_SUCCESS);

	return CLOS_SUCCESS;

err_FUNC_ReadAllTypeObjects:

	//
	LOG_MsgError(LOG_TO,"Error en ReadAllTypeObjects");
	if ( bNumber ) 
		free(bNumber);

	if ( blocks ) {

		SMEM_Destroy((void *) blocks, sizeof(block_object_t)*bCount);
		blocks = NULL;
	}

	return ret;
}





/*
***************************************************************************
* Function 10. Read all Occuped Blocks.
* type  Size               Name           Sematics
*  <-    1                 func_id        Function. Always take 10 value.
*  ->    1                 error          Byte error.
*  ->    4                 total_blocks   Number of blocks.
*  ->    4                 pos_block_1    First block position.
*  ->    4                 pos_block_2    Second block position.
*  ->    4                 pos_block_3    Third block position.
*  ...
*  ->    4                 pos_block_n    N block position.
*  -> 10240*total_blocks   blocks         The blocks themselves.
***************************************************************************
*/

int FUNC_ReadAllOccupedBlocks ( session_t *s )
{
	block_object_t *blocks = NULL;
	int b, to, bCount, *bNumber = NULL;
	int ret;
	char aux[512];
	//    char idAux[21];

	LOG_BeginFunc(LOG_TO);

	LOG_Msg(LOG_TO,"Paso por FUNC_READALL ");

	ret = IO_Seek(s->hClauer, 0, IO_SEEK_OBJECT);
	if ( ret != IO_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);		
		LOG_Error(LOG_TO, "Error positioning device pointer : %d", ret);
		goto err_FUNC_ReadAllOccupedBlocks;
	}

	to = ( s->ib->cb == -1 ) ? s->ib->totalBlocks : s->ib->cb;

	blocks = ( block_object_t *) malloc ( sizeof(block_object_t) * 100);
	if ( ! blocks ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);
		LOG_MsgError(LOG_TO, "Out of memory");
		goto err_FUNC_ReadAllOccupedBlocks;
	}

	bNumber = ( int * ) malloc ( sizeof(int) * 100);
	if ( ! bNumber ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);		
		LOG_MsgError(LOG_TO, "Out of memory");
		goto err_FUNC_ReadAllOccupedBlocks;
	}

	bCount = 0;
	for ( b = 0 ; b < to ; b++ ) {

		ret = IO_Read(s->hClauer, (unsigned char *) (blocks + bCount));
		if ( ret != IO_SUCCESS ) {
			FUNC_SendResult(s->tr, FUNC_ERROR);
			snprintf((char *) aux, 512, "Error reading block %d : %d", b, ret);
			LOG_MsgError(LOG_TO, aux);
			goto err_FUNC_ReadAllOccupedBlocks;
		}

		snprintf((char *) aux, 512, "bNumber[%d] = %d",bCount,b);
		LOG_Debug(LOG_TO,"%s", aux);
		bNumber[bCount] = b;	

		if ( (( s->type == SESS_RDWR ) && ( BLOCK_OBJECT_Get_Mode(blocks + bCount) != MODE_EMPTY )) ||
			(( s->type == SESS_RDONLY ) && ( BLOCK_OBJECT_Get_Mode(blocks + bCount) == MODE_CLEAR )) ) 
		{
			if ( BLOCK_OBJECT_Get_Mode(blocks + bCount) == MODE_CIPHERED ) {
				ret = BLOCK_OBJECT_Decipher((block_object_t *) (blocks+bCount), s->pwd, s->ib);
				if ( ret != CLOS_SUCCESS ) {
					FUNC_SendResult(s->tr, FUNC_ERROR);
					snprintf(aux, 512, "Error deciphering block %ld : %d type: %d", bNumber[b], ret,(blocks+b)->type);
					LOG_Error(LOG_TO, "%s", aux);
					goto err_FUNC_ReadAllOccupedBlocks;
				}
			}

			bCount++;
			if ( ((bCount+1) % 100) == 0 ) {

				blocks = ( block_object_t *) realloc ( (void *) blocks, sizeof(block_object_t) * (bCount + 101));
				if ( ! blocks ) {
					FUNC_SendResult(s->tr, FUNC_ERROR);  
					LOG_Error(LOG_TO, "Out of memory: %d", ret);
					goto err_FUNC_ReadAllOccupedBlocks;
				}

				bNumber = ( int * ) realloc ( (void *) bNumber, sizeof(int) * (bCount + 101));
				if ( ! bNumber ) {
					FUNC_SendResult(s->tr, FUNC_ERROR);

					LOG_Error(LOG_TO, "Out of memory: %d", ret);
					goto err_FUNC_ReadAllOccupedBlocks;
				}
			}

		}
	}
	ret = FUNC_SendResult(s->tr, FUNC_OK);
	if ( ret != CLOS_SUCCESS ) {
		LOG_MsgError(LOG_TO, "Error sending OK to the client");
		goto err_FUNC_ReadAllOccupedBlocks;
	}


	LOG_Debug(LOG_TO,"bCount=%d",bCount);
	ret = TRANS_Send(s->tr, &bCount, 4);
	if ( ret != CLOS_SUCCESS ) {	
		LOG_MsgError(LOG_TO, "Error sending block count");
		goto err_FUNC_ReadAllOccupedBlocks;
	}


	ret = TRANS_Send(s->tr, bNumber, sizeof(int)*bCount);
	if ( ret != CLOS_SUCCESS ) {	
		LOG_MsgError(LOG_TO, "Error sending block numbers");
		goto err_FUNC_ReadAllOccupedBlocks;
	}

	free(bNumber);
	bNumber = NULL;

	ret = TRANS_Send(s->tr, blocks, sizeof(block_object_t)*bCount);
	if ( ret != CLOS_SUCCESS ) {

		LOG_MsgError(LOG_TO, "Error sending blocks");
		goto err_FUNC_ReadAllOccupedBlocks;
	}

	ret = SMEM_Destroy((void *) blocks, sizeof(block_object_t)*bCount);
	if ( ret != CLOS_SUCCESS ) {

		LOG_MsgError(LOG_TO, "Error destroying memeory");
		goto err_FUNC_ReadAllOccupedBlocks;
	}

	free(blocks);
	blocks = NULL;

	LOG_EndFunc(LOG_TO, CLOS_SUCCESS);

	return CLOS_SUCCESS;

err_FUNC_ReadAllOccupedBlocks:


	if ( bNumber ) 
		free(bNumber);

	if ( blocks ) {

		SMEM_Destroy((void *) blocks, sizeof(block_object_t)*bCount);
		blocks = NULL;
	}

	return ret;

}



/*
*************************************************************************
* Function 11. Write Object Zone Block. 
* type  Size      Name         Sematics
*  <-    1        func_id      Function. Always take 11 value.
*  <-    4        block_num    Position where the block must be writen.
*  <-  10240      block        The block to be inserted.
*  ->    1        error        Byte Error.            
*************************************************************************
*/

int FUNC_WriteObjectZoneBlock ( session_t *s )
{
	int nBlock;
	block_object_t block;
	int ret;
	char aux[512];



	ret = TRANS_Receive(s->tr, &nBlock, 4);
	if ( ret != CLOS_SUCCESS ) {

		LOG_Error(LOG_TO, "Error receiving block number : %d", ret);
		goto err_FUNC_WriteObjectZoneBlock;
	}

	ret = TRANS_Receive(s->tr, &block, sizeof(block_object_t));
	if ( ret != CLOS_SUCCESS ) {

		LOG_Error(LOG_TO, "Error receiving block : %d", ret);
		goto err_FUNC_WriteObjectZoneBlock;
	}

	if ( s->type == SESS_RDONLY ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);

		LOG_MsgError(LOG_TO, "Error. Session is readonly");
		goto err_FUNC_WriteObjectZoneBlock;
	}

	if ( nBlock >= (int)s->ib->totalBlocks ) {
		ret = FUNC_SendResult(s->tr, FUNC_ERROR);
		if ( ret != CLOS_SUCCESS ) {

			LOG_Error(LOG_TO, "Error sending error to the client : %d", ret);
			goto err_FUNC_WriteObjectZoneBlock;
		}


		snprintf(aux, 512, "Block number %d out of bounds : %d", nBlock, ret);
		LOG_Error(LOG_TO, "%s", aux);

		goto err_FUNC_WriteObjectZoneBlock;
	}


	/* Just in case IB has changed and cache does not realize it */
	ret = IO_ReadInfoBlock ( s->hClauer, s->ib );
	if ( ret != IO_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);
		LOG_Error(LOG_TO, "Error reading information block: %d", ret);
		goto err_FUNC_WriteObjectZoneBlock;
	}


	/* Now cipher the block if necessary */

	if ( BLOCK_OBJECT_Get_Mode ( &block ) == MODE_CIPHERED ) {

		ret = BLOCK_OBJECT_Cipher(&block, s->pwd, s->ib);
		if ( ret != CLOS_SUCCESS ) {
			FUNC_SendResult(s->tr, FUNC_ERROR);

			LOG_Error(LOG_TO, "Impossible to cipher block : %d", ret);
			goto err_FUNC_WriteObjectZoneBlock;
		}

	}


	ret = IO_Seek(s->hClauer, nBlock, IO_SEEK_OBJECT);
	if ( ret != IO_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);

		LOG_MsgError(LOG_TO, "Error positioning device pointer");
		goto err_FUNC_WriteObjectZoneBlock;
	}

	ret = IO_Write(s->hClauer, (unsigned char *) &block);
	if ( ret != IO_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);

		LOG_MsgError(LOG_TO, "Error writing block");
		goto err_FUNC_WriteObjectZoneBlock;
	}

	// TODO: Really need to be updated we are wiriting, not appending a block
	/*
	Update current block
	*/

	if ( s->ib->cb != -1 ) {

		if ( nBlock == (s->ib->totalBlocks-1) )
			s->ib->cb = -1;
		else if ( nBlock > s->ib->cb )
			s->ib->cb = nBlock+1; 

		//
#ifdef DEBUG
		LOG_Debug(LOG_TO,"New current block: %d",s->ib->cb);
#endif
		ret = IO_WriteInfoBlock ( s->hClauer, s->ib );
		if ( ret != CLOS_SUCCESS ) {
			FUNC_SendResult(s->tr, FUNC_ERROR);

#ifdef DEBUG
			LOG_Error(LOG_TO, "Error updating current block : %d", ret);
#endif
			goto err_FUNC_WriteObjectZoneBlock;
		}

	}


	ret = BLOCK_OBJECT_Delete (&block);
	if ( ret != CLOS_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);

		LOG_Error(LOG_TO, "Error destroying block object : %d", ret);
		goto err_FUNC_WriteObjectZoneBlock;
	}

	ret = FUNC_SendResult(s->tr, FUNC_OK);
	if ( ret != CLOS_SUCCESS ) {

		LOG_Error(LOG_TO, "Error sending OK to client : %d", ret);
		goto err_FUNC_WriteObjectZoneBlock;
	}

	return CLOS_SUCCESS;

err_FUNC_WriteObjectZoneBlock:

	BLOCK_OBJECT_Delete(&block);


	LOG_EndFunc(LOG_TO, ret);

	return ret;

}




/*
***************************************************************************
* Function 12. Insert Object Zone Block. 
* type  Size      Name         Sematics
*  <-    1        func_id      Function. Always take 12 value.
*  <-  10240      block        The block to be inserted.
*  ->    1        error        Byte Error.
*  ->    4        block_num    Position where the block has been inserted.                    
***************************************************************************
*/

int FUNC_InsertObjectZoneBlock ( session_t *s )
{
	block_object_t ob,obAux;
	long b;
	int ret, foundEmpty = 0;
	char aux[512];

	LOG_BeginFunc(LOG_TO);

	ret = TRANS_Receive(s->tr, &ob, BLOCK_SIZE);
	if ( ret != CLOS_SUCCESS ) {

		LOG_Error(LOG_TO, "Error receiving block to insert : %d", ret);
		goto err_FUNC_InsertObjectZoneBlock;
	}

	if ( s->type == SESS_RDONLY ) {

		LOG_MsgError(LOG_TO, "Session is readonly");
		FUNC_SendResult(s->tr, FUNC_ERROR);
		goto err_FUNC_InsertObjectZoneBlock;
	}		


	/* Just in case IB has changed and cache does not realize it */
	ret = IO_ReadInfoBlock ( s->hClauer, s->ib );
	if ( ret != IO_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);
		LOG_Error(LOG_TO, "Error reading information block: %d", ret);
		goto err_FUNC_InsertObjectZoneBlock;
	}


	LOG_Debug(LOG_TO,"Current block is %d",s->ib->cb);
	if ( s->ib->cb == -1 ) {
		ret = IO_Seek(s->hClauer, 0, IO_SEEK_OBJECT);
		if ( ret != IO_SUCCESS ) {

			LOG_Error(LOG_TO, "Error positioning device pointer : %d", ret);
			FUNC_SendResult(s->tr, FUNC_ERROR);
			goto err_FUNC_InsertObjectZoneBlock;
		}

		for ( b = 0 ; ! foundEmpty && (b < (int)s->ib->totalBlocks) ; b++ ) {

			ret = IO_Read(s->hClauer, (void *) &obAux);
			if ( ret != IO_SUCCESS ) {


				snprintf(aux, 512,"Error reading block %d : %d", b, ret);
				LOG_Error(LOG_TO, "%s", aux);

				FUNC_SendResult(s->tr, FUNC_ERROR);
			}

			if ( BLOCK_OBJECT_Get_Mode(&obAux) == MODE_EMPTY )
				foundEmpty = 1;
		} 

		if ( ! foundEmpty ) {

			LOG_Msg(LOG_TO, "Device is full");
			FUNC_SendResult(s->tr, FUNC_ERROR);
			goto err_FUNC_InsertObjectZoneBlock;	
		}

		--b;		

	} else 
		b = s->ib->cb;

	LOG_Debug(LOG_TO, "NUMERO DE BLOQUE DE INSERCION: %ld", b);
	if ( BLOCK_OBJECT_Get_Mode(&ob) == MODE_CIPHERED ) {

		ret = BLOCK_OBJECT_Cipher(&ob, s->pwd, s->ib);
		if ( ret != CLOS_SUCCESS ) {

			LOG_MsgError(LOG_TO, "Error ciphering block");
			FUNC_SendResult(s->tr, FUNC_ERROR);
			goto err_FUNC_InsertObjectZoneBlock;
		}

	}


	LOG_Debug(LOG_TO,"Writing in b= %d",b);
	ret = IO_Seek(s->hClauer, b, IO_SEEK_OBJECT);
	if ( ret != IO_SUCCESS ) {

		LOG_MsgError(LOG_TO, "Error positioning device pointer");
		FUNC_SendResult(s->tr, FUNC_ERROR);
		goto err_FUNC_InsertObjectZoneBlock;
	}	

	ret = IO_Write(s->hClauer, (void *) &ob);
	if ( ret != IO_SUCCESS ) {

		snprintf(aux, 512, "Error writing block %d : %d", b, ret);
		LOG_Error(LOG_TO, "%s", aux);

		FUNC_SendResult(s->tr, FUNC_ERROR);
		goto err_FUNC_InsertObjectZoneBlock;
	}


	/*
	Update current block
	*/

	if ( s->ib->cb != -1 ) {
		if ( s->ib->cb +1 == (s->ib->totalBlocks-1) )
			s->ib->cb = -1;
		else
			s->ib->cb +=1; 

		LOG_Debug(LOG_TO,"New current block: %d",s->ib->cb);
		ret = IO_WriteInfoBlock ( s->hClauer, s->ib );

		LOG_Debug(LOG_TO,"New writen current block: %d",s->ib->cb);
		if ( ret != IO_SUCCESS ) {
			FUNC_SendResult(s->tr, FUNC_ERROR);

			LOG_Error(LOG_TO, "Error updating current block : %d", ret);
			goto err_FUNC_InsertObjectZoneBlock;
		}
	}


	ret = BLOCK_OBJECT_Delete (&ob);
	if ( ret != CLOS_SUCCESS ) {

		LOG_Error(LOG_TO, "Error destroying block object : %d", ret);
		FUNC_SendResult(s->tr, FUNC_ERROR);
		goto err_FUNC_InsertObjectZoneBlock;
	}


	ret = FUNC_SendResult(s->tr, FUNC_OK);
	if ( ret != CLOS_SUCCESS ) {

		LOG_Error(LOG_TO, "Error sending OK to the client : %d", ret);
		goto err_FUNC_InsertObjectZoneBlock;
	}

	ret = TRANS_Send(s->tr, &b, 4);
	if ( ret != CLOS_SUCCESS ) {

		LOG_Error(LOG_TO, "Error sending block number to the client : %d", ret);
		FUNC_SendResult(s->tr, FUNC_ERROR);
		goto err_FUNC_InsertObjectZoneBlock;
	}


	LOG_EndFunc(LOG_TO, CLOS_SUCCESS);
	return CLOS_SUCCESS;

err_FUNC_InsertObjectZoneBlock:

	BLOCK_OBJECT_Delete(&ob);


	LOG_EndFunc(LOG_TO, ret);

	return ret;
}




/*
***************************************************************************
* Function 13. Erase Object Zone Block. 
* type  Size      Name         Sematics
*  <-    1        func_id      Function. Always take 13 value.
*  <-    1        block_num    Position of the block to be deleted.
*  ->    1        error        Byte Error.
***************************************************************************
*/

int FUNC_EraseObjectBlock ( session_t *s )
{
	block_object_t ob;
	int nBlock;
	int ret, empty=1;
	char aux[512];

	LOG_BeginFunc(LOG_TO);

	ret = TRANS_Receive(s->tr, &nBlock, 4);
	if ( ret != CLOS_SUCCESS ) {	
		LOG_Error(LOG_TO, "Error receiving block number : %d", ret);
		goto err_FUNC_EraseObjectBlock;
	}


	/* Just in case IB has changed and cache does not realize it */
	ret = IO_ReadInfoBlock ( s->hClauer, s->ib );
	if ( ret != IO_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);
		LOG_Error(LOG_TO, "Error reading information block: %d", ret);
		goto err_FUNC_EraseObjectBlock;
	}

	if ( nBlock >= (int)s->ib->totalBlocks ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);

		LOG_Error(LOG_TO, "Block %ld out of bounds", nBlock);
		goto err_FUNC_EraseObjectBlock;
	}

	ret = IO_Seek(s->hClauer, nBlock, IO_SEEK_OBJECT);
	if ( ret != IO_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);

		LOG_Error(LOG_TO, "Positioning device pointer : %d", ret);
		goto err_FUNC_EraseObjectBlock;
	}

	ret = BLOCK_OBJECT_New_Empty(&ob);
	if ( ret != CLOS_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);

		LOG_Error(LOG_TO, "Creating an empty block : %d", ret);
		goto err_FUNC_EraseObjectBlock;
	}

	ret = IO_Write(s->hClauer, (unsigned char *) &ob);
	if ( ret != IO_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);

		LOG_Error(LOG_TO, "Error writing empty block : %d", ret);
		return CLOS_SUCCESS;
	}


	// Here we must check the current block pointer, perhaps a repositionamente is 
	// needed.

	snprintf((char *) aux, 512, "nBlock= %d, totalBlocks= %d, s->ib->cb= %d",nBlock,s->ib->totalBlocks,s->ib->cb);
	LOG_Debug(LOG_TO,"%s", aux);
	if (nBlock == (s->ib->totalBlocks-1) || (nBlock == s->ib->cb-1) ){
		while (empty){
			// Pointer reposition
			ret = IO_Seek(s->hClauer, nBlock, IO_SEEK_OBJECT);
			if ( ret != IO_SUCCESS ) {
				FUNC_SendResult(s->tr, FUNC_ERROR);

				LOG_Error(LOG_TO, "Positioning device pointer : %d", ret);
				goto err_FUNC_EraseObjectBlock;
			}

			// Block reading
			ret = IO_Read(s->hClauer, (unsigned char *) &ob);
			if ( ret != IO_SUCCESS ) {
				FUNC_SendResult(s->tr, FUNC_ERROR);

				LOG_MsgError(LOG_TO, "Could not read block.");
				goto err_FUNC_EraseObjectBlock;
			}

			if ( ob.mode == 0 && s->ib->cb !=0 )
				s->ib->cb= nBlock;
			if (nBlock == 0 || ob.mode != 0 )
				empty=0;
			nBlock--;

			LOG_Debug(LOG_TO,"Current block pointer reallocated to: %d",s->ib->cb);
		}
	}

	if ( IO_WriteInfoBlock(s->hClauer, s->ib) != IO_SUCCESS ) {
		LOG_MsgError(LOG_TO, "Information block update impossible");
		LOG_MsgError(LOG_TO, "We don't send error to client");
	}

	// End reposition of the cb pointer

	ret = FUNC_SendResult(s->tr, FUNC_OK);
	if ( ret != CLOS_SUCCESS ) {	
		LOG_Error(LOG_TO, "Error sending OK to the client : %d", ret);
		goto err_FUNC_EraseObjectBlock;
	}


	LOG_EndFunc(LOG_TO, CLOS_SUCCESS);

	return CLOS_SUCCESS;

err_FUNC_EraseObjectBlock:


	LOG_EndFunc(LOG_TO, ret);

	return ret;
}







/* ************************************************************************************
* Function 14. Get hardware Id 
* type  Size     Name         Sematics
*  <-    1       func_id      Function. Always take 1 value.
*  ->    1       error        Byte Error
*  ->   16       hw_dev_id    Hardware Device identifier
**************************************************************************************
*/


int FUNC_GetHardwareId  ( session_t *s )
{
	int ret;

	// By the time, we return no error and 
	// the hardware clauer Id printed by StartSession 
	// function

	ret = FUNC_SendResult(s->tr, FUNC_OK);
	if ( ret != CLOS_SUCCESS ) {

		LOG_Error(LOG_TO, "Error sending OK to the client : %d", ret);
		goto err_FUNC_GetHardwareId;
	}


	ret = TRANS_Send(s->tr, s->ib->hwId, HW_ID_LEN);
	if ( ret != CLOS_SUCCESS ) {

		LOG_Error(LOG_TO, "Error sending hardware id to the client : %d", ret);
		FUNC_SendResult(s->tr, FUNC_ERROR);
		goto err_FUNC_GetHardwareId;
	}


	/* 
	TODO: here we must return the system hardware id, by the time, 
	we will return the same as the one in the info block 
	( all zero )
	*/ 
	ret = TRANS_Send(s->tr, s->ib->hwId, HW_ID_LEN);
	if ( ret != CLOS_SUCCESS ) {

		LOG_Error(LOG_TO, "Error sending hardware id to the client : %d", ret);
		FUNC_SendResult(s->tr, FUNC_ERROR);
		goto err_FUNC_GetHardwareId;
	}

	//    LOG_Debug(LOG_TO, "Devolviendo SUCCESS\n");
	return CLOS_SUCCESS;

err_FUNC_GetHardwareId:

	LOG_EndFunc(LOG_TO, ret);

	return ret;
}


/* ************************************************************************************
* Function 16    Returns to the client the current's clos version
* type  Size     Name         Sematics
*  <-    1       func_id      Function. Always take 16 value.
*  ->    4       size      size.
*  ->    size       version      Version number in xyz format (i.e.  302 would be  3.0.2).
**************************************************************************************
*/
int FUNC_GetClosVersion ( session_t *s ){
    
    char  buff[10], * res, err=0;
	char * v0= "0.0.0";
    int l= 0;
    
	if (child_info.sw_version == NULL){
		child_info.sw_version= (char *) malloc(strlen(v0)+1);
		strncpy(child_info.sw_version, v0, strlen(v0)+1);
	}

    LOG_BeginFunc(LOG_TO);

	//Prepare the string to be returned
    snprintf(buff, 10, "%d",child_info.sw_type);
    l= strlen(buff) + strlen(child_info.sw_version); 
	res= malloc(sizeof(char) * (l+2));
    strcpy(res,buff);
    strcat(res,"#");
    strcat(res,child_info.sw_version); 
    l++; 


	if ( TRANS_Send(s->tr, (void *) &l, 4) != CLOS_SUCCESS ) {
    	return ERR_CLOS;
	}

	LOG_Debug(LOG_TO,"sending version=%s", res);
	if ( TRANS_Send(s->tr, (void *) res, l) != CLOS_SUCCESS ){ 
    	return ERR_CLOS;
	}

	if ( TRANS_Send(s->tr, (void *) &err, 1) != CLOS_SUCCESS ){ 
    	return ERR_CLOS;
	}

	LOG_EndFunc(LOG_TO, CLOS_SUCCESS);
    return CLOS_SUCCESS;
}


/* ************************************************************************************
* Function 17    Windows only, given a PHISYCALDRIVE offers the logic unit. 
* type  Size     Name         Sematics
*  <-    1       func_id      Function. Always take 17 value.
*  <-    4       size         size of the PHISYCALDRIVE string.
*  <-    size    string       the physical drive string itself.
*  ->    1       error	      the error byte. 
*  ->    4       size         the size of the logical unit string. 
*  ->    size    logical_unit The logical unit string itself. 
**************************************************************************************
*/
int FUNC_GetLogicalUnitFromPhysicalDrive ( session_t *s ){
#ifdef WIN32  
    int lenPhys=0, lenLog=3, found=0, ret, result;
	char * strPhys, strLog[3];
	clauer_handle_t hClauer;

    LOG_BeginFunc(LOG_TO);

	ret = TRANS_Receive(s->tr, &lenPhys, 4);
	if ( ret != CLOS_SUCCESS ) {	
		LOG_Error(1, "Error receiving physical drive len number : %d", ret);
		return ERR_CLOS;
	}

	LOG_Debug(1, "GOT Physical drive string length as: %d", lenPhys);
	strPhys= (char*) malloc(lenPhys+1);
	if (!strPhys){
		return ERR_CLOS;
	}

	ret = TRANS_Receive(s->tr, strPhys, lenPhys);
	if ( ret != CLOS_SUCCESS ) {	
		LOG_Error(1, "Error receiving physicaldrive string : %d", ret);
		return ERR_CLOS;
	}

	strPhys[lenPhys]= 0;

	LOG_Debug (1, "Received Physical drive= %s", strPhys);
 
	// Get the logical unit. 
	LOG_Debug(1,"Abiendo device = %s", strPhys);

	result = IO_Open( strPhys, &hClauer, IO_RDWR, -1 );
	if ( result != IO_SUCCESS ) {
		return ERR_FORMAT_OPEN_DEVICE;
	}

	result= IO_GetDriveLetter( hClauer, strLog, &found );
	if ( result != IO_SUCCESS ){
		IO_Close( hClauer );
		FUNC_SendResult(s->tr, FUNC_OK);
		return ERR_CLOS;
	}

	if ( found==0 ){
		IO_Close( hClauer );
		FUNC_SendResult(s->tr, FUNC_OK);
		return ERR_CLOS;
	}
	//And send it if there is no error.

	FUNC_SendResult(s->tr, FUNC_OK);
	if ( TRANS_Send(s->tr, (void *) &lenLog, 4) != CLOS_SUCCESS ) {
    	return ERR_CLOS;
	}

	if ( TRANS_Send(s->tr, (void *) &strLog, lenLog) != CLOS_SUCCESS ) {
    	return ERR_CLOS;
	}

	LOG_EndFunc(1, CLOS_SUCCESS);
#endif
    return CLOS_SUCCESS;
}



/*
***************************************************************************
* Function 20. Close Session. 
* type  Size      Name         Sematics
*  <-    1        func_id      Function. Always take 20 value.
*  ->    1        error        Byte Error.
***************************************************************************
*/

int FUNC_CloseSession ( session_t *s )
{
	int ret;


	LOG_BeginFunc(LOG_TO);

	if ( s->pwd ) {
		ret = SMEM_Free(s->pwd, (unsigned long) (strlen(s->pwd)+1));
		if ( ret != CLOS_SUCCESS ) {
			FUNC_SendResult(s->tr, FUNC_ERROR);
			goto err_FUNC_CloseSession;
		}
	}

	LOG_Msg(LOG_TO,"Antes del primer FREE");


	free(s->ib);
	s->ib= NULL;

	LOG_Msg(LOG_TO,"Despues del primer FREE");

	ret = IO_Close(s->hClauer );
	if ( ret != IO_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);
		goto err_FUNC_CloseSession;
	}

	ret = FUNC_SendResult(s->tr, FUNC_OK);
	if ( ret != CLOS_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);
		goto err_FUNC_CloseSession;
	}

	LOG_Msg(LOG_TO,"Antes del TRANS_CLOSE");
	
	TRANS_Close_Ex(s->tr);
	
	LOG_Msg(LOG_TO,"Despues del TRANS_CLOSE");

	
	LOG_EndFunc(LOG_TO, CLOS_SUCCESS);

	return CLOS_SUCCESS;
	
 err_FUNC_CloseSession:


	LOG_EndFunc(LOG_TO, ret);
	return ret;
}




/* ADMIN FUNCTIONS */
/***********************************************************
* Function 128. Enumerate Devices (Usb devices Clauers and no cluers)
* type  Size    Name         Sematics
*  <-    1      func_id      Function. Always take 0 value.
*  ->    1      num_disp     Number of inserted devices.
*  ->    1      disp_num_1   First device number.
*  ->    1      disp_num_2   Second device number.
*  ->    1      disp_num_3   Third device number
*  ...
*  ->    1      disp_num_n   N device number.
*  ->    1      error byte   2 when client is not a local administrator.
***********************************************************
*/

int FUNC_EnumerateUSBs( session_t *s )
{
	int  i; 
	unsigned char nusbs;
	char * usbPath[MAX_CLAUERS];
	int mode = IO_ENUM_USB_ALL;

	
//TODO: Empezamos a implementar el protocolo por aquí 	
#ifdef _TEST //_WINDOWS  
   int nombre, valor;

   nombre=0;
   valor=0; 
   if ( TRANS_Send(s->tr, (void *) &nombre, 4) != CLOS_SUCCESS ) 
		return ERR_CLOS;

   if ( TRANS_Send(s->tr, (void *) &valor, 4) != CLOS_SUCCESS ) 
		return ERR_CLOS;
#endif
//END TODO.
 //ESTO DE MOMENTO 
  if ( ! s->admin ){
	   nusbs= 0;
	   if ( TRANS_Send(s->tr, (void *) &nusbs, 1) != CLOS_SUCCESS )
		   return ERR_CLOS;
	   FUNC_SendResult(s->tr, ERROR_NO_ADMIN);
	   return CLOS_SUCCESS;
   }
 // END ---

	nusbs= 0;

	if ( IO_EnumClauers(&nusbs, usbPath, mode) != IO_SUCCESS ) {
		LOG_Msg(LOG_TO, "Error enumerando los usbs");	
		return ERR_CLOS;
	}	


	if ( TRANS_Send(s->tr, (void *) &nusbs, 1) != CLOS_SUCCESS ) 
		return ERR_CLOS;


	for( i=0 ; i < nusbs && i < IO_MAX_DEVICES  ; i++ ){
		size_t aux;
		aux= strlen(usbPath[i]);

		LOG_Debug(LOG_TO,"Enviando  tam= %d",aux);
		if ( TRANS_Send(s->tr, (void *) &aux , 4) != CLOS_SUCCESS ) {
			LOG_Msg(LOG_TO,"[ERROR]Enviando tamaño");
			return ERR_CLOS;
		}

		LOG_Msg(LOG_TO,"OK");

		LOG_Debug(LOG_TO,"Enviando  child_info.devices = %s",usbPath[i]);
		if ( TRANS_Send(s->tr, (void *) usbPath[i] , (unsigned long) aux) != CLOS_SUCCESS ) {
			LOG_Debug(LOG_TO, "[ERROR]Enviando cadena %s", usbPath[i]);
			return ERR_CLOS;
		}	
		LOG_Msg(LOG_TO,"OK");
	}

	FUNC_SendResult(s->tr, FUNC_OK );

	TRANS_Close_Ex(s->tr);
	LOG_Msg(LOG_TO,"Saliendo OK");
	return CLOS_SUCCESS;
}


int FUNC_CreateClauer ( session_t *s ){
#ifdef WIN32 
	int lenDeviceSel=0, ret=0;
	void * aux = NULL;
	char deviceName[MAX_DEVICE_LEN+1], percent=0;

	LOG_Msg(1,"Pasando por CreateClauer");
	ret = TRANS_Receive( s->tr, &lenDeviceSel, 4 );
	if ( ret != CLOS_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);
		LOG_Error(LOG_TO, "Function 129. Receiving size of device selected. Returned value: %d", ret);
		return ERR_CLOS;
	}

	LOG_Debug(LOG_TO,"Received size of path = %d",lenDeviceSel);

	if ( lenDeviceSel > MAX_DEVICE_LEN  ){
		FUNC_SendResult(s->tr, FUNC_ERROR);
		LOG_MsgError(LOG_TO, "Function 129. Receiving device selected too long.");
		return ERR_INVALID_PARAMETER;
	}

	ret = TRANS_Receive( s->tr, deviceName, lenDeviceSel );
	if ( ret != CLOS_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);
		LOG_Error(LOG_TO, "Function 129. Receiving device selected. Returned value: %d", ret);
		return ERR_CLOS;
	}
	deviceName[lenDeviceSel]= '\0';

	LOG_Debug(LOG_TO,"Received device = %s",deviceName);

	ret = TRANS_Receive( s->tr, &percent, 1 );
	if ( ret != CLOS_SUCCESS ) {
		LOG_Error(LOG_TO, "Function 129. Receiving the perncent. Returned value: %d", ret);
		FUNC_SendResult(s->tr, FUNC_ERROR);		
		return ERR_CLOS;
	}

	LOG_Debug(LOG_TO,"Checking if user is admin = %d", s->admin);
	if ( ! s->admin ){
		FUNC_SendResult(s->tr, ERROR_NO_ADMIN);
		LOG_MsgError(1, "Te user is not admin and cannot format the device");
		return CLOS_SUCCESS;
	}

	LOG_Msg(1,"Checking if is removable.");
	if  ( IO_Is_Removable(deviceName) ){
		LOG_Msg(1,"It is removable.");
		LOG_Msg(1,"Invoking FORMAT_CreateClauer.");
		ret= FORMAT_CreateClauer(deviceName, percent);
		if ( ret != FORMAT_OK ){
			LOG_Error(LOG_TO, "Function 129. Creating MBR. Returned value: %d", ret);
			FUNC_SendResult(s->tr, FUNC_ERROR);	
			return ERR_CLOS;
		}
		LOG_Msg(1,"FORMAT Ok.");
		ret = FUNC_SendResult(s->tr, FUNC_OK);
		if ( ret != CLOS_SUCCESS ) {
			LOG_MsgError(LOG_TO, "cannot send ok");
			FUNC_SendResult(s->tr, FUNC_ERROR);
			return ERR_CLOS;
		}
	}
	else{
		LOG_MsgError(LOG_TO, "NOT REMOVABLE");
		FUNC_SendResult(s->tr, FUNC_ERROR_IS_NOT_REMOVABLE);
	} 
#else 
        //Function not implemented 
        FUNC_SendResult(s->tr, FUNC_ERROR);
#endif 
	LOG_Msg(1,"Exiting with Success.");
	return CLOS_SUCCESS;
}



int FUNC_FormatData ( session_t *s ){
	
#ifdef WIN32 
        int lenDeviceSel=0, ret=0;
	void * aux = NULL;
	char deviceName[MAX_DEVICE_LEN+1], percent=0;


	ret = TRANS_Receive( s->tr, &lenDeviceSel, 4 );
	if ( ret != CLOS_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);
		LOG_Error(LOG_TO, "Function 131. Receiving size of device selected. Returned value: %d", ret);
		return ERR_CLOS;
	}

	LOG_Debug(LOG_TO,"Received size of path = %d",lenDeviceSel);

	if ( lenDeviceSel > MAX_DEVICE_LEN  ){
		FUNC_SendResult(s->tr, FUNC_ERROR);
		LOG_MsgError(LOG_TO, "Function 131. Receiving device selected too long.");
		return ERR_INVALID_PARAMETER;
	}

	ret = TRANS_Receive( s->tr, deviceName, lenDeviceSel );
	if ( ret != CLOS_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);
		LOG_Error(LOG_TO, "Function 131. Receiving device selected. Returned value: %d", ret);
		return ERR_CLOS;
	}
	deviceName[lenDeviceSel]= '\0';

	LOG_Debug(LOG_TO,"Received device = %s",deviceName);

	if ( ! s->admin ){
		FUNC_SendResult(s->tr, ERROR_NO_ADMIN);
		return CLOS_SUCCESS;
	}

	if  ( IO_Is_Removable(deviceName) ){
		ret= FORMAT_FormatClauerData(deviceName);
		if ( ret != FORMAT_OK ){
			LOG_Error(LOG_TO, "Function 129. Creating MBR. Returned value: %d", ret);
			FUNC_SendResult(s->tr, FUNC_ERROR);	
			return ERR_CLOS;
		}

		ret = FUNC_SendResult(s->tr, FUNC_OK);
		if ( ret != CLOS_SUCCESS ) {
			FUNC_SendResult(s->tr, FUNC_ERROR);
			return ERR_CLOS;
		}
	}
	else{
		FUNC_SendResult(s->tr, FUNC_ERROR_IS_NOT_REMOVABLE);
	} 

#else
        //Function not implemented 
	FUNC_SendResult(s->tr, FUNC_ERROR);
#endif 

	return CLOS_SUCCESS;
}



int FUNC_FormatCrypto ( session_t *s ){
    int lenDeviceSel=0, lenPwd=0, ret=0;
    void * pwd = NULL;
    char deviceName[MAX_DEVICE_LEN+1];
    
    
    ret = TRANS_Receive( s->tr, &lenDeviceSel, 4 );
    if ( ret != CLOS_SUCCESS ) {
	FUNC_SendResult(s->tr, FUNC_ERROR);
	LOG_Error(LOG_TO, "Function 132. Receiving size of device selected. Returned value: %d", ret);
	return ERR_CLOS;
    }
    
    LOG_Debug(LOG_TO,"Received size of path = %d",lenDeviceSel);
    
    if ( lenDeviceSel > MAX_DEVICE_LEN  ){
	FUNC_SendResult(s->tr, FUNC_ERROR);
	LOG_MsgError(LOG_TO, "Function 132. Receiving device selected too long.");
	return ERR_INVALID_PARAMETER;
    }
    
    ret = TRANS_Receive( s->tr, deviceName, lenDeviceSel );
    if ( ret != CLOS_SUCCESS ) {
	FUNC_SendResult(s->tr, FUNC_ERROR);
	LOG_Error(LOG_TO, "Function 132. Receiving device selected. Returned value: %d", ret);
	return ERR_CLOS;
    }
    deviceName[lenDeviceSel]= '\0';
    
    LOG_Debug(LOG_TO,"Received device = %s",deviceName);
    
    
    // Now we must receive the password
    ret = TRANS_Receive( s->tr, &lenPwd, 4 );
    if ( ret != CLOS_SUCCESS ) {
	FUNC_SendResult(s->tr, FUNC_ERROR);
	LOG_Error(LOG_TO, "Function 132. Receiving size of the pwd. Returned value: %d", ret);
	return ERR_CLOS;
    }
    
    if ( lenPwd > MAX_PASS_LEN  ){
	FUNC_SendResult(s->tr, FUNC_ERROR);
	LOG_MsgError(LOG_TO, "Function 132. Receiving pwd too long.");
	return ERR_INVALID_PARAMETER;
    }
    
    ret = SMEM_New ( &pwd, lenPwd + 1 );
    if ( ret != CLOS_SUCCESS ) {
	FUNC_SendResult(s->tr, FUNC_ERROR);
	LOG_Error(LOG_TO, "Function 1 : %d", ret);
	return ERR_CLOS;
    }
    
    
    ret = TRANS_Receive(s->tr, pwd, lenPwd);
    if ( ret != CLOS_SUCCESS ) {
	SMEM_Free(pwd, lenPwd+1);
	FUNC_SendResult(s->tr, FUNC_ERROR);
	
	LOG_Error(LOG_TO, "Function 1. Receiving password: %d", ret);
	return ERR_CLOS;
    }
    
    *((unsigned char *)pwd + lenPwd) = 0;
    
    if ( ! s->admin ){
	FUNC_SendResult(s->tr, ERROR_NO_ADMIN);
	return CLOS_SUCCESS;
    }
#ifdef WIN32  
    if  ( IO_Is_Removable(deviceName) ){
#endif
	ret= FORMAT_FormatClauerCrypto(deviceName, pwd);
	if ( ret != FORMAT_OK ){
	    SMEM_Free(pwd, lenPwd+1);
	    LOG_Error(LOG_TO, "Function 132. Creating MBR. Returned value: %d", ret);
	    FUNC_SendResult(s->tr, FUNC_ERROR);	
	    return ERR_CLOS;
	}
	
	ret = FUNC_SendResult(s->tr, FUNC_OK);
	if ( ret != CLOS_SUCCESS ) {
	    SMEM_Free(pwd, lenPwd+1);
	    FUNC_SendResult(s->tr, FUNC_ERROR);
	    return ERR_CLOS;
	}
#ifdef WIN32 
    }
    else{
	FUNC_SendResult(s->tr, FUNC_ERROR_IS_NOT_REMOVABLE);
    }
#endif    
    SMEM_Free(pwd, lenPwd+1);
    
    return CLOS_SUCCESS;
}



int FUNC_GetClauerLayout( session_t *s){
#ifdef WIN32 
	int lenDeviceSel=0, ret=0;
	void * aux = NULL;
	char deviceName[MAX_DEVICE_LEN+1], percent=0;
	CLAUER_LAYOUT_INFO cli;

	ret = TRANS_Receive( s->tr, &lenDeviceSel, 4 );
	if ( ret != CLOS_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);
		LOG_Error(LOG_TO, "Function 130. Receiving size of device selected. Returned value: %d", ret);
		return ERR_CLOS;
	}

	LOG_Debug(LOG_TO,"Received size of path = %d",lenDeviceSel);

	if ( lenDeviceSel > MAX_DEVICE_LEN  ){
		FUNC_SendResult(s->tr, FUNC_ERROR);
		LOG_MsgError(LOG_TO, "Function 130. Receiving device selected too long.");
		return ERR_INVALID_PARAMETER;
	}

	ret = TRANS_Receive( s->tr, deviceName, lenDeviceSel );
	if ( ret != CLOS_SUCCESS ) {
		FUNC_SendResult(s->tr, FUNC_ERROR);
		LOG_Error(LOG_TO, "Function 130. Receiving device selected. Returned value: %d", ret);
		return ERR_CLOS;
	}
	deviceName[lenDeviceSel]= '\0';

	LOG_Debug(LOG_TO,"Received device = %s",deviceName);

	if ( ! s->admin ){
		FUNC_SendResult(s->tr, ERROR_NO_ADMIN);
		return CLOS_SUCCESS;
	}

	if  ( IO_Is_Removable(deviceName) ){
		ret= FORMAT_GetClauerLayout(deviceName, &cli);
		if ( ret != FORMAT_OK ){
			LOG_Error(LOG_TO, "Function 130. Creating MBR. Returned value: %d", ret);
			FUNC_SendResult(s->tr, FUNC_ERROR);	
			return ERR_CLOS;
		}

		// Ahora enviamos el struct
		LOG_Debug(1, "Enviando cli.npartitions = %d", cli.npartitions);
		LOG_Debug(1, "Con tamaño = %d", sizeof(cli));
		ret = TRANS_Send( s->tr, &cli, sizeof(cli) );
		if ( ret != CLOS_SUCCESS ) {
			FUNC_SendResult(s->tr, FUNC_ERROR);
			LOG_Error(LOG_TO, "Function 130. Receiving device selected. Returned value: %d", ret);
			return ERR_CLOS;
		}

		ret = FUNC_SendResult(s->tr, FUNC_OK);
		if ( ret != CLOS_SUCCESS ) {
			FUNC_SendResult(s->tr, FUNC_ERROR);
			return ERR_CLOS;
		}
	}
	else{
		FUNC_SendResult(s->tr, FUNC_ERROR_IS_NOT_REMOVABLE);
	} 
#else	
   //Function not implemented on other systems  
   FUNC_SendResult(s->tr, FUNC_ERROR);
#endif   

return CLOS_SUCCESS;


}
/* END ADMIN FUNCTIONS */



int FUNC_LockClauer ( session_t *s )
{


	return CLOS_SUCCESS;

}


int FUNC_UnlockClauer ( session_t *s )
{


	return CLOS_SUCCESS;
}

