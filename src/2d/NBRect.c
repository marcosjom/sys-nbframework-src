//
//  NBRect.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/12/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/2d/NBRect.h"
#include "nb/core/NBMngrStructMaps.h"

STNBStructMapsRec NBRect_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRect_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBRect_sharedStructMap);
	if(NBRect_sharedStructMap.map == NULL){
		STNBRect s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRect);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addFloatM(map, s, x);
		NBStructMap_addFloatM(map, s, y);
		NBStructMap_addFloatM(map, s, width);
		NBStructMap_addFloatM(map, s, height);
		NBRect_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBRect_sharedStructMap);
	return NBRect_sharedStructMap.map;
}

//

STNBStructMapsRec NBRectI_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRectI_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBRectI_sharedStructMap);
	if(NBRectI_sharedStructMap.map == NULL){
		STNBRectI s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRectI);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addIntM(map, s, x);
		NBStructMap_addIntM(map, s, y);
		NBStructMap_addIntM(map, s, width);
		NBStructMap_addIntM(map, s, height);
		NBRectI_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBRectI_sharedStructMap);
	return NBRectI_sharedStructMap.map;
}

//

STNBStructMapsRec NBRectI16_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBRectI16_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBRectI16_sharedStructMap);
	if(NBRectI16_sharedStructMap.map == NULL){
		STNBRectI16 s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBRectI16);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addIntM(map, s, x);
		NBStructMap_addIntM(map, s, y);
		NBStructMap_addIntM(map, s, width);
		NBStructMap_addIntM(map, s, height);
		NBRectI16_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBRectI16_sharedStructMap);
	return NBRectI16_sharedStructMap.map;
}
