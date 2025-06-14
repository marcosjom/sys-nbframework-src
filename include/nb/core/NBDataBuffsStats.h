
#ifndef NBDataBuffsStats_h
#define NBDataBuffsStats_h

#include "nb/NBFrameworkDefs.h"
//
#include "nb/NBObject.h"
#include "nb/core/NBString.h"
#include "nb/core/NBLog.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct STNBDataBuffsStatsBuffers_ {
	UI32	count;
	UI64	bytesCount;
	//wired
	struct {
		UI32	count;
		UI64	bytesCount;
	} wired;
} STNBDataBuffsStatsBuffers;

typedef struct STNBDataBuffsStatsState_ {
	STNBDataBuffsStatsBuffers		buffs;
	UI64							updCalls;
} STNBDataBuffsStatsState;

typedef struct STNBDataBuffsStatsData_ {
	STNBDataBuffsStatsState	loaded;		//loaded
	struct {
		STNBDataBuffsStatsState added;		//added since last reset
		STNBDataBuffsStatsState removed;	//removed since last reset
	} accum;
	struct {
		STNBDataBuffsStatsState added;		//added since start
		STNBDataBuffsStatsState removed;	//removed since start
	} total;
} STNBDataBuffsStatsData;

//Update

typedef struct STNBDataBuffsStatsUpdItm_ {
	UI32 count;
	UI64 bytes;
} STNBDataBuffsStatsUpdItm;

typedef struct STNBDataBuffsStatsUpd_ {
	STNBDataBuffsStatsUpdItm	alloc;		//allocted by the caller
	STNBDataBuffsStatsUpdItm	wired;		//tranfered temporary ownership to the caller
	STNBDataBuffsStatsUpdItm	unwired;	//returned temporary ownership by the caller
	STNBDataBuffsStatsUpdItm	freed;		//freed by the caller
} STNBDataBuffsStatsUpd;

//------------------
//- NBDataBuffsStats
//------------------

NB_OBJREF_HEADER(NBDataBuffsStats)

//

STNBDataBuffsStatsData NBDataBuffsStats_getData(STNBDataBuffsStatsRef ref, const BOOL resetAccum);
void NBDataBuffsStats_concat(STNBDataBuffsStatsRef ref, const ENNBLogLevel logLvl, const BOOL loaded, const BOOL accumAdded, const BOOL accumRemoved, const BOOL totalAdded, const BOOL totalRemoved, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, STNBString* dst, const BOOL resetAccum);
void NBDataBuffsStats_concatData(const STNBDataBuffsStatsData* obj, const ENNBLogLevel logLvl, const BOOL loaded, const BOOL accumAdded, const BOOL accumRemoved, const BOOL totalAdded, const BOOL totalRemoved, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, STNBString* dst);
void NBDataBuffsStats_concatState(const STNBDataBuffsStatsState* obj, const ENNBLogLevel logLvl, const BOOL isLoadedRecord, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, STNBString* dst);
void NBDataBuffsStats_concatBuffers(const STNBDataBuffsStatsBuffers* obj, const ENNBLogLevel logLvl, const BOOL isLoadedRecord, const BOOL ignoreEmpties, STNBString* dst);

//Buffers
void NBDataBuffsStats_buffsUpdated(STNBDataBuffsStatsRef ref, const STNBDataBuffsStatsUpd* upd);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
