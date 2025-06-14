//
//  NBFontGlyphs.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/6/18.
//

#ifndef NBFontGlyphs_h
#define NBFontGlyphs_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/2d/NBAABox.h"
#include "nb/2d/NBMatrix.h"
#include "nb/fonts/NBFontNative.h"
#include "nb/fonts/NBFontMetrics.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBFontGlyphs_ {
		void* opaque;
	} STNBFontGlyphs;
	
	void NBFontGlyphs_initWithProps(STNBFontGlyphs* obj, const SI32 emBoxSz, const char* family, const char* subfamily, const UI8 styleMask, const SI32 ascender, const SI32 descender, const SI32 height, STNBFontNativeI* itf, void* itfParam);
	void NBFontGlyphs_retain(STNBFontGlyphs* obj);
	void NBFontGlyphs_release(STNBFontGlyphs* obj);
	
	//Style
	void					NBFontGlyphs_getStyle(const STNBFontGlyphs* obj, const char** dstFamily, const char** dstSubfamily, UI8* dstStyleMask);
	//Metrics
	STNBFontMetricsIRef		NBFontGlyphs_getMetricsItfRef(STNBFontGlyphs* obj, const float defFontSize);
	void					NBFontGlyphs_getMetricsItf(STNBFontGlyphs* obj, STNBFontMetricsI* dstItf, void** dstItfParam);
	STNBFontMetrics			NBFontGlyphs_getMetrics(const STNBFontGlyphs* obj);
	STNBFontMetrics			NBFontGlyphs_getMetricsForSz(const STNBFontGlyphs* obj, const float fontSz);
	//Shapes
	UI32					NBFontGlyphs_getShapeId(const STNBFontGlyphs* obj, const UI32 unicode);
	STNBFontShapeMetrics	NBFontGlyphs_getShapeMetrics(const STNBFontGlyphs* obj, const UI32 shapeId);
	STNBFontShapeMetrics	NBFontGlyphs_getShapeMetricsForSz(const STNBFontGlyphs* obj, const UI32 shapeId, const float fontSz);
	//Data
	BOOL					NBFontGlyphs_getShapesLines(STNBFontGlyphs* obj, const UI32 shapeId, const UI32 fontSize, const STNBMatrix matrix, STNBFontLinesData* dstData);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBFontGlyphs_h */
