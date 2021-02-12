
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

#include "common.h"
#ifdef WIN32
#include "windows.h"
#include <stdio.h>
#endif

#ifdef LOG
#include "log.h"
#endif 

// La funcion relleno hace una copia de una cadena y
// realiza un relleno con espacios hasta que la cadena tenga
// la longitud que indique el parametro longitudConRelleno
char * padding(char *destino, const char *origen, unsigned int longitudConRelleno)
{
    unsigned int i;

    //Copiamos la cadena origen a la cadena destino
    for(i = 0; i < strlen(origen); i++)
	destino[i] = origen[i];
    //Completamos con espacios
    for(; i < longitudConRelleno; i++)
	destino[i] = ' ';
    //Nota: la cadena queda sin marcar

    return destino;

}


CK_ATTRIBUTE_PTR buscarEnPlantilla(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount, CK_ATTRIBUTE_TYPE type)
{
    unsigned long contador;

    contador = 0;

    //LOG_Debug(LOG_TO, "buscando en plantilla el tipo: %02x", type);

    while(contador < ulCount)
        {
	    if (pTemplate[contador].type == type){
		//LOG_Msg(LOG_TO, "Tipo encontrado !!!");
		return &pTemplate[contador];
	    }
	    else
		contador++;
        }
    return NULL_PTR;
}


// La funcion destruirPlantilla libera la memoria que se alojo
// para la plantilla
void destruirPlantilla(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount)
{
    unsigned int contador;

    for (contador = 0; contador < ulCount; contador++)
	if(pTemplate[contador].pValue)
	    free(pTemplate[contador].pValue);
    free(pTemplate);
}

// From clauer-utils
void SMEM_Destroy ( void *buf, unsigned long size )
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




void parse_config_file ( const char * file, char ** r_version, int * r_type ){

    int fsize, type, i, idx=0, state=1;
    FILE * fd;
    char buff[BUFF_SIZE+1],aux_buff[BUFF_SIZE+1], * version;
    char * p, * nl, * eq, *ini, *end;

    fd= fopen(file,"r");
    if (!fd){
        fprintf(stderr,"[Warning] Configuration file at %s not found.\n", file);
        version= (char*) malloc(10);
        strncpy(version,"0.0.0",5);
        *r_type=0;
        *r_version=version;
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
    version= (char *) malloc(10);
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

    *r_type=type;
    *r_version=version;
}





// Languaje on windows
//1034 espa�ol
//1027 catala
//otro ingles
DWORD IDIOMA_Obtener (void)
{
    char * version;
    int type; 

#ifdef WIN32
    HKEY hKey;
    DWORD tamValue;
    char * idioma;
    DWORD res;
    
    res = EN;
        
    if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		      "SOFTWARE\\Universitat Jaume I\\Projecte Clauer",
		      0,
		      KEY_QUERY_VALUE,
		      &hKey) != ERROR_SUCCESS )
        {
	    //			LOG_MsgError(LOG_TO, "Error 2");   
	    return res;
        }
    
    if ( RegQueryValueEx(hKey,"Idioma", NULL, NULL, NULL, &tamValue) != ERROR_SUCCESS ) {
	RegCloseKey(hKey);
	//				LOG_MsgError(LOG_TO, "Error 3");   
	return res;
    }
    
    if ( tamValue > 5 ) {
	RegCloseKey(hKey);
	//				LOG_MsgError(LOG_TO, "Error 4");   
	return res;
    }
    
    idioma = new char [tamValue];
    
    if ( !idioma ) {
	RegCloseKey(hKey);
	//				LOG_MsgError(LOG_TO, "Error 5");   
	return res;
    }
    
    if ( RegQueryValueEx(hKey,"Idioma", NULL, NULL, (BYTE *)idioma,
			 &tamValue) != ERROR_SUCCESS ) {
	delete [] idioma;
	idioma = NULL;
	RegCloseKey(hKey);
	//				LOG_MsgError(LOG_TO, "Error 6");   
	return res;
    }
    
    RegCloseKey(hKey);
    
    if ( strncmp(idioma,"1034",4) == 0) 
	return ES;
    else if ( strncmp(idioma,"1027",4) == 0 )
	return CA;
    
    return res;
    
#else 

    // Here is Linux or Mac so /etc/clos.conf is available for parsing. 
    parse_config_file(DEFAULT_FILE,&version,&type);
    //printf("Version: %s && type: %d\n", version, type);
    switch (type){
    case 0:
	return CA;
    case 1:
	return CA_ACCV;
    case 2: 
	return CA_IDCAT;
    case 3:
	return CA_COITAV;
    }
    return ES;
#endif        
}
