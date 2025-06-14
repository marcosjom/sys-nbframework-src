#include "nb/NBFrameworkPch.h"
#include "nb/ssl/NBSsl.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBThread.h"

//SSL
#ifdef NB_ENABLE_OPENSSL
#	include "openssl/rand.h"
#	include "openssl/ssl.h"
#	include "openssl/err.h"
//#   include "internal/bio.h"
#   include "openssl/bio.h"
#endif

#ifdef NB_CONFIG_INCLUDE_ASSERTS
#	define NBSSL_DBG_WAIT(NAME)	//{ UI32 ms = (1 + (rand() % 50)); NBThread_mSleep(ms); if(ms > 35){ PRINTF_INFO("NBSSL_DBG_WAIT(%d ms at '%s').\n", ms, NAME); } }
#	else
#	define NBSSL_DBG_WAIT(NAME)
#endif

#define NB_SOCKET_ACTION_BIT_ACCEPT             (0x1 << 0)
#define NB_SOCKET_ACTION_BIT_CONNECT			(0x1 << 1)
#define NB_SOCKET_ACTION_BIT_RECEIVE			(0x1 << 2)
#define NB_SOCKET_ACTION_BIT_SEND				(0x1 << 3)
#define NB_SOCKET_ACTION_BIT_GET_PEER_CERT		(0x1 << 4)

//

typedef struct STNBSslOpq_ {
	STNBObject			prnt;
	STNBSslContextRef	context;
#	ifdef NB_ENABLE_OPENSSL
	SSL*				ssl;
    BIO_METHOD*         mbio;
#   endif
    STNBIOLnk           ioLnk;
	//async actions
	struct {
		UI8				activeMask;
	} async;
} STNBSslOpq;

NB_OBJREF_BODY(NBSsl, STNBSslOpq, NBObject)

//BIO (custom)

#ifdef NB_ENABLE_OPENSSL
int NBSsl_bio_bwrite_(BIO* obj, const char* src, size_t srcSz, size_t* dstWrittenCount);
int NBSsl_bio_bwrite_old_(BIO* obj, const char* src, int srcSz);
int NBSsl_bio_bread_(BIO* obj, char* dst, size_t dstSz, size_t* dstReadCount);
int NBSsl_bio_bread_old_(BIO* obj, char* dst, int dstSz);
int NBSsl_bio_bputs_(BIO* obj, const char* src);
int NBSsl_bio_bgets_(BIO* obj, char* dst, int dstSz);
long NBSsl_bio_ctrl_(BIO* obj, int cmd, long num, void* ptr);
int NBSsl_bio_create_(BIO* obj);
int NBSsl_bio_destroy_(BIO* obj);
//long NBSsl_bio_callback_ctrl_(BIO* obj, int, BIO_info_cb *); //not implemented
#endif

void NBSsl_initZeroed(STNBObject* obj) {
    STNBSslOpq* opq = (STNBSslOpq*)obj;
    NBIOLnk_init(&opq->ioLnk);
    //
#	ifdef NB_ENABLE_OPENSSL
    {
        BIO_METHOD* m = BIO_meth_new(BIO_TYPE_SOCKET, "NBSsl_bio");
        if (m != NULL) {
            BIO_meth_set_write(m, NBSsl_bio_bwrite_old_);
            BIO_meth_set_write_ex(m, NBSsl_bio_bwrite_);
            BIO_meth_set_read(m, NBSsl_bio_bread_old_);
            BIO_meth_set_read_ex(m, NBSsl_bio_bread_);
            BIO_meth_set_puts(m, NBSsl_bio_bputs_);
            BIO_meth_set_gets(m, NBSsl_bio_bgets_);
            BIO_meth_set_ctrl(m, NBSsl_bio_ctrl_);
            BIO_meth_set_create(m, NBSsl_bio_create_);
            BIO_meth_set_destroy(m, NBSsl_bio_destroy_);
            //int BIO_meth_set_callback_ctrl(BIO_METHOD * biom,long (*callback_ctrl) (BIO*, int, BIO_info_cb*)); //not implemented, NBSsl_bio_callback_ctrl_
            opq->mbio = m;
        }
    }
#   endif
}

void NBSsl_uninitLocked(STNBObject* obj){
	STNBSslOpq* opq = (STNBSslOpq*)obj;
	//
#	ifdef NB_ENABLE_OPENSSL
	if(opq->ssl != NULL){
		NBASSERT(NBSslContext_isSet(opq->context))
		if(NBSslContext_isSet(opq->context)){
			NBSslContext_destroyHandle(opq->context, opq->ssl);
		}
		opq->ssl = NULL;
	}
#   endif
	if(NBSslContext_isSet(opq->context)){
		NBSslContext_release(&opq->context);
		NBSslContext_null(&opq->context);
	}
    NBIOLnk_release(&opq->ioLnk);
    //
#	ifdef NB_ENABLE_OPENSSL
    if (opq->mbio != NULL) {
        BIO_meth_free(opq->mbio);
        opq->mbio = NULL;
    }
#   endif
}

#ifdef NB_ENABLE_OPENSSL
/*struct bio_method_st NBSsl_bio_def_ = {
    BIO_TYPE_SOCKET, //BIO_TYPE_MEM,
    "NBSsl_bio",
    NBSsl_bio_bwrite_,
    NBSsl_bio_bwrite_old_,
    NBSsl_bio_bread_,
    NBSsl_bio_bread_old_,
    NBSsl_bio_bputs_,
    NBSsl_bio_bgets_,
    NBSsl_bio_ctrl_,
    NBSsl_bio_create_,
    NBSsl_bio_destroy_,
    NULL, //not implemented, NBSsl_bio_callback_ctrl_
};*/
#endif

