//
//  NBDataPtr.h
//  thinstream
//
//  Created by Marcos Ortega on 9/3/19.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBDataPtr.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBThreadCond.h"
#include "nb/core/NBDataPtrsPool.h"
//

void NBDataPtr_releaseBuffDontSet_(STNBDataPtr* obj); //releases its buff.ptr without unseting the values to reduce uncecesary var-updates; caller must set values after.

void NBDataPtr_init(STNBDataPtr* obj){
	NBMemory_setZeroSt(*obj, STNBDataPtr);
}

void NBDataPtr_release(STNBDataPtr* obj){
	//release cur-buff
	if(obj->def.allocType != ENNBDataPtrAllocType_Null){
		NBDataPtr_releaseBuffDontSet_(obj);
	}
	//set
	NBMemory_setZeroSt(*obj, STNBDataPtr);
}

//

void NBDataPtr_releaseBuffDontSet_(STNBDataPtr* obj){
	switch (obj->def.allocType) {
		case ENNBDataPtrAllocType_Null:
			NBASSERT(obj->ptr == NULL && obj->use == 0 && obj->def.alloc.ptr == NULL && obj->def.alloc.size == 0 && !NBDataPtrsPool_isSet(obj->def.alloc.pool))
			break;
		case ENNBDataPtrAllocType_PoolReturn:
		NBASSERT((obj->ptr == NULL || (obj->ptr >= obj->def.alloc.ptr && obj->ptr < (void*)((BYTE*)obj->def.alloc.ptr + obj->def.alloc.size))) && obj->def.alloc.ptr != NULL && obj->def.alloc.size > 0 && NBDataPtrsPool_isSet(obj->def.alloc.pool))
			if(!NBDataPtrsPool_isSet(obj->def.alloc.pool) || !NBDataPtrsPool_addPtr(obj->def.alloc.pool, obj->def.alloc.ptr, obj->def.alloc.size)){
				NBMemory_free(obj->def.alloc.ptr);
			}
			break;
		case ENNBDataPtrAllocType_Alloc:
			NBASSERT((obj->ptr == NULL || (obj->ptr >= obj->def.alloc.ptr && obj->ptr < (void*)((BYTE*)obj->def.alloc.ptr + obj->def.alloc.size))) && obj->def.alloc.ptr != NULL && obj->def.alloc.size > 0 && !NBDataPtrsPool_isSet(obj->def.alloc.pool))
			if(obj->def.alloc.ptr != NULL){
				NBMemory_free(obj->def.alloc.ptr);
				obj->def.alloc.ptr = NULL;
			}
			break;
		case ENNBDataPtrAllocType_External:
			NBASSERT((obj->ptr == NULL || (obj->ptr >= obj->def.alloc.ptr && obj->ptr < (void*)((BYTE*)obj->def.alloc.ptr + obj->def.alloc.size))) && obj->def.alloc.ptr != NULL && obj->def.alloc.size > 0 && !NBDataPtrsPool_isSet(obj->def.alloc.pool))
			break;
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		default:
			NBASSERT(FALSE) //unexpected 'allocType' value
			break;
#		endif
	}
}

//

void NBDataPtr_ptrsReleaseGrouped(STNBDataPtr* objs, const UI32 size){
	if(objs != NULL && size > 0){
		STNBDataPtr* obj = objs;
		const STNBDataPtr* objAfterEnd = objs + size;
		UI8 grpAllocType			= obj->def.allocType;
		STNBDataPtrsPoolRef grpPool	= obj->def.alloc.pool;
		STNBDataPtr* grpFirstObj	= obj++;
		while(obj <= objAfterEnd){
			if(obj == objAfterEnd || !NBDataPtrsPool_isSame(grpPool, obj->def.alloc.pool) || grpAllocType != obj->def.allocType){
				//send to pool
				if(grpAllocType == ENNBDataPtrAllocType_PoolReturn && NBDataPtrsPool_isSet(grpPool)){
					NBASSERT(grpFirstObj < obj) //at least one
					NBDataPtrsPool_addDataPtrs(grpPool, grpFirstObj, (UI32)(obj - grpFirstObj));
				}
				//release pointers
				while(grpFirstObj < obj){
					NBDataPtr_release(grpFirstObj++);
				} NBASSERT(grpFirstObj == obj)
				//start next pool
				grpFirstObj = obj;
				if(obj < objAfterEnd){
					grpAllocType	= obj->def.allocType;
					grpPool			= obj->def.alloc.pool;
				}
			}
			obj++;
		} NBASSERT(obj == (objAfterEnd + 1))
	}
}

//

void NBDataPtr_empty(STNBDataPtr* obj){
	obj->ptr = obj->def.alloc.ptr;
	obj->use = 0;
}

