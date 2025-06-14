//
//  NBDataBuffsProvider.h
//  thinstream
//
//  Created by Marcos Ortega on 9/3/19.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBDataBuffsProvider.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBThreadCond.h"
#include "nb/core/NBDataBuff.h"
#include "nb/core/NBArray.h"

//--------------------
//- NBDataBuffsProvider
//--------------------

typedef struct STNBDataBuffsProviderOpq_ {
	STNBObject					prnt;			//parent
	STNBThreadCond				cond;
	BOOL						waitingForOrphans;
	//cfg
	struct {
		UI32					newBuffsSz;		//in bytes
	} cfg;
	//stats
	struct {
		STNBDataBuffsStatsRef	provider;
	} stats;
	//buffers
	struct {
		UI64					seqCur;			//amount of buffers created
		UI32					orphansCount;	//amount of buffers left in hands of reader
		UI32					aliveCount;		//amount of buffer alive (current and orphaned)
		UI64					bytesCount;		//amount of bytes alive (current and orphaned)
		STNBDataBuffRef			cur;			//current buffer
		STNBDataBuffsPoolRef	pool;
	} buffs;
} STNBDataBuffsProviderOpq;

NB_OBJREF_BODY(NBDataBuffsProvider, STNBDataBuffsProviderOpq, NBObject)

//Callbacks

void NBDataBuffsProvider_consumeZeroReadersState_(void* pObj, STNBDataBuffRef buff, const STNBDataBuffState* bState);

//

void NBDataBuffsProvider_initZeroed(STNBObject* obj){
	STNBDataBuffsProviderOpq* opq = (STNBDataBuffsProviderOpq*)obj;
	NBThreadCond_init(&opq->cond);
}

void NBDataBuffsProvider_uninitLocked(STNBObject* obj){
	STNBDataBuffsProviderOpq* opq = (STNBDataBuffsProviderOpq*)obj;
	STNBDataBuffsStatsUpd upd;
	NBMemory_setZeroSt(upd, STNBDataBuffsStatsUpd);
	//Release current buffer
	if(NBDataBuff_isSet(opq->buffs.cur)){
		const STNBDataBuffState bState = NBDataBuff_invalidate(opq->buffs.cur);
		if(bState.readersCount > 0){
			//leave orphan
			opq->buffs.orphansCount++;
			NBDataBuff_null(&opq->buffs.cur);
		} else {
			//release
			{
				//PRINTF_INFO("NBDataBuffsProvider, cur buffer(%llu, %u bytes) released while releasing and zero-readers.\n", (UI64)opq->buffs.cur->opaque, buffSz);
				NBDataBuff_release(&opq->buffs.cur);
				NBDataBuff_null(&opq->buffs.cur);
				upd.freed.count++;
				upd.unwired.count++;
				upd.freed.bytes		+= bState.buffSz;
				upd.unwired.bytes	+= bState.buffSz;
			}
			NBASSERT(opq->buffs.aliveCount > 0)
			NBASSERT(opq->buffs.bytesCount >= bState.buffSz)
			opq->buffs.aliveCount--;
			opq->buffs.bytesCount -= bState.buffSz;
		}
	}
	//Wait for all orphans buffers to be released
	{
		while(opq->buffs.orphansCount > 0){
			opq->waitingForOrphans = TRUE;
			NBThreadCond_waitObj(&opq->cond, opq);
		}
		opq->waitingForOrphans = FALSE;
		NBASSERT(opq->buffs.orphansCount == 0) //must be zero
		NBASSERT(opq->buffs.bytesCount == 0)	//must be zero
	}
	NBASSERT(opq->buffs.orphansCount == 0)
	NBASSERT(opq->buffs.aliveCount == 0)
	NBASSERT(opq->buffs.bytesCount == 0)
	NBASSERT(!NBDataBuff_isSet(opq->buffs.cur))
	//stats
	if(NBDataBuffsStats_isSet(opq->stats.provider)){
		NBDataBuffsStats_buffsUpdated(opq->stats.provider, &upd);
		NBDataBuffsStats_release(&opq->stats.provider);
	}
	//Buffers
	NBDataBuffsPool_release(&opq->buffs.pool);
	//
	NBThreadCond_release(&opq->cond);
}

