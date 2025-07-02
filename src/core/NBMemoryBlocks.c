
#include "nb/NBFrameworkPch.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBMemoryBlocks.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBStructMaps.h"
#include "nb/core/NBMngrStructMaps.h"

// NBMemoryBlocksCfg

STNBStructMapsRec NBMemoryBlocksCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBMemoryBlocksCfg_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBMemoryBlocksCfg_sharedStructMap);
    if(NBMemoryBlocksCfg_sharedStructMap.map == NULL){
        STNBMemoryBlocksCfg s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBMemoryBlocksCfg);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, sizePerBlock); //ammount of bytes allocable per block
        NBStructMap_addUIntM(map, s, sizeInitial);  //memory to allocate initially
        NBStructMap_addUIntM(map, s, sizeMax);      //max allowed size in bytes (0 is infinite)
        NBStructMap_addUIntM(map, s, sizeAlign);    //whole memory block size alignment
        NBStructMap_addUIntM(map, s, idxsAlign);    //individual pointers alignment
        NBStructMap_addBoolM(map, s, idxZeroIsValid); //idx=0 is an assignable address
        NBMemoryBlocksCfg_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBMemoryBlocksCfg_sharedStructMap);
    return NBMemoryBlocksCfg_sharedStructMap.map;
}

//STNBMemoryBlocksState

#define STNBMemoryBlocksState_Zero   { 0 }

typedef struct STNBMemoryBlocksState_ {
    UI32        idxsTotalSz;       //iOffset for next block
} STNBMemoryBlocksState;

// STNBMemoryBlocksBlock

#define STNBMemoryBlocksBlock_Zero  { 0, 0, 0, STNBObjRef_Zero }

typedef struct STNBMemoryBlocksBlock_ {
    UI32                iOffset;    //idx-at-block + iOffset = abstract-idx-at-blocks
    UI32                idxsSz;     //ammount of addreses from idx-0
    UI32                szSmallestMallocFailed; //for 'NBMemoryBlock_malloc' quick-ignores
    STNBMemoryBlockRef  block;      //memory data
} STNBMemoryBlocksBlock;

//STNBMemoryBlocksOpq

typedef struct STNBMemoryBlocksOpq_ {
    STNBObject              prnt;
    STNBMemoryBlocksCfg     cfg;
    STNBMemoryBlocksState   state;
    STNBArray               blocks; //STNBMemoryBlocksBlock
} STNBMemoryBlocksOpq;

NB_OBJREF_BODY(NBMemoryBlocks, STNBMemoryBlocksOpq, NBObject)

void NBMemoryBlocks_initZeroed(STNBObject* obj) {
    STNBMemoryBlocksOpq* opq = (STNBMemoryBlocksOpq*)obj;
    //
    NBArray_initWithSz(&opq->blocks, sizeof(STNBMemoryBlocksBlock), NULL, 0, 4, 0.1f);
}

