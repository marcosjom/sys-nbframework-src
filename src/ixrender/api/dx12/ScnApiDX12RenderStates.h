//
//  ScnApiDX12RenderStates.h
//  ixtli-render
//
//  Created by Marcos Ortega on 10/9/25.
//

#ifndef ScnApiDX12RenderStates_h
#define ScnApiDX12RenderStates_h

#include "ixrender/ixtli-defs.h"
//
#include "ScnApiDX12Device.h"
//
#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>
#import <TargetConditionals.h>  //for TARGET_OS_* macros

#ifdef __cplusplus
extern "C" {
#endif

//STScnApiDX12RenderStates

#define ScnApiDX12RenderStates_ENScnVertexTypeFromVBuffCfg(VBUFF_CFG)  (VBUFF_CFG->texCoords[ENScnGpuTextureIdx_2].amm > 0 ? ENScnVertexType_2DTex3 : VBUFF_CFG->texCoords[ENScnGpuTextureIdx_1].amm > 0 ? ENScnVertexType_2DTex2 : VBUFF_CFG->texCoords[ENScnGpuTextureIdx_0].amm > 0 ? ENScnVertexType_2DTex : ENScnVertexType_2DColor)

typedef struct STScnApiDX12RenderStates {
    MTLPixelFormat              color;
    id<MTLRenderPipelineState>  states[ENScnVertexType_Count]; //shader and fragment for this framebuffer
} STScnApiDX12RenderStates;

void    STScnApiDX12RenderStates_init(STScnApiDX12RenderStates* obj);
void    STScnApiDX12RenderStates_destroy(STScnApiDX12RenderStates* obj);
ScnBOOL STScnApiDX12RenderStates_load(STScnApiDX12RenderStates* obj, STScnApiDX12Device* dev, MTLPixelFormat color);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnApiDX12RenderStates_h */
