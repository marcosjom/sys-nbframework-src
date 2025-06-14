#include "nb/NBFrameworkPch.h"
#include "nb/ssl/NBSslContext.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBString.h"

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

//

//STNBSslContextRef _defaultNBSslContext = NB_OBJREF_NULL;

typedef struct STNBSslContextOpq_ {
	STNBObject			prnt;			
	ENNBSslContextMode	mode;
#	ifdef NB_ENABLE_OPENSSL
	SSL_CTX*			ctx;
	X509*				cert;
	EVP_PKEY*			pkey;
#	endif
} STNBSslContextOpq;

NB_OBJREF_BODY(NBSslContext, STNBSslContextOpq, NBObject)

void NBSslContext_initZeroed(STNBObject* obj) {
	STNBSslContextOpq* opq	= (STNBSslContextOpq*)obj;
	opq->mode = ENNBSslContextMode_none;
}

void NBSslContext_uninitLocked(STNBObject* obj){
	STNBSslContextOpq* opq	= (STNBSslContextOpq*)obj;
#	ifdef NB_ENABLE_OPENSSL
	if(opq->ctx){
		SSL_CTX_free(opq->ctx);
		opq->ctx = NULL;
	}
	if(opq->cert != NULL){
		X509_free(opq->cert);
		opq->cert = NULL;
	}
	if(opq->pkey != NULL){
		EVP_PKEY_free(opq->pkey);
		opq->pkey = NULL;
	}
#	endif
	opq->mode = ENNBSslContextMode_none;
}

//

//Error
void NBSslContext_concatLastErrStack(STNBString* dst){
    long errIdx = 0;
    unsigned long errcode = 0;
    if((errcode = ERR_get_error()) == 0){
        //
    } else {
        do {
            char buffStr[1024];
            ERR_error_string_n(errcode, buffStr, sizeof(buffStr));
            const char* eLib = ERR_lib_error_string(errcode);
            const char* eFnc = ERR_func_error_string(errcode);
            const char* eRsn = ERR_reason_error_string(errcode);
            if(dst != NULL){
                NBString_concat(dst, "#");
                NBString_concatSI64(dst, (errIdx + 1));
                NBString_concat(dst, " ");
                NBString_concat(dst, buffStr);
                NBString_concat(dst, " lib('");NBString_concat(dst, eLib); NBString_concat(dst, "')");
                NBString_concat(dst, " func('");NBString_concat(dst, eFnc); NBString_concat(dst, "')");
                NBString_concat(dst, " reason('");NBString_concat(dst, eRsn); NBString_concat(dst, "')");
                NBString_concat(dst, "\n");
            }
            errIdx++;
        } while((errcode = ERR_get_error()) != 0);
    }
}

void NBSslContext_printLastErrStack(const char* lastMethodCalled){
    long errIdx = 0;
    unsigned long errcode = 0;
    if((errcode = ERR_get_error()) == 0){
        PRINTF_ERROR("Ssl, #%ld '%s' failed: empty-error-stack.\n", (errIdx + 1), lastMethodCalled);
    } else {
        do {
            char buffStr[1024];
            ERR_error_string_n(errcode, buffStr, sizeof(buffStr));
            IF_PRINTF(const char* eLib = ERR_lib_error_string(errcode);)
            IF_PRINTF(const char* eFnc = ERR_func_error_string(errcode);)
            IF_PRINTF(const char* eRsn = ERR_reason_error_string(errcode);)
            PRINTF_ERROR("Ssl, #%ld '%s' failed: '%s' lib('%s') func('%s') reason('%s').\n", (errIdx + 1), lastMethodCalled, buffStr, eLib, eFnc, eRsn);
            /*const char* errstr = "";
             switch(errcode){
             case SSL_ERROR_NONE: errstr = "Error-none."; break;
             case SSL_ERROR_ZERO_RETURN: errstr = "The TLS/SSL connection has been closed."; break;
             case SSL_ERROR_WANT_READ: errstr = "The operation-read did not complete"; break;
             case SSL_ERROR_WANT_WRITE: errstr = "The operation-write did not complete"; break;
             case SSL_ERROR_WANT_CONNECT: errstr = "The operation-connect did not complete"; break;
             case SSL_ERROR_WANT_ACCEPT: errstr = "The operation-accept did not complete"; break;
             case SSL_ERROR_WANT_X509_LOOKUP: errstr = "The operation did not complete because an application callback set by SSL_CTX_set_client_cert_cb() has asked to be called again."; break;
             case SSL_ERROR_SYSCALL: errstr = "Some I/O error occurred. "; break;
             case SSL_ERROR_SSL: errstr = "A failure in the SSL library occurred, usually a protocol error. "; break;
             default: errstr = "Error-unknown"; break;
             }
             PRINTF_ERROR("Ssl, #%ld '%s' failed: ret(%d) sslerrorno(%ld) sslErr('%s')\n", (errIdx + 1), method, ret, errcode, errstr);*/
            errIdx++;
        } while((errcode = ERR_get_error()) != 0);
    }
}

