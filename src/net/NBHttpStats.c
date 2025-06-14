
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBHttpStats.h"
//
#include "nb/core/NBStruct.h"
#include "nb/core/NBMngrStructMaps.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBThread.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBThreadCond.h"
#include "nb/net/NBSocket.h"
#include "nb/ssl/NBSsl.h"
#include "nb/net/NBHttpHeader.h"
#include "nb/net/NBHttpBody.h"
#include "nb/net/NBHttpMessage.h"
#include "nb/net/NBHttpBuilder.h"

//Traffic

STNBStructMapsRec NBHttpServiceTraffic_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBHttpServiceTraffic_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBHttpServiceTraffic_sharedStructMap);
	if(NBHttpServiceTraffic_sharedStructMap.map == NULL){
		STNBHttpServiceTraffic s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBHttpServiceTraffic);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addUIntM(map, s, in);
		NBStructMap_addUIntM(map, s, out);
		NBHttpServiceTraffic_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBHttpServiceTraffic_sharedStructMap);
	return NBHttpServiceTraffic_sharedStructMap.map;
}

//Flow

STNBStructMapsRec NBHttpServiceFlow_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBHttpServiceFlow_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBHttpServiceFlow_sharedStructMap);
	if(NBHttpServiceFlow_sharedStructMap.map == NULL){
		STNBHttpServiceFlow s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBHttpServiceFlow);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addUIntM(map, s, connsIn);
		NBStructMap_addUIntM(map, s, connsRejects);
		NBStructMap_addUIntM(map, s, requests);
		NBStructMap_addUIntM(map, s, secsIdle);
		NBStructMap_addStructM(map, s, bytes, NBHttpServiceTraffic_getSharedStructMap());
		NBHttpServiceFlow_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBHttpServiceFlow_sharedStructMap);
	return NBHttpServiceFlow_sharedStructMap.map;
}

//Responses

STNBStructMapsRec NBHttpStatsResp_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBHttpStatsResp_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBHttpStatsResp_sharedStructMap);
	if(NBHttpStatsResp_sharedStructMap.map == NULL){
		STNBHttpStatsResp s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBHttpStatsResp);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addIntM(map, s, code); //-1 for reject/disconnection
		NBStructMap_addStrPtrM(map, s, reason);
		NBStructMap_addUIntM(map, s, count);
		NBHttpStatsResp_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBHttpStatsResp_sharedStructMap);
	return NBHttpStatsResp_sharedStructMap.map;
}

//Requests

STNBStructMapsRec NBHttpStatsReq_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBHttpStatsReq_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBHttpStatsReq_sharedStructMap);
	if(NBHttpStatsReq_sharedStructMap.map == NULL){
		STNBHttpStatsReq s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBHttpStatsReq);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addStrPtrM(map, s, target);
		NBStructMap_addUIntM(map, s, count);
		NBStructMap_addPtrToArrayOfStructM(map, s, resps, respsSz, ENNBStructMapSign_Unsigned, NBHttpStatsResp_getSharedStructMap());
		NBHttpStatsReq_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBHttpStatsReq_sharedStructMap);
	return NBHttpStatsReq_sharedStructMap.map;
}

//Stats data

STNBStructMapsRec NBHttpStatsData_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBHttpStatsData_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBHttpStatsData_sharedStructMap);
	if(NBHttpStatsData_sharedStructMap.map == NULL){
		STNBHttpStatsData s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBHttpStatsData);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addStructM(map, s, flow, NBHttpServiceFlow_getSharedStructMap());
		NBStructMap_addPtrToArrayOfStructM(map, s, reqs, reqsSz, ENNBStructMapSign_Unsigned, NBHttpStatsReq_getSharedStructMap());
		NBHttpStatsData_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBHttpStatsData_sharedStructMap);
	return NBHttpStatsData_sharedStructMap.map;
}

//Stats data parts

