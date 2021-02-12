
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
 * Description: Contains the functions related to clauer token, it 
 *              handles objects and Clauer initialization. 
 *              At first we support just one Clauer, and 
 *              clauer supports just one mechanism for 
 *              Signing and ciphering/ deciphering with 
 *              RSA keys.
 * 
 * Notes: Windows code is not been implemented yet. 
 *  
 * 
 *                         Clauer Team 2006 
 **/               


#include "common.h" // padding function.
#include "clauer.h"
#include "log.h"

#include <openssl/bio.h>

#include <stdlib.h>
#include <stdio.h>



#ifdef WIN32

#ifdef CLUI
#include "clui.h"
#endif

#include <openssl/rand.h>
#include <openssl/x509.h>
#include <openssl/err.h>

#define snprintf _snprintf

#endif



Clauer::Clauer(CK_CHAR *_label,
	       CK_CHAR *_manufacturerID,
	       CK_CHAR *_model,
	       CK_CHAR *_serialNumber,
	       CK_FLAGS _flags,
	       CK_ULONG _ulMaxSessionCount,
	       CK_ULONG _ulSessionCount,
	       CK_ULONG _ulMaxRwSessionCount,
	       CK_ULONG _ulRwSessionCount,
	       CK_ULONG _ulMaxPinLen,
	       CK_ULONG _ulMinPinLen,
	       CK_ULONG _ulTotalPublicMemory,
	       CK_ULONG _ulFreePublicMemory,
	       CK_ULONG _ulTotalPrivateMemory,
	       CK_ULONG _ulFreePrivateMemory,
	       CK_VERSION *_hardwareVersion,
	       CK_VERSION *_firmwareVersion,
	       CK_CHAR *_utcTime)
{
  // Ciphering and signature RSA mechanism.
  pMechanism = new Mechanism(CKM_RSA_PKCS,
			     MIN_KEY_SIZE,	// Minimum key size
			     MAX_KEY_SIZE,	// Maximum key size
			     CKF_HW | CKF_DECRYPT | CKF_SIGN | CKF_ENCRYPT // Flags: Hardware mechanism for Encrypting, 	      
			     );

  // Ciphering and signature RSA mechanism.
  pMechanism_SHA1_PKCS = new Mechanism(CKM_SHA1_RSA_PKCS,
				       MIN_KEY_SIZE,	// Minimum key size
				       MAX_KEY_SIZE,	// Maximum key size
				       CKF_HW | CKF_DECRYPT | CKF_SIGN | CKF_ENCRYPT // Flags: Hardware mechanism for Encrypting, 	      
				       );

  // RSA Key Generation Mechanism 
  pMechanism_keyGen = new Mechanism(CKM_RSA_PKCS_KEY_PAIR_GEN,
				    MIN_KEY_SIZE,	// Minimum key size
				    MAX_KEY_SIZE,	// Maximum key size
				    CKF_HW | CKF_DECRYPT | CKF_SIGN | CKF_ENCRYPT // Flags: Here really doesn't matter. 
				    );


    
  padding((char *)label,(char *)_label,32);
  padding((char *)manufacturerID,(char *)_manufacturerID,32);
  padding((char *)model,(char *)_model,16);
  padding((char *)serialNumber,(char *)_serialNumber,16);
  flags = _flags;
  ulMaxSessionCount   = _ulMaxSessionCount;
  ulSessionCount      = _ulSessionCount;
  ulMaxRwSessionCount = _ulMaxRwSessionCount;
  ulRwSessionCount    = _ulRwSessionCount;
  ulMaxPinLen         = _ulMaxPinLen;
  ulMinPinLen         = _ulMinPinLen;
  ulTotalPublicMemory  = _ulTotalPublicMemory;
  ulFreePublicMemory   = _ulFreePublicMemory;
  ulTotalPrivateMemory = _ulTotalPrivateMemory;
  ulFreePrivateMemory  = _ulFreePrivateMemory;
  hardwareVersion.major = _hardwareVersion->major;
  hardwareVersion.minor = _hardwareVersion->minor;
  firmwareVersion.major = _firmwareVersion->major;
  firmwareVersion.minor = _firmwareVersion->minor;
  memcpy(utcTime,_utcTime,16);

  _isLogged= false;
  _objectsLoaded= false;
  _initialized= false;
  memset( _lastClauerId, 0, 20 );
  memset(_pin, 0, MAX_PIN_LEN);
  memset(_actualClauer, 0, MAX_PATH_LEN);

  memset( _pin, 0, MAX_PIN_LEN );

}

Clauer::~Clauer()
{
  //delete pListaMecanismos;
}

unsigned char * _getIdFromPrivateKey( RSA * rsa );
RSA * _getRSAkeyFromBlock( unsigned char * key, int tam );


CK_RV Clauer::C_GetTokenInfo (CK_TOKEN_INFO_PTR pInfo)
{
  memcpy(pInfo->label,label,32);
  memcpy(pInfo->manufacturerID,manufacturerID,32);
  memcpy(pInfo->model,model,16);
  memcpy(pInfo->serialNumber,serialNumber,16);
  pInfo->flags = flags;
  pInfo->ulMaxSessionCount   = ulMaxSessionCount;
  pInfo->ulSessionCount      = ulSessionCount;
  pInfo->ulMaxRwSessionCount = ulMaxRwSessionCount;
  pInfo->ulRwSessionCount    = ulRwSessionCount;
  pInfo->ulMaxPinLen = ulMaxPinLen;
  pInfo->ulMinPinLen = ulMinPinLen;
  pInfo->ulTotalPublicMemory  = ulTotalPublicMemory;
  pInfo->ulFreePublicMemory   = ulFreePublicMemory;
  pInfo->ulTotalPrivateMemory = ulTotalPrivateMemory;
  pInfo->ulFreePrivateMemory  = ulFreePrivateMemory;
  pInfo->hardwareVersion.major = hardwareVersion.major;
  pInfo->hardwareVersion.minor = hardwareVersion.minor;
  pInfo->firmwareVersion.major = firmwareVersion.major;
  pInfo->firmwareVersion.minor = firmwareVersion.minor;
  memcpy(pInfo->utcTime,utcTime,16);

  return CKR_OK;
}


CK_RV Clauer::C_GetMechanismList (CK_MECHANISM_TYPE_PTR pMechanismList, CK_ULONG_PTR pulCount)
{
  if (pMechanismList)
    {
      // If the list is large enough
      if (*pulCount >= 3)
	{
	  pMechanismList[0] = pMechanism->get_type();
	  pMechanismList[1] = pMechanism_keyGen->get_type();	       
	  pMechanismList[2] = pMechanism_SHA1_PKCS->get_type();			
	}
      else
	{
	  return CKR_BUFFER_TOO_SMALL;
	}
    }
  // Just the number of mechanisms.
  else{
    *pulCount=3;
  }
    
  return CKR_OK;
}


CK_RV Clauer::C_GetMechanismInfo(CK_MECHANISM_TYPE type, CK_MECHANISM_INFO_PTR pInfo)
{
  // We have only two mechanism, return corresponding info.
  if (pMechanism->get_type() == type){
    return pMechanism->C_GetMechanismInfo(pInfo);
  }
  else if ( pMechanism_keyGen->get_type() == type ) {
    return pMechanism_keyGen->C_GetMechanismInfo(pInfo);
  }
	
  return CKR_MECHANISM_INVALID;
}

CK_RV Clauer::C_SetPIN(CK_SESSION_HANDLE hSession, CK_CHAR_PTR pOldPin, CK_ULONG ulOldLen,
		       CK_CHAR_PTR pNewPin, CK_ULONG ulNewLen){
    
  unsigned char * dispositivos[MAX_DEVICES];
  int nDisp;
  USBCERTS_HANDLE handleClauer; 
    
    
  LOG_Msg(LOG_TO,"Pasando por C_SetPIN"); 
    
    
  if ( LIBRT_ListarDispositivos(&nDisp,dispositivos) != 0 ){	

    LOG_MsgError(LOG_TO,"Listando dispositivos para cambiar el pin"); 

    return CKR_FUNCTION_FAILED;
  }

    
  if ( LIBRT_IniciarDispositivo(dispositivos [0], (char *)pOldPin, &handleClauer) != 0 ){       

    LOG_MsgError(LOG_TO,"Iniciando el dispositivo para cambiar el pin"); 

    return CKR_PIN_INCORRECT;
  }

    
  if ( ulNewLen > MAX_PIN_LEN ){
	
    LOG_MsgError(LOG_TO,"Pin demasiado largo"); 

    return CKR_PIN_LEN_RANGE;
  }

  if ( LIBRT_CambiarPassword(&handleClauer, (char *) pNewPin) != 0){
	
    LOG_MsgError(LOG_TO,"Cambiando el pin del dispositivo"); 

    return CKR_FUNCTION_FAILED;
  }

    
  if ( LIBRT_FinalizarDispositivo(&handleClauer) != 0){
	
    LOG_Msg(LOG_TO,"Finalizando el dispositivo (Finalizando por timeout)"); 
   // return CKR_FUNCTION_FAILED;
  }

  strncpy(_pin, (char *)pNewPin, ulNewLen);
  //_pinLen= ulNewLen;

  LOG_Msg(LOG_TO,"Saliendo por el OK"); 
    
  return CKR_OK;
}


CK_RV Clauer::C_Login(CK_CHAR_PTR pin, CK_ULONG _pinLen, listaObjetos * pListaObjetos )
{
  unsigned char * dispositivos[MAX_DEVICES], _block[TAM_BLOQUE], * cka_id;
  int nDisp, nBlock, found= 0, func=1, res;
  USBCERTS_HANDLE handle; 
 
  char  pinAux[MAX_PIN_LEN];
    
    
  if ( ( _pinLen  == 0 ) || ( pin == NULL ) ){
    LOG_Msg(LOG_TO, "Function failed por _pinLen==0 o pin==NULL"); 
    return  CKR_FUNCTION_FAILED;
  }
    

  if ( _isLogged ){
    LOG_Msg(LOG_TO, "Usuario ya logeado"); 
    return CKR_USER_ALREADY_LOGGED_IN;
  }
    
    
  if ( _pinLen >= MAX_PIN_LEN ){
    LOG_Debug( LOG_TO,"El pin excede de la longitud máxima len= %d" , _pinLen );
    return  CKR_FUNCTION_FAILED;
    
  }
    
  memcpy(pinAux,pin,_pinLen);
  pinAux[_pinLen]= '\0';    
    

  if ( (res=LIBRT_ListarDispositivos(&nDisp, dispositivos)) != 0 ){
    SecureZeroMemory(pinAux, _pinLen);
    LOG_Debug(LOG_TO,"res: %d", res );
    LOG_MsgError(LOG_TO, "Listando dispositivos"); 
    return CKR_FUNCTION_FAILED; 
  }
    
  if ( nDisp == 0 ){
    SecureZeroMemory(pinAux, _pinLen);
    LOG_MsgError(LOG_TO, "No se detectaron clauers conectados en el equipo"); 
    return CKR_DEVICE_REMOVED; 
  }
        
  if ( LIBRT_IniciarDispositivo(dispositivos[0], (char *)pinAux, &handle) != 0 ){
    SecureZeroMemory(pinAux, _pinLen);
	LOG_Debug(LOG_TO, "_pinLen= %d", _pinLen); 
    LOG_MsgError(LOG_TO, "Iniciando dispositivo"); 
    return CKR_PIN_INCORRECT; 
  }
    
  // TODO: QUITAR ESTO
  //LOG_Debug(LOG_TO, "Dispositivo iniciado correctamente con password %s", pinAux);

  /* Here we must load private keys with no asociate certificate so 
     in case of key generation and certificate requests, it is necesary.
     Steps:
     - Loop ( While end of clauer reached )
     . Look for a private key in the clauer.
     . Get its keyid as in the clauer.
     . Look for it into the object list.
     . If it is not, calculate sha1(n) let n be the 
     modulus of the private key
     . Insert it into the object list.
     . Load also the public key of that private key.
    

     End
  */
    
  nBlock= 0;
  while ( ( LIBRT_LeerTipoBloqueCrypto ( &handle, BLOQUE_LLAVE_PRIVADA, 
					 func, _block, (long *) &nBlock) != ERR_LIBRT_SI) && ( nBlock != -1 ) ) {
	
    pListaObjetos->iniciarBusqueda();
    while( !pListaObjetos->esFin() ){	    
      if ( pListaObjetos->elementoActual()->get_class() == CKA_PRIVATE ){
		
	if ( memcmp(BLOQUE_LLAVEPRIVADA_Get_Id(_block), 
		    ((ClaveRSA *)(pListaObjetos->elementoActual()))->get_keyId(), 20) == 0){
	  found= 1;
	  break;
	}
      }
      pListaObjetos->avanzar();
    }
	
    pListaObjetos->iniciarBusqueda();	
	
	
    if (!found){
	    
      RSA * rsa= _getRSAkeyFromBlock(BLOQUE_LLAVEPRIVADA_Get_Objeto(_block), BLOQUE_LLAVEPRIVADA_Get_Tam(_block) );	    
	    
      cka_id= _getIdFromPrivateKey(rsa);
	    
      if ( cka_id == NULL ){
	LOG_MsgError(LOG_TO,"No se pudo insertar llave privada");
	RSA_free(rsa);
	continue;
      }
	    
      _insertPublicKeyObject(rsa, NULL, 0, cka_id, pListaObjetos, NULL, NULL, 0 );    	  	    	    	    	    
      _insertKeyObject( pListaObjetos, ( CK_BYTE * )cka_id, 20, 
			BLOQUE_LLAVEPRIVADA_Get_Id(_block ), NULL, NULL, 0, NULL);	

      RSA_free(rsa);
    }
	
    SecureZeroMemory( _block, TAM_BLOQUE );
    found= 0;
    func= 0;
  }
    
  strncpy(_pin, (char *)pin, _pinLen);
  _isLogged= true;
  LOG_Msg(LOG_TO, "Poniendo _isLogged a true");
        
  if ( LIBRT_FinalizarDispositivo( &handle )  != 0 ){
    LOG_MsgError(LOG_TO, "Finalizando dispositivo (Finalizando por timeout)"); 
  }
    
  LOG_Msg(LOG_TO, "Despues de finalizar fispositivo");
        
  SecureZeroMemory(pinAux, _pinLen);

  LOG_Msg(LOG_TO, "Devolviendo CKR_OK");

  return CKR_OK;
}


CK_RV Clauer::C_Logout(void){
    
  if ( _isLogged ){
    LOG_Msg(LOG_TO, "Hacemos logout en la sesion "); 
    _isLogged= false;
    memset(_pin, 0, MAX_PIN_LEN);
    return CKR_OK;
  }
  else{      
    LOG_Msg(LOG_TO, "Logout sin haber logeado "); 
    return CKR_USER_NOT_LOGGED_IN;
  }
}


CK_RV Clauer::C_CloseSession(void)
{
  // Really, close session do nothing, because pin 
  // should be smashed when logout.
    
  // if ( _isLogged ){
  //	 _isLogged= false;
  //	 memset(_pin, 0, MAX_PIN_LEN);
  //  }
  return CKR_OK;
}


//
// This function checks if the clauer plugged in has changed 
// in order to reload objects from the new clauer it checks 
// that using clauer identifier.
//
// return values are:
//      1  --> everything ok
//      2  --> no clauers on system.
//     -1  --> an error has happened.
//

