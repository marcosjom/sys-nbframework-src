//
//  NBFontGlyph.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/6/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/fonts/NBFontGlyphs.h"

typedef struct STNBFontGlyphsOpq_ {
	UI32			family;
	UI32			subfamily;
	UI8				styleMask;	//ENNBFontStyleBit
	STNBFontMetrics	metrics;
	STNBFontNativeI	itf;
	void*			itfParam;
	STNBString		strs;
	UI32			retainCount;
} STNBFontGlyphsOpq;

//Metrics Itf
void					NBFontGlyphs_retain_(void* obj);
void					NBFontGlyphs_release_(void* obj);
UI32 					NBFontGlyphs_getShapeId_(void* obj, const UI32 unicode);
STNBFontMetrics 		NBFontGlyphs_getFontMetrics_(void* obj);
STNBFontMetrics 		NBFontGlyphs_getFontMetricsForSz_(void* obj, const float fontSz);
STNBFontShapeMetrics	NBFontGlyphs_getFontShapeMetrics_(void* obj, const UI32 shapeId);
STNBFontShapeMetrics	NBFontGlyphs_getFontShapeMetricsForSz_(void* obj, const UI32 shapeId, const float fontSz);

void NBFontGlyphs_initWithProps(STNBFontGlyphs* obj, const SI32 emBoxSz, const char* family, const char* subfamily, const UI8 styleMask, const SI32 ascender, const SI32 descender, const SI32 height, STNBFontNativeI* itf, void* itfParam){
	STNBFontGlyphsOpq* opq	= obj->opaque = NBMemory_allocType(STNBFontGlyphsOpq);
	NBMemory_setZeroSt(*opq, STNBFontGlyphsOpq);
	NBString_init(&opq->strs);
	opq->family				= opq->strs.length; NBString_concat(&opq->strs, family); NBString_concatByte(&opq->strs, '\0');
	opq->subfamily			= opq->strs.length; NBString_concat(&opq->strs, subfamily); NBString_concatByte(&opq->strs, '\0');
	opq->styleMask			= styleMask;
	opq->metrics.emBoxSz	= emBoxSz;
	opq->metrics.ascender	= ascender;
	opq->metrics.descender	= descender;
	opq->metrics.height		= height;
	{
		if(itf != NULL){
			opq->itf		= *itf;
		} else {
			NBMemory_set(&opq->itf, 0, sizeof(opq->itf));
		}
		opq->itfParam		= itfParam;
	}
	opq->retainCount	= 1;
}

