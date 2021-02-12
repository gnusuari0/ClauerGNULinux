
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

#include <stdlib.h>  // malloc
#include "certif.h"
#include <string.h>	// memcpy


Certificado::Certificado(void):Objeto()
{
	bcertificateType = FALSE;
}

Certificado::Certificado(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount):Objeto(pTemplate,ulCount)
{
    unsigned int contador;
    
    bcertificateType = FALSE;
    
    contador = 0;
    while (contador < ulCount)
	{
	    switch(pTemplate[contador].type)
		{
		case CKA_CERTIFICATE_TYPE:
		    certificateType = *((CK_CERTIFICATE_TYPE *)pTemplate[contador].pValue);
		    bcertificateType = TRUE;
		    break;
		}
	    contador++;
	}
}

Certificado::~Certificado(void)
{
    // El destructor no hace nada. No tiene que liberar memoria.
}

void Certificado::copiarObjeto(Certificado *pNuevoObjeto)
{
    Objeto::copiarObjeto(pNuevoObjeto);
    pNuevoObjeto->certificateType = this->certificateType;
    pNuevoObjeto->bcertificateType = this->bcertificateType;
}

CK_RV Certificado::modificarObjeto(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount)
{
    unsigned int contador;
    CK_RV ck_rv;
    
    contador = 0;
    while (contador < ulCount)
	{
	    switch(pTemplate[contador].type)
		{
		case CKA_CERTIFICATE_TYPE:
		    if (bmodifiable && modifiable)
			{
			    certificateType = *((CK_CERTIFICATE_TYPE *)pTemplate[contador].pValue);
			    bcertificateType = TRUE;
			}
		    else
			return CKR_ATTRIBUTE_READ_ONLY;
		    break;
		default:
		    ck_rv = Objeto::modificarObjeto(&pTemplate[contador],1);
		    if (ck_rv != CKR_OK)
			return ck_rv;
		}
	    contador++;
	}
    return CKR_OK;
}


void Certificado::volcarPlantilla(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulMaxCount, CK_ULONG &ulCount)
{
    unsigned long contador;
    
    Objeto::volcarPlantilla(pTemplate,ulMaxCount,contador);
    
    if (contador < ulMaxCount)
	{
	    pTemplate[contador].type = CKA_CERTIFICATE_TYPE;
	    if (bcertificateType)
		{
		    pTemplate[contador].ulValueLen = sizeof(certificateType);
		    pTemplate[contador].pValue = (CK_OBJECT_CLASS *) malloc(pTemplate[contador].ulValueLen);
		    *((CK_CERTIFICATE_TYPE *)(pTemplate[contador].pValue)) = certificateType;
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

unsigned int Certificado::numeroAtributos(void)
{
	return  Objeto::numeroAtributos() + 1;
}