int Clauer::getActiveClauer(char activeClauer[MAX_PATH_LEN] )
{
  //    USBCERTS_HANDLE  auxhandle;
  unsigned char * dispositivos[MAX_DEVICES];
  int err, nDispositivos;
    
    
  err= LIBRT_ListarDispositivos(&nDispositivos,dispositivos);
  if ( err == ERR_LIBRT_SI){
	
    LOG_MsgError(LOG_TO,"Error listando dispositivos"); 
	
    return -1;
  }
   
  if ( nDispositivos == 0 ){
    // No clauers, no objects, everything ok.
	
    LOG_Msg(LOG_TO,"No se encontraron dispositivos en el sistema"); 

    return 2;
  }
    
  strncpy(activeClauer,( const char * )dispositivos[0], MAX_PATH_LEN);
  return 1;
}


CK_RV Clauer::getRSAKey( unsigned char * keyId, RSA ** rsa, ID_ACCESS_HANDLE * th_id_handle  ){

  int res, keyFound= 0;
  unsigned char _bKey[TAM_BLOQUE];
  long nBlock= 0;
  char activeClauer[MAX_PATH_LEN];
  USBCERTS_HANDLE handle;      // Handle to the device.
  char keyIdAux[41];
  int i, ciphered= 0, pass_tries= 3;

#ifdef WIN32
#ifdef CLUI
  char szPass[CLUI_MAX_PASS_LEN];
  char szDescription[] = "Inserte el password de la llave.", buff[100];
  char str_id[41];
  map<string, PASS_CACHE_INFO *>::const_iterator iKey;
  PASS_CACHE_INFO * paux;
#endif
#endif

  LOG_Msg(LOG_TO, "Obtenemos la llave privada " );
  BIO * keyBio;
  *(rsa)= RSA_new();
    
    
  for ( i= 0; i< 20; i++){
    sprintf(keyIdAux+i*2,"%02x",keyId[i]);
  }
   
  keyIdAux[40]= 0;
    
  LOG_Debug(LOG_TO,"%s",keyIdAux);
    
  if ( strlen(_pin) == 0 || _isLogged == false ){
    LOG_Msg(LOG_TO, "Devolvemos CKR_FUNCTION_FAILED en strlen(_pin) == 0 || _isLogged == false" );
    return CKR_FUNCTION_FAILED; 
  }

  res= getActiveClauer(activeClauer);

  if (  res == 1  ){
    LIBRT_IniciarDispositivo( ( unsigned char * ) activeClauer, _pin, &handle);
  } else{	
    LOG_MsgError(LOG_TO,"Error detectando cambio de clauer"); 
    return CKR_FUNCTION_FAILED;
  }

  /* Primero buscamos las claves privadas convencionales */
  if ( LIBRT_LeerTipoBloqueCrypto(&handle, BLOQUE_LLAVE_PRIVADA, 1, _bKey, (long *) &nBlock) != 0 ) {
    LOG_MsgError(LOG_TO,"Accediendo al clauer");
    LIBRT_FinalizarDispositivo(&handle);
    LOG_Msg(LOG_TO, "Leyendo tipo bloque crypto" );
    return CKR_FUNCTION_FAILED;
  }
  LOG_Msg(LOG_TO, "3" );
  LOG_Msg(LOG_TO, "Despues de leerTipoBloqueCrypto");
    
  while ( ( ! keyFound ) && (nBlock != -1) ) {
    if ( memcmp(BLOQUE_LLAVEPRIVADA_Get_Id(_bKey),
		keyId,
		20) == 0 ) {

      ciphered = 0; 
      keyFound = 1;
    } 
    else{
	
      SMEM_Destroy((void *) _bKey, TAM_BLOQUE);
      if ( LIBRT_LeerTipoBloqueCrypto(&handle, BLOQUE_LLAVE_PRIVADA, 0, _bKey,(long *) &nBlock) != 0 ) {

	LOG_MsgError(LOG_TO,"Accediendo al clauer");

	LIBRT_FinalizarDispositivo(&handle);
	return CKR_FUNCTION_FAILED;
      } 
    } 
  }

#ifdef WIN32
#ifdef CLUI
  /* Ahora buscamos las claves que poseen otro password extra */
  nBlock= -1;
  LOG_Msg(LOG_TO, "2" );
  if ( LIBRT_LeerTipoBloqueCrypto(&handle, BLOQUE_CIPHER_PRIVKEY_PEM, 1, _bKey, (long *) &nBlock) != 0 ) {
    LOG_MsgError(LOG_TO,"Accediendo al clauer");
    LIBRT_FinalizarDispositivo(&handle);
    LOG_Msg(LOG_TO, "Leyendo tipo bloque crypto" );
    return CKR_FUNCTION_FAILED;
  }
  LOG_Msg(LOG_TO, "3" );
  LOG_Msg(LOG_TO, "Despues de leerTipoBloqueCrypto");
    
  while ( ( ! keyFound ) && (nBlock != -1) ) {
    if ( memcmp(BLOQUE_CIPHER_PRIVKEY_PEM_Get_Id(_bKey),
		keyId,
		20) == 0 ) {

      ciphered = 1; 
      keyFound = 1;
    } 
    else{
	
      SMEM_Destroy((void *) _bKey, TAM_BLOQUE);
      if ( LIBRT_LeerTipoBloqueCrypto(&handle, BLOQUE_CIPHER_PRIVKEY_PEM, 0, _bKey,(long *) &nBlock) != 0 ) {

	LOG_MsgError(LOG_TO,"Accediendo al clauer");

	LIBRT_FinalizarDispositivo(&handle);
	return CKR_FUNCTION_FAILED;
      } 
    } 
  }

  if ( LIBRT_FinalizarDispositivo(&handle) != 0 ) {

    LOG_MsgError(LOG_TO,"Error Finalizando dispositivo al leer la llave privada");

    if ( keyFound ) SMEM_Destroy(_bKey, TAM_BLOQUE);
    return CKR_FUNCTION_FAILED;
  }

  /*********************************/
#endif
#endif

  if ( LIBRT_FinalizarDispositivo(&handle)!= 0 ){
	  LOG_MsgError(1,"Error finalizando el dispositivo.");
  }

  if ( keyFound != 1 ){
      return CKR_FUNCTION_FAILED;
  }

  if ( ciphered ){
#ifdef WIN32
#ifdef CLUI
    keyBio= BIO_new_mem_buf(BLOQUE_CIPHER_PRIVKEY_PEM_Get_Objeto(_bKey), BLOQUE_CIPHER_PRIVKEY_PEM_Get_Tam(_bKey));
    *szPass= 0;
    BLOQUE_CIPHER_PRIVKEY_PEM_Get_String_Id(_bKey,(unsigned char *) str_id);
		
    WaitForSingleObject(th_id_handle->keyIdMutex, INFINITE);
    iKey = th_id_handle->keyCache.find(str_id);
    if ( iKey != th_id_handle->keyCache.end() ) {
      strncpy(szPass, th_id_handle->keyCache[iKey->first]->pass, 127);
    }
	      
    ReleaseMutex(th_id_handle->keyIdMutex);
	

    if ( *szPass == 0 ){       			
      while (pass_tries--){
	CLUI_AskDoublePassphrase ( NULL,
				   szDescription,
				   szPass );
	*(rsa) = PEM_read_bio_RSAPrivateKey ( keyBio, NULL,NULL, szPass ); 
	if ( *(rsa) ){
	  /* Insertamos la password en la cache */
	  paux= new PASS_CACHE_INFO;  
	  memcpy( paux->id, BLOQUE_CIPHER_PRIVKEY_PEM_Get_Id(_bKey), 20 );
	  strncpy( paux->pass, szPass, 128 );
	  paux->ttl= 20; /* This is the time * 6 in seconds*/
	  WaitForSingleObject(th_id_handle->keyIdMutex, INFINITE);
	  th_id_handle->keyCache[str_id] = paux;
	  ReleaseMutex(th_id_handle->keyIdMutex);

	  WaitForSingleObject(th_id_handle->clauerIdMutex, INFINITE);
	  th_id_handle->global_pass_ttl= 20;//50;
	  ReleaseMutex(th_id_handle->clauerIdMutex);
				
	  break;
	}
      }
    }
    else{
      *(rsa) = PEM_read_bio_RSAPrivateKey ( keyBio, NULL,NULL, szPass ); 
    }

    if (!*(rsa)){
      return CKR_FUNCTION_FAILED;
    }
#endif
#endif
  }
  else{
    keyBio= BIO_new_mem_buf(BLOQUE_LLAVEPRIVADA_Get_Objeto(_bKey), BLOQUE_LLAVEPRIVADA_Get_Tam(_bKey));
    *(rsa) = PEM_read_bio_RSAPrivateKey(keyBio, NULL, 0, NULL);
  }
    
  if ( BIO_free(keyBio) == 0 ) {

    LOG_MsgError(LOG_TO,"Error Finalizando dispositivo al leer la llave privada");
	
    RSA_free(*(rsa));
    SMEM_Destroy(_bKey, TAM_BLOQUE);
                   
    return CKR_FUNCTION_FAILED;
  }
    
  if ( *(rsa) == NULL ){
    SMEM_Destroy(_bKey, TAM_BLOQUE);
    LOG_Msg(LOG_TO, "al leer la llave del bio" );
    return CKR_FUNCTION_FAILED; ;
  }
   
  // Private key is also in the block, we must destroy it 
  SMEM_Destroy(_bKey, TAM_BLOQUE);
  LOG_Msg(LOG_TO, "LLAVE PRIVADA OBTENIDA OK" );
  return CKR_OK;
}




char * Clauer::_getKeyUsage(X509 * xcert){
	// From Juan's CryptoWrapper
	ASN1_BIT_STRING *usage = NULL;
	unsigned long keyUsage;
	char *ret; 
	int len;

	if((usage=(ASN1_BIT_STRING *)X509_get_ext_d2i(xcert, NID_key_usage, NULL, NULL))) {
		if(usage->length > 0) {
			keyUsage = usage->data[0];
			if(usage->length > 1)
				keyUsage |= usage->data[1] << 8;

			if ( ( keyUsage & CRYPTO_KU_DIGITAL_SIGNATURE ) ||
				( keyUsage & CRYPTO_KU_KEY_CERT_SIGN ) ||
				( keyUsage & CRYPTO_KU_CRL_SIGN ) ){
					len=strlen("FIRMA");
					ret= (char*)malloc( len + 1 );
					strncpy(ret,"FIRMA",len);
					ret[len]=0;
					return ret;
			}
			if ( ( keyUsage & CRYPTO_KU_KEY_ENCIPHERMENT ) ||
				( keyUsage & CRYPTO_KU_DATA_ENCIPHERMENT ) ||
				( keyUsage & CRYPTO_KU_KEY_AGREEMENT ) ||
				( keyUsage & CRYPTO_KU_ENCIPHER_ONLY ) ||
				( keyUsage & CRYPTO_KU_DECIPHER_ONLY ) ){

					len=strlen("CIFRADO");
					ret= (char*)malloc( len + 1 );
					strncpy(ret,"CIFRADO",len);
					ret[len]=0;
					return ret;
			}
		}
	}
	len= strlen("PROP. DESCONOCIDO");
	ret= (char*)malloc( len + 1 );
	strncpy(ret,"PROP. DESCONOCIDO",len);
	ret[len]=0;
	return ret;		
}

