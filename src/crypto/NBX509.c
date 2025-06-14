#include "nb/NBFrameworkPch.h"
#include "nb/crypto/NBX509.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBThreadMutex.h"
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

typedef struct STNBX509Opq_ {
#	ifdef NB_ENABLE_OPENSSL
	X509*			x509;
#	endif
	STNBThreadMutex	mutex;
} STNBX509Opq;

void NBX509_init(STNBX509* obj){
	obj->opaque	= NULL;
}

void NBX509_release(STNBX509* obj){
	if(obj->opaque != NULL){
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		NBThreadMutex_lock(&opq->mutex);
		{
#			ifdef NB_ENABLE_OPENSSL
			if(opq->x509 != NULL){
				X509_free(opq->x509);
				opq->x509 = NULL;
			}
#			endif
		}
		NBThreadMutex_unlock(&opq->mutex);
		NBThreadMutex_release(&opq->mutex);
		//
		NBMemory_free(obj->opaque);
		obj->opaque = NULL;
	}
}

//

BOOL NBX509_isCreated(const STNBX509* obj){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		r = (opq->x509 != NULL ? TRUE : FALSE);
#		endif
	}
	return r;
}

BOOL NBX509_createEmpty(STNBX509* obj){
	BOOL r = FALSE;
	if(obj->opaque == NULL){
#		ifdef NB_ENABLE_OPENSSL
		X509* x509 = X509_new();
		if(x509 != NULL){
			STNBX509Opq* opq = obj->opaque = NBMemory_allocType(STNBX509Opq);
			NBMemory_setZeroSt(*opq, STNBX509Opq);
			opq->x509		= x509;
			NBThreadMutex_init(&opq->mutex);
			r = TRUE;
		}
#		endif
	}
	return r;
}

BOOL NBX509_createFromOther(STNBX509* obj, STNBX509* other){
	BOOL r = FALSE;
	if(other->opaque != NULL){
		STNBString str;
		NBString_init(&str);
		if(NBX509_getAsDERString(other, &str)){
			r = NBX509_createFromDERBytes(obj, str.str, str.length);
		}
		NBString_release(&str);
	}
	return r;
}

BOOL NBX509_createFromDERBytes(STNBX509* obj, const void* data, UI32 dataSz){ //binary
	BOOL r = FALSE;
	if(obj->opaque == NULL && data != NULL && dataSz > 0){
#		ifdef NB_ENABLE_OPENSSL
		const BYTE* tmpPtr = (const BYTE*)data; //use of variable is mandatory
		X509* x509 = d2i_X509(NULL, &tmpPtr, dataSz);
		if(x509 != NULL){
			STNBX509Opq* opq = obj->opaque = NBMemory_allocType(STNBX509Opq);
			NBMemory_setZeroSt(*opq, STNBX509Opq);
			opq->x509		= x509;
			NBThreadMutex_init(&opq->mutex);
			r = TRUE;
			/*STNBString str;
			 NBString_init(&str);
			 NBString_empty(&str);
			 NBX509_concatFingerprint(obj, &str);
			 PRINTF_INFO("NBX509_concatFingerprint (%d bytes): '%s'.\n", str.length, str.str);
			 NBString_empty(&str);
			 NBX509_concatFingerprintHex(obj, &str);
			 PRINTF_INFO("NBX509_concatFingerprintHex (%d bytes): '%s'.\n", str.length, str.str);
			 NBString_empty(&str);
			 NBX509_concatFingerprintBase64(obj, &str);
			 PRINTF_INFO("NBX509_concatFingerprintBase64 (%d bytes): '%s'.\n", str.length, str.str);
			 NBString_empty(&str);
			 NBX509_concatPubKeyHash(obj, &str);
			 PRINTF_INFO("NBX509_concatPubKeyHash (%d bytes): '%s'.\n", str.length, str.str);
			 NBString_empty(&str);
			 NBX509_concatPubKeyHashHex(obj, &str);
			 PRINTF_INFO("NBX509_concatPubKeyHashHex (%d bytes): '%s'.\n", str.length, str.str);
			 NBString_empty(&str);
			 NBX509_concatPubKeyHashBase64(obj, &str);
			 PRINTF_INFO("NBX509_concatPubKeyHashBase64 (%d bytes): '%s'.\n", str.length, str.str);
			 NBString_release(&str);*/
		}
#		endif
	}
	return r;
}

BOOL NBX509_createFromDERFile(STNBX509* obj, STNBFileRef file){ //binary
	BOOL r = FALSE;
	if(obj->opaque == NULL){
#		ifdef NB_ENABLE_OPENSSL
		if(NBFile_isSet(file)){
			STNBString str;
			NBString_init(&str);
			//Read file to BIO
			{
				char buff[16];
				while(TRUE){
					int read = NBFile_read(file, buff, sizeof(buff));
					if(read <= 0){
						break;
					} else {
						NBString_concatBytes(&str, buff, read);
					}
				};
			}
			//Load cert from BIO
			{
				const BYTE* tmpPtr = (const BYTE*)str.str; //use of variable is mandatory
				X509* x509 = d2i_X509(NULL, &tmpPtr, str.length);
				if(x509 != NULL){
					STNBX509Opq* opq = obj->opaque = NBMemory_allocType(STNBX509Opq);
					NBMemory_setZeroSt(*opq, STNBX509Opq);
					opq->x509		= x509;
					NBThreadMutex_init(&opq->mutex);
					r = TRUE;
					/*STNBString str;
					 NBString_init(&str);
					 NBString_empty(&str);
					 NBX509_concatFingerprint(obj, &str);
					 PRINTF_INFO("NBX509_concatFingerprint (%d bytes): '%s'.\n", str.length, str.str);
					 NBString_empty(&str);
					 NBX509_concatFingerprintHex(obj, &str);
					 PRINTF_INFO("NBX509_concatFingerprintHex (%d bytes): '%s'.\n", str.length, str.str);
					 NBString_empty(&str);
					 NBX509_concatFingerprintBase64(obj, &str);
					 PRINTF_INFO("NBX509_concatFingerprintBase64 (%d bytes): '%s'.\n", str.length, str.str);
					 NBString_empty(&str);
					 NBX509_concatPubKeyHash(obj, &str);
					 PRINTF_INFO("NBX509_concatPubKeyHash (%d bytes): '%s'.\n", str.length, str.str);
					 NBString_empty(&str);
					 NBX509_concatPubKeyHashHex(obj, &str);
					 PRINTF_INFO("NBX509_concatPubKeyHashHex (%d bytes): '%s'.\n", str.length, str.str);
					 NBString_empty(&str);
					 NBX509_concatPubKeyHashBase64(obj, &str);
					 PRINTF_INFO("NBX509_concatPubKeyHashBase64 (%d bytes): '%s'.\n", str.length, str.str);
					 NBString_release(&str);*/
				}
			}
			NBString_release(&str);
		}
#		endif
	}
	return r;
}

