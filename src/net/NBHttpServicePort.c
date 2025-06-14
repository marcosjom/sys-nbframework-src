
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBHttpServicePort.h"
//
#include "nb/core/NBStruct.h"
#include "nb/core/NBMngrStructMaps.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBThread.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBThreadCond.h"
#include "nb/core/NBStopFlag.h"
#include "nb/net/NBSocket.h"
#include "nb/net/NBHttpHeader.h"
#include "nb/net/NBHttpBody.h"
#include "nb/net/NBHttpMessage.h"
#include "nb/net/NBHttpBuilder.h"
#include "nb/net/NBHttpServiceConn.h"
#include "nb/ssl/NBSsl.h"
#include "nb/crypto/NBPkcs12.h"

//pollster callbacks

void NBHttpServicePort_pollConsumeMask_(STNBIOLnk ioLnk, const UI8 pollMask, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData);
void NBHttpServicePort_pollConsumeNoOp_(STNBIOLnk ioLnk, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData);
void NBHttpServicePort_pollRemoved_(STNBIOLnk ioLnk, void* usrData);

//conn callbacks

void NBHttpServicePort_httpConnStopped_(STNBHttpServiceConnRef conn, void* usrData);	//when socket-error or stopFlags is consumed
BOOL NBHttpServicePort_httpConnReqArrived_(STNBHttpServiceConnRef conn, const STNBHttpServiceReqDesc reqDesc, STNBHttpServiceReqArrivalLnk reqLnk, void* usrData);

//NBHttpServicePort

typedef struct STNBHttpServicePortOpq_ {
	STNBObject				prnt;
	STNBThreadCond			cond;		//cond (triggered on every added or removed client)
    STNBStopFlagRef			stopFlag;	//
	STNBHttpServicePortLstnr lstnr;
	//cfg
	struct {
		STNBHttpPortCfg		def;
	} cfg;
	//pollster
	struct {
		STNBIOPollsterRef	def; //default
		STNBIOPollsterSyncRef sync; //default
		STNBIOPollstersProviderRef provider; //provider
		BOOL				isListening;
	} pollster;
	//net
	struct {
		BOOL				isBinded;		//isBinded
		UI32				port;			//port listening
		STNBSocketRef		socket;			//socket listening
		//ssl
		struct {
			STNBSslContextRef context;		//Server ssl context (with certificate and keys binded)
			STNBPKey*		key;			//linked to 'context'
			STNBX509*		cert;			//linked to 'context'
			STNBX509*		caCert;			//CertificatAuthorities
		} ssl;
	} net;
	//conns
	struct {
		STNBArray			arr;			//STNBHttpServiceCltRef
	} conns;
	//stats
	struct {
		STNBHttpStatsRef    provider;	//includes reference to parent
		STNBHttpStatsData   upd;	//pending to apply data
		//flush
		struct {
			UI32		    iSeqReq;		//flush sequence requested
			UI32		    iSeqDone;		//flush sequence done
		} flush;
	} stats;
	UI32					tunnelsCount;	//How many clients are tunnels
	UI32					netBuffSize;	//Read net-buffer size
	UI32 					httpBuffSz;		//Read http-buffer size (zero if data must be ignored)
	UI32 					storgBuffSz;	//Storage body-buffer sizes (zero if must not be stored, only passed to listener)
} STNBHttpServicePortOpq;

//

NB_OBJREF_BODY(NBHttpServicePort, STNBHttpServicePortOpq, NBObject)

//

#define NB_HTTP_SERVICE_PORT_HAS_PEND_TASK_FLUSH_STATS(PTR)	((PTR)->stats.flush.iSeqDone != (PTR)->stats.flush.iSeqReq)
#define NB_HTTP_SERVICE_PORT_HAS_PEND_TASK(PTR)				NB_HTTP_SERVICE_PORT_HAS_PEND_TASK_FLUSH_STATS(PTR)

void NBHttpServicePort_consumePendTaskStatsFlushLockedOpq_(STNBHttpServicePortOpq* opq);
void NBHttpServicePort_consumePendTasksLockedOpq_(STNBHttpServicePortOpq* opq);

//

