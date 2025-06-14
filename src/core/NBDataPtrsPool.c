//
//  NBDataPtrsPool.h
//  thinstream
//
//  Created by Marcos Ortega on 9/3/19.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBDataPtrsPool.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBMngrStructMaps.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBThreadMutexRW.h"
#include "nb/core/NBThreadCond.h"
#include "nb/core/NBDataPtr.h"
#include "nb/core/NBArray.h"

//CfgBuffers

STNBStructMapsRec STNBDataPtrsPoolCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBDataPtrsPoolCfg_getSharedStructMap(void){
	NBMngrStructMaps_lock(&STNBDataPtrsPoolCfg_sharedStructMap);
	if(STNBDataPtrsPoolCfg_sharedStructMap.map == NULL){
		STNBDataPtrsPoolCfg s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBDataPtrsPoolCfg);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addStrPtrM(map, s, maxKeep);	//string representing a number-bytes and oprional-sufix, like "1", "1K", "1kb", "1M", "1G", "1Tb".
		NBStructMap_addUIntM(map, s, maxKeepBytes);	//if zero, will be interpreted from the string version.
		NBStructMap_addBoolM(map, s, atomicStats);	//apply stats to provider after each action (do not wait for explicit statsFlush)
		//
		STNBDataPtrsPoolCfg_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&STNBDataPtrsPoolCfg_sharedStructMap);
	return STNBDataPtrsPoolCfg_sharedStructMap.map;
}

struct STNBDataPtrsPoolOpq_;

//-------------------
//- NBDataPtrsPoolPtr
//-------------------

typedef struct STNBDataPtrsPoolPtr_ {
	void*	ptr;
	UI32	size;
} STNBDataPtrsPoolPtr;

//---------------------
//- NBDataPtrsPoolStack
//---------------------

typedef struct STNBDataPtrsPoolStack_ {
	STNBThreadMutexRW		mutex;		//independent mutex
	STNBDataPtrsPoolPtr*	avail;		//avail pointers
	UI32					availUse;	//avail ptrs use
	UI32					availSz;	//avail ptrs size
	STNBDataPtrsStatsUpd	upd;		//update
	UI32					bytesTotal;	//total bytes in buffs.avail
} STNBDataPtrsPoolStack;

void NBDataPtrsPoolStack_init(STNBDataPtrsPoolStack* obj);
void NBDataPtrsPoolStack_release(STNBDataPtrsPoolStack* obj);
void NBDataPtrsPoolStack_emptyStack(STNBDataPtrsPoolStack* obj);

//-------------------
//- NBDataPtrsPoolOpq
//-------------------

typedef struct STNBDataPtrsPoolOpq_ {
	STNBObject				prnt;		//parent
	STNBDataPtrsPoolCfg		cfg;
	//buffers
	struct {
		STNBDataPtrsPoolStack*	outgoing;	//ptrs to be given (independent lock from 'incoming')
		STNBDataPtrsPoolStack*	incoming;	//ptrs to receive (independent lock from 'outgoing')
	} buffs;
	//stats
	struct {
		STNBDataPtrsStatsRef provider;
	} stats;
} STNBDataPtrsPoolOpq;

NB_OBJREF_BODY(NBDataPtrsPool, STNBDataPtrsPoolOpq, NBObject)

//

void NBDataPtrsPoolStack_init(STNBDataPtrsPoolStack* obj){
	NBMemory_setZeroSt(*obj, STNBDataPtrsPoolStack);
	NBThreadMutexRW_init(&obj->mutex);
}
	
