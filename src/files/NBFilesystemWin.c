//
//  NBFilesystemWin.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/25/18.
//

#include "nb/NBFrameworkPch.h"
//Complementary PCH
//#define WIN32_LEAN_AND_MEAN
#include <windows.h>	//for HANDLE and more
#include <Shlobj.h> //for 'SHGetFolderPath'
//
#include "nb/files/NBFilesystemWin.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"

typedef struct STNBFilesystemWinOpq_ {
	//Values
	STNBString			roots[ENNBFilesystemRoot_Count];
	UI32				retainCount;
} STNBFilesystemWinOpq;

//Filesystem itf
void NBFilesystemWin_retain_(void* obj);
void NBFilesystemWin_release_(void** obj);
//
BOOL NBFilesystemWin_concatRootPath_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* fileName, STNBString* dst);
//
BOOL NBFilesystemWin_getFolders_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
BOOL NBFilesystemWin_folderExists_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath);
BOOL NBFilesystemWin_createFolder_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath);
BOOL NBFilesystemWin_deleteFolder_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath);
//
BOOL NBFilesystemWin_open_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* filepath, const ENNBFileMode mode, STNBFileRef dst);
BOOL NBFilesystemWin_getFiles_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath, const BOOL includeStats, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
BOOL NBFilesystemWin_moveFile_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot srcRoot, const char* srcFilepath, const ENNBFilesystemRoot dstRoot, const char* dstFilepath);
BOOL NBFilesystemWin_deleteFile_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* filepath);

//

void NBFilesystemWin_init(STNBFilesystemWin* obj){
	STNBFilesystemWinOpq* opq = obj->opaque = NBMemory_allocType(STNBFilesystemWinOpq);
	NBMemory_setZeroSt(*opq, STNBFilesystemWinOpq);
	//
	{
		UI32 i; for(i = 0; i < ENNBFilesystemRoot_Count; i++){
			STNBString* str = &opq->roots[i];
			//Load value
			switch(i) {
				case ENNBFilesystemRoot_AppBundle:
					NBString_initWithStr(str, "./");
					break;
				case ENNBFilesystemRoot_Docs:
					{
						CHAR strPath[MAX_PATH + 1];
						HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL /*FOLDERID_Documents*/, NULL, SHGFP_TYPE_CURRENT, strPath);
						if (result == S_OK){
							NBString_initWithStr(str, strPath);
						}
					}
					break;
				case ENNBFilesystemRoot_Lib:
					{
						CHAR strPath[MAX_PATH + 1];
						HRESULT result = SHGetFolderPath(NULL, CSIDL_APPDATA /*FOLDERID_RoamingAppData*/, NULL, SHGFP_TYPE_CURRENT, strPath);
						if (result == S_OK){
							NBString_initWithStr(str, strPath);
						}
					}
					break;
				case ENNBFilesystemRoot_Cache:
					{
						CHAR strPath[MAX_PATH + 1];
						if(GetTempPath(MAX_PATH, strPath) != 0){
							NBString_initWithStr(str, strPath);
						}
					}
					break;
				default:
					NBString_init(str);
					break;
			}
			//Set value
			if(str->length > 0){
				if(str->str[str->length - 1] != '/'){
					NBString_concatByte(str, '/');
				}
			}
		}
	}
	opq->retainCount = 1;
}

void NBFilesystemWin_retain(STNBFilesystemWin* obj){
	NBFilesystemWin_retain_(obj->opaque);
}

void NBFilesystemWin_release(STNBFilesystemWin* obj){
	NBFilesystemWin_release_(&obj->opaque);
}

//

void NBFilesystemWin_createItf(void* pObj, IFilesystemItf* dst){
	STNBFilesystemWin* obj = (STNBFilesystemWin*)pObj;
	STNBFilesystemWinOpq* opq = (STNBFilesystemWinOpq*)obj->opaque;
	//Set iterface
	NBMemory_set(dst, 0, sizeof(*dst));
	//
	dst->obj			= opq;
	//
	dst->retain			= NBFilesystemWin_retain_;
	dst->release		= NBFilesystemWin_release_;
	//
	dst->concatRootPath	= NBFilesystemWin_concatRootPath_;
	//
	dst->getFolders		= NBFilesystemWin_getFolders_;
	dst->folderExists	= NBFilesystemWin_folderExists_;
	dst->createFolder	= NBFilesystemWin_createFolder_;
	dst->deleteFolder	= NBFilesystemWin_deleteFolder_;
	//
	dst->open			= NBFilesystemWin_open_;
	dst->getFiles		= NBFilesystemWin_getFiles_;
	dst->moveFile		= NBFilesystemWin_moveFile_;
	dst->deleteFile		= NBFilesystemWin_deleteFile_;
}

//Filesystem itf

