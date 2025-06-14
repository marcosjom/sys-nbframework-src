//
//  XUCadena.c
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBLocaleCfgAnlz.h"
//
#include "nb/core/NBMngrStructMaps.h"
#include "nb/core/NBArraySorted.h"
#include "nb/core/NBEncoding.h"

//Global stats

STNBStructMapsRec NBLocaleCfgStats_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBLocaleCfgStats_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBLocaleCfgStats_sharedStructMap);
	if(NBLocaleCfgStats_sharedStructMap.map == NULL){
		STNBLocaleCfgStats s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBLocaleCfgStats);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addUIntM(map, s, uidsCount);	//uid total
		NBStructMap_addUIntM(map, s, langsCount);	//languages total
		NBStructMap_addStructPtrM(map, s, langs, NBLocaleCfgStatsLangArr_getSharedStructMap());
		NBLocaleCfgStats_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBLocaleCfgStats_sharedStructMap);
	return NBLocaleCfgStats_sharedStructMap.map;
}

STNBStructMapsRec NBLocaleCfgStatsLangArr_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBLocaleCfgStatsLangArr_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBLocaleCfgStatsLangArr_sharedStructMap);
	if(NBLocaleCfgStatsLangArr_sharedStructMap.map == NULL){
		STNBLocaleCfgStatsLangArr s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBLocaleCfgStatsLangArr);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addPtrToArrayOfStructM(map, s, langs, langsSz, ENNBStructMapSign_Unsigned, NBLocaleCfgStatsLang_getSharedStructMap());
		NBLocaleCfgStatsLangArr_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBLocaleCfgStatsLangArr_sharedStructMap);
	return NBLocaleCfgStatsLangArr_sharedStructMap.map;
}

//Lang stats

STNBStructMapsRec NBLocaleCfgStatsLangLclArr_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBLocaleCfgStatsLangLclArr_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBLocaleCfgStatsLangLclArr_sharedStructMap);
	if(NBLocaleCfgStatsLangLclArr_sharedStructMap.map == NULL){
		STNBLocaleCfgStatsLangLclArr s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBLocaleCfgStatsLangLclArr);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addPtrToArrayOfStructM(map, s, lcls, lclsSz, ENNBStructMapSign_Unsigned, NBLocaleCfgStatsLangLcl_getSharedStructMap());
		NBLocaleCfgStatsLangLclArr_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBLocaleCfgStatsLangLclArr_sharedStructMap);
	return NBLocaleCfgStatsLangLclArr_sharedStructMap.map;
}

STNBStructMapsRec NBLocaleCfgStatsLang_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBLocaleCfgStatsLang_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBLocaleCfgStatsLang_sharedStructMap);
	if(NBLocaleCfgStatsLang_sharedStructMap.map == NULL){
		STNBLocaleCfgStatsLang s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBLocaleCfgStatsLang);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addStrPtrM(map, s, lng);
		NBStructMap_addUIntM(map, s, wordsCount);		//words (including one-char word)
		//NBStructMap_addUIntM(map, s, wordsCharsCount);	//only chars in words
		//NBStructMap_addUIntM(map, s, charsCount);		//including spaces and punctuations
		NBStructMap_addStructPtrM(map, s, countsByLen, NBLocaleCfgStatsCountByLenArr_getSharedStructMap());
		NBStructMap_addStructPtrM(map, s, lcls, NBLocaleCfgStatsLangLclArr_getSharedStructMap());
		NBLocaleCfgStatsLang_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBLocaleCfgStatsLang_sharedStructMap);
	return NBLocaleCfgStatsLang_sharedStructMap.map;
}

//Lang itm

STNBStructMapsRec NBLocaleCfgStatsLangLcl_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBLocaleCfgStatsLangLcl_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBLocaleCfgStatsLangLcl_sharedStructMap);
	if(NBLocaleCfgStatsLangLcl_sharedStructMap.map == NULL){
		STNBLocaleCfgStatsLangLcl s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBLocaleCfgStatsLangLcl);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addStrPtrM(map, s, lcl);			//the text
		NBStructMap_addStrPtrM(map, s, hints);			//the original hint (user defined)
		NBStructMap_addUIntM(map, s, wordsCount);		//words (including one-char word)
		//NBStructMap_addUIntM(map, s, wordsCharsCount);	//only chars in words
		//NBStructMap_addUIntM(map, s, charsCount);		//including spaces and punctuations
		NBStructMap_addStrPtrM(map, s, otherLangs);		//other langs values (automatic)
		NBStructMap_addPtrToArrayOfStrPtrM(map, s, uids, uidsSz, ENNBStructMapSign_Unsigned); //multiple uid
		NBLocaleCfgStatsLangLcl_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBLocaleCfgStatsLangLcl_sharedStructMap);
	return NBLocaleCfgStatsLangLcl_sharedStructMap.map;
}

