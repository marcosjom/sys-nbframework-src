//
//  ScnApiDX12Framebuff.h
//  ixtli-render
//
//  Created by Marcos Ortega on 10/9/25.
//

#ifndef ScnApiDX12Framebuff_h
#define ScnApiDX12Framebuff_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnContext.h"
#include "ixrender/type/ScnSize.h"
#include "ixrender/gpu/ScnGpuFramebuff.h"
#include "ixrender/api/ScnApiItf.h"
//
#include "ScnApiDX12RenderStates.h"
#include "ScnApiDX12Buffer.h"
//
#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>
#import <TargetConditionals.h>  //for TARGET_OS_* macros

#ifdef __cplusplus
extern "C" {
#endif

//STScnApiDX12FramebuffView

typedef struct STScnApiDX12FramebuffView {
    ScnContextRef           ctx;
    MTKView*                mtkView;
    STScnSize2DU            size;
    STScnGpuFramebuffProps  props;
    STScnApiItf             itf;
    STScnApiDX12RenderStates rndrShaders; //shaders
    //cur (state while sending commands)
    struct {
        //verts
        struct {
            ENScnVertexType type;
            STScnApiDX12Buffer* buff;
            STScnApiDX12Buffer* idxs;
        } verts;
    } cur;
} STScnApiDX12FramebuffView;

void            ScnApiDX12Framebuff_view_free(void* data);
STScnSize2DU    ScnApiDX12Framebuff_view_getSize(void* pObj);
ScnBOOL         ScnApiDX12Framebuff_view_syncSize(void* pObj, const STScnSize2DU size);
STScnGpuFramebuffProps ScnApiDX12Framebuff_view_getProps(void* data);
ScnBOOL         ScnApiDX12Framebuff_view_setProps(void* data, const STScnGpuFramebuffProps* const props);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnApiDX12Framebuff_h */
