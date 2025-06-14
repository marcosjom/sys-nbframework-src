//
//  XUXml.c
//  XUMonitorLogicaNegocios_OSX
//
//  Created by Marcos Ortega on 21/01/14.
//  Copyright (c) 2014 XurpasLatam. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBPlistOldParser.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBEncoding.h"

//Private members

ENPlistOldParserSeqMode NBPlistOldParser_privStateSeqModePush(STNBPlistOldParser* state, const ENPlistOldParserSeqMode newMode, const BOOL emptyAccum);
ENPlistOldParserSeqMode NBPlistOldParser_privStateSeqModePop(STNBPlistOldParser* state, const BOOL emptyAccum);
ENPlistOldParserSeqMode NBPlistOldParser_privConsumeLiteralSlashChar(STNBPlistOldParser* state, const char curChar, const ENPlistOldParserSeqMode pCurSeqMode, const BOOL emptyAccumAfter);

//

void NBPlistOldParser_init(STNBPlistOldParser* state){
	state->fmtLogicErr			= FALSE;
	state->prevCharPend			= '\0';
	state->strAccumSeparators	= 0;
    NBArray_init(&state->seqModesStack, sizeof(ENPlistOldParserSeqMode), NULL);
    NBString_init(&state->strAcum);
	NBString_init(&state->strComment);
    NBString_init(&state->literalSChars);
}

//State Finish

void NBPlistOldParser_release(STNBPlistOldParser* state){
	//Release data
	state->fmtLogicErr			= FALSE;
	state->prevCharPend			= '\0';
	state->strAccumSeparators	= 0;
    NBString_release(&state->literalSChars);
	NBString_release(&state->strComment);
    NBString_release(&state->strAcum);
    NBArray_release(&state->seqModesStack);
}

//State consume data

typedef enum ENPlistOldParserStrType_ {
	ENPlistOldParserStrType_Plain = 0,			//Text with no quotes
	ENPlistOldParserStrType_Literal,			//Text inside quotes
	ENPlistOldParserStrType_Hexadec				//Text hexadecimal
} ENPlistOldParserStrType;

void NBPlistOldParser_feedStr(STNBPlistOldParser* state, ENPlistOldParserSeqMode* curSeqMode, const ENPlistOldParserStrType strType, const char* str, const UI32 strSz, IPlistOldParserListener* listener, void* listenerParam){
	NBASSERT(*curSeqMode != ENPlistOldParserSeqMode_CommentMono && *curSeqMode != ENPlistOldParserSeqMode_CommentMulti)
	switch(*curSeqMode){
		case ENPlistOldParserSeqMode_Value:
			if(state->strAccumSeparators != 0){
				//PRINTF_ERROR("PLIST logic ERROR: two consecutive strings found.\n");
				state->fmtLogicErr = TRUE; NBASSERT(FALSE)
			} else {
				//Consume string
				if(listener != NULL && state->strAcum.length > 0){
					if(strType == ENPlistOldParserStrType_Plain){
						(listener->consumePlain)(state, state->strAcum.str, state->strAcum.length, listenerParam);
					} else if(strType == ENPlistOldParserStrType_Literal){
						(listener->consumeLiteral)(state, state->strAcum.str, state->strAcum.length, listenerParam);
					} else if(strType == ENPlistOldParserStrType_Hexadec){
						(listener->consumeHexData)(state, state->strAcum.str, state->strAcum.length, listenerParam);
					} else {
						NBASSERT(FALSE) //Program logic error
					}
				}
			}
			break;
		case ENPlistOldParserSeqMode_Any:
			if(state->strAccumSeparators != 0){
				//PRINTF_ERROR("PLIST logic ERROR: two consecutive strings found.\n");
				state->fmtLogicErr = TRUE; NBASSERT(FALSE)
			} else {
				if(state->seqModesStack.use < 2){
					//PRINTF_ERROR("PLIST logic ERROR: string without parent node.\n");
					state->fmtLogicErr = TRUE; NBASSERT(FALSE)
				} else {
					const ENPlistOldParserSeqMode parSeqMode = *((ENPlistOldParserSeqMode*)NBArray_itemAtIndex(&state->seqModesStack, state->seqModesStack.use - 2));
					switch(parSeqMode){
						case ENPlistOldParserSeqMode_Object:
							//Open new member and consume string
							if(listener != NULL){
								(listener->memberStarted)(state, listenerParam);
								if(state->strAcum.length > 0){
									(listener->consumeName)(state, state->strAcum.str, listenerParam);
								}
							}
							break;
						case ENPlistOldParserSeqMode_Array:
							//Open new member and consume string
							if(listener != NULL){
								(listener->memberStarted)(state, listenerParam);
								if(strType == ENPlistOldParserStrType_Plain){
									(listener->consumePlain)(state, state->strAcum.str, state->strAcum.length, listenerParam);
								} else if(strType == ENPlistOldParserStrType_Literal){
									(listener->consumeLiteral)(state, state->strAcum.str, state->strAcum.length, listenerParam);
								} else if(strType == ENPlistOldParserStrType_Hexadec){
									(listener->consumeHexData)(state, state->strAcum.str, state->strAcum.length, listenerParam);
								} else {
									NBASSERT(FALSE) //Program logic error
								}
							}
							break;
						default:
							//PRINTF_ERROR("PLIST logic ERROR: expected string at object or array level.\n");
							state->fmtLogicErr = TRUE; NBASSERT(FALSE)
							break;
					}
				}
			}
			break;
		default:
			//PRINTF_ERROR("PLIST logic ERROR: unexpected string location.\n");
			state->fmtLogicErr = TRUE; NBASSERT(FALSE)
			break;
	}
}

