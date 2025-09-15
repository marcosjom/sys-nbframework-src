//
//  ScnApiDX12RenderJobState.h
//  ixtli-render
//
//  Created by Marcos Ortega on 10/9/25.
//

#ifndef ScnApiDX12RenderJobState_h
#define ScnApiDX12RenderJobState_h

#include "ixrender/ixtli-defs.h"
//
#include "ScnApiDX12Framebuff.h"
//
#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>
#import <TargetConditionals.h>  //for TARGET_OS_* macros

#ifdef __cplusplus
extern "C" {
#endif

// STScnApiDX12RenderJobState

typedef struct STScnApiDX12RenderJobState {
    STScnApiDX12FramebuffView* fb;
    MTLRenderPassDescriptor*    rndrDesc; //per active framebuff
    id<MTLRenderCommandEncoder> rndrEnc; //per active framebuff
} STScnApiDX12RenderJobState;

void    ScnApiDX12RenderJobState_init(STScnApiDX12RenderJobState* obj);
void    ScnApiDX12RenderJobState_destroy(STScnApiDX12RenderJobState* obj);
//
void    ScnApiDX12RenderJobState_reset(STScnApiDX12RenderJobState* obj);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnApiDX12RenderJobState_h */
