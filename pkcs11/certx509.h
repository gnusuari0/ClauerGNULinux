
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

#ifndef _CERTIFICADO_X509_H_
#define _CERTIFICADO_X509_H_ 1

#include "pkcs11.h"
#include "certif.h"
#include "common.h"

class CertificadoX509 : public Certificado
{
 public:
    CertificadoX509(void);		// Constructor -sobrecargado-
    CertificadoX509(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount, unsigned char * certId);	// Constructor
    ~CertificadoX509(void);		// Destructor
    void copiarObjeto(CertificadoX509 *pNuevoObjeto);
    CK_RV modificarObjeto(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount);
    void volcarPlantilla(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulMaxCount, CK_ULONG &ulCount);
    unsigned int numeroAtributos(void);

    CK_BYTE *get_subject (void);
    CK_BYTE *get_id (void);
    CK_BYTE *get_issuer (void);
    CK_BYTE *get_serial_number (void);
    CK_BYTE *get_value (void);
	unsigned char * get_certId(void);
 protected:
    // Por cada atributo tenemos otro con el mismo nombre, pero con
    // el prefijo 'tam', que guarda el tamano del objeto apuntado
    // por el atributo pareja.

    CK_BYTE *subject;
    CK_ULONG tamSubject;

    CK_BYTE *id;
    CK_ULONG tamId;

    CK_BYTE *issuer;
    CK_ULONG tamIssuer;
	
    CK_BYTE *serial_number;
    CK_ULONG tamSerial_number;
	
    CK_BYTE *value;
    CK_ULONG tamValue;
	unsigned char _certId[CERT_ID_LEN];

};

// Metodos get

inline unsigned char *CertificadoX509::get_certId(void)
{
	return _certId;
}

inline CK_BYTE *CertificadoX509::get_subject (void)
{
    return subject;
}

inline CK_BYTE *CertificadoX509::get_id (void)
{
    return id;
}

inline CK_BYTE *CertificadoX509::get_issuer (void)
{
    return issuer;
}

inline CK_BYTE *CertificadoX509::get_serial_number (void)
{
    return serial_number;
}

inline CK_BYTE *CertificadoX509::get_value (void)
{
    return value;
}

#endif
