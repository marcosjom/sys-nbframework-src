//
//  NBKey.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/5/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/crypto/NBPKey.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBThreadMutex.h"

//SSL
#ifdef NB_ENABLE_OPENSSL
#	include "openssl/rand.h"
#	include "openssl/ssl.h"
#	include "openssl/err.h"
#endif

typedef struct STNBPKeyOpq_ {
#	ifdef NB_ENABLE_OPENSSL
	EVP_PKEY*		pkey;
#	endif
	STNBThreadMutex	mutex;
} STNBPKeyOpq;

void NBPKey_init(STNBPKey* obj){
	obj->opaque	= NULL;
}

void NBPKey_release(STNBPKey* obj){
	if(obj->opaque != NULL){
		STNBPKeyOpq* opq = (STNBPKeyOpq*)obj->opaque;
		NBThreadMutex_lock(&opq->mutex);
		{
#			ifdef NB_ENABLE_OPENSSL
			if(opq->pkey != NULL){
				EVP_PKEY_free(opq->pkey);
				opq->pkey = NULL;
			}
#			endif
			NBThreadMutex_unlock(&opq->mutex);
		}
		NBThreadMutex_release(&opq->mutex);
		//
		NBMemory_free(obj->opaque);
		obj->opaque = NULL;
	}
}

//

BOOL NBPKey_isCreated(const STNBPKey* obj){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBPKeyOpq* opq = (STNBPKeyOpq*)obj->opaque;
		r = (opq->pkey != NULL ? TRUE : FALSE);
#		endif
	}
	return r;
}

BOOL NBPKey_createEmpty(STNBPKey* obj){
	BOOL r = FALSE;
	if(obj->opaque == NULL){
#		ifdef NB_ENABLE_OPENSSL
		EVP_PKEY* EVP_PKEY = EVP_PKEY_new();
		if(EVP_PKEY != NULL){
			STNBPKeyOpq* opq = obj->opaque = NBMemory_allocType(STNBPKeyOpq);
			NBMemory_setZeroSt(*opq, STNBPKeyOpq);
			opq->pkey		= EVP_PKEY;
			NBThreadMutex_init(&opq->mutex);
			r = TRUE;
		}
#		endif
	}
	return r;
}

BOOL NBPKey_createFromOther(STNBPKey* obj, STNBPKey* other){
	BOOL r = FALSE;
	if(other->opaque != NULL){
		STNBString str;
		NBString_init(&str);
		if(NBPKey_getAsDERString(other, &str)){
			r = NBPKey_createFromDERBytes(obj, str.str, str.length);
		}
		NBString_release(&str);
	}
	return r;
}

BOOL NBPKey_createFromDERBytes(STNBPKey* obj, const void* data, UI32 dataSz){ //binary
	BOOL r = FALSE;
	if(obj->opaque == NULL && data != NULL && dataSz > 0){
#		ifdef NB_ENABLE_OPENSSL
		const BYTE* tmpPtr = data; //use of variable is mandatory
		EVP_PKEY* pkey = d2i_AutoPrivateKey(NULL, &tmpPtr, dataSz);
		if(pkey != NULL){
			STNBPKeyOpq* opq = obj->opaque = NBMemory_allocType(STNBPKeyOpq);
			NBMemory_setZeroSt(*opq, STNBPKeyOpq);
			opq->pkey		= pkey;
			NBThreadMutex_init(&opq->mutex);
			r = TRUE;
		}
#		endif
	}
	return r;
}

BOOL NBPKey_createFromDERFile(STNBPKey* obj, const char* filepath){ //binary
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
				EVP_PKEY* pkey = d2i_AutoPrivateKey(NULL, &tmpPtr, str.length);
				if(pkey != NULL){
					STNBPKeyOpq* opq = obj->opaque = NBMemory_allocType(STNBPKeyOpq);
					NBMemory_setZeroSt(*opq, STNBPKeyOpq);
					opq->pkey		= pkey;
					NBThreadMutex_init(&opq->mutex);
					r = TRUE;
				}
			}
			NBString_release(&str);
		}
