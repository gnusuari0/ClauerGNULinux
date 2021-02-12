
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

//---------------------------------------------------------------------------

#include "libRT.h"
#include "log.h"


int gInitialized= 0;

/*! \brief Realiza una reconexión pidiendo el pin al usuario si es posible. Si no lo es
 *         devuelve error.
 *
 * Realiza una reconexión pidiendo el pin al usuario si es posible. Si no lo es
 * devuelve error. Los casos posibles de error:
 *
 *      -# El usuario ha cancelado la operación. Puede ser el caso en el que
 *	       se disponga de una interfaz gráfica.
 *
 *      -# El usuario ha seleccionado un dispositivo cuyo identificador no coincide
 *		   con el actual.
 *
 *      -# Algún otro error
 *
 * \param handle
 *		  El handle al runtime
 *
 * \retval 0
 *		   Ok
 *
 * \retval !=0
 *		   Error - no se pudo realizar la reconexión
 */

int LIBRT_Reconectar (USBCERTS_HANDLE *handle)
{
    int nDispositivos, dispSel, ret = 0,i= 0;
	#ifdef NO_CRYF_SUPPORT
	unsigned char *dispositivos = NULL;
	#else
	unsigned char *dispositivos[MAX_DEVICES];
	#endif
	unsigned char * idDispositivo[20];
	char *pin = NULL;
	LIBRT_PIN_cb cb;

#ifdef WIN32
	HANDLE hHeap = NULL;
#endif


	if ( !handle ) {
	  ret = 1;
	  goto finLIBRT_Reconectar;
	}

	if ( ! handle->pin_cb ) {
	  ret = 1;
	  goto finLIBRT_Reconectar;
	}

	/* Cerramos el socket por si acaso
	 */

	LIBMSG_Cerrar(handle->sock);

	#ifdef NO_CRYF_SUPPORT 
	if ( LIBRT_ListarDispositivos ( &nDispositivos, &dispositivos) != 0 ) {
	  ret = 1;
	  goto finLIBRT_Reconectar;
	}
        #else
	if ( LIBRT_ListarDispositivos ( &nDispositivos, dispositivos) != 0 ) {
	  ret = 1;
	  goto finLIBRT_Reconectar;
	} 
	#endif
	
	if ( nDispositivos == 0 ) {
	  ret = 1;
	  goto finLIBRT_Reconectar;
	}
	
	/* Ahora pedimos el pin
	 */
#ifdef WIN32       
	hHeap = HeapCreate(HEAP_NO_SERIALIZE, 100, 100);
	if ( !hHeap ) {
	  ret = 1;
	  goto finLIBRT_Reconectar;
	}
	
	pin = HeapAlloc(hHeap, HEAP_ZERO_MEMORY | HEAP_NO_SERIALIZE, 100);
	if ( !pin ) {
	  ret = 1;
	  goto finLIBRT_Reconectar;
	}
#elif defined(LINUX)

	pin = (char *) malloc (sizeof(char) * 100);

	if ( ! pin ) {
	  ret = 1;
	  goto finLIBRT_Reconectar;
	}

#endif
	
	#ifdef NO_CRYF_SUPPORT
	if ( handle->pin_cb(handle, dispositivos, nDispositivos, handle->appData, pin, &dispSel) != 0 ) {
	  ret = 1;
	  goto finLIBRT_Reconectar;
	}
        #else
	if ( handle->pin_cb(handle, dispositivos, nDispositivos, handle->appData, pin, &dispSel) != 0 ) {
	  ret = 1;
	  goto finLIBRT_Reconectar;
	}
	#endif
	
	/* Inicializamos el dispositivo
	 */
	
	memcpy(idDispositivo, handle->idDispositivo, 20);
	
	cb = handle->pin_cb;

	#ifdef NO_CRYF_SUPPORT
	if ( LIBRT_IniciarDispositivo(dispSel, pin, handle) != 0 ) {
		ret = 1;
		goto finLIBRT_Reconectar;
	}
        #else
	if ( LIBRT_IniciarDispositivo(dispositivos[dispSel], pin, handle) != 0 ) {
		ret = 1;
		goto finLIBRT_Reconectar;
	}
	#endif

	//SecureZeroMemory(pin, 100);
	memset(pin, 0, 100);

#ifdef WIN32
	HeapFree(hHeap, HEAP_NO_SERIALIZE, pin);
	HeapDestroy(hHeap);
	hHeap = NULL;
#endif

	pin = NULL;


	handle->pin_cb = cb;

	/* Si los identificadores de ambos dispositivos son distintos
	 * pues a fer la mà
	 */

	if ( memcmp(idDispositivo, handle->idDispositivo, 20) != 0 ) {
		LIBRT_FinalizarDispositivo(handle);
		ret = 1;
		goto finLIBRT_Reconectar;
	}

finLIBRT_Reconectar:

	#ifdef NO_CRYF_SUPPORT
	if ( dispositivos ) {
	  free(dispositivos);
	  dispositivos = NULL;
	}
        #else
	if ( dispositivos ) {
	    for ( i=0 ; i<MAX_DEVICES ; i++ ){
		free(dispositivos[i]);
		dispositivos[i] = NULL;
	    }
	}
	#endif

	if ( pin ) {
	  memset(pin, 0, 100);
	  pin = NULL;
#ifdef WIN32
	  HeapFree(hHeap, HEAP_NO_SERIALIZE, pin);
#endif
	}

#ifdef WIN32
	if ( hHeap ) {
		HeapDestroy(hHeap);
		hHeap = NULL;
	}
#endif

	return ret;

}





/*! \brief Inicializa la librería. Es necesario llamar a esta función antes de utilizar el resto.
 *
 * Inicializa la librería. Es necesario llamar a esta función antes de utilizar el resto. Es un error
 * bastante común no llamar a esta función. Así es que si algo va mal y no sabes por qué... tachan, tachan...
 * llama a LIBRT_Ini()
 *
 * \retval 0
 *		   OK
 *
 * \retval != 0
 *		   FALLO
 */

int LIBRT_Ini (void)
{
	int ret;

#ifdef LOG
	LOG_Ini(LOG_WHERE_FILE, 1);
#endif

	if (! gInitialized ){
		gInitialized= 1;	
		LOG_Msg(1, "Inicializando LIBRT ");
		ret= LIBMSG_Ini();
		LOG_Debug(1,"Devolviendo ret= %d", ret);
		return ret;
	}
	return 0;
}


/*! \brief Finaliza la librería. Es conveniente llamar a esta función cuando ya no tengamos que usar LIBRT.
 *
 * Finaliza la librería. Es conveniente llamar a esta función cuando ya no tengamos que usar LIBRT. Libera
 * recursos.
 *
 * \retval 0
 *		   OK
 *
 * \retval != 0
 *		   FALLO
 */

int LIBRT_Fin (void)
{
#ifdef LOG 
        LOG_End( );
#endif
	if ( gInitialized ){ 
		gInitialized= 0;
		return LIBMSG_Fin();
	}
	return 0;
}


/*! \brief Finaliza la utilización de un dispositivo. Cuando ya no vayamos a interactuar más con el clauer
 *         hay que llamar a esta función.
 *
 * Finaliza la utilización de un dispositivo. Cuando ya no vayamos a interactuar más con el clauer
 * hay que llamar a esta función.
 *
 * \retval 0
 *		   OK
 *
 * \retval != 0
 *		   FALLO
 */

int LIBRT_FinalizarDispositivo(USBCERTS_HANDLE *handle)
{

	unsigned char err = 0,func = 20;

	if ( ! handle ){
		LOG_MsgError(1,"Param incorrecto");	
		return ERR_LIBRT_PARAMETRO_INCORRECTO;
	}
	if ( !LIBRT_MUTEX_Lock(handle->mutex) ){ 
		LOG_MsgError(1,"En el bloqueo del mutex");	
		return ERR_LIBRT_SI;
	}

	/*
	 * Envío la petición. La atención de la reconexión se hace sólo en la
	 * primera función.
	 */

	if ( LIBMSG_Enviar(handle->sock, (unsigned char *) &func, 1) != ERR_LIBMSG_NO ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		LOG_MsgError(1,"Enviando número de func 1");	
		return ERR_LIBRT_SI;
	}

	/*
	 * Recibo la respuesta
	 */

	if ( LIBMSG_Recibir(handle->sock, (unsigned char *)&err, 1) != 0 ){
		LOG_MsgError(1,"Recibiendo el ERROR");		
		return ERR_LIBRT_SI;
	}
	
	if ( err != 0 ) {
		LOG_Debug(1,"Encontrado error = %d", err);	
		return ERR_LIBRT_SI;
	}

	LIBMSG_Cerrar(handle->sock);	

	LIBRT_MUTEX_Unlock(handle->mutex);


	return 0;
}



/*! \brief F0. Lista los clauers insertados en el sistema.
 *
 * F0. Lista los clauers insertados en el sistema.
 *
 * \param nDispositivos
 *        SALIDA. El número de dispositivos disponibles.
 *
 * \param dispositivos
 *		  SALIDA. Array con los números de dispositivos disponibles. La función reserva
 *		  espacio para esta variable. Es responsabilidad del programador liberarla
 *		  con free().
 *
 * \retval 0
 *		   OK
 *
 * \retval != 0
 *		   FALLO
 */

