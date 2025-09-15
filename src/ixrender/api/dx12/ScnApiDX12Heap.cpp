//
//  ScnApiDX12Heap.cpp
//  ixtli-render
//
//  Created by Marcos Ortega on 10/9/25.
//

#include "ScnApiDX12Heap.h"
#include "ScnApiDX12Device.h"

// STScnApiDX12HeapCfg

void ScnApiDX12HeapCfg_init(STScnApiDX12HeapCfg* obj) {
    ScnMemory_setZeroSt(*obj);
}

void ScnApiDX12HeapCfg_destroy(STScnApiDX12HeapCfg* obj) {
    ScnMemElastic_releaseAndNull(&obj->mem);
    obj->flags = D3D12_HEAP_FLAG_NONE;
    obj->dev = NULL;
}

ScnBOOL ScnApiDX12HeapCfg_prepare(STScnApiDX12HeapCfg* obj, ScnContextRef ctx, STScnMemElasticCfg* cfg, STScnMemBlockAllocItf* itf, void* itfParam) {
    ScnBOOL r = ScnFALSE;
    ScnMemElasticRef mem = ScnMemElastic_alloc(ctx);
    if (!ScnMemElastic_isNull(mem)) {
        SCN_PRINTF_ERROR("DX12,ScnMemElastic_alloc failed.\n");
    } else if (!ScnMemElastic_prepare(mem, cfg, itf, itfParam, NULL)) {
        SCN_PRINTF_ERROR("DX12,ScnMemElastic_prepare failed.\n");
    } else {
        ScnMemElastic_set(&obj->mem, mem);
        r = ScnTRUE;
    }
    ScnMemElastic_releaseAndNull(&mem);
    return r;
}

// STScnApiDX12Heap

ScnSI32 ScnCompare_STScnApiDX12Heap(const void* data1, const void* data2, const ScnUI32 dataSz) {
    SCN_ASSERT(dataSz == sizeof(STScnApiDX12Heap))
    if (dataSz == sizeof(STScnApiDX12Heap)) {
        const STScnApiDX12Heap* d1 = (STScnApiDX12Heap*)data1;
        const STScnApiDX12Heap* d2 = (STScnApiDX12Heap*)data2;
        return (d1->iVirtualAddr < d2->iVirtualAddr ? -1 : d1->iVirtualAddr > d2->iVirtualAddr ? 1 : 0);
    }
    return -2;
}

void* ScnApiDX12Heap_malloc(const ScnUI32 size, const char* dbgHintStr, void* itfParam) {
    void* r = NULL;
    STScnApiDX12HeapCfg* cfg = (STScnApiDX12HeapCfg*)itfParam;
    if (cfg == NULL || cfg->dev == nullptr || size <= 0) { //zero-sz heaps are not allowed
        return r;
    }
    SCN_ASSERT((size % D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) == 0) //user missconfiguration
    if ((size % D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) != 0) { //heap must be aligned
        return r;
    }
    STScnApiDX12Device* obj = cfg->dev;
    ScnMutex_lock(obj->heaps.mutex);
    {
        //allocate new heap
        D3D12_HEAP_DESC heapDesc = {};
        heapDesc.SizeInBytes = size;
        heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
        heapDesc.Flags = D3D12_HEAP_FLAG_CREATE_NOT_ZEROED | cfg->flags;
        /*
            D3D12_HEAP_FLAG_NONE = 0,
            D3D12_HEAP_FLAG_SHARED = 0x1,
            D3D12_HEAP_FLAG_DENY_BUFFERS = 0x4,
            D3D12_HEAP_FLAG_ALLOW_DISPLAY = 0x8,
            D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER = 0x20,
            D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES = 0x40,
            D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES = 0x80,
            D3D12_HEAP_FLAG_HARDWARE_PROTECTED = 0x100,
            D3D12_HEAP_FLAG_ALLOW_WRITE_WATCH = 0x200,
            D3D12_HEAP_FLAG_ALLOW_SHADER_ATOMICS = 0x400,
            D3D12_HEAP_FLAG_CREATE_NOT_RESIDENT = 0x800,
            D3D12_HEAP_FLAG_CREATE_NOT_ZEROED = 0x1000,
            D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES = 0,
            D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS = 0xc0,
            D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES = 0x44,
            D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES = 0x84
            */
        //add record
        ID3D12Heap* heap = nullptr;
        if(FAILED(obj->dev->CreateHeap(&heapDesc, IID_PPV_ARGS(&heap)))) {
            SCN_PRINTF_ERROR("DX12, ScnApiDX12Heap_malloc::CreateHeap(%u bytes) failed.\n", size);
        } else {
            STScnApiDX12Heap h;
            ScnMemory_setZeroSt(h);
            h.iVirtualAddr  = obj->heaps.iAddrSeq;
            h.obj           = heap;
            h.size          = size;
            if(NULL == ScnArraySorted_addPtr(obj->ctx, &obj->heaps.arr, &h, STScnApiDX12Heap)) {
                SCN_PRINTF_ERROR("DX12, ScnApiDX12Heap_malloc::ScnArraySorted_addPtr(%u bytes) failed.\n", size);
            } else {
                SCN_PRINTF_INFO("DX12, ScnApiDX12Heap_malloc::(%u bytes) success at v-address: %llu.\n", size, h.iVirtualAddr);
                heap = nullptr; //consume
                r = (void*)h.iVirtualAddr;
                obj->heaps.iAddrSeq += size;
            }
        }
        //release (if not consumed)
        if (heap != nullptr) {
            heap->Release();
            heap = nullptr;
        }
    }
    ScnMutex_unlock(obj->heaps.mutex);
    return r;
}

void ScnApiDX12Heap_free(void* ptr, void* itfParam) {
    STScnApiDX12Device* dev = (STScnApiDX12Device*)itfParam;
    if (dev == NULL || dev->dev == nullptr || ptr == NULL) {
        return;
    }
    ScnMutex_lock(dev->heaps.mutex);
    {
        STScnApiDX12Heap srch;
        srch.iVirtualAddr = (ScnUI64)ptr;
        const ScnSI32 iFnd = ScnArraySorted_indexOf(&dev->heaps.arr, &srch);
        SCN_ASSERT(iFnd >= 0) //should be found
        if (iFnd >= 0) {
            //destroy heap
            STScnApiDX12Heap* h = &dev->heaps.arr.arr[iFnd];
            SCN_PRINTF_INFO("DX12, ScnApiDX12Heap_malloc::(%u bytes) success.\n", h->size);
            if (h->obj != nullptr) {
                h->obj->Release();
                h->obj = nullptr;
            }
            //remove record
            ScnArraySorted_removeItemAtIndex(&dev->heaps.arr, iFnd);
        }
    }
    ScnMutex_unlock(dev->heaps.mutex);
}
