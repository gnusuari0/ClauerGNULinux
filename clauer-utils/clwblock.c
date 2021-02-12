
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
 *  Fecha:       16-03-2006
 *  Descripcion: Esta aplicaci-Aón recibe un nombre de fichero, comprueba-b
 *               que el tama-Año son 10240 bytes y lo escribe en el clauer -b
 *               por medio del clos.
 *  
 *  Paul Santapau Nebot
 *       Clauer Team 
*/

#include <LIBRT/libRT.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if defined (LINUX)
#include <unistd.h>
#endif

#include "misc.h"

#ifdef WIN32
#include <getopt/getopt.h>
#else
#define _GNU_SOURCE
#include <getopt.h>
#endif

#define MAX_DEVICES 128 
#define BLOCK_SIZE  10240 
#define NUM_TRIES   3


enum {OPT_FILE, OPT_PASSWORD, OPT_DEVICE, OPT_NUMBER, OPT_HELP, OPT_LIST, OPT_EXIT};
char * password= NULL, * file= NULL, * device= NULL;
int  g_numBloque=-1;
// char device[MAX_PATH_LEN];


void usage ( void )
{
    fprintf(stderr, "Modo de empleo: clwblock [-h] | [-l] | [-n num_block] [-d dispositivo] [-p password]  -f file  \n");
    fprintf(stderr, "Escribe un bloque en el clauer\n\n");
    fprintf(stderr, "Las opciones posibles son:\n");
    fprintf(stderr, "   -h, --help               Imprime este mensaje de ayuda\n");
    fprintf(stderr, "   -l, --list               Lista los clauers presentes en el sistema\n");
    fprintf(stderr, "   -d, --device  disp       Selecciona el clauer con el que queremos operar\n");
    fprintf(stderr, "   -f, --file fichero       Fichero desde el que se leerá el bloque.\n");
    fprintf(stderr, "   -n, --number num_bloque  Posición en la que insertar el bloque.\n");
    fprintf(stderr, "   -p, --password password  Modo inseguro de indicar la password, si se obvia se solicita mediante getpass().\n\n");
}




int parse( int argc, char ** argv ){
    int c=0, option_index=0, ret=0, aux;

    static struct option long_options[] = {
        {"file", 1, 0, OPT_FILE},
        {"password", 1, 0, OPT_PASSWORD},
        {"device", 1, 0, OPT_DEVICE},
	{"number", 1, 0, OPT_NUMBER},
        {"help", 0, 0, OPT_HELP},
        {"list", 0, 0, OPT_LIST},
        {0, 0, 0, 0}
    };

    opterr= 0;
    if ( argc == 0 ){
        usage();
        return OPT_EXIT;
    }
    while ( (c= getopt_long (argc, argv, "hld:n:f:p:",
                             long_options, &option_index)) != -1 ){
        switch (c){
        case 'd':
        case OPT_DEVICE:
	    aux= strlen(optarg);
	    if (aux >= MAX_PATH_LEN ){
		fprintf(stderr,"[ERROR] La ruta del dispositivo excede del máximo número de caracteres permitido\n");
		return OPT_EXIT;
	    }
	    device= (char *) malloc( (sizeof(char) * aux) +1 );
	    if (!device){
		fprintf(stderr,"[ERROR]  Al reservar memoria\n");
		return OPT_EXIT;
	    }
            strncpy(device, optarg, aux+1);
            break;
	
        case 'n':
	case OPT_NUMBER:
	    //Nos quedamos con el número de bloque donde insertar.
	    g_numBloque= atoi(optarg);
	    ret=OPT_NUMBER;
	    break;    

        case 'f':
        case OPT_FILE:
	    aux= strlen(optarg); 
	    if (aux >=128) 
		return OPT_EXIT;
	    file= (char *)malloc(sizeof(char) * aux + 1 );
	    if (!file){
		fprintf(stderr,"[ERROR] Al reservar memoria\n");
		return OPT_EXIT;
	    }
	    strncpy(file, optarg, aux + 1 );
            ret= OPT_FILE;
            break;
        case 'p':
        case OPT_PASSWORD:       
	    aux= strlen(optarg); 
	    if (aux >=128) 
		return OPT_EXIT;
	    password= (char *)malloc((sizeof(char) * aux) + 1 );
	    if(!password){
		fprintf(stderr,"[ERROR] Al reservar memoria\n");
		return OPT_EXIT;
	    }	
	    strncpy(password, optarg, aux + 1 );
            break;
        default:
            usage();
            return OPT_EXIT;
        }
    }
    return ret;
}                             