BOOL NBX509_createFromPEMBytes(STNBX509* obj, const char* strX509, const UI32 strX509Sz){ //base64-text
	BOOL r = FALSE;
	if(obj->opaque == NULL && strX509 != NULL && strX509Sz > 0){
#		ifdef NB_ENABLE_OPENSSL
		BIO* bio = BIO_new(BIO_s_mem());
		if(BIO_write(bio, strX509, strX509Sz) == strX509Sz){
			X509* x509 = PEM_read_bio_X509(bio, NULL, NULL, NULL);
			if(x509 != NULL){
				STNBX509Opq* opq = obj->opaque = NBMemory_allocType(STNBX509Opq);
				NBMemory_setZeroSt(*opq, STNBX509Opq);
				opq->x509		= x509;
				NBThreadMutex_init(&opq->mutex);
				r = TRUE;
				/*STNBString str;
				 NBString_init(&str);
				 NBString_empty(&str);
				 NBX509_concatFingerprint(obj, &str);
				 PRINTF_INFO("NBX509_concatFingerprint (%d bytes): '%s'.\n", str.length, str.str);
				 NBString_empty(&str);
				 NBX509_concatFingerprintHex(obj, &str);
				 PRINTF_INFO("NBX509_concatFingerprintHex (%d bytes): '%s'.\n", str.length, str.str);
				 NBString_empty(&str);
				 NBX509_concatFingerprintBase64(obj, &str);
				 PRINTF_INFO("NBX509_concatFingerprintBase64 (%d bytes): '%s'.\n", str.length, str.str);
				 NBString_empty(&str);
				 NBX509_concatPubKeyHash(obj, &str);
				 PRINTF_INFO("NBX509_concatPubKeyHash (%d bytes): '%s'.\n", str.length, str.str);
				 NBString_empty(&str);
				 NBX509_concatPubKeyHashHex(obj, &str);
				 PRINTF_INFO("NBX509_concatPubKeyHashHex (%d bytes): '%s'.\n", str.length, str.str);
				 NBString_empty(&str);
				 NBX509_concatPubKeyHashBase64(obj, &str);
				 PRINTF_INFO("NBX509_concatPubKeyHashBase64 (%d bytes): '%s'.\n", str.length, str.str);
				 NBString_release(&str);*/
			}
		}
		BIO_free(bio);
#		endif
	}
	return r;
}

BOOL NBX509_createFromPEMFile(STNBX509* obj, STNBFileRef file){ //base64-text
	BOOL r = FALSE;
	if(obj->opaque == NULL){
#		ifdef NB_ENABLE_OPENSSL
		if(NBFile_isSet(file)){
			BIO* bio = BIO_new(BIO_s_mem());
			//Read file to BIO
			{
				char buff[16];
				while(TRUE){
					int read = NBFile_read(file, buff, sizeof(buff));
					if(read <= 0){
						break;
					} else {
						BIO_write(bio, buff, read);
					}
				};
			}
			//Load cert from BIO
			{
				X509* x509 = PEM_read_bio_X509(bio, NULL, NULL, NULL);
				if(x509 != NULL){
					STNBX509Opq* opq = obj->opaque = NBMemory_allocType(STNBX509Opq);
					NBMemory_setZeroSt(*opq, STNBX509Opq);
					opq->x509		= x509;
					NBThreadMutex_init(&opq->mutex);
					r = TRUE;
					/*STNBString str;
					NBString_init(&str);
					NBString_empty(&str);
					NBX509_concatFingerprint(obj, &str);
					PRINTF_INFO("NBX509_concatFingerprint (%d bytes): '%s'.\n", str.length, str.str);
					NBString_empty(&str);
					NBX509_concatFingerprintHex(obj, &str);
					PRINTF_INFO("NBX509_concatFingerprintHex (%d bytes): '%s'.\n", str.length, str.str);
					NBString_empty(&str);
					NBX509_concatFingerprintBase64(obj, &str);
					PRINTF_INFO("NBX509_concatFingerprintBase64 (%d bytes): '%s'.\n", str.length, str.str);
					NBString_empty(&str);
					NBX509_concatPubKeyHash(obj, &str);
					PRINTF_INFO("NBX509_concatPubKeyHash (%d bytes): '%s'.\n", str.length, str.str);
					NBString_empty(&str);
					NBX509_concatPubKeyHashHex(obj, &str);
					PRINTF_INFO("NBX509_concatPubKeyHashHex (%d bytes): '%s'.\n", str.length, str.str);
					NBString_empty(&str);
					NBX509_concatPubKeyHashBase64(obj, &str);
					PRINTF_INFO("NBX509_concatPubKeyHashBase64 (%d bytes): '%s'.\n", str.length, str.str);
					NBString_release(&str);*/
				}
			}
			BIO_free(bio);
		}
#		endif
	}
	return r;
}

