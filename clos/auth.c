
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

#include "auth.h"

#include "log.h"


#include "block.h"

#include <string.h>

#include <CRYPTOWrapper/CRYPTOWrap.h>



/*! \brief It verifies the clauer's passphrase.
 *
 * \param hClauer
 *        A previously opened clauer handle.
 *
 * \param passphrase
 *        The passphrase to verify.
 *
 * \retval CLOS_SUCCESS
 *         The passphrase is ok.
 *
 * \retval ERR_AUTH_INVALID_PASSPHRASE
 *         The passphrase isn't ok.
 *
 * \retval ERR_INVALID_PARAMETER
 *         One ore more of the parameters are invalid.
 *
 * \retval ERR_CLOS
 *         Error
 */

int AUTH_VerifyClauerPassphrase ( clauer_handle_t hClauer, char *passphrase)
{

  block_info_t ib;
  char clearIdenString[40];
  int clearIdenStringSize, ret;

  LOG_BeginFunc(2);

  if ( ! hClauer ) {
	LOG_EndFunc(2, ERR_INVALID_PARAMETER);
    return ERR_INVALID_PARAMETER;
  }

  if ( ! passphrase ) {
	LOG_EndFunc(2, ERR_INVALID_PARAMETER);
    return ERR_INVALID_PARAMETER;
  }

  /* Read the info block to get the salt */

  if ( IO_ReadInfoBlock ( hClauer, &ib ) != 0 ) {
	LOG_EndFunc(2, ERR_CLOS);
    return ERR_CLOS;  
  }

  
  if ( CRYPTO_PBE_Descifrar(passphrase, 
			    (unsigned char *) (ib.id), 
			    20,
			    1000,
			    1, 
			    CRYPTO_CIPHER_DES_EDE3_CBC, 
			    (unsigned char *) (ib.idenString), 
			    40, 
			    (unsigned char *) clearIdenString, 
			    &clearIdenStringSize) != 0 ) 
    {
	  LOG_EndFunc(2, ERR_CLOS);
      return ERR_AUTH_INVALID_PASSPHRASE;
    }
    
	LOG_Msg(2, "Fin Descifrar PBE...");

  if ( strncmp(clearIdenString, "UJI - Clauer PKI storage system", 31) != 0 )
    ret = ERR_AUTH_INVALID_PASSPHRASE;
  else
    ret = CLOS_SUCCESS;

  
  LOG_EndFunc(2, ret);
    
  return ret;

}

/* Given a pid returns 0 if it is not executed by a member 
of Administrators group and returns a 1 if it is */
int AUTH_IsPidAdmin(int pid)
{
 #ifdef WIN32 
	int isAdmo= FALSE;
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	PSID AdministratorsGroup;
	HANDLE hProcess, hToken, hImpToken;
	
	// Here we check the register variable:
	DWORD tamValue;
    DWORD allow_user_format=0;
    DWORD type;
    HKEY hKey = NULL;

	LOG_Debug(1,"pid= %d", pid);
	
	if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Universitat Jaume I\\Projecte Clauer", 
		               0, KEY_READ, &hKey) != ERROR_SUCCESS ) {
			
			LOG_Debug(1,"Error Abriendo el registrooolll LastError= %d", GetLastError());
			allow_user_format=0;
    }

	tamValue= sizeof(allow_user_format);
    RegQueryValueEx(hKey, "ALLOW_USER_FORMAT", NULL, &type, (LPBYTE)&allow_user_format, &tamValue);

	LOG_Debug(1,"Obtenido ALLOW_USER_FORMAT= %d", allow_user_format);
	
	if ( allow_user_format==1 )
		return TRUE;

	if ( pid == -1 ){
	  return FALSE;
	}

    if ( hKey )
        RegCloseKey(hKey);

	if ( allow_user_format==0 ){ 
		hProcess= OpenProcess( PROCESS_ALL_ACCESS, FALSE, pid );
		if (! hProcess ){
			LOG_Msg(1, "ERROR handle del proceso");
		}

		if ( ! OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken ) ){
			LOG_Msg(1, "ERROR handle del token");
			LOG_Debug(1, "GetLastError= %d", GetLastError());
		}


		/* Impersonamos el token */
		if ( ! DuplicateToken(hToken, SecurityDelegation, &hImpToken  )){
			LOG_Debug(1, "Error Impersonando el token =%d", GetLastError());
		}

		/* Inicializamos el sid del grupo de administradores */
		if ( ! AllocateAndInitializeSid(&NtAuthority,
			2,
			SECURITY_BUILTIN_DOMAIN_RID,
			DOMAIN_ALIAS_RID_ADMINS,
			0, 0, 0, 0, 0, 0,
			&AdministratorsGroup) )
		{
			LOG_Msg(1, "ERROR AL ALlOCATE INITIALIZESID");
		}

		if ( ! CheckTokenMembership( hImpToken, AdministratorsGroup, &isAdmo) ){
			LOG_Debug(1, "NO ES ADMIN pid=%d", pid);
			LOG_Debug(1, "GetLastError= %d", GetLastError());
		}
		else{
			LOG_Debug(1, "ES ADMIN=%d", isAdmo);
			LOG_Debug(1, "pid=%d", pid);
		}

		FreeSid(AdministratorsGroup);
	}

	return isAdmo;
#else 
  return 0;
#endif
}


/* Given a socket get the port of the other side, its 
pid if it is a localhost connection and if it pertains to an 
administrator group*/
int AUTH_IsPortPidAdmin(int port){
	
	/*
	 * First we must get the pid of the program who is 
	 * binded on that port.
	 *
	 */ 	
	  
	 /* From netstatp code (sysinternals) */


	
	
	return 0;
}
