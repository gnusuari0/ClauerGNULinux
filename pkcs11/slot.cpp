
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


/* 
 * Description: Contains the implementation of the 
 *              functions related to the slot.
 *  
 * 
 *                         Clauer Team 2006 
 **/               

#include "common.h"	 // padding function

#include "log.h"
#include "slot.h"
#include <stdlib.h>
#include <string.h>	 // memcpy
#include <LIBRT/libRT.h>  


#ifdef WIN32
#include <windows.h>
#endif

#ifdef OPENSSL_ERR
#include <openssl/err.h>
#endif 

Slot::Slot(CK_SLOT_ID _id,
           CK_CHAR *_slotDescription,
           CK_CHAR *_manufacturerID,
           CK_FLAGS _flags)
{
    
    USBCERTS_HANDLE handle;
    unsigned char * dispositivos[MAX_DEVICES];
    int err, nDispositivos, setZero= 0;
    
	multiSignOp=0; 
    slotID = _id;
    slotState= CKS_RW_PUBLIC_SESSION;
    
    strncpy((char *)slotDescription, (const char * )_slotDescription, 64);
    strncpy((char *)manufacturerID, (const char *)_manufacturerID, 32);
    
    slotFlags = _flags;
    hardwareVersion.major = HARDWARE_VERSION_MAJOR;
    hardwareVersion.minor = HARDWARE_VERSION_MINOR;
    firmwareVersion.major = FIRMWARE_VERSION_MAJOR;
    firmwareVersion.minor = FIRMWARE_VERSION_MINOR;
  
    DWORD lang= IDIOMA_Obtener();


#ifdef IDCAT_VERSION
    switch ( lang ){
    case ES:
	pClauer = new Clauer((CK_CHAR *)LABEL_IDCAT_CA,
			     (CK_CHAR *)MANUFACTURER_ID_IDCAT_ES,
			     (CK_CHAR *)MODEL_IDCAT_ES,
			     (CK_CHAR *)SERIAL_NUMBER,
			     TOKEN_FLAGS,
			     MAX_SESSIONS,0,0,0,
			     MAX_PIN_LEN,MIN_PIN_LEN,
			     MEM_PUBLICA,MEM_PUBLICA,
			     MEM_PRIVADA,MEM_PRIVADA,
			     &hardwareVersion, &firmwareVersion,
			     (CK_CHAR *)UTC_TIME
			     );

	break;
    case CA:
	pClauer = new Clauer((CK_CHAR *)LABEL_IDCAT_CA,
			     (CK_CHAR *)MANUFACTURER_ID_IDCAT_CA,
			     (CK_CHAR *)MODEL_IDCAT_CA,
			     (CK_CHAR *)SERIAL_NUMBER,
			     TOKEN_FLAGS,
			     MAX_SESSIONS,0,0,0,
			     MAX_PIN_LEN,MIN_PIN_LEN,
			     MEM_PUBLICA,MEM_PUBLICA,
			     MEM_PRIVADA,MEM_PRIVADA,
			     &hardwareVersion, &firmwareVersion,
			     (CK_CHAR *)UTC_TIME
			     );
	break;
    default:
	pClauer = new Clauer((CK_CHAR *)LABEL_IDCAT_EN,
			     (CK_CHAR *)MANUFACTURER_ID_IDCAT_EN,
			     (CK_CHAR *)MODEL_IDCAT_EN,
			     (CK_CHAR *)SERIAL_NUMBER,
			     TOKEN_FLAGS,
			     MAX_SESSIONS,0,0,0,
			     MAX_PIN_LEN,MIN_PIN_LEN,
			     MEM_PUBLICA,MEM_PUBLICA,
			     MEM_PRIVADA,MEM_PRIVADA,
			     &hardwareVersion, &firmwareVersion,
			     (CK_CHAR *)UTC_TIME
			     );
    }

#elif ACCV_VERSION
 switch ( lang ){
    case ES:
	pClauer = new Clauer((CK_CHAR *)LABEL_ACCV_CA,
			     (CK_CHAR *)MANUFACTURER_ID_ACCV_ES,
			     (CK_CHAR *)MODEL_ACCV_ES,
			     (CK_CHAR *)SERIAL_NUMBER,
			     TOKEN_FLAGS,
			     MAX_SESSIONS,0,0,0,
			     MAX_PIN_LEN,MIN_PIN_LEN,
			     MEM_PUBLICA,MEM_PUBLICA,
			     MEM_PRIVADA,MEM_PRIVADA,
			     &hardwareVersion, &firmwareVersion,
			     (CK_CHAR *)UTC_TIME
			     );
	break;
    case CA:
	pClauer = new Clauer((CK_CHAR *)LABEL_ACCV_CA,
			     (CK_CHAR *)MANUFACTURER_ID_ACCV_CA,
			     (CK_CHAR *)MODEL_ACCV_CA,
			     (CK_CHAR *)SERIAL_NUMBER,
			     TOKEN_FLAGS,
			     MAX_SESSIONS,0,0,0,
			     MAX_PIN_LEN,MIN_PIN_LEN,
			     MEM_PUBLICA,MEM_PUBLICA,
			     MEM_PRIVADA,MEM_PRIVADA,
			     &hardwareVersion, &firmwareVersion,
			     (CK_CHAR *)UTC_TIME
			     );
	break;
    default:
	pClauer = new Clauer((CK_CHAR *)LABEL_ACCV_EN,
			     (CK_CHAR *)MANUFACTURER_ID_ACCV_EN,
			     (CK_CHAR *)MODEL_ACCV_EN,
			     (CK_CHAR *)SERIAL_NUMBER,
			     TOKEN_FLAGS,
			     MAX_SESSIONS,0,0,0,
			     MAX_PIN_LEN,MIN_PIN_LEN,
			     MEM_PUBLICA,MEM_PUBLICA,
			     MEM_PRIVADA,MEM_PRIVADA,
			     &hardwareVersion, &firmwareVersion,
			     (CK_CHAR *)UTC_TIME
			     );
    }
#else
    switch ( lang ){
    case ES:
	pClauer = new Clauer((CK_CHAR *)LABEL_ES,
			     (CK_CHAR *)MANUFACTURER_ID_ES,
			     (CK_CHAR *)MODEL_ES,
			     (CK_CHAR *)SERIAL_NUMBER,
			     TOKEN_FLAGS,
			     MAX_SESSIONS,0,0,0,
			     MAX_PIN_LEN,MIN_PIN_LEN,
			     MEM_PUBLICA,MEM_PUBLICA,
			     MEM_PRIVADA,MEM_PRIVADA,
			     &hardwareVersion, &firmwareVersion,
			     (CK_CHAR *)UTC_TIME
			     );

	break;
    case CA:
	pClauer = new Clauer((CK_CHAR *)LABEL_CA,
			     (CK_CHAR *)MANUFACTURER_ID_CA,
			     (CK_CHAR *)MODEL_CA,
			     (CK_CHAR *)SERIAL_NUMBER,
			     TOKEN_FLAGS,
			     MAX_SESSIONS,0,0,0,
			     MAX_PIN_LEN,MIN_PIN_LEN,
			     MEM_PUBLICA,MEM_PUBLICA,
			     MEM_PRIVADA,MEM_PRIVADA,
			     &hardwareVersion, &firmwareVersion,
			     (CK_CHAR *)UTC_TIME
			     );
	break;
    default:
	pClauer = new Clauer((CK_CHAR *)LABEL_EN,
			     (CK_CHAR *)MANUFACTURER_ID_EN,
			     (CK_CHAR *)MODEL_EN,
			     (CK_CHAR *)SERIAL_NUMBER,
			     TOKEN_FLAGS,
			     MAX_SESSIONS,0,0,0,
			     MAX_PIN_LEN,MIN_PIN_LEN,
			     MEM_PUBLICA,MEM_PUBLICA,
			     MEM_PRIVADA,MEM_PRIVADA,
			     &hardwareVersion, &firmwareVersion,
			     (CK_CHAR *)UTC_TIME
			     );
    }
#endif 


   
    
    // Global list that holds token objects
    // it is passed to the token functions
    pListaObjetos= new listaObjetos;
    
    _pMechanism= NULL;
    
    memset(zeroId, 0, CLAUER_ID_LEN);
    
    // On an slot creation, we list for clauers existance 
    // and initialize the clauerId to it.
    err= LIBRT_ListarDispositivos(&nDispositivos,dispositivos);
    if ( err == ERR_LIBRT_SI){
	setZero=1;
	LOG_MsgError(LOG_TO,"Error listando dispositivos");	
    }
    else{
	if ( nDispositivos == 0 ){
	    LOG_Msg(LOG_TO,"No se encontraron dispositivos en el sistema"); 	    
	    setZero=1;
	}
	else{
	    err= LIBRT_IniciarDispositivo( dispositivos[0], NULL, &handle );
	    if ( err == ERR_LIBRT_SI ){
		setZero=1;
		LOG_MsgError(LOG_TO,"Al iniciar el dispositivo"); 	    
	    }
	    else{
		memcpy( th_id_handle.newClauerId, &handle.idDispositivo, CLAUER_ID_LEN );
		memcpy( clauerId, &handle.idDispositivo, CLAUER_ID_LEN );

		pClauer->LoadClauerObjects( pListaObjetos );
		
		slotFlags= (CKF_REMOVABLE_DEVICE | CKF_TOKEN_PRESENT);
		LIBRT_FinalizarDispositivo(&handle);
	    }	
	}
    }
    
    if ( setZero ){
	memset(th_id_handle.newClauerId, 0, CLAUER_ID_LEN);	       
	memset(clauerId, 0, CLAUER_ID_LEN);		 
    }
    // End
    
#ifdef LINUX
    pthread_mutex_init( &th_id_handle.clauerIdMutex, NULL );
    pthread_create( &thread, NULL, updateIdThread,  (void *) &th_id_handle);
#elif defined(WIN32)
    th_id_handle.clauerIdMutex = CreateMutex(NULL, FALSE, NULL);
    thread = CreateThread(NULL, 0, updateIdThread, (LPVOID) &th_id_handle, 0, NULL);
#endif
    
}

