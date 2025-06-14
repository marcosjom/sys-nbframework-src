//
//  NBFilesystem.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/8/18.
//

#include "nb/NBFrameworkPch.h"
//Complementary PCH
#ifdef _WIN32
//#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>		//for HANDLE and more
#else
#	include <sys/stat.h>	//for directories and simlinks
#	include <dirent.h>		//for directories
#	include <unistd.h>		//for rmdir
#endif
#include <stdio.h>			//for rename(), remove()
//
#include "nb/core/NBMemory.h"
#include "nb/files/NBFilesystem.h"

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
#	include "nb/core/NBMngrProcess.h"
#endif

typedef struct STNBFilesystemOpq_ {
	IFilesystemItf*	itfs;		//iterfaces FILO stack
	UI32			itfsSz;		//iterfaces FILO stack
	UI32			retainCount;
} STNBFilesystemOpq;

//Native itf

void NBFilesystem_retain_(void* obj);
void NBFilesystem_release_(void** obj);

BOOL NBFilesystem_concatRootPath_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* fileName, STNBString* dst);

BOOL NBFilesystem_getFolders_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
BOOL NBFilesystem_folderExists_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath);
BOOL NBFilesystem_createFolder_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath);
BOOL NBFilesystem_deleteFolder_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath);
//
BOOL NBFilesystem_open_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* filepath, const ENNBFileMode mode, STNBFileRef dst);
BOOL NBFilesystem_getFiles_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath, const BOOL includeStats, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
BOOL NBFilesystem_moveFile_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot srcRoot, const char* srcFilepath, const ENNBFilesystemRoot dstRoot, const char* dstFilepath);
BOOL NBFilesystem_deleteFile_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* filepath);

void NBFilesystem_populateNativeItf(IFilesystemItf* dstItf){
	NBMemory_set(dstItf, 0, sizeof(*dstItf));
	dstItf->retain			= NBFilesystem_retain_;
	dstItf->release			= NBFilesystem_release_;
	//
	dstItf->concatRootPath	= NBFilesystem_concatRootPath_;
	//
	dstItf->getFolders		= NBFilesystem_getFolders_;
	dstItf->folderExists	= NBFilesystem_folderExists_;
	dstItf->createFolder	= NBFilesystem_createFolder_;
	dstItf->deleteFolder	= NBFilesystem_deleteFolder_;
	//
	dstItf->open			= NBFilesystem_open_;
	dstItf->getFiles		= NBFilesystem_getFiles_;
	dstItf->moveFile		= NBFilesystem_moveFile_;
	dstItf->deleteFile		= NBFilesystem_deleteFile_;
}

//

void NBFilesystem_init(STNBFilesystem* obj){
	STNBFilesystemOpq* opq = obj->opaque = NBMemory_allocType(STNBFilesystemOpq);
	NBMemory_setZeroSt(*opq, STNBFilesystemOpq);
	//init stack
	{
		opq->itfs		= NBMemory_allocType(IFilesystemItf);
		opq->itfsSz		= 1;
		//first
		{
			IFilesystemItf* itf = &opq->itfs[0];
			NBFilesystem_populateNativeItf(itf);
			itf->obj	= opq;
		}
	}
	opq->retainCount	= 1;
}

void NBFilesystem_retain(STNBFilesystem* obj){
	NBFilesystem_retain_(obj->opaque);
}
	
void NBFilesystem_release(STNBFilesystem* obj){
	NBFilesystem_release_(&obj->opaque);
}

BOOL NBFilesystem_pushItf(STNBFilesystem* obj, IFilesystemItfCreateFunc createFunc, void* createObj){
	BOOL r = FALSE;
	if(createFunc != NULL){
		STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
		IFilesystemItf itf;
		NBMemory_setZeroSt(itf, IFilesystemItf);
		(*createFunc)(createObj, &itf);
		//push as first
		{
			IFilesystemItf* arr = NBMemory_allocTypes(IFilesystemItf, (size_t)opq->itfsSz + 1);
			NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
			if(opq->itfs != NULL){
				if(opq->itfsSz > 0){
					NBMemory_copy(&arr[1], &opq->itfs[0], sizeof(opq->itfs[0]) * opq->itfsSz);
				}
				NBMemory_free(opq->itfs);
				opq->itfs = NULL;
			}
			arr[0]		= itf;
			opq->itfs	= arr;
			opq->itfsSz++;
		}
		//retain
		if(itf.retain != NULL){
			(*itf.retain)(itf.obj);
		}
		r = TRUE;
	}
	return r;
}

//Folders

BOOL NBFilesystem_getFolders(const STNBFilesystem* obj, const char* folderpath, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];  
	if(topItf->getFolders != NULL){
		const BOOL pkgs = (*topItf->getFolders)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_PkgsDir, folderpath, dstStrs, dstFiles);
		const BOOL wokr = (*topItf->getFolders)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_WorkDir, folderpath, dstStrs, dstFiles);
		r = (pkgs || wokr);
	}
	return r;
}

BOOL NBFilesystem_getFoldersStrs(const STNBFilesystem* obj, const char** arr, const UI32 arrSz, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->getFolders != NULL){
		STNBString filepath;
		NBString_initWithStrs(&filepath, arr, arrSz);
		{
			const BOOL pkgs = (*topItf->getFolders)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_PkgsDir, filepath.str, dstStrs, dstFiles);
			const BOOL wokr = (*topItf->getFolders)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_WorkDir, filepath.str, dstStrs, dstFiles);
			r = (pkgs || wokr);
		}
		NBString_release(&filepath);
	}
	return r;
}

BOOL NBFilesystem_getFoldersStrsAndNull(const STNBFilesystem* obj, const char** arrAndNull, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->getFolders != NULL){
		STNBString filepath;
		NBString_initWithStrsAndNull(&filepath, arrAndNull);
		{
			const BOOL pkgs = (*topItf->getFolders)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_PkgsDir, filepath.str, dstStrs, dstFiles);
			const BOOL wokr = (*topItf->getFolders)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_WorkDir, filepath.str, dstStrs, dstFiles);
			r = (pkgs || wokr);
		}
		NBString_release(&filepath);
	}
	return r;
}

BOOL NBFilesystem_getFoldersAtRoot(const STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* folderpath, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->getFolders != NULL){
		r = (*topItf->getFolders)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), root, folderpath, dstStrs, dstFiles);
	}
	return r;
}

BOOL NBFilesystem_getFoldersStrsAtRoot(const STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arr, const UI32 arrSz, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->getFolders != NULL){
		STNBString filepath;
		NBString_initWithStrs(&filepath, arr, arrSz);
		r = (*topItf->getFolders)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), root, filepath.str, dstStrs, dstFiles);
		NBString_release(&filepath);
	}
	return r;
}

