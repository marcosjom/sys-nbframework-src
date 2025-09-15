//
//  ScnApiDX12Device.m
//  ixtli-render
//
//  Created by Marcos Ortega on 10/9/25.
//

#include "ScnApiDX12Device.h"
#include "ixrender/api/ScnApiDX12.h"
#include "ScnApiDX12Heap.h"
//#include "ScnApiDX12Sampler.h"
//#include "ScnApiDX12Buffer.h"
//#include "ScnApiDX12Vertexbuff.h"
//#include "ScnApiDX12Texture.h"
//#include "ScnApiDX12Framebuff.h"
//#include "ScnApiDX12RenderJob.h"
//
#include "ixrender/gpu/ScnGpuSampler.h"

//
#include <dxgi1_6.h>

//using namespace DirectX;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
#include <wrl.h>
using Microsoft::WRL::ComPtr;

#ifdef SCN_ASSERTS_ACTIVATED
//#include <string.h> //strncmp()
#endif

ScnGpuDeviceRef ScnApiDX12_allocDevice(ScnContextRef ctx, const STScnGpuDeviceCfg* cfg){
    ScnGpuDeviceRef r = ScnObjRef_Zero;
    if (ScnContext_isNull(ctx)) {
        SCN_PRINTF_ERROR("DX12, ScnApiDX12_allocDevice with null ScnContext.\n");
        return r;
    }
    UINT dxgiFactoryFlags = 0;
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
#   if defined(_DEBUG)
    {
        ComPtr<ID3D12Debug> debugController;
        if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
            SCN_PRINTF_ERROR("DX12, D3D12GetDebugInterface failed.\n");
        } else {
            debugController->EnableDebugLayer();
            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#   endif
    ScnBOOL useSoftAdapter = ScnFALSE;
    ScnBOOL requestHighPerformanceAdapter = ScnFALSE;
    ComPtr<IDXGIFactory4> factory = nullptr;
    ComPtr<ID3D12Device> dev = nullptr;
    ComPtr<ID3D12CommandQueue> cmdQueue = nullptr;
    D3D12_FEATURE_DATA_D3D12_OPTIONS devOpts;
    ScnMemory_setZeroSt(devOpts);
    //Create factory
    if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)))) {
        SCN_PRINTF_ERROR("DX12, CreateDXGIFactory2 failed.\n");
        return r;
    }
    //Open adapter and create device
    if (useSoftAdapter) {
        ComPtr<IDXGIAdapter> warpAdapter = nullptr;
        if (FAILED(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)))) {
            SCN_PRINTF_ERROR("DX12, EnumWarpAdapter failed.\n");
        } else if (S_OK != D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&dev))) {
            SCN_PRINTF_ERROR("DX12, D3D12CreateDevice(warpAdapter) failed.\n");
        }
    } else {
        ComPtr<IDXGIAdapter1> hardwareAdapter = nullptr;
        ComPtr<IDXGIAdapter1> adapter = nullptr;
        ComPtr<IDXGIFactory6> factory6 = nullptr;
        //Query devices
        if (FAILED(factory->QueryInterface(IID_PPV_ARGS(&factory6)))) {
            SCN_PRINTF_ERROR("DX12, QueryInterface(IDXGIFactory6) failed.\n");
        } else {
            for (
                UINT adapterIndex = 0;
                SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                    adapterIndex,
                    requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                    IID_PPV_ARGS(&adapter)));
                ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);
                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }
                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
                    break;
                }
            }
        }
        //Query devices
        if (adapter == nullptr) {
            for (UINT adapterIndex = 0; SUCCEEDED(factory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex) {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);
                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }
                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
                    break;
                }
            }
        }
        //create device
        if (adapter == nullptr) {
            SCN_PRINTF_ERROR("DX12, no adapter found.\n");
            return r;
        } else if (FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&dev)))) {
            SCN_PRINTF_ERROR("DX12, D3D12CreateDevice failed.\n");
            return r;
        }
    }
    //Query device options
    {
        if (FAILED(dev->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &devOpts, sizeof(devOpts)))) {
            SCN_PRINTF_ERROR("DX12, CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS) failed.\n");
                return r;
        }
    }
    //Create command queue
    {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        //
        if (FAILED(dev->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&cmdQueue)))) {
            SCN_PRINTF_ERROR("DX12, CreateCommandQueue failed.\n");
            return r;
        }
    }
    //Ceate
    SCN_ASSERT(dev.Get() != nullptr);
    SCN_ASSERT(cmdQueue.Get() != nullptr);
    STScnApiDX12Device* obj = (STScnApiDX12Device*)ScnContext_malloc(ctx, sizeof(STScnApiDX12Device), SCN_DBG_STR("STScnApiDX12Device"));
    if (obj == NULL) {
        SCN_PRINTF_ERROR("DX12, ScnContext_malloc(STScnApiDX12Device) failed.\n");
    } else {
        //init STScnApiDX12Device
        ScnMemory_setZeroSt(*obj);
        ScnContext_set(&obj->ctx, ctx);
        obj->dev = dev.Detach();
        obj->devOpts = devOpts;
        obj->cmdQueue = cmdQueue.Detach();
        //heaps
        {
            //init (heaps)
            obj->heaps.mutex    = ScnContext_allocMutex(ctx);
            obj->heaps.iAddrSeq = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
            ScnArraySorted_init(ctx, &obj->heaps.arr, 0, 16, STScnApiDX12Heap, ScnCompare_STScnApiDX12Heap);
            //by type
            {
                ScnSI32 i; for (i = 0; i < sizeof(obj->heaps.byType) / sizeof(obj->heaps.byType[0]); ++i) {
                    STScnApiDX12HeapCfg* cfg = &obj->heaps.byType[i];
                    ScnApiDX12HeapCfg_init(cfg);
                    cfg->dev = obj;
                }
            }
            //configure (heaps)
            STScnMemBlockAllocItf heapItf;
            heapItf.malloc  = ScnApiDX12Heap_malloc;
            heapItf.free    = ScnApiDX12Heap_free;
            //
            //Constant buffer view (CBV)
            //Unordered Access view (UAV)
            //Shader resource view (SRV)
            //Samplers entries cannot share a heap with CBV, UAV or SRV entries
            switch (devOpts.ResourceHeapTier) {
                case D3D12_RESOURCE_HEAP_TIER_1:
                    //Three different heaps: (1)buffers, (2)render target or depth stencil textures, (3) other textures
                    //D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS
                    SCN_PRINTF_WARNING("DX12,Device is D3D12_RESOURCE_HEAP_TIER_1; heaps must be divided by types.\n");
                    //configure buffers-heap
                    {
                        ENScnApiDX12HeapType type = ENScnApiDX12HeapType_Buffers;
                        STScnMemElasticCfg mCfg = STScnMemElasticCfg_Zero;
                        mCfg.idxsAlign = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT; //32
                        mCfg.sizeAlign = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
                        mCfg.sizePerBlock = (1024 * 1024 * 4) / mCfg.sizeAlign * mCfg.sizeAlign;
                        mCfg.sizeInitial = 0; //do not allocate memory yet
                        mCfg.isIdxZeroValid = ScnTRUE;
                        STScnApiDX12HeapCfg* cfg = &obj->heaps.byType[type];
                        cfg->flags = D3D12_HEAP_FLAG_NONE;
                        cfg->dev = obj;
                        if (!ScnApiDX12HeapCfg_prepare(cfg, ctx, &mCfg, &heapItf, obj)) {
                            SCN_PRINTF_ERROR("DX12, ScnApiDX12HeapCfg_prepare failed.\n");
                        }
                    }
                    {
                        ENScnApiDX12HeapType type = ENScnApiDX12HeapType_Render;
                        STScnMemElasticCfg mCfg = STScnMemElasticCfg_Zero;
                        mCfg.idxsAlign = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT; //32
                        mCfg.sizeAlign = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
                        mCfg.sizePerBlock = (1024 * 1024 * 4) / mCfg.sizeAlign * mCfg.sizeAlign;
                        mCfg.sizeInitial = 0; //do not allocate memory yet
                        mCfg.isIdxZeroValid = ScnTRUE;
                        STScnApiDX12HeapCfg* cfg = &obj->heaps.byType[type];
                        cfg->flags = D3D12_HEAP_FLAG_NONE;
                        cfg->dev = obj;
                        if (!ScnApiDX12HeapCfg_prepare(cfg, ctx, &mCfg, &heapItf, obj)) {
                            SCN_PRINTF_ERROR("DX12, ScnApiDX12HeapCfg_prepare failed.\n");
                        }
                    }
                    {
                        ENScnApiDX12HeapType type = ENScnApiDX12HeapType_Textures;
                        STScnMemElasticCfg mCfg = STScnMemElasticCfg_Zero;
                        mCfg.idxsAlign = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT; //32
                        mCfg.sizeAlign = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
                        mCfg.sizePerBlock = (1024 * 1024 * 4) / mCfg.sizeAlign * mCfg.sizeAlign;
                        mCfg.sizeInitial = 0; //do not allocate memory yet
                        mCfg.isIdxZeroValid = ScnTRUE;
                        STScnApiDX12HeapCfg* cfg = &obj->heaps.byType[type];
                        cfg->flags = D3D12_HEAP_FLAG_NONE;
                        cfg->dev = obj;
                        if (!ScnApiDX12HeapCfg_prepare(cfg, ctx, &mCfg, &heapItf, obj)) {
                            SCN_PRINTF_ERROR("DX12, ScnApiDX12HeapCfg_prepare failed.\n");
                        }
                    }
                    //configure other heaps to use textures-heap
                    {
                        ENScnApiDX12HeapType type = ENScnApiDX12HeapType_Textures;
                        STScnApiDX12HeapCfg* cfg2 = &obj->heaps.byType[type];
                        ScnSI32 i; for (i = 0; i < sizeof(obj->heaps.byType) / sizeof(obj->heaps.byType[0]); ++i) {
                            STScnApiDX12HeapCfg* cfg = &obj->heaps.byType[i];
                            if (!ScnMemElastic_isNull(cfg->mem)) {
                                continue;
                            }
                            cfg->flags = cfg2->flags;
                            cfg->dev = obj;
                            ScnMemElastic_set(&cfg->mem, cfg2->mem);
                        }
                    }
                    break;
                default: //D3D12_RESOURCE_HEAP_TIER_2
                    //Heaps can host any type of objects
                    SCN_PRINTF_INFO("DX12,Device is >= D3D12_RESOURCE_HEAP_TIER_2; heaps are universal.\n");
                    {
                        //configure buffers-heap
                        {
                            ENScnApiDX12HeapType type = ENScnApiDX12HeapType_Buffers;
                            STScnMemElasticCfg mCfg = STScnMemElasticCfg_Zero;
                            mCfg.idxsAlign = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT; //32
                            mCfg.sizeAlign = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
                            mCfg.sizePerBlock = (1024 * 1024 * 4) / mCfg.sizeAlign * mCfg.sizeAlign;
                            mCfg.sizeInitial = 0; //do not allocate memory yet
                            mCfg.isIdxZeroValid = ScnTRUE;
                            //
                            STScnApiDX12HeapCfg* cfg = &obj->heaps.byType[type];
                            cfg->flags = D3D12_HEAP_FLAG_NONE;
                            cfg->dev = obj;
                            if(!ScnApiDX12HeapCfg_prepare(cfg, ctx, &mCfg, &heapItf, obj)){
                                SCN_PRINTF_ERROR("DX12, ScnApiDX12HeapCfg_prepare failed.\n");
                            }
                        }
                        //configure other heaps to use buffers-heap
                        {
                            STScnApiDX12HeapCfg* cfg2 = &obj->heaps.byType[ENScnApiDX12HeapType_Buffers];
                            ScnSI32 i; for (i = 0; i < sizeof(obj->heaps.byType) / sizeof(obj->heaps.byType[0]); ++i) {
                                STScnApiDX12HeapCfg* cfg = &obj->heaps.byType[i];
                                if (!ScnMemElastic_isNull(cfg->mem)) {
                                    continue;
                                }
                                cfg->flags = cfg2->flags;
                                cfg->dev = obj;
                                ScnMemElastic_set(&cfg->mem, cfg2->mem);
                            }
                        }
                    }
                    break;
            }
        }
        //
        if (!ScnApiDX12_getApiItf(&obj->itf)) {
            SCN_PRINTF_ERROR("ScnApiDX12_allocDevice::ScnApiDX12_getApiItf failed.\n");
        } else {
            ScnGpuDeviceRef d = ScnGpuDevice_alloc(ctx);
            if (!ScnGpuDevice_isNull(d)) {
                if (!ScnGpuDevice_prepare(d, &obj->itf.dev, obj)) {
                    SCN_PRINTF_ERROR("ScnApiDX12_allocDevice::ScnGpuDevice_prepare failed.\n");
                } else {
                    ScnGpuDevice_set(&r, d);
                    obj = NULL; //consume
                }
                ScnGpuDevice_releaseAndNull(&d);
            }
        }
    }
    //release (if not consumed)
    if (obj != NULL) {
        ScnApiDX12Device_free(obj);
        obj = NULL;
    }
    //
    return r;
}

