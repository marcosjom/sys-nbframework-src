// NBBase64.h: interface for the NBBase64 class.
//
//////////////////////////////////////////////////////////////////////

#ifndef NB_BASE64_H
#define NB_BASE64_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef __cplusplus
extern "C" {
#endif
	
	char	NBBase64_token(const char idx);
	BOOL	NBBase64_isToken(const char c);
	void    NBBase64_code3Bytes(const void* bytes3, const SI32 numBytes, char* buffDst4Bytes);
	UI8	    NBBase64_decode4Bytes(const char* bytes4, char* buff3Bytes);
	
	void    NBBase64_code(STNBString* dst, const char* src);
	void    NBBase64_codeBytes(STNBString* dst, const char* src, const UI32 len);
	
	BOOL    NBBase64_decode(STNBString* dst, const char* src);
	BOOL    NBBase64_decodeBytes(STNBString* dst, const char* src, const UI32 len);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
