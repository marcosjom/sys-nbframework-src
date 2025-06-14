
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBRtspClientStats.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBStructMaps.h"
#include "nb/core/NBMngrStructMaps.h"

//Rtcp

STNBStructMapsRec NBRtcpStatsCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtcpStatsCfg_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBRtcpStatsCfg_sharedStructMap);
	if(NBRtcpStatsCfg_sharedStructMap.map == NULL){
		STNBRtcpStatsCfg s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtcpStatsCfg);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addEnumM(map, s, statsLevel, NBLogLevel_getSharedEnumMap());
		NBRtcpStatsCfg_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBRtcpStatsCfg_sharedStructMap);
	return NBRtcpStatsCfg_sharedStructMap.map;
}

//Rtsp

STNBStructMapsRec NBRtspStatsCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtspStatsCfg_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBRtspStatsCfg_sharedStructMap);
	if(NBRtspStatsCfg_sharedStructMap.map == NULL){
		STNBRtspStatsCfg s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtspStatsCfg);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addEnumM(map, s, statsLevel, NBLogLevel_getSharedEnumMap());
		NBStructMap_addStructM(map, s, rtp, NBRtpStatsCfg_getSharedStructMap());
		NBStructMap_addStructM(map, s, rtcp, NBRtcpStatsCfg_getSharedStructMap());
		NBRtspStatsCfg_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBRtspStatsCfg_sharedStructMap);
	return NBRtspStatsCfg_sharedStructMap.map;
}

// NBRtspClientStatsBuffer

STNBStructMapsRec NBRtspClientStatsBuffer_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtspClientStatsBuffer_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBRtspClientStatsBuffer_sharedStructMap);
    if(NBRtspClientStatsBuffer_sharedStructMap.map == NULL){
        STNBRtspClientStatsBuffer s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtspClientStatsBuffer);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, count);
        NBStructMap_addUIntM(map, s, bytesCount);
        NBRtspClientStatsBuffer_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBRtspClientStatsBuffer_sharedStructMap);
    return NBRtspClientStatsBuffer_sharedStructMap.map;
}

// NBRtspClientStatsBuffers

STNBStructMapsRec NBRtspClientStatsBuffers_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtspClientStatsBuffers_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBRtspClientStatsBuffers_sharedStructMap);
    if(NBRtspClientStatsBuffers_sharedStructMap.map == NULL){
        STNBRtspClientStatsBuffers s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtspClientStatsBuffers);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, count);
        NBStructMap_addUIntM(map, s, bytesCount);
        NBStructMap_addStructM(map, s, wired, NBRtspClientStatsBuffer_getSharedStructMap());
        NBRtspClientStatsBuffers_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBRtspClientStatsBuffers_sharedStructMap);
    return NBRtspClientStatsBuffers_sharedStructMap.map;
}

// NBRtspClientStatsState

STNBStructMapsRec NBRtspClientStatsState_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtspClientStatsState_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBRtspClientStatsState_sharedStructMap);
    if(NBRtspClientStatsState_sharedStructMap.map == NULL){
        STNBRtspClientStatsState s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtspClientStatsState);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addStructM(map, s, buffs, NBRtspClientStatsBuffers_getSharedStructMap());
        NBStructMap_addStructM(map, s, rtp, NBRtpClientStatsState_getSharedStructMap());
        NBStructMap_addUIntM(map, s, updCalls);
        NBRtspClientStatsState_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBRtspClientStatsState_sharedStructMap);
    return NBRtspClientStatsState_sharedStructMap.map;
}

// NBRtspClientStatsData

STNBStructMapsRec NBRtspClientStatsData_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtspClientStatsData_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBRtspClientStatsData_sharedStructMap);
    if(NBRtspClientStatsData_sharedStructMap.map == NULL){
        STNBRtspClientStatsData s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtspClientStatsData);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addStructM(map, s, loaded, NBRtspClientStatsState_getSharedStructMap());
        NBStructMap_addStructM(map, s, accum, NBRtspClientStatsState_getSharedStructMap());
        NBStructMap_addStructM(map, s, total, NBRtspClientStatsState_getSharedStructMap());
        NBRtspClientStatsData_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBRtspClientStatsData_sharedStructMap);
    return NBRtspClientStatsData_sharedStructMap.map;
}

//NBRtspClientStatsOpq

typedef struct STNBRtspClientStatsOpq_ {
	STNBRtspClientStatsData	data;
	STNBRtpClientStats		rtp;	//substats
	STNBThreadMutex			mutex;
	//
	SI32					retainCount;
} STNBRtspClientStatsOpq;

