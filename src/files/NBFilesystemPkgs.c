//
//  NBFilesystemPkgs.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/9/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/files/NBFilesystemPkgs.h"
//
#include "nb/core/NBMemory.h"
#include "nb/files/NBFilesPkg.h"
#include "nb/files/NBFilesystem.h"

typedef struct STNBFilesystemPkgsPkg_ {
	ENNBFilesystemRoot	root;
	UI32				filename;
	STNBFileRef			parentFile;
	STNBFilesPkg		pkg;
} STNBFilesystemPkgsPkg;

typedef struct STNBFilesystemPkgsOpq_ {
	//default
	struct {
		STNBFilesystem	fs;			//filesystem used when none is provided
		IFilesystemItf	itf;		//itf (of default filesystem)
	} def;
	STNBArray			pkgs;		//STNBFilesystemPkgsPkg
	STNBString			strs;
	UI32				retainCount;
} STNBFilesystemPkgsOpq;

//Filesystem itf
void NBFilesystemPkgs_retain_(void* obj);
void NBFilesystemPkgs_release_(void** obj);
//
BOOL NBFilesystemPkgs_concatRootPath_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* fileName, STNBString* dst);
//
BOOL NBFilesystemPkgs_getFolders_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
BOOL NBFilesystemPkgs_folderExists_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath);
BOOL NBFilesystemPkgs_createFolder_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath);
BOOL NBFilesystemPkgs_deleteFolder_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath);
//
BOOL NBFilesystemPkgs_open_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* filepath, const ENNBFileMode mode, STNBFileRef dst);
BOOL NBFilesystemPkgs_getFiles_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath, const BOOL includeStats, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
BOOL NBFilesystemPkgs_moveFile_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot srcRoot, const char* srcFilepath, const ENNBFilesystemRoot dstRoot, const char* dstFilepath);
BOOL NBFilesystemPkgs_deleteFile_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* filepath);

//

void NBFilesystemPkgs_init(STNBFilesystemPkgs* obj){
	STNBFilesystemPkgsOpq* opq = obj->opaque = NBMemory_allocType(STNBFilesystemPkgsOpq);
	NBMemory_setZeroSt(*opq, STNBFilesystemPkgsOpq);
	//default
	{
		NBFilesystem_init(&opq->def.fs);
		NBFilesystem_createItf(&opq->def.fs, &opq->def.itf);
	}
	NBArray_init(&opq->pkgs, sizeof(STNBFilesystemPkgsPkg), NULL);
	NBString_init(&opq->strs);
	NBString_concatByte(&opq->strs, '\0');
	opq->retainCount = 1;
}

void NBFilesystemPkgs_retain(STNBFilesystemPkgs* obj){
	NBFilesystemPkgs_retain_(obj->opaque);
}
	
void NBFilesystemPkgs_release(STNBFilesystemPkgs* obj){
	NBFilesystemPkgs_release_(&obj->opaque);
}

//

void NBFilesystemPkgs_createItf(void* pObj, IFilesystemItf* dst){
	STNBFilesystemPkgs* obj = (STNBFilesystemPkgs*)pObj;
	STNBFilesystemPkgsOpq* opq = (STNBFilesystemPkgsOpq*)obj->opaque;
	//Set iterface
	NBMemory_set(dst, 0, sizeof(*dst));
	//
	dst->obj			= opq;
	//
	dst->retain			= NBFilesystemPkgs_retain_;
	dst->release		= NBFilesystemPkgs_release_;
	//
	dst->concatRootPath	= NBFilesystemPkgs_concatRootPath_;
	//
	dst->getFolders		= NBFilesystemPkgs_getFolders_;
	dst->folderExists	= NBFilesystemPkgs_folderExists_;
	dst->createFolder	= NBFilesystemPkgs_createFolder_;
	dst->deleteFolder	= NBFilesystemPkgs_deleteFolder_;
	//
	dst->open			= NBFilesystemPkgs_open_;
	dst->getFiles		= NBFilesystemPkgs_getFiles_;
	dst->moveFile		= NBFilesystemPkgs_moveFile_;
	dst->deleteFile		= NBFilesystemPkgs_deleteFile_;
}

//Pkgs

