
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

#include "log.h"

#if defined(WIN32)
#include <windows.h>
#include <winioctl.h>
#include <tchar.h>


/* En windows snprintf es _snprintf
 */

#define snprintf _snprintf

/* Tampoco tenemos una función de comparación de cadenas
 * como strncmp()
 */

#define strncasecmp My_strncasecmp
int My_strncasecmp ( const char *s1, const char *s2, int n )
{
  while ( (n > 1) && *s1 && *s2 && (toupper(*s1) == toupper(*s2)) ) {
    ++s1;
    ++s2;
    --n;
  }

  return toupper(*s1) - toupper(*s2);
}

#endif

#include "clio.h"


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef LINUX
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>


extern int errno;
#endif

#define CRYPTO_PART_TYPE         0x69    /* The cryptographic partition type */
#define CRYPTO_PART_TYPE_OFFSET  4       /* The offset to find the type in a partition entry */
#define CRYPTO_PART_OFFSET       0x1ee   /* The offset of the fourth partition entry */
#define CRYPTO_PART_SIZE         16      /* A partition entry size in bytes */


/* 
 * The internal struct of the handle
 */

struct i_clauer_handle {
  char device[MAX_DEVICE_LEN];   /* The device path (eg. "/dev/sda4") */
  int ibInit;                        /* true if the information block has been read */
  block_info_t ib;                   /* The information block of the clauer */

  int isFile;
  
#if defined(LINUX)
  int hDevice;                       /* The handle to the device */
#elif defined(WIN32)
  HANDLE hDevice;
  PPARTITION_INFORMATION partInfo;   /* Info of the fourth partition */

  DWORD pos;     /* The current file pointer position for the device. We use
		  * it because ReadFile() doesn't returns EOF
		  */

  DWORD dwMaxPos;   /* The maximum possible value for pos. This makes
		     * sense only when working with Physical Devices
		     */

#endif

};
typedef struct i_clauer_handle i_clauer_handle_t;
int g_logIni=0;

int readnline(int fp, char * buff, int maxlen);

/*! \brief Determines if a device is a clauer or not.
 *
 * Determines if a device is a clauer or not. It's a clauer if the fourth partition is of type 69.
 *
 * \param device
 *        The device to test.
 *
 * \retval IO_IS_NOT_CLAUER
 *         The device is not a clauer
 *
 * \retval IO_IS_CLAUER
 *         The device is a Clauer
 *
 * \retval ERR_IO_NO_PERM
 *         We are not allowed to open the device.
 *
 * \retval ERR_IO_CANNOT_OPEN_DEVICE
 *         We can't open the device due to another failure.
 *
 * \retval ERR_IO_READ
 *         Read error.
 *
 * \retval ERR_IO_CLOSE
 *         Error closing the device.
 *
 */

int IO_Is_Clauer ( const char *device )
{
#if defined(LINUX)
  int fd;
  unsigned char mbr[512];
#elif defined(WIN32)
  HANDLE hDev, hFindFile;
  WIN32_FIND_DATA findData;
  PDRIVE_LAYOUT_INFORMATION layout= NULL;
  int len, i;
  DWORD aux, outSize;
#endif

  LOG_Msg(1, "Pasamos por IO_Is_Clauer");
  if ( ! device ) 
    return ERR_IO_INVALID_PARAMETER;

#if defined(LINUX)
  fd = open (device, O_RDONLY);
  
  if ( fd == -1 ) {
    
    if ( EACCES == errno ) {
      LOG_Msg(1, "ERROR: No perm");
      
      return ERR_IO_NO_PERM;
    } else {
      LOG_Debug(1, "ERROR: cannot open device errno= %d", errno);
      LOG_Debug(1, "ERROR: cannot open device= %s", device);

      return ERR_IO_CANNOT_OPEN_DEVICE;
    }
  }
  
  if ( read (fd, (void *) mbr, 512 ) == -1 ) {
    close(fd);
    LOG_Msg(1, "ERROR: IO_READ");

    return ERR_IO_READ;
  }
  
  if ( close (fd) == -1 ) {
    LOG_Msg(1, "ERROR: IO_CLOSE");
    
    return ERR_IO_CLOSE;
  }
  
  int len=  strlen(device);
  
  if ( len >= 12 && isdigit(device[len-7]) && 
       isdigit(device[len-6]) && isdigit(device[len-5]) &&
       strncasecmp(device + (len-12),"CRYF_",5) == 0 && strncasecmp(device+len-12+8,".CLA",4) == 0 )
    return IO_IS_CLAUER;
  
  if ( *(mbr + CRYPTO_PART_OFFSET + CRYPTO_PART_TYPE_OFFSET) != CRYPTO_PART_TYPE ) {
    LOG_Msg(1, "ERROR: MBR part 4 not 69");
   
    return IO_IS_NOT_CLAUER;
  }

#elif defined(WIN32)
  
  /* Determinamos si es un fichero lo que tenemos. Si lo
   * es realizamos la comprobación de nombre
   */
  
  LOG_Debug(1,"Analizando device= %s", device);

  hFindFile = FindFirstFile(device, &findData);
  if ( hFindFile != INVALID_HANDLE_VALUE ) {

    /* Es un fichero
     */

    //len = strlen(device);
  
    FindClose(hFindFile);
    len = strlen(device);
    if ( len >= 12 && 
	 isdigit(device[len-7]) && 
	 isdigit(device[len-6]) && 
	 isdigit(device[len-5]) &&
	 (strncasecmp(device + len - 12,"CRYF_",5) == 0) && 
	 (strncasecmp(device + len - 12 + 8,".CLA",4) == 0 ) )
      return IO_IS_CLAUER;
    else
      return IO_IS_NOT_CLAUER;

  } else {
  
    /* No es un fichero
     */
    
    hDev = CreateFile(device, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_NO_BUFFERING, 0);
    if ( hDev == INVALID_HANDLE_VALUE ) {
      if ( GetLastError() == ERROR_ACCESS_DENIED ) {
	return ERR_IO_NO_PERM;
      } else {
	return ERR_IO_CANNOT_OPEN_DEVICE;
      }
    }
    
    layout = ( PDRIVE_LAYOUT_INFORMATION ) malloc ( sizeof(DRIVE_LAYOUT_INFORMATION) + 4*sizeof(PARTITION_INFORMATION) );
    memset(layout, 0, sizeof(DRIVE_LAYOUT_INFORMATION) + 4*sizeof(PARTITION_INFORMATION) );

    /* 
       LOG_Debug(1,"Ap1= %d", layout->PartitionEntry[0].PartitionType );
       LOG_Debug(1,"Ap2= %d", layout->PartitionEntry[1].PartitionType );
       LOG_Debug(1,"AP3= %d", layout->PartitionEntry[2].PartitionType );
       LOG_Debug(1,"AP4= %d", layout->PartitionEntry[3].PartitionType );
    */

    if ( ! layout ) { 
      CloseHandle(hDev);
      return ERR_IO_OUT_OF_MEMORY;
    }
    if ( ! DeviceIoControl(hDev, IOCTL_DISK_GET_DRIVE_LAYOUT, NULL, 0, layout, sizeof(DRIVE_LAYOUT_INFORMATION) + 4*sizeof(PARTITION_INFORMATION), &outSize, NULL) ) {      
      CloseHandle(hDev);
      return ERR_IO;
    }
    
    /*
      LOG_Debug(1,"outSize= %d", outSize );
      LOG_Debug(1,"p1= %d", layout->PartitionEntry[0].PartitionType );
      LOG_Debug(1,"p2= %d", layout->PartitionEntry[1].PartitionType );
      LOG_Debug(1,"P3= %d", layout->PartitionEntry[2].PartitionType );
      LOG_Debug(1,"P4= %d", layout->PartitionEntry[3].PartitionType );
    */

    CloseHandle(hDev);
       
    /*Este es más restrictivo, probar:
     *if ( layout->PartitionCount != 4 || 
     (layout->PartitionEntry[0].PartitionType != 4 && layout->PartitionEntry[0].PartitionType != 0xb) ||
     layout->PartitionEntry[3].PartitionType != 0x69){
    */
    if ( layout->PartitionEntry[3].PartitionType != 0x69 ) {
      LOG_Msg(1,"No encontrado 0x69 en el layout");
      free(layout);
      return IO_IS_NOT_CLAUER;
    }
    LOG_Msg(1,"Encontrado 0x69 en el layout");
    free(layout);
  }  /* es fichero */

#endif
  
    
  LOG_Msg(1, "IO_IS_CLAUER");
  return IO_IS_CLAUER;
  
}




