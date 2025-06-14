//
//  XUCadena.c
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBMngrStructMaps.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBMemory.h"

//NBEnumMapRecord

STNBStructMapsRec NBEnumMapRecord_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBEnumMapRecord_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBEnumMapRecord_sharedStructMap);
	if(NBEnumMapRecord_sharedStructMap.map == NULL){
		STNBEnumMapRecord s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBEnumMapRecord);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addIntM(map, s, intValue);
		NBStructMap_addStrPtrM(map, s, varName);
		NBStructMap_addStrPtrM(map, s, strValue);
		NBEnumMapRecord_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBEnumMapRecord_sharedStructMap);
	return NBEnumMapRecord_sharedStructMap.map;
}

//NBEnumMap

STNBStructMapsRec NBEnumMap_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBEnumMap_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBEnumMap_sharedStructMap);
	if(NBEnumMap_sharedStructMap.map == NULL){
		STNBEnumMap s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBEnumMap);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addStrPtrM(map, s, enumName);
		NBStructMap_addPtrToArrayOfStructM(map, s, records, recordsSz, ENNBStructMapSign_Unsigned, NBEnumMapRecord_getSharedStructMap());
		NBEnumMap_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBEnumMap_sharedStructMap);
	return NBEnumMap_sharedStructMap.map;
}

//
//ENNBStructMapSign
//

STNBEnumMapRecord NBStructMapSign_sharedEnumMapRecs[] = {
	{ ENNBStructMapSign_Signed, "ENNBStructMapSign_Signed", "s" }
	, { ENNBStructMapSign_Unsigned, "ENNBStructMapSign_Unsigned", "u" }
};

STNBEnumMap NBStructMapSign_sharedEnumMap = {
	"ENNBStructMapSign"
	, NBStructMapSign_sharedEnumMapRecs
	, (sizeof(NBStructMapSign_sharedEnumMapRecs) / sizeof(NBStructMapSign_sharedEnumMapRecs[0]))
};

const STNBEnumMap* NBStructMapSign_getSharedEnumMap(void){
	return &NBStructMapSign_sharedEnumMap;
}

//ENNBStructMapMemberType

const STNBEnumMap* NBStructMapMemberType_getSharedEnumMap(void);

STNBEnumMapRecord NBStructMapMemberType_sharedEnumMapRecs[] = {
	{ ENNBStructMapMemberType_Bool, "ENNBStructMapMemberType_Bool", "b" }
	, { ENNBStructMapMemberType_Int, "ENNBStructMapMemberType_Int", "i" }
	, { ENNBStructMapMemberType_UInt, "ENNBStructMapMemberType_UInt", "u" }
	, { ENNBStructMapMemberType_Float, "ENNBStructMapMemberType_Float", "f" }
	, { ENNBStructMapMemberType_Double, "ENNBStructMapMemberType_Double", "d" }
	, { ENNBStructMapMemberType_Chars, "ENNBStructMapMemberType_Chars", "cs" }
	, { ENNBStructMapMemberType_Bytes, "ENNBStructMapMemberType_Bytes", "bs" }
	, { ENNBStructMapMemberType_StrPtr, "ENNBStructMapMemberType_StrPtr", "str" }
	, { ENNBStructMapMemberType_Enum, "ENNBStructMapMemberType_Enum", "en" }
	, { ENNBStructMapMemberType_Struct, "ENNBStructMapMemberType_Struct", "st" }
};

STNBEnumMap NBStructMapMemberType_sharedEnumMap = {
	"ENNBStructMapMemberType"
	, NBStructMapMemberType_sharedEnumMapRecs
	, (sizeof(NBStructMapMemberType_sharedEnumMapRecs) / sizeof(NBStructMapMemberType_sharedEnumMapRecs[0]))
};

const STNBEnumMap* NBStructMapMemberType_getSharedEnumMap(void){
	return &NBStructMapMemberType_sharedEnumMap;
}

//Private maps

STNBStructMapsRec NBStructMapMemberElem_sharedStructMap = STNBStructMapsRec_empty;
STNBStructMapsRec NBStructMapMemberData_sharedStructMap = STNBStructMapsRec_empty;
STNBStructMapsRec NBStructMapMember_sharedStructMap = STNBStructMapsRec_empty;

//NBStructMap map

