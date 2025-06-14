#ifndef NB_HTTP_BODY_H
#define NB_HTTP_BODY_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/net/NBHttpParser.h"
#include "nb/net/NBHttpHeader.h"

//

#ifdef __cplusplus
extern "C" {
#endif
	
	//-----------------
	//-- HTTP 1.1 - RFC7230
	//-- https://tools.ietf.org/html/rfc7230
	//-----------------
	//-- ERRATA
	//--- https://www.rfc-editor.org/errata_search.php?rfc=7230
	//-----------------
	
	typedef struct STNBHttpBodyChunk_ {
		UI64	notif;	//current notifying position
		UI64	use;	//current filling position
		UI64	size;	//expected size
		BYTE*	data;	//data
	} STNBHttpBodyChunk;
	
	typedef struct STNBHttpBodyField_ {
		UI32	name;	//index in "strs"
		UI32	value;	//index in "strs"
	} STNBHttpBodyField;
	
	typedef enum ENNBHttpBodyMode_ {
		ENNBHttpBodyMode_untillClose = 0,	//No content-length and not chunked (read untill the connection is close)
		ENNBHttpBodyMode_contentLength,		//Explicit content-length
		ENNBHttpBodyMode_chunked,			//Chunked
	} ENNBHttpBodyMode;
	
	struct STNBHttpBody_;
	
	typedef struct IHttpBodyListener_ {
		BOOL (*consumeData)(const struct STNBHttpBody_* obj, const void* data, const UI64 dataSz, void* lparam);
		BOOL (*consumeTrailerField)(const struct STNBHttpBody_* obj, const STNBHttpBodyField* field, void* lparam);
		BOOL (*consumeEnd)(const struct STNBHttpBody_* obj, void* lparam);
	} IHttpBodyListener;
	
	typedef struct STNBHttpBody_ {
		BOOL				isCompleted;		//Received all data or all the chunks and trailer.
		UI32				readBuffSz;			//Read buffer size (zero if data must be ignored)
		UI32				storgBuffSz;		//Storage buffer sizes (zero if must not be stored, only passed to listener)
		ENNBHttpBodyMode	mode;
		UI64				expctLength;		//When Content-length is provided
		UI64				dataTotalSz;
		STNBHttpParser*		httpParser;
		STNBArray			chunks;				//STNBHttpBodyChunk
		STNBArray			fields;				//STNBHttpBodyField (trailer-part)
		STNBString			strs;				//String for all values
		IHttpBodyListener	listnr;
		void*				listnrParam;
	} STNBHttpBody;
	
	void NBHttpBody_init(STNBHttpBody* obj);
	void NBHttpBody_release(STNBHttpBody* obj);
	
	void NBHttpBody_empty(STNBHttpBody* obj);
	void NBHttpBody_emptyChuncksData(STNBHttpBody* obj);
	
	//Listener
	void NBHttpBody_setListener(STNBHttpBody* obj, const IHttpBodyListener* listener, void* listenerParam);
	
	//Chunks
	UI32 NBHttpBody_chunksTotalBytes(const STNBHttpBody* obj);
	void NBHttpBody_chunksConcatAll(const STNBHttpBody* obj, STNBString* dst);
	
	//Feed
	void NBHttpBody_feedStartWithHeader(STNBHttpBody* obj, STNBHttpHeader* header, const UI32 readBuffSize, const UI32 storgBuffSz, const char* optReqMethod);
	void NBHttpBody_feedSetStorageBuffSize(STNBHttpBody* obj, const UI32 storgBuffSz);
	void NBHttpBody_feedStartUntillClose(STNBHttpBody* obj, const UI32 readBuffSize, const UI32 storgBuffSz);
	void NBHttpBody_feedStartWithContentLength(STNBHttpBody* obj, const UI64 contentLength, const UI32 readBuffSize, const UI32 storgBuffSz);
	void NBHttpBody_feedStartChunked(STNBHttpBody* obj, const UI32 readBuffSize, const UI32 storgBuffSz);
	BOOL NBHttpBody_feedByte(STNBHttpBody* obj, const char c);
	UI32 NBHttpBody_feed(STNBHttpBody* obj, const char* str);
	UI32 NBHttpBody_feedBytes(STNBHttpBody* obj, const void* data, const UI32 dataSz);
	BOOL NBHttpBody_feedIsComplete(STNBHttpBody* obj);
	BOOL NBHttpBody_feedEnd(STNBHttpBody* obj);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