#		endif
	}
	return r;
}

BOOL NBPKey_createFromPEMBytes(STNBPKey* obj, const char* strPKey, const UI32 strPKeySz){ //base64-text
	BOOL r = FALSE;
	if(obj->opaque == NULL && strPKey != NULL && strPKeySz > 0){
#		ifdef NB_ENABLE_OPENSSL
		BIO* bio = BIO_new(BIO_s_mem());
		if(BIO_write(bio, strPKey, strPKeySz) == strPKeySz){
			EVP_PKEY* pkey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
			if(pkey != NULL){
				STNBPKeyOpq* opq = obj->opaque = NBMemory_allocType(STNBPKeyOpq);
				NBMemory_setZeroSt(*opq, STNBPKeyOpq);
				opq->pkey		= pkey;
				NBThreadMutex_init(&opq->mutex);
				r = TRUE;
			}
		}
		BIO_free(bio);
#		endif
	}
	return r;
}

BOOL NBPKey_createFromPEMFile(STNBPKey* obj, const char* filepath){ //base64-text
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
				EVP_PKEY* pkey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
				if(pkey != NULL){
					STNBPKeyOpq* opq = obj->opaque = NBMemory_allocType(STNBPKeyOpq);
					NBMemory_setZeroSt(*opq, STNBPKeyOpq);
					opq->pkey		= pkey;
					NBThreadMutex_init(&opq->mutex);
					r = TRUE;
				}
			}
			BIO_free(bio);
		}
#		endif
	}
	return r;
}

//

BOOL NBPKey_getAsDERString(const STNBPKey* obj, STNBString* dstStr){ //binary
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBPKeyOpq* opq = (STNBPKeyOpq*)obj->opaque;
		NBThreadMutex_lock(&opq->mutex);
		{
			const UI32 len = i2d_PrivateKey(opq->pkey, NULL);
			if(len > 0){
				NBString_empty(dstStr);
				NBString_concatRepeatedByte(dstStr, '\0', len);
				{
					BYTE* tmpPtr = (BYTE*)dstStr->str; //use of variable is mandatory
					i2d_PrivateKey(opq->pkey, &tmpPtr);
					NBASSERT(tmpPtr == (BYTE*)(dstStr->str + dstStr->length))
				}
				r = TRUE;
			}
		}
		NBThreadMutex_unlock(&opq->mutex);
#		endif
	}
	return r;
}

