
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
 * Description: Contains the functions related to the Mechanism 
 *              associated with clauer token. 
 * 
 * Notes: Windows code is not been implemented yet. 
 *  
 * 
 *                         Clauer Team 2006 
 **/               


#ifndef __MECNISM_H__
#define __MECNISM_H__ 

#include "pkcs11.h"

// Key sizes
#define MIN_KEY_SIZE 1024 
#define MAX_KEY_SIZE 4096

class Mechanism 
{
 public:
    Mechanism(CK_MECHANISM_TYPE _type, CK_ULONG _ulMinKeySize, CK_ULONG _ulMaxKeySize, CK_FLAGS _flags);
    ~Mechanism(void);	// Destructor
    CK_RV C_GetMechanismInfo(CK_MECHANISM_INFO_PTR pInfo);
  
    CK_MECHANISM_TYPE get_type (void);
    CK_ULONG get_ulMinKeySize (void);
    CK_ULONG get_ulMaxKeySize (void);
    CK_FLAGS get_flags (void);
  
 protected:
    CK_MECHANISM_TYPE type;
    CK_ULONG ulMinKeySize;
    CK_ULONG ulMaxKeySize;
    CK_FLAGS flags;
};
// Metodos get
inline CK_MECHANISM_TYPE Mechanism::get_type (void)
{
    return type;
}

inline CK_ULONG Mechanism::get_ulMinKeySize (void)
{
    return ulMinKeySize;
}

inline CK_ULONG Mechanism::get_ulMaxKeySize (void)
{
    return ulMaxKeySize;
}

inline CK_FLAGS Mechanism::get_flags (void)
{
    return flags;
}

#endif
