
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBRtpQueue.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBThreadMutex.h"
//
#include "nb/core/NBStructMaps.h"
#include "nb/core/NBMngrStructMaps.h"

// NBRtpQueueCount32

STNBStructMapsRec NBRtpQueueCount32_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtpQueueCount32_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBRtpQueueCount32_sharedStructMap);
    if(NBRtpQueueCount32_sharedStructMap.map == NULL){
        STNBRtpQueueCount32 s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtpQueueCount32);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, count);
        NBStructMap_addUIntM(map, s, bytesCount);
        NBRtpQueueCount32_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBRtpQueueCount32_sharedStructMap);
    return NBRtpQueueCount32_sharedStructMap.map;
}

// NBRtpQueueCount32Seq

STNBStructMapsRec NBRtpQueueCount32Seq_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtpQueueCount32Seq_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBRtpQueueCount32Seq_sharedStructMap);
    if(NBRtpQueueCount32Seq_sharedStructMap.map == NULL){
        STNBRtpQueueCount32Seq s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtpQueueCount32Seq);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, count);
        NBStructMap_addUIntM(map, s, bytesCount);
        NBStructMap_addUIntM(map, s, gapMax);
        NBRtpQueueCount32Seq_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBRtpQueueCount32Seq_sharedStructMap);
    return NBRtpQueueCount32Seq_sharedStructMap.map;
}

// NBRtpQueueCount64

STNBStructMapsRec NBRtpQueueCount64_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtpQueueCount64_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBRtpQueueCount64_sharedStructMap);
    if(NBRtpQueueCount64_sharedStructMap.map == NULL){
        STNBRtpQueueCount64 s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtpQueueCount64);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, count);
        NBStructMap_addUIntM(map, s, bytesCount);
        NBRtpQueueCount64_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBRtpQueueCount64_sharedStructMap);
    return NBRtpQueueCount64_sharedStructMap.map;
}

// NBRtpQueueCount64Seq

STNBStructMapsRec NBRtpQueueCount64Seq_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtpQueueCount64Seq_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBRtpQueueCount64Seq_sharedStructMap);
    if(NBRtpQueueCount64Seq_sharedStructMap.map == NULL){
        STNBRtpQueueCount64Seq s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtpQueueCount64Seq);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, count);
        NBStructMap_addUIntM(map, s, bytesCount);
        NBStructMap_addUIntM(map, s, gapMax);
        NBRtpQueueCount64Seq_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBRtpQueueCount64Seq_sharedStructMap);
    return NBRtpQueueCount64Seq_sharedStructMap.map;
}

// NBRtpPckIdx

STNBStructMapsRec NBRtpPckIdx_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtpPckIdx_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBRtpPckIdx_sharedStructMap);
    if(NBRtpPckIdx_sharedStructMap.map == NULL){
        STNBRtpPckIdx s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtpPckIdx);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, seqCicle);
        NBStructMap_addUIntM(map, s, seqNum);
        NBStructMap_addUIntM(map, s, timestamp);
        NBRtpPckIdx_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBRtpPckIdx_sharedStructMap);
    return NBRtpPckIdx_sharedStructMap.map;
}

// NBRtpStatsCsmUpd

STNBStructMapsRec NBRtpStatsCsmUpd_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtpStatsCsmUpd_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBRtpStatsCsmUpd_sharedStructMap);
    if(NBRtpStatsCsmUpd_sharedStructMap.map == NULL){
        STNBRtpStatsCsmUpd s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtpStatsCsmUpd);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, count);
        NBStructMap_addUIntM(map, s, bytesCount);
        NBStructMap_addStructM(map, s, ignored, NBRtpQueueCount32_getSharedStructMap());
        NBStructMap_addStructM(map, s, passedThrough, NBRtpQueueCount64_getSharedStructMap());
        NBStructMap_addStructM(map, s, queued, NBRtpQueueCount32_getSharedStructMap());
        NBStructMap_addStructM(map, s, unqueued, NBRtpQueueCount32_getSharedStructMap());
        NBStructMap_addStructM(map, s, delayed, NBRtpQueueCount32_getSharedStructMap());
        NBStructMap_addStructM(map, s, repeated, NBRtpQueueCount32_getSharedStructMap());
        NBStructMap_addStructM(map, s, ignLate, NBRtpQueueCount32Seq_getSharedStructMap());
        NBStructMap_addStructM(map, s, lost, NBRtpQueueCount32_getSharedStructMap());
        NBStructMap_addStructM(map, s, overQueue, NBRtpQueueCount32_getSharedStructMap());
        NBRtpStatsCsmUpd_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBRtpStatsCsmUpd_sharedStructMap);
    return NBRtpStatsCsmUpd_sharedStructMap.map;
}

//RtpPacketSlot

typedef struct STNBRtpQueueSlot_ {
	//Rtp packet
	UI32				seqCicle;	//iteration of seqNum (UI16) this packet belongs
	UI16				seqNum;		//16 bits, sequence number (random start)
	STNBRtpHdrBasic		head;		//parsed header
	STNBDataPtr			chunk;		//packet data
	//
	BOOL				isPopulated;
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	BOOL				dbgIsSlotOcupied;	//used to verify queue management
#	endif
} STNBRtpQueueSlot;

