//
//  NBRect.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/12/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBRange.h"
//
#include "nb/core/NBStruct.h"
#include "nb/core/NBStructMaps.h"
#include "nb/core/NBMngrStructMaps.h"

//STNBRange

BOOL NBCompare_STNBRange(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
    NBASSERT(dataSz == sizeof(STNBRange))
    if(dataSz == sizeof(STNBRange)){
        const STNBRange* d1 = (STNBRange*)data1;
        const STNBRange* d2 = (STNBRange*)data2;
        switch (mode) {
            case ENCompareMode_Equal: return d1->start == d2->start;
            case ENCompareMode_Lower: return d1->start < d2->start;
            case ENCompareMode_LowerOrEqual: return d1->start <= d2->start;
            case ENCompareMode_Greater: return d1->start > d2->start;
            case ENCompareMode_GreaterOrEqual: return d1->start >= d2->start;
            default: NBASSERT(FALSE) break;
        }
    }
    return FALSE;
}

// NBRange

STNBStructMapsRec NBRange_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRange_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBRange_sharedStructMap);
    if(NBRange_sharedStructMap.map == NULL){
        STNBRange s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRange);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addFloatM(map, s, start);
        NBStructMap_addFloatM(map, s, size);
        NBRange_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBRange_sharedStructMap);
    return NBRange_sharedStructMap.map;
}


//STNBRangeI

BOOL NBCompare_STNBRangeI(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
    NBASSERT(dataSz == sizeof(STNBRangeI))
    if(dataSz == sizeof(STNBRangeI)){
        const STNBRangeI* d1 = (STNBRangeI*)data1;
        const STNBRangeI* d2 = (STNBRangeI*)data2;
        switch (mode) {
            case ENCompareMode_Equal: return d1->start == d2->start;
            case ENCompareMode_Lower: return d1->start < d2->start;
            case ENCompareMode_LowerOrEqual: return d1->start <= d2->start;
            case ENCompareMode_Greater: return d1->start > d2->start;
            case ENCompareMode_GreaterOrEqual: return d1->start >= d2->start;
            default: NBASSERT(FALSE) break;
        }
    }
    return FALSE;
}

// NBRangeI

STNBStructMapsRec NBRangeI_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRangeI_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBRangeI_sharedStructMap);
    if(NBRangeI_sharedStructMap.map == NULL){
        STNBRangeI s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRangeI);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addIntM(map, s, start);
        NBStructMap_addIntM(map, s, size);
        NBRangeI_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBRangeI_sharedStructMap);
    return NBRangeI_sharedStructMap.map;
}


//STNBRangeU

BOOL NBCompare_STNBRangeU(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
    NBASSERT(dataSz == sizeof(STNBRangeU))
    if(dataSz == sizeof(STNBRangeU)){
        const STNBRangeU* d1 = (STNBRangeU*)data1;
        const STNBRangeU* d2 = (STNBRangeU*)data2;
        switch (mode) {
            case ENCompareMode_Equal: return d1->start == d2->start;
            case ENCompareMode_Lower: return d1->start < d2->start;
            case ENCompareMode_LowerOrEqual: return d1->start <= d2->start;
            case ENCompareMode_Greater: return d1->start > d2->start;
            case ENCompareMode_GreaterOrEqual: return d1->start >= d2->start;
            default: NBASSERT(FALSE) break;
        }
    }
    return FALSE;
}

// NBRangeU

STNBStructMapsRec NBRangeU_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRangeU_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBRangeU_sharedStructMap);
    if(NBRangeU_sharedStructMap.map == NULL){
        STNBRangeU s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRangeU);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, start);
        NBStructMap_addUIntM(map, s, size);
        NBRangeU_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBRangeU_sharedStructMap);
    return NBRangeU_sharedStructMap.map;
}
