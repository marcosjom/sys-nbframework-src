//
//  ScnApiDX12Device.h
//  ixtli-render
//
//  Created by Marcos Ortega on 10/9/25.
//

#ifndef ScnApiDX12Device_h
#define ScnApiDX12Device_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnContext.h"
#include "ixrender/core/ScnMutex.h"
#include "ixrender/core/ScnArraySorted.h"
#include "ixrender/api/ScnApiItf.h"
#include "ScnApiDX12Heap.h"
//
#ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif
#include <windows.h>
#include <d3d12.h>

#ifdef __cplusplus
extern "C" {
#endif

//STScnApiDX12Device

typedef struct STScnApiDX12Device {
    ScnContextRef   ctx;
    STScnApiItf     itf;
    ID3D12Device*   dev;
    D3D12_FEATURE_DATA_D3D12_OPTIONS devOpts;
    ID3D12CommandQueue* cmdQueue;
    //heaps
    struct {
        ScnMutexRef mutex;
        ScnUI64     iAddrSeq;   //total ammount of bytes ever allocated, used to assign an 'unique' range of addresses to each Heap allocated
        ScnArraySortedStruct(arr, STScnApiDX12Heap);
        //byType
        STScnApiDX12HeapCfg byType[ENScnApiDX12HeapType_Count];
    } heaps;
} STScnApiDX12Device;

ScnGpuDeviceRef     ScnApiDX12_allocDevice(ScnContextRef ctx, const STScnGpuDeviceCfg* cfg);
void                ScnApiDX12Device_free(void* obj);
/*
void*               ScnApiDX12Device_getApiDevice(void* obj);
STScnGpuDeviceDesc  ScnApiDX12Device_getDesc(void* obj);
ScnGpuBufferRef     ScnApiDX12Device_allocBuffer(void* obj, ScnMemElasticRef mem);
ScnGpuVertexbuffRef ScnApiDX12Device_allocVertexBuff(void* obj, const STScnGpuVertexbuffCfg* const cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff);
ScnGpuFramebuffRef  ScnApiDX12Device_allocFramebuffFromOSView(void* obj, void* mtkView);
ScnGpuTextureRef    ScnApiDX12Device_allocTexture(void* obj, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const srcProps, const void* srcData);
ScnGpuSamplerRef    ScnApiDX12Device_allocSampler(void* obj, const STScnGpuSamplerCfg* const cfg);
ScnGpuRenderJobRef  ScnApiDX12Device_allocRenderJob(void* obj);
*/

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnApiDX12Device_h */
