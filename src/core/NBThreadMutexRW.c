//
//  NBThreadMutexRW.c
//  nbframework
//
//  Created by Marcos Ortega on 28/2/19.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBThreadMutexRW.h"
//Complementary PCH
#ifdef _WIN32
//#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>	//for CRITICAL_SECTION
#else
#	include <pthread.h>
#endif
//
#include "nb/core/NBMemory.h"
#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
#	include "nb/core/NBMngrProcess.h"
#endif

typedef struct STNBThreadMutexRWOpq_ {
#	ifdef _WIN32
	SRWLOCK				mutex;
#	else
	pthread_rwlock_t	mutex;
#	endif
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	UI64				uid; //uniqueId
#	endif
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	struct {
		SI32			locksDepth;
	} dbg;
#	endif
} STNBThreadMutexRWOpq;


void NBThreadMutexRW_init_(STNBThreadMutexRW* obj NB_THREAD_MUTEX_IO_EXTRA_PARAMS_DEF0){
	STNBThreadMutexRWOpq* opq = obj->opaque = NBMemory_allocType(STNBThreadMutexRWOpq);
	NBMemory_setZeroSt(*opq, STNBThreadMutexRWOpq);
	//
#	ifdef _WIN32
	InitializeSRWLock(&opq->mutex);
#	else
	pthread_rwlock_init(&opq->mutex, NULL);
#	endif
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	opq->uid = NBMngrProcess_mutexCreated(name, fullpath, line, func);
#	endif	
}

void NBThreadMutexRW_release_(STNBThreadMutexRW* obj){
	STNBThreadMutexRWOpq* opq = (STNBThreadMutexRWOpq*)obj->opaque;
	if(opq != NULL){
		NBASSERT(opq->dbg.locksDepth == 0) //must be unlocked
#		ifdef _WIN32
		//Nothing
#		else
		pthread_rwlock_destroy(&opq->mutex);
#		endif
#		ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
		{
			NBASSERT(opq->uid != 0)
			if(opq->uid != 0){
				NBMngrProcess_mutexDestroyed(opq->uid);
				opq->uid = 0;
			}
		}
#		endif
		//
		NBMemory_free(opq);
		obj->opaque = NULL;
	}
}

//Read lock

BOOL NBThreadMutexRW_tryLockForRead_(STNBThreadMutexRW* obj NB_THREAD_MUTEX_IO_EXTRA_PARAMS_DEF){
	BOOL r = FALSE;
	STNBThreadMutexRWOpq* opq = (STNBThreadMutexRWOpq*)obj->opaque;
#	ifdef _WIN32
	{
		if(TryAcquireSRWLockShared(&opq->mutex)){
			r = TRUE;
		}
	}
#	else
	{
		int rr;
		if((rr = pthread_rwlock_tryrdlock(&opq->mutex)) == 0){
			r = TRUE;
		}
	}
#	endif
	return r;
}

BOOL NBThreadMutexRW_lockForRead_(STNBThreadMutexRW* obj NB_THREAD_MUTEX_IO_EXTRA_PARAMS_DEF){
	BOOL r = FALSE;
	STNBThreadMutexRWOpq* opq = (STNBThreadMutexRWOpq*)obj->opaque;
#	ifdef _WIN32
	{
		AcquireSRWLockShared(&opq->mutex);
		r = TRUE;
	}
#	else
	{
		int rr;
		if((rr = pthread_rwlock_rdlock(&opq->mutex)) != 0){
			NBASSERT(FALSE)
		} else {
			r = TRUE;
		}
	}
#	endif
	return r;
}

BOOL NBThreadMutexRW_unlockFromRead_(STNBThreadMutexRW* obj NB_THREAD_MUTEX_IO_EXTRA_PARAMS_DEF){
	BOOL r = FALSE;
	STNBThreadMutexRWOpq* opq = (STNBThreadMutexRWOpq*)obj->opaque;
#	ifdef _WIN32
	{
		ReleaseSRWLockShared(&opq->mutex);
		r = TRUE;
	}
#	else
	{
		int rr;
		if((rr = pthread_rwlock_unlock(&opq->mutex)) != 0){
			NBASSERT(FALSE)
		} else {
			r = TRUE;
		}
	}
#	endif
	return r;
}

//Write lock

BOOL NBThreadMutexRW_tryLockForWrite_(STNBThreadMutexRW* obj NB_THREAD_MUTEX_IO_EXTRA_PARAMS_DEF){
	BOOL r = FALSE;
	STNBThreadMutexRWOpq* opq = (STNBThreadMutexRWOpq*)obj->opaque;
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	NBMngrProcess_mutexLockPush(opq->uid, fullpath, line, func, ENNBThreadLockPushMode_NonCompete);
#	endif
#	ifdef _WIN32
	{
		if(TryAcquireSRWLockExclusive(&opq->mutex)){
			r = TRUE;
#			ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
			NBMngrProcess_mutexLockStarted(opq->uid, fullpath, line, func);
#			endif
			{
				IF_NBASSERT(opq->dbg.locksDepth++;)
			}
#		ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
		} else {
			NBMngrProcess_mutexLockPop(opq->uid, fullpath, line, func);
#		endif
		}
	}
#	else
	{
		int rr;
		if((rr = pthread_rwlock_trywrlock(&opq->mutex)) == 0){
			r = TRUE;
#			ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
			NBMngrProcess_mutexLockStarted(opq->uid, fullpath, line, func);
#			endif
			{
				IF_NBASSERT(opq->dbg.locksDepth++;)
			}
#		ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
		} else {
			NBMngrProcess_mutexLockPop(opq->uid, fullpath, line, func);
#		endif
		}
	}
#	endif
	return r;
}

BOOL NBThreadMutexRW_lockForWrite_(STNBThreadMutexRW* obj NB_THREAD_MUTEX_IO_EXTRA_PARAMS_DEF){
	BOOL r = FALSE;
	STNBThreadMutexRWOpq* opq = (STNBThreadMutexRWOpq*)obj->opaque;
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	NBMngrProcess_mutexLockPush(opq->uid, fullpath, line, func, ENNBThreadLockPushMode_Compete);
#	endif
#	ifdef _WIN32
	{
		AcquireSRWLockExclusive(&opq->mutex);
		r = TRUE;
	}
#	else
	{
		int rr;
		if((rr = pthread_rwlock_wrlock(&opq->mutex)) != 0){
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

BOOL NBThreadMutexRW_unlockFromWrite_(STNBThreadMutexRW* obj NB_THREAD_MUTEX_IO_EXTRA_PARAMS_DEF){
	BOOL r = FALSE;
	STNBThreadMutexRWOpq* opq = (STNBThreadMutexRWOpq*)obj->opaque;
	{
		NBASSERT(opq->dbg.locksDepth > 0)
		IF_NBASSERT(opq->dbg.locksDepth--;)
	}
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	NBMngrProcess_mutexLockPop(opq->uid, fullpath, line, func);
#	endif
#	ifdef _WIN32
	{
		ReleaseSRWLockExclusive(&opq->mutex);
		r = TRUE;
	}
#	else
	{
		int rr;
		if((rr = pthread_rwlock_unlock(&opq->mutex)) != 0){
			NBASSERT(FALSE)
		} else {
			r = TRUE;
		}
	}
#	endif
	return r;
}