STNBStructMapsRec NBStructMap_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBStructMap_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBStructMap_sharedStructMap);
	if(NBStructMap_sharedStructMap.map == NULL){
		STNBStructMap* map			= NBMngrStructMaps_allocTypeM(STNBStructMap);
		STNBStructMap* mapMbr		= NBMngrStructMaps_allocTypeM(STNBStructMapMember);
		STNBStructMap* mapMbrElem	= NBMngrStructMaps_allocTypeM(STNBStructMapMemberElem);
		STNBStructMap* mapMbrData	= NBMngrStructMaps_allocTypeM(STNBStructMapMemberData);
		STNBStructMap s;
		STNBStructMapMember sMbr;
		STNBStructMapMemberElem sMbrElem;
		STNBStructMapMemberData sMbrData;
		NBStructMap_init(map, sizeof(s));
		NBStructMap_init(mapMbr, sizeof(sMbr));
		NBStructMap_init(mapMbrElem, sizeof(sMbrElem));
		NBStructMap_init(mapMbrData, sizeof(sMbrData));
		//STNBStructMap
		{
			NBStructMap_addUIntM(map, s, stSize);
			NBStructMap_addPtrToArrayOfStructM(map, s, mbrs, mbrsSz, ENNBStructMapSign_Unsigned, mapMbr);
			//NBStructMap_addUIntM(map, s, mbrsSzMem); //Not required
		}
		//STNBStructMapMember
		{
			NBStructMap_addStrPtrM(mapMbr, sMbr, name);
			NBStructMap_addUIntM(mapMbr, sMbr, iPos);
			NBStructMap_addUIntM(mapMbr, sMbr, size);
			NBStructMap_addStructM(mapMbr, sMbr, elem, mapMbrElem);
			NBStructMap_addStructM(mapMbr, sMbr, data, mapMbrData);
		}
		//STNBStructMapMemberElem
		{
			NBStructMap_addBoolM(mapMbrElem, sMbrElem, isPtr);
			NBStructMap_addUIntM(mapMbrElem, sMbrElem, count);
			NBStructMap_addStructPtrM(mapMbrElem, sMbrElem, vCount, mapMbr);
		}
		//STNBStructMapMemberData
		{
			NBStructMap_addEnumM(mapMbrData, sMbrData, type, NBStructMapMemberType_getSharedEnumMap());
			NBStructMap_addUIntM(mapMbrData, sMbrData, size);
			NBStructMap_addStructPtrM(mapMbrData, sMbrData, enMap, NBEnumMap_getSharedStructMap());
			NBStructMap_addStructPtrM(mapMbrData, sMbrData, stMap, map);
		}
		NBStructMap_sharedStructMap.map				= map;
		NBStructMapMember_sharedStructMap.map		= mapMbr;		//private
		NBStructMapMemberElem_sharedStructMap.map	= mapMbrElem;	//private
		NBStructMapMemberData_sharedStructMap.map	= mapMbrData;	//private
	}
	NBMngrStructMaps_unlock(&NBStructMap_sharedStructMap);
	return NBStructMap_sharedStructMap.map;
}

//Struct's map

void NBStructMap_init(STNBStructMap* obj, const UI32 stSize){
	NBMemory_set(obj, 0, sizeof(*obj));
	obj->stSize	= stSize;
}

void NBStructMap_release(STNBStructMap* obj){
	if(obj->mbrs != NULL){
		UI32 i; for(i = 0; i < obj->mbrsSz; i++){
			STNBStructMapMember* m = &obj->mbrs[i];
			NBStructMapMember_release(m);
		}
		NBMemory_free(obj->mbrs);
		obj->mbrs	= NULL;
		obj->mbrsSz	= 0;
		obj->mbrsSzMem	= 0;
	}
    obj->stSize = 0;
}

//

BOOL NBStructMap_hasMemberNamed(STNBStructMap* obj, const char* name){
	BOOL r = FALSE;
	UI32 i; for(i = 0; i < obj->mbrsSz; i++){
		STNBStructMapMember* m = &obj->mbrs[i];
		if(NBStructMapMember_strIsEqual(name, m->name)){
			r = TRUE;
			break;
		} else if(m->elem.vCount != NULL){
			if(NBStructMapMember_strIsEqual(name, m->elem.vCount->name)){
				r = TRUE;
				break;
			}
		}
	}
	return r;
}

//Members

