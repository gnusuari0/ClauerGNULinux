
/*

                        LICENCIA

1. Este programa puede ser ejecutado sin ninguna restricci�n
   por parte del usuario final del mismo.

2. La  Universitat Jaume I autoriza la copia y  distribuci�n
   del programa con cualquier fin y por cualquier medio  con
   la  �nica limitaci�n de que, de forma  apropiada, se haga
   constar  en  cada  una  de las copias la  autor�a de esta  
   Universidad  y  una reproducci�n  exacta de las presentes 
   condiciones   y   de   la   declaraci�n  de  exenci�n  de 
   responsabilidad.

3. La  Universitat  Jaume  I autoriza  la  modificaci�n  del
   software  y  su  redistribuci�n  siempre que en el cambio
   del  c�digo  conste la autor�a de la Universidad respecto  
   al  software  original  y  la  url de descarga del c�digo
   fuente  original. Adem�s, su denominaci�n no debe inducir 
   a  error  o  confusi�n con el original. Cualquier persona
   o  entidad  que  modifique  y  redistribuya  el  software 
   modificado deber�  informar de tal circunstancia mediante
   el  env�o  de  un  mensaje  de  correo  electr�nico  a la 
   direcci�n  clauer@uji.es  y  remitir una copia del c�digo 
   fuente modificado.

4. El  c�digo  fuente  de todos los programas amparados bajo 
   esta licencia  est�  disponible para su descarga gratuita
   desde la p�gina web http//:clauer.uji.es.

5. El hecho en s� del uso, copia o distribuci�n del presente 
   programa implica la aceptaci�n de estas condiciones.

6. La  copia y distribuci�n del programa supone la extensi�n 
   de las presentes condiciones al destinatario.
   El  distribuidor no puede imponer condiciones adicionales
   que limiten las aqu� establecidas.

       DECLARACI�N DE EXENCI�N DE RESPONSABILIDAD

Este  programa  se  distribuye  gratuitamente. La Universitat 
Jaume  I  no  ofrece  ning�n  tipo de garant�a sobre el mismo
ni acepta ninguna responsabilidad por su uso o  imposibilidad
de uso.

*/

/*
 * Formatea una partici�n criptogr�fica dado el 
 * dispositivo y el password
 * 
 * */

#include <stdio.h>
#include <stdlib.h>
#include "misc.h"
#include "nls.h"

#ifdef LINUX 

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>

#elif defined(WIN32)
#include <getopt/getopt.h>
/* En windows parece que no tenemos snprintf()
 */
#define snprintf _snprintf
#endif 

#include <CRYPTOWrapper/CRYPTOWrap.h>


#define BLOCK_SIZE 10240
#define NRSV_BLOCKS 7


struct infoZone{
  char idenString[40];
  unsigned char id[20];
  int nrsv;
  int currentBlock;
  int totalBlocks;
  int formatVersion;
  char info[10240-76];
};

static struct option long_options[] =
    {
	{"password", 1, 0, 0},
	{"device", 1, 0, 0},
	{"help", 0, 0, 'h'},
	{0,0,0,0}
    };

