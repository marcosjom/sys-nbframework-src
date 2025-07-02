
#include "nb/NBFrameworkPch.h"
#include "nb/scene/NBScnRender.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBStructMaps.h"
#include "nb/core/NBMngrStructMaps.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBArraySorted.h"

//STNBScnRenderBuff

#define STNBScnRenderBuff_Zero { 0, STNBObjRef_Zero }

typedef struct STNBScnRenderBuff_ {
    UI32                uid;    //internal unique-id
    STNBGpuBufferRef    buff;   //buffer
} STNBScnRenderBuff;

void NBScnRenderBuff_release(STNBScnRenderBuff* obj);

BOOL NBCompare_NBScnRenderBuff(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
    NBASSERT(dataSz == sizeof(STNBScnRenderBuff))
    if(dataSz == sizeof(STNBScnRenderBuff)){
        const STNBScnRenderBuff* d1 = (STNBScnRenderBuff*)data1;
        const STNBScnRenderBuff* d2 = (STNBScnRenderBuff*)data2;
        switch (mode) {
            case ENCompareMode_Equal: return (d1->uid == d2->uid);
            case ENCompareMode_Lower: return (d1->uid < d2->uid);
            case ENCompareMode_LowerOrEqual: return (d1->uid <= d2->uid);
            case ENCompareMode_Greater: return (d1->uid > d2->uid);
            case ENCompareMode_GreaterOrEqual: return (d1->uid >= d2->uid);
            default: NBASSERT(FALSE) break;
        }
    }
    return FALSE;
}

//STNBScnRenderVertexBuff

#define STNBScnRenderVertexBuff_Zero { 0, STNBObjRef_Zero }

typedef struct STNBScnRenderVertexBuff_ {
    UI32                uid;    //internal unique-id
    STNBGpuVertexBufferRef buff;   //buffer
} STNBScnRenderVertexBuff;

void NBScnRenderVertexBuff_release(STNBScnRenderVertexBuff* obj);

BOOL NBCompare_NBScnRenderVertexBuff(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
    NBASSERT(dataSz == sizeof(STNBScnRenderVertexBuff))
    if(dataSz == sizeof(STNBScnRenderVertexBuff)){
        const STNBScnRenderVertexBuff* d1 = (STNBScnRenderVertexBuff*)data1;
        const STNBScnRenderVertexBuff* d2 = (STNBScnRenderVertexBuff*)data2;
        switch (mode) {
            case ENCompareMode_Equal: return (d1->uid == d2->uid);
            case ENCompareMode_Lower: return (d1->uid < d2->uid);
            case ENCompareMode_LowerOrEqual: return (d1->uid <= d2->uid);
            case ENCompareMode_Greater: return (d1->uid > d2->uid);
            case ENCompareMode_GreaterOrEqual: return (d1->uid >= d2->uid);
            default: NBASSERT(FALSE) break;
        }
    }
    return FALSE;
}

//STNBScnRenderOpq

typedef struct STNBScnRenderOpq_ {
    STNBObject  prnt;
    //api
    struct {
        STNBScnRenderApiItf itf;
        void*               itfParam;
        void*               data;
    } api;
    //buffs
    struct {
        UI32            iSeq;   //uid seq
        STNBArraySorted arr;    //STNBScnRenderBuff
    } buffs;
    //buffs
    struct {
        UI32            iSeq;   //uid seq
        STNBArraySorted arr;    //STNBScnRenderBuff
    } vertexBuffs;
    //job
    struct {
        //nodes
        struct {
            STNBArray   arr;      //STNBScnRenderNode
            UI16        iDepth;
            UI32        ammOpen;
            UI32        ammPoppedAtEnd; //pop-optimization, ammount of nodes already closed at the end of the array
        } nodes;
        STNBArray   cmds;       //STNBScnRenderCmd
    } job;
} STNBScnRenderOpq;

NB_OBJREF_BODY(NBScnRender, STNBScnRenderOpq, NBObject)

