//
//  XUCadena.c
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBMemory.h"

#define NB_ARRAY_VERIF_START					888
#define NB_ARRAY_VERIF_END						999
//
#define NB_ARRAY_SIZE_FOR_EXTERNAL_BUFFER		9999999	//unknown size
#define NB_ARRAY_DEFAULT_GROWTH_MIN				32
#define NB_ARRAY_DEFAULT_GROWTH_REL_EXTRA		0.5		//After 1.0f
#define NB_ARRAY_DEFAULT_GROWTH_REL_PRECISION	16		//growthRel is stored in fixed precision

//Factory

void NBArray_init(STNBArray* obj, const SI32 bytesPerItem, NBCompareFunc cmpFunc){
	NBASSERT(bytesPerItem > 0)
	obj->use			= 0;
	obj->_bytesPerItem	= bytesPerItem;
	obj->_cmpFunc		= cmpFunc;
	//
	obj->_buffData		= NULL;
	obj->_buffSize		= 0;
	obj->_buffGrowthMin	= NB_ARRAY_DEFAULT_GROWTH_MIN; NBASSERT(obj->_buffGrowthMin > 0)
	obj->_buffGrowthRel	= NB_ARRAY_DEFAULT_GROWTH_REL_EXTRA * NB_ARRAY_DEFAULT_GROWTH_REL_PRECISION; NBASSERT(obj->_buffGrowthRel >= 0)
	obj->_buffIsExtern	= FALSE;
}