void NBRtpQueueSlot_init(STNBRtpQueueSlot* obj);
void NBRtpQueueSlot_release(STNBRtpQueueSlot* obj);
void NBRtpQueueSlot_swapBuffPtr(STNBRtpQueueSlot* obj, STNBDataPtr* chunk);
void NBRtpQueueSlot_setNoData(STNBRtpQueueSlot* obj, const UI32 seqCicle, const UI16 seqNum);
void NBRtpQueueSlot_setWithData(STNBRtpQueueSlot* obj, const UI32 seqCicle, const STNBRtpHdrBasic* head, STNBDataPtr* chunk);
void NBRtpQueueSlot_setWithDataUnocupied(STNBRtpQueueSlot* obj, const UI32 seqCicle, const STNBRtpHdrBasic* head, STNBDataPtr* chunk);
void NBRtpQueueSlot_updateWithData(STNBRtpQueueSlot* obj, const UI32 seqCicle, const STNBRtpHdrBasic* head, STNBDataPtr* chunk);
void NBRtpQueueSlot_unset(STNBRtpQueueSlot* obj);

void NBRtpQueueSlot_init(STNBRtpQueueSlot* obj){
	NBMemory_setZeroSt(*obj, STNBRtpQueueSlot);
	NBRtpHdrBasic_init(&obj->head);
	NBDataPtr_init(&obj->chunk);
}

void NBRtpQueueSlot_release(STNBRtpQueueSlot* obj){
	NBRtpHdrBasic_release(&obj->head);
	NBDataPtr_release(&obj->chunk);
}

void NBRtpQueueSlot_swapBuffPtr(STNBRtpQueueSlot* obj, STNBDataPtr* chunk){
	STNBDataPtr tmp	= *chunk;
	*chunk			= obj->chunk;
	obj->chunk		= tmp;
}

void NBRtpQueueSlot_setNoData(STNBRtpQueueSlot* obj, const UI32 seqCicle, const UI16 seqNum){
	NBASSERT(!obj->dbgIsSlotOcupied)
	NBASSERT(!obj->isPopulated)
	obj->seqCicle		= seqCicle;
	obj->seqNum			= seqNum;
	obj->isPopulated	= FALSE;
	IF_NBASSERT(obj->dbgIsSlotOcupied = TRUE;)
}

void NBRtpQueueSlot_setWithData(STNBRtpQueueSlot* obj, const UI32 seqCicle, const STNBRtpHdrBasic* head, STNBDataPtr* chunk){
	NBASSERT(!obj->dbgIsSlotOcupied)
	NBASSERT(!obj->isPopulated)
	NBASSERT(head != NULL)
	NBASSERT(chunk != NULL)
	obj->seqCicle		= seqCicle;
	obj->seqNum			= head->head.seqNum;
	obj->isPopulated	= TRUE;
	NBRtpHdrBasic_copy(&obj->head, head);
	NBDataPtr_swapCompatibleOther(&obj->chunk, chunk);
	IF_NBASSERT(obj->dbgIsSlotOcupied = TRUE;)
}

void NBRtpQueueSlot_setWithDataUnocupied(STNBRtpQueueSlot* obj, const UI32 seqCicle, const STNBRtpHdrBasic* head, STNBDataPtr* chunk){
	//Optimization: equivalent to 'NBRtpQueueSlot_setWithData()' + 'NBRtpQueueSlot_unset()'
	NBASSERT(!obj->dbgIsSlotOcupied)
	NBASSERT(!obj->isPopulated)
	NBASSERT(head != NULL)
	NBASSERT(chunk != NULL)
	obj->seqCicle		= seqCicle;
	obj->seqNum			= head->head.seqNum;
	NBRtpHdrBasic_copy(&obj->head, head);
	NBDataPtr_swapCompatibleOther(&obj->chunk, chunk);
}

void NBRtpQueueSlot_updateWithData(STNBRtpQueueSlot* obj, const UI32 seqCicle, const STNBRtpHdrBasic* head, STNBDataPtr* chunk){
	NBASSERT(obj->dbgIsSlotOcupied)
	NBASSERT(!obj->isPopulated)
	NBASSERT(head != NULL)
	NBASSERT(chunk != NULL)
	obj->seqCicle		= seqCicle;
	obj->seqNum			= head->head.seqNum;
	obj->isPopulated	= TRUE;
	NBRtpHdrBasic_copy(&obj->head, head);
	NBDataPtr_swapCompatibleOther(&obj->chunk, chunk);
	IF_NBASSERT(obj->dbgIsSlotOcupied = TRUE;)
}

void NBRtpQueueSlot_unset(STNBRtpQueueSlot* obj){
	NBASSERT(obj->dbgIsSlotOcupied)
	obj->isPopulated		= FALSE;
	IF_NBASSERT(obj->dbgIsSlotOcupied	= FALSE;)
}

//-----------------
//NBRtpQueue MACROS
//-----------------

//Auto-safe action (depends of 'unsafe.isEnabled')

#define NB_RTP_QUEUE_AUTOSAFE_ACTION_START(OPQ) \
	BOOL _locked_ = FALSE; \
	NBASSERT((OPQ) != NULL) \
	NBASSERT((OPQ)->unsafe.depth == 0) /*user logic error, unsafe mode must be enabled and only serial calls are allowed*/ \
	if((OPQ)->unsafe.isEnabled) {\
		(OPQ)->unsafe.depth++; \
	} else { \
		NBThreadMutex_lock(&(OPQ)->mutex); \
		_locked_ = TRUE; \
	} \
	NBASSERT((OPQ)->retainCount > 0)

#define NB_RTP_QUEUE_AUTOSAFE_ACTION_END(OPQ) \
	if(_locked_){ \
		NBThreadMutex_unlock(&(OPQ)->mutex); \
	} else { \
		NBASSERT((OPQ)->unsafe.depth > 0) /*program logic error*/ \
		(OPQ)->unsafe.depth--; \
	}

