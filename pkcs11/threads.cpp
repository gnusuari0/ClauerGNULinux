
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

#include "threads.h"
#include "log.h"
#include <LIBRT/libRT.h>


// Here we must receive the clauer id.
#ifdef LINUX
void * updateIdThread(void * ptr) {
#elif defined(WIN32)
    DWORD WINAPI updateIdThread(void * ptr) {
#endif

	USBCERTS_HANDLE handle; 
	unsigned char * dispositivos[MAX_DEVICES];
	int err, nDispositivos;
	ID_ACCESS_HANDLE * th_id_handle = ( ID_ACCESS_HANDLE *) ptr;
    
#ifdef WIN32
	map<string, PASS_CACHE_INFO *>::iterator iKey;
	string str_cache_aux;
#endif 
    

	while (1) {
	    /* First we decrement the ttl of pass_cache */
#ifdef WIN32 
#ifdef CLUI	    
	    WaitForSingleObject(th_id_handle->keyIdMutex, INFINITE);
	    for ( iKey = th_id_handle->keyCache.begin(); iKey != th_id_handle->keyCache.end(); ) {
		str_cache_aux= iKey->first;      
		th_id_handle->keyCache[str_cache_aux]->ttl--;
		
		if ( ! th_id_handle->keyCache[str_cache_aux]->ttl ){
		    iKey= th_id_handle->keyCache.erase(iKey);
		}else{
		    iKey++;
		}
	    }
	    ReleaseMutex(th_id_handle->keyIdMutex);

#endif	 
#endif	    
	    LOG_Msg(LOG_TO, "Entrando en el thread");
	    err= LIBRT_ListarDispositivos(&nDispositivos,dispositivos);
	    if ( err == ERR_LIBRT_SI)
		{ 
		    LOG_MsgError(LOG_TO, "Error en el thread listando dispositivos");
#ifdef LINUX
		    pthread_mutex_lock( &th_id_handle->clauerIdMutex );
#elif defined(WIN32)		
		    WaitForSingleObject(th_id_handle->clauerIdMutex, INFINITE);
#endif		    
		    memset(th_id_handle->newClauerId,0,CLAUER_ID_LEN);

#ifdef LINUX
		    LOG_Msg(LOG_TO, "Bloqueando mutex en err ListarDispositivos");
		    pthread_mutex_unlock( &th_id_handle->clauerIdMutex );
#elif defined(WIN32)	
		    ReleaseMutex(th_id_handle->clauerIdMutex);
#endif		    
		}
		else{
			LOG_Debug(LOG_TO, "NDISPOSITIVOS: %d", nDispositivos);

			if ( nDispositivos == 0 ){
			// No clauers, no objects, everything ok.
	  
#ifdef LINUX
			LOG_Msg(LOG_TO, "Bloqueando mutex en nDispositivos == 0");
			pthread_mutex_lock( &th_id_handle->clauerIdMutex );
#elif defined(WIN32)
			WaitForSingleObject(th_id_handle->clauerIdMutex, INFINITE);
#endif      
			LOG_Msg(LOG_TO, "Poniendo id a ceros");
			memset(&th_id_handle->newClauerId,0,CLAUER_ID_LEN);

#ifdef LINUX
			LOG_Msg(LOG_TO, "Desbloqueando mutex");
			pthread_mutex_unlock( &th_id_handle->clauerIdMutex );
#elif defined(WIN32)	
			ReleaseMutex(th_id_handle->clauerIdMutex);
#endif

			}
			else{
				err= LIBRT_IniciarDispositivo( dispositivos[0], NULL, &handle );
				if ( !err ){
					LOG_Msg(LOG_TO, "Bloqueando mutex en el inicio de dispositivo");
#ifdef LINUX
					pthread_mutex_lock( &th_id_handle->clauerIdMutex );
#elif defined(WIN32)
					WaitForSingleObject(th_id_handle->clauerIdMutex, INFINITE);
#ifdef CLUI	
					th_id_handle->global_pass_ttl--;
					if ( ! th_id_handle->global_pass_ttl ){
						memset(&th_id_handle->newClauerId,0,CLAUER_ID_LEN);
					}
					else{
						memcpy( &th_id_handle->newClauerId, &handle.idDispositivo, CLAUER_ID_LEN );
					}
#else
					memcpy( &th_id_handle->newClauerId, &handle.idDispositivo, CLAUER_ID_LEN );
#endif
					LIBRT_FinalizarDispositivo(&handle);
#endif
#ifdef LINUX
					memcpy( &th_id_handle->newClauerId, &handle.idDispositivo, CLAUER_ID_LEN );
					LIBRT_FinalizarDispositivo(&handle);
					pthread_mutex_unlock( &th_id_handle->clauerIdMutex );
#elif defined(WIN32)
					ReleaseMutex(th_id_handle->clauerIdMutex);
#endif
			}
			else{
				LOG_Error( LOG_TO, "Imposible inicializar dispositivo: %s", dispositivos[0] );
				memset(&th_id_handle->newClauerId,0,CLAUER_ID_LEN);
			}
		}
		}
#ifdef LINUX
	    sleep(CHECK_FOR_CLAUER_INTERVAL);
#elif  defined(WIN32)	
	    SleepEx(CHECK_FOR_CLAUER_INTERVAL * 1000, FALSE);
#endif
	}

#ifdef WIN32
	return 0;
#endif
	
    }
