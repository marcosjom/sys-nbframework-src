//
//  ScnApiDX12.m
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 10/9/25.
//

#include "ixrender/api/ScnApiDX12.h"
#include "ixrender/gpu/ScnGpuDevice.h"
#include "ixrender/gpu/ScnGpuBuffer.h"
#include "ixrender/gpu/ScnGpuSampler.h"
#include "ixrender/gpu/ScnGpuTexture.h"
#include "ixrender/scene/ScnRenderCmd.h"
//

#include "ScnApiDX12Device.h"
#include "ScnApiDX12Sampler.h"
#include "ScnApiDX12Buffer.h"
#include "ScnApiDX12Vertexbuff.h"
#include "ScnApiDX12Texture.h"
#include "ScnApiDX12RenderStates.h"
#include "ScnApiDX12Framebuff.h"
#include "ScnApiDX12RenderJobState.h"
#include "ScnApiDX12RenderJob.h"

ScnBOOL ScnApiDX12_getApiItf(STScnApiItf* dst){
    if(dst == NULL) return ScnFALSE;
    //
    ScnMemory_setZeroSt(*dst);
    //gobal
    dst->allocDevice        = ScnApiDX12_allocDevice;
    //device
    dst->dev.free           = ScnApiDX12Device_free;
    dst->dev.getApiDevice   = ScnApiDX12Device_getApiDevice;
    dst->dev.getDesc        = ScnApiDX12Device_getDesc;
    dst->dev.allocBuffer    = ScnApiDX12Device_allocBuffer;
    dst->dev.allocVertexBuff = ScnApiDX12Device_allocVertexBuff;
    dst->dev.allocFramebuffFromOSView = ScnApiDX12Device_allocFramebuffFromOSView;
    dst->dev.allocTexture   = ScnApiDX12Device_allocTexture;
    dst->dev.allocSampler   = ScnApiDX12Device_allocSampler;
    dst->dev.allocRenderJob = ScnApiDX12Device_allocRenderJob;
    //buffer
    dst->buff.free          = ScnApiDX12Buffer_free;
    dst->buff.sync          = ScnApiDX12Buffer_sync;
    //vertexbuff
    dst->vertexBuff.free    = ScnApiDX12Vertexbuff_free;
    dst->vertexBuff.sync    = ScnApiDX12Vertexbuff_sync;
    dst->vertexBuff.activate = ScnApiDX12Vertexbuff_activate;
    dst->vertexBuff.deactivate = ScnApiDX12Vertexbuff_deactivate;
    //
    return ScnTRUE;
}






