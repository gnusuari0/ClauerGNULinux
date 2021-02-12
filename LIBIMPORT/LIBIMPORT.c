
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

#include "LIBIMPORT.h"

#include <CRYPTOWrapper/CRYPTOWrap.h>

#include <openssl/bio.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

#include <sys/mman.h>

/* Indica si la libería ha sido o no inicializada
 */

static BOOL gbLibImportIni = FALSE;  



/*! \brief Crea un key container en el stick.
 *
 * Crea un key container en el stick.
 *
 * \param hClauer
 *        Handle abierto (sesión autenticada) contra el clauer
 *        destino.
 *
 * \param nombreKeyContainer
 *        El nombre del key container a crear.
 *
 * \param hSignature
 *        Bloque donde se encuentra almacenada la llave
 *        de tipo AT_SIGNATURE
 *
 * \param hExchange
 *        Bloque donde se encuentra almacenada la llave
 *        de tipo AT_KEYEXCHANGE
 *
 * \param idSignature
 *        El identificador de la llave de tipo AT_SIGNATURE.
 *
 * \param idExchange
 *        El identificador de la llave de tipo AT_KEYECHANGE
 *
 * \param existe
 *        ¿?¿?¿?¿?
 *
 * \retval 0
 *         Ok
 *
 * \retval 1
 *         Error
 */

int LIBIMPORT_KeyContainer_Insertar (USBCERTS_HANDLE *hClauer,
							         LPTSTR nombreKeyContainer,
									 long hSignature,
									 long hExchange,
									 BYTE idSignature[20],
									 BYTE idExchange[20],
									 BOOL existe)
{
	BYTE bloque[TAM_BLOQUE];
	long nBloque;
	int ret = 0;
	INFO_KEY_CONTAINER *kcs = NULL;
	unsigned int nKcs, i;
	BOOL insertarAqui = FALSE;

	if ( !gbLibImportIni ) {
		LIBRT_Ini();
		CRYPTO_Ini();
		gbLibImportIni = TRUE;
	}

	if ( LIBRT_LeerTipoBloqueCrypto(hClauer, BLOQUE_KEY_CONTAINERS, TRUE, bloque, &nBloque) != 0 ) {
		ret = 1;
		goto finLIBIMPORT_KeyContainer_Insertar;
	}

	while ( (nBloque != -1) && ( insertarAqui == FALSE ) ) {

		if ( BLOQUE_KeyContainer_Enumerar(bloque, NULL, &nKcs) != 0 ) {
			ret = 1;
			goto finLIBIMPORT_KeyContainer_Insertar;
		}

		kcs = ( INFO_KEY_CONTAINER *) malloc (sizeof(INFO_KEY_CONTAINER)*nKcs);

		if ( !kcs ) {
			ret = 1;
			goto finLIBIMPORT_KeyContainer_Insertar;
		}

		if ( BLOQUE_KeyContainer_Enumerar(bloque, kcs, &nKcs) != 0 ) {
			ret = 1;
			goto finLIBIMPORT_KeyContainer_Insertar;
		}


		if ( existe ) {

			/* Si el blob ya existía anteriormente lo que hacemos es buscar en qué
			 * container estaba y recalcularlo todo
			 */

			for ( i = 0 ; i < nKcs ; i++ ) {

				if ( hSignature != -1 ) {

					if ( memcmp(kcs[i].idSignature, idSignature, 20) == 0 ) {
						/* Lo hemos encontrado */

						if ( BLOQUE_KeyContainer_EstablecerSIGNATURE(bloque, kcs[i].nombreKeyContainer, hSignature, kcs[i].exportaSig) != 0 ) {
							ret = 1;
							goto finLIBIMPORT_KeyContainer_Insertar;
						}
						if ( BLOQUE_KeyContainer_Establecer_ID_Signature(bloque, kcs[i].nombreKeyContainer, idSignature) != 0 ) {
							ret = 1;
							goto finLIBIMPORT_KeyContainer_Insertar;
						}


						if ( LIBRT_EscribirBloqueCrypto(hClauer, nBloque, bloque) != 0 ) {
							ret = 1;
							goto finLIBIMPORT_KeyContainer_Insertar;
						}

					} /* memcmp */
				} /* hSignature */


				if ( hExchange != -1 ) {

					if ( memcmp(kcs[i].idExchange, idExchange, 20) == 0 ) {
						/* Lo hemos encontrado */

						if ( BLOQUE_KeyContainer_EstablecerEXCHANGE(bloque, kcs[i].nombreKeyContainer, hExchange, kcs[i].exportaEx) != 0 ) {
							ret = 1;
							goto finLIBIMPORT_KeyContainer_Insertar;
						}
						if ( BLOQUE_KeyContainer_Establecer_ID_Exchange(bloque, kcs[i].nombreKeyContainer, idExchange) != 0 ) {
							ret = 1;
							goto finLIBIMPORT_KeyContainer_Insertar;
						}


						if ( LIBRT_EscribirBloqueCrypto(hClauer, nBloque, bloque) != 0 ) {
							ret = 1;
							goto finLIBIMPORT_KeyContainer_Insertar;
						}

					} /* memcmp */
				} /* hExchange */

			} /* for */

		} else {

			/* El blob no estaba insertado previamente, por lo tanto buscamos un sitio para insertarlo
			 */

			switch ( BLOQUE_KeyContainer_Insertar(bloque, nombreKeyContainer) ) {
			case ERR_BLOQUE_SIN_ESPACIO:
				insertarAqui = FALSE;
				break;

			case ERR_BLOQUE_NO:
				insertarAqui = TRUE;
				break;

			default:
				ret = 1;
				goto finLIBIMPORT_KeyContainer_Insertar;
			}
		}

		CRYPTO_SecureZeroMemory(kcs, sizeof(INFO_KEY_CONTAINER)*nKcs);
		free(kcs);
		kcs = NULL;

		if ( insertarAqui )
			break;


		if ( LIBRT_LeerTipoBloqueCrypto(hClauer, BLOQUE_KEY_CONTAINERS, FALSE, bloque, &nBloque) != 0 ) {
			ret = 1;
			goto finLIBIMPORT_KeyContainer_Insertar;
		}

	}


	if ( !existe ) {
		if ( FALSE == insertarAqui ) {

			/* Creamos un nuevo bloque y lo insertamos
			 */

			CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
			BLOQUE_Set_Claro(bloque);
			BLOQUE_KeyContainer_Nuevo ( bloque );
			BLOQUE_KeyContainer_Insertar ( bloque, nombreKeyContainer);

			if ( hSignature != -1 ) {
				BLOQUE_KeyContainer_EstablecerSIGNATURE(bloque, nombreKeyContainer, hSignature, FALSE);
				BLOQUE_KeyContainer_Establecer_ID_Signature(bloque, nombreKeyContainer, idSignature);
			} else {
				BLOQUE_KeyContainer_EstablecerEXCHANGE(bloque, nombreKeyContainer, hExchange, FALSE);
				BLOQUE_KeyContainer_Establecer_ID_Exchange(bloque, nombreKeyContainer, idExchange);
			}

			if ( LIBRT_InsertarBloqueCrypto(hClauer, bloque, &nBloque) != 0 ) {
				ret = 1;
				goto finLIBIMPORT_KeyContainer_Insertar;
			}

		} else {

			BLOQUE_KeyContainer_Insertar ( bloque, nombreKeyContainer);

			if ( hSignature != -1 ) {
				BLOQUE_KeyContainer_EstablecerSIGNATURE(bloque, nombreKeyContainer, hSignature, FALSE);
				BLOQUE_KeyContainer_Establecer_ID_Signature(bloque, nombreKeyContainer, idSignature);
			} else {
				BLOQUE_KeyContainer_EstablecerEXCHANGE(bloque, nombreKeyContainer, hExchange, FALSE);
				BLOQUE_KeyContainer_Establecer_ID_Exchange(bloque, nombreKeyContainer, idExchange);
			}

			if ( LIBRT_EscribirBloqueCrypto(hClauer, nBloque, bloque) != 0 ) {
				ret = 1;
				goto finLIBIMPORT_KeyContainer_Insertar;
			}
								
		}

	}



finLIBIMPORT_KeyContainer_Insertar:

	CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);

	if ( kcs ) {
		CRYPTO_SecureZeroMemory(kcs, sizeof(INFO_KEY_CONTAINER)*nKcs);
		free(kcs);
		kcs = NULL;
	}

	return ret;
}


