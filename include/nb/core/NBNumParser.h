//
//  XUXml.h
//  XUMonitorLogicaNegocios_OSX
//
//  Created by Marcos Ortega on 21/01/14.
//  Copyright (c) 2014 XurpasLatam. All rights reserved.
//

#ifndef NB_NUM_PARSER_H
#define NB_NUM_PARSER_H

#include "nb/NBFrameworkDefs.h"

//
//http://en.cppreference.com/w/cpp/language/integer_literal
//Integer literal:
//decimal-literal integer-suffix(optional)
//octal-literal integer-suffix(optional)
//hex-literal integer-suffix(optional)
//binary-literal integer-suffix(optional)
//
//http://en.cppreference.com/w/cpp/language/floating_literal
//Floating literal:
//significand exponent(optional) suffix(optional)

#define STR_NumericFormat(X)		(X == ENNumericFormat_DigSeq ? "DIGIT_SEQ" : X == ENNumericFormat_Hexadec ? "HEXADEC" : X == ENNumericFormat_Octal ? "OCTAL" : X == ENNumericFormat_Binary ? "BINARY" : "UNKNOWN")
#define STR_NumericType(X)			(X == ENNumericType_Integer ? "INTEGER" : X == ENNumericType_Floating ? "FLOAT" : "UNKNOWN")
#define STR_NumericTypeSub(X)		(X == ENNumericTypeSub_Int ? "INT" : X == ENNumericTypeSub_IntU ? "U_INT" : X == ENNumericTypeSub_Long ? "LONG" : X == ENNumericTypeSub_LongU ? "U_LONG" : X == ENNumericTypeSub_LongLong ? "LONG_LONG" : X == ENNumericTypeSub_LongLongU ? "U_LONG_LONG" : X == ENNumericTypeSub_Float ? "FLOAT" : X == ENNumericTypeSub_Double ? "DOUBLE" : X == ENNumericTypeSub_DoubleLong ? "LONG_DOUBLE" : "UNKNOWN")


#ifdef __cplusplus
extern "C" {
#endif
	
	typedef enum ENNumericFormat_ {
		ENNumericFormat_DigSeq = 0,		//Digits sequence: '123456790' (integers and floating)
		ENNumericFormat_Hexadec,		//Hexadecimal: '0xabcdef' (integers and floating)
		ENNumericFormat_Octal,			//Octal: '012345670' (only for integers)
		ENNumericFormat_Binary,			//Binary: 0b1010111 (only for integers)
		ENNumericFormat_Count,
	} ENNumericFormat;
	
	typedef enum ENNumericType_ {
		ENNumericType_Integer = 0,		//integer
		ENNumericType_Floating,			//floating point
		ENNumericType_Count
	} ENNumericType;
	
	typedef enum ENNumericTypeSub_ {
		ENNumericTypeSub_Int = 0,		//integer signed
		ENNumericTypeSub_IntU,			//integer unsigned
		ENNumericTypeSub_Long,			//long signed
		ENNumericTypeSub_LongU,			//long unsigned
		ENNumericTypeSub_LongLong,		//long long signed
		ENNumericTypeSub_LongLongU,		//long long unsigned
		ENNumericTypeSub_Float,			//float
		ENNumericTypeSub_Double,		//double
		ENNumericTypeSub_DoubleLong,	//long double
		ENNumericTypeSub_Count
	} ENNumericTypeSub;

	const char* NBNumericTypeSub_getNameByUid(const ENNumericTypeSub uid); 
	
	typedef struct STNBNumParser_ {
		BOOL				isErr;
		//Basic
		UI8					charsTotal;		//Count all chars processed
		UI8					charsPrefx;		//Count of chars at prefix
		UI8					charsBody;		//Count of chars between prefix and sufix
		UI8					charsSufix;		//Count of chars at sufix
		//
		UI8					charsBodyExp;		//Count of chars inside the body that are part of the exponent
		UI8					charsBodyExpDigits;	//Count of chars inside the exponent that are digits (at least one must be found)
		UI8					implicitExp;	//Implicit exponent defined by content after the float-dot (1 for each decimal digit, 4 for each hex digit)
		BOOL				isExpNeg;		//If (charsBodyExp != 0), it is negative?
		UI64				partInt;		//Integer part's value (no sign)
		UI16				partExp;		//Exponent part's value (no sign)
		//
		ENNumericFormat		format;
		ENNumericType		type;
		ENNumericTypeSub	typeSub;
		//Final value
		union {
			int					valInt;
			unsigned int		valIntU;
			long				valLong;
			unsigned long		valLongU;
			long long			valLongLong;
			unsigned long long	valLongLongU;
			float				valFloat;
			double				valDouble;
			long double			valDoubleLong;
		};
	} STNBNumParser;
	
	void NBNumParser_init(STNBNumParser* obj);
	void NBNumParser_release(STNBNumParser* obj);
	//Parsing unsigned
	BOOL NBNumParser_feedSufix(STNBNumParser* state, const char c);
	BOOL NBNumParser_feedByte(STNBNumParser* state, const char c);
	BOOL NBNumParser_feed(STNBNumParser* state, const char* str);
	BOOL NBNumParser_feedBytes(STNBNumParser* state, const char* str, const SI32 len);
	BOOL NBNumParser_hasValidEnd(const STNBNumParser* state, ENNumericType* dstType);
	BOOL NBNumParser_end(STNBNumParser* state);
	//Static methods
	STNBNumParser NBNumParser_strParseUnsigned(const char* str);
	STNBNumParser NBNumParser_strParseUnsignedBytes(const char* str, const UI32 strSz);
	SI32 NBNumParser_toSI32(const char* str, BOOL* dstSuccess);
	SI64 NBNumParser_toSI64(const char* str, BOOL* dstSuccess);
	UI32 NBNumParser_toUI32(const char* str, BOOL* dstSuccess);
	UI64 NBNumParser_toUI64(const char* str, BOOL* dstSuccess);
	UI32 NBNumParser_hexToUI32(const char* str, BOOL* dstSuccess);
	UI64 NBNumParser_hexToUI64(const char* str, BOOL* dstSuccess);
	//
	SI32 NBNumParser_toSI32Bytes(const char* str, const UI32 strSz, BOOL* dstSuccess);
	SI64 NBNumParser_toSI64Bytes(const char* str, const UI32 strSz, BOOL* dstSuccess);
	UI32 NBNumParser_toUI32Bytes(const char* str, const UI32 strSz, BOOL* dstSuccess);
	UI64 NBNumParser_toUI64Bytes(const char* str, const UI32 strSz, BOOL* dstSuccess);
	UI32 NBNumParser_hexToUI32Bytes(const char* str, const UI32 strSz, BOOL* dstSuccess);
	UI64 NBNumParser_hexToUI64Bytes(const char* str, const UI32 strSz, BOOL* dstSuccess);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
