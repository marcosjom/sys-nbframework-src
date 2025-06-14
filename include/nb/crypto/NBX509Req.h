#ifndef NB_X509_REQ_H
#define NB_X509_REQ_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBStructMap.h"
#include "nb/crypto/NBHash.h"
#include "nb/crypto/NBX500Name.h"
#include "nb/crypto/NBPKey.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	//X509 definition

	typedef struct STNBX509Def_ {
		STNBX500NamePair* subject;	//UI32 (pairs, "C", "...", "ST", "...", "L", "...", "CN", "...", "O", "...", "OU", "...")
		UI32		subjectSz;
		UI32		bits;
		UI32		daysValid;
	} STNBX509Def;

	const STNBStructMap* NBX509Def_getSharedStructMap(void);

	//X509Req

	typedef struct STNBX509Req_ {
		void*			opaque;
	} STNBX509Req;
	
	void NBX509Req_init(STNBX509Req* obj);
	void NBX509Req_release(STNBX509Req* obj);
	//
	BOOL NBX509Req_canCreateFromDERBytes(const void* data, const UI32 dataSz);
	//
	BOOL NBX509Req_isCreated(const STNBX509Req* obj);
	BOOL NBX509Req_createEmpty(STNBX509Req* obj);
	BOOL NBX509Req_createFromDERBytes(STNBX509Req* obj, const void* data, UI32 dataSz); //binary
	BOOL NBX509Req_createFromDERFile(STNBX509Req* obj, const char* filepath); //binary
	BOOL NBX509Req_createFromPEMBytes(STNBX509Req* obj, const char* strX509, const UI32 strX509Sz); //base64-text
	BOOL NBX509Req_createFromPEMFile(STNBX509Req* obj, const char* filepath); //base64-text
	BOOL NBX509Req_createSelfSigned(STNBX509Req* obj, const UI32 bits, const STNBX500NamePair* namePairs, const UI32 namePairsSz, STNBPKey* dstPrivKey);
	//
	BOOL NBX509Req_getAsDERString(const STNBX509Req* obj, STNBString* dstStr); //binary
	BOOL NBX509Req_getAsPEMString(const STNBX509Req* obj, STNBString* dstStr); //base64-text
	//Certificate fingerprint
	BOOL NBX509Req_concatFingerprint(const STNBX509Req* obj, const ENNBHashType type, STNBString* dstStr);
	BOOL NBX509Req_concatFingerprintHex(const STNBX509Req* obj, const ENNBHashType type, STNBString* dstStr);
	BOOL NBX509Req_concatFingerprintBase64(const STNBX509Req* obj, const ENNBHashType type, STNBString* dstStr);
	//Public key pin
	BOOL NBX509Req_concatPubKeyHash(const STNBX509Req* obj, const ENNBHashType type, STNBString* dstStr);
	BOOL NBX509Req_concatPubKeyHashHex(const STNBX509Req* obj, const ENNBHashType type, STNBString* dstStr);
	BOOL NBX509Req_concatPubKeyHashBase64(const STNBX509Req* obj, const ENNBHashType type, STNBString* dstStr);
	//
	BOOL NBX509Req_concatSubject(const STNBX509Req* obj, STNBString* dstStr);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