/*! \brief Comprueba la existencia de un certificado en el Clauer.
 *
 * Comprueba la existencia de un certificado en el Clauer.
 *
 * \param certPEM
 *        Certificado en formato PEM que vamos a buscar enel stick.
 *
 * \param tamCert
 *        El tamaño, en bytes, de certPEM.
 *
 * \param tipoCert
 *        El tipo del certificado que estamos buscando. Puede ser
 *        BLOQUE_CERT_PROPIO, BLOQUE_CERT_RAIZ y BLOQUE_CERT_INTERMEDIO.
 *
 * \param hClauer
 *        Handle al Clauer.
 *
 * \param posBloque
 *        Posición donde se encuentra el bloque con el certificado.
 *
 * \retval 0
 *         El certificado no ha sido insertado en el Clauer.
 *
 * \retval 1
 *         El certificado existe en el Clauer
 *
 * \retval 2
 *         Error
 */

int LIBIMPORT_ExisteCertificado (BYTE *certPEM, unsigned long tamCert, unsigned char tipoCert, USBCERTS_HANDLE *hClauer, long *posBloque )
{
	int ret = 0;
	BYTE bloque[TAM_BLOQUE], *certBloque;
	long nBloque;
	unsigned long tamCertBloque;

	if ( (tipoCert != BLOQUE_CERT_PROPIO) &&
		 (tipoCert != BLOQUE_CERT_RAIZ) &&
		 (tipoCert != BLOQUE_CERT_INTERMEDIO) )
	{
		ret = 2;
		goto finLIBIMPORT_ExisteCertificado;
	}

	if ( LIBRT_LeerTipoBloqueCrypto (hClauer, tipoCert, TRUE, bloque, &nBloque) != 0 ) {
		ret = 2;
		goto finLIBIMPORT_ExisteCertificado;
	}

	while ( (nBloque != -1) && (ret == 0 ) ) {

		switch ( tipoCert ) {

		case BLOQUE_CERT_PROPIO:
			certBloque = BLOQUE_CERTPROPIO_Get_Objeto(bloque);
			tamCertBloque = BLOQUE_CERTPROPIO_Get_Tam(bloque);
			break;

		case BLOQUE_CERT_INTERMEDIO:
			certBloque = BLOQUE_CERTINTERMEDIO_Get_Objeto(bloque);
			tamCertBloque = BLOQUE_CERTINTERMEDIO_Get_Tam(bloque);
			break;

		case BLOQUE_CERT_RAIZ:
			certBloque = BLOQUE_CERTRAIZ_Get_Objeto(bloque);
			tamCertBloque = BLOQUE_CERTRAIZ_Get_Tam(bloque);
			break;

		}

		switch ( CRYPTO_CERT_Cmp(certPEM, tamCert, certBloque, tamCertBloque) ) {
		case 0:
			ret = 1;
			break;
		case 2:
			ret = 2;
			goto finLIBIMPORT_ExisteCertificado;
		}


		if ( LIBRT_LeerTipoBloqueCrypto (hClauer, tipoCert, FALSE, bloque, &nBloque) != 0 ) {
			ret = 2;
			goto finLIBIMPORT_ExisteCertificado;
		}

	}

	if ( ret == 1 ) 
		*posBloque = nBloque;

finLIBIMPORT_ExisteCertificado:

	CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);

	return ret;

}


/*! \brief Busca una llave privada en el clauer.
 *
 * Busca una llave privada en el Clauer.
 *
 * \param llavePrivada
 *        Llave privada en formato PEM que buscamos.
 *
 * \param tamLlavePrivada
 *        Tamaño en bytes de llavePrivada
 *
 * \param hClauer
 *        Handle al clauer.
 *
 * \param pBloque
 *        Bloque donde se encuentra la llave privada
 *
 * \retval 0
 *         No existe la llave
 *
 * \retval 1
 *         La llave se encontró
 *
 * \retval 2
 *         Error
 */

int LIBIMPORT_BuscarLlavePrivada (BYTE *llavePrivada, DWORD tamLlavePrivada, USBCERTS_HANDLE *hClauer, long *pBloque, int mode)
{
	BYTE bloque[TAM_BLOQUE];
	long nBloque;
	int ret = 0;
	unsigned char id[20];

	/* Calculamos el identificador de la llave
	 */

	if ( CRYPTO_LLAVE_PEM_Id (llavePrivada, tamLlavePrivada, 1, NULL, id) != 0 ) {
		ret = 2;
		goto finLIBIMPORT_BuscarLlavePrivada;
	}

	/* Buscamos una llave privada con ese identificador
	 */

	if ( mode == PRIVKEY_DEFAULT_CIPHER || mode == PRIVKEY_NO_CIPHER ){

	    if ( LIBRT_LeerTipoBloqueCrypto(hClauer, BLOQUE_LLAVE_PRIVADA, TRUE, bloque, &nBloque) != 0 ) {
		ret = 2;
		goto finLIBIMPORT_BuscarLlavePrivada;
	    }
	    
	    while ( nBloque != -1 ) {
		
		if ( memcmp(id, BLOQUE_LLAVEPRIVADA_Get_Id(bloque),20) == 0 ) {
		    ret = 1;
		    *pBloque = nBloque;
		    break;
		}
		
		CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
		
		if ( LIBRT_LeerTipoBloqueCrypto(hClauer, BLOQUE_LLAVE_PRIVADA, FALSE, bloque, &nBloque) != 0 ) {
		    ret = 2;
		    goto finLIBIMPORT_BuscarLlavePrivada;
		}
	    }
	}
	else if ( mode ==  PRIVKEY_DOUBLE_CIPHER || mode == PRIVKEY_PASSWORD_CIPHER ){
	    if ( LIBRT_LeerTipoBloqueCrypto(hClauer, BLOQUE_CIPHER_PRIVKEY_PEM, TRUE, bloque, &nBloque) != 0 ) {
		ret = 2;
		goto finLIBIMPORT_BuscarLlavePrivada;
	    }
	    
	    while ( nBloque != -1 ) {
		
		if ( memcmp(id, BLOQUE_LLAVEPRIVADA_Get_Id(bloque),20) == 0 ) {
		    ret = 1;
		    *pBloque = nBloque;
		    break;
		}
		
		CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
		
		if ( LIBRT_LeerTipoBloqueCrypto(hClauer, BLOQUE_LLAVE_PRIVADA, FALSE, bloque, &nBloque) != 0 ) {
		    ret = 2;
		    goto finLIBIMPORT_BuscarLlavePrivada;
		}
	    }	    	    
	}



finLIBIMPORT_BuscarLlavePrivada:

	CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
	CRYPTO_SecureZeroMemory(id, 20);

	return ret;

}


