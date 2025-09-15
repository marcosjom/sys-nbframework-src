//
//  ScnApiDX12RenderJob.h
//  ixtli-render
//
//  Created by Marcos Ortega on 10/9/25.
//

#ifndef ScnApiDX12RenderJob_h
#define ScnApiDX12RenderJob_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnContext.h"
#include "ixrender/gpu/ScnGpuBuffer.h"
//
#include "ScnApiDX12Device.h"
#include "ScnApiDX12Buffer.h"
#include "ScnApiDX12RenderJobState.h"
//
#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>
#import <TargetConditionals.h>  //for TARGET_OS_* macros

#ifdef __cplusplus
extern "C" {
#endif

// STScnApiDX12RenderJob

typedef struct STScnApiDX12RenderJob {
    ScnContextRef           ctx;
    STScnApiDX12Device*    dev;
    //bPropsScns
    struct {
        ScnGpuBufferRef     ref;
        STScnApiDX12Buffer* obj;
    } bPropsScns;
    //bPropsMdls
    struct {
        ScnGpuBufferRef     ref;
        STScnApiDX12Buffer* obj;
    } bPropsMdls;
    //
    id<MTLCommandBuffer>    cmdsBuff;
    //
    STScnApiDX12RenderJobState state;
} STScnApiDX12RenderJob;

void    ScnApiDX12RenderJob_free(void* data);
ENScnGpuRenderJobState ScnApiDX12RenderJob_getState(void* data);
ScnBOOL ScnApiDX12RenderJob_buildBegin(void* data, ScnGpuBufferRef bPropsScns, ScnGpuBufferRef bPropsMdls);
ScnBOOL ScnApiDX12RenderJob_buildAddCmds(void* data, const struct STScnRenderCmd* const cmds, const ScnUI32 cmdsSz);
ScnBOOL ScnApiDX12RenderJob_buildEndAndEnqueue(void* data);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnApiDX12RenderJob_h */
