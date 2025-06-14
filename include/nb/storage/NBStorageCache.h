//
//  NBStorageCache.h
//  NBFramework
//
//  Created by Marcos Ortega on 1/3/19.
//

#ifndef NBStorageCache_h
#define NBStorageCache_h

#include "nb/NBFrameworkDefs.h"
#include "nb/files/NBFilesystem.h"

//

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef enum ENNBStorageNotifMode_ {
		ENNBStorageNotifMode_Inmediate = 0,	//Notify listeners inmediatly
		ENNBStorageNotifMode_Manual,		//Accum notifications untill expicit sent
		ENNBStorageNotifMode_Count
	} ENNBStorageNotifMode;
	
	typedef struct STNBStorageCacheItmLstnr_ {
		void	(*retain)(void* param);
		void	(*release)(void* param);
		void	(*recordChanged)(void* param, const char* relRoot, const char* relPath);
	} STNBStorageCacheItmLstnr;
	
	typedef void (*NBStorageCacheNotifyAddMethod)(void* pOpq, const char* pRootPath, const char* pRelPath, void* refObj, STNBStorageCacheItmLstnr* methods);
	
	typedef struct STNBStorageCacheItmDataPart_ {
		UI64		seq;	//Sequential (specific, public or private modifications)
		UI64		modf;	//UTC timestamp
		void*		stData;
	} STNBStorageCacheItmDataPart;
	
	typedef struct STNBStorageCacheItmData_ {
		UI64		seq;	//Sequential (public and private modifications)
		UI64		made;	//UTC timestamp
		UI64		modf;	//UTC timestamp
		STNBStorageCacheItmDataPart pub;	//public part (can be synced)
		STNBStorageCacheItmDataPart priv;	//private part
	} STNBStorageCacheItmData;
	
	//
	
	typedef struct STNBStorageCache_ {
		void* opaque;
	} STNBStorageCache;
	
	void NBStorageCache_init(STNBStorageCache* obj);
	void NBStorageCache_retain(STNBStorageCache* obj);
	void NBStorageCache_release(STNBStorageCache* obj);
	
	BOOL NBStorageCache_setFilesystem(STNBStorageCache* obj, STNBFilesystem* fs);
	BOOL NBStorageCache_setNotifMode(STNBStorageCache* obj, const ENNBStorageNotifMode mode, NBStorageCacheNotifyAddMethod method, void* methodParam);
	BOOL NBStorageCache_setRoot(STNBStorageCache* obj, const char* parentRoot, const char* fullRoot);
	
	//Listeners
	void NBStorageCache_addListener(STNBStorageCache* obj, const char* relPath, void* refObj, const STNBStorageCacheItmLstnr* methods);
	void NBStorageCache_removeListener(STNBStorageCache* obj, const char* relPath, void* refObj);
	
	//Read
	const STNBStorageCacheItmData* NBStorageCache_getRecordLockedForRead(STNBStorageCache* obj, const char* relPath, const STNBStructMap* pubMap, const STNBStructMap* privMap, const BOOL createIfFileExists, const BOOL createIfNew);
	void NBStorageCache_returnRecordLockedFromRead(STNBStorageCache* obj, const STNBStorageCacheItmData* data);
	
	//Write
	STNBStorageCacheItmData* NBStorageCache_getRecordLockedForWrite(STNBStorageCache* obj, const char* relPath, const STNBStructMap* pubMap, const STNBStructMap* privMap, const BOOL createIfFileExists, const BOOL createIfNew);
	void NBStorageCache_returnRecordLockedFromWrite(STNBStorageCache* obj, STNBStorageCacheItmData* data, const BOOL pubModified, const BOOL privModified);
	
	//Touch record (trigger a notification to listeners without saving to file)
	void NBStoragesCache_touchRecord(STNBStorageCache* obj, const char* relPath);
	
	void NBStoragesCache_resetAllRecords(STNBStorageCache* obj, const BOOL deleteFiles);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBStorageCache_h */
