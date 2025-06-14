//
//  NBDataCursor.h
//  thinstream
//
//  Created by Marcos Ortega on 9/3/19.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBDataCursor.h"
//
#include "nb/core/NBMemory.h"

//NBDataCursorLstnr

typedef struct STNBDataCursorLstnr_ {
	void*						usrData;
	STNBDataCursorLstnrItf		itf;
} STNBDataCursorLstnr;

//NBDataCursorProvider

typedef struct STNBDataCursorProvider_ {
	void*						usrData;
	STNBDataCursorProviderItf	itf;
} STNBDataCursorProvider;

//NBDataCursorOpq

typedef struct STNBDataCursorOpq_ {
	STNBObject					prnt;
	STNBDataCursorProvider		provider;	//current provider
	STNBDataCursorLstnr			lstnr;		//current listener
	//curBuff
	struct {
		STNBDataCursorBuffDef	def;		//current buffer (reading or writting)
		UI32					iPos;		//current position
	} curBuff;
} STNBDataCursorOpq;

NB_OBJREF_BODY(NBDataCursor, STNBDataCursorOpq, NBObject)

//

void NBDataCursor_initZeroed(STNBObject* obj){
	//STNBDataCursorOpq* opq = (STNBDataCursorOpq*)obj;
}

void NBDataCursor_uninitLocked(STNBObject* obj){
	STNBDataCursorOpq* opq = (STNBDataCursorOpq*)obj;
	//release current provider
	{
		//end sequence
		if(opq->provider.itf.endSequence != NULL){
			(*opq->provider.itf.endSequence)(opq->provider.usrData, &opq->curBuff.def);
		}
		//release
		if(opq->provider.itf.release != NULL){
			(*opq->provider.itf.release)(opq->provider.usrData);
		}
		NBMemory_setZeroSt(opq->provider, STNBDataCursorProvider);
		NBMemory_setZero(opq->curBuff);
	}
}

//config

BOOL NBDataCursor_setProvider(STNBDataCursorRef ref, void* usrData, const STNBDataCursorProviderItf* itf){
	STNBDataCursorOpq* opq = (STNBDataCursorOpq*)ref.opaque; NBASSERT(NBDataCursor_isClass(ref)) 
	//retain new provider
	if(usrData != NULL && itf != NULL && itf->retain != NULL){
		(*itf->retain)(usrData);
	}
	//release current provider
	{
		//end sequence
		if(opq->provider.itf.endSequence != NULL){
			(*opq->provider.itf.endSequence)(opq->provider.usrData, &opq->curBuff.def);
		}
		//release
		if(opq->provider.itf.release != NULL){
			(*opq->provider.itf.release)(opq->provider.usrData);
		}
		NBMemory_setZeroSt(opq->provider, STNBDataCursorProvider);
		NBMemory_setZero(opq->curBuff);
	}
	//set new provider
	if(itf != NULL){
		//retain
		opq->provider.usrData = usrData;
		opq->provider.itf = *itf;
		//retain
		if(opq->provider.itf.retain != NULL){
			(*opq->provider.itf.retain)(opq->provider.usrData);
		}
		//start sequence
		if(opq->provider.itf.startSequence != NULL){
			if((*opq->provider.itf.startSequence)(opq->provider.usrData, &opq->curBuff.def)){
				opq->curBuff.iPos = 0;
				//PRINTF_INFO("NBDataCursor, first-buffer is %d bytes.\n", opq->curBuff.def.buff.size);
			}
		}
	}
	return TRUE;
}

BOOL NBDataCursor_setListener(STNBDataCursorRef ref, void* usrData, const STNBDataCursorLstnrItf* itf){
	STNBDataCursorOpq* opq = (STNBDataCursorOpq*)ref.opaque; NBASSERT(NBDataCursor_isClass(ref))
	opq->lstnr.usrData = usrData;
	if(itf == NULL){
		NBMemory_setZeroSt(opq->lstnr.itf, STNBDataCursorLstnrItf);
	} else {
		opq->lstnr.itf		= *itf;
	}
	return TRUE;
}

//get

BOOL NBDataCursor_isMoreAvailable(STNBDataCursorRef ref){ //return TRUE if no more buffers are available at the moment.
	STNBDataCursorOpq* opq = (STNBDataCursorOpq*)ref.opaque; NBASSERT(NBDataCursor_isClass(ref))
	return (
			opq->curBuff.iPos < opq->curBuff.def.buff.data.use
			|| (
				opq->provider.itf.isMoreAvailable != NULL
				&& (*opq->provider.itf.isMoreAvailable)(opq->provider.usrData, &opq->curBuff.def)
				)
			);
}

UI32 NBDataCursor_curBuffBytesRemainCount(const STNBDataCursorRef ref){
	STNBDataCursorOpq* opq = (STNBDataCursorOpq*)ref.opaque; NBASSERT(NBDataCursor_isClass(ref))
	return (UI32)(opq->curBuff.def.buff.data.use - opq->curBuff.iPos);
}

