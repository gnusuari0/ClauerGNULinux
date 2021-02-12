
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

/* Clauer utils miscellaneous functions */

#include "misc.h"
#include "nls.h"
#include <stdio.h>


#ifdef LINUX
#include <stdlib.h>
#include <unistd.h>
#include <CRYPTOWrapper/CRYPTOWrap.h>
#elif defined(WIN32)
#include <windows.h>
#include <wincred.h>
#include <wincrypt.h>

#include <b64/b64.h>
#endif


int CLUTILS_ListarDispositivos( int * nDev, unsigned char ** dev ) {
    
    USBCERTS_HANDLE hClauer;
    int i;
   
    LIBRT_RegenerarCache();
    
    if ( LIBRT_ListarDispositivos(nDev, dev) != 0 ) {
	fprintf(stderr, _("[ERROR] Unable to get Clauers on the system.\n"));
	return 1;
    }
    
    printf("Pasando por listar disp ndev=%d\n", *nDev);

    if ( (*nDev) == 0 ) {
	fprintf(stderr, _("[ERROR] No Clauers detected on the system.\n"));
	return 1;
    } 
    else if ( (*nDev) > 1 ) {
	for (i=0; i< (*nDev); i++){
	    free(dev[i]);
	}
	fprintf(stderr, _("[WARNING] More than a Clauer plugged on the system.\n"));
	return 1;
    }
    
    /* 
     * En este punto, intentamos abrir el dispositivo, si esto falla, regeneramos cache y 
     * lo volvemos a intentar.  
     */
    
    if ( LIBRT_IniciarDispositivo ( dev[0], NULL, &hClauer ) != 0 ) {
	for(i=0; i< (*nDev); i++)
	    free(dev[i]);
	/*LIBRT_RegenerarCache();*/
	if ( LIBRT_ListarDispositivos(nDev, dev) != 0 ) {
	    fprintf(stderr, _("[ERROR] Unable to get Clauers on the system.\n"));
	    return 1;
	}
	
	if ( (*nDev) == 0 ) {
	    fprintf(stderr, _("[WARNING] No Clauers detected on the system.\n"));
	    return 1;
	} 
	else if ( (*nDev) > 1 ) {
	    for (i=0; i< (*nDev); i++){
		free(dev[i]);
	    }
	    fprintf(stderr, _("[WARNING] More than a Clauer plugged on the system.\n"));
	    return 1;
	}
	
	if ( LIBRT_IniciarDispositivo ( dev[0], NULL, &hClauer ) != 0 ) {
	  fprintf(stderr, _("[ERROR] Unable to initialize the device\n"));
	  return 1;
	}
    }
    
    LIBRT_FinalizarDispositivo( &hClauer );
    
    return 0;
}


int CLUTILS_Connect ( USBCERTS_HANDLE *hClauer, int auth )
{
  
    unsigned char *dev[128];

    int nDev = 0,i= 0;
    char pass[MAX_PASSPHRASE];

    
    /* 
       if ( LIBRT_ListarDispositivos(&nDev, dev) != 0 ) {
       fprintf(stderr, _("[ERROR] Imposible enumerar dispositivos\n"));
       return 1;
       }
       
       
       if ( nDev == 0 ) {
       fprintf(stderr, _("[ERROR] No se detectaron clauers en el sistema\n"));
       return 1;
       } else if ( nDev > 1 ) {
       for (i=0; i< nDev; i++){
       free(dev[i]);
       }
       fprintf(stderr, _("[ERROR] MÃ¡s de un clauer conectado en el sistema\n"));
       return 1;
       }
    */

    if( CLUTILS_ListarDispositivos( &nDev, dev ) != 0 ){
	fprintf(stderr, _("[ERROR] Unable to get Clauers on the system.\n"));
	return 1;	
    }

    if ( auth ) {

	if ( CLUTILS_AskPassphrase(_("Clauer's password: "), pass) != 0 ) {
	    fprintf(stderr, _("[ERROR] Unable to get the password\n"));
	    return 1;	
	}

	if ( LIBRT_IniciarDispositivo ( dev[0], pass, hClauer ) != 0 ) {
	    fprintf(stderr, _("[ERROR] Unable to connect with the Clauer. Incorrect password?\n"));
	    /*LIBRT_RegenerarCache();*/
	    CLUTILS_Destroy((void *) pass, MAX_PASSPHRASE);
	    for(i=0; i< nDev; i++)
		  free(dev[i]);
	    return 1;
	}

	CLUTILS_Destroy((void *) pass, MAX_PASSPHRASE);

    } else {

	if ( LIBRT_IniciarDispositivo ( dev[0], NULL, hClauer ) != 0 ) {
	   fprintf(stderr, _("[ERROR] Unable to initialize the device\n"));
	    /*LIBRT_RegenerarCache();*/
            free(dev[0]);
	    return 1;
	}

    }
    for (i=0; i< nDev; i++){
      fflush(stdout);
      free(dev[i]);
    }

    return 0;

}




