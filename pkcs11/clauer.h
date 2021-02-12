
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

#ifndef __CLAUER_H__
#define __CLAUER_H__ 

#include "pkcs11.h"
#include "mechanism.h" 
#include "lobjeto.h"

#ifdef DEBUG
#include "log.h"
#endif
#include "certx509.h"
#include "clvrsa.h"
#include "clvprvda.h"
#include "threads.h"
#include <CRYPTOWrapper/CRYPTOWrap.h>
#include <LIBRT/libRT.h>


// TODO: Thats have to do with openssl
//       should it be embedded into CRYPTOWrapper? 
// ----------------------
#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
// ----------------------


#define CERT_OWN   0
#define CERT_CA    1
#define CERT_ROOT  2

#if defined(LINUX)
	
#define BOOL unsigned char
#define BYTE unsigned char
#define LPTSTR char *
#define DWORD unsigned int

// Funci~n de windows, equivalente para Linux en
// CryptoWrapper
//
#define SecureZeroMemory CRYPTO_SecureZeroMemory

// Defines de windows para compatibilidad en
// Linux.

// Obtenido de windows.h
#define MAX_PATH 260

// Obtenidos de wincrypt.h
#define ALG_CLASS_KEY_EXCHANGE  (5 << 13)
#define ALG_CLASS_SIGNATURE     (1 << 13)
#define ALG_TYPE_RSA            (2 << 9)
#define ALG_SID_RSA_ANY          0

#define CALG_RSA_KEYX  (ALG_CLASS_KEY_EXCHANGE|ALG_TYPE_RSA|ALG_SID_RSA_ANY) // 0x0000a400
#define CALG_RSA_SIGN  (ALG_CLASS_SIGNATURE|ALG_TYPE_RSA|ALG_SID_RSA_ANY)    // 0x00002400
#endif

class Clauer
{
 public:
    Clauer (CK_CHAR *label,
	    CK_CHAR *manufacturerID,
	    CK_CHAR *model,
	    CK_CHAR *serialNumber,
	    CK_FLAGS flags,
	    CK_ULONG ulMaxSessionCount,
	    CK_ULONG ulSessionCount,
	    CK_ULONG ulMaxRwSessionCount,
	    CK_ULONG ulRwSessionCount,
	    CK_ULONG ulMaxPinLen,
	    CK_ULONG ulMinPinLen,
	    CK_ULONG ulTotalPublicMemory,
	    CK_ULONG ulFreePublicMemory,
	    CK_ULONG ulTotalPrivateMemory,
	    CK_ULONG ulFreePrivateMemory,
	    CK_VERSION *hardwareVersion,
	    CK_VERSION *firmwareVersion,
	    CK_CHAR *utcTime);			// Constructor
    ~Clauer (void);	                        // Destructor

    CK_RV C_GetTokenInfo (CK_TOKEN_INFO_PTR pInfo);
    CK_RV C_GetMechanismList (CK_MECHANISM_TYPE_PTR pMechanismList, CK_ULONG_PTR pulCount);
    CK_RV C_GetMechanismInfo(CK_MECHANISM_TYPE type, CK_MECHANISM_INFO_PTR pInfo);
    CK_RV C_SetPIN(CK_SESSION_HANDLE hSession, CK_CHAR_PTR pOldPin, CK_ULONG ulOldLen,
	CK_CHAR_PTR pNewPin, CK_ULONG ulNewLen);
	
    CK_RV C_Login(CK_CHAR_PTR _pin, CK_ULONG _pinLen, listaObjetos * pListaObjetos);
    CK_RV C_Logout(void);
    CK_RV C_CloseSession(void);
	
    CK_RV LoadClauerObjects(listaObjetos * pListaObjetos );
    CK_RV UnloadClauerObjects(listaObjetos * pListaObjetos);
	
    CK_RV C_CreateObject(
			 CK_SESSION_HANDLE hSession,    /* the session's handle */
			 CK_ATTRIBUTE_PTR  pTemplate,   /* the object's template */
			 CK_ULONG          ulCount,     /* attributes in template */
			 CK_OBJECT_HANDLE_PTR phObject,  /* receives new object's handle. */
			 listaObjetos * pListaObjetos
			 );

	CK_RV C_DestroyObject( CK_OBJECT_HANDLE  hObject, listaObjetos * pListaObjetos );

    CK_RV C_GenerateKeyPair(
			    CK_SESSION_HANDLE    hSession,                    /* the session's handle */
			    CK_MECHANISM_PTR     pMechanism,                  /* the key gen. mech. */
			    CK_ATTRIBUTE_PTR     pPublicKeyTemplate,          /* pub. attr. template */
			    CK_ULONG             ulPublicKeyAttributeCount,   /* # of pub. attrs. */
			    CK_ATTRIBUTE_PTR     pPrivateKeyTemplate,         /* priv. attr. template */
			    CK_ULONG             ulPrivateKeyAttributeCount,  /* # of priv. attrs. */
			    CK_OBJECT_HANDLE_PTR phPublicKey,                 /* gets pub. key handle */
			    CK_OBJECT_HANDLE_PTR phPrivateKey,                /* gets priv. key handle */
			    listaObjetos * pListaObjetos                      /* Pointer to current object list*/
			    );

    CK_ULONG get_ulMaxSessionCount (void);
    CK_ULONG get_ulSessionCount (void);
    void set_ulSessionCount (CK_ULONG value);

    int getActiveClauer( char activeClauer[MAX_PATH_LEN] );
    
    CK_RV getRSAKey( unsigned char * keyId, RSA ** rsa, ID_ACCESS_HANDLE * th_id_handle );
   
