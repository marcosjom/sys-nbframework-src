//
//  NBString.h
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#ifndef NB_COMPARE_H
#define NB_COMPARE_H

#include "nb/NBFrameworkDefs.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef enum ENCompareMode_ {
		ENCompareMode_Equal = 0,
		ENCompareMode_Lower,
		ENCompareMode_LowerOrEqual,
		ENCompareMode_Greater,
		ENCompareMode_GreaterOrEqual
	} ENCompareMode;
	
	typedef BOOL (*NBCompareFunc)(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);
	
#	define NB_COMPARE_NATIVE_FUNC_HEADER(TYPE)	\
		BOOL NBCompare ## TYPE(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz)

#	define NB_COMPARE_NATIVE_FUNC_BODY(TYPE)	\
		NBASSERT(dataSz == sizeof(TYPE)) \
		if(dataSz == sizeof(TYPE)){ \
			switch (mode) { \
				case ENCompareMode_Equal: \
					return *((TYPE*)data1) == *((TYPE*)data2); \
				case ENCompareMode_Lower: \
					return *((TYPE*)data1) < *((TYPE*)data2); \
				case ENCompareMode_LowerOrEqual: \
					return *((TYPE*)data1) <= *((TYPE*)data2); \
				case ENCompareMode_Greater: \
					return *((TYPE*)data1) > *((TYPE*)data2); \
				case ENCompareMode_GreaterOrEqual: \
					return *((TYPE*)data1) >= *((TYPE*)data2); \
				default: \
					NBASSERT(FALSE) \
					break; \
			} \
		} \
	NBASSERT(FALSE) \
	return FALSE; \

#	define NB_COMPARE_NATIVE_FUNC_DECLARATION(TYPE)	NB_COMPARE_NATIVE_FUNC_HEADER(TYPE)
	
#	define NB_COMPARE_NATIVE_FUNC_DEFINITION(TYPE)	NB_COMPARE_NATIVE_FUNC_HEADER(TYPE) { NB_COMPARE_NATIVE_FUNC_BODY(TYPE) }

	NB_COMPARE_NATIVE_FUNC_HEADER(BOOL);
	NB_COMPARE_NATIVE_FUNC_HEADER(BYTE);
	NB_COMPARE_NATIVE_FUNC_HEADER(char);
	NB_COMPARE_NATIVE_FUNC_HEADER(SI8);
	NB_COMPARE_NATIVE_FUNC_HEADER(SI16);
	NB_COMPARE_NATIVE_FUNC_HEADER(SI32);
	NB_COMPARE_NATIVE_FUNC_HEADER(SI64);
	NB_COMPARE_NATIVE_FUNC_HEADER(UI8);
	NB_COMPARE_NATIVE_FUNC_HEADER(UI16);
	NB_COMPARE_NATIVE_FUNC_HEADER(UI32);
	NB_COMPARE_NATIVE_FUNC_HEADER(UI64);
	NB_COMPARE_NATIVE_FUNC_HEADER(FLOAT);
	NB_COMPARE_NATIVE_FUNC_HEADER(DOUBLE);
	
	BOOL NBCompare_PtrVoid(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
