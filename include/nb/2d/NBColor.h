//
//  NBColor.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/12/18.
//

#ifndef NBColor_h
#define NBColor_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBCompare.h"
#include "nb/core/NBStructMap.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBColor_ {
		union {
			struct {
				float r, g, b, a;
			};
			float c[4];
		};
	} STNBColor;
	
	const STNBStructMap* NBColor_getSharedStructMap(void);
	
	typedef struct STNBColor8_ {
		union {
			struct {
				UI8 r, g, b, a;
			};
			UI8 c[4];
		};
	} STNBColor8;
	
	const STNBStructMap* NBColor8_getSharedStructMap(void);
	
	typedef struct STNBColorI_ {
		union {
			struct {
				SI32 r, g, b, a;
			};
			SI32 c[4];
		};
	} STNBColorI;
	
	const STNBStructMap* NBColorI_getSharedStructMap(void);
	
	BOOL NBCompare_NBColor8(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBColor_h */
