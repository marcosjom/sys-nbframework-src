
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBRtpClientStats.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBStructMaps.h"
#include "nb/core/NBMngrStructMaps.h"

// NBRtpClientStatsTime

STNBStructMapsRec NBRtpClientStatsTime_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtpClientStatsTime_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBRtpClientStatsTime_sharedStructMap);
    if(NBRtpClientStatsTime_sharedStructMap.map == NULL){
        STNBRtpClientStatsTime s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtpClientStatsTime);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, count);
        NBStructMap_addUIntM(map, s, msMin);
        NBStructMap_addUIntM(map, s, msMax);
        NBStructMap_addUIntM(map, s, msTotal);
        NBRtpClientStatsTime_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBRtpClientStatsTime_sharedStructMap);
    return NBRtpClientStatsTime_sharedStructMap.map;
}

// NBRtpClientStatsRcvd

STNBStructMapsRec NBRtpClientStatsRcvd_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtpClientStatsRcvd_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBRtpClientStatsRcvd_sharedStructMap);
    if(NBRtpClientStatsRcvd_sharedStructMap.map == NULL){
        STNBRtpClientStatsRcvd s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtpClientStatsRcvd);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, grpsCount);
        NBStructMap_addUIntM(map, s, pcktsCount);
        NBStructMap_addUIntM(map, s, bytesCount);
        NBStructMap_addUIntM(map, s, minBytes);
        NBStructMap_addUIntM(map, s, maxBytes);
        NBStructMap_addUIntM(map, s, minRcvGrp);
        NBStructMap_addUIntM(map, s, maxRcvGrp);
        NBStructMap_addStructM(map, s, malform, NBRtpQueueCount32_getSharedStructMap());
        NBStructMap_addStructM(map, s, overBuff, NBRtpQueueCount32_getSharedStructMap());
        NBRtpClientStatsRcvd_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBRtpClientStatsRcvd_sharedStructMap);
    return NBRtpClientStatsRcvd_sharedStructMap.map;
}

// NBRtpClientStatsPacketsRcvd

STNBStructMapsRec NBRtpClientStatsPacketsRcvd_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtpClientStatsPacketsRcvd_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBRtpClientStatsPacketsRcvd_sharedStructMap);
    if(NBRtpClientStatsPacketsRcvd_sharedStructMap.map == NULL){
        STNBRtpClientStatsPacketsRcvd s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtpClientStatsPacketsRcvd);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, grpsCount);
        NBStructMap_addUIntM(map, s, pcktsCount);
        NBStructMap_addUIntM(map, s, bytesCount);
        NBStructMap_addUIntM(map, s, minBytes);
        NBStructMap_addUIntM(map, s, maxBytes);
        NBStructMap_addUIntM(map, s, minRcvGrp);
        NBStructMap_addUIntM(map, s, maxRcvGrp);
        NBStructMap_addStructM(map, s, malform, NBRtpQueueCount32_getSharedStructMap());
        NBStructMap_addStructM(map, s, overBuff, NBRtpQueueCount32_getSharedStructMap());
        NBStructMap_addStructM(map, s, wait, NBRtpClientStatsTime_getSharedStructMap());
        NBRtpClientStatsPacketsRcvd_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBRtpClientStatsPacketsRcvd_sharedStructMap);
    return NBRtpClientStatsPacketsRcvd_sharedStructMap.map;
}

// NBRtpClientStatsPacketsCsmd

STNBStructMapsRec NBRtpClientStatsPacketsCsmd_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtpClientStatsPacketsCsmd_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBRtpClientStatsPacketsCsmd_sharedStructMap);
    if(NBRtpClientStatsPacketsCsmd_sharedStructMap.map == NULL){
        STNBRtpClientStatsPacketsCsmd s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtpClientStatsPacketsCsmd);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, count);
        NBStructMap_addUIntM(map, s, bytesCount);
        NBStructMap_addStructM(map, s, ignored, NBRtpQueueCount64_getSharedStructMap());
        NBStructMap_addStructM(map, s, passedThrough, NBRtpQueueCount64_getSharedStructMap());
        NBStructMap_addStructM(map, s, queued, NBRtpQueueCount64_getSharedStructMap());
        NBStructMap_addStructM(map, s, unqueued, NBRtpQueueCount64_getSharedStructMap());
        NBStructMap_addStructM(map, s, delayed, NBRtpQueueCount64_getSharedStructMap());
        NBStructMap_addStructM(map, s, repeated, NBRtpQueueCount64_getSharedStructMap());
        NBStructMap_addStructM(map, s, ignLate, NBRtpQueueCount64Seq_getSharedStructMap());
        NBStructMap_addStructM(map, s, lost, NBRtpQueueCount64_getSharedStructMap());
        NBStructMap_addStructM(map, s, overQueue, NBRtpQueueCount64_getSharedStructMap());
        NBRtpClientStatsPacketsCsmd_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBRtpClientStatsPacketsCsmd_sharedStructMap);
    return NBRtpClientStatsPacketsCsmd_sharedStructMap.map;
}

