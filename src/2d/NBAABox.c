//
//  NBAABox.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/6/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/2d/NBAABox.h"
#include "nb/core/NBMngrStructMaps.h"

STNBStructMapsRec NBAABox_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBAABox_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBAABox_sharedStructMap);
	if(NBAABox_sharedStructMap.map == NULL){
		STNBAABox s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBAABox);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addFloatM(map, s, xMin);
		NBStructMap_addFloatM(map, s, xMax);
		NBStructMap_addFloatM(map, s, yMin);
		NBStructMap_addFloatM(map, s, yMax);
		NBAABox_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBAABox_sharedStructMap);
	return NBAABox_sharedStructMap.map;
}

//

STNBStructMapsRec NBAABoxI_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBAABoxI_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBAABoxI_sharedStructMap);
	if(NBAABoxI_sharedStructMap.map == NULL){
		STNBAABoxI s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBAABoxI);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addIntM(map, s, xMin);
		NBStructMap_addIntM(map, s, xMax);
		NBStructMap_addIntM(map, s, yMin);
		NBStructMap_addIntM(map, s, yMax);
		NBAABoxI_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBAABoxI_sharedStructMap);
	return NBAABoxI_sharedStructMap.map;
}

//

BOOL NBCompare_NBAABox(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBAABox))
	if(dataSz == sizeof(STNBAABox)){
		const STNBAABox* o1 = (const STNBAABox*)data1;
		const STNBAABox* o2 = (const STNBAABox*)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return (o1->xMin == o2->xMin && o1->yMin == o2->yMin && o1->xMax == o2->xMax && o1->yMax == o2->yMax);
			case ENCompareMode_Lower:
				NBASSERT(FALSE)
				break;
			case ENCompareMode_LowerOrEqual:
				NBASSERT(FALSE)
				break;
			case ENCompareMode_Greater:
				NBASSERT(FALSE)
				break;
			case ENCompareMode_GreaterOrEqual:
				NBASSERT(FALSE)
				break;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}

void NBAABox_init(STNBAABox* obj){
	obj->xMin = -1.0;
	obj->xMax = -1.0;
	obj->yMin = -0.0;
	obj->yMax = -0.0;
}

BOOL NBAABox_isEmpty(const STNBAABox* obj){
	return obj->xMin == -1.0 && obj->xMax == -1.0 && obj->yMin==-0.0;
}

void NBAABox_wrapPoint(STNBAABox* obj, const STNBPoint p){
	if(NBAABox_isEmpty(obj)){
		obj->xMin = obj->xMax = p.x;
		obj->yMin = obj->yMax = p.y;
	} else {
		if(obj->xMin > p.x) obj->xMin = p.x;
		if(obj->yMin > p.y) obj->yMin = p.y;
		if(obj->xMax < p.x) obj->xMax = p.x;
		if(obj->yMax < p.y) obj->yMax = p.y;
	}
}

void NBAABox_wrapFirstPoint(STNBAABox* obj, const STNBPoint p){
	obj->xMin = obj->xMax = p.x;
	obj->yMin = obj->yMax = p.y;
}

void NBAABox_wrapNextPoint(STNBAABox* obj, const STNBPoint p){
	if(obj->xMin > p.x) obj->xMin = p.x;
	if(obj->yMin > p.y) obj->yMin = p.y;
	if(obj->xMax < p.x) obj->xMax = p.x;
	if(obj->yMax < p.y) obj->yMax = p.y;
}

void NBAABox_wrapPoints(STNBAABox* obj, const STNBPoint* arr, const UI32 arrSz){
	const STNBPoint* p = arr;
	const STNBPoint* pAfterLast = arr + arrSz;
	//First
	if(p < pAfterLast){
		if(NBAABox_isEmpty(obj)){
			obj->xMin = obj->xMax = p->x;
			obj->yMin = obj->yMax = p->y;
			p++;
		}
	}
	//Next (do not validate empty)
	while(p < pAfterLast){
		if(obj->xMin > p->x) obj->xMin = p->x;
		if(obj->yMin > p->y) obj->yMin = p->y;
		if(obj->xMax < p->x) obj->xMax = p->x;
		if(obj->yMax < p->y) obj->yMax = p->y;
		p++;
	}
}

//

