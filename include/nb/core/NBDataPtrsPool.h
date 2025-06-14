//
//  NBDataPtrsPool.h
//  thinstream
//
//  Created by Marcos Ortega on 9/3/19.
//

#ifndef NBDataPtrsPool_h
#define NBDataPtrsPool_h

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBLog.h"
#include "nb/core/NBDataPtr.h"
#include "nb/core/NBDataPtrsStats.h"

#ifdef __cplusplus
extern "C" {
#endif

//CfgBuffers

typedef struct STNBDataPtrsPoolCfg_ {
	char*			maxKeep;		//string representing a number-bytes and oprional-sufix, like "1", "1K", "1kb", "1M", "1G", "1Tb".
	UI64			maxKeepBytes;	//if zero, will be interpreted from the string version.
	BOOL			atomicStats;	//apply stats to provider after each action (do not wait for explicit statsFlush)
} STNBDataPtrsPoolCfg;

const STNBStructMap* NBDataPtrsPoolCfg_getSharedStructMap(void);

//---------------
//- DataBuffsPool
//---------------

NB_OBJREF_HEADER(NBDataPtrsPool)

//
BOOL NBDataPtrsPool_setStatsProvider(STNBDataPtrsPoolRef ref, STNBDataPtrsStatsRef provider);
BOOL NBDataPtrsPool_setCfg(STNBDataPtrsPoolRef ref, const STNBDataPtrsPoolCfg* cfg);

//Actions
void NBDataPtrsPool_flushStats(STNBDataPtrsPoolRef ref);
UI32 NBDataPtrsPool_allocPtrs(STNBDataPtrsPoolRef ref, const UI32 bytesPerPtr, const UI32 ammPtrs);
BOOL NBDataPtrsPool_addDataPtrs(STNBDataPtrsPoolRef ref, STNBDataPtr* ptrs, const UI32 ptrsSz);
BOOL NBDataPtrsPool_addPtr(STNBDataPtrsPoolRef ref, void* ptr, const UI32 size);
UI32 NBDataPtrsPool_getPtrs(STNBDataPtrsPoolRef ref, const UI32 minBytesPerPtr, STNBDataPtr* srcAndDst, const UI32 srcAndDstSz);

#ifdef __cplusplus
} //extern "C"
#endif
		
#endif /* NBDataPtr_h */
