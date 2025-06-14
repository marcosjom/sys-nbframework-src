//
//  NBThreadsPool.h
//  thinstream
//
//  Created by Marcos Ortega on 9/3/19.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBThreadsPool.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBMngrStructMaps.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBThreadCond.h"
#include "nb/core/NBArray.h"

//NBThreadsPoolCfg

STNBStructMapsRec STNBThreadsPoolCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBThreadsPoolCfg_getSharedStructMap(void){
	NBMngrStructMaps_lock(&STNBThreadsPoolCfg_sharedStructMap);
	if(STNBThreadsPoolCfg_sharedStructMap.map == NULL){
		STNBThreadsPoolCfg s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBThreadsPoolCfg);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addUIntM(map, s, minAlloc); //amount of threads to allocate at start
		NBStructMap_addUIntM(map, s, maxKeep); //amount of threads to keep max
		//
		STNBThreadsPoolCfg_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&STNBThreadsPoolCfg_sharedStructMap);
	return STNBThreadsPoolCfg_sharedStructMap.map;
}

//--------------------
//- NBThreadsPoolThread
//--------------------

typedef struct STNBThreadsPoolTask_ {
	ENNBThreadsTaskType		type;
	ENNBThreadsTaskPriority	priority;
    //thread (will be execute once)
    struct {
        NBThreadPoolRoutinePtr routine;
        NBThreadPoolCleanupPtr cleanup;
    } task;
	//timer (will be repetead untill returns FALSE or is explicitly removed)
    struct {
        UI64                uid;
        NBThreadPoolTimerTickPtr routine;
        STNBTimestampMicro  lastTick;
        BOOL                isFirstTickDone; //to calculate 'usNextTick' based on 'msPerTick'
        UI64                usNextTick;
        UI64                msPerTick;
    } timer;
	void*					param;
} STNBThreadsPoolTask;

//NBThreadsPoolThread

typedef struct STNBThreadsPoolThread_ {
    struct STNBThreadsPoolOpq_* pool;   //parent
	STNBThread			thread;			//thread obj
    STNBStopFlagRef		stopFlag;		//exit thread
	BOOL				stopAfterTask;	//run one task only
	BOOL				isRunning;		//thread is running (waiting for tasks and executing them)
    //curTask
    struct {
        ENNBThreadsTaskType type;       //determines if is currently bussy or not
        UI64            timerId;        //non-zero if the task is a timer
        BOOL            timerRemoved;   //current timer was locked-removed while in unlocked-execution.
    } curTask;
	//
	STNBThreadMutex		mutex;			//mutex
	STNBThreadCond		cond;			//cond
	//tasks queue
	STNBArray			queue;			//STNBThreadsPoolTask
} STNBThreadsPoolThread;

void NBThreadsPoolThread_init(STNBThreadsPoolThread* obj);
void NBThreadsPoolThread_release(STNBThreadsPoolThread* obj);

//--------------------
//- NBThreadsPool
//--------------------

typedef struct STNBThreadsPoolOpq_ {
	STNBObject				prnt;	    //parent
    STNBStopFlagRef         stopFlag;   //exit thread
	STNBThreadsPoolCfg		cfg;
    //timers
    struct {
        UI64                iSeq;   //unique-ids generation
    } timers;
	//threads
	struct {
        STNBThreadMutex     mutex;
        STNBThreadCond      cond;
		STNBArray			arr;	//STNBThreadsPoolThread* (greather are kept: 'cfg.maxKeep' or 'threads.keepReqs.total')
        SI32                runningCount;
		//keepReqs (threads keep requests)
		struct {
			UI32			total;	//ammount of threads to keep by active tasks
		} keepReqs;
	} threads;
} STNBThreadsPoolOpq;

NB_OBJREF_BODY(NBThreadsPool, STNBThreadsPoolOpq, NBObject)

//

//------------------
//- NBThreadsPoolThread
//------------------

void NBThreadsPoolThread_init(STNBThreadsPoolThread* obj){
	NBMemory_setZeroSt(*obj, STNBThreadsPoolThread);
	obj->curTask.type   = ENNBThreadsTaskType_Count;
    {
        obj->stopFlag   = NBStopFlag_alloc(NULL);
    }
	NBThread_init(&obj->thread);
	NBThread_setIsJoinable(&obj->thread, FALSE);
	NBThreadMutex_init(&obj->mutex);
	NBThreadCond_init(&obj->cond);
	NBArray_init(&obj->queue, sizeof(STNBThreadsPoolTask), NULL);
}

void NBThreadsPoolThread_release(STNBThreadsPoolThread* obj){
	NBASSERT(!obj->isRunning)	//should be not-in-use
	NBASSERT(obj->curTask.type == ENNBThreadsTaskType_Count) //should be not-in-use
	NBASSERT(obj->queue.use == 0) //should be not-in-use
    if(NBStopFlag_isSet(obj->stopFlag)){
        NBStopFlag_release(&obj->stopFlag);
        NBStopFlag_null(&obj->stopFlag);
    }
	NBArray_release(&obj->queue);
	NBThread_release(&obj->thread);
	NBThreadMutex_release(&obj->mutex);
	NBThreadCond_release(&obj->cond);
}

