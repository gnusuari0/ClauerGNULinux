
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

#include "common.h"	// buscarEnPlantilla
#include "objeto.h"
#include <string.h>		// memcpy
#include <stdlib.h>		// free
#include <stdio.h>

// Inicializamos el handle. Conforme se vayan creando objetos,
// su valor se ira incrementando
// El valor 0 esta reservado
CK_OBJECT_HANDLE Objeto::handleUnico = 1;

Objeto::Objeto(void)
{
    handle = handleUnico;
    // Incrementamos el valor de handleUnico para que el siguiente
    // objeto que se cree tenga un handle distinto
    handleUnico++;

    bclass      = FALSE;
    btoken      = FALSE;
    bprivate    = FALSE;
    bmodifiable = FALSE;
    // El tamano del atributo label es 0, lo que indica que el
    // objeto no tiene etiqueta
    tamLabel    = 0;
}

Objeto::Objeto(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount)
{
    unsigned int contador;

    handle = handleUnico;
    LOG_Debug(LOG_TO,"Handle �nico para objetos vale= %d",handle);

    // Incrementamos el valor de handleUnico para que el siguiente
    // objeto que se cree tenga un handle distinto
    handleUnico++;

    bclass      = FALSE;
    btoken      = FALSE;
    bprivate    = FALSE;
    bmodifiable = FALSE;

    // El tamano del atributo label es 0, lo que indica que el
    // objeto no tiene etiqueta
    tamLabel    = 0;

    contador = 0;
    while (contador < ulCount)
	{
	    switch(pTemplate[contador].type)
		{
		case CKA_CLASS:
		    _class = *((CK_OBJECT_CLASS *)pTemplate[contador].pValue);
		    bclass = TRUE;
		    break;
		case CKA_TOKEN:
		    token = *((CK_BBOOL *)pTemplate[contador].pValue);
		    btoken = TRUE;
		    break;
		case CKA_PRIVATE:
		    _private = *((CK_BBOOL *)pTemplate[contador].pValue);
		    bprivate = TRUE;
		    break;
		case CKA_MODIFIABLE:
		    modifiable = *((CK_BBOOL *)pTemplate[contador].pValue);
		    bmodifiable = TRUE;
		    break;
		case CKA_LABEL:
		    tamLabel = pTemplate[contador].ulValueLen;
		    label = (CK_CHAR_PTR) malloc(tamLabel);
		    memcpy(label,pTemplate[contador].pValue,tamLabel);
		    break;
		}
	    contador++;
	}
}

Objeto::~Objeto(void)
{
    if (tamLabel)
	free(label);
}

void Objeto::copiarObjeto(Objeto *pNuevoObjeto)
{
    pNuevoObjeto->_class = this->_class;
    pNuevoObjeto->bclass = this->bclass;
    pNuevoObjeto->token = this->token;
    pNuevoObjeto->btoken = this->btoken;
    pNuevoObjeto->_private = this->_private;
    pNuevoObjeto->bprivate = this->bprivate;
    pNuevoObjeto->modifiable = this->modifiable;
    pNuevoObjeto->bmodifiable = this->bmodifiable;
	
    if(this->tamLabel)
	{
	    pNuevoObjeto->label = (unsigned char *) malloc (tamLabel);
	    memcpy(pNuevoObjeto->label,this->label,tamLabel);
	}
    else
	pNuevoObjeto->label = NULL;

    pNuevoObjeto->tamLabel = this->tamLabel;
}

