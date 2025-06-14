
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBRtpClient.h"
//
#include <stdlib.h>	//for "rand()" 
//
#include "nb/core/NBArray.h"
#include "nb/core/NBThread.h"
#include "nb/core/NBThreadMutexRW.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBThreadCond.h"
#include "nb/core/NBArraySorted.h"
#include "nb/core/NBMngrStructMaps.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBIOPollster.h"

//NBRtpCfgPort

STNBStructMapsRec STNBRtpCfgPort_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtpCfgPort_getSharedStructMap(void){
	NBMngrStructMaps_lock(&STNBRtpCfgPort_sharedStructMap);
	if(STNBRtpCfgPort_sharedStructMap.map == NULL){
		STNBRtpCfgPort s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtpCfgPort);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addUIntM(map, s, packetsBuffSz);	//amount of packets in socket read buffer
		NBStructMap_addStrPtrM(map, s, allocBuffSz);	//string representing a number-bytes and oprional-sufix, like "1", "1K", "1kb", "1M", "1G", "1Tb".
		NBStructMap_addUIntM(map, s, allocBuffSzBytes);	//amount of packets in allocation buffer
		STNBRtpCfgPort_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&STNBRtpCfgPort_sharedStructMap);
	return STNBRtpCfgPort_sharedStructMap.map;
}

//NBRtpCfgStream

STNBStructMapsRec STNBRtpCfgStream_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtpCfgStream_getSharedStructMap(void){
	NBMngrStructMaps_lock(&STNBRtpCfgStream_sharedStructMap);
	if(STNBRtpCfgStream_sharedStructMap.map == NULL){
		STNBRtpCfgStream s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtpCfgStream);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addUIntM(map, s, packetsBuffSz);		//amount of packets in buffer
		NBStructMap_addUIntM(map, s, orderingQueueSz);		//packets ordering queue size (this will determine wich packets are lost or delayed)
		STNBRtpCfgStream_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&STNBRtpCfgStream_sharedStructMap);
	return STNBRtpCfgStream_sharedStructMap.map;
}

//CfgRtpPackets

STNBStructMapsRec STNBRtpCfgPackets_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtpCfgPackets_getSharedStructMap(void){
	NBMngrStructMaps_lock(&STNBRtpCfgPackets_sharedStructMap);
	if(STNBRtpCfgPackets_sharedStructMap.map == NULL){
		STNBRtpCfgPackets s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtpCfgPackets);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addStrPtrM(map, s, maxSize);		//max size of each packet, string representing a number-bytes and oprional-sufix, like "1", "1K", "1kb", "1M", "1G", "1Tb".
		NBStructMap_addUIntM(map, s, maxSizeBytes);		//max size of each packet
		NBStructMap_addStrPtrM(map, s, initAlloc);		//initial allocation of packets buffer, string representing a number-bytes and oprional-sufix, like "1", "1K", "1kb", "1M", "1G", "1Tb".
		NBStructMap_addUIntM(map, s, initAllocBytes);	//initial allocation of packets buffer
		STNBRtpCfgPackets_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&STNBRtpCfgPackets_sharedStructMap);
	return STNBRtpCfgPackets_sharedStructMap.map;
}

//CfgRtpDebug

STNBStructMapsRec STNBRtpCfgDebug_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtpCfgDebug_getSharedStructMap(void){
	NBMngrStructMaps_lock(&STNBRtpCfgDebug_sharedStructMap);
	if(STNBRtpCfgDebug_sharedStructMap.map == NULL){
		STNBRtpCfgDebug s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtpCfgDebug);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addUIntM(map, s, packetLostSimDiv); //Zero for no-packet-lost simulation; lost = ((rand() % packetLostSimDiv) == 0);
		NBStructMap_addBoolM(map, s, doNotNotify);		//Usefull for testing packet lost at network recv without payload: rtp packets are parsed, listener identified, packet added to its queue, but packet is not send ahead.
		STNBRtpCfgDebug_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&STNBRtpCfgDebug_sharedStructMap);
	return STNBRtpCfgDebug_sharedStructMap.map;
}

//CfgRtp

STNBStructMapsRec STNBRtpCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtpCfg_getSharedStructMap(void){
	NBMngrStructMaps_lock(&STNBRtpCfg_sharedStructMap);
	if(STNBRtpCfg_sharedStructMap.map == NULL){
		STNBRtpCfg s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtpCfg);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addPtrToArrayOfUIntM(map, s, ports, portsSz, ENNBStructMapSign_Unsigned); //incoming ports
		NBStructMap_addBoolM(map, s, atomicStats);										//apply stats to provider after each action (do not wait for explicit statsFlush)
		NBStructMap_addStructM(map, s, packets, NBRtpCfgPackets_getSharedStructMap());	//packets
		NBStructMap_addStructM(map, s, perPort, NBRtpCfgPort_getSharedStructMap()); 	//perPort config
		NBStructMap_addStructM(map, s, perStream, NBRtpCfgStream_getSharedStructMap()); //perStream config
		NBStructMap_addStructM(map, s, debug, NBRtpCfgDebug_getSharedStructMap());		//debug			
		STNBRtpCfg_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&STNBRtpCfg_sharedStructMap);
	return STNBRtpCfg_sharedStructMap.map;
}

//

struct STNBRtpClientOpq_;
NB_OBJREF_HEADER(NBRtpClientPort)
NB_OBJREF_HEADER(NBRtpClientStream)

//-----------------------
//- NBRtpClientPacketFnd (packet)
//-----------------------

typedef struct STNBRtpClientPacketFnd_ {
	STNBSocketAddr*			from;		//packet origin
	STNBRtpHdrBasic*		hdr;		//packet header
	STNBDataPtr*			chunk;		//packet chunk
	STNBRtpClientStreamRef	stream;		//stream itm
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	BOOL					streamSet;	//listener already evaluated
	BOOL					grouped;	//gouped
	BOOL					notified;	//notified
#	endif
} STNBRtpClientPacketFnd;

//-----------------------
//- NBRtpClientPort (receiver port)
//-----------------------

typedef struct STNBRtpClientPortOpq_ {
	STNBObject				prnt;
	struct STNBRtpClientOpq_* opq;		//parent
	UI32					port;
	STNBSocketRef			socket;
	//streams
	struct {
		STNBArraySorted		arr;		//STNBRtpClientStreamRef, (port streams)
		//pend
		struct {
			UI32			iSeqReq;	//pend sequence requested
			UI32			iSeqDone;	//pend sequence done
			STNBArray		add;		//STNBRtpClientStreamRef, (port streams)
			STNBArray		rmv;		//STNBRtpClientStreamRef, (port streams)
		} pend;
	} streams;
	//pollster
	struct {
		STNBIOPollsterRef	ref;
		BOOL				isListening;	//socket added to pollster
        STNBStopFlagRef		stopFlag;
	} pollster;
	//buffer
	struct {
		STNBSocketPackets	pckts;
		STNBRtpHdrBasic*	hdrs;
		STNBDataPtrsPoolRef	pool;
		UI32				bytesPerPacket;
		//alloc
		struct {
			STNBDataPtr*	ptrs;
			UI32			emptied;	//consumed left border
			UI32			filled;		//allocated left border
			UI32			size;		//right border
		} alloc;
		//tmp
		struct {
			//fndPtrs
			struct {
				STNBRtpClientPacketFnd*	arr;
				UI32					sz;
			} fndPtrs;
			//grpdPtrs
			struct {
				STNBRtpClientPacketFnd*	arr;
				UI32					sz;
			} grpdPtrs;
		} tmp;
	} buffs;
	//stats
	struct {
		STNBRtpClientStats*		provider;	//for port-specific stats calculation
		STNBRtpClientStatsUpd	upd;		//accumlated stats
		//flush
		struct {
			UI32		iSeqReq;		//flush sequence requested
			UI32		iSeqDone;		//flush sequence done
		} flush;
	} stats;
} STNBRtpClientPortOpq;

SI32 NBRtpClientPort_rcvMany_(STNBRtpClientPortOpq* p, STNBSocketRef socket, const UI32 packetLostSimDiv);
UI32 NBRtpClientPort_getPtrs_(const UI32 minBytesPerPtr, STNBDataPtr* srcAndDst, const UI32 srcAndDstSz, void* usrData);

//---------------------
//- NBRtpClientStream
//---------------------

typedef struct STNBRtpClientStreamOpq_ {
	STNBObject			prnt;
	STNBRtpClientLstnr	lstnr;	//includes the 'ssrc' (unique-id)
	//port (reverse reference)
	struct {
		UI32			num;
		STNBRtpClientPortRef port;
	} port;
	//buffer
	struct {
		STNBRtpHdrBasic* hdrs;
		STNBDataPtr*	chunks;
		UI32			iNxtRead;	//Next to read (filled)
		UI32			iNxtWrite;	//Next to write (empty)
		UI32			sz;
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		//dbg (packets per notificactions)
		struct {
			UI64		timeLst;
			UI64		packtsCount;
			UI64		notifsCount;
		} dbg;
#		endif
	} buffs;
	//queue
	STNBRtpQueue		queue;		//packets queue
	//notify tmp arrays
	struct {
		STNBArray		hdrs;		//STNBRtpHdrBasic, packets to notify
		STNBArray		chunks;		//STNBDataPtr, packets to notify
	} notify;
	//stats (to be executed when the thread is not bussy)
	struct {
		STNBRtpClientStats*		provider;	//for stream-specific stats calculation
		STNBRtpClientStatsUpd	upd;		//accumlated stats
		//flush
		struct {
			UI32		iSeqReq;		//flush sequence requested
			UI32		iSeqDone;		//flush sequence done
		} flush;
	} stats;
} STNBRtpClientStreamOpq;

//-------------
//- NBRtpClient
//-------------
	
typedef struct STNBRtpClientOpq_ {
	STNBRtpCfg				cfg;
	STNBThreadMutex			mutex;
    STNBStopFlagRef			stopFlag;
	//buffers
	struct {
		STNBDataPtrsPoolRef	pool;
	} buffs;
	//pollster
	struct {
		STNBIOPollsterRef			def; //default
		STNBIOPollsterSyncRef		sync; //default
		STNBIOPollstersProviderRef	provider;
	} pollster;
	//ports
	struct {
		STNBArray			arr;			//STNBRtpClientPort
		BOOL 				areListening;
	} ports;
	//streams (global list)
	struct {
		STNBArraySorted		arr;			//STNBRtpClientStreamRef
	} streams;
	//Stats
	struct {
		STNBRtpClientStats* provider;
		//flush
		struct {
			UI32		iSeqReq;		//flush sequence requested
			UI32		iSeqDone;		//flush sequence done
		} flush;
	} stats;
} STNBRtpClientOpq;

//

#define NB_RTP_CLIENT_PORT_HAS_PEND_TASK_STREAMS_SYNC(P_PTR)	((P_PTR)->streams.pend.iSeqDone != (P_PTR)->streams.pend.iSeqReq)
#define NB_RTP_CLIENT_PORT_HAS_PEND_TASK_FLUSH_STATS(P_PTR)		((P_PTR)->stats.flush.iSeqDone != (P_PTR)->stats.flush.iSeqReq)
#define NB_RTP_CLIENT_PORT_HAS_PEND_TASK(P_PTR)					(NB_RTP_CLIENT_PORT_HAS_PEND_TASK_STREAMS_SYNC(P_PTR) || NB_RTP_CLIENT_PORT_HAS_PEND_TASK_FLUSH_STATS(P_PTR))

void NBRtpClient_portConsumePendTaskStreamsSyncLockedOpq_(STNBRtpClientOpq* opq, STNBRtpClientPortOpq* p);
void NBRtpClient_portConsumePendTaskStatsFlushLockedOpq_(STNBRtpClientOpq* opq, STNBRtpClientPortOpq* p);
void NBRtpClient_portConsumePendTasksLockedOpq_(STNBRtpClientOpq* opq, STNBRtpClientPortOpq* p);
void NBRtpClient_portConsumePendTasksOpq_(STNBRtpClientOpq* opq, STNBRtpClientPortOpq* p);

//

