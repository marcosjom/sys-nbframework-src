
#include "nb/NBFrameworkPch.h"
#include "nb/core/NBIOPollstersPool.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBThread.h"
#include "nb/core/NBThreadCond.h"
#include "nb/core/NBMngrStructMaps.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBStopFlag.h"

//NBIOPollstersPoolTimeout

STNBStructMapsRec STNBIOPollstersPoolTimeout_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBIOPollstersPoolTimeout_getSharedStructMap(void){
    NBMngrStructMaps_lock(&STNBIOPollstersPoolTimeout_sharedStructMap);
    if(STNBIOPollstersPoolTimeout_sharedStructMap.map == NULL){
        STNBIOPollstersPoolTimeout s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBIOPollstersPoolTimeout);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, ms);
        //
        STNBIOPollstersPoolTimeout_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&STNBIOPollstersPoolTimeout_sharedStructMap);
    return STNBIOPollstersPoolTimeout_sharedStructMap.map;
}

//NBIOPollstersPoolCfg

STNBStructMapsRec STNBIOPollstersPoolCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBIOPollstersPoolCfg_getSharedStructMap(void){
    NBMngrStructMaps_lock(&STNBIOPollstersPoolCfg_sharedStructMap);
    if(STNBIOPollstersPoolCfg_sharedStructMap.map == NULL){
        STNBIOPollstersPoolCfg s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBIOPollstersPoolCfg);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, ammount); //ammount of pollsters
        NBStructMap_addStructM(map, s, timeout, NBIOPollstersPoolTimeout_getSharedStructMap());
        //
        STNBIOPollstersPoolCfg_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&STNBIOPollstersPoolCfg_sharedStructMap);
    return STNBIOPollstersPoolCfg_sharedStructMap.map;
}

//

struct STNBIOPollstersPoolOpq_;

//NBIOPollstersPoolItm

typedef struct STNBIOPollstersPoolItm_ {
    struct STNBIOPollstersPoolOpq_* pool;   //parent
    STNBIOPollsterRef       pollster;   //pollster object
    STNBIOPollsterSyncRef   pollSync;   //pollster-sync object
    BOOL                    isStarted;  //engine started
    BOOL                    isRunning;  //thread is running
    STNBStopFlagRef         stopFlag;   //itm stop flag (global stop flag can be applied)
} STNBIOPollstersPoolItm;

//NBIOPollstersPoolOpq

typedef struct STNBIOPollstersPoolOpq_ {
	STNBObject			prnt;
    STNBThreadCond      cond;       //cond
    STNBStopFlagRef     stopFlag;   //global stop flag (global stop flag can be applied)
    STNBIOPollstersPoolCfg cfg;
    //pool
    struct {
        STNBArray       arr;        //STNBIOPollstersPoolItm*
        SI32            iNext;      //next polllster to give
        SI32            startedCount; //global pollster-engine-started count
    } pool;
    //threads
    struct {
        STNBThreadsPoolRef pool;    //optional
        SI32            runningCount; //global running count
    } threads;
} STNBIOPollstersPoolOpq;

NB_OBJREF_BODY(NBIOPollstersPool, STNBIOPollstersPoolOpq, NBObject)

void NBIOPollstersPool_initZeroed(STNBObject* obj) {
	STNBIOPollstersPoolOpq* opq	= (STNBIOPollstersPoolOpq*)obj;
    NBThreadCond_init(&opq->cond);
    {
        opq->stopFlag = NBStopFlag_alloc(NULL);
    }
    //cfg
    {
        //
    }
    //pool
    {
        NBArray_initWithSz(&opq->pool.arr, sizeof(STNBIOPollstersPoolItm*), NULL, 0, 1, 0.1f);
    }
    //threads
    {
        //
    }
}

