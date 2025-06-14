//
//  NBSize.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/6/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/2d/NBSize.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBMngrStructMaps.h"

//

STNBStructMapsRec NBSize_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBSize_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBSize_sharedStructMap);
	if(NBSize_sharedStructMap.map == NULL){
		STNBSize s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBSize);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addFloatM(map, s, width);
		NBStructMap_addFloatM(map, s, height);
		NBSize_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBSize_sharedStructMap);
	return NBSize_sharedStructMap.map;
}

//

STNBStructMapsRec NBSizeI_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBSizeI_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBSizeI_sharedStructMap);
	if(NBSizeI_sharedStructMap.map == NULL){
		STNBSizeI s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBSizeI);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addIntM(map, s, width);
		NBStructMap_addIntM(map, s, height);
		NBSizeI_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBSizeI_sharedStructMap);
	return NBSizeI_sharedStructMap.map;
}

//

STNBStructMapsRec NBSizeI16_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBSizeI16_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBSizeI16_sharedStructMap);
	if(NBSizeI16_sharedStructMap.map == NULL){
		STNBSizeI16 s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBSizeI16);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addIntM(map, s, width);
		NBStructMap_addIntM(map, s, height);
		NBSizeI16_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBSizeI16_sharedStructMap);
	return NBSizeI16_sharedStructMap.map;
}

//

BOOL NBCompare_NBSize(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBSize))
	if(dataSz == sizeof(STNBSize)){
		const STNBSize* o1 = (const STNBSize*)data1;
		const STNBSize* o2 = (const STNBSize*)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return (o1->width == o2->width && o1->height == o2->height) ? TRUE : FALSE;
			case ENCompareMode_Lower:
				return (o1->width < o2->width || (o1->width == o2->width && o1->height < o2->height)) ? TRUE : FALSE;
			case ENCompareMode_LowerOrEqual:
				return (o1->width < o2->width || (o1->width == o2->width && o1->height <= o2->height)) ? TRUE : FALSE;
			case ENCompareMode_Greater:
				return (o1->width > o2->width || (o1->width == o2->width && o1->height > o2->height)) ? TRUE : FALSE;
			case ENCompareMode_GreaterOrEqual:
				return (o1->width > o2->width || (o1->width == o2->width && o1->height >= o2->height)) ? TRUE : FALSE;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}

//

void NBSize_set(STNBSize* obj, const float width, const float height){
	obj->width	= width;
	obj->height	= height;
}