//Cfg

BOOL NBDataBuffsProvider_setStatsProvider(STNBDataBuffsProviderRef ref, STNBDataBuffsStatsRef provider){
	BOOL r = FALSE;
	STNBDataBuffsProviderOpq* opq = (STNBDataBuffsProviderOpq*)ref.opaque;
	NBASSERT(NBDataBuffsProvider_isClass(ref))
	NBObject_lock(opq);
	if(opq->buffs.seqCur == 0){
		NBDataBuffsStats_set(&opq->stats.provider, &provider);
		r = TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBDataBuffsProvider_setPool(STNBDataBuffsProviderRef ref, STNBDataBuffsPoolRef pool){
	BOOL r = FALSE;
	STNBDataBuffsProviderOpq* opq = (STNBDataBuffsProviderOpq*)ref.opaque;
	NBASSERT(NBDataBuffsProvider_isClass(ref))
	NBObject_lock(opq);
	if(opq->buffs.seqCur == 0){
		NBDataBuffsPool_set(&opq->buffs.pool, &pool);
		r = TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBDataBuffsProvider_setNewBufferSz(STNBDataBuffsProviderRef ref, const UI32 bytesCount){
	BOOL r = FALSE;
	STNBDataBuffsProviderOpq* opq = (STNBDataBuffsProviderOpq*)ref.opaque;
	NBASSERT(NBDataBuffsProvider_isClass(ref))
	NBObject_lock(opq);
	if(opq->buffs.seqCur == 0){
		opq->cfg.newBuffsSz = bytesCount;
		r = TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

//Chunk
	
BOOL NBDataBuffsProvider_appendChunk(STNBDataBuffsProviderRef ref, const void* data, const UI32 dataSz, STNBDataChunk* dst){
	BOOL r = FALSE;
	STNBDataBuffsProviderOpq* opq = (STNBDataBuffsProviderOpq*)ref.opaque;
	NBASSERT(NBDataBuffsProvider_isClass(ref))
	//add
	{
		STNBDataBuffsStatsUpd upd;
		NBMemory_setZeroSt(upd, STNBDataBuffsStatsUpd);
		NBObject_lock(opq);
		//Add to current buffer
		if(NBDataBuff_isSet(opq->buffs.cur)){
			STNBDataBuffState bState;
			if(NBDataBuff_appendChunk(opq->buffs.cur, data, dataSz, dst, &bState)){
				r = TRUE;
			} else if(bState.readersCount > 0){
				//leave as orphan
				//PRINTF_INFO("NBDataBuffsProvider, left orphan cur-buffer(%llu, %u bytes) with %u readers.\n", (UI64)opq->buffs.cur->opaque, NBDataBuff_getState(opq->buffs.cur).buffSz, readersCount);
				opq->buffs.orphansCount++;
				NBDataBuff_null(&opq->buffs.cur);
			} else {
				//reuse buffer
				if(!NBDataBuff_empty(opq->buffs.cur, &bState)){
					NBASSERT(FALSE) //program logic error
				} else if(NBDataBuff_appendChunk(opq->buffs.cur, data, dataSz, dst, &bState)){
					//PRINTF_INFO("NBDataBuffsProvider, restarted cur-buffer(%llu, %u bytes) after full and zero-readers.\n", (UI64)opq->buffs.cur->opaque, NBDataBuff_getState(opq->buffs.cur).buffSz);
					r = TRUE;
				} else if(bState.readersCount > 0){
					//leave as orphan
					//PRINTF_INFO("NBDataBuffsProvider, left orphan cur-buffer(%llu, %u bytes) with %u readers.\n", (UI64)opq->buffs.cur->opaque, NBDataBuff_getState(opq->buffs.cur).buffSz, readersCount);
					opq->buffs.orphansCount++;
					NBDataBuff_null(&opq->buffs.cur);
					NBASSERT(FALSE) //program logic error
				}
				//release buffer
				if(!r && NBDataBuff_isSet(opq->buffs.cur)){
					//send to external buffers
					{
						if(NBDataBuffsPool_isSet(opq->buffs.pool)){
							if(NBDataBuffsPool_addAvailableBuffer(opq->buffs.pool, opq->buffs.cur)){
								NBDataBuff_null(&opq->buffs.cur);
								upd.unwired.count++;
								upd.unwired.bytes += bState.buffSz;
							}
						}
					}
					//release
					if(NBDataBuff_isSet(opq->buffs.cur)){
						//PRINTF_INFO("NBDataBuffsProvider, released cur buffer(%llu, %u bytes) while full and zero-readers.\n", (UI64)opq->buffs.cur->opaque, buffSz);
						NBDataBuff_release(&opq->buffs.cur);
						NBDataBuff_null(&opq->buffs.cur);
						upd.freed.count++;
						upd.unwired.count++;
						upd.freed.bytes		+= bState.buffSz;
						upd.unwired.bytes	+= bState.buffSz;
					}
					NBASSERT(opq->buffs.aliveCount > 0)
					NBASSERT(opq->buffs.bytesCount >= bState.buffSz)
					opq->buffs.aliveCount--;
					opq->buffs.bytesCount -= bState.buffSz;
				}
			}
		}
		//Add to new buffer
		if(!r && !NBDataBuff_isSet(opq->buffs.cur)){
			BOOL forbidNewBuff = FALSE;
			//try to reuse buffer
			if(NBDataBuffsPool_isSet(opq->buffs.pool)){
				STNBDataBuffRef buff = NBDataBuffsPool_getAvailableBuffer(opq->buffs.pool, &forbidNewBuff);
				if(NBDataBuff_isSet(buff)){
					STNBDataBuffState bState;
					NBMemory_setZeroSt(bState, STNBDataBuffState);
					//Empty and set listener
					{
						const BOOL empty = TRUE;
						STNBDataBuffLstnr lstnr;
						NBMemory_setZeroSt(lstnr, STNBDataBuffLstnr);
						lstnr.obj = opq;
						lstnr.consumeZeroReadersState = NBDataBuffsProvider_consumeZeroReadersState_;
						NBDataBuff_setListener(buff, &lstnr, empty, &bState);
					}
					NBASSERT(bState.buffUse == 0)
					NBASSERT(bState.invalidsCount == 0)
					NBASSERT(bState.missesCount == 0)
					NBASSERT(bState.readersCount == 0)
					opq->buffs.aliveCount++;
					opq->buffs.bytesCount	+= bState.buffSz;
					opq->buffs.cur			= buff;
					opq->buffs.seqCur++;
					//PRINTF_INFO("NBDataBuffsProvider, recycled buffer(%llu, %u bytes) used (%u orphans).\n", (UI64)opq->buffs.cur->opaque, NBDataBuff_getState(opq->buffs.cur).buffSz, opq->buffs.orphansCount);
					upd.wired.count++;
					upd.wired.bytes			+= bState.buffSz;
				}
			}
			//allocate new buffer
			if(!NBDataBuff_isSet(opq->buffs.cur) && !forbidNewBuff){
				const UI32 buffSzCfg	= opq->cfg.newBuffsSz; 
				const UI32 buffSz		= (buffSzCfg < dataSz ? dataSz : buffSzCfg);
				STNBDataBuffRef buff	= NBDataBuff_alloc(NULL);
				STNBDataBuffState bState;
				NBMemory_setZeroSt(bState, STNBDataBuffState);
				NBDataBuff_createBuffer(buff, buffSz);
				//Empty and set listener
				{
					const BOOL empty = TRUE;
					STNBDataBuffLstnr lstnr;
					NBMemory_setZeroSt(lstnr, STNBDataBuffLstnr);
					lstnr.obj = opq;
					lstnr.consumeZeroReadersState = NBDataBuffsProvider_consumeZeroReadersState_;
					NBDataBuff_setListener(buff, &lstnr, empty, &bState);
				}
				NBASSERT(bState.buffUse == 0)
				NBASSERT(bState.invalidsCount == 0)
				NBASSERT(bState.missesCount == 0)
				NBASSERT(bState.readersCount == 0)
				//
				opq->buffs.aliveCount++;
				opq->buffs.bytesCount += buffSz;
				opq->buffs.cur = buff;
				opq->buffs.seqCur++;
				//PRINTF_INFO("NBDataBuffsProvider, new cur buffer(%llu, %u bytes) allocated (%u orphans).\n", (UI64)buff->opaque, buffSz, opq->buffs.orphansCount);
				upd.alloc.count++;
				upd.wired.count++;
				upd.alloc.bytes += buffSz;
				upd.wired.bytes += buffSz;
			}
			//append chunk
			if(NBDataBuff_isSet(opq->buffs.cur)){
				STNBDataBuffState bState;
				if(NBDataBuff_appendChunk(opq->buffs.cur, data, dataSz, dst, &bState)){
					r = TRUE;
				} else if(bState.readersCount > 0){
					//leave as orphan
					//PRINTF_INFO("NBDataBuffsProvider, left orphan cur-buffer(%llu, %u bytes) with %u readers.\n", (UI64)opq->buffs.cur->opaque, NBDataBuff_getState(opq->buffs.cur).buffSz, readersCount);
					NBDataBuff_null(&opq->buffs.cur);
					opq->buffs.orphansCount++;
				}
			}
		}
		//notify
		if(NBDataBuffsStats_isSet(opq->stats.provider)){
			NBDataBuffsStats_buffsUpdated(opq->stats.provider, &upd);
		}
		NBObject_unlock(opq);
	}
	return r;
}

UI32 NBDataBuffsProvider_appendChunks(STNBDataBuffsProviderRef ref, const STNBDataChunkPtr* ptrs, const UI32 ptrsSz, STNBArray* dst /*STNBDataChunk*/){
	UI32 r = 0;
	if(ptrs != NULL && ptrsSz > 0){
		STNBDataBuffsProviderOpq* opq = (STNBDataBuffsProviderOpq*)ref.opaque;
		NBASSERT(NBDataBuffsProvider_isClass(ref))
		//add
		{
			STNBDataBuffsStatsUpd upd;
			NBMemory_setZeroSt(upd, STNBDataBuffsStatsUpd);
			NBObject_lock(opq);
			{
				UI32 cnsmdTotal = 0;
				do {
					//Add to current buffer
					if(NBDataBuff_isSet(opq->buffs.cur)){
						STNBDataBuffState bState;
						const UI32 cnsmdTick = NBDataBuff_appendChunks(opq->buffs.cur, &ptrs[cnsmdTotal], (ptrsSz - cnsmdTotal), dst, &bState);
						cnsmdTotal += cnsmdTick; NBASSERT(cnsmdTotal <= ptrsSz)
						//PRINTF_INFO("NBDataBuffsProvider, cur-buffer(%llu, %u bytes) %u of %u chunks added (%d current) with %u readers.\n", (UI64)opq->buffs.cur->opaque, bState.buffSz, cnsmdTick, ptrsSz, cnsmdTotal, bState.readersCount);
						if(cnsmdTotal < ptrsSz){
							//Release buffer (some remains)
							if(bState.readersCount > 0){
								//leave as orphan
								//PRINTF_INFO("NBDataBuffsProvider, left orphan cur-buffer(%llu, %u bytes) with %u readers.\n", (UI64)opq->buffs.cur->opaque, bState.buffSz, bState.readersCount);
								NBDataBuff_null(&opq->buffs.cur);
								opq->buffs.orphansCount++;
							} else {
								//send to external buffers
								{
									if(NBDataBuffsPool_isSet(opq->buffs.pool)){
										if(NBDataBuffsPool_addAvailableBuffer(opq->buffs.pool, opq->buffs.cur)){
											//PRINTF_INFO("NBDataBuffsProvider, sent for reuse cur-buffer(%llu, %u bytes) with %u readers.\n", (UI64)opq->buffs.cur->opaque, bState.buffSz, bState.readersCount);
											NBDataBuff_null(&opq->buffs.cur);
											upd.unwired.count++;
											upd.unwired.bytes += bState.buffSz;
										}
									}
								}
								//release
								if(NBDataBuff_isSet(opq->buffs.cur)){
									//PRINTF_INFO("NBDataBuffsProvider, released cur buffer(%llu, %u bytes) while full with %u readers.\n", (UI64)opq->buffs.cur->opaque, bState.buffSz, bState.readersCount);
									NBDataBuff_release(&opq->buffs.cur);
									NBDataBuff_null(&opq->buffs.cur);
									upd.freed.count++;
									upd.unwired.count++;
									upd.freed.bytes += bState.buffSz;
									upd.unwired.bytes += bState.buffSz;
								}
								NBASSERT(opq->buffs.aliveCount > 0)
								NBASSERT(opq->buffs.bytesCount >= bState.buffSz)
								opq->buffs.aliveCount--;
								opq->buffs.bytesCount -= bState.buffSz;
							}
							//Stop
							if(cnsmdTick == 0 && bState.buffUse == 0){
								//PRINTF_INFO("NBDataBuffsProvider, stopping.\n");
								//Empty buffer, nothing fits
								break;
							}
						}
					}
					//Create new buffer
					if(!NBDataBuff_isSet(opq->buffs.cur)){
						BOOL forbidNewBuff = FALSE;
						//try to reuse buffer
						if(NBDataBuffsPool_isSet(opq->buffs.pool)){
							STNBDataBuffRef buff = NBDataBuffsPool_getAvailableBuffer(opq->buffs.pool, &forbidNewBuff);
							if(NBDataBuff_isSet(buff)){
								STNBDataBuffState bState;
								NBMemory_setZeroSt(bState, STNBDataBuffState);
								//Empty and set listener
								{
									const BOOL empty = TRUE;
									STNBDataBuffLstnr lstnr;
									NBMemory_setZeroSt(lstnr, STNBDataBuffLstnr);
									lstnr.obj = opq;
									lstnr.consumeZeroReadersState = NBDataBuffsProvider_consumeZeroReadersState_;
									NBDataBuff_setListener(buff, &lstnr, empty, &bState);
								}
								NBASSERT(bState.buffUse == 0)
								NBASSERT(bState.invalidsCount == 0)
								NBASSERT(bState.missesCount == 0)
								NBASSERT(bState.readersCount == 0)
								opq->buffs.aliveCount++;
								opq->buffs.bytesCount	+= bState.buffSz;
								opq->buffs.cur			= buff;
								opq->buffs.seqCur++;
								//PRINTF_INFO("NBDataBuffsProvider, recycled buffer(%llu, %u bytes) used (%u orphans).\n", (UI64)opq->buffs.cur->opaque, bState.buffSz, opq->buffs.orphansCount);
								upd.wired.count++;
								upd.wired.bytes			+= bState.buffSz;
							}
						}
						//allocate new buffer
						if(!NBDataBuff_isSet(opq->buffs.cur) && !forbidNewBuff){
							const UI32 buffSzCfg	= opq->cfg.newBuffsSz; 
							const UI32 buffSz		= buffSzCfg;
							STNBDataBuffRef buff	= NBDataBuff_alloc(NULL);
							STNBDataBuffState bState;
							NBMemory_setZeroSt(bState, STNBDataBuffState);
							NBDataBuff_createBuffer(buff, buffSz);
							//Empty and set listener
							{
								const BOOL empty = TRUE;
								STNBDataBuffLstnr lstnr;
								NBMemory_setZeroSt(lstnr, STNBDataBuffLstnr);
								lstnr.obj = opq;
								lstnr.consumeZeroReadersState = NBDataBuffsProvider_consumeZeroReadersState_;
								NBDataBuff_setListener(buff, &lstnr, empty, &bState);
							}
							NBASSERT(bState.buffUse == 0)
							NBASSERT(bState.invalidsCount == 0)
							NBASSERT(bState.missesCount == 0)
							NBASSERT(bState.readersCount == 0)
							//
							opq->buffs.aliveCount++;
							opq->buffs.bytesCount += buffSz;
							opq->buffs.cur = buff;
							opq->buffs.seqCur++;
							//PRINTF_INFO("NBDataBuffsProvider, new cur buffer(%llu, %u bytes) allocated (%u orphans).\n", (UI64)buff->opaque, bState.buffSz, opq->buffs.orphansCount);
							upd.alloc.count++;
							upd.wired.count++;
							upd.alloc.bytes += buffSz;
							upd.wired.bytes += buffSz;
						}
						//No buffer
						if(!NBDataBuff_isSet(opq->buffs.cur)){
							//PRINTF_INFO("NBDataBuffsProvider, stopping (no new buffer).\n");
							break;
						}
					}
				} while(cnsmdTotal < ptrsSz);
				//result
				r = cnsmdTotal;
			}
			//notify
			if(NBDataBuffsStats_isSet(opq->stats.provider)){
				NBDataBuffsStats_buffsUpdated(opq->stats.provider, &upd);
			}
			NBObject_unlock(opq);
		}
	}
	return r;
}

void NBDataBuffsProvider_consumeZeroReadersState_(void* pObj, STNBDataBuffRef buff, const STNBDataBuffState* bState){
	STNBDataBuffsProviderOpq* opq = (STNBDataBuffsProviderOpq*)pObj;
	if(opq != NULL && NBDataBuff_isSet(buff) && bState != NULL){
		if(bState->invalidsCount != 0 || bState->missesCount != 0){
			STNBDataBuffsStatsUpd upd;
			NBMemory_setZeroSt(upd, STNBDataBuffsStatsUpd);
			NBObject_lock(opq); //requires lock, beacause of broadcast
			//Do not validate if is the current buffer,
			//this callback is invoked only if the 'buff' is-going/was left orphan 
			{
				//send to external buffers
				{
					if(NBDataBuffsPool_isSet(opq->buffs.pool)){
						if(NBDataBuffsPool_addAvailableBuffer(opq->buffs.pool, buff)){
							NBDataBuff_null(&buff);
							upd.unwired.count++;
							upd.unwired.bytes += bState->buffSz;
						}
					}
				}
				//release orphan
				if(NBDataBuff_isSet(buff)){
					//PRINTF_INFO("NBDataBuffsProvider, buffer(%llu, %u bytes) orphan released after zero-readers (%u orphans remain).\n", (UI64)buff->opaque, buffSz, (opq->buffs.orphansCount - 1));
					NBDataBuff_release(&buff);
					NBDataBuff_null(&buff);
					//stats
					upd.freed.count++;
					upd.freed.bytes += bState->buffSz;
					upd.unwired.count++;
					upd.unwired.bytes += bState->buffSz;
				}
				NBASSERT(opq->buffs.aliveCount > 0)
				NBASSERT(opq->buffs.bytesCount >= bState->buffSz)
				opq->buffs.aliveCount--;
				opq->buffs.bytesCount -= bState->buffSz;
				//
				NBASSERT(opq->buffs.orphansCount > 0)
				opq->buffs.orphansCount--;
			}
			//notify
			if(NBDataBuffsStats_isSet(opq->stats.provider)){
				NBDataBuffsStats_buffsUpdated(opq->stats.provider, &upd);
			}
			//broadcast orphan release
			if(!NBDataBuff_isSet(buff) && opq->waitingForOrphans){
				NBThreadCond_broadcast(&opq->cond);
			}
			NBObject_unlock(opq);
		}
	}
}
