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

#include "nls.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef WIN32
#include <CRYPTOWrapper/CRYPTOWrap.h>
#endif

#include <LIBRT/libRT.h>

#include "misc.h"


#ifdef LINUX
#include <errno.h>
#include "misc.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#elif defined(WIN32)

/* En windows parece que no tenemos snprintf()
 */
#define snprintf _snprintf

#endif

#define MAX_DEVICE_LEN	256


typedef enum { 
    ST_INI,
    ST_HELP,
    ST_DEV_NAME,
    ST_DEV,
    ST_LIST,
    ST_GETTYPE,
    ST_PASSWORD,
    ST_RAW,
    ST_OK,
    ST_ERR
} parse_st_t;


/* Tipos a listar */
unsigned char g_types[256]; 

/* Si listar todos los tipos */
int g_all;

int g_unknown=0;
int g_raw=0;

char g_device[MAX_PATH_LEN+1], * password=NULL;   /* Dispositivo seleccionado */



void usage ( void )
{
  fprintf(stderr, _("Usage mode: clls [-h] | -l | -r |[-d device] [-p password] (-t TYPE)*\n"));
  fprintf(stderr, _("List the objects in the Clauer\n\n"));
  fprintf(stderr, _("The possible options are:\n"));
  fprintf(stderr, _("   -h, --help               Prints this help message\n"));
  fprintf(stderr, _("   -l, --list               List the clauers plugged on the system\n"));
  fprintf(stderr, _("   -d, --device   disp      Selects the clauer to be listed\n"));
  fprintf(stderr, _("   -p, --password password  Insecure mode of give the password, if not given, the password is asked with getpass.\n"));
  fprintf(stderr, _("   -r, --raw                Raw content listing mode incompatible with -t.\n"));
  fprintf(stderr, _("   -t TYPE, --type TYPE     Lists all the objects of a given type\n\n"));
  fprintf(stderr, _("TYPE can be one of the next values:\n\n"));
  fprintf(stderr, _("   CERT. For certificates with an associated private key\n"));
  fprintf(stderr, _("   ROOT. For root certificates\n"));
  fprintf(stderr, _("   CA.   For intermediate certificates\n"));
  fprintf(stderr, _("   WEB.  For web certificates.\n"));
  fprintf(stderr, _("   PRIV. For private keys\n"));
  fprintf(stderr, _("   CONT. For key containers\n"));
  fprintf(stderr, _("   TOK.  For the token's wallet\n"));
  fprintf(stderr, _("   UNK.  For the unknown type blocks\n"));
  fprintf(stderr, _("   ALL.  All the objects\n"));
  
    /*
      fprintf(stderr, _("Modo de empleo: clls [-h] | (-t TIPO)*\n"));
      fprintf(stderr, _("Lista los objetos contenidos en el Clauer\n\n"));
      fprintf(stderr, _("Las opciones posibles son:\n"));
      fprintf(stderr, _("   -h, --help               imprime este mensaje de ayuda\n"));
      fprintf(stderr, _("   -l, --list               lista los clauers presentes en el sistema\n"));
      fprintf(stderr, _("   -d, --device             selecciona el clauer que queremos listar\n"));
      fprintf(stderr, _("   -p, --password password  Modo inseguro de indicar la password, si no se indica, se pide mediante getpass().\n"));
      fprintf(stderr, _("   -t TIPO, --type TIPO  lista los objetos de un tipo\n\n"));
      fprintf(stderr, _("TIPO puede tomar los siguientes valores:\n\n"));
      fprintf(stderr, _("   CERT. Para certificados con llave privada asociada\n"));
      fprintf(stderr, _("   ROOT. Para certificados raÃ­z\n"));
      fprintf(stderr, _("   CA. Para certiticados intermedios\n"));
      fprintf(stderr, _("   WEB. Para certificados web\n"));
      fprintf(stderr, _("   PRIV. Para llaves privadas\n"));
      fprintf(stderr, _("   CONT. Para key containers\n"));
      fprintf(stderr, _("   TOK. Para cartera de tokens\n"));
      fprintf(stderr, _("   ALL. Todos los objetos del stick\n"));
    */
}