void NBPlistOldParser_feedChar(STNBPlistOldParser* state, ENPlistOldParserSeqMode* curSeqMode, const char curChar, IPlistOldParserListener* listener, void* listenerParam){
	//Process char
	const BOOL isSpace = (curChar == ' ' || curChar == '\t' || curChar == '\r' || curChar == '\n');
	NBASSERT(*curSeqMode != ENPlistOldParserSeqMode_CommentMono && *curSeqMode != ENPlistOldParserSeqMode_CommentMulti)
	switch(*curSeqMode){
		case ENPlistOldParserSeqMode_HexData:
			if(state->strAccumSeparators != 0){
				//PRINTF_ERROR("PLIST logic ERROR: multiples value-strings found '%c'.\n");
				state->fmtLogicErr = TRUE; NBASSERT(FALSE)
			} else if(curChar == '>'){
				//Consume string and pop mode
				*curSeqMode = NBPlistOldParser_privStateSeqModePop(state, FALSE);
				NBPlistOldParser_feedStr(state, curSeqMode, ENPlistOldParserStrType_Hexadec, state->strAcum.str, state->strAcum.length, listener, listenerParam);
				NBString_empty(&state->strAcum); state->strAccumSeparators++;
			} else if((curChar > 47 && curChar < 58) /*0-9*/ || (curChar > 64 && curChar < 71) /*A-F*/ || (curChar > 96 && curChar < 103) /*a-f*/){
				//Consume value portion (avoid big buffer)
				if(state->strAcum.length >= 1024){
					NBPlistOldParser_feedStr(state, curSeqMode, ENPlistOldParserStrType_Hexadec, state->strAcum.str, state->strAcum.length, listener, listenerParam);
					NBString_empty(&state->strAcum);
				}
				NBString_concatByte(&state->strAcum, curChar);
			} else if(!isSpace){ //Ignore spaces
				//PRINTF_ERROR("PLIST logic ERROR: '%c' found when expecting only hex-char-or-space.\n", curChar);
				state->fmtLogicErr = TRUE; NBASSERT(FALSE)
			}
			break;
		case ENPlistOldParserSeqMode_Literal:
			if(state->strAccumSeparators != 0){
				//PRINTF_ERROR("PLIST logic ERROR: multiples value-strings found.\n");
				state->fmtLogicErr = TRUE; NBASSERT(FALSE)
			} else if(curChar == '\\'){
				//Starting a special sequence in a literal
				NBString_empty(&state->literalSChars);
				*curSeqMode = NBPlistOldParser_privStateSeqModePush(state, ENPlistOldParserSeqMode_LiteralSChar, FALSE);
			} else if(curChar == '\"'){
				//Consume string and pop mode
				*curSeqMode = NBPlistOldParser_privStateSeqModePop(state, FALSE);
				NBPlistOldParser_feedStr(state, curSeqMode, ENPlistOldParserStrType_Literal, state->strAcum.str, state->strAcum.length, listener, listenerParam);
				NBString_empty(&state->strAcum); state->strAccumSeparators++;
			} else {
				//Consume value portion (avoid big buffer)
				if(state->strAcum.length >= 1024){
					NBPlistOldParser_feedStr(state, curSeqMode, ENPlistOldParserStrType_Literal, state->strAcum.str, state->strAcum.length, listener, listenerParam);
					NBString_empty(&state->strAcum);
				}
				NBString_concatByte(&state->strAcum, curChar);
			}
			break;
		case ENPlistOldParserSeqMode_LiteralSChar:
			//Something after special slash in a literal string
			//" ... \" \\ \/ \b \f \n \r \t \uFFFF ..."
			*curSeqMode = NBPlistOldParser_privConsumeLiteralSlashChar(state, curChar, *curSeqMode, FALSE);
			break;
		case ENPlistOldParserSeqMode_Any:
			switch (curChar) {
				case '=':
					//Started an object, array or value
					if(state->seqModesStack.use < 2){
						//PRINTF_ERROR("PLIST logic ERROR: something without parent node.\n");
						state->fmtLogicErr = TRUE; NBASSERT(FALSE)
					} else {
						const ENPlistOldParserSeqMode parSeqMode = *((ENPlistOldParserSeqMode*)NBArray_itemAtIndex(&state->seqModesStack, state->seqModesStack.use - 2));
						if(parSeqMode != ENPlistOldParserSeqMode_Object){
							//PRINTF_ERROR("PLIST logic ERROR: member's name found outside an object.\n");
							state->fmtLogicErr = TRUE; NBASSERT(FALSE)
						} else {
							//Consume name if necesary
							if(state->strAccumSeparators == 0){
								if(state->strAcum.length == 0){
									//PRINTF_ERROR("PLIST logic ERROR: expectd a name before '='.\n");
									state->fmtLogicErr = TRUE; NBASSERT(FALSE)
								} else {
									//Flush current word and start spaces count.
									NBPlistOldParser_feedStr(state, curSeqMode, ENPlistOldParserStrType_Plain, state->strAcum.str, state->strAcum.length, listener, listenerParam);
									NBString_empty(&state->strAcum); state->strAccumSeparators++;
								}
							}
							//Start value
							if(state->strAccumSeparators == 0){
								//PRINTF_ERROR("PLIST logic ERROR: expectd a name before '='.\n");
								state->fmtLogicErr = TRUE; NBASSERT(FALSE)
							} else {
								//Replace 'any' with 'value'
								*curSeqMode = NBPlistOldParser_privStateSeqModePop(state, TRUE);
								*curSeqMode = NBPlistOldParser_privStateSeqModePush(state, ENPlistOldParserSeqMode_Value, TRUE);
							}
						}
					}
					break;
				case '<':
					if(state->strAccumSeparators != 0){
						//PRINTF_ERROR("PLIST logic ERROR: unexpected aditional content after after word.\n");
						state->fmtLogicErr = TRUE; NBASSERT(FALSE)
					} else {
						*curSeqMode = NBPlistOldParser_privStateSeqModePush(state, ENPlistOldParserSeqMode_HexData, TRUE);
					}
					break;
				case '\"':
					if(state->strAccumSeparators != 0){
						//PRINTF_ERROR("PLIST logic ERROR: unexpected aditional content after after word.\n");
						state->fmtLogicErr = TRUE; NBASSERT(FALSE)
					} else {
						*curSeqMode = NBPlistOldParser_privStateSeqModePush(state, ENPlistOldParserSeqMode_Literal, TRUE);
					}
					break;
				case '{': case '(':
					//Started an object, array or value
					if(state->seqModesStack.use < 2){
						//PRINTF_ERROR("PLIST logic ERROR: something without parent node.\n");
						state->fmtLogicErr = TRUE; NBASSERT(FALSE)
					} else {
						const ENPlistOldParserSeqMode parSeqMode = *((ENPlistOldParserSeqMode*)NBArray_itemAtIndex(&state->seqModesStack, state->seqModesStack.use - 2));
						switch(parSeqMode){
							case ENPlistOldParserSeqMode_Array:
								//Starting a value inside an array
								//Replace 'any' with 'any' mode
								*curSeqMode = NBPlistOldParser_privStateSeqModePop(state, FALSE);
								*curSeqMode = NBPlistOldParser_privStateSeqModePush(state, ENPlistOldParserSeqMode_Any, TRUE);
								if(listener != NULL){
									(listener->memberStarted)(state, listenerParam);
								}
								if(curChar == '{'){
									*curSeqMode = NBPlistOldParser_privStateSeqModePush(state, ENPlistOldParserSeqMode_Object, TRUE);
									if(listener != NULL){
										(listener->objectStarted)(state, listenerParam);
									}
								} else if(curChar == '('){
									*curSeqMode = NBPlistOldParser_privStateSeqModePush(state, ENPlistOldParserSeqMode_Array, TRUE);
									if(listener != NULL){
										(listener->arrayStarted)(state, listenerParam);
									}
								} else {
									NBASSERT(FALSE) //Program logic error
								}
								break;
							default:
								NBASSERT(FALSE) //Program logic error
								break;
						}
					}
					break;
				case ';': //Allways expected after a 'value' sequence
				case '>': //Allways expected inside a 'hexString' sequence
					//PRINTF_ERROR("PLIST logic ERROR: forbidden char('%c') found at name.\n", curChar);
					state->fmtLogicErr = TRUE; NBASSERT(FALSE)
					break;
				case ',':
					//Started an object, array or value
					if(state->seqModesStack.use < 2){
						//PRINTF_ERROR("PLIST logic ERROR: something without parent node.\n");
						state->fmtLogicErr = TRUE; NBASSERT(FALSE)
					} else {
						const ENPlistOldParserSeqMode parSeqMode = *((ENPlistOldParserSeqMode*)NBArray_itemAtIndex(&state->seqModesStack, state->seqModesStack.use - 2));
						if(parSeqMode != ENPlistOldParserSeqMode_Array){
							//PRINTF_ERROR("PLIST logic ERROR: member's name found outside an array.\n");
							state->fmtLogicErr = TRUE; NBASSERT(FALSE)
						} else {
							//Consume name if necesary
							if(state->strAccumSeparators == 0){
								if(state->strAcum.length == 0){
									//PRINTF_ERROR("PLIST logic ERROR: expectd a name before '='.\n");
									state->fmtLogicErr = TRUE; NBASSERT(FALSE)
								} else {
									//Flush current word and start spaces count.
									NBPlistOldParser_feedStr(state, curSeqMode, ENPlistOldParserStrType_Plain, state->strAcum.str, state->strAcum.length, listener, listenerParam);
									NBString_empty(&state->strAcum); state->strAccumSeparators++;
								}
							}
							//Close member
							if(listener != NULL && state->strAccumSeparators != 0){
								(listener->memberEnded)(state, listenerParam);
							}
							//Replace 'any' with a new 'any'
							*curSeqMode = NBPlistOldParser_privStateSeqModePop(state, TRUE);
							*curSeqMode = NBPlistOldParser_privStateSeqModePush(state, ENPlistOldParserSeqMode_Any, TRUE);
						}
					}
					break;
				case '}':
					if(state->strAcum.length != 0 || state->strAccumSeparators != 0){
						//PRINTF_ERROR("PLIST logic ERROR: found '}' after member name with no value.\n", curChar);
						state->fmtLogicErr = TRUE; NBASSERT(FALSE)
					} else {
						if(state->seqModesStack.use < 2){
							//PRINTF_ERROR("PLIST logic ERROR: something without parent node.\n");
							state->fmtLogicErr = TRUE; NBASSERT(FALSE)
						} else {
							const ENPlistOldParserSeqMode parSeqMode = *((ENPlistOldParserSeqMode*)NBArray_itemAtIndex(&state->seqModesStack, state->seqModesStack.use - 2));
							if(parSeqMode != ENPlistOldParserSeqMode_Object){
								//PRINTF_ERROR("PLIST logic ERROR: extra '}' found.\n");
								state->fmtLogicErr = TRUE; NBASSERT(FALSE)
							} else {
								//Remove 'any' and 'object' modes
								*curSeqMode = NBPlistOldParser_privStateSeqModePop(state, TRUE);
								*curSeqMode = NBPlistOldParser_privStateSeqModePop(state, TRUE);
								NBASSERT(*curSeqMode == ENPlistOldParserSeqMode_Value)
								if(listener != NULL){
									(listener->objectEnded)(state, listenerParam);
								}
							}
						}
					}
					break;
				case ')':
					if(state->seqModesStack.use < 2){
						//PRINTF_ERROR("PLIST logic ERROR: something without parent node.\n");
						state->fmtLogicErr = TRUE; NBASSERT(FALSE)
					} else {
						const ENPlistOldParserSeqMode parSeqMode = *((ENPlistOldParserSeqMode*)NBArray_itemAtIndex(&state->seqModesStack, state->seqModesStack.use - 2));
						if(parSeqMode != ENPlistOldParserSeqMode_Array){
							//PRINTF_ERROR("PLIST logic ERROR: extra ')' found.\n");
							state->fmtLogicErr = TRUE; NBASSERT(FALSE)
						} else {
							if(state->strAcum.length != 0){
								if(state->strAccumSeparators != 0){
									//PRINTF_ERROR("PLIST logic ERROR: second value found.\n");
									state->fmtLogicErr = TRUE; NBASSERT(FALSE)
								} else {
									//Add value
									NBPlistOldParser_feedStr(state, curSeqMode, ENPlistOldParserStrType_Plain, state->strAcum.str, state->strAcum.length, listener, listenerParam);
									NBString_empty(&state->strAcum); state->strAccumSeparators++;
								}
							}
							//Close array
							if(listener != NULL){
								//Close member
								if(state->strAccumSeparators != 0){
									(listener->memberEnded)(state, listenerParam);
								}
								(listener->arrayEnded)(state, listenerParam);
							}
							//Remove 'any' and 'array' modes
							*curSeqMode = NBPlistOldParser_privStateSeqModePop(state, TRUE);
							*curSeqMode = NBPlistOldParser_privStateSeqModePop(state, TRUE);
							NBASSERT(*curSeqMode == ENPlistOldParserSeqMode_Value)
						}
					}
					break;
				default:
					//Accumulate char
					if(state->strAcum.length == 0){
						//Start accumulating new word when the first non-space char appears
						if(isSpace){
							if(state->strAccumSeparators != 0){
								state->strAccumSeparators++;
							}
						} else {
							if(state->strAccumSeparators == 0){
								//Start of first word
								NBString_concatByte(&state->strAcum, curChar);
							} else {
								//PRINTF_ERROR("PLIST logic ERROR: found more than one word in a non-literal value.\n");
								state->fmtLogicErr = TRUE; NBASSERT(FALSE)
							}
						}
					} else {
						//Continue accumulating word
						if(!isSpace){
							//Acummulate char
							NBString_concatByte(&state->strAcum, curChar);
						} else {
							NBASSERT(state->strAccumSeparators == 0)
							//Flush current word and start spaces count.
							NBPlistOldParser_feedStr(state, curSeqMode, ENPlistOldParserStrType_Plain, state->strAcum.str, state->strAcum.length, listener, listenerParam);
							NBString_empty(&state->strAcum); state->strAccumSeparators++;
						}
					}
					break;
			}
			break;
		case ENPlistOldParserSeqMode_Value:
			//Reading a string, number, object, array, TRUE, FALSE or null.
			switch(curChar){
				case '<':
					if(state->strAcum.length != 0 || state->strAccumSeparators != 0){
						//PRINTF_ERROR("PLIST logic ERROR: found '<' after string start.\n");
						state->fmtLogicErr = TRUE; NBASSERT(FALSE)
					} else {
						//Open new data
						*curSeqMode = NBPlistOldParser_privStateSeqModePush(state, ENPlistOldParserSeqMode_HexData, TRUE);
					}
					break;
				case '\"':
					if(state->strAcum.length != 0 || state->strAccumSeparators != 0){
						//PRINTF_ERROR("PLIST logic ERROR: found '\"' after string start.\n");
						state->fmtLogicErr = TRUE; NBASSERT(FALSE)
					} else {
						//Open new literal
						*curSeqMode = NBPlistOldParser_privStateSeqModePush(state, ENPlistOldParserSeqMode_Literal, TRUE);
					}
					break;
				case '{':
					if(state->strAcum.length != 0 || state->strAccumSeparators != 0){
						//PRINTF_ERROR("PLIST logic ERROR: found '{' after value started.\n");
						state->fmtLogicErr = TRUE; NBASSERT(FALSE)
					} else {
						//Open new object
						*curSeqMode = NBPlistOldParser_privStateSeqModePush(state, ENPlistOldParserSeqMode_Object, TRUE);
						if(listener != NULL){
							(listener->objectStarted)(state, listenerParam);
						}
						//Expecting anything
						*curSeqMode = NBPlistOldParser_privStateSeqModePush(state, ENPlistOldParserSeqMode_Any, TRUE);
					}
					break;
				case '(':
					if(state->strAcum.length != 0 || state->strAccumSeparators != 0){
						//PRINTF_ERROR("PLIST logic ERROR: found '(' after value started.\n");
						state->fmtLogicErr = TRUE; NBASSERT(FALSE)
					} else {
						//Open new array
						*curSeqMode = NBPlistOldParser_privStateSeqModePush(state, ENPlistOldParserSeqMode_Array, TRUE);
						if(listener != NULL){
							(listener->arrayStarted)(state, listenerParam);
						}
						//Expecting anything
						*curSeqMode = NBPlistOldParser_privStateSeqModePush(state, ENPlistOldParserSeqMode_Any, TRUE);
					}
					break;
				case ';':
					//Flush value
					if(state->strAcum.length != 0){
						if(state->strAccumSeparators != 0){
							//PRINTF_ERROR("PLIST logic ERROR: second value found.\n");
							state->fmtLogicErr = TRUE; NBASSERT(FALSE)
						} else {
							//Add value
							NBPlistOldParser_feedStr(state, curSeqMode, ENPlistOldParserStrType_Plain, state->strAcum.str, state->strAcum.length, listener, listenerParam);
							NBString_empty(&state->strAcum); state->strAccumSeparators++;
						}
					}
					//Close member
					if(listener != NULL){
						(listener->memberEnded)(state, listenerParam);
					}
					*curSeqMode = NBPlistOldParser_privStateSeqModePop(state, TRUE);
					if(*curSeqMode != ENPlistOldParserSeqMode_Object){
						//PRINTF_ERROR("PLIST logic ERROR: member ';' ended outside an object.\n");
						state->fmtLogicErr = TRUE; NBASSERT(FALSE)
					} else {
						//Expecting anything
						*curSeqMode = NBPlistOldParser_privStateSeqModePush(state, ENPlistOldParserSeqMode_Any, TRUE);
					}
					break;
				case ',':
				case ')':
				case '>':
				case '}':
				case '=':
					//PRINTF_ERROR("PLIST logic ERROR: forbidden char('%c') found at value.\n", curChar);
					state->fmtLogicErr = TRUE; NBASSERT(FALSE)
					break;
				default:
					//Accumulate char
					if(state->strAcum.length == 0){
						//Start accumulating new word when the first non-space char appears
						if(isSpace){
							if(state->strAccumSeparators != 0){
								state->strAccumSeparators++;
							}
						} else {
							if(state->strAccumSeparators == 0){
								//Start of first word
								NBString_concatByte(&state->strAcum, curChar);
							} else {
								//PRINTF_ERROR("PLIST logic ERROR: found more than one word in a non-literal value.\n");
								state->fmtLogicErr = TRUE; NBASSERT(FALSE)
							}
						}
					} else {
						//Continue accumulating word
						if(!isSpace){
							//Acummulate char
							NBString_concatByte(&state->strAcum, curChar);
						} else {
							NBASSERT(state->strAccumSeparators == 0)
							//Flush current word and start spaces count.
							NBPlistOldParser_feedStr(state, curSeqMode, ENPlistOldParserStrType_Plain, state->strAcum.str, state->strAcum.length, listener, listenerParam);
							NBString_empty(&state->strAcum); state->strAccumSeparators++;
						}
					}
					break;
			}
			break;
		default:
			NBASSERT(FALSE) //Program logic error
			break;
	}
}

