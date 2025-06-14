#include "nb/NBFrameworkPch.h"
//Complementary PCH
#ifdef _WIN32
//#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>	//for CRITICAL_SECTION
#else
#	include <pthread.h>
#endif
//
#include "nb/NBObject.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBThreadMutex.h"

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
#	include "nb/core/NBMngrProcess.h"
#endif

//--------------
//- NBObjectLock
//--------------

typedef struct STNBObjectMutex_ {
#	ifdef _WIN32
	CRITICAL_SECTION	mutex;
#	else
	pthread_mutex_t		mutex;
#	endif
#	ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
	UI64				uid;	//uniqueId
#	endif
} STNBObjectMutex;

#ifdef _WIN32
#	define	NBObjectMutex_createV(OBJ)		InitializeCriticalSection((CRITICAL_SECTION*)(OBJ))
#	define	NBObjectMutex_destroyV(OBJ)		DeleteCriticalSection((CRITICAL_SECTION*)(OBJ))
#	define	NBObjectMutex_lockV(OBJ)		EnterCriticalSection((CRITICAL_SECTION*)(OBJ))
#	define	NBObjectMutex_unlockV(OBJ)		LeaveCriticalSection((CRITICAL_SECTION*)(OBJ))
#else
#	define	NBObjectMutex_createV(OBJ)		pthread_mutex_init((pthread_mutex_t*)(OBJ), NULL)
#	define	NBObjectMutex_destroyV(OBJ)		pthread_mutex_destroy((pthread_mutex_t*)(OBJ))
#	define	NBObjectMutex_lockV(OBJ)		pthread_mutex_lock((pthread_mutex_t*)(OBJ))
#	define	NBObjectMutex_unlockV(OBJ)		pthread_mutex_unlock((pthread_mutex_t*)(OBJ))
#endif

//----------
//- NBObject
//- Referenceable object (must be allocated at heap,
//- and can be locked and retained)
//----------

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
#		define NBObject_mutexInit(OBJ, NAME, PATH, LINE, FUNC)	{ NBObjectMutex_createV(&((STNBObjectMutex*)OBJ)->mutex); ((STNBObjectMutex*)OBJ)->uid = NBMngrProcess_mutexCreated((NAME), (PATH), (LINE), (FUNC)); }
#		define NBObject_mutexRelease(OBJ, PATH, LINE, FUNC)		{ NBObjectMutex_destroyV(&((STNBObjectMutex*)OBJ)->mutex); NBMngrProcess_mutexDestroyed(((STNBObjectMutex*)OBJ)->uid); ((STNBObjectMutex*)OBJ)->uid = 0; }
#		define NBObject_mutexLock(OBJ, PATH, LINE, FUNC)		{ NBMngrProcess_mutexLockPush(((STNBObjectMutex*)OBJ)->uid, (PATH), (LINE), (FUNC), ENNBThreadLockPushMode_Compete); NBObjectMutex_lockV(&((STNBObjectMutex*)OBJ)->mutex); NBMngrProcess_mutexLockStarted(((STNBObjectMutex*)OBJ)->uid, (PATH), (LINE), (FUNC)); }
#		define NBObject_mutexUnlock(OBJ, PATH, LINE, FUNC)		{ NBMngrProcess_mutexLockPop(((STNBObjectMutex*)OBJ)->uid, (PATH), (LINE), (FUNC)); NBObjectMutex_unlockV(&((STNBObjectMutex*)OBJ)->mutex); }
#else
#		define NBObject_mutexInit(OBJ, NAME, PATH, LINE, FUNC)	NBObjectMutex_createV(&((STNBObjectMutex*)OBJ)->mutex)
#		define NBObject_mutexRelease(OBJ, PATH, LINE, FUNC)		NBObjectMutex_destroyV(&((STNBObjectMutex*)OBJ)->mutex)
#		define NBObject_mutexLock(OBJ, PATH, LINE, FUNC)		NBObjectMutex_lockV(&((STNBObjectMutex*)OBJ)->mutex)
#		define NBObject_mutexUnlock(OBJ, PATH, LINE, FUNC)		NBObjectMutex_unlockV(&((STNBObjectMutex*)OBJ)->mutex)
#endif

void NBObject_initZeroedRecursively(STNBObject* obj){
	//this method will be called by child-classes
	NBASSERT(obj->cls != NULL)
	NBASSERT(obj->refCount > 0)
}

