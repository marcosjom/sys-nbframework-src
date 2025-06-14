//
//  NBKeychain.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/21/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/crypto/NBKeychain.h"
//
#include "nb/core/NBMemory.h"
#include "nb/files/NBFile.h"

#ifdef _WIN32
//#	define WIN32_LEAN_AND_MEAN
#include <windows.h> 
#include <Lmcons.h>		//for UNLEN
#include <winbase.h>	//for GetUserNameA
#include <dpapi.h>		//for DATA_BLOB, CryptProtectData and CryptUnprotectData
#endif

#ifdef _WIN32
#	pragma comment(lib, "Crypt32.lib")
#endif

typedef struct STNBKeychainOpq_ {
	STNBFilesystem* fs;
	STNBKeychainItf	itf;
} STNBKeychainOpq;

void NBKeychain_init(STNBKeychain* obj){
	STNBKeychainOpq* opq = obj->opaque = NBMemory_allocType(STNBKeychainOpq);
	NBMemory_setZeroSt(*opq, STNBKeychainOpq);
	opq->fs		= NULL;
}

void NBKeychain_release(STNBKeychain* obj){
	if(obj->opaque != NULL){
		STNBKeychainOpq* opq = (STNBKeychainOpq*)obj->opaque;
		opq->fs = NULL;
		{
			if(opq->itf.destroy != NULL){
				(opq->itf.destroy)(&opq->itf.param);
				opq->itf.destroy = NULL;
			}
		}
		NBMemory_free(obj->opaque);
		obj->opaque = NULL;
	}
}

//

BOOL NBKeychain_setFilesystem(STNBKeychain* obj, STNBFilesystem* fs){
	BOOL r = FALSE;
	STNBKeychainOpq* opq = (STNBKeychainOpq*)obj->opaque;
	if(opq != NULL){
		opq->fs = fs;
		r = TRUE;
	}
	return r;
}

BOOL NBKeychain_setItf(STNBKeychain* obj, STNBKeychainItf* itf){
	BOOL r = FALSE;
	STNBKeychainOpq* opq = (STNBKeychainOpq*)obj->opaque;
	if(opq != NULL){
		//Detroy prev
		if(opq->itf.destroy != NULL){
			(*opq->itf.destroy)(&opq->itf.param);
		}
		//Set new
		if(itf != NULL){
			opq->itf = *itf;
		} else {
			NBMemory_setZeroSt(opq->itf, STNBKeychainItf);
		}
		//Create
		if(opq->itf.create != NULL){
			(*opq->itf.create)(&opq->itf.param);
		}
	}
	return r;
}

//Generic password

BOOL NBKeychain_addPassword(STNBKeychain* obj, const char* account, const char* passwd, const BOOL allowDevicesSync){
	BOOL r = FALSE;
	STNBKeychainOpq* opq = (STNBKeychainOpq*)obj->opaque;
	if(opq != NULL){
		if(opq->itf.addPassword != NULL){
			r = (*opq->itf.addPassword)(opq->itf.param, account, passwd, allowDevicesSync);
		} else {
			STNBString fname;
			NBString_init(&fname);
			NBString_concat(&fname, "nbKey_");
			NBString_concat(&fname, account);
			{
				NBString_replace(&fname, "/", "__01__");
				NBString_replace(&fname, "\\", "__02__");
				NBString_replace(&fname, "?", "__03__");
				NBString_replace(&fname, "%", "__04__");
				NBString_replace(&fname, "*", "__05__");
				NBString_replace(&fname, ":", "__06__");
				NBString_replace(&fname, "|", "__07__");
				NBString_replace(&fname, "\"", "__08__");
				NBString_replace(&fname, "<", "__09__");
				NBString_replace(&fname, ">", "__10__");
				NBString_replace(&fname, ".", "__11__");
				NBString_replace(&fname, " ", "__12__");
			}
			NBString_concat(&fname, ".bin");
			if(opq->fs != NULL){
#				ifdef _WIN32
				{
					char uname[UNLEN + 1];
					DWORD unameLen = UNLEN + 1;
					STNBString fpath;
					NBString_initWithStr(&fpath, "nbKeys\\");
					if (!GetUserNameA(uname, &unameLen)) {
						PRINTF_ERROR("NBKeychain, GetUserNameA failed.\n");
					} else {
						NBString_concat(&fpath, uname);
						NBString_concat(&fpath, "\\");
						NBFilesystem_createFolderPathAtRoot(opq->fs, ENNBFilesystemRoot_Lib, fpath.str);
						NBString_concat(&fpath, fname.str);
						{
							DATA_BLOB bIn, bOut;
							bIn.cbData = NBString_strLenBytes(passwd) + 1;
							bIn.pbData = (BYTE*)passwd;
							if (!CryptProtectData(&bIn, NULL, NULL, NULL, NULL, 0, &bOut)) {
								PRINTF_ERROR("NBKeychain, CryptProtectData failed.\n");
							} else {
								if (!NBFilesystem_writeToFilepathAtRoot(opq->fs, ENNBFilesystemRoot_Lib, fpath.str, bOut.pbData, bOut.cbData)) {
									PRINTF_ERROR("NBKeychain, NBFilesystem_writeToFilepathAtRoot(lib + '%s') failed.\n", fpath.str);
								} else {
									r = true;
								}
								LocalFree(bOut.pbData);
								
							}
						}
					}
					NBString_release(&fpath);
				}
#				else
				{
					STNBFileRef file = NBFile_alloc(NULL);
					if(!NBFilesystem_openAtRoot(opq->fs, ENNBFilesystemRoot_Lib, fname.str, ENNBFileMode_Write, file)){
						//
					} else {
						NBFile_lock(file);
						NBFile_write(file, passwd, NBString_strLenBytes(passwd));
						PRINTF_WARNING("Password saved with no KEYCHAIN.\n");
						NBFile_unlock(file);
						r = TRUE;
					}
					NBFile_release(&file);
				}
#				endif
			}
			NBString_release(&fname);
		}
	}
	return r;
}

