//
//  XUCadena.c
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBArraySorted.h"
#include "nb/core/NBMemory.h"

#define NB_ARRAY_SORTED_VERIF_START						881
#define NB_ARRAY_SORTED_VERIF_END						991
//
#define NB_ARRAY_SORTED_SIZE_FOR_EXTERNAL_BUFFER		9999999	//unknown size
#define NB_ARRAY_SORTED_DEFAULT_GROWTH_MIN				32
#define NB_ARRAY_SORTED_DEFAULT_GROWTH_REL_EXTRA		0.5		//After 1.0f
#define NB_ARRAY_SORTED_DEFAULT_GROWTH_REL_PRECISION	16		//growthRel is stored in fixed precision

//Factory

void NBArraySorted_init(STNBArraySorted* obj, const SI32 bytesPerItem, NBCompareFunc cmpFunc){
	NBASSERT(bytesPerItem > 0)
	obj->use			= 0;
	obj->_bytesPerItem	= bytesPerItem;
	obj->_cmpFunc		= cmpFunc; NBASSERT(cmpFunc != NULL) //Required
	//
	obj->_buffData		= NULL;
	obj->_buffSize		= 0;
	obj->_buffGrowthMin	= NB_ARRAY_SORTED_DEFAULT_GROWTH_MIN; NBASSERT(obj->_buffGrowthMin > 0)
	obj->_buffGrowthRel	= NB_ARRAY_SORTED_DEFAULT_GROWTH_REL_EXTRA * NB_ARRAY_SORTED_DEFAULT_GROWTH_REL_PRECISION; NBASSERT(obj->_buffGrowthRel >= 0)
	obj->_buffIsExtern	= FALSE;
}

void NBArraySorted_initWithOther(STNBArraySorted* obj, const STNBArraySorted* other){
	obj->use			= other->use;
	obj->_bytesPerItem	= other->_bytesPerItem;
	obj->_cmpFunc		= other->_cmpFunc; NBASSERT(other->_cmpFunc != NULL) //Required
	//
	obj->_buffData		= NULL;
	obj->_buffSize		= other->use;
	obj->_buffGrowthMin	= other->_buffGrowthMin; NBASSERT(obj->_buffGrowthMin > 0)
	obj->_buffGrowthRel	= other->_buffGrowthRel; NBASSERT(obj->_buffGrowthRel >= 0)
	obj->_buffIsExtern	= FALSE;
	//
	if(obj->_buffSize > 0){
		obj->_buffData	= (BYTE*)NBMemory_alloc(obj->_buffSize * obj->_bytesPerItem);
		NBMemory_copy(obj->_buffData, other->_buffData, (other->use * other->_bytesPerItem));
	}
}

void NBArraySorted_initWithSz(STNBArraySorted* obj, const SI32 bytesPerItem, NBCompareFunc cmpFunc, const SI32 initialSize, const SI32 growthMin, const float growthRelExtra /*after 1.0*/){
	NBASSERT(bytesPerItem > 0)
	NBASSERT(initialSize >= 0)
	NBASSERT(growthMin > 0 && growthRelExtra >= 0)
	obj->use			= 0;
	obj->_bytesPerItem	= bytesPerItem;
	obj->_cmpFunc		= cmpFunc; NBASSERT(cmpFunc != NULL) //Required
	//
	if(initialSize > 0){
		obj->_buffSize	= initialSize;
		obj->_buffData	= (BYTE*)NBMemory_alloc(obj->_buffSize * obj->_bytesPerItem);
	} else {
		obj->_buffSize	= 0;
		obj->_buffData	= NULL;
	}
	obj->_buffGrowthMin	= (growthMin < 1 ? 1 : growthMin); NBASSERT(obj->_buffGrowthMin > 0)
	obj->_buffGrowthRel	= (growthRelExtra < 0.0f ? 0.0f : growthRelExtra) * NB_ARRAY_SORTED_DEFAULT_GROWTH_REL_PRECISION; NBASSERT(obj->_buffGrowthRel >= 0)
	obj->_buffIsExtern	= FALSE;
}

