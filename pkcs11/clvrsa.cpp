
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

//#include "general.h"	// buscarEnPlantilla, malloc
#include "clvrsa.h"
#include <string.h>		// memcpy
#include <stdlib.h>		// free

ClaveRSA::ClaveRSA(void):ClavePrivada()
{
    tamModulus = 0;
    tamPublicExponent  = 0;
    tamPrivateExponent = 0;
    tamPrime1 = 0;
    tamPrime2 = 0;
    tamExponent1   = 0;
    tamExponent2   = 0;
    tamCoefficient = 0;
    tamClave       = 0;
}

ClaveRSA::ClaveRSA(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount, unsigned char * keyId ):ClavePrivada(pTemplate,ulCount)
{
    unsigned int contador;

    tamModulus = 0;
    tamPublicExponent  = 0;
    tamPrivateExponent = 0;
    tamPrime1 = 0;
    tamPrime2 = 0;
    tamExponent1   = 0;
    tamExponent2   = 0;
    tamCoefficient = 0;
    tamClave       = 0;

	
    contador = 0;
    while (contador < ulCount)
	{
	    switch(pTemplate[contador].type)
		{
		case CKA_MODULUS:
		    tamModulus = pTemplate[contador].ulValueLen;
		    modulus = (CK_BYTE *) malloc(tamModulus);
		    memcpy(modulus,pTemplate[contador].pValue,tamModulus);
		    break;
		case CKA_PUBLIC_EXPONENT:
		    tamPublicExponent = pTemplate[contador].ulValueLen;
		    publicExponent = (CK_BYTE *) malloc(tamPublicExponent);
		    memcpy(publicExponent,pTemplate[contador].pValue,tamPublicExponent);
		    break;
		case CKA_PRIVATE_EXPONENT:
		    tamPrivateExponent = pTemplate[contador].ulValueLen;
		    privateExponent = (CK_BYTE *) malloc(tamPrivateExponent);
		    memcpy(privateExponent,pTemplate[contador].pValue,tamPrivateExponent);
		    break;
		case CKA_PRIME_1:
		    tamPrime1 = pTemplate[contador].ulValueLen;
		    prime1 = (CK_BYTE *) malloc(tamPrime1);
		    memcpy(prime1,pTemplate[contador].pValue,tamPrime1);
		    break;
		case CKA_PRIME_2:
		    tamPrime2 = pTemplate[contador].ulValueLen;
		    prime2 = (CK_BYTE *) malloc(tamPrime2);
		    memcpy(prime2,pTemplate[contador].pValue,tamPrime2);
		    break;
		case CKA_EXPONENT_1:
		    tamExponent1 = pTemplate[contador].ulValueLen;
		    exponent1 = (CK_BYTE *) malloc(tamExponent1);
		    memcpy(exponent1,pTemplate[contador].pValue,tamExponent1);
		    break;
		case CKA_EXPONENT_2:
		    tamExponent2 = pTemplate[contador].ulValueLen;
		    exponent2 = (CK_BYTE *) malloc(tamExponent2);
		    memcpy(exponent2,pTemplate[contador].pValue,tamExponent2);
		    break;
		case CKA_COEFFICIENT:
		    tamCoefficient = pTemplate[contador].ulValueLen;
		    coefficient = (CK_BYTE *) malloc(tamCoefficient);
		    memcpy(coefficient,pTemplate[contador].pValue,tamCoefficient);
		    break;
		case CKA_VALUE:
		    tamClave = pTemplate[contador].ulValueLen;
		    clave = *((RSA **)pTemplate[contador].pValue);
		    break;
		}
	    contador++;
	}
    memcpy( _keyId, keyId, CERT_ID_LEN );
}

ClaveRSA::~ClaveRSA(void)
{
    if (tamModulus)
	free(modulus);
    if (tamPublicExponent)
	free(publicExponent);
    if (tamPrivateExponent)
	free(privateExponent);
    if (tamPrime1)
	free(prime1);
    if (tamPrime2)
	free(prime2);
    if (tamExponent1)
	free(exponent1);
    if (tamExponent2)
	free(exponent2);
    if (tamCoefficient)
	free(coefficient);
    if (tamClave)
	RSA_free(clave);
}

