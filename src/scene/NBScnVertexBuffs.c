
#include "nb/NBFrameworkPch.h"
#include "nb/scene/NBScnVertexBuffs.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBStructMaps.h"
#include "nb/core/NBMngrStructMaps.h"

//STNBScnVertexBuffsOpq

typedef struct STNBScnVertexBuffsOpq_ {
    STNBObject  prnt;
    STNBGpuVertexBufferRef    vBuffs[ENNBScnVertexType_Count];
} STNBScnVertexBuffsOpq;

NB_OBJREF_BODY(NBScnVertexBuffs, STNBScnVertexBuffsOpq, NBObject)

void NBScnVertexBuffs_initZeroed(STNBObject* obj) {
    //STNBScnVertexBuffsOpq* opq = (STNBScnVertexBuffsOpq*)obj;
}

void NBScnVertexBuffs_uninitLocked(STNBObject* obj){
    STNBScnVertexBuffsOpq* opq = (STNBScnVertexBuffsOpq*)obj;
    //vBuffs
    {
        STNBGpuVertexBufferRef* b = opq->vBuffs;
        const STNBGpuVertexBufferRef* bAfterEnd = b + ENNBScnVertexType_Count;
        while(b < bAfterEnd){
            if(NBGpuVertexBuffer_isSet(*b)){
                NBGpuVertexBuffer_release(b);
                NBGpuVertexBuffer_null(b);
            }
            ++b;
        }
    }
}

//prepare

BOOL NBScnVertexBuffs_prepare(STNBScnVertexBuffsRef ref, const STNBGpuVertexBufferRef* vBuffs, const UI32 vBuffsSz){
    BOOL r = FALSE;
    STNBScnVertexBuffsOpq* opq = (STNBScnVertexBuffsOpq*)ref.opaque;
    NBASSERT(NBScnVertexBuffs_isClass(ref))
    NBObject_lock(opq);
    {
        r = TRUE;
        //validate
        {
            SI32 i; for(i = 0; r && i < vBuffsSz && i < ENNBScnVertexType_Count; i++){
                if(NBGpuVertexBuffer_isSet(vBuffs[i])){
                    const UI32 szPerRecord = NBGpuVertexBuffer_getSzPerRecord(vBuffs[i]);
                    UI32 szPerRecordReq = 0;
                    switch (i) {
                        case ENNBScnVertexType_Color:   szPerRecordReq = sizeof(STNBScnVertex); break;
                        case ENNBScnVertexType_Tex:     szPerRecordReq = sizeof(STNBScnVertexTex); break;
                        case ENNBScnVertexType_Tex2:    szPerRecordReq = sizeof(STNBScnVertexTex2); break;
                        case ENNBScnVertexType_Tex3:    szPerRecordReq = sizeof(STNBScnVertexTex3); break;
                        default: r = FALSE; NBASSERT(FALSE) break;
                    }
                    if(szPerRecord != szPerRecordReq){
                        r = FALSE;
                        break;
                    }
                }
            }
        }
        //apply
        if(r){
            //release
            {
                STNBGpuVertexBufferRef* b = opq->vBuffs;
                const STNBGpuVertexBufferRef* bAfterEnd = b + ENNBScnVertexType_Count;
                while(b < bAfterEnd){
                    if(NBGpuVertexBuffer_isSet(*b)){
                        NBGpuVertexBuffer_release(b);
                        NBGpuVertexBuffer_null(b);
                    }
                    ++b;
                }
            }
            //set
            {
                SI32 i; for(i = 0; i < vBuffsSz && i < ENNBScnVertexType_Count; i++){
                    NBGpuVertexBuffer_set(&opq->vBuffs[i], &vBuffs[i]);
                }
            }
        }
    }
    NBObject_unlock(opq);
    return r;
}

#define NBScnVertexBuffs_vNAlloc(V_IDX, PT_CAST_TYPE, GET_BUFF_METHOD)  \
    STNBScnVertexBuffsOpq* opq = (STNBScnVertexBuffsOpq*)ref.opaque; \
    NBASSERT(NBScnVertexBuffs_isClass(ref)) \
    NBObject_lock(opq); \
    { \
        STNBGpuVertexBufferRef vb = opq->vBuffs[V_IDX]; \
        if(NBGpuVertexBuffer_isSet(vb)){ \
            const UI32 szPerRecord = NBGpuVertexBuffer_getSzPerRecord(vb); \
            STNBGpuBufferRef b = GET_BUFF_METHOD(vb); \
            if(NBGpuBuffer_isSet(b)){ \
                STNBAbsPtr r2 = NBGpuBuffer_malloc(b, amm * szPerRecord); \
                r.idx = r2.idx; \
                r.ptr = (PT_CAST_TYPE*)r2.ptr; \
            } \
        } \
    } \
    NBObject_unlock(opq);

