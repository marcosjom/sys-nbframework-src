#ifndef NB_MEMORY_BLOCKS_H
#define NB_MEMORY_BLOCKS_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBMemoryBlock.h"

#ifdef __cplusplus
extern "C" {
#endif

//STNBMemoryBlockCfg

#define STNBMemoryBlocksCfg_Zero { 0, 0, 0, 0, 0, FALSE }

typedef struct STNBMemoryBlocksCfg_ {
    UI32        sizePerBlock;   //ammount of bytes allocable per block (including the idx-0)
    UI32        sizeInitial;    //memory to allocate initially
    UI32        sizeMax;        //max allowed size in bytes (0 is infinite)
    UI32        sizeAlign;      //whole memory block size alignment
    UI32        idxsAlign;      //individual pointers alignment
    BOOL        idxZeroIsValid; //idx=0 is an assignable address
} STNBMemoryBlocksCfg;

const STNBStructMap* NBMemoryBlocksCfg_getSharedStructMap(void);

//NBMemoryBlocks

NB_OBJREF_HEADER(NBMemoryBlocks)

//

BOOL NBMemoryBlocks_prepare(STNBMemoryBlocksRef ref, const STNBMemoryBlocksCfg* cfg, UI32* optDstBlocksTotalSz);

//allocations
STNBAbsPtr NBMemoryBlocks_malloc(STNBMemoryBlocksRef ref, const UI32 usableSz, UI32* optDstBlocksTotalSz);
BOOL NBMemoryBlocks_mfree(STNBMemoryBlocksRef ref, const STNBAbsPtr ptr);

//
void NBMemoryBlocks_clear(STNBMemoryBlocksRef ref); //clears the index, all pointers are invalid after this call

//dbg
BOOL NBMemoryBlocks_validateIndex(STNBMemoryBlockRef ref);

#ifdef __cplusplus
} //extern "C"
#endif


#endif