BOOL NBStructMap_addMember_(STNBStructMap* obj, const ENNBStructMapMemberType type, const BYTE* pStPtr, const char* mbrName, const BYTE* pMbrPtr, const UI32 mbrSz, const BOOL elemArePtrs, const UI32 dataSz, const STNBEnumMap* enMap, const STNBStructMap* stMap, const char* szMbrName, const void* pSzmbrPtr, const UI32 szMbrSz, const ENNBStructMapSign szMbrSign){
	BOOL r = FALSE;
	const char* stPtr		= (const char*)pStPtr;
	const char* mbrPtr		= (const char*)pMbrPtr;
	if(stPtr > mbrPtr){
		NBASSERT(FALSE) //The member is out of range
	} else {
		const UI32 iPos = (UI32)(mbrPtr - stPtr);
		if(iPos >= obj->stSize || (iPos + mbrSz) > obj->stSize){
			NBASSERT(FALSE) //The member is out of range
		} else if(NBStructMap_hasMemberNamed(obj, mbrName)){
			NBASSERT(FALSE) //Name collision (every name must be unique for the json saving/loading)
		} else if(type == ENNBStructMapMemberType_Enum && enMap == NULL){
			NBASSERT(FALSE) //Required enMap
		} else if(type == ENNBStructMapMemberType_Struct && stMap == NULL){
			NBASSERT(FALSE) //Required stMap
		} else {
			STNBStructMapMember m;
			NBStructMapMember_init(&m, mbrName);
			//member
			m.iPos			= iPos;
			m.size			= mbrSz;
			r				= TRUE;
			const UI32 eSz	= (elemArePtrs ? sizeof(void*) : dataSz);
			if((mbrSz % eSz) != 0){
				r = FALSE; NBASSERT(FALSE) //Member size error
			} else {
				//elements
				m.elem.isPtr	= elemArePtrs;
				m.elem.count	= (mbrSz / eSz);
				//data
				m.data.type		= type; NBASSERT((type != ENNBStructMapMemberType_Enum || enMap != NULL) && (type != ENNBStructMapMemberType_Struct || stMap != NULL))
				m.data.size		= dataSz;
				{
					//Union
					if(enMap != NULL) m.data.enMap = enMap;
					if(stMap != NULL) m.data.stMap = stMap;
				}
				if(szMbrName != NULL){
					const char* mbrPtr = (const char*)pSzmbrPtr;
					if(stPtr > mbrPtr){
						r = FALSE; NBASSERT(FALSE) //The member is out of range
					} else {
						const UI32 iPos = (UI32)(mbrPtr - stPtr);
						if(iPos >= obj->stSize || (iPos + szMbrSz) > obj->stSize){
							r = FALSE; NBASSERT(FALSE) //The member is out of range
						} else if(NBStructMap_hasMemberNamed(obj, szMbrName)){
							r = FALSE; NBASSERT(FALSE) //Name collision (every name must be unique for the json saving/loading)
						} else if(szMbrSign != ENNBStructMapSign_Signed && szMbrSign != ENNBStructMapSign_Unsigned){
							r = FALSE; NBASSERT(FALSE) //Invalid sign
						} else {
							STNBStructMapMember* vc = NBMemory_allocType(STNBStructMapMember);
							NBStructMapMember_init(vc, szMbrName);
							vc->iPos		= iPos;
							vc->size		= szMbrSz;
							vc->data.type	= (szMbrSign == ENNBStructMapSign_Signed ? ENNBStructMapMemberType_Int : ENNBStructMapMemberType_UInt);
							vc->data.size	= szMbrSz;
							m.elem.vCount	= vc;
						}
					}
				}
			}
			//Add to array
			if(r){
				//Increase array
				if(obj->mbrsSz == obj->mbrsSzMem){
					obj->mbrsSzMem += 4;
					STNBStructMapMember* newArr = NBMemory_allocTypes(STNBStructMapMember, obj->mbrsSzMem);
					if(obj->mbrs != NULL){
						NBMemory_copy(newArr, obj->mbrs, (sizeof(STNBStructMapMember) * obj->mbrsSz));
						NBMemory_free(obj->mbrs);
					}
					obj->mbrs = newArr;
				}
				//Set element
				obj->mbrs[obj->mbrsSz++] = m;
			}
		}
	}
	return r;
}

//Embeded members (void)

