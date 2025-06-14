//
//  NBFontLines.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/6/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/fonts/NBFontLines.h"
//
#include "nb/core/NBCompare.h"
#include "nb/2d/NBAABox.h"
#include "nb/2d/NBAtlasMap.h"
#include "nb/fonts/NBFontCodesMap.h"
#include "nb/fonts/NBFontShapesMetricsMap.h"

//STNBFontLinesShape

BOOL NBCompare_NBFontLinesShape(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBFontLinesShape))
	if(dataSz == sizeof(STNBFontLinesShape)){
		const STNBFontLinesShape* o1 = (const STNBFontLinesShape*)data1;
		const STNBFontLinesShape* o2 = (const STNBFontLinesShape*)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return (o1->shapeId == o2->shapeId) ? TRUE : FALSE;
			case ENCompareMode_Lower:
				return (o1->shapeId < o2->shapeId) ? TRUE : FALSE;
			case ENCompareMode_LowerOrEqual:
				return (o1->shapeId <= o2->shapeId) ? TRUE : FALSE;
			case ENCompareMode_Greater:
				return (o1->shapeId > o2->shapeId) ? TRUE : FALSE;
			case ENCompareMode_GreaterOrEqual:
				return (o1->shapeId >= o2->shapeId) ? TRUE : FALSE;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}

//STNBFontLinesSize

BOOL NBCompare_NBFontLinesSize(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBFontLinesSize))
	if(dataSz == sizeof(STNBFontLinesSize)){
		const STNBFontLinesSize* o1 = (const STNBFontLinesSize*)data1;
		const STNBFontLinesSize* o2 = (const STNBFontLinesSize*)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return o1->fontSize == o2->fontSize ? TRUE : FALSE;
			case ENCompareMode_Lower:
				return o1->fontSize < o2->fontSize ? TRUE : FALSE;
			case ENCompareMode_LowerOrEqual:
				return o1->fontSize <= o2->fontSize ? TRUE : FALSE;
			case ENCompareMode_Greater:
				return o1->fontSize > o2->fontSize ? TRUE : FALSE;
			case ENCompareMode_GreaterOrEqual:
				return o1->fontSize >= o2->fontSize ? TRUE : FALSE;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}

//Metrics Itf
void 					NBFontLines_retain_(void* obj);
void 					NBFontLines_release_(void* obj);
UI32 					NBFontLines_getShapeId_(void* obj, const UI32 unicode);
STNBFontMetrics 		NBFontLines_getFontMetrics_(void* obj);
STNBFontMetrics 		NBFontLines_getFontMetricsForSz_(void* obj, const float fontSz);
STNBFontShapeMetrics	NBFontLines_getFontShapeMetrics_(void* obj, const UI32 shapeId);
STNBFontShapeMetrics	NBFontLines_getFontShapeMetricsForSz_(void* obj, const UI32 shapeId, const float fontSz);

typedef struct STNBFontLinesOpq_ {
	STNBFontGlyphs*				font;
	UI32						family;
	UI32						subfamily;
	UI8							styleMask;
	STNBFontMetrics				metrics;
	STNBFontCodesMap			codesMap;
	STNBFontShapesMetricsMap	metricsMap;
	STNBArraySorted				sizes;		//STNBFontLinesSize
	STNBString					strs;
	UI32						retainCount;
} STNBFontLinesOpq;

void NBFontLines_init(STNBFontLines* obj, STNBFontGlyphs* font){
	STNBFontLinesOpq* opq = obj->opaque = NBMemory_allocType(STNBFontLinesOpq);
	NBMemory_setZeroSt(*opq, STNBFontLinesOpq);
	opq->font			= font;
	opq->family			= 0;
	opq->subfamily		= 0;
	opq->styleMask		= 0;
	NBFontCodesMap_init(&opq->codesMap);
	NBFontShapesMetricsMap_init(&opq->metricsMap);
	NBArraySorted_init(&opq->sizes, sizeof(STNBFontLinesSize), NBCompare_NBFontLinesSize);
	NBString_init(&opq->strs);
	NBString_concatByte(&opq->strs, '\0');
	if(opq->font != NULL){
		opq->metrics		= NBFontGlyphs_getMetrics(opq->font);
		{
			const char* family		= NULL;
			const char* subfamily	= NULL;
			UI8 styleMask			= 0;
			NBFontGlyphs_getStyle(opq->font, &family, &subfamily, &styleMask);
			opq->family		= opq->strs.length; NBString_concat(&opq->strs, family); NBString_concatByte(&opq->strs, '\0');
			opq->subfamily	= opq->strs.length; NBString_concat(&opq->strs, subfamily); NBString_concatByte(&opq->strs, '\0');
			opq->styleMask	= styleMask;
		}
		NBFontGlyphs_retain(opq->font);
	}
	opq->retainCount	= 1;
}