//Create

void* NBSslContext_getClientMode(ENNBSslContextMode* dstMode){
#	ifdef NB_ENABLE_OPENSSL
	if (dstMode != NULL) *dstMode = ENNBSslContextMode_client;
	return (void*)TLS_client_method();
#	else
	return NULL;
#	endif
	//return (void*)SSLv23_client_method(); //ToDo: whats the difference
}

void* NBSslContext_getServerMode(ENNBSslContextMode* dstMode){
	if(dstMode != NULL) *dstMode = ENNBSslContextMode_server;
	return (void*)TLS_server_method();
	//return (void*)SSLv23_server_method(); //ToDo: whats the difference
}

void* NBSslContext_getBothMode(ENNBSslContextMode* dstMode){
#	ifdef NB_ENABLE_OPENSSL
	if (dstMode != NULL) *dstMode = ENNBSslContextMode_server;
	return (void*)TLS_server_method();
#	else
	return NULL;
#	endif
	//return (void*)SSLv23_method(); //ToDo: whats the difference
}

BOOL NBSslContext_isCreated(STNBSslContextRef ref){
	BOOL r = FALSE;
	NBASSERT(NBSslContext_isClass(ref));
	if(ref.opaque != NULL){
		STNBSslContextOpq* opq = (STNBSslContextOpq*)ref.opaque;
#		ifdef NB_ENABLE_OPENSSL
		NBObject_lock(opq);
		r = (opq->ctx != NULL ? TRUE : FALSE );
		NBObject_unlock(opq);
#		endif
	}
	return r;
}

BOOL NBSslContext_create(STNBSslContextRef ref, NBSsslContextModeFunc getModeFunc){
	BOOL r = FALSE;
	NBASSERT(NBSslContext_isClass(ref));
	if(ref.opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBSslContextOpq* opq = (STNBSslContextOpq*)ref.opaque;
		NBObject_lock(opq);
		NBASSERT(opq->ctx == NULL) //Create* method called more than once
		if(opq->ctx == NULL){
			if(getModeFunc != NULL){
				ENNBSslContextMode mode		= ENNBSslContextMode_none;
				const SSL_METHOD* sslMethods = (const SSL_METHOD*)((*getModeFunc)(&mode));
				if(sslMethods != NULL){
					SSL_CTX* ctx			= SSL_CTX_new(sslMethods);
					if(ctx != NULL){
						opq->mode			= mode;
						opq->ctx			= ctx;
						NBASSERT(opq->cert == NULL)
						NBASSERT(opq->pkey == NULL)
						r = TRUE;
					}
				}
			}
		}
		NBObject_unlock(opq);
#		endif
	}
	return r;
}

//ToDo: remove
/*BOOL NBSslContext_createAsRefTo(STNBSslContextRef ref, STNBSslContextRef other){
	BOOL r = FALSE;
	NBASSERT(NBSslContext_isClass(ref));
	if(ref.opaque != NULL && other->opaque != NULL){
		STNBSslContextCtx* ctx2 = NULL;
		//Get retained
		{
			STNBSslContextOpq* opq2 = (STNBSslContextOpq*)other->opaque;
			NBThreadMutex_lock(&opq2->mutex);
			if(opq2->ctx != NULL){
				NBThreadMutex_lock(&opq2->ctx->sslCtxtMutex);
				NBASSERT(opq2->ctx->retainCount > 0)
				opq2->ctx->retainCount++;
				ctx2	= opq2->ctx;
				NBThreadMutex_unlock(&opq2->ctx->sslCtxtMutex);
			}
			NBThreadMutex_unlock(&opq2->mutex);
		}
		//Set
		{
			STNBSslContextOpq* opq = (STNBSslContextOpq*)ref.opaque;
			NBObject_lock(opq);
			if(opq->ctx != NULL){
				NBASSERT(FALSE) //Create* called more than once
				NBSslContext_ctxRelease_(ctx2);
				ctx2 = NULL;
			} else {
				opq->ctx = ctx2;
				r = TRUE;
			}
			NBObject_unlock(opq);
		}
	}
	return r;
}*/

//Security level
//https://www.openssl.org/docs/man1.1.1/man3/SSL_set_security_level.html