Slot::~Slot(void)
{
#ifdef LINUX
    pthread_mutex_destroy( &th_id_handle.clauerIdMutex );
#elif defined(WIN32)
    CloseHandle(th_id_handle.clauerIdMutex);
	TerminateThread(thread,0);
#endif

    delete pClauer;
}



CK_BBOOL Slot::isTokenPresent(void)
{

    CK_BBOOL ret;

    if ( CPresent() != CKR_OK )
	ret=FALSE;
    
    else if( slotFlags & CKF_TOKEN_PRESENT )
	ret=TRUE;
 
    else
	ret=FALSE;
    
    return ret;
}


CK_RV Slot::CPresent(void)
{

#ifdef LINUX
    pthread_mutex_lock( &th_id_handle.clauerIdMutex );
#elif defined(WIN32)
    WaitForSingleObject(th_id_handle.clauerIdMutex, INFINITE);
#endif

    if ( memcmp(th_id_handle.newClauerId, zeroId, CLAUER_ID_LEN ) == 0 ){
	// Some times, Logout will do nothing, but when a clauer is 
	// unplagged and then plugged again, password must not remain at
	// memory 
	C_Logout();
	memset(clauerId, 0, CLAUER_ID_LEN);
	slotFlags= CKF_REMOVABLE_DEVICE;
    }
    else if ( memcmp(th_id_handle.newClauerId, clauerId, CLAUER_ID_LEN ) == 0){
	// The clauer plugged has no changed.
	slotFlags= (CKF_REMOVABLE_DEVICE | CKF_TOKEN_PRESENT);
    }
    else {
	// We have another clauer plugged.
	memcpy( clauerId, th_id_handle.newClauerId, CLAUER_ID_LEN );
	C_Logout();
	pClauer->UnloadClauerObjects( pListaObjetos );
	pClauer->LoadClauerObjects( pListaObjetos );
	slotFlags= (CKF_REMOVABLE_DEVICE | CKF_TOKEN_PRESENT);
    }
#ifdef LINUX
    pthread_mutex_unlock( &th_id_handle.clauerIdMutex );
#elif defined(WIN32)
    ReleaseMutex(th_id_handle.clauerIdMutex);
#endif
    
    return CKR_OK;
}


CK_RV Slot::C_GetSlotInfo (CK_SLOT_INFO_PTR pInfo)
{
    
    slotDescription[63]= '\0';
    padding((char *)pInfo->slotDescription, (char *)slotDescription,64);
    padding((char *)pInfo->manufacturerID, (char *)manufacturerID,32);

    CPresent();

    pInfo->flags = slotFlags;
    //  printf("pInfo->flags= 0x%x\n",(unsigned int) pInfo->flags);
    pInfo->hardwareVersion.major = hardwareVersion.major;
    pInfo->hardwareVersion.minor = hardwareVersion.minor;
    pInfo->firmwareVersion.major = firmwareVersion.major;
    pInfo->firmwareVersion.minor = firmwareVersion.minor;
    
    return CKR_OK;
}


CK_RV Slot::C_GetTokenInfo (CK_TOKEN_INFO_PTR pInfo)
{
    return pClauer->C_GetTokenInfo(pInfo);
}


CK_RV Slot::C_GetMechanismList (CK_MECHANISM_TYPE_PTR pMechanismList, CK_ULONG_PTR pulCount)
{
    return pClauer->C_GetMechanismList(pMechanismList,pulCount);
}

CK_RV Slot::C_GetMechanismInfo(CK_MECHANISM_TYPE type, CK_MECHANISM_INFO_PTR pInfo)
{
    return pClauer->C_GetMechanismInfo(type,pInfo);
}


CK_RV Slot::C_OpenSession (CK_FLAGS _flags, CK_VOID_PTR pApplication, CK_NOTIFY Notify, CK_SESSION_HANDLE_PTR phSession)
{

    if (pClauer->get_ulMaxSessionCount() == pClauer->get_ulSessionCount()){
	LOG_Msg(LOG_TO,"Salimos por sesion count"); 
	//return CKR_SESSION_COUNT;
    }
	

    if (!(_flags & CKF_SERIAL_SESSION)){			// No parallel sessions
	LOG_Msg(LOG_TO,"Salimos por no parallel sessions"); 
	return CKR_SESSION_PARALLEL_NOT_SUPPORTED;
    }

    if (_flags & CKF_INSERTION_CALLBACK){			// No callbacks
	LOG_Msg(LOG_TO,"Salimos por no callbacks"); 
	return CKR_INSERTION_CALLBACK_NOT_SUPPORTED;
    }
    
    *phSession=1;
    pClauer->set_ulSessionCount(1);
    
    return CKR_OK;
}


