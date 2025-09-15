//
//  ScnApiDX12Vertexbuff.h
//  ixtli-render
//
//  Created by Marcos Ortega on 10/9/25.
//

#ifndef ScnApiDX12Vertexbuff_h
#define ScnApiDX12Vertexbuff_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnContext.h"
#include "ixrender/gpu/ScnGpuVertexbuff.h"
#include "ixrender/gpu/ScnGpuBuffer.h"
#include "ixrender/api/ScnApiItf.h"
//
#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>
#import <TargetConditionals.h>  //for TARGET_OS_* macros

#ifdef __cplusplus
extern "C" {
#endif

//STScnApiDX12Vertexbuffer

typedef struct STScnApiDX12Vertexbuff {
    ScnContextRef           ctx;
    STScnGpuVertexbuffCfg   cfg;
    STScnApiItf             itf;
    ScnGpuBufferRef         vBuff;
    ScnGpuBufferRef         idxBuff;
} STScnApiDX12Vertexbuff;

void    ScnApiDX12Vertexbuff_free(void* data);
ScnBOOL ScnApiDX12Vertexbuff_sync(void* data, const STScnGpuVertexbuffCfg* const cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff);
ScnBOOL ScnApiDX12Vertexbuff_activate(void* data);
ScnBOOL ScnApiDX12Vertexbuff_deactivate(void* data);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnApiDX12Vertexbuff_h */
