
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

#ifndef __ERR_H__
#define __ERR_H__


#define CLOS_SUCCESS    0
#define ERR_NO_ERROR    0

/* Some function error (it will be sended to the client )
 * error > 0 
*/

#define FUNC_ERROR_IS_NOT_REMOVABLE   10 

/* General errors */

#define ERR_CLOS                     -1
#define ERR_OUT_OF_MEMORY            -100
#define ERR_INVALID_PARAMETER        -101
#define ERR_AUTH_INVALID_PASSPHRASE  -103

/* IO Errors */

/*
#define ERR_IO_CANNOT_OPEN_DEVICE   -1000
#define ERR_IO_NOT_CLAUER           -1001
#define ERR_IO_READ                 -1002
#define ERR_IO_CLOSE                -1003
#define ERR_IO_DISK_FULL	    -1004
#define ERR_IO_WRITE		    -1005
#define ERR_IO_SEEK		    -1006
#define ERR_IO_OUT_OF_BOUNDS        -1007
#define ERR_IO_NO_PERM              -1008
#define ERR_IO_EOF                  -1009
*/

/* SMEM Errors */

#define ERR_SMEM_CANNOT_UNLOCK      -2000
#define ERR_SMEM_NO_POOL	    -2001
#define ERR_SMEM_CANNOT_LOCK        -2002

/* SESSION Errors */

#define ERR_SESSION_POOL_EMPTY       -3000
#define ERR_SESSION_NOT_FOUND        -3001

/* TRANSPORT Errors */

#define ERR_TRANS_SOCKET             -4000
#define ERR_TRANS_BAD_IP             -4001
#define ERR_TRANS_BIND               -4002
#define ERR_TRANS_CLOSE              -4003
#define ERR_TRANS_ACCEPT             -4004
#define ERR_TRANS_PEER_CLOSED        -4005
#define ERR_TRANS_SEND               -4006
#define ERR_TRANS_RECEIVE            -4007
#define ERR_TRANS_LISTEN             -4008

/* FUNCTION Errors */
#define ERR_FUNC_SESSION_IS_READONLY      -5000
#define ERR_FUNC_BUFFER_TOO_LARGE         -5001
#define ERR_FUNC_ABORT			  -5002

/* STUB errors */
#define ERR_STUB_UNDEFINED_FUNCTION	-6000

/* FORMAT errors */
#define ERR_FORMAT_INVALID_PARAMETER           -7000
#define ERR_FORMAT_CANNOT_GET_BYTES_SECTOR     -7001
#define ERR_FORMAT_CANNOT_ALLOCATE_MEMORY      -7002
#define ERR_FORMAT_INVALID_PERCENT             -7003
#define ERR_FORMAT_CANNOT_RETRIEVE_SIZE        -7004
#define ERR_FORMAT_CREATEFILE_FAILED           -7005
#define ERR_FORMAT_IOCTL                       -7006 
#define ERR_FORMAT_CLOSE_HANDLE                -7007
#define ERR_FORMAT_GET_GEOMETRY                -7008
#define ERR_FORMAT_OPEN_DEVICE                 -7009 
#define ERR_FORMAT_WRITE_MBR                   -7010
#define ERR_FORMAT_UPDATE_PROPS                -7011
#define ERR_FORMAT_CLOSE	               -7012
#define ERR_FORMAT_GET_DRIVE_LETTER            -7013
#define ERR_FORMAT_DRIVE_LETTER_NOT_FOUND      -7014
#define ERR_FORMAT_FORMATEANDO_DATOS           -7015
#define ERR_FORMAT_NO_PERM                     -7016
#define ERR_FORMAT_CANNOT_OPEN_DEVICE          -7017
#define ERR_FORMAT_ISNOT_CLAUER                -7018
#define ERR_FORMAT_CANNOT_SEED_RNG             -7019
#define ERR_FORMAT_CANNOT_CIPHER               -7020
#define ERR_FORMAT_WRITE_SECTOR                -7021
#define ERR_FORMAT_CANNOT_SEEK                 -7022
#define ERR_FORMAT_NOT_IMPLEMENTED             -7023

#define FORMAT_OK                               0

#endif
