
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBHttpServiceResp.h"
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

#define NB_HTTP_SERVICE_RESP_DEFAULT_QUEUE_BUFF_INITIAL_SZ  0
#define NB_HTTP_SERVICE_RESP_DEFAULT_QUEUE_BUFF_GROWTH_SZ   (1024 * 128)

//STNBHttpServiceResp

typedef struct STNBHttpServiceRespOpq_ {
	STNBObject				prnt;
	BOOL					isLogicError;	    //logic error found, or explicitly-invalidated
    BOOL                    isUpgradedToRaw;    //upgraded from managed-response to raw-response
    BOOL                    isRawRead;          //upgrade includes reading incoming data
    BOOL                    isOrphan;           //release by callback
    STNBHttpConnCfg         cfg;
	//write
	struct {
		UI64				bytesQueued;
		UI64				bytesQueuedCsmd;
		//pend
		struct {
			SI32			csmd;
			STNBString		buff;
			STNBHttpBuilder bldr;
		} pend;
		//resp
		struct {
			//byHttp (when NBHttpServiceResp_setResponseCode() and/or NBHttpServiceResp_concatBody() is used)
			struct {
				UI64			bytesQueued;
				//hdr
				struct {
					UI64		bytesQueued;
					UI32		code;
					STNBString	reason;
					BOOL		isClosed;		//empty-line sent
				} hdr;
				//body
				struct {
					UI64		bytesQueued;	//body started
					UI64		bytesExp;		//size expected
					BOOL		bytesExpIsSet;	//size expected
					BOOL		cLenghtIsSent;	//content-length header
					STNBString	accum;
				} body;
			} byHttp;
			//byRaw
			struct {
				UI64			bytesQueued;			//'byRaw' already activated
			} byRaw;
		} resp;
	} write;
} STNBHttpServiceRespOpq;

NB_OBJREF_BODY(NBHttpServiceResp, STNBHttpServiceRespOpq, NBObject)

//resp private methods

void NBHttpServiceResp_respInvalidateOpq_(STNBHttpServiceRespOpq* opq);
void NBHttpServiceResp_respCloseOpq_(STNBHttpServiceRespOpq* opq);
BOOL NBHttpServiceResp_upgradeToRawOpq_(STNBHttpServiceRespOpq* opq, STNBHttpServiceRespRawLnk* dstLnk, const BOOL includeRawRead);
//response-header
BOOL NBHttpServiceResp_setResponseCodeOpq_(STNBHttpServiceRespOpq* opq, const UI32 code, const char* reason);
UI32 NBHttpServiceResp_getResponseCodeOpq_(STNBHttpServiceRespOpq* opq);
BOOL NBHttpServiceResp_addHeaderFieldOpq_(STNBHttpServiceRespOpq* opq, const char* name, const char* value);
BOOL NBHttpServiceResp_endHeaderOpq_(STNBHttpServiceRespOpq* opq);
//response-body
BOOL NBHttpServiceResp_setContentLengthOpq_(STNBHttpServiceRespOpq* opq, const UI64 contentLength);
BOOL NBHttpServiceResp_unsetContentLengthOpq_(STNBHttpServiceRespOpq* opq);
BOOL NBHttpServiceResp_concatBodyBytesOpq_(STNBHttpServiceRespOpq* opq, const void* data, const UI32 dataSz);
BOOL NBHttpServiceResp_concatBodyOpq_(STNBHttpServiceRespOpq* opq, const char* str);
BOOL NBHttpServiceResp_concatBodyStructAsJsonOpq_(STNBHttpServiceRespOpq* opq, const STNBStructMap* structMap, const void* src, const UI32 srcSz);
BOOL NBHttpServiceResp_sendBodyOpq_(STNBHttpServiceRespOpq* opq, const STNBHttpBody* body);
BOOL NBHttpServiceResp_flushBodyLockedOpq_(STNBHttpServiceRespOpq* opq);
BOOL NBHttpServiceResp_flushBodyOpq_(STNBHttpServiceRespOpq* opq);

//resp-raw private methods

BOOL NBHttpServiceResp_sendHeaderOpq_(STNBHttpServiceRespOpq* opq, const STNBHttpHeader* header, const BOOL doNotSendHeaderEnd);
BOOL NBHttpServiceResp_sendHeaderEndOpq_(STNBHttpServiceRespOpq* opq);
//response-raw
BOOL NBHttpServiceResp_sendBytesOpq_(STNBHttpServiceRespOpq* opq, const void* data, const UI32 dataSz);

//resp itf

void NBHttpServiceResp_httpReqRespInvalidate_(void* usrData); //to stop buffering
void NBHttpServiceResp_httpReqRespClose_(void* usrData); //required, to release ownership
BOOL NBHttpServiceResp_httpReqRespUpgradeToRaw_(STNBHttpServiceRespRawLnk* dstLnk, const BOOL includeRawRead, void* usrData);

//response-header
BOOL NBHttpServiceResp_httpReqRespSetResponseCode_(const UI32 code, const char* reason, void* usrData); //required at least once
UI32 NBHttpServiceResp_httpReqRespGetResponseCode_(void* usrData);
BOOL NBHttpServiceResp_httpReqRespAddHeaderField_(const char* name, const char* value, void* usrData); //optional
BOOL NBHttpServiceResp_httpReqRespEndHeader_(void* usrData); //optional, mostly used when empty-body; the header is auto-ended when the body is started.

//response-body
BOOL NBHttpServiceResp_httpReqRespSetContentLength_(const UI64 contentLength, void* usrData); //optimization, when size is known buffer is not used (data is send inmediatly)
BOOL NBHttpServiceResp_httpReqRespUnsetContentLength_(void* usrData); //reverse of 'setContentLength'
BOOL NBHttpServiceResp_httpReqRespConcatBody_(const char* str, void* usrData);
BOOL NBHttpServiceResp_httpReqRespConcatBodyBytes_(const void* data, const UI32 dataSz, void* usrData);
BOOL NBHttpServiceResp_httpReqRespConcatBodyStructAsJson_(const STNBStructMap* structMap, const void* src, const UI32 srcSz, void* usrData);
BOOL NBHttpServiceResp_httpReqRespSendBody_(const STNBHttpBody* body, void* usrData);

//resp-raw itf
BOOL NBHttpServiceResp_httpReqRespSendHeader_(const STNBHttpHeader* header, const BOOL doNotSendHeaderEnd, void* usrData);
BOOL NBHttpServiceResp_httpReqRespSendHeaderEnd_(void* usrData);