BOOL NBFilesystem_getFoldersStrsAndNullAtRoot(const STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arrAndNull, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->getFolders != NULL){
		STNBString filepath;
		NBString_initWithStrsAndNull(&filepath, arrAndNull);
		r = (*topItf->getFolders)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), root, filepath.str, dstStrs, dstFiles);
		NBString_release(&filepath);
	}
	return r;
}

//

BOOL NBFilesystem_folderExists(const STNBFilesystem* obj, const char* folderpath){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->folderExists != NULL){
		r = (*topItf->folderExists)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_PkgsDir, folderpath);
		if(!r){
			r = (*topItf->folderExists)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_WorkDir, folderpath);
		}
	}
	return r;
}

BOOL NBFilesystem_folderExistsStrs(const STNBFilesystem* obj, const char** arr, const UI32 arrSz){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->folderExists != NULL){
		STNBString filepath;
		NBString_initWithStrs(&filepath, arr, arrSz);
		r = (*topItf->folderExists)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_PkgsDir, filepath.str);
		if(!r){
			r = (*topItf->folderExists)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_WorkDir, filepath.str);
		}
		NBString_release(&filepath);
	}
	return r;
}

BOOL NBFilesystem_folderExistsStrsAndNull(const STNBFilesystem* obj, const char** arrAndNull){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->folderExists != NULL){
		STNBString filepath;
		NBString_initWithStrsAndNull(&filepath, arrAndNull);
		r = (*topItf->folderExists)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_PkgsDir, filepath.str);
		if(!r){
			r = (*topItf->folderExists)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_WorkDir, filepath.str);
		}
		NBString_release(&filepath);
	}
	return r;
}

BOOL NBFilesystem_folderExistsAtRoot(const STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* folderpath){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->folderExists != NULL){
		r = (*topItf->folderExists)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), root, folderpath);
	}
	return r;
}

BOOL NBFilesystem_folderExistsStrsAtRoot(const STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arr, const UI32 arrSz){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->folderExists != NULL){
		STNBString filepath;
		NBString_initWithStrs(&filepath, arr, arrSz);
		r = (*topItf->folderExists)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), root, filepath.str);
		NBString_release(&filepath);
	}
	return r;
}

BOOL NBFilesystem_folderExistsStrsAndNullAtRoot(const STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arrAndNull){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->folderExists != NULL){
		STNBString filepath;
		NBString_initWithStrsAndNull(&filepath, arrAndNull);
		r = (*topItf->folderExists)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), root, filepath.str);
		NBString_release(&filepath);
	}
	return r;
}

//

BOOL NBFilesystem_createFolder(STNBFilesystem* obj, const char* folderpath){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->createFolder != NULL){
		r = (*topItf->folderExists)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_PkgsDir, folderpath);
		if(!r){
			r = (*topItf->createFolder)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_WorkDir, folderpath);
		}
	}
	return r;
}

BOOL NBFilesystem_createFolderStrs(STNBFilesystem* obj, const char** arr, const UI32 arrSz){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->createFolder != NULL){
		STNBString filepath;
		NBString_initWithStrs(&filepath, arr, arrSz);
		r = (*topItf->folderExists)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_PkgsDir, filepath.str);
		if(!r){
			r = (*topItf->createFolder)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_WorkDir, filepath.str);
		}
		NBString_release(&filepath);
	}
	return r;
}

BOOL NBFilesystem_createFolderStrsAndNull(STNBFilesystem* obj, const char** arrAndNull){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->createFolder != NULL){
		STNBString filepath;
		NBString_initWithStrsAndNull(&filepath, arrAndNull);
		r = (*topItf->folderExists)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_PkgsDir, filepath.str);
		if(!r){
			r = (*topItf->createFolder)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_WorkDir, filepath.str);
		}
		NBString_release(&filepath);
	}
	return r;
}

BOOL NBFilesystem_createFolderAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* folderpath){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->createFolder != NULL){
		r = (*topItf->createFolder)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), root, folderpath);
	}
	return r;
}

BOOL NBFilesystem_createFolderStrsAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arr, const UI32 arrSz){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->createFolder != NULL){
		STNBString filepath;
		NBString_initWithStrs(&filepath, arr, arrSz);
		r = (*topItf->createFolder)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), root, filepath.str);
		NBString_release(&filepath);
	}
	return r;
}

BOOL NBFilesystem_createFolderStrsAndNullAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arrAndNull){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->createFolder != NULL){
		STNBString filepath;
		NBString_initWithStrsAndNull(&filepath, arrAndNull);
		r = (*topItf->createFolder)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), root, filepath.str);
		NBString_release(&filepath);
	}
	return r;
}

//

BOOL NBFilesystem_deleteFolder(STNBFilesystem* obj, const char* folderpath){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->deleteFolder != NULL){
		r = (*topItf->deleteFolder)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_WorkDir, folderpath);
	}
	return r;
}

BOOL NBFilesystem_deleteFolderStrs(STNBFilesystem* obj, const char** arr, const UI32 arrSz){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->deleteFolder != NULL){
		STNBString filepath;
		NBString_initWithStrs(&filepath, arr, arrSz);
		r = (*topItf->deleteFolder)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_WorkDir, filepath.str);
		NBString_release(&filepath);
	}
	return r;
}

BOOL NBFilesystem_deleteFolderStrsAndNull(STNBFilesystem* obj, const char** arrAndNull){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->deleteFolder != NULL){
		STNBString filepath;
		NBString_initWithStrsAndNull(&filepath, arrAndNull);
		r = (*topItf->deleteFolder)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_WorkDir, filepath.str);
		NBString_release(&filepath);
	}
	return r;
}

BOOL NBFilesystem_deleteFolderAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* folderpath){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->deleteFolder != NULL){
		r = (*topItf->deleteFolder)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), root, folderpath);
	}
	return r;
}

BOOL NBFilesystem_deleteFolderStrsAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arr, const UI32 arrSz){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->deleteFolder != NULL){
		STNBString filepath;
		NBString_initWithStrs(&filepath, arr, arrSz);
		r = (*topItf->deleteFolder)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), root, filepath.str);
		NBString_release(&filepath);
	}
	return r;
}

BOOL NBFilesystem_deleteFolderStrsAndNullAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arrAndNull){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->deleteFolder != NULL){
		STNBString filepath;
		NBString_initWithStrsAndNull(&filepath, arrAndNull);
		r = (*topItf->deleteFolder)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), root, filepath.str);
		NBString_release(&filepath);
	}
	return r;
}

//

BOOL NBFilesystem_createFolderPath(STNBFilesystem* obj, const char* folderpath){
	BOOL r = TRUE;
	SI32 iChar = 0; char c;
	STNBString path;
	NBString_init(&path);
	while(TRUE){
		c = folderpath[iChar++];
		if(c == '/' || c == '\\' || c == '\0'){
			//PRINTF_INFO("Creating folder: '%s'.\n", path.str);
			NBFilesystem_createFolder(obj, path.str);
		}
		if(c == '\0') break;
		NBString_concatByte(&path, c);
	}
	NBString_release(&path);
	return r;
}

