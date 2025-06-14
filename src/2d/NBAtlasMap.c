//
//  NBAtlasMap.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/11/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/2d/NBSize.h"
#include "nb/2d/NBAABox.h"
#include "nb/2d/NBAtlasMap.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"

typedef struct STNBAtlasMapAreaPriv_ {
	STNBRectI	used;		//area used
	STNBRectI	reserved;	//area reserved
} STNBAtlasMapAreaPriv;

typedef struct STNBAtlasMapOpq_ {
	UI8			scaleBase2;
	STNBSizeI	size;
	STNBArray	areas;	//STNBAtlasMapAreaPriv
	UI32		retainCount;
} STNBAtlasMapOpq;

/*void NBAtlasMap_init(STNBAtlasMap* obj){
	NBAtlasMap_initWithSz(obj, 1, 0, 0);
}*/

void NBAtlasMap_initWithSz(STNBAtlasMap* obj, const SI32 scaleBase2, const SI32 width, const SI32 height){
	STNBAtlasMapOpq* opq	= obj->opaque = NBMemory_allocType(STNBAtlasMapOpq);
	NBMemory_setZeroSt(*opq, STNBAtlasMapOpq);
	opq->scaleBase2		= (UI8)scaleBase2;
	opq->size.width		= width;
	opq->size.height	= height;
	NBArray_init(&opq->areas, sizeof(STNBAtlasMapAreaPriv), NULL);
	opq->retainCount	= 1;
}

