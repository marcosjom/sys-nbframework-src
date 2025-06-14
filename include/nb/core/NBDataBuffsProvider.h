//
//  NBDataBuffsProvider.h
//  thinstream
//
//  Created by Marcos Ortega on 9/3/19.
//

#ifndef NBDataBuffsProvider_h
#define NBDataBuffsProvider_h

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBDataChunk.h"
#include "nb/core/NBDataBuff.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBDataBuffsStats.h"
#include "nb/core/NBDataBuffsPool.h"

#ifdef __cplusplus
extern "C" {
#endif

//-------------------
//- DataBuffsProvider
//-------------------

NB_OBJREF_HEADER(NBDataBuffsProvider)

//Cfg
BOOL NBDataBuffsProvider_setStatsProvider(STNBDataBuffsProviderRef ref, STNBDataBuffsStatsRef provider);
BOOL NBDataBuffsProvider_setPool(STNBDataBuffsProviderRef ref, STNBDataBuffsPoolRef pool);
BOOL NBDataBuffsProvider_setNewBufferSz(STNBDataBuffsProviderRef ref, const UI32 bytesCount);

//Chunk
BOOL NBDataBuffsProvider_appendChunk(STNBDataBuffsProviderRef ref, const void* data, const UI32 dataSz, STNBDataChunk* dst);
UI32 NBDataBuffsProvider_appendChunks(STNBDataBuffsProviderRef ref, const STNBDataChunkPtr* ptrs, const UI32 ptrsSz, STNBArray* dst /*STNBDataChunk*/);

#ifdef __cplusplus
} //extern "C"
#endif
		
#endif /* NBDataBuffsProvider_h */
