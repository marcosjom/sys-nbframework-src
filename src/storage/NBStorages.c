//
//  NBStorages.c
//  NBFramework
//
//  Created by Marcos Ortega on 1/3/19.
//

#include "nb/NBFrameworkPch.h"
#include "nb/storage/NBStorages.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBThreadStorage.h"
#include "nb/core/NBThreadMutexRW.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"

#ifndef _WIN32

//Thread local storage
static STNBThreadStorage __statePerThread = NBThreadStorage_initialValue;
void __statePerThreadCreateMthd(void);
void __statePerThreadDestroyDataMthd(void* data);

typedef struct STNBStoragesThreadStackItm_ {
	char*	fullPath;
	BOOL	isWriteOp;
} STNBStoragesThreadStackItm;

typedef struct STNBStoragesThreadState_ {
	STNBArray	stack;	//STNBStoragesThreadStackItm
} STNBStoragesThreadState;

void __statePerThreadCreateMthd(void){
	//PRINTF_INFO("__statePerThreadCreateMthd.\n");
	NBThreadStorage_create(&__statePerThread);
	NBASSERT(NULL == NBThreadStorage_getData(&__statePerThread))
}

void __statePerThreadDestroyDataMthd(void* data){
	//PRINTF_INFO("__statePerThreadDestroyDataMthd.\n");
	if(data != NULL){
		STNBStoragesThreadState* st = (STNBStoragesThreadState*)data;
		{
			SI32 i; for(i = 0 ; i < st->stack.use; i++){
				STNBStoragesThreadStackItm* itm = NBArray_itmPtrAtIndex(&st->stack, STNBStoragesThreadStackItm, i);
				if(itm->fullPath != NULL){
					NBMemory_free(itm->fullPath);
					itm->fullPath = NULL;
				}
			}
			NBArray_empty(&st->stack);
			NBArray_release(&st->stack);
		}
		NBMemory_setZeroSt(*st, STNBStoragesThreadState);
		NBMemory_free(st);
		st = NULL;
	}
}

#endif

//

typedef struct STNBStoragesNotif_ {
	UI32	iRootPath;
	UI32	iRelPath;
	void*	refObj;
	STNBStorageCacheItmLstnr methods;
} STNBStoragesNotif;

typedef struct STNBStoragesNotifs_ {
	STNBArray		pend;	//STNBStoragesNotif
	STNBThreadMutex	mutex;
	STNBString		strs;
} STNBStoragesNotifs;

typedef struct STNBStoragesItm_ {
	STNBString			relPath;	//storage relative path (without root)
	STNBStorageCache*	cache;		//Records
} STNBStoragesItm;

typedef struct STNBStoragesOpq_ {
	STNBFilesystem*			fs;
	ENNBStorageNotifMode	notifMode;
	STNBStoragesNotifs		notifs;
	STNBString				rootPath;	//prefix to be added to paths
	STNBArray				itms;		//STNBStoragesItm
	STNBThreadMutex			mutex;
	SI32					retainCount;
} STNBStoragesOpq;

void NBStorages_init(STNBStorages* obj){
	STNBStoragesOpq* opq = obj->opaque = NBMemory_allocType(STNBStoragesOpq);
	NBMemory_setZeroSt(*opq, STNBStoragesOpq);
	//Init TLS key
#	ifndef _WIN32
	{
		NBThreadStorage_initOnce(&__statePerThread, __statePerThreadCreateMthd);
	}
#	endif
	//
	opq->fs			= NULL;
	//Pend notifs
	{
		NBArray_init(&opq->notifs.pend, sizeof(STNBStoragesNotif), NULL);
		NBThreadMutex_init(&opq->notifs.mutex);
		NBString_init(&opq->notifs.strs);
	}
	opq->notifMode	= ENNBStorageNotifMode_Inmediate;
	NBString_init(&opq->rootPath);
	NBArray_init(&opq->itms, sizeof(STNBStoragesItm), NULL);
	NBThreadMutex_init(&opq->mutex);
	opq->retainCount	= 1;
}

void NBStorages_retain(STNBStorages* obj){
	STNBStoragesOpq* opq = (STNBStoragesOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	{
		NBASSERT(opq->retainCount > 0)
		opq->retainCount++;
	}
	NBThreadMutex_unlock(&opq->mutex);
}

void NBStorages_release(STNBStorages* obj){
	STNBStoragesOpq* opq = (STNBStoragesOpq*)obj->opaque;
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
					STNBStoragesItm* itm = NBArray_itmPtrAtIndex(&opq->itms, STNBStoragesItm, i);
					{
						NBString_release(&itm->relPath);
						NBStorageCache_release(itm->cache);
						NBMemory_free(itm->cache);
					}
				}
				NBArray_empty(&opq->itms);
				NBArray_release(&opq->itms);
			}
			//Pend notifs
			{
				NBThreadMutex_lock(&opq->notifs.mutex);
				{
					//Release retained
					{
						SI32 i; for(i = 0 ; i < opq->notifs.pend.use; i++){
							STNBStoragesNotif* itm = NBArray_itmPtrAtIndex(&opq->notifs.pend, STNBStoragesNotif, i);
							NBASSERT((itm->methods.retain == NULL && itm->methods.release == NULL) || (itm->methods.retain != NULL && itm->methods.release != NULL))
							if(itm->methods.retain != NULL && itm->methods.release != NULL){
								(*itm->methods.release)(itm->refObj);
							}
						}
					}
					NBArray_empty(&opq->notifs.pend);
					NBArray_release(&opq->notifs.pend);
				}
				NBString_release(&opq->notifs.strs);
				NBThreadMutex_unlock(&opq->notifs.mutex);
				NBThreadMutex_release(&opq->notifs.mutex);
			}
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