void NBScnRender_initZeroed(STNBObject* obj) {
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)obj;
    //api
    {
        //
    }
    //buffers
    {
        NBArraySorted_initWithSz(&opq->buffs.arr, sizeof(STNBScnRenderBuff), NBCompare_NBScnRenderBuff, 0, 16, 0.1f);
    }
    //vertexBuffers
    {
        NBArraySorted_initWithSz(&opq->vertexBuffs.arr, sizeof(STNBScnRenderVertexBuff), NBCompare_NBScnRenderVertexBuff, 0, 16, 0.1f);
    }
    //job
    {
        NBArray_initWithSz(&opq->job.nodes.arr, sizeof(STNBScnRenderNode), NULL, 256, 256, 0.1f);
        NBArray_initWithSz(&opq->job.cmds, sizeof(STNBScnRenderCmd), NULL, 256, 256, 0.1f);
    }
}

void NBScnRender_uninitLocked(STNBObject* obj){
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)obj;
    //job
    {
        NBArray_release(&opq->job.nodes.arr);
        NBArray_release(&opq->job.cmds);
    }
    //vertexBuffs
    {
        STNBScnRenderVertexBuff* b = NBArraySorted_dataPtr(&opq->vertexBuffs.arr, STNBScnRenderVertexBuff);
        const STNBScnRenderVertexBuff* bAfterEnd = b + opq->vertexBuffs.arr.use;
        while(b < bAfterEnd){
            NBScnRenderVertexBuff_release(b);
            ++b;
        }
        NBArraySorted_empty(&opq->vertexBuffs.arr);
        NBArraySorted_release(&opq->vertexBuffs.arr);
        opq->vertexBuffs.iSeq = 0;
    }
    //buffs
    {
        STNBScnRenderBuff* b = NBArraySorted_dataPtr(&opq->buffs.arr, STNBScnRenderBuff);
        const STNBScnRenderBuff* bAfterEnd = b + opq->buffs.arr.use;
        while(b < bAfterEnd){
            NBScnRenderBuff_release(b);
            ++b;
        }
        NBArraySorted_empty(&opq->buffs.arr);
        NBArraySorted_release(&opq->buffs.arr);
        opq->buffs.iSeq = 0;
    }
    //api
    {
        if(opq->api.data != NULL){
            opq->api.data = NULL;
        }
        NBMemory_setZeroSt(opq->api.itf, STNBScnRenderApiItf);
        opq->api.itfParam = NULL;
    }
}

//prepare

