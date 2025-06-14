
#include "nb/NBFrameworkPch.h"
#include "nb/core/NBMngrStructMaps.h"
#include "nb/core/NBMemory.h"

static STNBStructMaps* __globalStructMaps = NULL;

void NBMngrStructMaps_init(void){
	__globalStructMaps = NBMemory_allocType(STNBStructMaps);
	NBStructMaps_init(__globalStructMaps);
}

void NBMngrStructMaps_release(void){
	NBASSERT(__globalStructMaps != NULL)
	if(__globalStructMaps != NULL){
		NBStructMaps_release(__globalStructMaps);
		NBMemory_free(__globalStructMaps);
		__globalStructMaps = NULL;
	}
}

//lock

void NBMngrStructMaps_lock(STNBStructMapsRec* map){
	NBASSERT(__globalStructMaps != NULL)
	if(__globalStructMaps != NULL){
		NBStructMaps_lock(__globalStructMaps, map);
	}
}

void NBMngrStructMaps_unlock(STNBStructMapsRec* map){
	NBASSERT(__globalStructMaps != NULL)
	if(__globalStructMaps != NULL){
		NBStructMaps_unlock(__globalStructMaps, map);
	}
}

//

STNBStructMap* NBMngrStructMaps_allocType(const char* name){
	STNBStructMap* r = NULL;
	NBASSERT(__globalStructMaps != NULL)
	if(__globalStructMaps != NULL){
		r = NBStructMaps_allocType(__globalStructMaps, name);
	}
	return r;
}
