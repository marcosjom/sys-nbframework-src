#ifndef NB_HTTP_READER_H
#define NB_HTTP_READER_H

#include "nb/NBFrameworkDefs.h"
#include "nb/net/NBHttpHeader.h"
#include "nb/net/NBHttpBody.h"

//

#ifdef __cplusplus
extern "C" {
#endif
	
	struct STNBHttpMessage_;
	
	typedef struct IHttpMessageListener_ {
		//Reading header
		BOOL (*consumeHeadStartLine)(const struct STNBHttpMessage_* obj, void* lparam); 
		BOOL (*consumeHeadFieldLine)(const struct STNBHttpMessage_* obj, const STNBHttpHeaderField* field, void* lparam);
		BOOL (*consumeHeadEnd)(const struct STNBHttpMessage_* obj, UI32* dstBodyBuffSz, void* lparam);
		//Reading body
		BOOL (*consumeBodyData)(const struct STNBHttpMessage_* obj, const void* data, const UI64 dataSz, void* lparam);
		BOOL (*consumeBodyTrailerField)(const struct STNBHttpMessage_* obj, const STNBHttpBodyField* field, void* lparam);
		BOOL (*consumeBodyEnd)(const struct STNBHttpMessage_* obj, void* lparam);
	} IHttpMessageListener;
	
	typedef struct STNBHttpMessage_ {
		BOOL				isCompleted;
		STNBHttpHeader		header;
		STNBHttpBody		body;
		UI32				fedTotal;
		UI32 				readBuffSz;		//Read buffer size (zero if data must be ignored)
		UI32 				storgBuffSz;	//Storage buffer sizes (zero if must not be stored, only passed to listener)
		UI32				readingMode; 	//0 = init header, 1 = read header, 2 = read body
		STNBString			origReqMethod;	//The request method, if the current line is a status/response. (needed to determine if the request has body)
		//
		IHttpMessageListener listener;
		void*				listenerParam;
	} STNBHttpMessage;
	
	void NBHttpMessage_init(STNBHttpMessage* obj);
	void NBHttpMessage_release(STNBHttpMessage* obj);
	
	void NBHttpMessage_setListener(STNBHttpMessage* obj, const IHttpMessageListener* listnr, void* listnrParam);

	void NBHttpMessage_feedStart(STNBHttpMessage* obj, const UI32 readBuffSz, const UI32 storgBuffSz);
	void NBHttpMessage_feedStartResponseOf(STNBHttpMessage* obj, const UI32 readBuffSz, const UI32 storgBuffSz, const char* origReqMethod);
	BOOL NBHttpMessage_feedByte(STNBHttpMessage* obj, const char c);
	UI32 NBHttpMessage_feed(STNBHttpMessage* obj, const char* str);
	UI32 NBHttpMessage_feedBytes(STNBHttpMessage* obj, const void* data, const UI32 dataSz);
	BOOL NBHttpMessage_feedIsComplete(STNBHttpMessage* obj);
	BOOL NBHttpMessage_feedEnd(STNBHttpMessage* obj);
	//
	UI32 NBHttpMessage_getBytesFedCount(const STNBHttpMessage* obj);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
