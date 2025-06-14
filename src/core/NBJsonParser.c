//
//  XUXml.c
//  XUMonitorLogicaNegocios_OSX
//
//  Created by Marcos Ortega on 21/01/14.
//  Copyright (c) 2014 XurpasLatam. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBJsonParser.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBEncoding.h"

//https://www.json.org/json-en.html
//https://github.com/nst/JSONTestSuite

typedef enum ENJsonParserSeqMode_ {
	ENJsonParserSeqMode_Value = 0,	//
	ENJsonParserSeqMode_String,		//
	ENJsonParserSeqMode_Number,		//
	ENJsonParserSeqMode_Object,		//
	ENJsonParserSeqMode_Array,		//
	//
	ENJsonParserSeqMode_Count
} ENJsonParserSeqMode;

//Value

typedef enum ENJsonParserSeqModeValue_ {
	ENJsonParserSeqModeValue_WhitespacePre = 0,	//pre-value whitespaces
	ENJsonParserSeqModeValue_ValueInline,		//inline value (true, false, null, ...)
	ENJsonParserSeqModeValue_Value,			    //value returned from stack
	ENJsonParserSeqModeValue_WhitespacePost,	//post-value whitespaces
	//
	ENJsonParserSeqModeValue_Count
} ENJsonParserSeqModeValue;

//String

typedef enum ENJsonParserSeqModeString_ {
	ENJsonParserSeqModeString_Any = 0,		//any codepoint except ", \ or control-chars
	ENJsonParserSeqModeString_AfterSlash,	//first char after slash (", \, /, b, f, n, r, t or u)
	ENJsonParserSeqModeString_4hex,		//four hex-digits
	//
	ENJsonParserSeqModeString_Count
} ENJsonParserSeqModeString;

//Number

typedef enum ENJsonParserSeqModeNumber_ {
	ENJsonParserSeqModeNumber_Sign = 0, 	//sign or first digit
	ENJsonParserSeqModeNumber_IntPartFirst, //integer part first digit
	ENJsonParserSeqModeNumber_IntPartSeq,	//integer part second and above digit
	ENJsonParserSeqModeNumber_AfterInt,	//'.' starts fraction, 'e' or 'E' starts exponent
	ENJsonParserSeqModeNumber_FractPart, 	//fraction part (digits)
	ENJsonParserSeqModeNumber_AfterFract,	//'e' or 'E' starts exponent
	ENJsonParserSeqModeNumber_ExpSing,		//'-', '+' or digit
	ENJsonParserSeqModeNumber_ExpPart,		//digits
	//
	ENJsonParserSeqModeNumber_Count
} ENJsonParserSeqModeNumber;

//Array

typedef enum ENJsonParserSeqModeArray_ {
	ENJsonParserSeqModeArray_Whitespace0 = 0, 	//whitespace or ']'
	ENJsonParserSeqModeArray_Value,			//value returned from stack
	ENJsonParserSeqModeArray_AfterValue,		//comma or ']'
	//
	ENJsonParserSeqModeArray_Count
} ENJsonParserSeqModeArray;

//Object

typedef enum ENJsonParserSeqModeObject_ {
	ENJsonParserSeqModeObject_Whitespace0 = 0, //whitespace, d-quote or '}'
	ENJsonParserSeqModeObject_Name,			//string returned from stack
	ENJsonParserSeqModeObject_AfterName,		//whitespace or ':'
	ENJsonParserSeqModeObject_Value,			//value returned from stack
	ENJsonParserSeqModeObject_AfterValue,		//',' or '}'
	ENJsonParserSeqModeObject_AfterComma,		//whitespace or d-quote
	//
	ENJsonParserSeqModeObject_Count
} ENJsonParserSeqModeObject;

//NBJsonParserStr4hex

typedef struct STNBJsonParserStr4hex_ {
	char	hex[8];	//
	UI8		hexUse;	//chars accumulated
	UI8		hexReq;	//chars expected (calculated after first 4)
	UI8		curSeq;	//current sequence use (0 to 4)
} STNBJsonParserStr4hex;

//NBJsonParserSeq

typedef struct STNBJsonParserSeq_ {
	ENJsonParserSeqMode	mode;
	UI32				submode;
	BOOL				doNotNotify;	//parsing internal element
	//details
	union {
		STNBJsonParserStr4hex	str4hex;	//if(mode == ENJsonParserSeqMode_String)
		STNBJsonParserNumDesc	numDesc;	//if(mode == ENJsonParserSeqMode_Number)
	};
} STNBJsonParserSeq;

//Private members

STNBJsonParserSeq* NBJsonParser_seqModePush_(STNBJsonParser* obj, const ENJsonParserSeqMode mode, const BOOL doNotNotify);
STNBJsonParserSeq* NBJsonParser_seqModePop_(STNBJsonParser* obj);
UI32 NBJsonParser_feed_(STNBJsonParser* obj, const void* pBuffer, const SI32 elemsAtbuffer, IJsonParserListener* listener, void* listenerParam, const BOOL ignoreAsFed);

#ifdef NB_CONFIG_INCLUDE_ASSERTS
BOOL NBJsonParser_dbgNumberIsValid_(const STNBJsonParserSeq* seq, const char* str, const UI32 strLen);
#endif

