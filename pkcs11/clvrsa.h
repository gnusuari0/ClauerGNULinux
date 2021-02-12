
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
