
#include "nb/NBFrameworkPch.h"
#include "nb/crypto/NBX509Req.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBMngrStructMaps.h"
#include "nb/crypto/NBSha1.h"
#include "nb/crypto/NBBase64.h"

//SSL
#ifdef NB_ENABLE_OPENSSL
#	include "openssl/rand.h"
#	include "openssl/ssl.h"
#	include "openssl/err.h"
//Create certificates
#	include "openssl/pem.h"
#	include "openssl/conf.h"
#	include "openssl/x509v3.h"
#endif

//X509 definition

STNBStructMapsRec NBX509Def_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBX509Def_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBX509Def_sharedStructMap);
	if(NBX509Def_sharedStructMap.map == NULL){
		STNBX509Def s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBX509Def);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addPtrToArrayOfStructM(map, s, subject, subjectSz, ENNBStructMapSign_Unsigned, NBX500NamePair_getSharedStructMap());
		NBStructMap_addUIntM(map, s, bits);
		NBStructMap_addUIntM(map, s, daysValid);
		NBX509Def_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBX509Def_sharedStructMap);
	return NBX509Def_sharedStructMap.map;
}

//X509Req

typedef struct STNBX509ReqOpq_ {
#	ifdef NB_ENABLE_OPENSSL
	X509_REQ*		x509;
#	else
	SI32			dummy;
#	endif
} STNBX509ReqOpq;

void NBX509Req_init(STNBX509Req* obj){
	obj->opaque	= NULL;
}

void NBX509Req_release(STNBX509Req* obj){
	if(obj->opaque != NULL){
		STNBX509ReqOpq* opq = (STNBX509ReqOpq*)obj->opaque;
#		ifdef NB_ENABLE_OPENSSL
		if(opq->x509 != NULL){
			X509_REQ_free(opq->x509);
			opq->x509 = NULL;
		}
#		endif
		//
		NBMemory_free(obj->opaque);
		obj->opaque = NULL;
	}
}

//

BOOL NBX509Req_canCreateFromDERBytes(const void* data, const UI32 dataSz){ //binary
	BOOL r = FALSE;
	STNBX509Req req;
	NBX509Req_init(&req);
	if(NBX509Req_createFromDERBytes(&req, data, dataSz)){
		r = TRUE;
	}
	NBX509Req_release(&req);
	return r;
}

//

BOOL NBX509Req_isCreated(const STNBX509Req* obj){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509ReqOpq* opq = (STNBX509ReqOpq*)obj->opaque;
		r = (opq->x509 != NULL ? TRUE : FALSE);
#		endif
	}
	return r;
}

BOOL NBX509Req_createEmpty(STNBX509Req* obj){
	BOOL r = FALSE;
	if(obj->opaque == NULL){
#		ifdef NB_ENABLE_OPENSSL
		X509_REQ* x509 = X509_REQ_new();
		if(x509 != NULL){
			STNBX509ReqOpq* opq = obj->opaque = NBMemory_allocType(STNBX509ReqOpq);
			NBMemory_setZeroSt(*opq, STNBX509ReqOpq);
			opq->x509	= x509;
			r = TRUE;
		}
#		endif
	}
	return r;
}

BOOL NBX509Req_createFromDERBytes(STNBX509Req* obj, const void* data, UI32 dataSz){ //binary
	BOOL r = FALSE;
	if(obj->opaque == NULL && data != NULL && dataSz > 0){
#		ifdef NB_ENABLE_OPENSSL
		const BYTE* tmpPtr = (const BYTE*)data; //use of variable is mandatory
		X509_REQ* x509 = d2i_X509_REQ(NULL, &tmpPtr, dataSz);
		if(x509 != NULL){
			STNBX509ReqOpq* opq = obj->opaque = NBMemory_allocType(STNBX509ReqOpq);
			NBMemory_setZeroSt(*opq, STNBX509ReqOpq);
			opq->x509	= x509;
			r = TRUE;
		}
#		endif
	}
	return r;
}