BOOL NBX509_getAsDERString(const STNBX509* obj, STNBString* dstStr){ //binary
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		NBThreadMutex_lock(&opq->mutex);
		{
			const UI32 len = i2d_X509(opq->x509, NULL);
			if(len > 0){
				NBString_empty(dstStr);
				NBString_concatRepeatedByte(dstStr, '\0', len);
				{
					BYTE* tmpPtr = (BYTE*)dstStr->str; //use of variable is mandatory
					i2d_X509(opq->x509, &tmpPtr);
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
BOOL NBX509_getAsPEMStringLocked_(X509* x509, STNBString* dstStr){
	BOOL r = FALSE;
	if(x509 != NULL){
		BIO* bio = BIO_new(BIO_s_mem());
		if(PEM_write_bio_X509(bio, x509) != 0){
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

BOOL NBX509_getAsPEMString(const STNBX509* obj, STNBString* dstStr){ //base64-text
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		NBThreadMutex_lock(&opq->mutex);
		{
			r = NBX509_getAsPEMStringLocked_(opq->x509, dstStr);
		}
		NBThreadMutex_unlock(&opq->mutex);
#		endif
	}
	return r;
}

//Names

//

#ifdef NB_ENABLE_OPENSSL
BOOL NBX509_concatNameOneline(X509_NAME* name, STNBString* dstStr){
	BOOL r = FALSE;
	if(name != NULL){
		const char* str = X509_NAME_oneline(name, NULL, 0);
		if(str != NULL){
			NBString_concat(dstStr, str);
			r = TRUE;
		}
	}
	return r;
}
#endif

#ifdef NB_ENABLE_OPENSSL
BOOL NBX509_concatNameNID(X509_NAME* name, const SI32 NID, STNBString* dstStr){
	BOOL r = FALSE;
	if(name != NULL){
		const SI32 sz = X509_NAME_get_text_by_NID(name, NID, NULL, 0);
		if(sz > 0){
			if(dstStr != NULL){
				const UI32 pos = dstStr->length;
				NBString_concatRepeatedByte(dstStr, '\0', sz);
				X509_NAME_get_text_by_NID(name, NID, &dstStr->str[pos], sz + 1);
			}
			r = TRUE;
		}
	}
	return r;
}
#endif

//Dates

BOOL NBX509_concatNotBeforeStr(const STNBX509* obj, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		NBThreadMutex_lock(&opq->mutex);
		if(opq->x509 != NULL){
			const ASN1_TIME* tm = X509_get0_notBefore(opq->x509);
			BIO* bio = BIO_new(BIO_s_mem());
			ASN1_TIME_print(bio, tm);
			{
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
		NBThreadMutex_unlock(&opq->mutex);
#		endif
	}
	return r;
}

BOOL NBX509_concatNotAfterStr(const STNBX509* obj, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		NBThreadMutex_lock(&opq->mutex);
		if(opq->x509 != NULL){
			const ASN1_TIME* tm = X509_get0_notAfter(opq->x509);
			BIO* bio = BIO_new(BIO_s_mem());
			ASN1_TIME_print(bio, tm);
			{
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
		NBThreadMutex_unlock(&opq->mutex);
#		endif
	}
	return r;
}

//Serial (unique per certificate signed by CA)

BOOL NBX509_concatSerial(const STNBX509* obj, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		NBThreadMutex_lock(&opq->mutex);
		if(opq->x509 != NULL){
			const ASN1_INTEGER* n = X509_get0_serialNumber(opq->x509);
			NBString_concatBytes(dstStr, (const char*)n->data, n->length);
			r = TRUE;
		}
		NBThreadMutex_unlock(&opq->mutex);
#		endif
	}
	return r;
}

BOOL NBX509_concatSerialHex(const STNBX509* obj, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		NBThreadMutex_lock(&opq->mutex);
		if(opq->x509 != NULL){
			const ASN1_INTEGER* n = X509_get0_serialNumber(opq->x509);
			NBString_concatBytesHex(dstStr, (const char*)n->data, n->length);
			r = TRUE;
		}
		NBThreadMutex_unlock(&opq->mutex);
#		endif
	}
	return r;
}

//Subject

/*const SI32 count = X509_NAME_entry_count(name);
 SI32 i; for(i = 0; i < count; i++){
 X509_NAME_ENTRY* e = X509_NAME_get_entry(name, i);
 ASN1_STRING* d = X509_NAME_ENTRY_get_data(e);
 ASN1_OBJECT* o = X509_NAME_ENTRY_get_object(e);
 }*/

BOOL NBX509_concatSubjectFullLine(const STNBX509* obj, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		if(opq->x509 != NULL){
			r = NBX509_concatNameOneline(X509_get_subject_name(opq->x509), dstStr);
		}
#		endif
	}
	return r;
}

BOOL NBX509_concatSubjectCommName(const STNBX509* obj, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		if(opq->x509 != NULL){
			return NBX509_concatNameNID(X509_get_subject_name(opq->x509), NID_commonName, dstStr);
		}
#		endif
	}
	return r;
}

BOOL NBX509_concatSubjectOrgName(const STNBX509* obj, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		if(opq->x509 != NULL){
			return NBX509_concatNameNID(X509_get_subject_name(opq->x509), NID_organizationName, dstStr);
		}
#		endif
	}
	return r;
}

BOOL NBX509_concatSubjectOrgUnitName(const STNBX509* obj, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		if(opq->x509 != NULL){
			return NBX509_concatNameNID(X509_get_subject_name(opq->x509), NID_organizationalUnitName, dstStr);
		}
#		endif
	}
	return r;
}

BOOL NBX509_concatSubjectCountryName(const STNBX509* obj, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		if(opq->x509 != NULL){
			return NBX509_concatNameNID(X509_get_subject_name(opq->x509), NID_countryName, dstStr);
		}
#		endif
	}
	return r;
}

BOOL NBX509_concatSubjectStateName(const STNBX509* obj, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		if(opq->x509 != NULL){
			return NBX509_concatNameNID(X509_get_subject_name(opq->x509), NID_stateOrProvinceName, dstStr);
		}
#		endif
	}
	return r;
}

