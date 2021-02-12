
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

