#ifndef NB_XML_PARSER_H
#define NB_XML_PARSER_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBString.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef enum ENNBXmlParserType_ {
		//Plain content or document's root
		ENNBXmlParserType_content = 0,	//content outside markup
		//Inside markup
		ENNBXmlParserType_tag,			//found '<', expecting '/' or '?', tag-name, '!' (when tag name has been defined); or attrib name when tagname is defined
		ENNBXmlParserType_tagDataOpen,	//found '<!', expecting '[CDATA[' or '-' (for comment)
		ENNBXmlParserType_tagData,		//found '<![CDATA[', expecting anything but ']]'
		ENNBXmlParserType_tagDataEnd,	//found ']', analyzing posible end
		ENNBXmlParserType_tagDataEnd2,	//found ']]', expecting '>'
		ENNBXmlParserType_tagName,		//found '<a', expecting any sequence of valid chars for a name
		ENNBXmlParserType_tagNameOnly,	//found '</a', expecting any sequence of valid chars for a name
		ENNBXmlParserType_tagAttrbName,	//found 'a', expecting any sequence of valid chars for a name until '='
		ENNBXmlParserType_tagAttrbEq,	//found 'a', expecting '='
		ENNBXmlParserType_tagAttrbVal,	//found 'a=', expecting double-quote as first char
		ENNBXmlParserType_tagAttrbValS,	//found "a='", expecting content untill next single-quote
		ENNBXmlParserType_tagAttrbValD,	//found 'a="', expecting content untill next double-quote
		ENNBXmlParserType_tagEndStart,	//found '/', expecting '>'
		ENNBXmlParserType_tagEndEnd,	//found '</name', expecting '>'
		//Xml header
		ENNBXmlParserType_declarOpen,	//found '<?' expecting 'xml'
		ENNBXmlParserType_declar,		//found '<?xml' expecting anything but '?'
		ENNBXmlParserType_declarEnd,	//found '?' expecting '>'
		//Comment
		ENNBXmlParserType_commOpen,		//found '<!', expecting '--'
		ENNBXmlParserType_comm,			//found '<!--', expecting anything except '--'
		ENNBXmlParserType_commEnd,		//found '-', analyzing posible end
		ENNBXmlParserType_commEnd2,		//found '--', expecting '>'
		//Scape seq
		ENNBXmlParserType_scapeSeq,		//found '&' expecting 'lt', 'get', 'amp', 'apos', 'quot' or '#'
		ENNBXmlParserType_scapeSeqN,	//found '&#' expecting 'x', or digit-seq until ';'
		ENNBXmlParserType_scapeSeqNX,	//found '&#x' expecting hex-digit-seq until ';'
	} ENNBXmlParserType;
	
	typedef struct STNBXmlParserEncBuff_ {
		char				data[8];	//Current encoding accum (one unicode char)
		UI8					bytesAcum;	//Current bytes accumulated
		UI8					bytesExpct;	//Current bytes expected
	} STNBXmlParserEncBuff;
	
	struct STNBXmlParser_;
	
	typedef struct IXmlParserListener_ {
		void (*consumeNodeOpening)(const struct STNBXmlParser_* state, const char* nodePath, const UI32 nodePathSz, const char* tagName, void* listenerParam);
		void (*consumeNodeContent)(const struct STNBXmlParser_* state, const char* nodePath, const UI32 nodePathSz, const char* tagName, const char* data, const SI32 dataSize, void* listenerParam);
		void (*consumeNodeAttrb)(const struct STNBXmlParser_* state, const char* nodePath, const UI32 nodePathSz, const char* tagName, const char* attrbName, const char* attrbValue, const UI32 attrbValueSz, void* listenerParam);
		void (*consumeNodeClosing)(const struct STNBXmlParser_* state, const char* nodePath, const UI32 nodePathSz, const char* tagName, void* listenerParam);
	} IXmlParserListener;
	
	typedef struct STNBXmlParser_ {
		//Encoding load
		STNBXmlParserEncBuff encBuff;
		//
		BOOL				fmtLogicError;
		STNBArray			tagsStackIdxs;	//UI32
		STNBString			tagsStack;		//All string in tags stack
		STNBString			curTagName;
		STNBString			curAttrbName;
		STNBString			curScapeSeq;
		STNBString			strAcum;
		//
		STNBArray			typesStack;	//ENNBXmlParserType
		ENNBXmlParserType	curType;
		UI32				tagsCount;
		//
		IXmlParserListener	listener;
		void*				listenerParam;
	} STNBXmlParser;
	
	//
	
	void NBXmlParser_init(STNBXmlParser* state);
	void NBXmlParser_initWithListener(STNBXmlParser* state, const IXmlParserListener* listener, void* listenerParam);
	void NBXmlParser_release(STNBXmlParser* state);
	
	BOOL NBXmlParser_feedStart(STNBXmlParser* state);
	BOOL NBXmlParser_feedBytes(STNBXmlParser* state, const void* data, const SI32 dataSz);
	BOOL NBXmlParser_feedIsComplete(STNBXmlParser* state);
	BOOL NBXmlParser_feedEnd(STNBXmlParser* state);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
