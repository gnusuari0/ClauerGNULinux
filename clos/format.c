
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

#include "format.h"
#include "err.h"
#include "log.h"
#include <clio/clio.h>
#include <CRYPTOWrapper/CRYPTOWrap.h>
//#include <assert.h>
#include <math.h>
#include <stdlib.h>


/** 
Clauer Fromat functions, this set of functions 
allow the service to format any USB on the  
system.
**/

#ifdef WIN32
int FORMAT_GetDeviceLayout(char * device, PDRIVE_LAYOUT_INFORMATION * layout){
	HANDLE hDev;
	DWORD outSize;


	if ( !device )
		return ERR_FORMAT_INVALID_PARAMETER;

	hDev = CreateFile(device, GENERIC_READ, 
		FILE_SHARE_READ|FILE_SHARE_WRITE, 
		NULL, OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_NO_BUFFERING, 0);

	if ( hDev == INVALID_HANDLE_VALUE ) {
		if ( GetLastError() == ERROR_ACCESS_DENIED ) {
			return ERR_FORMAT_NO_PERM;
		} else {
			return ERR_FORMAT_CANNOT_OPEN_DEVICE;
		}
	}

	*layout = ( PDRIVE_LAYOUT_INFORMATION ) malloc ( sizeof(DRIVE_LAYOUT_INFORMATION) + 4*sizeof(PARTITION_INFORMATION) );
	memset(*layout, 0, sizeof(DRIVE_LAYOUT_INFORMATION) + 4*sizeof(PARTITION_INFORMATION) );

	if ( ! *layout ) { 
		CloseHandle(hDev);
		return ERR_FORMAT_CANNOT_ALLOCATE_MEMORY;
	}
	if ( ! DeviceIoControl(hDev, IOCTL_DISK_GET_DRIVE_LAYOUT, 
		NULL, 0, *layout, 
		sizeof(DRIVE_LAYOUT_INFORMATION) + 4*sizeof(PARTITION_INFORMATION),
		&outSize, NULL) ) { 

			CloseHandle(hDev);
			free(*layout);
			return ERR_FORMAT_IOCTL;
	}
	CloseHandle(hDev);  

	return FORMAT_OK;
}


int FORMAT_GetClauerLayout(char * device, CLAUER_LAYOUT_INFO * cli){
	PDRIVE_LAYOUT_INFORMATION  layout;
	int res=0;
	unsigned int i=0;
	long long lAux=0;

	res= FORMAT_GetDeviceLayout(device, &layout);
	if ( res != FORMAT_OK )
		return res; 

	memset(cli, 0, sizeof(CLAUER_LAYOUT_INFO));

	cli->npartitions= layout->PartitionCount;
	LOG_Debug(1, "Poniendo partitionCount a cli->npartitions = %d", cli->npartitions);
	for (i=0; i<layout->PartitionCount; i++){
		// We must convert LARGE_INTEGER to long long.

		lAux= layout->PartitionEntry[i].PartitionLength.LowPart;
		memcpy(((char *)(&lAux))+4, &layout->PartitionEntry[i].PartitionLength.HighPart, 4 );

		cli->cpi[i].size= lAux;
		cli->cpi[i].type= layout->PartitionEntry[i].PartitionType;
	}

	return FORMAT_OK;	
}
#endif

