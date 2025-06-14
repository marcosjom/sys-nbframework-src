
#include "nb/NBFrameworkPch.h"
#include "nb/core/NBDataPtrsStats.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBStructMaps.h"
#include "nb/core/NBMngrStructMaps.h"

//Buffers

STNBStructMapsRec NBDataPtrsStatsCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBDataPtrsStatsCfg_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBDataPtrsStatsCfg_sharedStructMap);
	if(NBDataPtrsStatsCfg_sharedStructMap.map == NULL){
		STNBDataPtrsStatsCfg s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBDataPtrsStatsCfg);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addEnumM(map, s, statsLevel, NBLogLevel_getSharedEnumMap());
		NBDataPtrsStatsCfg_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBDataPtrsStatsCfg_sharedStructMap);
	return NBDataPtrsStatsCfg_sharedStructMap.map;
}

// NBDataPtrsStatsBuffer

STNBStructMapsRec NBDataPtrsStatsBuffer_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBDataPtrsStatsBuffer_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBDataPtrsStatsBuffer_sharedStructMap);
    if(NBDataPtrsStatsBuffer_sharedStructMap.map == NULL){
        STNBDataPtrsStatsBuffer s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBDataPtrsStatsBuffer);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, count);
        NBStructMap_addUIntM(map, s, bytesCount);
        NBDataPtrsStatsBuffer_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBDataPtrsStatsBuffer_sharedStructMap);
    return NBDataPtrsStatsBuffer_sharedStructMap.map;
}

// NBDataPtrsStatsStacks

STNBStructMapsRec NBDataPtrsStatsStacks_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBDataPtrsStatsStacks_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBDataPtrsStatsStacks_sharedStructMap);
    if(NBDataPtrsStatsStacks_sharedStructMap.map == NULL){
        STNBDataPtrsStatsStacks s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBDataPtrsStatsStacks);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, resizesCount);
        NBStructMap_addUIntM(map, s, swapsCount);
        NBDataPtrsStatsStacks_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBDataPtrsStatsStacks_sharedStructMap);
    return NBDataPtrsStatsStacks_sharedStructMap.map;
}

// NBDataPtrsStatsBuffers

STNBStructMapsRec NBDataPtrsStatsBuffers_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBDataPtrsStatsBuffers_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBDataPtrsStatsBuffers_sharedStructMap);
    if(NBDataPtrsStatsBuffers_sharedStructMap.map == NULL){
        STNBDataPtrsStatsBuffers s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBDataPtrsStatsBuffers);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addStructM(map, s, cur, NBDataPtrsStatsBuffer_getSharedStructMap());
        NBStructMap_addStructM(map, s, min, NBDataPtrsStatsBuffer_getSharedStructMap());
        NBStructMap_addStructM(map, s, max, NBDataPtrsStatsBuffer_getSharedStructMap());
        NBStructMap_addStructM(map, s, stacks, NBDataPtrsStatsStacks_getSharedStructMap());
        NBStructMap_addStructM(map, s, alloc, NBDataPtrsStatsBuffer_getSharedStructMap());
        NBStructMap_addStructM(map, s, freed, NBDataPtrsStatsBuffer_getSharedStructMap());
        NBStructMap_addStructM(map, s, given, NBDataPtrsStatsBuffer_getSharedStructMap());
        NBStructMap_addStructM(map, s, taken, NBDataPtrsStatsBuffer_getSharedStructMap());
        NBDataPtrsStatsBuffers_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBDataPtrsStatsBuffers_sharedStructMap);
    return NBDataPtrsStatsBuffers_sharedStructMap.map;
}

// NBDataPtrsStatsState

STNBStructMapsRec NBDataPtrsStatsState_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBDataPtrsStatsState_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBDataPtrsStatsState_sharedStructMap);
    if(NBDataPtrsStatsState_sharedStructMap.map == NULL){
        STNBDataPtrsStatsState s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBDataPtrsStatsState);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addStructM(map, s, buffs, NBDataPtrsStatsBuffers_getSharedStructMap());
        NBStructMap_addUIntM(map, s, updCalls);
        NBDataPtrsStatsState_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBDataPtrsStatsState_sharedStructMap);
    return NBDataPtrsStatsState_sharedStructMap.map;
}

