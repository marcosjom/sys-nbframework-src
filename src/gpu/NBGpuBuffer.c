
#include "nb/NBFrameworkPch.h"
#include "nb/gpu/NBGpuBuffer.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBStructMaps.h"
#include "nb/core/NBMngrStructMaps.h"
#include "nb/core/NBArraySorted.h"

// NBGpuBufferCfg

STNBStructMapsRec NBGpuBufferCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBGpuBufferCfg_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBGpuBufferCfg_sharedStructMap);
    if(NBGpuBufferCfg_sharedStructMap.map == NULL){
        STNBGpuBufferCfg s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBGpuBufferCfg);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addStructM(map, s, mem, NBMemoryBlocksCfg_getSharedStructMap()); //memory blocks cfg
        NBGpuBufferCfg_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBGpuBufferCfg_sharedStructMap);
    return NBGpuBufferCfg_sharedStructMap.map;
}

// NBGpuBufferChanges

STNBStructMapsRec NBGpuBufferChanges_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBGpuBufferChanges_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBGpuBufferChanges_sharedStructMap);
    if(NBGpuBufferChanges_sharedStructMap.map == NULL){
        STNBGpuBufferChanges s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBGpuBufferChanges);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addBoolM(map, s, size); //buffer's size changed
        NBStructMap_addPtrToArrayOfStructM(map, s, rngs, rngsUse, ENNBStructMapSign_Unsigned, NBRangeU_getSharedStructMap());
        NBGpuBufferChanges_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBGpuBufferChanges_sharedStructMap);
    return NBGpuBufferChanges_sharedStructMap.map;
}

//STNBGpuBufferOpq

typedef struct STNBGpuBufferOpq_ {
    STNBObject prnt;
    //
    STNBGpuBufferCfg    cfg;    //config
    STNBMemoryBlocksRef mem;    //memory
    //state
    struct {
        UI32            totalSzLast;
    } state;
    //changes
    struct {
        BOOL            size;   //buffer size changed
        STNBArraySorted rngs;   //STNBRangeU
    } changes;
    //api
    struct {
        STNBGpuBufferApiItf itf;
        void*               itfParam;
        void*               data;
    } api;
} STNBGpuBufferOpq;

NB_OBJREF_BODY(NBGpuBuffer, STNBGpuBufferOpq, NBObject)

void NBGpuBuffer_initZeroed(STNBObject* obj) {
    STNBGpuBufferOpq* opq = (STNBGpuBufferOpq*)obj;
    //changes
    {
        NBArraySorted_initWithSz(&opq->changes.rngs, sizeof(STNBRangeU), NBCompare_STNBRangeU, 0, 128, 0.1f);
    }
}

