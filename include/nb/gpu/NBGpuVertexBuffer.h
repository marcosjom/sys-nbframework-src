#ifndef NB_GPU_VERTEX_BUFFER_H
#define NB_GPU_VERTEX_BUFFER_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBMemoryBlocks.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBRange.h"
//
#include "nb/scene/NBScnVertices.h"
#include "nb/gpu/NBGpuDataType.h"
#include "nb/gpu/NBGpuTexture.h"
#include "nb/gpu/NBGpuBuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

//STNBGpuVertexPartDef

#define STNBGpuVertexPartDef_Zero    { 0, 0, 0 }

typedef struct STNBGpuVertexPartDef_ {
    UI8 amm;        //ammount of elements of this part (coords, color, textureCoords, ...)
    UI8 type;       //ENNBGpuDataType
    UI16 offset;    //offset inside a record to the first element
} STNBGpuVertexPartDef;

const STNBStructMap* NBGpuVertexPartDef_getSharedStructMap(void);

//STNBGpuVertexBufferCfg

#define STNBGpuVertexBufferCfg_Zero   { 0, STNBGpuVertexPartDef_Zero, STNBGpuVertexPartDef_Zero, STNBGpuVertexPartDef_Zero, { STNBGpuVertexPartDef_Zero, STNBGpuVertexPartDef_Zero, STNBGpuVertexPartDef_Zero } }

typedef struct STNBGpuVertexBufferCfg_ {
    UI32                    szPerRecord;    //bytes per record
    STNBGpuVertexPartDef    indices;
    STNBGpuVertexPartDef    coord;
    STNBGpuVertexPartDef    color;
    STNBGpuVertexPartDef    texCoords[ENNBGpuTextureIdx_Count];
} STNBGpuVertexBufferCfg;

const STNBStructMap* NBGpuVertexBufferCfg_getSharedStructMap(void);

//STNBGpuVertexBufferApiItf

typedef struct STNBGpuVertexBufferApiItf_ {
    void* (*create)(const STNBGpuVertexBufferCfg* cfg, STNBGpuBufferRef vertexBuff, STNBGpuBufferRef idxsBuff, void* usrData);
    void  (*destroy)(void* data, void* usrData);
    //
    BOOL  (*activate)(void* data, const STNBGpuVertexBufferCfg* cfg, void* usrData);
    BOOL  (*deactivate)(void* data, void* usrData);
} STNBGpuVertexBufferApiItf;

//

NB_OBJREF_HEADER(NBGpuVertexBuffer)

//

BOOL NBGpuVertexBuffer_prepare(STNBGpuVertexBufferRef ref, const STNBGpuVertexBufferCfg* cfg, STNBGpuBufferRef vertexBuff, STNBGpuBufferRef idxsBuff, const STNBGpuVertexBufferApiItf* itf, void* itfParam);

BOOL NBGpuVertexBuffer_activate(STNBGpuVertexBufferRef ref);
BOOL NBGpuVertexBuffer_deactivate(STNBGpuVertexBufferRef ref);

UI32 NBGpuVertexBuffer_getSzPerRecord(STNBGpuVertexBufferRef ref);
STNBGpuBufferRef NBGpuVertexBuffer_getVertexBuff(STNBGpuVertexBufferRef ref);
STNBGpuBufferRef NBGpuVertexBuffer_getIdxsBuff(STNBGpuVertexBufferRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
