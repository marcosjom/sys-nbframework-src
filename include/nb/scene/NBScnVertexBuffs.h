#ifndef NB_SCN_VERTEX_BUFFS_H
#define NB_SCN_VERTEX_BUFFS_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
//
#include "nb/core/NBStructMap.h"
#include "nb/core/NBRange.h"
//
#include "nb/gpu/NBGpuBuffer.h"
#include "nb/gpu/NBGpuVertexBuffer.h"
//
#include "nb/scene/NBScnVertices.h"

#ifdef __cplusplus
extern "C" {
#endif

//STNBScnVertexIdxPtr, abstract pointer

#define STNBScnVertexIdxPtr_Zero { NULL, 0 }

typedef struct STNBScnVertexIdxPtr_ {
    STNBScnVertexIdx*   ptr;    //memory address, must be first element of struct to allow casting struct to a bare-pointer.
    UI32                idx;    //abstract address
} STNBScnVertexIdxPtr;

//STNBScnVertexPtr, abstract pointer

#define STNBScnVertexPtr_Zero { NULL, 0 }

typedef struct STNBScnVertexPtr_ {
    STNBScnVertex*  ptr;    //memory address, must be first element of struct to allow casting struct to a bare-pointer.
    UI32            idx;    //abstract address
} STNBScnVertexPtr;

//STNBScnVertexTexPtr, abstract pointer

#define STNBScnVertexTexPtr_Zero { NULL, 0 }

typedef struct STNBScnVertexTexPtr_ {
    STNBScnVertexTex*   ptr;    //memory address, must be first element of struct to allow casting struct to a bare-pointer.
    UI32                idx;    //abstract address
} STNBScnVertexTexPtr;

//STNBScnVertexTex2Ptr, abstract pointer

#define STNBScnVertexTex2Ptr_Zero { NULL, 0 }

typedef struct STNBScnVertexTex2Ptr_ {
    STNBScnVertexTex2*  ptr;    //memory address, must be first element of struct to allow casting struct to a bare-pointer.
    UI32                idx;    //abstract address
} STNBScnVertexTex2Ptr;

//STNBScnVertexTex3Ptr, abstract pointer

#define STNBScnVertexTex3Ptr_Zero { NULL, 0 }

typedef struct STNBScnVertexTex3Ptr_ {
    STNBScnVertexTex3*  ptr;    //memory address, must be first element of struct to allow casting struct to a bare-pointer.
    UI32                idx;    //abstract address
} STNBScnVertexTex3Ptr;

//

NB_OBJREF_HEADER(NBScnVertexBuffs)

//Prepare

BOOL NBScnVertexBuffs_prepare(STNBScnVertexBuffsRef ref, const STNBGpuVertexBufferRef* vBuffs, const UI32 vBuffsSz);

//ENNBScnVertexType_Color //no texture

STNBScnVertexPtr    NBScnVertexBuffs_v0Alloc(STNBScnVertexBuffsRef ref, const UI32 amm);
BOOL                NBScnVertexBuffs_v0Invalidate(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr, const UI32 sz);
BOOL                NBScnVertexBuffs_v0Free(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr);
//
STNBScnVertexIdxPtr NBScnVertexBuffs_v0IdxsAlloc(STNBScnVertexBuffsRef ref, const UI32 amm);
BOOL                NBScnVertexBuffs_v0IdxsInvalidate(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr, const UI32 sz);
BOOL                NBScnVertexBuffs_v0IdxsFree(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr);

//ENNBScnVertexType_Tex  //one texture

STNBScnVertexTexPtr NBScnVertexBuffs_v1Alloc(STNBScnVertexBuffsRef ref, const UI32 amm);
BOOL                NBScnVertexBuffs_v1Invalidate(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr, const UI32 sz);
BOOL                NBScnVertexBuffs_v1Free(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr);
//
STNBScnVertexIdxPtr NBScnVertexBuffs_v1IdxsAlloc(STNBScnVertexBuffsRef ref, const UI32 amm);
BOOL                NBScnVertexBuffs_v1IdxsInvalidate(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr, const UI32 sz);
BOOL                NBScnVertexBuffs_v1IdxsFree(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr);

//ENNBScnVertexType_Tex2 //two textures

STNBScnVertexTex2Ptr NBScnVertexBuffs_v2Alloc(STNBScnVertexBuffsRef ref, const UI32 amm);
BOOL                NBScnVertexBuffs_v2Invalidate(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr, const UI32 sz);
BOOL                NBScnVertexBuffs_v2Free(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr);
//
STNBScnVertexIdxPtr NBScnVertexBuffs_v2IdxsAlloc(STNBScnVertexBuffsRef ref, const UI32 amm);
BOOL                NBScnVertexBuffs_v2IdxsInvalidate(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr, const UI32 sz);
BOOL                NBScnVertexBuffs_v2IdxsFree(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr);

//ENNBScnVertexType_Tex3 //three textures

STNBScnVertexTex3Ptr NBScnVertexBuffs_v3Alloc(STNBScnVertexBuffsRef ref, const UI32 amm);
BOOL                NBScnVertexBuffs_v3Invalidate(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr, const UI32 sz);
BOOL                NBScnVertexBuffs_v3Free(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr);
//
STNBScnVertexIdxPtr NBScnVertexBuffs_v3IdxsAlloc(STNBScnVertexBuffsRef ref, const UI32 amm);
BOOL                NBScnVertexBuffs_v3IdxsInvalidate(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr, const UI32 sz);
BOOL                NBScnVertexBuffs_v3IdxsFree(STNBScnVertexBuffsRef ref, const STNBScnVertexPtr ptr);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
