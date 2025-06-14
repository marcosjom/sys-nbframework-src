//
//  NBAABox.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/6/18.
//

#ifndef NBAABox_h
#define NBAABox_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBCompare.h"
#include "nb/core/NBStruct.h"
#include "nb/2d/NBPoint.h"
#include "nb/2d/NBSize.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBAABox_ {
		float	xMin;
		float	xMax;
		float	yMin;
		float	yMax;
	} STNBAABox;
	
	const STNBStructMap* NBAABox_getSharedStructMap(void);
	
	typedef struct STNBAABoxI_ {
		SI32	xMin;
		SI32	xMax;
		SI32	yMin;
		SI32	yMax;
	} STNBAABoxI;
	
	const STNBStructMap* NBAABoxI_getSharedStructMap(void);
	
	//
	
	BOOL NBCompare_NBAABox(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);
	
	//
	void NBAABox_init(STNBAABox* obj);
	
	BOOL NBAABox_isEmpty(const STNBAABox* obj);
	void NBAABox_wrapPoint(STNBAABox* obj, const STNBPoint p);
	void NBAABox_wrapFirstPoint(STNBAABox* obj, const STNBPoint p);
	void NBAABox_wrapNextPoint(STNBAABox* obj, const STNBPoint p);
	void NBAABox_wrapPoints(STNBAABox* obj, const STNBPoint* arr, const UI32 arrSz);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBAABox_h */
