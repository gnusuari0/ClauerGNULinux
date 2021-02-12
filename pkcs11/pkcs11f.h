
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

/* This function contains pretty much everything about all */
/* the Cryptoki function prototypes. */



/* General-purpose */

/* C_Initialize initializes the Cryptoki library. */
CK_EXTERN _CK_RV CK_FUNC(C_Initialize)
#ifdef CK_NEED_ARG_LIST
(
  CK_VOID_PTR   pReserved  /* reserved.  Should be NULL_PTR */
);
#endif


/* C_Finalize indicates that an application is done with the Cryptoki
 * library. */
CK_EXTERN _CK_RV CK_FUNC(C_Finalize)
#ifdef CK_NEED_ARG_LIST
(
  CK_VOID_PTR   pReserved  /* reserved.  Should be NULL_PTR */
);
#endif


/* C_GetInfo returns general information about Cryptoki. */
CK_EXTERN _CK_RV CK_FUNC(C_GetInfo)
#ifdef CK_NEED_ARG_LIST
(
  CK_INFO_PTR   pInfo  /* location that receives the information */
);
#endif


/* C_GetFunctionList returns the function list. */
CK_EXTERN _CK_RV CK_FUNC(C_GetFunctionList)
#ifdef CK_NEED_ARG_LIST
(
  CK_FUNCTION_LIST_PTR_PTR ppFunctionList  /* receives ptr to function list */
);
#endif



/* Slot and token management */

/* C_GetSlotList obtains a list of slots in the system. */
CK_EXTERN _CK_RV CK_FUNC(C_GetSlotList)
#ifdef CK_NEED_ARG_LIST
(
  CK_BBOOL       tokenPresent,  /* only slots with token present */
  CK_SLOT_ID_PTR pSlotList,     /* receives the array of slot IDs */
  CK_ULONG_PTR   pulCount       /* receives the number of slots */
);
#endif


/* C_GetSlotInfo obtains information about a particular slot in the system. */
CK_EXTERN _CK_RV CK_FUNC(C_GetSlotInfo)
#ifdef CK_NEED_ARG_LIST
(
  CK_SLOT_ID       slotID,  /* the ID of the slot */
  CK_SLOT_INFO_PTR pInfo    /* receives the slot information */
);
#endif


/* C_GetTokenInfo obtains information about a particular token in the
 * system. */
CK_EXTERN _CK_RV CK_FUNC(C_GetTokenInfo)
#ifdef CK_NEED_ARG_LIST
(
  CK_SLOT_ID        slotID,  /* ID of the token's slot */
  CK_TOKEN_INFO_PTR pInfo    /* receives the token information */
);
#endif


/* C_GetMechanismList obtains a list of mechanism types supported by
 * a token. */
CK_EXTERN _CK_RV CK_FUNC(C_GetMechanismList)
#ifdef CK_NEED_ARG_LIST
(
  CK_SLOT_ID            slotID,          /* ID of the token's slot */
  CK_MECHANISM_TYPE_PTR pMechanismList,  /* receives mech. types array */
  CK_ULONG_PTR          pulCount         /* receives number of mechs. */
);
#endif


/* C_GetMechanismInfo obtains information about a particular mechanism 
 * possibly supported by a token. */
CK_EXTERN _CK_RV CK_FUNC(C_GetMechanismInfo)
#ifdef CK_NEED_ARG_LIST
(
  CK_SLOT_ID            slotID,  /* ID of the token's slot */
  CK_MECHANISM_TYPE     type,    /* type of mechanism */
  CK_MECHANISM_INFO_PTR pInfo    /* receives mechanism information */
);
#endif


/* C_InitToken initializes a token. */
CK_EXTERN _CK_RV CK_FUNC(C_InitToken)
#ifdef CK_NEED_ARG_LIST
(
  CK_SLOT_ID     slotID,    /* ID of the token's slot */
  CK_CHAR_PTR    pPin,      /* the SO's initial PIN */
  CK_ULONG       ulPinLen,  /* length in bytes of the PIN */
  CK_CHAR_PTR    pLabel     /* 32-byte token label (blank padded) */
);
#endif