void NBArray_initWithOther(STNBArray* obj, const STNBArray* other){
	obj->use			= other->use;
	obj->_bytesPerItem	= other->_bytesPerItem;
	obj->_cmpFunc		= other->_cmpFunc;
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

void NBArray_initWithSz(STNBArray* obj, const SI32 bytesPerItem, NBCompareFunc cmpFunc, const SI32 initialSize, const SI32 growthMin, const float growthRelExtra /*after 1.0*/){
	NBASSERT(bytesPerItem > 0)
	NBASSERT(initialSize >= 0)
	NBASSERT(growthMin > 0 && growthRelExtra >= 0)
	obj->use			= 0;
	obj->_bytesPerItem	= bytesPerItem;
	obj->_cmpFunc		= cmpFunc;
	//
	if(initialSize > 0){
		obj->_buffSize	= initialSize;
		obj->_buffData	= (BYTE*)NBMemory_alloc(obj->_buffSize * obj->_bytesPerItem);
	} else {
		obj->_buffSize	= 0;
		obj->_buffData	= NULL;
	}
	obj->_buffGrowthMin	= (growthMin < 1 ? 1 : growthMin); NBASSERT(obj->_buffGrowthMin > 0)
	obj->_buffGrowthRel	= (growthRelExtra < 0.0f ? 0.0f : growthRelExtra) * NB_ARRAY_DEFAULT_GROWTH_REL_PRECISION; NBASSERT(obj->_buffGrowthRel >= 0)
	obj->_buffIsExtern	= FALSE;
}

void NBArray_initWithExternalBuffer(STNBArray* obj, const SI32 bytesPerItem, NBCompareFunc cmpFunc, BYTE* externalBuffer){
	NBASSERT(bytesPerItem > 0)
	NBASSERT(externalBuffer != NULL)
	obj->use			= 0;
	obj->_bytesPerItem	= bytesPerItem;
	obj->_cmpFunc		= cmpFunc;
	//
	obj->_buffSize		= NB_ARRAY_SIZE_FOR_EXTERNAL_BUFFER;
	obj->_buffData		= externalBuffer;
	obj->_buffGrowthMin	= NB_ARRAY_DEFAULT_GROWTH_MIN; NBASSERT(obj->_buffGrowthMin > 0)
	obj->_buffGrowthRel	= NB_ARRAY_DEFAULT_GROWTH_REL_EXTRA * NB_ARRAY_DEFAULT_GROWTH_REL_PRECISION; NBASSERT(obj->_buffGrowthRel >= 0)
	obj->_buffIsExtern	= TRUE;
}

void NBArray_release(STNBArray* obj){
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

void* NBArray_data(const STNBArray* obj){
	return obj->_buffData;
}

void* NBArray_itemAtIndex(const STNBArray* obj, const SI32 i){
	void* r = NULL;
    NBASSERT(i >= 0 && i < obj->use)
	if(i >= 0 && i < obj->use){
		r = &obj->_buffData[i * obj->_bytesPerItem];
	}
	return r;
}

void* NBArray_itemAtIndexWithSize(const STNBArray* obj, const SI32 i, const SI32 itemSize){
	void* r = NULL;
	NBASSERT(itemSize == obj->_bytesPerItem)
	NBASSERT(i >= 0 && i < obj->use)
	if(itemSize == obj->_bytesPerItem && i >= 0 && i < obj->use){
		r = &obj->_buffData[i * obj->_bytesPerItem];
	}
	return r;
}

SI32 NBArray_indexOf(const STNBArray* obj, const void* data, const SI32 dataSz){
	SI32 r = -1;
    NBASSERT(dataSz == obj->_bytesPerItem)
	NBASSERT(obj->_cmpFunc != NULL)
	if(dataSz == obj->_bytesPerItem && obj->_cmpFunc != NULL){
		SI32 i; for(i = 0; i < obj->use; i++){
			const void* data2 = &obj->_buffData[i * obj->_bytesPerItem];
			if((*obj->_cmpFunc)(ENCompareMode_Equal, data, data2, obj->_bytesPerItem)){
				r = i;
				break;
			}
		}
	}
	return r;
}

//Remove

void NBArray_removeItemAtIndex(STNBArray* obj, const SI32 index){
    NBArray_removeItemsAtIndex(obj, index, 1);
}

void NBArray_removeItemsAtIndex(STNBArray* obj, const SI32 index, const SI32 itemsCount){
	SI32 i, iCount;
    NBASSERT(index >= 0 && (index + itemsCount) <= obj->use)
	//reordenar el arreglo
	obj->use	-= itemsCount;
	iCount		= obj->use;
	for(i = index; i < iCount; i++){
		NBMemory_copy(&obj->_buffData[i * obj->_bytesPerItem], &obj->_buffData[(i + itemsCount) * obj->_bytesPerItem], obj->_bytesPerItem);
	}
}

//Redim

void NBArray_resignToBuffer(STNBArray* obj){
	obj->_buffData	= NULL;
	obj->_buffSize	= 0;
	obj->use		= 0;
}

BOOL NBArray_swapBuffers(STNBArray* obj, STNBArray* obj2){
	BOOL r = FALSE;
	NBASSERT(obj->_bytesPerItem == obj2->_bytesPerItem)
	if(obj->_bytesPerItem == obj2->_bytesPerItem && obj->_cmpFunc == obj2->_cmpFunc){
		STNBArray tmp = *obj;
		*obj = *obj2;
		*obj2 = tmp;
		r = TRUE;
	} NBASSERT(r)
	return r;
}

void NBArray_empty(STNBArray* obj){
	obj->use = 0;
}

void NBArray_truncateBuffSize(STNBArray* obj, const SI32 qItems){
    NBASSERT(!obj->_buffIsExtern)
    if(!obj->_buffIsExtern && qItems < obj->_buffSize){
        BYTE* newArr = NULL;
        obj->_buffSize = qItems;
        if(obj->use > qItems){
            obj->use = qItems;
        }
        if(obj->_buffSize > 0 && obj->_bytesPerItem > 0){
            newArr = (BYTE*)NBMemory_alloc(obj->_buffSize * obj->_bytesPerItem);
        }
        if(obj->_buffData != NULL){
            if(newArr != NULL){
                NBMemory_copy(newArr, obj->_buffData, (obj->use * obj->_bytesPerItem));
            }
            NBMemory_free(obj->_buffData);
        }
        obj->_buffData = newArr;
    }
}

SI32 NBArray_getBuffSize(STNBArray* obj){
	return obj->_buffSize;
}

void NBArray_growBuffer(STNBArray* obj, const SI32 qItems){
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

void NBArray_prepareForGrowth(STNBArray* obj, const SI32 qItems) {
	NBASSERT(!obj->_buffIsExtern)
	if (qItems > 0 && !obj->_buffIsExtern) {
		const SI32 nSz = obj->use + qItems;
		if (nSz > obj->_buffSize) {
			NBArray_growBuffer(obj, nSz - obj->_buffSize);
		}
	}
}

void* NBArray_add(STNBArray* obj, const void* data, const SI32 itemSize){
	void* r = NULL;
	NBASSERT(itemSize == obj->_bytesPerItem)
	if(itemSize == obj->_bytesPerItem){
		//Grow, if necesary
		NBASSERT(obj->use <= obj->_buffSize)
		if(obj->use == obj->_buffSize){
			const SI32 relGrowth = (obj->_buffSize * obj->_buffGrowthRel) / NB_ARRAY_DEFAULT_GROWTH_REL_PRECISION;
			NBArray_growBuffer(obj, (relGrowth < obj->_buffGrowthMin ? obj->_buffGrowthMin : relGrowth));
			NBASSERT(obj->use < obj->_buffSize)
		}
		r = &obj->_buffData[obj->use * obj->_bytesPerItem];
		if(data != NULL){
			NBMemory_copy(r, data, obj->_bytesPerItem);
		}
		obj->use++;
	}
	return r;
}

void* NBArray_addItems(STNBArray* obj, const void* data, const SI32 itemSize, const SI32 itemsCount){
	void* r = NULL;
	NBASSERT(itemSize == obj->_bytesPerItem)
	if(itemSize == obj->_bytesPerItem){
		//Grow, if necesary
		if((obj->use + itemsCount) > obj->_buffSize){
			const SI32 needed		= (obj->use + itemsCount) - obj->_buffSize;
			const SI32 relGrowth	= (obj->_buffSize * obj->_buffGrowthRel) / NB_ARRAY_DEFAULT_GROWTH_REL_PRECISION;
			const SI32 growth		= (relGrowth < needed ? (needed < obj->_buffGrowthMin ? obj->_buffGrowthMin : needed) : relGrowth);
			NBArray_growBuffer(obj, growth);
			NBASSERT((obj->use + itemsCount) <= obj->_buffSize)
		}
		r = &obj->_buffData[obj->use * obj->_bytesPerItem];
		//Copy data
		if(data != NULL && itemsCount > 0){
			NBMemory_copy(r, data, obj->_bytesPerItem * itemsCount);
		}
		obj->use += itemsCount;
	}
	return r;
}

void* NBArray_addItemsAtIndex(STNBArray* obj, const SI32 index, const void* data, const SI32 itemSize, const SI32 itemsCount){
	void* r = NULL;
	NBASSERT(itemSize == obj->_bytesPerItem)
	NBASSERT(index <= obj->use)
	if(itemSize == obj->_bytesPerItem && itemsCount > 0 && index <= obj->use){
		if((obj->use + itemsCount) > obj->_buffSize){
			//Grow and copy (optimization)
			const SI32 needed		= (obj->use + itemsCount) - obj->_buffSize;
			const SI32 relGrowth	= (obj->_buffSize * obj->_buffGrowthRel) / NB_ARRAY_DEFAULT_GROWTH_REL_PRECISION;
			//PRINTF_INFO("AddItems-by-buffer-growth from %d to %d.\n", obj->_buffSize, (obj->_buffSize + (relGrowth < needed ? (needed < obj->_buffGrowthMin ? obj->_buffGrowthMin : needed) : relGrowth)));
			BYTE* newArr			= NULL;
			obj->_buffSize			+= (relGrowth < needed ? (needed < obj->_buffGrowthMin ? obj->_buffGrowthMin : needed) : relGrowth);
			newArr = (BYTE*)NBMemory_alloc(obj->_buffSize * obj->_bytesPerItem);
			//Copy content
			{
				//Copy left-content
				if(obj->_buffData != NULL && index > 0){
					NBASSERT(index <= obj->use)
					NBMemory_copy(newArr, obj->_buffData, (index * obj->_bytesPerItem));
				}
				//Copy new content
				if(data != NULL && itemsCount > 0){
					NBMemory_copy(&newArr[index * obj->_bytesPerItem], data, itemsCount * obj->_bytesPerItem);
				}
				//Copy right-content
				NBASSERT(index <= obj->use)
				if(obj->_buffData != NULL && index < obj->use){
					NBMemory_copy(&newArr[(index + itemsCount) * obj->_bytesPerItem], &obj->_buffData[index * obj->_bytesPerItem], ((obj->use - index) * obj->_bytesPerItem));
				}
			}
			//Delete prev buffer
			if(obj->_buffData != NULL){
				NBMemory_free(obj->_buffData);
			}
			//Set new buffer and used-size
			obj->_buffData = newArr;
			obj->use += itemsCount;
		} else {
			//PRINTF_INFO("AddItems-by-making-room.\n");
			//Make room for data
			if(index < obj->use){
				NBASSERT(index <= obj->use)
				BYTE* itm = obj->_buffData + ((obj->use - 1) * obj->_bytesPerItem);
				const BYTE* first = obj->_buffData + (index * obj->_bytesPerItem);
				while(TRUE){
					BYTE* dst = itm + (itemsCount * obj->_bytesPerItem);
					NBMemory_copy(dst, itm, obj->_bytesPerItem);
					if(itm == first) break;
					itm -= obj->_bytesPerItem;
				}
			}
			//Copy new data
			if(data != NULL){
				NBMemory_copy(&obj->_buffData[index * obj->_bytesPerItem], data, itemsCount * obj->_bytesPerItem);
			}
			obj->use += itemsCount;
		}
	}
	r = &obj->_buffData[index * obj->_bytesPerItem];
	return r;
}

void* NBArray_setItemAt(STNBArray* obj, const SI32 index, const void* data, const SI32 itemSize){
	void* r = NULL;
    NBASSERT(itemSize == obj->_bytesPerItem)
	if(itemSize == obj->_bytesPerItem){
		if(index >= 0 && index < obj->use){
			r = &obj->_buffData[index * obj->_bytesPerItem];
			NBMemory_copy(r, data, obj->_bytesPerItem);
		}
	}
	return r;
}

//File
SI32 NBArray_writeToFile(const STNBArray* obj, STNBFileRef file, const BOOL includeUnusedBuffer){
	const SI32 verfiStart		= NB_ARRAY_VERIF_START, verifEnd = NB_ARRAY_VERIF_END;
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

SI32 NBArray_initFromFile(STNBArray* obj, STNBFileRef file, BYTE* optExternalBuffer){
	SI32 verfiStart, verifEnd; SI32 incUnusedBuffer; SI32 sizeToInit = 0;
	if(obj == NULL || !NBFile_isSet(file)){
	    NBASSERT_LOADFILE(0); return -1;
	}
	if(NBFile_read(file, &verfiStart, sizeof(verfiStart)) != sizeof(verfiStart)){
	    NBASSERT_LOADFILE(0); return -1;
	} else if(verfiStart != NB_ARRAY_VERIF_START){
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
	    NBASSERT(obj->_buffSize == NB_ARRAY_SIZE_FOR_EXTERNAL_BUFFER)
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
	} else if(verifEnd != NB_ARRAY_VERIF_END){
		if(obj->_buffData != NULL){ NBMemory_free(obj->_buffData); obj->_buffData = NULL; }
	    NBASSERT_LOADFILE(0); return -1;
	}
	return 0;
}

#ifdef NB_CONFIG_INCLUDE_ASSERTS
#include <stdlib.h>		//for rand()
#endif

#ifdef NB_CONFIG_INCLUDE_ASSERTS
void NBArrays_dbgTestAddItems(const UI32 totalTests, const UI32 ciclesPerTest, const UI8 printLevel){
	UI32 i, j;
	STNBArray strTest, str2, str3;
	NBArray_init(&strTest, sizeof(UI32), NULL);
	NBArray_init(&str2, sizeof(UI32), NULL);
	NBArray_init(&str3, sizeof(UI32), NULL);
	//
	const UI32 origMaxSz = 512;
	const UI32 replcMaxSz = 512;
	//Tests
	for(i = 0; i < totalTests; i++){
		//Random reset string (to reset buffer)
		if((rand() % 100) < 2){
			NBArray_release(&strTest);
			NBArray_initWithSz(&strTest, sizeof(UI32), NULL, 1 + (rand() % 100), 1 + (rand() % 7), 1.5);
		}
		//Set initial value
		{
			UI32 i; const UI32 charsCount = (rand() % origMaxSz);
			NBArray_empty(&strTest);
			for(i = 0; i < charsCount; i++){
				const UI32 v = (rand() % 102400);
				NBArray_addValue(&strTest, v);
			}
		}
		//ciclesPerTest
		for(j = 0; j < ciclesPerTest; j++){
			//Test addItems
			{
				const UI32 start = (rand() % (strTest.use + 1));
				//Set add values
				{
					UI32 i; const UI32 charsCount = (rand() % replcMaxSz);
					NBArray_empty(&str2);
					for(i = 0; i < charsCount; i++){
						const UI32 v = (rand() % 102400);
						NBArray_addValue(&str2, v);
					}
				}
				//Clone
				NBArray_release(&str3);
				NBArray_initWithOther(&str3, &strTest);
				//Action
				NBArray_addItemsAtIndex(&strTest, start, str2._buffData, sizeof(UI32), str2.use);
				//Compare
				{
					UI32 i = 0;
					//Compare left
					for(; i < start; i++){
						const UI32* finl = NBArray_itmPtrAtIndex(&strTest, UI32, i);
						const UI32* orig = NBArray_itmPtrAtIndex(&str3, UI32, i);
						NBASSERT(*finl == *orig)
					}
					//Compare added
					for(; i < (start + str2.use); i++){
						const UI32* finl = NBArray_itmPtrAtIndex(&strTest, UI32, i);
						const UI32* orig = NBArray_itmPtrAtIndex(&str2, UI32, (i - start));
						NBASSERT(*finl == *orig)
					}
					//Compare right
					for(; i < (str3.use + str2.use); i++){
						const UI32* finl = NBArray_itmPtrAtIndex(&strTest, UI32, i);
						const UI32* orig = NBArray_itmPtrAtIndex(&str3, UI32, (i - str2.use));
						NBASSERT(*finl == *orig)
					}
					//Compare size
					NBASSERT(strTest.use == (str3.use + str2.use))
				}
				if(printLevel > 0){
					PRINTF_INFO("#%d-test-#%d-cicle arr(%d) addr(%d, +%d).\n", (i + 1), (j + 1), str3.use, start, str2.use);
				}
			}
			/*if((rand() % 2) == 0){
				const UI32 orgLen = strTest.length;
				//Test remove
				UI32 start = 0, size = 0;
				if(strTest.length > 0){
					start = (rand() % strTest.length) + (rand() % ((strTest.length < 5 ? 5 : strTest.length) / 5));
					size = (rand() % (strTest.length * 5) / 4);
				} else {
					start = (rand() % 3);
					size = (rand() % 3);
				}
				//Build compare
				{
					NBArray_empty(&str2);
					if(start > 0){
						NBArray_concatBytes(&str2, &strTest.str[0], (start < strTest.length ? start : strTest.length));
					}
					if((start + size) < strTest.length){
						NBArray_concatBytes(&str2, &strTest.str[start + size], strTest.length - start - size);
					}
				}
				//Replace
				if(printLevel > 1){
					PRINTF_INFO("-------------\n");
					PRINTF_INFO("Orig: %s.\n", strTest.str);
					PRINTF_INFO("Epct: %s.\n", str2.str);
				}
				if(printLevel > 0){
					PRINTF_INFO("#%d-test-#%d-cicle REMOVE(%d, +%d) on %d bytes%s%s%s.\n", (i + 1), (j + 1), start, size, orgLen, (start >= orgLen ? " start-out-of-rng" : ""), (start + size) == orgLen ? " remove-at-end" : "", (start + size) > orgLen ? " size-out-of-rng" : "");
				}
				NBArray_removeBytes(&strTest, start, size);
				if(printLevel > 1){
					PRINTF_INFO("Rslt: %s.\n", strTest.str);
				}
				//Compare
				NBASSERT(strTest.length == str2.length)
				NBASSERT(NBArray_strIsEqual(strTest.str, str2.str));
			} else {
				//Test replace
				const UI32 orgLen = strTest.length;
				//Test remove
				UI32 start = 0, size = 0;
				if(strTest.length > 0){
					start = (rand() % strTest.length) + (rand() % ((strTest.length < 5 ? 5 : strTest.length) / 5));
					size = (rand() % (strTest.length * 5) / 4);
				} else {
					start = (rand() % 3);
					size = (rand() % 3);
				}
				//Set replace value
				{
					UI32 i; const UI32 charsCount = (rand() % 64);
					NBArray_empty(&str3);
					for(i = 0; i < charsCount; i++){
						NBArray_concatByte(&str3, 'a' + (rand() % 10));
					}
				}
				//Build compare
				{
					NBArray_empty(&str2);
					if(start > 0){
						NBArray_concatBytes(&str2, &strTest.str[0], (start < strTest.length ? start : strTest.length));
					}
					NBArray_concatBytes(&str2, str3.str, str3.length);
					if((start + size) < strTest.length){
						NBArray_concatBytes(&str2, &strTest.str[start + size], strTest.length - start - size);
					}
				}
				//Replace
				if(printLevel > 1){
					PRINTF_INFO("-------------\n");
					PRINTF_INFO("Rplc: %s.\n", str3.str);
					PRINTF_INFO("Orig: %s.\n", strTest.str);
					PRINTF_INFO("Epct: %s.\n", str2.str);
				}
				if(printLevel > 0){
					PRINTF_INFO("#%d-test-#%d-cicle REPLACE(%d, +%d, with +%d [%s]) on %d bytes%s%s%s.\n", (i + 1), (j + 1), start, size, str3.length, (str3.length < size ? "reduce" : str3.length > size ? "growth" : "same"), orgLen, (start >= orgLen ? " start-out-of-rng" : ""), (start + size) == orgLen ? " remove-at-end" : "", (start + size) > orgLen ? " size-out-of-rng" : "");
				}
				NBArray_replaceBytes(&strTest, start, size, str3.str, str3.length);
				if(printLevel > 1){
					PRINTF_INFO("Rslt: %s.\n", strTest.str);
				}
				//Compare
				NBASSERT(strTest.length == str2.length)
				NBASSERT(NBArray_strIsEqual(strTest.str, str2.str));
			}*/
		}
	}
	NBArray_release(&str3);
	NBArray_release(&str2);
	NBArray_release(&strTest);
}
#endif
