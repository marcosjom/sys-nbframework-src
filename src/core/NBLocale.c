//
//  XUCadena.c
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBLocale.h"
//
#include "nb/core/NBArraySorted.h"

//Lang

typedef struct STNBLocaleLang_ {
	char*			lang;
} STNBLocaleLang;

//Key Idx

typedef struct STNBLocaleKeyIdx_ {
	UI32			iStr;		//in strs
	STNBString*		strs;		//ref
	UI32			keyPos;		//in array
} STNBLocaleKeyIdx;

BOOL NBCompare_STNBLocaleKeyIdx(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBLocaleKeyIdx))
	if(dataSz == sizeof(STNBLocaleKeyIdx)){
		const STNBLocaleKeyIdx* d1 = (const STNBLocaleKeyIdx*)data1;
		const STNBLocaleKeyIdx* d2 = (const STNBLocaleKeyIdx*)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return NBString_strIsEqual(&d1->strs->str[d1->iStr], &d2->strs->str[d2->iStr]);
			case ENCompareMode_Lower:
				return NBString_strIsLower(&d1->strs->str[d1->iStr], &d2->strs->str[d2->iStr]);
			case ENCompareMode_LowerOrEqual:
				return NBString_strIsLowerOrEqual(&d1->strs->str[d1->iStr], &d2->strs->str[d2->iStr]);
			case ENCompareMode_Greater:
				return NBString_strIsGreater(&d1->strs->str[d1->iStr], &d2->strs->str[d2->iStr]);
			case ENCompareMode_GreaterOrEqual:
				return NBString_strIsGreaterOrEqual(&d1->strs->str[d1->iStr], &d2->strs->str[d2->iStr]);
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}

//Lang Idx

typedef struct STNBLocaleLangIdx_ {
	STNBArray		langs;				//STNBLocaleLang
	STNBArray		langsPrefOrd;		//SI32
	STNBArraySorted	keysOrdered;		//STNBLocaleKeyIdx
	UI32			bytesPerIdxBlock;	//4 bytes for per-value idx
	UI32*			idxsBlocks;			//one UI32-per-lang
	STNBString		strs;	//All strings
} STNBLocaleLangIdx;

void NBLocaleLangIdx_init(STNBLocaleLangIdx* obj){
	NBArray_init(&obj->langs, sizeof(STNBLocaleLang), NULL);
	NBArray_init(&obj->langsPrefOrd, sizeof(SI32), NULL);
	NBArraySorted_init(&obj->keysOrdered, sizeof(STNBLocaleKeyIdx), NBCompare_STNBLocaleKeyIdx); //STNBLocaleKeyIdx
	obj->bytesPerIdxBlock	= 0;	//4 bytes for per-value idx (langs.use values per block)
	obj->idxsBlocks			= NULL;	//one UI32-per-lang
	NBString_init(&obj->strs);
}

void NBLocaleLangIdx_empty(STNBLocaleLangIdx* obj){
	{
		SI32 i; for(i = 0; i < obj->langs.use; i++){
			STNBLocaleLang* lng = NBArray_itmPtrAtIndex(&obj->langs, STNBLocaleLang, i);
			if(lng->lang != NULL) NBMemory_free(lng->lang); lng->lang = NULL;
		}
		NBArray_empty(&obj->langs);
		NBArray_empty(&obj->langsPrefOrd);
	}
	NBArraySorted_empty(&obj->keysOrdered); //STNBLocaleKeyIdx
	obj->bytesPerIdxBlock	= 0;	//4 bytes for per-value idx (langs.use values per block)
	if(obj->idxsBlocks != NULL) NBMemory_free(obj->idxsBlocks); obj->idxsBlocks = NULL;
	NBString_empty(&obj->strs);
}

void NBLocaleLangIdx_release(STNBLocaleLangIdx* obj){
	NBLocaleLangIdx_empty(obj);
	NBArray_release(&obj->langs);
	NBArray_release(&obj->langsPrefOrd);
	NBArraySorted_release(&obj->keysOrdered); //STNBLocaleKeyIdx
	NBString_release(&obj->strs);
}

