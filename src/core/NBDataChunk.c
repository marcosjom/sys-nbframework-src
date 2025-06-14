//
//  NBDataChunk.h
//  thinstream
//
//  Created by Marcos Ortega on 9/3/19.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBDataChunk.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBThreadCond.h"
#include "nb/core/NBDataBuff.h"
//

void NBDataChunk_init(STNBDataChunk* obj){
	NBMemory_setZeroSt(*obj, STNBDataChunk);
}

void NBDataChunk_release(STNBDataChunk* obj){
	//Decrease buffer reader
	if(NBDataBuff_isSet(obj->buff)){
		if(obj->rdrCount > 0){
			NBDataBuff_decreaseReaders(obj->buff, obj->rdrCount);
			obj->rdrCount = 0;
		}
		NBDataBuff_null(&obj->buff);
	}
}

//Optimization (reduces locks and internal data cloning)

void NBDataChunk_preResign(STNBDataChunk* obj){
	obj->preResigned = TRUE;
}

BOOL NBDataChunk_absorbResources(STNBDataChunk* obj, STNBDataChunk* other){
	BOOL r = FALSE;
	if(other != NULL){
		//Must be from same NBDataBuffer 
		if(NBDataBuff_isSet(obj->buff) && NBDataBuff_isSame(obj->buff, other->buff)){
			obj->rdrCount += other->rdrCount;
			other->rdrCount = 0;
			r = TRUE;
		}
	}
	return r;
}

//

BOOL NBDataChunk_setAsOther(STNBDataChunk* obj, const STNBDataChunk* other){
	BOOL r = FALSE;
	if(other != NULL){
		//Must be first time or from same NBDataBuffer 
		if(!NBDataBuff_isSet(obj->buff) || NBDataBuff_isSame(obj->buff, other->buff)){
			UI32 rdrCount = obj->rdrCount;
			//retain DataBuffer
			if(NBDataBuff_isSet(other->buff)){
				if(other->preResigned){
					//just keep resigned data
					rdrCount += other->rdrCount;
					((STNBDataChunk*)other)->rdrCount = 0;
				} else {
					//increase reader
					rdrCount++;
					NBDataBuff_increaseReaders(other->buff, 1);
				}
			}
			//set
			*obj = *other;
			obj->rdrCount = rdrCount;
			//
			r = TRUE;
		}
	}
	NBASSERT(r)
	return r;
}

BOOL NBDataChunk_setAsSubchunk(STNBDataChunk* obj, const STNBDataChunk* other, const UI32 iStart, const UI32 dataSz){
	BOOL r = FALSE;
	if(other != NULL){
		//Must be first time or from same NBDataBuffer 
		if(!NBDataBuff_isSet(obj->buff) || NBDataBuff_isSame(obj->buff, other->buff)){
			NBASSERT((iStart + dataSz) <= other->size)
			if((iStart + dataSz) <= other->size){
				UI32 rdrCount = obj->rdrCount;
				//retain DataBuffer
				if(NBDataBuff_isSet(other->buff)){
					if(other->preResigned){
						rdrCount += other->rdrCount;
						((STNBDataChunk*)other)->rdrCount = 0;
					} else {
						rdrCount++;
						NBDataBuff_increaseReaders(other->buff, 1);
					}
				}
				//set
				obj->data		= (BYTE*)other->data + iStart;
				obj->size		= dataSz;
				obj->buff		= other->buff;
				obj->rdrCount	= rdrCount;
				//
				r = TRUE;
			}
		}
	}
	NBASSERT(r)
	return r;
}
