
#include "nb/NBFrameworkPch.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBMemoryBlock.h"
#include "nb/core/NBArraySorted.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBStructMaps.h"
#include "nb/core/NBMngrStructMaps.h"

// NBMemoryBlockCfg

STNBStructMapsRec NBMemoryBlockCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBMemoryBlockCfg_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBMemoryBlockCfg_sharedStructMap);
    if(NBMemoryBlockCfg_sharedStructMap.map == NULL){
        STNBMemoryBlockCfg s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBMemoryBlockCfg);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, size);         //ammount of bytes allocable (including the idx-0)
        NBStructMap_addUIntM(map, s, sizeAlign);    //whole memory block size alignment
        NBStructMap_addUIntM(map, s, idxsAlign);    //individual pointers alignment
        NBStructMap_addBoolM(map, s, idxZeroIsValid); //idx=0 is an assignable address
        NBMemoryBlockCfg_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBMemoryBlockCfg_sharedStructMap);
    return NBMemoryBlockCfg_sharedStructMap.map;
}

//STNBMemoryBlockPtr

#define STNBMemoryBlockPtr_Zero { NULL, 0 }

typedef struct STNBMemoryBlockPtr_ {
    void*       ptr;  //pointer returned by 'NBMemoryBlock_malloc'
    UI32        sz;   //size at 'NBMemoryBlock_malloc' call
} STNBMemoryBlockPtr;

BOOL NBCompare_NBMemoryBlockPtr(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
    NBASSERT(dataSz == sizeof(STNBMemoryBlockPtr))
    if(dataSz == sizeof(STNBMemoryBlockPtr)){
        const STNBMemoryBlockPtr* d1 = (STNBMemoryBlockPtr*)data1;
        const STNBMemoryBlockPtr* d2 = (STNBMemoryBlockPtr*)data2;
        switch (mode) {
            case ENCompareMode_Equal: return d1->ptr == d2->ptr;
            case ENCompareMode_Lower: return d1->ptr < d2->ptr;
            case ENCompareMode_LowerOrEqual: return d1->ptr <= d2->ptr;
            case ENCompareMode_Greater: return d1->ptr > d2->ptr;
            case ENCompareMode_GreaterOrEqual: return d1->ptr >= d2->ptr;
            default: NBASSERT(FALSE) break;
        }
    }
    return FALSE;
}

//STNBMemoryBlockGap

#define STNBMemoryBlockGap_Zero { 0, 0 }

typedef struct STNBMemoryBlockGap_ {
    UI32       iStart;  //start of gap in bytes
    UI32       sz;      //size of gap in bytes
} STNBMemoryBlockGap;

BOOL NBCompare_NBMemoryBlockGap(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
    NBASSERT(dataSz == sizeof(STNBMemoryBlockGap))
    if(dataSz == sizeof(STNBMemoryBlockGap)){
        const STNBMemoryBlockGap* d1 = (STNBMemoryBlockGap*)data1;
        const STNBMemoryBlockGap* d2 = (STNBMemoryBlockGap*)data2;
        switch (mode) {
            case ENCompareMode_Equal: return d1->iStart == d2->iStart;
            case ENCompareMode_Lower: return d1->iStart < d2->iStart;
            case ENCompareMode_LowerOrEqual: return d1->iStart <= d2->iStart;
            case ENCompareMode_Greater: return d1->iStart > d2->iStart;
            case ENCompareMode_GreaterOrEqual: return d1->iStart >= d2->iStart;
            default: NBASSERT(FALSE) break;
        }
    }
    return FALSE;
}

//STNBMemoryBlockChunk

#define STNBMemoryBlockChunk_Zero   { NULL, 0, 0, 0 }

typedef struct STNBMemoryBlockChunk_ {
    BYTE*       ptr;        //allocated memory
    UI32        ptrSz;      //allocated memory size
    UI32        rngStart;   //first usable position
    UI32        rngAfterEnd;//first non-usable position.
} STNBMemoryBlockChunk;