BOOL NBKeychain_getPassword(STNBKeychain* obj, const char* account, STNBString* dst){
	BOOL r = FALSE;
	STNBKeychainOpq* opq = (STNBKeychainOpq*)obj->opaque;
	if(opq != NULL){
		if(opq->itf.getPassword != NULL){
			r = (*opq->itf.getPassword)(opq->itf.param, account, dst);
		} else {
			STNBString fname;
			NBString_init(&fname);
			NBString_concat(&fname, "nbKey_");
			NBString_concat(&fname, account);
			{
				NBString_replace(&fname, "/", "__01__");
				NBString_replace(&fname, "\\", "__02__");
				NBString_replace(&fname, "?", "__03__");
				NBString_replace(&fname, "%", "__04__");
				NBString_replace(&fname, "*", "__05__");
				NBString_replace(&fname, ":", "__06__");
				NBString_replace(&fname, "|", "__07__");
				NBString_replace(&fname, "\"", "__08__");
				NBString_replace(&fname, "<", "__09__");
				NBString_replace(&fname, ">", "__10__");
				NBString_replace(&fname, ".", "__11__");
				NBString_replace(&fname, " ", "__12__");
			}
			NBString_concat(&fname, ".bin");
			if(opq->fs != NULL){
#				ifdef _WIN32
				{
					char uname[UNLEN + 1];
					DWORD unameLen = UNLEN + 1;
					STNBString fpath;
					NBString_initWithStr(&fpath, "nbKeys\\");
					if (!GetUserNameA(uname, &unameLen)) {
						PRINTF_ERROR("NBKeychain, GetUserNameA failed.\n");
					} else {
						NBString_concat(&fpath, uname);
						NBString_concat(&fpath, "\\");
						NBString_concat(&fpath, fname.str);
						{
							STNBString enc;
							NBString_init(&enc);
							if (!NBFilesystem_readFromFilepathAtRoot(opq->fs, ENNBFilesystemRoot_Lib, fpath.str, &enc)) {
								PRINTF_ERROR("NBKeychain, NBFilesystem_writeToFilepathAtRoot(lib + '%s') failed.\n", fpath.str);
							} else {
								DATA_BLOB bIn, bOut;
								bIn.cbData = enc.length + 1;
								bIn.pbData = (BYTE*)enc.str;
								if (!CryptUnprotectData(&bIn, NULL, NULL, NULL, NULL, 0, &bOut)) {
									PRINTF_ERROR("NBKeychain, CryptProtectData failed.\n");
								} else {
									if(dst != NULL){
										NBString_setBytes(dst, (const char *)bOut.pbData, bOut.cbData);
									}
									r = true;
									LocalFree(bOut.pbData);
								}
							}
							NBString_release(&enc);
						}
					}
					NBString_release(&fpath);
				}
#				else
				{
					STNBFileRef file = NBFile_alloc(NULL);
					if(!NBFilesystem_openAtRoot(opq->fs, ENNBFilesystemRoot_Lib, fname.str, ENNBFileMode_Read, file)){
						//
					} else {
						SI32 bytesTotal = 0;
						NBFile_lock(file);
						PRINTF_WARNING("Password loaded with no KEYCHAIN.\n");
						if(dst != NULL){
							NBString_empty(dst);
						}
						{
							BYTE buff[1024];
							while(TRUE){
								const SI32 read = NBFile_read(file, buff, sizeof(buff));
								if(read <= 0){
									break;
								} else {
									bytesTotal += read;
									if(dst != NULL){
										NBString_concatBytes(dst, (const char*)buff, read);
									}
								}
							}
						}
						NBFile_unlock(file);
						r = (bytesTotal > 0 ? TRUE : FALSE);
					}
					NBFile_release(&file);
				}
#				endif
			}
			NBString_release(&fname);
		}
	}
	return r;
}