BOOL NBCompare_STNBLocaleCfgStatsLangLcl(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBLocaleCfgStatsLangLcl))
	if(dataSz == sizeof(STNBLocaleCfgStatsLangLcl)){
		const STNBLocaleCfgStatsLangLcl* d1 = (const STNBLocaleCfgStatsLangLcl*)data1;
		const STNBLocaleCfgStatsLangLcl* d2 = (const STNBLocaleCfgStatsLangLcl*)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return NBString_strIsEqual(d1->lcl, d2->lcl);
			case ENCompareMode_Lower:
				return NBString_strIsLower(d1->lcl, d2->lcl);
			case ENCompareMode_LowerOrEqual:
				return NBString_strIsLowerOrEqual(d1->lcl, d2->lcl);
			case ENCompareMode_Greater:
				return NBString_strIsGreater(d1->lcl, d2->lcl);
			case ENCompareMode_GreaterOrEqual:
				return NBString_strIsGreaterOrEqual(d1->lcl, d2->lcl);
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}

// Count by len

STNBStructMapsRec NBLocaleCfgStatsCountByLen_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBLocaleCfgStatsCountByLen_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBLocaleCfgStatsCountByLen_sharedStructMap);
	if(NBLocaleCfgStatsCountByLen_sharedStructMap.map == NULL){
		STNBLocaleCfgStatsCountByLen s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBLocaleCfgStatsCountByLen);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addUIntM(map, s, len);
		NBStructMap_addUIntM(map, s, count);
		NBLocaleCfgStatsCountByLen_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBLocaleCfgStatsCountByLen_sharedStructMap);
	return NBLocaleCfgStatsCountByLen_sharedStructMap.map;
}

BOOL NBCompare_STNBLocaleCfgStatsCountByLen(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBLocaleCfgStatsCountByLen))
	if(dataSz == sizeof(STNBLocaleCfgStatsCountByLen)){
		const STNBLocaleCfgStatsCountByLen* d1 = (const STNBLocaleCfgStatsCountByLen*)data1;
		const STNBLocaleCfgStatsCountByLen* d2 = (const STNBLocaleCfgStatsCountByLen*)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return (d1->len == d2->len);
			case ENCompareMode_Lower:
				return (d1->len < d2->len);
			case ENCompareMode_LowerOrEqual:
				return (d1->len <= d2->len);
			case ENCompareMode_Greater:
				return (d1->len > d2->len);
			case ENCompareMode_GreaterOrEqual:
				return (d1->len >= d2->len);
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}

STNBStructMapsRec NBLocaleCfgStatsCountByLenArr_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBLocaleCfgStatsCountByLenArr_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBLocaleCfgStatsCountByLenArr_sharedStructMap);
	if(NBLocaleCfgStatsCountByLenArr_sharedStructMap.map == NULL){
		STNBLocaleCfgStatsCountByLenArr s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBLocaleCfgStatsCountByLenArr);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addPtrToArrayOfStructM(map, s, counts, countsSz, ENNBStructMapSign_Unsigned, NBLocaleCfgStatsCountByLen_getSharedStructMap());
		NBLocaleCfgStatsCountByLenArr_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBLocaleCfgStatsCountByLenArr_sharedStructMap);
	return NBLocaleCfgStatsCountByLenArr_sharedStructMap.map;
}


//Analyzer

typedef struct STNBLocaleCfgAnlzUID_ {
	char*	uid;
} STNBLocaleCfgAnlzUID;