int CLUTILS_ConnectEx ( USBCERTS_HANDLE *hClauer, char *passphrase )
{
    #ifdef NO_CRYF_SUPPORT
    unsigned char *dev = NULL;
    #else
    unsigned char *dev[128];
    #endif 
   
    int nDev = 0, i= 0;
    
 
    if ( LIBRT_ListarDispositivos(&nDev, dev) != 0 ) {
	fprintf(stderr, _("[ERROR] Unable to get Clauers on the system.\n"));
	return 1;
    }

    if ( nDev == 0 ) {
	fprintf(stderr, _("[ERROR] No Clauers detected on the system.\n"));
	return 1;
    } else if ( nDev > 1 ) {
	for (i=0; i< nDev; i++){
	    free(dev[i]);
	}
	fprintf(stderr, _("[WARNING] More than a Clauer plugged on the system.\n"));
	return 1;
    }


    if ( LIBRT_IniciarDispositivo ( dev[0], passphrase, hClauer ) != 0 ) {
	fprintf(stderr, _("[ERROR] Unable to connect with the Clauer. Incorrect password?\n"));
	for (i=0; i< nDev; i++){
	    free(dev[i]);
	}
	return 1;
    }
 
    for (i=0; i< nDev; i++){
      free(dev[i]);
    }

    return 0;

}




void CLUTILS_Destroy ( void *buf, unsigned long size )
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


int CLUTILS_AskPassphrase ( char *prompt, char passphrase[MAX_PASSPHRASE])
{

    char *pass;
    char c; int i=0;

#ifdef WIN32
    HANDLE hStdIn;
    DWORD dwConsoleMode, dwCharsRead;
#endif

    if ( ! prompt )
	return 1;

    if ( ! passphrase ) 
	return 1;

    memset(passphrase, 0, MAX_PASSPHRASE);

#ifdef LINUX
    // Patched by cuevas (www.cuevas.cat). 
    if (isatty(0)) { /* interactive */
    	pass = getpass(prompt);
        strncpy(passphrase, pass, MAX_PASSPHRASE-1);
        CLUTILS_Destroy(pass, strlen(pass));
    } else {
	while ( (c = getchar()) != '\n' && c != EOF ) {
		if (i < MAX_PASSPHRASE) {
		passphrase[i++] = c;
		}
	}
	passphrase[i] = '\0'; /* terminate NTBS */
    }
   //End patch 

#elif defined(WIN32)

    printf("%s", prompt);
    fflush(stdout);

    hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    if ( hStdIn == INVALID_HANDLE_VALUE )
      return 1;

    if ( ! GetConsoleMode(hStdIn, &dwConsoleMode) )
      return 1;

    if ( ! SetConsoleMode(hStdIn, ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT) )
      return 1;

    pass = ( char * ) malloc ( MAX_PASSPHRASE + 1 );
    if ( ! pass ) {
      SetConsoleMode(hStdIn, dwConsoleMode);
      return 1;
    }

    if ( ! ReadConsole(hStdIn, pass, MAX_PASSPHRASE, &dwCharsRead, NULL) ) {
      free(pass);
      SetConsoleMode(hStdIn, dwConsoleMode);
      return 1;
    }
    printf("\n");
    
    if ( *(pass + dwCharsRead - 1) == '\n' )
      --dwCharsRead;
    if ( *(pass + dwCharsRead - 1) == '\r' )
      --dwCharsRead;

    *(pass + dwCharsRead) = 0;
    strcpy(passphrase, pass);

    CLUTILS_Destroy(pass, dwCharsRead);
    free(pass);    
    SetConsoleMode(hStdIn, dwConsoleMode);

#endif

    return 0;
}



