//
//  NBRange.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/12/18.
//

#ifndef NBRange_h
#define NBRange_h

#include "nb/NBFrameworkDefs.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBRange_ {
		float	start;
		float	size;
	} STNBRange;
	
	typedef struct STNBRangeI_ {
		SI32	start;
		SI32	size;
	} STNBRangeI;
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBRange_h */
