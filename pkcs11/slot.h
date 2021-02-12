
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
 * Description: Contains the functions related to the slot.
 * 
 * Notes: Windows code is not been implemented yet. 
 *  
 * 
 *                         Clauer Team 2006 
 **/               

#ifndef __SLOT_H__
#define __SLOT_H__ 

#include "pkcs11.h"
#include "clauer.h"		// Slot has an associated clauer
#include "lobjeto.h"

#ifdef DEBUG
#include "log.h"
#endif

#include <openssl/rsa.h>
#include "threads.h"

// #include "session.h"		// Slot has an associated sesion too

/* Some global variables for signature creation */
static const unsigned char sha1sigheader[]={
									  0x30,0x21, // SEQUENCE, LENGTH
									  0x30,0x09, // SEQUENCE, LENGTH
									  0x06,0x05, // OCTET STRING, LENGTH 
									  0x2b,0x0e,0x03,0x02,0x1a, // SHA1 OID ( 1 4 14 3 2 26 )
									  0x05,0x00, // OPTIONAL ANY algorithm params (NULL)
									  0x04,0x14  // OCTECT STRING ( 20 bytes )
								    };

static const unsigned char sha1sigheader_noparam[]={
									  0x30,0x1f, // SEQUENCE, LENGTH
									  0x30,0x07, // SEQUENCE, LENGTH
									  0x06,0x05, // OCTET STRING, LENGTH 
									  0x2b,0x0e,0x03,0x02,0x1a, // SHA1 OID ( 1 4 14 3 2 26 )
									  0x04,0x14  // OCTECT STRING ( 20 bytes )
								    };

static const unsigned char md5sigheader[] ={
											0x30, 0x20, // SEQUENCE, LENGTH
											0x30, 0x0C, // SEQUENCE, LENGTH
											0x06, 0x08, // OCTET STRING, LENGTH
											0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x02, 0x05, // MD5 OID (1 2 840 113549 2 5)
											0x05, 0x00, // OPTIONAL ANY algorithm params (NULL)
											0x04, 0x10 // OCTECT STRING (16 bytes)
											};


class Slot
{
 public:
    Slot(CK_SLOT_ID id, 
	 CK_CHAR *slotDescription,
	 CK_CHAR *manufacturerID,
	 CK_FLAGS flags);
    ~Slot (void);	        // Destructor
	
    CK_RV C_GetSlotInfo (CK_SLOT_INFO_PTR pInfo);
    CK_RV C_GetTokenInfo (CK_TOKEN_INFO_PTR pInfo);
    CK_RV C_GetMechanismList (CK_MECHANISM_TYPE_PTR pMechanismList, CK_ULONG_PTR pulCount);
    CK_RV C_GetMechanismInfo(CK_MECHANISM_TYPE type, CK_MECHANISM_INFO_PTR pInfo);
	
    CK_RV C_OpenSession (CK_FLAGS flags, CK_VOID_PTR pApplication, CK_NOTIFY Notify, CK_SESSION_HANDLE_PTR phSession);
    CK_RV C_CloseSession (CK_SESSION_HANDLE pSession);
    CK_RV C_CloseAllSessions(void);


    CK_RV C_SignInit (CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hKey);
    CK_RV C_Sign (CK_BYTE_PTR pData, CK_ULONG ulDataLen, CK_BYTE_PTR pSignature, CK_ULONG_PTR pulSignatureLen);
    CK_RV C_SignUpdate(CK_BYTE_PTR pPart, CK_ULONG ulPartLen);  
	CK_RV C_SignFinal(CK_BYTE_PTR pSignature, CK_ULONG_PTR pulSignatureLen);

	CK_RV C_VerifyInit( CK_SESSION_HANDLE hSession,    /* the session's handle */
						CK_MECHANISM_PTR  pMechanism,  /* the verification mechanism */
						CK_OBJECT_HANDLE  hKey         /* handle of the verification key */ 
					   );