/* C_InitPIN initializes the normal user's PIN. */
CK_EXTERN _CK_RV CK_FUNC(C_InitPIN)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,  /* the session's handle */
  CK_CHAR_PTR       pPin,      /* the normal user's PIN */
  CK_ULONG          ulPinLen   /* length in bytes of the PIN */
);
#endif


/* C_SetPIN modifies the PIN of user that is currently logged in. */
CK_EXTERN _CK_RV CK_FUNC(C_SetPIN)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,  /* the session's handle */
  CK_CHAR_PTR       pOldPin,   /* the old PIN */
  CK_ULONG          ulOldLen,  /* length of the old PIN */
  CK_CHAR_PTR       pNewPin,   /* the new PIN */
  CK_ULONG          ulNewLen   /* length of the new PIN */
);
#endif



/* Session management */

/* C_OpenSession opens a session between an application and a token. */
CK_EXTERN _CK_RV CK_FUNC(C_OpenSession)
#ifdef CK_NEED_ARG_LIST
(
  CK_SLOT_ID            slotID,        /* the slot's ID */
  CK_FLAGS              flags,         /* defined in CK_SESSION_INFO */
  CK_VOID_PTR           pApplication,  /* pointer passed to callback */
  CK_NOTIFY             Notify,        /* notification callback function */
  CK_SESSION_HANDLE_PTR phSession      /* receives new session handle */
);
#endif


/* C_CloseSession closes a session between an application and a token. */
CK_EXTERN _CK_RV CK_FUNC(C_CloseSession)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession  /* the session's handle */
);
#endif


/* C_CloseAllSessions closes all sessions with a token. */
CK_EXTERN _CK_RV CK_FUNC(C_CloseAllSessions)
#ifdef CK_NEED_ARG_LIST
(
  CK_SLOT_ID     slotID  /* the token's slot */
);
#endif


/* C_GetSessionInfo obtains information about the session. */
CK_EXTERN _CK_RV CK_FUNC(C_GetSessionInfo)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE   hSession,  /* the session's handle */
  CK_SESSION_INFO_PTR pInfo      /* receives session information */
);
#endif


/* C_GetOperationState obtains the state of the cryptographic operation
 * in a session. */
CK_EXTERN _CK_RV CK_FUNC(C_GetOperationState)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,             /* the session's handle */
  CK_BYTE_PTR       pOperationState,      /* location receiving state */
  CK_ULONG_PTR      pulOperationStateLen  /* location receiving state length */
);
#endif


/* C_SetOperationState restores the state of the cryptographic operation
 * in a session. */
CK_EXTERN _CK_RV CK_FUNC(C_SetOperationState)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,            /* the session's handle */
  CK_BYTE_PTR      pOperationState,      /* the location holding the state */
  CK_ULONG         ulOperationStateLen,  /* location holding state length */
  CK_OBJECT_HANDLE hEncryptionKey,       /* handle of en/decryption key */
  CK_OBJECT_HANDLE hAuthenticationKey    /* handle of sign/verify key */
);
#endif


/* C_Login logs a user into a token. */
CK_EXTERN _CK_RV CK_FUNC(C_Login)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,  /* the session's handle */
  CK_USER_TYPE      userType,  /* the user type */
  CK_CHAR_PTR       pPin,      /* the user's PIN */
  CK_ULONG          ulPinLen   /* the length of the PIN */
);
#endif


/* C_Logout logs a user out from a token. */
CK_EXTERN _CK_RV CK_FUNC(C_Logout)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession  /* the session's handle */
);
#endif



/* Object management */

/* C_CreateObject creates a new object. */
CK_EXTERN _CK_RV CK_FUNC(C_CreateObject)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,    /* the session's handle */
  CK_ATTRIBUTE_PTR  pTemplate,   /* the object's template */
  CK_ULONG          ulCount,     /* attributes in template */
  CK_OBJECT_HANDLE_PTR phObject  /* receives new object's handle. */
);
#endif


/* C_CopyObject copies an object, creating a new object for the copy. */
CK_EXTERN _CK_RV CK_FUNC(C_CopyObject)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE    hSession,    /* the session's handle */
  CK_OBJECT_HANDLE     hObject,     /* the object's handle */
  CK_ATTRIBUTE_PTR     pTemplate,   /* template for new object */
  CK_ULONG             ulCount,     /* attributes in template */
  CK_OBJECT_HANDLE_PTR phNewObject  /* receives handle of copy */
);
#endif