void parse ( int argc, char **argv, parse_st_t *st, char errMsg[256] )
{
  int op, aux, incomp=0;
    unsigned char zero[256];
  
    g_all = 0;
    memset(g_types, 0, sizeof(g_types));
    *g_device = 0;
  
    *st = ST_INI;
  
    for ( op = 1 ; (op < argc) && (*st != ST_ERR) ; op++ ) {
      
	if ( *st == ST_INI ) {
	  
	    if (  ( strcmp(argv[op], "-h") == 0 ) ||
		  ( strcmp(argv[op], "--help") == 0 ) ) {
	      
		*st = ST_HELP;
	      
	    } else if ( ( strcmp(argv[op], "-t") == 0 ) ||
			( strcmp(argv[op], "--type") == 0 ) ) {
	      
		*st = ST_GETTYPE;
	      
	    }
	    else if ( ( strcmp(argv[op], "-r") == 0 ) || strcmp(argv[op], "--raw") == 0 ) {
	      g_raw= 1;
	       if ( incomp ){
		 snprintf(errMsg, 256, _("-r is incompatible with -t\n"));
		 *st= ST_ERR;
	       }else{
		 incomp=1;
		 *st = ST_INI;
	       }	       
	    }
	    else if ( ( strcmp(argv[op], "-l") == 0 ) || strcmp(argv[op], "--list") == 0 ) {
		*st = ST_LIST;
	    } else if ( ( strcmp(argv[op], "-d") == 0 ) || strcmp(argv[op], "--device") == 0 ) {
		*st = ST_DEV_NAME;
	    } else if ( ( strcmp(argv[op], "-p") == 0 ) || strcmp(argv[op], "--password") == 0) {
		*st= ST_PASSWORD;
	    } else {
		snprintf(errMsg, 256, _("Unknown option: %s"), argv[op]);
		*st = ST_ERR;
	    }
	  
	} else if (*st == ST_PASSWORD ){
	    if (password){
		snprintf(errMsg, 256, _("More than one password given\n"));
		*st = ST_ERR;
	    }
	    else{
		aux= strlen(argv[op]);
		if ( aux >= 128 ){
		    snprintf(errMsg, 256, _("The password is longer than 127 characters\n"));
		    *st = ST_ERR;
		}
		else{
		  password= (char *) malloc((sizeof(char) * aux)+1);
		    strncpy(password, argv[op], aux+1);
		    *st = ST_INI;
		}
	    }	    
	}       
	else if ( *st == ST_DEV_NAME ) {	  
	  if ( *g_device ) {
	    snprintf(errMsg, 256, _("You can only select one device"));
	    *st = ST_ERR;
	  } else {
	    strncpy(g_device, argv[op], MAX_PATH_LEN);
	    g_device[MAX_PATH_LEN] = 0;
	    *st = ST_INI;
	  }	
	} else if ( *st == ST_HELP ) {
      
	    snprintf(errMsg, 256, _("No more options allowed when -h or --help are given\n"));
	    *st = ST_ERR;
      
	} else if ( *st == ST_GETTYPE ) {
	  if ( incomp ){
	    snprintf(errMsg, 256, _("-r is incompatible with -t\n"));
	    *st= ST_ERR;
	  }
	  else{
	    incomp=1;
	    if ( strcmp(argv[op], "CERT") == 0 ) {
	      g_types[BLOQUE_CERT_PROPIO] = 1;
	    } else if ( strcmp(argv[op], "CA") == 0 ) {
		g_types[BLOQUE_CERT_INTERMEDIO] = 1;
	    } else if ( strcmp(argv[op], "ROOT") == 0 ) {
		g_types[BLOQUE_CERT_RAIZ] = 1;
	    } else if ( strcmp(argv[op], "PRIV") == 0 ) {
		g_types[BLOQUE_LLAVE_PRIVADA] = 1;
		g_types[BLOQUE_PRIVKEY_BLOB] = 1;		
		g_types[BLOQUE_CIPHER_PRIVKEY_BLOB] = 1;
		g_types[BLOQUE_CIPHER_PRIVKEY_PEM] = 1;
	    } else if ( strcmp(argv[op], "WEB") == 0 ) {
		g_types[BLOQUE_CERT_WEB] = 1;
	    } else if ( strcmp(argv[op], "CONT") == 0 ) {
		g_types[BLOQUE_KEY_CONTAINERS] = 1;
	    } else if ( strcmp(argv[op], "ALL") == 0 ) {
		g_all = 1;
	    } else if ( strcmp(argv[op], "UNK") == 0 ) {	 
		g_unknown=1;		
	    }else if ( strcmp(argv[op], "TOK") == 0 ) {
		g_types[BLOQUE_CRYPTO_WALLET] = 1;
	    } else {
		snprintf(errMsg, 256, _("Unknown object type: %s"), argv[op]);
		*st = ST_ERR;
	    }
      
	    if ( *st != ST_ERR )
		*st = ST_INI;
	  }
      
	} else if ( *st == ST_LIST ) {
	    snprintf(errMsg, 256, _("-l o --list must be given alone. Invalid option: %s"), argv[op]);
	    *st = ST_ERR;
	}
    }
  
    memset(zero, 0, 256);
  
    if ( memcmp(g_types, zero, 256) == 0 ) {
	g_types[BLOQUE_CERT_PROPIO] = 1;
    }

    if ( *st == ST_INI )
	*st = ST_OK;
  
    /* Si hemos acabado en un estado no final, entonces
     * error
     */
  
    if ( *st == ST_GETTYPE ) {
	snprintf(errMsg, 256, _("No object type specified"));
	*st = ST_ERR;
    } else if ( *st == ST_DEV_NAME ) {
	snprintf(errMsg, 256, _("No device name specified"));
	*st = ST_ERR;
    }
  
  
}