void ClaveRSA::copiarObjeto(ClaveRSA *pNuevoObjeto)
{
	
    ClavePrivada::copiarObjeto(pNuevoObjeto);

    if(this->tamModulus)
	{
	    pNuevoObjeto->modulus = (CK_BYTE *) malloc (tamModulus);
	    memcpy(pNuevoObjeto->modulus,this->modulus,tamModulus);
	}
    else
	pNuevoObjeto->modulus = NULL;
    pNuevoObjeto->tamModulus = this->tamModulus;

    if(this->tamPublicExponent)
	{
	    pNuevoObjeto->publicExponent = (CK_BYTE *) malloc (tamPublicExponent);
	    memcpy(pNuevoObjeto->publicExponent,this->publicExponent,tamPublicExponent);
	}
    else
	pNuevoObjeto->publicExponent = NULL;
    pNuevoObjeto->tamPublicExponent = this->tamPublicExponent;

    if(this->tamPrivateExponent)
	{
	    pNuevoObjeto->privateExponent = (CK_BYTE *) malloc (tamPrivateExponent);
	    memcpy(pNuevoObjeto->privateExponent,this->privateExponent,tamPrivateExponent);
	}
    else
	pNuevoObjeto->privateExponent = NULL;
    pNuevoObjeto->tamPrivateExponent = this->tamPrivateExponent;

    if(this->tamPrime1)
	{
	    pNuevoObjeto->prime1 = (CK_BYTE *) malloc (tamPrime1);
	    memcpy(pNuevoObjeto->prime1,this->prime1,tamPrime1);
	}
    else
	pNuevoObjeto->prime1 = NULL;
    pNuevoObjeto->tamPrime1 = this->tamPrime1;

    if(this->tamPrime2)
	{
	    pNuevoObjeto->prime2 = (CK_BYTE *) malloc (tamPrime2);
	    memcpy(pNuevoObjeto->prime2,this->prime2,tamPrime2);
	}
    else
	pNuevoObjeto->prime2 = NULL;
    pNuevoObjeto->tamPrime2 = this->tamPrime2;

    if(this->tamExponent1)
	{
	    pNuevoObjeto->exponent1 = (CK_BYTE *) malloc (tamExponent1);
	    memcpy(pNuevoObjeto->exponent1,this->exponent1,tamExponent1);
	}
    else
	pNuevoObjeto->exponent1 = NULL;
    pNuevoObjeto->tamExponent1 = this->tamExponent1;

    if(this->tamExponent2)
	{
	    pNuevoObjeto->exponent2 = (CK_BYTE *) malloc (tamExponent2);
	    memcpy(pNuevoObjeto->exponent2,this->exponent2,tamExponent2);
	}
    else
	pNuevoObjeto->exponent2 = NULL;
    pNuevoObjeto->tamExponent2 = this->tamExponent2;

    if(this->tamCoefficient)
	{
	    pNuevoObjeto->coefficient = (CK_BYTE *) malloc (tamCoefficient);
	    memcpy(pNuevoObjeto->coefficient,this->coefficient,tamCoefficient);
	}
    else
	pNuevoObjeto->coefficient = NULL;
    pNuevoObjeto->tamCoefficient = this->tamCoefficient;

    if(this->tamClave)
	{
	    // CUIDADO: en vez de hacer un malloc y despues un memcpy habria que hacer un 
	    //	    RSA_new y despues completar los campos de la misma forma que se hizo 
	    //	    con la estructura fuente
	    pNuevoObjeto->clave = (RSA *) malloc (tamClave);
	    memcpy(pNuevoObjeto->clave,this->clave,tamClave);
	}
    else
	pNuevoObjeto->clave = NULL;
    pNuevoObjeto->tamClave = this->tamClave;

}