#ifdef LINUX
int IO_Get_Checksum( void ){

  int aux;
    
  char * idx1, * idx2;
  int fp, ret, checksum=0, sysfs=0, i;
  char line[1024], dname[128];
  DIR * dir; 
  struct dirent * dirent;

  srand(time(NULL));
  aux =  rand();
	
    
  /* Looking for a mounted sysfs */ 
  fp= open("/etc/mtab",O_RDONLY);
  if (fp != -1) {
    ret= readnline(fp,line,1024); 
    while ( ret != 0 ) {      if (ret == -1) {
	break;
      }
      line[ret]= '\0';
      for ( i=0 ; i<ret ; i++ ){
	checksum += line[i];
      }
      idx1= index(line,' '); idx1= index(++idx1,' ');
      idx1++;
      if (idx1 != NULL){
	idx2= index(idx1,' ');
	if (idx2 != NULL){
	  idx2[0]='\0';
	  if ( strncmp(idx1,"sysfs",5) == 0 ){
	    sysfs=1;
	  }
	}
      }	
      ret= readnline(fp,line,1024); 
    }
    close(fp);
    /* Here we know /proc is mounted and sysfs tell us if its present*/

    /* First sysfs /sys/block/ */
    if ( sysfs ){	
      dir = opendir("/sys/block/");
      if ( dir ) {
	dirent= readdir(dir);
	while ( dirent != NULL ){
	  for(i=0; i<strlen(dirent->d_name); i++){
	    checksum += dirent->d_name[i];
	  }
	  dirent= readdir(dir);
	}
	closedir(dir);
      }
    }


    /* Secondly /proc/scsi/scsi */
    fp= open("/proc/scsi/scsi",O_RDONLY);
    if (fp != -1) {
      ret= readnline(fp,line,1024); 
      while ( ret != 0 ) {
	if (ret == -1) {
	  break;
	}
	for (i=0 ; i<ret ; i++){
	  checksum += line[i];
	}
	ret= readnline(fp,line,1024); 
      }
      close(fp);
    }
	
    /* Third /proc/scsi/usb-storage/ */
    dir = opendir("/proc/scsi/usb-storage/");
    if ( dir ) {
      dirent= readdir(dir);
      while ( dirent != NULL ){
	for(i=0; i<strlen(dirent->d_name); i++){
	  checksum += dirent->d_name[i];
	}
	dirent= readdir(dir);
      }
      closedir(dir);
    }

    /* Finally /proc/scsi/usb-storage-[0-32] */
    for (i=0 ; i<32 ; i++){
      snprintf(dname, 128, "/proc/scsi/usb-storage-%d/",i);
      dir = opendir(dname);
      if ( dir ) {
	checksum += i;
	closedir(dir);
      }
    }
  }
  else{ 
    /* If /proc does not exists, there nothig to do, the best thing is return
     * a random Checksum to make application refresh their status  
     */ 
    checksum=aux;
	
  }

  return checksum;
}
#endif



/*! \brief Opens a Clauer.
 *
 * Opens a Clauer device.
 *
 * \param device
 *        The device to be opened
 *
 * \param hClauer
 *        The handle of the Clauer returned. This parameter will be
 *        used in the other io functions.
 *
 * \param type
 *        If we want RD/WR access or just RD access: IO_RDWR or IO_RD
 *
 * \retval IO_SUCCESS
 *         Ok
 *
 * \retval ERR_IO_INVALID_PARAMETER
 *	   One or both of the parameters are invalid (NULL)
 *
 * \retval ERR_IO_NOT_CLAUER
 *         The device isn't a Clauer
 *
 * \retval ERR_IO_OUT_OF_MEMORY
 *         Impossible to allocate memory
 *
 * \retval ERR_IO_NO_PERM
 *         We have not privileges enough to open de device.
 *
 * \retval ERR_IO_CANNOT_OPEN_DEVICE
 *         It cannot open the device.
 */

int IO_Open  ( const char *device, clauer_handle_t *hClauer, int type, int check )
{
  int ret, opened = 0, fd;
  i_clauer_handle_t *i_hClauer = NULL;
  int len;
  int lpBRetDIOC;

#if defined(WIN32)
  PDRIVE_LAYOUT_INFORMATION layout = NULL;
  DWORD outSize,i;
  char unidad[4];
#endif  

#ifndef NO_LOG_INIT
  if (!g_logIni){ LOG_Ini(2, 10); g_logIni=1;}
#endif 

  LOG_Msg(2,"Abriendo dispositivo");

  if ( ! device ){
    LOG_MsgError(2,"Parámetro dispositivo incorrecto");
    return ERR_IO_INVALID_PARAMETER;
  }

  if ( ! hClauer ){
    LOG_MsgError(2,"Parámetro hClauer incorrecto");
    return ERR_IO_INVALID_PARAMETER;
  }

  if ( (type != IO_RD) && (type != IO_RDWR) ){
    LOG_MsgError(2,"Parámetro type incorrecto");
    return ERR_IO_INVALID_PARAMETER;
  }

  *hClauer = NULL;

  LOG_Msg(2,"2");

  if ( check == IO_CHECK_IS_CLAUER ){
    ret = IO_Is_Clauer(device);
      
    switch ( ret ) {
    case IO_IS_CLAUER:
      LOG_Msg(2,"És un clauer");
      break;
	  
    case IO_IS_NOT_CLAUER:
      ret = ERR_IO_NOT_CLAUER;
      LOG_Msg(2,"No és un clauer");
      goto err_IO_Open;
	  
    default:
      LOG_Msg(2,"Saliendo por default");
      goto err_IO_Open;    
    }
  }
  
  /* Build the handle */
    LOG_Msg(2,"3");
  i_hClauer = ( i_clauer_handle_t * ) malloc ( sizeof(i_clauer_handle_t) );
  
  if ( ! i_hClauer ) {
    ret = ERR_IO_OUT_OF_MEMORY;
    LOG_MsgError(2,"Sin memoria");
    goto err_IO_Open;
  }
  
  LOG_Msg(2,"4");
  memset(i_hClauer, 0, sizeof(i_clauer_handle_t));

  if ( strstr(device,"CRYF_") != NULL || strstr(device,".cla") != NULL )
    strncpy(i_hClauer->device, device, MAX_DEVICE_LEN);
  else {
#if defined(LINUX)
#if defined(MAC)
    snprintf(i_hClauer->device, MAX_DEVICE_LEN, "%ss4", device); 
#else
    snprintf(i_hClauer->device, MAX_DEVICE_LEN, "%s4", device);
#endif 
#elif defined(WIN32)
    snprintf(i_hClauer->device, MAX_DEVICE_LEN, "%s", device);
#endif
  }
  
  /* open the device */

  LOG_Msg(2,"5");
  if ( type == IO_RDWR ){
#if defined(LINUX)
    i_hClauer->hDevice = open(i_hClauer->device, O_RDWR);
#elif defined(WIN32)

    /* En el caso de que el CRYF esté en un CDROM, la operación de apertura del
     * fichero debe ser de solo lectura
     */

    strncpy(unidad, i_hClauer->device, 3);
    unidad[3] = 0;
  
    LOG_Msg(2,"6");
    
    if ( GetDriveType(unidad) == DRIVE_CDROM ) {
      i_hClauer->hDevice = CreateFile( i_hClauer->device, GENERIC_READ, 
				       FILE_SHARE_READ, NULL, 
				       OPEN_EXISTING, 
				       FILE_ATTRIBUTE_NORMAL|FILE_FLAG_NO_BUFFERING, 0);
    } 
    else {
	  LOG_Msg(2, i_hClauer->device);
      i_hClauer->hDevice = CreateFile( i_hClauer->device, 
				       GENERIC_READ | GENERIC_WRITE, 
				       FILE_SHARE_READ | FILE_SHARE_WRITE, 
				       NULL, OPEN_EXISTING, 
				       FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH|FILE_FLAG_NO_BUFFERING, 0);
	}
    LOG_Msg(2,"7");
#endif
  } 
  else{
#if defined(LINUX)
    i_hClauer->hDevice = open(i_hClauer->device, O_RDONLY);
#elif defined(WIN32)
    i_hClauer->hDevice = CreateFile(i_hClauer->device, GENERIC_READ, 
				    FILE_SHARE_READ|FILE_SHARE_WRITE, 
				    NULL, OPEN_EXISTING, 
				    FILE_ATTRIBUTE_NORMAL|FILE_FLAG_NO_BUFFERING, 0);
#endif
  }
LOG_Msg(2,"8");
#if defined(LINUX)
  if ( i_hClauer->hDevice == -1 ) {
    i_hClauer->hDevice = open(i_hClauer->device, O_RDONLY);
  }
#endif
  
#if defined(LINUX)
  if (  i_hClauer->hDevice == -1 ) {
    if ( EACCES == errno ) {
      ret = ERR_IO_NO_PERM;
      goto err_IO_Open;
    } else {
      ret = ERR_IO_CANNOT_OPEN_DEVICE;
      goto err_IO_Open;
    }
  }
#elif defined(WIN32)
  if (  i_hClauer->hDevice == INVALID_HANDLE_VALUE ) {
    if ( GetLastError() == ERROR_ACCESS_DENIED ) {
      LOG_MsgError(2,"Sin permiso de acceso");
      ret = ERR_IO_NO_PERM;
      goto err_IO_Open;
    } else {
      LOG_Debug(2,"No se puede hacer open LastError= %d", GetLastError());
      LOG_MsgError(2,"Error al abrir el dispositivo");
      ret = ERR_IO_CANNOT_OPEN_DEVICE;
      goto err_IO_Open;
    }
  }
#endif
  LOG_Msg(2,"9");  
  opened = 1;
  
  /* Determinamos si es un fichero clauer o no.
   * Esto es de interés particular para windows. En linux no hacemos
   * diferenciación entre fichero y dispositivo ya que abrimos
   * directamente la cuarta partición y no el dispositivo
   * físico
   */
  LOG_Msg(2,"10");
  len = strlen(device);
  if ( len >= 12 && isdigit(device[len-7]) && 
       isdigit(device[len-6]) && isdigit(device[len-5]) &&
       strncasecmp(device + (len-12),"CRYF_",5) == 0 && 
       strncasecmp(device+len-12+8,".CLA",4) == 0 )
    i_hClauer->isFile = 1;
  else
    i_hClauer->isFile = 0;
  
  /* Return successfully */
  
  *hClauer = (clauer_handle_t) i_hClauer;
  
  LOG_Msg(2,"11");

  return IO_SUCCESS;
  
 err_IO_Open:

#if defined(LINUX)
  if ( opened )
    close(fd);
#elif defined(WIN32)
  if ( opened )
    CloseHandle(i_hClauer->hDevice);

  if ( layout )
    free(layout);
#endif

  if ( i_hClauer ) {
#if defined(WIN32)
    if ( i_hClauer->partInfo )
      free(i_hClauer->partInfo);
#endif
    free(i_hClauer);
    *hClauer = NULL;
  }
  
  return ret;
}

