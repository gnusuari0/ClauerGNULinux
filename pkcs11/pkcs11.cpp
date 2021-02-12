
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
 * Description: Contains the functions of the main defined functions
 *              at PKCS#11's API. Not implemented functions returns
 *              CKR_FUNCTION_NOT_SUPPORTED.
 *              First we have implemented basic support for signature
 *              and later ciphering. The functionality will be 
 *              increasing.
 * 
 *                         Clauer Team 2006 
 **/               

#include "pkcs11.h"		// CK_FUNCTION_LIST Structure


#include "log.h"

#include "common.h"
#include "slot.h"

#include <LIBRT/libRT.h>

#include <openssl/rsa.h>
#include <openssl/asn1.h>
#include <openssl/x509.h>
#include <openssl/bn.h> 

#ifdef _UNIX
#include <stdlib.h>	// getenv()
#include <stdio.h>      // printf()
#include <string.h>     // memcpy()

#elif defined(WIN32)
#include <windows.h>
#ifdef CLUI
#include <clui.h>
#endif
#else
#error Platform must be defines: _WINDOWS o _UNIX
#endif



// TODO: At log.c we must differenciate between Windows version and Linux version.

#include <stdio.h>

int initialized=0;

// Finally the only global variable we must have is the slot.
Slot * slotClauer;
DWORD lang;

CK_FUNCTION_LIST FunctionList = {
    {2,0},
    &C_Initialize , &C_Finalize, &C_GetInfo, &C_GetFunctionList, &C_GetSlotList, 
    &C_GetSlotInfo, &C_GetTokenInfo, &C_GetMechanismList, &C_GetMechanismInfo,
    &C_InitToken, &C_InitPIN, &C_SetPIN, &C_OpenSession, &C_CloseSession, 
    &C_CloseAllSessions, &C_GetSessionInfo, &C_GetOperationState, &C_SetOperationState,
    &C_Login, &C_Logout, &C_CreateObject, &C_CopyObject, &C_DestroyObject,
    &C_GetObjectSize, &C_GetAttributeValue, &C_SetAttributeValue, &C_FindObjectsInit,
    &C_FindObjects, &C_FindObjectsFinal, &C_EncryptInit, &C_Encrypt, &C_EncryptUpdate,
    &C_EncryptFinal, &C_DecryptInit, &C_Decrypt, &C_DecryptUpdate, &C_DecryptFinal,
    &C_DigestInit, &C_Digest, &C_DigestUpdate, &C_DigestKey, &C_DigestFinal, &C_SignInit,
    &C_Sign, &C_SignUpdate, &C_SignFinal, &C_SignRecoverInit, &C_SignRecover, 
    &C_VerifyInit, &C_Verify, &C_VerifyUpdate, &C_VerifyFinal, &C_VerifyRecoverInit, 
    &C_VerifyRecover, &C_DigestEncryptUpdate, &C_DecryptDigestUpdate, &C_SignEncryptUpdate,
    &C_DecryptVerifyUpdate, &C_GenerateKey, &C_GenerateKeyPair, &C_WrapKey, &C_UnwrapKey,
    &C_DeriveKey, &C_SeedRandom, &C_GenerateRandom, &C_GetFunctionStatus, 
    &C_CancelFunction
};



// C_Initialize

CK_RV C_Initialize(CK_VOID_PTR pReserved)
{
	LOG_Ini(LOG_WHERE_FILE, 1000);
    LOG_Msg(LOG_TO, "Inicializando módulo pkcs11"); 
    lang = IDIOMA_Obtener();
    LOG_Debug(LOG_TO,"Obtenido idioma %d",lang);
 
	/* First initialize  libraries */
    LIBRT_Ini();
    CRYPTO_Ini();
    
	// Create the slot
    switch ( lang ){
   /* case ES_IDCAT:
	slotClauer= new Slot(1,
			     (CK_CHAR *)SLOT_DESCRIPTION_IDCAT_ES,
			     (CK_CHAR *)MANUFACTURER_ID_IDCAT_ES,
			     SLOT_FLAGS);
	break;
    case CA_IDCAT:
	slotClauer= new Slot(1,
			     (CK_CHAR *)SLOT_DESCRIPTION_IDCAT_CA,
			     (CK_CHAR *)MANUFACTURER_ID_IDCAT_CA,
			     SLOT_FLAGS);
	break;
   case EN_IDCAT:
	slotClauer= new Slot(1,
			     (CK_CHAR *)SLOT_DESCRIPTION_IDCAT_EN,
			     (CK_CHAR *)MANUFACTURER_ID_IDCAT_EN,
			     SLOT_FLAGS);
	break;
    case ES_ACCV:
	slotClauer= new Slot(1,
			     (CK_CHAR *)SLOT_DESCRIPTION_ACCV_ES,
			     (CK_CHAR *)MANUFACTURER_ID_ACCV_ES,
			     SLOT_FLAGS);
	break;
    case CA_ACCV:
	slotClauer= new Slot(1,
			     (CK_CHAR *)SLOT_DESCRIPTION_ACCV_CA,
			     (CK_CHAR *)MANUFACTURER_ID_ACCV_CA,
			     SLOT_FLAGS);
	break;
    case EN_ACCV:
	slotClauer= new Slot(1,
			     (CK_CHAR *)SLOT_DESCRIPTION_ACCV_EN,
			     (CK_CHAR *)MANUFACTURER_ID_ACCV_EN,
			     SLOT_FLAGS);
	break;
    case ES_COITAV:
	slotClauer= new Slot(1,
			     (CK_CHAR *)SLOT_DESCRIPTION_COITAVC_ES,
			     (CK_CHAR *)MANUFACTURER_ID_COITAVC_ES,
			     SLOT_FLAGS);
	break;
    case CA_COITAV:
	slotClauer= new Slot(1,
			     (CK_CHAR *)SLOT_DESCRIPTION_COITAVC_CA,
			     (CK_CHAR *)MANUFACTURER_ID_COITAVC_CA,
			     SLOT_FLAGS);
	break;
    case EN_COITAV:
	slotClauer= new Slot(1,
			     (CK_CHAR *)SLOT_DESCRIPTION_COITAVC_EN,
			     (CK_CHAR *)MANUFACTURER_ID_COITAVC_EN,
			     SLOT_FLAGS);
	break;*/

    case ES:
	slotClauer= new Slot(1,
			     (CK_CHAR *)SLOT_DESCRIPTION_ES,
			     (CK_CHAR *)MANUFACTURER_ID_ES,
			     SLOT_FLAGS);
	break;
    case CA:
	slotClauer= new Slot(1,
			     (CK_CHAR *)SLOT_DESCRIPTION_CA,
			     (CK_CHAR *)MANUFACTURER_ID_CA,
			     SLOT_FLAGS);
	break;
    default:
	slotClauer= new Slot(1,
			     (CK_CHAR *)SLOT_DESCRIPTION_EN,
			     (CK_CHAR *)MANUFACTURER_ID_EN,
			     SLOT_FLAGS);
    }

    return CKR_OK; 
}

