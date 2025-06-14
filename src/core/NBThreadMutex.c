#include "nb/NBFrameworkPch.h"
#include "nb/NBFrameworkDefs.h"

//Complementary PCH
#ifdef _WIN32
//#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>	//for CRITICAL_SECTION
#else
#	include <pthread.h>
#endif
//
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBMemory.h"

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
#	include "nb/core/NBMngrProcess.h"
#endif

typedef struct STNBThreadMutexOpq_ {
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
		CRITICAL_SECTION	mutex;
#		else
		BOOL				mutexDummy;
#		endif
#	else
	pthread_mutex_t		mutex;
#	endif
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	UI64				uid; //uniqueId
#	endif
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	struct {
		SI32			locksDepth;
	} dbg;
#	endif
} STNBThreadMutexOpq;

//

void NBThreadMutex_init_(STNBThreadMutex* obj
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	, const char* name, const char* fullpath, const SI32 line, const char* func
#	endif
) {
	STNBThreadMutexOpq* opq = obj->opaque = NBMemory_allocType(STNBThreadMutexOpq);
	NBMemory_setZeroSt(*opq, STNBThreadMutexOpq);
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
		{
			InitializeCriticalSection(&opq->mutex);
		}
#		endif
#	else
	pthread_mutex_init(&opq->mutex, NULL);
#	endif
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	opq->uid = NBMngrProcess_mutexCreated(name, fullpath, line, func);
#	endif
}

void NBThreadMutex_release_(STNBThreadMutex* obj
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	, const char* fullpath, const SI32 line, const char* func
#	endif
) {
	STNBThreadMutexOpq* opq = (STNBThreadMutexOpq*)obj->opaque;
	NBASSERT(opq->dbg.locksDepth == 0) //must be unlocked
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
		{
			DeleteCriticalSection(&opq->mutex);
		}
#		endif
#	else
	pthread_mutex_destroy(&opq->mutex);
#	endif
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	{
		NBASSERT(opq->uid != 0)
		if(opq->uid != 0){
			NBMngrProcess_mutexDestroyed(opq->uid);
			opq->uid = 0;
		}
	}
#	endif
	NBMemory_free(opq);
	obj->opaque = NULL;
}

//

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
UI64 NBThreadMutex_getUid(STNBThreadMutex* obj){
	STNBThreadMutexOpq* opq = (STNBThreadMutexOpq*)obj->opaque;
	return opq->uid;
}
#endif

//

BOOL NBThreadMutex_lock_(STNBThreadMutex* obj
#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	, const char* fullpath, const SI32 line, const char* func
#endif						 
){
	BOOL r = FALSE;
	STNBThreadMutexOpq* opq = (STNBThreadMutexOpq*)obj->opaque;
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	NBMngrProcess_mutexLockPush(opq->uid, fullpath, line, func, ENNBThreadLockPushMode_Compete);
#	endif
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
		{
			EnterCriticalSection(&opq->mutex);
			r = TRUE;
		}
#		else
		{
			opq->mutexDummy = TRUE;
		}
#		endif
#	else
	{
		int rr;
		if ((rr = pthread_mutex_lock(&opq->mutex)) != 0) {
			NBASSERT(FALSE)
		} else {
			r = TRUE;
		}
	}
#	endif
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	NBMngrProcess_mutexLockStarted(opq->uid, fullpath, line, func);
#	endif
	{
		IF_NBASSERT(opq->dbg.locksDepth++;)
	}
	return r;
}


//unlock 

BOOL NBThreadMutex_unlock_(STNBThreadMutex* obj
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	, const char* fullpath, const SI32 line, const char* func
#	endif
){
	BOOL r = FALSE;
	STNBThreadMutexOpq* opq = (STNBThreadMutexOpq*)obj->opaque;
	{
		NBASSERT(opq->dbg.locksDepth > 0)
		IF_NBASSERT(opq->dbg.locksDepth--;)
	}
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	NBMngrProcess_mutexLockPop(opq->uid, fullpath, line, func);
#	endif
#	ifdef _WIN32
#		ifndef NB_COMPILE_DRIVER_MODE
		{
			LeaveCriticalSection(&opq->mutex);
			r = TRUE;
		}
#		else
		{
			opq->mutexDummy = FALSE;
		}
#		endif
#	else
	int rr;
	if((rr = pthread_mutex_unlock(&opq->mutex)) != 0){
		NBASSERT(FALSE)
	} else {
		r = TRUE;
	}
#	endif
	return r;
}