BOOL NBSslContext_setSecurityLevel(STNBSslContextRef ref, int lvl){
    BOOL r = FALSE;
    NBASSERT(NBSslContext_isClass(ref));
    if(ref.opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
        STNBSslContextOpq* opq = (STNBSslContextOpq*)ref.opaque;
        NBObject_lock(opq);
        NBASSERT(opq->ctx != NULL)
        NBASSERT(opq->cert == NULL)
        NBASSERT(opq->pkey == NULL)
        if(opq->ctx != NULL){
            SSL_CTX_set_security_level(opq->ctx, lvl);
            r = TRUE;
        }
        NBObject_unlock(opq);
#		endif
    }
    return r;
}

//Attach certificate and key to context
BOOL NBSslContext_attachCertAndkey(STNBSslContextRef ref, const STNBX509* pCert, const STNBPKey* pPkey){
	BOOL r = FALSE;
	NBASSERT(NBSslContext_isClass(ref));
	if(ref.opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBSslContextOpq* opq = (STNBSslContextOpq*)ref.opaque;
		NBObject_lock(opq);
		NBASSERT(opq->ctx != NULL)
		NBASSERT(opq->cert == NULL)
		NBASSERT(opq->pkey == NULL)
		if(opq->ctx != NULL && opq->cert == NULL && opq->pkey == NULL){
			X509* cert = NULL;
			EVP_PKEY* pkey = NULL;
			{
				STNBString str;
				NBString_init(&str);
				if(pCert != NULL){
					if(NBX509_getAsPEMString(pCert, &str)){
						BIO* bio = BIO_new(BIO_s_mem());
						if(BIO_write(bio, str.str, str.length) == str.length){
							cert = PEM_read_bio_X509(bio, NULL, NULL, NULL);
						}
						BIO_free(bio);
					}
				}
				NBString_release(&str);
			}
			{
				STNBString str;
				NBString_init(&str);
				if(pPkey != NULL){
					if(NBPKey_getAsPEMString(pPkey, &str)){
						BIO* bio = BIO_new(BIO_s_mem());
						if(BIO_write(bio, str.str, str.length) == str.length){
							pkey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
						}
						BIO_free(bio);
					}
				}
				NBString_release(&str);
			}
			if(cert == NULL || pkey == NULL){
				PRINTF_ERROR("SSLContext, could not load key or cert from strings.\n");
			} else {
				//Load certificate and key
				if(SSL_CTX_use_certificate(opq->ctx, cert) <= 0){
					PRINTF_ERROR("SSLContext, could not link to certificate.\n");
                    NBSslContext_printLastErrStack("SSL_CTX_use_certificate");
				} else {
					if(SSL_CTX_use_PrivateKey(opq->ctx, pkey) <= 0 ){
						PRINTF_ERROR("SSLContext, could not link to key.\n");
                        NBSslContext_printLastErrStack("SSL_CTX_use_PrivateKey");
					} else {
						if (!SSL_CTX_check_private_key(opq->ctx)){
							PRINTF_ERROR("SSLContext, cert-key check failed.\n");
                            NBSslContext_printLastErrStack("SSL_CTX_check_private_key");
						} else {
							//opq->cert = cert;
							//opq->pkey = pkey;
							//cert = NULL;
							//pkey = NULL;
							r = TRUE;
						}
					}
				}
			}
			if(cert != NULL){
				X509_free(cert);
				cert = NULL;
			}
			if(pkey != NULL){
				EVP_PKEY_free(pkey);
				pkey = NULL;
			}
		}
		NBObject_unlock(opq);
#		endif
	}
	return r;
}

/*static int verify_callback(int preverify_ok, X509_STORE_CTX *ctx){
	char    buf[256];
	X509   *err_cert;
	int     err, depth;
	SSL    *ssl;
	//
	err_cert = X509_STORE_CTX_get_current_cert(ctx);
	err = X509_STORE_CTX_get_error(ctx);
	depth = X509_STORE_CTX_get_error_depth(ctx);
	
	//
	//Retrieve the pointer to the SSL of the connection currently treated
	//and the application specific data stored into the SSL object.
	//
	ssl = X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
	X509_NAME_oneline(X509_get_subject_name(err_cert), buf, 256);
	//
	if (!preverify_ok) {
		NBPRINTF_INFO("verify error:num=%d:%s:depth=%d:%s\n", err,
			   X509_verify_cert_error_string(err), depth, buf);
	} else {
		NBPRINTF_INFO("depth=%d:%s\n", depth, buf);
	}
	// At this point, err contains the last verification error. We can use
	// it for something special
	if (!preverify_ok && (err == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT)){
		X509_NAME_oneline(X509_get_issuer_name(err_cert), buf, 256);
		NBPRINTF_INFO("issuer= %s\n", buf);
	}
	//if (mydata->always_continue)
	//		return 1;
	//else
	//	return preverify_ok;
	return 1;
}*/

