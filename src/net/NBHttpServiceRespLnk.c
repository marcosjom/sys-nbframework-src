
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBHttpServiceRespLnk.h"

//NBHttpServiceRespLnk (used to send a managed-response without exposing this object)

BOOL NBHttpServiceRespLnk_isSet(const STNBHttpServiceRespLnk* obj){
    return (obj != NULL && obj->itf.httpReqRespClose != NULL);
}

//

void NBHttpServiceRespLnk_close(const STNBHttpServiceRespLnk* obj){ //required, to release ownership
    if(obj != NULL && obj->itf.httpReqRespClose != NULL){
        (*obj->itf.httpReqRespClose)(obj->itfParam);
    }
}

BOOL NBHttpServiceRespLnk_upgradeToRaw(const STNBHttpServiceRespLnk* obj, STNBHttpServiceRespRawLnk* dstLnk, const BOOL includeRawRead){
    BOOL r = FALSE;
    if(obj != NULL && obj->itf.httpReqRespUpgradeToRaw != NULL){
        r = (*obj->itf.httpReqRespUpgradeToRaw)(dstLnk, includeRawRead, obj->itfParam);
    }
    return r;
}

//response-header

BOOL NBHttpServiceRespLnk_setResponseCode(const STNBHttpServiceRespLnk* obj, const UI32 code, const char* reason){ //required at least once
    BOOL r = FALSE;
    if(obj != NULL && obj->itf.httpReqRespSetResponseCode != NULL){
        r = (*obj->itf.httpReqRespSetResponseCode)(code, reason, obj->itfParam);
    }
    return r;
}

UI32 NBHttpServiceRespLnk_getResponseCode(const STNBHttpServiceRespLnk* obj){
    UI32 r = 0;
    if(obj != NULL && obj->itf.httpReqRespGetResponseCode != NULL){
        r = (*obj->itf.httpReqRespGetResponseCode)(obj->itfParam);
    }
    return r;
}

BOOL NBHttpServiceRespLnk_addHeaderField(const STNBHttpServiceRespLnk* obj, const char* name, const char* value){ //optional
    BOOL r = FALSE;
    if(obj != NULL && obj->itf.httpReqRespAddHeaderField != NULL){
        r = (*obj->itf.httpReqRespAddHeaderField)(name, value, obj->itfParam);
    }
    return r;
}

BOOL NBHttpServiceRespLnk_endHeader(const STNBHttpServiceRespLnk* obj){ //optional, mostly used when empty-body; the header is auto-ended when the body is started.
    BOOL r = FALSE;
    if(obj != NULL && obj->itf.httpReqRespEndHeader != NULL){
        r = (*obj->itf.httpReqRespEndHeader)(obj->itfParam);
    }
    return r;
}

//response-body

BOOL NBHttpServiceRespLnk_setContentLength(const STNBHttpServiceRespLnk* obj, const UI64 contentLength){ //optimization, when size is known buffer is not used (data is send inmediatly)
    BOOL r = FALSE;
    if(obj != NULL && obj->itf.httpReqRespSetContentLength != NULL){
        r = (*obj->itf.httpReqRespSetContentLength)(contentLength, obj->itfParam);
    }
    return r;
}

BOOL NBHttpServiceRespLnk_unsetContentLength(const STNBHttpServiceRespLnk* obj){ //reverse of 'setContentLength'
    BOOL r = FALSE;
    if(obj != NULL && obj->itf.httpReqRespUnsetContentLength != NULL){
        r = (*obj->itf.httpReqRespUnsetContentLength)(obj->itfParam);
    }
    return r;
}

BOOL NBHttpServiceRespLnk_concatBody(const STNBHttpServiceRespLnk* obj, const char* str){
    BOOL r = FALSE;
    if(obj != NULL && obj->itf.httpReqRespConcatBody != NULL){
        r = (*obj->itf.httpReqRespConcatBody)(str, obj->itfParam);
    }
    return r;
}

BOOL NBHttpServiceRespLnk_concatBodyBytes(const STNBHttpServiceRespLnk* obj, const void* data, const UI32 dataSz){
    BOOL r = FALSE;
    if(obj != NULL && obj->itf.httpReqRespConcatBodyBytes != NULL){
        r = (*obj->itf.httpReqRespConcatBodyBytes)(data, dataSz, obj->itfParam);
    }
    return r;
}

BOOL NBHttpServiceRespLnk_concatBodyStructAsJson(const STNBHttpServiceRespLnk* obj, const STNBStructMap* structMap, const void* src, const UI32 srcSz){
    BOOL r = FALSE;
    if(obj != NULL && obj->itf.httpReqRespConcatBodyStructAsJson != NULL){
        r = (*obj->itf.httpReqRespConcatBodyStructAsJson)(structMap, src, srcSz, obj->itfParam);
    }
    return r;
}

BOOL NBHttpServiceRespLnk_sendBody(const STNBHttpServiceRespLnk* obj, const STNBHttpBody* body){
    BOOL r = FALSE;
    if(obj != NULL && obj->itf.httpReqRespSendBody != NULL){
        r = (*obj->itf.httpReqRespSendBody)(body, obj->itfParam);
    }
    return r;
}
