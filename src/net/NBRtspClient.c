
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBRtspClient.h"
//
#include "nb/net/NBSdp.h"
#include "nb/net/NBRtpQueue.h"
#include "nb/net/NBRtpClient.h"
#include "nb/net/NBRtcpClient.h"
#include "nb/net/NBSocket.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBNumParser.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBMngrStructMaps.h"
#include "nb/core/NBThread.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBThreadCond.h"
#include "nb/core/NBArraySorted.h"
#include "nb/core/NBDataPtr.h"
#include "nb/core/NBStopFlag.h"
#include "nb/crypto/NBMd5.h"

#define NB_RTSP_CLIENT_START_RTSP_REQ_LOCKED(OPQ, RESS, REQ_ID_STR, RTSP_METHD, TIMEOUTS)	\
	/*PRINTF_INFO("NBRtspClient, '%s' command for: '%s'%s%s.\n", (REQ_ID_STR), (RESS)->uri, (RESS)->setup != NULL ? " session: " : "", (RESS)->setup != NULL ? (RESS)->setup->session.sessionId : "");*/ \
	if(!NBRtspClient_ressStartReqLocked_((OPQ), (RESS), (RTSP_METHD), NULL, 0, TIMEOUTS)){ \
		PRINTF_ERROR("NBRtspClient, could not start '%s' for: '%s'.\n", (REQ_ID_STR), (RESS)->uri); \
	}


//CfgRtp

STNBStructMapsRec STNBRtspClientCfgRtp_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtspClientCfgRtp_getSharedStructMap(void){
	NBMngrStructMaps_lock(&STNBRtspClientCfgRtp_sharedStructMap);
	if(STNBRtspClientCfgRtp_sharedStructMap.map == NULL){
		STNBRtspClientCfgRtp s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtspClientCfgRtp);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addBoolM(map, s, isDisabled);
		NBStructMap_addStructM(map, s, server, NBRtpCfg_getSharedStructMap());
		STNBRtspClientCfgRtp_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&STNBRtspClientCfgRtp_sharedStructMap);
	return STNBRtspClientCfgRtp_sharedStructMap.map;
}

//CfgRtcp

STNBStructMapsRec STNBRtspClientCfgRtcp_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtspClientCfgRtcp_getSharedStructMap(void){
	NBMngrStructMaps_lock(&STNBRtspClientCfgRtcp_sharedStructMap);
	if(STNBRtspClientCfgRtcp_sharedStructMap.map == NULL){
		STNBRtspClientCfgRtcp s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtspClientCfgRtcp);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addBoolM(map, s, isDisabled);	//isDisabled
		NBStructMap_addUIntM(map, s, port);			//incoming port
		STNBRtspClientCfgRtcp_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&STNBRtspClientCfgRtcp_sharedStructMap);
	return STNBRtspClientCfgRtcp_sharedStructMap.map;
}

//CfgRtsp

STNBStructMapsRec STNBRtspClientCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtspClientCfg_getSharedStructMap(void){
	NBMngrStructMaps_lock(&STNBRtspClientCfg_sharedStructMap);
	if(STNBRtspClientCfg_sharedStructMap.map == NULL){
		STNBRtspClientCfg s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtspClientCfg);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addBoolM(map, s, isDisabled);	//isDisabled
		NBStructMap_addStructM(map, s, requests, NBRtspCfgRequests_getSharedStructMap()); //non blocking timeouts
		NBStructMap_addStructM(map, s, rtp, NBRtspClientCfgRtp_getSharedStructMap());
		NBStructMap_addStructM(map, s, rtcp, NBRtspClientCfgRtcp_getSharedStructMap());
		STNBRtspClientCfg_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&STNBRtspClientCfg_sharedStructMap);
	return STNBRtspClientCfg_sharedStructMap.map;
}

//NBRtspClientCfgStreamClientRtp

STNBStructMapsRec STNBRtspClientCfgStreamClientRtp_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtspClientCfgStreamClientRtp_getSharedStructMap(void){
    NBMngrStructMaps_lock(&STNBRtspClientCfgStreamClientRtp_sharedStructMap);
    if(STNBRtspClientCfgStreamClientRtp_sharedStructMap.map == NULL){
        STNBRtspClientCfgStreamClientRtp s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtspClientCfgStreamClientRtp);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, port);
        NBStructMap_addUIntM(map, s, assumeConnDroppedAfterSecs);
        STNBRtspClientCfgStreamClientRtp_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&STNBRtspClientCfgStreamClientRtp_sharedStructMap);
    return STNBRtspClientCfgStreamClientRtp_sharedStructMap.map;
}

//NBRtspClientCfgStreamClientRtcp

STNBStructMapsRec STNBRtspClientCfgStreamClientRtcp_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtspClientCfgStreamClientRtcp_getSharedStructMap(void){
    NBMngrStructMaps_lock(&STNBRtspClientCfgStreamClientRtcp_sharedStructMap);
    if(STNBRtspClientCfgStreamClientRtcp_sharedStructMap.map == NULL){
        STNBRtspClientCfgStreamClientRtcp s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtspClientCfgStreamClientRtcp);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, port);
        STNBRtspClientCfgStreamClientRtcp_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&STNBRtspClientCfgStreamClientRtcp_sharedStructMap);
    return STNBRtspClientCfgStreamClientRtcp_sharedStructMap.map;
}

//NBRtspClientCfgStreamClient

STNBStructMapsRec STNBRtspClientCfgStreamClient_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtspClientCfgStreamClient_getSharedStructMap(void){
    NBMngrStructMaps_lock(&STNBRtspClientCfgStreamClient_sharedStructMap);
    if(STNBRtspClientCfgStreamClient_sharedStructMap.map == NULL){
        STNBRtspClientCfgStreamClient s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtspClientCfgStreamClient);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addStructM(map, s, rtp, NBRtspClientCfgStreamClientRtp_getSharedStructMap());
        NBStructMap_addStructM(map, s, rtcp, NBRtspClientCfgStreamClientRtcp_getSharedStructMap());
        STNBRtspClientCfgStreamClient_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&STNBRtspClientCfgStreamClient_sharedStructMap);
    return STNBRtspClientCfgStreamClient_sharedStructMap.map;
}

//NBRtspClientCfgStream

STNBStructMapsRec STNBRtspClientCfgStream_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtspClientCfgStream_getSharedStructMap(void){
    NBMngrStructMaps_lock(&STNBRtspClientCfgStream_sharedStructMap);
    if(STNBRtspClientCfgStream_sharedStructMap.map == NULL){
        STNBRtspClientCfgStream s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtspClientCfgStream);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addEnumM(map, s, redirMode, ENNBRtspClientConnRedirMode_getSharedEnumMap());
        NBStructMap_addStrPtrM(map, s, server);
        NBStructMap_addUIntM(map, s, port);
        NBStructMap_addBoolM(map, s, useSSL);
        NBStructMap_addStructM(map, s, client, NBRtspClientCfgStreamClient_getSharedStructMap());
        //
        NBStructMap_addPtrToArrayOfCharsM(map, s, user, userSz, ENNBStructMapSign_Unsigned);
        NBStructMap_addPtrToArrayOfCharsM(map, s, pass, passSz, ENNBStructMapSign_Unsigned);
        STNBRtspClientCfgStream_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&STNBRtspClientCfgStream_sharedStructMap);
    return STNBRtspClientCfgStream_sharedStructMap.map;
}

//

struct STNBRtspClientOpq_;

//RtpServer

typedef struct STNBRtspClientRtpSrvrPort_ {
	UI32		port;
	UI32		lstnrsCount;	//local count (to avoid quering/interrupting the rtpServer)
} STNBRtspClientRtpSrvrPort;

typedef struct STNBRtspClientRtpSrvr_ {
	STNBRtpClient				server;
	STNBRtspClientCfgRtp		cfg;
	STNBRtspClientRtpSrvrPort*	ports;		//local count (to avoid quering/interrupting the rtpServer)
	UI32						portsSz;	//local count (to avoid quering/interrupting the rtpServer)
	STNBThreadMutex				mutex;		//local count (to avoid quering/interrupting the rtpServer)
} STNBRtspClientRtpSrvr;

void NBRtspClientRtpSrvr_init(STNBRtspClientRtpSrvr* obj);
void NBRtspClientRtpSrvr_release(STNBRtspClientRtpSrvr* obj);
void NBRtspClientRtpSrvr_setCfg(STNBRtspClientRtpSrvr* obj, const STNBRtspClientCfgRtp* cfg);

//NBRtspClientRes

typedef struct STNBRtspClientRes_ {
	struct STNBRtspClientOpq_* opq;		//opq parent state
	char*					uri;		//uri
	STNBRtspClientCfgStream	cfg;		//config
	//
	STNBRtspClientConnRef	conn;		//conn
	STNBThreadMutex			mutex;		//mutex
	//
	STNBRtspOptions*		options;	//'OPTIONS' method result
	STNBSdpDesc*			desc;		//'DESCRIBE' method result
	STNBRtspSetup*			setup;		//'SETUP' metho result
	//
	STNBRtspClientResLstnr	lstnr;		//listener
	//RTP
	struct {
		STNBRtspClientRtpSrvr* lclSvr;			//rtp local server
		STNBRtspClientRtpSrvrPort* lclPort;		//rtp local port
		UI32				ssrc;				//ssrc in "setup"
		STNBSocketAddr		ssrcAddr;			//ssrc socket address
	} rtp;
	//state
	struct {
		ENNBRtspRessState	current;			//state current
		ENNBRtspRessState	desired;			//state desired
		//start
		struct {
			UI32			iStage;				//ENTSClientRequesterConnRtspStage if RTSP
		} start;
		//request
		struct {
			ENNBRtspMethod	curMethod;			//current requests
			ENNBRtspMethod	lstMethod;			//last request method
			UI64			timeLastAttempt;	//
			UI32			seqErrsCount;		//amount of consecutive errors in actions (zero if last action was sucess)
            BOOL            isTeardownPend;     //a teardown must be completed before continuing
		} request;
		//rtp
		struct {
			UI16			ssrc;				//current ssrc known
			//received
			struct {
				//time (to determine if connection was lost)
				struct {
					UI64	start;	//first packet time
					UI64	last;	//last packet
				} utcTime;
			} packets;
		} rtp;
	} state;
	//dbg
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	struct {
		BOOL		inUnsafeAction;	//currently executing unsafe code
	} dbg;
#	endif
} STNBRtspClientRes;

void NBRtspClientRes_init(STNBRtspClientRes* obj);
void NBRtspClientRes_release(STNBRtspClientRes* obj);
BOOL NBRtspClientRes_isBusy(STNBRtspClientRes* obj);
void NBRtspClientRes_applyStopFlag(STNBRtspClientRes* obj);

//Ssrc index (for quick search of RTP packet owner)
typedef struct STNBRtspClientSsrcIdx_ {
	UI32				ssrc;		//unique-id
	STNBSocketAddr		addr;		//source address
	STNBRtspClientRes*	ress;		//resource
} STNBRtspClientSsrcIdx;

BOOL NBCompare_NBRtspClientSsrcIdx(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);

//Opaque state

typedef struct STNBRtspClientOpq_ {
	STNBThreadMutex		mutex;
	STNBRtspClientCfg	cfg;
    STNBStopFlagRef		stopFlag;
	//pollster
	struct {
		STNBIOPollsterRef		def; //default
		STNBIOPollsterSyncRef	sync; //default
		STNBIOPollstersProviderRef provider;
	} pollster;
	//buffers
	struct {
		STNBDataPtrsPoolRef	pool;
	} buffs;
	//Resources (RTSP targets)
	struct {
		STNBArray		arr;	//STNBRtspClientRes*
		STNBArraySorted	idx;	//STNBRtspClientSsrcIdx
	} ress;
	//RTP-RTCP internal servers (optional)
	struct {
		STNBRtspClientRtpSrvr*	rtp;
		STNBRtcpClient*	rtcp;
	} servers;
	//Stats
	struct {
		STNBRtspClientStats* provider;
	} stats;
	//dbg
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	struct {
		BOOL			inUnsafeAction;	//currently executing unsafe code
	} dbg;
#	endif
	SI32				retainCount;
} STNBRtspClientOpq;

//
BOOL NBRtspClient_ressStartNextRequestLockedOpq_(STNBRtspClientOpq* opq, STNBRtspClientRes* ress, const UI32 onlyIfSecsElapsed);
BOOL NBRtspClient_ressProcessResponseOPTIONSLockedOpq_(STNBRtspClientOpq* opq, STNBRtspClientRes* ress, const STNBSocketAddr* remoteAddr, const STNBHttpResponse* resp);
BOOL NBRtspClient_ressProcessResponseDESCRIBELockedOpq_(STNBRtspClientOpq* opq, STNBRtspClientRes* ress, const STNBSocketAddr* remoteAddr, const STNBHttpResponse* resp);
BOOL NBRtspClient_ressProcessResponseSETUPLockedOpq_(STNBRtspClientOpq* opq, STNBRtspClientRes* ress, const STNBSocketAddr* remoteAddr, const STNBHttpResponse* resp);
BOOL NBRtspClient_ressProcessResponsePLAYLockedOpq_(STNBRtspClientOpq* opq, STNBRtspClientRes* ress, const STNBSocketAddr* remoteAddr, const STNBHttpResponse* resp);
BOOL NBRtspClient_ressProcessResponsePAUSELockedOpq_(STNBRtspClientOpq* opq, STNBRtspClientRes* ress, const STNBSocketAddr* remoteAddr, const STNBHttpResponse* resp);
BOOL NBRtspClient_ressProcessResponseTEARDOWNLockedOpq_(STNBRtspClientOpq* opq, STNBRtspClientRes* ress, const STNBSocketAddr* remoteAddr, const STNBHttpResponse* resp);

