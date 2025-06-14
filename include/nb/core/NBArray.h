//
//  NBArray.h
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#ifndef NB_ARRAY_H
#define NB_ARRAY_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBCompare.h"
#include "nb/files/NBFile.h"

#define NBArray_addValue(PTR_ARR, VALUE)				NBArray_add(PTR_ARR, &VALUE, sizeof(VALUE))
#define NBArray_addValueCopy(PTR_ARR, TYPE, VALUE)		{ const TYPE tmp = VALUE; NBArray_add(PTR_ARR, &tmp, sizeof(tmp)); }
#define NBArray_dataPtr(PTR_ARR, TYPE)					((TYPE*)NBArray_data(PTR_ARR))
#define NBArray_itmPtrAtIndex(PTR_ARR, TYPE, IDX)		((TYPE*)NBArray_itemAtIndexWithSize(PTR_ARR, IDX, sizeof(TYPE)))
#define NBArray_itmValueAtIndex(PTR_ARR, TYPE, IDX)		(*((TYPE*)NBArray_itemAtIndexWithSize(PTR_ARR, IDX, sizeof(TYPE))))
#define NBArray_indexOfValue(PTR_ARR, VALUE)			NBArray_indexOf(PTR_ARR, &VALUE, sizeof(VALUE))

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBArray_ {
		SI32			use;			//Buffer use count (in item units)
		SI32			_bytesPerItem;	//Size of each element, in bytes
		NBCompareFunc	_cmpFunc;		//Items compare function
		//
		BYTE*			_buffData;		//Buffer data
		SI32			_buffSize;		//Buffer curr max size (in items units)
		SI32			_buffGrowthMin;	//Growth minimun of the array buffer size when the limit is reached
		UI16			_buffGrowthRel;	//Growth exponent in 1/16 units (1.0 is added to this value)
		BOOL			_buffIsExtern;
	} STNBArray;
	
	void NBArray_init(STNBArray* obj, const SI32 bytesPerItem, NBCompareFunc cmpFunc);
	void NBArray_initWithOther(STNBArray* obj, const STNBArray* other);
	void NBArray_initWithSz(STNBArray* obj, const SI32 bytesPerItem, NBCompareFunc cmpFunc, const SI32 initialSize, const SI32 growthMin, const float growthRelExtra /*after 1.0*/);
	void NBArray_initWithExternalBuffer(STNBArray* obj, const SI32 bytesPerItem, NBCompareFunc cmpFunc, BYTE* externalBuffer);
	void NBArray_release(STNBArray* obj);
	
	//Search
	void* NBArray_data(const STNBArray* obj);
	void* NBArray_itemAtIndex(const STNBArray* obj, const SI32 index);
	void* NBArray_itemAtIndexWithSize(const STNBArray* obj, const SI32 index, const SI32 itemSize);
	SI32 NBArray_indexOf(const STNBArray* obj, const void* data, const SI32 dataSz);
	//Remove
	void NBArray_removeItemAtIndex(STNBArray* obj, const SI32 index);
	void NBArray_removeItemsAtIndex(STNBArray* obj, const SI32 index, const SI32 itemsCount);
	//Redim
	void NBArray_resignToBuffer(STNBArray* obj);
	BOOL NBArray_swapBuffers(STNBArray* obj, STNBArray* obj2);
	void NBArray_empty(STNBArray* obj);
    void NBArray_truncateBuffSize(STNBArray* obj, const SI32 qItems);
	SI32 NBArray_getBuffSize(STNBArray* obj);
	void NBArray_growBuffer(STNBArray* obj, const SI32 qItems);
	void NBArray_prepareForGrowth(STNBArray* obj, const SI32 qItems);
	void* NBArray_add(STNBArray* obj, const void* data, const SI32 itemSize);
	void* NBArray_addItems(STNBArray* obj, const void* data, const SI32 itemSize, const SI32 itemsCount);
	void* NBArray_addItemsAtIndex(STNBArray* obj, const SI32 index, const void* data, const SI32 itemSize, const SI32 itemsCount);
	void* NBArray_setItemAt(STNBArray* obj, const SI32 index, const void* data, const SI32 itemSize);
	//File
	SI32 NBArray_writeToFile(const STNBArray* obj, STNBFileRef file, const BOOL includeUnusedBuffer);
	SI32 NBArray_initFromFile(STNBArray* obj, STNBFileRef file, BYTE* optExternalBuffer);
	//Debug
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	void NBArrays_dbgTestAddItems(const UI32 totalTests, const UI32 ciclesPerTest, const UI8 printLevel);
#	endif
#ifdef __cplusplus
} //extern "C"
#endif

#endif
