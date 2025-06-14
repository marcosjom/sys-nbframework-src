//
//  NBFilesystem.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/8/18.
//

#ifndef NBFilesystem_h
#define NBFilesystem_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/files/NBFile.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef enum ENNBFilesystemRoot_ {
		ENNBFilesystemRoot_WorkDir = 0,	//Explicit current working directory
		ENNBFilesystemRoot_PkgsDir,		//Explicit loaded packages only
		ENNBFilesystemRoot_AppBundle,	//Explicit app's bundle dir
		ENNBFilesystemRoot_Docs,		//Explicit document dir
		ENNBFilesystemRoot_Lib,			//Explicit library dir
		ENNBFilesystemRoot_Cache,		//Explicit cache dir
		//
		ENNBFilesystemRoot_Count
	} ENNBFilesystemRoot;
	
	typedef struct STNBFilesystemFile_ {
		UI32				name;		//The file's name start index.
		BOOL				isSymLink;	//The file or folder is a symbolic link.
		UI64				readTime;
		UI64				modTime;
		UI64				createTime;
		ENNBFilesystemRoot	root;		//The file's root: ENNBFilesystemRoot.
	} STNBFilesystemFile;
	
	typedef struct STNBFilesystem_ {
		void* opaque;
	} STNBFilesystem;

	typedef struct STNBFilesystemResult_ {
		//Folders
		struct {
			UI32	total;
			UI32	success;
			UI32	ignored;
			UI32	error;
		} foldersCount;
		//Files
		struct {
			UI32	total;
			UI32	success;
			UI32	ignored;
			UI32	error;
		} filesCount;
	} STNBFilesystemResult;
	
	typedef struct IFilesystemItf_ {
		void* obj;
		//
		void (*retain)(void* obj);
		void (*release)(void** obj);
		//
		BOOL (*concatRootPath)(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* fileName, STNBString* dst);
		//
		BOOL (*getFolders)(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
		BOOL (*folderExists)(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath);
		BOOL (*createFolder)(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath);
		BOOL (*deleteFolder)(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath);
		//
		BOOL (*open)(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* filepath, const ENNBFileMode mode, STNBFileRef dst);
		BOOL (*getFiles)(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath, const BOOL includeStats, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
		BOOL (*moveFile)(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot srcRoot, const char* srcFilepath, const ENNBFilesystemRoot dstRoot, const char* dstFilepath);
		BOOL (*deleteFile)(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* filepath);
	} IFilesystemItf;
	
	typedef void (*IFilesystemItfCreateFunc)(void* obj, IFilesystemItf* dst);
	
	//
	
	void NBFilesystem_init(STNBFilesystem* obj);
	void NBFilesystem_retain(STNBFilesystem* obj);
	void NBFilesystem_release(STNBFilesystem* obj);
	
	void NBFilesystem_createItf(void* obj, IFilesystemItf* dst);
	
	BOOL NBFilesystem_pushItf(STNBFilesystem* obj, IFilesystemItfCreateFunc createFunc, void* createObj);
	
	//Folders
	BOOL NBFilesystem_getFolders(const STNBFilesystem* obj, const char* folderpath, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
	BOOL NBFilesystem_getFoldersStrs(const STNBFilesystem* obj, const char** arr, const UI32 arrSz, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
	BOOL NBFilesystem_getFoldersStrsAndNull(const STNBFilesystem* obj, const char** arrAndNull, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
	BOOL NBFilesystem_getFoldersAtRoot(const STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* folderpath, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
	BOOL NBFilesystem_getFoldersStrsAtRoot(const STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arr, const UI32 arrSz, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
	BOOL NBFilesystem_getFoldersStrsAndNullAtRoot(const STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arrAndNull, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
	
	BOOL NBFilesystem_folderExists(const STNBFilesystem* obj, const char* folderpath);
	BOOL NBFilesystem_folderExistsStrs(const STNBFilesystem* obj, const char** arr, const UI32 arrSz);
	BOOL NBFilesystem_folderExistsStrsAndNull(const STNBFilesystem* obj, const char** arrAndNull);
	BOOL NBFilesystem_folderExistsAtRoot(const STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* folderpath);
	BOOL NBFilesystem_folderExistsStrsAtRoot(const STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arr, const UI32 arrSz);
	BOOL NBFilesystem_folderExistsStrsAndNullAtRoot(const STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arrAndNull);
	
	BOOL NBFilesystem_createFolder(STNBFilesystem* obj, const char* folderpath);
	BOOL NBFilesystem_createFolderStrs(STNBFilesystem* obj, const char** arr, const UI32 arrSz);
	BOOL NBFilesystem_createFolderStrsAndNull(STNBFilesystem* obj, const char** arrAndNull);
	BOOL NBFilesystem_createFolderAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* folderpath);
	BOOL NBFilesystem_createFolderStrsAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arr, const UI32 arrSz);
	BOOL NBFilesystem_createFolderStrsAndNullAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arrAndNull);
	
	BOOL NBFilesystem_deleteFolder(STNBFilesystem* obj, const char* folderpath);
	BOOL NBFilesystem_deleteFolderStrs(STNBFilesystem* obj, const char** arr, const UI32 arrSz);
	BOOL NBFilesystem_deleteFolderStrsAndNull(STNBFilesystem* obj, const char** arrAndNull);
	BOOL NBFilesystem_deleteFolderAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* folderpath);
	BOOL NBFilesystem_deleteFolderStrsAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arr, const UI32 arrSz);
	BOOL NBFilesystem_deleteFolderStrsAndNullAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arrAndNull);
	
	BOOL NBFilesystem_createFolderPath(STNBFilesystem* obj, const char* folderpath);
	BOOL NBFilesystem_createFolderPathStrs(STNBFilesystem* obj, const char** arr, const UI32 arrSz);
	BOOL NBFilesystem_createFolderPathStrsAndNull(STNBFilesystem* obj, const char** arrAndNull);
	BOOL NBFilesystem_createFolderPathAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* folderpath);
	BOOL NBFilesystem_createFolderPathStrsAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arr, const UI32 arrSz);
	BOOL NBFilesystem_createFolderPathStrsAndNullAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arrAndNull);

	//Last component (filename is ignored)
	BOOL NBFilesystem_createFolderPathOfFile(STNBFilesystem* obj, const char* folderpath);
	BOOL NBFilesystem_createFolderPathOfFileAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* folderpath);
	
	//
	BOOL NBFilesystem_concatRootPath(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* fileName, STNBString* dst);

	//Open
	BOOL NBFilesystem_open(STNBFilesystem* obj, const char* filepath, const ENNBFileMode mode, STNBFileRef dst);
	BOOL NBFilesystem_openStrs(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arr, const UI32 arrSz, const ENNBFileMode mode, STNBFileRef dst);
	BOOL NBFilesystem_openStrsAndNull(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arrAndNull, const ENNBFileMode mode, STNBFileRef dst);
	BOOL NBFilesystem_openAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* filepath, const ENNBFileMode mode, STNBFileRef dst);
	BOOL NBFilesystem_openStrsAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arr, const UI32 arrSz, const ENNBFileMode mode, STNBFileRef dst);
	BOOL NBFilesystem_openStrsAndNullAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arrAndNull, const ENNBFileMode mode, STNBFileRef dst);
	
	BOOL NBFilesystem_getFiles(const STNBFilesystem* obj, const char* folderpath, const BOOL includeStats, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
	BOOL NBFilesystem_moveFile(STNBFilesystem* obj, const char* srcFilepath, const char* dstFilepath);
	BOOL NBFilesystem_deleteFile(STNBFilesystem* obj, const char* filepath);
	STNBFilesystemResult NBFilesystem_deleteFilesAtFolder(STNBFilesystem* obj, const char* folderPath, const BOOL recursive);
	
	//Files (explicit root)
	BOOL NBFilesystem_getFilesAtRoot(const STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* folderpath, const BOOL includeStats, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
	BOOL NBFilesystem_moveFileAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot srcRoot, const char* srcFilepath, const ENNBFilesystemRoot dstRoot, const char* dstFilepath);
	BOOL NBFilesystem_deleteFileAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* filepath);
	STNBFilesystemResult NBFilesystem_deleteFilesAtFolderAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* folderPath, const BOOL recursive);
	
	//Write
	BOOL NBFilesystem_writeToFilepath(STNBFilesystem* obj, const char* filepath, void* data, const UI32 dataSz);
	BOOL NBFilesystem_writeToFilepathAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* filepath, void* data, const UI32 dataSz);
	BOOL NBFilesystem_writeToFilepathStrs(STNBFilesystem* obj, const char** arr, const UI32 arrSz, void* data, const UI32 dataSz);
	BOOL NBFilesystem_writeToFilepathStrsAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arr, const UI32 arrSz, void* data, const UI32 dataSz);
	BOOL NBFilesystem_writeToFilepathStrsAndNull(STNBFilesystem* obj, const char** arrAndNull, void* data, const UI32 dataSz);
	BOOL NBFilesystem_writeToFilepathStrsAndNullAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arrAndNull, void* data, const UI32 dataSz);
	
	//Read
	BOOL NBFilesystem_readFromFilepath(STNBFilesystem* obj, const char* filepath, STNBString* dst);
	BOOL NBFilesystem_readFromFilepathAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char* filepath, STNBString* dst);
	BOOL NBFilesystem_readFromFilepathStrs(STNBFilesystem* obj, const char** arr, const UI32 arrSz, STNBString* dst);
	BOOL NBFilesystem_readFromFilepathStrsAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arr, const UI32 arrSz, STNBString* dst);
	BOOL NBFilesystem_readFromFilepathStrsAndNull(STNBFilesystem* obj, const char** arrAndNull, STNBString* dst);
	BOOL NBFilesystem_readFromFilepathStrsAndNullAtRoot(STNBFilesystem* obj, const ENNBFilesystemRoot root, const char** arrAndNull, STNBString* dst);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBFilesystem_h */
