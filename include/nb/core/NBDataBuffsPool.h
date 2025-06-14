//
//  NBDataBuffsPool.h
//  thinstream
//
//  Created by Marcos Ortega on 9/3/19.
//

#ifndef NBDataBuffsPool_h
#define NBDataBuffsPool_h

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBLog.h"
#include "nb/core/NBDataChunk.h"
#include "nb/core/NBDataBuff.h"
#include "nb/core/NBDataBuffsStats.h"

#ifdef __cplusplus
extern "C" {
#endif

//CfgBuffers

typedef struct STNBDataBuffsPoolCfg_ {
	char*			minAlloc;		//string representing a number-bytes and oprional-sufix, like "1", "1K", "1kb", "1M", "1G", "1Tb".
	char*			maxKeep;		//string representing a number-bytes and oprional-sufix, like "1", "1K", "1kb", "1M", "1G", "1Tb".
	char*			perBuffer;		//string representing a number-bytes and oprional-sufix, like "1", "1K", "1kb", "1M", "1G", "1Tb".
	//Loaded in runtime
	UI64			minAllocBytes;	//if zero, will be interpreted from the string version.
	UI64			maxKeepBytes;	//if zero, will be interpreted from the string version.
	UI32			perBufferBytes;	//if zero, will be interpreted from the string version.
} STNBDataBuffsPoolCfg;

const STNBStructMap* NBDataBuffsPoolCfg_getSharedStructMap(void);

//---------------
//- DataBuffsPool
//---------------

NB_OBJREF_HEADER(NBDataBuffsPool)

//
BOOL NBDataBuffsPool_setStatsProvider(STNBDataBuffsPoolRef ref, STNBDataBuffsStatsRef provider);
BOOL NBDataBuffsPool_setCfg(STNBDataBuffsPoolRef ref, const STNBDataBuffsPoolCfg* cfg);
BOOL NBDataBuffsPool_prepare(STNBDataBuffsPoolRef ref); //create buffers

//Actions
BOOL NBDataBuffsPool_addAvailableBuffer(STNBDataBuffsPoolRef ref, STNBDataBuffRef buff);				//send the buffer to be managed outside this factory
STNBDataBuffRef NBDataBuffsPool_getAvailableBuffer(STNBDataBuffsPoolRef ref, BOOL* dstForbidCreateOwn);	//request a buffer managed from outside this factory

#ifdef __cplusplus
} //extern "C"
#endif
		
#endif /* NBDataBuffsPool_h */