char * GetStrType ( unsigned char type )
{

    switch ( type ) {

    case BLOQUE_CERT_PROPIO:
	return _("Own Certificate");

    case BLOQUE_CERT_WEB:
	return _("Web Certificate");

    case BLOQUE_CERT_RAIZ:
	return _("Root Certificate");

    case BLOQUE_LLAVE_PRIVADA:
	return _("Private Key");

    case BLOQUE_PRIVKEY_BLOB:
	return _("M$ Private Key Blob");

    case BLOQUE_KEY_CONTAINERS:
	return _("Key Containers");

    default:
	return _("Unknown");

    }


}


int  print (void)
{
    int type;
    unsigned char block[TAM_BLOQUE];
    long bNumber;
    USBCERTS_HANDLE h;
    int nDisp, i;
    unsigned char *devs[MAX_DEVICES];
    char pass[MAX_PASSPHRASE];
    
    
    if ( ! *g_device ) {
	
	LIBRT_RegenerarCache();
	
	if ( LIBRT_ListarDispositivos(&nDisp, devs) != 0 ) {
	    fprintf(stderr, _("[ERROR] Unable to get Clauers on the system.\n"));
	    return 1;
	}

	if ( nDisp > 1 ) {
	    printf(_("[ATENTION] More than a Clauer plugged on the system.\n"
		     "           Use -d option\n"));
	    for ( i = 0 ; i < nDisp ; i++ )
		free(devs[i]);
	    return 1;
	
	} else if ( nDisp == 0 ) {
	    printf(_("[ATENTION] No Clauers detected on the system.\n"));
	    for ( i = 0 ; i < nDisp ; i++ )
		free(devs[i]);
	    return 0;
	}

	strcpy(g_device, (char *)devs[0]);
	for ( i = 0 ; i < nDisp ; i++ )
	    free(devs[i]);

    }
   
    if ( g_types[BLOQUE_LLAVE_PRIVADA] || g_types[BLOQUE_PRIVKEY_BLOB] || g_types[BLOQUE_CRYPTO_WALLET] ) {
	if ( !password ){
	    if ( CLUTILS_AskPassphrase(_("Clauer's password: "), pass) != 0 ) {
		fprintf(stderr, _("[ERROR] Unable to get the password\n"));
		return 1;
	    }
	}
	else{
	    strncpy(pass, password, strlen(password)+1);
	    CLUTILS_Destroy(pass, strlen(pass)+1);
	}
	
	if ( CLUTILS_ConnectEx2(&h, pass, g_device) != 0 ) {
	    CLUTILS_Destroy(pass, strlen(pass));
	    return 1;
	}
	CLUTILS_Destroy(pass, strlen(pass));
    } else {
	if ( CLUTILS_ConnectEx2(&h, NULL, g_device) != 0 ) {
	    // fprintf(stderr, "[ERROR] Imposible conectar con dispositivo\n");
	    CLUTILS_Destroy(pass, strlen(pass));
	    return 1;
	}
	CLUTILS_Destroy(pass, strlen(pass));
    }
    
    
    for ( type = 0 ; type < 256 ; type++ ) {
      
	if ( g_types[type] ) {
	
	    if ( LIBRT_LeerTipoBloqueCrypto(&h, type, 1, block, &bNumber) != 0 ) {
		fprintf(stderr, _("[ERROR] Enumerating blocks of type %s\n"), GetStrType(type));
		// Clos ended the session itself: LIBRT_FinalizarDispositivo(&h);
		return 1;
	    }

	    while ( bNumber != -1 ) {

		switch ( type ) {

		case BLOQUE_CERT_PROPIO:
		case BLOQUE_CERT_WEB:
		case BLOQUE_CERT_RAIZ:
		case BLOQUE_CERT_INTERMEDIO:
		    CLUTILS_PrintCert(bNumber, block, type);
		    break;

		case BLOQUE_LLAVE_PRIVADA:
		case BLOQUE_CIPHER_PRIVKEY_PEM:
		    CLUTILS_PrintLlave(bNumber, block);
		    break;

		case BLOQUE_PRIVKEY_BLOB:
		case BLOQUE_CIPHER_PRIVKEY_BLOB:
		    CLUTILS_PrintBlob(&h, bNumber, block);
		    break;

		case BLOQUE_KEY_CONTAINERS:
		    CLUTILS_PrintContainer(bNumber, block);

		case BLOQUE_CRYPTO_WALLET:
		    CLUTILS_PrintWallet(bNumber, block);

		}
                #ifdef WIN32
		SecureZeroMemory(block, TAM_BLOQUE);
                #else
		CRYPTO_SecureZeroMemory(block, TAM_BLOQUE);
                #endif
		
		// En el runtime de la parte de windows, leerTipoCryto 
		// da un error cuando llega al final de los bloques cuando 
		// comportamiento correcto es devolver -1. 
		// temporalmente 
		if ( LIBRT_LeerTipoBloqueCrypto(&h, type, 0, block, &bNumber) != 0 ) {
		    //fprintf(stderr, "[ERROR] 2 Enumerando objetos de tipo %s\n", GetStrType(type));
		    bNumber= -1;
		    LIBRT_FinalizarDispositivo(&h);
		    //  return;
		}
		
	    }

	}
    }

    LIBRT_FinalizarDispositivo(&h);
    
    return 0;
}


