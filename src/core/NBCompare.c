//
//  XUCadena.c
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBCompare.h"

NB_COMPARE_NATIVE_FUNC_DEFINITION(BOOL);
NB_COMPARE_NATIVE_FUNC_DEFINITION(BYTE);
NB_COMPARE_NATIVE_FUNC_DEFINITION(char);
NB_COMPARE_NATIVE_FUNC_DEFINITION(SI8);
NB_COMPARE_NATIVE_FUNC_DEFINITION(SI16);
NB_COMPARE_NATIVE_FUNC_DEFINITION(SI32);
NB_COMPARE_NATIVE_FUNC_DEFINITION(SI64);
NB_COMPARE_NATIVE_FUNC_DEFINITION(UI8);
NB_COMPARE_NATIVE_FUNC_DEFINITION(UI16);
NB_COMPARE_NATIVE_FUNC_DEFINITION(UI32);
NB_COMPARE_NATIVE_FUNC_DEFINITION(UI64);
NB_COMPARE_NATIVE_FUNC_DEFINITION(FLOAT);
NB_COMPARE_NATIVE_FUNC_DEFINITION(DOUBLE);

BOOL NBCompare_PtrVoid(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(void*))
	if(dataSz == sizeof(void*)){
		switch (mode) {
		case ENCompareMode_Equal:
			return *((void**)data1) == *((void**)data2);
		case ENCompareMode_Lower:
			return *((void**)data1) < *((void**)data2);
		case ENCompareMode_LowerOrEqual:
			return *((void**)data1) <= *((void**)data2);
		case ENCompareMode_Greater:
			return *((void**)data1) > *((void**)data2);
		case ENCompareMode_GreaterOrEqual:
			return *((void**)data1) >= *((void**)data2);
		default:
			NBASSERT(FALSE)
			break;
		}
	}
	NBASSERT(FALSE)
	return FALSE;
}
