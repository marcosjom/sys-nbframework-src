#ifndef NB_SCN_RENDERER_H
#define NB_SCN_RENDERER_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
//
#include "nb/core/NBStructMap.h"
#include "nb/core/NBRange.h"
//
#include "nb/gpu/NBGpuBuffer.h"
#include "nb/gpu/NBGpuVertexBuffer.h"
#include "nb/gpu/NBGpuTexture.h"
#include "nb/gpu/NBGpuRenderbuffer.h"
#include "nb/gpu/NBGpuFramebuffer.h"
//
#include "nb/scene/NBScnVertices.h"
#include "nb/scene/NBScnTransform.h"
#include "nb/scene/NBScnRenderCmd.h"
#include "nb/scene/NBScnVertexBuffs.h"

#ifdef __cplusplus
extern "C" {
#endif

//NBScnRenderNode

#define STNBScnRenderNode_Zero  { 0, FALSE, 0, STNBScnTransform_Zero, STNBRangeI_Zero }

typedef struct STNBScnRenderNode_ {
    UI16                iDepth;     //level in the tree
    BOOL                isPopped;   //node is still open (pushed but not popped yet)
    UI32                underCount; //ammount of nodes affected by this (children and below)
    STNBScnTransform    tform;      //transform properties
    STNBRangeI          cmds;       //range of commands
} STNBScnRenderNode;

//NBScnRenderApiItf

typedef struct STNBScnRenderApiItf_ {
    STNBGpuBufferApiItf         buff;   //buffers
    STNBGpuVertexBufferApiItf   vertexBuff;   //vertexBuff
    STNBGpuTextureApiItf        tex;    //textures
    STNBGpuRenderbufferApiItf   rbuff;  //render buffers
    STNBGpuFramebufferApiItf    fbuff;  //framebuffers
} STNBScnRenderApiItf;

//

NB_OBJREF_HEADER(NBScnRender)

//Prepare

BOOL NBScnRender_prepare(STNBScnRenderRef ref, const STNBScnRenderApiItf* itf, void* itfParam);

//Vertices

STNBScnVertexBuffsRef NBScnRender_getDefaultVertexBuffs(STNBScnRenderRef ref);
BOOL NBScnRender_createVertexBuffs(STNBScnRenderRef ref, STNBScnVertexBuffsRef* dst);

//Buffers

UI32 NBScnRender_bufferCreate(STNBScnRenderRef ref, const STNBGpuBufferCfg* cfg);     //allocates a new buffer
BOOL NBScnRender_bufferDestroy(STNBScnRenderRef ref, const UI32 bid);                 //flags a buffer for destruction

//job

void NBScnRender_jobStart(STNBScnRenderRef ref);
BOOL NBScnRender_jobEnd(STNBScnRenderRef ref);

//job nodes

void NBScnRender_nodePush(STNBScnRenderRef ref, const STNBScnTransform* tform);
BOOL NBScnRender_nodePop(STNBScnRenderRef ref);

//job cmds

void NBScnRender_cmdMaskModePush(STNBScnRenderRef ref);
void NBScnRender_cmdMaskModePop(STNBScnRenderRef ref);
void NBScnRender_cmdSetTexture(STNBScnRenderRef ref, const UI32 index, const UI32 tex /*const STNBGpuTextureRef tex*/);
void NBScnRender_cmdSetVertsType(STNBScnRenderRef ref, const ENNBScnVertexType type);
void NBScnRender_cmdDawVerts(STNBScnRenderRef ref, const ENNBScnRenderShape mode, const UI32 iFirst, const UI32 count);
void NBScnRender_cmdDawIndexes(STNBScnRenderRef ref, const ENNBScnRenderShape mode, const UI32 iFirst, const UI32 count);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
