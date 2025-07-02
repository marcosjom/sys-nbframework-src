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

//STNBScnVertexIdxPtr, abstract pointer

#define STNBScnVertexIdxPtr_Zero { NULL, 0 }

typedef struct STNBScnVertexIdxPtr_ {
    STNBScnVertexIdx*   ptr;    //memory address
    UI32                idx;    //abstract address
} STNBScnVertexIdxPtr;

//STNBScnVertexPtr, abstract pointer

#define STNBScnVertexPtr_Zero { NULL, 0 }

typedef struct STNBScnVertexPtr_ {
    STNBScnVertex*  ptr;    //memory address
    UI32            idx;    //abstract address
} STNBScnVertexPtr;

//STNBScnVertexTexPtr, abstract pointer

#define STNBScnVertexTexPtr_Zero { NULL, 0 }

typedef struct STNBScnVertexTexPtr_ {
    STNBScnVertexTex*   ptr;    //memory address
    UI32                idx;    //abstract address
} STNBScnVertexTexPtr;

//STNBScnVertexTex2Ptr, abstract pointer

#define STNBScnVertexTex2Ptr_Zero { NULL, 0 }

typedef struct STNBScnVertexTex2Ptr_ {
    STNBScnVertexTex2*  ptr;    //memory address
    UI32                idx;    //abstract address
} STNBScnVertexTex2Ptr;

//STNBScnVertexTex3Ptr, abstract pointer

#define STNBScnVertexTex3Ptr_Zero { NULL, 0 }

typedef struct STNBScnVertexTex3Ptr_ {
    STNBScnVertexTex3*  ptr;    //memory address
    UI32                idx;    //abstract address
} STNBScnVertexTex3Ptr;

//

NB_OBJREF_HEADER(NBScnRender)

//Prepare

BOOL NBScnRender_prepare(STNBScnRenderRef ref, const STNBScnRenderApiItf* itf, void* itfParam);

//Vertices

STNBScnVertexIdxPtr     NBScnRender_vIdxsAlloc(STNBScnRenderRef ref, const UI32 amm);
STNBScnVertexPtr        NBScnRender_vertsAlloc(STNBScnRenderRef ref, const UI32 amm);
STNBScnVertexTexPtr     NBScnRender_vertsTexAlloc(STNBScnRenderRef ref, const UI32 amm);
STNBScnVertexTex2Ptr    NBScnRender_vertsTex2Alloc(STNBScnRenderRef ref, const UI32 amm);
STNBScnVertexTex3Ptr    NBScnRender_vertsTex3Alloc(STNBScnRenderRef ref, const UI32 amm);

BOOL NBScnRender_vIdxsInvalidate(STNBScnRenderRef ref, const STNBScnVertexIdxPtr ptr, const UI32 sz);
BOOL NBScnRender_vertsInvalidate(STNBScnRenderRef ref, const STNBScnVertexPtr ptr, const UI32 sz);
BOOL NBScnRender_vertsTexInvalidate(STNBScnRenderRef ref, const STNBScnVertexTexPtr ptr, const UI32 sz);
BOOL NBScnRender_vertsTex2Invalidate(STNBScnRenderRef ref, const STNBScnVertexTex2Ptr ptr, const UI32 sz);
BOOL NBScnRender_vertsTex3Invalidate(STNBScnRenderRef ref, const STNBScnVertexTex3Ptr ptr, const UI32 sz);

BOOL NBScnRender_vIdxsFree(STNBScnRenderRef ref, const STNBScnVertexIdxPtr ptr);
BOOL NBScnRender_vertsFree(STNBScnRenderRef ref, const STNBScnVertexPtr ptr);
BOOL NBScnRender_vertsTexFree(STNBScnRenderRef ref, const STNBScnVertexTexPtr ptr);
BOOL NBScnRender_vertsTex2Free(STNBScnRenderRef ref, const STNBScnVertexTex2Ptr ptr);
BOOL NBScnRender_vertsTex3Free(STNBScnRenderRef ref, const STNBScnVertexTex3Ptr ptr);

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
