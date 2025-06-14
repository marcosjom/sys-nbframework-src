//
//  NBDataBuffsPool.h
//  thinstream
//
//  Created by Marcos Ortega on 9/3/19.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBDataBuffsPool.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBMngrStructMaps.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBThreadCond.h"
#include "nb/core/NBDataBuff.h"
#include "nb/core/NBArray.h"

//CfgBuffers

STNBStructMapsRec STNBDataBuffsPoolCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBDataBuffsPoolCfg_getSharedStructMap(void){
	NBMngrStructMaps_lock(&STNBDataBuffsPoolCfg_sharedStructMap);
	if(STNBDataBuffsPoolCfg_sharedStructMap.map == NULL){
		STNBDataBuffsPoolCfg s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBDataBuffsPoolCfg);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addStrPtrM(map, s, minAlloc);		//string representing a number-bytes and oprional-sufix, like "1", "1K", "1kb", "1M", "1G", "1Tb".
		NBStructMap_addStrPtrM(map, s, maxKeep);		//string representing a number-bytes and oprional-sufix, like "1", "1K", "1kb", "1M", "1G", "1Tb".
		NBStructMap_addStrPtrM(map, s, perBuffer);		//string representing a number-bytes and oprional-sufix, like "1", "1K", "1kb", "1M", "1G", "1Tb".
		//Loaded in runtime
		NBStructMap_addUIntM(map, s, minAllocBytes);	//if zero, will be interpreted from the string version.
		NBStructMap_addUIntM(map, s, maxKeepBytes);		//if zero, will be interpreted from the string version.
		NBStructMap_addUIntM(map, s, perBufferBytes);	//if zero, will be interpreted from the string version.
		//
		STNBDataBuffsPoolCfg_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&STNBDataBuffsPoolCfg_sharedStructMap);
	return STNBDataBuffsPoolCfg_sharedStructMap.map;
}

//--------------------
//- NBDataBuffsPool
//--------------------

typedef struct STNBDataBuffsPoolOpq_ {
	STNBObject				prnt;		//parent
	STNBDataBuffsPoolCfg	cfg;
	//buffers
	struct {
		STNBArray			arr;		//STNBDataBuffRef
	} buffs;
	//stats
	struct {
		STNBDataBuffsStatsRef provider;
	} stats;
} STNBDataBuffsPoolOpq;

NB_OBJREF_BODY(NBDataBuffsPool, STNBDataBuffsPoolOpq, NBObject)

//

void NBDataBuffsPool_initZeroed(STNBObject* obj){
	STNBDataBuffsPoolOpq* opq = (STNBDataBuffsPoolOpq*)obj;
	NBArray_init(&opq->buffs.arr, sizeof(STNBDataBuffRef), NULL);
}

