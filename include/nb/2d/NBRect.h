//
//  NBRect.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/12/18.
//

#ifndef NBRect_h
#define NBRect_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBStructMap.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBRect_ {
		float	x;
		float	y;
		float	width;
		float	height;
	} STNBRect;
	
	const STNBStructMap* NBRect_getSharedStructMap(void);
	
	typedef struct STNBRectI_ {
		SI32	x;
		SI32	y;
		SI32	width;
		SI32	height;
	} STNBRectI;
	
	const STNBStructMap* NBRectI_getSharedStructMap(void);

	typedef struct STNBRectI16_ {
		SI16	x;
		SI16	y;
		SI16	width;
		SI16	height;
	} STNBRectI16;

	const STNBStructMap* NBRectI16_getSharedStructMap(void);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBRect_h */
