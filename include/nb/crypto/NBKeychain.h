//
//  NBKeychain.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/21/18.
//

#ifndef NBKeychain_h
#define NBKeychain_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/crypto/NBHash.h"
#include "nb/crypto/NBX509.h"
#include "nb/crypto/NBPKey.h"
#include "nb/files/NBFilesystem.h"

#ifdef __cplusplus
extern "C" {
#endif
	
typedef struct STNBKeychainItf_ {
	void* param;
	void (*create)(void** obj);
	void (*destroy)(void** obj);
	//Generic password
	BOOL (*addPassword)(void* obj, const char* account, const char* passwd, const BOOL allowDevicesSync);
	BOOL (*getPassword)(void* obj, const char* account, STNBString* dst);
	BOOL (*removePassword)(void* obj, const char* account);
	//Public certificates
	/*BOOL (*addCert)(void* obj, const STNBX509* cert, const BOOL allowDevicesSync);
	BOOL (*getCertByCN)(void* obj, const char* label, STNBX509* dst);
	BOOL (*getCertsIssuedByCert)(void* obj, const STNBX509* issuer, STNBArray* dst); //STNBX509
	BOOL (*removeCertByCN)(void* obj, const char* cn);*/
} STNBKeychainItf;

typedef struct STNBKeychain_ {
	void*			opaque;
} STNBKeychain;

void NBKeychain_init(STNBKeychain* obj);
void NBKeychain_release(STNBKeychain* obj);

//
BOOL NBKeychain_setFilesystem(STNBKeychain* obj, STNBFilesystem* fs);
BOOL NBKeychain_setItf(STNBKeychain* obj, STNBKeychainItf* itf);

//Generic password
BOOL NBKeychain_addPassword(STNBKeychain* obj, const char* account, const char* passwd, const BOOL allowDevicesSync);
BOOL NBKeychain_getPassword(STNBKeychain* obj, const char* account, STNBString* dst);
BOOL NBKeychain_removePassword(STNBKeychain* obj, const char* account);

//Public certificates
/*BOOL NBKeychain_addCert(STNBKeychain* obj, const STNBX509* cert, const BOOL allowDevicesSync);
BOOL NBKeychain_getCertByCN(STNBKeychain* obj, const char* label, STNBX509* dst);
BOOL NBKeychain_getCertsIssuedByCert(STNBKeychain* obj, const STNBX509* issuer, STNBArray* dst); //STNBX509
BOOL NBKeychain_removeCertByCN(STNBKeychain* obj, const char* cn);*/
//

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBKeychain_h */
