#ifndef NB_HTTP_TRANSFER_ENCODING_H
#define NB_HTTP_TRANSFER_ENCODING_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBString.h"
#include "nb/net/NBHttpTransferCoding.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	//-----------------
	//-- HTTP 1.1 - RFC7230
	//-- https://tools.ietf.org/html/rfc7230
	//-----------------
	//transfer-encoding:	//1#transfer-coding == transfer-coding *( OWS "," OWS transfer-coding )
	
	typedef struct STNBHttpTransferEncoding_ {
		STNBArray	codings;		//STHttpTransferCoding
	} STNBHttpTransferEncoding;
	
	//
	
	void NBHttpTransferEncoding_init(STNBHttpTransferEncoding* obj);
	void NBHttpTransferEncoding_release(STNBHttpTransferEncoding* obj);
	
	//
	
	BOOL NBHttpTransferEncoding_isChunked(STNBHttpTransferEncoding* obj);
	
	//
	
	BOOL NBHttpTransferEncoding_addFromLine(STNBHttpTransferEncoding* obj, const char* line);
	BOOL NBHttpTransferEncoding_addFromLineBytes(STNBHttpTransferEncoding* obj, const char* line, const UI32 lineSz);
	BOOL NBHttpTransferEncoding_addCodingType(STNBHttpTransferEncoding* obj, const ENNBHttpTransferCodingType type);
	BOOL NBHttpTransferEncoding_addCoding(STNBHttpTransferEncoding* obj, const STNBHttpTransferCoding* coding);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
