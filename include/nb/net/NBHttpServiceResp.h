#ifndef NBHttpServiceResp_h
#define NBHttpServiceResp_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBIOPollster.h"
#include "nb/net/NBHttpHeader.h"
#include "nb/net/NBHttpBody.h"
#include "nb/net/NBHttpMessage.h"
#include "nb/net/NBHttpCfg.h"
#include "nb/crypto/NBX509.h"
#include "nb/ssl/NBSslContext.h"
//
#include "nb/net/NBHttpStats.h"
#include "nb/net/NBHttpServiceRespLnk.h"
#include "nb/net/NBHttpServiceRespRawLnk.h"

#ifdef __cplusplus
extern "C" {
#endif

    //NBHttpServiceResp

    NB_OBJREF_HEADER(NBHttpServiceResp)    //client request's reponse

    //NBHttpServiceRespSenderItf (used to send data without direct access to the Socket object)

    typedef struct STNBHttpServiceRespSenderItf_ {
        SI32 (*httpReqSend)(STNBHttpServiceRespRef req, void* data, const SI32 dataSz, void* usrData);
    } STNBHttpServiceRespSenderItf;

    BOOL NBHttpServiceResp_prepare(STNBHttpServiceRespRef ref, const STNBHttpConnCfg* cfg);

    //state
    void NBHttpServiceResp_invalidate(STNBHttpServiceRespRef ref);        //stop buffering
	BOOL NBHttpServiceResp_isLogicError(STNBHttpServiceRespRef ref);      //internal logic error or expliclty-invalidated
    BOOL NBHttpServiceResp_isUpgradedRaw(STNBHttpServiceRespRef ref);     //send raw-response
    BOOL NBHttpServiceResp_isUpgradedRawRead(STNBHttpServiceRespRef ref); //send raw-response and read socket data
    BOOL NBHttpServiceResp_isOrphan(STNBHttpServiceRespRef ref);

    //itf to send managed-response
    void NBHttpServiceResp_getRespItf(STNBHttpServiceRespRef ref, STNBHttpServiceRespLnkItf* dstItf, void** dstUsrParam);

	//io-write
    UI64 NBHttpServiceResp_writeQueuedTotal(STNBHttpServiceRespRef ref);   //ammount of bytes queued
    UI64 NBHttpServiceResp_writeQueuedConsumed(STNBHttpServiceRespRef ref);//ammount of bytes consumed/sent after queued
	BOOL NBHttpServiceResp_isWritePend(STNBHttpServiceRespRef ref);
	SI32 NBHttpServiceResp_write(STNBHttpServiceRespRef ref, STNBHttpServiceRespSenderItf* sndrItf, void* sndrUsrData);

    //response-header
    BOOL NBHttpServiceResp_setResponseCode(STNBHttpServiceRespRef ref, const UI32 code, const char* reason);
    UI32 NBHttpServiceResp_getResponseCode(STNBHttpServiceRespRef ref);
    BOOL NBHttpServiceResp_addHeaderField(STNBHttpServiceRespRef ref, const char* name, const char* value);
    BOOL NBHttpServiceResp_endHeader(STNBHttpServiceRespRef ref);
    //response-body
    BOOL NBHttpServiceResp_setContentLength(STNBHttpServiceRespRef ref, const UI64 contentLength);
    BOOL NBHttpServiceResp_unsetContentLength(STNBHttpServiceRespRef ref);
    BOOL NBHttpServiceResp_concatBodyBytes(STNBHttpServiceRespRef ref, const void* data, const UI32 dataSz);
    BOOL NBHttpServiceResp_concatBody(STNBHttpServiceRespRef ref, const char* str);
    BOOL NBHttpServiceResp_concatBodyStructAsJson(STNBHttpServiceRespRef ref, const STNBStructMap* structMap, const void* src, const UI32 srcSz);
    BOOL NBHttpServiceResp_sendBody(STNBHttpServiceRespRef ref, const STNBHttpBody* body);
    BOOL NBHttpServiceResp_flushBody(STNBHttpServiceRespRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
