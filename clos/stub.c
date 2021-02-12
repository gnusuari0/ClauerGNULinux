
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

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#endif

#include <stdlib.h>
#include "stub.h"

#include "session.h"
#include "func.h"
#include "err.h"
#include "log.h"
#include "transport.h"
#include "auth.h"
#include <CRYPTOWrapper/CRYPTOWrap.h>

#ifdef LINUX
#include <unistd.h>
#include <signal.h>
#include <time.h>
#endif

#include <string.h>

static session_t *g_s = NULL;
int kk= 0;

#define RCV_TIMEOUT    10    /* seconds */


void destroy_session ( void )
{
    
    if ( g_s->tr ){
        LOG_Msg(LOG_TO, " Doing TRANS_CLOSE ...");
        TRANS_Close(g_s->tr);
     } 

    if ( g_s->hClauer )
            IO_Close(g_s->hClauer);//tr);

    if ( g_s->ib )
	    free(g_s->ib);
	if ( g_s->pwd )
		SMEM_Free(g_s->pwd, (unsigned long) (strlen(g_s->pwd)+1));
   free(g_s);
}



void timeout_handler ( int signal )
{
    
#ifdef DEBUG
  LOG_Msg(LOG_TO, "Timeout reached!!!");
  if( kk==1)
    LOG_Msg(LOG_TO, "  Listando dispositivos!!!");
  else
    LOG_Msg(LOG_TO, "  En una session !!!");
#endif
  destroy_session();
  exit(0);
}