CK_RV Clauer::_insertCertificateObjectAndPublicKey( unsigned char * block, listaObjetos * pListaObjetos,   
						    CK_BYTE * identificador, int tamId, CK_OBJECT_HANDLE_PTR phObject,
						    unsigned char * certId, char **label, int insertPrivKey){
  BIO * b;
  X509 * cert= X509_new();
  int len;
  unsigned int longitudCertificado;	    // Certificate length  
  unsigned char * _certificado;	    // Buffer where hold the certificate. 
    
  CK_BYTE_PTR certx509 = NULL;	// Certificado
  CertificadoX509 *pCertificado = NULL;	// Objeto certificado segun la especificacion PKCS#11	
    
  char commonName[1024];	// Nombre de la persona que posee el certificado
  int lcommonName;		// Longitud del nombre de la persona que posee ese certificad
    
  X509 *pX509 = NULL;		// Estructura X.509
  X509_NAME *pname = NULL;	// Para acceder al campo nombre de la entidad que posee el certificado
  X509_NAME *pissuer = NULL;	// Idem, para el campo issuer
  ASN1_INTEGER *pserialNumber = NULL;
  
  void *auxiliar = NULL;
  EVP_PKEY * publicKey;
  ClaveRSA *pClaveRSA = NULL;	

  char  cabeceraEtiqueta[100];
 
  char * strUsage;
  char * labelAux= (char *)"";
	  
  DWORD lang= IDIOMA_Obtener();

  switch ( lang ){
  case ES:
    strncpy(cabeceraEtiqueta," Certificado en Clauer: ",24);
    cabeceraEtiqueta[24]= '\0';
    break;
  case CA:
    strncpy(cabeceraEtiqueta," Certificat en Clauer: ",23);
    cabeceraEtiqueta[23]= '\0';
    break;
  default:
    strncpy(cabeceraEtiqueta," Certificate on Clauer: ",24);
    cabeceraEtiqueta[24]= '\0';
  }
  

  b= BIO_new_mem_buf(BLOQUE_CERTPROPIO_Get_Objeto(block), BLOQUE_CERTPROPIO_Get_Tam(block));
  cert= PEM_read_bio_X509(b,NULL,0,NULL);
    
  if ( cert==NULL ){       
    LOG_MsgError(LOG_TO, "Houston tenemos un problema en la lectura del BIO"); 
    return CKR_FUNCTION_FAILED;
  }

  _certificado= NULL;
  len= i2d_X509(cert,&_certificado);
  

  longitudCertificado= len;                

  if (_certificado == NULL){	
    LOG_MsgError(LOG_TO,"ERROR en la lectura del certificado\n"); 
    return CKR_FUNCTION_FAILED;
  }
  
  certx509 = _certificado;

  // Creamos una estructura del tipo X.509
  if ((pX509 = X509_new()) == NULL)
    {
      LOG_MsgError(LOG_TO,"Memoria disponible");
      return CKR_HOST_MEMORY;
    }
    

  // Transformamos el certificado de formato DER a formato interno
  auxiliar = certx509;
  
#if (OPENSSL_VERSION_NUMBER <=  0x009070afL)//0x0090709fL) 
  if (d2i_X509(&pX509,(unsigned char**)&_certificado,longitudCertificado) == NULL)
#else
    if (d2i_X509(&pX509,(const unsigned char**)&_certificado,longitudCertificado) == NULL)
    //if (d2i_X509(&pX509,( unsigned char**)&_certificado,longitudCertificado) == NULL)
#endif
      {		
	LOG_MsgError(LOG_TO,"Certificado incorrecto");
	X509_free(pX509);
	return CKR_FUNCTION_FAILED;
      }
  // La funcion d2i_X509 modifica el puntero certx509. Lo restauramos a su valor original
 


   certx509 = (unsigned char *) auxiliar;
  
  /**************************************************************************************/
  // Objeto certificado.
  // Va a tener los siguientes atributos (11):
  // - class
  // - token
  // - private
  // - modifiable
  // - label
  // - certificate type
  // - subject
  // - id
  // - issuer
  // - serial number
  // - value
  
  CK_OBJECT_CLASS clase = CKO_CERTIFICATE;
  CK_BBOOL _true = TRUE;
  CK_BBOOL _false = FALSE;
  CK_CERTIFICATE_TYPE tipoCertificado = CKC_X_509;
  


  CK_ATTRIBUTE plantillaCertificado[] = {
    {CKA_CLASS,&clase,sizeof(clase)},
    {CKA_TOKEN,&_true,sizeof(_true)},
    {CKA_PRIVATE,&_false,sizeof(_false)},
    {CKA_MODIFIABLE,&_false,sizeof(_false)},
    {CKA_LABEL,NULL_PTR,0},
    {CKA_CERTIFICATE_TYPE,&tipoCertificado,sizeof(tipoCertificado)},
    {CKA_SUBJECT,NULL_PTR,0},
    {CKA_ID,identificador,tamId},
    {CKA_ISSUER,NULL_PTR,0},
    {CKA_SERIAL_NUMBER,NULL_PTR,0},
    {CKA_VALUE,NULL_PTR,0},
  };
  
 
  if ( (pname = X509_get_subject_name(pX509)) == NULL)
    {
      X509_free(pX509);
      LOG_MsgError(LOG_TO,"Al obtener en subject del certificado");
      return CKR_FUNCTION_FAILED;
    }
  
  if ( (lcommonName = X509_NAME_get_text_by_NID(pname, NID_commonName, commonName, 1024)) <= 0)
    {
      X509_free(pX509);
      LOG_MsgError(LOG_TO,"Al obtener el common name del certificado");
      return CKR_FUNCTION_FAILED;
    }
 
    strUsage= _getKeyUsage(pX509);
//The label purpose compilation was intended for ACCV. 
#ifdef LABEL_PURPOSE
    plantillaCertificado[4].ulValueLen = strlen(strUsage) + (CK_ULONG)strlen(cabeceraEtiqueta) + lcommonName;
    plantillaCertificado[4].pValue = malloc(plantillaCertificado[4].ulValueLen + 1 );// + 1 porque strcat va a incluir el '\0'
    
    if (plantillaCertificado[4].pValue == NULL)
	{
	    free(certx509);	    
	    return CKR_HOST_MEMORY;
	}
    
    strcpy((char *)plantillaCertificado[4].pValue,strUsage);
    strcat((char *)plantillaCertificado[4].pValue,cabeceraEtiqueta);
    strcat((char *)plantillaCertificado[4].pValue,commonName);

	if ( label != NULL ){
	  *label= (char *)malloc(plantillaCertificado[4].ulValueLen + 1);
	  strcpy((char *)*label, strUsage);
	  strcat((char *)*label, cabeceraEtiqueta);
	  strcat((char *)*label, commonName);		
	  labelAux= *label;
    }
#else
    // NOTA: al ser la etiqueta un LocalString se supone que debe ir marcada al final con el '\0'
    // Sin embargo, parece que el propio Netscape ya pone el '\0'.
    //plantillaCertificado[4].ulValueLen = (CK_ULONG)strlen(cabeceraEtiqueta) + lcommonName;

	plantillaCertificado[4].ulValueLen = lcommonName;
    plantillaCertificado[4].pValue = malloc(plantillaCertificado[4].ulValueLen + 1 );// + 1 porque strcpy va a incluir el '\0'
    if (plantillaCertificado[4].pValue == NULL)
	{
	    free(certx509);	    
	      return CKR_HOST_MEMORY;
	}
    strncpy((char *)plantillaCertificado[4].pValue, commonName, lcommonName);

	if ( label != NULL ){
	  *label= (char *)malloc(plantillaCertificado[4].ulValueLen + 1);
	  strncpy((char *)*label, commonName, lcommonName);		
	  labelAux= *label;
    }
#endif

  plantillaCertificado[6].ulValueLen = i2d_X509_NAME(pname,NULL);
  plantillaCertificado[6].pValue = malloc(plantillaCertificado[6].ulValueLen);
  if (plantillaCertificado[6].pValue == NULL)
    {
      free(plantillaCertificado[4].pValue);
      X509_free(pX509);
      return CKR_HOST_MEMORY;
    }
  // La funcion i2d_X509_NAME cambia el valor de plantillaCertificado[6].pValue
  auxiliar = plantillaCertificado[6].pValue;
  i2d_X509_NAME(pname,(unsigned char **) &(plantillaCertificado[6].pValue));
  plantillaCertificado[6].pValue = auxiliar;
  

  pissuer = X509_get_issuer_name(pX509);
  plantillaCertificado[8].ulValueLen = i2d_X509_NAME(pissuer,NULL);
  plantillaCertificado[8].pValue = malloc(plantillaCertificado[8].ulValueLen);
  if (plantillaCertificado[8].pValue == NULL)
    {
      free(plantillaCertificado[6].pValue);
      free(plantillaCertificado[4].pValue);
      X509_free(pX509);
      return CKR_HOST_MEMORY;
    }
  

  auxiliar = plantillaCertificado[8].pValue;
  i2d_X509_NAME(pissuer,(unsigned char **)&(plantillaCertificado[8].pValue));
  plantillaCertificado[8].pValue = auxiliar;

  // Serial Number  
   LOG_Msg(LOG_TO,"Serial Number");
  pserialNumber = X509_get_serialNumber(pX509);
  plantillaCertificado[9].ulValueLen = i2d_ASN1_INTEGER(pserialNumber,NULL);
  plantillaCertificado[9].pValue = malloc(plantillaCertificado[9].ulValueLen);
  if (plantillaCertificado[9].pValue == NULL)
    {
      free(plantillaCertificado[8].pValue);
      free(plantillaCertificado[6].pValue);
      free(plantillaCertificado[4].pValue);
      X509_free(pX509);
      return CKR_HOST_MEMORY;
    }
  auxiliar = plantillaCertificado[9].pValue;
  i2d_ASN1_INTEGER(pserialNumber,(unsigned char **)&(plantillaCertificado[9].pValue));
  plantillaCertificado[9].pValue = auxiliar;
  
  // Valor del certificado  
   LOG_Msg(LOG_TO,"Certificate Value");
  plantillaCertificado[10].ulValueLen = longitudCertificado;
  plantillaCertificado[10].pValue = malloc(plantillaCertificado[10].ulValueLen);
  if (plantillaCertificado[10].pValue == NULL)
    {
      free(plantillaCertificado[9].pValue);
      free(plantillaCertificado[8].pValue);
      free(plantillaCertificado[6].pValue);
      free(plantillaCertificado[4].pValue);
      X509_free(pX509);
      return CKR_HOST_MEMORY;
    }
  memcpy(plantillaCertificado[10].pValue,certx509,longitudCertificado);
  
  // Creamos un objeto de la clase Certificado X509
   LOG_Msg(LOG_TO,"X509Cert");
  pCertificado = new CertificadoX509(plantillaCertificado,11, certId);
  if (!pCertificado)
    {
      free(plantillaCertificado[10].pValue);
      free(plantillaCertificado[9].pValue);
      free(plantillaCertificado[8].pValue);
      free(plantillaCertificado[6].pValue);
      free(plantillaCertificado[4].pValue);
      X509_free(pX509);
      return CKR_HOST_MEMORY;
    }
  // Lo insertamos en la lista de objetos de la sesion
   LOG_Msg(LOG_TO,"Inserción lista Objetos");
  pListaObjetos->insertarObjeto(pCertificado);

  if (phObject!= NULL){
    *((int *)phObject)= pCertificado->get_handle();
  }
    
  LOG_Msg(LOG_TO,"Hemos insertado un certificado"); 

  
  // Liberamos espacio
  free(plantillaCertificado[10].pValue);
  free(plantillaCertificado[9].pValue);
  free(plantillaCertificado[8].pValue);
  free(plantillaCertificado[6].pValue);
  free(plantillaCertificado[4].pValue);


  // Extraemos la llave publica
  publicKey= X509_extract_key(pX509);
  if (!publicKey){
    X509_free(pX509);
    LOG_MsgError(LOG_TO, "Obteninedo la llave publica");
    return CKR_FUNCTION_FAILED;
  }
  LOG_Msg(LOG_TO, "Obtenida la llave publica");
    
  // Free the certificate
  X509_free(pX509);
 


  // Insertamos la llave publica
  RSA * pRSA= NULL;
  pRSA=  EVP_PKEY_get1_RSA(publicKey);
    
  if ( ! pRSA ){
    LOG_MsgError(LOG_TO, "Obteninedo la estructura RSA desde la llave publica");
    pListaObjetos->eliminarObjeto(pCertificado);
    return CKR_HOST_MEMORY;
  }

  LOG_Msg(LOG_TO, "Obtenida la estructura RSA desde la llave publica");
    

  /* Necesitamos el módulo y el exponente público */
  unsigned char * mod, * pex;
  int m_size, pex_size;
    
  // TODO: Some control and test of this 
  // Here we put a leading zero to assure that every application interprets 
  // the module as a positive unsigned big integer
  m_size= BN_num_bytes(pRSA->n) + 1;
  pex_size= BN_num_bytes(pRSA->e);


  mod= (unsigned char * ) malloc(m_size);
  pex= (unsigned char * ) malloc(pex_size);
  
  LOG_Msg(LOG_TO, "Reservados exponente y módulo");

  mod[0]= 0;

  if (! BN_bn2bin(pRSA->n, mod+1)){
    return CKR_FUNCTION_FAILED;
  }
    
  if (! BN_bn2bin(pRSA->e, pex)){
    return CKR_FUNCTION_FAILED;
  }
       
  LOG_Msg(LOG_TO, "Pasamos los bn2bin");
    

  // Va a tener los siguientes atributos (18):
  // - class
  // - token
  // - private
  // - modifiable
  // - label
  // - key_type
  // - id
  // - local
  // - sensitive
  // - decrypt
  // - sign
  // - sign_recover
  // - unwrap
  // - extractable
  // - always sensitive
  // - never extractable
  // - Modulo 
  // - Exponente 

  // Los valores restantes se dejan en una estructura RSA
    
  clase = CKO_PUBLIC_KEY;
  CK_CHAR etiqueta[] = "Clave publica";
  CK_KEY_TYPE tipoClave = CKK_RSA;
  CK_ATTRIBUTE plantillaPublica[] = {
    {CKA_CLASS,&clase,sizeof(clase)},
    {CKA_TOKEN,              &_true,        sizeof(_true)},
    {CKA_PRIVATE,            &_true,        sizeof(_true)},
    {CKA_MODIFIABLE,         &_false,       sizeof(_false)},
    {CKA_LABEL,              labelAux,      strlen(labelAux)},
    {CKA_KEY_TYPE,           &tipoClave,    sizeof(tipoClave)},
    {CKA_ID,                 identificador, tamId},
    {CKA_LOCAL,              &_false,       sizeof(_false)},
    {CKA_SENSITIVE,          &_true,        sizeof(_true)},
    {CKA_ENCRYPT,            &_true,        sizeof(_true)},
    {CKA_SIGN,               &_true,        sizeof(_true)},
    {CKA_SIGN_RECOVER,       &_false,       sizeof(_false)},
    {CKA_UNWRAP,             &_false,       sizeof(_false)},
    {CKA_EXTRACTABLE,        &_false,       sizeof(_false)},
    {CKA_ALWAYS_SENSITIVE,   &_true,        sizeof(_true)},
    {CKA_NEVER_EXTRACTABLE,  &_true,        sizeof(_true)},
    {CKA_MODULUS,            mod,           m_size},
    {CKA_PUBLIC_EXPONENT,    pex,           pex_size},

	
  };
    
  LOG_Msg(LOG_TO, "Pasamos la plantilla");

  pClaveRSA = new ClaveRSA((CK_ATTRIBUTE *)&plantillaPublica, 18, BLOQUE_CERTPROPIO_Get_Id( block ));
  pListaObjetos->insertarObjeto(pClaveRSA);

  LOG_Msg(LOG_TO, "Pasamos la creación/inserción");

  // Insert a private key reference with public info. 
  if (insertPrivKey){
	clase = CKO_PRIVATE_KEY;
    CK_CHAR  etiquetaDef[] = "Clave Privada";
    CK_CHAR * etiqueta= NULL;
	int atributos;

	if ( *label != NULL )
		etiqueta= (CK_CHAR *)*label;
    else
		etiqueta= etiquetaDef;
    
	LOG_Debug(LOG_TO, "AQUÍ: Metiendo label a la llave privada= %s", etiqueta);

    CK_KEY_TYPE tipoClave = CKK_RSA;
    CK_ATTRIBUTE plantillaPrivada[] = {
      {CKA_CLASS,             &clase,        sizeof(clase)},
      {CKA_TOKEN,             &_true,        sizeof(_true)},
      {CKA_PRIVATE,           &_true,        sizeof(_true)},
      {CKA_MODIFIABLE,        &_false,       sizeof(_false)},
      {CKA_LABEL,             labelAux,      strlen(labelAux)},
      {CKA_KEY_TYPE,          &tipoClave,    sizeof(tipoClave)},
      {CKA_ID,                identificador, tamId},
      {CKA_LOCAL,             &_false,       sizeof(_false)},
      {CKA_SENSITIVE,         &_true,        sizeof(_true)},
      {CKA_DECRYPT,           &_false,       sizeof(_true)},
      {CKA_SIGN,              &_true,        sizeof(_true)},
      {CKA_SIGN_RECOVER,      &_false,       sizeof(_false)},
      {CKA_UNWRAP,            &_false,       sizeof(_false)},
      {CKA_EXTRACTABLE,       &_false,       sizeof(_false)},
      {CKA_ALWAYS_SENSITIVE,  &_true,        sizeof(_true)},
      {CKA_NEVER_EXTRACTABLE, &_true,        sizeof(_true)},
      {CKA_MODULUS,           mod,           m_size},
	  {CKA_PUBLIC_EXPONENT,   pex,           pex_size},
      //{CKA_VALUE,NULL_PTR,0}
    };

	//atributos= 16;
    atributos= 18;
    pClaveRSA = new ClaveRSA((CK_ATTRIBUTE *)&plantillaPrivada, atributos, certId);
	LOG_Debug(LOG_TO,"Insertada clave privada con etiqueta= %s", etiqueta);
  
	pListaObjetos->insertarObjeto(pClaveRSA);
  
  
  }

  return CKR_OK;
}


