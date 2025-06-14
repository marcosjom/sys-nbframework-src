//
//  NBBezier2.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/6/18.
//

#ifndef NBBezier2_h
#define NBBezier2_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBCompare.h"
#include "nb/core/NBStructMap.h"
#include "nb/2d/NBPoint.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBBezier2_ {
		STNBPoint	start;
		STNBPoint	ref;
		STNBPoint	end;
	} STNBBezier2;
	
	const STNBStructMap* NBBezier2_getSharedStructMap(void);
	
	//
	
	STNBPoint	NBBezier2_getMidPoint(const STNBBezier2* obj, const float relPos);
	float		NBBezier2_length(const STNBBezier2* obj);
	
	//
	
	STNBPoint	NBBezier2_bzGetMidPoint(const STNBPoint start, const STNBPoint ref, const STNBPoint end, const float relPos);
	float		NBBezier2_bzLength(const STNBPoint start, const STNBPoint ref, const STNBPoint end);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBBezier2_h */