BOOL NBCompare_STNBLocaleCfgAnlzUID(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBLocaleCfgAnlzUID))
	if(dataSz == sizeof(STNBLocaleCfgAnlzUID)){
		const STNBLocaleCfgAnlzUID* d1 = (const STNBLocaleCfgAnlzUID*)data1;
		const STNBLocaleCfgAnlzUID* d2 = (const STNBLocaleCfgAnlzUID*)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return NBString_strIsEqual(d1->uid, d2->uid);
			case ENCompareMode_Lower:
				return NBString_strIsLower(d1->uid, d2->uid);
			case ENCompareMode_LowerOrEqual:
				return NBString_strIsLowerOrEqual(d1->uid, d2->uid);
			case ENCompareMode_Greater:
				return NBString_strIsGreater(d1->uid, d2->uid);
			case ENCompareMode_GreaterOrEqual:
				return NBString_strIsGreaterOrEqual(d1->uid, d2->uid);
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}

typedef struct STNBLocaleCfgAnlzLang_ {
	STNBLocaleCfgStatsLang	stats;
	STNBArraySorted			lcls;			//STNBLocaleCfgStatsLangLcl
	STNBArraySorted			countsByLen;	//STNBLocaleCfgStatsCountByLen
} STNBLocaleCfgAnlzLang;

BOOL NBCompare_STNBLocaleCfgAnlzLang(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBLocaleCfgAnlzLang))
	if(dataSz == sizeof(STNBLocaleCfgAnlzLang)){
		const STNBLocaleCfgAnlzLang* d1 = (const STNBLocaleCfgAnlzLang*)data1;
		const STNBLocaleCfgAnlzLang* d2 = (const STNBLocaleCfgAnlzLang*)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return NBString_strIsEqual(d1->stats.lng, d2->stats.lng);
			case ENCompareMode_Lower:
				return NBString_strIsLower(d1->stats.lng, d2->stats.lng);
			case ENCompareMode_LowerOrEqual:
				return NBString_strIsLowerOrEqual(d1->stats.lng, d2->stats.lng);
			case ENCompareMode_Greater:
				return NBString_strIsGreater(d1->stats.lng, d2->stats.lng);
			case ENCompareMode_GreaterOrEqual:
				return NBString_strIsGreaterOrEqual(d1->stats.lng, d2->stats.lng);
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}

typedef struct STNBLocaleCfgAnlzOpq_ {
	STNBArraySorted		uids;	//STNBLocaleCfgAnlzUID
	STNBArraySorted		langs;	//STNBLocaleCfgAnlzLang
} STNBLocaleCfgAnlzOpq;

void NBLocaleCfgAnlz_init(STNBLocaleCfgAnlz* obj){
	STNBLocaleCfgAnlzOpq* opq = obj->opaque = NBMemory_allocType(STNBLocaleCfgAnlzOpq);
	NBMemory_setZeroSt(*opq, STNBLocaleCfgAnlzOpq);
	//
	NBArraySorted_init(&opq->uids, sizeof(STNBLocaleCfgAnlzUID), NBCompare_STNBLocaleCfgAnlzUID);
	NBArraySorted_init(&opq->langs, sizeof(STNBLocaleCfgAnlzLang), NBCompare_STNBLocaleCfgAnlzLang);
}

void NBLocaleCfgAnlz_release(STNBLocaleCfgAnlz* obj){
	STNBLocaleCfgAnlzOpq* opq = (STNBLocaleCfgAnlzOpq*)obj->opaque;
	if(opq != NULL){
		//uids
		{
			SI32 i; for(i = 0; i < opq->uids.use; i++){
				STNBLocaleCfgAnlzUID* itm = NBArraySorted_itmPtrAtIndex(&opq->uids, STNBLocaleCfgAnlzUID, i);
				if(itm->uid != NULL) NBMemory_free(itm->uid); itm->uid = NULL;
			}
			NBArraySorted_empty(&opq->uids);
			NBArraySorted_release(&opq->uids);
		}
		//langs
		{
			const STNBStructMap* lngMap	= NBLocaleCfgStatsLang_getSharedStructMap();
			const STNBStructMap* lclMap = NBLocaleCfgStatsLangLcl_getSharedStructMap();
			const STNBStructMap* cntMap = NBLocaleCfgStatsCountByLen_getSharedStructMap();
			SI32 i; for(i = 0; i < opq->langs.use; i++){
				STNBLocaleCfgAnlzLang* itm = NBArraySorted_itmPtrAtIndex(&opq->langs, STNBLocaleCfgAnlzLang, i);
				NBStruct_stRelease(lngMap, &itm->stats, sizeof(itm->stats));
				//Lcls
				{
					SI32 i; for(i = 0; i < itm->lcls.use; i++){
						STNBLocaleCfgStatsLangLcl* lcl = NBArraySorted_itmPtrAtIndex(&itm->lcls, STNBLocaleCfgStatsLangLcl, i);
						NBStruct_stRelease(lclMap, lcl, sizeof(*lcl));
					}
					NBArraySorted_empty(&itm->lcls);
					NBArraySorted_release(&itm->lcls);
				}
				//Counts by len
				{
					SI32 i; for(i = 0; i < itm->countsByLen.use; i++){
						STNBLocaleCfgStatsCountByLen* lcl = NBArraySorted_itmPtrAtIndex(&itm->countsByLen, STNBLocaleCfgStatsCountByLen, i);
						NBStruct_stRelease(cntMap, lcl, sizeof(*lcl));
					}
					NBArraySorted_empty(&itm->countsByLen);
					NBArraySorted_release(&itm->countsByLen);
				}
			}
			NBArraySorted_empty(&opq->langs);
			NBArraySorted_release(&opq->langs);
		}
		//
		NBMemory_free(opq);
		obj->opaque = NULL;
	}
}