int LIBRT_ListarDispositivos ( int *nDispositivos, unsigned char **dispositivos)
{
    int len= 0, i= 0, err=0;
    unsigned char aux= 0, funcion=0, acaba=0;
    SOCKET sock;
    
	if ( ! nDispositivos )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;
	if ( ! dispositivos )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;


	/*
	 * Petición de la función
	 */
	
	LOG_Msg(1, "Conectamos ... ");
	
	if ( (err=LIBMSG_Conectar(IP, PORTNUMBER, &sock)) != 0 ){
		LOG_Error(1, "Error al conectar ret= %d ... ", err);
		return ERR_LIBRT_SI;
	}

	LOG_Msg(1, "Enviamos la función 0");	
	if ( LIBMSG_Enviar(sock, (unsigned char *) &funcion, 1) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el número de función ... ");
		return ERR_LIBRT_SI;
	}

	/*
	 * Respuesta del runtime
	 */
	LOG_Msg(1, "Recibimos respuesta del runtime");
	if ( (err= LIBMSG_Recibir(sock, (unsigned char *)&aux,1)) != 0 ) {
        //err= GetLastError();
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al recibir el resultado del error ... ");
		return ERR_LIBRT_SI;
	}

	*nDispositivos = aux;

	#ifndef NO_CRYF_SUPPORT
	// Check we have no more than MAX_DEVICES devices 
	if ( aux > MAX_DEVICES ) {
	    LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error demasiados dispositivos ... ");
		return ERR_LIBRT_SI;
	}
	#endif
	
	if ( aux > 0 ) {
		LOG_Msg(1, "Encontrados dispositivos");	
		//dispositivos = (unsigned char *) malloc (*nDispositivos);
	    
		#ifdef NO_CRYF_SUPPORT
		for ( i = 0 ; i < *nDispositivos ; i++ ) {
			if ( LIBMSG_Recibir(sock,(*dispositivos)+i,1) != 0 ) {
				LIBMSG_Cerrar(sock);
				return ERR_LIBRT_SI;
			}
		}
		//printf("Paso NO_CRYF_SUPPORT\n");
		#else
		for ( i = 0 ; i < *nDispositivos ; i++ ) {
		    // Receiving size of the next string
			LOG_Msg(1, "Recibiendo len de dispositivo");
		    if ( (err=LIBMSG_Recibir(sock,(unsigned char *)&len ,4)) != 0 ) {
				LIBMSG_Cerrar(sock);
				LOG_Error(1, "Error recibiendo el tamaño de la siguiente cadena error = %d ... ", err);
				return ERR_LIBRT_SI;
		    }
			LOG_Debug(1, "Tamaño de cadena recibido len= %d", len );
		    dispositivos[i]= (unsigned char *) malloc ( len * sizeof(char) + 1 );
		    LOG_Msg(1, "Recibiendo dispositivo");
		    if ( (err=LIBMSG_Recibir(sock,dispositivos[i],len)) != 0 ) {
				LIBMSG_Cerrar(sock);
				LOG_Error(1," Al recibir la cadena error= %d", err);
				return ERR_LIBRT_SI;
		    }
		    dispositivos[i][len]='\0';
			LOG_Debug(1, "Recibida cadena str= %s", dispositivos[i] );
		}
		//printf("NO Paso NO_CRYF_SUPPORT\n");
                #endif
	}

	/*if ( LIBMSG_Enviar(sock, (unsigned char *) &acaba, 1) != 0 ) {
		LIBMSG_Cerrar(sock);
		return ERR_LIBRT_SI;
	}*/

	LIBMSG_Cerrar(sock);

	LOG_Msg(1,"SALIENDO POR ERR_LIBRT_NO");
	return ERR_LIBRT_NO;

}







/*! \brief F128. Lista los usbs insertados en el sistema sean clauer o no.
 *
 * F128. Lista los usbs insertados en el sistema.
 *
 * \param nDispositivos
 *        SALIDA. El número de dispositivos disponibles.
 *
 * \param dispositivos
 *		  SALIDA. Array con los números de dispositivos disponibles. La función reserva
 *		  espacio para esta variable. Es responsabilidad del programador liberarla
 *		  con free().
 *
 * \retval 0
 *		   OK
 *
 * \retval != 0
 *		   FALLO
 */

int LIBRT_ListarUSBs ( int *nDispositivos, unsigned char **dispositivos)
{
    int len= 0, i= 0, err=0;
    unsigned char aux= 0, funcion=128, acaba=0;
    SOCKET sock;
    
	if ( ! nDispositivos )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;
	if ( ! dispositivos )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;


	/*
	 * Petición de la función
	 */
	
	LOG_Msg(1, "Conectamos ... ");
	
	if ( (err=LIBMSG_Conectar(IP, PORTNUMBER, &sock)) != 0 ){
		LOG_Error(1, "Error al conectar ret= %d ... ", err);
		return ERR_LIBRT_SI;
	}

	LOG_Msg(1, "Enviamos la función 0");	
	if ( LIBMSG_Enviar(sock, (unsigned char *) &funcion, 1) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el número de función ... ");
		return ERR_LIBRT_SI;
	}

	/*
	 * Respuesta del runtime
	 */
	
	if ( (err= LIBMSG_Recibir(sock, (unsigned char *)&aux,1)) != 0 ) {
        //err= GetLastError();
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al recibir el resultado del error ... ");
		return ERR_LIBRT_SI;
	}

	*nDispositivos = aux;

	#ifndef NO_CRYF_SUPPORT
	// Check we have no more than MAX_DEVICES devices 
	if ( aux > MAX_DEVICES ) {
	    LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error demasiados dispositivos ... ");
		return ERR_LIBRT_SI;
	}
	#endif
	
	if ( aux > 0 ) {
	    //dispositivos = (unsigned char *) malloc (*nDispositivos);
	    
		#ifdef NO_CRYF_SUPPORT
		for ( i = 0 ; i < *nDispositivos ; i++ ) {
			if ( LIBMSG_Recibir(sock,(*dispositivos)+i,1) != 0 ) {
				LIBMSG_Cerrar(sock);
				return ERR_LIBRT_SI;
			}
		}
		//printf("Paso NO_CRYF_SUPPORT\n");
		#else
		LOG_Debug(1, "nDispositivos len= %d", *nDispositivos );
		for ( i = 0 ; i < *nDispositivos ; i++ ) {
		    // Receiving size of the next string
		    if ( (err=LIBMSG_Recibir(sock,(unsigned char *)&len ,4)) != 0 ) {
				LIBMSG_Cerrar(sock);
				LOG_Error(1, "Error recibiendo el tamaño de la siguiente cadena error = %d ... ", err);
				return ERR_LIBRT_SI;
		    }
			LOG_Debug(1, "Tamaño de cadena recibido len= %d", len );
		    dispositivos[i]= (unsigned char *) malloc ( len * sizeof(char) + 1 );
		    
		    if ( (err=LIBMSG_Recibir(sock,dispositivos[i],len)) != 0 ) {
				LIBMSG_Cerrar(sock);
				LOG_Error(1," Al recibir la cadena error= %d", err);
				return ERR_LIBRT_SI;
		    }
		    dispositivos[i][len]='\0';
			LOG_Debug(1, "Recibida cadena str= %s", dispositivos[i] );
		}
		//printf("NO Paso NO_CRYF_SUPPORT\n");
                #endif
	}

	/*if ( LIBMSG_Enviar(sock, (unsigned char *) &acaba, 1) != 0 ) {
		LIBMSG_Cerrar(sock);
		return ERR_LIBRT_SI;
	}*/

	if ( LIBMSG_Recibir(sock, (unsigned char *)&err, 1) != 0 ){
		LOG_MsgError(1,"Recibiendo el ERROR");		
		return ERR_LIBRT_SI;
	}
	
	if ( err != 0 ) {
		LOG_Debug(1,"Encontrado error = %d", err);	
		return ERR_LIBRT_NO_ADMIN;
	}

	LIBMSG_Cerrar(sock);

	LOG_Msg(1,"SALIENDO POR ERR_LIBRT_NO");
	return ERR_LIBRT_NO;

}



/*! \brief F1. Inicializa un dispositivo para su posterior utilización.
 *
 * F1. Inicializa un dispositivo para su posterior utilización. La inicialización
 * puede ser sin password o con password. 
 *
 * \param nDevice
 *        ENTRADA. El número de dispositivo a utilizar. Puede obtenerse una lista
 *        de dispositivos disponibles mediante la función LIBRT_ListarDispostivos().
 *
 * \param pwd
 *		  ENTRADA. La password que protege el clauer. Si el parámetro es NULL se inicia
 *        el dispositivo sin password, lo que únicamente permite acceder a su zona pública.
 *
 * \param handle
 *	      SALIDA. Handle al dispositivo. Permite utilizar el resto de funciones.
 *
 * \retval 0
 *		   OK
 *
 * \retval != 0
 *		   FALLO
 */


