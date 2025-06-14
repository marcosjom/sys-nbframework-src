//
//  NBBezier3.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/6/18.
//

#ifndef NBBezier3_h
#define NBBezier3_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBCompare.h"
#include "nb/core/NBStructMap.h"
#include "nb/2d/NBPoint.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBBezier3_ {
		STNBPoint	start;
		STNBPoint	ref1;
		STNBPoint	ref2;
		STNBPoint	end;
	} STNBBezier3;
	
	const STNBStructMap* NBBezier3_getSharedStructMap(void);
	
	//
	
	STNBPoint NBBezier3_getMidPoint(const STNBBezier3* obj, const float relPos);
	
	//
	
	STNBPoint NBBezier3_bezGetMidPoint(const STNBPoint start, const STNBPoint ref1, const STNBPoint ref2, const STNBPoint end, const float relPos);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBBezier3_h */