STNBStructMapsRec NBHttpStatsDataParts_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBHttpStatsDataParts_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBHttpStatsDataParts_sharedStructMap);
	if(NBHttpStatsDataParts_sharedStructMap.map == NULL){
		STNBHttpStatsDataParts s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBHttpStatsDataParts);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addBoolM(map, s, conns);
		NBStructMap_addBoolM(map, s, bytesIn);
		NBStructMap_addBoolM(map, s, bytesOut);
		NBStructMap_addBoolM(map, s, requests);
		NBHttpStatsDataParts_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBHttpStatsDataParts_sharedStructMap);
	return NBHttpStatsDataParts_sharedStructMap.map;
}

//NBHttpStatsData

void NBHttpStatsData_accumConnIn(STNBHttpStatsData* obj, const BOOL wasRejected){
	obj->flow.connsIn++;
	if(wasRejected){
		obj->flow.connsRejects++;
	}
}

void NBHttpStatsData_accumBytes(STNBHttpStatsData* obj, const UI64 in, const UI64 out){
	obj->flow.bytes.in	+= in;
	obj->flow.bytes.out	+= out;
}

void NBHttpStatsData_accumRequest(STNBHttpStatsData* obj, const char* target){
	STNBHttpStatsReq* req = NBHttpStatsData_getTargetData(obj, target, TRUE);
	if(req != NULL){
		obj->flow.requests++;
		req->count++;
	}
}

void NBHttpStatsData_accumResponse(STNBHttpStatsData* obj, const char* target, const SI32 code, const char* pReason){
	STNBHttpStatsReq* reqq = NBHttpStatsData_getTargetData(obj, target, TRUE);
	if(reqq != NULL){
		const char* reason = (pReason == NULL ? "" : pReason);
		BOOL fnd = FALSE;
		//Search
		if(reqq->resps != NULL && reqq->respsSz > 0){
			SI32 i; for(i = 0; i < reqq->respsSz; i++){
				STNBHttpStatsResp* resp = &reqq->resps[i];
				if(resp->code == code){
					if(NBString_strIsEqual(resp->reason, reason)){
						resp->count++;
						fnd = TRUE;
						break;
					}
				}
			}
		}
		//Add new
		if(!fnd){
			STNBHttpStatsResp* resps = NBMemory_allocTypes(STNBHttpStatsResp, reqq->respsSz + 1);
			if(reqq->resps != NULL){
				if(reqq->respsSz > 0){
					NBMemory_copy(resps, reqq->resps, sizeof(STNBHttpStatsResp) * reqq->respsSz);
				}
				NBMemory_free(reqq->resps);
				reqq->resps = NULL;
			}
			{
				STNBHttpStatsResp resp;
				NBMemory_setZeroSt(resp, STNBHttpStatsResp);
				resp.code				= code;
				resp.reason				= NBString_strNewBuffer(reason);
				resp.count				= 1;
				resps[reqq->respsSz]	= resp;
				//
				reqq->resps				= resps;
				reqq->respsSz++;
			}
		}
	}
}

