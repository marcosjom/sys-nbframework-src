#ifndef NB_X509_H
#define NB_X509_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/crypto/NBHash.h"
#include "nb/crypto/NBX500Name.h"
#include "nb/crypto/NBX509Req.h"
#include "nb/crypto/NBPKey.h"
#include "nb/files/NBFile.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	//X509
	typedef struct STNBX509_ {
		void*			opaque;
	} STNBX509;
	
	void NBX509_init(STNBX509* obj);
	void NBX509_release(STNBX509* obj);
	//
	BOOL NBX509_isCreated(const STNBX509* obj);
	BOOL NBX509_createEmpty(STNBX509* obj);
	BOOL NBX509_createFromOther(STNBX509* obj,STNBX509* other);
	BOOL NBX509_createFromDERBytes(STNBX509* obj, const void* data, UI32 dataSz); //binary
	BOOL NBX509_createFromDERFile(STNBX509* obj, STNBFileRef file); //binary
	BOOL NBX509_createFromPEMBytes(STNBX509* obj, const char* strX509, const UI32 strX509Sz); //base64-text
	BOOL NBX509_createFromPEMFile(STNBX509* obj, STNBFileRef file); //base64-text
	BOOL NBX509_createSelfSigned(STNBX509* obj, const SI32 bits, const UI32 days, const STNBX500NamePair* namePairs, const UI32 namePairsSz, STNBPKey* dstPrivKey);
	BOOL NBX509_createSelfSignedFrom(STNBX509* obj, const SI32 bits, const UI32 days, const STNBX500NamePair* namePairs, const UI32 namePairsSz, STNBX509* baseCert, STNBPKey* baseKey, STNBPKey* dstPrivKey);
	BOOL NBX509_createFromRequest(STNBX509* obj, const STNBX509Req* csrReq, const STNBX509* caCert, const STNBPKey* caPrivKey, const UI32 bits, const UI32 days, const void* serial, const UI32 serialSz);
	//
	BOOL NBX509_getAsDERString(const STNBX509* obj, STNBString* dstStr); //binary
	BOOL NBX509_getAsPEMString(const STNBX509* obj, STNBString* dstStr); //base64-text
	//Serial (unique per certificate signed by CA)
	BOOL NBX509_concatSerial(const STNBX509* obj, STNBString* dstStr);
	BOOL NBX509_concatSerialHex(const STNBX509* obj, STNBString* dstStr);
	BOOL NBX509_concatNotBeforeStr(const STNBX509* obj, STNBString* dstStr);
	BOOL NBX509_concatNotAfterStr(const STNBX509* obj, STNBString* dstStr);
	//Names
	BOOL NBX509_concatSubjectFullLine(const STNBX509* obj, STNBString* dstStr);
	BOOL NBX509_concatSubjectCommName(const STNBX509* obj, STNBString* dstStr);
	BOOL NBX509_concatSubjectOrgName(const STNBX509* obj, STNBString* dstStr);
	BOOL NBX509_concatSubjectOrgUnitName(const STNBX509* obj, STNBString* dstStr);
	BOOL NBX509_concatSubjectCountryName(const STNBX509* obj, STNBString* dstStr);
	BOOL NBX509_concatSubjectStateName(const STNBX509* obj, STNBString* dstStr);
	BOOL NBX509_concatSubjectLocatityName(const STNBX509* obj, STNBString* dstStr);
	//
	BOOL NBX509_concatIssuerFullLine(const STNBX509* obj, STNBString* dstStr);
	BOOL NBX509_concatIssuerDER(const STNBX509* obj, STNBString* dstStr);
	BOOL NBX509_concatIssuerCommName(const STNBX509* obj, STNBString* dstStr);
	BOOL NBX509_concatIssuerOrgName(const STNBX509* obj, STNBString* dstStr);
	BOOL NBX509_concatIssuerOrgUnitName(const STNBX509* obj, STNBString* dstStr);
	BOOL NBX509_concatIssuerCountryName(const STNBX509* obj, STNBString* dstStr);
	BOOL NBX509_concatIssuerStateName(const STNBX509* obj, STNBString* dstStr);
	BOOL NBX509_concatIssuerLocatityName(const STNBX509* obj, STNBString* dstStr);
	//Certificate fingerprint
	BOOL NBX509_concatFingerprint(const STNBX509* obj, const ENNBHashType type, STNBString* dstStr);
	BOOL NBX509_concatFingerprintHex(const STNBX509* obj, const ENNBHashType type, STNBString* dstStr);
	BOOL NBX509_concatFingerprintBase64(const STNBX509* obj, const ENNBHashType type, STNBString* dstStr);
	//Public key pin
	BOOL NBX509_concatPubKeyHash(const STNBX509* obj, const ENNBHashType type, STNBString* dstStr);
	BOOL NBX509_concatPubKeyHashHex(const STNBX509* obj, const ENNBHashType type, STNBString* dstStr);
	BOOL NBX509_concatPubKeyHashBase64(const STNBX509* obj, const ENNBHashType type, STNBString* dstStr);
	//Certificate body (for signature verification)
	BOOL NBX509_concatTbsDER(const STNBX509* obj, STNBString* dstStr);
	BOOL NBX509_concatTbsDERHash(const STNBX509* obj, const ENNBHashType type, STNBString* dstStr);
	//Signature
	BOOL NBX509_isSignedBy(const STNBX509* obj, const STNBX509* signer);
	BOOL NBX509_isValidSignature(const STNBX509* obj, const void* data, const SI32 dataSz, const void* singBin, const SI32 signBinSz);
	//Encryption
	BOOL NBX509_concatEncryptedBytes(STNBX509* obj, const void* data, const SI32 dataSz, STNBString* dstEnc);
	BOOL NBX509_concatDecryptedBytes(STNBX509* obj, const void* data, const SI32 dataSz, STNBString* dstEnc);
#ifdef __cplusplus
} //extern "C"
#endif

#endif