BOOL NBFilesystemPkgs_addPkgFromPath(STNBFilesystemPkgs* obj, const ENNBFilesystemRoot root, const char* filepath){
	BOOL r = FALSE;
	STNBFilesystemPkgsOpq* opq = (STNBFilesystemPkgsOpq*)obj->opaque;
	if(opq->def.itf.open != NULL){
		STNBFileRef parentFile = NBFile_alloc(NULL);
		if(!(*opq->def.itf.open)(opq->def.itf.obj, NULL, 0, root, filepath, ENNBFileMode_Read, parentFile)){
			PRINTF_INFO("Could not open file-with-root pkg: '%s'.\n", filepath);
		} else {
			STNBFilesPkg pkg;
			NBFilesPkg_init(&pkg);
			if(!NBFilesPkg_loadFromFile(&pkg, parentFile)){
				PRINTF_INFO("Could not load pkg: '%s'.\n", filepath);
				NBFilesPkg_release(&pkg);
			} else {
				STNBFilesystemPkgsPkg rPkg;
				rPkg.root		= root;
				rPkg.filename	= opq->strs.length;
                rPkg.parentFile	= parentFile; NBFile_null(&parentFile);
				rPkg.pkg		= pkg;
				NBString_concat(&opq->strs, filepath);
				NBString_concatByte(&opq->strs, '\0');
				NBArray_addValue(&opq->pkgs, rPkg);
				//PRINTF_INFO("Pkg added: '%s' (total %d).\n", filepath, opq->pkgs.use);
				r = TRUE;
			}
		}
		if(NBFile_isSet(parentFile)){
			NBFile_release(&parentFile);
            NBFile_null(&parentFile);
		}
	} else {
		STNBFilesPkg pkg;
		NBFilesPkg_init(&pkg);
		if(!NBFilesPkg_loadFromFilepath(&pkg, filepath)){
			PRINTF_INFO("Could not load pkg: '%s'.\n", filepath);
			NBFilesPkg_release(&pkg);
		} else {
			STNBFilesystemPkgsPkg rPkg;
            NBMemory_setZeroSt(rPkg, STNBFilesystemPkgsPkg);
			rPkg.root		= root;
			rPkg.filename	= opq->strs.length;
			rPkg.pkg		= pkg;
			NBString_concat(&opq->strs, filepath);
			NBString_concatByte(&opq->strs, '\0');
			NBArray_addValue(&opq->pkgs, rPkg);
			//PRINTF_INFO("Pkg added: '%s' (total %d).\n", filepath, opq->pkgs.use);
			r = TRUE;
		}
	}
	return r;
}

BOOL NBFilesystemPkgs_addPkgFromPersistentData(STNBFilesystemPkgs* obj, const char* pkgName, const BYTE* data, const UI64 dataSz){
	BOOL r = FALSE;
	STNBFilesystemPkgsOpq* opq = (STNBFilesystemPkgsOpq*)obj->opaque;
	STNBFileRef parentFile = NBFile_alloc(NULL);
	if(!NBFile_openAsDataRng(parentFile, (void*)data, (UI32)dataSz)){
		PRINTF_INFO("Could not open file-with-data pkg: '%s'.\n", pkgName);
	} else {
		STNBFilesPkg pkg;
		NBFilesPkg_init(&pkg);
		if(!NBFilesPkg_loadFromFile(&pkg, parentFile)){
			PRINTF_INFO("Could not load pkg: '%s'.\n", pkgName);
			NBFilesPkg_release(&pkg);
		} else {
			STNBFilesystemPkgsPkg rPkg;
			rPkg.root		= ENNBFilesystemRoot_Count;
			rPkg.filename	= opq->strs.length;
            rPkg.parentFile	= parentFile; NBFile_null(&parentFile);
			rPkg.pkg		= pkg;
			NBString_concat(&opq->strs, pkgName);
			NBString_concatByte(&opq->strs, '\0');
			NBArray_addValue(&opq->pkgs, rPkg);
			//PRINTF_INFO("Pkg added: '%s' (total %d).\n", filepath, opq->pkgs.use);
			r = TRUE;
		}
	}
	if(NBFile_isSet(parentFile)){
		NBFile_release(&parentFile);
        NBFile_null(&parentFile);
	}
	return r;
}

