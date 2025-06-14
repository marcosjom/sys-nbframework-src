//
//  Header.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/12/18.
//

#ifndef Header_h
#define Header_h

#include "nb/NBFrameworkDefs.h"
#include "nb/fonts/NBFontMetrics.h"

//Style

typedef enum ENNBFontStyleBit_ {
	ENNBFontStyleBit_Bold		= 1,
	ENNBFontStyleBit_Italic		= 2,
	ENNBFontStyleBit_Undelined	= 4,
	ENNBFontStyleBit_Striked	= 8,
	ENNBFontStyleBit_Shadowed	= 16,
	ENNBFontStyleBit_Condensed	= 32,
	ENNBFontStyleBit_Extended	= 64
} ENNBFontStyleBit;

//Vertex (lines raster)

typedef struct STNBFontLineVertex_ {
	SI16	line;
	SI16	pos;
	UI8		intensity;
} STNBFontLineVertex;

typedef struct STNBFontLinesData_ {
	STNBFontShapeMetrics	metrics;
	SI32					spansCount;
	STNBAABox				box;
	STNBArray				vPoints;	//STNBFontLineVertex
	STNBArray				vLines;		//STNBFontLineVertex
} STNBFontLinesData;

//Itf to native font

typedef struct STNBFontNativeI_ {
	UI32 					(*getShapeId)(void* obj, const UI32 unicode);
	STNBFontShapeMetrics	(*getShapeMetrics)(void* obj, const UI32 shapeId);
	BOOL 					(*getShapeLines)(void* obj, const UI32 shapeId, const UI32 fontSize, const STNBMatrix matrix, STNBFontLinesData* dstData);
} STNBFontNativeI;

#endif /* Header_h */
