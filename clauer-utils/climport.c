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
#include <ctype.h>

#include <LIBIMPORT/LIBIMPORT.h>
#include <CRYPTOWrapper/CRYPTOWrap.h>
#include <LIBRT/libRT.h>

#include "misc.h"
#include "nls.h"
#include <getopt.h>


#define MAX_CERT_FILES      256
#define MAX_FILE_NAME       256
#define MAX_PKCS12_FILES    256

#ifdef WIN32
#define snprintf _snprintf
#endif

/* Prototipos de funciones de importación
 */

int  ImportPKCS12 ( void );
int  ImportCerts  ( void );
int  ImportTokens ( void );

/* Definición de tipo para punteros a 
 * funciones de importación
 */

typedef int (*IMPORT_FUNC) ( void );

/* Estados utilizados para parsear
 * la línea de comandos
 */

typedef enum {
    ST_INI,
    ST_ERR,
    ST_HELP,
    ST_LIST
} parse_t;

/* Estructura utilizada para la importación
 * de certificados sueltos
 */

typedef enum { CERT_CA, CERT_ROOT, CERT_OTHER } cert_type_t;
typedef struct {
    char szFileName[MAX_FILE_NAME+1];
    cert_type_t type;
} cert_info_t;

/* Vector global con punteros a las funcinoes
 * de importación que se emplerán. Cuando se
 * decida añadir nuevas funciones hay que
 * actualizar este vector
 */

IMPORT_FUNC g_pImportFuncs[] = { ImportPKCS12,
				 ImportCerts,
				 NULL };
  
/* Variables globales
 */

char g_szDevice[MAX_PATH_LEN+1];
char *g_pszPkcs12Files[MAX_PKCS12_FILES];
int g_p12Pos;
cert_info_t g_certFiles[MAX_CERT_FILES];
int g_certPos, g_mode= 0;

char g_szClauerPass[MAX_PASSPHRASE+1];
char g_szPkcs12Pass[MAX_PASSPHRASE+1];
char g_szExtraPass[MAX_PASSPHRASE+1];
int g_interactive= 1;

/* Imprime el uso del comando
 */

void usage ( void )
{
    fprintf(stderr, _("Usage mode: climport -h | -l | [-d device] [-p pass] [-x pass] [-m mode] ([-f pkcs12]* [-c cert]* [-o cert])*\n"));
    fprintf(stderr, _("Imports objects to the Clauer\n\n"));
    fprintf(stderr, _("Options are:\n"));
    fprintf(stderr, _("   -h, --help                Prints this help message\n"));
    fprintf(stderr, _("   -l, --list                List the clauers plugged on the system\n"));
    fprintf(stderr, _("   -d, --device   device     Selects the clauer to be used\n"));
    fprintf(stderr, _("   -p, --password password   Insecure mode of give the password, if not given, the password is asked with getpass.\n"));
    fprintf(stderr, _("   -i, --import-password     Password for the pkcs12 file\n"));
    fprintf(stderr, _("   -f, --fp12   file         imports a pkcs12 file\n"));
    fprintf(stderr, _("   -r, --root                imports a root certificate\n"));
    fprintf(stderr, _("   -c, --CA                  imports a intermediate CA certificate\n"));
    fprintf(stderr, _("   -o, --other               imports a other person certificate\n"));
    fprintf(stderr, _("   -m, --mode MODE           set the mode that the private keys must be stored\n"));
    fprintf(stderr, _("   -x, --extra-password password  The password that have to be used for modes 1 and 2\n"));
    fprintf(stderr, _("\nMODE for private keys could be:\n"));
    fprintf(stderr, _("    0  Default mode. Ciphered with the clauer's global password.\n"));
    fprintf(stderr, _("    1  Ciphered with a password different than the clauer's global password.\n"));
    fprintf(stderr, _("    2  Ciphered with clauer's global password and an extra password.\n"));
    fprintf(stderr, _("    3  No ciphering is applied, the private key is stored clear. (INSECURE) \n\n"));
}




