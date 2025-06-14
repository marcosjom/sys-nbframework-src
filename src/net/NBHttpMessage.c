
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBHttpMessage.h"
//

void NBHttpMessage_init(STNBHttpMessage* obj){
	obj->isCompleted	= FALSE;
	NBHttpHeader_init(&obj->header);
	NBHttpBody_init(&obj->body);
	obj->readBuffSz		= 4096;
	obj->storgBuffSz	= 4096;
	obj->readingMode	= 0; //0 = header, 1 = body, 2 = ended
	NBString_init(&obj->origReqMethod);
	//
	NBMemory_set(&obj->listener, 0, sizeof(obj->listener));
	obj->listenerParam	= NULL;
}

void NBHttpMessage_release(STNBHttpMessage* obj){
	obj->isCompleted	= FALSE;
	NBHttpHeader_release(&obj->header);
	NBHttpBody_release(&obj->body);
	obj->readBuffSz		= 0;
	obj->storgBuffSz	= 0;
	obj->readingMode	= 0; //0 = header, 1 = body, 2 = ended
	NBString_release(&obj->origReqMethod);
	//
	NBMemory_set(&obj->listener, 0, sizeof(obj->listener));
	obj->listenerParam	= NULL;
}

void NBHttpMessage_setListener(STNBHttpMessage* obj, const IHttpMessageListener* listnr, void* listnrParam){
	if(listnr != NULL){
		obj->listener	= *listnr;
	} else {
		NBMemory_set(&obj->listener, 0, sizeof(obj->listener));
	}
	obj->listenerParam	= listnrParam;
}

//Header callbacks

BOOL NBHttpMessage_consumeHeadStartLine_(const STNBHttpHeader* obj, void* lparam){
	BOOL r = TRUE;
	STNBHttpMessage* obj2 = (STNBHttpMessage*)lparam;
	if(obj2->listener.consumeHeadStartLine != NULL){
		r = (*obj2->listener.consumeHeadStartLine)(obj2, obj2->listenerParam);
	}
	return r;
}
BOOL NBHttpMessage_consumeHeadFieldLine_(const STNBHttpHeader* obj, const STNBHttpHeaderField* field, void* lparam){
	BOOL r = TRUE;
	STNBHttpMessage* obj2 = (STNBHttpMessage*)lparam;
	if(obj2->listener.consumeHeadFieldLine != NULL){
		r = (*obj2->listener.consumeHeadFieldLine)(obj2, field, obj2->listenerParam);
	}
	return r;
}
BOOL NBHttpMessage_consumeHeadEnd_(const STNBHttpHeader* obj, void* lparam){
	BOOL r = TRUE;
	STNBHttpMessage* obj2	= (STNBHttpMessage*)lparam;
	UI32 storgBuffSz		= obj2->storgBuffSz;
	if(obj2->listener.consumeHeadEnd != NULL){
		r = (*obj2->listener.consumeHeadEnd)(obj2, &storgBuffSz, obj2->listenerParam);
	}
	obj2->storgBuffSz		= storgBuffSz;
	//Validate
	if(r){
		NBASSERT(obj->statusLine == NULL || obj2->origReqMethod.length > 0)
		if(obj->statusLine != NULL && obj2->origReqMethod.length == 0){
			PRINTF_ERROR("A origRequestMethod is required to determine if expecting a http-msg-body.\n");
			r = FALSE; NBASSERT(FALSE)
		}
	}
	return r;
}

//Body callbacks

BOOL NBHttpMessage_consumeBodyData_(const STNBHttpBody* obj, const void* data, const UI64 dataSz, void* lparam){
	BOOL r = TRUE;
	STNBHttpMessage* obj2	= (STNBHttpMessage*)lparam;
	if(obj2->listener.consumeBodyData != NULL){
		r = (*obj2->listener.consumeBodyData)(obj2, data, dataSz, obj2->listenerParam);
	}
	return r;
}
BOOL NBHttpMessage_consumeBodyTrailerField_(const STNBHttpBody* obj, const STNBHttpBodyField* field, void* lparam){
	BOOL r = TRUE;
	STNBHttpMessage* obj2	= (STNBHttpMessage*)lparam;
	if(obj2->listener.consumeBodyTrailerField != NULL){
		r = (*obj2->listener.consumeBodyTrailerField)(obj2, field, obj2->listenerParam);
	}
	return r;
}
BOOL NBHttpMessage_consumeBodyEnd_(const STNBHttpBody* obj, void* lparam){
	BOOL r = TRUE;
	STNBHttpMessage* obj2	= (STNBHttpMessage*)lparam;
	if(obj2->listener.consumeBodyEnd != NULL){
		r = (*obj2->listener.consumeBodyEnd)(obj2, obj2->listenerParam);
	}
	return r;
}


