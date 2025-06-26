#ifndef NB_MEMORY_BLOCK_H
#define NB_MEMORY_BLOCK_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBStructMap.h"

#ifdef __cplusplus
extern "C" {
#endif

//STNBAbsPtr, abstract pointer

#define STNBAbsPtr_Zero { 0, 0, NULL }

typedef struct STNBAbsPtr_ {
    UI32    idx;        //abstract address
    UI32    reserved;   //reserved
    void*   ptr;        //memory address
} STNBAbsPtr;

//STNBMemoryBlockCfg

#define STNBMemoryBlockCfg_Zero { 0, 0, 0, FALSE }

typedef struct STNBMemoryBlockCfg_ {
    UI32        allocableSz;    //ammount of bytes allocable
    UI32        sizeAlign;      //whole memory block size alignment
    UI32        idxsAlign;      //individual pointers alignment
    BOOL        idxZeroIsValid; //idx=0 is an assignable address
} STNBMemoryBlockCfg;

const STNBStructMap* NBMemoryBlockCfg_getSharedStructMap(void);

//NBMemoryBlock

NB_OBJREF_HEADER(NBMemoryBlock)

BOOL    NBMemoryBlock_prepare(STNBMemoryBlockRef ref, const STNBMemoryBlockCfg* cfg);

//allocations
STNBAbsPtr NBMemoryBlock_malloc(STNBMemoryBlockRef ref, const UI32 usableSz);
BOOL    NBMemoryBlock_mfree(STNBMemoryBlockRef ref, const STNBAbsPtr ptr);
UI32    NBMemoryBlock_mAvailSz(STNBMemoryBlockRef ref);
void    NBMemoryBlock_prepareForNewMallocsActions(STNBMemoryBlockRef ref, const UI32 ammActions);   //increases the index sz
//dbg
BOOL    NBMemoryBlock_validateIndex(STNBMemoryBlockRef ref);

#ifdef __cplusplus
} //extern "C"
#endif


#endif
