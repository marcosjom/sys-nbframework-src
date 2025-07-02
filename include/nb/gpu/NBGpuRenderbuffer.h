#ifndef NB_GPU_RENDERBUFFER_H
#define NB_GPU_RENDERBUFFER_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBMemoryBlocks.h"
#include "nb/core/NBStructMap.h"
#include "nb/2d/NBBitmap.h"

#ifdef __cplusplus
extern "C" {
#endif

//STNBGpuRenderbufferCfg

#define STNBGpuRenderbufferCfg_Zero   { ENNBBitmapColor_undef, 0, 0 }

typedef struct STNBGpuRenderbufferCfg_ {
    ENNBBitmapColor color;
    UI32            width;
    UI32            height;
} STNBGpuRenderbufferCfg;

const STNBStructMap* NBGpuRenderbufferCfg_getSharedStructMap(void);

//STNBGpuRenderbufferChanges

typedef struct STNBGpuRenderbufferChanges_ {
    UI32 dummy;  //nothing
} STNBGpuRenderbufferChanges;

const STNBStructMap* NBGpuRenderbufferChanges_getSharedStructMap(void);

//STNBGpuRenderbufferApiItf

typedef struct STNBGpuRenderbufferApiItf_ {
    void* (*create)(const STNBGpuRenderbufferCfg* cfg, void* usrData);
    void  (*destroy)(void* data, void* usrData);
    //
    BOOL  (*sync)(void* data, const STNBGpuRenderbufferCfg* cfg, const STNBGpuRenderbufferChanges* changes, void* usrData);
} STNBGpuRenderbufferApiItf;

//

NB_OBJREF_HEADER(NBGpuRenderbuffer)

//

BOOL NBGpuRenderbuffer_prepare(STNBGpuRenderbufferRef ref, const STNBGpuRenderbufferCfg* cfg, const STNBGpuRenderbufferApiItf* itf, void* itfParam);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
