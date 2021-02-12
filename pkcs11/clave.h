
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