#ifdef NB_ENABLE_OPENSSL
int NBSsl_bio_bwrite_(BIO* obj, const char* src, size_t srcSz, size_t* dstWrittenCount){
    int r = -1;
    STNBIOLnk* ioLnk = (STNBIOLnk*)BIO_get_data(obj);
    if(dstWrittenCount != NULL){
        *dstWrittenCount = 0;
    }
    if(ioLnk != NULL){
        //ToDo: validate srcSz to MAX_INT value
        r = NBIOLnk_write(ioLnk, src, (int)srcSz);
        if(r > 0 && dstWrittenCount != NULL){
            *dstWrittenCount = r;
        }
    }
    return r;
}
#endif

#ifdef NB_ENABLE_OPENSSL
int NBSsl_bio_bwrite_old_(BIO* obj, const char* src, int srcSz){
    int r = -1;
    STNBIOLnk* ioLnk = (STNBIOLnk*)BIO_get_data(obj);
    if(ioLnk != NULL){
        r = NBIOLnk_write(ioLnk, src, srcSz);
    }
    return r;
}
#endif

#ifdef NB_ENABLE_OPENSSL
int NBSsl_bio_bread_(BIO* obj, char* dst, size_t dstSz, size_t* dstReadCount){
    int r = -1;
    STNBIOLnk* ioLnk = (STNBIOLnk*)BIO_get_data(obj);
    if(dstReadCount != NULL){
        *dstReadCount = 0;
    }
    if(ioLnk != NULL){
        //ToDo: validate srcSz to MAX_INT value
        r = NBIOLnk_read(ioLnk, dst, (int)dstSz);
        if(r > 0 && dstReadCount != NULL){
            *dstReadCount = r;
        }
    }
    return r;
}
#endif

#ifdef NB_ENABLE_OPENSSL
int NBSsl_bio_bread_old_(BIO* obj, char* dst, int dstSz){
    int r = -1;
    STNBIOLnk* ioLnk = (STNBIOLnk*)BIO_get_data(obj);
    if(ioLnk != NULL){
        //ToDo: validate srcSz to MAX_INT value
        r = NBIOLnk_read(ioLnk, dst, (int)dstSz);
    }
    return r;
}
#endif

#ifdef NB_ENABLE_OPENSSL
int NBSsl_bio_bputs_(BIO* obj, const char* src){
    int r = -1;
    STNBIOLnk* ioLnk = (STNBIOLnk*)BIO_get_data(obj);
    if(ioLnk != NULL){
        const SI32 srcSz = (SI32)NBString_strLenBytes(src);
        if(srcSz == 0){
            r = 0; //nothing-to-do
        } else {
            r = NBIOLnk_write(ioLnk, src, srcSz);
        }
    }
    return r;
}
#endif

#ifdef NB_ENABLE_OPENSSL
int NBSsl_bio_bgets_(BIO* obj, char * dst, int dstSz){
    NBASSERT(FALSE); //ToDo: not implemented
    return -1;
}
#endif

#ifdef NB_ENABLE_OPENSSL
long NBSsl_bio_ctrl_(BIO* obj, int cmd, long num, void *ptr){
    long r = 1;
    switch (cmd) {
        /*case BIO_CTRL_RESET:        //1,  opt - rewind/zero etc
            r = 1; PRINTF_INFO("NBSsl_bio_ctrl_(BIO_CTRL_RESET), returning(%ld).\n", r); break;
        case BIO_CTRL_EOF:          //2, opt - are we at the eof
            r = 0; PRINTF_INFO("NBSsl_bio_ctrl_(BIO_CTRL_EOF), returning(%ld).\n", r); break;
        case BIO_CTRL_INFO:         //3, opt - extra tit-bits
            r = 0; PRINTF_INFO("NBSsl_bio_ctrl_(BIO_CTRL_INFO), returning(%ld).\n", r); break;*/
        case BIO_CTRL_SET:            //4, man - set the 'IO' type
            r = 1; /*PRINTF_INFO("NBSsl_bio_ctrl_(BIO_CTRL_SET), returning(%ld).\n", r);*/ break;
        case BIO_CTRL_GET:            //5, man - get the 'IO' type
            r = 0; /*PRINTF_INFO("NBSsl_bio_ctrl_(BIO_CTRL_GET), returning(%ld).\n", r);*/ break;
        /*case BIO_CTRL_PUSH:         //6, opt - internal, used to signify change
            r = 0; PRINTF_INFO("NBSsl_bio_ctrl_(BIO_CTRL_PUSH), returning(%ld).\n", r); break;
        case BIO_CTRL_POP:            //7, opt - internal, used to signify change
            r = 0; PRINTF_INFO("NBSsl_bio_ctrl_(BIO_CTRL_POP), returning(%ld).\n", r); break;*/
        case BIO_CTRL_GET_CLOSE:      //8, man - set the 'close' on free
            r = 0; /*PRINTF_INFO("NBSsl_bio_ctrl_(BIO_CTRL_GET_CLOSE), returning(%ld).\n", r);*/ break;
        case BIO_CTRL_SET_CLOSE:      //9, man - set the 'close' on free
            r = 1; /*PRINTF_INFO("NBSsl_bio_ctrl_(BIO_CTRL_SET_CLOSE), returning(%ld).\n", r);*/ break;
        /*case BIO_CTRL_PENDING:      //10, opt - is their more data buffered
            r = 0; PRINTF_INFO("NBSsl_bio_ctrl_(BIO_CTRL_PENDING), returning(%ld).\n", r); break;
        case BIO_CTRL_FLUSH:          //11, opt - 'flush' buffered output
            r = 1; PRINTF_INFO("NBSsl_bio_ctrl_(BIO_CTRL_FLUSH), returning(%ld).\n", r); break;*/
        case BIO_CTRL_DUP:            //12, man - extra stuff for 'duped' BIO
            r = 1; /*PRINTF_INFO("NBSsl_bio_ctrl_(BIO_CTRL_DUP), returning(%ld).\n", r);*/ break;
        /*case BIO_CTRL_WPENDING:      //13, opt - number of bytes still to write
            r = 0; PRINTF_INFO("NBSsl_bio_ctrl_(BIO_CTRL_WPENDING), returning(%ld).\n", r); break;
        case BIO_CTRL_SET_CALLBACK:   //14, opt - set callback function
            r = 00; PRINTF_INFO("NBSsl_bio_ctrl_(BIO_CTRL_SET_CALLBACK), returning(%ld).\n", r); break;
        case BIO_CTRL_GET_CALLBACK:   //15, opt - set callback function
            r = 00; PRINTF_INFO("NBSsl_bio_ctrl_(BIO_CTRL_GET_CALLBACK), returning(%ld).\n", r); break;
        case BIO_CTRL_PEEK:           //29, BIO_f_buffer special
            r = 00; PRINTF_INFO("NBSsl_bio_ctrl_(BIO_CTRL_PEEK), returning(%ld).\n", r); break;
        case BIO_CTRL_SET_FILENAME:   //30, BIO_s_file special
            r = 00; PRINTF_INFO("NBSsl_bio_ctrl_(BIO_CTRL_SET_FILENAME), returning(%ld).\n", r); break;*/
        /*default:
            PRINTF_INFO("NBSsl_bio_ctrl_(%d), returning(%ld).\n", cmd, r); break;*/
    }
    return r;
}
#endif

