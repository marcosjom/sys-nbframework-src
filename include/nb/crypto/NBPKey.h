//
//  NBPKey.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/5/18.
//

#ifndef NBPKey_h
#define NBPKey_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/crypto/NBHash.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBPKey_ {
		void*			opaque;
	} STNBPKey;
	
	void NBPKey_init(STNBPKey* obj);
	void NBPKey_release(STNBPKey* obj);
	//
	BOOL NBPKey_isCreated(const STNBPKey* obj);
	BOOL NBPKey_createEmpty(STNBPKey* obj);
	BOOL NBPKey_createFromOther(STNBPKey* obj, STNBPKey* other);
	BOOL NBPKey_createFromDERBytes(STNBPKey* obj, const void* data, UI32 dataSz); //binary
	BOOL NBPKey_createFromDERFile(STNBPKey* obj, const char* filepath); //binary
	BOOL NBPKey_createFromPEMBytes(STNBPKey* obj, const char* strPKey, const UI32 strPKeySz); //base64-text
	BOOL NBPKey_createFromPEMFile(STNBPKey* obj, const char* filepath); //base64-text
	//
	BOOL NBPKey_getAsDERString(const STNBPKey* obj, STNBString* dstStr); //binary
	BOOL NBPKey_getAsPEMString(const STNBPKey* obj, STNBString* dstStr); //base64-text
	//Signature
	BOOL NBPKey_concatBytesSignatureSha1(STNBPKey* obj, const void* data, const SI32 dataSz, STNBString* dstBin);
	BOOL NBPKey_concatBytesSignatureSha256(STNBPKey* obj, const void* data, const SI32 dataSz, STNBString* dstBin);
	//Encryption
	BOOL NBPKey_concatEncryptedBytes(STNBPKey* obj, const void* data, const SI32 dataSz, STNBString* dstEnc);
	BOOL NBPKey_concatDecryptedBytes(STNBPKey* obj, const void* data, const SI32 dataSz, STNBString* dstEnc);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBPKey_h */
