
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
#include <string.h>

#include "err.h"
#include "block.h"
#include "clio.h"
#ifdef DEBUG
#include "log.h"
#endif
#include "session.h"
#include "smem.h"
#include "transport.h"
#include "auth.h"
#include "stub.h"
#include "common.h"

#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include <CRYPTOWrapper/CRYPTOWrap.h>



extern int errno;
trans_object_t tr_listener, tr_client;

void exit_fun(int sig_num)
{
    if ( TRANS_Close(tr_listener) != CLOS_SUCCESS ) {
	exit(1);
    }
    exit(0);
}

int main ( int argc, char ** argv )
{
    int ret, checksum=-1, aux_checksum, mode, i, j, err= 0;
    block_info_t  aux_block;
    clauer_handle_t hClauer;   
    
    memset( &child_info, 0, sizeof(child_info) );
    
    options_t opts;
    if ( argc > 0 ) 
	parse(argc, argv, &(child_info.opt));
  
    /* If the configuration file has been privided from command line, 
     * parse this, /etc/clos.conf otherwise.
     */
    parse_config(&child_info);

    /* Aqui va el fork que desvincula el clos del terminal 
     * y cierra todos los descriptores
     */

    ret= fork();
    if ( ret != 0  ){
  	// printf("%d",ret);
	fflush(stdout);
  	close(0);
	close(1);
  	close(2);
  	exit(0);	
    }
    close(0);
    close(1);
    close(2);
  
  

    LOG_Ini(LOG_TO, 2);


    CRYPTO_Ini();


    if ( signal(SIGCHLD, SIG_IGN) == SIG_ERR ) {
	LOG_Error(LOG_TO, "Impossible ignore SIGCHLD signal");
    }

    if ( signal(SIGTERM, exit_fun) == SIG_ERR ) {  
	LOG_Error(LOG_TO, "Impossible ignore SIGCHLD signal");
    }


    ret = TRANS_Create(&tr_listener);
    if ( ret != CLOS_SUCCESS ) {
	LOG_Error(LOG_TO, "Impossible to create listener object");
    }
  
    mode= IO_ENUM_USB;
 
    if ( child_info.opt.cryf && child_info.opt.floppy )
	mode= IO_ENUM_ALL;
  
    else if ( child_info.opt.cryf )
	mode= IO_ENUM_FILES;
    err=0;
    while ( 1 ) {
  
	ret = TRANS_Accept(tr_listener, &tr_client);

	if ( ret != CLOS_SUCCESS ) {
	    sleep(15);
	    err++;
	    
	    LOG_Debug(LOG_TO, "Error aceptando conexiones, err= %d", err);

	    if (err == 3){
		LOG_Error(LOG_TO, "Error trying to accept connections");
		return 1;
	    }
		
	    continue;
	    
	}
	else
	    err=0;

	LOG_Debug(LOG_TO, "Atendiendo la peticion devolviendo el error a err= %d", err);
       
    
	if ( TRANS_Receive(tr_client, &(child_info.funcId), 1) != CLOS_SUCCESS ) {	
	    LOG_MsgError(LOG_TO, "Receiving function");
	    TRANS_Close(tr_client);
	    continue; 
	}

	aux_checksum= IO_Get_Checksum();
      
	LOG_Debug(LOG_TO, "aux_checksum (calculado) = %d", aux_checksum);
	LOG_Debug(LOG_TO, "checksum (original) = %d", checksum);
    
	/* Si la funcion es la 15 forzamos la relectura */
	if ( child_info.funcId == FUNC_REGENERATE_CACHE ){ 
		LOG_Debug(LOG_TO, "Atendiendo REGENERACION DE CACHE f15");
	 	checksum= -1;
	}		
   	
        LOG_Debug(LOG_TO, "checksum= %d, aux_checksum= %d\n", checksum, aux_checksum);	
	if ( checksum != aux_checksum ){
	    for( i=0 ; i < child_info.nDevs && i < IO_MAX_DEVICES  ; i++ ){
		free( child_info.devices[i] );
		free( child_info.ibs[i] );
	    }
	
	    checksum= aux_checksum;
	
	    if ( IO_EnumClauers(( unsigned char * ) &(child_info.nDevs), child_info.devices, mode) != IO_SUCCESS ) {
		LOG_Error(LOG_TO, "Enumerando clauers");
		err= 1;
	    }
	    LOG_Debug(LOG_TO, "Regenerando cache child_info.nDevs= %d",child_info.nDevs);	
	    for( i=0 ; i < child_info.nDevs && i < IO_MAX_DEVICES  ; i++ ){	    
		child_info.ibs[i] = ( block_info_t * ) malloc(sizeof(block_info_t));

		ret = IO_Open(child_info.devices[i], &(hClauer), SESS_RDONLY, IO_CHECK_IS_CLAUER );	    
		if ( ret != IO_SUCCESS ) {
		    LOG_Error(LOG_TO, "Abriendo dispositivo");
		    err= 1;
		}
	    
		ret = IO_ReadInfoBlock( hClauer, child_info.ibs[i] );
		if ( ret != IO_SUCCESS ) {
		    LOG_Error(LOG_TO, "Leyendo Bloque de información");
		    err= 1;
		}
	   	LOG_Debug(LOG_TO, "Pasamos por el FOR"); 
		ret = IO_Close( hClauer );
		if ( ret != IO_SUCCESS ) {
		    LOG_Error(LOG_TO, "Cerrando dispositivo");
		    err= 1;
		}
		
		// memcpy( child_info.ids[i], aux_block.id, ID_LEN );
	    }
	
	}
    
	if ( child_info.funcId==FUNC_REGENERATE_CACHE ){
	    if ( err ){
		FUNC_SendResult(tr_client, 1); //FUNC_ERROR
		LOG_Error(LOG_TO, "Errores previos al crear la cache.");
		checksum= -1;
		err= 0;
		continue;
		
	    }
	    else{
		LOG_Debug(LOG_TO, "Fin creación de chache, enviando ok al cliente.");
		FUNC_SendResult(tr_client, 0); // FUNC_OK
                err= TRANS_Close(tr_client);
                if (err!=0) {
                     LOG_Error(LOG_TO, "Error closing socket when regenerating cache: %d", err);
                }
		continue;
	    }
	}
	else if ( err ) {
	    LOG_Error(LOG_TO, "Errores previos al crear la cache");
	    checksum= -1;
    	    err= 0;	    
	}
    

	ret = fork();
	if ( ret == -1 ) {

	    LOG_Error(LOG_TO, "Error doing a fork() : %d", errno);

	    TRANS_Close(tr_client);

	} else if ( ret == 0 ) {
	
	    /* Child code */
      
	    if ( TRANS_Close(tr_listener) != CLOS_SUCCESS ) {
	

		LOG_Error(LOG_TO, "Error closing listener");

		exit(1);
	    }
      
	    if ( stub(tr_client) != CLOS_SUCCESS ) {
	
		LOG_Error(LOG_TO, "Error in the stub");

		exit(1);
	    }

	    exit(0);

	}

	if ( TRANS_Close(tr_client) != CLOS_SUCCESS ) {
      
	    LOG_Error(LOG_TO, "Error closing client : parent");

	    exit(1);
	}
    
    }


    return 0;
}