BOOL NBPlistOldParser_feedStart(STNBPlistOldParser* state, IPlistOldParserListener* listener, void* listenerParam){
	if(state->seqModesStack.use == 0){
		NBPlistOldParser_privStateSeqModePush(state, ENPlistOldParserSeqMode_Value, TRUE);
		if(listener != NULL){
			(listener->memberStarted)(state, listenerParam);
		}
	} else {
		PRINTF_ERROR("PLIST PARSER logic ERROR: modes stack wasnt empty at first call.");
		state->fmtLogicErr = TRUE; NBASSERT(FALSE)
	}
	return (!state->fmtLogicErr);
}

BOOL NBPlistOldParser_feedEnd(STNBPlistOldParser* state, IPlistOldParserListener* listener, void* listenerParam){
	//Only doc-obj must remain at stack
	if(state->seqModesStack.use != 1){
		IF_NBASSERT(PRINTF_WARNING("PLIST parse error: %d depths remained open at the end.\n", (state->seqModesStack.use - 1));)
		IF_NBASSERT(PRINTF_WARNING("PLIST parser acumm last content (%d bytes): '%s'.\n", state->strAcum.length, state->strAcum.str);)
		state->fmtLogicErr = TRUE; NBASSERT(FALSE) //Do not assert, this is posible when user cancels the connection (or network problems).
	}
	return (!state->fmtLogicErr);
}