BOOL NBFilesystem_createFolderPathStrs(STNBFilesystem* obj, const char** arr, const UI32 arrSz){
	BOOL r = TRUE;
	STNBString path;
	NBString_init(&path);
	{
		SI32 iStr = 0, iChar; char c;
		while(iStr < arrSz){
			const char* str = arr[iStr++];
			iChar = 0;
			while(TRUE){
				c = str[iChar++];
				if(c == '/' || c == '\\' || (c == '\0' && iStr == arrSz)){
					//PRINTF_INFO("Creating folder: '%s'.\n", path.str);
					NBFilesystem_createFolder(obj, path.str);
				}
				if(c == '\0') break;
				NBString_concatByte(&path, c);
			}
		}
	}
	NBString_release(&path);
	return r;
}

BOOL NBFilesystem_createFolderPathStrsAndNull(STNBFilesystem* obj, const char** arrAndNull){
	BOOL r = TRUE;
	STNBString path;
	NBString_init(&path);
	{
		SI32 iStr = 0, iChar; char c;
		while(TRUE){
			const char* str = arrAndNull[iStr++];
			iChar = 0;
			if(str == NULL){
				//PRINTF_INFO("Creating folder: '%s'.\n", path.str);
				NBFilesystem_createFolder(obj, path.str);
				break;
			}
			while(TRUE){
				c = str[iChar++];
				if(c == '/' || c == '\\'){
					//PRINTF_INFO("Creating folder: '%s'.\n", path.str);
					NBFilesystem_createFolder(obj, path.str);
				}
				if(c == '\0') break;
				NBString_concatByte(&path, c);
			}
		}
	}
	NBString_release(&path);
	return r;
}

BOOL NBFilesystem_createFolderPathAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* folderpath){
	BOOL r = TRUE;
	SI32 iChar = 0; char c;
	STNBString path;
	NBString_init(&path);
	while(TRUE){
		c = folderpath[iChar++];
		if(c == '/' || c == '\\' || c == '\0'){
			//PRINTF_INFO("Creating folder: '%s'.\n", path.str);
			NBFilesystem_createFolderAtRoot(obj, root, path.str);
		}
		if(c == '\0') break;
		NBString_concatByte(&path, c);
	}
	NBString_release(&path);
	return r;
}

BOOL NBFilesystem_createFolderPathStrsAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arr, const UI32 arrSz){
	BOOL r = TRUE;
	STNBString path;
	NBString_init(&path);
	{
		SI32 iStr = 0, iChar; char c;
		while(iStr < arrSz){
			const char* str = arr[iStr++];
			iChar = 0;
			while(TRUE){
				c = str[iChar++];
				if(c == '/' || c == '\\' || (c == '\0' && iStr == arrSz)){
					//PRINTF_INFO("Creating folder: '%s'.\n", path.str);
					NBFilesystem_createFolderAtRoot(obj, root, path.str);
				}
				if(c == '\0') break;
				NBString_concatByte(&path, c);
			}
		}
	}
	NBString_release(&path);
	return r;
}

BOOL NBFilesystem_createFolderPathStrsAndNullAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arrAndNull){
	BOOL r = TRUE;
	STNBString path;
	NBString_init(&path);
	{
		SI32 iStr = 0, iChar; char c;
		while(TRUE){
			const char* str = arrAndNull[iStr++];
			iChar = 0;
			if(str == NULL){
				//PRINTF_INFO("Creating folder: '%s'.\n", path.str);
				NBFilesystem_createFolderAtRoot(obj, root, path.str);
				break;
			}
			while(TRUE){
				c = str[iChar++];
				if(c == '/' || c == '\\'){
					//PRINTF_INFO("Creating folder: '%s'.\n", path.str);
					NBFilesystem_createFolderAtRoot(obj, root, path.str);
				}
				if(c == '\0') break;
				NBString_concatByte(&path, c);
			}
		}
	}
	NBString_release(&path);
	return r;
}

//Last component (filename is ignored)

BOOL NBFilesystem_createFolderPathOfFile(STNBFilesystem* obj, const char* folderpath){
	BOOL r = TRUE;
	SI32 iChar = 0; char c;
	STNBString path;
	NBString_init(&path);
	while(TRUE){
		c = folderpath[iChar++];
		if(c == '/' || c == '\\'){ //Ignoring last component
			//PRINTF_INFO("Creating folder: '%s'.\n", path.str);
			NBFilesystem_createFolder(obj, path.str);
		}
		if(c == '\0') break;
		NBString_concatByte(&path, c);
	}
	NBString_release(&path);
	return r;
}

BOOL NBFilesystem_createFolderPathOfFileAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* folderpath){
	BOOL r = TRUE;
	SI32 iChar = 0; char c;
	STNBString path;
	NBString_init(&path);
	while(TRUE){
		c = folderpath[iChar++];
		if(c == '/' || c == '\\'){ //Ignoring last component
			//PRINTF_INFO("Creating folder: '%s'.\n", path.str);
			NBFilesystem_createFolderAtRoot(obj, root, path.str);
		}
		if(c == '\0') break;
		NBString_concatByte(&path, c);
	}
	NBString_release(&path);
	return r;
}

//

BOOL NBFilesystem_concatRootPath(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* fileName, STNBString* dst){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->concatRootPath != NULL){
		r = (*topItf->concatRootPath)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), root, fileName, dst);
	}
	return r;
}

//Files

BOOL NBFilesystem_open(STNBFilesystem* obj, const char* filepath, const ENNBFileMode mode, STNBFileRef dst){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->open != NULL){
		r = (*topItf->open)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_PkgsDir, filepath, mode, dst);
		if(!r){
			r = (*topItf->open)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_WorkDir, filepath, mode, dst);
		}
		IF_NBASSERT(if(r && NBFile_isSet(dst)){ NBFile_dbgSetPathRef(dst, filepath); })
	}
	return r;
}

BOOL NBFilesystem_openStrs(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arr, const UI32 arrSz, const ENNBFileMode mode, STNBFileRef dst){
	BOOL r = FALSE;
	STNBString filepath;
	NBString_initWithStrs(&filepath, arr, arrSz);
	r = NBFilesystem_open(obj, filepath.str, mode, dst);
	NBString_release(&filepath);
	return r;
}

BOOL NBFilesystem_openStrsAndNull(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arrAndNull, const ENNBFileMode mode, STNBFileRef dst){
	BOOL r = FALSE;
	STNBString filepath;
	NBString_initWithStrsAndNull(&filepath, arrAndNull);
	r = NBFilesystem_open(obj, filepath.str, mode, dst);
	NBString_release(&filepath);
	return r;
}

BOOL NBFilesystem_openAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* filepath, const ENNBFileMode mode, STNBFileRef dst){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->open != NULL){
		r = (*topItf->open)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), root, filepath, mode, dst);
		IF_NBASSERT(if(r && NBFile_isSet(dst)){ NBFile_dbgSetPathRef(dst, filepath); })
	}
	return r;
}