void ScnApiDX12Device_free(void* pObj){
    STScnApiDX12Device* obj = (STScnApiDX12Device*)pObj;
    ScnContextRef ctx = obj->ctx;
    {
        //heaps
        {
            //by type
            {
                ScnSI32 i; for (i = 0; i < sizeof(obj->heaps.byType) / sizeof(obj->heaps.byType[0]); ++i) {
                    STScnApiDX12HeapCfg* cfg = &obj->heaps.byType[i];
                    ScnApiDX12HeapCfg_destroy(cfg);
                }
            }
            SCN_ASSERT(obj->heaps.arr.use == 0) //all heaps should be already destroyed
            ScnArraySorted_empty(&obj->heaps.arr);
            ScnArraySorted_destroy(ctx, &obj->heaps.arr);
            ScnMutex_free(&obj->heaps.mutex);
        }
        if(obj->cmdQueue != nullptr){
            obj->cmdQueue->Release();
            obj->cmdQueue = nullptr;
        }
        if(obj->dev != nullptr){
            obj->dev->Release();
            obj->dev = nullptr;
        }
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

// STScnApiDX12Sampler

ScnGpuSamplerRef ScnApiDX12Device_allocSampler(void* pObj, const STScnGpuSamplerCfg* const cfg){
    ScnGpuSamplerRef r = ScnGpuSamplerRef_Zero;
    STScnApiDX12Device* dev = (STScnApiDX12Device*)pObj;
    if (dev == NULL || dev->dev == nullptr || cfg == NULL) {
        SCN_PRINTF_ERROR("ScnApiDX12_device_allocSampler missing params.\n");
        return r;
    }
    //sampler description
    D3D12_SAMPLER_DESC sDesc = {};
    sDesc.Filter = (
        cfg->minFilter == ENScnGpuSamplerFilter_Linear && cfg->magFilter == ENScnGpuSamplerFilter_Linear ? D3D12_FILTER_MIN_MAG_MIP_LINEAR :
        cfg->minFilter == ENScnGpuSamplerFilter_Linear && cfg->magFilter == ENScnGpuSamplerFilter_Nearest ? D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT :
        cfg->minFilter == ENScnGpuSamplerFilter_Nearest && cfg->magFilter == ENScnGpuSamplerFilter_Linear ? D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR :
        D3D12_FILTER_MIN_MAG_MIP_POINT
    );
    sDesc.AddressU = sDesc.AddressV = sDesc.AddressW = (
        cfg->address == ENScnGpusamplerAddress_Clamp ? D3D12_TEXTURE_ADDRESS_MODE_BORDER : D3D12_TEXTURE_ADDRESS_MODE_WRAP
        );
    sDesc.MipLODBias = 0;
    sDesc.MaxAnisotropy = 0;
    sDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    //sDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK; //static_sampler_only
    sDesc.MinLOD = 0.0f;
    sDesc.MaxLOD = D3D12_FLOAT32_MAX;
    //sDesc.ShaderRegister = 0; //static_sampler_only
    //sDesc.RegisterSpace = 0; //static_sampler_only
    //sDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //static_sampler_only
    //create sampler
    const ScnUI32 sz = dev->dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    STScnApiDX12HeapCfg* heap = &dev->heaps.byType[ENScnApiDX12HeapType_Samplers];
    STScnAbsPtr ptr = ScnMemElastic_malloc(heap->mem, sz, NULL);
    if (ptr.ptr == NULL) {
        SCN_PRINTF_ERROR("DX12, ScnApiDX12_device_allocSampler, ScnMemElastic_malloc(%u) failed.\n", sz);
        return r;
    }
    SCN_ASSERT(ptr.itfParam != NULL);
    STScnApiDX12Heap* heap = (STScnApiDX12Heap*)ptr.itfParam;
    if (dev->dev->CreateSampler(&sDesc, )) {


        ScnMutexRef mutex;
        ScnUI64     iAddrSeq;   //total ammount of bytes ever allocated, used to assign an 'unique' range of addresses to each Heap allocated
        ScnArraySortedStruct(arr, STScnApiDX12Heap);
        //byType
        STScnApiDX12HeapCfg byType[ENScnApiDX12HeapType_Count];
    }
    //ID3D12SamplerState* sampler = nullptr;
    

    /*
    {
        MTLSamplerDescriptor* desc = [MTLSamplerDescriptor new];
        if(desc == nullptr){
            SCN_PRINTF_ERROR("[MTLSamplerDescriptor new] failed.\n");
        } else {
            const MTLSamplerAddressMode addressMode = (cfg->address == ENScnGpusamplerAddress_Clamp ? MTLSamplerAddressModeClampToEdge : MTLSamplerAddressModeRepeat);
            desc.minFilter = (cfg->magFilter == ENScnGpuSamplerFilter_Linear ? MTLSamplerMinMagFilterLinear : MTLSamplerMinMagFilterNearest);
            desc.magFilter = (cfg->minFilter == ENScnGpuSamplerFilter_Linear ? MTLSamplerMinMagFilterLinear : MTLSamplerMinMagFilterNearest);
            desc.sAddressMode = desc.tAddressMode = desc.rAddressMode = addressMode;
            id<MTLSamplerState> sampler = [dev->dev newSamplerStateWithDescriptor:desc];
            if(sampler == nullptr){
                SCN_PRINTF_ERROR("[dev newSamplerStateWithDescriptor] failed.\n");
            } else {
                STScnApiDX12Sampler* obj = (STScnApiDX12Sampler*)ScnContext_malloc(dev->ctx, sizeof(STScnApiDX12Sampler), SCN_DBG_STR("STScnApiDX12Sampler"));
                if(obj == NULL){
                    SCN_PRINTF_ERROR("ScnContext_malloc(STScnApiDX12Sampler) failed.\n");
                } else {
                    ScnMemory_setZeroSt(*obj);
                    ScnContext_set(&obj->ctx, dev->ctx);
                    obj->cfg    = *cfg;
                    obj->smplr  = sampler; [sampler retain];
                    //
                    ScnGpuSamplerRef s = ScnGpuSampler_alloc(dev->ctx);
                    if(!ScnGpuSampler_isNull(s)){
                        STScnGpuSamplerApiItf itf;
                        ScnMemory_setZeroSt(itf);
                        itf.free    = ScnApiDX12_sampler_free;
                        itf.getCfg  = ScnApiDX12_sampler_getCfg;
                        if(!ScnGpuSampler_prepare(s, &itf, obj)){
                            SCN_PRINTF_ERROR("ScnApiDX12_device_allocSampler::ScnGpuSampler_prepare failed.\n");
                        } else {
                            ScnGpuSampler_set(&r, s);
                            obj = NULL; //consume
                        }
                        ScnGpuSampler_releaseAndNull(&s);
                    }
                }
                //release (if not consumed)
                if(obj != NULL){
                    ScnApiDX12_sampler_free(obj);
                    obj = NULL;
                }
                //
                [sampler release];
                sampler = nullptr;
            }
            //
            [desc release];
            desc = nullptr;
        }
    }*/
    return r;
}

void* ScnApiDX12Device_getApiDevice(void* pObj){
    STScnApiDX12Device* dev = (STScnApiDX12Device*)pObj;
    return (dev != NULL ? dev->dev : NULL);
}

STScnGpuDeviceDesc ScnApiDX12Device_getDesc(void* obj){
    STScnGpuDeviceDesc r = STScnGpuDeviceDesc_Zero;
    {
        r.isTexFmtInfoRequired  = ScnTRUE;  //fragment shader requires the textures format info to produce correct color output
#       if TARGET_OS_SIMULATOR
        //Simulators require 256B alignment.
        //https://developer.apple.com/documentation/metal/developing-metal-apps-that-run-in-simulator?language=objc
        r.offsetsAlign          = 256;       //buffers offsets aligment
#       else
        r.offsetsAlign          = 32;       //buffers offsets aligment
#       endif
        r.memBlockAlign         = 256;      //buffer memory copy alignment
    }
    return r;
}

ScnGpuBufferRef ScnApiDX12Device_allocBuffer(void* pObj, ScnMemElasticRef mem){
    ScnGpuBufferRef r = ScnObjRef_Zero;
    STScnApiDX12Device* dev = (STScnApiDX12Device*)pObj;
    if(dev != NULL && dev->dev != NULL && !ScnMemElastic_isNull(mem)){
        const ScnUI32 cpuBuffSz = ScnMemElastic_getAddressableSize(mem);
        if(cpuBuffSz <= 0){
            SCN_PRINTF_ERROR("allocating zero-sz gpu buffer is not allowed.\n");
        } else {
            id<MTLBuffer> buff = [dev->dev newBufferWithLength:cpuBuffSz options:MTLResourceStorageModeShared | MTLResourceCPUCacheModeWriteCombined];
            if(buff != nil){
                const STScnRangeU rngAll = ScnMemElastic_getUsedAddressesRngAligned(mem);
                if(!ScnApiDX12Buffer_syncRanges_(buff, mem, &rngAll, 1)){
                    SCN_PRINTF_ERROR("ScnApiDX12Buffer_syncRanges_ failed.\n");
                } else {
                    //synced
                    STScnApiDX12Buffer* obj = (STScnApiDX12Buffer*)ScnContext_malloc(dev->ctx, sizeof(STScnApiDX12Buffer), SCN_DBG_STR("STScnApiDX12Buffer"));
                    if(obj == NULL){
                        SCN_PRINTF_ERROR("ScnContext_malloc(STScnApiDX12Buffer) failed.\n");
                    } else {
                        ScnMemory_setZeroSt(*obj);
                        ScnContext_set(&obj->ctx, dev->ctx);
                        obj->itf        = dev->itf;
                        obj->dev        = dev->dev;  //retain?
                        obj->buff       = buff; [buff retain];
                        //
                        ScnGpuBufferRef d = ScnGpuBuffer_alloc(dev->ctx);
                        if(!ScnGpuBuffer_isNull(d)){
                            if(!ScnGpuBuffer_prepare(d, &obj->itf.buff, obj)){
                                SCN_PRINTF_ERROR("ScnApiDX12_allocDevice::ScnGpuBuffer_prepare failed.\n");
                            } else {
                                ScnGpuBuffer_set(&r, d);
                                obj = NULL; //consume
                            }
                            ScnGpuBuffer_releaseAndNull(&d);
                        }
                    }
                    //release (if not consumed)
                    if(obj != NULL){
                        ScnApiDX12Buffer_free(obj);
                        obj = NULL;
                    }
                }
                [buff release];
                buff = nil;
            }
        }
    }
    return r;
}

ScnGpuVertexbuffRef ScnApiDX12Device_allocVertexBuff(void* pObj, const STScnGpuVertexbuffCfg* const cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff){
    ScnGpuVertexbuffRef r = ScnObjRef_Zero;
    STScnApiDX12Device* dev = (STScnApiDX12Device*)pObj;
    if(dev != NULL && dev->dev != NULL && cfg != NULL && !ScnGpuBuffer_isNull(vBuff)){ //idxBuff is optional
        //synced
        STScnApiDX12Vertexbuff* obj = (STScnApiDX12Vertexbuff*)ScnContext_malloc(dev->ctx, sizeof(STScnApiDX12Vertexbuff), SCN_DBG_STR("STScnApiDX12Vertexbuff"));
        if(obj == NULL){
            SCN_PRINTF_ERROR("ScnContext_malloc(STScnApiDX12Vertexbuff) failed.\n");
        } else {
            ScnMemory_setZeroSt(*obj);
            ScnContext_set(&obj->ctx, dev->ctx);
            obj->itf        = dev->itf;
            obj->cfg        = *cfg;
            ScnGpuBuffer_set(&obj->vBuff, vBuff);
            ScnGpuBuffer_set(&obj->idxBuff, idxBuff);
            //
            ScnGpuVertexbuffRef d = ScnGpuVertexbuff_alloc(dev->ctx);
            if(!ScnGpuVertexbuff_isNull(d)){
                if(!ScnGpuVertexbuff_prepare(d, &obj->itf.vertexBuff, obj)){
                    SCN_PRINTF_ERROR("ScnApiDX12Device_allocVertexBuff::ScnGpuVertexbuff_prepare failed.\n");
                } else {
                    ScnGpuVertexbuff_set(&r, d);
                    obj = NULL; //consume
                }
                ScnGpuVertexbuff_releaseAndNull(&d);
            }
        }
        //release (if not consumed)
        if(obj != NULL){
            ScnApiDX12Vertexbuff_free(obj);
            obj = NULL;
        }
    }
    return r;
}

ScnGpuTextureRef ScnApiDX12Device_allocTexture(void* pObj, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const srcProps, const void* srcData){
    ScnGpuTextureRef r = ScnObjRef_Zero;
    STScnApiDX12Device* dev = (STScnApiDX12Device*)pObj;
    if(dev != NULL && dev->dev != nil && cfg != NULL){
        MTLPixelFormat fmt = MTLPixelFormatInvalid;
        switch (cfg->color) {
            case ENScnBitmapColor_ALPHA8: fmt = MTLPixelFormatA8Unorm; break;
            case ENScnBitmapColor_GRAY8: fmt = MTLPixelFormatR8Unorm; break;
            case ENScnBitmapColor_GRAYALPHA8: fmt = MTLPixelFormatRG8Unorm; break;
            case ENScnBitmapColor_RGBA8: fmt = MTLPixelFormatRGBA8Unorm; break;
            default: break;
        }
        if(fmt == MTLPixelFormatInvalid){
            SCN_PRINTF_ERROR("unsupported texture color format(%d).\n", cfg->color);
        } else if(cfg->width <= 0 && cfg->height <= 0){
            SCN_PRINTF_ERROR("invalid texture size(%d, %d).\n", cfg->width, cfg->height);
        } else {
            STScnApiDX12Texture* obj = NULL;
            MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
            id<MTLTexture> tex = nil;
            //
            texDesc.pixelFormat = fmt;
            texDesc.width       = cfg->width;
            texDesc.height      = cfg->height;
            //
            tex = [dev->dev newTextureWithDescriptor:texDesc];
            if(tex == nil){
                SCN_PRINTF_ERROR("newTextureWithDescriptor failed.\n");
            } else if(srcProps != NULL && (srcProps->size.width != cfg->width || srcProps->size.height != cfg->height || srcProps->color != cfg->color || srcProps->bytesPerLine <= 0 || srcData == NULL)){
                SCN_PRINTF_ERROR("texture and source props missmatch.\n");
            } else if(NULL == (obj = (STScnApiDX12Texture*)ScnContext_malloc(dev->ctx, sizeof(STScnApiDX12Texture), SCN_DBG_STR("STScnApiDX12Texture")))){
                SCN_PRINTF_ERROR("ScnContext_malloc(STScnApiDX12Texture) failed.\n");
            } else {
                ScnMemory_setZeroSt(*obj);
                ScnContext_set(&obj->ctx, dev->ctx);
                obj->itf            = dev->itf;
                obj->tex            = tex; [obj->tex retain];
                obj->cfg            = *cfg;
                obj->sampler        = ScnApiDX12Device_allocSampler(dev, &cfg->sampler);
                if(ScnGpuSampler_isNull(obj->sampler)){
                    SCN_PRINTF_ERROR("ScnApiDX12Device_allocTexture::ScnApiDX12Device_allocSampler failed.\n");
                } else {
                    ScnGpuTextureRef d = ScnGpuTexture_alloc(dev->ctx);
                    if(!ScnGpuTexture_isNull(d)){
                        STScnGpuTextureApiItf itf;
                        ScnMemory_setZeroSt(itf);
                        itf.free        = ScnApiDX12Texture_free;
                        itf.sync        = ScnApiDX12Texture_sync;
                        if(!ScnGpuTexture_prepare(d, &itf, obj)){
                            SCN_PRINTF_ERROR("ScnApiDX12Device_allocTexture::ScnGpuTexture_prepare failed.\n");
                        } else {
                            //apply data
                            if(srcProps != NULL && srcData != NULL){
                                MTLRegion region = { { 0, 0, 0 }, {cfg->width, cfg->height, 1} };
                                [tex replaceRegion:region mipmapLevel:0 withBytes:srcData bytesPerRow:srcProps->bytesPerLine];
                            }
                            //
                            ScnGpuTexture_set(&r, d);
                            obj = NULL; //consume
                        }
                        ScnGpuTexture_releaseAndNull(&d);
                    }
                }
            }
            //
            if(texDesc != nil){ [texDesc release]; texDesc = nil; }
            if(tex != nil){ [tex release]; tex = nil; }
            if(obj != NULL){
                ScnApiDX12Texture_free(obj);
                obj = NULL;
            }
        }
    }
    return r;
}

ScnGpuFramebuffRef ScnApiDX12Device_allocFramebuffFromOSView(void* pObj, void* pMtkView){
    ScnGpuFramebuffRef r = ScnObjRef_Zero;
    STScnApiDX12Device* dev = (STScnApiDX12Device*)pObj;
    MTKView* mtkView = (MTKView*)pMtkView;
    if(dev != NULL && dev->dev != NULL && mtkView != nil){
        STScnApiDX12FramebuffView* obj = NULL;
        STScnApiDX12RenderStates rndrShaders;
        STScnApiDX12RenderStates_init(&rndrShaders);
        if(!STScnApiDX12RenderStates_load(&rndrShaders, dev, mtkView.colorPixelFormat)){
            SCN_PRINTF_ERROR("STScnApiDX12RenderStates_load failed.\n");
        } else if(NULL == (obj = (STScnApiDX12FramebuffView*)ScnContext_malloc(dev->ctx, sizeof(STScnApiDX12FramebuffView), SCN_DBG_STR("STScnApiDX12FramebuffView")))){
            SCN_PRINTF_ERROR("ScnContext_malloc(STScnApiDX12FramebuffView) failed.\n");
            STScnApiDX12RenderStates_destroy(&rndrShaders);
        } else {
            CGSize viewSz = mtkView.drawableSize;
            ScnMemory_setZeroSt(*obj);
            ScnContext_set(&obj->ctx, dev->ctx);
            obj->itf            = dev->itf;
            obj->mtkView        = mtkView; [obj->mtkView retain];
            obj->rndrShaders   = rndrShaders;
            {
                //size
                obj->size.width             = viewSz.width;
                obj->size.height            = viewSz.height;
                //viewport
                obj->props.viewport.x       = 0;
                obj->props.viewport.y       = 0;
                obj->props.viewport.width   = obj->size.width;
                obj->props.viewport.height  = obj->size.height;
                //ortho2d
                obj->props.ortho.x.min      = 0.f;
                obj->props.ortho.x.max      = obj->size.width;
                obj->props.ortho.y.min      = 0.f;
                obj->props.ortho.y.max      = obj->size.height;
            }
            //
            ScnGpuFramebuffRef d = ScnGpuFramebuff_alloc(dev->ctx);
            if(!ScnGpuFramebuff_isNull(d)){
                STScnGpuFramebuffApiItf itf;
                ScnMemory_setZeroSt(itf);
                itf.free        = ScnApiDX12Framebuff_view_free;
                itf.getSize     = ScnApiDX12Framebuff_view_getSize;
                itf.syncSize    = ScnApiDX12Framebuff_view_syncSize;
                itf.getProps    = ScnApiDX12Framebuff_view_getProps;
                itf.setProps    = ScnApiDX12Framebuff_view_setProps;
                if(!ScnGpuFramebuff_prepare(d, &itf, obj)){
                    SCN_PRINTF_ERROR("ScnApiDX12Device_allocFramebuffFromOSView::ScnGpuFramebuff_prepare failed.\n");
                } else {
                    //configure view
                    mtkView.device = dev->dev;
                    //
                    ScnGpuFramebuff_set(&r, d);
                    obj = NULL; //consume
                }
                ScnGpuFramebuff_releaseAndNull(&d);
            }
        }
        //release (if not consumed)
        if(obj != NULL){
            ScnApiDX12Framebuff_view_free(obj);
            obj = NULL;
        }
    }
    return r;
}


ScnGpuRenderJobRef ScnApiDX12Device_allocRenderJob(void* pObj){
    ScnGpuRenderJobRef r = ScnGpuRenderJobRef_Zero;
    STScnApiDX12Device* dev = (STScnApiDX12Device*)pObj;
    if(dev != NULL && dev->dev != NULL && dev->cmdQueue != nil){
        STScnApiDX12RenderJob* obj = (STScnApiDX12RenderJob*)ScnContext_malloc(dev->ctx, sizeof(STScnApiDX12RenderJob), SCN_DBG_STR("STScnApiDX12RenderJob"));
        if(obj == NULL){
            SCN_PRINTF_ERROR("ScnContext_malloc(STScnApiDX12RenderJob) failed.\n");
        } else {
            ScnMemory_setZeroSt(*obj);
            ScnContext_set(&obj->ctx, dev->ctx);
            ScnApiDX12RenderJobState_init(&obj->state);
            obj->dev        = dev;  //retain?
            //
            ScnGpuRenderJobRef d = ScnGpuRenderJob_alloc(dev->ctx);
            if(!ScnGpuRenderJob_isNull(d)){
                //
                STScnGpuRenderJobApiItf itf;
                ScnMemory_setZeroSt(itf);
                itf.free        = ScnApiDX12RenderJob_free;
                itf.getState    = ScnApiDX12RenderJob_getState;
                itf.buildBegin  = ScnApiDX12RenderJob_buildBegin;
                itf.buildAddCmds = ScnApiDX12RenderJob_buildAddCmds;
                itf.buildEndAndEnqueue = ScnApiDX12RenderJob_buildEndAndEnqueue;
                //
                if(!ScnGpuRenderJob_prepare(d, &itf, obj)){
                    SCN_PRINTF_ERROR("ScnApiDX12Device_allocRenderJob::ScnGpuRenderJob_prepare failed.\n");
                } else {
                    ScnGpuRenderJob_set(&r, d);
                    obj = NULL; //consume
                }
                ScnGpuRenderJob_releaseAndNull(&d);
            }
        }
        //release (if not consumed)
        if(obj != NULL){
            ScnApiDX12RenderJob_free(obj);
            obj = NULL;
        }
    }
    return r;
}
*/
