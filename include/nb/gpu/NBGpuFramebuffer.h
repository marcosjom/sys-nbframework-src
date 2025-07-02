#ifndef NB_GPU_FRAME_BUFFER_H
#define NB_GPU_FRAME_BUFFER_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBMemoryBlocks.h"
#include "nb/core/NBStructMap.h"
#include "nb/2d/NBBitmap.h"

#ifdef __cplusplus
extern "C" {
#endif

//STNBGpuFramebufferCfg

#define STNBGpuFramebufferCfg_Zero   { ENNBBitmapColor_undef, 0, 0 }

typedef struct STNBGpuFramebufferCfg_ {
    UI32 dummy;
} STNBGpuFramebufferCfg;

const STNBStructMap* NBGpuFramebufferCfg_getSharedStructMap(void);

//ENNBGpuFramebufferDstType

typedef enum ENNBGpuFramebufferDstType_ {
    ENNBGpuFramebufferDstType_None = 0,
    ENNBGpuFramebufferDstType_Texture,
    ENNBGpuFramebufferDstType_Renderbuffer,
    //Count
    ENNBGpuFramebufferDstType_Count
} ENNBGpuFramebufferDstType;

//STNBGpuFramebufferChanges

typedef struct STNBGpuFramebufferChanges_ {
    BOOL    bind;
} STNBGpuFramebufferChanges;

const STNBStructMap* NBGpuFramebufferChanges_getSharedStructMap(void);

//STNBGpuFramebufferApiItf

typedef struct STNBGpuFramebufferApiItf_ {
    void* (*create)(const STNBGpuFramebufferCfg* cfg, void* usrData);
    void  (*destroy)(void* data, void* usrData);
    //
    BOOL  (*sync)(void* data, const STNBGpuFramebufferCfg* cfg, const STNBGpuFramebufferChanges* changes, void* usrData);
} STNBGpuFramebufferApiItf;

//

NB_OBJREF_HEADER(NBGpuFramebuffer)

//

BOOL NBGpuFramebuffer_prepare(STNBGpuFramebufferRef ref, const STNBGpuFramebufferCfg* cfg, const STNBGpuFramebufferApiItf* itf, void* itfParam);
BOOL NBGpuFramebuffer_bindTo(STNBGpuFramebufferRef ref, const STNBObjRef dstRef);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
