
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

#ifndef WIN32
#include <CRYPTOWrapper/CRYPTOWrap.h>
#endif

#include <LIBRT/libRT.h>

#include "misc.h"
#include "nls.h" 


#ifdef LINUX
#include <errno.h>
#include "misc.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#elif defined(WIN32)

/* En windows parece que no tenemos snprintf()
 */
#define snprintf _snprintf

#endif

#include <ctype.h>

#define MAX_DEVICE_LEN	256
#define MAX_CONT_LEN	128


typedef enum { 
    ST_INI,
    ST_HELP,
    ST_LIST,
    ST_DEV_NAME,
    ST_DEV,
    ST_GETNUMBLOCK,
    ST_GETNUMCERT,
    ST_PASSWORD,
    ST_KEYCONT,
    ST_OK,
    ST_ERR
} parse_st_t;

int block_num= -1, cert_num= -1, interactive= 1;


char g_device[MAX_PATH_LEN+1], * password;   /* Dispositivo seleccionado */
char g_keycontname[MAX_CONT_LEN+1];



void usage ( void )
{ 
    fprintf(stderr, _("Usage mode: cldel [-h] | [-l] |[-y] [-d device] [-p password]  ( -b NUM | -c NUM | -k name) \n"));
    fprintf(stderr, _("Delete blocks from Clauer\n\n"));
    fprintf(stderr, _("Options are:\n"));
    fprintf(stderr, _("   -h, --help               Prints this help message\n"));
    fprintf(stderr, _("   -l, --list               List the clauers plugged on the system\n"));
    fprintf(stderr, _("   -y, --yes                No ask for confirmation when deleting\n"));
    fprintf(stderr, _("   -d, --device DEVICE      Selects the clauer to work with\n"));
    fprintf(stderr, _("   -b, --block NUM          Deletes the block number NUM from Clauer.\n"));
    fprintf(stderr, _("   -k, --key-container name Deletes the entry with that name on clauers key container block.\n"));
    fprintf(stderr, _("   -c, --certificate NUM    Deletes the certificate and their associated blocks in the Clauer\n"));
    fprintf(stderr, _("   -p, --password password  Insecure mode of give the password, if not given, the password is asked with getpass.\n"));
    fprintf(stderr, _("\nThe identifier NUM is the first number than appears when executing clls\n\n"));
    
}



