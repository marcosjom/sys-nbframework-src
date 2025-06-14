//
//  NBArray.h
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#ifndef NB_STRUCT_MAP_H
#define NB_STRUCT_MAP_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBCompare.h"
#include "nb/core/NBEnumMap.h"
#include "nb/core/NBStructMapMember.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	struct STNBStructMap_;
	
	//Enum
	
	const struct STNBStructMap_* NBEnumMapRecord_getSharedStructMap(void);
	const struct STNBStructMap_* NBEnumMap_getSharedStructMap(void);
	
	//Sign
	
	typedef enum ENNBStructMapSign_ {
		ENNBStructMapSign_Signed = 0,
		ENNBStructMapSign_Unsigned,
		ENNBStructMapSign_Count
	} ENNBStructMapSign;
	
	const STNBEnumMap* NBStructMapSign_getSharedEnumMap(void);
	
	//Member type (from NBStructMapMember.h)
	
	const STNBEnumMap* NBStructMapMemberType_getSharedEnumMap(void);
	
	//StructMap
	
	typedef struct STNBStructMap_ {
		UI32					stSize;
		STNBStructMapMember*	mbrs;
		UI16					mbrsSz;
		UI16					mbrsSzMem;
	} STNBStructMap;
	
	const STNBStructMap* NBStructMap_getSharedStructMap(void);
	
	//
	
	void NBStructMap_init(STNBStructMap* obj, const UI32 stSize);
	void NBStructMap_release(STNBStructMap* obj);
	
	//Embeded members (void)
#	define NBStructMap_addBoolM(OBJ, ST, MBR_NAME)				NBStructMap_addBool(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME))
#	define NBStructMap_addIntM(OBJ, ST, MBR_NAME)				NBStructMap_addInt(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME))
#	define NBStructMap_addUIntM(OBJ, ST, MBR_NAME)				NBStructMap_addUInt(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME))
#	define NBStructMap_addFloatM(OBJ, ST, MBR_NAME)				NBStructMap_addFloat(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME))
#	define NBStructMap_addDoubleM(OBJ, ST, MBR_NAME)			NBStructMap_addDouble(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME))
#	define NBStructMap_addCharsM(OBJ, ST, MBR_NAME)				NBStructMap_addChars(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME))[0])
#	define NBStructMap_addBytesM(OBJ, ST, MBR_NAME)				NBStructMap_addBytes(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME))
#	define NBStructMap_addStrPtrM(OBJ, ST, MBR_NAME)			NBStructMap_addStrPtr(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME))[0])
#	define NBStructMap_addEnumM(OBJ, ST, MBR_NAME, EN_MAP)		NBStructMap_addEnum(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), EN_MAP)
#	define NBStructMap_addStructM(OBJ, ST, MBR_NAME, ST_MAP)	NBStructMap_addStruct(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), ST_MAP)
	
	//Pointers to one element (void*)
#	define NBStructMap_addBoolPtrM(OBJ, ST, MBR_NAME)			NBStructMap_addBoolPtr(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(*((ST).MBR_NAME)))
#	define NBStructMap_addIntPtrM(OBJ, ST, MBR_NAME)			NBStructMap_addIntPtr(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(*((ST).MBR_NAME)))
#	define NBStructMap_addUIntPtrM(OBJ, ST, MBR_NAME)			NBStructMap_addUIntPtr(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(*((ST).MBR_NAME)))
#	define NBStructMap_addFloatPtrM(OBJ, ST, MBR_NAME)			NBStructMap_addFloatPtr(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(*((ST).MBR_NAME)))
#	define NBStructMap_addDoublePtrM(OBJ, ST, MBR_NAME)			NBStructMap_addDoublePtr(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(*((ST).MBR_NAME)))
#	define NBStructMap_addCharsPtrM(OBJ, ST, MBR_NAME)			NBStructMap_addCharsPtr(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(*((ST).MBR_NAME)), sizeof(((ST).MBR_NAME)[0][0]))
#	define NBStructMap_addBytesPtrM(OBJ, ST, MBR_NAME)			NBStructMap_addBytesPtr(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(*((ST).MBR_NAME)))
#	define NBStructMap_addStrPtrPtrM(OBJ, ST, MBR_NAME)			NBStructMap_addStrPtrPtr(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(*((ST).MBR_NAME)), sizeof(((ST).MBR_NAME)[0][0]))
#	define NBStructMap_addEnumPtrM(OBJ, ST, MBR_NAME, EN_MAP)	NBStructMap_addEnumPtr(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(*((ST).MBR_NAME)), EN_MAP)
#	define NBStructMap_addStructPtrM(OBJ, ST, MBR_NAME, ST_MAP)	NBStructMap_addStructPtr(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(*((ST).MBR_NAME)), ST_MAP)
	
	//Embeded array of elements ((void)[])
