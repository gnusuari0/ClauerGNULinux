
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

#include "clave.h"
#include <string.h>		// memcpy
#include <stdlib.h>		// free
#include <stdio.h>

Clave::Clave(void):Objeto()
{
    bkeyType = FALSE;
    tamId    = 0;
    bstartDate = FALSE;
    bendDate = FALSE;
    bderive  = FALSE;
    blocal   = FALSE;
}

Clave::Clave(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount):Objeto(pTemplate,ulCount)
{
    unsigned int contador;

    bkeyType = FALSE;
    tamId    = 0;
    bstartDate = FALSE;
    bendDate = FALSE;
    bderive  = FALSE;
    blocal   = FALSE;

    contador = 0;
    while (contador < ulCount)
	{	 
	    switch(pTemplate[contador].type)
		{
		case CKA_KEY_TYPE:
		    keyType = *((CK_KEY_TYPE *)pTemplate[contador].pValue);
		    bkeyType = TRUE;
		    break;
		case CKA_ID:
		    tamId = pTemplate[contador].ulValueLen;				
		    id = (CK_BYTE *) malloc(tamId);
		    memcpy(id,pTemplate[contador].pValue,tamId);	       
		    break;
		case CKA_START_DATE:
		    memcpy((CK_BYTE *)&startDate,(CK_BYTE *)pTemplate[contador].pValue,pTemplate[contador].ulValueLen);
		    bstartDate = TRUE;
		    break;
		case CKA_END_DATE:
		    memcpy((CK_BYTE *)&endDate,(CK_BYTE *)pTemplate[contador].pValue,pTemplate[contador].ulValueLen);
		    bendDate = TRUE;
		    break;
		case CKA_DERIVE:
		    derive = *((CK_BBOOL *)pTemplate[contador].pValue);
		    bderive = TRUE;
		    break;
		case CKA_LOCAL:
		    local = *((CK_BBOOL *)pTemplate[contador].pValue);
		    blocal = TRUE;
		    break;
		}
	    contador++;
	}
}

Clave::~Clave(void)
{
    if (tamId)
	free(id);
}

void Clave::copiarObjeto(Clave *pNuevoObjeto)
{
	
    Objeto::copiarObjeto(pNuevoObjeto);
    
    pNuevoObjeto->keyType = this->keyType;
    pNuevoObjeto->bkeyType = this->bkeyType;
	
    pNuevoObjeto->derive= this->derive;
    pNuevoObjeto->bderive = this->bderive;
	
    pNuevoObjeto->local= this->local;
    pNuevoObjeto->blocal = this->blocal;

    if(this->tamId)
	{
	    pNuevoObjeto->id = (unsigned char *) malloc (tamId);
	    memcpy(pNuevoObjeto->id,this->id,tamId);
	}
    else
	pNuevoObjeto->id = NULL;
    pNuevoObjeto->tamId = this->tamId;

    if(this->bstartDate)
	memcpy((CK_BYTE *)&(pNuevoObjeto->startDate),(CK_BYTE *)&(this->startDate),sizeof(CK_DATE));
    pNuevoObjeto->bstartDate = this->bstartDate;

    if(this->bendDate)
	memcpy((CK_BYTE *)&(pNuevoObjeto->endDate),(CK_BYTE *)&(this->endDate),sizeof(CK_DATE));
    pNuevoObjeto->bendDate = this->bendDate;
}