//STNBMemoryBlockState

#define STNBMemoryBlockState_Zero   { 0, 0 }

typedef struct STNBMemoryBlockState_ {
    UI32        szAvail;                //for 'NBMemoryBlock_malloc' early return
    UI32        szSmallestMallocFailed; //for 'NBMemoryBlock_malloc' early return
} STNBMemoryBlockState;

//STNBMemoryBlockOpq

typedef struct STNBMemoryBlockOpq_ {
    STNBObject              prnt;
    STNBMemoryBlockChunk    chunk;
    STNBMemoryBlockCfg      cfg;
    STNBMemoryBlockState    state;
    STNBArraySorted         ptrs;   //STNBMemoryBlockPtr, returned by 'NBMemoryBlock_malloc'
    STNBArraySorted         gaps;   //STNBMemoryBlockGap
} STNBMemoryBlockOpq;

NB_OBJREF_BODY(NBMemoryBlock, STNBMemoryBlockOpq, NBObject)

//

void NBMemoryBlock_initZeroed(STNBObject* obj) {
    STNBMemoryBlockOpq* opq = (STNBMemoryBlockOpq*)obj;
    //
    NBArraySorted_initWithSz(&opq->ptrs, sizeof(STNBMemoryBlockPtr), NBCompare_NBMemoryBlockPtr, 0, 256, 0.1f);
    NBArraySorted_initWithSz(&opq->gaps, sizeof(STNBMemoryBlockGap), NBCompare_NBMemoryBlockGap, 0, 128, 0.1f);
}