void NBDataPtrsPoolStack_release(STNBDataPtrsPoolStack* obj){
	NBThreadMutexRW_lockForWrite(&obj->mutex);
	NBASSERT(obj->availUse == 0)
	{
		if(obj->avail != NULL){
			UI32 i; for(i = 0; i < obj->availUse; i++){
				STNBDataPtrsPoolPtr* ptr = &obj->avail[i]; NBASSERT(ptr->ptr != NULL)
				if(ptr->ptr != NULL){
					obj->upd.freed.count++;
					obj->upd.freed.bytes += ptr->size;
					NBMemory_free(ptr->ptr);
					NBASSERT(obj->bytesTotal >= ptr->size)
					obj->bytesTotal -= ptr->size;
					ptr->ptr = NULL;
					ptr->size = 0;
				}
			}
			NBMemory_free(obj->avail);
			obj->avail = NULL;
		}
		obj->availUse = 0;
		obj->availSz = 0;
	}
	NBThreadMutexRW_unlockFromWrite(&obj->mutex);
	NBThreadMutexRW_release(&obj->mutex);
}

void NBDataPtrsPoolStack_emptyStack(STNBDataPtrsPoolStack* obj){
	NBThreadMutexRW_lockForWrite(&obj->mutex);
	{
		if(obj->avail != NULL){
			UI32 i; for(i = 0; i < obj->availUse; i++){
				STNBDataPtrsPoolPtr* ptr = &obj->avail[i]; NBASSERT(ptr->ptr != NULL)
				if(ptr->ptr != NULL){
					obj->upd.freed.count++;
					obj->upd.freed.bytes += ptr->size;
					NBMemory_free(ptr->ptr);
					NBASSERT(obj->bytesTotal >= ptr->size)
					obj->bytesTotal -= ptr->size;
					ptr->ptr = NULL;
					ptr->size = 0;
				}
			}
		}
		obj->availUse = 0;
	}
	NBThreadMutexRW_unlockFromWrite(&obj->mutex);
}

//

void NBDataPtrsPool_initZeroed(STNBObject* obj){
	STNBDataPtrsPoolOpq* opq = (STNBDataPtrsPoolOpq*)obj;
	//buffers
	{
		{
			opq->buffs.outgoing = NBMemory_allocType(STNBDataPtrsPoolStack);
			NBDataPtrsPoolStack_init(opq->buffs.outgoing);
		}
		{
			opq->buffs.incoming = NBMemory_allocType(STNBDataPtrsPoolStack);
			NBDataPtrsPoolStack_init(opq->buffs.incoming);
		}
	}
}

