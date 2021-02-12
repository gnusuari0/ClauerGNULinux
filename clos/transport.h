
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

#ifndef __TRANSPORT_H__
#define __TRANSPORT_H__

#include "log.h"
#include "smem.h"
#include "err.h"

#include <stdlib.h>

#ifdef WIN32
#include <winsock2.h>
#endif

#ifdef MAC 
 #ifndef MSG_NOSIGNAL
 #define MSG_NOSIGNAL 0
 #endif 
#endif 

#define RCV_TIMEOUT 10

#ifdef __cplusplus
extern "C" {
#endif

/******************************/
/*                            */
/* socket <-> pid discovering */
/*         code               */
/*                            */
/*****************************/

#ifdef WIN32

typedef LONG NTSTATUS;

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID Pointer;
    };
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef void (WINAPI * PIO_APC_ROUTINE)(PVOID, PIO_STATUS_BLOCK, DWORD);
typedef LONG TDI_STATUS;
typedef PVOID  CONNECTION_CONTEXT;        ;// connection context

typedef struct _TDI_REQUEST {
    union {
        HANDLE AddressHandle;
        CONNECTION_CONTEXT ConnectionContext;
        HANDLE ControlChannel;
    } Handle;

	PVOID RequestNotifyObject;
    PVOID RequestContext;
    TDI_STATUS TdiStatus;

} TDI_REQUEST, *PTDI_REQUEST;

typedef struct _TDI_CONNECTION_INFORMATION {
    LONG UserDataLength;        // length of user data buffer
    PVOID  UserData;            // pointer to user data buffer
    LONG OptionsLength;         // length of following buffer
    PVOID  Options;         //  pointer to buffer containing options
    LONG RemoteAddressLength;   // length of following buffer
    PVOID RemoteAddress;        // buffer containing the remote address
} TDI_CONNECTION_INFORMATION, *PTDI_CONNECTION_INFORMATION;

typedef struct _TDI_REQUEST_QUERY_INFORMATION {
    TDI_REQUEST Request;
    ULONG  QueryType;                        //  class of information to be queried.
    PTDI_CONNECTION_INFORMATION RequestConnectionInformation;
} TDI_REQUEST_QUERY_INFORMATION, *PTDI_REQUEST_QUERY_INFORMATION;

#define  TDI_QUERY_ADDRESS_INFO           0x00000003
#define  IOCTL_TDI_QUERY_INFORMATION       CTL_CODE(FILE_DEVICE_TRANSPORT, 4, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

typedef VOID *POBJECT;

typedef struct _SYSTEM_HANDLE {
     ULONG       uIdProcess;
     UCHAR       ObjectType;    // OB_TYPE_* (OB_TYPE_TYPE, etc.)
     UCHAR       Flags;         // HANDLE_FLAG_* (HANDLE_FLAG_INHERIT, etc.)
     USHORT         Handle;
     POBJECT         pObject;
     ACCESS_MASK    GrantedAccess;
} SYSTEM_HANDLE, *PSYSTEM_HANDLE;

typedef struct _SYSTEM_HANDLE_INFORMATION {
     ULONG           uCount;
     SYSTEM_HANDLE     Handles[1];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING;

typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;
typedef UNICODE_STRING OBJECT_NAME_INFORMATION;
typedef UNICODE_STRING *POBJECT_NAME_INFORMATION;

#define  SystemHandleInformation           16
#define  ObjectNameInformation           1
#define  STATUS_SUCCESS                  ((NTSTATUS)0x00000000L)
#define  STATUS_INFO_LENGTH_MISMATCH       ((NTSTATUS)0xC0000004L)
#define  STATUS_BUFFER_OVERFLOW           ((NTSTATUS)0x80000005L)
 
/* DLL invoked functions */
typedef NTSTATUS (WINAPI *tNTQSI)(DWORD SystemInformationClass, PVOID SystemInformation,
                                      DWORD SystemInformationLength, PDWORD ReturnLength);

typedef NTSTATUS (WINAPI *tNTQO)(HANDLE ObjectHandle, DWORD ObjectInformationClass, PVOID ObjectInformation,
                                     DWORD Length, PDWORD ResultLength);

typedef NTSTATUS (WINAPI *tNTDIOCF)(HANDLE FileHandle, HANDLE Event, PIO_APC_ROUTINE ApcRoutine, PVOID ApcContext,
                                   PIO_STATUS_BLOCK IoStatusBlock, DWORD IoControlCode,
                                   PVOID InputBuffer, DWORD InputBufferLength,
                                   PVOID OutputBuffer, DWORD OutputBufferLength);

/*
*
* End of socket <-> pid code 
*
*/
#endif


/*
 * The transport object is opaque to the client
 */

typedef void * trans_object_t;

int TRANS_Init ( void );

int TRANS_Create  ( trans_object_t *tr_listener );
int TRANS_Close   ( trans_object_t tr );
int TRANS_Close_Ex   ( trans_object_t tr );
int TRANS_Accept  ( trans_object_t tr_listener, trans_object_t *tr_client );
int TRANS_Send    ( trans_object_t tr, void *data, unsigned long size );
int TRANS_Receive ( trans_object_t tr, void *data, unsigned long size );

#ifdef WIN32
int TRANS_SetRcvTimeout ( trans_object_t tr, int timeout_ms );
int TRANS_WIN_ChildCreate ( WSAPROTOCOL_INFO pi_parent, trans_object_t *tr );
SOCKET TRANS_GetSocket ( trans_object_t tr);
unsigned short TRANS_GetPort ( trans_object_t tr);
void TRANS_SetRemotePid(trans_object_t tr, int pid);
int TRANS_GetRemotePid(trans_object_t tr);
int TRANS_GetPidFromPort ( trans_object_t tr );
#endif

#ifdef __cplusplus
}
#endif

#endif
