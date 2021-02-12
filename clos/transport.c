
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

#include "transport.h"
#include "stub.h"
#include <string.h>

#ifdef LINUX

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#include <errno.h>

#elif defined(WIN32)

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include <shlwapi.h>
#include <WtsApi32.h>

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "shlwapi.lib")

#define MAX_NAME 256
//#define  _WIN32_WINNT                      0x0501

#endif


struct i_trans_object {
#if defined(LINUX)
  int fd;
#elif defined(WIN32)
  SOCKET fd;
#endif
  int port, rpid;
  struct sockaddr_in myAddr;
};
typedef struct i_trans_object i_trans_object_t;

#ifdef LINUX
extern int errno;
#endif



#define LISTEN_ADDR   "127.0.0.1"
#define LISTEN_PORT   969
#define PENDING_QUEUE 8             /* how many pending connections will hold */
#define LOG_TO 2


int TRANS_Init ( void )
{
#if defined(WIN32)
  WSADATA wsaData;

  if ( WSAStartup(MAKEWORD(2,2), &wsaData) != 0 )
    return ERR_CLOS;
#endif

  return CLOS_SUCCESS;
}




/*! \brief Creates a new transport object that listens at 127.0.0.1:969.
 *
 * Creates a new transport object that listens at 127.0.0.1:969.
 *
 * \param tr_listener
 *        The transport object that will be initialized.
 *
 * \retval CLOS_SUCCESS
 *         Ok
 *
 * \retval ERR_INVALID_PARAMETER
 *         tr_listener is NULL.
 *
 * \retval ERR_OUT_OF_MEMORY
 *         Can't allocate memory
 *
 * \retval ERR_TRANS_SOCKET
 *         Error creating a socket
 *
 * \retval ERR_TANS_BIND
 *         Error binding the socket to the address and port
 *
 * \retval ERR_TRANS_LISTEN
 *         Error at listening address
 *
 */

int TRANS_Create  ( trans_object_t *tr_listener )
{

  int opt;
  i_trans_object_t *i_tr_listener = NULL;

#if defined(WIN32)
//  BOOL val;
  int rcvTimeout = (RCV_TIMEOUT+10) * 1000;
  //struct linger vlinger;
#endif


  if ( ! tr_listener )
    return ERR_INVALID_PARAMETER;

  i_tr_listener = (i_trans_object_t *) malloc (sizeof(i_trans_object_t));

  if ( ! i_tr_listener ) 
    return ERR_OUT_OF_MEMORY;

#if defined(LINUX)
  // printf("Pasamos por la creación del socket\n");
  // i_tr_listener->fd = socket(PF_LOCAL, SOCK_STREAM, 0);
  i_tr_listener->fd = socket(PF_INET, SOCK_STREAM, 0);
  if ( i_tr_listener->fd == -1 ) {
    free(i_tr_listener);
    // printf("No pude crear el socket local\n");
    return ERR_TRANS_SOCKET;
  }
#elif defined(WIN32)
  /* We use WSASocket instead of socket() to obtain a non-overlapped
   * socket
   */

/*  i_tr_listener->fd = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);*/
  i_tr_listener->fd = socket(AF_INET, SOCK_STREAM, 0);
  if ( i_tr_listener->fd == INVALID_SOCKET ) {
    free(i_tr_listener);
    return ERR_TRANS_SOCKET;
  }
#endif

  
  memset((void *) &(i_tr_listener->myAddr), 0, sizeof(struct sockaddr_in));

  i_tr_listener->myAddr.sin_family      = AF_INET;
  //i_tr_listener->myAddr.sin_family      = AF_LOCAL;
  
  i_tr_listener->myAddr.sin_port        = htons(LISTEN_PORT);
  i_tr_listener->myAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Local connections only.
  					 			// INADDR_ANY; // automatically fill with my IP
#ifdef WIN32
  opt = 1;
  if ( setsockopt(i_tr_listener->fd, SOL_SOCKET, SO_REUSEADDR,  (const char *)&opt, sizeof (opt)) == SOCKET_ERROR ) {
  //if ( setsockopt(i_tr_listener->fd, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (const char *) &val, sizeof val) == SOCKET_ERROR ) {
    LOG_Error(5, "Failed to set SO_EXCLUSIVEADDRUSE socket option: %ld", GetLastError());
	closesocket(i_tr_listener->fd);
    free(i_tr_listener);
    return ERR_TRANS_SOCKET;
  }
  if ( setsockopt(i_tr_listener->fd, SOL_SOCKET, SO_RCVTIMEO,(const char *) &rcvTimeout, sizeof rcvTimeout) == SOCKET_ERROR) 
	  LOG_Error(5, "Failed to ser SO_RCVTIMEOUT socket option: %ld", GetLastError());

#endif
  
#if defined(LINUX)
  opt = 1;
  if (setsockopt ( i_tr_listener->fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (opt)) < 0){
    free(i_tr_listener);
    // printf("Error al hacer el setsockopt errno=%d \n",errno);
    return ERR_TRANS_SOCKET;
  }  
  if ( bind(i_tr_listener->fd, (struct sockaddr *) &(i_tr_listener->myAddr), sizeof(struct sockaddr_in)) == -1 ) {
    free(i_tr_listener);
    // printf("Error al hacer el bind errno=%d \n",errno);
    return ERR_TRANS_BIND;
  }
#elif defined(WIN32)
  if ( bind(i_tr_listener->fd, (struct sockaddr *) &(i_tr_listener->myAddr), sizeof(struct sockaddr_in)) == SOCKET_ERROR ) {
	closesocket(i_tr_listener->fd);
    free(i_tr_listener);
    return ERR_TRANS_BIND;
  }
#endif

#if defined(LINUX)
  if ( listen(i_tr_listener->fd, PENDING_QUEUE) == -1 ) {
    free(i_tr_listener);
    return ERR_TRANS_LISTEN;
  }
#elif defined(WIN32)
  if ( listen(i_tr_listener->fd, PENDING_QUEUE) == SOCKET_ERROR ) {
	closesocket(i_tr_listener->fd);
    free(i_tr_listener);
    return ERR_TRANS_LISTEN;
  }
#endif
  
  *tr_listener = (trans_object_t) i_tr_listener;
  
  return CLOS_SUCCESS;
}