void NBDataBuffsPool_uninitLocked(STNBObject* obj){
	STNBDataBuffsPoolOpq* opq = (STNBDataBuffsPoolOpq*)obj;
	STNBDataBuffsStatsUpd upd;
	NBMemory_setZeroSt(upd, STNBDataBuffsStatsUpd);
	//buffs
	{
		SI32 i; for(i = 0; i < opq->buffs.arr.use; i++){
			STNBDataBuffRef buff = NBArray_itmValueAtIndex(&opq->buffs.arr, STNBDataBuffRef, i);
			const STNBDataBuffState bState = NBDataBuff_getState(buff); 
			NBASSERT(bState.readersCount == 0)
			NBDataBuff_release(&buff);
			//
			upd.freed.count++;
			upd.freed.bytes += bState.buffSz; 
		}
		NBArray_empty(&opq->buffs.arr);
		NBArray_release(&opq->buffs.arr);
	}
	//stats
	if(NBDataBuffsStats_isSet(opq->stats.provider)){
		NBDataBuffsStats_buffsUpdated(opq->stats.provider, &upd);
		NBDataBuffsStats_release(&opq->stats.provider);
	}
	//Cfg
	{
		NBStruct_stRelease(NBDataBuffsPoolCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
	}
}

//

BOOL NBDataBuffsPool_setStatsProvider(STNBDataBuffsPoolRef ref, STNBDataBuffsStatsRef provider){
	BOOL r = FALSE;
	STNBDataBuffsPoolOpq* opq = (STNBDataBuffsPoolOpq*)ref.opaque;
	NBASSERT(NBDataBuffsPool_isClass(ref))
	NBObject_lock(opq);
	if(TRUE){ //ToDo validate activity
		NBDataBuffsStats_set(&opq->stats.provider, &provider);
		r = TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBDataBuffsPool_setCfg(STNBDataBuffsPoolRef ref, const STNBDataBuffsPoolCfg* cfg){
	BOOL r = FALSE;
	STNBDataBuffsPoolOpq* opq = (STNBDataBuffsPoolOpq*)ref.opaque;
	NBASSERT(NBDataBuffsPool_isClass(ref))
	NBObject_lock(opq);
	if(TRUE){ //ToDo validate activity
		STNBDataBuffsPoolCfg buffs2; //validated version
		NBMemory_setZeroSt(buffs2, STNBDataBuffsPoolCfg);
		r = TRUE;
		//Validate
		if(cfg != NULL){
			buffs2 = *cfg;
			//buffers
			if(r){
				if(r && !NBString_strIsEmpty(buffs2.minAlloc)){
					const SI64 bytes = NBString_strToBytes(buffs2.minAlloc);
					//PRINTF_INFO("NBDataBuffsPool, '%s' parsed to %lld bytes\n", buffs2.minAlloc, bytes);
					if(bytes <= 0){
						r = FALSE;
					} else {
						buffs2.minAllocBytes = (UI64)bytes;
					}
				}
				if(r && !NBString_strIsEmpty(buffs2.maxKeep)){
					const SI64 bytes = NBString_strToBytes(buffs2.maxKeep);
					//PRINTF_INFO("NBDataBuffsPool, '%s' parsed to %lld bytes\n", buffs2.maxKeep, bytes);
					if(bytes <= 0){
						r = FALSE;
					} else {
						buffs2.maxKeepBytes = bytes;
					}
				}
				if(r && !NBString_strIsEmpty(buffs2.perBuffer)){
					const SI64 bytes = NBString_strToBytes(buffs2.perBuffer);
					//PRINTF_INFO("NBDataBuffsPool, '%s' parsed to %lld bytes\n", buffs2.perBuffer, bytes);
					if(bytes <= 0){
						r = FALSE;
					} else if(bytes > 0xFFFFFFFFU){ //max: 32-bits value
						r = FALSE;
					} else {
						buffs2.perBufferBytes = (UI32)bytes;
					}
				}
				//min <= max
				if(buffs2.maxKeepBytes < buffs2.minAllocBytes){
					r = FALSE;
				}
				//perBufferBytes > 0
				if(buffs2.perBufferBytes <= 0){
					r = FALSE;
				}
			}
		}
		//Apply
		if(r){
			NBStruct_stRelease(NBDataBuffsPoolCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
			if(cfg != NULL){
				NBStruct_stClone(NBDataBuffsPoolCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
				//Apply parsed values
				opq->cfg.minAllocBytes	= buffs2.minAllocBytes;
				opq->cfg.maxKeepBytes	= buffs2.maxKeepBytes; 
				opq->cfg.perBufferBytes	= buffs2.perBufferBytes;
			}
		}
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBDataBuffsPool_prepare(STNBDataBuffsPoolRef ref){ //create buffers
	BOOL r = FALSE;
	STNBDataBuffsPoolOpq* opq = (STNBDataBuffsPoolOpq*)ref.opaque;
	NBASSERT(NBDataBuffsPool_isClass(ref))
	NBObject_lock(opq);
	if(TRUE){ //ToDo validate activity
		r = TRUE;
		//Sync buffers
		{
			UI64 curBytes = 0;
			STNBDataBuffsStatsUpd upd;
			NBMemory_setZeroSt(upd, STNBDataBuffsStatsUpd);
			//Count bytes
			{
				SI32 i; for(i = 0; i < opq->buffs.arr.use; i++){
					STNBDataBuffRef buff = NBArray_itmValueAtIndex(&opq->buffs.arr, STNBDataBuffRef, i);
					const STNBDataBuffState bState = NBDataBuff_getState(buff);
					NBASSERT(bState.readersCount == 0)
					curBytes += bState.buffSz;
				}
			}
			//Release buffers
			{
				SI32 i; for(i = (SI32)opq->buffs.arr.use - 1; i >= 0 && curBytes > opq->cfg.maxKeepBytes; i--){
					STNBDataBuffRef buff = NBArray_itmValueAtIndex(&opq->buffs.arr, STNBDataBuffRef, i);
					const STNBDataBuffState bState = NBDataBuff_getState(buff); 
					NBASSERT(bState.readersCount == 0)
					NBDataBuff_release(&buff);
					//
					upd.freed.count++;
					upd.freed.bytes += bState.buffSz;
					NBASSERT(bState.buffSz <= curBytes)
					curBytes -= bState.buffSz;
					//
					NBArray_removeItemAtIndex(&opq->buffs.arr, i);
				}
			}
			//create buffers
			if(opq->cfg.perBufferBytes > 0 && opq->cfg.minAllocBytes > 0){
				const UI32 buffSz = opq->cfg.perBufferBytes;
				while(curBytes < opq->cfg.minAllocBytes){
					STNBDataBuffRef buff = NBDataBuff_alloc(NULL);
					NBDataBuff_createBuffer(buff, buffSz);
					upd.alloc.count++;
					upd.alloc.bytes += buffSz;
					curBytes += buffSz;
					NBArray_addValue(&opq->buffs.arr, buff);
				}
			}
			//stats
			if(NBDataBuffsStats_isSet(opq->stats.provider)){
				NBDataBuffsStats_buffsUpdated(opq->stats.provider, &upd);
			} 
		}
	}
	NBObject_unlock(opq);
	return r;
}

//Actions

//send the buffer to be managed outside this factory

BOOL NBDataBuffsPool_addAvailableBuffer(STNBDataBuffsPoolRef ref, STNBDataBuffRef buff){
	BOOL r = FALSE;
	STNBDataBuffsPoolOpq* opq = (STNBDataBuffsPoolOpq*)ref.opaque;
	NBASSERT(NBDataBuffsPool_isClass(ref))
	NBObject_lock(opq);
	{
		NBArray_addValue(&opq->buffs.arr, buff);
		r = TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

//request a buffer managed from outside this factory

STNBDataBuffRef NBDataBuffsPool_getAvailableBuffer(STNBDataBuffsPoolRef ref, BOOL* dstForbidCreateOwn){
	STNBDataBuffRef r;
	STNBDataBuffsPoolOpq* opq = (STNBDataBuffsPoolOpq*)ref.opaque;
	NBASSERT(NBDataBuffsPool_isClass(ref))
	NBObject_lock(opq);
	if(opq->buffs.arr.use <= 0){
		NBDataBuff_null(&r);
	} else {
		r = NBArray_itmValueAtIndex(&opq->buffs.arr, STNBDataBuffRef, opq->buffs.arr.use - 1);
		NBArray_removeItemAtIndex(&opq->buffs.arr, opq->buffs.arr.use - 1);
	}
	NBObject_unlock(opq);
	return r;
}
