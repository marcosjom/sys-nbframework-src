#ifndef NB_SOCKET_POLLSTERS_POLL_H
#define NB_SOCKET_POLLSTERS_POLL_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBIOPollster.h"
#include "nb/core/NBThreadsPool.h"
#include "nb/core/NBIOPollstersProvider.h"

#ifdef __cplusplus
extern "C" {
#endif

//NBIOPollstersPoolCfg

typedef struct STNBIOPollstersPoolTimeout_ {
    UI32    ms;     //ms timeout for each poll-cmd
} STNBIOPollstersPoolTimeout;

const STNBStructMap* NBIOPollstersPoolTimeout_getSharedStructMap(void);

//NBIOPollstersPoolCfg

typedef struct STNBIOPollstersPoolCfg_ {
    UI32    ammount;    //ammount of pollsters
    STNBIOPollstersPoolTimeout timeout;
} STNBIOPollstersPoolCfg;

const STNBStructMap* NBIOPollstersPoolCfg_getSharedStructMap(void);

//NBIOPollstersPool

NB_OBJREF_HEADER(NBIOPollstersPool)

//cfg
BOOL NBIOPollstersPool_setThreadsPool(STNBIOPollstersPoolRef ref, STNBThreadsPoolRef threadsPool); //run on shared threads owned by the pool (optimization)

//prepare
BOOL NBIOPollstersPool_setParentStopFlag(STNBIOPollstersPoolRef ref, STNBStopFlagRef* parentStopFlag);
BOOL NBIOPollstersPool_prepare(STNBIOPollstersPoolRef ref, const STNBIOPollstersPoolCfg* cfg);

//provider
BOOL NBIOPollstersPool_linkToProvider(STNBIOPollstersPoolRef ref, STNBIOPollstersProviderRef* dst); //configures the provider to this NBIOPollstersPool

//run
BOOL NBIOPollstersPool_startThreads(STNBIOPollstersPoolRef ref);
BOOL NBIOPollstersPool_isBussy(STNBIOPollstersPoolRef ref);
void NBIOPollstersPool_stopFlag(STNBIOPollstersPoolRef ref);    //stop flag and return inmediatly
void NBIOPollstersPool_stop(STNBIOPollstersPoolRef ref);        //stop and wait for all threads
void NBIOPollstersPool_waitForAll(STNBIOPollstersPoolRef ref);  //wait for all threads

#ifdef __cplusplus
} //extern "C"
#endif


#endif
