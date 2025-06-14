

#ifndef NB_STRINGS_POOL_H
#define NB_STRINGS_POOL_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBThreadMutex.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBStringsPoolItm_ {
		STNBString*			str;
		UI32				retainCount;
	} STNBStringsPoolItm;
	
	typedef struct STNBStringsPool_ {
		STNBArray		itms;		//STNBStringsPoolItm
		STNBThreadMutex	itmsMutex;
		UI32			strsLenInitial;
		UI32			strsLenMinGrowth;
	} STNBStringsPool;
	
	//Init and release
	void NBStringsPool_init(STNBStringsPool* obj, const UI32 strsLenInitial, const UI32 strsLenMinGrowth);
	void NBStringsPool_release(STNBStringsPool* obj);
	
	//Get or add conn
	STNBString* NBStringsPool_get(STNBStringsPool* obj);
	BOOL NBStringsPool_isChild(STNBStringsPool* obj, const STNBString* str);
	BOOL NBStringsPool_return(STNBStringsPool* obj, const STNBString* str);
	
	//
	void NBStringsPool_empty(STNBStringsPool* obj);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
