#ifndef NB_HTTP2_HPACK_H
#define NB_HTTP2_HPACK_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
//
#include "nb/net/NBHttpHeader.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	//-----------------
	//-- HTTP 2.0 - RFC7541
	//-- https://tools.ietf.org/html/rfc7541
	//-----------------
	
	typedef struct STNBHttp2HpackTableItmVal_ {
		BOOL		isError;	//not found
		const char*	name;		//idx in strs
		const char* value;		//idx in strs
	} STNBHttp2HpackTableItmVal;
	
	typedef struct STNBHttp2HpackTableItm_ {
		UI32	iName;		//idx in strs
		UI32	iValue;		//idx in strs
	} STNBHttp2HpackTableItm;
	
	typedef struct STNBHttp2HpackSrchResult_ {
		UI32 iName;		//zero if not found
		UI32 iValue;	//zero if not found
	} STNBHttp2HpackSrchResult;
		
	typedef struct STNBHttp2Hpack_ {
		STNBArray		tblStatic;	//STNBHttp2HpackTableItm
		STNBArray		tblDynamic;	//STNBHttp2HpackTableItm
		STNBString		strs;			//all strings
	} STNBHttp2Hpack;
	
	void NBHttp2Hpack_init(STNBHttp2Hpack* obj);
	void NBHttp2Hpack_release(STNBHttp2Hpack* obj);
	
	UI32 NBHttp2Hpack_tablesTotalSizes(STNBHttp2Hpack* obj);
	
	STNBHttp2HpackTableItmVal	NBHttp2Hpack_getItmValue(STNBHttp2Hpack* obj, const UI32 idx);
	STNBHttp2HpackSrchResult	NBHttp2Hpack_getPairIndexes(STNBHttp2Hpack* obj, const char* name, const char* value);
	
	//Primitives
	BOOL NBHttp2Hpack_encodeInteger(const UI8 prefixN, const UI64 pValue, STNBString* dst);
	BOOL NBHttp2Hpack_decodeInteger(const UI8 prefixN, UI32* iPos, const void* pData, const UI32 dataSz, UI64* dst);
	BOOL NBHttp2Hpack_encodeStrLiteral(const void* str, const UI32 strLen, const BOOL doHuffman, STNBString* dst);
	BOOL NBHttp2Hpack_decodeStrLiteral(UI32* iPos, const void* pData, const UI32 dataSz, STNBString* dstData, BOOL* dstIsHuffman);
	
	//Blocks
	BOOL NBHttp2Hpack_decodeBlock(STNBHttp2Hpack* obj, const void* data, const UI32 dataSz, STNBHttpHeader* dst);
	BOOL NBHttp2Hpack_encodeBlock(STNBHttp2Hpack* obj, const STNBHttpHeader* pairs, STNBString* dst);
	BOOL NBHttp2Hpack_encodePair(STNBHttp2Hpack* obj, const char* name, const char* value, const BOOL useTables, const BOOL allowTableStored, STNBString* dst);
	
	//
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