BOOL NBFilesystemPkgs_isPkgLoaded(STNBFilesystemPkgs* obj, const ENNBFilesystemRoot root, const char* filepath){
	BOOL r = FALSE;
	STNBFilesystemPkgsOpq* opq = (STNBFilesystemPkgsOpq*)obj->opaque;
	UI32 i; for(i = 0 ; i < opq->pkgs.use; i++){
		STNBFilesystemPkgsPkg* pkg = NBArray_itmPtrAtIndex(&opq->pkgs, STNBFilesystemPkgsPkg, i);
		if(pkg->root == root){
			if(NBString_strIsEqual(&opq->strs.str[pkg->filename], filepath)){
				r = TRUE;
				break;
			}
		}
	}
	return r;
}

//Folders

BOOL NBFilesystemPkgs_getFolders(const STNBFilesystemPkgs* obj, const char* folderpath, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/){
	STNBFilesystemPkgsOpq* opq = (STNBFilesystemPkgsOpq*)obj->opaque;
	return NBFilesystemPkgs_getFolders_(opq, &opq->def.itf, 1, ENNBFilesystemRoot_PkgsDir, folderpath, dstStrs, dstFiles);
}

BOOL NBFilesystemPkgs_folderExists(const STNBFilesystemPkgs* obj, const char* folderpath){
	STNBFilesystemPkgsOpq* opq = (STNBFilesystemPkgsOpq*)obj->opaque;
	return NBFilesystemPkgs_folderExists_(opq, &opq->def.itf, 1, ENNBFilesystemRoot_PkgsDir, folderpath);
}

BOOL NBFilesystemPkgs_createFolder(STNBFilesystemPkgs* obj, const char* folderpath){
	STNBFilesystemPkgsOpq* opq = (STNBFilesystemPkgsOpq*)obj->opaque;
	return NBFilesystemPkgs_createFolder_(opq, &opq->def.itf, 1, ENNBFilesystemRoot_PkgsDir, folderpath);
}

//Files

BOOL NBFilesystemPkgs_open(STNBFilesystemPkgs* obj, const char* filepath, const ENNBFileMode mode, STNBFileRef dst){
	STNBFilesystemPkgsOpq* opq = (STNBFilesystemPkgsOpq*)obj->opaque;
	return NBFilesystemPkgs_open_(opq, &opq->def.itf, 1, ENNBFilesystemRoot_PkgsDir, filepath, mode, dst);
}

BOOL NBFilesystemPkgs_getFiles(const STNBFilesystemPkgs* obj, const char* folderpath, const BOOL includeStats, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/){
	STNBFilesystemPkgsOpq* opq = (STNBFilesystemPkgsOpq*)obj->opaque;
	return NBFilesystemPkgs_getFiles_(opq, &opq->def.itf, 1, ENNBFilesystemRoot_PkgsDir, folderpath, includeStats, dstStrs, dstFiles);
}

BOOL NBFilesystemPkgs_moveFile(STNBFilesystemPkgs* obj, const char* srcFilepath, const ENNBFilesystemRoot dstRoot, const char* dstFilepath){
	STNBFilesystemPkgsOpq* opq = (STNBFilesystemPkgsOpq*)obj->opaque;
	return NBFilesystemPkgs_moveFile_(opq, &opq->def.itf, 1, ENNBFilesystemRoot_PkgsDir, srcFilepath, dstRoot, dstFilepath);
}

BOOL NBFilesystemPkgs_deleteFile(STNBFilesystemPkgs* obj, const char* filepath){
	STNBFilesystemPkgsOpq* opq = (STNBFilesystemPkgsOpq*)obj->opaque;
	return NBFilesystemPkgs_deleteFile_(opq, &opq->def.itf, 1, ENNBFilesystemRoot_PkgsDir, filepath);
}

//Native Itf