void NBIOPollstersPool_uninitLocked(STNBObject* obj){
	STNBIOPollstersPoolOpq* opq = (STNBIOPollstersPoolOpq*)obj;
    //stop-flag
    PRINTF_INFO("NBIOPollstersPool, uninitLocked.\n");
    if(NBStopFlag_isSet(opq->stopFlag)){
        NBStopFlag_activate(opq->stopFlag);
    }
    //pool
    {
        //arr
        {
            SI32 i; for(i = 0; i < opq->pool.arr.use; i++){
                STNBIOPollstersPoolItm* itm = NBArray_itmValueAtIndex(&opq->pool.arr, STNBIOPollstersPoolItm*, i);
                {
                    if(NBIOPollster_isSet(itm->pollster)){
                        NBIOPollster_release(&itm->pollster);
                        NBIOPollster_null(&itm->pollster);
                    }
                    if(NBIOPollsterSync_isSet(itm->pollSync)){
                        NBIOPollsterSync_release(&itm->pollSync);
                        NBIOPollsterSync_null(&itm->pollSync);
                    }
                    if(NBStopFlag_isSet(itm->stopFlag)){
                        NBStopFlag_release(&itm->stopFlag);
                        NBStopFlag_null(&itm->stopFlag);
                    }
                    itm->pool = NULL;
                }
                NBMemory_free(itm);
                itm = NULL;
            }
            NBArray_empty(&opq->pool.arr);
            NBArray_release(&opq->pool.arr);
        }
        NBASSERT(opq->pool.startedCount == 0) //should match
    }
    //threads
    {
        if(NBThreadsPool_isSet(opq->threads.pool)){
            NBThreadsPool_release(&opq->threads.pool);
            NBThreadsPool_null(&opq->threads.pool);
        }
        NBASSERT(opq->threads.runningCount == 0) //should match
    }
    if(NBStopFlag_isSet(opq->stopFlag)){
        NBStopFlag_release(&opq->stopFlag);
        NBStopFlag_null(&opq->stopFlag);
    }
    //cfg
    {
        NBStruct_stRelease(NBIOPollstersPoolCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
    }
    //
    NBThreadCond_release(&opq->cond);
}

//cfg

BOOL NBIOPollstersPool_setThreadsPool(STNBIOPollstersPoolRef ref, STNBThreadsPoolRef threadsPool){ //run on shared threads owned by the pool (optimization)
    BOOL r = FALSE;
    STNBIOPollstersPoolOpq* opq = (STNBIOPollstersPoolOpq*)ref.opaque; NBASSERT(NBIOPollstersPool_isClass(ref))
    NBObject_lock(opq);
    if(opq->threads.runningCount == 0){
        NBThreadsPool_set(&opq->threads.pool, &threadsPool);
        r = TRUE;
    }
    NBObject_unlock(opq);
    return r;
}

//prepare

BOOL NBIOPollstersPool_setParentStopFlag(STNBIOPollstersPoolRef ref, STNBStopFlagRef* parentStopFlag){
    BOOL r = FALSE;
    STNBIOPollstersPoolOpq* opq = (STNBIOPollstersPoolOpq*)ref.opaque; NBASSERT(NBIOPollstersPool_isClass(ref))
    NBObject_lock(opq);
    if(opq->pool.arr.use == 0){
        NBStopFlag_setParentFlag(opq->stopFlag, parentStopFlag);
        r = TRUE;
    }
    NBObject_unlock(opq);
    return r;
}

BOOL NBIOPollstersPool_prepare(STNBIOPollstersPoolRef ref, const STNBIOPollstersPoolCfg* cfg){
    BOOL r = FALSE;
    STNBIOPollstersPoolOpq* opq = (STNBIOPollstersPoolOpq*)ref.opaque; NBASSERT(NBIOPollstersPool_isClass(ref))
    NBObject_lock(opq);
    if(opq->pool.arr.use == 0 && cfg != NULL && cfg->ammount > 0){
        r = TRUE;
        //
        NBStopFlag_reset(opq->stopFlag);
        //pollsters
        if(r){
            UI32 i; for(i = 0; i < cfg->ammount; i++){
                STNBIOPollstersPoolItm* itm = NBMemory_allocType(STNBIOPollstersPoolItm);
                NBMemory_setZeroSt(*itm, STNBIOPollstersPoolItm);
                itm->pool = opq;
                {
                    itm->stopFlag = NBStopFlag_alloc(NULL);
                    NBStopFlag_setParentFlag(itm->stopFlag, &opq->stopFlag);
                }
                //pollster
                {
                    itm->pollster = NBIOPollster_alloc(NULL);
                }
                //pollsync
                {
                    itm->pollSync   = NBIOPollsterSync_alloc(NULL);
                }
                NBArray_addValue(&opq->pool.arr, itm);
            }
        }
        //cfg
        if(r){
            NBStruct_stRelease(NBIOPollstersPoolCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
            NBStruct_stClone(NBIOPollstersPoolCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
        }
    }
    NBObject_unlock(opq);
    return r;
}

//provider

STNBIOPollsterRef        NBIOPollstersPool_getPollster_(void* usrData); //returns a pollster to use
STNBIOPollsterSyncRef    NBIOPollstersPool_getPollsterSync_(void* usrData); //returns a pollster to use

BOOL NBIOPollstersPool_linkToProvider(STNBIOPollstersPoolRef ref, STNBIOPollstersProviderRef* dst){ //configures the provider to this NBIOPollstersPool
    BOOL r = FALSE;
    STNBIOPollstersPoolOpq* opq = (STNBIOPollstersPoolOpq*)ref.opaque; NBASSERT(NBIOPollstersPool_isClass(ref))
    if(!NBStopFlag_isAnyActivated(opq->stopFlag) && dst != NULL && NBIOPollstersProvider_isSet(*dst)){
        //do not lock (to reduce unecesary risk of mutual locks)
        {
            STNBIOPollstersProviderItf itf;
            NBMemory_setZeroSt(itf, STNBIOPollstersProviderItf);
            itf.getPollster         = NBIOPollstersPool_getPollster_;
            itf.getPollsterSync     = NBIOPollstersPool_getPollsterSync_;
            NBIOPollstersProvider_setItf(*dst, &itf, opq);
            r = TRUE;
        }
    }
    return r;
}

STNBIOPollsterRef NBIOPollstersPool_getPollster_(void* usrData){ //returns a pollster to use
    STNBIOPollstersPoolOpq* opq = (STNBIOPollstersPoolOpq*)usrData; NBASSERT(NBIOPollstersPool_isClass(NBObjRef_fromOpqPtr(opq)))
    STNBIOPollsterRef r;
    NBMemory_setZeroSt(r, STNBIOPollsterRef);
    NBObject_lock(opq);
    if(!NBStopFlag_isAnyActivated(opq->stopFlag) && opq->pool.arr.use > 0){
        opq->pool.iNext = (opq->pool.iNext + 1) % opq->pool.arr.use;
        {
            STNBIOPollstersPoolItm* itm = NBArray_itmValueAtIndex(&opq->pool.arr, STNBIOPollstersPoolItm*, opq->pool.iNext);
            r = itm->pollster;
        }
    }
    NBObject_unlock(opq);
    return r;
}

STNBIOPollsterSyncRef NBIOPollstersPool_getPollsterSync_(void* usrData){ //returns a pollster to use
    STNBIOPollstersPoolOpq* opq = (STNBIOPollstersPoolOpq*)usrData; NBASSERT(NBIOPollstersPool_isClass(NBObjRef_fromOpqPtr(opq)))
    STNBIOPollsterSyncRef r;
    NBMemory_setZeroSt(r, STNBIOPollsterSyncRef);
    NBObject_lock(opq);
    if(!NBStopFlag_isAnyActivated(opq->stopFlag) && opq->pool.arr.use > 0){
        opq->pool.iNext = (opq->pool.iNext + 1) % opq->pool.arr.use;
        {
            STNBIOPollstersPoolItm* itm = NBArray_itmValueAtIndex(&opq->pool.arr, STNBIOPollstersPoolItm*, opq->pool.iNext);
            r = itm->pollSync;
        }
    }
    NBObject_unlock(opq);
    return r;
}

//run

//own-thread execution

SI64 NBIOPollstersPool_itmRunMethod_(STNBThread* thread, void* param);
void NBIOPollstersPool_itmRunEnd_(STNBIOPollstersPoolItm* itm);

//poll-threads execution

//ToDo: remove
//SI64 NBIOPollstersPool_itmPooledRunMethod_(void* param);
//void NBIOPollstersPool_itmPooledCleanupMethod_(void* param);
BOOL NBIOPollstersPool_itmPooledTickMethod_(const UI64 msSinceLast, UI32* msMinForNext, UI32* msPerTick, void* param);

BOOL NBIOPollstersPool_startThreads(STNBIOPollstersPoolRef ref){
    BOOL r = FALSE;
    STNBIOPollstersPoolOpq* opq = (STNBIOPollstersPoolOpq*)ref.opaque; NBASSERT(NBIOPollstersPool_isClass(ref))
    NBObject_lock(opq); //no need to lock
    if(opq->threads.runningCount == 0 && opq->pool.arr.use > 0){
        r = TRUE;
        //
        NBStopFlag_reset(opq->stopFlag);
        //create threads
        {
            SI32 i; for(i = 0; i < opq->pool.arr.use && r; i++){
                STNBIOPollstersPoolItm* itm = NBArray_itmValueAtIndex(&opq->pool.arr, STNBIOPollstersPoolItm*, i);
                if(itm->isStarted || itm->isRunning){
                    //error, none should be running
                    r = FALSE;
                } else if(!NBIOPollster_isSet(itm->pollster)){
                    //error, pollster should be created
                    r = FALSE;
                } else if(!NBIOPollster_engineStart(itm->pollster)){
                    //error, pollster start failed
                    r = FALSE;
                } else {
                    //
                    NBASSERT(r)
                    //
                    NBStopFlag_reset(itm->stopFlag);
                    //apply-state
                    {
                        {
                            itm->isRunning = TRUE;
                            opq->threads.runningCount++;
                        }
                        {
                            itm->isStarted = TRUE;
                            opq->pool.startedCount++;
                        }
                        NBThreadCond_broadcast(&opq->cond);
                    }
                    //start thread
                    if(NBThreadsPool_isSet(opq->threads.pool)){
                        //use threadsPool
                        const UI64 timerId = NBThreadsPool_addTimer(opq->threads.pool, ENNBThreadsTaskType_Quick, 0, 0, NBIOPollstersPool_itmPooledTickMethod_, itm, NULL);
                        if(timerId == 0){
                            r = FALSE;
                        }
                        //ToDo: remove
                        /*if(!NBThreadsPool_startTask(opq->threads.pool, ENNBThreadsTaskType_Quick, ENNBThreadsTaskPriority_Default, NBIOPollstersPool_itmPooledRunMethod_, NBIOPollstersPool_itmPooledCleanupMethod_, itm, NULL)){
                            r = FALSE;
                        }*/
                    } else {
                        //create own thread
                        STNBThread* t = NBMemory_allocType(STNBThread);
                        NBThread_init(t);
                        NBThread_setIsJoinable(t, FALSE);
                        if(!NBThread_start(t, NBIOPollstersPool_itmRunMethod_, itm, NULL)){
                            r = FALSE;
                        } else {
                            t = NULL; //consume
                        }
                        //release (if not consumed)
                        if(t != NULL){
                            NBThread_release(t);
                            NBMemory_free(t);
                            t = NULL;
                        }
                    }
                    //revert-state
                    if(!r){
                        if(itm->isStarted){
                            NBIOPollster_engineStop(itm->pollster);
                            itm->isStarted = FALSE;
                            NBASSERT(opq->pool.startedCount > 0)
                            if(opq->pool.startedCount > 0){
                                opq->pool.startedCount--;
                            }
                        }
                        //revert running-state
                        if(itm->isRunning){
                            itm->isRunning = FALSE;
                            NBASSERT(opq->threads.runningCount > 0)
                            if(opq->threads.runningCount > 0){
                                opq->threads.runningCount--;
                            }
                        }
                        NBThreadCond_broadcast(&opq->cond);
                    }
                }
            }
        }
        //revert threads (if error)
        if(!r){
            //set stop-flags
            {
                SI32 i; for(i = 0; i < opq->pool.arr.use; i++){
                    STNBIOPollstersPoolItm* itm = NBArray_itmValueAtIndex(&opq->pool.arr, STNBIOPollstersPoolItm*, i);
                    if(NBStopFlag_isSet(itm->stopFlag)){
                        NBStopFlag_activate(itm->stopFlag);
                    }
                }
            }
            //wait for threads (unlocked)
            {
                while(opq->threads.runningCount > 0){
                    NBThreadCond_waitObj(&opq->cond, opq);
                }
                NBASSERT(opq->pool.startedCount == 0)
            }
        }
    }
    NBObject_unlock(opq);
    return r;
}

//own-thread execution

void NBIOPollstersPool_itmRunEnd_(STNBIOPollstersPoolItm* itm){
    STNBIOPollstersPoolOpq* opq = itm->pool;
    NBObject_lock(opq);
    {
        //
        if(itm->isStarted){
            NBIOPollster_engineStop(itm->pollster);
            itm->isStarted = FALSE;
            NBASSERT(opq->pool.startedCount > 0)
            if(opq->pool.startedCount > 0){
                opq->pool.startedCount--;
            }
            NBThreadCond_broadcast(&opq->cond);
        }
        //
        NBASSERT(itm->isRunning)
        itm->isRunning = FALSE;
        NBASSERT(opq->threads.runningCount > 0)
        if(opq->threads.runningCount > 0){
            opq->threads.runningCount--;
            NBThreadCond_broadcast(&opq->cond);
        }
    }
    NBObject_unlock(opq);
}

SI64 NBIOPollstersPool_itmRunMethod_(STNBThread* thread, void* param){
    SI64 r = 0;
    STNBIOPollstersPoolItm* itm = (STNBIOPollstersPoolItm*)param;
    STNBIOPollstersPoolOpq* opq = itm->pool;
    //start
    {
        NBASSERT(opq->threads.runningCount > 0) //at least this thread should be counted as runnning
        NBASSERT(opq->pool.startedCount > 0) //at least this thread should be counted as runnning
        NBASSERT(itm->isRunning) //must be flaged by parent
        NBASSERT(NBIOPollster_isSet(itm->pollster))
    }
    //cycle
    while(!NBStopFlag_isAnyActivated(itm->stopFlag)){
        //ToDo: implement msTimeout as config
        NBIOPollster_enginePoll(itm->pollster, opq->cfg.timeout.ms, itm->pollSync);
    }
    //end
    {
        NBIOPollstersPool_itmRunEnd_(itm);
    }
    //release thread
    if(thread != NULL){
        NBThread_release(thread);
        NBMemory_free(thread);
        thread = NULL;
    }
    return r;
}

//poll-threads execution

BOOL NBIOPollstersPool_itmPooledTickMethod_(const UI64 msSinceLast, UI32* msMinForNext, UI32* msPerTick, void* param){
    BOOL r = FALSE;
    STNBIOPollstersPoolItm* itm = (STNBIOPollstersPoolItm*)param;
    STNBIOPollstersPoolOpq* opq = itm->pool;
    //validate-state
    {
        NBASSERT(opq->threads.runningCount > 0) //at least this thread should be counted as runnning
        NBASSERT(opq->pool.startedCount > 0) //at least this thread should be counted as runnning
        NBASSERT(itm->isRunning) //must be flaged by parent
        NBASSERT(NBIOPollster_isSet(itm->pollster))
        NBASSERT(NBThreadsPool_isSet(opq->threads.pool))
    }
    //tick
    if(!NBStopFlag_isAnyActivated(itm->stopFlag)){
        //ToDo: implement msTimeout as config
        NBIOPollster_enginePoll(itm->pollster, opq->cfg.timeout.ms, itm->pollSync);
        r = TRUE;
    } else {
        NBIOPollstersPool_itmRunEnd_(itm);
        r = FALSE; //stopping (remove timer)
    }
    return r;
}

//ToDO: remove
/*
SI64 NBIOPollstersPool_itmPooledRunMethod_(void* param){
    SI64 r = 0;
    STNBIOPollstersPoolItm* itm = (STNBIOPollstersPoolItm*)param;
    STNBIOPollstersPoolOpq* opq = itm->pool;
    //validate-state
    {
        NBASSERT(opq->threads.runningCount > 0) //at least this thread should be counted as runnning
        NBASSERT(opq->pool.startedCount > 0) //at least this thread should be counted as runnning
        NBASSERT(itm->isRunning) //must be flaged by parent
        NBASSERT(NBIOPollster_isSet(itm->pollster))
        NBASSERT(NBThreadsPool_isSet(opq->threads.pool))
    }
    //tick
    if(!NBStopFlag_isAnyActivated(itm->stopFlag)){
        //ToDo: implement msTimeout as config
        NBIOPollster_enginePoll(itm->pollster, opq->cfg.timeout.ms, itm->pollSync);
    }
    return r;
}
*/

//ToDO: remove
/*
void NBIOPollstersPool_itmPooledCleanupMethod_(void* param){
    STNBIOPollstersPoolItm* itm = (STNBIOPollstersPoolItm*)param;
    STNBIOPollstersPoolOpq* opq = itm->pool;
    //validate-state
    {
        NBASSERT(opq->threads.runningCount > 0) //at least this thread should be counted as runnning
        NBASSERT(opq->pool.startedCount > 0) //at least this thread should be counted as runnning
        NBASSERT(itm->isRunning) //must be flaged by parent
        NBASSERT(NBIOPollster_isSet(itm->pollster))
        NBASSERT(NBThreadsPool_isSet(opq->threads.pool))
    }
    //action
    if(!NBStopFlag_isAnyActivated(itm->stopFlag) && NBThreadsPool_isSet(opq->threads.pool)){
        //add again (ToDo: implement repetitive tasks in NBThreadsPoll)
        if(!NBThreadsPool_startTask(opq->threads.pool, ENNBThreadsTaskType_Quick, ENNBThreadsTaskPriority_Default, NBIOPollstersPool_itmPooledRunMethod_, NBIOPollstersPool_itmPooledCleanupMethod_, itm, NULL)){
            //end
            NBIOPollstersPool_itmRunEnd_(itm);
        }
    } else {
        //end
        NBIOPollstersPool_itmRunEnd_(itm);
    }
}*/


BOOL NBIOPollstersPool_isBussy(STNBIOPollstersPoolRef ref){
    BOOL r = FALSE;
    STNBIOPollstersPoolOpq* opq = (STNBIOPollstersPoolOpq*)ref.opaque; NBASSERT(NBIOPollstersPool_isClass(ref))
    //NBObject_lock(opq); //no need to lock
    {
        r = (opq->threads.runningCount > 0);
    }
    //NBObject_unlock(opq);
    return r;
}

void NBIOPollstersPool_stopFlag(STNBIOPollstersPoolRef ref){
    STNBIOPollstersPoolOpq* opq = (STNBIOPollstersPoolOpq*)ref.opaque; NBASSERT(NBIOPollstersPool_isClass(ref))
    //NBObject_lock(opq); //no need to lock
    {
        if(NBStopFlag_isSet(opq->stopFlag)){
            NBStopFlag_activate(opq->stopFlag);
        }
    }
    //NBObject_unlock(opq);
}

void NBIOPollstersPool_stop(STNBIOPollstersPoolRef ref){        //stop and wait for all threads
    STNBIOPollstersPoolOpq* opq = (STNBIOPollstersPoolOpq*)ref.opaque; NBASSERT(NBIOPollstersPool_isClass(ref))
    NBObject_lock(opq); //no need to lock
    {
        if(NBStopFlag_isSet(opq->stopFlag)){
            NBStopFlag_activate(opq->stopFlag);
        }
        //wait for threads (unlocked)
        {
            while(opq->threads.runningCount > 0){
                NBThreadCond_waitObj(&opq->cond, opq);
            }
            NBASSERT(opq->pool.startedCount == 0)
        }
    }
    NBObject_unlock(opq);
}

void NBIOPollstersPool_waitForAll(STNBIOPollstersPoolRef ref){  //wait for all threads
    STNBIOPollstersPoolOpq* opq = (STNBIOPollstersPoolOpq*)ref.opaque; NBASSERT(NBIOPollstersPool_isClass(ref))
    NBObject_lock(opq); //no need to lock
    {
        //wait for threads (unlocked)
        {
            while(opq->threads.runningCount > 0){
                NBThreadCond_waitObj(&opq->cond, opq);
            }
            NBASSERT(opq->pool.startedCount == 0)
        }
    }
    NBObject_unlock(opq);
}


