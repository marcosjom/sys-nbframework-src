//
//  NBArray.h
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#ifndef NB_STRUCT_H
#define NB_STRUCT_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBJson.h"
#include "nb/files/NBFile.h"
#include "nb/files/NBFilesystem.h"
#include "nb/crypto/NBCrc32.h"

#define NBStruct_stGetMemberValueM(OBJ_PTR, MBR_NAME, STRUCT_MAP)		NBStruct_stGetMemberValue(STRUCT_MAP, #MBR_NAME, OBJ_PTR, sizeof(*(OBJ_PTR)), sizeof((OBJ_PTR)->MBR_NAME))
#define NBStruct_stEmptyMemberValueM(OBJ_PTR, MBR_NAME, STRUCT_MAP)		NBStruct_stEmptyMemberValue(STRUCT_MAP, #MBR_NAME, OBJ_PTR, sizeof(*(OBJ_PTR)), sizeof((OBJ_PTR)->MBR_NAME))
#define NBStruct_stAddItmToArrayMemberM(OBJ_PTR, MBR_NAME, NEW_VALUE, STRUCT_MAP) NBStruct_stAddItmToArrayMember(STRUCT_MAP, #MBR_NAME, OBJ_PTR, sizeof(*(OBJ_PTR)), sizeof((OBJ_PTR)->MBR_NAME), &(NEW_VALUE), sizeof(NEW_VALUE))
#ifdef __cplusplus
extern "C" {
#endif
	
	//CRC checksum
	typedef struct STNBStructCrc_ {
		UI32 crc32;
		UI32 bytesFed;
	} STNBStructCrc;
	
	//Concat format
	typedef struct STNBStructConcatFormat_ {
		BOOL	namesInNewLine;		//All names starts in a new line
		BOOL	valuesInNewLine;	//All values starts in new line
		BOOL	objectsInNewLine;	//All objects starts in new line
		BOOL	ignoreZeroValues;	//Ignore native members with value zero
		BOOL	escapeOnlyDQuotes;	//Only double quotes wil be scaped
		UI32	tabStartLvl;		//Initial level of tabulation
		char*	tabChar;			//Character to be repeated per level of tabulation
		UI32	tabCharLen;			//Len in bytes
	} STNBStructConcatFormat;
	
	//Verify
	STNBStructCrc NBStruct_stCalculateCrc(const STNBStructMap* structMap, const void* src, const UI32 srcSz);
	BOOL NBStruct_stIsEqualByCrc(const STNBStructMap* structMap, const void* src, const UI32 srcSz, const void* src2, const UI32 src2Sz);
	
	//Clone
	BOOL NBStruct_stClone(const STNBStructMap* structMap, const void* src, const UI32 srcSz, void* dst, const UI32 dstSz);
	
	//Release
	BOOL NBStruct_stRelease(const STNBStructMap* structMap, void* src, const UI32 srcSz);

	//Release only structs
	BOOL NBStruct_stReleaseInternalStructs(const STNBStructMap* structMap, void* src, const UI32 srcSz, const STNBStructMap* structMapFilter);
	
	//Load from file
	BOOL NBStruct_stReadFromJsonFilepath(const char* filepath, const STNBStructMap* structMap, void* dst, const UI32 dstSz);
	BOOL NBStruct_stReadFromJsonFilepathAtRoot(STNBFilesystem* fs, const ENNBFilesystemRoot root, const char* filepath, const STNBStructMap* structMap, void* dst, const UI32 dstSz);
	BOOL NBStruct_stReadFromJsonFilepathStrsAndNullAtRoot(STNBFilesystem* fs, const ENNBFilesystemRoot root, const char** arrAndNull, const STNBStructMap* structMap, void* dst, const UI32 dstSz);
	BOOL NBStruct_stReadFromJsonFile(STNBFileRef file, const STNBStructMap* structMap, void* dst, const UI32 dstSz);
	BOOL NBStruct_stReadFromJsonNode(STNBJson* json, const STNBJsonNode* pNode, const STNBStructMap* structMap, void* dst, const UI32 dstSz);

	//Load from string
	BOOL NBStruct_stReadFromJsonStr(const char* str, const UI32 strSz, const STNBStructMap* structMap, void* dst, const UI32 dstSz);
	BOOL NBStruct_stReadFromJsonBase64Str(const char* str, const UI32 strSz, const STNBStructMap* structMap, void* dst, const UI32 dstSz);
	
	//Concat to string
	BOOL NBStruct_stConcatAsJson(STNBString* dst, const STNBStructMap* structMap, const void* src, const UI32 srcSz);
	BOOL NBStruct_stConcatAsJsonWithFormat(STNBString* dst, const STNBStructMap* structMap, const void* src, const UI32 srcSz, const STNBStructConcatFormat* format);
	BOOL NBStruct_stConcatAsJsonBase64(STNBString* dst, const STNBStructMap* structMap, const void* src, const UI32 srcSz);
	BOOL NBStruct_stConcatAsJsonBase64WithFormat(STNBString* dst, const STNBStructMap* structMap, const void* src, const UI32 srcSz, const STNBStructConcatFormat* format);
	
	//Write
	BOOL NBStruct_stWriteToJsonFilepath(const char* filepath, const STNBStructMap* structMap, const void* src, const UI32 srcSz);
	BOOL NBStruct_stWriteToJsonFilepathWithFormat(const char* filepath, const STNBStructMap* structMap, const void* src, const UI32 srcSz, const STNBStructConcatFormat* format);
	BOOL NBStruct_stWriteToJsonFilepathAtRoot(STNBFilesystem* fs, const ENNBFilesystemRoot root, const char* filepath, const STNBStructMap* structMap, const void* src, const UI32 srcSz);
	BOOL NBStruct_stWriteToJsonFilepathAtRootWithFormat(STNBFilesystem* fs, const ENNBFilesystemRoot root, const char* filepath, const STNBStructMap* structMap, const void* src, const UI32 srcSz, const STNBStructConcatFormat* format);
	BOOL NBStruct_stWriteToJsonFilepathStrsAndNullAtRoot(STNBFilesystem* fs, const ENNBFilesystemRoot root, const char** arrAndNull, const STNBStructMap* structMap, const void* src, const UI32 srcSz);
	BOOL NBStruct_stWriteToJsonFilepathStrsAndNullAtRootWithFormat(STNBFilesystem* fs, const ENNBFilesystemRoot root, const char** arrAndNull, const STNBStructMap* structMap, const void* src, const UI32 srcSz, const STNBStructConcatFormat* format);
	BOOL NBStruct_stWriteToJsonFile(STNBFileRef file, const STNBStructMap* structMap, const void* src, const UI32 srcSz);
	BOOL NBStruct_stWriteToJsonFileWithFormat(STNBFileRef file, const STNBStructMap* structMap, const void* src, const UI32 srcSz, const STNBStructConcatFormat* format);
	
	//Individual tasks
	void* NBStruct_stGetMemberValue(const STNBStructMap* obj, const char* name, const void* src, const UI32 srcSz, const UI32 mbrSz);
	void* NBStruct_stEmptyMemberValue(const STNBStructMap* obj, const char* name, void* src, const UI32 srcSz, const UI32 mbrSz);
	BOOL  NBStruct_stAddItmToArrayMember(const STNBStructMap* obj, const char* name, void* src, const UI32 srcSz, const UI32 mbrSz, void* newItm, const UI32 newItmSz);
	
	//Test
	BOOL NBStruct_test(void);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