BOOL NBScnRender_prepare(STNBScnRenderRef ref, const STNBScnRenderApiItf* itf, void* itfParam){
    BOOL r = FALSE;
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    if(opq->buffs.arr.use == 0 && itf != NULL){
        r = TRUE;
        //
        opq->api.itf = *itf;
        opq->api.itfParam = itfParam;
        //initial bufffers
        {
            SI32 i; for(i = 0; i <= ENNBScnVertexType_Count; i++){
                UI32 itmSz = 0, ammPerBock = 0;
                switch(i){
                    case ENNBScnVertexType_Color:
                        itmSz = sizeof(STNBScnVertex);
                        ammPerBock = 256;
                        break;
                    case ENNBScnVertexType_Tex:
                        itmSz = sizeof(STNBScnVertexTex);
                        ammPerBock = 1024;
                        break;
                    case ENNBScnVertexType_Tex2:
                        itmSz = sizeof(STNBScnVertexTex2);
                        ammPerBock = 256;
                        break;
                    case ENNBScnVertexType_Tex3:
                        itmSz = sizeof(STNBScnVertexTex3);
                        ammPerBock = 256;
                        break;
                    case ENNBScnVertexType_Count:
                        itmSz = sizeof(STNBScnVertexIdx);
                        ammPerBock = 2048;
                        break;
                    default: NBASSERT(FALSE); break;
                }
                {
                    STNBGpuBufferCfg cfg    = STNBGpuBufferCfg_Zero;
                    cfg.type                = (ENNBScnVertexType)i;
                    cfg.mem.idxsAlign       = itmSz;
                    cfg.mem.sizeAlign       = 256;
                    cfg.mem.sizeInitial     = 0;
                    //calculate sizePerBlock
                    {
                        UI32 idxExtra = 0;
                        cfg.mem.sizePerBlock = ((ammPerBock * cfg.mem.idxsAlign) + cfg.mem.sizeAlign - 1) / cfg.mem.sizeAlign * cfg.mem.sizeAlign;
                        idxExtra = cfg.mem.sizePerBlock % cfg.mem.idxsAlign;
                        if(idxExtra > 0){
                            cfg.mem.sizePerBlock *= (cfg.mem.idxsAlign - idxExtra);
                        }
                        NBASSERT((cfg.mem.sizePerBlock % cfg.mem.idxsAlign) == 0)
                        NBASSERT((cfg.mem.sizePerBlock % cfg.mem.sizeAlign) == 0)
                    }
                    STNBScnRenderBuff b = STNBScnRenderBuff_Zero;
                    b.buff = NBGpuBuffer_alloc(NULL);
                    if(!NBGpuBuffer_prepare(b.buff, &cfg, &opq->api.itf.buff, opq->api.itfParam)){
                        //error
                        NBASSERT(FALSE)
                        r = FALSE;
                    } else {
                        b.uid = ++opq->buffs.iSeq;
                        NBArraySorted_addValue(&opq->buffs.arr, b);
                    }
                }
            }
        }
        //initial vertexBuffers
        {
            SI32 i; for(i = 0; i < ENNBScnVertexType_Count; i++){
                STNBGpuVertexBufferCfg cfg = STNBGpuVertexBufferCfg_Zero;
                STNBGpuBufferRef vertexBuff = NBArraySorted_itmPtrAtIndex(&opq->buffs.arr, STNBScnRenderBuff, i)->buff;
                STNBGpuBufferRef idxsBuff = NBArraySorted_itmPtrAtIndex(&opq->buffs.arr, STNBScnRenderBuff, ENNBScnVertexType_Count)->buff;
                //size
                switch(i){
                    case ENNBScnVertexType_Tex3: cfg.szPerRecord = sizeof(STNBScnVertexTex3); break;
                    case ENNBScnVertexType_Tex2: cfg.szPerRecord = sizeof(STNBScnVertexTex2); break;
                    case ENNBScnVertexType_Tex: cfg.szPerRecord = sizeof(STNBScnVertexTex); break;
                    case ENNBScnVertexType_Color: cfg.szPerRecord = sizeof(STNBScnVertex); break;
                    default: NBASSERT(FALSE) break;
                }
                //elems
                switch(i){
                    case ENNBScnVertexType_Tex3:
                        cfg.texCoords[2].amm    = 2;
                        cfg.texCoords[2].type   = ENNBGpuDataType_FLOAT32;
                        cfg.texCoords[2].offset = NBScnVertexTex3_IDX_tex3_x;
                    case ENNBScnVertexType_Tex2:
                        cfg.texCoords[1].amm    = 2;
                        cfg.texCoords[1].type   = ENNBGpuDataType_FLOAT32;
                        cfg.texCoords[1].offset = NBScnVertexTex2_IDX_tex2_x;
                    case ENNBScnVertexType_Tex:
                        cfg.texCoords[0].amm    = 2;
                        cfg.texCoords[0].type   = ENNBGpuDataType_FLOAT32;
                        cfg.texCoords[0].offset = NBScnVertexTex_IDX_tex_x;
                    default:
                        //color
                        cfg.color.amm           = 4;
                        cfg.color.type          = ENNBGpuDataType_UI8;
                        cfg.color.offset        = NBScnVertex_IDX_color;
                        //coord
                        cfg.coord.amm           = 2;
                        cfg.coord.type          = ENNBGpuDataType_FLOAT32;
                        cfg.coord.offset        = NBScnVertex_IDX_x;
                        //indices
                        if(NBGpuBuffer_isSet(idxsBuff)){
                            cfg.indices.amm     = 1;
                            cfg.indices.type    = ENNBGpuDataType_UI32;
                            cfg.indices.offset  = 0;
                        }
                        break;
                }
                {
                    STNBScnRenderBuff b = STNBScnRenderBuff_Zero;
                    b.buff = NBGpuVertexBuffer_alloc(NULL);
                    if(!NBGpuVertexBuffer_prepare(b.buff, &cfg, vertexBuff, idxsBuff, &opq->api.itf.vertexBuff, opq->api.itfParam)){
                        //error
                        NBASSERT(FALSE)
                        r = FALSE;
                    } else {
                        b.uid = ++opq->vertexBuffs.iSeq;
                        NBArraySorted_addValue(&opq->vertexBuffs.arr, b);
                    }
                }
            }
        }
    }
    NBObject_unlock(opq);
    return r;
}