void NBFilesystemPkgs_retain_(void* obj){
	STNBFilesystemPkgsOpq* opq = (STNBFilesystemPkgsOpq*)obj;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount++;
}
void NBFilesystemPkgs_release_(void** obj){
	STNBFilesystemPkgsOpq* opq = (STNBFilesystemPkgsOpq*)*obj;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount--;
	if(opq->retainCount == 0){
		//Packages
		{
			UI32 i; for(i = 0 ; i < opq->pkgs.use; i++){
				STNBFilesystemPkgsPkg* pkg = NBArray_itmPtrAtIndex(&opq->pkgs, STNBFilesystemPkgsPkg, i);
				NBFilesPkg_release(&pkg->pkg);
				if(NBFile_isSet(pkg->parentFile)){
					NBFile_release(&pkg->parentFile);
                    NBFile_null(&pkg->parentFile);
				}
			}
			NBArray_empty(&opq->pkgs);
			NBArray_release(&opq->pkgs);
		}
		//default
		{
			NBMemory_setZeroSt(opq->def.itf, IFilesystemItf);
			NBFilesystem_release(&opq->def.fs);
		}
		//Strs
		NBString_release(&opq->strs);
		//Memory
		NBMemory_free(*obj);
		*obj = NULL;
	}
}

//

BOOL NBFilesystemPkgs_concatRootPath_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* fileName, STNBString* dst){
	BOOL r = FALSE;
	//const STNBFilesystemPkgsOpq* opq = (const STNBFilesystemPkgsOpq*)obj;
	if(root >= 0 && root < ENNBFilesystemRoot_Count){
		if(root == ENNBFilesystemRoot_PkgsDir){
			if(dst != NULL && fileName != NULL){
				NBString_concat(dst, fileName);
			}
			r  = TRUE;
		} else if(parents != NULL && parentsSz > 0){
			//Call parent itf
			const IFilesystemItf* pItf = &parents[0];
			if(pItf->concatRootPath != NULL){
				r = (*pItf->concatRootPath)(pItf->obj, &parents[1], (parentsSz - 1), root, fileName, dst);
			}
		}
	}
	return r;
}

BOOL NBFilesystemPkgs_getFolders_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/){
	BOOL r = TRUE;
	//const STNBFilesystemPkgsOpq* opq = (const STNBFilesystemPkgsOpq*)obj;
	if(root == ENNBFilesystemRoot_PkgsDir){
		//Process explicit package "&###&..."
		//Or process all pakcages
		/*if(r){
		 UI32 i; for(i = 0 ; i < opq->pkgs.use; i++){
		 STNBFilesystemPkgsPkg* pkg = NBArray_itmPtrAtIndex(&opq->pkgs, STNBFilesystemPkgsPkg, i);
		 if(!NBFilesPkg_getFolders(&pkg->pkg, folderpath, dstStrs, dstFiles)){
		 r = FALSE; NBASSERT(FALSE)
		 break;
		 }
		 }
		 }*/
	} else if(parents != NULL && parentsSz > 0){
		//Call parent itf
		const IFilesystemItf* pItf = &parents[0];
		if(pItf->getFolders != NULL){
			r = (*pItf->getFolders)(pItf->obj, &parents[1], (parentsSz - 1), root, folderpath, dstStrs, dstFiles);
		}
	}
	return r;
}

BOOL NBFilesystemPkgs_folderExists_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath){
	BOOL r = FALSE;
	//const STNBFilesystemPkgsOpq* opq = (const STNBFilesystemPkgsOpq*)obj;
	if(root == ENNBFilesystemRoot_PkgsDir){
		//Process explicit package "&###&..."
		//Or process all pakcages
		/*if(r){
		 UI32 i; for(i = 0 ; i < opq->pkgs.use; i++){
		 STNBFilesystemPkgsPkg* pkg = NBArray_itmPtrAtIndex(&opq->pkgs, STNBFilesystemPkgsPkg, i);
		 if(!NBFilesPkg_getFolders(&pkg->pkg, folderpath, dstStrs, dstFiles)){
		 r = FALSE; NBASSERT(FALSE)
		 break;
		 }
		 }
		 }*/
	} else if(parents != NULL && parentsSz > 0){
		//Call parent itf
		const IFilesystemItf* pItf = &parents[0];
		if(pItf->folderExists != NULL){
			r = (*pItf->folderExists)(pItf->obj, &parents[1], (parentsSz - 1), root, folderpath);
		}
	}
	return r;
}

