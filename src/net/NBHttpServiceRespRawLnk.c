
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBHttpServiceRespRawLnk.h"

//NBHttpServiceRespRawLnk

BOOL NBHttpServiceRespRawLnk_isSet(const STNBHttpServiceRespRawLnk* obj){
    return (obj != NULL && obj->itf.httpReqRespClose != NULL);
}

void NBHttpServiceRespRawLnk_close(const STNBHttpServiceRespRawLnk* obj){ //required, to release ownership
    if(obj != NULL && obj->itf.httpReqRespClose != NULL){
        (*obj->itf.httpReqRespClose)(obj->itfParam);
    }
}

//response-header

BOOL NBHttpServiceRespRawLnk_sendHeader(const STNBHttpServiceRespRawLnk* obj, const STNBHttpHeader* header, const BOOL doNotSendHeaderEnd){
    BOOL r = FALSE;
    if(obj != NULL && obj->itf.httpReqRespSendHeader != NULL){
        r = (*obj->itf.httpReqRespSendHeader)(header, doNotSendHeaderEnd, obj->itfParam);
    }
    return r;
}

BOOL NBHttpServiceRespRawLnk_sendHeaderEnd(const STNBHttpServiceRespRawLnk* obj){
    BOOL r = FALSE;
    if(obj != NULL && obj->itf.httpReqRespSendHeaderEnd != NULL){
        r = (*obj->itf.httpReqRespSendHeaderEnd)(obj->itfParam);
    }
    return r;
}

//response-raw

BOOL NBHttpServiceRespRawLnk_sendBytes(const STNBHttpServiceRespRawLnk* obj, const void* data, const UI32 dataSz){
    BOOL r = FALSE;
    if(obj != NULL && obj->itf.httpReqRespSendBytes != NULL){
        r = (*obj->itf.httpReqRespSendBytes)(data, dataSz, obj->itfParam);
    }
    return r;
}