void NBGpuBuffer_uninitLocked(STNBObject* obj){
    STNBGpuBufferOpq* opq = (STNBGpuBufferOpq*)obj;
    //api
    {
        if(opq->api.data != NULL){
            if(opq->api.itf.destroy != NULL){
                (*opq->api.itf.destroy)(opq->api.data, opq->api.itfParam);
            }
            opq->api.data = NULL;
        }
        NBMemory_setZeroSt(opq->api.itf, STNBGpuBufferApiItf);
        opq->api.itfParam = NULL;
    }
    //
    if(NBMemoryBlocks_isSet(opq->mem)){
        NBMemoryBlocks_release(&opq->mem);
        NBMemoryBlocks_null(&opq->mem);
    }
    NBStruct_stRelease(NBGpuBufferCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
    //changes
    {
        NBArraySorted_release(&opq->changes.rngs);
    }
}

//

BOOL NBGpuBuffer_prepare(STNBGpuBufferRef ref, const STNBGpuBufferCfg* cfg, const STNBGpuBufferApiItf* itf, void* itfParam) {
    BOOL r = FALSE;
    STNBGpuBufferOpq* opq = (STNBGpuBufferOpq*)ref.opaque;
    NBASSERT(NBGpuBuffer_isClass(ref))
    NBObject_lock(opq);
    if(cfg != NULL && !NBMemoryBlocks_isSet(opq->mem) && itf != NULL && itf->create != NULL && itf->destroy != NULL){
        void* data = (*itf->create)(cfg, itfParam);
        if(data != NULL){
            UI32 totalSz = 0;
            STNBMemoryBlocksRef mem = NBMemoryBlocks_alloc(NULL);
            if(NBMemoryBlocks_prepare(mem, &cfg->mem, &totalSz)){
                //set
                NBMemoryBlocks_set(&opq->mem, &mem);
                NBMemoryBlocks_null(&mem); //consume
                //
                NBStruct_stRelease(NBGpuBufferCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
                NBStruct_stClone(NBGpuBufferCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
                //state
                {
                    opq->state.totalSzLast = totalSz;
                }
                //changes
                {
                    opq->changes.size = TRUE;
                    NBArraySorted_empty(&opq->changes.rngs);
                }
                //api
                {
                    if(opq->api.data != NULL){
                        if(opq->api.itf.destroy != NULL){
                            (*opq->api.itf.destroy)(opq->api.data, opq->api.itfParam);
                        }
                        opq->api.data = NULL;
                    }
                    NBMemory_setZeroSt(opq->api.itf, STNBGpuBufferApiItf);
                    opq->api.itfParam = NULL;
                    //
                    if(itf != NULL){
                        opq->api.itf = *itf;
                        opq->api.itfParam = itfParam;
                    }
                    //data
                    opq->api.data = data; data = NULL; //consume
                }
                r = TRUE;
            }
            //relese (if not consumed)
            if(NBMemoryBlocks_isSet(mem)){
                NBMemoryBlocks_release(&mem);
                NBMemoryBlocks_null(&mem);
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

BOOL NBGpuBuffer_clear(STNBGpuBufferRef ref){
    BOOL r = FALSE;
    STNBGpuBufferOpq* opq = (STNBGpuBufferOpq*)ref.opaque;
    NBASSERT(NBGpuBuffer_isClass(ref))
    NBObject_lock(opq);
    if(NBMemoryBlocks_isSet(opq->mem)){
        NBMemoryBlocks_clear(opq->mem);
        r = TRUE;
    }
    NBObject_unlock(opq);
    return r;
}

STNBAbsPtr NBGpuBuffer_malloc(STNBGpuBufferRef ref, const UI32 usableSz){
    STNBAbsPtr r = STNBAbsPtr_Zero;
    STNBGpuBufferOpq* opq = (STNBGpuBufferOpq*)ref.opaque;
    NBASSERT(NBGpuBuffer_isClass(ref))
    NBObject_lock(opq);
    if(NBMemoryBlocks_isSet(opq->mem)){
        UI32 totalSz = 0;
        r = NBMemoryBlocks_malloc(opq->mem, usableSz, &totalSz);
        if(opq->state.totalSzLast != totalSz){
            opq->state.totalSzLast = totalSz;
            //changes
            opq->changes.size = TRUE;
            NBArraySorted_empty(&opq->changes.rngs);
        }
    }
    NBObject_unlock(opq);
    return r;
}

BOOL NBGpuBuffer_mfree(STNBGpuBufferRef ref, const STNBAbsPtr ptr){
    BOOL r = FALSE;
    STNBGpuBufferOpq* opq = (STNBGpuBufferOpq*)ref.opaque;
    NBASSERT(NBGpuBuffer_isClass(ref))
    NBObject_lock(opq);
    if(NBMemoryBlocks_isSet(opq->mem)){
        r = NBMemoryBlocks_mfree(opq->mem, ptr);
    }
    NBObject_unlock(opq);
    return r;
}

BOOL NBGpuBuffer_mInvalidate(STNBGpuBufferRef ref, const STNBAbsPtr ptr, const UI32 sz){
    BOOL r = FALSE;
    STNBGpuBufferOpq* opq = (STNBGpuBufferOpq*)ref.opaque;
    NBASSERT(NBGpuBuffer_isClass(ref))
    NBObject_lock(opq);
    if(NBMemoryBlocks_isSet(opq->mem) && opq->cfg.mem.sizeAlign > 0){
        if(sz > 0 && !opq->changes.size){ //no need to evaluate rngs if the whole buffer requires an update
            STNBRangeU rng = STNBRangeU_Zero;
            rng.start  = ptr.idx / opq->cfg.mem.sizeAlign * opq->cfg.mem.sizeAlign;
            rng.size      = ((ptr.idx + sz + opq->cfg.mem.sizeAlign - 1) / opq->cfg.mem.sizeAlign * opq->cfg.mem.sizeAlign) - rng.start;
            NBASSERT(rng.start <= ptr.idx && (ptr.idx + sz) <= (rng.start + rng.size))
            //ToDo
            STNBRangeU* gStart = NBArraySorted_dataPtr(&opq->changes.rngs, STNBRangeU);
            const SI32 iNxtRng = NBArraySorted_indexForNew(&opq->changes.rngs, &rng);
            if(iNxtRng < opq->changes.rngs.use && rng.start == gStart[iNxtRng].start && rng.size <= gStart[iNxtRng].size){
                //range already covered by range in current position
            } else if(iNxtRng > 0 && (rng.start + rng.size) <= (gStart[iNxtRng - 1].start + gStart[iNxtRng - 1].size)){
                //range already covered by previous range
            } else {
                //add new range
                NBArraySorted_addValue(&opq->changes.rngs, rng);
            }
        }
        r = TRUE;
    }
    NBObject_unlock(opq);
    return r;
}

