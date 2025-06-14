
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBHttpServiceConn.h"
//
#include "nb/core/NBStruct.h"
#include "nb/core/NBMngrStructMaps.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBThread.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBThreadCond.h"
#include "nb/net/NBSocket.h"
#include "nb/net/NBHttpHeader.h"
#include "nb/net/NBHttpBody.h"
#include "nb/net/NBHttpMessage.h"
#include "nb/net/NBHttpBuilder.h"
#include "nb/ssl/NBSsl.h"
#include "nb/crypto/NBPkcs12.h"
//

#define NB_HTTP_SERVICE_CONN_DEFAULT_READ_BUFF_SZ           (1024 * 16)    //http parser read-buffer size, and socket read-buffer-size
#define NB_HTTP_SERVICE_CONN_DEFAULT_READ_BODY_CHUNKS_SZ    (1024 * 16)    //http body storage segments size

//STNBHttpServiceConnLstnr

typedef struct STNBHttpServiceConnLstnr_ {
	STNBHttpServiceConnLstnrItf	itf;
	void*						usrData;
} STNBHttpServiceConnLstnr;

//pollster callbacks

void NBHttpServiceConn_pollConsumeMask_(STNBIOLnk ioLnk, const UI8 pollMask, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData);
void NBHttpServiceConn_pollConsumeNoOp_(STNBIOLnk ioLnk, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData);
void NBHttpServiceConn_pollRemoved_(STNBIOLnk ioLnk, void* usrData);

//socket sender callbacks

SI32 NBHttpServiceConn_httpReqSend_(STNBHttpServiceRespRef req, void* data, const SI32 dataSz, void* usrData);

//

//NBHttpServiceConnBuff

typedef struct STNBHttpServiceConnBuff_ {
    BYTE*   data;
    UI32    size;    //right bo
    UI32    csmd;    //left border
    UI32    filled;  //right border
} STNBHttpServiceConnBuff;

//NBHttpServiceConnBuffs

typedef struct STNBHttpServiceConnBuffs_ {
    STNBHttpServiceConnBuff* fill;  //current filling buffer
    STNBHttpServiceConnBuff* read;  //current read buffer
    //allocs
    struct {
        STNBHttpServiceConnBuff buff0;      //final-buff if no-ssl, encrypted-buff if ssl
        STNBHttpServiceConnBuff buff1;      //final-buff if no-ssl, encrypted-buff if ssl
    } allocs;
    //totals
    struct {
        UI64    csmd;      //total filled-bytes consumed from buffer
        UI64    filled;    //total filled-bytes populated on buffer
    } totals;
} STNBHttpServiceConnBuffs;

void NBHttpServiceConnBuffs_init(STNBHttpServiceConnBuffs* obj);
void NBHttpServiceConnBuffs_release(STNBHttpServiceConnBuffs* obj);
//
void NBHttpServiceConnBuffs_create(STNBHttpServiceConnBuffs* obj, const SI32 buffSize);
//reading
#define NBHttpServiceConnBuffs_readIsEmpty(OBJ)     ((OBJ)->read->csmd >= (OBJ)->read->filled)
#define NBHttpServiceConnBuffs_readAvailSz(OBJ)     ((OBJ)->read->filled - (OBJ)->read->csmd)
#define NBHttpServiceConnBuffs_readAvailPtr(OBJ)    (&(OBJ)->read->data[(OBJ)->read->csmd])
void NBHttpServiceConnBuffs_readMoveCursor(STNBHttpServiceConnBuffs* obj, const SI32 moveSz);
//filling
#define NBHttpServiceConnBuffs_fillIsFull(OBJ)      ((OBJ)->fill->filled >= (OBJ)->fill->size)
#define NBHttpServiceConnBuffs_fillAvailsz(OBJ)     ((OBJ)->fill->size - (OBJ)->fill->filled)
#define NBHttpServiceConnBuffs_fillAvailPtr(OBJ)    (&(OBJ)->fill->data[(OBJ)->fill->filled])
void NBHttpServiceConnBuffs_fillMoveCursor(STNBHttpServiceConnBuffs* obj, const SI32 moveSz);

//

struct STNBHttpServiceConnOpq_;

//

typedef enum ENNBHttpSslCmd_ {
    ENNBHttpSslCmd_None = 0,    //no command
    ENNBHttpSslCmd_Accept,      //accept
    ENNBHttpSslCmd_Read,        //read
    ENNBHttpSslCmd_Write,       //write
    //count
    ENNBHttpSslCmd_Count
} ENNBHttpSslCmd;

//NBHttpServiceConn

typedef struct STNBHttpServiceConnOpq_ {
	STNBObject				prnt;
    STNBThreadCond          cond;
    STNBStopFlagRef			stopFlag;
	STNBHttpServiceConnLstnr lstnr;
    STNBHttpConnCfg         cfg;
    //state
    struct {
        UI64                secsOverQueueingAccum;  //increased and reduced each second (queued >= sent)
        //lastSec
        struct {
            UI64            usecsAccum;             //one sec calculation
            //write
            struct {
                UI64        queuedTotal;
                UI64        csmdTotal;
            } write;
        } lastSec;
    } state;
	//pollster
	struct {
		STNBIOPollsterSyncRef sync;
		BOOL				isAdded;
	} pollster;
	//net
	struct {
		STNBSocketRef		socket;         //net-socket
        STNBHttpServiceConnBuffs buffs;     //final-buff if no-ssl, encrypted-buff if ssl
        BOOL                isRecvBlocked;  //explicit blocking, last 'recv' action returned would-block
        BOOL                isSendBlocked;  //explicit blocking, last 'send' action returned would-block
        UI64                bytesSent;      //global socket-sent-counter
        //flushed
        struct {
            UI64            totalLast;
        } flushed;
		//ssl
		struct {
			STNBSslRef		conn;			//(if connection is secure, 'sslContext' is not NULL)
            STNBHttpServiceConnBuffs buffs; //final-buff if ssl
            BOOL            isRecvBlocked;  //explicit blocking, last 'recv' action returned would-block
            BOOL            isSendBlocked;  //explicit blocking, last 'send' action returned would-block
            UI64            bytesSent;      //global socket-sent-counter
            //cmd
            struct {
                ENNBHttpSslCmd cur;         //current command (if more than one call was required)
                ENNBSslResult  result;      //last result (read or write pending?)
            } cmd;
		} ssl;
	} net;
	//cert
	struct {
        STNBX509            ca;
		STNBX509			cur;
		BOOL				isSignedByCA;
	} cert;
	//parser
	struct {
        STNBHttpServiceRespCtx ctx;     //context
		//http
		struct {
			STNBHttpMessage	http;
		} parser;
        //lstnr (active analyzer and/or responder)
        struct {
            STNBHttpServiceReqLstnrItf  itf;        //request owner (response producer)
            void*                       itfParam;   //request owner (response producer)
        } lstnr;
	} req;
    //response
    struct {
        STNBHttpServiceRespRef      obj;       //response object
        //default
        struct {
            //header
            struct {
                UI32        code;
                STNBString  reason;
                //fields
                struct {
                    STNBArray   arr;      //STNBHttpHeaderField
                    STNBString  str;     //shared string for header fields.
                } fields;
            } header;
            STNBString  body;
        } def;
    } resp;
	//stats
	struct {
		STNBHttpStatsRef		provider;	//includes reference to parent
		STNBHttpStatsData		upd;	//pending to apply data
		//flush
		struct {
			UI32		iSeqReq;		//flush sequence requested
			UI32		iSeqDone;		//flush sequence done
		} flush;
	} stats;
} STNBHttpServiceConnOpq;

//

NB_OBJREF_BODY(NBHttpServiceConn, STNBHttpServiceConnOpq, NBObject)

//

#define NB_HTTP_SERVICE_CONN_HAS_PEND_TASK_FLUSH_STATS(PTR)	((PTR)->stats.flush.iSeqDone != (PTR)->stats.flush.iSeqReq)
#define NB_HTTP_SERVICE_CONN_HAS_PEND_TASK(PTR)				NB_HTTP_SERVICE_CONN_HAS_PEND_TASK_FLUSH_STATS(PTR)

void NBHttpServiceConn_consumePendTaskStatsFlushLockedOpq_(STNBHttpServiceConnOpq* opq);
void NBHttpServiceConn_consumePendTasksLockedOpq_(STNBHttpServiceConnOpq* opq);

//

void NBHttpServiceConn_consumeReadBufferOpq_(STNBHttpServiceConnOpq* opq, STNBHttpServiceConnBuffs* buffs);
BOOL NBHttpServiceConn_consumeReadBufferPlainOpq_(STNBHttpServiceConnOpq* opq, STNBHttpServiceConnBuffs* buffs);

//

#define NBHttpServiceConn_reqHasOwnerOpq_(OPQ) (NBHttpServiceReqLstnrItf_isSet(&(OPQ)->req.lstnr.itf))      //TRUE if a request is being processed by a rsponder, if not should be flushed or closed
void NBHttpServiceConn_reqFeedRestartLockedOpq_(STNBHttpServiceConnOpq* opq);
void NBHttpServiceConn_reqFeedCompletedLockedOpq_(STNBHttpServiceConnOpq* opq);
void NBHttpServiceConn_reqGetDescLockedOpq_(STNBHttpServiceConnOpq* opq, STNBHttpServiceReqDesc* dst);
void NBHttpServiceConn_reqGetArrivalLnkLockedOpq_(STNBHttpServiceConnOpq* opq, STNBHttpServiceReqArrivalLnk* dst);
//
void NBHttpServiceConn_reqRespStartLockedOpq_(STNBHttpServiceConnOpq* opq, const STNBHttpServiceReqLstnrItf* itf, void* usrParam, const UI32 msPerTick);
void NBHttpServiceConn_reqRespEndLockedOpq_(STNBHttpServiceConnOpq* opq);

//NBHttpMessage listening

BOOL NBHttpServiceConn_consumeHeadStartLine_(const STNBHttpMessage* obj, void* lparam);
BOOL NBHttpServiceConn_consumeHeadFieldLine_(const STNBHttpMessage* obj, const STNBHttpHeaderField* field, void* lparam);
BOOL NBHttpServiceConn_consumeHeadEnd_(const STNBHttpMessage* obj, UI32* dstBodyBuffSz, void* lparam);
BOOL NBHttpServiceConn_consumeBodyData_(const STNBHttpMessage* obj, const void* data, const UI64 dataSz, void* lparam);
BOOL NBHttpServiceConn_consumeBodyTrailerField_(const STNBHttpMessage* obj, const STNBHttpBodyField* field, void* lparam);
BOOL NBHttpServiceConn_consumeBodyEnd_(const STNBHttpMessage* obj, void* lparam);

//NBHttpServiceReqArrivalLnk

UI32 NBHttpServiceConn_httpReqGetDefaultResponseCode_(STNBString* dstReason, void* usrData);   //get request's default response code
BOOL NBHttpServiceConn_httpReqSetDefaultResponseCode_(const UI32 code, const char* reason, void* usrData);   //set request's default response code
BOOL NBHttpServiceConn_httpReqAddDefaultResponseField_(const char* name, const char* value, void* usrData);   //adds a header-field's for the default response without checking (allows duplicated fields)
BOOL NBHttpServiceConn_httpReqSetDefaultResponseField_(const char* name, const char* value, void* usrData);   //adds or updates a header-field's for the default response without checking (wont allow duplicated fields)
BOOL NBHttpServiceConn_httpReqSetDefaultResponseBodyStr_(const char* body, void* usrData);   //set request's default response body
BOOL NBHttpServiceConn_httpReqSetDefaultResponseBodyData_(const void* data, const UI32 dataSz, void* usrData);   //set request's default response body
BOOL NBHttpServiceConn_httpReqConcatDefaultResponseBodyStruct_(const STNBStructMap* structMap, const void* src, const UI32 srcSz, void* usrData);   //set request's default response body
BOOL NBHttpServiceConn_httpReqSetOwner_(const STNBHttpServiceReqLstnrItf* itf, void* itfParam, const UI32 msPerTick, void* usrData);   //set request owner


//NBIOLink
BOOL NBHttpServiceConn_ssl_ioIsObjRef_(STNBObjRef objRef, void* usrData);
SI32 NBHttpServiceConn_ssl_ioRead_(void* dst, const SI32 dstSz, void* usrData); //read data to destination buffer, returns the ammount of bytes read, negative in case of error
SI32 NBHttpServiceConn_ssl_ioWrite_(const void* src, const SI32 srcSz, void* usrData); //write data from source buffer, returns the ammount of bytes written, negative in case of error
void NBHttpServiceConn_ssl_ioFlush_(void* usrData); //flush write-data
void NBHttpServiceConn_ssl_ioShutdown_(const UI8 mask, void* usrData); //NB_IO_BIT_READ | NB_IO_BIT_WRITE
void NBHttpServiceConn_ssl_ioClose_(void* usrData); //close ungracefully