int Clauer::_insertKeyObject( listaObjetos * pListaObjetos, CK_BYTE * identificador, 
			      int tamIdentificador, unsigned char * certId, 
			      CK_OBJECT_HANDLE_PTR phObject, CK_ATTRIBUTE_PTR pPrivateKeyTemplate, 
			      CK_ULONG ulPrivateKeyAttributeCount, char * label ){
  
  ClaveRSA *pClaveRSA = NULL;		// Objeto clave privada del tipo RSA

  CK_BBOOL _true = TRUE;
  CK_BBOOL _false = FALSE;
  CK_ULONG i;

  int atributos;
    
    
  // Los valores restantes se dejan en una estructura RSA    
  // TODO: ACCV --> Intentar aquí lo de los CKA_LABEL firma/cifrado.
  if (! pPrivateKeyTemplate ){
    CK_OBJECT_CLASS clase = CKO_PRIVATE_KEY;
    CK_CHAR  etiquetaDef[] = "Clave Privada";
    CK_CHAR * etiqueta= NULL;

	if ( label != NULL )
		etiqueta= (CK_CHAR *)label;
    else
		etiqueta= etiquetaDef;
    
	LOG_Debug(LOG_TO, "AQUÍ: Metiendo label a la llave privada= %s", etiqueta);

    CK_KEY_TYPE tipoClave = CKK_RSA;
    CK_ATTRIBUTE plantillaPrivada[] = {
      {CKA_CLASS,             &clase,        sizeof(clase)},
      {CKA_TOKEN,             &_true,        sizeof(_true)},
      {CKA_PRIVATE,           &_true,        sizeof(_true)},
      {CKA_MODIFIABLE,        &_false,       sizeof(_false)},
      {CKA_LABEL,             etiqueta,     strlen((const char *)etiqueta)},
      {CKA_KEY_TYPE,          &tipoClave,    sizeof(tipoClave)},
      {CKA_ID,                identificador, tamIdentificador},
      {CKA_LOCAL,             &_false,       sizeof(_false)},
      {CKA_SENSITIVE,         &_true,        sizeof(_true)},
      {CKA_DECRYPT,           &_false,       sizeof(_true)},
      {CKA_SIGN,              &_true,        sizeof(_true)},
      {CKA_SIGN_RECOVER,      &_false,       sizeof(_false)},
      {CKA_UNWRAP,            &_false,       sizeof(_false)},
      {CKA_EXTRACTABLE,       &_false,       sizeof(_false)},
      {CKA_ALWAYS_SENSITIVE,  &_true,        sizeof(_true)},
      {CKA_NEVER_EXTRACTABLE, &_true,        sizeof(_true)},
      //{CKA_MODULUS,NULL_PTR, 0},
      //{CKA_PUBLIC_EXPONENT,NULL_PTR, 0}
      //{CKA_VALUE,NULL_PTR,0}
    };

	atributos= 16;
    //atributos= 18;
    pClaveRSA = new ClaveRSA((CK_ATTRIBUTE *)&plantillaPrivada, atributos, certId);
	LOG_Debug(LOG_TO,"Insertada clave privada con etiqueta= %s", etiqueta);
	
  }
  else{
    //CK_ATTRIBUTE plantillaPrivada[ulPrivateKeyAttributeCount + 4];
    CK_ATTRIBUTE * plantillaPrivada= ( CK_ATTRIBUTE * ) malloc( sizeof(CK_ATTRIBUTE) * (ulPrivateKeyAttributeCount + 4) );
    CK_OBJECT_CLASS clase= CKO_PRIVATE_KEY;
    CK_KEY_TYPE tipoClave = CKK_RSA;
    CK_BBOOL tiene_id= FALSE;
    CK_BBOOL tiene_decrypt= FALSE;

	
    // Copy the template, that could be a realloc for eficience porpouses 
    for ( i=0; i<ulPrivateKeyAttributeCount; i++ ){
      if ( pPrivateKeyTemplate[i].type == CKA_ID ){
	tiene_id= TRUE;
      }
      if( pPrivateKeyTemplate[i].type == CKA_DECRYPT ){
	tiene_decrypt= TRUE;
	LOG_Debug( LOG_TO, "CKA_DECRYPT encontrado que vale %ld", *((CK_BBOOL *)pPrivateKeyTemplate[i].pValue) );
		
	plantillaPrivada[i].type= pPrivateKeyTemplate[i].type;
	plantillaPrivada[i].ulValueLen= sizeof(_false);
	plantillaPrivada[i].pValue= (void *) malloc(sizeof(_false));
	if (! plantillaPrivada[i].pValue ){
	  free( plantillaPrivada );
	  return CKR_HOST_MEMORY;
	}	   
	memcpy( plantillaPrivada[i].pValue, &_false, sizeof(_false)); 
      }
      else{
	plantillaPrivada[i].type= pPrivateKeyTemplate[i].type;
	plantillaPrivada[i].ulValueLen= pPrivateKeyTemplate[i].ulValueLen;
	plantillaPrivada[i].pValue= (void *) malloc(pPrivateKeyTemplate[i].ulValueLen);
	if (! plantillaPrivada[i].pValue ){
	  free( plantillaPrivada );		
	  return CKR_HOST_MEMORY;
	}	   
	memcpy( plantillaPrivada[i].pValue, pPrivateKeyTemplate[i].pValue, pPrivateKeyTemplate[i].ulValueLen); 
      }
    }

    // CLASS 
    plantillaPrivada[i].type= CKA_CLASS;
    plantillaPrivada[i].ulValueLen= sizeof(clase);
    plantillaPrivada[i].pValue= (void *) malloc(sizeof(clase));
    if (! plantillaPrivada[i].pValue ){
      free( plantillaPrivada );
      return CKR_HOST_MEMORY;
    }	   
    memcpy( plantillaPrivada[i].pValue, &clase, sizeof(clase));		


    // KEY_TYPE 
    i++;
    plantillaPrivada[i].type= CKA_KEY_TYPE;
    plantillaPrivada[i].ulValueLen= sizeof(tipoClave);
    plantillaPrivada[i].pValue= (void *) malloc(sizeof(tipoClave));
    if (! plantillaPrivada[i].pValue ){
      free( plantillaPrivada );
      return CKR_HOST_MEMORY;
    }	   
    memcpy( plantillaPrivada[i].pValue, &tipoClave, sizeof(tipoClave));		
		
	
    atributos= ulPrivateKeyAttributeCount + 2;

    /* Si no tiene CKA_ID se lo ponemos nosotros */
    if ( ! tiene_id ){
      i++;
      // CKA_ID
      LOG_Msg(LOG_TO, "No se ha encontrado CKA_ID en la plantilla privada, asignando uno interno.");
      plantillaPrivada[i].type= CKA_ID;
      plantillaPrivada[i].ulValueLen= tamIdentificador;
      plantillaPrivada[i].pValue= (void *) malloc(tamIdentificador);
      if (! plantillaPrivada[i].pValue ){
	free( plantillaPrivada );
	return CKR_HOST_MEMORY;
      }	   
      memcpy( plantillaPrivada[i].pValue, identificador, tamIdentificador );		
	    
      atributos++;
    }    

    /* Este atibuto se añade explicitamente por 
       compatibilidad con version de ffox 1.5.0.9 
       que requiere este atributo para la generación
       de claves
    */ 	
    if ( ! tiene_decrypt ){
      i++;
      // CKA_DECRYPT
      LOG_Msg(LOG_TO, "No se ha encontrado CKA_DECRYPT en la plantilla privada, asignando uno interno.");
      plantillaPrivada[i].type= CKA_DECRYPT;
      plantillaPrivada[i].ulValueLen= sizeof(_false);
      plantillaPrivada[i].pValue= (void *) malloc(sizeof(_false));
      if (! plantillaPrivada[i].pValue ){
	free(plantillaPrivada);
	return CKR_HOST_MEMORY;
      }	   
      memcpy( plantillaPrivada[i].pValue, &_false, sizeof(_false) );		
	    
      atributos++;
    }    	
	

    pClaveRSA = new ClaveRSA((CK_ATTRIBUTE *)plantillaPrivada, atributos, certId);	
    free( plantillaPrivada );
  }
    
    

  if ( phObject !=  NULL ){
    *((int *)phObject)= pClaveRSA->get_handle();	 
  }

  LOG_Debug(LOG_TO, "Insertamos llave privada con handle = %d ",pClaveRSA->get_handle() ); 
  pListaObjetos->insertarObjeto(pClaveRSA);

  return 0;
}


CK_RV Clauer::_insertPublicKeyObject( RSA * rsa, unsigned char * identificador, int tamIdentificador, unsigned char realId[20], 
				      listaObjetos * pListaObjetos, CK_OBJECT_HANDLE_PTR phObject,
				      CK_ATTRIBUTE_PTR pPublicKeyTemplate, CK_ULONG ulPublicKeyAttributeCount ){
    
  ClaveRSA *pClaveRSA = NULL;		// Objeto clave privada del tipo RSA
  CK_BBOOL _true = TRUE;
  CK_BBOOL _false = FALSE;
  CK_ULONG i= 0;

  unsigned char *mod, *pex;
  int m_size, pex_size, atributos;
    
  // TODO: Some control and test of this 
  // Here we put a leading zero to assure that every application interprets 
  // the module as a positive unsigned big integer
  //Sleep(5);
  m_size= BN_num_bytes(rsa->n) + 1;
  pex_size= BN_num_bytes(rsa->e);

  mod= (unsigned char * ) malloc(m_size);
  pex= (unsigned char * ) malloc(pex_size);
    
  mod[0]= 0;
  if (! BN_bn2bin(rsa->n, mod+1)){
    return CKR_FUNCTION_FAILED;
  }
    
  if (! BN_bn2bin(rsa->e, pex)){
    return CKR_FUNCTION_FAILED;
  }
    

  /* 
   *  Here we need to read the private key on an rsa  structure and 
   *  Get the modulus and the public exponent from it. 
   *
   */ 
    
  if ( tamIdentificador == 0 ){
    tamIdentificador= 20;
    identificador= (unsigned char * ) malloc(20);
    if (! identificador ){
      return CKR_FUNCTION_FAILED;
    }

    memcpy (identificador, realId, 20);       
  }
    

  // Los valores restantes se dejan en una estructura RSA
  if (! pPublicKeyTemplate ){
    CK_OBJECT_CLASS clase= CKO_PUBLIC_KEY;
    CK_CHAR etiqueta[] = "Clave publica";
    CK_KEY_TYPE tipoClave = CKK_RSA;
    CK_ATTRIBUTE plantillaPublica[] = {
      {CKA_CLASS,               &clase,        sizeof(clase)},
      {CKA_TOKEN,               &_true,        sizeof(_true)},
      {CKA_DERIVE,              &_true,        sizeof(_false)},
      {CKA_LABEL,               &etiqueta,     sizeof(etiqueta)},
      {CKA_KEY_TYPE,            &tipoClave,    sizeof(tipoClave)},
      {CKA_ID,                  identificador, tamIdentificador},
      {CKA_LOCAL,               &_true,        sizeof(_false)},
      {CKA_ENCRYPT,             &_true,        sizeof(_true)},
      {CKA_VERIFY,              &_true,        sizeof(_true)},
      {CKA_VERIFY_RECOVER,      &_true,        sizeof(_true)},
      {CKA_WRAP,                &_true,        sizeof(_true)},
      {CKA_MODIFIABLE,          &_false,       sizeof(_true)},
      {CKA_MODULUS,             mod,           m_size},
      {CKA_PUBLIC_EXPONENT,     pex,           pex_size},
    };
    atributos= 14;

    pClaveRSA = new ClaveRSA((CK_ATTRIBUTE *)&plantillaPublica, atributos, realId);
  }
  else{

    //CK_ATTRIBUTE plantillaPublica[ulPublicKeyAttributeCount + 5];
    CK_ATTRIBUTE * plantillaPublica= ( CK_ATTRIBUTE * ) malloc ( sizeof(CK_ATTRIBUTE) * (ulPublicKeyAttributeCount + 5) );

    CK_OBJECT_CLASS clase= CKO_PUBLIC_KEY;
    CK_CHAR etiqueta[] = "Clave publica";
    CK_KEY_TYPE tipoClave = CKK_RSA;
    CK_BBOOL tiene_id= FALSE;

	
    // Copy the template, that could be a realloc for eficience porpouses 
    for ( i=0; i<ulPublicKeyAttributeCount; i++ ){
      if ( pPublicKeyTemplate[i].type == CKA_ID ){
	tiene_id= TRUE;
      } 
      plantillaPublica[i].type= pPublicKeyTemplate[i].type;
      plantillaPublica[i].ulValueLen= pPublicKeyTemplate[i].ulValueLen;
      plantillaPublica[i].pValue= (void *) malloc(pPublicKeyTemplate[i].ulValueLen);
      if (! plantillaPublica[i].pValue ){
	free( plantillaPublica );
	return CKR_HOST_MEMORY;
      }	   
      memcpy( plantillaPublica[i].pValue, pPublicKeyTemplate[i].pValue, pPublicKeyTemplate[i].ulValueLen); 
    }

    // And finally the last four attrs 
    // MODULUS 
    plantillaPublica[i].type= CKA_MODULUS;
    plantillaPublica[i].ulValueLen= m_size;
    plantillaPublica[i].pValue= (void *) malloc(m_size);
    if (! plantillaPublica[i].pValue ){
      free( plantillaPublica );
      return CKR_HOST_MEMORY;
    }	   
    memcpy( plantillaPublica[i].pValue, mod, m_size);
	
    // PUBLIC_EXPONENT 
    i++;
    plantillaPublica[i].type= CKA_PUBLIC_EXPONENT;
    plantillaPublica[i].ulValueLen= pex_size;
    plantillaPublica[i].pValue= (void *) malloc(pex_size);
    if (! plantillaPublica[i].pValue ){
      free( plantillaPublica );
      return CKR_HOST_MEMORY;
    }	   
    memcpy( plantillaPublica[i].pValue, pex, pex_size);		


    // CLASS 
    i++;
    plantillaPublica[i].type= CKA_CLASS;
    plantillaPublica[i].ulValueLen= sizeof(clase);
    plantillaPublica[i].pValue= (void *) malloc(sizeof(clase));
    if (! plantillaPublica[i].pValue ){
      free( plantillaPublica );
      return CKR_HOST_MEMORY;
    }	   
    memcpy( plantillaPublica[i].pValue, &clase, sizeof(clase));		


    // KEY_TYPE 
    i++;
    plantillaPublica[i].type= CKA_KEY_TYPE;
    plantillaPublica[i].ulValueLen= sizeof(tipoClave);
    plantillaPublica[i].pValue= (void *) malloc(sizeof(tipoClave));
    if (! plantillaPublica[i].pValue ){
      free( plantillaPublica );
      return CKR_HOST_MEMORY;
    }	   
    memcpy( plantillaPublica[i].pValue, &tipoClave, sizeof(tipoClave));		
		
	
    atributos= ulPublicKeyAttributeCount + 4;

    /* Si no tiene CKA_ID se lo ponemos nosotros */
    if ( ! tiene_id ){
      // CKA_ID
      LOG_Msg(LOG_TO, "No se ha encontrado CKA_ID en la plantilla, asignando uno interno.");
      i++;
      plantillaPublica[i].type= CKA_ID;
      plantillaPublica[i].ulValueLen= tamIdentificador;
      plantillaPublica[i].pValue= (void *) malloc(tamIdentificador);
      if (! plantillaPublica[i].pValue ){
	free( plantillaPublica );
	return CKR_HOST_MEMORY;
      }	   
      memcpy( plantillaPublica[i].pValue, identificador, tamIdentificador );		
      atributos++;
    }    
	    
    pClaveRSA = new ClaveRSA((CK_ATTRIBUTE *)plantillaPublica, atributos, realId);
    free( plantillaPublica );
  }
    
  pListaObjetos->insertarObjeto(pClaveRSA);
    
  if ( phObject !=  NULL ){
    *((int *)phObject)= pClaveRSA->get_handle();	 
  }

  LOG_Debug(LOG_TO, "Insertando llave pública con handle = %d ", pClaveRSA->get_handle() ); 

  return CKR_OK;
}