void parse ( int argc, char **argv, parse_st_t *st, char errMsg[256] )
{
    int op, aux;

    *g_device = 0;
    *g_keycontname= 0;
  
    *st = ST_INI;
  
    for ( op = 1 ; (op < argc) && (*st != ST_ERR) ; op++ ) {
	
	if ( *st == ST_INI ) {
	    
	    if (  ( strcmp(argv[op], "-h") == 0 ) ||
		  ( strcmp(argv[op], "--help") == 0 ) ) {
		
		*st = ST_HELP;
		
	    } 
	    else if ( ( strcmp(argv[op], "-b") == 0 ) ||
		      ( strcmp(argv[op], "--block") == 0 ) ) {
		
		*st = ST_GETNUMBLOCK;
	    } 
	    else if ( ( strcmp(argv[op], "-c") == 0 ) || strcmp(argv[op], "--certificate") == 0 ){
		*st = ST_GETNUMCERT;
	    }
	    else if ( ( strcmp(argv[op], "-l") == 0 ) || strcmp(argv[op], "--list") == 0 ) {
		*st = ST_LIST;
	    } 
	    else if ( ( strcmp(argv[op], "-d") == 0 ) || strcmp(argv[op], "--device") == 0 ) {
		*st = ST_DEV_NAME;
	    } 
	    else if ( ( strcmp(argv[op], "-y") == 0 ) || strcmp(argv[op], "--yes") == 0 ){
		interactive= 0;	 
	    } else if ( ( strcmp(argv[op], "-p") == 0 ) || strcmp(argv[op], "--password") == 0) {
		*st= ST_PASSWORD;      
	    } else if ( ( strcmp(argv[op], "-k") == 0 ) || strcmp(argv[op], "--key-container") == 0) {
		*st= ST_KEYCONT;      
	    }
	    else {
		snprintf(errMsg, 256, _("Unknown option: %s"), argv[op]);
		*st = ST_ERR;
	    }	    
	} 
	else if ( *st == ST_DEV_NAME ) {
	    
	    if ( *g_device ) {
		snprintf(errMsg, 256, _("You can only select one device"));
		*st = ST_ERR;
	    } 
	    else {
		strncpy(g_device, argv[op], MAX_PATH_LEN);
		g_device[MAX_PATH_LEN] = 0;
		*st = ST_INI;
	    }
	    
	} 
	else if ( *st == ST_KEYCONT ) {
	    
	    if ( *g_keycontname ) {
		snprintf(errMsg, 256, _("You can only select one key container to delete"));
		*st = ST_ERR;
	    } 
	    else {
		strncpy(g_keycontname, argv[op], MAX_CONT_LEN);
		g_device[MAX_CONT_LEN] = 0;
		*st = ST_INI;
	    }
	    
	} 
	
	else if ( *st == ST_HELP ) {
	    snprintf(errMsg, 256, _("No more options allowed when -h or --help are given\n"));
	    *st = ST_ERR;
	    
	} 
        else if (*st == ST_PASSWORD ){
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
		  password= (char *) malloc((sizeof(char) * aux)+1 );
		    strncpy(password, argv[op], aux+1);
		    *st = ST_INI;
		}
	    }	   
	}
	else if ( *st == ST_GETNUMBLOCK){
	    if ( cert_num != -1  ){
		snprintf(errMsg, 256, _("-b o --block must be given alone"));
		*st = ST_ERR;
	    }
	    else{
		block_num= atoi(argv[op]);
		*st = ST_INI;
	    }
	} 
	else if (*st == ST_GETNUMCERT ) {
	    if ( block_num != -1 ){
		snprintf(errMsg, 256, _("-c o --certfiicate must be given alone"));
		*st = ST_ERR;
	    }
	    else{
		cert_num= atoi(argv[op]);
		*st = ST_INI;
	    }
	}
	else if ( *st == ST_LIST ) {
	    snprintf(errMsg, 256,  _("-l o --list must be given alone. Invalid option: %s"), argv[op]);
	    *st = ST_ERR;
	}
    }
    
    if ( *st == ST_INI )
	*st = ST_OK;
    
    /* Si hemos acabado en un estado no final, entonces
     * error
     */
    
    if ( *st == ST_GETNUMBLOCK ) {
	snprintf(errMsg, 256, _("No block to delete specified"));
	*st = ST_ERR;
    }
    else if ( *st == ST_GETNUMCERT ){
	snprintf(errMsg, 256, _("No certificate to delete specified"));
	*st = ST_ERR;
    }
    else if ( *st == ST_DEV_NAME ) {
	snprintf(errMsg, 256, _("No device name specified"));
	*st = ST_ERR;
    }
}