#ifdef NB_ENABLE_OPENSSL
BOOL NBPKey_getAsPEMStringLocked_(EVP_PKEY* pkey, STNBString* dstStr){
	BOOL r = FALSE;
	if(pkey != NULL){
		BIO* bio = BIO_new(BIO_s_mem());
		if(PEM_write_bio_PrivateKey(bio, pkey, NULL, NULL, 0, NULL, NULL) != 0){
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

BOOL NBPKey_getAsPEMString(const STNBPKey* obj, STNBString* dstStr){ //base64-text
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBPKeyOpq* opq = (STNBPKeyOpq*)obj->opaque;
		NBThreadMutex_lock(&opq->mutex);
		{
			r = NBPKey_getAsPEMStringLocked_(opq->pkey, dstStr);
		}
		NBThreadMutex_unlock(&opq->mutex);
#		endif
	}
	return r;
}

//Signature

BOOL NBPKey_concatBytesSignatureSha1(STNBPKey* obj, const void* data, const SI32 dataSz, STNBString* dstBin){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBPKeyOpq* opq = (STNBPKeyOpq*)obj->opaque;
		NBThreadMutex_lock(&opq->mutex);
		{
			unsigned char sig_buf[4096];
			unsigned int sig_len = sizeof(sig_buf);
			EVP_MD_CTX* md_ctx = EVP_MD_CTX_create();
			if(!EVP_SignInit(md_ctx, EVP_sha1())){
				//PRINTF_ERROR("NBPKey, could not EVP_SignInit.\n");
			} else if(!EVP_SignUpdate(md_ctx, data, dataSz)){
				//PRINTF_ERROR("NBPKey, could not EVP_SignUpdate.\n");
			} else if(EVP_SignFinal(md_ctx, sig_buf, &sig_len, opq->pkey) == 1){
				if(dstBin != NULL){
					NBString_setBytes(dstBin, (const char*)sig_buf, sig_len);
				}
				r = TRUE;
			}
			EVP_MD_CTX_destroy(md_ctx);
		}
		NBThreadMutex_unlock(&opq->mutex);
#		endif
	}
	return r;
}

BOOL NBPKey_concatBytesSignatureSha256(STNBPKey* obj, const void* data, const SI32 dataSz, STNBString* dstBin){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBPKeyOpq* opq = (STNBPKeyOpq*)obj->opaque;
		NBThreadMutex_lock(&opq->mutex);
		/*{
			EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
			if(mdctx != NULL){
				unsigned char md_value[EVP_MAX_MD_SIZE];
				unsigned int md_len = 0;
				if(!EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL)){
					PRINTF_ERROR("NBPKey, EVP_DigestInit_ex failed.\n");
				} else if(!EVP_DigestUpdate(mdctx, data, dataSz)){
					PRINTF_ERROR("NBPKey, EVP_DigestUpdate failed.\n");
				} else if(!EVP_DigestFinal_ex(mdctx, md_value, &md_len)){
					PRINTF_ERROR("NBPKey, EVP_DigestFinal_ex failed.\n");
				} else {
					size_t siglen = 0;
					EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(opq->pkey, NULL); //no engine
					if(ctx == NULL){
						PRINTF_ERROR("NBPKey, EVP_PKEY_CTX_new failed.\n");
					} else if(EVP_PKEY_sign_init(ctx) <= 0){
						PRINTF_ERROR("NBPKey, EVP_PKEY_sign_init failed.\n");
					} else if(EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0){
						PRINTF_ERROR("NBPKey, EVP_PKEY_CTX_set_rsa_padding failed.\n");
					} else if(EVP_PKEY_CTX_set_signature_md(ctx, EVP_sha256()) <= 0){
						PRINTF_ERROR("NBPKey, EVP_PKEY_CTX_set_signature_md failed.\n");
					} else if(EVP_PKEY_sign(ctx, NULL, &siglen, md_value, md_len) <= 0){
						PRINTF_ERROR("NBPKey, EVP_PKEY_sign failed for signature len.\n");
					} else {
						BYTE* sig = NBMemory_alloc(siglen);
						if(sig != NULL){
							if(EVP_PKEY_sign(ctx, sig, &siglen, md_value, md_len) <= 0){
								PRINTF_ERROR("NBPKey, EVP_PKEY_sign failed for data-hash.\n");
							} else {
								if(dstBin != NULL){
									NBString_setBytes(dstBin, (const char*)sig, (UI32)siglen);
								}
								r = TRUE;
							}
							//Release
							NBMemory_free(sig);
							sig = NULL;
						}
					}
				}
				EVP_MD_CTX_free(mdctx);
			}
		}*/
		{
			unsigned char sig_buf[4096];
			unsigned int sig_len = sizeof(sig_buf);
			EVP_MD_CTX* md_ctx = EVP_MD_CTX_create();
			if(!EVP_SignInit(md_ctx, EVP_sha256())){
				//PRINTF_ERROR("NBPKey, could not EVP_SignInit.\n");
			} else if(!EVP_SignUpdate(md_ctx, data, dataSz)){
				//PRINTF_ERROR("NBPKey, could not EVP_SignUpdate.\n");
			} else if(EVP_SignFinal(md_ctx, sig_buf, &sig_len, opq->pkey) == 1){
				if(dstBin != NULL){
					NBString_setBytes(dstBin, (const char*)sig_buf, sig_len);
				}
				r = TRUE;
			}
			EVP_MD_CTX_destroy(md_ctx);
		}
		NBThreadMutex_unlock(&opq->mutex);
#		endif
	}
	return r;
}

//Encryption