BOOL NBFilesystem_openStrsAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arr, const UI32 arrSz, const ENNBFileMode mode, STNBFileRef dst){
	BOOL r = FALSE;
	STNBString filepath;
	NBString_initWithStrs(&filepath, arr, arrSz);
	r = NBFilesystem_openAtRoot(obj, root, filepath.str, mode, dst);
	NBString_release(&filepath);
	return r;
}

BOOL NBFilesystem_openStrsAndNullAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arrAndNull, const ENNBFileMode mode, STNBFileRef dst){
	BOOL r = FALSE;
	STNBString filepath;
	NBString_initWithStrsAndNull(&filepath, arrAndNull);
	r = NBFilesystem_openAtRoot(obj, root, filepath.str, mode, dst);
	NBString_release(&filepath);
	return r;
}


//get

BOOL NBFilesystem_getFiles(const STNBFilesystem* obj, const char* folderpath, const BOOL includeStats, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->getFiles != NULL){
		const BOOL pkgs = (*topItf->getFiles)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_PkgsDir, folderpath, includeStats, dstStrs, dstFiles);
		const BOOL wokr = (*topItf->getFiles)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_WorkDir, folderpath, includeStats, dstStrs, dstFiles);
		r = (pkgs || wokr);
	}
	return r;
}

BOOL NBFilesystem_moveFile(STNBFilesystem* obj, const char* srcFilepath, const char* dstFilepath){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->moveFile != NULL){
		r = (*topItf->moveFile)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_WorkDir, srcFilepath, ENNBFilesystemRoot_WorkDir, dstFilepath);
	}
	return r;
}

BOOL NBFilesystem_deleteFile(STNBFilesystem* obj, const char* filepath){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->deleteFile != NULL){
		r = (*topItf->deleteFile)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_PkgsDir, filepath);
		if(!r){
			r = (*topItf->deleteFile)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), ENNBFilesystemRoot_WorkDir, filepath);
		}
	}
	return r;
}

STNBFilesystemResult NBFilesystem_deleteFilesAtFolder(STNBFilesystem* obj, const char* folderPath, const BOOL recursive){
	STNBFilesystemResult r;
	NBMemory_setZeroSt(r, STNBFilesystemResult);
	{
		STNBString strs, strPath;
		STNBArray arr;
		NBString_init(&strs);
		NBString_init(&strPath);
		NBArray_init(&arr, sizeof(STNBFilesystemFile), NULL);
		//Delete files
		{
			NBArray_empty(&arr);
			NBString_empty(&strs);
			if(!NBFilesystem_getFiles(obj, folderPath, FALSE, &strs, &arr)){
				r.foldersCount.error++;
			} else {
				SI32 i; for(i =0; i < arr.use; i++){
					const STNBFilesystemFile* ff = NBArray_itmPtrAtIndex(&arr, STNBFilesystemFile, i);
					const char* ffName = &strs.str[ff->name];
					//Build filepath
					{
						NBString_set(&strPath, folderPath);
						if(strPath.length > 0){
							if(strPath.str[strPath.length - 1] != '/' && strPath.str[strPath.length - 1] != '\\'){
								NBString_concatByte(&strPath, '/');
							}
						}
						NBString_concat(&strPath, ffName);
					}
					if(!NBFilesystem_deleteFile(obj, strPath.str)){
						r.filesCount.error++;
					} else {
						r.filesCount.success++;
					}
					r.filesCount.total++;
				}
				r.foldersCount.success++;
			}
		}
		//Process subfolders
		if(recursive){
			NBArray_empty(&arr);
			NBString_empty(&strs);
			if(NBFilesystem_getFolders(obj, folderPath, &strs, &arr)){
				SI32 i; for(i =0; i < arr.use; i++){
					const STNBFilesystemFile* ff = NBArray_itmPtrAtIndex(&arr, STNBFilesystemFile, i);
					const char* ffName = &strs.str[ff->name];
					//Build filepath
					{
						NBString_set(&strPath, folderPath);
						if(strPath.length > 0){
							if(strPath.str[strPath.length - 1] != '/' && strPath.str[strPath.length - 1] != '\\'){
								NBString_concatByte(&strPath, '/');
							}
						}
						NBString_concat(&strPath, ffName);
					}
					{
						const STNBFilesystemResult rr = NBFilesystem_deleteFilesAtFolder(obj, strPath.str, recursive);
						//
						r.filesCount.total		+= rr.filesCount.total;
						r.filesCount.ignored	+= rr.filesCount.ignored;
						r.filesCount.success	+= rr.filesCount.success;
						r.filesCount.error		+= rr.filesCount.error;
						//
						r.foldersCount.total	+= rr.foldersCount.total;
						r.foldersCount.ignored	+= rr.foldersCount.ignored;
						r.foldersCount.success	+= rr.foldersCount.success;
						r.foldersCount.error	+= rr.foldersCount.error;
						
					}
				}
			}
		}
		NBString_release(&strs);
		NBString_release(&strPath);
		NBArray_release(&arr);
	}
	return r;
}

//Files (explicit root)

BOOL NBFilesystem_getFilesAtRoot(const STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* folderpath, const BOOL includeStats, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->getFiles != NULL){
		r = (*topItf->getFiles)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), root, folderpath, includeStats, dstStrs, dstFiles);
	}
	return r;
}

BOOL NBFilesystem_moveFileAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot srcRoot, const char* srcFilepath, const ENNBFilesystemRoot dstRoot, const char* dstFilepath){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->moveFile != NULL){
		r = (*topItf->moveFile)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), srcRoot, srcFilepath, dstRoot, dstFilepath);
	}
	return r;
}

BOOL NBFilesystem_deleteFileAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* filepath){
	BOOL r = FALSE;
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj->opaque;
	NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
	IFilesystemItf* topItf = &opq->itfs[0];
	if(topItf->deleteFile != NULL){
		r = (*topItf->deleteFile)(topItf->obj, &opq->itfs[1], (opq->itfsSz - 1), root, filepath);
	}
	return r;
}