void NBObject_uninitLockedRecursively(STNBObject* obj){
	//this method will be called by child-classes
	NBASSERT(obj->cls != NULL)
	NBASSERT(obj->refCount == 0)
}

void NBObject_initZeroed(STNBObject* obj){
	//this method will be called by child-classes
	NBASSERT(obj->cls != NULL)
	NBASSERT(obj->refCount > 0)
}

void NBObject_uninitLocked(STNBObject* obj){
	//this method will be called by child-classes
	NBASSERT(obj->cls != NULL)
	NBASSERT(obj->refCount == 0)
}

void NBObject_initZeroedInternal_(STNBObject* obj, const STNBClass* cls, STNBContext* ctx
#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
, const char* name, const char* fullpath, const SI32 line, const char* func
#endif
){
	//Expected to be zeroed by caller
	obj->cls	= cls;
	obj->ctx	= ctx;
	//mutex
	{
		obj->mutex = (STNBObjectMutex*)NBMemory_alloc(sizeof(STNBObjectMutex));
		NBObject_mutexInit(obj->mutex, name, fullpath, line, func);
	}
	//retain count
	obj->refCount = 1;
#   ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
    obj->objId = NBMngrProcess_objCreated(name, fullpath, line, func);
#   endif
}

void NBObject_retainInternal_(STNBObject* obj
#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
, const char* fullpath, const SI32 line, const char* func
#endif
){
	NBObject_mutexLock(obj->mutex, fullpath, line, func);
	{
		NBASSERT(obj->refCount > 0)
		obj->refCount++;
	}
	NBObject_mutexUnlock(obj->mutex, fullpath, line, func);
}

BOOL NBObject_releaseInternal_(STNBObject* obj
#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
, const char* fullpath, const SI32 line, const char* func
#endif
){
	BOOL r = FALSE;
	NBObject_mutexLock(obj->mutex, fullpath, line, func);
	NBASSERT(obj->refCount > 0)
	obj->refCount--;
	if(obj->refCount > 0){
		NBObject_mutexUnlock(obj->mutex, fullpath, line, func);
	} else {
		void* mutex = obj->mutex;
#       ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
        NBASSERT(obj->objId > 0)
        if(obj->objId > 0){
            NBMngrProcess_objDestroyed(obj->objId);
            obj->objId = 0;
        }
#       endif
		//Release myself
		{
			NBASSERT(obj->cls != NULL)
			if(obj->cls != NULL){
				if(obj->cls->calls.uninitLockedRecursively != NULL){
					(*obj->cls->calls.uninitLockedRecursively)(obj);
				}
				obj->cls = NULL;
			}
			obj->ctx = NULL;
			NBMemory_free(obj);
		}
		NBObject_mutexUnlock(mutex, fullpath, line, func);
		//release orphaned mutex
		NBObject_mutexRelease(mutex, fullpath, line, func);
		NBMemory_free(mutex);
		//
		r = TRUE;
	}
	return r;
}

//

UI32 NBObject_retainCount(STNBObject* obj){
	return obj->refCount;
}

//mutex

void NBObject_lock_(void* obj
#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
, const char* fullpath, const SI32 line, const char* func
#endif
){
	STNBObject* obj2 = (STNBObject*)obj;
	NBASSERT(obj2->cls != NULL)
	NBObject_mutexLock(obj2->mutex, fullpath, line, func);
}

void NBObject_unlock_(void* obj
#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
, const char* fullpath, const SI32 line, const char* func
#endif
){
	STNBObject* obj2 = (STNBObject*)obj;
	NBASSERT(obj2->cls != NULL)
	NBObject_mutexUnlock(obj2->mutex, fullpath, line, func);
}

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
UI64 NBObject_getMutexId(void* obj){
	STNBObject* obj2 = (STNBObject*)obj;
	return ((STNBObjectMutex*)obj2->mutex)->uid;
}
#endif

//Class

static const STNBClass NBObject_class_ = {
	NB_CLASS_HEAD_32, 
	"NBObject",
	{
		NBObject_initZeroedRecursively,
		NBObject_uninitLockedRecursively,
		NBObject_getObjSz,
		NULL //parent class (none)
	}
};