//Feed

void NBHttpMessage_feedStart(STNBHttpMessage* obj, const UI32 readBuffSz, const UI32 storgBuffSz){
	NBHttpMessage_feedStartResponseOf(obj, readBuffSz, storgBuffSz, NULL);
}

void NBHttpMessage_feedStartResponseOf(STNBHttpMessage* obj, const UI32 readBuffSz, const UI32 storgBuffSz, const char* origReqMethod){
	NBHttpHeader_feedStart(&obj->header);
	obj->isCompleted	= FALSE;
	obj->fedTotal		= 0;
	obj->readBuffSz		= readBuffSz;
	obj->storgBuffSz	= storgBuffSz;
	obj->readingMode	= 0; //0 = header, 1 = body, 2 = ended
	NBString_empty(&obj->origReqMethod);
	if(origReqMethod != NULL){
		NBString_set(&obj->origReqMethod, origReqMethod);
	}
	//
	IHttpHeaderListener lstnr;
	NBMemory_set(&lstnr, 0, sizeof(lstnr));
	lstnr.consumeStartLine	= NBHttpMessage_consumeHeadStartLine_;
	lstnr.consumeFieldLine	= NBHttpMessage_consumeHeadFieldLine_;
	lstnr.consumeEnd		= NBHttpMessage_consumeHeadEnd_;
	NBHttpHeader_setListener(&obj->header, &lstnr, obj);
}

BOOL NBHttpMessage_feedByte(STNBHttpMessage* obj, const char c){
	BOOL r = FALSE;
	//Reading header
	if(obj->readingMode == 0){
		r = NBHttpHeader_feedByte(&obj->header, c);
		if(NBHttpHeader_feedIsComplete(&obj->header)){
			if(!NBHttpHeader_feedEnd(&obj->header)){
				NBASSERT(FALSE)
			} else {
				//PRINTF_INFO("NBHttpHeader ended.\n");
				NBASSERT(obj->header.isCompleted);
				IHttpBodyListener lstnr;
				NBMemory_set(&lstnr, 0, sizeof(lstnr));
				lstnr.consumeData			= NBHttpMessage_consumeBodyData_;
				lstnr.consumeTrailerField	= NBHttpMessage_consumeBodyTrailerField_;
				lstnr.consumeEnd			= NBHttpMessage_consumeBodyEnd_;
				NBHttpBody_setListener(&obj->body, &lstnr, obj);
				NBHttpBody_feedStartWithHeader(&obj->body, &obj->header, obj->readBuffSz, obj->storgBuffSz, obj->origReqMethod.str);
				obj->readingMode++;
			}
		}
	}
	//Reading body
	if(obj->readingMode == 1){
		if(!r){
			r = NBHttpBody_feedByte(&obj->body, c);
		}
		if(NBHttpBody_feedIsComplete(&obj->body)){
			if(!NBHttpBody_feedEnd(&obj->body)){
                //Note: this point can be reached if the 'consumeBodyEnd' user's callback returns FALSE to signal the end of the connection.
				//NBASSERT(FALSE)
			} else {
				//PRINTF_INFO("NBHttpBody %llu bytes ended.\n", obj->body.dataTotalSz);
				//End of body
				NBASSERT(obj->header.isCompleted && obj->body.isCompleted)
				obj->readingMode++;
				obj->isCompleted = TRUE;
			}
		}
	}
	if(r){
		obj->fedTotal++;
	}
	return r;
}