#define NBScnVertexBuffs_vNInvalidate(V_IDX, GET_BUFF_METHOD) \
    BOOL r = FALSE; \
    STNBScnVertexBuffsOpq* opq = (STNBScnVertexBuffsOpq*)ref.opaque; \
    NBASSERT(NBScnVertexBuffs_isClass(ref)) \
    NBObject_lock(opq); \
    { \
        STNBGpuVertexBufferRef vb = opq->vBuffs[V_IDX]; \
        if(NBGpuVertexBuffer_isSet(vb)){ \
            const UI32 szPerRecord = NBGpuVertexBuffer_getSzPerRecord(vb); \
            STNBGpuBufferRef b = GET_BUFF_METHOD(vb); \
            if(NBGpuBuffer_isSet(b)){ \
                STNBAbsPtr ptr2 = STNBAbsPtr_Zero; \
                ptr2.idx = ptr.idx; \
                ptr2.ptr = ptr.ptr; \
                r = NBGpuBuffer_mInvalidate(b, ptr2, sz * szPerRecord);\
            } \
        } \
    } \
    NBObject_unlock(opq); \
    return r;

#define NBScnVertexBuffs_vNFree(V_IDX, GET_BUFF_METHOD) \
    BOOL r = FALSE; \
    STNBScnVertexBuffsOpq* opq = (STNBScnVertexBuffsOpq*)ref.opaque; \
    NBASSERT(NBScnVertexBuffs_isClass(ref)) \
    NBObject_lock(opq); \
    { \
        STNBGpuVertexBufferRef vb = opq->vBuffs[V_IDX]; \
        if(NBGpuVertexBuffer_isSet(vb)){ \
            STNBGpuBufferRef b = GET_BUFF_METHOD(vb); \
            if(NBGpuBuffer_isSet(b)){ \
                STNBAbsPtr ptr2 = STNBAbsPtr_Zero; \
                ptr2.idx = ptr.idx; \
                ptr2.ptr = ptr.ptr; \
                r = NBGpuBuffer_mfree(b, ptr2); \
            } \
        } \
    } \
    NBObject_unlock(opq); \
    return r;

//ENNBScnVertexType_Color //no texture

STNBScnVertexPtr NBScnVertexBuffs_v0Alloc(STNBScnVertexBuffsRef ref, const UI32 amm){
    STNBScnVertexPtr r = STNBScnVertexPtr_Zero;
    NBScnVertexBuffs_vNAlloc(ENNBScnVertexType_Color, STNBScnVertex, NBGpuVertexBuffer_getVertexBuff)
    return r;
}

BOOL NBScnVertexBuffs_v0Invalidate(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr, const UI32 sz){
    NBScnVertexBuffs_vNInvalidate(ENNBScnVertexType_Color, NBGpuVertexBuffer_getVertexBuff)
}

BOOL NBScnVertexBuffs_v0Free(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr){
    NBScnVertexBuffs_vNFree(ENNBScnVertexType_Color, NBGpuVertexBuffer_getVertexBuff);
}

//

STNBScnVertexIdxPtr NBScnVertexBuffs_v0IdxsAlloc(STNBScnVertexBuffsRef ref, const UI32 amm){
    STNBScnVertexIdxPtr r = STNBScnVertexPtr_Zero;
    NBScnVertexBuffs_vNAlloc(ENNBScnVertexType_Color, STNBScnVertexIdx, NBGpuVertexBuffer_getIdxsBuff)
    return r;
}

BOOL NBScnVertexBuffs_v0IdxsInvalidate(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr, const UI32 sz){
    NBScnVertexBuffs_vNInvalidate(ENNBScnVertexType_Color, NBGpuVertexBuffer_getIdxsBuff)
}

BOOL NBScnVertexBuffs_v0IdxsFree(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr){
    NBScnVertexBuffs_vNFree(ENNBScnVertexType_Color, NBGpuVertexBuffer_getIdxsBuff);
}

//ENNBScnVertexType_Tex  //one texture

STNBScnVertexTexPtr NBScnVertexBuffs_v1Alloc(STNBScnVertexBuffsRef ref, const UI32 amm){
    STNBScnVertexTexPtr r = STNBScnVertexPtr_Zero;
    NBScnVertexBuffs_vNAlloc(ENNBScnVertexType_Tex, STNBScnVertexTex, NBGpuVertexBuffer_getVertexBuff)
    return r;
}

BOOL NBScnVertexBuffs_v1Invalidate(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr, const UI32 sz){
    NBScnVertexBuffs_vNInvalidate(ENNBScnVertexType_Tex, NBGpuVertexBuffer_getVertexBuff)
}

