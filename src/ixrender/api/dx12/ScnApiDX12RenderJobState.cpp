//
//  ScnApiDX12RenderJobState.m
//  ixtli-render
//
//  Created by Marcos Ortega on 10/9/25.
//

#include "ScnApiDX12RenderJobState.h"


// STScnApiDX12RenderJobState

void ScnApiDX12RenderJobState_init(STScnApiDX12RenderJobState* obj){
    ScnMemory_setZeroSt(*obj);
}

void ScnApiDX12RenderJobState_destroy(STScnApiDX12RenderJobState* obj){
    if(obj->rndrDesc != nil){ [obj->rndrDesc release]; obj->rndrDesc = nil; }
    if(obj->rndrEnc != nil){ [obj->rndrEnc release]; obj->rndrEnc = nil; }
}

void ScnApiDX12RenderJobState_reset(STScnApiDX12RenderJobState* obj){
    if(obj->rndrDesc != nil){ [obj->rndrDesc release]; obj->rndrDesc = nil; }
    if(obj->rndrEnc != nil){ [obj->rndrEnc release]; obj->rndrEnc = nil; }
    ScnMemory_setZeroSt(*obj);
}