STNBFilesystemResult NBFilesystem_deleteFilesAtFolderAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* folderPath, const BOOL recursive){
	STNBFilesystemResult r;
	NBMemory_setZeroSt(r, STNBFilesystemResult);
	{
		STNBString strs, strPath;
		STNBArray arr;
		NBString_init(&strs);
		NBString_init(&strPath);
		NBArray_init(&arr, sizeof(STNBFilesystemFile), NULL);
		//Delete files
		{
			NBArray_empty(&arr);
			NBString_empty(&strs);
			if(!NBFilesystem_getFilesAtRoot(obj, root, folderPath, FALSE, &strs, &arr)){
				r.foldersCount.error++;
			} else {
				SI32 i; for(i =0; i < arr.use; i++){
					const STNBFilesystemFile* ff = NBArray_itmPtrAtIndex(&arr, STNBFilesystemFile, i);
					const char* ffName = &strs.str[ff->name];
					//Build filepath
					{
						NBString_set(&strPath, folderPath);
						if(strPath.length > 0){
							if(strPath.str[strPath.length - 1] != '/' && strPath.str[strPath.length - 1] != '\\'){
								NBString_concatByte(&strPath, '/');
							}
						}
						NBString_concat(&strPath, ffName);
					}
					if(!NBFilesystem_deleteFileAtRoot(obj, root, strPath.str)){
						r.filesCount.error++;
					} else {
						r.filesCount.success++;
					}
					r.filesCount.total++;
				}
				r.foldersCount.success++;
			}
		}
		//Process subfolders
		if(recursive){
			NBArray_empty(&arr);
			NBString_empty(&strs);
			if(NBFilesystem_getFoldersAtRoot(obj, root, folderPath, &strs, &arr)){
				SI32 i; for(i =0; i < arr.use; i++){
					const STNBFilesystemFile* ff = NBArray_itmPtrAtIndex(&arr, STNBFilesystemFile, i);
					const char* ffName = &strs.str[ff->name];
					//Build filepath
					{
						NBString_set(&strPath, folderPath);
						if(strPath.length > 0){
							if(strPath.str[strPath.length - 1] != '/' && strPath.str[strPath.length - 1] != '\\'){
								NBString_concatByte(&strPath, '/');
							}
						}
						NBString_concat(&strPath, ffName);
					}
					{
						const STNBFilesystemResult rr = NBFilesystem_deleteFilesAtFolderAtRoot(obj, root, strPath.str, recursive);
						//
						r.filesCount.total		+= rr.filesCount.total;
						r.filesCount.ignored	+= rr.filesCount.ignored;
						r.filesCount.success	+= rr.filesCount.success;
						r.filesCount.error		+= rr.filesCount.error;
						//
						r.foldersCount.total	+= rr.foldersCount.total;
						r.foldersCount.ignored	+= rr.foldersCount.ignored;
						r.foldersCount.success	+= rr.foldersCount.success;
						r.foldersCount.error	+= rr.foldersCount.error;
						
					}
				}
			}
		}
		NBString_release(&strs);
		NBString_release(&strPath);
		NBArray_release(&arr);
	}
	return r;
}

//Write

BOOL NBFilesystem_writeToFilepath(STNBFilesystem* obj, const char* filepath, void* data, const UI32 dataSz){
	BOOL r = FALSE;
	STNBFileRef file = NBFile_alloc(NULL);
	if(!NBFilesystem_open(obj, filepath, ENNBFileMode_Write, file)){
		PRINTF_ERROR("Could not open file to write: '%s'.\n", filepath);
	} else {
		NBFile_lock(file);
		if(NBFile_write(file, data, dataSz) != dataSz){
			PRINTF_ERROR("Could not write %d bytes at opened file '%s'.\n", dataSz, filepath);
		} else {
			r = TRUE;
		}
		NBFile_unlock(file);
		NBFile_close(file);
	}
	NBFile_release(&file);
	return r;
}

BOOL NBFilesystem_writeToFilepathAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* filepath, void* data, const UI32 dataSz){
	BOOL r = FALSE;
	STNBFileRef file = NBFile_alloc(NULL);
	if(!NBFilesystem_openAtRoot(obj, root, filepath, ENNBFileMode_Write, file)){
		//PRINTF_ERROR("Could not open file (at root %d) to write: '%s'.\n", root, filepath);
	} else {
		NBFile_lock(file);
		if(NBFile_write(file, data, dataSz) != dataSz){
			PRINTF_ERROR("Could not write %d bytes at opened (at root %d) file.\n", dataSz, root);
		} else {
			r = TRUE;
		}
		NBFile_unlock(file);
		NBFile_close(file);
	}
	NBFile_release(&file);
	return r;
}

BOOL NBFilesystem_writeToFilepathStrs(STNBFilesystem* obj, const char** arr, const UI32 arrSz, void* data, const UI32 dataSz){
	BOOL r = FALSE;
	STNBString filepath;
	NBString_initWithStrs(&filepath, arr, arrSz);
	r = NBFilesystem_writeToFilepath(obj, filepath.str, data, dataSz);
	NBString_release(&filepath);
	return r;
}

BOOL NBFilesystem_writeToFilepathStrsAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arr, const UI32 arrSz, void* data, const UI32 dataSz){
	BOOL r = FALSE;
	STNBString filepath;
	NBString_initWithStrs(&filepath, arr, arrSz);
	r = NBFilesystem_writeToFilepathAtRoot(obj, root, filepath.str, data, dataSz);
	NBString_release(&filepath);
	return r;
}

BOOL NBFilesystem_writeToFilepathStrsAndNull(STNBFilesystem* obj, const char** arrAndNull, void* data, const UI32 dataSz){
	BOOL r = FALSE;
	STNBString filepath;
	NBString_initWithStrsAndNull(&filepath, arrAndNull);
	r = NBFilesystem_writeToFilepath(obj, filepath.str, data, dataSz);
	NBString_release(&filepath);
	return r;
}

BOOL NBFilesystem_writeToFilepathStrsAndNullAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arrAndNull, void* data, const UI32 dataSz){
	BOOL r = FALSE;
	STNBString filepath;
	NBString_initWithStrsAndNull(&filepath, arrAndNull);
	r = NBFilesystem_writeToFilepathAtRoot(obj, root, filepath.str, data, dataSz);
	NBString_release(&filepath);
	return r;
}

//Read

BOOL NBFilesystem_readFromFilepath(STNBFilesystem* obj, const char* filepath, STNBString* dst){
	BOOL r = FALSE;
	STNBFileRef file = NBFile_alloc(NULL);
	if(!NBFilesystem_open(obj, filepath, ENNBFileMode_Read, file)){
		PRINTF_ERROR("Could not open file to read: '%s'.\n", filepath);
	} else {
		NBFile_lock(file);
		{
			char buff[4096]; SI32 read;
			do {
				read = NBFile_read(file, buff, 4096);
				if(read <= 0) break;
				NBString_concatBytes(dst, buff, read);
			} while(TRUE);
		}
		NBFile_unlock(file);
		NBFile_close(file);
		r = TRUE;
	}
	NBFile_release(&file);
	return r;
}

BOOL NBFilesystem_readFromFilepathAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* filepath, STNBString* dst){
	BOOL r = FALSE;
	STNBFileRef file = NBFile_alloc(NULL);
	if(!NBFilesystem_openAtRoot(obj, root, filepath, ENNBFileMode_Read, file)){
		//PRINTF_ERROR("Could not open file (at root %d) to read: '%s'.\n", root, filepath);
	} else {
		NBFile_lock(file);
		{
			char buff[4096]; SI32 read;
			do {
				read = NBFile_read(file, buff, 4096);
				if(read <= 0) break;
				NBString_concatBytes(dst, buff, read);
			} while(TRUE);
		}
		NBFile_unlock(file);
		NBFile_close(file);
		r = TRUE;
	}
	NBFile_release(&file);
	return r;
}

