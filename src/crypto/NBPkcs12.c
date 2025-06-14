//
//  NBPkcs12.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/21/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/crypto/NBPkcs12.h"
//
#include "nb/core/NBMemory.h"

//SSL
#ifdef NB_ENABLE_OPENSSL
#	include "openssl/rand.h"
#	include "openssl/ssl.h"
#	include "openssl/err.h"
#	include "openssl/pkcs12.h"
#endif


typedef struct STNBPkcs12Opq_ {
#	ifdef NB_ENABLE_OPENSSL
	PKCS12* pkcs;
#	else
	SI32		dummy;
#	endif
} STNBPkcs12Opq;

void NBPkcs12_init(STNBPkcs12* obj){
	obj->opaque	= NULL;
}

void NBPkcs12_release(STNBPkcs12* obj){
	if(obj->opaque != NULL){
		STNBPkcs12Opq* opq = (STNBPkcs12Opq*)obj->opaque;
#		ifdef NB_ENABLE_OPENSSL
		if(opq->pkcs != NULL){
			PKCS12_free(opq->pkcs);
			opq->pkcs = NULL;
		}
#		endif
		//
		NBMemory_free(obj->opaque);
		obj->opaque = NULL;
	}
}

//

BOOL NBPkcs12_isCreated(const STNBPkcs12* obj){
	return (obj->opaque != NULL ? TRUE : FALSE);
}

BOOL NBPkcs12_createEmpty(STNBPkcs12* obj){
	BOOL r = FALSE;
	if(obj->opaque == NULL){
#		ifdef NB_ENABLE_OPENSSL
		PKCS12* p = PKCS12_new();
		if(p != NULL){
			STNBPkcs12Opq* opq = obj->opaque = NBMemory_allocType(STNBPkcs12Opq);
			NBMemory_setZeroSt(*opq, STNBPkcs12Opq);
			opq->pkcs	= p;
			r = TRUE;
		}
#		endif
	}
	return r;
}

BOOL NBPkcs12_createFromDERBytes(STNBPkcs12* obj, const void* strDER, const UI32 strDERSz){ //binary
	BOOL r = FALSE;
	if(obj->opaque == NULL && strDER != NULL && strDERSz > 0){
#		ifdef NB_ENABLE_OPENSSL
		const BYTE* tmpPtr = (const BYTE*)strDER; //use of variable is mandatory
		PKCS12* p = d2i_PKCS12(NULL, &tmpPtr, strDERSz);
		if(p != NULL){
			STNBPkcs12Opq* opq = obj->opaque = NBMemory_allocType(STNBPkcs12Opq);
			NBMemory_setZeroSt(*opq, STNBPkcs12Opq);
			opq->pkcs	= p;
			r = TRUE;
		}
#		endif
	}
	return r;
}

BOOL NBPkcs12_createFromDERFile(STNBPkcs12* obj, const char* filepath){ //binary
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
				PKCS12* p = d2i_PKCS12(NULL, &tmpPtr, str.length);
				if(p != NULL){
					STNBPkcs12Opq* opq = obj->opaque = NBMemory_allocType(STNBPkcs12Opq);
					NBMemory_setZeroSt(*opq, STNBPkcs12Opq);
					opq->pkcs	= p;
					r = TRUE;
				}
			}
			NBString_release(&str);
		}
#		endif
	}
	return r;
}

