
#include "nb/NBFrameworkPch.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBHndlNative.h"


BOOL NBHndlNative_set(STNBHndlNative* obj, const ENNBHndlNativeType type, void* src, const SI32 srcSz){
    BOOL r = FALSE;
    NBASSERT(type >= 0 && type < ENNBHndlNativeType_Count)
    NBASSERT(srcSz >= 0 && srcSz <= sizeof(obj->v))
    if(type >= 0 && type < ENNBHndlNativeType_Count && srcSz >= 0 && srcSz <= sizeof(obj->v)){
        obj->type   = type;
        obj->vUse   = srcSz;
        if(srcSz > 0){
            NBMemory_copy(obj->v, src, srcSz);
        }
        r = TRUE;
    }
    return r;
}

BOOL NBHndlNative_get(STNBHndlNative* obj, void* dst, const SI32 dstSz){
    BOOL r = FALSE;
    NBASSERT(dstSz == obj->vUse)
    if(dstSz == obj->vUse){
        if(dst != NULL && obj->vUse > 0){
            NBMemory_copy(dst, obj->v, obj->vUse);
        }
        r = TRUE;
    }
    return r;
}