//BOOL
BOOL NBStructMap_addBool(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz == 1 || mbrSz == 2 || mbrSz == 4 || mbrSz == 8)
	if(mbrSz == 1 || mbrSz == 2 || mbrSz == 4 || mbrSz == 8){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Bool, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, FALSE, mbrSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//INT
BOOL NBStructMap_addInt(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz == 1 || mbrSz == 2 || mbrSz == 4 || mbrSz == 8)
	if(mbrSz == 1 || mbrSz == 2 || mbrSz == 4 || mbrSz == 8){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Int, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, FALSE, mbrSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//UINT
BOOL NBStructMap_addUInt(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz == 1 || mbrSz == 2 || mbrSz == 4 || mbrSz == 8)
	if(mbrSz == 1 || mbrSz == 2 || mbrSz == 4 || mbrSz == 8){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_UInt, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, FALSE, mbrSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//float
BOOL NBStructMap_addFloat(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz == sizeof(float))
	if(mbrSz == sizeof(float)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Float, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, FALSE, mbrSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//double
BOOL NBStructMap_addDouble(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz == sizeof(double))
	if(mbrSz == sizeof(double)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Double, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, FALSE, mbrSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//char[x]
BOOL NBStructMap_addChars(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz > 0 && elemSz == sizeof(char))
	if(mbrSz > 0 && elemSz == sizeof(char)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Chars, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, FALSE, mbrSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//char[x] (binary)
BOOL NBStructMap_addBytes(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz > 0 && mbrSz == sizeof(BYTE))
	if(mbrSz > 0 && mbrSz == sizeof(BYTE)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Bytes, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, FALSE, mbrSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//char*
BOOL NBStructMap_addStrPtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz == sizeof(void*) && elemSz == sizeof(char))
	if(mbrSz == sizeof(void*) && elemSz == sizeof(char)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_StrPtr, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, FALSE, mbrSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//ENUM
BOOL NBStructMap_addEnum(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const STNBEnumMap* enMap){
	BOOL r = FALSE;
	NBASSERT(enMap != NULL && (mbrSz == 1 || mbrSz == 2 || mbrSz == 4 || mbrSz == 8))
	if(enMap != NULL && (mbrSz == 1 || mbrSz == 2 || mbrSz == 4 || mbrSz == 8)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Enum, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, FALSE, mbrSz, enMap, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//STRUCT
BOOL NBStructMap_addStruct(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const STNBStructMap* stMap){
	BOOL r = FALSE;
	NBASSERT(stMap != NULL && mbrSz == stMap->stSize)
	if(stMap != NULL && mbrSz == stMap->stSize){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Struct, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, FALSE, mbrSz, NULL, stMap,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//--------------------------------
//Pointer to one element (void*)
//--------------------------------

//BOOL*
BOOL NBStructMap_addBoolPtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz == sizeof(void*) && (elemSz == 1 || elemSz == 2 || elemSz == 4 || elemSz == 8)) //ToDo: allow void* pointers (elemSz == 1 but not the actual value)
	if(mbrSz == sizeof(void*) && (elemSz == 1 || elemSz == 2 || elemSz == 4 || elemSz == 8)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Bool, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//INT*
BOOL NBStructMap_addIntPtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz == sizeof(void*) && (elemSz == 1 || elemSz == 2 || elemSz == 4 || elemSz == 8)) //ToDo: allow void* pointers (elemSz == 1 but not the actual value)
	if(mbrSz == sizeof(void*) && (elemSz == 1 || elemSz == 2 || elemSz == 4 || elemSz == 8)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Int, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//UINT*
BOOL NBStructMap_addUIntPtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz == sizeof(void*) && (elemSz == 1 || elemSz == 2 || elemSz == 4 || elemSz == 8)) //ToDo: allow void* pointers (elemSz == 1 but not the actual value)
	if(mbrSz == sizeof(void*) && (elemSz == 1 || elemSz == 2 || elemSz == 4 || elemSz == 8)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_UInt, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//float*
BOOL NBStructMap_addFloatPtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz == sizeof(void*) && elemSz == sizeof(float)) //ToDo: allow void* pointers (elemSz == 1 but not the actual value)
	if(mbrSz == sizeof(void*) && elemSz == sizeof(float)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Float, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//double*
BOOL NBStructMap_addDoublePtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz == sizeof(void*) && elemSz == sizeof(double)) //ToDo: allow void* pointers (elemSz == 1 but not the actual value)
	if(mbrSz == sizeof(void*) && elemSz == sizeof(double)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Double, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//char (*)[x];
BOOL NBStructMap_addCharsPtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz == sizeof(void*) && elemSz > 0 && elemElemSz == sizeof(char)) //ToDo: allow void* pointers (elemSz == 1 but not the actual value)
	if(mbrSz == sizeof(void*) && elemSz > 0 && elemElemSz == sizeof(char)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Chars, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//BYTE (*)[x]; (binary)
BOOL NBStructMap_addBytesPtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz == sizeof(void*) && elemSz == sizeof(char)) //ToDo: allow void* pointers (elemSz == 1 but not the actual value)
	if(mbrSz == sizeof(void*) && elemSz == sizeof(char)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Bytes, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//char**;
BOOL NBStructMap_addStrPtrPtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz == sizeof(void*) && elemSz == sizeof(void*) && elemElemSz == sizeof(char)) //ToDo: allow void* pointers (elemSz == 1 but not the actual value)
	if(mbrSz == sizeof(void*) && elemSz == sizeof(void*) && elemElemSz == sizeof(char)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_StrPtr, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}


//ENUM*
BOOL NBStructMap_addEnumPtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const STNBEnumMap* enMap){
	BOOL r = FALSE;
	NBASSERT(mbrSz == sizeof(void*) && enMap != NULL && (elemSz == 1 || elemSz == 2 || elemSz == 4 || elemSz == 8)) //ToDo: allow void* pointers (elemSz == 1 but not the actual value)
	if(mbrSz == sizeof(void*) && enMap != NULL && (elemSz == 1 || elemSz == 2 || elemSz == 4 || elemSz == 8)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Enum, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemSz, enMap, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//STRUCT*
BOOL NBStructMap_addStructPtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const STNBStructMap* stMap){
	BOOL r = FALSE;
	NBASSERT(mbrSz == sizeof(void*) && stMap != NULL && (stMap->stSize == elemSz || elemSz == sizeof(void))) //elemSz == sizeof(void) when a void* is used to reference data
	if(mbrSz == sizeof(void*) && stMap != NULL){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Struct, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, stMap->stSize, NULL, stMap,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//--------------------------------
//Embeded array of elements (void[], implicit-size)
//--------------------------------

//BOOL[x]
BOOL NBStructMap_addArrayOfBool(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && (elemSz == 1 || elemSz == 2 || elemSz == 4 || elemSz == 8))
	if((mbrSz >= elemSz && (mbrSz % elemSz) == 0 && (elemSz == 1 || elemSz == 2 || elemSz == 4 || elemSz == 8))){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Bool, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, FALSE, elemSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//INT[x]
BOOL NBStructMap_addArrayOfInt(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && (elemSz == 1 || elemSz == 2 || elemSz == 4 || elemSz == 8))
	if((mbrSz >= elemSz && (mbrSz % elemSz) == 0 && (elemSz == 1 || elemSz == 2 || elemSz == 4 || elemSz == 8))){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Int, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, FALSE, elemSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//UINT[x]
BOOL NBStructMap_addArrayOfUInt(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && (elemSz == 1 || elemSz == 2 || elemSz == 4 || elemSz == 8))
	if((mbrSz >= elemSz && (mbrSz % elemSz) == 0 && (elemSz == 1 || elemSz == 2 || elemSz == 4 || elemSz == 8))){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_UInt, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, FALSE, elemSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//floa[x]
BOOL NBStructMap_addArrayOfFloat(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(float))
	if(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(float)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Float, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, FALSE, elemSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//double[x]
BOOL NBStructMap_addArrayOfDouble(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(double))
	if(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(double)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Double, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, FALSE, elemSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//(char[x])[]
BOOL NBStructMap_addArrayOfChars(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz > elemSz && (mbrSz % elemSz) == 0 && elemElemSz == sizeof(char))
	if(mbrSz > elemSz && (mbrSz % elemSz) == 0 && elemElemSz == sizeof(char)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Chars, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, FALSE, elemSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//(char[x])[] (binary)
BOOL NBStructMap_addArrayOfBytes(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz > elemSz && elemSz == sizeof(char))
	if(mbrSz > elemSz && elemSz == sizeof(char)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Bytes, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, FALSE, elemSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//char*[x]
BOOL NBStructMap_addArrayOfStrPtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz > elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(void*) && elemElemSz == sizeof(char))
	if(mbrSz > elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(void*) && elemElemSz == sizeof(char)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_StrPtr, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, FALSE, elemSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//ENUM[x]
BOOL NBStructMap_addArrayOfEnum(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const STNBEnumMap* enMap){
	BOOL r = FALSE;
	NBASSERT(mbrSz > elemSz && (mbrSz % elemSz) == 0 && enMap != NULL && (elemSz == 1 || elemSz == 2 || elemSz == 4 || elemSz == 8))
	if(mbrSz > elemSz && (mbrSz % elemSz) == 0 && enMap != NULL && (elemSz == 1 || elemSz == 2 || elemSz == 4 || elemSz == 8)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Enum, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, FALSE, elemSz, enMap, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//STRUCT[x]
BOOL NBStructMap_addArrayOfStruct(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const STNBStructMap* stMap){
	BOOL r = FALSE;
	NBASSERT(mbrSz > elemSz && (mbrSz % elemSz) == 0 && stMap != NULL && elemSz == stMap->stSize)
	if(mbrSz > elemSz && (mbrSz % elemSz) == 0 && stMap != NULL && elemSz == stMap->stSize){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Struct, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, FALSE, elemSz, NULL, stMap,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//--------------------------------
//Embeded array of pointers to elements (void*[], implicit-size)
//--------------------------------

//(BOOL*)[x]
BOOL NBStructMap_addArrayOfBoolPtrs(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(void*) && (elemElemSz == 1 || elemElemSz == 2 || elemElemSz == 4 || elemElemSz == 8))
	if(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(void*) && (elemElemSz == 1 || elemElemSz == 2 || elemElemSz == 4 || elemElemSz == 8)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Bool, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemElemSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//(INT*)[x]
BOOL NBStructMap_addArrayOfIntPtrs(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(void*) && (elemElemSz == 1 || elemElemSz == 2 || elemElemSz == 4 || elemElemSz == 8))
	if(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(void*) && (elemElemSz == 1 || elemElemSz == 2 || elemElemSz == 4 || elemElemSz == 8)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Int, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemElemSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//(UINT*)[x]
BOOL NBStructMap_addArrayOfUIntPtrs(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(void*) && (elemElemSz == 1 || elemElemSz == 2 || elemElemSz == 4 || elemElemSz == 8))
	if(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(void*) && (elemElemSz == 1 || elemElemSz == 2 || elemElemSz == 4 || elemElemSz == 8)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_UInt, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemElemSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//(float*)[x]
BOOL NBStructMap_addArrayOfFloatPtrs(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(void*) && elemElemSz == sizeof(float))
	if(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(void*) && elemElemSz == sizeof(float)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Float, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemElemSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//(double*)[x]
BOOL NBStructMap_addArrayOfDoublePtrs(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(void*) && elemElemSz == sizeof(double))
	if(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(void*) && elemElemSz == sizeof(double)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Double, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemElemSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//(char*[])[x]
BOOL NBStructMap_addArrayOfCharsPtrs(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz, const UI32 elemElemElemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(void*) && elemElemSz > 0 && elemElemElemSz == sizeof(char))
	if(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(void*) && elemElemSz > 0 && elemElemElemSz == sizeof(char)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Chars, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemElemSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//(char*[])[x] (binary)
BOOL NBStructMap_addArrayOfBytesPtrs(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz, const UI32 elemElemElemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(void*) && elemElemSz > 0 && elemElemElemSz == sizeof(char))
	if(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(void*) && elemElemSz > 0 && elemElemElemSz == sizeof(char)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Bytes, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemElemSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//(char*)[x]
BOOL NBStructMap_addArrayOfStrPtrPtrs(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz, const UI32 elemElemElemSz){
	BOOL r = FALSE;
	NBASSERT(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(void*) && elemElemSz == sizeof(void*) && elemElemElemSz == sizeof(char))
	if(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(void*) && elemElemSz == sizeof(void*) && elemElemElemSz == sizeof(char)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_StrPtr, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemElemSz, NULL, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//(ENUM*)[x]
BOOL NBStructMap_addArrayOfEnumPtrs(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz, const STNBEnumMap* enMap){
	BOOL r = FALSE;
	NBASSERT(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(void*) && enMap != NULL && (elemElemSz == 1 || elemElemSz == 2 || elemElemSz == 4 || elemElemSz == 8))
	if(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(void*) && enMap != NULL && (elemElemSz == 1 || elemElemSz == 2 || elemElemSz == 4 || elemElemSz == 8)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Enum, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemElemSz, enMap, NULL,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

BOOL NBStructMap_addArrayOfStructPtrs(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr,const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz, const STNBStructMap* stMap){
	BOOL r = FALSE;
	NBASSERT(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(void*) && stMap != NULL && elemElemSz == stMap->stSize)
	if(mbrSz >= elemSz && (mbrSz % elemSz) == 0 && elemSz == sizeof(void*) && stMap != NULL && elemElemSz == stMap->stSize){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Struct, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemElemSz, NULL, stMap,    NULL, NULL, 0, ENNBStructMapSign_Count);
	}
	return r;
}

//--------------------------------
//Pointer to array of elements (void* + explicit-size)
//--------------------------------

//BOOL**
BOOL NBStructMap_addPtrToArrayOfBool(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const char* szMbrName, const void* pSzmbrPtr, const UI32 szMbrSz, const ENNBStructMapSign szMbrSign){
	BOOL r = FALSE;
	NBASSERT(mbrSz == sizeof(void*) && (elemSz == 1 || elemSz == 2 || elemSz == 4 || elemSz == 8) && (szMbrSz == 1 || szMbrSz == 2 || szMbrSz == 4 || szMbrSz == 8) && (szMbrSign == ENNBStructMapSign_Signed || szMbrSign == ENNBStructMapSign_Unsigned))
	if(mbrSz == sizeof(void*) && (elemSz == 1 || elemSz == 2 || elemSz == 4 || elemSz == 8) && (szMbrSz == 1 || szMbrSz == 2 || szMbrSz == 4 || szMbrSz == 8) && (szMbrSign == ENNBStructMapSign_Signed || szMbrSign == ENNBStructMapSign_Unsigned)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Bool, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemSz, NULL, NULL,    szMbrName, (const BYTE*)pSzmbrPtr, szMbrSz, szMbrSign);
	}
	return r;
}

//INT**
BOOL NBStructMap_addPtrToArrayOfInt(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const char* szMbrName, const void* pSzmbrPtr, const UI32 szMbrSz, const ENNBStructMapSign szMbrSign){
	BOOL r = FALSE;
	NBASSERT(mbrSz == sizeof(void*) && (elemSz == 1 || elemSz == 2 || elemSz == 4 || elemSz == 8) && (szMbrSz == 1 || szMbrSz == 2 || szMbrSz == 4 || szMbrSz == 8) && (szMbrSign == ENNBStructMapSign_Signed || szMbrSign == ENNBStructMapSign_Unsigned))
	if(mbrSz == sizeof(void*) && (elemSz == 1 || elemSz == 2 || elemSz == 4 || elemSz == 8) && (szMbrSz == 1 || szMbrSz == 2 || szMbrSz == 4 || szMbrSz == 8) && (szMbrSign == ENNBStructMapSign_Signed || szMbrSign == ENNBStructMapSign_Unsigned)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Int, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemSz, NULL, NULL,    szMbrName, (const BYTE*)pSzmbrPtr, szMbrSz, szMbrSign);
	}
	return r;
}

//UINT**
BOOL NBStructMap_addPtrToArrayOfUInt(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const char* szMbrName, const void* pSzmbrPtr, const UI32 szMbrSz, const ENNBStructMapSign szMbrSign){
	BOOL r = FALSE;
	NBASSERT(mbrSz == sizeof(void*) && (elemSz == 1 || elemSz == 2 || elemSz == 4 || elemSz == 8) && (szMbrSz == 1 || szMbrSz == 2 || szMbrSz == 4 || szMbrSz == 8) && (szMbrSign == ENNBStructMapSign_Signed || szMbrSign == ENNBStructMapSign_Unsigned))
	if(mbrSz == sizeof(void*) && (elemSz == 1 || elemSz == 2 || elemSz == 4 || elemSz == 8) && (szMbrSz == 1 || szMbrSz == 2 || szMbrSz == 4 || szMbrSz == 8) && (szMbrSign == ENNBStructMapSign_Signed || szMbrSign == ENNBStructMapSign_Unsigned)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_UInt, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemSz, NULL, NULL,    szMbrName, (const BYTE*)pSzmbrPtr, szMbrSz, szMbrSign);
	}
	return r;
}

//float**
BOOL NBStructMap_addPtrToArrayOfFloat(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const char* szMbrName, const void* pSzmbrPtr, const UI32 szMbrSz, const ENNBStructMapSign szMbrSign){
	BOOL r = FALSE;
	NBASSERT(mbrSz == sizeof(void*) && elemSz == sizeof(float) && (szMbrSz == 1 || szMbrSz == 2 || szMbrSz == 4 || szMbrSz == 8) && (szMbrSign == ENNBStructMapSign_Signed || szMbrSign == ENNBStructMapSign_Unsigned))
	if(mbrSz == sizeof(void*) && elemSz == sizeof(float) && (szMbrSz == 1 || szMbrSz == 2 || szMbrSz == 4 || szMbrSz == 8) && (szMbrSign == ENNBStructMapSign_Signed || szMbrSign == ENNBStructMapSign_Unsigned)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Float, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemSz, NULL, NULL,    szMbrName, (const BYTE*)pSzmbrPtr, szMbrSz, szMbrSign);
	}
	return r;
}

//double**
BOOL NBStructMap_addPtrToArrayOfDouble(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const char* szMbrName, const void* pSzmbrPtr, const UI32 szMbrSz, const ENNBStructMapSign szMbrSign){
	BOOL r = FALSE;
	NBASSERT(mbrSz == sizeof(void*) && elemSz == sizeof(double) && (szMbrSz == 1 || szMbrSz == 2 || szMbrSz == 4 || szMbrSz == 8) && (szMbrSign == ENNBStructMapSign_Signed || szMbrSign == ENNBStructMapSign_Unsigned))
	if(mbrSz == sizeof(void*) && elemSz == sizeof(double) && (szMbrSz == 1 || szMbrSz == 2 || szMbrSz == 4 || szMbrSz == 8) && (szMbrSign == ENNBStructMapSign_Signed || szMbrSign == ENNBStructMapSign_Unsigned)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Double, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemSz, NULL, NULL,    szMbrName, (const BYTE*)pSzmbrPtr, szMbrSz, szMbrSign);
	}
	return r;
}

//char(*)[x]
BOOL NBStructMap_addPtrToArrayOfChars(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const char* szMbrName, const void* pSzmbrPtr, const UI32 szMbrSz, const ENNBStructMapSign szMbrSign){
	BOOL r = FALSE;
	NBASSERT(mbrSz == sizeof(void*) && elemSz > 0 && (szMbrSz == 1 || szMbrSz == 2 || szMbrSz == 4 || szMbrSz == 8) && (szMbrSign == ENNBStructMapSign_Signed || szMbrSign == ENNBStructMapSign_Unsigned))
	if(mbrSz == sizeof(void*) && elemSz > 0 && (szMbrSz == 1 || szMbrSz == 2 || szMbrSz == 4 || szMbrSz == 8) && (szMbrSign == ENNBStructMapSign_Signed || szMbrSign == ENNBStructMapSign_Unsigned)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Chars, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemSz, NULL, NULL,    szMbrName, (const BYTE*)pSzmbrPtr, szMbrSz, szMbrSign);
	}
	return r;
}

//BYTE(*)[x] (binary)
BOOL NBStructMap_addPtrToArrayOfBytes(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const char* szMbrName, const void* pSzmbrPtr, const UI32 szMbrSz, const ENNBStructMapSign szMbrSign){
	BOOL r = FALSE;
	NBASSERT(mbrSz == sizeof(void*) && elemSz > 0 && (szMbrSz == 1 || szMbrSz == 2 || szMbrSz == 4 || szMbrSz == 8) && (szMbrSign == ENNBStructMapSign_Signed || szMbrSign == ENNBStructMapSign_Unsigned))
	if(mbrSz == sizeof(void*) && elemSz > 0 && (szMbrSz == 1 || szMbrSz == 2 || szMbrSz == 4 || szMbrSz == 8) && (szMbrSign == ENNBStructMapSign_Signed || szMbrSign == ENNBStructMapSign_Unsigned)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Bytes, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemSz, NULL, NULL,    szMbrName, (const BYTE*)pSzmbrPtr, szMbrSz, szMbrSign);
	}
	return r;
}

//(char**)[]
BOOL NBStructMap_addPtrToArrayOfStrPtr(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const UI32 elemElemSz, const char* szMbrName, const void* pSzmbrPtr, const UI32 szMbrSz, const ENNBStructMapSign szMbrSign){
	BOOL r = FALSE;
	NBASSERT(mbrSz == sizeof(void*) && elemSz == sizeof(void*) && elemElemSz == sizeof(char) && (szMbrSz == 1 || szMbrSz == 2 || szMbrSz == 4 || szMbrSz == 8) && (szMbrSign == ENNBStructMapSign_Signed || szMbrSign == ENNBStructMapSign_Unsigned))
	if(mbrSz == sizeof(void*) && elemSz == sizeof(void*) && elemElemSz == sizeof(char) && (szMbrSz == 1 || szMbrSz == 2 || szMbrSz == 4 || szMbrSz == 8) && (szMbrSign == ENNBStructMapSign_Signed || szMbrSign == ENNBStructMapSign_Unsigned)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_StrPtr, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemSz, NULL, NULL,    szMbrName, (const BYTE*)pSzmbrPtr, szMbrSz, szMbrSign);
	}
	return r;
}

//ENUM**
BOOL NBStructMap_addPtrToArrayOfEnum(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr, const UI32 mbrSz, const UI32 elemSz, const STNBEnumMap* enMap, const char* szMbrName, const void* pSzmbrPtr, const UI32 szMbrSz, const ENNBStructMapSign szMbrSign){
	BOOL r = FALSE;
	NBASSERT(mbrSz == sizeof(void*) && enMap != NULL && (elemSz == 1 || elemSz == 2 || elemSz == 4 || elemSz == 8) && (szMbrSz == 1 || szMbrSz == 2 || szMbrSz == 4 || szMbrSz == 8) && (szMbrSign == ENNBStructMapSign_Signed || szMbrSign == ENNBStructMapSign_Unsigned))
	if(mbrSz == sizeof(void*) && enMap != NULL && (elemSz == 1 || elemSz == 2 || elemSz == 4 || elemSz == 8) && (szMbrSz == 1 || szMbrSz == 2 || szMbrSz == 4 || szMbrSz == 8) && (szMbrSign == ENNBStructMapSign_Signed || szMbrSign == ENNBStructMapSign_Unsigned)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Enum, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemSz, enMap, NULL,    szMbrName, (const BYTE*)pSzmbrPtr, szMbrSz, szMbrSign);
	}
	return r;
}

//STRUCT**
BOOL NBStructMap_addPtrToArrayOfStruct(STNBStructMap* obj, const char* name, const void* stPtr, const void* mbrPtr,const UI32 mbrSz, const UI32 elemSz, const STNBStructMap* stMap, const char* szMbrName, const void* pSzmbrPtr, const UI32 szMbrSz, const ENNBStructMapSign szMbrSign){
	BOOL r = FALSE;
	NBASSERT(mbrSz == sizeof(void*) && stMap != NULL && elemSz == stMap->stSize && (szMbrSz == 1 || szMbrSz == 2 || szMbrSz == 4 || szMbrSz == 8) && (szMbrSign == ENNBStructMapSign_Signed || szMbrSign == ENNBStructMapSign_Unsigned))
	if(mbrSz == sizeof(void*) && stMap != NULL && elemSz == stMap->stSize && (szMbrSz == 1 || szMbrSz == 2 || szMbrSz == 4 || szMbrSz == 8) && (szMbrSign == ENNBStructMapSign_Signed || szMbrSign == ENNBStructMapSign_Unsigned)){
		r = NBStructMap_addMember_(obj, ENNBStructMapMemberType_Struct, (const BYTE*)stPtr, name, (const BYTE*)mbrPtr, mbrSz, TRUE, elemSz, NULL, stMap,    szMbrName, (const BYTE*)pSzmbrPtr, szMbrSz, szMbrSign);
	}
	return r;
}

//Compare

BOOL NBStructMap_strIsEqual(const char* str1, const char* str2){
	if(str1 == str2) return TRUE; //both can be NULL or the same pointer
	if((str1 == NULL && str2 != NULL) || (str1 != NULL && str2 == NULL)) return FALSE;
	//PRINTF_INFO("Comparando cadenas: '%s' vs '%s'\n", str1, str2);
	const char* c1 = str1;
	const char* c2 = str2;
	while(TRUE){
		if(*c1 != *c2) return FALSE;
		if(*c1 == '\0') break;
		c1++; c2++;
	}
	return TRUE;
}

BOOL NBStructMap_memIsEqual(const STNBStructMapMember* m, const STNBStructMapMember* m2){
	BOOL r = FALSE;
	if(m == m2){ //both can be NULL or the same pointer
		r = TRUE;
	} else if(m != NULL && m2 != NULL){
		r = TRUE;
		if(m->iPos != m2->iPos || m->size != m2->size){
			r = FALSE;
		} else if(!NBStructMap_strIsEqual(m->name, m2->name)){
			r = FALSE;
		} else if(m->elem.isPtr != m2->elem.isPtr || m->elem.count != m2->elem.count){
			r = FALSE;
		} else {
			r = FALSE; NBASSERT(FALSE) //ToDO: implement this.
			/*typedef struct STNBStructMapMemberElem_ {
				BOOL							isPtr;	//element are pointers
				UI32							count;	//fixed amount of elements inside this member
				struct STNBStructMapMember_* 	vCount;	//variable amount of elements
			} STNBStructMapMemberElem;*/
		}
	}
	return r;
}

//

const STNBStructMapMember* NBStructMap_getMember(const STNBStructMap* obj, const char* name){
	const STNBStructMapMember* r = NULL;
	UI32 i; for(i = 0; i < obj->mbrsSz; i++){
		const STNBStructMapMember* m = &obj->mbrs[i];
		if(NBStructMapMember_strIsEqual(m->name, name)){
			r = m;
			break;
		}
	}
	return r;
}