void NBArraySorted_initWithExternalBuffer(STNBArraySorted* obj, const SI32 bytesPerItem, NBCompareFunc cmpFunc, BYTE* externalBuffer){
	NBASSERT(bytesPerItem > 0)
	NBASSERT(externalBuffer != NULL)
	obj->use			= 0;
	obj->_bytesPerItem	= bytesPerItem;
	obj->_cmpFunc		= cmpFunc; NBASSERT(cmpFunc != NULL) //Required
	//
	obj->_buffSize		= NB_ARRAY_SORTED_SIZE_FOR_EXTERNAL_BUFFER;
	obj->_buffData		= externalBuffer;
	obj->_buffGrowthMin	= NB_ARRAY_SORTED_DEFAULT_GROWTH_MIN; NBASSERT(obj->_buffGrowthMin > 0)
	obj->_buffGrowthRel	= NB_ARRAY_SORTED_DEFAULT_GROWTH_REL_EXTRA * NB_ARRAY_SORTED_DEFAULT_GROWTH_REL_PRECISION; NBASSERT(obj->_buffGrowthRel >= 0)
	obj->_buffIsExtern	= TRUE;
}

void NBArraySorted_release(STNBArraySorted* obj){
	obj->use			= 0;
	obj->_bytesPerItem	= 0;
	obj->_cmpFunc		= NULL;
	//
	obj->_buffSize		= 0;
	if(obj->_buffData != NULL){
		if(!obj->_buffIsExtern){
			NBMemory_free(obj->_buffData);
		}
		obj->_buffData	= NULL;
	}
	obj->_buffGrowthMin	= 0;
	obj->_buffGrowthRel	= 0;
	obj->_buffIsExtern	= FALSE;
}

//Search

void* NBArraySorted_data(const STNBArraySorted* obj){
	return obj->_buffData;
}

void* NBArraySorted_itemAtIndex(const STNBArraySorted* obj, const SI32 i){
	void* r = NULL;
	NBASSERT(i >= 0 && i < obj->use)
	if(i >= 0 && i < obj->use){
		r = &obj->_buffData[i * obj->_bytesPerItem];
	}
	return r;
}

void* NBArraySorted_itemAtIndexWithSize(const STNBArraySorted* obj, const SI32 i, const SI32 itemSize){
	void* r = NULL;
	NBASSERT(itemSize == obj->_bytesPerItem)
	NBASSERT(i >= 0 && i < obj->use)
	if(itemSize == obj->_bytesPerItem && i >= 0 && i < obj->use){
		r = &obj->_buffData[i * obj->_bytesPerItem];
	}
	return r;
}

SI32 NBArraySorted_indexOf(const STNBArraySorted* obj, const void* data, const SI32 dataSz, const SI32* optPosHint){
	NBASSERT(dataSz == obj->_bytesPerItem)
	if(obj->use > 0){
		SI32 posEnd		= (obj->use - 1);
		if(optPosHint != NULL){
			//Optimization: search first on the position-hint.
			//This optimize the search when looking for items in order.
			if(*optPosHint >= 0 && *optPosHint < obj->use){
				if((*obj->_cmpFunc)(ENCompareMode_Equal, data, &obj->_buffData[*optPosHint * obj->_bytesPerItem], obj->_bytesPerItem)){
					return *optPosHint;
				}
			}
		} else {
			//Optimization: only search when the searched item is last item is lower  than the last item in the array.
			//This optimize the search for new items to add at the end (by example: when reading and ordered table).
			if((*obj->_cmpFunc)(ENCompareMode_Equal, data, &obj->_buffData[posEnd * obj->_bytesPerItem], obj->_bytesPerItem)){
				return posEnd;
			} else if((*obj->_cmpFunc)(ENCompareMode_Greater, data, &obj->_buffData[posEnd * obj->_bytesPerItem], obj->_bytesPerItem)){
				//Optimization: only search when the searched-item is last item is lower than the last-item in the array.
				return -1;
			}
		}
		//Binary Search
		{
			SI32 posStart	= 0;
			SI32 posMidd;
			const BYTE* dataMidd = NULL;
			NBASSERT(obj->_bytesPerItem > 0)
			while(posStart <= posEnd){
				posMidd		= posStart + ((posEnd - posStart)/2);
				dataMidd	= &obj->_buffData[posMidd * obj->_bytesPerItem];
				if((*obj->_cmpFunc)(ENCompareMode_Equal, dataMidd, data, obj->_bytesPerItem)){
					return posMidd;
				} else {
					if((*obj->_cmpFunc)(ENCompareMode_Lower, data, dataMidd, obj->_bytesPerItem)){
						posEnd		= posMidd - 1;
					} else {
						posStart	= posMidd + 1;
					}
				}
			}
		}
	}
	return -1;
}