//

void NBJsonParser_init(STNBJsonParser* obj){
	obj->errFnd		= FALSE;
    obj->endFnd     = FALSE;
	obj->bytesFeed	= 0;
    //NBArray_init(&obj->seqModesStack, sizeof(ENJsonParserSeqMode), NULL);
	NBArray_init(&obj->seqsStack, sizeof(STNBJsonParserSeq), NULL);
    NBString_init(&obj->strAcum); //StrFor accumulation of names and values. Will be flushed every ~4Kbs of accumulation.
	NBString_init(&obj->errDesc);	//descriptive parsing error
}

//State Finish

void NBJsonParser_release(STNBJsonParser* obj){
	//Release data
	obj->errFnd		= FALSE;
    obj->endFnd     = FALSE;
	obj->bytesFeed	= 0;
    NBString_release(&obj->strAcum);
	NBArray_release(&obj->seqsStack);
	NBString_release(&obj->errDesc);	//descriptive parsing error
}

//State consume data

BOOL NBJsonParser_feedStart(STNBJsonParser* obj, IJsonParserListener* listener, void* listenerParam){
	obj->errFnd		= FALSE;
    obj->endFnd     = FALSE;
	obj->bytesFeed	= 0;
	NBArray_empty(&obj->seqsStack);
	NBString_empty(&obj->strAcum);
	NBString_empty(&obj->errDesc);
	//
	NBJsonParser_seqModePush_(obj, ENJsonParserSeqMode_Value, FALSE);
	//
	return TRUE;
}

BOOL NBJsonParser_feedIsComplete(STNBJsonParser* obj){
	BOOL r = FALSE;
	if(obj->seqsStack.use == 1){
		const STNBJsonParserSeq* seq = NBArray_itmPtrAtIndex(&obj->seqsStack, STNBJsonParserSeq, obj->seqsStack.use - 1);
		r = (
             !obj->errFnd
             && seq->mode == ENJsonParserSeqMode_Value
             && (
                 seq->submode == ENJsonParserSeqModeValue_Value //inline-value or value was closed (ignoring chars ahead)
                 || seq->submode == ENJsonParserSeqModeValue_WhitespacePost //value was closed and extra-spaces are allowed
                 )
             );
	}
	return r;
}

BOOL NBJsonParser_feedEnd(STNBJsonParser* obj, IJsonParserListener* listener, void* listenerParam){
	BOOL spaceFed = FALSE;
	//Close inner values
	if(!obj->errFnd && obj->seqsStack.use > 1){
		if(!spaceFed){
			NBJsonParser_feed_(obj, " ", 1, listener, listenerParam, TRUE);
			spaceFed = TRUE;
		}
	}
	//Close root-value
	if(!obj->errFnd && obj->seqsStack.use == 1){
		const STNBJsonParserSeq* seq = NBArray_itmPtrAtIndex(&obj->seqsStack, STNBJsonParserSeq, obj->seqsStack.use - 1);
		if(seq->mode == ENJsonParserSeqMode_Value && seq->submode != ENJsonParserSeqModeValue_WhitespacePost){
			if(!spaceFed){
				NBJsonParser_feed_(obj, " ", 1, listener, listenerParam, TRUE);
				spaceFed = TRUE;
			}
		}
	}
	//
	return NBJsonParser_feedIsComplete(obj);
}

UI32 NBJsonParser_feed(STNBJsonParser* obj, const void* pBuffer, const SI32 elemsAtbuffer, IJsonParserListener* listener, void* listenerParam){
	return NBJsonParser_feed_(obj, pBuffer, elemsAtbuffer, listener, listenerParam, FALSE);
}

