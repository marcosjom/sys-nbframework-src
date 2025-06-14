//
//  NBArray.h
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#ifndef NBLocaleCfgStats_h
#define NBLocaleCfgStats_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBLocaleCfg.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	//Count pen eln
	
	typedef struct STNBLocaleCfgStatsCountByLen_ {
		UI32			len;		//including spaces and punctuations
		UI32			count;		//only chars in words
	} STNBLocaleCfgStatsCountByLen;
	
	const STNBStructMap* NBLocaleCfgStatsCountByLen_getSharedStructMap(void);
	BOOL NBCompare_STNBLocaleCfgStatsCountByLen(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);
	
	typedef struct STNBLocaleCfgStatsCountByLenArr_ {
		STNBLocaleCfgStatsCountByLen*	counts;		//counts
		UI32							countsSz;	//counts
	} STNBLocaleCfgStatsCountByLenArr;
	
	const STNBStructMap* NBLocaleCfgStatsCountByLenArr_getSharedStructMap(void);
	
	//Lang itm
	
	typedef struct STNBLocaleCfgStatsLangLcl_ {
		char*			lcl;				//the text
		char*			hints;				//the original hint (user defined)
		UI32			wordsCount;			//words (including one-char word)
		//UI32			wordsCharsCount;	//only chars in words
		//UI32			charsCount;			//including spaces and punctuations
		char*			otherLangs;			//other langs values (automatic)
		char**			uids;				//multiple uid
		UI32			uidsSz;				//multiple uid
	} STNBLocaleCfgStatsLangLcl;
	
	const STNBStructMap* NBLocaleCfgStatsLangLcl_getSharedStructMap(void);
	BOOL NBCompare_STNBLocaleCfgStatsLangLcl(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);
	
	typedef struct STNBLocaleCfgStatsLangLclArr_ {
		STNBLocaleCfgStatsLangLcl*	lcls;				//Locales
		UI32						lclsSz;				//Locales
	} STNBLocaleCfgStatsLangLclArr;
	
	const STNBStructMap* NBLocaleCfgStatsLangArr_getSharedStructMap(void);
	
	//Lang stats
	
	typedef struct STNBLocaleCfgStatsLang_ {
		char*								lng;
		UI32								wordsCount;			//words (including one-char word)
		//UI32								wordsCharsCount;	//only chars in words
		//UI32								charsCount;			//including spaces and punctuations
		STNBLocaleCfgStatsCountByLenArr*	countsByLen;		//Counts
		STNBLocaleCfgStatsLangLclArr*		lcls;				//Locales
	} STNBLocaleCfgStatsLang;
	
	const STNBStructMap* NBLocaleCfgStatsLang_getSharedStructMap(void);
	
	typedef struct STNBLocaleCfgStatsLangArr_ {
		STNBLocaleCfgStatsLang*		langs;				//langs
		UI32						langsSz;			//langs
	} STNBLocaleCfgStatsLangArr;
	
	const STNBStructMap* NBLocaleCfgStatsLangArr_getSharedStructMap(void);
	
	//Global stats
	
	typedef struct STNBLocaleCfgStats_ {
		UI32						uidsCount;			//uid total
		UI32						langsCount;			//languages total
		STNBLocaleCfgStatsLangArr*	langs;				//langs
	} STNBLocaleCfgStats;
	
	const STNBStructMap* NBLocaleCfgStats_getSharedStructMap(void);
	
	//Analyzer
	
	typedef struct STNBLocaleCfgAnlz_ {
		void* opaque;
	} STNBLocaleCfgAnlz;
	
	void NBLocaleCfgAnlz_init(STNBLocaleCfgAnlz* obj);
	void NBLocaleCfgAnlz_release(STNBLocaleCfgAnlz* obj);
	
	//Analyze
	BOOL NBLocaleCfgAnlz_getResults(STNBLocaleCfgAnlz* obj, STNBLocaleCfgStats* dst);
	BOOL NBLocaleCfgAnlz_addLocaleFilepath(STNBLocaleCfgAnlz* obj, const char* filepath);
	BOOL NBLocaleCfgAnlz_addLocale(STNBLocaleCfgAnlz* obj, const STNBLocaleCfg* src);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