//Optimization (reduces locks and internal data cloning)
void NBDataPtr_preResign(STNBDataPtr* obj){
	obj->def.preResigned = TRUE;
}

//

void NBDataPtr_allocEmptyPtr(STNBDataPtr* obj, const UI32 size){
	//release cur-buff
	if(obj->def.allocType != ENNBDataPtrAllocType_Null){
		NBDataPtr_releaseBuffDontSet_(obj);
	}
	//set 
	if(size <= 0){
		obj->ptr		= NULL;
		obj->use		= 0;
		NBMemory_setZeroSt(obj->def, STNBDataPtrDef);
	} else {
		obj->ptr			= obj->def.alloc.ptr = NBMemory_alloc(size);
		obj->use			= 0;
		obj->def.allocType	= ENNBDataPtrAllocType_Alloc;
		obj->def.preResigned = FALSE;
		obj->def.alloc.size	= size;
		NBDataPtrsPool_null(&obj->def.alloc.pool);
		NBASSERT(obj->ptr != NULL && obj->use <= obj->def.alloc.size && obj->ptr == obj->def.alloc.ptr && obj->def.alloc.size > 0 && !NBDataPtrsPool_isSet(obj->def.alloc.pool))
	}
	NBASSERT(obj->ptr >= obj->def.alloc.ptr && obj->use <= obj->def.alloc.size && (((BYTE*)obj->ptr) + obj->use) <= (((BYTE*)obj->def.alloc.ptr) + obj->def.alloc.size))
}

BOOL NBDataPtr_swapCompatibleOther(STNBDataPtr* obj, STNBDataPtr* other){
	BOOL r = FALSE;
	NBASSERT(obj->def.alloc.size == other->def.alloc.size)
	if(obj->def.alloc.size == other->def.alloc.size){
		STNBDataPtr tmp = *other;
		NBASSERT(obj->ptr >= obj->def.alloc.ptr && obj->use <= obj->def.alloc.size && (((BYTE*)obj->ptr) + obj->use) <= (((BYTE*)obj->def.alloc.ptr) + obj->def.alloc.size))
		NBASSERT(other->ptr >= other->def.alloc.ptr && other->use <= other->def.alloc.size && (((BYTE*)other->ptr) + other->use) <= (((BYTE*)other->def.alloc.ptr) + other->def.alloc.size))
		//
		*other	= *obj;
		*obj	= tmp;
		//
		r = TRUE;
	}
	return r;
}

void NBDataPtr_resignToBuffer(STNBDataPtr* obj){
	NBASSERT(obj->ptr >= obj->def.alloc.ptr && obj->use <= obj->def.alloc.size && (((BYTE*)obj->ptr) + obj->use) <= (((BYTE*)obj->def.alloc.ptr) + obj->def.alloc.size))
	NBMemory_setZeroSt(*obj, STNBDataPtr);
}

void NBDataPtr_setAsEmptyPoolPtr(STNBDataPtr* obj, STNBObjRef pool, void* ptr, const UI32 ptrSz){
	//release cur-buff
	if(obj->def.allocType != ENNBDataPtrAllocType_Null){
		NBDataPtr_releaseBuffDontSet_(obj);
	}
	//set
	obj->ptr			= ptr;
	obj->use			= 0; //empty
	//
	obj->def.preResigned = FALSE;
	obj->def.allocType	= ENNBDataPtrAllocType_PoolReturn;
	//
	obj->def.alloc.ptr	= ptr;
	obj->def.alloc.size	= ptrSz;
	obj->def.alloc.pool	= pool;
	//
	NBASSERT(obj->ptr >= obj->def.alloc.ptr && obj->use <= obj->def.alloc.size && (((BYTE*)obj->ptr) + obj->use) <= (((BYTE*)obj->def.alloc.ptr) + obj->def.alloc.size))
}

