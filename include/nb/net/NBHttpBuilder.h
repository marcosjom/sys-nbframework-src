#ifndef NB_HTTP_BUILDER_H
#define NB_HTTP_BUILDER_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/core/NBStructMap.h"
#include "nb/net/NBHttpTransferEncoding.h"
#include "nb/net/NBHttpChunkExt.h"
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
	
	typedef struct STNBHttpBuilder_ {
		UI64		bytesTotal;
	} STNBHttpBuilder;
	
	void NBHttpBuilder_init(STNBHttpBuilder* obj);
	void NBHttpBuilder_release(STNBHttpBuilder* obj);
	
	//
	
	void NBHttpBuilder_empty(STNBHttpBuilder* obj);
	
	//
	BOOL NBHttpBuilder_addHeader(STNBHttpBuilder* obj, STNBString* dst, const STNBHttpHeader* header);
	BOOL NBHttpBuilder_addHeaderAndEnd(STNBHttpBuilder* obj, STNBString* dst, const STNBHttpHeader* header);
	BOOL NBHttpBuilder_addHeaderFields(STNBHttpBuilder* obj, STNBString* dst, const STNBHttpHeader* header);
	BOOL NBHttpBuilder_addHeaderFieldsPlain(STNBHttpBuilder* obj, STNBString* dst, const STNBHttpHeader* header);
	
	//Header
	BOOL NBHttpBuilder_addRequestLine(STNBHttpBuilder* obj, STNBString* dst, const char* method, const char* target, const UI8 majorVer, const UI8 minorVer);
	BOOL NBHttpBuilder_addRequestLineWithProtocol(STNBHttpBuilder* obj, STNBString* dst, const char* protocol, const char* method, const char* target, const UI8 majorVer, const UI8 minorVer);
	BOOL NBHttpBuilder_addStatusLine(STNBHttpBuilder* obj, STNBString* dst, const UI8 majorVer, const UI8 minorVer, const UI32 statusCode, const char* reasonPhrase);
	BOOL NBHttpBuilder_addHeaderField(STNBHttpBuilder* obj, STNBString* dst, const char* name, const char* value);
	BOOL NBHttpBuilder_addHeaderFieldPlain(STNBHttpBuilder* obj, STNBString* dst, const char* name, const char* value);
	BOOL NBHttpBuilder_addHeaderFieldFromUnscaped(STNBHttpBuilder* obj, STNBString* dst, const char* name, const char* value);
	BOOL NBHttpBuilder_addHeaderFieldStruct(STNBHttpBuilder* obj, STNBString* dst, const char* name, const STNBStructMap* structMap, const void* src, const UI32 srcSz);
	BOOL NBHttpBuilder_addHost(STNBHttpBuilder* obj, STNBString* dst, const char* host, const SI32 port);
	BOOL NBHttpBuilder_addHostFromUnscaped(STNBHttpBuilder* obj, STNBString* dst, const char* host);
	BOOL NBHttpBuilder_addContentLength(STNBHttpBuilder* obj, STNBString* dst, const UI64 contentLength);
	BOOL NBHttpBuilder_addTransferEncoding(STNBHttpBuilder* obj, STNBString* dst, const STNBHttpTransferEncoding* encoding);
	BOOL NBHttpBuilder_addHeaderEnd(STNBHttpBuilder* obj, STNBString* dst);
	
	//Chunked body
	BOOL NBHttpBuilder_addChunkHeader(STNBHttpBuilder* obj, STNBString* dst, const UI64 chunkSz);
	BOOL NBHttpBuilder_addChunkHeaderWithExt(STNBHttpBuilder* obj, STNBString* dst, const UI64 chunkSz, const STNBHttpChunkExt* ext);
	BOOL NBHttpBuilder_addChunkDataEnd(STNBHttpBuilder* obj, STNBString* dst);
	BOOL NBHttpBuilder_addChunkBodyEnd(STNBHttpBuilder* obj, STNBString* dst);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
