
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

#include "common.h"	// buscarEnPlantilla
#include "lobjeto.h"
#include <stdlib.h>		// free
#include <string.h>		// memcmp
#include <stdio.h>


#include "log.h"


nodoObjeto::nodoObjeto(Objeto *valor)
{
    ant = NULL_PTR;
    sig = NULL_PTR;
    pObjeto = valor;
}

nodoObjeto::~nodoObjeto(void)
{
    delete pObjeto;
}

listaObjetos::listaObjetos()
{
    cabeceraListaObjetos = NULL_PTR;
    cursor = NULL_PTR;
    busquedaIniciada = FALSE;
    _contador= 0;
}

listaObjetos::~listaObjetos()
{

    nodoObjeto *pNodo;
 
    pNodo = cabeceraListaObjetos;

    //Recorremos la lista, liberando la memoria ocupada por cada nodo
    while (pNodo)
	{
	    cabeceraListaObjetos = pNodo->get_sig();
	    delete pNodo;
	    pNodo = cabeceraListaObjetos;
	}
}

void listaObjetos::insertarObjeto(Objeto *valor)
{

    nodoObjeto *pNodo;
    
    pNodo = new nodoObjeto(valor);
    
    if (pNodo)
	{	   
	    LOG_Debug(LOG_TO, "Insertando objeto numero %d", _contador); 
	    _contador++;
	    pNodo->set_sig(cabeceraListaObjetos);
	    if (cabeceraListaObjetos)
		cabeceraListaObjetos->set_ant(pNodo);
	    pNodo->set_ant(NULL_PTR);
	    cabeceraListaObjetos = pNodo;
	}
}


void listaObjetos::deleteAllObjects()
{
    
    nodoObjeto *pNodo;
    
    pNodo = cabeceraListaObjetos;
    
    //Recorremos la lista, liberando la memoria ocupada por cada nodo
    while (pNodo)
	{
	    
	    LOG_Debug(LOG_TO, " Borramos objeto= %d", pNodo->get_objeto()->get_handle() ); 
	    cabeceraListaObjetos = pNodo->get_sig();
	    delete pNodo;
	    pNodo = cabeceraListaObjetos;
	}
    
    cabeceraListaObjetos = NULL_PTR;
    cursor = NULL_PTR;
    busquedaIniciada = FALSE;
    _contador= 0;
}

void listaObjetos::eliminarObjeto(Objeto *valor)
{
    nodoObjeto *pNodo;
    int encontrado;

    pNodo = cabeceraListaObjetos;
    encontrado = 0;

    // Buscamos el nodo que contiene el Objeto que se pasa como parametro
    while((pNodo) && (!encontrado))
	{
	    if (pNodo->get_objeto() == valor)
		encontrado = 1;
	    else
		pNodo = pNodo->get_sig();
	}
    // Si lo encontramos, entonces se elimina
    if (encontrado)
	{
	    if (pNodo->get_ant())
		(pNodo->get_ant())->set_sig(pNodo->get_sig());
	    if (pNodo->get_sig())
		(pNodo->get_sig())->set_ant(pNodo->get_ant());
	    if (cabeceraListaObjetos == pNodo)
		cabeceraListaObjetos = pNodo->get_sig();
	    delete pNodo;
	}	
}

Objeto *listaObjetos::buscarObjeto(CK_OBJECT_HANDLE valor)
{
    
    nodoObjeto *pNodo;
    int encontrado;
    
    
    LOG_Debug(LOG_TO,"Loking for object with handle=%d", valor); 
    
    pNodo = cabeceraListaObjetos;
    encontrado = 0;
        
    // Hay que recorrer la lista comparando el atributo 'handle' de cada
    // objeto con el parametro 'valor'
    while((pNodo) && (!encontrado))
	{       
	    if (pNodo->get_objeto()->get_handle() == valor){
		encontrado = 1;
		
		LOG_Msg(LOG_TO,"Objeto encontrado"); 
	    }
	    else
		pNodo = pNodo->get_sig();
	}
    if (encontrado)
	return pNodo->get_objeto();
    else
	return NULL_PTR;
}

void listaObjetos::iniciarBusqueda( void )
{
    cursor = cabeceraListaObjetos;
}

void listaObjetos::iniciarBusquedaPlantilla(CK_ATTRIBUTE_PTR pPlantilla, CK_ULONG numeroAtributos)
{
    pPlantillaBusqueda = pPlantilla;
    plantillaNumeroAtributos = numeroAtributos;
    cursor = cabeceraListaObjetos;
    busquedaIniciada = TRUE;
}