/* Imprime los bloques en Modo raw */
int printRaw ( )
{
 USBCERTS_HANDLE h;
    unsigned long bNumbers, i, j;
    long *hBlock = NULL;
    unsigned char *blocks = NULL, type;
    int nDisp, zeros=0;
    unsigned char *devs[MAX_DEVICES];
    char pass[MAX_PASSPHRASE],cont;


    if ( ! *g_device ) {

      LIBRT_RegenerarCache();

      if ( LIBRT_ListarDispositivos(&nDisp, devs) != 0 ) {
	fprintf(stderr, _("[ERROR] Unable to get Clauers on the system.\n"));
	return 1;
      }
      
      if ( nDisp > 1 ) {
	printf(_("[ATENTION] More than a Clauer plugged on the system.\n"
		 "           Use -d option\n"));
	for ( i = 0 ; i < nDisp ; i++ )
	  free(devs[i]);
	return 0;
	
      } else if ( nDisp == 0 ) {
	printf(_("[ATENTION] No Clauers detected on the system.\n"));
	for ( i = 0 ; i < nDisp ; i++ )
	  free(devs[i]);
	return 0;
      }
      
      strncpy(g_device, (char *)devs[0],MAX_PATH_LEN);
      for ( i = 0 ; i < nDisp ; i++ )
	free(devs[i]);
    }
    
    
    if ( !password ){
      if ( CLUTILS_AskPassphrase(_("Clauer's password: "), pass) != 0 ) {
	fprintf(stderr, _("[ERROR] Unable to get the password"));
	return 1;
      }
    }
    else{
	strncpy(pass, password, strlen(password)+1);
	CLUTILS_Destroy(password, strlen(password));
    }
    
    if ( CLUTILS_ConnectEx2(&h, pass, g_device) != 0 ) {
	CLUTILS_Destroy(pass, strlen(pass));
	return 1;
    }

    CLUTILS_Destroy(pass, strlen(pass));

    if ( LIBRT_LeerTodosBloquesOcupados ( &h, NULL, NULL, &bNumbers ) != 0 ) {
    	    fprintf(stderr, _("[ERROR] Unable to enumerate objects\n"));
	return 1;
    }

    // printf("Leidos los bloques ocupados: %d\n",bNumbers);
    
    blocks = (unsigned char *) malloc (bNumbers * TAM_BLOQUE);
    
    if ( ! blocks ) {
	LIBRT_FinalizarDispositivo(&h);
	fprintf(stderr, _("[ERROR] No memory available \n"));
	return 1;
    }
    hBlock = ( long * ) malloc ( sizeof(long) * bNumbers );
    if ( ! hBlock ) {
	LIBRT_FinalizarDispositivo(&h);
	free(blocks);
	fprintf(stderr, _("[ERROR] No memory available \n"));
    }

    if ( LIBRT_LeerTodosBloquesOcupados ( &h, hBlock, blocks, &bNumbers ) != 0 ) {
	    fprintf(stderr, _("[ERROR] Unable to enumerate objects\n"));
	return 1;
    }

    LIBRT_FinalizarDispositivo(&h);

   
    for ( i = 0 ; i < bNumbers ; i++ ) {

      type = *(blocks+1);
      
      switch ( type ) {
	
      case BLOQUE_CERT_WEB:
      case BLOQUE_CERT_RAIZ:
      case BLOQUE_CERT_INTERMEDIO:		
      case BLOQUE_CERT_PROPIO:
	printf(_("\n========== BLOQUE CERTIFICADO Tipo: %02x posicion: %ld ==============\n"),type,hBlock[i]);
	for ( j=0; j<63 ;j++) {
	  printf("%02x",blocks[j]);
	  if ((j+1)%32==0) printf("\n");
	}
	printf(_("\n%s"),BLOQUE_CERTPROPIO_Get_Objeto(blocks));
	printf(_("\n==================================================================\n\n"));
	printf(_("Intro para continuar: "));
	scanf(_("%c"),&cont);
	break;
	
      case BLOQUE_LLAVE_PRIVADA:	
      case BLOQUE_CIPHER_PRIVKEY_PEM:			    
	printf(_("\n========== PRIVATE KEY BLOB type: %02x position: %ld ==============\n"),type,hBlock[i]);
	for ( j=0; j<63 ;j++) {
	  printf("%02x",blocks[j]);
	  if ((j+1)%32==0) printf("\n");
	}
	printf(_("\n%s"),BLOQUE_LLAVEPRIVADA_Get_Objeto(blocks));
	printf(_("\n==================================================================\n\n"));
	printf(_("Enter to continue: "));
	scanf(_("%c"),&cont);
	break;          
      case BLOQUE_CIPHER_PRIVKEY_BLOB:
      case BLOQUE_PRIVKEY_BLOB:
	  
	  printf(_("\n========== PRIVATE KEY BLOB type: %02x position: %ld ============\n"),type,hBlock[i]);
	
	for ( j=0; j<63 + BLOQUE_CIPHPRIVKEYBLOB_Get_Tam(blocks) && j<10240 ;j++) {
	  printf("%02x",blocks[j]);
	  if ((j+1)%32==0) printf("\n");
	}
	printf(_("\n==================================================================\n\n"));
	printf(_("Enter to continue: "));
	scanf(_("%c"),&cont);
	break;		          
      case BLOQUE_KEY_CONTAINERS:
	  printf(_("\n========= KEY CONTAINERS BLOCK type: %02x position: %ld ===========\n"),type,hBlock[i]);
	  for ( j=0; j<10240 ;j++) {
	      if ( j<10236 && !blocks[j] &&  !blocks[j+1] &&  !blocks[j+2] ){
		  while(!blocks[j] && j<10236){
		      zeros++; j++;
		  }
		  printf(_("\n *** %d more zeroes till here *** \n"),zeros);
		  zeros=0;
	      }
	      printf(_("%02x"),blocks[j]);
	      if ((j+1)%32==0) printf("\n");
	  }
	  printf(_("\n==================================================================\n\n"));
	break;
	
	     /*	case BLOQUE_CRYPTO_WALLET:
	       if (all)
	       CLUTILS_PrintWallet(hBlock[i], blocks);
	       break;
	     */
      default:
	printf(_("\n=============== UNKNOWN BLOCK type: %02x position: %ld =============\n"),type,hBlock[i]);
	for ( j=0; j<10240 ;j++) {
	  if ( j<10236 && !blocks[j] &&  !blocks[j+1] &&  !blocks[j+2] ){
	    while(!blocks[j] && j<10236){
	      zeros++; j++;
	    }
	    printf(_("\n *** %d more zeroes till here *** \n"),zeros);
	    zeros=0;
	  }
	  printf(_("%02x"),blocks[j]);
	  if ((j+1)%32==0) printf("\n");
	}
	printf(_("\n==================================================================\n\n"));
	printf(_("Enter to continue: "));
	scanf(_("%c"),&cont);

      }
      blocks += TAM_BLOQUE;
    }   

    return 0;
}