// NBDataPtrsStatsData

STNBStructMapsRec NBDataPtrsStatsData_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBDataPtrsStatsData_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBDataPtrsStatsData_sharedStructMap);
    if(NBDataPtrsStatsData_sharedStructMap.map == NULL){
        STNBDataPtrsStatsData s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBDataPtrsStatsData);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addStructM(map, s, loaded, NBDataPtrsStatsState_getSharedStructMap());
        NBStructMap_addStructM(map, s, accum, NBDataPtrsStatsState_getSharedStructMap());
        NBStructMap_addStructM(map, s, total, NBDataPtrsStatsState_getSharedStructMap());
        NBDataPtrsStatsData_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBDataPtrsStatsData_sharedStructMap);
    return NBDataPtrsStatsData_sharedStructMap.map;
}

// NBDataPtrsStatsUpdItm

STNBStructMapsRec NBDataPtrsStatsUpdItm_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBDataPtrsStatsUpdItm_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBDataPtrsStatsUpdItm_sharedStructMap);
    if(NBDataPtrsStatsUpdItm_sharedStructMap.map == NULL){
        STNBDataPtrsStatsUpdItm s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBDataPtrsStatsUpdItm);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, count);
        NBStructMap_addUIntM(map, s, bytes);
        NBDataPtrsStatsUpdItm_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBDataPtrsStatsUpdItm_sharedStructMap);
    return NBDataPtrsStatsUpdItm_sharedStructMap.map;
}

// NBDataPtrsStatsUpd

STNBStructMapsRec NBDataPtrsStatsUpd_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBDataPtrsStatsUpd_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBDataPtrsStatsUpd_sharedStructMap);
    if(NBDataPtrsStatsUpd_sharedStructMap.map == NULL){
        STNBDataPtrsStatsUpd s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBDataPtrsStatsUpd);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addStructM(map, s, alloc, NBDataPtrsStatsUpdItm_getSharedStructMap());
        NBStructMap_addStructM(map, s, freed, NBDataPtrsStatsUpdItm_getSharedStructMap());
        NBStructMap_addStructM(map, s, given, NBDataPtrsStatsUpdItm_getSharedStructMap());
        NBStructMap_addStructM(map, s, taken, NBDataPtrsStatsUpdItm_getSharedStructMap());
        NBStructMap_addUIntM(map, s, stackResize);
        NBStructMap_addUIntM(map, s, stacksSwaps);
        NBDataPtrsStatsUpd_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBDataPtrsStatsUpd_sharedStructMap);
    return NBDataPtrsStatsUpd_sharedStructMap.map;
}


//Opaque state

typedef struct STNBDataPtrsStatsOpq_ {
	STNBObject				prnt;	//parent
	STNBDataPtrsStatsData	data;
} STNBDataPtrsStatsOpq;

NB_OBJREF_BODY(NBDataPtrsStats, STNBDataPtrsStatsOpq, NBObject)

//init

void NBDataPtrsStats_initZeroed(STNBObject* obj){
	//STNBDataPtrsStatsOpq* opq = (STNBDataPtrsStatsOpq*)obj;
}

void NBDataPtrsStats_uninitLocked(STNBObject* obj){
	//STNBDataPtrsStatsOpq* opq = (STNBDataPtrsStatsOpq*)obj;
}

//
	
STNBDataPtrsStatsData NBDataPtrsStats_getData(STNBDataPtrsStatsRef ref, const BOOL resetAccum){
	STNBDataPtrsStatsData r;
	STNBDataPtrsStatsOpq* opq = (STNBDataPtrsStatsOpq*)ref.opaque;
	NBASSERT(NBDataPtrsStats_isClass(ref))
	NBObject_lock(opq);
	{
		r = opq->data;
		//reset
		if(resetAccum){
			NBMemory_setZero(opq->data.accum);
		}
	}
	NBObject_unlock(opq);
	return r;
}

//

