//
//  NBDataPtr.h
//  thinstream
//
//  Created by Marcos Ortega on 9/3/19.
//

#ifndef NBDataPtr_h
#define NBDataPtr_h

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"

#ifdef __cplusplus
extern "C" {
#endif

struct STNBDataPtr_;

//ENNBDataPtrAllocType

typedef enum ENNBDataPtrAllocType_ {
	ENNBDataPtrAllocType_Null = 0, 		//non-allocated yet
	ENNBDataPtrAllocType_PoolReturn,	//must return to a pool
	ENNBDataPtrAllocType_Alloc,			//allocated directly to memory
	ENNBDataPtrAllocType_External,		//owned by external code, should not be released by this
	//Count
	ENNBDataPtrAllocType_Count
} ENNBDataPtrAllocType;

//NBDataPtrAlloc 

typedef struct STNBDataPtrAlloc_ {
	void*			ptr;			//start of allocated area
	UI32			size;			//allocated size
	STNBObjRef		pool;			//NBDataPtrsPoolRef that contains the chunk
} STNBDataPtrAlloc;

//NBDataPtrReleaserItf
//For capturing packages before returning to pool.

typedef struct STNBDataPtrReleaserItf_ {
	void		(*ptrsReleaseGrouped)(struct STNBDataPtr_* objs, const UI32 size, void* usrData);
} STNBDataPtrReleaserItf;

//NBDataPtrReleaser

typedef struct STNBDataPtrReleaser_ {
	STNBDataPtrReleaserItf	itf;
	void*					usrData;
} STNBDataPtrReleaser;

//NBDataPtrDef 

typedef struct STNBDataPtrDef_ {
	BOOL				preResigned;	//ptr can be given to other 'NBDataPtr'
	UI8					allocType;		//ENNBDataPtrAllocType
	STNBDataPtrAlloc	alloc;			//
} STNBDataPtrDef;

//NBDataPtr

typedef struct STNBDataPtr_ {
	void*			ptr;	//ptr inside 'def.alloc.ptr' and 'def.alloc.ptr + 'def.alloc.size' - 1;
	UI32			use;	//populated bytes
	STNBDataPtrDef	def;	//buffer
} STNBDataPtr;

void NBDataPtr_init(STNBDataPtr* obj);
void NBDataPtr_release(STNBDataPtr* obj);
void NBDataPtr_ptrsReleaseGrouped(STNBDataPtr* objs, const UI32 size);

//Optimization (reduces locks and internal data cloning)
void NBDataPtr_preResign(STNBDataPtr* obj);

//data
void NBDataPtr_empty(STNBDataPtr* obj);
void NBDataPtr_allocEmptyPtr(STNBDataPtr* obj, const UI32 size);
BOOL NBDataPtr_swapCompatibleOther(STNBDataPtr* obj, STNBDataPtr* other);
void NBDataPtr_resignToBuffer(STNBDataPtr* obj);
void NBDataPtr_setAsEmptyPoolPtr(STNBDataPtr* obj, STNBObjRef pool, void* ptr, const UI32 ptrSz);
void NBDataPtr_setAsOther(STNBDataPtr* obj, STNBDataPtr* other);
BOOL NBDataPtr_setAsSubRng(STNBDataPtr* obj, STNBDataPtr* other, const UI32 iStart, const UI32 dataSz);
void NBDataPtr_setAsExternalBytes(STNBDataPtr* obj, void* data, const UI32 dataSz);
//
UI32 NBDataPtr_allocatedSize(const STNBDataPtr* obj);

#ifdef __cplusplus
} //extern "C"
#endif
		
#endif /* NBDataPtr_h */
