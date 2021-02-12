
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

/* Para usar la librería es necesario definir la constante
 * simbólica LOG en el fichero que se necesite. Cuando esta
 * constante no está definida, las macros se definen como
 * blancos, de modo que el código de loggeo se elimina.
 *
 * El compilador de microsoft no admite número de argumentos
 * variables en macros, por lo tanto, la definición de éstas
 * LOG_Debug() y LOG_Error() cambian según estemos o no utilizando
 * un compilador GNU C (LINUX) o no. Esto hace que éstas macros,
 * en la versión WIN32 sean un tanto más limitadas. Se añade
 * la macro LOG_Msg() que permite escribir un único mensaje
 * sin cadena de formato. Las macros LOG_Debug() y LOG_Error()
 * admiten cadena de formato con un único carácter de conversión
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