//response-raw
BOOL NBHttpServiceResp_httpReqRespSendBytes_(const void* data, const UI32 dataSz, void* usrData);

//

void NBHttpServiceResp_initZeroed(STNBObject* obj){
	STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)obj;
	//write
	{
		//pend
		NBString_initWithSz(&opq->write.pend.buff, NB_HTTP_SERVICE_RESP_DEFAULT_QUEUE_BUFF_INITIAL_SZ, NB_HTTP_SERVICE_RESP_DEFAULT_QUEUE_BUFF_GROWTH_SZ, 0.10f);
		NBHttpBuilder_init(&opq->write.pend.bldr);
		//resp
		{
			//byHttp (when NBHttpServiceResp_setResponseCode() and/or NBHttpServiceResp_concatBody() is used)
			{
				//hdr
				{
					NBString_initWithSz(&opq->write.resp.byHttp.hdr.reason, 0, 256, 0.1f);
				}
				//body
				{
					NBString_initWithSz(&opq->write.resp.byHttp.body.accum, 0, 4096, 0.1f);
				}
			}
			//byRaw
			{
				//
			}
		}
	}
}

void NBHttpServiceResp_uninitLocked(STNBObject* obj){
	STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)obj;
	//write
	{
		//pend
		NBString_release(&opq->write.pend.buff);
		NBHttpBuilder_release(&opq->write.pend.bldr);
		//resp
		{
			//byHttp (when NBHttpServiceResp_setResponseCode() and/or NBHttpServiceResp_concatBody() is used)
			{
				//hdr
				{
					NBString_release(&opq->write.resp.byHttp.hdr.reason);
				}
				//body
				{
					NBString_release(&opq->write.resp.byHttp.body.accum);
				}
			}
			//byRaw
			{
				//
			}
		}
	}
    //cfg
    {
        NBStruct_stRelease(NBHttpConnCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
    }
}

//

