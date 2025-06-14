//
//  NBCryptAES.cpp
//  AUFramework
//
//  Created by Marcos Ortega Morales on 11/12/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
#include "nb/crypto/NBAes256.h"
#include "nb/core/NBMemory.h"
//
#ifdef NB_ENABLE_OPENSSL
#include "openssl/evp.h"
#include "openssl/aes.h"
#include "openssl/err.h"
#endif

typedef struct STNBAes256Opq_ {
	ENAes256Op		op;
#	ifdef NB_ENABLE_OPENSSL
	EVP_CIPHER_CTX*	ctx;
#	endif
} STNBAes256Opq;

void NBAes256_init(STNBAes256* obj){
	obj->opaque = NULL;
}

void NBAes256_release(STNBAes256* obj){
	STNBAes256Opq* opq = (STNBAes256Opq*)obj->opaque;
	if(opq != NULL){
#		ifdef NB_ENABLE_OPENSSL
		if(opq->ctx != NULL){
			EVP_CIPHER_CTX_free(opq->ctx);
			opq->ctx = NULL;
		}
#		endif
		NBMemory_free(opq);
		obj->opaque = NULL;
	}
}

UI32 NBAes256_blockSize(void){
#	ifdef NB_ENABLE_OPENSSL
	return AES_BLOCK_SIZE;
#	else
	return 0;
#	endif
}

UI32 NBAes256_encryptedSize(const UI32 plainSize){
#	ifdef NB_ENABLE_OPENSSL
	return ((plainSize / AES_BLOCK_SIZE) + 1) * AES_BLOCK_SIZE;
#	else
	return 0;
#	endif
}

BOOL NBAes256_feedStart(STNBAes256* obj, const ENAes256Op op, const char* pass, const UI32 passSz, const BYTE* salt, const UI8 saltSz, const UI16 iterations){
	BOOL r = FALSE;
	if(obj->opaque == NULL){
#		ifdef NB_ENABLE_OPENSSL
		if((op == ENAes256Op_Encrypt || op == ENAes256Op_Decrypt) && passSz != 0){
			BYTE key[32];
			//RFC 2898 suggests an iteration count of at least 1000.
			if(!PKCS5_PBKDF2_HMAC(pass, (SI32)passSz, salt, (SI32)saltSz, (SI32)iterations, EVP_sha1(), 32, key)) {
				PRINTF_ERROR("PKCS5_PBKDF2_HMAC could not generate 256bits key from pass(%d len) with salt(%d len).\n", passSz, saltSz);
			} else {
				EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
				if(ctx != NULL){
					if(op == ENAes256Op_Encrypt){
						if(!EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, NULL)){
							PRINTF_ERROR("EVP_EncryptInit_ex failed.\n");
							ERR_print_errors_fp(stderr);
						} else {
							STNBAes256Opq* opq = obj->opaque = NBMemory_allocType(STNBAes256Opq);
							NBMemory_setZeroSt(*opq, STNBAes256Opq);
							opq->op			= op;
							opq->ctx		= ctx;
							r = TRUE;
						}
					} else {
						if(!EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, NULL)){
							PRINTF_ERROR("EVP_DecryptInit_ex failed.\n");
							ERR_print_errors_fp(stderr);
						} else {
							STNBAes256Opq* opq = obj->opaque = NBMemory_allocType(STNBAes256Opq);
							NBMemory_setZeroSt(*opq, STNBAes256Opq);
							opq->op			= op;
							opq->ctx		= ctx;
							r = TRUE;
						}
					}
				}
			}
		}
#		endif
	}
	return r;
}

//Encrypt