BOOL NBX509Req_createFromDERFile(STNBX509Req* obj, const char* filepath){ //binary
	BOOL r = FALSE;
	if(obj->opaque == NULL){
#		ifdef NB_ENABLE_OPENSSL
		FILE* f = NULL;
#		ifdef _WIN32
		fopen_s(&f, filepath, "rb");
#		else
		f = fopen(filepath, "rb");
#		endif
		if(f != NULL){
			STNBString str;
			NBString_init(&str);
			//Read file to BIO
			{
				char buff[16];
				while(TRUE){
					int read = (int)fread(buff, 1, sizeof(buff), f);
					if(read <= 0){
						break;
					} else {
						NBString_concatBytes(&str, buff, read);
					}
				};
				fclose(f);
			}
			//Load cert from BIO
			{
				const BYTE* tmpPtr = (const BYTE*)str.str; //use of variable is mandatory
				X509_REQ* x509 = d2i_X509_REQ(NULL, &tmpPtr, str.length);
				if(x509 != NULL){
					STNBX509ReqOpq* opq = obj->opaque = NBMemory_allocType(STNBX509ReqOpq);
					NBMemory_setZeroSt(*opq, STNBX509ReqOpq);
					opq->x509	= x509;
					r = TRUE;
				}
			}
			NBString_release(&str);
		}
#		endif
	}
	return r;
}

BOOL NBX509Req_createFromPEMBytes(STNBX509Req* obj, const char* strX509, const UI32 strX509Sz){ //base64-text
	BOOL r = FALSE;
	if(obj->opaque == NULL && strX509 != NULL && strX509Sz > 0){
#		ifdef NB_ENABLE_OPENSSL
		BIO* bio = BIO_new(BIO_s_mem());
		if(BIO_write(bio, strX509, strX509Sz) == strX509Sz){
			X509_REQ* x509 = PEM_read_bio_X509_REQ(bio, NULL, NULL, NULL);
			if(x509 != NULL){
				STNBX509ReqOpq* opq = obj->opaque = NBMemory_allocType(STNBX509ReqOpq);
				NBMemory_setZeroSt(*opq, STNBX509ReqOpq);
				opq->x509	= x509;
				r = TRUE;
			}
		}
		BIO_free(bio);
#		endif
	}
	return r;
}

BOOL NBX509Req_createFromPEMFile(STNBX509Req* obj, const char* filepath){ //base64-text
	BOOL r = FALSE;
	if(obj->opaque == NULL){
#		ifdef NB_ENABLE_OPENSSL
		FILE* f = NULL;
#		ifdef _WIN32
		fopen_s(&f, filepath, "rb");
#		else
		f = fopen(filepath, "rb");
#		endif
		if(f != NULL){
			BIO* bio = BIO_new(BIO_s_mem());
			//Read file to BIO
			{
				char buff[16];
				while(TRUE){
					int read = (int)fread(buff, 1, sizeof(buff), f);
					if(read <= 0){
						break;
					} else {
						BIO_write(bio, buff, read);
					}
				};
				fclose(f);
			}
			//Load cert from BIO
			{
				X509_REQ* x509 = PEM_read_bio_X509_REQ(bio, NULL, NULL, NULL);
				if(x509 != NULL){
					STNBX509ReqOpq* opq = obj->opaque = NBMemory_allocType(STNBX509ReqOpq);
					NBMemory_setZeroSt(*opq, STNBX509ReqOpq);
					opq->x509	= x509;
					r = TRUE;
				}
			}
			BIO_free(bio);
		}
#		endif
	}
	return r;
}

