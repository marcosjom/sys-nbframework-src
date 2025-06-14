#ifndef NB_RTP_SERVER_H
#define NB_RTP_SERVER_H

#include "nb/NBFrameworkDefs.h"
//
#include "nb/NBObject.h"
#include "nb/core/NBDataPtr.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBStopFlag.h"
#include "nb/core/NBIOPollster.h"
#include "nb/core/NBIOPollstersProvider.h"
#include "nb/net/NBRtpHeader.h"
#include "nb/net/NBRtpClientStats.h"
#include "nb/net/NBSocket.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct STNBRtpClientSsrcStatsDataStateRequest_ {
    SI32            curMethod;            //ENNBRtspMethod, current requests
    UI64            timeLastAttempt;    //
    UI32            seqErrsCount;        //amount of consecutive errors in actions (zero if last action was sucess)
} STNBRtpClientSsrcStatsDataStateRequest;

typedef struct STNBRtpClientSsrcStatsDataState_ {
    SI32                current;            //ENNBRtspRessState, state current
    SI32                desired;            //ENNBRtspRessState, state desired
    STNBRtpClientSsrcStatsDataStateRequest request;
} STNBRtpClientSsrcStatsDataState;

typedef struct STNBRtpClientSsrcStatsData_ {
	UI16 ssrc;
    STNBRtpClientSsrcStatsDataState state;
	STNBRtpClientStatsData          data;
} STNBRtpClientSsrcStatsData;

const STNBStructMap* NBRtpCfgPort_getSharedStructMap(void);

typedef struct STNBRtpClientPortStatsData_ {
	UI32 port;
	STNBRtpClientStatsData data;
} STNBRtpClientPortStatsData;

//NBRtpCfgPort

typedef struct STNBRtpCfgPort_ {
	UI32		packetsBuffSz;		//amount of packets in socket read buffer
	char*		allocBuffSz;		//string representing a number-bytes and oprional-sufix, like "1", "1K", "1kb", "1M", "1G", "1Tb".
	UI64		allocBuffSzBytes;	//amount of packets in allocation buffer
} STNBRtpCfgPort;

const STNBStructMap* NBRtpCfgPort_getSharedStructMap(void);

//NBRtpCfgStream

typedef struct STNBRtpCfgStream_ {
	UI32		packetsBuffSz;		//amount of packets in buffer
	UI32		orderingQueueSz;	//packets ordering queue size (this will determine wich packets are lost or delayed)
} STNBRtpCfgStream;

const STNBStructMap* NBRtpCfgStream_getSharedStructMap(void);

//NBRtpCfgPackets

typedef struct STNBRtpCfgPackets_ {
	char*		maxSize;		//max size of each packet, string representing a number-bytes and oprional-sufix, like "1", "1K", "1kb", "1M", "1G", "1Tb".
	UI16		maxSizeBytes;	//max size of each packet
	char*		initAlloc;		//initial allocation of packets buffer, string representing a number-bytes and oprional-sufix, like "1", "1K", "1kb", "1M", "1G", "1Tb".
	UI64		initAllocBytes;	//initial allocation of packets buffer
} STNBRtpCfgPackets;

const STNBStructMap* NBRtpCfgPackets_getSharedStructMap(void);

//NBRtpCfgDebug

typedef struct STNBRtpCfgDebug_ {
	UI32		packetLostSimDiv;		//Zero for no-packet-lost simulation; lost = ((rand() % packetLostSimDiv) == 0); 
	BOOL		doNotNotify;			//Usefull for testing packet lost at network recv without payload: rtp packets are parsed, listener identified, packet added to its queue, but packet is not send ahead.
} STNBRtpCfgDebug;

const STNBStructMap* NBRtpCfgDebug_getSharedStructMap(void);

//CfgRtp

typedef struct STNBRtpCfg_ {
	UI32*				ports;		//incoming ports list
	UI32				portsSz;	//incoming ports list-sz
	BOOL				atomicStats; //apply stats to provider after each action (do not wait for explicit statsFlush)
	STNBRtpCfgPackets	packets;	//packets
	STNBRtpCfgPort		perPort;	//perPort config
	STNBRtpCfgStream	perStream;	//perStream config
	STNBRtpCfgDebug		debug;		//debug
} STNBRtpCfg;

const STNBStructMap* NBRtpCfg_getSharedStructMap(void);

//---------------------
//- NBRtpClientLstnr
//---------------------

typedef struct STNBRtpClientLstnr_ {
	UI32			port;
	UI32			ssrc;
	STNBSocketAddr	addr;
	void*			obj;
	BOOL			(*rtpBorderPresent)(void* obj, const STNBRtpHdrBasic* hdrs, const STNBDataPtr* chunks, const UI32 chunksSz);
	void			(*rtpConsume)(void* obj, STNBRtpHdrBasic* hdrs, STNBDataPtr* chunks, const UI32 chunksSz, STNBRtpClientStatsUpd* statsUpd, STNBDataPtrReleaser* optPtrsReleaser);
	void			(*rtpFlushStats)(void* obj);
} STNBRtpClientLstnr;

//---------------
//- NBRtpCmdState
//---------------

typedef struct STNBRtpCmdState_ {
	UI32		seq;
	BOOL		isPend;		//zero means command was completed
} STNBRtpCmdState;

void NBRtpCmdState_init(STNBRtpCmdState* obj);
void NBRtpCmdState_release(STNBRtpCmdState* obj);


//-------------
//- NBRtpClient
//-------------

typedef struct STNBRtpClient_ {
	void* opaque;
} STNBRtpClient;

void NBRtpClient_init(STNBRtpClient* obj);
void NBRtpClient_release(STNBRtpClient* obj);

//Listeners
BOOL NBRtpClient_addStreamLstnr(STNBRtpClient* obj, STNBRtpClientLstnr* listener);
BOOL NBRtpClient_removeStreamLstnr(STNBRtpClient* obj, const UI32 ssrc, const STNBSocketAddr addr);

//Actions
BOOL NBRtpClient_setExternalBuffers(STNBRtpClient* obj, STNBDataPtrsPoolRef pool);
BOOL NBRtpClient_setStatsProvider(STNBRtpClient* obj, STNBRtpClientStats* stats);
BOOL NBRtpClient_setCfg(STNBRtpClient* obj, const STNBRtpCfg* cfg);
BOOL NBRtpClient_setParentStopFlag(STNBRtpClient* obj, STNBStopFlagRef* stopFlag);
BOOL NBRtpClient_setPollster(STNBRtpClient* obj, STNBIOPollsterRef pollster, STNBIOPollsterSyncRef pollSync);	//when one pollster only
BOOL NBRtpClient_setPollstersProvider(STNBRtpClient* obj, STNBIOPollstersProviderRef provider); //when multiple pollsters
BOOL NBRtpClient_prepare(STNBRtpClient* obj);
BOOL NBRtpClient_startListening(STNBRtpClient* obj);
BOOL NBRtpClient_isBusy(STNBRtpClient* obj);
void NBRtpClient_stopFlag(STNBRtpClient* obj);

//Commands
void NBRtpClient_statsFlushStart(STNBRtpClient* obj, STNBRtpCmdState* dst);
BOOL NBRtpClient_statsFlushIsPend(STNBRtpClient* obj, STNBRtpCmdState* dst);
void NBRtpClient_statsGet(STNBRtpClient* obj, STNBArray* dstPortsStats /*STNBRtpClientPortStatsData*/, STNBArray* dstSsrcsStats /*STNBRtpClientSsrcStatsData*/, const BOOL resetAccum);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