/*! \brief Busca un blob en el clauer.
 *
 * Busca un blob en el Clauer.
 *
 * \param blob
 *        El PRIVATEKEYBLOB que buscamos
 *
 * \param tamBlob
 *        Tamaño en bytes de blob
 *
 * \param hClauer
 *        Handle al clauer.
 *
 * \param id
 *        Identificador del blob
 *
 * \param pBloque
 *        Bloque donde se encuentra el blob
 *
 * \retval 0
 *         No existe la llave
 *
 * \retval 1
 *         La llave se encontró
 *
 * \retval 2
 *         Error
 */

int LIBIMPORT_BuscarBlob (BYTE *blob, DWORD tamBlob, USBCERTS_HANDLE *hClauer, unsigned char id[20], long *pBloque, int mode )
{
	BYTE bloque[TAM_BLOQUE];
	long nBloque;
	int ret = 0;
	

	/* Buscamos una llave privada con ese identificador
	 */

	
	if ( mode == PRIVKEY_DEFAULT_CIPHER || mode == PRIVKEY_NO_CIPHER ){
	    
	    if ( LIBRT_LeerTipoBloqueCrypto(hClauer, BLOQUE_PRIVKEY_BLOB, TRUE, bloque, &nBloque) != 0 ) {
		ret = 2;
		goto finLIBIMPORT_BuscarBlob;
	    }
	    
	    while ( nBloque != -1 ) {
		
		if ( memcmp(id, BLOQUE_LLAVEPRIVADA_Get_Id(bloque),20) == 0 ) {
		    ret = 1;
		    *pBloque = nBloque;
		    break;
		}
		
		CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
		
		if ( LIBRT_LeerTipoBloqueCrypto(hClauer, BLOQUE_PRIVKEY_BLOB, FALSE, bloque, &nBloque) != 0 ) {
		    ret = 2;
			goto finLIBIMPORT_BuscarBlob;
		}
	    }
	}
	else if ( mode ==  PRIVKEY_DOUBLE_CIPHER || mode == PRIVKEY_PASSWORD_CIPHER ){
	    if ( LIBRT_LeerTipoBloqueCrypto(hClauer, BLOQUE_CIPHER_PRIVKEY_BLOB, TRUE, bloque, &nBloque) != 0 ) {
		ret = 2;
		goto finLIBIMPORT_BuscarBlob;
		}
	    
	    while ( nBloque != -1 ) {
		    
		if ( memcmp(id, BLOQUE_LLAVEPRIVADA_Get_Id(bloque),20) == 0 ) {
		    ret = 1;
		    *pBloque = nBloque;
		    break;
		}
		
		CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
		
		if ( LIBRT_LeerTipoBloqueCrypto(hClauer, BLOQUE_PRIVKEY_BLOB, FALSE, bloque, &nBloque) != 0 ) {
		    ret = 2;
		    goto finLIBIMPORT_BuscarBlob;
		}
	    }
	}
	

finLIBIMPORT_BuscarBlob:

	CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
	//CRYPTO_SecureZeroMemory(id, 20);

	return ret;

}


BOOL LIBIMPORT_ImportarPKCS12 (const char *fileName, const char *pwd, USBCERTS_HANDLE *hClauer, int mode, char *extra_pass ){

  return LIBIMPORT_ImportarPKCS12_internal (fileName, pwd, hClauer,  mode, extra_pass, 1);
}




/*! \brief Importa los objetos contenidos en un pkcs12 en el clauer.
 *
 * Importa los objetos contenidos en un pkcs#12 en el clauer.
 *
 * \param fileName
 *        El nombre de fichero del pkcs#12.
 *
 * \param pwd
 *        La contraseña que protege el clauer.
 *
 * \param hClauer
 *        Handle al clauer.
 * 
 * \param mode 
 *        Ciphering mode that must be applied to keys.
 * 
 * \param extra_pwd
 *        Extra password for ciphering modes that requiere it.
 * 
 * \retval TRUE
 *         Ok
 *
 * \retval FALSE
 *         Error
 *
 */

BOOL LIBIMPORT_ImportarPKCS12_internal (const char *fileName, const char *pwd, USBCERTS_HANDLE *hClauer, int mode, char *extra_pwd, BOOL importarCAs)
{

  	FILE *f;
	DWORD tam = 0;
	BYTE *pkcs12;
	BOOL vuelta;


	// printf("CALG_RSA_KEYX= %x\n", CALG_RSA_KEYX);
	// printf("CALG_RSA_SIGN=  %x\n", CALG_RSA_SIGN);

	/* Pasos:
	 *
	 *       1. Parsear el PKCS12
	 *		 2. Crear un PRIVATEKEYBLOB de CRYPTOApi
	 *       3. Importar el blob, el certificado y la llave privada en el stick
	 */


	/* Parseamos el PKCS12 
	 */

	f = fopen(fileName, "rb");
	if ( !f )
		return FALSE;

	fseek(f,0,SEEK_END);
	tam = ftell(f);
	fseek(f,0,SEEK_SET);

	pkcs12 = (BYTE *) malloc (tam);

	if ( !pkcs12 ) {
		fclose(f);
		return FALSE;
	}

	fread(pkcs12,1,tam,f);
	fclose(f);

	vuelta = LIBIMPORT_ImportarPKCS12deBuffer (pkcs12, tam,pwd, hClauer,  mode, extra_pwd, importarCAs);
	free(pkcs12);
	return vuelta;
}

	
/*! \brief Importa los objetos contenidos en un pkcs12 en memoria en el clauer.
 *
 * Importa los objetos contenidos en un pkcs#12 en el clauer.
 *
 * \param pkcs12
 *        El buffer que contiene el objeto pkcs12.
 * 
 * \param tam
 *        El tamaño del buffer que contiene el objeto pkcs12.
 *
 * \param pwd
 *        La contraseña que protege el clauer.
 *
 * \param hClauer
 *        Handle al clauer.
 * 
 * \param mode 
 *        Ciphering mode that must be applied to keys.
 * 
 * \param extra_pwd
 *        Extra password for ciphering modes that requiere it.
 * 
 * \param importarCAs
 *        0 o 1 según se desee que se importen o no los certs de CA root e intermedia asociados al cert personal.
 *
 * \retval TRUE
 *         Ok
 *
 * \retval FALSE
 *         Error
 *
 */


