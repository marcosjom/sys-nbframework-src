#include "nb/NBFrameworkPch.h"
//Complementary PCH
#ifdef _WIN32
//#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>	//for CONDITION_VARIABLE
#else
#	include <pthread.h>
#	include <errno.h>	//for ETIMEDOUT
#	include <time.h>	//clock_gettime()
#endif
//
#include "nb/core/NBThreadCond.h"
#include "nb/core/NBMemory.h"
#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
#	include "nb/core/NBMngrProcess.h"
#	include "nb/core/NBThreadMutex.h"
#endif
//

typedef struct STNBThreadCondOpq_ {
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
			CONDITION_VARIABLE	cond; //Windows Vista
#		else
			BOOL condDummy;
#		endif
#	else
	pthread_cond_t		cond;
#	endif
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	UI64				uid; //uniqueId
#	endif
} STNBThreadCondOpq;

//

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBThreadCond_init_(STNBThreadCond* obj , const char* name, const char* fullpath, const SI32 line, const char* func)
#else
void NBThreadCond_init(STNBThreadCond* obj)
#endif
{
	STNBThreadCondOpq* opq = obj->opaque = NBMemory_allocType(STNBThreadCondOpq);
	NBMemory_setZeroSt(*opq, STNBThreadCondOpq);
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
		{
			InitializeConditionVariable(&opq->cond); //Windows Vista
		}
#		endif
#	else
	pthread_cond_init(&opq->cond, NULL);
#	endif
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	opq->uid = NBMngrProcess_condCreated(name, fullpath, line, func);
#	endif
}

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
void NBThreadCond_release_(STNBThreadCond* obj, const char* fullpath, const SI32 line, const char* func)
#else
void NBThreadCond_release(STNBThreadCond* obj)
#endif
{
	STNBThreadCondOpq* opq = (STNBThreadCondOpq*)obj->opaque;
#	ifdef _WIN32
	//No need to destroy the 'cond' variable
#	else
	pthread_cond_destroy(&opq->cond);
#	endif
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	{
		NBASSERT(opq->uid != 0)
		if(opq->uid != 0){
			NBMngrProcess_condDestroyed(opq->uid);
			opq->uid = 0;
		}
	}
#	endif
	NBMemory_free(opq);
	obj->opaque = NULL;
}

//wait

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
BOOL NBThreadCond_wait_(STNBThreadCond* obj, STNBThreadMutex* mutex, const char* fullpath, const SI32 line, const char* func)
#else
BOOL NBThreadCond_wait(STNBThreadCond* obj, STNBThreadMutex* mutex)
#endif
{
	BOOL r = FALSE;
	STNBThreadCondOpq* opq = (STNBThreadCondOpq*)obj->opaque;
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	const UI64 mutexId = NBThreadMutex_getUid(mutex);
	NBMngrProcess_condWaitPush(opq->uid, mutexId, fullpath, line, func);
#	endif
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
		{
			r = SleepConditionVariableCS(&opq->cond, (CRITICAL_SECTION*)mutex->opaque, INFINITE); //Windows Vista
		}
#		else
		{
			opq->condDummy = TRUE;
		}
#		endif
#	else
	int rr;
	if((rr = pthread_cond_wait(&opq->cond, (pthread_mutex_t*)mutex->opaque)) != 0){
		NBASSERT(FALSE)
	} else {
		r = TRUE;
	}
#	endif
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	NBMngrProcess_condWaitPop(opq->uid, mutexId, fullpath, line, func);
#	endif
	return r;
}

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
BOOL NBThreadCond_waitRef_(STNBThreadCond* obj, STNBObjRef refLock, const char* fullpath, const SI32 line, const char* func)
#else
BOOL NBThreadCond_waitRef(STNBThreadCond* obj, STNBObjRef refLock)
#endif
{
	BOOL r = FALSE;
	STNBThreadCondOpq* opq = (STNBThreadCondOpq*)obj->opaque;
	NBASSERT(refLock.opaque != NULL)
	NBASSERT(((STNBObject*)refLock.opaque)->mutex != NULL)
	NBASSERT(NBObject_isClass(refLock.opaque))
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	const UI64 mutexId = NBObject_getMutexId(refLock.opaque);
	NBMngrProcess_condWaitPush(opq->uid, mutexId, fullpath, line, func);
#	endif
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
		{
			r = SleepConditionVariableCS(&opq->cond, (CRITICAL_SECTION*)((STNBObject*)refLock.opaque)->mutex, INFINITE); //Windows Vista
		}
#		else
		{
			opq->condDummy = TRUE;
		}
#		endif
#	else
	int rr;
	if((rr = pthread_cond_wait(&opq->cond, (pthread_mutex_t*)((STNBObject*)refLock.opaque)->mutex)) != 0){
		NBASSERT(FALSE)
	} else {
		r = TRUE;
	}
#	endif
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	NBMngrProcess_condWaitPop(opq->uid, mutexId, fullpath, line, func);
#	endif
	return r;
}

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
BOOL NBThreadCond_waitObj_(STNBThreadCond* obj, void* objLock, const char* fullpath, const SI32 line, const char* func)
#else
BOOL NBThreadCond_waitObj(STNBThreadCond* obj, void* objLock)
#endif
{
	BOOL r = FALSE;
	STNBThreadCondOpq* opq = (STNBThreadCondOpq*)obj->opaque;
	NBASSERT(objLock != NULL)
	NBASSERT(((STNBObject*)objLock)->mutex != NULL)
	NBASSERT(NBObject_isClass(objLock))
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	const UI64 mutexId = NBObject_getMutexId(objLock);
	NBMngrProcess_condWaitPush(opq->uid, mutexId, fullpath, line, func);
#	endif
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
		{
			r = SleepConditionVariableCS(&opq->cond, (CRITICAL_SECTION*)((STNBObject*)objLock)->mutex, INFINITE); //Windows Vista
		}
#		else
		{
			opq->condDummy = TRUE;
		}
#		endif
#	else
	int rr;
	if((rr = pthread_cond_wait(&opq->cond, (pthread_mutex_t*)((STNBObject*)objLock)->mutex)) != 0){
		NBASSERT(FALSE)
	} else {
		r = TRUE;
	}
#	endif
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	NBMngrProcess_condWaitPop(opq->uid, mutexId, fullpath, line, func);
#	endif
	return r;
}