    CK_RV _insertCertificateObjectAndPublicKey( unsigned char * block, listaObjetos * pListaObjetos,   
						CK_BYTE *  identificador, int tamId, CK_OBJECT_HANDLE_PTR phObject,
						unsigned char * certId, char ** label, int insertPrivKey  );
    
    int _insertKeyObject( listaObjetos * pListaObjetos, CK_BYTE * identificador, 
			  int tamIdentificador, unsigned char * certId, CK_OBJECT_HANDLE_PTR phObject,
			  CK_ATTRIBUTE_PTR pPrivateKeyTemplate, CK_ULONG ulPrivateKeyAttributeCount,
			  char * label);	

    CK_RV _insertPublicKeyObject( RSA * rsa, unsigned char * identificador, int tamId, unsigned char realId[20],
				  listaObjetos * pListaObjetos, CK_OBJECT_HANDLE_PTR phObject,
				  CK_ATTRIBUTE_PTR pPublicKeyTemplate, CK_ULONG ulPublicKeyAttributeCount );
	
    CK_RV _insertOpensslRsaKeyObject(RSA * rsa,  listaObjetos * pListaObjetos, 
				     unsigned char * cka_id, int cka_id_size,  
				     CK_OBJECT_HANDLE_PTR phObject, unsigned char realId[20],
				     CK_ATTRIBUTE_PTR pPrivateKeyTemplate, CK_ULONG ulPrivateKeyAttributeCount);
	    

    CK_CHAR *get_label (void);
    CK_CHAR *get_manufacturerID (void);
    CK_CHAR *get_model (void);
    CK_CHAR *get_serialNumber (void);
    CK_FLAGS get_flags (void);
    CK_ULONG get_ulMaxRwSessionCount (void);
    CK_ULONG get_ulRwSessionCount (void);
    CK_ULONG get_ulMaxPinLen (void);
    CK_ULONG get_ulMinPinLen (void);
    CK_ULONG get_ulTotalPublicMemory (void);
    CK_ULONG get_ulFreePublicMemory (void);
    CK_ULONG get_ulTotalPrivateMemory (void);
    CK_ULONG get_ulFreePrivateMemory (void);
    CK_VERSION get_hardwareVersion (void);
    CK_VERSION get_firmwareVersion (void);
    CK_CHAR *get_utcTime (void);
	bool isLogged (void);
      
 protected:
    CK_RV _deleteCert( unsigned char * id, listaObjetos * pListaObjetos );
    char* _getKeyUsage(X509 * xcert);
    int  _ExistsCertificate( unsigned char * id );
    CK_CHAR label[32];
    CK_CHAR manufacturerID[32];
    CK_CHAR model[16];
    CK_CHAR serialNumber[16];
    CK_FLAGS flags;
    CK_ULONG ulMaxSessionCount;
    CK_ULONG ulSessionCount;
    CK_ULONG ulMaxRwSessionCount;
    CK_ULONG ulRwSessionCount;
    CK_ULONG ulMaxPinLen;
    CK_ULONG ulMinPinLen;
    CK_ULONG ulTotalPublicMemory;
    CK_ULONG ulFreePublicMemory;
    CK_ULONG ulTotalPrivateMemory;
    CK_ULONG ulFreePrivateMemory;
    CK_VERSION hardwareVersion;
    CK_VERSION firmwareVersion;
    CK_CHAR utcTime[16];
    bool _isLogged;
    bool _objectsLoaded;
    bool _initialized;
    char _pin[MAX_PIN_LEN];
    char _actualClauer[MAX_PATH_LEN];
    char _lastClauerId[CLAUER_ID_LEN];
    Mechanism * pMechanism;	
    Mechanism * pMechanism_keyGen;	
	Mechanism * pMechanism_SHA1_PKCS;
};

inline bool Clauer::isLogged(void)
{
 return _isLogged;
}

// Get Methods 
inline CK_CHAR *Clauer::get_label (void)
{
    return label;
}

inline CK_CHAR *Clauer::get_manufacturerID (void)
{
    return manufacturerID;
}

inline CK_CHAR *Clauer::get_model (void)
{
    return model;
}

inline CK_CHAR *Clauer::get_serialNumber (void)
{
    return serialNumber;
}

inline CK_FLAGS Clauer::get_flags (void)
{
    return flags;
}

inline CK_ULONG Clauer::get_ulMaxSessionCount (void)
{
    return ulMaxSessionCount;
}

inline CK_ULONG Clauer::get_ulSessionCount (void)
{
    return ulSessionCount;
}

inline void Clauer::set_ulSessionCount (CK_ULONG value)
{
    ulSessionCount = value;
}

inline CK_ULONG Clauer::get_ulMaxRwSessionCount (void)
{
    return ulMaxRwSessionCount;
}

inline CK_ULONG Clauer::get_ulRwSessionCount (void)
{
    return ulRwSessionCount;
}

inline CK_ULONG Clauer::get_ulMaxPinLen (void)
{
    return ulMaxPinLen;
}

inline CK_ULONG Clauer::get_ulMinPinLen (void)
{
    return ulMinPinLen;
}

inline CK_ULONG Clauer::get_ulTotalPublicMemory (void)
{
    return ulTotalPublicMemory;
}

inline CK_ULONG Clauer::get_ulFreePublicMemory (void)
{
    return ulFreePublicMemory;
}

inline CK_ULONG Clauer::get_ulTotalPrivateMemory (void)
{
    return ulTotalPrivateMemory;
}

inline CK_ULONG Clauer::get_ulFreePrivateMemory (void)
{
    return ulFreePrivateMemory;
}

inline CK_VERSION Clauer::get_hardwareVersion (void)
{
    return hardwareVersion;
}

inline CK_VERSION Clauer::get_firmwareVersion (void)
{
    return firmwareVersion;
}

inline CK_CHAR *Clauer::get_utcTime (void)
{
    return utcTime;
}

#endif
