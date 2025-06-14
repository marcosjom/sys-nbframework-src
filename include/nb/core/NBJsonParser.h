//
//  XUXml.h
//  XUMonitorLogicaNegocios_OSX
//
//  Created by Marcos Ortega on 21/01/14.
//  Copyright (c) 2014 XurpasLatam. All rights reserved.
//

#ifndef NB_JSON_PARSER_H
#define NB_JSON_PARSER_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBString.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	//https://www.json.org/json-en.html
	//https://github.com/nst/JSONTestSuite

	#define NB_JSON_IS_WHITESPACE(C) 			((C) == ' ' || (C) == '\t' || (C) == '\r' || (C) == '\n')
	#define NB_JSON_IS_CONTROL_CHAR(C)			((C) >= 0 && (C) <= 31)
	#define NB_JSON_IS_FORBIDDEN_STR_CHAR(C)	(NB_JSON_IS_CONTROL_CHAR(C) && (C) != '\t' && (C) != '\r' && (C) != '\n')

	//NBJsonParserNumDesc

	typedef struct STNBJsonParserNumDesc_ {
		//integer part
		struct {
			UI8		signChars;	//count of initial '-' chars
			UI16	valChars;	//count of integer part chars
		} iPart;
		//fraction part
		struct {
			UI8		dotChars;	//count of initial '.' chars
			UI16	valChars;	//count of integer part chars
		} fPart;
		//exponent part
		struct {
			UI8		eChars;		//count of initial 'e' or 'E' chars
			UI8		signChars;	//count of initial '-' or '+' chars
			UI16	valChars;	//count of exponent part chars
		} ePart;
	} STNBJsonParserNumDesc;

	//NBJsonParser

	typedef struct STNBJsonParser_ {
		BOOL		errFnd;
        BOOL        endFnd;     //root member closed, remaining content could be garbage or next object
		UI32		bytesFeed;  //Total bytes feed to parser
		STNBArray	seqsStack;  //STNBJsonParserSeq
		STNBString	strAcum;    //str for accumulation of names and values. Will be flushed every ~1Kbs of accumulation.
		STNBString	errDesc;    //descriptive parsing error
	} STNBJsonParser;
	
	typedef struct IJsonParserListener_ {
		void (*memberStarted)(const STNBJsonParser* state, void* listenerParam);
		void (*consumeName)(const STNBJsonParser* state, const char* unscapedName, void* listenerParam);
		void (*consumePlain)(const STNBJsonParser* state, const char* data, const SI32 dataSize, void* listenerParam);
		void (*consumeNumber)(const STNBJsonParser* state, const char* data, const STNBJsonParserNumDesc* numDesc, void* listenerParam);
		void (*consumeLiteral)(const STNBJsonParser* state, const char* unscapedData, const SI32 dataSize, void* listenerParam);
		void (*objectStarted)(const STNBJsonParser* state, void* listenerParam);
		void (*objectEnded)(const STNBJsonParser* state, void* listenerParam);
		void (*arrayStarted)(const STNBJsonParser* state, void* listenerParam);
		void (*arrayEnded)(const STNBJsonParser* state, void* listenerParam);
		void (*memberEnded)(const STNBJsonParser* state, void* listenerParam);
	} IJsonParserListener;
	
	void NBJsonParser_init(STNBJsonParser* state);
	void NBJsonParser_release(STNBJsonParser* state);
	//
	BOOL NBJsonParser_feedStart(STNBJsonParser* state, IJsonParserListener* listener, void* listenerParam);
	UI32 NBJsonParser_feed(STNBJsonParser* state, const void* data, const SI32 dataSz, IJsonParserListener* listener, void* listenerParam);
	BOOL NBJsonParser_feedIsComplete(STNBJsonParser* state);
	BOOL NBJsonParser_feedEnd(STNBJsonParser* state, IJsonParserListener* listener, void* listenerParam);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