void NBDataPtrsStats_concatFormatedBytes_(const UI64 bytesCount, STNBString* dst){
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

void NBDataPtrsStats_concat(STNBDataPtrsStatsRef ref, const STNBDataPtrsStatsCfg* cfg, const BOOL loaded, const BOOL accum, const BOOL total, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, STNBString* dst, const BOOL resetAccum){
	STNBDataPtrsStatsOpq* opq = (STNBDataPtrsStatsOpq*)ref.opaque;
	NBASSERT(NBDataPtrsStats_isClass(ref))
	NBObject_lock(opq);
	{
		NBDataPtrsStats_concatData(&opq->data, cfg, loaded, accum, total, prefixFirst, prefixOthers, ignoreEmpties, dst);
		//reset
		if(resetAccum){
			NBMemory_setZero(opq->data.accum);
		}
	}
	NBObject_unlock(opq);
}

void NBDataPtrsStats_concatData(const STNBDataPtrsStatsData* obj, const STNBDataPtrsStatsCfg* cfg, const BOOL loaded, const BOOL accum, const BOOL total, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, STNBString* dst){
	if(dst != NULL){
		BOOL opened = FALSE;
		const char* preExtra = "     |- ";
		STNBString str, pre;
		NBString_initWithSz(&str, 0, 256, 0.10f);
		NBString_initWithSz(&pre, NBString_strLenBytes(prefixOthers) + NBString_strLenBytes(preExtra) + 1, 256, 0.10f);
		NBString_concat(&pre, prefixOthers);
		NBString_concat(&pre, preExtra);
		if(loaded){
			NBString_empty(&str);
			NBDataPtrsStats_concatState(&obj->loaded, cfg, TRUE, "", pre.str, ignoreEmpties, &str);
			if(str.length > 0){
				if(opened){
					NBString_concat(dst, "\n");
					NBString_concat(dst, prefixOthers);
				} else {
					NBString_concat(dst, prefixFirst);
				}
				NBString_concat(dst,  "loaded: ");
				NBString_concat(dst, str.str);
				opened = TRUE;
			}
		}
		if(accum){
			NBString_empty(&str);
			NBDataPtrsStats_concatState(&obj->accum, cfg, FALSE, "", pre.str, ignoreEmpties, &str);
			if(str.length > 0){
				if(opened){
					NBString_concat(dst, "\n");
					NBString_concat(dst, prefixOthers);
				} else {
					NBString_concat(dst, prefixFirst);
				}
				NBString_concat(dst,  " accum: ");
				NBString_concat(dst, str.str);
				opened = TRUE;
			}
		}
		if(total){
			NBString_empty(&str);
			NBDataPtrsStats_concatState(&obj->total, cfg, FALSE, "", pre.str, ignoreEmpties, &str);
			if(str.length > 0){
				if(opened){
					NBString_concat(dst, "\n");
					NBString_concat(dst, prefixOthers);
				} else {
					NBString_concat(dst, prefixFirst);
				}
				NBString_concat(dst,  " total: ");
				NBString_concat(dst, str.str);
				opened = TRUE;
			}
		}
		NBString_release(&pre);
		NBString_release(&str);
	}
}

void NBDataPtrsStats_concatState(const STNBDataPtrsStatsState* obj, const STNBDataPtrsStatsCfg* cfg, const BOOL isLoadedRecord, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, STNBString* dst){
	if(dst != NULL){
		BOOL opened = FALSE;
		const char* preExtra = "       |- ";
		STNBString str, pre;
		NBString_initWithSz(&str, 0, 256, 0.10f);
		NBString_initWithSz(&pre, NBString_strLenBytes(prefixOthers) + NBString_strLenBytes(preExtra) + 1, 256, 0.10f);
		NBString_concat(&pre, prefixOthers);
		NBString_concat(&pre, preExtra);
		//buffers
		{
			NBString_empty(&str);
			NBDataPtrsStats_concatBuffers(&obj->buffs, cfg, isLoadedRecord, ignoreEmpties, &str);
			if(str.length > 0){
				if(opened){
					NBString_concat(dst, "\n");
					NBString_concat(dst, prefixOthers);
				} else {
					NBString_concat(dst, prefixFirst);
				}
				NBString_concat(dst,  "  buffs:  ");
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

void NBDataPtrsStats_concatBuffers(const STNBDataPtrsStatsBuffers* obj, const STNBDataPtrsStatsCfg* cfg, const BOOL isLoadedRecord, const BOOL ignoreEmpties, STNBString* dst){
	if(dst != NULL && cfg != NULL && cfg->statsLevel > ENNBLogLevel_None){
		BOOL opened = FALSE;
		//current
		if(cfg->statsLevel >= ENNBLogLevel_Info && (!ignoreEmpties || obj->cur.bytesCount > 0)){
			if(opened) NBString_concat(dst, ", ");
			NBDataPtrsStats_concatFormatedBytes_(obj->cur.bytesCount, dst);
			if(obj->cur.count > 0){
				NBString_concat(dst, "/");
				NBString_concatUI32(dst, obj->cur.count);
			}
			opened = TRUE;
		}
		//min/max
		if(
		   (cfg->statsLevel >= ENNBLogLevel_Info && (!ignoreEmpties || obj->min.bytesCount > 0 || obj->max.bytesCount > 0))
		   || (cfg->statsLevel >= ENNBLogLevel_Warning && obj->min.bytesCount == 0)
		   ){
			if(opened) NBString_concat(dst, ", ");
			NBString_concat(dst, "rng(");
			NBDataPtrsStats_concatFormatedBytes_(obj->min.bytesCount, dst);
			if(obj->min.count > 0){
				NBString_concat(dst, "/");
				NBString_concatUI32(dst, obj->min.count);
			}
			NBString_concat(dst, " min, ");
			NBDataPtrsStats_concatFormatedBytes_(obj->max.bytesCount, dst);
			if(obj->min.count > 0){
				NBString_concat(dst, "/");
				NBString_concatUI32(dst, obj->max.count);
			}
			NBString_concat(dst, " max)");
			opened = TRUE;
		}
		//alloc
		if(cfg->statsLevel >= ENNBLogLevel_Warning && (!ignoreEmpties || obj->alloc.bytesCount > 0)){
			if(opened) NBString_concat(dst, ", ");
			NBString_concat(dst, "alloc(");
			NBDataPtrsStats_concatFormatedBytes_(obj->alloc.bytesCount, dst);
			if(obj->alloc.count > 0){
				NBString_concat(dst, "/");
				NBString_concatUI32(dst, obj->alloc.count);
			}
			NBString_concat(dst, ")");
			opened = TRUE;
		}
		//taken
		if((cfg->statsLevel >= ENNBLogLevel_Info && (!ignoreEmpties || obj->taken.bytesCount > 0))){
			if(opened) NBString_concat(dst, ", ");
			NBString_concat(dst, "taken(");
			NBDataPtrsStats_concatFormatedBytes_(obj->taken.bytesCount, dst);
			if(obj->taken.count > 0){
				NBString_concat(dst, "/");
				NBString_concatUI32(dst, obj->taken.count);
			}
			NBString_concat(dst, ")");
			opened = TRUE;
		}
		//given
		if((cfg->statsLevel >= ENNBLogLevel_Info && (!ignoreEmpties || obj->given.bytesCount > 0))){
			if(opened) NBString_concat(dst, ", ");
			NBString_concat(dst, "given(");
			NBDataPtrsStats_concatFormatedBytes_(obj->given.bytesCount, dst);
			if(obj->given.count > 0){
				NBString_concat(dst, "/");
				NBString_concatUI32(dst, obj->given.count);
			}
			NBString_concat(dst, ")");
			opened = TRUE;
		}
		//freed
		if(cfg->statsLevel >= ENNBLogLevel_Warning && (!ignoreEmpties || obj->freed.bytesCount > 0)){
			if(opened) NBString_concat(dst, ", ");
			NBString_concat(dst, "freed(");
			NBDataPtrsStats_concatFormatedBytes_(obj->freed.bytesCount, dst);
			if(obj->freed.count > 0){
				NBString_concat(dst, "/");
				NBString_concatUI32(dst, obj->freed.count);
			}
			NBString_concat(dst, ")");
			opened = TRUE;
		}
		//stacks actions
		if(
		   (cfg->statsLevel >= ENNBLogLevel_Info && (!ignoreEmpties || (obj->stacks.resizesCount > 0 || obj->stacks.swapsCount > 0)))
		   || (cfg->statsLevel >= ENNBLogLevel_Warning && (!ignoreEmpties || (obj->stacks.resizesCount > 0))) 
		   ){
			BOOL opened2 = FALSE;
			if(opened) NBString_concat(dst, ", ");
			NBString_concat(dst, "stcks(");
			if(obj->stacks.resizesCount > 0){
				if(opened2) NBString_concat(dst, ", ");
				NBString_concatUI32(dst, obj->stacks.resizesCount);
				NBString_concat(dst, " rszs");
				opened2 = TRUE;
			}
			if(obj->stacks.swapsCount > 0){
				if(opened2) NBString_concat(dst, ", ");
				NBString_concatUI32(dst, obj->stacks.swapsCount);
				NBString_concat(dst, " swps");
				opened2 = TRUE;
			}
			NBString_concat(dst, ")");
			opened = TRUE;
		}
	}
}

//Buffers

void NBDataPtrsStats_buffsUpdated(STNBDataPtrsStatsRef ref, const STNBDataPtrsStatsUpd* upd){
	STNBDataPtrsStatsOpq* opq = (STNBDataPtrsStatsOpq*)ref.opaque;
	NBASSERT(NBDataPtrsStats_isClass(ref))
	if(upd != NULL){
		if(upd->alloc.count != 0 || upd->freed.count != 0 || upd->given.count != 0 || upd->taken.count != 0){
			NBObject_lock(opq);
            UI32 curCountMax = 0;
            UI64 curBytesCountMax = 0;
			//loaded
			{
				opq->data.loaded.buffs.cur.count += upd->alloc.count + upd->taken.count;
				opq->data.loaded.buffs.cur.bytesCount += upd->alloc.bytes + upd->taken.bytes;
                curCountMax = opq->data.loaded.buffs.cur.count;
                curBytesCountMax = opq->data.loaded.buffs.cur.bytesCount;
				NBASSERT(opq->data.loaded.buffs.cur.count >= (upd->freed.count + upd->given.count))
				NBASSERT(opq->data.loaded.buffs.cur.bytesCount >= (upd->freed.bytes + upd->given.bytes))
				opq->data.loaded.buffs.cur.count -= upd->freed.count + upd->given.count;
				opq->data.loaded.buffs.cur.bytesCount -= upd->freed.bytes + upd->given.bytes;
				// 
				if(opq->data.total.updCalls == 0){
					opq->data.loaded.buffs.min.count = opq->data.loaded.buffs.cur.count;
					opq->data.loaded.buffs.min.bytesCount = opq->data.loaded.buffs.cur.bytesCount;
					//
					opq->data.loaded.buffs.max.count = curCountMax;
					opq->data.loaded.buffs.max.bytesCount = opq->data.loaded.buffs.cur.bytesCount;
				} else {
					if(opq->data.loaded.buffs.min.count > opq->data.loaded.buffs.cur.count){
						opq->data.loaded.buffs.min.count = opq->data.loaded.buffs.cur.count;
					}
					if(opq->data.loaded.buffs.max.count < curCountMax){
						opq->data.loaded.buffs.max.count = curCountMax;
					}
					if(opq->data.loaded.buffs.min.bytesCount > opq->data.loaded.buffs.cur.bytesCount){
						opq->data.loaded.buffs.min.bytesCount = opq->data.loaded.buffs.cur.bytesCount;
					}
					if(opq->data.loaded.buffs.max.bytesCount < opq->data.loaded.buffs.cur.bytesCount){
						opq->data.loaded.buffs.max.bytesCount = opq->data.loaded.buffs.cur.bytesCount;
					}
				}
			}
			//accum
			{
				opq->data.accum.buffs.alloc.count += upd->alloc.count;
				opq->data.accum.buffs.alloc.bytesCount += upd->alloc.bytes;
				//
				opq->data.accum.buffs.freed.count += upd->freed.count;
				opq->data.accum.buffs.freed.bytesCount += upd->freed.bytes;
				//
				opq->data.accum.buffs.given.count += upd->given.count;
				opq->data.accum.buffs.given.bytesCount += upd->given.bytes;
				//
				opq->data.accum.buffs.taken.count += upd->taken.count;
				opq->data.accum.buffs.taken.bytesCount += upd->taken.bytes;
				//
				opq->data.accum.buffs.stacks.resizesCount += upd->stackResize;
				opq->data.accum.buffs.stacks.swapsCount += upd->stacksSwaps;
				//
				if(opq->data.accum.updCalls == 0){
					opq->data.accum.buffs.min.count = opq->data.loaded.buffs.cur.count;
					opq->data.accum.buffs.min.bytesCount = opq->data.loaded.buffs.cur.bytesCount;
					//
					opq->data.accum.buffs.max.count = curCountMax;
					opq->data.accum.buffs.max.bytesCount = opq->data.loaded.buffs.cur.bytesCount;
				} else {
					if(opq->data.accum.buffs.min.count > opq->data.loaded.buffs.cur.count){
						opq->data.accum.buffs.min.count = opq->data.loaded.buffs.cur.count;
					}
					if(opq->data.accum.buffs.max.count < curCountMax){
						opq->data.accum.buffs.max.count = curCountMax;
					}
					if(opq->data.accum.buffs.min.bytesCount > opq->data.loaded.buffs.cur.bytesCount){
						opq->data.accum.buffs.min.bytesCount = opq->data.loaded.buffs.cur.bytesCount;
					}
					if(opq->data.accum.buffs.max.bytesCount < opq->data.loaded.buffs.cur.bytesCount){
						opq->data.accum.buffs.max.bytesCount = opq->data.loaded.buffs.cur.bytesCount;
					}
				}
			}
			//total
			{
				opq->data.total.buffs.alloc.count += upd->alloc.count;
				opq->data.total.buffs.alloc.bytesCount += upd->alloc.bytes;
				//
				opq->data.total.buffs.freed.count += upd->freed.count;
				opq->data.total.buffs.freed.bytesCount += upd->freed.bytes;
				//
				opq->data.total.buffs.given.count += upd->given.count;
				opq->data.total.buffs.given.bytesCount += upd->given.bytes;
				//
				opq->data.total.buffs.taken.count += upd->taken.count;
				opq->data.total.buffs.taken.bytesCount += upd->taken.bytes;
				//
				opq->data.total.buffs.stacks.resizesCount += upd->stackResize;
				opq->data.total.buffs.stacks.swapsCount += upd->stacksSwaps;
				//
				if(opq->data.total.updCalls == 0){
					opq->data.total.buffs.min.count = opq->data.loaded.buffs.cur.count;
					opq->data.total.buffs.min.bytesCount = opq->data.loaded.buffs.cur.bytesCount;
					//
					opq->data.total.buffs.max.count = curCountMax;
					opq->data.total.buffs.max.bytesCount = opq->data.loaded.buffs.cur.bytesCount;
				} else {
					if(opq->data.total.buffs.min.count > opq->data.loaded.buffs.cur.count){
						opq->data.total.buffs.min.count = opq->data.loaded.buffs.cur.count;
					}
					if(opq->data.total.buffs.max.count < curCountMax){
						opq->data.total.buffs.max.count = curCountMax;
					}
					if(opq->data.total.buffs.min.bytesCount > opq->data.loaded.buffs.cur.bytesCount){
						opq->data.total.buffs.min.bytesCount = opq->data.loaded.buffs.cur.bytesCount;
					}
					if(opq->data.total.buffs.max.bytesCount < opq->data.loaded.buffs.cur.bytesCount){
						opq->data.total.buffs.max.bytesCount = opq->data.loaded.buffs.cur.bytesCount;
					}
				}
			}
			//updCalls
			{
				//accum
				opq->data.accum.updCalls++;
				//total
				opq->data.total.updCalls++;
			}
			NBObject_unlock(opq);
		}
	}
}