UI32 NBJsonParser_feed_(STNBJsonParser* obj, const void* pBuffer, const SI32 elemsAtbuffer, IJsonParserListener* listener, void* listenerParam, const BOOL ignoreAsFed){
	UI32 r = 0; const char* cStart = (const char*)pBuffer;
	if(pBuffer != NULL && elemsAtbuffer > 0 && !obj->errFnd && !obj->endFnd){
		if(obj->seqsStack.use < 1){
			obj->errFnd = true;
		} else {
			char cToAdd = '\0';
			const char* c = cStart;
			const char* cAfterEnd = c + elemsAtbuffer;
			STNBJsonParserSeq* seqOwner = NULL;
			STNBJsonParserSeq* seq = NBArray_itmPtrAtIndex(&obj->seqsStack, STNBJsonParserSeq, obj->seqsStack.use - 1);
			while(!obj->errFnd && !obj->endFnd && c < cAfterEnd){
				seqOwner = seq;
				switch (seq->mode) {
					case ENJsonParserSeqMode_Value:
						//---------------------
						//- Value
						//---------------------
						while(seq == seqOwner && c < cAfterEnd){
							switch (seq->submode) {
								case ENJsonParserSeqModeValue_WhitespacePre:
									//pre-value whitespaces
									NBASSERT(c < cAfterEnd && seq->submode == ENJsonParserSeqModeValue_WhitespacePre)
									while(NB_JSON_IS_WHITESPACE(*c) && c < cAfterEnd){
										c++;
									}
									if(c < cAfterEnd){
										if(*c == '\"'){
											//value is string
											seq->submode = ENJsonParserSeqModeValue_Value;
											seq = NBJsonParser_seqModePush_(obj, ENJsonParserSeqMode_String, FALSE);
											c++; break;
										} else if(*c == '-' || (*c >= '0' && *c <= '9')){
											//value is object
											seq->submode = ENJsonParserSeqModeValue_Value;
											seq = NBJsonParser_seqModePush_(obj, ENJsonParserSeqMode_Number, FALSE);
											//do not consume char
											break;
										} else if(*c == '{'){
											//notify-object
											if(!seq->doNotNotify && listener != NULL && listener->objectStarted != NULL){
												//PRINTF_INFO("NBJsonParser, listener objectStarted.\n");
												(*listener->objectStarted)(obj, listenerParam);
											}
											//value is object
											seq->submode = ENJsonParserSeqModeValue_Value;
											seq = NBJsonParser_seqModePush_(obj, ENJsonParserSeqMode_Object, FALSE);
											c++; break;
										} else if(*c == '['){
											//notify-array
											if(!seq->doNotNotify && listener != NULL && listener->arrayStarted != NULL){
												//PRINTF_INFO("NBJsonParser, listener arrayStarted.\n");
												(*listener->arrayStarted)(obj, listenerParam);
											}
											//value is array
											seq->submode = ENJsonParserSeqModeValue_Value;
											seq = NBJsonParser_seqModePush_(obj, ENJsonParserSeqMode_Array, FALSE);
											c++; break;
										} else {
											//plain value
											seq->submode = ENJsonParserSeqModeValue_ValueInline;
										}
									}
									if(c >= cAfterEnd) break;
								case ENJsonParserSeqModeValue_ValueInline:
                                    //accumulate valid inlined-chars
									NBASSERT(c < cAfterEnd && seq->submode == ENJsonParserSeqModeValue_ValueInline)
									while(
                                          c < cAfterEnd && *c > 32 && *c <= 126 && *c != ',' && *c != '{' && *c != '}' && *c != '[' && *c != ']'
                                          && obj->strAcum.length <= 5 //'true', 'null' and 'false' are the only allowed values
                                          ){
										NBString_concatByte(&obj->strAcum, *c);
										c++;
									}
                                    //analyze accumulated inline-value
									if(
                                       (obj->strAcum.length == 4 && (NBString_isEqual(&obj->strAcum, "true") || NBString_isEqual(&obj->strAcum, "null")))
                                       ||
                                       (obj->strAcum.length == 5 && (NBString_isEqual(&obj->strAcum, "false")))
                                       )
                                    {
										//PRINTF_INFO("NBJson, inlined value '%s'.\n", obj->strAcum.str);
										//notify-value
										if(!seq->doNotNotify && listener != NULL && listener->consumePlain != NULL){
											//PRINTF_INFO("NBJsonParser, listener consumePlain.\n");
											(*listener->consumePlain)(obj, obj->strAcum.str, obj->strAcum.length, listenerParam);
										}
										NBString_empty(&obj->strAcum);
										seq->submode = ENJsonParserSeqModeValue_Value;
										//non-break
									} else if(c < cAfterEnd){ //more chars available
										//PRINTF_INFO("NBJson, inlined value '%s'.\n", obj->strAcum.str);
										NBString_concat(&obj->errDesc, "Only 'true', 'false' or 'null' are allowed as inlined values.");
										obj->errFnd = TRUE; seqOwner = NULL;
										break;
									}
									if(c >= cAfterEnd) break;
								case ENJsonParserSeqModeValue_Value:
									//poped from value
									NBASSERT(c < cAfterEnd && seq->submode == ENJsonParserSeqModeValue_Value)
									seq->submode = ENJsonParserSeqModeValue_WhitespacePost;
									if(c >= cAfterEnd) break;
								case ENJsonParserSeqModeValue_WhitespacePost:
									//post-value whitespaces
									NBASSERT(c < cAfterEnd && seq->submode == ENJsonParserSeqModeValue_WhitespacePost)
									while(c < cAfterEnd && NB_JSON_IS_WHITESPACE(*c)){
										c++;
									}
									if(c < cAfterEnd){
                                        seq = NBJsonParser_seqModePop_(obj);
									}
									break;
#								ifdef NB_CONFIG_INCLUDE_ASSERTS
								default:
									NBASSERT(FALSE)	//program logic error
									break;
#								endif
							}
						}
						break;
					case ENJsonParserSeqMode_String:
						//---------------------
						//- String
						//---------------------
						while(seq == seqOwner && c < cAfterEnd){
							switch (seq->submode) {
								case ENJsonParserSeqModeString_AfterSlash:
									//first char after slash (", \, /, b, f, n, r, t or u)
									NBASSERT(c < cAfterEnd && seq->submode == ENJsonParserSeqModeString_AfterSlash)
									cToAdd = '\0';
									switch (*c) {
										case '\"': cToAdd = '\"'; break;
										case '\\': cToAdd = '\\'; break;
										case '/': cToAdd = '/'; break;
										case 'b': cToAdd = '\b'; break;
										case 'f': cToAdd = '\f'; break;
										case 'n': cToAdd = '\n'; break;
										case 'r': cToAdd = '\r'; break;
										case 't': cToAdd = '\t'; break;
										case 'u':
											seq->submode = ENJsonParserSeqModeString_4hex;
											seq->str4hex.curSeq = 0; c++;
											break;
										default:
											NBString_concat(&obj->errDesc, "String expected d-quote, \\, /, b, f, n, r, t or u after '\\'.");
											obj->errFnd = TRUE; seqOwner = NULL;
											break;
									}
									//add char
									if(cToAdd != '\0'){
										if(seq->str4hex.hexUse != 0){
											NBString_concat(&obj->errDesc, "String incomplete 4-hex sequences.");
											obj->errFnd = TRUE; seqOwner = NULL;
										} else {
											NBString_concatByte(&obj->strAcum, cToAdd); c++;
											seq->submode = ENJsonParserSeqModeString_Any;
										}
									}
									break;
								case ENJsonParserSeqModeString_4hex:
									//four hex-digits
									NBASSERT(c < cAfterEnd && seq->submode == ENJsonParserSeqModeString_4hex)
									cToAdd		= *c;
									//add digit
									if((cToAdd >= '0' && cToAdd <= '9') || (cToAdd >= 'a' && cToAdd <= 'f') || (cToAdd >= 'A' && cToAdd <= 'F')){
										seq->str4hex.hex[seq->str4hex.hexUse++] = cToAdd; c++;
										if(seq->str4hex.hexUse % 4 == 0){
											if(NBEncoding_utf16SurrogateIsNonCharacterHex(&seq->str4hex.hex[seq->str4hex.hexUse - 4])){
												//concat non-characters
												NBString_concatBytes(&obj->strAcum, seq->str4hex.hex, seq->str4hex.hexUse);
												seq->str4hex.hexUse = 0;
												seq->submode = ENJsonParserSeqModeString_Any;
											} else {
												UI16 vUtf16[2]		= {0, 0};
												vUtf16[0]			= NBEncoding_utf16SurrogateFromHex(seq->str4hex.hex);
												seq->str4hex.hexReq	= NBEncoding_utf16SurrogatesExpected(vUtf16[0]) * 4;
												if(seq->str4hex.hexReq > 8){
													NBString_concat(&obj->errDesc, "String implementation supports up to two sequential 4-hex.");
													obj->errFnd = TRUE; seqOwner = NULL;
												} else if(seq->str4hex.hexReq == seq->str4hex.hexUse){
													UI32 unicode; char utf8[7];
													if(seq->str4hex.hexUse == 8){
														vUtf16[1] = NBEncoding_utf16SurrogateFromHex(&seq->str4hex.hex[4]);
													}
													unicode = NBEncoding_unicodeFromUtf16(vUtf16, 0xFFFFFFFF);
													if(unicode == 0xFFFFFFFF){
														NBString_concat(&obj->errDesc, "String invalid 4-hex sequence.");
														obj->errFnd = TRUE; seqOwner = NULL;
													} else {
														NBASSERT(unicode == NBEncoding_unicodeFromUtf16s(vUtf16, (seq->str4hex.hexUse == 4 ? 1 : 2), 0))
														NBEncoding_utf8FromUnicode(unicode, utf8);
														NBString_concat(&obj->strAcum, utf8);
														seq->str4hex.hexUse = 0;
														seq->submode = ENJsonParserSeqModeString_Any;
													}
												}
											}
										}
									} else if(seq->str4hex.hexUse == 0 || (seq->str4hex.hexUse % 4) != 0){
										NBString_concat(&obj->errDesc, "String incomplete 4-hexadecimal-chars after '\\u'.");
										obj->errFnd = TRUE; seqOwner = NULL;
									} else {
										seq->submode = ENJsonParserSeqModeString_Any;
									}
									break;
								case ENJsonParserSeqModeString_Any:
									//any codepoint except ", \ or control-chars
									NBASSERT(c < cAfterEnd && seq->submode == ENJsonParserSeqModeString_Any)
									cToAdd = *c;
									if(cToAdd == '\\'){
										seq->submode = ENJsonParserSeqModeString_AfterSlash; c++;
									} else if(seq->str4hex.hexUse != 0){
										NBString_concat(&obj->errDesc, "String incomplete 4-hex sequences.");
										obj->errFnd = TRUE; seqOwner = NULL;
									} else if(cToAdd == '\"'){
										//notify-value
										if(!seq->doNotNotify && listener != NULL && listener->consumeLiteral != NULL){
											//PRINTF_INFO("NBJsonParser, listener consumeLiteral.\n");
											(*listener->consumeLiteral)(obj, obj->strAcum.str, obj->strAcum.length, listenerParam);
										}
										//end-of-string
                                        seq = NBJsonParser_seqModePop_(obj); c++;
									} else if(NB_JSON_IS_FORBIDDEN_STR_CHAR(cToAdd)){
										NBString_concat(&obj->errDesc, "String forbids control-chars.");
										obj->errFnd = TRUE; seqOwner = NULL; 
									} else {
										NBString_concatByte(&obj->strAcum, cToAdd); c++;
										while(c < cAfterEnd && *c != '\\' && *c != '\"' && !NB_JSON_IS_FORBIDDEN_STR_CHAR(*c)){
											NBString_concatByte(&obj->strAcum, *c); c++;
										}
									}
									break;
								default:
									NBASSERT(FALSE)
									break;
							}
						}
						break;
					case ENJsonParserSeqMode_Number:
						//---------------------
						//- Number
						//---------------------
						while(seq == seqOwner && c < cAfterEnd){
							switch (seq->submode) {
								case ENJsonParserSeqModeNumber_Sign:
									//sign or first digit
									NBASSERT(c < cAfterEnd && seq->submode == ENJsonParserSeqModeNumber_Sign)
									cToAdd = *c;
									if(cToAdd == '-'){
										seq->numDesc.iPart.signChars++;
										NBString_concatByte(&obj->strAcum, cToAdd); c++;
										seq->submode = ENJsonParserSeqModeNumber_IntPartFirst;
										//non-break;
									} else if(cToAdd == '0'){
										seq->numDesc.iPart.valChars++;
										NBString_concatByte(&obj->strAcum, cToAdd); c++;
										seq->submode = ENJsonParserSeqModeNumber_AfterInt;
										break;
									} else if(cToAdd > '0' && cToAdd <= '9'){
										seq->numDesc.iPart.valChars++;
										NBString_concatByte(&obj->strAcum, cToAdd); c++;
										seq->submode = ENJsonParserSeqModeNumber_IntPartSeq;
										break;
									} else {
										NBString_concat(&obj->errDesc, "Number expected '-', or digit at beginning.");
										obj->errFnd = TRUE; seqOwner = NULL;
										break;
									}
									if(c >= cAfterEnd) break;
								case ENJsonParserSeqModeNumber_IntPartFirst:
									//integer part (digits)
									NBASSERT(c < cAfterEnd && seq->submode == ENJsonParserSeqModeNumber_IntPartFirst)
									cToAdd = *c;
									if(cToAdd == '0'){
										seq->numDesc.iPart.valChars++;
										NBString_concatByte(&obj->strAcum, cToAdd); c++;
										seq->submode = ENJsonParserSeqModeNumber_AfterInt;
										break;
									} else if(cToAdd > '0' && cToAdd <= '9'){
										seq->numDesc.iPart.valChars++;
										NBString_concatByte(&obj->strAcum, cToAdd); c++;
										seq->submode = ENJsonParserSeqModeNumber_IntPartSeq;
										//non-break;
									} else {
										NBString_concat(&obj->errDesc, "Number expected digit as interger part.");
										obj->errFnd = TRUE; seqOwner = NULL;
										break;
									}
									if(c >= cAfterEnd) break;
								case ENJsonParserSeqModeNumber_IntPartSeq:
									//integer part (digits)
									NBASSERT(c < cAfterEnd && seq->submode == ENJsonParserSeqModeNumber_IntPartSeq)
									while(c < cAfterEnd && *c >= '0' && *c <= '9'){
										seq->numDesc.iPart.valChars++;
										NBString_concatByte(&obj->strAcum, *c); c++;
									}
									if(c < cAfterEnd){
										NBASSERT(seq->numDesc.iPart.valChars > 0)
										seq->submode = ENJsonParserSeqModeNumber_AfterInt;
										//non-break
									} else {
										break;
									}
								case ENJsonParserSeqModeNumber_AfterInt:
									//'.' starts fraction, 'e' or 'E' starts exponent
									NBASSERT(c < cAfterEnd && seq->submode == ENJsonParserSeqModeNumber_AfterInt)
									if(*c == '.'){
										seq->numDesc.fPart.dotChars++;
										NBString_concatByte(&obj->strAcum, *c); c++;
										seq->submode = ENJsonParserSeqModeNumber_FractPart;
									} else if(*c == 'e' || *c == 'E'){
										seq->numDesc.ePart.eChars++;
										NBString_concatByte(&obj->strAcum, *c); c++;
										seq->submode = ENJsonParserSeqModeNumber_ExpSing;
										break;
									} else {
										//notify-value
										if(!seq->doNotNotify && listener != NULL){
											if(listener->consumeNumber != NULL){
												//PRINTF_INFO("NBJsonParser, listener consumeNumber.\n");
												(*listener->consumeNumber)(obj, obj->strAcum.str, &seq->numDesc, listenerParam);
											} else if(listener->consumePlain != NULL){
												//PRINTF_INFO("NBJsonParser, listener consumePlain.\n");
												(*listener->consumePlain)(obj, obj->strAcum.str, obj->strAcum.length, listenerParam);
											}
										}
										//pop
										NBASSERT(NBJsonParser_dbgNumberIsValid_(seq, obj->strAcum.str, obj->strAcum.length))
                                        seq = NBJsonParser_seqModePop_(obj);
										break;
									}
									if(c >= cAfterEnd) break;
								case ENJsonParserSeqModeNumber_FractPart:
									//fraction part (digits)
									NBASSERT(c < cAfterEnd && seq->submode == ENJsonParserSeqModeNumber_FractPart)
									while(c < cAfterEnd && *c >= '0' && *c <= '9'){
										seq->numDesc.fPart.valChars++;
										NBString_concatByte(&obj->strAcum, *c); c++;
									}
									if(c < cAfterEnd){
										if(seq->numDesc.fPart.valChars > 0){
											seq->submode = ENJsonParserSeqModeNumber_AfterFract;
										} else {
											NBString_concat(&obj->errDesc, "Number expected digits after '.'.");
											obj->errFnd = TRUE; seqOwner = NULL;
											break;
										}
										//non-break
									} else {
										break;
									}
								case ENJsonParserSeqModeNumber_AfterFract:
									//'e' or 'E' starts exponent
									NBASSERT(c < cAfterEnd && seq->submode == ENJsonParserSeqModeNumber_AfterFract)
									if(*c == 'e' || *c == 'E'){
										seq->numDesc.ePart.eChars++;
										NBString_concatByte(&obj->strAcum, *c); c++;
										seq->submode = ENJsonParserSeqModeNumber_ExpSing;
										break;
									} else {
										//notify-value
										if(!seq->doNotNotify && listener != NULL){
											if(listener->consumeNumber != NULL){
												//PRINTF_INFO("NBJsonParser, listener consumeNumber.\n");
												(*listener->consumeNumber)(obj, obj->strAcum.str, &seq->numDesc, listenerParam);
											} else if(listener->consumePlain != NULL){
												//PRINTF_INFO("NBJsonParser, listener consumePlain.\n");
												(*listener->consumePlain)(obj, obj->strAcum.str, obj->strAcum.length, listenerParam);
											}
										}
										//pop
										NBASSERT(NBJsonParser_dbgNumberIsValid_(seq, obj->strAcum.str, obj->strAcum.length))
                                        seq = NBJsonParser_seqModePop_(obj);
										break;
									}
								case ENJsonParserSeqModeNumber_ExpSing:
									//'-', '+' or digit
									NBASSERT(c < cAfterEnd && seq->submode == ENJsonParserSeqModeNumber_ExpSing)
									if(*c == '-' || *c == '+'){
										seq->numDesc.ePart.signChars++;
										NBString_concatByte(&obj->strAcum, *c); c++;
									}
									seq->submode = ENJsonParserSeqModeNumber_ExpPart;
									if(c >= cAfterEnd) break;
								case ENJsonParserSeqModeNumber_ExpPart:
									//digits
									NBASSERT(c < cAfterEnd && seq->submode == ENJsonParserSeqModeNumber_ExpPart)
									while(c < cAfterEnd && *c >= '0' && *c <= '9'){
										seq->numDesc.ePart.valChars++;
										NBString_concatByte(&obj->strAcum, *c); c++;
									}
									if(c < cAfterEnd){
										if(seq->numDesc.ePart.valChars > 0){
											//notify-value
											if(!seq->doNotNotify && listener != NULL){
												if(listener->consumeNumber != NULL){
													//PRINTF_INFO("NBJsonParser, listener consumeNumber.\n");
													(*listener->consumeNumber)(obj, obj->strAcum.str, &seq->numDesc, listenerParam);
												} else if(listener->consumePlain != NULL){
													//PRINTF_INFO("NBJsonParser, listener consumePlain.\n");
													(*listener->consumePlain)(obj, obj->strAcum.str, obj->strAcum.length, listenerParam);
												}
											}
											//pop
											NBASSERT(NBJsonParser_dbgNumberIsValid_(seq, obj->strAcum.str, obj->strAcum.length))
                                            seq = NBJsonParser_seqModePop_(obj);
										} else {
											NBString_concat(&obj->errDesc, "Number expected digits after 'e/E[+/-]'.");
											obj->errFnd = TRUE; seqOwner = NULL;
											break;
										}
									}
									break;
#								ifdef NB_CONFIG_INCLUDE_ASSERTS
								default:
									NBASSERT(FALSE) //program logic error
									break;
#								endif
							}
						}
						break;
					case ENJsonParserSeqMode_Object:
						//---------------------
						//- Object
						//---------------------
						while(seq == seqOwner && c < cAfterEnd){
							switch (seq->submode) {
								case ENJsonParserSeqModeObject_Whitespace0:
									//whitespace, d-quote or '}'
									NBASSERT(c < cAfterEnd && seq->submode == ENJsonParserSeqModeObject_Whitespace0)
									while(NB_JSON_IS_WHITESPACE(*c) && c < cAfterEnd){
										c++;
									}
									if(c < cAfterEnd){
										if(*c == '}'){
											//notify-object
											if(!seq->doNotNotify && listener != NULL && listener->objectEnded != NULL){
												//PRINTF_INFO("NBJsonParser, listener objectEnded.\n");
												(*listener->objectEnded)(obj, listenerParam);
											}
											//pop
											seq = NBJsonParser_seqModePop_(obj); c++;
										} else if(*c == '\"'){
											//notify-member
											if(!seq->doNotNotify && listener != NULL && listener->memberStarted != NULL){
												//PRINTF_INFO("NBJsonParser, listener memberStarted.\n");
												(*listener->memberStarted)(obj, listenerParam);
											}
											seq->submode = ENJsonParserSeqModeObject_Name;
											seq = NBJsonParser_seqModePush_(obj, ENJsonParserSeqMode_String, TRUE);
											c++;
										} else {
											NBString_concat(&obj->errDesc, "Object expected '\"' or '}' at beginning.");
											obj->errFnd = TRUE; seqOwner = NULL;
										}
									}
									break;
								case ENJsonParserSeqModeObject_Name:
									//string returned from stack
									NBASSERT(c < cAfterEnd && seq->submode == ENJsonParserSeqModeObject_Name)
									//notify-name
									if(!seq->doNotNotify && listener != NULL && listener->consumeName != NULL && obj->strAcum.length > 0){
										//PRINTF_INFO("NBJsonParser, listener consumeName('%s').\n", obj->strAcum.str);
										(*listener->consumeName)(obj, obj->strAcum.str, listenerParam);
									}
									NBString_empty(&obj->strAcum);
									seq->submode = ENJsonParserSeqModeObject_AfterName;
									if(c >= cAfterEnd) break;
								case ENJsonParserSeqModeObject_AfterName:
									//whitespace or ':'
									NBASSERT(c < cAfterEnd && seq->submode == ENJsonParserSeqModeObject_AfterName)
									while(NB_JSON_IS_WHITESPACE(*c) && c < cAfterEnd){
										c++;
									}
									if(c < cAfterEnd){
										if(*c == ':'){
											seq->submode = ENJsonParserSeqModeObject_Value;
											seq = NBJsonParser_seqModePush_(obj, ENJsonParserSeqMode_Value, FALSE);
											c++;
										} else {
											NBString_concat(&obj->errDesc, "Object expected ':' after value-name.");
											obj->errFnd = TRUE; seqOwner = NULL;
										}
									}
									break;
								case ENJsonParserSeqModeObject_Value:
									//value returned from stack
									NBASSERT(c < cAfterEnd && seq->submode == ENJsonParserSeqModeObject_Value)
									seq->submode = ENJsonParserSeqModeObject_AfterValue;
									//notify-member
									if(!seq->doNotNotify && listener != NULL && listener->memberEnded != NULL){
										//PRINTF_INFO("NBJsonParser, listener memberEnded.\n");
										(*listener->memberEnded)(obj, listenerParam);
									}
									if(c >= cAfterEnd) break;
								case ENJsonParserSeqModeObject_AfterValue:
									//',' or '}'
									NBASSERT(c < cAfterEnd && seq->submode == ENJsonParserSeqModeObject_AfterValue)
									if(*c == ','){
										seq->submode = ENJsonParserSeqModeObject_AfterComma; c++;
										//non break
									} else if(*c == '}'){
										//notify-object
										if(!seq->doNotNotify && listener != NULL && listener->objectEnded != NULL){
											//PRINTF_INFO("NBJsonParser, listener objectEnded.\n");
											(*listener->objectEnded)(obj, listenerParam);
										}
										//pop
										seq = NBJsonParser_seqModePop_(obj); c++;
										break;
									} else {
										NBString_concat(&obj->errDesc, "Object expected ',' or '}' after value.");
										obj->errFnd = TRUE; seqOwner = NULL;
										break;
									}
									if(c >= cAfterEnd) break;
								case ENJsonParserSeqModeObject_AfterComma:
									//whitespace or d-quote
									NBASSERT(c < cAfterEnd && seq->submode == ENJsonParserSeqModeObject_AfterComma)
									while(NB_JSON_IS_WHITESPACE(*c) && c < cAfterEnd){
										c++;
									}
									if(c < cAfterEnd){
										if(*c == '\"'){
											//notify-member
											if(!seq->doNotNotify && listener != NULL && listener->memberStarted != NULL){
												//PRINTF_INFO("NBJsonParser, listener memberStarted.\n");
												(*listener->memberStarted)(obj, listenerParam);
											}
											seq->submode = ENJsonParserSeqModeObject_Name;
											seq = NBJsonParser_seqModePush_(obj, ENJsonParserSeqMode_String, TRUE);
											c++;
										} else {
											NBString_concat(&obj->errDesc, "Object expected '\"' after comma.");
											obj->errFnd = TRUE; seqOwner = NULL;
										}
									}
									break;
								default:
									break;
							}
						}
						break;
					case ENJsonParserSeqMode_Array:
						//---------------------
						//- Array
						//---------------------
						while(seq == seqOwner && c < cAfterEnd){
							switch (seq->submode) {
								case ENJsonParserSeqModeArray_Whitespace0:
									//whitespace or ']'
									NBASSERT(c < cAfterEnd && seq->submode == ENJsonParserSeqModeArray_Whitespace0)
									while(NB_JSON_IS_WHITESPACE(*c) && c < cAfterEnd){
										c++;
									}
									if(c < cAfterEnd){
										if(*c == ']'){
											//notify-array
											if(!seq->doNotNotify && listener != NULL && listener->arrayEnded != NULL){
												//PRINTF_INFO("NBJsonParser, listener arrayEnded.\n");
												(*listener->arrayEnded)(obj, listenerParam);
											}
											//pop
											seq = NBJsonParser_seqModePop_(obj); c++;
										} else {
											//notify-member
											if(!seq->doNotNotify && listener != NULL && listener->memberStarted != NULL){
												//PRINTF_INFO("NBJsonParser, listener memberStarted.\n");
												(*listener->memberStarted)(obj, listenerParam);
											}
											seq->submode = ENJsonParserSeqModeArray_Value;
											seq = NBJsonParser_seqModePush_(obj, ENJsonParserSeqMode_Value, FALSE);
										}
									}
									break;
								case ENJsonParserSeqModeArray_Value:
									//notify-member
									if(!seq->doNotNotify && listener != NULL && listener->memberEnded != NULL){
										//PRINTF_INFO("NBJsonParser, listener memberEnded.\n");
										(*listener->memberEnded)(obj, listenerParam);
									}
									//poped from value
									NBASSERT(c < cAfterEnd && seq->submode == ENJsonParserSeqModeArray_Value)
									seq->submode = ENJsonParserSeqModeArray_AfterValue;
									if(c >= cAfterEnd) break;
								case ENJsonParserSeqModeArray_AfterValue:
									NBASSERT(c < cAfterEnd && seq->submode == ENJsonParserSeqModeArray_AfterValue)
									if(*c == ']'){
										//notify-array
										if(!seq->doNotNotify && listener != NULL && listener->arrayEnded != NULL){
											//PRINTF_INFO("NBJsonParser, listener arrayEnded.\n");
											(*listener->arrayEnded)(obj, listenerParam);
										}
                                        seq = NBJsonParser_seqModePop_(obj); c++;
									} else if(*c == ','){
										//notify-member
										if(!seq->doNotNotify && listener != NULL && listener->memberStarted != NULL){
											//PRINTF_INFO("NBJsonParser, listener memberStarted.\n");
											(*listener->memberStarted)(obj, listenerParam);
										}
										seq->submode = ENJsonParserSeqModeArray_Value;
										seq = NBJsonParser_seqModePush_(obj, ENJsonParserSeqMode_Value, FALSE);
										c++;
									} else {
										NBString_concat(&obj->errDesc, "Array expected ',' or ']' after value.");
										obj->errFnd = TRUE; seqOwner = NULL;
									}
									break;
#								ifdef NB_CONFIG_INCLUDE_ASSERTS
								default:
									NBASSERT(FALSE)	//program logic error
									break;
#								endif
							}
						}
						break;
					default:
						NBASSERT(FALSE)	//program logic error
						break;
				}
			}
			//result
			r = (UI32)(c - cStart);
			//results update
			if(!ignoreAsFed){
				//count bytes feed
				obj->bytesFeed += r;
				//concat error position
				if(obj->errFnd){
					const SI32 cRemain = elemsAtbuffer - r;
					//add "..."
					NBString_concat(&obj->errDesc, "...");
					//add already parsed portion
					if(r >= 32){
						NBString_concatBytes(&obj->errDesc, &cStart[r - 32], 32);
					} else if(r > 0){
						NBString_concatBytes(&obj->errDesc, cStart, r);
					}
					NBString_concat(&obj->errDesc, "(<-here)");
					//add remain portion
					if(cRemain >= 32){
						NBString_concatBytes(&obj->errDesc, &cStart[r], 32);
					} else if(cRemain > 0){
						NBString_concatBytes(&obj->errDesc, &cStart[r], cRemain);
					}
					NBString_concat(&obj->errDesc, "...");
				}
			}
		}
	}
	return r;
}

