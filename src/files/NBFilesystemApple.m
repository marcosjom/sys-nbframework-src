//
//  NBFilesystemApple.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/25/18.
//

#include "nb/NBFrameworkPch.h"
#ifdef __OBJC__
#	import <Foundation/Foundation.h>
#endif
#include "nb/files/NBFilesystemApple.h"
//
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
#	include "nb/core/NBMngrProcess.h"
#endif

typedef struct STNBFilesystemAppleOpq_ {
	STNBString		roots[ENNBFilesystemRoot_Count];
	UI32			retainCount;
} STNBFilesystemAppleOpq;

//Filesystem itf
void NBFilesystemApple_retain_(void* obj);
void NBFilesystemApple_release_(void** obj);
//
BOOL NBFilesystemApple_concatRootPath_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* fileName, STNBString* dst);
//
BOOL NBFilesystemApple_getFolders_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
BOOL NBFilesystemApple_folderExists_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath);
BOOL NBFilesystemApple_createFolder_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath);
BOOL NBFilesystemApple_deleteFolder_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath);
//
BOOL NBFilesystemApple_open_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* filepath, const ENNBFileMode mode, STNBFileRef dst);
BOOL NBFilesystemApple_getFiles_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath, const BOOL includeStats, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
BOOL NBFilesystemApple_moveFile_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot srcRoot, const char* srcFilepath, const ENNBFilesystemRoot dstRoot, const char* dstFilepath);
BOOL NBFilesystemApple_deleteFile_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* filepath);

void NBFilesystemApple_init(STNBFilesystemApple* obj){
	STNBFilesystemAppleOpq* opq = obj->opaque = NBMemory_allocType(STNBFilesystemAppleOpq);
	NBMemory_setZeroSt(*opq, STNBFilesystemAppleOpq);
	{
		NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
		NSBundle* bundle = [NSBundle mainBundle];
		UI32 i; for(i = 0; i < ENNBFilesystemRoot_Count; i++){
			STNBString* str = &opq->roots[i];
			switch(i) {
				case ENNBFilesystemRoot_AppBundle:
					NBString_initWithStr(str, [[bundle resourcePath] UTF8String]);
					PRINTF_INFO("NBFilesystemApple, AppBundle: '%s'\n", str->str);
					break;
				case ENNBFilesystemRoot_Docs:
					NBString_initWithStr(str, [[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0] UTF8String]);
					PRINTF_INFO("NBFilesystemApple, Docs: '%s'\n", str->str);
					break;
				case ENNBFilesystemRoot_Lib:
					NBString_initWithStr(str, [[NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES) objectAtIndex:0] UTF8String]);
					PRINTF_INFO("NBFilesystemApple, Lib: '%s'\n", str->str);
					break;
				case ENNBFilesystemRoot_Cache:
					NBString_initWithStr(str, [[NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) objectAtIndex:0] UTF8String]);
					PRINTF_INFO("NBFilesystemApple, Cache: '%s'\n", str->str);
					break;
				default:
					NBString_init(str);
					break;
			}
			if(str->length > 0){
				if(str->str[str->length - 1] != '/'){
					NBString_concatByte(str, '/');
				}
			}
		}
		[pool release];
	}
	opq->retainCount = 1;
}

void NBFilesystemApple_retain(STNBFilesystemApple* obj){
	NBFilesystemApple_retain_(obj->opaque);
}

void NBFilesystemApple_release(STNBFilesystemApple* obj){
	NBFilesystemApple_release_(&obj->opaque);
}

//

void NBFilesystemApple_createItf(void* pObj, IFilesystemItf* dst){
	STNBFilesystemApple* obj = (STNBFilesystemApple*)pObj;
	STNBFilesystemAppleOpq* opq = (STNBFilesystemAppleOpq*)obj->opaque;
	//Set iterface
	NBMemory_set(dst, 0, sizeof(*dst));
	dst->obj			= opq;
	dst->retain			= NBFilesystemApple_retain_;
	dst->release		= NBFilesystemApple_release_;
	//
	dst->concatRootPath	= NBFilesystemApple_concatRootPath_;
	//
	dst->getFolders		= NBFilesystemApple_getFolders_;
	dst->folderExists	= NBFilesystemApple_folderExists_;
	dst->createFolder	= NBFilesystemApple_createFolder_;
	dst->deleteFolder	= NBFilesystemApple_deleteFolder_;
	//
	dst->open			= NBFilesystemApple_open_;
	dst->getFiles		= NBFilesystemApple_getFiles_;
	dst->moveFile		= NBFilesystemApple_moveFile_;
	dst->deleteFile		= NBFilesystemApple_deleteFile_;
}

//Filesystem itf

void NBFilesystemApple_retain_(void* obj){
	STNBFilesystemAppleOpq* opq = (STNBFilesystemAppleOpq*)obj;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount++;
}

void NBFilesystemApple_release_(void** obj){
	STNBFilesystemAppleOpq* opq = (STNBFilesystemAppleOpq*)*obj;
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
		NBMemory_free(opq);
		*obj = NULL;
	}
}

//