BOOL NBX509_concatSubjectLocatityName(const STNBX509* obj, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		if(opq->x509 != NULL){
			return NBX509_concatNameNID(X509_get_subject_name(opq->x509), NID_localityName, dstStr);
		}
#		endif
	}
	return r;
}

BOOL NBX509_concatSubjectU(const STNBX509* obj, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		if(opq->x509 != NULL){
			return NBX509_concatNameNID(X509_get_subject_name(opq->x509), NID_organizationName, dstStr);
		}
#		endif
	}
	return r;
}


BOOL NBX509_concatIssuerFullLine(const STNBX509* obj, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		if(opq->x509 != NULL){
			r = NBX509_concatNameOneline(X509_get_issuer_name(opq->x509), dstStr);
		}
#		endif
	}
	return r;
}

BOOL NBX509_concatIssuerDER(const STNBX509* obj, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		if(opq->x509 != NULL){
			X509_NAME* name = X509_get_issuer_name(opq->x509);
			const UI32 len = i2d_X509_NAME(name, NULL);
			if(len > 0){
				const UI32 pos = dstStr->length;
				NBString_concatRepeatedByte(dstStr, '\0', len);
				{
					BYTE* tmpPtr = (BYTE*)&dstStr->str[pos]; //use of variable is mandatory
					i2d_X509_NAME(name, &tmpPtr);
					NBASSERT(tmpPtr == (BYTE*)(dstStr->str + dstStr->length))
				}
				r = TRUE;
			}
		}
#		endif
	}
	return r;
}

BOOL NBX509_concatIssuerCommName(const STNBX509* obj, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		if(opq->x509 != NULL){
			r = NBX509_concatNameNID(X509_get_issuer_name(opq->x509), NID_commonName, dstStr);
		}
#		endif
	}
	return r;
}

BOOL NBX509_concatIssuerOrgName(const STNBX509* obj, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		if(opq->x509 != NULL){
			r = NBX509_concatNameNID(X509_get_issuer_name(opq->x509), NID_organizationName, dstStr);
		}
#		endif
	}
	return r;
}

BOOL NBX509_concatIssuerOrgUnitName(const STNBX509* obj, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		if(opq->x509 != NULL){
			r = NBX509_concatNameNID(X509_get_issuer_name(opq->x509), NID_organizationalUnitName, dstStr);
		}
#		endif
	}
	return r;
}

BOOL NBX509_concatIssuerCountryName(const STNBX509* obj, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		if(opq->x509 != NULL){
			r = NBX509_concatNameNID(X509_get_issuer_name(opq->x509), NID_countryName, dstStr);
		}
#		endif
	}
	return r;
}

BOOL NBX509_concatIssuerStateName(const STNBX509* obj, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		if(opq->x509 != NULL){
			r = NBX509_concatNameNID(X509_get_issuer_name(opq->x509), NID_stateOrProvinceName, dstStr);
		}
#		endif
	}
	return r;
}

BOOL NBX509_concatIssuerLocatityName(const STNBX509* obj, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		if(opq->x509 != NULL){
			r = NBX509_concatNameNID(X509_get_issuer_name(opq->x509), NID_localityName, dstStr);
		}
#		endif
	}
	return r;
}

//Certificate fingerprint

BOOL NBX509_concatFingerprint(const STNBX509* obj, const ENNBHashType type, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		switch(type) {
			case ENNBHashType_Sha1:
				{
					unsigned int len = 0;
					unsigned char digest[EVP_MAX_MD_SIZE];
					if(X509_digest(opq->x509, EVP_sha1(), digest, &len)){
						NBASSERT(len == 20)
						if(len > 0){
							NBString_concatBytes(dstStr, (const char*)digest, len);
							r = TRUE;
						}
					}
				}
				break;
			default:
				break;
		}
#		endif
	}
	return r;
}

BOOL NBX509_concatFingerprintHex(const STNBX509* obj, const ENNBHashType type, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		switch(type) {
			case ENNBHashType_Sha1:
				{
					unsigned int len = 0;
					unsigned char digest[EVP_MAX_MD_SIZE];
					if(X509_digest(opq->x509, EVP_sha1(), digest, &len)){
						NBASSERT(len == 20)
						if(len > 0){
							char hexs[41]; NBSha1_hashBinToHexChars(digest, hexs);
							NBString_concatBytes(dstStr, (const char*)hexs, 40);
							r = TRUE;
						}
					}
				}
				break;
			default:
				break;
		}
#		endif
	}
	return r;
}

BOOL NBX509_concatFingerprintBase64(const STNBX509* obj, const ENNBHashType type, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		switch(type) {
			case ENNBHashType_Sha1:
				{
					unsigned int len = 0;
					unsigned char digest[EVP_MAX_MD_SIZE];
					if(X509_digest(opq->x509, EVP_sha1(), digest, &len)){
						NBASSERT(len == 20)
						if(len > 0){
							NBBase64_codeBytes(dstStr, (const char*)digest, 20);
							r = TRUE;
						}
					}
				}
				break;
			default:
				break;
		}
#		endif
	}
	return r;
}