SI32 NBAes256_feed(STNBAes256* obj, BYTE* outBuff, const SI32 outBuffSz, const BYTE* plainData, SI32 plainDataSz){
	SI32 r = -1;
	STNBAes256Opq* opq = (STNBAes256Opq*)obj->opaque;
	if(opq != NULL){
#		ifdef NB_ENABLE_OPENSSL
		//Buffer size must be (plainDataSz + AES_BLOCK_SIZE) minimun
		/*if((plainDataSz % AES_BLOCK_SIZE) != 0){
		 PRINTF_ERROR("outBuff must be multiple of AES_BLOCK_SIZE(%d).\n", AES_BLOCK_SIZE);
		 } else*/ if(outBuffSz < (plainDataSz + AES_BLOCK_SIZE)){
			 PRINTF_ERROR("outBuff too small, muts be at least 'plainDataSz + AES_BLOCK_SIZE(%d)'.\n", AES_BLOCK_SIZE);
		 } else {
			 SI32 outWritten = 0;
			 switch(opq->op) {
				 case ENAes256Op_Encrypt:
					 if(!EVP_EncryptUpdate(opq->ctx, outBuff, &outWritten, plainData, plainDataSz)){
						 PRINTF_ERROR("EVP_EncryptUpdate returned error.\n");
						 ERR_print_errors_fp(stderr);
					 } else {
						 r = outWritten;
					 }
					 break;
				 case ENAes256Op_Decrypt:
					 if(!EVP_DecryptUpdate(opq->ctx, outBuff, &outWritten, plainData, plainDataSz)){
						 PRINTF_ERROR("EVP_DecryptUpdate returned error.\n");
						 ERR_print_errors_fp(stderr);
					 } else {
						 r = outWritten;
					 }
					 break;
				 default:
				 	NBASSERT(FALSE)
				 	break;
			}
		}
#		endif
	}
	return r;
}

SI32 NBAes256_feedEnd(STNBAes256* obj, BYTE* outBuff, const SI32 outBuffSz){
	SI32 r = -1;
	STNBAes256Opq* opq = (STNBAes256Opq*)obj->opaque;
	if(opq != NULL){
#		ifdef NB_ENABLE_OPENSSL
		//Buffer size must be (plainDataSz + AES_BLOCK_SIZE) minimun
		if(outBuffSz < AES_BLOCK_SIZE){
			PRINTF_ERROR("outBuff too small, muts be at least AES_BLOCK_SIZE(%d) for final block.\n", AES_BLOCK_SIZE);
		} else {
			int rr = 0;
			SI32 outWritten = 0;
			switch(opq->op) {
				case ENAes256Op_Encrypt:
					rr = EVP_EncryptFinal_ex(opq->ctx, outBuff, &outWritten);
					if(!rr){
						PRINTF_ERROR("EVP_EncryptFinal_ex returned error.\n");
						ERR_print_errors_fp(stderr);
					} else {
						r = outWritten;
					}
					break;
				case ENAes256Op_Decrypt:
					rr = EVP_DecryptFinal_ex(opq->ctx, outBuff, &outWritten);
					if(!rr){
						PRINTF_ERROR("EVP_DecryptFinal_ex returned error.\n");
						ERR_print_errors_fp(stderr);
					} else {
						r = outWritten;
					}
					break;
				default:
					break;
			}
		}
		//
		{
			if(opq->ctx != NULL){
				EVP_CIPHER_CTX_free(opq->ctx);
				opq->ctx = NULL;
			}
			NBMemory_free(opq);
			obj->opaque = NULL;
		}
#		endif
	}
	return r;
}

//

BOOL NBAes256_aesEncrypt(const void* pPlainData, const UI32 plainDataSz, const char* pass, const UI32 passSz, const void* pSalt, const UI8 saltSz, const UI16 iterations, STNBString* dst){
	BOOL r = FALSE;
	const UI32 encBlockSz			= NBAes256_blockSize();
	/*if((plainDataSz % encBlockSz) != 0){
		PRINTF_ERROR("NBAes256_aesEncrypt, crypt data must be multiple-of-block.\n");
	} else*/ {
		const BYTE* salt			= (const BYTE*)pSalt;
		STNBAes256 enc;
		NBAes256_init(&enc);
		if(NBAes256_feedStart(&enc, ENAes256Op_Encrypt, pass, passSz, salt, saltSz, iterations)){
			r = TRUE;
			{
				const BYTE* plainData	= (const BYTE*)pPlainData;
				const UI32 bytesPerRead	= (256 * encBlockSz);
				const UI32 outBuffSz	= bytesPerRead + encBlockSz;
				BYTE* outBuff			= (BYTE*)NBMemory_alloc(outBuffSz);
				UI32 iByte = 0, wTotal = 0;
				while(iByte < plainDataSz){
					const UI32 bytesRemain	= (plainDataSz - iByte); //NBASSERT((bytesRemain % encBlockSz) == 0)
					const UI32 bytesToRead	= (bytesRemain > bytesPerRead ? bytesPerRead : bytesRemain); //NBASSERT((bytesToRead % encBlockSz) == 0)
					const SI32 written		= NBAes256_feed(&enc, outBuff, (const SI32)outBuffSz, &plainData[iByte], bytesToRead); //NBASSERT((written % encBlockSz) == 0)
					if(written < 0){
						r = FALSE; //NBASSERT(FALSE)
						break;
					} else {
						if(written > 0 && dst != NULL){
							NBString_concatBytes(dst, (const char*)outBuff, written);
						}
						iByte += bytesToRead;
						wTotal += written;
					}
				}
				//Final
				if(r){
					const SI32 written = NBAes256_feedEnd(&enc, outBuff, (const SI32)outBuffSz);
					if(written < 0){
						r = FALSE; //NBASSERT(FALSE)
					} else {
						if(written > 0 && dst != NULL){
							NBString_concatBytes(dst, (const char*)outBuff, written);
						}
						wTotal += written;
					}
				}
				//PRINTF_INFO("NBAes256_aesEncrypt, %d of %d bytes processed (%d bytes produced, %d-encBlockSz).\n", iByte, plainDataSz, wTotal, encBlockSz);
				NBASSERT(iByte == plainDataSz)
				NBASSERT(wTotal >= plainDataSz && wTotal <= (plainDataSz + encBlockSz))
				NBMemory_free(outBuff);
			}
		}
		NBAes256_release(&enc);
	}
	return r;
}