BOOL NBFilesystemPkgs_createFolder_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath){
	BOOL r = FALSE;
	//const STNBFilesystemPkgsOpq* opq = (const STNBFilesystemPkgsOpq*)obj;
	if(root == ENNBFilesystemRoot_PkgsDir){
		//Process explicit package "&###&..."
		//Or process all pakcages
		/*if(r){
		 UI32 i; for(i = 0 ; i < opq->pkgs.use; i++){
		 STNBFilesystemPkgsPkg* pkg = NBArray_itmPtrAtIndex(&opq->pkgs, STNBFilesystemPkgsPkg, i);
		 if(!NBFilesPkg_getFolders(&pkg->pkg, folderpath, dstStrs, dstFiles)){
		 r = FALSE; NBASSERT(FALSE)
		 break;
		 }
		 }
		 }*/
	} else if(parents != NULL && parentsSz > 0){
		//Call parent itf
		const IFilesystemItf* pItf = &parents[0];
		if(pItf->createFolder != NULL){
			r = (*pItf->createFolder)(pItf->obj, &parents[1], (parentsSz - 1), root, folderpath);
		}
	}
	return r;
}

BOOL NBFilesystemPkgs_deleteFolder_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath){
	BOOL r = FALSE;
	//const STNBFilesystemPkgsOpq* opq = (const STNBFilesystemPkgsOpq*)obj;
	if(root == ENNBFilesystemRoot_PkgsDir){
		//Process explicit package "&###&..."
		//Or process all pakcages
		/*if(r){
		 UI32 i; for(i = 0 ; i < opq->pkgs.use; i++){
		 STNBFilesystemPkgsPkg* pkg = NBArray_itmPtrAtIndex(&opq->pkgs, STNBFilesystemPkgsPkg, i);
		 if(!NBFilesPkg_getFolders(&pkg->pkg, folderpath, dstStrs, dstFiles)){
		 r = FALSE; NBASSERT(FALSE)
		 break;
		 }
		 }
		 }*/
	} else if(parents != NULL && parentsSz > 0){
		//Call parent itf
		const IFilesystemItf* pItf = &parents[0];
		if(pItf->deleteFolder != NULL){
			r = (*pItf->deleteFolder)(pItf->obj, &parents[1], (parentsSz - 1), root, folderpath);
		}
	}
	return r;
}

//

BOOL NBFilesystemPkgs_open_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* filepath, const ENNBFileMode mode, STNBFileRef dst){
	BOOL r = FALSE;
	const STNBFilesystemPkgsOpq* opq = (const STNBFilesystemPkgsOpq*)obj;
	if(root == ENNBFilesystemRoot_PkgsDir){
		if(mode == ENNBFileMode_Read){
			//Process explicit package "&###&..."
			UI32 iPkg = opq->pkgs.use;
			if(filepath[0] == '&'){
				if(filepath[1] >= '0' && filepath[1] <= '9'){
					if(filepath[2] >= '0' && filepath[3] <= '9'){
						if(filepath[3] >= '0' && filepath[3] <= '9'){
							if(filepath[4] == '&'){
								iPkg = (100 * (UI32)(filepath[1] - '0')) + (10 * (UI32)(filepath[2] - '0')) + (1 * (UI32)(filepath[3] - '0'));
							}
						}
					}
				}
			}
			if(iPkg < opq->pkgs.use){
				STNBFilesystemPkgsPkg* pkg = NBArray_itmPtrAtIndex(&opq->pkgs, STNBFilesystemPkgsPkg, iPkg);
				if(NBFilesPkg_openFile(&pkg->pkg, &filepath[5], dst)){
					r = TRUE;
				}
			} else {
				//Or process all pakcages
				UI32 i; for(i = 0 ; i < opq->pkgs.use; i++){
					STNBFilesystemPkgsPkg* pkg = NBArray_itmPtrAtIndex(&opq->pkgs, STNBFilesystemPkgsPkg, i);
					if(NBFilesPkg_openFile(&pkg->pkg, filepath, dst)){
						r = TRUE;
						break;
					}
				}
			}
		}
	} else if(parents != NULL && parentsSz > 0){
		//Call parent itf
		const IFilesystemItf* pItf = &parents[0];
		if(pItf->open != NULL){
			r = (*pItf->open)(pItf->obj, &parents[1], (parentsSz - 1), root, filepath, mode, dst);
		}
	}
	return r;
}

