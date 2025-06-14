//
//  NBKeychainApple.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/21/18.
//

#ifndef NBKeychainApple_h
#define NBKeychainApple_h

#include "nb/NBFrameworkDefs.h"
#include "nb/crypto/NBKeychain.h"

#ifdef __cplusplus
extern "C" {
#endif
	
typedef struct STNBKeychainApple_ {
	void*	opaque;
} STNBKeychainApple;

STNBKeychainItf NBKeychainApple_getItf(void);

void NBKeychainApple_init(STNBKeychainApple* obj);
void NBKeychainApple_release(STNBKeychainApple* obj);

//Generic password
BOOL NBKeychainApple_addPassword(STNBKeychainApple* obj, const char* account, const char* passwd, const BOOL allowDevicesSync);
BOOL NBKeychainApple_getPassword(STNBKeychainApple* obj, const char* account, STNBString* dst);
BOOL NBKeychainApple_removePassword(STNBKeychainApple* obj, const char* account);

//Public certificates
/*BOOL NBKeychainApple_addCert(STNBKeychainApple* obj, const STNBX509* cert, const BOOL allowDevicesSync);
BOOL NBKeychainApple_getCertByCN(STNBKeychainApple* obj, const char* label, STNBX509* dst);
BOOL NBKeychainApple_getCertsIssuedByCert(STNBKeychainApple* obj, const STNBX509* issuer, STNBArray* dst);
BOOL NBKeychainApple_removeCertByCN(STNBKeychainApple* obj, const char* cn);*/

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBKeychainApple_h */
