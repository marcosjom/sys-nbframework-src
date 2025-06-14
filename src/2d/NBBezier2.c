//
//  NBBezier2.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/6/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/2d/NBBezier2.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBMngrStructMaps.h"

STNBStructMapsRec NBBezier2_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBBezier2_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBBezier2_sharedStructMap);
	if(NBBezier2_sharedStructMap.map == NULL){
		STNBBezier2 s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBBezier2);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addStructM(map, s, start, NBPoint_getSharedStructMap());
		NBStructMap_addStructM(map, s, ref, NBPoint_getSharedStructMap());
		NBStructMap_addStructM(map, s, end, NBPoint_getSharedStructMap());
		NBBezier2_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBBezier2_sharedStructMap);
	return NBBezier2_sharedStructMap.map;
}

//

STNBPoint NBBezier2_getMidPoint(const STNBBezier2* obj, const float relPos){
	const float t			= relPos;
	const float oneMinusT	= 1.0f - t;
	const float	ta			= oneMinusT * oneMinusT;
	const float tb			= 2.0f * oneMinusT * t;
	const float tc			= t * t;
	return (STNBPoint){
		(ta * obj->start.x) + (tb * obj->ref.x) + (tc * obj->end.x)
		, (ta * obj->start.y) + (tb * obj->ref.y) + (tc * obj->end.y)
	};
}

float NBBezier2_length(const STNBBezier2* obj){
	float r = 0.0f;
	float A, B, C, Sabc, A_2, A_32, C_2, BA;
	STNBPoint a, b;
	a.x		= obj->start.x - 2 * obj->ref.x + obj->end.x;
	a.y		= obj->start.y - 2 * obj->ref.y + obj->end.y;
	b.x		= 2 * obj->ref.x - 2 * obj->start.x;
	b.y		= 2 * obj->ref.y - 2 * obj->start.y;
	//
	A		= 4*(a.x*a.x + a.y*a.y);
	B		= 4*(a.x*b.x + a.y*b.y);
	C		= b.x*b.x + b.y*b.y;
	//
	Sabc	= 2 * sqrt(A + B + C);
	A_2		= sqrt(A);
	A_32	= 2 * A * A_2;
	C_2		= 2 * sqrt(C);
	BA		= B / A_2;
	//
	r		= (A_32 * Sabc + A_2 * B * (Sabc - C_2) + (4 * C * A - B * B) * log( (2 * A_2 + BA + Sabc) / (BA + C_2))) / (4 * A_32);
	return r;
}

//

STNBPoint NBBezier2_bzGetMidPoint(const STNBPoint start, const STNBPoint ref, const STNBPoint end, const float relPos){
	const float t			= relPos;
	const float oneMinusT	= 1.0f - t;
	const float	ta			= oneMinusT * oneMinusT;
	const float tb			= 2.0f * oneMinusT * t;
	const float tc			= t * t;
	return (STNBPoint){
		(ta * start.x) + (tb * ref.x) + (tc * end.x)
		, (ta * start.y) + (tb * ref.y) + (tc * end.y)
	};
}

float NBBezier2_bzLength(const STNBPoint start, const STNBPoint ref, const STNBPoint end){
	float r = 0.0f;
	float A, B, C, Sabc, A_2, A_32, C_2, BA;
	STNBPoint a, b;
	a.x		= start.x - 2 * ref.x + end.x;
	a.y		= start.y - 2 * ref.y + end.y;
	b.x		= 2 * ref.x - 2 * start.x;
	b.y		= 2 * ref.y - 2 * start.y;
	//
	A		= 4*(a.x*a.x + a.y*a.y);
	B		= 4*(a.x*b.x + a.y*b.y);
	C		= b.x*b.x + b.y*b.y;
	//
	Sabc	= 2 * sqrt(A + B + C);
	A_2		= sqrt(A);
	A_32	= 2 * A * A_2;
	C_2		= 2 * sqrt(C);
	BA		= B / A_2;
	//
	r		= (A_32 * Sabc + A_2 * B * (Sabc - C_2) + (4 * C * A - B * B) * log( (2 * A_2 + BA + Sabc) / (BA + C_2))) / (4 * A_32);
	return r;
}