/*! \brief Closes a previously opened transport object.
 *
 * Closes a previously opened transport object.
 *
 * \param tr
 *        The transport object to be closed
 *
 * \retval CLOS_SUCESS
 *         Ok
 *
 * \retval ERR_INVALID_PARAMTER
 *         tr isn't a valid parameter
 *
 * \retval ERR_TRANS_CLOSE
 *         Error closing the socket
 *
 */

int TRANS_Close   ( trans_object_t tr )
{
  i_trans_object_t *i_tr = NULL;
  // struct linger linger = {1, 0};  

  if ( ! tr ){	
	  LOG_MsgError(1,"invalid parameter");	   
	  return ERR_INVALID_PARAMETER;
  }
  i_tr = ( i_trans_object_t *) tr;

#if defined(LINUX)
  if ( close(i_tr->fd) == -1 ){ 
	 LOG_Debug(LOG_TO, "ERROR al cerrar el socket frente a timeout errno= %d", errno); 
         return ERR_TRANS_CLOSE;
  }
  LOG_Msg(LOG_TO, "Cerrando Socket"); 
  
#elif defined(WIN32)

  /* abort the connection entirely */
  // setsockopt(socket, SOL_SOCKET, SO_LINGER, (void *) &linger, sizeof(linger)); 


  if ( closesocket(i_tr->fd) == SOCKET_ERROR ){
      LOG_Debug(1,"Socket error= %d", WSAGetLastError());		   
	  return ERR_TRANS_CLOSE;
  }
#endif

  free(i_tr);
  i_tr= NULL; 

  return CLOS_SUCCESS;
}


int TRANS_Close_Ex   ( trans_object_t tr )
{
  i_trans_object_t *i_tr = NULL;

  if ( ! tr )
    return ERR_INVALID_PARAMETER;

  i_tr = ( i_trans_object_t *) tr;

#if defined(LINUX)
  if ( close(i_tr->fd) == -1 ){ 
	 LOG_Debug(LOG_TO, "ERROR al cerrar el socket frente a timeout errno= %d", errno); 
         return ERR_TRANS_CLOSE;
  }
#elif defined(WIN32)


  /* Note:  To assure that all data is sent and received on a connection, 
   * 			an application should call shutdown before calling closesocket 
   *			(see Graceful shutdown, linger options, and socket closure for more information). 
   *			Also note, an FD_CLOSE network event is not posted after closesocket is called.  
   * 
   * Reference: http://msdn2.microsoft.com/en-us/library/ms737582.aspx		
   *			http://msdn2.microsoft.com/en-us/library/ms738547.aspx
   */

  if ( shutdown(i_tr->fd, SD_SEND ) == SOCKET_ERROR )
	  return ERR_TRANS_CLOSE;
  
  if ( closesocket(i_tr->fd) == SOCKET_ERROR )
    return ERR_TRANS_CLOSE;
#endif

  free(i_tr);
  i_tr= NULL; 

  return CLOS_SUCCESS;
}




