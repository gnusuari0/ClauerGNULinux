
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

#include "libMSG.h"


/*! \brief Inicializa la librería.
 *
 * Inicializa la librería. Antes de llamar a cualquier otra función es
 * necesario llamar a ésta.
 *
 * \retval 1
 *         ERROR
 *
 * \retval 0
 *         ÉXITO
 *
 * \remarks En linux no es necesario realizar esta llamada
 */

int LIBMSG_Ini (void)
{
#ifdef WIN32
  WSADATA info;
  DWORD err=0, i=0;

  if ( WSAStartup(MAKEWORD(1,1), &info) != 0 ){
		err= GetLastError();
		while ( (err == WSANOTINITIALISED) && (i != 10) ){
			Sleep(100);	i++;
			WSAStartup(MAKEWORD(1,1), &info);
			err= GetLastError();
			if ( err == 0 ){
				return 0;
			}
		}
	  return 1;
  }
#endif
  
  return 0;
}


/*! \brief Finaliza la librería.
 *
 * Finaliza la librería. Cuando ya no se vaya a utilizar más libMSG debe
 * llamarse a esta función.
 *
 * \retval 1
 *         ERROR
 *
 * \retval 0
 *         ÉXITO
 *
 * \remarks En linux no es necesario realizar esta llamada.
 */

int LIBMSG_Fin (void)
{
#ifdef WIN32
  WSACleanup();
#endif
  return 0;
}



/*! \brief Envía los datos de buffer por el socket socket.
 *
 * Envía los datos de buffer por el socket socket.
 *
 * \param socket
 *        Un socket con conexión establecida.
 *
 * \param buffer
 *        Los datos a enviar.
 *
 * \param tamBuffer
 *        Los bytes a enviar de buffer.
 *
 * \retval ERR_LIBMSG_TIMEOUT
 *		   Vencido el timeout
 *
 * \retval ERR_LIBMSG_SI
 *         ERROR
 *
 * \retval ERR_LIBMSG_NO
 *         �XITO
 *
 * \todo Timeouts en linux
 *
 */

int LIBMSG_Enviar  (SOCKET socket, unsigned char *buffer, int tamBuffer)
{
  int env = 0, n, err=0;
  
#ifdef LINUX
  time_t t_ini= 0, t_curr= 0;
  t_ini= time(NULL);
#endif


  while ( env < tamBuffer ) {
    n = send(socket, buffer+env, tamBuffer-env, 0);

#ifdef WIN32
    if (n == SOCKET_ERROR) {
      if ( (err=WSAGetLastError()) == WSAETIMEDOUT )
		return ERR_LIBMSG_TIMEOUT;
      else
		return ERR_LIBMSG_SI;
	}

#elif LINUX
    t_curr= time(NULL);
    //printf("Paso n=%d, tamBuffer=%d y tcurr - t_ini=%d", n, tamBuffer, (t_curr - t_ini));
    if ( (n == -1) || (t_curr - t_ini > MSGTIMEOUT))
      return ERR_LIBMSG_SI;
#endif
    
    env += n;
  }
  
  return ERR_LIBMSG_NO;
}



/*! \brief Lee tamBuffer bytes de socket y los almacena en buffer.
 *
 * Lee tamBuffer bytes de socket y los almacena en buffer.
 *
 * \param socket
 *        El socket del que vamos a leer.
 *
 * \param buffer
 *        El buffer donde se almacenarán los datos
 *
 * \param tamBuffer
 *        Los bytes a leer de socket.
 *
 * \retval ERR_LIBMG_TIMEOUT
 *         Vencido el timeuot
 *
 * \retval ERR_LIBMSG_SI
 *         ERROR
 *
 * \retval ERR_LIBMSG_NO
 *         �XITO
 *
 * \todo Devolver un error de timeout en linux. No s� exactamente c�mo se hace
 */