//Reference

/*BOOL NBRtspClient_refIsSet(const STNBRtspClient* obj){
	return (obj->opaque != NULL ? TRUE : FALSE);
}*/

/*void NBRtspClient_refSet(STNBRtspClient* obj, const STNBRtspClient* other){
	obj->opaque = other->opaque;
}*/

/*void NBRtspClient_refUnset(STNBRtspClient* obj){
	obj->opaque = NULL;
}*/

//Callback

void NBRtspClient_reqResult_(void* obj, const UI32 uid, const STNBSocketAddr* remoteAddr, const char* method, const char* uri, const ENNBRtspClientConnReqStatus status, const STNBHttpResponse* response);
BOOL NBRtspClient_rtpBorderPresent_(void* obj, const STNBRtpHdrBasic* hdrs, const STNBDataPtr* chunks, const UI32 chunksSz);
void NBRtspClient_rtpConsume_(void* obj, STNBRtpHdrBasic* hdrs, STNBDataPtr* chunks, const UI32 chunksSz, STNBRtpClientStatsUpd* statsUpd, STNBDataPtrReleaser* optPtrsReleaser);
void NBRtspClient_rtpFlushStats_(void* obj);


//Compare

BOOL NBCompare_NBRtspClientSsrcIdx(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	const STNBRtspClientSsrcIdx* d1 = (STNBRtspClientSsrcIdx*)data1;
	const STNBRtspClientSsrcIdx* d2 = (STNBRtspClientSsrcIdx*)data2;
	NBASSERT(dataSz == sizeof(*d1))
	if(dataSz == sizeof(*d1)){
		switch (mode) {
		case ENCompareMode_Equal:
			return d1->ssrc == d2->ssrc && NBCompare_NBSocketAddr_addrOnly(mode, &d1->addr, &d2->addr, sizeof(d1->addr));
		case ENCompareMode_Lower:
			return d1->ssrc < d2->ssrc || (d1->ssrc == d2->ssrc && NBCompare_NBSocketAddr_addrOnly(mode, &d1->addr, &d2->addr, sizeof(d1->addr)));
		case ENCompareMode_LowerOrEqual:
			return d1->ssrc < d2->ssrc || (d1->ssrc == d2->ssrc && NBCompare_NBSocketAddr_addrOnly(mode, &d1->addr, &d2->addr, sizeof(d1->addr)));
		case ENCompareMode_Greater:
			return d1->ssrc > d2->ssrc || (d1->ssrc == d2->ssrc && NBCompare_NBSocketAddr_addrOnly(mode, &d1->addr, &d2->addr, sizeof(d1->addr)));
		case ENCompareMode_GreaterOrEqual:
			return d1->ssrc > d2->ssrc || (d1->ssrc == d2->ssrc && NBCompare_NBSocketAddr_addrOnly(mode, &d1->addr, &d2->addr, sizeof(d1->addr)));
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		default:
			NBASSERT(FALSE)
			break;
#		endif
		}
	}
	NBASSERT(FALSE)
	return FALSE;
}

//----------------
//- NBRtspCmdState
//----------------

void NBRtspCmdState_init(STNBRtspCmdState* obj){
	NBMemory_setZeroSt(*obj, STNBRtspCmdState);
	NBRtpCmdState_init(&obj->rtp);
}

void NBRtspCmdState_release(STNBRtspCmdState* obj){
	NBRtpCmdState_release(&obj->rtp);
}

//Factory

void NBRtspClient_init(STNBRtspClient* obj){
	STNBRtspClientOpq* opq = obj->opaque = NBMemory_allocType(STNBRtspClientOpq);
	NBMemory_setZeroSt(*opq, STNBRtspClientOpq);
	NBThreadMutex_init(&opq->mutex);
    {
        opq->stopFlag = NBStopFlag_alloc(NULL);
    }
	//Resources
	{
		NBArray_init(&opq->ress.arr, sizeof(STNBRtspClientRes*), NULL);
		NBArraySorted_init(&opq->ress.idx, sizeof(STNBRtspClientSsrcIdx), NBCompare_NBRtspClientSsrcIdx);
	}
	//RTP-RTCP internal servers (optional)
	{
		//
	}
	//Reqs
	{
		//
	}
	//
	opq->retainCount	= 1;
}

void NBRtspClient_retain(STNBRtspClient* obj){
	STNBRtspClientOpq* opq = (STNBRtspClientOpq*)obj->opaque;
	NBASSERT(opq != NULL)
	NBThreadMutex_lock(&opq->mutex);
	{
		NBASSERT(opq->retainCount > 0)
		opq->retainCount++;
	}
	NBThreadMutex_unlock(&opq->mutex);
}