#ifdef NB_ENABLE_OPENSSL
int NBSslContext_setVerifyCallback_(int preverify_ok, X509_STORE_CTX *x509_ctx){
	if(preverify_ok == 0){
		//PRINTF_WARNING("NBSslContext_setVerifyCallback_, ignoring failed preverify_ok.\n");
	}
	return 1;
}
#endif

BOOL NBSslContext_setVerifyPeerCert(STNBSslContextRef ref, const BOOL certIsRequested, const BOOL certIsRequired){
	BOOL r = FALSE;
	NBASSERT(NBSslContext_isClass(ref));
	if(ref.opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBSslContextOpq* opq = (STNBSslContextOpq*)ref.opaque;
		NBObject_lock(opq);
		if(opq->ctx != NULL){
			if(certIsRequested){
				SSL_CTX_set_verify(opq->ctx, SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE | (certIsRequired ? SSL_VERIFY_FAIL_IF_NO_PEER_CERT : 0), NBSslContext_setVerifyCallback_);
			} else {
				SSL_CTX_set_verify(opq->ctx, SSL_VERIFY_NONE, NULL);
			}
			r = TRUE;
		}
		NBObject_unlock(opq);
#		endif
	}
	return r;
}

//The certificate-issuer will be added to the list of CA requested to the incoming client.
BOOL NBSslContext_addCAToRequestList(STNBSslContextRef ref, const STNBX509* cert){
	BOOL r = FALSE;
	NBASSERT(NBSslContext_isClass(ref));
	if(ref.opaque != NULL && cert != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBSslContextOpq* opq = (STNBSslContextOpq*)ref.opaque;
		NBObject_lock(opq);
		if(opq->ctx != NULL){
			STNBString strCert;
			NBString_init(&strCert);
			if(NBX509_getAsPEMString(cert, &strCert)){
				BIO* bio = BIO_new(BIO_s_mem());
				if(BIO_write(bio, strCert.str, strCert.length) == strCert.length){
					X509* x509 = PEM_read_bio_X509(bio, NULL, NULL, NULL);
					if(x509 != NULL){
						if(SSL_CTX_add_client_CA(opq->ctx, x509)){
							r = TRUE;
						}
						X509_free(x509);
						x509 = NULL;
					}
				}
				BIO_free(bio);
			}
			NBString_release(&strCert);
		}
		NBObject_unlock(opq);
#		endif
	}
	return r;
}

BOOL NBSslContext_addCAToStore(STNBSslContextRef ref, const STNBX509* cert){
	BOOL r = FALSE;
	NBASSERT(NBSslContext_isClass(ref));
	if(ref.opaque != NULL && cert != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBSslContextOpq* opq = (STNBSslContextOpq*)ref.opaque;
		NBObject_lock(opq);
		if(opq->ctx != NULL){
			STNBString strCert;
			NBString_init(&strCert);
			if(NBX509_getAsPEMString(cert, &strCert)){
				BIO* bio = BIO_new(BIO_s_mem());
				if(BIO_write(bio, strCert.str, strCert.length) == strCert.length){
					X509* x509 = PEM_read_bio_X509(bio, NULL, NULL, NULL);
					if(x509 != NULL){
						{
							X509_STORE* store = SSL_CTX_get_cert_store(opq->ctx); NBASSERT(store != NULL)
							if(store != NULL){
								if(X509_STORE_add_cert(store, x509)){
									r = TRUE;
								}
							}
						}
						X509_free(x509);
						x509 = NULL;
					}
				}
				BIO_free(bio);
			}
			NBString_release(&strCert);
		}
		NBObject_unlock(opq);
#		endif
	}
	return r;
}

//Handle

void* NBSslContext_createHandle(STNBSslContextRef ref){
	void* r = NULL;
	NBASSERT(NBSslContext_isClass(ref));
	if(ref.opaque != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBSslContextOpq* opq = (STNBSslContextOpq*)ref.opaque;
		NBObject_lock(opq);
		if(opq->ctx != NULL){
			r = SSL_new(opq->ctx);
		}
		NBObject_unlock(opq);
#		endif
	}
	return r;
}

void NBSslContext_destroyHandle(STNBSslContextRef ref, void* sslHandle){
	NBASSERT(NBSslContext_isClass(ref));
	if(ref.opaque != NULL && sslHandle != NULL){
#		ifdef NB_ENABLE_OPENSSL
		STNBSslContextOpq* opq = (STNBSslContextOpq*)ref.opaque;
		NBObject_lock(opq);
		if(opq->ctx != NULL){
			SSL_shutdown(sslHandle);
			SSL_free(sslHandle);
		}
		NBObject_unlock(opq);
#		endif
	}
}


