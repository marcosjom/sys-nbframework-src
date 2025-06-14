//
//  NBPkcs12.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/21/18.
//

#ifndef NBPkcs12_h
#define NBPkcs12_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/crypto/NBHash.h"
#include "nb/crypto/NBX509.h"
#include "nb/crypto/NBPKey.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBPkcs12_ {
		void*			opaque;
	} STNBPkcs12;
	
	void NBPkcs12_init(STNBPkcs12* obj);
	void NBPkcs12_release(STNBPkcs12* obj);
	//
	BOOL NBPkcs12_isCreated(const STNBPkcs12* obj);
	BOOL NBPkcs12_createEmpty(STNBPkcs12* obj);
	BOOL NBPkcs12_createFromDERBytes(STNBPkcs12* obj, const void* strDER, const UI32 strDERSz); //binary
	BOOL NBPkcs12_createFromDERFile(STNBPkcs12* obj, const char* filepath); //binary
	BOOL NBPkcs12_createBundle(STNBPkcs12* obj, const char* name, const STNBPKey* pkey, const STNBX509* cert, const STNBX509* caCertsStack, const UI32 caCertsStackSz, const char* pass);
	//
	BOOL NBPkcs12_getCertAndKey(STNBPkcs12* obj, STNBPKey* dstkey, STNBX509* dstcert, const char* pass);
	//
	BOOL NBPkcs12_getAsDERString(const STNBPkcs12* obj, STNBString* dstStr); //binary
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBPkcs12_h */