BOOL NBScnVertexBuffs_v1Free(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr){
    NBScnVertexBuffs_vNFree(ENNBScnVertexType_Tex, NBGpuVertexBuffer_getVertexBuff);
}

//

STNBScnVertexIdxPtr NBScnVertexBuffs_v1IdxsAlloc(STNBScnVertexBuffsRef ref, const UI32 amm){
    STNBScnVertexIdxPtr r = STNBScnVertexPtr_Zero;
    NBScnVertexBuffs_vNAlloc(ENNBScnVertexType_Tex, STNBScnVertexIdx, NBGpuVertexBuffer_getIdxsBuff)
    return r;
}

BOOL NBScnVertexBuffs_v1IdxsInvalidate(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr, const UI32 sz){
    NBScnVertexBuffs_vNInvalidate(ENNBScnVertexType_Tex, NBGpuVertexBuffer_getIdxsBuff)
}

BOOL NBScnVertexBuffs_v1IdxsFree(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr){
    NBScnVertexBuffs_vNFree(ENNBScnVertexType_Tex, NBGpuVertexBuffer_getIdxsBuff);
}

//ENNBScnVertexType_Tex2 //two textures

STNBScnVertexTex2Ptr NBScnVertexBuffs_v2Alloc(STNBScnVertexBuffsRef ref, const UI32 amm){
    STNBScnVertexTex2Ptr r = STNBScnVertexPtr_Zero;
    NBScnVertexBuffs_vNAlloc(ENNBScnVertexType_Tex2, STNBScnVertexTex2, NBGpuVertexBuffer_getVertexBuff)
    return r;
}

BOOL NBScnVertexBuffs_v2Invalidate(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr, const UI32 sz){
    NBScnVertexBuffs_vNInvalidate(ENNBScnVertexType_Tex2, NBGpuVertexBuffer_getVertexBuff)
}

BOOL NBScnVertexBuffs_v2Free(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr){
    NBScnVertexBuffs_vNFree(ENNBScnVertexType_Tex2, NBGpuVertexBuffer_getVertexBuff);
}

//

STNBScnVertexIdxPtr NBScnVertexBuffs_v2IdxsAlloc(STNBScnVertexBuffsRef ref, const UI32 amm){
    STNBScnVertexIdxPtr r = STNBScnVertexPtr_Zero;
    NBScnVertexBuffs_vNAlloc(ENNBScnVertexType_Tex2, STNBScnVertexIdx, NBGpuVertexBuffer_getIdxsBuff)
    return r;
}

BOOL NBScnVertexBuffs_v2IdxsInvalidate(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr, const UI32 sz){
    NBScnVertexBuffs_vNInvalidate(ENNBScnVertexType_Tex2, NBGpuVertexBuffer_getIdxsBuff)
}

BOOL NBScnVertexBuffs_v2IdxsFree(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr){
    NBScnVertexBuffs_vNFree(ENNBScnVertexType_Tex2, NBGpuVertexBuffer_getIdxsBuff);
}

//ENNBScnVertexType_Tex3 //three textures

STNBScnVertexTex3Ptr NBScnVertexBuffs_v3Alloc(STNBScnVertexBuffsRef ref, const UI32 amm){
    STNBScnVertexTex3Ptr r = STNBScnVertexPtr_Zero;
    NBScnVertexBuffs_vNAlloc(ENNBScnVertexType_Tex3, STNBScnVertexTex3, NBGpuVertexBuffer_getVertexBuff)
    return r;
}

BOOL NBScnVertexBuffs_v3Invalidate(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr, const UI32 sz){
    NBScnVertexBuffs_vNInvalidate(ENNBScnVertexType_Tex3, NBGpuVertexBuffer_getVertexBuff)
}

BOOL NBScnVertexBuffs_v3Free(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr){
    NBScnVertexBuffs_vNFree(ENNBScnVertexType_Tex3, NBGpuVertexBuffer_getVertexBuff);
}

//

STNBScnVertexIdxPtr NBScnVertexBuffs_v3IdxsAlloc(STNBScnVertexBuffsRef ref, const UI32 amm){
    STNBScnVertexIdxPtr r = STNBScnVertexPtr_Zero;
    NBScnVertexBuffs_vNAlloc(ENNBScnVertexType_Tex3, STNBScnVertexIdx, NBGpuVertexBuffer_getIdxsBuff)
    return r;
}

BOOL NBScnVertexBuffs_v3IdxsInvalidate(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr, const UI32 sz){
    NBScnVertexBuffs_vNInvalidate(ENNBScnVertexType_Tex3, NBGpuVertexBuffer_getIdxsBuff)
}

BOOL NBScnVertexBuffs_v3IdxsFree(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr){
    NBScnVertexBuffs_vNFree(ENNBScnVertexType_Tex3, NBGpuVertexBuffer_getIdxsBuff);
}

