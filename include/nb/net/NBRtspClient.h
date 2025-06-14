
#ifndef NB_RTSPCLIENT_H
#define NB_RTSPCLIENT_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBDataPtr.h"
#include "nb/core/NBDataPtrsPool.h"
#include "nb/core/NBDataPtrsStats.h"
#include "nb/core/NBIOPollster.h"
#include "nb/core/NBIOPollstersProvider.h"
#include "nb/net/NBSdp.h"
#include "nb/net/NBRtpHeader.h"
#include "nb/net/NBRtsp.h"
#include "nb/net/NBRtspClientConn.h"
#include "nb/net/NBRtspClientStats.h"
#include "nb/net/NBRtpClient.h"
//
#include "nb/core/NBString.h"
#include "nb/core/NBLog.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef enum ENNBRtspRessState_ {
		ENNBRtspRessState_Stopped = 0,	//equivalent to TEARDOWN
		ENNBRtspRessState_Options,		//equivalent to OPTIONS
		ENNBRtspRessState_Described,	//equivalent to OPTIONS + DESCRIBE
		ENNBRtspRessState_Paused,		//equivalent to OPTIONS + DESCRIBE + SETUP [+ PAUSE if PLAYING]
		ENNBRtspRessState_Playing,		//equivalent to OPTIONS + DESCRIBE + SETUP + PLAY
		//Count
		ENNBRtspRessState_Count
	} ENNBRtspRessState;

	//CfgRtp

	typedef struct STNBRtspClientCfgRtp_ {
		BOOL			isDisabled;	//isDisabled
		STNBRtpCfg		server;
	} STNBRtspClientCfgRtp;

	const STNBStructMap* NBRtspClientCfgRtp_getSharedStructMap(void);

	//CfgRtcp

	typedef struct STNBRtspClientCfgRtcp_ {
		BOOL			isDisabled;	//isDisabled
		UI32			port;		//incoming port
	} STNBRtspClientCfgRtcp;

	const STNBStructMap* NBRtspClientCfgRtcp_getSharedStructMap(void);

	//CfgRtsp

	typedef struct STNBRtspClientCfg_ {
		BOOL					isDisabled;		//isDisabled
		STNBRtspCfgRequests		requests;		//requests config
		STNBRtspClientCfgRtp	rtp;			//rtp local server
		STNBRtspClientCfgRtcp	rtcp;			//rtcp local server
	} STNBRtspClientCfg;

	const STNBStructMap* NBRtspClientCfg_getSharedStructMap(void);

    //CfgRessClientRtp

    typedef struct STNBRtspClientCfgStreamClientRtp_ {
        UI32    port;
        UI32    assumeConnDroppedAfterSecs;
    } STNBRtspClientCfgStreamClientRtp;

    const STNBStructMap* NBRtspClientCfgStreamClientRtp_getSharedStructMap(void);

    //CfgRessClientRtcp

    typedef struct STNBRtspClientCfgStreamClientRtcp_ {
        UI32    port;
    } STNBRtspClientCfgStreamClientRtcp;

    const STNBStructMap* NBRtspClientCfgStreamClientRtcp_getSharedStructMap(void);

    //CfgRessClient

    typedef struct STNBRtspClientCfgStreamClient_ {
        STNBRtspClientCfgStreamClientRtp    rtp;
        STNBRtspClientCfgStreamClientRtcp   rtcp;
    } STNBRtspClientCfgStreamClient;

    const STNBStructMap* NBRtspClientCfgStreamClient_getSharedStructMap(void);

	//CfgRess

	typedef struct STNBRtspClientCfgStream_ {
		ENNBRtspClientConnRedirMode redirMode;
		char*		server;
		UI32		port;
		BOOL		useSSL;
        STNBRtspClientCfgStreamClient client;
		//
		char*		user;
		UI32		userSz;
		char*		pass;
		UI32		passSz;
	} STNBRtspClientCfgStream;

    const STNBStructMap* NBRtspClientCfgStream_getSharedStructMap(void);

	//----------------
	//- NBRtspCmdState
	//----------------

	typedef struct STNBRtspCmdState_ {
		UI32				seq;
		BOOL				isPend;		//zero means command was completed
		STNBRtpCmdState		rtp;		//
	} STNBRtspCmdState;

	void NBRtspCmdState_init(STNBRtspCmdState* obj);
	void NBRtspCmdState_release(STNBRtspCmdState* obj);


	//---------------
	//- RTSP v1
	//- RFC: https://tools.ietf.org/html/rfc2326
	//---------------

	typedef struct STNBRtspClientResLstnr_ {
		void*	obj;
		void	(*resRtspStateChanged)(void* obj, const char* uri, const ENNBRtspRessState state);
		void	(*resRtpConsumeReqResult)(void* obj, const char* uri, const char* method, ENNBRtspClientConnReqStatus status);
		void	(*resRtpConsumeOptions)(void* obj, const char* uri, const STNBRtspOptions* options);
		void	(*resRtpConsumeSessionDesc)(void* obj, const char* uri, const STNBSdpDesc* desc);
		void	(*resRtpConsumeSetup)(void* obj, const char* uri, const STNBRtspSetup* setup, const UI16 ssrcId);
		BOOL	(*resRtpIsBorderPresent)(void* obj, const char* uri, const STNBRtpHdrBasic* hdrs, const STNBDataPtr* chunks, const UI32 chunksSz);
		void	(*resRtpConsumePackets)(void* obj, const char* uri, STNBRtpHdrBasic* hdrs, STNBDataPtr* chunks, const UI32 chunksSz, const UI64 curUtcTime, STNBDataPtrReleaser* optPtrsReleaser);
		void	(*resRtpFlushStats)(void* obj);
	} STNBRtspClientResLstnr;

	typedef struct STNBRtspClient_ {
		void*		opaque;
	} STNBRtspClient;
	
	//Reference
	//BOOL NBRtspClient_refIsSet(const STNBRtspClient* obj);
	//void NBRtspClient_refSet(STNBRtspClient* obj, const STNBRtspClient* other);
	//void NBRtspClient_refUnset(STNBRtspClient* obj);

	//Factory
	void NBRtspClient_init(STNBRtspClient* obj);
	void NBRtspClient_retain(STNBRtspClient* obj);
	void NBRtspClient_release(STNBRtspClient* obj);

	//Config
	BOOL NBRtspClient_setCfg(STNBRtspClient* obj, const STNBRtspClientCfg* cfg);
	BOOL NBRtspClient_setPollster(STNBRtspClient* obj, STNBIOPollsterRef pollster, STNBIOPollsterSyncRef pollSync);	//when one pollster only
	BOOL NBRtspClient_setPollstersProvider(STNBRtspClient* obj, STNBIOPollstersProviderRef provider); //when multiple pollsters
	BOOL NBRtspClient_setStatsProvider(STNBRtspClient* obj, STNBRtspClientStats* stats);
	BOOL NBRtspClient_setExternalBuffers(STNBRtspClient* obj, STNBDataPtrsPoolRef pool);
	
	//Resources
	BOOL NBRtspClient_addResource(STNBRtspClient* obj, const char* uri, const STNBRtspClientResLstnr* lstnr, const STNBRtspClientCfgStream* cfg, const ENNBRtspRessState state);
	BOOL NBRtspClient_removeResource(STNBRtspClient* obj, const char* uri);
	BOOL NBRtspClient_setResourceState(STNBRtspClient* obj, const char* uri, const ENNBRtspRessState state);
    BOOL NBRtspClient_getResourceCfg(STNBRtspClient* obj, const char* uri, STNBRtspClientCfgStream* dst);
	void NBRtspClient_tickAnalyzeQuick(STNBRtspClient* obj, const UI32 onlyIfSecsElapsed);

	//Actions
	BOOL NBRtspClient_prepare(STNBRtspClient* obj);	//create buffers, internal servers, bind and more
	BOOL NBRtspClient_start(STNBRtspClient* obj);	//threads and binded sockets
	void NBRtspClient_stopFlag(STNBRtspClient* obj);
	BOOL NBRtspClient_isBusy(STNBRtspClient* obj);	//stream not cleaned-up

	//Commands
	void NBRtspClient_statsFlushStart(STNBRtspClient* obj, STNBRtspCmdState* dst);
	BOOL NBRtspClient_statsFlushIsPend(STNBRtspClient* obj, STNBRtspCmdState* dst);
	void NBRtspClient_statsGet(STNBRtspClient* obj, STNBArray* dstPortsStats /*STNBRtpClientPortStatsData*/, STNBArray* dstSsrcsStats /*STNBRtpClientSsrcStatsData*/, const BOOL resetAccum);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