//Public key pin

BOOL NBX509_concatPubKeyHash(const STNBX509* obj, const ENNBHashType type, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		X509_PUBKEY* pkey = X509_get_X509_PUBKEY(opq->x509);
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
						break;
				}
				NBMemory_free(data);
			}
		}
#		endif
	}
	return r;
}

BOOL NBX509_concatPubKeyHashHex(const STNBX509* obj, const ENNBHashType type, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		X509_PUBKEY* pkey = X509_get_X509_PUBKEY(opq->x509);
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
						break;
				}
				NBMemory_free(data);
			}
		}
#		endif
	}
	return r;
}

BOOL NBX509_concatPubKeyHashBase64(const STNBX509* obj, const ENNBHashType type, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		X509_PUBKEY* pkey = X509_get_X509_PUBKEY(opq->x509);
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
						break;
				}
				NBMemory_free(data);
			}
		}
#		endif
	}
	return r;
}

//Certificate body (for signature verification)

BOOL NBX509_concatTbsDER(const STNBX509* obj, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		const SI32 len = i2d_re_X509_tbs(opq->x509, NULL);
		if(len > 0){
			const UI32 pos = dstStr->length;
			NBString_concatRepeatedByte(dstStr, '\0', len);
			BYTE* tmpPtr = (BYTE*)&dstStr->str[pos]; //use of variable is mandatory
			i2d_re_X509_tbs(opq->x509, &tmpPtr);
			NBASSERT(tmpPtr == (BYTE*)(dstStr->str + dstStr->length))
		}
#		endif
	}
	return r;
}

BOOL NBX509_concatTbsDERHash(const STNBX509* obj, const ENNBHashType type, STNBString* dstStr){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		const int len = i2d_re_X509_tbs(opq->x509, NULL);
		if(len > 0){
			BYTE* data = (BYTE*)NBMemory_alloc(len);
			{
				BYTE* tmpPtr = (BYTE*)data; //use of variable is mandatory
				i2d_re_X509_tbs(opq->x509, &tmpPtr);
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
					break;
			}
			NBMemory_free(data);
		}
#		endif
	}
	return r;
}

//Signature
BOOL NBX509_isSignedBy(const STNBX509* obj, const STNBX509* signer){
	BOOL r = FALSE;
	if(obj->opaque != NULL && signer->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		NBThreadMutex_lock(&opq->mutex);
		{
			const int len = i2d_re_X509_tbs(opq->x509, NULL);
			if(len > 0){
				BYTE* data = (BYTE*)NBMemory_alloc(len);
				{
					BYTE* tmpPtr = (BYTE*)data; //use of variable is mandatory
					i2d_re_X509_tbs(opq->x509, &tmpPtr);
				}
				STNBX509Opq* opq2 = (STNBX509Opq*)signer->opaque;
				const ASN1_BIT_STRING* sig = NULL; const X509_ALGOR* alg = NULL; //Internal pointer, do not free.
				EVP_PKEY* signkey = X509_get0_pubkey(opq2->x509); //does not increment retainCount
				X509_get0_signature(&sig, &alg, opq->x509);
				if(signkey != NULL && sig != NULL && alg != NULL){
					const ASN1_OBJECT* alg2 = NULL; int alg3;
					X509_ALGOR_get0(&alg2, NULL, NULL, alg);
					alg3 = OBJ_obj2nid(alg2);
					{
						EVP_MD_CTX* ctx = EVP_MD_CTX_create();
						EVP_MD_CTX_init(ctx);
						const EVP_MD* md = EVP_get_digestbynid(alg3);
						if(!EVP_VerifyInit_ex(ctx, md, NULL)){
							PRINTF_ERROR("NBX509, could not EVP_VerifyInit_ex.\n");
						} else if(!EVP_VerifyUpdate(ctx, data, len)){
							PRINTF_ERROR("NBX509, could not EVP_VerifyUpdate.\n");
						} else if(EVP_VerifyFinal(ctx, sig->data, sig->length, signkey) == 1){
							r = TRUE;
						}
						EVP_MD_CTX_destroy(ctx);
					}
				}
				NBMemory_free(data);
			}
		}
		NBThreadMutex_unlock(&opq->mutex);
#		endif
	}
	return r;
}

BOOL NBX509_isValidSignature(const STNBX509* obj, const void* data, const SI32 dataSz, const void* singBin, const SI32 signBinSz){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		NBThreadMutex_lock(&opq->mutex);
		{
			EVP_PKEY* pkey = X509_get0_pubkey(opq->x509); //does not increment retainCount
			EVP_MD_CTX* ctx = EVP_MD_CTX_create();
			if(!EVP_VerifyInit(ctx, EVP_sha1())){
				PRINTF_ERROR("NBX509, could not EVP_VerifyInit_ex.\n");
			} else if(!EVP_VerifyUpdate(ctx, data, dataSz)){
				PRINTF_ERROR("NBX509, could not EVP_VerifyUpdate.\n");
			} else if(EVP_VerifyFinal(ctx, singBin, signBinSz, pkey) == 1){
				r = TRUE;
			}
			EVP_MD_CTX_destroy(ctx);
		}
		NBThreadMutex_unlock(&opq->mutex);
#		endif
	}
	return r;
}

//Encryption

