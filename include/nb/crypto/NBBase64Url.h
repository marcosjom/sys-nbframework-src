// NBBase64Url.h: interface for the NBBase64Url class.
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
	
	char	NBBase64Url_token(const char idx);
	BOOL	NBBase64Url_isToken(const char c);
	void    NBBase64Url_code3Bytes(const void* bytes3, const SI32 numBytes, char* buffDst4Bytes, UI8* dstBytesFilledCount, const BOOL includePadding);
	//UI8	    NBBase64Url_decode4Bytes(const char* bytes4, char* buff3Bytes); //ToDo: implement ready for omited '=' chars
	
	void    NBBase64Url_code(STNBString* dst, const char* src, const BOOL includePadding);
	void    NBBase64Url_codeBytes(STNBString* dst, const char* src, const UI32 len, const BOOL includePadding);
	
	//BOOL    NBBase64Url_decode(STNBString* dst, const char* src); //ToDo: implement ready for omited '=' chars
	//BOOL    NBBase64Url_decodeBytes(STNBString* dst, const char* src, const UI32 len); //ToDo: implement ready for omited '=' chars
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
