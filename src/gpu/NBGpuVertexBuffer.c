
#include "nb/NBFrameworkPch.h"
#include "nb/gpu/NBGpuVertexBuffer.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBStructMaps.h"
#include "nb/core/NBMngrStructMaps.h"
#include "nb/core/NBArraySorted.h"

// NBGpuVertexPartDef

STNBStructMapsRec NBGpuVertexPartDef_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBGpuVertexPartDef_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBGpuVertexPartDef_sharedStructMap);
    if(NBGpuVertexPartDef_sharedStructMap.map == NULL){
        STNBGpuVertexPartDef s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBGpuVertexPartDef);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, amm);      //ammount of elements of this part (coords, color, textureCoords, ...)
        NBStructMap_addUIntM(map, s, type);     //ENNBGpuDataType
        NBStructMap_addUIntM(map, s, offset);   //offset inside a record to the first element
        NBGpuVertexPartDef_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBGpuVertexPartDef_sharedStructMap);
    return NBGpuVertexPartDef_sharedStructMap.map;
}

// NBGpuVertexBufferCfg

STNBStructMapsRec NBGpuVertexBufferCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBGpuVertexBufferCfg_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBGpuVertexBufferCfg_sharedStructMap);
    if(NBGpuVertexBufferCfg_sharedStructMap.map == NULL){
        STNBGpuVertexBufferCfg s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBGpuVertexBufferCfg);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, szPerRecord); //bytes per record
        NBStructMap_addStructM(map, s, indices, NBGpuVertexPartDef_getSharedStructMap());
        NBStructMap_addStructM(map, s, coord, NBGpuVertexPartDef_getSharedStructMap());
        NBStructMap_addStructM(map, s, color, NBGpuVertexPartDef_getSharedStructMap());
        NBStructMap_addArrayOfStructM(map, s, texCoords, NBGpuVertexPartDef_getSharedStructMap());
        NBGpuVertexBufferCfg_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBGpuVertexBufferCfg_sharedStructMap);
    return NBGpuVertexBufferCfg_sharedStructMap.map;
}

//STNBGpuVertexBufferOpq

typedef struct STNBGpuVertexBufferOpq_ {
    STNBObject prnt;
    //
    STNBGpuVertexBufferCfg    cfg;    //config
    //buffs
    struct {
        STNBGpuBufferRef vertex;
        STNBGpuBufferRef idxs;
    } buffs;
    //api
    struct {
        STNBGpuVertexBufferApiItf itf;
        void*               itfParam;
        void*               data;
    } api;
} STNBGpuVertexBufferOpq;

NB_OBJREF_BODY(NBGpuVertexBuffer, STNBGpuVertexBufferOpq, NBObject)

void NBGpuVertexBuffer_initZeroed(STNBObject* obj) {
    //STNBGpuVertexBufferOpq* opq = (STNBGpuVertexBufferOpq*)obj;
}

void NBGpuVertexBuffer_uninitLocked(STNBObject* obj){
    STNBGpuVertexBufferOpq* opq = (STNBGpuVertexBufferOpq*)obj;
    //api
    {
        if(opq->api.data != NULL){
            if(opq->api.itf.destroy != NULL){
                (*opq->api.itf.destroy)(opq->api.data, opq->api.itfParam);
            }
            opq->api.data = NULL;
        }
        NBMemory_setZeroSt(opq->api.itf, STNBGpuVertexBufferApiItf);
        opq->api.itfParam = NULL;
    }
    //buffs
    {
        if(NBGpuBuffer_isSet(opq->buffs.vertex)){
            NBGpuBuffer_release(&opq->buffs.vertex);
            NBGpuBuffer_null(&opq->buffs.vertex);
        }
        if(NBGpuBuffer_isSet(opq->buffs.idxs)){
            NBGpuBuffer_release(&opq->buffs.idxs);
            NBGpuBuffer_null(&opq->buffs.idxs);
        }
    }
    //
    NBStruct_stRelease(NBGpuVertexBufferCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
}

//

BOOL NBGpuVertexBuffer_prepare(STNBGpuVertexBufferRef ref, const STNBGpuVertexBufferCfg* cfg, STNBGpuBufferRef vertexBuff, STNBGpuBufferRef idxsBuff, const STNBGpuVertexBufferApiItf* itf, void* itfParam) {
    BOOL r = FALSE;
    STNBGpuVertexBufferOpq* opq = (STNBGpuVertexBufferOpq*)ref.opaque;
    NBASSERT(NBGpuVertexBuffer_isClass(ref))
    NBObject_lock(opq);
    if(cfg != NULL && itf != NULL && itf->create != NULL && itf->destroy != NULL && NBGpuBuffer_isSet(vertexBuff)){
        void* data = (*itf->create)(cfg, vertexBuff, idxsBuff, itfParam);
        if(data != NULL){
            //
            NBStruct_stRelease(NBGpuVertexBufferCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
            NBStruct_stClone(NBGpuVertexBufferCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
            //api
            {
                if(opq->api.data != NULL){
                    if(opq->api.itf.destroy != NULL){
                        (*opq->api.itf.destroy)(opq->api.data, opq->api.itfParam);
                    }
                    opq->api.data = NULL;
                }
                if(NBGpuBuffer_isSet(opq->buffs.vertex)){
                    NBGpuBuffer_release(&opq->buffs.vertex);
                    NBGpuBuffer_null(&opq->buffs.vertex);
                }
                if(NBGpuBuffer_isSet(opq->buffs.idxs)){
                    NBGpuBuffer_release(&opq->buffs.idxs);
                    NBGpuBuffer_null(&opq->buffs.idxs);
                }
                //
                NBMemory_setZeroSt(opq->api.itf, STNBGpuVertexBufferApiItf);
                opq->api.itfParam = NULL;
                //
                if(itf != NULL){
                    opq->api.itf = *itf;
                    opq->api.itfParam = itfParam;
                }
                //data
                opq->api.data = data; data = NULL; //consume
                //
                NBGpuBuffer_set(&opq->buffs.vertex, &vertexBuff);
                NBGpuBuffer_set(&opq->buffs.idxs, &idxsBuff);
            }
            r = TRUE;
        }
        //destroy (if not consumed)
        if(data != NULL && itf->destroy != NULL){
            (*itf->destroy)(data, itfParam);
        }
    }
    NBObject_unlock(opq);
    return r;
}

BOOL NBGpuVertexBuffer_activate(STNBGpuVertexBufferRef ref){
    BOOL r = FALSE;
    STNBGpuVertexBufferOpq* opq = (STNBGpuVertexBufferOpq*)ref.opaque;
    NBASSERT(NBGpuVertexBuffer_isClass(ref))
    NBObject_lock(opq);
    if(opq->api.itf.activate != NULL){
        r = (*opq->api.itf.activate)(opq->api.data, &opq->cfg, opq->api.itfParam);
    }
    NBObject_unlock(opq);
    return r;
}

BOOL NBGpuVertexBuffer_deactivate(STNBGpuVertexBufferRef ref){
    BOOL r = FALSE;
    STNBGpuVertexBufferOpq* opq = (STNBGpuVertexBufferOpq*)ref.opaque;
    NBASSERT(NBGpuVertexBuffer_isClass(ref))
    NBObject_lock(opq);
    if(opq->api.itf.deactivate != NULL){
        r = (*opq->api.itf.deactivate)(opq->api.data, opq->api.itfParam);
    }
    NBObject_unlock(opq);
    return r;
}

