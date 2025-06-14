
#ifndef NB_RTSPCLIENTCONN_H
#define NB_RTSPCLIENTCONN_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBString.h"
#include "nb/core/NBThread.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBThreadCond.h"
#include "nb/core/NBStopFlag.h"
#include "nb/core/NBIOPollster.h"
#include "nb/core/NBThreadsPool.h"
#include "nb/net/NBRtsp.h"
#include "nb/net/NBHttpRequest.h"
#include "nb/net/NBHttpResponse.h"
#include "nb/net/NBSocket.h"
#include "nb/net/NBUrl.h"
#include "nb/ssl/NBSsl.h"
#include "nb/ssl/NBSslContext.h"
//

#ifdef __cplusplus
extern "C" {
#endif

	//NBRtspCfgTimeout

	typedef struct STNBRtspCfgTimeouts_ {
		UI32	retries;	//amount of additional attempts
		UI32	msConnect;	//connect timeout
	} STNBRtspCfgTimeouts;

	const STNBStructMap* NBRtspCfgTimeouts_getSharedStructMap(void);

	//NBRtspCfgTimeoutGrp

	typedef struct STNBRtspCfgTimeoutsGrp_ {
		STNBRtspCfgTimeouts	min; //when quick responsiveness is required
		STNBRtspCfgTimeouts max; //normal operation
	} STNBRtspCfgTimeoutsGrp;

	const STNBStructMap* NBRtspCfgTimeoutsGrp_getSharedStructMap(void);

	//NBRtspCfgRequests

	typedef struct STNBRtspCfgRequests_ {
		STNBRtspCfgTimeoutsGrp timeouts;	//timeouts
	} STNBRtspCfgRequests;

	const STNBStructMap* NBRtspCfgRequests_getSharedStructMap(void);

	//---------------
	//- RTSP v1
	//- RFC: https://tools.ietf.org/html/rfc2326
	//---------------

	typedef enum ENNBRtspClientConnRedirMode_ {
		ENNBRtspClientConnRedirMode_None = 0,		//Run only once
		ENNBRtspClientConnRedirMode_FollowAllways,	//Follow ALL redirects automatically
	} ENNBRtspClientConnRedirMode;

    const STNBEnumMap* ENNBRtspClientConnRedirMode_getSharedEnumMap(void);

    //

	typedef enum ENNBRtspClientConnReqStatus_ {
		ENNBRtspClientConnReqStatus_Undef = 0,
		ENNBRtspClientConnReqStatus_Pending,	//At queue
		ENNBRtspClientConnReqStatus_Sending,	//Sending header
		ENNBRtspClientConnReqStatus_Sent,		//Header sent
		ENNBRtspClientConnReqStatus_Receiving,	//Receiving response
		ENNBRtspClientConnReqStatus_Success,	//Code 200-299
		ENNBRtspClientConnReqStatus_Error
	} ENNBRtspClientConnReqStatus;
	
	//RequestParam

	typedef struct STNBRtspClientConnReqParam_ {
		const char*		name;
		const char*		value;
	} STNBRtspClientConnReqParam;
	
	const STNBStructMap* NBRtspClientConnReqParam_getSharedStructMap(void);

	//RequestParams

	typedef struct STNBRtspClientConnReqParams_ {
		STNBRtspClientConnReqParam* params;
		UI32						paramsSz;
	} STNBRtspClientConnReqParams;

	const STNBStructMap* NBRtspClientConnReqParams_getSharedStructMap(void);

	//Config

	typedef struct STNBRtspClientConnConfig_ {
		ENNBRtspClientConnRedirMode redirMode;
		char*		server;
		UI16		port;
		BOOL		useSSL;
		//
		char*		user;
		UI32		userSz;
		char*		pass;
		UI32		passSz;
	} STNBRtspClientConnConfig;

	//

	typedef struct STNBRtspClientConnCallback_ {
		void*	obj;
		void	(*reqResult)(void* obj, const UI32 uid, const STNBSocketAddr* remoteAddr, const char* method, const char* uri, const ENNBRtspClientConnReqStatus status, const STNBHttpResponse* response);
	} STNBRtspClientConnCallback;

	//

	NB_OBJREF_HEADER(NBRtspClientConn)
	
	//Config
	BOOL NBRtspClientConn_setServer(STNBRtspClientConnRef ref, const char* server, const UI16 port, const BOOL useSSL, const ENNBRtspClientConnRedirMode redirMode);
	BOOL NBRtspClientConn_setUserAndPass(STNBRtspClientConnRef ref, const char* user, const UI32 userSz, const char* pass, const UI32 passSz);
	BOOL NBRtspClientConn_setCallback(STNBRtspClientConnRef ref, const STNBRtspClientConnCallback* callback);

	//Actions
	BOOL NBRtspClientConn_startListening(STNBRtspClientConnRef ref, STNBIOPollsterSyncRef pollSync);
	BOOL NBRtspClientConn_startReq(STNBRtspClientConnRef ref, const char* method, const char* uri, const char* sessionId, const SI32 rtpPort, const SI32 rtcpPort, const STNBRtspClientConnReqParam* params, const UI32 paramsSz, const STNBRtspCfgTimeouts* timeouts);
	void NBRtspClientConn_stopListening(STNBRtspClientConnRef ref);
	BOOL NBRtspClientConn_isBusy(STNBRtspClientConnRef ref);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