void NBDataPtrsPool_uninitLocked(STNBObject* obj){
	STNBDataPtrsPoolOpq* opq = (STNBDataPtrsPoolOpq*)obj;
	//buffs
	{
		if(opq->buffs.outgoing != NULL){
			NBDataPtrsPoolStack_emptyStack(opq->buffs.outgoing);
			NBASSERT(opq->buffs.outgoing->bytesTotal == 0)
			if(NBDataPtrsStats_isSet(opq->stats.provider)){
				NBDataPtrsStats_buffsUpdated(opq->stats.provider, &opq->buffs.outgoing->upd);
			}
			NBMemory_setZeroSt(opq->buffs.outgoing->upd, STNBDataPtrsStatsUpd);
			NBDataPtrsPoolStack_release(opq->buffs.outgoing);
			NBMemory_free(opq->buffs.outgoing);
			opq->buffs.outgoing = NULL;
		}
		if(opq->buffs.incoming != NULL){
			NBDataPtrsPoolStack_emptyStack(opq->buffs.incoming);
			NBASSERT(opq->buffs.incoming->bytesTotal == 0)
			if(NBDataPtrsStats_isSet(opq->stats.provider)){
				NBDataPtrsStats_buffsUpdated(opq->stats.provider, &opq->buffs.incoming->upd);
			}
			NBMemory_setZeroSt(opq->buffs.incoming->upd, STNBDataPtrsStatsUpd);
			NBDataPtrsPoolStack_release(opq->buffs.incoming);
			NBMemory_free(opq->buffs.incoming);
			opq->buffs.incoming = NULL;
		}
	}
	//stats
	{
		if(NBDataPtrsStats_isSet(opq->stats.provider)){
			NBDataPtrsStats_release(&opq->stats.provider);
			NBDataPtrsStats_null(&opq->stats.provider);
		}
	}
	//Cfg
	{
		NBStruct_stRelease(NBDataPtrsPoolCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
	}
}

//

void NBDataPtrsPool_flushStats(STNBDataPtrsPoolRef ref){
	STNBDataPtrsPoolOpq* opq = (STNBDataPtrsPoolOpq*)ref.opaque;
	NBASSERT(NBDataPtrsPool_isClass(ref))
	{
		STNBDataPtrsPoolStack* outgoing = opq->buffs.outgoing;
		STNBDataPtrsPoolStack* incoming = opq->buffs.incoming;
		//validate swapping in other thread that resulted in both pointers temporary at same address
		while(outgoing == incoming){
			incoming = (opq->buffs.outgoing != incoming ? opq->buffs.outgoing : opq->buffs.incoming);
		}
		//flush stats
		{
			NBThreadMutexRW_lockForWrite(&outgoing->mutex);
			{
				if(NBDataPtrsStats_isSet(opq->stats.provider)){
					NBDataPtrsStats_buffsUpdated(opq->stats.provider, &outgoing->upd);
				} 
				NBMemory_setZeroSt(outgoing->upd, STNBDataPtrsStatsUpd);
			}
			NBThreadMutexRW_unlockFromWrite(&outgoing->mutex);
		}
		//flush stats
		{
			NBThreadMutexRW_lockForWrite(&incoming->mutex);
			{
				if(NBDataPtrsStats_isSet(opq->stats.provider)){
					NBDataPtrsStats_buffsUpdated(opq->stats.provider, &incoming->upd);
				} 
				NBMemory_setZeroSt(incoming->upd, STNBDataPtrsStatsUpd);
			}
			NBThreadMutexRW_unlockFromWrite(&incoming->mutex);
		}
	}
}

BOOL NBDataPtrsPool_setStatsProvider(STNBDataPtrsPoolRef ref, STNBDataPtrsStatsRef provider){
	BOOL r = FALSE;
	STNBDataPtrsPoolOpq* opq = (STNBDataPtrsPoolOpq*)ref.opaque;
	NBASSERT(NBDataPtrsPool_isClass(ref))
	NBObject_lock(opq);
	if(TRUE){ //ToDo validate activity
		NBDataPtrsStats_set(&opq->stats.provider, &provider);
		r = TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBDataPtrsPool_setCfg(STNBDataPtrsPoolRef ref, const STNBDataPtrsPoolCfg* cfg){
	BOOL r = FALSE;
	STNBDataPtrsPoolOpq* opq = (STNBDataPtrsPoolOpq*)ref.opaque;
	NBASSERT(NBDataPtrsPool_isClass(ref))
	NBObject_lock(opq);
	if(TRUE){ //ToDo validate activity
		STNBDataPtrsPoolCfg buffs2; //validated version
		NBMemory_setZeroSt(buffs2, STNBDataPtrsPoolCfg);
		r = TRUE;
		//Validate
		if(cfg != NULL){
			buffs2 = *cfg;
			//buffers
			if(r){
				if(r && !NBString_strIsEmpty(buffs2.maxKeep)){
					const SI64 bytes = NBString_strToBytes(buffs2.maxKeep);
					//PRINTF_INFO("NBDataPtrsPool, '%s' parsed to %lld bytes\n", buffs2.maxKeep, bytes);
					if(bytes < 0){
						r = FALSE;
					} else {
						buffs2.maxKeepBytes = bytes;
					}
				}
			}
		}
		//Apply
		if(r){
			NBStruct_stRelease(NBDataPtrsPoolCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
			if(cfg != NULL){
				NBStruct_stClone(NBDataPtrsPoolCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
				//Apply parsed values
				opq->cfg.maxKeepBytes = buffs2.maxKeepBytes; 
			}
		}
	}
	NBObject_unlock(opq);
	return r;
}

//Actions

UI32 NBDataPtrsPool_allocPtrsAtStackLocked_(STNBDataPtrsPoolStack* stack, const UI32 bytesPerPtr, const UI32 ammPtrs){
	UI32 r = 0;
	NBASSERT(bytesPerPtr > 0 && ammPtrs > 0)
	const UI32 useStart = stack->availUse;
	//allocate array slots
	if((stack->availUse + ammPtrs) > stack->availSz){
		STNBDataPtrsPoolPtr* availN = NBMemory_allocTypes(STNBDataPtrsPoolPtr, stack->availUse + ammPtrs);
		if(stack->avail != NULL){
			if(stack->availUse > 0){
				NBMemory_copy(availN, stack->avail, sizeof(stack->avail[0]) * stack->availUse);
			}
			NBMemory_free(stack->avail);
		}
		stack->avail	= availN;
		stack->availSz	= stack->availUse + ammPtrs;
		stack->upd.stackResize++;
	}
	//allocate new ptrs
	{
		UI32 useAfterEnd = stack->availUse + ammPtrs; 
		while(stack->availUse < useAfterEnd){
			NBASSERT(stack->availUse < stack->availSz)
			void* p = NBMemory_alloc(bytesPerPtr);
			if(p == NULL){
				//system error
				break;
			} else {
				STNBDataPtrsPoolPtr* ptr = &stack->avail[stack->availUse++];
				ptr->ptr	= p;
				ptr->size	= bytesPerPtr;
				stack->bytesTotal += ptr->size;
				stack->upd.alloc.count++;
				stack->upd.alloc.bytes += ptr->size;
			}
		}
	}
	r = (stack->availUse - useStart);
	return r;
}
	
UI32 NBDataPtrsPool_allocPtrs(STNBDataPtrsPoolRef ref, const UI32 bytesPerPtr, const UI32 ammPtrs){
	UI32 r = 0;
	STNBDataPtrsPoolOpq* opq = (STNBDataPtrsPoolOpq*)ref.opaque;
	NBASSERT(NBDataPtrsPool_isClass(ref))
	if(bytesPerPtr > 0 && ammPtrs > 0){
		STNBDataPtrsPoolStack* outgoing = opq->buffs.outgoing;
		STNBDataPtrsPoolStack* incoming = opq->buffs.incoming;
		UI32 incomingStackSz = 0;
		//validate swapping in other thread that resulted in both pointers temporary at same address
		while(outgoing == incoming){
			incoming = (opq->buffs.outgoing != incoming ? opq->buffs.outgoing : opq->buffs.incoming);
		}
		//alloc
		{
			NBThreadMutexRW_lockForWrite(&incoming->mutex);
			//alloc
			{
				r = NBDataPtrsPool_allocPtrsAtStackLocked_(incoming, bytesPerPtr, ammPtrs);
			}
			//stats (if atomic)
			if(opq->cfg.atomicStats){
				if(NBDataPtrsStats_isSet(opq->stats.provider)){
					NBDataPtrsStats_buffsUpdated(opq->stats.provider, &incoming->upd);
				} 
				NBMemory_setZeroSt(incoming->upd, STNBDataPtrsStatsUpd);
			}
			incomingStackSz = incoming->availSz;
			NBThreadMutexRW_unlockFromWrite(&incoming->mutex);
		}
		//Grow outgoing stack to reduce chance of stackResizes
		{
			NBThreadMutexRW_lockForWrite(&outgoing->mutex);
			if(outgoing->availSz < incomingStackSz){
				STNBDataPtrsPoolPtr* availN = NBMemory_allocTypes(STNBDataPtrsPoolPtr, incomingStackSz);
				if(outgoing->avail != NULL){
					if(outgoing->availUse > 0){
						NBMemory_copy(availN, outgoing->avail, sizeof(outgoing->avail[0]) * outgoing->availUse);
					}
					NBMemory_free(outgoing->avail);
				}
				outgoing->avail		= availN;
				outgoing->availSz	= incomingStackSz;
				outgoing->upd.stackResize++;
				//stats (if atomic)
				if(opq->cfg.atomicStats){
					if(NBDataPtrsStats_isSet(opq->stats.provider)){
						NBDataPtrsStats_buffsUpdated(opq->stats.provider, &outgoing->upd);
					} 
					NBMemory_setZeroSt(outgoing->upd, STNBDataPtrsStatsUpd);
				}
			}
			NBThreadMutexRW_unlockFromWrite(&outgoing->mutex);
		}
	}
	return r;
}

BOOL NBDataPtrsPool_addPtrAtStackLocked_(const STNBDataPtrsPoolOpq* opq, STNBDataPtrsPoolStack* stack, void* ptr, const UI32 size){
	BOOL r = FALSE;
	if(ptr != NULL && size > 0){
		if((stack->bytesTotal + size) > opq->cfg.maxKeepBytes){
			//PRINTF_INFO("NBDataPtrsPool, freeing ptr (%d bytes) currently %d bytes in pool.\n", size, opq->buffs.bytesTotal);
			stack->upd.freed.count++;
			stack->upd.freed.bytes += size;
			//release inmediatly
			NBMemory_free(ptr);
			r = TRUE;
		} else {
			//add for reutilization
			if(stack->availUse == stack->availSz){
				STNBDataPtrsPoolPtr* availN;
				stack->availSz += 32;
				availN = NBMemory_allocTypes(STNBDataPtrsPoolPtr, stack->availSz);
				if(stack->avail != NULL){
					if(stack->availUse > 0){
						NBMemory_copy(availN, stack->avail, sizeof(stack->avail[0]) * stack->availUse);
					}
					NBMemory_free(stack->avail);
				}
				stack->avail	= availN;
				stack->upd.stackResize++;
			}
			//add
			{
				STNBDataPtrsPoolPtr* cur = &stack->avail[stack->availUse++];
				cur->ptr	= ptr;
				cur->size	= size;
				//upd.alloc.count++;
				//upd.alloc.bytes += ptr->size;
				stack->bytesTotal += cur->size;
				//PRINTF_INFO("NBDataPtrsPool, queued ptr (%d bytes) currently %d bytes in pool.\n", size, opq->buffs.bytesTotal);
				stack->upd.taken.count++;
				stack->upd.taken.bytes += size;
			}
			//
			r = TRUE;
		}
	}
	NBASSERT(r)
	return r;
}

BOOL NBDataPtrsPool_addDataPtrs(STNBDataPtrsPoolRef ref, STNBDataPtr* ptrs, const UI32 ptrsSz){
	BOOL r = FALSE;
	STNBDataPtrsPoolOpq* opq = (STNBDataPtrsPoolOpq*)ref.opaque;
	NBASSERT(NBDataPtrsPool_isClass(ref))
	if(ptrs != NULL && ptrsSz > 0){
		STNBDataPtrsPoolStack* stack = opq->buffs.incoming; 
		NBThreadMutexRW_lockForWrite(&stack->mutex);
		{
			//add
			{
				STNBDataPtr* ptr = ptrs;
				const STNBDataPtr* ptrAfterEnd = ptrs + ptrsSz;
				while(ptr < ptrAfterEnd){
					if(ptr->def.alloc.ptr != NULL && ptr->def.alloc.size > 0){
						if(NBDataPtrsPool_addPtrAtStackLocked_(opq, stack, ptr->def.alloc.ptr, ptr->def.alloc.size)){
							NBDataPtr_resignToBuffer(ptr);
							r = TRUE;
						} else {
							NBASSERT(FALSE)
						}
					}
					//next
					ptr++;
				}
			}
			//stats (if atomic)
			if(opq->cfg.atomicStats){
				if(NBDataPtrsStats_isSet(opq->stats.provider)){
					NBDataPtrsStats_buffsUpdated(opq->stats.provider, &stack->upd);
				} 
				NBMemory_setZeroSt(stack->upd, STNBDataPtrsStatsUpd);
			}
		}
		NBThreadMutexRW_unlockFromWrite(&stack->mutex);
	}
	NBASSERT(r)
	return r;
}

BOOL NBDataPtrsPool_addPtr(STNBDataPtrsPoolRef ref, void* ptr, const UI32 size){
	BOOL r = FALSE;
	STNBDataPtrsPoolOpq* opq = (STNBDataPtrsPoolOpq*)ref.opaque;
	NBASSERT(NBDataPtrsPool_isClass(ref))
	if(ptr != NULL && size != 0){
		STNBDataPtrsPoolStack* stack = opq->buffs.incoming; 
		NBThreadMutexRW_lockForWrite(&stack->mutex);
		{
			//add
			{
				r = NBDataPtrsPool_addPtrAtStackLocked_(opq, stack, ptr, size);
			}
			//stats (if atomic)
			if(opq->cfg.atomicStats){
				if(NBDataPtrsStats_isSet(opq->stats.provider)){
					NBDataPtrsStats_buffsUpdated(opq->stats.provider, &stack->upd);
				} 
				NBMemory_setZeroSt(stack->upd, STNBDataPtrsStatsUpd);
			}
		}
		NBThreadMutexRW_unlockFromWrite(&stack->mutex);
	}
	NBASSERT(r)
	return r;
}


//Actions

UI32 NBDataPtrsPool_getPtrsFromStackLocked_(STNBDataPtrsPoolRef ref, STNBDataPtrsPoolStack* stack, const UI32 minBytesPerPtr, STNBDataPtr* srcAndDst, const UI32 srcAndDstSz, const BOOL allocateIfNecesary){
	UI32 r = 0;
	NBASSERT(minBytesPerPtr > 0 && srcAndDst != NULL && srcAndDstSz > 0)
	STNBDataPtr* dst = srcAndDst;
	const STNBDataPtr* dstAfterEnd = srcAndDst + srcAndDstSz;
	while(dst < dstAfterEnd){
		//search in pool
		if(dst->def.alloc.ptr != NULL && dst->def.alloc.size >= minBytesPerPtr){
			//current ptr works (no need for new allocation)
		} else {
			BOOL fnd = FALSE;
			//allocate from available list
			if(stack->availUse > 0){
				STNBDataPtrsPoolPtr* last = &stack->avail[stack->availUse - 1];
				STNBDataPtrsPoolPtr* cur = last;
				while(stack->avail <= cur){
					if(cur->ptr != NULL && cur->size >= minBytesPerPtr){
						if(dst->def.alloc.ptr != NULL && dst->def.alloc.size > 0){
							//PRINTF_INFO("NBDataPtrsPool, swapping ptr (%d vs %d bytes).\n", dst->def.alloc.size, cur->size);
							//NBASSERT(dst->def.alloc.size == cur->size) //Temporal: testing integrity
							//swap pointer to buffer
							STNBDataPtrsPoolPtr tmp = *cur;
							//
							stack->upd.taken.count++;
							stack->upd.taken.bytes += dst->def.alloc.size;
							//
							cur->ptr	= dst->def.alloc.ptr;
							cur->size	= dst->def.alloc.size;
							NBDataPtr_resignToBuffer(dst);
							NBDataPtr_setAsEmptyPoolPtr(dst, ref, tmp.ptr, tmp.size);
							//
							stack->upd.given.count++;
							stack->upd.given.bytes += dst->def.alloc.size;
						} else {
							//set pointer and release
							//PRINTF_INFO("NBDataPtrsPool, giving ptr (%d bytes).\n", cur->size);
							NBASSERT(stack->bytesTotal >= cur->size)
							stack->bytesTotal -= cur->size;
							NBDataPtr_setAsEmptyPoolPtr(dst, ref, cur->ptr, cur->size);
							//
							stack->upd.given.count++;
							stack->upd.given.bytes += dst->def.alloc.size;
							//
							//fill gap
							last--;
							stack->availUse--;
							while(cur < last){
								*cur = *(cur + 1);
								cur++;
							}
						}
						//stop
						fnd = TRUE;
						break;
					}
					cur--;
				}
			}
			//allocate new (if necesary)
			if(!fnd){
				if(allocateIfNecesary){
					NBDataPtr_allocEmptyPtr(dst, minBytesPerPtr);
					stack->upd.alloc.count++;
					stack->upd.alloc.bytes += minBytesPerPtr;
					stack->upd.given.count++;
					stack->upd.given.bytes += minBytesPerPtr;
				} else {
					//stop cicle before 'dst++'
					break;
				}
			}
		}
		//next
		dst++;
	}
	r = (UI32)(dst - srcAndDst);
	return r;
}

UI32 NBDataPtrsPool_getPtrs(STNBDataPtrsPoolRef ref, const UI32 minBytesPerPtr, STNBDataPtr* srcAndDst, const UI32 srcAndDstSz){
	UI32 r = 0;
	STNBDataPtrsPoolOpq* opq = (STNBDataPtrsPoolOpq*)ref.opaque;
	NBASSERT(NBDataPtrsPool_isClass(ref))
	if(minBytesPerPtr > 0 && srcAndDst != NULL && srcAndDstSz > 0){
		BOOL swapAndRetry = FALSE;
		STNBDataPtrsPoolStack* stack = opq->buffs.outgoing;
		//get from outgoing stack
		{
			NBThreadMutexRW_lockForWrite(&stack->mutex);
			{
				//get (without allocating new)
				{
					r += NBDataPtrsPool_getPtrsFromStackLocked_(ref, stack, minBytesPerPtr, &srcAndDst[r], (srcAndDstSz - r), FALSE);
				}
				//stats (if atomic)
				if(opq->cfg.atomicStats){
					if(NBDataPtrsStats_isSet(opq->stats.provider)){
						NBDataPtrsStats_buffsUpdated(opq->stats.provider, &stack->upd);
					} 
					NBMemory_setZeroSt(stack->upd, STNBDataPtrsStatsUpd);
				}
				//Not enough (swap buffers and try again)
				if(r < srcAndDstSz){
					swapAndRetry = TRUE;
				}
			}
			NBThreadMutexRW_unlockFromWrite(&stack->mutex);
		}
		//swap and get from other
		if(swapAndRetry){
			//Swap (locked)
			{
				NBObject_lock(opq);
				{
					STNBDataPtrsPoolStack* stackTmp = opq->buffs.outgoing;
					opq->buffs.outgoing = opq->buffs.incoming;
					opq->buffs.incoming = stackTmp;
					//retry
					stack = opq->buffs.outgoing;
				}
				NBObject_unlock(opq);
			}
			//Retry
			NBThreadMutexRW_lockForWrite(&stack->mutex);
			{
				stack->upd.stacksSwaps++;
				//get (allow allocation of new ptrs)
				{
					r += NBDataPtrsPool_getPtrsFromStackLocked_(ref, stack, minBytesPerPtr, &srcAndDst[r], (srcAndDstSz - r), TRUE);
				}
				//stats (if atomic)
				if(opq->cfg.atomicStats){
					if(NBDataPtrsStats_isSet(opq->stats.provider)){
						NBDataPtrsStats_buffsUpdated(opq->stats.provider, &stack->upd);
					} 
					NBMemory_setZeroSt(stack->upd, STNBDataPtrsStatsUpd);
				}
			}
			NBThreadMutexRW_unlockFromWrite(&stack->mutex);
		}
	}
	NBASSERT(r > 0)
	return r;
}



