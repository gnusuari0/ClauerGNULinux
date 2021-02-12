
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

#ifndef _CLVPRVDA_H_
#define _CLVPRVDA_H_ 1

#include "pkcs11.h"
#include "clave.h"

class ClavePrivada : public Clave
{
 public:
    ClavePrivada(void);	// Constructor -sobrecargado-
    ClavePrivada(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount);	// Constructor
    virtual ~ClavePrivada(void);	// Destructor
    void copiarObjeto(ClavePrivada *pNuevoObjeto);
    CK_RV modificarObjeto(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount);
    void volcarPlantilla(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulMaxCount, CK_ULONG &ulCount);
    unsigned int numeroAtributos(void);
    CK_BYTE *get_subject (void);
    CK_BBOOL get_sensitive (void);
    CK_BBOOL get_decrypt (void);
    CK_BBOOL get_sign (void);
    CK_BBOOL get_signRecover (void);
    CK_BBOOL get_unwrap (void);
    CK_BBOOL get_extractable (void);
    CK_BBOOL get_alwaysSensitive (void);
    CK_BBOOL get_neverExtractable (void);

 protected:
    CK_BYTE *subject;
    CK_ULONG tamSubject;
    CK_BBOOL sensitive;
    CK_BBOOL bsensitive;
    CK_BBOOL decrypt;
    CK_BBOOL bdecrypt;
    CK_BBOOL sign;
    CK_BBOOL bsign;
    CK_BBOOL signRecover;
    CK_BBOOL bsignRecover;
    CK_BBOOL unwrap;
    CK_BBOOL bunwrap;
    CK_BBOOL extractable;
    CK_BBOOL bextractable;
    CK_BBOOL alwaysSensitive;
    CK_BBOOL balwaysSensitive;
    CK_BBOOL neverExtractable;
    CK_BBOOL bneverExtractable;
};

inline CK_BYTE *ClavePrivada::get_subject (void)
{
    return subject;
}

inline CK_BBOOL ClavePrivada::get_sensitive (void)
{
    return sensitive;
}

inline CK_BBOOL ClavePrivada::get_decrypt (void)
{
    return decrypt;
}

inline CK_BBOOL ClavePrivada::get_sign (void)
{
    return sign;
}

inline CK_BBOOL ClavePrivada::get_signRecover (void)
{
    return signRecover;
}

inline CK_BBOOL ClavePrivada::get_unwrap (void)
{
    return unwrap;
}

inline CK_BBOOL ClavePrivada::get_extractable (void)
{
    return extractable;
}

inline CK_BBOOL ClavePrivada::get_alwaysSensitive (void)
{
    return alwaysSensitive;
}

inline CK_BBOOL ClavePrivada::get_neverExtractable (void)
{
    return neverExtractable;
}

#endif