int main(int argc, char ** argv){
    int fp, size, leido, i;
    long numBloque;
    unsigned char * dispositivos[MAX_DEVICES], buff[BLOCK_SIZE];
    char passphrase[MAX_PASSPHRASE];
    int nDisp;
    USBCERTS_HANDLE handle;


    if (argc == 1){
	usage();
	return 1;
    }
    
    switch ( parse( argc, argv ) ){
    case OPT_FILE:   
	break;
    case OPT_EXIT:
	return 1;
	
    }
    
        
    fp= open(file, O_RDONLY);
    if ( fp == -1 ){
	fprintf(stderr,"ERROR, no puedo abrir el fichero especificado.\n");
	return 1;
    }
    
    size= lseek( fp, 0, SEEK_END );
    if ( size != 10240 ){
	fprintf(stderr,"ERROR, el tamanyo del fichero (%d) no son 10240 bytes.\n", size );
	close(fp);
	return 1;
    }

    lseek( fp, 0, SEEK_SET );
    leido= read(fp, buff, BLOCK_SIZE);
    if ( leido != BLOCK_SIZE ){
	fprintf(stderr,"ERROR Solo se pudieron leer %d bytes del fichero.\n", leido);
	close(fp);
	return 1;
    }
    close(fp);
    
    LIBRT_RegenerarCache();
    
    if ( LIBRT_ListarDispositivos(&nDisp, dispositivos ) != 0 ){
	fprintf(stderr,"ERROR Listando dispositivos, esta el clos ejecutandose?");
	return 1;
    }
    
    if ( nDisp == 0 ){
	fprintf(stderr,"No se detecto ningun clauer conectado.\n");
	return 1;
    }
    
    if (device){
	free(dispositivos[0]);
	dispositivos[0]= (unsigned char *)device ;
    }
    
    if (password==NULL){
	memset(passphrase, 0, MAX_PASSPHRASE);
	for ( i= 0; i < NUM_TRIES; i++  ){
	    if ( CLUTILS_AskPassphrase ( "Introduce la password del dispositivo: ", passphrase ) == 0 ){
		if ( strlen(passphrase) != 0 && LIBRT_IniciarDispositivo(dispositivos[0], passphrase, &handle) == 0 )
		    break;
		else{
		    fprintf(stderr,"ERROR Contrasenya incorrecta, tienes %d intentos\n",NUM_TRIES-i-1);
		}
	    }
	}
    }
    else {
	if ( strlen(password) == 0 || LIBRT_IniciarDispositivo(dispositivos[0], password, &handle) != 0 ){
	    fprintf(stderr,"ERROR Contraseña incorrecta\n");
	    return 1;
	}
    }
    if (g_numBloque != -1){
	if ( LIBRT_EscribirBloqueCrypto (&handle, g_numBloque, buff) == 0){
	    printf("Bloque insertado correctamente en la posicion %d de bloques del Clauer\n", g_numBloque);
	}
	else{
	    fprintf(stderr,"ERROR Insertando en bloque.\n");
	    exit(1);
	}
    }
    else{
	if ( LIBRT_InsertarBloqueCrypto (&handle, buff, &numBloque) == 0){
	    printf("Bloque insertado correctamente en la posicion %ld de bloques del Clauer\n", numBloque);
	}
	else{
	    fprintf(stderr,"ERROR Insertando en bloque.\n");
	    exit(1);
	}
    }
    
    LIBRT_FinalizarDispositivo(&handle);
    
    return 0; 
}