int FORMAT_CreateClauer(char * device, char porcentaje){

#ifdef WIN32
	unsigned char* mbr;
	int result;
	unsigned long long int tamanyo, tbytes=0, maxsize, rel_sectors, num_sectors, bytesSector;
	BYTE * sector;
	unsigned int indice, chs_overflow=0, H, S, C, C1, aux;
	clauer_handle_t hClauer;
	unsigned char signature[3] = "\x55\xAA";

	LOG_Msg(0,"Get into FORMAT_CreateClauer function.");
	
	if ( !device ){
		return ERR_FORMAT_INVALID_PARAMETER;	
	} else {
	    LOG_Debug(0,"Got device (1st argument): %s.", device);
	}

	bytesSector= IO_GetBytesSectorByPath( (const char *)device );
	
	if ( bytesSector < 0 ){
		LOG_MsgError(0,"Cannot get bytes/sector");
		return ERR_FORMAT_CANNOT_GET_BYTES_SECTOR;
	} else {
	    LOG_Debug(0, "Got %d bytes/sector.", bytesSector);
	}

    aux= atoi(&porcentaje);

	if (aux > 100 || aux < 0) {
		LOG_MsgError(0,"Invalid percent value (aux).");
		return ERR_FORMAT_INVALID_PERCENT;
	} else {
	    LOG_Debug(0, "Got %d percent.", aux);
	}

	LOG_Msg(0,"Addressing memory for a sector.");
	sector= (BYTE *) malloc(bytesSector);
	if( sector == NULL ){
		LOG_MsgError(0,"Out of memory");
		return ERR_FORMAT_CANNOT_ALLOCATE_MEMORY;
	}

	result= FORMAT_GetSize(device, &tamanyo);
	// Get a copy of tamanyo in tbytes
	tbytes= tamanyo;
	//ASSERT(tbytes == tamanyo);

	if ( result != FORMAT_OK ){
		LOG_Error(0,"FORMAT_GetSize returned %d .", result);	
		return ERR_FORMAT_CANNOT_RETRIEVE_SIZE;
	} else {
	    LOG_Debug(0,"Got disk size: %lld bytes", tamanyo);
	}

	if ( aux == 0 ) {
		tamanyo /= 1024000;
		LOG_Debug(0,"Got disk size: %lld MB", tamanyo);
		
		if ( tamanyo >= 512 ){
			LOG_Msg(0, "Disk size >= 512 MB");
			/**
			 * If the disk size is greater or equal than 
			 * 512 MB get only the 1% for crypto partition.
			 */
			aux= 99;
			LOG_Msg(0, "Getting 99% of total size for data partition");
		}
		else {
			/**
			 * Otherwise (disk size < 512 MB) get 4% for crypto partition:
			 */
			aux= 96;
		}
	}

	if (aux > 100 || aux <= 0) {
		LOG_MsgError(0,"Invalid percent value (aux).");
		return ERR_FORMAT_INVALID_PERCENT;
	} else {
	    LOG_Debug(0,"Final percent got for data partition: %d", aux);
	}

	LOG_Debug(0, "tbytes= %lld", tbytes);
	//LOG_Debug(0, "1023 * 255 * 63  * bytesSector= %d (integer)", 1023 * 255 * 63  * bytesSector);
	//LOG_Debug(0, "1023 * 255 * 63  * bytesSector= %lld (long long int)", 1023 * 255 * 63  * bytesSector);

	/**
	 * Warning: Next operation is a little bit tricky,
	 * Visual C++ compiler get the biggest operator type
	 * to store the result, independently of the type of
	 * the variable when it's stored... so be careful!
	 */
	maxsize= 1023 * 255 * 63 * bytesSector;
	LOG_Debug(0, "Computed maxsize: %lld", maxsize);
	/**
	 * Check if disk size is greater than 7.8 GB
	 */
	//ASSERT(maxsize >= 0);
	//ASSERT(tbytes  >= 0);
	if (tbytes  > maxsize ){
		   LOG_Msg(0, "CHS overflow detected (disk size > 7.8 GB)");
           chs_overflow= 1;
	       H = 255;
           S = 63;
	       C = tbytes/(H*S*bytesSector);
		   LOG_Debug(0, "Cylinders= %d", C);
	}
    else{
	   LOG_Msg(0, "Getting optimal geometry");
	   result = FORMAT_GetOptimalGeometry( device, &H, &S, &C );
	   if ( result != FORMAT_OK ) {
	 	 LOG_Error(0,"FORMAT_GetOptimalGeometry returned %d", result);	
	 	 return ERR_FORMAT_GET_GEOMETRY;
	   }
    }

	mbr = (unsigned char *) malloc (bytesSector * sizeof(char));
	memset( mbr, 0, bytesSector*sizeof(char) );

	LOG_Msg(0, "Opening device for writing");
	result = IO_Open( device, &hClauer, IO_RDWR, -1 );
	if ( result != IO_SUCCESS ) {
		LOG_Error(0,"IO_Open returned %d", result);	
		return ERR_FORMAT_OPEN_DEVICE;
	}
	LOG_Msg(0, "Device successfully opened");

	rel_sectors= S;
	C1= aux*C/100;
	LOG_Debug(0, "Got C1 %d", C1);
	
	if (chs_overflow) {
	  /**
	   * \todo Adaptar esto al esquema actual 4% o 1%
	   num_sectors = (tbytes - (tbytes*0.01))/bytesSector;
	   */
	  num_sectors = (tbytes - 5242880)/bytesSector; //total - 5MB / bytesSector //C1 * H * S - S;
	}
	else {
	  num_sectors= C1 * H * S - S;
	}

	if (num_sectors <= 0) {
		LOG_Debug(0, "Computed wrong num_sectors: <= 0", num_sectors);
		return ERR_FORMAT_CANNOT_GET_BYTES_SECTOR;
	}
	else {
		LOG_Debug(0, "num_sectors= %lld", num_sectors);
	}


	/* Particion 1 */
	mbr[446] = (BYTE) '\x80';	                /* Partición activa					*/

	mbr[447] = (BYTE) '\x01';					/* Cabeza inicial = 1				*/
	mbr[448] = (BYTE) 0x00000001;				/* Sector inicial = 1				*/
	mbr[449] = (BYTE) 0x00000000;				/* Cilindro inicial = 0				*/

	mbr[450] = (BYTE) 0x00000006;				/* Tipo de la partición = FAT16		*/

	mbr[451] = (BYTE) H-1;					   /* Cabeza final, por parámetro		*/

	if (chs_overflow && C1 > 1023){
		mbr[452] = 0xff;
		mbr[453] = 0xff;
	}
	else{ 
		mbr[452] = ((BYTE) (((C1-1) & 0x00000300) >> 2)) | (BYTE) S;	/* Cilindro final, por parámetro	*/
		mbr[453] = (BYTE) (0x000000FF & (C1-1));
	}

	memcpy(mbr+454, &rel_sectors, 4);
	memcpy(mbr+458, &num_sectors, 4);

	if (aux!=100){
		if (chs_overflow) {
	      /**
	       * \todo Adaptar esto al esquema actual 4% o 1%
		   rel_sectors = (tbytes - (tbytes*0.01))/bytesSector;
		   num_sectors = (tbytes*0.01)/bytesSector;
	       */
		   rel_sectors = (tbytes - 5242880)/bytesSector; // C1*H*S;
		   num_sectors = 5242880/bytesSector;
		}
		else{
		   rel_sectors= C1*H*S;
		   num_sectors= (C-C1) * H * S;
		}
		/* Particion 4 */
		mbr[494] = (BYTE) '\x00';               					    /* Partición no activa				*/
		mbr[495] = (BYTE) '\x00';				                     	/* Cabeza inicial = 0				*/
		mbr[496] = (BYTE) ((BYTE) ((C1 & 0x00000300) >> 2))| (BYTE) 1;	/* Sector inicial = 0				*/
		mbr[497] = (BYTE) (0x000000FF & C1);	                        /* Cilindro inicial = 0				*/
		mbr[498] = (BYTE) 0x69;											/* Filesystem type                  */
		mbr[499] = (BYTE) H-1;					                        /* Cabeza final = 0					*/
		
        if (chs_overflow){
           mbr[452] = 0xff;
           mbr[453] = 0xff;
        }
        else{
           mbr[500] = (BYTE) (((C-1) & 0x00000300) >> 2) | (BYTE) S;		/* Cilindro final = 0 */
		   mbr[501] = (BYTE) (0x000000FF & (C-1));                                 /* Sector final = 0   */
        }
		memcpy(mbr+502, &rel_sectors, 4);
		memcpy(mbr+506, &num_sectors, 4);
	}
	/* Signature */
	memcpy(mbr+510, signature, 2);

	/* Writes MBRs sector */
	LOG_Msg(0, "Write the MBR sector to the device");
	result = IO_WriteSector( hClauer, 0, mbr );
	if ( result != IO_SUCCESS  ){
		LOG_Error(0,"IO_WriteSector returned value=%d", result);	
		return ERR_FORMAT_WRITE_MBR;
	}
	free(mbr);

	/* This forces rereading */
	LOG_Msg(0, "Forcing rereading (part table update)");
	for ( indice=0; indice<bytesSector; indice++ ) sector[indice]=(BYTE) 0;
	result = IO_WriteSector( hClauer, 1, sector );
	if ( result != IO_SUCCESS ){
		LOG_Error(0,"IO_WriteSector 2 returned value=%d", result);
		return ERR_FORMAT_WRITE_MBR;
	}

	result = IO_UpdateProperties( hClauer );
	if ( result != IO_SUCCESS ){
		LOG_Error(0,"IO_UpdateProperties returned value=%d", result);
		return ERR_FORMAT_UPDATE_PROPS;
	}

	result = IO_Close( hClauer );
	if ( result != IO_SUCCESS ){
		LOG_Error(0,"IO_Close returned value=%d", result);
		return ERR_FORMAT_CLOSE;
	}

#endif

	LOG_Msg(0,"Exiting ok!");
	return FORMAT_OK;
}