BOOL NBPKey_concatEncryptedBytes(STNBPKey* obj, const void* data, const SI32 dataSz, STNBString* dstEnc){
	BOOL r = FALSE;
	NBASSERT(dataSz < 245) //If fails, maybe trying to encrypt already encrypted data.
	if(obj->opaque != NULL && dataSz < 245){ //RSA has a limit in size ~245 bytes
#		ifdef NB_ENABLE_OPENSSL
		STNBPKeyOpq* opq = (STNBPKeyOpq*)obj->opaque;
		NBThreadMutex_lock(&opq->mutex);
		{
			RSA* rsa		= EVP_PKEY_get0_RSA(opq->pkey);
			int rsaLen		= RSA_size(rsa);
			unsigned char* buff = (unsigned char*)NBMemory_alloc(rsaLen) ;
			{
				int rr = RSA_private_encrypt(dataSz, (const unsigned char*)data, buff, rsa, RSA_PKCS1_PADDING);
				if(rr >= 0){
					if(dstEnc != NULL){
						NBString_concatBytes(dstEnc, (const char*)buff, rr);
					}
					r = TRUE;
#				ifdef NB_CONFIG_INCLUDE_ASSERTS
				/*} else {
						char errDesc[1024];
						unsigned long errCode = ERR_get_error();
						ERR_error_string_n(errCode, errDesc, sizeof(errDesc));
						PRINTF_ERROR("NBPKey, RSA_private_encrypt returned(%d) errCode(%ld) desc: '%s'.\n", rr, errCode, errDesc);
				 */
#				endif
				}
			}
			NBMemory_free(buff);
		}
		NBThreadMutex_unlock(&opq->mutex);
#		endif
	}
	return r;
}

#include "nb/crypto/NBBase64.h"

BOOL NBPKey_concatDecryptedBytes(STNBPKey* obj, const void* data, const SI32 dataSz, STNBString* dstEnc){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBPKeyOpq* opq = (STNBPKeyOpq*)obj->opaque;
		NBThreadMutex_lock(&opq->mutex);
		{
			RSA* rsa		= EVP_PKEY_get0_RSA(opq->pkey);
			int rsaLen		= RSA_size(rsa);
			unsigned char* buff = (unsigned char*)NBMemory_alloc(rsaLen) ;
			{
				int rr = RSA_private_decrypt(dataSz, (const unsigned char*)data, buff, rsa, RSA_PKCS1_PADDING);
				if(rr >= 0){
					if(dstEnc != NULL){
						NBString_concatBytes(dstEnc, (const char*)buff, rr);
					}
					r = TRUE;
#					ifdef NB_CONFIG_INCLUDE_ASSERTS
					/*{
					 STNBString str, dd;
					 NBString_init(&str);
					 NBString_init(&dd);
					 NBBase64_codeBytes(&dd, (char*)data, dataSz);
					 NBPKey_getAsPEMStringLocked_(opq->pkey, &str);
					 PRINTF_INFO("Success NBPKey:\n-->%s<--\nData:-->%s<--\n", str.str, dd.str);
					 NBString_release(&dd);
					 NBString_release(&str);
					 }*/
#					endif
#				ifdef NB_CONFIG_INCLUDE_ASSERTS
				} else {
					char errDesc[1024];
					unsigned long errCode = ERR_get_error();
					ERR_error_string_n(errCode, errDesc, sizeof(errDesc));
					PRINTF_ERROR("NBPKey, RSA_private_decrypt returned(%d) errCode(%ld) desc: '%s'.\n", rr, errCode, errDesc);
					/*{
					 STNBString str, dd;
					 NBString_init(&str);
					 NBString_init(&dd);
					 NBBase64_codeBytes(&dd, (char*)data, dataSz);
					 NBPKey_getAsPEMStringLocked_(opq->pkey, &str);
					 PRINTF_ERROR("Error NBPKey:\n-->%s<--\nData:-->%s<--\n", str.str, dd.str);
					 NBString_release(&dd);
					 NBString_release(&str);
					 }*/
#				endif
				}
			}
			NBMemory_free(buff);
		}
		NBThreadMutex_unlock(&opq->mutex);
#		endif
	}
	return r;
}
