//
//  NBStructMapMember.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/28/18.
//

#ifndef NBStructMapMember_h
#define NBStructMapMember_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBEnumMap.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef enum ENNBStructMapMemberType_ {
		ENNBStructMapMemberType_Bool = 0,		//(char, short, int, long, long long)
		ENNBStructMapMemberType_Int,			//(char, short, int, long, long long)
		ENNBStructMapMemberType_UInt,			//(unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long)
		ENNBStructMapMemberType_Float,			//(float)
		ENNBStructMapMemberType_Double,			//(double)
		ENNBStructMapMemberType_Chars,			//(fixed-size string)
		ENNBStructMapMemberType_Bytes,			//(fixed-size data-bytes)
		ENNBStructMapMemberType_StrPtr,			//(pointer to a null-terminated string)
		ENNBStructMapMemberType_Enum,			//(char, short, int, long, long long) with map
		ENNBStructMapMemberType_Struct,			//(another struct) with map
		ENNBStructMapMemberType_Count
	} ENNBStructMapMemberType;
	
	typedef struct STNBStructMapMemberElem_ {
		BOOL							isPtr;	//element are pointers
		UI32							count;	//fixed amount of elements inside this member
		struct STNBStructMapMember_* 	vCount;	//variable amount of elements
	} STNBStructMapMemberElem;
	
	typedef struct STNBStructMapMemberData_ {
		ENNBStructMapMemberType			type;	//element's type
		UI32							size;	//element's data size (different to memberSz if the member is an pointer or an array)
		const STNBEnumMap*				enMap;	//element's map, if enum
		const struct STNBStructMap_*	stMap;	//element's map, if struct
	} STNBStructMapMemberData;
		
	struct STNBStructMap_;
	
	typedef struct STNBStructMapMember_ {
		char*					name;	//member's name
		UI32					iPos;	//member position in the struct
		UI32					size;	//member size in the struct
		STNBStructMapMemberElem	elem;	//elements description
		STNBStructMapMemberData	data;	//data description
	} STNBStructMapMember;
	
	void NBStructMapMember_init(STNBStructMapMember* obj, const char* name);
	void NBStructMapMember_release(STNBStructMapMember* obj);
	
	UI32 NBStructMapMember_strLenBytes(const char* str);
	BOOL NBStructMapMember_strIsEqual(const char* c1, const char* c2);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBStructMapMember_h */