#ifdef NB_ENABLE_OPENSSL
BOOL NBX509Req_getAsPEMString_(X509_REQ* x509, STNBString* dstStr){ //base64-text
	BOOL r = FALSE;
	if(x509 != NULL){
		BIO* bio = BIO_new(BIO_s_mem());
		if(PEM_write_bio_X509_REQ(bio, x509) != 0){
			const SI32 sz = (const SI32)BIO_number_written(bio);
			if(sz > 0){
				char* buff = NBMemory_allocTypes(char, sz);
				BIO_read(bio, buff, sz);
				if(dstStr != NULL){
					NBString_concatBytes(dstStr, buff, sz);
				}
				NBMemory_free(buff);
				r = TRUE;
			}
		}
		BIO_free(bio);
	}
	return r;
}
#endif

BOOL NBX509Req_getAsDERString(const STNBX509Req* obj, STNBString* dstStr){ //binary
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509ReqOpq* opq = (STNBX509ReqOpq*)obj->opaque;
		const UI32 len = i2d_X509_REQ(opq->x509, NULL);
		if(len > 0){
			NBString_empty(dstStr);
			NBString_concatRepeatedByte(dstStr, '\0', len);
			{
				BYTE* tmpPtr = (BYTE*)dstStr->str; //use of variable is mandatory
				i2d_X509_REQ(opq->x509, &tmpPtr);
				NBASSERT(tmpPtr == (BYTE*)(dstStr->str + dstStr->length))
			}
			r = TRUE;
		}
#		endif
	}
	return r;
}

BOOL NBX509Req_getAsPEMString(const STNBX509Req* obj, STNBString* dstStr){ //base64-text
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509ReqOpq* opq = (STNBX509ReqOpq*)obj->opaque;
		r = NBX509Req_getAsPEMString_(opq->x509, dstStr);
#		endif
	}
	return r;
}

//Certificate fingerprint

