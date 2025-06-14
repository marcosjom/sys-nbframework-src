//
//  NBStringsLib.h
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 10/09/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#ifndef NB_STRINGS_LIB_H
#define NB_STRINGS_LIB_H

#include "nb/NBFrameworkDefs.h"
#include <stdio.h>	//for FILE

//----------------------------------------------------------------------
//-- Esta clase es una biblioteca de cadenas sin ordenar.
//-- Esta clase es util para almacenar cadenas sin repetir.
//----------------------------------------------------------------------
#define NBLIBSTR_CHARS_BUFFER_SIZE_START	256 //256
#define NBLIBSTR_CHARS_BUFFER_GROWTH_SIZE	4096 //4096
#define NBBIBSTR_CHARS_BUFFER_GROWTH_MAX	(1024*1024*2) //2MBs

#define NBLIBSTR_INDEX_BUFFER_SIZE_START	16 //64
#define NBLIBSTR_INDEX_BUFFER_GROWTH_SIZE	128 //64
#define NBBIBSTR_INDEXES_BUFFER_GROWTH_MAX	200000 //200000

#define NBLIBSTR_FILE_ID_START				10254	//Elegido al azar (UI32, diferente del resto de IDs)
#define NBLIBSTR_FILE_ID_END				39959	//Elegido al azar (UI32, diferente del resto de IDs)

#ifdef __cplusplus
extern "C" {
#endif
	
	struct NBStringsLib {
		UI32 useChars;
		UI32 sizeChars;
		char* chars;
		//
		UI32 useIndexes;
		UI32 sizeIndexes;
		UI32* indexes;
	};
	
	//---------------------------
	//Metodos de alimentacion de biblioteca
	//---------------------------
	void NBStringsLib_init(struct NBStringsLib* objLibrary);
	void NBStringsLib_release(struct NBStringsLib* objLibrary);
	void NBStringsLib_empty(struct NBStringsLib* objLibrary);
	UI32 NBStringsLib_indexOfString(struct NBStringsLib* objLibrary, const char* valueString);
	UI32 NBStringsLib_indexOfStringSearchOnly(const struct NBStringsLib* objLibrary, const char* valueString, UI8* saveFoundAt);
	void NBStringsLib_growCharBuffer(struct NBStringsLib* objLibrary);
	const char* NBStringsLib_stringAtIndex(const struct NBStringsLib* objLibrary, const UI32 index);
	UI8 NBStringsLib_stringAtIndexBool(const struct NBStringsLib* objLibrary, const UI32 index);
	
	void NBStringsLib_saveToFile(struct NBStringsLib* objLibrary, FILE* objFile);
	UI32 NBStringsLib_loadFromFile(struct NBStringsLib* objLibrary, FILE* file);
	
	//---------------------------
	//Metodos auxiliares
	//---------------------------
	UI32 NBStringsLib_stringLength(const char* string);
	UI32 NBStringsLib_copyString(char* destination, const char* string);
	UI32 NBStringsLib_compareStrings(const char* string1, const char* string2);
	char* NBStringsLib_createCopyString(const char* string);
	char* NBStringsLib_loadString(FILE* objFile);
	void NBStringsLib_saveString(const char* string, FILE* objFile);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