#	define NBStructMap_addArrayOfBoolM(OBJ, ST, MBR_NAME)			NBStructMap_addArrayOfBool(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]))
#	define NBStructMap_addArrayOfIntM(OBJ, ST, MBR_NAME)			NBStructMap_addArrayOfInt(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]))
#	define NBStructMap_addArrayOfUIntM(OBJ, ST, MBR_NAME)			NBStructMap_addArrayOfUInt(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]))
#	define NBStructMap_addArrayOfFloatM(OBJ, ST, MBR_NAME)			NBStructMap_addArrayOfFloat(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]))
#	define NBStructMap_addArrayOfDoubleM(OBJ, ST, MBR_NAME)			NBStructMap_addArrayOfDouble(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]))
#	define NBStructMap_addArrayOfCharsM(OBJ, ST, MBR_NAME)			NBStructMap_addArrayOfChars(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]), sizeof(((ST).MBR_NAME)[0][0]))
#	define NBStructMap_addArrayOfBytesM(OBJ, ST, MBR_NAME)			NBStructMap_addArrayOfBytes(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]))
#	define NBStructMap_addArrayOfStrPtrM(OBJ, ST, MBR_NAME)			NBStructMap_addArrayOfStrPtr(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]), sizeof(((ST).MBR_NAME)[0][0]))
#	define NBStructMap_addArrayOfEnumM(OBJ, ST, MBR_NAME, EN_MAP)	NBStructMap_addArrayOfEnum(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]), EN_MAP)
#	define NBStructMap_addArrayOfStructM(OBJ, ST, MBR_NAME, ST_MAP)	NBStructMap_addArrayOfStruct(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]), ST_MAP)
	
	//Embeded array of elements pointers ((void*)[])
#	define NBStructMap_addArrayOfBoolPtrsM(OBJ, ST, MBR_NAME)			NBStructMap_addArrayOfBoolPtrs(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]), sizeof(((ST).MBR_NAME)[0][0]))
#	define NBStructMap_addArrayOfIntPtrsM(OBJ, ST, MBR_NAME)			NBStructMap_addArrayOfIntPtrs(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]), sizeof(((ST).MBR_NAME)[0][0]))
#	define NBStructMap_addArrayOfUIntPtrsM(OBJ, ST, MBR_NAME)			NBStructMap_addArrayOfUIntPtrs(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]), sizeof(((ST).MBR_NAME)[0][0]))
#	define NBStructMap_addArrayOfFloatPtrsM(OBJ, ST, MBR_NAME)			NBStructMap_addArrayOfFloatPtrs(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]), sizeof(((ST).MBR_NAME)[0][0]))
#	define NBStructMap_addArrayOfDoublePtrsM(OBJ, ST, MBR_NAME)			NBStructMap_addArrayOfDoublePtrs(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]), sizeof(((ST).MBR_NAME)[0][0]))
#	define NBStructMap_addArrayOfCharsPtrsM(OBJ, ST, MBR_NAME)			NBStructMap_addArrayOfCharsPtrs(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]), sizeof(((ST).MBR_NAME)[0][0]), sizeof(((ST).MBR_NAME)[0][0][0]))
#	define NBStructMap_addArrayOfBytesPtrsM(OBJ, ST, MBR_NAME)			NBStructMap_addArrayOfBytesPtrs(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]), sizeof(((ST).MBR_NAME)[0][0]), sizeof(((ST).MBR_NAME)[0][0][0]))
#	define NBStructMap_addArrayOfStrPtrPtrsM(OBJ, ST, MBR_NAME)			NBStructMap_addArrayOfStrPtrPtrs(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]), sizeof(((ST).MBR_NAME)[0][0]), sizeof(((ST).MBR_NAME)[0][0][0]))
#	define NBStructMap_addArrayOfEnumPtrsM(OBJ, ST, MBR_NAME, EN_MAP)	NBStructMap_addArrayOfEnumPtrs(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]), sizeof(((ST).MBR_NAME)[0][0]), EN_MAP)
#	define NBStructMap_addArrayOfStructPtrsM(OBJ, ST, MBR_NAME, ST_MAP)	NBStructMap_addArrayOfStructPtrs(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]), sizeof(((ST).MBR_NAME)[0][0]), ST_MAP)

	//Pointer to variable-size array of elements pointers ((void*)[])
