//
//  NBArray.h
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#ifndef NBLocale_h
#define NBLocale_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBLocaleCfg.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBLocale_ {
		void*		opaque;
	} STNBLocale;
	
	void		NBLocale_init(STNBLocale* obj);
	void		NBLocale_retain(STNBLocale* obj);
	void		NBLocale_release(STNBLocale* obj);
	//
	BOOL		NBLocale_loadFromBytes(STNBLocale* obj, const void* data, const UI32 dataSz);
	BOOL		NBLocale_loadFromFile(STNBLocale* obj, STNBFileRef file);
	BOOL		NBLocale_loadFromFilePath(STNBLocale* obj, const char* filePath);
	//
	BOOL		NBLocale_pushPreferedLang(STNBLocale* obj, const char* lang);
	//
	const char* NBLocale_getPreferedLangAtIdx(STNBLocale* obj, const UI32 idx, const char* lngDef);
	const char* NBLocale_getStr(STNBLocale* obj, const char* uid, const char* strDefault);
	const char* NBLocale_getStrLang(STNBLocale* obj, const char* lang, const char* uid, const char* strDefault);
	//Merge configs
	void		NBLocaleCfg_merge(STNBLocaleCfg* srcAndDst, const STNBLocaleCfg* other, const BOOL forceAllNotEmptyValues);
	void		NBLocaleCfg_mergeUsingDuplicates(STNBLocaleCfg* srcAndDst, const char* langBase, const char* langToFill);
	//Build json
	void		NBLocaleCfg_concatAsJson(const STNBLocaleCfg* src, STNBString* dst);
	void		NBLocaleCfg_concatAsJsonOnlyEmptyLang(const STNBLocaleCfg* src, const char* emptyLang, STNBString* dst);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