/* C_DestroyObject destroys an object. */
CK_EXTERN _CK_RV CK_FUNC(C_DestroyObject)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,  /* the session's handle */
  CK_OBJECT_HANDLE  hObject    /* the object's handle */
);
#endif


/* C_GetObjectSize gets the size of an object in bytes. */
CK_EXTERN _CK_RV CK_FUNC(C_GetObjectSize)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,  /* the session's handle */
  CK_OBJECT_HANDLE  hObject,   /* the object's handle */
  CK_ULONG_PTR      pulSize    /* receives size of object */
);
#endif


/* C_GetAttributeValue obtains the value of one or more object attributes. */
CK_EXTERN _CK_RV CK_FUNC(C_GetAttributeValue)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,   /* the session's handle */
  CK_OBJECT_HANDLE  hObject,    /* the object's handle */
  CK_ATTRIBUTE_PTR  pTemplate,  /* specifies attributes, gets values */
  CK_ULONG          ulCount     /* attributes in template */
);
#endif


/* C_SetAttributeValue modifies the value of one or more object attributes */
CK_EXTERN _CK_RV CK_FUNC(C_SetAttributeValue)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,   /* the session's handle */
  CK_OBJECT_HANDLE  hObject,    /* the object's handle */
  CK_ATTRIBUTE_PTR  pTemplate,  /* specifies attributes and values */
  CK_ULONG          ulCount     /* attributes in template */
);
#endif


/* C_FindObjectsInit initializes a search for token and session objects 
 * that match a template. */
CK_EXTERN _CK_RV CK_FUNC(C_FindObjectsInit)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,   /* the session's handle */
  CK_ATTRIBUTE_PTR  pTemplate,  /* attribute values to match */
  CK_ULONG          ulCount     /* attributes in search template */
);
#endif


/* C_FindObjects continues a search for token and session objects 
 * that match a template, obtaining additional object handles. */
CK_EXTERN _CK_RV CK_FUNC(C_FindObjects)
#ifdef CK_NEED_ARG_LIST
(
 CK_SESSION_HANDLE    hSession,          /* the session's handle */
 CK_OBJECT_HANDLE_PTR phObject,          /* receives object handle array */
 CK_ULONG             ulMaxObjectCount,  /* max handles to be returned */
 CK_ULONG_PTR         pulObjectCount     /* actual number returned */
);
#endif


/* C_FindObjectsFinal finishes a search for token and session objects. */
CK_EXTERN _CK_RV CK_FUNC(C_FindObjectsFinal)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession  /* the session's handle */
);
#endif



/* Encryption and decryption */

/* C_EncryptInit initializes an encryption operation. */
CK_EXTERN _CK_RV CK_FUNC(C_EncryptInit)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,    /* the session's handle */
  CK_MECHANISM_PTR  pMechanism,  /* the encryption mechanism */
  CK_OBJECT_HANDLE  hKey         /* handle of encryption key */
);
#endif


/* C_Encrypt encrypts single-part data. */
CK_EXTERN _CK_RV CK_FUNC(C_Encrypt)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,            /* the session's handle */
  CK_BYTE_PTR       pData,               /* the plaintext data */
  CK_ULONG          ulDataLen,           /* bytes of plaintext data */
  CK_BYTE_PTR       pEncryptedData,      /* receives encrypted data */
  CK_ULONG_PTR      pulEncryptedDataLen  /* receives encrypted byte count */
);
#endif


/* C_EncryptUpdate continues a multiple-part encryption operation. */
CK_EXTERN _CK_RV CK_FUNC(C_EncryptUpdate)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,           /* the session's handle */
  CK_BYTE_PTR       pPart,              /* the plaintext data */
  CK_ULONG          ulPartLen,          /* bytes of plaintext data */
  CK_BYTE_PTR       pEncryptedPart,     /* receives encrypted data */
  CK_ULONG_PTR      pulEncryptedPartLen /* receives encrypted byte count */
);
#endif