//

BOOL NBLocaleCfgAnlz_getResults(STNBLocaleCfgAnlz* obj, STNBLocaleCfgStats* dst){
	BOOL r = FALSE;
	STNBLocaleCfgAnlzOpq* opq = (STNBLocaleCfgAnlzOpq*)obj->opaque;
	if(opq != NULL && dst != NULL){
		r = TRUE;
		//Global
		{
			dst->uidsCount	= opq->uids.use;
			dst->langsCount	= opq->langs.use;
		}
		//Langs
		{
			//Release previous
			if(dst->langs != NULL){
				NBStruct_stRelease(NBLocaleCfgStatsLangArr_getSharedStructMap(), dst->langs, sizeof(*dst->langs));
				NBMemory_free(dst->langs);
				dst->langs = NULL;
			}
			//Create new array
			if(opq->langs.use > 0){
				STNBLocaleCfgStatsLangArr* nLangs = NBMemory_allocType(STNBLocaleCfgStatsLangArr);
				NBMemory_setZeroSt(*nLangs, STNBLocaleCfgStatsLangArr);
				nLangs->langsSz	= opq->langs.use;
				nLangs->langs	= NBMemory_allocTypes(STNBLocaleCfgStatsLang, opq->langs.use);
				{
					const STNBStructMap* lngMap = NBLocaleCfgStatsLang_getSharedStructMap();
					const STNBStructMap* lclMap = NBLocaleCfgStatsLangLcl_getSharedStructMap();
					const STNBStructMap* cntMap = NBLocaleCfgStatsCountByLen_getSharedStructMap();
					SI32 i; for(i = 0; i < opq->langs.use; i++){
						const STNBLocaleCfgAnlzLang* lng = NBArraySorted_itmPtrAtIndex(&opq->langs, STNBLocaleCfgAnlzLang, i);
						STNBLocaleCfgStatsLang* lngDst = &nLangs->langs[i];
						NBMemory_setZeroSt(*lngDst, STNBLocaleCfgStatsLang);
						NBStruct_stClone(lngMap, &lng->stats, sizeof(lng->stats), lngDst, sizeof(*lngDst));
						//Clone lcls
						{
							//Release previous
							if(lngDst->lcls != NULL){
								NBStruct_stRelease(NBLocaleCfgStatsLangLclArr_getSharedStructMap(), lngDst->lcls, sizeof(*lngDst->lcls));
								NBMemory_free(lngDst->lcls);
								lngDst->lcls = NULL;
							}
							//Create new array
							if(lng->lcls.use > 0){
								STNBLocaleCfgStatsLangLclArr* nLcls = NBMemory_allocType(STNBLocaleCfgStatsLangLclArr);
								NBMemory_setZeroSt(*nLcls, STNBLocaleCfgStatsLangLclArr);
								nLcls->lclsSz	= lng->lcls.use;
								nLcls->lcls		= NBMemory_allocTypes(STNBLocaleCfgStatsLangLcl, lng->lcls.use);
								{
									SI32 i; for(i = 0; i < lng->lcls.use; i++){
										STNBLocaleCfgStatsLangLcl* srcLng	= NBArraySorted_itmPtrAtIndex(&lng->lcls, STNBLocaleCfgStatsLangLcl, i);
										STNBLocaleCfgStatsLangLcl* dstLng	= &nLcls->lcls[i];
										NBMemory_setZeroSt(*dstLng, STNBLocaleCfgStatsLangLcl);
										NBStruct_stClone(lclMap, srcLng, sizeof(*srcLng), dstLng, sizeof(*dstLng));
									}
								}
								lngDst->lcls = nLcls;
							}
						}
						//Clone countsByLen
						{
							//Release previous
							if(lngDst->countsByLen != NULL){
								NBStruct_stRelease(NBLocaleCfgStatsCountByLenArr_getSharedStructMap(), lngDst->countsByLen, sizeof(*lngDst->countsByLen));
								NBMemory_free(lngDst->countsByLen);
								lngDst->countsByLen = NULL;
							}
							//Create new array
							if(lng->countsByLen.use > 0){
								STNBLocaleCfgStatsCountByLenArr* nCnts = NBMemory_allocType(STNBLocaleCfgStatsCountByLenArr);
								NBMemory_setZeroSt(*nCnts, STNBLocaleCfgStatsCountByLenArr);
								nCnts->countsSz	= lng->countsByLen.use;
								nCnts->counts	= NBMemory_allocTypes(STNBLocaleCfgStatsCountByLen, lng->countsByLen.use);
								{
									SI32 i; for(i = 0; i < lng->countsByLen.use; i++){
										STNBLocaleCfgStatsCountByLen* srcLng	= NBArraySorted_itmPtrAtIndex(&lng->countsByLen, STNBLocaleCfgStatsCountByLen, i);
										STNBLocaleCfgStatsCountByLen* dstLng	= &nCnts->counts[i];
										NBMemory_setZeroSt(*dstLng, STNBLocaleCfgStatsCountByLen);
										NBStruct_stClone(cntMap, srcLng, sizeof(*srcLng), dstLng, sizeof(*dstLng));
									}
								}
								lngDst->countsByLen = nCnts;
							}
						}
					}
				}
				dst->langs = nLangs;
			}
		}
	}
	return r;
}