BOOL NBPlistOldParser_feed(STNBPlistOldParser* state, const void* pBuffer, const SI32 elemsAtbuffer, IPlistOldParserListener* listener, void* listenerParam){
	const char* readBuff			= (const char*)pBuffer;
	ENPlistOldParserSeqMode curSeqMode	= (ENPlistOldParserSeqMode)0;
	//Load current parsing mode
	if(state->seqModesStack.use > 0){
		curSeqMode = *((ENPlistOldParserSeqMode*)NBArray_itemAtIndex(&state->seqModesStack, state->seqModesStack.use - 1));
	} else {
		PRINTF_ERROR("PLIST logic ERROR: empty sequence-modes-stack.");
		state->fmtLogicErr = TRUE; NBASSERT(FALSE)
	}
	//-----------------------
	//-- Parse content
	//-----------------------
	if(!state->fmtLogicErr){
		SI32 iCurChar	= 0;
		char curChar	= readBuff[iCurChar];
		//PRINTF_INFO("PLIST parse, consuming %d bytes.\n", elemsAtbuffer);
		while(iCurChar < elemsAtbuffer && !state->fmtLogicErr){
			if(curSeqMode == ENPlistOldParserSeqMode_Literal || curSeqMode == ENPlistOldParserSeqMode_LiteralSChar){
				//Consume char inside literal
				NBASSERT(state->prevCharPend == '\0')
				NBPlistOldParser_feedChar(state, &curSeqMode, curChar, listener, listenerParam);
			} else {
				//Analyze posible comment start
				if(curSeqMode == ENPlistOldParserSeqMode_CommentMono){
					NBASSERT(state->prevCharPend == '\0')
					if(curChar != '\n'){
						//Inside mono-line comment
						NBString_concatByte(&state->strComment, curChar);
					} else {
						//End mono-line comment
						//PRINTF_INFO("PLIST, mono-line comment: '%s'.\n", state->strComment.str);
						curSeqMode = NBPlistOldParser_privStateSeqModePop(state, FALSE);
						NBString_empty(&state->strComment);
					}
				} else if(curSeqMode == ENPlistOldParserSeqMode_CommentMulti){
					if(state->prevCharPend == '\0' && curChar == '*'){
						//Acumulate '*' as posible start of '*/'
						state->prevCharPend = curChar;
					} else if(state->prevCharPend == '*' && curChar == '/'){
						//End multi-line comment
						//PRINTF_INFO("PLIST, multi-line comment: '%s'.\n", state->strComment.str);
						curSeqMode = NBPlistOldParser_privStateSeqModePop(state, FALSE);
						NBString_empty(&state->strComment);
						state->prevCharPend = '\0';
					} else {
						//Inside multi-line comment
						if(state->prevCharPend != '\0'){
							NBString_concatByte(&state->strComment, state->prevCharPend);
							state->prevCharPend = '\0';
						}
						NBString_concatByte(&state->strComment, curChar);
					}
				} else {
					if(state->prevCharPend == '\0' && curChar == '/'){
						//Acumulate '/' as posible start of '//' or '/*'
						state->prevCharPend = curChar;
					} else if(state->prevCharPend == '/' && curChar == '/'){
						//Generate an artificial space
						NBPlistOldParser_feedChar(state, &curSeqMode, ' ', listener, listenerParam);
						//Start of a mono-line comment
						curSeqMode = NBPlistOldParser_privStateSeqModePush(state, ENPlistOldParserSeqMode_CommentMono, FALSE);
						state->prevCharPend = '\0';
					} else if(state->prevCharPend == '/' && curChar == '*'){
						//Generate an artificial space
						NBPlistOldParser_feedChar(state, &curSeqMode, ' ', listener, listenerParam);
						//Start of a multi-line comment
						curSeqMode = NBPlistOldParser_privStateSeqModePush(state, ENPlistOldParserSeqMode_CommentMulti, FALSE);
						state->prevCharPend = '\0';
					} else {
						//Normal content
						if(state->prevCharPend != '\0'){
							NBPlistOldParser_feedChar(state, &curSeqMode, state->prevCharPend, listener, listenerParam);
							state->prevCharPend = '\0';
						}
						NBPlistOldParser_feedChar(state, &curSeqMode, curChar, listener, listenerParam);
					}
				}
			}
			//Next char
			iCurChar++;
			if(iCurChar < elemsAtbuffer){
				curChar = readBuff[iCurChar];
			}
		}
		//PRINTF_INFO("PLIST parse, %d of %d bytes consumed.\n", iCurChar, elemsAtbuffer);
		if(state->fmtLogicErr){
			STNBString strAtErr;
		    NBString_init(&strAtErr);
			const SI32 charsRemain = (elemsAtbuffer - iCurChar);
			if(iCurChar >= 32 || charsRemain < 32){
			    NBString_concatBytes(&strAtErr, &readBuff[iCurChar - 32], 32);
				PRINTF_ERROR("PLIST logic ERROR at end of: '%s'.\n", strAtErr.str);
			} else {
			    NBString_concatBytes(&strAtErr, &readBuff[iCurChar], (charsRemain > 32 ? 32 : charsRemain));
				PRINTF_ERROR("PLIST logic ERROR before: '%s'.\n", strAtErr.str);
			}
		    NBString_release(&strAtErr);
		}
	}
	//
	return (!state->fmtLogicErr);
}