BOOL NBStorages_setFilesystem(STNBStorages* obj, STNBFilesystem* fs){
	BOOL r = FALSE;
	STNBStoragesOpq* opq = (STNBStoragesOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	if(opq->itms.use > 0){
		PRINTF_ERROR("NBStorages, forbidden to set filesystem when not-empty.\n");
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
	return r;
}

BOOL NBStorages_setNotifMode(STNBStorages* obj, const ENNBStorageNotifMode mode){
	BOOL r = FALSE;
	STNBStoragesOpq* opq = (STNBStoragesOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	if(opq->itms.use > 0){
		PRINTF_ERROR("NBStorages, forbidden to set filesystem when not-empty.\n");
	} else {
		opq->notifMode = mode;
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

const char* NBStorages_getRootRelative(const STNBStorages* obj){
	STNBStoragesOpq* opq = (STNBStoragesOpq*)obj->opaque;
	return opq->rootPath.str;
}

BOOL NBStorages_setRootRelative(STNBStorages* obj, const char* root){
	BOOL r = FALSE;
	STNBStoragesOpq* opq = (STNBStoragesOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	if(opq->itms.use > 0){
		PRINTF_ERROR("NBStorages, forbidden to set root when not-empty.\n");
	} else if(opq->fs == NULL){
		PRINTF_ERROR("NBStorages, forbidden to set when filesystem is NULL.\n");
	} else {
		NBString_set(&opq->rootPath, root);
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
	return r;
}

//

void NBStorages_notifyAdd_(void* pOpq, const char* pRootPath, const char* pRelPath, void* refObj, STNBStorageCacheItmLstnr* methods);

STNBStorageCache* NBStorages_getStorageRetained(STNBStorages* obj, const char* relPath, const BOOL createIfNew){
	STNBStorageCache* r = NULL;
	STNBStoragesOpq* opq = (STNBStoragesOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	if(opq->fs != NULL){
		//Search
		{
			SI32 i; for(i = 0; i < opq->itms.use; i++){
				STNBStoragesItm* itm = NBArray_itmPtrAtIndex(&opq->itms, STNBStoragesItm, i);
				if(NBString_isLike(&itm->relPath, relPath)){
					NBStorageCache_retain(itm->cache);
					r = itm->cache;
					break;
				}
			}
		}
		//Create new storageCache
		if(r == NULL && createIfNew){
			{
				STNBStoragesItm itm;
				NBMemory_setZeroSt(itm, STNBStoragesItm);
				NBString_initWithStr(&itm.relPath, relPath);
				itm.cache = NBMemory_allocType(STNBStorageCache);
				NBStorageCache_init(itm.cache);
				{
					STNBString path;
					NBString_init(&path);
					NBString_concat(&path, opq->rootPath.str);
					NBString_concat(&path, relPath);
					NBStorageCache_setFilesystem(itm.cache, opq->fs);
					NBStorageCache_setNotifMode(itm.cache, opq->notifMode, NBStorages_notifyAdd_, opq);
					NBStorageCache_setRoot(itm.cache, opq->rootPath.str, path.str);
					NBString_release(&path);
				}
				NBArray_addValue(&opq->itms, itm);
				//PRINTF_INFO("NBStorages, created new storageCache for: '%s'.\n", relPath);
				//Return
				NBStorageCache_retain(itm.cache);
				r = itm.cache;
			}
		}
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

//

void NBStorages_notifyAdd_(void* pOpq, const char* pRootPath, const char* pRelPath, void* refObj, STNBStorageCacheItmLstnr* methods){
	STNBStoragesOpq* opq = (STNBStoragesOpq*)pOpq;
	NBASSERT(pRootPath != NULL && pRelPath != NULL && refObj != NULL && methods != NULL)
	//Add while locked
	NBThreadMutex_lock(&opq->notifs.mutex);
	{
		SI32 i; for(i = 0 ; i < opq->notifs.pend.use; i++){
			STNBStoragesNotif* itm = NBArray_itmPtrAtIndex(&opq->notifs.pend, STNBStoragesNotif, i);
			if(itm->refObj == refObj){
				NBASSERT(itm->refObj != NULL)
				NBASSERT(itm->iRootPath >= 0 && itm->iRootPath < opq->notifs.strs.length)
				NBASSERT(itm->iRelPath >= 0 && itm->iRelPath < opq->notifs.strs.length)
				const char* rootPath = &opq->notifs.strs.str[itm->iRootPath];
				const char* relPath = &opq->notifs.strs.str[itm->iRelPath];
				if(NBString_strIsEqual(pRootPath, rootPath)){
					if(NBString_strIsEqual(pRelPath, relPath)){
						//Already added (do not add again)
						break;
					}
				}
			}
		}
		if(i < opq->notifs.pend.use){
			//Ignore to avoid multple retain-actions.
			PRINTF_INFO("NBStorages_notifyAdd_, already added (ignored): '%s' '%s'.\n", pRootPath, pRelPath);
		} else {
			//Add new
			STNBStoragesNotif itm;
			itm.refObj		= refObj;
			itm.methods		= *methods;
			itm.iRootPath	= opq->notifs.strs.length;
			NBString_concat(&opq->notifs.strs, pRootPath); NBString_concatByte(&opq->notifs.strs, '\0');
			itm.iRelPath	= opq->notifs.strs.length;
			NBString_concat(&opq->notifs.strs, pRelPath); NBString_concatByte(&opq->notifs.strs, '\0');
			//Retain
			{
				NBASSERT((itm.methods.retain == NULL && itm.methods.release == NULL) || (itm.methods.retain != NULL && itm.methods.release != NULL))
				if(itm.methods.retain != NULL && itm.methods.release != NULL){
					(*itm.methods.retain)(itm.refObj);
				}
			}
			NBArray_addValue(&opq->notifs.pend, itm);
			//PRINTF_INFO("NBStorages_notifyAdd_, added: '%s' '%s'.\n", pRootPath, pRelPath);
		}
	}
	NBThreadMutex_unlock(&opq->notifs.mutex);
}

void NBStorages_notifyAll(STNBStorages* obj){
	STNBStoragesOpq* opq = (STNBStoragesOpq*)obj->opaque;
	STNBArray		pend;	//STNBStoragesNotif
	STNBString		strs;
	{
		//Clone (locked)
		{
			NBThreadMutex_lock(&opq->notifs.mutex);
			{
				//Clone array
				NBArray_initWithOther(&pend, &opq->notifs.pend);
				NBArray_empty(&opq->notifs.pend);
				//Clone string
				NBString_initWithOther(&strs, &opq->notifs.strs);
				NBString_empty(&opq->notifs.strs);
			}
			NBThreadMutex_unlock(&opq->notifs.mutex);
		}
		//Execute (unlocked)
		{
			SI32 i; for(i = 0 ; i < pend.use; i++){
				STNBStoragesNotif* itm = NBArray_itmPtrAtIndex(&pend, STNBStoragesNotif, i);
				NBASSERT(itm->refObj != NULL)
				NBASSERT(itm->iRootPath >= 0 && itm->iRootPath < strs.length)
				NBASSERT(itm->iRelPath >= 0 && itm->iRelPath < strs.length)
				const char* rootPath = &strs.str[itm->iRootPath];
				const char* relPath = &strs.str[itm->iRelPath];
				if(itm->methods.recordChanged != NULL){
					//PRINTF_INFO("NBStorages_notifyAll, notifying (acumm): '%s' '%s'.\n", rootPath, relPath);
					(*itm->methods.recordChanged)(itm->refObj, rootPath, relPath);
				}
				//Release
				{
					NBASSERT((itm->methods.retain == NULL && itm->methods.release == NULL) || (itm->methods.retain != NULL && itm->methods.release != NULL))
					if(itm->methods.retain != NULL && itm->methods.release != NULL){
						(*itm->methods.release)(itm->refObj);
					}
				}
			}
		}
	}
	NBArray_release(&pend);
	NBString_release(&strs);
}

//Records listeners

void NBStorages_addRecordListener(STNBStorages* obj, const char* storagePath, const char* recordPath, void* refObj, const STNBStorageCacheItmLstnr* methods){
	STNBStorageCache* container = NBStorages_getStorageRetained(obj, storagePath, TRUE); NBASSERT(container != NULL)
	if(container != NULL){
		NBStorageCache_addListener(container, recordPath, refObj, methods);
		NBStorageCache_release(container);
	}
}

void NBStorages_removeRecordListener(STNBStorages* obj, const char* storagePath, const char* recordPath, void* refObj){
	STNBStorageCache* container = NBStorages_getStorageRetained(obj, storagePath, TRUE); NBASSERT(container != NULL)
	if(container != NULL){
		NBStorageCache_removeListener(container, recordPath, refObj);
		NBStorageCache_release(container);
	}
}

//Read record

STNBStorageRecordRead NBStorages_getStorageRecordForRead(STNBStorages* obj, const char* storagePath, const char* recordPath, const STNBStructMap* pubMap, const STNBStructMap* privMap){
	STNBStorageRecordRead r;
	NBMemory_setZeroSt(r, STNBStorageRecordRead);
	//Analyze if record is deeper
	{
		BOOL isDeeperOp = FALSE;
		STNBString fullPath;
		NBString_init(&fullPath);
		NBString_concat(&fullPath, storagePath);
		NBString_concat(&fullPath, recordPath);
#		ifndef _WIN32
		{
			NBThreadStorage_initOnce(&__statePerThread, __statePerThreadCreateMthd);
			{
				STNBStoragesThreadState* tState = (STNBStoragesThreadState*)NBThreadStorage_getData(&__statePerThread);
				if(tState == NULL){
					//Set initial data
					tState = NBMemory_allocType(STNBStoragesThreadState);
					NBMemory_setZeroSt(*tState, STNBStoragesThreadState);
					NBArray_init(&tState->stack, sizeof(STNBStoragesThreadStackItm), NULL); //STNBStoragesThreadStackItm
					NBThreadStorage_setData(&__statePerThread, tState, __statePerThreadDestroyDataMthd);
					NBASSERT(NBThreadStorage_getData(&__statePerThread) != NULL)
				}
				if(tState->stack.use == 0){
					//First element at the stack (just add)
					STNBStoragesThreadStackItm itm;
					NBMemory_setZeroSt(itm, STNBStoragesThreadStackItm);
					NBString_strFreeAndNewBuffer(&itm.fullPath, fullPath.str);
					itm.isWriteOp	= FALSE;
					//PRINTF_INFO("NBStorages, pushing first level: '%s' (%s).\n", itm.fullPath, (itm.isWriteOp ? "write" : "read"));
					NBArray_addValue(&tState->stack, itm);
					isDeeperOp		= TRUE;
				} else {
					STNBStoragesThreadStackItm* lastItm = NBArray_itmPtrAtIndex(&tState->stack, STNBStoragesThreadStackItm, tState->stack.use - 1);
					if(NBString_isEqual(&fullPath, lastItm->fullPath)){
						//PRINTF_ERROR("NBStorages, only deeper nested operations are allowed; to avoid tables mutual-locks: '%s' (%s) -> '%s' (%s).\n", lastItm->fullPath, (lastItm->isWriteOp ? "write" : "read"), fullPath.str, (FALSE ? "write" : "read"));
						isDeeperOp = FALSE;
					} else if(!NBString_strStartsWith(fullPath.str, lastItm->fullPath)){
						//PRINTF_ERROR("NBStorages, only deeper nested operations are allowed; to avoid tables mutual-locks: '%s' (%s) -> '%s' (%s).\n", lastItm->fullPath, (lastItm->isWriteOp ? "write" : "read"), fullPath.str, (FALSE ? "write" : "read"));
						isDeeperOp = FALSE;
					} else {
						STNBStoragesThreadStackItm itm;
						NBMemory_setZeroSt(itm, STNBStoragesThreadStackItm);
						NBString_strFreeAndNewBuffer(&itm.fullPath, fullPath.str);
						itm.isWriteOp	= FALSE;
						//PRINTF_INFO("NBStorages, pushing depper level: '%s' (%s) -> '%s' (%s).\n", lastItm->fullPath, (lastItm->isWriteOp ? "write" : "read"), itm.fullPath, (itm.isWriteOp ? "write" : "read"));
						NBArray_addValue(&tState->stack, itm);
						isDeeperOp		= TRUE;
					}
				}
#				ifdef NB_CONFIG_INCLUDE_ASSERTS
				if(!isDeeperOp){
					PRINTF_ERROR("NBStorages, pushing for read #%d '%s' (only deeper nested operations are allowed).\n", (tState->stack.use + 1), fullPath.str);
					{
						SI32 i; for(i = ((SI32)tState->stack.use - 1); i >= 0; i--){
							const STNBStoragesThreadStackItm* itm = NBArray_itmPtrAtIndex(&tState->stack, STNBStoragesThreadStackItm, i);
							PRINTF_ERROR("NBStorages, stack #%d '%s'.\n", (i + 1), itm->fullPath);
						}
					}
				}
				NBASSERT(isDeeperOp)
#				endif
			}
		}
#		else
		{
			isDeeperOp		= TRUE;
		}
#		endif
		if(isDeeperOp){
			r.stack.pushed	= TRUE;
			NBString_initWithOther(&r.stack.fullpath, &fullPath);
			{
				STNBStorageCache* container = NBStorages_getStorageRetained(obj, storagePath, TRUE); //container must be  loaded allways
				if(container != NULL){
					const STNBStorageCacheItmData* data = NBStorageCache_getRecordLockedForRead(container, recordPath, pubMap, privMap, TRUE /*createIfFileExists*/, FALSE /*creatIfNew*/);
					if(data == NULL){
						NBStorageCache_release(container);
					} else {
						r.data				= data;
						r.extra.hadData		= TRUE;
						r.extra.container	= container;
						r.extra.privMap		= privMap;
						r.extra.pubMap		= pubMap;
#						if defined(SSTORAGE_VALIDATE_RETURNED_STRUCTS_USING_JSON_SHA) && defined(CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT)
						{
							STNBString str;
							NBString_init(&str);
							{
								NBASSERT((data->priv.stData != NULL && privMap != NULL) || (data->priv.stData == NULL && privMap == NULL))
								if(data->priv.stData != NULL && privMap != NULL){
									NBString_empty(&str);
									NBStruct_stConcatAsJson(&str, privMap, data->priv.stData, privMap->stSize);
									r.extra.privHash = NBSha1_getHashBytes(str.str, str.length);
								}
							}
							{
								NBASSERT((data->pub.stData != NULL && pubMap != NULL) || (data->pub.stData == NULL && pubMap == NULL))
								if(data->pub.stData != NULL && pubMap != NULL){
									NBString_empty(&str);
									NBStruct_stConcatAsJson(&str, pubMap, data->pub.stData, pubMap->stSize);
									r.extra.pubHash = NBSha1_getHashBytes(str.str, str.length);
								}
							}
							NBString_release(&str);
						}
#						endif
#						if defined(SSTORAGE_VALIDATE_RETURNED_STRUCTS_USING_BIN_CRC32) && defined(CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT)
						{
							{
								NBASSERT((data->priv.stData != NULL && privMap != NULL) || (data->priv.stData == NULL && privMap == NULL))
								if(data->priv.stData != NULL && privMap != NULL){
									r.extra.privCrc = NBStruct_stCalculateCrc(privMap, data->priv.stData, privMap->stSize);
								}
							}
							{
								NBASSERT((data->pub.stData != NULL && pubMap != NULL) || (data->pub.stData == NULL && pubMap == NULL))
								if(data->pub.stData != NULL && pubMap != NULL){
									r.extra.pubCrc = NBStruct_stCalculateCrc(pubMap, data->pub.stData, pubMap->stSize);
								}
							}
						}
#						endif
					}
				}
			}
		}
		NBString_release(&fullPath);
	}
	return r;
}

void NBStorages_returnStorageRecordFromRead(STNBStorages* obj, STNBStorageRecordRead* ref){
	NBASSERT(ref != NULL)
	if(ref != NULL){
		NBASSERT((ref->data == NULL && !ref->extra.hadData) || (ref->data != NULL && ref->extra.hadData)) //If fails, data was nullyfied or added (not allowed in read ops)
		BOOL isValidOp = FALSE;
#		ifndef _WIN32
		if(ref->stack.pushed){
			NBThreadStorage_initOnce(&__statePerThread, __statePerThreadCreateMthd);
			{
				STNBStoragesThreadState* tState = (STNBStoragesThreadState*)NBThreadStorage_getData(&__statePerThread);
				if(tState == NULL){
					//PRINTF_INFO("NBStorages, popping when stack is NULL.\n");
					isValidOp		= FALSE;
					NBASSERT(isValidOp)
				} else if(tState->stack.use == 0){
					//PRINTF_INFO("NBStorages, popping when stack is empty.\n");
					isValidOp		= FALSE;
					NBASSERT(isValidOp)
				} else {
					STNBStoragesThreadStackItm* lastItm = NBArray_itmPtrAtIndex(&tState->stack, STNBStoragesThreadStackItm, tState->stack.use - 1);
					if(!NBString_isEqual(&ref->stack.fullpath, lastItm->fullPath)){
						//PRINTF_ERROR("NBStorages, unexpected popping; expected(%s), found(%s).\n", lastItm->fullPath, ref->stack.fullpath.str);
						isValidOp = FALSE;
						NBASSERT(isValidOp)
					} else if(lastItm->isWriteOp){
						//PRINTF_ERROR("NBStorages, unexpected popping; expected-from(read) found-from(write).\n");
						isValidOp = FALSE;
						NBASSERT(isValidOp)
					} else {
						//Remove last itm
						//PRINTF_INFO("NBStorages, popping level: from ('%s').\n", lastItm->fullPath);
						if(lastItm->fullPath != NULL) NBMemory_free(lastItm->fullPath);
						NBArray_removeItemAtIndex(&tState->stack, tState->stack.use - 1);
						isValidOp	= TRUE;
					}
				}
				NBASSERT(isValidOp)
			}
			NBString_release(&ref->stack.fullpath);
			ref->stack.pushed = FALSE;
		}
#		else
		{
			isValidOp	= TRUE;
		}
#		endif
		//Release record
		if(ref->data != NULL){
			if(isValidOp){
#				if defined(SSTORAGE_VALIDATE_RETURNED_STRUCTS_USING_JSON_SHA) && defined(CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT)
				{
					STNBString str;
					NBString_init(&str);
					{
						NBASSERT((ref->data->priv.stData != NULL && ref->extra.privMap != NULL) || (ref->data->priv.stData == NULL && ref->extra.privMap == NULL))
						if(ref->data->priv.stData != NULL && ref->extra.privMap != NULL){
							NBString_empty(&str);
							NBStruct_stConcatAsJson(&str, ref->extra.privMap, ref->data->priv.stData, ref->extra.privMap->stSize);
							const STNBSha1Hash hash = NBSha1_getHashBytes(str.str, str.length);
							NBASSERT(NBString_strIsEqualBytes((const char*)&hash, sizeof(hash), (const char*)&ref->extra.privHash, sizeof(hash))) //Record was modified, not allowed
						}
					}
					{
						NBASSERT((ref->data->pub.stData != NULL && ref->extra.pubMap != NULL) || (ref->data->pub.stData == NULL && ref->extra.pubMap == NULL))
						if(ref->data->pub.stData != NULL && ref->extra.pubMap != NULL){
							NBString_empty(&str);
							NBStruct_stConcatAsJson(&str, ref->extra.pubMap, ref->data->pub.stData, ref->extra.pubMap->stSize);
							const STNBSha1Hash hash = NBSha1_getHashBytes(str.str, str.length);
							NBASSERT(NBString_strIsEqualBytes((const char*)&hash, sizeof(hash), (const char*)&ref->extra.pubHash, sizeof(hash))) //Record was modified, not allowed
						}
					}
					NBString_release(&str);
				}
#				endif
#				if defined(SSTORAGE_VALIDATE_RETURNED_STRUCTS_USING_BIN_CRC32) && defined(CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT)
				{
					{
						NBASSERT((ref->data->priv.stData != NULL && ref->extra.privMap != NULL) || (ref->data->priv.stData == NULL && ref->extra.privMap == NULL))
						if(ref->data->priv.stData != NULL && ref->extra.privMap != NULL){
							const STNBStructCrc crc = NBStruct_stCalculateCrc(ref->extra.privMap, ref->data->priv.stData, ref->extra.privMap->stSize);
							NBASSERT(crc.crc32 == ref->extra.privCrc.crc32 && crc.bytesFed == ref->extra.privCrc.bytesFed)
						}
					}
					{
						NBASSERT((ref->data->pub.stData != NULL && ref->extra.pubMap != NULL) || (ref->data->pub.stData == NULL && ref->extra.pubMap == NULL))
						if(ref->data->pub.stData != NULL && ref->extra.pubMap != NULL){
							const STNBStructCrc crc = NBStruct_stCalculateCrc(ref->extra.pubMap, ref->data->pub.stData, ref->extra.pubMap->stSize);
							NBASSERT(crc.crc32 == ref->extra.pubCrc.crc32 && crc.bytesFed == ref->extra.pubCrc.bytesFed)
						}
					}
				}
#				endif
				//Release container
				NBASSERT(ref->extra.container != NULL)
				if(ref->extra.container != NULL){
					NBStorageCache_returnRecordLockedFromRead(ref->extra.container, ref->data);
					NBStorageCache_release(ref->extra.container);
					ref->extra.container = NULL;
				}
				ref->extra.hadData = FALSE;
				ref->data = NULL;
			}
		}
	}
}

//Write record

STNBStorageRecordWrite NBStorages_getStorageRecordForWrite(STNBStorages* obj, const char* storagePath, const char* recordPath, const BOOL createIfNew, const STNBStructMap* pubMap, const STNBStructMap* privMap){
	STNBStorageRecordWrite r;
	NBMemory_setZeroSt(r, STNBStorageRecordWrite);
	{
		BOOL isDeeperOp = FALSE;
		STNBString fullPath;
		NBString_init(&fullPath);
		NBString_concat(&fullPath, storagePath);
		NBString_concat(&fullPath, recordPath);
#		ifndef _WIN32
		{
			NBThreadStorage_initOnce(&__statePerThread, __statePerThreadCreateMthd);
			{
				STNBStoragesThreadState* tState = (STNBStoragesThreadState*)NBThreadStorage_getData(&__statePerThread);
				if(tState == NULL){
					//Set initial data
					tState = NBMemory_allocType(STNBStoragesThreadState);
					NBMemory_setZeroSt(*tState, STNBStoragesThreadState);
					NBArray_init(&tState->stack, sizeof(STNBStoragesThreadStackItm), NULL); //STNBStoragesThreadStackItm
					NBThreadStorage_setData(&__statePerThread, tState, __statePerThreadDestroyDataMthd);
					NBASSERT(NBThreadStorage_getData(&__statePerThread) != NULL)
				}
				if(tState->stack.use == 0){
					//First element at the stack (just add)
					STNBStoragesThreadStackItm itm;
					NBMemory_setZeroSt(itm, STNBStoragesThreadStackItm);
					NBString_strFreeAndNewBuffer(&itm.fullPath, fullPath.str);
					itm.isWriteOp	= TRUE;
					//PRINTF_INFO("NBStorages, pushing first level: '%s' (%s).\n", itm.fullPath, (itm.isWriteOp ? "write" : "read"));
					NBArray_addValue(&tState->stack, itm);
					isDeeperOp		= TRUE;
				} else {
					STNBStoragesThreadStackItm* lastItm = NBArray_itmPtrAtIndex(&tState->stack, STNBStoragesThreadStackItm, tState->stack.use - 1);
					if(NBString_isEqual(&fullPath, lastItm->fullPath)){
						//PRINTF_ERROR("NBStorages, only deeper nested operations are allowed; to avoid tables mutual-locks: '%s' (%s) -> '%s' (%s).\n", lastItm->fullPath, (lastItm->isWriteOp ? "write" : "read"), fullPath.str, (TRUE ? "write" : "read"));
						isDeeperOp = FALSE;
					} else if(!NBString_strStartsWith(fullPath.str, lastItm->fullPath)){
						//PRINTF_ERROR("NBStorages, only deeper nested operations are allowed; to avoid tables mutual-locks: '%s' (%s) -> '%s' (%s).\n", lastItm->fullPath, (lastItm->isWriteOp ? "write" : "read"), fullPath.str, (TRUE ? "write" : "read"));
						isDeeperOp = FALSE;
					} else {
						STNBStoragesThreadStackItm itm;
						NBMemory_setZeroSt(itm, STNBStoragesThreadStackItm);
						NBString_strFreeAndNewBuffer(&itm.fullPath, fullPath.str);
						itm.isWriteOp	= TRUE;
						//PRINTF_INFO("NBStorages, pushing depper level: '%s' (%s) from '%s' (%s).\n", itm.fullPath, (itm.isWriteOp ? "write" : "read"), lastItm->fullPath, (lastItm->isWriteOp ? "write" : "read"));
						NBArray_addValue(&tState->stack, itm);
						isDeeperOp		= TRUE;
					}
				}
#				ifdef NB_CONFIG_INCLUDE_ASSERTS
				if(!isDeeperOp){
					PRINTF_ERROR("NBStorages, pushing for write #%d '%s' (only deeper nested operations are allowed).\n", (tState->stack.use + 1), fullPath.str);
					{
						SI32 i; for(i = ((SI32)tState->stack.use - 1); i >= 0; i--){
							const STNBStoragesThreadStackItm* itm = NBArray_itmPtrAtIndex(&tState->stack, STNBStoragesThreadStackItm, i);
							PRINTF_ERROR("NBStorages, stack #%d '%s'.\n", (i + 1), itm->fullPath);
						}
					}
				}
				NBASSERT(isDeeperOp)
#				endif
			}
		}
#		else
		{
			isDeeperOp		= TRUE;
		}
#		endif
		if(isDeeperOp){
			r.stack.pushed = TRUE;
			NBString_initWithOther(&r.stack.fullpath, &fullPath);
			{
				STNBStorageCache* container = NBStorages_getStorageRetained(obj, storagePath, TRUE); //container must be allways loaded
				if(container != NULL){
					STNBStorageCacheItmData* data = NBStorageCache_getRecordLockedForWrite(container, recordPath, pubMap, privMap, TRUE /*createIfFileExists*/, createIfNew);
					if(data == NULL){
						NBStorageCache_release(container);
					} else {
						r.data				= data;
						r.extra.hadData		= TRUE;
						r.extra.container	= container;
						r.extra.pubMap		= pubMap;
						r.extra.privMap		= privMap;
#						if defined(SSTORAGE_VALIDATE_RETURNED_STRUCTS_USING_JSON_SHA) && defined(CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT)
						{
							STNBString str;
							NBString_init(&str);
							{
								NBASSERT((data->priv.stData != NULL && privMap != NULL) || (data->priv.stData == NULL && privMap == NULL))
								if(data->priv.stData != NULL && privMap != NULL){
									NBString_empty(&str);
									NBStruct_stConcatAsJson(&str, privMap, data->priv.stData, privMap->stSize);
									r.extra.privHash = NBSha1_getHashBytes(str.str, str.length);
								}
							}
							{
								NBASSERT((data->pub.stData != NULL && pubMap != NULL) || (data->pub.stData == NULL && pubMap == NULL))
								if(data->pub.stData != NULL && pubMap != NULL){
									NBString_empty(&str);
									NBStruct_stConcatAsJson(&str, pubMap, data->pub.stData, pubMap->stSize);
									r.extra.pubHash = NBSha1_getHashBytes(str.str, str.length);
								}
							}
							NBString_release(&str);
						}
#						endif
#						if defined(SSTORAGE_VALIDATE_RETURNED_STRUCTS_USING_BIN_CRC32) && defined(CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT)
						{
							{
								NBASSERT((data->priv.stData != NULL && privMap != NULL) || (data->priv.stData == NULL && privMap == NULL))
								if(data->priv.stData != NULL && privMap != NULL){
									r.extra.privCrc = NBStruct_stCalculateCrc(privMap, data->priv.stData, privMap->stSize);
								}
							}
							{
								NBASSERT((data->pub.stData != NULL && pubMap != NULL) || (data->pub.stData == NULL && pubMap == NULL))
								if(data->pub.stData != NULL && pubMap != NULL){
									r.extra.pubCrc = NBStruct_stCalculateCrc(pubMap, data->pub.stData, pubMap->stSize);
								}
							}
						}
#						endif
					}
				}
			}
		}
		NBString_release(&fullPath);
	}
	return r;
}

void NBStorages_returnStorageRecordFromWrite(STNBStorages* obj, STNBStorageRecordWrite* ref){
	NBASSERT(ref != NULL)
	if(ref != NULL){
		NBASSERT((ref->data == NULL && !ref->extra.hadData) || (ref->data != NULL && ref->extra.hadData)) //If fails, data was nullyfied or added (not allowed in Write ops)
		BOOL isValidOp = FALSE;
#		ifndef _WIN32
		if(ref->stack.pushed){
			NBThreadStorage_initOnce(&__statePerThread, __statePerThreadCreateMthd);
			{
				STNBStoragesThreadState* tState = (STNBStoragesThreadState*)NBThreadStorage_getData(&__statePerThread);
				if(tState == NULL){
					//PRINTF_INFO("NBStorages, popping when stack is NULL.\n");
					isValidOp		= FALSE;
					NBASSERT(isValidOp)
				} else if(tState->stack.use == 0){
					//PRINTF_INFO("NBStorages, popping when stack is empty.\n");
					isValidOp		= FALSE;
					NBASSERT(isValidOp)
				} else {
					STNBStoragesThreadStackItm* lastItm = NBArray_itmPtrAtIndex(&tState->stack, STNBStoragesThreadStackItm, tState->stack.use - 1);
					if(!NBString_isEqual(&ref->stack.fullpath, lastItm->fullPath)){
						PRINTF_ERROR("NBStorages, unexpected popping; expected(%s), found(%s).\n", lastItm->fullPath, ref->stack.fullpath.str);
						isValidOp = FALSE;
						NBASSERT(isValidOp)
					} else if(!lastItm->isWriteOp){
						PRINTF_ERROR("NBStorages, unexpected popping; expected-from(write) found-from(read).\n");
						isValidOp = FALSE;
						NBASSERT(isValidOp)
					} else {
						//Remove last itm
						//PRINTF_INFO("NBStorages, popping level: from ('%s').\n", lastItm->fullPath);
						if(lastItm->fullPath != NULL) NBMemory_free(lastItm->fullPath);
						NBArray_removeItemAtIndex(&tState->stack, tState->stack.use - 1);
						isValidOp	= TRUE;
					}
				}
				NBASSERT(isValidOp)
			}
			NBString_release(&ref->stack.fullpath);
			ref->stack.pushed = FALSE;
		}
#		else
		{
			isValidOp	= TRUE;
		}
#		endif
		//Release record
		if(ref->data != NULL){
			if(isValidOp){
#				if defined(SSTORAGE_VALIDATE_RETURNED_STRUCTS_USING_JSON_SHA) && defined(CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT)
				{
					STNBString str;
					NBString_initWithSz(&str, 4096);
					{
						NBASSERT((ref->data->priv.stData != NULL && ref->extra.privMap != NULL) || (ref->data->priv.stData == NULL && ref->extra.privMap == NULL))
						if(ref->data->priv.stData != NULL && ref->extra.privMap != NULL){
							NBString_empty(&str);
							NBStruct_stConcatAsJson(&str, ref->extra.privMap, ref->data->priv.stData, ref->extra.privMap->stSize);
							const STNBSha1Hash hash = NBSha1_getHashBytes(str.str, str.length);
							const BOOL modif = (NBString_strIsEqualBytes((const char*)&hash, sizeof(hash), (const char*)&ref->extra.privHash, sizeof(hash)) ? FALSE : TRUE);
							NBASSERT(ref->privModified || (!modif && !ref->privModified))
						}
					}
					{
						NBASSERT((ref->data->pub.stData != NULL && ref->extra.pubMap != NULL) || (ref->data->pub.stData == NULL && ref->extra.pubMap == NULL))
						if(ref->data->pub.stData != NULL && ref->extra.pubMap != NULL){
							NBString_empty(&str);
							NBStruct_stConcatAsJson(&str, ref->extra.pubMap, ref->data->pub.stData, ref->extra.pubMap->stSize);
							const STNBSha1Hash hash = NBSha1_getHashBytes(str.str, str.length);
							const BOOL modif = (NBString_strIsEqualBytes((const char*)&hash, sizeof(hash), (const char*)&ref->extra.pubHash, sizeof(hash)) ? FALSE : TRUE);
							NBASSERT(ref->pubModified || (!modif && !ref->pubModified))
						}
					}
					NBString_release(&str);
				}
#				endif
#				if defined(SSTORAGE_VALIDATE_RETURNED_STRUCTS_USING_BIN_CRC32) && defined(CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT)
				{
					{
						NBASSERT((ref->data->priv.stData != NULL && ref->extra.privMap != NULL) || (ref->data->priv.stData == NULL && ref->extra.privMap == NULL))
						if(ref->data->priv.stData != NULL && ref->extra.privMap != NULL){
							const STNBStructCrc crc = NBStruct_stCalculateCrc(ref->extra.privMap, ref->data->priv.stData, ref->extra.privMap->stSize);
							NBASSERT(ref->privModified || (crc.crc32 == ref->extra.privCrc.crc32 && crc.bytesFed == ref->extra.privCrc.bytesFed))
						}
					}
					{
						NBASSERT((ref->data->pub.stData != NULL && ref->extra.pubMap != NULL) || (ref->data->pub.stData == NULL && ref->extra.pubMap == NULL))
						if(ref->data->pub.stData != NULL && ref->extra.pubMap != NULL){
							const STNBStructCrc crc = NBStruct_stCalculateCrc(ref->extra.pubMap, ref->data->pub.stData, ref->extra.pubMap->stSize);
							NBASSERT(ref->pubModified || (crc.crc32 == ref->extra.pubCrc.crc32 && crc.bytesFed == ref->extra.pubCrc.bytesFed))
						}
					}
				}
#				endif
				//Release container
				NBASSERT(ref->extra.container != NULL)
				if(ref->extra.container != NULL){
					NBStorageCache_returnRecordLockedFromWrite(ref->extra.container, ref->data, ref->pubModified, ref->privModified);
					NBStorageCache_release(ref->extra.container);
					ref->extra.container = NULL;
				}
				ref->extra.hadData = FALSE;
				ref->data = NULL;
			}
		}
	}
}

//Touch record (trigger a notification to listeners without saving to file)

void NBStorages_touchRecord(STNBStorages* obj, const char* storagePath, const char* recordPath){
	BOOL isDeeperOp = FALSE;
	STNBString fullPath;
	NBString_init(&fullPath);
	NBString_concat(&fullPath, storagePath);
	NBString_concat(&fullPath, recordPath);
	//Push verification stack
#	ifndef _WIN32
	{
		NBThreadStorage_initOnce(&__statePerThread, __statePerThreadCreateMthd);
		{
			STNBStoragesThreadState* tState = (STNBStoragesThreadState*)NBThreadStorage_getData(&__statePerThread);
			if(tState == NULL){
				//Set initial data
				tState = NBMemory_allocType(STNBStoragesThreadState);
				NBMemory_setZeroSt(*tState, STNBStoragesThreadState);
				NBArray_init(&tState->stack, sizeof(STNBStoragesThreadStackItm), NULL); //STNBStoragesThreadStackItm
				NBThreadStorage_setData(&__statePerThread, tState, __statePerThreadDestroyDataMthd);
				NBASSERT(NBThreadStorage_getData(&__statePerThread) != NULL)
			}
			if(tState->stack.use == 0){
				//First element at the stack (just add)
				STNBStoragesThreadStackItm itm;
				NBMemory_setZeroSt(itm, STNBStoragesThreadStackItm);
				NBString_strFreeAndNewBuffer(&itm.fullPath, fullPath.str);
				itm.isWriteOp	= TRUE;
				//PRINTF_INFO("NBStorages, pushing first level: '%s' (%s).\n", itm.fullPath, (itm.isWriteOp ? "write" : "read"));
				NBArray_addValue(&tState->stack, itm);
				isDeeperOp		= TRUE;
			} else {
				STNBStoragesThreadStackItm* lastItm = NBArray_itmPtrAtIndex(&tState->stack, STNBStoragesThreadStackItm, tState->stack.use - 1);
				if(NBString_isEqual(&fullPath, lastItm->fullPath)){
					//PRINTF_ERROR("NBStorages, only deeper nested operations are allowed; to avoid tables mutual-locks: '%s' (%s) -> '%s' (%s).\n", lastItm->fullPath, (lastItm->isWriteOp ? "write" : "read"), fullPath.str, (TRUE ? "write" : "read"));
					isDeeperOp = FALSE;
				} else if(!NBString_strStartsWith(fullPath.str, lastItm->fullPath)){
					//PRINTF_ERROR("NBStorages, only deeper nested operations are allowed; to avoid tables mutual-locks: '%s' (%s) -> '%s' (%s).\n", lastItm->fullPath, (lastItm->isWriteOp ? "write" : "read"), fullPath.str, (TRUE ? "write" : "read"));
					isDeeperOp = FALSE;
				} else {
					STNBStoragesThreadStackItm itm;
					NBMemory_setZeroSt(itm, STNBStoragesThreadStackItm);
					NBString_strFreeAndNewBuffer(&itm.fullPath, fullPath.str);
					itm.isWriteOp	= TRUE;
					//PRINTF_INFO("NBStorages, pushing depper level: '%s' (%s) from '%s' (%s).\n", itm.fullPath, (itm.isWriteOp ? "write" : "read"), lastItm->fullPath, (lastItm->isWriteOp ? "write" : "read"));
					NBArray_addValue(&tState->stack, itm);
					isDeeperOp		= TRUE;
				}
			}
#			ifdef NB_CONFIG_INCLUDE_ASSERTS
			if(!isDeeperOp){
				PRINTF_ERROR("NBStorages, pushing for touch #%d '%s' (only deeper nested operations are allowed).\n", (tState->stack.use + 1), fullPath.str);
				{
					SI32 i; for(i = ((SI32)tState->stack.use - 1); i >= 0; i--){
						const STNBStoragesThreadStackItm* itm = NBArray_itmPtrAtIndex(&tState->stack, STNBStoragesThreadStackItm, i);
						PRINTF_ERROR("NBStorages, stack #%d '%s'.\n", (i + 1), itm->fullPath);
					}
				}
			}
			NBASSERT(isDeeperOp)
#			endif
		}
	}
#	else
	{
		isDeeperOp		= TRUE;
	}
#	endif
	if(isDeeperOp){
		//Touch record
		{
			STNBStorageCache* container = NBStorages_getStorageRetained(obj, storagePath, TRUE); //container must be  loaded allways
			if(container != NULL){
				NBStoragesCache_touchRecord(container, recordPath);
				NBStorageCache_release(container);
			}
		}
		//Pop verification stack
#		ifndef _WIN32
		{
			BOOL isValidOp = FALSE;
			STNBStoragesThreadState* tState = (STNBStoragesThreadState*)NBThreadStorage_getData(&__statePerThread);
			if(tState == NULL){
				//PRINTF_INFO("NBStorages, popping when stack is NULL.\n");
				isValidOp		= FALSE;
				NBASSERT(isValidOp)
			} else if(tState->stack.use == 0){
				//PRINTF_INFO("NBStorages, popping when stack is empty.\n");
				isValidOp		= FALSE;
				NBASSERT(isValidOp)
			} else {
				STNBStoragesThreadStackItm* lastItm = NBArray_itmPtrAtIndex(&tState->stack, STNBStoragesThreadStackItm, tState->stack.use - 1);
				if(!NBString_isEqual(&fullPath, lastItm->fullPath)){
					PRINTF_ERROR("NBStorages, unexpected popping; expected(%s), found(%s).\n", lastItm->fullPath, fullPath.str);
					isValidOp = FALSE;
					NBASSERT(isValidOp)
				} else if(!lastItm->isWriteOp){
					PRINTF_ERROR("NBStorages, unexpected popping; expected-from(write) found-from(read).\n");
					isValidOp = FALSE;
					NBASSERT(isValidOp)
				} else {
					//Remove last itm
					//PRINTF_INFO("NBStorages, popping level: from ('%s').\n", lastItm->fullPath);
					if(lastItm->fullPath != NULL) NBMemory_free(lastItm->fullPath);
					NBArray_removeItemAtIndex(&tState->stack, tState->stack.use - 1);
					isValidOp	= TRUE;
				}
			}
			NBASSERT(isValidOp)
		}
#		endif
	}
	NBString_release(&fullPath);
}

//List

void NBStorages_getFiles(STNBStorages* obj, const char* storagePath, const BOOL includeStats, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/){
	STNBStoragesOpq* opq = (STNBStoragesOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	{
		STNBString fullPath;
		NBString_init(&fullPath);
		NBString_concat(&fullPath, opq->rootPath.str);
		//Concat base
		if(fullPath.length > 0){
			if(fullPath.str[fullPath.length - 1] != '/' && fullPath.str[fullPath.length - 1] != '\\'){
				NBString_concat(&fullPath, "/");
			}
		}
		//Concat relPath
		if(!NBString_strIsEmpty(storagePath)){
			SI32 iFirstChar = 0;
			while(storagePath[iFirstChar] == '/' || storagePath[iFirstChar] == '\\'){
				iFirstChar++;
			}
			NBString_concat(&fullPath, &storagePath[iFirstChar]);
		}
		//Get files
		if(opq->fs != NULL){
			NBFilesystem_getFilesAtRoot(opq->fs, ENNBFilesystemRoot_Lib, fullPath.str, includeStats, dstStrs, dstFiles);
		}
		NBString_release(&fullPath);
	}
	NBThreadMutex_unlock(&opq->mutex);
}

//Reset and delete

void NBStorages_resetAllStorages(STNBStorages* obj, const BOOL deleteFiles){
	STNBStoragesOpq* opq = (STNBStoragesOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	if(opq->fs != NULL){
		//Reset records
		{
			SI32 i; for(i = 0; i < opq->itms.use; i++){
				STNBStoragesItm* itm = NBArray_itmPtrAtIndex(&opq->itms, STNBStoragesItm, i);
				NBStoragesCache_resetAllRecords(itm->cache, deleteFiles);
			}
		}
		//Delete files (records not loaded at cache wont be touched)
		{
			NBFilesystem_deleteFilesAtFolderAtRoot(opq->fs, ENNBFilesystemRoot_Lib, opq->rootPath.str, TRUE);
		}
	}
	NBThreadMutex_unlock(&opq->mutex);
}