// NBRtpClientStatsPackets

STNBStructMapsRec NBRtpClientStatsPackets_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtpClientStatsPackets_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBRtpClientStatsPackets_sharedStructMap);
    if(NBRtpClientStatsPackets_sharedStructMap.map == NULL){
        STNBRtpClientStatsPackets s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtpClientStatsPackets);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addStructM(map, s, rcvd, NBRtpClientStatsPacketsRcvd_getSharedStructMap());
        NBStructMap_addStructM(map, s, csmd, NBRtpClientStatsPacketsCsmd_getSharedStructMap());
        NBRtpClientStatsPackets_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBRtpClientStatsPackets_sharedStructMap);
    return NBRtpClientStatsPackets_sharedStructMap.map;
}

// NBRtpClientStatsState

STNBStructMapsRec NBRtpClientStatsState_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtpClientStatsState_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBRtpClientStatsState_sharedStructMap);
    if(NBRtpClientStatsState_sharedStructMap.map == NULL){
        STNBRtpClientStatsState s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtpClientStatsState);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addStructM(map, s, packets, NBRtpClientStatsPackets_getSharedStructMap());
        NBStructMap_addUIntM(map, s, updCalls);
        NBRtpClientStatsState_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBRtpClientStatsState_sharedStructMap);
    return NBRtpClientStatsState_sharedStructMap.map;
}

// NBRtpClientStatsData

STNBStructMapsRec NBRtpClientStatsData_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtpClientStatsData_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBRtpClientStatsData_sharedStructMap);
    if(NBRtpClientStatsData_sharedStructMap.map == NULL){
        STNBRtpClientStatsData s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtpClientStatsData);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addStructM(map, s, loaded, NBRtpClientStatsState_getSharedStructMap());
        NBStructMap_addStructM(map, s, accum, NBRtpClientStatsState_getSharedStructMap());
        NBStructMap_addStructM(map, s, total, NBRtpClientStatsState_getSharedStructMap());
        NBRtpClientStatsData_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBRtpClientStatsData_sharedStructMap);
    return NBRtpClientStatsData_sharedStructMap.map;
}

//Rtp

STNBStructMapsRec NBRtpStatsCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtpStatsCfg_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBRtpStatsCfg_sharedStructMap);
	if(NBRtpStatsCfg_sharedStructMap.map == NULL){
		STNBRtpStatsCfg s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtpStatsCfg);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addEnumM(map, s, statsLevel, NBLogLevel_getSharedEnumMap());
		NBRtpStatsCfg_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBRtpStatsCfg_sharedStructMap);
	return NBRtpStatsCfg_sharedStructMap.map;
}

//Opaque state

typedef struct STNBRtpClientStatsOpq_ {
	STNBRtpClientStatsData	data;
	STNBThreadMutex			mutex;
	//
	SI32					retainCount;
} STNBRtpClientStatsOpq;

//Factory

void NBRtpClientStats_init(STNBRtpClientStats* obj){
	STNBRtpClientStatsOpq* opq = obj->opaque = NBMemory_allocType(STNBRtpClientStatsOpq);
	NBMemory_setZeroSt(*opq, STNBRtpClientStatsOpq);
	NBThreadMutex_init(&opq->mutex);
	//
	opq->retainCount	= 1;
}

void NBRtpClientStats_retain_(STNBRtpClientStatsOpq* opq){
	NBASSERT(opq != NULL)
	NBThreadMutex_lock(&opq->mutex);
	{
		NBASSERT(opq->retainCount > 0)
		opq->retainCount++;
	}
	NBThreadMutex_unlock(&opq->mutex);
}


void NBRtpClientStats_retain(STNBRtpClientStats* obj){
	NBRtpClientStats_retain_((STNBRtpClientStatsOpq*)obj->opaque);
}

void NBRtpClientStats_release_(STNBRtpClientStatsOpq* opq){
	NBASSERT(opq != NULL)
	NBThreadMutex_lock(&opq->mutex);
    NBASSERT(opq->retainCount > 0)
	opq->retainCount--;
	if(opq->retainCount > 0){
		NBThreadMutex_unlock(&opq->mutex);
	} else {
		NBThreadMutex_unlock(&opq->mutex);
		NBThreadMutex_release(&opq->mutex);
		NBMemory_free(opq);
		opq = NULL;
	}
}

void NBRtpClientStats_release(STNBRtpClientStats* obj){
	NBRtpClientStats_release_((STNBRtpClientStatsOpq*)obj->opaque);
}

//Data