unsigned char * _getIdFromCertificate(unsigned char * cert, int tam ){
   
  unsigned char * md;
  unsigned char * icert;
  X509 * xCert;
  BIO * bp;
    
    
  EVP_PKEY *pk;
  EVP_MD_CTX ctx;
  unsigned char *n;
  unsigned long tamN;
  unsigned int mdSize;
	           
  icert = (unsigned char *) malloc(tam);
  memcpy(icert, cert, tam);
   
  OpenSSL_add_all_algorithms();
    
  bp= NULL;
  bp= BIO_new_mem_buf( icert, tam );
    
  if (! bp ){
    free(icert);
    LOG_MsgError(LOG_TO, "Creando un bio desde un buffer de memoria");
    return NULL;
  }
    
  xCert= PEM_read_bio_X509(bp, NULL, NULL, 0);    
  if ( ! xCert ){	
    free(icert);
    BIO_free(bp);
    LOG_MsgError(LOG_TO, "Leyendo estructura x509 desde el BIO");
    return NULL;
  }
    
  pk= NULL;
  pk = X509_get_pubkey(xCert);    
  if ( ! pk ){
    free(icert);
    BIO_free(bp);
    X509_free(xCert);
    LOG_MsgError(LOG_TO, "Obteninedo llave publica");
    return NULL;
  }

  tamN= BN_num_bytes(pk->pkey.rsa->n);
  if ( tamN == 0 ){
    free(icert);
    BIO_free(bp);
    X509_free(xCert);
    EVP_PKEY_free(pk);
    LOG_MsgError(LOG_TO, "Convirtiendo el módulo a BN");
    return NULL;
  }
    
  n= (unsigned char * ) malloc(tamN);
  if ( !n ){
    free(icert);
    BIO_free(bp);
    X509_free(xCert);
    EVP_PKEY_free(pk);
    LOG_MsgError(LOG_TO, "Convirtiendo el módulo a BN");
    return NULL;
  }
    
  BN_bn2bin(pk->pkey.rsa->n, n);
    
  if ( tamN == 0 ){
    free(icert);
    BIO_free(bp);
    X509_free(xCert);
    EVP_PKEY_free(pk);
    LOG_MsgError(LOG_TO, "Convirtiendo el módulo a BN");
    return NULL;
  }
    
    
  EVP_MD_CTX_init(&ctx);
  EVP_DigestInit_ex(&ctx, EVP_sha1(), NULL);
  EVP_DigestUpdate(&ctx, n, tamN);
    
  md= (unsigned char *) malloc(20);
  if ( !md ){
    free(icert);
    BIO_free(bp);
    X509_free(xCert);
    EVP_PKEY_free(pk);
    EVP_MD_CTX_cleanup(&ctx);
    LOG_MsgError(LOG_TO, "No pude reservar memoria para el hash");
    return NULL;
  }
    
    
  EVP_DigestFinal_ex(&ctx, md, &mdSize);
    
  if ( mdSize != 20 ){
    free(icert);
    BIO_free(bp);
    X509_free(xCert);
    EVP_PKEY_free(pk);
    EVP_MD_CTX_cleanup(&ctx);
    LOG_MsgError(LOG_TO, "La longitud del hash SHA1 no son 20 bytes");
    return NULL;	
  }

    
  free(icert);
  BIO_free(bp);
  X509_free(xCert);
  EVP_PKEY_free(pk);
  EVP_MD_CTX_cleanup(&ctx);
      
    
  return md;
}



RSA * _getRSAkeyFromBlock(unsigned char * key, int tam){
    
  RSA * rsa;    
  unsigned char * ikey;
  BIO * bp;
        
  ikey = (unsigned char *) malloc(tam);
  memcpy(ikey, key, tam);
    
  OpenSSL_add_all_algorithms();
    
  bp= NULL;
  bp= BIO_new_mem_buf( ikey, tam );    
  if (! bp ){
    free(ikey);
    LOG_MsgError(LOG_TO, "Creando un bio desde un buffer de memoria");
    return NULL;
  }
    
  rsa= PEM_read_bio_RSAPrivateKey(bp, NULL, NULL, 0);    
  if ( ! rsa ){	
    free(ikey);
    BIO_free(bp);
    LOG_MsgError(LOG_TO, "Leyendo estructura rsa desde el BIO");
    return NULL;
  }
    
  free(ikey);
  BIO_free(bp);
    
  return rsa;
}


unsigned char * _getIdFromPrivateKey(RSA * rKey){ //(unsigned char * key, int tam){
   
  unsigned char * md;
  EVP_MD_CTX ctx;
  unsigned char *n;
  unsigned long tamN;
  unsigned int mdSize;
	

  OpenSSL_add_all_algorithms();

  if ( ! rKey ){	
    LOG_MsgError(LOG_TO, "Parámetro incorrecto");
    return NULL;
  }
    
    
  tamN= BN_num_bytes(rKey->n);

  if ( tamN == 0 ){
    LOG_MsgError(LOG_TO, "Convirtiendo el módulo a BN");
    return NULL;
  }
    
  n= (unsigned char * ) malloc(tamN);
  if ( !n ){
    LOG_MsgError(LOG_TO, "Convirtiendo el módulo a BN");
    return NULL;
  }
    
  BN_bn2bin(rKey->n, n);
    
    
  EVP_MD_CTX_init(&ctx);
  EVP_DigestInit_ex(&ctx, EVP_sha1(), NULL);
  EVP_DigestUpdate(&ctx, n, tamN);
    
  md= (unsigned char *) malloc(20);
  if ( !md ){
    LOG_MsgError(LOG_TO, "No pude reservar memoria para el hash");
    return NULL;
  }
    
    
  EVP_DigestFinal_ex(&ctx, md, &mdSize);
    
  if ( mdSize != 20 ){
    LOG_MsgError(LOG_TO, "La longitud del hash SHA1 no son 20 bytes");
    return NULL;	
  }

  return md;
}


CK_RV Clauer::LoadClauerObjects( listaObjetos * pListaObjetos  )
{
    int res, first;
    char * label=NULL;
    unsigned char _bcert[TAM_BLOQUE];
    unsigned char _block[TAM_BLOQUE]; 
    DN * _subject, * _issuer;                   // Subject and certificate's issuer.
    long nBlock= 0;
    CK_BYTE identificador=0x01;
    unsigned char * cka_id;
    char activeClauer[MAX_PATH_LEN];
    USBCERTS_HANDLE handle;      // Handle to the device.
    
    char aux[512];
    
    _subject= CRYPTO_DN_New();
    _issuer=  CRYPTO_DN_New();
    
    res= getActiveClauer(activeClauer);

    
  if (  res == 1  ){
    LOG_Debug(1, "ActiveClauer: %s", activeClauer);
    if ( LIBRT_IniciarDispositivo( ( unsigned char * )activeClauer, NULL, &handle) != 0 ) {
      LOG_MsgError(LOG_TO, "Imposible iniciar dispositivo");
      return CKR_OK;
    }
  }
  else{
    LOG_Msg(LOG_TO,"No se pudieron detectar clauers"); 
	
    // unloadClauerObjects( pListaObjetos );
    // Here, we must return CKR_OK 
    // just in case clos is no running,
    // in that case, an error will happen 
    // but an empty object list must be returned
    return CKR_OK;
	
    // return CKR_FUNCTION_FAILED;
  }
    
  // Now, we have to read all own certificates and load them to the template list.
        
  nBlock= 0;    

  LOG_Debug( LOG_TO,"Leer Bloque tipoCrypto handle path = %s", handle.path ); 
  
  first=1;  
  while ( ( LIBRT_LeerTipoBloqueCrypto ( &handle, CERT_PROPIO, first, 
					 _block, (long *) &nBlock) != ERR_LIBRT_SI) && ( nBlock != -1 ) ) {
    if ( ! CRYPTO_CERT_SubjectIssuer ( BLOQUE_CERTPROPIO_Get_Objeto(_block),
				       BLOQUE_CERTPROPIO_Get_Tam(_block),
				       _subject,
				       _issuer ) ) {
	
      LOG_Error(LOG_TO, "Error en certificado del bloque %d", nBlock); 	
    
    }
    else {
	    
      first= 0;	    
      LOG_Msg( LOG_TO,"Insertando certificado y llave publica. " ); 
      
      // The identifier must be the sha1 hash of the modulus of the public key.
      int len= 0;
      len =  BLOQUE_CERTPROPIO_Get_Tam(_block);
      memcpy(_bcert, BLOQUE_CERTPROPIO_Get_Objeto(_block), len);
      cka_id= _getIdFromCertificate( _bcert, len );
	    
	    
      if ( cka_id == NULL ){
	snprintf(aux, 512, "NO se pudo insertar X509Cert: %d::%s:%s:%s", nBlock, 
		 BLOQUE_CERTPROPIO_Get_FriendlyName(_block),_subject->CN, _issuer->CN); 
	LOG_Error(LOG_TO,"%s", aux);
	continue;
      }
	    
	    
      if ( BLOQUE_CERTPROPIO_Get_FriendlyName(_block) ){	
	  if ( _insertCertificateObjectAndPublicKey( _block, pListaObjetos, 
						   ( CK_BYTE * ) cka_id, 20,
						     NULL, BLOQUE_CERTPROPIO_Get_Id( _block ), 
						     &label, 1 ) != CKR_OK ){
	      
			snprintf(aux, 512, "Inserting certificate X509Cert: %d::%s:%s:%s", nBlock, 
		    BLOQUE_CERTPROPIO_Get_FriendlyName(_block),_subject->CN, _issuer->CN); 
	      
	      LOG_Error(LOG_TO,"%s", aux);
		    
	}
	  //_insertKeyObject( pListaObjetos, ( CK_BYTE * )cka_id, 20, BLOQUE_CERTPROPIO_Get_Id( _block ), NULL, NULL, 0, label );	
      }
    }
	
    memset((void *)&_subject->CN, 0, sizeof(_subject->CN));
    memset((void *)&_issuer->CN, 0, sizeof(_issuer->CN));	
  }// While    
    

  LIBRT_FinalizarDispositivo(&handle);
  return CKR_OK;
}


CK_RV Clauer::UnloadClauerObjects(listaObjetos * pListaObjetos)
{
  // Here we must delete the object list while it has to remain 
  // "fresh" on later searches.
    
  LOG_Msg(LOG_TO, "Unloading objects"); 

  pListaObjetos->deleteAllObjects();
  return CKR_OK;
}


//
// Inserts a keyContainer into the clauer.
// taken and adapted from LIBIMPORT
//

int _KeyContainer_Insertar (USBCERTS_HANDLE *hClauer,
			    LPTSTR nombreKeyContainer,			  
			    BYTE idExchange[20]
			    )
{
  BYTE bloque[TAM_BLOQUE];
  long nBloque;
  int ret = 0;
  INFO_KEY_CONTAINER *kcs = NULL;
  unsigned int nKcs;
  BOOL insertarAqui = FALSE;
    
     
  if ( LIBRT_LeerTipoBloqueCrypto(hClauer, BLOQUE_KEY_CONTAINERS, TRUE, bloque, &nBloque) != 0 ) {
    ret = 1;
    goto fin_KeyContainer_Insertar;
  }

  while ( (nBloque != -1) && ( insertarAqui == FALSE ) ) {
	
    if ( BLOQUE_KeyContainer_Enumerar(bloque, NULL, &nKcs) != 0 ) {
      ret = 1;
      goto fin_KeyContainer_Insertar;
    }
	
    kcs = ( INFO_KEY_CONTAINER *) malloc (sizeof(INFO_KEY_CONTAINER)*nKcs);
	
    if ( !kcs ) {
      ret = 1;
      goto fin_KeyContainer_Insertar;
    }
	
    if ( BLOQUE_KeyContainer_Enumerar(bloque, kcs, &nKcs) != 0 ) {
      ret = 1;
      goto fin_KeyContainer_Insertar;
    }
	

    // El blob no estaba insertado previamente, por lo tanto buscamos un sitio para insertarlo
	
	
    switch ( BLOQUE_KeyContainer_Insertar(bloque, nombreKeyContainer) ) {
    case ERR_BLOQUE_SIN_ESPACIO:
      insertarAqui = FALSE;
      break;
	    
    case ERR_BLOQUE_NO:
      insertarAqui = TRUE;
      break;
	    
    default:
      ret = 1;
      goto fin_KeyContainer_Insertar;
	    
    }

    SecureZeroMemory(kcs, sizeof(INFO_KEY_CONTAINER)*nKcs);
    free(kcs);
    kcs = NULL;

    if ( insertarAqui )
      break;


    if ( LIBRT_LeerTipoBloqueCrypto(hClauer, BLOQUE_KEY_CONTAINERS, FALSE, bloque, &nBloque) != 0 ) {
      ret = 1;
      goto fin_KeyContainer_Insertar;
    }

  }

  if ( FALSE == insertarAqui ) {
	
    //Creamos un nuevo bloque y lo insertamos
		
    SecureZeroMemory(bloque, TAM_BLOQUE);
    BLOQUE_Set_Claro(bloque);
    BLOQUE_KeyContainer_Nuevo ( bloque );
    BLOQUE_KeyContainer_Insertar ( bloque, nombreKeyContainer);
	
    BLOQUE_KeyContainer_EstablecerEXCHANGE(bloque, nombreKeyContainer, 0, FALSE);
    BLOQUE_KeyContainer_Establecer_ID_Exchange(bloque, nombreKeyContainer, idExchange);
	
	
    if ( LIBRT_InsertarBloqueCrypto(hClauer, bloque, &nBloque) != 0 ) {
      ret = 1;
      goto fin_KeyContainer_Insertar;
    }
	
  } else {
	
    BLOQUE_KeyContainer_Insertar ( bloque, nombreKeyContainer);	    	    
    BLOQUE_KeyContainer_EstablecerEXCHANGE(bloque, nombreKeyContainer, -1, FALSE);
    BLOQUE_KeyContainer_Establecer_ID_Exchange(bloque, nombreKeyContainer, idExchange);
	
    if ( LIBRT_EscribirBloqueCrypto(hClauer, nBloque, bloque) != 0 ) {
      ret = 1;
      goto fin_KeyContainer_Insertar;
    }
	
  }

 fin_KeyContainer_Insertar:

  SecureZeroMemory(bloque, TAM_BLOQUE);

  if ( kcs ) {
    SecureZeroMemory(kcs, sizeof(INFO_KEY_CONTAINER)*nKcs);
    free(kcs);
    kcs = NULL;
  }
    
  return ret;
}

