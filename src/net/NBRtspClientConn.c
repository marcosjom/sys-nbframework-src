
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBRtspClientConn.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBNumParser.h"
#include "nb/core/NBMngrStructMaps.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBIOPollster.h"
#include "nb/net/NBSocket.h"
#include "nb/net/NBHttpBuilder.h"
#include "nb/net/NBHttpResponse.h"
#include "nb/crypto/NBMd5.h"
#include "nb/crypto/NBBase64.h"

//ENNBRtspClientConnRedirMode

STNBEnumMapRecord ENNBRtspClientConnRedirMode_sharedEnumMapRecs[] = {
    { ENNBRtspClientConnRedirMode_None, "ENNBRtspClientConnRedirMode_None", "none" }
    , { ENNBRtspClientConnRedirMode_FollowAllways, "ENNBRtspClientConnRedirMode_FollowAllways", "followAllways"}
};

STNBEnumMap ENNBRtspClientConnRedirMode_sharedEnumMap = {
    "ENNBRtspClientConnRedirMode"
    , ENNBRtspClientConnRedirMode_sharedEnumMapRecs
    , (sizeof(ENNBRtspClientConnRedirMode_sharedEnumMapRecs) / sizeof(ENNBRtspClientConnRedirMode_sharedEnumMapRecs[0]))
};

const STNBEnumMap* ENNBRtspClientConnRedirMode_getSharedEnumMap(void){
    return &ENNBRtspClientConnRedirMode_sharedEnumMap;
}

//NBRtspClientConnReqParam

STNBStructMapsRec NBRtspClientConnReqParam_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtspClientConnReqParam_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBRtspClientConnReqParam_sharedStructMap);
	if(NBRtspClientConnReqParam_sharedStructMap.map == NULL){
		STNBRtspClientConnReqParam s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtspClientConnReqParam);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addStrPtrM(map, s, name);
		NBStructMap_addStrPtrM(map, s, value);
		NBRtspClientConnReqParam_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBRtspClientConnReqParam_sharedStructMap);
	return NBRtspClientConnReqParam_sharedStructMap.map;
}

//NBRtspClientConnReqParams

STNBStructMapsRec NBRtspClientConnReqParams_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtspClientConnReqParams_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBRtspClientConnReqParams_sharedStructMap);
	if(NBRtspClientConnReqParams_sharedStructMap.map == NULL){
		STNBRtspClientConnReqParams s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtspClientConnReqParams);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addPtrToArrayOfStructM(map, s, params, paramsSz, ENNBStructMapSign_Unsigned, NBRtspClientConnReqParam_getSharedStructMap());
		NBRtspClientConnReqParams_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBRtspClientConnReqParams_sharedStructMap);
	return NBRtspClientConnReqParams_sharedStructMap.map;
}

//NBRtspCfgTimeouts

STNBStructMapsRec STNBRtspCfgTimeouts_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtspCfgTimeouts_getSharedStructMap(void){
	NBMngrStructMaps_lock(&STNBRtspCfgTimeouts_sharedStructMap);
	if(STNBRtspCfgTimeouts_sharedStructMap.map == NULL){
		STNBRtspCfgTimeouts s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtspCfgTimeouts);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addUIntM(map, s, retries);		//amount of additional attempts
		NBStructMap_addUIntM(map, s, msConnect);	//normal operation
		STNBRtspCfgTimeouts_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&STNBRtspCfgTimeouts_sharedStructMap);
	return STNBRtspCfgTimeouts_sharedStructMap.map;
}

//NBRtspCfgTimeoutsGrp

STNBStructMapsRec STNBRtspCfgTimeoutsGrp_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtspCfgTimeoutsGrp_getSharedStructMap(void){
	NBMngrStructMaps_lock(&STNBRtspCfgTimeoutsGrp_sharedStructMap);
	if(STNBRtspCfgTimeoutsGrp_sharedStructMap.map == NULL){
		STNBRtspCfgTimeoutsGrp s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtspCfgTimeoutsGrp);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addStructM(map, s, min, NBRtspCfgTimeouts_getSharedStructMap());
		NBStructMap_addStructM(map, s, max, NBRtspCfgTimeouts_getSharedStructMap());
		STNBRtspCfgTimeoutsGrp_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&STNBRtspCfgTimeoutsGrp_sharedStructMap);
	return STNBRtspCfgTimeoutsGrp_sharedStructMap.map;
}

//NBRtspCfgRequests

STNBStructMapsRec STNBRtspCfgRequests_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRtspCfgRequests_getSharedStructMap(void){
	NBMngrStructMaps_lock(&STNBRtspCfgRequests_sharedStructMap);
	if(STNBRtspCfgRequests_sharedStructMap.map == NULL){
		STNBRtspCfgRequests s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRtspCfgRequests);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addStructM(map, s, timeouts, NBRtspCfgTimeoutsGrp_getSharedStructMap()); //timeouts
		STNBRtspCfgRequests_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&STNBRtspCfgRequests_sharedStructMap);
	return STNBRtspCfgRequests_sharedStructMap.map;
}

//Request

typedef struct STNBRtspClientConnReqParamI_ {
	char*		name;
	char*		value;
} STNBRtspClientConnReqParamI;

typedef struct STNBRtspClientConnReq_ {
	UI32						uid;		//unique-internal-id
	char*						method;		//method
	char*						uri;		//resource uri
	char*						sessionId;	//sessionId
	STNBArray					params;		//STNBRtspClientConnReqParamI
	STNBRtspCfgTimeouts			timeouts;
	UI32						retryCount;	//retry count
	//out
	struct {
		STNBString				buff;		//buffers
		UI32					iPos;		//position
	} out;
	//in
	struct {
		STNBHttpResponse		parser;
	} in;
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	//dbg
	struct {
		//time
		struct {
			UI64				creation;
			UI64				start;
		} timestamp;
	} dbg;
#	endif
} STNBRtspClientConnReq;

void NBRtspClientConnReq_init(STNBRtspClientConnReq* obj);
void NBRtspClientConnReq_release(STNBRtspClientConnReq* obj);
//
void NBRtspClientConnReq_resetParser(STNBRtspClientConnReq* obj);

//Opaque state