//ssl actions

ENNBSslResult NBHttpServiceConn_sslAcceptTick_(STNBHttpServiceConnOpq* opq, STNBSslRef* ssl);
void NBHttpServiceConn_sslReadTick_(STNBHttpServiceConnOpq* opq);
void NBHttpServiceConn_sslWriteTick_(STNBHttpServiceConnOpq* opq);

//

void NBHttpServiceConn_initZeroed(STNBObject* obj){
	STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)obj;
    NBThreadCond_init(&opq->cond);
    //
    opq->stopFlag = NBStopFlag_alloc(NULL);
	//cfg
	{
		//
	}

	//cert
	{
        NBX509_init(&opq->cert.ca);
		NBX509_init(&opq->cert.cur);
	}
    //net
    {
        //buffs
        {
            NBHttpServiceConnBuffs_init(&opq->net.buffs);
            NBHttpServiceConnBuffs_create(&opq->net.buffs, NB_HTTP_SERVICE_CONN_DEFAULT_READ_BUFF_SZ);
        }
        //ssl
        {
            NBHttpServiceConnBuffs_init(&opq->net.ssl.buffs);
        }
    }
	//parser
	{
		//buff
		{
            
		}
        //buff2
        {
            //non-allocated unless ssl
        }
		//http
		{
			NBHttpMessage_init(&opq->req.parser.http);
            {
                IHttpMessageListener itf;
                NBMemory_setZeroSt(itf, IHttpMessageListener);
                itf.consumeHeadStartLine    = NBHttpServiceConn_consumeHeadStartLine_;
                itf.consumeHeadFieldLine    = NBHttpServiceConn_consumeHeadFieldLine_;
                itf.consumeHeadEnd          = NBHttpServiceConn_consumeHeadEnd_;
                itf.consumeBodyData         = NBHttpServiceConn_consumeBodyData_;
                itf.consumeBodyTrailerField = NBHttpServiceConn_consumeBodyTrailerField_;
                itf.consumeBodyEnd          = NBHttpServiceConn_consumeBodyEnd_;
                NBHttpMessage_setListener(&opq->req.parser.http, &itf, opq);
            }
            {
                const UI32 readSz = (opq->cfg.recv.buffSz > 0 ? opq->cfg.recv.buffSz : NB_HTTP_SERVICE_CONN_DEFAULT_READ_BUFF_SZ);
                const UI32 readBodyChunksSz = (opq->cfg.recv.bodyChunkSz > 0 ? opq->cfg.recv.bodyChunkSz : NB_HTTP_SERVICE_CONN_DEFAULT_READ_BODY_CHUNKS_SZ);
                NBHttpMessage_feedStart(&opq->req.parser.http, readSz, readBodyChunksSz);
            }
			
		}
	}
	//req
	{
		//
	}
    //resp
    {
        //default
        {
            //header
            {
                NBString_initWithSz(&opq->resp.def.header.reason, 0, 256, 0.10f);
                //fields
                {
                    NBArray_initWithSz(&opq->resp.def.header.fields.arr, sizeof(STNBHttpHeaderField), NULL, 0, 8, 0.10f);
                    NBString_initWithSz(&opq->resp.def.header.fields.str, 0, 256, 0.10f);
                }
            }
            NBString_initWithSz(&opq->resp.def.body, 0, 1024, 0.10f);
        }
    }
	//stats
	{
		opq->stats.provider = NBHttpStats_alloc(NULL);
		//flush
		{
			//
		}
	}
}

void NBHttpServiceConn_uninitLocked(STNBObject* obj){
	STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)obj;
    PRINTF_INFO("NBHttpServiceConn_uninitLocked to '%lld'.\n", (SI64)opq);
    //
    NBStopFlag_activate(opq->stopFlag);
    //wait for pollster removal
    {
        while(opq->pollster.isAdded){
            NBThreadCond_waitObj(&opq->cond, opq); //wait for 'isAdded' change
        }
        NBASSERT(!opq->pollster.isAdded)
    }
    NBASSERT(!NBHttpServiceResp_isSet(opq->resp.obj)) //clean-exit required
	//status
	{
		//
	}
	//net
	{
		//socket
		if(NBSocket_isSet(opq->net.socket)){
			NBSocket_release(&opq->net.socket);
			NBSocket_null(&opq->net.socket);
		}
		//ssl
		{
			if(NBSsl_isSet(opq->net.ssl.conn)){
				NBSsl_release(&opq->net.ssl.conn);
				NBSsl_null(&opq->net.ssl.conn);
			}
            NBHttpServiceConnBuffs_release(&opq->net.ssl.buffs);
		}
        NBHttpServiceConnBuffs_release(&opq->net.buffs);
	}
	//pollster
	{
		if(NBIOPollsterSync_isSet(opq->pollster.sync)){
			NBIOPollsterSync_release(&opq->pollster.sync);
			NBIOPollsterSync_null(&opq->pollster.sync);
		}
	}
	//cert
	{
        NBX509_release(&opq->cert.ca);
		NBX509_release(&opq->cert.cur);
	}
	//parser
	{
		//http
		{
			NBHttpMessage_release(&opq->req.parser.http);
		}
	}
	//req
	{
        //reset response
        NBHttpServiceConn_reqRespEndLockedOpq_(opq);
	}
    //resp
    {
        //default
        {
            //header
            {
                NBString_release(&opq->resp.def.header.reason);
                //fields
                NBArray_empty(&opq->resp.def.header.fields.arr);
                NBArray_release(&opq->resp.def.header.fields.arr);
                NBString_release(&opq->resp.def.header.fields.str);
            }
            NBString_release(&opq->resp.def.body);
        }
    }
	//stats
	{
		//provider
		if(NBHttpStats_isSet(opq->stats.provider)){
			//flush upd
			NBHttpStats_accumData(opq->stats.provider, &opq->stats.upd);
			//release
			NBHttpStats_release(&opq->stats.provider);
			NBHttpStats_null(&opq->stats.provider);
		}
		//upd
		{
			NBStruct_stRelease(NBHttpStatsData_getSharedStructMap(), &opq->stats.upd, sizeof(opq->stats.upd));
		}
	}
    //stop flag
    if(NBStopFlag_isSet(opq->stopFlag)){
        NBStopFlag_release(&opq->stopFlag);
        NBStopFlag_null(&opq->stopFlag);
    }
	//cfg
	{
		NBStruct_stRelease(NBHttpConnCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
	}
    NBThreadCond_release(&opq->cond);
}

//request

void NBHttpServiceConn_reqFeedRestartLockedOpq_(STNBHttpServiceConnOpq* opq){
    //
    //PRINTF_INFO("NBHttpServiceConn, reqFeedRestartLockedOpq_.\n");
    //
    const UI32 readSz = (opq->cfg.recv.buffSz > 0 ? opq->cfg.recv.buffSz : NB_HTTP_SERVICE_CONN_DEFAULT_READ_BUFF_SZ);
    const UI32 readBodyChunksSz = (opq->cfg.recv.bodyChunkSz > 0 ? opq->cfg.recv.bodyChunkSz : NB_HTTP_SERVICE_CONN_DEFAULT_READ_BODY_CHUNKS_SZ);
    NBHttpMessage_feedStart(&opq->req.parser.http, readSz, readBodyChunksSz);
    //reset response
    NBHttpServiceConn_reqRespEndLockedOpq_(opq);
    //reset context
    NBMemory_setZeroSt(opq->req.ctx, STNBHttpServiceRespCtx);
}

void NBHttpServiceConn_reqFeedCompletedLockedOpq_(STNBHttpServiceConnOpq* opq){
    //
    //PRINTF_INFO("NBHttpServiceConn, reqFeedCompletedLockedOpq_.\n");
    //
    NBASSERT(opq->req.ctx.req.header.isCompleted) //header must be completed
    NBHttpMessage_feedEnd(&opq->req.parser.http);
    NBASSERT(opq->req.ctx.req.header.isCompleted && opq->req.ctx.req.body.isCompleted) //header and boody must be completed
    //close connection if no-owner-or-response is defined
    if(!NBHttpServiceConn_reqHasOwnerOpq_(opq) && !NBHttpServiceResp_isSet(opq->resp.obj)){
        PRINTF_ERROR("NBHttpServiceConn, stopping after request has no response.\n");
        if(NBStopFlag_isSet(opq->stopFlag)){
            NBStopFlag_activate(opq->stopFlag);
        }
    }
}

void NBHttpServiceConn_reqGetDescLockedOpq_(STNBHttpServiceConnOpq* opq, STNBHttpServiceReqDesc* dst){
    NBMemory_setZeroSt(*dst, STNBHttpServiceReqDesc);
    //first-line
    if(opq->req.parser.http.header.requestLine != NULL){
        dst->firstLine.method   = NBHttpHeader_getRequestMethod(&opq->req.parser.http.header);
        dst->firstLine.target   = NBHttpHeader_getRequestTarget(&opq->req.parser.http.header);
        dst->firstLine.majorVer = opq->req.parser.http.header.requestLine->majorVer;
        dst->firstLine.minorVer = opq->req.parser.http.header.requestLine->minorVer;
    }
    //header
    if(opq->req.ctx.req.header.isCompleted){
        dst->header = &opq->req.parser.http.header;
    }
    //body
    if(opq->req.ctx.req.body.isCompleted){
        dst->body = &opq->req.parser.http.body;
    }
}

void NBHttpServiceConn_reqGetArrivalLnkLockedOpq_(STNBHttpServiceConnOpq* opq, STNBHttpServiceReqArrivalLnk* dst){
    NBMemory_setZeroSt(*dst, STNBHttpServiceReqArrivalLnk);
    dst->itf.httpReqGetDefaultResponseCode = NBHttpServiceConn_httpReqGetDefaultResponseCode_;
    dst->itf.httpReqSetDefaultResponseCode = NBHttpServiceConn_httpReqSetDefaultResponseCode_;
    dst->itf.httpReqAddDefaultResponseField = NBHttpServiceConn_httpReqAddDefaultResponseField_;
    dst->itf.httpReqSetDefaultResponseField = NBHttpServiceConn_httpReqSetDefaultResponseField_;
    dst->itf.httpReqSetDefaultResponseBodyStr = NBHttpServiceConn_httpReqSetDefaultResponseBodyStr_;
    dst->itf.httpReqSetDefaultResponseBodyData = NBHttpServiceConn_httpReqSetDefaultResponseBodyData_;
    dst->itf.httpReqConcatDefaultResponseBodyStruct = NBHttpServiceConn_httpReqConcatDefaultResponseBodyStruct_;
    dst->itf.httpReqSetOwner = NBHttpServiceConn_httpReqSetOwner_;
    dst->usrParam = opq;
}

//response

void NBHttpServiceConn_reqRespStartLockedOpq_(STNBHttpServiceConnOpq* opq, const STNBHttpServiceReqLstnrItf* itf, void* usrParam, const UI32 msPerTick){
    NBASSERT(!NBHttpServiceConn_reqHasOwnerOpq_(opq)) //out of sync, 'NBHttpServiceConn_reqRespEndLockedOpq_' should be called
    NBASSERT(!NBHttpServiceResp_isSet(opq->resp.obj)) //out of sync, 'NBHttpServiceConn_reqRespEndLockedOpq_' should be called
    //start
    NBASSERT(itf != NULL)
    if(!NBHttpServiceResp_isSet(opq->resp.obj)){
        opq->resp.obj = NBHttpServiceResp_alloc(NULL);
        if(!NBHttpServiceResp_prepare(opq->resp.obj, &opq->cfg)){
            PRINTF_ERROR("NBHttpServiceConn, reqRespStartLockedOpq_, NBHttpServiceResp_prepare failed.\n");
            if(NBStopFlag_isSet(opq->stopFlag)){
                NBStopFlag_activate(opq->stopFlag);
            }
        } else {
            //create response itf
            NBHttpServiceResp_getRespItf(opq->resp.obj, &opq->req.ctx.resp.lnk.itf, &opq->req.ctx.resp.lnk.itfParam);
        }
        opq->state.secsOverQueueingAccum        = 0;
        opq->state.lastSec.write.queuedTotal    = 0;
        opq->state.lastSec.write.csmdTotal      = 0;
    }
    //itf
    {
        if(itf == NULL){
            NBMemory_setZeroSt(opq->req.lstnr.itf, STNBHttpServiceReqLstnrItf);
        } else {
            opq->req.lstnr.itf   = *itf;
        }
        opq->req.lstnr.itfParam = usrParam;
    }
    //ticks
    {
        opq->req.ctx.ticks.msPerTick = msPerTick;
    }
}