BOOL NBX509Req_concatFingerprint(const STNBX509Req* obj, const ENNBHashType type, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
		STNBX509ReqOpq* opq = (STNBX509ReqOpq*)obj->opaque;
		switch(type) {
			case ENNBHashType_Sha1:
#				ifdef NB_ENABLE_OPENSSL
				{
					unsigned int len = 0;
					unsigned char digest[EVP_MAX_MD_SIZE];
					if(X509_REQ_digest(opq->x509, EVP_sha1(), digest, &len)){
						NBASSERT(len == 20)
						if(len > 0){
							NBString_concatBytes(dstStr, (const char*)digest, len);
							r = TRUE;
						}
					}
				}
#				endif
				break;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return r;
}

BOOL NBX509Req_concatFingerprintHex(const STNBX509Req* obj, const ENNBHashType type, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
		STNBX509ReqOpq* opq = (STNBX509ReqOpq*)obj->opaque;
		switch(type) {
			case ENNBHashType_Sha1:
#				ifdef NB_ENABLE_OPENSSL
				{
					unsigned int len = 0;
					unsigned char digest[EVP_MAX_MD_SIZE];
					if(X509_REQ_digest(opq->x509, EVP_sha1(), digest, &len)){
						NBASSERT(len == 20)
						if(len > 0){
							char hexs[41]; NBSha1_hashBinToHexChars(digest, hexs);
							NBString_concatBytes(dstStr, (const char*)hexs, 40);
							r = TRUE;
						}
					}
				}
#				endif
				break;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return r;
}

BOOL NBX509Req_concatFingerprintBase64(const STNBX509Req* obj, const ENNBHashType type, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
		STNBX509ReqOpq* opq = (STNBX509ReqOpq*)obj->opaque;
		switch(type) {
			case ENNBHashType_Sha1:
#				ifdef NB_ENABLE_OPENSSL
				{
					unsigned int len = 0;
					unsigned char digest[EVP_MAX_MD_SIZE];
					if(X509_REQ_digest(opq->x509, EVP_sha1(), digest, &len)){
						NBASSERT(len == 20)
						if(len > 0){
							NBBase64_codeBytes(dstStr, (const char*)digest, 20);
							r = TRUE;
						}
					}
				}
#				endif
				break;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return r;
}

//Public key pin

BOOL NBX509Req_concatPubKeyHash(const STNBX509Req* obj, const ENNBHashType type, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509ReqOpq* opq = (STNBX509ReqOpq*)obj->opaque;
		X509_PUBKEY* pkey = X509_REQ_get_X509_PUBKEY(opq->x509);
		if(pkey != NULL){
			const int len = i2d_X509_PUBKEY(pkey, NULL);
			if(len > 0){
				BYTE* data = (BYTE*)NBMemory_alloc(len);
				{
					BYTE* tmpPtr = (BYTE*)data; //use of variable is mandatory
					i2d_X509_PUBKEY(pkey, &tmpPtr);
				}
				switch(type) {
					case ENNBHashType_Sha1:
						{
							STNBSha1 s;
							NBSha1_init(&s);
							NBSha1_feed(&s, data, len);
							NBSha1_feedEnd(&s);
							NBASSERT(sizeof(s.hash.v) == 20)
							NBString_concatBytes(dstStr, (const char*)s.hash.v, (const UI32)sizeof(s.hash.v));
							r = TRUE;
						}
						break;
					default:
						NBASSERT(FALSE)
						break;
				}
				NBMemory_free(data);
			}
		}
#		endif
	}
	return r;
}

BOOL NBX509Req_concatPubKeyHashHex(const STNBX509Req* obj, const ENNBHashType type, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509ReqOpq* opq = (STNBX509ReqOpq*)obj->opaque;
		X509_PUBKEY* pkey = X509_REQ_get_X509_PUBKEY(opq->x509);
		if(pkey != NULL){
			const int len = i2d_X509_PUBKEY(pkey, NULL);
			if(len > 0){
				BYTE* data = (BYTE*)NBMemory_alloc(len);
				{
					BYTE* tmpPtr = (BYTE*)data; //use of variable is mandatory
					i2d_X509_PUBKEY(pkey, &tmpPtr);
				}
				switch(type) {
					case ENNBHashType_Sha1:
					{
						STNBSha1 s;
						NBSha1_init(&s);
						NBSha1_feed(&s, data, len);
						NBSha1_feedEnd(&s);
						const STNBSha1HashHex hex = NBSha1_getHashHex(&s);
						NBString_concatBytes(dstStr, hex.v, 40);
						r = TRUE;
					}
						break;
					default:
						NBASSERT(FALSE)
						break;
				}
				NBMemory_free(data);
			}
		}
#		endif
	}
	return r;
}

BOOL NBX509Req_concatPubKeyHashBase64(const STNBX509Req* obj, const ENNBHashType type, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509ReqOpq* opq = (STNBX509ReqOpq*)obj->opaque;
		X509_PUBKEY* pkey = X509_REQ_get_X509_PUBKEY(opq->x509);
		if(pkey != NULL){
			const int len = i2d_X509_PUBKEY(pkey, NULL);
			if(len > 0){
				BYTE* data = (BYTE*)NBMemory_alloc(len);
				{
					BYTE* tmpPtr = (BYTE*)data; //use of variable is mandatory
					i2d_X509_PUBKEY(pkey, &tmpPtr);
				}
				switch(type) {
					case ENNBHashType_Sha1:
						{
							STNBSha1 s;
							NBSha1_init(&s);
							NBSha1_feed(&s, data, len);
							NBSha1_feedEnd(&s);
							NBASSERT(sizeof(s.hash.v) == 20)
							NBBase64_codeBytes(dstStr, (const char*)s.hash.v, (const UI32)sizeof(s.hash.v));
							r = TRUE;
						}
						break;
					default:
						NBASSERT(FALSE)
						break;
				}
				NBMemory_free(data);
			}
		}
#		endif
	}
	return r;
}

//