int _LIBRT_IniciarDispositivo ( unsigned char * device, char * pwd, USBCERTS_HANDLE *handle )
{
    unsigned char funcion = 1, err = 0, tamPassword = 0;
    int len= 0;

	if ( ! device )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;

	if ( ! handle )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;

	/*
	 * Petición de la función
	 */

    	//printf("Paso por la funcion\n");
	handle->pin_cb = NULL;

	if ( !LIBRT_MUTEX_Crear(&(handle->mutex)) ) 
		return ERR_LIBRT_SI;
	

	if ( LIBMSG_Conectar(IP, PORTNUMBER, &(handle->sock)) != 0 ) {
    		// printf("Error no puedo conectar\n");
		LIBRT_MUTEX_Destruir(handle->mutex);
		return ERR_LIBRT_SI;
	}

	if ( LIBMSG_Enviar(handle->sock, (unsigned char *)&funcion, 1) != 0 ) {
    		// printf("Error al enviar\n");
		LIBRT_MUTEX_Destruir(handle->mutex);
		LIBMSG_Cerrar(handle->sock);
		return ERR_LIBRT_SI;
	}
	
	#ifdef NO_CRYF_SUPPORT
	if ( LIBMSG_Enviar(handle->sock, (unsigned char *)&ndevice, 1) != 0 ) {
    		// printf("Error al enviar\n");
		LIBRT_MUTEX_Destruir(handle->mutex);
		LIBMSG_Cerrar(handle->sock);
		return ERR_LIBRT_SI;
	}
	#else 
	len= strlen((char *)device);
	// We send the size of the string containing the path.
	if ( LIBMSG_Enviar(handle->sock, (unsigned char *)&len, 4) != 0 ) {
    		// printf("Error al enviar\n");
	    LIBRT_MUTEX_Destruir(handle->mutex);
	    LIBMSG_Cerrar(handle->sock);
	    return ERR_LIBRT_SI;
	}
	
	// Sending the device path itself.
	if ( LIBMSG_Enviar(handle->sock, (unsigned char *)device, len) != 0 ) {
    		// printf("Error al enviar el path \n");
	    LIBRT_MUTEX_Destruir(handle->mutex);
	    LIBMSG_Cerrar(handle->sock);
	    return ERR_LIBRT_SI;
	}
	device[len]='\0';
        #endif
	
	if ( pwd ) {
		tamPassword = strlen(pwd);
		handle->auth = 1;
	} else {
		tamPassword = 0;
		handle->auth = 0;
	}

	if ( LIBMSG_Enviar(handle->sock, (unsigned char *)&tamPassword, 1) != 0 ) {
    		// printf("Error al enviar el tamanyo de la password\n");
		LIBRT_MUTEX_Destruir(handle->mutex);
		LIBMSG_Cerrar(handle->sock);
		return ERR_LIBRT_SI;
	}


	if ( pwd ) {
	    if ( LIBMSG_Enviar(handle->sock, (unsigned char *)pwd, tamPassword) != 0 ) {
			LIBRT_MUTEX_Destruir(handle->mutex);
			LIBMSG_Cerrar(handle->sock);
			return ERR_LIBRT_SI;
		}
	}
	
	/*
	 * Respuesta del runtime
	 */
	
	// printf("Recibo respuesta del runtime\n");
	if ( LIBMSG_Recibir(handle->sock, (unsigned char *)&err, 1) != 0 ) {
    		// printf("Error al recibir el error\n");
		LIBRT_MUTEX_Destruir(handle->mutex);
		LIBMSG_Cerrar(handle->sock);
		return ERR_LIBRT_SI;
	}

	if ( err != 0 ) {
    		// printf("He recibido un error %d \n Regenerando cache\n",err);
		LIBRT_RegenerarCache();
		LIBRT_MUTEX_Destruir(handle->mutex);
		LIBMSG_Cerrar(handle->sock);
		return ERR_LIBRT_SI;
	}

	if ( LIBMSG_Recibir(handle->sock, handle->idDispositivo, 20) != 0 ) {
		// printf("Error al recibir el id del dispositivo\n");
		LIBRT_MUTEX_Destruir(handle->mutex);
		LIBMSG_Cerrar(handle->sock);
		return ERR_LIBRT_SI;
	}
	
	#ifdef NO_CRYF_SUPPORT
	handle->nDevice = ndevice;
	#else
	strncpy(handle->path,(char *)device,MAX_PATH_LEN);
    #endif 
	
	return ERR_LIBRT_NO;
}




/*! \brief Wrapper F1. Esta función gestiona la inicialización de un dispositivo       
 *
 * F1. Gestiona la inicialización de un dispositivo, regenerando la cache frente un 
 *     error e intentando volver a inicarlo contra el clos.
 *
 * \param nDevice
 *        ENTRADA. El número de dispositivo a utilizar. Puede obtenerse una lista
 *        de dispositivos disponibles mediante la función LIBRT_ListarDispostivos().
 *
 * \param pwd
 *		  ENTRADA. La password que protege el clauer. Si el parámetro es NULL se inicia
 *        el dispositivo sin password, lo que únicamente permite acceder a su zona pública.
 *
 * \param handle
 *	      SALIDA. Handle al dispositivo. Permite utilizar el resto de funciones.
 *
 * \retval 0
 *		   OK
 *
 * \retval != 0
 *		   FALLO
 */

int LIBRT_IniciarDispositivo ( unsigned char * device, char * pwd, USBCERTS_HANDLE *handle )
{

    if  ( _LIBRT_IniciarDispositivo ( device, pwd, handle ) == ERR_LIBRT_SI ) {
	
	LIBRT_RegenerarCache();
    
	if  ( _LIBRT_IniciarDispositivo ( device, pwd, handle ) == ERR_LIBRT_SI )
	    
	    return ERR_LIBRT_SI;
    }
    
    return ERR_LIBRT_NO;
    
}





/*! \brief F2. Lee toda la zona reservada del dispositivo.
 *
 * F2. Lee toda la zona reservada del dispositivo. Esta función debe llamarse en
 * dos pasos. En el primero se pasa buffer como NULL, de esa forma obtenemos
 * en bytesBuffer.
 *
 * \param handle
 *        ENTRADA. El manejador de dispositivo obtenido mediante la función
 *				   LIBRT_IniciarDispositivo().
 *
 * \param buffer
 *		  SALIDA. El buffer
 *
 * \param handle
 *	      SALIDA. Handle al dispositivo. Permite utilizar el resto de funciones.
 *
 * \retval 0
 *		   OK
 *
 * \retval != 0
 *		   FALLO
 */

int LIBRT_LeerZonaReservada (USBCERTS_HANDLE *handle, unsigned char *buffer, unsigned long *bytesBuffer)
{
	unsigned char err = 0,func = 2;

	/*
	 * Parámetro de salida, setup para 64/32  bits
 	 */
	#ifdef IAx86_64
		int aux; 
	#else 
		long aux;
 	#endif


	if ( ! handle )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;

	if ( ! bytesBuffer )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;

	/*
	 * Sólo adquiero el mutex la primera vez que llamo
	 * a la función. La segunda ya está adquirido
	 */
	

	if ( ! buffer ) {
		if ( ! LIBRT_MUTEX_Lock(handle->mutex) ) 
			return ERR_LIBRT_SI;

		/*
		 * Envío la petición. La atención de la reconexión se hace sólo en la
		 * primera función.
		 */

		switch ( LIBMSG_Enviar(handle->sock, (unsigned char *)&func, 1) )
		{
			case ERR_LIBMSG_NO:
				break;

			case ERR_LIBMSG_TIMEOUT:
				if ( LIBRT_Reconectar(handle) != 0 ) {
					LIBRT_MUTEX_Unlock(handle->mutex);
					return ERR_LIBRT_SI;
				}
				break;

			default:
				LIBRT_MUTEX_Unlock(handle->mutex);
				return ERR_LIBRT_SI;
		}

		/*
		 * Recibo la respuesta
		 */

		if ( LIBMSG_Recibir(handle->sock, (unsigned char *) &err, 1) != 0 )
			return ERR_LIBRT_SI;

		if ( err != 0 ) 
			return ERR_LIBRT_SI;


		*bytesBuffer = 0;

		if (LIBMSG_Recibir(handle->sock, (unsigned char *)&aux, 4) != 0) {
			LIBRT_MUTEX_Unlock(handle->mutex);
			return ERR_LIBRT_SI;
		}
		
        	*bytesBuffer= aux;
	} else {

		/*
           	 * Esta es la segunda llamada
		 */

		if ( LIBMSG_Recibir(handle->sock, buffer, *bytesBuffer) != 0 ) {
			LIBRT_MUTEX_Unlock(handle->mutex);
			return ERR_LIBRT_SI;
		}

	}


	LIBRT_MUTEX_Unlock(handle->mutex);

	return ERR_LIBRT_NO;
}





/*! \brief F3. Escribe la zona reservada.
 *
 * F3. Escribe la zona reservada. Escribe buffer en la zona reservada.
 *
 * \param handle
 *        ENTRADA. El manejador de dispositivo obtenido mediante la función
 *				   LIBRT_IniciarDispositivo().
 *
 * \param buffer
 *		  ENTRADA. El buffer que se escribirá en la zona reservada.
 *
 * \param bytesBuffer
 *	      ENTADA. Los datos que se escribirán de buffer en la zona reservada.
 *
 * \retval 0
 *		   OK
 *
 * \retval != 0
 *		   FALLO
 */

