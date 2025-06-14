//
//  NBFontCodesMap.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/12/18.
//

#ifndef NBFontCodesMap_h
#define NBFontCodesMap_h

#include "nb/NBFrameworkDefs.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBFontCodesMap_ {
		void* opaque;
	} STNBFontCodesMap;
	
	void NBFontCodesMap_init(STNBFontCodesMap* obj);
	void NBFontCodesMap_retain(STNBFontCodesMap* obj);
	void NBFontCodesMap_release(STNBFontCodesMap* obj);
	
	BOOL NBFontCodesMap_getShapeId(const STNBFontCodesMap* obj, const UI32 unicode, UI32* dstShapeId);
	void NBFontCodesMap_addRecord(STNBFontCodesMap* obj, const UI32 unicode, const UI32 shapeId);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBFontCodesMap_h */