BOOL NBX509Req_concatSubject(const STNBX509Req* obj, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509ReqOpq* opq = (STNBX509ReqOpq*)obj->opaque;
		if(opq->x509 != NULL){
			const char* str = X509_NAME_oneline(X509_REQ_get_subject_name(opq->x509), 0, 0);
			if(str != NULL){
				NBString_concat(dstStr, str);
				r = TRUE;
			}
		}
#		endif
	}
	return r;
}

BOOL NBX509Req_createSelfSigned(STNBX509Req* obj, const UI32 bits, const STNBX500NamePair* namePairs, const UI32 namePairsSz, STNBPKey* dstPrivKey){
	BOOL r = FALSE;
	if(obj->opaque == NULL){
#		ifdef NB_ENABLE_OPENSSL
		EVP_PKEY* keys = EVP_PKEY_new();
		if(keys == NULL){
			PRINTF_ERROR("Could not create PKEY.\n");
		} else {
			RSA* rsa = RSA_new();
			if(rsa == NULL){
				PRINTF_ERROR("Could not create RSA.\n");
			} else {
				BIGNUM* bne = BN_new();
				if(bne == NULL){
					PRINTF_ERROR("Could not create BIGNUM.\n");
				} else {
					if(!BN_set_word(bne, RSA_F4)){
						PRINTF_ERROR("Could not set BIGNUM.\n");
					} else {
						RSA_generate_key_ex(rsa, bits, bne, NULL);
						if(!EVP_PKEY_assign_RSA(keys, rsa)){ //rsa will be free by the key
							PRINTF_ERROR("Could not assign rsa-key.\n");
						} else {
							X509_REQ* x509 = X509_REQ_new();
							if(x509 == NULL){
								PRINTF_ERROR("Could not create X509.\n");
							} else {
								//Version
								X509_REQ_set_version(x509, 0); //0 = All versions (CSR)
								//Issuer
								{
									X509_NAME* name = X509_REQ_get_subject_name(x509);
									//CN: CommonName ("*.foo.org", "foo.org/emailAddress=admin@foo.org" for https; "Foo company" for other)
									//L : LocalityName ("Managua")
									//ST: StateOrProvinceName ("Managua")
									//O : OrganizationName ("Foo Org")
									//OU: OrganizationalUnitName ("Administration")
									//C : CountryName ("NI")
									UI32 i; for(i = 0; i < namePairsSz; i++){
										const STNBX500NamePair* pair = &namePairs[i];
										if(pair->type != NULL && pair->value != NULL){
											X509_NAME_add_entry_by_txt(name, pair->type,  MBSTRING_ASC, (unsigned char *)pair->value, -1, -1, 0);
										}
									}
								}
								//Subject public key info
								X509_REQ_set_pubkey(x509, keys);
								//Certificate Signature
								if(!X509_REQ_sign(x509, keys, EVP_sha1())){
									PRINTF_ERROR("Could not sign certificate.\n");
								} else {
									//Save key
									const UI32 len = i2d_PrivateKey(keys, NULL);
									if(len > 0){
										BYTE* data = (BYTE*)NBMemory_alloc(len);
										{
											BYTE* tmpPtr = data; //use of variable is mandatory
											i2d_PrivateKey(keys, &tmpPtr);
										}
										if(!NBPKey_createFromDERBytes(dstPrivKey, data, len)){
											PRINTF_ERROR("Could not save private key.\n");
										} else {
											STNBX509ReqOpq* opq = obj->opaque = NBMemory_allocType(STNBX509ReqOpq);
											NBMemory_setZeroSt(*opq, STNBX509ReqOpq);
											opq->x509	= x509;
											x509 = NULL;
											r = TRUE;
										}
										NBMemory_free(data);
									}
								}
								if(x509 != NULL){
									X509_REQ_free(x509);
									x509 = NULL;
								}
							}
						}
					}
					BN_free(bne);
				}
				//RSA_free(rsa); //rsa will be free by the key
			}
			EVP_PKEY_free(keys);
		}
#		endif
	}
	return r;
}