CK_RV Slot::C_CloseSession (CK_SESSION_HANDLE pSession)
{
    pClauer->set_ulSessionCount(0);
    return pClauer->C_CloseSession();
}


CK_RV Slot::C_CloseAllSessions(void)
{
    CK_SESSION_HANDLE pSession= 1;
    return C_CloseSession(pSession);
}




CK_RV Slot::C_SignInit (CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hKey)
{

	LOG_Msg(LOG_TO,"\n\n ====  Iniciando secuencia de firma ====");
    LOG_Debug(LOG_TO,"Estado el slot es: %d",slotState );

    if (_pMechanism){
		LOG_Msg(LOG_TO,"Saliendo de SignInit por CKR_OPERATION_ACTIVE");
		return CKR_OPERATION_ACTIVE;
    }
    else
        {
		
		if ( pMechanism->mechanism != CKM_RSA_PKCS && pMechanism->mechanism != CKM_SHA1_RSA_PKCS ) {
			LOG_Msg(LOG_TO, "Saliendo por tipo de mecanísmo no soportado.");
			return CKR_MECHANISM_INVALID;
		}

	    if (!pListaObjetos->buscarObjeto(hKey)){
			LOG_Debug(LOG_TO,"Handle de la llave invalido hKey= %d", hKey);
			return CKR_KEY_HANDLE_INVALID;
	    }
	    else{
			multiSignOp=0;
			EVP_MD_CTX_cleanup(&multiCtx);
		    LOG_Msg(LOG_TO, "Bloqueando mecanísmo");
			_pMechanism = (CK_MECHANISM_PTR) malloc(sizeof(CK_MECHANISM));
		 	memcpy(_pMechanism,pMechanism,sizeof(CK_MECHANISM));
			_hKey = hKey;
        }
    }

	LOG_Msg(LOG_TO, "Saliendo por CKR_OK SignInit");
    return CKR_OK;
}


CK_RV Slot::C_Sign (CK_BYTE_PTR pData, CK_ULONG ulDataLen, CK_BYTE_PTR pSignature, CK_ULONG_PTR pulSignatureLen)
{
    RSA *RSAKey;
    CK_ULONG longbuffer;
    ClaveRSA *pObjectRSAKey;
    unsigned char * keyId, * auxBuff, * md;;
	unsigned long err;
	unsigned int mdSize;
	EVP_MD_CTX ctx;

    CK_RV ret;
	
    LOG_Msg(LOG_TO, "FIRMANDO ...  " );
    pObjectRSAKey = (ClaveRSA *)pListaObjetos->buscarObjeto(_hKey);
    if (!pObjectRSAKey){
		LOG_MsgError(LOG_TO, "No se encontro el objeto llave RSA devolviendo CKR_FUNCTION_CANCELLED");
		free(_pMechanism);
		_pMechanism = NULL;
		return CKR_FUNCTION_CANCELED;
    }
    
	LOG_Debug(LOG_TO, "Se ha encontrado la clave con handle= %d",_hKey);
    LOG_Msg(LOG_TO, "Recuperando clave...");
	
	keyId= pObjectRSAKey->get_keyId();    
    ret= pClauer->getRSAKey(keyId, &RSAKey, &th_id_handle);
    if ( ret== CKR_FUNCTION_FAILED ){
		LOG_Msg(LOG_TO, "No se ha podido obtener la clave para firmar" );
		return CKR_FUNCTION_FAILED;
    }

    if (!RSAKey || ret != CKR_OK ){
		LOG_MsgError(LOG_TO, "No se encontro el objeto llave RSA");
		LOG_Msg(LOG_TO, "RSAKEY ES NULL " );
		free(_pMechanism);
		_pMechanism = NULL;
		return CKR_FUNCTION_CANCELED;
    }
    LOG_Msg(LOG_TO, "La clave RSA se ha recuperado con éxito desde el clauer.");

    longbuffer = RSA_size(RSAKey);
    
	LOG_Debug(LOG_TO, "Obteniendo tamaño resultante de la firma= %ld", longbuffer);

    if (!pSignature){
	    
		*pulSignatureLen = longbuffer;
	    RSA_free(RSAKey);
	    
		LOG_Debug(LOG_TO, "pSignature es NULL_PTR, devolviendo tamaño de la firma en *pulSignatureLen= %ld",*pulSignatureLen );
	    LOG_Msg(LOG_TO,"Devolviendo CKR_OK");
		
		return CKR_OK;
    
	}
    else{
		//if (*pulSignatureLen >= longbuffer){
		if (*pulSignatureLen >= longbuffer){

#ifdef _DEBUG
			FILE * fp;
			fp= fopen("c:\\d.bin","w+");
			fwrite(pData, ulDataLen, 1, fp);
			fclose(fp);
#endif

			switch ( _pMechanism->mechanism ){
			
				case CKM_RSA_PKCS:
					LOG_Msg(LOG_TO, "Entramos por mecanismo CKM_RSA_PKCS.");
					*pulSignatureLen =	RSA_private_encrypt(ulDataLen, pData, pSignature, RSAKey, RSA_PKCS1_PADDING);
					LOG_Debug(LOG_TO, "Entrando por *pulSignatureLen > longbuffer, devolviendo tamaño en *pulSignatureLen= %ld", *pulSignatureLen);
			        LOG_Debug(LOG_TO, "Entrando por *pulSignatureLen > longbuffer, tamaño longbuffer= %ld", longbuffer);
					
					if ( *pulSignatureLen  == -1 ){
						LOG_MsgError(LOG_TO, "Saliendo por *pulSignatureLen == -1");

#ifdef OPENSSL_ERR
						ERR_load_crypto_strings();
						ERR_load_RSA_strings();
   
						err= ERR_get_error();

						LOG_Error(LOG_TO, "Código de error para openssl (RSA_private_encrypt): %ld", err);
						LOG_Error(LOG_TO, "ERR_lib_error_string:    %s", ERR_lib_error_string(err));
						LOG_Error(LOG_TO, "ERR_func_error_string:   %s", ERR_func_error_string(err));
						LOG_Error(LOG_TO, "ERR_reason_error_string: %s", ERR_reason_error_string(err));
#endif
						return CKR_DATA_LEN_RANGE;
					}
					break;

					case CKM_SHA1_RSA_PKCS:
						
						/* Si nos pasan este mecanismo tenemos que calcular el hash */
						LOG_Msg(LOG_TO, "Entramos por mecanismo CKM_SHA1_RSA_PKCS.");
						EVP_MD_CTX_init(&ctx);
						EVP_DigestInit_ex(&ctx, EVP_sha1(), NULL);
						EVP_DigestUpdate(&ctx, pData, ulDataLen);
    
						md= (unsigned char *) malloc(20);
						if ( !md ){
							LOG_MsgError(LOG_TO, "No pude reservar memoria para el hash");
							return CKR_FUNCTION_CANCELED;
						}
      
						EVP_DigestFinal_ex(&ctx, md, &mdSize);
    
						if ( mdSize != 20 ){
							LOG_MsgError(LOG_TO, "La longitud del hash SHA1 no son 20 bytes");
							return CKR_FUNCTION_CANCELED;	
						}


#ifdef _DEBUG
						LOG_Msg(LOG_TO, "Logeando hash");
							
						unsigned char mdauxBuff[41];
						for(char i=0; i<20; i++){
								snprintf( (char * )mdauxBuff+(2*i), 2, "%02x", md[i] );
						}
						mdauxBuff[40]='\0';
						LOG_Debug(LOG_TO, "RSA_SIGN: HASH: en hexadecimal es: %s",mdauxBuff);
						
#endif
					
						/* Y ahora lo ciframos */
						unsigned char newMD[35];
						memcpy(newMD, sha1sigheader, 15);
						memcpy(newMD+15, md, 20);
						
						//newMD[34]=0;

						//*pulSignatureLen =	RSA_private_encrypt(mdSize, md, pSignature, RSAKey, RSA_PKCS1_PADDING);
						*pulSignatureLen =	RSA_private_encrypt(35, newMD, pSignature, RSAKey, RSA_PKCS1_PADDING);
						//if ( ! RSA_sign( NID_sha1, md, mdSize, pSignature, (unsigned int *)pulSignatureLen, RSAKey ) ){
						if ( *pulSignatureLen == -1 ){	
							LOG_Debug(LOG_TO, "RSA_SIGN: Entrando por *pulSignatureLen >= longbuffer, devolviendo tamaño en *pulSignatureLen= %ld", *pulSignatureLen);
			
							free(md);
							EVP_MD_CTX_cleanup(&ctx);
							LOG_MsgError(LOG_TO, "Saliendo por *pulSignatureLen == -1");

#ifdef OPENSSL_ERR
							ERR_load_crypto_strings();
							ERR_load_RSA_strings();
   
							err= ERR_get_error();

							LOG_Error(LOG_TO, "Código de error para openssl (RSA_private_encrypt): %ld", err);
							LOG_Error(LOG_TO, "ERR_lib_error_string:    %s", ERR_lib_error_string(err));
							LOG_Error(LOG_TO, "ERR_func_error_string:   %s", ERR_func_error_string(err));
							LOG_Error(LOG_TO, "ERR_reason_error_string: %s", ERR_reason_error_string(err));
#endif
							return CKR_DATA_LEN_RANGE;
						}
							
						break;

					default:
						/* clean up and mechanism invalid */
						free(_pMechanism);
						_pMechanism = NULL; 
						RSA_free(RSAKey);
						LOG_Msg(LOG_TO, "Saliendo por default en el case");
						return CKR_MECHANISM_INVALID;
			}
		}
		else{
		    free(_pMechanism);
		    _pMechanism = NULL; 
		    *pulSignatureLen = longbuffer;
		    RSA_free(RSAKey);
			LOG_Debug(LOG_TO, "Entrando por else *pulSignatureLen < longbuffer, devolviendo CKR_BUFFER_TOO_SMALL *pulSignatureLen= %ld", *pulSignatureLen);
		    return CKR_BUFFER_TOO_SMALL;
		}
    }

    // Concluimos la operacion de firma
    free(_pMechanism);
    _pMechanism = NULL;
    RSA_free(RSAKey);
 
#ifdef _DEBUG
	LOG_Msg(LOG_TO, "La firma se ha realizado correctamente devolviendo CKR_OK" );
	LOG_Debug(LOG_TO, "El tamaño final de la firma es *pulSignatureLen= %ld", *pulSignatureLen);
	
	auxBuff= (unsigned char * )malloc(((*pulSignatureLen )* 2 )+1);
	if ( !auxBuff ){
	  LOG_MsgError(LOG_TO, "No hay memoria para logear la firma");
	}
	else{
		for(unsigned long i=0; i<*pulSignatureLen; i++){
			snprintf( ((char*)auxBuff)+(2*i), 2, "%02x", *(pSignature+i) );
		}
		auxBuff[(*pulSignatureLen)* 2]='\0';

		LOG_Debug(LOG_TO, "Y el resultado en hexadecimal es: %s",auxBuff);
	}
	LOG_Msg(LOG_TO, "Devolviendo CKR_OK");
	free(auxBuff);
#endif

    return CKR_OK;
}