void NBHttpServiceConn_reqRespEndLockedOpq_(STNBHttpServiceConnOpq* opq){
    STNBHttpServiceReqLstnrItf  itf = opq->req.lstnr.itf;
    void* itfParam = opq->req.lstnr.itfParam;
    //unset itf
    {
        NBMemory_setZero(opq->req.lstnr.itf);
        opq->req.lstnr.itfParam = NULL;
    }
    //resp
    {
        //unset obj
        if(NBHttpServiceResp_isSet(opq->resp.obj)){
            NBHttpServiceResp_release(&opq->resp.obj);
            NBHttpServiceResp_null(&opq->resp.obj);
        }
        //default
        {
            opq->resp.def.header.code = 0;
            NBString_empty(&opq->resp.def.header.reason);
            NBString_empty(&opq->resp.def.body);
        }
    }
    //Notify (unlocked)
    if(itf.httpReqOwnershipEnded != NULL){
        NBObject_unlock(opq);
        {
            (*itf.httpReqOwnershipEnded)(opq->req.ctx, itfParam);
        }
        NBObject_lock(opq);
    }
}

//cfg

BOOL NBHttpServiceConn_isCertPresent(STNBHttpServiceConnRef ref){
    BOOL r = FALSE;
	STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)ref.opaque; NBASSERT(NBHttpServiceConn_isClass(ref))
    NBObject_lock(opq);
    {
        //cert.cur never changes, safe to read unlocked
        r = NBX509_isCreated(&opq->cert.cur);
    }
    NBObject_unlock(opq);
	return r;
}

BOOL NBHttpServiceConn_isCertSignedByCA(STNBHttpServiceConnRef ref){
	STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)ref.opaque; NBASSERT(NBHttpServiceConn_isClass(ref))
	//cert.isSignedByCA never changes, safe to read unlocked
	return opq->cert.isSignedByCA;
}

BOOL NBHttpServiceConn_getCert(STNBHttpServiceConnRef ref, STNBX509* dst, BOOL* dstIsSignedByCA){
    BOOL r = FALSE;
    STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)ref.opaque; NBASSERT(NBHttpServiceConn_isClass(ref))
    NBObject_lock(opq);
    {
        //cert.cur never changes, safe to read unlocked
        if(NBX509_isCreated(&opq->cert.cur)){
            if(dst != NULL){
                r = NBX509_createFromOther(dst, &opq->cert.cur);
            } else {
                r = TRUE;
            }
        }
        if(dstIsSignedByCA != NULL){
            *dstIsSignedByCA = opq->cert.isSignedByCA;
        }
    }
    NBObject_unlock(opq);
	return r;
}

BOOL NBHttpServiceConn_isSocket(STNBHttpServiceConnRef ref, STNBSocketRef socket){
    BOOL r = FALSE;
	STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)ref.opaque; NBASSERT(NBHttpServiceConn_isClass(ref))
    NBObject_lock(opq);
    {
        r = NBSocket_isSame(opq->net.socket, socket);
    }
    NBObject_unlock(opq);
	return r;
}

//

ENNBSslResult NBHttpServiceConn_sslAcceptTick_(STNBHttpServiceConnOpq* opq, STNBSslRef* ssl){
    NBASSERT(NBSsl_isSet(*ssl) && (opq->net.ssl.cmd.cur == ENNBHttpSslCmd_None || opq->net.ssl.cmd.cur == ENNBHttpSslCmd_Accept))
    const ENNBSslResult r = NBSsl_accept(*ssl);
    switch (r) {
        case ENNBSslResult_Success:
            opq->net.ssl.cmd.cur = ENNBHttpSslCmd_None;
            opq->net.ssl.cmd.result = r;
            {
                opq->cert.isSignedByCA = FALSE;
                NBX509_release(&opq->cert.cur);
                NBX509_init(&opq->cert.cur);
                if(NBSsl_getPeerCertificate(*ssl, &opq->cert.cur)){
                    if(NBX509_isCreated(&opq->cert.ca) && NBX509_isCreated(&opq->cert.cur)){
                        opq->cert.isSignedByCA = NBX509_isSignedBy(&opq->cert.cur, &opq->cert.ca);
                    }
                    PRINTF_INFO("NBHttpServiceConn, NBSsl_accept completed with peer providing a certificate (%s-by-CA).\n", opq->cert.isSignedByCA ? "signed" : "not-signed");
                } else {
                    PRINTF_INFO("NBHttpServiceConn, NBSsl_accept completed with peer NOT providing a certificate.\n");
                }
            }
            break;
        case ENNBSslResult_ErrWantRead:
            //PRINTF_INFO("NBHttpServiceConn, NBSsl_accept hungry-for-read.\n");
            opq->net.ssl.cmd.cur = ENNBHttpSslCmd_Accept;
            opq->net.ssl.cmd.result = r;
            break;
        case ENNBSslResult_ErrWantWrite:
            //PRINTF_INFO("NBHttpServiceConn, NBSsl_accept hungry-for-write.\n");
            opq->net.ssl.cmd.cur = ENNBHttpSslCmd_Accept;
            opq->net.ssl.cmd.result = r;
            break;
        default:
            if(NBStopFlag_isSet(opq->stopFlag)){
                NBStopFlag_activate(opq->stopFlag);
            }
            opq->net.ssl.cmd.cur = ENNBHttpSslCmd_None;
            opq->net.ssl.cmd.result = ENNBSslResult_Error;
            break;
    }
    return r;
}

void NBHttpServiceConn_sslReadTick_(STNBHttpServiceConnOpq* opq){
    STNBHttpServiceConnBuffs* buffs = &opq->net.ssl.buffs;
    NBASSERT(buffs->fill != NULL && buffs->fill->data != NULL && buffs->fill->size > 0)
    //ToDo: voluntarily stop reading after a limit (avoid chance of hijacking the thread on fast network action)
    //read and consume (while not explicit-blocked and buffer-has-space)
    BOOL csmdCalled = FALSE;
    opq->net.ssl.isRecvBlocked = FALSE;
    while(!NBStopFlag_isMineActivated(opq->stopFlag) && !opq->net.ssl.isRecvBlocked && !NBHttpServiceConnBuffs_fillIsFull(buffs)){
        //recv
        const SI32 rcvd = NBSsl_read(opq->net.ssl.conn, NBHttpServiceConnBuffs_fillAvailPtr(buffs), NBHttpServiceConnBuffs_fillAvailsz(buffs));
        //process
        if(rcvd < 0){
            //results
            switch (rcvd) {
                case ENNBSslResult_ErrWantRead:
                    PRINTF_INFO("NBHttpServiceConn, NBSsl_read hungry-for-read.\n");
                    opq->net.ssl.cmd.cur = ENNBHttpSslCmd_Read;
                    opq->net.ssl.cmd.result = (ENNBSslResult)rcvd;
                    break;
                case ENNBSslResult_ErrWantWrite:
                    PRINTF_INFO("NBHttpServiceConn, NBSsl_read hungry-for-write.\n");
                    opq->net.ssl.cmd.cur = ENNBHttpSslCmd_Read;
                    opq->net.ssl.cmd.result = (ENNBSslResult)rcvd;
                    break;
                default:
                    PRINTF_ERROR("NBHttpServiceConn, NBSsl_read failed with (%d) plain.\n", rcvd);
                    NBASSERT(rcvd == ENNBSslResult_Error)
                    opq->net.ssl.cmd.cur = ENNBHttpSslCmd_Read;
                    opq->net.ssl.cmd.result = (ENNBSslResult)rcvd;
                    if(NBStopFlag_isSet(opq->stopFlag)){
                        NBStopFlag_activate(opq->stopFlag);
                    }
                    break;
            }
            opq->net.ssl.isRecvBlocked = TRUE;
            break;
        } else {
            //PRINTF_INFO("NBHttpServiceConn, NBSsl_read received %d bytes.\n", rcvd);
            opq->net.ssl.cmd.cur = ENNBHttpSslCmd_None;
            opq->net.ssl.cmd.result = ENNBSslResult_Success;
            if(rcvd == 0){
                opq->net.ssl.isRecvBlocked = TRUE;
            }
            //
            /*if(rcvd <= 0){
                PRINTF_INFO("NBHttpServiceConn, NBSsl_read received %d bytes.\n", rcvd);
            } else {
                STNBString str;
                NBString_initWithStrBytes(&str, (const char*)NBHttpServiceConnBuffs_fillAvailPtr(buffs), rcvd);
                PRINTF_INFO("NBHttpServiceConn, NBSsl_read received %d bytes:--->\n%s<---.\n", rcvd, str.str);
                NBString_release(&str);
            }*/
            NBHttpServiceConnBuffs_fillMoveCursor(buffs, rcvd);
            //consume inmediatly
            NBHttpServiceConn_consumeReadBufferPlainOpq_(opq, buffs);
            csmdCalled = TRUE;
        }
    }
    //consume (at least once)
    if(!csmdCalled){
        NBHttpServiceConn_consumeReadBufferPlainOpq_(opq, buffs);
        csmdCalled = TRUE;
    }
}

void NBHttpServiceConn_sslWriteTick_(STNBHttpServiceConnOpq* opq){
    //plain-send
    STNBHttpServiceRespSenderItf itf;
    NBMemory_setZeroSt(itf, STNBHttpServiceRespSenderItf);
    itf.httpReqSend = NBHttpServiceConn_httpReqSend_;
    if(NBHttpServiceResp_write(opq->resp.obj, &itf, opq) < 0){
        PRINTF_ERROR("NBHttpServiceConn, NBHttpServiceResp_write failed.\n");
        if(NBStopFlag_isSet(opq->stopFlag)){
            NBStopFlag_activate(opq->stopFlag);
        }
    }
}

