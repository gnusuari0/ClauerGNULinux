
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

#include "clvprvda.h"
#include <string.h>		// memcpy
#include <stdlib.h>		// free
#include <stdio.h>

ClavePrivada::ClavePrivada(void):Clave()
{
    tamSubject = 0;
    bsensitive = FALSE;
    bdecrypt   = FALSE;
    bsign      = FALSE;
    bsignRecover = FALSE;
    bunwrap      = FALSE;
    bextractable = FALSE;
    balwaysSensitive  = FALSE;
    bneverExtractable = FALSE;
}

ClavePrivada::ClavePrivada(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount):Clave(pTemplate,ulCount)
{
    unsigned int contador;
	
    tamSubject = 0;
    bsensitive = FALSE;
    bdecrypt   = FALSE;
    bsign      = FALSE;
    bsignRecover = FALSE;
    bunwrap      = FALSE;
    bextractable = FALSE;
    balwaysSensitive  = FALSE;
    bneverExtractable = FALSE;

	
    contador = 0;
    while (contador < ulCount)
	{
	    switch(pTemplate[contador].type)
		{
		case CKA_SUBJECT:
		    tamSubject = pTemplate[contador].ulValueLen;
		    subject = (CK_BYTE *) malloc(tamSubject);
		    memcpy(subject,pTemplate[contador].pValue,tamSubject);
		    break;
		case CKA_SENSITIVE:
		    sensitive = *((CK_BBOOL *)pTemplate[contador].pValue);
		    bsensitive = TRUE;
		    break;
		case CKA_DECRYPT:
		    decrypt = *((CK_BBOOL *)pTemplate[contador].pValue);
		    bdecrypt = TRUE;
		    break;
		case CKA_SIGN:
		    sign = *((CK_BBOOL *)pTemplate[contador].pValue);
		    bsign = TRUE;
		    break;
		case CKA_SIGN_RECOVER:
		    signRecover = *((CK_BBOOL *)pTemplate[contador].pValue);
		    bsignRecover = TRUE;
		    break;
		case CKA_UNWRAP:
		    unwrap = *((CK_BBOOL *)pTemplate[contador].pValue);
		    bunwrap = TRUE;
		    break;
		case CKA_EXTRACTABLE:
		    extractable = *((CK_BBOOL *)pTemplate[contador].pValue);
		    bextractable = TRUE;
		    break;
		case CKA_ALWAYS_SENSITIVE:
		    alwaysSensitive = *((CK_BBOOL *)pTemplate[contador].pValue);
		    balwaysSensitive = TRUE;
		    break;
		case CKA_NEVER_EXTRACTABLE:
		    neverExtractable = *((CK_BBOOL *)pTemplate[contador].pValue);
		    bneverExtractable = TRUE;
		    break;
		}
	    contador++;
	}
}

ClavePrivada::~ClavePrivada(void)
{
    if (tamSubject)
	free(subject);
}


void ClavePrivada::copiarObjeto(ClavePrivada *pNuevoObjeto)
{
	
    Clave::copiarObjeto(pNuevoObjeto);

    pNuevoObjeto->sensitive = this->sensitive;
    pNuevoObjeto->bsensitive = this->bsensitive;
	
    pNuevoObjeto->decrypt= this->decrypt;
    pNuevoObjeto->bdecrypt = this->bdecrypt;
	
    pNuevoObjeto->sign= this->sign;
    pNuevoObjeto->bsign = this->bsign;

    pNuevoObjeto->signRecover = this->signRecover;
    pNuevoObjeto->bsignRecover = this->bsignRecover;

    pNuevoObjeto->unwrap = this->unwrap;
    pNuevoObjeto->bunwrap = this->bunwrap;

    pNuevoObjeto->extractable = this->extractable;
    pNuevoObjeto->bextractable = this->bextractable;

    pNuevoObjeto->alwaysSensitive = this->alwaysSensitive;
    pNuevoObjeto->balwaysSensitive = this->balwaysSensitive;

    pNuevoObjeto->neverExtractable = this->neverExtractable;
    pNuevoObjeto->bneverExtractable = this->bneverExtractable;

    if(this->tamSubject)
	{
	    pNuevoObjeto->subject = (unsigned char *) malloc (tamSubject);
	    memcpy(pNuevoObjeto->subject,this->subject,tamSubject);
	}
    else
	pNuevoObjeto->subject = NULL;
    pNuevoObjeto->tamSubject = this->tamSubject;
}