/*! \brief Closes a previously opened Clauer.
 * 
 * Closes a previously opened Clauer
 *
 * \param hClauer
 *        Handle to the Clauer.
 *
 * \retval IO_SUCCESS
 *         Ok
 *
 * \retval ERR_IO_INVALID_PARAMETER
 *         The handle is invalid
 *
 * \retval ERR_IO_CLOSE
 *         Error closing the device
 *
 */
int IO_Close ( clauer_handle_t hClauer )
{
  i_clauer_handle_t *i_hClauer = NULL;

  if ( ! hClauer )
    return ERR_IO_INVALID_PARAMETER;

  i_hClauer = ( i_clauer_handle_t *) hClauer;
 	
#if defined(LINUX)
  if ( close(i_hClauer->hDevice) == -1 )
    return ERR_IO_CLOSE;
#elif defined(WIN32)
  if ( ! CloseHandle(i_hClauer->hDevice) )
    return ERR_IO_CLOSE;
#endif
 
  free(i_hClauer);
  i_hClauer= NULL;

  return IO_SUCCESS;
}



#ifdef WIN32
int IO_ReadPartInfo ( clauer_handle_t hClauer )
{
  i_clauer_handle_t *i_hClauer = NULL;
  PDRIVE_LAYOUT_INFORMATION layout = NULL;
  int ret;
  DWORD outSize;

  /* Obtenemos información sobre la cuarta partición. Realmente nos interesa el offset
   */

  i_hClauer= (i_clauer_handle_t * ) hClauer;
  i_hClauer->partInfo = (PPARTITION_INFORMATION) malloc ( sizeof(PARTITION_INFORMATION) );
  
  if ( ! i_hClauer->partInfo ) {
    ret = ERR_IO_OUT_OF_MEMORY;
    goto err_IO_ReadPartInfo;
  }

  ZeroMemory(i_hClauer->partInfo, sizeof(PARTITION_INFORMATION));

  if ( i_hClauer->isFile ) {

    /* Si es un fichero rellenamos la estructura partition info
     * a mano
     */
    LARGE_INTEGER aux;
    ZeroMemory(&aux, sizeof aux);

    i_hClauer->partInfo->StartingOffset = aux;
    if ( ! GetFileSizeEx(i_hClauer->hDevice, &(i_hClauer->partInfo->PartitionLength)) ) {
      ret = ERR_IO;
      goto err_IO_ReadPartInfo;
    }

  } else {
    
    layout = ( PDRIVE_LAYOUT_INFORMATION ) malloc ( sizeof(DRIVE_LAYOUT_INFORMATION) + 4*sizeof(PARTITION_INFORMATION) );
    if ( ! layout ) { 
      ret = ERR_IO_OUT_OF_MEMORY;
      goto err_IO_ReadPartInfo;
    }
    
    ZeroMemory(layout, sizeof(DRIVE_LAYOUT_INFORMATION));
	       
    if ( ! DeviceIoControl(i_hClauer->hDevice, IOCTL_DISK_GET_DRIVE_LAYOUT,
			   NULL, 0, layout, sizeof(DRIVE_LAYOUT_INFORMATION) + 4 * sizeof(PARTITION_INFORMATION),
			   &outSize, NULL) ) {
      ret = ERR_IO;
      goto err_IO_ReadPartInfo;
    }
    
    memcpy(i_hClauer->partInfo, &(layout->PartitionEntry[3]), sizeof(PARTITION_INFORMATION));
    
    free(layout);
    layout = NULL;
  }

  return IO_SUCCESS;

 err_IO_ReadPartInfo:

  if ( layout ) 
    free(layout);

  if ( i_hClauer->partInfo ) {
    free(i_hClauer->partInfo);
    i_hClauer->partInfo = NULL;
  }


  return ret;
}
#endif






/*! \brief Reads a block from the device pointer's position.
 *
 * Reads a block from the device pointer's position. To change this
 * pointer use IO_Seek() function.
 *
 * \param hClauer
 *        A successfully opened clauer' handle.
 * 
 * \param block
 *        The buffer where the block will be put
 *
 * \retval ERR_IO_INVALID_PARAMETER
 *         One or both of the parameters aren't valid
 *
 * \param ERR_IO_READ
 *        Read error.
 *
 * \param IO_SUCCESS
 *        Ok
 */

int IO_Read  ( clauer_handle_t hClauer, unsigned char *block )
{
  i_clauer_handle_t *i_hClauer = NULL;

#if defined(WIN32)
  DWORD bytesRead;
  int ret;
#endif

  if ( ! hClauer )
    return ERR_IO_INVALID_PARAMETER;

  if ( ! block )
    return ERR_IO_INVALID_PARAMETER;

  i_hClauer = ( i_clauer_handle_t *) hClauer;

#ifdef WIN32
  if ( ! i_hClauer->partInfo ) {
    ret = IO_ReadPartInfo(hClauer);
    if ( ret != IO_SUCCESS ){
      LOG_Msg(2,"Fallo IO_ReadPartInfo");     
      return ret;
    }
  }
#endif

LOG_Msg(2,"Paso");     

#if defined(LINUX)
  if ( read(i_hClauer->hDevice, (void *) block, BLOCK_SIZE) == -1 )
    return ERR_IO_READ;
#elif defined(WIN32)

  LOG_Msg(2,"1");     
  if ( (i_hClauer->pos + BLOCK_SIZE) > i_hClauer->partInfo->PartitionLength.LowPart )
    return ERR_IO_EOF;

  LOG_Msg(2,"2");     
  if ( ! ReadFile(i_hClauer->hDevice, (void *) block, BLOCK_SIZE, &bytesRead, NULL) ){
    LOG_Debug(1,"Error= %d", GetLastError());
    return ERR_IO_READ;
  }

  LOG_Msg(2,"3");     

  /* ReadFile() returns bytesRead == 0 and TRUE if we're beyond EOF.
   * We return ERR_IO_EOF if the bytes read are less than BLOCK_SIZE
   * to indicate that we're trying to read more than we can 
   * (ie. we've read the last block or the crypto part size isn't multiple
   * of the block size)
   *
   * On the other hand. This behaviour isn't true when working with
   * physical devices, so we test ourselves
   */

  if ( i_hClauer->isFile ) {
    if ( bytesRead < BLOCK_SIZE )
      return ERR_IO_EOF;
  } else
    i_hClauer->pos += BLOCK_SIZE;

  LOG_Msg(2,"4");     
#endif


  return IO_SUCCESS;
}



/* UPDATE POPERTIES FUNCTION */

/*! \brief Sends an ioctl in order to update disk properties.
 *
 * Sends an ioctl in order to update disk properties.
 *
 * \param hClauer
 *        The handle to the clauer.
 *
 * \retval IO_SUCCESS
 *         Ok
 *
 * \retval ERR_
 *         One or both of the parameters are invalid.
 *
 * \retval ERR_IO_DISK_FULL
 *         The device is full
 *
 * \retval ERR_IO_WRITE
 *         Error writing to the device
 */