void NBHttpServicePort_initZeroed(STNBObject* obj){
	STNBHttpServicePortOpq* opq = (STNBHttpServicePortOpq*)obj;
	NBThreadCond_init(&opq->cond);
    {
        opq->stopFlag = NBStopFlag_alloc(NULL);
    }
	//stats
	/*{
		NBThreadMutex_init(&opq->stats.mutex);
		NBThreadCond_init(&opq->stats.cond);
	}*/
	//cfg
	{
		//
	}
	//net
	{
		//socket
		{
			opq->net.socket		= NBSocket_alloc(NULL); 
			//
			NBSocket_createHnd(opq->net.socket);
			NBSocket_setNoSIGPIPE(opq->net.socket, TRUE);
			NBSocket_setCorkEnabled(opq->net.socket, FALSE);
			NBSocket_setDelayEnabled(opq->net.socket, FALSE);
			//
			NBSocket_setReuseAddr(opq->net.socket, TRUE); //Recommended before bind
			NBSocket_setReusePort(opq->net.socket, TRUE); //Recommended before bind
			//
			NBSocket_setNonBlocking(opq->net.socket, TRUE);
			NBSocket_setUnsafeMode(opq->net.socket, TRUE);
		}
		//ssl
		{
			//
		}
	}
	//conns
	{
		NBArray_init(&opq->conns.arr, sizeof(STNBHttpServiceConnRef), NULL);
	}
	//stats
	{
		opq->stats.provider = NBHttpStats_alloc(NULL);
		//flush
		{
			//
		}
	}
	opq->netBuffSize	= 4096;
	opq->httpBuffSz		= 4096;
	opq->storgBuffSz	= 4096;
}

void NBHttpServicePort_uninitLocked(STNBObject* obj){
	STNBHttpServicePortOpq* opq = (STNBHttpServicePortOpq*)obj;
	//
	NBASSERT(!opq->pollster.isListening)	//clean-exit required
	NBASSERT(opq->conns.arr.use == 0)		//clean-exit required
	//Stop and wait
	{
        NBStopFlag_activate(opq->stopFlag);
        NBThreadCond_broadcast(&opq->cond);
	}
	//Release
	{
		//opq->net.port		= 0;
		NBASSERT(opq->tunnelsCount == 0)
		opq->netBuffSize	= 0;
		opq->httpBuffSz		= 0;
		opq->storgBuffSz	= 0;
		//net
		{
			if(NBSocket_isSet(opq->net.socket)){
				NBSocket_close(opq->net.socket);
			}
			//ssl
			{
				if(NBSslContext_isSet(opq->net.ssl.context)){
					NBSslContext_release(&opq->net.ssl.context);
					NBSslContext_null(&opq->net.ssl.context);
				}
				if(opq->net.ssl.key != NULL){
					NBPKey_release(opq->net.ssl.key);
					NBMemory_free(opq->net.ssl.key);
					opq->net.ssl.key = NULL;
				}
				if(opq->net.ssl.cert != NULL){
					NBX509_release(opq->net.ssl.cert);
					NBMemory_free(opq->net.ssl.cert);
					opq->net.ssl.cert = NULL;
				}
				if(opq->net.ssl.caCert != NULL){
					opq->net.ssl.caCert = NULL;
				}
			}
			if(NBSocket_isSet(opq->net.socket)){
				NBSocket_release(&opq->net.socket);
				NBSocket_null(&opq->net.socket);
			}
		}
		//pollster
		{
			if(NBIOPollster_isSet(opq->pollster.def)){
				NBIOPollster_release(&opq->pollster.def);
				NBIOPollster_null(&opq->pollster.def);
			}
			if(NBIOPollsterSync_isSet(opq->pollster.sync)){
				NBIOPollsterSync_release(&opq->pollster.sync);
				NBIOPollsterSync_null(&opq->pollster.sync);
			}
			if(NBIOPollstersProvider_isSet(opq->pollster.provider)){
				NBIOPollstersProvider_release(&opq->pollster.provider);
				NBIOPollstersProvider_null(&opq->pollster.provider);
			}
		}
		//stats
		{
			//provider
			if(NBHttpStats_isSet(opq->stats.provider)){
				//flush upd
				NBHttpStats_accumData(opq->stats.provider, &opq->stats.upd);
				//release
				NBHttpStats_release(&opq->stats.provider);
				NBHttpStats_null(&opq->stats.provider);
			}
			//upd
			{
				NBStruct_stRelease(NBHttpStatsData_getSharedStructMap(), &opq->stats.upd, sizeof(opq->stats.upd));
			}
		}
		//cfg
		{
			NBStruct_stRelease(NBHttpPortCfg_getSharedStructMap(), &opq->cfg.def, sizeof(opq->cfg.def));
		}
		NBArray_release(&opq->conns.arr);
		NBMemory_setZeroSt(opq->lstnr, STNBHttpServicePortLstnr);
        //
        if(NBStopFlag_isSet(opq->stopFlag)){
            NBStopFlag_release(&opq->stopFlag);
            NBStopFlag_null(&opq->stopFlag);
        }
		//
		NBThreadCond_release(&opq->cond);
	}
}

//