/* Esta función tiene su sentido en windows. El motivo es que la apertura
 * de un dispositivo en éste último posiciona el puntero en el primer sector
 * no en la cuarta partición. Por lo tanto, es un dato a tener en cuenta.
 */

int CLUTILS_Clauer_Is_File ( char deviceName[MAX_PATH_LEN+1] )
{
#if defined(LINUX)
  /* En linux el tratamiento del clauer como fichero y como
   * dispositivo es idéntico, por lo tanto devolvemos siempre
   * cierto
   */
  return 1;
#elif  defined (WIN32)

  HANDLE hFindFile;
  WIN32_FIND_DATA findData;

  hFindFile = FindFirstFile(deviceName, &findData);
  if ( hFindFile == INVALID_HANDLE_VALUE )
    return 0;

  FindClose(hFindFile);
  return 1;

#endif
}



int CLUTILS_PrintClauers ( void )
{
  unsigned char *devices[MAX_DEVICES];
  int nDev, i;
  char owner[MAX_CLAUER_OWNER+1];

  memset(devices, 0, MAX_DEVICES * sizeof(char *));

  //printf("Antes de listar disps\n");

  if ( LIBRT_ListarDispositivos(&nDev, devices) != 0 ) {
    printf("ERROR\n");
    return 0;
  }

  //printf("Despues de listar disp\n");

  if ( nDev == 0 ) 
      fprintf(stderr, _("[ERROR] No Clauers detected on the system.\n"));
  else {
    printf(_("Clauers inserted on the system:\n\n"));
    for ( i = 0 ; i < nDev ; i++ ) {
	CLUTILS_Get_Clauer_Owner((char *)devices[i], owner);
      printf("\t%s (%s)\n", devices[i], owner);
    }
  }

  return 1;
}


int CLUTILS_ConnectEx2 ( USBCERTS_HANDLE *hClauer, char *passphrase, char *device )
{
    unsigned char *dev[128];
    int i= 0;

    if ( ! device )
      return CLUTILS_ConnectEx(hClauer, passphrase);

    memset(dev, 0, sizeof dev);

    /* Cuando el dispositivo se indica, hay que regenerar la cache */

    /*LIBRT_RegenerarCache();*/
    
    if ( LIBRT_IniciarDispositivo ((unsigned char *) device, passphrase, hClauer ) != 0 ) {
	fprintf(stderr, _("[ERROR] Unable to connect with the Clauer. Incorrect password?\n"));
      return 1;
    } 
    
    for (i=0; i< 128 && dev[i]; i++){
      free(dev[i]);
    }
    
    return 0;
}



/*! \brief Convierte un certificado en formato PEM a DER.
 *
 * Convierte un certificado en formato PEM a DER.
 *
 * \param pem
 *		  ENTRADA. El certificado en formato PEM.
 *
 * \param tamPEM
 *		  ENTRADA. El tamaño (en bytes) del certificado alojado en pem.
 *
 * \param der
 *		  SALIDA. El certificado en formato DER.
 *
 * \param tamDER
 *		  SALIDA. El tamaño del certificado DER devuelto.
 *
 * \retval TRUE
 *		   Ok
 *
 * \retval FALSE
 *		   Error
 */