CK_RV Clave::modificarObjeto(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount)
{
    unsigned int contador;
    CK_RV ck_rv;

    contador = 0;
    while (contador < ulCount)
	{
	    switch(pTemplate[contador].type)
		{
		case CKA_KEY_TYPE:
		    if (bmodifiable && modifiable)
			{
			    keyType = *((CK_KEY_TYPE *)pTemplate[contador].pValue);
			    bkeyType = TRUE;
			}
		    else
			return CKR_ATTRIBUTE_READ_ONLY;
		    break;
		case CKA_ID:
		    if (bmodifiable && modifiable)
			{
			    if (pTemplate[contador].ulValueLen != tamId)
				{
				    free(id);
				    tamId = pTemplate[contador].ulValueLen;
				    id = (CK_BYTE *) malloc(tamId);
				}
			    memcpy(id,pTemplate[contador].pValue,tamId);
			}
		    else
			return CKR_ATTRIBUTE_READ_ONLY;
		    break;
		case CKA_START_DATE:
		    if (bmodifiable && modifiable)
			{
			    memcpy((CK_BYTE *)&startDate,(CK_BYTE *)pTemplate[contador].pValue,pTemplate[contador].ulValueLen);
			    bstartDate = TRUE;
			}
		    else
			return CKR_ATTRIBUTE_READ_ONLY;
		    break;
		case CKA_END_DATE:
		    if (bmodifiable && modifiable)
			{
			    memcpy((CK_BYTE *)&endDate,pTemplate[contador].pValue,pTemplate[contador].ulValueLen);
			    bendDate = TRUE;
			}
		    else
			return CKR_ATTRIBUTE_READ_ONLY;
		    break;
		case CKA_DERIVE:
		    if (bmodifiable && modifiable)
			{
			    derive = *((CK_BBOOL *)pTemplate[contador].pValue);
			    bderive = TRUE;
			}
		    else
			return CKR_ATTRIBUTE_READ_ONLY;
		    break;
		case CKA_LOCAL:
		    if (bmodifiable && modifiable)
			{
			    local = *((CK_BBOOL *)pTemplate[contador].pValue);
			    blocal = TRUE;
			}
		    else
			return CKR_ATTRIBUTE_READ_ONLY;
		    break;
		default:
		    ck_rv = Objeto::modificarObjeto(&pTemplate[contador],1);
		    if ( ck_rv != CKR_OK)
			return ck_rv;
		}
	    contador++;
	}
    return CKR_OK;
}

void Clave::volcarPlantilla(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulMaxCount, CK_ULONG &ulCount)
{
    unsigned long contador;

    Objeto::volcarPlantilla(pTemplate,ulMaxCount,contador);
    if (contador < ulMaxCount)
	{
	    pTemplate[contador].type = CKA_KEY_TYPE;
	    if (bkeyType)
		{
		    pTemplate[contador].ulValueLen = sizeof(CK_KEY_TYPE);
		    pTemplate[contador].pValue = (CK_KEY_TYPE *) malloc(pTemplate[contador].ulValueLen);
		    *((CK_KEY_TYPE *)(pTemplate[contador].pValue)) = keyType;
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
	    pTemplate[contador].type = CKA_ID;
	    if (tamId)
		{
		    pTemplate[contador].ulValueLen = tamId;
		    pTemplate[contador].pValue = (CK_BYTE *) malloc(tamId);
		    memcpy(pTemplate[contador].pValue,id,tamId);
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
	    pTemplate[contador].type = CKA_START_DATE;
	    if (bstartDate)
		{
		    pTemplate[contador].ulValueLen = sizeof(CK_DATE);
		    pTemplate[contador].pValue = (CK_DATE *) malloc(pTemplate[contador].ulValueLen);
		    memcpy(pTemplate[contador].pValue,(CK_BYTE *)&startDate,pTemplate[contador].ulValueLen);
		}
	    else
		{
		    pTemplate[contador].pValue = NULL;
		    // TODO: MODIFICAR ESTO!!!!!!!11
		    pTemplate[contador].ulValueLen = (CK_LONG) -1;
		}
	    contador++;
	}

    if (contador < ulMaxCount)
	{
	    pTemplate[contador].type = CKA_END_DATE;
	    if (bendDate)
		{
		    pTemplate[contador].ulValueLen = sizeof(CK_DATE);
		    pTemplate[contador].pValue = (CK_DATE *) malloc(pTemplate[contador].ulValueLen);
		    memcpy(pTemplate[contador].pValue,(CK_BYTE *)&endDate,pTemplate[contador].ulValueLen);
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
	    pTemplate[contador].type = CKA_DERIVE;
	    if (bderive)
		{
		    pTemplate[contador].ulValueLen = sizeof(CK_BBOOL);
		    pTemplate[contador].pValue = (CK_BBOOL *) malloc(pTemplate[contador].ulValueLen);
		    *((CK_BBOOL *)(pTemplate[contador].pValue)) = derive;
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
	    pTemplate[contador].type = CKA_LOCAL;
	    if (blocal)
		{
		    pTemplate[contador].ulValueLen = sizeof(CK_BBOOL);
		    pTemplate[contador].pValue = (CK_BBOOL *) malloc(pTemplate[contador].ulValueLen);
		    *((CK_BBOOL *)(pTemplate[contador].pValue)) = local;
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

unsigned int Clave::numeroAtributos(void)
{
    return Objeto::numeroAtributos() + 6;
}

