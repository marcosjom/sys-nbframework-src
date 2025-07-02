#ifndef NB_GPU_BUFFER_H
#define NB_GPU_BUFFER_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBMemoryBlocks.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBRange.h"
//
#include "nb/scene/NBScnVertices.h"

#ifdef __cplusplus
extern "C" {
#endif

//ENNBScnBufferType

typedef enum ENNBScnBufferType_ {
    ENNBScnBufferType_Unknown = 0,
    //
    ENNBScnBufferType_Color,        //no texture
    ENNBScnBufferType_Tex,          //one texture
    ENNBScnBufferType_Tex2,         //two textures
    ENNBScnBufferType_Tex3,         //three textures
    ENNBScnBufferType_Index,        //index
    //
    ENNBScnBufferType_Count
} ENNBScnBufferType;

//STNBGpuBufferCfg

#define STNBGpuBufferCfg_Zero   { ENNBScnVertexType_Color, STNBMemoryBlocksCfg_Zero }

typedef struct STNBGpuBufferCfg_ {
    ENNBScnVertexType   type;
    STNBMemoryBlocksCfg mem;    //memory blocks cfg
} STNBGpuBufferCfg;

const STNBStructMap* NBGpuBufferCfg_getSharedStructMap(void);

//STNBGpuBufferChanges

typedef struct STNBGpuBufferChanges_ {
    BOOL            size;  //buffer's size changed
    STNBRangeU*     rngs;  //subimages areas
    UI32            rngsUse;
} STNBGpuBufferChanges;

const STNBStructMap* NBGpuBufferChanges_getSharedStructMap(void);

//STNBGpuBufferApiItf

typedef struct STNBGpuBufferApiItf_ {
    void* (*create)(const STNBGpuBufferCfg* cfg, void* usrData);
    void  (*destroy)(void* data, void* usrData);
    //
    BOOL  (*sync)(void* data, const STNBGpuBufferCfg* cfg, STNBMemoryBlocksRef mem, const STNBGpuBufferChanges* changes, void* usrData);
} STNBGpuBufferApiItf;

//

NB_OBJREF_HEADER(NBGpuBuffer)

//

BOOL NBGpuBuffer_prepare(STNBGpuBufferRef ref, const STNBGpuBufferCfg* cfg, const STNBGpuBufferApiItf* itf, void* itfParam);
BOOL NBGpuBuffer_clear(STNBGpuBufferRef ref);

STNBAbsPtr NBGpuBuffer_malloc(STNBGpuBufferRef ref, const UI32 usableSz);
BOOL NBGpuBuffer_mfree(STNBGpuBufferRef ref, const STNBAbsPtr ptr);
BOOL NBGpuBuffer_mInvalidate(STNBGpuBufferRef ref, const STNBAbsPtr ptr, const UI32 sz);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
