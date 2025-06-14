//
//  NBEnumMap.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/28/18.
//

#ifndef NBEnumMap_h
#define NBEnumMap_h

#include "nb/NBFrameworkDefs.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBEnumMapRecord_ {
		SI32			intValue;	//value of the enum member
		const char*		varName;	//the exact value member
		const char*		strValue;	//a string representation (for json wrtite/load)
	} STNBEnumMapRecord;
	
	typedef struct STNBEnumMap_ {
		const char*			enumName;
		STNBEnumMapRecord*	records;
		UI32				recordsSz;
	} STNBEnumMap;
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBEnumMap_h */
