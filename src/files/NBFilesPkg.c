//
//  NBFilesPkg.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/8/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBString.h"
#include "nb/files/NBFilesystem.h"
#include "nb/files/NBFilesPkgIndex.h"
#include "nb/files/NBFilesPkg.h"
#include "nb/files/NBFile.h"

typedef struct STNBFilesPkgOpq_ {
	STNBString			filepath;
	STNBFileRef			file;
	STNBFilesPkgIndex	index;
	//File description
	UI32				idxPos;
	UI32				idxSz;
	UI32				dataPos;
	UI32				dataSz;
	UI32				retainCount;
} STNBFilesPkgOpq;

void NBFilesPkg_init(STNBFilesPkg* obj){
	STNBFilesPkgOpq* opq = obj->opaque = NBMemory_allocType(STNBFilesPkgOpq);
	NBMemory_setZeroSt(*opq, STNBFilesPkgOpq);
	NBString_init(&opq->filepath);
    opq->file       = NBFile_alloc(NULL);
    NBFilesPkgIndex_init(&opq->index);
	opq->idxPos		= 0;
	opq->idxSz		= 0;
	opq->dataPos	= 0;
	opq->dataSz		= 0;
	opq->retainCount = 1;
}

void NBFilesPkg_retain(STNBFilesPkg* obj){
	STNBFilesPkgOpq* opq = (STNBFilesPkgOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount++;
}
	
void NBFilesPkg_release(STNBFilesPkg* obj){
	STNBFilesPkgOpq* opq = (STNBFilesPkgOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount--;
	if(opq->retainCount == 0){
		NBString_release(&opq->filepath);
		NBFile_release(&opq->file);
        NBFile_null(&opq->file);
		NBFilesPkgIndex_release(&opq->index);
		opq->idxPos		= 0;
		opq->idxSz		= 0;
		opq->dataPos	= 0;
		opq->dataSz		= 0;
		NBMemory_free(opq);
		obj->opaque = NULL;
	}
}

//

/*BOOL NBFilesPkg_getFolders(STNBFilesPkg* obj, const char* folderpath, STNBString* dstStrs, STNBArray* dstFiles / *STNBFilesystemFile* /){
	BOOL r = FALSE;
	STNBFilesPkgOpq* opq = (STNBFilesPkgOpq*)obj->opaque;
	r = NBFilesPkgIndex_getFolders(&opq->index, folderpath, dstStrs, dstFiles);
	return r;
}*/

BOOL NBFilesPkg_getFiles(STNBFilesPkg* obj, const char* folderpath, const BOOL includeStats, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/){
	BOOL r = FALSE;
	STNBFilesPkgOpq* opq = (STNBFilesPkgOpq*)obj->opaque;
	r = NBFilesPkgIndex_getFiles(&opq->index, folderpath, includeStats, dstStrs, dstFiles);
	return r;
}

BOOL NBFilesPkg_fileExists(STNBFilesPkg* obj, const char* filepath){
	BOOL r = FALSE;
	STNBFilesPkgOpq* opq = (STNBFilesPkgOpq*)obj->opaque;
	r = NBFilesPkgIndex_fileExists(&opq->index, filepath);
	return r;
}

BOOL NBFilesPkg_openFile(STNBFilesPkg* obj, const char* filepath, STNBFileRef dstFile){
	BOOL r = FALSE;
	STNBFilesPkgOpq* opq = (STNBFilesPkgOpq*)obj->opaque;
	UI32 start = 0, size = 0;
	if(NBFilesPkgIndex_getFileRng(&opq->index, filepath, &start, &size)){
		if(NBFile_openAsFileRng(dstFile, opq->file, opq->dataPos + start, size)){
			IF_NBASSERT(NBFile_dbgSetPathRef(dstFile, filepath);)
			r = TRUE;
		}
	}
	return r;
}

//Files
/*BOOL NBFilesPkg_writeToFile(const STNBFilesPkg* obj, STNBFileRef file){
	
}*/

BOOL NBFilesPkg_loadFromFile(STNBFilesPkg* obj, STNBFileRef file){
	BOOL r = FALSE;
	STNBFilesPkgOpq* opq = (STNBFilesPkgOpq*)obj->opaque;
	if(NBFile_isOpen(opq->file)){
		PRINTF_ERROR("Pkg-file already loaded.\n");
	} else {
		NBFile_lock(file);
		SI32 idxSz = -1, dataSz = -1;
        NBFile_read(file, &idxSz, sizeof(idxSz));
		NBFile_read(file, &dataSz, sizeof(dataSz));
		if(idxSz < 0 || dataSz < 0){
			PRINTF_ERROR("Sizes iun package are not valid idx(%d bytes) data(%d bytes).\n", idxSz, dataSz);
		} else {
			if(!NBFilesPkgIndex_loadFromFile(&opq->index, file)){
				PRINTF_ERROR("Could not load package's index.\n");
			} else {
				const SI64 curPos = NBFile_curPos(file);
				if((curPos - sizeof(idxSz) - sizeof(dataSz)) != idxSz){
					PRINTF_ERROR("Index was not exact-readed.\n");
				} else {
                    NBFile_unlock(file);
					if(!NBFile_openAsFileRng(opq->file, file, (UI32)curPos - idxSz - sizeof(idxSz) - sizeof(dataSz), sizeof(idxSz) + sizeof(dataSz) + idxSz + dataSz)){
						PRINTF_ERROR("Could not open as subfile of package.\n");
					} else {
						NBString_empty(&opq->filepath);
						opq->idxPos				= (UI32)curPos - idxSz;
						opq->idxSz				= idxSz;
						opq->dataPos			= sizeof(idxSz) + sizeof(dataSz) + idxSz;
						opq->dataSz				= dataSz;
						//PRINTF_INFO("Package loaded from file pointer\n");
						r = TRUE;
					}
                    NBFile_lock(file);
				}
			}
		}
		NBFile_unlock(file);
	}
	return r;
}

BOOL NBFilesPkg_loadFromFilepath(STNBFilesPkg* obj, const char* filepath){
	BOOL r = FALSE;
	STNBFilesPkgOpq* opq = (STNBFilesPkgOpq*)obj->opaque;
	if(NBFile_isOpen(opq->file)){
		PRINTF_ERROR("Pkg-file already loaded, trying to load: '%s'.\n", filepath);
	} else {
		if(!NBFile_open(opq->file, filepath, ENNBFileMode_Read)){
			PRINTF_ERROR("Could not open pkg-file: '%s'.\n", filepath);
		} else {
			NBFile_lock(opq->file);
			SI32 idxSz = -1, dataSz = -1; NBFile_read(opq->file, &idxSz, sizeof(idxSz));
			NBFile_read(opq->file, &dataSz, sizeof(dataSz));
			if(idxSz < 0 || dataSz < 0){
				PRINTF_ERROR("Sizes iun package are not valid idx(%d bytes) data(%d bytes).\n", idxSz, dataSz);
			} else {
				if(!NBFilesPkgIndex_loadFromFile(&opq->index, opq->file)){
					PRINTF_ERROR("Could not load package's index.\n");
				} else {
					const SI64 curPos = NBFile_curPos(opq->file);
					if((curPos - sizeof(idxSz) - sizeof(dataSz)) != idxSz){
						PRINTF_ERROR("Index was not exact-readed.\n");
					} else {
						NBString_set(&opq->filepath, filepath);
						opq->idxPos				= (UI32)curPos - idxSz;
						opq->idxSz				= idxSz;
						opq->dataPos			= sizeof(idxSz) + sizeof(dataSz) + idxSz;
						opq->dataSz				= dataSz;
						PRINTF_INFO("Package loaded: '%s'\n", filepath);
						r = TRUE;
					}
				}
			}
			NBFile_unlock(opq->file);
		}
	}
	return r;
}