void NBFilesystemWin_retain_(void* obj){
	STNBFilesystemWinOpq* opq = (STNBFilesystemWinOpq*)obj;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount++;
}

void NBFilesystemWin_release_(void** obj){
	STNBFilesystemWinOpq* opq = (STNBFilesystemWinOpq*)*obj;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount--;
	if(opq->retainCount == 0){
		//Paths
		{
			UI32 i; for(i = 0; i < ENNBFilesystemRoot_Count; i++){
				STNBString* str = &opq->roots[i];
				NBString_release(str);
			}
		}
		//Memory
		NBMemory_free(*obj);
		*obj = NULL;
	}
}

//

BOOL NBFilesystemWin_concatRootPath_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* fileName, STNBString* dst){
	BOOL r = FALSE;
	STNBFilesystemWinOpq* opq = (STNBFilesystemWinOpq*)obj;
	if(root >= 0 && root < ENNBFilesystemRoot_Count){
		if(dst != NULL){
			BOOL removeSlashes = FALSE;
			const char* rootStr = opq->roots[root].str;
			if(*rootStr != '\0'){
				NBString_concat(dst, rootStr);
				removeSlashes = TRUE;
			}
			if(fileName != NULL){
				if(removeSlashes){
					while(*fileName == '/' || *fileName == '\\'){
						fileName++;
					}
				}
				NBString_concat(dst, fileName);
			}
		}
		r = TRUE;
	}
	return r;
}


//

BOOL NBFilesystemWin_getFolders_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/){
	BOOL r = FALSE;
	if (parents != NULL && parentsSz > 0) {
		STNBFilesystemWinOpq* opq = (STNBFilesystemWinOpq*)obj;
		const IFilesystemItf* pItf = &parents[0];
		if (pItf->getFolders != NULL) {
			const STNBString* strRoot = &opq->roots[root];
			if (strRoot->length > 0) {
				STNBString tmpPath;
				NBString_initWithStringAndStr(&tmpPath, strRoot, folderpath);
				r = (*pItf->getFolders)(pItf->obj, &parents[1], (parentsSz - 1), ENNBFilesystemRoot_WorkDir, tmpPath.str, dstStrs, dstFiles);
				NBString_release(&tmpPath);
			} else {
				r = (*pItf->getFolders)(pItf->obj, &parents[1], (parentsSz - 1), root, folderpath, dstStrs, dstFiles);
			}
		}
	}
	return r;
}

BOOL NBFilesystemWin_folderExists_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath){
	BOOL r = FALSE;
	if (parents != NULL && parentsSz > 0) {
		STNBFilesystemWinOpq* opq = (STNBFilesystemWinOpq*)obj;
		const IFilesystemItf* pItf = &parents[0];
		if (pItf->folderExists != NULL) {
			const STNBString* strRoot = &opq->roots[root];
			if (strRoot->length > 0) {
				STNBString tmpPath;
				NBString_initWithStringAndStr(&tmpPath, strRoot, folderpath);
				r = (*pItf->folderExists)(pItf->obj, &parents[1], (parentsSz - 1), ENNBFilesystemRoot_WorkDir, tmpPath.str);
				NBString_release(&tmpPath);
			} else {
				r = (*pItf->folderExists)(pItf->obj, &parents[1], (parentsSz - 1), root, folderpath);
			}
		}
	}
	return r;
}

BOOL NBFilesystemWin_createFolder_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath){
	BOOL r = FALSE;
	if (parents != NULL && parentsSz > 0) {
		STNBFilesystemWinOpq* opq = (STNBFilesystemWinOpq*)obj;
		const IFilesystemItf* pItf = &parents[0];
		if (pItf->createFolder != NULL) {
			const STNBString* strRoot = &opq->roots[root];
			if (strRoot->length > 0) {
				STNBString tmpPath;
				NBString_initWithStringAndStr(&tmpPath, strRoot, folderpath);
				r = (*pItf->createFolder)(pItf->obj, &parents[1], (parentsSz - 1), ENNBFilesystemRoot_WorkDir, tmpPath.str);
				NBString_release(&tmpPath);
			} else {
				r = (*pItf->createFolder)(pItf->obj, &parents[1], (parentsSz - 1), root, folderpath);
			}
		}
	}
	return r;
}

BOOL NBFilesystemWin_deleteFolder_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath){
	BOOL r = FALSE;
	if (parents != NULL && parentsSz > 0) {
		STNBFilesystemWinOpq* opq = (STNBFilesystemWinOpq*)obj;
		const IFilesystemItf* pItf = &parents[0];
		if (pItf->deleteFolder != NULL) {
			const STNBString* strRoot = &opq->roots[root];
			if (strRoot->length > 0) {
				STNBString tmpPath;
				NBString_initWithStringAndStr(&tmpPath, strRoot, folderpath);
				r = (*pItf->deleteFolder)(pItf->obj, &parents[1], (parentsSz - 1), ENNBFilesystemRoot_WorkDir, tmpPath.str);
				NBString_release(&tmpPath);
			} else {
				r = (*pItf->deleteFolder)(pItf->obj, &parents[1], (parentsSz - 1), root, folderpath);
			}
		}
	}
	return r;
}


