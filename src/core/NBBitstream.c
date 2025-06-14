//
//  NBBitstream.c
//  nbframework
//
//  Created by Marcos Ortega on 9/21/20.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBBitstream.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBString.h"

//Methods

UI8 NBBitstream_readU8Lwr_(STNBBitstream* obj, UI8 bits); //bits wont move to next byte
UI8 NBBitstream_readU8Eql_(STNBBitstream* obj, UI8 bits); //bits will move to start of next byte
UI8 NBBitstream_readU8Hgr_(STNBBitstream* obj, UI8 bits); //bits will move to non-start of next byte

//

void NBBitstream_moveToNextPtrPayload_(STNBBitstream* obj);	//previously reading prefix, now move to continuos payload
void NBBitstream_moveToNextPtrChunkNext_(STNBBitstream* obj);	//previously reading prefix or chunk, now move to the next-chunkc

//

void NBBitstream_moveToPrevPtrPrefixPay_(STNBBitstream* obj);	//previously reading payload, now move to prefix
void NBBitstream_moveToPrevPtrPrefixChunk_(STNBBitstream* obj);//previously reading chunk, now move to prefix
void NBBitstream_moveToPrevPtrChunkPrev_(STNBBitstream* obj);	//previously reading second+ chunk, now move to prev chunk

//Bitstream

static BYTE NBBitstreamBitsMasks_[] = {
	0x0		//0000-0000
	, 0x1	//0000-0001
	, 0x3	//0000-0011
	, 0x7	//0000-0111
	, 0xF	//0000-1111
	, 0x1F	//0001-1111
	, 0x3F	//0011-1111
	, 0x7F	//0111-1111
	, 0xFF	//1111-1111
};

//NBBitstream

void NBBitstream_init(STNBBitstream* obj){
	NBMemory_setZeroSt(*obj, STNBBitstream);
}

void NBBitstream_release(STNBBitstream* obj){
	NBMemory_setZeroSt(*obj, STNBBitstream);
}

//

void NBBitstream_setAsOther(STNBBitstream* obj, const STNBBitstream* other){
	*obj = *other;
}

void NBBitstream_setAsPayload(STNBBitstream* obj, const void* prefix, const UI32 prefixSz, const void* pay, const UI32 paySz){
	obj->zeroesSeqCount		= 0;
	obj->stopFlags			= 0;
	obj->src.type			= ENNBBitstreamSrcType_Payload;
	obj->moveToPrevPtrMthod	= NULL;
	//
	if(prefix != NULL && prefixSz > 0){
		//start with prefix
		obj->bitsRemain					= 8;
		obj->src.start					= (BYTE*)prefix;
		obj->src.cur					= (BYTE*)prefix;
		obj->src.afterEnd				= (BYTE*)prefix + prefixSz;
		obj->src.global.iPos			= 0;
		obj->src.prefix.start			= (BYTE*)prefix;
		obj->src.prefix.afterEnd 		= (BYTE*)prefix + prefixSz;
		obj->src.remain.payload.start	= (BYTE*)pay;
		obj->src.remain.payload.afterEnd = (BYTE*)pay + paySz;
		if(pay != NULL && paySz > 0){
			obj->moveToNextPtrMthod		= NBBitstream_moveToNextPtrPayload_;
		} else {
			obj->moveToNextPtrMthod		= NULL;
		}
		if(*obj->src.cur == 0x00){
			obj->zeroesSeqCount++;
		}
	} else if(pay != NULL && paySz > 0){
		//start with payload
		obj->bitsRemain					= 8;
		obj->src.start					= (BYTE*)pay;
		obj->src.cur					= (BYTE*)pay;
		obj->src.afterEnd				= (BYTE*)pay + paySz;
		obj->src.global.iPos			= 0;
		obj->src.prefix.start			= NULL;
		obj->src.prefix.afterEnd 		= NULL;
		obj->src.remain.payload.start	= (BYTE*)pay;
		obj->src.remain.payload.afterEnd = (BYTE*)pay + paySz;
		obj->moveToNextPtrMthod			= NULL;
		if(*obj->src.cur == 0x00){
			obj->zeroesSeqCount++;
		}
	} else {
		//empty
		obj->bitsRemain					= 0;
		obj->src.start					= NULL;
		obj->src.cur					= NULL;
		obj->src.afterEnd				= NULL;
		obj->src.global.iPos			= 0;
		obj->src.prefix.start			= NULL;
		obj->src.prefix.afterEnd 		= NULL;
		obj->src.remain.payload.start	= NULL;
		obj->src.remain.payload.afterEnd = NULL;
		obj->moveToNextPtrMthod			= NULL;
	}
}