void parse ( int argc, char **argv, parse_t *st, char errMsg[256] )
{
    int option_index = 0, aux;
    //int this_option_optind = optind ? optind : 1;
    static struct option long_options[] =
	{
	    {"help", 0, 0, 0},
	    {"device", 1, 0, 0},
	    {"list", 0, 0, 0},
	    {"p12", 1, 0, 0},
	    {"root", 1, 0, 0},
	    {"ca", 1, 0, 0},
	    {"other", 1, 0, 0},
	    {"password",1, 0, 0},
	    {"mode",1, 0, 0},
	    {"extra-password",1, 0, 0},
	    {"import-password",1, 0, 0},
	    {0,0,0,0}
	};

    int c;

    /* Inicializamos las variables globales
     */

    memset(g_pszPkcs12Files, 0, sizeof g_pszPkcs12Files);
    *g_szDevice = 0;
    g_p12Pos = 0;
    memset(g_certFiles, 0, sizeof g_certFiles);
    g_certPos = 0;

    *g_szClauerPass= 0;
    *g_szPkcs12Pass= 0;
  
    /* Empezamos a parsear
     */

    *st = ST_INI;

    while ( *st != ST_ERR ) {

	c = getopt_long(argc, argv, "hld:f:i:p:r:c:o:m:x:", long_options, &option_index);
	if ( c == -1 )
	    break;

	switch ( c ) {
	case 0:
	    if ( strcmp(long_options[option_index].name, "help") == 0 ) {
		if ( *st == ST_LIST || *st == ST_HELP )
		    *st = ST_ERR;
		else
		    *st = ST_HELP;
	    } else if ( strcmp(long_options[option_index].name, "device") == 0 ) {
		if ( *st == ST_LIST || *st == ST_HELP )
		    *st = ST_ERR;
		else {
		    strncpy(g_szDevice, optarg, MAX_PATH_LEN);	
		    *st = ST_INI;
		}
	    } else if ( strcmp(long_options[option_index].name, "list") == 0 ) {
		if ( *st == ST_LIST || *st == ST_HELP )
		    *st = ST_ERR;
		else
		    *st = ST_LIST;
	    } else if ( strcmp(long_options[option_index].name, "fp12") == 0 ) {
		if ( *st == ST_LIST || *st == ST_HELP )
		    *st = ST_ERR;
		else {
		    if ( g_p12Pos == MAX_PKCS12_FILES ) {
			fprintf(stderr, _("[WARNING] Maximum number of pkcs12 exceeded. Next will be ignored\n"));
		    } else {
			g_pszPkcs12Files[g_p12Pos] = ( char * ) malloc ( MAX_FILE_NAME + 1 );
			if ( ! g_pszPkcs12Files[g_p12Pos] ) {
			    fprintf(stderr, _("[ERROR] No memory available \n"));
			    *st = ST_ERR;
			    return;
			}
			strncpy(g_pszPkcs12Files[g_p12Pos], optarg, MAX_FILE_NAME);
			g_pszPkcs12Files[g_p12Pos][MAX_FILE_NAME] = 0;
			g_p12Pos++; 
		    }	  
		}	

	    } else if ( strcmp(long_options[option_index].name, "password") == 0 ) {
		if ( *st == ST_LIST || *st == ST_HELP )
		    *st = ST_ERR;
		else{
		    aux= strlen(optarg);
		    if ( aux >= MAX_PASSPHRASE){
			*st= ST_ERR;
			fprintf(stderr, _("The password is longer than 127 characters\n"));
		    }
		    strncpy(g_szClauerPass, optarg, strlen(optarg)+1);
		}	
	    } else if ( strcmp(long_options[option_index].name, "import-password") == 0 ) {
		if ( *st == ST_LIST || *st == ST_HELP )
		    *st = ST_ERR;
		else{
		    aux= strlen(optarg);
		    if ( aux >= MAX_PASSPHRASE){
			*st= ST_ERR;
			fprintf(stderr, _("The password is longer than 127 characters\n"));
		    }
		    strncpy(g_szPkcs12Pass, optarg, strlen(optarg)+1);
		}					
	    } else if ( strcmp(long_options[option_index].name, "root") == 0 ) {
		if ( *st == ST_LIST || *st == ST_HELP )
		    *st = ST_ERR;
		else {
	  
		    if ( g_certPos == MAX_CERT_FILES ) 
			fprintf(stderr, _("[WARNING] Maximum number of certificates exceeded. Next will be ignored\n"));
		    else {
			strncpy(g_certFiles[g_certPos].szFileName, optarg, MAX_FILE_NAME);
			g_certFiles[g_certPos].szFileName[MAX_FILE_NAME] = 0;
			g_certFiles[g_certPos].type = CERT_ROOT;
			g_certPos++;
		    }
		}
	    } else if ( strcmp(long_options[option_index].name, "ca") == 0 ) {
		if ( *st == ST_LIST || *st == ST_HELP )
		    *st = ST_ERR;
		else {
	  
		    if ( g_certPos == MAX_CERT_FILES ) 
			fprintf(stderr, _("[WARNING] Maximum number of certificates exceeded. Next will be ignored\n"));
		    else {
			strncpy(g_certFiles[g_certPos].szFileName, optarg, MAX_FILE_NAME);
			g_certFiles[g_certPos].szFileName[MAX_FILE_NAME] = 0;
			g_certFiles[g_certPos].type = CERT_CA;
			g_certPos++;
		    }
		}
	    }
	    else if ( strcmp(long_options[option_index].name, "extra-password") == 0 ) {
		if ( *st == ST_LIST || *st == ST_HELP )
		    *st = ST_ERR;
		else{
		    aux= strlen(optarg);
		    if ( aux >= MAX_PASSPHRASE){
			*st= ST_ERR;
			fprintf(stderr, _("The password is longer than 127 characters\n"));
		    }
		    strncpy(g_szExtraPass, optarg, strlen(optarg)+1);
		}		  	  
	    }            
	    else if ( strcmp(long_options[option_index].name, "mode") == 0 ) {
		if ( *st == ST_LIST || *st == ST_HELP )
		    *st = ST_ERR;
		else {
		    g_mode= atoi(optarg);
		    if ( g_mode < 0 || g_mode > 3 ){
			*st = ST_ERR;
		    }   
		}	  	  
	    }  else if ( strcmp(long_options[option_index].name, "other") == 0 ) {
		if ( *st == ST_LIST || *st == ST_HELP )
		    *st = ST_ERR;
		else {	  
		    if ( g_certPos == MAX_CERT_FILES ) 
			fprintf(stderr, _("[WARNING] Maximum number of certificates exceeded. Next will be ignored\n"));
		    else {
			strncpy(g_certFiles[g_certPos].szFileName, optarg, MAX_FILE_NAME);
			g_certFiles[g_certPos].szFileName[MAX_FILE_NAME] = 0;
			g_certFiles[g_certPos].type = CERT_OTHER;
			g_certPos++;
		    }
		}
	    } else 
		*st = ST_ERR;

	    break;

	case 'c':

	    if ( *st == ST_LIST || *st == ST_HELP )
		*st = ST_ERR;
	    else {
	
		if ( g_certPos == MAX_CERT_FILES ) 
		    fprintf(stderr, _("[WARNING] Maximum number of certificates exceeded. Next will be ignored\n"));
		else {
		    strncpy(g_certFiles[g_certPos].szFileName, optarg, MAX_FILE_NAME);
		    g_certFiles[g_certPos].szFileName[MAX_FILE_NAME] = 0;
		    g_certFiles[g_certPos].type = CERT_CA;
		    g_certPos++;
		}
	    }
	    break;

	case 'r':
	    if ( *st == ST_LIST || *st == ST_HELP )
		*st = ST_ERR;
	    else {
	
		if ( g_certPos == MAX_CERT_FILES ) 
		    fprintf(stderr, _("[WARNING] Maximum number of certificates exceeded. Next will be ignored\n"));
		else {
		    strncpy(g_certFiles[g_certPos].szFileName, optarg, MAX_FILE_NAME);
		    g_certFiles[g_certPos].szFileName[MAX_FILE_NAME] = 0;
		    g_certFiles[g_certPos].type = CERT_ROOT;
		    g_certPos++;
		}
	    }      
	    break;

	case 'o':
	    if ( *st == ST_LIST || *st == ST_HELP )
		*st = ST_ERR;
	    else {
	
		if ( g_certPos == MAX_CERT_FILES ) 
		    fprintf(stderr, _("[WARNING] Maximum number of certificates exceeded. Next will be ignored\n"));
		else {
		    strncpy(g_certFiles[g_certPos].szFileName, optarg, MAX_FILE_NAME);
		    g_certFiles[g_certPos].szFileName[MAX_FILE_NAME] = 0;
		    g_certFiles[g_certPos].type = CERT_OTHER;
		    g_certPos++;
		}
	    }
      
	    break;

	case 'h':
	    if ( *st == ST_LIST || *st == ST_HELP )
		*st = ST_ERR;
	    else
		*st = ST_HELP;
	    break;
      
	case 'l':
	    if ( *st == ST_LIST || *st == ST_HELP )
		*st = ST_ERR;
	    else
		*st = ST_LIST;
	    break;

	case 'd':
	    strncpy(g_szDevice, optarg, MAX_PATH_LEN);
	    *st = ST_INI;
	    break;

	case 'i':
	    if ( *st == ST_LIST || *st == ST_HELP )
		*st = ST_ERR;
	    else{
		aux= strlen(optarg);
		if ( aux >= MAX_PASSPHRASE){
		    *st= ST_ERR;
		    fprintf(stderr,  _("The password is longer than 127 characters\n"));
		}
		strncpy(g_szPkcs12Pass, optarg, strlen(optarg)+1);
	    }
	    break;

	case 'x':
	    if ( *st == ST_LIST || *st == ST_HELP )
		*st = ST_ERR;
	    else{
		aux= strlen(optarg);
		if ( aux >= MAX_PASSPHRASE){
		    *st= ST_ERR;
		    fprintf(stderr,  _("The password is longer than 127 characters\n"));
		    break;
		}
		strncpy(g_szExtraPass, optarg, strlen(optarg)+1);
	    }
	    break;
	       
	case 'p':
	    if ( *st == ST_LIST || *st == ST_HELP )
		*st = ST_ERR;
	    else{
		aux= strlen(optarg);
		if ( aux >= MAX_PASSPHRASE){
		    *st= ST_ERR;
		    fprintf(stderr,  _("The password is longer than 127 characters\n"));
		    break;
		}
		strncpy(g_szClauerPass, optarg, strlen(optarg)+1);
	    }
	    break;

	case 'm':
	    if ( *st == ST_LIST || *st == ST_HELP )
		*st = ST_ERR;
	    else{
		g_mode= atoi(optarg);
		if ( g_mode < 0 || g_mode > 3 ){
		    *st = ST_ERR;
		}   
	    }
	    break;
	
	case 'f':

	    if ( *st == ST_LIST || *st == ST_HELP )
		*st = ST_ERR;
	    else {
		if ( g_p12Pos == MAX_PKCS12_FILES ) {
		    fprintf(stderr, _("[WARNING] Maximum number of pkcs12 exceeded. Next will be ignored\n"));
		} else {

		    g_pszPkcs12Files[g_p12Pos] = ( char * ) malloc ( MAX_FILE_NAME + 1 );

		    if ( ! g_pszPkcs12Files[g_p12Pos] ) {
			fprintf(stderr, _("[ERROR] No memory available \n"));
			*st = ST_ERR;
			return;
		    }
		    strncpy(g_pszPkcs12Files[g_p12Pos], optarg, MAX_FILE_NAME);
		    g_pszPkcs12Files[g_p12Pos][MAX_FILE_NAME] = 0;
		    g_p12Pos++;
		}	  
	    }

	    break;
      
	default:
	    *st = ST_ERR;
	    break;

	}

    }


    if ( optind < argc ) {
	*st = ST_ERR;
	snprintf(errMsg, 256, _("Elements that are no options: %s"), argv[optind]);
    }

}






