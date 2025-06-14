//
//  NBString.h
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#ifndef NB_ENCODING_H
#define NB_ENCODING_H

#include "nb/NBFrameworkDefs.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	//ASCII
	char	NBEncoding_asciiOtherCase(const char c); //changing from uppercase to lowercase and viceversa
	char	NBEncoding_asciiUpper(const char c);
	char	NBEncoding_asciiLower(const char c);
	
	//UTF8
	UI8		NBEncoding_utf8BytesExpected(const char first);
	UI8     NBEncoding_utf8FromUnicode(const UI32 unicode, char* dstChars7);
	
	//UTF16
	UI8		NBEncoding_utf16BytesExpected(const UI16 firstSurrogate);
	UI8		NBEncoding_utf16SurrogatesExpected(const UI16 firstSurrogate);
	UI16	NBEncoding_utf16SurrogateFromHex(const char* chars4);
	BOOL	NBEncoding_utf16SurrogateIsNonCharacterHex(const char* chars4);
	
	//UNICODE
	UI32	NBEncoding_unicodeFromUtf8(const char* charsUtf8, const UI32 errValue);
	UI32	NBEncoding_unicodeFromUtf8s(const char* charsUtf8, const UI8 surrogatesCount, const UI32 errValue);
	UI32	NBEncoding_unicodeFromUtf16(const UI16* charsUtf16, const UI32 errValue);
	UI32	NBEncoding_unicodeFromUtf16s(const UI16* charsUtf16, const UI8 surrogatesCount, const UI32 errValue);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
