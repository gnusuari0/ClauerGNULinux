
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

#ifndef __THREADS_H__
#define __THREADS_H__

#ifdef LINUX
#include <pthread.h>
#include <unistd.h>
#elif defined(WIN32)
#include <windows.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include "common.h"


#ifdef WIN32
#include <map>
#include <string>
struct PASS_CACHE_INFO {
    char pass[128];             // the password null terminated
    char id[20];             // the privkey id
    int ttl;              // time stamp. Indicates the last time the password was requested
};
#endif

using namespace std;

typedef struct ID_ACCESS_HANDLE {
#ifdef LINUX
    pthread_mutex_t clauerIdMutex; // Mutex that protects newClauerId
#elif defined(WIN32)
    HANDLE clauerIdMutex;
    HANDLE keyIdMutex;
    map<string, PASS_CACHE_INFO *> keyCache;
    int global_pass_ttl;
#endif
    unsigned char newClauerId[CLAUER_ID_LEN];
} ID_ACCESS_HANDLE;

#ifdef LINUX
void * updateIdThread(void * ptr);
#elif defined(WIN32)
DWORD WINAPI updateIdThread(void *ptr);
#endif

#endif
