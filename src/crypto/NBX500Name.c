//
//  NBX500Name.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/28/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/crypto/NBX500Name.h"
//
#include "nb/core/NBStruct.h"
#include "nb/core/NBMngrStructMaps.h"

STNBStructMapsRec NBX500NamePair_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBX500NamePair_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBX500NamePair_sharedStructMap);
	if(NBX500NamePair_sharedStructMap.map == NULL){
		STNBX500NamePair s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBX500NamePair);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addStrPtrM(map, s, type);
		NBStructMap_addStrPtrM(map, s, value);
		NBX500NamePair_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBX500NamePair_sharedStructMap);
	return NBX500NamePair_sharedStructMap.map;
}
