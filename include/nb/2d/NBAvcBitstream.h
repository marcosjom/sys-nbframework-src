//
//  NBAvcBitstream.h
//  nbframework
//
//  Created by Marcos Ortega on 9/21/20.
//

#ifndef NBAvcBitstream_h
#define NBAvcBitstream_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBDataPtr.h"

struct STNBAvcBitstream_;

//NBAvcBitstreamMthds

typedef void (*NBAvcBitstreamMovePtrMethod)(struct STNBAvcBitstream_* bs);

//ENNBAvcBitstreamSrcType

typedef enum ENNBAvcBitstreamSrcType_ {
	ENNBAvcBitstreamSrcType_Payload = 0,	//one continuos payload
	ENNBAvcBitstreamSrcType_Chuncks,		//one or more chunks
	//Count
	ENNBAvcBitstreamSrcType_Count
} ENNBAvcBitstreamSrcType;

//NBAvcBitstreamSrc

typedef struct STNBAvcBitstreamSrc_ {
	ENNBAvcBitstreamSrcType		type;
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
		//payload (if type == ENNBAvcBitstreamSrcType_Payload)
		struct {
			const BYTE*			start;
			const BYTE*			afterEnd;
		} payload;
		//chuncks (if type == ENNBAvcBitstreamSrcType_Chuncks)
		struct {
			const STNBDataPtr*	start;
			const STNBDataPtr*	next;
			const STNBDataPtr*	afterEnd;
		} chuncks;
	} remain;
} STNBAvcBitstreamSrc;

//NBAvcBitstream

#define NB_AVC_BITSTREAM_STOP_OVERFLOWED_READ	(0x1 << 0)	//attempted to read over the data limits
#define NB_AVC_BITSTREAM_STOP_OVERFLOWED_VALUE  (0x1 << 1)  //attempted a value that does not fits its type
#define NB_AVC_BITSTREAM_STOP_VALUE_LOGIC_ERROR (0x1 << 2)  //value is invalid in the logic defined in specifications

typedef struct STNBAvcBitstream_ {
	UI8							bitsRemain;			//bits remain at current 'src' byte
	UI8							zeroesSeqCount;		//Each '0x00 0x00 0x03' sequence implies to ignore the '0x03' (emulation_prevention_three_byte)
	UI8							stopFlags;			//Any reason to stop NB_AVC_BITSTREAM_STOP_*
	STNBAvcBitstreamSrc			src;				//src
	NBAvcBitstreamMovePtrMethod moveToNextPtrMthod;	//normal operation, read chunks forward
	NBAvcBitstreamMovePtrMethod moveToPrevPtrMthod; //read chunks backwards
} STNBAvcBitstream;

void NBAvcBitstream_init(STNBAvcBitstream* obj);
void NBAvcBitstream_release(STNBAvcBitstream* obj);
void NBAvcBitstream_setAsOther(STNBAvcBitstream* obj, const STNBAvcBitstream* other);
void NBAvcBitstream_setAsPayload(STNBAvcBitstream* obj, const void* pPrefix, const UI32 prefixSz, const void* pay, const UI32 paySz);
void NBAvcBitstream_setAsChunks(STNBAvcBitstream* obj, const void* pPrefix, const UI32 prefixSz, const STNBDataPtr* chunks, const SI32 chunksSz);

//
UI8 NBAvcBitstream_readU8(STNBAvcBitstream* obj, const UI8 bits);
UI16 NBAvcBitstream_readU16(STNBAvcBitstream* obj, const UI8 bits);
UI32 NBAvcBitstream_readU24(STNBAvcBitstream* obj, const UI8 bits);
UI32 NBAvcBitstream_readU32(STNBAvcBitstream* obj, const UI8 bits);
UI16 NBAvcBitstream_readU16v(STNBAvcBitstream* obj, const UI8 bits);
UI32 NBAvcBitstream_readU32v(STNBAvcBitstream* obj, const UI8 bits);
UI64 NBAvcBitstream_readU64v(STNBAvcBitstream* obj, const UI8 bits);
UI64 NBAvcBitstream_readGolombUE64(STNBAvcBitstream* obj);
SI64 NBAvcBitstream_readGolombSE64(STNBAvcBitstream* obj);
//
UI32 NBAvcBitstream_getCurGlobalPos(const STNBAvcBitstream* obj);
UI8  NBAvcBitstream_getBitsRemainCount(STNBAvcBitstream* obj);
UI8  NBAvcBitstream_getCurByte(STNBAvcBitstream* obj);
//
BOOL NBAvcBitstream_isAtStart(const STNBAvcBitstream* obj);
BOOL NBAvcBitstream_isAtEnd(const STNBAvcBitstream* obj);
SI32 NBAvcBitstream_moveToStart(STNBAvcBitstream* obj);
SI32 NBAvcBitstream_moveToEnd(STNBAvcBitstream* obj);
BOOL NBAvcBitstream_moveToNextByte(STNBAvcBitstream* obj);
BOOL NBAvcBitstream_moveToPrevByte(STNBAvcBitstream* obj);

#ifdef NB_CONFIG_INCLUDE_ASSERTS
BOOL NBAvcBitstream_dbgTest(const UI32 testsTotal);
#endif

#endif /* NBAvcBitstream_h */
