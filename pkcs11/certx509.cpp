
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

#include "certx509.h"
#include <string.h>		// memcpy
#include <stdlib.h>		// free
#include <stdio.h>

CertificadoX509::CertificadoX509(void):Certificado()
{
    tamSubject= 0;
    tamId	  = 0;
    tamIssuer = 0;
    tamSerial_number = 0;
    tamValue= 0;
}

CertificadoX509::CertificadoX509(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount, unsigned char * certId ):Certificado(pTemplate,ulCount)
{
    unsigned int contador;
    
    tamSubject= 0;
    tamId	  = 0;
    tamIssuer = 0;
    tamSerial_number = 0;
    tamValue= 0;

	
    contador = 0;
    while (contador < ulCount)
	{
	    switch(pTemplate[contador].type)
		{
		case CKA_SUBJECT:
		    tamSubject = pTemplate[contador].ulValueLen;
		    subject = (CK_BYTE *)malloc(tamSubject);
		    memcpy(subject,pTemplate[contador].pValue,tamSubject);
		    break;
		case CKA_ID:
		    tamId = pTemplate[contador].ulValueLen;
		    id = (CK_BYTE *)malloc(tamId);
		    memcpy(id,pTemplate[contador].pValue,tamId);				
		    break;
		case CKA_ISSUER:
		    tamIssuer = pTemplate[contador].ulValueLen;
		    issuer = (CK_BYTE *)malloc(tamIssuer);
		    memcpy(issuer,pTemplate[contador].pValue,tamIssuer);
		    break;
		case CKA_SERIAL_NUMBER:
		    tamSerial_number = pTemplate[contador].ulValueLen;
		    serial_number = (CK_BYTE *)malloc(tamSerial_number);
		    memcpy(serial_number,pTemplate[contador].pValue,tamSerial_number);
		    break;
		case CKA_VALUE:
		    tamValue = pTemplate[contador].ulValueLen;
		    value = (CK_BYTE *)malloc(tamValue);
		    memcpy(value,pTemplate[contador].pValue,tamValue);
		    break;
		}
	    contador++;
	}
	memcpy( _certId, certId, CERT_ID_LEN );
}

CertificadoX509::~CertificadoX509(void)
{
    if (tamSubject)
	free(subject);
    if (tamId)
	free(id);
    if (tamIssuer)
	free(issuer);
    if (tamSerial_number)
	free(serial_number);
    if (tamValue)
	free(value);
}

void CertificadoX509::copiarObjeto(CertificadoX509 *pNuevoObjeto)
{
    Certificado::copiarObjeto(pNuevoObjeto);

    if(this->tamSubject)
	{
	    pNuevoObjeto->subject = (CK_BYTE *) malloc (tamSubject);
	    memcpy(pNuevoObjeto->subject,this->subject,tamSubject);
	}
    else
	pNuevoObjeto->subject = NULL;
    pNuevoObjeto->tamSubject = this->tamSubject;

    if(this->tamId)
	{
	    pNuevoObjeto->id = (CK_BYTE *) malloc (tamId);
	    memcpy(pNuevoObjeto->id,this->id,tamId);
	}
    else
	pNuevoObjeto->id = NULL;
    pNuevoObjeto->tamId = this->tamId;

    if(this->tamIssuer)
	{
	    pNuevoObjeto->issuer = (CK_BYTE *) malloc (tamIssuer);
	    memcpy(pNuevoObjeto->issuer,this->issuer,tamIssuer);
	}
    else
	pNuevoObjeto->issuer = NULL;
    pNuevoObjeto->tamIssuer = this->tamIssuer;

    if(this->tamSerial_number)
	{
	    pNuevoObjeto->serial_number = (CK_BYTE *) malloc (tamSerial_number);
	    memcpy(pNuevoObjeto->serial_number,this->serial_number,tamSerial_number);
	}
    else
	pNuevoObjeto->serial_number = NULL;
    pNuevoObjeto->tamSerial_number = this->tamSerial_number;

    if(this->tamValue)
	{
	    pNuevoObjeto->value = (CK_BYTE *) malloc (tamValue);
	    memcpy(pNuevoObjeto->value,this->value,tamValue);
	}
    else
	pNuevoObjeto->value = NULL;
    pNuevoObjeto->tamValue = this->tamValue;
}

