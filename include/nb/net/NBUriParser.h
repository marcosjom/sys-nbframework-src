#ifndef NB_URI_PARSER_H
#define NB_URI_PARSER_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBString.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	//-----------------
	//-- URI - RFC3986
	//-- https://tools.ietf.org/html/rfc3986
	//-----------------
	//-- ERRATA
	//--- https://www.rfc-editor.org/errata_search.php?rfc=3986
	//reg-name		= *( unreserved / pct-encoded / "-" / ".")
	//path-empty	= ""
	//fragment		= *( pchar / "/" / "?" / "#" )
	//IPv6address	= (see document)
	//hier-part		= "//" authority path-abempty / path-absolute / path-noscheme / path-rootless / path-empty
	//-----------------
	
	typedef enum ENNBUriParserType_ {
		ENNBUriParserType_uri = 0,		//scheme ":" hier-part [ "?" query ] [ "#" fragment ]
		ENNBUriParserType_absoluteUri,	//scheme ":" hier-part [ "?" query ]
		ENNBUriParserType_scheme,		//ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
		//Pct-encoded
		ENNBUriParserType_pctEncoded,	//'%' HEXDIG HEXDIG
		//Hier part
		ENNBUriParserType_hierPart,		//"//" authority path-abempty / path-absolute / path-noscheme / path-rootless / path-empty
		//Path
		//Note: future parsing of path-rule, should considere the literal definition:
		//      "The path is terminated by the first question mark ("?")
		//      or number sign ("#") character, or by the end of the URI."
		//      That way the end-of-the-URI will be determinated by the "feed" caller (to consider the container delimiters only).
		ENNBUriParserType_pathAbEmpty,	//*( "/" segment )
		ENNBUriParserType_pathAbsolute,	//"/" [ segment-nz *( "/" segment ) ]
		ENNBUriParserType_pathNoScheme,	//segment-nz-nc *( "/" segment )
		ENNBUriParserType_pathRootless,	//segment-nz *( "/" segment )
		ENNBUriParserType_pathEmpty,	//""
		//Authority
		//Note: future parsing of auth-rule, should considere the literal definition:
		//      "The authority component is preceded by a double slash ("//") and is
		//      terminated by the next slash ("/"), question mark ("?"), or number
		//      sign ("#") character, or by the end of the URI."
		//      That way the end-of-the-URI will be determinated by the "feed" caller (to consider the container delimiters only).
		ENNBUriParserType_authority,	//[ userinfo "@" ] host [ ":" port ]
		//Path components
		ENNBUriParserType_segment,		//*(pchar)
		ENNBUriParserType_segmentNz,	//1*(pchar)
		ENNBUriParserType_segmentNzNc,	//1*(unreserved / pct-encoded / sub-delims / "@")
		//User info
		ENNBUriParserType_userInfo,		//*( unreserved / pct-encoded / sub-delims / ":" )
		//Host
		ENNBUriParserType_port,			//*DIGIT
		ENNBUriParserType_host,			//IP-literal / IPv4address / reg-name
		ENNBUriParserType_IPLiteral,	//"[" ( IPv6address / IPvFuture  ) "]"
		ENNBUriParserType_IPvFuture,	//"v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )
		ENNBUriParserType_IPv6Address,	//(only h16, ":" or ls32 chars)
		ENNBUriParserType_ls32,			//( h16 ":" h16 ) / IPv4address
		ENNBUriParserType_h16,			//1*4HEXDIG
		ENNBUriParserType_IPV4Address,	//dec-octet "." dec-octet "." dec-octet "." dec-octet
		ENNBUriParserType_decOctet,		//0-9, 10-99, 100-199, 200-249, 250-255
		ENNBUriParserType_regName,		//*( unreserved / pct-encoded / "-" / ".")
		//Query '?'
		//Note: parsing will ignore this spec and use this:
		//      "The query component is indicated by the first question mark ("?") character
		//      and terminated by a number sign ("#") character or by the end of the URI."
		//      That way the end-of-the-URI will be determinated by the "feed" caller (to consider the container delimiters only).
		ENNBUriParserType_query,		//*( pchar / "/" / "?" )
		//Fragment '#'
		//Note: parsing will ignore this spec and use this:
		//      "Afragment identifier component is indicated by the presence of a
		//      number sign ("#") character and terminated by the end of the URI."
		//      That way the end-of-the-URI will be determinated by the "feed" caller (to consider the container delimiters only).
		ENNBUriParserType_fragment,		//*( pchar / "/" / "?" / "#" )
		//Unreserved
		ENNBUriParserType_unreserved,	//*( unreserved )	//Not in the specs, used to scape/unscape
		//
		ENNBUriParserType_count
	} ENNBUriParserType;
	
	typedef struct STNBUriParserRng_ {
		UI32		iStart;
		UI32		count;
	} STNBUriParserRng;
	
	typedef struct STNBUriParserNode_ {
		ENNBUriParserType	type;		//Depth type
		BOOL				popped;		//Still alive?
		UI8					iCurPosib;	//Currently testing posibility
		UI8					iCurPart;	//Currently testing part in posibility
		BOOL				curPartErr;	//Error ocurred in current evaluated part
		UI32				iDeepLvl;	//deep level (reference)
		STNBUriParserRng	rng;	//Range of content in not-ignored seqs (the ones added to the strAccum).
	} STNBUriParserNode;
		
	typedef struct STNBUriParser_ {
		BOOL				fmtLogicError;
		STNBArray			nodesStack;		//STNBUriParserNode
		STNBUriParserNode*	curNode;		//Current active node
		STNBString			strAcum;		//Accumulated chars
		STNBString			strPend;		//Pending chars
		UI32				bytesFeed;		//total bytes feed to the parser
	} STNBUriParser;
	
	//
	
	void NBUriParser_init(STNBUriParser* state);
	void NBUriParser_release(STNBUriParser* state);
	
	//Encoding
	BOOL NBUriParser_concatUnencoded(STNBString* dst, const char* encd);
	BOOL NBUriParser_concatUnencodedBytes(STNBString* dst, const char* encd, const UI32 encdSz);
	BOOL NBUriParser_concatEncoded(STNBString* dst, const char* unenc, const ENNBUriParserType type);
	
	//Feed
	
	BOOL NBUriParser_feedStart(STNBUriParser* state);
	BOOL NBUriParser_feedStartWithType(STNBUriParser* state, const ENNBUriParserType rootType);
	BOOL NBUriParser_feedChar(STNBUriParser* state, const char c);
	BOOL NBUriParser_feed(STNBUriParser* state, const void* data, const SI32 dataSz);
	BOOL NBUriParser_feedEnd(STNBUriParser* state);
	BOOL NBUriParser_isCompleted(STNBUriParser* state);
	
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	BOOL NBUriParser_dbgTestUris(void);
	BOOL NBUriParser_dbgTestUri(const char* uri);
#	endif
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
