//
//  NBFontMetrics.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/12/18.
//

#ifndef STNBFontMetricsI_h
#define STNBFontMetricsI_h

#include "nb/NBFrameworkDefs.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBFontMetrics_ {
		float	emBoxSz;
		float	ascender;
		float	descender;	//is negative
		float	height;
	} STNBFontMetrics;
	
	//https://www.freetype.org/freetype2/docs/glyphs/glyphs-3.html
	typedef struct STNBFontShapeMetrics_ {
		float	hBearingX;
		float	hBearingY;
		float	hAdvance;
		float	vBearingX;
		float	vBearingY;
		float	vAdvance;
		float	width;
		float	height;
	} STNBFontShapeMetrics;
	
	typedef struct STNBFontMetricsI_ {
		void					(*retain)(void* obj);
		void					(*release)(void* obj);
		//Native size (if render is fixed size)
		float 					(*getNativeSz)(void* obj);
		UI32 					(*getShapeId)(void* obj, const UI32 unicode);
		//Font metrics
		STNBFontMetrics 		(*getFontMetrics)(void* obj);
		STNBFontMetrics 		(*getFontMetricsForSz)(void* obj, const float fontSz);
		//Shapes metrics
		STNBFontShapeMetrics	(*getFontShapeMetrics)(void* obj, const UI32 shapeId);
		STNBFontShapeMetrics	(*getFontShapeMetricsForSz)(void* obj, const UI32 shapeId, const float fontSz);
	} STNBFontMetricsI;
	
	typedef struct STNBFontMetricsIRef_ {
		STNBFontMetricsI		itf;
		void*					itfParam;
		float					fontSz;
	} STNBFontMetricsIRef;
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* STNBFontMetricsI_h */
