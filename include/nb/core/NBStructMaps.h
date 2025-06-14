
#ifndef NB_STRUCT_MAPS_H
#define NB_STRUCT_MAPS_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBThreadMutex.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBStructMapsRec_ {
		STNBStructMap*	map;			//Map
		BOOL			isInited;		//Record var inited
		BOOL			isPopulated;	//Map populated
		BOOL			isLocked;		//Record locked
		STNBThreadMutex	mutex;			//Mutex
	} STNBStructMapsRec;
	
#	define STNBStructMapsRec_empty		{ NULL, FALSE, FALSE, FALSE, { NULL } }
	
	typedef struct STNBStructMaps_ {
		void* opaque;
	} STNBStructMaps;
	
	void NBStructMaps_init(STNBStructMaps* obj);
	void NBStructMaps_release(STNBStructMaps* obj);
	
	//lock
	
	void NBStructMaps_lock(STNBStructMaps* obj, STNBStructMapsRec* map);
	void NBStructMaps_unlock(STNBStructMaps* obj, STNBStructMapsRec* map);
	
	//records
	
	STNBStructMap* NBStructMaps_allocType(STNBStructMaps* obj, const char* name);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