void NBDataCursor_getNextPart(STNBDataCursorRef ref, STNBDataCursorPart* dst){
	STNBDataCursorOpq* opq = (STNBDataCursorOpq*)ref.opaque; NBASSERT(NBDataCursor_isClass(ref))
	//load new buffer (if necesary)
	if(opq->curBuff.iPos >= opq->curBuff.def.buff.data.use){
		if(opq->provider.itf.continueSequence != NULL){
			if((*opq->provider.itf.continueSequence)(opq->provider.usrData, &opq->curBuff.def)){
				opq->curBuff.iPos = 0;
				//PRINTF_INFO("NBDataCursor, next-buffer is %d bytes.\n", opq->curBuff.def.buff.size);
			}
		}
	}
	//return buff
	if(dst != NULL){
		//the provided data is retained while sequence is active.
		NBDataPtr_setAsExternalBytes(&dst->data, (BYTE*)opq->curBuff.def.buff.data.ptr + opq->curBuff.iPos, opq->curBuff.def.buff.data.use - opq->curBuff.iPos);
		dst->canContinueHint = opq->curBuff.def.buff.canContinueHint;
	}
	//move to end
	opq->curBuff.iPos = opq->curBuff.def.buff.data.use;
}

void NBDataCursor_getNextPartBytes(STNBDataCursorRef ref, STNBDataCursorPart* dst, const UI32 bytesMax){ //consumed bytes from the current buffer up to the maximun
	STNBDataCursorOpq* opq = (STNBDataCursorOpq*)ref.opaque; NBASSERT(NBDataCursor_isClass(ref))
	UI32 remain, consumed;
	//load new buffer (if necesary)
	if(opq->curBuff.iPos >= opq->curBuff.def.buff.data.use){
		if(opq->provider.itf.continueSequence != NULL){
			if((*opq->provider.itf.continueSequence)(opq->provider.usrData, &opq->curBuff.def)){
				opq->curBuff.iPos = 0;
				//PRINTF_INFO("NBDataCursor, next-buffer is %d bytes.\n", opq->curBuff.def.buff.size);
			}
		}
	}
	remain		= (opq->curBuff.def.buff.data.use - opq->curBuff.iPos);
	consumed	= (remain > bytesMax ? bytesMax : remain);
	//return buff
	if(dst != NULL){
		//the provided data is retained while sequence is active.
		NBDataPtr_setAsExternalBytes(&dst->data, (BYTE*)opq->curBuff.def.buff.data.ptr + opq->curBuff.iPos, consumed);
		dst->canContinueHint = (consumed < remain || opq->curBuff.def.buff.canContinueHint);
	}
	//move to end
	opq->curBuff.iPos += consumed;
}

//NBDataCursorPart

void NBDataCursorPart_init(STNBDataCursorPart* obj){
	NBMemory_setZeroSt(*obj, STNBDataCursorPart)
	NBDataPtr_init(&obj->data);
}

void NBDataCursorPart_release(STNBDataCursorPart* obj){
	NBDataPtr_release(&obj->data);
}

void NBDataCursorPart_setDataPtr(STNBDataCursorPart* obj, STNBDataPtr* ptr, void* hdr, const UI32 hdrSz, const BOOL canContinueHint){
	if(ptr == NULL){
		NBDataPtr_setAsExternalBytes(&obj->data, NULL, 0);
	} else {
		NBDataPtr_setAsExternalBytes(&obj->data, ptr->ptr, ptr->use);
	}
	obj->hdr.data = hdr;
	obj->hdr.size = hdrSz;
	obj->canContinueHint = canContinueHint;
}

void NBDataCursorPart_setDataBytes(STNBDataCursorPart* obj, void* data, const UI32 dataSz, void* hdr, const UI32 hdrSz, const BOOL canContinueHint){
	NBDataPtr_setAsExternalBytes(&obj->data, data, dataSz);
	obj->hdr.data = hdr;
	obj->hdr.size = hdrSz;
	obj->canContinueHint = canContinueHint;
}

//NBDataCursorBuffDef

void NBDataCursorBuffDef_init(STNBDataCursorBuffDef* obj){
	NBMemory_setZeroSt(*obj, STNBDataCursorBuffDef);
	NBDataCursorPart_init(&obj->buff);
}				

void NBDataCursorBuffDef_release(STNBDataCursorBuffDef* obj){
	NBDataCursorPart_release(&obj->buff);
}

void NBDataCursorBuffDef_setDataPtr(STNBDataCursorBuffDef* obj, STNBDataPtr* ptr, void* hdr, const UI32 hdrSz, const BOOL canContinueHint, void* usrData){
	NBDataCursorPart_setDataPtr(&obj->buff, ptr, hdr, hdrSz, canContinueHint);
	obj->usrData = usrData;
}

void NBDataCursorBuffDef_setDataBytes(STNBDataCursorBuffDef* obj, void* data, const UI32 dataSz, void* hdr, const UI32 hdrSz, const BOOL canContinueHint, void* usrData){
	NBDataCursorPart_setDataBytes(&obj->buff, data, dataSz, hdr, hdrSz, canContinueHint);
	obj->usrData = usrData;
}
