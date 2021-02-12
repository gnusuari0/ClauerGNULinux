
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
#include "common.h"
#include "string.h"
#include <stdio.h>


void usage ( void )
{
    fprintf(stderr, "modo de empleo: clos [-h] | -c -f \n");
    fprintf(stderr, "Sistema opeartivo del Clauer\n\n");
    fprintf(stderr, "Las opciones posibles son:\n");
    fprintf(stderr, "   -h, --help              Imprime este mensaje de ayuda.\n");
    fprintf(stderr, "   -c, --cryf              Habilita el soporte de ficheros en formato clauer desde cdrom.\n");
    fprintf(stderr, "   -f, --fconfig   arg     Ruta el fichero de configuración en el sistema, por defecto /etc/clos.conf.\n");
    /* fprintf(stderr, "   -f, --floppy            Habilita el soporte de ficheros en formato clauer también busca en la disquetera.\n\n"); */
}


void parse( int argc, char ** argv,  options_t * opts ){

    int i, l;
    FILE * fd; 
    
    opts->cryf=   0;
    opts->floppy= 0;
    opts->config= 0;

    for ( i=1 ; i< argc ; i++ ){
	if ( ( strncmp(argv[i], "-h", 2 ) == 0 )  || ( strncmp(argv[i], "--help", 6 ) == 0 ) ){
	    usage();
	    exit(0);
	}
	else if ( ( strncmp(argv[i], "-c", 2 ) == 0 )  || ( strncmp(argv[i], "--cryf", 6 ) == 0 ) ){
	    opts->cryf= 1;
	}
	else if ( ( strncmp(argv[i], "-f", 2 ) == 0 )  || ( strncmp(argv[i], "--fconfig", 6 ) == 0 ) ){
	    if (argc <= i+1){
               fprintf(stderr, "Debe proporcionar un argumento a la opción %s.\n",argv[i]);
            }
	    else{
		fd= fopen(argv[i+1],"r");
		if (!fd) 
		    fprintf(stderr, "No puedo abrir %s para lectura.\n", argv[i+1]);
		else{
                    l= strlen(argv[i+1]);
		    opts->config= malloc(l*sizeof(char));
		    strncpy(opts->config, argv[i+1], l);
                    i++;
		}
	    }
	}
/*	else if ( ( strncmp(argv[i], "-f", 2 ) == 0 )  || ( strncmp(argv[i], "--floppy", 8 ) == 0 ) ){
	    opts->floppy= 1;
	    opts->cryf= 1;
	}
*/
	else{
	    fprintf(stderr, "%s Opción desconocida.\n",argv[i]);
	    exit(-1);
	}
    }
}

void parse_config_file ( char * file, child_info_t * ch_info ){

    int fsize, type, i, idx=0, state=1;
    FILE * fd;
    char buff[BUFF_SIZE+1],aux_buff[BUFF_SIZE+1], * version;
    char * p, * nl, * eq, *ini, *end;

    fd= fopen(file,"r");
    if (!fd){
	fprintf(stderr,"[Warning] Configuration file at %s not found.\n", file);
	version= malloc(10);
	strncpy(version,"0.0.0",5);
	ch_info->sw_type=0; 
	ch_info->sw_version=version;
	return;
    }
    
    fsize= fread(buff,1,BUFF_SIZE,fd);
    
    if (fsize<BUFF_SIZE)
	buff[fsize]= 0;
    else
	buff[BUFF_SIZE]=0;
    
    //strip out comments
    //state=1 -> copy
    //state=0 -> not to copy
    
    for(i=0; i<BUFF_SIZE; i++){
	if (buff[i]=='#')
	    state=0;
	if (buff[i]=='\n')
	    state=1;
	if (state)
	    aux_buff[idx++]=buff[i];
	
    }
    aux_buff[idx]=0;
    
    //Parse for type
    p= strstr(buff,"type");
    if (!p){
	fprintf(stderr, "type not found in config, assuming 0.");
	type=0;
    }
    else{
	nl= strstr(p,"\n");
	eq= strstr(p, "=");
	if (nl && eq){
	    type=atoi(eq+1);
	}
    }
    
    //Parse for version
    //That will be available for pointing outside this function
    version= malloc(10);
    p= strstr(buff,"version");
    if (!p){
	fprintf(stderr, "version not found in config, setting 0.0.0");
	strncpy(version,"0.0.0",5);
    }
    else{
	eq= strstr(p, "=");
	ini=eq+1;
	end=eq+1;
	while(*end!='#' && *end!='\n' && *end!=0) end++;
	*end=0;
	//Take out whitespaces:
	// leading ones
	while ( *ini==' ' && ini!=end )ini++;
	// Trailing ones
	while ( end!=ini && *(end-1)==' ') {*(end-1)=0; end--;}
	strncpy(version,ini,10);
    }
    
    ch_info->sw_type=type; 
    ch_info->sw_version=version;
}

#ifdef WIN32
int get_version_and_type(child_info_t * ch_info) {
/* Obtenemos Version del Software y tipo */
	DWORD type, tamValue;
	HKEY hKey = NULL; 

	ch_info->sw_type= 0;
	ch_info->sw_version= NULL;
	
	if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Universitat Jaume I\\Projecte Clauer",
		0, KEY_READ, &hKey) != ERROR_SUCCESS ) {
			LOG_Debug(1,"Error Abriendo el registrooolll LastError= %d", GetLastError());
	}
	else{
		tamValue= sizeof(type);
		RegQueryValueEx(hKey, "SW_TYPE", NULL, &type, (LPBYTE)&(ch_info->sw_type), &tamValue);
		LOG_Debug(1,"Obtenido SW_TYPE= %d", ch_info->sw_type);
		RegQueryValueEx(hKey, "SW_VERSION", NULL, &type, NULL, &tamValue);
		LOG_Debug(1,"Obtenido tamaño version= %d", tamValue);
		ch_info->sw_version= (BYTE *)malloc(tamValue + 1);
		
		RegQueryValueEx(hKey, "SW_VERSION", NULL, &type, ch_info->sw_version, &tamValue);
		if (ch_info->sw_version != NULL){
			ch_info->sw_version[tamValue]=0;
			LOG_Debug(1,"Obtenido tamaño sw_version= %s", ch_info->sw_version);
		}		
	}	

  return 0;
}
#endif

void parse_config(child_info_t * ch_info) {
    
    if (!ch_info->opt.config)
	parse_config_file(DEFAULT_FILE, ch_info);
    else
	parse_config_file(ch_info->opt.config, ch_info);
 
}
