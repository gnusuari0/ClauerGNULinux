
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

#include "log.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#ifdef LINUX 
#include <unistd.h>
#elif defined(WIN32)
#include <windows.h>
#endif


static FILE *g_hLog = NULL;
static int g_logLevel = 0;


unsigned long GetPID (void)
{
  /* 
   * Both linux and windows assigns a different pid for
   * the threads
   */

#ifdef LINUX
  return getpid();
#elif defined(WIN32)
	return GetCurrentThreadId();
  //return GetCurrentProcessId();
#endif

}



int LOG_Ini_Func ( int where, int level )
{
	if ( ! g_hLog ) {
		switch ( where ) {
	case LOG_WHERE_FILE:
		g_hLog = fopen (LOG_FILE, "a");
		if ( ! g_hLog ) 
			return -1;
		break;

	case LOG_WHERE_STDERR:
		g_hLog = stderr;
		break;

	default:
		return -1;
		}
	}

  
  g_logLevel = level;

  return 0;
  
}


int LOG_End ( void )
{
  
  if ( ! g_hLog )
    return 0;

  if ( g_hLog != stderr )
    if ( fclose(g_hLog) == EOF )
      return -1;


  return 0;

}



#define MAX_DEBUG_LINE_LEN     80

void LOG_Write ( int type, int level, char *file, int line, const char *func, const char *format, ...)
{
  va_list ap;
  long seconds;

#ifdef WIN32
  char *aux;
  size_t size;
#endif

  if ( 0 == g_logLevel )
    return;

#ifdef WIN32
  size = strlen(file);
  aux = file+size-1;
  while ( (*aux) != '\\' && (aux != file) )
	  --aux;
  file = aux+1;
#endif

  if ( level <= g_logLevel ) {

    if ( ! g_hLog )
      return;

    va_start(ap, format);
    
    if ( LOG_TYPE_ERROR == type )
      fprintf(g_hLog, "[ERROR] ");
    else
      fprintf(g_hLog, "[DEBUG] ");
   
	seconds= (long)time(NULL);
	fprintf(g_hLog, "%ld:%ld:%s:%d:%s: ", GetPID(), seconds, file, line, func);
   
    vfprintf(g_hLog, format, ap);
    
    fprintf(g_hLog, "\n");
    fflush(g_hLog);
    
    va_end(ap);
  }
}


void LOG_WriteBlockInfo ( int level, block_info_t *ib )
{

  register int i;

  if ( 0 == g_logLevel )
    return;

  if ( level <= g_logLevel ) {
    fprintf(g_hLog, "[DEBUG] INFO BLOCK DUMP\n");

    fprintf(g_hLog, "        ID: ");
    for ( i = 0 ; i < 20 ; i++ ) 
      fprintf(g_hLog, "%02x", ib->id[i]);
    fprintf(g_hLog, "\n");

    fprintf(g_hLog, "        Reserved Zone Blocks: %ld\n", ib->rzSize);
    fprintf(g_hLog, "        Current Block: %ld\n", ib->cb);
    fprintf(g_hLog, "        Object Zone Blocks: %ld\n", ib->totalBlocks);
    fprintf(g_hLog, "        Version: %ld\n", ib->version);   
  }

}



void LOG_WriteBlockObject   ( int level, block_object_t *ob )
{

  if ( 0 == g_logLevel )
    return;

  if ( level <= g_logLevel ) {
    fprintf(g_hLog, "[DEBUG] OBJECT BLOCK DUMP\n");

    fprintf(g_hLog, "        Mode: %d\n", ob->mode);
    fprintf(g_hLog, "        Type: %d\n", ob->type);
  }

}