void NBBitstream_setAsChunks(STNBBitstream* obj, const void* prefix, const UI32 prefixSz, const STNBDataPtr* pChunks, const SI32 pChunksSz){
	const STNBDataPtr* chunks = pChunks;
	SI32 chunksSz = pChunksSz;
	obj->zeroesSeqCount		= 0;
	obj->stopFlags			= 0;
	obj->src.type			= ENNBBitstreamSrcType_Chuncks;
	obj->moveToPrevPtrMthod	= NULL;
	//filter empty chunks
	if(chunks != NULL && chunksSz > 0){
		const STNBDataPtr* chunkAfterEnd = chunks + chunksSz;
		//filter left side
		while(chunks < chunkAfterEnd && (chunks->ptr == NULL || chunks->use <= 0)){
			chunks++; chunksSz--;
		}
		//filter right side
		while(chunks < chunkAfterEnd && (chunkAfterEnd[-1].ptr == NULL || chunkAfterEnd[-1].use <= 0)){
			chunkAfterEnd--; chunksSz--;
		}
	}
	//
	if(prefix != NULL && prefixSz > 0){
		//start with prefix
		obj->bitsRemain					= 8;
		obj->src.start					= (BYTE*)prefix;
		obj->src.cur					= (BYTE*)prefix;
		obj->src.afterEnd				= (BYTE*)prefix + prefixSz;
		obj->src.global.iPos			= 0;
		obj->src.prefix.start			= (BYTE*)prefix;
		obj->src.prefix.afterEnd 		= (BYTE*)prefix + prefixSz;
		obj->src.remain.chuncks.start	= chunks;
		obj->src.remain.chuncks.next	= chunks;
		obj->src.remain.chuncks.afterEnd = chunks + chunksSz;
		if(chunks != NULL && chunksSz > 0){
			obj->moveToNextPtrMthod		= NBBitstream_moveToNextPtrChunkNext_;
		} else {
			obj->moveToNextPtrMthod		= NULL;
		}
		if(*obj->src.cur == 0x00){
			obj->zeroesSeqCount++;
		}
	} else if(chunks != NULL && chunksSz > 0){
		//start with payload
		obj->bitsRemain					= 8;
		obj->src.start					= (BYTE*)chunks->ptr; //first chunk
		obj->src.cur					= (BYTE*)chunks->ptr; //first chunk
		obj->src.afterEnd				= (BYTE*)chunks->ptr + chunks->use;
		obj->src.global.iPos			= 0;
		obj->src.prefix.start			= NULL;
		obj->src.prefix.afterEnd 		= NULL;
		obj->src.remain.chuncks.start	= chunks;
		obj->src.remain.chuncks.next	= chunks + 1; //second chunks and ahead
		obj->src.remain.chuncks.afterEnd = chunks + chunksSz;
		if((chunks + 1) < (chunks + chunksSz)){
			obj->moveToNextPtrMthod		= NBBitstream_moveToNextPtrChunkNext_;
		} else {
			obj->moveToNextPtrMthod		= NULL;
		}
		if(*obj->src.cur == 0x00){
			obj->zeroesSeqCount++;
		}
	} else {
		//empty
		obj->bitsRemain					= 0;
		obj->src.start					= NULL;
		obj->src.cur					= NULL;
		obj->src.afterEnd				= NULL;
		obj->src.global.iPos			= 0;
		obj->src.prefix.start			= NULL;
		obj->src.prefix.afterEnd 		= NULL;
		obj->src.remain.chuncks.start	= NULL;
		obj->src.remain.chuncks.next	= NULL;
		obj->src.remain.chuncks.afterEnd = NULL;
		obj->moveToNextPtrMthod			= NULL;
	}
}

//moveToNextPtr

void NBBitstream_moveToNextPtrPayload_(STNBBitstream* obj){	//previously reading prefix, now move to continuos payload
	NBASSERT(obj->src.type == ENNBBitstreamSrcType_Payload)
	NBASSERT(obj->src.cur != obj->src.remain.payload.start && obj->src.remain.payload.start != NULL && obj->src.remain.payload.start < obj->src.remain.payload.afterEnd)
	obj->src.global.iPos	+= (SI32)(obj->src.afterEnd - obj->src.cur);
	obj->bitsRemain			= 8;
	obj->src.start			= obj->src.cur = obj->src.remain.payload.start;
	obj->src.afterEnd		= obj->src.remain.payload.afterEnd;
	obj->moveToNextPtrMthod	= NULL;
	obj->moveToPrevPtrMthod	= NBBitstream_moveToPrevPtrPrefixPay_; NBASSERT(obj->src.prefix.start != NULL)
	//PRINTF_INFO("NBBitstream, moved to next-pay-ptr.\n");
}

void NBBitstream_moveToNextPtrChunkNext_(STNBBitstream* obj){	//previously reading first-chunk, now move to the next-chunkc
	NBASSERT(obj->src.type == ENNBBitstreamSrcType_Chuncks)
	NBASSERT(obj->src.remain.chuncks.next < obj->src.remain.chuncks.afterEnd)
	obj->src.global.iPos	+= (SI32)(obj->src.afterEnd - obj->src.cur);
	obj->bitsRemain			= 8;
	obj->src.start			= obj->src.cur = (BYTE*)obj->src.remain.chuncks.next->ptr;
	obj->src.afterEnd		= obj->src.cur + obj->src.remain.chuncks.next->use;
	obj->moveToPrevPtrMthod	= (obj->src.remain.chuncks.next == obj->src.remain.chuncks.start ? NBBitstream_moveToPrevPtrPrefixChunk_ : NBBitstream_moveToPrevPtrChunkPrev_);
	obj->src.remain.chuncks.next++;
	if(obj->src.remain.chuncks.next >= obj->src.remain.chuncks.afterEnd){
		obj->moveToNextPtrMthod	= NULL;
	}
	//PRINTF_INFO("NBBitstream, moved to next-chunk-ptr.\n");
}

//moveToPrevPtr

void NBBitstream_moveToPrevPtrPrefixPay_(STNBBitstream* obj){	//previously reading payload, now move to prefix
	NBASSERT(obj->src.type == ENNBBitstreamSrcType_Payload)
	NBASSERT(obj->src.remain.payload.start != NULL)
	NBASSERT(obj->src.cur >= obj->src.remain.payload.start && obj->src.cur <= obj->src.remain.payload.afterEnd)
	NBASSERT(obj->src.prefix.start != NULL && obj->src.prefix.start < obj->src.prefix.afterEnd)
	NBASSERT(obj->src.global.iPos >= (SI32)(obj->src.cur - obj->src.start) + 1)
	obj->src.global.iPos	-= (SI32)(obj->src.cur - obj->src.start) + 1;
	obj->bitsRemain			= 8;
	obj->src.start			= obj->src.prefix.start;
	obj->src.afterEnd		= obj->src.prefix.afterEnd;
	obj->src.cur			= obj->src.afterEnd - 1;
	obj->moveToNextPtrMthod	= NBBitstream_moveToNextPtrPayload_;
	obj->moveToPrevPtrMthod	= NULL;
	//PRINTF_INFO("NBBitstream, moved to pay-prefix-pay.\n");
}

void NBBitstream_moveToPrevPtrPrefixChunk_(STNBBitstream* obj){ //previously reading chunk, now move to prefix
	NBASSERT(obj->src.type == ENNBBitstreamSrcType_Chuncks)
	NBASSERT(obj->src.remain.chuncks.start != NULL)
	NBASSERT(obj->src.global.iPos >= (SI32)(obj->src.cur - obj->src.start) + 1)
	NBASSERT((SI32)(obj->src.cur - obj->src.start) >= 0)
	obj->src.global.iPos	-= (SI32)(obj->src.cur - obj->src.start) + 1;
	obj->bitsRemain			= 8;
	//
	NBASSERT((obj->src.remain.chuncks.start + 1) == obj->src.remain.chuncks.next)
	obj->src.remain.chuncks.next--;
	//
	obj->src.start			= obj->src.prefix.start;
	obj->src.afterEnd		= obj->src.prefix.afterEnd;
	obj->src.cur			= obj->src.afterEnd - 1;
	obj->moveToNextPtrMthod	= NBBitstream_moveToNextPtrChunkNext_;
	obj->moveToPrevPtrMthod	= NULL;
	//PRINTF_INFO("NBBitstream, moved to prev-prefix-chunks.\n");
}

void NBBitstream_moveToPrevPtrChunkPrev_(STNBBitstream* obj){	//previously reading second+ chunk, now move to prev chunk
	NBASSERT(obj->src.type == ENNBBitstreamSrcType_Chuncks)
	NBASSERT(obj->src.remain.chuncks.start < obj->src.remain.chuncks.next && obj->src.remain.chuncks.next <= obj->src.remain.chuncks.afterEnd)
	NBASSERT(obj->src.global.iPos >= (SI32)(obj->src.cur - obj->src.start))
	NBASSERT((SI32)(obj->src.cur - obj->src.start) >= 0)
	obj->src.global.iPos	-= (SI32)(obj->src.cur - obj->src.start);
	obj->bitsRemain			= 8;
	//
	NBASSERT((obj->src.remain.chuncks.start + 1) < obj->src.remain.chuncks.next)
	obj->src.remain.chuncks.next--;
	obj->src.start			= (BYTE*)obj->src.remain.chuncks.next[-1].ptr;
	obj->src.afterEnd		= obj->src.start + obj->src.remain.chuncks.next[-1].use;
	obj->src.cur			= obj->src.afterEnd - (obj->src.remain.chuncks.next[-1].use > 0 ? 1 : 0);
	//non-empty
	if(obj->src.start < obj->src.afterEnd){
		NBASSERT(obj->src.global.iPos > 0)
		obj->src.global.iPos--;
	}
	if((obj->src.remain.chuncks.start + 1) == obj->src.remain.chuncks.next){
		obj->moveToPrevPtrMthod = (obj->src.prefix.start != NULL ? NBBitstream_moveToPrevPtrPrefixChunk_ : NULL);
	}
	obj->moveToNextPtrMthod	= NBBitstream_moveToNextPtrChunkNext_;
	//PRINTF_INFO("NBBitstream, moved to prev-chunk.\n");
}

//U

UI8 NBBitstream_readU8Lwr_(STNBBitstream* obj, UI8 bits){
	//Lower-version: bits wont move to next byte
	NBASSERT(bits > 0 && bits < obj->bitsRemain)
	return (*obj->src.cur >> (obj->bitsRemain = (obj->bitsRemain - bits))) & NBBitstreamBitsMasks_[bits];
}

UI8 NBBitstream_readU8Eql_(STNBBitstream* obj, UI8 bits){
	//Equal-version: bits to consume are exactly bits remaining
	NBASSERT(bits > 0 && bits == obj->bitsRemain)
	//
	if(obj->bitsRemain == 0){
		obj->stopFlags |= NB_BITSTREAM_STOP_OVERFLOWED;
		return 0;
	}
	//value
	bits = (*(obj->src.cur++) & NBBitstreamBitsMasks_[obj->bitsRemain]);
	obj->src.global.iPos++;
	//open next byte
	while(obj->src.cur == obj->src.afterEnd){
		if(obj->moveToNextPtrMthod == NULL){
			obj->bitsRemain = 0;
			break;
		} else {
			(*obj->moveToNextPtrMthod)(obj);
			NBASSERT(obj->bitsRemain == 8)
		}
	}
	if(obj->src.cur < obj->src.afterEnd){
		obj->bitsRemain = 8;
		if(*obj->src.cur == 0x00){
			obj->zeroesSeqCount++;
		} else {
			if(*obj->src.cur == 0x03 && obj->zeroesSeqCount > 1){
				obj->src.cur++; //PRINTF_INFO("NBBitstream, em_prev_0x000003 byte 0x03 skipped.\n");
				obj->src.global.iPos++;
				//open next byte
				while(obj->src.cur == obj->src.afterEnd){
					if(obj->moveToNextPtrMthod == NULL){
						obj->bitsRemain = 0;
						break;
					} else {
						(*obj->moveToNextPtrMthod)(obj);
						NBASSERT(obj->bitsRemain == 8)
					}
				}
			}
			obj->zeroesSeqCount = 0;
		}
	}
	return bits; //just reusing 'bits' var
}

UI8 NBBitstream_readU8Hgr_(STNBBitstream* obj, UI8 bits){
	//Higher-version: bits consume current and next byte
	NBASSERT(bits > 0 && bits > obj->bitsRemain)
	NBASSERT(obj->src.cur <= obj->src.afterEnd)
	UI8 bitsRght, left;
	//
	if(obj->bitsRemain == 0){
		obj->stopFlags |= NB_BITSTREAM_STOP_OVERFLOWED;
		return 0;
	}
	//consume current byte
	bitsRght	= (bits - obj->bitsRemain);
	left		= ((*(obj->src.cur++) & NBBitstreamBitsMasks_[obj->bitsRemain])) << bitsRght;
	obj->src.global.iPos++;
	//open next byte
	while(obj->src.cur == obj->src.afterEnd){
		if(obj->moveToNextPtrMthod == NULL){
			obj->bitsRemain	= 0;
			obj->stopFlags	|= NB_BITSTREAM_STOP_OVERFLOWED;
			return 0;
		} else {
			(*obj->moveToNextPtrMthod)(obj);
			NBASSERT(obj->bitsRemain == 8)
		}
	}
	if(obj->src.cur < obj->src.afterEnd){
		obj->bitsRemain = 8 - bitsRght;
		if(*obj->src.cur == 0x00){
			obj->zeroesSeqCount++;
		} else {
			if(*obj->src.cur == 0x03 && obj->zeroesSeqCount > 1){
				obj->src.cur++; //PRINTF_INFO("NBBitstream, em_prev_0x000003 byte 0x03 skipped.\n");
				obj->src.global.iPos++;
				//open next byte
				while(obj->src.cur == obj->src.afterEnd){
					if(obj->moveToNextPtrMthod == NULL){
						obj->bitsRemain	= 0;
						obj->stopFlags	|= NB_BITSTREAM_STOP_OVERFLOWED;
						return 0;
					} else {
						(*obj->moveToNextPtrMthod)(obj);
						NBASSERT(obj->bitsRemain == 8)
						obj->bitsRemain = 8 - bitsRght;
					}
				}
			}
			obj->zeroesSeqCount = 0;
		}
	}
	//read next byte
	NBASSERT(obj->src.cur < obj->src.afterEnd && obj->bitsRemain > 0) //should be validated before this point
	return (left | ((*obj->src.cur >> obj->bitsRemain) & NBBitstreamBitsMasks_[bitsRght]));
}

//

UI8 NBBitstream_readU8(STNBBitstream* obj, UI8 bits){
	return (bits < obj->bitsRemain ? NBBitstream_readU8Lwr_ : bits == obj->bitsRemain ? NBBitstream_readU8Eql_ : NBBitstream_readU8Hgr_)(obj, bits);
}

UI16 NBBitstream_readU16(STNBBitstream* obj, const UI8 bits){
	NBASSERT(bits > 8 && bits <= 16)
	if(obj->bitsRemain == 8){
		//Full first-byte and full-or-partial next-byte
		return ((UI16)NBBitstream_readU8Eql_(obj, 8) << 8) | (bits == 16 ? NBBitstream_readU8Eql_ : NBBitstream_readU8Lwr_)(obj, bits - 8);
	} else {
		const UI8 bitsRight = (bits - obj->bitsRemain);
		if(bitsRight <= 8){
			//Remaining first byte and partial next byte
			return ((UI16)NBBitstream_readU8Eql_(obj, obj->bitsRemain) << bitsRight) | (bitsRight == 8 ? NBBitstream_readU8Eql_ : NBBitstream_readU8Lwr_)(obj, bitsRight);
		} else {
			//Remaining first byte, full next byte and partial next byte
			return ((UI16)NBBitstream_readU8Eql_(obj, obj->bitsRemain) << bitsRight) | ((UI16)NBBitstream_readU8Eql_(obj, 8) << (bitsRight - 8)) | NBBitstream_readU8Lwr_(obj, (bitsRight - 8));
		}
	}
	return 0;
	//return ((UI16)NBBitstream_readU8(obj, bits % 8) << 8) | NBBitstream_readU8(obj, 8);
}

UI32 NBBitstream_readU24(STNBBitstream* obj, const UI8 bits){
	NBASSERT(bits > 16 && bits <= 24)
	if(obj->bitsRemain == 8){
		//Full first-byte and full-or-partial next-byte
		return ((UI32)NBBitstream_readU8Eql_(obj, 8) << 16) | ((UI32)NBBitstream_readU8Eql_(obj, 8) << 8) | (bits == 24 ? NBBitstream_readU8Eql_ : NBBitstream_readU8Lwr_)(obj, bits - 16);
	} else {
		const UI8 bitsRight = (bits - obj->bitsRemain); NBASSERT(bitsRight > 0)
		if(bitsRight <= 16){
			//Remaining first byte and partial next byte
			return ((UI32)NBBitstream_readU8Eql_(obj, obj->bitsRemain) << bitsRight) | ((UI32)NBBitstream_readU8Eql_(obj, 8) << (bitsRight - 8)) | (bitsRight == 16 ? NBBitstream_readU8Eql_ : NBBitstream_readU8Lwr_)(obj, bitsRight - 8);
		} else {
			//Remaining first byte, full next byte and partial next byte
			return ((UI32)NBBitstream_readU8Eql_(obj, obj->bitsRemain) << bitsRight) | ((UI32)NBBitstream_readU8Eql_(obj, 8) << (bitsRight - 8)) | ((UI32)NBBitstream_readU8Eql_(obj, 8) << (bitsRight - 16)) | NBBitstream_readU8Lwr_(obj, (bitsRight - 16));
		}
	}
	return 0;
	//return ((UI32)NBBitstream_readU8(obj, bits % 16) << 16) | ((UI32)NBBitstream_readU8(obj, 8) << 8) | NBBitstream_readU8(obj, 8);
}

UI32 NBBitstream_readU32(STNBBitstream* obj, const UI8 bits){
	NBASSERT(bits > 24 && bits <= 32)
	if(obj->bitsRemain == 8){
		//Full first-byte and full-or-partial next-byte
		return ((UI32)NBBitstream_readU8Eql_(obj, 8) << 24) | ((UI32)NBBitstream_readU8Eql_(obj, 8) << 16) | ((UI32)NBBitstream_readU8Eql_(obj, 8) << 8) | (bits == 32 ? NBBitstream_readU8Eql_ : NBBitstream_readU8Lwr_)(obj, bits - 24);
	} else {
		const UI8 bitsRight = (bits - obj->bitsRemain); NBASSERT(bitsRight > 0)
		if(bitsRight <= 24){
			//Remaining first byte and partial next byte
			return ((UI32)NBBitstream_readU8Eql_(obj, obj->bitsRemain) << bitsRight) | ((UI32)NBBitstream_readU8Eql_(obj, 8) << (bitsRight - 8)) | ((UI32)NBBitstream_readU8Eql_(obj, 8) << (bitsRight - 16)) | (bitsRight == 24 ? NBBitstream_readU8Eql_ : NBBitstream_readU8Lwr_)(obj, bitsRight - 16);
		} else {
			//Remaining first byte, full next byte and partial next byte
			return ((UI32)NBBitstream_readU8Eql_(obj, obj->bitsRemain) << bitsRight) | ((UI32)NBBitstream_readU8Eql_(obj, 8) << (bitsRight - 8)) | ((UI32)NBBitstream_readU8Eql_(obj, 8) << (bitsRight - 16)) | ((UI32)NBBitstream_readU8Eql_(obj, 8) << (bitsRight - 24)) | NBBitstream_readU8Lwr_(obj, bitsRight - 24);
		}
	}
	return 0;
	//return ((UI32)NBBitstream_readU8(obj, bits % 24) << 24) | ((UI32)NBBitstream_readU8(obj, 8) << 16) | ((UI32)NBBitstream_readU8(obj, 8) << 8) | NBBitstream_readU8(obj, 8);
}

//U(v)

UI16 NBBitstream_readU16v(STNBBitstream* obj, const UI8 bits){
	NBASSERT(bits <= 16)
	if(bits <= 8){
		return NBBitstream_readU8(obj, bits);
	} else {
		return NBBitstream_readU16(obj, bits);
	}
	NBASSERT(FALSE)
	return 0;
}

UI32 NBBitstream_readU32v(STNBBitstream* obj, const UI8 bits){
	NBASSERT(bits <= 32)
	if(bits <= 8){
		return NBBitstream_readU8(obj, bits);
	} else if(bits <= 16){
		return NBBitstream_readU16(obj, bits);
	} else if(bits <= 24){
		return NBBitstream_readU24(obj, bits);
	} else {
		return NBBitstream_readU32(obj, bits);
	}
	NBASSERT(FALSE)
	return 0;
}

UI64 NBBitstream_readU64v(STNBBitstream* obj, const UI8 bits){
	NBASSERT(bits <= 64)
	if(bits <= 8){
		return NBBitstream_readU8(obj, bits);
	} else if(bits <= 16){
		return NBBitstream_readU16(obj, bits);
	} else if(bits <= 24){
		return NBBitstream_readU24(obj, bits);
	} else if(bits <= 32){
		return NBBitstream_readU32(obj, bits);
	} else if(bits <= 40){
		return ((UI64)NBBitstream_readU32(obj, bits - 8) << (bits - 8)) | NBBitstream_readU8(obj, bits - 32);
	} else if(bits <= 48){
		return ((UI64)NBBitstream_readU32(obj, bits - 16) << (bits - 16)) | NBBitstream_readU16(obj, bits - 32);
	} else if(bits <= 56){
		return ((UI64)NBBitstream_readU32(obj, bits - 24) << (bits - 24)) | NBBitstream_readU24(obj, bits - 32);
	} else {
		return ((UI64)NBBitstream_readU32(obj, bits - 32) << (bits - 32)) | NBBitstream_readU32(obj, bits - 32);
	}
	NBASSERT(FALSE)
	return 0;
}


//Golomb

UI64 NBBitstream_readGolombUE64(STNBBitstream* obj){
	/*
	 This process shall be equivalent to the following:
	 leadingZeroBits = -1;
	 for( b = 0; !b; leadingZeroBits++ )
		b = read_bits( 1 )
	 The variable codeNum is then assigned as follows:
		codeNum = 2^(leadingZeroBits) – 1 + read_bits(leadingZeroBits) 
	*/
	if(NBBitstream_readU8(obj, 1) == 1){
		//Golomb 0, is just 1-bit.
		return 0;
	} else {
		//First zero confirmed
		UI64 val = 1;
		UI8 bit = 0; SI32 leadingZeroBits = 0, leadingZeroesPlusOne = 0;
		//Read leading zeroes
		while(!bit && !obj->stopFlags){
			bit = NBBitstream_readU8(obj, 1);
			leadingZeroBits++;
		}
		//Set "2^(leadingZeroBits) – 1"
		while((leadingZeroesPlusOne++) < leadingZeroBits){
			val *= 2;
		}
		val--;
		//Set "+ read_bits(leadingZeroBits)"
		NBASSERT(leadingZeroBits > 0)
		if(leadingZeroBits <= 8){
			val += NBBitstream_readU8(obj, leadingZeroBits);
		} else if(leadingZeroBits <= 16){
			val += NBBitstream_readU16(obj, leadingZeroBits);
		} else if(leadingZeroBits <= 24){
			val += NBBitstream_readU24(obj, leadingZeroBits);
		} else if(leadingZeroBits <= 32){
			val += NBBitstream_readU32(obj, leadingZeroBits);
		} else {
			//Result will overflow 64 bits
			NBASSERT(FALSE)
		}
		return val;
	}
	return 0;
}

SI64 NBBitstream_readGolombSE64(STNBBitstream* obj){
	const UI64 codeNum = NBBitstream_readGolombUE64(obj);
	return (SI64)(codeNum % 2 ? 1LL : -1LL) * (SI64)((codeNum / 2) + (codeNum % 2));
}

//

UI32 NBBitstream_getCurGlobalPos(const STNBBitstream* obj){
	return obj->src.global.iPos;
}

UI8 NBBitstream_getBitsRemainCount(STNBBitstream* obj){
	return obj->bitsRemain;
}

UI8 NBBitstream_getCurByte(STNBBitstream* obj){
	return (obj->src.cur < obj->src.afterEnd ? *obj->src.cur : 0);
}

//

SI32 NBBitstream_moveToEnd(STNBBitstream* obj){
	SI32 r = 0, pCount;
	//consume all remainig ptrs
	while(obj->src.cur < obj->src.afterEnd || obj->moveToNextPtrMthod != NULL){
		pCount = (SI32)(obj->src.afterEnd - obj->src.cur);
		r						+= pCount;
		obj->src.global.iPos	+= pCount;
		obj->src.cur			= obj->src.afterEnd;
		if(obj->moveToNextPtrMthod != NULL){
			(*obj->moveToNextPtrMthod)(obj);
			NBASSERT(obj->bitsRemain == 8)
		}
	}
	//allways set
	obj->bitsRemain			= 0;
	return r;
}

SI32 NBBitstream_moveToStart(STNBBitstream* obj){
	SI32 r = 0, pCount;
	//consume all remainig ptrs
	while(obj->src.start < obj->src.cur || obj->moveToPrevPtrMthod != NULL){
		NBASSERT(obj->src.start <= obj->src.cur) //must be positive
		pCount			= (SI32)(obj->src.cur - obj->src.start); 
		r				+= pCount; //PRINTF_INFO("NBBitstream, rewinded %d bytes.\n", pCount);
		NBASSERT(obj->src.global.iPos >= pCount)
		obj->src.global.iPos -= pCount;
		obj->src.cur	= obj->src.start;
		//prev chunk
		if(obj->moveToPrevPtrMthod != NULL){
			(*obj->moveToPrevPtrMthod)(obj);
			NBASSERT(obj->bitsRemain == 8)
		}
	}
	//allways set
	obj->bitsRemain = 8;
	return r;
}

BOOL NBBitstream_moveToNextByte(STNBBitstream* obj){
	if(obj->bitsRemain != 0){
		NBBitstream_readU8Eql_(obj, obj->bitsRemain);
		return TRUE;
	}
	return FALSE;
}

BOOL NBBitstream_moveToPrevByte(STNBBitstream* obj){
	//move to prev non-empty-chunk
	while(obj->src.start == obj->src.cur && obj->moveToPrevPtrMthod != NULL){
		(*obj->moveToPrevPtrMthod)(obj);
		if(obj->src.start < obj->src.afterEnd){
			return TRUE;
		}
	}
	//move to prev byte
	if(obj->src.start < obj->src.cur){
		obj->src.cur--;
		NBASSERT(obj->src.global.iPos > 0)
		obj->src.global.iPos--;
		obj->bitsRemain = 8;
		return TRUE;
	}
	//default
	return FALSE;
}


