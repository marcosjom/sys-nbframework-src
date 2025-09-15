//
//  ScnApiDX12Vertexbuff.m
//  ixtli-render
//
//  Created by Marcos Ortega on 10/9/25.
//

#include "ScnApiDX12Vertexbuff.h"


void ScnApiDX12Vertexbuff_free(void* pObj){
    STScnApiDX12Vertexbuff* obj = (STScnApiDX12Vertexbuff*)pObj;
    ScnContextRef ctx = obj->ctx;
    {
        ScnGpuBuffer_releaseAndNull(&obj->vBuff);
        ScnGpuBuffer_releaseAndNull(&obj->idxBuff);
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

ScnBOOL ScnApiDX12Vertexbuff_sync(void* pObj, const STScnGpuVertexbuffCfg* const cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff){
    ScnBOOL r = ScnFALSE;
    STScnApiDX12Vertexbuff* obj = (STScnApiDX12Vertexbuff*)pObj;
    {
        obj->cfg = *cfg;
        ScnGpuBuffer_set(&obj->vBuff, vBuff);
        ScnGpuBuffer_set(&obj->idxBuff, idxBuff);
        //ToDo: implement
        r = ScnTRUE;
    }
    return r;
}

ScnBOOL ScnApiDX12Vertexbuff_activate(void* pObj){
    ScnBOOL r = ScnFALSE;
    //STScnApiDX12Vertexbuff* obj = (STScnApiDX12Vertexbuff*)pObj;
    {
        //ToDo: implement
        r = ScnTRUE;
    }
    return r;
}

ScnBOOL ScnApiDX12Vertexbuff_deactivate(void* pObj){
    ScnBOOL r = ScnFALSE;
    //STScnApiDX12Vertexbuff* obj = (STScnApiDX12Vertexbuff*)pObj;
    {
        //ToDo: implement
        r = ScnTRUE;
    }
    return r;
}