SI32 NBArraySorted_indexForNew(const STNBArraySorted* obj, const void* dataToInsert){
	//Binary search for new item's position
	SI32 r = -1;
	NBASSERT(obj->_cmpFunc != NULL)
	NBASSERT(obj->_bytesPerItem > 0)
	if(obj->_cmpFunc != NULL){
		r = 0;
		if(obj->use > 0){
			SI32 posStart		= 0;
			SI32 posEnd			= (obj->use - 1);
			do {
				if((*obj->_cmpFunc)(ENCompareMode_LowerOrEqual, &obj->_buffData[posEnd * obj->_bytesPerItem], dataToInsert, obj->_bytesPerItem)){
					r			= posEnd + 1;
					break;
				} else if((*obj->_cmpFunc)(ENCompareMode_GreaterOrEqual, &obj->_buffData[posStart * obj->_bytesPerItem], dataToInsert, obj->_bytesPerItem)){
					r			= posStart;
					break;
				} else {
					const UI32 posMidd = (posStart + posEnd) / 2;
					if((*obj->_cmpFunc)(ENCompareMode_LowerOrEqual, &obj->_buffData[posMidd * obj->_bytesPerItem], dataToInsert, obj->_bytesPerItem)){
						posStart = posMidd + 1;
					} else {
						posEnd	= posMidd;
					}
				}
			} while(1);
		}
	}
	return r;
}

//Remove

void NBArraySorted_removeItemAtIndex(STNBArraySorted* obj, const SI32 index){
	NBArraySorted_removeItemsAtIndex(obj, index, 1);
}

void NBArraySorted_removeItemsAtIndex(STNBArraySorted* obj, const SI32 index, const SI32 itemsCount){
	SI32 i, iCount;
	NBASSERT(index >= 0 && (index + itemsCount) <= obj->use)
	//reordenar el arreglo
	obj->use -= itemsCount;
	iCount = obj->use;
	for(i = index; i < iCount; i++){
		NBMemory_copy(&obj->_buffData[i * obj->_bytesPerItem], &obj->_buffData[(i + itemsCount) * obj->_bytesPerItem], obj->_bytesPerItem);
	}
}

//Redim

void NBArraySorted_resignToBuffer(STNBArraySorted* obj){
	obj->_buffData	= NULL;
	obj->_buffSize	= 0;
	obj->use		= 0;
}

void NBArraySorted_empty(STNBArraySorted* obj){
	obj->use = 0;
}

void NBArraySorted_growBuffer(STNBArraySorted* obj, const SI32 qItems){
	NBASSERT(!obj->_buffIsExtern)
	if(qItems > 0 && !obj->_buffIsExtern){
		BYTE* newArr = NULL;
		obj->_buffSize += qItems;
		newArr = (BYTE*)NBMemory_alloc(obj->_buffSize * obj->_bytesPerItem);
		if(obj->_buffData != NULL){
			NBMemory_copy(newArr, obj->_buffData, (obj->use * obj->_bytesPerItem));
			NBMemory_free(obj->_buffData);
		}
		obj->_buffData = newArr;
	}
}

void* NBArraySorted_add(STNBArraySorted* obj, const void* data, const SI32 itemSize){
	void* r = NULL;
	NBASSERT(itemSize == obj->_bytesPerItem)
	if(itemSize == obj->_bytesPerItem){
		//Grow, if necesary
		NBASSERT(obj->use <= obj->_buffSize)
		if(obj->use == obj->_buffSize){
			const SI32 relGrowth = (obj->_buffSize * obj->_buffGrowthRel) / NB_ARRAY_SORTED_DEFAULT_GROWTH_REL_PRECISION;
			NBArraySorted_growBuffer(obj, (relGrowth < obj->_buffGrowthMin ? obj->_buffGrowthMin : relGrowth));
			NBASSERT(obj->use < obj->_buffSize)
		}
		//Add
		const SI32 dstIndex = NBArraySorted_indexForNew(obj, (BYTE*)data); NBASSERT(dstIndex >= 0 && dstIndex <= obj->use)
		//PRINTF_INFO("Adding to index %d of %d.\n", dstIndex, obj->use);
		//Make room for the new record
		if(dstIndex < obj->use){
			SI32 i; for(i = (obj->use - 1); i >= dstIndex; i--){
				NBMemory_copy(&obj->_buffData[(i + 1) * obj->_bytesPerItem], &obj->_buffData[i * obj->_bytesPerItem], obj->_bytesPerItem);
			}
		}
		//Set record value
		NBMemory_copy(&obj->_buffData[dstIndex * obj->_bytesPerItem], data, obj->_bytesPerItem);
		obj->use++;
		r = &obj->_buffData[dstIndex * obj->_bytesPerItem];
		//
		//IF_NBASSERT(NBArraySorted_dbgValidate(obj);)
	}
	return r;
}

