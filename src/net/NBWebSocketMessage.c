
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBWebSocketMessage.h"
//
#include "nb/core/NBStruct.h"

//-----------------
//-- WebSocket - RFC6455
//-- https://datatracker.ietf.org/doc/html/rfc6455
//-----------------

void NBWebSocketMessage_init(STNBWebSocketMessage* obj){
	NBMemory_setZeroSt(*obj, STNBWebSocketMessage);
}

void NBWebSocketMessage_release(STNBWebSocketMessage* obj){
	if(obj->frames.arr != NULL){
		UI32 i; for(i = 0; i < obj->frames.size; i++){
			STNBWebSocketFrame* f = &obj->frames.arr[i];
			NBWebSocketFrame_release(f);
		}
		NBMemory_free(obj->frames.arr);
		obj->frames.arr = NULL;
		obj->frames.size = 0;
	}
}

//

void NBWebSocketMessage_feedStart(STNBWebSocketMessage* obj){
	obj->fedBytesCount = 0;
	if(obj->frames.arr != NULL){
		UI32 i; for(i = 0; i < obj->frames.size; i++){
			STNBWebSocketFrame* f = &obj->frames.arr[i];
			NBWebSocketFrame_release(f);
		}
		NBMemory_free(obj->frames.arr);
		obj->frames.arr = NULL;
		obj->frames.size = 0;
	}
}

UI32 NBWebSocketMessage_feedBytes(STNBWebSocketMessage* obj, const void* pData, const UI32 dataSz){
	const BYTE* data = (const BYTE*)pData;
	const BYTE* dataStart = data;
	const BYTE* dataAfterEnd = dataStart + dataSz;
	//first frame
	if(obj->frames.arr == NULL || obj->frames.size <= 0){
		STNBWebSocketFrame* f = NULL;
		if(obj->frames.arr == NULL){
			obj->frames.arr = f = NBMemory_allocType(STNBWebSocketFrame);
		} else {
			f = &obj->frames.arr[0];
		}
		NBWebSocketFrame_init(f);
		NBWebSocketFrame_feedStart(f);
		obj->frames.size	= 1;
	}
	//feed frames
	{
		STNBWebSocketFrame* lastF = NULL;
		UI32 fed = 0;
		while(data < dataAfterEnd){
			lastF = &obj->frames.arr[obj->frames.size - 1];
			if(!NBWebSocketFrame_feedIsCompleted(lastF)){
				//feed to frame
				fed = NBWebSocketFrame_feedBytes(lastF, data, (UI32)(dataAfterEnd - data)); 
				if(fed == 0){
					break;
				} else {
					data += fed;
				}
			} else if(NBWebSocketFrame_isFinalFrame(lastF)){
				//end-of-message
				break;
			} else {
				//start new frame
				STNBWebSocketFrame* f = NULL;
				STNBWebSocketFrame* arrN = NBMemory_allocTypes(STNBWebSocketFrame, obj->frames.size + 1);
				if(obj->frames.arr != NULL){
					if(obj->frames.size > 0){
						NBMemory_copy(arrN, obj->frames.arr, sizeof(obj->frames.arr[0]) * obj->frames.size);
					}
					NBMemory_free(obj->frames.arr);
				}
				obj->frames.arr = arrN;
				f = &obj->frames.arr[obj->frames.size++];
				NBWebSocketFrame_init(f);
				NBWebSocketFrame_feedStart(f);
				fed = NBWebSocketFrame_feedBytes(f, data, (UI32)(dataAfterEnd - data)); 
				if(fed == 0){
					break;
				} else {
					data += fed;
				}
			}
		}
	}
	obj->fedBytesCount += (UI32)(data - dataStart); 
	return (UI32)(data - dataStart);
}

BOOL NBWebSocketMessage_feedIsCompleted(const STNBWebSocketMessage* obj){
	return (obj->frames.arr != NULL && obj->frames.size > 0 && NBWebSocketFrame_feedIsCompleted(&obj->frames.arr[obj->frames.size - 1]) && NBWebSocketFrame_isFinalFrame(&obj->frames.arr[obj->frames.size - 1]));
}

#ifdef NB_CONFIG_INCLUDE_ASSERTS
/*void NBWebSocketMessage_concatFedStrHex(const STNBWebSocketMessage* obj, const UI32 bytesPerGrp, const UI32 bytesPerLine, STNBString* dst){
	if(obj->frames.arr != NULL){
		UI32 i; for(i = 0; i < obj->frames.size; i++){
			const STNBWebSocketFrame* f = &obj->frames.arr[i];
			if(i != 0) NBString_concatByte(dst, '\n');
			NBWebSocketFrame_concatFedStrHex(f, bytesPerGrp, bytesPerLine, dst);
		}
	}
}*/
#endif

//

BOOL NBWebSocketMessage_concat(const STNBWebSocketMessage* obj, STNBString* dst){
	BOOL r = TRUE;
	if(obj->frames.arr != NULL){
		UI32 i; for(i = 0; i < obj->frames.size; i++){
			const STNBWebSocketFrame* f = &obj->frames.arr[i];
			if(!NBWebSocketFrame_concat(f, dst)){
				r = FALSE;
				break;
			}
		}
	}
	return r;
}

void NBWebSocketMessage_concatUnmaskedData(const STNBWebSocketMessage* obj, STNBString* dst){
	if(obj->frames.arr != NULL){
		UI32 i; for(i = 0; i < obj->frames.size; i++){
			const STNBWebSocketFrame* f = &obj->frames.arr[i];
			NBWebSocketFrame_concatUnmaskedData(f, dst);
		}
	}
}

UI32 NBWebSocketMessage_getFedBytesCount(const STNBWebSocketMessage* obj){
	return obj->fedBytesCount; 
}
