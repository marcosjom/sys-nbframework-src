//
//  NBStorages.h
//  NBFramework
//
//  Created by Marcos Ortega on 1/3/19.
//

#ifndef NBStorages_h
#define NBStorages_h

#include "nb/NBFrameworkDefs.h"
//
#include "nb/core/NBStruct.h"
#include "nb/files/NBFilesystem.h"
#include "nb/storage/NBStorageCache.h"
#include "nb/crypto/NBSha1.h"

#ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	//If defined, every 'getStorageRecord*' and 'returnStorageRecord',
	//will build the json of the struct, calculate the sha-hash and
	//compare at return*-call to validate if 'pubModified' and
	//'privModified' are correctly set.
//#	define SSTORAGE_VALIDATE_RETURNED_STRUCTS_USING_JSON_SHA

	//If defined, every 'getStorageRecord*' and 'returnStorageRecord',
	//will calculate the crc32 of the struct data and compare at return*-call
	//to validate if 'pubModified' and 'privModified' are correctly set.
#	define SSTORAGE_VALIDATE_RETURNED_STRUCTS_USING_BIN_CRC32
#endif

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBStorageRecordRead_ {
		const STNBStorageCacheItmData*		data;
		struct {
			BOOL							pushed;
			STNBString						fullpath;
		} stack;
		struct {
			BOOL							hadData;
			STNBStorageCache*				container;
			const STNBStructMap*			pubMap;
			const STNBStructMap*			privMap;
#			if defined(SSTORAGE_VALIDATE_RETURNED_STRUCTS_USING_JSON_SHA) && defined(CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT)
			STNBSha1Hash					pubHash;	//to verify
			STNBSha1Hash					privHash;	//to verify
#			endif
#			if defined(SSTORAGE_VALIDATE_RETURNED_STRUCTS_USING_BIN_CRC32) && defined(CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT)
			STNBStructCrc					pubCrc;		//to verify
			STNBStructCrc					privCrc;	//to verify
#			endif
		} extra;
	} STNBStorageRecordRead;
	
	typedef struct STNBStorageRecordWrite_ {
		STNBStorageCacheItmData*		data;
		BOOL							pubModified;	//Must be manually set to TRUE
		BOOL							privModified;	//Must be manually set to TRUE
		struct {
			BOOL						pushed;
			STNBString					fullpath;
		} stack;
		struct {
			BOOL						hadData;
			STNBStorageCache*			container;
			const STNBStructMap*		pubMap;
			const STNBStructMap*		privMap;
#			if defined(SSTORAGE_VALIDATE_RETURNED_STRUCTS_USING_JSON_SHA) && defined(CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT)
			STNBSha1Hash				pubHash;	//to verify
			STNBSha1Hash				privHash;	//to verify
#			endif
#			if defined(SSTORAGE_VALIDATE_RETURNED_STRUCTS_USING_BIN_CRC32) && defined(CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT)
			STNBStructCrc				pubCrc;		//to verify
			STNBStructCrc				privCrc;	//to verify
#			endif
		} extra;
	} STNBStorageRecordWrite;
	
	typedef struct STNBStorages_ {
		void* opaque;
	} STNBStorages;
	
	
	void NBStorages_init(STNBStorages* obj);
	void NBStorages_retain(STNBStorages* obj);
	void NBStorages_release(STNBStorages* obj);
	
	//
	BOOL					NBStorages_setFilesystem(STNBStorages* obj, STNBFilesystem* fs);
	BOOL					NBStorages_setNotifMode(STNBStorages* obj, const ENNBStorageNotifMode mode);
	
	const char*				NBStorages_getRootRelative(const STNBStorages* obj);
	BOOL					NBStorages_setRootRelative(STNBStorages* obj, const char* root);
	
	void					NBStorages_notifyAll(STNBStorages* obj);
	
	//Records listeners
	void					NBStorages_addRecordListener(STNBStorages* obj, const char* storagePath, const char* recordPath, void* refObj, const STNBStorageCacheItmLstnr* methods);
	void					NBStorages_removeRecordListener(STNBStorages* obj, const char* storagePath, const char* recordPath, void* refObj);
	
	//Read record
	STNBStorageRecordRead	NBStorages_getStorageRecordForRead(STNBStorages* obj, const char* storagePath, const char* recordPath, const STNBStructMap* pubMap, const STNBStructMap* privMap);
	void					NBStorages_returnStorageRecordFromRead(STNBStorages* obj, STNBStorageRecordRead* ref);
	
	//Write record
	STNBStorageRecordWrite	NBStorages_getStorageRecordForWrite(STNBStorages* obj, const char* storagePath, const char* recordPath, const BOOL createIfNew, const STNBStructMap* pubMap, const STNBStructMap* privMap);
	void					NBStorages_returnStorageRecordFromWrite(STNBStorages* obj, STNBStorageRecordWrite* ref);
	
	//Touch record (trigger a notification to listeners without saving to file)
	void					NBStorages_touchRecord(STNBStorages* obj, const char* storagePath, const char* recordPath);
	
	//List
	void					NBStorages_getFiles(STNBStorages* obj, const char* storagePath, const BOOL includeStats, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);

	//Reset and delete
	void					NBStorages_resetAllStorages(STNBStorages* obj, const BOOL deleteFiles);

#ifdef __cplusplus
} //extern "C" 
#endif
		
#endif /* NBStorages_h */