/* Importa una lista de PKCS12s
 */

int ImportPKCS12 ( void )
{
    USBCERTS_HANDLE hClauer;
    // char szPkcs12Pass[MAX_PASSPHRASE];
    FILE *fp;

    int i;
    char szAux[70], res;

    if ( g_p12Pos == 0 ) {
	return 0; // Must not return err because it means that pkcs12 is not indicated. 
    }
    printf(_("Importing PKCS12...\n"));
    fflush(stdout);

    if ( ! *g_szClauerPass ) {

	if ( CLUTILS_AskPassphrase(_("\tClauer's password: "), g_szClauerPass) != 0 ) {
	    fprintf(stderr, _("[ERROR] Unable to get the password\n"));
	    return 1;
	}
    
    }

    if ( LIBRT_IniciarDispositivo((unsigned char *) g_szDevice, g_szClauerPass, &hClauer) != 0 ) {
	fprintf(stderr, _("[ERROR] Unable to connect with the Clauer %s\n        Incorrect password?\n"), g_szDevice);            
	return 1;
    }
    LIBRT_FinalizarDispositivo(&hClauer);

    if ( *g_szPkcs12Pass && g_p12Pos!=1  ){
	fprintf(stdout,_("[WARNING] You have given more than a pkcs12 file with --import-password option or -i\n           the password for all the pkcs12 files must be the same if this option is indicated\n."));
    }
  
    for ( i = 0 ; i < g_p12Pos ; i++ ) {

	fp= fopen(g_pszPkcs12Files[i], "rb");
	if ( !fp ){
	    fprintf(stderr, _("[ERROR] Unable to open/create the output file %s\n"), g_pszPkcs12Files[i]);
	    return 1;
	}
	fclose(fp);
    	
	if ( ! *g_szPkcs12Pass ){
	    snprintf(szAux, 70, _("\tType the password for %s:"), g_pszPkcs12Files[i]);
	    szAux[69] = 0;
	    if ( CLUTILS_AskPassphrase ( szAux, g_szPkcs12Pass ) != 0 ){
		fprintf(stderr, _("[ERROR] Unable to get the password\n"));	  
		return 1;
	    }
	}    
    
	if ( g_mode == 1 || g_mode == 2 ){
	    if ( ! *g_szExtraPass ){
		snprintf(szAux, 70, _("\tType the extra password for the private key:"));
		szAux[69] = 0;
		if ( CLUTILS_AskPassphrase ( szAux, g_szExtraPass ) != 0 ){
		    fprintf(stderr, _("[ERROR] Unable to get the password\n"));	  
		    return 1;
		}
	    }
	
	}
   
	if ( g_interactive ){
	    if ( g_mode == PRIVKEY_NO_CIPHER ){
		fprintf(stdout, "[WARNING] The ciphering mode you are using leave your private keys exposed.\n");
		fprintf(stdout, "          Are you sure you want that ciphering mode? y/n ");
	    
		scanf("%c",&res);
	    	    
		if (tolower(res) != 'y'){
		    exit(0);
		}	    	    
	    }
	}  

	if ( LIBRT_IniciarDispositivo((unsigned char *) g_szDevice, g_szClauerPass, &hClauer) != 0 ) {
	    fprintf(stderr, _("[ERROR] Unable to connect with the Clauer %s\n        Incorrect password?\n"), g_szDevice);
	    return 1;
	}

	if ( LIBIMPORT_ImportarPKCS12 (g_pszPkcs12Files[i], g_szPkcs12Pass, &hClauer, g_mode, g_szExtraPass ) == 0 ){
	    LIBRT_FinalizarDispositivo(&hClauer);
	    fprintf(stderr, _("[ERROR] Unable to import the certificate, incorrect password?\n"));
	    return 1;
	}
    
	if ( LIBRT_FinalizarDispositivo(&hClauer) != 0 ) {
	    fprintf(stderr, _("[ERROR] Closing Clauer's connectiono\n"));
	    return 1;
	}
    
	if ( ! *g_szPkcs12Pass )
	    CLUTILS_Destroy((void *) g_szPkcs12Pass, MAX_PASSPHRASE);
    }
  
    return 0;
}