BOOL NBObject_isClass(void* ptr){
	return TRUE; //ToDo: implement
}

UI32 NBObject_getObjSz(void){
	return sizeof(STNBObject);
}

const STNBClass* NBObject_getClass(void){
	return &NBObject_class_;
}

//----------
//- NBObjRef
//- Referenceable object (object is allocated at heap,
//- and can be locked and retained)
//----------

STNBObjRef NBObjRef_create_(STNBContext* ctx, const STNBClass* cls
#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
, const char* name, const char* fullpath, const SI32 line, const char* func
#endif
){
	STNBObjRef r;
	r.opaque = NULL;
	if(cls != NULL){
		if(cls->calls.getObjSz != NULL && cls->calls.initZeroedRecursively != NULL){
			const UI32 sz = (*cls->calls.getObjSz)();
			NBASSERT(sz >= sizeof(STNBObject))
			if(sz >= sizeof(STNBObject)){
				STNBObject* obj = (STNBObject*)NBMemory_alloc(sz);
				NBMemory_set(obj, 0, sz);
				NBObject_initZeroedInternal(obj, cls, ctx, name, fullpath, line, func);
				//Init opaque
				(*cls->calls.initZeroedRecursively)(obj);
				//validate init-final-state
				NBASSERT(obj->ctx == ctx && obj->cls == cls && obj->refCount > 0)
				//return
				r.opaque = obj;
			}
		}
	}
	return r;
}

//Itf methods
void NBObjRef_retainOpq(void* opqPtr){
	NBASSERT(opqPtr != NULL)
	NBObject_retainInternal(opqPtr, __FILE__, __LINE__, __func__);
}

void NBObjRef_releaseOpq(void* opqPtr){
	NBASSERT(opqPtr != NULL)
	NBObject_releaseInternal(opqPtr, __FILE__, __LINE__, __func__);
}

BOOL NBObjRef_objLnkIsSameObjectOpq_(STNBObject* obj, void* usrData){
    return (usrData != NULL && obj != NULL && obj == (STNBObject*)usrData);
}

BOOL NBObjRef_objLnkIsSameObjRefOpq_(STNBObjRef* ref, void* usrData){
    return (usrData != NULL && ref != NULL && ref->opaque == usrData);
}

void NBObjRef_retain_(STNBObjRef ref
#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
, const char* fullpath, const SI32 line, const char* func
#endif
){
	STNBObject* obj = (STNBObject*)ref.opaque;
	if(obj != NULL){
		NBObject_retainInternal(obj, fullpath, line, func);
	}
}

void NBObjRef_release_(STNBObjRef* ref
#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
, const char* fullpath, const SI32 line, const char* func
#endif
){
	STNBObject* obj = (STNBObject*)ref->opaque;
	if(obj != NULL){
		if(NBObject_releaseInternal(obj, fullpath, line, func)){
			ref->opaque = NULL;
		}
	}
}

void NBObjRef_set_(STNBObjRef* ref, const STNBObjRef* other
#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
, const char* fullpath, const SI32 line, const char* func
#endif
){
	//Retain other
	if(other != NULL && other->opaque != NULL){
		STNBObject* obj2 = (STNBObject*)other->opaque;
		NBObject_retainInternal(obj2, fullpath, line, func);
	}
	//Release current
	if(ref->opaque != NULL){
		STNBObject* obj2 = (STNBObject*)ref->opaque;
		NBObject_releaseInternal(obj2, fullpath, line, func);
	}
	//Set
	if(other == NULL){
		ref->opaque = NULL;
	} else {
		ref->opaque = other->opaque;
	}
}

//Actions

UI32 NBObjRef_retainCount(STNBObjRef ref){
	return ((STNBObject*)ref.opaque)->refCount;
}

BOOL NBObjRef_isClass(STNBObjRef ref, const STNBClass* cls){
	BOOL r = FALSE;
	STNBObject* obj2 = (STNBObject*)ref.opaque;
	if(obj2 != NULL){
		const STNBClass* cls2 = obj2->cls;
		while(cls2 != NULL){
			//Compare
			if(cls == cls2){
				r = TRUE;
				break;
			}
			//Parent
			if(cls2->calls.getParentClass != NULL){
				cls2 = (*cls2->calls.getParentClass)();
			} else {
				cls2 = NULL;
			}
		}
	}
	return r;
}