int FORMAT_FormatClauerData( char * device ){
#ifdef WIN32
	char logical_unit[3];
	int found=0, result;

	clauer_handle_t hClauer;

	/* Now it is time to format the partition 1 as fat32 */
	/* First we must obtain the logical letter that represents the first partition */

	LOG_Debug(1,"Abiendo device = %s", device);

	result = IO_Open( device, &hClauer, IO_RDWR, -1 );
	if ( result != IO_SUCCESS ) {
		return ERR_FORMAT_OPEN_DEVICE;
	}

	result= IO_GetDriveLetter( hClauer, logical_unit, &found );
	if ( result != IO_SUCCESS ){
		IO_Close( hClauer );
		return ERR_FORMAT_GET_DRIVE_LETTER;
	}

	if ( found==0 ){
		IO_Close( hClauer );
		return ERR_FORMAT_DRIVE_LETTER_NOT_FOUND;
	}

	/* IO_GetDriveLetter returns back the letter specification as 
	* X:\ but the format command need X: only so we override 
	* the \ with a 0 */
	logical_unit[2]=0;

	LOG_Debug(1,"Got driveLetter = %s", logical_unit);

	/* Now apply the format */
	LOG_Msg(1,"Formateando...");

	result= FORMAT_FormatearUnidadLogica(logical_unit);
	if ( result != FORMAT_OK ) {
		IO_Close( hClauer );
		return result;
	}

	LOG_Msg(1,"Después de Formatear...");

	result = IO_UpdateProperties( hClauer );
	if ( result != IO_SUCCESS ) return ERR_FORMAT_UPDATE_PROPS;

	result = IO_Close( hClauer );
	if ( result != IO_SUCCESS ) return ERR_FORMAT_CLOSE;

	return 0;
#endif
	return ERR_FORMAT_NOT_IMPLEMENTED;
}