void NBHttpStatsData_accumData(STNBHttpStatsData* obj, const STNBHttpStatsData* other){
	if(other != NULL){
		//flow
		{
			//conns
			{
				obj->flow.connsIn += other->flow.connsIn;
				obj->flow.connsRejects += other->flow.connsRejects;
			}
			//bytes
			{
				obj->flow.bytes.in	+= other->flow.bytes.in;
				obj->flow.bytes.out	+= other->flow.bytes.out;
			}
			//requets
			{
				obj->flow.requests += other->flow.requests;
			}
		}
		//requests
		if(other->reqs != NULL && other->reqsSz > 0){
			const STNBHttpStatsReq* reqSrc = &other->reqs[0];
			const STNBHttpStatsReq* reqAfterEnd = reqSrc + other->reqsSz;
			while(reqSrc < reqAfterEnd){
				STNBHttpStatsReq* reqq = NBHttpStatsData_getTargetData(obj, reqSrc->target, TRUE);
				if(reqq != NULL){
					reqq->count += reqSrc->count;
					//responses
					if(reqSrc->resps != NULL && reqSrc->respsSz > 0){
						const STNBHttpStatsResp* respSrc = &reqSrc->resps[0];
						const STNBHttpStatsResp* respAfterEnd = respSrc + reqSrc->respsSz;
						while(respSrc < respAfterEnd){
							BOOL fnd = FALSE;
							//Search
							if(reqq->resps != NULL && reqq->respsSz > 0){
								SI32 i; for(i = 0; i < reqq->respsSz; i++){
									STNBHttpStatsResp* resp = &reqq->resps[i];
									if(resp->code == respSrc->code && NBString_strIsEqual(resp->reason, respSrc->reason)){
										resp->count += respSrc->count;
										fnd = TRUE;
										break;
									}
								}
							}
							//Add new
							if(!fnd){
								STNBHttpStatsResp* resps = NBMemory_allocTypes(STNBHttpStatsResp, reqq->respsSz + 1);
								if(reqq->resps != NULL){
									if(reqq->respsSz > 0){
										NBMemory_copy(resps, reqq->resps, sizeof(STNBHttpStatsResp) * reqq->respsSz);
									}
									NBMemory_free(reqq->resps);
									reqq->resps = NULL;
								}
								{
									STNBHttpStatsResp resp;
									NBMemory_setZeroSt(resp, STNBHttpStatsResp);
									resp.code				= respSrc->code;
									resp.reason				= NBString_strNewBuffer(respSrc->reason);
									resp.count				= respSrc->count;
									resps[reqq->respsSz]	= resp;
									//
									reqq->resps				= resps;
									reqq->respsSz++;
								}
							}
							//next
							respSrc++;
						}
					}
				}
				//next
				reqSrc++;
			}
		}
	}
}

STNBHttpStatsReq* NBHttpStatsData_getTargetData(STNBHttpStatsData* obj, const char* pTarget, const BOOL createIfNecesary){
	STNBHttpStatsReq* r = NULL;
	const char* target = (pTarget == NULL ? "" : pTarget);
	BOOL fnd = FALSE;
	//Search
	if(obj->reqs != NULL && obj->reqsSz > 0){
		SI32 i; for(i = 0; i < obj->reqsSz; i++){
			STNBHttpStatsReq* req = &obj->reqs[i];
			if(NBString_strIsEqual(req->target, target)){
				r = req;
				fnd = TRUE;
				break;
			}
		}
	}
	//Add new
	if(!fnd && createIfNecesary){
		STNBHttpStatsReq* reqs = NBMemory_allocTypes(STNBHttpStatsReq, obj->reqsSz + 1);
		if(obj->reqs != NULL){
			if(obj->reqsSz > 0){
				NBMemory_copy(reqs, obj->reqs, sizeof(STNBHttpStatsReq) * obj->reqsSz);
			}
			NBMemory_free(obj->reqs);
			obj->reqs = NULL;
		}
		{
			STNBHttpStatsReq req;
			NBMemory_setZeroSt(req, STNBHttpStatsReq);
			req.target			= NBString_strNewBuffer(target);
			req.count			= 0;
			reqs[obj->reqsSz]	= req;
			r = &reqs[obj->reqsSz];
			//
			obj->reqs = reqs;
			obj->reqsSz++;
		}
	}
	return r;
}

//NBHttpStats

typedef struct STNBHttpStatsOpq_ {
	STNBObject			prnt;
	STNBHttpStatsData	data;		//curent data
	//cfg
	struct {
		STNBHttpStatsDataParts	ignoreParts; //parts to ignore
        //limits
        struct {
            STNBHttpLimitsCfg       conns;
        } limitss;
	} cfg;
	//prnt
	struct {
		STNBHttpStatsDataParts ignoreParts;	//parts that are not interest to any parent in the chain up
		STNBHttpStatsDataParts atomicParts;	//parts that need to be notified inmediatly due to a limit restriction.
		//pend
		struct {
			BOOL		populated;	//pend data is populated
			STNBHttpStatsData data;	//curent data
		} pend;
		STNBHttpStatsRef stats;	//parent stats
	} parent;
} STNBHttpStatsOpq;

NB_OBJREF_BODY(NBHttpStats, STNBHttpStatsOpq, NBObject)

void NBHttpStats_initZeroed(STNBObject* obj){
	//STNBHttpStatsOpq* opq = (STNBHttpStatsOpq*)obj;
}