BOOL NBHttpServiceConn_startListeningOwningSocket(STNBHttpServiceConnRef ref, const STNBHttpConnCfg* limits, STNBIOPollsterSyncRef pollSync, STNBStopFlagRef* parentStopFlag, STNBSocketRef socket, STNBX509* optCACert, STNBSslContextRef optSslContext, STNBHttpStatsRef parentProvider, STNBHttpServiceConnLstnrItf* lstnrItf, void* lstnrUsrData){
	BOOL r = FALSE;
	STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)ref.opaque; NBASSERT(NBHttpServiceConn_isClass(ref))
    if(NBIOPollsterSync_isSet(pollSync) && NBSocket_isSet(socket)){
        NBObject_lock(opq);
        if(!opq->pollster.isAdded && !NBIOPollsterSync_isSet(opq->pollster.sync) && !NBSocket_isSet(opq->net.socket)){
            r = TRUE;
            //set-vars
            {
                //parent stop flag
                if(r){
                    NBStopFlag_setParentFlag(opq->stopFlag, parentStopFlag);
                }
                //ssl
                if(r){
                    //ca
                    {
                        //release
                        NBX509_release(&opq->cert.ca);
                        //alloc/set
                        NBX509_init(&opq->cert.ca);
                        if(optCACert != NULL){
                            NBX509_createFromOther(&opq->cert.ca, optCACert);
                        }
                    }
                    //ssl
                    {
                        //release
                        if(NBSsl_isSet(opq->net.ssl.conn)){
                            NBSsl_release(&opq->net.ssl.conn);
                            NBSsl_null(&opq->net.ssl.conn);
                        }
                        //alloc/set
                        if(NBSslContext_isSet(optSslContext)){
                            //init connection (unlocked)
                            {
                                STNBSslRef ssl = NBSsl_alloc(NULL);
                                STNBIOLnkItf itf;
                                NBMemory_setZeroSt(itf, STNBIOLnkItf);
                                //no-lock required
                                itf.ioRetain   = NULL; //NBObjRef_retainOpq;
                                itf.ioRelease  = NULL; //NBObjRef_releaseOpq;
                                //
                                itf.ioIsObjRef = NBHttpServiceConn_ssl_ioIsObjRef_;
                                //
                                itf.ioRead     = NBHttpServiceConn_ssl_ioRead_;
                                itf.ioWrite    = NBHttpServiceConn_ssl_ioWrite_;
                                itf.ioFlush    = NBHttpServiceConn_ssl_ioFlush_;
                                itf.ioShutdown = NBHttpServiceConn_ssl_ioShutdown_;
                                itf.ioClose    = NBHttpServiceConn_ssl_ioClose_;
                                //
                                if(!NBSsl_createWithIOLnkItf(ssl, optSslContext, &itf, opq)){
                                    PRINTF_ERROR("NBHttpServiceConn, initial NBSsl_createWithIOLnkItf failed.\n");
                                    NBStopFlag_activate(opq->stopFlag);
                                    r = FALSE;
                                } else if(ENNBSslResult_Error == NBHttpServiceConn_sslAcceptTick_(opq, &ssl)){
                                    PRINTF_ERROR("NBHttpServiceConn, initial NBHttpServiceConn_sslAcceptTick_ failed.\n");
                                    NBStopFlag_activate(opq->stopFlag);
                                    r = FALSE;
                                } else {
                                    //allocate buffers
                                    const UI32 readSz = (opq->cfg.recv.buffSz > 0 ? opq->cfg.recv.buffSz : NB_HTTP_SERVICE_CONN_DEFAULT_READ_BUFF_SZ);
                                    NBHttpServiceConnBuffs_create(&opq->net.ssl.buffs, readSz);
                                    //consume
                                    NBSsl_set(&opq->net.ssl.conn, &ssl); //apply
                                }
                                //release (if not consumed)
                                if(NBSsl_isSet(ssl)){
                                    NBSsl_release(&ssl);
                                    NBSsl_null(&ssl);
                                }
                            }
                        }
                    }
                }
                //socket
                if(r){
                    opq->net.socket = socket;
                    NBSocket_setNoSIGPIPE(opq->net.socket, TRUE);
                    NBSocket_setCorkEnabled(opq->net.socket, FALSE);
                    NBSocket_setDelayEnabled(opq->net.socket, FALSE);
                    NBSocket_setNonBlocking(opq->net.socket, TRUE);
                    NBSocket_setUnsafeMode(opq->net.socket, TRUE);
                }
                //pollster
                if(r){
                    NBIOPollsterSync_set(&opq->pollster.sync, &pollSync);
                }
                //lstnr
                if(r){
                    if(lstnrItf == NULL){
                        NBMemory_setZeroSt(opq->lstnr.itf, STNBHttpServiceConnLstnrItf);
                    } else {
                        opq->lstnr.itf = *lstnrItf;
                    }
                    opq->lstnr.usrData = lstnrUsrData;
                }
            }
            //set parent provider
            if(r){
                //flush-upd while previous provider is set
                {
                    NBHttpStats_accumData(opq->stats.provider, &opq->stats.upd);
                    NBStruct_stRelease(NBHttpStatsData_getSharedStructMap(), &opq->stats.upd, sizeof(opq->stats.upd));
                }
                NBHttpStats_setParentStats(opq->stats.provider, parentProvider);
            }
            //set read-buffer
            if(r){
                const UI32 readSz = (opq->cfg.recv.buffSz > 0 ? opq->cfg.recv.buffSz : NB_HTTP_SERVICE_CONN_DEFAULT_READ_BUFF_SZ);
                NBHttpServiceConnBuffs_create(&opq->net.buffs, readSz);
            }
            //set cfg
            if(r){
                NBStruct_stRelease(NBHttpConnCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
                if(limits != NULL){
                    NBStruct_stClone(NBHttpConnCfg_getSharedStructMap(), limits, sizeof(*limits), &opq->cfg, sizeof(opq->cfg));
                }
            }
            //start listenting
            if(r){
                STNBIOPollsterLstrnItf itf;
                NBMemory_setZeroSt(itf, STNBIOPollsterLstrnItf);
                itf.pollConsumeMask = NBHttpServiceConn_pollConsumeMask_;
                itf.pollConsumeNoOp = NBHttpServiceConn_pollConsumeNoOp_;
                itf.pollRemoved     = NBHttpServiceConn_pollRemoved_;
                opq->pollster.isAdded = TRUE;
                if(!NBIOPollsterSync_addSocketWithItf(pollSync, socket, ENNBIOPollsterOpBit_Read, &itf, opq)){
                    PRINTF_ERROR("NBHttpServiceConn, initial NBIOPollsterSync_addSocketWithItf failed.\n");
                    NBStopFlag_activate(opq->stopFlag);
                    //unset-vars
                    NBMemory_setZeroSt(opq->lstnr, STNBHttpServiceConnLstnr);
                    opq->pollster.isAdded = FALSE;
                    NBIOPollsterSync_release(&opq->pollster.sync);
                    NBIOPollsterSync_null(&opq->pollster.sync);
                    NBSocket_null(&opq->net.socket);
                    r = FALSE;
                }
                //
                NBThreadCond_broadcast(&opq->cond); //notify 'isAdded' change
            }
        }
        NBObject_unlock(opq);
    }
	return r;
}


void NBHttpServiceConn_stopFlag(STNBHttpServiceConnRef ref){
	STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)ref.opaque; NBASSERT(NBHttpServiceConn_isClass(ref))
    NBObject_lock(opq);
    {
        NBStopFlag_activate(opq->stopFlag);
    }
    NBObject_unlock(opq);
}

BOOL NBHttpServiceConn_isBusy(STNBHttpServiceConnRef ref){
	STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)ref.opaque; NBASSERT(NBHttpServiceConn_isClass(ref))
	return opq->pollster.isAdded;
}

void NBHttpServiceConn_consumeReadBufferOpq_(STNBHttpServiceConnOpq* opq, STNBHttpServiceConnBuffs* buffs){
    //consume read-buffer
    if(NBSsl_isSet(opq->net.ssl.conn)){
        //consume as encrypted-action
        BOOL rr = TRUE;
        while(!NBStopFlag_isMineActivated(opq->stopFlag) && !NBHttpServiceConnBuffs_readIsEmpty(buffs) && rr){
            rr = FALSE;
            //continue command
            switch (opq->net.ssl.cmd.cur) {
                case ENNBHttpSslCmd_None:
                    //try to read, no command is active and data arrived
                    //call 'read'
                    {
                        const UI64 csmdBefore = buffs->totals.csmd;
                        NBHttpServiceConn_sslReadTick_(opq);
                        if(csmdBefore != buffs->totals.csmd){
                            rr = TRUE;
                        }
                    }
                    break;
                case ENNBHttpSslCmd_Accept:
                    if(opq->net.ssl.cmd.result == ENNBSslResult_ErrWantRead){
                        //call 'accept' again
                        const UI64 csmdBefore = buffs->totals.csmd;
                        if(ENNBSslResult_Error == NBHttpServiceConn_sslAcceptTick_(opq, &opq->net.ssl.conn)){
                            PRINTF_ERROR("NBHttpServiceConn, read-continuation NBSsl_accept failed.\n");
                            NBStopFlag_activate(opq->stopFlag);
                        } else if(csmdBefore != buffs->totals.csmd){
                            rr = TRUE;
                        }
                    }
                    break;
                case ENNBHttpSslCmd_Read:
                    if(opq->net.ssl.cmd.result == ENNBSslResult_ErrWantRead){
                        //call 'read' again
                        const UI64 csmdBefore = buffs->totals.csmd;
                        NBHttpServiceConn_sslReadTick_(opq);
                        if(csmdBefore != buffs->totals.csmd){
                            rr = TRUE;
                        }
                    }
                    break;
                case ENNBHttpSslCmd_Write:
                    break;
                default:
                    break;
            }
        }
    } else {
        //consume as plain-buffer
        BOOL rr = TRUE;
        while(!NBStopFlag_isMineActivated(opq->stopFlag) && !NBHttpServiceConnBuffs_readIsEmpty(buffs) && rr){
            rr = FALSE;
            if(NBHttpServiceConn_consumeReadBufferPlainOpq_(opq, buffs)){
                rr = TRUE;
            }
        }
    }
}

void NBHttpServiceConn_consumeWriteOportunityOpq_(STNBHttpServiceConnOpq* opq){
    //action
    if(NBSsl_isSet(opq->net.ssl.conn)){
        //encrypted action
        switch (opq->net.ssl.cmd.cur) {
            case ENNBHttpSslCmd_None:
                //try to write, no command is active and data arrived
                //call 'write'
                if(opq->req.ctx.req.body.isCompleted && NBHttpServiceResp_isSet(opq->resp.obj) && NBHttpServiceResp_isWritePend(opq->resp.obj)){
                    NBHttpServiceConn_sslWriteTick_(opq);
                }
                break;
            case ENNBHttpSslCmd_Accept:
                if(opq->net.ssl.cmd.result == ENNBSslResult_ErrWantWrite){
                    //call 'accept' again
                    if(ENNBSslResult_Error == NBHttpServiceConn_sslAcceptTick_(opq, &opq->net.ssl.conn)){
                        PRINTF_ERROR("NBHttpServiceConn, write-continuation NBSsl_accept failed.\n");
                        NBStopFlag_activate(opq->stopFlag);
                    }
                }
                break;
            case ENNBHttpSslCmd_Read:
                if(opq->net.ssl.cmd.result == ENNBSslResult_ErrWantWrite){
                    //call 'read' again
                    NBHttpServiceConn_sslReadTick_(opq);
                }
                break;
            case ENNBHttpSslCmd_Write:
                if(opq->net.ssl.cmd.result == ENNBSslResult_ErrWantWrite){
                    //call 'write' again
                    NBHttpServiceConn_sslWriteTick_(opq);
                }
                break;
            default:
                NBASSERT(FALSE) //unsupported command (unsyned source code?)
                break;
        }
    } else if(opq->req.ctx.req.body.isCompleted && NBHttpServiceResp_isSet(opq->resp.obj) && NBHttpServiceResp_isWritePend(opq->resp.obj)){
        //plain-send
        STNBHttpServiceRespSenderItf itf;
        NBMemory_setZeroSt(itf, STNBHttpServiceRespSenderItf);
        itf.httpReqSend = NBHttpServiceConn_httpReqSend_;
        if(NBHttpServiceResp_write(opq->resp.obj, &itf, opq) < 0){
            PRINTF_ERROR("NBHttpServiceConn, NBHttpServiceResp_write failed.\n");
            NBStopFlag_activate(opq->stopFlag);
        }
    }
}

BOOL NBHttpServiceConn_consumeReadBufferPlainOpq_(STNBHttpServiceConnOpq* opq, STNBHttpServiceConnBuffs* buffs){
    const UI64 csmBefore = buffs->totals.csmd;
    //Chars-format
    /*{
        SI32 i = buffs->read->csmd;
        STNBString str;
        NBString_init(&str);
        while(i < buffs->read->filled){
            const char b = ((char*)buffs->read->data)[i];
            if(b == '\0'){
                NBString_concat(&str, "[\0]");
            } else {
                NBString_concatByte(&str, b);
            }
            if(((i + 1) % 64) == 0){
                NBString_concat(&str, " //");
                NBString_concatUI32(&str, (i + 1));
                NBString_concatByte(&str, '\n');
            }
            i++;
        }
        if((i % 64) != 0){
            NBString_concat(&str, " //");
            NBString_concatUI32(&str, i);
            NBString_concatByte(&str, '\n');
        }
        PRINTF_INFO("NBHttpServiceConn, received %d chars:\n%s\n\n", NBHttpServiceConnBuffs_readAvailSz(buffs), str.str);
        NBString_release(&str);
    }*/
    //Hex-format
    /*{
        const char* hex = "0123456789abcdef";
        SI32 i = buffs->read->csmd;
        STNBString str;
        NBString_init(&str);
        while(i < buffs->read->filled){
            const BYTE b = ((BYTE*)buffs->read->data)[i];
            const BYTE b0 = (b >> 4) & 0xF;
            const BYTE b1 = b & 0xF;
            NBString_concatByte(&str, hex[b0]);
            NBString_concatByte(&str, hex[b1]);
            if(((i + 1) % 64) == 0){
                NBString_concat(&str, " //");
                NBString_concatUI32(&str, (i + 1));
                NBString_concatByte(&str, '\n');
            } else if(((i + 1) % 4) == 0){
                NBString_concatByte(&str, ' ');
            }
            i++;
        }
        if((i % 64) != 0){
            NBString_concat(&str, " //");
            NBString_concatUI32(&str, i);
            NBString_concatByte(&str, '\n');
        }
        PRINTF_INFO("NBHttpServiceConn, received %d bytes:\n%s\n\n", NBHttpServiceConnBuffs_readAvailSz(buffs), str.str);
        NBString_release(&str);
    }*/
    //feed parser
    //buff->csmd
    if(
       !NBStopFlag_isMineActivated(opq->stopFlag)
       && !opq->req.ctx.req.body.isCompleted //parse only data before request completion
       && !NBHttpServiceConnBuffs_readIsEmpty(buffs) //bytes available
       )
    {
        const UI32 csmd = NBHttpMessage_feedBytes(&opq->req.parser.http, NBHttpServiceConnBuffs_readAvailPtr(buffs), NBHttpServiceConnBuffs_readAvailSz(buffs));
        if(csmd <= 0){
            PRINTF_ERROR("NBHttpServiceConn, malformed http message (dropping connection).\n")
            NBStopFlag_activate(opq->stopFlag);
        } else {
            //move read cursor
            NBHttpServiceConnBuffs_readMoveCursor(buffs, csmd);
            //feed completed?
            if(NBHttpMessage_feedIsComplete(&opq->req.parser.http)){
                NBObject_lock(opq);
                {
                    NBHttpServiceConn_reqFeedCompletedLockedOpq_(opq);
                }
                NBObject_unlock(opq);
            }
        }
    }
    //feed responder (raw-stream)
    if(
       !NBStopFlag_isMineActivated(opq->stopFlag)
       && opq->req.ctx.req.body.isCompleted //raw-read only data after request-body
       && !NBHttpServiceConnBuffs_readIsEmpty(buffs) //bytes available
       && opq->req.lstnr.itf.httpReqRcvd != NULL
       && NBHttpServiceResp_isSet(opq->resp.obj) && NBHttpServiceResp_isUpgradedRawRead(opq->resp.obj) //response was upgraded to raw-including-read
       )
    {
        const UI32 csmd = (*opq->req.lstnr.itf.httpReqRcvd)(opq->req.ctx, NBHttpServiceConnBuffs_readAvailPtr(buffs), NBHttpServiceConnBuffs_readAvailSz(buffs), opq->req.lstnr.itfParam);
        if(csmd > 0){
            //move read cursor
            NBHttpServiceConnBuffs_readMoveCursor(buffs, csmd);
        }
    }
    return (csmBefore != buffs->totals.csmd); //TRUE if any byte was consumed
}
    
