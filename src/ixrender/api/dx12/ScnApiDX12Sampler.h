//
//  ScnApiDX12Sampler.h
//  ixtli-render
//
//  Created by Marcos Ortega on 10/9/25.
//

#ifndef ScnApiDX12Sampler_h
#define ScnApiDX12Sampler_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnContext.h"
#include "ixrender/gpu/ScnGpuSampler.h"
//
#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>
#import <TargetConditionals.h>  //for TARGET_OS_* macros

#ifdef __cplusplus
extern "C" {
#endif

// STScnApiDX12Sampler

typedef struct STScnApiDX12Sampler {
    ScnContextRef           ctx;
    STScnGpuSamplerCfg      cfg;
    //id<MTLSamplerState>     smplr;
} STScnApiDX12Sampler;

void                ScnApiDX12Sampler_free(void* pObj);
STScnGpuSamplerCfg  ScnApiDX12Sampler_getCfg(void* data);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnApiDX12Sampler_h */
