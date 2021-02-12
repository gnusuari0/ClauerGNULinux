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

#include <stdio.h>
#include <stdlib.h>

#include <CRYPTOWrapper/CRYPTOWrap.h>
#include <LIBRT/libRT.h>
#include <clio/clio.h>

#ifdef WIN32
#include <getopt/getopt.h>
#else
#include <unistd.h>
#define _GNU_SOURCE
#include <getopt.h>
#endif

#include <ctype.h>

#include "misc.h"
#include "nls.h"


#ifdef WIN32
#define snprintf _snprintf
#endif

#define MAX_ITEMS          256
#define MAX_FILE_NAME      256
#define MAX_P12_BLOCKS     256
#define MAX_FILES          50

typedef enum {
  ST_INI,
  ST_HELP,
  ST_LIST,
  ST_ERR
} parse_st_t;


/* Prototipos de funciones de exportación
 */

int ExportPKCS12     ( int nItem );
int ExportBlock      ( int nItem );
int ExportCryptoZone ( int nItem );

/* Tipo puntero a función de exportación
 */

typedef int ( * EXPORT_FUNC ) ( int nItem );

/* Información sobre los items a exportar
 */

typedef enum { EXP_P12, EXP_OBJ, EXP_BLOCK, EXP_DUMP } export_type_t;
typedef struct {
  char szFileName[MAX_FILE_NAME+1];
  export_type_t expType;
  EXPORT_FUNC f;
  long nb;
} export_item_t;

/* Variables globales
 */

export_item_t g_exportItems[MAX_ITEMS];    /* Items a exportar del clauer */
int g_itemsPos;

char g_szDevice[MAX_PATH_LEN+1];             /* Clauer a utilizar */
char g_szClauerPass[MAX_PASSPHRASE];

/* Imprime el uso del certificado
 */

void usage ( void )
{
  fprintf(stderr, _("Usage Mode: clexport [-h] | [-l] | [-d device] (-c | -p block | -b block) [-o file]\n"));
  fprintf(stderr, _("Exports objects from Clauer\n\n"));
  fprintf(stderr, _("Options are:\n"));
  fprintf(stderr, _("  -h, --help          Prints this help message\n"));
  fprintf(stderr, _("  -d, --device device Selects the clauer to be listed\n"));
  fprintf(stderr, _("  -l, --list          List the clauers plugged on the system\n"));
  fprintf(stderr, _("  -c, --crypto        dumps the whole criptographic partition\n"));
  fprintf(stderr, _("  -o, --out file      output file\n"));
  fprintf(stderr, _("  -p, --p12 block     exports a pkcs#12 with the given block number\n"));
  fprintf(stderr, _("  -b, --block block   exports the object in the block block\n"));
  

  /*  fprintf(stderr, "   -l, --list         lista los certificados y su ubicaciÃ³n\n");
  fprintf(stderr, "   -h, --help         imprime este mesane de ayuda\n");
  fprintf(stderr, "   -c, --crypto       vuelca la partición criptográfica entera\n");
  fprintf(stderr, "   -o, --out file     fichero de salida. Si no se especifica es stdout\n\n");
  fprintf(stderr, "Algunos ejemplos de utilización:\n\n");
  fprintf(stderr, "\t. Hacer un dump de la partición criptográfica por salida estándar\n"
	  "\t  clexport -c\n\n");
  fprintf(stderr, "\t. Hacer un dump de la partición criptográfica al fichero \"nisu.cripto\"\n");
  fprintf(stderr, "\t  clexport -o nisu.cripto -c\n\n");
  fprintf(stderr, "\t. Exportar un certificado que se sitúa en el bloque 3 con sus llaves privadas\n");
  fprintf(stderr, "\t  clexport -o nisu.p12 3\n");
  */
}



/* A valid number is: [0-9]+ */

int is_valid_number ( char *s )
{
  while ( *s != '\0' ) {
    if ( ! isdigit((int) *s) )
      return 0;

    ++s;
  }

  return 1;
}