//Unsafe action (depends of 'unsafe.isEnabled' but wont touch 'unsafe.depth' value)

#define NB_RTP_QUEUE_UNSAFE_ACTION_START(OPQ) \
	BOOL _locked_ = FALSE; \
	NBASSERT((OPQ) != NULL) \
	if(!(OPQ)->unsafe.isEnabled) {\
		NBThreadMutex_lock(&(OPQ)->mutex); \
		_locked_ = TRUE; \
	} \
	NBASSERT((OPQ)->retainCount > 0)

#define NB_RTP_QUEUE_UNSAFE_ACTION_END(OPQ) \
	if(_locked_){ \
		NBThreadMutex_unlock(&(OPQ)->mutex); \
	}

//Safe action (forbids 'unsafe.isEnabled' and 'unsafe.depth')

#define NB_RTP_QUEUE_SAFE_ACTION_START(OPQ) \
	NBASSERT((OPQ) != NULL) \
	NBASSERT((OPQ)->unsafe.depth == 0) /*user logic error, unsafe mode must be disabled and not action should be in progress*/ \
	NBASSERT(!(OPQ)->unsafe.isEnabled) /*user logic error, unsafe mode must be disabled and not action should be in progress*/ \
	NBThreadMutex_lock(&(OPQ)->mutex); \
	NBASSERT((OPQ)->retainCount > 0)

#define NB_RTP_QUEUE_SAFE_ACTION_END(OPQ) \
	NBThreadMutex_unlock(&(OPQ)->mutex);
	

//------------------------
//- NBRtpQueueBuffProvider
//------------------------

typedef struct STNBRtpQueueBuffProvider_ {
	STNBRtpQueueBuffProviderItf itf;
	void*						usrData;
} STNBRtpQueueBuffProvider;

//------------
//- NBRtpQueue
//------------

//Opaque state

typedef struct STNBRtpQueueOpq_ {
	STNBThreadMutex			mutex;
	UI32					bytesPerPacket;
	//received
	struct {
		BOOL				isFirstKnown;	//First packet received (to determine the first seqNum received)
		STNBRtpPckIdx		lowest;			//lowest (first) packet
		STNBRtpPckIdx		highest;		//highest packet
	} rcvd;
	//notified
	struct {
		BOOL				isFirstKnown;	//First packet notified (to determine the first seqNum notified)
		UI32				lastSeqNum;		//last notified (to detect gaps)
	} notified;
	//buffers
	struct {
		STNBDataPtr*		arr;
		UI32				use;
		UI32				size;
		STNBRtpQueueBuffProvider provider;
	} buffs;
	//circular queue
	struct {
		UI32				iFirst;	//First element in queue
		UI32				use;	//Items ocupied after iFirst
		UI32				size;	//size of queue
		UI32				bytesPerPacket;	//size of packets
		STNBRtpQueueSlot*	itms;	//queue
	} queue;
	//unsafe mode (optimization)
	struct {
		BOOL 				isEnabled;
		UI32				depth;
	} unsafe;
	//
	SI32					retainCount;
} STNBRtpQueueOpq;

void NBRtpQueue_init(STNBRtpQueue* obj){
	STNBRtpQueueOpq* opq = obj->opaque = NBMemory_allocType(STNBRtpQueueOpq);
	NBMemory_setZeroSt(*opq, STNBRtpQueueOpq);
	NBThreadMutex_init(&opq->mutex);
	//
	opq->retainCount	= 1;
}

void NBRtpQueue_retain_(STNBRtpQueueOpq* opq){
	NB_RTP_QUEUE_SAFE_ACTION_START(opq)
	{
		opq->retainCount++;
	}
	NB_RTP_QUEUE_SAFE_ACTION_END(opq)
}


void NBRtpQueue_retain(STNBRtpQueue* obj){
	NBRtpQueue_retain_((STNBRtpQueueOpq*)obj->opaque);
}

void NBRtpQueue_release_(STNBRtpQueueOpq* opq){
	NBASSERT(opq != NULL)
	NB_RTP_QUEUE_SAFE_ACTION_START(opq)
	opq->retainCount--;
	if(opq->retainCount > 0){
		NB_RTP_QUEUE_SAFE_ACTION_END(opq)
	} else {
		//buffer
		{
			if(opq->buffs.arr != NULL){
				if(opq->buffs.use > 0){
					NBDataPtr_ptrsReleaseGrouped(opq->buffs.arr, opq->buffs.use);
				}
				NBMemory_free(opq->buffs.arr);
				opq->buffs.use = 0;
			}
			opq->buffs.size = 0;
		}
		//circular queue
		{ 
			if(opq->queue.itms != NULL){
				UI32 i; for(i = 0; i < opq->queue.size; i++){
					STNBRtpQueueSlot* slot = &opq->queue.itms[i];
					NBRtpQueueSlot_release(slot);
				}
				NBMemory_free(opq->queue.itms);
				opq->queue.itms = NULL;
			}
			opq->queue.size = 0;
		}
		{
			NBMemory_setZeroSt(opq->buffs.provider, STNBRtpQueueBuffProvider);
		}
		NB_RTP_QUEUE_SAFE_ACTION_END(opq)
		NBThreadMutex_release(&opq->mutex);
		NBMemory_free(opq);
		opq = NULL;
	}
}