/* C_EncryptFinal finishes a multiple-part encryption operation. */
CK_EXTERN _CK_RV CK_FUNC(C_EncryptFinal)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,                /* the session's handle */
  CK_BYTE_PTR       pLastEncryptedPart,      /* receives encrypted last part */
  CK_ULONG_PTR      pulLastEncryptedPartLen  /* receives byte count */
);
#endif


/* C_DecryptInit initializes a decryption operation. */
CK_EXTERN _CK_RV CK_FUNC(C_DecryptInit)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,    /* the session's handle */
  CK_MECHANISM_PTR  pMechanism,  /* the decryption mechanism */
  CK_OBJECT_HANDLE  hKey         /* handle of the decryption key */
);
#endif


/* C_Decrypt decrypts encrypted data in a single part. */
CK_EXTERN _CK_RV CK_FUNC(C_Decrypt)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,           /* the session's handle */
  CK_BYTE_PTR       pEncryptedData,     /* input encrypted data */
  CK_ULONG          ulEncryptedDataLen, /* count of bytes of input */
  CK_BYTE_PTR       pData,              /* receives decrypted output */
  CK_ULONG_PTR      pulDataLen          /* receives decrypted byte count */
);
#endif


/* C_DecryptUpdate continues a multiple-part decryption operation. */
CK_EXTERN _CK_RV CK_FUNC(C_DecryptUpdate)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,            /* the session's handle */
  CK_BYTE_PTR       pEncryptedPart,      /* input encrypted data */
  CK_ULONG          ulEncryptedPartLen,  /* count of bytes of input */
  CK_BYTE_PTR       pPart,               /* receives decrypted output */
  CK_ULONG_PTR      pulPartLen           /* receives decrypted byte count */
);
#endif


/* C_DecryptFinal finishes a multiple-part decryption operation. */
CK_EXTERN _CK_RV CK_FUNC(C_DecryptFinal)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,       /* the session's handle */
  CK_BYTE_PTR       pLastPart,      /* receives decrypted output */
  CK_ULONG_PTR      pulLastPartLen  /* receives decrypted byte count */
);
#endif



/* Message digesting */

/* C_DigestInit initializes a message-digesting operation. */
CK_EXTERN _CK_RV CK_FUNC(C_DigestInit)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,   /* the session's handle */
  CK_MECHANISM_PTR  pMechanism  /* the digesting mechanism */
);
#endif


/* C_Digest digests data in a single part. */
CK_EXTERN _CK_RV CK_FUNC(C_Digest)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,     /* the session's handle */
  CK_BYTE_PTR       pData,        /* data to be digested */
  CK_ULONG          ulDataLen,    /* bytes of data to be digested */
  CK_BYTE_PTR       pDigest,      /* receives the message digest */
  CK_ULONG_PTR      pulDigestLen  /* receives byte length of digest */
);
#endif


/* C_DigestUpdate continues a multiple-part message-digesting operation. */
CK_EXTERN _CK_RV CK_FUNC(C_DigestUpdate)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,  /* the session's handle */
  CK_BYTE_PTR       pPart,     /* data to be digested */
  CK_ULONG          ulPartLen  /* bytes of data to be digested */
);
#endif


/* C_DigestKey continues a multi-part message-digesting operation, by
 * digesting the value of a secret key as part of the data already digested.
 */
CK_EXTERN _CK_RV CK_FUNC(C_DigestKey)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,  /* the session's handle */
  CK_OBJECT_HANDLE  hKey       /* handle of secret key to digest */
);
#endif


/* C_DigestFinal finishes a multiple-part message-digesting operation. */
CK_EXTERN _CK_RV CK_FUNC(C_DigestFinal)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,     /* the session's handle */
  CK_BYTE_PTR       pDigest,      /* receives the message digest */
  CK_ULONG_PTR      pulDigestLen  /* receives byte count of digest */
);
#endif



/* Signing and MACing */

/* C_SignInit initializes a signature (private key encryption) operation,
 * where the signature is (will be) an appendix to the data, 
 * and plaintext cannot be recovered from the signature */