CK_RV Slot::C_SignUpdate( CK_BYTE_PTR pPart, CK_ULONG ulPartLen )
{

	switch ( _pMechanism->mechanism ){

		case CKM_SHA1_RSA_PKCS:
			if (multiSignOp == 0){
		       EVP_MD_CTX_init(&multiCtx);
			   EVP_DigestInit_ex(&multiCtx, EVP_sha1(), NULL);
			   multiSignOp=1;  
			}

			EVP_DigestUpdate(&multiCtx, pPart, ulPartLen);
			break;
		default:
			/* clean up and mechanism invalid */
			free(_pMechanism);
			_pMechanism = NULL; 
			LOG_Msg(LOG_TO, "Saliendo por default en el case");
			return CKR_MECHANISM_INVALID;
	}

	return CKR_OK;
}

CK_RV Slot::C_SignFinal( CK_BYTE_PTR pSignature, CK_ULONG_PTR pulSignatureLen )
{
	RSA *RSAKey;
	CK_ULONG longbuffer;
	ClaveRSA *pObjectRSAKey;
	unsigned char * keyId, * auxBuff, * md;;
	unsigned long err;
	unsigned int mdSize;
	EVP_MD_CTX ctx;
	CK_RV ret;

	if ( multiSignOp==0 || !_pMechanism ){
		return CKR_FUNCTION_FAILED;
	}  


	pObjectRSAKey = (ClaveRSA *)pListaObjetos->buscarObjeto(_hKey);
	if (!pObjectRSAKey){
		LOG_MsgError(LOG_TO, "No se encontro el objeto llave RSA devolviendo CKR_FUNCTION_CANCELLED");
		free(_pMechanism);
		_pMechanism = NULL;
		return CKR_FUNCTION_CANCELED;
	}

	LOG_Debug(LOG_TO, "Se ha encontrado la clave con handle= %d",_hKey);
	LOG_Msg(LOG_TO, "Recuperando clave...");

	keyId= pObjectRSAKey->get_keyId();    
	ret= pClauer->getRSAKey(keyId, &RSAKey, &th_id_handle);
	if ( ret== CKR_FUNCTION_FAILED ){
		LOG_Msg(LOG_TO, "No se ha podido obtener la clave para firmar" );
		return CKR_FUNCTION_FAILED;
	}

	if (!RSAKey || ret != CKR_OK ){
		LOG_MsgError(LOG_TO, "No se encontro el objeto llave RSA");
		LOG_Msg(LOG_TO, "RSAKEY ES NULL " );
		free(_pMechanism);
		_pMechanism = NULL;
		return CKR_FUNCTION_CANCELED;
	}
	LOG_Msg(LOG_TO, "La clave RSA se ha recuperado con éxito desde el clauer.");

	longbuffer = RSA_size(RSAKey);

	LOG_Debug(LOG_TO, "Obteniendo tamaño resultante de la firma= %ld", longbuffer);

	if (!pSignature){

		*pulSignatureLen = longbuffer;
		RSA_free(RSAKey);

		LOG_Debug(LOG_TO, "pSignature es NULL_PTR, devolviendo tamaño de la firma en *pulSignatureLen= %ld",*pulSignatureLen );
		LOG_Msg(LOG_TO,"Devolviendo CKR_OK");

		return CKR_OK;

	}
	else{
		//if (*pulSignatureLen >= longbuffer){
		if (*pulSignatureLen >= longbuffer){
			//First we get the digest from the context:
			md= (unsigned char *) malloc(20);
			if ( !md ){
				LOG_MsgError(LOG_TO, "No pude reservar memoria para el hash");
				return CKR_FUNCTION_CANCELED;
			}

			EVP_DigestFinal_ex(&multiCtx, md, &mdSize);

			if ( mdSize != 20 ){
				LOG_MsgError(LOG_TO, "La longitud del hash SHA1 no son 20 bytes");
				return CKR_FUNCTION_CANCELED;	
			}	

			//Compute the signature 
			unsigned char newMD[35];
			memcpy(newMD, sha1sigheader, 15);
			memcpy(newMD+15, md, 20);

			*pulSignatureLen =	RSA_private_encrypt(35, newMD, pSignature, RSAKey, RSA_PKCS1_PADDING);
			LOG_Debug(LOG_TO, "Firma Calculada: *pulSignatureLen= %ld", *pulSignatureLen);
			if ( *pulSignatureLen == -1 ){	
				LOG_Debug(LOG_TO, "RSA_SIGN: Entrando por *pulSignatureLen >= longbuffer, devolviendo tamaño en *pulSignatureLen= %ld", *pulSignatureLen);
				free(md);
				EVP_MD_CTX_cleanup(&multiCtx);
				LOG_MsgError(LOG_TO, "Saliendo por *pulSignatureLen == -1");

#ifdef OPENSSL_ERR
				ERR_load_crypto_strings();
				ERR_load_RSA_strings();

				err= ERR_get_error();

				LOG_Error(LOG_TO, "Código de error para openssl (RSA_private_encrypt): %ld", err);
				LOG_Error(LOG_TO, "ERR_lib_error_string:    %s", ERR_lib_error_string(err));
				LOG_Error(LOG_TO, "ERR_func_error_string:   %s", ERR_func_error_string(err));
				LOG_Error(LOG_TO, "ERR_reason_error_string: %s", ERR_reason_error_string(err));
#endif
				return CKR_DATA_LEN_RANGE;
			}
		}
		else{
			free(_pMechanism);
			_pMechanism = NULL; 
			*pulSignatureLen = longbuffer;
			RSA_free(RSAKey);
			LOG_Debug(LOG_TO, "Entrando por else *pulSignatureLen < longbuffer, devolviendo CKR_BUFFER_TOO_SMALL *pulSignatureLen= %ld", *pulSignatureLen);
			return CKR_BUFFER_TOO_SMALL;
		}
	}

	LOG_Msg(LOG_TO,"Devolviendo CKR_OK");
	free(_pMechanism); 
	_pMechanism = NULL;
	RSA_free(RSAKey);
	
	return CKR_OK; 
}




