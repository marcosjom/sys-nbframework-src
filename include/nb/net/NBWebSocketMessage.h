#ifndef NB_WEBSOCKET_MESSAGE_H
#define NB_WEBSOCKET_MESSAGE_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBStructMap.h"

#include "nb/net/NBWebSocketFrame.h"
//

#ifdef __cplusplus
extern "C" {
#endif
	
	//NBWebSocketMessage

	typedef struct STNBWebSocketMessage_ {
		UI32					fedBytesCount;
		//frames
		struct {
			STNBWebSocketFrame* arr;
			UI32				size;
		} frames;
	} STNBWebSocketMessage;

	void NBWebSocketMessage_init(STNBWebSocketMessage* obj);
	void NBWebSocketMessage_release(STNBWebSocketMessage* obj);

	//
	void NBWebSocketMessage_feedStart(STNBWebSocketMessage* obj);
	UI32 NBWebSocketMessage_feedBytes(STNBWebSocketMessage* obj, const void* data, const UI32 dataSz);
	BOOL NBWebSocketMessage_feedIsCompleted(const STNBWebSocketMessage* obj);
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	//void NBWebSocketMessage_concatFedStrHex(const STNBWebSocketMessage* obj, const UI32 bytesPerGrp, const UI32 bytesPerLine, STNBString* dst);
#	endif

	BOOL NBWebSocketMessage_concat(const STNBWebSocketMessage* obj, STNBString* dst);
	void NBWebSocketMessage_concatUnmaskedData(const STNBWebSocketMessage* obj, STNBString* dst);

	UI32 NBWebSocketMessage_getFedBytesCount(const STNBWebSocketMessage* obj);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
