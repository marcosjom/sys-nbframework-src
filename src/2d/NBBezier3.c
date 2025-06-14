//
//  NBBezier3.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/6/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/2d/NBBezier3.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBMngrStructMaps.h"

STNBStructMapsRec NBBezier3_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBBezier3_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBBezier3_sharedStructMap);
	if(NBBezier3_sharedStructMap.map == NULL){
		STNBBezier3 s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBBezier3);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addStructM(map, s, start, NBPoint_getSharedStructMap());
		NBStructMap_addStructM(map, s, ref1, NBPoint_getSharedStructMap());
		NBStructMap_addStructM(map, s, ref2, NBPoint_getSharedStructMap());
		NBStructMap_addStructM(map, s, end, NBPoint_getSharedStructMap());
		NBBezier3_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBBezier3_sharedStructMap);
	return NBBezier3_sharedStructMap.map;
}

//

STNBPoint NBBezier3_getMidPoint(const STNBBezier3* obj, const float relPos){
	const float t			= relPos;
	const float unoMenosT	= 1.0f - t;
	const float ta			= unoMenosT * unoMenosT * unoMenosT;
	const float tb			= 3.0f * t * unoMenosT * unoMenosT;
	const float tc			= 3.0f * t * t * unoMenosT;
	const float td			= t * t * t;
	return (STNBPoint){
		(ta * obj->start.x) + (tb * obj->ref1.x) + (tc * obj->ref2.x) + (td * obj->end.x)
		, (ta * obj->start.y) + (tb * obj->ref1.y) + (tc * obj->ref2.y) + (td * obj->end.y)
	};
}

//

STNBPoint NBBezier3_bezGetMidPoint(const STNBPoint start, const STNBPoint ref1, const STNBPoint ref2, const STNBPoint end, const float relPos){
	const float t			= relPos;
	const float unoMenosT	= 1.0f - t;
	const float ta			= unoMenosT * unoMenosT * unoMenosT;
	const float tb			= 3.0f * t * unoMenosT * unoMenosT;
	const float tc			= 3.0f * t * t * unoMenosT;
	const float td			= t * t * t;
	return (STNBPoint){
		(ta * start.x) + (tb * ref1.x) + (tc * ref2.x) + (td * end.x)
		, (ta * start.y) + (tb * ref1.y) + (tc * ref2.y) + (td * end.y)
	};
}