#ifdef WIN32
BOOL CLUTILS_CERT_PEM2DER (BYTE *pem, unsigned long tamPEM, BYTE *der, unsigned long *tamDER)
{
  BYTE *cuerpo_i, *cuerpo_f,*aux;
  BOOL encontrado = FALSE, ret = TRUE;
  unsigned long pos;
  
  /* Buscamos la cabecera
   */

  pos = 0;
  aux = pem;
  while (!encontrado && (pos<tamPEM)) {
    while ( ((*aux) != '-') && (pos< tamPEM) ) {
      ++aux;
      ++pos;
    }
    if ( strncmp((char *)aux,"-----BEGIN CERTIFICATE-----", 27) == 0 )
      encontrado = TRUE;
    else {
      ++aux;
      ++pos;
    }
  }
  
  if ( !encontrado ) {
    ret = FALSE;
    goto finPEM2DER;
  }
  
  
  cuerpo_i = aux + 28; /* del begin certificate y el cambio de línea */
  
  /*
   * Ahora eliminamos el pie
   */
  
  aux = cuerpo_i;
  encontrado = FALSE;
  while ( !encontrado && (pos < tamPEM)) {
    while ( (*aux != '-') && (pos < tamPEM)) {
      ++aux;
      ++pos;
    }
    if ( strncmp((char *)aux,"-----END CERTIFICATE-----", 25) == 0 ) {
      encontrado = TRUE;
    } else {
      ++pos;
      ++aux;
    }
  }
  
  
  if (!encontrado) {
    ret = FALSE;
    goto finPEM2DER;
  }
  
  cuerpo_f = aux-1;
  
  B64_Decodificar(cuerpo_i, cuerpo_f-cuerpo_i+1,NULL,tamDER);
  
  if ( !der ) {
    ret = TRUE;
    goto finPEM2DER;
  }
  
  B64_Decodificar(cuerpo_i, cuerpo_f-cuerpo_i+1,der,tamDER);
  
 finPEM2DER:
  
  return ret;
  
}
#endif




/* Inicialmente, las funciones que se implementan a continuación 
 * se encontraban dentro de clls, pero dado que cldel puede reutilizarlas
 * para el modo interactivo, han pasado a formar parte de las 
 * CLUTILS
 */

void CLUTILS_PrintCert ( unsigned long bNumber, unsigned char *bloque, unsigned char type )
{
    unsigned char *id;
    unsigned char * bloqueAux= NULL;
    int i, tam=0;
#ifdef WIN32
    PCCERT_CONTEXT certCtx;
    BYTE *certDer;
    unsigned long tamDer;
    char cn[MAX_CLAUER_OWNER+1];
#elif defined(LINUX)
    DN *subject, *issuer;
#endif

    //printf("\n\nINICIO FUNCION Certificado: %s", BLOQUE_CERTPROPIO_Get_Objeto(bloque));
    //printf("---- FIN Certificado ----");
    //fflush(stdout);
    

    printf("%ld::", bNumber); 
    if ( type == BLOQUE_CERT_PROPIO ) 
	printf("CERT::%s::", BLOQUE_CERTPROPIO_Get_FriendlyName(bloque));
    else {
	switch ( type ) {
	case BLOQUE_CERT_RAIZ:
	    printf("ROOT::%s::", BLOQUE_CERTRAIZ_Get_FriendlyName(bloque));
	    break;

	case BLOQUE_CERT_WEB:
	    printf("WEB::%s::", BLOQUE_CERTRAIZ_Get_FriendlyName(bloque));
	    break;

	case BLOQUE_CERT_INTERMEDIO:
	    printf("CA::%s::", BLOQUE_CERTRAIZ_Get_FriendlyName(bloque));
	    break;

	case BLOQUE_CERT_OTROS:
	    printf("OTHERS::%s::", BLOQUE_CERTRAIZ_Get_FriendlyName(bloque));
	    break;
	default:
	    printf("UNKNOWN");
	    break;
	}

    }


#ifdef WIN32

    if ( ! CLUTILS_CERT_PEM2DER ( BLOQUE_CERTPROPIO_Get_Objeto(bloque),
				  BLOQUE_CERTPROPIO_Get_Tam(bloque),
				  NULL,
				  &tamDer ) ) 
	{

	    fprintf(stderr, _("[ERROR] Impossible to get the CN of the certificate\n"));
	    return;
	}
        
    certDer = ( BYTE * ) malloc ( tamDer );
    if ( ! certDer ) {
	fprintf(stderr, _("[ERROR] Impossible to get the CN of the certificate\n"));
	return;
    }

    if ( ! CLUTILS_CERT_PEM2DER ( BLOQUE_CERTPROPIO_Get_Objeto(bloque),
				  BLOQUE_CERTPROPIO_Get_Tam(bloque),
				  certDer,
				  &tamDer ) ) 
	{
	    fprintf(stderr, _("[ERROR] Impossible to get the CN of the certificate\n"));
	    return;
	}
        
    certCtx = CertCreateCertificateContext(X509_ASN_ENCODING, certDer, tamDer);
    if ( ! certCtx ) {
	fprintf(stderr, _("[ERROR] Impossible to get the CN of the certificate\n"));
	return;
    }

    CertGetNameString(certCtx, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, NULL, cn, MAX_CLAUER_OWNER+1);

    CertFreeCertificateContext(certCtx);
    free(certDer);

    printf("%s::", cn);    

#elif LINUX

    subject = CRYPTO_DN_New();
    if ( !subject ) {
	printf( _("[ERROR] No memory available \n"));
	return;
    }
    issuer  = CRYPTO_DN_New();
    if ( ! issuer ) {
	CRYPTO_DN_Free(subject);
	printf( _("[ERROR] No memory available \n"));
	return;
    }
    
    switch( type ){
    case BLOQUE_CERT_RAIZ:
    case BLOQUE_CERT_WEB:
    case BLOQUE_CERT_INTERMEDIO:
    case BLOQUE_CERT_OTROS:
	bloqueAux=  BLOQUE_CERTRAIZ_Get_Objeto(bloque);
	tam= BLOQUE_CERTRAIZ_Get_Tam(bloque);
	break;
	
    case BLOQUE_CERT_PROPIO:
	bloqueAux=  BLOQUE_CERTPROPIO_Get_Objeto(bloque);
	tam= BLOQUE_CERTPROPIO_Get_Tam(bloque);
	break;
    }
    
    fflush(stdout);
    
    if ( ! CRYPTO_CERT_SubjectIssuer( bloqueAux,
				      tam, 
				      subject,
				      issuer) ) {
	CRYPTO_DN_Free(subject);
	CRYPTO_DN_Free(issuer);
	printf(_("Impossible to get certificate's subject\n"));
	return;
    }
    
    if (!subject->CN){

	printf("%s", subject->OU);
    }
    else {
	printf("%s", subject->CN);
    }
    
    CRYPTO_DN_Free(subject);
    CRYPTO_DN_Free(issuer);
#endif

    if ( type== BLOQUE_CERT_PROPIO ){
	printf("::");
	id = BLOQUE_CERTPROPIO_Get_Id(bloque);
	for ( i = 0 ; i < 20 ; i++ )
	    printf("%02x", *(id+i));
    }
    printf("\n");

}