void NBMemoryBlock_uninitLocked(STNBObject* obj){
    STNBMemoryBlockOpq* opq = (STNBMemoryBlockOpq*)obj;
    //
    {
        if(opq->chunk.ptr != NULL){
            NBMemory_free(opq->chunk.ptr);
            opq->chunk.ptr    = NULL;
        }
        NBMemory_setZeroSt(opq->chunk, STNBMemoryBlockChunk);
    }
    NBStruct_stRelease(NBMemoryBlockCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
    //
    NBArraySorted_release(&opq->ptrs);
    NBArraySorted_release(&opq->gaps);
}

//

BOOL NBMemoryBlock_validateIndexLockepOpq_(STNBMemoryBlockOpq* opq);

//

BOOL NBMemoryBlock_prepare(STNBMemoryBlockRef ref, const STNBMemoryBlockCfg* cfg, STNBAbsPtr* dstPtrAfterEnd){
    BOOL r = FALSE;
    STNBMemoryBlockOpq* opq = (STNBMemoryBlockOpq*)ref.opaque;
    //
    NBObject_lock(opq);
    if(cfg != NULL && opq->chunk.ptr == NULL){
        const UI32 idxsAlign = (cfg->idxsAlign > 0 ? cfg->idxsAlign : 4);   //individual pointers alignment
        const UI32 sizeAlign = (cfg->sizeAlign > 0 ? cfg->sizeAlign : idxsAlign > 0 ? idxsAlign : 4);   //whole memory block size alignment
        STNBMemoryBlockChunk chunkN = STNBMemoryBlockChunk_Zero;
        chunkN.rngStart     = (cfg->idxZeroIsValid ? 0 : idxsAlign);
        chunkN.rngAfterEnd  = (cfg->size + idxsAlign - 1) / idxsAlign * idxsAlign;
        chunkN.ptrSz        = (chunkN.rngAfterEnd + sizeAlign - 1) / sizeAlign * sizeAlign;
        chunkN.ptr          = (BYTE*)NBMemory_alloc(chunkN.ptrSz);
        if(chunkN.ptr != NULL){
            //copy chunk
            {
                if(opq->chunk.ptr != NULL){
                    NBMemory_free(opq->chunk.ptr);
                    opq->chunk.ptr = NULL;
                }
                opq->chunk = chunkN;
            }
            //copy cfg
            {
                NBStruct_stRelease(NBMemoryBlockCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
                NBStruct_stClone(NBMemoryBlockCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
                opq->cfg.sizeAlign  = sizeAlign;
                opq->cfg.idxsAlign  = idxsAlign;
                opq->cfg.size       = chunkN.rngAfterEnd;
            }
            //set initial state
            {
                NBArraySorted_empty(&opq->ptrs);
                NBArraySorted_empty(&opq->gaps);
                //reset state
                NBMemory_setZeroSt(opq->state, STNBMemoryBlockState);
                //register the whole range as the initial gap
                if(chunkN.rngStart < chunkN.rngAfterEnd){
                    STNBMemoryBlockGap gap = STNBMemoryBlockGap_Zero;
                    gap.iStart  = chunkN.rngStart;
                    gap.sz      = chunkN.rngAfterEnd - chunkN.rngStart;
                    NBArraySorted_addValue(&opq->gaps, gap);
                    opq->state.szAvail = gap.sz;
                }
            }
            //TMP
            //NBMemoryBlock_validateIndexLockepOpq_(opq);
            if(dstPtrAfterEnd != NULL){
                dstPtrAfterEnd->idx = chunkN.rngAfterEnd;
                dstPtrAfterEnd->ptr = chunkN.ptr + chunkN.rngAfterEnd;
            }
            r = TRUE;
        }
    }
    NBObject_unlock(opq);
    return r;
}

//allocations

STNBAbsPtr NBMemoryBlock_malloc(STNBMemoryBlockRef ref, const UI32 usableSz){
    STNBAbsPtr r = STNBAbsPtr_Zero;
    STNBMemoryBlockOpq* opq = (STNBMemoryBlockOpq*)ref.opaque;
    NBObject_lock(opq);
    if(opq->cfg.idxsAlign > 0){
        const UI32 sz = (usableSz + opq->cfg.idxsAlign - 1) / opq->cfg.idxsAlign * opq->cfg.idxsAlign;
        if(sz <= opq->state.szAvail && (opq->state.szSmallestMallocFailed == 0 || sz < opq->state.szSmallestMallocFailed)){
            //search for a gap
            STNBMemoryBlockGap* gStart = NBArraySorted_dataPtr(&opq->gaps, STNBMemoryBlockGap);
            STNBMemoryBlockGap* g = gStart;
            const STNBMemoryBlockGap* gAfterEnd = g + opq->gaps.use;
            while(g < gAfterEnd){
                if(g->sz >= sz){
                    //register pointer
                    STNBMemoryBlockPtr ptr = STNBMemoryBlockPtr_Zero;
                    ptr.ptr = &opq->chunk.ptr[g->iStart];
                    ptr.sz  = sz;
                    NBArraySorted_addValue(&opq->ptrs, ptr);
                    //remove/modify gap
                    if(g->sz == sz){
                        //remove gap
                        NBArraySorted_removeItemAtIndex(&opq->gaps, (SI32)(g - gStart));
                    } else {
                        //edit gap
                        g->iStart   += sz;
                        g->sz       -= sz;
                    }
                    NBASSERT(opq->state.szAvail >= sz)
                    opq->state.szAvail -= sz;
                    r.idx   = (UI32)((BYTE*)ptr.ptr - opq->chunk.ptr);
                    r.ptr   = ptr.ptr;
                    break;
                }
                //next
                g++;
            }
            //keep track of last failed 'NBMemoryBlock_malloc'
            if(r.ptr == NULL && opq->state.szSmallestMallocFailed > sz){
                opq->state.szSmallestMallocFailed = sz;
            }
            //TMP
            NBMemoryBlock_validateIndexLockepOpq_(opq);
        }
    }
    NBObject_unlock(opq);
    return r;
    
}

BOOL NBMemoryBlock_mfree(STNBMemoryBlockRef ref, const STNBAbsPtr ptr){
    BOOL r = FALSE;
    STNBMemoryBlockOpq* opq = (STNBMemoryBlockOpq*)ref.opaque;
    NBObject_lock(opq);
    {
        //search ptr
        STNBMemoryBlockPtr srchPtr = STNBMemoryBlockPtr_Zero;
        srchPtr.ptr = (void*)ptr.ptr;
        const SI32 iFnd = NBArraySorted_indexOf(&opq->ptrs, &srchPtr, sizeof(srchPtr), NULL);
        if(iFnd < 0){
            //not found
            NBASSERT(FALSE);
        } else {
            STNBMemoryBlockPtr* fndPtr = NBArraySorted_itmPtrAtIndex(&opq->ptrs, STNBMemoryBlockPtr, iFnd);
            NBASSERT(fndPtr->ptr == ptr.ptr)
            //find gap
            {
                SI32 iNxtGap;
                UI32 newGapSz = fndPtr->sz;
                STNBMemoryBlockGap* gStart = NBArraySorted_dataPtr(&opq->gaps, STNBMemoryBlockGap);
                STNBMemoryBlockGap srchGap = STNBMemoryBlockGap_Zero;
                srchGap.iStart = (UI32)((BYTE*)ptr.ptr - opq->chunk.ptr);
                iNxtGap = NBArraySorted_indexForNew(&opq->gaps, &srchGap);
                if(iNxtGap < opq->gaps.use && ((BYTE*)ptr.ptr + fndPtr->sz) == (opq->chunk.ptr + gStart[iNxtGap].iStart)){
                    //merge new gap with next gap
                    gStart[iNxtGap].iStart  -= fndPtr->sz;
                    gStart[iNxtGap].sz      += fndPtr->sz;
                    newGapSz                = gStart[iNxtGap].sz;
                } else if(iNxtGap > 0 && (opq->chunk.ptr + gStart[iNxtGap - 1].iStart + gStart[iNxtGap  - 1].sz) == (BYTE*)ptr.ptr){
                    //merge new gap with prev gap
                    gStart[iNxtGap - 1].sz  += fndPtr->sz;
                    newGapSz                = gStart[iNxtGap - 1].sz;
                } else {
                    //create new gap
                    STNBMemoryBlockGap gap = STNBMemoryBlockGap_Zero;
                    gap.iStart  = (UI32)((BYTE*)ptr.ptr - opq->chunk.ptr);
                    gap.sz      = fndPtr->sz;
                    NBArraySorted_addValue(&opq->gaps, gap);
                }
                NBASSERT(opq->state.szAvail + fndPtr->sz <= (opq->chunk.rngAfterEnd - opq->chunk.rngStart));
                opq->state.szAvail += fndPtr->sz;
                //
                if(opq->state.szSmallestMallocFailed <= newGapSz){
                    opq->state.szSmallestMallocFailed = 0;
                }
            }
            //remove ptr
            NBArraySorted_removeItemAtIndex(&opq->ptrs, iFnd);
            //TMP
            //NBMemoryBlock_validateIndexLockepOpq_(opq);
            r = TRUE;
        }
    }
    NBObject_unlock(opq);
    return r;
}

UI32 NBMemoryBlock_mAvailSz(STNBMemoryBlockRef ref){
    STNBMemoryBlockOpq* opq = (STNBMemoryBlockOpq*)ref.opaque;
    return opq->state.szAvail;
}

void NBMemoryBlock_prepareForNewMallocsActions(STNBMemoryBlockRef ref, const UI32 ammActions){   //increases the index sz
    STNBMemoryBlockOpq* opq = (STNBMemoryBlockOpq*)ref.opaque;
    NBObject_lock(opq);
    {
        NBArraySorted_prepareForGrowth(&opq->ptrs, ammActions);
    }
    NBObject_unlock(opq);
}

void NBMemoryBlock_clear(STNBMemoryBlockRef ref){ //clears the index, all pointers are invalid after this call
    STNBMemoryBlockOpq* opq = (STNBMemoryBlockOpq*)ref.opaque;
    NBObject_lock(opq);
    {
        //set initial state
        NBArraySorted_empty(&opq->ptrs);
        NBArraySorted_empty(&opq->gaps);
        //reset state
        NBMemory_setZeroSt(opq->state, STNBMemoryBlockState);
        //register the whole range as the initial gap
        if(opq->chunk.rngStart < opq->chunk.rngAfterEnd){
            STNBMemoryBlockGap gap = STNBMemoryBlockGap_Zero;
            gap.iStart  = opq->chunk.rngStart;
            gap.sz      = opq->chunk.rngAfterEnd - opq->chunk.rngStart;
            NBArraySorted_addValue(&opq->gaps, gap);
            opq->state.szAvail = gap.sz;
        }
    }
    NBObject_unlock(opq);
}

//dgb

BOOL NBMemoryBlock_validateIndexLockepOpq_(STNBMemoryBlockOpq* opq){
    BOOL r = FALSE;
    UI32 gapsTotalSz = 0, gapLargestSz = 0, ptrsTotalSz = 0;
    //
    BYTE* ptr = opq->chunk.ptr + opq->chunk.rngStart;
    const BYTE* ptrAfterEnd = opq->chunk.ptr + opq->chunk.rngAfterEnd;
    //
    STNBMemoryBlockGap* gStart = NBArraySorted_dataPtr(&opq->gaps, STNBMemoryBlockGap);
    STNBMemoryBlockGap* g = gStart;
    const STNBMemoryBlockGap* gAfterEnd = g + opq->gaps.use;
    //
    STNBMemoryBlockPtr* pStart = NBArraySorted_dataPtr(&opq->ptrs, STNBMemoryBlockPtr);
    STNBMemoryBlockPtr* p = pStart;
    const STNBMemoryBlockPtr* pAfterEnd = p + opq->ptrs.use;
    //walk the pointer ahead
    while(g < gAfterEnd || p < pAfterEnd || ptr < ptrAfterEnd){
        if(g < gAfterEnd && (opq->chunk.ptr + g->iStart) == ptr){
            //a gap
            if(gapLargestSz < g->sz) gapLargestSz = g->sz;
            gapsTotalSz += g->sz;
            ptr += g->sz;
            g++;
        } else if(p < pAfterEnd && p->ptr == ptr){
            //an allocated pointer
            ptrsTotalSz += p->sz;
            ptr += p->sz;
            p++;
        } else {
            NBASSERT(FALSE); //indexes are not valid
            break;
        }
    }
    NBASSERT(gapsTotalSz + ptrsTotalSz == (opq->chunk.rngAfterEnd - opq->chunk.rngStart))
    NBASSERT(g == gAfterEnd && p == pAfterEnd && ptr == ptrAfterEnd)
    NBASSERT(gapsTotalSz == opq->state.szAvail && (opq->state.szSmallestMallocFailed == 0 || gapLargestSz < opq->state.szSmallestMallocFailed))
    if(gapsTotalSz + ptrsTotalSz == (opq->chunk.rngAfterEnd - opq->chunk.rngStart) && g == gAfterEnd && p == pAfterEnd && ptr == ptrAfterEnd && gapsTotalSz == opq->state.szAvail && (opq->state.szSmallestMallocFailed == 0 || gapLargestSz < opq->state.szSmallestMallocFailed)){
        r = TRUE;
    }
    return r;
}
    
BOOL NBMemoryBlock_validateIndex(STNBMemoryBlockRef ref){
    BOOL r = FALSE;
    STNBMemoryBlockOpq* opq = (STNBMemoryBlockOpq*)ref.opaque;
    NBObject_lock(opq);
    {
        r = NBMemoryBlock_validateIndexLockepOpq_(opq);
    }
    NBObject_unlock(opq);
    return r;
}