int printEx ( int all )
{
    USBCERTS_HANDLE h;
    unsigned long bNumbers, i;
    long *hBlock = NULL;
    unsigned char *blocks = NULL, type;
    int nDisp;
    unsigned char *devs[MAX_DEVICES];
    char pass[MAX_PASSPHRASE];


    if ( ! *g_device ) {

      LIBRT_RegenerarCache();

      if ( LIBRT_ListarDispositivos(&nDisp, devs) != 0 ) {

	fprintf(stderr, _("[ERROR] Unable to get Clauers on the system.\n"));
	return 1;
      }
      
      if ( nDisp > 1 ) {
	printf(_("[ATENTION] More than a Clauer plugged on the system.\n"
		 "           Use -d option\n"));
	for ( i = 0 ; i < nDisp ; i++ )
	  free(devs[i]);
	return 0;
	
      } else if ( nDisp == 0 ) {
	printf(_("[ATENTION] No Clauers detected on the system.\n"));
	for ( i = 0 ; i < nDisp ; i++ )
	  free(devs[i]);
	return 0;
      }
      
      strncpy(g_device, (char *)devs[0],MAX_PATH_LEN);
      for ( i = 0 ; i < nDisp ; i++ )
	free(devs[i]);
    }
    
    
    if ( !password ){
      if ( CLUTILS_AskPassphrase(_("Clauer's password: "), pass) != 0 ) {
	fprintf(stderr, _("[ERROR] Unable to get the password"));
	return 1;
      }
    }
    else{
	strncpy(pass, password, strlen(password)+1);
	CLUTILS_Destroy(password, strlen(password));
    }
    
    if ( CLUTILS_ConnectEx2(&h, pass, g_device) != 0 ) {
	CLUTILS_Destroy(pass, strlen(pass));
	return 1;
    }

    CLUTILS_Destroy(pass, strlen(pass));

    if ( LIBRT_LeerTodosBloquesOcupados ( &h, NULL, NULL, &bNumbers ) != 0 ) {
    	    fprintf(stderr, _("[ERROR] Unable to enumerate objects\n"));
	return 1;
    }

    
    blocks = (unsigned char *) malloc (bNumbers * TAM_BLOQUE);
    
    if ( ! blocks ) {
	LIBRT_FinalizarDispositivo(&h);
	fprintf(stderr, _("[ERROR] No memory available \n"));
	return 1;
    }
    hBlock = ( long * ) malloc ( sizeof(long) * bNumbers );
    if ( ! hBlock ) {
	LIBRT_FinalizarDispositivo(&h);
	free(blocks);
	fprintf(stderr, _("[ERROR] No memory available \n"));
    }

    if ( LIBRT_LeerTodosBloquesOcupados ( &h, hBlock, blocks, &bNumbers ) != 0 ) {
      	    fprintf(stderr, _("[ERROR] Unable to enumerate objects\n"));
	return 1;
    }

   // TODO: Considerar CERT_OTROS.
    for ( i = 0 ; i < bNumbers ; i++ ) {

	type = *(blocks+1);

	switch ( type ) {

	case BLOQUE_CERT_WEB:
	case BLOQUE_CERT_RAIZ:
	case BLOQUE_CERT_INTERMEDIO:		
	case BLOQUE_CERT_PROPIO:
	    if (all)
		CLUTILS_PrintCert(hBlock[i], blocks, type);
	    break;

	case BLOQUE_LLAVE_PRIVADA:	
	case BLOQUE_CIPHER_PRIVKEY_PEM:			    
	    if (all)
		CLUTILS_PrintLlave(hBlock[i], blocks);
	    break;          
	case BLOQUE_PRIVKEY_BLOB:
	    if (all)
		CLUTILS_PrintBlob(&h, hBlock[i], blocks);
	    break;

	case BLOQUE_CIPHER_PRIVKEY_BLOB:
	    if (all)
		CLUTILS_PrintBlob(&h, hBlock[i], blocks);
	    break;

	case BLOQUE_KEY_CONTAINERS:
	    if (all)
		CLUTILS_PrintContainer(hBlock[i], blocks);
	     break;

	case BLOQUE_CRYPTO_WALLET:
	    if (all)
		CLUTILS_PrintWallet(hBlock[i], blocks);
	    break;
	
	default:
	    /* That is Unknown type */
	    printf("%ld::UNKNOWN::TYPE=0x%02x::", hBlock[i], type);
	    if (BLOQUE_Es_Cifrado(blocks))
		printf("MODE=CIPHERED\n");
	    else 
		printf("MODE=CLEAR\n");	     	    	
	}

	blocks += TAM_BLOQUE;

    }
    
    LIBRT_FinalizarDispositivo(&h);
    return 0;

}




int main ( int argc, char **argv )
{
    parse_st_t st;
    char errMsg[256];
    int err=0;


#ifndef MAC
    nls_lang_init();	
#endif
    
   LIBRT_Ini();

#ifdef LINUX
    CRYPTO_Ini();
#endif

    parse(argc, argv, &st, errMsg);

    if ( st == ST_ERR ) {
      fprintf(stderr, "[ERROR] %s\n", errMsg);
      usage();
    } else if ( st == ST_LIST ) {
      CLUTILS_PrintClauers();      
    } else if ( st == ST_HELP ) {
      usage();
    } else if ( st == ST_OK ) {
	if ( g_all ){
	    err= printEx(1);
	}
	else if ( g_unknown ){
	    printEx(0);
	}
	else if ( g_raw ){
	  printRaw();
	}
	else {
	    err= print();
	}
    }
    
    return err;

}