//pollster callbacks
	
void NBHttpServiceConn_pollConsumeMask_(STNBIOLnk ioLnk, const UI8 pollMask, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData){
	STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)usrData;
    const UI64 netSentBefore = opq->net.bytesSent;
    UI64 netRecvTick = 0;
	IF_NBASSERT(STNBTimestampMicro timeLastAction = NBTimestampMicro_getMonotonicFast(), timeCur; SI64 usDiff;)
    //
    //unset blocking-flags
    STNBHttpServiceConnBuffs* buffs = &opq->net.buffs;
    opq->net.isRecvBlocked = ((pollMask & ENNBIOPollsterOpBit_Read) == 0); //read only when explicit available.
    if(pollMask & ENNBIOPollsterOpBit_Write){ //write is implicit available untill explicit unlocked.
        opq->net.isSendBlocked = FALSE;
    }
	//read
    {
        //ToDo: voluntarily stop reading after a limit (avoid chance of hijacking the thread on fast network action)
        //read and consume (while not explicit-blocked and buffer-has-space)
        BOOL csmdCalled = FALSE;
        while(!NBStopFlag_isMineActivated(opq->stopFlag) && !opq->net.isRecvBlocked && !NBHttpServiceConnBuffs_fillIsFull(buffs)){
            //recv
            const SI32 rcvd = NBIOLnk_read(&ioLnk, NBHttpServiceConnBuffs_fillAvailPtr(buffs), NBHttpServiceConnBuffs_fillAvailsz(buffs));
            //process
            if(rcvd < 0){
                //stop (socket error)
                PRINTF_ERROR("NBHttpServiceConn, NBSocket_recv failed with (%d).\n", rcvd);
                NBStopFlag_activate(opq->stopFlag);
                opq->net.isRecvBlocked = TRUE;
                break;
            } else if(rcvd > 0){
                //PRINTF_INFO("NBHttpServiceConn, NBSocket_recv received %d bytes.\n", rcvd);
#               ifndef CONFIG_NB_DESHABILITAR_IMPRESIONES_PRINTF
                if(!NBSsl_isSet(opq->net.ssl.conn)){
                    STNBString str;
                    NBString_initWithStrBytes(&str, (const char*)NBHttpServiceConnBuffs_fillAvailPtr(buffs), rcvd);
                    PRINTF_INFO("NBHttpServiceConn, NBSocket_recv received %d bytes:--->\n%s<---.\n", rcvd, str.str);
                    NBString_release(&str);
                }
#               endif
                NBHttpServiceConnBuffs_fillMoveCursor(buffs, rcvd);
                //consume inmediatly
                NBHttpServiceConn_consumeReadBufferOpq_(opq, buffs);
                csmdCalled = TRUE;
                //stats
                netRecvTick += rcvd;
            } else {
                //wait for flag to be cleaned
                opq->net.isRecvBlocked = TRUE;
            }
        }
        //consume (at least once)
        if(!csmdCalled){
            NBHttpServiceConn_consumeReadBufferOpq_(opq, buffs);
            csmdCalled = TRUE;
        }
    }
	//write
	{
        //ToDo: voluntarily stop sending after a limit (avoid chance of hijacking the thread on fast network action)
        UI64 sentBefore = opq->net.bytesSent + 1; //+1 to force first action.
        while(!NBStopFlag_isMineActivated(opq->stopFlag) && !opq->net.isSendBlocked && sentBefore != opq->net.bytesSent){
            sentBefore = opq->net.bytesSent;
            NBHttpServiceConn_consumeWriteOportunityOpq_(opq);
        }
        //notify flush-event (the write buffer was fully consumed after populated at least once)
        if(!NBStopFlag_isMineActivated(opq->stopFlag) && NBHttpServiceResp_isSet(opq->resp.obj)){
            const UI64 totalQueued = NBHttpServiceResp_writeQueuedTotal(opq->resp.obj);
            if(opq->net.flushed.totalLast != totalQueued && !NBHttpServiceResp_isWritePend(opq->resp.obj)){
                opq->net.flushed.totalLast = totalQueued;
                if(opq->req.lstnr.itf.httpSendFlushed != NULL){
                    (*opq->req.lstnr.itf.httpSendFlushed)(opq->req.ctx, opq->req.lstnr.itfParam);
                }
                //reset overqueueing accum
                opq->state.secsOverQueueingAccum = 0;
            }
        }
	}
    //end request (if orphan and write-buffer is empty)
    if(!NBStopFlag_isMineActivated(opq->stopFlag) && NBHttpServiceResp_isSet(opq->resp.obj) && (NBHttpServiceResp_isOrphan(opq->resp.obj) || !NBHttpServiceConn_reqHasOwnerOpq_(opq)) && !NBHttpServiceResp_isWritePend(opq->resp.obj)){
        if(NBHttpServiceResp_isUpgradedRaw(opq->resp.obj)){
            //close connection
            PRINTF_INFO("NBHttpServiceConn, stopping-conn after raw-ended-and-flushed.\n");
            NBStopFlag_activate(opq->stopFlag);
        } else {
            //start next request
            NBObject_lock(opq);
            {
                NBHttpServiceConn_reqFeedRestartLockedOpq_(opq);
            }
            NBObject_unlock(opq);
        }
    }
	//tick
	if(!NBStopFlag_isMineActivated(opq->stopFlag)){
        const STNBTimestampMicro nowMicro = NBTimestampMicro_getMonotonicFast();
        const SI64 uDiff = (opq->req.ctx.ticks.isLastSet ? NBTimestampMicro_getDiffInUs(&opq->req.ctx.ticks.last, &nowMicro) : 0);
        //NBASSERT(uDiff >= 0)
        if(uDiff < 0){
            PRINTF_WARNING("NBHttpSericeConn, tick, NBTimestampMicro_getDiffInUs returned negative(%lld us).\n", uDiff);
        } else {
            //tick callback
            if(opq->req.lstnr.itf.httpReqTick != NULL && NBHttpServiceResp_isSet(opq->resp.obj)){
                opq->req.ctx.ticks.usAccum += uDiff;
                if(opq->req.ctx.ticks.usAccum >= (opq->req.ctx.ticks.msPerTick * 1000ULL)){
                    UI32 msPerTickNew = NB_HTTP_SERVICE_REQ_MS_NEXT_TICK_UNCHANGED;
                    //msPassed
                    const UI64 msPassed = (opq->req.ctx.ticks.usAccum / 1000ULL);
                    opq->req.ctx.ticks.usAccum %= 1000ULL; //keep remainig-us
                    //validating 1ms max-precision
#                   ifdef NB_CONFIG_INCLUDE_ASSERTS
                    /*if((opq->req.ctx.ticks.usAccum - (opq->req.ctx.ticks.msPerTick * 1000ULL)) > 1000ULL){
                     PRINTF_INFO("NBHttpServiceReq, %llu.%llu%llu of %dms since last tick (%llu%%).\n"
                     , (opq->req.ctx.ticks.usAccum / 1000ULL), (opq->req.ctx.ticks.usAccum % 1000ULL) / 100ULL, (opq->req.ctx.ticks.usAccum % 100ULL) / 10ULL
                     , opq->req.ctx.ticks.msPerTick
                     , 100ULL * opq->req.ctx.ticks.usAccum / (opq->req.ctx.ticks.msPerTick * 1000ULL)
                     );
                     }*/
#                   endif
                    //notify
                    (*opq->req.lstnr.itf.httpReqTick)(opq->req.ctx, nowMicro, msPassed, opq->req.ctx.ticks.msPerTick, &msPerTickNew, opq->req.lstnr.itfParam);
                    //save updated msTicks
                    if(msPerTickNew != NB_HTTP_SERVICE_REQ_MS_NEXT_TICK_UNCHANGED){
                        opq->req.ctx.ticks.msPerTick = msPerTickNew;
                    }
                }
            }
            //per-sec analysis
            {
                opq->state.lastSec.usecsAccum += uDiff;
                if(opq->state.lastSec.usecsAccum >= (1000000ULL)){
                    //analyze overqueueing (sending too slow)
                    if(!NBHttpServiceResp_isSet(opq->resp.obj)){
                        opq->state.secsOverQueueingAccum        = 0;
                        opq->state.lastSec.write.queuedTotal    = 0;
                        opq->state.lastSec.write.csmdTotal      = 0;
                    } else {
                        const UI64 queueTotal = NBHttpServiceResp_writeQueuedTotal(opq->resp.obj);
                        const UI64 csmdTotal  = NBHttpServiceResp_writeQueuedConsumed(opq->resp.obj);
                        if(opq->state.lastSec.write.queuedTotal <= queueTotal && opq->state.lastSec.write.csmdTotal <= csmdTotal){
                            const UI64 secQueued = (queueTotal - opq->state.lastSec.write.queuedTotal);
                            const UI64 secCsmd   = (csmdTotal - opq->state.lastSec.write.csmdTotal);
                            if(secQueued > secCsmd){
                                opq->state.secsOverQueueingAccum++;
                                //PRINTF_ERROR("NBHttpServiceConn, conn-overqueueing balance is +%d (of %d max).\n", opq->state.secsOverQueueingAccum, opq->cfg.limits.secsOverqueueing);
                                if(opq->cfg.limits.secsOverqueueing > 0 && opq->state.secsOverQueueingAccum > opq->cfg.limits.secsOverqueueing){
                                    //stop
                                    PRINTF_ERROR("NBHttpServiceConn, dropping conn-overqueueing balance is +%d (%d max) secs (network is too slow).\n", opq->state.secsOverQueueingAccum, opq->cfg.limits.secsOverqueueing);
                                    NBStopFlag_activate(opq->stopFlag);
                                }
                            } else if(secQueued < secCsmd && opq->state.secsOverQueueingAccum > 0){
                                opq->state.secsOverQueueingAccum--;
                            }
                        }
                        //keep last-sec values
                        opq->state.lastSec.write.queuedTotal = queueTotal;
                        opq->state.lastSec.write.csmdTotal = csmdTotal;
                    }
                    //keep remaining usecs
                    opq->state.lastSec.usecsAccum %= (1000000ULL);
                }
            }
        }
        opq->req.ctx.ticks.last = nowMicro;
        opq->req.ctx.ticks.isLastSet = TRUE;
	}
	//error
	if(pollMask & ENNBIOPollsterOpBits_ErrOrGone){
		//stop
		PRINTF_ERROR("NBHttpServiceConn, conn-error-or-gone.\n");
        NBStopFlag_activate(opq->stopFlag);
	}
    //stats
    if (netRecvTick > 0 || netSentBefore < opq->net.bytesSent) {
        NBObject_lock(opq);
        {
            opq->stats.upd.flow.bytes.in += netRecvTick;
            opq->stats.upd.flow.bytes.out += (opq->net.bytesSent - netSentBefore);
        }
        NBObject_unlock(opq);
    }
	//consume pend tasks
	/*if(NB_RTP_CLIENT_PORT_HAS_PEND_TASK(p)){
		NBRtpClient_portConsumePendTasksOpq_(opq, p);
	}*/
	//return
	dstUpd->opsMasks = ENNBIOPollsterOpBit_Read | ((opq->net.ssl.cmd.cur != ENNBHttpSslCmd_None && opq->net.ssl.cmd.result == ENNBSslResult_ErrWantWrite) || (opq->net.ssl.cmd.cur == ENNBHttpSslCmd_None && opq->req.ctx.req.body.isCompleted && NBHttpServiceResp_isSet(opq->resp.obj) && NBHttpServiceResp_isWritePend(opq->resp.obj)) ? ENNBIOPollsterOpBit_Write : 0);
    if(dstUpd->opsMasks & ENNBIOPollsterOpBit_Write){
        //PRINTF_INFO("NBHttpServiceConn, waiting for write.\n");
    } else {
        //PRINTF_INFO("NBHttpServiceConn, not-waiting for write.\n");
    }
	//consume stopFlag
	if(NBStopFlag_isAnyActivated(opq->stopFlag)){
        //invalidate current response
        if(NBHttpServiceResp_isSet(opq->resp.obj)){
            NBHttpServiceResp_invalidate(opq->resp.obj);
        }
        //remove from pollster
        if(NBIOPollsterSync_isSet(dstSync)){
            NBASSERT(opq->pollster.isAdded)
            if(NBSsl_isSet(opq->net.ssl.conn)){
                PRINTF_INFO("NBHttpServiceConn, stopping after %d bytes rcvd (%d encrypted), %d bytes sent (%d encrypted).\n", opq->net.ssl.buffs.totals.filled, opq->net.buffs.totals.filled, opq->net.ssl.bytesSent, opq->net.bytesSent);
            } else {
                PRINTF_INFO("NBHttpServiceConn, stopping after %d bytes rcvd and %d bytes sent.\n", opq->net.buffs.totals.filled, opq->net.bytesSent);
            }
            NBIOPollsterSync_removeIOLnk(dstSync, &ioLnk);
        }
	}
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	{
		timeCur	= NBTimestampMicro_getMonotonicFast();
		usDiff	= NBTimestampMicro_getDiffInUs(&timeLastAction, &timeCur);
		/*if(usDiff >= 1000ULL){
			PRINTF_INFO("NBHttpServiceConn, pollConsumeMask(%s%s%s) took %llu.%llu%llums.\n", (pollMask & ENNBIOPollsterOpBit_Read ? "+read" : ""), (pollMask & ENNBIOPollsterOpBit_Write ? "+write" : ""), (pollMask & ENNBIOPollsterOpBits_ErrOrGone ? "+errOrGone" : ""), (usDiff / 1000ULL), (usDiff % 1000ULL) % 100ULL, (usDiff % 100ULL) % 10ULL);
		}*/
		timeLastAction = timeCur;
	}