CK_RV Slot::C_VerifyInit( CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR  pMechanism, CK_OBJECT_HANDLE  hKey )
{
 
    if (_pMechanism){
		LOG_Msg(LOG_TO, "Mecanismo bloqueado");
		return CKR_OPERATION_ACTIVE;
    }
    else
        {
		if ( pMechanism->mechanism != CKM_RSA_PKCS && pMechanism->mechanism != CKM_SHA1_RSA_PKCS ) {
			LOG_Msg(LOG_TO, "Saliendo por tipo de mecanísmo no soportado.");
			return CKR_MECHANISM_INVALID;
		}
	
		if (!pListaObjetos->buscarObjeto(hKey) || pListaObjetos->buscarObjeto(hKey)->get_class() != CKO_PUBLIC_KEY ){
			LOG_Debug(LOG_TO,"Handle de la llave invalido hKey= %d", hKey);
			return CKR_KEY_HANDLE_INVALID;
	    }
	    else{
			_pMechanism = (CK_MECHANISM_PTR) malloc(sizeof(CK_MECHANISM));
			memcpy(_pMechanism,pMechanism,sizeof(CK_MECHANISM));
			_hKeyV = hKey;
         }
     }
	
	LOG_Msg(LOG_TO, "Inicialización de verificación ok");

    return CKR_OK;
}

