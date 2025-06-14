//
//  NBThreadsPool.h
//  thinstream
//
//  Created by Marcos Ortega on 9/3/19.
//

#ifndef NBThreadsPool_h
#define NBThreadsPool_h

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBLog.h"
#include "nb/core/NBMngrProcess.h"
#include "nb/core/NBThread.h"
#include "nb/core/NBStopFlag.h"

#ifdef __cplusplus
extern "C" {
#endif

//TaskType

typedef enum ENNBThreadsTaskType_ {
	ENNBThreadsTaskType_Unknown = 0,	//unknown duration (thread cannot be shared)
	ENNBThreadsTaskType_Quick,			//short duration (thread can be shared)
	ENNBThreadsTaskType_Count
} ENNBThreadsTaskType;

//TaskPriority

typedef enum ENNBThreadsTaskPriority_ {
	ENNBThreadsTaskPriority_Default = 0,	//low priority
	ENNBThreadsTaskPriority_Mid,			//medium priority
	ENNBThreadsTaskPriority_High,			//higher priority
	//
	ENNBThreadsTaskPriority_Count
} ENNBThreadsTaskPriority;

//NBThreadsPoolCfg

typedef struct STNBThreadsPoolCfg_ {
	UI32	minAlloc;	//amount of threads to allocate at start
	UI32	maxKeep;	//amount of threads to keep max
} STNBThreadsPoolCfg;

const STNBStructMap* NBThreadsPoolCfg_getSharedStructMap(void);

typedef SI64 (*NBThreadPoolRoutinePtr)(void* param); //task (will run once)
typedef BOOL (*NBThreadPoolTimerTickPtr)(const UI64 msSinceLast, UI32* msMinForNext, UI32* msPerTick, void* param); //timer (will run many times)
typedef void (*NBThreadPoolCleanupPtr)(void* param); //cleanup (when removed)

//---------------
//- DataBuffsPool
//---------------

NB_OBJREF_HEADER(NBThreadsPool)

//
BOOL NBThreadsPool_setCfg(STNBThreadsPoolRef ref, const STNBThreadsPoolCfg* cfg);
BOOL NBThreadsPool_setParentStopFlag(STNBThreadsPoolRef ref, STNBStopFlagRef* parentStopFlag);
BOOL NBThreadsPool_prepare(STNBThreadsPoolRef ref); //allocate threads
BOOL NBThreadsPool_isBussy(STNBThreadsPoolRef ref); //any thread is allocated

//thread groups
BOOL NBThreadsPool_threadsGroupStart(STNBThreadsPoolRef ref, const UI32 ammThreads);	//hint that overrites the 'cfg.maxKeep' param
BOOL NBThreadsPool_threadsGroupEnd(STNBThreadsPoolRef ref, const UI32 ammThreads);		//hint that overrites the 'cfg.maxKeep' param
//tasks
BOOL NBThreadsPool_startTask(STNBThreadsPoolRef ref, const ENNBThreadsTaskType type, const ENNBThreadsTaskPriority priority, NBThreadPoolRoutinePtr routine, NBThreadPoolCleanupPtr cleanup, void* param, BOOL* dstErrIsTemp);
BOOL NBThreadsPool_queueTask(STNBThreadsPoolRef ref, const ENNBThreadsTaskType type, const ENNBThreadsTaskPriority priority, NBThreadPoolRoutinePtr routine, NBThreadPoolCleanupPtr cleanup, void* param, BOOL* dstErrIsTemp, const UI32 ammThreadsLimit);
//timers
UI64 NBThreadsPool_addTimer(STNBThreadsPoolRef ref, const ENNBThreadsTaskType type, const UI32 msFirstTick, const UI32 msPerTick, NBThreadPoolTimerTickPtr routine, void* param, BOOL* dstErrIsTemp);
BOOL NBThreadsPool_removeTimer(STNBThreadsPoolRef ref, const UI64 timerId);

#ifdef __cplusplus
} //extern "C"
#endif
		
#endif /* NBThreadsPool_h */
