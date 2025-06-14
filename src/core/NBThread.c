#include "nb/NBFrameworkPch.h"
//Complementary PCH
#ifdef _WIN32
//#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>	//for SwitchToThread
#else
#	include <sched.h>	//for sched_yield
#	include <pthread.h>	//
#	include <unistd.h>	//for usleep()
#	include <errno.h>	//for EAGAIN
#endif
//
#include "nb/core/NBThread.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBMngrProcess.h"
#ifdef CONFIG_NB_INCLUDE_THREADS_STORAGE_CLEANUP
#   include "nb/core/NBMngrProcess.h"
#endif

typedef struct STNBThreadOpq_ {
	BOOL				created;	//Start state.
	BOOL				isRunning;	//While running.
	BOOL				joined;		//ToDo: implement.
	BOOL				isJoinable;	//Cfg: when joinable resources will be released only after 'join'.
	UI32				stackSz;	//Cfg: stack size in bytes. Zero means 'default'.
	//
	NBThreadRoutinePtr	routine;
	void*				param;
	SI64				retVal;	//value returned by the function
	//
#	ifdef _WIN32
	HANDLE				handle;
	DWORD				threadId;
#	else
	pthread_t			thread;
#	endif
} STNBThreadOpq;

void NBThread_init(STNBThread* obj){
	STNBThreadOpq* opq = obj->opaque = NBMemory_allocType(STNBThreadOpq);
	NBMemory_setZeroSt(*opq, STNBThreadOpq);
	//
	opq->created	= FALSE;
	opq->isRunning	= FALSE;
	opq->joined		= FALSE;
	opq->isJoinable	= TRUE;
	opq->stackSz	= 0;	//Default
	//
	opq->routine	= NULL;
	opq->param		= NULL;
	opq->retVal		= 0;
	//
#	ifdef _WIN32
	opq->handle		= NULL;
	opq->threadId	= 0;
#	else
	//opq->thread
#	endif
}

void NBThread_release_(STNBThread* obj, const char* fullpath, const SI32 line, const char* func){
	STNBThreadOpq* opq = (STNBThreadOpq*)obj->opaque;
	if(opq->created){
		/*
		 Important: Only when a terminated joinable thread has been joined are the last of its
		 resources released back to the system.  When a detached thread
		 terminates, its resources are automatically released back to the
		 system: it is not possible to join with the thread in order to obtain
		 its exit status.
		 */
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		if(!(!(opq->isJoinable && !opq->isRunning) || !opq->isJoinable)){
			PRINTF_ERROR("NBThread_release_, expected JOIN before releasing the thread ('%s' at '%s:%d').\n", func, fullpath, line); //Expected JOIN
			NBASSERT(FALSE)
		}
		if(!(opq->isJoinable || (!opq->isJoinable && opq->isRunning))){
			PRINTF_ERROR("NBThread_release_, expected to release at the routine function ('%s' at '%s:%d').\n", func, fullpath, line); //Expected to be released at the "routine" function
			NBASSERT(FALSE)
		}
		if(!((opq->isJoinable && !opq->isRunning) || (!opq->isJoinable && opq->isRunning))){
			PRINTF_ERROR("NBThread_release_, unexpected thread configuration at the end ('%s' at '%s:%d').\n", func, fullpath, line); //Expected JOIN or at the "routine" function.
		}
#		endif
#		ifdef _WIN32
		NBASSERT(opq->handle != NULL)
		CloseHandle(opq->handle);
		opq->handle		= NULL;
		opq->threadId	= 0;
#		else
		NBASSERT(!(opq->created && opq->isJoinable && !opq->joined)) //Resource leak, this thread expected a 'join' to release resources.
#		endif
		opq->created = FALSE;
	}
	NBMemory_free(obj->opaque);
	obj->opaque = NULL;
}

BOOL NBThread_setIsJoinable(STNBThread* obj, const BOOL isJoinable){
	BOOL r = FALSE;
	STNBThreadOpq* opq = (STNBThreadOpq*)obj->opaque;
	if(!opq->created){ //Cfg only posible before creation
		opq->isJoinable = isJoinable;
		r = TRUE;
	}
	return r;
}

BOOL NBThread_setStackSz(STNBThread* obj, const UI32 stackSz){
	BOOL r = FALSE;
	STNBThreadOpq* opq = (STNBThreadOpq*)obj->opaque;
	if(!opq->created){ //Cfg only posible before creation
		opq->stackSz = stackSz;
		r = TRUE;
	}
	return r;
}

BOOL NBThread_isRunning(STNBThread* obj){
	return ((STNBThreadOpq*)obj->opaque)->isRunning;
}

SI64 NBThread_retVal(STNBThread* obj){
	return ((STNBThreadOpq*)obj->opaque)->retVal;
}

void NBThread_yield(void){
#	ifdef _WIN32
	SwitchToThread();
#	else
	sched_yield();
#	endif
}

void NBThread_mSleep(const UI64 mSecs){
#	ifdef _WIN32
	Sleep((DWORD)mSecs);
#	else
	usleep((useconds_t)mSecs * 1000);
#	endif
}

