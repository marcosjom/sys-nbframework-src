#ifndef NB_HTTP_TRANSFER_CODING_H
#define NB_HTTP_TRANSFER_CODING_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBString.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	//-----------------
	//-- HTTP 1.1 - RFC7230
	//-- https://tools.ietf.org/html/rfc7230
	//-----------------
	//transfer-coding:		"chunked" / "compress" / "deflate" / "gzip" / transfer-extension
	//transfer-extension:	token *( OWS ";" OWS transfer-parameter )
	//transfer-parameter:	token / token BWS "=" BWS ( token / quoted-string )
	
	typedef enum ENNBHttpTransferCodingType_ {
		ENNBHttpTransferCodingType_chunked = 0
		, ENNBHttpTransferCodingType_compress
		, ENNBHttpTransferCodingType_deflate
		, ENNBHttpTransferCodingType_gzip
		, ENNBHttpTransferCodingType_extension
		//
		, ENNBHttpTransferCodingType_count
	} ENNBHttpTransferCodingType;
	
	typedef struct STNBHttpTransferParam_ {
		UI32	iName;
		UI32	iValue;
	} STNBHttpTransferParam;
	
	typedef struct STNBHttpTransferCoding_ {
		ENNBHttpTransferCodingType	type;
		STNBString*					extensionType;
		STNBArray*					params;			//STNBHttpTransferParam
		STNBString*					paramsStrs;
	} STNBHttpTransferCoding;
	
	//
	
	void NBHttpTransferCoding_init(STNBHttpTransferCoding* obj);
	void NBHttpTransferCoding_initWithOther(STNBHttpTransferCoding* obj, const STNBHttpTransferCoding* other);
	void NBHttpTransferCoding_release(STNBHttpTransferCoding* obj);
	
	//
	
	void NBHttpTransferCoding_setType(STNBHttpTransferCoding* obj, const ENNBHttpTransferCodingType type);
	void NBHttpTransferCoding_setTypeExtension(STNBHttpTransferCoding* obj, const char* type);
	void NBHttpTransferCoding_setTypeExtensionBytes(STNBHttpTransferCoding* obj, const char* type, const UI32 typeSz);
	
	void NBHttpTransferCoding_addParam(STNBHttpTransferCoding* obj, const char* token, const char* value);
	void NBHttpTransferCoding_addParamBytes(STNBHttpTransferCoding* obj, const char* token, const UI32 tokenSz, const char* value, const UI32 valueSz);
	
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