int stub ( trans_object_t tr )
{
    unsigned char funcId = 10;
    int err;

#ifdef LINUX
    if ( SESSION_New ( SESS_RDONLY, tr, NULL, NULL, NULL, &(child_info.opt), &g_s) != CLOS_SUCCESS ) 
#elif defined(WIN32)
    if ( SESSION_New ( SESS_RDONLY, tr, NULL, NULL, NULL, NULL, &g_s) != CLOS_SUCCESS ) 
#endif 
	{
		LOG_MsgError(LOG_TO, "Impossible to create session object");
		return ERR_CLOS;
    }

#ifdef LINUX
    if ( signal(SIGALRM, timeout_handler) == SIG_ERR ) {	
		LOG_MsgError(LOG_TO, "Impossible to stablish timeout");
		return ERR_CLOS;
    }
#elif defined(WIN32)
	/*if ( TRANS_SetRcvTimeout(tr, 5000) != CLOS_SUCCESS ) {
		LOG_MsgError(LOG_TO, "Imposible to stablish timeout");
		return ERR_CLOS;
	}*/
#endif

    /* if ( TRANS_Receive(tr, &funcId, 1) != CLOS_SUCCESS ) {	
		LOG_MsgError(LOG_TO, "Receiving function");
		return ERR_CLOS;
		}*/
    
    funcId= child_info.funcId;
    LOG_Debug(LOG_TO,"Invocando funcion: %d",funcId);

    /* When a enumeration operation is performed, the library (LIBRT) 
     * opens and close a new connection to handle the request thus 
     * we haven't to do session handling, just return. The same situation is 
	 * given with the admin functions.
     */

	/* TODO: NEED TO BE CHECKED FOR ADMIN PRIVILEGES !!!
	*
	*  FUNC_ENUMERATE_USBS
	*  FUNC_CREATE_CLAUER 
	*  FUNC_GET_SIZES 
	*  FUNC_FORMAT_DATA_PARTITION 
	*  FUNC_FORMAT_CRYPTO_PARTITION
	*
	*/
#ifdef WIN32 	
	if ( AUTH_IsPidAdmin(TRANS_GetRemotePid(tr)) ){ 
		LOG_Msg(1, "The user is admin");
		g_s->admin= 1;
	}
	else{
		LOG_Msg(1, "The user is not admin");
		g_s->admin= 0;
	}
	
#else 
	// Clos allow on Linux based systems to format by default 
	// The admin control and the configuration file directive must to be 
        // added.
         g_s->admin=1;
	 LOG_Debug(LOG_TO,"Is admin= %d", g_s->admin);

#endif 
	 LOG_Debug(1,"BEGIN function exec; Switching func ID: %d ",funcId);
	 switch (funcId){
		case FUNC_ENUMERATE_DEVICES:
			
			err= FUNC_EnumerateDevices ( g_s );
			if (err != CLOS_SUCCESS){
				LOG_Error(LOG_TO, "FUNC_EnumerateUSBS has returned the value: %d", err);
				LOG_MsgError(LOG_TO, "Enumerating devices");
				destroy_session();    
				return ERR_CLOS;	
			}
			destroy_session();
			return CLOS_SUCCESS;
			
			break;
		
		case FUNC_ENUMERATE_USBS:
			LOG_Msg(1,"Enumerating USBS ... ");
		    err= FUNC_EnumerateUSBs ( g_s );  
			if (err != CLOS_SUCCESS){
				LOG_MsgError(LOG_TO, "Enumerating USBS");
				LOG_Error(LOG_TO, "FUNC_EnumerateUSBS has returned the value: %d", err);
				destroy_session();    
				return ERR_CLOS;	
			}
			destroy_session();
			return CLOS_SUCCESS;

			break;
		
		case FUNC_CREATE_CLAUER:
			LOG_Msg(1, "Invoking FUNC_CreateClauer get in ...");
			if (FUNC_CreateClauer ( g_s ) != CLOS_SUCCESS){
				LOG_MsgError(LOG_TO, "Creating Clauer");
				destroy_session();    
				return ERR_CLOS;	
			}
			destroy_session();
			return CLOS_SUCCESS;

			break;
		
		
		case FUNC_GET_LAYOUT:
			
			if ( FUNC_GetClauerLayout(g_s) != CLOS_SUCCESS){
				LOG_MsgError(LOG_TO, "Formating data zone");
				destroy_session();    
				return ERR_CLOS;	
			}
			destroy_session();
			return CLOS_SUCCESS;

			break;
		
		
		case FUNC_FORMAT_DATA_PARTITION:
			
			if ( FUNC_FormatData(g_s) != CLOS_SUCCESS){
				LOG_MsgError(LOG_TO, "Formating data zone");
				destroy_session();    
				return ERR_CLOS;	
			}
			destroy_session();
			return CLOS_SUCCESS;
			
			break;
		
		
		case FUNC_FORMAT_CRYPTO_PARTITION:
		    LOG_Msg(LOG_TO,"FUNC_FORMAT_CRYPTO_PARTITION");
		    if ( FUNC_FormatCrypto(g_s) != CLOS_SUCCESS){
			LOG_MsgError(LOG_TO, "Formating crypto zone");
			destroy_session();    
			return ERR_CLOS;	
		    }
		    destroy_session();
		    return CLOS_SUCCESS;
		    
		    break;		
	}
	 



	/* 
	* 
	* The old way! 
	*
	*/
	if ( funcId == FUNC_ENUMERATE_DEVICES ){
		kk=1;
		if ( FUNC_EnumerateDevices ( g_s ) != CLOS_SUCCESS ) {

			LOG_MsgError(LOG_TO, "Enumerating devices");
			destroy_session();   
			return ERR_CLOS;       
		}
		else{
			LOG_Msg(LOG_TO,"Saliendo del la enumeración OK");
			destroy_session();
			return CLOS_SUCCESS;
		}
	}

	if ( funcId == FUNC_GET_CLOS_VERSION ){
   	  LOG_Msg(LOG_TO,"FUNCTION_GET_CLOS_VERSION ... ");
	  if ( FUNC_GetClosVersion ( g_s) != CLOS_SUCCESS ) {
	    LOG_MsgError(LOG_TO, "Getting clos version");
	    destroy_session();   
	    return ERR_CLOS;       
	  }
	  else{
	    LOG_Msg(LOG_TO,"GOT clos version ok");
	    destroy_session();
	    return CLOS_SUCCESS;
	  }
	}

	if ( funcId == FUNC_GET_LOGICAL_UNIT ){
   	  LOG_Msg(1,"FUNCTION_GET_LOGICAL_UNIT ... ");
	  if ( FUNC_GetLogicalUnitFromPhysicalDrive ( g_s) != CLOS_SUCCESS ) {
	    LOG_MsgError(LOG_TO, "Getting logical unit");
	    destroy_session();   
	    return ERR_CLOS;       
	  }
	  else{
	    LOG_Msg(LOG_TO,"GOT logical unit ok");
	    destroy_session();
	    return CLOS_SUCCESS;
	  }
	}
	  
    /* 
     * The first function to receive is the start session
     * function a part from the admin functions.
     */

    if ( funcId != FUNC_START_SESSION ){
		LOG_Error(LOG_TO, "Expected function 1 and received %d", funcId);
		return ERR_CLOS;
    }

    if ( FUNC_StartSession ( g_s ) != CLOS_SUCCESS ) {
		LOG_MsgError(LOG_TO, "Error starting new session");
		return ERR_CLOS;
    }


    do {

	err = TRANS_Receive( tr, &funcId, 1);
	if ( err == ERR_TRANS_PEER_CLOSED ) {	    
		LOG_Msg(LOG_TO, "Connection closed by the client");
		return CLOS_SUCCESS;
	}
	
	
 LOG_Debug(LOG_TO,"Invocando funcion: %d",funcId);

#ifdef LINUX
	if ( err != CLOS_SUCCESS ) {
	    alarm(0);
	    
 LOG_MsgError(LOG_TO, "Receiving function id");
	    return ERR_CLOS;
	}
#endif

	switch ( funcId ) {
	  
	    // case FUNC_ENUMERATE_DEVICES:
	    //   err= FUNC_EnumerateDevices ( g_s );
	    //   if ( err != CLOS_SUCCESS )
	    //     goto err_stub;
	    //   break;
	  
	    case FUNC_READ_RESERVED_ZONE:

		err = FUNC_ReadReservedZone ( g_s);
		if ( err != CLOS_SUCCESS ) 
			goto err_stub;
		break;
		
	    case FUNC_WRITE_RESERVED_ZONE:

		/* If we've a session error we've no need to
                 * close the connection
   		 */
	
		err = FUNC_WriteReservedZone ( g_s );
		if ( ( err != ERR_FUNC_SESSION_IS_READONLY ) &&
                     ( err != CLOS_SUCCESS ) )
			goto err_stub;

		break;
	    case FUNC_CHANGE_PASSPHRASE:

	      err=  FUNC_ChangePassphrase ( g_s );
	      if ( err != CLOS_SUCCESS )
		goto err_stub;
	        break;

	    case FUNC_READ_INFO_ZONE:

		err = FUNC_ReadInfoZone ( g_s );
		if ( err == ERR_FUNC_ABORT ) {			
			LOG_MsgError(LOG_TO, "Error reading info zone. Aborting connection");
			goto err_stub;
		}
	    
		break;
	    case FUNC_READ_OBJECT_ZONE_BLOCK:
	      err = FUNC_ReadObjectZoneBlock ( g_s );
	      if ( err != CLOS_SUCCESS ) {		
			LOG_MsgError(LOG_TO, "Error reading object zone block.");
			goto err_stub;
	      }
	      
	      break;
	    
	    case FUNC_READ_ENUM_FIRST_OBJECT_TYPE:
	      err = FUNC_ReadEnumFirstObjectType ( g_s );
	      if ( err != CLOS_SUCCESS ) {		
			LOG_MsgError(LOG_TO, "Error reading first object block. (given a type)");
			goto err_stub;
	      }
	      
	      break;
	    
	    case FUNC_READ_ENUM_OBJECT_TYPE:
	      err = FUNC_ReadEnumObjectType ( g_s );
	      if ( err != CLOS_SUCCESS ) {		
			LOG_MsgError(LOG_TO, "Error reading object block. (given a start)");
			goto err_stub;
	      }
	    
	      break;
	    
	    case FUNC_READ_ALL_TYPE_OBJECTS:
	     
		  LOG_Msg(1,"FUNC_ReadAllTypeObjects");
	      err =  FUNC_ReadAllTypeObjects ( g_s );
	      if ( err != CLOS_SUCCESS ) {
			  LOG_MsgError(LOG_TO, "Error reading all object blocks. (given a type)");
			  goto err_stub;
	      }

		break;

	    case FUNC_READ_ALL_OCCUPED_BLOCKS:
	      err =  FUNC_ReadAllOccupedBlocks ( g_s );
	      if ( err != CLOS_SUCCESS ) {		
			LOG_MsgError(LOG_TO, "Error reading all occuped object blocks.");
			goto err_stub;
	      } 
	      
	      break;
	      
	    case FUNC_WRITE_OBJECT_ZONE_BLOCK:

	      err =  FUNC_WriteObjectZoneBlock ( g_s );

	      if ( err != CLOS_SUCCESS ) {
			 LOG_MsgError(LOG_TO, "Error writing blocks.");
			 goto err_stub;
	      } 
	      
	      break;
	    case FUNC_INSERT_OBJECT_ZONE_BLOCK:
	      err =  FUNC_InsertObjectZoneBlock ( g_s );
	      if ( err != CLOS_SUCCESS ) {		
			 LOG_MsgError(LOG_TO, "Error writing blocks.");
			 goto err_stub;
	      }

	      break;

	    case FUNC_ERASE_OBJECT_BLOCK:
	      err =  FUNC_EraseObjectBlock ( g_s );
	      if ( err != CLOS_SUCCESS ) {
		
 LOG_MsgError(LOG_TO, "Error erasing blocks.");
		goto err_stub;
	      }
	      break;
	      
	case FUNC_GET_HARDWARE_ID:
	  err =  FUNC_GetHardwareId ( g_s );
	  if ( err != CLOS_SUCCESS ) {
		
		LOG_MsgError(LOG_TO, "Error getting Hardware Id.");
		goto err_stub;
	    }
	    break;
	    
	case FUNC_CLOSE_SESSION:
	    break;
	    
	default:
		
 LOG_Error(LOG_TO, "Function not defined: %d", funcId);;
		return ERR_STUB_UNDEFINED_FUNCTION;
	}


    } while ( funcId != FUNC_CLOSE_SESSION );

    err = FUNC_CloseSession(g_s);

    return CLOS_SUCCESS;

err_stub:

	destroy_session();
	return err;

}



