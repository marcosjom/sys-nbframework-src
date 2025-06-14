//
//  NBDataBuff.h
//  thinstream
//
//  Created by Marcos Ortega on 9/3/19.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBDataBuff.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBThreadCond.h"
#include "nb/core/NBDataChunk.h"

//

typedef struct STNBDataBuffOpq_ {
	STNBObject			prnt;	//parent
	STNBThreadCond		cond;
	STNBDataBuffLstnr	lstnr;
	//State
	struct {
		UI32			readersCount;
		UI32			invalidsCount;	//times 'invalidate' was called
		UI32			missesCount;	//times 'appendChunk' failed
		BOOL			waitingForZeroReaders;	//flag for cond
	} state;
	//Data
	struct {
		BYTE*			buff;
		UI32			buffUse;
		UI32			buffSz;
	} data;
} STNBDataBuffOpq;

NB_OBJREF_BODY(NBDataBuff, STNBDataBuffOpq, NBObject)

//

void NBDataBuff_initZeroed(STNBObject* obj){
	STNBDataBuffOpq* opq = (STNBDataBuffOpq*)obj;
	NBThreadCond_init(&opq->cond);
}

void NBDataBuff_uninitLocked(STNBObject* obj){
	STNBDataBuffOpq* opq = (STNBDataBuffOpq*)obj;
	//Wait for readers zero
	while(opq->state.readersCount > 0){
		opq->state.waitingForZeroReaders = TRUE;
		NBThreadCond_waitObj(&opq->cond, opq);
	}
	opq->state.waitingForZeroReaders = FALSE;
	NBASSERT(opq->state.readersCount == 0) //should be zero
	//Data
	{
		if(opq->data.buff != NULL){
			NBMemory_free(opq->data.buff);
			opq->data.buff		= NULL;
			opq->data.buffSz	= 0;
		}
	}
	//Cond
	NBThreadCond_release(&opq->cond);
}

STNBDataBuffState NBDataBuff_getStateLockedOpq_(STNBDataBuffOpq* opq){
	STNBDataBuffState r;
	NBMemory_setZeroSt(r, STNBDataBuffState);
	{
		r.readersCount	= opq->state.readersCount;
		r.invalidsCount	= opq->state.invalidsCount;
		r.missesCount	= opq->state.missesCount;
		r.buffUse		= opq->data.buffUse;
		r.buffSz		= opq->data.buffSz;
	}
	return r;
}
	
STNBDataBuffState NBDataBuff_getState(STNBDataBuffRef ref){
	STNBDataBuffState r;
	STNBDataBuffOpq* opq = (STNBDataBuffOpq*)ref.opaque;
	NBASSERT(NBDataBuff_isClass(ref))
	NBObject_lock(opq);
	{
		r = NBDataBuff_getStateLockedOpq_(opq);
	}
	NBObject_unlock(opq);
	return r;
}

//Config

BOOL NBDataBuff_setListener(STNBDataBuffRef ref, const STNBDataBuffLstnr* lstnr, const BOOL empty, STNBDataBuffState* dstState){
	BOOL r = FALSE;
	STNBDataBuffOpq* opq = (STNBDataBuffOpq*)ref.opaque;
	NBASSERT(NBDataBuff_isClass(ref))
	NBObject_lock(opq);
	if(opq->state.readersCount == 0){
		//Empty
		if(empty){
			opq->state.invalidsCount	= 0;
			opq->state.missesCount		= 0;
			opq->data.buffUse			= 0;
		}
		//Apply listener
		if(lstnr == NULL){
			NBMemory_setZeroSt(opq->lstnr, STNBDataBuffLstnr);
		} else {
			opq->lstnr = *lstnr;
		}
		r = TRUE;
	}
	if(dstState != NULL){
		*dstState = NBDataBuff_getStateLockedOpq_(opq);
	}
	NBObject_unlock(opq);
	return r;
}

//Data

