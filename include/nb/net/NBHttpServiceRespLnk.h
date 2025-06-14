#ifndef NBHttpServiceRespLnk_h
#define NBHttpServiceRespLnk_h

#include "nb/NBFrameworkDefs.h"
#include "nb/net/NBHttpHeader.h"
#include "nb/net/NBHttpBody.h"
#include "nb/net/NBHttpMessage.h"
//
#include "nb/net/NBHttpServiceRespRawLnk.h"

#ifdef __cplusplus
extern "C" {
#endif

    //NBHttpServiceRespLnkItf (used to send a managed-response without exposing this object)

    typedef struct STNBHttpServiceRespLnkItf_ {
        void (*httpReqRespInvalidate)(void* usrData); //to stop buffering
        void (*httpReqRespClose)(void* usrData); //required, to release ownership
        BOOL (*httpReqRespUpgradeToRaw)(STNBHttpServiceRespRawLnk* dstLnk, const BOOL includeRawRead, void* usrData);
        //response-header
        BOOL (*httpReqRespSetResponseCode)(const UI32 code, const char* reason, void* usrData); //required at least once
        UI32 (*httpReqRespGetResponseCode)(void* usrData);
        BOOL (*httpReqRespAddHeaderField)(const char* name, const char* value, void* usrData); //optional
        BOOL (*httpReqRespEndHeader)(void* usrData); //optional, mostly used when empty-body; the header is auto-ended when the body is started.
        //response-body
        BOOL (*httpReqRespSetContentLength)(const UI64 contentLength, void* usrData); //optimization, when size is known buffer is not used (data is send inmediatly)
        BOOL (*httpReqRespUnsetContentLength)(void* usrData); //reverse of 'setContentLength'
        BOOL (*httpReqRespConcatBody)(const char* str, void* usrData);
        BOOL (*httpReqRespConcatBodyBytes)(const void* data, const UI32 dataSz, void* usrData);
        BOOL (*httpReqRespConcatBodyStructAsJson)(const STNBStructMap* structMap, const void* src, const UI32 srcSz, void* usrData);
        BOOL (*httpReqRespSendBody)(const STNBHttpBody* body, void* usrData);
    } STNBHttpServiceRespLnkItf;

    //NBHttpServiceRespLnk (used to send a managed-response without exposing this object)

    typedef struct STNBHttpServiceRespLnk_ {
        STNBHttpServiceRespLnkItf   itf;
        void*                       itfParam;
    } STNBHttpServiceRespLnk;
    
    BOOL NBHttpServiceRespLnk_isSet(const STNBHttpServiceRespLnk* obj);
    //
    void NBHttpServiceRespLnk_close(const STNBHttpServiceRespLnk* obj); //required, to release ownership
    BOOL NBHttpServiceRespLnk_upgradeToRaw(const STNBHttpServiceRespLnk* obj, STNBHttpServiceRespRawLnk* dstLnk, const BOOL includeRawRead);
    //response-header
    BOOL NBHttpServiceRespLnk_setResponseCode(const STNBHttpServiceRespLnk* obj, const UI32 code, const char* reason); //required at least once
    UI32 NBHttpServiceRespLnk_getResponseCode(const STNBHttpServiceRespLnk* obj);
    BOOL NBHttpServiceRespLnk_addHeaderField(const STNBHttpServiceRespLnk* obj, const char* name, const char* value); //optional
    BOOL NBHttpServiceRespLnk_endHeader(const STNBHttpServiceRespLnk* obj); //optional, mostly used when empty-body; the header is auto-ended when the body is started.
    //response-body
    BOOL NBHttpServiceRespLnk_setContentLength(const STNBHttpServiceRespLnk* obj, const UI64 contentLength); //optimization, when size is known buffer is not used (data is send inmediatly)
    BOOL NBHttpServiceRespLnk_unsetContentLength(const STNBHttpServiceRespLnk* obj); //reverse of 'setContentLength'
    BOOL NBHttpServiceRespLnk_concatBody(const STNBHttpServiceRespLnk* obj, const char* str);
    BOOL NBHttpServiceRespLnk_concatBodyBytes(const STNBHttpServiceRespLnk* obj, const void* data, const UI32 dataSz);
    BOOL NBHttpServiceRespLnk_concatBodyStructAsJson(const STNBHttpServiceRespLnk* obj, const STNBStructMap* structMap, const void* src, const UI32 srcSz);
    BOOL NBHttpServiceRespLnk_sendBody(const STNBHttpServiceRespLnk* obj, const STNBHttpBody* body);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