BOOL 
LIBIMPORT_ImportarPKCS12deBuffer (BYTE * pkcs12, DWORD tam, const char *pwd, USBCERTS_HANDLE *hClauer, int mode, char *extra_pwd, BOOL importarCAs)
{
	
	DWORD tamTotal = 0;
	BYTE *llavePrivada, *certLlave, *certs,*blob, *blobCifrado, iv[20], desc[31];
	unsigned int i, j;
	long nb;
	int esRoot, res, tamaux;
	BYTE bloque[TAM_BLOQUE];
	BYTE id[20], idExchange[20], idSignature[20];
	char container[MAX_PATH];
	unsigned long tamBlob;
	long hSignature=-1, hExchange = -1;

	char *friendlyName = NULL;
	unsigned long tamFriendlyName;

	unsigned long tamLlavePrivada, tamCertLlave, *tamCerts, numCerts, keyUsage;
	int tamBlobCifrado, auxtamBlob;
	DWORD alg;
	long posBloque;

	int TAM=0, ciphering=0;

	BOOL blobExiste = FALSE;
	
	BIO * bio_plain, * bio_pkcs8;
	RSA * plain_key;
	
	EVP_PKEY *pkey;
	unsigned char * p;
	
	OpenSSL_add_all_algorithms();
	
	if ( !gbLibImportIni ) {
		LIBRT_Ini();
		CRYPTO_Ini();
		gbLibImportIni = TRUE;
	}

	memset(idExchange, 0, 20);
	memset(idSignature, 0, 20);

	if ( !pkcs12 ) {
	  return FALSE;
	}
	
	if ( CRYPTO_ParsePKCS12 (pkcs12, tam, (char *)pwd, 
				 NULL, &tamLlavePrivada, NULL, 
				 &tamCertLlave, NULL, NULL, 
				 &numCerts, NULL, &tamFriendlyName) != 0) {

		CRYPTO_SecureZeroMemory(pkcs12, tam);
		// printf("LIBIMPORT: Error parseando pkcs12\n");
		return FALSE;
	}

	fflush(stdout);
	llavePrivada = (BYTE *) malloc (tamLlavePrivada);
	if ( !llavePrivada ) {
		CRYPTO_SecureZeroMemory(pkcs12, tam);
		return FALSE;
	}

	certLlave = (BYTE *) malloc (tamCertLlave);
	if ( !certLlave ) {
		CRYPTO_SecureZeroMemory(pkcs12, tam);
		free(llavePrivada);
		llavePrivada = NULL;
		return FALSE;
	}

	tamCerts = (unsigned long *) malloc (sizeof(unsigned long)*numCerts);

	if ( !tamCerts ) {
		CRYPTO_SecureZeroMemory(pkcs12, tam);
		free(certLlave);
		free(llavePrivada);
		llavePrivada = NULL;
		return FALSE;
	}

	if ( tamFriendlyName > 0 ) {

		friendlyName = (char *) malloc (tamFriendlyName+1);

		if ( !friendlyName ) {
			CRYPTO_SecureZeroMemory(pkcs12, tam);
			free(certLlave);
			free(llavePrivada);
			free(tamCerts);
			tamCerts = NULL;
			llavePrivada = NULL;
			return FALSE;
		}

	}
	
	if ( CRYPTO_ParsePKCS12 (pkcs12, tam, (char *)pwd,
				 llavePrivada, &tamLlavePrivada,
				 certLlave, &tamCertLlave,
				 NULL, tamCerts, &numCerts, friendlyName, &tamFriendlyName) != 0 ) 
	    {
		CRYPTO_SecureZeroMemory(pkcs12, tam);
		free(certLlave);
		free(llavePrivada);
		if ( friendlyName ) free(friendlyName);
		free(tamCerts);
		llavePrivada = NULL;
		return FALSE;	
	}

	for ( i = 0 ; i < numCerts ; i++ )
		tamTotal += tamCerts[i];

	certs = (BYTE *) malloc (tamTotal);

	if ( !certs ) {
		CRYPTO_SecureZeroMemory(llavePrivada, tamLlavePrivada);
		CRYPTO_SecureZeroMemory(pkcs12, tam);
		free(certLlave);
		free(llavePrivada);
		free(tamCerts);
		if ( friendlyName ) free(friendlyName);
		llavePrivada = NULL;
		return FALSE;	
	}


	if ( CRYPTO_ParsePKCS12 (pkcs12, tam, (char *) pwd,
				 llavePrivada, &tamLlavePrivada,
				 certLlave, &tamCertLlave,
				 certs, tamCerts, &numCerts, NULL, NULL) != 0) 
	    {
		CRYPTO_SecureZeroMemory(llavePrivada, tamLlavePrivada);
		CRYPTO_SecureZeroMemory(pkcs12, tam);
		free(certLlave);
		free(llavePrivada);
		free(tamCerts);
		free(certs);
		if ( friendlyName ) free(friendlyName);
		llavePrivada = NULL;
		return FALSE;
	    }
	
	CRYPTO_SecureZeroMemory(pkcs12, tam);
	
	/* Importamos la llave privada
	 */

	if ( CRYPTO_LLAVE_PEM_Id (llavePrivada, tamLlavePrivada, 1, NULL, id) != 0) {
		CRYPTO_SecureZeroMemory(llavePrivada, tamLlavePrivada);
		free(certLlave);
		free(llavePrivada);
		free(tamCerts);
		free(certs);
		if ( friendlyName ) free(friendlyName);
		llavePrivada = NULL;
		return FALSE;		
	}

	memset(bloque, 0, TAM_BLOQUE);

	/* IMPLEMENTACIÓN DE LOS MODOS DE CIFRADO */ 


	switch (mode){
	
	case PRIVKEY_DEFAULT_CIPHER:
	    
	    BLOQUE_LLAVEPRIVADA_Nuevo(bloque);
	    BLOQUE_Set_Cifrado(bloque);
	    BLOQUE_LLAVEPRIVADA_Set_Tam(bloque, tamLlavePrivada);
	    BLOQUE_LLAVEPRIVADA_Set_Id(bloque,id);
	    BLOQUE_LLAVEPRIVADA_Set_Objeto(bloque, llavePrivada, tamLlavePrivada);	    

	    break; 
	    
	case PRIVKEY_NO_CIPHER:

	    BLOQUE_LLAVEPRIVADA_Nuevo(bloque);
	    BLOQUE_Set_Claro(bloque);    
	    BLOQUE_LLAVEPRIVADA_Set_Tam(bloque, tamLlavePrivada);
	    BLOQUE_LLAVEPRIVADA_Set_Id(bloque,id);
	    BLOQUE_LLAVEPRIVADA_Set_Objeto(bloque, llavePrivada, tamLlavePrivada);	    

	    break;

	case PRIVKEY_DOUBLE_CIPHER:
	    BLOQUE_CIPHER_PRIVKEY_PEM_Nuevo(bloque);	    
	    BLOQUE_Set_Cifrado(bloque);    	   	    
	    BLOQUE_CIPHER_PRIVKEY_PEM_Set_Id(bloque,id);
	    
	    break;

	case PRIVKEY_PASSWORD_CIPHER:
	    
	    BLOQUE_CIPHER_PRIVKEY_PEM_Nuevo(bloque);	    	
	    BLOQUE_Set_Claro(bloque);
	    BLOQUE_CIPHER_PRIVKEY_PEM_Set_Id(bloque,id);

	    break;
	}
	

	if ( mode == PRIVKEY_DOUBLE_CIPHER || mode == PRIVKEY_PASSWORD_CIPHER ){ 
	    /* Continua la fiesta ... */
	    
	    /* Aquí creamos el PKCS#8 cifrado con la password que nos han pasado */ 
	    
	    bio_plain= BIO_new_mem_buf( llavePrivada , tamLlavePrivada );
	    if ( !bio_plain ){
		CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
		CRYPTO_SecureZeroMemory(llavePrivada, tamLlavePrivada);
		free(certLlave);		
		free(llavePrivada);
		free(tamCerts);
		free(certs);
		if ( friendlyName ) free(friendlyName);
		llavePrivada = NULL;
		
		return FALSE;
		
	    }
	    
	    plain_key= PEM_read_bio_RSAPrivateKey( bio_plain, NULL, NULL,NULL );
	    
	    if ( ! plain_key ){
		CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
		CRYPTO_SecureZeroMemory(llavePrivada, tamLlavePrivada);
		free(certLlave);		
		BIO_free(bio_plain);
		free(tamCerts);
		free(certs);
		if ( friendlyName ) free(friendlyName);
		llavePrivada = NULL;		

		return FALSE;	       
	    }

	    pkey = EVP_PKEY_new();
	    
	    if ( ! pkey ){
		CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
		CRYPTO_SecureZeroMemory(llavePrivada, tamLlavePrivada);
		free(certLlave);	
		RSA_free(plain_key);
		BIO_free(bio_plain);
		free(tamCerts);
		free(certs);
		if ( friendlyName ) free(friendlyName);
		llavePrivada = NULL;			
		
		return FALSE;
	    }
	    
	    if ( ! EVP_PKEY_assign_RSA(pkey, plain_key) ){
		CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
		CRYPTO_SecureZeroMemory(llavePrivada, tamLlavePrivada);
		free(certLlave);	
		RSA_free(plain_key);
		BIO_free(bio_plain);
		free(tamCerts);
		free(certs);
		if ( friendlyName ) free(friendlyName);
		llavePrivada = NULL;			
		
		return FALSE;
	    }
	    
	    
	    bio_pkcs8= BIO_new(BIO_s_mem());
	    
	    if ( ! bio_pkcs8 ){
		CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
		CRYPTO_SecureZeroMemory(llavePrivada, tamLlavePrivada);
		free(certLlave);	
		RSA_free(plain_key);
		BIO_free(bio_plain);
		EVP_PKEY_free(pkey);
		free(tamCerts);
		free(certs);
		if ( friendlyName ) free(friendlyName);
		llavePrivada = NULL;		
		
		return FALSE;						
	    }
	    
	    if ( extra_pwd == NULL ){
		CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
		CRYPTO_SecureZeroMemory(llavePrivada, tamLlavePrivada);
		free(certLlave);	
		RSA_free(plain_key);
		BIO_free(bio_plain);
		BIO_free(bio_pkcs8);
		EVP_PKEY_free(pkey);
		free(tamCerts);
		free(certs);
		if ( friendlyName ) free(friendlyName);
		llavePrivada = NULL;			
		
		return FALSE;
	    }
	    
	    res= PEM_write_bio_PKCS8PrivateKey(bio_pkcs8, pkey, EVP_des_ede3_cbc(),
					       extra_pwd, strlen(extra_pwd),
					       0, NULL);
	    
	    if ( ! res ){
		CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
		CRYPTO_SecureZeroMemory(llavePrivada, tamLlavePrivada);
		free(certLlave);	
		RSA_free(plain_key);
		BIO_free(bio_plain);
		BIO_free(bio_pkcs8);
		EVP_PKEY_free(pkey);
		free(tamCerts);
		free(certs);
		if ( friendlyName ) free(friendlyName);
		llavePrivada = NULL;			
		
		return FALSE;
	    }
	    
	    
	    
	    tam = BIO_get_mem_data(bio_pkcs8, &p);
	    
	    if ( tam <= 0 ){
		CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
		CRYPTO_SecureZeroMemory(llavePrivada, tamLlavePrivada);
		free(certLlave);	
		RSA_free(plain_key);
		BIO_free(bio_plain);
		BIO_free(bio_pkcs8);
		EVP_PKEY_free(pkey);
		free(tamCerts);
		free(certs);
		if ( friendlyName ) free(friendlyName);
		llavePrivada = NULL;	
		
		return FALSE;
	    }	    	    
	    
	    BLOQUE_CIPHER_PRIVKEY_PEM_Set_Tam(bloque, tam);
	    BLOQUE_CIPHER_PRIVKEY_PEM_Set_Objeto(bloque, p, tam);

	    ciphering= 1;
	}
	    
	switch ( LIBIMPORT_BuscarLlavePrivada (llavePrivada, tamLlavePrivada, hClauer, &posBloque, mode) ) {

	case 0:

		/* Si no existe la insertamos en un nuevo bloque
		 */


		if ( LIBRT_InsertarBloqueCrypto(hClauer,bloque,&nb) != 0 ) {
			CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
			CRYPTO_SecureZeroMemory(llavePrivada, tamLlavePrivada);
			free(certLlave);
			free(llavePrivada);
			free(tamCerts);
			free(certs);
			if ( friendlyName ) free(friendlyName);
			llavePrivada = NULL;
			return FALSE;		
		}

		break;

	case 1:

		/* Si ya existe machacamos la llave que teníamos
		 */


		if ( LIBRT_EscribirBloqueCrypto(hClauer, posBloque, bloque) != 0 ) {
			CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
			CRYPTO_SecureZeroMemory(llavePrivada, tamLlavePrivada);
			free(certLlave);
			free(llavePrivada);
			free(tamCerts);
			free(certs);
			if ( friendlyName ) free(friendlyName);
			llavePrivada = NULL;
			return FALSE;		
		}

		break;

	case 2:

		/* Si error a la porra
		 */


		CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
		CRYPTO_SecureZeroMemory(llavePrivada, tamLlavePrivada);
		free(certLlave);
		free(llavePrivada);
		free(tamCerts);
		free(certs);
		if ( friendlyName ) free(friendlyName);
		llavePrivada = NULL;
		return FALSE;		

		break;

	}
	if ( ciphering ){
	    RSA_free(plain_key);
	    BIO_free(bio_plain);
	    BIO_free(bio_pkcs8);
	    // EVP_PKEY_free(pkey);       
	}
	
	CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);


	/* Creamos el PRIVATEKEYBLOB para el CRYPTO API y lo importamos
	 */
	


	/* En función del propósito del certificado, el blob debe insertarse como
	 * AT_KEYEXCHANGE o como AT_SIGNATURE. El criterio que utilizamos es el
	 * siguiente. Si el propósito del certificado es sólo firma (nada de cifrado), entonces
	 * AT_SIGNATURE si en algún momento aparece algo de cifrado AT_KEYEXCHANGE
	 * (independientemente de que pueda aparecer algo de firma)
	 */

	if ( CRYPTO_X509_Get_KeyUsage (certLlave, tamCertLlave, &keyUsage) != 0 ) {
		CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
		CRYPTO_SecureZeroMemory(llavePrivada, tamLlavePrivada);
		free(certLlave);
		free(llavePrivada);
		free(tamCerts);
		free(certs);
		if ( friendlyName ) free(friendlyName);
		llavePrivada = NULL;
		return FALSE;
	}

	alg = CALG_RSA_KEYX;

	if ( ( keyUsage & CRYPTO_KU_DIGITAL_SIGNATURE ) || 
		 ( keyUsage & CRYPTO_KU_KEY_CERT_SIGN ) ||
		 ( keyUsage & CRYPTO_KU_CRL_SIGN ) )
		alg = CALG_RSA_SIGN;

	if ( ( keyUsage & CRYPTO_KU_KEY_ENCIPHERMENT ) ||
		 ( keyUsage & CRYPTO_KU_DATA_ENCIPHERMENT ) ||
		 ( keyUsage & CRYPTO_KU_KEY_AGREEMENT ) ||
		 ( keyUsage & CRYPTO_KU_ENCIPHER_ONLY ) ||
		 ( keyUsage & CRYPTO_KU_DECIPHER_ONLY ) )
		 alg = CALG_RSA_KEYX;
		

	/* Ahora sí... creamos el private key blob
	 */ 

	if ( !CRYPTO_LLAVE2BLOB(llavePrivada, tamLlavePrivada, NULL, alg, NULL, &tamBlob)) {
		CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
		CRYPTO_SecureZeroMemory(llavePrivada, tamLlavePrivada);
		free(certLlave);
		free(llavePrivada);
		free(tamCerts);
		free(certs);
		if ( friendlyName ) free(friendlyName);
		llavePrivada = NULL;
		return FALSE;		
	}
	
	
	blob = (BYTE *) malloc (tamBlob);

	if ( !blob ) {
		CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
		CRYPTO_SecureZeroMemory(llavePrivada, tamLlavePrivada);
		free(certLlave);
		free(llavePrivada);
		free(tamCerts);
		free(certs);
		if ( friendlyName ) free(friendlyName);
		llavePrivada = NULL;
		return FALSE;		
	}

	if ( !CRYPTO_LLAVE2BLOB(llavePrivada, tamLlavePrivada, NULL, alg, blob, &tamBlob) != 0) {
		CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
		CRYPTO_SecureZeroMemory(llavePrivada, tamLlavePrivada);
		free(certLlave);
		free(llavePrivada);
		free(tamCerts);
		free(certs);
		free(blob);
		if ( friendlyName ) free(friendlyName);
		llavePrivada = NULL;
		return FALSE;		
	}

	CRYPTO_SecureZeroMemory(llavePrivada, tamLlavePrivada);
	free(llavePrivada);
	llavePrivada = NULL;
	


	
	/*
	 
	En este punto, si el modo a almacenar el objeto es 
	con doble password, sin password o con una password 
	distinta a la del clauer, debemos contemplarlo.
	
	*/
   



	/* BLOQUE_Set_Cifrado(bloque);    		
	BLOQUE_PRIVKEYBLOB_Nuevo(bloque);
	BLOQUE_PRIVKEYBLOB_Set_Tam(bloque, tamBlob);
	BLOQUE_PRIVKEYBLOB_Set_Id(bloque, id);
	BLOQUE_PRIVKEYBLOB_Set_Objeto(bloque, blob, tamBlob);      	    
	*/
	    
	
	switch (mode){
	    
	case PRIVKEY_DEFAULT_CIPHER:
	    BLOQUE_Set_Cifrado(bloque);    
	    BLOQUE_PRIVKEYBLOB_Nuevo(bloque);
	    BLOQUE_PRIVKEYBLOB_Set_Tam(bloque, tamBlob);
	    BLOQUE_PRIVKEYBLOB_Set_Id(bloque, id);
	    BLOQUE_PRIVKEYBLOB_Set_Objeto(bloque, blob, tamBlob);      	    
	    break;
	    
	case PRIVKEY_NO_CIPHER:       
	    BLOQUE_Set_Claro(bloque);    
	    BLOQUE_PRIVKEYBLOB_Nuevo(bloque);
	    BLOQUE_PRIVKEYBLOB_Set_Tam(bloque, tamBlob);
	    BLOQUE_PRIVKEYBLOB_Set_Id(bloque, id);
	    BLOQUE_PRIVKEYBLOB_Set_Objeto(bloque, blob, tamBlob);      	    
	    break;
		
	case PRIVKEY_DOUBLE_CIPHER:
	    BLOQUE_Set_Cifrado(bloque);
	    BLOQUE_CIPHPRIVKEYBLOB_Nuevo(bloque);
	    break;
	   	    
	case PRIVKEY_PASSWORD_CIPHER:
	    BLOQUE_Set_Claro(bloque);
	    BLOQUE_CIPHPRIVKEYBLOB_Nuevo(bloque);
       	    break;
	
	}

	if ( mode == PRIVKEY_DOUBLE_CIPHER  || mode == PRIVKEY_PASSWORD_CIPHER ){
	    
	    if(!extra_pwd){
		
		return FALSE;
	    }
	    
	    auxtamBlob = tamBlob - sizeof(BLOBHEADER); 
	    blobCifrado= (unsigned char *) malloc(tamBlob + 8);
	    memcpy(blobCifrado, blob, sizeof(BLOBHEADER));
	    
	  	    
	    if ( CRYPTO_PBE_Cifrar ( extra_pwd, 
				     hClauer->idDispositivo, 
				     20, 1000, 1, CRYPTO_CIPHER_DES_EDE3_CBC,
				     blob + sizeof(BLOBHEADER), auxtamBlob,
				     blobCifrado + sizeof(BLOBHEADER),  &tamBlobCifrado ) != 0 ){
		
		CRYPTO_SecureZeroMemory(blob, tamBlob);
		CRYPTO_SecureZeroMemory(extra_pwd, strlen(extra_pwd));
		CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
		free(certLlave);
		free(tamCerts);
		free(certs);
		free(blob);
		if ( friendlyName ) free(friendlyName);
		blob = NULL;
		
		return FALSE;
	    }
	    
	    // printf("auxtamBlob= %d\n", auxtamBlob);
	    // printf("tamBlobCifrado= %d\n", tamBlobCifrado);
	    // printf("Sizeof(BLOBHEADER)= %d\n", sizeof(BLOBHEADER));

	    memset(iv,0,20);
	    memset(desc,0,31);
	    
	    
	    BLOQUE_CIPHPRIVKEYBLOB_Set_Objeto(bloque, blobCifrado, tamBlobCifrado);
	    BLOQUE_CIPHPRIVKEYBLOB_Set_Salt(bloque, hClauer->idDispositivo);
	    BLOQUE_CIPHPRIVKEYBLOB_Set_Id(bloque, id);
	    BLOQUE_CIPHPRIVKEYBLOB_Set_IterCount(bloque, 1000);
	    BLOQUE_CIPHPRIVKEYBLOB_Set_Iv(bloque, iv);
	    BLOQUE_CIPHPRIVKEYBLOB_Set_Desc(bloque, (char *)desc);	    	    	    	   	    
	    
	    ciphering= 1;
	    
	}

	switch ( LIBIMPORT_BuscarBlob (blob, tamBlob, hClauer, id, &posBloque, mode) ) {

	case 0:
		/* Si no existe lo insertamos en un bloque nuevo
		 */

		blobExiste = FALSE;

		if ( LIBRT_InsertarBloqueCrypto(hClauer,bloque,&hSignature) != 0 ) {
			CRYPTO_SecureZeroMemory(blob, tamBlob);
			CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
			free(certLlave);
			free(tamCerts);
			free(certs);
			free(blob);
			if ( friendlyName ) free(friendlyName);
			blob = NULL;
			return FALSE;		
		}

		break;

	case 1:
		/* Si existe machacamos el bloque que teníamos
		 */

		blobExiste = TRUE;

		if ( LIBRT_EscribirBloqueCrypto(hClauer, posBloque, bloque) != 0 ) {
			CRYPTO_SecureZeroMemory(blob, tamBlob);
			CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
			free(certLlave);
			free(tamCerts);
			free(certs);
			free(blob);
			if ( friendlyName ) free(friendlyName);
			blob = NULL;
			return FALSE;		
		}

		if ( alg == CALG_RSA_SIGN )
			hSignature = posBloque;
		else if ( alg == CALG_RSA_KEYX )
			hExchange = posBloque;

		break;

	case 2:
		/* Si error a la porra
		 */

		CRYPTO_SecureZeroMemory(blob, tamBlob);
		CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
		free(certLlave);
		free(tamCerts);
		free(certs);
		free(blob);
		if ( friendlyName ) free(friendlyName);
		blob = NULL;
		return FALSE;		
	}

	CRYPTO_SecureZeroMemory(blob, tamBlob);
	free(blob);
	blob = NULL;

	if ( ciphering ){	    
	    CRYPTO_SecureZeroMemory(extra_pwd, strlen(extra_pwd));
	    CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);	  	  
	    free(blobCifrado);	  
	    blobCifrado = NULL;
	}

	/* Añadimos un key container si el blob no existía ya. Si existía entonces malament.
	 */

	CRYPTO_Random(10,(unsigned char *)container);
	for ( i = 0 ; i < 10 ; i++ ) 
		container[i] = 'a' + ((unsigned char)container[i] % 10);
	container[9] = '\0';

	/* Hago un pequeño arreglo. Si el blob es de una llave AT_SIGNATURE entonces hExchange = -1 y
	 * hSignature se queda como está. Si no lo que hago es asignar a hExchange el valor de hSignature
	 * y hSignature= -1
	 */

	if ( alg == CALG_RSA_KEYX ) {
		if ( hExchange == -1 ) {
			hExchange = hSignature;
			hSignature = -1;
		}
		memcpy(idExchange, id, 20);
	} else
		memcpy(idSignature, id, 20);

	if ( LIBIMPORT_KeyContainer_Insertar(hClauer, container, hSignature, hExchange, idSignature, idExchange, blobExiste) != 0 ) {
		CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
		free(certLlave);
		free(tamCerts);
		free(certs);
		if ( friendlyName )
			free(friendlyName);
		return FALSE;
	}

	/* Ahora insertamos el certificado correspondiente a la llave privada
	 */

	switch ( LIBIMPORT_ExisteCertificado(certLlave, tamCertLlave, BLOQUE_CERT_PROPIO, hClauer, &posBloque) ) {

	case 2:

		/* Error
	     */

		CRYPTO_SecureZeroMemory(bloque, TAM_BLOQUE);
		free(certLlave);
		free(tamCerts);
		free(certs);
		if ( friendlyName ) free(friendlyName);
		return FALSE;

	case 0:

		/* El certificado no estaba insertado
		 */

	    if ( CRYPTO_CERT_PEM_Id (certLlave, tamCertLlave, id) != 0) {
			free(certLlave);
			free(tamCerts);
			free(certs);
			if ( friendlyName ) free(friendlyName);
			return FALSE;		
		}

		BLOQUE_Set_Claro(bloque);
		BLOQUE_CERTPROPIO_Nuevo(bloque);
		BLOQUE_CERTPROPIO_Set_Tam(bloque, tamCertLlave);
		BLOQUE_CERTPROPIO_Set_Id(bloque, id);
		BLOQUE_CERTPROPIO_Set_Objeto(bloque, certLlave, tamCertLlave);

		if ( friendlyName ) {
			BLOQUE_CERTPROPIO_Set_FriendlyName(bloque, friendlyName);
			free(friendlyName);
		}

	
		if ( LIBRT_InsertarBloqueCrypto(hClauer,bloque,&nb) !=0 ) {
			free(certLlave);
			free(tamCerts);
			free(certs);;
			return FALSE;		
		}

		free(certLlave);
		break;


	} /* switch */

	if(!importarCAs) numCerts=0;

	/* Ahora isertamos los certificados raíz
	 */
        tamaux= 0;
	for ( i = 0 ; i < numCerts ; i++ ) {
		memset(bloque, 0, TAM_BLOQUE);

		BLOQUE_Set_Claro(bloque);
		esRoot = CRYPTO_X509_EsRoot(certs+TAM, tamCerts[i]);

		TAM+=tamCerts[i];

		if ( esRoot == 1 ) {	

			switch ( LIBIMPORT_ExisteCertificado(certs+i*TAM_BLOQUE, 
							     tamCerts[i], BLOQUE_CERT_RAIZ, 
							     hClauer, &posBloque) ) {
			case 2:
				/* ERROR
				 */
				free(tamCerts);
				free(certs);;
				return FALSE;

			case 0:
				/* El certificado no estaba insertado
				 */

				BLOQUE_CERTRAIZ_Nuevo(bloque);
				BLOQUE_CERTRAIZ_Set_Tam(bloque, tamCerts[i]);
				BLOQUE_CERTRAIZ_Set_Objeto(bloque, certs + tamaux, tamCerts[i]);


				if ( LIBRT_InsertarBloqueCrypto(hClauer,bloque,&nb) != 0 ) {
					free(certs);
					free(tamCerts);
					return FALSE;
				}

				break;

			} /* switch */

		} else if ( esRoot == 0 ) {


			switch ( LIBIMPORT_ExisteCertificado(certs + tamaux, tamCerts[i], 
							     BLOQUE_CERT_INTERMEDIO, hClauer, 
							     &posBloque) ) {
			case 2:
				/* Error
				 */
				free(tamCerts);
				free(certs);
				return FALSE;

			case 0:
				/* Certificado no estaba insertado
				 */

				BLOQUE_CERTINTERMEDIO_Nuevo(bloque);
				BLOQUE_CERTINTERMEDIO_Set_Tam(bloque, tamCerts[i]);
				BLOQUE_CERTINTERMEDIO_Set_Objeto(bloque, certs + tamaux, tamCerts[i]);
                               
				if ( LIBRT_InsertarBloqueCrypto(hClauer,bloque,&nb) != 0 ) {
					free(certs);
					free(tamCerts);
					return FALSE;
				}

				break;
			} /* switch */
				

		} else {
			free(certs);
			free(tamCerts);
			return FALSE;
		}
        
        tamaux+= tamCerts[i]; 
	}

	/* S'acabó */

	free(certs);
	free(tamCerts);

	return TRUE;
}