/* Importa certificados
 */

int ImportCerts ( void )
{
    int i;
    USBCERTS_HANDLE hClauer;
    unsigned char block[TAM_BLOQUE];
    long nb;
  
    FILE *fp;
    unsigned char *b;
    long fileSize;

    if ( g_certPos == 0 ) {
	return 0;
    }
    printf(_("Importing Certificates...\n"));
  
    fflush(stdout);

    if ( ! *g_szClauerPass ) {
	if ( CLUTILS_AskPassphrase(_("\tClauer's password: "), g_szClauerPass) != 0 ) {
	    fprintf(stderr, _("[ERROR] Unable to get the password\n"));
	    return 1;
	}
    }
    
    if ( LIBRT_IniciarDispositivo((unsigned char *) g_szDevice, g_szClauerPass, &hClauer) != 0 ) {
	fprintf(stderr, _("[ERROR] Unable to connect with the Clauer %s\n        Incorrect password?\n"), g_szDevice);      
	return 1;
    }

    for ( i = 0 ; i < g_certPos ; i++ ) {

	fp = fopen(g_certFiles[i].szFileName, "rb");
	if ( ! fp ) {
	    fprintf(stderr, _("\t[ERROR] Unable to open %s\n"), g_certFiles[i].szFileName);
	    continue;
	}
	if ( fseek(fp, 0, SEEK_END) == -1 ) {
	    fprintf(stderr, _("\t[ERROR] Unable to guess the size of the file %s\n"), g_certFiles[i].szFileName);
	    fclose(fp);
	    continue;
	}
	fileSize = ftell(fp);
	if ( fileSize == -1 ) {
	    fprintf(stderr, _("\t[ERROR] Unable to guess the size of the file %s\n"), g_certFiles[i].szFileName);
	    fclose(fp);
	    continue;
	}
	if ( fseek(fp, 0, SEEK_SET) == -1 ) {
	    fprintf(stderr, _("\t[ERROR] Unable to guess the size of the file %s\n"), g_certFiles[i].szFileName);
	    fclose(fp);
	    continue;
	}
	b = (unsigned char * ) malloc ( fileSize );
	if ( ! b ) {
	    fprintf(stderr,  _("[ERROR] No memory available \n"));
	    fclose(fp);
	    LIBRT_FinalizarDispositivo(&hClauer);
	    return 1;
	}
	fread(b, fileSize, 1, fp);
	if ( ferror(fp) ) {
	    fprintf(stderr, _("[ERROR] Reading the file %s\n"), g_certFiles[i].szFileName);
	    fclose(fp);
	    free(b);
	    continue;
	}
	fclose(fp);

	memset(block, 0, sizeof block);
	BLOQUE_Set_Claro(block);
	if ( g_certFiles[i].type == CERT_ROOT ) {
	    BLOQUE_CERTRAIZ_Nuevo(block);
	    BLOQUE_CERTRAIZ_Set_Tam(block, fileSize);
	    BLOQUE_CERTRAIZ_Set_Objeto(block, b, fileSize);
	} else if ( g_certFiles[i].type == CERT_CA ) {
	    BLOQUE_CERTINTERMEDIO_Nuevo(block);
	    BLOQUE_CERTINTERMEDIO_Set_Tam(block, fileSize);
	    BLOQUE_CERTINTERMEDIO_Set_Objeto(block, b, fileSize);
	} else if ( g_certFiles[i].type == CERT_OTHER ) {
	    BLOQUE_CERTOTROS_Nuevo(block);
	    BLOQUE_CERTOTROS_Set_Tam(block, fileSize);
	    BLOQUE_CERTOTROS_Set_Objeto(block, b, fileSize);
	}

	free(b);
    
	if ( LIBRT_InsertarBloqueCrypto(&hClauer, block, &nb) != 0 ) {
	    fprintf(stderr, _("\t[ERROR] Inserting the certificate: %s\n"), g_certFiles[i].szFileName);
	    return 1;
	}
    }

    return 0;
  
}




