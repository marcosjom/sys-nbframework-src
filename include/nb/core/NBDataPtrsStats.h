
#ifndef NBDataPtrsStats_h
#define NBDataPtrsStats_h

#include "nb/NBFrameworkDefs.h"
//
#include "nb/NBObject.h"
#include "nb/core/NBString.h"
#include "nb/core/NBLog.h"
#include "nb/core/NBStructMap.h"
#ifdef __cplusplus
extern "C" {
#endif

//Buffers

typedef struct STNBDataPtrsStatsCfg_ {
	ENNBLogLevel	statsLevel;
} STNBDataPtrsStatsCfg;

const STNBStructMap* NBDataPtrsStatsCfg_getSharedStructMap(void);

// NBDataPtrsStatsBuffer

typedef struct STNBDataPtrsStatsBuffer_ {
    UI32    count;
    UI64    bytesCount;
} STNBDataPtrsStatsBuffer;

const STNBStructMap* NBDataPtrsStatsBuffer_getSharedStructMap(void);

// NBDataPtrsStatsBuffer

typedef struct STNBDataPtrsStatsStacks_ {
    UI32    resizesCount;
    UI32    swapsCount;
} STNBDataPtrsStatsStacks;

const STNBStructMap* NBDataPtrsStatsStacks_getSharedStructMap(void);

//NBDataPtrsStatsBuffers

typedef struct STNBDataPtrsStatsBuffers_ {
    STNBDataPtrsStatsBuffer cur;    //cur (at the pool)
    STNBDataPtrsStatsBuffer min;    //min (at the pool)
    STNBDataPtrsStatsBuffer max;    //max (at the pool)
    STNBDataPtrsStatsStacks stacks; //stackSwaps (incoming and outging stacks swapping, locks optimization)
    STNBDataPtrsStatsBuffer alloc;  //allocated (by the pool)
    STNBDataPtrsStatsBuffer freed;  //freed (by the pool)
    STNBDataPtrsStatsBuffer given; //provided (from the pool to user)
    STNBDataPtrsStatsBuffer taken;  //received (from user to pool)
} STNBDataPtrsStatsBuffers;

const STNBStructMap* NBDataPtrsStatsBuffers_getSharedStructMap(void);

//NBDataPtrsStatsState

typedef struct STNBDataPtrsStatsState_ {
	STNBDataPtrsStatsBuffers	buffs;
	UI64						updCalls;
} STNBDataPtrsStatsState;

const STNBStructMap* NBDataPtrsStatsState_getSharedStructMap(void);

//NBDataPtrsStatsData

typedef struct STNBDataPtrsStatsData_ {
	STNBDataPtrsStatsState	loaded;		//loaded
	STNBDataPtrsStatsState	accum;		//accum
	STNBDataPtrsStatsState	total;		//total
} STNBDataPtrsStatsData;

const STNBStructMap* NBDataPtrsStatsData_getSharedStructMap(void);

//NBDataPtrsStatsUpdItm

typedef struct STNBDataPtrsStatsUpdItm_ {
	UI32 count;
	UI64 bytes;
} STNBDataPtrsStatsUpdItm;

const STNBStructMap* NBDataPtrsStatsUpdItm_getSharedStructMap(void);

//NBDataPtrsStatsUpd

typedef struct STNBDataPtrsStatsUpd_ {
	STNBDataPtrsStatsUpdItm	alloc;		//allocted by the caller
	STNBDataPtrsStatsUpdItm	freed;		//deallocted by the caller
	STNBDataPtrsStatsUpdItm	given;		//tranfered temporary ownership to the caller
	STNBDataPtrsStatsUpdItm	taken;		//returned temporary ownership by the caller
	UI32					stackResize;
	UI32					stacksSwaps;
} STNBDataPtrsStatsUpd;

const STNBStructMap* NBDataPtrsStatsUpd_getSharedStructMap(void);

//------------------
//- NBDataPtrsStats
//------------------

NB_OBJREF_HEADER(NBDataPtrsStats)

//

STNBDataPtrsStatsData NBDataPtrsStats_getData(STNBDataPtrsStatsRef ref, const BOOL resetAccum);
void NBDataPtrsStats_concat(STNBDataPtrsStatsRef ref, const STNBDataPtrsStatsCfg* cfg, const BOOL loaded, const BOOL accum, const BOOL total, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, STNBString* dst, const BOOL resetAccum);
void NBDataPtrsStats_concatData(const STNBDataPtrsStatsData* obj, const STNBDataPtrsStatsCfg* cfg, const BOOL loaded, const BOOL accum, const BOOL total, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, STNBString* dst);
void NBDataPtrsStats_concatState(const STNBDataPtrsStatsState* obj, const STNBDataPtrsStatsCfg* cfg, const BOOL isLoadedRecord, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, STNBString* dst);
void NBDataPtrsStats_concatBuffers(const STNBDataPtrsStatsBuffers* obj, const STNBDataPtrsStatsCfg* cfg, const BOOL isLoadedRecord, const BOOL ignoreEmpties, STNBString* dst);

//Buffers
void NBDataPtrsStats_buffsUpdated(STNBDataPtrsStatsRef ref, const STNBDataPtrsStatsUpd* upd);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
