//
//  ScnApiDX12Sampler.m
//  ixtli-render
//
//  Created by Marcos Ortega on 10/9/25.
//

#include "ScnApiDX12Sampler.h"


void ScnApiDX12Sampler_free(void* pObj){
    STScnApiDX12Sampler* obj = (STScnApiDX12Sampler*)pObj;
    ScnContextRef ctx = obj->ctx;
    {
        /*if (obj->smplr != nullptr) {
            [obj->smplr release];
            obj->smplr = nullptr;
        }*/
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

STScnGpuSamplerCfg ScnApiDX12Sampler_getCfg(void* pObj){
    STScnGpuSamplerCfg r = STScnGpuSamplerCfg_Zero;
    STScnApiDX12Sampler* obj = (STScnApiDX12Sampler*)pObj;
    if(obj != NULL ){
        r = obj->cfg;
    }
    return r;
}