CK_RV Objeto::modificarObjeto(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount)
{
    unsigned long contador;


    contador = 0;
    while (contador < ulCount)
	{

	    switch(pTemplate[contador].type)
		{
		case CKA_CLASS:
		    return CKR_ATTRIBUTE_READ_ONLY;
		case CKA_TOKEN:
		    if (bmodifiable && modifiable)
			{
			    token = *((CK_BBOOL *)pTemplate[contador].pValue);
			    btoken = TRUE;
			}
		    else{

			return CKR_ATTRIBUTE_READ_ONLY;					
		    }
		    break;
		case CKA_PRIVATE:
		    if (bmodifiable && modifiable)
			{
			    _private = *((CK_BBOOL *)pTemplate[contador].pValue);
			    bprivate = TRUE;
			}
		    else{
	
			return CKR_ATTRIBUTE_READ_ONLY;
		    }
		    break;
		case CKA_MODIFIABLE:
		    if (bmodifiable && modifiable)
			{
			    modifiable = *((CK_BBOOL *)pTemplate[contador].pValue);
			    bmodifiable = TRUE;
			}
		    else{
	
			return CKR_ATTRIBUTE_READ_ONLY;
		    }
		    break;
		case CKA_LABEL:
		    if (bmodifiable && modifiable)
			{
			    if (pTemplate[contador].ulValueLen != tamLabel)
				{
				    free(label);
				    tamLabel = pTemplate[contador].ulValueLen;
				    label = (CK_CHAR_PTR) malloc(tamLabel);
				}
			    memcpy(label,pTemplate[contador].pValue,tamLabel);
			}
		    else{
	
			return CKR_ATTRIBUTE_READ_ONLY;
		    }
		    break;
		default:
	
		    return CKR_ATTRIBUTE_TYPE_INVALID;
		}
	    contador++;
	}
    return CKR_OK;
}

void Objeto::volcarPlantilla(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulMaxCount, CK_ULONG &ulCount)
{
    unsigned long contador = 0;

    if (contador < ulMaxCount)
	{
	    pTemplate[contador].type = CKA_CLASS;
	    if (bclass)
		{
		    pTemplate[contador].ulValueLen = sizeof(_class);
		    pTemplate[contador].pValue = (CK_OBJECT_CLASS *) malloc(pTemplate[contador].ulValueLen);
		    *((CK_OBJECT_CLASS *)(pTemplate[contador].pValue)) = _class;
		}
	    else
		{
		    pTemplate[contador].pValue = NULL;
		    pTemplate[contador].ulValueLen = (CK_LONG) -1;
		}
	    contador++;
	}
	
    if (contador < ulMaxCount)
	{
	    pTemplate[contador].type = CKA_TOKEN;
	    if (btoken)
		{
		    pTemplate[contador].ulValueLen = sizeof(token);
		    pTemplate[contador].pValue = (CK_BBOOL *) malloc(pTemplate[contador].ulValueLen);
		    *((CK_BBOOL *)(pTemplate[contador].pValue)) = token;

		}
	    else
		{
		    pTemplate[contador].pValue = NULL;
		    pTemplate[contador].ulValueLen = (CK_LONG) -1;
		}
	    contador++;
	}
	
    if (contador < ulMaxCount)
	{
	    pTemplate[contador].type = CKA_PRIVATE;
	    if (bprivate)
		{
		    pTemplate[contador].ulValueLen = sizeof(_private);
		    pTemplate[contador].pValue = (CK_BBOOL *) malloc(pTemplate[contador].ulValueLen);
		    *((CK_BBOOL *)(pTemplate[contador].pValue)) = _private;

		}
	    else
		{
		    pTemplate[contador].pValue = NULL;
		    pTemplate[contador].ulValueLen = (CK_LONG) -1;
		}
	    contador++;
	}
	
    if (contador < ulMaxCount)
	{
	    pTemplate[contador].type = CKA_MODIFIABLE;
	    if (bmodifiable)
		{
		    pTemplate[contador].ulValueLen = sizeof(modifiable);
		    pTemplate[contador].pValue = (CK_BBOOL *) malloc(pTemplate[contador].ulValueLen);
		    *((CK_BBOOL *)(pTemplate[contador].pValue)) = modifiable;

		}
	    else
		{
		    pTemplate[contador].pValue = NULL;
		    pTemplate[contador].ulValueLen = (CK_LONG) -1;
		}
	    contador++;
	}

    if (contador < ulMaxCount)
	{
	    pTemplate[contador].type = CKA_LABEL;
	    if (tamLabel)
		{
		    pTemplate[contador].ulValueLen = tamLabel;
		    pTemplate[contador].pValue = (char *) malloc(tamLabel);
		    memcpy(pTemplate[contador].pValue,label,tamLabel);
		}
	    else
		{
		    pTemplate[contador].pValue = NULL;
		    pTemplate[contador].ulValueLen = (CK_LONG) -1;
		}
	    contador++;
	}
    ulCount = contador;
}

unsigned int Objeto::numeroAtributos(void)
{
    // El objeto tiene 5 atributos (no se tienen en cuenta los atributos pareja)
    return 5;
}


