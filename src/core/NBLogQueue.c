
#include "nb/NBFrameworkPch.h"
#include "nb/core/NBLogQueue.h"
#include "nb/core/NBMemory.h"

void NBLogQueue_init(STNBLogQueue* obj, const SI32 maxSize){
	obj->maxSize	= maxSize;
    NBArray_initWithSz(&obj->items, sizeof(STNBLogItem), NULL, maxSize + 1, 1024, 0.5f);
	//
	NBThreadMutex_init(&obj->mutex);
}

void NBLogQueue_release(STNBLogQueue* obj){
	obj->maxSize	= 0;
	//
	{
		SI32 i;
		NBThreadMutex_lock(&obj->mutex);
		for(i = 0; i < obj->items.use; i++){
			STNBLogItem* logItem = (STNBLogItem*)NBArray_itemAtIndex(&obj->items, i);
		    NBString_release(&logItem->logMessage);
		}
	    NBArray_empty(&obj->items);
	    NBArray_release(&obj->items);
		NBThreadMutex_unlock(&obj->mutex);
	}
	//
	NBThreadMutex_release(&obj->mutex);
}

void NBLogQueue_empty(STNBLogQueue* obj){
	NBThreadMutex_lock(&obj->mutex);
    NBLogQueue_emptyNoLock(obj);
	NBThreadMutex_unlock(&obj->mutex);
}

void NBLogQueue_emptyNoLock(STNBLogQueue* obj){
	SI32 i;
	for(i = 0; i < obj->items.use; i++){
		STNBLogItem* logItem = (STNBLogItem*)NBArray_itemAtIndex(&obj->items, i);
	    NBString_release(&logItem->logMessage);
	}
    NBArray_empty(&obj->items);
}

void NBLogQueue_addMessage(STNBLogQueue* obj, const ENNBLogQueueItemType type, const char* strMessage){
	NBThreadMutex_lock(&obj->mutex);
    NBLogQueue_addMessageNoLock(obj, type, strMessage);
	NBThreadMutex_unlock(&obj->mutex);
}

void NBLogQueue_addMessageNoLock(STNBLogQueue* obj, const ENNBLogQueueItemType type, const char* strMessage){
	STNBLogItem newMesg;
	newMesg.type = type;
    NBString_init(&newMesg.logMessage);
    NBString_concat(&newMesg.logMessage, strMessage);
    NBArray_addValue(&obj->items, newMesg);
	//Remove olds logs (max size)
	while(obj->items.use > obj->maxSize){
		STNBLogItem* logItem = (STNBLogItem*)NBArray_itemAtIndex(&obj->items, 0);
	    NBString_release(&logItem->logMessage);
	    NBArray_removeItemAtIndex(&obj->items, 0);
	}	
}

void NBLogQueue_queueLock(STNBLogQueue* obj){
	NBThreadMutex_lock(&obj->mutex);
}

void NBLogQueue_queueGetNoLock(STNBLogQueue* obj, STNBLogItem** dstPointer, SI32* dstSize){
	//Note: this call requieres the queue be locked by the user using 'DRLogQueue_queueLock'
	if(obj->items.use == 0){
		*dstPointer = NULL;
		*dstSize = 0;
	} else {
		*dstPointer = (STNBLogItem*)NBArray_itemAtIndex(&obj->items, 0);
		*dstSize = obj->items.use;
	}
}

void NBLogQueue_queueUnlock(STNBLogQueue* obj){
	NBThreadMutex_unlock(&obj->mutex);
}
//