//private

ENPlistOldParserSeqMode NBPlistOldParser_privStateSeqModePush(STNBPlistOldParser* state, const ENPlistOldParserSeqMode newMode, const BOOL emptyAccum){
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	{
		const char* modeName = "?";
		switch (newMode) {
			case ENPlistOldParserSeqMode_Value:			modeName = "A_VALUE"; break;
			case ENPlistOldParserSeqMode_Any:			modeName = "ANY_STR"; break;
			case ENPlistOldParserSeqMode_Object:		modeName = "OBJECTT"; break;
			case ENPlistOldParserSeqMode_Array:			modeName = "ARRAYYY"; break;
			case ENPlistOldParserSeqMode_HexData:		modeName = "HEXDATA"; break;
			case ENPlistOldParserSeqMode_Literal:		modeName = "LITERAL"; break;
			case ENPlistOldParserSeqMode_LiteralSChar:	modeName = "LIT_SCH"; break;
			case ENPlistOldParserSeqMode_CommentMono:	modeName = "COMM_MONO"; break;
			case ENPlistOldParserSeqMode_CommentMulti:	modeName = "COMM_MULTI"; break;
#			ifdef NB_CONFIG_INCLUDE_ASSERTS
			default:
				NBASSERT(FALSE) 
				break;
#			endif
		}
		//char cDeep[512]; PRINTF_INFO("%sPLIST parse, push-to %s (after acumm: '%s')\n", NBString_strRepeatByte(cDeep, ' ', state->seqModesStack.use), modeName, state->strAcum.str);
	}
#	endif
    NBArray_addValue(&state->seqModesStack, newMode);
	if(emptyAccum){
	    NBString_empty(&state->strAcum);
		state->strAccumSeparators = 0;
	}
	return newMode;
}