#ifdef WIN32


int WINAPI WinMain ( HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
)
{
	HANDLE hStdIn;
	DWORD dwBytesRead;
	trans_object_t tr;
    WSAPROTOCOL_INFO pi;

	int tamDevice, i, pid=0, tamVersion=0;

	LOG_Ini(LOG_WHERE_FILE, 1);	
	CRYPTO_Ini();
	TRANS_Init();

	hStdIn = GetStdHandle(STD_INPUT_HANDLE);

	if ( hStdIn == INVALID_HANDLE_VALUE || hStdIn == NULL ) 
		exit(1);
	
	

	dwBytesRead = 0;

	LOG_Debug(1, "sizeof pi (HIJO): %ld", sizeof(WSAPROTOCOL_INFO));

	if ( ! ReadFile(hStdIn, &pi, sizeof(WSAPROTOCOL_INFO), &dwBytesRead, NULL) ) {
		LOG_Error(1, "Imposible atender petición. No se recibió caché de dispositivos: %ld", GetLastError());
		exit(1);
	}

	LOG_Debug(0, "pi: %ld", dwBytesRead);
	dwBytesRead = 0;

	if ( ! ReadFile(hStdIn, &(child_info.nDevs), sizeof(int), &dwBytesRead, NULL) ) {
		LOG_Debug(1, "nDevs: %ld", dwBytesRead);
		dwBytesRead = 0;
		
		LOG_Error(1, "Imposible atender petición. No se recibió caché de dispositivos: %ld", GetLastError());
		exit(1);
	}

	LOG_Debug(1, "nDevs= %ld", child_info.nDevs);
	
	for ( i = 0 ; i < child_info.nDevs ; i++ ) {

		if ( ! ReadFile(hStdIn, &tamDevice, sizeof(int), &dwBytesRead, NULL) ) {
			LOG_Debug(1, "tamDevice: %ld", dwBytesRead);
			dwBytesRead = 0;

			LOG_Error(1, "Imposible atender petición. No se recibió caché de dispositivos: %ld", GetLastError());
			exit(1);
		}

		LOG_Debug(1, "tamDevice: %ld\n", tamDevice);

		child_info.devices[i] = (unsigned char *) malloc ( tamDevice );
		if ( ! child_info.devices[i] ) {
			LOG_Error(1, "Imposible atender petición. No se recibió caché de dispositivos: %ld", GetLastError());
			exit(1);
		}
		
		if ( ! ReadFile(hStdIn, child_info.devices[i], tamDevice, &dwBytesRead, NULL) ) {
			LOG_Error(1, "Imposible atender petición. No se recibió caché de dispositivos: %ld", GetLastError());
			exit(1);
		}

		child_info.ibs[i] = (block_info_t *) malloc ( sizeof(block_info_t) );
		if ( ! child_info.ibs[i] ) {
			LOG_Error(1, "Imposible atender petición. No se recibió caché de dispositivos: %ld", GetLastError());
			exit(1);
		}

		if ( ! ReadFile(hStdIn, child_info.ibs[i], sizeof(block_info_t), &dwBytesRead, NULL) ) {
			LOG_Error(1, "Imposible atender petición. No se recibió caché de dispositivos: %ld", GetLastError());
			exit(1);
		}
	}

	if ( ! ReadFile(hStdIn, &(pid), sizeof(pid), &dwBytesRead, NULL) ) {
			LOG_Error(1, "Imposible atender petición. No se recibió número de pid del cliente: %ld", GetLastError());
			exit(1);
	}
	
	if ( ! ReadFile(hStdIn, &(child_info.funcId), sizeof(child_info.funcId), &dwBytesRead, NULL) ) {
			LOG_Error(1, "Imposible atender petición. No se recibió número de función inicial: %ld", GetLastError());
			exit(1);
	}
 
	 
	if ( ! ReadFile(hStdIn, &(child_info.sw_type), sizeof(child_info.sw_type), &dwBytesRead, NULL) ) {
			LOG_Error(1, "Imposible atender petición. No se recibió número de función inicial: %ld", GetLastError());
			exit(1);
	}
 
	if ( ! ReadFile(hStdIn, &tamVersion, sizeof(tamVersion), &dwBytesRead, NULL) ) {
			LOG_Error(1, "Imposible atender petición. No se recibió número de función inicial: %ld", GetLastError());
			exit(1);
	}

	if (tamVersion != 0){
		child_info.sw_version= (char*)malloc(tamVersion);
		if ( ! ReadFile(hStdIn, child_info.sw_version, tamVersion, &dwBytesRead, NULL) ) {
			LOG_Error(1, "Imposible atender petición. No se recibió número de función inicial: %ld", GetLastError());
			exit(1);
		}
	}



	LOG_Debug(1, "FUNCID: %ld\n", child_info.funcId);

	CloseHandle(hStdIn);


	if ( TRANS_WIN_ChildCreate ( pi, &tr ) != CLOS_SUCCESS ){
        LOG_Msg(1, "Fallo Trans_Win_ChildCreate.");    	
		exit(1);
	}


	TRANS_SetRemotePid(tr, pid);

 	LOG_Msg(1, "TRANS_SetRemotePid.");    	

	if ( stub(tr) != CLOS_SUCCESS ){
		LOG_Msg(1, "Saliendo por error.");    		
		exit(1);
	}

	LOG_Msg(1, "Clos-client saliendo correctamente");
	ExitProcess(0);
}

#endif