// C_Finalize
CK_RV C_Finalize(CK_VOID_PTR pReserved)
{
    // Perform exiting operations.     
    LOG_Msg(LOG_TO,"Deleting clauer Slot"); 

    delete slotClauer;

    /* And finalize libraries */
    LIBRT_Fin();
    CRYPTO_Fin();

    return CKR_OK;
}


// C_GetInfo
CK_RV C_GetInfo(CK_INFO_PTR pInfo)
{ 
      
    LOG_Msg(LOG_TO,"Obtaining info about the cryptographic module"); 
    
    pInfo->cryptokiVersion.major = CRIPTOKI_VERSION_MAJOR;
    pInfo->cryptokiVersion.minor = CRIPTOKI_VERSION_MINOR;
    


    switch ( lang ){
    /*case ES_IDCAT:
	padding( (char *) pInfo->manufacturerID, MANUFACTURER_ID_IDCAT_ES,32);
	padding( (char *) pInfo->libraryDescription, LIBRARY_DESCRIPTION_IDCAT_ES, 32);
	break;
    case CA_IDCAT:
	padding( (char *) pInfo->manufacturerID, MANUFACTURER_ID_IDCAT_CA,32);
	padding( (char *) pInfo->libraryDescription, LIBRARY_DESCRIPTION_IDCAT_CA, 32);
	break;
    case EN_IDCAT:
	padding( (char *) pInfo->manufacturerID, MANUFACTURER_ID_IDCAT_EN,32);
	padding( (char *) pInfo->libraryDescription, LIBRARY_DESCRIPTION_IDCAT_EN, 32);
	break;
    case ES_ACCV:
	padding( (char *) pInfo->manufacturerID, MANUFACTURER_ID_ACCV_ES,32);
	padding( (char *) pInfo->libraryDescription, LIBRARY_DESCRIPTION_ACCV_ES, 32);
	break;
    case CA_ACCV:
	padding( (char *) pInfo->manufacturerID, MANUFACTURER_ID_ACCV_CA,32);
	padding( (char *) pInfo->libraryDescription, LIBRARY_DESCRIPTION_ACCV_CA, 32);
	break;
    case EN_ACCV:
	padding( (char *) pInfo->manufacturerID, MANUFACTURER_ID_ACCV_EN,32);
	padding( (char *) pInfo->libraryDescription, LIBRARY_DESCRIPTION_ACCV_EN, 32);
	break;
    case ES_COITAV:
	padding( (char *) pInfo->manufacturerID, MANUFACTURER_ID_COITAVC_ES,32);
	padding( (char *) pInfo->libraryDescription, LIBRARY_DESCRIPTION_COITAVC_ES, 32);
	break;
    case CA_COITAV:
	padding( (char *) pInfo->manufacturerID, MANUFACTURER_ID_ACCV_CA,32);
	padding( (char *) pInfo->libraryDescription, LIBRARY_DESCRIPTION_COITAVC_CA, 32);
	break;
    case EN_COITAV:
	padding( (char *) pInfo->manufacturerID, MANUFACTURER_ID_ACCV_EN,32);
	padding( (char *) pInfo->libraryDescription, LIBRARY_DESCRIPTION_COITAVC_EN, 32);
	break;*/
    case ES:
	padding( (char *) pInfo->manufacturerID, MANUFACTURER_ID_ES,32);
	padding( (char *) pInfo->libraryDescription, LIBRARY_DESCRIPTION_ES, 32);
	break;
    case CA:
	padding( (char *) pInfo->manufacturerID, MANUFACTURER_ID_CA,32);
	padding( (char *) pInfo->libraryDescription, LIBRARY_DESCRIPTION_CA, 32);
	break;
    case EN:
	padding( (char *) pInfo->manufacturerID, MANUFACTURER_ID_EN,32);
	padding( (char *) pInfo->libraryDescription, LIBRARY_DESCRIPTION_EN, 32);
	break;
    }

    pInfo->flags = (CKF_TOKEN_PRESENT | CKF_REMOVABLE_DEVICE);
    pInfo->libraryVersion.major = LIBRARY_VERSION_MAJOR;
    pInfo->libraryVersion.minor = LIBRARY_VERSION_MINOR; 
    
    return CKR_OK;
}


// C_GetFunctionList
CK_RV C_GetFunctionList(CK_FUNCTION_LIST_PTR_PTR ppFunctionList)
{
  
    // Here we pass the function list to the application
      
    LOG_Msg(LOG_TO,"Obtaining the function list "); 
    
    *ppFunctionList= &FunctionList;
    
    return CKR_OK;
}

// C_GetSlotList
CK_RV C_GetSlotList(
		    CK_BBOOL       tokenPresent,
		    CK_SLOT_ID_PTR pSlotList,
		    CK_ULONG_PTR   pulCount
		    )
{

    //char aux[512];
    //TODO: Here we must get the clauers switched on the system 
    //      And return an appropiate structure for tell it to 
    //      the application.
  
    
    //snprintf(aux, 512, "pulCount=%ld tokenPresent=%d",*pulCount, tokenPresent); 
    //LOG_Debug(LOG_TO,"%s", aux); 

    if ( pSlotList == NULL_PTR ){

		if ( tokenPresent == TRUE ) {
			if ( slotClauer->isTokenPresent() ){
			*pulCount=1;
	    }
	    else{
		*pulCount=0;
	    }
	}
	else{
	    *pulCount=1;
	}
    }
    else{
	//tokenPresent is passed from app, it tells if the list must return only present 
	// tokens (TRUE) or all (FALSE)

	if ( *pulCount < 1 )
	    return CKR_BUFFER_TOO_SMALL;
	
	pSlotList[0]=1;
    }
    return CKR_OK;
}