ENPlistOldParserSeqMode NBPlistOldParser_privStateSeqModePop(STNBPlistOldParser* state, const BOOL emptyAccum){
	ENPlistOldParserSeqMode newMode = (ENPlistOldParserSeqMode)0;
	//Remove top mode
	if(state->seqModesStack.use > 0){
	    NBArray_removeItemAtIndex(&state->seqModesStack, state->seqModesStack.use - 1);
	} else {
		PRINTF_ERROR("PLIST logic ERROR: more closing than opening tags.");
		state->fmtLogicErr = TRUE; NBASSERT(FALSE)
	}
	//Load new mode
	if(state->seqModesStack.use > 0){
		newMode = *((ENPlistOldParserSeqMode*)NBArray_itemAtIndex(&state->seqModesStack, state->seqModesStack.use - 1));
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		{
			const char* modeName = "?";
			switch (newMode) {
				case ENPlistOldParserSeqMode_Value:			modeName = "A_VALUE"; break;
				case ENPlistOldParserSeqMode_Any:			modeName = "ANY_STR"; break;
				case ENPlistOldParserSeqMode_Object:		modeName = "OBJECTT"; break;
				case ENPlistOldParserSeqMode_Array:			modeName = "ARRAYYY"; break;
				case ENPlistOldParserSeqMode_HexData:		modeName = "HEXDATA"; break;
				case ENPlistOldParserSeqMode_Literal:		modeName = "LITERAL"; break;
				case ENPlistOldParserSeqMode_LiteralSChar:	modeName = "LIT_SCH"; break;
				case ENPlistOldParserSeqMode_CommentMono:	modeName = "COMM_MONO"; break;
				case ENPlistOldParserSeqMode_CommentMulti:	modeName = "COMM_MULTI"; break;
#				ifdef NB_CONFIG_INCLUDE_ASSERTS
				default:
					NBASSERT(FALSE) 
					break;
#				endif
			}
			//char cDeep[512]; PRINTF_INFO("%sPLIST parse,  pop-to %s (after acumm: '%s')\n", NBString_strRepeatByte(cDeep, ' ', state->seqModesStack.use), modeName, state->strAcum.str);
		}
#		endif
	} else {
		//Logic error: the stack should never be empty
		//The doc-object must always be the first in the stack.
		PRINTF_ERROR("PLIST logic ERROR: more closing than opening tags.");
		state->fmtLogicErr = TRUE; NBASSERT(FALSE)
	}
	//
	if(emptyAccum){
	    NBString_empty(&state->strAcum);
		state->strAccumSeparators = 0;
	}
	return newMode;
}

