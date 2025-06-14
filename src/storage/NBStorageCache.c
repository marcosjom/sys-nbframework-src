//
//  NBStorageCache.c
//  NBFramework
//
//  Created by Marcos Ortega on 1/3/19.
//

#include "nb/NBFrameworkPch.h"
#include "nb/storage/NBStorageCache.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBThreadMutexRW.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"

typedef struct STNBStorageCacheItmLstnrOpq_ {
	BOOL	retained;
	void*	refObj;
	SI32	refObjCount;	//times the refObj has been added (willbe removed when zero)
	STNBStorageCacheItmLstnr methods;
} STNBStorageCacheItmLstnrOpq;

typedef struct STNBStorageCacheItm_ {
	STNBString				relPath;		//storage relative path (without root)
	STNBString				relBase;		//Folder part in at left of "relPart"
	struct {
		BOOL					triedLoad;	//Attempted to load its data?
		STNBStorageCacheItmData	value;		//private and public data
		STNBThreadMutexRW		mutex;		//access mutex (read and write)
	} dataa;
	struct {
		STNBArray				arr;			//STNBStorageCacheItmLstnrOpq
		STNBThreadMutex			mutex;
	} lstnrs;
} STNBStorageCacheItm;

typedef struct STNBStorageCacheOpq_ {
	STNBFilesystem*			fs;
	ENNBStorageNotifMode	notifMode;
	NBStorageCacheNotifyAddMethod notifMethod;
	void*					notifMethodParam;
	STNBString				rootBase;		//left part of the root that is part of the parent's path
	STNBString				rootPath;		//prefix to be added to paths
	STNBArray				itms;			//STNBStorageCacheItm*
	STNBStructMap*			itmsDataMap;	//Itm map, defined when first set (build)
	STNBStructMap*			itmsPubMap;		//Public data map (build)
	STNBStructMap*			itmsPrivMap;	//Private data map (build)
	const STNBStructMap*	itmsPubMapSt;	//Public data map (given, for validation)
	const STNBStructMap*	itmsPrivMapSt;	//Private data map (given, for validation)
	STNBThreadMutex			mutex;
	SI32					retainCount;
} STNBStorageCacheOpq;

void NBStorageCache_init(STNBStorageCache* obj){
	STNBStorageCacheOpq* opq = obj->opaque = NBMemory_allocType(STNBStorageCacheOpq);
	NBMemory_setZeroSt(*opq, STNBStorageCacheOpq);
	//
	opq->fs				= NULL;
	opq->notifMode		= ENNBStorageNotifMode_Inmediate;
	opq->notifMethod	= NULL;
	opq->notifMethodParam = NULL;
	NBString_init(&opq->rootBase);
	NBString_init(&opq->rootPath);
	NBArray_init(&opq->itms, sizeof(STNBStorageCacheItm*), NULL);
	opq->itmsDataMap	= NULL;	//Itm map, defined when first set
	opq->itmsPubMap		= NULL;	//Public data map
	opq->itmsPrivMap	= NULL;	//Private data map
	opq->itmsPubMapSt	= NULL;	//Public data map (given, for validation)
	opq->itmsPrivMapSt	= NULL;	//Private data map (given, for validation)
	NBThreadMutex_init(&opq->mutex);
	opq->retainCount	= 1;
}

