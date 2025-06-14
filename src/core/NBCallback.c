//
//  NBCallback.c
//  nbframework
//
//  Created by Marcos Ortega on 13/3/19.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBCallback.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBThreadMutex.h"

typedef struct STNBCallbackOpqq_ {
	BOOL				active;
	STNBCallbackMethod	mthds;
	STNBThreadMutex		mutex;
	SI32				retainCount;
} STNBCallbackOpqq;

typedef struct STNBCallbackOpq_ {
	BOOL				isRef;
	STNBCallbackOpqq*	shared;
} STNBCallbackOpq;

void NBCallback_init(STNBCallback* obj){
	obj->opaque = NULL;
}

void NBCallback_initWithMethods(STNBCallback* obj, STNBCallbackMethod* mthds){
	STNBCallbackOpq* opq	= obj->opaque = NBMemory_allocType(STNBCallbackOpq);
	NBMemory_setZeroSt(*opq, STNBCallbackOpq);
	opq->isRef				= FALSE;
	//Shared
	{
		opq->shared			= NBMemory_allocType(STNBCallbackOpqq);
		NBMemory_setZeroSt(*opq->shared, STNBCallbackOpqq);
		opq->shared->active	= TRUE;
		if(mthds != NULL){
			opq->shared->mthds = *mthds;
		}
		NBThreadMutex_init(&opq->shared->mutex);
		opq->shared->retainCount = 1;
	}
}

void NBCallback_initAsRefOf(STNBCallback* obj, const STNBCallback* other){
	obj->opaque = NULL;
	if(other->opaque != NULL){
		STNBCallbackOpq* opq = (STNBCallbackOpq*)other->opaque;
		STNBCallbackOpqq* shared = opq->shared;
		NBThreadMutex_lock(&shared->mutex);
		NBASSERT(shared->retainCount > 0)
		{
			STNBCallbackOpq* opq = obj->opaque = NBMemory_allocType(STNBCallbackOpq);
			NBMemory_setZeroSt(*opq, STNBCallbackOpq);
			opq->isRef		= TRUE;
			opq->shared		= shared;
			opq->shared->retainCount++;
		}
		NBThreadMutex_unlock(&shared->mutex);
	}
}

void NBCallback_release(STNBCallback* obj){
	STNBCallbackOpq* opq = (STNBCallbackOpq*)obj->opaque;
	if(opq != NULL){
		NBThreadMutex_lock(&opq->shared->mutex);
		NBASSERT(opq->shared->retainCount > 0)
		//Invalidate
		if(!opq->isRef){
			NBMemory_setZeroSt(opq->shared->mthds, STNBCallbackMethod);
			//PRINTF_INFO("NBCallback invalidated with %d retains remainig.\n", (opq->shared->retainCount - 1));
		}
		//Release shared
		{
			opq->shared->retainCount--;
			if(opq->shared->retainCount == 0){
				opq->shared->active		= FALSE;
				NBMemory_setZeroSt(opq->shared->mthds, STNBCallbackMethod);
				NBThreadMutex_unlock(&opq->shared->mutex);
				NBThreadMutex_release(&opq->shared->mutex);
				NBMemory_free(opq->shared);
				opq->shared = NULL;
			} else {
				NBThreadMutex_unlock(&opq->shared->mutex);
			}
		}
		//Free opaque
		NBMemory_free(opq);
		obj->opaque = NULL;
	}
}


void NBCallback_call(STNBCallback* obj){
	STNBCallbackOpq* opq = (STNBCallbackOpq*)obj->opaque;
	if(opq != NULL){
		STNBCallbackMethod m;
		NBMemory_setZeroSt(m, STNBCallbackMethod);
		//Retain and obtain method (locked)
		{
			NBThreadMutex_lock(&opq->shared->mutex);
			NBASSERT(opq->shared->retainCount > 0)
			if(opq->shared->active){
				m = opq->shared->mthds;
				NBASSERT((m.retain != NULL && m.release != NULL) || (m.retain == NULL && m.release == NULL))
				if(m.retain != NULL && m.release != NULL){
					(*m.retain)(m.obj);
				}
			}
			NBThreadMutex_unlock(&opq->shared->mutex);
		}
		//Call methods (unlocked)
		{
			//Call
			if(m.callback != NULL){
				(*m.callback)(m.obj);
			}
			//Release
			if(m.retain != NULL && m.release != NULL){
				(*m.release)(m.obj);
			}
		}
	}
}

void NBCallback_setActive(STNBCallback* obj, const BOOL active){
	STNBCallbackOpq* opq = (STNBCallbackOpq*)obj->opaque;
	if(opq != NULL){
		NBThreadMutex_lock(&opq->shared->mutex);
		{
			opq->shared->active = active;
		}
		NBThreadMutex_unlock(&opq->shared->mutex);
	}
}
