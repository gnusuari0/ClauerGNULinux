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

#ifndef __CLIO_H__
#define __CLIO_H__

#include <blocktypes.h>

#define IO_IS_CLAUER     1
#define IO_IS_NOT_CLAUER 0

#define IO_IS_REMOVABLE     1
#define IO_IS_NOT_REMOVABLE 0

#define MAX_DEVICE_LEN   1024     /* The maximum device name in characters */
#define MAX_SCSI_DEV     128      /* The maximum devices to consider (Linux only) */
#define BLOCK_SIZE       10240
#define IO_MAX_DEVICES   128

#define IO_RDWR    1
#define IO_RD      0

/* Some enumeration types */

#define IO_ENUM_ALL       10  /* Enumerate scsi devices and cryf files on floppy drives also */
#define IO_ENUM_USB       11  /* Enumerate just usb clauer devices */
#define IO_ENUM_FILES     12  /* Enumerate usb devices and cryf files NOT on floppy drives*/
#define IO_ENUM_USB_ALL   13  /* Enumerate just usb devices clauer or not */

#define IO_CHECK_IS_CLAUER 14

#ifdef __cplusplus
extern "C" {
#endif
    
    /*
     * Defines the whence parameter values of the IO_Seek function
     */
    
    typedef enum { 
	IO_SEEK_INFO,
	IO_SEEK_RESERVED,
	IO_SEEK_OBJECT 
    } io_whence_t;
    
    /* The handle is opaque from the outside
     */
    
    typedef void * clauer_handle_t;
    
    /* Function prototypes
     */
    
    int IO_EnumClauers  ( unsigned char *nDevs, char *devices[IO_MAX_DEVICES], int mode );
    
    int IO_Is_Clauer        ( const char *device );
   int IO_Is_Removable      ( const char *device );
    
    /* Returns a checksum that represent the actual state of the system related with the devices */

    int IO_Get_Checksum     ( void );
  
  /* 
     The last parameter could be 0 or IO_CHECK_IS_CLAUER, with the first value, this function 
     does not check if the device you're opening is a clauer, with the second parameter, this 
     check it is done.
  */
  
  int IO_GetBytesSectorByPath (const char * device);
  int IO_Open             ( const char * device, clauer_handle_t *hClauer, int type, int check);
  int IO_Close            ( clauer_handle_t hClauer );
  int IO_Read             ( clauer_handle_t hClauer, unsigned char *block );
  int IO_Write            ( clauer_handle_t hClauer, unsigned char *block );
  int IO_UpdateProperties ( clauer_handle_t hClauer );
  int IO_WriteSector      ( clauer_handle_t hClauer, int offset, unsigned char *sector );
  int IO_Seek             ( clauer_handle_t hClauer, unsigned long long offset, io_whence_t whence );
  int IO_ReadInfoBlock    ( clauer_handle_t hClauer, block_info_t *ib );
  int IO_WriteInfoBlock   ( clauer_handle_t hClauer, block_info_t *ib );
  int IO_obtenerBytesSector(int dispositivo);

  int IO_GetDriveLetter(clauer_handle_t * hClauer, char * logical_unit, int * found);
  
  /* Interface for setting the clauer's info block without reading from the device */
  int IO_SetInfoBlock     ( clauer_handle_t * hClauer, block_info_t *ib ); 
  /* TBE */
  
  int IO_Block ( clauer_handle_t hClauer );
    
  
  /* Return values
   */
  
#define IO_SUCCESS                      0
    
#define ERR_IO                         -1
#define ERR_IO_INVALID_PARAMETER       -2
#define ERR_IO_OUT_OF_MEMORY           -3
#define ERR_IO_CANNOT_OPEN_DEVICE   -1000
#define ERR_IO_NOT_CLAUER           -1001
#define ERR_IO_READ                 -1002
#define ERR_IO_CLOSE                -1003
#define ERR_IO_DISK_FULL            -1004
#define ERR_IO_WRITE                -1005
#define ERR_IO_SEEK                 -1006
#define ERR_IO_OUT_OF_BOUNDS        -1007
#define ERR_IO_NO_PERM              -1008
#define ERR_IO_EOF                  -1009
#define ERR_IO_GET_BYTES_SECTOR     -1010
#define ERR_IO_CANNOT_WRITE_FILE    -1011
#define ERR_IO_END_PARTITION        -1012
#define ERR_IO_INCORRECT_NSECTORS   -1013
#define ERR_IO_IOCTL                -1014
    
#ifdef __cplusplus
}
#endif

#endif