CK_RV Slot::C_Verify(	CK_SESSION_HANDLE hSession, CK_BYTE_PTR pData, 
			CK_ULONG ulDataLen, CK_BYTE_PTR pSignature, CK_ULONG ulSignatureLen )
{
    ClaveRSA *pObjectRSAKey;
    CK_BYTE * mod, * exp;
    CK_ULONG mod_size, exp_size;
    unsigned char * md;;
	unsigned long err;
	unsigned int mdSize;
	
	EVP_MD_CTX ctx;
	
	CK_RV ret= CKR_OK;

    unsigned char * plainData;
       
    int plainData_size, auxSize;
	
	if ( _pMechanism == NULL ){
		LOG_MsgError(1,"Operación no inicializada"); 
		return CKR_OPERATION_NOT_INITIALIZED;
	}

	if ( _pMechanism->mechanism != CKM_RSA_PKCS && _pMechanism->mechanism != CKM_SHA1_RSA_PKCS ) {
		LOG_Msg(LOG_TO, "Saliendo por tipo de mecanísmo no soportado.");
		free(_pMechanism);
		_pMechanism = NULL;
		return CKR_MECHANISM_INVALID;
	}

    pObjectRSAKey = (ClaveRSA *)pListaObjetos->buscarObjeto(_hKeyV);
    if (!pObjectRSAKey){
		LOG_MsgError(LOG_TO, "No se encontro el objeto llave RSA");
		free(_pMechanism);
		_pMechanism = NULL;
		return CKR_FUNCTION_FAILED;
    }

    mod= pObjectRSAKey->get_modulus();
    exp= pObjectRSAKey->get_publicExponent();

    if ( ! mod || ! exp ){
		LOG_MsgError(LOG_TO, "No se pudo recuperar modulo o exponente RSA");
		free(_pMechanism);
		_pMechanism = NULL;
		return CKR_FUNCTION_FAILED;
    }

    mod_size= pObjectRSAKey->get_modulus_size();
    exp_size= pObjectRSAKey->get_publicExponent_size();

#ifdef _DEBUG
	LOG_Msg(LOG_TO, "Logeando hash");
	if ( mod_size <=520  ){						
		unsigned char mdauxBuff[525 * 2];
		for(unsigned long i=0; i<mod_size; i++){
			snprintf( (char * )mdauxBuff+(2*i), 2, "%02x", mod[i] );
		}
		mdauxBuff[mod_size]='\0';
		LOG_Debug(LOG_TO, "MÓDULO: en hexadecimal es: %s",mdauxBuff);
	}else{
		LOG_Error(LOG_TO, "No se pudo loggear el módulo, es demasiado grande: %d", mod_size);
	}
#endif 


    RSA * rsa= RSA_new();

    if (! (rsa->n = BN_bin2bn(mod, mod_size, NULL)) ){
		LOG_MsgError(LOG_TO, "Al pasar el modulo a BN");
		free(_pMechanism);
		_pMechanism = NULL;
		return CKR_FUNCTION_FAILED;
    }

    if (! (rsa->e = BN_bin2bn(exp, exp_size, NULL)) ){
		LOG_MsgError(LOG_TO, "Al pasar el exponente a BN");
		free(_pMechanism);
		_pMechanism = NULL;
		return CKR_FUNCTION_FAILED;
    }

    auxSize=  RSA_size(rsa);
    if ( auxSize == 0 ){
		LOG_MsgError(LOG_TO, "Al obtener el tamaño de la llave");
		free(_pMechanism);
		_pMechanism = NULL;
		return CKR_FUNCTION_FAILED;
    }

    plainData= (unsigned char * ) malloc(auxSize); 	
    if (! plainData ){
		LOG_MsgError(LOG_TO, "Reservando memoria para descifrar");
		free(_pMechanism);
		_pMechanism = NULL;
		return CKR_FUNCTION_FAILED;
    }

	if ( _pMechanism->mechanism == CKM_RSA_PKCS ){
		
		plainData_size= RSA_public_decrypt( ulSignatureLen, pSignature, plainData, rsa, RSA_PKCS1_PADDING);
		if ( plainData_size == 0){
			LOG_MsgError(LOG_TO, "Descifrando.");
			free(_pMechanism);
			_pMechanism = NULL;
			free(plainData);
			return CKR_FUNCTION_FAILED;
		}
	
		/* Finalmente comparamos los bufferes de datos  */
		if ( memcmp(plainData, pData, ulDataLen ) == 0 ){
			LOG_Msg(LOG_TO, "Verificación correcta!");
			ret= CKR_OK;
		}
		else{
			LOG_Msg(LOG_TO, "Los datos no coinciden, firma incorecta!");
			ret= CKR_SIGNATURE_INVALID;
		}
	}
	else{
		/* Mecanismo es CKM_SHA1_RSA_PKCS */ 
		/* Resumimos los datos */
		LOG_Msg(LOG_TO, "Entramos por mecanismo CKM_SHA1_RSA_PKCS.");
		
		EVP_MD_CTX_init(&ctx);
		EVP_DigestInit_ex(&ctx, EVP_sha1(), NULL);
		EVP_DigestUpdate(&ctx, pData, ulDataLen);
    
		md= (unsigned char *) malloc(20);
		if ( !md ){
			LOG_MsgError(LOG_TO, "No pude reservar memoria para el hash");
			ret= CKR_FUNCTION_CANCELED;
			free(md);
			EVP_MD_CTX_cleanup(&ctx);
			goto end_C_Verify;
		}
      
		EVP_DigestFinal_ex(&ctx, md, &mdSize);
  
		if ( mdSize != 20 ){
			LOG_MsgError(LOG_TO, "La longitud del hash SHA1 no son 20 bytes");
			ret= CKR_FUNCTION_CANCELED;
			free(md);
			EVP_MD_CTX_cleanup(&ctx);
			goto end_C_Verify;
		}
		
		plainData_size= RSA_public_decrypt( ulSignatureLen, pSignature, plainData, rsa, RSA_PKCS1_PADDING);
		if ( plainData_size <= 0){
			LOG_MsgError(LOG_TO, "Descifrando.");
			ret= CKR_FUNCTION_FAILED;
			free(md);
			EVP_MD_CTX_cleanup(&ctx);
			goto end_C_Verify;
		}
		LOG_Debug(LOG_TO,"Los datos descifrados tienen una longitud de %d", plainData_size);

		if ( plainData_size < 33 ){
			LOG_Msg(LOG_TO, "Firma incorrecta resultado descifrado < 33 .");
			ret= CKR_SIGNATURE_INVALID;
			free(md);
			EVP_MD_CTX_cleanup(&ctx);
			goto end_C_Verify;
		}

		/* Finalmente verificamos la estructura asn1 y el hash */
		ret= CKR_OK; int offset=0, i=0;
		if ( plainData[11] == 0x05 ){ // La esctructura asn1 tiene algorithm params (NULL)
			offset=15;
			if ( plainData_size < 35 ){
				LOG_Msg(LOG_TO, "Firma incorrecta resultado descifrado < 35 .");
				ret= CKR_SIGNATURE_INVALID;
				free(md);
				EVP_MD_CTX_cleanup(&ctx);
				goto end_C_Verify;
			}
			for (i=0; i<15; i++){
				if ( sha1sigheader[i] != plainData[i] ){
					LOG_Msg(LOG_TO, "Parseando la estructura asn1.");
					ret= CKR_SIGNATURE_INVALID;
					free(md);
					EVP_MD_CTX_cleanup(&ctx);
					goto end_C_Verify;
				}
			}
		}		
		else{ // carece de algorithm params (NULL)
			offset=13;
			for (i=0; i<13; i++){
				if ( sha1sigheader_noparam[i] != plainData[i] ){
					LOG_Msg(LOG_TO, "Parseando la estructura asn1.");
					ret= CKR_SIGNATURE_INVALID;
					free(md);
					EVP_MD_CTX_cleanup(&ctx);
					goto end_C_Verify;
				}
			}
				
		}
		
		if ( memcmp(plainData + offset, md, mdSize) == 0 && ret == CKR_OK ){
			LOG_Msg(LOG_TO, "Verificación correcta!");
			ret= CKR_OK;
		}
		else{
			LOG_Msg(LOG_TO, "Los datos no coinciden, firma incorecta!");
			ret= CKR_SIGNATURE_INVALID;
		}
		free(md);
		EVP_MD_CTX_cleanup(&ctx);
	}

    // Concluimos la operacion de verificación

end_C_Verify:

	free(_pMechanism);
    free(plainData);
    _pMechanism = NULL;
    RSA_free(rsa);
    LOG_Msg(LOG_TO, "Fin Verificación" );
	
    return ret;
}




CK_RV Slot::C_DecryptInit (CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hKey)
{

    if ( !pMechanism ){
	return CKR_MECHANISM_INVALID;
    }
    
    // Comprobamos que no se esta realizando ninguna operacion
    if (_pMechanism)
		return CKR_OPERATION_ACTIVE;
    else
        {
		if ( pMechanism->mechanism != CKM_RSA_PKCS ){
			LOG_Msg(LOG_TO, "Saliendo por tipo de mecanísmo no soportado.");
			return CKR_MECHANISM_INVALID;
		}

		// Buscamos el objeto en la lista de objetos de la sesion
	    if (!pListaObjetos->buscarObjeto(hKey))
			return CKR_KEY_HANDLE_INVALID;
	    else
            {
			_pMechanism = (CK_MECHANISM_PTR) malloc(sizeof(CK_MECHANISM));
			memcpy(_pMechanism,pMechanism,sizeof(CK_MECHANISM));
		    _hKey = hKey;
        }
     }
    
    return CKR_OK;
}



