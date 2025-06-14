//
//  NBFontBitmaps.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/6/18.
//

#ifndef NBFontBitmaps_h
#define NBFontBitmaps_h

#include "nb/NBFrameworkDefs.h"
#include "nb/2d/NBPoint.h"
#include "nb/2d/NBAtlasMap.h"
#include "nb/2d/NBBitmap.h"
#include "nb/fonts/NBFontMetrics.h"
#include "nb/fonts/NBFontLines.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBFontBitmap_ {
		UI32				version; 	//increments each time a shape is drawn in this atlas
		const STNBAtlasMap*	atlas;
		const STNBBitmap*	bitmap;
		const STNBArray*	shapeIds;	//UI32
	} STNBFontBitmap;
	
	typedef struct STNBFontBitmapShape_ {
		UI32					shapeId;
		UI32					version; //increments each time a shape is drawn in this atlas
		STNBFontShapeMetrics	metrics;
		UI16					iBitmap;
		UI16					iArea;
		const STNBBitmap*		bitmap;
		STNBRectI				area;
		STNBPoint				center;
	} STNBFontBitmapShape;
	
	typedef struct STNBFontBitmaps_ {
		void*			opaque;
	} STNBFontBitmaps;
	
	void NBFontBitmaps_initWithSz(STNBFontBitmaps* obj, STNBFontLines* lines, const float fontSz, const UI16 shapesMargin);
	void NBFontBitmaps_retain(STNBFontBitmaps* obj);
	void NBFontBitmaps_release(STNBFontBitmaps* obj);
	
	//Version
	UI32					NBFontBitmaps_getVersion(const STNBFontBitmaps* obj);
	//Style
	void					NBFontBitmaps_getStyle(const STNBFontBitmaps* obj, const char** dstFamily, const char** dstSubfamily, UI8* dstStyleMask);
	float					NBFontBitmaps_getSize(const STNBFontBitmaps* obj);
	//Metrics
	STNBFontMetricsIRef		NBFontBitmaps_getMetricsItfRef(STNBFontBitmaps* obj, const float defFontSize);
	void					NBFontBitmaps_getMetricsItf(STNBFontBitmaps* obj, STNBFontMetricsI* dstItf, void** dstItfParam);
	STNBFontMetrics			NBFontBitmaps_getMetrics(const STNBFontBitmaps* obj);
	STNBFontMetrics			NBFontBitmaps_getMetricsForSz(const STNBFontBitmaps* obj, const float fontSz);
	//Shapes
	UI32					NBFontBitmaps_getShapeId(STNBFontBitmaps* obj, const UI32 unicode);
	STNBFontShapeMetrics	NBFontBitmaps_getShapeMetrics(STNBFontBitmaps* obj, const UI32 shapeId);
	STNBFontShapeMetrics	NBFontBitmaps_getShapeMetricsForSz(STNBFontBitmaps* obj, const UI32 shapeId, const float fontSz);
	//Data
	STNBFontBitmap			NBFontBitmaps_getBitmapAtIndex(const STNBFontBitmaps* obj, const UI32 index);
	STNBFontBitmapShape		NBFontBitmaps_getBitmapShape(STNBFontBitmaps* obj, const UI32 shapeId, const BOOL syncIfNecesary);
	STNBFontBitmapShape		NBFontBitmaps_getBitmapShapeByUnicode(STNBFontBitmaps* obj, const UI32 unicode, const BOOL syncIfNecesary);
	UI32					NBFontBitmaps_syncBitmapShapes(STNBFontBitmaps* obj, const UI32* shapeIds, const UI32 shapeIdsSz);
	UI32					NBFontBitmaps_syncBitmapShapesOfUnicodes(STNBFontBitmaps* obj, const UI32* unicodes, const UI32 unicodesSz);
	UI32					NBFontBitmaps_syncBitmapsShapes(STNBFontBitmaps* obj);
	UI32					NBFontBitmaps_syncBitmapsShapesOfFont(STNBFontBitmaps* obj, STNBFontLines* font);
	
	//UI32 NBFontBitmaps_syncBitmaps(STNBFontBitmaps* obj, const UI32* unicodes, const UI32 unicodesSz);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBFontBitmaps_h */
