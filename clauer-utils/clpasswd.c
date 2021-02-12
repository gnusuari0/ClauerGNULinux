
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

#include <errno.h>
#include "misc.h"
#include "nls.h"
#ifdef LINUX
#include <sys/mman.h>
#elif defined(WIN32)
#include <windows.h>
#define snprintf _snprintf
#endif


typedef enum { 
    ST_INI,
    ST_HELP,
    ST_DEV,
    ST_DEV_NAME,
    ST_LIST,
    ST_OLD_PASSWORD,
    ST_NEW_PASSWORD,
    ST_ERR
} parse_st_t;



static char g_device[MAX_PATH_LEN+1];
char * g_newpass= NULL, * g_oldpass= NULL;



void usage ( void )
{
    fprintf(stderr, _("Usage mode: clpasswd -h | -l | [-op pass] [-np pass] -d (device|file)\n"));
    fprintf(stderr, _("Changes the Clauer's password\n\n"));
    fprintf(stderr, _("   -h,  --help                   Prints this help message\n"));
    fprintf(stderr, _("   -l,  --list                   List the clauers plugged on the system\n"));
    fprintf(stderr, _("   -d,  --device device          Selects the clauer to be listed\n"));
    fprintf(stderr, _("   -op, --old-password password  Insecure mode of give the password, if not given, the password is asked with getpass.\n"));
    fprintf(stderr, _("   -np, --new-password password  Insecure mode of give the password, if not given, the password is asked with getpass.\n"));
}



void parse ( int argc, char **argv, parse_st_t *st, char errMsg[256] )
{
    int op, aux;


    *g_device = 0;
  
    *st = ST_INI;
  
    for ( op = 1 ; (op < argc) && (*st != ST_ERR) ; op++ ) {
    
	if ( *st == ST_INI ) {

	    if ( ( strcmp(argv[op], "-h" ) == 0) || ( strcmp(argv[op], "--help") == 0 ) ) 
		*st = ST_HELP;
	    else if ( strcmp(argv[op], "-l") == 0 || strcmp(argv[op], "--list") == 0 ) 
		*st = ST_LIST;
	    else if ( strcmp(argv[op], "-op") == 0 || strcmp(argv[op], "--old-password") == 0 )
		*st = ST_OLD_PASSWORD;
	    else if ( strcmp(argv[op], "-np") == 0 || strcmp(argv[op], "--new-password") == 0 )
		*st = ST_NEW_PASSWORD;
	    else if ( strcmp(argv[op], "-d") == 0 || strcmp(argv[op], "--device") == 0 ) 
		*st = ST_DEV_NAME;
	    else {
		snprintf(errMsg, 256, _("Invalid option: %s"), argv[op]);
		*st = ST_ERR;
	    }
	} else if ( *st == ST_LIST ) {
	    snprintf(errMsg, 256, _("Invalid option: %s"), argv[op]);
	    *st = ST_ERR;
	} else if ( *st == ST_OLD_PASSWORD ) {
	    if ( g_oldpass ){
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
		  g_oldpass= (char *) malloc((sizeof(char) * aux)+1);
		    strncpy(g_oldpass, argv[op], aux+1);
		    *st = ST_INI;
		}
	    }	    
	} else if ( *st == ST_NEW_PASSWORD ) {
	    if ( g_newpass ){
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
		  g_newpass= (char *) malloc((sizeof(char) * aux)+1);
		    strncpy(g_newpass, argv[op], aux+1);
		    *st = ST_INI;
		}
	    }	    
	} else if ( *st == ST_DEV_NAME ) {
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
	}
    }

  
    if ( *st == ST_DEV_NAME ) {
	snprintf(errMsg, 256, _("No device name specified"));
	*st = ST_ERR;
    }
    
}



/*! \brief Implements an algorithm that indicates the quality of a passphrase.
 *
 * Implements an algotihm that indicates the quality of a passphrase.
 *
 * \param passphrase
 *        The passphrase to be tested.
 *
 * \param q
 *        The quality of the passphrase in the range [0, 1].
 *
 * \retval 0
 *         You shouldn't use this passphrase
 *
 * \retval 1
 *         The passphrase is la rejostia!!!
 */