BOOL NBX509_concatEncryptedBytes(STNBX509* obj, const void* data, const SI32 dataSz, STNBString* dstEnc){
	BOOL r = FALSE;
	if(obj->opaque != NULL && dataSz < 245){ //RSA has a limit in size ~245 bytes
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		NBThreadMutex_lock(&opq->mutex);
		{
			EVP_PKEY* pkey	= X509_get0_pubkey(opq->x509); //does not increment retainCount
			RSA* rsa		= EVP_PKEY_get0_RSA(pkey);
			int rsaLen		= RSA_size(rsa);
			unsigned char* buff = (unsigned char*)NBMemory_alloc(rsaLen) ;
			{
				int rr = RSA_public_encrypt(dataSz, (const unsigned char*)data, buff, rsa, RSA_PKCS1_PADDING);
				if(rr < 0){
					PRINTF_ERROR("NBX509, could not RSA_public_encrypt.\n");
				} else {
					if(dstEnc != NULL){
						NBString_concatBytes(dstEnc, (const char*)buff, rr);
					}
					r = TRUE;
				}
			}
			NBMemory_free(buff);
		}
		NBThreadMutex_unlock(&opq->mutex);
#		endif
	}
	return r;
}

BOOL NBX509_concatDecryptedBytes(STNBX509* obj, const void* data, const SI32 dataSz, STNBString* dstEnc){
	BOOL r = FALSE;
	if(obj->opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBX509Opq* opq = (STNBX509Opq*)obj->opaque;
		NBThreadMutex_lock(&opq->mutex);
		{
			EVP_PKEY* pkey	= X509_get0_pubkey(opq->x509); //does not increment retainCount
			RSA* rsa		= EVP_PKEY_get0_RSA(pkey);
			int rsaLen		= RSA_size(rsa);
			unsigned char* buff = (unsigned char*)NBMemory_alloc(rsaLen) ;
			{
				int rr = RSA_public_decrypt(dataSz, (const unsigned char*)data, buff, rsa, RSA_PKCS1_PADDING);
				if(rr < 0){
					PRINTF_ERROR("NBX509, could not RSA_public_decrypt.\n");
				} else {
					if(dstEnc != NULL){
						NBString_concatBytes(dstEnc, (const char*)buff, rr);
					}
					r = TRUE;
				}
			}
			NBMemory_free(buff);
		}
		NBThreadMutex_unlock(&opq->mutex);
#		endif
	}
	return r;
}

//Generate

#ifdef NB_ENABLE_OPENSSL
int NBX509_addExt(X509 *cert, int nid, char *value){
	X509_EXTENSION *ex;
	X509V3_CTX ctx;
	/* This sets the 'context' of the extensions. */
	/* No configuration database */
	X509V3_set_ctx_nodb(&ctx);
	/* Issuer and subject certs: both the target since it is self signed,
	 * no request and no CRL
	 */
	X509V3_set_ctx(&ctx, cert, cert, NULL, NULL, 0);
	ex = X509V3_EXT_conf_nid(NULL, &ctx, nid, value);
	if (!ex)
		return 0;
	
	X509_add_ext(cert,ex,-1);
	X509_EXTENSION_free(ex);
	return 1;
}
#endif

#ifdef NB_ENABLE_OPENSSL
BOOL NBX509_createSelfSigned_(STNBX509* obj, X509* x509, long version, ASN1_INTEGER* serial, const SI32 bits, const UI32 days, const STNBX500NamePair* namePairs, const UI32 namePairsSz, EVP_PKEY* keys, STNBPKey* dstPrivKey){
	BOOL r = FALSE;
	//Good reading:	https://gist.github.com/Soarez/9688998
	//X509-RFC:		https://tools.ietf.org/html/rfc5280#section-4.1.2.4
	//------------------------------------
	//A X.509 v3 digital certificate has this structure:
	//------------------------------------
	//- Certificate
	//  + Version
	//  + Serial Number
	//  - Algorithm ID
	//  + Issuer
	//  + Validity
	//    - Not Before
	//    - Not After
	//  - Subject
	//  - Subject public key info
	//  - Issuer Unique Identifier (optional)
	//  - Subject Unique Identifier (optional)
	//  - Extensions (optional)
	//     ...
	//- Certificate Signature Algorithm
	//- Certificate Signature
	//------------------------------------
	{
		//Version
		X509_set_version(x509, version);
		//Serial
		X509_set_serialNumber(x509, serial);
		//ASN1_INTEGER_set(X509_get_serialNumber(x509), serial); //ToDo: remove
		//Issuer
		{
			X509_NAME* name = X509_get_subject_name(x509);
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
			X509_set_issuer_name(x509, name);
		}
		//Validity
		{
			//Not Before
			X509_gmtime_adj(X509_get_notBefore(x509), 0);
			//Not After
			X509_gmtime_adj(X509_get_notAfter(x509), (long)(60 * 60 * 24 * days));
		}
		//Subject public key info
		X509_set_pubkey(x509, keys);
		//Extensions (optional)
		{
			//Add standard extensions
			//CA, the certified public key may be used to verify certificate signatures
			NBX509_addExt(x509, NID_basic_constraints, "critical,CA:TRUE");
			//The "keyCertSign" bit is asserted when the subject public key is
			//used for verifying signatures on public key certificates.
			//The "cRLSign" bit is asserted when the subject public key is used
			//for verifying signatures on certificate revocation lists (e.g.,
			//CRLs, delta CRLs, or ARLs).
			NBX509_addExt(x509, NID_key_usage, "critical,digitalSignature,keyEncipherment,dataEncipherment,keyCertSign,cRLSign");
			//The subject key identifier extension provides a means of identifying
			//certificates that contain a particular public key.
			//NBX509_addExt(x509, NID_subject_key_identifier, "hash");
			//--
			//Example reading alt-names: https://stackoverflow.com/questions/15875494/how-to-retrieve-issuer-alternative-name-for-ssl-certificate-by-openssl
			/*UI32 i; for(i = 0; i < altNamesSz; i++){
			 const char* altName = altNames[i];
			 NBX509_addExt(x509, NID_subject_alt_name, (char*)altName);
			 }*/
		}
		//Certificate Signature Algorithm
		//Certificate Signature
		if(!X509_sign(x509, keys, EVP_sha1())){
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
					STNBX509Opq* opq = obj->opaque = NBMemory_allocType(STNBX509Opq);
					NBMemory_setZeroSt(*opq, STNBX509Opq);
					opq->x509		= x509;
					NBThreadMutex_init(&opq->mutex);
					r = TRUE;
				}
				NBMemory_free(data);
			}
		}
	}
	return r;
}
#endif
	