CK_RV ClavePrivada::modificarObjeto(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount)
{
    unsigned int contador;
    CK_RV ck_rv;

    contador = 0;
    while (contador < ulCount)
	{
	    switch(pTemplate[contador].type)
		{
		case CKA_SUBJECT:
		    if (bmodifiable && modifiable)
			{
			    if (pTemplate[contador].ulValueLen != tamSubject)
				{
				    free(subject);
				    tamSubject = pTemplate[contador].ulValueLen;
				    subject = (CK_BYTE *) malloc(tamSubject);
				}
			    memcpy(subject,pTemplate[contador].pValue,tamSubject);
			}
		    else
			return CKR_ATTRIBUTE_READ_ONLY;
		    break;
		case CKA_SENSITIVE:
		    if (bmodifiable && modifiable)
			{
			    sensitive = *((CK_BBOOL *)pTemplate[contador].pValue);
			    bsensitive = TRUE;
			}
		    else
			return CKR_ATTRIBUTE_READ_ONLY;
		    break;
		case CKA_DECRYPT:
		    if (bmodifiable && modifiable)
			{
			    decrypt = *((CK_BBOOL *)pTemplate[contador].pValue);
			    bdecrypt = TRUE;
			}
		    else
			return CKR_ATTRIBUTE_READ_ONLY;
		    break;
		case CKA_SIGN:
		    if (bmodifiable && modifiable)
			{
			    sign = *((CK_BBOOL *)pTemplate[contador].pValue);
			    bsign = TRUE;
			}
		    else
			return CKR_ATTRIBUTE_READ_ONLY;
		    break;
		case CKA_SIGN_RECOVER:
		    if (bmodifiable && modifiable)
			{
			    signRecover = *((CK_BBOOL *)pTemplate[contador].pValue);
			    bsignRecover = TRUE;
			}
		    else
			return CKR_ATTRIBUTE_READ_ONLY;
		    break;
		case CKA_UNWRAP:
		    if (bmodifiable && modifiable)
			{
			    unwrap = *((CK_BBOOL *)pTemplate[contador].pValue);
			    bunwrap = TRUE;
			}
		    else
			return CKR_ATTRIBUTE_READ_ONLY;
		    break;
		case CKA_EXTRACTABLE:
		    if (bmodifiable && modifiable)
			{
			    extractable = *((CK_BBOOL *)pTemplate[contador].pValue);
			    bextractable = TRUE;
			}
		    else
			return CKR_ATTRIBUTE_READ_ONLY;
		    break;
		case CKA_ALWAYS_SENSITIVE:
		    if (bmodifiable && modifiable)
			{
			    alwaysSensitive = *((CK_BBOOL *)pTemplate[contador].pValue);
			    balwaysSensitive = TRUE;
			}
		    else
			return CKR_ATTRIBUTE_READ_ONLY;
		    break;
		case CKA_NEVER_EXTRACTABLE:
		    if (bmodifiable && modifiable)
			{
			    neverExtractable = *((CK_BBOOL *)pTemplate[contador].pValue);
			    bneverExtractable = TRUE;
			}
		    else
			return CKR_ATTRIBUTE_READ_ONLY;
		    break;
		default:			 
		    ck_rv = Clave::modificarObjeto(&pTemplate[contador],1);
		    if ( ck_rv != CKR_OK)
			return ck_rv;
		}
	    contador++;
	}
    return CKR_OK;
}