//Vertices

STNBScnVertexIdxPtr NBScnRender_vIdxsAlloc(STNBScnRenderRef ref, const UI32 amm){
    STNBScnVertexIdxPtr r = STNBScnVertexIdxPtr_Zero;
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    if(ENNBScnVertexType_Count < opq->buffs.arr.use){
        STNBScnRenderBuff* b = NBArraySorted_itmPtrAtIndex(&opq->buffs.arr, STNBScnRenderBuff, ENNBScnVertexType_Count);
        STNBAbsPtr ptr = NBGpuBuffer_malloc(b->buff, sizeof(r.ptr[0]) * amm);
        r.idx = ptr.idx;
        r.ptr = (STNBScnVertexIdx*)ptr.ptr;
    }
    NBObject_unlock(opq);
    return r;
}

STNBScnVertexPtr NBScnRender_vertsAlloc(STNBScnRenderRef ref, const UI32 amm){
    STNBScnVertexPtr r = STNBScnVertexIdxPtr_Zero;
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    if(ENNBScnVertexType_Color < opq->buffs.arr.use){
        STNBScnRenderBuff* b = NBArraySorted_itmPtrAtIndex(&opq->buffs.arr, STNBScnRenderBuff, ENNBScnVertexType_Color);
        STNBAbsPtr ptr = NBGpuBuffer_malloc(b->buff, sizeof(r.ptr[0]) * amm);
        r.idx = ptr.idx;
        r.ptr = (STNBScnVertex*)ptr.ptr;
    }
    NBObject_unlock(opq);
    return r;
}

STNBScnVertexTexPtr NBScnRender_vertsTexAlloc(STNBScnRenderRef ref, const UI32 amm){
    STNBScnVertexTexPtr r = STNBScnVertexIdxPtr_Zero;
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    if(ENNBScnVertexType_Tex < opq->buffs.arr.use){
        STNBScnRenderBuff* b = NBArraySorted_itmPtrAtIndex(&opq->buffs.arr, STNBScnRenderBuff, ENNBScnVertexType_Tex);
        STNBAbsPtr ptr = NBGpuBuffer_malloc(b->buff, sizeof(r.ptr[0]) * amm);
        r.idx = ptr.idx;
        r.ptr = (STNBScnVertexTex*)ptr.ptr;
    }
    NBObject_unlock(opq);
    return r;
}

STNBScnVertexTex2Ptr NBScnRender_vertsTex2Alloc(STNBScnRenderRef ref, const UI32 amm){
    STNBScnVertexTex2Ptr r = STNBScnVertexIdxPtr_Zero;
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    if(ENNBScnVertexType_Tex2 < opq->buffs.arr.use){
        STNBScnRenderBuff* b = NBArraySorted_itmPtrAtIndex(&opq->buffs.arr, STNBScnRenderBuff, ENNBScnVertexType_Tex2);
        STNBAbsPtr ptr = NBGpuBuffer_malloc(b->buff, sizeof(r.ptr[0]) * amm);
        r.idx = ptr.idx;
        r.ptr = (STNBScnVertexTex2*)ptr.ptr;
    }
    NBObject_unlock(opq);
    return r;
}

STNBScnVertexTex3Ptr NBScnRender_vertsTex3Alloc(STNBScnRenderRef ref, const UI32 amm){
    STNBScnVertexTex3Ptr r = STNBScnVertexIdxPtr_Zero;
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    if(ENNBScnVertexType_Tex3 < opq->buffs.arr.use){
        STNBScnRenderBuff* b = NBArraySorted_itmPtrAtIndex(&opq->buffs.arr, STNBScnRenderBuff, ENNBScnVertexType_Tex3);
        STNBAbsPtr ptr = NBGpuBuffer_malloc(b->buff, sizeof(r.ptr[0]) * amm);
        r.idx = ptr.idx;
        r.ptr = (STNBScnVertexTex3*)ptr.ptr;
    }
    NBObject_unlock(opq);
    return r;
}