BOOL NBDataBuff_createBuffer(STNBDataBuffRef ref, const UI32 sz){
	BOOL r = FALSE;
	STNBDataBuffOpq* opq = (STNBDataBuffOpq*)ref.opaque;
	NBASSERT(NBDataBuff_isClass(ref))
	NBObject_lock(opq);
	if(opq->data.buff == NULL && sz > 0){
		opq->data.buff		= NBMemory_alloc(sz);
		opq->data.buffSz	= sz;
		r = TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBDataBuff_empty(STNBDataBuffRef ref, STNBDataBuffState* dstState){
	BOOL r = FALSE;
	STNBDataBuffOpq* opq = (STNBDataBuffOpq*)ref.opaque;
	NBASSERT(NBDataBuff_isClass(ref))
	NBObject_lock(opq);
	if(opq->state.readersCount == 0){
		opq->state.invalidsCount	= 0;
		opq->state.missesCount		= 0;
		opq->data.buffUse			= 0;
		r = TRUE;
	}
	if(dstState != NULL){
		*dstState = NBDataBuff_getStateLockedOpq_(opq);
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBDataBuff_appendChunk(STNBDataBuffRef ref, const void* data, const UI32 dataSz, STNBDataChunk* dst, STNBDataBuffState* dstState){
	BOOL r = FALSE;
	STNBDataBuffOpq* opq = (STNBDataBuffOpq*)ref.opaque;
	NBASSERT(NBDataBuff_isClass(ref))
	if(data != NULL && dataSz > 0){
		STNBDataChunk chunkToRelease; BOOL chunkToReleaseSet = FALSE;
		NBObject_lock(opq);
		{
			//Append
			{
				if((opq->data.buffUse + dataSz) <= opq->data.buffSz){
					void* dataPtr		= &opq->data.buff[opq->data.buffUse];
					NBMemory_copy(dataPtr, data, dataSz);
					opq->data.buffUse += dataSz;
					//Populate dst
					if(dst != NULL){
						//queue to release
						if(NBDataBuff_isSet(dst->buff)){
							chunkToRelease		= *dst;
							chunkToReleaseSet	= TRUE;
							NBDataChunk_init(dst);
						}
						//set
						dst->data		= dataPtr;
						dst->size		= dataSz;
						dst->buff		= ref;
						dst->rdrCount	= 1;
						//Retain myself (locked)
						opq->state.readersCount++;
						//PRINTF_INFO("NBDataBuff, (%llu) ++readers: %u (%u bytes used).\n", (UI64)opq, opq->state.readersCount, opq->data.buffUse);
					}
					r = TRUE;
				}
			}
			//update state
			if(!r){
				opq->state.missesCount++;
			}
			//Populate state
			if(dstState != NULL){
				*dstState = NBDataBuff_getStateLockedOpq_(opq);
			}
		}
		NBObject_unlock(opq);
		//Release old buff ref (unlocked)
		if(chunkToReleaseSet){
			NBDataChunk_release(&chunkToRelease);
		}
	}
	return r;
}

UI32 NBDataBuff_appendChunks(STNBDataBuffRef ref, const STNBDataChunkPtr* ptrs, const UI32 ptrsSz, STNBArray* dst /*STNBDataChunk*/, STNBDataBuffState* dstState){
	UI32 r = 0;
	STNBDataBuffOpq* opq = (STNBDataBuffOpq*)ref.opaque;
	NBASSERT(NBDataBuff_isClass(ref))
	if(ptrs != NULL && ptrsSz > 0){
		NBObject_lock(opq);
		{
			const STNBDataChunkPtr* ptr = ptrs;
			const STNBDataChunkPtr* ptrAfterEnd = ptrs + ptrsSz;
			while(ptr < ptrAfterEnd){
				if(ptr->data != NULL){
					if((opq->data.buffUse + ptr->size) > opq->data.buffSz){
						//stop
						break;
					} else {
						void* dataPtr = &opq->data.buff[opq->data.buffUse];
						NBMemory_copy(dataPtr, ptr->data, ptr->size);
						opq->data.buffUse += ptr->size;
						//Populate dst
						if(dst != NULL){
							STNBDataChunk chunk2;
							NBDataChunk_init(&chunk2);
							//set
							chunk2.data		= dataPtr;
							chunk2.size		= ptr->size;
							chunk2.buff		= ref;
							chunk2.rdrCount	= 1;
							//Retain myself (locked)
							opq->state.readersCount++;
							//PRINTF_INFO("NBDataBuff, (%llu) ++readers: %u (%u bytes used).\n", (UI64)opq, opq->state.readersCount, opq->data.buffUse);
							NBArray_addValue(dst, chunk2);
						}
					}
				}
				//Next
				ptr++;
			}
			//Result
			r = (UI32)(ptr - ptrs);
			//update state
			if(r < ptrsSz){
				opq->state.missesCount++;
			}
			//Populate state
			if(dstState != NULL){
				*dstState = NBDataBuff_getStateLockedOpq_(opq);
			}
		}
		NBObject_unlock(opq);
	}
	return r;
}

STNBDataBuffState NBDataBuff_invalidate(STNBDataBuffRef ref){
	STNBDataBuffState r;
	STNBDataBuffOpq* opq = (STNBDataBuffOpq*)ref.opaque;
	NBASSERT(NBDataBuff_isClass(ref))
	NBObject_lock(opq);
	{
		opq->state.invalidsCount++;
		r = NBDataBuff_getStateLockedOpq_(opq);
	}
	NBObject_unlock(opq);
	return r;
}

void* NBDataBuff_dataPtr(STNBDataBuffRef ref, const UI32 iStart, const UI32 size){
	void* r = NULL;
	STNBDataBuffOpq* opq = (STNBDataBuffOpq*)ref.opaque;
	NBASSERT(NBDataBuff_isClass(ref))
	NBObject_lock(opq);
	NBASSERT(iStart <= opq->data.buffUse && (iStart + size) <= opq->data.buffUse)
	if(iStart <= opq->data.buffUse && (iStart + size) <= opq->data.buffUse){
		r = &opq->data.buff[iStart];
	}
	NBObject_unlock(opq);
	return r;
}

//Readers

BOOL NBDataBuff_increaseReaders(STNBDataBuffRef ref, const UI32 readersCount){
	BOOL r = FALSE;
	STNBDataBuffOpq* opq = (STNBDataBuffOpq*)ref.opaque;
	NBASSERT(NBDataBuff_isClass(ref))
	NBObject_lock(opq);
	NBASSERT(opq->data.buff != NULL)
	{
		opq->state.readersCount += readersCount;
		//PRINTF_INFO("NBDataBuff, (%llu) ++readers: %u (%u bytes used).\n", (UI64)opq, opq->state.readersCount, opq->data.buffUse);
		r = TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBDataBuff_decreaseReaders(STNBDataBuffRef ref, const UI32 readersCount){
	BOOL r = FALSE;
	STNBDataBuffOpq* opq = (STNBDataBuffOpq*)ref.opaque;
	NBASSERT(NBDataBuff_isClass(ref))
	STNBDataBuffState bState; STNBDataBuffLstnr lstnr; BOOL notify = FALSE;
	NBMemory_setZeroSt(lstnr, STNBDataBuffLstnr)
	NBObject_lock(opq);
	NBASSERT(opq->data.buff != NULL)
	NBASSERT(opq->state.readersCount >= readersCount)
	if(opq->state.readersCount >= readersCount){
		opq->state.readersCount -= readersCount;
		//PRINTF_INFO("NBDataBuff, (%llu) --readers: %u (%u bytes used).\n", (UI64)opq, opq->state.readersCount, opq->data.buffUse);
		//Notify
		if(opq->state.readersCount == 0){
			if(opq->lstnr.consumeZeroReadersState != NULL){
				bState	= NBDataBuff_getStateLockedOpq_(opq);
				lstnr	= opq->lstnr;
				notify	= TRUE;
			} 
			//Broadcast zero state
			if(opq->state.waitingForZeroReaders){
				NBThreadCond_broadcast(&opq->cond);
			}
		}
		r = TRUE;
	} 
	NBObject_unlock(opq);
	//Notify (unlocked)
	if(notify){
		if(lstnr.consumeZeroReadersState != NULL){
			(*lstnr.consumeZeroReadersState)(lstnr.obj, ref, &bState);
		}
	}
	return r;
	
}
