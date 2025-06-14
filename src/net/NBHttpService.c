
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBHttpService.h"
//
#include "nb/NBObject.h"
#include "nb/core/NBThreadCond.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBThread.h"
#include "nb/core/NBStruct.h"
#include "nb/net/NBHttpServicePort.h"

//NBHttpServiceLstnr

typedef struct STNBHttpServiceLstnr_ {
	STNBHttpServiceLstnrItf itf;
	void*					usrData;
} STNBHttpServiceLstnr;

//NBHttpService

typedef struct STNBHttpServiceOpq_ {
	STNBObject			prnt;
    STNBThreadCond      cond;
	STNBHttpServiceCfg	cfg;
    STNBStopFlagRef		stopFlag;
	//cert
	struct {
		STNBX509*		cert;
		STNBPKey*		key;
	} ca;
	//ports
	struct {
		STNBArray		arr;		//STNBHttpServicePortRef
		//active
		struct {
			SI32		portsCount;	//active ports
			SI32		connsCount;	//active conns
		} active;
	} ports;
	//pollster
	struct {
		STNBIOPollsterRef		def; //default
		STNBIOPollsterSyncRef	sync; //default
		STNBIOPollstersProviderRef provider;
	} pollster;
	//stats
	struct {
		STNBHttpStatsRef		provider;	//includes reference to parent
		STNBHttpStatsData		upd;	//pending to apply data
		//flush
		struct {
			UI32		iSeqReq;		//flush sequence requested
			UI32		iSeqDone;		//flush sequence done
		} flush;
	} stats;
	//lstnr
	STNBHttpServiceLstnr lstnr;
} STNBHttpServiceOpq;

//

NB_OBJREF_BODY(NBHttpService, STNBHttpServiceOpq, NBObject)

//ServerChannel lstnr methods

void NBHttpService_httpPortStopped_(STNBHttpServicePortRef port, void* usrData);
BOOL NBHttpService_httpPortCltConnected_(STNBHttpServicePortRef port, STNBHttpServiceConnRef conn, void* usrData);
void NBHttpService_httpPortCltDisconnected_(STNBHttpServicePortRef port, STNBHttpServiceConnRef conn, void* usrData);
BOOL NBHttpService_httpPortCltReqArrived_(STNBHttpServicePortRef port, STNBHttpServiceConnRef conn, const STNBHttpServiceReqDesc reqDesc, STNBHttpServiceReqArrivalLnk reqLnk, void* usrData);    //called when header-frist-line arrived, when header completion arrived or when body completing arrived; first to populate required methods into 'dstLtnr' take ownership and stops further calls to this method.

//

#define NB_HTTP_SERVICE_HAS_PEND_TASK_FLUSH_STATS(PTR)	((PTR)->stats.flush.iSeqDone != (PTR)->stats.flush.iSeqReq)
#define NB_HTTP_SERVICE_HAS_PEND_TASK(PTR)				NB_HTTP_SERVICE_HAS_PEND_TASK_FLUSH_STATS(PTR)

void NBHttpService_consumePendTaskStatsFlushLockedOpq_(STNBHttpServiceOpq* opq);
void NBHttpService_consumePendTasksLockedOpq_(STNBHttpServiceOpq* opq);

//

void NBHttpService_initZeroed(STNBObject* obj){
	STNBHttpServiceOpq* opq = (STNBHttpServiceOpq*)obj;
    NBThreadCond_init(&opq->cond);
    {
        opq->stopFlag = NBStopFlag_alloc(NULL);
    }
	//ports
	{
		//arr
		{
			NBArray_init(&opq->ports.arr, sizeof(STNBHttpServicePortRef), NULL);
		}
	}
	//stats
	{
		opq->stats.provider = NBHttpStats_alloc(NULL);
		//flush
		{
			//
		}
	}
}