//

BOOL NBScnRender_vIdxsInvalidate(STNBScnRenderRef ref, const STNBScnVertexIdxPtr ptr, const UI32 sz){
    BOOL r = FALSE;
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    if(ENNBScnVertexType_Count < opq->buffs.arr.use){
        STNBScnRenderBuff* b = NBArraySorted_itmPtrAtIndex(&opq->buffs.arr, STNBScnRenderBuff, ENNBScnVertexType_Count);
        STNBAbsPtr ptr2;
        ptr2.idx = ptr.idx;
        ptr2.ptr = ptr.ptr;
        r = NBGpuBuffer_mInvalidate(b->buff, ptr2, sizeof(ptr.ptr[0]) * sz );
    }
    NBObject_unlock(opq);
    return r;
}

BOOL NBScnRender_vertsInvalidate(STNBScnRenderRef ref, const STNBScnVertexPtr ptr, const UI32 sz){
    BOOL r = FALSE;
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    if(ENNBScnVertexType_Color < opq->buffs.arr.use){
        STNBScnRenderBuff* b = NBArraySorted_itmPtrAtIndex(&opq->buffs.arr, STNBScnRenderBuff, ENNBScnVertexType_Color);
        STNBAbsPtr ptr2;
        ptr2.idx = ptr.idx;
        ptr2.ptr = ptr.ptr;
        r = NBGpuBuffer_mInvalidate(b->buff, ptr2, sizeof(ptr.ptr[0]) * sz );
    }
    NBObject_unlock(opq);
    return r;
}

BOOL NBScnRender_vertsTexInvalidate(STNBScnRenderRef ref, const STNBScnVertexTexPtr ptr, const UI32 sz){
    BOOL r = FALSE;
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    if(ENNBScnVertexType_Tex < opq->buffs.arr.use){
        STNBScnRenderBuff* b = NBArraySorted_itmPtrAtIndex(&opq->buffs.arr, STNBScnRenderBuff, ENNBScnVertexType_Tex);
        STNBAbsPtr ptr2;
        ptr2.idx = ptr.idx;
        ptr2.ptr = ptr.ptr;
        r = NBGpuBuffer_mInvalidate(b->buff, ptr2, sizeof(ptr.ptr[0]) * sz );
    }
    NBObject_unlock(opq);
    return r;
}

BOOL NBScnRender_vertsTex2Invalidate(STNBScnRenderRef ref, const STNBScnVertexTex2Ptr ptr, const UI32 sz){
    BOOL r = FALSE;
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    if(ENNBScnVertexType_Tex2 < opq->buffs.arr.use){
        STNBScnRenderBuff* b = NBArraySorted_itmPtrAtIndex(&opq->buffs.arr, STNBScnRenderBuff, ENNBScnVertexType_Tex2);
        STNBAbsPtr ptr2;
        ptr2.idx = ptr.idx;
        ptr2.ptr = ptr.ptr;
        r = NBGpuBuffer_mInvalidate(b->buff, ptr2, sizeof(ptr.ptr[0]) * sz );
    }
    NBObject_unlock(opq);
    return r;
}

BOOL NBScnRender_vertsTex3Invalidate(STNBScnRenderRef ref, const STNBScnVertexTex3Ptr ptr, const UI32 sz){
    BOOL r = FALSE;
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    if(ENNBScnVertexType_Tex3 < opq->buffs.arr.use){
        STNBScnRenderBuff* b = NBArraySorted_itmPtrAtIndex(&opq->buffs.arr, STNBScnRenderBuff, ENNBScnVertexType_Tex3);
        STNBAbsPtr ptr2;
        ptr2.idx = ptr.idx;
        ptr2.ptr = ptr.ptr;
        r = NBGpuBuffer_mInvalidate(b->buff, ptr2, sizeof(ptr.ptr[0]) * sz );
    }
    NBObject_unlock(opq);
    return r;
}