void NBFontGlyphs_retain(STNBFontGlyphs* obj){
	STNBFontGlyphsOpq* opq	= (STNBFontGlyphsOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount++;
}

void NBFontGlyphs_release(STNBFontGlyphs* obj){
	STNBFontGlyphsOpq* opq	= (STNBFontGlyphsOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount--;
	if(opq->retainCount == 0){
		opq->metrics.emBoxSz	= 0;
		opq->metrics.ascender	= 0;
		opq->metrics.descender	= 0;
		opq->metrics.height		= 0;
		opq->family				= 0;
		opq->subfamily			= 0;
		opq->styleMask			= 0;
		{
			NBMemory_set(&opq->itf, 0, sizeof(opq->itf));
			opq->itfParam = NULL;
		}
		NBString_release(&opq->strs);
		//
		NBMemory_free(obj->opaque);
		obj->opaque = NULL;
	}
}

//Font

void NBFontGlyphs_getStyle(const STNBFontGlyphs* obj, const char** dstFamily, const char** dstSubfamily, UI8* dstStyleMask){
	const STNBFontGlyphsOpq* opq	= (STNBFontGlyphsOpq*)obj->opaque;
	if(dstFamily != NULL) *dstFamily		= &opq->strs.str[opq->family];
	if(dstSubfamily != NULL) *dstSubfamily	= &opq->strs.str[opq->subfamily];
	if(dstStyleMask != NULL) *dstStyleMask	= opq->styleMask;
}

//Metrics

STNBFontMetricsIRef NBFontGlyphs_getMetricsItfRef(STNBFontGlyphs* obj, const float defFontSize){
	STNBFontMetricsIRef r;
	NBFontGlyphs_getMetricsItf(obj, &r.itf, &r.itfParam);
	r.fontSz = defFontSize;
	return r;
}

void NBFontGlyphs_getMetricsItf(STNBFontGlyphs* obj, STNBFontMetricsI* dstItf, void** dstItfParam){
	dstItf->retain						= NBFontGlyphs_retain_;
	dstItf->release						= NBFontGlyphs_release_;
	dstItf->getNativeSz					= NULL;
	dstItf->getShapeId					= NBFontGlyphs_getShapeId_;
	dstItf->getFontMetrics				= NBFontGlyphs_getFontMetrics_;
	dstItf->getFontMetricsForSz			= NBFontGlyphs_getFontMetricsForSz_;
	dstItf->getFontShapeMetrics			= NBFontGlyphs_getFontShapeMetrics_;
	dstItf->getFontShapeMetricsForSz	= NBFontGlyphs_getFontShapeMetricsForSz_;
	*dstItfParam						= obj;
}

STNBFontMetrics NBFontGlyphs_getMetrics(const STNBFontGlyphs* obj){
	const STNBFontGlyphsOpq* opq = (STNBFontGlyphsOpq*)obj->opaque;
	return opq->metrics;
}

STNBFontMetrics	NBFontGlyphs_getMetricsForSz(const STNBFontGlyphs* obj, const float fontSz){
	STNBFontMetrics r;
	const STNBFontGlyphsOpq* opq = (STNBFontGlyphsOpq*)obj->opaque;
	const float emScale = fontSz / opq->metrics.emBoxSz;
	r.emBoxSz	= opq->metrics.emBoxSz * emScale;
	r.ascender	= opq->metrics.ascender * emScale;
	r.descender	= opq->metrics.descender * emScale;
	r.height	= opq->metrics.height * emScale;
	return r;
}

//Shape

UI32 NBFontGlyphs_getShapeId(const STNBFontGlyphs* obj, const UI32 unicode){
	UI32 r = 0;
	const STNBFontGlyphsOpq* opq	= (STNBFontGlyphsOpq*)obj->opaque;
	if(opq->itf.getShapeId != NULL){
		r = (*opq->itf.getShapeId)(opq->itfParam, unicode);
	}
	return r;
}

STNBFontShapeMetrics NBFontGlyphs_getShapeMetrics(const STNBFontGlyphs* obj, const UI32 shapeId){
	STNBFontShapeMetrics r;
	STNBFontGlyphsOpq* opq	= (STNBFontGlyphsOpq*)obj->opaque;
	if(opq->itf.getShapeMetrics == NULL){
		NBMemory_setZeroSt(r, STNBFontShapeMetrics);
	} else {
		r = (*opq->itf.getShapeMetrics)(opq->itfParam, shapeId);
	}
	return r;
}

STNBFontShapeMetrics NBFontGlyphs_getShapeMetricsForSz(const STNBFontGlyphs* obj, const UI32 shapeId, const float fontSz){
	STNBFontShapeMetrics r;
	STNBFontGlyphsOpq* opq	= (STNBFontGlyphsOpq*)obj->opaque;
	if(opq->itf.getShapeMetrics == NULL){
		NBMemory_setZeroSt(r, STNBFontShapeMetrics);
	} else {
		const float emScale = fontSz / opq->metrics.emBoxSz;
		r = (*opq->itf.getShapeMetrics)(opq->itfParam, shapeId);
		r.hBearingX *= emScale;
		r.hBearingY *= emScale;
		r.hAdvance	*= emScale;
		r.vBearingX *= emScale;
		r.vBearingY *= emScale;
		r.vAdvance	*= emScale;
		r.width 	*= emScale;
		r.height	*= emScale;
	}
	return r;
}

//Data

BOOL NBFontGlyphs_getShapesLines(STNBFontGlyphs* obj, const UI32 shapeId, const UI32 fontSize, const STNBMatrix matrix, STNBFontLinesData* dstData){
	BOOL r = FALSE;
	STNBFontGlyphsOpq* opq	= (STNBFontGlyphsOpq*)obj->opaque;
	NBASSERT(opq->itf.getShapeLines != NULL)
	if(opq->itf.getShapeLines != NULL){
		r = (*opq->itf.getShapeLines)(opq->itfParam, shapeId, fontSize, matrix, dstData);
	}
	return r;
}

//Metrics Itf
void NBFontGlyphs_retain_(void* obj){
	NBFontGlyphs_retain((STNBFontGlyphs*)obj);
}
void NBFontGlyphs_release_(void* obj){
	NBFontGlyphs_release((STNBFontGlyphs*)obj);
}
UI32 NBFontGlyphs_getShapeId_(void* obj, const UI32 unicode){
	return NBFontGlyphs_getShapeId((STNBFontGlyphs*)obj, unicode);
}
STNBFontMetrics NBFontGlyphs_getFontMetrics_(void* obj){
	return NBFontGlyphs_getMetrics((STNBFontGlyphs*)obj);
}
STNBFontMetrics NBFontGlyphs_getFontMetricsForSz_(void* obj, const float fontSz){
	return NBFontGlyphs_getMetricsForSz((STNBFontGlyphs*)obj, fontSz);
}
STNBFontShapeMetrics NBFontGlyphs_getFontShapeMetrics_(void* obj, const UI32 shapeId){
	return NBFontGlyphs_getShapeMetrics((STNBFontGlyphs*)obj, shapeId);
}
STNBFontShapeMetrics NBFontGlyphs_getFontShapeMetricsForSz_(void* obj, const UI32 shapeId, const float fontSz){
	return NBFontGlyphs_getShapeMetricsForSz((STNBFontGlyphs*)obj, shapeId, fontSz);
}