CK_OBJECT_HANDLE listaObjetos::buscarPlantilla(void)
{
    Objeto *pObjeto;
    CK_ATTRIBUTE_PTR pAtributo, pPlantillaVolcada;
    CK_ULONG numeroAtributos, numAtribPlantillaVolcada;
    unsigned long contador;
    CK_BBOOL salida;

    // Vemos si se quieren buscar todos los objetos -no importa la plantilla de busqueda-	
    if (plantillaNumeroAtributos == 0)
	{
	    if (!esFin())
		{
		    pObjeto = elementoActual();
		    avanzar();
		    return pObjeto->get_handle();
		}
	    else
		return (CK_OBJECT_HANDLE) 0;
	}
    else
	{
	    // Recorremos la lista de objetos
	    while (!esFin())
		{
		    pObjeto = elementoActual();
		    // Volcamos los atributos del objeto a una plantilla para facilitar la busqueda
		    numeroAtributos = pObjeto->numeroAtributos();
		    pPlantillaVolcada = (CK_ATTRIBUTE_PTR) malloc(numeroAtributos * sizeof(CK_ATTRIBUTE));
		    pObjeto->volcarPlantilla(pPlantillaVolcada,numeroAtributos,numAtribPlantillaVolcada);

		    // Comparamos atributo por atributo
		    contador = 0;
		    salida = FALSE;
		    while (contador < plantillaNumeroAtributos && !salida)
			{
			    // Buscamos un atributo de la plantilla de busqueda en la plantilla volcada del objeto
			    pAtributo = buscarEnPlantilla(pPlantillaVolcada,numAtribPlantillaVolcada,pPlantillaBusqueda[contador].type);
			    if (pAtributo)
				{
					LOG_Debug(LOG_TO,"Encontrado atributo = 0x%x en la búsqueda",pPlantillaBusqueda[contador].type);
					LOG_Debug(LOG_TO,"    Valor = 0x%x",*((int *)pPlantillaBusqueda[contador].pValue));	
				    if(pAtributo->ulValueLen == pPlantillaBusqueda[contador].ulValueLen)
					{
						LOG_Debug(LOG_TO,"Tiene igual tamaño = %d", pPlantillaBusqueda[contador].ulValueLen);
						if (!memcmp(pAtributo->pValue,pPlantillaBusqueda[contador].pValue,pAtributo->ulValueLen))
						{
							LOG_Msg(LOG_TO,"Iguales !");						
							contador++;
						}
					    else
							salida = TRUE;  // Salimos del bucle porque el valor de los atributos no es el mismo
					}
					else{
					salida = TRUE;  // Salimos del bucle porque los atributos no tienen igual tamaño
					LOG_Debug(LOG_TO, "Tamaños distintos pAtributo->ulValueLen= %d", pAtributo->ulValueLen);
					LOG_Debug(LOG_TO, "           Contenido= %s", pAtributo->pValue);
					LOG_Debug(LOG_TO, "Tamaños distintos pPlantillaBusqueda[contador].ulValueLen= %d", pPlantillaBusqueda[contador].ulValueLen);
					LOG_Debug(LOG_TO, "           Contenido= %s", pPlantillaBusqueda[contador].pValue);
					}
				}
			    else
				salida = TRUE;  // Salimos del bucle porque no hemos encontrado el atributo
			}
		    destruirPlantilla(pPlantillaVolcada,numAtribPlantillaVolcada);
		    // Dejamos el cursor apuntando al siguiente objeto de la lista
		    // para que en sucesivas llamadas, continuar a partir de donde lo dejamos
		    avanzar();
		    if (contador == plantillaNumeroAtributos){			  
			LOG_Debug(LOG_TO,"Object found with handle  %d",pObjeto->get_handle()); 
			return pObjeto->get_handle();
		    }
		}
	    return (CK_OBJECT_HANDLE) 0;
	}
}


void listaObjetos::finalizarBusquedaPlantilla(void)
{
    CK_ULONG contador;

    for (contador = 0; contador < plantillaNumeroAtributos; contador++)
	if (pPlantillaBusqueda[contador].pValue)
	    free(pPlantillaBusqueda[contador].pValue);
    free(pPlantillaBusqueda);
    busquedaIniciada = FALSE;
}

void listaObjetos::avanzar(void)
{
    cursor = cursor->get_sig();
}

int listaObjetos::esFin(void)
{
    return cursor == NULL_PTR;
}

Objeto *listaObjetos::elementoActual(void)
{
    return cursor->get_objeto();
}