int is_a_good_passphrase ( char *passphrase, float *q )
{
    *q = 1.0;
    return 1;
}




int change (void)
{
    USBCERTS_HANDLE h;
    char *oldPass = NULL, *newPass = NULL, *confPass = NULL;
    int errLockOld, errLockNew;
    float q;

    unsigned char *devs[MAX_DEVICES];
    int nDevs, i, aux, sizeMemOldPass= MAX_PASSPHRASE, sizeMemNewPass= MAX_PASSPHRASE;

    /* Si no se especificó dispositivo, obligamos a que sólo haya uno
     */

    if ( ! *g_device ) {

	if ( LIBRT_ListarDispositivos(&nDevs, devs) != 0 ) {
	    fprintf(stderr, _("[ERROR] Unable to get Clauers on the system.\n"));
	    return 1;
	}
    
	if ( nDevs == 0 ) {
	    for ( i = 0 ; i < nDevs ; i++ )
		free(devs[i]);
	    fprintf(stderr, _("[ERROR] No Clauers detected on the system.\n"));
	    return 1;
	} else if ( nDevs > 1 ) {
	    for ( i = 0 ; i < nDevs ; i++ )
		free(devs[i]);	    
	    fprintf(stderr,_("[ERROR] More than a Clauer plugged on the system.\n"
			     "           Use -d option\n"));
	    return 1;
	}
    
	strcpy(g_device, ( char * )devs[0]);
    
	for ( i = 0 ; i < nDevs ; i++ )
	    free(devs[i]);
    }
  
    /* Memory allocation stuff
     */
  
    if ( g_oldpass ){
	aux= strlen(g_oldpass);
	sizeMemOldPass= aux;
	oldPass = ( char * ) malloc ( (sizeof(char) * aux)+1 );
	strncpy(oldPass, g_oldpass, aux+1);
	memset(g_oldpass, 0, aux);
    }
    else{
	oldPass = ( char * ) malloc ( MAX_PASSPHRASE );
	if ( ! oldPass ) {
	    fprintf(stderr, _("[ERROR] No memory available \n"));
	    return 1;
	}
    }

#if defined(LINUX)
    if ( mlock(oldPass, MAX_PASSPHRASE) == -1 )
	errLockOld = 1;
    else
	errLockOld = 0;
#elif defined(WIN32)
    if ( ! VirtualLock(oldPass, MAX_PASSPHRASE) )
	errLockOld = 1;
    else
	errLockOld = 0;
#endif
  

    if ( g_newpass ){
	aux= strlen(g_newpass);
	sizeMemNewPass= aux;
	newPass = ( char * ) malloc ( (sizeof(char) * aux) +1);
	strncpy(newPass, g_newpass, aux+1);
	memset(g_newpass, 0, aux);
    }
    else{ 
	newPass = ( char * ) malloc ( MAX_PASSPHRASE );
	if ( ! newPass ) {
	    if ( ! errLockOld ) {
#if defined(LINUX)
		munlock(oldPass, MAX_PASSPHRASE);
#elif defined(WIN32)
		VirtualUnlock(oldPass, MAX_PASSPHRASE);
#endif
	    }
	    free(oldPass);
	    fprintf(stderr, _("[ERROR] No memory available \n"));
	    return 1;
	}
    }
  
#if defined(LINUX)
    if ( mlock(newPass, sizeMemNewPass) == -1 )
	errLockNew = 1;
    else
	errLockNew = 0;
#elif defined(WIN32)
    if ( ! VirtualLock(newPass, sizeMemNewPass) == -1 )
	errLockNew = 1;
    else
	errLockNew = 0;
#endif
  
    confPass = ( char * ) malloc ( MAX_PASSPHRASE );
    if ( ! confPass ) {
#if defined(LINUX)
	if ( ! errLockOld )
	    munlock(oldPass, MAX_PASSPHRASE);
	if ( ! errLockNew )
	    munlock(newPass, sizeMemNewPass);
#elif defined(WIN32)
	if ( ! errLockOld )
	    VirtualUnlock(oldPass, sizeMemOldPass);
	if ( ! errLockNew )
	    VirtualUnlock(newPass, sizeMemNewPass);
#endif
	free(oldPass);
	free(newPass);
	fprintf(stderr, _("[ERROR] No memory available \n"));
	return 1;
    }
  
    /* Ok... do the real work
     */
  
    if ( ! g_oldpass){
        *oldPass=0;
        while (strlen(oldPass)==0){
  	  if ( CLUTILS_AskPassphrase (_("Clauer's password: "), oldPass) != 0 ) {
	    fprintf(stderr, _("[ERROR] Unable to get the password"));
	    goto endChange;
	  }
        }
    }
 
    if ( LIBRT_IniciarDispositivo(( unsigned char *)g_device, oldPass, &h) != 0 ) {
	fprintf(stderr, _("[ERROR] Unable to connect with the Clauer %s\n        Incorrect password?\n"), g_device);
	goto endChange;
    }

    LIBRT_FinalizarDispositivo(&h);

    if ( ! g_newpass ){
        *newPass=0;
        while (strlen(newPass)==0){
	  if ( CLUTILS_AskPassphrase (_("New Clauer's password: "), newPass) != 0 ) {
	    fprintf(stderr, _("[ERROR] Unable to get the new password"));
	    goto endChange;
	  }
        }
    }
    
    if ( ! is_a_good_passphrase (newPass, &q) ) {
	fprintf(stderr, _("[ERROR] This password has less future than Chewaka on a depilation challenge. Try again\n"));
	goto endChange;
    }
  
    if ( ! g_newpass ){
	if ( CLUTILS_AskPassphrase (_("New password (confirmation): "), confPass) != 0 ) {
	    fprintf(stderr, _("[ERROR] Error getting confirmation\n"));
	    goto endChange;
	}
      
	if ( strcmp(newPass, confPass) != 0 ) {
	    fprintf(stderr, _("[ERROR] Passwords do not match\n"));
	    goto endChange;
	}
    }
  
    if ( CLUTILS_ConnectEx2(&h, oldPass, g_device) != 0 )
	goto endChange;
  
    if ( LIBRT_CambiarPassword(&h, newPass) != 0 ) {
	fprintf(stderr, _("[ERROR] An error happened changing the password\n"));
	goto endChange;
    }
  
 endChange:
  
#if defined(LINUX)

    CRYPTO_SecureZeroMemory(oldPass, sizeMemOldPass);
    CRYPTO_SecureZeroMemory(newPass, sizeMemNewPass);
    CRYPTO_SecureZeroMemory(confPass, MAX_PASSPHRASE);

    if ( ! errLockOld )
	if ( munlock(oldPass, sizeMemOldPass) == -1 ){
	    fprintf(stderr,_("[ERROR] Unable to unlock oldPass memory errno= %d.\n"),errno);
	    return 1;	  
	}
    if ( ! errLockNew )
	if ( munlock(newPass, sizeMemNewPass) == -1 ){
	    fprintf(stderr,_("[ERROR] Unable to unlock newPass memory errno= %d.\n"),errno);
	    return 1;
	}


#elif defined(WIN32)

    SecureZeroMemory(oldPass, MAX_PASSPHRASE);
    SecureZeroMemory(newPass, MAX_PASSPHRASE);
    SecureZeroMemory(confPass, MAX_PASSPHRASE);

    if ( ! errLockOld )
	VirtualUnlock(oldPass, sizeMemOldPass);
    if ( ! errLockNew )
	VirtualUnlock(newPass, sizeMemNewPass);
#endif
  
    free(oldPass);
    free(newPass);
    free(confPass);
  
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
      return 1;
    } else if ( st == ST_LIST ){
      CLUTILS_PrintClauers();
    }else if ( st == ST_HELP ) {
      usage();
    } else {
      err= change();
    }
    
    return err;
    
}


