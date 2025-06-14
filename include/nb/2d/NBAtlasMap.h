//
//  NBAtlasMap.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/11/18.
//

#ifndef NBAtlasMap_h
#define NBAtlasMap_h

#include "nb/NBFrameworkDefs.h"
#include "nb/2d/NBSize.h"
#include "nb/2d/NBRect.h"
#include "nb/2d/NBAABox.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBAtlasMap_ {
		void* opaque;
	} STNBAtlasMap;
	
	//void NBAtlasMap_init(STNBAtlasMap* obj);
	void NBAtlasMap_initWithSz(STNBAtlasMap* obj, const SI32 scaleBase2, const SI32 width, const SI32 height);
	void NBAtlasMap_retain(STNBAtlasMap* obj);
	void NBAtlasMap_release(STNBAtlasMap* obj);
	
	//Props
	UI32 		NBAtlasMap_getAreasCount(const STNBAtlasMap* obj);
	BOOL 		NBAtlasMap_getAreasAABox(const STNBAtlasMap* obj, STNBAABox* dst);
	BOOL 		NBAtlasMap_getSize(const STNBAtlasMap* obj, STNBSize* dst);
	UI8  		NBAtlasMap_getScaleBase2(const STNBAtlasMap* obj);
	
	//Search
	STNBRectI	NBAtlasMap_getAreaAtIndex(const STNBAtlasMap* obj, const UI32 idx);
	BOOL		NBAtlasMap_hasRoomFor(const STNBAtlasMap* obj, const SI32 width, const SI32 height, const SI32 hMargin, const SI32 vMargin);
	
	//Build
	void		NBAtlasMap_empty(STNBAtlasMap* obj);
	void		NBAtlasMap_setSize(STNBAtlasMap* obj, const SI32 width, const SI32 height);
	void		NBAtlasMap_addArea(STNBAtlasMap* obj, const SI32 x, const SI32 y, const SI32 width, const SI32 height);
	BOOL		NBAtlasMap_setAreaAtIndex(STNBAtlasMap* obj, const UI32 idx, const SI32 x, const SI32 y, const SI32 width, const SI32 height);
	STNBRectI	NBAtlasMap_reserveArea(STNBAtlasMap* obj, const SI32 width, const SI32 height, const SI32 hMargin, const SI32 vMargin);
	UI32		NBAtlasMap_removeArea(STNBAtlasMap* obj, const SI32 x, const SI32 y, const SI32 width, const SI32 height);
	BOOL		NBAtlasMap_removeAreaAtIndex(STNBAtlasMap* obj, const UI32 idx);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBAtlasMap_h */