void NBFontLines_retain(STNBFontLines* obj){
	STNBFontLinesOpq* opq = (STNBFontLinesOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount++;
}

void NBFontLines_release(STNBFontLines* obj){
	STNBFontLinesOpq* opq = (STNBFontLinesOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount--;
	if(opq->retainCount == 0){
		{
			UI32 i; for(i = 0; i < opq->sizes.use; i++){
				STNBFontLinesSize* fSz = NBArraySorted_itmPtrAtIndex(&opq->sizes, STNBFontLinesSize, i);
				NBArraySorted_release(&fSz->shapes);
				NBArray_release(&fSz->vertexs);
			}
			NBArraySorted_empty(&opq->sizes);
			NBArraySorted_release(&opq->sizes);
		}
		if(opq->font != NULL){
			NBFontGlyphs_release(opq->font);
			opq->font = NULL;
		}
		NBFontShapesMetricsMap_release(&opq->metricsMap);
		NBFontCodesMap_release(&opq->codesMap);
		NBString_release(&opq->strs);
		//
		NBMemory_free(obj->opaque);
		obj->opaque = NULL;
	}
}

//Style

void NBFontLines_getStyle(const STNBFontLines* obj, const char** dstFamily, const char** dstSubfamily, UI8* dstStyleMask){
	const STNBFontLinesOpq* opq = (STNBFontLinesOpq*)obj->opaque;
	if(dstFamily != NULL) *dstFamily		= &opq->strs.str[opq->family];
	if(dstSubfamily != NULL) *dstSubfamily	= &opq->strs.str[opq->subfamily];
	if(dstStyleMask != NULL) *dstStyleMask	= opq->styleMask;
}

//Metrics

void* NBFontGlyphs_getItfMethod(void* obj, STNBFontMetricsI* dstItf){
	void* r = NULL;
	if(obj != NULL){
		//ToDo: validate 'obj's type
		STNBFontGlyphs* obj2 = (STNBFontGlyphs*)obj;
		NBFontGlyphs_getMetricsItf(obj2, dstItf, &r);
	}
	return r;
}

STNBFontMetricsIRef NBFontLines_getMetricsItfRef(STNBFontLines* obj, const float defFontSize){
	STNBFontMetricsIRef r;
	NBFontLines_getMetricsItf(obj, &r.itf, &r.itfParam);
	r.fontSz = defFontSize;
	return r;
}

void NBFontLines_getMetricsItf(STNBFontLines* obj, STNBFontMetricsI* dstItf, void** dstItfParam){
	dstItf->retain						= NBFontLines_retain_;
	dstItf->release						= NBFontLines_release_;
	dstItf->getNativeSz					= NULL;
	dstItf->getShapeId					= NBFontLines_getShapeId_;
	dstItf->getFontMetrics				= NBFontLines_getFontMetrics_;
	dstItf->getFontMetricsForSz			= NBFontLines_getFontMetricsForSz_;
	dstItf->getFontShapeMetrics			= NBFontLines_getFontShapeMetrics_;
	dstItf->getFontShapeMetricsForSz	= NBFontLines_getFontShapeMetricsForSz_;
	*dstItfParam						= obj;
}

STNBFontMetrics NBFontLines_getMetrics(const STNBFontLines* obj){
	STNBFontLinesOpq* opq = (STNBFontLinesOpq*)obj->opaque;
	return opq->metrics;
}

STNBFontMetrics NBFontLines_getMetricsForSz(const STNBFontLines* obj, const float fontSz){
	STNBFontMetrics r;
	const STNBFontLinesOpq* opq = (STNBFontLinesOpq*)obj->opaque;
	const float emScale = fontSz / opq->metrics.emBoxSz;
	r.emBoxSz	= opq->metrics.emBoxSz * emScale;
	r.ascender	= opq->metrics.ascender * emScale;
	r.descender	= opq->metrics.descender * emScale;
	r.height	= opq->metrics.height * emScale;
	return r;
}

//Shapes

UI32 NBFontLines_getShapeIdOpq(STNBFontLinesOpq* opq, const UI32 unicode, BOOL* dstIsNewShape){
	UI32 r = 0;
	if(dstIsNewShape != NULL) *dstIsNewShape = FALSE;
	if(!NBFontCodesMap_getShapeId(&opq->codesMap, unicode, &r)){
		//Search in parent font
		if(opq->font != NULL){
			r = NBFontGlyphs_getShapeId(opq->font, unicode);
			//Add to codesMap
			NBFontCodesMap_addRecord(&opq->codesMap, unicode, r);
			if(dstIsNewShape != NULL) *dstIsNewShape = TRUE;
		}
	}
	return r;
}

UI32 NBFontLines_getShapeId(STNBFontLines* obj, const UI32 unicode){
	STNBFontLinesOpq* opq = (STNBFontLinesOpq*)obj->opaque;
	return NBFontLines_getShapeIdOpq(opq, unicode, NULL);
}

STNBFontShapeMetrics NBFontLines_getShapeMetricsOpq(STNBFontLinesOpq* opq, const UI32 shapeId){
	STNBFontShapeMetrics r;
	if(!NBFontShapesMetricsMap_getMetrics(&opq->metricsMap, shapeId, &r)){
		//Search in parent font
		if(opq->font != NULL){
			r = NBFontGlyphs_getShapeMetrics(opq->font, shapeId);
			//Add to metrics map
			NBFontShapesMetricsMap_addRecord(&opq->metricsMap, shapeId, &r);
		}
	}
	return r;
}

STNBFontShapeMetrics NBFontLines_getShapeMetrics(STNBFontLines* obj, const UI32 shapeId){
	STNBFontLinesOpq* opq = (STNBFontLinesOpq*)obj->opaque;
	return NBFontLines_getShapeMetricsOpq(opq, shapeId);
}

STNBFontShapeMetrics NBFontLines_getShapeMetricsForSz(STNBFontLines* obj, const UI32 shapeId, const float fontSz){
	STNBFontShapeMetrics r = NBFontLines_getShapeMetrics(obj, shapeId);
	STNBFontLinesOpq* opq = (STNBFontLinesOpq*)obj->opaque;
	const float emScale = fontSz / opq->metrics.emBoxSz;
	r.hBearingX *= emScale;
	r.hBearingY *= emScale;
	r.hAdvance	*= emScale;
	r.vBearingX *= emScale;
	r.vBearingY *= emScale;
	r.vAdvance	*= emScale;
	r.width 	*= emScale;
	r.height	*= emScale;
	return r;
}

//Search

STNBFontLinesSize* NBFontLines_getSizeOpq(STNBFontLinesOpq* opq, const UI16 fontSize, const BOOL createIfNecesary){
	STNBFontLinesSize* r	= NULL;
	//Search
	STNBFontLinesSize srch;
	srch.fontSize = fontSize;
	const SI32 i = NBArraySorted_indexOf(&opq->sizes, &srch, sizeof(srch), NULL);
	if(i != -1){
		r = NBArraySorted_itmPtrAtIndex(&opq->sizes, STNBFontLinesSize, i);
	} else if(createIfNecesary){
		srch.version = 0;
		NBArraySorted_init(&srch.shapes, sizeof(STNBFontLinesShape), NBCompare_NBFontLinesShape);
		NBArray_init(&srch.vertexs, sizeof(STNBFontLineVertex), NULL);
		r = NBArraySorted_add(&opq->sizes, &srch, sizeof(srch));
	}
	return r;
}

const STNBFontLinesSize* NBFontLines_getLinesSize(STNBFontLines* obj, const UI16 fontSize){
	STNBFontLinesOpq* opq	= (STNBFontLinesOpq*)obj->opaque;
	return NBFontLines_getSizeOpq(opq, fontSize, TRUE);
}

//Generate

STNBFontLinesSizeShape NBFontLines_getLinesShapeOpq(STNBFontLinesOpq* opq, const UI16 fontSize, const UI32 shapeId, STNBFontLinesData* dstData, BOOL* dstWasCreated){
	STNBFontLinesSizeShape r;
	NBMemory_setZeroSt(r, STNBFontLinesSizeShape);
	if(opq->font != NULL){
		STNBFontLinesSize* fontSz	= NBFontLines_getSizeOpq(opq, fontSize, TRUE); NBASSERT(fontSz != NULL)
		if(fontSz != NULL){
			STNBFontLinesShape srch;
			srch.shapeId		= shapeId;
			const SI32 i		= NBArraySorted_indexOf(&fontSz->shapes, &srch, sizeof(srch), NULL);
			if(i != -1){
				r.grpSize		= fontSz;
				r.shape			= NBArraySorted_itmPtrAtIndex(&fontSz->shapes, STNBFontLinesShape, i);
				if(dstWasCreated != NULL) *dstWasCreated = FALSE;
			} else {
				STNBMatrix matrix;
				NBMatrix_setIdentity(&matrix);
				if(NBFontGlyphs_getShapesLines(opq->font, shapeId, fontSize, matrix, dstData)){
					srch.version		= 1;
					srch.box			= dstData->box;
					srch.metrics		= dstData->metrics;
					//PRINTF_INFO("FontLines, shapeId(%d) size(%d) advance(%f) hBearing(%f).\n", shapeId, fontSize, srch.metrics.hAdvance, srch.metrics.hBearingX);
					srch.points.first	= fontSz->vertexs.use;
					srch.points.count	= dstData->vPoints.use;
					srch.lines.first	= fontSz->vertexs.use + srch.points.count;
					srch.lines.count	= dstData->vLines.use / 2;
					NBArray_growBuffer(&fontSz->vertexs, dstData->vPoints.use + dstData->vLines.use);
					NBArray_addItems(&fontSz->vertexs, NBArray_data(&dstData->vPoints), sizeof(STNBFontLineVertex), dstData->vPoints.use);
					NBArray_addItems(&fontSz->vertexs, NBArray_data(&dstData->vLines), sizeof(STNBFontLineVertex), dstData->vLines.use);
					r.grpSize			= fontSz;
					r.shape				= NBArraySorted_add(&fontSz->shapes, &srch, sizeof(srch));
					if(dstWasCreated != NULL) *dstWasCreated = TRUE;
					fontSz->version++;
					//Register shape-metrics to map
					NBFontLines_getShapeMetricsOpq(opq, srch.shapeId);
				}
			}
		}
	}
	return r;
}

STNBFontLinesSizeShape NBFontLines_getLinesShape(STNBFontLines* obj, const UI16 fontSize, const UI32 shapeId){
	STNBFontLinesSizeShape r;
	STNBFontLinesOpq* opq = (STNBFontLinesOpq*)obj->opaque;
	STNBFontLinesData data;
	NBMemory_setZeroSt(data, STNBFontLinesData);
	NBArray_init(&data.vPoints, sizeof(STNBFontLineVertex), NULL);
	NBArray_init(&data.vLines, sizeof(STNBFontLineVertex), NULL);
	r = NBFontLines_getLinesShapeOpq(opq, fontSize, shapeId, &data, NULL);
	NBArray_release(&data.vPoints);
	NBArray_release(&data.vLines);
	return r;
}

bool NBFontLines_getLinesShapeCalculated(STNBFontLines* obj, const float fontSize, const STNBMatrix matrix, const UI32 shapeId, STNBFontLinesData* dstData){
	bool r = false;
	STNBFontLinesOpq* opq = (STNBFontLinesOpq*)obj->opaque;
	NBASSERT(opq->font != NULL)
	if(opq->font != NULL){
		r = NBFontGlyphs_getShapesLines(opq->font, shapeId, fontSize, matrix, dstData);
	}
	return r;
}

UI32 NBFontLines_syncLinesShapesOfUnicodes(STNBFontLines* obj, const UI16 fontSize, const UI32* unicodes, const UI32 unicodesSz){
	UI32 r = 0;
	STNBFontLinesOpq* opq	= (STNBFontLinesOpq*)obj->opaque;
	STNBFontLinesData data;
	NBMemory_setZeroSt(data, STNBFontLinesData);
	NBArray_init(&data.vPoints, sizeof(STNBFontLineVertex), NULL);
	NBArray_init(&data.vLines, sizeof(STNBFontLineVertex), NULL);
	UI32 i; for(i = 0; i < unicodesSz; i++){
		BOOL wasCreated = FALSE, isNewShapeId = FALSE;
		const UI32 shapeId = NBFontLines_getShapeIdOpq(opq, unicodes[i], &isNewShapeId);
		if(isNewShapeId){
			const STNBFontLinesSizeShape rr = NBFontLines_getLinesShapeOpq(opq, fontSize, shapeId, &data, &wasCreated);
			if(rr.shape != NULL && wasCreated){
				r++;
			}
		}
		NBArray_empty(&data.vPoints);
		NBArray_empty(&data.vLines);
	}
	NBArray_release(&data.vPoints);
	NBArray_release(&data.vLines);
	return r;
}


//Metrics Itf
void NBFontLines_retain_(void* obj){
	NBFontLines_retain((STNBFontLines*)obj);
}
void NBFontLines_release_(void* obj){
	NBFontLines_release((STNBFontLines*)obj);
}
UI32 NBFontLines_getShapeId_(void* obj, const UI32 unicode){
	return NBFontLines_getShapeId((STNBFontLines*)obj, unicode);
}
STNBFontMetrics NBFontLines_getFontMetrics_(void* obj){
	return NBFontLines_getMetrics((STNBFontLines*)obj);
}
STNBFontMetrics NBFontLines_getFontMetricsForSz_(void* obj, const float fontSz){
	return NBFontLines_getMetricsForSz((STNBFontLines*)obj, fontSz);
}
STNBFontShapeMetrics NBFontLines_getFontShapeMetrics_(void* obj, const UI32 shapeId){
	return NBFontLines_getShapeMetrics((STNBFontLines*)obj, shapeId);
}
STNBFontShapeMetrics NBFontLines_getFontShapeMetricsForSz_(void* obj, const UI32 shapeId, const float fontSz){
	return NBFontLines_getShapeMetricsForSz((STNBFontLines*)obj, shapeId, fontSz);
}