//Factory

void NBRtspClientStats_init(STNBRtspClientStats* obj){
	STNBRtspClientStatsOpq* opq = obj->opaque = NBMemory_allocType(STNBRtspClientStatsOpq);
	NBMemory_setZeroSt(*opq, STNBRtspClientStatsOpq);
	NBRtpClientStats_init(&opq->rtp);
	NBThreadMutex_init(&opq->mutex);
	//
	opq->retainCount	= 1;
}

void NBRtspClientStats_retain_(STNBRtspClientStatsOpq* opq){
	NBASSERT(opq != NULL)
	NBThreadMutex_lock(&opq->mutex);
	{
		NBASSERT(opq->retainCount > 0)
		opq->retainCount++;
	}
	NBThreadMutex_unlock(&opq->mutex);
}


void NBRtspClientStats_retain(STNBRtspClientStats* obj){
	NBRtspClientStats_retain_((STNBRtspClientStatsOpq*)obj->opaque);
}

void NBRtspClientStats_release_(STNBRtspClientStatsOpq* opq){
	NBASSERT(opq != NULL)
	NBThreadMutex_lock(&opq->mutex);
    NBASSERT(opq->retainCount > 0)
	opq->retainCount--;
	if(opq->retainCount > 0){
		NBThreadMutex_unlock(&opq->mutex);
	} else {
		NBRtpClientStats_release(&opq->rtp);
		NBThreadMutex_unlock(&opq->mutex);
		NBThreadMutex_release(&opq->mutex);
		NBMemory_free(opq);
		opq = NULL;
	}
}

void NBRtspClientStats_release(STNBRtspClientStats* obj){
	NBRtspClientStats_release_((STNBRtspClientStatsOpq*)obj->opaque);
}

//

STNBRtspClientStatsData NBRtspClientStats_getDataLockedOpq_(STNBRtspClientStatsOpq* opq, const BOOL resetAccum){
	STNBRtspClientStatsData r = opq->data;
	//reset
	if(resetAccum){
		NBMemory_setZero(opq->data.accum);
	}
	//apply rtp
	{
		const STNBRtpClientStatsData subRtp = NBRtpClientStats_getData(&opq->rtp, resetAccum);
		r.loaded.rtp	= subRtp.loaded;
		r.accum.rtp		= subRtp.accum;
		r.total.rtp		= subRtp.total;
	}
	return r;
}
	