int LIBRT_EscribirZonaReservada (USBCERTS_HANDLE *handle, unsigned char *buffer, unsigned long bytesBuffer)
{

	unsigned char err=0,func = 3;
	

	if ( ! handle )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;
	if ( ! buffer )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;

	if ( !LIBRT_MUTEX_Lock(handle->mutex) ) {
		return ERR_LIBRT_SI;
	}

	/*
	 * Envío la petición
	 */

	switch (LIBMSG_Enviar(handle->sock, (unsigned char *)&func, 1) ) {
		case ERR_LIBMSG_NO:
			break;

		case ERR_LIBMSG_TIMEOUT:
			if ( LIBRT_Reconectar(handle) != 0 ) {
				LIBRT_MUTEX_Unlock(handle->mutex);
				return ERR_LIBRT_SI;
			}
			break;

		default:
			LIBRT_MUTEX_Unlock(handle->mutex);
			return ERR_LIBRT_SI;
	}


	if (LIBMSG_Enviar(handle->sock, buffer, bytesBuffer) != 0 ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}	

	/*
	 * Recibo la respuesta
	 */

	if ( LIBMSG_Recibir(handle->sock, (unsigned char *)&err, 1) != 0 ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}

	if ( err != 0 ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}

	LIBRT_MUTEX_Unlock(handle->mutex);
	
	LIBRT_RegenerarCache();
	
	return ERR_LIBRT_NO;

}



/*! \brief F4. Cambia la password del dispositivo.
 *
 * F4. Cambia la password del dispositivo.
 *
 * \param handle
 *        ENTRADA. El manejador de dispositivo obtenido mediante la función
 *				   LIBRT_IniciarDispositivo().
 *
 * \param nuevaPassword
 *		  ENTRADA. La nueva password del dispositivo.
 *
 * \retval 0
 *		   OK
 *
 * \retval != 0
 *		   FALLO
 */

int LIBRT_CambiarPassword ( USBCERTS_HANDLE *handle, char * nuevaPassword)
{

	unsigned char err=0, func = 4, tamPassword = 0;


	if ( ! handle )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;

	if ( ! nuevaPassword )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;


	if ( !LIBRT_MUTEX_Lock(handle->mutex) ) 
		return ERR_LIBRT_SI;

	/*
	 * Envío la petición
	 */

	switch (LIBMSG_Enviar(handle->sock, (unsigned char *)&func, 1))
	{
		case ERR_LIBMSG_NO:
			break;

		case ERR_LIBMSG_TIMEOUT:
			if ( LIBRT_Reconectar(handle) != 0 ) {
				LIBRT_MUTEX_Unlock(handle->mutex);
				return ERR_LIBRT_SI;
			}
			break;

		default:
			LIBRT_MUTEX_Unlock(handle->mutex);
			return ERR_LIBRT_SI;
	}

	tamPassword = strlen(nuevaPassword);

	if ( LIBMSG_Enviar(handle->sock, (unsigned char *)&tamPassword, 1 ) != 0 ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}


	if (LIBMSG_Enviar(handle->sock, (unsigned char * )nuevaPassword, tamPassword) != 0 ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}


	/*
	 * Recibo la respuesta
	 */


	if ( LIBMSG_Recibir(handle->sock, (unsigned char *) &err, 1) != 0 ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}

	if ( err != 0 ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}

	LIBRT_MUTEX_Unlock(handle->mutex);
	LIBRT_RegenerarCache();
	
	return ERR_LIBRT_NO;

}




/*! \brief F5. Leer bloque identificativo.
 *
 * F5. Leer bloque identificativo.
 *
 * \param handle
 *        ENTRADA. El manejador de dispositivo obtenido mediante la función
 *				   LIBRT_IniciarDispositivo().
 *
 * \param bloque
 *		  SALIDA. El bloque identificativo.
 *
 * \retval 0
 *		   OK
 *
 * \retval != 0
 *		   FALLO
 */

int LIBRT_LeerBloqueIdentificativo (USBCERTS_HANDLE *handle, unsigned char *bloque)
{
	unsigned char err=0,func = 5;

	if ( ! handle )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;

	if ( ! bloque )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;


	if ( !LIBRT_MUTEX_Lock(handle->mutex) ) {
		return ERR_LIBRT_SI;
	}

	/*
	 * Envío la petición
	 */

	switch (LIBMSG_Enviar(handle->sock, (unsigned char *)&func, 1) ) 
	{
		case ERR_LIBMSG_NO:
			break;

		case ERR_LIBMSG_TIMEOUT:
			if ( LIBRT_Reconectar(handle) != 0 ) {
				LIBRT_MUTEX_Unlock(handle->mutex);
				return ERR_LIBRT_SI;
			}
			break;

		default:
			LIBRT_MUTEX_Unlock(handle->mutex);
			return ERR_LIBRT_SI;
	}

	/*
	 * Recibo la respuesta
	 */

	if ( LIBMSG_Recibir(handle->sock, (unsigned char *)&err, 1) != 0 ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}

	if ( err != 0 ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}

	if (LIBMSG_Recibir(handle->sock, (unsigned char *)bloque, TAM_BLOQUE) != 0) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}

	LIBRT_MUTEX_Unlock(handle->mutex);

	return ERR_LIBRT_NO;

}




/*! \brief F6. Lee un bloque de la zona de objetos de la partición criptográfica.
 *
 * F6. Lee un bloque de la zona de objetos de la partición criptográfica.
 *
 * \param handle
 *        ENTRADA. El manejador de dispositivo obtenido mediante la función
 *				   LIBRT_IniciarDispositivo().
 *
 * \param numBloque
 *		  ENTRADA. El número de bloque que se quiere leer.
 *
 * \retval 0
 *		   OK
 *
 * \retval != 0
 *		   FALLO
 */

int LIBRT_LeerBloqueCrypto (USBCERTS_HANDLE *handle, int numBloque, unsigned char *bloque)
{

	unsigned char err = 0,func = 6;

	LIBRT_RegenerarCache();

	if ( ! handle )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;

	if ( ! bloque )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;

	if ( !LIBRT_MUTEX_Lock(handle->mutex) ) 
		return ERR_LIBRT_SI;

	/*
	 * Envío la petición
	 */

	switch (LIBMSG_Enviar(handle->sock, (unsigned char *)&func, 1) ) 
	{
		case ERR_LIBMSG_NO:
			break;

		case ERR_LIBMSG_TIMEOUT:
			if ( LIBRT_Reconectar(handle) != 0 ) {
				LIBRT_MUTEX_Unlock(handle->mutex);
				return ERR_LIBRT_SI;
			}
			break;

		default:
			LIBRT_MUTEX_Unlock(handle->mutex);
			return ERR_LIBRT_SI;
	}

	if ( LIBMSG_Enviar(handle->sock, (unsigned char *)&numBloque, 4) != 0 ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}

	/*
	 * Recibo la respuesta
	 */

	if ( LIBMSG_Recibir(handle->sock, (unsigned char *)&err, 1) != 0 ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}

	if ( err != 0 ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}

	if (LIBMSG_Recibir(handle->sock, bloque, TAM_BLOQUE) != 0)  {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}


	LIBRT_MUTEX_Unlock(handle->mutex);


	return ERR_LIBRT_NO;

}




/*! \brief F7/F8. Lee un bloque del tipo especificado.
 *
 * F7/F8. Lee un bloque del tipo especificado. Para leer el primer bloque de ese
 * tipo se pone primero a 1. Subsiguientes llamadas deben pasar primero a 0.
 *
 * \param handle
 *        ENTRADA. El manejador de dispositivo obtenido mediante la función
 *				   LIBRT_IniciarDispositivo().
 *
 * \param tipo
 *		  ENTRADA. El tipo de bloque que queremos leer.
 *
 * \param primero
 *		  ENTRADA. Indica si se desea leer el primer bloque del tipo indicado.
 *
 * \param bloque
 *		  SALIDA. Devuelve el bloque que hemos pedido.
 *
 * \param numBloque
 *		  SALIDA. Indica el número de bloque donde estaba almacenado.
 *
 * \retval 0
 *		   OK
 *
 * \retval != 0
 *		   FALLO
 *
 */