void NBThread_uSleep(const UI64 uSecs){
#	ifdef _WIN32
	NBASSERT(FALSE) //ToDo
#	else
	usleep((useconds_t)uSecs);
#	endif
}

NBTHREAD_CLOCK NBThread_clock(void){
	NBTHREAD_CLOCK r;
#	ifdef _WIN32
	{
		LARGE_INTEGER t;
		QueryPerformanceCounter(&t);
		r = t.QuadPart;
	}
#	else
	r = clock();
#	endif
	return r;
}

NBTHREAD_CLOCK NBThread_clocksPerSec(void){
	NBTHREAD_CLOCK r;
#	ifdef _WIN32
	{
		LARGE_INTEGER t;
		QueryPerformanceFrequency(&t);
		r = t.QuadPart;
	}
#	else
	r = CLOCKS_PER_SEC;
#	endif
	return r;
}

#ifdef _WIN32
DWORD WINAPI NBThread_runRoutine(LPVOID param)
#else
void* NBThread_runRoutine(void* param)
#endif
{
#	ifdef _WIN32
	DWORD r = 0;
#	else
	void* r = NULL;
#	endif
	STNBThread* obj	= (STNBThread*)param;
	STNBThreadOpq* opq = (STNBThreadOpq*)obj->opaque;
	const BOOL isJoinable = opq->isJoinable;
	//run
	opq->isRunning	= TRUE;
	//notify at NBThreadMngr as explicit-thread
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	{
		NBMngrProcess_setCurThreadAsExplicit();
	}
#	endif
	//run
#	ifdef _WIN32
	r = (DWORD)(*opq->routine)(obj, opq->param);
#	else
	r = (void*)(*opq->routine)(obj, opq->param);
#	endif
    //Destroy storages
#   ifdef CONFIG_NB_INCLUDE_THREADS_STORAGE_CLEANUP
    NBMngrProcess_storageDestroyAll(); //at current thread
#   endif
	//if not joinable, the 'opq' object is released inside the tread
	if(isJoinable){
		opq->retVal		= (SI64)r;
		opq->isRunning	= FALSE;
	}
	return r;
}

//The stack is the amount of data for excution, recursivity and loca variable allocation.
BOOL NBThread_start(STNBThread* obj, NBThreadRoutinePtr routine, void* param, BOOL* dstErrIsTemp){
	BOOL r = FALSE;
	STNBThreadOpq* opq = (STNBThreadOpq*)obj->opaque;
	if(!opq->created){
#		ifdef _WIN32
		//NBASSERT(FALSE) //ToDo: implement stackSz
		if(opq->handle == NULL && opq->threadId == 0){
			opq->created	= TRUE;
			opq->isRunning	= TRUE;
			opq->routine	= routine;
			opq->param		= param;
			opq->retVal		= 0;
			if((opq->handle = CreateThread(NULL,					//LPSECURITY_ATTRIBUTES secAttr,
											opq->stackSz,			//SIZE_T stackSize,
											NBThread_runRoutine,	//LPTHREAD_START_ROUTINE threadFunc,
											obj,					//LPVOID param,
											(DWORD)0,				//DWORD flags,
											&opq->threadId			//LPDWORD threadID
											)) == NULL){
				opq->created	= FALSE;
				opq->isRunning	= FALSE;
				NBASSERT(opq->handle == NULL)
				opq->threadId	= 0;
				opq->routine	= NULL;
				opq->param		= NULL;
				opq->retVal		= 0;
			} else {
				r = TRUE;
			}
		}
#		else
		BOOL cfgrd = TRUE;
		pthread_attr_t attrs;
		pthread_attr_init(&attrs);
		//Set stackSz (in bytes)
		if(cfgrd && opq->stackSz > 0){
			if(pthread_attr_setstacksize(&attrs, opq->stackSz) != 0){
				cfgrd = FALSE; NBASSERT(FALSE);
			}
		}
		//Set joinabled or detached (determine when system's resources will be released)
		if(cfgrd){
			if(pthread_attr_setdetachstate(&attrs, (opq->isJoinable ? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED)) != 0){
				cfgrd = FALSE; NBASSERT(FALSE);
			}
		}
		if(cfgrd){
			opq->created	= TRUE;
			opq->isRunning	= TRUE;
			opq->routine	= routine;
			opq->param		= param;
			opq->retVal		= 0;
			int rp;
			if((rp = pthread_create(&opq->thread, &attrs, NBThread_runRoutine, obj)) != 0){
				opq->created	= FALSE;
				opq->isRunning	= FALSE;
				opq->routine	= NULL;
				opq->param		= NULL;
				opq->retVal		= 0;
				//35 == EAGAIN; The system lacked the necessary resources to create another thread;
				if(dstErrIsTemp != NULL) *dstErrIsTemp = (rp == EAGAIN ? TRUE : FALSE);
			} else {
				r = TRUE;
			}
		}
		pthread_attr_destroy(&attrs);
#		endif
	}
	return r;
}