BOOL NBHttpServiceResp_prepare(STNBHttpServiceRespRef ref, const STNBHttpConnCfg* cfg){
    BOOL r = FALSE;
    STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
    if(!opq->isLogicError && opq->write.pend.buff.length == 0){
        r = TRUE;
        //resize queue
        {
            const UI32 growthSz = (cfg->sendQueue.growthSz > 0 ? cfg->sendQueue.growthSz : NB_HTTP_SERVICE_RESP_DEFAULT_QUEUE_BUFF_GROWTH_SZ);
            NBString_release(&opq->write.pend.buff);
            NBString_initWithSz(&opq->write.pend.buff, cfg->sendQueue.initialSz, growthSz, 0.10f);
        }
        //cfg
        if(r){
            NBStruct_stRelease(NBHttpConnCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
            if(cfg != NULL){
                NBStruct_stClone(NBHttpConnCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
            }
        }
    }
    return r;
}

//

void NBHttpServiceResp_invalidate(STNBHttpServiceRespRef ref){        //stop buffering
    STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
    opq->isLogicError = TRUE;
}

BOOL NBHttpServiceResp_isLogicError(STNBHttpServiceRespRef ref){ //internal logic error or expliclty-invalidated
	STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
	return opq->isLogicError;
}

BOOL NBHttpServiceResp_isUpgradedRaw(STNBHttpServiceRespRef ref){ //send raw-response
    STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
    return opq->isUpgradedToRaw;
}

BOOL NBHttpServiceResp_isUpgradedRawRead(STNBHttpServiceRespRef ref){ //send raw-response and read socket data
    STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
    return (opq->isUpgradedToRaw && opq->isRawRead);
}

BOOL NBHttpServiceResp_isOrphan(STNBHttpServiceRespRef ref){
    STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
    return opq->isOrphan;
}

//resp

void NBHttpServiceResp_getRespItf(STNBHttpServiceRespRef ref, STNBHttpServiceRespLnkItf* dstItf, void** dstUsrParam){
    STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
    if(dstItf != NULL){
        NBMemory_setZeroSt(*dstItf, STNBHttpServiceRespLnkItf);
        //
        dstItf->httpReqRespInvalidate       = NBHttpServiceResp_httpReqRespInvalidate_;
        dstItf->httpReqRespClose            = NBHttpServiceResp_httpReqRespClose_;
        dstItf->httpReqRespUpgradeToRaw     = NBHttpServiceResp_httpReqRespUpgradeToRaw_;
        //response-header
        dstItf->httpReqRespSetResponseCode  = NBHttpServiceResp_httpReqRespSetResponseCode_;
        dstItf->httpReqRespGetResponseCode  = NBHttpServiceResp_httpReqRespGetResponseCode_;
        dstItf->httpReqRespAddHeaderField   = NBHttpServiceResp_httpReqRespAddHeaderField_;
        dstItf->httpReqRespEndHeader        = NBHttpServiceResp_httpReqRespEndHeader_;
        //response-body
        dstItf->httpReqRespSetContentLength = NBHttpServiceResp_httpReqRespSetContentLength_;
        dstItf->httpReqRespUnsetContentLength = NBHttpServiceResp_httpReqRespUnsetContentLength_;
        dstItf->httpReqRespConcatBody       = NBHttpServiceResp_httpReqRespConcatBody_;
        dstItf->httpReqRespConcatBodyBytes  = NBHttpServiceResp_httpReqRespConcatBodyBytes_;
        dstItf->httpReqRespConcatBodyStructAsJson = NBHttpServiceResp_httpReqRespConcatBodyStructAsJson_;
        dstItf->httpReqRespSendBody         = NBHttpServiceResp_httpReqRespSendBody_;
    }
    if(dstUsrParam != NULL){
        *dstUsrParam = opq;
    }
}

//io-write

UI64 NBHttpServiceResp_writeQueuedTotal(STNBHttpServiceRespRef ref){   //ammount of bytes queued
    STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
    return opq->write.bytesQueued;
}

UI64 NBHttpServiceResp_writeQueuedConsumed(STNBHttpServiceRespRef ref){     //ammount of bytes queued sent
    STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
    return opq->write.bytesQueuedCsmd;
}

BOOL NBHttpServiceResp_isWritePend(STNBHttpServiceRespRef ref){
	STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
    NBASSERT(opq->write.bytesQueuedCsmd <= opq->write.bytesQueued)
    NBASSERT(opq->write.pend.csmd <= opq->write.pend.buff.length)
    NBASSERT((opq->write.bytesQueued - opq->write.bytesQueuedCsmd) == (opq->write.pend.buff.length - opq->write.pend.csmd))
	return (opq->write.pend.csmd < opq->write.pend.buff.length);
}

SI32 NBHttpServiceResp_write(STNBHttpServiceRespRef ref, STNBHttpServiceRespSenderItf* sndrItf, void* sndrUsrData){
	SI32 r = 0;
	STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
	if(sndrItf != NULL && sndrItf->httpReqSend != NULL && sndrUsrData != NULL){
		NBObject_lock(opq);
		{
            SI32 snt = 0, avail = 0;
			while(opq->write.pend.csmd < opq->write.pend.buff.length){
				//PRINTF_INFO("NBHttpServiceResp, sending response (%d bytes):\n-->%s<--\n", (opq->write.pend.buff.length - opq->write.pend.csmd), &opq->write.pend.buff.str[opq->write.pend.csmd]);
				avail	= opq->write.pend.buff.length - opq->write.pend.csmd;
				snt		= (*sndrItf->httpReqSend)(ref, &opq->write.pend.buff.str[opq->write.pend.csmd], avail, sndrUsrData);
				if(snt < 0){
                    r = -1;
					opq->isLogicError = TRUE;
					break;
				} else {
					r += snt;
					opq->write.bytesQueuedCsmd += snt; NBASSERT(opq->write.bytesQueuedCsmd <= opq->write.bytesQueued)
					opq->write.pend.csmd += snt;
					if(opq->write.pend.csmd == opq->write.pend.buff.length){
						NBString_empty(&opq->write.pend.buff);
						opq->write.pend.csmd = 0;
                    } else if(opq->write.pend.buff.length > NBString_getBufferGrowthMin(&opq->write.pend.buff) && opq->write.pend.csmd > (opq->write.pend.buff.length / 2)){ // '>' instead of '>=' because we want to be sure at least half is empty
                        //at least half the buffer is consumed (memcpy wont overlap)
                        NBMemory_move(opq->write.pend.buff.str, &opq->write.pend.buff.str[opq->write.pend.csmd], (opq->write.pend.buff.length - opq->write.pend.csmd));
                        NBString_truncate(&opq->write.pend.buff, (opq->write.pend.buff.length - opq->write.pend.csmd));
                        opq->write.pend.csmd = 0;
                    }
					if(snt == 0){
						break;
					}
				}
			} 
		}
		NBObject_unlock(opq);
	}
	return r;
}

//header

BOOL NBHttpServiceResp_sendHdrFirstLineLockedOpq_(STNBHttpServiceRespOpq* opq);
BOOL NBHttpServiceResp_sendHdrContentLenghtLockedOpq_(STNBHttpServiceRespOpq* opq, const UI64 contentLenght);
BOOL NBHttpServiceResp_sendHdrEndLockedOpq_(STNBHttpServiceRespOpq* opq);

//

void NBHttpServiceResp_respInvalidateOpq_(STNBHttpServiceRespOpq* opq){
    opq->isLogicError = TRUE;
}

void NBHttpServiceResp_respCloseOpq_(STNBHttpServiceRespOpq* opq){
    NBObject_lock(opq);
    if(!opq->isOrphan){
        if(!opq->isUpgradedToRaw){
            NBHttpServiceResp_flushBodyLockedOpq_(opq);
        }
        opq->isOrphan = TRUE;
    }
    NBObject_unlock(opq);
}

BOOL NBHttpServiceResp_upgradeToRawOpq_(STNBHttpServiceRespOpq* opq, STNBHttpServiceRespRawLnk* dstLnk, const BOOL includeRawRead){
    BOOL r = FALSE;
    NBObject_lock(opq);
    if(dstLnk != NULL && !opq->isLogicError && !opq->isUpgradedToRaw && !opq->isOrphan){
        NBMemory_setZeroSt(*dstLnk, STNBHttpServiceRespRawLnk);
        {
            dstLnk->itf.httpReqRespInvalidate       = NBHttpServiceResp_httpReqRespInvalidate_;
            dstLnk->itf.httpReqRespClose            = NBHttpServiceResp_httpReqRespClose_;
            //response-header
            dstLnk->itf.httpReqRespSendHeader       = NBHttpServiceResp_httpReqRespSendHeader_;
            dstLnk->itf.httpReqRespSendHeaderEnd    = NBHttpServiceResp_httpReqRespSendHeaderEnd_;
            //response-raw
            dstLnk->itf.httpReqRespSendBytes        = NBHttpServiceResp_httpReqRespSendBytes_;
        }
        {
            dstLnk->itfParam    = opq;
        }
        opq->isUpgradedToRaw    = TRUE; //raw-write enabled
        opq->isRawRead          = includeRawRead; //raw-read enabled
        r = TRUE;
    }
    NBObject_unlock(opq);
    return r;
}

BOOL NBHttpServiceResp_setResponseCodeOpq_(STNBHttpServiceRespOpq* opq, const UI32 code, const char* reason){
    BOOL r = FALSE;
    NBObject_lock(opq);
    if(!opq->isLogicError && !opq->isUpgradedToRaw && !opq->isOrphan){
        if(opq->write.bytesQueued != 0){
            PRINTF_ERROR("NBHttpServiceResp, cannot apply response code after data sent.\n");
            opq->isLogicError = TRUE;
        } else {
            opq->write.resp.byHttp.hdr.code = code;
            NBString_set(&opq->write.resp.byHttp.hdr.reason, reason);
            r = TRUE;
        }
    }
    NBObject_unlock(opq);
    return r;
}

UI32 NBHttpServiceResp_getResponseCodeOpq_(STNBHttpServiceRespOpq* opq){
    return opq->write.resp.byHttp.hdr.code;
}
    
BOOL NBHttpServiceResp_addHeaderFieldOpq_(STNBHttpServiceRespOpq* opq, const char* name, const char* value){
    BOOL r = FALSE;
    if(!NBString_strIsEmpty(name)){
        NBObject_lock(opq);
        if(!opq->isLogicError && !opq->isUpgradedToRaw && !opq->isOrphan){
            //send response-code (if necesary)
            if(opq->write.bytesQueued == 0 && !NBHttpServiceResp_sendHdrFirstLineLockedOpq_(opq)){
                PRINTF_ERROR("NBHttpServiceResp, NBHttpServiceResp_sendHdrFirstLineLockedOpq_ failed.\n");
                opq->isLogicError = TRUE;
            }
            //send header field
            if(!opq->isLogicError && opq->write.bytesQueued != 0){
                const SI32 lenBefore = opq->write.pend.buff.length;
                if(opq->write.resp.byHttp.hdr.isClosed){
                    PRINTF_ERROR("NBHttpServiceResp, cannot add-header-field after header-end.\n");
                    opq->isLogicError = TRUE;
                } else if(!NBHttpBuilder_addHeaderField(&opq->write.pend.bldr, &opq->write.pend.buff, name, value)){
                    PRINTF_ERROR("NBHttpServiceResp, NBHttpBuilder_addHeaderField failed.\n");
                    opq->isLogicError = TRUE;
                } else {
                    const SI32 added = opq->write.pend.buff.length - lenBefore;
                    //PRINTF_INFO("NBHttpServiceResp, hdr-field queued:--->\n%s<---.\n", &opq->write.pend.buff.str[lenBefore]);
                    opq->write.resp.byHttp.hdr.bytesQueued += added;
                    opq->write.resp.byHttp.bytesQueued += added;
                    opq->write.bytesQueued += added;
                    //result
                    r = TRUE;
                }
            }
        }
        NBObject_unlock(opq);
    }
    return r;
}

BOOL NBHttpServiceResp_endHeaderOpq_(STNBHttpServiceRespOpq* opq){
    BOOL r = FALSE;
    NBObject_lock(opq);
    if(!opq->isLogicError && !opq->isUpgradedToRaw && !opq->isOrphan){
        //send response-code (if necesary)
        if(opq->write.bytesQueued == 0 && !NBHttpServiceResp_sendHdrFirstLineLockedOpq_(opq)){
            PRINTF_ERROR("NBHttpServiceResp, NBHttpServiceResp_sendHdrFirstLineLockedOpq_ failed.\n");
            opq->isLogicError = TRUE;
        }
        //send content-length (if necesary)
        if(opq->write.bytesQueued != 0 && !opq->write.resp.byHttp.body.cLenghtIsSent && opq->write.resp.byHttp.body.bytesExpIsSet){
            NBHttpServiceResp_sendHdrContentLenghtLockedOpq_(opq, opq->write.resp.byHttp.body.bytesExp);
        }
        //send header-end
        if(!opq->isLogicError && opq->write.bytesQueued != 0 && !opq->write.resp.byHttp.hdr.isClosed && !NBHttpServiceResp_sendHdrEndLockedOpq_(opq)){
            PRINTF_ERROR("NBHttpServiceResp, NBHttpServiceResp_sendHdrEndLockedOpq_ failed.\n");
            opq->isLogicError = TRUE;
        } else {
            r = TRUE;
        }
    }
    NBObject_unlock(opq);
    return r;
}

//body

BOOL NBHttpServiceResp_setContentLengthOpq_(STNBHttpServiceRespOpq* opq, const UI64 contentLength){
    BOOL r = FALSE;
    NBObject_lock(opq);
    {
        if(opq->write.resp.byHttp.body.bytesQueued != 0){
            PRINTF_ERROR("NBHttpServiceResp, cannot set content-length after body was sent.\n");
            opq->isLogicError = TRUE;
        } else {
            opq->write.resp.byHttp.body.bytesExp = contentLength;
            opq->write.resp.byHttp.body.bytesExpIsSet = TRUE;
            r = TRUE;
        }
    }
    NBObject_unlock(opq);
    return r;
}

BOOL NBHttpServiceResp_unsetContentLengthOpq_(STNBHttpServiceRespOpq* opq){
    BOOL r = FALSE;
    NBObject_lock(opq);
    {
        if(opq->write.resp.byHttp.body.bytesQueued != 0){
            PRINTF_ERROR("NBHttpServiceResp, cannot unset content-length after body was sent.\n");
            opq->isLogicError = TRUE;
        } else {
            opq->write.resp.byHttp.body.bytesExp = 0;
            opq->write.resp.byHttp.body.bytesExpIsSet = FALSE;
            r = TRUE;
        }
    }
    NBObject_unlock(opq);
    return r;
}

BOOL NBHttpServiceResp_concatBodyBytesOpq_(STNBHttpServiceRespOpq* opq, const void* data, const UI32 dataSz){
    BOOL r = FALSE;
    NBObject_lock(opq);
    if(!opq->isLogicError && !opq->isUpgradedToRaw && !opq->isOrphan){
        if(!opq->write.resp.byHttp.body.bytesExpIsSet){
            //just queue bytes in body
            NBString_concatBytes(&opq->write.resp.byHttp.body.accum, data, dataSz);
            opq->write.resp.byHttp.body.bytesQueued += dataSz;
            r = TRUE;
        } else {
            //send without queue
            //send response-code (if necesary)
            if(opq->write.bytesQueued == 0 && !NBHttpServiceResp_sendHdrFirstLineLockedOpq_(opq)){
                PRINTF_ERROR("NBHttpServiceResp, NBHttpServiceResp_sendHdrFirstLineLockedOpq_ failed.\n");
                opq->isLogicError = TRUE;
            }
            //send content-length (if necesary)
            if(opq->write.bytesQueued != 0 && !opq->write.resp.byHttp.body.cLenghtIsSent){
                NBHttpServiceResp_sendHdrContentLenghtLockedOpq_(opq, opq->write.resp.byHttp.body.bytesExp);
            }
            //send hdr-end (if necesary)
            if(opq->write.bytesQueued != 0 && !opq->write.resp.byHttp.hdr.isClosed){
                NBHttpServiceResp_sendHdrEndLockedOpq_(opq);
            }
            //queue bytes in write buffer (no body buffer necesary)
            {
                NBString_concatBytes(&opq->write.pend.buff, data, dataSz);
                //PRINTF_INFO("NBHttpServiceResp, body-bytes queued:--->\n%s<---.\n", &opq->write.pend.buff.str[opq->write.pend.buff.length - dataSz]);
                opq->write.resp.byHttp.body.bytesQueued += dataSz;
                opq->write.resp.byHttp.bytesQueued += dataSz;
                opq->write.bytesQueued += dataSz;
                //if(opq->write.resp.byHttp.body.bytesQueued == opq->write.resp.byHttp.body.bytesExp){
                //    PRINTF_INFO("NBHttpServiceResp, queued-body is completed (%d of %d bytes queued).\n", opq->write.resp.byHttp.body.bytesQueued, opq->write.resp.byHttp.body.bytesExp);
                //}
                //validate content length
                if(opq->write.resp.byHttp.body.bytesQueued > opq->write.resp.byHttp.body.bytesExp){
                    PRINTF_ERROR("NBHttpServiceResp, queued-body is larger than content-length.\n");
                    opq->isLogicError = TRUE;
                } else {
                    r = TRUE;
                }
            }
        }
    }
    NBObject_unlock(opq);
    return r;
}

BOOL NBHttpServiceResp_concatBodyOpq_(STNBHttpServiceRespOpq* opq, const char* str){
    return NBHttpServiceResp_concatBodyBytesOpq_(opq, str, NBString_strLenBytes(str));
}

BOOL NBHttpServiceResp_concatBodyStructAsJsonOpq_(STNBHttpServiceRespOpq* opq, const STNBStructMap* structMap, const void* src, const UI32 srcSz){
    BOOL r = FALSE;
    if(structMap != NULL && src != NULL && srcSz != 0){
        NBObject_lock(opq);
        if(!opq->isLogicError && !opq->isUpgradedToRaw && !opq->isOrphan){
            if(!opq->write.resp.byHttp.body.bytesExpIsSet){
                //just queue bytes in body
                const SI32 lenBefore = opq->write.resp.byHttp.body.accum.length;
                NBStruct_stConcatAsJson(&opq->write.resp.byHttp.body.accum, structMap, src, srcSz);
                {
                    const SI32 added = opq->write.resp.byHttp.body.accum.length - lenBefore;
                    opq->write.resp.byHttp.body.bytesQueued += added;
                }
                r = TRUE;
            } else {
                //send without queue
                //send response-code (if necesary)
                if(opq->write.bytesQueued == 0 && !NBHttpServiceResp_sendHdrFirstLineLockedOpq_(opq)){
                    PRINTF_ERROR("NBHttpServiceResp, NBHttpServiceResp_sendHdrFirstLineLockedOpq_ failed.\n");
                    opq->isLogicError = TRUE;
                }
                //send content-length (if necesary)
                if(opq->write.bytesQueued != 0 && !opq->write.resp.byHttp.body.cLenghtIsSent){
                    NBHttpServiceResp_sendHdrContentLenghtLockedOpq_(opq, opq->write.resp.byHttp.body.bytesExp);
                }
                //send hdr-end (if necesary)
                if(opq->write.bytesQueued != 0 && !opq->write.resp.byHttp.hdr.isClosed){
                    NBHttpServiceResp_sendHdrEndLockedOpq_(opq);
                }
                //queue bytes in write buffer (no body buffer necesary)
                {
                    const SI32 lenBefore = opq->write.pend.buff.length;
                    NBStruct_stConcatAsJson(&opq->write.pend.buff, structMap, src, srcSz);
                    {
                        const SI32 added = opq->write.pend.buff.length - lenBefore;
                        //PRINTF_INFO("NBHttpServiceResp, body-json-as-str queued:--->\n%s<---.\n", &opq->write.pend.buff.str[lenBefore]);
                        opq->write.resp.byHttp.body.bytesQueued += added;
                        opq->write.resp.byHttp.bytesQueued += added;
                        opq->write.bytesQueued += added;
                    }
                    //validate content length
                    if(opq->write.resp.byHttp.body.bytesQueued > opq->write.resp.byHttp.body.bytesExp){
                        PRINTF_ERROR("NBHttpServiceResp, queued-body is larger than content-length.\n");
                        opq->isLogicError = TRUE;
                    } else {
                        r = TRUE;
                    }
                }
            }
        }
        NBObject_unlock(opq);
    }
    return r;
}

BOOL NBHttpServiceResp_sendBodyOpq_(STNBHttpServiceRespOpq* opq, const STNBHttpBody* body){
    BOOL r = FALSE;
    if(body != NULL){
        NBObject_lock(opq);
        if(!opq->isLogicError && !opq->isUpgradedToRaw && !opq->isOrphan){
            if(!opq->write.resp.byHttp.body.bytesExpIsSet){
                //just queue bytes in body
                const SI32 lenBefore = opq->write.resp.byHttp.body.accum.length;
                NBHttpBody_chunksConcatAll(body, &opq->write.resp.byHttp.body.accum);
                {
                    const SI32 added = opq->write.resp.byHttp.body.accum.length - lenBefore;
                    opq->write.resp.byHttp.body.bytesQueued += added;
                }
                r = TRUE;
            } else {
                //send without queue
                //send response-code (if necesary)
                if(opq->write.bytesQueued == 0 && !NBHttpServiceResp_sendHdrFirstLineLockedOpq_(opq)){
                    PRINTF_ERROR("NBHttpServiceResp, NBHttpServiceResp_sendHdrFirstLineLockedOpq_ failed.\n");
                    opq->isLogicError = TRUE;
                }
                //send content-length (if necesary)
                if(opq->write.bytesQueued != 0 && !opq->write.resp.byHttp.body.cLenghtIsSent){
                    NBHttpServiceResp_sendHdrContentLenghtLockedOpq_(opq, opq->write.resp.byHttp.body.bytesExp);
                }
                //send hdr-end (if necesary)
                if(opq->write.bytesQueued != 0 && !opq->write.resp.byHttp.hdr.isClosed){
                    NBHttpServiceResp_sendHdrEndLockedOpq_(opq);
                }
                //queue bytes in write buffer (no body buffer necesary)
                {
                    const SI32 lenBefore = opq->write.pend.buff.length;
                    NBHttpBody_chunksConcatAll(body, &opq->write.pend.buff);
                    {
                        const SI32 added = opq->write.pend.buff.length - lenBefore;
                        //PRINTF_INFO("NBHttpServiceResp, body queued:--->\n%s<---.\n", &opq->write.pend.buff.str[lenBefore]);
                        opq->write.resp.byHttp.body.bytesQueued += added;
                        opq->write.resp.byHttp.bytesQueued += added;
                        opq->write.bytesQueued += added;
                    }
                    //validate content length
                    if(opq->write.resp.byHttp.body.bytesQueued > opq->write.resp.byHttp.body.bytesExp){
                        PRINTF_ERROR("NBHttpServiceResp, queued-body is larger than content-length.\n");
                        opq->isLogicError = TRUE;
                    } else {
                        r = TRUE;
                    }
                }
            }
        }
        NBObject_unlock(opq);
    }
    return r;
}

BOOL NBHttpServiceResp_flushBodyLockedOpq_(STNBHttpServiceRespOpq* opq){
    BOOL r = FALSE;
    if(!opq->isLogicError && !opq->isUpgradedToRaw && !opq->isOrphan){
        //send response-code (if necesary)
        if(opq->write.bytesQueued == 0 && !NBHttpServiceResp_sendHdrFirstLineLockedOpq_(opq)){
            PRINTF_ERROR("NBHttpServiceResp, NBHttpServiceResp_sendHdrFirstLineLockedOpq_ failed.\n");
            opq->isLogicError = TRUE;
        }
        //send content-length (if necesary)
        if(opq->write.bytesQueued != 0 && !opq->write.resp.byHttp.body.cLenghtIsSent){
            NBHttpServiceResp_sendHdrContentLenghtLockedOpq_(opq, opq->write.resp.byHttp.body.accum.length);
        }
        //send hdr-end (if necesary)
        if(opq->write.bytesQueued != 0 && !opq->write.resp.byHttp.hdr.isClosed){
            NBHttpServiceResp_sendHdrEndLockedOpq_(opq);
        }
        //queue bytes in write buffer (no body buffer necesary)
        {
            NBString_concatBytes(&opq->write.pend.buff, opq->write.resp.byHttp.body.accum.str, opq->write.resp.byHttp.body.accum.length);
            //PRINTF_INFO("NBHttpServiceResp, body-flush queued:--->\n%s<---.\n", &opq->write.pend.buff.str[opq->write.pend.buff.length - opq->write.resp.byHttp.body.accum.length]);
            opq->write.resp.byHttp.bytesQueued += opq->write.resp.byHttp.body.accum.length;
            opq->write.bytesQueued += opq->write.resp.byHttp.body.accum.length;
            //
            NBString_empty(&opq->write.resp.byHttp.body.accum);
            //validate content length
            if(opq->write.resp.byHttp.body.bytesExpIsSet && opq->write.resp.byHttp.body.bytesQueued > opq->write.resp.byHttp.body.bytesExp){
                PRINTF_ERROR("NBHttpServiceResp, queued-body is larger than content-length.\n");
                opq->isLogicError = TRUE;
            } else {
                r = TRUE;
            }
        }
    }
    return r;
}

BOOL NBHttpServiceResp_flushBodyOpq_(STNBHttpServiceRespOpq* opq){
    BOOL r = FALSE;
    NBObject_lock(opq);
    {
        NBHttpServiceResp_flushBodyLockedOpq_(opq);
    }
    NBObject_unlock(opq);
    return r;
}

//raw

BOOL NBHttpServiceResp_sendHeaderOpq_(STNBHttpServiceRespOpq* opq, const STNBHttpHeader* header, const BOOL doNotSendHeaderEnd){
    BOOL r = FALSE;
    if(header != NULL){
        NBObject_lock(opq);
        if(!opq->isLogicError && opq->isUpgradedToRaw && !opq->isOrphan){
            const SI32 lenBefore = opq->write.pend.buff.length;
            if(opq->write.resp.byHttp.bytesQueued != 0){
                PRINTF_ERROR("NBHttpServiceResp, cannot raw-send after http sent.\n");
                opq->isLogicError = TRUE;
            } else if(!NBHttpBuilder_addHeader(&opq->write.pend.bldr, &opq->write.pend.buff, header)){
                PRINTF_ERROR("NBHttpServiceResp, NBHttpBuilder_addHeader failed.\n");
                opq->isLogicError = TRUE;
            } else if(!doNotSendHeaderEnd && !NBHttpBuilder_addHeaderEnd(&opq->write.pend.bldr, &opq->write.pend.buff)){
                PRINTF_ERROR("NBHttpServiceResp, NBHttpBuilder_addHeader failed.\n");
                opq->isLogicError = TRUE;
            } else {
                const SI32 added = opq->write.pend.buff.length - lenBefore;
                //PRINTF_INFO("NBHttpServiceResp, header queued:--->\n%s<---.\n", &opq->write.pend.buff.str[lenBefore]);
                opq->write.resp.byRaw.bytesQueued += added;
                opq->write.bytesQueued += added;
                r = TRUE;
            }
        }
        NBObject_unlock(opq);
    }
    return r;
}


BOOL NBHttpServiceResp_sendRawHeader(STNBHttpServiceRespRef ref, const STNBHttpHeader* header){
	BOOL r = FALSE;
	STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
    {
        r = NBHttpServiceResp_sendHeaderOpq_(opq, header, TRUE); //keep open
    }
	return r;
}

BOOL NBHttpServiceResp_sendRawHeaderAndEnd(STNBHttpServiceRespRef ref, const STNBHttpHeader* header){
	BOOL r = FALSE;
	STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
    {
        r = NBHttpServiceResp_sendHeaderOpq_(opq, header, FALSE); //close
    }
	return r;
}

BOOL NBHttpServiceResp_sendHeaderEndOpq_(STNBHttpServiceRespOpq* opq){
    BOOL r = FALSE;
    NBObject_lock(opq);
    if(!opq->isLogicError && opq->isUpgradedToRaw && !opq->isOrphan){
        const SI32 lenBefore = opq->write.pend.buff.length;
        if(opq->write.resp.byHttp.bytesQueued != 0){
            PRINTF_ERROR("NBHttpServiceResp, cannot raw-send after http sent.\n");
            opq->isLogicError = TRUE;
        } else if(!NBHttpBuilder_addHeaderEnd(&opq->write.pend.bldr, &opq->write.pend.buff)){
            PRINTF_ERROR("NBHttpServiceResp, NBHttpBuilder_addHeaderEnd failed.\n");
            opq->isLogicError = TRUE;
        } else {
            const SI32 added = opq->write.pend.buff.length - lenBefore;
            //PRINTF_INFO("NBHttpServiceResp, header queued:--->\n%s<---.\n", &opq->write.pend.buff.str[lenBefore]);
            opq->write.resp.byRaw.bytesQueued += added;
            opq->write.bytesQueued += added;
            r = TRUE;
        }
    }
    NBObject_unlock(opq);
    return r;
}

BOOL NBHttpServiceResp_sendRawHeaderEnd(STNBHttpServiceRespRef ref){
	BOOL r = FALSE;
	STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
    {
        r = NBHttpServiceResp_sendHeaderEndOpq_(opq);
    }
	return r;
}

//response-raw

BOOL NBHttpServiceResp_sendBytesOpq_(STNBHttpServiceRespOpq* opq, const void* data, const UI32 dataSz){
    BOOL r = FALSE;
    NBObject_lock(opq);
    if(!opq->isLogicError && opq->isUpgradedToRaw && !opq->isOrphan){
        if(opq->write.resp.byHttp.bytesQueued != 0){
            PRINTF_ERROR("NBHttpServiceResp, cannot raw-send after http sent.\n");
            opq->isLogicError = TRUE;
        } else {
            NBString_concatBytes(&opq->write.pend.buff, data, dataSz);
            //PRINTF_INFO("NBHttpServiceResp, raw-bytes queued:--->\n%s<---.\n", &opq->write.pend.buff.str[opq->write.pend.buff.length - dataSz]);
            opq->write.resp.byRaw.bytesQueued += dataSz;
            opq->write.bytesQueued += dataSz;
            //PRINTF_INFO("NBHttpServiceResp, queued %d bytes (%d in queue, %d sent).\n", dataSz, opq->write.pend.buff.length, opq->write.bytesQueuedCsmd);
            r = TRUE;
        }
    }
    NBObject_unlock(opq);
    return r;
}

BOOL NBHttpServiceResp_sendBytes(STNBHttpServiceRespRef ref, const void* data, const UI32 dataSz){
	BOOL r = FALSE;
	STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
	{
        r = NBHttpServiceResp_sendBytesOpq_(opq, data, dataSz);
    }
	return r;
}

//

BOOL NBHttpServiceResp_sendHdrFirstLineLockedOpq_(STNBHttpServiceRespOpq* opq){
	BOOL r = FALSE;
	NBASSERT(opq->write.bytesQueued == 0)
	//send first line
	const SI32 lenBefore = opq->write.pend.buff.length;
	if(opq->write.resp.byRaw.bytesQueued != 0){
		PRINTF_ERROR("NBHttpServiceResp, cannot http-send after raw-send.\n");
		opq->isLogicError = TRUE;
	} else if(opq->write.resp.byHttp.hdr.code == 0){
		PRINTF_ERROR("NBHttpServiceResp, cannot send-status-line before response-code.\n");
		opq->isLogicError = TRUE;
	} else if(!NBHttpBuilder_addStatusLine(&opq->write.pend.bldr, &opq->write.pend.buff, 1, 1, opq->write.resp.byHttp.hdr.code, opq->write.resp.byHttp.hdr.reason.str)){
		PRINTF_ERROR("NBHttpServiceResp, NBHttpBuilder_addStatusLine failed.\n");
		opq->isLogicError = TRUE;
	} else {
		const SI32 added = opq->write.pend.buff.length - lenBefore;
        //PRINTF_INFO("NBHttpServiceResp, auto-hdr-first-line queued:--->\n%s<---.\n", &opq->write.pend.buff.str[lenBefore]);
		opq->write.resp.byHttp.hdr.bytesQueued += added;
		opq->write.resp.byHttp.bytesQueued += added;
		opq->write.bytesQueued += added;
		r = TRUE;
	}
	return r;
}

BOOL NBHttpServiceResp_sendHdrContentLenghtLockedOpq_(STNBHttpServiceRespOpq* opq, const UI64 contentLenght){
	BOOL r = FALSE;
	NBASSERT(opq->write.bytesQueued != 0)
	NBASSERT(!opq->write.resp.byHttp.body.cLenghtIsSent)
	if(opq->write.bytesQueued != 0 && !opq->write.resp.byHttp.body.cLenghtIsSent){
		const SI32 lenBefore = opq->write.pend.buff.length;
		if(opq->write.resp.byHttp.hdr.isClosed){
			PRINTF_ERROR("NBHttpServiceResp, cannot add-header-field after header-end.\n");
			opq->isLogicError = TRUE;
		} else if(!NBHttpBuilder_addContentLength(&opq->write.pend.bldr, &opq->write.pend.buff, contentLenght)){
			PRINTF_ERROR("NBHttpServiceResp, NBHttpBuilder_addHeaderField failed.\n");
			opq->isLogicError = TRUE;
		} else {
			const SI32 added = opq->write.pend.buff.length - lenBefore;
            //PRINTF_INFO("NBHttpServiceResp, auto-hdr-content-length queued:--->\n%s<---.\n", &opq->write.pend.buff.str[lenBefore]);
			opq->write.resp.byHttp.hdr.bytesQueued += added;
			opq->write.resp.byHttp.bytesQueued += added;
			opq->write.bytesQueued += added;
			//
			opq->write.resp.byHttp.body.bytesExpIsSet = TRUE;
			opq->write.resp.byHttp.body.bytesExp = contentLenght;
			//result
			opq->write.resp.byHttp.body.cLenghtIsSent = TRUE;
			r = TRUE;
		}
	}
	return r;
}

BOOL NBHttpServiceResp_sendHdrEndLockedOpq_(STNBHttpServiceRespOpq* opq){
	BOOL r = FALSE;
	NBASSERT(opq->write.bytesQueued != 0)
	NBASSERT(!opq->write.resp.byHttp.hdr.isClosed)
	if(opq->write.bytesQueued != 0 && !opq->write.resp.byHttp.hdr.isClosed){
		const SI32 lenBefore = opq->write.pend.buff.length;
		if(!NBHttpBuilder_addHeaderEnd(&opq->write.pend.bldr, &opq->write.pend.buff)){
			PRINTF_ERROR("NBHttpServiceResp, NBHttpBuilder_addHeaderEnd failed.\n");
			opq->isLogicError = TRUE;
		} else {
			const SI32 added = opq->write.pend.buff.length - lenBefore;
            //PRINTF_INFO("NBHttpServiceResp, auto-hdr-end queued:--->\n%s<---.\n", &opq->write.pend.buff.str[lenBefore]);
			opq->write.resp.byHttp.hdr.bytesQueued += added;
			opq->write.resp.byHttp.bytesQueued += added;
			opq->write.bytesQueued += added;
			//
			opq->write.resp.byHttp.hdr.isClosed = TRUE;
			//result
			r = TRUE;
		}
	}
	return r;
}

//response-header

BOOL NBHttpServiceResp_setResponseCode(STNBHttpServiceRespRef ref, const UI32 code, const char* reason){
    STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
    return NBHttpServiceResp_setResponseCodeOpq_(opq, code, reason);
}

UI32 NBHttpServiceResp_getResponseCode(STNBHttpServiceRespRef ref){
    STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
    return NBHttpServiceResp_getResponseCodeOpq_(opq);
}

BOOL NBHttpServiceResp_addHeaderField(STNBHttpServiceRespRef ref, const char* name, const char* value){
    STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
    return NBHttpServiceResp_addHeaderFieldOpq_(opq, name, value);
}

BOOL NBHttpServiceResp_endHeader(STNBHttpServiceRespRef ref){
    STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
    return NBHttpServiceResp_endHeaderOpq_(opq);
}

//response-body

BOOL NBHttpServiceResp_setContentLength(STNBHttpServiceRespRef ref, const UI64 contentLength){
    STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
    return NBHttpServiceResp_setContentLengthOpq_(opq, contentLength);
}

BOOL NBHttpServiceResp_unsetContentLength(STNBHttpServiceRespRef ref){
    STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
    return NBHttpServiceResp_unsetContentLengthOpq_(opq);
}

BOOL NBHttpServiceResp_concatBodyBytes(STNBHttpServiceRespRef ref, const void* data, const UI32 dataSz){
    STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
    return NBHttpServiceResp_concatBodyBytesOpq_(opq, data, dataSz);
}

BOOL NBHttpServiceResp_concatBody(STNBHttpServiceRespRef ref, const char* str){
    STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
    return NBHttpServiceResp_concatBodyOpq_(opq, str);
}

BOOL NBHttpServiceResp_concatBodyStructAsJson(STNBHttpServiceRespRef ref, const STNBStructMap* structMap, const void* src, const UI32 srcSz){
    STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
    return NBHttpServiceResp_concatBodyStructAsJsonOpq_(opq, structMap, src, srcSz);
}

BOOL NBHttpServiceResp_sendBody(STNBHttpServiceRespRef ref, const STNBHttpBody* body){
    STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
    return NBHttpServiceResp_sendBodyOpq_(opq, body);
}

BOOL NBHttpServiceResp_flushBody(STNBHttpServiceRespRef ref){
    STNBHttpServiceRespOpq* opq = (STNBHttpServiceRespOpq*)ref.opaque; NBASSERT(NBHttpServiceResp_isClass(ref))
    return NBHttpServiceResp_flushBodyOpq_(opq);
}


//resp itf

//response-header

void NBHttpServiceResp_httpReqRespInvalidate_(void* usrData){ //to stop buffering
    NBHttpServiceResp_respInvalidateOpq_((STNBHttpServiceRespOpq*)usrData);
}

void NBHttpServiceResp_httpReqRespClose_(void* usrData){ //required, to release ownership
    NBHttpServiceResp_respCloseOpq_((STNBHttpServiceRespOpq*)usrData);
}

BOOL NBHttpServiceResp_httpReqRespUpgradeToRaw_(STNBHttpServiceRespRawLnk* dstLnk, const BOOL includeRawRead, void* usrData){
    return NBHttpServiceResp_upgradeToRawOpq_((STNBHttpServiceRespOpq*)usrData, dstLnk, includeRawRead);
}

BOOL NBHttpServiceResp_httpReqRespSetResponseCode_(const UI32 code, const char* reason, void* usrData){ //required at least once
    return NBHttpServiceResp_setResponseCodeOpq_((STNBHttpServiceRespOpq*)usrData, code, reason);
}

UI32 NBHttpServiceResp_httpReqRespGetResponseCode_(void* usrData){
    return NBHttpServiceResp_getResponseCodeOpq_((STNBHttpServiceRespOpq*)usrData);
}

BOOL NBHttpServiceResp_httpReqRespAddHeaderField_(const char* name, const char* value, void* usrData){ //optional
    return NBHttpServiceResp_addHeaderFieldOpq_((STNBHttpServiceRespOpq*)usrData, name, value);
}

BOOL NBHttpServiceResp_httpReqRespEndHeader_(void* usrData){ //optional, mostly used when empty-body; the header is auto-ended when the body is started.
    return NBHttpServiceResp_endHeaderOpq_((STNBHttpServiceRespOpq*)usrData);
}

//response-body
BOOL NBHttpServiceResp_httpReqRespSetContentLength_(const UI64 contentLength, void* usrData){ //optimization, when size is known buffer is not used (data is send inmediatly)
    return NBHttpServiceResp_setContentLengthOpq_((STNBHttpServiceRespOpq*)usrData, contentLength);
}

BOOL NBHttpServiceResp_httpReqRespUnsetContentLength_(void* usrData){ //reverse of 'setContentLength'
    return NBHttpServiceResp_unsetContentLengthOpq_((STNBHttpServiceRespOpq*)usrData);
}

BOOL NBHttpServiceResp_httpReqRespConcatBody_(const char* str, void* usrData){
    return NBHttpServiceResp_concatBodyOpq_((STNBHttpServiceRespOpq*)usrData, str);
}

BOOL NBHttpServiceResp_httpReqRespConcatBodyBytes_(const void* data, const UI32 dataSz, void* usrData){
    return NBHttpServiceResp_concatBodyBytesOpq_((STNBHttpServiceRespOpq*)usrData, data, dataSz);
}

BOOL NBHttpServiceResp_httpReqRespConcatBodyStructAsJson_(const STNBStructMap* structMap, const void* src, const UI32 srcSz, void* usrData){
    return NBHttpServiceResp_concatBodyStructAsJsonOpq_((STNBHttpServiceRespOpq*)usrData, structMap, src, srcSz);
}

BOOL NBHttpServiceResp_httpReqRespSendBody_(const STNBHttpBody* body, void* usrData){
    return NBHttpServiceResp_sendBodyOpq_((STNBHttpServiceRespOpq*)usrData, body);
}

//resp-raw itf

BOOL NBHttpServiceResp_httpReqRespSendHeader_(const STNBHttpHeader* header, const BOOL doNotSendHeaderEnd, void* usrData){
    return NBHttpServiceResp_sendHeaderOpq_((STNBHttpServiceRespOpq*)usrData, header, doNotSendHeaderEnd);
}

BOOL NBHttpServiceResp_httpReqRespSendHeaderEnd_(void* usrData){
    return NBHttpServiceResp_sendHeaderEndOpq_((STNBHttpServiceRespOpq*)usrData);
}

//response-raw
BOOL NBHttpServiceResp_httpReqRespSendBytes_(const void* data, const UI32 dataSz, void* usrData){
    return NBHttpServiceResp_sendBytesOpq_((STNBHttpServiceRespOpq*)usrData, data, dataSz);
}
