//
//  NBThreadMutexRW.h
//  nbframework
//
//  Created by Marcos Ortega on 28/2/19.
//

#ifndef NBThreadMutexRW_h
#define NBThreadMutexRW_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBThreadCond.h"

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
#	define NB_THREAD_MUTEX_IO_EXTRA_PARAMS_LST0	, name, fullpath, line, func
#	define NB_THREAD_MUTEX_IO_EXTRA_PARAMS_DEF0	, const char* name, const char* fullpath, const SI32 line, const char* func
#	define NB_THREAD_MUTEX_IO_EXTRA_PARAMS_LST	, fullpath, line, func
#	define NB_THREAD_MUTEX_IO_EXTRA_PARAMS_DEF	, const char* fullpath, const SI32 line, const char* func
#else
#	define NB_THREAD_MUTEX_IO_EXTRA_PARAMS_LST0
#	define NB_THREAD_MUTEX_IO_EXTRA_PARAMS_DEF0
#	define NB_THREAD_MUTEX_IO_EXTRA_PARAMS_LST
#	define NB_THREAD_MUTEX_IO_EXTRA_PARAMS_DEF
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
#	define NBThreadMutexRW_init(OBJ)								NBThreadMutexRW_init_((OBJ), #OBJ, __FILE__, __LINE__, __func__)
#	define NBThreadMutexRW_release(OBJ)								NBThreadMutexRW_release_((OBJ))
//	read
#	define NBThreadMutexRW_tryLockForRead(OBJ)						NBThreadMutexRW_tryLockForRead_((OBJ), __FILE__, __LINE__, __func__)
#	define NBThreadMutexRW_lockForRead(OBJ)							NBThreadMutexRW_lockForRead_((OBJ), __FILE__, __LINE__, __func__)
#	define NBThreadMutexRW_unlockFromRead(OBJ)						NBThreadMutexRW_unlockFromRead_((OBJ), __FILE__, __LINE__, __func__)
//	write
#	define NBThreadMutexRW_tryLockForWrite(OBJ)						NBThreadMutexRW_tryLockForWrite_((OBJ), __FILE__, __LINE__, __func__)
#	define NBThreadMutexRW_lockForWrite(OBJ)						NBThreadMutexRW_lockForWrite_((OBJ), __FILE__, __LINE__, __func__)
#	define NBThreadMutexRW_unlockFromWrite(OBJ)						NBThreadMutexRW_unlockFromWrite_((OBJ), __FILE__, __LINE__, __func__)
#else
#	define NBThreadMutexRW_init(OBJ)								NBThreadMutexRW_init_((OBJ))
#	define NBThreadMutexRW_release(OBJ)								NBThreadMutexRW_release_((OBJ))
//	read
#	define NBThreadMutexRW_tryLockForRead(OBJ)						NBThreadMutexRW_tryLockForRead_((OBJ))
#	define NBThreadMutexRW_lockForRead(OBJ)							NBThreadMutexRW_lockForRead_((OBJ))
#	define NBThreadMutexRW_unlockFromRead(OBJ)						NBThreadMutexRW_unlockFromRead_((OBJ))
//	write
#	define NBThreadMutexRW_tryLockForWrite(OBJ)						NBThreadMutexRW_tryLockForWrite_((OBJ))
#	define NBThreadMutexRW_lockForWrite(OBJ)						NBThreadMutexRW_lockForWrite_((OBJ))
#	define NBThreadMutexRW_unlockFromWrite(OBJ)						NBThreadMutexRW_unlockFromWrite_((OBJ))
#	endif

typedef struct STNBThreadMutexRW_ {
	void* opaque;
} STNBThreadMutexRW;

void NBThreadMutexRW_init_(STNBThreadMutexRW* obj NB_THREAD_MUTEX_IO_EXTRA_PARAMS_DEF0);
void NBThreadMutexRW_release_(STNBThreadMutexRW* obj);

//Read lock
BOOL NBThreadMutexRW_tryLockForRead_(STNBThreadMutexRW* obj NB_THREAD_MUTEX_IO_EXTRA_PARAMS_DEF);
BOOL NBThreadMutexRW_lockForRead_(STNBThreadMutexRW* obj NB_THREAD_MUTEX_IO_EXTRA_PARAMS_DEF);
BOOL NBThreadMutexRW_unlockFromRead_(STNBThreadMutexRW* obj NB_THREAD_MUTEX_IO_EXTRA_PARAMS_DEF);

//Write lock
BOOL NBThreadMutexRW_tryLockForWrite_(STNBThreadMutexRW* obj NB_THREAD_MUTEX_IO_EXTRA_PARAMS_DEF);
BOOL NBThreadMutexRW_lockForWrite_(STNBThreadMutexRW* obj NB_THREAD_MUTEX_IO_EXTRA_PARAMS_DEF);
BOOL NBThreadMutexRW_unlockFromWrite_(STNBThreadMutexRW* obj NB_THREAD_MUTEX_IO_EXTRA_PARAMS_DEF);

#endif /* NBThreadMutexRW_h */