// C_GetSlotInfo
CK_RV C_GetSlotInfo(
		    CK_SLOT_ID       slotID,
		    CK_SLOT_INFO_PTR pInfo
		    )
{
 
    LOG_Debug(LOG_TO,"slotID= %d", slotID); 

    return slotClauer->C_GetSlotInfo(pInfo);

    //return CKR_SLOT_ID_INVALID;
}


// C_GetTokenInfo
CK_RV C_GetTokenInfo(
		     CK_SLOT_ID        slotID,
		     CK_TOKEN_INFO_PTR pInfo
		     )
{
    
    LOG_Debug(LOG_TO,"slotID=%d",slotID); 

    return slotClauer->C_GetTokenInfo(pInfo);
}


// C_GetMechanismList
CK_RV C_GetMechanismList(
			 CK_SLOT_ID            slotID,
			 CK_MECHANISM_TYPE_PTR pMechanismList,
			 CK_ULONG_PTR          pulCount
			 )
{
    char aux[512];

    snprintf(aux, 512, "slotID=%ld, ... , pulCount=%ld ",slotID, *pulCount);
    LOG_Debug(LOG_TO,"%s", aux);

    return slotClauer->C_GetMechanismList (pMechanismList,pulCount);

}

// C_GetMechanismInfo
CK_RV C_GetMechanismInfo(CK_SLOT_ID slotID, CK_MECHANISM_TYPE type, CK_MECHANISM_INFO_PTR pInfo)
{
    
    LOG_Debug(LOG_TO,"slotID= %ld ... ",slotID); 

    return slotClauer->C_GetMechanismInfo(type, pInfo);
}

// C_InitToken (no soportada)
CK_RV C_InitToken(
		  CK_SLOT_ID     slotID,    /* ID of the token's slot */
		  CK_CHAR_PTR    pPin,      /* the SO's initial PIN */
		  CK_ULONG       ulPinLen,  /* length in bytes of the PIN */
		  CK_CHAR_PTR    pLabel     /* 32-byte token label (blank padded) */
		  )
{
  
    LOG_Msg(LOG_TO,"C_InitToken Call (Not supported)"); 

    return CKR_FUNCTION_NOT_SUPPORTED;
}

// C_InitPIN (no soportada)
CK_RV C_InitPIN(
		CK_SESSION_HANDLE hSession,  /* the session's handle */
		CK_CHAR_PTR       pPin,      /* the normal user's PIN */
		CK_ULONG          ulPinLen   /* length in bytes of the PIN */
		)
{
  
    LOG_Msg(LOG_TO,"C_InitPIN Call (Not supported)"); 

    return CKR_FUNCTION_NOT_SUPPORTED;
}

// C_SetPIN (no soportada)
CK_RV C_SetPIN(
	       CK_SESSION_HANDLE hSession,  /* the session's handle */
	       CK_CHAR_PTR       pOldPin,   /* the old PIN */
	       CK_ULONG          ulOldLen,  /* length of the old PIN */
	       CK_CHAR_PTR       pNewPin,   /* the new PIN */
	       CK_ULONG          ulNewLen   /* length of the new PIN */
	       )
{
  
    LOG_Msg(LOG_TO,"C_SetPIN Call "); 

    return slotClauer->C_SetPIN( hSession, pOldPin, ulOldLen, pNewPin, ulNewLen );
  
}


// C_OpenSession
CK_RV C_OpenSession(
		    CK_SLOT_ID            slotID,        /* the slot's ID */
		    CK_FLAGS              flags,         /* defined in CK_SESSION_INFO */
		    CK_VOID_PTR           pApplication,  /* pointer passed to callback */
		    CK_NOTIFY             Notify,        /* notification callback function */
		    CK_SESSION_HANDLE_PTR phSession      /* receives new session handle */
		    )
{
    
    LOG_Msg(LOG_TO,"C_OpenSession Call"); 

    return slotClauer->C_OpenSession(flags, pApplication, Notify, phSession);
}

// C_CloseSession
CK_RV C_CloseSession(CK_SESSION_HANDLE hSession)
{
    

    LOG_Debug(LOG_TO,"hSession= %d",hSession); 

    if ( hSession != 1 )
		return CKR_SESSION_HANDLE_INVALID;
    else 
		return slotClauer->C_CloseSession(hSession);
}


// C_CloseAllSessions
CK_RV C_CloseAllSessions(CK_SLOT_ID slotID)
{

    LOG_Debug(LOG_TO,"slotID: %d", slotID); 

    return slotClauer->C_CloseAllSessions();
}


// C_GetSessionInfo
CK_RV C_GetSessionInfo(
		       CK_SESSION_HANDLE   hSession,
		       CK_SESSION_INFO_PTR pInfo
		       )
{
  
    LOG_Debug(LOG_TO,"hSession= %d", hSession); 

    return slotClauer->C_GetSessionInfo(pInfo);
}