int IO_UpdateProperties ( clauer_handle_t hClauer )
{
#ifdef WIN32 
  i_clauer_handle_t *i_hClauer = NULL;
  int error, nbytes;


#ifndef NO_LOG_INIT
  if (!g_logIni){ LOG_Ini(2, 10); g_logIni=1;}
#endif 

  LOG_Msg(1, "Entrando en Update props ");
  if ( ! hClauer ){
    LOG_MsgError(1,"Invalid Param");
    return ERR_IO_INVALID_PARAMETER;
  }

  i_hClauer = ( i_clauer_handle_t *) hClauer;
  
  
  error = DeviceIoControl(  i_hClauer->hDevice, IOCTL_DISK_UPDATE_PROPERTIES, NULL, 0, NULL, 0, &nbytes, NULL );	
  if ( error == 0 ) {
    LOG_Error(1,"Error launching IO_CTL=%d", GetLastError());
    if ( GetLastError() == ERROR_NOT_SUPPORTED ) return ERR_IO_IOCTL;
  }
  
  LOG_Msg(1, "Saliendo OK! ");
#endif
  return IO_SUCCESS;
}
/* END UPDATEPROPS */




/* WRITE SECTOR FUNCTION */

/*! \brief Writes a sector skipping offset sectors.
 *
 * Writes a sector skipping offset sectors.
 *
 * \param hClauer
 *        The handle to the clauer.
 *
 * \param sector
 *        The sector to be written in the device.
 *
 * \retval IO_SUCCESS
 *         Ok
 *
 * \retval ERR_INVALID_PARAMTER
 *         One or both of the parameters are invalid.
 *
 * \retval ERR_IO_DISK_FULL
 *         The device is full
 *
 * \retval ERR_IO_WRITE
 *         Error writing to the device
 */

int IO_WriteSector ( clauer_handle_t hClauer, int offset, unsigned char *sector )
{
  i_clauer_handle_t *i_hClauer = NULL;

#if defined(LINUX)
  ssize_t s;
#elif defined(WIN32)
  DWORD bytesWritten;
  int ret, bytesSector;
  long long nbytes= 0, lloffset= 0, llbs= 0;
  long despl_low, despl_high, result;
#endif


#ifndef NO_LOG_INIT
  if (!g_logIni){ LOG_Ini(2, 10); g_logIni=1;}
#endif 
    
  if ( ! hClauer )
    return ERR_IO_INVALID_PARAMETER;

  i_hClauer = ( i_clauer_handle_t *) hClauer;

#ifdef WIN32
  bytesSector= IO_GetBytesSectorByPath(i_hClauer->device);
  if ( bytesSector < 0 )
    return ERR_IO_GET_BYTES_SECTOR;

  /*
    if ( ! i_hClauer->partInfo ) {
    ret = IO_ReadPartInfo(hClauer);
    if ( ret != IO_SUCCESS )
    return ret;
    }
  */
#endif


#if defined(LINUX)

  s = write(i_hClauer->hDevice, (const void *) sector, BLOCK_SIZE);

  if ( s == -1 ) {
    if ( errno == ENOSPC )
      return ERR_IO_DISK_FULL;
    else
      return ERR_IO_WRITE;
  }

#elif defined(WIN32)

  if ( i_hClauer->isFile ) {
    return ERR_IO_CANNOT_WRITE_FILE;
  }

  /* We skip as many sector as offset indicates */
  
  if ( offset >= 0 ) {

    LOG_Debug(1,"Jumping to offset= %ld", offset);

    lloffset= offset;
    llbs= bytesSector;

    nbytes = lloffset * llbs;

    LOG_Debug(1,"Jumping to nbytes= %lld", nbytes);
    
    if ( nbytes >= 2147483648 ) {

      LOG_Msg(1, "GOT into >=");

      despl_low = (LONG) nbytes & 0xFFFFFFFF;
      nbytes = (nbytes & 0xFFFFFFFF00000000) >> 32;
      despl_high = (LONG) nbytes;

      result = SetFilePointer( i_hClauer->hDevice, despl_low, &despl_high, FILE_BEGIN );

      if ( (result == INVALID_SET_FILE_POINTER) && (GetLastError != NO_ERROR) || (result == ERROR_NEGATIVE_SEEK) ) {
	LOG_Error(1, "ERROR en WriteSector = %d", GetLastError());
	return ERR_IO_END_PARTITION;
      }
    }
    else {

      nbytes = (LONG) nbytes;
      result = SetFilePointer( i_hClauer->hDevice, nbytes, NULL, FILE_BEGIN );

      if ( ( result == INVALID_SET_FILE_POINTER ) || ( result == ERROR_NEGATIVE_SEEK ) ) {
	return  ERR_IO_END_PARTITION;
      }

    }
  }
  else if ( offset < 0 ) return ERR_IO_INCORRECT_NSECTORS;
  /****/

  if ( ! WriteFile(i_hClauer->hDevice, (LPCVOID) sector, bytesSector, &bytesWritten, NULL) ) {
    if ( GetLastError() == ERROR_HANDLE_DISK_FULL )
      return ERR_IO_DISK_FULL;
	else{
	    LOG_Error(1, "ERROR en WriteFile = %d", GetLastError());
        return ERR_IO_WRITE;
	}
  }

  if ( ! i_hClauer->isFile )
    i_hClauer->pos += BLOCK_SIZE;

#endif

  return IO_SUCCESS;
}
/* END WRITE SECTOR */






/*! \brief Writes a block starting from the device pointer's position.
 *
 * Writes a block starting from the device pointer's position.
 *
 * \param hClauer
 *        The handle to the clauer.
 *
 * \param block
 *        The block to be written in the device.
 *
 * \retval IO_SUCCESS
 *         Ok
 *
 * \retval ERR_INVALID_PARAMTER
 *         One or both of the parameters are invalid.
 *
 * \retval ERR_IO_DISK_FULL
 *         The device is full
 *
 * \retval ERR_IO_WRITE
 *         Error writing to the device
 */

int IO_Write ( clauer_handle_t hClauer, unsigned char *block )
{
  i_clauer_handle_t *i_hClauer = NULL;

#if defined(LINUX)
  ssize_t s;
#elif defined(WIN32)
  DWORD bytesWritten;
  int ret;
#endif

  if ( ! hClauer )
    return ERR_IO_INVALID_PARAMETER;

  i_hClauer = ( i_clauer_handle_t *) hClauer;

#ifdef WIN32
  if ( ! i_hClauer->partInfo ) {
    ret = IO_ReadPartInfo(hClauer);
    if ( ret != IO_SUCCESS )
      return ret;
  }
#endif




#if defined(LINUX)

  s = write(i_hClauer->hDevice, (const void *) block, BLOCK_SIZE);

  if ( s == -1 ) {
    if ( errno == ENOSPC )
      return ERR_IO_DISK_FULL;
    else
      return ERR_IO_WRITE;
  }

#elif defined(WIN32)

  if ( i_hClauer->isFile ) {
    if ( (i_hClauer->pos + BLOCK_SIZE) > i_hClauer->partInfo->PartitionLength.LowPart ) {
      return ERR_IO_EOF;
    }
  }

  if ( ! WriteFile(i_hClauer->hDevice, (LPCVOID) block, BLOCK_SIZE, &bytesWritten, NULL) ) {
    if ( GetLastError() == ERROR_HANDLE_DISK_FULL )
      return ERR_IO_DISK_FULL;
    else
      return ERR_IO_WRITE;
  }

  if ( ! i_hClauer->isFile )
    i_hClauer->pos += BLOCK_SIZE;

#endif

  return IO_SUCCESS;
}

/*! \brief Reads the information block.
 *
 * Reads the information block.
 *
 * \param hClauer
 *        A successfully opened clauer's handle.
 * 
 * \param block
 *        The buffer where the information block will be returned.
 *
 * \retval IO_SUCCESS
 *         Ok
 *
 * \retval ERR_IO_INVALID_PARAMETER
 *         One or both of the parameters are incorrect.
 *
 * \retval ERR_IO_SEEK
 *         Error positioning the device's pointer.
 *
 * \retval ERR_IO_READ
 *         Read error
 *
 */

int IO_ReadInfoBlock ( clauer_handle_t hClauer, block_info_t *block )
{
  i_clauer_handle_t *i_hClauer;
 

#ifndef NO_LOG_INIT
  if (!g_logIni){ LOG_Ini(2, 10); g_logIni=1;}
#endif   

  
  if ( ! hClauer ) 
    return ERR_IO_INVALID_PARAMETER;
  
  if ( ! block ) 
    return ERR_IO_INVALID_PARAMETER;
  
  i_hClauer = ( i_clauer_handle_t * ) hClauer;

  if ( IO_Seek(hClauer, 0, IO_SEEK_INFO) != IO_SUCCESS ) {
    LOG_Msg(2,"Fallo en el IO_SEEK");
    return ERR_IO_SEEK;
  }
  
  if ( IO_Read(hClauer, (unsigned char *) block) != IO_SUCCESS ) {
    LOG_Msg(2,"Fallo en el IO_READ");
    return ERR_IO_SEEK;
  }

 
  return IO_SUCCESS;
}