CK_RV ClaveRSA::modificarObjeto(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount)
{
    unsigned int contador;
    CK_RV ck_rv;
    
    contador = 0;
    while (contador < ulCount)
	{
	    switch(pTemplate[contador].type)
		{
		case CKA_MODULUS:
		    if (bmodifiable && modifiable)
			{
			    if (pTemplate[contador].ulValueLen != tamModulus)
				{
				    free(modulus);
				    tamModulus = pTemplate[contador].ulValueLen;
				    modulus = (CK_BYTE *) malloc(tamModulus);
				}
			    memcpy(modulus,pTemplate[contador].pValue,tamModulus);
			}
		    else{
			
			return CKR_ATTRIBUTE_READ_ONLY;
		    }
		    break;
		case CKA_PUBLIC_EXPONENT:
		    if (bmodifiable && modifiable)
			{
			    if (pTemplate[contador].ulValueLen != tamPublicExponent)
				{
				    free(publicExponent);
				    tamPublicExponent = pTemplate[contador].ulValueLen;
				    publicExponent = (CK_BYTE *) malloc(tamPublicExponent);
				}
			    memcpy(publicExponent,pTemplate[contador].pValue,tamPublicExponent);
			}
		    else{
			
			return CKR_ATTRIBUTE_READ_ONLY;				     
		    }
		    break;
		case CKA_PRIVATE_EXPONENT:
		    if (bmodifiable && modifiable)
			{
			    if (pTemplate[contador].ulValueLen != tamPrivateExponent)
				{
				    free(privateExponent);
				    tamPrivateExponent = pTemplate[contador].ulValueLen;
				    privateExponent = (CK_BYTE *) malloc(tamPrivateExponent);
				}
			    memcpy(privateExponent,pTemplate[contador].pValue,tamPrivateExponent);
			}
		    else{
			
			return CKR_ATTRIBUTE_READ_ONLY;
		    }
		    break;
		case CKA_PRIME_1:
		    if (bmodifiable && modifiable)
			{
			    if (pTemplate[contador].ulValueLen != tamPrime1)
				{
				    free(prime1);
				    tamPrime1 = pTemplate[contador].ulValueLen;
				    prime1 = (CK_BYTE *) malloc(tamPrime1);
				}
			    memcpy(prime1,pTemplate[contador].pValue,tamPrime1);
			}
		    else{
			
			return CKR_ATTRIBUTE_READ_ONLY;
		    }
		    break;
		case CKA_PRIME_2:
		    if (bmodifiable && modifiable)
			{
			    if (pTemplate[contador].ulValueLen != tamPrime2)
				{
				    free(prime2);
				    tamPrime2 = pTemplate[contador].ulValueLen;
				    prime2 = (CK_BYTE *) malloc(tamPrime2);
				}
			    memcpy(prime2,pTemplate[contador].pValue,tamPrime2);
			}
		    else{
			
			return CKR_ATTRIBUTE_READ_ONLY;
		    }
		    break;
		case CKA_EXPONENT_1:
		    if (bmodifiable && modifiable)
			{
			    if (pTemplate[contador].ulValueLen != tamExponent1)
				{
				    free(exponent1);
				    tamExponent1 = pTemplate[contador].ulValueLen;
				    exponent1 = (CK_BYTE *) malloc(tamExponent1);
				}
			    memcpy(exponent1,pTemplate[contador].pValue,tamExponent1);
			}
		    else{
			
			return CKR_ATTRIBUTE_READ_ONLY;
		    }
		    break;
		case CKA_EXPONENT_2:
		    if (bmodifiable && modifiable)
			{
			    if (pTemplate[contador].ulValueLen != tamExponent2)
				{
				    free(exponent2);
				    tamExponent2 = pTemplate[contador].ulValueLen;
				    exponent2 = (CK_BYTE *) malloc(tamExponent2);
				}
			    memcpy(exponent2,pTemplate[contador].pValue,tamExponent2);
			}
		    else{
			
			return CKR_ATTRIBUTE_READ_ONLY;
		    }
		    break;
		case CKA_COEFFICIENT:
		    if (bmodifiable && modifiable)
			{
			    if (pTemplate[contador].ulValueLen != tamCoefficient)
				{
				    free(coefficient);
				    tamCoefficient = pTemplate[contador].ulValueLen;
				    coefficient = (CK_BYTE *) malloc(tamCoefficient);
				}
			    memcpy(coefficient,pTemplate[contador].pValue,tamCoefficient);
			}
		    else{
			
			return CKR_ATTRIBUTE_READ_ONLY;
		    }
		    break;
		case CKA_VALUE:
		    
		    return CKR_ATTRIBUTE_READ_ONLY;
		    break;
		default:
		    ck_rv = ClavePrivada::modificarObjeto(&pTemplate[contador],1);
		    if ( ck_rv != CKR_OK)
			return ck_rv;
		}
	    contador++;
	}
    return CKR_OK;
}

