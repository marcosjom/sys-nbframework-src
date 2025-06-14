
#ifndef NB_RTPPACKETSQUEUE_H
#define NB_RTPPACKETSQUEUE_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBLog.h"
#include "nb/core/NBDataPtr.h"
#include "nb/core/NBDataPtrsPool.h"
#include "nb/core/NBArray.h"
#include "nb/net/NBRtpHeader.h"

#ifdef __cplusplus
extern "C" {
#endif

    //
    //NBRtpQueue is a queue for ordering and determining packets losts.
    //
    //If the packets arrive in order, this queue has little effect and packets are
    //passed down inmediatly; if a packet is missing or delayed, the queue is
    //filled up to its size leaving gaps for the missing ones. If the missing packet
    //arrives, the sequence of packets is passed down up to the next gap.
    //If the queue is full (including gaps) the missing packet is considered
    //lost and the queue is reduced to the next gap.
    //
    //The size of the queue determines the tolerance for unordered arrival of packets.

    //NBRtpQueueCount32

    typedef struct STNBRtpQueueCount32_ {
        UI32 count;         //packets
        UI32 bytesCount;    //bytes
    } STNBRtpQueueCount32;

    const STNBStructMap* NBRtpQueueCount32_getSharedStructMap(void);

    //NBRtpQueueCount32Seq

    typedef struct STNBRtpQueueCount32Seq_ {
        UI32    count;         //packets
        UI32    bytesCount;    //bytes
        UI32    gapMax;        //max gap late
    } STNBRtpQueueCount32Seq;

    const STNBStructMap* NBRtpQueueCount32Seq_getSharedStructMap(void);

    //NBRtpQueueCount64

    typedef struct STNBRtpQueueCount64_ {
        UI32 count;         //packets
        UI64 bytesCount;    //bytes
    } STNBRtpQueueCount64;

    const STNBStructMap* NBRtpQueueCount64_getSharedStructMap(void);

    //NBRtpQueueCount64Seq

    typedef struct STNBRtpQueueCount64Seq_ {
        UI32    count;
        UI64    bytesCount;
        UI32    gapMax;        //max gap late
    } STNBRtpQueueCount64Seq;

    const STNBStructMap* NBRtpQueueCount64Seq_getSharedStructMap(void);

	//NBRtpPckIdx

	typedef struct STNBRtpPckIdx_ {
		UI32	seqCicle;		//iteration of seqNum (UI16) this packet belongs
		UI16	seqNum;			//16 bits, sequence number (random start)
		UI32	timestamp;		//32 bits
	} STNBRtpPckIdx;

    const STNBStructMap* NBRtpPckIdx_getSharedStructMap(void);

    //Rtp packet queue consumption stats

	typedef struct STNBRtpStatsCsmUpd_ {
		UI32	count;		//packets
		UI32	bytesCount;	//bytes
        STNBRtpQueueCount32 ignored; //ignored, not processed
        STNBRtpQueueCount64 passedThrough; //pass-through (passed directly without using the queue, best scenario posible)
        STNBRtpQueueCount32 queued; //added, populated added to queue
        STNBRtpQueueCount32 unqueued; //removed, populated removed from queue
        STNBRtpQueueCount32 delayed; //filled an existing gap-slot
        STNBRtpQueueCount32 repeated; //repeated (received twice while in queue)
        STNBRtpQueueCount32Seq ignLate; //ingored-late (received after queue moved ahead)
        STNBRtpQueueCount32 lost; //lost (queue moved forward before receiving its payload)
        STNBRtpQueueCount32 overQueue; //overflowed queue, lack of space on rtpQueue (ordering and delaying buffer)
	} STNBRtpStatsCsmUpd;

    const STNBStructMap* NBRtpStatsCsmUpd_getSharedStructMap(void);

	//NBRtpQueueBuffProviderItf

	typedef struct STNBRtpQueueBuffProviderItf_ {
		UI32 (*getPtrs)(const UI32 minBytesPerPtr, STNBDataPtr* srcAndDst, const UI32 srcAndDstSz, void* usrData);
	} STNBRtpQueueBuffProviderItf;

	//

	typedef struct STNBRtpQueue_ {
		void*		opaque;
	} STNBRtpQueue;
	
	//Factory

	void NBRtpQueue_init(STNBRtpQueue* obj);
	void NBRtpQueue_retain(STNBRtpQueue* obj);
	void NBRtpQueue_release(STNBRtpQueue* obj);

	//Config
	BOOL NBRtpQueue_setBuffersProvider(STNBRtpQueue* obj, STNBRtpQueueBuffProviderItf* itf, void* usrData);
	void NBRtpQueue_setSize(STNBRtpQueue* obj, const UI32 bytesPerPacket, const UI32 queueSize, const UI32 ptrsBuffSize);
	void NBRtpQueue_setUnsafeMode(STNBRtpQueue* obj, const BOOL unsafeEnabled);	//when enabled, internal locks are disabled for efficiency but non-parallels calls must be ensured by user

	//Packets
	void NBRtpQueue_reset(STNBRtpQueue* obj);
	BOOL NBRtpQueue_addPacket(STNBRtpQueue* obj, STNBRtpHdrBasic* head, STNBDataPtr* chunk, STNBArray* dstRtpHdrs /*STNBRtpHdrBasic*/, STNBArray* dstChunks /*STNBDataPtr*/, STNBRtpStatsCsmUpd* dstStats);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
