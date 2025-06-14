#ifndef NB_THREAD_COND_H
#define NB_THREAD_COND_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBThreadMutex.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct STNBThreadCond_ {
		void*	opaque;
	} STNBThreadCond;
	
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
#	define NBThreadCond_init(OBJ)						NBThreadCond_init_((OBJ), #OBJ, __FILE__, __LINE__, __func__)
#	define NBThreadCond_release(OBJ)					NBThreadCond_release_((OBJ), __FILE__, __LINE__, __func__)
#	define NBThreadCond_wait(OBJ, MUTEX)				NBThreadCond_wait_((OBJ), (MUTEX), __FILE__, __LINE__, __func__)
#	define NBThreadCond_waitRef(OBJ, RLCK)				NBThreadCond_waitRef_((OBJ), (RLCK), __FILE__, __LINE__, __func__)
#	define NBThreadCond_waitObj(OBJ, OLCK)				NBThreadCond_waitObj_((OBJ), (OLCK), __FILE__, __LINE__, __func__)
#	define NBThreadCond_timedWait(OBJ, MUTEX, MSECS)	NBThreadCond_timedWait_((OBJ), (MUTEX), (MSECS), __FILE__, __LINE__, __func__)
#	define NBThreadCond_timedWaitRef(OBJ, RLCK, MSECS)	NBThreadCond_timedWaitRef_((OBJ), (RLCK), (MSECS), __FILE__, __LINE__, __func__)
#	define NBThreadCond_timedWaitObj(OBJ, OLCK, MSECS)	NBThreadCond_timedWaitObj_((OBJ), (OLCK), (MSECS), __FILE__, __LINE__, __func__)
#	define NBThreadCond_signal(OBJ)						NBThreadCond_signal_((OBJ), __FILE__, __LINE__, __func__)
#	define NBThreadCond_broadcast(OBJ)					NBThreadCond_broadcast_((OBJ), __FILE__, __LINE__, __func__)
	void NBThreadCond_init_(STNBThreadCond* obj, const char* name, const char* fullpath, const SI32 line, const char* func);
	void NBThreadCond_release_(STNBThreadCond* obj, const char* fullpath, const SI32 line, const char* func);
	BOOL NBThreadCond_wait_(STNBThreadCond* obj, STNBThreadMutex* mutex, const char* fullpath, const SI32 line, const char* func);
	BOOL NBThreadCond_waitRef_(STNBThreadCond* obj, STNBObjRef refLock, const char* fullpath, const SI32 line, const char* func);
	BOOL NBThreadCond_waitObj_(STNBThreadCond* obj, void* objLock, const char* fullpath, const SI32 line, const char* func);
	BOOL NBThreadCond_timedWait_(STNBThreadCond* obj, STNBThreadMutex* mutex, const UI32 ms, const char* fullpath, const SI32 line, const char* func);
	BOOL NBThreadCond_timedWaitRef_(STNBThreadCond* obj, STNBObjRef refLock, const UI32 ms, const char* fullpath, const SI32 line, const char* func);
	BOOL NBThreadCond_timedWaitObj_(STNBThreadCond* obj, void* objLock, const UI32 ms, const char* fullpath, const SI32 line, const char* func);
	BOOL NBThreadCond_signal_(STNBThreadCond* obj, const char* fullpath, const SI32 line, const char* func);
	BOOL NBThreadCond_broadcast_(STNBThreadCond* obj, const char* fullpath, const SI32 line, const char* func);
#	else
#	define NBThreadCond_init_(OBJ)						NBThreadCond_init((OBJ))
#	define NBThreadCond_release_(OBJ)					NBThreadCond_release((OBJ))
#	define NBThreadCond_wait_(OBJ, MUTEX)				NBThreadCond_wait((OBJ), (MUTEX))
#	define NBThreadCond_waitRef_(OBJ, RLCK)				NBThreadCond_waitRef((OBJ), (RLCK))
#	define NBThreadCond_waitObj_(OBJ, OLCK)				NBThreadCond_waitObj((OBJ), (OLCK))
#	define NBThreadCond_timedWait_(OBJ, MUTEX, MSECS)	NBThreadCond_timedWait((OBJ), (MUTEX), (MSECS))
#	define NBThreadCond_timedWaitRef_(OBJ, RLCK, MSECS)	NBThreadCond_timedWaitRef((OBJ), (RLCK), (MSECS))
#	define NBThreadCond_timedWaitObj_(OBJ, OLCK, MSECS)	NBThreadCond_timedWaitObj((OBJ), (OLCK), (MSECS))
#	define NBThreadCond_signal_(OBJ)					NBThreadCond_signal((OBJ))
#	define NBThreadCond_broadcast_(OBJ)					NBThreadCond_broadcast((OBJ))
	void NBThreadCond_init(STNBThreadCond* obj);
	void NBThreadCond_release(STNBThreadCond* obj);
	BOOL NBThreadCond_wait(STNBThreadCond* obj, STNBThreadMutex* mutex);
	BOOL NBThreadCond_waitRef(STNBThreadCond* obj, STNBObjRef refLock);
	BOOL NBThreadCond_waitObj(STNBThreadCond* obj, void* objLock);
	BOOL NBThreadCond_timedWait(STNBThreadCond* obj, STNBThreadMutex* mutex, const UI32 ms);
	BOOL NBThreadCond_timedWaitRef(STNBThreadCond* obj, STNBObjRef refLock, const UI32 ms);
	BOOL NBThreadCond_timedWaitObj(STNBThreadCond* obj, void* objLock, const UI32 ms);
	BOOL NBThreadCond_signal(STNBThreadCond* obj);
	BOOL NBThreadCond_broadcast(STNBThreadCond* obj);
#	endif
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif

