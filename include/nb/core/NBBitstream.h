//
//  NBBitstream.h
//  nbframework
//
//  Created by Marcos Ortega on 9/21/20.
//

#ifndef NBBitstream_h
#define NBBitstream_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBDataPtr.h"

struct STNBBitstream_;

//NBBitstreamMthds

typedef void (*NBBitstreamMovePtrMethod)(struct STNBBitstream_* bs);

//ENNBBitstreamSrcType

typedef enum ENNBBitstreamSrcType_ {
	ENNBBitstreamSrcType_Payload = 0,	//one continuos payload
	ENNBBitstreamSrcType_Chuncks,		//one or more chunks
	//Count
	ENNBBitstreamSrcType_Count
} ENNBBitstreamSrcType;

//NBBitstreamSrc

typedef struct STNBBitstreamSrc_ {
	ENNBBitstreamSrcType		type;
	const BYTE*					start;		//start of current buffer
	const BYTE*					cur;		//cur pos in current buffer
	const BYTE*					afterEnd;	//pointer after last byte in current buffer
	//global
	struct {
		SI32					iPos;		//current position in global stream (all prefix and chunks as one continous payload)
	} global;
	//prefix
	struct {
		const BYTE*				start;
		const BYTE*				afterEnd;
	} prefix;
	//remain
	union {
		//payload (if type == ENNBBitstreamSrcType_Payload)
		struct {
			const BYTE*			start;
			const BYTE*			afterEnd;
		} payload;
		//chuncks (if type == ENNBBitstreamSrcType_Chuncks)
		struct {
			const STNBDataPtr*	start;
			const STNBDataPtr*	next;
			const STNBDataPtr*	afterEnd;
		} chuncks;
	} remain;
} STNBBitstreamSrc;

//NBBitstream

#define NB_BITSTREAM_STOP_OVERFLOWED	(0x1 << 0)	//attempted to read over the limits
#define NB_BITSTREAM_STOP_OUT_OF_SYNC	(0x1 << 1)	//read-logic was stopped by consumer, making the bitstream unable to read ahead

typedef struct STNBBitstream_ {
	UI8							bitsRemain;			//bits remain at current 'src' byte
	UI8							zeroesSeqCount;		//Each '0x00 0x00 0x03' sequence implies to ignore the '0x03' (emulation_prevention_three_byte)
	UI8							stopFlags;			//Any reason to stop NB_BITSTREAM_STOP_*
	STNBBitstreamSrc			src;				//src
	NBBitstreamMovePtrMethod moveToNextPtrMthod;	//normal operation, read chunks forward
	NBBitstreamMovePtrMethod moveToPrevPtrMthod; //read chunks backwards
} STNBBitstream;

void NBBitstream_init(STNBBitstream* obj);
void NBBitstream_release(STNBBitstream* obj);
void NBBitstream_setAsOther(STNBBitstream* obj, const STNBBitstream* other);
void NBBitstream_setAsPayload(STNBBitstream* obj, const void* pPrefix, const UI32 prefixSz, const void* pay, const UI32 paySz);
void NBBitstream_setAsChunks(STNBBitstream* obj, const void* pPrefix, const UI32 prefixSz, const STNBDataPtr* chunks, const SI32 chunksSz);

//
UI8 NBBitstream_readU8(STNBBitstream* obj, UI8 bits);
UI16 NBBitstream_readU16(STNBBitstream* obj, const UI8 bits);
UI32 NBBitstream_readU24(STNBBitstream* obj, const UI8 bits);
UI32 NBBitstream_readU32(STNBBitstream* obj, const UI8 bits);
UI16 NBBitstream_readU16v(STNBBitstream* obj, const UI8 bits);
UI32 NBBitstream_readU32v(STNBBitstream* obj, const UI8 bits);
UI64 NBBitstream_readU64v(STNBBitstream* obj, const UI8 bits);
UI64 NBBitstream_readGolombUE64(STNBBitstream* obj);
SI64 NBBitstream_readGolombSE64(STNBBitstream* obj);
//
UI32 NBBitstream_getCurGlobalPos(const STNBBitstream* obj);
UI8  NBBitstream_getBitsRemainCount(STNBBitstream* obj);
UI8  NBBitstream_getCurByte(STNBBitstream* obj);
//
SI32 NBBitstream_moveToStart(STNBBitstream* obj);
SI32 NBBitstream_moveToEnd(STNBBitstream* obj);
BOOL NBBitstream_moveToNextByte(STNBBitstream* obj);
BOOL NBBitstream_moveToPrevByte(STNBBitstream* obj);

#ifdef NB_CONFIG_INCLUDE_ASSERTS
BOOL NBBitstream_dbgTest(const UI32 testsTotal);
#endif

#endif /* NBBitstream_h */
