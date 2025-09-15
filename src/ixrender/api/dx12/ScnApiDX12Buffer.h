//
//  ScnApiDX12Buffer.h
//  ixtli-render
//
//  Created by Marcos Ortega on 10/9/25.
//

#ifndef ScnApiDX12Buffer_h
#define ScnApiDX12Buffer_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnContext.h"
#include "ixrender/api/ScnApiItf.h"
//
#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>
#import <TargetConditionals.h>  //for TARGET_OS_* macros

#ifdef __cplusplus
extern "C" {
#endif

//STScnApiDX12Buffer

typedef struct STScnApiDX12Buffer {
    ScnContextRef   ctx;
    STScnApiItf     itf;
    id<MTLDevice>   dev;
    id<MTLBuffer>   buff;
} STScnApiDX12Buffer;

ScnBOOL ScnApiDX12Buffer_syncRanges_(id<MTLBuffer> buff, ScnMemElasticRef mem, const STScnRangeU* const rngs, const ScnUI32 rngsUse);

void    ScnApiDX12Buffer_free(void* data);
ScnBOOL ScnApiDX12Buffer_sync(void* data, ScnMemElasticRef mem, const STScnGpuBufferChanges* changes);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnApiDX12Buffer_h */