BOOL NBKeychain_removePassword(STNBKeychain* obj, const char* account){
	BOOL r = FALSE;
	STNBKeychainOpq* opq = (STNBKeychainOpq*)obj->opaque;
	if(opq != NULL){
		if(opq->itf.removePassword != NULL){
			r = (*opq->itf.removePassword)(opq->itf.param, account);
		} else if(opq->fs != NULL){
			STNBString fname;
			NBString_init(&fname);
			NBString_concat(&fname, "nbKey_");
			NBString_concat(&fname, account);
			{
				NBString_replace(&fname, "/", "__01__");
				NBString_replace(&fname, "\\", "__02__");
				NBString_replace(&fname, "?", "__03__");
				NBString_replace(&fname, "%", "__04__");
				NBString_replace(&fname, "*", "__05__");
				NBString_replace(&fname, ":", "__06__");
				NBString_replace(&fname, "|", "__07__");
				NBString_replace(&fname, "\"", "__08__");
				NBString_replace(&fname, "<", "__09__");
				NBString_replace(&fname, ">", "__10__");
				NBString_replace(&fname, ".", "__11__");
				NBString_replace(&fname, " ", "__12__");
			}
			NBString_concat(&fname, ".bin");
#			ifdef _WIN32
			{
				char uname[UNLEN + 1];
				DWORD unameLen = UNLEN + 1;
				STNBString fpath;
				NBString_initWithStr(&fpath, "nbKeys\\");
				if (!GetUserNameA(uname, &unameLen)) {
					PRINTF_ERROR("NBKeychain, GetUserNameA failed.\n");
				} else {
					NBString_concat(&fpath, uname);
					NBString_concat(&fpath, "\\");
					NBString_concat(&fpath, fname.str);
					NBFilesystem_deleteFileAtRoot(opq->fs, ENNBFilesystemRoot_Lib, fpath.str);
				}
				NBString_release(&fpath);
			}
#			else
			{
				NBFilesystem_deleteFileAtRoot(opq->fs, ENNBFilesystemRoot_Lib, fname.str);
				r = TRUE;
			}
#			endif
			NBString_release(&fname);
		}
	}
	return r;
}

//Public certificates

/*BOOL NBKeychain_addCert(STNBKeychain* obj, const STNBX509* cert, const BOOL allowDevicesSync){
	BOOL r = FALSE;
	STNBKeychainOpq* opq = (STNBKeychainOpq*)obj->opaque;
	if(opq != NULL){
		if(opq->itf.addCert != NULL){
			r = (*opq->itf.addCert)(opq->itfParam, cert, allowDevicesSync);
		}
	}
	return r;
}

BOOL NBKeychain_getCertByCN(STNBKeychain* obj, const char* commonName, STNBX509* dst){
	BOOL r = FALSE;
	STNBKeychainOpq* opq = (STNBKeychainOpq*)obj->opaque;
	if(opq != NULL){
		if(opq->itf.getCertByCN != NULL){
			r = (*opq->itf.getCertByCN)(opq->itfParam, commonName, dst);
		}
	}
	return r;
}

BOOL NBKeychain_getCertsIssuedByCert(STNBKeychain* obj, const STNBX509* issuer, STNBArray* dst){ //STNBX509
	BOOL r = FALSE;
	STNBKeychainOpq* opq = (STNBKeychainOpq*)obj->opaque;
	if(opq != NULL){
		if(opq->itf.getCertsIssuedByCert != NULL){
			r = (*opq->itf.getCertsIssuedByCert)(opq->itfParam, issuer, dst);
		}
	}
	return r;
}

BOOL NBKeychain_removeCertByCN(STNBKeychain* obj, const char* commonName){
	BOOL r = FALSE;
	STNBKeychainOpq* opq = (STNBKeychainOpq*)obj->opaque;
	if(opq != NULL){
		if(opq->itf.removeCertByCN != NULL){
			r = (*opq->itf.removeCertByCN)(opq->itfParam, commonName);
		}
	}
	return r;
}*/