void NBHttpStats_uninitLocked(STNBObject* obj){
	STNBHttpStatsOpq* opq = (STNBHttpStatsOpq*)obj;
	//data
	{
		NBStruct_stRelease(NBHttpStatsData_getSharedStructMap(), &opq->data, sizeof(opq->data));
	}
	//cfg
	{
		//ignoreParts
		{
			NBStruct_stRelease(NBHttpStatsDataParts_getSharedStructMap(), &opq->cfg.ignoreParts, sizeof(opq->cfg.ignoreParts));
		} 
		//limits
		{
            NBStruct_stRelease(NBHttpLimitsCfg_getSharedStructMap(), &opq->cfg.limitss.conns, sizeof(opq->cfg.limitss.conns));
		}
	}
	//parent
	{
		//ignoreParts
		{
			NBStruct_stRelease(NBHttpStatsDataParts_getSharedStructMap(), &opq->parent.ignoreParts, sizeof(opq->parent.ignoreParts));
		}
		//atomicParts
		{
			NBStruct_stRelease(NBHttpStatsDataParts_getSharedStructMap(), &opq->parent.atomicParts, sizeof(opq->parent.atomicParts));
		}
		//pend
		{
			//flush
			if(opq->parent.pend.populated){
				if(NBHttpStats_isSet(opq->parent.stats)){
					NBHttpStats_accumData(opq->parent.stats, &opq->parent.pend.data);
				}
				opq->parent.pend.populated = FALSE;
				NBStruct_stRelease(NBHttpStatsData_getSharedStructMap(), &opq->parent.pend.data, sizeof(opq->parent.pend.data));
			}
		}
		//
		if(NBHttpStats_isSet(opq->parent.stats)){
			NBHttpStats_release(&opq->parent.stats);
			NBHttpStats_null(&opq->parent.stats);
		}
	}
	
}

//

