//
//  NBColor.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/12/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/2d/NBColor.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBMngrStructMaps.h"

STNBStructMapsRec NBColor_sharedStructMap = STNBStructMapsRec_empty;
STNBStructMapsRec NBColor8_sharedStructMap = STNBStructMapsRec_empty;
STNBStructMapsRec NBColorI_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBColor_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBColor_sharedStructMap);
	if(NBColor_sharedStructMap.map == NULL){
		STNBColor s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBColor);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addFloatM(map, s, r);
		NBStructMap_addFloatM(map, s, g);
		NBStructMap_addFloatM(map, s, b);
		NBStructMap_addFloatM(map, s, a);
		NBColor_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBColor_sharedStructMap);
	return NBColor_sharedStructMap.map;
}

const STNBStructMap* NBColor8_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBColor8_sharedStructMap);
	if(NBColor8_sharedStructMap.map == NULL){
		STNBColor8 s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBColor8);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addUIntM(map, s, r);
		NBStructMap_addUIntM(map, s, g);
		NBStructMap_addUIntM(map, s, b);
		NBStructMap_addUIntM(map, s, a);
		NBColor8_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBColor8_sharedStructMap);
	return NBColor8_sharedStructMap.map;
}

const STNBStructMap* NBColorI_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBColorI_sharedStructMap);
	if(NBColorI_sharedStructMap.map == NULL){
		STNBColorI s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBColorI);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addIntM(map, s, r);
		NBStructMap_addIntM(map, s, g);
		NBStructMap_addIntM(map, s, b);
		NBStructMap_addIntM(map, s, a);
		NBColorI_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBColorI_sharedStructMap);
	return NBColorI_sharedStructMap.map;
}

BOOL NBCompare_NBColor8(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBColor8))
	NBASSERT(sizeof(STNBColor8) == sizeof(UI32))
	if(dataSz == sizeof(STNBColor8)){
		const UI32* o1 = (const UI32*)data1;
		const UI32* o2 = (const UI32*)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return *o1 == *o2 ? TRUE : FALSE;
			case ENCompareMode_Lower:
				return *o1 < *o2 ? TRUE : FALSE;
			case ENCompareMode_LowerOrEqual:
				return *o1 <= *o2 ? TRUE : FALSE;
			case ENCompareMode_Greater:
				return *o1 > *o2 ? TRUE : FALSE;
			case ENCompareMode_GreaterOrEqual:
				return *o1 >= *o2 ? TRUE : FALSE;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}