void NBArraySorted_addItems(STNBArraySorted* obj, const void* data, const SI32 itemSize, const SI32 itemsCount){
	NBASSERT(itemSize == obj->_bytesPerItem)
	if(itemSize == obj->_bytesPerItem){
		//Grow, if necesary
		if((obj->use + itemsCount) > obj->_buffSize){
			const SI32 needed		= (obj->use + itemsCount) - obj->_buffSize;
			const SI32 relGrowth	= (obj->_buffSize * obj->_buffGrowthRel) / NB_ARRAY_SORTED_DEFAULT_GROWTH_REL_PRECISION;
			const SI32 growth		= (relGrowth < needed ? (needed < obj->_buffGrowthMin ? obj->_buffGrowthMin : needed) : relGrowth);
			NBArraySorted_growBuffer(obj, growth);
			NBASSERT((obj->use + itemsCount) <= obj->_buffSize)
		}
		//Add individually (will be ordered)
		SI32 i; for(i = 0; i < itemsCount; i++){
			NBArraySorted_add(obj, &((BYTE*)data)[i * itemSize], itemSize);
		}
	}
}

//File
SI32 NBArraySorted_writeToFile(const STNBArraySorted* obj, STNBFileRef file, const BOOL includeUnusedBuffer){
	const SI32 verfiStart		= NB_ARRAY_SORTED_VERIF_START, verifEnd = NB_ARRAY_SORTED_VERIF_END;
	const SI32 incUnusedBuffer	= (includeUnusedBuffer != FALSE ? 1 : 0);
	const SI32 itemsToWrite		= (includeUnusedBuffer != FALSE ? obj->_buffSize : obj->use);
	if(obj == NULL || !NBFile_isSet(file)){
		NBASSERT(0); return -1;
	}
	NBASSERT(obj->use == 0 || obj->_buffData != NULL)
	if(NBFile_write(file, &verfiStart, sizeof(verfiStart)) != sizeof(verfiStart)){
		NBASSERT(0); return -1;
	}
	if(NBFile_write(file, &obj->use, sizeof(obj->use)) != sizeof(obj->use)){
		NBASSERT(0); return -1;
	}
	if(NBFile_write(file, &incUnusedBuffer, sizeof(incUnusedBuffer)) != sizeof(incUnusedBuffer)){
		NBASSERT(0); return -1;
	}
	if(incUnusedBuffer){
		if(NBFile_write(file, &obj->_buffSize, sizeof(obj->_buffSize)) != sizeof(obj->_buffSize)){
			NBASSERT(0); return -1;
		}
	}
	if(NBFile_write(file, &obj->_bytesPerItem, sizeof(obj->_bytesPerItem)) != sizeof(obj->_bytesPerItem)){
		NBASSERT(0); return -1;
	}
	if(NBFile_write(file, &obj->_buffGrowthMin, sizeof(obj->_buffGrowthMin)) != sizeof(obj->_buffGrowthMin)){
		NBASSERT(0); return -1;
	}
	if(NBFile_write(file, &obj->_buffGrowthRel, sizeof(obj->_buffGrowthRel)) != sizeof(obj->_buffGrowthRel)){
		NBASSERT(0); return -1;
	}
	if(itemsToWrite > 0 && !obj->_buffIsExtern){
		if(NBFile_write(file, obj->_buffData, (obj->_bytesPerItem * itemsToWrite)) != (obj->_bytesPerItem * itemsToWrite)){
			NBASSERT(0); return -1;
		}
	}
	if(NBFile_write(file, &verifEnd, sizeof(verifEnd)) != sizeof(verifEnd)){
		NBASSERT(0); return -1;
	}
	return 0;
}