void NBDataPtr_setAsOther(STNBDataPtr* obj, STNBDataPtr* other){
	//release cur-buff
	if(obj->def.allocType != ENNBDataPtrAllocType_Null){
		NBDataPtr_releaseBuffDontSet_(obj);
	}
	//set
	if(other == NULL){
		NBMemory_setZeroSt(*obj, STNBDataPtr);
	} else {
		NBASSERT(other->ptr >= other->def.alloc.ptr && other->use <= other->def.alloc.size && (((BYTE*)other->ptr) + other->use) <= (((BYTE*)other->def.alloc.ptr) + other->def.alloc.size))
		if(other->def.preResigned && other->def.allocType != ENNBDataPtrAllocType_External){ //extenal_ptrs should not be passed down, data must be cloned.
			//pass data without cloning
			*obj = *other;
			obj->def.preResigned = FALSE;
			//make other's resignation effective
			NBMemory_setZeroSt(*other, STNBDataPtr);
		} else if(other->ptr != NULL && other->use > 0){
			//clone other's data
			obj->use		= obj->def.alloc.size = other->use;
			obj->ptr		= obj->def.alloc.ptr = NBMemory_alloc(other->use);
			if(other->use > 0){
				NBMemory_copy(obj->ptr, other->ptr, other->use);
			}
			obj->def.preResigned	= FALSE;
			obj->def.allocType		= ENNBDataPtrAllocType_Alloc;
			NBDataPtrsPool_null(&obj->def.alloc.pool);
			NBASSERT(obj->ptr != NULL && obj->use <= obj->def.alloc.size && obj->ptr == obj->def.alloc.ptr && obj->def.alloc.size > 0 && !NBDataPtrsPool_isSet(obj->def.alloc.pool))
		} else {
			//no data to clone
			NBMemory_setZeroSt(*obj, STNBDataPtr);
		}
		NBASSERT(obj->ptr >= obj->def.alloc.ptr && obj->use <= obj->def.alloc.size && (((BYTE*)obj->ptr) + obj->use) <= (((BYTE*)obj->def.alloc.ptr) + obj->def.alloc.size))
	}
}

BOOL NBDataPtr_setAsSubRng(STNBDataPtr* obj, STNBDataPtr* other, const UI32 iStart, const UI32 dataSz){
	BOOL r = FALSE;
	if(other != NULL && iStart < other->use && (iStart + dataSz) <= other->use){
		//release cur-buff
		if(obj->def.allocType != ENNBDataPtrAllocType_Null){
			NBDataPtr_releaseBuffDontSet_(obj);
		}
		NBASSERT(other->ptr >= other->def.alloc.ptr && other->use <= other->def.alloc.size && (((BYTE*)other->ptr) + other->use) <= (((BYTE*)other->def.alloc.ptr) + other->def.alloc.size))
		if(other->def.preResigned && other->def.allocType != ENNBDataPtrAllocType_External){ //extenal_ptrs should not be passed down, data must be cloned.
			//pass data without cloning
			obj->ptr				= (BYTE*)other->ptr + iStart;
			obj->use				= dataSz;
			obj->def.preResigned	= FALSE;
			obj->def.allocType		= other->def.allocType;
			obj->def.alloc.ptr		= other->def.alloc.ptr;
			obj->def.alloc.size		= other->def.alloc.size;
			obj->def.alloc.pool		= other->def.alloc.pool;
			//make other's resignation effective
			NBMemory_setZeroSt(*other, STNBDataPtr);
		} else if(other->ptr != NULL && dataSz > 0){
			//clone other's data
			obj->use		= obj->def.alloc.size = dataSz;
			obj->ptr		= obj->def.alloc.ptr = NBMemory_alloc(dataSz);
			if(dataSz > 0){
				NBMemory_copy(obj->ptr, (BYTE*)other->ptr + iStart, dataSz);
			}
			obj->def.preResigned	= FALSE;
			obj->def.allocType		= ENNBDataPtrAllocType_Alloc;
			NBDataPtrsPool_null(&obj->def.alloc.pool);
			NBASSERT(obj->ptr != NULL && obj->use <= obj->def.alloc.size && obj->ptr == obj->def.alloc.ptr && obj->def.alloc.size > 0 && !NBDataPtrsPool_isSet(obj->def.alloc.pool))
		} else {
			//no data to clone
			NBMemory_setZeroSt(*obj, STNBDataPtr);
		}
		NBASSERT(obj->ptr >= obj->def.alloc.ptr && obj->use <= obj->def.alloc.size && (((BYTE*)obj->ptr) + obj->use) <= (((BYTE*)obj->def.alloc.ptr) + obj->def.alloc.size))
		//
		r = TRUE;
	}
	NBASSERT(r)
	return r;
}

void NBDataPtr_setAsExternalBytes(STNBDataPtr* obj, void* data, const UI32 dataSz){
	//release cur-buff
	if(obj->def.allocType != ENNBDataPtrAllocType_Null){
		NBDataPtr_releaseBuffDontSet_(obj);
	}
	if(data != NULL && dataSz > 0){
		//set
		obj->use				= obj->def.alloc.size = dataSz;
		obj->ptr				= obj->def.alloc.ptr = data;
		obj->def.preResigned	= FALSE;
		obj->def.allocType		= ENNBDataPtrAllocType_External;
		NBDataPtrsPool_null(&obj->def.alloc.pool);
	} else {
		//no data to clone
		NBMemory_setZeroSt(*obj, STNBDataPtr);
	}
}

UI32 NBDataPtr_allocatedSize(const STNBDataPtr* obj){
	return obj->def.alloc.size;
}