//private

STNBJsonParserSeq* NBJsonParser_seqModePush_(STNBJsonParser* obj, const ENJsonParserSeqMode mode, const BOOL doNotNotify){
	STNBJsonParserSeq seq;
	NBMemory_setZeroSt(seq, STNBJsonParserSeq);
	seq.mode = mode;
	seq.doNotNotify = doNotNotify;
	//
	NBString_empty(&obj->strAcum);
	//
	return NBArray_addValue(&obj->seqsStack, seq);
}

STNBJsonParserSeq* NBJsonParser_seqModePop_(STNBJsonParser* obj){
	STNBJsonParserSeq* r = NULL;
	//Remove top mode
	NBASSERT(obj->seqsStack.use > 0) //program logic error
	if(obj->seqsStack.use > 1){
		NBArray_removeItemAtIndex(&obj->seqsStack, obj->seqsStack.use - 1);
		r = NBArray_itmPtrAtIndex(&obj->seqsStack, STNBJsonParserSeq, obj->seqsStack.use - 1);
    } else if(!obj->endFnd){
        obj->endFnd = TRUE;
    } else {
        NBString_concat(&obj->errDesc, "Garbage after root-value node.");
		obj->errFnd = TRUE;
	}
	return r;
}

#ifdef NB_CONFIG_INCLUDE_ASSERTS
BOOL NBJsonParser_dbgNumberIsValid_(const STNBJsonParserSeq* seq, const char* str, const UI32 strLen){
	if(seq->mode != ENJsonParserSeqMode_Number){
		return FALSE;
	} else if(
			  (seq->numDesc.iPart.signChars + seq->numDesc.iPart.valChars
			  + seq->numDesc.fPart.dotChars + seq->numDesc.fPart.valChars
			  + seq->numDesc.ePart.signChars + + seq->numDesc.ePart.eChars + + seq->numDesc.ePart.valChars)
			  != strLen
			  )
	{
		return FALSE;
	}
	return TRUE;
}
#endif


