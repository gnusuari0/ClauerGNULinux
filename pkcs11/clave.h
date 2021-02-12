
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

#ifndef _CLAVE_H_
#define _CLAVE_H_ 1

#include "pkcs11.h"
#include "objeto.h"

class Clave : public Objeto
{
 public:
    Clave(void);	// Constructor -sobrecargado-
    Clave(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount);	// Constructor
    ~Clave(void);	// Destructor
    void copiarObjeto(Clave *pNuevoObjeto);
    CK_RV modificarObjeto(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount);
    void volcarPlantilla(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulMaxCount, CK_ULONG &ulCount);
    unsigned int numeroAtributos(void);

    CK_KEY_TYPE get_keyType (void);
    CK_BYTE *get_id (void);
    CK_DATE get_startDate (void);
    CK_DATE get_endDate (void);
    CK_BBOOL get_derive (void);
    CK_BBOOL get_local (void);
 protected:	// atributos
    CK_KEY_TYPE keyType;
    CK_BBOOL bkeyType;
    CK_BYTE *id;
    CK_ULONG tamId;
    CK_DATE startDate;
    CK_BBOOL bstartDate;
    CK_DATE endDate;
    CK_BBOOL bendDate;
    CK_BBOOL derive;
    CK_BBOOL bderive;
    CK_BBOOL local;
    CK_BBOOL blocal;
};


// Metodos get
inline CK_KEY_TYPE Clave::get_keyType (void)
{
    return keyType;
}

inline CK_BYTE *Clave::get_id (void)
{
    return id;
}

inline CK_DATE Clave::get_startDate (void)
{
    return startDate;
}

inline CK_DATE Clave::get_endDate (void)
{
    return endDate;
}

inline CK_BBOOL Clave::get_derive (void)
{
    return derive;
}

inline CK_BBOOL Clave::get_local (void)
{
    return local;
}

#endif