int FORMAT_FormatClauerCrypto( char * device, char * pwd ){

#ifdef WIN32

	int res=0, bs=0, nbloques=0, aux=0;
	unsigned int i=0;
	long long lAux=0;
	unsigned char newIden[40];

	PDRIVE_LAYOUT_INFORMATION layout= NULL;
	INFO_ZONE iz;
	clauer_handle_t hClauer;

	if ( !device ){
		LOG_MsgError(1,"Parámetro device incorrecto.");
		return ERR_FORMAT_INVALID_PARAMETER;
	}

	if ( !pwd ){
		LOG_MsgError(1,"Parámetro pwd incorrecto.");
		return ERR_FORMAT_INVALID_PARAMETER;
	}

	memset(&iz, 0, sizeof(INFO_ZONE));
	
	res=FORMAT_GetDeviceLayout( device, &layout);
	if ( res!= FORMAT_OK ){
		LOG_MsgError(1,"Al invocar FORMAT_GetDeviceLayout.");
		return res;
	}

#ifdef LOG
	LOG_Msg(1,"Begin Partition information: ");
	LOG_Debug(1,"    PartitionCount: %d", layout->PartitionCount);
	for (i=0; i<layout->PartitionCount; i++){
	    LOG_Debug(1,"    Partitionentry Number: %d", i);
	    LOG_Debug(1,"        StartingOffset: %lld", layout->PartitionEntry[i].StartingOffset);
	    LOG_Debug(1,"        PartitionLength: %lld", layout->PartitionEntry[i].PartitionLength);
	    LOG_Debug(1,"        HiddenSectors: %d", layout->PartitionEntry[i].HiddenSectors);
	    LOG_Debug(1,"        PartitionNumber: %d", layout->PartitionEntry[i].PartitionNumber);
	    LOG_Debug(1,"        PartitionType: 0x%02x", layout->PartitionEntry[i].PartitionType);
	    LOG_Debug(1,"        BootIndicator: 0x%02x", layout->PartitionEntry[i].BootIndicator);
	    LOG_Debug(1,"        RecognizedPartition: 0x%02x", layout->PartitionEntry[i].RecognizedPartition);
	    LOG_Debug(1,"        RewritePartition: 0x%02x", layout->PartitionEntry[i].RewritePartition);
	}
#endif

	if ( layout->PartitionCount != 4 || 
		(layout->PartitionEntry[0].PartitionType != 6 && layout->PartitionEntry[0].PartitionType != 0xb && layout->PartitionEntry[0].PartitionType != 0xc) ||
		layout->PartitionEntry[3].PartitionType != 0x69){

			LOG_MsgError(1,"El dispositivo seleccionado no es un clauer.");
			return ERR_FORMAT_ISNOT_CLAUER;
	}

	bs=  IO_GetBytesSectorByPath(device);
	if ( bs == -1 ){
		LOG_MsgError(1,"No se pudo obtener los BytesSector.");
		free(layout);	
		return ERR_FORMAT_CANNOT_GET_BYTES_SECTOR;
	}

	LOG_Debug(1,"Obtenido bytesSector= %d", bs);
	lAux= layout->PartitionEntry[3].PartitionLength.LowPart;
	memcpy(((char *)(&lAux))+4, &layout->PartitionEntry[3].PartitionLength.HighPart, 4 );	

	nbloques= (int)(lAux/BLOCK_SIZE);

	iz.nrsv= NRSV_BLOCKS;
	iz.totalBlocks= nbloques - 1 - NRSV_BLOCKS;
	iz.currentBlock= 0;   /* Inicializamos primer bloque libre 0 en zona de objetos */
	iz.formatVersion= 1;  /* Versión 1 del formato, me lo invento yo! */

	CRYPTO_Ini();
	/* Generamos identificador random del clauer */
	if ( CRYPTO_Random (20,(unsigned char * ) iz.id)== ERR_CW_SI ){
		LOG_MsgError(1,"No puedo generar Random.");
		free(layout);
		return ERR_FORMAT_CANNOT_SEED_RNG;
	}

	/* Ciframos con la password el churro "UJI - Clauer PKI storage system" */
	res= CRYPTO_PBE_Cifrar (pwd,
		(unsigned char *)&iz.id,
		20,
		1000,
		1,
		CRYPTO_CIPHER_DES_EDE3_CBC,
		(unsigned char *)"UJI - Clauer PKI storage system",
		32,
		newIden,
		&aux );

	if  ( res != 0 ){
		LOG_MsgError(1,"Error al cifrar UJI - Clauer PKI storage system.");
		return ERR_FORMAT_CANNOT_CIPHER;
	}

	memcpy(iz.idenString, newIden, aux);
	CRYPTO_Fin();

	res = IO_Open( device, &hClauer, IO_RDWR, -1 );
	if ( res != IO_SUCCESS ) {
		LOG_Error(1,"IO_Open res= %d.", res);
		return ERR_FORMAT_OPEN_DEVICE;
	}

	lAux= layout->PartitionEntry[3].StartingOffset.LowPart;
	memcpy(((char *)(&lAux))+4, &layout->PartitionEntry[3].StartingOffset.HighPart, 4 );	

	res= IO_WriteSector(hClauer, (int)(lAux/bs), (unsigned char *) &iz);
	if ( res!= IO_SUCCESS ){
		LOG_Error(1,"WriteSector err= %d.", res);
		LOG_Error(1,"bs= %d.", bs);
		LOG_Error(1,"div= %d.", (int)(lAux/bs));
		IO_Close(hClauer);
		free(layout);
		return ERR_FORMAT_WRITE_SECTOR;
	}

	LOG_Msg(1,"Saliendo OK.");
	res = IO_UpdateProperties( hClauer );
	if ( res != IO_SUCCESS ) return ERR_FORMAT_UPDATE_PROPS;

	IO_Close(hClauer);
	free(layout);

#endif


//LINUX Version of this function
#ifdef LINUX
   
  char device4[MAX_PATH_LEN+1];
  struct infoZone iz;
  int i, devSize= 0, fp, nbloques, aux, res, option_index;
  unsigned char newIden[40];
  FILE * fd;

  LOG_Msg(LOG_TO,"Formateando!"); 
  /* Obtenemos el tamanyo del dispositivo */
  snprintf(device4,MAX_PATH_LEN+1,"%s4",device);
  fd= fopen(device4,"r");                   
	
  if ( fd == NULL ){                       
      return ERR_FORMAT_OPEN_DEVICE;
  }                                                  
	
  if ( fseek(fd, 0L, SEEK_END) != 0 ){               
      return ERR_FORMAT_CANNOT_SEEK;
  }                                                           
                                                              
  devSize= ftell(fd);                                         
  if ( devSize == -1 ){                                       
      return ERR_FORMAT_CANNOT_RETRIEVE_SIZE;
  }                                                           
                                                              
  fclose(fd);                                                 
                                                              
  /* Formateamos el dispositivo */                            
  fp= open(device4,O_RDWR);                                    

  if ( fp == -1 ){
      return ERR_FORMAT_OPEN_DEVICE;
  }                                                   
                                                      
  nbloques= devSize/BLOCK_SIZE;                       
                                                      
  iz.nrsv= NRSV_BLOCKS;                               
  iz.totalBlocks= nbloques - 1 - NRSV_BLOCKS;         
  iz.currentBlock= 0;   /* Inicializamos primer bloque libre 0 en zona de objetos */
  iz.formatVersion= 1;  /* Versiï¿½n 1 del formato, me lo invento yo! */              

  CRYPTO_Ini();
  /* Generamos identificador random del clauer */
  if ( CRYPTO_Random (20,(unsigned char * ) iz.id)== ERR_CW_SI ){
      return ERR_FORMAT_CANNOT_SEED_RNG;
  }                                                              


  /* Ciframos con la password el churro "UJI - Clauer PKI storage system" */
  res= CRYPTO_PBE_Cifrar (pwd,                                             
                          (unsigned char *)&iz.id,                          
                          20,                                               
                          1000,                                             
                          1,                                                
                          CRYPTO_CIPHER_DES_EDE3_CBC,
                          (unsigned char *)"UJI - Clauer PKI storage system",
                          32,
                          newIden,
                          &aux );

  if  ( res != 0 ){
      return ERR_FORMAT_CANNOT_CIPHER;
  }

  memcpy(iz.idenString, newIden, aux);

  CRYPTO_Fin();

  if ( write( fp, &iz,BLOCK_SIZE ) == -1 ){
      return  ERR_FORMAT_WRITE_SECTOR;
  }

  memset(&iz, 0, sizeof(iz));

  for ( i=0; i < nbloques-1; i++ ){
        write( fp, &iz, BLOCK_SIZE );
  }

  close(fp);
#endif
	return 0;
}


