
#include "nb/NBFrameworkPch.h"
#include "nb/scene/NBScnTransform.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBStructMaps.h"
#include "nb/core/NBMngrStructMaps.h"

// NBScnTransform

STNBStructMapsRec NBScnTransform_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBScnTransform_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBScnTransform_sharedStructMap);
    if(NBScnTransform_sharedStructMap.map == NULL){
        STNBScnTransform s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBScnTransform);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addFloatM(map, s, sx);  //scale-x
        NBStructMap_addFloatM(map, s, sy);  //scale-y
        NBStructMap_addFloatM(map, s, tx);  //traslate-x
        NBStructMap_addFloatM(map, s, ty);  //traslate-y
        NBStructMap_addFloatM(map, s, deg); //rotaiton in degrees
        NBScnTransform_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBScnTransform_sharedStructMap);
    return NBScnTransform_sharedStructMap.map;
}
