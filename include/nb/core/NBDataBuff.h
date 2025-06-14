//
//  NBDataBuff.h
//  thinstream
//
//  Created by Marcos Ortega on 9/3/19.
//

#ifndef NBDataBuff_h
#define NBDataBuff_h

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBDataChunk.h"
#include "nb/core/NBArray.h"

#ifdef __cplusplus
extern "C" {
#endif

struct STNBDataChunk_;

typedef struct STNBDataBuffState_ {
	UI32	readersCount;
	UI32	invalidsCount;	//times 'invalidate' was called
	UI32	missesCount;	//times 'appendChunk' failed
	UI32	buffUse;
	UI32	buffSz;
} STNBDataBuffState;

//------------
//- NBDataBuff
//------------

NB_OBJREF_HEADER(NBDataBuff)

typedef struct STNBDataBuffLstnr_ {
	void*	obj;
	void	(*consumeZeroReadersState)(void* obj, STNBDataBuffRef buff, const STNBDataBuffState* bState);
} STNBDataBuffLstnr;

//Config
BOOL NBDataBuff_setListener(STNBDataBuffRef ref, const STNBDataBuffLstnr* lstnr, const BOOL empty, STNBDataBuffState* dstState);

//Data
BOOL NBDataBuff_createBuffer(STNBDataBuffRef ref, const UI32 sz);
BOOL NBDataBuff_empty(STNBDataBuffRef ref, STNBDataBuffState* dstState);
BOOL NBDataBuff_appendChunk(STNBDataBuffRef ref, const void* data, const UI32 dataSz, struct STNBDataChunk_* dst, STNBDataBuffState* dstState);
UI32 NBDataBuff_appendChunks(STNBDataBuffRef ref, const STNBDataChunkPtr* ptrs, const UI32 ptrsSz, STNBArray* dst /*STNBDataChunk*/, STNBDataBuffState* dstState);
STNBDataBuffState NBDataBuff_invalidate(STNBDataBuffRef ref);
void* NBDataBuff_dataPtr(STNBDataBuffRef ref, const UI32 iStart, const UI32 size);

//State
STNBDataBuffState NBDataBuff_getState(STNBDataBuffRef ref); 

//Readers
BOOL NBDataBuff_increaseReaders(STNBDataBuffRef ref, const UI32 readersCount);
BOOL NBDataBuff_decreaseReaders(STNBDataBuffRef ref, const UI32 readersCount);

#ifdef __cplusplus
} //extern "C"
#endif
		
#endif /* NBDataBuff_h */