/*! \brief Puts the device's pointer to the position specified.
 *
 * Moves the device's pointer to the position specified.
 *
 * \param hClauer
 *        A successfully opened clauer handle.
 *
 * \param offset
 *        The offset starting from the zone specified by whence.
 *
 * \param whence
 *        Specifies the zone from wich to consider offset. It can
 *        take the values defined in the io_whence_t type:
 *
 *             -# IO_SEEK_INFO. Information zone
 *             -# IO_SEEK_RESERVED. Reserved zone
 *             -# IO_SEEK_OBJECT. The object zone
 *
 * \retval IO_SUCCESS
 *         Ok
 *
 * \retval ERR_IO_INVALID_PARAMETER
 *         One or more of the parameters are invalid.
 * 
 * \retval ERR_IO_OUT_OF_BOUNDS
 *         The offset parameter points out of the zone.
 *
 * \retval ERR_IO_SEEK
 *         Error positioning the device's pointer
 *
 */

int IO_Seek  ( clauer_handle_t hClauer, unsigned long long offset, io_whence_t whence )
{
  i_clauer_handle_t *i_hClauer;
  int ret;

#ifdef  WIN32
  LARGE_INTEGER lint;
  long long ll1, ll2;
#endif 
 
  LOG_Msg(2,"Entramos en IO_SEEK");
  // LOG_Debug(2,"size of long long =%d", sizeof llaux);
  if ( ! hClauer )
    return ERR_IO_INVALID_PARAMETER;
  
  i_hClauer = (i_clauer_handle_t *) hClauer;

#ifdef WIN32
  LOG_Msg(2,"antes de isFile");
  if ( ! i_hClauer->isFile ) {
    LOG_Msg(2,"des de isFile");
    if ( ! i_hClauer->partInfo ) {
      LOG_Msg(2,"des de partInfo");
      ret = IO_ReadPartInfo(hClauer);
      if ( ret != IO_SUCCESS ) {
	LOG_Debug(2,"ret=%d",ret);
	return ret;
      }
    }
  }
#endif

  /* Si no se ha leído todavía el bloque de información,
   * lo leemos.
   */

  if ( ! i_hClauer->ibInit ) {

#if defined(LINUX)    
    if ( lseek(i_hClauer->hDevice, 0, SEEK_SET) == -1 ) 
      return ERR_IO_SEEK;
#elif defined(WIN32)   
    if ( i_hClauer->isFile ) {
      if ( SetFilePointer(i_hClauer->hDevice, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER )
	return ERR_IO_SEEK;
    } else {
      LOG_Debug(2,"Antes SetFilePointer LowPart= %lld", i_hClauer->partInfo->StartingOffset);
      if ( SetFilePointer(i_hClauer->hDevice, i_hClauer->partInfo->StartingOffset.LowPart,
			  &(i_hClauer->partInfo->StartingOffset.HighPart),
			  FILE_BEGIN) == INVALID_SET_FILE_POINTER ){
	LOG_Msg(2,"ERROR SetFilePointer");
	LOG_Debug(2,"ERROR= %d ", GetLastError());
	return ERR_IO_SEEK;
      }
      i_hClauer->pos = 0;
    }
#endif
    
    if ( IO_Read(hClauer, (unsigned char *) &(i_hClauer->ib)) != IO_SUCCESS ) {
      LOG_Msg(2,"ERROR IO_Read");
      return ERR_IO_SEEK;
    }
    i_hClauer->ibInit = 1;
  }
  
  switch ( whence ) {
    
  case IO_SEEK_INFO:

#ifndef NO_LOG_INIT
  if (!g_logIni){ LOG_Ini(2, 10); g_logIni=1;}
#endif 
    LOG_Msg(2,"Entramos en SEEK_INFO");
    if ( offset >= 1 ){
      LOG_Debug(2," offset= %d", offset);
      return ERR_IO_OUT_OF_BOUNDS;
    }

#if defined(LINUX)    
    if ( lseek(i_hClauer->hDevice, 0, SEEK_SET) == -1 ) 
      return ERR_IO_SEEK;
#elif defined(WIN32)   
    if ( i_hClauer->isFile ) {
      if ( SetFilePointer(i_hClauer->hDevice, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER )
	return ERR_IO_SEEK;
    } else {
      LOG_Debug(2," i_hClauer->partInfo->StartingOffset.LowPart= %d", i_hClauer->partInfo->StartingOffset.LowPart);
      if ( SetFilePointer(i_hClauer->hDevice, i_hClauer->partInfo->StartingOffset.LowPart,
			  &(i_hClauer->partInfo->StartingOffset.HighPart),
			  FILE_BEGIN) == INVALID_SET_FILE_POINTER ){
	LOG_Msg(2,"ERROR SetFilePointer");
	LOG_Debug(2,"ERROR= %d ", GetLastError());
	return ERR_IO_SEEK;
      }
      i_hClauer->pos = 0;
    }
    
#endif
    
    break;
    
  case IO_SEEK_RESERVED:
    
    if ( offset >= i_hClauer->ib.rzSize )
      return ERR_IO_OUT_OF_BOUNDS;
    
#if defined(LINUX)
    if ( lseek(i_hClauer->hDevice, BLOCK_SIZE, SEEK_SET) == -1 ) 
      return ERR_IO_SEEK;
#elif defined(WIN32)
    if ( i_hClauer->isFile ) {
      if ( SetFilePointer(i_hClauer->hDevice, i_hClauer->partInfo->StartingOffset.LowPart,
			  &(i_hClauer->partInfo->StartingOffset.HighPart),
			  FILE_BEGIN) == INVALID_SET_FILE_POINTER ){
	LOG_Msg(2,"ERROR SetFilePointer");
	LOG_Debug(2,"ERROR= %d ", GetLastError());
	return ERR_IO_SEEK;
      }
    } else {
      if ( SetFilePointer(i_hClauer->hDevice, i_hClauer->partInfo->StartingOffset.LowPart,
			  &(i_hClauer->partInfo->StartingOffset.HighPart),
			  FILE_BEGIN) == INVALID_SET_FILE_POINTER ){
	LOG_Msg(2,"ERROR SetFilePointer");
	LOG_Debug(2,"ERROR= %d ", GetLastError());
	return ERR_IO_SEEK;
      }
      i_hClauer->pos = BLOCK_SIZE;
    }
#endif   
    
    break;
    
  case IO_SEEK_OBJECT:

    if ( offset >= i_hClauer->ib.totalBlocks )
      return ERR_IO_OUT_OF_BOUNDS;
    
#if defined(LINUX)
    if ( (ret=lseek(i_hClauer->hDevice, (1 + offset + i_hClauer->ib.rzSize)* BLOCK_SIZE , SEEK_SET)) == -1 )
      return ERR_IO_SEEK;
#elif defined(WIN32)
    if ( i_hClauer->isFile ) {
      if ( SetFilePointer(i_hClauer->hDevice, (1 + offset + i_hClauer->ib.rzSize)*BLOCK_SIZE, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER )
	return ERR_IO_SEEK;
    } else {
      ll1= (1 + offset + i_hClauer->ib.rzSize)*BLOCK_SIZE;
      ll2=  i_hClauer->partInfo->StartingOffset.LowPart;
      
      memcpy(((char *)(&ll2))+4, &i_hClauer->partInfo->StartingOffset.HighPart, 4 ); 
      ll1= ll1+ll2;
      
      lint.LowPart= ll1;
      memcpy(((char *)(&lint.HighPart)), (((char *) (&ll1))+4), 4 ); 
 
      if ( SetFilePointer(i_hClauer->hDevice, 
			  lint.LowPart, &lint.HighPart,
			  FILE_BEGIN) == INVALID_SET_FILE_POINTER )
	return ERR_IO_SEEK;

      i_hClauer->pos = (1 + offset + i_hClauer->ib.rzSize)*BLOCK_SIZE;

    }
#endif
    break;
    
  default:
    
    return ERR_IO_INVALID_PARAMETER;
    
  }
  
  return IO_SUCCESS;
  
}


/*! \brief Writes the information block.
 *
 * Writes the information block.
 *
 * \param hClauer
 *        The successfully opened clauer handle.
 *
 * \param ib
 *        The information block.
 *
 * \retval IO_SUCCESS
 *         Ok
 *
 * \retval IO_Write()
 *         IO_Write() error codes.
 *
 */

int IO_WriteInfoBlock ( clauer_handle_t hClauer, block_info_t *ib )
{
  i_clauer_handle_t *i_hClauer = (i_clauer_handle_t *) hClauer;
  int ret;
  
  ret = IO_Seek(hClauer, 0, IO_SEEK_INFO);
  if ( ret != IO_SUCCESS ) 
    return ret;
  
  ret = IO_Write(hClauer, (unsigned char *) ib);
  if ( ret != IO_SUCCESS )
    return ret;
  
  memcpy((void *)&(i_hClauer->ib), (void *)ib, BLOCK_SIZE);
  
  return IO_SUCCESS;
}





#ifdef LINUX
// Auxiliar function readnline
// Reads a line from fp into a 
// buffer delimitated by maxlen
// returns -1 or the length of 
// the line. 

int readnline(int fp, char * buff, int maxlen)
{
  int ret,readed= 0;
  char aux;
    
  ret= read(fp,&aux,1);
  if (ret == -1) 
    return -1; 
  while ( ret != 0 && aux != '\n' && readed < maxlen ){
    buff[readed++]=aux;
    ret= read(fp,&aux,1);
    if (ret == -1) 
      return -1; 
  }
  return readed;
}
#endif


#ifdef LINUX
int getClauerFilesInDir(char * dirname, char ** clauerPath, unsigned char * nclauersPath){
  DIR * dir; 
  struct dirent * dirent; 
  char path[MAX_DEVICE_LEN];
    
  dir = opendir(dirname);
  if ( dir ) {
    dirent= readdir(dir);
    while ( dirent != NULL ){
      if ( strlen(dirent->d_name)==12 && isdigit(dirent->d_name[5]) && 
	   isdigit(dirent->d_name[6]) && isdigit(dirent->d_name[7]) &&
	   strncasecmp(dirent->d_name,"CRYF_",5) == 0 && strncasecmp(dirent->d_name+8,".CLA",4) == 0 ){
	  
	if (dirname[strlen(dirname)-1] == '/')
	  snprintf(path,MAX_DEVICE_LEN,"%s%s",dirname,dirent->d_name);
	else{
	  snprintf(path,MAX_DEVICE_LEN,"%s/%s",dirname,dirent->d_name);
	}
	clauerPath[*nclauersPath]= (char *) calloc((strlen(path) + 1),
                                                   sizeof(char));
	memcpy(clauerPath[*nclauersPath],path,strlen(path) + 1);
	//clauerPath[*nclauersPath][strlen(path)]='\0';
		
	(*nclauersPath)++;
      }
      dirent= readdir(dir);
    }
    closedir(dir);
  }
  return 0;
}
#endif



#ifdef WIN32

int IO_GetBytesSectorByPath(const char * device){

  int disp;
  
  if ((!device) || (strlen(device) <= 0))
    return -1;

  disp= atoi(&device[strlen(device)-1]);

  LOG_Debug(1, "Obtenido número de dispositivo: %d", disp);
  
  return IO_obtenerBytesSector(disp);
  
}


// 02-04-2007 
// Obtiene los BytesPorSector.
int IO_obtenerBytesSector(int dispositivo)
{
  HANDLE hDevice;               // handle to the drive to be examined 
  BOOL bResult;                 // results flag
  DWORD junk;                   // discard results
  TCHAR volumePath[MAX_PATH];
  DISK_GEOMETRY pdg;
  int res;

  _stprintf( volumePath, _T("\\\\.\\PHYSICALDRIVE%d"), dispositivo );
  hDevice = CreateFile(volumePath,  // drive 
		       0,                // no access to the drive
		       FILE_SHARE_READ | // share mode
		       FILE_SHARE_WRITE, 
		       NULL,             // default security attributes
		       OPEN_EXISTING,    // disposition
		       0,                // file attributes
		       NULL);            // do not copy file attributes

  if (hDevice == INVALID_HANDLE_VALUE) // cannot open the drive
    {
      return (FALSE);
    }

  bResult = DeviceIoControl(hDevice,  // device to be queried
			    IOCTL_DISK_GET_DRIVE_GEOMETRY,  // operation to perform
			    NULL, 0, // no input buffer
			    &pdg, sizeof(pdg),     // output buffer
			    &junk,                 // # bytes returned
			    (LPOVERLAPPED) NULL);  // synchronous I/O

  CloseHandle(hDevice);
  LOG_Debug(1,"BytesPerSector= %d ", pdg.BytesPerSector);

  if ( !bResult ){
    LOG_Debug(1,"Fallo  el DeviceIoControl motivo Error= %d ", GetLastError());
    res= -1;
  }
  else 
    res= pdg.BytesPerSector;

  return res;
}



int IO_IsReadable ( char* deviceName, int dispositivo ) 
{
  
  int se_puede_leer = 0, bytesSector;
  char cadena_unidad[20];
  HANDLE handle;
  int result;
  BYTE * sector;

  LOG_Msg(1, "ENTRAMOS EN IO_IsReadable");

  bytesSector= IO_obtenerBytesSector(dispositivo);
  if ( bytesSector < 0 ){
    LOG_Error(1, "Imposible obtener bytesSector= %d", bytesSector);
    return 0;
  }
  
  sector= (BYTE *) malloc(bytesSector);

  //strcpy( cadena_unidad, "\\\\.\\" );
  //strcat( cadena_unidad, unidad );

  LOG_Debug(1, "Cadena de deviceName: %s", deviceName);
 
  handle = CreateFile(deviceName,			     // leemos del dispositivo ndevice		    
		      GENERIC_READ|GENERIC_WRITE,        // lo abrimos para leer y escribir
		      FILE_SHARE_READ|FILE_SHARE_WRITE,  // compartido para leer y escribir
		      NULL,								 // el handle no se puede heredar
		      OPEN_EXISTING,						 // se supone que existe
		      FILE_ATTRIBUTE_NORMAL, // fichero normal
		      NULL);								 // sin template file
  
  if ( handle != INVALID_HANDLE_VALUE ) {
    DWORD dwBytesRead;    
    LOG_Debug(1, "CORRECT HANDLE VALUE AL ABRIR %s", deviceName);
    LOG_Debug(1, "Obtenido anteriormente bytesSector= %d", bytesSector);
    /* Probamos a leer un sector de la unidad */    
    if ( ReadFile(handle, sector, bytesSector, &dwBytesRead, NULL) ){
      LOG_Msg(1, "OK Leyendo...");
      se_puede_leer = 1;
    }
  }
  else
    LOG_Error(1, "ERROR AL ABRIR EL HANDLE  %d", GetLastError());
  
  CloseHandle( handle );
  
  return se_puede_leer;
  
}
#endif





int IO_EnumClauers ( unsigned char *nDevs, char *devices[IO_MAX_DEVICES], int mode)
{

#ifdef LINUX
  char  i, j;
  int fp, ret; 
  char * idx1, * idx2;
  char deviceName[MAX_DEVICE_LEN];
  unsigned char nclauers=0, nclauersPath=0;
  // char * clauerPath[MAX_SCSI_DEV];
  char line[1024];
  
  nclauers= 0;

  LOG_Msg(1, "ENTRAMOS EN IO_EnumClauers");

  // First we will look for usb devices 
#ifdef MAC 
  // look for /dev/disk[0-9] on Mac
  for (i='0';i<='9';i++){
    sprintf(deviceName, "/dev/disk%c", i); 
#else 
    
  LOG_Msg(1, "Paseamos por /dev/sdX");
  for (i='a';i<='z';i++){
     sprintf(deviceName, "/dev/sd%c", i);
     LOG_Debug(1, "Considerando /dev/sd%c",i);

#endif
      if ( IO_Is_Clauer(deviceName) == IO_IS_CLAUER ){
	devices[nclauers]= (char *) malloc(sizeof(char) * (strlen(deviceName) + 1));
	if ( devices[nclauers] == NULL )
	  break;
	memcpy(devices[nclauersPath],deviceName,strlen(deviceName)+1);
	nclauers++;
	nclauersPath++;
      }
      else{
         LOG_Debug(1, "Considerando /dev/sd%c NO ES CLAUER",i);
      }
    }

    // look for /dev/disk[00-99] on mac 
#ifdef MAC 
    for (i='0';i<='9';i++){
      for (j='0';j<='9';j++){
        sprintf(deviceName, "/dev/disk%c%c", i, j);
        if ( IO_Is_Clauer(deviceName) == IO_IS_CLAUER ){
          devices[nclauers]= (char *) malloc(sizeof(char) * (strlen(deviceName) + 1));
	  if ( devices[nclauers] == NULL ) 
	    break;
          strncpy(devices[nclauersPath],deviceName, strlen(deviceName)+1);
          nclauers++;
          nclauersPath++;
	}
      }
    }
#endif 
  
  
    // Secondly we will look for CRYF_[0-9][0-9][0-9].cla files 
    // on mounted filesystems root if it is enabled.

    if (mode == IO_ENUM_ALL || mode == IO_ENUM_FILES ){
      fp= open("/etc/mtab",O_RDONLY);
      if (fp != -1) {
	ret= readnline(fp,line,1024); 
	while ( ret != 0 ) {
	  if (ret == -1) {
	    break;
	  }
	  line[ret]= '\0';
	  if ( line[0]== '/' ){ // Physical devices begin with /dev on /proc/mounts 
	    // virtual filesystems are not interesting.
	    if ( ( mode == IO_ENUM_FILES && strstr(line,"/dev/fd") == NULL ) || ( mode == IO_ENUM_ALL )  ){
	      idx1= index(line,' ');
	      idx1++;
	      if (idx1 != NULL){
		idx2= index(idx1,' ');
		if (idx2 != NULL){
		  idx2[0]='\0';
		  getClauerFilesInDir(idx1,devices, &nclauersPath);
		}
	      }
	    }
	  }
	  ret= readnline(fp,line,1024); 
	}
      }
#ifdef MAC // If it is a Mac, wr look for CRYF files on / explicitly
      getClauerFilesInDir("/",devices, &nclauersPath);
#endif 
    }
    nclauers= nclauersPath;
    *nDevs= nclauers;
  
#ifdef DEBUG_CRYF
    // int fpx;
    // if ( (fpx=open("/media/cdrom/CRYF_000.cla",O_RDONLY)) != -1 )
    //	nclauers++;
    // close(fpx);
#endif

#elif defined(WIN32)

  
    char *driveLetters = NULL, deviceName[MAX_DEVICE_LEN], drive[3], fileName[16],auxbuff[3];
    DWORD tam, deviceNameSize, i, j, dwBytesReturned;
    UINT driveType;
    HANDLE hFindFile = NULL, hFile = NULL,hDev=NULL;
    WIN32_FIND_DATA findData;
    BOOL esClauer;
    char cdromDeviceName[10];
    int err, nbytes,x,disp;
    char auxDev[8];
    STORAGE_DEVICE_NUMBER sdn;
  
#ifdef LOG

#ifndef NO_LOG_INIT
  if (!g_logIni){ LOG_Ini(2, 10); g_logIni=1;}
#endif 
    LOG_Msg(1, "Pasando por enumerar clauers");
#endif

    memset((void *) devices, 0, IO_MAX_DEVICES * sizeof(char *));
  
    *nDevs = 0;
  
    tam = GetLogicalDriveStrings(0, NULL);
    if ( ! tam ) {
      err = ERR_IO;
      goto err_IO_EnumClauers;
    }

    LOG_Debug(1, "Obtenido tamaño con GetLogicalDriveStrings = %d",tam);

    driveLetters = ( char * ) malloc ( sizeof(char) * (tam +1));
    if ( ! driveLetters ) {
      err = ERR_IO_OUT_OF_MEMORY;
      goto err_IO_EnumClauers;
    }

    driveLetters[tam]= 0;
    //LOG_Debug(1, "Obtenidas DriveLetters = %s",driveLetters);
  
    if ( ! GetLogicalDriveStrings(tam, driveLetters) ) {
      err = ERR_IO;
      goto err_IO_EnumClauers;
    }
  
    LOG_Debug(1, "Obtenido de GetLogicalDriveStrings = %d",tam);
  
    i=0;
    while ( i < (tam-1) ) {
    
      if ( *nDevs >= IO_MAX_DEVICES ) {
	err = ERR_IO;
	goto err_IO_EnumClauers;
      }
    
    
      driveType = GetDriveType(driveLetters+i);
      LOG_Debug(1, "driveType = %d",driveType);

      if ( driveType == DRIVE_CDROM || ( strncmp(driveLetters+i, "C:", 2) == 0 ) ) {
	 
	LOG_Msg(1, "Que es DRIVE_CDROM o C:");

	/* We've a CD, so lookup for CRYF_xxx.cla files
	 */
      
	strncpy(fileName, driveLetters+i, 3);
	fileName[3] = 0;
	strcat(fileName, "CRYF_???.cla");
      
	strncpy(drive,driveLetters+i,2);
	drive[2] = 0;
	deviceNameSize = QueryDosDevice(drive, deviceName, MAX_DEVICE_LEN);
	strncpy(cdromDeviceName,"\\\\.\\",8);
	strcat(cdromDeviceName,drive);
	cdromDeviceName[9]='\0';		
      
	// Bajo ciertas condiciones, windows 2000 (XP no se ha probado)
	// no actualiza el estado del cdrom frente a una extracción y la
	// función FindFirstFile hace que aparezca una ventana indicando 
	// que debemos insertar el cd. 
	// Este comportamiento se puede evitar mediante el siguiente 
	// CreateFile, que de algún modo actualiza el estado 
	// interno del Windows con respecto a la unidad de cdrom.
      
	if ( driveType == DRIVE_CDROM  && ( mode==IO_ENUM_ALL || mode==IO_ENUM_FILES ) ) {
	  LOG_Msg(1, "Es de tipo DRIVE_CDROM");
	  hFile = CreateFile(cdromDeviceName,
			     GENERIC_READ,
			     FILE_SHARE_READ,
			     NULL,								 
			     OPEN_EXISTING,						
			     FILE_ATTRIBUTE_NORMAL,
			     NULL);													 
	
	  // Podríamos hacer despues un IO_Control, pero no es necesario, el hecho de 
	  // abrir y cerrar el handle es suficiente.
	  // err = DeviceIoControl( hFile, IOCTL_DISK_UPDATE_PROPERTIES, NULL, 0, NULL, 0, &nbytes, NULL );	
	  // if ( err == 0 )  
	  // 	return ERR_LOWLEVEL_SI;

	  if ( hFile == INVALID_HANDLE_VALUE ) {
	    LOG_Debug(1, "hFile ha devuelto invalid value GetLastError= %d", GetLastError());
	    while ( (driveLetters[i] != '\0') && (i < (tam-1)))
	      ++i;
	    ++i;
	    continue;
	  }
	
	  CloseHandle(hFile);
	}

	
	hFindFile = FindFirstFile(fileName, &findData);	
      
	if ( hFindFile != INVALID_HANDLE_VALUE ) {
	  LOG_Msg(1, "FindFirstFile Ok");
	  do {
	    /* Test if ??? are numbers. It's up to the caller
	     * application to test if it's a Clauer or not.
	     */
	  
	    if ( isdigit(*(findData.cFileName + 5)) && 
		 isdigit(*(findData.cFileName + 6)) &&
		 isdigit(*(findData.cFileName + 7)) ) {
	    
	      LOG_Debug(1, "Encontrado CRYF= %s ", findData.cFileName);
	      devices[*nDevs] = (char *) malloc ( 16 );
	      if ( ! devices[*nDevs] ) {
		err = ERR_IO;
		goto err_IO_EnumClauers;
	      }
	    
	      strcpy(devices[*nDevs], driveLetters+i);
	      strcat(devices[*nDevs], findData.cFileName);
	      (*nDevs)++;
	    }
	  
	  } while ( FindNextFile(hFindFile, &findData) );
	
	  FindClose(hFindFile);
	  hFindFile = NULL;
	}
      
      } 

      // TODO: Adaptación a Windows Vista, consiste en eliminar el código de este else if
      //       y añadir lo que hay más abajo.
      /*COMENTAR DESDE AQUÍ*/

      else if ( GetDriveType(driveLetters+i) == DRIVE_REMOVABLE ) {
      
	LOG_Debug(1, "Encontrado DriveRemovable= %c ", *(driveLetters+i));
      
	strncpy(drive,driveLetters+i,2);
	drive[2] = 0;
      
	LOG_Debug(1, "Que es  Drive= %s ", drive);
      
	deviceNameSize = QueryDosDevice(drive, deviceName, MAX_DEVICE_LEN);
      
	LOG_Debug(1, "Hacemos un QueryDosDevice= %s", deviceName);
      
	if ( ! deviceNameSize ) {
	  LOG_Error(1, "Error haciendo QueryDosDevice deviceNameSize= %d", deviceNameSize);
	  err = ERR_IO;
	  goto err_IO_EnumClauers;
	}
	LOG_Debug(1, "Tenemos deviceNameSize= %d", deviceNameSize);
      
	// Test if deviceName is a HardDisk by parsing
	// it ( we look for the Harddisk substring)
	//
      
	j = 0;
	while ( *(deviceName+j) ) {
	  //LOG_Debug(1, "Consideramos deviceName= %c ", *(deviceName+j));
	
	  if ( *(deviceName+j) == 'H' ) {
	  
	    if ( strncmp(deviceName+j, "Harddisk", 8) == 0 ) {

	      char num[3];
	      num[0] = deviceName[j+8];
	      if ( isdigit(deviceName[j+9]) ) {
		num[1] = deviceName[j+9];
		num[2] = 0;
	      } 
	      else
		num[1] = 0;
	    
	      disp= atoi(num);
	    
	    
	      // Si el dispositivo se puede leer se añade
	     

		LOG_Msg(1, " Entrando por Harddisk IO_Is_Readable");
	    
		strncpy(auxDev, "\\\\.\\",4);
		strncpy(auxDev+4,drive,2);
		auxDev[6]='\0';
	    
		LOG_Debug(1, "Consideramos DEVICE= %s ", auxDev);
	    
		hDev = CreateFile(auxDev, 0, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, 
				  OPEN_EXISTING, 0, NULL);

		if ( hDev == INVALID_HANDLE_VALUE ) {
		  if ( GetLastError() == ERROR_ACCESS_DENIED ) {
		    LOG_MsgError(1,"Error acceso denegado");
		  } else {
		    LOG_MsgError(1,"No se puede abrir dispositivo");
		  }
		}
    
		if ( DeviceIoControl(hDev,
				     IOCTL_STORAGE_GET_DEVICE_NUMBER,
				     NULL,
				     0,
				     &sdn,
				     sizeof(sdn),
				     &dwBytesReturned,
				     NULL
				     ) == 0 )
		  {
		    LOG_Debug(1,"Error al lanzr IOCTL= %d=", GetLastError());
		  }
	    
		CloseHandle(hDev);

		LOG_Debug(1,"Obtenido VOLUME NUMBER= %d", sdn.DeviceNumber );
		snprintf(auxbuff,3,"%d", sdn.DeviceNumber);

		devices[*nDevs] = (char *) malloc ( strlen("\\\\.\\PHYSICALDRIVE") + strlen( auxbuff ) + 1 );
		if ( ! devices[*nDevs] ) {
		  err = ERR_IO;
		  goto err_IO_EnumClauers;
		}
	      
		strcpy(devices[*nDevs], "\\\\.\\PHYSICALDRIVE");
		strcat(devices[*nDevs],  auxbuff );
	      
		LOG_Debug(1, "Construido device completo= %s ", devices[*nDevs]);
		if ( (mode == IO_ENUM_USB_ALL) || (IO_Is_Clauer( devices[*nDevs] ) == IO_IS_CLAUER) ){
		  LOG_Debug(1, "Modo= %d ", mode);
		  LOG_Msg(1, "IO_Is_Clauer devuelve cierto o flag IO_ENUM_USB_ALL, incrementando número de devices obtenidos."); 
		   if ( IO_IsReadable(devices[*nDevs], disp) ) {
		     (*nDevs)++;
		   }
		}
		else{
		  LOG_Msg(1, "IO_Is_Clauer devuelve falso.");
		}
	    } 
	  
	  } // if Harddisk 
	
	  ++j;
	
	} // while 
      
      } // driveRemovable 
          
      while ( (driveLetters[i] != '\0') && (i < (tam-1)))
	++i;
      ++i;
    
    } /* while */
    
    free(driveLetters);

    /* Comprobamos si hay cryf en C:\
     */
   
    LOG_Debug(1, "Obtenidos NDEV = %d",*nDevs);   
    LOG_Msg(1,"Saliendo de la función");
    //LOG_End(); 

    return IO_SUCCESS;
  
  err_IO_EnumClauers:
  
    if ( driveLetters )
      free(driveLetters);
  
    if ( hFindFile )
      CloseHandle(hFindFile);
  
    for ( i = 0 ; (i < IO_MAX_DEVICES) && devices[i] ; i++ ) 
      free(devices[i]);
  
    return err;
   
#endif
  
  
    return IO_SUCCESS;
  }


#ifdef WIN32
  int IO_GetDriveLetter(clauer_handle_t * hClauer, char * logical_unit, int * found){

	  HANDLE hDev;
	  i_clauer_handle_t * i_hClauer;
	  char * driveLetters= NULL, auxDev[7], auxbuff[4];
	  char * auxphys= NULL;
	  int i=0, len=0, tam=0, err=0;
	  DWORD dwBytesReturned=0;
	  STORAGE_DEVICE_NUMBER sdn;

	  if ( !hClauer || !logical_unit  ){
		  return ERR_IO_INVALID_PARAMETER;
	  }

	  i_hClauer= (i_clauer_handle_t *)  hClauer; 
	  *found= 0;

	  tam = GetLogicalDriveStrings(0, NULL);
	  if ( ! tam ) {
		  return ERR_IO;    
	  }

	  LOG_Debug(1, "Obtenido tamaño con GetLogicalDriveStrings = %d",tam);
	 

	  driveLetters = ( char * ) malloc ( sizeof(char) * (tam +1));
	  if ( ! driveLetters ) {
		  return ERR_IO_OUT_OF_MEMORY;
	  }

	  driveLetters[tam]= 0;

	 
	  if ( ! GetLogicalDriveStrings(tam, driveLetters) ) {
		  free(driveLetters);
		  return  ERR_IO;
	  }

	  i=0;
	  while ( i < (tam-1) ) {	

		  strncpy(auxDev, "\\\\.\\",4);
		  strncpy(auxDev+4,driveLetters+i,2);
		  auxDev[6]='\0';

		  hDev = CreateFile(auxDev, 0, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, 
			  OPEN_EXISTING, 0, NULL);

		  if ( hDev == INVALID_HANDLE_VALUE ) {
			  if ( GetLastError() == ERROR_ACCESS_DENIED ) {
				  LOG_MsgError(1,"Error acceso denegado");
			  }
			  else {
				  LOG_MsgError(1,"No se puede abrir dispositivo");
			  }
		  }

		  if ( DeviceIoControl(hDev,
			  IOCTL_STORAGE_GET_DEVICE_NUMBER,
			  NULL,
			  0,
			  &sdn,
			  sizeof(sdn),
			  &dwBytesReturned,
			  NULL
			  ) == 0 )
		  {
			  LOG_Debug(1,"Error al lanzar IOCTL= %d=", GetLastError());
		  }

		  LOG_Debug(1,"Obtenido VOLUME NUMBER= %d", sdn.DeviceNumber );
		  LOG_Debug(1,"Obtenido File device type= 0x%x", sdn.DeviceType);

		  snprintf(auxbuff,3,"%d", sdn.DeviceNumber);

		  len=strlen("\\\\.\\PHYSICALDRIVE") + strlen( auxbuff ) + 1 ;
		  auxphys= (char *) malloc ( len );
		  if ( ! auxphys ) {
			  free(driveLetters);
			  CloseHandle(hDev);
			  return ERR_IO;

		  }

		  strcpy(auxphys, "\\\\.\\PHYSICALDRIVE");
		  strcat(auxphys,  auxbuff );

		 if ( sdn.DeviceType==FILE_DEVICE_DISK &&  strncmp(auxphys, i_hClauer->device, len ) == 0 ){
			  *found= 1;
			  strncpy(logical_unit, driveLetters+i, 3);
			  logical_unit[3]=0; 
			  CloseHandle(hDev);
			  free(auxphys);
			  free(driveLetters);
			  return IO_SUCCESS;
		  }


		  free(auxphys);  
		  CloseHandle(hDev);

		  while ( (driveLetters[i] != '\0') && (i < (tam-1)))
			  ++i;
		  ++i;
	  }

	  free(driveLetters);	

	  return IO_SUCCESS;  
  }
#endif 
 
 int IO_SetInfoBlock(clauer_handle_t * hClauer, block_info_t *ib ){
   
   i_clauer_handle_t * i_hClauer;
   
   i_hClauer= (i_clauer_handle_t *)  hClauer; 
   memcpy( &(i_hClauer->ib), ib, BLOCK_SIZE );
   i_hClauer->ibInit = 1;
   
   return IO_SUCCESS;
 }

#ifdef WIN32 
 int IO_Is_Removable(const char * device){
   
   char letter[4];
   clauer_handle_t hClauer;
   int found;
   UINT driveType;
   
   LOG_Debug(1, "IO_Is_Removable: %s", device);

   if ( IO_Open(device, &hClauer, IO_RDWR, -1) != IO_SUCCESS ){
	   LOG_MsgError(1,"IO_Open(device, &hClauer, IO_RDWR, -1)");    
	   return ERR_IO;
   }
   

   if ( IO_GetDriveLetter(hClauer, letter, &found) != IO_SUCCESS ){
	 LOG_MsgError(1,"IO_GetDriveLetter(hClauer, letter, &found)");  
     IO_Close(hClauer);
     return ERR_IO;
   }
   // Close the string. Expected format: "F:\"+"\0"
   letter[3]= '\0';

   if ( IO_Close(hClauer) != IO_SUCCESS ){
	 LOG_MsgError(1,"IO_Close(hClauer)");
     return ERR_IO;
   }
   
   if (! found ){
	 LOG_MsgError(1,"IO_GetDriveLetter(hClauer, letter, &found) returned ! found");
     return IO_IS_NOT_REMOVABLE;
   }
  
   LOG_Debug(1,"letter= %s",letter);
   LOG_Debug(1,"device = %s",device);

   driveType = GetDriveType(letter);
   if ( driveType ==  DRIVE_REMOVABLE ){
	   LOG_Msg(1,"GetDriveType(letter) returned DRIVE_REMOVABLE");    
	   return IO_IS_REMOVABLE;
   }
   else{
	 LOG_Debug(1, "GetDriveType(letter) returned NOT REMOVABLE, value: %d", driveType);
	 switch (driveType) {
		case 0:
			LOG_Msg(1, "driveType= DRIVE_UNKNOWN");
			break;
		case 1:
			LOG_Msg(1, "driveType= DRIVE_NO_ROOT_DIR");
			break;
		case 3:
			LOG_Msg(1, "driveType= DRIVE_FIXED");
			break;
		case 4:
			LOG_Msg(1, "driveType= DRIVE_REMOTE");
			break;
		case 5:
			LOG_Msg(1, "driveType= DRIVE_CDROM");
			break;
		case 6:
			LOG_Msg(1, "driveType= DRIVE_RAMDISK");
			break;
		default:
			LOG_Msg(1, "Unknown return type.");
			break;
	 }
     return IO_IS_NOT_REMOVABLE;
   }
   return IO_IS_REMOVABLE;
}
#endif 