CK_EXTERN _CK_RV CK_FUNC(C_SignInit)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,    /* the session's handle */
  CK_MECHANISM_PTR  pMechanism,  /* the signature mechanism */
  CK_OBJECT_HANDLE  hKey         /* handle of the signature key */
);
#endif


/* C_Sign signs (encrypts with private key) data in a single part,
 * where the signature is (will be) an appendix to the data, 
 * and plaintext cannot be recovered from the signature */
CK_EXTERN _CK_RV CK_FUNC(C_Sign)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,        /* the session's handle */
  CK_BYTE_PTR       pData,           /* the data (digest) to be signed */
  CK_ULONG          ulDataLen,       /* count of bytes to be signed */
  CK_BYTE_PTR       pSignature,      /* receives the signature */
  CK_ULONG_PTR      pulSignatureLen  /* receives byte count of signature */
);
#endif


/* C_SignUpdate continues a multiple-part signature operation,
 * where the signature is (will be) an appendix to the data, 
 * and plaintext cannot be recovered from the signature */
CK_EXTERN _CK_RV CK_FUNC(C_SignUpdate)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,  /* the session's handle */
  CK_BYTE_PTR       pPart,     /* the data (digest) to be signed */
  CK_ULONG          ulPartLen  /* count of bytes to be signed */
);
#endif


/* C_SignFinal finishes a multiple-part signature operation, 
 * returning the signature. */
CK_EXTERN _CK_RV CK_FUNC(C_SignFinal)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,        /* the session's handle */
  CK_BYTE_PTR       pSignature,      /* receives the signature */
  CK_ULONG_PTR      pulSignatureLen  /* receives byte count of signature */
);
#endif


/* C_SignRecoverInit initializes a signature operation,
 * where the (digest) data can be recovered from the signature. 
 * E.g. encryption with the user's private key */
CK_EXTERN _CK_RV CK_FUNC(C_SignRecoverInit)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,   /* the session's handle */
  CK_MECHANISM_PTR  pMechanism, /* the signature mechanism */
  CK_OBJECT_HANDLE  hKey        /* handle of the signature key */
);
#endif


/* C_SignRecover signs data in a single operation
 * where the (digest) data can be recovered from the signature. 
 * E.g. encryption with the user's private key */
CK_EXTERN _CK_RV CK_FUNC(C_SignRecover)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,        /* the session's handle */
  CK_BYTE_PTR       pData,           /* the data (digest) to be signed */
  CK_ULONG          ulDataLen,       /* count of bytes to be signed */
  CK_BYTE_PTR       pSignature,      /* receives the signature */
  CK_ULONG_PTR      pulSignatureLen  /* receives byte count of signature */
);
#endif



/* Verifying signatures and MACs */

/* C_VerifyInit initializes a verification operation, 
 * where the signature is an appendix to the data, 
 * and plaintext cannot be recovered from the signature (e.g. DSA) */
CK_EXTERN _CK_RV CK_FUNC(C_VerifyInit)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,    /* the session's handle */
  CK_MECHANISM_PTR  pMechanism,  /* the verification mechanism */
  CK_OBJECT_HANDLE  hKey         /* handle of the verification key */ 
);
#endif


/* C_Verify verifies a signature in a single-part operation, 
 * where the signature is an appendix to the data, 
 * and plaintext cannot be recovered from the signature */
CK_EXTERN _CK_RV CK_FUNC(C_Verify)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,       /* the session's handle */
  CK_BYTE_PTR       pData,          /* plaintext data (digest) to compare */
  CK_ULONG          ulDataLen,      /* length of data (digest) in bytes */
  CK_BYTE_PTR       pSignature,     /* the signature to be verified */
  CK_ULONG          ulSignatureLen  /* count of bytes of signature */
);
#endif


/* C_VerifyUpdate continues a multiple-part verification operation, 
 * where the signature is an appendix to the data, 
 * and plaintext cannot be recovered from the signature */
CK_EXTERN _CK_RV CK_FUNC(C_VerifyUpdate)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,  /* the session's handle */
  CK_BYTE_PTR       pPart,     /* plaintext data (digest) to compare */
  CK_ULONG          ulPartLen  /* length of data (digest) in bytes */
);
#endif


/* C_VerifyFinal finishes a multiple-part verification operation, 
 * checking the signature. */
