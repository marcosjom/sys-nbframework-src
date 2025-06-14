
#include "nb/NBFrameworkPch.h"
#include "nb/core/NBDataBuffsStats.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBThreadMutex.h"

//Opaque state

typedef struct STNBDataBuffsStatsOpq_ {
	STNBObject				prnt;	//parent
	STNBDataBuffsStatsData	data;
} STNBDataBuffsStatsOpq;

NB_OBJREF_BODY(NBDataBuffsStats, STNBDataBuffsStatsOpq, NBObject)

//init

void NBDataBuffsStats_initZeroed(STNBObject* obj){
	//STNBDataBuffsStatsOpq* opq = (STNBDataBuffsStatsOpq*)obj;
}

void NBDataBuffsStats_uninitLocked(STNBObject* obj){
	//STNBDataBuffsStatsOpq* opq = (STNBDataBuffsStatsOpq*)obj;
}

//
	
STNBDataBuffsStatsData NBDataBuffsStats_getData(STNBDataBuffsStatsRef ref, const BOOL resetAccum){
	STNBDataBuffsStatsData r;
	STNBDataBuffsStatsOpq* opq = (STNBDataBuffsStatsOpq*)ref.opaque;
	NBASSERT(NBDataBuffsStats_isClass(ref))
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

void NBDataBuffsStats_concatFormatedBytes_(const UI64 bytesCount, STNBString* dst){
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

void NBDataBuffsStats_concat(STNBDataBuffsStatsRef ref, const ENNBLogLevel logLvl, const BOOL loaded, const BOOL accumAdded, const BOOL accumRemoved, const BOOL totalAdded, const BOOL totalRemoved, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, STNBString* dst, const BOOL resetAccum){
	STNBDataBuffsStatsOpq* opq = (STNBDataBuffsStatsOpq*)ref.opaque;
	NBASSERT(NBDataBuffsStats_isClass(ref))
	NBObject_lock(opq);
	{
		NBDataBuffsStats_concatData(&opq->data, logLvl, loaded, accumAdded, accumRemoved, totalAdded, totalRemoved, prefixFirst, prefixOthers, ignoreEmpties, dst);
		//reset
		if(resetAccum){
			NBMemory_setZero(opq->data.accum);
		}
	}
	NBObject_unlock(opq);
}

void NBDataBuffsStats_concatData(const STNBDataBuffsStatsData* obj, const ENNBLogLevel logLvl, const BOOL loaded, const BOOL accumAdded, const BOOL accumRemoved, const BOOL totalAdded, const BOOL totalRemoved, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, STNBString* dst){
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
			NBDataBuffsStats_concatState(&obj->loaded, logLvl, TRUE, "", pre.str, ignoreEmpties, &str);
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
		if(accumAdded){
			NBString_empty(&str);
			NBDataBuffsStats_concatState(&obj->accum.added, logLvl, FALSE, "", pre.str, ignoreEmpties, &str);
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
		if(accumRemoved){
			NBString_empty(&str);
			NBDataBuffsStats_concatState(&obj->accum.removed, logLvl, FALSE, "", pre.str, ignoreEmpties, &str);
			if(str.length > 0){
				if(opened){
					NBString_concat(dst, "\n");
					NBString_concat(dst, prefixOthers);
				} else {
					NBString_concat(dst, prefixFirst);
				}
				NBString_concat(dst,  "accum(-):  ");
				NBString_concat(dst, str.str);
				opened = TRUE;
			}
		}
		if(totalAdded){
			NBString_empty(&str);
			NBDataBuffsStats_concatState(&obj->total.added, logLvl, FALSE, "", pre.str, ignoreEmpties, &str);
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
		if(totalRemoved){
			NBString_empty(&str);
			NBDataBuffsStats_concatState(&obj->total.removed, logLvl, FALSE, "", pre.str, ignoreEmpties, &str);
			if(str.length > 0){
				if(opened){
					NBString_concat(dst, "\n");
					NBString_concat(dst, prefixOthers);
				} else {
					NBString_concat(dst, prefixFirst);
				}
				NBString_concat(dst,  "total(-):  ");
				NBString_concat(dst, str.str);
				opened = TRUE;
			}
		}
		NBString_release(&pre);
		NBString_release(&str);
	}
}

void NBDataBuffsStats_concatState(const STNBDataBuffsStatsState* obj, const ENNBLogLevel logLvl, const BOOL isLoadedRecord, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, STNBString* dst){
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
			NBDataBuffsStats_concatBuffers(&obj->buffs, logLvl, isLoadedRecord, ignoreEmpties, &str);
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

void NBDataBuffsStats_concatBuffers(const STNBDataBuffsStatsBuffers* obj, const ENNBLogLevel logLvl, const BOOL isLoadedRecord, const BOOL ignoreEmpties, STNBString* dst){
	if(dst != NULL){
		BOOL opened = FALSE;
		//count
		if(!ignoreEmpties || obj->bytesCount > 0){
			if(opened) NBString_concat(dst, ", ");
			NBDataBuffsStats_concatFormatedBytes_(obj->bytesCount, dst);
			if(obj->count > 0){
				NBString_concat(dst, "/");
				NBString_concatUI32(dst, obj->count);
			}
			if(isLoadedRecord){
				//wired
				if(obj->wired.bytesCount > 0){
					NBString_concat(dst, ", ");
					NBDataBuffsStats_concatFormatedBytes_(obj->wired.bytesCount, dst);
					if(obj->wired.count > 0){
						NBString_concat(dst, "/");
						NBString_concatUI32(dst, obj->wired.count);
					}
					NBString_concat(dst, " wired");
					if(obj->bytesCount > 0){
						NBString_concat(dst, "~");
						NBString_concatUI64(dst, (obj->wired.bytesCount * 100) / obj->bytesCount);
						NBString_concat(dst, "%");
					}
				}
				//free
				{
					if(obj->wired.bytesCount <= obj->bytesCount){
						const UI32 count =  obj->count - obj->wired.count;
						const UI64 bytesCount = obj->bytesCount - obj->wired.bytesCount;
						NBString_concat(dst, ", ");
						NBDataBuffsStats_concatFormatedBytes_(bytesCount, dst);
						if(count > 0){
							NBString_concat(dst, "/");
							NBString_concatUI32(dst, count);
						}
						NBString_concat(dst, " free");
						if(obj->bytesCount > 0){
							NBString_concat(dst, "~");
							NBString_concatUI64(dst, (bytesCount * 100) / obj->bytesCount);
							NBString_concat(dst, "%");
						}
					}
				}
			}
			opened = TRUE;
		}
		//wired
		if(!isLoadedRecord && (!ignoreEmpties || obj->wired.count > 0)){
			if(opened) NBString_concat(dst, ", ");
			NBString_concat(dst, "wired(");
			if(obj->wired.bytesCount > 0){
				NBDataBuffsStats_concatFormatedBytes_(obj->wired.bytesCount, dst);
				NBString_concat(dst, "/");
			}
			NBString_concatUI32(dst, obj->wired.count);
			NBString_concat(dst, ")");
			opened = TRUE;
		}
	}
}

//Buffers

void NBDataBuffsStats_buffsUpdated(STNBDataBuffsStatsRef ref, const STNBDataBuffsStatsUpd* upd){
	STNBDataBuffsStatsOpq* opq = (STNBDataBuffsStatsOpq*)ref.opaque;
	NBASSERT(NBDataBuffsStats_isClass(ref))
	if(upd != NULL){
		if(upd->alloc.count != 0 || upd->wired.count != 0 || upd->unwired.count != 0 || upd->freed.count != 0){
			NBObject_lock(opq);
			//+
			{
				//loaded
				opq->data.loaded.buffs.count += upd->alloc.count;
				opq->data.loaded.buffs.bytesCount += upd->alloc.bytes;
				opq->data.loaded.buffs.wired.count += upd->wired.count;
				opq->data.loaded.buffs.wired.bytesCount += upd->wired.bytes;
				//accum
				opq->data.accum.added.buffs.count += upd->alloc.count;
				opq->data.accum.added.buffs.bytesCount += upd->alloc.bytes;
				opq->data.accum.added.buffs.wired.count += upd->wired.count;
				opq->data.accum.added.buffs.wired.bytesCount += upd->wired.bytes;
				//accum
				opq->data.total.added.buffs.count += upd->alloc.count;
				opq->data.total.added.buffs.bytesCount += upd->alloc.bytes;
				opq->data.total.added.buffs.wired.count += upd->wired.count;
				opq->data.total.added.buffs.wired.bytesCount += upd->wired.bytes;
			}
			//-
			{
				//loaded
				NBASSERT(opq->data.loaded.buffs.count >= upd->freed.count)
				NBASSERT(opq->data.loaded.buffs.bytesCount >= upd->freed.bytes)
				NBASSERT(opq->data.loaded.buffs.wired.count >= upd->unwired.count)
				NBASSERT(opq->data.loaded.buffs.wired.bytesCount >= upd->unwired.bytes)
				opq->data.loaded.buffs.count -= upd->freed.count;
				opq->data.loaded.buffs.bytesCount -= upd->freed.bytes;
				opq->data.loaded.buffs.wired.count -= upd->unwired.count;
				opq->data.loaded.buffs.wired.bytesCount -= upd->unwired.bytes;
				NBASSERT(opq->data.loaded.buffs.wired.count <= opq->data.loaded.buffs.count)
				NBASSERT(opq->data.loaded.buffs.wired.bytesCount <= opq->data.loaded.buffs.bytesCount)
				//accum
				opq->data.accum.removed.buffs.count += upd->freed.count;
				opq->data.accum.removed.buffs.bytesCount += upd->freed.bytes;
				opq->data.accum.removed.buffs.wired.count += upd->unwired.count;
				opq->data.accum.removed.buffs.wired.bytesCount += upd->unwired.bytes;
				//accum
				opq->data.total.removed.buffs.count += upd->freed.count;
				opq->data.total.removed.buffs.bytesCount += upd->freed.bytes;
				opq->data.total.removed.buffs.wired.count += upd->unwired.count;
				opq->data.total.removed.buffs.wired.bytesCount += upd->unwired.bytes;
			}
			//updCalls
			{
				//accum
				opq->data.accum.added.updCalls++;
				//total
				opq->data.total.added.updCalls++;
			}
			NBObject_unlock(opq);
		}
	}
}
