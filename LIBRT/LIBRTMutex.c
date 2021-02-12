
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

#include "LIBRTMutex.h"



int LIBRT_MUTEX_Crear (LIBRT_MUTEX *m)
{
#if defined(WIN32)
	*m = CreateMutex (NULL, 0, NULL);

	if ( *m == NULL )
		return 0;
	else
		return 1;
#elif defined(LINUX)
	pthread_mutex_init(m, NULL);
	return 1;
#endif
}



int LIBRT_MUTEX_Destruir(LIBRT_MUTEX m)
{

#if defined(WIN32)
	return CloseHandle(m);
#elif defined(LINUX)
	if ( pthread_mutex_destroy(&m) != 0 )
	  return 0;
	else
	  return 1;
#endif

}



int LIBRT_MUTEX_Lock (LIBRT_MUTEX m)
{
#if defined(WIN32)
	if ( WaitForSingleObject(m, LIBRT_MUTEX_TIMEOUT) == WAIT_FAILED ) {
		return 0;
	} else {
		return 1;
	}
#elif defined(LINUX)
	if ( pthread_mutex_lock(&m) != 0 )
	  return 0;
	else
	  return 1;
#endif
}


int LIBRT_MUTEX_Unlock (LIBRT_MUTEX m)
{
#if defined(WIN32)
	if ( ReleaseMutex(m) == 0 ) {
		return 0;
	} else {
		return 1;
	}
#elif defined(LINUX)
	if (pthread_mutex_unlock(&m) != 0 )
	  return 0;
	else
	  return 1;
#endif
}