BOOL NBFilesystem_readFromFilepathStrs(STNBFilesystem* obj, const char** arr, const UI32 arrSz, STNBString* dst){
	BOOL r = FALSE;
	STNBString filepath;
	NBString_initWithStrs(&filepath, arr, arrSz);
	r = NBFilesystem_readFromFilepath(obj, filepath.str, dst);
	NBString_release(&filepath);
	return r;
}

BOOL NBFilesystem_readFromFilepathStrsAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arr, const UI32 arrSz, STNBString* dst){
	BOOL r = FALSE;
	STNBString filepath;
	NBString_initWithStrs(&filepath, arr, arrSz);
	r = NBFilesystem_readFromFilepathAtRoot(obj, root, filepath.str, dst);
	NBString_release(&filepath);
	return r;
}

BOOL NBFilesystem_readFromFilepathStrsAndNull(STNBFilesystem* obj, const char** arrAndNull, STNBString* dst){
	BOOL r = FALSE;
	STNBString filepath;
	NBString_initWithStrsAndNull(&filepath, arrAndNull);
	r = NBFilesystem_readFromFilepath(obj, filepath.str, dst);
	NBString_release(&filepath);
	return r;
}

BOOL NBFilesystem_readFromFilepathStrsAndNullAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arrAndNull, STNBString* dst){
	BOOL r = FALSE;
	STNBString filepath;
	NBString_initWithStrsAndNull(&filepath, arrAndNull);
	r = NBFilesystem_readFromFilepathAtRoot(obj, root, filepath.str, dst);
	NBString_release(&filepath);
	return r;
}

//------------------------
//-- Native Itf
//------------------------

void NBFilesystem_createItf(void* pObj, IFilesystemItf* dst){
	STNBFilesystem* obj		= (STNBFilesystem*)pObj;
	STNBFilesystemOpq* opq	= (STNBFilesystemOpq*)obj->opaque;
	//Current itf
	NBFilesystem_populateNativeItf(dst);
	dst->obj				= opq;
}

void NBFilesystem_retain_(void* obj){
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)obj;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount++;
}

void NBFilesystem_release_(void** obj){
	STNBFilesystemOpq* opq = (STNBFilesystemOpq*)*obj;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount--;
	if(opq->retainCount == 0){
		//itfs stack
		{
			NBASSERT(opq->itfs != NULL && opq->itfsSz > 0)
			if(opq->itfs != NULL){
				//call release on all itf except myself (the last one)
				if(opq->itfsSz > 1){
					SI32 i; for(i = (SI32)opq->itfsSz - 2; i >= 0; i--){
						IFilesystemItf* itf = &opq->itfs[i];
						if(itf->release != NULL){
							(*itf->release)(&itf->obj);
						}
					}
				}
				NBMemory_free(opq->itfs);
				opq->itfs = NULL;
			}
			opq->itfsSz = 0;
		}
		NBMemory_free(*obj);
		*obj = NULL;
	}
}
//

BOOL NBFilesystem_concatRootPath_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* fileName, STNBString* dst){
	BOOL r = FALSE;
	if(root >= 0 && root < ENNBFilesystemRoot_Count){
		if(dst != NULL && fileName != NULL){
			NBString_concat(dst, fileName);
		}
		r = TRUE;
	}
	return r;
}

//Folders