void NBRtpQueue_release(STNBRtpQueue* obj){
	NBRtpQueue_release_((STNBRtpQueueOpq*)obj->opaque);
}

//Config

BOOL NBRtpQueue_setBuffersProvider(STNBRtpQueue* obj, STNBRtpQueueBuffProviderItf* itf, void* usrData){
	BOOL r = FALSE;
	STNBRtpQueueOpq* opq = (STNBRtpQueueOpq*)obj->opaque;
	NB_RTP_QUEUE_SAFE_ACTION_START(opq)
	{
		if(itf == NULL){
			NBMemory_setZeroSt(opq->buffs.provider, STNBRtpQueueBuffProvider);
		} else {
			opq->buffs.provider.itf = *itf;
			opq->buffs.provider.usrData = usrData;
		}
	}
	NB_RTP_QUEUE_SAFE_ACTION_END(opq)
	return r;
}

void NBRtpQueue_fillBufferLockedOpq_(STNBDataPtr* arr, UI32* use, const UI32 size, STNBRtpQueueBuffProvider* provider, const UI32 bytesPerPacket){
	if(bytesPerPacket > 0){
		NBASSERT(*use == 0)
		//const UI32 useBefore = *use; 
		while(*use < size){
			STNBDataPtr* ptr = &arr[(*use)++];
			NBDataPtr_init(ptr);
		}
		{
			IF_NBASSERT(UI32 dbgFromPollCount = 0, dbgFromAllocCount = 0;)
			UI32 i = 0;
			while(i < *use){
				STNBDataPtr* ptr = &arr[i];
				//pool allocation
				if(provider != NULL && provider->itf.getPtrs != NULL){
					const UI32 allocated = (*provider->itf.getPtrs)(bytesPerPacket, ptr, (*use - i), provider->usrData);
					if(allocated > 0){
						IF_NBASSERT(dbgFromPollCount += allocated;)
						i += allocated;
						continue;
					}
				}
				//Individual allocation (if no-pool or pool-failed)
				if(i < *use){
					NBDataPtr_allocEmptyPtr(ptr, bytesPerPacket);
					IF_NBASSERT(dbgFromAllocCount++;)
					i++;
				}
			}
#			ifdef NB_CONFIG_INCLUDE_ASSERTS
			{
				UI32 i = 0;
				while(i < *use){
					NBASSERT(arr[i].use == 0) //should be not in use
					NBASSERT(NBDataPtr_allocatedSize(&arr[i]) >= bytesPerPacket) //should be big enough
					i++;
				}
			}
#			endif
		}
		//PRINTF_INFO("NBRtpQueue, %d packets allocated for buffer.\n", (*use - useBefore));
	}
}

void NBRtpQueue_setSize(STNBRtpQueue* obj, const UI32 bytesPerPacket, const UI32 queueSize, const UI32 ptrsBuffSize){
	STNBRtpQueueOpq* opq = (STNBRtpQueueOpq*)obj->opaque;
	NB_RTP_QUEUE_SAFE_ACTION_START(opq)
	//Buffer
	if(opq->queue.bytesPerPacket != bytesPerPacket || opq->buffs.size != ptrsBuffSize){
		//release
		{
			if(opq->buffs.arr != NULL){
				if(opq->buffs.use > 0){
					NBDataPtr_ptrsReleaseGrouped(opq->buffs.arr, opq->buffs.use);
				}
				NBMemory_free(opq->buffs.arr);
				opq->buffs.use = 0;
			}
			opq->buffs.size = 0;
		}
		//create
		if(ptrsBuffSize > 0 && bytesPerPacket > 0){
			opq->buffs.size	= ptrsBuffSize;
			opq->buffs.arr	= NBMemory_allocTypes(STNBDataPtr, ptrsBuffSize);
			NBASSERT(opq->buffs.use == 0)
			NBRtpQueue_fillBufferLockedOpq_(opq->buffs.arr, &opq->buffs.use, opq->buffs.size, &opq->buffs.provider, bytesPerPacket);
			NBASSERT(opq->buffs.use == opq->buffs.size)
		}
	}
	//Queue
	if(opq->queue.bytesPerPacket != bytesPerPacket || opq->queue.size != queueSize){
		//release
		{
			if(opq->queue.itms != NULL){
				UI32 i; for(i = 0; i < opq->queue.size; i++){
					STNBRtpQueueSlot* slot = &opq->queue.itms[i];
					NBRtpQueueSlot_release(slot);
				}
				NBMemory_free(opq->queue.itms);
				opq->queue.itms = NULL;
			}
			opq->queue.size = 0;
		}
		//create
		{
			UI32 ptrsSize		= queueSize;
			UI32 ptrsUse		= 0;
			STNBDataPtr* ptrs	= NBMemory_allocTypes(STNBDataPtr, queueSize);
			NBRtpQueue_fillBufferLockedOpq_(ptrs, &ptrsUse, ptrsSize, &opq->buffs.provider, bytesPerPacket);
			{
				//Init queue
				opq->queue.itms		= NBMemory_allocTypes(STNBRtpQueueSlot, queueSize);
				opq->queue.size		= queueSize;
				opq->queue.use		= 0;
				opq->queue.iFirst	= 0;
				{
					UI32 i; for(i = 0; i < opq->queue.size; i++){
						STNBRtpQueueSlot* slot = &opq->queue.itms[i];
						NBRtpQueueSlot_init(slot);
						NBRtpQueueSlot_swapBuffPtr(slot, &ptrs[i]);
					}
				}
			}
			//release
			if(ptrs != NULL){
				if(ptrsUse > 0){
					NBDataPtr_ptrsReleaseGrouped(ptrs, ptrsUse);
				}
				NBMemory_free(ptrs);
				ptrsUse = ptrsSize = 0;
			}
		}
	}
	opq->bytesPerPacket = bytesPerPacket;
	NB_RTP_QUEUE_SAFE_ACTION_END(opq)
}