int deleteBlock()
{
    USBCERTS_HANDLE h;
    int nDisp, i;
    unsigned char *devs[MAX_DEVICES], aux='k', nl;
    char pass[MAX_PASSPHRASE];
    unsigned char  bloque[TAM_BLOQUE];
    

    if ( ! *g_device ) {
	
	if ( LIBRT_ListarDispositivos(&nDisp, devs) != 0 ) {
	    fprintf(stderr, _("[ERROR] Unable to get Clauers on the system.\n"));
	    return 1;
	}
	
	if ( nDisp > 1 ) {
	    printf(_("[WARNING] More than a Clauer plugged on the system.\n"
		     "           Use -d option\n"));
	    for ( i = 0 ; i < nDisp ; i++ )
		free(devs[i]);
	    return 0;
	    
	} else if ( nDisp == 0 ) {
	    printf(_("[WARNING] No Clauers detected on the system.\n"));
	    for ( i = 0 ; i < nDisp ; i++ )
		free(devs[i]);
	    return 0;
	}
	
	strcpy(g_device, (char *)devs[0]);
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
	CLUTILS_Destroy(password, strlen(password)+1);
    }
    
    if ( CLUTILS_ConnectEx2(&h, pass, g_device) != 0 ) {
	CLUTILS_Destroy(pass, strlen(pass));
	return 1;
    }
    
    CLUTILS_Destroy(pass, strlen(pass));

    
    if ( interactive ){

	if ( LIBRT_LeerBloqueCrypto( &h, block_num, bloque) != 0) {
	    fprintf(stderr, _("[ERROR] Unable to read the specified block"));
	    return 1;
	}
	
	switch ( bloque[1] ) {
	
	case BLOQUE_CERT_PROPIO:
	    printf(_("You are going to delete the block that contains:\n\t"));
	    CLUTILS_PrintCert(block_num, bloque, BLOQUE_CERT_PROPIO);
	    break;
	    
	case BLOQUE_CERT_WEB:
	    printf(_("You are going to delete the block that contains:\n\t"));
	    CLUTILS_PrintCert(block_num, bloque, BLOQUE_CERT_WEB );
	    break;
	
	case BLOQUE_CERT_RAIZ:
	    printf(_("You are going to delete the block that contains:\n\t"));
	    CLUTILS_PrintCert(block_num, bloque,BLOQUE_CERT_RAIZ );
	    break;
	    
	case BLOQUE_CERT_INTERMEDIO:
	    printf(_("You are going to delete the block that contains:\n\t"));
	    CLUTILS_PrintCert(block_num, bloque, BLOQUE_CERT_INTERMEDIO );
	    break;
	    
	case BLOQUE_LLAVE_PRIVADA:
	    printf(_("You are going to delete the block that contains:\n\t"));
	    CLUTILS_PrintLlave(block_num, bloque);
	    break;
	    
	case BLOQUE_PRIVKEY_BLOB:
	    printf(_("You are going to delete the block that contains:\n\t"));
	    CLUTILS_PrintBlob(&h, block_num, bloque);
	    break;
	
	case BLOQUE_KEY_CONTAINERS:
	    printf(_("You are going to delete the block that contains:\n\t"));
	    CLUTILS_PrintContainer(block_num, bloque);
	
	case 0:
	    printf(_("The block is empty!, exiting ...\n"));
	    return 1;
	
	default: 
	    printf( _("[WARNING] Unknown block type\n") );
	    break;
	
	}
	
	while ( tolower(aux) != 'y' &&  tolower(aux) != 'n' ){
	    printf(_("Are you sure (y/n)? "));
	    scanf("%c",&aux);
	    scanf("%c",&nl);
	}
	if ( tolower(aux)== 'n') 
	    return 0; 
    }
    
    
    if ( LIBRT_BorrarBloqueCrypto ( &h, block_num ) != 0 ) { 
	fprintf(stderr, _("[ERROR] Deleting block = %d "),block_num);
	return 1;
    }
 
    printf(_("Block %d successfully deleted\n"), block_num);
    
    return 0;
}


int deleteContainerEntry()
{
    
    USBCERTS_HANDLE h;
    int nDisp, i, j, rewrite=0, func=1;
    long nBlock=0;
    unsigned char *devs[MAX_DEVICES], block[TAM_BLOQUE];
    char pass[MAX_PASSPHRASE];


    unsigned int lstContainerSize = NUM_KEY_CONTAINERS;
    INFO_KEY_CONTAINER lstContainer[NUM_KEY_CONTAINERS];
    
    if ( ! *g_keycontname ){
	return 1;
    }


    
    /* Listamos dispositivos */ 
    if ( ! *g_device ) {
	if ( LIBRT_ListarDispositivos(&nDisp, devs) != 0 ) {
	    fprintf(stderr, _("[ERROR] Unable to get Clauers on the system.\n"));
	    return 1;
	}
	
	if ( nDisp > 1 ) {
	    printf(_("[WARNING] More than a Clauer plugged on the system.\n"
		     "           Use -d option\n"));
	    for ( i = 0 ; i < nDisp ; i++ )
		free(devs[i]);
	    return 1;
	    
	} else if ( nDisp == 0 ) {
	    printf(_("[WARNING] No Clauers detected on the system.\n"));
	    for ( i = 0 ; i < nDisp ; i++ )
		free(devs[i]);
	    return 1;
	}
	
	strcpy(g_device, (char *)devs[0]);
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
	CLUTILS_Destroy(password, strlen(password)+1);
    }
    
    if ( CLUTILS_ConnectEx2(&h, pass, g_device) != 0 ) {
	CLUTILS_Destroy(pass, strlen(pass));
	return 1;
    }
    
    CLUTILS_Destroy(pass, strlen(pass));
    
       
    while ( ( LIBRT_LeerTipoBloqueCrypto ( &h, 
					   BLOQUE_KEY_CONTAINERS, 
					   func, block, 
					   &nBlock) != ERR_LIBRT_SI) 
	    && ( nBlock != -1 ) ) {
	
	
	rewrite= 0; func=0;
	if ( BLOQUE_KeyContainer_Enumerar(block, lstContainer, &lstContainerSize) != 0 ) {
	    printf(_("[ERROR] Enumerating key Containers\n"));
	    break;
	}
	for ( j = 0 ; j < lstContainerSize ; j++ ) {
	    if ( strncmp(lstContainer[j].nombreKeyContainer, g_keycontname, strlen(g_keycontname)) == 0 ) {		  
		if ( BLOQUE_KeyContainer_Borrar ( block , lstContainer[j].nombreKeyContainer ) != 0 ){
		    fprintf(stderr, _("[ERROR] Unable to delete the associated key entry on the key container\n"));
		    return 1;
		}
		else {
		    if (interactive ) 
			printf(_("Container %s successfully deleted\n"),lstContainer[j].nombreKeyContainer );
		    /* Es necesario reescribir el bloque en el clauer */
		    rewrite= 1; 
		}
	    }
	    
	    if ( rewrite == 1 ){
		if ( BLOQUE_KeyContainer_Enumerar(block, lstContainer, &lstContainerSize) != 0 ) {
		    printf(_("[ERROR] Enumerating key Containers\n"));
		    break;
		}
		
		if ( lstContainerSize == 0 ){
		    if ( LIBRT_BorrarBloqueCrypto ( &h, nBlock ) != 0 ) { 
			fprintf(stderr, _("[ERROR] Deleting block = %ld\n"),nBlock);
			return 1;
		    }
		}
		else{
		    if ( LIBRT_EscribirBloqueCrypto ( &h, nBlock, block ) != 0 ){
			fprintf(stderr, _("[ERROR] Writing changes to the Clauer\n"));
			return 1;
		    }
		}
		
		break;
	    }
	}
    }
    
    LIBRT_FinalizarDispositivo(&h);
   
    return 0;
}


