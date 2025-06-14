//
//  NBString.h
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#ifndef NB_STRING_H
#define NB_STRING_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBDate.h"
#include "nb/core/NBDatetime.h"

struct STNBString_; //NBString and NBFile have mutual dependency
#include "nb/files/NBFile.h"

typedef enum ENStringFind_ {
	ENStringFind_First = 0,	//stop search at first found (find first)
	ENStringFind_All		//stop search at end of haystack (find all)
} ENStringFind;

#ifdef __cplusplus
extern "C" {
#endif
	
	//STNBString
	typedef struct STNBString_ {
		char*	str;
		UI32	length;
		//
		UI32	_buffSz;
		UI32	_buffGrowthMin;	//Growth minimun of the string buffer size when the limit is reached
		UI16	_buffGrowthRel;	//Growth exponent in 1/16 units (1.0 is added to this value)
	} STNBString;
	
	BOOL NBCompare_STNBString(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);
	BOOL NBCompare_charPtr(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);
	
	void NBString_init(STNBString* string);
	void NBString_initWithSz(STNBString* string, const UI32 sz, const SI32 growthMin, const float growthRelExtra /*after 1.0*/);
	void NBString_initWithOther(STNBString* string, const STNBString* str);
	void NBString_initWithStringAndStr(STNBString* string, const STNBString* other, const char* str);
	void NBString_initWithStr(STNBString* string, const char* str);
	void NBString_initWithStrs(STNBString* string, const char** arr, const UI32 arrSz);
	void NBString_initWithStrsAndNull(STNBString* string, const char** arrAndNull);
	void NBString_initWithStrBytes(STNBString* string, const char* str, const UI32 strSz);
	void NBString_release(STNBString* string);
	//
    UI32 NBString_getBufferGrowthMin(STNBString* string);
	void NBString_increaseBuffer(STNBString* string, const UI32 additionalMinimunReq);
	void NBString_resignToContent(STNBString* string); //Leaves the buffer orphan
	BOOL NBString_swapContent(STNBString* string, STNBString* string2);
	BOOL NBString_swapContentBytes(STNBString* string, char* buff, const UI32 buffUse, const UI32 buffSz);
	void NBString_empty(STNBString* string);
    BOOL NBString_truncate(STNBString* string, const UI32 strLen);
	void NBString_set(STNBString* string, const char* str);
	void NBString_setByte(STNBString* string, const char c);
	void NBString_setBytes(STNBString* string, const char* str, const UI32 strSz);
	UI32 NBString_replace(STNBString* string, const char* strFind, const char* strReplace);
	UI32 NBString_replaceByte(STNBString* string, const char cFind, const char cReplace);
	UI32 NBString_replaceSI32(STNBString* string, const char* strFind, const SI32 strReplace);
	UI32 NBString_replaceUI32(STNBString* string, const char* strFind, const UI32 strReplace);
	UI32 NBString_replaceSI64(STNBString* string, const char* strFind, const SI64 strReplace);
	UI32 NBString_replaceUI64(STNBString* string, const char* strFind, const UI64 strReplace);
	UI32 NBString_replaceFloat(STNBString* string, const char* strFind, const float strReplace, const SI8 decimals);
	UI32 NBString_replaceDouble(STNBString* string, const char* strFind, const double strReplace, const SI8 decimals);
	void NBString_concatDateTimeCompact(STNBString* string, const STNBDatetime dateTime);
	void NBString_concatSqlDate(STNBString* string, const STNBDate date);
	void NBString_concatSqlDatePart(STNBString* string, const STNBDatetime time);
	void NBString_concatSqlDatetime(STNBString* string, const STNBDatetime time);
	//void NBString_concatDateTimeCurrent(STNBString* string);
	//void NBString_concatDateTimeCompactCurrent(STNBString* string);
	//void NBString_concatDateTime(STNBString* string, const time_t dateTime);
	void NBString_concatSecondsWithFormat(STNBString* string, UI64 seconds);
	void NBString_concat(STNBString* string, const char* stringToAdd);
	void NBString_concatLower(STNBString* string, const char* stringToAdd);
	void NBString_concatLowerBytes(STNBString* string, const char* stringToAdd, const UI32 strSz);
	void NBString_concatUpper(STNBString* string, const char* stringToAdd);
	void NBString_concatUpperBytes(STNBString* string, const char* stringToAdd, const UI32 strSz);
	void NBString_concatByte(STNBString* string, const char charAdd);
	void NBString_concatBytes(STNBString* string, const void* data, const UI32 dataSz);
	void NBString_concatBytesHex(STNBString* string, const void* data, const UI32 dataSz);
	void NBString_concatUI32(STNBString* string, UI32 number);
	void NBString_concatUI64(STNBString* string, UI64 number);
	void NBString_concatSI32(STNBString* string, SI32 number);
	void NBString_concatSI64(STNBString* string, SI64 number);
	void NBString_concatFloat(STNBString* string, float number, const SI8 decimals);
	void NBString_concatDouble(STNBString* string, double number, const SI8 decimals);
	void NBString_concatRepeatedByte(STNBString* string, const char charAdd, const UI32 qnt);
	//UTF16
	void NBString_concatAsUtf16(STNBString* string, const char* str);
	void NBString_concatAsUtf16Bytes(STNBString* string, const char* str, const UI32 bytes);
	void NBString_concatUtf16(STNBString* string, const UI16* str);
	void NBString_concatUtf16Chars(STNBString* string, const UI16* str, const UI32 chars);
	//Unicode
	void NBString_concatUnicode(STNBString* string, const UI32* str);
	void NBString_concatUnicodeChars(STNBString* string, const UI32* str, const UI32 chars);
	//
	void NBString_removeBytes(STNBString* string, const UI32 start, const UI32 numBytes);
	void NBString_removeFirstBytes(STNBString* string, const UI32 numBytes);
	void NBString_removeLastBytes(STNBString* string, const UI32 numBytes);
	//
	void NBString_replaceBytes(STNBString* string, const UI32 start, const UI32 numBytes, const char* newStr, const UI32 newStrLen);
	//
	UI32 NBString_lenBytes(const STNBString* string);
	UI32 NBString_lenUnicodes(const STNBString* string);
	SI32 NBString_indexOf(const STNBString* string, const char* needle, SI32 posInitial);
	SI32 NBString_indexOfLike(const STNBString* string, const char* needle, SI32 posInitial);
	SI32 NBString_lastIndexOf(const STNBString* string, const char* needle, SI32 posInitialReverse);
	BOOL NBString_startsWith(const STNBString* string, const char* stringBase);
	BOOL NBString_isEqual(const STNBString* string, const char* string2);
	BOOL NBString_isEqualBytes(const STNBString* string, const char* string2, const UI32 len);
	BOOL NBString_isLike(const STNBString* string, const char* string2);
	BOOL NBString_isLikeBytes(const STNBString* string, const char* string2, const UI32 len);
	BOOL NBString_isLower(const STNBString* string, const char* str2);
	BOOL NBString_isLowerOrEqual(const STNBString* string, const char* str2);
	BOOL NBString_isGreater(const STNBString* string, const char* str2);
	BOOL NBString_isGreaterOrEqual(const STNBString* string, const char* str2);
	//
	char* NBString_strRepeatByte(char* dst, const char c, const UI32 times);
	//
	char* NBString_strNewBuffer(const char* string);
	char* NBString_strNewBufferBytes(const char* string, const UI32 len);
	void NBString_strFreeAndNewBuffer(char** org, const char* string);
	void NBString_strFreeAndNewBufferBytes(char** org, const char* string, const UI32 len);
	BOOL NBString_strIsEmpty(const char* str);
	UI32 NBString_strLenBytes(const char* string);
	UI32 NBString_strLenUnicodes(const char* string);
	SI32 NBString_strIndexOf(const char* haystack, const char* needle, SI32 posInitial);
	SI32 NBString_strIndexOfBytes(const char* haystack, const UI32 haystackLen, const char* needle, SI32 posInitial);
	SI32 NBString_strIndexOfLike(const char* haystack, const char* needle, SI32 posInitial);
	SI32 NBString_strLastIndexOf(const char* haystack, const char* needle, SI32 posInitialReverse);
	BOOL NBString_strStartsWith(const char* stringCompare, const char* stringBase);
	BOOL NBString_strStartsWithBytes(const char* stringCompare, const char* stringBase, const UI32 stringBaseSz);
	BOOL NBString_strEndsWith(const char* stringCompare, const char* stringEnd);
	BOOL NBString_strContainsImperfect(const char* haystack, const char* needle, const UI32 maxErrors, const ENStringFind srchMode); //Errors are miss or extra unicode-chars
	//is equal
	BOOL NBString_strIsEqual(const char* string1, const char* string2);
	BOOL NBString_strIsEqualBytes(const char* string1, const UI32 len1, const char* string2, const UI32 len2);
	BOOL NBString_strIsEqualStrBytes(const char* string1, const char* string2, const UI32 len2);
	//is like
	BOOL NBString_strIsLike(const char* string1, const char* string2);
	BOOL NBString_strIsLikeBytes(const char* string1, const UI32 len1, const char* string2, const UI32 len2);
	BOOL NBString_strIsLikeStrBytes(const char* string1, const char* string2, const UI32 len2);
	//is lower
	BOOL NBString_strIsLower(const char* str1, const char* str2);
	BOOL NBString_strIsLowerBytes(const char* str1, const UI32 len1, const char* str2, const UI32 len2);
	BOOL NBString_strIsLowerStrBytes(const char* str1, const char* str2, const UI32 len2);
	//is lower or equal
	BOOL NBString_strIsLowerOrEqual(const char* str1, const char* str2);
	BOOL NBString_strIsLowerOrEqualBytes(const char* str1, const UI32 len1, const char* str2, const UI32 len2);
	BOOL NBString_strIsLowerOrEqualStrBytes(const char* str1, const char* str2, const UI32 len2);
	//is greather
	BOOL NBString_strIsGreater(const char* str1, const char* str2);
	BOOL NBString_strIsGreaterBytes(const char* str1, const UI32 len1, const char* str2, const UI32 len2);
	BOOL NBString_strIsGreaterStrBytes(const char* str1, const char* str2, const UI32 len2);
	//is greather or equal
	BOOL NBString_strIsGreaterOrEqual(const char* str1, const char* str2);
	BOOL NBString_strIsGreaterOrEqualBytes(const char* str1, const UI32 len1, const char* str2, const UI32 len2);
	BOOL NBString_strIsGreaterOrEqualStrBytes(const char* str1, const char* str2, const UI32 len2);
	//
	UI8 NBString_strIsInteger(const char* string);
	UI8 NBString_strIsIntegerBytes(const char* string, const SI32 bytesCount);
	UI8 NBString_strIsDecimal(const char* string);
	//
	SI32 NBString_strToSI32(const char* string);
	SI32 NBString_strToSI32Bytes(const char* string, const SI32 bytesCount);
	SI64 NBString_strToSI64(const char* string);
	UI32 NBString_strToUI32(const char* string);
	UI32 NBString_strToUI32Bytes(const char* string, const SI32 bytesCount);
	UI64 NBString_strToUI64(const char* string);
	float NBString_strToFloat(const char* string);
	double NBString_strToDouble(const char* string);
	double NBString_strToDoubleBytes(const char* string, const SI32 bytesCount);
	long double NBString_strToLongDouble(const char* string);
	long double NBString_strToLongDoubleBytes(const char* string, const SI32 bytesCount);
	SI32 NBString_strToSI32FromHex(const char* string);
	SI32 NBString_strToSI32FromHexLen(const char* string, const SI32 strLen);
	SI64 NBString_strToBytes(const char* string);
	SI64 NBString_strToSecs(const char* string);
	//
	SI32 NBString_strToSI32IfValid(const char* string, const SI32 valueDefault);
	SI64 NBString_strToSI64IfValid(const char* string, const SI64 valueDefault);
	UI32 NBString_strToUI32IfValid(const char* string, const UI32 valueDefault);
	UI64 NBString_strToUI64IfValid(const char* string, const UI64 valueDefault);
	float NBString_strToFloatIfValid(const char* string, const float valueDefault);
	double NBString_strToDoubleIfValid(const char* string, const double valueDefault);
	SI32 NBString_strToSI32FromHexIfValid(const char* string, const SI32 valueDefault);
	SI32 NBString_strToSI32FromHexLenIfValid(const char* string, const SI32 strLen, const SI32 valueDefault);
	//File
	SI32 NBString_writeToFile(const STNBString* obj, STNBObjRef file, const BOOL includeUnusedBuffer);
	SI32 NBString_initFromFile(STNBString* obj, STNBObjRef file);
	
	//dbg
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	void NBString_dbgTestRemoveAndReplace(const UI32 totalTests, const UI32 ciclesPerTest, const UI8 printLevel);
#	endif

#ifdef __cplusplus
} //extern "C"
#endif

#endif