int main ( int argc, char **argv )
{

    parse_t st;
    char errMsg[256];
    int nDevs, i, err;
    unsigned char *devs[MAX_DEVICES];
    USBCERTS_HANDLE hClauer;
  
#ifndef MAC
    nls_lang_init();	
#endif
  
    LIBRT_Ini();
    CRYPTO_Ini();

    parse(argc, argv, &st, errMsg);
    if ( st == ST_ERR ) {
	usage();
	return 1;
    } else if ( st == ST_HELP ) {
	usage();
	return 1;
    } else if ( st == ST_LIST ) {    
	err = !CLUTILS_PrintClauers();
    } else {

	if ( !g_p12Pos && !g_certPos ) {
	    usage();
	    return 0;
	}

    
	if ( ! g_szDevice[0] ) {
	    LIBRT_RegenerarCache();	
	    if ( LIBRT_ListarDispositivos(&nDevs, devs) != 0 ) {
		fprintf(stderr, _("[ERROR] Listing devices\n"));
		return 1;
	    }

	    if ( nDevs > 1 ) {
		printf(_("[WARNING] More than a Clauer plugged on the system.\n"
			 "           Use -d option\n"));
		return 1;
	    } else if ( nDevs == 0 ) {
		fprintf(stderr, _("[ERROR] No Clauers detected on the system.\n"));
		return 1;
	    }

	    strncpy(g_szDevice, (char *) devs[0], MAX_PATH_LEN);
	    g_szDevice[MAX_PATH_LEN] = 0;

	    for ( i = 0 ; i < nDevs ; i++ )
		free(devs[i]);
	}
    
	if ( LIBRT_IniciarDispositivo((unsigned char *) g_szDevice, NULL, &hClauer) != 0 ) {
	    fprintf(stderr, _("[ERROR] Unable to connect with the Clauer %s\n        Correct device? Try to use -l option\n"), g_szDevice);
	    return 1;
	}
	LIBRT_FinalizarDispositivo(&hClauer);

	/* Empezamos a importar objetos 
	 */

	i = 0;
	while ( *(g_pImportFuncs+i) ) {
	    //	printf ("Entro función \n");
	    err= (*(g_pImportFuncs+i))();
	    if (err == 1)
		return err;
	    ++i;
	}
    
	CLUTILS_Destroy(g_szClauerPass, sizeof g_szClauerPass);    

    }

    return 0;


}