void NBLocaleLangIdx_rebuild(STNBLocaleLangIdx* obj, const STNBLocaleCfg* cfg){
	//Empty current
	NBLocaleLangIdx_empty(obj);
	//Rebuild new
	if(cfg->itmsSz > 0){
		//------
		//Analyze string lenghs and langs count
		//------
		UI32 strsLenWithZero = 0;
		{
			NBASSERT(obj->langs.use == 0)
			NBASSERT(obj->strs.length == 0)
			SI32 i; for(i = 0; i < cfg->itmsSz; i++){
				const STNBLocaleCfgItm* itm = &cfg->itms[i];
				//Add idx
				{
					strsLenWithZero += NBString_strLenBytes(itm->uid) + 1;
				}
				//See all languages
				{
					SI32 i; for(i = 0; i < itm->lclsSz; i++){
						const STNBLocaleCfgItmPair* pair = &itm->lcls[i];
						if(pair->lng != NULL && pair->lcl != NULL){
							if(pair->lng[0] != '\0' && pair->lcl[0] != '\0'){
								//Analyze if language is new
								{
									SI32 i; for(i = 0; i < obj->langs.use; i++){
										STNBLocaleLang* lng = NBArray_itmPtrAtIndex(&obj->langs, STNBLocaleLang, i);
										if(NBString_strIsEqual(pair->lng, lng->lang)){
											break;
										}
									}
									//Add new language
									if(i == obj->langs.use){
										STNBString langLow;
										NBString_init(&langLow);
										NBString_concatLower(&langLow, pair->lng);
										{
											STNBLocaleLang lng;
											NBMemory_setZeroSt(lng, STNBLocaleLang);
											NBString_strFreeAndNewBuffer(&lng.lang, langLow.str);
											NBArray_addValue(&obj->langsPrefOrd, obj->langs.use);
											NBArray_addValue(&obj->langs, lng);
											PRINTF_INFO("Locale, new lang found: '%s' (as '%s').\n", pair->lng, langLow.str);
										}
										NBString_release(&langLow);
									}
								}
								//Add string length
								strsLenWithZero += NBString_strLenBytes(pair->lcl) + 1;
							}
						}
					}
				}
			}
		}
		//
		//Build string
		//
		if(obj->langs.use > 0){
			//Reset string
			{
				//
				{
					NBArraySorted_release(&obj->keysOrdered);
					NBArraySorted_initWithSz(&obj->keysOrdered, sizeof(STNBLocaleKeyIdx), NBCompare_STNBLocaleKeyIdx, cfg->itmsSz, 4096, 0.10f);
				}
				//
				{
					//PRINTF_INFO("Locale, all strings expect-len: '%d'.\n", strsLenWithZero);
					NBString_release(&obj->strs);
					NBString_initWithSz(&obj->strs, strsLenWithZero + 1, 4096, 0.10f);
					NBString_concatByte(&obj->strs, '\0'); //Index zero is always empty string
				}
				//
				{
					NBASSERT(obj->bytesPerIdxBlock == 0)
					NBASSERT(obj->idxsBlocks == NULL)
					obj->bytesPerIdxBlock	= sizeof(UI32) * obj->langs.use;
					obj->idxsBlocks			= NBMemory_alloc(obj->bytesPerIdxBlock * cfg->itmsSz);
					NBMemory_set(obj->idxsBlocks, 0, obj->bytesPerIdxBlock * cfg->itmsSz);
				}
			}
			//Build string and index
			{
				NBASSERT(obj->langs.use > 0)
				NBASSERT(obj->strs.length == 1) //only zero-char
				SI32 i; for(i = 0; i < cfg->itmsSz; i++){
					const STNBLocaleCfgItm* itm = &cfg->itms[i];
					//Add key idx
					{
						STNBLocaleKeyIdx kIdx;
						kIdx.iStr	= obj->strs.length;
						kIdx.strs	= &obj->strs;
						kIdx.keyPos	= i;
						NBString_concat(&obj->strs, itm->uid);
						NBString_concatByte(&obj->strs, '\0');
						NBArraySorted_addValue(&obj->keysOrdered, kIdx);
					}
					//See all languages
					{
						UI32* block = &obj->idxsBlocks[i * obj->langs.use];
						SI32 i; for(i = 0; i < itm->lclsSz; i++){
							const STNBLocaleCfgItmPair* pair = &itm->lcls[i];
							if(pair->lng != NULL && pair->lcl != NULL){
								if(pair->lng[0] != '\0' && pair->lcl[0] != '\0'){
									const UI32 iStr = obj->strs.length;
									//Add string
									{
										NBString_concat(&obj->strs, pair->lcl);
										NBString_concatByte(&obj->strs, '\0');
									}
									//Add string idx at lang-order
									SI32 i; for(i = 0; i < obj->langs.use; i++){
										STNBLocaleLang* lng = NBArray_itmPtrAtIndex(&obj->langs, STNBLocaleLang, i);
										if(NBString_strIsEqual(pair->lng, lng->lang)){
											block[i] = iStr;
											break;
										}
									} NBASSERT(i < obj->langs.use)
								}
							}
						}
					}
				}
				PRINTF_INFO("Locale, all strings final-len: '%d'.\n", obj->strs.length);
			}
		}
	}
}