/*

BOOL NBAABox_intersectsPoint(const STNBAABox* obj, const float P_X, const float P_Y){
	return !((P_X) < obj->xMin || (P_X) > obj->xMax || (P_Y) < obj->yMin || (P_Y) > obj->yMax) ? TRUE : FALSE;
}

BOOL NBAABox_intersectsAABox(const STNBAABox* obj, const STNBAABox* other){
	return !(other->xMax < obj->xMin || other->xMin > obj->xMax || other->yMax < obj->yMin || other->yMin > obj->yMax) ? TRUE : FALSE;
}

BOOL NBAABox_interiorIntersectsPoint(const STNBAABox* obj, const float P_X, const float P_Y){
	return (!NBAABox_isEmpty(obj) && !((P_X) <= obj->xMin || (P_X) >= obj->xMax || (P_Y) <= obj->yMin || (P_Y) >= obj->yMax)) ? TRUE: FALSE;
}

BOOL NBAABox_interiorIntersectsAABox(const STNBAABox* obj, const STNBAABox* other){
	return (!NBAABox_isEmpty(obj) && !(other->xMax <= obj->xMin || other->xMin >= obj->xMax || other->yMax <= obj->yMin || other->yMin >= obj->yMax)) ? TRUE: FALSE;
}

BOOL NBAABox_wrapsAABox(const STNBAABox* obj, const STNBAABox* wrapd){
	return (!NBAABox_isEmpty(obj) && wrapd->xMax <= obj->xMax && wrapd->xMin >= obj->xMin && wrapd->yMax <= obj->yMax && wrapd->yMin >= obj->yMin) ? TRUE : FALSE;
}

STNBSize NBAABox_size(const STNBAABox* obj){
	STNBSize r;
	r.width		= (obj->xMax - obj->xMin);
	r.height	= (obj->yMax - obj->yMin);
	return r;
}

void NBAABox_sizeTo(const STNBAABox* obj, STNBSize* dst){
	dst->width	= (obj->xMax - obj->xMin);
	dst->height	= (obj->yMax - obj->yMin);
}

STNBPoint NBAABox_center(const STNBAABox* obj){
	STNBPoint r;
	r.x		= obj->xMin + ((obj->xMax - obj->xMin) / 2.0f);
	r.y		= obj->yMin + ((obj->yMax - obj->yMin) / 2.0f);
	return r;
}

void NBAABox_centerTo(const STNBAABox* obj, STNBPoint* dst){
	dst->x	= obj->xMin + ((obj->xMax - obj->xMin) / 2.0f);
	dst->y	= obj->yMin + ((obj->yMax - obj->yMin) / 2.0f);
}

void NBAABox_translate(STNBAABox* obj, const float x, const float y){
	obj->xMin += x;
	obj->xMax += x;
	obj->yMin += y;
	obj->yMax += y;
}

void NBAABox_scale(STNBAABox* obj, const float wRel, const float hRel){
	obj->xMin *= wRel;
	obj->xMax *= wRel;
	obj->yMin *= hRel;
	obj->yMax *= hRel;
}



void NBAABox_wrapAABox(STNBAABox* obj, const STNBAABox* other){
	if(NBAABox_isEmpty(obj)){
		*obj = *other;
	} else {
		if(obj->xMin > other->xMin) obj->xMin = other->xMin;
		if(obj->yMin > other->yMin) obj->yMin = other->yMin;
		if(obj->xMax < other->xMax) obj->xMax = other->xMax;
		if(obj->yMax < other->yMax) obj->yMax = other->yMax;
	}
}

/ *#define NBCAJAAABB_ENVOLVER_CAJA_MUTLIPLICADA_POR_MATRIZ(CAJA_DEST_Y_BASE, CAJA_ENVOLVER, MATRIZ_X_CAJA_ENVOLVER)
{
STNBPoint punto;
NBMATRIZ_MULTIPLICAR_PUNTO(punto, MATRIZ_X_CAJA_ENVOLVER, CAJA_ENVOLVER.xMin, CAJA_ENVOLVER.yMin); NBCAJAAABB_ENVOLVER_PUNTO(CAJA_DEST_Y_BASE, punto.x, punto.y);
NBMATRIZ_MULTIPLICAR_PUNTO(punto, MATRIZ_X_CAJA_ENVOLVER, CAJA_ENVOLVER.xMin, CAJA_ENVOLVER.yMax); NBCAJAAABB_ENVOLVER_PUNTO(CAJA_DEST_Y_BASE, punto.x, punto.y);
NBMATRIZ_MULTIPLICAR_PUNTO(punto, MATRIZ_X_CAJA_ENVOLVER, CAJA_ENVOLVER.xMax, CAJA_ENVOLVER.yMin); NBCAJAAABB_ENVOLVER_PUNTO(CAJA_DEST_Y_BASE, punto.x, punto.y);
NBMATRIZ_MULTIPLICAR_PUNTO(punto, MATRIZ_X_CAJA_ENVOLVER, CAJA_ENVOLVER.xMax, CAJA_ENVOLVER.yMax); NBCAJAAABB_ENVOLVER_PUNTO(CAJA_DEST_Y_BASE, punto.x, punto.y);
}* /

void NBAABox_wrapRotatedAABox(STNBAABox* obj, const STNBAABox* other, const float rotX, const float rotY, const float rotDeg){
	STNBAABox tmp; STNBPoint pRot;
	tmp.xMin		= other->xMin - rotX;
	tmp.xMax		= other->xMax - rotX;
	tmp.yMin		= other->yMin - rotY;
	tmp.yMax		= other->yMax - rotY;
	NBPoint_set(&pRot, tmp.xMin, tmp.yMin); NBPUNTO_ROTAR(__puntoRotandoCaja, __puntoRotandoCaja, rotDeg); NBCAJAAABB_ENVOLVER_PUNTO(CAJA_DEST_Y_BASE, __puntoRotandoobj->x + rotX, __puntoRotandoobj->y + rotY);
	NBPoint_set(&pRot, tmp.xMin, tmp.yMax); NBPUNTO_ROTAR(__puntoRotandoCaja, __puntoRotandoCaja, rotDeg); NBCAJAAABB_ENVOLVER_PUNTO(CAJA_DEST_Y_BASE, __puntoRotandoobj->x + rotX, __puntoRotandoobj->y + rotY);
	NBPoint_set(&pRot, tmp.xMax, tmp.yMin); NBPUNTO_ROTAR(__puntoRotandoCaja, __puntoRotandoCaja, rotDeg); NBCAJAAABB_ENVOLVER_PUNTO(CAJA_DEST_Y_BASE, __puntoRotandoobj->x + rotX, __puntoRotandoobj->y + rotY);
	NBPoint_set(&pRot, tmp.xMax, tmp.yMax); NBPUNTO_ROTAR(__puntoRotandoCaja, __puntoRotandoCaja, rotDeg); NBCAJAAABB_ENVOLVER_PUNTO(CAJA_DEST_Y_BASE, __puntoRotandoobj->x + rotX, __puntoRotandoobj->y + rotY);

}
	
#define NBCAJAAABB_ENVOLVER_CAJA_ROTADA(CAJA_DEST_Y_BASE, CAJA_ENVOLVER, CEN_ROT_X, CEN_ROT_Y, GRADOS)
{
float rotDeg	= (GRADOS);\
float rotX	= (CEN_ROT_X);
float rotY	= (CEN_ROT_Y);
NBCajaAABBP<float> tmp; / *asumiendo valores float* /
tmp.xMin		= CAJA_ENVOLVER.xMin - rotX;
tmp.xMax		= CAJA_ENVOLVER.xMax - rotX;
tmp.yMin		= CAJA_ENVOLVER.yMin - rotY;
tmp.yMax		= CAJA_ENVOLVER.yMax - rotY;
NBPuntoP<float> __puntoRotandoCaja; / *asumiendo valores float* /
NBPUNTO_ESTABLECER(__puntoRotandoCaja, tmp.xMin, tmp.yMin); NBPUNTO_ROTAR(__puntoRotandoCaja, __puntoRotandoCaja, rotDeg); NBCAJAAABB_ENVOLVER_PUNTO(CAJA_DEST_Y_BASE, __puntoRotandoobj->x + rotX, __puntoRotandoobj->y + rotY);
NBPUNTO_ESTABLECER(__puntoRotandoCaja, tmp.xMin, tmp.yMax); NBPUNTO_ROTAR(__puntoRotandoCaja, __puntoRotandoCaja, rotDeg); NBCAJAAABB_ENVOLVER_PUNTO(CAJA_DEST_Y_BASE, __puntoRotandoobj->x + rotX, __puntoRotandoobj->y + rotY);
NBPUNTO_ESTABLECER(__puntoRotandoCaja, tmp.xMax, tmp.yMin); NBPUNTO_ROTAR(__puntoRotandoCaja, __puntoRotandoCaja, rotDeg); NBCAJAAABB_ENVOLVER_PUNTO(CAJA_DEST_Y_BASE, __puntoRotandoobj->x + rotX, __puntoRotandoobj->y + rotY);
NBPUNTO_ESTABLECER(__puntoRotandoCaja, tmp.xMax, tmp.yMax); NBPUNTO_ROTAR(__puntoRotandoCaja, __puntoRotandoCaja, rotDeg); NBCAJAAABB_ENVOLVER_PUNTO(CAJA_DEST_Y_BASE, __puntoRotandoobj->x + rotX, __puntoRotandoobj->y + rotY);
}

#define NBCAJAAABB_ENVOLVER_CAJA_INCLUYENDO_TODAS_SUS_ROTACIONES(CAJA_DEST_Y_BASE, CAJA_ROTAR_ENVOLVER, CEN_ROT_X, CEN_ROT_Y)
{
float __centMultiRotandoCajaX = (CEN_ROT_X); / *asumiendo valores float* /\
float __centMultiRotandoCajaY = (CEN_ROT_Y); / *asumiendo valores float* /\
NBCajaAABBP<float> __cajaCentradaMultiRotando; / *asumiendo valores float* /\
__cajaCentradaMultiRotando.xMin			= CAJA_ROTAR_ENVOLVER.xMin - __centMultiRotandoCajaX;
__cajaCentradaMultiRotando.xMax			= CAJA_ROTAR_ENVOLVER.xMax - __centMultiRotandoCajaX;
__cajaCentradaMultiRotando.yMin			= CAJA_ROTAR_ENVOLVER.yMin - __centMultiRotandoCajaY;
__cajaCentradaMultiRotando.yMax			= CAJA_ROTAR_ENVOLVER.yMax - __centMultiRotandoCajaY;
float __distanciaMaximaDelCentroMultiRotando = 0.0f;
if(__cajaCentradaMultiRotando.xMin>__distanciaMaximaDelCentroMultiRotando || -__cajaCentradaMultiRotando.xMin>__distanciaMaximaDelCentroMultiRotando){
__distanciaMaximaDelCentroMultiRotando = (__cajaCentradaMultiRotando.xMin<0.0f?-__cajaCentradaMultiRotando.xMin:__cajaCentradaMultiRotando.xMin);
}
if(__cajaCentradaMultiRotando.xMax>__distanciaMaximaDelCentroMultiRotando || -__cajaCentradaMultiRotando.xMax>__distanciaMaximaDelCentroMultiRotando){
__distanciaMaximaDelCentroMultiRotando = (__cajaCentradaMultiRotando.xMax<0.0f?-__cajaCentradaMultiRotando.xMax:__cajaCentradaMultiRotando.xMax);
}
if(__cajaCentradaMultiRotando.yMin>__distanciaMaximaDelCentroMultiRotando || -__cajaCentradaMultiRotando.yMin>__distanciaMaximaDelCentroMultiRotando){
__distanciaMaximaDelCentroMultiRotando = (__cajaCentradaMultiRotando.yMin<0.0f?-__cajaCentradaMultiRotando.yMin:__cajaCentradaMultiRotando.yMin);
}
if(__cajaCentradaMultiRotando.yMax>__distanciaMaximaDelCentroMultiRotando || -__cajaCentradaMultiRotando.yMax>__distanciaMaximaDelCentroMultiRotando){
__distanciaMaximaDelCentroMultiRotando = (__cajaCentradaMultiRotando.yMax<0.0f?-__cajaCentradaMultiRotando.yMax:__cajaCentradaMultiRotando.yMax);
}
float __distanciaDiagonalMultiRotando = NBPUNTO_DISTANCIA(-__distanciaMaximaDelCentroMultiRotando, -__distanciaMaximaDelCentroMultiRotando, __distanciaMaximaDelCentroMultiRotando, __distanciaMaximaDelCentroMultiRotando);
NBCAJAAABB_ENVOLVER_PUNTO(CAJA_DEST_Y_BASE, (__centMultiRotandoCajaX-(__distanciaDiagonalMultiRotando/2.0f)), (__centMultiRotandoCajaY-(__distanciaDiagonalMultiRotando/2.0f)));
NBCAJAAABB_ENVOLVER_PUNTO(CAJA_DEST_Y_BASE, (__centMultiRotandoCajaX+(__distanciaDiagonalMultiRotando/2.0f)), (__centMultiRotandoCajaY+(__distanciaDiagonalMultiRotando/2.0f)));
}

*/