BOOL NBHttpStats_setCfg(STNBHttpStatsRef ref, const STNBHttpLimitsCfg* limitsConns, const STNBHttpStatsDataParts* ignoreParts, STNBHttpStatsRef prntStats){
	BOOL r = FALSE;
	STNBHttpStatsOpq* opq = (STNBHttpStatsOpq*)ref.opaque;
	NBObject_lock(opq);
	{
		r = TRUE;
		//parent stats
		if(r){
			if(NBHttpStats_isSet(prntStats)){
				if(!NBHttpStats_getPartsCfg(prntStats, &opq->parent.ignoreParts, &opq->parent.atomicParts)){
					r = FALSE;
				} else {
					NBHttpStats_retain(prntStats);
				}
			}
			if(r){
				if(NBHttpStats_isSet(opq->parent.stats)){
					NBHttpStats_release(&opq->parent.stats);
				}
				opq->parent.stats = prntStats;
			}
		}
		//cfg
		if(r){
			//ignore parts
			{
				NBStruct_stRelease(NBHttpStatsDataParts_getSharedStructMap(), &opq->cfg.ignoreParts, sizeof(opq->cfg.ignoreParts));
				if(ignoreParts != NULL){
					NBStruct_stClone(NBHttpStatsDataParts_getSharedStructMap(), &ignoreParts, sizeof(ignoreParts), &opq->cfg.ignoreParts, sizeof(opq->cfg.ignoreParts));
				}
			}
			//limits
			{
                NBStruct_stRelease(NBHttpLimitsCfg_getSharedStructMap(), &opq->cfg.limitss.conns, sizeof(opq->cfg.limitss.conns));
                if(limitsConns != NULL){
                    NBStruct_stClone(NBHttpLimitsCfg_getSharedStructMap(), limitsConns, sizeof(*limitsConns), &opq->cfg.limitss.conns, sizeof(opq->cfg.limitss.conns));
                }
			}
		}
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBHttpStats_setLimits(STNBHttpStatsRef ref, const STNBHttpLimitsCfg* limitsConns){
	BOOL r = FALSE;
	STNBHttpStatsOpq* opq = (STNBHttpStatsOpq*)ref.opaque;
	NBObject_lock(opq);
	{
        NBStruct_stRelease(NBHttpLimitsCfg_getSharedStructMap(), &opq->cfg.limitss.conns, sizeof(opq->cfg.limitss.conns));
        if(limitsConns != NULL){
            NBStruct_stClone(NBHttpLimitsCfg_getSharedStructMap(), limitsConns, sizeof(*limitsConns), &opq->cfg.limitss.conns, sizeof(opq->cfg.limitss.conns));
        }
		r = TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBHttpStats_setIgnoreParts(STNBHttpStatsRef ref, const STNBHttpStatsDataParts* ignoreParts){
	BOOL r = FALSE;
	STNBHttpStatsOpq* opq = (STNBHttpStatsOpq*)ref.opaque;
	NBObject_lock(opq);
	{
		NBStruct_stRelease(NBHttpStatsDataParts_getSharedStructMap(), &opq->cfg.ignoreParts, sizeof(opq->cfg.ignoreParts));
		if(ignoreParts != NULL){
			NBStruct_stClone(NBHttpStatsDataParts_getSharedStructMap(), &ignoreParts, sizeof(ignoreParts), &opq->cfg.ignoreParts, sizeof(opq->cfg.ignoreParts));
		}
		r = TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBHttpStats_setParentStats(STNBHttpStatsRef ref, STNBHttpStatsRef prntStats){
	BOOL r = FALSE;
	STNBHttpStatsOpq* opq = (STNBHttpStatsOpq*)ref.opaque;
	NBObject_lock(opq);
	{
		r = TRUE;
		if(NBHttpStats_isSet(prntStats)){
			if(!NBHttpStats_getPartsCfg(prntStats, &opq->parent.ignoreParts, &opq->parent.atomicParts)){
				r = FALSE;
			} else {
				NBHttpStats_retain(prntStats);
			}
		}
		if(r){
			if(NBHttpStats_isSet(opq->parent.stats)){
				NBHttpStats_release(&opq->parent.stats);
			}
			opq->parent.stats = prntStats;
		}
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBHttpStats_getPartsCfg(STNBHttpStatsRef ref, STNBHttpStatsDataParts* dstIgnoreParts, STNBHttpStatsDataParts* dstAtomicParts){
	BOOL r = FALSE;
	STNBHttpStatsOpq* opq = (STNBHttpStatsOpq*)ref.opaque;
	NBObject_lock(opq);
	{
		//parts that are not interest to any parent in the chain up
		if(dstIgnoreParts != NULL){
			STNBHttpStatsDataParts* dst = dstIgnoreParts;
			NBStruct_stRelease(NBHttpStatsDataParts_getSharedStructMap(), dst, sizeof(*dst));
			dst->conns		= (opq->parent.ignoreParts.conns && opq->cfg.ignoreParts.conns);
			dst->bytesIn	= (opq->parent.ignoreParts.bytesIn && opq->cfg.ignoreParts.bytesIn);
			dst->bytesOut	= (opq->parent.ignoreParts.bytesOut && opq->cfg.ignoreParts.bytesOut);
			dst->requests	= (opq->parent.ignoreParts.requests && opq->cfg.ignoreParts.requests);
		}
		//parts that need to be notified inmediatly due to a limit restriction.
		if(dstAtomicParts != NULL){
			STNBHttpStatsDataParts* dst = dstAtomicParts;
			NBStruct_stRelease(NBHttpStatsDataParts_getSharedStructMap(), dst, sizeof(*dst));
			dst->conns		= (opq->parent.atomicParts.conns || (opq->cfg.limitss.conns.conns > 0));
			dst->bytesIn	= (opq->parent.atomicParts.bytesIn || (opq->cfg.limitss.conns.bps.in > 0));
			dst->bytesOut	= (opq->parent.atomicParts.bytesOut || (opq->cfg.limitss.conns.bps.out > 0));
			dst->requests	= (opq->parent.atomicParts.requests);
		}
		//
		r = TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

//

void NBHttpStats_setData(STNBHttpStatsRef ref, const STNBHttpStatsData* src){
	STNBHttpStatsOpq* opq = (STNBHttpStatsOpq*)ref.opaque;
	NBObject_lock(opq);
	{
		if(src != NULL){
			NBStruct_stRelease(NBHttpStatsData_getSharedStructMap(), &opq->data, sizeof(opq->data));
			NBStruct_stClone(NBHttpStatsData_getSharedStructMap(), src, sizeof(*src), &opq->data, sizeof(opq->data));
		}
	}
	NBObject_unlock(opq);
}

void NBHttpStats_getData(STNBHttpStatsRef ref, STNBHttpStatsData* dst, const BOOL reset){
	STNBHttpStatsOpq* opq = (STNBHttpStatsOpq*)ref.opaque;
	NBObject_lock(opq);
	{
		if(dst != NULL){
			NBStruct_stRelease(NBHttpStatsData_getSharedStructMap(), dst, sizeof(*dst));
			NBStruct_stClone(NBHttpStatsData_getSharedStructMap(), &opq->data, sizeof(opq->data), dst, sizeof(*dst));
		}
		if(reset){
			NBStruct_stRelease(NBHttpStatsData_getSharedStructMap(), &opq->data, sizeof(opq->data));
		}
	}
	NBObject_unlock(opq);
}

void NBHttpStats_flush(STNBHttpStatsRef ref){
	STNBHttpStatsOpq* opq = (STNBHttpStatsOpq*)ref.opaque;
	NBObject_lock(opq);
	{
		//flush pend to parent
		if(opq->parent.pend.populated){
			if(NBHttpStats_isSet(opq->parent.stats)){
				NBHttpStats_accumData(opq->parent.stats, &opq->parent.pend.data);
			}
			opq->parent.pend.populated = FALSE;
			NBStruct_stRelease(NBHttpStatsData_getSharedStructMap(), &opq->parent.pend.data, sizeof(opq->parent.pend.data));
		}
	}
	NBObject_unlock(opq);
}

void NBHttpStats_empty(STNBHttpStatsRef ref){
	STNBHttpStatsOpq* opq = (STNBHttpStatsOpq*)ref.opaque;
	NBObject_lock(opq);
	{
		//flush pend to parent
		{
			if(opq->parent.pend.populated){
				if(NBHttpStats_isSet(opq->parent.stats)){
					NBHttpStats_accumData(opq->parent.stats, &opq->parent.pend.data);
				}
				opq->parent.pend.populated = FALSE;
				NBStruct_stRelease(NBHttpStatsData_getSharedStructMap(), &opq->parent.pend.data, sizeof(opq->parent.pend.data));
			}
		}
		//emoty my data
		{
			NBStruct_stRelease(NBHttpStatsData_getSharedStructMap(), &opq->data, sizeof(opq->data));
		}
	}
	NBObject_unlock(opq);
}

//

void NBHttpStats_accumConnIn(STNBHttpStatsRef ref, const BOOL wasRejected){
	STNBHttpStatsOpq* opq = (STNBHttpStatsOpq*)ref.opaque;
	NBObject_lock(opq);
	{
		//accum locally
		if(!opq->cfg.ignoreParts.conns){
			NBHttpStatsData_accumConnIn(&opq->data, wasRejected);
		}
		//notify parent
		if(NBHttpStats_isSet(opq->parent.stats)){
			if(!opq->parent.ignoreParts.conns){
				if(opq->parent.atomicParts.conns){
					//now
					NBHttpStats_accumConnIn(opq->parent.stats, wasRejected);
				} else {
					//accumulate
					NBHttpStatsData_accumConnIn(&opq->parent.pend.data, wasRejected);
					opq->parent.pend.populated = TRUE;
				}
			}
		}
	}
	NBObject_unlock(opq);
}

void NBHttpStats_accumBytes(STNBHttpStatsRef ref, const UI64 in, const UI64 out){
	STNBHttpStatsOpq* opq = (STNBHttpStatsOpq*)ref.opaque;
	NBObject_lock(opq);
	{
		//accum locally
		if((!opq->cfg.ignoreParts.bytesIn && in != 0) || (!opq->cfg.ignoreParts.bytesOut && out != 0)){
			NBHttpStatsData_accumBytes(&opq->data, in, out);
		}
		//notify parent
		if(NBHttpStats_isSet(opq->parent.stats)){
			if((!opq->parent.ignoreParts.bytesIn && in != 0) || (!opq->parent.ignoreParts.bytesOut && out != 0)){
				if(opq->parent.atomicParts.conns){
					//now
					NBHttpStats_accumBytes(opq->parent.stats, (!opq->parent.ignoreParts.bytesIn ? in : 0), (!opq->parent.ignoreParts.bytesOut ? out : 0));
				} else {
					//accumulate
					NBHttpStatsData_accumBytes(&opq->parent.pend.data, (!opq->parent.ignoreParts.bytesIn ? in : 0), (!opq->parent.ignoreParts.bytesOut ? out : 0));
					opq->parent.pend.populated = TRUE;
				}
			}
		}
	}
	NBObject_unlock(opq);
}
	
void NBHttpStats_accumRequest(STNBHttpStatsRef ref, const char* target){
	STNBHttpStatsOpq* opq = (STNBHttpStatsOpq*)ref.opaque;
	NBObject_lock(opq);
	{
		//accum locally
		if(!opq->cfg.ignoreParts.requests){
			NBHttpStatsData_accumRequest(&opq->data, target);
		}
		//notify parent
		if(NBHttpStats_isSet(opq->parent.stats)){
			if(!opq->parent.ignoreParts.requests){
				if(opq->parent.atomicParts.requests){
					//now
					NBHttpStats_accumRequest(opq->parent.stats, target);
				} else {
					//accumulate
					NBHttpStatsData_accumRequest(&opq->parent.pend.data, target);
					opq->parent.pend.populated = TRUE;
				}
			}
		}
	}
	NBObject_unlock(opq);
}

void NBHttpStats_accumResponse(STNBHttpStatsRef ref, const char* target, const SI32 code, const char* pReason){
	STNBHttpStatsOpq* opq = (STNBHttpStatsOpq*)ref.opaque;
	NBObject_lock(opq);
	{
		//accum locally
		if(!opq->cfg.ignoreParts.requests){
			NBHttpStatsData_accumResponse(&opq->data, target, code, pReason);
		}
		//notify parent
		if(NBHttpStats_isSet(opq->parent.stats)){
			if(!opq->parent.ignoreParts.requests){
				if(opq->parent.atomicParts.requests){
					//now
					NBHttpStats_accumResponse(opq->parent.stats, target, code, pReason);
				} else {
					//accumulate
					NBHttpStatsData_accumResponse(&opq->parent.pend.data, target, code, pReason);
					opq->parent.pend.populated = TRUE;
				}
			}
		}
	}
	NBObject_unlock(opq);
}

void NBHttpStats_accumData(STNBHttpStatsRef ref, STNBHttpStatsData* data){
	STNBHttpStatsOpq* opq = (STNBHttpStatsOpq*)ref.opaque;
	NBObject_lock(opq);
	{
		//accum locally
		{
			NBHttpStatsData_accumData(&opq->data, data);
		}
		//notify parent
		if(NBHttpStats_isSet(opq->parent.stats)){
			const BOOL conns = (!opq->parent.ignoreParts.conns && (data->flow.connsIn != 0 || data->flow.connsRejects != 0));
			const BOOL bytesIn = (!opq->parent.ignoreParts.bytesIn && data->flow.bytes.in != 0);
			const BOOL bytesOut = (!opq->parent.ignoreParts.bytesOut && data->flow.bytes.out != 0);
			const BOOL requests = (!opq->parent.ignoreParts.requests && data->reqs != NULL && data->reqsSz != 0); 
			if(conns || bytesIn || bytesOut || requests){
				if(
				   (conns && opq->parent.atomicParts.conns)
				   || (bytesIn && opq->parent.atomicParts.bytesIn)
				   || (bytesOut && opq->parent.atomicParts.bytesOut)
				   || (requests && opq->parent.atomicParts.requests)
				   )
				{
					//send-now to parent
					NBHttpStats_accumData(opq->parent.stats, data);
				} else {
					//accumulate to flush later
					NBHttpStatsData_accumData(&opq->parent.pend.data, data);
					opq->parent.pend.populated = TRUE;
				}
			}
		}
	}
	NBObject_unlock(opq);
}