BOOL NBFilesystemPkgs_getFiles_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath, const BOOL includeStats, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/){
	BOOL r = TRUE;
	const STNBFilesystemPkgsOpq* opq = (const STNBFilesystemPkgsOpq*)obj;
	if(root == ENNBFilesystemRoot_PkgsDir){
		//Process explicit package "&###&..."
		UI32 iPkg = opq->pkgs.use;
		if(folderpath[0] == '&'){
			if(folderpath[1] >= '0' && folderpath[1] <= '9'){
				if(folderpath[2] >= '0' && folderpath[3] <= '9'){
					if(folderpath[3] >= '0' && folderpath[3] <= '9'){
						if(folderpath[4] == '&'){
							iPkg = (100 * (UI32)(folderpath[1] - '0')) + (10 * (UI32)(folderpath[2] - '0')) + (1 * (UI32)(folderpath[3] - '0'));
						}
					}
				}
			}
		}
		if(iPkg < opq->pkgs.use){
			STNBFilesystemPkgsPkg* pkg = NBArray_itmPtrAtIndex(&opq->pkgs, STNBFilesystemPkgsPkg, iPkg);
			if(!NBFilesPkg_getFiles(&pkg->pkg, &folderpath[5], includeStats, dstStrs, dstFiles)){
				r = FALSE; NBASSERT(FALSE)
			}
		} else {
			//Or process all pakcages
			UI32 i; for(i = 0 ; i < opq->pkgs.use; i++){
				STNBFilesystemPkgsPkg* pkg = NBArray_itmPtrAtIndex(&opq->pkgs, STNBFilesystemPkgsPkg, i);
				if(!NBFilesPkg_getFiles(&pkg->pkg, folderpath, includeStats, dstStrs, dstFiles)){
					r = FALSE; NBASSERT(FALSE)
					break;
				}
			}
		}
	} else if(parents != NULL && parentsSz > 0){
		//Call parent itf
		const IFilesystemItf* pItf = &parents[0];
		if(pItf->getFiles != NULL){
			r = (*pItf->getFiles)(pItf->obj, &parents[1], (parentsSz - 1), root, folderpath, includeStats, dstStrs, dstFiles);
		}
	}
	return r;
}

BOOL NBFilesystemPkgs_moveFile_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot srcRoot, const char* srcFilepath, const ENNBFilesystemRoot dstRoot, const char* dstFilepath){
	BOOL r = FALSE;
	//const STNBFilesystemPkgsOpq* opq = (const STNBFilesystemPkgsOpq*)obj;
	if(srcRoot == ENNBFilesystemRoot_PkgsDir){
		//Process explicit package "&###&..."
		//Or process all pakcages
		/*UI32 i; for(i = 0 ; i < opq->pkgs.use; i++){
		 STNBFilesystemPkgsPkg* pkg = NBArray_itmPtrAtIndex(&opq->pkgs, STNBFilesystemPkgsPkg, i);
		 if(NBFilesPkg_moveFile(&pkg->pkg, srcFilepath, dstFilepath)){
		 r = TRUE;
		 break;
		 }
		 }*/
	} else if(parents != NULL && parentsSz > 0){
		//Call parent itf
		const IFilesystemItf* pItf = &parents[0];
		if(pItf->moveFile != NULL){
			r = (*pItf->moveFile)(pItf->obj, &parents[1], (parentsSz - 1), srcRoot, srcFilepath, dstRoot, dstFilepath);
		}
	}
	return r;
}

BOOL NBFilesystemPkgs_deleteFile_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* filepath){
	BOOL r = FALSE;
	//const STNBFilesystemPkgsOpq* opq = (const STNBFilesystemPkgsOpq*)obj;
	if(root == ENNBFilesystemRoot_PkgsDir){
		//Process explicit package "&###&..."
		//Or process all pakcages
		/*UI32 i; for(i = 0 ; i < opq->pkgs.use; i++){
		 STNBFilesystemPkgsPkg* pkg = NBArray_itmPtrAtIndex(&opq->pkgs, STNBFilesystemPkgsPkg, i);
		 if(NBFilesPkg_deleteFile(&pkg->pkg, filepath)){
		 r = TRUE;
		 break;
		 }
		 }*/
	} else if(parents != NULL && parentsSz > 0){
		//Call parent itf
		const IFilesystemItf* pItf = &parents[0];
		if(pItf->deleteFile != NULL){
			r = (*pItf->deleteFile)(pItf->obj, &parents[1], (parentsSz - 1), root, filepath);
		}
	}
	return r;
}