CK_EXTERN _CK_RV CK_FUNC(C_VerifyFinal)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,       /* the session's handle */
  CK_BYTE_PTR       pSignature,     /* the signature to be verified */
  CK_ULONG          ulSignatureLen  /* count of bytes of signature */
);
#endif


/* C_VerifyRecoverInit initializes a signature verification operation, 
 * where the data is recovered from the signature. 
 * E.g. Decryption with the user's public key */
CK_EXTERN _CK_RV CK_FUNC(C_VerifyRecoverInit)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,    /* the session's handle */
  CK_MECHANISM_PTR  pMechanism,  /* the verification mechanism */
  CK_OBJECT_HANDLE  hKey         /* handle of the verification key */
);
#endif


/* C_VerifyRecover verifies a signature in a single-part operation, 
 * where the data is recovered from the signature. 
 * E.g. Decryption with the user's public key */
CK_EXTERN _CK_RV CK_FUNC(C_VerifyRecover)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,        /* the session's handle */
  CK_BYTE_PTR       pSignature,      /* the signature to be verified */
  CK_ULONG          ulSignatureLen,  /* count of bytes of signature */
  CK_BYTE_PTR       pData,           /* receives decrypted data (digest) */
  CK_ULONG_PTR      pulDataLen       /* receives byte count of data */
);
#endif



/* Dual-function cryptographic operations */

/* C_DigestEncryptUpdate continues a multiple-part digesting and encryption operation. */
CK_EXTERN _CK_RV CK_FUNC(C_DigestEncryptUpdate)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,            /* the session's handle */
  CK_BYTE_PTR       pPart,               /* the plaintext data */
  CK_ULONG          ulPartLen,           /* bytes of plaintext data */
  CK_BYTE_PTR       pEncryptedPart,      /* receives encrypted data */
  CK_ULONG_PTR      pulEncryptedPartLen  /* receives encrypted byte count */
);
#endif


/* C_DecryptDigestUpdate continues a multiple-part decryption and
 * digesting operation. */
CK_EXTERN _CK_RV CK_FUNC(C_DecryptDigestUpdate)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,            /* the session's handle */
  CK_BYTE_PTR       pEncryptedPart,      /* input encrypted data */
  CK_ULONG          ulEncryptedPartLen,  /* count of bytes of input */
  CK_BYTE_PTR       pPart,               /* receives decrypted output */
  CK_ULONG_PTR      pulPartLen           /* receives decrypted byte count */
);
#endif


/* C_SignEncryptUpdate continues a multiple-part signing and
 * encryption operation. */
CK_EXTERN _CK_RV CK_FUNC(C_SignEncryptUpdate)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,            /* the session's handle */
  CK_BYTE_PTR       pPart,               /* the plaintext data */
  CK_ULONG          ulPartLen,           /* bytes of plaintext data */
  CK_BYTE_PTR       pEncryptedPart,      /* receives encrypted data */
  CK_ULONG_PTR      pulEncryptedPartLen  /* receives encrypted byte count */
);
#endif


/* C_DecryptVerifyUpdate continues a multiple-part decryption and
 * verify operation. */
CK_EXTERN _CK_RV CK_FUNC(C_DecryptVerifyUpdate)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,            /* the session's handle */
  CK_BYTE_PTR       pEncryptedPart,      /* input encrypted data */
  CK_ULONG          ulEncryptedPartLen,  /* count of byes of input */
  CK_BYTE_PTR       pPart,               /* receives decrypted output */
  CK_ULONG_PTR      pulPartLen           /* receives decrypted byte count */
);
#endif



/* Key management */

/* C_GenerateKey generates a secret key, creating a new key object. */
CK_EXTERN _CK_RV CK_FUNC(C_GenerateKey)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE    hSession,    /* the session's handle */
  CK_MECHANISM_PTR     pMechanism,  /* the key generation mechanism */
  CK_ATTRIBUTE_PTR     pTemplate,   /* template for the new key */
  CK_ULONG             ulCount,     /* number of attributes in template */
  CK_OBJECT_HANDLE_PTR phKey        /* receives handle of new key */
);
#endif