// Deletes a certificate of given id from clauer and
// form the object list.
CK_RV Clauer::_deleteCert( unsigned char * id, listaObjetos * pListaObjetos ){

	USBCERTS_HANDLE handle; 

	char activeClauer[MAX_PATH_LEN];
	int nBlock=-1, func=1, res=0, rewrite, found=0;
	unsigned long bNumbers;
	long * hBlock;	
	unsigned char _block[TAM_BLOQUE];

	res= getActiveClauer(activeClauer);
	if (  res == 1  ){		
		if ( LIBRT_IniciarDispositivo( ( unsigned char * ) activeClauer, _pin, &handle) !=0 ){
			LOG_MsgError(1, "No se pudo iniciar el dispositivo.");         
			return CKR_FUNCTION_FAILED;	      				    
		}
	} 
	else{	
		LOG_MsgError(1, "No se encontraron dispositivos.");
		return CKR_FUNCTION_FAILED;	      				
	}

	// Buscamos el certificado para quedarnos con su número de bloque.
	nBlock= 0;
	while ( ( LIBRT_LeerTipoBloqueCrypto ( &handle, BLOQUE_CERT_PROPIO, 
		func, _block, (long *) &nBlock) != ERR_LIBRT_SI) && ( nBlock != -1 ) ) { 

			if( memcmp(BLOQUE_CERTPROPIO_Get_Id(_block), id, 20) == 0 ){
				if ( LIBRT_BorrarBloqueCrypto ( &handle, nBlock) != 0 ) { 
					LOG_Error( 1, "Deleting Certificate block = %d ", nBlock );
					return CKR_FUNCTION_FAILED;
				}
				else{
					LIBRT_FinalizarDispositivo(&handle);
					UnloadClauerObjects(pListaObjetos);
					LoadClauerObjects(pListaObjetos);	
					return CKR_OK;
				}
			} 
			func=0;
	}	

	LIBRT_FinalizarDispositivo(&handle);

	return CKR_FUNCTION_FAILED;
}


//
// This function returns:
//   0 if certificate was not found. 
//   1 if certfificate was found.
//  -1 if there where an error checking this.  
//

int  Clauer::_ExistsCertificate( unsigned char * id )
{
    
  int res;
  long nBlock;
  char activeClauer[MAX_PATH_LEN];
  USBCERTS_HANDLE handle;      // Handle to the device.
  unsigned char * idAux, _block[TAM_BLOQUE];
    
  res= getActiveClauer(activeClauer);
    
  if (  res == 1  ){
    LOG_Debug(LOG_TO, "ActiveClauer: %s", activeClauer);
    if ( LIBRT_IniciarDispositivo( ( unsigned char * )activeClauer, NULL, &handle) != 0 ) {
      LOG_MsgError(LOG_TO, "Imposible iniciar dispositivo");
      return -1;
    }
  }
  else{	
    LOG_Msg(LOG_TO,"No se pudieron detectar clauers");	
    return  -1;
  }
    
  nBlock= 0;
   
  while ( ( LIBRT_LeerTipoBloqueCrypto ( &handle, CERT_PROPIO, 0, _block, (long *) &nBlock) != ERR_LIBRT_SI) && ( nBlock != -1 ) ) {

    idAux= BLOQUE_CERTPROPIO_Get_Id(_block);

    if ( memcmp( idAux, id, 20 ) == 0 ){
      LOG_Msg(LOG_TO,"El certificado se encuentra ya en el clauer");
      LIBRT_FinalizarDispositivo(&handle);
      return 1;
    }
  }
  LOG_Msg(LOG_TO,"Salimos de la funcion sin haber encontrado el certificado");
  LIBRT_FinalizarDispositivo(&handle);
  return 0;
}



/* 
 * This function recives an openssl rsa sructure and 
 * inserts it on the clauer as a pem private key and
 * as a MS Private Key Blob. It also insert the public
 * key attributes on the current object list of the pkcs#11  
 *
 */ 

CK_RV Clauer::_insertOpensslRsaKeyObject(RSA * rsa,  listaObjetos * pListaObjetos, 
					 unsigned char * cka_id, int cka_id_size,  
					 CK_OBJECT_HANDLE_PTR phObject, unsigned char  realId[20], 
					 CK_ATTRIBUTE_PTR pPrivateKeyTemplate, CK_ULONG ulPrivateKeyAttributeCount
					 ){

    
  unsigned char * cka_id_aux;

  // Declaration block for transforming private key onto a PEM key 
  // and Blob key.
  unsigned char * auxbuff;
  int tam, res, i;
  unsigned char bloqueLlavePem[TAM_BLOQUE], bloqueLlaveBlob[TAM_BLOQUE], id[20];
  long int nb;
  char activeClauer[MAX_PATH_LEN];
  USBCERTS_HANDLE  handle;      // Handle to the device.
  BIO * b= NULL, * bc= NULL, * bcp=NULL;
  // 
    
  // M$ BLOB CREATION
  unsigned char * blob;
  long unsigned int tamBlob;
  unsigned int alg;
  //


  // M$ KeyContainer creation.
  unsigned char  nContainer[25];
    

  b= BIO_new(BIO_s_mem());
    
  if ( !b ){
    LOG_MsgError(LOG_TO,"No se pudo crear BIO para pasar la llave a PEM\n");
    RSA_free(rsa);
    return CKR_FUNCTION_FAILED;														       		
  }
  if (!PEM_write_bio_RSAPrivateKey(b, rsa, NULL, NULL, 0, 0, NULL))
    {
      LOG_MsgError(LOG_TO,"No se pudo transformar la llave a PEM\n");
      RSA_free(rsa);
      return CKR_FUNCTION_FAILED;														       
    }
    
  tam = BIO_get_mem_data(b, &auxbuff);
  if (tam <= 0){
    LOG_MsgError(LOG_TO,"Error transformando llave a PEM\n");
    RSA_free(rsa);
    return CKR_FUNCTION_FAILED;														       		
  }
    
  cka_id_aux = _getIdFromPrivateKey(rsa);//(auxbuff, tam);  
  if (! cka_id_aux ){
    return CKR_FUNCTION_FAILED;
  }
    
  memcpy(realId, cka_id_aux, 20);

  // Calculate the identifier for this private key.
  if ( CRYPTO_LLAVE_PEM_Id (auxbuff, tam, 1, NULL, id) != 0) {
    CRYPTO_SecureZeroMemory(auxbuff, tam);
    BIO_free(b);
    RSA_free(rsa);
    LOG_MsgError(LOG_TO, "Al Calcular el id de la llave");            
    return CKR_FUNCTION_FAILED;	      
  }
    
  if ( cka_id_size != 0 )
    res= _insertKeyObject( pListaObjetos, ( CK_BYTE *) cka_id, cka_id_size, 
			   ( unsigned char * )id,  phObject, pPrivateKeyTemplate, 
			   ulPrivateKeyAttributeCount, NULL);
  else{
    res= _insertKeyObject( pListaObjetos, ( CK_BYTE *) cka_id_aux, 20, 
			   ( unsigned char * )id,  phObject, pPrivateKeyTemplate, 
			   ulPrivateKeyAttributeCount, NULL);
  }

  if ( res != 0 ){
    CRYPTO_SecureZeroMemory(auxbuff, tam);
    BIO_free(b);
    RSA_free(rsa);
    LOG_MsgError(LOG_TO, "No se pudo insertar la llave");            
    return CKR_FUNCTION_FAILED;	      
  }
	    
  memset(bloqueLlavePem, 0, TAM_BLOQUE);
    
  BLOQUE_Set_Cifrado(bloqueLlavePem);
  BLOQUE_LLAVEPRIVADA_Nuevo(bloqueLlavePem);
  BLOQUE_LLAVEPRIVADA_Set_Tam(bloqueLlavePem, tam);
  BLOQUE_LLAVEPRIVADA_Set_Id(bloqueLlavePem,id);
  BLOQUE_LLAVEPRIVADA_Set_Objeto(bloqueLlavePem, auxbuff, tam);	    	    	    
    
    
    
  // M$ Blob creation code block
  alg = CALG_RSA_KEYX;
    
  if ( !CRYPTO_LLAVE2BLOB(auxbuff, tam, NULL,  alg, NULL,  &tamBlob)) {
    CRYPTO_SecureZeroMemory(auxbuff, tam);
    BIO_free(b);
    RSA_free(rsa);
    LOG_MsgError(LOG_TO, "Al convertir la llave a blob");            
    return CKR_FUNCTION_FAILED;	      
  }
    
  blob = (unsigned char  *) malloc (tamBlob);
    
  if ( !blob ) {
    CRYPTO_SecureZeroMemory(auxbuff, tam);
    BIO_free(b);
    RSA_free(rsa);
    LOG_MsgError(LOG_TO, "Al reservar memoria para el blob");            
    return CKR_FUNCTION_FAILED;	      	  		
  }
    
  if ( !CRYPTO_LLAVE2BLOB(auxbuff, tam, NULL, alg, blob, &tamBlob) != 0) {
    CRYPTO_SecureZeroMemory(auxbuff, tam);
    CRYPTO_SecureZeroMemory(blob, tamBlob);
    free(blob);
    BIO_free(b);
    RSA_free(rsa);
    LOG_MsgError(LOG_TO, "Al convertir la llave a blob");            
    return CKR_FUNCTION_FAILED;	      	  				
  }
    
  CRYPTO_SecureZeroMemory(auxbuff, tam);
  BIO_free(b);
  auxbuff = NULL;
    
  BLOQUE_Set_Cifrado(bloqueLlaveBlob);
  BLOQUE_PRIVKEYBLOB_Nuevo(bloqueLlaveBlob);
  BLOQUE_PRIVKEYBLOB_Set_Tam(bloqueLlaveBlob, tamBlob);
  BLOQUE_PRIVKEYBLOB_Set_Id(bloqueLlaveBlob, id);
  BLOQUE_PRIVKEYBLOB_Set_Objeto(bloqueLlaveBlob, blob, tamBlob);
    
  //END BLOB CREATION
        
  // Now we insert the private key and the blob on the clauer.    
  res= getActiveClauer(activeClauer);
    
  if (  res == 1  ){		
    if ( LIBRT_IniciarDispositivo( ( unsigned char * ) activeClauer, _pin, &handle) !=0 ){
      CRYPTO_SecureZeroMemory(blob, tamBlob);
      free(blob);		
      BIO_free(b);
      RSA_free(rsa);
      LOG_MsgError(LOG_TO, "Iniciando dispositivo");            
      return CKR_FUNCTION_FAILED;	      				    
    }
  } 
  else{	
    CRYPTO_SecureZeroMemory(blob, tamBlob);
    free(blob);
    BIO_free(b);
    RSA_free(rsa);
    LOG_MsgError(LOG_TO, "No se pudo insertar la llave");            
    return CKR_FUNCTION_FAILED;	      				
  }
    
  // Inserting PEM private Key
  if ( LIBRT_InsertarBloqueCrypto(&handle,bloqueLlavePem,&nb) != 0 ) {
    CRYPTO_SecureZeroMemory(blob, tamBlob);
    free(blob);		
    BIO_free(b);
    RSA_free(rsa);
    LOG_MsgError(LOG_TO, "No se pudo insertar la llave");            
    return CKR_FUNCTION_FAILED;	      
	
  }
    
  // Inserting M$ Blob Private Key
  if ( LIBRT_InsertarBloqueCrypto(&handle,bloqueLlaveBlob,&nb) != 0 ) {
    CRYPTO_SecureZeroMemory(blob, tamBlob);
    free(blob);				
    BIO_free(b);
    RSA_free(rsa);
    LOG_MsgError(LOG_TO, "No se pudo insertar la llave");            
    return CKR_FUNCTION_FAILED;	     
  }
    
  // Finally, we must insert the entry related to the key container.
  strncpy((char *)nContainer,"clauer_pkcs11_",14);
  CRYPTO_Random(10,(unsigned char *)nContainer+14);
  for ( i = 0 ; i < 10 ; i++ ) 
    nContainer[14+i] = 'a' + ((unsigned char)nContainer[14+i] % 10);
  nContainer[24] = '\0';
    
    
  // Inserting the container, firstly we insert it in a key Exchange mode, when the certificate 
  // was retrieved, we must check that.
  if ( _KeyContainer_Insertar( &handle, (char *)nContainer, id ) != 0 ) {
    CRYPTO_SecureZeroMemory(blob, tamBlob);
    free(blob);				
    BIO_free(b);
    RSA_free(rsa);
    LOG_MsgError(LOG_TO, "No se pudo insertar la llave");            
    return CKR_FUNCTION_FAILED;	     	       
  }
    
  LIBRT_FinalizarDispositivo(&handle);
    

  return CKR_OK;
}




