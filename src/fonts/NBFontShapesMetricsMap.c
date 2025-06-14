
//
//  NBFontShapesMetricsMap.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/12/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/fonts/NBFontShapesMetricsMap.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBCompare.h"
#include "nb/core/NBArraySorted.h"

typedef struct STNBFontShapesMetricsMapRec_ {
	UI32					shapeId;
	STNBFontShapeMetrics	metrics;
} STNBFontShapesMetricsMapRec;

BOOL NBCompare_NBFontShapesMetricsMapRec(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBFontShapesMetricsMapRec))
	if(dataSz == sizeof(STNBFontShapesMetricsMapRec)){
		const STNBFontShapesMetricsMapRec* o1 = (const STNBFontShapesMetricsMapRec*)data1;
		const STNBFontShapesMetricsMapRec* o2 = (const STNBFontShapesMetricsMapRec*)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return o1->shapeId == o2->shapeId ? TRUE : FALSE;
			case ENCompareMode_Lower:
				return o1->shapeId < o2->shapeId ? TRUE : FALSE;
			case ENCompareMode_LowerOrEqual:
				return o1->shapeId <= o2->shapeId ? TRUE : FALSE;
			case ENCompareMode_Greater:
				return o1->shapeId > o2->shapeId ? TRUE : FALSE;
			case ENCompareMode_GreaterOrEqual:
				return o1->shapeId >= o2->shapeId ? TRUE : FALSE;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}

typedef struct STNBFontShapesMetricsMapOpq_ {
	STNBArraySorted map;		//STNBFontShapesMetricsMapRec, shapes
	UI32			retainCount;
} STNBFontShapesMetricsMapOpq;

void NBFontShapesMetricsMap_init(STNBFontShapesMetricsMap* obj){
	STNBFontShapesMetricsMapOpq* opq = obj->opaque = NBMemory_allocType(STNBFontShapesMetricsMapOpq);
	NBMemory_setZeroSt(*opq, STNBFontShapesMetricsMapOpq);
	NBArraySorted_init(&opq->map, sizeof(STNBFontShapesMetricsMapRec), NBCompare_NBFontShapesMetricsMapRec);
	opq->retainCount = 1;
}

void NBFontShapesMetricsMap_retain(STNBFontShapesMetricsMap* obj){
	STNBFontShapesMetricsMapOpq* opq = (STNBFontShapesMetricsMapOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount++;
}

void NBFontShapesMetricsMap_release(STNBFontShapesMetricsMap* obj){
	STNBFontShapesMetricsMapOpq* opq = (STNBFontShapesMetricsMapOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount--;
	if(opq->retainCount == 0){
		NBArraySorted_release(&opq->map);
	}
}

BOOL NBFontShapesMetricsMap_getMetrics(const STNBFontShapesMetricsMap* obj, const UI32 shapeId, STNBFontShapeMetrics* dstMetrics){
	BOOL r = FALSE;
	STNBFontShapesMetricsMapOpq* opq = (STNBFontShapesMetricsMapOpq*)obj->opaque;
	STNBFontShapesMetricsMapRec srch;
	srch.shapeId	= shapeId;
	const SI32 idx	= NBArraySorted_indexOf(&opq->map, &srch, sizeof(srch), NULL);
	if(idx != -1){
		const STNBFontShapesMetricsMapRec* rr = NBArraySorted_itmPtrAtIndex(&opq->map, STNBFontShapesMetricsMapRec, idx);
		if(dstMetrics != NULL) *dstMetrics = rr->metrics;
		r = TRUE;
	}
	return r;
}

void NBFontShapesMetricsMap_addRecord(STNBFontShapesMetricsMap* obj, const UI32 shapeId, const STNBFontShapeMetrics* metrics){
	STNBFontShapesMetricsMapOpq* opq = (STNBFontShapesMetricsMapOpq*)obj->opaque;
	STNBFontShapesMetricsMapRec srch;
	srch.shapeId	= shapeId;
	srch.metrics	= *metrics;
	NBArraySorted_add(&opq->map, &srch, sizeof(srch));
}