#	endif
}

void NBHttpServiceConn_pollConsumeNoOp_(STNBIOLnk ioLnk, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData){
	NBHttpServiceConn_pollConsumeMask_(ioLnk, 0, dstUpd, dstSync, usrData);
}

void NBHttpServiceConn_pollRemoved_(STNBIOLnk ioLnk, void* usrData){
	STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)usrData;
	//PRINTF_INFO("NBHttpServiceConn, client safetly-removed from pollster.\n");
    PRINTF_INFO("NBHttpServiceConn_pollRemoved_ to '%lld'.\n", (SI64)opq);
    NBObject_lock(opq);
    {
        //remove request
        NBHttpServiceConn_reqFeedRestartLockedOpq_(opq);
        //
        NBASSERT(!NBHttpServiceConn_reqHasOwnerOpq_(opq))
        NBASSERT(!NBHttpServiceResp_isSet(opq->resp.obj))
        //change 'isAdded'
        NBASSERT(opq->pollster.isAdded) //must be added before
        opq->pollster.isAdded = FALSE;
        NBThreadCond_broadcast(&opq->cond); //notify 'isAdded' change
    }
    NBObject_unlock(opq);
    //notify lstnr (unlocked)
    if(opq->lstnr.itf.httpConnStopped != NULL){
        (*opq->lstnr.itf.httpConnStopped)(NBHttpServiceConn_fromOpqPtr(opq), opq->lstnr.usrData);
    }
}

//socket sender callbacks

SI32 NBHttpServiceConn_httpReqSend_(STNBHttpServiceRespRef req, void* data, const SI32 dataSz, void* usrData){
	SI32 r = -1;
	STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)usrData;
    NBASSERT(dataSz > 0)
    //do
    if(NBSsl_isSet(opq->net.ssl.conn)){
        NBASSERT(opq->net.ssl.cmd.cur == ENNBHttpSslCmd_None || opq->net.ssl.cmd.cur == ENNBHttpSslCmd_Write) //must be first write or a continuation
        //Secure send
        r = NBSsl_write(opq->net.ssl.conn, data, dataSz);
        if(r < 0){
            //results
            switch (r) {
                case ENNBSslResult_ErrWantRead:
                    PRINTF_INFO("NBHttpServiceConn, NBSsl_write hungry-for-read.\n");
                    opq->net.ssl.cmd.cur = ENNBHttpSslCmd_Write;
                    opq->net.ssl.cmd.result = (ENNBSslResult)r;
                    break;
                case ENNBSslResult_ErrWantWrite:
                    PRINTF_INFO("NBHttpServiceConn, NBSsl_write hungry-for-write.\n");
                    opq->net.ssl.cmd.cur = ENNBHttpSslCmd_Write;
                    opq->net.ssl.cmd.result = (ENNBSslResult)r;
                    break;
                default:
                    PRINTF_ERROR("NBHttpServiceConn, NBSsl_write failed with (%d) plain.\n", r);
                    NBASSERT(r == ENNBSslResult_Error)
                    opq->net.ssl.cmd.cur = ENNBHttpSslCmd_None;
                    opq->net.ssl.cmd.result = ENNBSslResult_Error;
                    NBStopFlag_activate(opq->stopFlag);
                    break;
            }
        } else {
            //PRINTF_INFO("NBHttpServiceConn, NBSsl_write completed.\n");
            opq->net.ssl.bytesSent += r;
            if(r < dataSz){
                opq->net.ssl.isSendBlocked = TRUE;
            }
            opq->net.ssl.cmd.cur = ENNBHttpSslCmd_None;
            opq->net.ssl.cmd.result = ENNBSslResult_Success;
            //print
            /*{
                STNBString str;
                NBString_initWithStrBytes(&str, (const char*) data, dataSz);
                PRINTF_INFO("NBHttpServiceConn, %d of %d bytes fed-to-ssl:--->\n%s<---\n", r, dataSz, str.str);
                NBString_release(&str);
            }*/
        }
    } else {
        //Plain send
        r = NBSocket_send(opq->net.socket, data, dataSz);
        if(r < 0){
            PRINTF_ERROR("NBHttpServiceConn, NBSocket_send failed with (%d) plain.\n", r);
            NBStopFlag_activate(opq->stopFlag);
        } else {
            opq->net.bytesSent += r;
            if(r < dataSz){
                opq->net.isSendBlocked = TRUE;
            }
            /*{
                STNBString str;
                NBString_initWithStrBytes(&str, (const char*) data, dataSz);
                //PRINTF_INFO("NBHttpServiceConn, %d of %d bytes sent:--->\n'%s'<---\n", r, dataSz, str.str);
                NBString_release(&str);
            }*/
            //PRINTF_INFO("NBHttpServiceConn, %d of %d bytes sent\n", r, dataSz);
        }
    }
	return r;
}

//Commands

void NBHttpServiceConn_consumePendTaskStatsFlushLockedOpq_(STNBHttpServiceConnOpq* opq){
	if(NBHttpStats_isSet(opq->stats.provider)){
		NBHttpStats_accumData(opq->stats.provider, &opq->stats.upd);
        NBHttpStats_flush(opq->stats.provider); //push pend-data to parents
	}
	NBStruct_stRelease(NBHttpStatsData_getSharedStructMap(), &opq->stats.upd, sizeof(opq->stats.upd));
	//set as current
	opq->stats.flush.iSeqDone = opq->stats.flush.iSeqReq;
}

void NBHttpServiceConn_consumePendTasksLockedOpq_(STNBHttpServiceConnOpq* opq){
	if(NB_HTTP_SERVICE_CONN_HAS_PEND_TASK_FLUSH_STATS(opq)){
		NBHttpServiceConn_consumePendTaskStatsFlushLockedOpq_(opq);
	}
}

//Commands

BOOL NBHttpServiceConn_statsFlushStart(STNBHttpServiceConnRef ref, const UI32 iSeq){
	BOOL r = FALSE;
	STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)ref.opaque; NBASSERT(NBHttpServiceConn_isClass(ref))
	NBASSERT(opq != NULL)
	NBObject_lock(opq);
	if(!NBStopFlag_isMineActivated(opq->stopFlag) && opq->stats.flush.iSeqReq < iSeq){
		opq->stats.flush.iSeqReq = iSeq;
		//flush here
		if(!r){
			if(NB_HTTP_SERVICE_CONN_HAS_PEND_TASK_FLUSH_STATS(opq)){
				NBHttpServiceConn_consumePendTaskStatsFlushLockedOpq_(opq);
				NBASSERT(!NB_HTTP_SERVICE_CONN_HAS_PEND_TASK_FLUSH_STATS(opq))
			}
			opq->stats.flush.iSeqDone = opq->stats.flush.iSeqReq;
		}
	}
	NBObject_unlock(opq);
	return r;
}

BOOL NBHttpServiceConn_statsFlushIsPend(STNBHttpServiceConnRef ref, const UI32 iSeq){
	BOOL r = FALSE;
	STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)ref.opaque; NBASSERT(NBHttpServiceConn_isClass(ref))
	NBASSERT(opq != NULL)
	NBObject_lock(opq);
	{
		//flush here
		if(!r){
			if(NB_HTTP_SERVICE_CONN_HAS_PEND_TASK_FLUSH_STATS(opq)){
				NBHttpServiceConn_consumePendTaskStatsFlushLockedOpq_(opq);
				NBASSERT(!NB_HTTP_SERVICE_CONN_HAS_PEND_TASK_FLUSH_STATS(opq))
			}
			opq->stats.flush.iSeqDone = opq->stats.flush.iSeqReq;
		}
	}
	NBObject_unlock(opq);
	return r;
}

void NBHttpServiceConn_statsGet(STNBHttpServiceConnRef ref, STNBHttpStatsData* dst, const BOOL resetAccum){
	STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)ref.opaque; NBASSERT(NBHttpServiceConn_isClass(ref))
	NBASSERT(opq != NULL)
	NBObject_lock(opq);
	if(NBHttpStats_isSet(opq->stats.provider)){
		NBHttpStats_getData(opq->stats.provider, dst, resetAccum);
	}
	NBObject_unlock(opq);
}

//NBHttpServiceReqArrivalLnkItf

UI32 NBHttpServiceConn_httpReqGetDefaultResponseCode_(STNBString* dstReason, void* usrData){ //get request's default response code
    UI32 r = 0;
    STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)usrData; NBASSERT(NBHttpServiceConn_isClass(NBObjRef_fromOpqPtr(opq)))
    {
        if(dstReason != NULL){
            NBString_set(dstReason, opq->resp.def.header.reason.str);
        }
        r = opq->resp.def.header.code;
    }
    return r;
}

BOOL NBHttpServiceConn_httpReqSetDefaultResponseCode_(const UI32 code, const char* reason, void* usrData){ //set request's default response code
    BOOL r = FALSE;
    STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)usrData; NBASSERT(NBHttpServiceConn_isClass(NBObjRef_fromOpqPtr(opq)))
    {
        NBObject_lock(opq);
        {
            opq->resp.def.header.code = code;
            NBString_set(&opq->resp.def.header.reason, reason);
            r = TRUE;
        }
        NBObject_unlock(opq);
    }
    return r;
}

BOOL NBHttpServiceConn_httpReqAddDefaultResponseField_(const char* name, const char* value, void* usrData){   //adds a header-field's for the default response without checking (allows duplicated fields)
    BOOL r = FALSE;
    STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)usrData; NBASSERT(NBHttpServiceConn_isClass(NBObjRef_fromOpqPtr(opq)))
    if(!NBString_strIsEmpty(name)){
        NBObject_lock(opq);
        {
            //add field
            STNBHttpHeaderField fld;
            NBMemory_setZeroSt(fld, STNBHttpHeaderField);
            //add name to buffer
            fld.name = opq->resp.def.header.fields.str.length;
            NBString_concat(&opq->resp.def.header.fields.str, name);
            NBString_concatByte(&opq->resp.def.header.fields.str, '\0');
            //add value to buffer
            fld.value = opq->resp.def.header.fields.str.length;
            if(!NBString_strIsEmpty(value)){
                NBString_concat(&opq->resp.def.header.fields.str, value);
            }
            NBString_concatByte(&opq->resp.def.header.fields.str, '\0');
            //
            NBArray_addValue(&opq->resp.def.header.fields.arr, fld);
            r = TRUE;
        }
        NBObject_unlock(opq);
    }
    return r;
}