/*! \brief Blocks for a connection at tr_listener and returns a client socket
 *         in tr_client parameter
 *
 * \param tr_listener
 *        The object where it'll be listening
 *
 * \param tr_client
 *        The client transport object
 *
 * \retval CLOS_SUCCESS
 *         Ok
 *
 * \retval ERR_INVALID_PARAMETER
 *         One or both of the parameters are invalid.
 *
 * \retval ERR_TRANS_ACCEPT
 *         Error accepting connections
 *
 * \retval ERR_OUT_OF_MEMORY
 *         Cannot allocate memory
 *
 */

int TRANS_Accept  ( trans_object_t tr_listener, trans_object_t *tr_client )
{
  i_trans_object_t *i_tr_listener, *i_tr_client;
  struct sockaddr_in peerAddr;
#if defined(LINUX)
  socklen_t size;
  int fd_client;
#elif defined(WIN32)
  int size;
  SOCKET fd_client;
#endif

  LOG_BeginFunc(5);

  /* Argument check */

  if ( ! tr_client ){
      LOG_Debug(LOG_TO, "Invalid parm, err= %d", errno);
      return ERR_INVALID_PARAMETER;
  }
  if ( ! tr_listener ){
      LOG_Debug(LOG_TO, "Invalid parm, err= %d", errno);
      return ERR_INVALID_PARAMETER;
  }
  *tr_client = NULL;
  i_tr_listener = (i_trans_object_t *) tr_listener;

  /* Accept conection (blocking) */

  size = sizeof(struct sockaddr_in);
  fd_client = accept(i_tr_listener->fd, (struct sockaddr *) &peerAddr, &size);
  LOG_Debug(1, "peerAddr port=%d", ntohs(peerAddr.sin_port));


#if defined(LINUX)
  if ( fd_client == -1) {
    LOG_EndFunc(5, ERR_TRANS_ACCEPT);
    LOG_Debug(LOG_TO, "fd_client , err= %d", errno);
    return ERR_TRANS_ACCEPT;
  }
#elif defined(WIN32)
  if ( fd_client == INVALID_SOCKET ) {
    LOG_EndFunc(5, ERR_TRANS_ACCEPT);
    return ERR_TRANS_ACCEPT;
  }
#endif


  /* Construct the client's transport object */

  i_tr_client = (i_trans_object_t *) malloc ( sizeof(i_trans_object_t) );

  if ( ! i_tr_client ){
      LOG_Debug(LOG_TO, "Outmem , err= %d", errno);
      return ERR_OUT_OF_MEMORY;
  }
    
  memset((void *) i_tr_client, 0, sizeof(i_trans_object_t));
  i_tr_client->fd = fd_client;
  memcpy((void *) &(i_tr_listener->myAddr), (void *) &peerAddr, sizeof(struct sockaddr_in));
  i_tr_client->port= ntohs(peerAddr.sin_port); 
  
  *tr_client = (trans_object_t *) i_tr_client;

  return CLOS_SUCCESS;
}

/*! \brief Sends data with transport object tr.
 *
 * Sends data with transport object tr.
 *
 * \param tr
 *        The transport object that will be used to send the data.
 *
 * \param data
 *        The data to be sent
 *
 * \param size
 *        The size in bytes of the data buffer
 *
 * \retval CLOS_SUCCESS
 *         Ok
 *
 * \retval ERR_INVALID_PARAMETER
 *         One or more parameters are invalid.
 *
 * \retval ERR_TRANS_PEER_CLOSED
 *         The peer has closed the connection
 *
 * \retval ERR_TRANS_SEND
 *         Error sending data
 *
 */

