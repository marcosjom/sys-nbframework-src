
#include "nb/NBFrameworkPch.h"
#include "nb/gpu/NBGpuFramebuffer.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBStructMaps.h"
#include "nb/core/NBMngrStructMaps.h"
//
#include "nb/gpu/NBGpuTexture.h"
#include "nb/gpu/NBGpuRenderbuffer.h"

// NBGpuFramebufferCfg

STNBStructMapsRec NBGpuFramebufferCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBGpuFramebufferCfg_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBGpuFramebufferCfg_sharedStructMap);
    if(NBGpuFramebufferCfg_sharedStructMap.map == NULL){
        STNBGpuFramebufferCfg s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBGpuFramebufferCfg);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, dummy);
        NBGpuFramebufferCfg_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBGpuFramebufferCfg_sharedStructMap);
    return NBGpuFramebufferCfg_sharedStructMap.map;
}

// NBGpuFramebufferChanges

STNBStructMapsRec NBGpuFramebufferChanges_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBGpuFramebufferChanges_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBGpuFramebufferChanges_sharedStructMap);
    if(NBGpuFramebufferChanges_sharedStructMap.map == NULL){
        STNBGpuFramebufferChanges s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBGpuFramebufferChanges);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addBoolM(map, s, bind);
        NBGpuFramebufferChanges_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBGpuFramebufferChanges_sharedStructMap);
    return NBGpuFramebufferChanges_sharedStructMap.map;
}

//STNBGpuFramebufferOpq

typedef struct STNBGpuFramebufferOpq_ {
    STNBObject prnt;
    //
    STNBGpuFramebufferCfg    cfg;    //config
    //bind
    struct {
        ENNBGpuFramebufferDstType  type;
        STNBObjRef                  ref;    //STNBGpuTextureRef, STNBGpuRenderbufferRef
    } bind;
    //changes
    struct {
        BOOL            bind;
    } changes;
    //api
    struct {
        STNBGpuFramebufferApiItf itf;
        void*                   itfParam;
        void*                   data;
    } api;
} STNBGpuFramebufferOpq;

NB_OBJREF_BODY(NBGpuFramebuffer, STNBGpuFramebufferOpq, NBObject)

void NBGpuFramebuffer_initZeroed(STNBObject* obj) {
    //STNBGpuFramebufferOpq* opq = (STNBGpuFramebufferOpq*)obj;
    //nothing to do
}

void NBGpuFramebuffer_uninitLocked(STNBObject* obj){
    STNBGpuFramebufferOpq* opq = (STNBGpuFramebufferOpq*)obj;
    //api
    {
        if(opq->api.data != NULL){
            if(opq->api.itf.destroy != NULL){
                (*opq->api.itf.destroy)(opq->api.data, opq->api.itfParam);
            }
            opq->api.data = NULL;
        }
        NBMemory_setZeroSt(opq->api.itf, STNBGpuFramebufferApiItf);
        opq->api.itfParam = NULL;
    }
    //
    NBStruct_stRelease(NBGpuFramebufferCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
    //bind
    if(NBObjRef_isSet(opq->bind.ref)){
        NBObjRef_release(&opq->bind.ref);
        NBObjRef_null(&opq->bind.ref);
    }
}

//

BOOL NBGpuFramebuffer_prepare(STNBGpuFramebufferRef ref, const STNBGpuFramebufferCfg* cfg, const STNBGpuFramebufferApiItf* itf, void* itfParam) {
    BOOL r = FALSE;
    STNBGpuFramebufferOpq* opq = (STNBGpuFramebufferOpq*)ref.opaque;
    NBASSERT(NBGpuFramebuffer_isClass(ref))
    NBObject_lock(opq);
    if(cfg != NULL && itf != NULL && itf->create != NULL && itf->destroy != NULL){
        void* data = (*itf->create)(cfg, itfParam);
        if(data != NULL){
            NBStruct_stRelease(NBGpuFramebufferCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
            NBStruct_stClone(NBGpuFramebufferCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
            //api
            {
                if(opq->api.data != NULL){
                    if(opq->api.itf.destroy != NULL){
                        (*opq->api.itf.destroy)(opq->api.data, opq->api.itfParam);
                    }
                    opq->api.data = NULL;
                }
                NBMemory_setZeroSt(opq->api.itf, STNBGpuFramebufferApiItf);
                opq->api.itfParam = NULL;
                //
                if(itf != NULL){
                    opq->api.itf = *itf;
                    opq->api.itfParam = itfParam;
                }
                //data
                opq->api.data = data; data = NULL; //consume
            }
        }
        //destroy (if not consumed)
        if(data != NULL && itf->destroy != NULL){
            (*itf->destroy)(data, itfParam);
        }
    }
    NBObject_unlock(opq);
    return r;
}

BOOL NBGpuFramebuffer_bindTo(STNBGpuFramebufferRef ref, const STNBObjRef dstRef){
    BOOL r = FALSE;
    STNBGpuFramebufferOpq* opq = (STNBGpuFramebufferOpq*)ref.opaque;
    NBASSERT(NBGpuFramebuffer_isClass(ref))
    NBObject_lock(opq);
    {
        if(!NBObjRef_isSet(dstRef)){
            opq->bind.type = ENNBGpuFramebufferDstType_None;
            if(NBObjRef_isSet(opq->bind.ref)){
                NBObjRef_release(&opq->bind.ref);
                NBObjRef_null(&opq->bind.ref);
                opq->changes.bind = TRUE;
            }
            r = TRUE;
        } else if(NBGpuTexture_isClass(dstRef)){
            opq->bind.type = ENNBGpuFramebufferDstType_Texture;
            if(!NBObjRef_isSame(opq->bind.ref, dstRef)){
                NBObjRef_set(&opq->bind.ref, &dstRef);
                opq->changes.bind = TRUE;
            }
            r = TRUE;
        } else if(NBGpuRenderbuffer_isClass(dstRef)){
            opq->bind.type = ENNBGpuFramebufferDstType_Renderbuffer;
            if(!NBObjRef_isSame(opq->bind.ref, dstRef)){
                NBObjRef_set(&opq->bind.ref, &dstRef);
                opq->changes.bind = TRUE;
            }
            r = TRUE;
        }
    }
    NBObject_unlock(opq);
    return r;
}