BOOL NBHttpServiceConn_httpReqSetDefaultResponseField_(const char* name, const char* value, void* usrData){   //adds or updates a header-field's for the default response without checking (wont allow duplicated fields)
    BOOL r = FALSE;
    STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)usrData; NBASSERT(NBHttpServiceConn_isClass(NBObjRef_fromOpqPtr(opq)))
    if(!NBString_strIsEmpty(name)){
        NBObject_lock(opq);
        {
            SI32 iValue = 0;
            //add value to buffer
            {
                iValue = opq->resp.def.header.fields.str.length;
                if(!NBString_strIsEmpty(value)){
                    NBString_concat(&opq->resp.def.header.fields.str, value);
                }
                NBString_concatByte(&opq->resp.def.header.fields.str, '\0');
            }
            //search
            SI32 fndCount = 0;
            SI32 i; for(i = 0; i < opq->resp.def.header.fields.arr.use; i++){
                STNBHttpHeaderField* fld = NBArray_itmPtrAtIndex(&opq->resp.def.header.fields.arr, STNBHttpHeaderField, i);
                if(NBString_strIsEqual(name, &opq->resp.def.header.fields.str.str[fld->name])){
                    //set value previously added to buffer
                    fld->value = iValue;
                    fndCount++;
                }
            }
            //add field (if not found)
            if(fndCount == 0){
                STNBHttpHeaderField fld;
                NBMemory_setZeroSt(fld, STNBHttpHeaderField);
                //add name to buffer
                fld.name = opq->resp.def.header.fields.str.length;
                NBString_concat(&opq->resp.def.header.fields.str, name);
                NBString_concatByte(&opq->resp.def.header.fields.str, '\0');
                //set value previously added to buffer
                fld.value = iValue;
                //
                NBArray_addValue(&opq->resp.def.header.fields.arr, fld);
            }
            r = TRUE;
        }
        NBObject_unlock(opq);
    }
    return r;
}


BOOL NBHttpServiceConn_httpReqSetDefaultResponseBodyStr_(const char* body, void* usrData){   //set request's default response body
    BOOL r = FALSE;
    STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)usrData; NBASSERT(NBHttpServiceConn_isClass(NBObjRef_fromOpqPtr(opq)))
    {
        NBObject_lock(opq);
        {
            NBString_set(&opq->resp.def.body, body);
            r = TRUE;
        }
        NBObject_unlock(opq);
    }
    return r;
}

BOOL NBHttpServiceConn_httpReqSetDefaultResponseBodyData_(const void* data, const UI32 dataSz, void* usrData){   //set request's default response body
    BOOL r = FALSE;
    STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)usrData; NBASSERT(NBHttpServiceConn_isClass(NBObjRef_fromOpqPtr(opq)))
    {
        NBObject_lock(opq);
        {
            NBString_setBytes(&opq->resp.def.body, (const char*)data, dataSz);
            r = TRUE;
        }
        NBObject_unlock(opq);
    }
    return r;
}

BOOL NBHttpServiceConn_httpReqConcatDefaultResponseBodyStruct_(const STNBStructMap* structMap, const void* src, const UI32 srcSz, void* usrData){   //set request's default response body
    BOOL r = FALSE;
    STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)usrData; NBASSERT(NBHttpServiceConn_isClass(NBObjRef_fromOpqPtr(opq)))
    {
        NBObject_lock(opq);
        {
            r = NBStruct_stConcatAsJson(&opq->resp.def.body, structMap, src, srcSz);
        }
        NBObject_unlock(opq);
    }
    return r;
}

BOOL NBHttpServiceConn_httpReqSetOwner_(const STNBHttpServiceReqLstnrItf* itf, void* itfParam, const UI32 msPerTick, void* usrData){ //set request owner
    BOOL r = FALSE;
    STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)usrData; NBASSERT(NBHttpServiceConn_isClass(NBObjRef_fromOpqPtr(opq)))
    if(itf != NULL){
        NBObject_lock(opq);
        {
            NBHttpServiceConn_reqRespStartLockedOpq_(opq, itf, itfParam, msPerTick);
            r = TRUE;
        }
        NBObject_unlock(opq);
    }
    return r;
}

//NBHttpMessage listening

BOOL NBHttpServiceConn_consumeHeadStartLine_(const STNBHttpMessage* obj, void* lparam){
    STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)lparam; NBASSERT(NBHttpServiceConn_isClass(NBObjRef_fromOpqPtr(opq)))
    //
    //PRINTF_INFO("NBHttpServiceConn, consumeHeadStartLine_.\n");
    //apply
    NBASSERT(!opq->req.ctx.req.header.isCompleted)  //header should not be completed
    NBASSERT(!opq->req.ctx.req.body.isCompleted)    //body should not be completed
    NBASSERT(opq->req.ctx.req.header.ref == NULL)   //should be unset
    //
    opq->req.ctx.req.header.ref = &opq->req.parser.http.header;
    //
    //define ownership
    if(opq->lstnr.itf.httpConnReqArrived != NULL && !NBHttpServiceConn_reqHasOwnerOpq_(opq)){
        STNBHttpServiceReqDesc reqDesc; STNBHttpServiceReqArrivalLnk reqLnk;
        NBObject_lock(opq);
        {
            NBHttpServiceConn_reqGetDescLockedOpq_(opq, &reqDesc);
            NBHttpServiceConn_reqGetArrivalLnkLockedOpq_(opq, &reqLnk);
        }
        NBObject_unlock(opq);
        if((*opq->lstnr.itf.httpConnReqArrived)(NBObjRef_fromOpqPtr(opq), reqDesc, reqLnk, opq->lstnr.usrData)){
            //
        }
    }
    return TRUE;
}

BOOL NBHttpServiceConn_consumeHeadFieldLine_(const STNBHttpMessage* obj, const STNBHttpHeaderField* field, void* lparam){
    BOOL r = TRUE; //default is success
    STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)lparam; NBASSERT(NBHttpServiceConn_isClass(NBObjRef_fromOpqPtr(opq)))
    //
    //PRINTF_INFO("NBHttpServiceConn, consumeHeadFieldLine_.\n");
    //apply
    NBASSERT(!opq->req.ctx.req.header.isCompleted) //header should not be completed
    NBASSERT(!opq->req.ctx.req.body.isCompleted) //body should not be completed
    //
    opq->req.ctx.req.header.ref = &opq->req.parser.http.header;
    //
    //notify
    //notify request owner
    if(opq->req.lstnr.itf.httpReqConsumeHeadFieldLine != NULL && NBHttpServiceResp_isSet(opq->resp.obj)){
        //The context is passed as copy to reduce pointer exposition.
        r = (*opq->req.lstnr.itf.httpReqConsumeHeadFieldLine)(opq->req.ctx, field, &opq->req.ctx.resp.lnk.itf);
    }
    return r;
}

BOOL NBHttpServiceConn_consumeHeadEnd_(const STNBHttpMessage* obj, UI32* dstBodyBuffSz, void* lparam){
    BOOL r = TRUE; //default is success
    STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)lparam; NBASSERT(NBHttpServiceConn_isClass(NBObjRef_fromOpqPtr(opq)))
    //
    //PRINTF_INFO("NBHttpServiceConn, consumeHeadEnd_.\n");
    //apply
    NBASSERT(!opq->req.ctx.req.header.isCompleted) //header should not be completed
    NBASSERT(!opq->req.ctx.req.body.isCompleted) //body should not be completed
    //
    opq->req.ctx.req.header.isCompleted = TRUE;
    opq->req.ctx.req.header.ref = &opq->req.parser.http.header;
    //
    //define ownership
    if(opq->lstnr.itf.httpConnReqArrived != NULL && !NBHttpServiceConn_reqHasOwnerOpq_(opq)){
        STNBHttpServiceReqDesc reqDesc; STNBHttpServiceReqArrivalLnk reqLnk;
        NBObject_lock(opq);
        {
            NBHttpServiceConn_reqGetDescLockedOpq_(opq, &reqDesc);
            NBHttpServiceConn_reqGetArrivalLnkLockedOpq_(opq, &reqLnk);
        }
        NBObject_unlock(opq);
        if((*opq->lstnr.itf.httpConnReqArrived)(NBObjRef_fromOpqPtr(opq), reqDesc, reqLnk, opq->lstnr.usrData)){
            //
        }
    }
    //notify request owner
    if(opq->req.lstnr.itf.httpReqConsumeHeadEnd != NULL && NBHttpServiceResp_isSet(opq->resp.obj)){
        //The context is passed as copy to reduce pointer exposition.
        r = (*opq->req.lstnr.itf.httpReqConsumeHeadEnd)(opq->req.ctx, dstBodyBuffSz, opq->req.lstnr.itfParam);
    }
    return r;
}

BOOL NBHttpServiceConn_consumeBodyData_(const STNBHttpMessage* obj, const void* data, const UI64 dataSz, void* lparam){
    BOOL r = TRUE; //default is success
    STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)lparam; NBASSERT(NBHttpServiceConn_isClass(NBObjRef_fromOpqPtr(opq)))
    //
    //PRINTF_INFO("NBHttpServiceConn, consumeBodyData_.\n");
    //apply
    NBASSERT(opq->req.ctx.req.header.isCompleted) //header should be completed
    NBASSERT(!opq->req.ctx.req.body.isCompleted) //body should not be completed
    //
    opq->req.ctx.req.body.ref = &opq->req.parser.http.body;
    //
    //notify request owner
    if(opq->req.lstnr.itf.httpReqConsumeBodyData != NULL && NBHttpServiceResp_isSet(opq->resp.obj)){
        //The context is passed as copy to reduce pointer exposition.
        r = (*opq->req.lstnr.itf.httpReqConsumeBodyData)(opq->req.ctx, data, dataSz, opq->req.lstnr.itfParam);
    }
    return r;
}

BOOL NBHttpServiceConn_consumeBodyTrailerField_(const STNBHttpMessage* obj, const STNBHttpBodyField* field, void* lparam){
    BOOL r = TRUE; //default is success
    STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)lparam; NBASSERT(NBHttpServiceConn_isClass(NBObjRef_fromOpqPtr(opq)))
    //
    //PRINTF_INFO("NBHttpServiceConn, consumeBodyTrailerField_.\n");
    //apply
    NBASSERT(opq->req.ctx.req.header.isCompleted) //header should be completed
    NBASSERT(!opq->req.ctx.req.body.isCompleted) //body should not be completed
    //
    opq->req.ctx.req.body.ref = &opq->req.parser.http.body;
    //
    //notify request owner
    if(opq->req.lstnr.itf.httpReqConsumeBodyTrailerField != NULL && NBHttpServiceResp_isSet(opq->resp.obj)){
        //The context is passed as copy to reduce pointer exposition.
        r = (*opq->req.lstnr.itf.httpReqConsumeBodyTrailerField)(opq->req.ctx, field, opq->req.lstnr.itfParam);
    }
    return r;
}

BOOL NBHttpServiceConn_consumeBodyEnd_(const STNBHttpMessage* obj, void* lparam){
    BOOL r = TRUE; //default is success
    STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)lparam; NBASSERT(NBHttpServiceConn_isClass(NBObjRef_fromOpqPtr(opq)))
    //
    //PRINTF_INFO("NBHttpServiceConn, consumeBodyEnd_.\n");
    //apply
    NBASSERT(opq->req.ctx.req.header.isCompleted) //header should be completed
    NBASSERT(!opq->req.ctx.req.body.isCompleted) //body should not be completed
    //
    opq->req.ctx.req.body.isCompleted = TRUE;
    opq->req.ctx.req.body.ref = &opq->req.parser.http.body;
    //
    //define ownership
    if(opq->lstnr.itf.httpConnReqArrived != NULL && !NBHttpServiceConn_reqHasOwnerOpq_(opq)){
        STNBHttpServiceReqDesc reqDesc; STNBHttpServiceReqArrivalLnk reqLnk;
        NBObject_lock(opq);
        {
            NBHttpServiceConn_reqGetDescLockedOpq_(opq, &reqDesc);
            NBHttpServiceConn_reqGetArrivalLnkLockedOpq_(opq, &reqLnk);
        }
        NBObject_unlock(opq);
        if((*opq->lstnr.itf.httpConnReqArrived)(NBObjRef_fromOpqPtr(opq), reqDesc, reqLnk, opq->lstnr.usrData)){
            //
        }
    }
    //notify request owner
    if(opq->req.lstnr.itf.httpReqConsumeBodyEnd != NULL && NBHttpServiceResp_isSet(opq->resp.obj)){
        //The context is passed as copy to reduce pointer exposition.
        r = (*opq->req.lstnr.itf.httpReqConsumeBodyEnd)(opq->req.ctx, opq->req.lstnr.itfParam);
    }
    //set default response (if no owner was set)
    if(!NBHttpServiceResp_isSet(opq->resp.obj)){
        NBObject_lock(opq);
        {
            opq->resp.obj = NBHttpServiceResp_alloc(NULL);
            opq->state.secsOverQueueingAccum        = 0;
            opq->state.lastSec.write.queuedTotal    = 0;
            opq->state.lastSec.write.csmdTotal      = 0;
            if(opq->resp.def.header.code <= 0){
                NBHttpServiceResp_setResponseCode(opq->resp.obj, 404, "Not implemented");
                NBHttpServiceResp_setContentLength(opq->resp.obj, 0);
                NBHttpServiceResp_flushBody(opq->resp.obj);
            } else {
                NBHttpServiceResp_setResponseCode(opq->resp.obj, opq->resp.def.header.code, opq->resp.def.header.reason.str);
                {
                    SI32 i; for(i = 0; i < opq->resp.def.header.fields.arr.use; i++){
                        STNBHttpHeaderField* fld = NBArray_itmPtrAtIndex(&opq->resp.def.header.fields.arr, STNBHttpHeaderField, i);
                        NBHttpServiceResp_addHeaderField(opq->resp.obj, &opq->resp.def.header.fields.str.str[fld->name], &opq->resp.def.header.fields.str.str[fld->value]);
                    }
                }
                NBHttpServiceResp_setContentLength(opq->resp.obj, opq->resp.def.body.length);
                NBHttpServiceResp_concatBodyBytes(opq->resp.obj, opq->resp.def.body.str, opq->resp.def.body.length);
                NBHttpServiceResp_flushBody(opq->resp.obj);
            }
        }
        NBObject_unlock(opq);
    }
    return r;
}