#ifdef NB_ENABLE_OPENSSL
int NBSsl_bio_create_(BIO* obj){
    BIO_set_init(obj, 1); //set initilizated-flag (required by ssl engine)
    return 1; //nothing
}
#endif

#ifdef NB_ENABLE_OPENSSL
int NBSsl_bio_destroy_(BIO* obj){
    STNBIOLnk* ioLnk = (STNBIOLnk*)BIO_get_data(obj);
    if(ioLnk != NULL){
        NBIOLnk_release(ioLnk);
        NBMemory_free(ioLnk);
        ioLnk = NULL;
    }
    return 1;
}
#endif

#ifdef NB_ENABLE_OPENSSL
//not implemented,
/*long NBSsl_bio_callback_ctrl_(BIO* obj, int, BIO_info_cb *){
    //not implemented
}*/
#endif

//ToDo: remove
//long NBSsl_sslWBioCallback_(BIO *b, int oper, const char *argp, size_t len, int argi, long argl, int ret, size_t *processed);

#ifdef NB_ENABLE_OPENSSL
void NBSsl_printError_(SSL* ssl, int ret, const char* method){
	int errcode = SSL_get_error(ssl, ret);
	if(errcode != 0){
        if(errcode == SSL_ERROR_SYSCALL || errcode == SSL_ERROR_SSL){
            //SSL_load_error_strings(); // strings not loaded yet
            NBSsl_printLastErrStack(method);
		} else {
			char buffStr[1024];
			ERR_error_string_n(errcode, buffStr, sizeof(buffStr));
			IF_PRINTF(const char* eLib = ERR_lib_error_string(errcode);)
			IF_PRINTF(const char* eFnc = ERR_func_error_string(errcode);)
			IF_PRINTF(const char* eRsn = ERR_reason_error_string(errcode);)
			PRINTF_ERROR("Ssl, '%s' failed: '%s' lib('%s') func('%s') reason('%s').\n", method, buffStr, eLib, eFnc, eRsn);
		}
	}
}
#endif