void NBMemoryBlocks_uninitLocked(STNBObject* obj){
    STNBMemoryBlocksOpq* opq = (STNBMemoryBlocksOpq*)obj;
    //blocks
    {
        STNBMemoryBlocksBlock* b = NBArray_dataPtr(&opq->blocks, STNBMemoryBlocksBlock);
        const STNBMemoryBlocksBlock* bAfterEnd = b + opq->blocks.use;
        while(b < bAfterEnd){
            NBMemoryBlock_release(&b->block);
            ++b;
        }
        NBArray_empty(&opq->blocks);
        NBArray_release(&opq->blocks);
    }
    //state
    {
        //nothing
    }
    //config
    {
        NBStruct_stRelease(NBMemoryBlocksCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
    }
}

//

BOOL NBMemoryBlocks_prepare(STNBMemoryBlocksRef ref, const STNBMemoryBlocksCfg* cfg, UI32* optDstBlocksTotalSz){
    BOOL r = FALSE;
    STNBMemoryBlocksOpq* opq = (STNBMemoryBlocksOpq*)ref.opaque;
    //
    NBObject_lock(opq);
    if(cfg != NULL && opq->blocks.use == 0){
        STNBMemoryBlockCfg bCfg = STNBMemoryBlockCfg_Zero;
        bCfg.sizeAlign   = (cfg->sizeAlign > 0 ? cfg->sizeAlign : 4);   //whole memory block size alignment
        bCfg.idxsAlign   = (cfg->idxsAlign > 0 ? cfg->idxsAlign : 4);   //individual pointers alignment
        bCfg.size = (cfg->sizePerBlock + bCfg.idxsAlign - 1) / bCfg.idxsAlign * bCfg.idxsAlign;
        bCfg.idxZeroIsValid = (opq->blocks.use == 0 ? cfg->idxZeroIsValid : TRUE);
        //copy cfg
        {
            NBStruct_stRelease(NBMemoryBlocksCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
            NBStruct_stClone(NBMemoryBlocksCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
            opq->cfg.sizeAlign = bCfg.sizeAlign;
            opq->cfg.idxsAlign = bCfg.idxsAlign;
            opq->cfg.sizePerBlock = bCfg.size;
        }
        //intial state
        {
            r = TRUE;
            opq->state.idxsTotalSz = 0;
            //allocate initial blocks
            if(bCfg.size > 0){
                UI32 usableSzInit = (((cfg->sizeInitial + bCfg.idxsAlign - 1) / bCfg.idxsAlign * bCfg.idxsAlign) + bCfg.size - 1) / bCfg.size * bCfg.size;
                while(usableSzInit > 0){
                    STNBAbsPtr ptrAfterEnd = STNBAbsPtr_Zero;
                    STNBMemoryBlocksBlock b = STNBMemoryBlocksBlock_Zero;
                    b.iOffset   = opq->state.idxsTotalSz;
                    b.idxsSz    = bCfg.size;
                    b.block     = NBMemoryBlock_alloc(NULL);
                    if(!NBMemoryBlock_prepare(b.block, &bCfg, &ptrAfterEnd)){
                        NBMemoryBlock_release(&b.block);
                        r = FALSE;
                        break;
                    } else {
                        b.idxsSz = ptrAfterEnd.idx;
                        opq->state.idxsTotalSz += ptrAfterEnd.idx;
                        NBArray_addValue(&opq->blocks, b);
                    }
                    usableSzInit -= bCfg.size;
                }
                opq->cfg.sizeInitial = usableSzInit;
            }
        }
        if(optDstBlocksTotalSz != NULL){
            *optDstBlocksTotalSz = opq->state.idxsTotalSz;
        }
    }
    NBObject_unlock(opq);
    return r;
}

//allocations

STNBAbsPtr NBMemoryBlocks_malloc(STNBMemoryBlocksRef ref, const UI32 usableSz, UI32* optDstBlocksTotalSz){
    STNBAbsPtr r = STNBAbsPtr_Zero;
    STNBMemoryBlocksOpq* opq = (STNBMemoryBlocksOpq*)ref.opaque;
    NBObject_lock(opq);
    if(opq->cfg.idxsAlign > 0){
        //try on current blocks
        STNBMemoryBlocksBlock* b = NBArray_dataPtr(&opq->blocks, STNBMemoryBlocksBlock);
        const STNBMemoryBlocksBlock* bAfterEnd = b + opq->blocks.use;
        while(b < bAfterEnd){
            if(b->szSmallestMallocFailed <= usableSz){
                //skip, already failed to allocate
            } else {
                //try to allocate
                r = NBMemoryBlock_malloc(b->block, usableSz);
                if(r.ptr == NULL){
                    b->szSmallestMallocFailed = usableSz;
                } else {
                    //convert block-index to blocks-index
                    r.idx += b->iOffset;
                    break;
                }
            }
            ++b;
        }
        //create new block
        if(b >= bAfterEnd){
            UI32 idxSz = ((usableSz > opq->cfg.sizePerBlock ? usableSz : opq->cfg.sizePerBlock) + opq->cfg.idxsAlign - 1) / opq->cfg.idxsAlign * opq->cfg.idxsAlign;
            //apply size limitation
            if(opq->cfg.sizeMax > 0 && opq->state.idxsTotalSz >= opq->cfg.sizeMax){
                //already reached limit
            } else {
                //allocate new block
                STNBMemoryBlocksBlock b = STNBMemoryBlocksBlock_Zero;
                b.idxsSz    = (opq->cfg.sizeMax == 0 || (opq->state.idxsTotalSz + idxSz) <= opq->cfg.sizeMax ? idxSz : opq->cfg.sizeMax - opq->state.idxsTotalSz );
                b.iOffset   = opq->state.idxsTotalSz;
                b.block     = NBMemoryBlock_alloc(NULL);
                {
                    STNBMemoryBlockCfg bCfg = STNBMemoryBlockCfg_Zero;
                    bCfg.sizeAlign   = opq->cfg.sizeAlign;   //whole memory block size alignment
                    bCfg.idxsAlign   = opq->cfg.idxsAlign;   //individual pointers alignment
                    bCfg.size = b.idxsSz;
                    bCfg.idxZeroIsValid = (opq->blocks.use == 0 ? opq->cfg.idxZeroIsValid : TRUE);
                    STNBAbsPtr ptrAfterEnd = STNBAbsPtr_Zero;
                    if(!NBMemoryBlock_prepare(b.block, &bCfg, &ptrAfterEnd)){
                        NBMemoryBlock_release(&b.block);
                    } else {
                        r = NBMemoryBlock_malloc(b.block, usableSz);
                        if(r.ptr == NULL){
                            NBASSERT(FALSE) //program-logic error
                            NBMemoryBlock_release(&b.block);
                        } else {
                            //convert block-index to blocks-index
                            r.idx += b.iOffset;
                            b.idxsSz = ptrAfterEnd.idx;
                            opq->state.idxsTotalSz += ptrAfterEnd.idx;
                            NBArray_addValue(&opq->blocks, b);
                        }
                    }
                }
            }
        }
        //
        if(optDstBlocksTotalSz != NULL){
            *optDstBlocksTotalSz = opq->state.idxsTotalSz;
        }
    }
    NBObject_unlock(opq);
    return r;
}

BOOL NBMemoryBlocks_mfree(STNBMemoryBlocksRef ref, const STNBAbsPtr ptr){
    BOOL r = FALSE;
    STNBMemoryBlocksOpq* opq = (STNBMemoryBlocksOpq*)ref.opaque;
    NBObject_lock(opq);
    if(opq->cfg.idxsAlign > 0){
        //find the idx-at-block of this abstract-idx-at-blocks
        STNBAbsPtr ptr2 = ptr;
        STNBMemoryBlocksBlock* b = NBArray_dataPtr(&opq->blocks, STNBMemoryBlocksBlock);
        const STNBMemoryBlocksBlock* bAfterEnd = b + opq->blocks.use;
        while(b < bAfterEnd){
            if(ptr2.idx < b->idxsSz){
                r = NBMemoryBlock_mfree(b->block, ptr2);
                if(r){
                    b->szSmallestMallocFailed = 0;
                }
                break;
            }
            NBASSERT(ptr2.idx >= b->idxsSz)
            ptr2.idx -= b->idxsSz;
            ++b;
        }
    }
    NBObject_unlock(opq);
    return r;
}

//

void NBMemoryBlocks_clear(STNBMemoryBlocksRef ref){ //clears the index, all pointers are invalid after this call
    STNBMemoryBlocksOpq* opq = (STNBMemoryBlocksOpq*)ref.opaque;
    NBObject_lock(opq);
    {
        STNBMemoryBlocksBlock* b = NBArray_dataPtr(&opq->blocks, STNBMemoryBlocksBlock);
        const STNBMemoryBlocksBlock* bAfterEnd = b + opq->blocks.use;
        while(b < bAfterEnd){
            NBMemoryBlock_clear(b->block);
            b->szSmallestMallocFailed = 0;
            ++b;
        }
    }
    NBObject_unlock(opq);
}

//dbg

BOOL NBMemoryBlocks_validateIndexLockedOpq_(STNBMemoryBlocksOpq* opq){
    BOOL r = TRUE;
    {
        STNBMemoryBlocksBlock* b = NBArray_dataPtr(&opq->blocks, STNBMemoryBlocksBlock);
        const STNBMemoryBlocksBlock* bAfterEnd = b + opq->blocks.use;
        UI32 idxsTotalSz = 0;
        while(b < bAfterEnd){
            if(b->iOffset != idxsTotalSz){
                r = FALSE;
            } else if(!NBMemoryBlock_validateIndex(b->block)){
                r = FALSE;
            }
            idxsTotalSz += b->idxsSz;
            ++b;
        }
        //
        if(idxsTotalSz != opq->state.idxsTotalSz){
            r = FALSE;
        }
    }
    return r;
}
    
BOOL NBMemoryBlocks_validateIndex(STNBMemoryBlockRef ref){
    BOOL r = TRUE;
    STNBMemoryBlocksOpq* opq = (STNBMemoryBlocksOpq*)ref.opaque;
    NBObject_lock(opq);
    {
        r = NBMemoryBlocks_validateIndexLockedOpq_(opq);
    }
    NBObject_unlock(opq);
    return r;
}