//NBHttpServiceReqArrivalLnk

UI32 NBHttpServiceReqArrivalLnk_getDefaultResponseCode(STNBHttpServiceReqArrivalLnk* obj){
    UI32 r = 0;
    if(obj->itf.httpReqGetDefaultResponseCode != NULL){
        r = (*obj->itf.httpReqGetDefaultResponseCode)(NULL, obj->usrParam);
    }
    return r;
}

UI32 NBHttpServiceReqArrivalLnk_getDefaultResponseCodeAndReason(STNBHttpServiceReqArrivalLnk* obj, STNBString* optDstReason){
    UI32 r = 0;
    if(obj->itf.httpReqGetDefaultResponseCode != NULL){
        r = (*obj->itf.httpReqGetDefaultResponseCode)(optDstReason, obj->usrParam);
    }
    return r;
}

BOOL NBHttpServiceReqArrivalLnk_setDefaultResponseCode(STNBHttpServiceReqArrivalLnk* obj, const UI32 code, const char* reason){
    BOOL r = FALSE;
    if(obj->itf.httpReqSetDefaultResponseCode != NULL){
        r = (*obj->itf.httpReqSetDefaultResponseCode)(code, reason, obj->usrParam);
    }
    return r;
}

BOOL NBHttpServiceReqArrivalLnk_addDefaultResponseField(STNBHttpServiceReqArrivalLnk* obj, const char* name, const char* value){
    BOOL r = FALSE;
    if(obj->itf.httpReqAddDefaultResponseField != NULL){
        r = (*obj->itf.httpReqAddDefaultResponseField)(name, value, obj->usrParam);
    }
    return r;
}

BOOL NBHttpServiceReqArrivalLnk_setDefaultResponseField(STNBHttpServiceReqArrivalLnk* obj, const char* name, const char* value){
    BOOL r = FALSE;
    if(obj->itf.httpReqSetDefaultResponseField != NULL){
        r = (*obj->itf.httpReqSetDefaultResponseField)(name, value, obj->usrParam);
    }
    return r;
}

BOOL NBHttpServiceReqArrivalLnk_setDefaultResponseBodyStr(STNBHttpServiceReqArrivalLnk* obj, const char* body){
    BOOL r = FALSE;
    if(obj->itf.httpReqSetDefaultResponseBodyStr != NULL){
        r = (*obj->itf.httpReqSetDefaultResponseBodyStr)(body, obj->usrParam);
    }
    return r;
}

BOOL NBHttpServiceReqArrivalLnk_setDefaultResponseBodyData(STNBHttpServiceReqArrivalLnk* obj, const void* data, const UI32 dataSz){
    BOOL r = FALSE;
    if(obj->itf.httpReqSetDefaultResponseBodyData != NULL){
        r = (*obj->itf.httpReqSetDefaultResponseBodyData)(data, dataSz, obj->usrParam);
    }
    return r;
}

BOOL NBHttpServiceReqArrivalLnk_concatDefaultResponseBodyStruct(STNBHttpServiceReqArrivalLnk* obj, const STNBStructMap* structMap, const void* src, const UI32 srcSz){
    BOOL r = FALSE;
    if(obj->itf.httpReqConcatDefaultResponseBodyStruct != NULL){
        r = (*obj->itf.httpReqConcatDefaultResponseBodyStruct)(structMap, src, srcSz, obj->usrParam);
    }
    return r;
}

BOOL NBHttpServiceReqArrivalLnk_setOwner(STNBHttpServiceReqArrivalLnk* obj, const STNBHttpServiceReqLstnrItf* itf, void* itfParam, const UI32 msPerTick){
    BOOL r = FALSE;
    if(obj->itf.httpReqSetOwner != NULL){
        r = (*obj->itf.httpReqSetOwner)(itf, itfParam, msPerTick, obj->usrParam);
    }
    return r;
}

//NBIOLink

BOOL NBHttpServiceConn_ssl_ioIsObjRef_(STNBObjRef objRef, void* usrData){
    STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)usrData; NBASSERT(NBHttpServiceConn_isClass(NBObjRef_fromOpqPtr(opq)))
    //ToDo: analyze parent
    return FALSE;
}

SI32 NBHttpServiceConn_ssl_ioRead_(void* dst, const SI32 dstSz, void* usrData){ //read data to destination buffer, returns the ammount of bytes read, negative in case of error
    SI32 r = -1;
    STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)usrData; NBASSERT(NBHttpServiceConn_isClass(NBObjRef_fromOpqPtr(opq)))
    if(!NBStopFlag_isMineActivated(opq->stopFlag) && dst != NULL && dstSz >= 0){
        STNBHttpServiceConnBuffs* buffs = &opq->net.buffs; //final-buff if no-ssl, encrypted-buff if ssl
        const SI32 avail = NBHttpServiceConnBuffs_readAvailSz(buffs);
        r = (avail <= 0 ? 0 : dstSz <= avail ? dstSz : avail);
        if(r > 0){
            NBMemory_copy(dst, NBHttpServiceConnBuffs_readAvailPtr(buffs), r);
            //PRINTF_INFO("NBHttpServiceConn, encrypted-ioRead did (%d of %d bytes from buffer).\n", r, dstSz);
        } else {
            //PRINTF_INFO("NBHttpServiceConn, encrypted-ioRead did (%d of %d bytes from buffer).\n", r, dstSz);
        }
        //consume buffer
        NBHttpServiceConnBuffs_readMoveCursor(buffs, r);
    }
    return r;
}

SI32 NBHttpServiceConn_ssl_ioWrite_(const void* src, const SI32 srcSz, void* usrData){ //write data from source buffer, returns the ammount of bytes written, negative in case of error
    SI32 r = -1;
    STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)usrData; NBASSERT(NBHttpServiceConn_isClass(NBObjRef_fromOpqPtr(opq)))
    if(!NBStopFlag_isMineActivated(opq->stopFlag)){
        if(srcSz == 0){
            r = 0;
        } else if(srcSz > 0 && src != NULL){
            //Plain send
            r = NBSocket_send(opq->net.socket, src, srcSz);
            if(r < 0){
                //PRINTF_ERROR("NBHttpServiceConn, encrypted-ioWrite did (%d of %d bytes).\n", r, srcSz);
                NBStopFlag_activate(opq->stopFlag);
            } else {
                opq->net.bytesSent += r;
                if(r < srcSz){
                    opq->net.isSendBlocked = TRUE;
                    opq->net.ssl.isSendBlocked = TRUE;
                }
                //PRINTF_INFO("NBHttpServiceConn, encrypted-ioWrite did (%d of %d bytes).\n", r, srcSz);
            }
        }
    }
    return r;
}

void NBHttpServiceConn_ssl_ioFlush_(void* usrData){ //flush write-data
    STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)usrData; NBASSERT(NBHttpServiceConn_isClass(NBObjRef_fromOpqPtr(opq)))
    {
        NBASSERT(FALSE) //ToDo: implement
    }
}

void NBHttpServiceConn_ssl_ioShutdown_(const UI8 mask, void* usrData){ //close gracefully (if posible)
    STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)usrData; NBASSERT(NBHttpServiceConn_isClass(NBObjRef_fromOpqPtr(opq)))
    {
        NBStopFlag_activate(opq->stopFlag);
    }
}

void NBHttpServiceConn_ssl_ioClose_(void* usrData){ //close ungracefully
    STNBHttpServiceConnOpq* opq = (STNBHttpServiceConnOpq*)usrData; NBASSERT(NBHttpServiceConn_isClass(NBObjRef_fromOpqPtr(opq)))
    {
        NBStopFlag_activate(opq->stopFlag);
    }
}


//NBHttpServiceConnBuffs


void NBHttpServiceConnBuffs_init(STNBHttpServiceConnBuffs* obj){
    NBMemory_setZeroSt(*obj, STNBHttpServiceConnBuffs);
}

void NBHttpServiceConnBuffs_release(STNBHttpServiceConnBuffs* obj){
    obj->fill = obj->read = NULL;
    //allocs
    {
        //buff0
        {
            STNBHttpServiceConnBuff* buff = &obj->allocs.buff0;   //final-buff if no-ssl, encrypted-buff if ssl
            if(buff->data != NULL){
                NBMemory_free(buff->data);
                buff->data = NULL;
            }
            buff->filled = buff->csmd = buff->size = 0;
        }
        //buff1
        {
            STNBHttpServiceConnBuff* buff = &obj->allocs.buff1;   //final-buff if no-ssl, encrypted-buff if ssl
            if(buff->data != NULL){
                NBMemory_free(buff->data);
                buff->data = NULL;
            }
            buff->filled = buff->csmd = buff->size = 0;
        }
    }
}

void NBHttpServiceConnBuffs_create(STNBHttpServiceConnBuffs* obj, const SI32 buffSize){
    //buff0
    {
        STNBHttpServiceConnBuff* buff = &obj->allocs.buff0; //final-buff if no-ssl, encrypted-buff if ssl
        if(buff->size != buffSize){
            if(buff->data != NULL){
                NBMemory_free(buff->data);
                buff->data = NULL;
                buff->size = 0;
            }
            if(buffSize > 0){
                buff->size = buffSize;
#           ifdef NB_CONFIG_INCLUDE_ASSERTS
                buff->data = (BYTE*)NBMemory_alloc(buff->size + 1); //+1 for a '\0'
#           else
                buff->data = (BYTE*)NBMemory_alloc(buff->size);
#           endif
            }
        }
        buff->filled = buff->csmd = 0;
    }
    //buff1
    {
        STNBHttpServiceConnBuff* buff = &obj->allocs.buff1; //final-buff if no-ssl, encrypted-buff if ssl
        if(buff->size != buffSize){
            if(buff->data != NULL){
                NBMemory_free(buff->data);
                buff->data = NULL;
                buff->size = 0;
            }
            if(buffSize > 0){
                buff->size = buffSize;
#           ifdef NB_CONFIG_INCLUDE_ASSERTS
                buff->data = (BYTE*)NBMemory_alloc(buff->size + 1); //+1 for a '\0'
#           else
                buff->data = (BYTE*)NBMemory_alloc(buff->size);
#           endif
            }
        }
        buff->filled = buff->csmd = 0;
    }
    //activate firtst buffer
    obj->fill = obj->read = &obj->allocs.buff0;
}

void NBHttpServiceConnBuffs_fillMoveCursor(STNBHttpServiceConnBuffs* obj, const SI32 moveSz){
    if(moveSz > 0){
        obj->fill->filled += moveSz;
        obj->totals.filled += moveSz;
        //swap fill-buffer
        NBASSERT(obj->read->filled <= obj->read->size)
        if(obj->read->filled == obj->read->size || obj->fill == obj->read){ //this allows two full-size read calls, the third and incoming read-calls are done over the remaining space on th second buffer.
            STNBHttpServiceConnBuff* nxt = (obj->fill == &obj->allocs.buff0 ? &obj->allocs.buff1 : &obj->allocs.buff0);
            if(nxt != obj->read){
                nxt->csmd = nxt->filled = 0;
                //also move 'read' buffer if consumed
                if(obj->read->csmd == obj->read->filled){
                    STNBHttpServiceConnBuff* nxt2 = (obj->read == &obj->allocs.buff0 ? &obj->allocs.buff1 : &obj->allocs.buff0);
                    obj->read = nxt2; NBASSERT(obj->read->csmd == 0)
                }
                obj->fill = nxt; NBASSERT(obj->fill->csmd == 0 && obj->fill->filled == 0)
            }
        }
    }
}

void NBHttpServiceConnBuffs_readMoveCursor(STNBHttpServiceConnBuffs* obj, const SI32 moveSz){
    if(moveSz > 0){
        obj->read->csmd += moveSz;
        obj->totals.csmd += moveSz;
        //reset buffer
        NBASSERT(obj->read->csmd <= obj->read->filled)
        if(obj->read->csmd == obj->read->filled && obj->fill != obj->read){
            STNBHttpServiceConnBuff* nxt = (obj->read == &obj->allocs.buff0 ? &obj->allocs.buff1 : &obj->allocs.buff0);
            obj->read = nxt; NBASSERT(obj->read->csmd == 0)
        }
    }
}