int deleteCert()
{
    USBCERTS_HANDLE h;
    int nDisp, i, j, found= 0, rewrite=0;
    unsigned long bNumbers;
    long * hBlock;
    unsigned char *devs[MAX_DEVICES], *blocks = NULL, type, *blocksAux;
    char pass[MAX_PASSPHRASE], aux='k', nl;
    unsigned char bloque[TAM_BLOQUE];
    unsigned char id[20], zero[20];

    unsigned int lstContainerSize = NUM_KEY_CONTAINERS;
    INFO_KEY_CONTAINER lstContainer[NUM_KEY_CONTAINERS];
    
#ifdef WIN32
    PCCERT_CONTEXT certCtx;
    BYTE *certDer;
    unsigned long tamDer;
    char cn[MAX_CLAUER_OWNER+1];
#endif
    

    memset(zero,0,20);

    if ( ! *g_device ) {
	if ( LIBRT_ListarDispositivos(&nDisp, devs) != 0 ) {
	    fprintf(stderr, _("[ERROR] Unable to get Clauers on the system.\n"));
	    return 1;
	}
	
	if ( nDisp > 1 ) {
	    printf(_("[WARNING] More than a Clauer plugged on the system.\n"
		     "           Use -d option\n"));
	    for ( i = 0 ; i < nDisp ; i++ )
		free(devs[i]);
	    return 1;
	    
	} else if ( nDisp == 0 ) {
	    printf(_("[WARNING] No Clauers detected on the system.\n"));
	    for ( i = 0 ; i < nDisp ; i++ )
		free(devs[i]);
	    return 1;
	}
	
	strcpy(g_device, (char *)devs[0]);
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
	CLUTILS_Destroy(password, strlen(password)+1);
    }
        
    if ( CLUTILS_ConnectEx2(&h, pass, g_device) != 0 ) {
	CLUTILS_Destroy(pass, strlen(pass));
	return 1;
    }
    
    CLUTILS_Destroy(pass, strlen(pass));
    
    if ( LIBRT_LeerBloqueCrypto( &h, cert_num, bloque) != 0) {
	fprintf(stderr, _("[ERROR] Unable to read the specified block"));
	return 1;
    }
	
    if ( interactive ){
	switch (bloque[1]) {
	case  BLOQUE_CERT_PROPIO:
	    printf(_("You are going to delete all the blocks associated with the certificate:\n\t"));
	    CLUTILS_PrintCert(cert_num, bloque, BLOQUE_CERT_PROPIO);
	    break;
	case  BLOQUE_CERT_WEB: 
	    printf(_("You are going to delete all the blocks associated with the certificate:\n\t"));
	    CLUTILS_PrintCert(cert_num, bloque, BLOQUE_CERT_WEB);
	    break;
	case  BLOQUE_CERT_RAIZ:
	    printf(_("You are going to delete all the blocks associated with the certificate:\n\t"));
	    CLUTILS_PrintCert(cert_num, bloque, BLOQUE_CERT_RAIZ);
	    break;
	case  BLOQUE_CERT_INTERMEDIO:
	    printf(_("You are going to delete all the blocks associated with the certificate:\n\t"));
	    CLUTILS_PrintCert(cert_num, bloque, BLOQUE_CERT_INTERMEDIO);
	    break;
	case 0:
	    printf(_("The block is empty!, exiting ...\n"));
	    return 1;
	default:
	    printf( _("[ERROR] Unknown block type, Really is it a Certificate? \n") );
	    return 1;
	}
	
	while ( tolower(aux) != 'y' &&  tolower(aux) != 'n' ){
	    printf(_("Are you sure (y/n)? "));
	    scanf("%c",&aux);
	    scanf("%c",&nl);
	}
	if ( tolower(aux)== 'n')
	    return 1; 
	    
    }
  
    /*  Nos copiamos el identificador del certificado. */
    memcpy(id, BLOQUE_CERTPROPIO_Get_Id(bloque), 20);

    if ( LIBRT_BorrarBloqueCrypto ( &h,cert_num) != 0 ) { 
	fprintf(stderr, _("[ERROR] Deleting block = %d "),cert_num);
	return 1;
    }
    
    /* Ahora, buscamos todos los bloques que contengan este 
     * identificador y los borramos, excepto los key containers
     * en los cuales sólo debemos modificar el puntero a la llave
     */
    
    if ( LIBRT_LeerTodosBloquesOcupados ( &h, NULL, NULL, &bNumbers ) != 0 ) {
	fprintf(stderr, _("[ERROR] Unable to enumerate objects\n"));
	return 1;
    }
    
    blocks = (unsigned char *) malloc (bNumbers * TAM_BLOQUE);
    blocksAux= blocks;

    if ( ! blocks ) {
	LIBRT_FinalizarDispositivo(&h);
	fprintf(stderr,  _("[ERROR] No memory available \n"));
	return 1;
    }
    
    hBlock = ( long * ) malloc ( sizeof(long) * bNumbers );
    
    if ( ! hBlock ) {
	LIBRT_FinalizarDispositivo(&h);
	fprintf(stderr,  _("[ERROR] No memory available \n"));
        free(blocksAux);
        return 1;
    }
    
    if ( LIBRT_LeerTodosBloquesOcupados ( &h, hBlock, blocks, &bNumbers ) != 0 ) {
	fprintf(stderr, _("[ERROR] Unable to enumerate objects\n"));
        free(blocksAux);
        free(hBlock);
        return 1;
    }
    
    for ( i = 0 ; i < bNumbers ; i++ ) {
	type = *(blocks+1);
	if ( type ==  BLOQUE_KEY_CONTAINERS ){
	    rewrite= 0;
	    if ( BLOQUE_KeyContainer_Enumerar(blocks, lstContainer, &lstContainerSize) != 0 ) {
		printf(_("[ERROR] Enumerating key Containers\n"));
		break;
	    }
	    for ( j = 0 ; j < lstContainerSize ; j++ ) {
		if ( memcmp(lstContainer[j].idExchange, id, 20) == 0 ) {
		    if ( memcmp(lstContainer[j].idSignature, id, 20) == 0 || 
			 memcmp(lstContainer[j].idSignature, zero, 20) == 0) {
			if ( BLOQUE_KeyContainer_Borrar ( blocks , lstContainer[j].nombreKeyContainer ) != 0 ){
			    fprintf(stderr, _("[ERROR] Unable to delete the associated key entry on the key container\n"));
			    free(blocksAux);
    			    free(hBlock);
   			    return 1;
			}
			else {
			    if (interactive ) 
				printf(_("Container %s successfully deleted\n"),lstContainer[j].nombreKeyContainer );
			    /* Es necesario reescribir el bloque en el clauer */
			    rewrite= 1; 
			}
		    }
		    else{
			if (  BLOQUE_KeyContainer_Establecer_ID_Exchange ( blocks, lstContainer[j].nombreKeyContainer, zero ) != 0  ){
			     fprintf(stderr, _("[ERROR] Unable to delete the associated key entry on the key container\n"));
			     free(blocksAux);
			     free(hBlock);
                             return 1;
			}
			else{
			     rewrite= 1; 
			}
		    }
		} else if ( memcmp(lstContainer[j].idSignature, id, 20) == 0 ) {
		    if ( memcmp(lstContainer[j].idExchange , id, 20) == 0 || 
			 memcmp(lstContainer[j].idExchange , zero, 20) == 0) {
			if ( BLOQUE_KeyContainer_Borrar ( blocks , lstContainer[j].nombreKeyContainer ) != 0 ){
 			     fprintf(stderr, _("[ERROR] Unable to delete the associated key entry on the key container\n"));
			     free(blocksAux);
		             free(hBlock);
 			     return 1;
			}
			else {
			    if ( interactive ) 
				printf(_("Container %s successfully deleted\n"),lstContainer[j].nombreKeyContainer );
			    /* Es necesario reescribir el bloque en el clauer */
			    rewrite= 1; 
			}
		    }
		    else{
			if (  BLOQUE_KeyContainer_Establecer_ID_Signature ( blocks, lstContainer[j].nombreKeyContainer, zero ) != 0 ){
			    free(blocksAux);
    	   		    free(hBlock);
		            fprintf(stderr, _("[ERROR] Unable to delete the associated key entry on the key container\n"));
			    return 1;
			}
			else{
			    rewrite= 1;	
			}
		    }
		}
	    }
	    if ( rewrite == 1 ){
		if ( BLOQUE_KeyContainer_Enumerar(blocks, lstContainer, &lstContainerSize) != 0 ) {
		    printf(_("[ERROR] Enumerating key Containers\n"));
		    break;
		}
		
		if ( lstContainerSize == 0 ){
		    if ( LIBRT_BorrarBloqueCrypto ( &h, hBlock[i] ) != 0 ) { 
			fprintf(stderr, _("[ERROR] Deleting block = %ld\n"),hBlock[i]);
			free(blocksAux);
			free(hBlock);
			return 1;
		    }
		}
		else{
		    if ( LIBRT_EscribirBloqueCrypto ( &h, hBlock[i], blocks ) != 0 ){
			free(blocksAux);
			free(hBlock);
			fprintf(stderr, _("[ERROR] Writing changes to the Clauer\n"));
			return 1;
		    }
		}
	    }
	}
	else {
	    found= 0;
	    switch ( type ) {
	    case BLOQUE_LLAVE_PRIVADA:
		if ( memcmp(id, BLOQUE_LLAVEPRIVADA_Get_Id(blocks), 20 ) == 0 ){
		    if ( interactive ) {
			printf ( _("Deleted block:    "));
			CLUTILS_PrintLlave(hBlock[i], blocks);
		    }
		    found= 1;
		}
		break;

	    case BLOQUE_CIPHER_PRIVKEY_PEM:
		if ( memcmp(id, BLOQUE_CIPHER_PRIVKEY_PEM_Get_Id(blocks), 20 ) == 0 ){
		    if ( interactive ) {
			printf ( _("Deleted block:    "));
			CLUTILS_PrintLlave(hBlock[i], blocks);
		    }
		    found= 1;
		}
		break;


		
		
	    case BLOQUE_PRIVKEY_BLOB:
		if ( memcmp(id, BLOQUE_PRIVKEYBLOB_Get_Id(blocks), 20 ) == 0 ){
		    if ( interactive ) {
			printf ( _("Deleted block:    "));
			CLUTILS_PrintBlob(&h, hBlock[i], blocks);
		    }
		    found= 1;
		}
		break;
	    
	    case BLOQUE_CIPHER_PRIVKEY_BLOB:
		if ( memcmp(id, BLOQUE_PRIVKEYBLOB_Get_Id(blocks), 20 ) == 0 ){
		    if ( interactive ) {
			printf ( _("Deleted block:    "));	       
			CLUTILS_PrintBlob(&h, hBlock[i], blocks);			
		    }
		    found= 1;
		}
		break;
	    }
	    if ( found ){
		if ( LIBRT_BorrarBloqueCrypto ( &h, hBlock[i] ) != 0 ) { 
		    free(blocksAux);
    		    free(hBlock);
		    fprintf(stderr, _("[ERROR] Deleting block = %ld\n"),hBlock[i]);
		    return 1;
		}
	    }
	}
	blocks += TAM_BLOQUE;
    }

    free(blocksAux);
    free(hBlock);

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
    } 
    else if ( st == ST_LIST ) {
	err= !CLUTILS_PrintClauers();
    } 
    else if ( st == ST_HELP ) {
	usage();
    }
    else if ( st == ST_OK ) {
	if ( cert_num != -1 ){
	    err= deleteCert();
	}
	else if( block_num != -1 ) {
	    err= deleteBlock();
	} 	
	else if( *g_keycontname ) {
	    err= deleteContainerEntry();
	}
	else{
	    usage();
	}
    }


#ifdef LINUX
    CRYPTO_Fin();
#endif 

    LIBRT_Fin();

    return err;

}


