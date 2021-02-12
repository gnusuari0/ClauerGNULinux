
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

#ifndef _CLAUER_FORMAT_H
#define _CLAUER_FORMAT_H

#ifdef WIN32 
//#include <windows.h>  //Conflicto de compilacion entre winsock.h y winsock2.h  
//#include <winbase.h>
#include <tchar.h>
#endif

#include "func.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/* Data structures */
typedef struct _PARTITION {
        long int cyl_ini;
        long int head_ini;
        long int sect_ini;

        long int cyl_fin;
        long int head_fin;
        long int sect_fin;

        long int id;

        long int rel_sectors;
        long int num_sectors;
} PARTITION;


typedef struct _clauer_partition_info{
 //All in bytes 
 long long size;
 int type; 
}CLAUER_PARTITION_INFO;

typedef struct _clauer_layout_info{
 int npartitions;
 CLAUER_PARTITION_INFO cpi[4];
}CLAUER_LAYOUT_INFO;


typedef struct infoZone{
 	  char idenString[40];
 	  unsigned char id[20];
 	  int nrsv;
 	  int currentBlock;
 	  int totalBlocks;
 	  int formatVersion;
 	  char info[10240-76];
}INFO_ZONE;

/* Functions */
int FORMAT_CreateClauer(char * device, char porcentaje);
int FORMAT_GetSize(char * device, long long * tamanyo);
int FORMAT_GetClauerLayout(char * device, CLAUER_LAYOUT_INFO * cli);
int FORMAT_GetOptimalGeometry( char * device, int *heads, int *sectors, int *cylinders );
int FORMAT_CreateClauer(char * device, char percent);
int FORMAT_FormatClauerData( char * device );
int FORMAT_FormatearUnidadLogica( char *unidad );
int FORMAT_FormatClauerCrypto( char * device, char * pwd );

/* External functions */
extern int IO_GetBytesSectorByPath( const char * device );

#define NRSV_BLOCKS 7

#endif
