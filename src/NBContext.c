#include "nb/NBFrameworkPch.h"
#include "nb/NBContext.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBThreadMutex.h"

//

typedef struct STNBContextOpq_ {
	STNBThreadMutex	mutex;
	SI32			retainCount;
} STNBContextOpq;

void NBContext_init(STNBContext* obj){
	STNBContextOpq* opq	= obj->opaque = NBMemory_allocType(STNBContextOpq);
	NBMemory_setZeroSt(*opq, STNBContextOpq);
	//
	NBThreadMutex_init(&opq->mutex);
	opq->retainCount	= 1;
}

void NBContext_retain(STNBContext* obj){
	STNBContextOpq* opq = (STNBContextOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	{
		NBASSERT(opq->retainCount > 0)
		opq->retainCount++;
	}
	NBThreadMutex_unlock(&opq->mutex);
}

void NBContext_release(STNBContext* obj){
	STNBContextOpq* opq = (STNBContextOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	{
		NBASSERT(opq->retainCount > 0)
		opq->retainCount--;
		if(opq->retainCount == 0){
			NBThreadMutex_unlock(&opq->mutex);
		} else {
			NBThreadMutex_unlock(&opq->mutex);
			NBThreadMutex_release(&opq->mutex);
			//Delete opaque
			NBMemory_free(opq);
			obj->opaque = NULL;
		}
	}
}
