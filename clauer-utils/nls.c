
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
  // A pesar de que la funci�n GetConsoleOutput parece que nos indica que el codepage es CP850,
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