int TRANS_Send ( trans_object_t tr, void *data, unsigned long size )
{
  size_t bytesSent = 0, sendSent;
  i_trans_object_t *i_tr = NULL;
  unsigned char *i_data;
#if defined(WIN32)
  int wsaErr;
#endif


  if ( ! data )
    return ERR_INVALID_PARAMETER;

  if ( ! tr )
    return ERR_INVALID_PARAMETER;

  i_tr   = ( i_trans_object_t * ) tr;
  i_data = (unsigned char *) data;

  while ( bytesSent < size ) {

#if defined(LINUX)
    sendSent = send(i_tr->fd, (const void *) (i_data+bytesSent), size-bytesSent, MSG_NOSIGNAL);
    if ( sendSent == -1 ) {     
      if ( errno == EPIPE )
		return ERR_TRANS_PEER_CLOSED;
      else
		return ERR_TRANS_SEND;
    }
#elif defined(WIN32)
    sendSent = send(i_tr->fd, (const void *) (i_data+bytesSent), size-bytesSent, 0);
    if ( sendSent == SOCKET_ERROR ) {
      wsaErr = WSAGetLastError();
      if ( wsaErr == WSAECONNRESET )
		return ERR_TRANS_PEER_CLOSED;
      else
		return ERR_TRANS_SEND;
    }
		    
#endif

    bytesSent += sendSent;
  }
  
  
  return CLOS_SUCCESS;

}

/*! \brief Receives (bloking) data at transport object tr.
 *
 * Receives data at transport object tr. Remember that
 * transport objects performs blocking io. If data has
 * not been received in RCV_TIMEOUT seconds a SIGALRM is
 * sent.
 *
 * \param tr
 *        The transport object to receive data from.
 *
 * \param data
 *        The data received.
 *
 * \param size
 *        The bytes to be received.
 *
 * \retval CLOS_SUCCESS
 *         Ok
 *
 * \retval ERR_INVALID_PARAMETER
 *         One or more of the parameters are invalid.
 *
 * \retval ERR_TRANS_RECEIVE
 *         Error receiving data
 *
 * \retval ERR_TRANS_PEER_CLOSED
 *         The peer closed the connection
 *
 * \remarks This function can cause a SIGALRM to be lanched,
 *          so be carefull with this.
 */

int TRANS_Receive ( trans_object_t tr, void *data, unsigned long size)
{
  size_t bytesRecv = 0, n;
  i_trans_object_t *i_tr = NULL;
  unsigned char *i_data;

  if ( ! data ){
    // printf("Saliendo por data\n");
    return ERR_INVALID_PARAMETER;
  }
  if ( ! tr ){
    // printf("Saliendo por tr \n");
    return ERR_INVALID_PARAMETER;
  }
  i_tr   = ( i_trans_object_t * ) tr;
  i_data = (unsigned char *) data;

  while ( bytesRecv < size ) {

#ifdef LINUX
    alarm(RCV_TIMEOUT);
    n = recv(i_tr->fd, i_data + bytesRecv, size-bytesRecv, 0);
    alarm(0);
    if ( n == -1 ) {
      // printf("Error en n == -1 \n");
      return ERR_TRANS_RECEIVE;
    }
    if ( n == 0 ) {
      // printf("Error n == 0 \n");
	return ERR_TRANS_PEER_CLOSED;
    }
#elif defined(WIN32)
	LOG_Msg(5,"Antes de receive");
    n = recv(i_tr->fd, i_data + bytesRecv, size-bytesRecv, 0);
	LOG_Msg(5, "después del receive");
	LOG_Debug(1,"Error %d", GetLastError());
	if ( n == SOCKET_ERROR ) {
		if ( WSAGetLastError() == WSAETIMEDOUT ) {
			LOG_Msg(5, "Timeout");
#ifdef TRANS_WITH_TIMEOUT_HANDLER
			timeout_handler(0);
#endif
		} else {
			return ERR_TRANS_RECEIVE;
		}

	} else if ( n == 0 )
      return ERR_TRANS_PEER_CLOSED;

#endif

    bytesRecv += n;

  }

  return CLOS_SUCCESS;

}

#ifdef WIN32

int TRANS_SetRcvTimeout ( trans_object_t tr, int timeout_ms )
{
	i_trans_object_t *i_tr_listener = NULL;

	if ( ! tr )
		return ERR_INVALID_PARAMETER;

	i_tr_listener = (i_trans_object_t *) tr;

	if ( setsockopt(i_tr_listener->fd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout_ms, sizeof(int)) == SOCKET_ERROR ) {
		LOG_Error(5, "Failed to set SO_RCVTIMEO socket option: %ld", GetLastError());
		return ERR_TRANS_SOCKET;
	}

	return CLOS_SUCCESS;
}