typedef struct STNBRtspClientConnOpq_ {
	STNBObject					prnt;
	STNBRtspClientConnConfig	cfg;
	STNBRtspClientConnCallback	callback;
	//pollster
	struct {
		STNBIOPollsterSyncRef	sync;
		BOOL					isListening;
        STNBStopFlagRef			stopFlag;
	} pollster;
	//reqs
	struct {
		UI32					uidSeq;	//global uids
		STNBArray				queue;	//STNBRtspClientConnReq*
		STNBRtspClientConnReq*	cur;	//current
	} reqs;
	//state
	struct {
		UI32				cSeq;
		UI32				bytesSent;
		UI32				bytesReceived;
		//socket
		struct {
			STNBSocketRef	ref;
			STNBString		server;
			UI32			port;
			BOOL			useSsl;
			BOOL			isUsed;			//socket was used and must be closed to recconnect
			UI64			timeConnecting;	//start of connecting state (zero if not connnecting)
			UI64			timeConnected;	//start of connected state (zero if not connnected)
		} socket;
		//buffs
		struct {
			//in (always the first request)
			struct {
				char*		data;	//input buffer
				UI32		size;	//input buffer size
			} in;
		} buffs;
		//authBase
		struct {
			char*			type;
			char*			realmHex;
			char*			nonceHex;
		} authBase;
	} state;
} STNBRtspClientConnOpq;

NB_OBJREF_BODY(NBRtspClientConn, STNBRtspClientConnOpq, NBObject)

//pollster callbacks
void NBRtspClientConn_pollConsumeMask_(STNBIOLnk ioLnk, const UI8 pollMask, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData);
void NBRtspClientConn_pollConsumeNoOp_(STNBIOLnk ioLnk, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData);
void NBRtspClientConn_pollRemoved_(STNBIOLnk ioLnk, void* usrData);

//Request
void NBRtspClientConn_reqBuildStrLockedOpq_(STNBRtspClientConnOpq* opq, STNBRtspClientConnReq* req);
void NBRtspClientConn_reqBuildStrOpq_(STNBRtspClientConnOpq* opq, STNBRtspClientConnReq* req);
void NBRtspClientConn_reqNetError_(STNBRtspClientConnOpq* opq, const BOOL allowRetry, const char* dbgDesc);
void NBRtspClientConn_reqRemoteError_(STNBRtspClientConnOpq* opq, const BOOL allowRetry, const char* dbgDesc);
void NBRtspClientConn_reqOldestNotifyResultAndRemoveOpq_(STNBRtspClientConnOpq* opq, const ENNBRtspClientConnReqStatus status, const char* dbgDesc);
BOOL NBRtspClientConn_reqGetAuthBaseFromResponse_(STNBRtspClientConnOpq* opq, const STNBHttpResponse* resp);
BOOL NBRtspClientConn_reqConcatAuthDigestResponseHash_(const char* realmHex, const char* nonceHex, const char* user, const UI32 userSz, const char* pass, const UI32 passSz, const char* method, const char* uri, STNBString* dst);

//Factory

void NBRtspClientConn_initZeroed(STNBObject* obj){
	STNBRtspClientConnOpq* opq = (STNBRtspClientConnOpq*)obj;
    //pollster
    {
        opq->pollster.stopFlag = NBStopFlag_alloc(NULL);
    }
	//Request
	{
		NBArray_init(&opq->reqs.queue, sizeof(STNBRtspClientConnReq*), NULL);
	}
	//state
	{
		//socket
		{
			//ref
			{
				opq->state.socket.ref = NBSocket_alloc(NULL);
				NBSocket_createHnd(opq->state.socket.ref);
				NBSocket_setNoSIGPIPE(opq->state.socket.ref, TRUE);
				NBSocket_setCorkEnabled(opq->state.socket.ref, FALSE);
				NBSocket_setDelayEnabled(opq->state.socket.ref, FALSE);
				NBSocket_setNonBlocking(opq->state.socket.ref, TRUE);
				NBSocket_setUnsafeMode(opq->state.socket.ref, TRUE);
			}
			NBString_init(&opq->state.socket.server);
		}
		//buffs
		{
			//in
			{
				opq->state.buffs.in.size = 4096;
				opq->state.buffs.in.data = NBMemory_alloc(opq->state.buffs.in.size);
			}
		}
		//authBase
		{
			//nothing
		}
	}
}

void NBRtspClientConn_uninitLocked(STNBObject* obj){
	STNBRtspClientConnOpq* opq = (STNBRtspClientConnOpq*)obj;
	//state
	{
		//socket
		{
			//ref
			{
				//remove from pollster
				if(opq->pollster.isListening){
					if(NBIOPollsterSync_isSet(opq->pollster.sync)){
						NBIOPollsterSync_removeSocket(opq->pollster.sync, opq->state.socket.ref);
					}
					opq->pollster.isListening = FALSE;
				}
				NBSocket_close(opq->state.socket.ref);
				NBSocket_release(&opq->state.socket.ref);
				NBSocket_null(&opq->state.socket.ref);
			}
			NBString_release(&opq->state.socket.server);
		}
		//buffs
		{
			//in
			{
				if(opq->state.buffs.in.data != NULL){
					NBMemory_free(opq->state.buffs.in.data);
					opq->state.buffs.in.data = NULL;
					opq->state.buffs.in.size = 0;
				}
			}
		}
		//authBase
		{
			if(opq->state.authBase.type != NULL){
				NBMemory_free(opq->state.authBase.type);
				opq->state.authBase.type = NULL;
			}
			if(opq->state.authBase.realmHex != NULL){
				NBMemory_free(opq->state.authBase.realmHex);
				opq->state.authBase.realmHex = NULL;
			}
			if(opq->state.authBase.nonceHex != NULL){
				NBMemory_free(opq->state.authBase.nonceHex);
				opq->state.authBase.nonceHex = NULL;
			}
		}
	}
	//reqs
	{
		const SI32 count = opq->reqs.queue.use;
		SI32 i; for(i = 0; i < count; i++){
			STNBRtspClientConnReq* req = NBArray_itmValueAtIndex(&opq->reqs.queue, STNBRtspClientConnReq*, i);
			if(req->method != NULL){
				NBMemory_free(req->method);
				req->method	= NULL;
			}
			NBRtspClientConnReq_release(req);
			NBMemory_free(req);
		}
		NBArray_empty(&opq->reqs.queue);
		NBArray_release(&opq->reqs.queue);
	}
	//cfg
	{
		if(opq->cfg.server != NULL){
			NBMemory_free(opq->cfg.server);
			opq->cfg.server = NULL;
		}
		if(opq->cfg.user != NULL){
			NBMemory_free(opq->cfg.user);
			opq->cfg.user = NULL;
			opq->cfg.userSz = 0;
		}
		if(opq->cfg.pass != NULL){
			NBMemory_free(opq->cfg.pass);
			opq->cfg.pass = NULL;
			opq->cfg.passSz = 0;
		}
	}
	//pollster
    {
        if(NBIOPollsterSync_isSet(opq->pollster.sync)){
            NBIOPollsterSync_release(&opq->pollster.sync);
            NBIOPollsterSync_null(&opq->pollster.sync);
        }
        if(NBStopFlag_isSet(opq->pollster.stopFlag)){
            NBStopFlag_release(&opq->pollster.stopFlag);
            NBStopFlag_null(&opq->pollster.stopFlag);
        }
    }
}

