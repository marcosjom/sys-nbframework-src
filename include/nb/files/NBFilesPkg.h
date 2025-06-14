//
//  NBFilesPkg.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/8/18.
//

#ifndef NBFilesPkg_h
#define NBFilesPkg_h

#include "nb/NBFrameworkDefs.h"
#include "nb/files/NBFile.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBFilesPkg_ {
		void* opaque;
	} STNBFilesPkg;
	
	void NBFilesPkg_init(STNBFilesPkg* obj);
	void NBFilesPkg_retain(STNBFilesPkg* obj);
	void NBFilesPkg_release(STNBFilesPkg* obj);
	
	//BOOL NBFilesPkg_getFolders(STNBFilesPkg* obj, const char* folderpath, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
	BOOL NBFilesPkg_getFiles(STNBFilesPkg* obj, const char* folderpath, const BOOL includeStats, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
	BOOL NBFilesPkg_fileExists(STNBFilesPkg* obj, const char* filepath);
	BOOL NBFilesPkg_openFile(STNBFilesPkg* obj, const char* filepath, STNBFileRef dstFile);
	
	//Files
	//BOOL NBFilesPkg_writeToFile(const STNBFilesPkg* obj, STNBFileRef file);
	BOOL NBFilesPkg_loadFromFile(STNBFilesPkg* obj, STNBFileRef file);
	BOOL NBFilesPkg_loadFromFilepath(STNBFilesPkg* obj, const char* filepath);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBFilesPkg_h */