void parse ( int argc, char **argv, parse_st_t *st, char errMsg[256] )
{
  int option_index = 0;
  
  //int this_option_optind = optind ? optind : 1;

  static struct option long_options[] =
    {
      {"help", 0, 0, 0},
      {"device", 1, 0, 0},
      {"list", 0, 0, 0},
      {"p12", 1, 0, 0},
      {"block", 1, 0, 0},
      {"crypto", 0, 0, 0},
      {"out", 1, 0, 0},
      {"other", 1, 0, 0},
      {0,0,0,0}
    };

  int c, i;

  char *endPtr = NULL;

  /* Inicializamos las variables globales
   */
  
  memset(g_exportItems, 0, sizeof g_exportItems);
  memset(g_szDevice, 0, sizeof g_szDevice);
  g_itemsPos = 0;
  *g_szClauerPass = 0;

  *st = ST_INI;

  while ( *st != ST_ERR ) {

    c = getopt_long(argc, argv, "hcld:b:o:p:", long_options, &option_index);
    if ( c == -1 )
      break;

    switch ( c ) {
    case 0:

      break;

    case 'h':
      if ( *st == ST_HELP || *st == ST_LIST )
	*st = ST_ERR;
      else 
	*st = ST_HELP;
      break;

    case 'c':

      if ( *st == ST_HELP || *st == ST_LIST )
	*st = ST_ERR;
      else {
	if ( g_itemsPos >= MAX_ITEMS ) {
	  fprintf(stderr, _("[ERROR] Maximum number of exportable objects exceeded\n"));
	  *st = ST_ERR;
	} else {	 
	  g_exportItems[g_itemsPos].nb            = -1;
	  g_exportItems[g_itemsPos].szFileName[0] = 0; /* Lo establecemos la siguiente vez que encontremos un -o */
	  g_exportItems[g_itemsPos].expType       = EXP_DUMP;
	  g_exportItems[g_itemsPos].f             = ExportCryptoZone;
	  ++g_itemsPos;

	}
      }

      break;

    case 'l':
      if ( *st == ST_HELP || *st == ST_LIST )
	*st = ST_ERR;
      else 
	*st = ST_LIST;
      break;

    case 'd':
      if ( *st == ST_HELP || *st == ST_LIST )
	*st = ST_ERR;
      else 
	strncpy(g_szDevice, optarg, MAX_PATH_LEN);
     
      break;

    case 'b':
      if ( *st == ST_HELP || *st == ST_LIST )
	*st = ST_ERR;
      else {
	if ( g_itemsPos >= MAX_ITEMS ) {
	    fprintf(stderr, _("[ERROR] Maximum number of exportable objects exceeded\n"));
	  *st = ST_ERR;
	} else {	 
	  g_exportItems[g_itemsPos].nb = strtol(optarg, &endPtr, 10);
	  if ( *endPtr != 0 ) {
	    fprintf(stderr, _("[ERROR] %s is not a valid block number\n"), optarg);
	    *st = ST_ERR;
	  } else {
	    g_exportItems[g_itemsPos].szFileName[0] = 0; /* Lo establecemos la siguiente vez que encontremos un -o */
	    g_exportItems[g_itemsPos].expType       = EXP_OBJ;
	    g_exportItems[g_itemsPos].f             = ExportBlock;
	    ++g_itemsPos;
	  }
	}
      }
      break;

    case 'o':
      if ( *st == ST_HELP || *st == ST_LIST )
	*st = ST_ERR;
      else {
	
	/* Buscamos en la lista de items a exportar
	 * e inicializamos el campo de fichero de salida
	 * para cada item
	 */

	for ( i = g_itemsPos-1 ; i >= 0 ; i-- ) {
	  if ( g_exportItems[i].szFileName[0] )
	    break;
	  else {
	    strncpy(g_exportItems[i].szFileName, optarg, MAX_FILE_NAME);
	    g_exportItems[i].szFileName[MAX_FILE_NAME] = 0;
	  }
	}
      }
      break;

    case 'p':
      if ( *st == ST_HELP || *st == ST_LIST )
	*st = ST_ERR;
      else {
	if ( g_itemsPos >= MAX_ITEMS ) {
	  fprintf(stderr, _("[ERROR] Maximum number of exportable objects exceeded\n"));
	  *st = ST_ERR;
	} else {	 
	  g_exportItems[g_itemsPos].nb = strtol(optarg, &endPtr, 10);
	  if ( *endPtr != 0 ) {
	    fprintf(stderr, _("[ERROR] %s is not a valid block number\n"), optarg);
	    *st = ST_ERR;
	  } else {
	    g_exportItems[g_itemsPos].szFileName[0] = 0; /* Lo establecemos la siguiente vez que encontremos un -o */
	    g_exportItems[g_itemsPos].expType       = EXP_P12;
	    g_exportItems[g_itemsPos].f             = ExportPKCS12;
	    ++g_itemsPos;
	  }
	}
      }      
      break;
      
    default:
      *st = ST_ERR;
      break;
      
    }
  }
  
}