BOOL NBAes256_aesDecrypt(const void* pCryptdData, const UI32 cryptdDataSz, const char* pass, const UI32 passSz, const void* pSalt, const UI8 saltSz, const UI16 iterations, STNBString* dst){
	BOOL r = FALSE;
	if(cryptdDataSz > 0){
		const UI32 encBlockSz			= NBAes256_blockSize();
		/*if((cryptdDataSz % encBlockSz) != 0){
			PRINTF_ERROR("NBAes256_aesDecrypt, crypt data must be multiple-of-block.\n");
		} else*/ {
			const BYTE* salt			= (const BYTE*)pSalt;
			STNBAes256 dec;
			NBAes256_init(&dec);
			if(NBAes256_feedStart(&dec, ENAes256Op_Decrypt, pass, passSz, salt, saltSz, iterations)){
				r = TRUE;
				{
					const BYTE* cryptdData	= (const BYTE*)pCryptdData;
					const UI32 bytesPerRead	= (256 * encBlockSz);
					const UI32 outBuffSz	= bytesPerRead + encBlockSz;
					BYTE* outBuff			= (BYTE*)NBMemory_alloc(outBuffSz);
					UI32 iByte = 0, wTotal = 0;
					while(iByte < cryptdDataSz){
						const UI32 bytesRemain	= (cryptdDataSz - iByte); //NBASSERT((bytesRemain % encBlockSz) == 0)
						const UI32 bytesToRead	= (bytesRemain > bytesPerRead ? bytesPerRead : bytesRemain); //NBASSERT((bytesToRead % encBlockSz) == 0)
						const SI32 written		= NBAes256_feed(&dec, outBuff, (const SI32)outBuffSz, &cryptdData[iByte], bytesToRead); //NBASSERT((written % encBlockSz) == 0)
						if(written < 0){
							r = FALSE; //NBASSERT(FALSE)
							break;
						} else {
							if(written > 0 && dst != NULL){
								NBString_concatBytes(dst, (const char*)outBuff, written);
							}
							iByte += bytesToRead;
							wTotal += written;
						}
					}
					//Final
					if(r){
						const SI32 written = NBAes256_feedEnd(&dec, outBuff, (const SI32)outBuffSz);
						if(written < 0){
							r = FALSE; //NBASSERT(FALSE)
						} else {
							if(written > 0 && dst != NULL){
								NBString_concatBytes(dst, (const char*)outBuff, written);
							}
							wTotal += written;
						}
					}
					//PRINTF_INFO("NBAes256_aesDecrypt, %d of %d bytes processed (%d bytes produced, %d-encBlockSz).\n", iByte, cryptdDataSz, wTotal, encBlockSz);
					NBASSERT(iByte == cryptdDataSz)
					NBASSERT(cryptdDataSz >= wTotal && cryptdDataSz <= (wTotal + encBlockSz))
					NBMemory_free(outBuff);
				}
			}
			NBAes256_release(&dec);
		}
	}
	return r;
}
