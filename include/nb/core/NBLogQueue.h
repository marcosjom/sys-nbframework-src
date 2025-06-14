
#ifndef NB_LOG_QUEUE_H
#define NB_LOG_QUEUE_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBThreadMutex.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef enum ENNBLogQueueItemType_ {
		ENNBLogQueueItemType_Info = 0,
		ENNBLogQueueItemType_Warning,
		ENNBLogQueueItemType_Error
	} ENNBLogQueueItemType;
	
	typedef struct STNBLogItem_ {
		ENNBLogQueueItemType type;
		STNBString logMessage;
	} STNBLogItem;
	
	typedef struct STNBLogQueue_ {
		SI32		maxSize;
		STNBArray	items;	//STNBLogItem
		//
		STNBThreadMutex mutex;
	} STNBLogQueue;
	
	
	//Factory
	void NBLogQueue_init(STNBLogQueue* obj, const SI32 maxSize);
	void NBLogQueue_release(STNBLogQueue* obj);
	//
	void NBLogQueue_addMessage(STNBLogQueue* obj, const ENNBLogQueueItemType type, const char* strMessage);
	void NBLogQueue_addMessageNoLock(STNBLogQueue* obj, const ENNBLogQueueItemType type, const char* strMessage);
	void NBLogQueue_empty(STNBLogQueue* obj);
	void NBLogQueue_emptyNoLock(STNBLogQueue* obj);
	//
	void NBLogQueue_queueGetNoLock(STNBLogQueue* obj, STNBLogItem** dstPointer, SI32* dstSize);
	//
	void NBLogQueue_queueLock(STNBLogQueue* obj);
	void NBLogQueue_queueUnlock(STNBLogQueue* obj);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