	CK_RV C_Verify(	CK_SESSION_HANDLE hSession,       /* the session's handle */
				    CK_BYTE_PTR       pData,          /* plaintext data (digest) to compare */
					CK_ULONG          ulDataLen,      /* length of data (digest) in bytes */
					CK_BYTE_PTR       pSignature,     /* the signature to be verified */
					CK_ULONG          ulSignatureLen  /* count of bytes of signature */
				   );
	
	CK_RV C_DecryptInit (CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hKey);
    CK_RV C_Decrypt (CK_BYTE_PTR pEncryptedData, CK_ULONG ulEncryptedDataLen, CK_BYTE_PTR pData, CK_ULONG_PTR pulDataLen);
    CK_RV C_GetSessionInfo (CK_SESSION_INFO_PTR pInfo);
	
    CK_RV C_GetAttributeValue(CK_OBJECT_HANDLE hObject, CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount);
    CK_RV C_SetAttributeValue(CK_OBJECT_HANDLE hObject, CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount);
	
    CK_RV C_FindObjectsInit (CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount);
    CK_RV C_FindObjects (CK_OBJECT_HANDLE_PTR phObject, CK_ULONG ulMaxObjectCount, CK_ULONG_PTR pulObjectCount);
    CK_RV C_FindObjectsFinal ();
	
    CK_RV C_Login (CK_USER_TYPE userType, CK_CHAR_PTR pPin, CK_ULONG ulPinLen);
    CK_RV C_Logout (void);
    CK_RV C_SetPIN(CK_SESSION_HANDLE hSession, CK_CHAR_PTR pOldPin, CK_ULONG ulOldLen, CK_CHAR_PTR pNewPin, CK_ULONG ulNewLen);
    CK_RV C_CreateObject(CK_SESSION_HANDLE hSession, CK_ATTRIBUTE_PTR  pTemplate,  CK_ULONG ulCount, CK_OBJECT_HANDLE_PTR phObject);
	CK_RV C_DestroyObject( CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE  hObject );
    CK_RV C_GenerateKeyPair( CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_ATTRIBUTE_PTR pPublicKeyTemplate,
			     CK_ULONG ulPublicKeyAttributeCount, CK_ATTRIBUTE_PTR pPrivateKeyTemplate, 
			     CK_ULONG ulPrivateKeyAttributeCount, CK_OBJECT_HANDLE_PTR phPublicKey,               
			     CK_OBJECT_HANDLE_PTR phPrivateKey );
	    	
    CK_SLOT_ID get_slotID (void);      
    CK_FLAGS get_flags (void);
    Clauer *get_token (void);

    // It is possible to change the token, but not our case
    // void set_token (Token *value);
    // Indicates if Clauer is present or not
    CK_RV CPresent(void);

    //Externally consulted
    CK_BBOOL isTokenPresent(void);

    listaObjetos *pListaObjetos;    // Session object list
    CK_OBJECT_HANDLE _hKey, _hKeyV; // Handle of the key to be used
    CK_MECHANISM_PTR _pMechanism;  
    unsigned char  clauerId[CLAUER_ID_LEN], zeroId[CLAUER_ID_LEN];
    ID_ACCESS_HANDLE th_id_handle;
	

 protected:
    CK_SLOT_ID slotID;
    CK_CHAR slotDescription[64];
    CK_CHAR manufacturerID[32];
    CK_FLAGS tokenFlags;
    CK_FLAGS slotFlags;
    CK_VERSION hardwareVersion;
    CK_VERSION firmwareVersion;
    CK_STATE  slotState;
    Clauer *pClauer;

	//For SignInit, SignUpdate and SignFinal 
	int multiSignOp; 
    EVP_MD_CTX multiCtx;
#ifdef LINUX
    pthread_t thread;
#elif defined(WIN32)
    HANDLE thread;
#endif
    // Session *pSession;
};

inline CK_SLOT_ID Slot::get_slotID (void)
{
    return slotID;
}

inline CK_FLAGS Slot::get_flags (void)
{
    return slotFlags;
}

inline Clauer * Slot::get_token (void)
{
    return pClauer;
}

#endif
