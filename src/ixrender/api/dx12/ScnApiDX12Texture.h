//
//  ScnApiDX12Texture.h
//  ixtli-render
//
//  Created by Marcos Ortega on 10/9/25.
//

#ifndef ScnApiDX12Texture_h
#define ScnApiDX12Texture_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/gpu/ScnGpuSampler.h"
#include "ixrender/gpu/ScnGpuTexture.h"
#include "ixrender/api/ScnApiItf.h"
//
#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>
#import <TargetConditionals.h>  //for TARGET_OS_* macros

#ifdef __cplusplus
extern "C" {
#endif

// STScnApiDX12Texture

typedef struct STScnApiDX12Texture {
    ScnContextRef           ctx;
    STScnGpuTextureCfg      cfg;
    id<MTLTexture>          tex;
    ScnGpuSamplerRef        sampler;
    STScnApiItf             itf;
} STScnApiDX12Texture;

void    ScnApiDX12Texture_free(void* pObj);
ScnBOOL ScnApiDX12Texture_sync(void* data, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const srcProps, const void* srcData, const STScnGpuTextureChanges* const changes);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnApiDX12Texture_h */