/********************************/

//Está definida en misc.h de las clutils, pero paso de enlazar otra librería sólo por esta
void LIBIMPORT_Destroy ( void *buf, unsigned long size )  
{
  volatile unsigned char *i_buf = ( volatile unsigned char * ) buf;
  register unsigned long i = size;
  
  while ( i-- ) {
    *i_buf = 0;
    *i_buf = 0xaa;
    *i_buf = 0x55;
    *(i_buf++) = 0;
  }
}


/*! \brief Exporta en un pkcs12 el certificado en el bloque proporcionado y su llave asociada.
 *
 * Exporta en un pkcs12 el certificado en el bloque proporcionado y su llave asociada.
 *
 * \param blockNum
 *        El número de bloque en el clauer que contiene el certificado personal a exportar.
 *
 * \param pwd
 *        La contraseña que protegerá al pkcs12.
 *
 * \param hClauer
 *        Handle al clauer.
 *
 * \param fileName
 *        El nombre de fichero del pkcs#12 destino.
 *
 * \retval TRUE
 *         Ok
 *
 * \retval FALSE
 *         Error
 *
 */

BOOL LIBIMPORT_ExportarPKCS12 (long blockNum, const char *pwd, USBCERTS_HANDLE *hClauer, const char *fileName)
{

  FILE *fp = NULL;
  unsigned long tamPkcs12;
  unsigned char *pkcs12 = NULL;
      
  fp = fopen(fileName, "wb");
  if ( ! fp ) {
    return 1;
  }
  
  
  //Llamar a  la de abajo
  if(LIBIMPORT_ExportarPKCS12enBuffer(blockNum,pwd,hClauer, &pkcs12, &tamPkcs12)){
    return 1;
  }

  if(!fwrite((const void *) pkcs12, tamPkcs12, 1, fp)){
    fclose(fp);
    return 1;
  }

  fclose(fp);
  LIBIMPORT_Destroy(pkcs12, tamPkcs12);
  free(pkcs12);
  return 0;
}