int FORMAT_GetSize(char * device, long long * tamanyo){

#ifdef WIN32

	BOOL result;
	HANDLE	hVolume;

	ULONG	bytesWritten;
	UCHAR	DiskExtentsBuffer[0x400];
	PVOLUME_DISK_EXTENTS DiskExtents = (PVOLUME_DISK_EXTENTS)DiskExtentsBuffer;


	hVolume = CreateFile( device,
		GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 
		NULL, OPEN_EXISTING, 0, NULL );

	if( hVolume == INVALID_HANDLE_VALUE ) {
		return ERR_FORMAT_CREATEFILE_FAILED;
	}
	else{
		result= DeviceIoControl( hVolume,
			IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
			NULL, 0,
			DiskExtents, sizeof(DiskExtentsBuffer),
			&bytesWritten, NULL ); 

		if ( result == 0 ) {
			return ERR_FORMAT_IOCTL;
		}

		result= CloseHandle(hVolume);
		if (result == 0 ){ 
			return ERR_FORMAT_CLOSE_HANDLE;
		}

		*tamanyo = DiskExtents->Extents[0].ExtentLength.LowPart;
		memcpy(((char*)tamanyo)+4, &(DiskExtents->Extents[0].ExtentLength.HighPart),4);
	}
#endif 

	return FORMAT_OK;
}