int ExportPKCS12 ( int nItem )
{
  USBCERTS_HANDLE hClauer;
  unsigned char bCert[TAM_BLOQUE], bKey[TAM_BLOQUE], *pkcs12 = NULL;
  int keyFound = 0;
  unsigned long tamPkcs12;
  char passPkcs12[MAX_PASSPHRASE], passPkcs12Aux[MAX_PASSPHRASE];
  FILE *fp = NULL;
  long privKeyBlock;
  char aux[256];


  if ( ! *g_szClauerPass ) {
    
    if ( CLUTILS_AskPassphrase(_("Clauer's password: "), g_szClauerPass) != 0 ) {
      fprintf(stderr, _("[ERROR] Unable to get the password\n"));
      return 1;
    }
  }

  if ( LIBRT_IniciarDispositivo((unsigned char *) g_szDevice, g_szClauerPass, &hClauer) != 0 ) {
    fprintf(stderr, _("[ERROR] Unable to connect with the Clauer %s\n        Incorrect password?\n"), g_szDevice);
    return 1;
  }

  if ( LIBRT_LeerBloqueCrypto(&hClauer, g_exportItems[nItem].nb, bCert) != 0 ) {
    fprintf(stderr, _("[ERROR] Unable to get the block %ld\n"), g_exportItems[nItem].nb);
    LIBRT_FinalizarDispositivo(&hClauer);
    return 1;
  }

  if ( ! BLOQUE_Es_Claro(bCert) && (*(bCert+1) != BLOQUE_CERT_PROPIO) ) {
    fprintf(stderr, _("[ERROR] Block %ld does not contains a own certificate\n"), g_exportItems[nItem].nb);
    LIBRT_FinalizarDispositivo(&hClauer);
    return 1;
  }


  /* Buscamos llave privada asociada */

  if ( LIBRT_LeerTipoBloqueCrypto(&hClauer, BLOQUE_LLAVE_PRIVADA, 1, bKey, &privKeyBlock) != 0 ) {
      fprintf(stderr, _("[ERROR] Looking for the private key associated to the certificate %ld\n"), g_exportItems[nItem].nb);
    LIBRT_FinalizarDispositivo(&hClauer);
    return 1;
  }

  while ( ( ! keyFound ) && (privKeyBlock != -1) ) {

    if ( memcmp(BLOQUE_LLAVEPRIVADA_Get_Id(bKey),
		BLOQUE_CERTPROPIO_Get_Id(bCert),
		20) == 0 ) {

      keyFound = 1;
    } else {

      CLUTILS_Destroy((void *) bKey, TAM_BLOQUE);

      CLUTILS_Destroy(bKey, TAM_BLOQUE);
      if ( LIBRT_LeerTipoBloqueCrypto(&hClauer, BLOQUE_LLAVE_PRIVADA, 0, bKey, &privKeyBlock) != 0 ) {
	CLUTILS_Destroy(bKey, TAM_BLOQUE);
	fprintf(stderr, _("[ERROR] Reading from Clauer\n"));
	LIBRT_FinalizarDispositivo(&hClauer);
	return 1;
      }

    }

  }

  if ( LIBRT_FinalizarDispositivo(&hClauer) != 0 ) {
    CLUTILS_Destroy(bKey, TAM_BLOQUE);
    fprintf(stderr, _("[ERROR] Impossible to finalize device\n"));
    return 1;
  }

  if ( ! keyFound ) {
    fprintf(stderr, _("[ERROR] Associated private key not found\n"));
    return 1;
  }


  snprintf(aux, 256, _("pkcs12's password for %s: "), BLOQUE_CERTPROPIO_Get_FriendlyName(bCert));
  aux[255] = 0;

  CLUTILS_AskPassphrase(aux, passPkcs12);
  CLUTILS_AskPassphrase(_("Repeat the password: "), passPkcs12Aux);
  while (strncmp (passPkcs12, passPkcs12Aux, MAX_PASSPHRASE ) != 0 ){
      fprintf(stderr,_("The passwords do not  match, write them again!.\n"));
      CLUTILS_AskPassphrase(aux, passPkcs12);
      CLUTILS_AskPassphrase(_("Repeat the password: "), passPkcs12Aux);
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
    CLUTILS_Destroy(bKey, TAM_BLOQUE);
    fprintf(stderr, _("[ERROR] Unable to build up the pkcs12\n"));
    return 1;
  }

  pkcs12 = ( unsigned char * ) malloc ( tamPkcs12 );

  if ( ! pkcs12 ) {
    CLUTILS_Destroy(bKey, TAM_BLOQUE);
    fprintf(stderr, _("[ERROR] No memory available \n"));
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

    CLUTILS_Destroy(bKey, TAM_BLOQUE);
    free(pkcs12);
    fprintf(stderr,_("[ERROR] Unable to build up the pkcs12\n"));
    return 1;
  }

  CLUTILS_Destroy(bKey, TAM_BLOQUE);

  /* Escupimos por salida standard */

  if ( *(g_exportItems[nItem].szFileName) == 0 )
    fp = stdout;
  else {
    fp = fopen(g_exportItems[nItem].szFileName, "wb");
    if ( ! fp ) {
      free(pkcs12);
      fprintf(stderr, _("[ERROR] Unable to open/create the output file\n"));
      return 1;
    }
  }

  fwrite((const void *) pkcs12, tamPkcs12, 1, fp);

  if ( fp != stdout )
    fclose(fp);

  CLUTILS_Destroy(pkcs12, tamPkcs12);
  free(pkcs12);

  return 0;

}




