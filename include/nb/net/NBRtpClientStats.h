
#ifndef NB_RTPSERVERSTATS_H
#define NB_RTPSERVERSTATS_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/core/NBLog.h"
#include "nb/core/NBStructMap.h"
#include "nb/net/NBRtpQueue.h"

#ifdef __cplusplus
extern "C" {
#endif


	//Rtp

	typedef struct STNBRtpStatsCfg_ {
		ENNBLogLevel	statsLevel;
	} STNBRtpStatsCfg;

	const STNBStructMap* NBRtpStatsCfg_getSharedStructMap(void);

    //NBRtpClientStatsTime

    typedef struct STNBRtpClientStatsTime_ {
        UI32    count;        //wait count
        UI32    msMin;        //wait between queries (min)
        UI32    msMax;        //wait between queries (max)
        UI32    msTotal;    //wait between queries (waitMsAvg = waitMsTotal / waitCount)
    } STNBRtpClientStatsTime;

    const STNBStructMap* NBRtpClientStatsTime_getSharedStructMap(void);

    //NBRtpClientStatsRcvd

    typedef struct STNBRtpClientStatsRcvd_ {
        UI32    grpsCount;    //populated rcv-calls (each call returns a groups of packets)
        UI32    pcktsCount;    //packets count
        UI64    bytesCount;    //bytes count
        UI32    minBytes;    //min size
        UI32    maxBytes;    //max size
        UI32    minRcvGrp;    //min packets received in one rcv call.
        UI32    maxRcvGrp;    //max packets received in one rcv call.
        STNBRtpQueueCount32 malform; //malform, RTP header parse failure
        STNBRtpQueueCount32 overBuff; //overflowed buffer, lack of space on rtpBuffer (UDP arrival)
    } STNBRtpClientStatsRcvd;

    const STNBStructMap* NBRtpClientStatsRcvd_getSharedStructMap(void);

	//Rtp packet consumption stats

	typedef struct STNBRtpClientStatsUpd_ {
        STNBRtpClientStatsRcvd  rcvd; //rcvd (socket queries wit packets)
		STNBRtpStatsCsmUpd      csmd; //consumed
	} STNBRtpClientStatsUpd;

    const STNBStructMap* NBRtpClientStatsUpd_getSharedStructMap(void);

	void NBRtpClientStatsUpd_init(STNBRtpClientStatsUpd* obj);
	void NBRtpClientStatsUpd_release(STNBRtpClientStatsUpd* obj);
	void NBRtpClientStatsUpd_reset(STNBRtpClientStatsUpd* obj);
	void NBRtpClientStatsUpd_add(STNBRtpClientStatsUpd* obj, const STNBRtpClientStatsUpd* other);

    //NBRtpClientStatsPacketsRcvd

    typedef struct STNBRtpClientStatsPacketsRcvd_ {
        UI32    grpsCount;    //populated rcv-calls (each call returns a groups of packets)
        UI32    pcktsCount;    //packets count
        UI64    bytesCount;    //bytes count
        UI32    minBytes;    //min size of packet
        UI32    maxBytes;    //max size of packet
        UI32    minRcvGrp;    //min packets received in one rcv call.
        UI32    maxRcvGrp;    //max packets received in one rcv call.
        STNBRtpQueueCount32 malform; //malform, RTP header parse failure
        STNBRtpQueueCount32 overBuff; //overflowed buffer, lack of space on rtpBuffer (UDP arrival)
        STNBRtpClientStatsTime wait; //wait (between socket queries with packets)
    } STNBRtpClientStatsPacketsRcvd;

    const STNBStructMap* NBRtpClientStatsPacketsRcvd_getSharedStructMap(void);

    //NBRtpClientStatsPacketsCsmd

    typedef struct STNBRtpClientStatsPacketsCsmd_ {
        UI32                count;
        UI64                bytesCount;
        STNBRtpQueueCount64 ignored; //ignored (not processed by queue)
        STNBRtpQueueCount64 passedThrough; //pass-through (passed directly without using the queue, best scenario posible)
        STNBRtpQueueCount64 queued; //queued
        STNBRtpQueueCount64 unqueued; //queued
        STNBRtpQueueCount64 delayed; //delayed (arrived late but on time)
        STNBRtpQueueCount64 repeated; //repeated (arrived while data already in queue)
        STNBRtpQueueCount64Seq ignLate; //ignored-late (arrived after queue moved ahead)
        STNBRtpQueueCount64 lost; //lost (never arrived)
        STNBRtpQueueCount64 overQueue; //overflowed queue, lack of space on rtpQueue (ordering and delaying buffer)
    } STNBRtpClientStatsPacketsCsmd;

    const STNBStructMap* NBRtpClientStatsPacketsCsmd_getSharedStructMap(void);

    //NBRtpClientStatsPackets

	typedef struct STNBRtpClientStatsPackets_ {
        STNBRtpClientStatsPacketsRcvd rcvd;
        STNBRtpClientStatsPacketsCsmd csmd;
	} STNBRtpClientStatsPackets;

    const STNBStructMap* NBRtpClientStatsPackets_getSharedStructMap(void);

    //NBRtpClientStatsState

	typedef struct STNBRtpClientStatsState_ {
		STNBRtpClientStatsPackets	packets;
		UI64						updCalls;	//metrics calls to update
	} STNBRtpClientStatsState;

    const STNBStructMap* NBRtpClientStatsState_getSharedStructMap(void);

    //NBRtpClientStatsData

	typedef struct STNBRtpClientStatsData_ {
		STNBRtpClientStatsState		loaded;		//loaded
		STNBRtpClientStatsState		accum;		//accum
		STNBRtpClientStatsState		total;		//total
	} STNBRtpClientStatsData;
	
    const STNBStructMap* NBRtpClientStatsData_getSharedStructMap(void);

	//

	typedef struct STNBRtpClientStats_ {
		void*		opaque;
	} STNBRtpClientStats;
	
	//Factory
	void NBRtpClientStats_init(STNBRtpClientStats* obj);
	void NBRtpClientStats_retain(STNBRtpClientStats* obj);
	void NBRtpClientStats_release(STNBRtpClientStats* obj);

	//Data
	STNBRtpClientStatsData NBRtpClientStats_getData(STNBRtpClientStats* obj, const BOOL resetAccum);
	void NBRtpClientStats_concat(STNBRtpClientStats* obj, const STNBRtpStatsCfg* cfg, const BOOL loaded, const BOOL accum, const BOOL total, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, STNBString* dst, const BOOL resetAccum);
	void NBRtpClientStats_concatData(const STNBRtpClientStatsData* obj, const STNBRtpStatsCfg* cfg, const BOOL loaded, const BOOL accum, const BOOL total, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, STNBString* dst);
	void NBRtpClientStats_concatState(const STNBRtpClientStatsState* obj, const STNBRtpStatsCfg* cfg, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, STNBString* dst);
	void NBRtpClientStats_concatPackets(const STNBRtpClientStatsPackets* obj, const STNBRtpStatsCfg* cfg, const BOOL ignoreEmpties, STNBString* dst);

	//Packets
	void NBRtpClientStats_applyUpdate(STNBRtpClientStats* obj, const STNBRtpClientStatsUpd* updResult);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