//when enabled, internal locks are disabled for efficiency but non-parallels calls must be ensured by user
void NBRtpQueue_setUnsafeMode(STNBRtpQueue* obj, const BOOL unsafeEnabled){
	STNBRtpQueueOpq* opq = (STNBRtpQueueOpq*)obj->opaque;
	NB_RTP_QUEUE_AUTOSAFE_ACTION_START(opq)
	{	
		opq->unsafe.isEnabled = unsafeEnabled;
	}
	NB_RTP_QUEUE_AUTOSAFE_ACTION_END(opq)
}

//Packet

 BOOL NBRtpQueue_addPacket(STNBRtpQueue* obj, STNBRtpHdrBasic* head, STNBDataPtr* chunk, STNBArray* dstRtpHdrs /*STNBRtpHdrBasic*/, STNBArray* dstChunks /*STNBDataPtr*/, STNBRtpStatsCsmUpd* dstStats){
	BOOL r = FALSE;
	STNBRtpQueueOpq* opq = (STNBRtpQueueOpq*)obj->opaque;
	NB_RTP_QUEUE_AUTOSAFE_ACTION_START(opq)
	{
		//Process packet
		NBASSERT(head != NULL && chunk != NULL && opq->queue.size > 0 && dstRtpHdrs != NULL && dstChunks != NULL)
		if(head != NULL && chunk != NULL && opq->queue.size > 0 && dstRtpHdrs != NULL && dstChunks != NULL){
			NBASSERT(opq->queue.itms != NULL)
			NBASSERT(chunk->ptr != NULL)
			SI32 seqCicleBack = opq->rcvd.highest.seqCicle, seqCicleAhead = opq->rcvd.highest.seqCicle, gapBackSz = 0, gapAheadSz = 0;
			//Calculate gaps against current highest seqNum
			if(!opq->rcvd.isFirstKnown){
				//First packet
				opq->rcvd.lowest.timestamp		= head->head.timestamp;
				opq->rcvd.highest.timestamp		= head->head.timestamp;
				if(head->head.seqNum > 0){
					opq->rcvd.lowest.seqCicle	= 0;
					opq->rcvd.lowest.seqNum		= head->head.seqNum - 1;
					opq->rcvd.highest.seqCicle	= 0;
					opq->rcvd.highest.seqNum	= head->head.seqNum - 1;
					seqCicleBack				= 0;
					seqCicleAhead				= 0;
				} else {
					opq->rcvd.lowest.seqCicle	= 0;
					opq->rcvd.lowest.seqNum		= 0xFFFF;
					opq->rcvd.highest.seqCicle	= 0;
					opq->rcvd.highest.seqNum	= 0xFFFF;
					seqCicleBack				= 0;
					seqCicleAhead				= 1;
				}
				opq->rcvd.isFirstKnown			= TRUE;
				NBASSERT(opq->queue.use == 0)
				gapBackSz	= 0xFFFF - 1; 
				gapAheadSz	= 0;
				seqCicleBack--;
				NBASSERT(gapBackSz < 0xFFFF)
				NBASSERT(gapAheadSz < 0xFFFF)
				NBASSERT((gapBackSz + gapAheadSz + 1) == 0xFFFF)
			} else if(head->head.seqNum > opq->rcvd.highest.seqNum){
				//Sequence goes forward (normal)
				gapBackSz	= (0xFFFF - head->head.seqNum) + opq->rcvd.highest.seqNum; 
				gapAheadSz	= (head->head.seqNum - opq->rcvd.highest.seqNum) - 1;
				seqCicleBack--;
				NBASSERT(gapBackSz < 0xFFFF)
				NBASSERT(gapAheadSz < 0xFFFF)
				NBASSERT((gapBackSz + gapAheadSz + 1) == 0xFFFF)
			} else if(head->head.seqNum < opq->rcvd.highest.seqNum){
				//Sequence goes backwards (new cicle or delayed packet)
				NBASSERT(head->head.seqNum < opq->rcvd.highest.seqNum)
				gapBackSz	= (opq->rcvd.highest.seqNum - head->head.seqNum) - 1;
				gapAheadSz	= (0xFFFF - opq->rcvd.highest.seqNum) + head->head.seqNum;
				seqCicleAhead++;
				NBASSERT(gapBackSz < 0xFFFF)
				NBASSERT(gapAheadSz < 0xFFFF)
				NBASSERT((gapBackSz + gapAheadSz + 1) == 0xFFFF)
			} else {
				//Repeated highest packet (just ignore)
				if(dstStats != NULL){
					NBASSERT(chunk->use > 0) //chunk should not be swapped yet
					dstStats->repeated.count++;
					dstStats->repeated.bytesCount += chunk->use;
				}
				r = TRUE;
			}
			//Process packet
			if(!r){
				NBASSERT((gapBackSz + gapAheadSz + 1) == 0xFFFF)
				//NBASSERT(gapBackSz != gapAheadSz) //ToDo: remove, same size gaps is valid on rare ocassions.
				NBASSERT(head->head.seqNum != opq->rcvd.highest.seqNum)
				if(gapBackSz < gapAheadSz){
					NBASSERT(!r)
					//Try to update a slot as delayed packet
					if((gapBackSz + 1) < opq->queue.use){
						UI32 i = opq->queue.iFirst;
						const UI32 iAfterEnd = (opq->queue.iFirst + opq->queue.use) % opq->queue.size;
						do {
							STNBRtpQueueSlot* itm = &opq->queue.itms[i];
							NBASSERT(itm->dbgIsSlotOcupied)
							if(itm->seqNum == head->head.seqNum){
								if(itm->isPopulated){
									NBASSERT(itm->chunk.use > 0)
									//Already received data for slot
									if(dstStats != NULL){
										NBASSERT(chunk->use > 0) //chunk should not be swapped yet
										dstStats->repeated.count++;
										dstStats->repeated.bytesCount += chunk->use;
									}
								} else {
									//Add data to slot
									NBASSERT(itm->chunk.use == 0)
									NBASSERT(chunk->use > 0) //chunk should not be swapped yet
									//PRINTF_INFO("NBRtpsQueue, RTP packet delayed arrived seqNum(%d).\n", itm->head.head.def.seqNum);
									//Consume
									if(dstStats != NULL){
										dstStats->queued.count++;
										dstStats->queued.bytesCount += chunk->use;
										//
										dstStats->delayed.count++;
										dstStats->delayed.bytesCount += chunk->use;
									}
									NBRtpQueueSlot_updateWithData(itm, itm->seqCicle, head, chunk);
								}
								r = TRUE;
								break;
							}
							//Next
							i = (i + 1) % opq->queue.size;
						} while(i != iAfterEnd);
						//Should be found
						NBASSERT(r)
					}
					//Add as ignLate
					if(!r){
						NBASSERT(chunk->use > 0) //chunk should not be swapped yet
						if(dstStats != NULL){
							dstStats->ignLate.count++;
							dstStats->ignLate.bytesCount += chunk->use;
							if(dstStats->ignLate.gapMax < gapBackSz) dstStats->ignLate.gapMax = (UI16)gapBackSz;
						}
						r = TRUE;
					}
					NBASSERT(r)
				} else {
					//Add new packet ahead
					SI32 seqCicleGap	= opq->rcvd.highest.seqCicle;
					UI16 seqNumGap		= opq->rcvd.highest.seqNum;
					SI32 gapRemain		= gapAheadSz;
					BOOL quickJumped	= FALSE;
					NBASSERT(!r)
					while(gapRemain >= 0){
						//Free oldest slot (if necesary)
						if(opq->queue.use == opq->queue.size){
							STNBRtpQueueSlot* itm = &opq->queue.itms[opq->queue.iFirst];
							NBASSERT(itm->dbgIsSlotOcupied)
							if(!itm->isPopulated){
								//stats
								if(dstStats != NULL){
									//slot was reserved but data never arrived
									dstStats->lost.count++;
									dstStats->lost.bytesCount += itm->chunk.use;
								}
							} else {
								//stats
								if(dstStats != NULL){
									dstStats->unqueued.count++;
									dstStats->unqueued.bytesCount += itm->chunk.use;
								}
								if(dstRtpHdrs != NULL && dstChunks != NULL){
									//Add gap-record (if necesary)
									if(opq->notified.isFirstKnown && itm->seqNum != ((opq->notified.lastSeqNum + 1) % 0xFFFF)){
										STNBDataPtr gapPtr;
										STNBRtpHdrBasic gapHdr;
										NBDataPtr_init(&gapPtr);
										NBRtpHdrBasic_init(&gapHdr);
										NBArray_addValue(dstChunks, gapPtr);
										NBArray_addValue(dstRtpHdrs, gapHdr);
									}
									//copy and resign to data (caller must release it)
									if(opq->buffs.use == 0){
										NBRtpQueue_fillBufferLockedOpq_(opq->buffs.arr, &opq->buffs.use, opq->buffs.size, &opq->buffs.provider, opq->bytesPerPacket);
										NBASSERT(opq->buffs.use == opq->buffs.size)
									}
									NBASSERT(opq->buffs.use > 0)
									if(opq->buffs.use > 0){
										NBASSERT(itm->chunk.use > 0)
										NBDataPtr_preResign(&itm->chunk);
										NBRtpHdrBasic_preResign(&itm->head);
										NBArray_addValue(dstChunks, itm->chunk);
										NBArray_addValue(dstRtpHdrs, itm->head);
										itm->chunk					= opq->buffs.arr[--opq->buffs.use];
										NBASSERT(itm->chunk.use == 0)
										NBASSERT(NBDataPtr_allocatedSize(&itm->chunk) >= opq->bytesPerPacket)
										opq->notified.isFirstKnown	= TRUE;
										opq->notified.lastSeqNum	= (UI32)itm->seqNum;
									}
								} else {
									NBASSERT(itm->chunk.ptr != NULL)
									if(dstStats != NULL){
										dstStats->overQueue.count++;
										dstStats->overQueue.bytesCount += itm->chunk.use;
									}
								}
							}
							NBRtpQueueSlot_unset(itm);
							//PRINTF_INFO("NBRtpsQueue, RTP packet released seqCicle(%d) seqNum(%d).\n", itm->seqCicle, itm->seqNum);
							opq->queue.iFirst = (opq->queue.iFirst + 1) % opq->queue.size;
							opq->queue.use--;
						}
						//increase new slot seq
						if(seqNumGap == 0xFFFF){
							seqCicleGap++;
							seqNumGap = 0;
						} else {
							seqNumGap++;
						}
						//Reserve gap slot
						NBASSERT(opq->queue.use < opq->queue.size)
						if(gapRemain > 0){
							if(gapRemain <= opq->queue.size){
								//Reserve new slot
								STNBRtpQueueSlot* itm = &opq->queue.itms[(opq->queue.iFirst + opq->queue.use) % opq->queue.size];
								NBASSERT(!itm->dbgIsSlotOcupied)
								NBRtpQueueSlot_setNoData(itm, seqCicleGap, seqNumGap);
								opq->queue.use++;
								NBASSERT(opq->queue.use <= opq->queue.size)
								//
								if(gapRemain == 0){
									//PRINTF_INFO("NBRtpsQueue, RTP next packet added seqCicle(%d) seqNum(%d).\n", seqCicleGap, seqNumGap);
								} else {
									//PRINTF_INFO("NBRtpsQueue, RTP gap-slot added seqCicle(%d) seqNum(%d).\n", seqCicleGap, seqNumGap);
								}
							} else if(opq->queue.use == 0){
								//queue is empty, quick-jump can be done
								NBASSERT(gapRemain > opq->queue.size)
								const UI32 jumpSz = (gapRemain - opq->queue.size);
								const UI32 seqNumDst = ((SI32)seqNumGap) + jumpSz;
								//PRINTF_INFO("NBRtpQueue, quick-jumped %d lost-slots.\n", jumpSz);
								seqCicleGap	+= (seqNumDst / 0x10000U); //(0x10000U = 0xFFFFU + 0x1U)
								seqNumGap	= (seqNumDst % 0x10000U); //(0x10000U = 0xFFFFU + 0x1U)
								gapRemain	-= jumpSz;
								quickJumped	= TRUE;
								//stats
								if(dstStats != NULL){
									//slot was reserved but data never arrived
									dstStats->lost.count += jumpSz;
									//dstStats->lost.bytesCount += 0;
								}
							}
						}
						//Next
						gapRemain--;
					}
					NBASSERT(opq->queue.use < opq->queue.size) //empty slot must remain
					NBASSERT(seqCicleGap == seqCicleAhead)
#					ifdef NB_CONFIG_INCLUDE_ASSERTS
					if(seqNumGap != head->head.seqNum){
						PRINTF_ERROR("NBRtpsQueue, quickJump failed for gapAheadSz(%d), result(%d) expected(%d).\n", gapAheadSz, seqNumGap, head->head.seqNum);
					}
#					endif
					NBASSERT(seqNumGap == head->head.seqNum)
					NBASSERT(!quickJumped || (opq->queue.use + 1) == opq->queue.size) //if quickjumped the queue should be full after this addition
					//Process
					{
						if(dstRtpHdrs != NULL && dstChunks != NULL && opq->queue.use == 0){
							//Add directly to user array
							//Add gap-record (if necesary)
							if(opq->notified.isFirstKnown && head->head.seqNum != ((opq->notified.lastSeqNum + 1) % 0xFFFF)){
								STNBDataPtr gapPtr;
								STNBRtpHdrBasic gapHdr;
								NBDataPtr_init(&gapPtr);
								NBRtpHdrBasic_init(&gapHdr);
								NBArray_addValue(dstChunks, gapPtr);
								NBArray_addValue(dstRtpHdrs, gapHdr);
							}
							//stats
							if(dstStats != NULL){
								NBASSERT(chunk->use > 0) //chunk should not be swapped yet
								dstStats->passedThrough.count++;
								dstStats->passedThrough.bytesCount += chunk->use;
							}
							//add record (reusing the internal buffer)
							{
								STNBRtpQueueSlot* itm = &opq->queue.itms[(opq->queue.iFirst + opq->queue.use) % opq->queue.size];
								NBASSERT(!itm->dbgIsSlotOcupied)
								NBASSERT(chunk->ptr != NULL && chunk->use > 0) //chunk should not be swapped yet
								NBRtpQueueSlot_setWithDataUnocupied(itm, seqCicleAhead, head, chunk);
								//copy and resign to data (caller must release it)
								if(opq->buffs.use == 0){
									NBRtpQueue_fillBufferLockedOpq_(opq->buffs.arr, &opq->buffs.use, opq->buffs.size, &opq->buffs.provider, opq->bytesPerPacket);
									NBASSERT(opq->buffs.use == opq->buffs.size)
								}
								NBASSERT(opq->buffs.use > 0)
								if(opq->buffs.use > 0){
									NBASSERT(itm->chunk.use > 0)
									NBDataPtr_preResign(&itm->chunk);
									NBRtpHdrBasic_preResign(&itm->head);
									NBArray_addValue(dstChunks, itm->chunk);
									NBArray_addValue(dstRtpHdrs, itm->head);
									itm->chunk					= opq->buffs.arr[--opq->buffs.use];
									NBASSERT(itm->chunk.use == 0)
									NBASSERT(NBDataPtr_allocatedSize(&itm->chunk) >= opq->bytesPerPacket)
									opq->notified.isFirstKnown	= TRUE;
									opq->notified.lastSeqNum	= (UI32)head->head.seqNum;
								}
							}
						} else {
							//Add to queue
							STNBRtpQueueSlot* itm = &opq->queue.itms[(opq->queue.iFirst + opq->queue.use) % opq->queue.size];
							NBASSERT(!itm->dbgIsSlotOcupied)
							//stats
							if(dstStats != NULL){
								NBASSERT(chunk->use > 0) //chunk should not be swapped yet
								dstStats->queued.count++;
								dstStats->queued.bytesCount += chunk->use;
							}
							NBRtpQueueSlot_setWithData(itm, seqCicleAhead, head, chunk);
							opq->queue.use++;
						}
						//highest
						opq->rcvd.highest.seqCicle	= seqCicleAhead;
						opq->rcvd.highest.seqNum	= head->head.seqNum;
						opq->rcvd.highest.timestamp	= head->head.timestamp;
					}
					r = TRUE;
				}
			}
			//Validate queue
#			ifdef NB_CONFIG_INCLUDE_ASSERTS
			/*if(opq->queue.itms != NULL && opq->queue.size > 0 && opq->queue.use > 0){
				const STNBRtpQueueSlot* prev = NULL;
				UI32 i = opq->queue.iFirst;
				const UI32 iAfterEnd = (opq->queue.iFirst + opq->queue.use) % opq->queue.size;
				do {
					const STNBRtpQueueSlot* itm = &opq->queue.itms[i];
					NBASSERT(itm->dbgIsSlotOcupied)
					if(prev != NULL){
						const BOOL isForward = (prev->pckt.seqCicle == itm->seqCicle && (prev->pckt.seqNum + 1) == itm->seqNum);
						const BOOL isBackward = ((prev->pckt.seqCicle + 1) == itm->seqCicle && prev->pckt.seqNum > itm->seqNum); 
						NBASSERT(isForward || isBackward)
					}
					//Next
					prev = itm;
					i = (i + 1) % opq->queue.size;
				} while(i != iAfterEnd);
				//Validate seqNum
				NBASSERT(prev->pckt.seqCicle == opq->rcvd.highest.seqCicle)
				NBASSERT(prev->pckt.seqNum == opq->rcvd.highest.seqNum)
				//Validate empty slots
				while(i != opq->queue.iFirst){
					const STNBRtpQueueSlot* itm = &opq->queue.itms[i];
					NBASSERT(!itm->dbgIsSlotOcupied)
					i = (i + 1) % opq->queue.size;
				}
			}*/
#			endif
		}
		//Add sequence of filled packets
		if(dstRtpHdrs != NULL && dstChunks != NULL){
			while(opq->queue.use > 0){
				NBASSERT(opq->queue.itms != NULL && opq->queue.size > 0)
				STNBRtpQueueSlot* itm = &opq->queue.itms[opq->queue.iFirst % opq->queue.size];
				NBASSERT(itm->dbgIsSlotOcupied)
				if(!itm->isPopulated){
					//still waiting for its data
					break;
				} else {
					//result
					if(dstStats != NULL){
						dstStats->unqueued.count++;
						dstStats->unqueued.bytesCount += itm->chunk.use;
					}
					//Add gap-record (if necesary)
					if(opq->notified.isFirstKnown && itm->seqNum != ((opq->notified.lastSeqNum + 1) % 0xFFFF)){
						STNBDataPtr gapPtr;
						STNBRtpHdrBasic gapHdr;
						NBDataPtr_init(&gapPtr);
						NBRtpHdrBasic_init(&gapHdr);
						NBArray_addValue(dstChunks, gapPtr);
						NBArray_addValue(dstRtpHdrs, gapHdr);
					}
					//copy and resign to data (caller must release it)
					if(opq->buffs.use == 0){
						NBRtpQueue_fillBufferLockedOpq_(opq->buffs.arr, &opq->buffs.use, opq->buffs.size, &opq->buffs.provider, opq->bytesPerPacket);
						NBASSERT(opq->buffs.use == opq->buffs.size)
					}
					NBASSERT(opq->buffs.use > 0)
					if(opq->buffs.use > 0){
						NBASSERT(itm->chunk.use > 0)
						NBDataPtr_preResign(&itm->chunk);
						NBRtpHdrBasic_preResign(&itm->head);
						NBArray_addValue(dstChunks, itm->chunk);
						NBArray_addValue(dstRtpHdrs, itm->head);
						itm->chunk					= opq->buffs.arr[--opq->buffs.use];
						NBASSERT(itm->chunk.use == 0)
						NBASSERT(NBDataPtr_allocatedSize(&itm->chunk) >= opq->bytesPerPacket)
						opq->notified.isFirstKnown	= TRUE;
						opq->notified.lastSeqNum	= (UI32)itm->seqNum;
					}
					//release slot
					opq->queue.iFirst = (opq->queue.iFirst + 1) % opq->queue.size;
					opq->queue.use--;
					NBRtpQueueSlot_unset(itm);
				}
			}
		}
	}
	NB_RTP_QUEUE_AUTOSAFE_ACTION_END(opq)
	return r;
}

void NBRtpQueue_reset(STNBRtpQueue* obj){
	STNBRtpQueueOpq* opq = (STNBRtpQueueOpq*)obj->opaque;
	NB_RTP_QUEUE_AUTOSAFE_ACTION_START(opq)
	{
		//Release packets
		while(opq->queue.use > 0){
			NBASSERT(opq->queue.itms != NULL && opq->queue.size > 0)
			STNBRtpQueueSlot* itm = &opq->queue.itms[opq->queue.iFirst % opq->queue.size];
			NBASSERT(itm->dbgIsSlotOcupied)
			NBRtpQueueSlot_unset(itm);
			//release slot
			opq->queue.iFirst = (opq->queue.iFirst + 1) % opq->queue.size;
			opq->queue.use--;
		}
		//Reset
		NBMemory_setZero(opq->rcvd);
	}
	NB_RTP_QUEUE_AUTOSAFE_ACTION_END(opq)
}