//

BOOL NBScnRender_vIdxsFree(STNBScnRenderRef ref, const STNBScnVertexIdxPtr ptr){
    BOOL r = FALSE;
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    if(ENNBScnVertexType_Count < opq->buffs.arr.use){
        STNBScnRenderBuff* b = NBArraySorted_itmPtrAtIndex(&opq->buffs.arr, STNBScnRenderBuff, ENNBScnVertexType_Count);
        STNBAbsPtr ptr2;
        ptr2.idx = ptr.idx;
        ptr2.ptr = ptr.ptr;
        r = NBGpuBuffer_mfree(b->buff, ptr2);
    }
    NBObject_unlock(opq);
    return r;
}

BOOL NBScnRender_vertsFree(STNBScnRenderRef ref, const STNBScnVertexPtr ptr){
    BOOL r = FALSE;
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    if(ENNBScnVertexType_Color < opq->buffs.arr.use){
        STNBScnRenderBuff* b = NBArraySorted_itmPtrAtIndex(&opq->buffs.arr, STNBScnRenderBuff, ENNBScnVertexType_Color);
        STNBAbsPtr ptr2;
        ptr2.idx = ptr.idx;
        ptr2.ptr = ptr.ptr;
        r = NBGpuBuffer_mfree(b->buff, ptr2);
    }
    NBObject_unlock(opq);
    return r;
}

BOOL NBScnRender_vertsTexFree(STNBScnRenderRef ref, const STNBScnVertexTexPtr ptr){
    BOOL r = FALSE;
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    if(ENNBScnVertexType_Tex < opq->buffs.arr.use){
        STNBScnRenderBuff* b = NBArraySorted_itmPtrAtIndex(&opq->buffs.arr, STNBScnRenderBuff, ENNBScnVertexType_Tex);
        STNBAbsPtr ptr2;
        ptr2.idx = ptr.idx;
        ptr2.ptr = ptr.ptr;
        r = NBGpuBuffer_mfree(b->buff, ptr2);
    }
    NBObject_unlock(opq);
    return r;
}

BOOL NBScnRender_vertsTex2Free(STNBScnRenderRef ref, const STNBScnVertexTex2Ptr ptr){
    BOOL r = FALSE;
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    if(ENNBScnVertexType_Tex2 < opq->buffs.arr.use){
        STNBScnRenderBuff* b = NBArraySorted_itmPtrAtIndex(&opq->buffs.arr, STNBScnRenderBuff, ENNBScnVertexType_Tex2);
        STNBAbsPtr ptr2;
        ptr2.idx = ptr.idx;
        ptr2.ptr = ptr.ptr;
        r = NBGpuBuffer_mfree(b->buff, ptr2);
    }
    NBObject_unlock(opq);
    return r;
}

BOOL NBScnRender_vertsTex3Free(STNBScnRenderRef ref, const STNBScnVertexTex3Ptr ptr){
    BOOL r = FALSE;
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    if(ENNBScnVertexType_Tex3 < opq->buffs.arr.use){
        STNBScnRenderBuff* b = NBArraySorted_itmPtrAtIndex(&opq->buffs.arr, STNBScnRenderBuff, ENNBScnVertexType_Tex3);
        STNBAbsPtr ptr2;
        ptr2.idx = ptr.idx;
        ptr2.ptr = ptr.ptr;
        r = NBGpuBuffer_mfree(b->buff, ptr2);
    }
    NBObject_unlock(opq);
    return r;
}

//Buffers

UI32 NBScnRender_bufferCreate(STNBScnRenderRef ref, const STNBGpuBufferCfg* cfg){ //allocates a new buffer
    UI32 r = 0;
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    if(cfg != NULL){
        STNBScnRenderBuff b = STNBScnRenderBuff_Zero;
        b.buff = NBGpuBuffer_alloc(NULL);
        if(!NBGpuBuffer_prepare(b.buff, cfg, NULL, NULL)){
            //error
            NBASSERT(FALSE)
        } else {
            b.uid = ++opq->buffs.iSeq;
            NBArraySorted_addValue(&opq->buffs.arr, b);
        }
    }
    NBObject_unlock(opq);
    return r;
}