BOOL NBCompare_NBRtpClientStreamPtrBySsrc(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	const STNBRtpClientStreamOpq* d1 = (const STNBRtpClientStreamOpq*)((STNBRtpClientStreamRef*)data1)->opaque;
	const STNBRtpClientStreamOpq* d2 = (const STNBRtpClientStreamOpq*)((STNBRtpClientStreamRef*)data2)->opaque;
	NBASSERT(dataSz == sizeof(STNBRtpClientStreamRef*))
	if(dataSz == sizeof(STNBRtpClientStreamRef*)){
		switch (mode) {
		case ENCompareMode_Equal:
			return d1->lstnr.ssrc == d2->lstnr.ssrc;
		case ENCompareMode_Lower:
			return d1->lstnr.ssrc < d2->lstnr.ssrc;
		case ENCompareMode_LowerOrEqual:
			return d1->lstnr.ssrc <= d2->lstnr.ssrc;
		case ENCompareMode_Greater:
			return d1->lstnr.ssrc > d2->lstnr.ssrc;
		case ENCompareMode_GreaterOrEqual:
			return d1->lstnr.ssrc >= d2->lstnr.ssrc;
		default:
			NBASSERT(FALSE)
			break;
		}
	}
	NBASSERT(FALSE)
	return FALSE;
}

//NBRtpCmdState

void NBRtpCmdState_init(STNBRtpCmdState* obj){
	NBMemory_setZeroSt(*obj, STNBRtpCmdState);
}

void NBRtpCmdState_release(STNBRtpCmdState* obj){
	//nothing
}

//

void NBRtpClient_init(STNBRtpClient* obj){
	STNBRtpClientOpq* opq = obj->opaque = NBMemory_allocType(STNBRtpClientOpq);
	NBMemory_setZeroSt(*opq, STNBRtpClientOpq);
	NBThreadMutex_init(&opq->mutex);
    opq->stopFlag = NBStopFlag_alloc(NULL);
	//ports
	{
		NBArray_init(&opq->ports.arr, sizeof(STNBRtpClientPortRef), NULL);
	}
	//streams
	{
		NBArraySorted_init(&opq->streams.arr, sizeof(STNBRtpClientStreamRef), NBCompare_NBRtpClientStreamPtrBySsrc);
	}
	
}

