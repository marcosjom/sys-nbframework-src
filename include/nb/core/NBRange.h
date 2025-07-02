//
//  NBRange.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/12/18.
//

#ifndef NBRange_h
#define NBRange_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBStructMap.h"

#ifdef __cplusplus
extern "C" {
#endif
	
#define STNBRange_Zero   { 0.f, 0.f }

	typedef struct STNBRange_ {
		float	start;
		float	size;
	} STNBRange;

    BOOL NBCompare_STNBRange(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);
    const STNBStructMap* NBRange_getSharedStructMap(void);
	
#define STNBRangeI_Zero   { 0, 0 }

	typedef struct STNBRangeI_ {
		SI32	start;
		SI32	size;
	} STNBRangeI;

    BOOL NBCompare_STNBRangeI(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);
    const STNBStructMap* NBRangeI_getSharedStructMap(void);

#define STNBRangeU_Zero   { 0u, 0u }

    typedef struct STNBRangeU_ {
        UI32    start;
        UI32    size;
    } STNBRangeU;

    BOOL NBCompare_STNBRangeU(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);
    const STNBStructMap* NBRangeU_getSharedStructMap(void);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBRange_h */