//Locale

typedef struct STNBLocaleOpq_ {
	STNBLocaleCfg		cfg;
	STNBLocaleLangIdx	idx;
	SI32				retainCount;
} STNBLocaleOpq;

void NBLocale_init(STNBLocale* obj){
	STNBLocaleOpq* opq = obj->opaque = NBMemory_allocType(STNBLocaleOpq);
	NBMemory_setZeroSt(*opq, STNBLocaleOpq);
	//
	NBLocaleLangIdx_init(&opq->idx);
	opq->retainCount = 1;
}

void NBLocale_retain(STNBLocale* obj){
	STNBLocaleOpq* opq = (STNBLocaleOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount++;
}

void NBLocale_release(STNBLocale* obj){
	STNBLocaleOpq* opq = (STNBLocaleOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount--;
	if(opq->retainCount == 0){
		NBLocaleLangIdx_release(&opq->idx);
		NBStruct_stRelease(NBLocaleCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
		//
		NBMemory_free(opq);
		obj->opaque = NULL;
	}
}

//

BOOL NBLocale_concatItmsByOwning(STNBLocaleOpq* opq, STNBLocaleCfg* cfg){
	BOOL r = FALSE;
	if(opq != NULL && cfg != NULL){
		r = TRUE;
		if(cfg->itms != NULL && cfg->itmsSz > 0){
			STNBArray arrN;
			NBArray_initWithSz(&arrN, sizeof(STNBLocaleCfgItm), NULL, (opq->cfg.itmsSz + cfg->itmsSz), 1024, 0.10f);
			//Add current
			if(opq->cfg.itms != NULL){
				if(opq->cfg.itmsSz > 0){
					NBArray_addItems(&arrN, opq->cfg.itms, sizeof(opq->cfg.itms[0]), opq->cfg.itmsSz);
				}
				NBMemory_free(opq->cfg.itms);
				opq->cfg.itms	= NULL;
				opq->cfg.itmsSz	= 0;
			}
			//Add new
			if(cfg->itms != NULL && cfg->itmsSz > 0){
				NBArray_addItems(&arrN, cfg->itms, sizeof(cfg->itms[0]), cfg->itmsSz);
				//Transfering ownership of data
				NBMemory_free(cfg->itms);
				cfg->itms	= NULL;
				cfg->itmsSz	= 0;
			}
			//Swap buffers
			{
				opq->cfg.itms	= NBArray_dataPtr(&arrN, STNBLocaleCfgItm);
				opq->cfg.itmsSz	= arrN.use;
				NBArray_resignToBuffer(&arrN);
			}
			NBArray_release(&arrN);
		}
	}
	return r;
}

BOOL NBLocale_loadFromBytes(STNBLocale* obj, const void* data, const UI32 dataSz){
	BOOL r = FALSE;
	STNBLocaleOpq* opq = (STNBLocaleOpq*)obj->opaque;
	if(opq != NULL){
		const STNBStructMap* stMap = NBLocaleCfg_getSharedStructMap();
		//Load
		STNBLocaleCfg cfg;
		NBMemory_setZeroSt(cfg, STNBLocaleCfg);
		if(NBStruct_stReadFromJsonStr((const char*)data, dataSz, stMap, &cfg, sizeof(cfg))){
			//Concat itms
			//const UI32 loadedSz = cfg.itmsSz;
			//PRINTF_INFO("NBLocale, itms before: %d.\n", opq->cfg.itmsSz);
			if(NBLocale_concatItmsByOwning(opq, &cfg)){
				//Rebuild-idx
				NBLocaleLangIdx_rebuild(&opq->idx, &opq->cfg);
				//PRINTF_INFO("NBLocale, itms after: %d (%d loaded); index-keys(%d)-langs(%d).\n", opq->cfg.itmsSz, loadedSz, opq->idx.keysOrdered.use, opq->idx.langs.use);
				r = TRUE;
			}
		}
		NBStruct_stRelease(stMap, &cfg, sizeof(cfg));
	}
	return r;
}

BOOL NBLocale_loadFromFile(STNBLocale* obj, STNBFileRef file){
	BOOL r = FALSE;
	STNBLocaleOpq* opq = (STNBLocaleOpq*)obj->opaque;
	if(opq != NULL){
		const STNBStructMap* stMap = NBLocaleCfg_getSharedStructMap();
		//Load
		STNBLocaleCfg cfg;
		NBMemory_setZeroSt(cfg, STNBLocaleCfg);
		if(NBStruct_stReadFromJsonFile(file, stMap, &cfg, sizeof(cfg))){
			//Concat itms
			//const UI32 loadedSz = cfg.itmsSz;
			//PRINTF_INFO("NBLocale, itms before: %d.\n", opq->cfg.itmsSz);
			if(NBLocale_concatItmsByOwning(opq, &cfg)){
				//Rebuild-idx
				NBLocaleLangIdx_rebuild(&opq->idx, &opq->cfg);
				//PRINTF_INFO("NBLocale, itms after: %d (%d loaded); index-keys(%d)-langs(%d).\n", opq->cfg.itmsSz, loadedSz, opq->idx.keysOrdered.use, opq->idx.langs.use);
				r = TRUE;
			}
		}
		NBStruct_stRelease(stMap, &cfg, sizeof(cfg));
	}
	return r;
}

BOOL NBLocale_loadFromFilePath(STNBLocale* obj, const char* filePath){
	BOOL r = FALSE;
	STNBLocaleOpq* opq = (STNBLocaleOpq*)obj->opaque;
	if(opq != NULL){
		//Load
		const STNBStructMap* stMap = NBLocaleCfg_getSharedStructMap();
		STNBLocaleCfg cfg;
		NBMemory_setZeroSt(cfg, STNBLocaleCfg);
		if(NBStruct_stReadFromJsonFilepath(filePath, stMap, &cfg, sizeof(cfg))){
			//Concat itms
			//const UI32 loadedSz = cfg.itmsSz;
			//PRINTF_INFO("NBLocale, itms before: %d.\n", opq->cfg.itmsSz);
			if(NBLocale_concatItmsByOwning(opq, &cfg)){
				//Rebuild-idx
				NBLocaleLangIdx_rebuild(&opq->idx, &opq->cfg);
				//PRINTF_INFO("NBLocale, itms after: %d (%d loaded); index-keys(%d)-langs(%d).\n", opq->cfg.itmsSz, loadedSz, opq->idx.keysOrdered.use, opq->idx.langs.use);
				r = TRUE;
			}
		}
		NBStruct_stRelease(stMap, &cfg, sizeof(cfg));
	}
	return r;
}

//

BOOL NBLocale_pushPreferedLang(STNBLocale* obj, const char* lang){
	BOOL r = FALSE;
	STNBLocaleOpq* opq = (STNBLocaleOpq*)obj->opaque;
	if(opq != NULL){
		STNBString langLow;
		NBString_init(&langLow);
		NBString_concatLower(&langLow, lang);
		//Search in order
		{
			NBASSERT(opq->idx.langsPrefOrd.use == opq->idx.langs.use)
			SI32 i; for(i = 0; i < opq->idx.langsPrefOrd.use; i++){
				const SI32 iLang = NBArray_itmValueAtIndex(&opq->idx.langsPrefOrd, SI32, i);
				STNBLocaleLang* lng = NBArray_itmPtrAtIndex(&opq->idx.langs, STNBLocaleLang, iLang);
				if(NBString_strIsEqual(lng->lang, langLow.str)){
					//Set and stop
					NBArray_removeItemAtIndex(&opq->idx.langsPrefOrd, i);
					NBArray_addItemsAtIndex(&opq->idx.langsPrefOrd, 0, &iLang, sizeof(iLang), 1);
					PRINTF_INFO("Pushing prefered language: '%s' as '%s' (extact-match).\n", lang, langLow.str);
					break;
				} else if(NBString_strStartsWith(lng->lang, langLow.str) || NBString_strStartsWith(langLow.str, lng->lang)){
					//Set but dont stop
					NBArray_removeItemAtIndex(&opq->idx.langsPrefOrd, i);
					NBArray_addItemsAtIndex(&opq->idx.langsPrefOrd, 0, &iLang, sizeof(iLang), 1);
					PRINTF_INFO("Pushing prefered language: '%s' as '%s' (is-like).\n", lang, langLow.str);
				}
			}
			if(i == opq->idx.langsPrefOrd.use){
				PRINTF_INFO("Exact prefered language not found: '%s' as '%s'.\n", lang, langLow.str);
			}
		}
		NBString_release(&langLow);
	}
	return r;
}

//

const char* NBLocale_getPreferedLangAtIdx(STNBLocale* obj, const UI32 idx, const char* lngDef){
	const char* r = lngDef;
	STNBLocaleOpq* opq = (STNBLocaleOpq*)obj->opaque;
	if(idx < opq->idx.langsPrefOrd.use){
		const SI32 iLang = NBArray_itmValueAtIndex(&opq->idx.langsPrefOrd, SI32, idx);
		const STNBLocaleLang* lngDef = NBArray_itmPtrAtIndex(&opq->idx.langs, STNBLocaleLang, iLang);
		r = lngDef->lang;
	}
	return r;
}

const char* NBLocale_getStr(STNBLocale* obj, const char* uid, const char* strDef){
	const char* r = strDef;
	STNBLocaleOpq* opq = (STNBLocaleOpq*)obj->opaque;
	if(opq != NULL){
		if(!NBString_strIsEmpty(uid)){
			//Ordered search
			{
				SI32 idxFnd = -1;
				{
					STNBString str;
					NBString_initWithStr(&str, uid);
					{
						STNBLocaleKeyIdx srch;
						srch.iStr	= 0;
						srch.strs	= &str;
						srch.keyPos	= 0;
						idxFnd = NBArraySorted_indexOf(&opq->idx.keysOrdered, &srch, sizeof(srch), NULL);
					}
					NBString_release(&str);
				}
				if(idxFnd == -1){
					//PRINTF_INFO("Locale not found for: '%s'.\n", uid);
				} else {
					//PRINTF_INFO("Locale at idx(%d) for '%s'.\n", idxFnd, uid);
					const STNBLocaleKeyIdx* keyIdx = NBArraySorted_itmPtrAtIndex(&opq->idx.keysOrdered, STNBLocaleKeyIdx, idxFnd);
					const UI32* keyBlock = &opq->idx.idxsBlocks[keyIdx->keyPos * opq->idx.langs.use];
					//Search first string (in preferered-lang order)
					{
						NBASSERT(opq->idx.langsPrefOrd.use == opq->idx.langs.use)
						SI32 i; for(i = 0; i < opq->idx.langsPrefOrd.use; i++){
							const SI32 iLang = NBArray_itmValueAtIndex(&opq->idx.langsPrefOrd, SI32, i);
							const UI32 iStr = keyBlock[iLang];
							if(iStr > 0){
								r = &opq->idx.strs.str[iStr];
								//PRINTF_INFO("Using lang('%s') '%s': '%s'.\n", NBArray_itmPtrAtIndex(&opq->idx.langs, STNBLocaleLang, iLang)->lang, uid, r);
								break;
							}
						}
					}
				}
			}
		}
	}
	return r;
}

const char* NBLocale_getStrLang(STNBLocale* obj, const char* lang, const char* uid, const char* strDef){
	const char* r = strDef;
	STNBLocaleOpq* opq = (STNBLocaleOpq*)obj->opaque;
	if(opq != NULL){
		if(!NBString_strIsEmpty(uid)){
			//Ordered search
			{
				SI32 idxFnd = -1;
				{
					STNBString str;
					NBString_initWithStr(&str, uid);
					{
						STNBLocaleKeyIdx srch;
						srch.iStr	= 0;
						srch.strs	= &str;
						srch.keyPos	= 0;
						idxFnd = NBArraySorted_indexOf(&opq->idx.keysOrdered, &srch, sizeof(srch), NULL);
					}
					NBString_release(&str);
				}
				if(idxFnd == -1){
					//PRINTF_INFO("Locale not found for: '%s'.\n", uid);
				} else {
					//PRINTF_INFO("Locale at idx(%d) for '%s'.\n", idxFnd, uid);
					const STNBLocaleKeyIdx* keyIdx = NBArraySorted_itmPtrAtIndex(&opq->idx.keysOrdered, STNBLocaleKeyIdx, idxFnd);
					const UI32* keyBlock = &opq->idx.idxsBlocks[keyIdx->keyPos * opq->idx.langs.use];
					//Search first string (in preferered-lang order)
					{
						NBASSERT(opq->idx.langsPrefOrd.use == opq->idx.langs.use)
						SI32 i; for(i = 0; i < opq->idx.langsPrefOrd.use; i++){
							const SI32 iLang = NBArray_itmValueAtIndex(&opq->idx.langsPrefOrd, SI32, i);
							const STNBLocaleLang* lngDef = NBArray_itmPtrAtIndex(&opq->idx.langs, STNBLocaleLang, iLang);
							if(NBString_strIsEqual(lang, lngDef->lang)){
								const UI32 iStr = keyBlock[iLang];
								if(iStr > 0){
									r = &opq->idx.strs.str[iStr];
									//PRINTF_INFO("Using lang('%s') '%s': '%s'.\n", NBArray_itmPtrAtIndex(&opq->idx.langs, STNBLocaleLang, iLang)->lang, uid, r);
									break;
								}
							}
						}
					}
				}
			}
		}
	}
	return r;
}

//Merge configs

void NBLocaleCfg_merge(STNBLocaleCfg* srcAndDst, const STNBLocaleCfg* other, const BOOL forceAllNotEmptyValues){
	if(other->itms != NULL && other->itmsSz > 0){
		STNBArraySorted arrItms, arrLngs;
		NBArraySorted_init(&arrItms, sizeof(STNBLocaleCfgItm), NBCompare_STNBLocaleCfgItm);
		NBArraySorted_init(&arrLngs, sizeof(STNBLocaleCfgItmPair), NBCompare_STNBLocaleCfgItmPair);
		//Add current
		if(srcAndDst->itms != NULL && srcAndDst->itmsSz > 0){
			NBArraySorted_addItems(&arrItms, srcAndDst->itms, sizeof(srcAndDst->itms[0]), srcAndDst->itmsSz);
		}
		//Add new
		{
			SI32 idsAdded = 0, lclsAdded = 0, lclsUpdated = 0;
			SI32 i; for(i = 0; i < other->itmsSz; i++){
				const STNBLocaleCfgItm* newItm = &other->itms[i];
				STNBLocaleCfgItm* curItm = NULL;
				//Search or create record
				{
					SI32 idx;
					STNBLocaleCfgItm srch;
					srch.uid = newItm->uid;
					idx = NBArraySorted_indexOf(&arrItms, &srch, sizeof(srch), NULL);
					if(idx >= 0){
						curItm = NBArraySorted_itmPtrAtIndex(&arrItms, STNBLocaleCfgItm, idx);
					} else {
						STNBLocaleCfgItm itm;
						NBMemory_setZeroSt(itm, STNBLocaleCfgItm);
						NBString_strFreeAndNewBuffer(&itm.uid, newItm->uid);
						curItm = (STNBLocaleCfgItm*)NBArraySorted_addValue(&arrItms, itm);
						idsAdded++;
					}
				} NBASSERT(curItm != NULL)
				//Sync langs
				NBASSERT(arrLngs.use == 0)
				if(newItm->lcls != NULL && newItm->lclsSz > 0){
					//Add current
					{
						NBArraySorted_addItems(&arrLngs, curItm->lcls, sizeof(curItm->lcls[0]), curItm->lclsSz);
					}
					//Search or create pair
					{
						SI32 i; for(i = 0; i < newItm->lclsSz; i++){
							const STNBLocaleCfgItmPair* newPair = &newItm->lcls[i];
							SI32 idx;
							STNBLocaleCfgItmPair srch;
							srch.lng = newPair->lng;
							idx = NBArraySorted_indexOf(&arrLngs, &srch, sizeof(srch), NULL);
							if(idx >= 0){
								//Update
								STNBLocaleCfgItmPair* curPair = NBArraySorted_itmPtrAtIndex(&arrLngs, STNBLocaleCfgItmPair, idx);
								if(!NBString_strIsEmpty(newPair->lcl)){
									if(NBString_strIsEmpty(curPair->lcl) || forceAllNotEmptyValues){
										NBString_strFreeAndNewBuffer(&curPair->lcl, newPair->lcl);
										lclsUpdated++;
									} else if(!NBString_strIsEqual(curPair->lcl, newPair->lcl)){
										PRINTF_WARNING("Two values for('%s'/'%s'):\n-->%s<--\nvs-->%s<--.\n", curItm->uid, curPair->lng, curPair->lcl, newPair->lcl);
									}
								}
							} else {
								//Add
								STNBLocaleCfgItmPair itm;
								NBMemory_setZeroSt(itm, STNBLocaleCfgItmPair);
								NBString_strFreeAndNewBuffer(&itm.lng, newPair->lng);
								NBString_strFreeAndNewBuffer(&itm.lcl, newPair->lcl);
								NBArraySorted_addValue(&arrLngs, itm);
								lclsAdded++;
							}
						}
					}
					//Swap buffers
					{
						if(curItm->lcls != NULL) NBMemory_free(curItm->lcls);
						curItm->lcls	= NBArraySorted_data(&arrLngs);
						curItm->lclsSz	= arrLngs.use;
						NBArraySorted_resignToBuffer(&arrLngs);
					}
				}
			}
			PRINTF_INFO("Merged: idsAdded(%d), lclsAdded(%d), lclsUpdated(%d)\n", idsAdded, lclsAdded, lclsUpdated);
		}
		//Swap buffers
		{
			if(srcAndDst->itms != NULL) NBMemory_free(srcAndDst->itms);
			srcAndDst->itms		= NBArraySorted_data(&arrItms);
			srcAndDst->itmsSz	= arrItms.use;
			NBArraySorted_resignToBuffer(&arrItms);
		}
		NBArraySorted_release(&arrLngs);
		NBArraySorted_release(&arrItms);
	}
}


void NBLocaleCfg_mergeUsingDuplicates(STNBLocaleCfg* srcAndDst, const char* langBase, const char* langToFill){
	if(srcAndDst->itms != NULL && srcAndDst->itmsSz > 0){
		SI32 lclsUpdated = 0;
		SI32 i; for(i = 0; i < srcAndDst->itmsSz; i++){
			STNBLocaleCfgItm* itm = &srcAndDst->itms[i];
			char** lclBase = NULL;
			char** lclToFill = NULL;
			{
				SI32 i; for(i = 0; i < itm->lclsSz; i++){
					STNBLocaleCfgItmPair* pair = &itm->lcls[i];
					if(NBString_strIsEqual(pair->lng, langToFill)){
						if(NBString_strIsEmpty(pair->lcl)){
							lclToFill = &pair->lcl;
						}
					} else if(NBString_strIsEqual(pair->lng, langBase)){
						if(!NBString_strIsEmpty(pair->lcl)){
							lclBase = &pair->lcl;
						}
					}
				}
			}
			if(lclBase != NULL && lclToFill != NULL){
				NBASSERT(!NBString_strIsEmpty(*lclBase))
				NBASSERT(NBString_strIsEmpty(*lclToFill))
				SI32 i; for(i = 0; i < srcAndDst->itmsSz; i++){
					STNBLocaleCfgItm* itm = &srcAndDst->itms[i];
					char** lclBase2 = NULL;
					char** lclToFill2 = NULL;
					{
						SI32 i; for(i = 0; i < itm->lclsSz; i++){
							STNBLocaleCfgItmPair* pair = &itm->lcls[i];
							if(NBString_strIsEqual(pair->lng, langToFill)){
								if(!NBString_strIsEmpty(pair->lcl)){
									lclToFill2 = &pair->lcl;
								}
							} else if(NBString_strIsEqual(pair->lng, langBase)){
								if(NBString_strIsEqual(pair->lcl, *lclBase)){
									lclBase2 = &pair->lcl;
								}
							}
						}
					}
					if(lclBase2 != NULL && lclToFill2 != NULL){
						NBASSERT(NBString_strIsEqual(*lclBase, *lclBase2))
						NBASSERT(NBString_strIsEmpty(*lclToFill))
						NBASSERT(!NBString_strIsEmpty(*lclToFill2))
						NBString_strFreeAndNewBuffer(lclToFill, *lclToFill2);
						lclsUpdated++;
						PRINTF_INFO("Merging: %s('%s') => %s('%s').\n", langBase, *lclBase2, langToFill, *lclToFill2);
						break;
					}
				}
			}
		}
		PRINTF_INFO("Merged: lclsUpdated(%d)\n", lclsUpdated);
	}
}

//Build json

void NBLocaleCfg_concatAsJson(const STNBLocaleCfg* src, STNBString* dst){
	NBLocaleCfg_concatAsJsonOnlyEmptyLang(src, NULL, dst);
}

void NBLocaleCfg_concatAsJsonOnlyEmptyLang(const STNBLocaleCfg* src, const char* emptyLang, STNBString* dst){
	NBString_concat(dst, "{\n");
	NBString_concat(dst, "\"itms\": [\n");
	if(src->itms != NULL && src->itmsSz > 0){
		SI32 added = 0;
		SI32 i; for(i = 0; i < src->itmsSz; i++){
			STNBLocaleCfgItm* itm = &src->itms[i];
			BOOL add = TRUE;
			//Determine if lang is not empty
			if(!NBString_strIsEmpty(emptyLang)){
				SI32 i; for(i = 0; i < itm->lclsSz; i++){
					STNBLocaleCfgItmPair* pair = &itm->lcls[i];
					if(NBString_strIsEqual(emptyLang, pair->lng)){
						if(!NBString_strIsEmpty(pair->lcl)){
							add = FALSE;
						}
					}
				}
			}
			//Add
			if(add){
				if(added != 0) NBString_concat(dst, ", ");
				NBString_concat(dst, "{\n");
				NBString_concat(dst, "	\"uid\": \"");
				NBString_concat(dst, itm->uid);
				NBString_concat(dst, "\"\n");
				if(!NBString_strIsEmpty(itm->hint)){
					NBString_concat(dst, ",	\"hint\": \"");
					NBString_concat(dst, itm->hint);
					NBString_concat(dst, "\"\n");
				}
				NBString_concat(dst, "	, \"lcls\": [\n");
				{
					SI32 added = 0;
					SI32 i; for(i = 0; i < itm->lclsSz; i++){
						STNBLocaleCfgItmPair* pair = &itm->lcls[i];
						NBString_concat(dst, (added == 0 ? "		" : ", "));
						NBString_concat(dst, "{\n");
						NBString_concat(dst, "			\"lng\": \"");
						NBString_concat(dst, pair->lng);
						NBString_concat(dst, "\"\n");
						/*if(pair->rev > 0){ //ToDo: enable the 'if'
							NBString_concat(dst, "			, \"rev\": \"");
							NBString_concatUI32(dst, pair->rev);
							NBString_concat(dst, "\"\n");
						}*/
						/*if(pair->revDate.year > 0 || pair->revDate.month > 0 || pair->revDate.day > 0){ //ToDo: enable the 'if'
							NBString_concat(dst, "			, \"revDate\": {\"year\": \"");
							NBString_concatUI32(dst, pair->revDate.year);
							NBString_concat(dst, "\", \"month\": ");
							NBString_concatUI32(dst, pair->revDate.month);
							NBString_concat(dst, "\", \"day\": ");
							NBString_concatUI32(dst, pair->revDate.day);
							NBString_concat(dst, "\"}\n");
						}*/
						if(pair->revTime > 0){
							NBString_concat(dst, "			, \"revTime\": \"");
							NBString_concatUI64(dst, pair->revTime);
							NBString_concat(dst, "\"\n");
						}
						if(!NBString_strIsEmpty(pair->revName)){ //ToDo: enable the 'if'
							NBString_concat(dst, "			, \"revName\": \"");
							NBJson_concatScaped(dst, pair->revName);
							NBString_concat(dst, "\"\n");
						}
						NBString_concat(dst, "			, \"lcl\": \"");
						if(!NBString_strIsEmpty(pair->lcl)){
							NBString_concat(dst, pair->lcl);
						}
						NBString_concat(dst, "\"\n");
						NBString_concat(dst, "		}");
						added++;
					}
				}
				NBString_concat(dst, "\n");
				NBString_concat(dst, "	]\n");
				NBString_concat(dst, "}");
				added++;
			}
		}
	}
	NBString_concat(dst, "\n");
	NBString_concat(dst, "]\n");
	NBString_concat(dst, "}\n");
}