void NBRtspClient_release(STNBRtspClient* obj){
	STNBRtspClientOpq* opq = (STNBRtspClientOpq*)obj->opaque;
	NBASSERT(opq != NULL)
	NBThreadMutex_lock(&opq->mutex);
	NBASSERT(opq->retainCount > 0)
	opq->retainCount--;
	if(opq->retainCount > 0){
		NBThreadMutex_unlock(&opq->mutex);
	} else {
		//set flag and stop-all-ress
		{
            NBStopFlag_activate(opq->stopFlag);
		}
		//Reqs
		{
			//
		}
		//Resources
		{
			//arr
			{
				SI32 i; for(i = 0; i < opq->ress.arr.use; i++){
					STNBRtspClientRes* ress = NBArray_itmValueAtIndex(&opq->ress.arr, STNBRtspClientRes*, i);
					if(ress != NULL){
						NBRtspClientRes_release(ress);
						NBMemory_free(ress);
						ress = NULL;
					}
				}
				NBArray_empty(&opq->ress.arr);
				NBArray_release(&opq->ress.arr);
			}
			//idx
			{
				NBArraySorted_empty(&opq->ress.idx);
				NBArraySorted_release(&opq->ress.idx);
			}
		}
		//RTP-RTCP internal servers (optional)
		{
			//Release
			if(opq->servers.rtp != NULL){
				NBRtspClientRtpSrvr_release(opq->servers.rtp);
				NBMemory_free(opq->servers.rtp);
				opq->servers.rtp = NULL;
			}
			if(opq->servers.rtcp != NULL){
				NBRtcpClient_release(opq->servers.rtcp);
				NBMemory_free(opq->servers.rtcp);
				opq->servers.rtcp = NULL;
			}
		}
		//Buffs
		{
			if(NBDataPtrsPool_isSet(opq->buffs.pool)){
				NBDataPtrsPool_release(&opq->buffs.pool);
				NBDataPtrsPool_null(&opq->buffs.pool);
			}
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
		//Stats
		{
			opq->stats.provider = NULL;
		}
        if(NBStopFlag_isSet(opq->stopFlag)){
            NBStopFlag_release(&opq->stopFlag);
            NBStopFlag_null(&opq->stopFlag);
        }
		//Cfg
		{
			NBStruct_stRelease(NBRtspClientCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
		}
		NBThreadMutex_unlock(&opq->mutex);
		NBThreadMutex_release(&opq->mutex);
		NBMemory_free(opq);
		opq = NULL;
	}
}

//Config

BOOL NBRtspClient_setCfg(STNBRtspClient* obj, const STNBRtspClientCfg* cfg){
	BOOL r = FALSE;
	STNBRtspClientOpq* opq = (STNBRtspClientOpq*)obj->opaque;
	NBASSERT(opq != NULL)
	NBThreadMutex_lock(&opq->mutex);
	if(opq->servers.rtp == NULL && opq->servers.rtcp == NULL && opq->ress.arr.use <= 0){
		UI64 maxSizeBytes = 0, initAllocBytes = 0;
		r = TRUE;
		//"maxSizeBytes"
		if(r && cfg != NULL){
			maxSizeBytes = cfg->rtp.server.packets.maxSizeBytes;
			if(!NBString_strIsEmpty(cfg->rtp.server.packets.maxSize)){
				const SI64 bytes = NBString_strToBytes(cfg->rtp.server.packets.maxSize);
				//PRINTF_INFO("NBRtspClient, '%s' parsed to %lld bytes\n", cfg->perBuffer, bytes);
				if(bytes < 0){
					r = FALSE;
				} else {
					maxSizeBytes = (UI32)bytes;
				}
			}
		}
		//"initAllocBytes"
		if(r && cfg != NULL){
			initAllocBytes = cfg->rtp.server.packets.initAllocBytes;
			if(!NBString_strIsEmpty(cfg->rtp.server.packets.initAlloc)){
				const SI64 bytes = NBString_strToBytes(cfg->rtp.server.packets.initAlloc);
				//PRINTF_INFO("NBRtspClient, '%s' parsed to %lld bytes\n", cfg->perBuffer, bytes);
				if(bytes < 0){
					r = FALSE;
				} else {
					initAllocBytes = (UI32)bytes;
				}
			}
		}
		//Validate
		if(r && cfg != NULL){
			//"packetMaxSz"
			if(!cfg->rtp.isDisabled){
				if(maxSizeBytes <= 0 || maxSizeBytes >= 0xFFFFU){ //16 bits max
					PRINTF_ERROR("NBRtspClient, maxSizeBytes must be between 0x1-0xFFFF.\n");
					r = FALSE;
				}
			}
			//nonBlocking
			if(r){
				if(cfg->requests.timeouts.min.msConnect <= 0){
					PRINTF_ERROR("NBRtspClient, timeouts/min/msConnect is required.\n");
					r = FALSE;
				} else if(cfg->requests.timeouts.max.msConnect <= 0){
					PRINTF_ERROR("NBRtspClient, timeouts/max/msConnect is required.\n");
					r = FALSE;
				}
			}
		}
		//Apply
		if(r){
			NBStruct_stRelease(NBRtspClientCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
			if(cfg != NULL){
				NBStruct_stClone(NBRtspClientCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
				//Apply interpreted
				opq->cfg.rtp.server.packets.maxSizeBytes	= (UI16)maxSizeBytes;
				opq->cfg.rtp.server.packets.initAllocBytes	= initAllocBytes;
			}
		}
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

BOOL NBRtspClient_setPollster(STNBRtspClient* obj, STNBIOPollsterRef pollster, STNBIOPollsterSyncRef pollSync){	//when one pollster only
	BOOL r = FALSE;
	STNBRtspClientOpq* opq = (STNBRtspClientOpq*)obj->opaque;
	NBASSERT(opq != NULL)
	NBThreadMutex_lock(&opq->mutex);
	if(opq->ress.arr.use <= 0){
		//set
		NBIOPollster_set(&opq->pollster.def, &pollster);
		NBIOPollsterSync_set(&opq->pollster.sync, &pollSync);
		//
		r = TRUE;
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

BOOL NBRtspClient_setPollstersProvider(STNBRtspClient* obj, STNBIOPollstersProviderRef provider){ //when multiple pollsters
	BOOL r = FALSE;
	STNBRtspClientOpq* opq = (STNBRtspClientOpq*)obj->opaque;
	NBASSERT(opq != NULL)
	NBThreadMutex_lock(&opq->mutex);
	if(opq->ress.arr.use <= 0){
		//set
		NBIOPollstersProvider_set(&opq->pollster.provider, &provider);
		//
		r = TRUE;
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

BOOL NBRtspClient_setStatsProvider(STNBRtspClient* obj, STNBRtspClientStats* stats){
	BOOL r = FALSE;
	STNBRtspClientOpq* opq = (STNBRtspClientOpq*)obj->opaque;
	NBASSERT(opq != NULL)
	NBThreadMutex_lock(&opq->mutex);
	if(opq->ress.arr.use <= 0){
		//Set stats
		opq->stats.provider = stats;
		r = TRUE;
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

BOOL NBRtspClient_setExternalBuffers(STNBRtspClient* obj, STNBDataPtrsPoolRef pool){
	BOOL r = FALSE;
	STNBRtspClientOpq* opq = (STNBRtspClientOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	if(opq->ress.arr.use <= 0){
		//pool
		NBDataPtrsPool_set(&opq->buffs.pool, &pool);
		//
		r = TRUE;
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

//Resources

typedef struct STNBRtspClientGetRessNewParams_ {
	const STNBRtspClientResLstnr* lstnr;
	const STNBRtspClientCfgStream* cfg;
} STNBRtspClientGetRessNewParams;

STNBRtspClientRes* NBRtspClient_getRessLocked_(STNBRtspClientOpq* opq, const char* uri, const STNBRtspClientGetRessNewParams* addIfNecesary){
	STNBRtspClientRes* r = NULL;
	NBASSERT(opq != NULL)
	if(!NBString_strIsEmpty(uri)){
		const SI32 count = opq->ress.arr.use;
		SI32 i; for(i = 0; i < count; i++){
			STNBRtspClientRes* ress = NBArray_itmValueAtIndex(&opq->ress.arr, STNBRtspClientRes*, i);
			//No need to individual-lock ('uri' never changes)
			if(NBString_strIsEqual(ress->uri, uri)){
				r = ress;
				break;
			}
		}
		//Add
		if(r == NULL && addIfNecesary != NULL){
			STNBRtspClientRes* ress = NBMemory_allocType(STNBRtspClientRes);
			NBRtspClientRes_init(ress);
			//Init
			ress->opq	= opq;
			ress->uri	= NBString_strNewBuffer(uri);
			//cfg
			if(addIfNecesary->cfg != NULL){
				const STNBRtspClientCfgStream* cfg = addIfNecesary->cfg;
				ress->cfg.redirMode	= cfg->redirMode;
				NBString_strFreeAndNewBuffer(&ress->cfg.server, cfg->server);
				ress->cfg.port		= cfg->port;
				ress->cfg.useSSL	= cfg->useSSL;
				// 
				ress->cfg.client.rtp.assumeConnDroppedAfterSecs = cfg->client.rtp.assumeConnDroppedAfterSecs; 
				//rtpc
				if(cfg->client.rtcp.port > 0){
					//explicit rtcp port
					ress->cfg.client.rtcp.port = cfg->client.rtcp.port;
				} else {
					//default rtcp port
					ress->cfg.client.rtcp.port = opq->cfg.rtcp.port;
					//PRINTF_INFO("NBRtspClient, auto-rtcp-port(%d) for: %s.\n", opq->cfg.rtcp.port, uri);
				}
				//
				NBString_strFreeAndNewBufferBytes(&ress->cfg.user, cfg->user, cfg->userSz);
				NBString_strFreeAndNewBufferBytes(&ress->cfg.pass, cfg->pass, cfg->passSz);
                ress->cfg.userSz = cfg->userSz;
				ress->cfg.passSz = cfg->passSz;
				//Assign explicit rtpServer
				if(cfg->client.rtp.port > 0 && opq->servers.rtp != NULL){
					STNBRtspClientRtpSrvr* srv = opq->servers.rtp;
					UI32 i2; for(i2 = 0; i2 < srv->portsSz; i2++){
						STNBRtspClientRtpSrvrPort* port = &srv->ports[i2];
						if(port->port == cfg->client.rtp.port){
							ress->cfg.client.rtp.port = port->port;
							ress->rtp.lclSvr	= srv;
							ress->rtp.lclPort	= port;
							{
								NBThreadMutex_lock(&srv->mutex);
								port->lstnrsCount++;
								NBThreadMutex_unlock(&srv->mutex);
							}
							break;
						}
					}
				}
				//Assign the least-busy rtpServer (if necesary)
				if(ress->rtp.lclSvr == NULL && opq->servers.rtp != NULL){
					STNBRtspClientRtpSrvr* lwrSrv = NULL;
					STNBRtspClientRtpSrvrPort* lwrPort = NULL;
					SI32 minLstnrsCount = 0;
					{
						STNBRtspClientRtpSrvr* srv = opq->servers.rtp;
						NBThreadMutex_lock(&srv->mutex);
						{
							UI32 i2; for(i2 = 0; i2 < srv->portsSz; i2++){
								STNBRtspClientRtpSrvrPort* port = &srv->ports[i2];
								if(lwrPort == NULL || minLstnrsCount > port->lstnrsCount){
									lwrSrv = srv;
									lwrPort = port;
									minLstnrsCount = port->lstnrsCount;
								}
							}
						}
						NBThreadMutex_unlock(&srv->mutex);
					}
					NBASSERT((lwrSrv == NULL && lwrPort == NULL && minLstnrsCount == 0) || (lwrSrv != NULL && lwrPort != NULL))
					if(lwrSrv != NULL){
						NBThreadMutex_lock(&lwrSrv->mutex);
						{
							ress->cfg.client.rtp.port	= lwrPort->port;
							ress->rtp.lclSvr			= lwrSrv;
							ress->rtp.lclPort			= lwrPort;
							lwrPort->lstnrsCount++;
							//PRINTF_INFO("NBRtspClient, auto-rtp-port(%d) for: %s.\n", lwrPort->port, uri);
						}
						NBThreadMutex_unlock(&lwrSrv->mutex);
					}
				}
			}
			if(addIfNecesary->lstnr != NULL){
				ress->lstnr = *addIfNecesary->lstnr;
			}
			NBArray_addValue(&opq->ress.arr, ress);
			r = ress;
		}
	}
	return r;
}

BOOL NBRtspClient_addResource(STNBRtspClient* obj, const char* uri, const STNBRtspClientResLstnr* lstnr, const STNBRtspClientCfgStream* cfg, const ENNBRtspRessState state){
	BOOL r = FALSE;
	STNBRtspClientOpq* opq = (STNBRtspClientOpq*)obj->opaque;
	NBASSERT(opq != NULL)
	NBThreadMutex_lock(&opq->mutex);
	if(!NBStopFlag_isMineActivated(opq->stopFlag)){
		STNBRtspClientRes* ress = NULL;
		STNBRtspClientGetRessNewParams paramsIfNew;
		NBMemory_setZeroSt(paramsIfNew, STNBRtspClientGetRessNewParams);
		paramsIfNew.lstnr	= lstnr;
		paramsIfNew.cfg		= cfg;
		ress = NBRtspClient_getRessLocked_(opq, uri, &paramsIfNew);
		if(ress != NULL){
			ress->state.desired = state;
			r = TRUE;
		}
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

BOOL NBRtspClient_removeResource(STNBRtspClient* obj, const char* uri){
	BOOL r = FALSE;
	STNBRtspClientOpq* opq = (STNBRtspClientOpq*)obj->opaque;
	NBASSERT(opq != NULL)
	NBThreadMutex_lock(&opq->mutex);
	if(!NBStopFlag_isMineActivated(opq->stopFlag)){
		STNBRtspClientRes* ress = NBRtspClient_getRessLocked_(opq, uri, NULL);
		if(ress != NULL){
			ress->state.desired = ENNBRtspRessState_Stopped;
		}
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

BOOL NBRtspClient_setResourceState(STNBRtspClient* obj, const char* uri, const ENNBRtspRessState state){
	BOOL r = FALSE;
	STNBRtspClientOpq* opq = (STNBRtspClientOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	if(!NBStopFlag_isMineActivated(opq->stopFlag)){
		const SI32 count = opq->ress.arr.use;
		SI32 i; for(i = 0; i < count; i++){
			STNBRtspClientRes* ress = NBArray_itmValueAtIndex(&opq->ress.arr, STNBRtspClientRes*, i);
			//No need to individual-lock ('uri' never changes)
			if(NBString_strIsEqual(ress->uri, uri)){
				ress->state.desired = state;
				r = TRUE;
				break;
			}
		}
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

BOOL NBRtspClient_getResourceCfg(STNBRtspClient* obj, const char* uri, STNBRtspClientCfgStream* dst){
    BOOL r = FALSE;
    STNBRtspClientOpq* opq = (STNBRtspClientOpq*)obj->opaque;
    NBThreadMutex_lock(&opq->mutex);
    if(!NBStopFlag_isMineActivated(opq->stopFlag)){
        const SI32 count = opq->ress.arr.use;
        SI32 i; for(i = 0; i < count; i++){
            STNBRtspClientRes* ress = NBArray_itmValueAtIndex(&opq->ress.arr, STNBRtspClientRes*, i);
            //No need to individual-lock ('uri' never changes)
            if(NBString_strIsEqual(ress->uri, uri)){
                if(dst != NULL){
                    const STNBStructMap* map = NBRtspClientCfgStream_getSharedStructMap();
                    NBStruct_stRelease(map, dst, sizeof(*dst));
                    NBStruct_stClone(map, &ress->cfg, sizeof(ress->cfg), dst, sizeof(*dst));
                }
                r = TRUE;
                break;
            }
        }
    }
    NBThreadMutex_unlock(&opq->mutex);
    return r;
}

//Actions
	
BOOL NBRtspClient_prepare(STNBRtspClient* obj){ //create buffers, internal servers and more
	BOOL r = FALSE;
	STNBRtspClientOpq* opq = (STNBRtspClientOpq*)obj->opaque;
	NBASSERT(opq != NULL)
	NBThreadMutex_lock(&opq->mutex);
	if(!NBStopFlag_isMineActivated(opq->stopFlag) && opq->servers.rtp == NULL && opq->servers.rtcp == NULL && opq->ress.arr.use <= 0 && (NBIOPollster_isSet(opq->pollster.def) || NBIOPollsterSync_isSet(opq->pollster.sync) || NBIOPollstersProvider_isSet(opq->pollster.provider))){
		STNBRtspClientRtpSrvr* srvRtp = NULL;
		STNBRtcpClient* srvRtcp = NULL;
		r = TRUE;
		//RTP server
		if(r){
			const STNBRtspClientCfgRtp* cfg = &opq->cfg.rtp;
			if(!cfg->isDisabled && cfg->server.portsSz > 0){
				STNBRtspClientRtpSrvr* srv = NBMemory_allocType(STNBRtspClientRtpSrvr);
				NBRtspClientRtpSrvr_init(srv);
				NBRtspClientRtpSrvr_setCfg(srv, cfg);
				if(!NBRtpClient_setExternalBuffers(&srv->server, opq->buffs.pool)){
					r = FALSE;
				} else if(!NBRtpClient_setStatsProvider(&srv->server, (opq->stats.provider != NULL ? NBRtspClientStats_getRtpStats(opq->stats.provider) : NULL))){
					r = FALSE;
				} else if(!NBRtpClient_setCfg(&srv->server, &cfg->server)){
					r = FALSE;
                } else if(!NBRtpClient_setParentStopFlag(&srv->server, &opq->stopFlag)){
                    r = FALSE;
				} else if(!NBRtpClient_setPollster(&srv->server, opq->pollster.def, opq->pollster.sync)){
					r = FALSE;
				} else if(!NBRtpClient_setPollstersProvider(&srv->server, opq->pollster.provider)){
					r = FALSE;	
				} else if(!NBRtpClient_prepare(&srv->server)){
					r = FALSE;
				} else {
					srvRtp = srv;
					srv = NULL;
				}
				//Release (if not consumed)
				if(srv != NULL){
					NBRtspClientRtpSrvr_release(srv);
					NBMemory_free(srv);
					srv = NULL;
				}
			}
		}
		//RTCP server
		if(r && !opq->cfg.rtcp.isDisabled){
			const STNBRtspClientCfgRtcp* cfg = &opq->cfg.rtcp; 
			if(cfg->port <= 0){
				r = FALSE;
			} else {
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
					r = FALSE;
				} else {
					srvRtcp = NBMemory_allocType(STNBRtcpClient);
					NBRtcpClient_init(srvRtcp);
                    if(!NBRtcpClient_setParentStopFlag(srvRtcp, &opq->stopFlag)){
                        r = FALSE;
                    } else if(!NBRtcpClient_setPollster(srvRtcp, pollster, NB_OBJREF_NULL)){
						r = FALSE;
					} else if(!NBRtcpClient_bind(srvRtcp, cfg->port)){
						r = FALSE;
					}
				}
			}
		}
		//Apply (if sucess)
		if(r){
			//RTP
			{
				
				if(opq->servers.rtp != NULL){
					NBRtspClientRtpSrvr_release(opq->servers.rtp);
					NBMemory_free(opq->servers.rtp);
					opq->servers.rtp = NULL;
				}
				opq->servers.rtp = srvRtp;
				srvRtp = NULL;
			}
			//RTCP
			{
				if(opq->servers.rtcp != NULL){
					NBRtcpClient_release(opq->servers.rtcp);
					NBMemory_free(opq->servers.rtcp);
					opq->servers.rtcp = NULL;
				}
				opq->servers.rtcp = srvRtcp;
				srvRtcp = NULL; //consume
			}
		}
		//Release (if not consumed)
		if(srvRtp != NULL){
			NBRtspClientRtpSrvr_release(srvRtp);
			NBMemory_free(srvRtp);
			srvRtp = NULL;
		}
		if(srvRtcp != NULL){
			NBRtcpClient_release(srvRtcp);
			NBMemory_free(srvRtcp);
			srvRtcp = NULL;
		}
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

BOOL NBRtspClient_start(STNBRtspClient* obj){	//threads and binded sockets
	BOOL r = FALSE;
	STNBRtspClientOpq* opq = (STNBRtspClientOpq*)obj->opaque;
	NBASSERT(opq != NULL)
	NBThreadMutex_lock(&opq->mutex);
	if(!NBStopFlag_isMineActivated(opq->stopFlag) && opq->ress.arr.use <= 0){
		r = TRUE;
		if(r && opq->servers.rtcp != NULL){
			if(!NBRtcpClient_startListening(opq->servers.rtcp)){
				r = FALSE;
			} else if(!NBRtpClient_startListening(&opq->servers.rtp->server)){
				r = FALSE;
			}
		}
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

void NBRtspClient_stopFlag(STNBRtspClient* obj){
	STNBRtspClientOpq* opq = (STNBRtspClientOpq*)obj->opaque;
    NBStopFlag_activate(opq->stopFlag);
}

BOOL NBRtspClient_isBusy(STNBRtspClient* obj){	//stream not cleaned-up
	BOOL r = FALSE;
	STNBRtspClientOpq* opq = (STNBRtspClientOpq*)obj->opaque; 
	NBASSERT(opq != NULL)
	NBThreadMutex_lock(&opq->mutex);
	NBASSERT(opq->retainCount > 0)
	{
		//ress
		if(!r){
			SI32 i = 0; for(i = (SI32)opq->ress.arr.use - 1; i >= 0 && !r; i--){
				STNBRtspClientRes* ress = NBArray_itmValueAtIndex(&opq->ress.arr, STNBRtspClientRes*, i);
				if(ress != NULL){
					NBThreadMutex_lock(&ress->mutex);
					{
						if(NBRtspClientRes_isBusy(ress)){
							//is bussy
							r = TRUE;
						} else {
							//apply stop flag
							if(NBStopFlag_isAnyActivated(opq->stopFlag)){
								NBRtspClientRes_applyStopFlag(ress);
							}
							//is bussy?
							if(NBRtspClientConn_isSet(ress->conn) && NBRtspClientConn_isBusy(ress->conn)){
								r = TRUE;
							}
						}
					}
					NBThreadMutex_unlock(&ress->mutex);
				}
			}
		}
		//rtcp
		if(!r && opq->servers.rtcp != NULL){
			if(NBRtcpClient_isBusy(opq->servers.rtcp)){
				//is bussy
				r = TRUE;
			}
		}
		//rtp
		if(r && opq->servers.rtp != NULL){
			if(NBRtpClient_isBusy(&opq->servers.rtp->server)){
				//is bussy
				r = TRUE;
			}
		}
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

BOOL NBRtspClient_ressStartReqLocked_(STNBRtspClientOpq* opq, STNBRtspClientRes* ress, const ENNBRtspMethod method, const STNBRtspClientConnReqParam* params, const UI32 paramsSz, const STNBRtspCfgTimeouts* timeouts){
	BOOL r = FALSE;
	NBASSERT(ress->state.request.curMethod == ENNBRtspMethod_Count)
	if(ress->state.request.curMethod == ENNBRtspMethod_Count){
		//start request
		const STNBRtspMethodDef* methodDef = NBRtsp_getMethodDefById(method);
		UI32 rtpPort = 0, rtcpPort = 0;
		STNBString sessionId;
		NBString_init(&sessionId);
		//set current 
		ress->state.request.lstMethod = ress->state.request.curMethod = method;
		//Analyze ress
		{
			//Create connection
			if(!NBRtspClientConn_isSet(ress->conn)){
				STNBIOPollsterSyncRef pollSync = NB_OBJREF_NULL;
				//get a pollster from provider
				if(NBIOPollstersProvider_isSet(opq->pollster.provider)){
					pollSync = NBIOPollstersProvider_getPollsterSync(opq->pollster.provider); 
				}
				//use default pollster
				if(!NBIOPollsterSync_isSet(pollSync)){
					pollSync = opq->pollster.sync;
				}
				//result
				if(!NBIOPollsterSync_isSet(pollSync)){
					PRINTF_ERROR("NBRtspClient, could not get a pollster-sync for connection.\n");
				} else {
					//Create conn for this resource
					STNBRtspClientConnRef conn = NBRtspClientConn_alloc(NULL);
					NBRtspClientConn_setServer(conn, ress->cfg.server, (UI16)ress->cfg.port, ress->cfg.useSSL, ress->cfg.redirMode);
					NBRtspClientConn_setUserAndPass(conn, ress->cfg.user, ress->cfg.userSz, ress->cfg.pass, ress->cfg.passSz);
					NBRtspClientConn_startListening(conn, pollSync);
					{
						STNBRtspClientConnCallback callback;
						NBMemory_setZeroSt(callback, STNBRtspClientConnCallback);
						callback.obj		= opq;
						callback.reqResult	= NBRtspClient_reqResult_;
						NBRtspClientConn_setCallback(conn, &callback);
					}
					//set
					ress->conn = conn;
				}
			}
			//load params
			{
				rtpPort     = ress->cfg.client.rtp.port;
				rtcpPort    = ress->cfg.client.rtcp.port;
				if(ress->setup != NULL){
					if(!NBString_strIsEmpty(ress->setup->session.sessionId)){
						NBString_set(&sessionId, ress->setup->session.sessionId);
					}
				}
			}
		}
		if(NBRtspClientConn_isSet(ress->conn)){
			NBThreadMutex_unlock(&ress->mutex);
			//Execute (unlocked)
			{
				//PRINTF_INFO("NBRtspClient_ressStartReqLocked_, %d request active.\n", opq->reqs.activeCount);
				NBThreadMutex_unlock(&opq->mutex);
				if(NBRtspClientConn_startReq(ress->conn, methodDef->method, ress->uri, sessionId.str, rtpPort, rtcpPort, params, paramsSz, timeouts)){
					r = TRUE; 
				}
				NBThreadMutex_lock(&opq->mutex);
			}
			NBThreadMutex_lock(&ress->mutex);
		}
		//unset current (if failed)
		if(!r){
			ress->state.request.curMethod = ENNBRtspMethod_Count;
			ress->state.request.seqErrsCount++;
		}
		NBString_release(&sessionId);
	}
	return r;
}

//Commands

void NBRtspClient_statsFlushStart(STNBRtspClient* obj, STNBRtspCmdState* dst){
	STNBRtspClientOpq* opq = (STNBRtspClientOpq*)obj->opaque;
	NBASSERT(opq != NULL)
	NBThreadMutex_lock(&opq->mutex);
	if(!NBStopFlag_isMineActivated(opq->stopFlag) && opq->servers.rtp != NULL){
		STNBRtspClientRtpSrvr* srv = opq->servers.rtp;
		NBRtpClient_statsFlushStart(&srv->server, (dst != NULL ? &dst->rtp : NULL));
		//result
		if(dst != NULL){
			dst->isPend = (dst->rtp.isPend);
		}
	} else {
		if(dst != NULL){
			dst->isPend = FALSE;
		}
	}
	NBThreadMutex_unlock(&opq->mutex);
}

BOOL NBRtspClient_statsFlushIsPend(STNBRtspClient* obj, STNBRtspCmdState* dst){
	BOOL r = FALSE;
	STNBRtspClientOpq* opq = (STNBRtspClientOpq*)obj->opaque;
	NBASSERT(opq != NULL)
	if(dst != NULL){
		NBThreadMutex_lock(&opq->mutex);
		{
			if(opq->servers.rtp == NULL){
				dst->rtp.isPend = FALSE;
			} else {
				STNBRtspClientRtpSrvr* srv = opq->servers.rtp;
				NBRtpClient_statsFlushIsPend(&srv->server, &dst->rtp);
			}
			//result
			r = dst->isPend = (dst->rtp.isPend);
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}

void NBRtspClient_statsGet(STNBRtspClient* obj, STNBArray* dstPortsStats /*STNBRtpClientPortStatsData*/, STNBArray* dstSsrcsStats /*STNBRtpClientSsrcStatsData*/, const BOOL resetAccum){
	STNBRtspClientOpq* opq = (STNBRtspClientOpq*)obj->opaque;
	NBASSERT(opq != NULL)
	NBThreadMutex_lock(&opq->mutex);
    //get RTP-stats
	if(opq->servers.rtp != NULL){
		STNBRtspClientRtpSrvr* srv = opq->servers.rtp;
		NBRtpClient_statsGet(&srv->server, dstPortsStats, dstSsrcsStats, resetAccum);
	}
    //complement records or add missing records
    {
        SI32 i = 0; for(i = (SI32)opq->ress.arr.use - 1; i >= 0; i--){
            STNBRtspClientRes* ress = NBArray_itmValueAtIndex(&opq->ress.arr, STNBRtspClientRes*, i);
            if(ress != NULL){
                NBThreadMutex_lock(&ress->mutex);
                {
                    STNBRtpClientSsrcStatsData* fnd = NULL;
                    //Find current record
                    if(ress->state.rtp.ssrc > 0){
                        SI32 i; for(i = 0; i < dstSsrcsStats->use; i++){
                            STNBRtpClientSsrcStatsData* d = NBArray_itmPtrAtIndex(dstSsrcsStats, STNBRtpClientSsrcStatsData, i);
                            if(d->ssrc == ress->state.rtp.ssrc){
                                fnd = d;
                                break;
                            }
                        }
                    }
                    //Add record (if necesary)
                    if(fnd == NULL){
                        STNBRtpClientSsrcStatsData d;
                        NBMemory_setZeroSt(d, STNBRtpClientSsrcStatsData);
                        d.ssrc    = ress->state.rtp.ssrc;
                        fnd = NBArray_addValue(dstSsrcsStats, d);
                    }
                    //Apply values
                    if(fnd != NULL){
                        fnd->state.current = ress->state.current;
                        fnd->state.desired = ress->state.desired;
                        //
                        fnd->state.request.curMethod = ress->state.request.curMethod;
                        fnd->state.request.timeLastAttempt = ress->state.request.timeLastAttempt;
                        fnd->state.request.seqErrsCount = ress->state.request.seqErrsCount;
                    }
                }
                NBThreadMutex_unlock(&ress->mutex);
            }
        }
    }
	NBThreadMutex_unlock(&opq->mutex);
}

//RtpServer

void NBRtspClientRtpSrvr_init(STNBRtspClientRtpSrvr* obj){
	NBMemory_setZeroSt(*obj, STNBRtspClientRtpSrvr);
	NBRtpClient_init(&obj->server);
	NBThreadMutex_init(&obj->mutex); //local count (to avoid quering/interrupting the rtpServer)
}

void NBRtspClientRtpSrvr_release(STNBRtspClientRtpSrvr* obj){
	NBRtpClient_release(&obj->server);
	NBStruct_stRelease(NBRtspClientCfgRtp_getSharedStructMap(), &obj->cfg, sizeof(obj->cfg));
	//ports
	NBThreadMutex_lock(&obj->mutex); //local count (to avoid quering/interrupting the rtpServer)
	if(obj->ports != NULL){
		UI32 i; for(i = 0; i < obj->portsSz; i++){
			IF_NBASSERT(STNBRtspClientRtpSrvrPort* port = &obj->ports[i];)
			NBASSERT(port->lstnrsCount == 0) //no listeners must be active
		}
		NBMemory_free(obj->ports);
		obj->ports		= NULL;
		obj->portsSz	= 0;
	}
	NBThreadMutex_unlock(&obj->mutex); //local count (to avoid quering/interrupting the rtpServer)
	NBThreadMutex_release(&obj->mutex); //local count (to avoid quering/interrupting the rtpServer)
}

void NBRtspClientRtpSrvr_setCfg(STNBRtspClientRtpSrvr* obj, const STNBRtspClientCfgRtp* cfg){
	NBThreadMutex_lock(&obj->mutex); //local count (to avoid quering/interrupting the rtpServer)
	//ports
	{
		//release current
		if(obj->ports != NULL){ 
			NBMemory_free(obj->ports);
			obj->ports		= NULL;
			obj->portsSz	= 0;
		}
		//create array
		if(cfg != NULL && cfg->server.portsSz > 0){
			obj->portsSz = cfg->server.portsSz; 
			obj->ports = NBMemory_allocTypes(STNBRtspClientRtpSrvrPort, obj->portsSz);
			{
				UI32 i; for(i = 0; i < obj->portsSz; i++){
					STNBRtspClientRtpSrvrPort* port = &obj->ports[i]; 
					NBMemory_setZeroSt(*port, STNBRtspClientRtpSrvrPort);
					port->port = cfg->server.ports[i];
				}
			}
		}
	}
	//cfg
	{
		const STNBStructMap* srvRtpMap = NBRtspClientCfgRtp_getSharedStructMap();
		NBStruct_stRelease(srvRtpMap, &obj->cfg, sizeof(obj->cfg));
		if(cfg != NULL){
			NBStruct_stClone(srvRtpMap, cfg, sizeof(*cfg), &obj->cfg, sizeof(obj->cfg));
		}
	}
	NBThreadMutex_unlock(&obj->mutex); //local count (to avoid quering/interrupting the rtpServer)
}

//Resource

void NBRtspClientRes_init(STNBRtspClientRes* obj){
	NBMemory_setZeroSt(*obj, STNBRtspClientRes);
	NBThreadMutex_init(&obj->mutex);
	//state
	{
		//request
		{
			obj->state.request.curMethod = ENNBRtspMethod_Count;
			obj->state.request.lstMethod = ENNBRtspMethod_Count;
		}
	}
}

void NBRtspClientRes_release(STNBRtspClientRes* obj){
	NBThreadMutex_lock(&obj->mutex);
	{
		//should be already removed
		/*ToDo: remove
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		if(obj->options != NULL){
			PRINTF_ERROR("NBRtspClientRes, 'options' is not null at release, for '%s'.\n", obj->uri);
		}
		if(obj->desc != NULL){
			PRINTF_ERROR("NBRtspClientRes, 'desc' is not null at release, for '%s'.\n", obj->uri);
		}
		if(obj->setup != NULL){
			PRINTF_ERROR("NBRtspClientRes, 'setup' is not null at release, for '%s'.\n", obj->uri);
		}
		if(obj->state.current != ENNBRtspRessState_Stopped){
			PRINTF_ERROR("NBRtspClientRes, 'state' is not 'stopped' at release, for '%s'.\n", obj->uri);
		}
		if(obj->state.request.curMethod != ENNBRtspMethod_Count){
			PRINTF_ERROR("NBRtspClientRes, is requesting at release, for '%s'.\n", obj->uri);
		}
#		endif*/
		NBASSERT(obj->options == NULL && obj->desc == NULL && obj->setup == NULL && obj->state.current == ENNBRtspRessState_Stopped && obj->state.request.curMethod == ENNBRtspMethod_Count)
		//Remove from listener
		if(obj->opq != NULL){
			//Decrease listener count
			if(obj->rtp.lclSvr != NULL){
				NBThreadMutex_lock(&obj->rtp.lclSvr->mutex);
				{
					NBASSERT(obj->rtp.lclPort->lstnrsCount > 0)
					obj->rtp.lclPort->lstnrsCount--;
				}
				NBThreadMutex_unlock(&obj->rtp.lclSvr->mutex);
			}
			//Remove from rtpServer
			if(obj->rtp.ssrc != 0){
				//RtpServer
				if(!NBRtpClient_removeStreamLstnr(&obj->rtp.lclSvr->server, obj->rtp.ssrc, obj->rtp.ssrcAddr)){
					NBASSERT(FALSE); //Program logic error
				}
				//Internal idx
				{
					STNBRtspClientSsrcIdx srch;
					srch.ssrc	= obj->rtp.ssrc;
					srch.addr	= obj->rtp.ssrcAddr;
					{
						const SI32 iFnd = NBArraySorted_indexOf(&obj->opq->ress.idx, &srch, sizeof(srch), NULL);
						if(iFnd >= 0){
							NBArraySorted_removeItemAtIndex(&obj->opq->ress.idx, iFnd);
						}
					}
				}
				//Set
				obj->rtp.ssrc = 0;
				NBMemory_setZeroSt(obj->rtp.ssrcAddr, STNBSocketAddr);
			}
		}
		//Conn
		if(NBRtspClientConn_isSet(obj->conn)){
#			ifdef NB_CONFIG_INCLUDE_ASSERTS
			if(NBRtspClientConn_isBusy(obj->conn)){
				PRINTF_ERROR("NBRtspClientRes, realeasing conn while still listening to: '%s' (use 'NBRtspClient_stopFlag()' and wait for '!NBRtspClient_isBusy()' before calling 'NBRtspClient_release()').\n", obj->uri);
				NBASSERT(FALSE)
			}
#			endif
			NBRtspClientConn_release(&obj->conn);
			NBRtspClientConn_null(&obj->conn);
		}
		//Options
		if(obj->options != NULL){
			NBMemory_free(obj->options);
			obj->options = NULL;
		}
		//Desc
		if(obj->desc != NULL){
			NBSdpDesc_release(obj->desc);
			NBMemory_free(obj->desc);
			obj->desc = NULL;
		}
		//Setup
		if(obj->setup != NULL){
			NBRtspSetup_release(obj->setup);
			NBMemory_free(obj->setup);
		}
		//cfg
		{
			if(obj->cfg.server != NULL){
				NBMemory_free(obj->cfg.server);
				obj->cfg.server = NULL;
			}
			if(obj->cfg.user != NULL){
				NBMemory_free(obj->cfg.user);
				obj->cfg.user	= NULL;
				obj->cfg.userSz	= 0;
			}
			if(obj->cfg.pass != NULL){
				NBMemory_free(obj->cfg.pass);
				obj->cfg.pass	= NULL;
				obj->cfg.passSz = 0;
			}
		}
		//uri
		if(obj->uri != NULL){
			NBMemory_free(obj->uri);
			obj->uri = NULL;
		}
	}
	NBThreadMutex_unlock(&obj->mutex);
	NBThreadMutex_release(&obj->mutex);
}

BOOL NBRtspClientRes_isBusy(STNBRtspClientRes* obj){
	return (obj->state.current != ENNBRtspRessState_Stopped || obj->state.request.curMethod != ENNBRtspMethod_Count);	
}

void NBRtspClientRes_applyStopFlag(STNBRtspClientRes* obj){
	if(NBRtspClientConn_isSet(obj->conn)){
		if(NBRtspClientConn_isBusy(obj->conn)){
			//stop signal
			NBRtspClientConn_stopListening(obj->conn);
		} else {
			//conn can be released
			//PRINTF_INFO("NBRtspClientRes, safetly-cleaning conn to: '%s'.\n", obj->uri);
			NBRtspClientConn_release(&obj->conn);
			NBRtspClientConn_null(&obj->conn);
		}
	}
}

//Ress notify state

typedef struct STNBNBRtspClientRessNotifyState_ {
	const char*				uri;
	STNBRtspClientResLstnr	lstnr;
	//before
	struct {
		STNBRtspOptions*	options;
		STNBSdpDesc*		desc;
		STNBRtspSetup*		setup;
		UI16				ssrcId;
		ENNBRtspRessState	state;
	} before;
	//after
	struct {
		STNBRtspOptions*	options;
		STNBSdpDesc*		desc;
		STNBRtspSetup*		setup;
		UI16				ssrcId;
		ENNBRtspRessState	state;
	} after;
	//ToNotify
	struct {
		BOOL		options;
		BOOL		desc;
		BOOL		setup;
		BOOL		state;
	} notify;
} STNBNBRtspClientRessNotifyState;

void NBNBRtspClientRessNotifyState_init(STNBNBRtspClientRessNotifyState* obj);
void NBNBRtspClientRessNotifyState_release(STNBNBRtspClientRessNotifyState* obj);
void NBNBRtspClientRessNotifyState_setInitialState(STNBNBRtspClientRessNotifyState* obj, STNBRtspClientRes* ress);
BOOL NBNBRtspClientRessNotifyState_setFinalState(STNBNBRtspClientRessNotifyState* obj, STNBRtspClientRes* ress);
void NBNBRtspClientRessNotifyState_notify(STNBNBRtspClientRessNotifyState* obj, STNBRtspClientRes* ress);

void NBNBRtspClientRessNotifyState_init(STNBNBRtspClientRessNotifyState* obj){
	NBMemory_setZeroSt(*obj, STNBNBRtspClientRessNotifyState);
}

void NBNBRtspClientRessNotifyState_release(STNBNBRtspClientRessNotifyState* obj){
	NBMemory_setZeroSt(*obj, STNBNBRtspClientRessNotifyState);
}

void NBNBRtspClientRessNotifyState_setInitialState(STNBNBRtspClientRessNotifyState* obj, STNBRtspClientRes* ress){
	obj->before.options = ress->options;
	obj->before.desc	= ress->desc;
	obj->before.setup	= ress->setup;
	obj->before.ssrcId	= (UI16)ress->rtp.ssrc;
	obj->before.state	= ress->state.current;
}

BOOL NBNBRtspClientRessNotifyState_setFinalState(STNBNBRtspClientRessNotifyState* obj, STNBRtspClientRes* ress){
	obj->uri			= ress->uri;
	obj->lstnr			= ress->lstnr;
	//
	obj->after.options	= ress->options;
	obj->after.desc		= ress->desc;
	obj->after.setup	= ress->setup;
	obj->after.ssrcId	= (UI16)ress->rtp.ssrc;
	obj->after.state	= ress->state.current;
	//
	obj->notify.options	= (obj->before.options != ress->options && ress->lstnr.resRtpConsumeOptions != NULL);
	obj->notify.desc	= (obj->before.desc != ress->desc && ress->lstnr.resRtpConsumeSessionDesc != NULL);
	obj->notify.setup	= ((obj->before.setup != ress->setup || obj->before.ssrcId != ress->rtp.ssrc) && ress->lstnr.resRtpConsumeSetup != NULL);
	obj->notify.state	= (obj->before.state != ress->state.current && ress->lstnr.resRtspStateChanged != NULL);
	//
	return (obj->notify.options || obj->notify.desc || obj->notify.setup || obj->notify.state);
}

void NBNBRtspClientRessNotifyState_notify(STNBNBRtspClientRessNotifyState* obj, STNBRtspClientRes* ress){
	if(obj->notify.options){
		(*obj->lstnr.resRtpConsumeOptions)(obj->lstnr.obj, obj->uri, obj->after.options);
	}
	if(obj->notify.desc){
		(*obj->lstnr.resRtpConsumeSessionDesc)(obj->lstnr.obj, obj->uri, obj->after.desc);
	}
	if(obj->notify.setup){
		(*obj->lstnr.resRtpConsumeSetup)(obj->lstnr.obj, obj->uri, obj->after.setup, obj->after.ssrcId);
	}
	if(obj->notify.state){
		(*obj->lstnr.resRtspStateChanged)(obj->lstnr.obj, obj->uri, obj->after.state);
	}
}

//

BOOL NBRtspClient_ressStartNextRequestLockedOpq_(STNBRtspClientOpq* opq, STNBRtspClientRes* ress, const UI32 onlyIfSecsElapsed){
	BOOL r = FALSE;
	//Start requests
    const UI64 now = NBDatetime_getCurUTCTimestamp();
    //Analyze forced TEARDOWN conditions
    if(!ress->state.request.isTeardownPend && NBStopFlag_isAnyActivated(opq->stopFlag)){
        ress->state.request.isTeardownPend = TRUE;
    }
    if(!ress->state.request.isTeardownPend && (ress->state.current == ENNBRtspRessState_Playing && (now < ress->state.rtp.packets.utcTime.last || (now - ress->state.rtp.packets.utcTime.last) > ress->cfg.client.rtp.assumeConnDroppedAfterSecs))){
        PRINTF_WARNING("NBRtspClient, asumming conn dropped after %d secs without packets-in for '%s'.\n", (now - ress->state.rtp.packets.utcTime.last), ress->uri);
        ress->state.request.isTeardownPend = TRUE;
    }
    if(!ress->state.request.isTeardownPend && ress->state.request.seqErrsCount >= opq->cfg.requests.timeouts.max.retries){
        PRINTF_WARNING("NBRtspClient, asumming conn dropped after %d of %d consecutive failed request attempts for '%s'.\n", ress->state.request.seqErrsCount, opq->cfg.requests.timeouts.max.retries, ress->uri);
        ress->state.request.isTeardownPend = TRUE;
    }
    if(ress->state.request.curMethod != ENNBRtspMethod_Count){
        const UI64 delta = (ress->state.request.timeLastAttempt <= now ? (now - ress->state.request.timeLastAttempt) : (ress->state.request.timeLastAttempt - now));
        if(delta > 0 && (delta % 60) == 0){
            PRINTF_WARNING("NBRtspClient, request-type(%d) is taking %llu secs for '%s'.\n", (SI32)ress->state.request.curMethod, delta, ress->uri);
        }
    }
	const ENNBRtspRessState stateRequired = (ress->state.request.isTeardownPend ? ENNBRtspRessState_Stopped : ress->state.desired);
	if(ress->state.request.curMethod == ENNBRtspMethod_Count && ress->state.current != stateRequired){
		const UI64 delta = (ress->state.request.timeLastAttempt <= now ? (now - ress->state.request.timeLastAttempt) : (ress->state.request.timeLastAttempt - now));
		if(delta >= onlyIfSecsElapsed){
			//const BOOL isFirstAttempt = (ress->state.request.timeLastAttempt == 0);
			ress->state.request.timeLastAttempt = now;
			if(stateRequired <= ENNBRtspRessState_Stopped){
				//Start TEARDOWN (if required)
				if(ress->setup != NULL){
					if(ress->state.request.lstMethod == ENNBRtspMethod_TEARDOWN && ress->state.request.seqErrsCount > (NBStopFlag_isAnyActivated(opq->stopFlag) ? opq->cfg.requests.timeouts.min.retries : opq->cfg.requests.timeouts.max.retries)){
						PRINTF_ERROR("NBRtspClient, abandoning TEARDOWN attempt after %d consecutive errors in conn for '%s'.\n", ress->state.request.seqErrsCount, ress->uri);
						STNBNBRtspClientRessNotifyState notif;
						NBNBRtspClientRessNotifyState_init(&notif);
						NBNBRtspClientRessNotifyState_setInitialState(&notif, ress);
						{
							NBRtspClient_ressProcessResponseTEARDOWNLockedOpq_(opq, ress, NULL, NULL);
						}
						//Notify (unlocked)
						if(NBNBRtspClientRessNotifyState_setFinalState(&notif, ress)){
							NBThreadMutex_unlock(&ress->mutex);
							{
								NBThreadMutex_unlock(&opq->mutex);
								{
									NBNBRtspClientRessNotifyState_notify(&notif, ress);
								}
								NBThreadMutex_lock(&opq->mutex);
							}
							NBThreadMutex_lock(&ress->mutex);
						}
						NBNBRtspClientRessNotifyState_release(&notif);
					} else {
						NB_RTSP_CLIENT_START_RTSP_REQ_LOCKED(opq, ress, "TEARDOWN", ENNBRtspMethod_TEARDOWN, (NBStopFlag_isAnyActivated(opq->stopFlag) ? &opq->cfg.requests.timeouts.min : &opq->cfg.requests.timeouts.max));
						r = (ress->state.request.curMethod == ENNBRtspMethod_TEARDOWN);
					}
				}
			} else {
				if(stateRequired >= ENNBRtspRessState_Options && ress->options == NULL){
					//OPTIONS
					NB_RTSP_CLIENT_START_RTSP_REQ_LOCKED(opq, ress, "OPTIONS", ENNBRtspMethod_OPTIONS, &opq->cfg.requests.timeouts.max);
					r = (ress->state.request.curMethod == ENNBRtspMethod_OPTIONS);
				} else if(stateRequired >= ENNBRtspRessState_Described && ress->desc == NULL){
					//DESCRIBE
					NB_RTSP_CLIENT_START_RTSP_REQ_LOCKED(opq, ress, "DESCRIBE", ENNBRtspMethod_DESCRIBE, &opq->cfg.requests.timeouts.max);
					r = (ress->state.request.curMethod == ENNBRtspMethod_DESCRIBE);
				} else if(stateRequired >= ENNBRtspRessState_Paused || stateRequired >= ENNBRtspRessState_Playing){
					if(ress->setup == NULL){
						//SETUP
						NB_RTSP_CLIENT_START_RTSP_REQ_LOCKED(opq, ress, "SETUP", ENNBRtspMethod_SETUP, &opq->cfg.requests.timeouts.max);
						r = (ress->state.request.curMethod == ENNBRtspMethod_SETUP);
					} else if(ress->state.current != ENNBRtspRessState_Playing && stateRequired >= ENNBRtspRessState_Playing){
						//PLAY
						NB_RTSP_CLIENT_START_RTSP_REQ_LOCKED(opq, ress, "PLAY", ENNBRtspMethod_PLAY, &opq->cfg.requests.timeouts.max);
						r = (ress->state.request.curMethod == ENNBRtspMethod_PLAY);
					} else if(ress->state.current == ENNBRtspRessState_Playing && stateRequired >= ENNBRtspRessState_Paused){
						//PAUSE
						NB_RTSP_CLIENT_START_RTSP_REQ_LOCKED(opq, ress, "PAUSE", ENNBRtspMethod_PAUSE, &opq->cfg.requests.timeouts.max);
						r = (ress->state.request.curMethod == ENNBRtspMethod_PAUSE);
					}
				}
			} 
		}
	}
	return r;
}

//Callback

void NBRtspClient_reqResult_(void* obj, const UI32 uid, const STNBSocketAddr* remoteAddr, const char* method, const char* uri, const ENNBRtspClientConnReqStatus status, const STNBHttpResponse* response){
	STNBRtspClientOpq* opq = (STNBRtspClientOpq*)obj;
	if(opq != NULL && !NBString_strIsEmpty(uri)){
		STNBRtspClientResLstnr lstnr;
		BOOL activeFound = FALSE, subReqStarted = FALSE;
		NBMemory_setZeroSt(lstnr, STNBRtspClientResLstnr);
		//Apply result
		{
			NBThreadMutex_lock(&opq->mutex);
			{	
				const SI32 count = opq->ress.arr.use;
				SI32 i; for(i = 0; i < count; i++){
					STNBRtspClientRes* ress = NBArray_itmValueAtIndex(&opq->ress.arr, STNBRtspClientRes*, i);
					if(NBString_strIsEqual(ress->uri, uri)){
						BOOL mustNotify = FALSE;
						STNBNBRtspClientRessNotifyState notify;
						NBNBRtspClientRessNotifyState_init(&notify);
						NBThreadMutex_lock(&ress->mutex);
						{
							NBNBRtspClientRessNotifyState_setInitialState(&notify, ress);
							//Unset request
							{
								activeFound = TRUE;
								ress->state.request.curMethod = ENNBRtspMethod_Count;
								//PRINTF_INFO("NBRtspClient_reqResult_, %d request active.\n", opq->reqs.activeCount);
							}
							//Process results
							if(status != ENNBRtspClientConnReqStatus_Success){
								ress->state.request.seqErrsCount++;
							} else {
								if(NBString_strIsLike(method, "OPTIONS")){
									NBASSERT(status == ENNBRtspClientConnReqStatus_Success)
									if(!NBRtspClient_ressProcessResponseOPTIONSLockedOpq_(opq, ress, remoteAddr, response)){
										ress->state.request.seqErrsCount++;
									} else {
										ress->state.request.seqErrsCount = 0;
									}
								} else if(NBString_strIsLike(method, "DESCRIBE")){
									NBASSERT(status == ENNBRtspClientConnReqStatus_Success)
									if(!NBRtspClient_ressProcessResponseDESCRIBELockedOpq_(opq, ress, remoteAddr, response)){
										ress->state.request.seqErrsCount++;
									} else {
										ress->state.request.seqErrsCount = 0;
									}
								} else if(NBString_strIsLike(method, "SETUP")){
									NBASSERT(status == ENNBRtspClientConnReqStatus_Success)
									if(!NBRtspClient_ressProcessResponseSETUPLockedOpq_(opq, ress, remoteAddr, response)){
										ress->state.request.seqErrsCount++;
									} else {
										ress->state.request.seqErrsCount = 0;
									}
								} else if(NBString_strIsLike(method, "PLAY")){
									NBASSERT(status == ENNBRtspClientConnReqStatus_Success)
									if(!NBRtspClient_ressProcessResponsePLAYLockedOpq_(opq, ress, remoteAddr, response)){
										ress->state.request.seqErrsCount++;
									} else {
										ress->state.request.seqErrsCount = 0;
									}
								} else if(NBString_strIsLike(method, "PAUSE")){
									NBASSERT(status == ENNBRtspClientConnReqStatus_Success)
									if(!NBRtspClient_ressProcessResponsePAUSELockedOpq_(opq, ress, remoteAddr, response)){
										ress->state.request.seqErrsCount++;
									} else {
										ress->state.request.seqErrsCount = 0;
									}
								} else if(NBString_strIsLike(method, "TEARDOWN")){
									NBASSERT(status == ENNBRtspClientConnReqStatus_Success)
									if(!NBRtspClient_ressProcessResponseTEARDOWNLockedOpq_(opq, ress, remoteAddr, response)){
										ress->state.request.seqErrsCount++;
									} else {
										ress->state.request.seqErrsCount = 0;
									}
								} else {
									NBASSERT(status == ENNBRtspClientConnReqStatus_Success)
									//Unexpected request type
									ress->state.request.seqErrsCount++;
								}
							}
							lstnr = ress->lstnr;
							mustNotify = NBNBRtspClientRessNotifyState_setFinalState(&notify, ress);
						}
						NBThreadMutex_unlock(&ress->mutex);
						//Notify (unlocked)
						if(mustNotify){
							NBThreadMutex_unlock(&opq->mutex);
							{
								NBNBRtspClientRessNotifyState_notify(&notify, ress);
							}
							NBThreadMutex_lock(&opq->mutex);
						}
						NBNBRtspClientRessNotifyState_release(&notify);
						//
						break;
					}
				}
			}
			NBThreadMutex_unlock(&opq->mutex);
		}
		//Notify (unlocked)
		{
			if(lstnr.resRtpConsumeReqResult != NULL){
				(lstnr.resRtpConsumeReqResult)(lstnr.obj, uri, method, status);
			}
		}
		//Process
		NBASSERT(activeFound)
		if(activeFound){
			NBThreadMutex_lock(&opq->mutex);
			//Start new request (current activeCount will be used)
			{
				const SI32 count = opq->ress.arr.use;
				SI32 i; for(i = 0; i < count; i++){
					STNBRtspClientRes* ress = NBArray_itmValueAtIndex(&opq->ress.arr, STNBRtspClientRes*, i);
					if(NBString_strIsEqual(ress->uri, uri)){
						NBThreadMutex_lock(&ress->mutex);
						{
							//start next request (if necesary)
							if(ress->state.request.curMethod == ENNBRtspMethod_Count && ress->state.request.seqErrsCount == 0){
								if(NBRtspClient_ressStartNextRequestLockedOpq_(opq, ress, 0)){
									subReqStarted =  TRUE;
								}
							}
							//apply stop flag
							if(NBStopFlag_isAnyActivated(opq->stopFlag) && !NBRtspClientRes_isBusy(ress)){
								NBRtspClientRes_applyStopFlag(ress);
							}
						}
						NBThreadMutex_unlock(&ress->mutex);
						break;
					}
				}
			}
			NBThreadMutex_unlock(&opq->mutex);
		}
	}
}

BOOL NBRtspClient_ressProcessResponseOPTIONSLockedOpq_(STNBRtspClientOpq* opq, STNBRtspClientRes* ress, const STNBSocketAddr* remoteAddr, const STNBHttpResponse* resp){
	/*
	RTSP/1.0 200 OK
	CSeq: 1
	Public: OPTIONS,DESCRIBE,SETUP,PLAY,PAUSE,TEARDOWN,ANNOUNCE,SET_PARAMETER,GET_PARAMETER
	*/
	//
	//https://en.wikipedia.org/wiki/Digest_access_authentication
	//
	BOOL r = FALSE;
	if(ress != NULL){
		//Public
		STNBHttpHeaderFieldCursor cursor;
		NBHttpHeaderFieldCursor_init(&cursor);
		while(NBHttpResponse_getNextFieldValue(resp, "Public", FALSE, &cursor)){
			if(cursor.value != NULL && cursor.valueLen > 0){
				const STNBRtspMethodDef* def = NBRtsp_getMethodDefByNameBytes(cursor.value, cursor.valueLen);
				//PRINTF_INFO("RTSP, option (%d bytes) '%s'.\n", cursor.valueLen, cursor.value);
				if(def == NULL){
					PRINTF_WARNING("RTSP, method (%d bytes) '%s' not-supported from OPTIONS list.\n", cursor.valueLen, cursor.value);
				} else {
					//Create (if necessary)
					if(ress->options == NULL){
						ress->options = NBMemory_allocType(STNBRtspOptions);
						NBMemory_setZeroSt(*ress->options, STNBRtspOptions);;
					}
					//Apply
					ress->options->pub.methods[def->methodId] = TRUE;
					r = TRUE;
					//Set state
					ress->state.request.seqErrsCount = 0;
					if(ress->state.current < ENNBRtspRessState_Options){
						ress->state.current = ENNBRtspRessState_Options;
					}
				}
			}
		}
		NBHttpHeaderFieldCursor_release(&cursor);
	}
	return r;
}

BOOL NBRtspClient_ressProcessResponseDESCRIBELockedOpq_(STNBRtspClientOpq* opq, STNBRtspClientRes* ress, const STNBSocketAddr* remoteAddr, const STNBHttpResponse* resp){
	/*
	 RTSP/1.0 200 OK
	 CSeq: 3
	 Content-Base: rtsp://192.168.1.65/media/video3
	 Content-Length: 508
	 Content-Type: application/sdp
	 
	 v=0
	 o=- 1001 1 IN IP4 192.168.0.222
	 s=VCP IPC Realtime stream
	 m=video 0 RTP/AVP 105
	 c=IN IP4 192.168.0.222
	 a=control:rtsp://192.168.0.222/media/video2/video
	 a=rtpmap:105 H264/90000
	 a=fmtp:105 profile-level-id=4d401f; packetization-mode=1; sprop-parameter-sets=Z01AH5WgLASabIAAAAMAgAAAGUI=,aO48gA==
	 a=recvonly
	 m=application 0 RTP/AVP 107
	 c=IN IP4 192.168.0.222
	 a=control:rtsp://192.168.0.222/media/video2/metadata
	 a=rtpmap:107 vnd.onvif.metadata/90000
	 a=fmtp:107 DecoderTag=h3c-v3 RTCP=0
	 a=recvonly
	 
	 v=0
	 o=- 1628966255381873 1628966255381873 IN IP4 192.168.0.221
	 s=Media Presentation
	 e=NONE
	 b=AS:5050
	 t=0 0
	 a=control:rtsp://192.168.0.221/Streaming/channels/102/trackID=1/
	 m=video 0 RTP/AVP 96
	 c=IN IP4 0.0.0.0
	 b=AS:5000
	 a=recvonly
	 a=x-dimensions:640,480
	 a=control:rtsp://192.168.0.221/Streaming/channels/102/trackID=1/trackID=1
	 a=rtpmap:96 H264/90000
	 a=fmtp:96 profile-level-id=420029; packetization-mode=1; sprop-parameter-sets=Z01AHo2NQFAe//+APwAvtwEBAUAAAPoAADDUOhgB6cAAJiWrvLjQwA9OAAExLV3lwoA=,aO44gA==
	 a=Media_header:MEDIAINFO=494D4B48010300000400000100000000000000000000000000000000000000000000000000000000;
	 a=appversion:1.0
	*/
	//PRINTF_INFO("NBRtspClient, DESCRIBE body: %s.\n", resp->body.str);
	//
	BOOL r = FALSE;
	if(ress != NULL && resp != NULL){
		if(resp->body.length > 0){
			UI32 cnmsd = 0;
			STNBSdpDesc* desc = NBMemory_allocType(STNBSdpDesc);
			NBSdpDesc_init(desc);
			cnmsd = NBSdpDesc_appendText(desc, resp->body.str, resp->body.length, TRUE); 
			if(cnmsd < resp->body.length){
				//Body partially consumed
			} else if(!NBSdpDesc_canBeClosed(desc)){
				//Body is not valid SessionDescriptionProtocol
			} else {
				if(ress->desc != NULL){
					NBSdpDesc_release(ress->desc);
					NBMemory_free(ress->desc);
					ress->desc = NULL;
				}
				ress->desc = desc;
				desc = NULL;
				r = TRUE;
				//Set state
				ress->state.request.seqErrsCount = 0;
				if(ress->state.current < ENNBRtspRessState_Described){
					ress->state.current = ENNBRtspRessState_Described;
				}
			}
			//Release (if not consumed)
			if(desc != NULL){
				NBSdpDesc_release(desc);
				NBMemory_free(desc);
				desc = NULL;
			}
		}
	}
	return r;
}

BOOL NBRtspClient_parseUIntPair_(const char* v, UI32* dst1, UI32* dst2){
	BOOL r = FALSE;
	const SI32 iSep		= NBString_strIndexOf(v, "-", 0);
	const SI32 strLen	= NBString_strLenBytes(v); 
	if(iSep < 0){
		//Only left digit
		if(strLen > 0 && NBString_strIsInteger(v)){
			if(dst1 != NULL){
				*dst1 = NBString_strToUI32(v);
			}
			r = TRUE;
		}
	} else if(iSep == 0){
		//Only right digit
		if(strLen > 0 && NBString_strIsInteger(&v[1])){
			if(dst2 != NULL){
				*dst2 = NBString_strToUI32(&v[1]);
			}
			r = TRUE;
		}
	} else {
		//Left and right digit
		if(NBString_strIsIntegerBytes(v, iSep) && (iSep + 1) < strLen && NBString_strIsInteger(&v[iSep + 1])){
			if(dst1 != NULL){
				*dst1 = NBString_strToUI32Bytes(v, iSep);
			}
			if(dst2 != NULL){
				*dst2 = NBString_strToUI32(&v[iSep + 1]);
			}
			r = TRUE;
		}
	}
	return r;
}

BOOL NBRtspClient_ressProcessResponseSETUPLockedOpq_(STNBRtspClientOpq* opq, STNBRtspClientRes* ress, const STNBSocketAddr* remoteAddr, const STNBHttpResponse* resp){
	/*
	RTSP/1.0 200 OK
	CSeq: 4
	Transport: RTP/AVP/UDP;unicast;destination=192.168.1.176;client_port=8000-8001;server_port=10046-10047;ssrc=7237962;mode="PLAY"
	Session: 156acc00ee6acc004a6acc00a26acc0;timeout=60
	*/
	/*Transports are comma separated, listed in order of preference.
	Parameters may be added to each transport, separated by a semicolon.*/
	//
	//https://en.wikipedia.org/wiki/Digest_access_authentication
	//
	BOOL r = FALSE;
	if(ress != NULL){
		BOOL ssrcAvpFnd = FALSE; UI32 ssrcAvp = 0;
		//Consume setup
		{
			//Transport
			{
				/*
				 Transport           =    "Transport" ":"
				 1\#transport-spec
				 transport-spec      =    transport-protocol/profile[/lower-transport]
				 *parameter
				 transport-protocol  =    "RTP"
				 profile             =    "AVP"
				 lower-transport     =    "TCP" | "UDP"
				 parameter           =    ( "unicast" | "multicast" )
				 |    ";" "destination" [ "=" address ]
				 |    ";" "interleaved" "=" channel [ "-" channel ]
				 |    ";" "append"
				 |    ";" "ttl" "=" ttl
				 |    ";" "layers" "=" 1*DIGIT
				 |    ";" "port" "=" port [ "-" port ]
				 |    ";" "client_port" "=" port [ "-" port ]
				 |    ";" "server_port" "=" port [ "-" port ]
				 |    ";" "ssrc" "=" ssrc
				 |    ";" "mode" = <"> 1\#mode <">
				 ttl                 =    1*3(DIGIT)
				 port                =    1*5(DIGIT)
				 ssrc                =    8*8(HEX)
				 channel             =    1*3(DIGIT)
				 address             =    host
				 mode                =    <"> *Method <"> | Method
				 */
				STNBHttpHeaderFieldCursor cursor;
				NBHttpHeaderFieldCursor_init(&cursor);
				if(ress->setup == NULL){
					ress->setup = NBMemory_allocType(STNBRtspSetup);
					NBRtspSetup_init(ress->setup);
				} else {
					//Remove all specs
					{
						SI32 i; for(i = 0; i < ress->setup->transport.specs.use; i++){
							STNBRtspTransportSpec* spec = NBArray_itmPtrAtIndex(&ress->setup->transport.specs, STNBRtspTransportSpec, i);
							NBRtspTransportSpec_release(spec);
						}
						NBArray_empty(&ress->setup->transport.specs);
					}
				}
				//Add new specs
				while(NBHttpResponse_getNextFieldValue(resp, "Transport", FALSE, &cursor)){
					if(cursor.value != NULL && cursor.valueLen > 0){
						const char* c = cursor.value;
						SI32 paramEnd = NBString_strIndexOfBytes(c, cursor.valueLen, ";", 0);
						if(paramEnd < 0){
							paramEnd = cursor.valueLen;
						}
						{
							const SI32 iSlash1 = NBString_strIndexOfBytes(c, cursor.valueLen, "/", 0);
							if(iSlash1 <= 0){
								PRINTF_ERROR("RTSP, transport, expected 'transport-protocol' at (%d bytes) '%s'.\n", cursor.valueLen, cursor.value);
							} else {
								const SI32 iSlash2 = NBString_strIndexOfBytes(c, cursor.valueLen, "/", iSlash1 + 1);
								STNBRtspTransportSpec spec;
								NBRtspTransportSpec_init(&spec);
								spec.protocol			= NBString_strNewBufferBytes(c, iSlash1);
								if(iSlash2 < 0){
									spec.profile		= NBString_strNewBufferBytes(&c[iSlash1 + 1], paramEnd - iSlash1 - 1); 
								} else {
									spec.profile		= NBString_strNewBufferBytes(&c[iSlash1 + 1], iSlash2 - iSlash1 - 1);
									spec.lowerTransport	= NBString_strNewBufferBytes(&c[iSlash2 + 1], paramEnd - iSlash2 - 1);
								}
								//PRINTF_INFO("RTSP, transport protocol('%s')-profile('%s')-lowerTransport('%s').\n", spec.protocol, spec.profile, spec.lowerTransport);
								//Params
								while(paramEnd < cursor.valueLen){
									SI32 paramEnd2 = NBString_strIndexOfBytes(c, cursor.valueLen, ";", paramEnd + 1);
									if(paramEnd2 < 0){
										paramEnd2 = cursor.valueLen;
									}
									if((paramEnd + 1) < paramEnd2){
										const char* param	= &c[paramEnd + 1];
										const UI32 paramLen = (paramEnd2 - paramEnd - 1); NBASSERT(paramLen > 0)
										const SI32 iEqual	= NBString_strIndexOfBytes(param, paramLen, "=", 0);
										if(iEqual == 0){
											//Unamed param
											PRINTF_ERROR("RTSP, transport-param, expected name at (%d bytes) '%s'.\n", paramLen, param);
										} else  if(iEqual < 0){
											//Boolean param
											STNBRtspParam p;
											NBRtspParam_init(&p);
											p.name = NBString_strNewBufferBytes(param, paramLen);
											//PRINTF_INFO("RTSP, transport-param('%s').\n", p.name);
											NBArray_addValue(&spec.params, p);
											if(NBString_strIsLike(p.name, "unicast")){
												spec.parsed.deliveryMode = ENRtspDeliveryMode_Unicast;
											} else if(NBString_strIsLike(p.name, "multicast")){
												spec.parsed.deliveryMode = ENRtspDeliveryMode_Multicast;
											} else if(NBString_strIsLike(p.name, "append")){
												spec.parsed.append = TRUE;
											} else {
												PRINTF_WARNING("RTSP, transport-param('%s') not implemented.\n", p.name);
											}
										} else {
											//name-value pair
											STNBRtspParam p;
											NBRtspParam_init(&p);
											p.name	= NBString_strNewBufferBytes(param, iEqual);
											p.value	= NBString_strNewBufferBytes(&param[iEqual + 1], paramLen - iEqual - 1);
											//PRINTF_INFO("RTSP, transport-param('%s' = '%s').\n", p.name, p.value);
											NBArray_addValue(&spec.params, p);
											if(NBString_strIsLike(p.name, "destination")){
												spec.parsed.destination = p.value;
											} else if(NBString_strIsLike(p.name, "mode")){
												spec.parsed.mode = p.value;
											} else if(NBString_strIsLike(p.name, "layers")){
												if(NBString_strIsInteger(p.value)){
													spec.parsed.layers = NBString_strToSI32(p.value);
												} else {
													PRINTF_WARNING("RTSP, transport-param('%s' = '%s') expected as integer.\n", p.name, p.value);
												}
											} else if(NBString_strIsLike(p.name, "ttl")){
												if(NBString_strIsInteger(p.value)){
													spec.parsed.multicast.ttl = NBString_strToSI32(p.value);
												} else {
													PRINTF_WARNING("RTSP, transport-param('%s' = '%s') expected as integer.\n", p.name, p.value);
												}
											} else if(NBString_strIsLike(p.name, "ssrc")){
												BOOL errFnd = FALSE; UI32 v = 0; const char* c = p.value;
												while(*c != '\0'){
													if(*c >= '0' && *c <= '9'){
														v = (v << 4) | (*c - '0');
													} else if(*c >= 'A' && *c <= 'F'){
														v = (v << 4) | (10 + (*c - 'A'));
													} else if(*c >= 'a' && *c <= 'f'){
														v = (v << 4) | (10 + (*c - 'a'));
													} else {
														PRINTF_ERROR("RTSP, transport-param('%s' = '%s') expected as hex-value.\n", p.name, p.value);
														errFnd = TRUE;
														break;
													}
													c++;
												}
												if(!errFnd){
													spec.parsed.rtp.ssrc = v;
													//PRINTF_INFO("RTSP, ssrc=%u.\n", v);
												}
											} else if(NBString_strIsLike(p.name, "interleaved")){
												if(!NBRtspClient_parseUIntPair_(p.value, &spec.parsed.interleaved.c0, &spec.parsed.interleaved.c1)){
													PRINTF_ERROR("RTSP, transport-param('%s' = '%s') expected as int[-int].\n", p.name, p.value);
												}
											} else if(NBString_strIsLike(p.name, "port")){
												if(!NBRtspClient_parseUIntPair_(p.value, &spec.parsed.rtp.port.rtp, &spec.parsed.rtp.port.rtcp)){
													PRINTF_ERROR("RTSP, transport-param('%s' = '%s') expected as int[-int].\n", p.name, p.value);
												}
											} else if(NBString_strIsLike(p.name, "client_port")){
												if(!NBRtspClient_parseUIntPair_(p.value, &spec.parsed.rtp.client_port.rtp, &spec.parsed.rtp.client_port.rtcp)){
													PRINTF_ERROR("RTSP, transport-param('%s' = '%s') expected as int[-int].\n", p.name, p.value);
												}
											} else if(NBString_strIsLike(p.name, "server_port")){
												if(!NBRtspClient_parseUIntPair_(p.value, &spec.parsed.rtp.server_port.rtp, &spec.parsed.rtp.server_port.rtcp)){
													PRINTF_ERROR("RTSP, transport-param('%s' = '%s') expected as int[-int].\n", p.name, p.value);
												}
											} else {
												PRINTF_WARNING("RTSP, transport-param('%s') not implemented.\n", p.name);
											}
										}
									}
									paramEnd = paramEnd2;
								}
								//Add to idx
								if(spec.parsed.rtp.ssrc != 0){
									if(NBString_strIsLike(spec.profile, "AVP")){
										ssrcAvp = spec.parsed.rtp.ssrc;
										ssrcAvpFnd = TRUE;
									}
								}
								NBArray_addValue(&ress->setup->transport.specs, spec);
							}
						}
						//PRINTF_INFO("RTSP, transport (%d bytes) '%s'.\n", cursor.valueLen, cursor.value);
					}
				}
				NBHttpHeaderFieldCursor_release(&cursor);
			}
			//Session
			{
				/*
				 Session  = "Session" ":" session-id [ ";" "timeout" "=" delta-seconds ]
				 */
				STNBHttpHeaderFieldCursor cursor;
				NBHttpHeaderFieldCursor_init(&cursor);
				if(ress->setup == NULL){
					ress->setup = NBMemory_allocType(STNBRtspSetup);
					NBRtspSetup_init(ress->setup);
				} else {
					//Remove session
					if(ress->setup->session.sessionId != NULL){
						NBMemory_free(ress->setup->session.sessionId);
						ress->setup->session.sessionId = NULL;
					}
					{
						SI32 i; for(i = 0; i < ress->setup->session.params.use; i++){
							STNBRtspParam* param = NBArray_itmPtrAtIndex(&ress->setup->session.params, STNBRtspParam, i);
							NBRtspParam_release(param);
						}
						NBArray_empty(&ress->setup->session.params);
					}
				}
				//Add new specs
				while(NBHttpResponse_getNextFieldValue(resp, "Session", FALSE, &cursor)){
					if(cursor.value != NULL && cursor.valueLen > 0){
						const char* c = cursor.value;
						SI32 paramEnd = NBString_strIndexOfBytes(c, cursor.valueLen, ";", 0);
						if(paramEnd < 0){
							paramEnd = cursor.valueLen;
						}
						//Set sessionId
						{
							if(ress->setup->session.sessionId != NULL){
								NBMemory_free(ress->setup->session.sessionId);
								ress->setup->session.sessionId = NULL;
							}
							if(paramEnd > 0){
								ress->setup->session.sessionId = NBString_strNewBufferBytes(c, paramEnd);
							}
							//PRINTF_INFO("RTSP, transport sessionId('%s').\n", ress->setup->session.sessionId);
						}
						//Load params
						while(paramEnd < cursor.valueLen){
							SI32 paramEnd2 = NBString_strIndexOfBytes(c, cursor.valueLen, ";", paramEnd + 1);
							if(paramEnd2 < 0){
								paramEnd2 = cursor.valueLen;
							}
							if((paramEnd + 1) < paramEnd2){
								const char* param	= &c[paramEnd + 1];
								const UI32 paramLen = (paramEnd2 - paramEnd - 1); NBASSERT(paramLen > 0)
								const SI32 iEqual	= NBString_strIndexOfBytes(param, paramLen, "=", 0);
								if(iEqual == 0){
									//Unamed param
									PRINTF_ERROR("RTSP, session-param, expected name at (%d bytes) '%s'.\n", paramLen, param);
								} else  if(iEqual < 0){
									//Boolean param
									STNBRtspParam p;
									NBRtspParam_init(&p);
									p.name = NBString_strNewBufferBytes(param, paramLen);
									//PRINTF_INFO("RTSP, transport-param('%s').\n", p.name);
									NBArray_addValue(&ress->setup->session.params, p);
									{
										PRINTF_WARNING("RTSP, session-param('%s') not implemented.\n", p.name);
									}
								} else {
									//name-value pair
									STNBRtspParam p;
									NBRtspParam_init(&p);
									p.name	= NBString_strNewBufferBytes(param, iEqual);
									p.value	= NBString_strNewBufferBytes(&param[iEqual + 1], paramLen - iEqual - 1);
									//PRINTF_INFO("RTSP, session-param('%s' = '%s').\n", p.name, p.value);
									NBArray_addValue(&ress->setup->session.params, p);
									if(NBString_strIsLike(p.name, "timeout")){
										if(NBString_strIsInteger(p.value)){
											ress->setup->session.parsed.timeout = NBString_strToSI32(p.value); 
										} else {
											PRINTF_WARNING("RTSP, session-param('%s' = '%s') expected as integer.\n", p.name, p.value);
										}
									} else {
										PRINTF_WARNING("RTSP, session-param('%s') not implemented.\n", p.name);
									}
								}
							}
							paramEnd = paramEnd2;
						}
						//PRINTF_INFO("RTSP, session (%d bytes) '%s'.\n", cursor.valueLen, cursor.value);
					}
				}
				NBHttpHeaderFieldCursor_release(&cursor);
			}
		}
		//Set ress at idx
		if(opq != NULL){
			//Remove lstnr
			if(ress->rtp.ssrc != 0){
				//RtpServer
				if(!NBRtpClient_removeStreamLstnr(&ress->rtp.lclSvr->server, ress->rtp.ssrc, ress->rtp.ssrcAddr)){
					NBASSERT(FALSE); //Program logic error
				}
				//Internal idx
				{
					STNBRtspClientSsrcIdx srch;
					srch.ssrc	= ress->rtp.ssrc;
					srch.addr	= ress->rtp.ssrcAddr;
					{
						const SI32 iFnd = NBArraySorted_indexOf(&opq->ress.idx, &srch, sizeof(srch), NULL);
						if(iFnd >= 0){
							NBArraySorted_removeItemAtIndex(&opq->ress.idx, iFnd);
							r = TRUE;
						}
					}
				}
				//Set
				ress->rtp.ssrc = 0;
				NBMemory_setZeroSt(ress->rtp.ssrcAddr, STNBSocketAddr);
			}
			//Add lstnr
			if(ssrcAvpFnd && ssrcAvp != 0){
#				ifdef NB_CONFIG_INCLUDE_ASSERTS
				/*{
					char addrStr[NB_SOCKET_ADDR_STR_MAX_SZ];
					if(remoteAddr == NULL){
						addrStr[0] = '\0';
					} else {
						NBSocketAddr_concatAddrOnly(remoteAddr, addrStr, sizeof(addrStr));
					}
					PRINTF_INFO("NBRtspClient, added 'SETUP', 'ssrc=%x' (%u) addr(%s) for: '%s'.\n", ssrcAvp, ssrcAvp, addrStr, ress->uri);
				}*/
#				endif
				//RtpServer
				{
					STNBRtpClientLstnr lstnr;
					NBMemory_setZeroSt(lstnr, STNBRtpClientLstnr);
					lstnr.port			= ress->rtp.lclPort->port;
					lstnr.ssrc			= ssrcAvp;
					lstnr.obj			= ress;
					lstnr.rtpBorderPresent	= NBRtspClient_rtpBorderPresent_;
					lstnr.rtpConsume		= NBRtspClient_rtpConsume_;
					lstnr.rtpFlushStats		= NBRtspClient_rtpFlushStats_;
					if(remoteAddr != NULL){
						lstnr.addr		= *remoteAddr;
					}
					if(!NBRtpClient_addStreamLstnr(&ress->rtp.lclSvr->server, &lstnr)){
						NBASSERT(FALSE); //Program logic error
					}
				}
				//Internal idx
				{
					STNBRtspClientSsrcIdx idx;
					NBMemory_setZeroSt(idx, STNBRtspClientSsrcIdx);
					idx.ssrc = ssrcAvp;
					idx.ress = ress;
					if(remoteAddr != NULL){
						idx.addr = *remoteAddr;
					}
					NBArraySorted_addValue(&opq->ress.idx, idx);
				}
				//Set
				ress->rtp.ssrc = ssrcAvp;
				if(remoteAddr == NULL){
					NBMemory_setZeroSt(ress->rtp.ssrcAddr, STNBSocketAddr);
				} else {
					ress->rtp.ssrcAddr = *remoteAddr;
				}
				//Set state
				ress->state.request.seqErrsCount = 0;
				if(ress->state.current < ENNBRtspRessState_Paused){
					ress->state.current = ENNBRtspRessState_Paused;
				}
				//
				r = TRUE;
			}
		}
	}
	return r;
}

BOOL NBRtspClient_ressProcessResponsePLAYLockedOpq_(STNBRtspClientOpq* opq, STNBRtspClientRes* ress, const STNBSocketAddr* remoteAddr, const STNBHttpResponse* resp){
	/*
	 RTSP/1.0 200 OK
	 CSeq: 8
	 Session:        953681080
	 RTP-Info: url=rtsp://192.168.0.221/Streaming/channels/103/trackID=1/;seq=63478;rtptime=1173688470
	 Date:  Mon, Jul 05 2021 17:56:55 GMT
	*/
	BOOL r = FALSE;
	if(ress != NULL && resp != NULL){
		r = TRUE;
		//Set state
		ress->state.request.seqErrsCount = 0;
		ress->state.rtp.packets.utcTime.start = ress->state.rtp.packets.utcTime.last = NBDatetime_getCurUTCTimestamp();
		if(ress->state.current < ENNBRtspRessState_Playing){
            PRINTF_CONSOLE("NBRtspClient, PLAYING '%s'.\n", ress->uri);
			ress->state.current = ENNBRtspRessState_Playing;
		}
	}
	return r;
}

BOOL NBRtspClient_ressProcessResponsePAUSELockedOpq_(STNBRtspClientOpq* opq, STNBRtspClientRes* ress, const STNBSocketAddr* remoteAddr, const STNBHttpResponse* resp){
	/*
	 RTSP/1.0 200 OK
	 CSeq: 8
	 Session:        953681080
	 RTP-Info: url=rtsp://192.168.0.221/Streaming/channels/103/trackID=1/;seq=63478;rtptime=1173688470
	 Date:  Mon, Jul 05 2021 17:56:55 GMT
	*/
	BOOL r = FALSE;
	if(ress != NULL && resp != NULL){
		r = TRUE;
		//Set state
		ress->state.request.seqErrsCount = 0;
		if(ress->state.current > ENNBRtspRessState_Paused){
			PRINTF_CONSOLE("NBRtspClient, PAUSED '%s'.\n", ress->uri);
			ress->state.current = ENNBRtspRessState_Paused;
		}
	}
	return r;
}

BOOL NBRtspClient_ressProcessResponseTEARDOWNLockedOpq_(STNBRtspClientOpq* opq, STNBRtspClientRes* ress, const STNBSocketAddr* remoteAddr, const STNBHttpResponse* resp){
	//
	//https://en.wikipedia.org/wiki/Digest_access_authentication
	//
	BOOL r = FALSE;
	if(ress != NULL){
		//Remove ress from idx
		if(opq != NULL){
			//Remove lstnr
			if(ress->rtp.ssrc != 0){
				//RtpServer
				if(!NBRtpClient_removeStreamLstnr(&ress->rtp.lclSvr->server, ress->rtp.ssrc, ress->rtp.ssrcAddr)){
					NBASSERT(FALSE); //Program logic error
				}
				//Internal idx
				{
					STNBRtspClientSsrcIdx srch;
					srch.ssrc	= ress->rtp.ssrc;
					srch.addr	= ress->rtp.ssrcAddr;
					{
						const SI32 iFnd = NBArraySorted_indexOf(&opq->ress.idx, &srch, sizeof(srch), NULL);
						if(iFnd >= 0){
							NBArraySorted_removeItemAtIndex(&opq->ress.idx, iFnd);
							r = TRUE;
						}
					}
				}
				//Set
				ress->rtp.ssrc = 0;
				NBMemory_setZeroSt(ress->rtp.ssrcAddr, STNBSocketAddr);
			}
		}
		//clean-up
		{
			//Setup
			if(ress->setup != NULL){
				NBRtspSetup_release(ress->setup);
				NBMemory_free(ress->setup);
				ress->setup = NULL;
			}
			//Options
			if(ress->options != NULL){
				NBMemory_free(ress->options);
				ress->options = NULL;
			}
			//Desc
			if(ress->desc != NULL){
				NBSdpDesc_release(ress->desc);
				NBMemory_free(ress->desc);
				ress->desc = NULL;
			}
			//Set state
            ress->state.request.isTeardownPend = FALSE;
			ress->state.request.seqErrsCount = 0;
			ress->state.current = ENNBRtspRessState_Stopped;
            PRINTF_CONSOLE("NBRtspClient, STOPPED '%s'.\n", ress->uri);
		}
		r = TRUE;
	}
	return r;
}

//

BOOL NBRtspClient_rtpBorderPresent_(void* obj, const STNBRtpHdrBasic* hdrs, const STNBDataPtr* chunks, const UI32 chunksSz){
	BOOL r = FALSE;
	STNBRtspClientRes* ress = (STNBRtspClientRes*)obj;
	if(ress != NULL && hdrs != NULL && chunks != NULL && chunksSz > 0){
		if(ress->lstnr.resRtpIsBorderPresent != NULL){ 
			r = (*ress->lstnr.resRtpIsBorderPresent)(ress->lstnr.obj, ress->uri, hdrs, chunks, chunksSz);
		}
	}
	return r;
}

void NBRtspClient_rtpConsume_(void* obj, STNBRtpHdrBasic* hdrs, STNBDataPtr* chunks, const UI32 chunksSz, STNBRtpClientStatsUpd* statsUpd, STNBDataPtrReleaser* optPtrsReleaser){
	STNBRtspClientRes* ress = (STNBRtspClientRes*)obj;
	if(ress != NULL && hdrs != NULL && chunks != NULL && chunksSz > 0){
		const UI64 curUtcTime = NBDatetime_getCurUTCTimestamp();
		//Set packet last time
		ress->state.rtp.packets.utcTime.last = curUtcTime;
		//Notify
		if(ress->lstnr.resRtpConsumePackets != NULL){ 
			(*ress->lstnr.resRtpConsumePackets)(ress->lstnr.obj, ress->uri, hdrs, chunks, chunksSz, curUtcTime, optPtrsReleaser);
		}
	}
}

void NBRtspClient_rtpFlushStats_(void* obj){
	STNBRtspClientRes* ress = (STNBRtspClientRes*)obj;
	if(ress != NULL && ress->lstnr.resRtpFlushStats != NULL){
		(*ress->lstnr.resRtpFlushStats)(ress->lstnr.obj);
	}
}

//Analyze

void NBRtspClient_tickAnalyzeQuick(STNBRtspClient* obj, const UI32 onlyIfSecsElapsed){
	STNBRtspClientOpq* opq = (STNBRtspClientOpq*)obj->opaque;
	if(opq != NULL){
		NBThreadMutex_lock(&opq->mutex);
		{
			SI32 i; for(i = 0; i < opq->ress.arr.use; i++){
				STNBRtspClientRes* ress = NBArray_itmValueAtIndex(&opq->ress.arr, STNBRtspClientRes*, i);
				if(ress != NULL){
					NBThreadMutex_lock(&ress->mutex);
					{
						NBRtspClient_ressStartNextRequestLockedOpq_(opq, ress, onlyIfSecsElapsed);
					}
					NBThreadMutex_unlock(&ress->mutex);
				}
			}
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
}

