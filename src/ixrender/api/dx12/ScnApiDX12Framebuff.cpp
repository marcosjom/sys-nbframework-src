//
//  ScnApiDX12Framebuff.m
//  ixtli-render
//
//  Created by Marcos Ortega on 10/9/25.
//

#include "ScnApiDX12Framebuff.h"

//frameBuffer (view)

void ScnApiDX12Framebuff_view_free(void* pObj){
    STScnApiDX12FramebuffView* obj = (STScnApiDX12FramebuffView*)pObj;
    ScnContextRef ctx = obj->ctx;
    {
        //
        STScnApiDX12RenderStates_destroy(&obj->rndrShaders);
        //
        if(obj->mtkView != nil){
            [obj->mtkView release];
            obj->mtkView = nil;
        }
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

STScnSize2DU ScnApiDX12Framebuff_view_getSize(void* pObj){
    STScnApiDX12FramebuffView* obj = (STScnApiDX12FramebuffView*)pObj;
    return obj->size;
}

ScnBOOL ScnApiDX12Framebuff_view_syncSize(void* pObj, const STScnSize2DU size){
    ScnBOOL r = ScnFALSE;
    STScnApiDX12FramebuffView* obj = (STScnApiDX12FramebuffView*)pObj;
    if(obj->mtkView != nil){
        //const CGSize sz = obj->mtkView.drawableSize;
        //obj->size.width = sz.width;
        //obj->size.height = sz.height;
        obj->size = size;
        r = ScnTRUE;
    }
    return r;
}

STScnGpuFramebuffProps ScnApiDX12Framebuff_view_getProps(void* pObj){
    STScnApiDX12FramebuffView* obj = (STScnApiDX12FramebuffView*)pObj;
    return obj->props;
}

ScnBOOL ScnApiDX12Framebuff_view_setProps(void* pObj, const STScnGpuFramebuffProps* const props){
    ScnBOOL r = ScnFALSE;
    STScnApiDX12FramebuffView* obj = (STScnApiDX12FramebuffView*)pObj;
    if(obj->mtkView != nil && props != NULL){
        obj->props = *props;
        r = ScnTRUE;
    }
    return r;
}