void ClaveRSA::volcarPlantilla(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulMaxCount, CK_ULONG &ulCount)
{
    unsigned long contador;

    ClavePrivada::volcarPlantilla(pTemplate,ulMaxCount,contador);
    if (contador < ulMaxCount)
	{
	    pTemplate[contador].type = CKA_MODULUS;
	    if (tamModulus)
		{
		    pTemplate[contador].ulValueLen = tamModulus;
		    pTemplate[contador].pValue = (CK_BYTE *) malloc(tamModulus);
		    memcpy(pTemplate[contador].pValue,modulus,tamModulus);
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
	    pTemplate[contador].type = CKA_PUBLIC_EXPONENT;
	    if (tamPublicExponent)
		{
		    pTemplate[contador].ulValueLen = tamPublicExponent;
		    pTemplate[contador].pValue = (CK_BYTE *) malloc(tamPublicExponent);
		    memcpy(pTemplate[contador].pValue,publicExponent,tamPublicExponent);
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
	    pTemplate[contador].type = CKA_PRIVATE_EXPONENT;
	    if (tamPrivateExponent)
		{
		    pTemplate[contador].ulValueLen = tamPrivateExponent;
		    pTemplate[contador].pValue = (CK_BYTE *) malloc(tamPrivateExponent);
		    memcpy(pTemplate[contador].pValue,privateExponent,tamPrivateExponent);
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
	    pTemplate[contador].type = CKA_PRIME_1;
	    if (tamPrime1)
		{
		    pTemplate[contador].ulValueLen = tamPrime1;
		    pTemplate[contador].pValue = (CK_BYTE *) malloc(tamPrime1);
		    memcpy(pTemplate[contador].pValue,prime1,tamPrime1);
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
	    pTemplate[contador].type = CKA_PRIME_2;
	    if (tamPrime2)
		{
		    pTemplate[contador].ulValueLen = tamPrime2;
		    pTemplate[contador].pValue = (CK_BYTE *) malloc(tamPrime2);
		    memcpy(pTemplate[contador].pValue,prime2,tamPrime2);
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
	    pTemplate[contador].type = CKA_EXPONENT_1;
	    if (tamExponent1)
		{
		    pTemplate[contador].ulValueLen = tamExponent1;
		    pTemplate[contador].pValue = (CK_BYTE *) malloc(tamExponent1);
		    memcpy(pTemplate[contador].pValue,exponent1,tamExponent1);
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
	    pTemplate[contador].type = CKA_EXPONENT_2;
	    if (tamExponent2)
		{
		    pTemplate[contador].ulValueLen = tamExponent2;
		    pTemplate[contador].pValue = (CK_BYTE *) malloc(tamExponent2);
		    memcpy(pTemplate[contador].pValue,exponent2,tamExponent2);
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
	    pTemplate[contador].type = CKA_COEFFICIENT;
	    if (tamCoefficient)
		{
		    pTemplate[contador].ulValueLen = tamCoefficient;
		    pTemplate[contador].pValue = (CK_BYTE *) malloc(tamCoefficient);
		    memcpy(pTemplate[contador].pValue,coefficient,tamCoefficient);
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
	    if (tamClave)
		{
		    // NOTA: la clave nunca se copia. Se copia un puntero ya que al hacer
		    //       un destruirPlantilla se intenta hacer un free de una estructura
		    //       de tipo RSA. Lo correcto es hacer un RSA_free
		    pTemplate[contador].ulValueLen = sizeof(RSA **);
		    pTemplate[contador].pValue = (CK_BYTE *) malloc(pTemplate[contador].ulValueLen);
		    *((RSA **)pTemplate[contador].pValue) = clave;
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

unsigned int ClaveRSA::numeroAtributos(void)
{
    return ClavePrivada::numeroAtributos() + 9;
}