#ifdef NB_CONFIG_INCLUDE_ASSERTS
#include <stdlib.h>	//rand()
BOOL NBBitstream_dbgTest(const UI32 testsTotal){
	BOOL r = TRUE;
	//init base payload
	UI8 fullPay[256]; UI32 iTest = 0;
	{
		UI32 i; for(i = 0; i < sizeof(fullPay); i++){
			fullPay[i] = (UI8)i;
		}
	}
	//run multiple random tests
	PRINTF_INFO("NBBitstream, running %u tests.\n", testsTotal);
	while(r && iTest < testsTotal){
		STNBDataPtr* chunks = NULL; SI32 chunksSz = 0;
		STNBString pay, desc;
		NBString_init(&pay);
		NBString_init(&desc);
		{
			const UI8 payType = (rand() % 2);
			STNBBitstream bs;
			NBBitstream_init(&bs);
			//init payload
			if(payType == 0){
				//payload type
				const UI8 prefixSz = (rand() % (sizeof(payType) + 1));
				NBString_concatBytes(&pay, &fullPay[prefixSz], sizeof(fullPay) - prefixSz);
				NBBitstream_setAsPayload(&bs, fullPay, prefixSz, pay.str, pay.length);
				NBASSERT((prefixSz + pay.length) == sizeof(fullPay))
				//desc
				if(prefixSz == 0){
					if(desc.length != 0) NBString_concatByte(&desc, ' ');
					NBString_concat(&desc, "no-prefix");
				} else {
					if(desc.length != 0) NBString_concatByte(&desc, ' ');
					NBString_concatUI32(&desc, prefixSz);
					NBString_concat(&desc, "-bytes-prefix");
				}
				if(pay.length == 0){
					if(desc.length != 0) NBString_concatByte(&desc, ' ');
					NBString_concat(&desc, "no-payload");
				} else {
					if(desc.length != 0) NBString_concatByte(&desc, ' ');
					NBString_concatUI32(&desc, pay.length);
					NBString_concat(&desc, "-bytes-payload");
				}
			} else {
				//chunks type
				chunksSz = (rand() % 4);
				if(chunksSz == 0){
					//prefix only
					NBBitstream_setAsChunks(&bs, fullPay, sizeof(fullPay), NULL, 0);
					//desc
					{
						if(desc.length != 0) NBString_concatByte(&desc, ' ');
						NBString_concatUI32(&desc, sizeof(fullPay));
						NBString_concat(&desc, "-bytes-prefix");
					}
					{
						if(desc.length != 0) NBString_concatByte(&desc, ' ');
						NBString_concat(&desc, "no-chunks");
					}
				} else {
					const UI8 prefixSz = (rand() % (sizeof(payType) + 1));
					UI32 i, iPos = prefixSz, rndSz;
					chunks = NBMemory_allocTypes(STNBDataPtr, chunksSz);
					if(prefixSz == 0){
						if(desc.length != 0) NBString_concatByte(&desc, ' ');
						NBString_concat(&desc, "no-prefix");
					} else {
						if(desc.length != 0) NBString_concatByte(&desc, ' ');
						NBString_concatUI32(&desc, prefixSz);
						NBString_concat(&desc, "-bytes-prefix");
					}
					for(i = 0; i < chunksSz; i++){
						NBDataPtr_init(&chunks[i]);
						if((i + 1) == chunksSz || iPos == sizeof(fullPay)){
							//last chunk (take all remainig bytes)
							rndSz = sizeof(fullPay) - iPos;
							NBDataPtr_setAsExternalBytes(&chunks[i], &fullPay[iPos], rndSz);
						} else {
							rndSz = (rand() % (sizeof(fullPay) - iPos + 1));
							NBDataPtr_setAsExternalBytes(&chunks[i], &fullPay[iPos], rndSz);
						}
						{
							if(desc.length != 0) NBString_concatByte(&desc, ' ');
							NBString_concat(&desc, "chk-");
							NBString_concatUI32(&desc, i);
							NBString_concat(&desc, "(");
							NBString_concatUI32(&desc, iPos);
							NBString_concat(&desc, ", +");
							NBString_concatUI32(&desc, rndSz);
							NBString_concat(&desc, ")");
						}
						iPos += rndSz;
					}
					NBASSERT(iPos == sizeof(fullPay))
					NBBitstream_setAsChunks(&bs, fullPay, prefixSz, chunks, chunksSz);
				}
			}
			//test
			{
				UI32 iCurByte = 0, iCurBit = 0;
				const UI32 actionsTotal = 1 + (rand() % 1024); UI32 iAction = 0, iCurAction, repeatTotal, iRepeat;
				PRINTF_INFO("NBBitstream, test #%d / %u, starting %d actions to: %s.\n", (iTest + 1), testsTotal, actionsTotal, desc.str);
				while(r && iAction < actionsTotal){
					iCurAction = rand() % 100;
					if(iCurAction < 5){
						//move to start
						NBBitstream_moveToStart(&bs);
						//validate
						if(NBBitstream_getCurGlobalPos(&bs) != 0){
							PRINTF_ERROR("NBBitstream, test #%d, action #%d / %d, 'rewindAll' FAIL (globalPos).\n", (iTest + 1), (iAction + 1), actionsTotal);
							NBASSERT(FALSE)
							r = FALSE;
						} else if(NBBitstream_getBitsRemainCount(&bs) != 8){
							PRINTF_ERROR("NBBitstream, test #%d, action #%d / %d, 'rewindAll' FAIL (bitsRemain).\n", (iTest + 1), (iAction + 1), actionsTotal);
							NBASSERT(FALSE)
							r = FALSE;
						} else if(NBBitstream_getCurByte(&bs) != fullPay[0]){
							PRINTF_ERROR("NBBitstream, test #%d, action #%d / %d, 'rewindAll' FAIL (curByte).\n", (iTest + 1), (iAction + 1), actionsTotal);
							NBASSERT(FALSE)
							r = FALSE;
						} else {
							iCurByte = iCurBit = 0;
							//PRINTF_INFO("NBBitstream, test #%d, action #%d / %d, 'rewindAll' OK.\n", (iTest + 1), (iAction + 1), actionsTotal);
						}
					} else if(iCurAction < 10){
						//move to end
						NBBitstream_moveToEnd(&bs);
						//validate
						if(NBBitstream_getCurGlobalPos(&bs) != sizeof(fullPay)){
							PRINTF_ERROR("NBBitstream, test #%d, action #%d / %d, 'skipAll' FAIL (globalPos).\n", (iTest + 1), (iAction + 1), actionsTotal);
							NBASSERT(FALSE)
							r = FALSE;
						} else if(NBBitstream_getBitsRemainCount(&bs) != 0){
							PRINTF_ERROR("NBBitstream, test #%d, action #%d / %d, 'skipAll' FAIL (bitsRemain).\n", (iTest + 1), (iAction + 1), actionsTotal);
							NBASSERT(FALSE)
							r = FALSE;
						} else {
							iCurByte = sizeof(fullPay); iCurBit = 8;
							//PRINTF_INFO("NBBitstream, test #%d, action #%d / %d, 'skipAll' OK.\n", (iTest + 1), (iAction + 1), actionsTotal);
						}
					} else if(iCurAction < 55){
						UI8 bits = 0, val = 0, valExpt = 0, pathDone = 0;
						//move forward multiple times
						repeatTotal = 1 + (rand() % 32);
						iRepeat = 0;
						while(r && iRepeat < repeatTotal){
							bits = 1 + (rand() % 8);
							if(NBBitstream_getCurGlobalPos(&bs) != iCurByte){
								PRINTF_ERROR("NBBitstream, test #%d, action #%d / %d, 'readBits(%d)' FAIL (globalPos).\n", (iTest + 1), (iAction + 1), actionsTotal, bits);
								NBASSERT(FALSE)
								r = FALSE;
							} else if(NBBitstream_getBitsRemainCount(&bs) != (8 - iCurBit)){
								PRINTF_ERROR("NBBitstream, test #%d, action #%d / %d, 'readBits(%d)' FAIL (bitsRemain).\n", (iTest + 1), (iAction + 1), actionsTotal, bits);
								NBASSERT(FALSE)
								r = FALSE;
							}
							val = NBBitstream_readU8(&bs, bits);
							if(iCurByte + ((iCurBit + bits) > 8 ? 1 : 0) >= sizeof(fullPay)){
								//not-enoght-bits (should fail)
								pathDone	= 0;
								iCurByte	= sizeof(fullPay);
								iCurBit		= 8;
								valExpt		= 0;
							} else if((iCurBit + bits) <= 8){
								//same byte
								pathDone	= 1;
								valExpt		= (fullPay[iCurByte] >> (8 - iCurBit - bits)) & NBBitstreamBitsMasks_[bits];
								iCurByte	+= (iCurBit + bits) / 8;
								iCurBit		= (iCurBit + bits) % 8;
								if(iCurByte == sizeof(fullPay)) iCurBit = 8;
							} else {
								//moving to next byte
								pathDone	= 2;
								valExpt		= ((fullPay[iCurByte] & NBBitstreamBitsMasks_[8 - iCurBit]) << ((iCurBit + bits) % 8)) | ((fullPay[iCurByte + 1] >> (8 - (iCurBit + bits) % 8)) & NBBitstreamBitsMasks_[iCurBit]);
								iCurByte	+= (iCurBit + bits) / 8;
								iCurBit		= (iCurBit + bits) % 8;
								if(iCurByte == sizeof(fullPay)) iCurBit = 8;
							}
							//result
							if(NBBitstream_getCurGlobalPos(&bs) != iCurByte){
								PRINTF_ERROR("NBBitstream, test #%d, action #%d / %d, 'readBits(%d)' FAIL (globalPos).\n", (iTest + 1), (iAction + 1), actionsTotal, bits);
								NBASSERT(FALSE)
								r = FALSE;
							} else if(NBBitstream_getBitsRemainCount(&bs) != (8 - iCurBit)){
								PRINTF_ERROR("NBBitstream, test #%d, action #%d / %d, 'readBits(%d)' FAIL (bitsRemain).\n", (iTest + 1), (iAction + 1), actionsTotal, bits);
								NBASSERT(FALSE)
								r = FALSE;
							} else if(val != valExpt){
								PRINTF_ERROR("NBBitstream, test #%d, action #%d / %d, 'readBits(%d)' FAIL (value).\n", (iTest + 1), (iAction + 1), actionsTotal, bits);
								NBASSERT(FALSE)
								r = FALSE;
							} else {
								//PRINTF_INFO("NBBitstream, test #%d, action #%d / %d, 'readBits(%d)' OK.\n", (iTest + 1), (iAction + 1), actionsTotal, bits);
							}
							//next
							iRepeat++;
						}
					} else {
						//move backwards multiple times
						repeatTotal = 1 + (rand() % 32);
						iRepeat = 0;
						while(r && iRepeat < repeatTotal){
							//
							if(iCurByte > 0){
								if(NBBitstream_moveToPrevByte(&bs)){
									iCurBit = 0;
									iCurByte--;
								} else {
									PRINTF_ERROR("NBBitstream, test #%d, action #%d / %d, 'moveToPrevByte' FAIL (call failed).\n", (iTest + 1), (iAction + 1), actionsTotal);
									NBASSERT(FALSE);
									r = FALSE;
								}
							} else {
								if(NBBitstream_moveToPrevByte(&bs)){
									PRINTF_ERROR("NBBitstream, test #%d, action #%d / %d, 'moveToPrevByte' FAIL (call should not succed).\n", (iTest + 1), (iAction + 1), actionsTotal);
									NBASSERT(FALSE);
									r = FALSE;
									iCurBit = 0;
									iCurByte--;
								}
							} 
							if(iCurByte < 0){
								PRINTF_ERROR("NBBitstream, test #%d, action #%d / %d, 'moveToPrevByte' FAIL (neg-globalPos).\n", (iTest + 1), (iAction + 1), actionsTotal);
								NBASSERT(FALSE);
								r = FALSE;
							} else if(NBBitstream_getCurGlobalPos(&bs) != iCurByte){
								PRINTF_ERROR("NBBitstream, test #%d, action #%d / %d, 'moveToPrevByte' FAIL (globalPos).\n", (iTest + 1), (iAction + 1), actionsTotal);
								NBASSERT(FALSE);
								r = FALSE;
							} else if(NBBitstream_getBitsRemainCount(&bs) != (8 - iCurBit)){
								PRINTF_ERROR("NBBitstream, test #%d, action #%d / %d, 'moveToPrevByte' FAIL (bitsRemain).\n", (iTest + 1), (iAction + 1), actionsTotal);
								NBASSERT(FALSE);
								r = FALSE;
							} else if(NBBitstream_getCurByte(&bs) != fullPay[iCurByte]){
								PRINTF_ERROR("NBBitstream, test #%d, action #%d / %d, 'moveToPrevByte' FAIL (value).\n", (iTest + 1), (iAction + 1), actionsTotal);
								NBASSERT(FALSE);
								r = FALSE;
							} else {
								//PRINTF_INFO("NBBitstream, test #%d, action #%d / %d, 'moveToPrevByte' OK.\n", (iTest + 1), (iAction + 1), actionsTotal);
							}
							//next
							iRepeat++;
						}
					}
					//next
					iAction++;
				}
			}
			NBBitstream_release(&bs);
		}
		//release
		if(chunks != NULL){
			UI32 i; for(i = 0; i < chunksSz; i++){
				NBDataPtr_release(&chunks[i]);
			}
			NBMemory_free(chunks);
			chunks = NULL;
			chunksSz = 0;
		}
		NBString_release(&pay);
		NBString_release(&desc);
		//next
		iTest++;
	} //while
	return r;
}
#endif
