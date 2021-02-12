
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

#ifndef __LOG_H__
#define __LOG_H__
/*
#include "block.h"
*/
#define LOG_TYPE_ERROR   0
#define LOG_TYPE_DEBUG   1

#define LOG_TO 1

#ifdef WIN32
#define LOG_FILE "c:\\pkcs11.log"
#elif defined(LINUX)
#define LOG_FILE "./pkcs11.log"
#endif

/* Para usar la librer�a es necesario definir la constante
 * simb�lica LOG en el fichero que se necesite. Cuando esta
 * constante no est� definida, las macros se definen como
 * blancos, de modo que el c�digo de loggeo se elimina.
 *
 * El compilador de microsoft no admite n�mero de argumentos
 * variables en macros, por lo tanto, la definici�n de �stas
 * LOG_Debug() y LOG_Error() cambian seg�n estemos o no utilizando
 * un compilador GNU C (LINUX) o no. Esto hace que �stas macros,
 * en la versi�n WIN32 sean un tanto m�s limitadas. Se a�ade
 * la macro LOG_Msg() que permite escribir un �nico mensaje
 * sin cadena de formato. Las macros LOG_Debug() y LOG_Error()
 * admiten cadena de formato con un �nico car�cter de conversi�n
 */

#ifndef LOG

#define LOG_Ini 
#define LOG_Msg
#define LOG_BeginFunc
#define LOG_EndFunc 
#define LOG_Debug
#define LOG_Error
#define LOG_MsgError 

#else 

#if defined(LINUX)

#define LOG_Debug(level,format, args...)   LOG_Write(LOG_TYPE_DEBUG, level, __FILE__,  __LINE__,  __FUNCTION__, format, ## args)
#define LOG_Error(level,format, args...)   LOG_Write(LOG_TYPE_ERROR, level, __FILE__,  __LINE__,  __FUNCTION__, format, ## args)
#define LOG_MsgError(level, msg)           LOG_Error(level, "%s", msg)
#define LOG_Msg(level, msg)                LOG_Debug(level, "%s", msg)

#elif defined(WIN32)

#define LOG_Debug(level,format, arg)   LOG_Write(LOG_TYPE_DEBUG, level, __FILE__, __LINE__, __FUNCTION__, format, arg)
#define LOG_Error(level,format, arg)   LOG_Write(LOG_TYPE_ERROR, level, __FILE__, __LINE__, __FUNCTION__, format, arg)
#define LOG_MsgError(level, msg)           LOG_Error(level, "%s", msg)
#define LOG_Msg(level, msg)            LOG_Debug(level, "%s", msg)

#endif

#define LOG_Ini(where, level)      LOG_Ini_Func (where, level)
#define LOG_BeginFunc(level)       LOG_Debug(level, "BEGIN %s", __FUNCTION__);
#define LOG_EndFunc(level,ret)     LOG_Write(LOG_TYPE_DEBUG, level, __FILE__, __LINE__, __FUNCTION__, "END %s : %d", __FUNCTION__, ret)

#endif /* defined(LOG) */

/*
 * Functions
 */

#define LOG_WHERE_STDERR  1
#define LOG_WHERE_FILE    2
#define LOG_WHERE_DEFAULT LOG_WHERE_FILE

int  LOG_Ini_Func ( int where, int level );
int  LOG_End      ( void );
void LOG_Write    ( int type, int level, const char *file, int line, const char *func, const char *format, ...);

/*void LOG_WriteBlockObject ( int level, block_object_t *ob );
void LOG_WriteBlockInfo   ( int level, block_info_t *ib );
*/


#endif