//

BOOL NBFilesystemWin_open_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* filepath, const ENNBFileMode mode, STNBFileRef dst){
	BOOL r = FALSE;
	if (parents != NULL && parentsSz > 0) {
		STNBFilesystemWinOpq* opq = (STNBFilesystemWinOpq*)obj;
		const IFilesystemItf* pItf = &parents[0];
		if (pItf->open != NULL) {
			//Search with parent
			const STNBString* strRoot = &opq->roots[root];
			if (strRoot->length > 0) {
				STNBString tmpPath;
				NBString_initWithStringAndStr(&tmpPath, strRoot, filepath);
				r = (*pItf->open)(pItf->obj, &parents[1], (parentsSz - 1), ENNBFilesystemRoot_WorkDir, tmpPath.str, mode, dst);
				NBString_release(&tmpPath);
			} else {
				r = (*pItf->open)(pItf->obj, &parents[1], (parentsSz - 1), root, filepath, mode, dst);
			}
		}
	}
	return r;
}

BOOL NBFilesystemWin_getFiles_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath, const BOOL includeStats, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/){
	BOOL r = FALSE;
	if (parents != NULL && parentsSz > 0) {
		STNBFilesystemWinOpq* opq = (STNBFilesystemWinOpq*)obj;
		const IFilesystemItf* pItf = &parents[0];
		if (pItf->getFiles != NULL) {
			const STNBString* strRoot = &opq->roots[root];
			if (strRoot->length > 0) {
				STNBString tmpPath;
				NBString_initWithStringAndStr(&tmpPath, strRoot, folderpath);
				r = (*pItf->getFiles)(pItf->obj, &parents[1], (parentsSz - 1), ENNBFilesystemRoot_WorkDir, tmpPath.str, includeStats, dstStrs, dstFiles);
				NBString_release(&tmpPath);
			} else {
				r = (*pItf->getFiles)(pItf->obj, &parents[1], (parentsSz - 1), root, folderpath, includeStats, dstStrs, dstFiles);
			}
		}
	}
	return r;
}

BOOL NBFilesystemWin_moveFile_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot srcRoot, const char* srcFilepath, const ENNBFilesystemRoot dstRoot, const char* dstFilepath){
	BOOL r = FALSE;
	if (parents != NULL && parentsSz > 0) {
		STNBFilesystemWinOpq* opq = (STNBFilesystemWinOpq*)obj;
		const IFilesystemItf* pItf = &parents[0];
		if (pItf->moveFile != NULL) {
			const STNBString* strRoot1 = &opq->roots[srcRoot];
			const STNBString* strRoot2 = &opq->roots[dstRoot];
			if (strRoot1->length > 0 || strRoot2->length > 0) {
				STNBString tmpPath, tmpPath2;
				NBString_initWithStringAndStr(&tmpPath, strRoot1, srcFilepath);
				NBString_initWithStringAndStr(&tmpPath2, strRoot2, dstFilepath);
				r = (*pItf->moveFile)(pItf->obj, &parents[1], (parentsSz - 1), (strRoot1->length > 0 ? ENNBFilesystemRoot_WorkDir : srcRoot), tmpPath.str, (strRoot2->length > 0 ? ENNBFilesystemRoot_WorkDir : dstRoot), tmpPath2.str);
				NBString_release(&tmpPath);
				NBString_release(&tmpPath2);
			} else {
				r = (*pItf->moveFile)(pItf->obj, &parents[1], (parentsSz - 1), srcRoot, srcFilepath, dstRoot, dstFilepath);
			}
		}
	}
	return r;
}

BOOL NBFilesystemWin_deleteFile_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* filepath){
	BOOL r = FALSE;
	if (parents != NULL && parentsSz > 0) {
		STNBFilesystemWinOpq* opq = (STNBFilesystemWinOpq*)obj;
		const IFilesystemItf* pItf = &parents[0];
		if (pItf->deleteFile != NULL) {
			const STNBString* strRoot = &opq->roots[root];
			if (strRoot->length > 0) {
				STNBString tmpPath;
				NBString_initWithStringAndStr(&tmpPath, strRoot, filepath);
				r = (*pItf->deleteFile)(pItf->obj, &parents[1], (parentsSz - 1), ENNBFilesystemRoot_WorkDir, tmpPath.str);
				NBString_release(&tmpPath);
			} else {
				r = (*pItf->deleteFile)(pItf->obj, &parents[1], (parentsSz - 1), root, filepath);
			}
		}
	}
	return r;
}

