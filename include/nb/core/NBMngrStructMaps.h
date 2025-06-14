
#ifndef NB_MNGR_STRUCT_MAPS_H
#define NB_MNGR_STRUCT_MAPS_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBStructMaps.h"

#define NBMngrStructMaps_allocTypeM(TYPE)	NBMngrStructMaps_allocType(#TYPE)

#ifdef __cplusplus
extern "C" {
#endif
	
	void NBMngrStructMaps_init(void);
	void NBMngrStructMaps_release(void);
	
	//lock
	
	void NBMngrStructMaps_lock(STNBStructMapsRec* map);
	void NBMngrStructMaps_unlock(STNBStructMapsRec* map);
	
	//records
	
	STNBStructMap* NBMngrStructMaps_allocType(const char* name);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