int LIBRT_LeerTipoBloqueCrypto (USBCERTS_HANDLE *handle, TIPO_BLOQUE tipo, int primero, 
		unsigned char *bloque, long *numBloque)
{

	unsigned char err = 0, func = 0;

	
	#ifdef IAx86_64
		int aux= (int) *numBloque;
	#else
		long aux;
		aux = *numBloque;
	#endif

	LIBRT_RegenerarCache();

	if ( ! handle )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;

	if ( ! numBloque )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;

	if ( ! bloque )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;

	func = (primero) ? 7 : 8;

	switch ( LIBMSG_Enviar(handle->sock, (unsigned char *)&func, 1) ) 
	{
		case ERR_LIBMSG_NO:
			break;

		case ERR_LIBMSG_TIMEOUT:
			if ( LIBRT_Reconectar(handle) != 0 ) {
				LIBRT_MUTEX_Unlock(handle->mutex);
				return ERR_LIBRT_SI;
			}
			break;

		default:
			return ERR_LIBRT_SI;
	}

	if ( LIBMSG_Enviar(handle->sock, &tipo, 1) != 0 ) 
		return ERR_LIBRT_SI;


	if ( func == 8 )  
		if ( LIBMSG_Enviar(handle->sock, (unsigned char *)&aux, 4) != 0 ) 
			return ERR_LIBRT_SI;
		
	if ( LIBMSG_Recibir(handle->sock, (unsigned char *)&err, 1) != 0 ){ 
		return ERR_LIBRT_SI;
	}

	if ( err == 1 ){
		return ERR_LIBRT_SI;
	}
	aux = 0;

	if (LIBMSG_Recibir(handle->sock, (unsigned char *)&aux, 4) != 0) 
		return ERR_LIBRT_SI;

	if ( aux != -1 )
		if (LIBMSG_Recibir(handle->sock, bloque, TAM_BLOQUE) != 0) 
			return ERR_LIBRT_SI;

	*numBloque= (long) aux;

	return ERR_LIBRT_NO;
}




/* \brief F9. Lee todos los bloques crypto de un tipo determinado.
 *
 * F9. Lee todos los bloques crypto de un tipo determinado. Para usar la función:
 *
 *		. La primera llamada pasamos handleBloques o bloques a NULL. De esta
 *		  forma se devuelve en numBloques el número de bloques a reservar en
 *		  ambos buffers.
 *
 *		. En la segunda llamada se pasa ya bloques y handleBloques apuntando
 *		  a una zona reservada de tamaño suficiente. numBloques debe pasarse
 *		  con el valor obtenido en la llamada anterior.
 *
 * \param handle
 *		  ENTRADA. El handle del dispositivo.
 * 
 * \param tipo
 *		  ENTRADA. El tipo del bloque a leer.
 *
 * \param handleBloques
 *		  SALIDA. Vector de tamaño *numBloques que contiene los números de bloque
 *				  que se leyeron.
 *
 * \param bloques
 *		  SALIDA. Vector de tamaño TAM_BLOQUE*(*numBloques)
 */

int LIBRT_LeerTodosBloquesTipo (USBCERTS_HANDLE *handle, 
		TIPO_BLOQUE tipo,
		long *handleBloques,
		unsigned char *bloques,
		long *numBloques)
{

	unsigned char err = 0, func = 9, i=0;

	

    #ifdef IAx86_64
            int aux= (int) *numBloques;
    #else
            long aux;
			aux= *numBloques;
    #endif

    LIBRT_RegenerarCache();

	if ( ! handle )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;
	if ( ! numBloques )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;

	/*
	 * El mutex sólo se adquiere en la primera llamada a la 
	 * función
	 */


	if ( (handleBloques == NULL) || (bloques == NULL) )  {

		if ( !LIBRT_MUTEX_Lock(handle->mutex) ) {
			return ERR_LIBRT_SI;
		}

		switch ( LIBMSG_Enviar(handle->sock, (unsigned char *)&func, 1) ) 
		{
			case ERR_LIBMSG_NO:
				break;

			case ERR_LIBMSG_TIMEOUT:
				if ( LIBRT_Reconectar(handle) != 0 ) {
					LIBRT_MUTEX_Unlock(handle->mutex);
					return ERR_LIBRT_SI;
				}
				break;

			default:
				LIBRT_MUTEX_Unlock(handle->mutex);
				return ERR_LIBRT_SI;

		}


		if ( LIBMSG_Enviar(handle->sock, &tipo, 1) != 0 )  {
			LIBRT_MUTEX_Unlock(handle->mutex);
			return ERR_LIBRT_SI;
		}

		/*
		 * Respuesta
		 */


		if ( LIBMSG_Recibir(handle->sock, (unsigned char *)&err, 1) != 0 ) {
			LIBRT_MUTEX_Unlock(handle->mutex);
			return ERR_LIBRT_SI;
		}


		if ( err != 0 ) {
			LIBRT_MUTEX_Unlock(handle->mutex);
			return ERR_LIBRT_SI;
		}

		if ( LIBMSG_Recibir(handle->sock, (unsigned char *)&aux, 4) != 0 ) {
			LIBRT_MUTEX_Unlock(handle->mutex);
			return ERR_LIBRT_SI;
		}


		/*
		 * Si el número de bloques es cero, entonces
		 * liberamos el lock
		 */

		if ( *numBloques == 0 )
			LIBRT_MUTEX_Unlock(handle->mutex);

		*numBloques= (long) aux;

	} else {

		/*
		 * En la segunda llamada a la función recibimos los datos
		 */
	
		/* Compatibilidad 64 bits */
		int * vtmp= (int *) malloc((*numBloques) * sizeof(int));
		if ( !vtmp ){
			LIBRT_MUTEX_Unlock(handle->mutex);
                        return ERR_LIBRT_SI;
		}

                switch ( LIBMSG_Recibir(handle->sock, (unsigned char *)vtmp, 4 * (*numBloques)) )
	        {	
			case ERR_LIBMSG_NO:
				break;

			case ERR_LIBMSG_TIMEOUT:
				if ( LIBRT_Reconectar(handle) != 0 ) {
					LIBRT_MUTEX_Unlock(handle->mutex);
					return ERR_LIBRT_SI;
				}
				break;

			default:
				LIBRT_MUTEX_Unlock(handle->mutex);
				return ERR_LIBRT_SI;

		}

	        for ( i=0 ; i<*numBloques ; i++ ){
			handleBloques[i]= (long) vtmp[i];
                }
		
 		free(vtmp);

		if ( LIBMSG_Recibir(handle->sock, bloques, TAM_BLOQUE * (*numBloques)) != 0 ) {
			LIBRT_MUTEX_Unlock(handle->mutex);
			return ERR_LIBRT_SI;
		}

		LIBRT_MUTEX_Unlock(handle->mutex);

	}

	return ERR_LIBRT_NO;

}


/*! \brief F11. Escribe un bloque en la posición (numBloque) especificada.
 *
 * F11. Escribe un bloque en la posición (numBloque) especificada.
 *
 * \param handle
 *        ENTRADA. El manejador de dispositivo obtenido mediante la función
 *				   LIBRT_IniciarDispositivo().
 *
 * \param numBloque
 *		  ENTRADA. Número de bloque de la zona de objetos donde queremos insertar
 *		           el objeto.
 *
 * \param bloque
 *		  SALIDA. El bloque que hemos pedido.
 *
 * \retval 0
 *		   OK
 *
 * \retval != 0
 *		   FALLO
 *
 */

int LIBRT_EscribirBloqueCrypto (USBCERTS_HANDLE *handle, unsigned long numBloque, unsigned char *bloque)
{

    unsigned char err = 0,func = 11;
    

    #ifdef IAx86_64
            int aux= (int) numBloque;
    #else
            long aux= numBloque;
    #endif


	if ( ! handle )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;

	if ( ! bloque )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;

	if ( !LIBRT_MUTEX_Lock(handle->mutex) ) 
		return ERR_LIBRT_SI;

	/*
	 * Envío la petición
	 */

	switch ( LIBMSG_Enviar(handle->sock, (unsigned char *) &func, 1) ) 
	{
		case ERR_LIBMSG_NO:
			break;

		case ERR_LIBMSG_TIMEOUT:
			if ( LIBRT_Reconectar(handle) != 0 ) {
				LIBRT_MUTEX_Unlock(handle->mutex);
				return ERR_LIBRT_SI;
			}
			break;

		default:
			LIBRT_MUTEX_Unlock(handle->mutex);
			return ERR_LIBRT_SI;
	}


	if ( LIBMSG_Enviar(handle->sock, (unsigned char *) &aux, 4) != 0 ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}

	if ( LIBMSG_Enviar(handle->sock, (unsigned char *) bloque, TAM_BLOQUE) != 0 ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}

	/*
	 * Recibo la respuesta
	 */

	if ( LIBMSG_Recibir(handle->sock, (unsigned char *) &err, 1) != 0 ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}

	if ( err != 0 )  {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}

	LIBRT_MUTEX_Unlock(handle->mutex);
	LIBRT_RegenerarCache();
	
	return ERR_LIBRT_NO;

}



/*! \brief F12. Inserta un bloque en cualquier posición libre del clauer.
 *
 * F12. Inserta un bloque en cualquier posición libre del clauer.
 *
 * \param handle
 *        ENTRADA. El manejador de dispositivo obtenido mediante la función
 *				   LIBRT_IniciarDispositivo().
 *
 * \param bloque
 *		  ENTRADA. El bloque a insertar.
 *
 * \param numBloque
 *		  SALIDA. El número de bloque donde vamos a insertar numBloque.
 *
 * \retval 0
 *		   OK
 *
 * \retval != 0
 *		   FALLO
 *
 */

