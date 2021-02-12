
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

//---------------------------------------------------------------------------

#include "libRT.h"
#include "log.h"


int gInitialized= 0;

/*! \brief Realiza una reconexi�n pidiendo el pin al usuario si es posible. Si no lo es
 *         devuelve error.
 *
 * Realiza una reconexi�n pidiendo el pin al usuario si es posible. Si no lo es
 * devuelve error. Los casos posibles de error:
 *
 *      -# El usuario ha cancelado la operaci�n. Puede ser el caso en el que
 *	       se disponga de una interfaz gr�fica.
 *
 *      -# El usuario ha seleccionado un dispositivo cuyo identificador no coincide
 *		   con el actual.
 *
 *      -# Alg�n otro error
 *
 * \param handle
 *		  El handle al runtime
 *
 * \retval 0
 *		   Ok
 *
 * \retval !=0
 *		   Error - no se pudo realizar la reconexi�n
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
	 * pues a fer la m�
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





/*! \brief Inicializa la librer�a. Es necesario llamar a esta funci�n antes de utilizar el resto.
 *
 * Inicializa la librer�a. Es necesario llamar a esta funci�n antes de utilizar el resto. Es un error
 * bastante com�n no llamar a esta funci�n. As� es que si algo va mal y no sabes por qu�... tachan, tachan...
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


/*! \brief Finaliza la librer�a. Es conveniente llamar a esta funci�n cuando ya no tengamos que usar LIBRT.
 *
 * Finaliza la librer�a. Es conveniente llamar a esta funci�n cuando ya no tengamos que usar LIBRT. Libera
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


/*! \brief Finaliza la utilizaci�n de un dispositivo. Cuando ya no vayamos a interactuar m�s con el clauer
 *         hay que llamar a esta funci�n.
 *
 * Finaliza la utilizaci�n de un dispositivo. Cuando ya no vayamos a interactuar m�s con el clauer
 * hay que llamar a esta funci�n.
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
	 * Env�o la petici�n. La atenci�n de la reconexi�n se hace s�lo en la
	 * primera funci�n.
	 */

	if ( LIBMSG_Enviar(handle->sock, (unsigned char *) &func, 1) != ERR_LIBMSG_NO ) {
		LIBRT_MUTEX_Unlock(handle->mutex);
		LOG_MsgError(1,"Enviando n�mero de func 1");	
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
 *        SALIDA. El n�mero de dispositivos disponibles.
 *
 * \param dispositivos
 *		  SALIDA. Array con los n�meros de dispositivos disponibles. La funci�n reserva
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
	 * Petici�n de la funci�n
	 */
	
	LOG_Msg(1, "Conectamos ... ");
	
	if ( (err=LIBMSG_Conectar(IP, PORTNUMBER, &sock)) != 0 ){
		LOG_Error(1, "Error al conectar ret= %d ... ", err);
		return ERR_LIBRT_SI;
	}

	LOG_Msg(1, "Enviamos la funci�n 0");	
	if ( LIBMSG_Enviar(sock, (unsigned char *) &funcion, 1) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el n�mero de funci�n ... ");
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
				LOG_Error(1, "Error recibiendo el tama�o de la siguiente cadena error = %d ... ", err);
				return ERR_LIBRT_SI;
		    }
			LOG_Debug(1, "Tama�o de cadena recibido len= %d", len );
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
 *        SALIDA. El n�mero de dispositivos disponibles.
 *
 * \param dispositivos
 *		  SALIDA. Array con los n�meros de dispositivos disponibles. La funci�n reserva
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
	 * Petici�n de la funci�n
	 */
	
	LOG_Msg(1, "Conectamos ... ");
	
	if ( (err=LIBMSG_Conectar(IP, PORTNUMBER, &sock)) != 0 ){
		LOG_Error(1, "Error al conectar ret= %d ... ", err);
		return ERR_LIBRT_SI;
	}

	LOG_Msg(1, "Enviamos la funci�n 0");	
	if ( LIBMSG_Enviar(sock, (unsigned char *) &funcion, 1) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el n�mero de funci�n ... ");
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
				LOG_Error(1, "Error recibiendo el tama�o de la siguiente cadena error = %d ... ", err);
				return ERR_LIBRT_SI;
		    }
			LOG_Debug(1, "Tama�o de cadena recibido len= %d", len );
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



/*! \brief F1. Inicializa un dispositivo para su posterior utilizaci�n.
 *
 * F1. Inicializa un dispositivo para su posterior utilizaci�n. La inicializaci�n
 * puede ser sin password o con password. 
 *
 * \param nDevice
 *        ENTRADA. El n�mero de dispositivo a utilizar. Puede obtenerse una lista
 *        de dispositivos disponibles mediante la funci�n LIBRT_ListarDispostivos().
 *
 * \param pwd
 *		  ENTRADA. La password que protege el clauer. Si el par�metro es NULL se inicia
 *        el dispositivo sin password, lo que �nicamente permite acceder a su zona p�blica.
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
	 * Petici�n de la funci�n
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




/*! \brief Wrapper F1. Esta funci�n gestiona la inicializaci�n de un dispositivo       
 *
 * F1. Gestiona la inicializaci�n de un dispositivo, regenerando la cache frente un 
 *     error e intentando volver a inicarlo contra el clos.
 *
 * \param nDevice
 *        ENTRADA. El n�mero de dispositivo a utilizar. Puede obtenerse una lista
 *        de dispositivos disponibles mediante la funci�n LIBRT_ListarDispostivos().
 *
 * \param pwd
 *		  ENTRADA. La password que protege el clauer. Si el par�metro es NULL se inicia
 *        el dispositivo sin password, lo que �nicamente permite acceder a su zona p�blica.
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
 * F2. Lee toda la zona reservada del dispositivo. Esta funci�n debe llamarse en
 * dos pasos. En el primero se pasa buffer como NULL, de esa forma obtenemos
 * en bytesBuffer.
 *
 * \param handle
 *        ENTRADA. El manejador de dispositivo obtenido mediante la funci�n
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
	 * Par�metro de salida, setup para 64/32  bits
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
	 * S�lo adquiero el mutex la primera vez que llamo
	 * a la funci�n. La segunda ya est� adquirido
	 */
	

	if ( ! buffer ) {
		if ( ! LIBRT_MUTEX_Lock(handle->mutex) ) 
			return ERR_LIBRT_SI;

		/*
		 * Env�o la petici�n. La atenci�n de la reconexi�n se hace s�lo en la
		 * primera funci�n.
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
 *        ENTRADA. El manejador de dispositivo obtenido mediante la funci�n
 *				   LIBRT_IniciarDispositivo().
 *
 * \param buffer
 *		  ENTRADA. El buffer que se escribir� en la zona reservada.
 *
 * \param bytesBuffer
 *	      ENTADA. Los datos que se escribir�n de buffer en la zona reservada.
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
	 * Env�o la petici�n
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
 *        ENTRADA. El manejador de dispositivo obtenido mediante la funci�n
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
	 * Env�o la petici�n
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
 *        ENTRADA. El manejador de dispositivo obtenido mediante la funci�n
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
	 * Env�o la petici�n
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




/*! \brief F6. Lee un bloque de la zona de objetos de la partici�n criptogr�fica.
 *
 * F6. Lee un bloque de la zona de objetos de la partici�n criptogr�fica.
 *
 * \param handle
 *        ENTRADA. El manejador de dispositivo obtenido mediante la funci�n
 *				   LIBRT_IniciarDispositivo().
 *
 * \param numBloque
 *		  ENTRADA. El n�mero de bloque que se quiere leer.
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
	 * Env�o la petici�n
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
 *        ENTRADA. El manejador de dispositivo obtenido mediante la funci�n
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
 *		  SALIDA. Indica el n�mero de bloque donde estaba almacenado.
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
 * F9. Lee todos los bloques crypto de un tipo determinado. Para usar la funci�n:
 *
 *		. La primera llamada pasamos handleBloques o bloques a NULL. De esta
 *		  forma se devuelve en numBloques el n�mero de bloques a reservar en
 *		  ambos buffers.
 *
 *		. En la segunda llamada se pasa ya bloques y handleBloques apuntando
 *		  a una zona reservada de tama�o suficiente. numBloques debe pasarse
 *		  con el valor obtenido en la llamada anterior.
 *
 * \param handle
 *		  ENTRADA. El handle del dispositivo.
 * 
 * \param tipo
 *		  ENTRADA. El tipo del bloque a leer.
 *
 * \param handleBloques
 *		  SALIDA. Vector de tama�o *numBloques que contiene los n�meros de bloque
 *				  que se leyeron.
 *
 * \param bloques
 *		  SALIDA. Vector de tama�o TAM_BLOQUE*(*numBloques)
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
	 * El mutex s�lo se adquiere en la primera llamada a la 
	 * funci�n
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
		 * Si el n�mero de bloques es cero, entonces
		 * liberamos el lock
		 */

		if ( *numBloques == 0 )
			LIBRT_MUTEX_Unlock(handle->mutex);

		*numBloques= (long) aux;

	} else {

		/*
		 * En la segunda llamada a la funci�n recibimos los datos
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


/*! \brief F11. Escribe un bloque en la posici�n (numBloque) especificada.
 *
 * F11. Escribe un bloque en la posici�n (numBloque) especificada.
 *
 * \param handle
 *        ENTRADA. El manejador de dispositivo obtenido mediante la funci�n
 *				   LIBRT_IniciarDispositivo().
 *
 * \param numBloque
 *		  ENTRADA. N�mero de bloque de la zona de objetos donde queremos insertar
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
	 * Env�o la petici�n
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



/*! \brief F12. Inserta un bloque en cualquier posici�n libre del clauer.
 *
 * F12. Inserta un bloque en cualquier posici�n libre del clauer.
 *
 * \param handle
 *        ENTRADA. El manejador de dispositivo obtenido mediante la funci�n
 *				   LIBRT_IniciarDispositivo().
 *
 * \param bloque
 *		  ENTRADA. El bloque a insertar.
 *
 * \param numBloque
 *		  SALIDA. El n�mero de bloque donde vamos a insertar numBloque.
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
	 * Env�o la petici�n
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
 *        ENTRADA. El manejador de dispositivo obtenido mediante la funci�n
 *				   LIBRT_IniciarDispositivo().
 *
 * \param numBloque
 *		  ENTRADA. La posici�n del bloque que queremos borrar.
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
	 * Env�o la petici�n
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
 *        ENTRADA. El manejador de dispositivo obtenido mediante la funci�n
 *				   LIBRT_IniciarDispositivo().
 *
 * \param handleBloques
 *		  SALIDA. Vector con las posiciones de cada bloque devuelto. Tama�o: *numBloques * 4
 *
 * \param bloques
 *		  SALIDA. Bloques devueltos. Tama�o: TAM_BLOQUE * *numBloques
 *
 * \param numBloques
 *		  SALIDA. El n�mero de bloques a devolver.
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
		 * Env�o la petici�n
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

		/* Recibo el n�mero de bloques que hay
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
     * Env�o la petici�n
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
     * Petici�n de la funci�n
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
   ** Petici�n de la funci�n
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


/*! \brief Establece el callback para la petici�n del pin en las reconexiones debidas al vencimiento
 *		   del timeout.
 *
 * Establece el callback para la petici�n del pin en las reconexiones debidas al vencimiento
 * del timeout.
 *
 * \param handle
 *		  El handle al runtime.
 *
 * \param pin_cb
 *		  Puntero a la funci�no que utilizaremos.
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



/* \brief A�ade datos de aplicaci�n al handle.
 *
 * A�ade datos de aplicaci�n al handle. Estos datos son los que se le pasar�n a la funci�n
 * callback de reconexi�n.
 *
 * \param handle
 *		  El handle al runtime.
 *
 * \param appData
 *		  Los datos de aplicaci�n que queremos a�adir al handle.
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


/*! \brief Elimina la zona criptogr�fica y deja el clauer todo a datos
 *
 *Elimina la zona criptogr�fica y deja el clauer todo a datos
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
	 * Petici�n de la funci�n
	 */
	
	LOG_Msg(1, "Conectamos ... ");
	
	if ( (err=LIBMSG_Conectar(IP, PORTNUMBER, &sock)) != 0 ){
		LOG_Error(1, "Error al conectar ret= %d ... ", err);
		return ERR_LIBRT_SI;
	}

	LOG_Msg(1, "Enviamos la funci�n 129");	
	if ( LIBMSG_Enviar(sock, (unsigned char *) &funcion, 1) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el n�mero de funci�n ... ");
		return ERR_LIBRT_SI;
	}

	LOG_Debug(1, "Enviamos la longitud del device: %d", len);
	if ( LIBMSG_Enviar(sock, (unsigned char *)&len, 4) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el n�mero de funci�n ... ");
		return ERR_LIBRT_SI;
	}

	
	
	LOG_Debug(1, "Enviamos els dispositivo: %s", dispositivo);
	if ( LIBMSG_Enviar(sock, (unsigned char *) dispositivo, strlen(dispositivo)) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el n�mero de funci�n ... ");
		return ERR_LIBRT_SI;
	}


	LOG_Debug(1, "Porcentaje: %d", percent);
	if ( LIBMSG_Enviar(sock, (unsigned char *)&percent, 1) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el n�mero de funci�n ... ");
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
	 * Petici�n de la funci�n
	 */
	
	LOG_Msg(1, "Conectamos ... ");
	
	if ( (err=LIBMSG_Conectar(IP, PORTNUMBER, &sock)) != 0 ){
		LOG_Error(1, "Error al conectar ret= %d ... ", err);
		return ERR_LIBRT_SI;
	}

	LOG_Msg(1, "Enviamos la funci�n 131");	
	if ( LIBMSG_Enviar(sock, (unsigned char *) &funcion, 1) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el n�mero de funci�n ... ");
		return ERR_LIBRT_SI;
	}

	LOG_Debug(1, "Enviamos la longitud del device: %d", len);
	if ( LIBMSG_Enviar(sock, (unsigned char *)&len, 4) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el n�mero de funci�n ... ");
		return ERR_LIBRT_SI;
	}
	
	LOG_Debug(1, "Enviamos els dispositivo: %s", dispositivo);
	if ( LIBMSG_Enviar(sock, (unsigned char *) dispositivo, strlen(dispositivo)) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el n�mero de funci�n ... ");
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


/*! \brief Formatea la parte criptogr�fica del clauer
 *
 * Formatea la parte criptogr�fica del clauer.
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
	 * Petici�n de la funci�n
	 */
	
	LOG_Msg(1, "Conectamos ... ");
	
	if ( (err=LIBMSG_Conectar(IP, PORTNUMBER, &sock)) != 0 ){
		LOG_Error(1, "Error al conectar ret= %d ... ", err);
		return ERR_LIBRT_SI;
	}

	LOG_Msg(1, "Enviamos la funci�n 132");	
	if ( LIBMSG_Enviar(sock, (unsigned char *) &funcion, 1) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el n�mero de funci�n ... ");
		return ERR_LIBRT_SI;
	}

	LOG_Debug(1, "Enviamos la longitud del device: %d", len);
	if ( LIBMSG_Enviar(sock, (unsigned char *)&len, 4) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el n�mero de funci�n ... ");
		return ERR_LIBRT_SI;
	}
	
	LOG_Debug(1, "Enviamos els dispositivo: %s", dispositivo);
	if ( LIBMSG_Enviar(sock, (unsigned char *) dispositivo, strlen(dispositivo)) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el n�mero de funci�n ... ");
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




/*! \brief Obtiene el layout del clauer que representa la informaci�n sobre las particiones 
 *
 * Obtiene el layout del clauer que representa la informaci�n sobre las particiones
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
	 * Petici�n de la funci�n
	 */
	
	LOG_Msg(1, "Conectamos ... ");
	
	if ( (err=LIBMSG_Conectar(IP, PORTNUMBER, &sock)) != 0 ){
		LOG_Error(1, "Error al conectar ret= %d ... ", err);
		return ERR_LIBRT_SI;
	}

	LOG_Msg(1, "Enviamos la funci�n 131");	
	if ( LIBMSG_Enviar(sock, (unsigned char *) &funcion, 1) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el n�mero de funci�n ... ");
		return ERR_LIBRT_SI;
	}

	LOG_Debug(1, "Enviamos la longitud del device: %d", len);
	if ( LIBMSG_Enviar(sock, (unsigned char *)&len, 4) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el n�mero de funci�n ... ");
		return ERR_LIBRT_SI;
	}
	
	LOG_Debug(1, "Enviamos els dispositivo: %s", dispositivo);
	if ( LIBMSG_Enviar(sock, (unsigned char *) dispositivo, strlen(dispositivo)) != 0 ) {
		LIBMSG_Cerrar(sock);
		LOG_MsgError(1, "Error al enviar el n�mero de funci�n ... ");
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