//Error
void NBSsl_concatLastErrStack(STNBString* dst){
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

void NBSsl_printLastErrStack(const char* lastMethodCalled){
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

//ToDo: remove
/*long NBSsl_sslWBioCallback_(BIO *b, int oper, const char *argp, size_t len, int argi, long argl, int ret, size_t *processed){
	long r = ret;
	//After a write operation
	if((oper & (BIO_CB_WRITE | BIO_CB_RETURN)) == (BIO_CB_WRITE | BIO_CB_RETURN)){
		void* callArg = (void*)BIO_get_callback_arg(b);
		STNBSocketRef socket = NBSocket_fromOpqPtr(callArg); 
		if(!NBSocket_isSet(socket)){
			PRINTF_ERROR("NBSsl_sslWBioCallback_, callback_arg is NULL.\n");
			r = -1;
		} else {
			unsigned char* data;
			const long dataSz = BIO_get_mem_data(b, &data);
			if(data != NULL && dataSz > 0){
				if(NBSocket_send(socket, (const char*)data, (int)dataSz) != (int)dataSz){
					//PRINTF_ERROR("NBSsl_sslWBioCallback_, could not send %d bytes.\n", (int)dataSz);
					r = -1;
                    if(processed != NULL) *processed = 0;
				} else {
					//PRINTF_INFO("NBSsl_sslWBioCallback_, %d bytes sent.\n", (int)dataSz);
                    if(processed != NULL) *processed = dataSz;
				}
				BIO_reset(b);
			}
		}
	}
	return r;
}*/

//

BOOL NBSsl_createWithSocket(STNBSslRef ref, STNBSslContextRef ctxt, STNBSocketRef socket){
    BOOL r = FALSE;
	NBASSERT(NBSsl_isClass(ref));
	if(NBSslContext_isSet(ctxt) && NBSocket_isSet(socket)){
#	    ifdef NB_ENABLE_OPENSSL
		STNBSslOpq* opq = (STNBSslOpq*)ref.opaque;
		if(opq != NULL){
            STNBIOLnk ioLnk;
            NBIOLnk_init(&ioLnk);
            if(!NBSocket_getIOLnk(socket, &ioLnk)){
                //error
            } else {
                STNBIOLnk* rLnk = NBMemory_allocType(STNBIOLnk);
                STNBIOLnk* wLnk = NBMemory_allocType(STNBIOLnk);
                NBIOLnk_init(rLnk);
                NBIOLnk_init(wLnk);
                NBIOLnk_set(rLnk, &ioLnk);
                NBIOLnk_set(wLnk, &ioLnk);
                NBObject_lock(opq);
                if(opq->ssl == NULL){ //only once
                    NBASSERT(!NBSslContext_isSet(opq->context))
                    NBASSERT(opq->ssl == NULL)
                    SSL* ssl = NBSslContext_createHandle(ctxt);
                    if(ssl != NULL){
                        BIO* rbio = BIO_new(opq->mbio);
                        BIO* wbio = BIO_new(opq->mbio);
                        BIO_set_data(rbio, rLnk); BIO_set_flags(rbio, BIO_FLAGS_READ); rLnk = NULL; //consume, flag 'BIO_FLAGS_READ' enables non-blocking mode
                        BIO_set_data(wbio, wLnk); BIO_set_flags(wbio, BIO_FLAGS_WRITE); wLnk = NULL; //consume, flag 'BIO_FLAGS_WRITE' enables non-blocking mode
                        SSL_set0_rbio(ssl, rbio); rbio = NULL; //consume
                        SSL_set0_wbio(ssl, wbio); wbio = NULL; //consume
                        {
                            NBASSERT(!NBSslContext_isSet(opq->context))
                            NBASSERT(opq->ssl == NULL)
                            //
                            opq->ssl = ssl; ssl = NULL; //consume
                            NBIOLnk_set(&opq->ioLnk, &ioLnk);
                            //
                            opq->context = ctxt;
                            NBSslContext_retain(opq->context);
                            NBASSERT(NBSslContext_isCreated(opq->context))
                            //
                            r = TRUE;
                        }
                        //Release (if not consumed)
                        if(rbio != NULL){
                            BIO_free(rbio);
                            rbio = NULL;
                        }
                        if(wbio != NULL){
                            BIO_free(wbio);
                            wbio = NULL;
                        }
                        if(ssl != NULL){
                            NBSslContext_destroyHandle(ctxt, ssl);
                            ssl = NULL;
                        }
                    }
                }
                NBObject_unlock(opq);
                //release (if not consumed)
                if(rLnk != NULL){
                    NBIOLnk_release(rLnk);
                    NBMemory_free(rLnk);
                    rLnk = NULL;
                }
                if(wLnk != NULL){
                    NBIOLnk_release(wLnk);
                    NBMemory_free(wLnk);
                    wLnk = NULL;
                }
            }
            NBIOLnk_release(&ioLnk);
		}
#       endif
	}
	return r;
}

BOOL NBSsl_createWithIOLnk(STNBSslRef ref, STNBSslContextRef ctxt, STNBIOLnk* ioLnk2){ //Incoming connection
    BOOL r = FALSE;
    NBASSERT(NBSsl_isClass(ref));
    if(NBSslContext_isSet(ctxt) && ioLnk2 != NULL && NBIOLnk_isSet(ioLnk2)){
#	    ifdef NB_ENABLE_OPENSSL
        STNBSslOpq* opq = (STNBSslOpq*)ref.opaque;
        if(opq != NULL){
            STNBIOLnk ioLnk;
            NBIOLnk_init(&ioLnk);
            NBIOLnk_set(&ioLnk, ioLnk2);
            {
                STNBIOLnk* rLnk = NBMemory_allocType(STNBIOLnk);
                STNBIOLnk* wLnk = NBMemory_allocType(STNBIOLnk);
                NBIOLnk_init(rLnk);
                NBIOLnk_init(wLnk);
                NBIOLnk_set(rLnk, &ioLnk);
                NBIOLnk_set(wLnk, &ioLnk);
                NBObject_lock(opq);
                if(opq->ssl == NULL){ //only once
                    NBASSERT(!NBSslContext_isSet(opq->context))
                    NBASSERT(opq->ssl == NULL)
                    SSL* ssl = NBSslContext_createHandle(ctxt);
                    if(ssl != NULL){
                        BIO* rbio = BIO_new(opq->mbio);
                        BIO* wbio = BIO_new(opq->mbio);
                        BIO_set_data(rbio, rLnk); BIO_set_flags(rbio, BIO_FLAGS_READ); rLnk = NULL; //consume, flag 'BIO_FLAGS_READ' enables non-blocking mode
                        BIO_set_data(wbio, wLnk); BIO_set_flags(wbio, BIO_FLAGS_WRITE); wLnk = NULL; //consume, flag 'BIO_FLAGS_WRITE' enables non-blocking mode
                        SSL_set0_rbio(ssl, rbio); rbio = NULL; //consume
                        SSL_set0_wbio(ssl, wbio); wbio = NULL; //consume
                        {
                            NBASSERT(!NBSslContext_isSet(opq->context))
                            NBASSERT(opq->ssl == NULL)
                            //
                            opq->ssl = ssl; ssl = NULL; //consume
                            NBIOLnk_set(&opq->ioLnk, &ioLnk);
                            //
                            opq->context = ctxt;
                            NBSslContext_retain(opq->context);
                            NBASSERT(NBSslContext_isCreated(opq->context))
                            //
                            r = TRUE;
                        }
                        //Release (if not consumed)
                        if(rbio != NULL){
                            BIO_free(rbio);
                            rbio = NULL;
                        }
                        if(wbio != NULL){
                            BIO_free(wbio);
                            wbio = NULL;
                        }
                        if(ssl != NULL){
                            NBSslContext_destroyHandle(ctxt, ssl);
                            ssl = NULL;
                        }
                    }
                }
                NBObject_unlock(opq);
                //release (if not consumed)
                if(rLnk != NULL){
                    NBIOLnk_release(rLnk);
                    NBMemory_free(rLnk);
                    rLnk = NULL;
                }
                if(wLnk != NULL){
                    NBIOLnk_release(wLnk);
                    NBMemory_free(wLnk);
                    wLnk = NULL;
                }
            }
            NBIOLnk_release(&ioLnk);
        }
#       endif
    }
    return r;
}

BOOL NBSsl_createWithIOLnkItf(STNBSslRef ref, STNBSslContextRef ctxt, STNBIOLnkItf* ioLnkItf, void* ioLnkUsrData){ //Incoming connection
    BOOL r = FALSE;
    NBASSERT(NBSsl_isClass(ref));
    if(NBSslContext_isSet(ctxt) && ioLnkItf != NULL){
#	    ifdef NB_ENABLE_OPENSSL
        STNBSslOpq* opq = (STNBSslOpq*)ref.opaque;
        if(opq != NULL){
            STNBIOLnk ioLnk;
            NBIOLnk_init(&ioLnk);
            NBIOLnk_setItf(&ioLnk, ioLnkItf, ioLnkUsrData);
            {
                STNBIOLnk* rLnk = NBMemory_allocType(STNBIOLnk);
                STNBIOLnk* wLnk = NBMemory_allocType(STNBIOLnk);
                NBIOLnk_init(rLnk);
                NBIOLnk_init(wLnk);
                NBIOLnk_set(rLnk, &ioLnk);
                NBIOLnk_set(wLnk, &ioLnk);
                NBObject_lock(opq);
                if(opq->ssl == NULL){ //only once
                    NBASSERT(!NBSslContext_isSet(opq->context))
                    NBASSERT(opq->ssl == NULL)
                    SSL* ssl = NBSslContext_createHandle(ctxt);
                    if(ssl != NULL){
                        BIO* rbio = BIO_new(opq->mbio);
                        BIO* wbio = BIO_new(opq->mbio);
                        BIO_set_data(rbio, rLnk); BIO_set_flags(rbio, BIO_FLAGS_READ); rLnk = NULL; //consume, flag 'BIO_FLAGS_READ' enables non-blocking mode
                        BIO_set_data(wbio, wLnk); BIO_set_flags(wbio, BIO_FLAGS_WRITE); wLnk = NULL; //consume, flag 'BIO_FLAGS_WRITE' enables non-blocking mode
                        SSL_set0_rbio(ssl, rbio); rbio = NULL; //consume
                        SSL_set0_wbio(ssl, wbio); wbio = NULL; //consume
                        {
                            NBASSERT(!NBSslContext_isSet(opq->context))
                            NBASSERT(opq->ssl == NULL)
                            //
                            opq->ssl = ssl; ssl = NULL; //consume
                            NBIOLnk_set(&opq->ioLnk, &ioLnk);
                            //
                            opq->context = ctxt;
                            NBSslContext_retain(opq->context);
                            NBASSERT(NBSslContext_isCreated(opq->context))
                            //
                            r = TRUE;
                        }
                        //Release (if not consumed)
                        if(rbio != NULL){
                            BIO_free(rbio);
                            rbio = NULL;
                        }
                        if(wbio != NULL){
                            BIO_free(wbio);
                            wbio = NULL;
                        }
                        if(ssl != NULL){
                            NBSslContext_destroyHandle(ctxt, ssl);
                            ssl = NULL;
                        }
                    }
                }
                NBObject_unlock(opq);
                //release (if not consumed)
                if(rLnk != NULL){
                    NBIOLnk_release(rLnk);
                    NBMemory_free(rLnk);
                    rLnk = NULL;
                }
                if(wLnk != NULL){
                    NBIOLnk_release(wLnk);
                    NBMemory_free(wLnk);
                    wLnk = NULL;
                }
            }
            NBIOLnk_release(&ioLnk);
        }
#       endif
    }
    return r;
}

ENNBSslResult NBSsl_accept(STNBSslRef ref){ //Incoming connection
    ENNBSslResult r = ENNBSslResult_Error; //-1
    STNBSslOpq* opq = (STNBSslOpq*)ref.opaque; NBASSERT(NBSsl_isClass(ref));
    if(opq != NULL){
#	    ifdef NB_ENABLE_OPENSSL
        NBObject_lock(opq);
        NBASSERT(!(opq->async.activeMask & (NB_SOCKET_ACTION_BIT_ACCEPT | NB_SOCKET_ACTION_BIT_SEND | NB_SOCKET_ACTION_BIT_RECEIVE))) //user logic error, multiple async taks
        if(opq->ssl == NULL || !NBSslContext_isCreated(opq->context) || !NBIOLnk_isSet(&opq->ioLnk) || (opq->async.activeMask & (NB_SOCKET_ACTION_BIT_ACCEPT | NB_SOCKET_ACTION_BIT_SEND | NB_SOCKET_ACTION_BIT_RECEIVE))){
            NBObject_unlock(opq);
        } else {
            opq->async.activeMask |= (NB_SOCKET_ACTION_BIT_ACCEPT | NB_SOCKET_ACTION_BIT_SEND | NB_SOCKET_ACTION_BIT_RECEIVE);
            NBObject_unlock(opq);
            //action (unlocked)
            {
                const int ret = SSL_accept(opq->ssl);
                //analyze
                if (ret <= 0){
                    const int err = SSL_get_error(opq->ssl, ret);
                    switch (err) {
                        case SSL_ERROR_WANT_READ:
                            r = ENNBSslResult_ErrWantRead;
                            break;
                        case SSL_ERROR_WANT_WRITE:
                            r = ENNBSslResult_ErrWantWrite;
                            break;
                        default:
                            NBSsl_printError_(opq->ssl, ret, "SSL_accept");
                            r = ENNBSslResult_Error;
                            break;
                    }
                } else {
                    r = ENNBSslResult_Success;
                }
                NBSSL_DBG_WAIT("NBSsl_accept")
            }
            //
            opq->async.activeMask &= ~(NB_SOCKET_ACTION_BIT_ACCEPT | NB_SOCKET_ACTION_BIT_SEND | NB_SOCKET_ACTION_BIT_RECEIVE);
        }
#       endif
    }
    return r;
}

ENNBSslResult NBSsl_connectHandshake(STNBSslRef ref){ //Outgoing connection
    ENNBSslResult r = ENNBSslResult_Error; //-1
    STNBSslOpq* opq = (STNBSslOpq*)ref.opaque; NBASSERT(NBSsl_isClass(ref));
    if(opq != NULL){
#	    ifdef NB_ENABLE_OPENSSL
        NBObject_lock(opq);
        NBASSERT(!(opq->async.activeMask & (NB_SOCKET_ACTION_BIT_CONNECT | NB_SOCKET_ACTION_BIT_SEND | NB_SOCKET_ACTION_BIT_RECEIVE))) //user logic error, multiple async taks
        if(opq->ssl == NULL || !NBSslContext_isCreated(opq->context) || !NBIOLnk_isSet(&opq->ioLnk) || (opq->async.activeMask & (NB_SOCKET_ACTION_BIT_CONNECT | NB_SOCKET_ACTION_BIT_SEND | NB_SOCKET_ACTION_BIT_RECEIVE))){
            NBObject_unlock(opq);
        } else {
            opq->async.activeMask |= (NB_SOCKET_ACTION_BIT_CONNECT | NB_SOCKET_ACTION_BIT_SEND | NB_SOCKET_ACTION_BIT_RECEIVE);
            NBObject_unlock(opq);
            //action (unlocked)
            {
                const int ret = SSL_accept(opq->ssl);
                //analyze
                if (ret <= 0){
                    const int err = SSL_get_error(opq->ssl, ret);
                    switch (err) {
                        case SSL_ERROR_WANT_READ:
                            r = ENNBSslResult_ErrWantRead;
                            break;
                        case SSL_ERROR_WANT_WRITE:
                            r = ENNBSslResult_ErrWantWrite;
                            break;
                        default:
                            NBSsl_printError_(opq->ssl, ret, "SSL_accept");
                            r = ENNBSslResult_Error;
                            break;
                    }
                } else {
                    r = ENNBSslResult_Success;
                }
                NBSSL_DBG_WAIT("NBSsl_accept")
            }
            //
            opq->async.activeMask &= ~(NB_SOCKET_ACTION_BIT_CONNECT | NB_SOCKET_ACTION_BIT_SEND | NB_SOCKET_ACTION_BIT_RECEIVE);
        }
#       endif
    }
    return r;
}

BOOL NBSsl_connect(STNBSslRef ref, STNBSslContextRef ctxt, STNBSocketRef socket){
	BOOL r = FALSE;
	STNBSslOpq* opq = (STNBSslOpq*)ref.opaque; NBASSERT(NBSsl_isClass(ref));
	NBASSERT(opq != NULL)
	if(opq != NULL && NBSslContext_isSet(ctxt) && NBSocket_isSet(socket)){
#	    ifdef NB_ENABLE_OPENSSL
        STNBIOLnk ioLnk;
        NBIOLnk_init(&ioLnk);
        if(!NBSocket_getIOLnk(socket, &ioLnk)){
            //error
        } else {
            NBObject_lock(opq);
            NBASSERT(!(opq->async.activeMask & (NB_SOCKET_ACTION_BIT_CONNECT | NB_SOCKET_ACTION_BIT_SEND | NB_SOCKET_ACTION_BIT_RECEIVE))) //user logic error, multiple async taks
            if(opq->ssl || (opq->async.activeMask & (NB_SOCKET_ACTION_BIT_CONNECT | NB_SOCKET_ACTION_BIT_SEND | NB_SOCKET_ACTION_BIT_RECEIVE))){
                NBObject_unlock(opq);
            } else {
                opq->async.activeMask |= (NB_SOCKET_ACTION_BIT_CONNECT | NB_SOCKET_ACTION_BIT_SEND | NB_SOCKET_ACTION_BIT_RECEIVE);
                NBObject_unlock(opq);
                if(opq->ssl == NULL){
                    NBASSERT(!NBSslContext_isSet(opq->context))
                    NBASSERT(opq->ssl == NULL)
                    SSL* ssl = NBSslContext_createHandle(ctxt);
                    if(ssl != NULL){
                        STNBIOLnk* rLnk = NBMemory_allocType(STNBIOLnk);
                        STNBIOLnk* wLnk = NBMemory_allocType(STNBIOLnk);
                        NBIOLnk_init(rLnk);
                        NBIOLnk_init(wLnk);
                        NBIOLnk_set(rLnk, &ioLnk);
                        NBIOLnk_set(wLnk, &ioLnk);
                        {
                            BIO* rbio = BIO_new(opq->mbio);
                            BIO* wbio = BIO_new(opq->mbio);
                            BIO_set_data(rbio, rLnk); BIO_set_flags(rbio, BIO_FLAGS_READ); rLnk = NULL; //consume, flag 'BIO_FLAGS_READ' enables non-blocking mode
                            BIO_set_data(wbio, wLnk); BIO_set_flags(wbio, BIO_FLAGS_WRITE); wLnk = NULL; //consume, flag 'BIO_FLAGS_WRITE' enables non-blocking mode
                            SSL_set0_rbio(ssl, rbio); rbio = NULL; //consume
                            SSL_set0_wbio(ssl, wbio); wbio = NULL; //consume
                            {
                                const int ret = SSL_connect(ssl);
                                if (ret != 1){
                                    NBSsl_printError_(ssl, ret, "SSL_connect");
                                    //NBLog_error("No se pudo negociar la conexion TCP/SSL con el destino ret(%d) sslerrorno(%d) sslErr('%s')\n", ret, errcode, errstr);
                                    r = FALSE;
                                } else {
                                    NBASSERT(!NBSslContext_isSet(opq->context))
                                    NBASSERT(opq->ssl == NULL)
                                    //
                                    opq->ssl = ssl; ssl = NULL; //consume
                                    NBIOLnk_set(&opq->ioLnk, &ioLnk);
                                    //
                                    opq->context = ctxt;
                                    NBSslContext_retain(opq->context);
                                    NBASSERT(NBSslContext_isCreated(opq->context))
                                    //
                                    NBSSL_DBG_WAIT("NBSsl_connect")
                                    r = TRUE;
                                }
                            }
                            //Release (if not consumed)
                            if(rbio != NULL){
                                BIO_free(rbio);
                                rbio = NULL;
                            }
                            if(wbio != NULL){
                                BIO_free(wbio);
                                wbio = NULL;
                            }
                            if(ssl != NULL){
                                NBSslContext_destroyHandle(ctxt, ssl);
                                ssl = NULL;
                            }
                        }
                        //release (if not consumed)
                        if(rLnk != NULL){
                            NBIOLnk_release(rLnk);
                            NBMemory_free(rLnk);
                            rLnk = NULL;
                        }
                        if(wLnk != NULL){
                            NBIOLnk_release(wLnk);
                            NBMemory_free(wLnk);
                            wLnk = NULL;
                        }
                    }
                }
                opq->async.activeMask &= ~(NB_SOCKET_ACTION_BIT_CONNECT | NB_SOCKET_ACTION_BIT_SEND | NB_SOCKET_ACTION_BIT_RECEIVE);
            }
        }
        //release ioLkn (if not consumed)
        NBIOLnk_release(&ioLnk);
#       endif
	}
	return r;
}

BOOL NBSsl_hasPendingRead(STNBSslRef ref){ //internal buffer containes posible 'read' data (note: sometimes a read can return zero data)
    BOOL r = FALSE;
    STNBSslOpq* opq = (STNBSslOpq*)ref.opaque; NBASSERT(NBSsl_isClass(ref));
    NBASSERT(opq != NULL)
    if(opq != NULL){
#	    ifdef NB_ENABLE_OPENSSL
        NBObject_lock(opq);
        NBASSERT(NBSslContext_isSet(opq->context)) //Must be already set
        NBASSERT(opq->ssl != NULL)                //Must be already set
        NBASSERT(!(opq->async.activeMask & NB_SOCKET_ACTION_BIT_RECEIVE)) //user logic error, multiple async taks
        if(!opq->ssl || (opq->async.activeMask & NB_SOCKET_ACTION_BIT_RECEIVE)){
            NBObject_unlock(opq);
        } else {
            SSL* ssl = opq->ssl;
            opq->async.activeMask |= NB_SOCKET_ACTION_BIT_RECEIVE;
            NBObject_unlock(opq);
            //Action (unlocked)
            {
                if(SSL_has_pending(ssl)){
                    r = TRUE;
                }
            }
            opq->async.activeMask &= ~NB_SOCKET_ACTION_BIT_RECEIVE;
        }
#       endif
    }
    return r;
}

SI32 NBSsl_read(STNBSslRef ref, void* buff, int buffSz){
	SI32 r = ENNBSslResult_Error; //-1
	STNBSslOpq* opq = (STNBSslOpq*)ref.opaque; NBASSERT(NBSsl_isClass(ref));
	NBASSERT(opq != NULL)
	if(opq != NULL){
#	    ifdef NB_ENABLE_OPENSSL
		NBObject_lock(opq);
		NBASSERT(NBSslContext_isSet(opq->context)) //Must be already set
		NBASSERT(opq->ssl != NULL)				//Must be already set
		NBASSERT(!(opq->async.activeMask & NB_SOCKET_ACTION_BIT_RECEIVE)) //user logic error, multiple async taks
		if(!opq->ssl || (opq->async.activeMask & NB_SOCKET_ACTION_BIT_RECEIVE)){
			NBObject_unlock(opq);
		} else {
			SSL* ssl = opq->ssl;
			opq->async.activeMask |= NB_SOCKET_ACTION_BIT_RECEIVE;
			NBObject_unlock(opq);
			//Action (unlocked)
			{
                r = SSL_read(ssl, buff, buffSz);
                if(r < 0){
                    const int err = SSL_get_error(opq->ssl, r);
                    switch (err) {
                        case SSL_ERROR_WANT_READ:
                            r = ENNBSslResult_ErrWantRead;
                            break;
                        case SSL_ERROR_WANT_WRITE:
                            r = ENNBSslResult_ErrWantWrite;
                            break;
                        default:
                            NBSsl_printError_(opq->ssl, r, "SSL_read");
                            r = ENNBSslResult_Error;
                            break;
                    }
                }
                NBSSL_DBG_WAIT("NBSsl_read")
			}
			opq->async.activeMask &= ~NB_SOCKET_ACTION_BIT_RECEIVE;
		}
#       endif
	}
	return r;
}

SI32 NBSsl_write(STNBSslRef ref, const void* data, int dataSz){
    SI32 r = ENNBSslResult_Error; //-1
	STNBSslOpq* opq = (STNBSslOpq*)ref.opaque; NBASSERT(NBSsl_isClass(ref));
	NBASSERT(opq != NULL)
	if(opq != NULL){
#	    ifdef NB_ENABLE_OPENSSL
		NBObject_lock(opq);
		NBASSERT(NBSslContext_isSet(opq->context)) //Must be already set
		NBASSERT(opq->ssl != NULL)				//Must be already set
		NBASSERT(!(opq->async.activeMask & NB_SOCKET_ACTION_BIT_SEND)) //user logic error, multiple async taks
		if(!opq->ssl || (opq->async.activeMask & NB_SOCKET_ACTION_BIT_SEND)){
			NBObject_unlock(opq);
		} else {
			SSL* ssl = opq->ssl;
			opq->async.activeMask |= NB_SOCKET_ACTION_BIT_SEND;
			NBObject_unlock(opq);
			//Action (unlocked)
			{
				r = SSL_write(ssl, data, dataSz);
                if(r < 0){
                    const int err = SSL_get_error(opq->ssl, r);
                    switch (err) {
                        case SSL_ERROR_WANT_READ:
                            r = ENNBSslResult_ErrWantRead;
                            break;
                        case SSL_ERROR_WANT_WRITE:
                            r = ENNBSslResult_ErrWantWrite;
                            break;
                        default:
                            NBSsl_printError_(opq->ssl, r, "SSL_write");
                            r = ENNBSslResult_Error;
                            break;
                    }
                }
				NBSSL_DBG_WAIT("NBSsl_write")
			}
			opq->async.activeMask &= ~NB_SOCKET_ACTION_BIT_SEND;
		}
#       endif
	}
	return r;
}

void NBSsl_flush(STNBSslRef ref){
    STNBSslOpq* opq = (STNBSslOpq*)ref.opaque; NBASSERT(NBSsl_isClass(ref));
    NBASSERT(opq != NULL)
    if(opq != NULL){
        NBObject_lock(opq);
        {
            NBIOLnk_flush(&opq->ioLnk);
        }
        NBObject_unlock(opq);
    }
}

void NBSsl_shutdown(STNBSslRef ref){
	STNBSslOpq* opq = (STNBSslOpq*)ref.opaque; NBASSERT(NBSsl_isClass(ref));
	NBASSERT(opq != NULL)
	if(opq != NULL){
		NBObject_lock(opq);
		{
            NBIOLnk_shutdown(&opq->ioLnk, NB_IO_BITS_RDWR);
		}
        NBObject_unlock(opq);
	}
}

//

BOOL NBSsl_getPeerCertificate(STNBSslRef ref, STNBX509* dst){
	BOOL r = FALSE;
	STNBSslOpq* opq = (STNBSslOpq*)ref.opaque; NBASSERT(NBSsl_isClass(ref));
	NBASSERT(opq != NULL)
	if(opq != NULL){
#	    ifdef NB_ENABLE_OPENSSL
		NBObject_lock(opq);
		NBASSERT(NBSslContext_isSet(opq->context)) //Must be already set
		NBASSERT(opq->ssl != NULL)				//Must be already set
		NBASSERT(!(opq->async.activeMask & (NB_SOCKET_ACTION_BIT_GET_PEER_CERT | NB_SOCKET_ACTION_BIT_SEND | NB_SOCKET_ACTION_BIT_RECEIVE))) //user logic error, multiple async taks
		if(!opq->ssl || (opq->async.activeMask & (NB_SOCKET_ACTION_BIT_GET_PEER_CERT | NB_SOCKET_ACTION_BIT_SEND | NB_SOCKET_ACTION_BIT_RECEIVE))){
			NBObject_unlock(opq);
		} else {
			SSL* ssl = opq->ssl;
			opq->async.activeMask |= (NB_SOCKET_ACTION_BIT_GET_PEER_CERT | NB_SOCKET_ACTION_BIT_SEND | NB_SOCKET_ACTION_BIT_RECEIVE);
			NBObject_unlock(opq);
			//Action (unlocked)
			{
#               ifdef OPENSSL_NO_DEPRECATED_3_0
                X509* cert = SSL_get1_peer_certificate(ssl);
#               else
                X509* cert = SSL_get_peer_certificate(ssl);
#               endif
				if(cert != NULL){
					const UI32 len = i2d_X509(cert, NULL);
					if(len > 0){
						STNBString str;
						NBString_init(&str);
						NBString_concatRepeatedByte(&str, '\0', len);
						{
							BYTE* tmpPtr = (BYTE*)str.str; //use of variable is mandatory
							i2d_X509(cert, &tmpPtr);
							NBASSERT(tmpPtr == (BYTE*)(str.str + str.length))
						}
						if(NBX509_createFromDERBytes(dst, (BYTE*)str.str, str.length)){
							r = TRUE;
						}
						NBString_release(&str);
					}
					X509_free(cert);
				}
				NBSSL_DBG_WAIT("NBSsl_getPeerCertificate")
			}
			opq->async.activeMask &= ~(NB_SOCKET_ACTION_BIT_GET_PEER_CERT | NB_SOCKET_ACTION_BIT_SEND | NB_SOCKET_ACTION_BIT_RECEIVE);
		}
#       endif
	}
	return r;
}