int LIBRT_InsertarBloqueCrypto (USBCERTS_HANDLE *handle, unsigned char *bloque, long *numBloque)
{
    unsigned char err=0,func = 12;

	char read_bloque[TAM_BLOQUE];
	
    #ifdef IAx86_64
           int aux;
    #else
           long aux;
    #endif



	if ( ! handle )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;
	if ( ! bloque )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;
	if ( ! numBloque )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;

	if ( !LIBRT_MUTEX_Lock(handle->mutex) ) {
		return ERR_LIBRT_SI;
	}

	/*
	 * Envío la petición
	 */

	switch ( err=LIBMSG_Enviar(handle->sock, (unsigned char *)&func, 1) ) 
	{
		case ERR_LIBMSG_NO:
			break;

		case ERR_LIBMSG_TIMEOUT:
			if ( LIBRT_Reconectar(handle) != 0 ) {
				LIBRT_MUTEX_Unlock(handle->mutex);
				return ERR_LIBRT_SI;
			}
			break;

		default:
			LIBRT_MUTEX_Unlock(handle->mutex);
			return ERR_LIBRT_SI;
	}

	if ( LIBMSG_Enviar(handle->sock, bloque, TAM_BLOQUE) != 0 ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}

	/*
	 * Recibo la respuesta
	 */

	if ( LIBMSG_Recibir(handle->sock, (unsigned char *)&err, 1) != 0 ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}

	if ( err != 0 ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}

	if ( LIBMSG_Recibir(handle->sock, (unsigned char *)&aux, 4) != 0 ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}
	
	*numBloque= (long) aux;

	/* Comprobamos que la insercion se ha efectuado correctamente. Hemos tenido problemas 
	// con algunos clauers "mentirosos" que no ofrecen input/output error ante una insercion
	// fallida y al leer devuelven random. 
	
	err= LIBRT_LeerBloqueCrypto( handle, numBloque, read_bloque);
	if ( err != 0 )
	{
		return err;
	}
	//Comprobamos el bloque: 
	if (memcmp(bloque, read_bloque, TAM_BLOQUE)!=0){
		return ERR_LIBRT_SI;
	}*/
	//EOCheck

	LIBRT_MUTEX_Unlock(handle->mutex);
	LIBRT_RegenerarCache();
	
	return ERR_LIBRT_NO;

}



/*! \brief F13. Borra el bloque especificado en numBloque.
 *
 * F13. Borra el bloque especificado en numBloque.
 *
 * \param handle
 *        ENTRADA. El manejador de dispositivo obtenido mediante la función
 *				   LIBRT_IniciarDispositivo().
 *
 * \param numBloque
 *		  ENTRADA. La posición del bloque que queremos borrar.
 *
 * \retval 0
 *		   OK
 *
 * \retval != 0
 *		   FALLO
 *
 */

int LIBRT_BorrarBloqueCrypto (USBCERTS_HANDLE *handle, unsigned long numBloque)
{

	unsigned char err=0,func = 13;

	
	#ifdef IAx86_64
                int aux= (int) numBloque;
        #else
                long aux= numBloque;
        #endif


	if ( ! handle )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;

	if ( !LIBRT_MUTEX_Lock(handle->mutex) ) {
		return ERR_LIBRT_SI;
	}

	/*
	 * Envío la petición
	 */

	switch (LIBMSG_Enviar(handle->sock, (unsigned char *)&func, 1) ) 
	{
		case ERR_LIBMSG_NO:
			break;

		case ERR_LIBMSG_TIMEOUT:
			if ( LIBRT_Reconectar(handle) != 0 ) {
				LIBRT_MUTEX_Unlock(handle->mutex);
				return ERR_LIBRT_SI;
			}
			break;

		default:
			LIBRT_MUTEX_Unlock(handle->mutex);
			return ERR_LIBRT_SI;

	}
	


	if ( LIBMSG_Enviar(handle->sock, (unsigned char *)&aux, 4) != 0 ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}

	/*
	 * Recibo la respuesta
	 */

	if ( LIBMSG_Recibir(handle->sock, (unsigned char *)&err, 1) != 0 ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}

	if ( err != 0 ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	}

	LIBRT_MUTEX_Unlock(handle->mutex);
	LIBRT_RegenerarCache();
	
	return ERR_LIBRT_NO;

}




/*! \brief F10. Lee todos los bloques ocupados.
 *
 * F10. Lee todos los bloques ocupados.
 *
 * \param handle
 *        ENTRADA. El manejador de dispositivo obtenido mediante la función
 *				   LIBRT_IniciarDispositivo().
 *
 * \param handleBloques
 *		  SALIDA. Vector con las posiciones de cada bloque devuelto. Tamaño: *numBloques * 4
 *
 * \param bloques
 *		  SALIDA. Bloques devueltos. Tamaño: TAM_BLOQUE * *numBloques
 *
 * \param numBloques
 *		  SALIDA. El número de bloques a devolver.
 *
 * \retval 0
 *		   OK
 *
 * \retval != 0
 *		   FALLO
 *
 */

int LIBRT_LeerTodosBloquesOcupados ( USBCERTS_HANDLE *handle, long *handleBloques, unsigned char *bloques, unsigned long *numBloques)
{

	unsigned char err=0,func = 10;
	int *vtmp;
	unsigned long i;

	
        
        #ifdef IAx86_64
	int aux= (int) *numBloques;
        #else
	long aux= *numBloques;
        #endif
  
	LIBRT_RegenerarCache();
	
	if ( ! handle )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;
	if ( ! numBloques )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;

	if ( bloques == NULL ) {

		if ( !LIBRT_MUTEX_Lock(handle->mutex) ) {
			return ERR_LIBRT_SI;
		}

		/*
		 * Envío la petición
		 */

		switch (LIBMSG_Enviar(handle->sock, (unsigned char *)&func, 1) ) 
		{
			case ERR_LIBMSG_NO:
				break;

			case ERR_LIBMSG_TIMEOUT:
				if ( LIBRT_Reconectar(handle) != 0 ) {
					LIBRT_MUTEX_Unlock(handle->mutex);
					return ERR_LIBRT_SI;
				}
				break;

			default:
				LIBRT_MUTEX_Unlock(handle->mutex);
				return ERR_LIBRT_SI;
		}

		/*
		 * Recibo la respuesta
		 */

		if ( LIBMSG_Recibir(handle->sock, (unsigned char *)&err, 1) != 0 ) {
			LIBRT_MUTEX_Unlock(handle->mutex);
			return ERR_LIBRT_SI;
		}

		if ( err != 0 ) {
			LIBRT_MUTEX_Unlock(handle->mutex);
			return ERR_LIBRT_SI;
		}

		/* Recibo el número de bloques que hay
		 */
		

		if ( LIBMSG_Recibir(handle->sock, (unsigned char *)&aux, 4) != 0 ) {
			LIBRT_MUTEX_Unlock(handle->mutex);
			return ERR_LIBRT_SI;
		}
		*numBloques= (long) aux;

	} else {

		if ( ! handleBloques )
			return ERR_LIBRT_PARAMETRO_INCORRECTO;
		if ( ! bloques )
			return ERR_LIBRT_PARAMETRO_INCORRECTO;
	
		/* Compatibilidad con 64 bits */	
		vtmp=(int *) malloc((*numBloques) * sizeof(int));
		
		for ( i = 0 ; i < *numBloques ; i++ ) {
			switch ( LIBMSG_Recibir(handle->sock, (unsigned char *)	(vtmp+i), 4) ) 
			{
				case ERR_LIBMSG_NO:
					break;

				case ERR_LIBMSG_TIMEOUT:
					if ( LIBRT_Reconectar(handle) != 0 ) {
						LIBRT_MUTEX_Unlock(handle->mutex);
						return ERR_LIBRT_SI;
					}
					break;

				default:
					LIBRT_MUTEX_Unlock(handle->mutex);
					return ERR_LIBRT_SI;
			}
		}
		
		for ( i = 0 ; i < *numBloques ; i++ ){
			handleBloques[i]= (long) vtmp[i];
		}
		free(vtmp);

		for ( i = 0 ; i < *numBloques ; i++ ) {
			if ( LIBMSG_Recibir(handle->sock, (unsigned char *)	(bloques+i*TAM_BLOQUE), TAM_BLOQUE) != 0 ) {
				LIBRT_MUTEX_Unlock(handle->mutex);
				return ERR_LIBRT_SI;
			}
		}

		LIBRT_MUTEX_Unlock(handle->mutex);
	}



	return ERR_LIBRT_NO;

}


/*! \brief Obtiene el identificador hardware almacenado en el dispositivo clauer, calcula el identificador 
 *         real y lo devuelve para que puedan ser comparados. 
 *
 * Obtiene el identificador hardware almacenado en el dispositivo clauer, calcula el identificador 
 * real y lo devuelve para que puedan ser comparados.
 * \param handle
 *		  El handle al runtime.
 *
 * \param hwIdDispositivo
 *		  El identificador HW almacenado en el dispositivo.
 *
 * \param hwIdSistema
 *		  El identificador HW recalculado en el sistema.
 * 
 * \retval 0
 *	       Correcto
 *
 * \retval !=0
 *         Error
 *
 * \remarks Primero hay que iniciar el dispositivo con LIBRT_IniciarDispositivo()
 *
 */