void CLUTILS_PrintLlave ( long bNumber, unsigned char block[TAM_BLOQUE] )
{
    unsigned char *id = NULL;
    int i = 0;
    unsigned char  type;

    
    type= *(block+1);


    if ( type == BLOQUE_CIPHER_PRIVKEY_PEM  ) 
	printf("%ld::CIPHERED_PRIV::", bNumber);
    else 
	printf("%ld::PRIV::", bNumber);


    if ( BLOQUE_Es_Cifrado ( block ) ){
	printf("Cifrado::");
    }
    else{
	printf("En claro::");
    }   
    
    id = BLOQUE_LLAVEPRIVADA_Get_Id(block);

    for ( i = 0 ; i < 20 ; i++ ) 
	printf("%02x", *(id+i));

    printf("\n");
}



void CLUTILS_PrintBlob ( USBCERTS_HANDLE *h, long bNumber, unsigned char block[TAM_BLOQUE])
{
    unsigned char *id = NULL, blockC[TAM_BLOQUE];
    int i = 0, err = 0, found;
    long bNumberC;
    unsigned int lstContainerSize = NUM_KEY_CONTAINERS;
    INFO_KEY_CONTAINER lstContainer[NUM_KEY_CONTAINERS];
    unsigned char  type;

    
    type= *(block+1);

    if ( type == BLOQUE_CIPHER_PRIVKEY_BLOB  ) 
	printf("%ld::CIPHERED_BLOB::", bNumber);
    else 
	printf("%ld::BLOB::", bNumber);

    if ( BLOQUE_Es_Cifrado ( block ) ){
	printf("Cifrado::");
    }
    else{
	printf("En claro::");
    }   
    
    
    /*id = BLOQUE_LLAVEPRIVADA_Get_Id(block);*/
    id = BLOQUE_PRIVKEYBLOB_Get_Id(block);

    /* Busco el key container en el que estÃ¡ la llave
     */

    bNumber = 0;

    if ( LIBRT_LeerTipoBloqueCrypto(h, BLOQUE_KEY_CONTAINERS, 1, blockC, &bNumberC) != 0 ) {
	err = 1;
	printf("@@ERROR@@::");
    }

    while ( bNumberC != -1  ) {
    
	if ( BLOQUE_KeyContainer_Enumerar(blockC, lstContainer, &lstContainerSize) != 0 ) {
	    printf("@@ERROR@@::");
	    break;
	}
	
	found = 0;
	for ( i = 0 ; !found && (i < lstContainerSize) ; i++ ) {
	    if ( memcmp(lstContainer[i].idExchange, id, 20) == 0 ) {
		printf("AT_KEYEXCHANGE::%s::", lstContainer[i].nombreKeyContainer);
		found = 1;
	    } else if ( memcmp(lstContainer[i].idSignature, id, 20) == 0 ) {
		printf("AT_SIGNATURE::%s::", lstContainer[i].nombreKeyContainer);
		found = 1;
	    }
	}

	if ( found )
	    break;

	if ( LIBRT_LeerTipoBloqueCrypto(h, BLOQUE_KEY_CONTAINERS, 0, blockC, &bNumberC) != 0 ) {
	    printf("@@ERROR@@::");
	    break;
	}

    }

    for ( i = 0 ; i < 20 ; i++ )
	printf("%02x", *(id+i));

    printf("\n");

}