/* C_GenerateKeyPair generates a public-key/private-key pair, 
 * creating new key objects. */
CK_EXTERN _CK_RV CK_FUNC(C_GenerateKeyPair)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE    hSession,                    /* the session's handle */
  CK_MECHANISM_PTR     pMechanism,                  /* the key gen. mech. */
  CK_ATTRIBUTE_PTR     pPublicKeyTemplate,          /* pub. attr. template */
  CK_ULONG             ulPublicKeyAttributeCount,   /* # of pub. attrs. */
  CK_ATTRIBUTE_PTR     pPrivateKeyTemplate,         /* priv. attr. template */
  CK_ULONG             ulPrivateKeyAttributeCount,  /* # of priv. attrs. */
  CK_OBJECT_HANDLE_PTR phPublicKey,                 /* gets pub. key handle */
  CK_OBJECT_HANDLE_PTR phPrivateKey                 /* gets priv. key handle */
);
#endif


/* C_WrapKey wraps (i.e., encrypts) a key. */
CK_EXTERN _CK_RV CK_FUNC(C_WrapKey)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,        /* the session's handle */
  CK_MECHANISM_PTR  pMechanism,      /* the wrapping mechanism */
  CK_OBJECT_HANDLE  hWrappingKey,    /* handle of the wrapping key */
  CK_OBJECT_HANDLE  hKey,            /* handle of the key to be wrapped */
  CK_BYTE_PTR       pWrappedKey,     /* receives the wrapped key */
  CK_ULONG_PTR      pulWrappedKeyLen /* receives byte size of wrapped key */
);
#endif


/* C_UnwrapKey unwraps (decrypts) a wrapped key, creating a new key object. */
CK_EXTERN _CK_RV CK_FUNC(C_UnwrapKey)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE    hSession,          /* the session's handle */
  CK_MECHANISM_PTR     pMechanism,        /* the unwrapping mechanism */
  CK_OBJECT_HANDLE     hUnwrappingKey,    /* handle of the unwrapping key */
  CK_BYTE_PTR          pWrappedKey,       /* the wrapped key */
  CK_ULONG             ulWrappedKeyLen,   /* bytes length of wrapped key */
  CK_ATTRIBUTE_PTR     pTemplate,         /* template for the new key */
  CK_ULONG             ulAttributeCount,  /* # of attributes in template */
  CK_OBJECT_HANDLE_PTR phKey              /* gets handle of recovered key */
);
#endif


/* C_DeriveKey derives a key from a base key, creating a new key object. */
CK_EXTERN _CK_RV CK_FUNC(C_DeriveKey)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE    hSession,          /* the session's handle */
  CK_MECHANISM_PTR     pMechanism,        /* the key derivation mechanism */
  CK_OBJECT_HANDLE     hBaseKey,          /* handle of the base key */
  CK_ATTRIBUTE_PTR     pTemplate,         /* template for the new key */
  CK_ULONG             ulAttributeCount,  /* # of attributes in template */
  CK_OBJECT_HANDLE_PTR phKey              /* gets handle of derived key */
);
#endif



/* Random number generation */

/* C_SeedRandom mixes additional seed material into the token's random number 
 * generator. */
CK_EXTERN _CK_RV CK_FUNC(C_SeedRandom)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,  /* the session's handle */
  CK_BYTE_PTR       pSeed,     /* the seed material */
  CK_ULONG          ulSeedLen  /* count of bytes of seed material */
);
#endif


/* C_GenerateRandom generates random data. */
CK_EXTERN _CK_RV CK_FUNC(C_GenerateRandom)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,    /* the session's handle */
  CK_BYTE_PTR       RandomData,  /* receives the random data */
  CK_ULONG          ulRandomLen  /* number of bytes to be generated */
);
#endif



/* Parallel function management */

/* C_GetFunctionStatus obtains an updated status of a function running 
 * in parallel with an application. */
CK_EXTERN _CK_RV CK_FUNC(C_GetFunctionStatus)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession  /* the session's handle */
);
#endif


/* C_CancelFunction cancels a function running in parallel. */
CK_EXTERN _CK_RV CK_FUNC(C_CancelFunction)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession  /* the session's handle */
);
#endif
