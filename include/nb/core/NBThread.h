#ifndef NB_THREAD_H
#define NB_THREAD_H

#include "nb/NBFrameworkDefs.h"
#include <time.h>	//for clock(), clock_gettime(), time_t, struct tm, time, localtime

#ifdef _WIN32
#	define NBTHREAD_CLOCK	UI64
#else
#	define NBTHREAD_CLOCK	clock_t
#endif

#ifdef __cplusplus
extern "C" {
#endif
	
#	define NBThread_release(OBJ)	NBThread_release_((OBJ), __FILE__, __LINE__, __func__)
	/*
	 Important: Only when a terminated joinable thread has been joined are the last of its
	 resources released back to the system.  When a detached thread
	 terminates, its resources are automatically released back to the
	 system: it is not possible to join with the thread in order to obtain
	 its exit status.
	 */
	typedef struct STNBThread_ {
		void*	opaque;
	} STNBThread;

	typedef SI64 (*NBThreadRoutinePtr)(STNBThread* thread, void* param);
	
	void NBThread_init(STNBThread* obj);
	void NBThread_release_(STNBThread* obj, const char* fullpath, const SI32 line, const char* func);
	
	BOOL NBThread_setIsJoinable(STNBThread* obj, const BOOL isJoinable);
	BOOL NBThread_setStackSz(STNBThread* obj, const UI32 stackSz);
	
	BOOL NBThread_start(STNBThread* obj, NBThreadRoutinePtr routine, void* param, BOOL* dstErrIsTemp);
	BOOL NBThread_isRunning(STNBThread* obj);
	SI64 NBThread_retVal(STNBThread* obj);
	
	
	void NBThread_yield(void);
	void NBThread_mSleep(const UI64 mSecs);
	void NBThread_uSleep(const UI64 uSecs); //1usec = 1000ms
	NBTHREAD_CLOCK NBThread_clock(void);
	NBTHREAD_CLOCK NBThread_clocksPerSec(void);
	

#ifdef __cplusplus
} //extern "C"
#endif

#endif