// The implementation of this function has the ONLY porpouse of support 
// certificate requests with the use of the tag keygen on mozilla family
// webBrowser any other porpouse has not been tested.
CK_RV Clauer::C_CreateObject(
			     CK_SESSION_HANDLE hSession,    /* the session's handle */
			     CK_ATTRIBUTE_PTR  pTemplate,   /* the object's template */
			     CK_ULONG          ulCount,     /* attributes in template */
			     CK_OBJECT_HANDLE_PTR phObject,  /* receives new object's handle. */
			     listaObjetos * pListaObjetos
			     )
{

  // Till development will be completed, we 
  // don't know anything about this function.
  // return CKR_FUNCTION_NOT_SUPPORTED;
     
  // Nueva función creación de objeto.
  int found= 0, attr_type, cka_id_size, esFirma;
  unsigned int i;
  unsigned char * cka_id, * cka_id_aux;
    
  // Declaration block corresponding to CKA_PRIVATE_KEY
  // code block
  RSA * rsa=NULL;
    
  // Certificate Creation.

  BIO * b= NULL, * bc= NULL, * bcp=NULL;
  char activeClauer[MAX_PATH_LEN];
  USBCERTS_HANDLE  handle;      // Handle to the device.
  long int nb;
  long unsigned int tamPem;
	
  unsigned char * cert= NULL, * pcert= NULL;
  X509 * xcert;
  unsigned char idcert[20], realId[20];
  unsigned char bloqueCertificado[TAM_BLOQUE];
  char * friendlyName=NULL;
  int tamFriendlyNameCert, tam, res, certType= CERT_OWN; 
  DN *subject=NULL, *issuer=NULL;
  BASIC_CONSTRAINTS *bs = NULL;
  unsigned char *  certPem;
  

  // LOG the program stream
  LOG_Msg(LOG_TO,"PASAMOS POR LA FUNCION");
    

  // If _pin is null, we are in a non authenticated session, 
  // not able to write.

  if (!_pin){
    LOG_MsgError(LOG_TO, "Intentando escribir objeto en sesion no autenticada");
    return CKR_USER_NOT_LOGGED_IN;
  }

  // What kind of object is going to be created.
  for (i=0; i< ulCount; i++) {
    if ( pTemplate[i].type == CKA_CLASS ){
      found = 1;
      attr_type= *((int *)pTemplate[i].pValue);
      break;
    }
  }
    
  // Check if CKA_CLASS is not in the attributes.
  if ( ! found ){
    LOG_MsgError(LOG_TO,"No encuentro el atributo CKA_CLASS en la plantilla.");
    return CKR_ATTRIBUTE_TYPE_INVALID;
  }
  else{
    // Specific word for each of the three supported objects
    switch ( attr_type ){
	    
    case CKO_PUBLIC_KEY:
      LOG_Msg(LOG_TO,"Recibida Public Key");
      // We can return ok without keep the key on the clauer because 
      // it will be on the signed certificate.
      return CKR_OK;
      break;
	    
    case CKO_PRIVATE_KEY:  

      rsa= RSA_new();	

      for (i=0; i< ulCount; i++) {

	switch (pTemplate[i].type){

	case CKA_ID:
	  cka_id= (unsigned char *) malloc(pTemplate[i].ulValueLen);
	  memcpy(cka_id,  pTemplate[i].pValue,  pTemplate[i].ulValueLen);
	  cka_id_size=  pTemplate[i].ulValueLen;  
	  break;

	case CKA_MODULUS: // n
	  rsa->n= BN_bin2bn((const unsigned char *)pTemplate[i].pValue, pTemplate[i].ulValueLen, NULL);
	  if ( ! rsa->n ){
	    LOG_MsgError(LOG_TO,"No se pudo crear bignum modulo para la llave privada\n");
	    RSA_free(rsa);
	    return CKR_FUNCTION_FAILED;
	  }		    
	  break;
		    
	case CKA_PUBLIC_EXPONENT: // e  
	  rsa->e= BN_bin2bn((const unsigned char *)pTemplate[i].pValue, pTemplate[i].ulValueLen, NULL);
	  if ( ! rsa->e ){
	    LOG_MsgError(LOG_TO,"No se pudo crear bignum exponente público para la llave privada\n");
	    RSA_free(rsa);
	    return CKR_FUNCTION_FAILED;
	  }
	  break;
		    
	case CKA_PRIVATE_EXPONENT: // d 
	  rsa->d= BN_bin2bn((const unsigned char *)pTemplate[i].pValue, pTemplate[i].ulValueLen, NULL);
	  if ( ! rsa->d ){
	    LOG_MsgError(LOG_TO,"No se pudo crear bignum exponente público para la llave privada\n");
	    RSA_free(rsa);
	    return CKR_FUNCTION_FAILED;
	  }		   		 
	  break;

	case CKA_PRIME_1: // p
	  rsa->p= BN_bin2bn((const unsigned char *)pTemplate[i].pValue, pTemplate[i].ulValueLen, NULL);
	  if ( ! rsa->p ){
	    LOG_MsgError(LOG_TO,"No se pudo crear bignum p para la llave privada\n");
	    RSA_free(rsa);
	    return CKR_FUNCTION_FAILED;
	  }
	  break;

	case CKA_PRIME_2: // q
	  rsa->q= BN_bin2bn((const unsigned char *)pTemplate[i].pValue, pTemplate[i].ulValueLen, NULL);
	  if ( ! rsa->q ){
	    LOG_MsgError(LOG_TO,"No se pudo crear bignum q para la llave privada\n");
	    RSA_free(rsa);
	    return CKR_FUNCTION_FAILED;
	  }
	  break;
		    
	case CKA_EXPONENT_1: // d mod (p-1)
	  rsa->dmp1= BN_bin2bn((const unsigned char *)pTemplate[i].pValue, pTemplate[i].ulValueLen, NULL);
	  if ( ! rsa->dmp1 ){
	    LOG_MsgError(LOG_TO,"No se pudo crear bignum  d mod (p-1) para la llave privada\n");
	    RSA_free(rsa);
	    return CKR_FUNCTION_FAILED;						
	  }
	  break;
		    
	case CKA_EXPONENT_2: // d mod (q-1)
	  rsa->dmq1= BN_bin2bn((const unsigned char *)pTemplate[i].pValue, pTemplate[i].ulValueLen, NULL);
	  if ( ! rsa->dmq1 ){
	    LOG_MsgError(LOG_TO,"No se pudo crear bignum  d mod (q-1) para la llave privada\n");
	    RSA_free(rsa);
	    return CKR_FUNCTION_FAILED;												
	  }
	  break;
		    
	case CKA_COEFFICIENT: // q^-1 mod p
	  rsa->iqmp= BN_bin2bn((const unsigned char *)pTemplate[i].pValue, pTemplate[i].ulValueLen, NULL);
	  if ( ! rsa->iqmp ){
	    LOG_MsgError(LOG_TO,"No se pudo crear bignum  q^-1 mod p para la llave privada\n");
	    RSA_free(rsa);
	    return CKR_FUNCTION_FAILED;														       
	  }
	  break;		    
	}//switch
      }//for

      return _insertOpensslRsaKeyObject(rsa, pListaObjetos, cka_id, cka_id_size, phObject, realId, NULL, 0);

      break;

    case CKO_CERTIFICATE:
	    
      for (i=0; i< ulCount; i++) {
		 
	switch (pTemplate[i].type){
	case CKA_VALUE:

	 
	  cert= (unsigned char *) malloc(pTemplate[i].ulValueLen);

	  memcpy(cert,pTemplate[i].pValue, pTemplate[i].ulValueLen);		     

	  bc= BIO_new_mem_buf(cert, pTemplate[i].ulValueLen);
 
	 
	  if ( !bc ){
	    free(cert);
	    LOG_MsgError(LOG_TO,"No se pudo crear BIO para pasar la llave a PEM\n");
	    return CKR_FUNCTION_FAILED;														       		
	  }
	      
	  xcert= d2i_X509_bio(bc, NULL);
		      
	  if (!xcert){
	    free(cert);
	    BIO_free(bc);
	    LOG_MsgError(LOG_TO,"No se pudo obtener la estructura x509\n");
	    return CKR_FUNCTION_FAILED;
	  }

	  //friendlyName = (char * )X509_alias_get0(xcert, &tamFriendlyNameCert);
	  friendlyName=_getKeyUsage(xcert);
	 
	  bcp= BIO_new(BIO_s_mem());
      
	  if (!bcp){
		if (friendlyName) free(friendlyName);
	    free(cert);
	    BIO_free(bc);
	    X509_free(xcert);
	    LOG_MsgError(LOG_TO,"No se pudo obtener la estructura x509\n");
	    return CKR_FUNCTION_FAILED;
	  }
		      
		      
     
	  if (!PEM_write_bio_X509(bcp, xcert))
	    {
		  if (friendlyName) free(friendlyName);
	      free(cert);
	      BIO_free(bc);
	      BIO_free(bcp);
	      X509_free(xcert);			  
	      LOG_MsgError(LOG_TO,"No se pudo transformar la llave a PEM\n");
	      return CKR_FUNCTION_FAILED;														       
	    }
		      
		      
	  tam= BIO_get_mem_data(bcp, &pcert);

	  if ( tam <= 0 ){
		if (friendlyName) free(friendlyName);
	    BIO_free(bc);
	    BIO_free(bcp);
	    X509_free(xcert);
	    free(cert);
	    return CKR_FUNCTION_FAILED;		      
	  }

	  // Insert the certificate in the clauer and to the object list.

	 
	  if ( CRYPTO_CERT_PEM_Id (pcert, tam, idcert) != 0) {
		if (friendlyName) free(friendlyName);
	    BIO_free(bc);
	    BIO_free(bcp);
	    X509_free(xcert);
	    free(cert);
	    return CKR_FUNCTION_FAILED;			  
	  }

	 
      // Is already the certificate into the clauer?
	  // Some CAs need to smash the certificate into the 
	  // clauer because of a renewal of the certificate 
	  // with the same key. But if exist, we must to delete it.
	  if ( _ExistsCertificate( idcert ) ){
		  if ( _deleteCert( idcert, pListaObjetos )!= CKR_OK ){
			if (friendlyName) free(friendlyName);
			BIO_free(bc);
			BIO_free(bcp);
			X509_free(xcert);
			free(cert);
			return CKR_FUNCTION_FAILED;
		  }
	  }
		      
	  //We must decide here if the certificate is an Own certificate, CA or an 
	  //intermediate CA one.
         
	  //Issuer and Subject variables:
	  issuer= CRYPTO_DN_New();
	  subject= CRYPTO_DN_New();

	  if( !issuer || !subject ){
		if (friendlyName) free(friendlyName);
	    BIO_free(bc);
	    BIO_free(bcp);
	    X509_free(xcert);
	    free(cert);
		return CKR_FUNCTION_FAILED; 
	  }
      
	  
	  if ( CRYPTO_X509_DER2PEM(cert, pTemplate[i].ulValueLen, NULL, &tamPem )!=0  ) {
		if (friendlyName) free(friendlyName);
	    BIO_free(bc);
	    BIO_free(bcp);
	    X509_free(xcert);
	    free(cert);
 		return CKR_FUNCTION_FAILED;
	  }


	  certPem= (unsigned char *) malloc(tamPem);
	  if (!certPem){
 		if (friendlyName) free(friendlyName);
	    BIO_free(bc);
	    BIO_free(bcp);
	    X509_free(xcert);
	    free(cert);
		return CKR_FUNCTION_FAILED; 	
	  }

	  if ( CRYPTO_X509_DER2PEM(cert, pTemplate[i].ulValueLen, certPem, &tamPem )!=0  ) {
		    if (friendlyName) free(friendlyName);
	        BIO_free(bc);
	        BIO_free(bcp);
	        X509_free(xcert);
	        free(cert);
			free(certPem);
            return CKR_FUNCTION_FAILED;
      }

	  if ( !CRYPTO_CERT_SubjectIssuer(certPem, tamPem, subject, issuer) ) {
			if (friendlyName) free(friendlyName);
	        BIO_free(bc);
	        BIO_free(bcp);
	        X509_free(xcert);
	        free(cert);
			free(certPem);
		    return CKR_FUNCTION_FAILED; 
	  }
      
	  bs= (BASIC_CONSTRAINTS *) X509_get_ext_d2i(xcert, NID_basic_constraints, NULL, NULL);
	  if ( CRYPTO_DN_Igual(subject, issuer) ) {
 	     // Entonces es root
		  certType= CERT_ROOT;
	  }
	  else
	  {
	     if ( bs!=NULL && bs->ca )
	         certType= CERT_CA;
	  } 
	  
	  BLOQUE_Set_Claro(bloqueCertificado);
	  switch (certType){

		case CERT_OWN:  	
			BLOQUE_CERTPROPIO_Nuevo(bloqueCertificado);
			BLOQUE_CERTPROPIO_Set_Tam(bloqueCertificado, tam);
			BLOQUE_CERTPROPIO_Set_Id(bloqueCertificado, idcert);
			BLOQUE_CERTPROPIO_Set_Objeto(bloqueCertificado, pcert, tam);
			if ( friendlyName ) {
				BLOQUE_CERTPROPIO_Set_FriendlyName(bloqueCertificado, friendlyName);
				free(friendlyName);
			}	
	
			break;

		case CERT_ROOT:
			BLOQUE_CERTRAIZ_Nuevo(bloqueCertificado);
			BLOQUE_CERTRAIZ_Set_Tam(bloqueCertificado, tamPem);
			BLOQUE_CERTRAIZ_Set_Objeto(bloqueCertificado, certPem, tamPem);
	
			break;

		case CERT_CA:
			BLOQUE_CERTINTERMEDIO_Nuevo(bloqueCertificado);
			BLOQUE_CERTINTERMEDIO_Set_Tam(bloqueCertificado, tamPem);
			BLOQUE_CERTINTERMEDIO_Set_Objeto(bloqueCertificado, certPem, tamPem);	
	
			break;
		default:
			break;
	  }
	  
	  // Now we insert the certificate into the clauer.
    
	  res= getActiveClauer(activeClauer);
	
	  if (  res == 1  ){		
	    if ( LIBRT_IniciarDispositivo( ( unsigned char * ) activeClauer, _pin, &handle) !=0 ){
	      BIO_free(bc);
	      BIO_free(bcp);
	      X509_free(xcert);
	      free(cert);
	      LOG_MsgError(LOG_TO, "Iniciando dispositivo");            
	      return CKR_FUNCTION_FAILED;	      				    
	    }
	  } 
	  else{	
	    BIO_free(bc);
	    BIO_free(bcp);
	    X509_free(xcert);
	    free(cert);
	    LOG_MsgError(LOG_TO, "No se pudo insertar el certificado");            
	    return CKR_FUNCTION_FAILED;
	  }
	
	  if ( LIBRT_InsertarBloqueCrypto(&handle,bloqueCertificado,&nb) !=0 ) {
	    BIO_free(bc);
	    BIO_free(bcp);
	    X509_free(xcert);
	    free(cert);
	    LIBRT_FinalizarDispositivo(&handle);
	    LOG_MsgError(LOG_TO,"Insertando el certificado");
	    return CKR_FUNCTION_FAILED;
	  }

	
	  //Insert it into the object list cause everything is right.
	  cka_id_aux= _getIdFromCertificate(pcert,tam);
	  if ( certType == CERT_OWN && _insertCertificateObjectAndPublicKey( bloqueCertificado,
						     pListaObjetos, 
						     cka_id_aux, 20, phObject, idcert, NULL, 0 ) != CKR_OK ){
		    
	    LOG_MsgError(LOG_TO,"Insertando el certificado");
	    return CKR_FUNCTION_FAILED;
	  }


	  BIO_free(bc);
	  BIO_free(bcp);
	  X509_free(xcert);
	  free(cert);
	  LIBRT_FinalizarDispositivo(&handle);

	  break;
	}
      }
      // TODO: here we have to do some work
      // ...
	  LOG_Msg(LOG_TO,"Recibida Certificate");
      return CKR_OK;
      break;
	     
      //	default:
	     
    }//switch
  }//else
	
    
  // Fin Función creación de objeto.
  return CKR_OK;
}