/*! \brief Exporta en un pkcs12 el certificado en el bloque proporcionado y su llave asociada.
 *
 * Exporta en un pkcs12 el certificado en el bloque proporcionado y su llave asociada.
 *
 * \param blockNum
 *        El número de bloque en el clauer que contiene el certificado personal a exportar.
 *
 * \param pwd
 *        La contraseña que protegerá al pkcs12.
 *
 * \param hClauer
 *        Handle al clauer.
 *
 * \param destBuf
 *        Puntero a buffer que reservaremos y usaremos para escribir el pkcs12 resultante.
 *
 * \retval TRUE
 *         Ok
 *
 * \retval FALSE
 *         Error
 *
 */


BOOL LIBIMPORT_ExportarPKCS12enBuffer (long blockNum, const char *pwd, USBCERTS_HANDLE *hClauer, BYTE ** destBuf, unsigned long * destTam)
{

  
  unsigned char bCert[TAM_BLOQUE], bKey[TAM_BLOQUE], *pkcs12 = NULL;
  int keyFound = 0;
  unsigned long tamPkcs12;
  char * passPkcs12;
  long privKeyBlock;
  
  
  
  if ( mlock(bKey, TAM_BLOQUE)!=0){  //bloqueamos el buffer
    return 1;
  }
  
  CRYPTO_Ini();
  
  if(strlen(pwd) > LIBIMPORT_MAX_PASSPHRASE ){
    return 1;
  }
  
  passPkcs12 = (char *) pwd;
  
  if ( LIBRT_LeerBloqueCrypto(hClauer, blockNum, bCert) != 0 ) {
    return 1;
  }

  if ( ! BLOQUE_Es_Claro(bCert) && (*(bCert+1) != BLOQUE_CERT_PROPIO) ) {
    return 1;
  }


  /* Buscamos llave privada asociada */
  if ( LIBRT_LeerTipoBloqueCrypto(hClauer, BLOQUE_LLAVE_PRIVADA, 1, bKey, &privKeyBlock) != 0 ) {
    return 1;
  }
  
  while ( ( ! keyFound ) && (privKeyBlock != -1) ) {
    
    if ( memcmp(BLOQUE_LLAVEPRIVADA_Get_Id(bKey),
		BLOQUE_CERTPROPIO_Get_Id(bCert),
		20) == 0 ) {
      
      keyFound = 1;
    } else {
      
      LIBIMPORT_Destroy((void *) bKey, TAM_BLOQUE);
      
      LIBIMPORT_Destroy(bKey, TAM_BLOQUE);
      if ( LIBRT_LeerTipoBloqueCrypto(hClauer, BLOQUE_LLAVE_PRIVADA, 0, bKey, &privKeyBlock) != 0 ) {
	LIBIMPORT_Destroy(bKey, TAM_BLOQUE);
	return 1;
      }
      
    }
    
  }
  
  if ( ! keyFound ) {
    return 1;
  }
  
  /* Creamos el PKCS#12 */
  
  if ( CRYPTO_PKCS12_Crear ( BLOQUE_LLAVEPRIVADA_Get_Objeto(bKey),
			     BLOQUE_LLAVEPRIVADA_Get_Tam(bKey),
			     NULL,
			     BLOQUE_CERTPROPIO_Get_Objeto(bCert), 
			     BLOQUE_CERTPROPIO_Get_Tam(bCert),
			     NULL,
			     NULL,
			     0,
			     passPkcs12,
			     BLOQUE_CERTPROPIO_Get_FriendlyName(bCert),
			     NULL,
			     &tamPkcs12) != 0 ) {
    LIBIMPORT_Destroy(bKey, TAM_BLOQUE);
    return 1;
  }

  pkcs12 = ( unsigned char * ) malloc ( tamPkcs12 );


  //fprintf(stderr, "tamPkcs12 %d\n",tamPkcs12);


  if ( ! pkcs12 ) {
    LIBIMPORT_Destroy(bKey, TAM_BLOQUE);
    return 1;
  }
  
  if ( CRYPTO_PKCS12_Crear ( BLOQUE_LLAVEPRIVADA_Get_Objeto(bKey),
			     BLOQUE_LLAVEPRIVADA_Get_Tam(bKey),
			     NULL,
			     BLOQUE_CERTPROPIO_Get_Objeto(bCert), 
			     BLOQUE_CERTPROPIO_Get_Tam(bCert),
			     NULL,
			     NULL,
			     0,
			     passPkcs12,
			     BLOQUE_CERTPROPIO_Get_FriendlyName(bCert),
			     pkcs12,
			     &tamPkcs12) != 0 ) {

    LIBIMPORT_Destroy(bKey, TAM_BLOQUE);


    if ( munlock(bKey, TAM_BLOQUE)!=0){  //desbloqueamos el buffer
      return 1;
    } 

    free(pkcs12);
    free(passPkcs12);
    
    return 1;
  }

  if ( munlock(bKey, TAM_BLOQUE)!=0){  //desbloqueamos el buffer
     return 1;
  }

  
  LIBIMPORT_Destroy(bKey, TAM_BLOQUE);
  
  * destBuf = pkcs12;
  * destTam = tamPkcs12;
  
  return 0;
}