BOOL NBFilesystem_getFolders_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/){
	BOOL r = FALSE;
	if(folderpath != NULL && folderpath[0] != '\0' && root == ENNBFilesystemRoot_WorkDir){
#		if defined(_WIN32)
        STNBString srchPath;
        NBString_init(&srchPath);
        NBString_concat(&srchPath, folderpath);
        if (srchPath.length == 0 || srchPath.str[srchPath.length - 1] != '/') {
            NBString_concat(&srchPath, "/");
        }
        NBString_concat(&srchPath, "*");
        WIN32_FIND_DATA FindFileData;
        HANDLE hFind = FindFirstFile(srchPath.str, &FindFileData);
        if (hFind == INVALID_HANDLE_VALUE) {
            PRINTF_ERROR("Could not FindFirstFile to searchPath '%s'\n", srchPath.str);
#			ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
            NBMngrProcess_fsFolderOpened(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Error);
#			endif
        } else {
#			ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
            NBMngrProcess_fsFolderOpened(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#			endif
            do{
#				ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
                NBMngrProcess_fsFileStat(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#				endif
                if((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0){
                    if(FindFileData.cFileName[0]!='.'){
                        const char* fname = FindFileData.cFileName;
                        STNBFilesystemFile f;
                        NBMemory_setZeroSt(f, STNBFilesystemFile);
                        f.name		= dstStrs->length; NBString_concat(dstStrs, fname); NBString_concatByte(dstStrs, '\0');
                        f.isSymLink	= FALSE;
                        f.root		= root;
                        NBArray_addValue(dstFiles, f);
                    }
                }
            } while (FindNextFile(hFind, &FindFileData));
            FindClose(hFind);
#			ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
            NBMngrProcess_fsFolderClosed(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#			endif
            r = TRUE;
        }
        NBString_release(&srchPath);
#		elif !defined(__QNX__) //BB10
        DIR* dp; struct dirent *entry;
        dp = opendir(folderpath);
        if (dp == NULL) {
            //PRINTF_ERROR("Could not open the folder '%s'\n", folderpath);
#			ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
            NBMngrProcess_fsFolderOpened(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Error);
#			endif
        } else {
#			ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
            NBMngrProcess_fsFolderOpened(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#			endif
            while((entry = readdir(dp))){
#				ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
                NBMngrProcess_fsFolderRead(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#				endif
                //DT_UNKNOWN	 0 //The file type is unknown.
                //DT_FIFO		 1 //This is a named pipe (FIFO).
                //DT_CHR		 2 //This is a character device.
                //DT_DIR		 4 //This is a directory.
                //DT_BLK		 6 //This is a block device.
                //DT_REG		 8 //This is a regular file.
                //DT_LNK		10 //This is a symbolic link.
                //DT_SOCK		12 //This is a UNIX domain socket.
                //DT_WHT		14 //
                if(!NBString_strIsEqual(entry->d_name, ".") && !NBString_strIsEqual(entry->d_name, "..")){ //Ignore '.' and '..'
                    if(entry->d_type == DT_DIR){
                        STNBFilesystemFile f;
                        NBMemory_setZeroSt(f, STNBFilesystemFile);
                        f.name		= dstStrs->length; NBString_concat(dstStrs, entry->d_name); NBString_concatByte(dstStrs, '\0');
                        f.isSymLink	= FALSE;
                        f.root		= root;
                        NBArray_addValue(dstFiles, f);
                    } else if(entry->d_type == DT_LNK){
                        struct stat lnkStat;
                        STNBString lnkpath;
                        NBString_initWithStr(&lnkpath, folderpath);
                        if(lnkpath.str[lnkpath.length - 1] != '/'){
                            NBString_concatByte(&lnkpath, '/');
                        }
                        NBString_concat(&lnkpath, entry->d_name);
                        if(stat(lnkpath.str, &lnkStat) == 0){
#							ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
                            NBMngrProcess_fsFileStat(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#							endif
                            if(S_ISDIR(lnkStat.st_mode)){
                                STNBFilesystemFile f;
                                NBMemory_setZeroSt(f, STNBFilesystemFile);
                                f.name		= dstStrs->length; NBString_concat(dstStrs, entry->d_name); NBString_concatByte(dstStrs, '\0');
#								ifdef __APPLE__
                                f.readTime	= lnkStat.st_atimespec.tv_sec;
                                f.modTime	= lnkStat.st_mtimespec.tv_sec;
                                f.createTime = lnkStat.st_ctimespec.tv_sec;
#								else
                                f.readTime	= lnkStat.st_atim.tv_sec;
                                f.modTime	= lnkStat.st_mtim.tv_sec;
                                f.createTime = lnkStat.st_ctim.tv_sec;
#								endif
                                f.isSymLink	= TRUE;
                                f.root		= root;
                                NBArray_addValue(dstFiles, f);
                            }
#						ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
                        } else {
                            //PRINTF_ERROR("stat returned error: '%s' for '%s'.\n", strerror(errno), lnkpath.str);
#							ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
                            NBMngrProcess_fsFileStat(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Error);
#							endif
#						endif
                        }
                        NBString_release(&lnkpath);
                    }
                }
            }
            closedir(dp);
#			ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
            NBMngrProcess_fsFolderClosed(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#			endif
            r = TRUE;
        }
#		endif
	}
	return r;
}

BOOL NBFilesystem_folderExists_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath){
	BOOL r = FALSE;
	if(folderpath != NULL && folderpath[0] != '\0' && root == ENNBFilesystemRoot_WorkDir){
#		if defined(_WIN32)
        STNBString srchPath;
        NBString_initWithStr(&srchPath, folderpath);
        if(srchPath.str[srchPath.length - 1] != '/'){
            NBString_concatByte(&srchPath, '/');
        }
        NBString_concatByte(&srchPath, '*');
        WIN32_FIND_DATA FindFileData;
        HANDLE hFind = FindFirstFile(srchPath.str, &FindFileData);
        if (hFind == INVALID_HANDLE_VALUE) {
#			ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
            NBMngrProcess_fsFolderOpened(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Error);
#			endif
        } else {
#			ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
            NBMngrProcess_fsFolderOpened(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#			endif
            FindClose(hFind);
#			ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
            NBMngrProcess_fsFolderClosed(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#			endif
            r = TRUE;
        }
        NBString_release(&srchPath);
#		elif !defined(__QNX__) //BB10
        DIR* dp;
        dp = opendir(folderpath);
        if (dp == NULL) {
#			ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
            NBMngrProcess_fsFolderOpened(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Error);
#			endif
        } else {
#			ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
            NBMngrProcess_fsFolderOpened(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#			endif
            closedir(dp);
#			ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
            NBMngrProcess_fsFolderClosed(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#			endif
            r = TRUE;
        }
#		endif
	}
	return r;
}

BOOL NBFilesystem_createFolder_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath){
	BOOL r = FALSE;
    if(folderpath != NULL && folderpath[0] != '\0' && root == ENNBFilesystemRoot_WorkDir){
#		ifdef _WIN32
        if(CreateDirectory(folderpath, NULL)){
            r = TRUE;
        }
#		else
        if(mkdir(folderpath, 0777) == 0){
            r = TRUE;
        }
#		endif
#		ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
        NBMngrProcess_fsFolderCreated(ENNBFilesystemStatsSrc_Native, (r ? ENNBFilesystemStatsResult_Success : ENNBFilesystemStatsResult_Error));
#		endif
    }
	return r;
}

BOOL NBFilesystem_deleteFolder_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath){
	BOOL r = FALSE;
	if(folderpath != NULL && folderpath[0] != '\0' && root == ENNBFilesystemRoot_WorkDir){
#		ifdef _WIN32
        NBASSERT(FALSE) //ToDo: implement
#		else
        if(rmdir(folderpath) == 0){
            r = TRUE;
        }
#		endif
#		ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
        NBMngrProcess_fsFolderDeleted(ENNBFilesystemStatsSrc_Native, (r ? ENNBFilesystemStatsResult_Success : ENNBFilesystemStatsResult_Error));
#		endif
	}
	return r;
}

//Files

BOOL NBFilesystem_open_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* filepath, const ENNBFileMode mode, STNBFileRef dst){
    BOOL r = FALSE;
    if(filepath != NULL && filepath[0] != '\0' && root == ENNBFilesystemRoot_WorkDir){
        r = NBFile_open(dst, filepath, mode);
    }
    return r;
}

BOOL NBFilesystem_getFiles_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath, const BOOL includeStats, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/){
	BOOL r = FALSE;
	if(folderpath != NULL && folderpath[0] != '\0' && root == ENNBFilesystemRoot_WorkDir){
        STNBString lnkpath;
        NBString_initWithSz(&lnkpath, 0, 64, 0.10f);
        //ToDo: load from packagesStore
        //Buscar dentro de paquetes cargados
        /*UI32 iPaq, iPaqConteo = _paquetes->conteo;
         for(iPaq=0; iPaq<iPaqConteo; iPaq++){
         STGestorArchivosPaquete* data = _paquetes->elemPtr(iPaq);
         data->indice->listarArchivosDe(rutaCarpetaPadreSinPlecaAlFinal, agregarEn);
         }*/
        //Buscar en sistema de archivo
#		if defined(_WIN32)
        STNBString srchPath;
        NBString_init(&srchPath);
        NBString_concat(&srchPath, folderpath);
        if (srchPath.length == 0 || srchPath.str[srchPath.length - 1] != '/') {
            NBString_concat(&srchPath, "/");
        }
        NBString_concat(&srchPath, "*");
        WIN32_FIND_DATA FindFileData;
        HANDLE hFind = FindFirstFile(srchPath.str, &FindFileData);
        if (hFind == INVALID_HANDLE_VALUE) {
            PRINTF_ERROR("no se pudo leer la carpeta '%s'\n", srchPath.str);
#			ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
            NBMngrProcess_fsFolderOpened(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Error);
#			endif
        } else {
#			ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
            NBMngrProcess_fsFolderOpened(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#			endif
            do {
#				ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
                NBMngrProcess_fsFileStat(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#				endif
                if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                    if (FindFileData.cFileName[0] != '.') {
                        const char* fname = FindFileData.cFileName;
                        STNBFilesystemFile f;
                        NBMemory_setZeroSt(f, STNBFilesystemFile);
                        f.name		= dstStrs->length; NBString_concat(dstStrs, fname); NBString_concatByte(dstStrs, '\0');
                        f.isSymLink = FALSE;
                        f.root		= root;
                        NBArray_addValue(dstFiles, f);
                    }
                }
            } while (FindNextFile(hFind, &FindFileData));
            FindClose(hFind);
#			ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
            NBMngrProcess_fsFolderClosed(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#			endif
            r = TRUE;
        }
        NBString_release(&srchPath);
#		elif !defined(__QNX__) //BB10
        DIR* dp; struct dirent *entry;
        dp = opendir(folderpath);
        if (dp == NULL) {
            //PRINTF_ERROR("Could not open the folder '%s'\n", folderpath);
#			ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
            NBMngrProcess_fsFolderOpened(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Error);
#			endif
        } else {
#			ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
            NBMngrProcess_fsFolderOpened(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#			endif
            while((entry = readdir(dp))){
#				ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
                NBMngrProcess_fsFolderRead(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#				endif
                //DT_UNKNOWN	 0 //The file type is unknown.
                //DT_FIFO		 1 //This is a named pipe (FIFO).
                //DT_CHR		 2 //This is a character device.
                //DT_DIR		 4 //This is a directory.
                //DT_BLK		 6 //This is a block device.
                //DT_REG		 8 //This is a regular file.
                //DT_LNK		10 //This is a symbolic link.
                //DT_SOCK		12 //This is a UNIX domain socket.
                //DT_WHT		14 //
                if(!NBString_strIsEqual(entry->d_name, ".") && !NBString_strIsEqual(entry->d_name, "..")){ //Ignore '.' and '..'
                    if(entry->d_type == DT_REG){
                        STNBFilesystemFile f;
                        NBMemory_setZeroSt(f, STNBFilesystemFile);
                        f.name		= dstStrs->length; NBString_concat(dstStrs, entry->d_name); NBString_concatByte(dstStrs, '\0');
                        f.isSymLink	= FALSE;
                        f.root		= root;
                        if(includeStats){
                            struct stat lnkStat;
                            NBString_set(&lnkpath, folderpath);
                            if(lnkpath.str[lnkpath.length - 1] != '/'){
                                NBString_concatByte(&lnkpath, '/');
                            }
                            NBString_concat(&lnkpath, entry->d_name);
                            if(stat(lnkpath.str, &lnkStat) == 0){
#								ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
                                NBMngrProcess_fsFileStat(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#								endif
#								ifdef __APPLE__
                                f.readTime	= lnkStat.st_atimespec.tv_sec;
                                f.modTime	= lnkStat.st_mtimespec.tv_sec;
                                f.createTime = lnkStat.st_ctimespec.tv_sec;
#								else
                                f.readTime	= lnkStat.st_atim.tv_sec;
                                f.modTime	= lnkStat.st_mtim.tv_sec;
                                f.createTime = lnkStat.st_ctim.tv_sec;
#								endif
                            }
#							ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
                            else {
                                //PRINTF_ERROR("stat returned error: '%s' for '%s'\n", strerror(errno), lnkpath.str);
#								ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
                                NBMngrProcess_fsFileStat(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Error);
#								endif
                            }
#							endif
                        }
                        NBArray_addValue(dstFiles, f);
                    } else if(entry->d_type == DT_LNK){
                        struct stat lnkStat;
                        NBString_set(&lnkpath, folderpath);
                        if(lnkpath.str[lnkpath.length - 1] != '/'){
                            NBString_concatByte(&lnkpath, '/');
                        }
                        NBString_concat(&lnkpath, entry->d_name);
                        if(stat(lnkpath.str, &lnkStat) == 0){
#							ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
                            NBMngrProcess_fsFileStat(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#							endif
                            if(!S_ISDIR(lnkStat.st_mode)){
                                STNBFilesystemFile f;
                                NBMemory_setZeroSt(f, STNBFilesystemFile);
                                f.name		= dstStrs->length; NBString_concat(dstStrs, entry->d_name); NBString_concatByte(dstStrs, '\0');
                                f.isSymLink	= TRUE;
#								ifdef __APPLE__
                                f.readTime	= lnkStat.st_atimespec.tv_sec;
                                f.modTime	= lnkStat.st_mtimespec.tv_sec;
                                f.createTime = lnkStat.st_ctimespec.tv_sec;
#								else
                                f.readTime	= lnkStat.st_atim.tv_sec;
                                f.modTime	= lnkStat.st_mtim.tv_sec;
                                f.createTime = lnkStat.st_ctim.tv_sec;
#								endif
                                f.root		= root;
                                NBArray_addValue(dstFiles, f);
                            }
                        }
#						ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
                        else {
                            //PRINTF_ERROR("stat returned error: '%s' for '%s'\n", strerror(errno), lnkpath.str);
#							ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
                            NBMngrProcess_fsFileStat(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Error);
#							endif
                            
                        }
#						endif
                    }
                }
            }
            closedir(dp);
#			ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
            NBMngrProcess_fsFolderClosed(ENNBFilesystemStatsSrc_Native, ENNBFilesystemStatsResult_Success);
#			endif
            r = TRUE;
        }
#		endif
        NBString_release(&lnkpath);
	}
	return r;
}

BOOL NBFilesystem_moveFile_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot srcRoot, const char* srcFilepath, const ENNBFilesystemRoot dstRoot, const char* dstFilepath){
	BOOL r = FALSE;
	if(srcFilepath != NULL && srcFilepath[0] != '\0' && dstFilepath != NULL && dstFilepath[0] != '\0' && srcRoot == ENNBFilesystemRoot_WorkDir && dstRoot == ENNBFilesystemRoot_WorkDir){
#		if defined(_WIN32)
        if(MoveFileEx(srcFilepath, dstFilepath, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)){
            r = TRUE;
        }
#		else
        if(rename(srcFilepath, dstFilepath) == 0){
            r = TRUE;
        }
#		endif
#		ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
        NBMngrProcess_fsFileMoved(ENNBFilesystemStatsSrc_Native, (r ? ENNBFilesystemStatsResult_Success : ENNBFilesystemStatsResult_Error));
#		endif
	}
	return r;
}

BOOL NBFilesystem_deleteFile_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* filepath){
	BOOL r = FALSE;
	if(filepath != NULL && filepath[0] != '\0' && root == ENNBFilesystemRoot_WorkDir){
#		if defined(_WIN32)
        if(DeleteFile(filepath)){
            r = TRUE;
        }
#		else
        if(remove(filepath) == 0){
            r = TRUE;
        }
#		endif
#		ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
        NBMngrProcess_fsFileDeleted(ENNBFilesystemStatsSrc_Native, (r ? ENNBFilesystemStatsResult_Success : ENNBFilesystemStatsResult_Error));
#		endif
	}
	return r;
}

