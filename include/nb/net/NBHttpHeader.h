#ifndef NB_HTTP_HEADER_H
#define NB_HTTP_HEADER_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/net/NBHttpParser.h"
#include "nb/net/NBHttpTransferEncoding.h"

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
	
	typedef struct STNBHttpHeaderRequestLine_ {
		UI32	method;			//index in "strs"
		UI32	target;			//index in "strs"
		UI8		majorVer;
		UI8		minorVer;
	} STNBHttpHeaderRequestLine;
	
	typedef struct STNBHttpHeaderStatusLine_ {
		UI8		majorVer;
		UI8		minorVer;
		UI32	statusCode;
		UI32	reasonPhrase;	//index in "strs"
	} STNBHttpHeaderStatusLine;
	
	typedef struct STNBHttpHeaderField_ {
		UI32	name;	//index in "strs"
		UI32	value;	//index in "strs"
	} STNBHttpHeaderField;

	//Cursor for reading header values:
	//Multiple http headers can be combined in one separated by comma;
	//quoted-strings "..." and comments (...) can be included.

	typedef struct STNBHttpHeaderFieldCursor_ {
		const char*	value;
		UI32		valueLen;
		struct {
			UI32	iField;	//Current field
			UI32	iByte;	//Next byte
		} state;
	} STNBHttpHeaderFieldCursor;

#	define NBHttpHeaderFieldCursor_init(OBJ)	NBMemory_setZeroSt(*(OBJ), STNBHttpHeaderFieldCursor)
#	define NBHttpHeaderFieldCursor_release(OBJ)	NBMemory_setZeroSt(*(OBJ), STNBHttpHeaderFieldCursor)

	//
	
	struct STNBHttpHeader_;
	
	typedef struct IHttpHeaderListener_ {
		BOOL (*consumeStartLine)(const struct STNBHttpHeader_* obj, void* lparam); //can be request-line or status-line (response)
		BOOL (*consumeFieldLine)(const struct STNBHttpHeader_* obj, const STNBHttpHeaderField* field, void* lparam);
		BOOL (*consumeEnd)(const struct STNBHttpHeader_* obj, void* lparam);
	} IHttpHeaderListener;
	
	typedef struct STNBHttpHeader_ {
		BOOL						isCompleted;		//Received the empty-line separator?
		STNBHttpParser*				httpParser;
		STNBHttpHeaderRequestLine*	requestLine;		//If message is a request
		STNBHttpHeaderStatusLine*	statusLine;			//If message is a status (response)
		STNBArray					fields;				//STNBHttpHeaderField
		STNBString					strs;				//String for all values
		IHttpHeaderListener			listener;
		void*						listenerParam;
	} STNBHttpHeader;
	
	void NBHttpHeader_init(STNBHttpHeader* obj);
	void NBHttpHeader_release(STNBHttpHeader* obj);
	//Listener
	void NBHttpHeader_setListener(STNBHttpHeader* obj, const IHttpHeaderListener* listener, void* listenerParam);
	//First line
	BOOL NBHttpHeader_setRequestLine(STNBHttpHeader* obj, const char* method, const char* target, const UI8 majorVer, const UI8 minorVer);
	BOOL NBHttpHeader_setStatusLine(STNBHttpHeader* obj, const UI8 majorVer, const UI8 minorVer, const UI32 statusCode, const char* reasonPhrase);
    BOOL NBHttpHeader_parseRequestTarget(const STNBHttpHeader* obj, STNBString* dstAbsPath, STNBString* dstQuery, STNBString* dstFragment);
    BOOL NBHttpHeader_strParseRequestTarget(const char* target, STNBString* dstAbsPath, STNBString* dstQuery, STNBString* dstFragment);
	//Fields
	void NBHttpHeader_empty(STNBHttpHeader* obj);
	void NBHttpHeader_addField(STNBHttpHeader* obj, const char* fieldName, const char* fieldValue);
    void NBHttpHeader_addFieldContentLength(STNBHttpHeader* obj, const UI32 contentLength);
	void NBHttpHeader_removeField(STNBHttpHeader* obj, const char* fieldName);
	//Fields
	BOOL NBHttpHeader_hasFieldValue(const STNBHttpHeader* obj, const char* fieldName, const char* fieldValue, const BOOL allowComments);
	const char* NBHttpHeader_getField(const STNBHttpHeader* obj, const char* fieldName);
	const char* NBHttpHeader_getFieldOrDefault(const STNBHttpHeader* obj, const char* fieldName, const char* defValue, const BOOL allowEmptyValue);
	//Fields as struct
	BOOL NBHttpHeader_getFieldAsStruct(const STNBHttpHeader* obj, const char* fieldName, const STNBStructMap* stMap, void* dst, const UI32 dstSz);
	//Field cursor
	BOOL NBHttpHeader_getNextFieldValue(const STNBHttpHeader* obj, const char* fieldName, const BOOL allowComments, STNBHttpHeaderFieldCursor* cursor);
	BOOL NBHttpHeader_getNextFieldValueStr(const char* fValue, const BOOL allowComments, STNBHttpHeaderFieldCursor* cursor);
	//Specific fields
	BOOL NBHttpHeader_isConnectionClose(const STNBHttpHeader* obj);
	BOOL NBHttpHeader_isContentLength(const STNBHttpHeader* obj);
	BOOL NBHttpHeader_isBodyChunked(const STNBHttpHeader* obj);
	UI64 NBHttpHeader_contentLength(const STNBHttpHeader* obj);
	//Concat as string
	void NBHttpHeader_concatHeader(const STNBHttpHeader* obj, STNBString* dst);
	//
	const char* NBHttpHeader_getRequestMethod(const STNBHttpHeader* obj);
    const char* NBHttpHeader_getRequestTarget(const STNBHttpHeader* obj);

	//Feed
	void NBHttpHeader_feedStart(STNBHttpHeader* obj);
	BOOL NBHttpHeader_feedByte(STNBHttpHeader* obj, const char c);
	UI32 NBHttpHeader_feed(STNBHttpHeader* obj, const char* str);
	UI32 NBHttpHeader_feedBytes(STNBHttpHeader* obj, const void* data, const UI32 dataSz);
	BOOL NBHttpHeader_feedIsComplete(STNBHttpHeader* obj);
	BOOL NBHttpHeader_feedEnd(STNBHttpHeader* obj);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
