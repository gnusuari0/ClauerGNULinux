
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

#include "nls.h"

#ifndef MAC
int nls_lang_init()
{
  
#ifdef WIN32
  char str_codepage[64];
  UINT codepage; 
#endif 
    
    /* Reset the locale variables LC_* */
    setlocale(LC_ALL, "");
  
#ifdef LINUX
  setlocale(LC_TIME, "" );
  setlocale(LC_MESSAGES, "");
#endif
 

  /* Bind with MO File */
  bindtextdomain( PACKAGE, LOCALE_PATH );
  textdomain( PACKAGE );

  /* Get system locale codepage and set gettext 
   * translation to this locale codepage. 
   */
  
#ifdef LINUX 
  if ( ! bind_textdomain_codeset( PACKAGE, nl_langinfo(CODESET) )){
    fprintf(stderr,_("[ERROR] Changing the language to your locale codepage, messages will be printed in English"));
    return 1;
  }
#elif WIN32
  codepage= GetConsoleOutputCP(); 
  str_codepage[0]='C';  str_codepage[1]='P';
  _itoa_s( codepage, str_codepage+2, 62 , 10 ); 
  
  // printf( "OBTENIDO CODEPAGE: %s y codepage del terminal= %d \n", str_codepage, codepage );
  // A pesar de que la función GetConsoleOutput parece que nos indica que el codepage es CP850,
  // el codepage que muestra bien los acentos tanto abiertos como cerrados es el CP1252, existe
  // un problema al mostrar mayusculas acentuadas.
  if ( ! bind_textdomain_codeset( PACKAGE, str_codepage )){
    fprintf(stderr,_("[ERROR] Changing the language to your locale codepage, messages will be printed in English"));
    return 1;
  }
#endif
  return 0;
}
#endif
