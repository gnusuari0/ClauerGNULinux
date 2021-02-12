
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

/*
 * Muestra el contenido del bloque  
 * de información
 * 
 * 
 */

#include <stdio.h>


#include <clio/clio.h>
#include "misc.h"
#include "nls.h"

#include <LIBRT/libRT.h>


#ifdef WIN32
#define snprintf _snprintf
#endif


typedef enum {
  ST_INI,
  ST_DEV,
  ST_DEV_NAME,
  ST_HELP,
  ST_LIST,
  ST_ERR
} parse_st;




static char g_devices[MAX_DEVICES][MAX_DEVICE_LEN+1];
static int g_nDevs;
static int g_all;




int parse ( int argc, char **argv, parse_st *st, char errMsg[256] )
{
  int op;

  /* Inicializamos las variables globales
   */

  for ( op = 0 ; op < MAX_DEVICES ; op++ ) 
    memset(g_devices[op], 0, MAX_DEVICE_LEN);
  g_nDevs = 0;
  g_all = 0;

  /* Parse 
   */

  *st = ST_INI;

  for ( op = 1 ; ( op < argc ) && ( *st != ST_ERR ) ; op++ ) {

    if ( *st == ST_INI ) {
      if ( strcmp("-h", argv[op]) == 0 || strcmp("--help", argv[op]) == 0 ) {
	*st = ST_HELP;
      } else if ( strcmp("-d", argv[op]) == 0 || strcmp("--device", argv[op]) == 0 ) {
	*st = ST_DEV_NAME;
      } else if ( strcmp("-l", argv[op]) == 0 || strcmp("--list", argv[op]) == 0 ) {
	*st = ST_LIST;
      } else {
	snprintf(errMsg, 256, _("Invalid option: %s"), argv[op]);
	*st = ST_ERR;
      }
    } else if ( *st == ST_HELP ) {
      snprintf(errMsg, 256, _("Invalid option: %s"), argv[op]);
      *st = ST_ERR;
     } else if ( *st == ST_DEV_NAME ) {
       if ( strcmp(argv[op], "ALL") == 0 )
	 g_all = 1;
       else
	 strncpy(g_devices[g_nDevs++], argv[op], MAX_DEVICE_LEN);
       *st = ST_DEV;
    } else if ( *st == ST_DEV ) { 
      if ( strcmp("-d", argv[op]) == 0 || strcmp("-d", argv[op]) == 0 ) {
	if ( g_nDevs >= MAX_DEVICES )
	  fprintf(stderr, _("[WARNING] Maximum number of devices execeeded\n"));
	else
	  *st = ST_DEV_NAME;
      } else {
	snprintf(errMsg, 256, _("Invalid option: %s"), argv[op]);
	*st = ST_ERR;
      }
    } else if ( *st == ST_LIST ) {
      snprintf(errMsg, 256, _("Invalid option: %s"), argv[op]);
      *st = ST_ERR;
    } else if ( *st == ST_ERR ) {
      break;
    }
  }

  if ( *st == ST_DEV_NAME ) {
    snprintf(errMsg, 256, _("No device name specified"));
    *st = ST_ERR;
  } else if ( *st == ST_INI )
    g_all = 1;


  return 1;

}



void usage ( void ) 
{
    fprintf(stderr, _("Usage mode: clview (-h|--help) | (-l|--list) | (-d|--device (device|file|ALL))+\n"));
    fprintf(stderr, _("Prints the information block content of the specified clauers\n\n"));
    fprintf(stderr, _("Options are:\n"));
    fprintf(stderr, _("    -h, --help              Prints this help message\n"));
    fprintf(stderr, _("    -l, --list              List the clauers plugged on the system\n"));
    fprintf(stderr, _("    -d, --device device     Selects the clauer to be listed\n"));
    fprintf(stderr, _("Without arguments, it prints  the information block of all clauers plugged on the system\n"));

}







int view ( void )
{
  block_info_t  ib;
  //clauer_handle_t hClauer;
  USBCERTS_HANDLE hClauer;
  int err, i, j;
  unsigned char *devs[MAX_DEVICES], owner[MAX_CLAUER_OWNER+1];
  unsigned char *deviceName;
 


  if ( g_all ) {
      if ( LIBRT_ListarDispositivos((int*)&g_nDevs, devs) != 0 ) {
	  fprintf(stderr, _("[ERROR] Unable to get Clauers on the system.\n"));    
      return 1;
    }
  }

  if ( g_nDevs == 0 ){
      fprintf(stderr, _("[ERROR] No Clauers detected on the system.\n"));
      return 1;
  } 

  for ( i = 0 ; i < g_nDevs ; i++ ) {
   
    if ( g_all ) {
      printf("%s:\n\n", devs[i]);
      deviceName = (unsigned char *)devs[i];
      err = LIBRT_IniciarDispositivo (devs[i], NULL, &hClauer);
      //err = IO_Open((char *)devs[i], &hClauer, IO_RD, IO_CHECK_IS_CLAUER);
    } else {
      deviceName = (unsigned char *)g_devices[i];
      printf("%s:\n\n", g_devices[i]);
      err = LIBRT_IniciarDispositivo (deviceName, NULL, &hClauer);
      //err = IO_Open(g_devices[i], &hClauer, IO_RD, IO_CHECK_IS_CLAUER);
    }

    if ( err != 0 ) { 
	fprintf(stderr, _("\t[ERROR] Cannot initialize the device\n"));
	if ( g_all ) {
	  for ( j = 0 ; j < g_nDevs ; j++ )
	    free(devs[j]);
	}
	continue;
    }

    if ( LIBRT_LeerBloqueIdentificativo(&hClauer, (unsigned char *) &ib) != IO_SUCCESS ) {
      fprintf(stderr, _("\t[ERROR] Unable to read the information block\n"));
      continue;
    }

    LIBRT_FinalizarDispositivo(&hClauer);
    CLUTILS_Get_Clauer_Owner((char *)deviceName,(char *)owner);
    if ( ! *owner )
	strcpy((char *)owner, _("Unable to guess it"));
    
    printf(_("\t%22.22s %s\n"), _("Owner:"), owner);
    printf(_("\t%22.22s "), _("Identifier:"));
    for ( j = 0 ; j < 20 ; j++ )
	printf("%02x", ib.id[j]);
    printf("\n");
    printf(_("\t%22.22s %d\n"), _("Format version:"), ib.version);
    printf(_("\t%22.22s %d\n"), _("Total Blocks:"), ib.totalBlocks);
    printf(_("\t%22.22s %d\n"), _("Reserved zone size:"), ib.rzSize);
    printf(_("\t%22.22s %d\n"), _("Current Block:"), ib.cb);
    putchar('\n');

  }

  if ( g_all ) {
    for ( j = 0 ; j < g_nDevs ; j++ )
      free(devs[j]);
  }

  return 0;

}




int main ( int argc, char ** argv )
{
    int err= 0;
    char errMsg[256];
    parse_st st;

#ifndef MAC
    nls_lang_init();	
#endif

    LIBRT_Ini();

    parse(argc, argv, &st, errMsg);

    switch ( st ) {

    case ST_ERR:
	fprintf(stderr, "[ERROR] %s\n\n", errMsg);
	usage();
	err= 1;
	break;

    case ST_HELP:
	usage();
	break;
    
    case ST_LIST:
	err= ! CLUTILS_PrintClauers();
	break;

    case ST_INI:
    case ST_DEV:
	err= view();
	break;
    default:
	break;  
    }

  
    return err;  
}
