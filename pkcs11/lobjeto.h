
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

#ifndef _LOBJETO_H_
#define _LOBJETO_H_ 1


#include "pkcs11.h"
#include "objeto.h"

#ifdef DEBUG
#include "log.h"
#endif

//----------------------------------------------------------------
//------------------------nodoObjeto------------------------------
//----------------------------------------------------------------

class nodoObjeto
{
 public:
    nodoObjeto(Objeto *valor);	//Constructor
    ~nodoObjeto(void);			//Destructor

    void set_sig(nodoObjeto *valor);
    nodoObjeto *get_sig(void);
	
    void set_ant(nodoObjeto *valor);
    nodoObjeto *get_ant(void);
	
    void set_objeto(Objeto *valor);
    Objeto *get_objeto(void);
 protected:
    nodoObjeto *sig;
    nodoObjeto *ant;
    Objeto *pObjeto;
	

};

// Metodos set y get
inline void nodoObjeto::set_sig(nodoObjeto *valor)
{
    sig = valor;
}

inline nodoObjeto *nodoObjeto::get_sig(void)
{
    return sig;
}
	
inline void nodoObjeto::set_ant(nodoObjeto *valor)
{
    ant = valor;
}

inline nodoObjeto *nodoObjeto::get_ant(void)
{
    return ant;
}

inline void nodoObjeto::set_objeto(Objeto *valor)
{
    pObjeto = valor;
}

inline Objeto *nodoObjeto::get_objeto(void)
{
    return pObjeto;
}
//----------------------------------------------------------------
//------------------------listaObjetos----------------------------
//----------------------------------------------------------------

class listaObjetos
{
 public:
    listaObjetos(void);
    ~listaObjetos(void);

    void insertarObjeto(Objeto *valor);
    void deleteAllObjects(void);
    void eliminarObjeto(Objeto *valor);
    Objeto *buscarObjeto(CK_OBJECT_HANDLE valor);
	
    void iniciarBusquedaPlantilla(CK_ATTRIBUTE_PTR pPlantilla, CK_ULONG numeroAtributos);
    void iniciarBusqueda(void);   
    CK_OBJECT_HANDLE buscarPlantilla(void);
    void finalizarBusquedaPlantilla(void);
    CK_BBOOL estaBusquedaIniciada(void);

    void avanzar(void);
    int  esFin(void);
    Objeto *elementoActual(void);
	
	
 protected:
    CK_ATTRIBUTE_PTR pPlantillaBusqueda;
    CK_ULONG plantillaNumeroAtributos;
    CK_BBOOL busquedaIniciada;
    nodoObjeto *cabeceraListaObjetos;
    nodoObjeto *cursor;
    int _contador;
	
};

inline CK_BBOOL listaObjetos::estaBusquedaIniciada(void)
{
    return busquedaIniciada;
}

#endif