BOOL NBScnRender_bufferDestroy(STNBScnRenderRef ref, const UI32 bid){ //flags a buffer for destruction
    BOOL r = FALSE;
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    {
        SI32 iFnd;
        STNBScnRenderBuff srch = STNBScnRenderBuff_Zero;
        srch.uid = bid;
        iFnd = NBArraySorted_indexOf(&opq->buffs.arr, &srch, sizeof(srch), NULL);
        if(iFnd > 0){
            STNBScnRenderBuff* b = NBArraySorted_itmPtrAtIndex(&opq->buffs.arr, STNBScnRenderBuff, iFnd);
            NBScnRenderBuff_release(b);
        }
    }
    NBObject_unlock(opq);
    return r;
}

//job
    
void NBScnRender_jobStart(STNBScnRenderRef ref){
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    {
        opq->job.nodes.iDepth = 0;
        opq->job.nodes.ammOpen = 0;
        opq->job.nodes.ammPoppedAtEnd = 0;
        NBArray_empty(&opq->job.nodes.arr);
        NBArray_empty(&opq->job.cmds);
    }
    NBObject_unlock(opq);
}
    
BOOL NBScnRender_jobEnd(STNBScnRenderRef ref){
    BOOL r = FALSE;
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    {
        //implicitly close remaining open nodes
        if(opq->job.nodes.arr.use > opq->job.nodes.ammPoppedAtEnd){
            STNBScnRenderNode* nFist = NBArray_dataPtr(&opq->job.nodes.arr, STNBScnRenderNode);
            STNBScnRenderNode* n = nFist + opq->job.nodes.arr.use - 1 - opq->job.nodes.ammPoppedAtEnd;
            const STNBScnRenderNode* nAfterLast = nFist + opq->job.nodes.arr.use;
            while(opq->job.nodes.ammOpen > 0 && n >= nFist) {
                if(!n->isPopped){
                    NBASSERT(n->cmds.start <= opq->job.cmds.use)
                    n->isPopped     = TRUE;
                    n->underCount   = (UI32)(nAfterLast - n);
                    n->cmds.size    = (UI32)(opq->job.cmds.use - n->cmds.start);
                    //
                    --opq->job.nodes.ammOpen;
                }
                ++opq->job.nodes.ammPoppedAtEnd;
                --n;
            }
        }
        NBASSERT(opq->job.nodes.ammOpen == 0)
        NBASSERT(opq->job.nodes.ammPoppedAtEnd == opq->job.nodes.arr.use);
        r = (opq->job.nodes.ammOpen == 0 && opq->job.nodes.ammPoppedAtEnd == opq->job.nodes.arr.use);
    }
    NBObject_unlock(opq);
    return r;
}

//job nodes

void NBScnRender_nodePush(STNBScnRenderRef ref, const STNBScnTransform* tform){
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    if(tform != NULL){
        NBObject_lock(opq);
        {
            STNBScnRenderNode n = STNBScnRenderNode_Zero;
            n.cmds.start    = opq->job.cmds.use;
            n.iDepth        = opq->job.nodes.iDepth++;
            n.tform         = *tform;
            NBArray_addValue(&opq->job.nodes.arr, n);
            //
            ++opq->job.nodes.ammOpen;
            opq->job.nodes.ammPoppedAtEnd = 0;
        }
        NBObject_unlock(opq);
    }
}
    
BOOL NBScnRender_nodePop(STNBScnRenderRef ref){
    BOOL r = FALSE;
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    NBASSERT(opq->job.nodes.ammOpen > 0 && opq->job.nodes.arr.use > opq->job.nodes.ammPoppedAtEnd) //popping beyond limit
    if(opq->job.nodes.ammOpen > 0 && opq->job.nodes.arr.use > opq->job.nodes.ammPoppedAtEnd){
        STNBScnRenderNode* nFist = NBArray_dataPtr(&opq->job.nodes.arr, STNBScnRenderNode);
        STNBScnRenderNode* n = nFist + opq->job.nodes.arr.use - 1 - opq->job.nodes.ammPoppedAtEnd;
        const STNBScnRenderNode* nAfterLast = nFist + opq->job.nodes.arr.use;
        while(opq->job.nodes.ammOpen > 0 && n >= nFist) {
            ++opq->job.nodes.ammPoppedAtEnd;
            if(!n->isPopped){
                NBASSERT(n->cmds.start <= opq->job.cmds.use)
                n->isPopped     = TRUE;
                n->underCount   = (UI32)(nAfterLast - n);
                n->cmds.size    = (UI32)(opq->job.cmds.use - n->cmds.start);
                //
                --opq->job.nodes.ammOpen;
                r = TRUE;
                break;
            }
            --n;
        }
    }
    NBObject_unlock(opq);
    return r;
}