int TRANS_WIN_ChildCreate ( WSAPROTOCOL_INFO pi_parent, trans_object_t *tr )
{
	i_trans_object_t *i_tr_listener = NULL;

	LOG_Msg(1, "1.");    	

	if ( ! tr ){
		LOG_Msg(1, "2.");    		
		return ERR_INVALID_PARAMETER;
	}

	i_tr_listener = (i_trans_object_t *) malloc (sizeof(i_trans_object_t));

	if ( ! i_tr_listener ){
	    LOG_Msg(1, "3."); 
		return ERR_OUT_OF_MEMORY;
	}
	memset(i_tr_listener, 0, sizeof(i_trans_object_t));

	i_tr_listener->fd = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, &pi_parent, 0, 0);
	LOG_Debug(1, "GetLastError() == %ld ", GetLastError());
	if ( i_tr_listener->fd == INVALID_SOCKET ) {
		DWORD err = GetLastError();
		free(i_tr_listener);
		LOG_Msg(1, "4."); 
		return ERR_CLOS;
	}
	LOG_Msg(1, "5."); 
	*tr = i_tr_listener;

	return CLOS_SUCCESS;

}


SOCKET TRANS_GetSocket ( trans_object_t tr)
{
	i_trans_object_t *i_tr = ( i_trans_object_t *) tr;
	return i_tr->fd;
}


unsigned short TRANS_GetPort ( trans_object_t tr )
{
	i_trans_object_t *i_tr = ( i_trans_object_t *) tr;	
	return  i_tr->port;
}

void TRANS_SetRemotePid(trans_object_t tr, int pid){
    i_trans_object_t *i_tr = ( i_trans_object_t *) tr;

	LOG_Debug(1,"Setting remote pid to: %d", pid);
	
	i_tr->rpid= pid;
}

int TRANS_GetRemotePid(trans_object_t tr){
  i_trans_object_t *i_tr = ( i_trans_object_t *) tr;	
  return i_tr->rpid;
}