void CLUTILS_PrintContainer (long bNumber, unsigned char block[TAM_BLOQUE])
{
    INFO_KEY_CONTAINER lstContainer[NUM_KEY_CONTAINERS];
    unsigned int lstContainerSize, i,j;


    if ( BLOQUE_KeyContainer_Enumerar(block, lstContainer, &lstContainerSize) != 0 ) {
	fprintf(stderr, _("[ERROR] Enumerating key containers\n"));
	return;
    }

    for ( i = 0 ; i < lstContainerSize ; i++ ) {

	printf("%ld::CONT::%s::", bNumber, lstContainer[i].nombreKeyContainer);

	for ( j = 0 ; j < 20 ; j++ )
	    printf("%02x", lstContainer[i].idSignature[j]);

	printf("::");

	for ( j = 0 ; j < 20 ; j++ )
	    printf("%02x", lstContainer[i].idExchange[j]);
	    
	printf("\n");

    }

}




/* Simplemente escojo el primer certificado propio con propósito
 * autenticación que haya en el clauer... puede ser o no el propietario
 * pero bueno, puede servir para distinguir dos clauers entre sí
 */

int CLUTILS_Get_Clauer_Owner ( char *device, char owner[MAX_CLAUER_OWNER+1] )
{
  USBCERTS_HANDLE hClauer;
  unsigned char bloque[TAM_BLOQUE];
  long nBloque;

  int isOwnerCert;

#ifdef WIN32
  unsigned long tamDer;
  BYTE *certDer;

  PCCERT_CONTEXT certCtx;
  BYTE keyUsage[2];
  DWORD dwKeyUsageSize;
#elif defined(LINUX)
  DN *subject, *issuer;
#endif

  *owner = 0;
  
  if ( LIBRT_IniciarDispositivo((unsigned char *)device, NULL, &hClauer) != 0 )
    return 1;

  if ( LIBRT_LeerTipoBloqueCrypto(&hClauer, BLOQUE_CERT_PROPIO, 1, bloque, &nBloque) != 0 ) {
    LIBRT_FinalizarDispositivo(&hClauer);
    return 1;
  }

  isOwnerCert = 0;
  while ( !isOwnerCert && (nBloque != -1) ) {

#ifdef WIN32

    /* Intento evitar en la medida de lo posible utilizar OpenSSL en Windows...
     * menos DLLs a instalar
     */

    if ( ! CLUTILS_CERT_PEM2DER ( BLOQUE_CERTPROPIO_Get_Objeto(bloque),
				  BLOQUE_CERTPROPIO_Get_Tam(bloque),
				  NULL,
				  &tamDer ) ) 
      {
	LIBRT_FinalizarDispositivo(&hClauer);
	return 1;
      }


    certDer = ( BYTE * ) malloc ( tamDer );
    if ( ! certDer ) {
      LIBRT_FinalizarDispositivo(&hClauer);
      return 1;
    }

    if ( ! CLUTILS_CERT_PEM2DER ( BLOQUE_CERTPROPIO_Get_Objeto(bloque),
				  BLOQUE_CERTPROPIO_Get_Tam(bloque),
				  certDer,
				  &tamDer ) ) 
      {
	LIBRT_FinalizarDispositivo(&hClauer);
	free(certDer);
	return 1;
      }
        
    certCtx = CertCreateCertificateContext(X509_ASN_ENCODING, certDer, tamDer);
    if ( ! certCtx ) {
      LIBRT_FinalizarDispositivo(&hClauer);
      free(certDer);
      return 1;
    }

    /* Compruebo que el key usage sea para autenticación... si no puedo comprobarlo
     * asumo que el certificado que tengo es adecuado
     */

    ZeroMemory(keyUsage, 2);
    dwKeyUsageSize = 2;

    if ( ! CertGetIntendedKeyUsage(X509_ASN_ENCODING, certCtx->pCertInfo, keyUsage, dwKeyUsageSize) ) {
      isOwnerCert = 1;
    } else {
      
      if ( ( keyUsage[0] & CERT_DIGITAL_SIGNATURE_KEY_USAGE ) ||
	   ( keyUsage[0] & CERT_NON_REPUDIATION_KEY_USAGE ) ||
	   ( keyUsage[1] & CERT_DIGITAL_SIGNATURE_KEY_USAGE ) ||
	   ( keyUsage[1] & CERT_NON_REPUDIATION_KEY_USAGE ) )
	isOwnerCert = 1;
      else
	isOwnerCert = 0;

    }

    if ( isOwnerCert )
      CertGetNameString(certCtx, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, NULL, owner, MAX_CLAUER_OWNER+1);

    CertFreeCertificateContext(certCtx);
    free(certDer);

#elif defined(LINUX)
    
    subject = CRYPTO_DN_New();
    if ( !subject ) 
      return 1;
    
    issuer  = CRYPTO_DN_New();
    if ( ! issuer ) {
      CRYPTO_DN_Free(subject);
      return 1;
    }
    
    if ( ! CRYPTO_CERT_SubjectIssuer(BLOQUE_CERTPROPIO_Get_Objeto(bloque),
				     BLOQUE_CERTPROPIO_Get_Tam(bloque),
				     subject,
				     issuer) ) {
      CRYPTO_DN_Free(subject);
      CRYPTO_DN_Free(issuer);
      return 1;
    }

    strncpy(owner, subject->CN, MAX_CLAUER_OWNER);
    owner[MAX_CLAUER_OWNER] = 0;
    
    CRYPTO_DN_Free(subject);
    CRYPTO_DN_Free(issuer);

#endif

  if ( LIBRT_LeerTipoBloqueCrypto(&hClauer, BLOQUE_CERT_PROPIO, 0, bloque, &nBloque) != 0 ) {
    LIBRT_FinalizarDispositivo(&hClauer);
    return 1;
  }

 }


  return 0;
}


/* Para imprimir una cartera de tokens
 */

typedef struct {
  char name[24];
  unsigned char attr;
  char cond[164];
  char value[24];
} token_t;

void CLUTILS_PrintWallet ( long bNumber, unsigned char block[TAM_BLOQUE] )
{

  token_t *t = ( token_t * ) (block+8);
  int i,j;

  if ( ! block )
    return;

  for ( i = 0 ; (i < 10224/sizeof(token_t)) && *(t->name) ; i++ ) {
    printf("%ld::TOKEN::%s::%02x::%s::", bNumber, t->name, t->attr, t->cond);
    for ( j = 0 ; j < 24 ; j++ )
      printf("%02x", t->value[j]);

    putchar('\n');
    fflush(stdout);
    ++t;
  }
}