int ExportCryptoZone ( int nItem )
{
  int  ret;
  clauer_handle_t hClauer;
  unsigned char block[BLOCK_SIZE];
  block_info_t ib;
  FILE *fp = NULL;
  unsigned long blocksToRead;


  ret = IO_Open((char*)g_szDevice, &hClauer, IO_RD, IO_CHECK_IS_CLAUER );
  switch ( ret ) {
  case IO_SUCCESS:
    break;

  case ERR_IO_NO_PERM:
    fprintf(stderr, _("[ERROR] You have to had permission (admin) to open the device to dump crypto zone\n"));
    return 1;

  default:
    fprintf(stderr, _("[ERROR] Unknown error opening the device\n"));
    return 1;
  }

  if ( IO_ReadInfoBlock(hClauer, &ib) != IO_SUCCESS ) {
    fprintf(stderr, _("[ERROR] Unable to get the information block\n"));
    return 1;
  }
  
  /* Calculo el total de bloques a leer: 1+res+objetos
   */
  
  blocksToRead = 1 + ib.rzSize;
  if ( ib.cb > -1 ){
    blocksToRead += ib.cb;
    ib.totalBlocks = ib.cb;
    ib.cb          = -1;
  } else 
    blocksToRead += ib.totalBlocks;
  
  /* Abrimos el fichero de salida
   */
  if ( *(g_exportItems[nItem].szFileName) ) {
    fp = fopen(g_exportItems[nItem].szFileName, "wb");
    if ( ! fp ) {
      fprintf(stderr, _("[ERROR] Unable to open/create the output file\n"));
      return 1;
    }
  } else
    fp = stdout;

  /* Escribimos el bloque de información
   */

  if ( fwrite(&ib, BLOCK_SIZE, 1, fp) == 0){
  	fprintf(stderr,_("[ERROR] Unable to write in the output file\n"));
	return 1;
  }
  while ( blocksToRead-- && ((ret = IO_Read(hClauer, block)) == IO_SUCCESS) )
    if ( fwrite(block, BLOCK_SIZE, 1, fp) == 0 ){
  	fprintf(stderr,_("[ERROR] Unable to write in the output file\n"));
	return 1;
    }

  if ( fp != stdout )
    fclose(fp);

  if ( ret != IO_SUCCESS )
    if ( ret != ERR_IO_EOF )
      fprintf(stderr, _("[ERROR] An error happened when dumping crypto partition\n        The dumped file countains an invalid partition\n"));
  
  return 0;

}