void NBHttpService_uninitLocked(STNBObject* obj){
	STNBHttpServiceOpq* opq = (STNBHttpServiceOpq*)obj;
	//ports
	{
		//stop
        NBStopFlag_activate(opq->stopFlag);
		//arr
		{
            //apply stop flags
            {
                SI32 i; for(i = 0; i < opq->ports.arr.use; i++){
                    STNBHttpServicePortRef p = NBArray_itmValueAtIndex(&opq->ports.arr, STNBHttpServicePortRef, i);
                    NBHttpServicePort_stopFlag(p);
                }
            }
            //wait for ports
            {
                SI32 i; for(i = 0; i < opq->ports.arr.use; i++){
                    STNBHttpServicePortRef p = NBArray_itmValueAtIndex(&opq->ports.arr, STNBHttpServicePortRef, i);
                    while(NBHttpServicePort_isBusy(p)){
                        NBHttpServicePort_stopFlag(p);
                        NBThreadCond_waitObj(&opq->cond, opq); //wait for 'portsCount' change
                    }
                    NBASSERT(!NBHttpServicePort_isBusy(p));
                    NBHttpServicePort_release(&p);
                    NBHttpServicePort_null(&p);
                }
            }
            //wait for clients
            while(opq->ports.active.connsCount > 0){
                NBThreadCond_waitObj(&opq->cond, opq); //wait for 'connsCount' change
            }
            NBASSERT(opq->ports.active.connsCount == 0) //clean-exit required
            NBASSERT(opq->ports.active.portsCount == 0) //clean-exit required
            NBArray_empty(&opq->ports.arr);
            NBArray_release(&opq->ports.arr);
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
    if(NBStopFlag_isSet(opq->stopFlag)){
        NBStopFlag_release(&opq->stopFlag);
        NBStopFlag_null(&opq->stopFlag);
    }
	//cfg
	{
		NBStruct_stRelease(NBHttpServiceCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
	}
    NBThreadCond_release(&opq->cond);
}

//Cfg

BOOL NBHttpService_setCaCert(STNBHttpServiceRef ref, STNBX509* caCert){
	BOOL r = FALSE;
	STNBHttpServiceOpq* opq = (STNBHttpServiceOpq*)ref.opaque; NBASSERT(NBHttpService_isClass(ref))
	NBObject_lock(opq);
	{
		opq->ca.cert	= caCert;
		opq->ca.key		= NULL;
		r = TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBHttpService_setCaCertAndKey(STNBHttpServiceRef ref, STNBX509* caCert, STNBPKey* caKey){
	BOOL r = FALSE;
	STNBHttpServiceOpq* opq = (STNBHttpServiceOpq*)ref.opaque; NBASSERT(NBHttpService_isClass(ref))
	NBObject_lock(opq);
	{
		opq->ca.cert	= caCert;
		opq->ca.key		= caKey;
		r = TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBHttpService_setPollster(STNBHttpServiceRef ref, STNBIOPollsterRef pollster, STNBIOPollsterSyncRef pollSync){	//when one pollster only
	BOOL r = FALSE;
	STNBHttpServiceOpq* opq = (STNBHttpServiceOpq*)ref.opaque;
	NBASSERT(opq != NULL)
	NBObject_lock(opq);
	{
		//set
		NBIOPollster_set(&opq->pollster.def, &pollster);
		NBIOPollsterSync_set(&opq->pollster.sync, &pollSync);
		//
		r = TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBHttpService_setPollstersProvider(STNBHttpServiceRef ref, STNBIOPollstersProviderRef provider){ //when multiple pollsters
	BOOL r = FALSE;
	STNBHttpServiceOpq* opq = (STNBHttpServiceOpq*)ref.opaque;
	NBASSERT(opq != NULL)
	NBObject_lock(opq);
	{
		//set
		NBIOPollstersProvider_set(&opq->pollster.provider, &provider);
		//
		r = TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBHttpService_setParentStopFlag(STNBHttpServiceRef ref, STNBStopFlagRef* parentStopFlag){
    BOOL r = FALSE;
    STNBHttpServiceOpq* opq = (STNBHttpServiceOpq*)ref.opaque;
    NBASSERT(opq != NULL)
    NBObject_lock(opq);
    {
        NBStopFlag_setParentFlag(opq->stopFlag, parentStopFlag);
        r = TRUE;
    }
    NBObject_unlock(opq);
    return r;
}

//

BOOL NBHttpService_prepare(STNBHttpServiceRef ref, const STNBHttpServiceCfg* pCfg, const STNBHttpServiceLstnrItf* lstnrItf, void* lstnrUsrData){
	BOOL r = FALSE;
	STNBHttpServiceOpq* opq = (STNBHttpServiceOpq*)ref.opaque; NBASSERT(NBHttpService_isClass(ref))
	NBObject_lock(opq);
	if(pCfg != NULL){
		//ports
		if(pCfg->ports != NULL && pCfg->portsSz > 0){
			r = TRUE;
            //
            NBStopFlag_reset(opq->stopFlag);
			//lstnr
			{
				if(lstnrItf == NULL){
					NBMemory_setZeroSt(opq->lstnr, STNBHttpServiceLstnr);
				} else {
					opq->lstnr.itf = *lstnrItf;
					opq->lstnr.usrData = lstnrUsrData;
				}
			}
			//ports
			{
				UI32 i; for(i = 0; i < pCfg->portsSz && r; i++){
					STNBHttpPortCfg* cfg = &pCfg->ports[i];
					if(!cfg->isDisabled){
						STNBHttpServicePortRef port = NBHttpServicePort_alloc(NULL);
						{
							STNBHttpServicePortLstnrItf itf;
							NBMemory_setZeroSt(itf, STNBHttpServicePortLstnrItf);
                            itf.httpPortStopped             = NBHttpService_httpPortStopped_;
							itf.httpPortCltConnected        = NBHttpService_httpPortCltConnected_;
							itf.httpPortCltDisconnected     = NBHttpService_httpPortCltDisconnected_;
                            itf.httpPortCltReqArrived       = NBHttpService_httpPortCltReqArrived_;
							if(!NBHttpServicePort_loadFromConfig(port, opq->ca.cert, cfg, opq->stats.provider, &itf, opq)){
								PRINTF_ERROR("NBHttpService, could not load port(%d) #%d from loaded cfg.\n", cfg->port, (i + 1));
								r = FALSE;
                            } else if(!NBHttpServicePort_setParentStopFlag(port, &opq->stopFlag)){
                                PRINTF_ERROR("NBHttpService, could not set-parent-stop-flag port(%d) #%d from loaded cfg.\n", cfg->port, (i + 1));
                                r = FALSE;
							} else if(!NBHttpServicePort_prepare(port)){
								PRINTF_ERROR("NBHttpService, could not prepare port(%d) #%d from loaded cfg.\n", cfg->port, (i + 1));
								r = FALSE;
							} else {
								STNBIOPollsterRef pollster = NB_OBJREF_NULL;
								STNBIOPollsterSyncRef pollSync = NB_OBJREF_NULL;
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
								//result
								if(!NBIOPollster_isSet(pollster) && !NBIOPollsterSync_isSet(pollSync) && !NBIOPollstersProvider_isSet(opq->pollster.provider)){
									r = FALSE;
								} else if(!NBHttpServicePort_setPollster(port, pollster, pollSync)){
									r = FALSE;
								} else if(!NBHttpServicePort_setPollstersProvider(port, opq->pollster.provider)){
									r = FALSE;
								} else {
									NBArray_addValue(&opq->ports.arr, port);
								}
							}
						}
						if(!r){
							NBHttpServicePort_release(&port);
							NBHttpServicePort_null(&port);
						}
					}
				}
			}
			//Revert ports (if failed)
			if(!r){
                NBStopFlag_activate(opq->stopFlag);
			} else {
				//flush-upd while previous provider is set
				{
					NBHttpStats_accumData(opq->stats.provider, &opq->stats.upd);
					NBStruct_stRelease(NBHttpStatsData_getSharedStructMap(), &opq->stats.upd, sizeof(opq->stats.upd));
				}
				//clone cfg
				{
					NBStruct_stRelease(NBHttpServiceCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
					if(pCfg != NULL){
						NBStruct_stClone(NBHttpServiceCfg_getSharedStructMap(), pCfg, sizeof(*pCfg), &opq->cfg, sizeof(opq->cfg));
					}
				}
			}
		}
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBHttpService_startListening(STNBHttpServiceRef ref){
	BOOL r = FALSE;
	STNBHttpServiceOpq* opq = (STNBHttpServiceOpq*)ref.opaque; NBASSERT(NBHttpService_isClass(ref))
	NBObject_lock(opq);
	{
		r = TRUE;
		NBASSERT(opq->ports.active.portsCount == 0)
		{
			SI32 i; for(i = 0; i < opq->ports.arr.use; i++){
				STNBHttpServicePortRef p = NBArray_itmValueAtIndex(&opq->ports.arr, STNBHttpServicePortRef, i);
				opq->ports.active.portsCount++;
				if(!NBHttpServicePort_startListening(p)){
					NBASSERT(opq->ports.active.portsCount > 0)
					opq->ports.active.portsCount--;
					r = FALSE;
					break;
                }
                NBThreadCond_broadcast(&opq->cond); //notifying 'portsCount' change
			}
			//stop ports
			if(!r){
				for(i = 0; i < opq->ports.arr.use; i++){
					STNBHttpServicePortRef p = NBArray_itmValueAtIndex(&opq->ports.arr, STNBHttpServicePortRef, i);
					NBHttpServicePort_stopFlag(p);
				}
			}
		}
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBHttpService_isBusy(STNBHttpServiceRef ref){
	BOOL r = FALSE;
	STNBHttpServiceOpq* opq = (STNBHttpServiceOpq*)ref.opaque; NBASSERT(NBHttpService_isClass(ref))
	NBObject_lock(opq);
	{
		SI32 i; for(i = 0; i < opq->ports.arr.use && !r; i++){
			STNBHttpServicePortRef p = NBArray_itmValueAtIndex(&opq->ports.arr, STNBHttpServicePortRef, i);
			if(NBHttpServicePort_isBusy(p)){
				r = TRUE;
			}
		}
	}
	NBObject_unlock(opq);
	return r;
}

//


void NBHttpService_stopFlag(STNBHttpServiceRef ref){
	STNBHttpServiceOpq* opq = (STNBHttpServiceOpq*)ref.opaque; NBASSERT(NBHttpService_isClass(ref))
    NBStopFlag_activate(opq->stopFlag);
}

//ServerChannel lstnr methods

void NBHttpService_httpPortStopped_(STNBHttpServicePortRef port, void* usrData){
    STNBHttpServiceOpq* opq = (STNBHttpServiceOpq*)usrData;
    PRINTF_INFO("NBHttpService, NBHttpService_httpPortStopped_(%d): '%s'\n", NBHttpServicePort_getPort(port));
    NBObject_lock(opq);
    {
        NBASSERT(opq->ports.active.portsCount > 0)
        if(opq->ports.active.portsCount > 0){
            opq->ports.active.portsCount--;
            NBThreadCond_broadcast(&opq->cond); //notifying 'portsCount' change
        }
    }
    NBObject_unlock(opq);
}

BOOL NBHttpService_httpPortCltConnected_(STNBHttpServicePortRef port, STNBHttpServiceConnRef conn, void* usrData){
	BOOL r = TRUE;
	STNBHttpServiceOpq* opq = (STNBHttpServiceOpq*)usrData;
	PRINTF_INFO("NBHttpService, NBHttpService_httpPortCltConnected_ cert(%s)\n", (NBHttpServiceConn_isCertPresent(conn) ? "YES" : "NO"));
    if(r && NBStopFlag_isAnyActivated(opq->stopFlag)){
        PRINTF_WARNING("NBHttpService, httpPortCltConnected, service is stopping (%d active) ....\n", opq->ports.active.connsCount);
        r = FALSE;
    }
	if(r && opq->cfg.conns.conns != 0 && opq->cfg.conns.conns <= opq->ports.active.connsCount){
		PRINTF_WARNING("NBHttpService, httpPortCltConnected, maxConns reached (%d active) ....\n", opq->ports.active.connsCount);
		r = FALSE;
	}
	if(r && opq->lstnr.itf.httpCltConnected != NULL){
		r = (*opq->lstnr.itf.httpCltConnected)(NBHttpService_fromOpqPtr(opq), NBHttpServicePort_getPort(port), conn, opq->lstnr.usrData);
	}
	if(r){
		NBObject_lock(opq);
		{
			opq->ports.active.connsCount++;
            NBThreadCond_broadcast(&opq->cond); //notifying 'connsCount' change
		}
		NBObject_unlock(opq);
	}
	return r;
}

void NBHttpService_httpPortCltDisconnected_(STNBHttpServicePortRef port, STNBHttpServiceConnRef conn, void* usrData){
	STNBHttpServiceOpq* opq = (STNBHttpServiceOpq*)usrData;
	if(opq->lstnr.itf.httpCltDisconnected != NULL){
		(*opq->lstnr.itf.httpCltDisconnected)(NBHttpService_fromOpqPtr(opq), NBHttpServicePort_getPort(port), conn, opq->lstnr.usrData);
	}
	NBObject_lock(opq);
	{
		NBASSERT(opq->ports.active.connsCount > 0)
		if(opq->ports.active.connsCount > 0){
			opq->ports.active.connsCount--;
            NBThreadCond_broadcast(&opq->cond); //notifying 'connsCount' change
		}
	}
	NBObject_unlock(opq);
	//Test (force out)
	//{
	//	PRINTF_WARNING("NBHttpService, forcing stop-signal.\n");
	//	NBHttpService_stopFlagOpq(opq);
	//}
	PRINTF_INFO("NBHttpService, NBHttpService_httpPortCltDisconnected_ cert(%s)\n", (NBHttpServiceConn_isCertPresent(conn) ? "YES" : "NO"));
}

BOOL NBHttpService_httpPortCltReqArrived_(STNBHttpServicePortRef port, STNBHttpServiceConnRef conn, const STNBHttpServiceReqDesc reqDesc, STNBHttpServiceReqArrivalLnk reqLnk, void* usrData){ //called when header-frist-line arrived, when header completion arrived or when body completing arrived; first to populate required methods into 'dstLtnr' take ownership and stops further calls to this method.
    BOOL r = TRUE;
    STNBHttpServiceOpq* opq = (STNBHttpServiceOpq*)usrData;
    PRINTF_INFO("NBHttpService, NBHttpService_httpPortCltReqArrived_ cert(%s): '%s'\n", (NBHttpServiceConn_isCertPresent(conn) ? "YES" : "NO"), reqDesc.firstLine.target);
    //Lstnr
    {
        if(opq->lstnr.itf.httpCltReqArrived != NULL){
            r = (*opq->lstnr.itf.httpCltReqArrived)(NBHttpService_fromOpqPtr(opq), NBHttpServicePort_getPort(port), conn, reqDesc, reqLnk, opq->lstnr.usrData);
        }
    }
    return r;
}

//Commands

void NBHttpService_consumePendTaskStatsFlushLockedOpq_(STNBHttpServiceOpq* opq){
	if(NBHttpStats_isSet(opq->stats.provider)){
		NBHttpStats_accumData(opq->stats.provider, &opq->stats.upd);
		NBHttpStats_flush(opq->stats.provider); //push pend-data to parents
	}
	NBStruct_stRelease(NBHttpStatsData_getSharedStructMap(), &opq->stats.upd, sizeof(opq->stats.upd));
	//set as current
	opq->stats.flush.iSeqDone = opq->stats.flush.iSeqReq;
}

void NBHttpService_consumePendTasksLockedOpq_(STNBHttpServiceOpq* opq){
	if(NB_HTTP_SERVICE_HAS_PEND_TASK_FLUSH_STATS(opq)){
		NBHttpService_consumePendTaskStatsFlushLockedOpq_(opq);
	}
}

//Commands

BOOL NBHttpService_statsFlushStart(STNBHttpServiceRef ref, STNBHttpServiceCmdState* dst){
	BOOL r = FALSE;
	STNBHttpServiceOpq* opq = (STNBHttpServiceOpq*)ref.opaque; NBASSERT(NBHttpService_isClass(ref))
	NBASSERT(opq != NULL)
	NBObject_lock(opq);
	if(!NBStopFlag_isMineActivated(opq->stopFlag)){
		const UI32 iSeq = ++opq->stats.flush.iSeqReq;
		//ports
		{
			SI32 i; for(i = 0; i < opq->ports.arr.use; i++){
				STNBHttpServicePortRef c = NBArray_itmValueAtIndex(&opq->ports.arr, STNBHttpServicePortRef, i);
				if(NBHttpServicePort_statsFlushStart(c, iSeq)){
					r = TRUE;
				}
			}
		}
		//flush here
		if(!r){
			if(NB_HTTP_SERVICE_HAS_PEND_TASK_FLUSH_STATS(opq)){
				NBHttpService_consumePendTaskStatsFlushLockedOpq_(opq);
				NBASSERT(!NB_HTTP_SERVICE_HAS_PEND_TASK_FLUSH_STATS(opq))
			}
			opq->stats.flush.iSeqDone = opq->stats.flush.iSeqReq;
		}
		//set results
		if(dst != NULL){
			dst->seq		= iSeq;
			dst->isPend		= r;
		}
	} else {
		if(dst != NULL){
			dst->seq		= opq->stats.flush.iSeqReq;
			dst->isPend		= r; NBASSERT(!r)
		}
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBHttpService_statsFlushIsPend(STNBHttpServiceRef ref, STNBHttpServiceCmdState* dst){
	BOOL r = FALSE;
	STNBHttpServiceOpq* opq = (STNBHttpServiceOpq*)ref.opaque; NBASSERT(NBHttpService_isClass(ref))
	NBASSERT(opq != NULL)
	if(dst != NULL){
		NBObject_lock(opq);
		{
			//ports
			{
				SI32 i; for(i = 0; i < opq->ports.arr.use; i++){
					STNBHttpServicePortRef c = NBArray_itmValueAtIndex(&opq->ports.arr, STNBHttpServicePortRef, i);
					if(NBHttpServicePort_statsFlushIsPend(c, dst->seq)){
						r = TRUE;
					}
				}
			}
			//flush here
			if(!r){
				if(NB_HTTP_SERVICE_HAS_PEND_TASK_FLUSH_STATS(opq)){
					NBHttpService_consumePendTaskStatsFlushLockedOpq_(opq);
					NBASSERT(!NB_HTTP_SERVICE_HAS_PEND_TASK_FLUSH_STATS(opq))
				}
				opq->stats.flush.iSeqDone = opq->stats.flush.iSeqReq;
			}
			//results
			dst->isPend = r;
		}
		NBObject_unlock(opq);
	}
	return r;
}

void NBHttpService_statsGet(STNBHttpServiceRef ref, STNBHttpStatsData* dst, const BOOL resetAccum){
	STNBHttpServiceOpq* opq = (STNBHttpServiceOpq*)ref.opaque; NBASSERT(NBHttpService_isClass(ref))
	NBASSERT(opq != NULL)
	NBObject_lock(opq);
	if(NBHttpStats_isSet(opq->stats.provider)){
		NBHttpStats_getData(opq->stats.provider, dst, resetAccum);
	}
	NBObject_unlock(opq);
}

//NBHttpServiceCmdState

void NBHttpServiceCmdState_init(STNBHttpServiceCmdState* obj){
	NBMemory_setZeroSt(*obj, STNBHttpServiceCmdState);
}

void NBHttpServiceCmdState_release(STNBHttpServiceCmdState* obj){
	//nothing
}

