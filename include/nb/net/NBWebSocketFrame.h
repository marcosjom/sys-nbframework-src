#ifndef NB_WEBSOCKET_FRAME_H
#define NB_WEBSOCKET_FRAME_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBString.h"

//

#ifdef __cplusplus
extern "C" {
#endif
	
	//-----------------
	//-- WebSocket - RFC6455
	//-- https://datatracker.ietf.org/doc/html/rfc6455
	//-----------------

	//ENNBWebSocketFrameOpCode

	typedef enum ENNBWebSocketFrameOpCode_ {
		ENNBWebSocketFrameOpCode_Continuation	= 0x0,	//continuation frame
		ENNBWebSocketFrameOpCode_Text			= 0x1, //text frame
		ENNBWebSocketFrameOpCode_Binary			= 0x2, //binary frame
		ENNBWebSocketFrameOpCode_Reserved3		= 0x3, //reserved for further non-control frames
		ENNBWebSocketFrameOpCode_Reserved4		= 0x4, //reserved for further non-control frames
		ENNBWebSocketFrameOpCode_Reserved5		= 0x5, //reserved for further non-control frames
		ENNBWebSocketFrameOpCode_Reserved6		= 0x6, //reserved for further non-control frames
		ENNBWebSocketFrameOpCode_Reserved7		= 0x7, //reserved for further non-control frames
		ENNBWebSocketFrameOpCode_Close			= 0x8, //connection close
		ENNBWebSocketFrameOpCode_Ping			= 0x9, //ping
		ENNBWebSocketFrameOpCode_Pong			= 0xA, //pong
		ENNBWebSocketFrameOpCode_Reserved11		= 0xB, //reserved for further control frames
		ENNBWebSocketFrameOpCode_Reserved12		= 0xC, //reserved for further control frames
		ENNBWebSocketFrameOpCode_Reserved13		= 0xD, //reserved for further control frames
		ENNBWebSocketFrameOpCode_Reserved14		= 0xE, //reserved for further control frames
		ENNBWebSocketFrameOpCode_Reserved15		= 0xF, //reserved for further control frames
		//
		ENNBWebSocketFrameOpCode_Count
	} ENNBWebSocketFrameOpCode;

	//NBWebSocketFrameLoadState

	struct STNBWebSocketFrame_;
	struct STNBWebSocketFrameLoadState_;

	typedef UI32 (*NBWebSocketFrameFeedMethod)(struct STNBWebSocketFrameLoadState_* obj, const BYTE* data, const BYTE* dataAfterEnd, struct STNBWebSocketFrame_* dst);

	typedef struct STNBWebSocketFrameLoadState_ {
		BOOL		errFnd;					//error in parsing
		NBWebSocketFrameFeedMethod method;	//current feed method
		//payLen
		struct {
			UI8		bytesExpect;			//bytes expected for payloadLen
			UI8		bytesFed;				//bytes fed for payloadLen
		} payLen;
		//maskKey
		struct {
			UI8		bytesFed;				//bytes fed for maskKey
		} maskKey;
		//payload
		struct {
			UI64	bytesFed;				//bytes fed for payload
		} payload;
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		/*struct {
			STNBString bytesFedStr;
		} dbg;*/
#		endif
	} STNBWebSocketFrameLoadState;

	void NBWebSocketFrameLoadState_init(STNBWebSocketFrameLoadState* obj);
	void NBWebSocketFrameLoadState_release(STNBWebSocketFrameLoadState* obj);
	void NBWebSocketFrameLoadState_reset(STNBWebSocketFrameLoadState* obj);

	//NBWebSocketFrame

	typedef struct STNBWebSocketMask_ {
		union {
			UI8		c[4];
			UI32	u32;
		};
	} STNBWebSocketMask;
	
	typedef struct STNBWebSocketFrame_ {
		BOOL		isFin;				//FIN bit
		BOOL		rsv1;				//Reserved bit
		BOOL		rsv2;				//Reserved bit
		BOOL		rsv3;				//Reserved bit
		ENNBWebSocketFrameOpCode opCode; //4-bits
		//mask
		struct {
			BOOL				isPresent;	//mask bit
			STNBWebSocketMask	value;		//32 bits (if present)
		} mask;
		//payload
		struct {
			UI64	length;				//payload length
			void*	data;				//payload
		} pay;
		//load state
		STNBWebSocketFrameLoadState* loadState;	//not NULL if loading
	} STNBWebSocketFrame;

	void NBWebSocketFrame_init(STNBWebSocketFrame* obj);
	void NBWebSocketFrame_release(STNBWebSocketFrame* obj);
	
	//
	BOOL NBWebSocketFrame_isFinalFrame(const STNBWebSocketFrame* obj);

	//
	void NBWebSocketFrame_feedStart(STNBWebSocketFrame* obj);
	UI32 NBWebSocketFrame_feedBytes(STNBWebSocketFrame* obj, const void* data, const UI32 dataSz);
	BOOL NBWebSocketFrame_feedIsCompleted(const STNBWebSocketFrame* obj);
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	//void NBWebSocketFrame_concatFedStrHex(const STNBWebSocketFrame* obj, const UI32 bytesPerGrp, const UI32 bytesPerLine, STNBString* dst);
#	endif

	//
	BOOL NBWebSocketFrame_concat(const STNBWebSocketFrame* obj, STNBString* dst);
	void NBWebSocketFrame_concatUnmaskedData(const STNBWebSocketFrame* obj, STNBString* dst);
	BOOL NBWebSocketFrame_concatWithData(const ENNBWebSocketFrameOpCode opCode, const UI32 maskKey, const void* data, const UI64 dataSz, const BOOL isMsgFirstFrame, const BOOL isMsgLastFrame, STNBString* dst);
	UI32 NBWebSocketFrame_writeHeader(const ENNBWebSocketFrameOpCode opCode, const UI32 maskKey, const UI64 dataSz, const BOOL isMsgFirstFrame, const BOOL isMsgLastFrame, void* dst14); //basic header is 14-bytes max

#ifdef __cplusplus
} //extern "C"
#endif

#endif