int LIBMSG_Recibir (SOCKET socket, unsigned char * buffer, int tamBuffer)
{
  int rec = 0, n, err;
 
#ifdef LINUX
  time_t t_ini= 0, t_curr= 0;
  t_ini= time(NULL);
#endif 

  while(rec < tamBuffer) {
    n = recv(socket, buffer+rec, tamBuffer-rec, 0);
#ifdef WIN32

    if (n == SOCKET_ERROR)
      if ( (err=WSAGetLastError()) == WSAETIMEDOUT )
		return ERR_LIBMSG_TIMEOUT;
	  else if ( err == WSAECONNRESET ){
		return ERR_LIBMSG_CONNRESET;
	  }else{
		return ERR_LIBMSG_SI;
	  }
#elif LINUX
    t_curr= time(NULL);
    //printf("Paso n=%d, tamBuffer=%d y tcurr - t_ini=%d", n, tamBuffer, (t_curr - t_ini));
    if ( (n == -1) || (t_curr - t_ini > MSGTIMEOUT))
      return ERR_LIBMSG_SI;

#endif
    
    rec += n;
  }
  return ERR_LIBMSG_NO;

}



/*! \brief Cierra el socket socket.
 *
 * Cierra el socket socket
 *
 * \param socket
 *        El socket a cerrar.
 *
 * \retval 1
 *         ERROR
 *
 * \retval 0
 *         ÉXITO
 */

int LIBMSG_Cerrar (SOCKET socket)
{
/* For winsock2
 * Note:  To assure that all data is sent and received on a connection, 
   * 			an application should call shutdown before calling closesocket 
   *			(see Graceful shutdown, linger options, and socket closure for more information). 
   *			Also note, an FD_CLOSE network event is not posted after closesocket is called.  
   * 
   * Reference: http://msdn2.microsoft.com/en-us/library/ms737582.aspx		
   *			http://msdn2.microsoft.com/en-us/library/ms738547.aspx
   */
 /* To avoid TIME_WAIT connection issues */
  struct linger linger = {1, 0};  

  /* abort the connection entirely */
  setsockopt(socket, SOL_SOCKET, SO_LINGER, (void *) &linger, sizeof(linger)); 

  if ( closesocket(socket) == SOCKET_ERROR )
    return 1;
  else
    return 0; 
}



/*! \brief Establece una conexión TCP/IP en la ip ip y puerto portnum.
 *
 * Establece una conexión TCP/IP en la ip ip y puerto portnum.
 *
 * \param ip
 *        La dirección IP de la máquina con la que queremos conectar. Es una
 *        cadena de caracteres.
 *
 * \param portnum
 *        El puerto con el que queremos conectar.
 *
 * \param s
 *        Socket de salida.
 *
 * \retval 1
 *         ERROR
 *
 * \retval 0
 *         ÉXITO
 */

int LIBMSG_Conectar (const char *ip, unsigned short portnum, SOCKET *s)
{
  struct sockaddr_in sa;
  int err;
  
  sa.sin_family = AF_INET;
  
#if defined(WIN32)
  
  sa.sin_port = htons((u_short)portnum);
  sa.sin_addr.S_un.S_addr = inet_addr(ip);
  
#elif defined(LINUX)
  
  sa.sin_port = htons(portnum);
  
  if ( !inet_aton(ip, &(sa.sin_addr)) )
    return 1;
  
#endif
  
  memset(&(sa.sin_zero), 0, 8);
  
  *s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP );
  if (*s == INVALID_SOCKET){
      err= ERR_LIBMSG_SI;
#ifdef WIN32  
      err= WSAGetLastError(); 
#endif  
      return err;
  }
  /* try to connect to the specified socket */
  
  if (connect(*s, (struct sockaddr *)&sa, sizeof(sa)) == SOCKET_ERROR) {
      err= ERR_LIBMSG_SI;
#ifdef WIN32  
      err= WSAGetLastError(); 
#endif      
      closesocket(*s);
      return err;
  }
  
  return 0;
}