void NBRtpClient_release(STNBRtpClient* obj){
	STNBRtpClientOpq* opq = (STNBRtpClientOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
    NBStopFlag_activate(opq->stopFlag);
	{
		//ports
		{
			SI32 i; for(i = 0; i < opq->ports.arr.use; i++){
				STNBRtpClientPortRef pRef = NBArray_itmValueAtIndex(&opq->ports.arr, STNBRtpClientPortRef, i);
				NBASSERT(NBRtpClientPort_isClass(pRef))
				NBRtpClientPort_release(&pRef);
			}
			NBArray_empty(&opq->ports.arr);
			NBArray_release(&opq->ports.arr);
		}
		//streams
		{
			NBASSERT(opq->streams.arr.use == 0)
			SI32 i; for(i = 0; i < opq->streams.arr.use; i++){
				STNBRtpClientStreamRef s = NBArraySorted_itmValueAtIndex(&opq->streams.arr, STNBRtpClientStreamRef, i);
				NBRtpClientStream_release(&s);
				NBRtpClientStream_null(&s);
			}
			NBArraySorted_empty(&opq->streams.arr);
			NBArraySorted_release(&opq->streams.arr);
		}
		//pollster
		{
			if(NBIOPollster_isSet(opq->pollster.def)){
				NBIOPollster_release(&opq->pollster.def);
				NBIOPollster_null(&opq->pollster.def);
			}
			if(NBIOPollsterSync_isSet(opq->pollster.sync)){
				NBIOPollsterSync_release(&opq->pollster.sync);
				NBIOPollsterSync_null(&opq->pollster.sync);
			}
			if(NBIOPollstersProvider_isSet(opq->pollster.provider)){
				NBIOPollstersProvider_release(&opq->pollster.provider);
				NBIOPollstersProvider_null(&opq->pollster.provider);
			}
		}
		//cfg
		{
			NBStruct_stRelease(NBRtpCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
		}
		//buffers
		{
			if(NBDataPtrsPool_isSet(opq->buffs.pool)){
				NBDataPtrsPool_release(&opq->buffs.pool);
				NBDataPtrsPool_null(&opq->buffs.pool);
			}
		}
	}
    if(NBStopFlag_isSet(opq->stopFlag)){
        NBStopFlag_release(&opq->stopFlag);
        NBStopFlag_null(&opq->stopFlag);
    }
	NBThreadMutex_unlock(&opq->mutex);
	NBThreadMutex_release(&opq->mutex);
	//
	NBMemory_free(opq);
	obj->opaque = NULL;
}

//Commands

void NBRtpClient_statsFlushStart(STNBRtpClient* obj, STNBRtpCmdState* dst){
	STNBRtpClientOpq* opq = (STNBRtpClientOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	if(!NBStopFlag_isMineActivated(opq->stopFlag)){
		BOOL pendFnd = FALSE;
		const UI32 iSeq = ++opq->stats.flush.iSeqReq;
		//add flush request to streams
		{
			SI32 i; for(i = 0; i < opq->streams.arr.use; i++){
				STNBRtpClientStreamRef sRef = NBArraySorted_itmValueAtIndex(&opq->streams.arr, STNBRtpClientStreamRef, i);
				STNBRtpClientStreamOpq* s = (STNBRtpClientStreamOpq*)sRef.opaque;
				NBObject_lock(s);
				{
					NBASSERT(s->stats.flush.iSeqReq < iSeq)
					s->stats.flush.iSeqReq = iSeq;
					pendFnd = TRUE;
				}
				NBObject_unlock(s);
			}
		}
		//add flush request to ports
		{
			SI32 i; for(i = 0; i < opq->ports.arr.use; i++){
				STNBRtpClientPortRef pRef = NBArray_itmValueAtIndex(&opq->ports.arr, STNBRtpClientPortRef, i);
				STNBRtpClientPortOpq* p = (STNBRtpClientPortOpq*)pRef.opaque; NBASSERT(NBRtpClientPort_isClass(pRef));
				NBObject_lock(p);
				{
					NBASSERT(p->stats.flush.iSeqReq < iSeq)
					p->stats.flush.iSeqReq = iSeq;
					pendFnd = TRUE;
				}
				NBObject_unlock(p);
			}
		}
		//set results
		if(dst != NULL){
			dst->seq		= iSeq;
			dst->isPend		= pendFnd;
		}
	}
	NBThreadMutex_unlock(&opq->mutex);
}

BOOL NBRtpClient_statsFlushIsPend(STNBRtpClient* obj, STNBRtpCmdState* dst){
	BOOL r = FALSE;
	STNBRtpClientOpq* opq = (STNBRtpClientOpq*)obj->opaque;
	if(dst != NULL){
		NBThreadMutex_lock(&opq->mutex);
		{
			//count request to ports
			if(!r){
				SI32 i; for(i = 0; i < opq->ports.arr.use && !r; i++){
					STNBRtpClientPortRef pRef = NBArray_itmValueAtIndex(&opq->ports.arr, STNBRtpClientPortRef, i);
					STNBRtpClientPortOpq* p = (STNBRtpClientPortOpq*)pRef.opaque;
					NBObject_lock(p);
					{
						//execute here (not running)
						if(!p->pollster.isListening && NB_RTP_CLIENT_PORT_HAS_PEND_TASK_FLUSH_STATS(p)){
							NBRtpClient_portConsumePendTaskStatsFlushLockedOpq_(opq, p);
							NBASSERT(!NB_RTP_CLIENT_PORT_HAS_PEND_TASK_FLUSH_STATS(p))
						}
						if(p->stats.flush.iSeqDone < dst->seq && dst->seq <= p->stats.flush.iSeqReq){
							r = TRUE;
						}
					}
					NBObject_unlock(p);
				}
			}
			//count request to streams
			if(!r){
				SI32 i; for(i = 0; i < opq->streams.arr.use && !r; i++){
					STNBRtpClientStreamRef sRef = NBArraySorted_itmValueAtIndex(&opq->streams.arr, STNBRtpClientStreamRef, i);
					STNBRtpClientStreamOpq* s = (STNBRtpClientStreamOpq*)sRef.opaque;
					NBObject_lock(s);
					if(s->stats.flush.iSeqDone < dst->seq && dst->seq <= s->stats.flush.iSeqReq){
						r = TRUE;
					}
					NBObject_unlock(s);
				}
			}
			dst->isPend = r;
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}

void NBRtpClient_statsGet(STNBRtpClient* obj, STNBArray* dstPortsStats /*STNBRtpClientPortStatsData*/, STNBArray* dstSsrcsStats /*STNBRtpClientSsrcStatsData*/, const BOOL resetAccum){
	STNBRtpClientOpq* opq = (STNBRtpClientOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	{
		//get ports stats
        //ToDo: update existing records instead of allways-adding
		if(dstPortsStats != NULL){
			SI32 i; for(i = 0; i < opq->ports.arr.use; i++){
				STNBRtpClientPortRef pRef = NBArray_itmValueAtIndex(&opq->ports.arr, STNBRtpClientPortRef, i);
				STNBRtpClientPortOpq* p = (STNBRtpClientPortOpq*)pRef.opaque; NBASSERT(NBRtpClientPort_isClass(pRef))
				NBObject_lock(p);
				{
					STNBRtpClientPortStatsData d;
					NBMemory_setZeroSt(d, STNBRtpClientPortStatsData);
					d.port	= p->port;
					d.data	= NBRtpClientStats_getData(p->stats.provider, resetAccum);
					NBArray_addValue(dstPortsStats, d);
				}
				NBObject_unlock(p);
			}
		}
		//get streams stats
        //ToDo: update existing records instead of allways-adding
		if(dstSsrcsStats != NULL){
			SI32 i; for(i = 0; i < opq->streams.arr.use; i++){
				STNBRtpClientStreamRef sRef = NBArraySorted_itmValueAtIndex(&opq->streams.arr, STNBRtpClientStreamRef, i);
				STNBRtpClientStreamOpq* s = (STNBRtpClientStreamOpq*)sRef.opaque;
				NBObject_lock(s);
				{
					STNBRtpClientSsrcStatsData d;
					NBMemory_setZeroSt(d, STNBRtpClientSsrcStatsData);
					d.ssrc	= (UI16)s->lstnr.ssrc;
					d.data	= NBRtpClientStats_getData(s->stats.provider, resetAccum);
					NBArray_addValue(dstSsrcsStats, d);
				}
				NBObject_unlock(s);
			}
		}
	}
	NBThreadMutex_unlock(&opq->mutex);
}

//Listeners

BOOL NBRtpClient_addStreamLstnr(STNBRtpClient* obj, STNBRtpClientLstnr* listener){
	BOOL r = FALSE;
	STNBRtpClientOpq* opq = (STNBRtpClientOpq*)obj->opaque;
	STNBRtpClientPortRef pRef = NB_OBJREF_NULL; STNBRtpClientStreamRef sRef = NB_OBJREF_NULL; UI32 iSeq = 0;
	//analyze and search for port
	{
		NBThreadMutex_lock(&opq->mutex);
		NBASSERT(listener != NULL && listener->port > 0 && listener->ssrc > 0)
		if(listener != NULL && listener->port > 0 && listener->ssrc > 0){
			STNBRtpClientStreamRef srchh;
			STNBRtpClientStreamOpq srchhOpq;
			NBMemory_setZeroSt(srchh, STNBRtpClientStreamRef);
			NBMemory_setZeroSt(srchhOpq, STNBRtpClientStreamOpq);
			srchh.opaque = &srchhOpq;
			srchhOpq.lstnr.ssrc	= listener->ssrc;
			srchhOpq.lstnr.addr	= listener->addr;
			{
				const SI32 iFnd = NBArraySorted_indexOf(&opq->streams.arr, &srchh, sizeof(srchh), NULL);
				if(iFnd >= 0){
#				ifdef NB_CONFIG_INCLUDE_ASSERTS
					{
						char buff[1024], buff2[1024];
						STNBRtpClientStreamRef fndRef = NBArraySorted_itmValueAtIndex(&opq->streams.arr, STNBRtpClientStreamRef, iFnd);
						STNBRtpClientStreamOpq* fnd = (STNBRtpClientStreamOpq*)fndRef.opaque;
						NBSocketAddr_concatAddrOnly(&listener->addr, buff, sizeof(buff));
						NBSocketAddr_concatAddrOnly(&fnd->lstnr.addr, buff2, sizeof(buff2));
						PRINTF_ERROR("NBRtpClient, lstnr ssrc(%u@%s) already registered as sscr(%u@%s).\n", listener->ssrc, buff, fnd->lstnr.ssrc, buff2);
					}
#				endif
				} else {
					//Search port
					{
						SI32 i; for(i = 0; i < opq->ports.arr.use; i++){
							STNBRtpClientPortRef p2Ref = NBArray_itmValueAtIndex(&opq->ports.arr, STNBRtpClientPortRef, i);
							STNBRtpClientPortOpq* p2 = (STNBRtpClientPortOpq*)p2Ref.opaque; NBASSERT(NBRtpClientPort_isClass(p2Ref))
							if(p2->port == listener->port){
								pRef = p2Ref;
								break;
							}
						}
					}
					//Add to port
					if(!NBRtpClientPort_isSet(pRef)){
#						ifdef NB_CONFIG_INCLUDE_ASSERTS
						{
							char buff[1024], buff2[1024];
							STNBRtpClientStreamRef fndRef = NBArraySorted_itmValueAtIndex(&opq->streams.arr, STNBRtpClientStreamRef, iFnd);
							STNBRtpClientStreamOpq* fnd = (STNBRtpClientStreamOpq*)fndRef.opaque;
							NBSocketAddr_concatAddrOnly(&listener->addr, buff, sizeof(buff));
							NBSocketAddr_concatAddrOnly(&fnd->lstnr.addr, buff2, sizeof(buff2));
							PRINTF_ERROR("NBRtpClient, lstnr ssrc(%u@%s), port(%d) not found.\n", listener->ssrc, buff, listener->port);
						}
#						endif
					} else {
						STNBRtpClientPortOpq* p = (STNBRtpClientPortOpq*)pRef.opaque; NBASSERT(NBRtpClientPort_isClass(pRef))
						NBObject_lock(p);
						{
							//search remove action and nullify
							BOOL fnd = FALSE;
							SI32 i; for(i = 0; i < p->streams.pend.rmv.use; i++){
								STNBRtpClientStreamRef s2Ref = NBArray_itmValueAtIndex(&p->streams.pend.rmv, STNBRtpClientStreamRef, i);
								STNBRtpClientStreamOpq* s2 = (STNBRtpClientStreamOpq*)s2Ref.opaque;
								if(s2->lstnr.ssrc == listener->ssrc){
									NBArray_removeItemAtIndex(&p->streams.pend.rmv, i);
									NBRtpClientStream_release(&s2Ref);
									fnd = TRUE;
									//result
									r = TRUE;
								}
							}
							if(!fnd){
								//create stream
								STNBRtpClientStreamOpq* s;
								sRef			= NBRtpClientStream_alloc(NULL);
								s				= (STNBRtpClientStreamOpq*)sRef.opaque;
								s->lstnr		= *listener;
								//port
								s->port.num		= p->port; NBASSERT(s->port.num > 0)
								s->port.port	= pRef;
								//stats
								if(opq->stats.provider != NULL){
									s->stats.provider = NBMemory_allocType(STNBRtpClientStats);
									NBRtpClientStats_init(s->stats.provider);
								}
								//Create buffers
								{
									s->buffs.sz		= opq->cfg.perStream.packetsBuffSz;
									s->buffs.hdrs	= NBMemory_allocTypes(STNBRtpHdrBasic, opq->cfg.perStream.packetsBuffSz);
									s->buffs.chunks	= NBMemory_allocTypes(STNBDataPtr, opq->cfg.perStream.packetsBuffSz);
									//init
									{
										UI32 i; for(i = 0; i < s->buffs.sz; i++){
											STNBRtpHdrBasic* h = &s->buffs.hdrs[i];
											STNBDataPtr* c = &s->buffs.chunks[i];
											NBRtpHdrBasic_init(h);
											NBDataPtr_init(c);
											//individual allocation
											if(!NBObjRef_isSet(opq->buffs.pool)){
												NBDataPtr_allocEmptyPtr(c, opq->cfg.packets.maxSizeBytes);
											}
										}
										//grouped allocation
										if(NBObjRef_isSet(opq->buffs.pool)){
											NBDataPtrsPool_getPtrs(opq->buffs.pool, opq->cfg.packets.maxSizeBytes, s->buffs.chunks, s->buffs.sz);
										}
									}
								}
								//queue
								{
									const UI32 bytesPerPacket	= opq->cfg.packets.maxSizeBytes;
									const UI32 queueSize		= opq->cfg.perStream.orderingQueueSz;
									const UI32 ptrsBuffSize		= (opq->cfg.perStream.packetsBuffSz + opq->cfg.perStream.orderingQueueSz);
									{
										STNBRtpQueueBuffProviderItf itf;
										NBMemory_setZeroSt(itf, STNBRtpQueueBuffProviderItf);
										itf.getPtrs = NBRtpClientPort_getPtrs_;
										NBRtpQueue_setBuffersProvider(&s->queue, &itf, p);
									}
									NBRtpQueue_setSize(&s->queue, bytesPerPacket, queueSize, ptrsBuffSize);
									NBRtpQueue_setUnsafeMode(&s->queue, TRUE);
								}
								//add to array
								NBArraySorted_addValue(&opq->streams.arr, sRef);
								//add to port
								{
									iSeq = ++p->streams.pend.iSeqReq;
									NBRtpClientStream_retain(sRef);
									NBArray_addValue(&p->streams.pend.add, sRef);
								}
								//result
								r = TRUE;
							}
						}
						NBObject_unlock(p);
					}
				}
			}
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}

BOOL NBRtpClient_removeStreamLstnr(STNBRtpClient* obj, const UI32 ssrc, const STNBSocketAddr addr){
	BOOL r = FALSE;
	STNBRtpClientOpq* opq = (STNBRtpClientOpq*)obj->opaque;
	STNBRtpClientPortRef pRef = NB_OBJREF_NULL; UI32 iSeq = 0; //STNBRtpClientStreamRef sRef = NB_OBJREF_NULL;
	{
		STNBRtpClientStreamRef srchh;
		STNBRtpClientStreamOpq srchhOpq;
		NBMemory_setZeroSt(srchh, STNBRtpClientStreamRef);
		NBMemory_setZeroSt(srchhOpq, STNBRtpClientStreamOpq);
		srchh.opaque		= &srchhOpq;
		srchhOpq.lstnr.ssrc	= ssrc;
		srchhOpq.lstnr.addr	= addr;
		NBThreadMutex_lock(&opq->mutex);
		{
			const SI32 iFnd = NBArraySorted_indexOf(&opq->streams.arr, &srchh, sizeof(srchh), NULL);
			if(iFnd >= 0){
				STNBRtpClientStreamRef sRef = NBArraySorted_itmValueAtIndex(&opq->streams.arr, STNBRtpClientStreamRef, iFnd);
				STNBRtpClientStreamOpq* s = (STNBRtpClientStreamOpq*)sRef.opaque;
				NBASSERT(NBRtpClientPort_isSet(s->port.port))
				if(NBRtpClientPort_isSet(s->port.port)){
					pRef = s->port.port;
					//port
					{
						STNBRtpClientPortOpq* p = (STNBRtpClientPortOpq*)pRef.opaque; NBASSERT(NBRtpClientPort_isClass(pRef))
						NBObject_lock(p);
						{
							//search remove action and nullify
							BOOL fnd = FALSE;
							SI32 i; for(i = 0; i < p->streams.pend.add.use; i++){
								STNBRtpClientStreamRef s2Ref = NBArray_itmValueAtIndex(&p->streams.pend.add, STNBRtpClientStreamRef, i);
								STNBRtpClientStreamOpq* s2 = (STNBRtpClientStreamOpq*)s2Ref.opaque;
								if(s == s2){
									NBArray_removeItemAtIndex(&p->streams.pend.add, i);
									NBRtpClientStream_release(&s2Ref);
									fnd = TRUE;
									//result
									r = TRUE;
								}
							}
							if(!fnd){
								//remove from port
								iSeq = ++p->streams.pend.iSeqReq;
								NBRtpClientStream_retain(sRef);
								NBArray_addValue(&p->streams.pend.rmv, sRef);
							}
						}
						NBObject_unlock(p);
					}
					NBArraySorted_removeItemAtIndex(&opq->streams.arr, iFnd);
					NBRtpClientStream_release(&sRef);
					//result
					r = TRUE;
				}
			}
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}

//Config

BOOL NBRtpClient_setExternalBuffers(STNBRtpClient* obj, STNBDataPtrsPoolRef pool){
	BOOL r = FALSE;
	STNBRtpClientOpq* opq = (STNBRtpClientOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	if(!opq->ports.areListening){
		NBDataPtrsPool_set(&opq->buffs.pool, &pool);
		r = TRUE;
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

BOOL NBRtpClient_setStatsProvider(STNBRtpClient* obj, STNBRtpClientStats* stats){
	BOOL r = FALSE;
	STNBRtpClientOpq* opq = (STNBRtpClientOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	if(!opq->ports.areListening){
		opq->stats.provider = stats;
		r = TRUE;
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

BOOL NBRtpClient_setCfg(STNBRtpClient* obj, const STNBRtpCfg* cfg){
	BOOL r = FALSE;
	STNBRtpClientOpq* opq = (STNBRtpClientOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	if(!opq->ports.areListening){
		r = TRUE;
		{
			UI64 maxSizeBytes = 0, initAllocBytes = 0, allocBuffSzBytes = 0;
			//"packetSizeMax"
			if(r && cfg != NULL){
				maxSizeBytes = cfg->packets.maxSizeBytes;
				if(!NBString_strIsEmpty(cfg->packets.maxSize)){
					const SI64 bytes = NBString_strToBytes(cfg->packets.maxSize);
					//PRINTF_INFO("NBRtpClient, '%s' parsed to %lld bytes\n", buffs2.minAlloc, bytes);
					if(bytes < 0){
						r = FALSE;
					} else {
						maxSizeBytes = (UI64)bytes;
					}
				}
			}
			//"initAllocBytes"
			if(r && cfg != NULL){
				initAllocBytes = cfg->packets.initAllocBytes;
				if(!NBString_strIsEmpty(cfg->packets.initAlloc)){
					const SI64 bytes = NBString_strToBytes(cfg->packets.initAlloc);
					//PRINTF_INFO("NBRtpClient, '%s' parsed to %lld bytes\n", buffs2.minAlloc, bytes);
					if(bytes < 0){
						r = FALSE;
					} else {
						initAllocBytes = (UI64)bytes;
					}
				}
			}
			//"allocBuffSzBytes"
			if(r && cfg != NULL){
				allocBuffSzBytes = cfg->perPort.allocBuffSzBytes;
				if(!NBString_strIsEmpty(cfg->perPort.allocBuffSz)){
					const SI64 bytes = NBString_strToBytes(cfg->perPort.allocBuffSz);
					//PRINTF_INFO("NBRtpClient, '%s' parsed to %lld bytes\n", buffs2.minAlloc, bytes);
					if(bytes < 0){
						r = FALSE;
					} else {
						allocBuffSzBytes = (UI64)bytes;
					}
				}
			}
			//Validate
			if(r){
				if(maxSizeBytes <= 0 || maxSizeBytes > 0xFFFFU){ //PacketMaxSize is required 
					r = FALSE;
				}
			}
			//Apply
			if(r){
				NBStruct_stRelease(NBRtpCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
				if(cfg != NULL){
					NBStruct_stClone(NBRtpCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
					//apply parsed valuesminAllocBytes
					opq->cfg.packets.maxSizeBytes		= (UI16)maxSizeBytes;
					opq->cfg.packets.initAllocBytes 	= initAllocBytes;
					opq->cfg.perPort.allocBuffSzBytes	= allocBuffSzBytes;
				}
			}
		}
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

BOOL NBRtpClient_setParentStopFlag(STNBRtpClient* obj, STNBStopFlagRef* stopFlag){
    BOOL r = FALSE;
    STNBRtpClientOpq* opq = (STNBRtpClientOpq*)obj->opaque;
    NBASSERT(opq != NULL)
    NBThreadMutex_lock(&opq->mutex);
    if(!opq->ports.areListening && stopFlag != NULL){
        //set
        NBStopFlag_setParentFlag(opq->stopFlag, stopFlag);
        //
        r = TRUE;
    }
    NBThreadMutex_unlock(&opq->mutex);
    return r;
}

BOOL NBRtpClient_setPollster(STNBRtpClient* obj, STNBIOPollsterRef pollster, STNBIOPollsterSyncRef pollSync){	//when one pollster only
	BOOL r = FALSE;
	STNBRtpClientOpq* opq = (STNBRtpClientOpq*)obj->opaque;
	NBASSERT(opq != NULL)
	NBThreadMutex_lock(&opq->mutex);
	if(!opq->ports.areListening){
		//set
		NBIOPollster_set(&opq->pollster.def, &pollster);
		NBIOPollsterSync_set(&opq->pollster.sync, &pollSync);
		//
		r = TRUE;
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

BOOL NBRtpClient_setPollstersProvider(STNBRtpClient* obj, STNBIOPollstersProviderRef provider){ //when multiple pollsters
	BOOL r = FALSE;
	STNBRtpClientOpq* opq = (STNBRtpClientOpq*)obj->opaque;
	NBASSERT(opq != NULL)
	NBThreadMutex_lock(&opq->mutex);
	if(!opq->ports.areListening){
		//set
		NBIOPollstersProvider_set(&opq->pollster.provider, &provider);
		//
		r = TRUE;
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

//Actions

BOOL NBRtpClient_prepare(STNBRtpClient* obj){
	BOOL r = FALSE;
	STNBRtpClientOpq* opq = (STNBRtpClientOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	if(!opq->ports.areListening
	   && opq->cfg.packets.maxSizeBytes > 0
	   && opq->cfg.ports != NULL && opq->cfg.portsSz > 0
	   && opq->cfg.perPort.packetsBuffSz > 0
	   && opq->cfg.perPort.allocBuffSzBytes > 0
	   && opq->cfg.perStream.packetsBuffSz > 1
	   && opq->cfg.perStream.orderingQueueSz > 0
	   ){
		r = TRUE;
		//Allocate buffers
		if(NBObjRef_isSet(opq->buffs.pool)){
			IF_PRINTF(const UI32 rr =) NBDataPtrsPool_allocPtrs(opq->buffs.pool, opq->cfg.packets.maxSizeBytes, (UI32)(opq->cfg.packets.initAllocBytes / opq->cfg.packets.maxSizeBytes));
			PRINTF_INFO("NBRtpClient, prepare %d pointers of %d byte allocated on ptrsPool.\n", rr, opq->cfg.packets.maxSizeBytes);
		}
		//ports
		{
			//allocate and assing pollsters
			if(r){
				UI32 i; for(i = 0; i < opq->cfg.portsSz; i++){
					STNBRtpClientPortRef pRef = NBRtpClientPort_alloc(NULL);
					STNBRtpClientPortOpq* p = (STNBRtpClientPortOpq*)pRef.opaque; NBASSERT(NBRtpClientPort_isClass(pRef))
					//cfg
					{
						p->opq	= opq;
						p->port	= opq->cfg.ports[i]; //number
                        NBStopFlag_setParentFlag(p->pollster.stopFlag, &opq->stopFlag);
					}
					//pollster
					if(p->port <= 0){
						PRINTF_ERROR("NBRtpClient, port(%d) #%d is not valid.\n", p->port, (i + 1));
						r = FALSE;
					} else {
						//get pollster
						STNBIOPollsterRef pollster = NB_OBJREF_NULL;
						//get a pollster from provider
						if(NBIOPollstersProvider_isSet(opq->pollster.provider)){
							pollster = NBIOPollstersProvider_getPollster(opq->pollster.provider); 
						}
						//use default pollster
						if(!NBIOPollster_isSet(pollster)){
							pollster = opq->pollster.def;
						}
						//result
						if(!NBIOPollster_isSet(pollster)){
							PRINTF_ERROR("NBRtpClient, port(%d) #%d pollster could not be defined.\n", p->port, (i + 1));
							r = FALSE;
						} else {
							NBIOPollster_set(&p->pollster.ref, &pollster);
						}
					}
					//result
					if(!r){
						PRINTF_ERROR("NBRtpClient, port(%d) #%d is not valid.\n", p->port, (i + 1));
						NBRtpClientPort_release(&pRef);
						NBRtpClientPort_null(&pRef);
						r = FALSE;
						break;
					} else {
						//stats
						{
							//provider
							if(opq->stats.provider != NULL){
								p->stats.provider = NBMemory_allocType(STNBRtpClientStats);
								NBRtpClientStats_init(p->stats.provider);
							}
						}
						//buffers
						{
							p->buffs.bytesPerPacket = opq->cfg.packets.maxSizeBytes;
							//pool
							{
								NBDataPtrsPool_set(&p->buffs.pool, &opq->buffs.pool);
							}
							//alloc
							{
								//release
								if(p->buffs.alloc.ptrs != NULL){
									NBMemory_free(p->buffs.alloc.ptrs);
									p->buffs.alloc.ptrs = NULL;
								}
								//alloc
								p->buffs.alloc.filled	= 0;
								p->buffs.alloc.emptied	= 0;
								if(opq->cfg.perPort.allocBuffSzBytes > 0 && NBDataPtrsPool_isSet(p->buffs.pool)){
									p->buffs.alloc.size		= (UI32)(opq->cfg.perPort.allocBuffSzBytes / opq->cfg.packets.maxSizeBytes);
									if(p->buffs.alloc.size != 0 && NBObjRef_isSet(opq->buffs.pool)){
										UI32 i, allocated = 0;
										p->buffs.alloc.ptrs = NBMemory_allocTypes(STNBDataPtr, p->buffs.alloc.size);
										//init
										for(i = 0; i < p->buffs.alloc.size; i++){
											NBDataPtr_init(&p->buffs.alloc.ptrs[i]);
										}
										while(p->buffs.alloc.filled < p->buffs.alloc.size){
											p->buffs.alloc.filled += (allocated = NBDataPtrsPool_getPtrs(p->buffs.pool, opq->cfg.packets.maxSizeBytes, &p->buffs.alloc.ptrs[p->buffs.alloc.filled], p->buffs.alloc.size - p->buffs.alloc.filled));
											if(allocated == 0){
												break;
											} else {
												PRINTF_INFO("NBRtpClientPort, pre-allocated %d chuncks (%d bytes each).\n", allocated, opq->cfg.packets.maxSizeBytes);
											}
										}
										NBASSERT(p->buffs.alloc.filled == p->buffs.alloc.size)
									}
								}
							}
							//tmp
							{
								//fndPtrs
								{
									if(p->buffs.tmp.fndPtrs.arr != NULL){
										NBMemory_free(p->buffs.tmp.fndPtrs.arr);
										p->buffs.tmp.fndPtrs.arr = NULL;
										p->buffs.tmp.fndPtrs.sz = 0;
									}
									if(opq->cfg.perPort.packetsBuffSz > 0){
										p->buffs.tmp.fndPtrs.sz	= opq->cfg.perPort.packetsBuffSz;
										p->buffs.tmp.fndPtrs.arr	= NBMemory_allocTypes(STNBRtpClientPacketFnd, p->buffs.tmp.fndPtrs.sz);
									}
								}
								//grpdPtrs
								{
									if(p->buffs.tmp.grpdPtrs.arr != NULL){
										NBMemory_free(p->buffs.tmp.grpdPtrs.arr);
										p->buffs.tmp.grpdPtrs.arr = NULL;
										p->buffs.tmp.grpdPtrs.sz = 0;
									}
									if(opq->cfg.perPort.packetsBuffSz > 0){
										p->buffs.tmp.grpdPtrs.sz		= opq->cfg.perPort.packetsBuffSz;
										p->buffs.tmp.grpdPtrs.arr	= NBMemory_allocTypes(STNBRtpClientPacketFnd, p->buffs.tmp.grpdPtrs.sz);
									}
								}
							}
							//resize buffer (if necesary)
							if(p->buffs.pckts.size != opq->cfg.perPort.packetsBuffSz){
								//headers
								{
									if(p->buffs.hdrs != NULL){
										if(p->buffs.pckts.size > 0){
											NBRtpHdrBasic_hdrsRelease(p->buffs.hdrs, p->buffs.pckts.size)
										}
										NBMemory_free(p->buffs.hdrs);
										p->buffs.hdrs = NULL;
									}
									p->buffs.hdrs	= NBMemory_allocTypes(STNBRtpHdrBasic, opq->cfg.perPort.packetsBuffSz);
									//init headers
									{
										UI32 i; for(i = 0; i < opq->cfg.perPort.packetsBuffSz; i++){
											STNBRtpHdrBasic* hdr = &p->buffs.hdrs[i];
											NBRtpHdrBasic_init(hdr);
										}
									}
								}
								//packets
								{
									NBSocketPackets_allocBuffersSameSizes(&p->buffs.pckts, opq->cfg.perPort.packetsBuffSz, opq->cfg.packets.maxSizeBytes);
									NBASSERT(p->buffs.pckts.size == opq->cfg.perPort.packetsBuffSz)
								}
							}
						}
						//add
						NBArray_addValue(&opq->ports.arr, pRef);
					}
				}
			}
			//bind and add to pollsters
			if(r){
				SI32 i; for(i = 0; i < opq->ports.arr.use; i++){
					STNBRtpClientPortRef pRef = NBArray_itmValueAtIndex(&opq->ports.arr, STNBRtpClientPortRef, i);
					STNBRtpClientPortOpq* p = (STNBRtpClientPortOpq*)pRef.opaque; NBASSERT(NBRtpClientPort_isClass(pRef))
					NBSocket_setReuseAddr(p->socket, TRUE); //Recommended before bind
					NBSocket_setReusePort(p->socket, TRUE); //Recommended before bind
					if(!NBSocket_bind(p->socket, p->port)){
						PRINTF_CONSOLE_ERROR("NBRtpClient, port(%d) #%d/%d bind failed.\n", p->port, (i + 1), opq->ports.arr.use);
						r = FALSE;
						break;
					} else {
						PRINTF_INFO("NBRtpClient, port(%d) #%d/%d binded.\n", p->port, (i + 1), opq->ports.arr.use);
					}
				}
				//success
				if(r){
					opq->ports.areListening = TRUE;
				}
			}
		}
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

//pollster callbacks
void NBRtpClientPort_pollConsumeMask_(STNBIOLnk ioLnk, const UI8 pollMask, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData);
void NBRtpClientPort_pollConsumeNoOp_(STNBIOLnk ioLnk, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData);
void NBRtpClientPort_pollRemoved_(STNBIOLnk ioLnk, void* usrData);

BOOL NBRtpClient_startListening(STNBRtpClient* obj){
	BOOL r = FALSE;
	STNBRtpClientOpq* opq = (STNBRtpClientOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	if(opq->ports.areListening){
		r = TRUE;
		//Add to pollsters
		if(r){
			SI32 i; for(i = 0; i < opq->ports.arr.use; i++){
				STNBRtpClientPortRef pRef = NBArray_itmValueAtIndex(&opq->ports.arr, STNBRtpClientPortRef, i);
				STNBRtpClientPortOpq* p = (STNBRtpClientPortOpq*)pRef.opaque; NBASSERT(NBRtpClientPort_isClass(pRef))
				if(!p->pollster.isListening){
					STNBIOPollsterLstrnItf itf;
					NBMemory_setZeroSt(itf, STNBIOPollsterLstrnItf);
					itf.pollConsumeMask = NBRtpClientPort_pollConsumeMask_;
					itf.pollConsumeNoOp = NBRtpClientPort_pollConsumeNoOp_;
					itf.pollRemoved		= NBRtpClientPort_pollRemoved_;
					if(!NBIOPollster_addSocketWithItf(p->pollster.ref, p->socket, ENNBIOPollsterOpBit_Read, &itf, p)){
						PRINTF_ERROR("NBRtpClient, port(%d) #%d NBIOPollster_addSocket failed.\n", p->port, (i + 1));
						r = FALSE;
						break;
					} else {
						PRINTF_INFO("NBRtpClient, port(%d) #%d at pollster.\n", p->port, (i + 1));
						p->pollster.isListening = TRUE;
					}
				}
			}
			//revert (if error)
			if(!r){
				for(i = 0; i < opq->ports.arr.use; i++){
					STNBRtpClientPortRef pRef = NBArray_itmValueAtIndex(&opq->ports.arr, STNBRtpClientPortRef, i);
					STNBRtpClientPortOpq* p = (STNBRtpClientPortOpq*)pRef.opaque; NBASSERT(NBRtpClientPort_isClass(pRef))
                    NBStopFlag_activate(p->pollster.stopFlag);
				}
			}
		}
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

BOOL NBRtpClient_isBusy(STNBRtpClient* obj){
	BOOL r = FALSE;
	STNBRtpClientOpq* opq = (STNBRtpClientOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	{
		SI32 i; for(i = 0; i < opq->ports.arr.use && !r; i++){
			STNBRtpClientPortRef pRef = NBArray_itmValueAtIndex(&opq->ports.arr, STNBRtpClientPortRef, i);
			STNBRtpClientPortOpq* p = (STNBRtpClientPortOpq*)pRef.opaque; NBASSERT(NBRtpClientPort_isClass(pRef))
			if(p->pollster.isListening){
				r = TRUE;
			}
		}
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

void NBRtpClient_stopFlag(STNBRtpClient* obj){
	STNBRtpClientOpq* opq = (STNBRtpClientOpq*)obj->opaque;
    NBStopFlag_activate(opq->stopFlag);
}

// inline and optimized NBDataPtrs releaser (adds them to the buffer instead of sending them to the pool)

void NBRtpClientStream_ptrsReleaseGroupedOpq_(STNBDataPtr* arrPtrs, const UI32 size, void* usrData){
	STNBRtpClientStreamOpq* s = (STNBRtpClientStreamOpq*)usrData;
	const STNBDataPtr* arrPtrsAfterEnd = arrPtrs + size;
	UI32 reused = 0, atomicReld = 0;
	if(NBRtpClientPort_isSet(s->port.port)){
		STNBRtpClientPortRef pRef = s->port.port;
		STNBRtpClientPortOpq* p = (STNBRtpClientPortOpq*)pRef.opaque; NBASSERT(NBRtpClientPort_isClass(pRef))
		while(p->buffs.alloc.emptied != 0 && arrPtrs < arrPtrsAfterEnd){
			if(arrPtrs->def.allocType == ENNBDataPtrAllocType_PoolReturn && arrPtrs->def.alloc.size == p->buffs.bytesPerPacket && NBDataPtrsPool_isSet(arrPtrs->def.alloc.pool)){
				//optimization, return to alloc-buffers pool (no lock)
				STNBDataPtr* dst = &p->buffs.alloc.ptrs[p->buffs.alloc.emptied - 1];
				NBASSERT(dst->def.allocType != ENNBDataPtrAllocType_PoolReturn)
				NBASSERT(!NBDataPtrsPool_isSet(dst->def.alloc.pool))
				if(dst->def.allocType != ENNBDataPtrAllocType_PoolReturn && !NBDataPtrsPool_isSet(dst->def.alloc.pool)){
					p->buffs.alloc.emptied--;
					NBDataPtr_empty(arrPtrs);	//empty data
					NBDataPtr_release(dst);		//release
					*dst = *arrPtrs;			//swap
					reused++;
				} else {
					//stop
					break;
				}
			} else {
				//release
				NBDataPtr_release(arrPtrs);
				atomicReld++;
			}
			//next
			arrPtrs++;
		}
	}
	//stats
	//if(reused != 0 /*|| atomicReld != 0*/){
	//	PRINTF_INFO("NBRtpClientStream, %d buffers reused, %d atomic-released and %d grpd-released.\n", reused, atomicReld, (UI32)(arrPtrsAfterEnd - arrPtrs));
	//}
	//release remain
	if(arrPtrs < arrPtrsAfterEnd){
		NBDataPtr_ptrsReleaseGrouped(arrPtrs, (UI32)(arrPtrsAfterEnd - arrPtrs));
	}
}

BOOL NBRtpClientStream_notifyPacketsIfBorderOrFull_(STNBRtpClientStreamRef sRef, const STNBRtpCfg* cfg, STNBRtpClientStatsUpd* dstStats){
	BOOL r = FALSE;
	STNBRtpClientStreamOpq* s = NULL;
	if(NBRtpClientStream_isSet(sRef) && (s = (STNBRtpClientStreamOpq*)sRef.opaque) != NULL && s->lstnr.rtpConsume != NULL){
		UI32 iNxtRead			= s->buffs.iNxtRead;
		const UI32 iNxtWrite	= s->buffs.iNxtWrite; //use local var to avoid reading more than pcktsSz buffer ('iNxtWrite' can be modified in parallel to 'iNxtRead').
		const UI32 iNxtWrite2	= ((iNxtWrite + 1) % s->buffs.sz);
		const UI32 filledAvail	= (iNxtRead == iNxtWrite ? 0 : iNxtWrite + (iNxtWrite < iNxtRead ? s->buffs.sz : 0) - iNxtRead);
		const BOOL isFull		= (iNxtWrite2 == iNxtRead);
		if(filledAvail > 0){
			NBASSERT(s->notify.hdrs.use == s->notify.chunks.use)
			const SI32 iFirstNewChunk = s->notify.chunks.use;
			//Add chunks to queue and notifyArrays
			{
				UI32 pCount = 0;
				while(iNxtRead != iNxtWrite){
					STNBRtpHdrBasic* hdr = &s->buffs.hdrs[iNxtRead];
					STNBDataPtr* chunk = &s->buffs.chunks[iNxtRead];
					if(!NBRtpQueue_addPacket(&s->queue, hdr, chunk, &s->notify.hdrs, &s->notify.chunks, &dstStats->csmd)){
						dstStats->csmd.ignored.count++;
						dstStats->csmd.ignored.bytesCount += chunk->use;
					}
					//
					dstStats->csmd.count++;
					dstStats->csmd.bytesCount += chunk->use;
					//next
					pCount++;
					iNxtRead = (iNxtRead + 1) % s->buffs.sz;
				}
				NBASSERT(pCount > 0)
				NBASSERT(pCount == filledAvail)
				//set new read-pos
				s->buffs.iNxtRead = iNxtRead;
			}
			//Notify
			NBASSERT(s->notify.hdrs.use == s->notify.chunks.use)
			if(s->notify.chunks.use > 0){
				BOOL notifyChunks = isFull;
				//Detect border to notify
				if(!notifyChunks && iFirstNewChunk < s->notify.chunks.use){
					const STNBRtpHdrBasic* arrHdrs	= NBArray_itmPtrAtIndex(&s->notify.hdrs, STNBRtpHdrBasic, iFirstNewChunk);
					const STNBDataPtr* arrPtrs		= NBArray_itmPtrAtIndex(&s->notify.chunks, STNBDataPtr, iFirstNewChunk);
					if(s->lstnr.rtpBorderPresent != NULL){
						notifyChunks = (*s->lstnr.rtpBorderPresent)(s->lstnr.obj, arrHdrs, arrPtrs, s->notify.chunks.use - iFirstNewChunk);
						//PRINTF_INFO("NBRtpClient, %d chunks does%s requires notification.\n", lstnr->notify.chunks.use, (notifyChunks ? "" : " NOT"));
					}
				}
				//Notify
				if(notifyChunks){
					STNBRtpHdrBasic* arrHdrs	= NBArray_dataPtr(&s->notify.hdrs, STNBRtpHdrBasic);
					STNBDataPtr* arrPtrs		= NBArray_dataPtr(&s->notify.chunks, STNBDataPtr);
					//Notify RTPPackets
					if(!cfg->debug.doNotNotify){
						STNBDataPtrReleaser ptrsReleaser;
						NBMemory_setZeroSt(ptrsReleaser, STNBDataPtrReleaser);
						ptrsReleaser.itf.ptrsReleaseGrouped = NBRtpClientStream_ptrsReleaseGroupedOpq_;
						ptrsReleaser.usrData = s;
						//ToDo: implement STNBDataPtrReleaser method
						(*s->lstnr.rtpConsume)(s->lstnr.obj, arrHdrs, arrPtrs, s->notify.chunks.use, dstStats, &ptrsReleaser);
					}
					//release
					//PRINTF_INFO("NBRtpClient, notified and released %d packets.\n", lstnr->notify.hdrs.use);
					//headers
					{
						if(s->notify.hdrs.use > 0){
							NBRtpHdrBasic_hdrsRelease(arrHdrs, s->notify.hdrs.use);
						}
						NBArray_empty(&s->notify.hdrs);
					}
					//chunks
					if(s->notify.chunks.use > 0){
						NBRtpClientStream_ptrsReleaseGroupedOpq_(arrPtrs, s->notify.chunks.use, s);
						NBArray_empty(&s->notify.chunks);
					}
				}
			}
			//result
			r = TRUE;
		}
	}
	return r;
}

void NBRtpClientStream_queuePacketsSameLstnr_(const STNBRtpCfg* cfg, STNBRtpClientPacketFnd* pckts, const UI32 pcktsSz){
	NBASSERT(pcktsSz > 0) //never call empty
	STNBRtpClientPacketFnd* frst	= &pckts[0];
	STNBRtpClientStreamRef lstnrRef	= frst->stream;
	STNBRtpClientStreamOpq* lstnr	= (STNBRtpClientStreamOpq*)lstnrRef.opaque;
	NBASSERT(frst->hdr != NULL && frst->from != NULL && frst->chunk != NULL && frst->chunk->ptr != NULL && frst->chunk->use > 0 && NBRtpClientStream_isSet(frst->stream) && frst->streamSet && frst->grouped && !frst->notified)
	//Process packets grouped by listener
	STNBRtpClientPacketFnd* fnd = frst;
	const STNBRtpClientPacketFnd* fndAfterEnd = frst + pcktsSz;
	while(fnd < fndAfterEnd){
		NBASSERT(!fnd->notified && fnd->hdr != NULL && fnd->from != NULL && fnd->chunk != NULL && fnd->chunk->ptr != NULL && fnd->chunk->use > 0 && NBRtpClientStream_isSet(fnd->stream) && fnd->streamSet && frst->grouped && NBRtpClientStream_isSame(fnd->stream, frst->stream))
		UI32 iNxtWrite	= lstnr->buffs.iNxtWrite;
		UI32 iNxtWrite2	= ((iNxtWrite + 1) % lstnr->buffs.sz);
		//Is full?
		NBASSERT(iNxtWrite == lstnr->buffs.iNxtWrite)
		if(iNxtWrite2 == lstnr->buffs.iNxtRead){
			if(NBRtpClientStream_notifyPacketsIfBorderOrFull_(lstnrRef, cfg, &lstnr->stats.upd)){
				//
			}
			//get updated values
			iNxtWrite	= lstnr->buffs.iNxtWrite;
			iNxtWrite2	= ((iNxtWrite + 1) % lstnr->buffs.sz);
		}
		//Add to queue
		NBASSERT(iNxtWrite == lstnr->buffs.iNxtWrite)
		if(iNxtWrite2 != lstnr->buffs.iNxtRead){
			//Swap packet buffer
			STNBRtpHdrBasic* lstnrHdr	= &lstnr->buffs.hdrs[iNxtWrite];
			STNBDataPtr* lstnrPtr		= &lstnr->buffs.chunks[iNxtWrite];
			if(NBDataPtr_swapCompatibleOther(lstnrPtr, fnd->chunk)){
				STNBRtpHdrBasic hdrTmp	= *lstnrHdr;
				*lstnrHdr				= *fnd->hdr;
				*fnd->hdr				= hdrTmp;
				lstnr->buffs.iNxtWrite	= iNxtWrite2;
			} else {
				//packet dropped (internal error)
				lstnr->stats.upd.rcvd.overBuff.count++;
				lstnr->stats.upd.rcvd.overBuff.bytesCount += fnd->chunk->use;
			} 
		} else {
			//packet dropped (internal error)
			lstnr->stats.upd.rcvd.overBuff.count++;
			lstnr->stats.upd.rcvd.overBuff.bytesCount += fnd->chunk->use;
		}
		//next
		fnd++;
	}
	//Last step for listener-grp
	if(NBRtpClientStream_notifyPacketsIfBorderOrFull_(lstnrRef, cfg, &lstnr->stats.upd)){
		//
	}
}

void NBRtpClient_portNotifyPackets_(STNBRtpClientOpq* opq, STNBRtpClientPortOpq* p, STNBRtpClientPacketFnd* fndPtrs, const UI32 fndPtrsSz, STNBRtpClientPacketFnd* grpdPtrs, const UI32 grpdPtrsSz){
	UI32 fndPtrsUse = 0;
	//build fnd-list
	{
		NBASSERT(fndPtrsUse == 0)
		UI32 i; for(i = 0; i < p->buffs.pckts.use; i++){
			STNBRtpHdrBasic* hdrPtr	= &p->buffs.hdrs[i];
			if(hdrPtr->head.ssrc != 0){ //parsed
				STNBRtpClientPacketFnd* fnd = &fndPtrs[fndPtrsUse++];
				STNBSocketAddr* addrPtr	= &p->buffs.pckts.addrs[i];
				STNBDataPtr* dataPtr	= &p->buffs.pckts.buffs[i];
				NBASSERT(dataPtr->use > 0 && NBDataPtr_allocatedSize(dataPtr) > 0)
				NBASSERT(dataPtr->ptr != NULL && dataPtr->use > 0 && dataPtr->use <= NBDataPtr_allocatedSize(dataPtr))
				NBASSERT(NBDataPtr_allocatedSize(dataPtr) == opq->cfg.packets.maxSizeBytes)
				NBMemory_setZeroSt(*fnd, STNBRtpClientPacketFnd);
				//set
				NBASSERT(NBDataPtr_allocatedSize(dataPtr) == opq->cfg.packets.maxSizeBytes)
				fnd->from			= addrPtr;
				fnd->hdr			= hdrPtr;
				fnd->chunk			= dataPtr;
			}
		}
	}
	//notify fnd-list (gropued by ssrc)
	if(fndPtrsUse > 0){
		IF_NBASSERT(const UI32 fndPtrsUseBefore = fndPtrsUse;)
		IF_NBASSERT(UI32 dbgFndPtrsEvalCount = 0;)
		//PRINTF_INFO("NBRtpClient, finding listener of %d udp packets.\n", fndPtrsUse);
		UI32 i; for(i = 0; i < fndPtrsUse; i++){
			STNBRtpClientPacketFnd* fnd = &fndPtrs[i];
			STNBRtpClientStreamOpq* fndOpq = NULL;
			NBASSERT(fnd->hdr != NULL && fnd->chunk->ptr != NULL && fnd->chunk->use > 0 && !fnd->streamSet)
			//port stats
			{
				//stats
				if(p->stats.upd.rcvd.pcktsCount == 0 || p->stats.upd.rcvd.minBytes > fnd->chunk->use){
					p->stats.upd.rcvd.minBytes = fnd->chunk->use;
				}
				if(p->stats.upd.rcvd.pcktsCount == 0 || p->stats.upd.rcvd.maxBytes < fnd->chunk->use){
					p->stats.upd.rcvd.maxBytes = fnd->chunk->use;
				}
				p->stats.upd.rcvd.pcktsCount++;
				p->stats.upd.rcvd.bytesCount += fnd->chunk->use;
				//
				p->stats.upd.csmd.count++;
				p->stats.upd.csmd.bytesCount += fnd->chunk->use;
			}
			//group same-stream packets
			{
				UI32 grpdPtrsUse	= 0;
				//Find lstnr
				STNBRtpClientStreamRef srchhRef;
				STNBRtpClientStreamOpq srchhOpq;
				srchhRef.opaque		= &srchhOpq;
				srchhOpq.lstnr.ssrc	= fnd->hdr->head.ssrc;
				srchhOpq.lstnr.addr	= *fnd->from;
				const SI32 iFnd = NBArraySorted_indexOf(&p->streams.arr, &srchhRef, sizeof(srchhRef), NULL);
				if(iFnd < 0){
					//packet ignored (no registered consumer)
					p->stats.upd.csmd.ignored.count++;
					p->stats.upd.csmd.ignored.bytesCount += fnd->chunk->use;
					//
					NBRtpClientStream_null(&fnd->stream);
				} else {
					//Apply stream-stats
					fnd->stream	= NBArraySorted_itmValueAtIndex(&p->streams.arr, STNBRtpClientStreamRef, iFnd);
					fndOpq		= (STNBRtpClientStreamOpq*)fnd->stream.opaque;
					//Stats
					if(fndOpq->stats.upd.rcvd.pcktsCount == 0 || fndOpq->stats.upd.rcvd.minBytes > fnd->chunk->use){
						fndOpq->stats.upd.rcvd.minBytes = fnd->chunk->use;
					}
					if(fndOpq->stats.upd.rcvd.pcktsCount == 0 || fndOpq->stats.upd.rcvd.maxBytes < fnd->chunk->use){
						fndOpq->stats.upd.rcvd.maxBytes = fnd->chunk->use;
					}
					fndOpq->stats.upd.rcvd.pcktsCount++;
					fndOpq->stats.upd.rcvd.bytesCount += fnd->chunk->use;
				}
				//
				IF_NBASSERT(fnd->streamSet	= TRUE;)
				IF_NBASSERT(fnd->grouped	= TRUE;)
				IF_NBASSERT(dbgFndPtrsEvalCount++;)
				//
				grpdPtrs[grpdPtrsUse++] = *fnd;
				//Apply same listener to others
				{
					UI32 grpGapSz = 0; 
					UI32 i2; for(i2 = i + 1; i2 < fndPtrsUse; i2++){
						STNBRtpClientPacketFnd* fnd2 = &fndPtrs[i2];
						STNBRtpClientStreamOpq* fnd2Opq = NULL;
						NBASSERT(fnd2->hdr != NULL && fnd2->chunk->ptr != NULL && fnd2->chunk->use > 0 && !fnd2->streamSet)
						if(fnd2->hdr->head.ssrc == fnd->hdr->head.ssrc){
							//Add to group
							fnd2->stream	= fnd->stream;
							fnd2Opq			= (STNBRtpClientStreamOpq*)fnd2->stream.opaque;
							//
							IF_NBASSERT(fnd2->streamSet	= TRUE;)
							IF_NBASSERT(fnd2->grouped	= TRUE;)
							IF_NBASSERT(dbgFndPtrsEvalCount++;)
							//port-stats
							{
								if(p->stats.upd.rcvd.pcktsCount == 0 || p->stats.upd.rcvd.minBytes > fnd2->chunk->use){
									p->stats.upd.rcvd.minBytes = fnd2->chunk->use;
								}
								if(p->stats.upd.rcvd.pcktsCount == 0 || p->stats.upd.rcvd.maxBytes < fnd2->chunk->use){
									p->stats.upd.rcvd.maxBytes = fnd2->chunk->use;
								}
								p->stats.upd.rcvd.pcktsCount++;
								p->stats.upd.rcvd.bytesCount += fnd2->chunk->use;
								//
								p->stats.upd.csmd.count++;
								p->stats.upd.csmd.bytesCount += fnd2->chunk->use;
							}
							//stream-stats
							if(!NBRtpClientStream_isSet(fnd2->stream)){
								//packet ignored (no registered consumer)
								p->stats.upd.csmd.ignored.count++;
								p->stats.upd.csmd.ignored.bytesCount += fnd2->chunk->use;
							} else {
								//packet rcvd for registered consumer
								if(fnd2Opq->stats.upd.rcvd.pcktsCount == 0 || fnd2Opq->stats.upd.rcvd.minBytes > fnd2->chunk->use){
									fnd2Opq->stats.upd.rcvd.minBytes = fnd2->chunk->use;
								}
								if(fnd2Opq->stats.upd.rcvd.pcktsCount == 0 || fnd2Opq->stats.upd.rcvd.maxBytes < fnd2->chunk->use){
									fnd2Opq->stats.upd.rcvd.maxBytes = fnd2->chunk->use;
								}
								fnd2Opq->stats.upd.rcvd.pcktsCount++;
								fnd2Opq->stats.upd.rcvd.bytesCount += fnd2->chunk->use;
							}
							grpdPtrs[grpdPtrsUse++] = *fnd2;
							grpGapSz++;
						} else if(grpGapSz != 0){
							//fill gap produced by already grouped elements
							fndPtrs[i2 - grpGapSz] = fndPtrs[i2];
						}
					}
					//consume group
					NBASSERT(grpGapSz <= fndPtrsUse)
					fndPtrsUse -= grpGapSz;
				}
				//Notify
				NBASSERT(grpdPtrsUse > 0)
				//PRINTF_INFO("NBRtpClient, rcvr notifying %d packets (%s).\n", grpdPtrsUse, (NBRtpClientStream_isSet(fnd->stream) ? "lstnr" : "no-lstnr"));
				if(NBRtpClientStream_isSet(fnd->stream)){
					NBRtpClientStream_queuePacketsSameLstnr_(&opq->cfg, grpdPtrs, grpdPtrsUse);
					//Apply listener-stats to port-stats
					if(opq->cfg.atomicStats){
						//ToDo: implement
						//NBRtpClient_flushStatsRcvrItm_(opq, fnd->lstnr, NULL);
					}
				}
			}
		}
		NBASSERT(fndPtrsUseBefore == dbgFndPtrsEvalCount)
	}
	//Reset buffer
	NBSocketPackets_syncInternalRng(&p->buffs.pckts, 0, p->buffs.pckts.use);
	NBSocketPackets_empty(&p->buffs.pckts);
}

void NBRtpClient_portConsumePendTaskStreamsSyncLockedOpq_(STNBRtpClientOpq* opq, STNBRtpClientPortOpq* p){
	//remove streams
	{
		SI32 i; for(i = 0; i < p->streams.pend.rmv.use; i++){
			BOOL fnd = FALSE;
			STNBRtpClientStreamRef s = NBArray_itmValueAtIndex(&p->streams.pend.rmv, STNBRtpClientStreamRef, i);
			//Find
			{
				SI32 i; for(i = 0; i < p->streams.arr.use; i++){
					STNBRtpClientStreamRef s2 = NBArraySorted_itmValueAtIndex(&p->streams.arr, STNBRtpClientStreamRef, i);
					if(NBRtpClientStream_isSame(s, s2)){
						NBArraySorted_removeItemAtIndex(&p->streams.arr, i);
						NBRtpClientStream_release(&s2);
						fnd = TRUE;
						break;
					}
				}
			} NBASSERT(fnd)
			NBRtpClientStream_release(&s);
		}
		NBArray_empty(&p->streams.pend.rmv);
	}
	//add streams
	{
		SI32 i; for(i = 0; i < p->streams.pend.add.use; i++){
			STNBRtpClientStreamRef s = NBArray_itmValueAtIndex(&p->streams.pend.add, STNBRtpClientStreamRef, i);
#			ifdef NB_CONFIG_INCLUDE_ASSERTS
			{
				SI32 i; for(i = 0; i < p->streams.arr.use; i++){
					STNBRtpClientStreamRef s2 = NBArraySorted_itmValueAtIndex(&p->streams.arr, STNBRtpClientStreamRef, i);
					NBASSERT(!NBRtpClientStream_isSame(s, s2)) //program logic error, duplicated add
				}
			}
#			endif
			NBArraySorted_addValue(&p->streams.arr, s);
		}
		NBArray_empty(&p->streams.pend.add);
	}
	p->streams.pend.iSeqDone = p->streams.pend.iSeqReq; 
}

void NBRtpClient_portConsumePendTaskStatsFlushLockedOpq_(STNBRtpClientOpq* opq, STNBRtpClientPortOpq* p){
	//apply port stats
	{
		//to parent stats
		if(opq->stats.provider != NULL){
			NBRtpClientStats_applyUpdate(opq->stats.provider, &p->stats.upd);
		}
		//to port stats
		if(p->stats.provider != NULL){
			NBRtpClientStats_applyUpdate(p->stats.provider, &p->stats.upd);
		}
		//reset
		NBRtpClientStatsUpd_reset(&p->stats.upd);
	}
	//apply streams stats
	{
		SI32 i; for(i = 0; i < p->streams.arr.use; i++){
			STNBRtpClientStreamRef sRef = NBArraySorted_itmValueAtIndex(&p->streams.arr, STNBRtpClientStreamRef, i);
			BOOL flushStats = FALSE;
			STNBRtpClientStreamOpq* s = (STNBRtpClientStreamOpq*)sRef.opaque; 
			NBASSERT(s->port.num == p->port)
			NBObject_lock(s);
			if(s->stats.flush.iSeqDone != s->stats.flush.iSeqReq){
				s->stats.flush.iSeqDone = s->stats.flush.iSeqReq;
				//apply stream stats
				if(s->stats.provider != NULL){
					NBRtpClientStats_applyUpdate(s->stats.provider, &s->stats.upd);
					NBRtpClientStatsUpd_reset(&s->stats.upd);
				}
				flushStats = TRUE;
			}
			NBObject_unlock(s);
			//call itf (unlocked)
			if(flushStats && s->lstnr.rtpFlushStats != NULL){
				(*s->lstnr.rtpFlushStats)(s->lstnr.obj);
			}
		}
	}
	p->stats.flush.iSeqDone = p->stats.flush.iSeqReq;
}
	
void NBRtpClient_portConsumePendTasksLockedOpq_(STNBRtpClientOpq* opq, STNBRtpClientPortOpq* p){
	//consume streams
	if(NB_RTP_CLIENT_PORT_HAS_PEND_TASK_STREAMS_SYNC(p)){
		NBRtpClient_portConsumePendTaskStreamsSyncLockedOpq_(opq, p);
		NBASSERT(!NB_RTP_CLIENT_PORT_HAS_PEND_TASK_STREAMS_SYNC(p))
	}
	//consume flush stats request
	if(NB_RTP_CLIENT_PORT_HAS_PEND_TASK_FLUSH_STATS(p)){
		NBRtpClient_portConsumePendTaskStatsFlushLockedOpq_(opq, p);
		NBASSERT(!NB_RTP_CLIENT_PORT_HAS_PEND_TASK_FLUSH_STATS(p))
	}
}

void NBRtpClient_portConsumePendTasksOpq_(STNBRtpClientOpq* opq, STNBRtpClientPortOpq* p){
	NBObject_lock(p);
	{
		NBRtpClient_portConsumePendTasksLockedOpq_(opq, p);
	}
	NBObject_unlock(p);
}

//pollster callbacks

void NBRtpClientPort_pollConsumeMask_(STNBIOLnk ioLnk, const UI8 pollMask, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData){
	STNBRtpClientPortOpq* p = (STNBRtpClientPortOpq*)usrData;
	STNBRtpClientOpq* opq = p->opq; NBASSERT(opq != NULL)
	IF_NBASSERT(STNBTimestampMicro timeLastAction = NBTimestampMicro_getMonotonicFast(), timeCur; SI64 usDiff;)
	//read
	if(pollMask & ENNBIOPollsterOpBit_Read){
		//read packets
		const UI32 fndPtrsSz				= p->buffs.tmp.fndPtrs.sz;
		const UI32 grpdPtrsSz				= p->buffs.tmp.grpdPtrs.sz;
		STNBRtpClientPacketFnd* fndPtrs		= p->buffs.tmp.fndPtrs.arr; //found
		STNBRtpClientPacketFnd* grpdPtrs	= p->buffs.tmp.grpdPtrs.arr; //grouped by ssrc
		//read packets
		{
			IF_NBASSERT(STNBTimestampMicro timeLastAction = NBTimestampMicro_getMonotonicFast(), timeCur; SI64 usDiff;)
			NBRtpClientPort_rcvMany_(p, p->socket, opq->cfg.debug.packetLostSimDiv);
#			ifdef NB_CONFIG_INCLUDE_ASSERTS
			{
				timeCur	= NBTimestampMicro_getMonotonicFast();
				usDiff	= NBTimestampMicro_getDiffInUs(&timeLastAction, &timeCur);
				/*if(usDiff >= 1000ULL){
					PRINTF_INFO("NBRtpClientPort_rcvMany_ took %llu.%llu%llums.\n", (usDiff / 1000ULL), (usDiff % 1000ULL) % 100ULL, (usDiff % 100ULL) % 10ULL);
				}*/
				timeLastAction = timeCur;
			}
#			endif
		}
		//notify (if buffer is full or mininum was reached)
		if(!NBStopFlag_isMineActivated(opq->stopFlag) && p->buffs.pckts.use > 0 /*&& (p->buffs.pckts.use >= p->buffs.pckts.size)*/){
			IF_NBASSERT(STNBTimestampMicro timeLastAction = NBTimestampMicro_getMonotonicFast(), timeCur; SI64 usDiff;)
			NBRtpClient_portNotifyPackets_(opq, p, fndPtrs, fndPtrsSz, grpdPtrs, grpdPtrsSz);
#			ifdef NB_CONFIG_INCLUDE_ASSERTS
			{
				timeCur	= NBTimestampMicro_getMonotonicFast();
				usDiff	= NBTimestampMicro_getDiffInUs(&timeLastAction, &timeCur);
				/*if(usDiff >= 1000ULL){
					PRINTF_INFO("NBRtpClient_portNotifyPackets_ took %llu.%llu%llums.\n", (usDiff / 1000ULL), (usDiff % 1000ULL) % 100ULL, (usDiff % 100ULL) % 10ULL);
				}*/
				timeLastAction = timeCur;
			}
#			endif
		}
		//Apply port-stats to thread-stats 
		if(opq->cfg.atomicStats){
			//ToDo: implement
		}
	}
	//error
	if(pollMask & ENNBIOPollsterOpBits_ErrOrGone){
		//ToDo: analyze
	}
	//consume pend tasks
	if(NB_RTP_CLIENT_PORT_HAS_PEND_TASK(p)){
		IF_NBASSERT(STNBTimestampMicro timeLastAction = NBTimestampMicro_getMonotonicFast(), timeCur; SI64 usDiff;)
		NBRtpClient_portConsumePendTasksOpq_(opq, p);
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		{
			timeCur	= NBTimestampMicro_getMonotonicFast();
			usDiff	= NBTimestampMicro_getDiffInUs(&timeLastAction, &timeCur);
			/*if(usDiff >= 1000ULL){
				PRINTF_INFO("NBRtpClient_portConsumePendTasksOpq_ took %llu.%llu%llums.\n", (usDiff / 1000ULL), (usDiff % 1000ULL) % 100ULL, (usDiff % 100ULL) % 10ULL);
			}*/
			timeLastAction = timeCur;
		}
#		endif
	}
	//return
	dstUpd->opsMasks = ENNBIOPollsterOpBit_Read;
	//consume stopFlag
	if(NBStopFlag_isAnyActivated(p->pollster.stopFlag) && NBIOPollsterSync_isSet(dstSync)){
		NBASSERT(p->pollster.isListening)
        NBIOPollsterSync_removeIOLnk(dstSync, &ioLnk);
	}
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	{
		timeCur	= NBTimestampMicro_getMonotonicFast();
		usDiff	= NBTimestampMicro_getDiffInUs(&timeLastAction, &timeCur);
		/*if(usDiff >= 1000ULL){
			PRINTF_INFO("NBRtpClientPort, pollConsumeMask(%s%s%s) took %llu.%llu%llums.\n", (pollMask & ENNBIOPollsterOpBit_Read ? "+read" : ""), (pollMask & ENNBIOPollsterOpBit_Write ? "+write" : ""), (pollMask & ENNBIOPollsterOpBits_ErrOrGone ? "+errOrGone" : ""), (usDiff / 1000ULL), (usDiff % 1000ULL) % 100ULL, (usDiff % 100ULL) % 10ULL);
		}*/
		timeLastAction = timeCur;
	}
#	endif
}

void NBRtpClientPort_pollConsumeNoOp_(STNBIOLnk ioLnk, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData){
	NBRtpClientPort_pollConsumeMask_(ioLnk, 0, dstUpd, dstSync, usrData);
}

void NBRtpClientPort_pollRemoved_(STNBIOLnk ioLnk, void* usrData){
	STNBRtpClientPortOpq* p = (STNBRtpClientPortOpq*)usrData;
	p->pollster.isListening	= FALSE;
	//PRINTF_INFO("NBRtpClientPort, port(%d) safetly-removed from pollster.\n", p->port);
}

//NBRtpClientStream

NB_OBJREF_BODY(NBRtpClientStream, STNBRtpClientStreamOpq, NBObject)

void NBRtpClientStream_initZeroed(STNBObject* obj){
	STNBRtpClientStreamOpq* opq = (STNBRtpClientStreamOpq*)obj;
	//queue
	{
		NBRtpQueue_init(&opq->queue);
	}
	//notify
	{
		NBArray_initWithSz(&opq->notify.hdrs, sizeof(STNBRtpHdrBasic), NULL, 256, 128, 0.10f);
		NBArray_initWithSz(&opq->notify.chunks, sizeof(STNBDataPtr), NULL, 256, 128, 0.10f);
	}
	//stats
	{
		NBRtpClientStatsUpd_init(&opq->stats.upd);
	}
}

void NBRtpClientStream_uninitLocked(STNBObject* obj){
	STNBRtpClientStreamOpq* opq = (STNBRtpClientStreamOpq*)obj;
	//packets to notify
	{
		//headers
		{
			if(opq->notify.hdrs.use > 0){
				NBRtpHdrBasic_hdrsRelease(NBArray_dataPtr(&obj->notify.hdrs, STNBRtpHdrBasic), obj->notify.hdrs.use);
			}
			NBArray_empty(&opq->notify.hdrs);
			NBArray_release(&opq->notify.hdrs);
		}
		//chunks
		{
			if(opq->notify.chunks.use > 0){
				STNBDataPtr* ptrs = NBArray_dataPtr(&opq->notify.chunks, STNBDataPtr);
				NBDataPtr_ptrsReleaseGrouped(ptrs, opq->notify.chunks.use);
			}
			NBArray_empty(&opq->notify.chunks);
			NBArray_release(&opq->notify.chunks);
		}
	}
	//queue
	{
		NBRtpQueue_setUnsafeMode(&opq->queue, FALSE);
		NBRtpQueue_release(&opq->queue); //circular queue
	}
	//Release buffers
	{
		//chunks
		if(opq->buffs.chunks != NULL){
			NBDataPtr_ptrsReleaseGrouped(opq->buffs.chunks, opq->buffs.sz);
			NBMemory_free(opq->buffs.chunks);
			opq->buffs.chunks = NULL;
		}
		//headers
		if(opq->buffs.hdrs != NULL){
			if(opq->buffs.sz > 0){
				NBRtpHdrBasic_hdrsRelease(opq->buffs.hdrs, opq->buffs.sz);
			}
			NBMemory_free(opq->buffs.hdrs);
			opq->buffs.hdrs = NULL;
		}
		opq->buffs.sz		= 0;
	}
	//stats
	{
		if(opq->stats.provider != NULL){
			NBRtpClientStats_release(opq->stats.provider);
			NBMemory_free(opq->stats.provider);
			opq->stats.provider = NULL;
		}
		NBRtpClientStatsUpd_release(&opq->stats.upd);
	}
}

//-----------------------
//- NBRtpClientPort (receiver port)
//-----------------------

NB_OBJREF_BODY(NBRtpClientPort, STNBRtpClientPortOpq, NBObject)

void NBRtpClientPort_initZeroed(STNBObject* obj){
	STNBRtpClientPortOpq* opq = (STNBRtpClientPortOpq*)obj;
    //
    opq->pollster.stopFlag = NBStopFlag_alloc(NULL);
	//socket
	{
		opq->socket = NBSocket_alloc(NULL);
		NBSocket_setType(opq->socket, ENNBSocketHndType_UDP);
		NBSocket_setCorkEnabled(opq->socket, FALSE);
		NBSocket_setDelayEnabled(opq->socket, FALSE);
		NBSocket_setNoSIGPIPE(opq->socket, TRUE);
		NBSocket_setNonBlocking(opq->socket, TRUE);
		NBSocket_setUnsafeMode(opq->socket, TRUE);
	}
	//streams
	{
		NBArraySorted_init(&opq->streams.arr, sizeof(STNBRtpClientStreamRef), NBCompare_NBRtpClientStreamPtrBySsrc);
		//pend
		{
			NBArray_init(&opq->streams.pend.add, sizeof(STNBRtpClientStreamRef), NULL);
			NBArray_init(&opq->streams.pend.rmv, sizeof(STNBRtpClientStreamRef), NULL);
		}
	}
	//buffer
	{
		NBSocketPackets_init(&opq->buffs.pckts);
	}
	//stats
	{
		NBRtpClientStatsUpd_init(&opq->stats.upd);
	}
}

void NBRtpClientPort_uninitLocked(STNBObject* obj){
	STNBRtpClientPortOpq* opq = (STNBRtpClientPortOpq*)obj;
    //
    NBStopFlag_activate(opq->pollster.stopFlag);
	//pollster
	if(NBIOPollster_isSet(opq->pollster.ref)){
		if(opq->pollster.isListening){
			if(NBSocket_isSet(opq->socket)){
				NBIOPollster_removeSocket(opq->pollster.ref, opq->socket);
			}
			opq->pollster.isListening = FALSE;
		}
		NBIOPollster_release(&opq->pollster.ref);
		NBIOPollster_null(&opq->pollster.ref);
	}
	//socket
	if(NBSocket_isSet(opq->socket)){
		NBSocket_release(&opq->socket);
		NBSocket_null(&opq->socket);
	}
	//buffs
	{
		//pool
		if(NBDataPtrsPool_isSet(opq->buffs.pool)){
			NBDataPtrsPool_release(&opq->buffs.pool);
			NBDataPtrsPool_null(&opq->buffs.pool);
		}
		//alloc
		{
			if(opq->buffs.alloc.ptrs != NULL){
				if(opq->buffs.alloc.emptied < opq->buffs.alloc.filled){
					NBDataPtr_ptrsReleaseGrouped(&opq->buffs.alloc.ptrs[opq->buffs.alloc.emptied], opq->buffs.alloc.filled - opq->buffs.alloc.emptied);
				}
				NBMemory_free(opq->buffs.alloc.ptrs);
				opq->buffs.alloc.ptrs = NULL;
			}
			opq->buffs.alloc.emptied = opq->buffs.alloc.filled = opq->buffs.alloc.size = 0;
		}
		//tmp
		{
			//fndPtrs
			if(opq->buffs.tmp.fndPtrs.arr != NULL){
				NBMemory_free(opq->buffs.tmp.fndPtrs.arr);
				opq->buffs.tmp.fndPtrs.arr = NULL;
				opq->buffs.tmp.fndPtrs.sz = 0;
			}
			//grpdPtrs
			if(opq->buffs.tmp.grpdPtrs.arr != NULL){
				NBMemory_free(opq->buffs.tmp.grpdPtrs.arr);
				opq->buffs.tmp.grpdPtrs.arr = NULL;
				opq->buffs.tmp.grpdPtrs.sz = 0;
			}
		}
		//hdrs
		if(opq->buffs.hdrs != NULL){
			if(opq->buffs.pckts.size > 0){
				NBRtpHdrBasic_hdrsRelease(opq->buffs.hdrs, opq->buffs.pckts.size)
			}
			NBMemory_free(opq->buffs.hdrs);
			opq->buffs.hdrs = NULL;
		}
		//packets
		NBSocketPackets_release(&opq->buffs.pckts);
	}
	//streams
	{
		//arr
		{
			SI32 i; for(i = 0; i < opq->streams.arr.use; i++){
				STNBRtpClientStreamRef s = NBArraySorted_itmValueAtIndex(&opq->streams.arr, STNBRtpClientStreamRef, i);
				NBASSERT(((STNBRtpClientStreamOpq*)s.opaque)->port.num == opq->port)
				NBRtpClientStream_release(&s);
			}
			NBArraySorted_empty(&opq->streams.arr);
			NBArraySorted_release(&opq->streams.arr);
		}
		//pend
		{
			//add
			{
				SI32 i; for(i = 0; i < opq->streams.pend.add.use; i++){
					STNBRtpClientStreamRef s = NBArray_itmValueAtIndex(&opq->streams.pend.add, STNBRtpClientStreamRef, i);
					NBASSERT(((STNBRtpClientStreamOpq*)s.opaque)->port.num == opq->port)
					NBRtpClientStream_release(&s);
				}
				NBArray_empty(&opq->streams.pend.add);
				NBArray_release(&opq->streams.pend.add);
			}
			//rmv
			{
				SI32 i; for(i = 0; i < opq->streams.pend.rmv.use; i++){
					STNBRtpClientStreamRef s = NBArray_itmValueAtIndex(&opq->streams.pend.rmv, STNBRtpClientStreamRef, i);
					NBASSERT(((STNBRtpClientStreamOpq*)s.opaque)->port.num == opq->port)
					NBRtpClientStream_release(&s);
				}
				NBArray_empty(&opq->streams.pend.rmv);
				NBArray_release(&opq->streams.pend.rmv);
			}
		}
	}
	//stats
	{
		if(opq->stats.provider != NULL){
			NBRtpClientStats_release(opq->stats.provider);
			NBMemory_free(opq->stats.provider);
			opq->stats.provider = NULL;
		}
		NBRtpClientStatsUpd_release(&opq->stats.upd);
	}
    if(NBStopFlag_isSet(opq->pollster.stopFlag)){
        NBStopFlag_release(&opq->pollster.stopFlag);
        NBStopFlag_null(&opq->pollster.stopFlag);
    }
}

SI32 NBRtpClientPort_rcvMany_(STNBRtpClientPortOpq* p, STNBSocketRef socket, const UI32 packetLostSimDiv){
	SI32 r = 0, rcvd = 0; UI32 useStart = p->buffs.pckts.use;
	NBASSERT(p->buffs.pckts.use < p->buffs.pckts.size)
	IF_NBASSERT(UI32 dbgPcktUseStart = 0;)
	//read socket
	{
		IF_NBASSERT(dbgPcktUseStart = p->buffs.pckts.use;)
		rcvd = NBSocket_recvMany(socket, &p->buffs.pckts);
		//PRINTF_INFO("NBRtpClient, NBSocket_recvMany returned %d.\n", rcvd);
		if(rcvd > 0){
			NBASSERT((dbgPcktUseStart + rcvd) == p->buffs.pckts.use)
			//stats (grp)
			if(p->stats.upd.rcvd.grpsCount == 0 || p->stats.upd.rcvd.minRcvGrp > rcvd){
				p->stats.upd.rcvd.minRcvGrp = rcvd;
			}
			if(p->stats.upd.rcvd.grpsCount == 0 || p->stats.upd.rcvd.maxRcvGrp < rcvd){
				p->stats.upd.rcvd.maxRcvGrp = rcvd;
			}
			p->stats.upd.rcvd.grpsCount++;
			//
			r += rcvd;
			//Simulating packet-lost
			if(packetLostSimDiv != 0 && (rand() % packetLostSimDiv) == 0){
				NBASSERT(p->buffs.pckts.use > 0)
				p->buffs.pckts.use--;
			}
		}
	}
	//calculate packets-wait-time
	if(r > 0){
		//PRINTF_INFO("NBRtpClientPort, port(%d) pollPackets(%d) msWaitRcvd(%dms).\n", p->port, r, msWaitRcvd);
		//parse headers (including arrival times)
		NBASSERT((useStart + r) <= p->buffs.pckts.size)
		{
			UI32 i; for(i = useStart; i < p->buffs.pckts.use; i++){
				STNBRtpHdrBasic* hdrPtr	= &p->buffs.hdrs[i];
				STNBDataPtr* dataPtr	= &p->buffs.pckts.buffs[i];
				NBASSERT(dataPtr->use > 0 && NBDataPtr_allocatedSize(dataPtr) > 0)
				NBASSERT(dataPtr->ptr != NULL && dataPtr->use > 0 && dataPtr->use <= NBDataPtr_allocatedSize(dataPtr))
				if(!NBRtpHdrBasic_parse(hdrPtr, dataPtr->ptr, dataPtr->use)){
					p->stats.upd.rcvd.malform.count++;
					p->stats.upd.rcvd.malform.bytesCount += dataPtr->use;
					hdrPtr->head.ssrc = 0;
				}
			}
		}
	}
	//return
	return r;
}

UI32 NBRtpClientPort_getPtrs_(const UI32 minBytesPerPtr, STNBDataPtr* srcAndDst, const UI32 srcAndDstSz, void* usrData){
	UI32 r = 0, remainToFill, remainFilled, toUse, added = 0;
	STNBRtpClientPortOpq* p = (STNBRtpClientPortOpq*)usrData;
	STNBDataPtr tmp; //, *src, *dst; 
	while(r < srcAndDstSz){
		added = 0;
		//add from local buffer (avoids lock)
		if(added == 0 && p->buffs.alloc.ptrs != NULL){
			if(p->buffs.alloc.emptied > 0 && p->buffs.alloc.emptied == p->buffs.alloc.filled){
				if(NBDataPtrsPool_isSet(p->buffs.pool)){
					const UI32 allocated = NBDataPtrsPool_getPtrs(p->buffs.pool, p->buffs.bytesPerPacket, p->buffs.alloc.ptrs, p->buffs.alloc.emptied);
					//PRINTF_INFO("NBRtpClientPort, allocated %d chunks (%d bytes each).\n", allocated, p->buffs.bytesPerPacket);
					if(allocated < p->buffs.alloc.emptied){
						const UI32 gapSz = (p->buffs.alloc.emptied - allocated); NBASSERT(gapSz > 0 && gapSz < p->buffs.alloc.emptied && gapSz < p->buffs.alloc.size)
						//PRINTF_INFO("NBRtpClientPort, filling gap(%d) in alloc-buffer.\n", gapSz);
						UI32 i; for(i = 0; i < gapSz; i++){
							tmp = p->buffs.alloc.ptrs[i];
							p->buffs.alloc.ptrs[i] = p->buffs.alloc.ptrs[p->buffs.alloc.emptied - 1 - i];
							p->buffs.alloc.ptrs[p->buffs.alloc.emptied - 1 - i] = tmp; 
						}
					}
					p->buffs.alloc.emptied -= allocated;
				}
			}
			//add already allocated ones
			remainToFill	= (srcAndDstSz - r);
			remainFilled	= (p->buffs.alloc.filled - p->buffs.alloc.emptied);
			toUse			= (remainFilled < remainToFill ? remainFilled : remainToFill);
			for(added = 0; added < toUse; added++){
				NBASSERT(p->buffs.alloc.ptrs[p->buffs.alloc.emptied].def.alloc.size == p->buffs.bytesPerPacket)
				tmp						= srcAndDst[r + added];
				srcAndDst[r + added]	= p->buffs.alloc.ptrs[p->buffs.alloc.emptied];
				p->buffs.alloc.ptrs[p->buffs.alloc.emptied] = tmp;
				p->buffs.alloc.emptied++;
			} NBASSERT(p->buffs.alloc.emptied <= p->buffs.alloc.filled)
			r += added;
		}
		//add from remote buffer (uses lock)
		if(added == 0 && NBDataPtrsPool_isSet(p->buffs.pool)){
			r += (added = NBDataPtrsPool_getPtrs(p->buffs.pool, p->buffs.bytesPerPacket, &srcAndDst[r], srcAndDstSz - r));
		}
		//avoid infinite cycle
		if(added == 0) break;
	}
	return r;
}
