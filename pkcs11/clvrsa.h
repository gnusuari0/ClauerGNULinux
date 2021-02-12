
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

#ifndef _CLVRSA_H_
#define _CLVRSA_H_ 1

#include "pkcs11.h"
#include "clvprvda.h"
#include "common.h"
// Incluimos rsa.h porque vamos a tener un atributo de tipo RSA
#include <stdio.h>
#include "openssl/rsa.h"

class ClaveRSA : public ClavePrivada
{
 public:
    ClaveRSA(void);	// Constructor -sobrecargado-
    ClaveRSA(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount, unsigned char * certId );	// Constructor
    ~ClaveRSA(void);
    void copiarObjeto(ClaveRSA *pNuevoObjeto);
    CK_RV modificarObjeto(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount);
    void volcarPlantilla(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulMaxCount, CK_ULONG &ulCount);
    unsigned int numeroAtributos(void);
    CK_BYTE *get_modulus (void);
	CK_ULONG get_modulus_size (void);
    
	CK_BYTE *get_publicExponent (void);
    CK_ULONG get_publicExponent_size (void);
	
	CK_BYTE *get_privateExponent (void);
    CK_BYTE *get_prime1 (void);
    CK_BYTE *get_prime2 (void);
    CK_BYTE *get_exponent1 (void);
    CK_BYTE *get_exponent2 (void);
    CK_BYTE *get_coefficient (void);
    unsigned char *get_keyId(void);
    RSA *get_clave (void);
 protected:
    CK_BYTE *modulus;
    CK_ULONG tamModulus;
    CK_BYTE *publicExponent;
    CK_ULONG tamPublicExponent;
    CK_BYTE *privateExponent;
    CK_ULONG tamPrivateExponent;
    CK_BYTE *prime1;
    CK_ULONG tamPrime1;
    CK_BYTE *prime2;
    CK_ULONG tamPrime2;
    CK_BYTE *exponent1;
    CK_ULONG tamExponent1;
    CK_BYTE *exponent2;
    CK_ULONG tamExponent2;
    CK_BYTE *coefficient;
    CK_ULONG tamCoefficient;
    // NOTA: El siguiente atributo no pertenece al objeto de la clase RSA
    //       Va a contener la clave que se utilizara para descifrar
    RSA *clave;
    CK_ULONG tamClave;
    unsigned char _keyId[CERT_ID_LEN];
};


inline CK_BYTE *ClaveRSA::get_keyId (void)
{
    return _keyId;
}

inline CK_BYTE *ClaveRSA::get_modulus (void)
{
    return modulus;
}

inline CK_ULONG ClaveRSA::get_modulus_size (void)
{
    return tamModulus;
}

inline CK_BYTE *ClaveRSA::get_publicExponent (void)
{
    return publicExponent;
}

inline CK_ULONG ClaveRSA::get_publicExponent_size (void)
{
    return tamPublicExponent;
}

inline CK_BYTE *ClaveRSA::get_privateExponent (void)
{
    return privateExponent;
}

inline CK_BYTE *ClaveRSA::get_prime1 (void)
{
    return prime1;
}

inline CK_BYTE *ClaveRSA::get_prime2 (void)
{
    return prime2;
}

inline CK_BYTE *ClaveRSA::get_exponent1 (void)
{
    return exponent1;
}

inline CK_BYTE *ClaveRSA::get_exponent2 (void)
{
    return exponent2;
}

inline CK_BYTE *ClaveRSA::get_coefficient (void)
{
    return coefficient;
}

inline RSA *ClaveRSA::get_clave (void)
{
    return clave;
}

#endif
