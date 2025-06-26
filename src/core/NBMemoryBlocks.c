
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
        NBStructMap_addUIntM(map, s, allocableSzPerBlock);  //ammount of bytes allocable
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
    UI32        allocableSzTotal;       //iOffset for next block
} STNBMemoryBlocksState;

// STNBMemoryBlocksBlock

#define STNBMemoryBlocksBlock_Zero  { 0, 0, 0, STNBObjRef_Zero }

typedef struct STNBMemoryBlocksBlock_ {
    UI32                iOffset;    //idx-at-block + iOffset = abstract-idx-at-blocks
    UI32                usableSz;   //usableSz, already idx-aligned
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

BOOL NBMemoryBlocks_prepare(STNBMemoryBlocksRef ref, const STNBMemoryBlocksCfg* cfg){
    BOOL r = FALSE;
    STNBMemoryBlocksOpq* opq = (STNBMemoryBlocksOpq*)ref.opaque;
    //
    NBObject_lock(opq);
    if(cfg != NULL && opq->blocks.use == 0){
        STNBMemoryBlockCfg bCfg = STNBMemoryBlockCfg_Zero;
        bCfg.sizeAlign   = (cfg->sizeAlign > 0 ? cfg->sizeAlign : 4);   //whole memory block size alignment
        bCfg.idxsAlign   = (cfg->idxsAlign > 0 ? cfg->idxsAlign : 4);   //individual pointers alignment
        bCfg.allocableSz = (cfg->allocableSzPerBlock + bCfg.idxsAlign - 1) / bCfg.idxsAlign * bCfg.idxsAlign;
        bCfg.idxZeroIsValid = (opq->blocks.use == 0 ? cfg->idxZeroIsValid : TRUE);
        //copy cfg
        {
            NBStruct_stRelease(NBMemoryBlocksCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
            NBStruct_stClone(NBMemoryBlocksCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
            opq->cfg.sizeAlign = bCfg.sizeAlign;
            opq->cfg.idxsAlign = bCfg.idxsAlign;
            opq->cfg.allocableSzPerBlock = bCfg.allocableSz;
        }
        //intial state
        {
            r = TRUE;
            opq->state.allocableSzTotal = 0;
            //allocate initial blocks
            if(bCfg.allocableSz > 0){
                UI32 usableSzInit = (((cfg->allocableSzInitial + bCfg.idxsAlign - 1) / bCfg.idxsAlign * bCfg.idxsAlign) + bCfg.allocableSz - 1) / bCfg.allocableSz * bCfg.allocableSz;
                while(usableSzInit > 0){
                    STNBMemoryBlocksBlock b = STNBMemoryBlocksBlock_Zero;
                    b.usableSz  = bCfg.allocableSz;
                    b.iOffset   = opq->state.allocableSzTotal;
                    b.block     = NBMemoryBlock_alloc(NULL);
                    if(!NBMemoryBlock_prepare(b.block, &bCfg)){
                        NBMemoryBlock_release(&b.block);
                        r = FALSE;
                        break;
                    } else {
                        NBArray_addValue(&opq->blocks, b);
                        opq->state.allocableSzTotal += b.usableSz;
                    }
                    usableSzInit -= bCfg.allocableSz;
                }
                opq->cfg.allocableSzInitial = usableSzInit;
            }
        }
    }
    NBObject_unlock(opq);
    return r;
}

//allocations

STNBAbsPtr NBMemoryBlocks_malloc(STNBMemoryBlocksRef ref, const UI32 usableSz){
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
        if(b == bAfterEnd){
            STNBMemoryBlocksBlock b = STNBMemoryBlocksBlock_Zero;
            b.usableSz  = ((usableSz > opq->cfg.allocableSzPerBlock ? usableSz : opq->cfg.allocableSzPerBlock) + opq->cfg.idxsAlign - 1) / opq->cfg.idxsAlign * opq->cfg.idxsAlign;
            b.iOffset   = opq->state.allocableSzTotal;
            b.block     = NBMemoryBlock_alloc(NULL);
            {
                STNBMemoryBlockCfg bCfg = STNBMemoryBlockCfg_Zero;
                bCfg.sizeAlign   = opq->cfg.sizeAlign;   //whole memory block size alignment
                bCfg.idxsAlign   = opq->cfg.idxsAlign;   //individual pointers alignment
                bCfg.allocableSz = b.usableSz;
                bCfg.idxZeroIsValid = (opq->blocks.use == 0 ? opq->cfg.idxZeroIsValid : TRUE);
                if(!NBMemoryBlock_prepare(b.block, &bCfg)){
                    NBMemoryBlock_release(&b.block);
                } else {
                    r = NBMemoryBlock_malloc(b.block, usableSz);
                    if(r.ptr == NULL){
                        NBASSERT(FALSE) //program-logic error
                        NBMemoryBlock_release(&b.block);
                    } else {
                        //convert block-index to blocks-index
                        r.idx += b.iOffset;
                        NBArray_addValue(&opq->blocks, b);
                        opq->state.allocableSzTotal += b.usableSz;
                    }
                }
            }
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
            if(ptr2.idx < b->usableSz){
                r = NBMemoryBlock_mfree(b->block, ptr2);
                if(r){
                    b->szSmallestMallocFailed = 0;
                }
                break;
            }
            NBASSERT(ptr2.idx >= b->usableSz)
            ptr2.idx -= b->usableSz;
            ++b;
        }
    }
    NBObject_unlock(opq);
    return r;
}

