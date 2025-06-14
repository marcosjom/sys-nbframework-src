#ifndef NB_HTTP_CHUNK_EXT_H
#define NB_HTTP_CHUNK_EXT_H

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
	//chunk-ext      = *( BWS  ";" BWS chunk-ext-name [ BWS  "=" BWS chunk-ext-val ] )
	//chunk-ext-name = token
	//chunk-ext-val  = token / quoted-string
	
	typedef struct STNBHttpChunkExtElem_ {
		UI32	iName;
		UI32	iValue;
	} STNBHttpChunkExtElem;
	
	typedef struct STNBHttpChunkExt_ {
		STNBArray		exts;			//STNBHttpChunkExtElem
		STNBString		strs;
	} STNBHttpChunkExt;
	
	//
	
	void NBHttpChunkExt_init(STNBHttpChunkExt* obj);
	void NBHttpChunkExt_initWithOther(STNBHttpChunkExt* obj, const STNBHttpChunkExt* other);
	void NBHttpChunkExt_release(STNBHttpChunkExt* obj);
	
	//
	
	void NBHttpChunkExt_addExt(STNBHttpChunkExt* obj, const char* name, const char* value);
	
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