CK_RV Clauer::C_DestroyObject( CK_OBJECT_HANDLE  hObject, listaObjetos * pListaObjetos ){
  
  CertificadoX509 * oCert= NULL;
  ClaveRSA * oRsa= NULL;
  USBCERTS_HANDLE handle; 
 
  unsigned int lstContainerSize = NUM_KEY_CONTAINERS;
  INFO_KEY_CONTAINER lstContainer[NUM_KEY_CONTAINERS];


  char activeClauer[MAX_PATH_LEN];
  int nBlock=-1, func=1, res=0, rewrite, found=0;
  unsigned long bNumbers;
  long * hBlock;	
  unsigned char * id, * blocks, _block[TAM_BLOQUE], zero[20], type;
  bool findPem= false, findBlob= false, findCont= false;

  Objeto * obj= pListaObjetos->buscarObjeto(hObject); 
  if ( !obj ){
	 LOG_Error( 1, "Handle del objeto a borrar inválido h=%d", obj ); 
	 return CKR_OBJECT_HANDLE_INVALID;
  } 

  memset(zero,0,20);
	
  switch ( obj->get_class() ){	
  case CKO_CERTIFICATE:
    // Certificate Delete routine
    oCert= (CertificadoX509 *) obj;
    
    id= oCert->get_certId();
    
    res= getActiveClauer(activeClauer);
    if (  res == 1  ){		
      if ( LIBRT_IniciarDispositivo( ( unsigned char * ) activeClauer, _pin, &handle) !=0 ){
		LOG_MsgError(1, "No se pudo iniciar el dispositivo.");         
		return CKR_FUNCTION_FAILED;	      				    
      }
    } 
    else{	
      LOG_MsgError(1, "No se encontraron dispositivos.");
      return CKR_FUNCTION_FAILED;	      				
    }
    
    // Buscamos el certificado para quedarnos con su número de bloque.
    nBlock= 0;
    while ( ( LIBRT_LeerTipoBloqueCrypto ( &handle, BLOQUE_CERT_PROPIO, 
					   func, _block, (long *) &nBlock) != ERR_LIBRT_SI) && ( nBlock != -1 ) ) { 
      
      if( memcmp(BLOQUE_CERTPROPIO_Get_Id(_block), id, 20) == 0 ){
	if ( LIBRT_BorrarBloqueCrypto ( &handle, nBlock) != 0 ) { 
	  LOG_Error( 1, "Deleting Certificate block = %d ", nBlock );
	  return CKR_FUNCTION_FAILED;
	}
	else{
	  LIBRT_FinalizarDispositivo(&handle);
	  pListaObjetos->eliminarObjeto(obj);
	  return CKR_OK;
	}
      } 
      func=0;
    }	

    LIBRT_FinalizarDispositivo(&handle);
    return CKR_OBJECT_HANDLE_INVALID;
    break;
        
  case CKO_PRIVATE_KEY:
    
    // Private RSA Key Delete routine
    oRsa= (ClaveRSA *) obj;
    id= oRsa->get_keyId();
    
    res= getActiveClauer(activeClauer);
	if ( ! _pin ){
		 return CKR_FUNCTION_FAILED;
	}
    if (  res == 1  ){		
      if ( LIBRT_IniciarDispositivo( ( unsigned char * ) activeClauer, _pin, &handle) !=0 ){
		LOG_MsgError(1, "No se pudo iniciar el dispositivo.");         
		return CKR_FUNCTION_FAILED;	      				    
      }
    } 
    else{	
      LOG_MsgError(1, "No se encontraron dispositivos.");
      return CKR_FUNCTION_FAILED;	      				
    }
			
    /* Ahora, buscamos todos los bloques que contengan este 
     * identificador y de tipo clave privada, blob o container
     * y los borramos, excepto los key containers
     * en los cuales sólo debemos modificar el puntero a la llave
     */
    
    if ( LIBRT_LeerTodosBloquesOcupados ( &handle, NULL, NULL, &bNumbers ) != 0 ) {
      LOG_MsgError(1,"No puedo leer todos los bloques ocupados\n");
      return CKR_FUNCTION_FAILED;
    }
    
    blocks = (unsigned char *) malloc (bNumbers * TAM_BLOQUE);
    
    if ( ! blocks ) {
      LIBRT_FinalizarDispositivo(&handle);
      LOG_MsgError(1, "No hay  memoria disponible. \n");
      return CKR_HOST_MEMORY;
    }
    
    hBlock = ( long * ) malloc ( sizeof(long) * bNumbers );
    
    if ( ! hBlock ) {
      LIBRT_FinalizarDispositivo(&handle);
      free(blocks);
      LOG_MsgError(1,"No Hay Memoria.");
      return CKR_HOST_MEMORY;
    }
    
    if ( LIBRT_LeerTodosBloquesOcupados ( &handle, hBlock, blocks, &bNumbers ) != 0 ) {
      LOG_MsgError(1, "No puedo enumerar objetos\n");
      return CKR_FUNCTION_FAILED;
    }
    
    for ( int i = 0 ; i < bNumbers ; i++ ) {
      type = *(blocks+1);
      if ( type ==  BLOQUE_KEY_CONTAINERS ){
	rewrite= 0;
	if ( BLOQUE_KeyContainer_Enumerar(blocks, lstContainer, &lstContainerSize) != 0 ) {
	  LOG_MsgError(1,"Enumerando keyContainers");
	  break;
	}
	for (int j = 0 ; j < lstContainerSize ; j++ ) {
	  if ( memcmp(lstContainer[j].idExchange, id, 20) == 0 ) {
	    if ( memcmp(lstContainer[j].idSignature, id, 20) == 0 || 
		 memcmp(lstContainer[j].idSignature, zero, 20) == 0) {
	      if ( BLOQUE_KeyContainer_Borrar ( blocks , lstContainer[j].nombreKeyContainer ) != 0 ){
		LOG_MsgError(1,"No se puede borrar la entrada asociada en el KeyContainer");
		return CKR_FUNCTION_FAILED;
	      }
	      else {
		/* Es necesario reescribir el bloque en el clauer */
		findCont= true;
		rewrite= 1; 
	      }
	    }
	    else{
	      if (  BLOQUE_KeyContainer_Establecer_ID_Exchange ( blocks, lstContainer[j].nombreKeyContainer, zero ) != 0  ){
		LOG_MsgError(1,"No se puede borrar la entrada asociada en el KeyContainer");
		return CKR_FUNCTION_FAILED;
	      }
	      else{
		findCont= true;
		rewrite= 1; 
	      }
	    }
	  } else if ( memcmp(lstContainer[j].idSignature, id, 20) == 0 ) {
	    if ( memcmp(lstContainer[j].idExchange , id, 20) == 0 || 
		 memcmp(lstContainer[j].idExchange , zero, 20) == 0) {
	      if ( BLOQUE_KeyContainer_Borrar ( blocks , lstContainer[j].nombreKeyContainer ) != 0 ){
		LOG_MsgError(1,"No se puede borrar la entrada asociada en el KeyContainer");
		return CKR_FUNCTION_FAILED;
	      }
	      else {
		findCont= true;
		/* Es necesario reescribir el bloque en el clauer */
		rewrite= 1; 
	      }
	    }
	    else{
	      if (  BLOQUE_KeyContainer_Establecer_ID_Signature ( blocks, lstContainer[j].nombreKeyContainer, zero ) != 0 ){
		LOG_MsgError(1,"No se pudo borrar la entrada asociada al KeyContainer");
		return CKR_FUNCTION_FAILED;
	      }
	      else{
		findCont= true;
		rewrite= 1;	
	      }
	    }
	  }
	}
	if ( rewrite == 1 ){
	  if ( BLOQUE_KeyContainer_Enumerar(blocks, lstContainer, &lstContainerSize) != 0 ) {
	    LOG_MsgError(1,"Enumerando KeyContainers");
	    break;
	  }
	  
	  if ( lstContainerSize == 0 ){
	    if ( LIBRT_BorrarBloqueCrypto ( &handle, hBlock[i] ) != 0 ) { 
	      LOG_Debug(1,"Borrando bloque = %d",hBlock[i]);
	      LOG_MsgError(1,"Error");
	      return CKR_FUNCTION_FAILED;
	    }
	  }
	  else{
	    if ( LIBRT_EscribirBloqueCrypto ( &handle, hBlock[i], blocks ) != 0 ){
	      LOG_MsgError(1,"Escribiendo los cambios en el clauer");
	      return CKR_FUNCTION_FAILED;
	    }
	  }
	}
      }
      else {
	found= 0;
	switch ( type ) {
	case BLOQUE_LLAVE_PRIVADA:
	  if ( memcmp(id, BLOQUE_LLAVEPRIVADA_Get_Id(blocks), 20 ) == 0 ){
	    found= 1;
	    findPem= true;
	  }
	  break;
	  
	case BLOQUE_CIPHER_PRIVKEY_PEM:
	  if ( memcmp(id, BLOQUE_CIPHER_PRIVKEY_PEM_Get_Id(blocks), 20 ) == 0 ){
	    found= 1;
	  }
	  break;		
	case BLOQUE_PRIVKEY_BLOB:
	  if ( memcmp(id, BLOQUE_PRIVKEYBLOB_Get_Id(blocks), 20 ) == 0 ){
	    found= 1;
	    findBlob= true;
	  }
	  break;
	  
	case BLOQUE_CIPHER_PRIVKEY_BLOB:
	  if ( memcmp(id, BLOQUE_PRIVKEYBLOB_Get_Id(blocks), 20 ) == 0 ){
	    found= 1;
	  }
	  break;
	}
	if ( found ){
	  if ( LIBRT_BorrarBloqueCrypto ( &handle, hBlock[i] ) != 0 ) { 
	    LOG_Debug(1, "Borrando bloque=%d\n",hBlock[i]);
	    LOG_MsgError(1, "ERROR");
	    return CKR_FUNCTION_FAILED;
	  }
	}
      }
      blocks += TAM_BLOQUE;
    }
    

	if ( LIBRT_FinalizarDispositivo(&handle)!= 0 ){
		LOG_MsgError(1,"Error finalizando el dispositivo");
	}


	if ( findPem && findBlob && findCont ){
	  pListaObjetos->eliminarObjeto(obj);
      return CKR_OK;
	}

    return CKR_FUNCTION_FAILED;
    
    break;

	case CKO_PUBLIC_KEY:
		pListaObjetos->eliminarObjeto(obj);
        return CKR_OK;
		break;
  
	default: // Object not allowed to delete
    return CKR_OBJECT_HANDLE_INVALID;
  }
 
  return CKR_OK;
}

CK_RV Clauer::C_GenerateKeyPair( CK_SESSION_HANDLE    hSession,                    /* the session's handle */
				 CK_MECHANISM_PTR     pMechanism,                  /* the key gen. mech. */
				 CK_ATTRIBUTE_PTR     pPublicKeyTemplate,          /* pub. attr. template */
				 CK_ULONG             ulPublicKeyAttributeCount,   /* # of pub. attrs. */
				 CK_ATTRIBUTE_PTR     pPrivateKeyTemplate,         /* priv. attr. template */
				 CK_ULONG             ulPrivateKeyAttributeCount,  /* # of priv. attrs. */
				 CK_OBJECT_HANDLE_PTR phPublicKey,                 /* gets pub. key handle */
				 CK_OBJECT_HANDLE_PTR phPrivateKey,                /* gets priv. key handle */
				 listaObjetos *       pListaObjetos                /* current object list pointer */            
				 )

{
  unsigned int pbk_mod_bits=0, pvk_mod_bits=0;
  unsigned long pbk_exponent=0, pvk_exponent=0;

  RSA* rsa= NULL;
   
  unsigned char * cka_id= NULL, realId[20];
  int cka_id_size= 0;
    
  CK_RV err;

  /* For key generation there must be authentication*/
  if (!_pin){
    LOG_MsgError(LOG_TO, "Intentando escribir objeto en sesion no autenticada");
    return CKR_USER_NOT_LOGGED_IN;
  }

  /* First of all we must check the mechanism */    
  if ( pMechanism->mechanism != CKM_RSA_PKCS_KEY_PAIR_GEN ){
    return CKR_MECHANISM_INVALID;
  }

  /* Now we examinate the attributes */
  for ( unsigned long i=0; i< ulPublicKeyAttributeCount; i++){
    if ( pPublicKeyTemplate[i].type == CKA_MODULUS_BITS ){
      pbk_mod_bits= *((int *)pPublicKeyTemplate[i].pValue);
    }
    else if ( pPublicKeyTemplate[i].type == CKA_PUBLIC_EXPONENT ){
      pbk_exponent= *((int *)pPublicKeyTemplate[i].pValue);
    }
  }

  for ( unsigned long i=0; i< ulPrivateKeyAttributeCount; i++){
    if ( pPrivateKeyTemplate[i].type == CKA_MODULUS_BITS ){
      pvk_mod_bits= *((long *)pPrivateKeyTemplate[i].pValue);;
    }
    else if ( pPrivateKeyTemplate[i].type == CKA_PUBLIC_EXPONENT ){
      pvk_exponent= *((long *)pPrivateKeyTemplate[i].pValue);
    }
    else if ( pPrivateKeyTemplate[i].type == CKA_ID ){
      cka_id= ( unsigned char * ) pPrivateKeyTemplate[i].pValue;
      cka_id_size= pPrivateKeyTemplate[i].ulValueLen;
    }
  }

  if ( pbk_mod_bits == 0 && pvk_mod_bits == 0){
    return CKR_TEMPLATE_INCOMPLETE;
  }
   
  if ( pbk_exponent == 0 && pvk_exponent == 0){
    /* pag 196, if pub exp is omitted, 65537 will be used */
	  LOG_Debug(0,"1- Using pbk_exponent: %ld",pbk_exponent); 
      pbk_exponent= 65537;
  }
  else{
    LOG_Debug(0,"2- Using pbk_exponent: %ld",pbk_exponent); 
  }

  if ( pbk_mod_bits != pvk_mod_bits && pbk_mod_bits != 0 && pvk_mod_bits != 0){
    return CKR_TEMPLATE_INCONSISTENT;
  } 
    
  if ( pbk_exponent != pvk_exponent && pbk_exponent != 0 && pvk_exponent != 0 ){
    return CKR_TEMPLATE_INCONSISTENT;
  }

  /* Now everything necessary is ok lets go to key generation */
  if ( pbk_mod_bits != 0 ){
    if ( pbk_mod_bits > pMechanism_keyGen->get_ulMaxKeySize() || 
	 pbk_mod_bits < pMechanism_keyGen->get_ulMinKeySize() ){
	    
      return CKR_ATTRIBUTE_VALUE_INVALID; 
		
    }

#ifdef WIN32

    HCRYPTPROV   hCryptProv;
    unsigned char  buff[4096];
    if(CryptAcquireContext( &hCryptProv,
			    NULL,
			    NULL,
			    PROV_RSA_FULL,
			    0)) {

      LOG_Msg(LOG_TO,"CryptAcquireContext para generar random con éxito.");
    }
    else
      {
	LOG_MsgError(LOG_TO,"Error durante CryptAcquireContext");
      }


    // Generate a random data for key generation.
    // at least 4 kB of data.
    if(CryptGenRandom(hCryptProv, 4096, buff)){
      LOG_Msg(LOG_TO, "Secuencia random para alimentar a RAND_seed generada con éxito. \n");
    }
    else
      {
	LOG_MsgError(LOG_TO, "Error durante CryptGenRandom.");
      }

    RAND_seed(buff, 4096);
#endif 

    rsa= RSA_generate_key(pbk_mod_bits, pbk_exponent, NULL, NULL);
  }
  else{	
    if ( pvk_mod_bits > pMechanism_keyGen->get_ulMaxKeySize() || 
	 pvk_mod_bits < pMechanism_keyGen->get_ulMinKeySize() ){
	    
      return CKR_ATTRIBUTE_VALUE_INVALID; 
	    
    }

    rsa= RSA_generate_key(pvk_mod_bits, pbk_exponent, NULL, NULL);
  }
    
    
  if ( rsa != NULL ){


#ifdef _DEBUG
	
    /*	
      unsigned char * pbkbuff;
      unsigned char * pvkbuff;
      int tam= 0;

      BIO * bpvk= BIO_new(BIO_s_mem());
      BIO * bpbk= BIO_new(BIO_s_mem());

      PEM_write_bio_RSAPrivateKey(bpvk, rsa, 0, 0, 0, 0, 0);
      PEM_write_bio_RSAPublicKey(bpbk, rsa);

      tam = BIO_get_mem_data(bpbk, &pbkbuff);
      pbkbuff[tam]= 0;
      LOG_Debug(LOG_TO, "\n\nClave pública: \n%s", pbkbuff);

      tam = BIO_get_mem_data(bpvk, &pvkbuff);
      pvkbuff[tam]= 0;
      LOG_Debug(LOG_TO, "\n\nClave privada: \n%s", pvkbuff);
    */
		
#endif 


    /* Call rsa key insertion with the given template */ 	
    err= _insertOpensslRsaKeyObject( rsa, pListaObjetos, cka_id, cka_id_size, phPrivateKey, realId, 
				     pPrivateKeyTemplate, ulPrivateKeyAttributeCount);
    if ( err != CKR_OK ){
      RSA_free( rsa );
      return CKR_FUNCTION_FAILED;
    }

    /* Now we must to insert public Key to the list object */
    err= _insertPublicKeyObject( rsa, cka_id, cka_id_size, realId, pListaObjetos, phPublicKey,
				 pPublicKeyTemplate, ulPublicKeyAttributeCount);
    if ( err != CKR_OK ){
      RSA_free( rsa );
      return CKR_FUNCTION_FAILED;
    }

    RSA_free( rsa );

  }
  else{
    /* Some kind of error has happened... */
    return CKR_FUNCTION_FAILED;
  }
    
  return CKR_OK; 
    
}