//job cmds

void NBScnRender_cmdMaskModePush(STNBScnRenderRef ref){
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    {
        STNBScnRenderCmd cmd;
        cmd.cmdId = ENNBScnRenderCmd_MaskModePush;
        NBArray_addValue(&opq->job.cmds, cmd);
    }
    NBObject_unlock(opq);
}

void NBScnRender_cmdMaskModePop(STNBScnRenderRef ref){
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    {
        STNBScnRenderCmd cmd;
        cmd.cmdId = ENNBScnRenderCmd_MaskModePop;
        NBArray_addValue(&opq->job.cmds, cmd);
    }
    NBObject_unlock(opq);
}

void NBScnRender_cmdSetTexture(STNBScnRenderRef ref, const UI32 index, const UI32 tex /*const STNBGpuTextureRef tex*/){
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    {
        STNBScnRenderCmd cmd;
        cmd.cmdId = ENNBScnRenderCmd_SetTexture;
        cmd.setTexture.index = index;
        cmd.setTexture.tex = tex;
        NBArray_addValue(&opq->job.cmds, cmd);
    }
    NBObject_unlock(opq);
}

void NBScnRender_cmdSetVertsType(STNBScnRenderRef ref, const ENNBScnVertexType type){
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    {
        STNBScnRenderCmd cmd;
        cmd.cmdId = ENNBScnRenderCmd_SetVertsType;
        cmd.setVertsType.type = type;
        NBArray_addValue(&opq->job.cmds, cmd);
    }
    NBObject_unlock(opq);
}

void NBScnRender_cmdDawVerts(STNBScnRenderRef ref, const ENNBScnRenderShape mode, const UI32 iFirst, const UI32 count){
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    {
        STNBScnRenderCmd cmd;
        cmd.cmdId = ENNBScnRenderCmd_DrawVerts;
        cmd.drawVerts.mode      = mode;
        cmd.drawVerts.iFirst    = iFirst;
        cmd.drawVerts.count     = count;
        NBArray_addValue(&opq->job.cmds, cmd);
    }
    NBObject_unlock(opq);
}

void NBScnRender_cmdDawIndexes(STNBScnRenderRef ref, const ENNBScnRenderShape mode, const UI32 iFirst, const UI32 count){
    STNBScnRenderOpq* opq = (STNBScnRenderOpq*)ref.opaque;
    NBASSERT(NBScnRender_isClass(ref))
    NBObject_lock(opq);
    {
        STNBScnRenderCmd cmd;
        cmd.cmdId = ENNBScnRenderCmd_DrawIndexes;
        cmd.drawVerts.mode      = mode;
        cmd.drawVerts.iFirst    = iFirst;
        cmd.drawVerts.count     = count;
        NBArray_addValue(&opq->job.cmds, cmd);
    }
    NBObject_unlock(opq);
}

//STNBScnRenderBuff

void NBScnRenderBuff_release(STNBScnRenderBuff* obj){
    if(NBGpuBuffer_isSet(obj->buff)){
        NBGpuBuffer_release(&obj->buff);
        NBGpuBuffer_null(&obj->buff);
    }
}

//STNBScnRenderVertexBuff

void NBScnRenderVertexBuff_release(STNBScnRenderVertexBuff* obj){
    if(NBGpuVertexBuffer_isSet(obj->buff)){
        NBGpuVertexBuffer_release(&obj->buff);
        NBGpuVertexBuffer_null(&obj->buff);
    }
}
