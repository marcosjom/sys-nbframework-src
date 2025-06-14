//
//  NBArray.h
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#ifndef NB_ARRAY_SORTED_H
#define NB_ARRAY_SORTED_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBCompare.h"
#include "nb/files/NBFile.h"

#define NBArraySorted_addValue(PTR_ARR, VALUE)					NBArraySorted_add(PTR_ARR, &VALUE, sizeof(VALUE))
#define NBArraySorted_addValueCopy(PTR_ARR, TYPE, VALUE)		{ const TYPE tmp = VALUE; NBArraySorted_add(PTR_ARR, &tmp, sizeof(tmp)); }
#define NBArraySorted_dataPtr(PTR_ARR, TYPE)					((TYPE*)NBArraySorted_data(PTR_ARR))
#define NBArraySorted_itmPtrAtIndex(PTR_ARR, TYPE, IDX)			((TYPE*)NBArraySorted_itemAtIndexWithSize(PTR_ARR, IDX, sizeof(TYPE)))
#define NBArraySorted_itmValueAtIndex(PTR_ARR, TYPE, IDX)		(*((TYPE*)NBArraySorted_itemAtIndexWithSize(PTR_ARR, IDX, sizeof(TYPE))))
#define NBArraySorted_indexOfValue(PTR_ARR, VALUE)				NBArraySorted_indexOf(PTR_ARR, &VALUE, sizeof(VALUE), NULL)

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBArraySorted_ {
		SI32			use;			//Buffer use count (in item units)
		SI32			_bytesPerItem;	//Size of each element, in bytes
		NBCompareFunc	_cmpFunc;		//Items compare function
		//
		BYTE*			_buffData;		//Buffer data
		SI32			_buffSize;		//Buffer curr max size (in items units)
		SI32			_buffGrowthMin;	//Growth minimun of the array buffer size when the limit is reached
		UI16			_buffGrowthRel;	//Growth exponent in 1/16 units (1.0 is added to this value)
		BOOL			_buffIsExtern;
	} STNBArraySorted;
	
	void NBArraySorted_init(STNBArraySorted* obj, const SI32 bytesPerItem, NBCompareFunc cmpFunc);
	void NBArraySorted_initWithOther(STNBArraySorted* obj, const STNBArraySorted* other);
	void NBArraySorted_initWithSz(STNBArraySorted* obj, const SI32 bytesPerItem, NBCompareFunc cmpFunc, const SI32 initialSize, const SI32 growthMin, const float growthRelExtra /*after 1.0*/);
	void NBArraySorted_initWithExternalBuffer(STNBArraySorted* obj, const SI32 bytesPerItem, NBCompareFunc cmpFunc, BYTE* externalBuffer);
	void NBArraySorted_release(STNBArraySorted* obj);
	//Search
	void* NBArraySorted_data(const STNBArraySorted* obj);
	void* NBArraySorted_itemAtIndex(const STNBArraySorted* obj, const SI32 index);
	void* NBArraySorted_itemAtIndexWithSize(const STNBArraySorted* obj, const SI32 index, const SI32 itemSize);
	SI32 NBArraySorted_indexOf(const STNBArraySorted* obj, const void* data, const SI32 dataSz, const SI32* optPosHint);
	SI32 NBArraySorted_indexForNew(const STNBArraySorted* obj, const void* dataToInsert); //SORTED-ONLY
	//Remove
	void NBArraySorted_removeItemAtIndex(STNBArraySorted* obj, const SI32 index);
	void NBArraySorted_removeItemsAtIndex(STNBArraySorted* obj, const SI32 index, const SI32 itemsCount);
	//Redim
	void NBArraySorted_resignToBuffer(STNBArraySorted* obj);
	void NBArraySorted_empty(STNBArraySorted* obj);
	void NBArraySorted_growBuffer(STNBArraySorted* obj, const SI32 qItems);
	void* NBArraySorted_add(STNBArraySorted* obj, const void* data, const SI32 itemSize);
	void NBArraySorted_addItems(STNBArraySorted* obj, const void* data, const SI32 itemSize, const SI32 itemsCount);
	//File
	SI32 NBArraySorted_writeToFile(const STNBArraySorted* obj, STNBFileRef file, const BOOL includeUnusedBuffer);
	SI32 NBArraySorted_initFromFile(STNBArraySorted* obj, STNBFileRef file, BYTE* optExternalBuffer);
	//
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	void NBArraySorted_dbgValidate(STNBArraySorted* obj);
#	endif
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