CK_RV Slot::C_Decrypt (CK_BYTE_PTR pEncryptedData, CK_ULONG ulEncryptedDataLen, CK_BYTE_PTR pData, CK_ULONG_PTR pulDataLen)
{
    RSA *RSAKey;
    CK_ULONG longbuffer;
    int longitudDatos;
    ClaveRSA *pObjectRSAKey;
    unsigned char * keyId;
    CK_RV ret;
    
    pObjectRSAKey = (ClaveRSA *)pListaObjetos->buscarObjeto(_hKey);
    if (!pObjectRSAKey){
	LOG_MsgError(LOG_TO, "No se encontro el objeto llave RSA");
	free(_pMechanism);
	_pMechanism = NULL;
	return CKR_FUNCTION_CANCELED;
    }
    
    keyId= pObjectRSAKey->get_keyId();
    
    ret= pClauer->getRSAKey(keyId, &RSAKey, &th_id_handle);
    
    if ( ret == CKR_FUNCTION_FAILED ) {
	free(_pMechanism);
	_pMechanism = NULL;
	return CKR_FUNCTION_CANCELED;
    }
	
    if (!RSAKey){
	LOG_MsgError(LOG_TO, "No se encontro el objeto llave RSA");
	free(_pMechanism);
	_pMechanism = NULL;
	return CKR_FUNCTION_CANCELED;
    }
       
    longbuffer = RSA_size(RSAKey);
    
    if (ulEncryptedDataLen < longbuffer){
	free(_pMechanism);
	_pMechanism = NULL;
	RSA_free(RSAKey);
	return CKR_ENCRYPTED_DATA_LEN_RANGE;
    }
    
    longitudDatos = RSA_private_decrypt(ulEncryptedDataLen, pEncryptedData, pData, RSAKey, RSA_PKCS1_PADDING);
    
    if (longitudDatos < 0){
	free(_pMechanism);
	_pMechanism = NULL;
	RSA_free(RSAKey);
	return CKR_ENCRYPTED_DATA_INVALID;
    }
    if (!pData)
        {
	    *pulDataLen = (CK_ULONG) longitudDatos;
	    RSA_free(RSAKey);
	    return CKR_OK;
        }
    else
        {
	    if (*pulDataLen >= (CK_ULONG) longitudDatos)
                {
		    *pulDataLen = (CK_ULONG) longitudDatos;
		    free(_pMechanism);
		    _pMechanism = NULL;
		    RSA_free(RSAKey);
		    return CKR_OK;
                }
	    else
                {
		    *pulDataLen = (CK_ULONG) longitudDatos;
		    RSA_free(RSAKey);
		    return CKR_BUFFER_TOO_SMALL;
                }
        }
}


CK_RV Slot::C_GetSessionInfo (CK_SESSION_INFO_PTR pInfo)
{
    pInfo-> slotID= 1; // It is an unsigned long int
    
    // slotState Mantains the state of the slot ( logged or not logged )
    // we must say state logged until client try to access private key 
    // then, we must assure user log in
    pInfo-> state= slotState; 
    pInfo-> flags= (CKF_TOKEN_PRESENT | CKF_REMOVABLE_DEVICE);
    return CKR_OK;
}


CK_RV Slot::C_FindObjectsInit (CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount)
{
    CK_ATTRIBUTE_PTR punteroAtributo;
    CK_VOID_PTR punteroAuxiliar;
    ASN1_INTEGER *pasn1_integer = NULL;
    CK_ATTRIBUTE_PTR copiaPlantilla;
    CK_ULONG contador;
    unsigned long valor=0;
    unsigned int i;
    unsigned char *pauxiliar;
    
   
    if (pListaObjetos->estaBusquedaIniciada())
	return CKR_OPERATION_ACTIVE;

    // Copiamos la plantilla ya que puede que haya que hacerle una modificacion
    copiaPlantilla = (CK_ATTRIBUTE_PTR) malloc(ulCount * sizeof(CK_ATTRIBUTE));
    if (!copiaPlantilla)
	return CKR_HOST_MEMORY;
    for (contador = 0; contador < ulCount; contador++)
	{

	    LOG_Debug(LOG_TO, "Buscamos objetos con atributos 0x%x", pTemplate[contador].type);
		LOG_Debug(LOG_TO, "                 Con Valor: 0x%x", *((int *)pTemplate[contador].pValue));
		if (pTemplate[contador].type == CKA_LABEL){
			LOG_Debug(LOG_TO, "Buscamos CKA_LABEL: %s", pTemplate[contador].pValue);
		}
	    copiaPlantilla[contador].type = pTemplate[contador].type;
		
	    copiaPlantilla[contador].ulValueLen = pTemplate[contador].ulValueLen;
	    if (copiaPlantilla[contador].ulValueLen)
		{
		    copiaPlantilla[contador].pValue = malloc(copiaPlantilla[contador].ulValueLen);
		    if (!copiaPlantilla[contador].pValue)
			return CKR_HOST_MEMORY;
		    else
			memcpy(copiaPlantilla[contador].pValue,pTemplate[contador].pValue,copiaPlantilla[contador].ulValueLen);
		}
	    else
		copiaPlantilla[contador].pValue = NULL_PTR;
	}
    /*
    punteroAtributo = buscarEnPlantilla(copiaPlantilla,ulCount,CKA_SERIAL_NUMBER);
    if (punteroAtributo)
	{
	    //
            // This conversión is not necessary now as 
            // browsers encode the serial number as a 
            // der object
            //
            // Actualizado Mayo 2007  	
	    pasn1_integer = ASN1_INTEGER_new();
	    pauxiliar = (unsigned char *)punteroAtributo->pValue;
	    for (i=0;i<punteroAtributo->ulValueLen;i++)
		{
		    valor = 0x100*valor + (unsigned long)pauxiliar[i];
		}
	    ASN1_INTEGER_set(pasn1_integer,valor);
	    punteroAtributo->ulValueLen = i2d_ASN1_INTEGER(pasn1_integer,NULL);
	    free(punteroAtributo->pValue);
	    punteroAtributo->pValue = malloc(punteroAtributo->ulValueLen);
	    if (!punteroAtributo->pValue)
		return CKR_HOST_MEMORY;

	    punteroAuxiliar = punteroAtributo->pValue;
	    i2d_ASN1_INTEGER(pasn1_integer,(unsigned char **)&(punteroAtributo->pValue));
	    punteroAtributo->pValue = punteroAuxiliar;
	    ASN1_INTEGER_free(pasn1_integer);
	}
    */

    pListaObjetos->iniciarBusquedaPlantilla(copiaPlantilla,ulCount);
    // }
    return CKR_OK;
}


CK_RV Slot::C_FindObjects (CK_OBJECT_HANDLE_PTR phObject, CK_ULONG ulMaxObjectCount, CK_ULONG_PTR pulObjectCount)
{
    unsigned long contador;
    CK_OBJECT_HANDLE handle;
    
    contador = 0;
    while(contador < ulMaxObjectCount)
	{
	    // La busqueda hace avanzar el cursor de la lista de objetos
	    // En la siguiente llamada, se continua buscando por donde se quedo
	    handle = pListaObjetos->buscarPlantilla();
	    if(handle)
		{
		    // 		    printf("Encontrado objeto con handle= %d\n", (int)handle);
			LOG_Debug(LOG_TO,"Encontrado objeto con handle= %d", handle);
		    phObject[contador++] = handle;
		}
	    else
		break;
	}
	
    *pulObjectCount = contador;
    return CKR_OK;
}


CK_RV Slot::C_FindObjectsFinal ()
{
    pListaObjetos->finalizarBusquedaPlantilla();
    return CKR_OK;
}