//timedWait

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
BOOL NBThreadCond_timedWait_(STNBThreadCond* obj, STNBThreadMutex* mutex, const UI32 ms, const char* fullpath, const SI32 line, const char* func)
#else
BOOL NBThreadCond_timedWait(STNBThreadCond* obj, STNBThreadMutex* mutex, const UI32 ms)
#endif
{
	BOOL r = FALSE;
	STNBThreadCondOpq* opq = (STNBThreadCondOpq*)obj->opaque;
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	const UI64 mutexId = NBThreadMutex_getUid(mutex);
	NBMngrProcess_condWaitPush(opq->uid, mutexId, fullpath, line, func);
#	endif
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
		{
			r = SleepConditionVariableCS(&opq->cond, (CRITICAL_SECTION*)mutex->opaque, ms); //Windows Vista
		}
#		else
		{
			opq->condDummy = TRUE;
		}
#		endif
#	else
	{
		struct timespec timeout;
        if(0 == clock_gettime(CLOCK_REALTIME, &timeout)){
            int rr;
            timeout.tv_nsec += (ms % 1000) * 1000000;
            timeout.tv_sec += (ms / 1000) + (timeout.tv_nsec / 1000000000);
            timeout.tv_nsec %= 1000000000;
            rr = pthread_cond_timedwait(&opq->cond, (pthread_mutex_t*)mutex->opaque, &timeout);
            if (rr != 0 && rr != ETIMEDOUT) {
                NBASSERT(FALSE)
            } else {
                r = TRUE;
            }
        }
	}
#	endif
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	NBMngrProcess_condWaitPop(opq->uid, mutexId, fullpath, line, func);
#	endif
	return r;
}

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
BOOL NBThreadCond_timedWaitRef_(STNBThreadCond* obj, STNBObjRef refLock, const UI32 ms, const char* fullpath, const SI32 line, const char* func)
#else
BOOL NBThreadCond_timedWaitRef(STNBThreadCond* obj, STNBObjRef refLock, const UI32 ms)
#endif
{
	BOOL r = FALSE;
	STNBThreadCondOpq* opq = (STNBThreadCondOpq*)obj->opaque;
	NBASSERT(refLock.opaque != NULL)
	NBASSERT(((STNBObject*)refLock.opaque)->mutex != NULL)
	NBASSERT(NBObject_isClass(refLock.opaque))
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	const UI64 mutexId = NBObject_getMutexId(refLock.opaque);
	NBMngrProcess_condWaitPush(opq->uid, mutexId, fullpath, line, func);
#	endif
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
		{
			r = SleepConditionVariableCS(&opq->cond, (CRITICAL_SECTION*)((STNBObject*)refLock.opaque)->mutex, ms); //Windows Vista
		}
#		else
		{
			opq->condDummy = TRUE;
		}
#		endif
#	else
	{
		struct timespec timeout;
        if(0 == clock_gettime(CLOCK_REALTIME, &timeout)){
            int rr;
            timeout.tv_nsec += (ms % 1000) * 1000000;
            timeout.tv_sec += (ms / 1000) + (timeout.tv_nsec / 1000000000);
            timeout.tv_nsec %= 1000000000;
            rr = pthread_cond_timedwait(&opq->cond, (pthread_mutex_t*)((STNBObject*)refLock.opaque)->mutex, &timeout);
            if (rr != 0 && rr != ETIMEDOUT) {
                NBASSERT(FALSE)
            } else {
                r = TRUE;
            }
        }
	}
#	endif
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	NBMngrProcess_condWaitPop(opq->uid, mutexId, fullpath, line, func);
#	endif
	return r;
}

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
BOOL NBThreadCond_timedWaitObj_(STNBThreadCond* obj, void* objLock, const UI32 ms, const char* fullpath, const SI32 line, const char* func)
#else
BOOL NBThreadCond_timedWaitObj(STNBThreadCond* obj, void* objLock, const UI32 ms)
#endif
{
	BOOL r = FALSE;
	STNBThreadCondOpq* opq = (STNBThreadCondOpq*)obj->opaque;
	NBASSERT(objLock != NULL)
	NBASSERT(((STNBObject*)objLock)->mutex != NULL)
	NBASSERT(NBObject_isClass(objLock))
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	const UI64 mutexId = NBObject_getMutexId(objLock);
	NBMngrProcess_condWaitPush(opq->uid, mutexId, fullpath, line, func);
#	endif
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
		{
			r = SleepConditionVariableCS(&opq->cond, (CRITICAL_SECTION*)((STNBObject*)objLock)->mutex, ms); //Windows Vista
		}
#		else
		{
			opq->condDummy = TRUE;
		}
#		endif
#	else
	{
		struct timespec timeout;
        if(0 == clock_gettime(CLOCK_REALTIME, &timeout)){
            int rr;
            timeout.tv_nsec += (ms % 1000) * 1000000;
            timeout.tv_sec += (ms / 1000) + (timeout.tv_nsec / 1000000000);
            timeout.tv_nsec %= 1000000000;
            rr = pthread_cond_timedwait(&opq->cond, (pthread_mutex_t*)((STNBObject*)objLock)->mutex, &timeout);
            if (rr != 0 && rr != ETIMEDOUT) {
                NBASSERT(FALSE)
            } else {
                r = TRUE;
            }
        }
	}
#	endif
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	NBMngrProcess_condWaitPop(opq->uid, mutexId, fullpath, line, func);
#	endif
	return r;
}