BOOL NBX509_createSelfSigned(STNBX509* obj, const SI32 bits, const UI32 days, const STNBX500NamePair* namePairs, const UI32 namePairsSz, STNBPKey* dstPrivKey){
	BOOL r = FALSE;
	if(obj->opaque == NULL){
#		ifdef NB_ENABLE_OPENSSL
		//------------------------
		//- Create new certificate
		//------------------------
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
							X509* x509 = X509_new();
							if(x509 == NULL){
								PRINTF_ERROR("Could not create X509.\n");
							} else {
								//Version
								long version = 2; // 2 = version 3
								//Serial Number (must be unique per issuer)
								BIGNUM* rndBN = BN_new();
								ASN1_INTEGER* rndSerial = ASN1_INTEGER_new();
								if(!BN_pseudo_rand(rndBN, 64, 0, 0)){
									PRINTF_ERROR("Could not BN_pseudo_rand for serial number.\n");
								} else if (!BN_to_ASN1_INTEGER(rndBN, rndSerial)){
									PRINTF_ERROR("Could not BN_to_ASN1_INTEGER for serial number.\n");
								} else if(!NBX509_createSelfSigned_(obj, x509, version, rndSerial, bits, days, namePairs, namePairsSz, keys, dstPrivKey)){
									PRINTF_ERROR("Could not NBX509_createSelfSigned_.\n");
								} else {
									x509	= NULL;
									r		= TRUE;
								}
								if(rndSerial != NULL){
									ASN1_INTEGER_free(rndSerial);
									rndSerial = NULL;
								}
								if(rndBN != NULL){
									BN_free(rndBN);
									rndBN = NULL;
								}
								if(x509 != NULL){
									X509_free(x509);
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

BOOL NBX509_createSelfSignedFrom(STNBX509* obj, const SI32 bits, const UI32 days, const STNBX500NamePair* namePairs, const UI32 namePairsSz, STNBX509* baseCert, STNBPKey* baseKey, STNBPKey* dstPrivKey){
	BOOL r = FALSE;
	if(obj->opaque == NULL){
#		ifdef NB_ENABLE_OPENSSL
		BIGNUM* rndBN			= NULL;
		ASN1_INTEGER* serial	= NULL;
		EVP_PKEY* keys			= NULL;
		//Load or create serial
		{
			//Load serial
			if(baseCert != NULL){
				STNBX509Opq* opq = (STNBX509Opq*)baseCert->opaque;
				if(opq->x509 != NULL){
					serial = X509_get_serialNumber(opq->x509);
				}
			}
			//Create serial
			if(serial == NULL){
				BIGNUM* rndBN2 = BN_new();
				ASN1_INTEGER* rndSerial2 = ASN1_INTEGER_new();
				if(!BN_pseudo_rand(rndBN2, 64, 0, 0)){
					PRINTF_ERROR("Could not BN_pseudo_rand for serial number.\n");
				} else if (!BN_to_ASN1_INTEGER(rndBN2, rndSerial2)){
					PRINTF_ERROR("Could not BN_to_ASN1_INTEGER for serial number.\n");
				} else {
					rndBN = rndBN2; rndBN2 = NULL;
					serial = rndSerial2; rndSerial2 = NULL;
				}
				if(rndBN2 != NULL){
					BN_free(rndBN2);
					rndBN2 = NULL;
				}
				if(rndSerial2 != NULL){
					ASN1_INTEGER_free(rndSerial2);
					rndSerial2 = NULL;
				}
			}
		}
		//Load key
		if(baseKey != NULL){
			if(NBPKey_isCreated(baseKey)){
				STNBString str;
				NBString_init(&str);
				if(NBPKey_getAsDERString(baseKey, &str)){
					const BYTE* tmpPtr = (const BYTE*)str.str; //use of variable is mandatory
					keys = d2i_AutoPrivateKey(NULL, &tmpPtr, str.length);
					NBASSERT(tmpPtr == (BYTE*)(str.str + str.length))
				}
				NBString_release(&str);
			}
		}
		//Create cert
		if(serial != NULL && keys != NULL){
			//Version
			long version = 2; // 2 = version 3
			X509* x509 = X509_new();
			if(x509 == NULL){
				PRINTF_ERROR("Could not create X509.\n");
			} else if(!NBX509_createSelfSigned_(obj, x509, version, serial, bits, days, namePairs, namePairsSz, keys, dstPrivKey)){
				PRINTF_ERROR("Could not NBX509_createSelfSigned_.\n");
			} else {
				x509	= NULL;
				r		= TRUE;
			}
			if(x509 != NULL){
				X509_free(x509);
				x509 = NULL;
			}
		}
		//Release keys
		if(keys != NULL){
			EVP_PKEY_free(keys);
			keys = NULL;
		}
		//Release serial
		if(rndBN != NULL){
			if(serial != NULL){
				ASN1_INTEGER_free(serial);
				serial = NULL;
			}
			BN_free(rndBN);
			rndBN = NULL;
		}
#		endif
	}
	return r;
}

BOOL NBX509_createFromRequest(STNBX509* obj, const STNBX509Req* pCSRReq, const STNBX509* pCACert, const STNBPKey* pCAPrivKey, const UI32 bits, const UI32 days, const void* serial, const UI32 serialSz){
	BOOL r = FALSE;
	//rfc5280, serial must be up to 20 octets
	if(obj->opaque == NULL && pCSRReq != NULL && pCACert != NULL && pCAPrivKey != NULL && serialSz > 0 && serialSz <= 20){
#		ifdef NB_ENABLE_OPENSSL
		//Load data
		X509_REQ* req	= NULL;
		X509* caCert	= NULL;
		EVP_PKEY* caKey	= NULL;
		{
			STNBString str;
			NBString_init(&str);
			{
				NBString_empty(&str);
				if(NBX509Req_getAsDERString(pCSRReq, &str)){
					const BYTE* tmpPtr = (const BYTE*)str.str; //use of variable is mandatory
					req = d2i_X509_REQ(NULL, &tmpPtr, str.length);
					NBASSERT(tmpPtr == (BYTE*)(str.str + str.length))
				}
			}
			{
				NBString_empty(&str);
				if(NBX509_getAsDERString(pCACert, &str)){
					const BYTE* tmpPtr = (const BYTE*)str.str; //use of variable is mandatory
					caCert = d2i_X509(NULL, &tmpPtr, str.length);
					NBASSERT(tmpPtr == (BYTE*)(str.str + str.length))
				}
			}
			{
				NBString_empty(&str);
				if(NBPKey_getAsDERString(pCAPrivKey, &str)){
					const BYTE* tmpPtr = (const BYTE*)str.str; //use of variable is mandatory
					caKey = d2i_AutoPrivateKey(NULL, &tmpPtr, str.length);
					NBASSERT(tmpPtr == (BYTE*)(str.str + str.length))
				}
			}
			NBString_release(&str);
		}
		//Generate certificate
		if(req != NULL && caCert != NULL && caKey != NULL){
			X509* x509 = X509_new();
			if(x509 == NULL){
				PRINTF_ERROR("Could not create X509.\n");
			} else {
				//Version
				X509_set_version(x509, 2); //2 = version 3
				//Serial Number (must be unique per issuer)
				ASN1_OCTET_STRING* sSerial = ASN1_OCTET_STRING_new();
				ASN1_OCTET_STRING_set(sSerial, serial, serialSz);
				/*BIGNUM* rndBN = BN_new();
				ASN1_INTEGER* rndSerial = ASN1_INTEGER_new();
				if(!BN_pseudo_rand(rndBN, 64, 0, 0)){
					PRINTF_ERROR("Could not BN_pseudo_rand for serial number.\n");
				} else if (!BN_to_ASN1_INTEGER(rndBN, rndSerial)){
					PRINTF_ERROR("Could not BN_to_ASN1_INTEGER for serial number.\n");
				} else*/
				if(X509_set_serialNumber(x509, sSerial) == 0){
					PRINTF_ERROR("Could not set %d bytes serial.\n", serialSz);
				} else {
					//Subject name
					{
						X509_NAME* subject = X509_REQ_get_subject_name(req);
						X509_set_subject_name(x509, subject);
					}
					//Issuer name
					{
						X509_NAME* issuer = X509_get_subject_name(caCert);
						X509_set_issuer_name(x509, issuer);
					}
					//Public key
					{
						EVP_PKEY* req_pubkey = X509_REQ_get0_pubkey(req); //does not increment retainCount
						X509_set_pubkey(x509, req_pubkey);
					}
					//Validity
					{
						//Not Before
						X509_gmtime_adj(X509_get_notBefore(x509), 0);
						//Not After
						X509_gmtime_adj(X509_get_notAfter(x509), (long)(60 * 60 * 24 * days));
					}
					//Extensions (optional)
					{
						//Add standard extensions
						//CA, the certified public key may be used to verify certificate signatures
						NBX509_addExt(x509, NID_basic_constraints, "critical,CA:FALSE");
						//The "keyCertSign" bit is asserted when the subject public key is
						//used for verifying signatures on public key certificates.
						//The "cRLSign" bit is asserted when the subject public key is used
						//for verifying signatures on certificate revocation lists (e.g.,
						//CRLs, delta CRLs, or ARLs).
						NBX509_addExt(x509, NID_key_usage, "digitalSignature,keyEncipherment,dataEncipherment"); //keyEncipherment includes SSL handshake
						//The subject key identifier extension provides a means of identifying
						//certificates that contain a particular public key.
						//NBX509_addExt(x509, NID_subject_key_identifier, "hash");
						X509V3_CTX ctx;
						X509V3_set_ctx(&ctx, caCert, x509, NULL, NULL, 0);
					}
					//Certificate Signature Algorithm
					//Certificate Signature
					if(!X509_sign(x509, caKey, EVP_sha1())){
						PRINTF_ERROR("Could not sign certificate.\n");
					} else {
						//Save certificate
						STNBX509Opq* opq = obj->opaque = NBMemory_allocType(STNBX509Opq);
						NBMemory_setZeroSt(*opq, STNBX509Opq);
						opq->x509		= x509;
						NBThreadMutex_init(&opq->mutex);
						x509 = NULL;
						r = TRUE;
					}
				}
				ASN1_OCTET_STRING_free(sSerial);
				/*ASN1_INTEGER_free(rndSerial);
				BN_free(rndBN);*/
				if(x509 != NULL){
					X509_free(x509);
					x509 = NULL;
				}
			}
		}
		//Release
		if(req != NULL){
			X509_REQ_free(req);
			req = NULL;
		}
		if(caCert != NULL){
			X509_free(caCert);
			caCert = NULL;
		}
		if(caKey != NULL){
			EVP_PKEY_free(caKey);
			caKey = NULL;
		}
#		endif
	}
	return r;
}


