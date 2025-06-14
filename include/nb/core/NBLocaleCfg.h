//
//  NBArray.h
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#ifndef NBLocaleCfg_h
#define NBLocaleCfg_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBStruct.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	//Pair
	
	typedef struct STNBLocaleCfgItmPair_ {
		char*		lng;		//lang
		char*		lcl;		//value
		UI64		revTime;	//revision date
		char*		revName;	//revision name
	} STNBLocaleCfgItmPair;
	
	const STNBStructMap* NBLocaleCfgItmPair_getSharedStructMap(void);
	BOOL NBCompare_STNBLocaleCfgItmPair(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);
	
	//Itm
	
	typedef struct STNBLocaleCfgItm_ {
		char*					uid;
		char*					hint;
		STNBLocaleCfgItmPair*	lcls;
		UI32					lclsSz;
	} STNBLocaleCfgItm;
	
	const STNBStructMap* NBLocaleCfgItm_getSharedStructMap(void);
	
	//Cfg
	
	typedef struct STNBLocaleCfg_ {
		STNBLocaleCfgItm*	itms;
		UI32				itmsSz;
	} STNBLocaleCfg;
	
	const STNBStructMap* NBLocaleCfg_getSharedStructMap(void);
	BOOL NBCompare_STNBLocaleCfgItm(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