void NBAtlasMap_retain(STNBAtlasMap* obj){
	STNBAtlasMapOpq* opq	= (STNBAtlasMapOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount++;
}

void NBAtlasMap_release(STNBAtlasMap* obj){
	STNBAtlasMapOpq* opq	= (STNBAtlasMapOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount--;
	if(opq->retainCount == 0){
		opq->scaleBase2		= 0;
		opq->size.width		= 0;
		opq->size.height	= 0;
		NBArray_release(&opq->areas);
		//
		NBMemory_free(obj->opaque);
		obj->opaque = NULL;
	}
}

//Props

UI32 NBAtlasMap_getAreasCount(const STNBAtlasMap* obj){
	STNBAtlasMapOpq* opq	= (STNBAtlasMapOpq*)obj->opaque;
	return opq->areas.use;
}

BOOL NBAtlasMap_getAreasAABox(const STNBAtlasMap* obj, STNBAABox* dst){
	BOOL r = FALSE;
	STNBAtlasMapOpq* opq	= (STNBAtlasMapOpq*)obj->opaque;
	if (opq->areas.use > 0) {
		r = TRUE;
		STNBAtlasMapAreaPriv* a = NBArray_itmPtrAtIndex(&opq->areas, STNBAtlasMapAreaPriv, 0);
		NBAABox_wrapFirstPoint(dst, NBST_P(STNBPoint, a->used.x, a->used.y));
		NBAABox_wrapNextPoint(dst, NBST_P(STNBPoint, a->used.x + a->used.width, a->used.y + a->used.height));
		UI32 i; for(i = 1; i < opq->areas.use; i++){
			STNBAtlasMapAreaPriv* a = NBArray_itmPtrAtIndex(&opq->areas, STNBAtlasMapAreaPriv, i);
			NBAABox_wrapNextPoint(dst, NBST_P(STNBPoint, a->used.x, a->used.y));
			NBAABox_wrapNextPoint(dst, NBST_P(STNBPoint, a->used.x + a->used.width, a->used.y + a->used.height));
		}
	}
	return r;
}

BOOL NBAtlasMap_getSize(const STNBAtlasMap* obj, STNBSize* dst){
	STNBAtlasMapOpq* opq	= (STNBAtlasMapOpq*)obj->opaque;
	NBSize_set(dst, opq->size.width, opq->size.height);
	return  TRUE;
}

UI8  NBAtlasMap_getScaleBase2(const STNBAtlasMap* obj){
	STNBAtlasMapOpq* opq	= (STNBAtlasMapOpq*)obj->opaque;
	return opq->scaleBase2;
}

//Search

STNBRectI NBAtlasMap_getAreaAtIndex(const STNBAtlasMap* obj, const UI32 idx){
	STNBRectI r;
	STNBAtlasMapOpq* opq	= (STNBAtlasMapOpq*)obj->opaque;
	if(idx < opq->areas.use){
		STNBAtlasMapAreaPriv* a = NBArray_itmPtrAtIndex(&opq->areas, STNBAtlasMapAreaPriv, idx);
		r	= a->used;
	} else {
		r.x = r.y = r.width = r.height = 0;
	}
	return r;
}

//Search room

BOOL NBAtlasMap_boxIntersectsAnyReserved(const STNBAtlasMapOpq* opq, const STNBAABoxI box, SI32* dtsRemainRight, SI32* dstRemainBelow){
	BOOL r = FALSE;
	SI32 minRemRight	= (opq->size.width - box.xMax - 1);
	SI32 minRemBelow	= (opq->size.height - box.yMax - 1);
	if(box.xMin < 0 || box.yMin < 0 || box.xMax >= opq->size.width || box.yMax >= opq->size.height){
		r = TRUE; //Outside ranges
	} else {
		UI32 i; for(i = 0; i < opq->areas.use; i++){
			STNBAtlasMapAreaPriv* a = NBArray_itmPtrAtIndex(&opq->areas, STNBAtlasMapAreaPriv, i);
			const BOOL intersects = !(a->used.x > box.xMax || a->used.y > box.yMax || (a->used.x + a->used.width ) <= box.xMin || (a->used.y + a->used.height) <= box.yMin);
			if(intersects){
				r = TRUE;
			} else {
				const BOOL intersectsRightH = (a->used.x > box.xMax && !(a->used.y > box.yMax || (a->used.y + a->used.height - 1) < box.yMin)) ? TRUE : FALSE;
				if(intersectsRightH){
					NBASSERT(box.xMax < a->used.x)
					const SI32 remain = a->used.x - box.xMax - 1;
					if(minRemRight > remain){
						minRemRight = remain;
					}
				}
				const BOOL intersectsBelowV = (a->used.y > box.yMax && !(a->used.x > box.xMax || (a->used.x+a->used.width - 1) < box.xMin)) ? TRUE : FALSE;
				if(intersectsBelowV){
					NBASSERT(box.yMax < a->used.y)
					const SI32 remain = a->used.y - box.yMax - 1;
					if(minRemBelow > remain){
						minRemBelow = remain;
					}
				}
			}
		}
	}
	if(dtsRemainRight != NULL) *dtsRemainRight = minRemRight;
	if(dstRemainBelow != NULL) *dstRemainBelow = minRemBelow;
	return r;
}

STNBRectI NBAtlasMap_searchAvailableArea(const STNBAtlasMapOpq* opq, const SI32 width, const SI32 height){
	STNBRectI r; r.x = r.y = r.width = r.height = 0;
	if(width > 0 && height > 0){
		if(opq->areas.use == 0 && width <= opq->size.width && height <= opq->size.height){
			//es el primer espacio reservado en toda el area
			r.x			= 0;
			r.y			= 0;
			r.width		= width;
			r.height	= height;
		} else {
			//no es el primero, buscar espacio
			SI32 minRemain	= -1;
			UI32 i; const UI32 count = opq->areas.use;
			for(i = 0; i < count; i++){
				STNBAtlasMapAreaPriv* a = NBArray_itmPtrAtIndex(&opq->areas, STNBAtlasMapAreaPriv, i);
				const STNBRectI areaOcupied = a->reserved;
				SI32 remainRight, remainBelow;
				STNBAABoxI boxTry;
				//Search right/below
				boxTry.xMin	= (areaOcupied.x + areaOcupied.width);
				boxTry.yMin	= (areaOcupied.y);
				boxTry.xMax	= boxTry.xMin + width - 1;
				boxTry.yMax	= boxTry.yMin + height - 1;
				if(!NBAtlasMap_boxIntersectsAnyReserved(opq, boxTry, &remainRight, &remainBelow)){
					//PRINTF_INFO("Remain right: %d (max %d)\n", remainRight, maxRemain);
					if(remainRight >= 0 && (minRemain == -1 || minRemain < remainRight)){
						r.x			= boxTry.xMin;
						r.y			= boxTry.yMin;
						r.width		= width;
						r.height	= height;
						minRemain	= remainRight;
						NBASSERT(r.x >= 0 && r.y >= 0 && (r.x + r.width) <= opq->size.width && (r.y + r.height) <= opq->size.height)
						break;
					}
				}
				//Search right/top
				boxTry.xMin	= (areaOcupied.x + areaOcupied.width);
				boxTry.yMin	= (areaOcupied.y + areaOcupied.height - height);
				boxTry.xMax	= boxTry.xMin + width - 1;
				boxTry.yMax	= boxTry.yMin + height - 1;
				if(!NBAtlasMap_boxIntersectsAnyReserved(opq, boxTry, &remainRight, &remainBelow)){
					//PRINTF_INFO("Remain right: %d (max %d)\n", remainRight, maxRemain);
					if(remainRight >= 0 && (minRemain == -1 || minRemain < remainRight)){
						r.x			= boxTry.xMin;
						r.y			= boxTry.yMin;
						r.width		= width;
						r.height	= height;
						minRemain	= remainRight;
						NBASSERT(r.x >= 0 && r.y >= 0 && (r.x + r.width) <= opq->size.width && (r.y + r.height) <= opq->size.height)
						break;
					}
				}
				//Search left/below
				boxTry.xMin	= (areaOcupied.x - width);
				boxTry.yMin	= (areaOcupied.y);
				boxTry.xMax	= boxTry.xMin + width - 1;
				boxTry.yMax	= boxTry.yMin + height - 1;
				if(!NBAtlasMap_boxIntersectsAnyReserved(opq, boxTry, &remainRight, &remainBelow)){
					//PRINTF_INFO("Remain right: %d (max %d)\n", remainRight, maxRemain);
					if(remainRight >= 0 && (minRemain == -1 || minRemain < remainRight)){
						r.x			= boxTry.xMin;
						r.y			= boxTry.yMin;
						r.width		= width;
						r.height	= height;
						minRemain	= remainRight;
						NBASSERT(r.x >= 0 && r.y >= 0 && (r.x + r.width) <= opq->size.width && (r.y + r.height) <= opq->size.height)
						break;
					}
				}
				//Search left/top
				boxTry.xMin	= (areaOcupied.x - width);
				boxTry.yMin	= (areaOcupied.y + areaOcupied.height - height);
				boxTry.xMax	= boxTry.xMin + width - 1;
				boxTry.yMax	= boxTry.yMin + height - 1;
				if(!NBAtlasMap_boxIntersectsAnyReserved(opq, boxTry, &remainRight, &remainBelow)){
					//PRINTF_INFO("Remain right: %d (max %d)\n", remainRight, maxRemain);
					if(remainRight >= 0 && (minRemain == -1 || minRemain < remainRight)){
						r.x			= boxTry.xMin;
						r.y			= boxTry.yMin;
						r.width		= width;
						r.height	= height;
						minRemain	= remainRight;
						NBASSERT(r.x >= 0 && r.y >= 0 && (r.x + r.width) <= opq->size.width && (r.y + r.height) <= opq->size.height)
						break;
					}
				}
				//Search below/right
				boxTry.xMin	= (areaOcupied.x);
				boxTry.yMin	= (areaOcupied.y + areaOcupied.height);
				boxTry.xMax	= boxTry.xMin + width - 1;
				boxTry.yMax	= boxTry.yMin + height - 1;
				if(!NBAtlasMap_boxIntersectsAnyReserved(opq, boxTry, &remainRight, &remainBelow)){
					//PRINTF_INFO("Sobrante debajo: %d (max %d)\n", remainBelow, sobranteMaximo);
					if(remainBelow >= 0 && (minRemain == -1 || minRemain < remainBelow)){
						r.x			= boxTry.xMin;
						r.y			= boxTry.yMin;
						r.width		= width;
						r.height	= height;
						minRemain	= remainBelow;
						NBASSERT(r.x >= 0 && r.y >= 0 && (r.x + r.width) <= opq->size.width && (r.y + r.height) <= opq->size.height)
						break;
					}
				}
				//Search below/left
				boxTry.xMin	= (areaOcupied.x + areaOcupied.width - width);
				boxTry.yMin	= (areaOcupied.y + areaOcupied.height);
				boxTry.xMax	= boxTry.xMin + width - 1;
				boxTry.yMax	= boxTry.yMin + height - 1;
				if(!NBAtlasMap_boxIntersectsAnyReserved(opq, boxTry, &remainRight, &remainBelow)){
					//PRINTF_INFO("Sobrante debajo: %d (max %d)\n", remainBelow, sobranteMaximo);
					if(remainBelow >= 0 && (minRemain == -1 || minRemain < remainBelow)){
						r.x			= boxTry.xMin;
						r.y			= boxTry.yMin;
						r.width		= width;
						r.height	= height;
						minRemain	= remainBelow;
						NBASSERT(r.x >= 0 && r.y >= 0 && (r.x + r.width) <= opq->size.width && (r.y + r.height) <= opq->size.height)
						break;
					}
				}
				//Search top/right
				boxTry.xMin	= (areaOcupied.x);
				boxTry.yMin	= (areaOcupied.y - height);
				boxTry.xMax	= boxTry.xMin + width - 1;
				boxTry.yMax	= boxTry.yMin + height - 1;
				if(!NBAtlasMap_boxIntersectsAnyReserved(opq, boxTry, &remainRight, &remainBelow)){
					//PRINTF_INFO("Sobrante debajo: %d (max %d)\n", remainBelow, sobranteMaximo);
					if(remainBelow >= 0 && (minRemain == -1 || minRemain < remainBelow)){
						r.x			= boxTry.xMin;
						r.y			= boxTry.yMin;
						r.width		= width;
						r.height	= height;
						minRemain	= remainBelow;
						NBASSERT(r.x >= 0 && r.y >= 0 && (r.x + r.width) <= opq->size.width && (r.y + r.height) <= opq->size.height)
						break;
					}
				}
				//Search top/left
				boxTry.xMin	= (areaOcupied.x + areaOcupied.width - width);
				boxTry.yMin	= (areaOcupied.y - height);
				boxTry.xMax	= boxTry.xMin + width - 1;
				boxTry.yMax	= boxTry.yMin + height - 1;
				if(!NBAtlasMap_boxIntersectsAnyReserved(opq, boxTry, &remainRight, &remainBelow)){
					//PRINTF_INFO("Sobrante debajo: %d (max %d)\n", remainBelow, sobranteMaximo);
					if(remainBelow >= 0 && (minRemain == -1 || minRemain < remainBelow)){
						r.x			= boxTry.xMin;
						r.y			= boxTry.yMin;
						r.width		= width;
						r.height	= height;
						minRemain	= remainBelow;
						NBASSERT(r.x >= 0 && r.y >= 0 && (r.x + r.width) <= opq->size.width && (r.y + r.height) <= opq->size.height)
						break;
					}
				}
			}
		}
	}
	return r;
}

BOOL NBAtlasMap_hasRoomFor(const STNBAtlasMap* obj, const SI32 width, const SI32 height, const SI32 hMargin, const SI32 vMargin){
	BOOL r = FALSE;
	STNBAtlasMapOpq* opq	= (STNBAtlasMapOpq*)obj->opaque;
	if(width == 0 && height == 0){
		r = TRUE;
	} else {
		const BOOL isEmpty = (opq->areas.use == 0 ? TRUE : FALSE);
		const STNBRectI rect = NBAtlasMap_searchAvailableArea(opq, width + (isEmpty ? 0 : hMargin), height + (isEmpty ? 0 : vMargin));
		r = (rect.width > 0 && rect.height > 0 ? TRUE : FALSE);
	}
	return r;
}

//Build

void NBAtlasMap_empty(STNBAtlasMap* obj){
	STNBAtlasMapOpq* opq	= (STNBAtlasMapOpq*)obj->opaque;
	NBArray_empty(&opq->areas);
}

void NBAtlasMap_setSize(STNBAtlasMap* obj, const SI32 width, const SI32 height){
	STNBAtlasMapOpq* opq	= (STNBAtlasMapOpq*)obj->opaque;
	opq->size.width		= width;
	opq->size.height	= height;
}

void NBAtlasMap_addArea(STNBAtlasMap* obj, const SI32 x, const SI32 y, const SI32 width, const SI32 height){
	STNBAtlasMapOpq* opq	= (STNBAtlasMapOpq*)obj->opaque;
	STNBAtlasMapAreaPriv a;
	a.used.x		= x;
	a.used.y		= y;
	a.used.width	= width;
	a.used.height	= height;
	a.reserved	= a.used;
	NBArray_addValue(&opq->areas, a);
}

BOOL NBAtlasMap_setAreaAtIndex(STNBAtlasMap* obj, const UI32 idx, const SI32 x, const SI32 y, const SI32 width, const SI32 height){
	BOOL r = FALSE;
	STNBAtlasMapOpq* opq	= (STNBAtlasMapOpq*)obj->opaque;
	if(idx < opq->areas.use){
		STNBAtlasMapAreaPriv* a = NBArray_itmPtrAtIndex(&opq->areas, STNBAtlasMapAreaPriv, idx);
		a->used.x		= x;
		a->used.y		= y;
		a->used.width	= width;
		a->used.height	= height;
		a->reserved	= a->used;
		r = TRUE;
	}
	return r;
}

STNBRectI NBAtlasMap_reserveArea(STNBAtlasMap* obj, const SI32 width, const SI32 height, const SI32 hMargin, const SI32 vMargin){
	STNBRectI r;
	STNBAtlasMapOpq* opq	= (STNBAtlasMapOpq*)obj->opaque;
	r.x = r.y = r.width = r.height = 0;
	if(width == 0 && height == 0){
		r.width		= width;
		r.height	= height;
	} else {
		const BOOL isEmpty = (opq->areas.use == 0 ? TRUE : FALSE);
		r = NBAtlasMap_searchAvailableArea(opq, width + (isEmpty ? 0 : hMargin), height + (isEmpty ? 0 : vMargin));
		NBASSERT(r.x >= 0 && r.y >= 0 && (r.x + r.width) <= opq->size.width && (r.y + r.height) <= opq->size.height)
	}
	if(r.width >= width && r.height >= height){
		STNBAtlasMapAreaPriv a;
		a.used.x		= r.x;
		a.used.y		= r.y;
		a.used.width	= width;
		a.used.height	= height;
		a.reserved		= r;
		NBArray_addValue(&opq->areas, a);
		//Return without margin area
		r.width			= width;
		r.height		= height;
	}
	return r;
}

UI32 NBAtlasMap_removeArea(STNBAtlasMap* obj, const SI32 x, const SI32 y, const SI32 width, const SI32 height){
	UI32 r = 0;
	STNBAtlasMapOpq* opq	= (STNBAtlasMapOpq*)obj->opaque;
	const SI32 xNextLast = (x + width);
	const SI32 yNextLast = (y + height);
	SI32 i; for(i = (SI32)opq->areas.use - 1; i >= 0; i--){
		STNBAtlasMapAreaPriv* a = NBArray_itmPtrAtIndex(&opq->areas, STNBAtlasMapAreaPriv, i);
		const BOOL intersects = !(a->used.x > xNextLast || a->used.y > yNextLast || (a->used.x + a->used.width) <= x || (a->used.y + a->used.height) <= y);
		if(intersects){
			NBArray_removeItemAtIndex(&opq->areas, i);
			r++;
		}
	}
	return r;
}

BOOL NBAtlasMap_removeAreaAtIndex(STNBAtlasMap* obj, const UI32 idx){
	BOOL r = FALSE;
	STNBAtlasMapOpq* opq	= (STNBAtlasMapOpq*)obj->opaque;
	if(idx < opq->areas.use){
		NBArray_removeItemAtIndex(&opq->areas, idx);
		r = TRUE;
	}
	return r;
}