UI32 NBHttpMessage_feed(STNBHttpMessage* obj, const char* str){
	const char* c = str;
	//Reading header
	if(obj->readingMode == 0){
		c += NBHttpHeader_feed(&obj->header, c);
		if(NBHttpHeader_feedIsComplete(&obj->header)){
			if(!NBHttpHeader_feedEnd(&obj->header)){
				NBASSERT(FALSE)
			} else {
				//PRINTF_INFO("NBHttpHeader ended.\n");
				NBASSERT(obj->header.isCompleted);
				IHttpBodyListener lstnr;
				NBMemory_set(&lstnr, 0, sizeof(lstnr));
				lstnr.consumeData			= NBHttpMessage_consumeBodyData_;
				lstnr.consumeTrailerField	= NBHttpMessage_consumeBodyTrailerField_;
				lstnr.consumeEnd			= NBHttpMessage_consumeBodyEnd_;
				NBHttpBody_setListener(&obj->body, &lstnr, obj);
				NBHttpBody_feedStartWithHeader(&obj->body, &obj->header, obj->readBuffSz, obj->storgBuffSz, obj->origReqMethod.str);
				obj->readingMode++;
			}
		}
	}
	//Reading body
	if(obj->readingMode == 1){
		c += NBHttpBody_feed(&obj->body, c);
		if(NBHttpBody_feedIsComplete(&obj->body)){
			if(!NBHttpBody_feedEnd(&obj->body)){
                //Note: this point can be reached if the 'consumeBodyEnd' user's callback returns FALSE to signal the end of the connection.
				//NBASSERT(FALSE)
			} else {
				//PRINTF_INFO("NBHttpBody %llu bytes ended.\n", obj->body.dataTotalSz);
				//End of body
				NBASSERT(obj->header.isCompleted && obj->body.isCompleted)
				obj->readingMode++;
				obj->isCompleted = TRUE;
			}
		}
	}
	obj->fedTotal += (UI32)(c - str);
	return (UI32)(c - str);
}

UI32 NBHttpMessage_feedBytes(STNBHttpMessage* obj, const void* pData, const UI32 dataSz){
	UI32 r = 0;
	const char* data = (const char*)pData;
	//Reading header
	if(obj->readingMode == 0){
		r += NBHttpHeader_feedBytes(&obj->header, &data[r], (dataSz - r));
		if(NBHttpHeader_feedIsComplete(&obj->header)){
			if(!NBHttpHeader_feedEnd(&obj->header)){
				NBASSERT(FALSE)
			} else {
				//PRINTF_INFO("NBHttpHeader ended.\n");
				NBASSERT(obj->header.isCompleted);
				IHttpBodyListener lstnr;
				NBMemory_set(&lstnr, 0, sizeof(lstnr));
				lstnr.consumeData			= NBHttpMessage_consumeBodyData_;
				lstnr.consumeTrailerField	= NBHttpMessage_consumeBodyTrailerField_;
				lstnr.consumeEnd			= NBHttpMessage_consumeBodyEnd_;
				NBHttpBody_setListener(&obj->body, &lstnr, obj);
				NBHttpBody_feedStartWithHeader(&obj->body, &obj->header, obj->readBuffSz, obj->storgBuffSz, obj->origReqMethod.str);
				obj->readingMode++;
			}
		}
	}
	//Reading body
	if(obj->readingMode == 1){
		r += NBHttpBody_feedBytes(&obj->body, &data[r], (dataSz - r));
		if(NBHttpBody_feedIsComplete(&obj->body)){
			if(!NBHttpBody_feedEnd(&obj->body)){
                //Note: this point can be reached if the 'consumeBodyEnd' user's callback returns FALSE to signal the end of the connection.
				//NBASSERT(FALSE)
			} else {
				//PRINTF_INFO("NBHttpBody %llu bytes ended.\n", obj->body.dataTotalSz);
				//End of body
				NBASSERT(obj->header.isCompleted && obj->body.isCompleted)
				obj->readingMode++;
				obj->isCompleted = TRUE;
			}
		}
	}
    obj->fedTotal += r;
	return r;
}

BOOL NBHttpMessage_feedIsComplete(STNBHttpMessage* obj){
	BOOL r = obj->isCompleted;
	if(!r){
		r = (NBHttpHeader_feedIsComplete(&obj->header) && NBHttpBody_feedIsComplete(&obj->body));
	}
	return r;
}

BOOL NBHttpMessage_feedEnd(STNBHttpMessage* obj){
	NBHttpMessage_feedByte(obj, '\0');
	return obj->isCompleted;
}

//

UI32 NBHttpMessage_getBytesFedCount(const STNBHttpMessage* obj){
	return obj->fedTotal;
}