int LIBRT_ObtenerHardwareId ( USBCERTS_HANDLE *handle, unsigned char hwIdDispositivo[HW_ID_LEN], unsigned char hwIdSistema[HW_ID_LEN] ){
    
    unsigned char err= 0, func= 14;

    if ( ! handle )
	return ERR_LIBRT_PARAMETRO_INCORRECTO;
    
    if ( !LIBRT_MUTEX_Lock(handle->mutex) ) {
	return ERR_LIBRT_SI;
    }
    
    /*
     * Envío la petición
     */

    switch (LIBMSG_Enviar(handle->sock, (unsigned char *)&func, 1) ) 
	{
	case ERR_LIBMSG_NO:
	    break;

	case ERR_LIBMSG_TIMEOUT:
	    if ( LIBRT_Reconectar(handle) != 0 ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		return ERR_LIBRT_SI;
	    }
	    break;

	default:
	    LIBRT_MUTEX_Unlock(handle->mutex);
	    return ERR_LIBRT_SI;
	}
    
    if ( LIBMSG_Recibir(handle->sock, (unsigned char *)&err, 1) != 0 ) {
	LIBRT_MUTEX_Unlock(handle->mutex);
	return ERR_LIBRT_SI;
    }
    
    if ( err != 0 ) {
	LIBRT_MUTEX_Unlock(handle->mutex);
	return ERR_LIBRT_SI;
    }
    
    
    if ( LIBMSG_Recibir(handle->sock, (unsigned char *) hwIdDispositivo, HW_ID_LEN) != 0 ) {
	LIBRT_MUTEX_Unlock(handle->mutex);
	return ERR_LIBRT_SI;
    }

    if ( LIBMSG_Recibir(handle->sock, (unsigned char *) hwIdSistema, HW_ID_LEN) != 0 ) {
	LIBRT_MUTEX_Unlock(handle->mutex);
	return ERR_LIBRT_SI;
    }

    LIBRT_MUTEX_Unlock(handle->mutex);
    
    return ERR_LIBRT_NO;
}





/*! \brief  Hace que el clos regenere su cache interna de informacion sobre los dispositivos. 
 *		   
 *
 *  Hace que el clos regenere su cache interna de informacion sobre los dispositivos.
 *
 * \retval 0
 *	       Correcto
 *
 * \retval !=0
 *         Error
 *
 * \remarks  Esta funcion es independiente, NO es necesario iniciar o listar dispositivos antes. 
 *
 */


int LIBRT_RegenerarCache (  ){
    
    unsigned char err= 0, func=15;
    SOCKET sock;
    
    /*
     * Petición de la función
     */
	
    if ( LIBMSG_Conectar(IP, PORTNUMBER, &sock) != 0 )
	return ERR_LIBRT_SI;


    if ( LIBMSG_Enviar(sock, (unsigned char *) &func, 1) != 0 ) {
	LIBMSG_Cerrar(sock);
	return ERR_LIBRT_SI;
    }

    /*
     * Respuesta del runtime
     */
	
    if ( LIBMSG_Recibir(sock, (unsigned char *)&err,1) != 0 ) {
	LIBMSG_Cerrar(sock);
	return ERR_LIBRT_SI;
    }
 
    if ( err != 0 ) {
	return ERR_LIBRT_SI;
    }

    LIBMSG_Cerrar(sock);
    
    return ERR_LIBRT_NO; 
}


/*! \brief Obtiene el identificador hardware almacenado en el dispositivo clauer, calcula el identificador
 *         real y lo devuelve para que puedan ser comparados.
 *
 * Obtiene el identificador hardware almacenado en el dispositivo clauer, calcula el identificador
 * real y lo devuelve para que puedan ser comparados.
 * \param handle
 *                El handle al runtime.
 *
 * \param version
 *                Parametro de salida de la version.
 *
 * \retval 0
 *             Correcto
 *
 * \retval !=0
 *         Error
 *
 * \remarks Primero hay que iniciar el dispositivo con LIBRT_IniciarDispositivo()
 *
 */


int LIBRT_ObtenerVersion (char ** version ){

    unsigned char err= 0, func= 16;
    int len;
    SOCKET sock; 


   /*
   ** Petición de la función
   **/

    if ( LIBMSG_Conectar(IP, PORTNUMBER, &sock) != 0 )
        return ERR_LIBRT_SI;


    if ( LIBMSG_Enviar(sock, (unsigned char *) &func, 1) != 0 ) {
        LIBMSG_Cerrar(sock);
        return ERR_LIBRT_SI;
    }

   /*
    ** Respuesta del runtime
    **/
    if ( LIBMSG_Recibir(sock, (unsigned char *)&len,4) != 0 ) {
        LIBMSG_Cerrar(sock);
        return ERR_LIBRT_SI;
    }

    *version= malloc(sizeof(char) * (len + 1));
    if (!version){
	return ERR_LIBRT_SI;
    }
 
    if ( LIBMSG_Recibir(sock, (unsigned char *)*version, len) != 0 ) {
        LIBMSG_Cerrar(sock);
        return ERR_LIBRT_SI;
    }
       
    (*version)[len]=0;
 
    if ( err != 0 ) {
        return ERR_LIBRT_SI;
    }

    LIBMSG_Cerrar(sock);

    return ERR_LIBRT_NO;
}


/*! \brief Establece el callback para la petición del pin en las reconexiones debidas al vencimiento
 *		   del timeout.
 *
 * Establece el callback para la petición del pin en las reconexiones debidas al vencimiento
 * del timeout.
 *
 * \param handle
 *		  El handle al runtime.
 *
 * \param pin_cb
 *		  Puntero a la funci´no que utilizaremos.
 *
 * \retval 0
 *	       Correcto
 *
 * \retval !=0
 *         Error
 *
 * \remarks Primero hay que iniciar el dispositivo con LIBRT_IniciarDispositivo()
 *
 */

int LIBRT_Set_PIN_Callback(USBCERTS_HANDLE *handle, LIBRT_PIN_cb pin_cb)
{
	if ( ! handle )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;
	if ( ! pin_cb )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;

	if ( !LIBRT_MUTEX_Lock(handle->mutex) ) 
		return ERR_LIBRT_SI;

	handle->pin_cb = pin_cb;

	LIBRT_MUTEX_Unlock(handle->mutex);

	return 0;
}



/* \brief Añade datos de aplicación al handle.
 *
 * Añade datos de aplicación al handle. Estos datos son los que se le pasarán a la función
 * callback de reconexión.
 *
 * \param handle
 *		  El handle al runtime.
 *
 * \param appData
 *		  Los datos de aplicación que queremos añadir al handle.
 *
 * \retval 0
 *		   Ok
 *
 * \remarks NO SE REALIZA COPIA DE appData
 */

int LIBRT_Set_AppData (USBCERTS_HANDLE *handle, void *appData)
{
	if ( ! handle )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;

	handle->appData = appData;

	return 0;
}



/*! \brief Indica si hay o no Runtime
 *
 * Indica si hay Runtime o no.
 *
 * \retval 1
 *		   Hay Runtime
 *
 * \retval 0
 *		   No hay runtime
 */

int LIBRT_HayRuntime (void)
{
	SOCKET sock;

	if ( LIBMSG_Conectar(IP, PORTNUMBER, &sock) != 0 ) {
		return 0;
	} else {
		LIBMSG_Cerrar(sock);
		return 1;
	}
}


/*! \brief Elimina la zona criptográfica y deja el clauer todo a datos
 *
 *Elimina la zona criptográfica y deja el clauer todo a datos
 *
 * \retval ERR_LIBRT_NO
 *		   Correcto
 *
 * \retval !=0
 *		   Hubo error.
 */
int LIBRT_EliminarClauer( char * dispositivo ){
	return LIBRT_CrearClauer ( dispositivo, 100 );
}


/*! \brief Escribe un MBR con los porcentajes indicados
 *
 * Escribe un MBR con los porcentajes indicados
 *
 * \retval ERR_LIBRT_NO
 *		   Correcto
 *
 * \retval !=0
 *		   Hubo error.
 */
