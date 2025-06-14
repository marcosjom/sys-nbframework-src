//
//  XUXml.h
//  XUMonitorLogicaNegocios_OSX
//
//  Created by Marcos Ortega on 21/01/14.
//  Copyright (c) 2014 XurpasLatam. All rights reserved.
//

#ifndef NB_PLIST_OLD_PARSER_H
#define NB_PLIST_OLD_PARSER_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBString.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef enum ENPlistOldParserSeqMode_ {
		ENPlistOldParserSeqMode_Value = 0,			//Reading plain, literal or the start of an object or array
		ENPlistOldParserSeqMode_Any,				//Reading a name or the end of an empty object or array
		ENPlistOldParserSeqMode_Object,				//Reading a "string : value" pairs element or sequence
		ENPlistOldParserSeqMode_Array,				//Reading a values sequences
		ENPlistOldParserSeqMode_HexData,			//Reading hex data between "<012...ABDEF>" (spaces are allowed)
		ENPlistOldParserSeqMode_Literal,			//Reading a literal string
		ENPlistOldParserSeqMode_LiteralSChar,		//Something after special slash in a literal string (like "...\n..." or "...\uFFFF...")
		ENPlistOldParserSeqMode_CommentMono,		//Monoline comment '//'
		ENPlistOldParserSeqMode_CommentMulti		//Multiline comment '/* ... */'
	} ENPlistOldParserSeqMode;
	
	typedef struct STNBPlistOldParser_ {
		BOOL		fmtLogicErr;
		char		prevCharPend;			//Pending char to consume ('\0' if no pending char)
		SI32		strAccumSeparators;		//spaces ignored after accumulating some content
		STNBArray	seqModesStack;			//ENPlistOldParserSeqMode
		STNBString	strAcum;
		STNBString	strComment;
		STNBString	literalSChars; 			//chars read after a slash in a literal string "(like "...\n..." or "...\uFFFF...")
	} STNBPlistOldParser;
	
	typedef struct IPlistOldParserListener_ {
		void (*memberStarted)(const STNBPlistOldParser* state, void* listenerParam);
		void (*consumeName)(const STNBPlistOldParser* state, const char* unscapedName, void* listenerParam);
		void (*consumePlain)(const STNBPlistOldParser* state, const char* data, const SI32 dataSize, void* listenerParam);
		void (*consumeHexData)(const STNBPlistOldParser* state, const char* hexData, const SI32 dataSize, void* listenerParam);
		void (*consumeLiteral)(const STNBPlistOldParser* state, const char* unscapedData, const SI32 dataSize, void* listenerParam);
		void (*objectStarted)(const STNBPlistOldParser* state, void* listenerParam);
		void (*objectEnded)(const STNBPlistOldParser* state, void* listenerParam);
		void (*arrayStarted)(const STNBPlistOldParser* state, void* listenerParam);
		void (*arrayEnded)(const STNBPlistOldParser* state, void* listenerParam);
		void (*memberEnded)(const STNBPlistOldParser* state, void* listenerParam);
	} IPlistOldParserListener;
	
	void NBPlistOldParser_init(STNBPlistOldParser* state);
	void NBPlistOldParser_release(STNBPlistOldParser* state);
	//
	BOOL NBPlistOldParser_feedStart(STNBPlistOldParser* state, IPlistOldParserListener* listener, void* listenerParam);
	BOOL NBPlistOldParser_feed(STNBPlistOldParser* state, const void* pBuffer, const SI32 elemsAtbuffer, IPlistOldParserListener* listener, void* listenerParam);
	BOOL NBPlistOldParser_feedEnd(STNBPlistOldParser* state, IPlistOldParserListener* listener, void* listenerParam);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
