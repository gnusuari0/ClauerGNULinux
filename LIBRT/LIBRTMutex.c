
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