STNBRtspClientStatsData NBRtspClientStats_getData(STNBRtspClientStats* obj, const BOOL resetAccum){
	STNBRtspClientStatsData r;
	STNBRtspClientStatsOpq* opq  = (STNBRtspClientStatsOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	{
		r = NBRtspClientStats_getDataLockedOpq_(opq, resetAccum);
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

STNBRtpClientStats* NBRtspClientStats_getRtpStats(STNBRtspClientStats* obj){
	STNBRtspClientStatsOpq* opq  = (STNBRtspClientStatsOpq*)obj->opaque;
	return &opq->rtp;
}

STNBRtpClientStatsData NBRtspClientStats_getRtpData(STNBRtspClientStats* obj, const BOOL resetAccum){
	STNBRtspClientStatsOpq* opq  = (STNBRtspClientStatsOpq*)obj->opaque;
	return NBRtpClientStats_getData(&opq->rtp, resetAccum);
}

//

void NBRtspClientStats_concatFormatedBytes_(const UI64 bytesCount, STNBString* dst){
	if(dst != NULL){
		if(bytesCount >= (1024 * 1024 * 1024)){
			const UI64 div = (1024 * 1024 * 1024);
			const double p0 = (double)(bytesCount / div); 
			const double p1 = (double)(bytesCount % div) / (double)div;
			NBString_concatDouble(dst, p0 + p1, 1);
			NBString_concat(dst, "GB");
		} else if(bytesCount >= (1024 * 1024)){
			const UI64 div = (1024 * 1024);
			const double p0 = (double)(bytesCount / div); 
			const double p1 = (double)(bytesCount % div) / (double)div;
			NBString_concatDouble(dst, p0 + p1, 1);
			NBString_concat(dst, "MB");
		} else if(bytesCount >= (1024)){
			const UI64 div = (1024);
			const double p0 = (double)(bytesCount / div); 
			const double p1 = (double)(bytesCount % div) / (double)div;
			NBString_concatDouble(dst, p0 + p1, 1);
			NBString_concat(dst, "KB");
		} else {
			NBString_concatUI64(dst, bytesCount);
			NBString_concat(dst, "B");
		}
	}
}

void NBRtspClientStats_concat(STNBRtspClientStats* obj, const STNBRtspStatsCfg* cfg, const BOOL loaded, const BOOL accum, const BOOL total, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, STNBString* dst, const BOOL resetAccum){
	STNBRtspClientStatsOpq* opq = (STNBRtspClientStatsOpq*)obj->opaque; 
	NBThreadMutex_lock(&opq->mutex);
	{
		const STNBRtspClientStatsData data = NBRtspClientStats_getDataLockedOpq_(opq, resetAccum);
		NBRtspClientStats_concatData(&data, cfg, loaded, accum, total, prefixFirst, prefixOthers, ignoreEmpties, dst);
	}
	NBThreadMutex_unlock(&opq->mutex);
}

void NBRtspClientStats_concatData(const STNBRtspClientStatsData* obj, const STNBRtspStatsCfg* cfg, const BOOL loaded, const BOOL accum, const BOOL total, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, STNBString* dst){
	if(dst != NULL){
		BOOL opened = FALSE;
		const char* preExtra = "        |- ";
		STNBString str, pre;
		NBString_initWithSz(&str, 0, 256, 0.10f);
		NBString_initWithSz(&pre, NBString_strLenBytes(prefixOthers) + NBString_strLenBytes(preExtra) + 1, 256, 0.10f);
		NBString_concat(&pre, prefixOthers);
		NBString_concat(&pre, preExtra);
		if(loaded){
			NBString_empty(&str);
			NBRtspClientStats_concatState(&obj->loaded, cfg, "", pre.str, ignoreEmpties, &str);
			if(str.length > 0){
				if(opened){
					NBString_concat(dst, "\n");
					NBString_concat(dst, prefixOthers);
				} else {
					NBString_concat(dst, prefixFirst);
				}
				NBString_concat(dst,  "loaded:  ");
				NBString_concat(dst, str.str);
				opened = TRUE;
			}
		}
		if(accum){
			NBString_empty(&str);
			NBRtspClientStats_concatState(&obj->accum, cfg, "", pre.str, ignoreEmpties, &str);
			if(str.length > 0){
				if(opened){
					NBString_concat(dst, "\n");
					NBString_concat(dst, prefixOthers);
				} else {
					NBString_concat(dst, prefixFirst);
				}
				NBString_concat(dst,  " accum:  ");
				NBString_concat(dst, str.str);
				opened = TRUE;
			}
		}
		if(total){
			NBString_empty(&str);
			NBRtspClientStats_concatState(&obj->total, cfg, "", pre.str, ignoreEmpties, &str);
			if(str.length > 0){
				if(opened){
					NBString_concat(dst, "\n");
					NBString_concat(dst, prefixOthers);
				} else {
					NBString_concat(dst, prefixFirst);
				}
				NBString_concat(dst,  " total:  ");
				NBString_concat(dst, str.str);
				opened = TRUE;
			}
		}
		NBString_release(&pre);
		NBString_release(&str);
	}
}

void NBRtspClientStats_concatState(const STNBRtspClientStatsState* obj, const STNBRtspStatsCfg* cfg, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, STNBString* dst){
	if(dst != NULL){
		BOOL opened = FALSE;
		const char* preExtra = "       |- ";
		STNBString str, pre;
		NBString_initWithSz(&str, 0, 256, 0.10f);
		NBString_initWithSz(&pre, NBString_strLenBytes(prefixOthers) + NBString_strLenBytes(preExtra) + 1, 256, 0.10f);
		NBString_concat(&pre, prefixOthers);
		NBString_concat(&pre, preExtra);
		//rtp
		{
			NBString_empty(&str);
			NBRtpClientStats_concatState(&obj->rtp, &cfg->rtp, "", pre.str, ignoreEmpties, &str);
			if(str.length > 0){
				if(opened){
					NBString_concat(dst, "\n");
					NBString_concat(dst, prefixOthers);
				} else {
					NBString_concat(dst, prefixFirst);
				}
				NBString_concat(dst,  "    rtp:  ");
				NBString_concat(dst, str.str);
				opened = TRUE;
			}
		}
		//UpdCalls
		/*{
			if(!ignoreEmpties || obj->updCalls > 0){
				if(opened){
					NBString_concat(dst, "\n");
					NBString_concat(dst, prefixOthers);
				} else {
					NBString_concat(dst, prefixFirst);
				}
				NBString_concat(dst,  "updCall: ");
				NBString_concatUI64(dst, obj->updCalls);
				opened = TRUE;
			}
		}*/
		NBString_release(&pre);
		NBString_release(&str);
	}
}
