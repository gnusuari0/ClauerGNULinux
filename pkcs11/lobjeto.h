
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