//Config

BOOL NBRtspClientConn_startListening(STNBRtspClientConnRef ref, STNBIOPollsterSyncRef pollSync){
	BOOL r = FALSE;
	STNBRtspClientConnOpq* opq = (STNBRtspClientConnOpq*)ref.opaque; NBASSERT(NBRtspClientConn_isClass(ref))
	NBASSERT(opq != NULL)
	NBObject_lock(opq);
	if(opq->state.socket.timeConnected == 0 && opq->state.socket.timeConnecting == 0){
		if(!NBIOPollsterSync_isSame(opq->pollster.sync, pollSync)){
			//remove from pollster
			if(opq->pollster.isListening){
				if(NBIOPollsterSync_isSet(opq->pollster.sync)){
					NBIOPollsterSync_removeSocket(opq->pollster.sync, opq->state.socket.ref);
				}
				opq->pollster.isListening = FALSE;
			}
			//set
			NBIOPollsterSync_set(&opq->pollster.sync, &pollSync);
			//add to pollster
			if(NBIOPollsterSync_isSet(opq->pollster.sync)){
				STNBIOPollsterLstrnItf itf;
				NBMemory_setZeroSt(itf, STNBIOPollsterLstrnItf);
				itf.pollConsumeMask = NBRtspClientConn_pollConsumeMask_;
				itf.pollConsumeNoOp = NBRtspClientConn_pollConsumeNoOp_;
				itf.pollRemoved		= NBRtspClientConn_pollRemoved_;
				if(!NBIOPollsterSync_addSocketWithItf(opq->pollster.sync, opq->state.socket.ref, ENNBIOPollsterOpBit_Read, &itf, opq)){
					opq->pollster.isListening = FALSE;
				} else {
					r = opq->pollster.isListening = TRUE;
				}
			}
		}
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBRtspClientConn_setServer(STNBRtspClientConnRef ref, const char* server, const UI16 port, const BOOL useSSL, const ENNBRtspClientConnRedirMode redirMode){
	BOOL r = FALSE;
	STNBRtspClientConnOpq* opq = (STNBRtspClientConnOpq*)ref.opaque; NBASSERT(NBRtspClientConn_isClass(ref))
	NBASSERT(opq != NULL)
	NBObject_lock(opq);
	if(opq->state.socket.timeConnected == 0 && opq->state.socket.timeConnecting == 0){
		//Set config
		opq->cfg.redirMode	= redirMode;
		opq->cfg.port		= port;
		opq->cfg.useSSL		= useSSL;
		NBString_strFreeAndNewBuffer(&opq->cfg.server, server);
		r = TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBRtspClientConn_setUserAndPass(STNBRtspClientConnRef ref, const char* user, const UI32 userSz, const char* pass, const UI32 passSz){
	BOOL r = FALSE;
	STNBRtspClientConnOpq* opq = (STNBRtspClientConnOpq*)ref.opaque; NBASSERT(NBRtspClientConn_isClass(ref))
	NBASSERT(opq != NULL)
	NBObject_lock(opq);
	if(opq->state.socket.timeConnected == 0 && opq->state.socket.timeConnecting == 0){
		if(opq->cfg.user != NULL){
			NBMemory_free(opq->cfg.user);
			opq->cfg.user	= NULL;
			opq->cfg.userSz	= 0;
		}
		if(opq->cfg.pass != NULL){
			NBMemory_free(opq->cfg.pass);
			opq->cfg.pass	= NULL;
			opq->cfg.passSz	= 0;
		}
		if(user != NULL && userSz > 0){
			opq->cfg.user	= NBString_strNewBufferBytes(user, userSz); 
			opq->cfg.userSz	= userSz;
		}
		if(pass != NULL && passSz > 0){
			opq->cfg.pass	= NBString_strNewBufferBytes(pass, passSz); 
			opq->cfg.passSz	= passSz;
		}
		r = TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBRtspClientConn_setCallback(STNBRtspClientConnRef ref, const STNBRtspClientConnCallback* callback){
	BOOL r = FALSE;
	STNBRtspClientConnOpq* opq = (STNBRtspClientConnOpq*)ref.opaque; NBASSERT(NBRtspClientConn_isClass(ref))
	NBASSERT(opq != NULL)
	NBObject_lock(opq);
	{
		if(callback == NULL){
			NBMemory_setZeroSt(opq->callback, STNBRtspClientConnCallback);
		} else {
			opq->callback = *callback; 
		}
	}
	NBObject_unlock(opq);
	return r;
}

//Actions
		
BOOL NBRtspClientConn_startReq(STNBRtspClientConnRef ref, const char* method, const char* uri, const char* sessionId, const SI32 rtpPort, const SI32 rtcpPort, const STNBRtspClientConnReqParam* params, const UI32 paramsSz, const STNBRtspCfgTimeouts* timeouts){
	BOOL r = FALSE;
	STNBRtspClientConnOpq* opq = (STNBRtspClientConnOpq*)ref.opaque; NBASSERT(NBRtspClientConn_isClass(ref))
	//Add request
	NBObject_lock(opq);
	{
		BOOL transportParamFnd = FALSE;
		STNBRtspClientConnReq* req = NBMemory_allocType(STNBRtspClientConnReq);
		NBRtspClientConnReq_init(req);
		req->uid		= ++opq->reqs.uidSeq;
		req->method		= NBString_strNewBuffer(method);
		req->uri		= NBString_strNewBuffer(uri);
		if(!NBString_strIsEmpty(sessionId)){
			req->sessionId = NBString_strNewBuffer(sessionId);
		}
		if(params != NULL && paramsSz > 0){
			UI32 i; for(i = 0; i < paramsSz; i++){
				const STNBRtspClientConnReqParam* p = &params[i];
				STNBRtspClientConnReqParamI pp;
				NBMemory_setZeroSt(pp, STNBRtspClientConnReqParamI);
				if(p->name != NULL){
					pp.name = NBString_strNewBuffer(p->name);
					if(NBString_strIsLike(p->name, "Transport")){
						transportParamFnd = TRUE;
					}
				}
				if(p->value != NULL){
					pp.value = NBString_strNewBuffer(p->value);
				}
				NBArray_addValue(&req->params, pp);
			}
		}
		//Add timeouts
		if(timeouts != NULL){
			NBStruct_stClone(NBRtspCfgTimeouts_getSharedStructMap(), timeouts, sizeof(*timeouts), &req->timeouts, sizeof(req->timeouts));
		}
		//Add 'Transport' param to SETUP
		if(NBString_strIsLike(method, "SETUP")){
			if(!transportParamFnd){
				STNBString str;
				NBString_init(&str);
				//
				NBString_concat(&str, "RTP/AVP/UDP;unicast");
				//unicast port
				{
					NBString_concat(&str, ";client_port=");
					NBString_concatUI32(&str, rtpPort);
					if(rtcpPort > 0){
						NBString_concat(&str, "-");
						NBString_concatUI32(&str, rtcpPort);
					}
				}
				//Add "Transport" param
				{
					STNBRtspClientConnReqParamI pp;
					NBMemory_setZeroSt(pp, STNBRtspClientConnReqParamI);
					pp.name 	= NBString_strNewBuffer("Transport");
					pp.value	= str.str; NBString_resignToContent(&str);
					NBArray_addValue(&req->params, pp);
				}
				NBString_release(&str);
			}
		}
		//dbg
		{
			IF_NBASSERT(req->dbg.timestamp.creation = NBDatetime_getCurUTCTimestamp();)
		}
		NBArray_addValue(&opq->reqs.queue, req);
		r = TRUE;
	}
	NBObject_unlock(opq);
	NBASSERT(r)
	return r;
}

void NBRtspClientConn_stopListening(STNBRtspClientConnRef ref){
	STNBRtspClientConnOpq* opq = (STNBRtspClientConnOpq*)ref.opaque; NBASSERT(NBRtspClientConn_isClass(ref))
    NBStopFlag_activate(opq->pollster.stopFlag);
}

BOOL NBRtspClientConn_isBusy(STNBRtspClientConnRef ref){
	STNBRtspClientConnOpq* opq = (STNBRtspClientConnOpq*)ref.opaque; NBASSERT(NBRtspClientConn_isClass(ref))
	return opq->pollster.isListening;
}

//pollster callbacks

void NBRtspClientConn_pollConsumeMask_(STNBIOLnk ioLnk, const UI8 pollMask, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData){
	STNBRtspClientConnOpq* opq = (STNBRtspClientConnOpq*)usrData;
	IF_NBASSERT(STNBTimestampMicro timeLastAction = NBTimestampMicro_getMonotonicFast(), timeCur; SI64 usDiff;)
	//write
	if(pollMask & ENNBIOPollsterOpBit_Write){
		//connected
		if(opq->state.socket.timeConnecting != 0){
			opq->state.socket.timeConnecting	= 0;
			opq->state.socket.timeConnected		= NBDatetime_getCurUTCTimestamp();
		}
		//write
		if(opq->reqs.cur != NULL && opq->reqs.cur->out.iPos < opq->reqs.cur->out.buff.length){
			const int sent = NBIOLnk_write(&ioLnk, &opq->reqs.cur->out.buff.str[opq->reqs.cur->out.iPos], opq->reqs.cur->out.buff.length - opq->reqs.cur->out.iPos);
			if(sent >= 0){
				//PRINTF_INFO("NBRtspClientConn, sent %d of %d bytes.\n", sent, opq->reqs.cur->out.buff.length - opq->reqs.cur->out.iPos);
#				ifdef NB_CONFIG_INCLUDE_ASSERTS
				/*{
				 STNBString str;
				 NBString_initWithSz(&str, 4096, 4096, 0.10f);
				 {
				 const char* c = &opq->reqs.cur->out.buff.str[opq->reqs.cur->out.iPos];
				 const char* cAfterEnd = c + sent;  
				 while(c < cAfterEnd){
				 if(*c < 32){
				 NBString_concatByte(&str, '(');
				 NBString_concatUI32(&str, *c);
				 NBString_concatByte(&str, ')');
				 if(*c == 10){
				 NBString_concatByte(&str, *c);
				 }
				 } else {
				 NBString_concatByte(&str, *c);
				 }
				 c++;
				 }
				 PRINTF_INFO("NBRtspClientConn, sent:\n-->\n%s<--\n", str.str);
				 }
				 NBString_release(&str);
				 }*/
#				endif
				opq->reqs.cur->out.iPos += sent;
				opq->state.bytesSent += sent;
			} else {
				//socket error (retry or move-to-next)
				NBRtspClientConn_reqNetError_(opq, TRUE, "io-write-error"); //allow retry
			}
		}
	}
	//read
	if(pollMask & ENNBIOPollsterOpBit_Read && opq->state.socket.timeConnected != 0 && opq->reqs.cur != NULL){
		NBASSERT(opq->state.buffs.in.size != 0 && opq->state.buffs.in.data != NULL)
		if(opq->state.buffs.in.size == 0 || opq->state.buffs.in.data == NULL){
			//notify as error (buffer not initialized)
			NBRtspClientConn_reqOldestNotifyResultAndRemoveOpq_(opq, ENNBRtspClientConnReqStatus_Error, "buffs not ready");
			NBASSERT(FALSE)
		} else {
			int rcvd; STNBHttpResponse* resp = &opq->reqs.cur->in.parser;
			do {
				//reset read buffer
				rcvd = NBIOLnk_read(&ioLnk, opq->state.buffs.in.data, opq->state.buffs.in.size);
				if(rcvd < 0){
					//socket error (retry or move-to-next)
					NBRtspClientConn_reqNetError_(opq, TRUE, "io-read-error"); //allow retry
					break;
				} else {
					//PRINTF_INFO("NBRtspClientConn, rcvd %d bytes.\n", rcvd);
#					ifdef NB_CONFIG_INCLUDE_ASSERTS
                    /*{
                        STNBString str;
                        NBString_initWithSz(&str, 4096, 4096, 0.10f);
                        {
                            const char* c = (const char*)opq->state.buffs.in.data;
                            const char* cAfterEnd = c + rcvd;
                            while(c < cAfterEnd){
                                if(*c < 32){
                                    NBString_concatByte(&str, '(');
                                    NBString_concatUI32(&str, *c);
                                    NBString_concatByte(&str, ')');
                                    if(*c == 10){
                                        NBString_concatByte(&str, *c);
                                    }
                                } else {
                                    NBString_concatByte(&str, *c);
                                }
                                c++;
                            }
                            PRINTF_INFO("NBRtspClientConn, rcvd:\n-->\n%s<--\n", str.str);
                        }
                        NBString_release(&str);
                    }*/
#					endif
					opq->state.bytesReceived += rcvd;
					//feed
					if(!NBHttpResponse_consumeBytes(resp, opq->state.buffs.in.data, rcvd, "RTSP")){
						//socket error (retry or move-to-next)
						NBRtspClientConn_reqNetError_(opq, FALSE, "consume-bytes-error, stop"); //do-not-retry
						break;
					} else if(opq->reqs.cur->in.parser._transfEnded){
						//Analyze response
						if(opq->reqs.cur->retryCount == 0 && resp->code == 401){
							if(!NBRtspClientConn_reqGetAuthBaseFromResponse_(opq, resp)){
								//Base for authentication not found (cannot authenticate)
								PRINTF_ERROR("NBRtspClientConn, RTSP, response-code(%d) received and authHashResponse could not be calculated: '%s'.\n", resp->code, (opq->reqs.cur != NULL ? opq->reqs.cur->uri : "(none)"));
								NBRtspClientConn_reqRemoteError_(opq, TRUE, "auth-base-not-provided"); //allow retry
							} else {
								//Base for authentication found (retry with authentication)
                                PRINTF_INFO("NBRtspClientConn, response-code(%d), retrying with authentication: '%s'.\n", resp->code, (opq->reqs.cur != NULL ? opq->reqs.cur->uri : "(none)"));
								NBRtspClientConn_reqBuildStrOpq_(opq, opq->reqs.cur);
								NBRtspClientConnReq_resetParser(opq->reqs.cur);
							}
						} else if(resp->code < 200 || resp->code >= 300){
							PRINTF_ERROR("NBRtspClientConn, RTSP, response-code(%d) received: '%s'.\n", resp->code, (opq->reqs.cur != NULL ? opq->reqs.cur->uri : "(none)"));
							NBRtspClientConn_reqRemoteError_(opq, TRUE, "response-code-not-2XX"); //allow retry
						} else {
							//notify as success
							NBRtspClientConn_reqOldestNotifyResultAndRemoveOpq_(opq, ENNBRtspClientConnReqStatus_Success, NULL);
						}
						break;
					}
				}
			} while(TRUE);
		}
	}
	//error
	if(pollMask & ENNBIOPollsterOpBits_ErrOrGone){
		if(opq->state.socket.timeConnecting != 0){
			PRINTF_ERROR("NBRtspClientConn, socket error (after %lld secs connnecting): '%s'.\n", (NBDatetime_getCurUTCTimestamp() - opq->state.socket.timeConnecting), (opq->reqs.cur != NULL ? opq->reqs.cur->uri : "(none)"));
		} else if(opq->state.socket.timeConnected != 0){
			PRINTF_ERROR("NBRtspClientConn, socket error (after %lld secs connnected): '%s'.\n", (NBDatetime_getCurUTCTimestamp() - opq->state.socket.timeConnected), (opq->reqs.cur != NULL ? opq->reqs.cur->uri : "(none)"));
		} else {
			PRINTF_ERROR("NBRtspClientConn, socket error (while inactive): '%s'.\n", (opq->reqs.cur != NULL ? opq->reqs.cur->uri : "(none)"));
		}
		opq->state.socket.timeConnecting	= 0;
		opq->state.socket.timeConnected		= 0;
		if(opq->reqs.cur != NULL){
			//socket error (retry or move-to-next)
			NBRtspClientConn_reqNetError_(opq, TRUE, "socket-err-inside-poll"); //allow retry
		}
	}
	//activate next request (if necesary and available)
	if(opq->reqs.cur == NULL && opq->reqs.queue.use > 0){
		NBObject_lock(opq);
		{
			opq->reqs.cur = NBArray_itmValueAtIndex(&opq->reqs.queue, STNBRtspClientConnReq*, 0);
			NBRtspClientConn_reqBuildStrLockedOpq_(opq, opq->reqs.cur);
			//PRINTF_INFO("NBRtspClientConn, oldest request activated: '%s'.\n", (opq->reqs.cur != NULL ? opq->reqs.cur->uri : "(none)"));
		}
		NBObject_unlock(opq);
	}
	//connection timeout
	if(opq->state.socket.timeConnecting != 0 && opq->reqs.cur != NULL && opq->reqs.cur->timeouts.msConnect != 0){
		const UI64 time = NBDatetime_getCurUTCTimestamp();
		if((time - opq->state.socket.timeConnecting) >= (1 + (opq->reqs.cur->timeouts.msConnect / 1000))){
			PRINTF_ERROR("NBRtspClientConn, connection timeout after %lld secs: '%s'.\n", (time - opq->state.socket.timeConnecting), (opq->reqs.cur != NULL ? opq->reqs.cur->uri : "(none)"));
			opq->state.socket.timeConnecting = 0;
			//socket error (retry or move-to-next)
			NBRtspClientConn_reqNetError_(opq, TRUE, "conn-timeout"); //allow retry
		}
	}
	//connection start
	if(opq->state.socket.timeConnecting == 0 && opq->state.socket.timeConnected == 0 && opq->reqs.cur != NULL && opq->reqs.cur->out.iPos < opq->reqs.cur->out.buff.length){
		if(opq->state.socket.isUsed){
			opq->state.socket.isUsed = FALSE;
			//close handle
			{
				NBSocket_close(opq->state.socket.ref);
			}
			//create handle
			{
				NBSocket_createHnd(opq->state.socket.ref);
				NBSocket_setNoSIGPIPE(opq->state.socket.ref, TRUE);
				NBSocket_setCorkEnabled(opq->state.socket.ref, FALSE);
				NBSocket_setDelayEnabled(opq->state.socket.ref, FALSE);
				NBSocket_setNonBlocking(opq->state.socket.ref, TRUE);
				NBSocket_setUnsafeMode(opq->state.socket.ref, TRUE);
			}
			//flag as handle changed
			dstUpd->syncFd = TRUE;
		}
		//connect
		{
			opq->state.socket.isUsed = TRUE;
			opq->state.socket.timeConnecting = NBDatetime_getCurUTCTimestamp();
			NBSocket_connect(opq->state.socket.ref, opq->cfg.server, opq->cfg.port);
			//PRINTF_INFO("NBRtspClientConn, connecting to '%s:%d': '%s'\n", opq->cfg.server, opq->cfg.port, (opq->reqs.cur != NULL ? opq->reqs.cur->uri : "(none)"));
		}
	}
	//required mask
	dstUpd->opsMasks = (opq->reqs.cur != NULL ? ENNBIOPollsterOpBit_Read : 0) | (opq->state.socket.timeConnecting != 0 || (opq->state.socket.timeConnected != 0 && opq->reqs.cur != NULL && opq->reqs.cur->out.iPos < opq->reqs.cur->out.buff.length) ? ENNBIOPollsterOpBit_Write : 0);
	//consume stopFlag
	if(NBStopFlag_isAnyActivated(opq->pollster.stopFlag) && NBIOPollsterSync_isSet(dstSync)){
		NBASSERT(opq->pollster.isListening)
        NBIOPollsterSync_removeIOLnk(dstSync, &ioLnk);
	}
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	{
		timeCur	= NBTimestampMicro_getMonotonicFast();
		usDiff	= NBTimestampMicro_getDiffInUs(&timeLastAction, &timeCur);
		/*if(usDiff >= 1000ULL){
			PRINTF_INFO("NBRtspClientConn, pollConsumeMask(%s%s%s) took %llu.%llu%llums.\n", (pollMask & ENNBIOPollsterOpBit_Read ? "+read" : ""), (pollMask & ENNBIOPollsterOpBit_Write ? "+write" : ""), (pollMask & ENNBIOPollsterOpBits_ErrOrGone ? "+errOrGone" : ""), (usDiff / 1000ULL), (usDiff % 1000ULL) % 100ULL, (usDiff % 100ULL) % 10ULL);
		}*/
		timeLastAction = timeCur;
	}
#	endif
}

void NBRtspClientConn_pollConsumeNoOp_(STNBIOLnk ioLnk, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData){
	NBRtspClientConn_pollConsumeMask_(ioLnk, 0, dstUpd, dstSync, usrData);
}

void NBRtspClientConn_pollRemoved_(STNBIOLnk ioLnk, void* usrData){
	STNBRtspClientConnOpq* opq = (STNBRtspClientConnOpq*)usrData;
	opq->pollster.isListening = FALSE;
	//PRINTF_INFO("NBRtspClientConn, safetly-removed from pollster ('%s:%d').\n", opq->cfg.server, opq->cfg.port);
}

//

void NBRtspClientConn_reqBuildStrLockedOpq_(STNBRtspClientConnOpq* opq, STNBRtspClientConnReq* req){
	STNBString tmp, tmp2;
	NBString_initWithSz(&tmp, 0, 256, 0.1f);
	NBString_initWithSz(&tmp2, 0, 256, 0.1f);
	{
		STNBString* dst = &req->out.buff; req->out.iPos = 0;
		STNBHttpBuilder bldr;
		NBHttpBuilder_init(&bldr);
		//empty
		{
			NBString_empty(dst);
		}
		//request-line
		{
			NBHttpBuilder_addRequestLineWithProtocol(&bldr, dst, "RTSP", req->method, req->uri, 1, 0);
		}
		//CSeq
		{
			NBString_empty(&tmp); NBString_concatSI32(&tmp, ++opq->state.cSeq);
			NBHttpBuilder_addHeaderField(&bldr, dst, "CSeq", tmp.str);
		}
		{
			//Calculate and add authorization
			if(NBString_strIsLike(opq->state.authBase.type, "Basic")){
				if(!NBString_strIsEmpty(opq->state.authBase.realmHex)){
					NBString_empty(&tmp);
					NBString_empty(&tmp2);
					NBString_concatBytes(&tmp, opq->cfg.user, opq->cfg.userSz);
					NBString_concatByte(&tmp, ':');
					NBString_concatBytes(&tmp, opq->cfg.pass, opq->cfg.passSz);
					{
						NBString_concat(&tmp2, "Basic ");
						NBBase64_codeBytes(&tmp2, tmp.str, tmp.length);
						//Add header
						NBHttpBuilder_addHeaderField(&bldr, dst, "Authorization", tmp2.str);
					}
				}
			} else if(NBString_strIsLike(opq->state.authBase.type, "Digest")){
				if(!NBString_strIsEmpty(opq->state.authBase.realmHex) || !NBString_strIsEmpty(opq->state.authBase.nonceHex)){
					NBString_empty(&tmp);
					NBString_empty(&tmp2);
					if(NBRtspClientConn_reqConcatAuthDigestResponseHash_(opq->state.authBase.realmHex, opq->state.authBase.nonceHex, opq->cfg.user, opq->cfg.userSz, opq->cfg.pass, opq->cfg.passSz, req->method, req->uri, &tmp2)){
						if(tmp2.length > 0){
							NBString_concat(&tmp, "Digest");
							NBString_concat(&tmp, " username=\""); NBString_concatBytes(&tmp, opq->cfg.user, opq->cfg.userSz); NBString_concat(&tmp, "\"");
							NBString_concat(&tmp, ", realm=\""); NBString_concat(&tmp, opq->state.authBase.realmHex); NBString_concat(&tmp, "\"");
							NBString_concat(&tmp, ", nonce=\""); NBString_concat(&tmp, opq->state.authBase.nonceHex); NBString_concat(&tmp, "\"");
							NBString_concat(&tmp, ", uri=\""); NBString_concat(&tmp, req->uri); NBString_concat(&tmp, "\"");
							NBString_concat(&tmp, ", response=\""); NBString_concat(&tmp, tmp2.str); NBString_concat(&tmp, "\"");
							//Add header
							NBHttpBuilder_addHeaderField(&bldr, dst, "Authorization", tmp.str);
						}
					}
				}
			}
			//Add session
			if(!NBString_strIsEmpty(req->sessionId)){
				NBHttpBuilder_addHeaderField(&bldr, dst, "Session", req->sessionId);
			}
		}
		//Params
		{
			SI32 i; for(i = 0; i < req->params.use; i++){
				const STNBRtspClientConnReqParamI* p = NBArray_itmPtrAtIndex(&req->params, STNBRtspClientConnReqParamI, i);
				NBHttpBuilder_addHeaderField(&bldr, dst, p->name, p->value);
			}
		}
		//content-length
		/*{
			NBHttpBuilder_addHeaderField(&bldr, dst, "Content-Length", "0");
		}*/
		//end of header
		{
			NBHttpBuilder_addHeaderEnd(&bldr, dst);
			//NBHttpBuilder_addHeaderEnd(&bldr, dst); //testing
			//NBHttpBuilder_addHeaderEnd(&bldr, dst); //testing
		}
	}
	NBString_release(&tmp);
	NBString_release(&tmp2);
}

void NBRtspClientConn_reqBuildStrOpq_(STNBRtspClientConnOpq* opq, STNBRtspClientConnReq* req){
	NBObject_lock(opq);
	{
		NBRtspClientConn_reqBuildStrLockedOpq_(opq, req);
	}
	NBObject_unlock(opq);
}

void NBRtspClientConn_reqNetError_(STNBRtspClientConnOpq* opq, const BOOL allowRetry, const char* dbgDesc){
	opq->state.socket.timeConnected = 0;
	NBASSERT(opq->state.socket.timeConnecting == 0)
	//retry or not
	NBRtspClientConn_reqRemoteError_(opq, allowRetry, dbgDesc);
}

void NBRtspClientConn_reqRemoteError_(STNBRtspClientConnOpq* opq, const BOOL allowRetry, const char* dbgDesc){
	BOOL retrying = FALSE;
	if(opq->reqs.cur != NULL){
		//count attempt
		if(allowRetry){
			opq->reqs.cur->retryCount++;
			if(opq->reqs.cur->retryCount < opq->reqs.cur->timeouts.retries){
				//PRINTF_INFO("NBRtspClientConn, retrying request.\n");
				opq->reqs.cur->out.iPos = 0;
				retrying = TRUE;
			}
		}
		if(retrying){
			NBRtspClientConnReq_resetParser(opq->reqs.cur);
		}
	}
	//notify as error
	if(!retrying){
		NBRtspClientConn_reqOldestNotifyResultAndRemoveOpq_(opq, ENNBRtspClientConnReqStatus_Error, dbgDesc);
	}
}

void NBRtspClientConn_reqOldestNotifyResultAndRemoveOpq_(STNBRtspClientConnOpq* opq, const ENNBRtspClientConnReqStatus status, const char* dbgDesc){
	STNBRtspClientConnReq* req = NULL;
	if(opq->reqs.cur != NULL){
		NBObject_lock(opq);
		{
			NBASSERT(opq->reqs.queue.use > 0)
			if(opq->reqs.queue.use > 0){
				req = NBArray_itmValueAtIndex(&opq->reqs.queue, STNBRtspClientConnReq*, 0);
				NBArray_removeItemAtIndex(&opq->reqs.queue, 0);
				NBASSERT(req == opq->reqs.cur)
				opq->reqs.cur = NULL;
			}
		}
		NBObject_unlock(opq);
	}
	//norify and release (unlocked)
	if(req != NULL){
		if(status == ENNBRtspClientConnReqStatus_Error){
			PRINTF_ERROR("NBRtspClientConn, %s request error after %d attempts: '%s'.\n", req->method, req->retryCount, dbgDesc);
		} else {
			//PRINTF_INFO("NBRtspClientConn, %s request %s after %d attempts: '%s'\n", req->method, (status == ENNBRtspClientConnReqStatus_Success ? "success" : "unkw-end"), req->retryCount, dbgDesc);
		}
		//notify
		if(opq->callback.reqResult != NULL){
			STNBSocketAddr addr;
			NBMemory_setZeroSt(addr, STNBSocketAddr);
			NBSocket_getAddressRemote(opq->state.socket.ref, &addr);
			(*opq->callback.reqResult)(opq->callback.obj, req->uid, &addr, req->method, req->uri, status, &req->in.parser);
		}
		//release
		{
			NBRtspClientConnReq_release(req);
			NBMemory_free(req);
		}
	}
}

//Request

void NBRtspClientConnReq_init(STNBRtspClientConnReq* obj){
	NBMemory_setZeroSt(*obj, STNBRtspClientConnReq);
	NBArray_init(&obj->params, sizeof(STNBRtspClientConnReqParamI), NULL);
	//out
	{
		NBString_initWithSz(&obj->out.buff, 4096, 4096, 0.1f);
	}
	//in
	{
		NBHttpResponse_init(&obj->in.parser);
		NBHttpResponse_setZeroContentLenAsDefault(&obj->in.parser, TRUE);
	}
}

void NBRtspClientConnReq_release(STNBRtspClientConnReq* obj){
	//out
	{
		NBString_release(&obj->out.buff);
	}
	//in
	{
		NBHttpResponse_release(&obj->in.parser);
	}
	if(obj->method != NULL){
		NBMemory_free(obj->method);
		obj->method = NULL;
	}
	if(obj->uri != NULL){
		NBMemory_free(obj->uri);
		obj->uri = NULL;
	}
	if(obj->sessionId != NULL){
		NBMemory_free(obj->sessionId);
		obj->sessionId = NULL;
	}
	{
		SI32 i; for(i = (SI32)obj->params.use - 1; i >= 0; i--){
			STNBRtspClientConnReqParamI* p = NBArray_itmPtrAtIndex(&obj->params, STNBRtspClientConnReqParamI, i);
			if(p->name != NULL){
				NBMemory_free(p->name);
				p->name = NULL;
			}
			if(p->value != NULL){
				NBMemory_free(p->value);
				p->value = NULL;
			}
		}
		NBArray_empty(&obj->params);
		NBArray_release(&obj->params);
	}
	{
		NBStruct_stRelease(NBRtspCfgTimeouts_getSharedStructMap(), &obj->timeouts, sizeof(obj->timeouts));
	}
}

void NBRtspClientConnReq_resetParser(STNBRtspClientConnReq* obj){
	NBHttpResponse_release(&obj->in.parser);
	//
	NBHttpResponse_init(&obj->in.parser);
	NBHttpResponse_setZeroContentLenAsDefault(&obj->in.parser, TRUE);
}

//Auth

BOOL NBRtspClientConn_reqGetAuthBaseFromResponse_(STNBRtspClientConnOpq* opq, const STNBHttpResponse* resp){
	BOOL r = FALSE;
	const char* wwwAuth	= NBHttpResponse_headerValue(resp, "WWW-Authenticate", NULL);
	NBASSERT(opq != NULL)
	if(NBString_strIsEmpty(wwwAuth)){
		PRINTF_ERROR("NBRtspClientConn, RTSP, 'WWW-Authenticate' not provided.\n");
	} else {
		//Expected:
		//WWW-Authenticate: Digest realm="48ea63c2402b",nonce="08834db25099945d53645c8d458f37e0", stale="FALSE"
		//WWW-Authenticate: Basic realm="Hisilicon"
		//Values
		BOOL typeValid = FALSE; const char* type = NULL; SI32 typeSz = 0;
		BOOL realmReq = FALSE; const char* realmHex = NULL; SI32 realmHexSz = 0;
		BOOL nonceReq = FALSE; const char* nonceHex = NULL; SI32 nonceHexSz = 0;
		//Get type
		{
			const char* typeStart = wwwAuth;
			const char* typeEnd = typeStart;
			while((*typeStart == ' ' || *typeStart == '\t') && *typeStart != '\0'){
				typeStart++;
			}
			typeEnd = typeStart;
			while(*typeEnd != ' ' && *typeEnd != '\t' && *typeEnd != '\0'){
				typeEnd++;
			}
			if(typeStart < typeEnd){
				type = typeStart;
				typeSz = (SI32)(typeEnd - typeStart);
			}
			if(NBString_strIsLikeStrBytes("Basic", type, typeSz)){
				typeValid = realmReq = TRUE;
			} else if(NBString_strIsLikeStrBytes("Digest", type, typeSz)){
				//Reference:
				//https://en.wikipedia.org/wiki/Digest_access_authentication
				typeValid = realmReq = nonceReq = TRUE;
			}
		}
		//Get params
		if(typeValid){
			//realm
			if(realmReq){
				const char* startVal = "realm=";
				SI32 startPos2 = NBString_strIndexOfLike(wwwAuth, startVal, 0);
				if(startPos2 >= 0){
					SI32 endAfter2 = NBString_strIndexOfLike(wwwAuth, ",", startPos2);
					if(endAfter2 < 0) endAfter2 = NBString_strLenBytes(wwwAuth);
					//Ignore start
					startPos2 += NBString_strLenBytes(startVal);
					//Ignore double quotes
					if(wwwAuth[startPos2] == '\"') startPos2++;
					if(wwwAuth[endAfter2 - 1] == '\"') endAfter2--;
					//Set value
					if(startPos2 < endAfter2){
						realmHexSz	= (endAfter2 - startPos2);
						realmHex	= &wwwAuth[startPos2];
					}
				}
			}
			//nonce
			if(nonceReq){
				const char* startVal = "nonce=";
				SI32 startPos2 = NBString_strIndexOfLike(wwwAuth, startVal, 0);
				if(startPos2 >= 0){
					SI32 endAfter2 = NBString_strIndexOfLike(wwwAuth, ",", startPos2);
					if(endAfter2 < 0) endAfter2 = NBString_strLenBytes(wwwAuth);
					//Ignore start
					startPos2 += NBString_strLenBytes(startVal);
					//Ignore double quotes
					if(wwwAuth[startPos2] == '\"') startPos2++;
					if(wwwAuth[endAfter2 - 1] == '\"') endAfter2--;
					//Set value
					if(startPos2 < endAfter2){
						nonceHexSz	= (endAfter2 - startPos2);
						nonceHex	= &wwwAuth[startPos2];
					}
				}
			}
			//Validate
			{
				const BOOL realmIsEmpty = (NBString_strIsEmpty(realmHex) || realmHexSz <= 0);
				const BOOL nonceIsEmpty = (NBString_strIsEmpty(nonceHex) || nonceHexSz <= 0); 
				if(realmReq && realmIsEmpty){
					PRINTF_ERROR("NBRtspClientConn, RTSP, 'WWW-Authenticate' expected 'realm' value.\n");
				} else if(nonceReq && nonceIsEmpty){
					PRINTF_ERROR("NBRtspClientConn, RTSP, 'WWW-Authenticate' expected 'nonce' value.\n");
				} else {
					if(opq->state.authBase.type != NULL){
						NBMemory_free(opq->state.authBase.type);
						opq->state.authBase.type = NULL;
					}
					if(opq->state.authBase.realmHex != NULL){
						NBMemory_free(opq->state.authBase.realmHex);
						opq->state.authBase.realmHex = NULL;
					}
					if(opq->state.authBase.nonceHex != NULL){
						NBMemory_free(opq->state.authBase.nonceHex);
						opq->state.authBase.nonceHex = NULL;
					}
					opq->state.authBase.type = NBString_strNewBufferBytes(type, typeSz);
					if(!realmIsEmpty){
						opq->state.authBase.realmHex = NBString_strNewBufferBytes(realmHex, realmHexSz);
					}
					if(!nonceIsEmpty){
						opq->state.authBase.nonceHex = NBString_strNewBufferBytes(nonceHex, nonceHexSz);
					}
					//PRINTF_INFO("NBRtspClientConn, type('%s') realmHex('%s') nonceHex('%s') value.\n", opq->state.authBase.type, opq->state.authBase.realmHex, opq->state.authBase.nonceHex);
					r = TRUE; 
				}
			}
		}
	}
	return r;
}

BOOL NBRtspClientConn_reqConcatAuthDigestResponseHash_(const char* realmHex, const char* nonceHex, const char* user, const UI32 userSz, const char* pass, const UI32 passSz, const char* method, const char* uri, STNBString* dst){
	BOOL r = FALSE;
	//Validate
	if(NBString_strIsEmpty(realmHex)){
		PRINTF_ERROR("NBRtspClientConn, RTSP, 'WWW-Authenticate' expected 'realm' value.\n");
	} else if(NBString_strIsEmpty(nonceHex)){
		PRINTF_ERROR("NBRtspClientConn, RTSP, 'WWW-Authenticate' expected 'nonce' value.\n");
	} else {
		STNBMd5HashHex ha1, ha2, resp;
		STNBString strTmp;
		NBString_init(&strTmp);
		//HA1 = MD5(username:realm:password)
		{
			NBString_empty(&strTmp);
			NBString_concatBytes(&strTmp, user, userSz);
			NBString_concatByte(&strTmp, ':');
			NBString_concat(&strTmp, realmHex);
			NBString_concatByte(&strTmp, ':');
			NBString_concatBytes(&strTmp, pass, passSz);
			ha1 = NBMd5_getHashBytesHex(strTmp.str, strTmp.length);
		}
		//HA2 = MD5(method:digestURI)
		{
			NBString_empty(&strTmp);
			NBString_concat(&strTmp, method);
			NBString_concatByte(&strTmp, ':');
			NBString_concat(&strTmp, uri);
			ha2 = NBMd5_getHashBytesHex(strTmp.str, strTmp.length);
		}
		//resp = MD5(HA1:nonce:HA2)
		{
			NBString_empty(&strTmp);
			NBString_concat(&strTmp, ha1.v);
			NBString_concatByte(&strTmp, ':');
			NBString_concat(&strTmp, nonceHex);
			NBString_concatByte(&strTmp, ':');
			NBString_concat(&strTmp, ha2.v);
			resp = NBMd5_getHashBytesHex(strTmp.str, strTmp.length);
			if(dst != NULL){
				NBString_concat(dst, resp.v);
			}
			r = TRUE;
		}
		NBString_release(&strTmp); 
	}
	return r;
}