CK_RV CertificadoX509::modificarObjeto(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount)
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
				    subject = (CK_BYTE_PTR) malloc(tamSubject);
				}
			    memcpy(subject,pTemplate[contador].pValue,tamSubject);
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
				    id = (CK_BYTE_PTR) malloc(tamId);
				}
			    memcpy(id,pTemplate[contador].pValue,tamId);
			}
		    else
			return CKR_ATTRIBUTE_READ_ONLY;
		    break;
		case CKA_ISSUER:
		    if (bmodifiable && modifiable)
			{
			    if (pTemplate[contador].ulValueLen != tamIssuer)
				{
				    free(issuer);
				    tamIssuer = pTemplate[contador].ulValueLen;
				    issuer = (CK_BYTE_PTR) malloc(tamIssuer);
				}
			    memcpy(issuer,pTemplate[contador].pValue,tamIssuer);
			}
		    else
			return CKR_ATTRIBUTE_READ_ONLY;
		    break;
		case CKA_SERIAL_NUMBER:
		    if (bmodifiable && modifiable)
			{
			    if (pTemplate[contador].ulValueLen != tamSerial_number)
				{
				    free(serial_number);
				    tamSerial_number = pTemplate[contador].ulValueLen;
				    serial_number = (CK_BYTE_PTR) malloc(tamSerial_number);
				}
			    memcpy(serial_number,pTemplate[contador].pValue,tamSerial_number);

			}
		    else
			return CKR_ATTRIBUTE_READ_ONLY;
		    break;
		case CKA_VALUE:
		    if (bmodifiable && modifiable)
			{
			    if (pTemplate[contador].ulValueLen != tamValue)
				{
				    free(value);
				    tamValue = pTemplate[contador].ulValueLen;
				    value = (CK_BYTE_PTR) malloc(tamValue);
				}
			    memcpy(value,pTemplate[contador].pValue,tamValue);
			}
		    else
			return CKR_ATTRIBUTE_READ_ONLY;
		    break;

		default:
		    ck_rv = Certificado::modificarObjeto(&pTemplate[contador],1);
		    if ( ck_rv != CKR_OK)
			return ck_rv;
		}
	    contador++;
	}
    return CKR_OK;
}


void CertificadoX509::volcarPlantilla(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulMaxCount, CK_ULONG &ulCount)
{
    unsigned long contador;

    Certificado::volcarPlantilla(pTemplate,ulMaxCount,contador);
    if (contador < ulMaxCount)
	{
	    pTemplate[contador].type = CKA_SUBJECT;
	    if (tamSubject)
		{
		    pTemplate[contador].ulValueLen = tamSubject;
		    pTemplate[contador].pValue = (CK_BYTE *) malloc(pTemplate[contador].ulValueLen);
		    memcpy(pTemplate[contador].pValue,subject,pTemplate[contador].ulValueLen);

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
		    pTemplate[contador].pValue = (CK_BYTE *) malloc(pTemplate[contador].ulValueLen);
		    memcpy(pTemplate[contador].pValue,id,pTemplate[contador].ulValueLen);

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
	    pTemplate[contador].type = CKA_ISSUER;
	    if (tamIssuer)
		{
		    pTemplate[contador].ulValueLen = tamIssuer;
		    pTemplate[contador].pValue = (CK_BYTE *) malloc(pTemplate[contador].ulValueLen);
		    memcpy(pTemplate[contador].pValue,issuer,pTemplate[contador].ulValueLen);

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
	    pTemplate[contador].type = CKA_SERIAL_NUMBER;
	    if (tamSerial_number)
		{
		    pTemplate[contador].ulValueLen = tamSerial_number;
		    pTemplate[contador].pValue = (CK_BYTE *) malloc(pTemplate[contador].ulValueLen);
		    memcpy(pTemplate[contador].pValue,serial_number,pTemplate[contador].ulValueLen);

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
	    pTemplate[contador].type = CKA_VALUE;
	    if (tamValue)
		{
		    pTemplate[contador].ulValueLen = tamValue;
		    pTemplate[contador].pValue = (CK_BYTE *) malloc(pTemplate[contador].ulValueLen);
		    memcpy(pTemplate[contador].pValue,value,pTemplate[contador].ulValueLen);

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

unsigned int CertificadoX509::numeroAtributos(void)
{
    return  Certificado::numeroAtributos() + 5;
}