void NBStorageCache_retain(STNBStorageCache* obj){
	STNBStorageCacheOpq* opq = (STNBStorageCacheOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	{
		NBASSERT(opq->retainCount > 0)
		opq->retainCount++;
	}
	NBThreadMutex_unlock(&opq->mutex);
}

void NBStorageCache_release(STNBStorageCache* obj){
	STNBStorageCacheOpq* opq = (STNBStorageCacheOpq*)obj->opaque;
	if(opq != NULL){
		NBThreadMutex_lock(&opq->mutex);
		NBASSERT(opq->retainCount > 0)
		opq->retainCount--;
		if(opq->retainCount > 0){
			NBThreadMutex_unlock(&opq->mutex);
		} else {
			//Release
			{
				SI32 i; for(i = 0; i < opq->itms.use; i++){
					STNBStorageCacheItm* itm = NBArray_itmValueAtIndex(&opq->itms, STNBStorageCacheItm*, i);
					{
						NBArray_empty(&itm->lstnrs.arr);
						NBArray_release(&itm->lstnrs.arr);
						NBThreadMutex_release(&itm->lstnrs.mutex);
						NBThreadMutexRW_release(&itm->dataa.mutex);
					}
					{
						NBString_release(&itm->relBase);
						NBString_release(&itm->relPath);
					}
					NBMemory_free(itm);
				}
				NBArray_empty(&opq->itms);
				NBArray_release(&opq->itms);
				//Release maps
				{
					if(opq->itmsDataMap != NULL){
						NBStructMap_release(opq->itmsDataMap);
						NBMemory_free(opq->itmsDataMap);
						opq->itmsDataMap = NULL;
					}
					if(opq->itmsPubMap != NULL){
						NBStructMap_release(opq->itmsPubMap);
						NBMemory_free(opq->itmsPubMap);
						opq->itmsPubMap = NULL;
					}
					if(opq->itmsPrivMap != NULL){
						NBStructMap_release(opq->itmsPrivMap);
						NBMemory_free(opq->itmsPrivMap);
						opq->itmsPrivMap = NULL;
					}
					opq->itmsPubMapSt	= NULL;	//Public data map
					opq->itmsPrivMapSt	= NULL;	//Private data map
				}
			}
			opq->notifMethod	= NULL;
			opq->notifMethodParam = NULL;
			NBASSERT(NBString_startsWith(&opq->rootPath, opq->rootBase.str))
			NBString_release(&opq->rootBase);
			NBString_release(&opq->rootPath);
			if(opq->fs != NULL){
				NBFilesystem_release(opq->fs);
				opq->fs = NULL;
			}
			NBThreadMutex_unlock(&opq->mutex);
			NBThreadMutex_release(&opq->mutex);
			//
			NBMemory_free(opq);
			obj->opaque = NULL;
		}
	}
}

//

BOOL NBStorageCache_setFilesystem(STNBStorageCache* obj, STNBFilesystem* fs){
	BOOL r = FALSE;
	STNBStorageCacheOpq* opq = (STNBStorageCacheOpq*)obj->opaque;
	if(opq != NULL){
		NBThreadMutex_lock(&opq->mutex);
		if(opq->itms.use > 0){
			PRINTF_ERROR("NBStorageCache, forbidden to set filesystem when not-empty.\n");
		} else {
			if(fs != NULL){
				NBFilesystem_retain(fs);
			}
			if(opq->fs != NULL){
				NBFilesystem_release(opq->fs);
			}
			opq->fs = fs;
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}

BOOL NBStorageCache_setNotifMode(STNBStorageCache* obj, const ENNBStorageNotifMode mode, NBStorageCacheNotifyAddMethod method, void* methodParam){
	BOOL r = FALSE;
	STNBStorageCacheOpq* opq = (STNBStorageCacheOpq*)obj->opaque;
	if(opq != NULL){
		NBThreadMutex_lock(&opq->mutex);
		if(opq->itms.use > 0){
			PRINTF_ERROR("NBStorageCache, forbidden to set filesystem when not-empty.\n");
		} else {
			opq->notifMode			= mode;
			opq->notifMethod		= method;
			opq->notifMethodParam	= methodParam;
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}

const char* NBStorageCache_getRoot(const STNBStorageCache* obj){
	STNBStorageCacheOpq* opq = (STNBStorageCacheOpq*)obj->opaque;
	return opq->rootPath.str;
}

BOOL NBStorageCache_setRoot(STNBStorageCache* obj, const char* parentRoot, const char* fullRoot){
	BOOL r = FALSE;
	STNBStorageCacheOpq* opq = (STNBStorageCacheOpq*)obj->opaque;
	if(opq != NULL){
		NBThreadMutex_lock(&opq->mutex);
		if(opq->itms.use > 0){
			PRINTF_ERROR("NBStorageCache, forbidden to set root when not-empty.\n");
		} else if(opq->fs == NULL){
			PRINTF_ERROR("NBStorageCache, forbidden to set when filesystem is NULL.\n");
		} else {
			NBString_set(&opq->rootBase, parentRoot);
			NBString_set(&opq->rootPath, fullRoot);
			NBASSERT(NBString_startsWith(&opq->rootPath, opq->rootBase.str))
			if(opq->rootPath.length > 0){
				if(opq->rootPath.str[opq->rootPath.length - 1] != '/'){
					NBString_concatByte(&opq->rootPath, '/');
				}
				if(opq->fs != NULL){
					if(!NBFilesystem_folderExistsAtRoot(opq->fs, ENNBFilesystemRoot_Lib, opq->rootPath.str)){
						NBFilesystem_createFolderPathAtRoot(opq->fs, ENNBFilesystemRoot_Lib, opq->rootPath.str);
					}
				}
			}
			r = TRUE;
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}

//

void NBStorageCache_createRecordsMap_(STNBStorageCacheOpq* opq, const STNBStructMap* pubMap, const STNBStructMap* privMap){
	NBASSERT(opq->itmsDataMap == NULL)
	if(opq->itmsDataMap == NULL){
		NBASSERT(opq->itmsPubMap == NULL)
		NBASSERT(opq->itmsPrivMap == NULL)
		NBASSERT(opq->itmsPrivMapSt == NULL)
		NBASSERT(opq->itmsPrivMapSt == NULL)
		//Build parts maps
		{
			STNBStorageCacheItmDataPart s;
			opq->itmsPubMap = NBMemory_allocType(STNBStructMap);
			NBStructMap_init(opq->itmsPubMap, sizeof(STNBStorageCacheItmDataPart));
			NBStructMap_addUIntM(opq->itmsPubMap, s, seq);
			NBStructMap_addUIntM(opq->itmsPubMap, s, modf);
			if(pubMap != NULL){
				//NBStructMap_addStructPtrM(opq->itmsPubMap, s, stData, pubMap);
				NBStructMap_addStructPtr(opq->itmsPubMap, "stData", &s, &s.stData, sizeof(s.stData), pubMap->stSize, pubMap);
			}
		}
		{
			STNBStorageCacheItmDataPart s;
			opq->itmsPrivMap = NBMemory_allocType(STNBStructMap);
			NBStructMap_init(opq->itmsPrivMap, sizeof(STNBStorageCacheItmDataPart));
			NBStructMap_addUIntM(opq->itmsPrivMap, s, seq);
			NBStructMap_addUIntM(opq->itmsPrivMap, s, modf);
			if(privMap != NULL){
				//NBStructMap_addStructPtrM(opq->itmsPrivMap, s, stData, privMap);
				NBStructMap_addStructPtr(opq->itmsPrivMap, "stData", &s, &s.stData, sizeof(s.stData), privMap->stSize, privMap);
			}
		}
		//Build itm map
		{
			STNBStorageCacheItmData s;
			opq->itmsDataMap = NBMemory_allocType(STNBStructMap);
			NBStructMap_init(opq->itmsDataMap, sizeof(STNBStorageCacheItmData));
			NBStructMap_addUIntM(opq->itmsDataMap, s, seq);
			NBStructMap_addUIntM(opq->itmsDataMap, s, made);
			NBStructMap_addUIntM(opq->itmsDataMap, s, modf);
			if(opq->itmsPubMap != NULL){
				NBStructMap_addStructM(opq->itmsDataMap, s, pub, opq->itmsPubMap);
			}
			if(opq->itmsPrivMap != NULL){
				NBStructMap_addStructM(opq->itmsDataMap, s, priv, opq->itmsPrivMap);
			}
			opq->itmsPubMapSt	= pubMap;
			opq->itmsPrivMapSt	= privMap;
		}
	}
	NBASSERT(opq->itmsDataMap != NULL)
}

void NBStorageCache_loadRecordData_(STNBStorageCacheOpq* opq, STNBStorageCacheItm* itm, STNBFileRef optOpenedFile){
	NBASSERT(!itm->dataa.triedLoad)
	if(NBFile_isSet(optOpenedFile)){
		//Load from already opened file
		NBFile_lock(optOpenedFile);
		if(!NBStruct_stReadFromJsonFile(optOpenedFile, opq->itmsDataMap, &itm->dataa.value, sizeof(itm->dataa.value))){
			if(NBFile_curPos(optOpenedFile) != 0){
				PRINTF_ERROR("NBStorageCache, could not load record from non-empty-file (optOpenedFile).\n");
#				ifdef NB_CONFIG_INCLUDE_ASSERTS
				{
					char buff[4096];
					STNBString str;
					NBString_init(&str);
					NBFile_seek(optOpenedFile, 0, ENNBFileRelative_Start);
					while(TRUE){
						const SI32 rr = NBFile_read(optOpenedFile, buff, sizeof(buff));
						if(rr <= 0) break;
						NBString_concatBytes(&str, buff, rr);
					}
					PRINTF_INFO("NBStorageCache, file full-content:\nstart-->%s<--end.\n", str.str);
					NBString_release(&str);
				}
#				endif
			}
		} else {
			//PRINTF_INFO("NBStorageCache, record-data from file (optOpenedFile).\n");
		}
		NBFile_unlock(optOpenedFile);
	} else {
		//Load from new file
		{
			STNBString path;
			NBString_init(&path);
			NBString_concatBytes(&path, opq->rootPath.str, opq->rootPath.length);
			NBString_concatBytes(&path, itm->relPath.str, itm->relPath.length);
			{
				STNBFileRef f = NBFile_alloc(NULL);
				if(NBFilesystem_openAtRoot(opq->fs, ENNBFilesystemRoot_Lib, path.str, ENNBFileMode_Read, f)){
					NBFile_lock(f);
					if(!NBStruct_stReadFromJsonFile(f, opq->itmsDataMap, &itm->dataa.value, sizeof(itm->dataa.value))){
						if(NBFile_curPos(f) != 0){
							PRINTF_ERROR("NBStorageCache, could not load record from non-empty-file: '%s'.\n", path.str);
#							ifdef NB_CONFIG_INCLUDE_ASSERTS
							{
								char buff[4096];
								STNBString str;
								NBString_init(&str);
								NBFile_seek(f, 0, ENNBFileRelative_Start);
								while(TRUE){
									const SI32 rr = NBFile_read(f, buff, sizeof(buff));
									if(rr <= 0) break;
									NBString_concatBytes(&str, buff, rr);
								}
								PRINTF_INFO("NBStorageCache, file full-content:\nstart-->%s<--end.\n", str.str);
								NBString_release(&str);
							}
#							endif
						}
					} else {
						//PRINTF_INFO("NBStorageCache, record-data from file '%s'.\n", path.str);
					}
					NBFile_unlock(f);
					NBFile_close(f);
				}
				NBFile_release(&f);
			}
			NBString_release(&path);
		}
	}
	//Create new data if necesary
	{
		BOOL created = FALSE;
		NBASSERT(opq->itmsPubMap != NULL)
		NBASSERT(opq->itmsPrivMap != NULL)
		if(itm->dataa.value.pub.stData == NULL && opq->itmsPubMapSt != NULL){
			itm->dataa.value.pub.stData = NBMemory_alloc(opq->itmsPubMapSt->stSize);
			NBMemory_set(itm->dataa.value.pub.stData, 0, opq->itmsPubMapSt->stSize);
			created = TRUE;
		}
		if(itm->dataa.value.priv.stData == NULL && opq->itmsPrivMapSt != NULL){
			itm->dataa.value.priv.stData = NBMemory_alloc(opq->itmsPrivMapSt->stSize);
			NBMemory_set(itm->dataa.value.priv.stData, 0, opq->itmsPrivMapSt->stSize);
			created = TRUE;
		}
		//if(created){
		//	PRINTF_INFO("NBStorageCache, record-data from empty '%s%s'.\n", opq->rootPath.str, itm->relPath.str);
		//}
	}
	//
	itm->dataa.triedLoad = TRUE;
}

typedef enum ENNBStorageCacheRecordLockMode_ {
	ENNBStorageCacheRecordLockMode_Read = 0,
	ENNBStorageCacheRecordLockMode_Write,
	ENNBStorageCacheRecordLockMode_Lstnrs,
	ENNBStorageCacheRecordLockMode_Count
} ENNBStorageCacheRecordLockMode;

STNBStorageCacheItm* NBStorageCache_getRecordLocked_(STNBStorageCacheOpq* opq, const char* relPath, const STNBStructMap* pubMap, const STNBStructMap* privMap,const BOOL createIfFileExists, const BOOL createIfNew, const BOOL dataRequired, const ENNBStorageCacheRecordLockMode lockMode){
	STNBStorageCacheItm* r = NULL;
	//Search record
	{
		NBThreadMutex_lock(&opq->mutex);
		if(opq->fs != NULL){
			BOOL error = FALSE;
			//Create map (if this is the first record that required data)
			if(dataRequired && opq->itmsDataMap == NULL){
				if(opq->itmsDataMap == NULL){ //Verify again inside the lock
					NBStorageCache_createRecordsMap_(opq, pubMap, privMap);
					NBASSERT(opq->itmsDataMap != NULL)
				}
			}
			//Search
			{
				SI32 i; for(i = 0; i < opq->itms.use; i++){
					STNBStorageCacheItm* itm = NBArray_itmValueAtIndex(&opq->itms, STNBStorageCacheItm*, i);
					if(NBString_isLike(&itm->relPath, relPath)){
						NBASSERT(!dataRequired || NBStruct_stIsEqualByCrc(NBStructMap_getSharedStructMap(), opq->itmsPubMapSt, sizeof(*opq->itmsPubMapSt), pubMap, sizeof(*pubMap)))
						NBASSERT(!dataRequired || NBStruct_stIsEqualByCrc(NBStructMap_getSharedStructMap(), opq->itmsPrivMapSt, sizeof(*opq->itmsPrivMapSt), privMap, sizeof(*privMap)))
						if(dataRequired && (!NBStruct_stIsEqualByCrc(NBStructMap_getSharedStructMap(), opq->itmsPubMapSt, sizeof(*opq->itmsPubMapSt), pubMap, sizeof(*pubMap)) || !NBStruct_stIsEqualByCrc(NBStructMap_getSharedStructMap(), opq->itmsPrivMapSt, sizeof(*opq->itmsPrivMapSt), privMap, sizeof(*privMap)))){
							error = TRUE; //must use the same structMaps
						} else {
							//ToDo: retainTheRecord
							//Return
							r = itm;
						}
						break;
					}
				}
			}
			//Create new storageCache
			if(r == NULL && (createIfFileExists || createIfNew) && !error){
				STNBFileRef f = NBFile_alloc(NULL);
				{
					//Determine if file exists
					BOOL fileOpened = FALSE;
					if(createIfFileExists && !createIfNew){
						STNBString path;
						NBString_init(&path);
						NBString_concatBytes(&path, opq->rootPath.str, opq->rootPath.length);
						NBString_concat(&path, relPath);
						if(NBFilesystem_openAtRoot(opq->fs, ENNBFilesystemRoot_Lib, path.str, ENNBFileMode_Read, f)){
							fileOpened = TRUE;
						}
						NBString_release(&path);
					}
					//Create record
					if(fileOpened || createIfNew){
						STNBStorageCacheItm* itm = NBMemory_allocType(STNBStorageCacheItm);
						NBMemory_setZeroSt(*itm, STNBStorageCacheItm);
						//PRINTF_INFO("NBStorageCache, created new record for: '%s%s'.\n", opq->rootPath.str, relPath);
						itm->dataa.triedLoad	= FALSE;
						itm->dataa.value.made	= NBDatetime_getCurUTCTimestamp();
						NBString_initWithStr(&itm->relPath, relPath);
						NBString_init(&itm->relBase);
						if(itm->relPath.length > 0){
							//Determine folder part
							const SI32 lastSlash = NBString_lastIndexOf(&itm->relPath, "/", itm->relPath.length - 1);
							if(lastSlash > 0){
								NBASSERT(lastSlash < (itm->relPath.length - 1)) //Empty filename?
								NBString_concatBytes(&itm->relBase, itm->relPath.str, lastSlash);
							}
						}
						NBThreadMutexRW_init(&itm->dataa.mutex);
						NBArray_init(&itm->lstnrs.arr, sizeof(STNBStorageCacheItmLstnrOpq), NULL);
						NBThreadMutex_init(&itm->lstnrs.mutex);
						NBArray_addValue(&opq->itms, itm);
						//Return
						r = itm;
					}
				}
				NBFile_release(&f);
			}
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	//LockRecord (while storage unlocked)
	if(r != NULL){
		//Try to load data from file
		if(dataRequired && !r->dataa.triedLoad){
			NBThreadMutexRW_lockForWrite(&r->dataa.mutex);
			if(!r->dataa.triedLoad){ //Verify again inside the lock
				NBStorageCache_loadRecordData_(opq, r, NB_OBJREF_NULL);
				NBASSERT(r->dataa.triedLoad)
			}
			NBThreadMutexRW_unlockFromWrite(&r->dataa.mutex);
		}
		//Lock for requester
		switch(lockMode) {
			case ENNBStorageCacheRecordLockMode_Read:
				//PRINTF_INFO("Record '%s%s' lock for READ.\n", opq->rootPath.str, relPath);
				NBThreadMutexRW_lockForRead(&r->dataa.mutex);
				break;
			case ENNBStorageCacheRecordLockMode_Write:
				//PRINTF_INFO("Record '%s%s' lock for WRITE.\n", opq->rootPath.str, relPath);
				NBThreadMutexRW_lockForWrite(&r->dataa.mutex);
				break;
			case ENNBStorageCacheRecordLockMode_Lstnrs:
				//PRINTF_INFO("Record '%s%s' lock for LSTNRS.\n", opq->rootPath.str, relPath);
				NBThreadMutex_lock(&r->lstnrs.mutex);
				break;
			default:
				NBASSERT(FALSE)
				break;
		}
		//ToDo: release record
	}
	return r;
}

//

//Listeners

void NBStorageCache_addListener(STNBStorageCache* obj, const char* relPath, void* refObj, const STNBStorageCacheItmLstnr* methods){
	if(relPath != NULL && refObj != NULL && methods != NULL){
		NBASSERT(methods->retain != NULL)
		NBASSERT(methods->release != NULL)
		NBASSERT(methods->recordChanged != NULL)
		STNBStorageCacheOpq* opq = (STNBStorageCacheOpq*)obj->opaque;
		BOOL lstnrRetained = FALSE, lstnrWasNew = FALSE;
		//Retain listener (unlocked)
		if(opq->notifMode == ENNBStorageNotifMode_Inmediate && methods != NULL){
			if(methods->retain != NULL && methods->release != NULL){
				(*methods->retain)(refObj);
				lstnrRetained = TRUE;
			}
		}
		//Add record (locked)
		{
			STNBStorageCacheItm* itm = NBStorageCache_getRecordLocked_(opq, relPath, NULL, NULL, FALSE /*createIfFileExists*/, TRUE /*create*/, FALSE /*dataNeeded*/, ENNBStorageCacheRecordLockMode_Lstnrs); NBASSERT(itm != NULL)
			//Update or add listener
			{
				SI32 i; for(i = 0; i < itm->lstnrs.arr.use; i++){
					STNBStorageCacheItmLstnrOpq* lstnr = NBArray_itmPtrAtIndex(&itm->lstnrs.arr, STNBStorageCacheItmLstnrOpq, i);
					if(lstnr->refObj == refObj){
						NBASSERT(lstnr->refObjCount > 0)
						lstnr->refObjCount++;
						//PRINTF_INFO("NBStorageCache, record listener ref-incremented-to(%d) for (%llu) '%s' (%d lstnrs total).\n", lstnr->refObjCount, (UI64)refObj, relPath, itm->lstnrs.arr.use);
						NBASSERT(NBString_strIsEqualBytes((const char*)methods, sizeof(*methods), (const char*)&lstnr->methods, sizeof(lstnr->methods)))
						break;
					}
				}
				//Add new
				if(i == itm->lstnrs.arr.use){
					STNBStorageCacheItmLstnrOpq lstnr;
					NBMemory_setZeroSt(lstnr, STNBStorageCacheItmLstnrOpq);
					lstnr.retained		= lstnrRetained;
					lstnr.refObj		= refObj;
					lstnr.refObjCount	= 1;
					lstnr.methods		= *methods;
					NBArray_addValue(&itm->lstnrs.arr, lstnr);
					//PRINTF_INFO("NBStorageCache, record listener added for (%llu) '%s' (%d lstnrs total).\n", (UI64)refObj, relPath, itm->lstnrs.arr.use);
					lstnrWasNew			= TRUE;
				}
			}
			//PRINTF_INFO("Existing record '%s%s' unlock for LSTNRS.\n", opq->rootPath.str, relPath);
			NBThreadMutex_unlock(&itm->lstnrs.mutex);
		}
		//Release listener (unlocked)
		if(lstnrRetained && !lstnrWasNew){
			NBASSERT(methods->retain != NULL && methods->release != NULL)
			if(methods->retain != NULL && methods->release != NULL){
				(*methods->release)(refObj);
				lstnrRetained = FALSE;
			}
		}
	}
}

void NBStorageCache_removeListener(STNBStorageCache* obj, const char* relPath, void* refObj){
	if(relPath != NULL && refObj != NULL){
		STNBStorageCacheOpq* opq = (STNBStorageCacheOpq*)obj->opaque;
		STNBStorageCacheItmLstnrOpq* lstnrToRelease = NULL;
		//Update or remove listener
		{
			STNBStorageCacheItm* itm = NBStorageCache_getRecordLocked_(opq, relPath, NULL, NULL, FALSE /*createIfFileExists*/, TRUE /*create*/, FALSE /*dataNeeded*/, ENNBStorageCacheRecordLockMode_Lstnrs); NBASSERT(itm != NULL)
			BOOL fnd = FALSE;
			SI32 i; for(i = 0; i < itm->lstnrs.arr.use; i++){
				STNBStorageCacheItmLstnrOpq* lstnr = NBArray_itmPtrAtIndex(&itm->lstnrs.arr, STNBStorageCacheItmLstnrOpq, i);
				//PRINTF_INFO("NBStorageCache, comparing #%d / %d, %llu vs %llu.\n", (i + 1), itm->lstnrs.arr.use, (UI64)lstnr->refObj, (UI64)refObj);
				if(lstnr->refObj == refObj){
					NBASSERT(lstnr->refObjCount > 0)
					lstnr->refObjCount--;
					if(lstnr->refObjCount == 0){
						if(lstnr->retained){
							lstnrToRelease	= NBMemory_allocType(STNBStorageCacheItmLstnrOpq);
							*lstnrToRelease	= *lstnr;
						}
						NBArray_removeItemAtIndex(&itm->lstnrs.arr, i);
						//PRINTF_INFO("NBStorageCache, record listener removed for (%llu) '%s'.\n", (UI64)refObj, relPath);
					} else {
						//PRINTF_INFO("NBStorageCache, record listener ref-reduced-to(%d) for (%llu) '%s'.\n", lstnr->refObjCount, (UI64)refObj, relPath);
					}
					fnd = TRUE;
					break;
				}
			}
			//if(!fnd){
				//PRINTF_ERROR("NBStorageCache, record listener not found in %d records for (%llu) '%s'.\n", itm->lstnrs.arr.use, (UI64)refObj, relPath);
			//}
			NBASSERT(fnd) //Must be found
			NBThreadMutex_unlock(&itm->lstnrs.mutex);
		}
		//PRINTF_INFO("Existing record '%s%s' unlock for LSTNRS.\n", opq->rootPath.str, relPath);
		//Release object (unlocked)
		if(lstnrToRelease != NULL){
			NBASSERT(lstnrToRelease->retained)
			NBASSERT(lstnrToRelease->methods.release != NULL)
			if(lstnrToRelease->retained && lstnrToRelease->methods.release != NULL){
				(*lstnrToRelease->methods.release)(lstnrToRelease->refObj);
			}
			NBMemory_free(lstnrToRelease);
			lstnrToRelease = NULL;
		}
	}
}

//

const STNBStorageCacheItmData* NBStorageCache_getRecordLockedForRead(STNBStorageCache* obj, const char* relPath, const STNBStructMap* pubMap, const STNBStructMap* privMap, const BOOL createIfFileExists, const BOOL createIfNew){
	STNBStorageCacheItmData* r = NULL;
	STNBStorageCacheOpq* opq = (STNBStorageCacheOpq*)obj->opaque;
	STNBStorageCacheItm* itm = NBStorageCache_getRecordLocked_(opq, relPath, pubMap, privMap, createIfFileExists, createIfNew, TRUE /*dataRequired*/, ENNBStorageCacheRecordLockMode_Read);
	if(itm != NULL){
		r = &itm->dataa.value;
	}
	return r;
}

STNBStorageCacheItmData* NBStorageCache_getRecordLockedForWrite(STNBStorageCache* obj, const char* relPath, const STNBStructMap* pubMap, const STNBStructMap* privMap, const BOOL createIfFileExists, const BOOL createIfNew){
	STNBStorageCacheItmData* r = NULL;
	STNBStorageCacheOpq* opq = (STNBStorageCacheOpq*)obj->opaque;
	STNBStorageCacheItm* itm = NBStorageCache_getRecordLocked_(opq, relPath, pubMap, privMap, createIfFileExists, createIfNew, TRUE /*dataRequired*/, ENNBStorageCacheRecordLockMode_Write);
	if(itm != NULL){
		r = &itm->dataa.value;
	}
	return r;
}

//

void NBStorageCache_returnRecordLockedFromRead(STNBStorageCache* obj, const STNBStorageCacheItmData* data){
	STNBStorageCacheOpq* opq = (STNBStorageCacheOpq*)obj->opaque;
	STNBStorageCacheItm* itmFnd = NULL;
	//Search
	{
		NBThreadMutex_lock(&opq->mutex);
		//Search
		{
			SI32 i; for(i = 0; i < opq->itms.use; i++){
				STNBStorageCacheItm* itm = NBArray_itmValueAtIndex(&opq->itms, STNBStorageCacheItm*, i);
				if(&itm->dataa.value == data){
					//PRINTF_INFO("Record '%s%s' unlock for READ.\n", opq->rootPath.str, itm->relPath.str);
					itmFnd = itm;
					//ToDo: retain
					break;
				}
			} NBASSERT(i < opq->itms.use) //Must be found (if not, parent code has error)
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	//Unlock record (storage unlocked)
	if(itmFnd != NULL){
		NBThreadMutexRW_unlockFromRead(&itmFnd->dataa.mutex);
		//ToDo: release itm
	}
}

void NBStorageCache_returnRecordLockedFromWrite_(STNBStorageCache* obj, STNBStorageCacheItmData* data, const BOOL pubModified, const BOOL privModified, const BOOL forceNotify){
	STNBStorageCacheOpq* opq = (STNBStorageCacheOpq*)obj->opaque;
	//To notify (after unlocking)
	STNBStorageCacheItm* itmFnd = NULL;
	//Search (storage locked)
	{
		NBThreadMutex_lock(&opq->mutex);
		//Search
		{
			SI32 i; for(i = 0; i < opq->itms.use; i++){
				STNBStorageCacheItm* itm = NBArray_itmValueAtIndex(&opq->itms, STNBStorageCacheItm*, i);
				if(&itm->dataa.value == data){
					itmFnd = itm;
					//ToDo: retain itm
					break;
				}
			} NBASSERT(i < opq->itms.use) //Must be found (if not, parent code has error)
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	//Unlock record (storage unlocked)
	if(itmFnd != NULL){
		STNBStorageCacheItm* itm = itmFnd;
		//Save data to file
		if(pubModified || privModified){
			STNBString path;
			NBString_init(&path);
			{
				NBASSERT(NBString_startsWith(&itm->relPath, itm->relBase.str));
				NBString_concat(&path, opq->rootPath.str);
				NBString_concat(&path, itm->relBase.str);
				//Create folder
				if(!NBFilesystem_folderExistsAtRoot(opq->fs, ENNBFilesystemRoot_Lib, opq->rootPath.str)){
					NBFilesystem_createFolderPathAtRoot(opq->fs, ENNBFilesystemRoot_Lib, opq->rootPath.str);
				}
				//Add remaining
				NBString_concat(&path, &itm->relPath.str[itm->relBase.length]);
			}
			//Update record
			{
				UI64 time = NBDatetime_getCurUTCTimestamp();
				if(pubModified){
					data->modf = data->pub.modf = time;
					data->pub.seq++;
				}
				if(privModified){
					data->modf = data->priv.modf = time;
					data->priv.seq++;
				}
				data->seq++;
			}
			{
				STNBFileRef f = NBFile_alloc(NULL);
				if(!NBFilesystem_openAtRoot(opq->fs, ENNBFilesystemRoot_Lib, path.str, ENNBFileMode_Write, f)){
					NBASSERT(FALSE)
				} else {
					NBFile_lock(f);
					if(!NBStruct_stWriteToJsonFile(f, opq->itmsDataMap, &itm->dataa.value, sizeof(itm->dataa.value))){
						PRINTF_ERROR("NBStorageCache, could not write record to file.\n");
						NBASSERT(FALSE)
					} else {
						//PRINTF_INFO("NBStorageCache, record written to file.\n");
					}
					NBFile_unlock(f);
					NBFile_close(f);
				}
				NBFile_release(&f);
			}
			NBString_release(&path);
		}
		//Unlock record
		{
			//PRINTF_INFO("Record '%s%s' unlock for WRITE.\n", opq->rootPath.str, itm->relPath.str);
			NBThreadMutexRW_unlockFromWrite(&itm->dataa.mutex);
		}
		//Notify
		if(pubModified || privModified || forceNotify){
			STNBStorageCacheItmLstnrOpq* lstnrs = NULL;
			SI32 lstnrsSz	= 0;
			char* rootPath	= NULL;
			char* relPath	= NULL;
			//Build notification list (locked)
			{
				NBThreadMutex_lock(&itm->lstnrs.mutex);
				//PRINTF_INFO("Record '%s%s' lock for LSTNRS.\n", opq->rootPath.str, itm->relPath.str);
				if(itm->lstnrs.arr.use > 0){
					if(opq->notifMode == ENNBStorageNotifMode_Inmediate){
						//Create copy of lstnrs to notify after unlocking the record
						lstnrsSz	= itm->lstnrs.arr.use;
						lstnrs		= NBMemory_allocTypes(STNBStorageCacheItmLstnrOpq, itm->lstnrs.arr.use);
						NBMemory_copy(lstnrs, NBArray_data(&itm->lstnrs.arr), sizeof(STNBStorageCacheItmLstnrOpq) * itm->lstnrs.arr.use);
						NBASSERT(opq->rootBase.length <= opq->rootPath.length);
						rootPath	= NBString_strNewBufferBytes(&opq->rootPath.str[opq->rootBase.length], (opq->rootPath.length - opq->rootBase.length));
						relPath		= NBString_strNewBufferBytes(itm->relPath.str, itm->relPath.length);
						//Retain all
						{
							SI32 i; for(i = 0 ; i < lstnrsSz; i++){
								STNBStorageCacheItmLstnrOpq* lstnr = &lstnrs[i];
								if(lstnr->methods.retain != NULL && lstnr->methods.release != NULL){
									(*lstnr->methods.retain)(lstnr->refObj);
								}
							}
						}
					} else {
						NBASSERT(opq->notifMethod != NULL)
						if(opq->notifMethod != NULL){
							const char* rootPath = &opq->rootPath.str[opq->rootBase.length];
							const char* relPath = itm->relPath.str;
							SI32 i; for(i = 0 ; i < itm->lstnrs.arr.use; i++){
								STNBStorageCacheItmLstnrOpq* itm2 = NBArray_itmPtrAtIndex(&itm->lstnrs.arr, STNBStorageCacheItmLstnrOpq, i);
								if(itm2->methods.recordChanged != NULL){
									(*opq->notifMethod)(opq->notifMethodParam, rootPath, relPath, itm2->refObj, &itm2->methods);
								}
							}
						}
					}
				}
				//PRINTF_INFO("Record '%s%s' unlock for LSTNRS.\n", opq->rootPath.str, itm->relPath.str);
				NBThreadMutex_unlock(&itm->lstnrs.mutex);
			}
			//Notify (storage, record and listenerList unlocked)
			if(lstnrs != NULL){
				NBASSERT(lstnrsSz > 0)
				NBASSERT(rootPath != NULL)
				NBASSERT(relPath != NULL)
				SI32 i; for(i = 0 ; i < lstnrsSz; i++){
					STNBStorageCacheItmLstnrOpq* lstnr = &lstnrs[i];
					//Call
					if(lstnr->methods.recordChanged != NULL){
						//PRINTF_INFO("NBStorageCache_returnRecordLockedFromWrite, notifying (real time): '%s' '%s'.\n", rootPath, relPath);
						(*lstnr->methods.recordChanged)(lstnr->refObj, rootPath, relPath);
					}
					//Release
					if(lstnr->methods.retain && lstnr->methods.release != NULL){
						(*lstnr->methods.release)(lstnr->refObj);
					}
				}
				NBMemory_free(rootPath); rootPath = NULL;
				NBMemory_free(relPath); relPath = NULL;
				NBMemory_free(lstnrs); lstnrs = NULL;
				lstnrsSz = 0;
			}
		}
		//ToDo: release record
	}
}

void NBStorageCache_returnRecordLockedFromWrite(STNBStorageCache* obj, STNBStorageCacheItmData* data, const BOOL pubModified, const BOOL privModified){
	NBStorageCache_returnRecordLockedFromWrite_(obj, data, pubModified, privModified, FALSE /*forceNotify*/);
}

//Touch record (trigger a notification to listeners without saving to file)

void NBStoragesCache_touchRecord(STNBStorageCache* obj, const char* relPath){
	STNBStorageCacheOpq* opq = (STNBStorageCacheOpq*)obj->opaque;
	STNBStorageCacheItm* itm = NBStorageCache_getRecordLocked_(opq, relPath, NULL, NULL, FALSE, FALSE, FALSE /*dataRequired*/, ENNBStorageCacheRecordLockMode_Write);
	if(itm != NULL){
		STNBStorageCacheItmData* data = &itm->dataa.value;
		NBStorageCache_returnRecordLockedFromWrite_(obj, data, FALSE, FALSE, TRUE /*forceNotify*/);
	}
}

void NBStoragesCache_resetAllRecords(STNBStorageCache* obj, const BOOL deleteFiles){
	STNBStorageCacheOpq* opq = (STNBStorageCacheOpq*)obj->opaque;
    STNBArray itms;
    NBArray_init(&itms, sizeof(STNBStorageCacheItm*), NULL);
    //get all records
    {
        NBThreadMutex_lock(&opq->mutex);
        SI32 i; for(i = 0; i < opq->itms.use; i++){
            STNBStorageCacheItm* itm = NBArray_itmValueAtIndex(&opq->itms, STNBStorageCacheItm*, i);
            NBArray_addValue(&itms, itm);
        }
        NBThreadMutex_unlock(&opq->mutex);
    }
    //process all records
    {
        SI32 i; for(i = 0; i < itms.use; i++){
            STNBStorageCacheItm* itm = NBArray_itmValueAtIndex(&itms, STNBStorageCacheItm*, i);
            //Reset data
            {
                NBThreadMutexRW_lockForWrite(&itm->dataa.mutex);
                {
                    //Release data
                    {
                        NBStruct_stRelease(opq->itmsDataMap, &itm->dataa.value, sizeof(itm->dataa.value));
                        itm->dataa.triedLoad = FALSE;
                    }
                    //Delete file
                    if(deleteFiles && opq->fs != NULL){
                        STNBString path;
                        NBString_init(&path);
                        NBString_concatBytes(&path, opq->rootPath.str, opq->rootPath.length);
                        NBString_concatBytes(&path, itm->relPath.str, itm->relPath.length);
                        NBFilesystem_deleteFileAtRoot(opq->fs, ENNBFilesystemRoot_Lib, path.str);
                        NBString_release(&path);
                    }
                }
                NBThreadMutexRW_unlockFromWrite(&itm->dataa.mutex);
            }
            //Notify
            {
                STNBStorageCacheItmLstnrOpq* lstnrs = NULL;
                SI32 lstnrsSz    = 0;
                char* rootPath    = NULL;
                char* relPath    = NULL;
                //Build notification list (locked)
                {
                    NBThreadMutex_lock(&itm->lstnrs.mutex);
                    //PRINTF_INFO("Record '%s%s' lock for LSTNRS.\n", opq->rootPath.str, itm->relPath.str);
                    if(itm->lstnrs.arr.use > 0){
                        if(opq->notifMode == ENNBStorageNotifMode_Inmediate){
                            //Create copy of lstnrs to notify after unlocking the record
                            lstnrsSz    = itm->lstnrs.arr.use;
                            lstnrs        = NBMemory_allocTypes(STNBStorageCacheItmLstnrOpq, itm->lstnrs.arr.use);
                            NBMemory_copy(lstnrs, NBArray_data(&itm->lstnrs.arr), sizeof(STNBStorageCacheItmLstnrOpq) * itm->lstnrs.arr.use);
                            NBASSERT(opq->rootBase.length <= opq->rootPath.length);
                            rootPath    = NBString_strNewBufferBytes(&opq->rootPath.str[opq->rootBase.length], (opq->rootPath.length - opq->rootBase.length));
                            relPath        = NBString_strNewBufferBytes(itm->relPath.str, itm->relPath.length);
                            //Retain all
                            {
                                SI32 i; for(i = 0 ; i < lstnrsSz; i++){
                                    STNBStorageCacheItmLstnrOpq* lstnr = &lstnrs[i];
                                    if(lstnr->methods.retain != NULL && lstnr->methods.release != NULL){
                                        (*lstnr->methods.retain)(lstnr->refObj);
                                    }
                                }
                            }
                        } else {
                            NBASSERT(opq->notifMethod != NULL)
                            if(opq->notifMethod != NULL){
                                const char* rootPath = &opq->rootPath.str[opq->rootBase.length];
                                const char* relPath = itm->relPath.str;
                                SI32 i; for(i = 0 ; i < itm->lstnrs.arr.use; i++){
                                    STNBStorageCacheItmLstnrOpq* itm2 = NBArray_itmPtrAtIndex(&itm->lstnrs.arr, STNBStorageCacheItmLstnrOpq, i);
                                    if(itm2->methods.recordChanged != NULL){
                                        (*opq->notifMethod)(opq->notifMethodParam, rootPath, relPath, itm2->refObj, &itm2->methods);
                                    }
                                }
                            }
                        }
                    }
                    //PRINTF_INFO("Record '%s%s' unlock for LSTNRS.\n", opq->rootPath.str, itm->relPath.str);
                    NBThreadMutex_unlock(&itm->lstnrs.mutex);
                }
                //Notify (storage, record and listenerList unlocked)
                if(lstnrs != NULL){
                    NBASSERT(lstnrsSz > 0)
                    NBASSERT(rootPath != NULL)
                    NBASSERT(relPath != NULL)
                    SI32 i; for(i = 0 ; i < lstnrsSz; i++){
                        STNBStorageCacheItmLstnrOpq* lstnr = &lstnrs[i];
                        //Call
                        if(lstnr->methods.recordChanged != NULL){
                            //PRINTF_INFO("NBStorageCache_returnRecordLockedFromWrite, notifying (real time): '%s' '%s'.\n", rootPath, relPath);
                            (*lstnr->methods.recordChanged)(lstnr->refObj, rootPath, relPath);
                        }
                        //Release
                        if(lstnr->methods.retain && lstnr->methods.release != NULL){
                            (*lstnr->methods.release)(lstnr->refObj);
                        }
                    }
                    NBMemory_free(rootPath); rootPath = NULL;
                    NBMemory_free(relPath); relPath = NULL;
                    NBMemory_free(lstnrs); lstnrs = NULL;
                    lstnrsSz = 0;
                }
            }
        }
        NBArray_empty(&itms);
    }
    NBArray_release(&itms);
}