int FORMAT_GetOptimalGeometry( char * device, int *heads, int *sectors, int *cylinders ){
#ifdef WIN32 
	int error;
	long long tam;
	long long tam_temp;
	long long tam_opt=0;
	unsigned int cabezas=1;			/* 1-256 cabezas */
	unsigned int cabezas_opt;
	unsigned int sectores=1;		/* 1-64 sectores */
	unsigned int sectores_opt;
	unsigned int cilindros=1024;	/* 1024 cilindros fijos */
	unsigned int encontrado=0, bytesSector;



	error = FORMAT_GetSize( device, &tam );
	if ( error != FORMAT_OK) return ERR_FORMAT_CANNOT_RETRIEVE_SIZE;

	bytesSector= IO_GetBytesSectorByPath( (const char *)device);
	if ( bytesSector < 0 )
		return ERR_FORMAT_CANNOT_GET_BYTES_SECTOR;

	sectores = 1; cabezas=1;

	while ( (!encontrado) && (sectores <=64) ) {
		cabezas=1;

		while ( (!encontrado) && (cabezas <= 256) ) {
			tam_temp = bytesSector * cabezas * sectores * cilindros;

			if ( ( tam_temp > tam_opt ) && ( tam_temp <= tam ) ){
				tam_opt = tam_temp;
				cabezas_opt = cabezas;
				sectores_opt = sectores;

				if ( tam_temp == tam ) encontrado = 1;
			}
			cabezas++;
		}
		sectores++;

	}
	*cylinders = cilindros;
	*heads = cabezas_opt;
	*sectors = sectores_opt;

	return FORMAT_OK;
#endif
	return ERR_FORMAT_NOT_IMPLEMENTED;
}