ENPlistOldParserSeqMode NBPlistOldParser_privConsumeLiteralSlashChar(STNBPlistOldParser* state, const char curChar, const ENPlistOldParserSeqMode pCurSeqMode, const BOOL emptyAccumAfter){
	ENPlistOldParserSeqMode curSeqMode = pCurSeqMode;
	//Something after special slash in a literal string
	//" ... \" \\ \/ \b \f \n \r \t \uFFFF \uFFFF\uFFFF..."
	if(state->literalSChars.length == 0){
		//First char
		switch(curChar) {
			case '\"':
			    NBString_concatByte(&state->strAcum, '\"');
				curSeqMode = NBPlistOldParser_privStateSeqModePop(state, emptyAccumAfter);
			    NBASSERT(curSeqMode == ENPlistOldParserSeqMode_Literal)
				break;
			case '\\':
			    NBString_concatByte(&state->strAcum, '\\');
				curSeqMode = NBPlistOldParser_privStateSeqModePop(state, emptyAccumAfter);
			    NBASSERT(curSeqMode == ENPlistOldParserSeqMode_Literal)
				break;
			case '/':
			    NBString_concatByte(&state->strAcum, '/');
				curSeqMode = NBPlistOldParser_privStateSeqModePop(state, emptyAccumAfter);
			    NBASSERT(curSeqMode == ENPlistOldParserSeqMode_Literal)
				break;
			case 'b':
			    NBString_concatByte(&state->strAcum, '\b');
				curSeqMode = NBPlistOldParser_privStateSeqModePop(state, emptyAccumAfter);
			    NBASSERT(curSeqMode == ENPlistOldParserSeqMode_Literal)
				break;
			case 'f':
			    NBString_concatByte(&state->strAcum, '\f');
				curSeqMode = NBPlistOldParser_privStateSeqModePop(state, emptyAccumAfter);
			    NBASSERT(curSeqMode == ENPlistOldParserSeqMode_Literal)
				break;
			case 'n':
			    NBString_concatByte(&state->strAcum, '\n');
				curSeqMode = NBPlistOldParser_privStateSeqModePop(state, emptyAccumAfter);
			    NBASSERT(curSeqMode == ENPlistOldParserSeqMode_Literal)
				break;
			case 'r':
			    NBString_concatByte(&state->strAcum, '\r');
				curSeqMode = NBPlistOldParser_privStateSeqModePop(state, emptyAccumAfter);
			    NBASSERT(curSeqMode == ENPlistOldParserSeqMode_Literal)
				break;
			case 't':
			    NBString_concatByte(&state->strAcum, '\t');
				curSeqMode = NBPlistOldParser_privStateSeqModePop(state, emptyAccumAfter);
			    NBASSERT(curSeqMode == ENPlistOldParserSeqMode_Literal)
				break;
			case 'u':
			    NBString_concatByte(&state->literalSChars, 'u');
				break;
			default:
				PRINTF_ERROR("PLIST logic ERROR: unexpected value after slash inside literal.");
				state->fmtLogicErr = TRUE; NBASSERT(FALSE)
				break;
		}
	} else {
		//Nexts chars
		const char initChar = state->literalSChars.str[0];
		switch(initChar){
			case 'u':
				/*
				 UTF16
				 2 bytes (00xx-xxxx xxxx-xxxx)
				 4 bytes (1101-10xx xxxx-xxxx 1101-11xx xxxx-xxxx)
				 UTF8
				 1 byte  (0xxxxxxx)
				 2 bytes (110xxxxx 10xxxxxx)
				 3 bytes (1110xxxx 10xxxxxx 10xxxxxx)
				 4 bytes (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
				 5 bytes (111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx)
				 6 bytes (1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx)
				 */
				if(state->literalSChars.length == 5){
					if(curChar == '\\'){
						NBString_concatByte(&state->literalSChars, curChar);
					} else {
						PRINTF_ERROR("PLIST logic ERROR: expected second utf16 surrogate #%d of 4 (after '\\%s').", state->literalSChars.length, state->literalSChars.str);
						state->fmtLogicErr = TRUE; NBASSERT(FALSE)
					}
				} else if(state->literalSChars.length == 6){
					if(curChar == 'u'){
						NBString_concatByte(&state->literalSChars, curChar);
					} else {
						PRINTF_ERROR("PLIST logic ERROR: expected second utf16 surrogate #%d of 4 (after '\\%s').", state->literalSChars.length, state->literalSChars.str);
						state->fmtLogicErr = TRUE; NBASSERT(FALSE)
					}
				} else {
					switch(curChar) {
						case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
						case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
						case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
							NBString_concatByte(&state->literalSChars, curChar);
							if(state->literalSChars.length == 5 || state->literalSChars.length == 11){ //5 'uFFFF', or 11 'uFFFF\uFFFF'
								UI16 vUtf16[2] = {0, 0};
								vUtf16[0] = NBEncoding_utf16SurrogateFromHex(&state->literalSChars.str[1]);
								if(state->literalSChars.length == 5 && NBEncoding_utf16SurrogatesExpected(vUtf16[0]) == 2){
									//Continue special-sequence, expecting another UTF16 surrogate.
									//PRINTF_INFO("Waiting for second utf16 surrogate, after: '%s'.\n", state->literalSChars.str);
								} else {
									//PRINTF_INFO("Parsing unicode, for: '%s'.\n", state->literalSChars.str);
									if(state->literalSChars.length == 11){
										vUtf16[1] = NBEncoding_utf16SurrogateFromHex(&state->literalSChars.str[7]);
									}
									UI32 unicode; char utf8[7];
									unicode = NBEncoding_unicodeFromUtf16(vUtf16, 0);
									NBASSERT(unicode == NBEncoding_unicodeFromUtf16s(vUtf16, (state->literalSChars.length == 5 ? 1 : 2), 0))
									NBEncoding_utf8FromUnicode(unicode, utf8);
									NBString_concat(&state->strAcum, utf8);
									//PRINTF_INFO("unicode(%u) to utf8('%s', %u bytes).\n", unicode, utf8, NBString_strLenBytes(utf8));
									curSeqMode = NBPlistOldParser_privStateSeqModePop(state, emptyAccumAfter);
									NBASSERT(curSeqMode == ENPlistOldParserSeqMode_Literal)
									//PRINTF_INFO("----------\n");
								}
							}
							break;
						default:
							PRINTF_ERROR("PLIST logic ERROR: expected hex-char #%d of 4 (after '\\%s').", state->literalSChars.length, state->literalSChars.str);
							state->fmtLogicErr = TRUE; NBASSERT(FALSE)
							break;
					}
				}
				break;
			default:
				//Program logic error, this
				//point should never be reached.
			    NBASSERT(FALSE)
				break;
		}
	}
	return curSeqMode;
}


