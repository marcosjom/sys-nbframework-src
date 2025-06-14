//
//  NBFontLines.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/6/18.
//

#ifndef NBFontLines_h
#define NBFontLines_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBArraySorted.h"
#include "nb/fonts/NBFontMetrics.h"
#include "nb/fonts/NBFontGlyphs.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBFontLinesShape_ {
		UI32					shapeId;
		UI32					version;
		STNBAABox				box;
		STNBFontShapeMetrics	metrics;
		struct {
			UI32	first;
			UI32	count;
		} points;
		struct {
			UI32	first;
			UI32	count;
		} lines;
	} STNBFontLinesShape;
	
	typedef struct STNBFontLinesSize_ {
		UI16			fontSize;	//in pixels
		UI32			version;	//increased every time a glyph is added
		STNBArraySorted	shapes;		//STNBFontLinesShape
		STNBArray		vertexs;	//STNBFontLineVertex
	} STNBFontLinesSize;
	
	typedef struct STNBFontLinesSizeShape_ {
		const STNBFontLinesSize*	grpSize;
		const STNBFontLinesShape*	shape;
	} STNBFontLinesSizeShape;
	
	BOOL NBCompare_NBFontLinesSize(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);
	
	typedef struct STNBFontLines_ {
		void*			opaque;
	} STNBFontLines;
	
	void NBFontLines_init(STNBFontLines* obj, STNBFontGlyphs* glyphs);
	void NBFontLines_retain(STNBFontLines* obj);
	void NBFontLines_release(STNBFontLines* obj);
	//Style
	void						NBFontLines_getStyle(const STNBFontLines* obj, const char** dstFamily, const char** dstSubfamily, UI8* dstStyleMask);
	//Metrics
	STNBFontMetricsIRef			NBFontLines_getMetricsItfRef(STNBFontLines* obj, const float defFontSize);
	void						NBFontLines_getMetricsItf(STNBFontLines* obj, STNBFontMetricsI* dstItf, void** dstItfParam);
	STNBFontMetrics				NBFontLines_getMetrics(const STNBFontLines* obj);
	STNBFontMetrics				NBFontLines_getMetricsForSz(const STNBFontLines* obj, const float fontSz);
	//Shapes
	UI32						NBFontLines_getShapeId(STNBFontLines* obj, const UI32 unicode);
	STNBFontShapeMetrics		NBFontLines_getShapeMetrics(STNBFontLines* obj, const UI32 shapeId);
	STNBFontShapeMetrics		NBFontLines_getShapeMetricsForSz(STNBFontLines* obj, const UI32 shapeId, const float fontSz);
	//Data
	const STNBFontLinesSize*	NBFontLines_getLinesSize(STNBFontLines* obj, const UI16 fontSize);
	STNBFontLinesSizeShape		NBFontLines_getLinesShape(STNBFontLines* obj, const UI16 fontSize, const UI32 shapeId);
	bool						NBFontLines_getLinesShapeCalculated(STNBFontLines* obj, const float fontSize, const STNBMatrix matrix, const UI32 shapeId, STNBFontLinesData* dstData);
	UI32						NBFontLines_syncLinesShapesOfUnicodes(STNBFontLines* obj, const UI16 fontSize, const UI32* unicodes, const UI32 unicodesSz);
	
#ifdef __cplusplus
} //extern "C"
#endif
	
#endif /* NBFontLines_h */