//Analyze

BOOL NBLocaleCfgAnlz_addLocaleFilepath(STNBLocaleCfgAnlz* obj, const char* filepath){
	BOOL r = FALSE;
	STNBLocaleCfgAnlzOpq* opq = (STNBLocaleCfgAnlzOpq*)obj->opaque;
	if(opq != NULL){
		if(!NBString_strIsEmpty(filepath)){
			const STNBStructMap* stMap = NBLocaleCfg_getSharedStructMap();
			STNBLocaleCfg cfg;
			NBMemory_setZeroSt(cfg, STNBLocaleCfg);
			if(!NBStruct_stReadFromJsonFilepath(filepath, stMap, &cfg, sizeof(cfg))){
				PRINTF_ERROR("Could not load locale: '%s'.\n", filepath);
			} else {
				r = NBLocaleCfgAnlz_addLocale(obj, &cfg);
			}
			NBStruct_stRelease(stMap, &cfg, sizeof(cfg));
		}
	}
	return r;
}

BOOL NBLocaleCfgAnlz_addLocale(STNBLocaleCfgAnlz* obj, const STNBLocaleCfg* src){
	BOOL r = FALSE;
	STNBLocaleCfgAnlzOpq* opq = (STNBLocaleCfgAnlzOpq*)obj->opaque;
	if(opq != NULL){
		if(src != NULL){
			r = TRUE;
			{
				STNBString str, str2;
				NBString_init(&str);
				NBString_init(&str2);
				{
					UI32 i; for(i = 0; i < src->itmsSz; i++){
						const STNBLocaleCfgItm* itm = &src->itms[i];
						if(NBString_strIsEmpty(itm->uid)){
							PRINTF_ERROR("NBLocaleCfgAnlz, no uid at itm #%d of %d.\n", (i + 1), src->itmsSz);
						} else {
							//Search in already analyzed uids
							STNBLocaleCfgAnlzUID srch;
							NBMemory_setZeroSt(srch, STNBLocaleCfgAnlzUID);
							srch.uid = itm->uid;
							if(NBArraySorted_indexOf(&opq->uids, &srch, sizeof(srch), NULL) >= 0){
								PRINTF_ERROR("NBLocaleCfgAnlz, duplicated uid('%s') at itm #%d of %d.\n", itm->uid, (i + 1), src->itmsSz);
							} else {
								//Add uid
								{
									STNBLocaleCfgAnlzUID n;
									NBMemory_setZeroSt(n, STNBLocaleCfgAnlzUID);
									NBString_strFreeAndNewBuffer(&n.uid, itm->uid);
									NBArraySorted_add(&opq->uids, &n, sizeof(n));
								}
								//Process pairs
								if(itm->lcls != NULL && itm->lclsSz > 0){
									SI32 i; for(i = 0; i < itm->lclsSz; i++){
										const STNBLocaleCfgItmPair* lcl = &itm->lcls[i];
										if(NBString_strIsEmpty(lcl->lng)){
											PRINTF_ERROR("NBLocaleCfgAnlz, no lngId at itm #%d of %d.\n", (i + 1), src->itmsSz);
										} else {
											STNBLocaleCfgAnlzLang* lng = NULL;
											//Search lang
											{
												SI32 idx		= -1;
												STNBLocaleCfgAnlzLang srch;
												NBMemory_setZeroSt(srch, STNBLocaleCfgAnlzLang);
												srch.stats.lng	= lcl->lng;
												idx				= NBArraySorted_indexOf(&opq->langs, &srch, sizeof(srch), NULL);
												if(idx >= 0){
													lng			= NBArraySorted_itmPtrAtIndex(&opq->langs, STNBLocaleCfgAnlzLang, idx);
												} else {
													STNBLocaleCfgAnlzLang n;
													NBMemory_setZeroSt(n, STNBLocaleCfgAnlzLang);
													NBString_strFreeAndNewBuffer(&n.stats.lng, lcl->lng);
													NBArraySorted_init(&n.lcls, sizeof(STNBLocaleCfgStatsLangLcl), NBCompare_STNBLocaleCfgStatsLangLcl);
													NBArraySorted_init(&n.countsByLen, sizeof(STNBLocaleCfgStatsCountByLen), NBCompare_STNBLocaleCfgStatsCountByLen);
													lng = (STNBLocaleCfgAnlzLang*)NBArraySorted_add(&opq->langs, &n, sizeof(n));
												}
											}
											//Process itms
											NBASSERT(lng != NULL)
											if(lng != NULL){
												STNBLocaleCfgStatsLangLcl* record = NULL;
												//Search for locale content
												{
													SI32 idx	= -1;
													STNBLocaleCfgStatsLangLcl srch;
													NBMemory_setZeroSt(srch, STNBLocaleCfgStatsLangLcl);
													srch.lcl	= lcl->lcl;
													idx			= NBArraySorted_indexOf(&lng->lcls, &srch, sizeof(srch), NULL);
													if(idx >= 0){
														//Found before
														record	= NBArraySorted_itmPtrAtIndex(&lng->lcls, STNBLocaleCfgStatsLangLcl, idx);
													} else {
														//New found
														STNBLocaleCfgStatsLangLcl n;
														NBMemory_setZeroSt(n, STNBLocaleCfgStatsLangLcl);
														NBString_strFreeAndNewBuffer(&n.lcl, lcl->lcl);
														record	= (STNBLocaleCfgStatsLangLcl*)NBArraySorted_addValue(&lng->lcls, n);
													}
												}
												NBASSERT(record != NULL)
												if(record != NULL){
													//Add hint (user defined)
													if(!NBString_strIsEmpty(itm->hint)){
														if(NBString_strIndexOf(record->hints, itm->hint, 0) < 0){
															//Add hint
															NBString_empty(&str);
															NBString_concat(&str, record->hints);
															/*if(str.length > 0)*/ NBString_concat(&str, "\n");
															NBString_concat(&str, "(hint)");
															NBString_concat(&str, itm->hint);
															NBString_strFreeAndNewBufferBytes(&record->hints, str.str, str.length);
															NBString_release(&str);
														}
													}
													//Add other languages
													if(NBString_strIsEmpty(record->otherLangs)){
														NBString_empty(&str);
														SI32 i; for(i = (itm->lclsSz - 1); i >= 0; i--){
															const STNBLocaleCfgItmPair* lcl2 = &itm->lcls[i];
															//if(!NBString_strIsEqual(lcl->lng, lcl2->lng)) //Include current language
															{
																if(!NBString_strIsEmpty(lcl2->lcl)){
																	/*if(str.length > 0)*/ NBString_concat(&str, "\n");
																	NBString_concat(&str, "(");
																	NBString_concat(&str, lcl2->lng);
																	NBString_concat(&str, ") ");
																	NBString_concat(&str, lcl2->lcl);
																}
															}
														}
														NBString_strFreeAndNewBufferBytes(&record->otherLangs, str.str, str.length);
													}
													//Add uid
													{
														STNBArraySorted uids;
														NBArraySorted_initWithSz(&uids, sizeof(char*), NBCompare_charPtr, record->uidsSz + 1, 4, 0.10f);
														//Add current
														if(record->uids != NULL && record->uids > 0){
															NBArraySorted_addItems(&uids, record->uids, sizeof(record->uids[0]), record->uidsSz);
														}
														//Add new
														{
															char* uid = NBString_strNewBuffer(itm->uid);
															NBArraySorted_addValue(&uids, uid);
														}
														//Swapt buffers
														{
															if(record->uids != NULL) NBMemory_free(record->uids);
															record->uidsSz	= uids.use;
															record->uids	= (char**)NBArraySorted_data(&uids);
															NBArraySorted_resignToBuffer(&uids);
														}
														NBArraySorted_release(&uids);
													}
													//Process words
													{
														const char* wordsSeparators = "\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037 !\"#$%&'()*+,./0123456789:;<=>?@[\\]^_`{|}~¿¡«»│";
														const char* val = lcl->lcl;
														const UI32 valLenBytes = NBString_strLenBytes(val);
														UI32 iPos = 0, iStartWord = 0, uniAccum = 0;
														UI8 bytesExpected = 0; BOOL isSeparator = FALSE;
														while(iPos <= valLenBytes){ //including the '\0' at the end
															bytesExpected = NBEncoding_utf8BytesExpected(val[iPos]);
															if(bytesExpected <= 0){
																PRINTF_ERROR("NBLocaleCfgAnlz, malformed UTF8 at uid('%s') lng('%s').\n", itm->uid, lcl->lng);
																break;
															} else {
																NBString_setBytes(&str, &val[iPos], bytesExpected);
																if(val[iPos] == '\0'){
																	isSeparator = TRUE;
																} else {
																	isSeparator = (NBString_strIndexOf(wordsSeparators, str.str, 0) >= 0);
																}
																if(!isSeparator){
																	//Accumulate
																	uniAccum++;
																	//lng->stats.wordsCharsCount++;
																	//record->wordsCharsCount++;
																} else {
																	//Separator found, process accumlated word
																	NBASSERT((uniAccum == 0 && iStartWord == iPos) || (uniAccum > 0 && iStartWord < iPos))
																	if(iStartWord < iPos){
																		NBASSERT(uniAccum > 0)
																		NBString_setBytes(&str, &val[iStartWord], (iPos - iStartWord));
																		PRINTF_INFO("Word '%s', uni(%d) utf8(%d).\n", str.str, uniAccum, (iPos - iStartWord));
																		//Accumulate word
																		{
																			STNBLocaleCfgStatsCountByLen* record = NULL;
																			//Search for locale content
																			SI32 idx	= -1;
																			STNBLocaleCfgStatsCountByLen srch;
																			NBMemory_setZeroSt(srch, STNBLocaleCfgStatsCountByLen);
																			srch.len	= uniAccum;
																			idx			= NBArraySorted_indexOf(&lng->countsByLen, &srch, sizeof(srch), NULL);
																			if(idx >= 0){
																				//Found before
																				record	= NBArraySorted_itmPtrAtIndex(&lng->countsByLen, STNBLocaleCfgStatsCountByLen, idx);
																			} else {
																				//New found
																				STNBLocaleCfgStatsCountByLen n;
																				NBMemory_setZeroSt(n, STNBLocaleCfgStatsCountByLen);
																				n.len	= uniAccum;
																				record	= (STNBLocaleCfgStatsCountByLen*)NBArraySorted_addValue(&lng->countsByLen, n);
																			}
																			NBASSERT(record != NULL)
																			if(record != NULL){
																				record->count++;
																			}
																		}
																		lng->stats.wordsCount++;
																		record->wordsCount++;
																	}
																	//Start next word
																	iStartWord	= iPos + bytesExpected;
																	uniAccum	= 0;
																}
																//lng->stats.charsCount++;
																//record->charsCount++;
																iPos += bytesExpected;
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
				NBString_release(&str);
				NBString_release(&str2);
			}
		}
	}
	return r;
}


