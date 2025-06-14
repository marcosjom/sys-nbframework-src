//
//  NBFilesystemPkgs.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/9/18.
//

#ifndef NBFilesystemPkgs_h
#define NBFilesystemPkgs_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/files/NBFilesystem.h"

#ifdef __cplusplus
extern "C" {
#endif
	
typedef struct STNBFilesystemPkgs_ {
	void* opaque;
} STNBFilesystemPkgs;

void NBFilesystemPkgs_init(STNBFilesystemPkgs* obj);
void NBFilesystemPkgs_retain(STNBFilesystemPkgs* obj);
void NBFilesystemPkgs_release(STNBFilesystemPkgs* obj);

//
void NBFilesystemPkgs_createItf(void* obj, IFilesystemItf* dst);

//Pkgs
BOOL NBFilesystemPkgs_addPkgFromPath(STNBFilesystemPkgs* obj, const ENNBFilesystemRoot root, const char* filepath);
BOOL NBFilesystemPkgs_addPkgFromPersistentData(STNBFilesystemPkgs* obj, const char* pkgName, const BYTE* data, const UI64 dataSz);
BOOL NBFilesystemPkgs_isPkgLoaded(STNBFilesystemPkgs* obj, const ENNBFilesystemRoot root, const char* filepath);

//Folders
BOOL NBFilesystemPkgs_getFolders(const STNBFilesystemPkgs* obj, const char* folderpath, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
BOOL NBFilesystemPkgs_folderExists(const STNBFilesystemPkgs* obj, const char* folderpath);
BOOL NBFilesystemPkgs_createFolder(STNBFilesystemPkgs* obj, const char* folderpath);

//Files
BOOL NBFilesystemPkgs_open(STNBFilesystemPkgs* obj, const char* filepath, const ENNBFileMode mode, STNBFileRef dst);
BOOL NBFilesystemPkgs_getFiles(const STNBFilesystemPkgs* obj, const char* folderpath, const BOOL includeStats, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
BOOL NBFilesystemPkgs_moveFile(STNBFilesystemPkgs* obj, const char* srcFilepath, const ENNBFilesystemRoot dstRoot, const char* dstFilepath);
BOOL NBFilesystemPkgs_deleteFile(STNBFilesystemPkgs* obj, const char* filepath);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBFilesystemPkgs_h */