BOOL NBFilesystemApple_concatRootPath_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* fileName, STNBString* dst){
	BOOL r = FALSE;
	STNBFilesystemAppleOpq* opq = (STNBFilesystemAppleOpq*)obj;
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

BOOL NBFilesystemApple_getFolders_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/){
	BOOL r = FALSE;
	if(parents != NULL && parentsSz > 0){
		STNBFilesystemAppleOpq* opq = (STNBFilesystemAppleOpq*)obj;
		const IFilesystemItf* pItf = &parents[0];
		if(pItf->getFolders != NULL){
			const STNBString* strRoot = &opq->roots[root];
			if(strRoot->length > 0){
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

BOOL NBFilesystemApple_folderExists_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath){
	BOOL r = FALSE;
	if(parents != NULL && parentsSz > 0){
		STNBFilesystemAppleOpq* opq = (STNBFilesystemAppleOpq*)obj;
		const IFilesystemItf* pItf = &parents[0];
		if(pItf->folderExists != NULL){
			const STNBString* strRoot = &opq->roots[root];
			if(strRoot->length > 0){
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

BOOL NBFilesystemApple_createFolder_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath){
	BOOL r = FALSE;
	if(parents != NULL && parentsSz > 0){
		STNBFilesystemAppleOpq* opq = (STNBFilesystemAppleOpq*)obj;
		const IFilesystemItf* pItf = &parents[0];
		if(pItf->createFolder != NULL){
			const STNBString* strRoot = &opq->roots[root];
			if(strRoot->length > 0){
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

BOOL NBFilesystemApple_deleteFolder_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath){
	BOOL r = FALSE;
	if(parents != NULL && parentsSz > 0){
		STNBFilesystemAppleOpq* opq = (STNBFilesystemAppleOpq*)obj;
		const IFilesystemItf* pItf = &parents[0];
		if(pItf->deleteFolder != NULL){
			const STNBString* strRoot = &opq->roots[root];
			if(strRoot->length > 0){
				STNBString tmpPath;
				NBString_initWithStringAndStr(&tmpPath, strRoot, folderpath);
				{
					NSError* error = nil;
					if([[NSFileManager defaultManager] removeItemAtPath:[NSString stringWithUTF8String: tmpPath.str] error: &error]){
#						ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
						NBMngrProcess_fsFolderDeleted(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#						endif
						r = TRUE;
					} else {
#						ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
						NBMngrProcess_fsFolderDeleted(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Error);
#						endif
						r = (*pItf->deleteFolder)(pItf->obj, &parents[1], (parentsSz - 1), ENNBFilesystemRoot_WorkDir, tmpPath.str);
					}
				}
				NBString_release(&tmpPath);
			} else {
				NSError* error = nil;
				if([[NSFileManager defaultManager] removeItemAtPath:[NSString stringWithUTF8String: folderpath] error: &error]){
#					ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
					NBMngrProcess_fsFolderDeleted(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#					endif
					r = TRUE;
				} else {
#					ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
					NBMngrProcess_fsFolderDeleted(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Error);
#					endif
					r = (*pItf->deleteFolder)(pItf->obj, &parents[1], (parentsSz - 1), root, folderpath);
				}
			}
		}
	}
	return r;
}

//

BOOL NBFilesystemApple_open_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* filepath, const ENNBFileMode mode, STNBFileRef dst){
	BOOL r = FALSE;
	if(parents != NULL && parentsSz > 0){
		STNBFilesystemAppleOpq* opq = (STNBFilesystemAppleOpq*)obj;
		const IFilesystemItf* pItf = &parents[0];
		if(pItf->open != NULL){
			const STNBString* strRoot = &opq->roots[root];
			if(strRoot->length > 0){
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

BOOL NBFilesystemApple_getFiles_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath, const BOOL includeStats, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/){
	BOOL r = FALSE;
	if(parents != NULL && parentsSz > 0){
		STNBFilesystemAppleOpq* opq = (STNBFilesystemAppleOpq*)obj;
		const IFilesystemItf* pItf = &parents[0];
		if(pItf->getFiles != NULL){
			const STNBString* strRoot = &opq->roots[root];
			if(strRoot->length > 0){
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

BOOL NBFilesystemApple_moveFile_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot srcRoot, const char* srcFilepath, const ENNBFilesystemRoot dstRoot, const char* dstFilepath){
	BOOL r = FALSE;
	if(parents != NULL && parentsSz > 0){
		STNBFilesystemAppleOpq* opq = (STNBFilesystemAppleOpq*)obj;
		const IFilesystemItf* pItf = &parents[0];
		if(pItf->moveFile != NULL){
			const STNBString* strRoot1 = &opq->roots[srcRoot];
			const STNBString* strRoot2 = &opq->roots[dstRoot];
			if(strRoot1->length > 0 || strRoot2->length > 0){
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

BOOL NBFilesystemApple_deleteFile_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* filepath){
	BOOL r = FALSE;
	if(parents != NULL && parentsSz > 0){
		STNBFilesystemAppleOpq* opq = (STNBFilesystemAppleOpq*)obj;
		const IFilesystemItf* pItf = &parents[0];
		if(pItf->deleteFile != NULL){
			const STNBString* strRoot = &opq->roots[root];
			if(strRoot->length > 0){
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
