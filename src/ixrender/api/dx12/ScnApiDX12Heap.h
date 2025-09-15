//
//  ScnApiDX12Heap.hpp
//  ixtli-render
//
//  Created by Marcos Ortega on 10/9/25.
//

#ifndef ScnApiDX12Heap_h
#define ScnApiDX12Heap_h

#ifdef __cplusplus
extern "C" {
#endif

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnContext.h"
#include "ixrender/core/ScnMemElastic.h"

#include <d3d12.h>

struct STScnApiDX12Device; //external

//ENScnApiDX12HeapType

typedef enum ENScnApiDX12HeapType {
    ENScnApiDX12HeapType_Buffers = 0,   //buffers
    ENScnApiDX12HeapType_Render,        //render target or depth stencil textures
    ENScnApiDX12HeapType_Textures,      //textures
    ENScnApiDX12HeapType_Samplers,      //samplers (textures)
    //
    ENScnApiDX12HeapType_Count
} ENScnApiDX12HeapType;

// STScnApiDX12HeapCfg

typedef struct STScnApiDX12HeapCfg {
    ScnMemElasticRef    mem;
    D3D12_HEAP_FLAGS    flags;
    struct STScnApiDX12Device* dev;
} STScnApiDX12HeapCfg;

void    ScnApiDX12HeapCfg_init(STScnApiDX12HeapCfg* obj);
void    ScnApiDX12HeapCfg_destroy(STScnApiDX12HeapCfg* obj);
ScnBOOL ScnApiDX12HeapCfg_prepare(STScnApiDX12HeapCfg* obj, ScnContextRef ctx, STScnMemElasticCfg* cfg, STScnMemBlockAllocItf* itf, void* itfParam);

// STScnApiDX12Heap

typedef struct STScnApiDX12Heap {
    ScnUI64         iVirtualAddr;
    ScnUI32         size;
    ID3D12Heap*     obj;
} STScnApiDX12Heap;

ScnSI32 ScnCompare_STScnApiDX12Heap(const void* data1, const void* data2, const ScnUI32 dataSz);

void* ScnApiDX12Heap_malloc(const ScnUI32 size, const char* dbgHintStr, void* itfParam);
void ScnApiDX12Heap_free(void* ptr, void* itfParam);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnApiDX12Heap_h */