//---------------
//- NBThreadsPool
//---------------

SI64 NBThreadsPoolThread_waitForTasksAndRun_(STNBThread* t, void* param);

void NBThreadsPool_initZeroed(STNBObject* obj){
	STNBThreadsPoolOpq* opq = (STNBThreadsPoolOpq*)obj;
    {
        opq->stopFlag   = NBStopFlag_alloc(NULL);
    }
    //threads
    {
        NBThreadMutex_init(&opq->threads.mutex);
        NBThreadCond_init(&opq->threads.cond);
        NBArray_init(&opq->threads.arr, sizeof(STNBThreadsPoolThread*), NULL);
    }
}

void NBThreadsPool_uninitLocked(STNBObject* obj){
	STNBThreadsPoolOpq* opq = (STNBThreadsPoolOpq*)obj;
    //stop-signal all threads
    if(NBStopFlag_isSet(opq->stopFlag)){
        NBStopFlag_activate(opq->stopFlag);
    }
	//threads
	{
        //broadcast all
        {
            SI32 i; for(i = 0; i < opq->threads.arr.use; i++){
                STNBThreadsPoolThread* itm = NBArray_itmValueAtIndex(&opq->threads.arr, STNBThreadsPoolThread*, i);
                NBThreadMutex_lock(&itm->mutex);
                {
                    if(NBStopFlag_isSet(itm->stopFlag)){
                        NBStopFlag_activate(itm->stopFlag);
                    }
                    NBThreadCond_broadcast(&itm->cond);
                }
                NBThreadMutex_unlock(&itm->mutex);
            }
        }
        //wait for all
        {
            SI32 i; for(i = 0; i < opq->threads.arr.use; i++){
                STNBThreadsPoolThread* itm = NBArray_itmValueAtIndex(&opq->threads.arr, STNBThreadsPoolThread*, i);
                NBThreadMutex_lock(&itm->mutex);
                {
                    while(itm->isRunning){
                        if(NBStopFlag_isSet(itm->stopFlag)){
                            NBStopFlag_activate(itm->stopFlag);
                        }
                        NBThreadCond_broadcast(&itm->cond);
                        NBThreadCond_wait(&itm->cond, &itm->mutex);
                    }
                }
                NBThreadMutex_unlock(&itm->mutex);
                NBThreadsPoolThread_release(itm);
                NBMemory_free(itm);
            }
        }
        //wait for 'runningCount' to be zero (all threads exit)
        {
            NBThreadMutex_lock(&opq->threads.mutex);
            while(opq->threads.runningCount > 0){
                NBThreadCond_wait(&opq->threads.cond, &opq->threads.mutex);
            }
            NBThreadMutex_unlock(&opq->threads.mutex);
        }
        NBASSERT(opq->threads.runningCount == 0)
		NBArray_empty(&opq->threads.arr);
		NBArray_release(&opq->threads.arr);
	}
    if(NBStopFlag_isSet(opq->stopFlag)){
        NBStopFlag_release(&opq->stopFlag);
        NBStopFlag_null(&opq->stopFlag);
    }
	//Cfg
	{
		NBStruct_stRelease(NBThreadsPoolCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
	}
    NBThreadCond_release(&opq->threads.cond);
    NBThreadMutex_release(&opq->threads.mutex);
}

//

BOOL NBThreadsPool_setCfg(STNBThreadsPoolRef ref, const STNBThreadsPoolCfg* cfg){
	BOOL r = FALSE;
	STNBThreadsPoolOpq* opq = (STNBThreadsPoolOpq*)ref.opaque;
	NBASSERT(NBThreadsPool_isClass(ref))
	NBObject_lock(opq);
	{
		r = TRUE;
		//Validate
		{
			//Nothing
		}
		//Apply
		if(r){
			NBStruct_stRelease(NBThreadsPoolCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
			if(cfg != NULL){
				NBStruct_stClone(NBThreadsPoolCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
			}
		}
	}
	NBObject_unlock(opq);
	return r;
}

SI64 NBThreadsPoolThread_waitForTasksAndRun_(STNBThread* t, void* param){
	SI64 r = 0;
	STNBThreadsPoolThread* itm = (STNBThreadsPoolThread*)param;
	NBThreadMutex_lock(&itm->mutex);
	{
		//flag as running
		{
			NBASSERT(itm->isRunning)
            NBASSERT(itm->pool->threads.runningCount > 0)
			NBASSERT(itm->curTask.type == ENNBThreadsTaskType_Count)
			itm->isRunning = TRUE;
			NBThreadCond_broadcast(&itm->cond);
		}
		//wait for tasks and run them
		while(!NBStopFlag_isAnyActivated(itm->stopFlag) || itm->queue.use != 0){ //all queued tasks must be completed (user-program-logic integrity)
			NBASSERT(itm->isRunning)
			NBASSERT(itm->curTask.type == ENNBThreadsTaskType_Count)
			if(itm->queue.use == 0){
				NBThreadCond_timedWait(&itm->cond, &itm->mutex, 100);
			} else {
                BOOL anyTaskRan = FALSE; UI32 timersCount = 0; UI64 usNextTimerMin = 0;
                SI32 i2; for(i2 = 0; i2 < itm->queue.use && !anyTaskRan; i2++){
                    STNBThreadsPoolTask task = NBArray_itmValueAtIndex(&itm->queue, STNBThreadsPoolTask, i2);
                    //Execute
                    if(task.timer.uid == 0){
                        //Task (thread, run once)
                        anyTaskRan = TRUE;
                        NBArray_removeItemAtIndex(&itm->queue, i2);
                        //flag as bussy
                        {
                            NBASSERT(itm->curTask.type == ENNBThreadsTaskType_Count)
                            itm->curTask.type           = task.type; NBASSERT(task.type >= 0 && task.type < ENNBThreadsTaskType_Count)
                            itm->curTask.timerId        = task.timer.uid; NBASSERT(task.timer.uid == 0 && task.task.routine != NULL)
                            itm->curTask.timerRemoved   = FALSE;
                            NBThreadCond_broadcast(&itm->cond);
                        }
                        //Execute task (unlocked)
                        {
                            NBASSERT(task.task.routine != NULL)
                            NBThreadMutex_unlock(&itm->mutex);
                            {
                                /*const SI64 r2 = */(*task.task.routine)(task.param);
                            }
                            NBThreadMutex_lock(&itm->mutex);
                        }
                        //flag as not-bussy
                        {
                            NBASSERT(itm->curTask.type < ENNBThreadsTaskType_Count)
                            itm->curTask.type           = ENNBThreadsTaskType_Count;
                            itm->curTask.timerId        = 0;
                            itm->curTask.timerRemoved   = FALSE;
                            NBThreadCond_broadcast(&itm->cond);
                        }
                        //Cleanup (unlocked and while not-bussy so new tasks can be queued on this thread)
                        if(task.timer.uid == 0 && task.task.cleanup != NULL){
                            NBThreadMutex_unlock(&itm->mutex);
                            {
                                (*task.task.cleanup)(task.param);
                            }
                            NBThreadMutex_lock(&itm->mutex);
                        }
                    } else {
                        //Timer (run on time)
                        //Analyze timer step
                        NBASSERT(task.timer.routine != NULL)
                        const STNBTimestampMicro now = NBTimestampMicro_getMonotonicFast();
                        const SI64 usSinceLastTick = NBTimestampMicro_getDiffInUs(&task.timer.lastTick, &now);
                        BOOL doTickNow = TRUE;
                        //should tick now?
                        if(usSinceLastTick < 0){
                            //negative-us (system or program logic error)
                            task.timer.lastTick = now;
                            doTickNow = FALSE;
                        } else if(usSinceLastTick < task.timer.usNextTick){
                            //still waiting
                            const UI64 usWaitRemain = (task.timer.usNextTick - usSinceLastTick);
                            doTickNow = FALSE;
                            if(timersCount == 0){
                                usNextTimerMin = usWaitRemain;
                            } else if(usNextTimerMin > usWaitRemain){
                                usNextTimerMin = usWaitRemain;
                            }
                            timersCount++;
                        }
                        //tick
                        if(doTickNow){
                            UI64 usNextTick = 0, usAhead = 0;
                            //Task (thread, run once)
                            anyTaskRan = TRUE;
                            NBArray_removeItemAtIndex(&itm->queue, i2);
                            //flag as bussy
                            {
                                NBASSERT(itm->curTask.type == ENNBThreadsTaskType_Count)
                                itm->curTask.type           = task.type; NBASSERT(task.type >= 0 && task.type < ENNBThreadsTaskType_Count)
                                itm->curTask.timerId        = task.timer.uid; NBASSERT(task.timer.uid != 0 && task.timer.routine != NULL)
                                itm->curTask.timerRemoved   = FALSE;
                                NBThreadCond_broadcast(&itm->cond);
                            }
                            //calculate next tick
                            {
                                task.timer.lastTick = NBTimestampMicro_getMonotonicFast();
                                if(!task.timer.isFirstTickDone){
                                    usNextTick = (task.timer.msPerTick * 1000ULL);
                                } else {
                                    usAhead = (usSinceLastTick - task.timer.usNextTick);
                                    if(usAhead >= (task.timer.msPerTick * 1000ULL)){
                                        //do next-tick inmediatly after this-tick
                                        usNextTick = 0;
                                    } else {
                                        //do next-tick inmediatly after this-tick
                                        usNextTick = (task.timer.msPerTick * 1000ULL) - usAhead;
                                    }
                                }
                            }
                            //Execute timer (unlocked)
                            {
                                BOOL rr = FALSE;
                                const UI64 msSinceLast2 = (UI64)(usSinceLastTick / 1000LL);
                                UI32 msNextTick2 = (UI32)(usNextTick / 1000ULL);
                                UI32 msPerTick2 = (UI32)task.timer.msPerTick;
                                NBASSERT(task.timer.routine != NULL)
                                NBASSERT(task.timer.uid == itm->curTask.timerId)
                                {
                                    //Execute task (unlocked)
                                    NBThreadMutex_unlock(&itm->mutex);
                                    {
                                        rr = (*task.timer.routine)(msSinceLast2, &msNextTick2, &msPerTick2, task.param);
                                    }
                                    NBThreadMutex_lock(&itm->mutex);
                                }
                                NBASSERT(task.timer.routine != NULL)
                                NBASSERT(task.timer.uid == itm->curTask.timerId)
                                if(!rr || itm->curTask.timerRemoved){
                                    //end of timer
                                } else {
                                    //set next time and add again
                                    if(msNextTick2 != (UI32)(usNextTick / 1000ULL) || msPerTick2 != (UI32)task.timer.msPerTick){
                                        //start new timing
                                        task.timer.isFirstTickDone = FALSE;
                                        task.timer.usNextTick   = (msNextTick2  * 1000ULL);
                                        task.timer.msPerTick    = msPerTick2;
                                    } else {
                                        //continue timing
                                        task.timer.isFirstTickDone = TRUE;
                                        if(usAhead < (task.timer.msPerTick * 1000ULL)){
                                            task.timer.usNextTick = (task.timer.msPerTick * 1000ULL) - usAhead;
                                        } else {
                                            task.timer.usNextTick = 0;
                                        }
                                    }
                                    NBArray_addValue(&itm->queue, task);
                                }
                            }
                            //flag as not-bussy
                            {
                                NBASSERT(itm->curTask.type < ENNBThreadsTaskType_Count)
                                itm->curTask.type           = ENNBThreadsTaskType_Count;
                                itm->curTask.timerId        = 0;
                                itm->curTask.timerRemoved   = FALSE;
                                NBThreadCond_broadcast(&itm->cond);
                            }
                        }
                    }
                }
                //auto-stop thread (for reutilization)
                if(itm->queue.use == 0 && itm->stopAfterTask){
                    NBStopFlag_activate(itm->stopFlag);
                } else if(!anyTaskRan && timersCount != 0 && usNextTimerMin > 0){
                    //wait for next timer (or new task)
                    NBThreadCond_timedWait(&itm->cond, &itm->mutex, (UI32)((usNextTimerMin + 999ULL) / 1000ULL));
                }
			}
		}
		//flag as not-running
		{
			NBASSERT(itm->isRunning)
            NBASSERT(itm->pool->threads.runningCount > 0)
			NBASSERT(itm->curTask.type == ENNBThreadsTaskType_Count)
			itm->isRunning = FALSE;
			NBThreadCond_broadcast(&itm->cond);
		}
	}
	NBThreadMutex_unlock(&itm->mutex);
    //pool
    {
        STNBThreadsPoolOpq* opq = itm->pool;
        NBThreadMutex_lock(&opq->threads.mutex);
        {
            NBASSERT(opq->threads.runningCount > 0)
            if(opq->threads.runningCount > 0){
                opq->threads.runningCount--;
                NBThreadCond_broadcast(&opq->threads.cond); //notify 'runningCount' change
            }
        }
        NBThreadMutex_unlock(&opq->threads.mutex);
    }
	return r;
}

BOOL NBThreadsPool_setParentStopFlag(STNBThreadsPoolRef ref, STNBStopFlagRef* parentStopFlag){
    BOOL r = FALSE;
    STNBThreadsPoolOpq* opq = (STNBThreadsPoolOpq*)ref.opaque; NBASSERT(NBThreadsPool_isClass(ref))
    NBObject_lock(opq);
    {
        NBStopFlag_setParentFlag(opq->stopFlag, parentStopFlag);
        r = TRUE;
    }
    NBObject_unlock(opq);
    return r;
}

BOOL NBThreadsPool_prepare(STNBThreadsPoolRef ref){ //allocate threads
	BOOL r = FALSE;
	STNBThreadsPoolOpq* opq = (STNBThreadsPoolOpq*)ref.opaque; NBASSERT(NBThreadsPool_isClass(ref))
	NBObject_lock(opq);
	{
		r = TRUE;
		//Sync threads
		{
			//Release threads
			{
				const UI32 maxKeep = (opq->threads.keepReqs.total > opq->cfg.maxKeep ? opq->threads.keepReqs.total : opq->cfg.maxKeep);
				SI32 i; for(i = (SI32)opq->threads.arr.use - 1; i >= 0 && i > maxKeep; i--){
					STNBThreadsPoolThread* itm = NBArray_itmValueAtIndex(&opq->threads.arr, STNBThreadsPoolThread*, i);
					NBThreadsPoolThread_release(itm);
					NBMemory_free(itm);
					//
					NBArray_removeItemAtIndex(&opq->threads.arr, i);
				}
			}
			//Allocate threads
			while(opq->threads.arr.use < opq->cfg.minAlloc){
				STNBThreadsPoolThread* itm = NBMemory_allocType(STNBThreadsPoolThread);
				NBThreadsPoolThread_init(itm);
                {
                    itm->pool = opq;
                    NBStopFlag_setParentFlag(itm->stopFlag, &opq->stopFlag);
                    itm->isRunning = TRUE;
                    NBThreadMutex_lock(&opq->threads.mutex);
                    {
                        opq->threads.runningCount++;
                    }
                    NBThreadMutex_unlock(&opq->threads.mutex);
                }
				if(!NBThread_start(&itm->thread, NBThreadsPoolThread_waitForTasksAndRun_, itm, NULL)){
					itm->isRunning = FALSE;
                    NBThreadMutex_lock(&opq->threads.mutex);
                    {
                        NBASSERT(opq->threads.runningCount > 0)
                        opq->threads.runningCount--;
                    }
                    NBThreadMutex_unlock(&opq->threads.mutex);
					NBThreadsPoolThread_release(itm);
					NBMemory_free(itm);
					r = FALSE;
					break;
				} else {
                    NBThreadMutex_lock(&opq->threads.mutex);
                    {
                        NBArray_addValue(&opq->threads.arr, itm);
                        NBThreadCond_broadcast(&opq->threads.cond); //notify 'runningCount' change
                    }
                    NBThreadMutex_unlock(&opq->threads.mutex);
				}
			} 
		}
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBThreadsPool_isBussy(STNBThreadsPoolRef ref){ //any thread is allocated
    STNBThreadsPoolOpq* opq = (STNBThreadsPoolOpq*)ref.opaque; NBASSERT(NBThreadsPool_isClass(ref))
    return (opq->threads.runningCount > 0);
}

//Actions

BOOL NBThreadsPool_startTaskLocked_(STNBThreadsPoolOpq* opq, const ENNBThreadsTaskType type, const ENNBThreadsTaskPriority priority, NBThreadPoolRoutinePtr taskRoutine, NBThreadPoolCleanupPtr taskCleanup, const UI32 timerMsFirstTick, const UI32 timerMsPerTick, NBThreadPoolTimerTickPtr timerRoutine, void* param, BOOL* dstErrIsTemp, UI64* dstTimerId){
	BOOL r = FALSE, errIsTmp = FALSE;
	if(taskRoutine != NULL || timerRoutine != NULL){
		//search for available thread
		if(!r){
			SI32 i; for(i = 0; i < opq->threads.arr.use && !r; i++){
				STNBThreadsPoolThread* itm = NBArray_itmValueAtIndex(&opq->threads.arr, STNBThreadsPoolThread*, i);
				NBThreadMutex_lock(&itm->mutex);
				if(!itm->isRunning){
					NBThreadMutex_unlock(&itm->mutex);
					//cleanup of unused thread
					NBThreadsPoolThread_release(itm);
					NBMemory_free(itm);
					NBArray_removeItemAtIndex(&opq->threads.arr, i);
					i--;
				} else {
					if(
					   itm->queue.use == 0 //no task in queue
					   && !NBStopFlag_isAnyActivated(itm->stopFlag)
					   && !itm->stopAfterTask
					   && (
						   itm->curTask.type == ENNBThreadsTaskType_Quick	    //current task will finish soon
						   || itm->curTask.type == ENNBThreadsTaskType_Count	//no current task (free-to-use)
						   )
					   ){
						//set up task
						STNBThreadsPoolTask task;
						NBMemory_setZeroSt(task, STNBThreadsPoolTask);
						task.type		= type;
						task.priority	= priority;
                        //task (will run once)
                        if(taskRoutine != NULL){
                           task.task.routine  = taskRoutine;
                            task.task.cleanup = taskCleanup;
                        }
                        //timer (will run multiple times)
                        if(timerRoutine != NULL){
                            task.timer.uid          = ++opq->timers.iSeq;
                            task.timer.routine      = timerRoutine;
                            task.timer.lastTick     = NBTimestampMicro_getMonotonicFast();
                            task.timer.usNextTick   = (timerMsFirstTick * 1000ULL);
                            task.timer.msPerTick    = timerMsPerTick;
                            if(dstTimerId != NULL){
                                *dstTimerId = task.timer.uid;
                            }
                        }
						task.param		= param;
						NBArray_addValue(&itm->queue, task);
						NBThreadCond_broadcast(&itm->cond);
						//PRINTF_INFO("NBThreadsPool, task added  in thread #%d / %d.\n", (i + 1), opq->threads.arr.use);
						r = TRUE;
					}
					NBThreadMutex_unlock(&itm->mutex);
				}
			}
			//release non-running threads
			for(; i < opq->threads.arr.use; i++){
				STNBThreadsPoolThread* itm = NBArray_itmValueAtIndex(&opq->threads.arr, STNBThreadsPoolThread*, i);
				NBThreadMutex_lock(&itm->mutex);
				if(!itm->isRunning){
					NBThreadMutex_unlock(&itm->mutex);
					//cleanup of unused thread
					NBThreadsPoolThread_release(itm);
					NBMemory_free(itm);
					NBArray_removeItemAtIndex(&opq->threads.arr, i);
					i--;
				} else {
					NBThreadMutex_unlock(&itm->mutex);
				}
			}
		}
		//Add new thread
		if(!r){
			const UI32 maxKeep = (opq->threads.keepReqs.total > opq->cfg.maxKeep ? opq->threads.keepReqs.total : opq->cfg.maxKeep);
			STNBThreadsPoolThread* itm = NBMemory_allocType(STNBThreadsPoolThread);
			NBThreadsPoolThread_init(itm);
            {
                itm->pool = opq;
                NBStopFlag_setParentFlag(itm->stopFlag, &opq->stopFlag);
                itm->isRunning = TRUE;
                NBThreadMutex_lock(&opq->threads.mutex);
                {
                    opq->threads.runningCount++;
                }
                NBThreadMutex_unlock(&opq->threads.mutex);
            }
			{
				STNBThreadsPoolTask task;
				NBMemory_setZeroSt(task, STNBThreadsPoolTask);
				task.type		= type;
                //task (will run once)
                if(taskRoutine != NULL){
                   task.task.routine = taskRoutine;
                   task.task.cleanup = taskCleanup;
                }
                //timer (will run multiple times)
                if(timerRoutine != NULL){
                    task.timer.uid          = ++opq->timers.iSeq;
                    task.timer.routine      = timerRoutine;
                    task.timer.lastTick     = NBTimestampMicro_getMonotonicFast();
                    task.timer.usNextTick   = (timerMsFirstTick * 1000ULL);
                    task.timer.msPerTick    = timerMsPerTick;
                    if(dstTimerId != NULL){
                        *dstTimerId = task.timer.uid;
                    }
                }
				task.param		= param;
				NBArray_addValue(&itm->queue, task);
				//PRINTF_INFO("NBThreadsPool, task added in (new) thread.\n");
			}
			if(opq->threads.arr.use >= maxKeep){
				itm->stopAfterTask	= TRUE;
			}
			if(!NBThread_start(&itm->thread, NBThreadsPoolThread_waitForTasksAndRun_, itm, &errIsTmp)){
				itm->isRunning		= FALSE;
                NBThreadMutex_lock(&opq->threads.mutex);
                {
                    NBASSERT(opq->threads.runningCount > 0)
                    opq->threads.runningCount--;
                }
                NBThreadMutex_unlock(&opq->threads.mutex);
				NBArray_empty(&itm->queue);
				NBThreadsPoolThread_release(itm);
				NBMemory_free(itm);
			} else {
                NBThreadMutex_lock(&opq->threads.mutex);
                {
                    NBArray_addValue(&opq->threads.arr, itm);
                    NBThreadCond_broadcast(&opq->threads.cond); //notify 'runningCount' change
                }
                NBThreadMutex_unlock(&opq->threads.mutex);
                r = TRUE;
			}
		}
	}
	if(dstErrIsTemp != NULL){
		*dstErrIsTemp = errIsTmp;
	}
	return r;
}
	
BOOL NBThreadsPool_threadsGroupStart(STNBThreadsPoolRef ref, const UI32 ammThreads){
	BOOL r = FALSE;
	STNBThreadsPoolOpq* opq = (STNBThreadsPoolOpq*)ref.opaque; NBASSERT(NBThreadsPool_isClass(ref))
	NBObject_lock(opq);
	{
		opq->threads.keepReqs.total += ammThreads;
		r = TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBThreadsPool_threadsGroupEnd(STNBThreadsPoolRef ref, const UI32 ammThreads){
	BOOL r = FALSE;
	STNBThreadsPoolOpq* opq = (STNBThreadsPoolOpq*)ref.opaque; NBASSERT(NBThreadsPool_isClass(ref))
	NBObject_lock(opq);
	{
		NBASSERT(opq->threads.keepReqs.total >= ammThreads)
		if(opq->threads.keepReqs.total >= ammThreads){
			opq->threads.keepReqs.total -= ammThreads;
		} else {
			opq->threads.keepReqs.total = 0;
		}
		//end threads
		{
			UI32 nonTmpCount = 0; //non-tmp-threads
			const UI32 maxKeep = (opq->threads.keepReqs.total > opq->cfg.maxKeep ? opq->threads.keepReqs.total : opq->cfg.maxKeep);
			SI32 i; for(i = (SI32)opq->threads.arr.use - 1; i >= 0; i--){
				STNBThreadsPoolThread* itm = NBArray_itmValueAtIndex(&opq->threads.arr, STNBThreadsPoolThread*, i);
				NBThreadMutex_lock(&itm->mutex);
				if(!itm->isRunning){
					NBThreadMutex_unlock(&itm->mutex);
					//cleanup of unused thread
					NBThreadsPoolThread_release(itm);
					NBMemory_free(itm);
					NBArray_removeItemAtIndex(&opq->threads.arr, i);
				} else {
					//count and stop non-tmp-threads
					if(!itm->stopAfterTask){
						nonTmpCount++;
						if(nonTmpCount > maxKeep){
							if(itm->curTask.type == ENNBThreadsTaskType_Count && itm->queue.use == 0){
                                NBStopFlag_activate(itm->stopFlag);
								NBThreadCond_broadcast(&itm->cond);
							} else {
								itm->stopAfterTask = TRUE;
							}
						}
					}
					NBThreadMutex_unlock(&itm->mutex);
				}
			}
		}
		r = TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBThreadsPool_startTask(STNBThreadsPoolRef ref, const ENNBThreadsTaskType type, const ENNBThreadsTaskPriority priority, NBThreadPoolRoutinePtr routine, NBThreadPoolCleanupPtr cleanup, void* param, BOOL* dstErrIsTemp){
	BOOL r = FALSE, errIsTmp = FALSE;
	STNBThreadsPoolOpq* opq = (STNBThreadsPoolOpq*)ref.opaque; NBASSERT(NBThreadsPool_isClass(ref))
	if(routine != NULL){
		NBObject_lock(opq);
		{
			r = NBThreadsPool_startTaskLocked_(opq, type, priority, routine, cleanup, 0, 0, NULL, param, dstErrIsTemp, NULL);
		}
		NBObject_unlock(opq);
	}
	if(dstErrIsTemp != NULL){
		*dstErrIsTemp = errIsTmp;
	}
	return r;
}

typedef struct STNBThreadsPoolThreadFnd_ {
	STNBThreadsPoolThread* thread;
	UI32				idx;
} STNBThreadsPoolThreadFnd;

BOOL NBThreadsPool_queueTask(STNBThreadsPoolRef ref, const ENNBThreadsTaskType type, const ENNBThreadsTaskPriority priority, NBThreadPoolRoutinePtr routine, NBThreadPoolCleanupPtr cleanup, void* param, BOOL* dstErrIsTemp, const UI32 ammThreadsLimit){
	BOOL r = FALSE, errIsTmp = FALSE;
	STNBThreadsPoolOpq* opq = (STNBThreadsPoolOpq*)ref.opaque; NBASSERT(NBThreadsPool_isClass(ref))
	if(routine != NULL){
		NBObject_lock(opq);
		{
			STNBArray quickThreads;
			NBArray_initWithSz(&quickThreads, sizeof(STNBThreadsPoolThreadFnd), NULL, opq->threads.arr.use, 16, 0.10f);
			//identify and lock all threads running quick tasks
			{
				SI32 i; for(i = 0; i < opq->threads.arr.use && (ammThreadsLimit == 0 || quickThreads.use < ammThreadsLimit); i++){
					STNBThreadsPoolThread* itm = NBArray_itmValueAtIndex(&opq->threads.arr, STNBThreadsPoolThread*, i);
					NBThreadMutex_lock(&itm->mutex);
					if(!itm->isRunning){
						NBThreadMutex_unlock(&itm->mutex);
						//cleanup of unused thread
						NBThreadsPoolThread_release(itm);
						NBMemory_free(itm);
						NBArray_removeItemAtIndex(&opq->threads.arr, i);
						i--;
					} else {
						BOOL hasQuickTaskOnly = TRUE;
                        if(NBStopFlag_isAnyActivated(itm->stopFlag) || itm->stopAfterTask){
                            //is stopping
                            hasQuickTaskOnly = FALSE;
                        } else if(itm->curTask.type < ENNBThreadsTaskType_Count && itm->curTask.type != ENNBThreadsTaskType_Quick){
                            //is running a non-quick-task
                            hasQuickTaskOnly = FALSE;
						} else {
							SI32 i; for(i = 0; i < itm->queue.use; i++){
								STNBThreadsPoolTask* tsk = NBArray_itmPtrAtIndex(&itm->queue, STNBThreadsPoolTask, i);
								if(tsk->type != ENNBThreadsTaskType_Quick){
                                    hasQuickTaskOnly = FALSE;
									break;
								}
							}
						}
						if(!hasQuickTaskOnly){
							//unlock
							NBThreadMutex_unlock(&itm->mutex);
						} else {
							//add to list and keep locked
							STNBThreadsPoolThreadFnd fnd;
							NBMemory_setZeroSt(fnd, STNBThreadsPoolThreadFnd);
							fnd.thread	= itm;
							fnd.idx		= i;
							NBArray_addValue(&quickThreads, fnd);
						}
					}
				}
				//release other non-running threads
				for(; i < opq->threads.arr.use; i++){
					STNBThreadsPoolThread* itm = NBArray_itmValueAtIndex(&opq->threads.arr, STNBThreadsPoolThread*, i);
					NBThreadMutex_lock(&itm->mutex);
					if(!itm->isRunning){
						NBThreadMutex_unlock(&itm->mutex);
						//cleanup of unused thread
						NBThreadsPoolThread_release(itm);
						NBMemory_free(itm);
						NBArray_removeItemAtIndex(&opq->threads.arr, i);
						i--;
					} else {
						NBThreadMutex_unlock(&itm->mutex);
					}
				}
			}
			//add to less bussy thread
			if(quickThreads.use > 0){
				//find less bussy
				STNBThreadsPoolThreadFnd* lessBussy = NULL;
				SI32 i; for(i = 0; i < quickThreads.use; i++){
					STNBThreadsPoolThreadFnd* fnd = NBArray_itmPtrAtIndex(&quickThreads, STNBThreadsPoolThreadFnd, i);
                    if(lessBussy == NULL || (lessBussy->thread->queue.use + (lessBussy->thread->curTask.type != ENNBThreadsTaskType_Count ? 1: 0)) > (fnd->thread->queue.use + (fnd->thread->curTask.type != ENNBThreadsTaskType_Count ? 1: 0))){
						lessBussy = fnd;
					}
				}
				NBASSERT(lessBussy != NULL) //should be found
				//unlock threads (in reverse order) untill lessBussy lock is in top
				{
					SI32 i; for(i = (SI32)quickThreads.use - 1; i >= 0; i--){
						STNBThreadsPoolThreadFnd* fnd = NBArray_itmPtrAtIndex(&quickThreads, STNBThreadsPoolThreadFnd, i);
						if(fnd == lessBussy){
							break;
						} else {
							NBThreadMutex_unlock(&fnd->thread->mutex);
						}
					}
					NBArray_removeItemsAtIndex(&quickThreads, i + 1, quickThreads.use - i - 1);
				}
				//add task to lessBussy
				if(lessBussy != NULL){
					//set up task
					STNBThreadsPoolTask task;
					NBMemory_setZeroSt(task, STNBThreadsPoolTask);
					task.type		= type;
					task.priority	= priority;
                    //task
                    {
                        task.task.routine = routine;
                        task.task.cleanup = cleanup;
                    }
					task.param		= param;
					{
						NBASSERT(quickThreads.use > 0)
						NBASSERT(lessBussy == NBArray_itmPtrAtIndex(&quickThreads, STNBThreadsPoolThreadFnd, quickThreads.use - 1)) //lessBussy should be at top
						NBArray_addValue(&lessBussy->thread->queue, task);
						NBThreadCond_broadcast(&lessBussy->thread->cond);
					}
					r = TRUE;
					//PRINTF_INFO("NBThreadsPool, task queued in thread #%d / %d.\n", (lessBussy->idx + 1), opq->threads.arr.use);
				}
				//unlock remainig (reverse order)
				{
					SI32 i; for(i = (SI32)quickThreads.use - 1; i >= 0; i--){
						STNBThreadsPoolThreadFnd* fnd = NBArray_itmPtrAtIndex(&quickThreads, STNBThreadsPoolThreadFnd, i);
						NBThreadMutex_unlock(&fnd->thread->mutex);
					}
					NBArray_empty(&quickThreads);
				}
			}
			//Add new thread
			if(!r){
				r = NBThreadsPool_startTaskLocked_(opq, type, priority, routine, cleanup, 0, 0, NULL, param, &errIsTmp, NULL);
			}
			NBArray_release(&quickThreads);
		}
		NBObject_unlock(opq);
	}
	if(dstErrIsTemp != NULL){
		*dstErrIsTemp = errIsTmp;
	}
	return r;
}

UI64 NBThreadsPool_addTimer(STNBThreadsPoolRef ref, const ENNBThreadsTaskType type, const UI32 msFirstTick, const UI32 msPerTick, NBThreadPoolTimerTickPtr routine, void* param, BOOL* dstErrIsTemp){
    UI64 r = 0;
    BOOL errIsTmp = FALSE;
    STNBThreadsPoolOpq* opq = (STNBThreadsPoolOpq*)ref.opaque; NBASSERT(NBThreadsPool_isClass(ref))
    if(routine != NULL){
        NBObject_lock(opq);
        {
            UI64 timerId = 0;
            if(NBThreadsPool_startTaskLocked_(opq, type, ENNBThreadsTaskPriority_Default, NULL, NULL, msFirstTick, msPerTick, routine, param, dstErrIsTemp, &timerId)){
                r = timerId;
            }
        }
        NBObject_unlock(opq);
    }
    if(dstErrIsTemp != NULL){
        *dstErrIsTemp = errIsTmp;
    }
    return r;
}

BOOL NBThreadsPool_removeTimer(STNBThreadsPoolRef ref, const UI64 timerId){
    BOOL r = FALSE;
    STNBThreadsPoolOpq* opq = (STNBThreadsPoolOpq*)ref.opaque; NBASSERT(NBThreadsPool_isClass(ref))
    if(timerId > 0){
        NBObject_lock(opq);
        {
            SI32 i; for(i = (SI32)opq->threads.arr.use - 1; i >= 0 && !r; i--){
                STNBThreadsPoolThread* itm = NBArray_itmValueAtIndex(&opq->threads.arr, STNBThreadsPoolThread*, i);
                NBThreadMutex_lock(&itm->mutex);
                {
                    //remove form queue
                    SI32 i2; for(i2 = (SI32)itm->queue.use - 1; i2 >= 0; i2--){
                        STNBThreadsPoolTask* tsk = NBArray_itmPtrAtIndex(&itm->queue, STNBThreadsPoolTask, i2);
                        if(tsk->timer.uid == timerId){
                            //remove
                            NBArray_removeItemAtIndex(&itm->queue, i2);
                            r = TRUE;
                            break;
                        }
                    }
                    //remove from currently-running
                    if(itm->curTask.timerId == timerId){
                        //Timer is currently in execution, flag to be removed inmedialtly after.
                        itm->curTask.timerRemoved = TRUE;
                    }
                }
                NBThreadMutex_unlock(&itm->mutex);
            }
        }
        NBObject_unlock(opq);
    }
    return r;
}