// C_GetOperationState (no soportada)
CK_RV C_GetOperationState(
			  CK_SESSION_HANDLE hSession,             /* the session's handle */
			  CK_BYTE_PTR       pOperationState,      /* location receiving state */
			  CK_ULONG_PTR      pulOperationStateLen  /* location receiving state length */
			  )
{
    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_SetOperationState (no soportada)
CK_RV C_SetOperationState(
			  CK_SESSION_HANDLE hSession,            /* the session's handle */
			  CK_BYTE_PTR      pOperationState,      /* the location holding the state */
			  CK_ULONG         ulOperationStateLen,  /* location holding state length */
			  CK_OBJECT_HANDLE hEncryptionKey,       /* handle of en/decryption key */
			  CK_OBJECT_HANDLE hAuthenticationKey    /* handle of sign/verify key */
			  )
{
    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_Login
CK_RV C_Login(
	      CK_SESSION_HANDLE hSession,
	      CK_USER_TYPE      userType,
	      CK_CHAR_PTR       pPin,
	      CK_ULONG          ulPinLen
	      )
{
  

    LOG_Debug(LOG_TO,"(hSession= %d, ...)",hSession); 

    return slotClauer->C_Login(userType,pPin,ulPinLen);
}


// C_Logout
CK_RV C_Logout(CK_SESSION_HANDLE hSession)
{
    // TODO: We must exit form session
  
    LOG_Debug(LOG_TO,"C_Logout(hSession=%d) Call ",hSession); 

    return slotClauer->C_Logout();
}

// C_CreateObject
CK_RV C_CreateObject(
		     CK_SESSION_HANDLE hSession,    /* the session's handle */
		     CK_ATTRIBUTE_PTR  pTemplate,   /* the object's template */
		     CK_ULONG          ulCount,     /* attributes in template */
		     CK_OBJECT_HANDLE_PTR phObject  /* receives new object's handle. */
		     )
{
    
    LOG_Debug(LOG_TO,"C_CreateObject(hSession=%d,...) Call ",hSession); 
    return slotClauer->C_CreateObject(hSession, pTemplate, ulCount, phObject);
}


// C_CopyObject (no soportada)
CK_RV C_CopyObject(
		   CK_SESSION_HANDLE    hSession,    /* the session's handle */
		   CK_OBJECT_HANDLE     hObject,     /* the object's handle */
		   CK_ATTRIBUTE_PTR     pTemplate,   /* template for new object */
		   CK_ULONG             ulCount,     /* attributes in template */
		   CK_OBJECT_HANDLE_PTR phNewObject  /* receives handle of copy */
		   )
{
    char aux[512];
        
    snprintf(aux, 512, "C_CopyObject(hSession=%ld, hObject=%ld, ..., ulCount=%ld, ...) Call ",hSession,hObject,ulCount);
  
    LOG_Debug(LOG_TO, "%s", aux);
    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_DestroyObject
CK_RV C_DestroyObject(
		      CK_SESSION_HANDLE hSession,  /* the session's handle */
		      CK_OBJECT_HANDLE  hObject    /* the object's handle */
		      )
{
    char aux[512];

    /* That function should be implemented just in case of CreateObject failure */
    snprintf(aux,512,"C_DestroyObject(hSession=%ld, hObject=%ld) Call ",hSession,hObject); 
    LOG_Debug(LOG_TO,"%s",aux);
    //return CKR_FUNCTION_NOT_SUPPORTED;
	return slotClauer->C_DestroyObject(hSession, hObject);
}


// C_GetObjectSize (no soportada)
CK_RV C_GetObjectSize(
		      CK_SESSION_HANDLE hSession,  /* the session's handle */
		      CK_OBJECT_HANDLE  hObject,   /* the object's handle */
		      CK_ULONG_PTR      pulSize    /* receives size of object */
		      )
{
    char aux[512];

    snprintf(aux,512,"C_GetObjectSize(hSession=%ld, hObject=%ld, ...) Call ",hSession,hObject); 
    LOG_Debug(LOG_TO,"%s",aux);
    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_GetAttributeValue
CK_RV C_GetAttributeValue(
			  CK_SESSION_HANDLE hSession,
			  CK_OBJECT_HANDLE  hObject,
			  CK_ATTRIBUTE_PTR  pTemplate,
			  CK_ULONG          ulCount
			  )
{
    char aux[512];
    snprintf(aux,512,"hSession= %ld, hObject= %ld, ..., ulCount= %ld ", hSession, hObject, ulCount); 
    LOG_Debug(LOG_TO,"%s",aux);
    return slotClauer->C_GetAttributeValue(hObject, pTemplate, ulCount); 
}


// C_SetAttributeValue
CK_RV C_SetAttributeValue(
			  CK_SESSION_HANDLE hSession,
			  CK_OBJECT_HANDLE  hObject,
			  CK_ATTRIBUTE_PTR  pTemplate,
			  CK_ULONG          ulCount
			  )
{
    char aux[512];
    snprintf(aux,512,"C_SetAttributeValue(hSession=%ld, hObject=%ld, ..., ulCount=%ld) Call ",hSession,hObject,ulCount);   
    LOG_Debug(LOG_TO,"%s",aux);
    return CKR_OK;
    
    // Be careful with this function it's been detected some code snippets that doesn't work with
    // it, in addition, on a keyPairGeneration call, applications like firefox need to do some 
    // SetAttributeCall that could be problematic if the objects are marked as not modifiable.

    
    // TODO: The same as the last function.
    //	Sesion *pSesion;
    // Verificamos que la sesion esta abierta
    //	pSesion = pUnModuloCriptografico->get_listaSesiones()->buscarSesion(hSession);
    //	if (pSesion)
    //return slotClauer->C_SetAttributeValue(hObject,pTemplate,ulCount);
    //	else
    //return CKR_SESSION_CLOSED;
}


// C_FindObjectsInit
CK_RV C_FindObjectsInit(
			CK_SESSION_HANDLE hSession,
			CK_ATTRIBUTE_PTR  pTemplate,
			CK_ULONG          ulCount
			)
{

    char aux[512];
	
    snprintf(aux,512,"(hSession=%ld, ..., ulCount=%ld) ",hSession, ulCount); 
    LOG_Debug(LOG_TO,"%s",aux);

    return slotClauer->C_FindObjectsInit(pTemplate, ulCount);
}


// C_FindObjects
CK_RV C_FindObjects(
		    CK_SESSION_HANDLE    hSession,          /* the session's handle */
		    CK_OBJECT_HANDLE_PTR phObject,          /* receives object handle array */
		    CK_ULONG             ulMaxObjectCount,  /* max handles to be returned */
		    CK_ULONG_PTR         pulObjectCount     /* actual number returned */
		    )
{
    char aux[512];
	
    snprintf(aux,512,"C_FindObjects(hSession=%ld, ..., ulMaxObjectCount=%ld, ...) Call ",hSession, ulMaxObjectCount); 
  
    LOG_Debug(LOG_TO,"%s",aux);
    return slotClauer->C_FindObjects(phObject, ulMaxObjectCount, pulObjectCount);
}


// C_FindObjectsFinal
CK_RV C_FindObjectsFinal(CK_SESSION_HANDLE hSession)
{
  
    LOG_Debug(LOG_TO,"C_FindObjectsFinal(hSession=%ld) Call ",hSession); 
    return slotClauer->C_FindObjectsFinal();
}


// C_EncryptInit (no soportada)
CK_RV C_EncryptInit(
		    CK_SESSION_HANDLE hSession,    /* the session's handle */
		    CK_MECHANISM_PTR  pMechanism,  /* the encryption mechanism */
		    CK_OBJECT_HANDLE  hKey         /* handle of encryption key */
		    )
{
    // TODO: Not supported at first version but it must be supported.
    char aux[512];

    snprintf(aux,512,"C_EncryptInit(hSession=%ld, ..., hkey=%ld) Call ",hSession, hKey); 
    LOG_Debug(LOG_TO,aux,512);
    
    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_Encrypt (no soportada)
CK_RV C_Encrypt(
		CK_SESSION_HANDLE hSession,            /* the session's handle */
		CK_BYTE_PTR       pData,               /* the plaintext data */
		CK_ULONG          ulDataLen,           /* bytes of plaintext data */
		CK_BYTE_PTR       pEncryptedData,      /* receives encrypted data */
		CK_ULONG_PTR      pulEncryptedDataLen  /* receives encrypted byte count */
		)
{
    // TODO: Not supported at first version but it must be supported.
    char aux[512];

    snprintf(aux,512,"C_EncryptInit(hSession=%ld, ..., ulDataLen=%ld, ...) Call ",hSession, ulDataLen); 
    LOG_Debug(LOG_TO,"%s",aux);

    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_EncryptUpdate (no soportada)
CK_RV C_EncryptUpdate(
		      CK_SESSION_HANDLE hSession,           /* the session's handle */
		      CK_BYTE_PTR       pPart,              /* the plaintext data */
		      CK_ULONG          ulPartLen,          /* bytes of plaintext data */
		      CK_BYTE_PTR       pEncryptedPart,     /* receives encrypted data */
		      CK_ULONG_PTR      pulEncryptedPartLen /* receives encrypted byte count */
		      )
{

    char aux[512];

    snprintf(aux,512,"C_EncryptUpdate(hSession=%ld, ..., ulPartLen=%ld) Call ",hSession, ulPartLen); 
    LOG_Debug(LOG_TO,"%s",aux);

    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_EncryptFinal (no soportada)
CK_RV C_EncryptFinal(
		     CK_SESSION_HANDLE hSession,                /* the session's handle */
		     CK_BYTE_PTR       pLastEncryptedPart,      /* receives encrypted last part */
		     CK_ULONG_PTR      pulLastEncryptedPartLen  /* receives byte count */
		     )
{
  

    LOG_Debug(LOG_TO,"C_EncryptFinal(hSession=%ld, ...) Call ",hSession); 
    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_DecryptInit
CK_RV C_DecryptInit(
		    CK_SESSION_HANDLE hSession,
		    CK_MECHANISM_PTR  pMechanism,
		    CK_OBJECT_HANDLE  hKey
		    )
{

    char aux[512];

    snprintf(aux,512,"C_DecryptInit(hSession=%ld, ..., hKey=%ld) Call ",hSession,hKey); 
    LOG_Debug(LOG_TO,"%s",aux);

    return slotClauer->C_DecryptInit(pMechanism, hKey);
}


// C_Decrypt
CK_RV C_Decrypt(
		CK_SESSION_HANDLE hSession,             /* Session to use for decryption     */
		CK_BYTE_PTR       pEncryptedData,       /* Buffer with the encrypted data    */
		CK_ULONG          ulEncryptedDataLen,   /* Size of the encrypted data buffer */
		CK_BYTE_PTR       pData,                /* Pointer to plain data buffer      */
		CK_ULONG_PTR      pulDataLen            /* Length to plain data len          */
		)
{
  
    char aux[512];

    snprintf(aux,512,"C_Decrypt(hSession=%ld, ..., ulEncryptedDataLen=%ld, ...) Call ",hSession,ulEncryptedDataLen); 
    LOG_Debug(LOG_TO,"%s",aux);

    return slotClauer->C_Decrypt( pEncryptedData, ulEncryptedDataLen, pData, pulDataLen );
}


// C_DecryptUpdate (no soportada)
CK_RV C_DecryptUpdate(
		      CK_SESSION_HANDLE hSession,            /* the session's handle */
		      CK_BYTE_PTR       pEncryptedPart,      /* input encrypted data */
		      CK_ULONG          ulEncryptedPartLen,  /* count of bytes of input */
		      CK_BYTE_PTR       pPart,               /* receives decrypted output */
		      CK_ULONG_PTR      pulPartLen           /* receives decrypted byte count */
		      )
{
		
    char aux[512];

    snprintf(aux,512,"C_DecryptUpdate(hSession=%ld, ..., ulEncryptedPartLen=%ld, ...) Call ",hSession,ulEncryptedPartLen); 
    LOG_Debug(LOG_TO,"%s",aux);

    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_DecryptFinal (no soportada)
CK_RV C_DecryptFinal(
		     CK_SESSION_HANDLE hSession,       /* the session's handle */
		     CK_BYTE_PTR       pLastPart,      /* receives decrypted output */
		     CK_ULONG_PTR      pulLastPartLen  /* receives decrypted byte count */
		     )
{
  
    LOG_Debug(LOG_TO,"C_DecryptFinal(hSession=%ld, ...) Call ",hSession); 
    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_DigestInit (no soportada)
CK_RV C_DigestInit(
		   CK_SESSION_HANDLE hSession,   /* the session's handle */
		   CK_MECHANISM_PTR  pMechanism  /* the digesting mechanism */
		   )
{
    // TODO: No soportada en la version inicial, deberiamos soportarla.
  
    LOG_Debug(LOG_TO,"C_DigestInit(hSession=%ld, ...) Call ",hSession); 
    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_Digest (no soportada)
CK_RV C_Digest(
	       CK_SESSION_HANDLE hSession,     /* the session's handle */
	       CK_BYTE_PTR       pData,        /* data to be digested */
	       CK_ULONG          ulDataLen,    /* bytes of data to be digested */
	       CK_BYTE_PTR       pDigest,      /* receives the message digest */
	       CK_ULONG_PTR      pulDigestLen  /* receives byte length of digest */
	       )
{

    char aux[512];

    snprintf(aux,512,"C_Digest(hSession=%ld, ..., ulDataLen=%ld, ...) Call ",hSession, ulDataLen); 
    LOG_Debug(LOG_TO,"%s",aux);

    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_DigestUpdate (no soportada)
CK_RV C_DigestUpdate(
		     CK_SESSION_HANDLE hSession,  /* the session's handle */
		     CK_BYTE_PTR       pPart,     /* data to be digested */
		     CK_ULONG          ulPartLen  /* bytes of data to be digested */
		     )
{
    
    char aux[512];

    snprintf(aux,512,"C_DigestUpdate(hSession=%ld, ..., ulPartLen=%ld, ...) Call ",hSession, ulPartLen); 
    LOG_Debug(LOG_TO,"%s",aux);

    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_DigestKey (no soportada)
CK_RV C_DigestKey(
		  CK_SESSION_HANDLE hSession,  /* the session's handle */
		  CK_OBJECT_HANDLE  hKey       /* handle of secret key to digest */
		  )
{
    // TODO: We could support that function to
  
    char aux[512];

    snprintf(aux, 512, "C_DigestKey(hSession=%ld, hkey=%ld, ...) Call ",hSession, hKey); 
    LOG_Debug(LOG_TO,"%s",aux);

    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_DigestFinal (no soportada)
CK_RV C_DigestFinal(
		    CK_SESSION_HANDLE hSession,     /* the session's handle */
		    CK_BYTE_PTR       pDigest,      /* receives the message digest */
		    CK_ULONG_PTR      pulDigestLen  /* receives byte count of digest */
		    )
{
  
    char aux[512];

    snprintf(aux, 512, "C_DigestFinal(hSession=%ld, ...) Call ",hSession); 
    LOG_Debug(LOG_TO,"%s",aux);

    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_SignInit
CK_RV C_SignInit(
		 CK_SESSION_HANDLE hSession,
		 CK_MECHANISM_PTR  pMechanism,
		 CK_OBJECT_HANDLE  hKey
		 )
{
  
    // TODO: The next three functions will be used to i.e. web authentication
    //       it is a must to implemente them.
  
    char aux[512];

    snprintf(aux, 512, "C_SignInit(hSession=%ld, ...,hkey=%ld) Call ",hSession,hKey); 
    LOG_Debug(LOG_TO,"%s",aux);

    return slotClauer->C_SignInit(pMechanism, hKey);
}


// C_Sign
CK_RV C_Sign(
	     CK_SESSION_HANDLE hSession,        /* Handle of the session to be used    */
	     CK_BYTE_PTR       pData,           /* The data buffer to be signed        */
	     CK_ULONG          ulDataLen,       /* The size of the buffer to be signed */
	     CK_BYTE_PTR       pSignature,      /* The signed data buffer              */
	     CK_ULONG_PTR      pulSignatureLen  /* Size of the signed data buffer      */
	     )
{

    char aux[512];

    snprintf(aux, 512, "C_Sign(hSession=%ld, ...,ulDataLen=%ld, ...) Call ",hSession,ulDataLen); 
    LOG_Debug(LOG_TO,"%s",aux);

    return slotClauer->C_Sign( pData, ulDataLen, pSignature, pulSignatureLen );
}


// C_SignUpdate (soportada)
CK_RV C_SignUpdate(
		   CK_SESSION_HANDLE hSession,  /* the session's handle           */
		   CK_BYTE_PTR       pPart,     /* the data (digest) to be signed */
		   CK_ULONG          ulPartLen  /* count of bytes to be signed    */
		   )
{
    // TODO: Not supported initialy.
	
    char aux[512];

    snprintf(aux, 512, "C_SignUpdate(hSession=%ld, ...,ulPartLen=%ld) Call ",hSession,ulPartLen); 
    LOG_Debug(LOG_TO,"%s",aux);

    //return CKR_FUNCTION_NOT_SUPPORTED;
	return slotClauer->C_SignUpdate(pPart, ulPartLen);
}


// C_SignFinal (no soportada)
CK_RV C_SignFinal(
		  CK_SESSION_HANDLE hSession,        /* the session's handle */
		  CK_BYTE_PTR       pSignature,      /* receives the signature */
		  CK_ULONG_PTR      pulSignatureLen  /* receives byte count of signature */
		  )
{
    // TODO: Not supported initialy.
  
    char aux[512];

    snprintf(aux, 512, "C_SignFinal(hSession=%ld, ...) Call ",hSession); 
    LOG_Debug(LOG_TO,"%s",aux);

    //return CKR_FUNCTION_NOT_SUPPORTED;
	return slotClauer->C_SignFinal(pSignature, pulSignatureLen);
}


// C_SignRecoverInit (no soportada)
CK_RV C_SignRecoverInit(
			CK_SESSION_HANDLE hSession,   /* the session's handle */
			CK_MECHANISM_PTR  pMechanism, /* the signature mechanism */
			CK_OBJECT_HANDLE  hKey        /* handle of the signature key */
			)
{
    // TODO: Not supported initialy.
    
    char aux[512];

    snprintf(aux, 512, "C_SignRecoverInit(hSession=%ld, ..., hkey=%ld) Call ",hSession,hKey); 
    LOG_Debug(LOG_TO,"%s",aux);

    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_SignRecover (no soportada)
CK_RV C_SignRecover(
		    CK_SESSION_HANDLE hSession,        /* the session's handle */
		    CK_BYTE_PTR       pData,           /* the data (digest) to be signed */
		    CK_ULONG          ulDataLen,       /* count of bytes to be signed */
		    CK_BYTE_PTR       pSignature,      /* receives the signature */
		    CK_ULONG_PTR      pulSignatureLen  /* receives byte count of signature */
		    )
{
    // TODO: Not supported initialy.
    char aux[512];

    snprintf(aux, 512, "C_SignRecover(hSession=%ld, ..., ulDataLen=%ld, ...) Call ",hSession,ulDataLen); 
    LOG_Debug(LOG_TO,"%s",aux);

    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_VerifyInit (no soportada)
CK_RV C_VerifyInit(
		   CK_SESSION_HANDLE hSession,    /* the session's handle */
		   CK_MECHANISM_PTR  pMechanism,  /* the verification mechanism */
		   CK_OBJECT_HANDLE  hKey         /* handle of the verification key */ 
		   )
{
    // TODO: Not supported initialy.
   
    char aux[512];

    snprintf(aux,512,"C_VerifyInit(hSession=%ld, ..., hkey=%ld) Call ",hSession,hKey); 
    LOG_Debug(LOG_TO,"%s",aux);

    return slotClauer->C_VerifyInit(hSession, pMechanism, hKey); //CKR_FUNCTION_NOT_SUPPORTED;
}


// C_Verify (no soportada)
CK_RV C_Verify(
	       CK_SESSION_HANDLE hSession,       /* the session's handle */
	       CK_BYTE_PTR       pData,          /* plaintext data (digest) to compare */
	       CK_ULONG          ulDataLen,      /* length of data (digest) in bytes */
	       CK_BYTE_PTR       pSignature,     /* the signature to be verified */
	       CK_ULONG          ulSignatureLen  /* count of bytes of signature */
	       )
{
    // TODO: Not supported initialy.
    
    char aux[512];

    snprintf(aux,512,"C_Verify(hSession=%ld, ..., ulDataLen=%ld, ...) Call ",hSession,ulDataLen); 
    LOG_Debug(LOG_TO,"%s",aux);

    return  slotClauer->C_Verify(hSession, pData, ulDataLen, pSignature, ulSignatureLen); //CKR_OK;//CKR_FUNCTION_NOT_SUPPORTED;
}


// C_VerifyUpdate (no soportada)
CK_RV C_VerifyUpdate(
		     CK_SESSION_HANDLE hSession,  /* the session's handle */
		     CK_BYTE_PTR       pPart,     /* plaintext data (digest) to compare */
		     CK_ULONG          ulPartLen  /* length of data (digest) in bytes */
		     )
{
    // TODO: Not supported initialy.
  
    LOG_Debug(LOG_TO,"C_VerifyUpdate(hSession=%ld, ...) Call ",hSession); 
    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_VerifyFinal (no soportada)
CK_RV C_VerifyFinal(
		    CK_SESSION_HANDLE hSession,       /* the session's handle */
		    CK_BYTE_PTR       pSignature,     /* the signature to be verified */
		    CK_ULONG          ulSignatureLen  /* count of bytes of signature */
		    )
{
    // TODO: Not supported initialy.
    char aux[512];

    snprintf(aux,512,"C_VerifyFinal(hSession=%ld, ..., ulSignatureLen=%ld) Call ",hSession,ulSignatureLen); 

    LOG_Debug(LOG_TO,"%s",aux);
    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_VerifyRecoverInit (no soportada)
CK_RV C_VerifyRecoverInit(
			  CK_SESSION_HANDLE hSession,    /* the session's handle */
			  CK_MECHANISM_PTR  pMechanism,  /* the verification mechanism */
			  CK_OBJECT_HANDLE  hKey         /* handle of the verification key */
			  )
{
    // TODO: Not supported initialy.
    char aux[512];

    snprintf(aux, 512, "C_VerifyRecoverInit(hSession=%ld, ..., hkey=%ld) Call ",hSession,hKey);   
    LOG_Debug(LOG_TO,"%s",aux);

    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_VerifyRecover (no soportada)
CK_RV C_VerifyRecover(
		      CK_SESSION_HANDLE hSession,        /* the session's handle */
		      CK_BYTE_PTR       pSignature,      /* the signature to be verified */
		      CK_ULONG          ulSignatureLen,  /* count of bytes of signature */
		      CK_BYTE_PTR       pData,           /* receives decrypted data (digest) */
		      CK_ULONG_PTR      pulDataLen       /* receives byte count of data */
		      )
{

    // TODO: Not supported initialy.
    char aux[512];

    snprintf(aux, 512, "C_VerifyRecover(hSession=%ld, ..., ulSignatureLen=%ld, ...) Call ",hSession,ulSignatureLen); 
    LOG_Debug(LOG_TO, "%s", aux);

    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_DigestEncryptUpdate (no soportada)
CK_RV C_DigestEncryptUpdate(
			    CK_SESSION_HANDLE hSession,            /* the session's handle */
			    CK_BYTE_PTR       pPart,               /* the plaintext data */
			    CK_ULONG          ulPartLen,           /* bytes of plaintext data */
			    CK_BYTE_PTR       pEncryptedPart,      /* receives encrypted data */
			    CK_ULONG_PTR      pulEncryptedPartLen  /* receives encrypted byte count */
			    )
{
    // TODO: Not supported initialy.
    char aux[512];

    snprintf(aux, 512, "C_DigestEncryptUpdate(hSession=%ld, ..., ulPartLen=%ld, ...) Call ",hSession,ulPartLen); 
    LOG_Debug(LOG_TO,"%s",aux);

    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_DecryptDigestUpdate (no soportada)
CK_RV C_DecryptDigestUpdate(
			    CK_SESSION_HANDLE hSession,            /* the session's handle */
			    CK_BYTE_PTR       pEncryptedPart,      /* input encrypted data */
			    CK_ULONG          ulEncryptedPartLen,  /* count of bytes of input */
			    CK_BYTE_PTR       pPart,               /* receives decrypted output */
			    CK_ULONG_PTR      pulPartLen           /* receives decrypted byte count */
			    )
{
    // TODO: Not supported initialy.
  
    char aux[512];
	
    snprintf(aux, 512, "C_DecryptDigestUdate(hSession=%ld, ..., ulEncryptedPartLen=%ld, ...) Call ",hSession,ulEncryptedPartLen); 
    LOG_Debug(LOG_TO,"%s",512);

    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_SignEncryptUpdate (no soportada)
CK_RV C_SignEncryptUpdate(
			  CK_SESSION_HANDLE hSession,            /* the session's handle */
			  CK_BYTE_PTR       pPart,               /* the plaintext data */
			  CK_ULONG          ulPartLen,           /* bytes of plaintext data */
			  CK_BYTE_PTR       pEncryptedPart,      /* receives encrypted data */
			  CK_ULONG_PTR      pulEncryptedPartLen  /* receives encrypted byte count */
			  )
{
    // TODO: Not supported initialy.
    char aux[512];
	
    snprintf(aux, 512, "C_SignEncryptUpdate(hSession=%ld, ..., ulPartLen=%ld, ...) Call ",hSession,ulPartLen);   
    LOG_Debug(LOG_TO,"%s",aux);

    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_DecryptVerifyUpdate (no soportada)
CK_RV C_DecryptVerifyUpdate(
			    CK_SESSION_HANDLE hSession,            /* the session's handle */
			    CK_BYTE_PTR       pEncryptedPart,      /* input encrypted data */
			    CK_ULONG          ulEncryptedPartLen,  /* count of byes of input */
			    CK_BYTE_PTR       pPart,               /* receives decrypted output */
			    CK_ULONG_PTR      pulPartLen           /* receives decrypted byte count */
			    )
{
    // TODO: Not supported initialy.

    char aux[512];
	
    snprintf(aux, 512, "C_DecryptVerifyUpdate(hSession=%ld, ..., ulEncryptedPartLen=%ld, ...) Call ",hSession,ulEncryptedPartLen); 
    LOG_Debug(LOG_TO,"%s",aux);

    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_GenerateKey (no soportada)
CK_RV C_GenerateKey(
		    CK_SESSION_HANDLE    hSession,    /* the session's handle */
		    CK_MECHANISM_PTR     pMechanism,  /* the key generation mechanism */
		    CK_ATTRIBUTE_PTR     pTemplate,   /* template for the new key */
		    CK_ULONG             ulCount,     /* number of attributes in template */
		    CK_OBJECT_HANDLE_PTR phKey        /* receives handle of new key */
		    )
{
    // TODO: Not supported initialy.
  
    char aux[512];
	
    snprintf(aux, 512, "C_GenerateKey(hSession=%ld, ..., ulCount=%ld, ...) Call ",hSession,ulCount); 
    LOG_Debug(LOG_TO, "%s", aux);
    
    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_GenerateKeyPair soportada
CK_RV C_GenerateKeyPair(
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

    LOG_Debug(LOG_TO,"C_GenerateKeyPair(hSession=%ld, ...) Call ",hSession);     
    return slotClauer->C_GenerateKeyPair( hSession,  pMechanism, pPublicKeyTemplate,  
					  ulPublicKeyAttributeCount, pPrivateKeyTemplate,      
					  ulPrivateKeyAttributeCount, phPublicKey, phPrivateKey 
					  );
}


// C_WrapKey (no soportada)
CK_RV C_WrapKey(
		CK_SESSION_HANDLE hSession,        /* the session's handle */
		CK_MECHANISM_PTR  pMechanism,      /* the wrapping mechanism */
		CK_OBJECT_HANDLE  hWrappingKey,    /* handle of the wrapping key */
		CK_OBJECT_HANDLE  hKey,            /* handle of the key to be wrapped */
		CK_BYTE_PTR       pWrappedKey,     /* receives the wrapped key */
		CK_ULONG_PTR      pulWrappedKeyLen /* receives byte size of wrapped key */
		)
{
    // TODO: Not supported initialy.
  
    char aux[512];
	
    snprintf(aux, 512, "C_WrapKey(hSession=%ld, ..., hkey=%ld, ...) Call ",hSession,hKey); 
    LOG_Debug(LOG_TO, "%s", aux);

    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_UnwrapKey (no soportada)
CK_RV C_UnwrapKey(
		  CK_SESSION_HANDLE    hSession,          /* the session's handle */
		  CK_MECHANISM_PTR     pMechanism,        /* the unwrapping mechanism */
		  CK_OBJECT_HANDLE     hUnwrappingKey,    /* handle of the unwrapping key */
		  CK_BYTE_PTR          pWrappedKey,       /* the wrapped key */
		  CK_ULONG             ulWrappedKeyLen,   /* bytes length of wrapped key */
		  CK_ATTRIBUTE_PTR     pTemplate,         /* template for the new key */
		  CK_ULONG             ulAttributeCount,  /* # of attributes in template */
		  CK_OBJECT_HANDLE_PTR phKey              /* gets handle of recovered key */
		  )
{
    // TODO: Not supported initialy.

    LOG_Debug(LOG_TO,"C_UnwrapKey(hSession=%ld, ...,) Call ",hSession); 
    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_DeriveKey (no soportada)
CK_RV C_DeriveKey(
		  CK_SESSION_HANDLE    hSession,          /* the session's handle */
		  CK_MECHANISM_PTR     pMechanism,        /* the key derivation mechanism */
		  CK_OBJECT_HANDLE     hBaseKey,          /* handle of the base key */
		  CK_ATTRIBUTE_PTR     pTemplate,         /* template for the new key */
		  CK_ULONG             ulAttributeCount,  /* # of attributes in template */
		  CK_OBJECT_HANDLE_PTR phKey              /* gets handle of derived key */
		  )
{
    // TODO: Not supported initialy.
  
    char aux[512];
	
    snprintf(aux, 512, "C_DeriveKey(hSession=%ld, ...,hBaseKey=%ld, ...) Call ",hSession,hBaseKey); 
    LOG_Debug(LOG_TO,"%s",aux);

    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_SeedRandom (no soportada)
CK_RV C_SeedRandom(
		   CK_SESSION_HANDLE hSession,  /* the session's handle */
		   CK_BYTE_PTR       pSeed,     /* the seed material */
		   CK_ULONG          ulSeedLen  /* count of bytes of seed material */
		   )
{
    // TODO: Not supported initially, should support it!!

    LOG_Debug(LOG_TO,"C_SeedRandom(hSession=%ld, ...) Call ",hSession); 
    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_GenerateRandom (no soportada)
CK_RV C_GenerateRandom(
		       CK_SESSION_HANDLE hSession,    /* the session's handle */
		       CK_BYTE_PTR       RandomData,  /* receives the random data */
		       CK_ULONG          ulRandomLen  /* number of bytes to be generated */
		       )
{
    // TODO: Not supported initially, should support it!!
  
    LOG_Debug(LOG_TO,"C_GenerateRandom(hSession=%ld, ...) Call ",hSession); 
    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_GetFunctionStatus (no soportada)
CK_RV C_GetFunctionStatus(
			  CK_SESSION_HANDLE hSession  /* the session's handle */
			  )
{
  
    LOG_Msg(LOG_TO,"C_GetFunctionStatus Call"); 
    return CKR_FUNCTION_NOT_SUPPORTED;
}


// C_CancelFunction (no soportada)
CK_RV C_CancelFunction(
		       CK_SESSION_HANDLE hSession  /* the session's handle */
		       )
{
    // TODO: Not supported initially, should support it!!
  
    LOG_Debug(LOG_TO,"C_CancelFunction(hSession=%ld) Call ",hSession); 
    return CKR_FUNCTION_NOT_SUPPORTED;
}



#ifdef WIN32

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD fdwReason, LPVOID lpReserved)
{
    switch(fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	    DisableThreadLibraryCalls(hModule);   
	    break;

	case DLL_THREAD_ATTACH:
  	case DLL_PROCESS_DETACH:
	case DLL_THREAD_DETACH:
	    break;
	}
    return TRUE;
}
#endif
