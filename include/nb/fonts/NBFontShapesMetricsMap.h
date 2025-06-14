//
//  NBFontShapesMetricsMap.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/12/18.
//

#ifndef NBFontShapesMetricsMap_h
#define NBFontShapesMetricsMap_h

#include "nb/NBFrameworkDefs.h"
#include "nb/fonts/NBFontMetrics.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBFontShapesMetricsMap_ {
		void* opaque;
	} STNBFontShapesMetricsMap;
	
	void NBFontShapesMetricsMap_init(STNBFontShapesMetricsMap* obj);
	void NBFontShapesMetricsMap_retain(STNBFontShapesMetricsMap* obj);
	void NBFontShapesMetricsMap_release(STNBFontShapesMetricsMap* obj);
	
	BOOL NBFontShapesMetricsMap_getMetrics(const STNBFontShapesMetricsMap* obj, const UI32 shapeId, STNBFontShapeMetrics* dstMetrics);
	void NBFontShapesMetricsMap_addRecord(STNBFontShapesMetricsMap* obj, const UI32 shapeId, const STNBFontShapeMetrics* metrics);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBFontShapesMetricsMap_h */
