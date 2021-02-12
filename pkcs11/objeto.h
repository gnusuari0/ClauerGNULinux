
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

#ifndef _OBJETO_H_
#define _OBJETO_H_ 1

#include "pkcs11.h"
#include "log.h"

class Objeto 
{
 public:
    Objeto(void);  // Constructor -sobrecargado-
    Objeto(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount);	// Constructor
    virtual ~Objeto(void);
    virtual void copiarObjeto(Objeto *pNuevoObjeto);
    virtual CK_RV modificarObjeto(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount);
    CK_OBJECT_HANDLE get_handle(void);
    CK_OBJECT_CLASS get_class (void);
    int set_handle(int handle);
    virtual void volcarPlantilla(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulMaxCount, CK_ULONG &ulCount);
    virtual unsigned int numeroAtributos(void);
 protected:
    CK_BBOOL get_token (void);
    CK_BBOOL get_private (void);
    CK_BBOOL get_modifiable (void);
    CK_CHAR *get_label (void);
    // El atributo handle es para identificar al objeto
    CK_OBJECT_HANDLE handle;	

    // Por cada atributo tenemos otro con el mismo nombre, pero con una 'b'
    // delante, que indica si el atributo tiene un valor asignado.
    // Esto es asi porque a la hora de crear un objeto nos pueden pasar
    // una plantilla en la que no se especifiquen todos los valores de 
    // todos los atributos.
    // Cuando el atributo es de tipo puntero, en vez de tener otro atributo
    // pareja cuya primera letra es una 'b' lo que se tiene es un atributo
    // que empieza por 'tam' y que guarda el tamano de su pareja.

    CK_OBJECT_CLASS _class;
    CK_BBOOL bclass;

    CK_BBOOL token;
    CK_BBOOL btoken;

    CK_BBOOL _private;
    CK_BBOOL bprivate;

    CK_BBOOL modifiable;
    CK_BBOOL bmodifiable;

    CK_CHAR *label;
    CK_ULONG tamLabel;
    // Atributo de clase. Se utiliza para proporcionar una handle unico
    // a cada objeto
    static CK_OBJECT_HANDLE handleUnico;
};


// Metodos set y get

inline int Objeto::set_handle(int ihandle)
{
    // TODO: Aqu� quiz�s deber�amos comprobar que el handle 
    // no este siendo utilizado por otro objeto.

    LOG_Debug(1,"Cambiando handle a %d\n",ihandle);
    handle= ihandle;
    return 1;
}


inline CK_OBJECT_HANDLE Objeto::get_handle(void)
{
    return handle;
}

inline CK_OBJECT_CLASS Objeto::get_class (void)
{
    return _class;
}

inline CK_BBOOL Objeto::get_token (void)
{
    return token;
}

inline CK_BBOOL Objeto::get_private (void)
{
    return _private;
}

inline CK_BBOOL Objeto::get_modifiable (void)
{
    return modifiable;
}

inline CK_CHAR *Objeto::get_label ()
{
    return label;
}


#endif
