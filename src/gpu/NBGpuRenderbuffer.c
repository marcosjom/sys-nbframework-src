
#include "nb/NBFrameworkPch.h"
#include "nb/gpu/NBGpuRenderbuffer.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBStructMaps.h"
#include "nb/core/NBMngrStructMaps.h"


// NBGpuRenderbufferCfg

STNBStructMapsRec NBGpuRenderbufferCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBGpuRenderbufferCfg_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBGpuRenderbufferCfg_sharedStructMap);
    if(NBGpuRenderbufferCfg_sharedStructMap.map == NULL){
        STNBGpuRenderbufferCfg s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBGpuRenderbufferCfg);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addEnumM(map, s, color, NBBitmapColor_getSharedEnumMap());
        NBStructMap_addUIntM(map, s, width);
        NBStructMap_addUIntM(map, s, color);
        NBGpuRenderbufferCfg_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBGpuRenderbufferCfg_sharedStructMap);
    return NBGpuRenderbufferCfg_sharedStructMap.map;
}

// NBGpuRenderbufferChanges

STNBStructMapsRec NBGpuRenderbufferChanges_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBGpuRenderbufferChanges_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBGpuRenderbufferChanges_sharedStructMap);
    if(NBGpuRenderbufferChanges_sharedStructMap.map == NULL){
        STNBGpuRenderbufferChanges s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBGpuRenderbufferChanges);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, dummy);
        NBGpuRenderbufferChanges_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBGpuRenderbufferChanges_sharedStructMap);
    return NBGpuRenderbufferChanges_sharedStructMap.map;
}

//STNBGpuRenderbufferOpq

typedef struct STNBGpuRenderbufferOpq_ {
    STNBObject prnt;
    //
    STNBGpuRenderbufferCfg    cfg;    //config
    //api
    struct {
        STNBGpuRenderbufferApiItf   itf;
        void*                       itfParam;
        void*                       data;
    } api;
} STNBGpuRenderbufferOpq;

NB_OBJREF_BODY(NBGpuRenderbuffer, STNBGpuRenderbufferOpq, NBObject)

void NBGpuRenderbuffer_initZeroed(STNBObject* obj) {
    //STNBGpuRenderbufferOpq* opq = (STNBGpuRenderbufferOpq*)obj;
    //nothing to do
}

void NBGpuRenderbuffer_uninitLocked(STNBObject* obj){
    STNBGpuRenderbufferOpq* opq = (STNBGpuRenderbufferOpq*)obj;
    //api
    {
        if(opq->api.data != NULL){
            if(opq->api.itf.destroy != NULL){
                (*opq->api.itf.destroy)(opq->api.data, opq->api.itfParam);
            }
            opq->api.data = NULL;
        }
        NBMemory_setZeroSt(opq->api.itf, STNBGpuRenderbufferApiItf);
        opq->api.itfParam = NULL;
    }
    //
    NBStruct_stRelease(NBGpuRenderbufferCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
}

//

BOOL NBGpuRenderbuffer_prepare(STNBGpuRenderbufferRef ref, const STNBGpuRenderbufferCfg* cfg, const STNBGpuRenderbufferApiItf* itf, void* itfParam) {
    BOOL r = FALSE;
    STNBGpuRenderbufferOpq* opq = (STNBGpuRenderbufferOpq*)ref.opaque;
    NBASSERT(NBGpuRenderbuffer_isClass(ref))
    NBObject_lock(opq);
    if(cfg != NULL && cfg->width > 0 && cfg->height > 0 && opq->cfg.width == 0 && itf != NULL && itf->create != NULL && itf->destroy != NULL){
        void* data = (*itf->create)(cfg, itfParam);
        if(data != NULL){
            NBStruct_stRelease(NBGpuRenderbufferCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
            NBStruct_stClone(NBGpuRenderbufferCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
            //api
            {
                if(opq->api.data != NULL){
                    if(opq->api.itf.destroy != NULL){
                        (*opq->api.itf.destroy)(opq->api.data, opq->api.itfParam);
                    }
                    opq->api.data = NULL;
                }
                NBMemory_setZeroSt(opq->api.itf, STNBGpuRenderbufferApiItf);
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