int ExportBlock ( int nItem ) 
{
  USBCERTS_HANDLE hClauer;
  unsigned char block[TAM_BLOQUE];
  unsigned char *obj;
  long objSize;

  FILE *fp;

  if ( ! *g_szClauerPass ) {    
    if ( CLUTILS_AskPassphrase(_("Clauer's password: "), g_szClauerPass) != 0 ) {
      fprintf(stderr, _("[ERROR] Unable to get the password\n"));
      return 1;
    }
  }
  

  if ( LIBRT_IniciarDispositivo((unsigned char *)g_szDevice, g_szClauerPass, &hClauer) != 0 ) {
      fprintf(stderr, _("[ERROR] Unable to connect with the Clauer %s\n        Incorrect password?\n"), g_szDevice);
    return 1;
  }

  if ( LIBRT_LeerBloqueCrypto(&hClauer, g_exportItems[nItem].nb, block) != 0 ) {
      fprintf(stderr, _("[ERROR] Unable to get the block %ld\n"), g_exportItems[nItem].nb);
      LIBRT_FinalizarDispositivo(&hClauer);
      return 1;
  }


  switch ( *(block+1) ) {

  case BLOQUE_CERT_PROPIO:
    obj     = BLOQUE_CERTPROPIO_Get_Objeto(block);
    objSize = BLOQUE_CERTPROPIO_Get_Tam(block);
    break;
  case BLOQUE_CERT_RAIZ:
    obj     = BLOQUE_CERTRAIZ_Get_Objeto(block);
    objSize = BLOQUE_CERTRAIZ_Get_Tam(block);
    break;
  case BLOQUE_CERT_OTROS:
    obj     = BLOQUE_CERTOTROS_Get_Objeto(block);
    objSize = BLOQUE_CERTOTROS_Get_Tam(block);
    break;
  case BLOQUE_CERT_INTERMEDIO:
    obj     = BLOQUE_CERTINTERMEDIO_Get_Objeto(block);
    objSize = BLOQUE_CERTINTERMEDIO_Get_Tam(block);
    break;
  case BLOQUE_LLAVE_PRIVADA:
    obj     = BLOQUE_LLAVEPRIVADA_Get_Objeto(block);
    objSize = BLOQUE_LLAVEPRIVADA_Get_Tam(block);
    break;
  case BLOQUE_PRIVKEY_BLOB:
    obj     = BLOQUE_LLAVEPRIVADA_Get_Objeto(block);
    objSize = BLOQUE_LLAVEPRIVADA_Get_Tam(block);
    break;

  case BLOQUE_CRYPTO_WALLET:
    obj     = block+8;
    objSize = TAM_BLOQUE-16;
    break;
    
  default:
    fprintf(stderr, _("[ERROR] Block %ld contains an unexportable object\n"), g_exportItems[nItem].nb);
    CLUTILS_Destroy(block, TAM_BLOQUE);
    return 1;
    break;
  }



  if ( *(g_exportItems[nItem].szFileName) == 0 )
    fp = stdout;
  else {
    fp = fopen(g_exportItems[nItem].szFileName, "ab");
    if ( ! fp ) {
      CLUTILS_Destroy(block, TAM_BLOQUE);
      fprintf(stderr, _("[ERROR] Unable to open/create the output file\n"));
      return 1;
    }
  }

  fwrite((const void *) obj, objSize, 1, fp);

  if ( fp != stdout )
    fclose(fp);

  return 0;
}







int main ( int argc, char **argv )
{
  char errMsg[256];
  int i;
  parse_st_t st;
  char *devs[MAX_DEVICES];
  int nDevs;
  USBCERTS_HANDLE hClauer;


#ifndef MAC
  nls_lang_init();	
#endif
  
  LIBRT_Ini();
  CRYPTO_Ini();

  parse(argc, argv, &st, errMsg);

  if ( st == ST_HELP ) {
    usage();
  } else if ( st == ST_LIST ) {
    CLUTILS_PrintClauers();
  } else if ( st == ST_ERR ) {
    usage();
  } else {
    
    if ( g_itemsPos == 0 ) {
      usage();
      return 0;
    }


    if ( ! *g_szDevice ) {
      
      /*LIBRT_RegenerarCache();*/

	if ( LIBRT_ListarDispositivos(&nDevs, (unsigned char **)devs) != 0 ) {
	fprintf(stderr, _("[ERROR] Listing devices\n"));
	return 1;
      }

      if ( nDevs == 0 ) {
	  fprintf(stderr,_("[ERROR] No Clauers detected on the system.\n"));
	  return 1;
      } else if ( nDevs > 1 ) {
	  printf(_("[WARNING] More than a Clauer plugged on the system.\n"
		   "           Use -d option\n"));
	  return 1;
      }

      strncpy(g_szDevice, devs[0], MAX_PATH_LEN);
      g_szDevice[MAX_PATH_LEN]= 0;

    }

    if ( LIBRT_IniciarDispositivo((unsigned char *) g_szDevice, NULL, &hClauer) != 0 ) {
      fprintf(stderr, _("[ERROR] Unable to connect with the Clauer %s\n        Correct device? Try to use -l option\n"), g_szDevice);
      return 1;
    }

    LIBRT_FinalizarDispositivo(&hClauer);
    

    for ( i = 0 ; i < g_itemsPos ; i++ ) {
      if ( g_exportItems[i].f(i) != 0 ) {
	CLUTILS_Destroy(g_szClauerPass, sizeof g_szClauerPass);
	return 1;
      }
    }

    CLUTILS_Destroy(g_szClauerPass, sizeof g_szClauerPass);

  }
  return 0;

}