BOOL NBHttpServicePort_loadFromConfig(STNBHttpServicePortRef ref, STNBX509* caCert, const STNBHttpPortCfg* cfg, STNBHttpStatsRef parentProvider, const STNBHttpServicePortLstnrItf* lstnrItf, void* lstnrUsrData){
	BOOL r = FALSE;
	STNBHttpServicePortOpq* opq = (STNBHttpServicePortOpq*)ref.opaque; NBASSERT(NBHttpServicePort_isClass(ref))
	NBObject_lock(opq);
	{
		if(!opq->pollster.isListening && cfg != NULL){
			r = TRUE;
			//cfg
			if(r){
                if(cfg->perConn.limits.secsOverqueueing <= 0){
                    PRINTF_CONSOLE_ERROR("NBHttpServicePort, cfg 'maxs.perConn.limits.secsOverqueueing' are required.\n");
                    r = FALSE;
                }
                //ToDo: remove only after 'STNBHttpReqRulesDef' was removed.
#               ifdef NB_CONFIG_INCLUDE_ASSERTS
                if(cfg->reqsRules != NULL){
                    PRINTF_CONSOLE_ERROR("NBHttpServicePort, 'reqsRules' was moved out of 'NBHttpService*' to user-space, please update your source code and config files.\n");
                    NBASSERT(cfg->reqsRules == NULL)
                    r = FALSE;
                }
#               endif
			}
			//Configure DRL
			if(r){
				const STNBHttpSslCfg* ssl = &cfg->ssl;
				if(!ssl->isDisabled){
					if(ssl->cert.isRequested && caCert == NULL){
						PRINTF_ERROR("NBHttpServicePort, caCertificate si required when 'cert.isRequested' is activated.\n");
					} else {
						STNBSslContextRef sslCtxt = NBSslContext_alloc(NULL);
						if(!NBSslContext_create(sslCtxt, NBSslContext_getServerMode)){
							PRINTF_ERROR("NBHttpServicePort, could not create ssl context.\n");
						} else if(!NBSslContext_setVerifyPeerCert(sslCtxt, ssl->cert.isRequested, ssl->cert.isRequired)){
							PRINTF_ERROR("NBHttpServicePort, could not set 'cert.isRequested'.\n");
						} else {
							if(ssl->cert.isRequested){
								if(!NBSslContext_addCAToRequestList(sslCtxt, caCert)){
									PRINTF_ERROR("NBHttpServicePort, could not add CA's cert with addCAToRequestList.\n");
									r = FALSE;
								} else if(!NBSslContext_addCAToStore(sslCtxt, caCert)){
									PRINTF_ERROR("NBHttpServicePort, could not add CA's cert with addCAToStore.\n");
									r = FALSE;
								}
							}
							if(r){
								const STNBHttpCertSrcCfg* s = &ssl->cert.source;
								if(s->key.pass == NULL || s->key.path == NULL){
									PRINTF_ERROR("NBHttpServicePort, both 'key.pass' and 'key.path' are required.\n");
								} else {
									const char* pKey = s->key.path;
									const char* pPass = s->key.pass;
									STNBPkcs12 p12;
									NBPkcs12_init(&p12);
									if(!NBPkcs12_createFromDERFile(&p12, pKey)){
										PRINTF_ERROR("NBHttpServicePort, could not load key: '%s'.\n", pKey);
										r = FALSE;
									} else {
										STNBPKey* key = NBMemory_allocType(STNBPKey);
										STNBX509* cert = NBMemory_allocType(STNBX509);
										NBPKey_init(key);
										NBX509_init(cert);
										if(!NBPkcs12_getCertAndKey(&p12, key, cert, pPass)){
											PRINTF_ERROR("NBHttpServicePort, could not extract key and cer: '%s'.\n", pKey);
											r = FALSE;
										} else {
											if(!NBSslContext_attachCertAndkey(sslCtxt, cert, key)){
												PRINTF_ERROR("NBHttpServicePort, could not attach cert/key to sslContext.\n");
											} else {
												//Apply
												{
													if(NBSslContext_isSet(opq->net.ssl.context)){
														NBSslContext_release(&opq->net.ssl.context);
														NBSslContext_null(&opq->net.ssl.context);
													}
													opq->net.ssl.context = sslCtxt;
													NBSslContext_null(&sslCtxt);
												}
												{
													if(opq->net.ssl.key != NULL){
														NBPKey_release(opq->net.ssl.key);
														NBMemory_free(opq->net.ssl.key);
														opq->net.ssl.key = NULL;
													}
													opq->net.ssl.key = key;
													key = NULL;
												}
												{
													if(opq->net.ssl.cert != NULL){
														NBX509_release(opq->net.ssl.cert);
														NBMemory_free(opq->net.ssl.cert);
														opq->net.ssl.cert = NULL;
													}
													opq->net.ssl.cert = cert;
													cert = NULL;
												}
												PRINTF_INFO("NBHttpServicePort, sslContext createad and attached with pkey and certificate.\n");
											}
										}
										if(key != NULL){
											NBPKey_release(key);
											NBMemory_free(key);
											key = NULL;
										}
										if(cert != NULL){
											NBX509_release(cert);
											NBMemory_free(cert);
											cert = NULL;
										}
									}
									NBPkcs12_release(&p12);
								}
							}
						}
						if(NBSslContext_isSet(sslCtxt)){
							NBSslContext_release(&sslCtxt);
							NBSslContext_null(&sslCtxt);
						}
					}
					if(!NBSslContext_isSet(opq->net.ssl.context) || opq->net.ssl.key == NULL || opq->net.ssl.cert == NULL){
						PRINTF_ERROR("NBHttpServicePort, could configure channel's ssl.\n");
						r = FALSE;
					}
				}
			}
			//apply results
			if(r){
				//set parent provider
				{
					//flush-upd while previous provider is set
					{
						NBHttpStats_accumData(opq->stats.provider, &opq->stats.upd);
						NBStruct_stRelease(NBHttpStatsData_getSharedStructMap(), &opq->stats.upd, sizeof(opq->stats.upd));
					}
					NBHttpStats_setParentStats(opq->stats.provider, parentProvider);
				}
				//ssl
				{
					opq->net.ssl.caCert = caCert;
				}
				//lstnr
				{
					if(lstnrItf == NULL){
						NBMemory_setZeroSt(opq->lstnr, STNBHttpServicePortLstnr);
					} else {
						opq->lstnr.itf = *lstnrItf;
						opq->lstnr.usrData = lstnrUsrData;
					}
				}
				//Clone config
				if(cfg == NULL){
					NBStruct_stRelease(NBHttpPortCfg_getSharedStructMap(), &opq->cfg.def, sizeof(opq->cfg.def));
				} else {
					NBStruct_stClone(NBHttpPortCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg.def, sizeof(opq->cfg.def));
				}
			}
		}
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBHttpServicePort_setBuffersSzs(STNBHttpServicePortRef ref, const UI32 netReadBuffSz, const UI32 httpBodyReadSz, const UI32 httpBodyStorgChunksSz){
	BOOL r = FALSE;
	STNBHttpServicePortOpq* opq = (STNBHttpServicePortOpq*)ref.opaque; NBASSERT(NBHttpServicePort_isClass(ref))
	NBObject_lock(opq);
	if(!opq->pollster.isListening){
		opq->netBuffSize	= netReadBuffSz;
		opq->httpBuffSz		= httpBodyReadSz;
		opq->storgBuffSz	= httpBodyStorgChunksSz;
		r = TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBHttpServicePort_setListener(STNBHttpServicePortRef ref, const STNBHttpServicePortLstnr* lstnr){
	BOOL r = FALSE;
	STNBHttpServicePortOpq* opq = (STNBHttpServicePortOpq*)ref.opaque; NBASSERT(NBHttpServicePort_isClass(ref))
	NBObject_lock(opq);
	if(!opq->pollster.isListening){
		if(lstnr == NULL){
			NBMemory_setZeroSt(opq->lstnr, STNBHttpServicePortLstnr);
		} else {
			opq->lstnr = *lstnr;
		}
		r = TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBHttpServicePort_setSslContext(STNBHttpServicePortRef ref, STNBSslContextRef sslContext){
	BOOL r = FALSE;
	STNBHttpServicePortOpq* opq = (STNBHttpServicePortOpq*)ref.opaque; NBASSERT(NBHttpServicePort_isClass(ref))
	NBObject_lock(opq);
	if(!opq->pollster.isListening){
		opq->net.ssl.context = sslContext;
		r =TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBHttpServicePort_setPollster(STNBHttpServicePortRef ref, STNBIOPollsterRef pollster, STNBIOPollsterSyncRef pollSync){	//when one pollster only
	BOOL r = FALSE;
	STNBHttpServicePortOpq* opq = (STNBHttpServicePortOpq*)ref.opaque; NBASSERT(NBHttpServicePort_isClass(ref))
	NBASSERT(opq != NULL)
	NBObject_lock(opq);
	if(!opq->pollster.isListening){
		//set
		NBIOPollster_set(&opq->pollster.def, &pollster);
		NBIOPollsterSync_set(&opq->pollster.sync, &pollSync);
		//
		r = TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBHttpServicePort_setPollstersProvider(STNBHttpServicePortRef ref, STNBIOPollstersProviderRef provider){ //when multiple pollsters
	BOOL r = FALSE;
	STNBHttpServicePortOpq* opq = (STNBHttpServicePortOpq*)ref.opaque; NBASSERT(NBHttpServicePort_isClass(ref))
	NBASSERT(opq != NULL)
	NBObject_lock(opq);
	if(!opq->pollster.isListening){
		//set
		NBIOPollstersProvider_set(&opq->pollster.provider, &provider);
		//
		r = TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBHttpServicePort_setParentStopFlag(STNBHttpServicePortRef ref, STNBStopFlagRef* parentStopFlag){
    BOOL r = FALSE;
    STNBHttpServicePortOpq* opq = (STNBHttpServicePortOpq*)ref.opaque; NBASSERT(NBHttpServicePort_isClass(ref))
    NBASSERT(opq != NULL)
    NBObject_lock(opq);
    if(!opq->pollster.isListening){
        //set
        NBStopFlag_setParentFlag(opq->stopFlag, parentStopFlag);
        //
        r = TRUE;
    }
    NBObject_unlock(opq);
    return r;
}

UI32 NBHttpServicePort_getPort(STNBHttpServicePortRef ref){
	STNBHttpServicePortOpq* opq = (STNBHttpServicePortOpq*)ref.opaque; NBASSERT(NBHttpServicePort_isClass(ref))
	return opq->net.port;
}

BOOL NBHttpServicePort_prepare(STNBHttpServicePortRef ref){
	BOOL r = FALSE;
	STNBHttpServicePortOpq* opq = (STNBHttpServicePortOpq*)ref.opaque; NBASSERT(NBHttpServicePort_isClass(ref))
	NBObject_lock(opq);
	if(!opq->pollster.isListening && !NBStopFlag_isMineActivated(opq->stopFlag) && opq->cfg.def.port > 0 && !opq->net.isBinded){
		if(!NBSocket_bind(opq->net.socket, opq->cfg.def.port)){
			PRINTF_ERROR("NBHttpServicePort, NBSocket_bind failed.\n");
		} else {
			opq->net.isBinded	= TRUE;
			opq->net.port		= opq->cfg.def.port;
			r = TRUE;
		}
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBHttpServicePort_startListening(STNBHttpServicePortRef ref){
	BOOL r = FALSE;
	STNBHttpServicePortOpq* opq = (STNBHttpServicePortOpq*)ref.opaque; NBASSERT(NBHttpServicePort_isClass(ref))
	NBObject_lock(opq);
	if(!NBStopFlag_isMineActivated(opq->stopFlag) && opq->cfg.def.port > 0 && opq->net.isBinded && !opq->pollster.isListening){
		if(!NBSocket_listen(opq->net.socket)){
			PRINTF_ERROR("NBHttpServicePort, NBSocket_listen failed.\n");
		} else {
			//pollster
			{
				STNBIOPollsterRef pollster = NB_OBJREF_NULL;
				STNBIOPollsterSyncRef pollSync = NB_OBJREF_NULL;
				STNBIOPollsterLstrnItf itf;
				//itf
				{
					NBMemory_setZeroSt(itf, STNBIOPollsterLstrnItf);
					itf.pollConsumeMask = NBHttpServicePort_pollConsumeMask_;
					itf.pollConsumeNoOp = NBHttpServicePort_pollConsumeNoOp_;
					itf.pollRemoved		= NBHttpServicePort_pollRemoved_;
				}
				//get a pollster from provider
				if(NBIOPollstersProvider_isSet(opq->pollster.provider)){
					pollster	= NBIOPollstersProvider_getPollster(opq->pollster.provider);
					if(!NBIOPollster_isSet(pollster)){
						pollSync =  NBIOPollstersProvider_getPollsterSync(opq->pollster.provider);
					}
				}
				//use default pollster
				if(!NBIOPollster_isSet(pollster)){
					pollster = opq->pollster.def;
				}
				//use default pollsterSync
				if(!NBIOPollsterSync_isSet(pollSync)){
					pollSync = opq->pollster.sync;
				}
				//add to pollster (if necesary)
                if(!opq->pollster.isListening && NBIOPollster_isSet(pollster)){
                    opq->pollster.isListening = TRUE; //set flag first in case callback is called
                    if(!NBIOPollster_addSocketWithItf(pollster, opq->net.socket, ENNBIOPollsterOpBit_None, &itf, opq)){
                        opq->pollster.isListening = FALSE;
                    }
                }
				//add to pollSync (if necesary)
				if(!opq->pollster.isListening && NBIOPollsterSync_isSet(pollSync)){
					opq->pollster.isListening = TRUE; //set flag first in case callback is called
                    if(!NBIOPollsterSync_addSocketWithItf(pollSync, opq->net.socket, ENNBIOPollsterOpBit_None, &itf, opq)){
                        opq->pollster.isListening = FALSE;
                    }
				}
				//result
				if(opq->pollster.isListening){
					r = TRUE;
				}
			}
		}
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBHttpServicePort_isBusy(STNBHttpServicePortRef ref){
	STNBHttpServicePortOpq* opq = (STNBHttpServicePortOpq*)ref.opaque; NBASSERT(NBHttpServicePort_isClass(ref))
	return (opq->pollster.isListening || opq->conns.arr.use > 0);
}
	
void NBHttpServicePort_stopFlag(STNBHttpServicePortRef ref){
	STNBHttpServicePortOpq* opq = (STNBHttpServicePortOpq*)ref.opaque; NBASSERT(NBHttpServicePort_isClass(ref))
    NBStopFlag_activate(opq->stopFlag);
}

//pollster callbacks

void NBHttpServicePort_pollConsumeMask_(STNBIOLnk ioLnk, const UI8 pollMask, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData){
	STNBHttpServicePortOpq* opq = (STNBHttpServicePortOpq*)usrData; NBASSERT(NBHttpServicePort_isClass(NBObjRef_fromOpqPtr(opq)))
	IF_NBASSERT(STNBTimestampMicro timeLastAction = NBTimestampMicro_getMonotonicFast(), timeCur; SI64 usDiff;)
	//read
	if(pollMask & ENNBIOPollsterOpBit_Read){
		//receive clients
		STNBSocketRef cSckt = NBSocket_alloc(NULL); 
		while(NBSocket_accept(opq->net.socket, cSckt)) {
			STNBIOPollsterSyncRef pollSync = NB_OBJREF_NULL;
			//get a pollSync from provider
			if(NBIOPollstersProvider_isSet(opq->pollster.provider)){
				pollSync =  NBIOPollstersProvider_getPollsterSync(opq->pollster.provider);
			}
			//use default pollsterSync
			if(!NBIOPollsterSync_isSet(pollSync)){
				pollSync = opq->pollster.sync;
			}
			//
			if(!NBIOPollsterSync_isSet(pollSync)){
				PRINTF_ERROR("NBHttpServicePort, new-client could not be started (no pollSync).\n");
				NBSocket_release(&cSckt);
			} else {
				BOOL ignore = FALSE, started = FALSE, acceptedByNotif = FALSE;
				STNBHttpServiceConnRef conn = NBHttpServiceConn_alloc(NULL);
				STNBHttpServiceConnLstnrItf itf;
				NBMemory_setZeroSt(itf, STNBHttpServiceConnLstnrItf);
				itf.httpConnStopped     = NBHttpServicePort_httpConnStopped_;
                itf.httpConnReqArrived  = NBHttpServicePort_httpConnReqArrived_;
				//analyze connsMax
				if(!ignore && opq->cfg.def.conns.conns != 0 && opq->cfg.def.conns.conns <= opq->conns.arr.use){
					PRINTF_WARNING("NBHttpServicePort, maxConns reached (%d active), rejecting ....\n", opq->conns.arr.use);
					ignore = TRUE;
				}
				//notify parent (unlocked)
				if(!ignore && opq->lstnr.itf.httpPortCltConnected != NULL){
					acceptedByNotif = ((*opq->lstnr.itf.httpPortCltConnected)(NBHttpServicePort_fromOpqPtr(opq), conn, opq->lstnr.usrData));
					ignore = !acceptedByNotif;
				}
                //NBSslContext_
				if(ignore){
					PRINTF_INFO("NBHttpServicePort, new-client ignored by httpPortCltConnected-result.\n");
					NBHttpServiceConn_release(&conn);
					NBHttpServiceConn_null(&conn);
					NBSocket_release(&cSckt);
					NBSocket_null(&cSckt);
				} else if(!NBHttpServiceConn_startListeningOwningSocket(conn, &opq->cfg.def.perConn, pollSync, &opq->stopFlag, cSckt, opq->net.ssl.caCert, opq->net.ssl.context, opq->stats.provider, &itf, opq)){
					PRINTF_ERROR("NBHttpServicePort, new-client could not be started (NBHttpServiceConn_startListeningOwningSocket failed).\n");
					NBHttpServiceConn_release(&conn);
					NBHttpServiceConn_null(&conn);
					NBSocket_release(&cSckt);
					NBSocket_null(&cSckt);
				} else {
					//PRINTF_INFO("NBHttpServicePort, new-client started.\n");
					started = TRUE;
				}
				//notify parent (unlocked)
				if(acceptedByNotif && !started && opq->lstnr.itf.httpPortCltDisconnected != NULL){
					(*opq->lstnr.itf.httpPortCltDisconnected)(NBHttpServicePort_fromOpqPtr(opq), conn, opq->lstnr.usrData);
				}
				//result
				{
					NBObject_lock(opq);
					{
						//accum stats-upd
						{
							opq->stats.upd.flow.connsIn++;
							if(!started){
								opq->stats.upd.flow.connsRejects++;
							}
						}
						//add conn
						if(started){
							NBArray_addValue(&opq->conns.arr, conn);
						}
					}
					NBObject_unlock(opq);
				}
			}
			//alloc next socket
			{
				NBSocket_null(&cSckt);
				cSckt = NBSocket_alloc(NULL);
			}
		}
		NBSocket_release(&cSckt);
		NBSocket_null(&cSckt);
	}
	//error
	if(pollMask & ENNBIOPollsterOpBits_ErrOrGone){
		//ToDo: analyze
	}
	//consume pend tasks
	/*if(NB_RTP_CLIENT_PORT_HAS_PEND_TASK(p)){
		NBRtpClient_portConsumePendTasksOpq_(opq, p);
	}*/
	//return
	dstUpd->opsMasks = ENNBIOPollsterOpBit_Read;
	//consume stopFlag
	if(NBStopFlag_isAnyActivated(opq->stopFlag) && NBIOPollsterSync_isSet(dstSync)){
		NBASSERT(opq->pollster.isListening)
        NBIOPollsterSync_removeIOLnk(dstSync, &ioLnk);
	}
	//
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	{
		timeCur	= NBTimestampMicro_getMonotonicFast();
		usDiff	= NBTimestampMicro_getDiffInUs(&timeLastAction, &timeCur);
		/*if(usDiff >= 1000ULL){
			PRINTF_INFO("NBHttpServicePort, pollConsumeMask(%s%s%s) took %llu.%llu%llums.\n", (pollMask & ENNBIOPollsterOpBit_Read ? "+read" : ""), (pollMask & ENNBIOPollsterOpBit_Write ? "+write" : ""), (pollMask & ENNBIOPollsterOpBits_ErrOrGone ? "+errOrGone" : ""), (usDiff / 1000ULL), (usDiff % 1000ULL) % 100ULL, (usDiff % 100ULL) % 10ULL);
		}*/
		timeLastAction = timeCur;
	}
#	endif
}

void NBHttpServicePort_pollConsumeNoOp_(STNBIOLnk ioLnk, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData){
	NBHttpServicePort_pollConsumeMask_(ioLnk, 0, dstUpd, dstSync, usrData);
}

void NBHttpServicePort_pollRemoved_(STNBIOLnk ioLnk, void* usrData){
	STNBHttpServicePortOpq* opq = (STNBHttpServicePortOpq*)usrData; NBASSERT(NBHttpServicePort_isClass(NBObjRef_fromOpqPtr(opq)))
	opq->pollster.isListening = FALSE;
	//PRINTF_INFO("NBHttpServicePort, port(%d) safetly-removed from pollster.\n", opq->cfg.def.port);
	if(opq->lstnr.itf.httpPortStopped != NULL){
		(*opq->lstnr.itf.httpPortStopped)(NBHttpServicePort_fromOpqPtr(opq), opq->lstnr.usrData);
	}
}

//conn callbacks

void NBHttpServicePort_httpConnStopped_(STNBHttpServiceConnRef pConn, void* usrData){	//when socket-error or stopFlags is consumed
	STNBHttpServicePortOpq* opq = (STNBHttpServicePortOpq*)usrData; NBASSERT(NBHttpServicePort_isClass(NBObjRef_fromOpqPtr(opq)))
    BOOL fnd = FALSE;
	//search and remove from array
    NBObject_lock(opq);
    {
        SI32 i; for(i = (SI32)opq->conns.arr.use - 1; i >= 0; i--){
            STNBHttpServiceConnRef conn = NBArray_itmValueAtIndex(&opq->conns.arr, STNBHttpServiceConnRef, i);
            if(NBHttpServiceConn_isSame(conn, pConn)){
                NBASSERT(!NBHttpServiceConn_isBusy(conn))
                NBArray_removeItemAtIndex(&opq->conns.arr, i);
                fnd = TRUE;
                break;
            }
        } NBASSERT(fnd)
    }
    NBObject_unlock(opq);
    //notify and release (unlocked)
    if(fnd){
        //notify
        if(opq->lstnr.itf.httpPortCltDisconnected != NULL){
            (*opq->lstnr.itf.httpPortCltDisconnected)(NBHttpServicePort_fromOpqPtr(opq), pConn, opq->lstnr.usrData);
        }
        //release
        NBASSERT(!NBHttpServiceConn_isBusy(pConn))
        NBHttpServiceConn_release(&pConn);
        NBHttpServiceConn_null(&pConn);
    }
}

BOOL NBHttpServicePort_httpConnReqArrived_(STNBHttpServiceConnRef pConn, const STNBHttpServiceReqDesc reqDesc, STNBHttpServiceReqArrivalLnk reqLnk, void* usrData){
    BOOL r = FALSE;
    STNBHttpServicePortOpq* opq = (STNBHttpServicePortOpq*)usrData; NBASSERT(NBHttpServicePort_isClass(NBObjRef_fromOpqPtr(opq)))
    {
		//stats-request
		if (reqDesc.header != NULL && reqDesc.body != NULL) {
			if (reqDesc.header->requestLine != NULL) {
				STNBString absPath;
				NBString_init(&absPath);
				if (NBHttpHeader_parseRequestTarget(reqDesc.header, &absPath, NULL, NULL)) {
					NBObject_lock(opq);
					{
						NBHttpStatsData_accumRequest(&opq->stats.upd, absPath.str);
					}
					NBObject_unlock(opq);
				}
				NBString_release(&absPath);
			}
		}
		//consume
        if(opq->lstnr.itf.httpPortCltReqArrived != NULL){
            r = (*opq->lstnr.itf.httpPortCltReqArrived)(NBHttpServicePort_fromOpqPtr(opq), pConn, reqDesc, reqLnk, opq->lstnr.usrData);
        }
        //stats-response
        if(reqDesc.header != NULL && reqDesc.body != NULL){
            if(reqDesc.header->requestLine != NULL){
                STNBString absPath;
                NBString_init(&absPath);
                if(NBHttpHeader_parseRequestTarget(reqDesc.header, &absPath, NULL, NULL)){
                    NBObject_lock(opq);
                    {
                        STNBString respReason;
                        NBString_init(&respReason);
                        {
                            const UI32 respCode = NBHttpServiceReqArrivalLnk_getDefaultResponseCodeAndReason(&reqLnk, &respReason);
                            NBHttpStatsData_accumResponse(&opq->stats.upd, absPath.str, respCode, respReason.str);
                        }
                        NBString_release(&respReason);
                    }
                    NBObject_unlock(opq);
                }
                NBString_release(&absPath);
            }
        }
    }
    return r;
}

//Commands

void NBHttpServicePort_consumePendTaskStatsFlushLockedOpq_(STNBHttpServicePortOpq* opq){
	if(NBHttpStats_isSet(opq->stats.provider)){
		NBHttpStats_accumData(opq->stats.provider, &opq->stats.upd);
		NBHttpStats_flush(opq->stats.provider); //push pend-data to parents
	}
	NBStruct_stRelease(NBHttpStatsData_getSharedStructMap(), &opq->stats.upd, sizeof(opq->stats.upd));
	//set as current
	opq->stats.flush.iSeqDone = opq->stats.flush.iSeqReq; 
}

void NBHttpServicePort_consumePendTasksLockedOpq_(STNBHttpServicePortOpq* opq){
	if(NB_HTTP_SERVICE_PORT_HAS_PEND_TASK_FLUSH_STATS(opq)){
		NBHttpServicePort_consumePendTaskStatsFlushLockedOpq_(opq);
	}
}

//Commands

BOOL NBHttpServicePort_statsFlushStart(STNBHttpServicePortRef ref, const UI32 iSeq){
	BOOL r = FALSE;
	STNBHttpServicePortOpq* opq = (STNBHttpServicePortOpq*)ref.opaque; NBASSERT(NBHttpServicePort_isClass(ref))
	NBASSERT(opq != NULL)
	NBObject_lock(opq);
	if(!NBStopFlag_isMineActivated(opq->stopFlag) && opq->stats.flush.iSeqReq < iSeq){
		opq->stats.flush.iSeqReq = iSeq;
		//conns
		{
			SI32 i; for(i = (SI32)opq->conns.arr.use - 1; i >= 0; i--){
				STNBHttpServiceConnRef conn = NBArray_itmValueAtIndex(&opq->conns.arr, STNBHttpServiceConnRef, i);
				if(NBHttpServiceConn_statsFlushStart(conn, iSeq)){
					r = TRUE;
				}
			}
		}
		//flush here
		if(!r){
			if(NB_HTTP_SERVICE_PORT_HAS_PEND_TASK_FLUSH_STATS(opq)){
				NBHttpServicePort_consumePendTaskStatsFlushLockedOpq_(opq);
				NBASSERT(!NB_HTTP_SERVICE_PORT_HAS_PEND_TASK_FLUSH_STATS(opq))
			}
			opq->stats.flush.iSeqDone = opq->stats.flush.iSeqReq;
		}
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBHttpServicePort_statsFlushIsPend(STNBHttpServicePortRef ref, const UI32 iSeq){
	BOOL r = FALSE;
	STNBHttpServicePortOpq* opq = (STNBHttpServicePortOpq*)ref.opaque; NBASSERT(NBHttpServicePort_isClass(ref))
	NBASSERT(opq != NULL)
	NBObject_lock(opq);
	{
		//conns
		{
			SI32 i; for(i = (SI32)opq->conns.arr.use - 1; i >= 0; i--){
				STNBHttpServiceConnRef conn = NBArray_itmValueAtIndex(&opq->conns.arr, STNBHttpServiceConnRef, i);
				if(NBHttpServiceConn_statsFlushIsPend(conn, iSeq)){
					r = TRUE;
				}
			}
		}
		//flush here
		if(!r){
			if(NB_HTTP_SERVICE_PORT_HAS_PEND_TASK_FLUSH_STATS(opq)){
				NBHttpServicePort_consumePendTaskStatsFlushLockedOpq_(opq);
				NBASSERT(!NB_HTTP_SERVICE_PORT_HAS_PEND_TASK_FLUSH_STATS(opq))
			}
			opq->stats.flush.iSeqDone = opq->stats.flush.iSeqReq;
		}
	}
	NBObject_unlock(opq);
	return r;
}

void NBHttpServicePort_statsGet(STNBHttpServicePortRef ref, STNBHttpStatsData* dst, const BOOL resetAccum){
	STNBHttpServicePortOpq* opq = (STNBHttpServicePortOpq*)ref.opaque; NBASSERT(NBHttpServicePort_isClass(ref))
	NBASSERT(opq != NULL)
	NBObject_lock(opq);
	if(NBHttpStats_isSet(opq->stats.provider)){
		NBHttpStats_getData(opq->stats.provider, dst, resetAccum);
	}
	NBObject_unlock(opq);
}
