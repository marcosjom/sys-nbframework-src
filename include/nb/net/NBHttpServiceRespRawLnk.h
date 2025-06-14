#ifndef NBHttpServiceRespRawLnk_h
#define NBHttpServiceRespRawLnk_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBStructMap.h"
#include "nb/net/NBHttpHeader.h"
#include "nb/net/NBHttpBody.h"
#include "nb/net/NBHttpMessage.h"

#ifdef __cplusplus
extern "C" {
#endif

    //NBHttpServiceRespRawLnkItf (used to send the raw-response and receive bytes without exposing this object)

    typedef struct STNBHttpServiceRespRawLnkItf_ {
        void (*httpReqRespInvalidate)(void* usrData); //to stop buffering
        void (*httpReqRespClose)(void* usrData); //required, to release ownership
        //response-header
        BOOL (*httpReqRespSendHeader)(const STNBHttpHeader* header, const BOOL doNotSendHeaderEnd, void* usrData);
        BOOL (*httpReqRespSendHeaderEnd)(void* usrData);
        //response-raw
        BOOL (*httpReqRespSendBytes)(const void* data, const UI32 dataSz, void* usrData);
    } STNBHttpServiceRespRawLnkItf;

    //NBHttpServiceRespRawLnk

    typedef struct STNBHttpServiceRespRawLnk_ {
        STNBHttpServiceRespRawLnkItf    itf;
        void*                           itfParam;
    } STNBHttpServiceRespRawLnk;

    BOOL NBHttpServiceRespRawLnk_isSet(const STNBHttpServiceRespRawLnk* obj);
    void NBHttpServiceRespRawLnk_close(const STNBHttpServiceRespRawLnk* obj);
    //response-header
    BOOL NBHttpServiceRespRawLnk_sendHeader(const STNBHttpServiceRespRawLnk* obj, const STNBHttpHeader* header, const BOOL doNotSendHeaderEnd);
    BOOL NBHttpServiceRespRawLnk_sendHeaderEnd(const STNBHttpServiceRespRawLnk* obj);
    //response-raw
    BOOL NBHttpServiceRespRawLnk_sendBytes(const STNBHttpServiceRespRawLnk* obj, const void* data, const UI32 dataSz);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
