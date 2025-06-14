//
//  NBFontCodesMap.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/12/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/fonts/NBFontCodesMap.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBCompare.h"
#include "nb/core/NBArraySorted.h"

typedef struct STNBFontCodesMapRec_ {
	UI32	unicode;
	UI32	shapeId;
} STNBFontCodesMapRec;

BOOL NBCompare_NBFontCodesMapRec(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBFontCodesMapRec))
	if(dataSz == sizeof(STNBFontCodesMapRec)){
		const STNBFontCodesMapRec* o1 = (const STNBFontCodesMapRec*)data1;
		const STNBFontCodesMapRec* o2 = (const STNBFontCodesMapRec*)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return o1->unicode == o2->unicode ? TRUE : FALSE;
			case ENCompareMode_Lower:
				return o1->unicode < o2->unicode ? TRUE : FALSE;
			case ENCompareMode_LowerOrEqual:
				return o1->unicode <= o2->unicode ? TRUE : FALSE;
			case ENCompareMode_Greater:
				return o1->unicode > o2->unicode ? TRUE : FALSE;
			case ENCompareMode_GreaterOrEqual:
				return o1->unicode >= o2->unicode ? TRUE : FALSE;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}

typedef struct STNBFontCodesMapOpq_ {
	STNBArraySorted map;		//STNBFontCodesMapRec, unicode to shapes
	UI32			retainCount;
} STNBFontCodesMapOpq;

void NBFontCodesMap_init(STNBFontCodesMap* obj){
	STNBFontCodesMapOpq* opq = obj->opaque = NBMemory_allocType(STNBFontCodesMapOpq);
	NBMemory_setZeroSt(*opq, STNBFontCodesMapOpq);
	NBArraySorted_init(&opq->map, sizeof(STNBFontCodesMapRec), NBCompare_NBFontCodesMapRec);
	opq->retainCount	= 1;
}

void NBFontCodesMap_retain(STNBFontCodesMap* obj){
	STNBFontCodesMapOpq* opq = (STNBFontCodesMapOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount++;
}

void NBFontCodesMap_release(STNBFontCodesMap* obj){
	STNBFontCodesMapOpq* opq = (STNBFontCodesMapOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount--;
	if(opq->retainCount == 0){
		NBArraySorted_release(&opq->map);
	}
}

BOOL NBFontCodesMap_getShapeId(const STNBFontCodesMap* obj, const UI32 unicode, UI32* dstShapeId){
	BOOL r = FALSE;
	STNBFontCodesMapOpq* opq = (STNBFontCodesMapOpq*)obj->opaque;
	STNBFontCodesMapRec srch;
	srch.unicode	= unicode;
	const SI32 idx	= NBArraySorted_indexOf(&opq->map, &srch, sizeof(srch), NULL);
	if(idx != -1){
		const STNBFontCodesMapRec* rr = NBArraySorted_itmPtrAtIndex(&opq->map, STNBFontCodesMapRec, idx);
		if(dstShapeId != NULL) *dstShapeId = rr->shapeId;
		r = TRUE;
	}
	return r;
}

void NBFontCodesMap_addRecord(STNBFontCodesMap* obj, const UI32 unicode, const UI32 shapeId){
	STNBFontCodesMapOpq* opq = (STNBFontCodesMapOpq*)obj->opaque;
	STNBFontCodesMapRec srch;
	srch.unicode	= unicode;
	srch.shapeId	= shapeId;
	NBArraySorted_add(&opq->map, &srch, sizeof(srch));
}
