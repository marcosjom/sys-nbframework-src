//
//  XUCadena.c
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBLocaleCfg.h"
//
#include "nb/core/NBMngrStructMaps.h"
#include "nb/core/NBArraySorted.h"
//Pait

STNBStructMapsRec NBLocaleCfgItmPair_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBLocaleCfgItmPair_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBLocaleCfgItmPair_sharedStructMap);
	if(NBLocaleCfgItmPair_sharedStructMap.map == NULL){
		STNBLocaleCfgItmPair s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBLocaleCfgItmPair);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addStrPtrM(map, s, lng);
		NBStructMap_addStrPtrM(map, s, lcl);
		NBStructMap_addUIntM(map, s, revTime);
		NBStructMap_addStrPtrM(map, s, revName);
		NBLocaleCfgItmPair_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBLocaleCfgItmPair_sharedStructMap);
	return NBLocaleCfgItmPair_sharedStructMap.map;
}

BOOL NBCompare_STNBLocaleCfgItmPair(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBLocaleCfgItmPair))
	if(dataSz == sizeof(STNBLocaleCfgItmPair)){
		const STNBLocaleCfgItmPair* d1 = (const STNBLocaleCfgItmPair*)data1;
		const STNBLocaleCfgItmPair* d2 = (const STNBLocaleCfgItmPair*)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return NBString_strIsEqual(d1->lng, d2->lng);
			case ENCompareMode_Lower:
				return NBString_strIsLower(d1->lng, d2->lng);
			case ENCompareMode_LowerOrEqual:
				return NBString_strIsLowerOrEqual(d1->lng, d2->lng);
			case ENCompareMode_Greater:
				return NBString_strIsGreater(d1->lng, d2->lng);
			case ENCompareMode_GreaterOrEqual:
				return NBString_strIsGreaterOrEqual(d1->lng, d2->lng);
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}

//Itm

STNBStructMapsRec NBLocaleCfgItm_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBLocaleCfgItm_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBLocaleCfgItm_sharedStructMap);
	if(NBLocaleCfgItm_sharedStructMap.map == NULL){
		STNBLocaleCfgItm s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBLocaleCfgItm);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addStrPtrM(map, s, uid);
		NBStructMap_addStrPtrM(map, s, hint);
		NBStructMap_addPtrToArrayOfStructM(map, s, lcls, lclsSz, ENNBStructMapSign_Unsigned, NBLocaleCfgItmPair_getSharedStructMap());
		NBLocaleCfgItm_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBLocaleCfgItm_sharedStructMap);
	return NBLocaleCfgItm_sharedStructMap.map;
}

BOOL NBCompare_STNBLocaleCfgItm(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBLocaleCfgItm))
	if(dataSz == sizeof(STNBLocaleCfgItm)){
		const STNBLocaleCfgItm* d1 = (const STNBLocaleCfgItm*)data1;
		const STNBLocaleCfgItm* d2 = (const STNBLocaleCfgItm*)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return NBString_strIsEqual(d1->uid, d2->uid);
			case ENCompareMode_Lower:
				return NBString_strIsLower(d1->uid, d2->uid);
			case ENCompareMode_LowerOrEqual:
				return NBString_strIsLowerOrEqual(d1->uid, d2->uid);
			case ENCompareMode_Greater:
				return NBString_strIsGreater(d1->uid, d2->uid);
			case ENCompareMode_GreaterOrEqual:
				return NBString_strIsGreaterOrEqual(d1->uid, d2->uid);
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}

//Cfg

STNBStructMapsRec NBLocaleCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBLocaleCfg_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBLocaleCfg_sharedStructMap);
	if(NBLocaleCfg_sharedStructMap.map == NULL){
		STNBLocaleCfg s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBLocaleCfg);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addPtrToArrayOfStructM(map, s, itms, itmsSz, ENNBStructMapSign_Unsigned, NBLocaleCfgItm_getSharedStructMap());
		NBLocaleCfg_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBLocaleCfg_sharedStructMap);
	return NBLocaleCfg_sharedStructMap.map;
}