int TRANS_GetPidFromPort ( trans_object_t tr )
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tokenPriv;
	LUID luidDebug;

	tNTQSI pNTQSI= NULL;
	tNTQO pNTQO= NULL;
	tNTDIOCF pNTDIOCF= NULL;

	HANDLE hProcess= NULL, hProcToken= NULL, hObject= NULL, hEvent2= NULL;
	LPWSTR lpwsName= NULL;
	LPSTR lpszProcess;

	DWORD dwIdx;

	POBJECT_NAME_INFORMATION pObjectInfo= NULL;
	NTSTATUS ntReturn= 0,ntReturn2=0;

	IO_STATUS_BLOCK IoStatusBlock;
	TDI_REQUEST_QUERY_INFORMATION tdiRequestAddress = {{0}, TDI_QUERY_ADDRESS_INFO};
	BYTE tdiAddress[128];

	int port=-1,pid=-1;

	i_trans_object_t *i_tr = ( i_trans_object_t *) tr;
	port= i_tr->port;

	/* Code tunned from netstatp */ 

	/* Enable debug privilege */
	if(OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken) != FALSE) {
		if(LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luidDebug) != FALSE)
		{
			tokenPriv.PrivilegeCount            =  1;
			tokenPriv.Privileges [0].Luid       = luidDebug;
			tokenPriv.Privileges [0].Attributes = SE_PRIVILEGE_ENABLED;
			AdjustTokenPrivileges(hToken,  FALSE, &tokenPriv, sizeof(tokenPriv), NULL, NULL);
		}
	}
	// End enabling debug privilege 

	// Querying for sysinfo and doing the hard work 
	pNTQSI = (tNTQSI)GetProcAddress(GetModuleHandle("NTDLL.DLL"), "NtQuerySystemInformation");

	if(pNTQSI != NULL){

		DWORD dwSize = sizeof(SYSTEM_HANDLE_INFORMATION);
		PSYSTEM_HANDLE_INFORMATION pHandleInfo = (PSYSTEM_HANDLE_INFORMATION) malloc(dwSize);
		NTSTATUS ntReturn = pNTQSI(SystemHandleInformation, pHandleInfo, dwSize, &dwSize);

		if(ntReturn == STATUS_INFO_LENGTH_MISMATCH){
			free (pHandleInfo);
			pHandleInfo = (PSYSTEM_HANDLE_INFORMATION) malloc(dwSize);
			ntReturn = pNTQSI(SystemHandleInformation, pHandleInfo, dwSize, &dwSize);
		}

		if(ntReturn == STATUS_SUCCESS){
			LOG_Debug(1," Found %d Handles. Listing TCP Handles...\n\n", pHandleInfo->uCount);
			
			for(dwIdx = 0; dwIdx < pHandleInfo->uCount; dwIdx++)
			{
				hProcess = OpenProcess(PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
										FALSE, pHandleInfo->Handles[dwIdx].uIdProcess);

				if(hProcess != INVALID_HANDLE_VALUE)
				{
					hObject = NULL;
					if(DuplicateHandle(hProcess, (HANDLE)pHandleInfo->Handles[dwIdx].Handle,
						GetCurrentProcess(), &hObject, STANDARD_RIGHTS_REQUIRED, FALSE, 0) != FALSE)
					{
						pNTQO = (tNTQO)GetProcAddress(GetModuleHandle("NTDLL.DLL"), "NtQueryObject");
						if(pNTQO != NULL){
							DWORD dwSize = sizeof(OBJECT_NAME_INFORMATION);
							pObjectInfo = (POBJECT_NAME_INFORMATION) malloc (dwSize);
							ntReturn = pNTQO(hObject, ObjectNameInformation, pObjectInfo, dwSize, &dwSize);
							if(ntReturn == STATUS_BUFFER_OVERFLOW){
								free (pObjectInfo);
								pObjectInfo = (POBJECT_NAME_INFORMATION) malloc (dwSize);
								ntReturn = pNTQO(hObject, ObjectNameInformation, pObjectInfo, dwSize, &dwSize);
							}

							if((ntReturn >= STATUS_SUCCESS) && (pObjectInfo->Buffer != NULL))
							{
								lpwsName = (LPWSTR) malloc (pObjectInfo->Length + sizeof(WCHAR));
								ZeroMemory(lpwsName,  pObjectInfo->Length + sizeof(WCHAR));
								CopyMemory(lpwsName,  pObjectInfo->Buffer, pObjectInfo->Length);
							}
							free (pObjectInfo);
						}
						// End Getting Object Name 
						if( lpwsName != NULL){
							if(!wcscmp(lpwsName, L"\\Device\\Tcp"))
							{
								lpszProcess = malloc(MAX_PATH);
								ZeroMemory(lpszProcess, MAX_PATH);
								GetModuleFileNameEx(hProcess, NULL, lpszProcess, MAX_PATH);
							
								// Get the port and return the pid 	
								pNTDIOCF = (tNTDIOCF)GetProcAddress(GetModuleHandle("NTDLL.DLL"), "NtDeviceIoControlFile");

								if(pNTDIOCF != NULL){
									hEvent2 = CreateEvent(NULL, TRUE, FALSE, NULL);
									ntReturn2 = pNTDIOCF(hObject, hEvent2, NULL, NULL, &IoStatusBlock, IOCTL_TDI_QUERY_INFORMATION,
										&tdiRequestAddress,  sizeof(tdiRequestAddress), &tdiAddress, sizeof(tdiAddress));
									if(hEvent2) CloseHandle(hEvent2);
									if(ntReturn2 == STATUS_SUCCESS){
										struct in_addr *pAddr = (struct in_addr *)&tdiAddress[14];
										LOG_Debug(1,"Address: %-16s", inet_ntoa(*pAddr));
										LOG_Debug(1,"Port: %d", ntohs(*(PUSHORT)&tdiAddress[12]));
										LOG_Debug(1,"Client port: %d", ntohs(*(PUSHORT)&tdiAddress[12]));
										LOG_Debug(1,"Pid: %d", pHandleInfo->Handles[dwIdx].uIdProcess);
										if (ntohs(*(PUSHORT)&tdiAddress[12]) == port ){
										  pid= pHandleInfo->Handles[dwIdx].uIdProcess;
										  LOG_Debug(1, "GOT client pid= %d", pid);
										}
									}
								}				
								//
								free (lpszProcess);
							}
							free(lpwsName);
							lpwsName= NULL;
						}
						CloseHandle(hObject);
					}
					else{
						// printf("LAST ERROR: GetLastError()= %d\n", GetLastError());
					}
					CloseHandle(hProcess);
				}
			}
		}else{
			LOG_MsgError(1,"Error while trying to allocate memory for System Handle Information.");
		}
		free (pHandleInfo);
	}else{
		LOG_MsgError(1,"Cannot find NtQuerySystemInformation API... Is this system not Win2K and above?");
	}
	////////////

	LOG_Debug(1,"El pid es= %d", pid);
	
	return pid;
}
#endif