int FORMAT_FormatearUnidadLogica( char *unidad ) {
#ifdef WIN32
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	char* comando;

	if ( unidad == NULL )
		return ERR_FORMAT_FORMATEANDO_DATOS;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

	comando = (char *) malloc (1024*sizeof(char));

	strcpy(comando, "format.com ");
	strcat(comando, unidad);

#ifdef CLOS_ACCV
	strcat(comando, " /BACKUP /FS:FAT32 /Q /X /V:ClauACCV");
#else
	strcat(comando, " /BACKUP /FS:FAT32 /Q /X /V:Clauer");
#endif

	LOG_Debug(1,"Enviando command: %s", comando);

	if( !CreateProcess( NULL,         // No module name (use command line).
		comando,                          // Command line.
		NULL,                             // Process handle not inheritable.
		NULL,                             // Thread handle not inheritable.
		FALSE,                            // Set handle inheritance to FALSE.
		CREATE_NO_WINDOW,                 // No creation flags.
		//CREATE_NEW_CONSOLE,
		NULL,                             // Use parent's environment block.
		NULL,                             // Use parent's starting directory.
		&si,                              // Pointer to STARTUPINFO structure.
		&pi )                             // Pointer to PROCESS_INFORMATION structure
		){

			return ERR_FORMAT_FORMATEANDO_DATOS;
	}
	free(comando);
	// Esperamos hasta que el proceso finalice
	WaitForSingleObject( pi.hProcess, INFINITE );

	// Cerramos los manejadores process y thread
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );


	return FORMAT_OK;
#endif
	return ERR_FORMAT_NOT_IMPLEMENTED;
}
