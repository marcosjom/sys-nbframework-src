#ifndef NB_MEMORY_BLOCK_H
#define NB_MEMORY_BLOCK_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBStructMap.h"

#ifdef __cplusplus
extern "C" {
#endif

//STNBAbsPtr, abstract pointer

#define STNBAbsPtr_Zero { NULL, 0 }

typedef struct STNBAbsPtr_ {
    void*   ptr;        //memory address, must be first element of struct to allow casting struct to a bare-pointer.
    UI32    idx;        //abstract address
    //Note: possible 4-bytes padding here.
} STNBAbsPtr;

//STNBMemoryBlockCfg

#define STNBMemoryBlockCfg_Zero { 0, 0, 0, FALSE }

typedef struct STNBMemoryBlockCfg_ {
    UI32        size;           //ammount of bytes allocable (including the idx-0)
    UI32        sizeAlign;      //whole memory block size alignment
    UI32        idxsAlign;      //individual pointers alignment
    BOOL        idxZeroIsValid; //idx=0 is an assignable address
} STNBMemoryBlockCfg;

const STNBStructMap* NBMemoryBlockCfg_getSharedStructMap(void);

//NBMemoryBlock

NB_OBJREF_HEADER(NBMemoryBlock)

BOOL    NBMemoryBlock_prepare(STNBMemoryBlockRef ref, const STNBMemoryBlockCfg* cfg, STNBAbsPtr* dstPtrAfterEnd);

//allocations
STNBAbsPtr NBMemoryBlock_malloc(STNBMemoryBlockRef ref, const UI32 usableSz);
BOOL    NBMemoryBlock_mfree(STNBMemoryBlockRef ref, const STNBAbsPtr ptr);
UI32    NBMemoryBlock_mAvailSz(STNBMemoryBlockRef ref);
//
void    NBMemoryBlock_prepareForNewMallocsActions(STNBMemoryBlockRef ref, const UI32 ammActions);   //increases the index's sz
void    NBMemoryBlock_clear(STNBMemoryBlockRef ref); //clears the index, all pointers are invalid after this call
//dbg
BOOL    NBMemoryBlock_validateIndex(STNBMemoryBlockRef ref);

#ifdef __cplusplus
} //extern "C"
#endif


#endif