//signal

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
BOOL NBThreadCond_signal_(STNBThreadCond* obj, const char* fullpath, const SI32 line, const char* func)
#else
BOOL NBThreadCond_signal(STNBThreadCond* obj)
#endif
{
	BOOL r = FALSE;
	STNBThreadCondOpq* opq = (STNBThreadCondOpq*)obj->opaque;
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	NBMngrProcess_condSignal(opq->uid, fullpath, line, func);
#	endif
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
		{
			WakeConditionVariable(&opq->cond); //WindowsVista+
			r = TRUE;
		}
#		else
		{
			opq->condDummy = TRUE;
		}
#		endif
#	else
	{
		int rr;
		if ((rr = pthread_cond_signal(&opq->cond)) != 0) {
			NBASSERT(FALSE)
		} else {
			r = TRUE;
		}
	}
#	endif
	return r;
}

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
BOOL NBThreadCond_broadcast_(STNBThreadCond* obj, const char* fullpath, const SI32 line, const char* func)
#else
BOOL NBThreadCond_broadcast(STNBThreadCond* obj)
#endif
{
	BOOL r = FALSE;
	STNBThreadCondOpq* opq = (STNBThreadCondOpq*)obj->opaque;
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	NBMngrProcess_condBroadcast(opq->uid, fullpath, line, func);
#	endif
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
		{
			WakeAllConditionVariable(&opq->cond); //WindowsVista+
			r = TRUE;
		}
#		else
		{
			opq->condDummy = TRUE;
		}
#		endif
#	else
	{
		int rr;
		if ((rr = pthread_cond_broadcast(&opq->cond)) != 0) {
			NBASSERT(FALSE)
		} else {
			r = TRUE;
		}
	}
#	endif
	return r;
}


