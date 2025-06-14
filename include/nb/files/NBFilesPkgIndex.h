//
//  NBFilesPkgIndex.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/8/18.
//

#ifndef NBFilesPkgIndex_h
#define NBFilesPkgIndex_h

#include "nb/NBFrameworkDefs.h"
#include "nb/files/NBFile.h"
#include "nb/files/NBFilesystem.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBFilesPkgIndex_ {
		void* opaque;
	} STNBFilesPkgIndex;
	
	void NBFilesPkgIndex_init(STNBFilesPkgIndex* obj);
	void NBFilesPkgIndex_retain(STNBFilesPkgIndex* obj);
	void NBFilesPkgIndex_release(STNBFilesPkgIndex* obj);
	
	UI32 NBFilesPkgIndex_getRecordId(STNBFilesPkgIndex* obj, const char* path);
	//BOOL NBFilesPkgIndex_getFolders(STNBFilesPkgIndex* obj, const char* folderpath, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
	BOOL NBFilesPkgIndex_getFiles(STNBFilesPkgIndex* obj, const char* folderpath, const BOOL includeStats, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
	BOOL NBFilesPkgIndex_getFileRng(STNBFilesPkgIndex* obj, const char* filepath, UI32* dstStart, UI32* dstSize);
	BOOL NBFilesPkgIndex_fileExists(STNBFilesPkgIndex* obj, const char* filepath);
	
	BOOL NBFilesPkgIndex_addFile(STNBFilesPkgIndex* obj, const char* filepath, const UI32 start, const UI32 size);
	
	//Files
	BOOL NBFilesPkgIndex_writeToFile(const STNBFilesPkgIndex* obj, STNBFileRef file);
	BOOL NBFilesPkgIndex_loadFromFile(STNBFilesPkgIndex* obj, STNBFileRef file);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBFilesPkgIndex_h */