BOOL NBPkcs12_createBundle(STNBPkcs12* obj, const char* name, const STNBPKey* pPkey, const STNBX509* pCert, const STNBX509* caCertsStack, const UI32 caCertsStackSz, const char* pass){
	BOOL r = FALSE;
	if(obj->opaque == NULL && pPkey != NULL && pCert != NULL){
#		ifdef NB_ENABLE_OPENSSL
		BOOL errFnd = FALSE;
		STNBString str;
		NBString_init(&str);
		//Build caCertsStack
		STACK_OF(X509)* caStack = NULL;
		{
			UI32 i; for(i = 0 ; i < caCertsStackSz; i++){
				if(caStack == NULL){
					caStack = sk_X509_new_null();
					if(caStack == NULL){
						PRINTF_ERROR("sk_X509_new_null failed.\n");
						break;
					}
				} NBASSERT(caStack != NULL)
				{
					const STNBX509* caCert = &caCertsStack[i];
					NBString_empty(&str);
					if(!NBX509_getAsDERString(caCert, &str)){
						PRINTF_ERROR("Could not generate clt-certificate's DER.\n");
						errFnd = TRUE;
						break;
					} else {
						const BYTE* tmpPtr = (const BYTE*)str.str; //use of variable is mandatory
						X509* x509 = d2i_X509(NULL, &tmpPtr, str.length);
						if(x509 == NULL){
							PRINTF_ERROR("d2i_X509 failed.\n");
							errFnd = TRUE;
							break;
						} else {
							sk_X509_push(caStack, x509);
							//The x509 will be released with the stack
						}
					}
				}
			}
		}
		if(caStack != NULL){
			if(!errFnd){
				X509* x509 = NULL;
				EVP_PKEY* pkey = NULL;
				//Certificate
				{
					NBString_empty(&str);
					if(!NBX509_getAsDERString(pCert, &str)){
						PRINTF_ERROR("Could not generate clt-certificate's DER.\n");
					} else {
						const BYTE* tmpPtr = (const BYTE*)str.str; //use of variable is mandatory
						x509 = d2i_X509(NULL, &tmpPtr, str.length);
						if(x509 == NULL){
							PRINTF_ERROR("d2i_X509 failed.\n");
						}
					}
				}
				//pkey
				{
					NBString_empty(&str);
					if(!NBPKey_getAsDERString(pPkey, &str)){
						PRINTF_ERROR("Could not generate clt-key's DER.\n");
					} else {
						const BYTE* tmpPtr = (const BYTE*)str.str; //use of variable is mandatory
						pkey = d2i_AutoPrivateKey(NULL, &tmpPtr, str.length);
						if(pkey == NULL){
							PRINTF_ERROR("d2i_AutoPrivateKey failed.\n");
						}
					}
				}
				//Generate
				if(x509 != NULL && pkey != NULL){
					//PKCS12* p = PKCS12_new();
					/* values of zero use the openssl default values */
					PKCS12* p = PKCS12_create(
											  pass,      // certbundle access password
											  name,		// friendly certname
											  pkey,		// the certificate private key
											  x509,        // the main certificate
											  caStack, // stack of CA cert chain
											  0,           // int nid_key (default 3DES)
											  0,           // int nid_cert (40bitRC2)
											  0,           // int iter (default 2048)
											  0,           // int mac_iter (default 1)
											  0            // int keytype (default no flag)
											  );
					if(p == NULL){
						PRINTF_ERROR("PKCS12_create failed.\n");
					} else {
						STNBPkcs12Opq* opq = obj->opaque = NBMemory_allocType(STNBPkcs12Opq);
						NBMemory_setZeroSt(*opq, STNBPkcs12Opq);
						opq->pkcs	= p;
						r = TRUE;
					}
				}
				//Release
				if(x509 != NULL){
					X509_free(x509);
					x509 = NULL;
				}
				if(pkey != NULL){
					EVP_PKEY_free(pkey);
					pkey = NULL;
				}
			}
			//Release elements and stack.
			//Note: 'sk_X509_free' will only release the stack but not the internal elements.
			sk_X509_pop_free(caStack, X509_free);
			caStack = NULL;
		}
		NBString_release(&str);
#		endif
	}
	return r;
}

BOOL NBPkcs12_getCertAndKey(STNBPkcs12* obj, STNBPKey* dstkey, STNBX509* dstcert, const char* pass){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
		STNBPkcs12Opq* opq = (STNBPkcs12Opq*)obj->opaque;
#		ifdef NB_ENABLE_OPENSSL
		if(opq->pkcs != NULL){
			EVP_PKEY* k = NULL; X509* c = NULL;
			if(!PKCS12_parse(opq->pkcs, pass, &k, &c, NULL)){
				//
			} else {
				if(k == NULL || c == NULL){
					//
				} else {
					STNBString derCert, derKey;
					NBString_init(&derCert);
					NBString_init(&derKey);
					{
						const UI32 len = i2d_X509(c, NULL);
						if(len > 0){
							NBString_concatRepeatedByte(&derCert, '\0', len);
							{
								BYTE* tmpPtr = (BYTE*)derCert.str; //use of variable is mandatory
								i2d_X509(c, &tmpPtr);
								NBASSERT(tmpPtr == (BYTE*)(derCert.str + derCert.length))
							}
						}
					}
					{
						const UI32 len = i2d_PrivateKey(k, NULL);
						if(len > 0){
							NBString_concatRepeatedByte(&derKey, '\0', len);
							{
								BYTE* tmpPtr = (BYTE*)derKey.str; //use of variable is mandatory
								i2d_PrivateKey(k, &tmpPtr);
								NBASSERT(tmpPtr == (BYTE*)(derKey.str + derKey.length))
							}
						}
					}
					if(derCert.length > 0 && derKey.length > 0){
						if(NBPKey_createFromDERBytes(dstkey, (BYTE*)derKey.str, derKey.length)){
							if(NBX509_createFromDERBytes(dstcert, (BYTE*)derCert.str, derCert.length)){
								r = TRUE;
							}
						}
					}
					NBString_release(&derCert);
					NBString_release(&derKey);
					X509_free(c);
					EVP_PKEY_free(k);
				}
			}
		}
#		endif
	}
	return r;
}

//

BOOL NBPkcs12_getAsDERString(const STNBPkcs12* obj, STNBString* dstStr){ //binary
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBPkcs12Opq* opq = (STNBPkcs12Opq*)obj->opaque;
		const UI32 len = i2d_PKCS12(opq->pkcs, NULL);
		if(len > 0){
			NBString_empty(dstStr);
			NBString_concatRepeatedByte(dstStr, '\0', len);
			{
				BYTE* tmpPtr = (BYTE*)dstStr->str; //use of variable is mandatory
				i2d_PKCS12(opq->pkcs, &tmpPtr);
				NBASSERT(tmpPtr == (BYTE*)(dstStr->str + dstStr->length))
			}
			r = TRUE;
		}
#		endif
	}
	return r;
}