void ClavePrivada::volcarPlantilla(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulMaxCount, CK_ULONG &ulCount)
{
    unsigned long contador;

    Clave::volcarPlantilla(pTemplate,ulMaxCount,contador);
    if (contador < ulMaxCount)
	{
	    pTemplate[contador].type = CKA_SUBJECT;
	    if (tamSubject)
		{
		    pTemplate[contador].ulValueLen = tamSubject;
		    pTemplate[contador].pValue = (CK_BYTE *) malloc(tamSubject);
		    memcpy(pTemplate[contador].pValue,subject,tamSubject);
		}
	    else
		{
		    pTemplate[contador].pValue = NULL;
		    pTemplate[contador].ulValueLen = (CK_ULONG) -1;
		}
	    contador++;
	}
	
    if (contador < ulMaxCount)
	{
	    pTemplate[contador].type = CKA_SENSITIVE;
	    if (bsensitive)
		{
		    pTemplate[contador].ulValueLen = sizeof(CK_BBOOL);
		    pTemplate[contador].pValue = (CK_BBOOL *) malloc(pTemplate[contador].ulValueLen);
		    *((CK_BBOOL *)(pTemplate[contador].pValue)) = sensitive;
		}
	    else
		{
		    pTemplate[contador].pValue = NULL;
		    pTemplate[contador].ulValueLen = (CK_ULONG) -1;
		}
	    contador++;
	}

    if (contador < ulMaxCount)
	{
	    pTemplate[contador].type = CKA_DECRYPT;
	    if (bdecrypt)
		{
		    pTemplate[contador].ulValueLen = sizeof(CK_BBOOL);
		    pTemplate[contador].pValue = (CK_BBOOL *) malloc(pTemplate[contador].ulValueLen);
		    *((CK_BBOOL *)(pTemplate[contador].pValue)) = decrypt;
		}
	    else
		{
		    pTemplate[contador].pValue = NULL;
		    pTemplate[contador].ulValueLen = (CK_ULONG) -1;
		}
	    contador++;
	}

    if (contador < ulMaxCount)
	{
	    pTemplate[contador].type = CKA_SIGN;
	    if (bsign)
		{
		    pTemplate[contador].ulValueLen = sizeof(CK_BBOOL);
		    pTemplate[contador].pValue = (CK_BBOOL *) malloc(pTemplate[contador].ulValueLen);
		    *((CK_BBOOL *)(pTemplate[contador].pValue)) = sign;
		}
	    else
		{
		    pTemplate[contador].pValue = NULL;
		    pTemplate[contador].ulValueLen = (CK_ULONG) -1;
		}
	    contador++;
	}

    if (contador < ulMaxCount)
	{
	    pTemplate[contador].type = CKA_SIGN_RECOVER;
	    if (bsignRecover)
		{
		    pTemplate[contador].ulValueLen = sizeof(CK_BBOOL);
		    pTemplate[contador].pValue = (CK_BBOOL *) malloc(pTemplate[contador].ulValueLen);
		    *((CK_BBOOL *)(pTemplate[contador].pValue)) = signRecover;
		}
	    else
		{
		    pTemplate[contador].pValue = NULL;
		    pTemplate[contador].ulValueLen = (CK_ULONG) -1;
		}
	    contador++;
	}

    if (contador < ulMaxCount)
	{
	    pTemplate[contador].type = CKA_UNWRAP;
	    if (bunwrap)
		{
		    pTemplate[contador].ulValueLen = sizeof(CK_BBOOL);
		    pTemplate[contador].pValue = (CK_BBOOL *) malloc(pTemplate[contador].ulValueLen);
		    *((CK_BBOOL *)(pTemplate[contador].pValue)) = unwrap;
		}
	    else
		{
		    pTemplate[contador].pValue = NULL;
		    pTemplate[contador].ulValueLen = (CK_ULONG) -1;
		}
	    contador++;
	}

    if (contador < ulMaxCount)
	{
	    pTemplate[contador].type = CKA_EXTRACTABLE;
	    if (bextractable)
		{
		    pTemplate[contador].ulValueLen = sizeof(CK_BBOOL);
		    pTemplate[contador].pValue = (CK_BBOOL *) malloc(pTemplate[contador].ulValueLen);
		    *((CK_BBOOL *)(pTemplate[contador].pValue)) = extractable;
		}
	    else
		{
		    pTemplate[contador].pValue = NULL;
		    pTemplate[contador].ulValueLen = (CK_ULONG) -1;
		}
	    contador++;
	}

    if (contador < ulMaxCount)
	{
	    pTemplate[contador].type = CKA_ALWAYS_SENSITIVE;
	    if (balwaysSensitive)
		{
		    pTemplate[contador].ulValueLen = sizeof(CK_BBOOL);
		    pTemplate[contador].pValue = (CK_BBOOL *) malloc(pTemplate[contador].ulValueLen);
		    *((CK_BBOOL *)(pTemplate[contador].pValue)) = alwaysSensitive;
		}
	    else
		{
		    pTemplate[contador].pValue = NULL;
		    pTemplate[contador].ulValueLen = (CK_ULONG) -1;
		}
	    contador++;
	}

    if (contador < ulMaxCount)
	{
	    pTemplate[contador].type = CKA_NEVER_EXTRACTABLE;
	    if (bneverExtractable)
		{
		    pTemplate[contador].ulValueLen = sizeof(CK_BBOOL);
		    pTemplate[contador].pValue = (CK_BBOOL *) malloc(pTemplate[contador].ulValueLen);
		    *((CK_BBOOL *)(pTemplate[contador].pValue)) = neverExtractable;
		}
	    else
		{
		    pTemplate[contador].pValue = NULL;
		    pTemplate[contador].ulValueLen = (CK_ULONG) -1;
		}
	    contador++;
	}

    ulCount = contador;
}

unsigned int ClavePrivada::numeroAtributos(void)
{
    return Clave::numeroAtributos() + 9;
}


