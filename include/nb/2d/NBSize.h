//
//  NBSize.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/6/18.
//

#ifndef NBSize_h
#define NBSize_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBCompare.h"
#include "nb/core/NBStructMap.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBSize_ {
		float	width;
		float	height;
	} STNBSize;
	
	const STNBStructMap* NBSize_getSharedStructMap(void);
	
	//

	typedef struct STNBSizeI_ {
		SI32	width;
		SI32	height;
	} STNBSizeI;
	
	const STNBStructMap* NBSizeI_getSharedStructMap(void);

	//

	typedef struct STNBSizeI16_ {
		SI16	width;
		SI16	height;
	} STNBSizeI16;

	const STNBStructMap* NBSizeI16_getSharedStructMap(void);
	
	//
	
	BOOL NBCompare_NBSize(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);
	
	//
	
	void NBSize_set(STNBSize* obj, const float width, const float height);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBSize_h */