#	define NBStructMap_addPtrToArrayOfBoolM(OBJ, ST, MBR_NAME, MBR_ARR_SZ, SIGN)			NBStructMap_addPtrToArrayOfBool(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]), #MBR_ARR_SZ, &((ST).MBR_ARR_SZ), sizeof((ST).MBR_ARR_SZ), SIGN)
#	define NBStructMap_addPtrToArrayOfIntM(OBJ, ST, MBR_NAME, MBR_ARR_SZ, SIGN)				NBStructMap_addPtrToArrayOfInt(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]), #MBR_ARR_SZ, &((ST).MBR_ARR_SZ), sizeof((ST).MBR_ARR_SZ), SIGN)
#	define NBStructMap_addPtrToArrayOfUIntM(OBJ, ST, MBR_NAME, MBR_ARR_SZ, SIGN)			NBStructMap_addPtrToArrayOfUInt(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]), #MBR_ARR_SZ, &((ST).MBR_ARR_SZ), sizeof((ST).MBR_ARR_SZ), SIGN)
#	define NBStructMap_addPtrToArrayOfFloatM(OBJ, ST, MBR_NAME, MBR_ARR_SZ, SIGN)			NBStructMap_addPtrToArrayOfFloat(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]), #MBR_ARR_SZ, &((ST).MBR_ARR_SZ), sizeof((ST).MBR_ARR_SZ), SIGN)
#	define NBStructMap_addPtrToArrayOfDoubleM(OBJ, ST, MBR_NAME, MBR_ARR_SZ, SIGN)			NBStructMap_addPtrToArrayOfDouble(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]), #MBR_ARR_SZ, &((ST).MBR_ARR_SZ), sizeof((ST).MBR_ARR_SZ), SIGN)
#	define NBStructMap_addPtrToArrayOfCharsM(OBJ, ST, MBR_NAME, MBR_ARR_SZ, SIGN)			NBStructMap_addPtrToArrayOfChars(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]), #MBR_ARR_SZ, &((ST).MBR_ARR_SZ), sizeof((ST).MBR_ARR_SZ), SIGN)
#	define NBStructMap_addPtrToArrayOfBytesM(OBJ, ST, MBR_NAME, MBR_ARR_SZ, SIGN)			NBStructMap_addPtrToArrayOfBytes(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]), #MBR_ARR_SZ, &((ST).MBR_ARR_SZ), sizeof((ST).MBR_ARR_SZ), SIGN)
#	define NBStructMap_addPtrToArrayOfStrPtrM(OBJ, ST, MBR_NAME, MBR_ARR_SZ, SIGN)			NBStructMap_addPtrToArrayOfStrPtr(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]), sizeof(((ST).MBR_NAME)[0][0]), #MBR_ARR_SZ, &((ST).MBR_ARR_SZ), sizeof((ST).MBR_ARR_SZ), SIGN)
#	define NBStructMap_addPtrToArrayOfEnumM(OBJ, ST, MBR_NAME, MBR_ARR_SZ, SIGN, EN_MAP)	NBStructMap_addPtrToArrayOfEnum(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]), EN_MAP, #MBR_ARR_SZ, &((ST).MBR_ARR_SZ), sizeof((ST).MBR_ARR_SZ), SIGN)
#	define NBStructMap_addPtrToArrayOfStructM(OBJ, ST, MBR_NAME, MBR_ARR_SZ, SIGN, ST_MAP)	NBStructMap_addPtrToArrayOfStruct(OBJ, #MBR_NAME, &(ST), &((ST).MBR_NAME), sizeof((ST).MBR_NAME), sizeof(((ST).MBR_NAME)[0]), ST_MAP, #MBR_ARR_SZ, &((ST).MBR_ARR_SZ), sizeof((ST).MBR_ARR_SZ), SIGN)

	
	//Embeded members (void)
	BOOL NBStructMap_addBool(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz);
	BOOL NBStructMap_addInt(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz);
	BOOL NBStructMap_addUInt(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz);
	BOOL NBStructMap_addFloat(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz);
	BOOL NBStructMap_addDouble(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz);
	BOOL NBStructMap_addChars(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz);
	BOOL NBStructMap_addBytes(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz);
	BOOL NBStructMap_addStrPtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz);
	BOOL NBStructMap_addEnum(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const STNBEnumMap* enumMap);
	BOOL NBStructMap_addStruct(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const STNBStructMap* structMap);
	
	//Pointer to one element (void*)
	BOOL NBStructMap_addBoolPtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz);
	BOOL NBStructMap_addIntPtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz);
	BOOL NBStructMap_addUIntPtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz);
	BOOL NBStructMap_addFloatPtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz);
	BOOL NBStructMap_addDoublePtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz);
	BOOL NBStructMap_addCharsPtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz);
	BOOL NBStructMap_addBytesPtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz);
	BOOL NBStructMap_addStrPtrPtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz);
	BOOL NBStructMap_addEnumPtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const STNBEnumMap* enumMap);
	BOOL NBStructMap_addStructPtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const STNBStructMap* structMap);
	
	//Embeded array of elements (void[], implicit-size)
	BOOL NBStructMap_addArrayOfBool(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz);
	BOOL NBStructMap_addArrayOfInt(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz);
	BOOL NBStructMap_addArrayOfUInt(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz);
	BOOL NBStructMap_addArrayOfFloat(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz);
	BOOL NBStructMap_addArrayOfDouble(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz);
	BOOL NBStructMap_addArrayOfChars(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz);
	BOOL NBStructMap_addArrayOfBytes(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz);
	BOOL NBStructMap_addArrayOfStrPtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz);
	BOOL NBStructMap_addArrayOfEnum(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const STNBEnumMap* enumMap);
	BOOL NBStructMap_addArrayOfStruct(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr,const UI32 mbrSz, const UI32 elemSz, const STNBStructMap* structMap);
	
	//Embeded array of pointers to elements (void*[], implicit-size)
	BOOL NBStructMap_addArrayOfBoolPtrs(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz);
	BOOL NBStructMap_addArrayOfIntPtrs(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz);
	BOOL NBStructMap_addArrayOfUIntPtrs(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz);
	BOOL NBStructMap_addArrayOfFloatPtrs(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz);
	BOOL NBStructMap_addArrayOfDoublePtrs(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz);
	BOOL NBStructMap_addArrayOfCharsPtrs(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz, const UI32 elemElemElemSz);
	BOOL NBStructMap_addArrayOfBytesPtrs(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz, const UI32 elemElemElemSz);
	BOOL NBStructMap_addArrayOfStrPtrPtrs(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz, const UI32 elemElemElemSz);
	BOOL NBStructMap_addArrayOfEnumPtrs(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz, const STNBEnumMap* enumMap);
	BOOL NBStructMap_addArrayOfStructPtrs(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr,const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz, const STNBStructMap* structMap);
	
	//Pointer to array of elements (void* + explicit-size)
	BOOL NBStructMap_addPtrToArrayOfBool(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const char* szMbrName, const void* pSzmbrPtr, const UI32 szMbrSz, const ENNBStructMapSign szMbrSign);
	BOOL NBStructMap_addPtrToArrayOfInt(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const char* szMbrName, const void* pSzmbrPtr, const UI32 szMbrSz, const ENNBStructMapSign szMbrSign);
	BOOL NBStructMap_addPtrToArrayOfUInt(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const char* szMbrName, const void* pSzmbrPtr, const UI32 szMbrSz, const ENNBStructMapSign szMbrSign);
	BOOL NBStructMap_addPtrToArrayOfFloat(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const char* szMbrName, const void* pSzmbrPtr, const UI32 szMbrSz, const ENNBStructMapSign szMbrSign);
	BOOL NBStructMap_addPtrToArrayOfDouble(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const char* szMbrName, const void* pSzmbrPtr, const UI32 szMbrSz, const ENNBStructMapSign szMbrSign);
	BOOL NBStructMap_addPtrToArrayOfChars(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const char* szMbrName, const void* pSzmbrPtr, const UI32 szMbrSz, const ENNBStructMapSign szMbrSign);
	BOOL NBStructMap_addPtrToArrayOfBytes(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const char* szMbrName, const void* pSzmbrPtr, const UI32 szMbrSz, const ENNBStructMapSign szMbrSign);
	BOOL NBStructMap_addPtrToArrayOfStrPtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz, const char* szMbrName, const void* pSzmbrPtr, const UI32 szMbrSz, const ENNBStructMapSign szMbrSign);
	BOOL NBStructMap_addPtrToArrayOfEnum(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const STNBEnumMap* enumMap, const char* szMbrName, const void* pSzmbrPtr, const UI32 szMbrSz, const ENNBStructMapSign szMbrSign);
	BOOL NBStructMap_addPtrToArrayOfStruct(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr,const UI32 mbrSz, const UI32 elemSz, const STNBStructMap* structMap, const char* szMbrName, const void* pSzmbrPtr, const UI32 szMbrSz, const ENNBStructMapSign szMbrSign);
	
	//
	const STNBStructMapMember* NBStructMap_getMember(const STNBStructMap* obj, const char* name);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