int LIBRT_CrearClauer ( char * dispositivo, char percent )
{
	int len= 0, i= 0, err=0;
    unsigned char aux= 0, funcion=129, acaba=0;
    SOCKET sock;
    
	if ( ! dispositivo )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;
	
	len= strlen(dispositivo);
	if ( len >= MAX_PATH_LEN )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;
		
	/*
	 * Petición de la función
	 */
	
	LOG_Msg(1, "Conectamos ... ");
	
	if ( (err=LIBMSG_Conectar(IP, PORTNUMBER, &sock)) != 0 ){
		LOG_Error(1, "Error al conectar ret= %d ... ", err);
		return ERR_LIBRT_SI;
	}

	LOG_Msg(1, "Enviamos la función 129");	
	if ( LIBMSG_Enviar(sock, (unsigned char *) &funcion, 1) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el número de función ... ");
		return ERR_LIBRT_SI;
	}

	LOG_Debug(1, "Enviamos la longitud del device: %d", len);
	if ( LIBMSG_Enviar(sock, (unsigned char *)&len, 4) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el número de función ... ");
		return ERR_LIBRT_SI;
	}

	
	
	LOG_Debug(1, "Enviamos els dispositivo: %s", dispositivo);
	if ( LIBMSG_Enviar(sock, (unsigned char *) dispositivo, strlen(dispositivo)) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el número de función ... ");
		return ERR_LIBRT_SI;
	}


	LOG_Debug(1, "Porcentaje: %d", percent);
	if ( LIBMSG_Enviar(sock, (unsigned char *)&percent, 1) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el número de función ... ");
		return ERR_LIBRT_SI;
	}


	/*
	 * Respuesta del runtime
	 */
	if ( (err= LIBMSG_Recibir(sock, (unsigned char *)&aux,1)) != 0 ) {
        //err= GetLastError();
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al recibir el resultado del error ... ");
		
		if ( aux == ERR_LIBRT_NO_ADMIN )
			return ERR_LIBRT_NO_ADMIN;
		
		return ERR_LIBRT_SI;
	}

	LIBMSG_Cerrar(sock);

	if ( aux != 0 ){
		LOG_Error(1, "Recibido error err= %d", aux );
		return ERR_LIBRT_SI;
	}

	LOG_Msg(1,"SALIENDO POR ERR_LIBRT_NO");	
	LIBRT_RegenerarCache();
	return ERR_LIBRT_NO;
}


/*! \brief Formatea la parte de datos del clauer
 *
 * Formatea la parte de datos del clauer.
 *
 * \retval ERR_LIBRT_NO
 *		   Correcto
 *
 * \retval !=0
 *		   Hubo error.
 */
int LIBRT_FormatearClauerDatos( char * dispositivo ){

	int len= 0, i= 0, err=0;
    unsigned char aux= 0, funcion=131, acaba=0;
    SOCKET sock;
    
	if ( ! dispositivo )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;
	
	len= strlen(dispositivo);
	if ( len >= MAX_PATH_LEN )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;
		
	/*
	 * Petición de la función
	 */
	
	LOG_Msg(1, "Conectamos ... ");
	
	if ( (err=LIBMSG_Conectar(IP, PORTNUMBER, &sock)) != 0 ){
		LOG_Error(1, "Error al conectar ret= %d ... ", err);
		return ERR_LIBRT_SI;
	}

	LOG_Msg(1, "Enviamos la función 131");	
	if ( LIBMSG_Enviar(sock, (unsigned char *) &funcion, 1) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el número de función ... ");
		return ERR_LIBRT_SI;
	}

	LOG_Debug(1, "Enviamos la longitud del device: %d", len);
	if ( LIBMSG_Enviar(sock, (unsigned char *)&len, 4) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el número de función ... ");
		return ERR_LIBRT_SI;
	}
	
	LOG_Debug(1, "Enviamos els dispositivo: %s", dispositivo);
	if ( LIBMSG_Enviar(sock, (unsigned char *) dispositivo, strlen(dispositivo)) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el número de función ... ");
		return ERR_LIBRT_SI;
	}

	/*
	 * Respuesta del runtime
	 */
	if ( (err= LIBMSG_Recibir(sock, (unsigned char *)&aux,1)) != 0 ) {
        //err= GetLastError();
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al recibir el resultado del error ... ");

		if ( aux == ERR_LIBRT_NO_ADMIN )
			return ERR_LIBRT_NO_ADMIN;
		
		return ERR_LIBRT_SI;
	}

	LIBMSG_Cerrar(sock);

	if ( aux != 0 )
		return ERR_LIBRT_SI;

	LOG_Msg(1,"SALIENDO POR ERR_LIBRT_NO");	
	return ERR_LIBRT_NO;
}


/*! \brief Formatea la parte criptográfica del clauer
 *
 * Formatea la parte criptográfica del clauer.
 *
 * \retval ERR_LIBRT_NO
 *		   Correcto
 *
 * \retval !=0
 *		   Hubo error.
 */
int LIBRT_FormatearClauerCrypto( char * dispositivo, char * pwd ){

int len= 0, i= 0, err=0, lenPwd=0;
    unsigned char aux= 0, funcion=132;
    SOCKET sock;
    
	if ( ! dispositivo )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;
	
	if ( ! pwd )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;

	len= strlen(dispositivo);
	if ( len >= MAX_PATH_LEN )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;
	
	lenPwd= strlen(pwd);
	if ( lenPwd >= MAX_PASS_LEN )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;
		
	/*
	 * Petición de la función
	 */
	
	LOG_Msg(1, "Conectamos ... ");
	
	if ( (err=LIBMSG_Conectar(IP, PORTNUMBER, &sock)) != 0 ){
		LOG_Error(1, "Error al conectar ret= %d ... ", err);
		return ERR_LIBRT_SI;
	}

	LOG_Msg(1, "Enviamos la función 132");	
	if ( LIBMSG_Enviar(sock, (unsigned char *) &funcion, 1) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el número de función ... ");
		return ERR_LIBRT_SI;
	}

	LOG_Debug(1, "Enviamos la longitud del device: %d", len);
	if ( LIBMSG_Enviar(sock, (unsigned char *)&len, 4) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el número de función ... ");
		return ERR_LIBRT_SI;
	}
	
	LOG_Debug(1, "Enviamos els dispositivo: %s", dispositivo);
	if ( LIBMSG_Enviar(sock, (unsigned char *) dispositivo, strlen(dispositivo)) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el número de función ... ");
		return ERR_LIBRT_SI;
	}

	if ( LIBMSG_Enviar(sock, (unsigned char *)&lenPwd, 4) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar la longitud del pwd ... ");
		return ERR_LIBRT_SI;
	}
	
	if ( LIBMSG_Enviar(sock, (unsigned char *) pwd, strlen(pwd)) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar pwd ... ");
		return ERR_LIBRT_SI;
	}

	/*
	 * Respuesta del runtime
	 */
	if ( (err= LIBMSG_Recibir(sock, (unsigned char *)&aux,1)) != 0 ) {
        //err= GetLastError();
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al recibir el resultado del error ... ");
		
		if ( aux == ERR_LIBRT_NO_ADMIN )
			return ERR_LIBRT_NO_ADMIN;
		
		return ERR_LIBRT_SI;
	}

	LIBMSG_Cerrar(sock);

	if ( aux != 0 )
		return ERR_LIBRT_SI;

	LOG_Msg(1,"SALIENDO POR ERR_LIBRT_NO");	
	return ERR_LIBRT_NO;
}




/*! \brief Obtiene el layout del clauer que representa la información sobre las particiones 
 *
 * Obtiene el layout del clauer que representa la información sobre las particiones
 *
 * \retval ERR_LIBRT_NO
 *		   Correcto
 *
 * \retval !=0
 *		   Hubo error.
 */
int LIBRT_ObtenerLayout( char * dispositivo, CLAUER_LAYOUT_INFO * cli){

	int len= 0, i= 0, err=0;
    unsigned char aux= 0, funcion=130, acaba=0;
    SOCKET sock;
    
	if ( ! dispositivo )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;
	
	if ( ! cli )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;

	len= strlen(dispositivo);
	if ( len >= MAX_PATH_LEN )
		return ERR_LIBRT_PARAMETRO_INCORRECTO;
		
	/*
	 * Petición de la función
	 */
	
	LOG_Msg(1, "Conectamos ... ");
	
	if ( (err=LIBMSG_Conectar(IP, PORTNUMBER, &sock)) != 0 ){
		LOG_Error(1, "Error al conectar ret= %d ... ", err);
		return ERR_LIBRT_SI;
	}

	LOG_Msg(1, "Enviamos la función 131");	
	if ( LIBMSG_Enviar(sock, (unsigned char *) &funcion, 1) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el número de función ... ");
		return ERR_LIBRT_SI;
	}

	LOG_Debug(1, "Enviamos la longitud del device: %d", len);
	if ( LIBMSG_Enviar(sock, (unsigned char *)&len, 4) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el número de función ... ");
		return ERR_LIBRT_SI;
	}
	
	LOG_Debug(1, "Enviamos els dispositivo: %s", dispositivo);
	if ( LIBMSG_Enviar(sock, (unsigned char *) dispositivo, strlen(dispositivo)) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el número de función ... ");
		return ERR_LIBRT_SI;
	}


	/* Recibimos cli */
	if ( (err= LIBMSG_Recibir(sock, (unsigned char *)cli,sizeof(*cli))) != 0 ) {
        //err= GetLastError();
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al recibir el resultado del error ... ");
		return ERR_LIBRT_SI;
	}


	/*
	 * Respuesta del runtime
	 */
	if ( (err= LIBMSG_Recibir(sock, (unsigned char *)&aux,1)) != 0 ) {
        //err= GetLastError();
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al recibir el resultado del error ... ");

		if ( aux == ERR_LIBRT_NO_ADMIN )
			return ERR_LIBRT_NO_ADMIN;
		
		return ERR_LIBRT_SI;
	}

	LIBMSG_Cerrar(sock);

	if ( aux != 0 )
		return ERR_LIBRT_SI;

	LOG_Msg(1,"SALIENDO POR ERR_LIBRT_NO");	
	return ERR_LIBRT_NO;
}
