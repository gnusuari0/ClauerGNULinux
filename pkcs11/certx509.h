
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