CK_RV Slot::C_GetAttributeValue(CK_OBJECT_HANDLE hObject, CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount)
{
    Objeto *pObjeto;
    CK_ATTRIBUTE_PTR pPlantillaObjeto, pAtributo;
    CK_ULONG numeroAtributos, numeroAtributosVolcados;
    unsigned int contador;
    
    // Comprobamos que el objeto esta en la lista de objetos de la sesion
    pObjeto = pListaObjetos->buscarObjeto(hObject);

    if (pObjeto)
	{
	    // Volcamos los atributos del objeto a una plantilla
	    numeroAtributos = pObjeto->numeroAtributos();
	    pPlantillaObjeto = (CK_ATTRIBUTE_PTR) malloc(numeroAtributos * sizeof(CK_ATTRIBUTE));
	    pObjeto->volcarPlantilla(pPlantillaObjeto,numeroAtributos,numeroAtributosVolcados);
	    
	    // for ( int i=0; i<numeroAtributos; i++ ){
	    //	LOG_Debug(LOG_TO,"Atributo= 0x%02x", pPlantillaObjeto[i].type );	
	    // }
	    
	    contador = 0;
	    while (contador < ulCount)
		{
		    // Buscamos cada atributo de la plantilla que se pasa como parametro 
		    // en la plantilla resultado de volcar el objeto       
		    LOG_Debug(1,"Para el objeto con handle: %d", hObject );
		    LOG_Debug(1,"Tipo de atributo que buscamos: 0x%x", pTemplate[contador].type );
		   
		    pAtributo = buscarEnPlantilla(pPlantillaObjeto,numeroAtributosVolcados,pTemplate[contador].type);
		    if (pAtributo)
			{
			    if( pTemplate[contador].pValue )
				{
				    if(pTemplate[contador].ulValueLen < pAtributo->ulValueLen)
					{
					    destruirPlantilla(pPlantillaObjeto,numeroAtributos);
					    LOG_Debug(LOG_TO,"Devolviendo buffer tamaño pequeño, nuestro tamaño es: %ld", pAtributo->ulValueLen );
					    return CKR_BUFFER_TOO_SMALL;
					}
				    else
					{
					    memcpy( pTemplate[contador].pValue,pAtributo->pValue,pAtributo->ulValueLen );
					    pTemplate[contador].ulValueLen = pAtributo->ulValueLen;
					    LOG_Debug(1,"Encontrado y copiado Atributo 0x%x", pTemplate[contador].type );
					}
				}
			    else	// Solo quiere el tamano
				{
				    LOG_Debug(LOG_TO,"Devolviendo solo el tamaño= %ld",  pAtributo->ulValueLen );				    			    pTemplate[contador].ulValueLen = pAtributo->ulValueLen;
			
				    if ( pAtributo->ulValueLen== -1 ){
					destruirPlantilla(pPlantillaObjeto,numeroAtributos);
					return CKR_ATTRIBUTE_TYPE_INVALID;
				    }

				}
			}
		    else
			{
			    LOG_Msg(LOG_TO, "Atributo inválido" );
			    pTemplate[contador].ulValueLen = (CK_ULONG) -1;
			    destruirPlantilla(pPlantillaObjeto,numeroAtributos);
			    return CKR_ATTRIBUTE_TYPE_INVALID;
			}
		    contador++;
		}
	    destruirPlantilla(pPlantillaObjeto,numeroAtributos);
	    return CKR_OK;
	}
    else
	return CKR_OBJECT_HANDLE_INVALID;
}

    
CK_RV Slot::C_SetAttributeValue(CK_OBJECT_HANDLE hObject, CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount)
{
    
    Objeto *pObjeto;
    CK_RV res;
    
    // Comprobamos que el objeto esta en la lista de objetos de la sesion
    pObjeto = pListaObjetos->buscarObjeto(hObject);
    if (pObjeto){
	res= pObjeto->modificarObjeto(pTemplate,ulCount);
	return CKR_OK;
	//return res;
    }  
    else{
	return CKR_OBJECT_HANDLE_INVALID;
    }
}


CK_RV Slot::C_Login (CK_USER_TYPE userType, CK_CHAR_PTR pPin, CK_ULONG ulPinLen)
{  
    CK_RV res;

    LOG_Msg(LOG_TO, "Antes de C_Login");
    res= pClauer->C_Login(pPin, ulPinLen, pListaObjetos );

    if ( res == CKR_OK ){
	slotState= CKS_RO_USER_FUNCTIONS;	
    }
    else {
	slotState= CKS_RO_PUBLIC_SESSION;
    }

    LOG_Msg(LOG_TO, "Después de C_Login");
    return res;
}


CK_RV Slot::C_Logout ()
{
    slotState= CKS_RW_PUBLIC_SESSION;
    return pClauer->C_Logout();
}

CK_RV Slot::C_SetPIN(CK_SESSION_HANDLE hSession, CK_CHAR_PTR pOldPin, CK_ULONG ulOldLen, CK_CHAR_PTR pNewPin, CK_ULONG ulNewLen)
{
    CK_RV res;

    res = pClauer->C_SetPIN(hSession, pOldPin, ulOldLen, pNewPin, ulNewLen);
    //C_Logout();
    return res;
}


CK_RV Slot::C_CreateObject(
			   CK_SESSION_HANDLE hSession,    /* the session's handle */
			   CK_ATTRIBUTE_PTR  pTemplate,   /* the object's template */
			   CK_ULONG          ulCount,     /* attributes in template */
			   CK_OBJECT_HANDLE_PTR phObject  /* receives new object's handle. */
			   )
{
    return pClauer->C_CreateObject(hSession, pTemplate, ulCount, phObject, pListaObjetos );
}


CK_RV Slot::C_DestroyObject( CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE  hObject ){
	
	if ( pClauer->get_ulSessionCount() != 1 ){
	  return CKR_SESSION_CLOSED;
	}
	if ( ! pClauer->isLogged() ){
		return CKR_SESSION_READ_ONLY;
	}

	return pClauer->C_DestroyObject( hObject, pListaObjetos );

}

CK_RV Slot::C_GenerateKeyPair(
			      CK_SESSION_HANDLE    hSession,                    /* the session's handle */
			      CK_MECHANISM_PTR     pMechanism,                  /* the key gen. mech. */
			      CK_ATTRIBUTE_PTR     pPublicKeyTemplate,          /* pub. attr. template */
			      CK_ULONG             ulPublicKeyAttributeCount,   /* # of pub. attrs. */
			      CK_ATTRIBUTE_PTR     pPrivateKeyTemplate,         /* priv. attr. template */
			      CK_ULONG             ulPrivateKeyAttributeCount,  /* # of priv. attrs. */
			      CK_OBJECT_HANDLE_PTR phPublicKey,                 /* gets pub. key handle */
			      CK_OBJECT_HANDLE_PTR phPrivateKey                 /* gets priv. key handle */
			      )
{

    

    return pClauer->C_GenerateKeyPair( hSession, pMechanism,                  
				       pPublicKeyTemplate, ulPublicKeyAttributeCount,   
				       pPrivateKeyTemplate, ulPrivateKeyAttributeCount,  
				       phPublicKey, phPrivateKey, 
				       pListaObjetos );

}