STNBRtpClientStatsData NBRtpClientStats_getData(STNBRtpClientStats* obj, const BOOL resetAccum){
	STNBRtpClientStatsData r;
	STNBRtpClientStatsOpq* opq  = (STNBRtpClientStatsOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	{
		r = opq->data;
		if(resetAccum){
			NBMemory_setZero(opq->data.accum);
		}
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

void NBRtpClientStats_concatFormatedBytes_(const UI64 bytesCount, STNBString* dst){
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

void NBRtpClientStats_concat(STNBRtpClientStats* obj, const STNBRtpStatsCfg* cfg, const BOOL loaded, const BOOL accum, const BOOL total, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, STNBString* dst, const BOOL resetAccum){
	STNBRtpClientStatsOpq* opq = (STNBRtpClientStatsOpq*)obj->opaque; 
	NBThreadMutex_lock(&opq->mutex);
	{
		NBRtpClientStats_concatData(&opq->data, cfg, loaded, accum, total, prefixFirst, prefixOthers, ignoreEmpties, dst);
		if(resetAccum){
			NBMemory_setZero(opq->data.accum);
		}
	}
	NBThreadMutex_unlock(&opq->mutex);
}

void NBRtpClientStats_concatData(const STNBRtpClientStatsData* obj, const STNBRtpStatsCfg* cfg, const BOOL loaded, const BOOL accum, const BOOL total, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, STNBString* dst){
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
			NBRtpClientStats_concatState(&obj->loaded, cfg, "", pre.str, ignoreEmpties, &str);
			if(str.length > 0){
				if(opened){
					NBString_concat(dst, "\n");
					NBString_concat(dst, prefixOthers);
				} else {
					NBString_concat(dst, prefixFirst);
				}
				NBString_concat(dst,  "  loaded:  ");
				NBString_concat(dst, str.str);
				opened = TRUE;
			}
		}
		if(accum){
			NBString_empty(&str);
			NBRtpClientStats_concatState(&obj->accum, cfg, "", pre.str, ignoreEmpties, &str);
			if(str.length > 0){
				if(opened){
					NBString_concat(dst, "\n");
					NBString_concat(dst, prefixOthers);
				} else {
					NBString_concat(dst, prefixFirst);
				}
				NBString_concat(dst,  "accum(+):  ");
				NBString_concat(dst, str.str);
				opened = TRUE;
			}
		}
		if(total){
			NBString_empty(&str);
			NBRtpClientStats_concatState(&obj->total, cfg, "", pre.str, ignoreEmpties, &str);
			if(str.length > 0){
				if(opened){
					NBString_concat(dst, "\n");
					NBString_concat(dst, prefixOthers);
				} else {
					NBString_concat(dst, prefixFirst);
				}
				NBString_concat(dst,  "total(+):  ");
				NBString_concat(dst, str.str);
				opened = TRUE;
			}
		}
		NBString_release(&pre);
		NBString_release(&str);
	}
}

void NBRtpClientStats_concatState(const STNBRtpClientStatsState* obj, const STNBRtpStatsCfg* cfg, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, STNBString* dst){
	if(dst != NULL && cfg != NULL){
		BOOL opened = FALSE;
		STNBString str;
		NBString_initWithSz(&str, 0, 256, 0.10f);
		//Packets
		if(cfg->statsLevel >= ENNBLogLevel_Info){
			NBString_empty(&str);
			NBRtpClientStats_concatPackets(&obj->packets, cfg, ignoreEmpties, &str);
			if(str.length > 0){
				if(opened){
					NBString_concat(dst, "\n");
					NBString_concat(dst, prefixOthers);
				} else {
					NBString_concat(dst, prefixFirst);
				}
				NBString_concat(dst,  "packets: ");
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
		NBString_release(&str);
	}
}

void NBRtpClientStats_concatPackets(const STNBRtpClientStatsPackets* obj, const STNBRtpStatsCfg* cfg, const BOOL ignoreEmpties, STNBString* dst){
	if(dst != NULL && cfg != NULL && cfg->statsLevel > ENNBLogLevel_None){
		BOOL opened = FALSE;
		//rcvd
		if(cfg->statsLevel >= ENNBLogLevel_Info && (!ignoreEmpties || obj->rcvd.pcktsCount > 0)){
			if(opened) NBString_concat(dst, ", ");
			if(obj->rcvd.bytesCount > 0){
				NBRtpClientStats_concatFormatedBytes_(obj->rcvd.bytesCount, dst);
				NBString_concat(dst, "/");
			}	
			NBString_concatUI32(dst, obj->rcvd.pcktsCount);
			if(obj->rcvd.grpsCount){
				NBString_concat(dst, "/");
				NBString_concatUI32(dst, obj->rcvd.grpsCount);
			}
			if(obj->rcvd.minBytes > 0 || obj->rcvd.maxBytes > 0 || obj->rcvd.grpsCount > 0){
				BOOL opened2 = FALSE;
				NBString_concat(dst, " grps(");
				if(obj->rcvd.minBytes > 0 || obj->rcvd.maxBytes > 0){
					if(opened2) NBString_concat(dst, ", ");
					if(obj->rcvd.minBytes == obj->rcvd.maxBytes){
						NBString_concatUI32(dst, obj->rcvd.maxBytes);
						NBString_concat(dst, "B");
					} else {
						NBString_concatUI32(dst, obj->rcvd.minBytes);
						NBString_concat(dst, "-");
						NBString_concatUI32(dst, (UI32)(obj->rcvd.bytesCount / obj->rcvd.pcktsCount));
						NBString_concat(dst, "-");
						NBString_concatUI32(dst, obj->rcvd.maxBytes);
						NBString_concat(dst, "B");
					}
					opened2 = TRUE;
				}
				if(obj->rcvd.grpsCount > 0){
					if(opened2) NBString_concat(dst, "/");
					if(obj->rcvd.minRcvGrp == obj->rcvd.maxRcvGrp){
						NBString_concatUI32(dst, obj->rcvd.maxRcvGrp);
					} else {
						NBString_concatUI32(dst, obj->rcvd.minRcvGrp);
						NBString_concat(dst, "-");
						NBString_concatUI32(dst, (UI32)(obj->rcvd.pcktsCount / obj->rcvd.grpsCount));
						NBString_concat(dst, "-");
						NBString_concatUI32(dst, obj->rcvd.maxRcvGrp);
					}
					opened2 = TRUE;
				}
				NBString_concat(dst, ")");
			}
			//socket queries with packets
			if(cfg->statsLevel >= ENNBLogLevel_Info && (!ignoreEmpties || obj->rcvd.wait.count > 0)){
				//wait
				NBString_concat(dst, " wait(");
				if(obj->rcvd.wait.msMin == obj->rcvd.wait.msMax){
					NBString_concatUI32(dst, obj->rcvd.wait.msMin);
					NBString_concat(dst, "ms");
				} else {
					NBString_concatUI32(dst, obj->rcvd.wait.msMin);
					NBString_concat(dst, "-");
					NBString_concatUI32(dst, (UI32)(obj->rcvd.wait.msTotal / obj->rcvd.wait.count));
					NBString_concat(dst, "-");
					NBString_concatUI32(dst, obj->rcvd.wait.msMax);
					NBString_concat(dst, "ms");
				}
				NBString_concat(dst, "/");
				NBString_concatUI32(dst, obj->rcvd.wait.count);
				NBString_concat(dst, ")");
			}
			opened = TRUE;
		}
		//Csmd
		{
			//pass-through (passed directly without using the queue, best scenario posible)
			if(cfg->statsLevel >= ENNBLogLevel_Info && (!ignoreEmpties || obj->csmd.passedThrough.count > 0)){
				if(opened) NBString_concat(dst, ", ");
				NBString_concat(dst, "passby: ");
				if(obj->csmd.passedThrough.bytesCount > 0){
					NBRtpClientStats_concatFormatedBytes_(obj->csmd.passedThrough.bytesCount, dst);
					NBString_concat(dst, "/");
				}
				NBString_concatUI32(dst, obj->csmd.passedThrough.count);
				opened = TRUE;
			}
			//queued and unqueued
			if(cfg->statsLevel >= ENNBLogLevel_Info && (!ignoreEmpties || obj->csmd.queued.count > 0 || obj->csmd.unqueued.count > 0)){
				BOOL opened2 = FALSE;
				if(opened) NBString_concat(dst, ", ");
				NBString_concat(dst, "queued: ");
				if(!ignoreEmpties || obj->csmd.queued.count > 0){
					if(opened2) NBString_concat(dst, " ");
					NBString_concat(dst, "+");
					if(obj->csmd.queued.bytesCount > 0){
						NBRtpClientStats_concatFormatedBytes_(obj->csmd.queued.bytesCount, dst);
						NBString_concat(dst, "/");
					}
					NBString_concatUI32(dst, obj->csmd.queued.count);
					opened2 = TRUE;
				}
				if(!ignoreEmpties || obj->csmd.unqueued.count > 0){
					if(opened2) NBString_concat(dst, " ");
					NBString_concat(dst, "-");
					if(obj->csmd.unqueued.bytesCount > 0){
						NBRtpClientStats_concatFormatedBytes_(obj->csmd.unqueued.bytesCount, dst);
						NBString_concat(dst, "/");
					}
					NBString_concatUI32(dst, obj->csmd.unqueued.count);
					opened2 = TRUE;
				}
				opened = TRUE;
			}
			//Repeated
			if(cfg->statsLevel >= ENNBLogLevel_Warning && (!ignoreEmpties || obj->csmd.repeated.count > 0)){
				if(opened) NBString_concat(dst, ", ");
				NBString_concat(dst, "repeatd: ");
				if(obj->csmd.repeated.bytesCount > 0){
					NBRtpClientStats_concatFormatedBytes_(obj->csmd.repeated.bytesCount, dst);
					NBString_concat(dst, "/");
				}
				NBString_concatUI32(dst, obj->csmd.repeated.count);
				opened = TRUE;
			}
			//Delayed arrival
			if(cfg->statsLevel >= ENNBLogLevel_Warning && (!ignoreEmpties || obj->csmd.delayed.count > 0)){
				if(opened) NBString_concat(dst, ", ");
				NBString_concat(dst, "delayed: ");
				if(obj->csmd.delayed.bytesCount > 0){
					NBRtpClientStats_concatFormatedBytes_(obj->csmd.delayed.bytesCount, dst);
					NBString_concat(dst, "/");
				}
				NBString_concatUI32(dst, obj->csmd.delayed.count);
				opened = TRUE;
			}
			//Dropped, no buffer slot available
			if(cfg->statsLevel >= ENNBLogLevel_Error && (!ignoreEmpties || obj->csmd.overQueue.count > 0)){
				if(opened) NBString_concat(dst, ", ");
				NBString_concat(dst, "overQueue: ");
				if(obj->csmd.overQueue.bytesCount > 0){
					NBRtpClientStats_concatFormatedBytes_(obj->csmd.overQueue.bytesCount, dst);
					NBString_concat(dst, "/");
				}
				NBString_concatUI32(dst, obj->csmd.overQueue.count);
				opened = TRUE;
			}
			//Ignored by late-arrival
			if(cfg->statsLevel >= ENNBLogLevel_Error && (!ignoreEmpties || obj->csmd.ignLate.count > 0)){
				if(opened) NBString_concat(dst, ", ");
				NBString_concat(dst, "ignLate: ");
				if(obj->csmd.ignLate.bytesCount > 0){
					NBRtpClientStats_concatFormatedBytes_(obj->csmd.ignLate.bytesCount, dst);
					NBString_concat(dst, "/");
				}	
				NBString_concatUI32(dst, obj->csmd.ignLate.count);
				NBString_concat(dst, " (");
				NBString_concatUI32(dst, (UI32)obj->csmd.ignLate.gapMax);
				NBString_concat(dst, " gapMax)");
				opened = TRUE;
			}
			//Lost
			if(cfg->statsLevel >= ENNBLogLevel_Error && (!ignoreEmpties || obj->csmd.lost.count > 0)){
				if(opened) NBString_concat(dst, ", ");
				NBString_concat(dst, "lost: ");
				if(obj->csmd.lost.bytesCount > 0){
					NBRtpClientStats_concatFormatedBytes_(obj->csmd.lost.bytesCount, dst);
					NBString_concat(dst, "/");
				}
				NBString_concatUI32(dst, obj->csmd.lost.count);
				opened = TRUE;
			}
			//Ignored (no regitered listener or not processed by queue)
			if(cfg->statsLevel >= ENNBLogLevel_Error && (!ignoreEmpties || obj->csmd.ignored.count > 0)){
				if(opened) NBString_concat(dst, ", ");
				NBString_concat(dst, "ignored: ");
				if(obj->csmd.ignored.bytesCount > 0){
					NBRtpClientStats_concatFormatedBytes_(obj->csmd.ignored.bytesCount, dst);
					NBString_concat(dst, "/");
				}
				NBString_concatUI32(dst, obj->csmd.ignored.count);
				opened = TRUE;
			}
		}
		//Recv
		if(cfg->statsLevel >= ENNBLogLevel_Error && (!ignoreEmpties || obj->rcvd.overBuff.count > 0)){
			if(opened) NBString_concat(dst, ", ");
			NBString_concat(dst, "overBuff: ");
			if(obj->rcvd.overBuff.bytesCount > 0){
				NBRtpClientStats_concatFormatedBytes_(obj->rcvd.overBuff.bytesCount, dst);
				NBString_concat(dst, "/");
			}
			NBString_concatUI32(dst, obj->rcvd.overBuff.count);
			opened = TRUE;
		}
		if(cfg->statsLevel >= ENNBLogLevel_Error && (!ignoreEmpties || obj->rcvd.malform.count > 0)){
			if(opened) NBString_concat(dst, ", ");
			NBString_concat(dst, "malform: ");
			if(obj->rcvd.malform.bytesCount > 0){
				NBRtpClientStats_concatFormatedBytes_(obj->rcvd.malform.bytesCount, dst);
				NBString_concat(dst, "/");
			}
			NBString_concatUI32(dst, obj->rcvd.malform.count);
			opened = TRUE;
		}
		//Add total packets
		/*if(opened && addTotalIfNoEmpty && (!ignoreEmpties || obj->rcvd.pcktsCount > 0)){
			if(opened) NBString_concat(dst, ", ");
			NBString_concat(dst, "rcvd(");
			if(obj->rcvd.bytesCount > 0){
				NBRtpClientStats_concatFormatedBytes_(obj->rcvd.bytesCount, dst);
				NBString_concat(dst, "/");
			}	
			NBString_concatUI32(dst, obj->rcvd.pcktsCount);
			NBString_concat(dst, ")");
			opened = TRUE;
		}*/
	}
}

//Packets

void NBRtpClientStats_applyUpdate(STNBRtpClientStats* obj, const STNBRtpClientStatsUpd* updResult){
	STNBRtpClientStatsOpq* opq  = (STNBRtpClientStatsOpq*)obj->opaque;
	if(updResult != NULL && (updResult->rcvd.pcktsCount != 0 || updResult->csmd.count != 0)){
		NBThreadMutex_lock(&opq->mutex);
		//rcvd
		if(updResult->rcvd.pcktsCount != 0){
			//rcvd
			{
				//accum
				{
					//bytesMinMax
					{
						if(opq->data.accum.packets.rcvd.pcktsCount == 0 || opq->data.accum.packets.rcvd.minBytes > updResult->rcvd.minBytes){
							opq->data.accum.packets.rcvd.minBytes = updResult->rcvd.minBytes;
						}
						if(opq->data.accum.packets.rcvd.pcktsCount == 0 || opq->data.accum.packets.rcvd.maxBytes < updResult->rcvd.maxBytes){
							opq->data.accum.packets.rcvd.maxBytes = updResult->rcvd.maxBytes;
						}
					}
					//grpsMinMax
					{
						if(opq->data.accum.packets.rcvd.grpsCount == 0 || opq->data.accum.packets.rcvd.minRcvGrp > updResult->rcvd.minRcvGrp){
							opq->data.accum.packets.rcvd.minRcvGrp = updResult->rcvd.minRcvGrp;
						}
						if(opq->data.accum.packets.rcvd.grpsCount == 0 || opq->data.accum.packets.rcvd.maxRcvGrp < updResult->rcvd.maxRcvGrp){
							opq->data.accum.packets.rcvd.maxRcvGrp = updResult->rcvd.maxRcvGrp;
						}
					}
					opq->data.accum.packets.rcvd.grpsCount += updResult->rcvd.grpsCount;
					opq->data.accum.packets.rcvd.pcktsCount += updResult->rcvd.pcktsCount;
					opq->data.accum.packets.rcvd.bytesCount += updResult->rcvd.bytesCount;
				}
				//total
				{
					//bytesMinMax
					{
						if(opq->data.total.packets.rcvd.pcktsCount == 0 || opq->data.total.packets.rcvd.minBytes > updResult->rcvd.minBytes){
							opq->data.total.packets.rcvd.minBytes = updResult->rcvd.minBytes;
						}
						if(opq->data.total.packets.rcvd.pcktsCount == 0 || opq->data.total.packets.rcvd.maxBytes < updResult->rcvd.maxBytes){
							opq->data.total.packets.rcvd.maxBytes = updResult->rcvd.maxBytes;
						}
					}
					//grpsMinMax
					{
						if(opq->data.total.packets.rcvd.grpsCount == 0 || opq->data.total.packets.rcvd.minRcvGrp > updResult->rcvd.minRcvGrp){
							opq->data.total.packets.rcvd.minRcvGrp = updResult->rcvd.minRcvGrp;
						}
						if(opq->data.total.packets.rcvd.grpsCount == 0 || opq->data.total.packets.rcvd.maxRcvGrp < updResult->rcvd.maxRcvGrp){
							opq->data.total.packets.rcvd.maxRcvGrp = updResult->rcvd.maxRcvGrp;
						}
					}
					opq->data.total.packets.rcvd.grpsCount += updResult->rcvd.grpsCount;
					opq->data.total.packets.rcvd.pcktsCount += updResult->rcvd.pcktsCount;
					opq->data.total.packets.rcvd.bytesCount += updResult->rcvd.bytesCount;
				}
			}
			//malformed
			{
				//accum
				opq->data.accum.packets.rcvd.malform.count		+= updResult->rcvd.malform.count;
				opq->data.accum.packets.rcvd.malform.bytesCount	+= updResult->rcvd.malform.bytesCount;
				//total
				opq->data.total.packets.rcvd.malform.count		+= updResult->rcvd.malform.count;
				opq->data.total.packets.rcvd.malform.bytesCount	+= updResult->rcvd.malform.bytesCount;
			}
			//overflowed buffer
			{
				//accum
				opq->data.accum.packets.rcvd.overBuff.count		+= updResult->rcvd.overBuff.count;
				opq->data.accum.packets.rcvd.overBuff.bytesCount	+= updResult->rcvd.overBuff.bytesCount;
				//total
				opq->data.total.packets.rcvd.overBuff.count		+= updResult->rcvd.overBuff.count;
				opq->data.total.packets.rcvd.overBuff.bytesCount	+= updResult->rcvd.overBuff.bytesCount;
			}
		}
		if(updResult->csmd.count != 0){
			//ignored
			{
				//accum
				opq->data.accum.packets.csmd.ignored.count += updResult->csmd.ignored.count;
				opq->data.accum.packets.csmd.ignored.bytesCount += updResult->csmd.ignored.bytesCount;
				//total
				opq->data.total.packets.csmd.ignored.count += updResult->csmd.ignored.count;
				opq->data.total.packets.csmd.ignored.bytesCount += updResult->csmd.ignored.bytesCount;
			}
			//pass-through (passed directly without using the queue, best scenario posible)
			{
				//accum
				opq->data.accum.packets.csmd.passedThrough.count		+= updResult->csmd.passedThrough.count;
				opq->data.accum.packets.csmd.passedThrough.bytesCount	+= updResult->csmd.passedThrough.bytesCount;
				//accum
				opq->data.total.packets.csmd.passedThrough.count		+= updResult->csmd.passedThrough.count;
				opq->data.total.packets.csmd.passedThrough.bytesCount	+= updResult->csmd.passedThrough.bytesCount;
			}
			//queued
			{
				//loaded
				opq->data.loaded.packets.csmd.queued.count				+= updResult->csmd.queued.count;
				opq->data.loaded.packets.csmd.queued.bytesCount			+= updResult->csmd.queued.bytesCount;
				//accum
				opq->data.accum.packets.csmd.queued.count			+= updResult->csmd.queued.count;
				opq->data.accum.packets.csmd.queued.bytesCount	+= updResult->csmd.queued.bytesCount;
				//total
				opq->data.total.packets.csmd.queued.count			+= updResult->csmd.queued.count;
				opq->data.total.packets.csmd.queued.bytesCount	+= updResult->csmd.queued.bytesCount;
			}
			//unqueued
			{
				//loaded
				NBASSERT(opq->data.loaded.packets.csmd.queued.count >= updResult->csmd.unqueued.count)
				NBASSERT(opq->data.loaded.packets.csmd.queued.bytesCount >= updResult->csmd.unqueued.bytesCount)
				opq->data.loaded.packets.csmd.queued.count -= updResult->csmd.unqueued.count;
				opq->data.loaded.packets.csmd.queued.bytesCount -= updResult->csmd.unqueued.bytesCount;
				//accum
				opq->data.accum.packets.csmd.unqueued.count += updResult->csmd.unqueued.count;
				opq->data.accum.packets.csmd.unqueued.bytesCount += updResult->csmd.unqueued.bytesCount;
				//total
				opq->data.total.packets.csmd.unqueued.count += updResult->csmd.unqueued.count;
				opq->data.total.packets.csmd.unqueued.bytesCount += updResult->csmd.unqueued.bytesCount;
			}
			//delayed
			{
				//accum
				opq->data.accum.packets.csmd.delayed.count		+= updResult->csmd.delayed.count;
				opq->data.accum.packets.csmd.delayed.bytesCount	+= updResult->csmd.delayed.bytesCount;
				//accum
				opq->data.total.packets.csmd.delayed.count		+= updResult->csmd.delayed.count;
				opq->data.total.packets.csmd.delayed.bytesCount	+= updResult->csmd.delayed.bytesCount;
			}
			//overflowed queue
			{
				//accum
				opq->data.accum.packets.csmd.overQueue.count		+= updResult->csmd.overQueue.count;
				opq->data.accum.packets.csmd.overQueue.bytesCount	+= updResult->csmd.overQueue.bytesCount;
				//total
				opq->data.total.packets.csmd.overQueue.count		+= updResult->csmd.overQueue.count;
				opq->data.total.packets.csmd.overQueue.bytesCount	+= updResult->csmd.overQueue.bytesCount;
			}
			//ignored-late
			{
				//accum
				opq->data.accum.packets.csmd.ignLate.count += updResult->csmd.ignLate.count;
				opq->data.accum.packets.csmd.ignLate.bytesCount += updResult->csmd.ignLate.bytesCount;
				if(opq->data.accum.packets.csmd.ignLate.gapMax < updResult->csmd.ignLate.gapMax){
					opq->data.accum.packets.csmd.ignLate.gapMax = updResult->csmd.ignLate.gapMax;
				}
				//total
				opq->data.total.packets.csmd.ignLate.count += updResult->csmd.ignLate.count;
				opq->data.total.packets.csmd.ignLate.bytesCount += updResult->csmd.ignLate.bytesCount;
				if(opq->data.total.packets.csmd.ignLate.gapMax < updResult->csmd.ignLate.gapMax){
					opq->data.total.packets.csmd.ignLate.gapMax = updResult->csmd.ignLate.gapMax;
				}
			}
			//repeated
			{
				//accum
				opq->data.accum.packets.csmd.repeated.count += updResult->csmd.repeated.count;
				opq->data.accum.packets.csmd.repeated.bytesCount += updResult->csmd.repeated.bytesCount;
				//total
				opq->data.total.packets.csmd.repeated.count += updResult->csmd.repeated.count;
				opq->data.total.packets.csmd.repeated.bytesCount += updResult->csmd.repeated.bytesCount;
			}
			//lost
			{
				//accum
				opq->data.accum.packets.csmd.lost.count += updResult->csmd.lost.count;
				opq->data.accum.packets.csmd.lost.bytesCount += updResult->csmd.lost.bytesCount;
				//total
				opq->data.total.packets.csmd.lost.count += updResult->csmd.lost.count;
				opq->data.total.packets.csmd.lost.bytesCount += updResult->csmd.lost.bytesCount;
			}
		}
		//upd calls
		{
			//accum
			opq->data.accum.updCalls++;
			//total
			opq->data.total.updCalls++;
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
}

//NBRtpClientStatsUpd

STNBStructMapsRec NBRtpClientStatsUpd_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtpClientStatsUpd_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBRtpClientStatsUpd_sharedStructMap);
    if(NBRtpClientStatsUpd_sharedStructMap.map == NULL){
        STNBRtpClientStatsUpd s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtpClientStatsUpd);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addStructM(map, s, rcvd, NBRtpClientStatsRcvd_getSharedStructMap());
        NBStructMap_addStructM(map, s, csmd, NBRtpStatsCsmUpd_getSharedStructMap());
        NBRtpClientStatsUpd_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBRtpClientStatsUpd_sharedStructMap);
    return NBRtpClientStatsUpd_sharedStructMap.map;
}

void NBRtpClientStatsUpd_init(STNBRtpClientStatsUpd* obj){
	NBMemory_setZeroSt(*obj, STNBRtpClientStatsUpd);
}

void NBRtpClientStatsUpd_release(STNBRtpClientStatsUpd* obj){
	//nothing
}

void NBRtpClientStatsUpd_reset(STNBRtpClientStatsUpd* obj){
	NBMemory_setZeroSt(*obj, STNBRtpClientStatsUpd);
}

void NBRtpClientStatsUpd_add(STNBRtpClientStatsUpd* obj, const STNBRtpClientStatsUpd* other){
	//received
	{
		if(other->rcvd.grpsCount > 0){
			if(obj->rcvd.grpsCount == 0){ 
				obj->rcvd.minRcvGrp		= other->rcvd.minRcvGrp;	//min packets received in one rcv call.
				obj->rcvd.maxRcvGrp		= other->rcvd.maxRcvGrp;	//max packets received in one rcv call.
			} else {
				if(obj->rcvd.minRcvGrp > other->rcvd.minRcvGrp) obj->rcvd.minRcvGrp = other->rcvd.minRcvGrp;
				if(obj->rcvd.maxRcvGrp < other->rcvd.maxRcvGrp) obj->rcvd.maxRcvGrp = other->rcvd.maxRcvGrp;
			}
		}
		if(other->rcvd.bytesCount > 0){
			if(obj->rcvd.bytesCount == 0){
				obj->rcvd.minBytes		= other->rcvd.minBytes;	//min size
				obj->rcvd.maxBytes		= other->rcvd.maxBytes;	//max size
			} else {
				if(obj->rcvd.minBytes > other->rcvd.minBytes) obj->rcvd.minBytes = other->rcvd.minBytes;
				if(obj->rcvd.maxBytes < other->rcvd.maxBytes) obj->rcvd.maxBytes = other->rcvd.maxBytes;
			}
		}
		obj->rcvd.grpsCount			+= other->rcvd.grpsCount;	//populated rcv-calls (each call returns a groups of packets)
		obj->rcvd.pcktsCount		+= other->rcvd.pcktsCount;	//packets count
		obj->rcvd.bytesCount		+= other->rcvd.bytesCount;	//bytes count
		//malform, RTP header parse failure
		obj->rcvd.malform.count		+= other->rcvd.malform.count;
		obj->rcvd.malform.bytesCount += other->rcvd.malform.bytesCount;
		//overflowed buffer, lack of space on rtpBuffer (UDP arrival)
		obj->rcvd.overBuff.count	+= other->rcvd.overBuff.count;
		obj->rcvd.overBuff.bytesCount += other->rcvd.overBuff.bytesCount;
	}
	//consumed
	{
		obj->csmd.count				+= other->csmd.count;
		obj->csmd.bytesCount		+= other->csmd.bytesCount;
		//ignored, not processed
		obj->csmd.ignored.count		+= other->csmd.ignored.count;
		obj->csmd.ignored.bytesCount += other->csmd.ignored.bytesCount;
		//pass-through (passed directly without using the queue, best scenario posible)
		obj->csmd.passedThrough.count += other->csmd.passedThrough.count;
		obj->csmd.passedThrough.bytesCount += other->csmd.passedThrough.bytesCount;
		//added, populated added to queue
		obj->csmd.queued.count		+= other->csmd.queued.count;
		obj->csmd.queued.bytesCount += other->csmd.queued.bytesCount;
		//removed, populated removed from queue
		obj->csmd.unqueued.count	+= other->csmd.unqueued.count;
		obj->csmd.unqueued.bytesCount += other->csmd.unqueued.bytesCount;
		//filled an existing gap-slot
		obj->csmd.delayed.count		+= other->csmd.delayed.count;
		obj->csmd.delayed.bytesCount += other->csmd.delayed.bytesCount;
		//repeated (received twice while in queue)
		obj->csmd.repeated.count	+= other->csmd.repeated.count;
		obj->csmd.repeated.bytesCount += other->csmd.repeated.bytesCount;
		//ingored-late (received after queue moved ahead)
		obj->csmd.ignLate.count		+= other->csmd.ignLate.count;
		obj->csmd.ignLate.bytesCount += other->csmd.ignLate.bytesCount;
		obj->csmd.ignLate.gapMax	+= other->csmd.ignLate.gapMax;
		//lost (queue moved forward before receiving its payload)
		obj->csmd.lost.count		+= other->csmd.lost.count;
		obj->csmd.lost.bytesCount	+= other->csmd.lost.bytesCount;
		//overflowed queue, lack of space on rtpQueue (ordering and delaying buffer)
		obj->csmd.overQueue.count	+= other->csmd.overQueue.count;
		obj->csmd.overQueue.bytesCount += other->csmd.overQueue.bytesCount;
	}
}
