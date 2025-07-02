
#include "nb/NBFrameworkPch.h"
#include "nb/scene/NBScnNodeProps.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBStructMaps.h"
#include "nb/core/NBMngrStructMaps.h"

// NBScnNodeProps

STNBStructMapsRec NBScnNodeProps_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBScnNodeProps_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBScnNodeProps_sharedStructMap);
    if(NBScnNodeProps_sharedStructMap.map == NULL){
        STNBScnNodeProps s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBScnNodeProps);
        NBStructMap_init(map, sizeof(s));
        //NBStructMap_addBoolM(map, s, isVisible);
        NBStructMap_addStructM(map, s, c8, NBColor_getSharedStructMap());
        NBStructMap_addStructM(map, s, tform, NBScnTransform_getSharedStructMap());
        NBScnNodeProps_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBScnNodeProps_sharedStructMap);
    return NBScnNodeProps_sharedStructMap.map;
}