SI32 NBArraySorted_initFromFile(STNBArraySorted* obj, STNBFileRef file, BYTE* optExternalBuffer){
	SI32 verfiStart, verifEnd; SI32 incUnusedBuffer; SI32 sizeToInit = 0;
	if(obj == NULL || !NBFile_isSet(file)){
		NBASSERT_LOADFILE(0); return -1;
	}
	if(NBFile_read(file, &verfiStart, sizeof(verfiStart)) != sizeof(verfiStart)){
		NBASSERT_LOADFILE(0); return -1;
	} else if(verfiStart != NB_ARRAY_SORTED_VERIF_START){
		NBASSERT_LOADFILE(0); return -1;
	}
	if(NBFile_read(file, &obj->use, sizeof(obj->use)) != sizeof(obj->use)){
		NBASSERT_LOADFILE(0); return -1;
	}
	if(NBFile_read(file, &incUnusedBuffer, sizeof(incUnusedBuffer)) != sizeof(incUnusedBuffer)){
		NBASSERT_LOADFILE(0); return -1;
	}
	if(incUnusedBuffer){
		if(NBFile_read(file, &obj->_buffSize, sizeof(obj->_buffSize)) != sizeof(obj->_buffSize)){
			NBASSERT_LOADFILE(0); return -1;
		}
		sizeToInit = obj->_buffSize;
	} else {
		sizeToInit = obj->use;
	}
	if(NBFile_read(file, &obj->_bytesPerItem, sizeof(obj->_bytesPerItem)) != sizeof(obj->_bytesPerItem)){
		NBASSERT_LOADFILE(0); return -1;
	}
	if(NBFile_read(file, &obj->_buffGrowthMin, sizeof(obj->_buffGrowthMin)) != sizeof(obj->_buffGrowthMin)){
		NBASSERT_LOADFILE(0); return -1;
	}
	if(NBFile_read(file, &obj->_buffGrowthRel, sizeof(obj->_buffGrowthRel)) != sizeof(obj->_buffGrowthRel)){
		NBASSERT_LOADFILE(0); return -1;
	}
	if(obj->use < 0 || obj->_bytesPerItem <= 0 || obj->_buffGrowthMin <= 0){
		NBASSERT_LOADFILE(0); return -1;
	}
	obj->_buffSize = sizeToInit;
	if(optExternalBuffer != NULL){
		obj->_buffData		= optExternalBuffer;
		obj->_buffIsExtern	= TRUE;
		NBASSERT(obj->_buffSize == NB_ARRAY_SORTED_SIZE_FOR_EXTERNAL_BUFFER)
	} else {
		obj->_buffData		= NULL;
		obj->_buffIsExtern	= FALSE;
		if(obj->_buffSize > 0){
			obj->_buffData = (BYTE*)NBMemory_alloc(obj->_bytesPerItem * obj->_buffSize);
			if(obj->_buffData == NULL){
				NBASSERT_LOADFILE(0); return -1;
			} else {
				if(NBFile_read(file, obj->_buffData, (obj->_bytesPerItem * obj->_buffSize)) != (obj->_bytesPerItem * obj->_buffSize)){
					NBMemory_free(obj->_buffData);
					NBASSERT_LOADFILE(0); return -1;
				}
			}
		}
	}
	if(NBFile_read(file, &verifEnd, sizeof(verifEnd)) != sizeof(verifEnd)){
		if(obj->_buffData != NULL){ NBMemory_free(obj->_buffData); obj->_buffData = NULL; }
		NBASSERT_LOADFILE(0); return -1;
	} else if(verifEnd != NB_ARRAY_SORTED_VERIF_END){
		if(obj->_buffData != NULL){ NBMemory_free(obj->_buffData); obj->_buffData = NULL; }
		NBASSERT_LOADFILE(0); return -1;
	}
	return 0;
}

//Debug

#ifdef NB_CONFIG_INCLUDE_ASSERTS
void NBArraySorted_dbgValidate(STNBArraySorted* obj){
    NBASSERT(obj->_bytesPerItem > 0)
	NBASSERT(obj->_cmpFunc != NULL)
	if(obj->_cmpFunc != NULL && obj->use > 1){
		SI32 i;
		BYTE* dataBefore = (BYTE*)NBArraySorted_itemAtIndex(obj, 0);
		for(i = 1; i < obj->use; i++){
			BYTE* data = (BYTE*)NBArraySorted_itemAtIndex(obj, i);
		    NBASSERT((*obj->_cmpFunc)(ENCompareMode_LowerOrEqual, dataBefore, data, obj->_bytesPerItem)) // <=
			dataBefore = data;
		}
	}
}
#endif