int main ( int argc, char ** argv ){
  
  struct infoZone iz;
  int i, devSize= 0, fp, nbloques, aux, res, option_index;
  unsigned char newIden[40];
  char pass[MAX_PASSPHRASE], device[MAX_PATH_LEN], c;
  FILE * fd;


#ifndef MAC
  nls_lang_init();
#endif

  *pass= 0; *device=0;
  while ( 1 ) {

    c = getopt_long(argc, argv, "d:p:h", long_options, &option_index);
    if ( c == -1 )
	break;
   
    switch ( c ) {
    case '?':
	return 1;
    case 0:
      if ( strcmp(long_options[option_index].name, "device") == 0 ) {
	  aux= strlen(optarg);
	  if (aux >= MAX_PATH_LEN){
	      fprintf(stderr, _("[ERROR] Device name longer than allowed\n"));
	      return 1;
	  }
	  strncpy(device, optarg, aux);
      }
      else if ( strcmp(long_options[option_index].name, "password") == 0 ) {
	  aux= strlen(optarg);
	  if (aux >= MAX_PASSPHRASE){
	      fprintf(stderr, _("The password is longer than 127 characters\n"));
	      return 1;
	  }
	  strncpy(pass, optarg, aux);
      }
      
    case 'd':
	aux= strlen(optarg);
	if (aux >= MAX_PATH_LEN){
	    fprintf(stderr, _("[ERROR] Device name longer than allowed\n"));
	    return 1;
	}
	strncpy(device, optarg, aux+1);
	break;

    case 'p':
	aux= strlen(optarg);
	if (aux >= MAX_PASSPHRASE){
	    fprintf(stderr, _("The password is longer than 127 characters\n"));
	    return 1;
	}
	strncpy(pass, optarg, aux+1);
	break;
    case 'h':
        fprintf(stderr,_("Usage mode: %s -d device_partition [-p password]\n"),argv[0]);
        fprintf(stderr,_("    If the password is not indicated it will be prompted with getpass\n"));
	fprintf(stderr,_("    Example: clmakefs -d /dev/sda4 -p foopass \n"));
	exit(1); 
    }
  }

  if ( *device == 0 ){
        
      fprintf(stderr,_("Usage mode: %s -d device_partition [-p password]\n"),argv[0]);
      fprintf(stderr,_("    If the password is not indicated it will be prompted with getpass\n"));
      fprintf(stderr,_("    Example: clmakefs -d /dev/sda4 -p foopass \n"));      
      exit(1);
  }

  /* Obtenemos el tama�o del dispositivo */
  fd= fopen(device,"r");
  
  if ( fd == NULL ){
    fprintf(stderr,_("[ERROR] Opening %s\n"),device);
    exit(1);
  }
  
  if ( fseek(fd, 0L, SEEK_END) != 0 ){
    fprintf(stderr,_("[ERROR] Getting size of %s\n"),argv[1]);
    exit(1);
  }
 
  devSize= ftell(fd);
  if ( devSize == -1 ){ 
    fprintf(stderr,_("[ERROR] Getting size of %s\n"),argv[1]);
    exit(1);
  } 
  
  fclose(fd);
  
  /* Formateamos el dispositivo */ 
  fp= open(device,O_RDWR);

  if ( fp == -1 ){
    fprintf(stderr,_("[ERROR] Opening %s\n"),argv[1]);
    exit(1);
  }
  
  nbloques= devSize/BLOCK_SIZE;
  
  iz.nrsv= NRSV_BLOCKS; 
  iz.totalBlocks= nbloques - 1 - NRSV_BLOCKS;
  iz.currentBlock= 0;   /* Inicializamos primer bloque libre 0 en zona de objetos */
  iz.formatVersion= 1;  /* Versi�n 1 del formato, me lo invento yo! */

  CRYPTO_Ini();
  /* Generamos identificador random del clauer */
  if ( CRYPTO_Random (20,(unsigned char * ) iz.id)== ERR_CW_SI ){
    fprintf(stderr,_("Unable to generate random numbers\n"));
    exit(1);
  }

  if ( *pass == 0 ){
      if ( CLUTILS_AskPassphrase(_("\tClauer's password: "), pass) != 0 ) {
	  fprintf(stderr, _("[ERROR] Unable to get the password\n"));
	  return 1;
      }
  }

  /* Ciframos con la password el churro "UJI - Clauer PKI storage system" */
  res= CRYPTO_PBE_Cifrar (pass,
			  (unsigned char *)&iz.id,
			  20,
			  1000,
			  1,
			  CRYPTO_CIPHER_DES_EDE3_CBC,
			  (unsigned char *)"UJI - Clauer PKI storage system",
			  32,
			  newIden,
			  &aux );

  if  ( res != 0 ){
    fprintf(stderr,_("[ERROR] Ciphering the identifier."));
    exit(-1);
  }
  
  memcpy(iz.idenString, newIden, aux);
  
  CRYPTO_Fin();
    
  if ( write( fp, &iz,BLOCK_SIZE ) == -1 ){
      fprintf(stderr,_("[ERROR] Unable to write on the device\n"));
      exit(1);
  } 

  memset(&iz, 0, sizeof(iz));
  
  for ( i=0; i < nbloques-1; i++ ){
  	write( fp, &iz, BLOCK_SIZE );
  }
    
  close(fp);
  return 0;  
}
