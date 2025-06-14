
#include "nb/NBFrameworkPch.h"
#include "nb/core/NBEncoding.h"
#include "nb/core/NBNumParser.h"
#include "nb/net/NBUriParser.h"

#ifdef NB_CONFIG_INCLUDE_ASSERTS
//#	define NBURI_PARSER_DBG_PRINT_STACK_CHANGES
#endif

//-----------------
//-- ABNF RFC5234 Syntax Notation (Core Rules)
//-- [RFC5234], Appendix B.1: https://tools.ietf.org/html/rfc5234
//-----------------
#define NBURI_IS_ALPHA(C)		((C >= 'A' && C <= 'Z') || (C >= 'a' && C <= 'z'))
#define NBURI_IS_BIT(C)			(C == '0' || C == '1')
#define NBURI_IS_CHAR(C)		(C >= 0x01 || C <= 0x7F)	//any 7-bit US-ASCII character excluding NULL
#define NBURI_IS_CR(C)			(C == 0x0D)					//carriage return
#define NBURI_IS_LF(C)			(C == 0x0A)					//linefeed
#define NBURI_IS_CRLF(C, C2)	(NBURI_IS_CR_CHAR(C) && NBURI_IS_LF_CHAR(C2)) //Internet standard newline
#define NBURI_IS_CTL(C)			((C >= 0x00 && C <= 0x1F) || C == 0x7F)	//controls
#define NBURI_IS_DIGIT(C)		(C >= '0' && C <= '9')		//0-9
#define NBURI_IS_DQUOTE(C)		(C == '\"')					//Double Quote
#define NBURI_IS_HEXDIG(C)		(NBURI_IS_DIGIT(C) || (C >= 'A' && C <= 'F') || (C >= 'a' && C <= 'f'))
#define NBURI_IS_HTAB(C)		(C == 0x09)					//horizontal tab
//define NBURI_IS_LWSP			*(WSP / CRLF WSP)			//linear-white-space rule
#define NBURI_IS_SP(C)			(C == 0x20)					//white space char
#define NBURI_IS_VCHAR(C)		(C >= 0x21 && C <= 0x7E)	//visible (printing) characters
#define NBURI_IS_WSP(C)			(NBURI_IS_SP(C) || NBURI_IS_HTAB(C)) //logic white space

//-----------------
//-- URI RFC3986
//-- [RFC3986], https://tools.ietf.org/html/rfc3986
//-----------------
#define NBURI_IS_GENS_DELIMS(C)	(C == ':' || C == '/' || C == '?' || C == '#' || C == '[' || C == ']' || C == '@') //gen-delims, reserved characters
#define NBURI_IS_SUB_DELIMS(C)	(C == '!' || C == '$' || C == '&' || C == '\'' || C == '(' || C == ')' || C == '*' || C == '+' || C == ',' || C == ';' || C == '=') //sub-delims, reserved characters
#define NBURI_IS_RESERVED(C)	(NBURI_IS_GENS_DELIMS(C) || NBURI_IS_SUB_DELIMS(C))
#define NBURI_IS_UNRESERVED(C)	(NBURI_IS_ALPHA(C) || NBURI_IS_DIGIT(C) || C == '-' || C == '.' || C == '_' || C == '~')
#define NBURI_IS_PCHAR(C)		(NBURI_IS_UNRESERVED(C) || NBURI_IS_SUB_DELIMS(C) || C == ':' || C == '@') //includes "pct-encoded"

static const char* strENNBUriParserType[] = {
	"uri"				//scheme ":" hier-part [ "?" query ] [ "#" fragment ]
	, "absoluteUri"		//scheme ":" hier-part [ "?" query ]
	, "scheme"			//ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
	//Pct-encoded
	, "pctEncoded"		//'%' HEXDIG HEXDIG
	//Hier part
	, "hierPart"		//"//" authority path-abempty" path-absolute" path-rootless or path-empty
	//Note: future parsing of path-rule, should considere the literal definition:
	//      "The path is terminated by the first question mark ("?")
	//      or number sign ("#") character, or by the end of the URI."
	//      That way the end-of-the-URI will be determinated by the "feed" caller (to consider the container delimiters only).
	, "pathAbEmpty"		//*( "/" segment )
	, "pathAbsolute"	//"/" [ segment-nz *( "/" segment ) ]
	, "pathNoScheme"	//segment-nz-nc *( "/" segment )
	, "pathRootless"	//segment-nz *( "/" segment )
	, "pathEmpty"		//0<pchar>
	//Authority
	//Note: future parsing of auth-rule, should considere the literal definition:
	//      "The authority component is preceded by a double slash ("//") and is
	//      terminated by the next slash ("/"), question mark ("?"), or number
	//      sign ("#") character, or by the end of the URI."
	//      That way the end-of-the-URI will be determinated by the "feed" caller (to consider the container delimiters only).
	, "authority"		//[ userinfo "@" ] host [ ":" port ]
	//Path components
	, "segment"			//*(pchar)
	, "segment_nz"		//1*(pchar)
	, "segment_nz_nc"	//1*(unreserved / pct-encoded / sub-delims / "@")
	//User info
	, "userinfo"		//*( unreserved / pct-encoded / sub-delims / ":" )
	//Host
	, "port"			//*DIGIT
	, "host"			//IP-literal / IPv4address / reg-name
	, "IPLiteral"		//"[" ( IPv6address / IPvFuture  ) "]"
	, "IPvFuture"		//"v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )
	, "IPv6address"		//(only h16, ":" or ls32 chars)
	, "ls32"			//( h16 ":" h16 ) / IPv4address
	, "h16"				//1*4HEXDIG
	, "IPV4Address"		//dec-octet "." dec-octet "." dec-octet "." dec-octet
	, "dec_octet"		//0-9" 10-99" 100-199" 200-249" 250-255
	, "reg_name"		//*( unreserved / pct-encoded / sub-delims )
	//Query '?'
	//Note: parsing will ignore this spec and use this:
	//      "The query component is indicated by the first question mark ("?") character
	//      and terminated by a number sign ("#") character or by the end of the URI."
	//      That way the end-of-the-URI will be determinated by the "feed" caller (to consider the container delimiters only).
	, "query"			//*( pchar / "/" / "?" )
	//Fragment '#'
	//Note: parsing will ignore this spec and use this:
	//      "Afragment identifier component is indicated by the presence of a
	//      number sign ("#") character and terminated by the end of the URI."
	//      That way the end-of-the-URI will be determinated by the "feed" caller (to consider the container delimiters only).
	, "fragment"		//*( pchar / "/" / "?" )
	//Unreserved
	, "unreserved"		//*( unreserved ) 			//Not in the specs, used to scape/unscape
};

const char NBUriParser_hexdigs[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

void NBUriParser_init(STNBUriParser* state){
	state->fmtLogicError	= FALSE;
	NBArray_init(&state->nodesStack, sizeof(STNBUriParserNode), NULL);
	state->curNode			= NULL;
	NBString_init(&state->strAcum);
	NBString_init(&state->strPend);
	state->bytesFeed		= 0;
	NBASSERT((sizeof(strENNBUriParserType) / sizeof(strENNBUriParserType[0])) == ENNBUriParserType_count)
}

void NBUriParser_release(STNBUriParser* state){
	state->fmtLogicError	= FALSE;
	NBArray_release(&state->nodesStack);
	state->curNode			= NULL;
	NBString_release(&state->strAcum);
	NBString_release(&state->strPend);
	state->bytesFeed		= 0;
}

//Encoding

BOOL NBUriParser_concatUnencoded(STNBString* dst, const char* encd){
	BOOL r = TRUE;
	UI32 encSz = 0; char hex0 = '\0', hex1 = '\0';
	while(*encd != '\0'){
		if(encSz == 0){
			if(*encd == '%'){
				NBASSERT(encSz == 0)
				encSz++;
			} else {
				NBString_concatByte(dst, *encd);
			}
		} else if(encSz == 1){
			hex0 = *encd;
			if(NBURI_IS_HEXDIG(hex0)){
				encSz++;
			} else {
				r = FALSE; NBASSERT(FALSE)
				break;
			}
		} else {
			NBASSERT(encSz == 2)
			hex1 = *encd;
			if(NBURI_IS_HEXDIG(hex1)){
				NBASSERT(NBURI_IS_HEXDIG(hex0))
				NBASSERT(NBURI_IS_HEXDIG(hex1))
				//Ascii order (0-9), (A-F), (a-f)
				const unsigned char utf8Octet = ((hex0 < 'A' ? hex0 - '0' : hex0 < 'a' ? 10 + (hex0 - 'A') : 10 + (hex0 - 'a')) << 4) + (hex1 < 'A' ? hex1 - '0' : hex1 < 'a' ? 10 + (hex1 - 'A') : 10 + (hex1 - 'a'));
				//PRINTF_INFO("UriParser, '%s' converted to %d (%d signed).\n", &state->strAcum.str[state->curNode->rng.iStart], utf8Octet, (char)utf8Octet);
				NBString_concatByte(dst, (char)utf8Octet);
				encSz = 0;
			} else {
				r = FALSE; NBASSERT(FALSE)
				break;
			}
		}
		encd++;
	}
	return (r && encSz == 0);
}

BOOL NBUriParser_concatUnencodedBytes(STNBString* dst, const char* encd, const UI32 encdSz){
	BOOL r = TRUE;
	UI32 encSz = 0; char hex0 = '\0', hex1 = '\0';
	UI32 i; for(i = 0; i < encdSz; i++){
		const char c = encd[i];
		if(encSz == 0){
			if(c == '%'){
				NBASSERT(encSz == 0)
				encSz++;
			} else {
				NBString_concatByte(dst, c);
			}
		} else if(encSz == 1){
			hex0 = c;
			if(NBURI_IS_HEXDIG(hex0)){
				encSz++;
			} else {
				r = FALSE; NBASSERT(FALSE)
				break;
			}
		} else {
			NBASSERT(encSz == 2)
			hex1 = c;
			if(NBURI_IS_HEXDIG(hex1)){
				NBASSERT(NBURI_IS_HEXDIG(hex0))
				NBASSERT(NBURI_IS_HEXDIG(hex1))
				//Ascii order (0-9), (A-F), (a-f)
				const unsigned char utf8Octet = ((hex0 < 'A' ? hex0 - '0' : hex0 < 'a' ? 10 + (hex0 - 'A') : 10 + (hex0 - 'a')) << 4) + (hex1 < 'A' ? hex1 - '0' : hex1 < 'a' ? 10 + (hex1 - 'A') : 10 + (hex1 - 'a'));
				//PRINTF_INFO("UriParser, '%s' converted to %d (%d signed).\n", &state->strAcum.str[state->curNode->rng.iStart], utf8Octet, (char)utf8Octet);
				NBString_concatByte(dst, (char)utf8Octet);
				encSz = 0;
			} else {
				r = FALSE; NBASSERT(FALSE)
				break;
			}
		}
	}
	return (r && encSz == 0);
}

BOOL NBUriParser_concatEncoded(STNBString* dst, const char* unenc, const ENNBUriParserType type){
	BOOL r = FALSE;
	const UI32 iStart = dst->length;
	switch(type){
		case ENNBUriParserType_scheme:
			//scheme: ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
			if(*unenc != '\0'){
				const char c = *unenc;
				if(NBURI_IS_ALPHA(c)){
					r = TRUE;
					NBString_concatByte(dst, c);
					unenc++;
					do {
						const char c = *unenc;
						if(!NBURI_IS_ALPHA(c) && !NBURI_IS_DIGIT(c) && c != '+' && c != '-' && c != '.'){
							r = FALSE; //NBASSERT(FALSE)
							break;
						}
						NBString_concatByte(dst, c);
						unenc++;
					} while(*unenc != '\0');
				}
			}
			break;
		case ENNBUriParserType_userInfo:
			//userInfo: *( unreserved / pct-encoded / sub-delims / ":" )
			{
				r = TRUE;
				do {
					const char c = *unenc;
					if(NBURI_IS_UNRESERVED(c) || NBURI_IS_SUB_DELIMS(c) || c == ':'){
						NBString_concatByte(dst, c);
					} else {
						NBString_concatByte(dst, '%');
						NBString_concatByte(dst, NBUriParser_hexdigs[((c >> 4) & 0xF)]);
						NBString_concatByte(dst, NBUriParser_hexdigs[(c & 0xF)]);
					}
					unenc++;
				} while(*unenc != '\0');
			}
			break;
		case ENNBUriParserType_segment:
			//segment: *(pchar)
			r = TRUE;
			do {
				const char c = *unenc;
				if(NBURI_IS_PCHAR(c)){
					NBString_concatByte(dst, c);
				} else {
					NBString_concatByte(dst, '%');
					NBString_concatByte(dst, NBUriParser_hexdigs[((c >> 4) & 0xF)]);
					NBString_concatByte(dst, NBUriParser_hexdigs[(c & 0xF)]);
				}
				unenc++;
			} while(*unenc != '\0');
			break;
		case ENNBUriParserType_segmentNz:
			//segment: 1*(pchar)
			if(*unenc != '\0'){
				r = TRUE;
				do {
					const char c = *unenc;
					if(NBURI_IS_PCHAR(c)){
						NBString_concatByte(dst, c);
					} else {
						NBString_concatByte(dst, '%');
						NBString_concatByte(dst, NBUriParser_hexdigs[((c >> 4) & 0xF)]);
						NBString_concatByte(dst, NBUriParser_hexdigs[(c & 0xF)]);
					}
					unenc++;
				} while(*unenc != '\0');
			}
			break;
		case ENNBUriParserType_segmentNzNc:
			//segment: 1*(unreserved / pct-encoded / sub-delims / "@")
			if(*unenc != '\0'){
				r = TRUE;
				do {
					const char c = *unenc;
					if(NBURI_IS_UNRESERVED(c) || NBURI_IS_SUB_DELIMS(c) || c == '@'){
						NBString_concatByte(dst, c);
					} else {
						NBString_concatByte(dst, '%');
						NBString_concatByte(dst, NBUriParser_hexdigs[((c >> 4) & 0xF)]);
						NBString_concatByte(dst, NBUriParser_hexdigs[(c & 0xF)]);
					}
					unenc++;
				} while(*unenc != '\0');
			}
			break;
		case ENNBUriParserType_IPv6Address:
			//IPv6address: (only h16, ":" or ls32 chars)
			//ls32:			( h16 ":" h16 ) / IPv4address
			//h16:			1*4HEXDIG
			if(*unenc != '\0'){
				r = TRUE;
				do {
					const char c = *unenc;
					if(NBURI_IS_HEXDIG(c) || c == ':' || NBURI_IS_DIGIT(c) || c == '.'){
						NBString_concatByte(dst, c);
					} else {
						r = FALSE;
						break;
					}
					unenc++;
				} while(*unenc != '\0');
			}
			break;
		case ENNBUriParserType_IPvFuture:
			//IPvFuture: "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )
			if(*unenc != '\0'){
				r = TRUE;
				do {
					const char c = *unenc;
					if(c == 'v' || NBURI_IS_HEXDIG(c) || c == '.' || NBURI_IS_UNRESERVED(c) || NBURI_IS_SUB_DELIMS(c) || c == ':'){
						NBString_concatByte(dst, c);
					} else {
						r = FALSE;
						break;
					}
					unenc++;
				} while(*unenc != '\0');
			}
			break;
		case ENNBUriParserType_IPV4Address:
			//IPV4Address: dec-octet "." dec-octet "." dec-octet "." dec-octet
			if(*unenc != '\0'){
				r = TRUE;
				do {
					const char c = *unenc;
					if(NBURI_IS_DIGIT(c) || c == '.'){
						NBString_concatByte(dst, c);
					} else {
						r = FALSE;
						break;
					}
					unenc++;
				} while(*unenc != '\0');
			}
			break;
		case ENNBUriParserType_regName:
			//regName: *( unreserved / pct-encoded / "-" / ".")
			{
				r = TRUE;
				do {
					const char c = *unenc;
					if(NBURI_IS_UNRESERVED(c) || c == '-' || c == '.'){
						NBString_concatByte(dst, c);
					} else {
						NBString_concatByte(dst, '%');
						NBString_concatByte(dst, NBUriParser_hexdigs[((c >> 4) & 0xF)]);
						NBString_concatByte(dst, NBUriParser_hexdigs[(c & 0xF)]);
					}
					unenc++;
				} while(*unenc != '\0');
			}
			break;
		case ENNBUriParserType_query:
			//query: *( pchar / "/" / "?" )
			//Note: parsing will ignore this spec and use this:
			//      "The query component is indicated by the first question mark ("?") character
			//      and terminated by a number sign ("#") character or by the end of the URI."
			//      That way the end-of-the-URI will be determinated by the "feed" caller (to consider the container delimiters only).
			{
				r = TRUE;
				do {
					const char c = *unenc;
					if(NBURI_IS_PCHAR(c) || c == '/' || c == '?'){
						NBString_concatByte(dst, c);
					} else {
						NBString_concatByte(dst, '%');
						NBString_concatByte(dst, NBUriParser_hexdigs[((c >> 4) & 0xF)]);
						NBString_concatByte(dst, NBUriParser_hexdigs[(c & 0xF)]);
					}
					unenc++;
				} while(*unenc != '\0');
			}
			break;
		case ENNBUriParserType_fragment:
			//fragment: *( pchar / "/" / "?" / "#" )
			//Note: parsing will ignore this spec and use this:
			//      "Afragment identifier component is indicated by the presence of a
			//      number sign ("#") character and terminated by the end of the URI."
			//      That way the end-of-the-URI will be determinated by the "feed" caller (to consider the container delimiters only).
			{
				r = TRUE;
				do {
					const char c = *unenc;
					if(NBURI_IS_PCHAR(c) || c == '/' || c == '?' || c == '#'){
						NBString_concatByte(dst, c);
					} else {
						NBString_concatByte(dst, '%');
						NBString_concatByte(dst, NBUriParser_hexdigs[((c >> 4) & 0xF)]);
						NBString_concatByte(dst, NBUriParser_hexdigs[(c & 0xF)]);
					}
					unenc++;
				} while(*unenc != '\0');
			}
			break;
		case ENNBUriParserType_unreserved: //Not in the specs, used to scape/unscape
			//*( unreserved )
			{
				r = TRUE;
				do {
					const char c = *unenc;
					if(NBURI_IS_UNRESERVED(c)){
						NBString_concatByte(dst, c);
					} else {
						NBString_concatByte(dst, '%');
						NBString_concatByte(dst, NBUriParser_hexdigs[((c >> 4) & 0xF)]);
						NBString_concatByte(dst, NBUriParser_hexdigs[(c & 0xF)]);
					}
					unenc++;
				} while(*unenc != '\0');
			}
			break;
		default:
			r = FALSE; NBASSERT(FALSE) //Unexpected
			break;
	}
	//Remove added content
	if(!r){
		//NBASSERT(FALSE)
		NBString_removeLastBytes(dst, (dst->length - iStart));
	}
	return r;
}

void NBUriParser_depthStackPush(STNBUriParser* state, const ENNBUriParserType type){
	STNBUriParserNode d;
	d.type				= type;
	d.popped			= FALSE;
	d.iCurPosib			= 0;
	d.iCurPart			= 0;
	d.curPartErr		= FALSE;
	d.iDeepLvl			= 0; if(state->curNode != NULL) d.iDeepLvl = (state->curNode->iDeepLvl + 1);
	d.rng.iStart	= state->strAcum.length;
	d.rng.count		= 0;
	NBArray_addValue(&state->nodesStack, d);
	state->curNode		= NBArray_itmPtrAtIndex(&state->nodesStack, STNBUriParserNode, state->nodesStack.use - 1);
#	ifdef NBURI_PARSER_DBG_PRINT_STACK_CHANGES
	{
		STNBString tabs;
		NBString_init(&tabs);
		NBString_concatRepeatedByte(&tabs, '\t', d.iDeepLvl);
		PRINTF_INFO("%sPUSHED %s.\n", tabs.str, strENNBUriParserType[d.type]);
		NBString_release(&tabs);
	}
#	endif
}

//Pop moves to the next part of current posib.
void NBUriParser_depthStackPop(STNBUriParser* state){
	const STNBUriParserNode* arrStart = NBArray_dataPtr(&state->nodesStack, STNBUriParserNode);
	NBASSERT(state->nodesStack.use > 0 && state->curNode >= arrStart)
	if(state->nodesStack.use > 0 && state->curNode >= arrStart){
		NBASSERT(state->curNode->popped == FALSE)
#		ifdef NBURI_PARSER_DBG_PRINT_STACK_CHANGES
		{
			STNBString tabs, data;
			NBString_init(&tabs);
			NBString_init(&data);
			NBString_concatRepeatedByte(&tabs, '\t', state->curNode->iDeepLvl);
			NBString_concatBytes(&data, &state->strAcum.str[state->curNode->rng.iStart], state->strAcum.length - state->curNode->rng.iStart);
			PRINTF_INFO("%sPOPPED %s = '%s'.\n", tabs.str, strENNBUriParserType[state->curNode->type], data.str);
			NBString_release(&data);
			NBString_release(&tabs);
		}
#		endif
		//Acumm range
		state->curNode->rng.count	= (state->strAcum.length - state->curNode->rng.iStart);
		state->curNode->popped			= TRUE;
		if(state->curNode == arrStart){
			//End-of-stack
			state->curNode = NULL;
		} else {
			//Move to first active parent
			do {
				state->curNode--;
				if(!state->curNode->popped) break;
			} while(state->curNode > arrStart);
			//Move parent to next part
			state->curNode->iCurPart++;
		}
	} else {
		state->fmtLogicError = TRUE; NBASSERT(FALSE)
	}
}

//Remove activates the 'error' flag at parent.
void NBUriParser_depthStackRemove(STNBUriParser* state){
	const STNBUriParserNode* arrStart = NBArray_dataPtr(&state->nodesStack, STNBUriParserNode);
	const STNBUriParserNode* arrAfterEnd = arrStart + state->nodesStack.use;
	NBASSERT(state->nodesStack.use > 0 && state->curNode >= arrStart)
	if(state->nodesStack.use > 0 && state->curNode >= arrStart){
		const STNBUriParserNode attmpd = *state->curNode;
		NBASSERT(state->curNode->popped == FALSE)
#		ifdef NBURI_PARSER_DBG_PRINT_STACK_CHANGES
		const UI32 dbgRemoved = (UI32)(arrAfterEnd - state->curNode);
#		endif
		//Remove from stack
		NBArray_removeItemsAtIndex(&state->nodesStack, (SI32)(state->curNode - arrStart), (SI32)(arrAfterEnd - state->curNode)); NBASSERT((arrAfterEnd - state->curNode) > 0)
		if(state->curNode == arrStart){
			//End of stack
			state->curNode = NULL;
		} else {
			//Move to first active parent
			do {
				state->curNode--;
				if(!state->curNode->popped) break;
			} while(state->curNode > arrStart);
			//Set parent's error flag
			state->curNode->curPartErr = TRUE;
		}
		//Recover unused chars
		{
			const UI32 iStart = attmpd.rng.iStart;
			NBASSERT(iStart <= state->strAcum.length)
			if(iStart < state->strAcum.length){
				NBString_concatBytes(&state->strPend, &state->strAcum.str[iStart], (state->strAcum.length - iStart));
				NBString_removeLastBytes(&state->strAcum, (state->strAcum.length - iStart));
			}
		}
#		ifdef NBURI_PARSER_DBG_PRINT_STACK_CHANGES
		{
			STNBString tabs;
			NBString_init(&tabs);
			NBString_concatRepeatedByte(&tabs, '\t', attmpd.iDeepLvl);
			if(dbgRemoved > 1){
				if(state->strPend.length == 0){
					PRINTF_INFO("%sFAILED %s (+%d).\n", tabs.str, strENNBUriParserType[attmpd.type], dbgRemoved - 1);
				} else {
					PRINTF_INFO("%sFAILED %s (+%d), pending '%s'.\n", tabs.str, strENNBUriParserType[attmpd.type], dbgRemoved - 1, state->strPend.str);
				}
			} else {
				if(state->strPend.length == 0){
					PRINTF_INFO("%sFAILED %s.\n", tabs.str, strENNBUriParserType[attmpd.type]);
				} else {
					PRINTF_INFO("%sFAILED %s, pending '%s'.\n", tabs.str, strENNBUriParserType[attmpd.type], state->strPend.str);
				}
			}
			NBString_release(&tabs);
		}
#		endif
	} else {
		state->fmtLogicError = TRUE; NBASSERT(FALSE)
	}
}

//Reset should be like "remove + push" without affecting parent.
//Reset moves to the next posibility.
void NBUriParser_depthStackReset(STNBUriParser* state){
	const STNBUriParserNode* arrStart = NBArray_dataPtr(&state->nodesStack, STNBUriParserNode);
	const STNBUriParserNode* arrAfterEnd = arrStart + state->nodesStack.use;
	NBASSERT(state->nodesStack.use > 0 && state->curNode >= arrStart)
	if(state->nodesStack.use > 0 && state->curNode >= arrStart){
		const STNBUriParserNode attmpd = *state->curNode;
		NBASSERT(state->curNode->popped == FALSE)
#		ifdef NBURI_PARSER_DBG_PRINT_STACK_CHANGES
		const UI32 dbgRemoved = (UI32)(arrAfterEnd - state->curNode - 1);
#		endif
		//Remove childs from stack
		NBArray_removeItemsAtIndex(&state->nodesStack, (SI32)(state->curNode - arrStart + 1), (SI32)(arrAfterEnd - state->curNode - 1)); NBASSERT((arrAfterEnd - state->curNode) > 0)
		//Recover unused chars
		{
			const UI32 iStart = attmpd.rng.iStart;
			NBASSERT(iStart <= state->strAcum.length)
			if(iStart < state->strAcum.length){
				NBString_concatBytes(&state->strPend, &state->strAcum.str[iStart], (state->strAcum.length - iStart));
				NBString_removeLastBytes(&state->strAcum, (state->strAcum.length - iStart));
			}
		}
		//Reset values
		{
			STNBUriParserNode* d = state->curNode;
			d->iCurPosib		= d->iCurPosib + 1;
			d->iCurPart			= 0;
			d->curPartErr		= FALSE;
		}
#		ifdef NBURI_PARSER_DBG_PRINT_STACK_CHANGES
		{
			STNBString tabs;
			NBString_init(&tabs);
			NBString_concatRepeatedByte(&tabs, '\t', attmpd.iDeepLvl);
			if(dbgRemoved > 0){
				if(state->strPend.length == 0){
					PRINTF_INFO("%sRESETING %s (+%d removed).\n", tabs.str, strENNBUriParserType[attmpd.type], dbgRemoved);
				} else {
					PRINTF_INFO("%sRESETING %s (+%d removed), pending '%s'.\n", tabs.str, strENNBUriParserType[attmpd.type], dbgRemoved, state->strPend.str);
				}
			} else {
				if(state->strPend.length == 0){
					PRINTF_INFO("%sRESETING %s.\n", tabs.str, strENNBUriParserType[attmpd.type]);
				} else {
					PRINTF_INFO("%sRESETING %s, pending '%s'.\n", tabs.str, strENNBUriParserType[attmpd.type], state->strPend.str);
				}
			}
			NBString_release(&tabs);
		}
#		endif
	} else {
		state->fmtLogicError = TRUE; NBASSERT(FALSE)
	}
}

BOOL NBUriParser_feedStart(STNBUriParser* state){
	return NBUriParser_feedStartWithType(state, ENNBUriParserType_uri);
}

BOOL NBUriParser_feedStartWithType(STNBUriParser* state, const ENNBUriParserType rootType){
	//Init values
	state->fmtLogicError	= FALSE;
	NBArray_empty(&state->nodesStack);
	NBString_empty(&state->strAcum);
	NBString_empty(&state->strPend);
	//
	NBUriParser_depthStackPush(state, rootType);
	//
	return TRUE;
}

BOOL NBUriParser_feedEnd(STNBUriParser* state){
	//Flush
	NBUriParser_feedChar(state, '\0');
	return NBUriParser_isCompleted(state);
}

BOOL NBUriParser_isCompleted(STNBUriParser* state){
	BOOL r = FALSE;
	if(!state->fmtLogicError){ //Format must be right
		if(state->nodesStack.use > 0){ //Must be not empty
			if(state->curNode == NULL){ //Must have no active node
				r = TRUE;
			}
		}
	}
	return r;
}

BOOL NBUriParser_feedChar(STNBUriParser* state, const char c){
	BOOL r = FALSE;
	//First, feed unused chars
	if(state->curNode != NULL && !state->fmtLogicError && state->strPend.length > 0){
		//Swap pending string
		STNBString str = state->strPend;
		NBString_init(&state->strPend);
		UI32 i; for(i = 0; i < str.length && state->curNode != NULL && !state->fmtLogicError; i++){
			const char c = str.str[i];
			if(!NBUriParser_feedChar(state, c)){
				state->fmtLogicError = TRUE; NBASSERT(FALSE)
				break;
			}
		}
		NBString_release(&str);
	}
	//Now, feed char
	if(state->curNode != NULL && !state->fmtLogicError){
		NBASSERT(state->curNode->rng.iStart <= state->strAcum.length)
		const UI8 iCurPosib = state->curNode->iCurPosib;
		const UI8 iCurPart = state->curNode->iCurPart;
		switch (state->curNode->type) {
			//-------------
			//- Uri
			//-------------
			case ENNBUriParserType_uri: //scheme ":" hier-part [ "?" query ] [ "#" fragment ]
			case ENNBUriParserType_absoluteUri:	//scheme ":" hier-part [ "?" query ]
				switch(iCurPart){
					case 0: //scheme
						if(!state->curNode->curPartErr){
							//Expecting scheme
							NBUriParser_depthStackPush(state, ENNBUriParserType_scheme);
							r = NBUriParser_feedChar(state, c);
						} else {
							//Error found
							NBUriParser_depthStackRemove(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					case 1: //":"
						if(c == ':'){
							NBString_concatByte(&state->strAcum, c); r = TRUE;
							state->curNode->iCurPart = iCurPart + 1;
						} else {
							//Error found
							NBUriParser_depthStackRemove(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					case 2: //hier-part
						if(!state->curNode->curPartErr){
							//Expecting hierpart
							NBUriParser_depthStackPush(state, ENNBUriParserType_hierPart);
							r = NBUriParser_feedChar(state, c);
						} else {
							//Error found
							NBUriParser_depthStackRemove(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					case 3: //"?" or "#"
						if(c == '?'){
							//Expecting query
							NBString_concatByte(&state->strAcum, c); r = TRUE;
							state->curNode->iCurPart = iCurPart + 1;
						} else if(c == '#'){
							//Expecting fragment
							NBString_concatByte(&state->strAcum, c); r = TRUE;
							state->curNode->iCurPart = iCurPart + 3; NBASSERT(state->curNode->iCurPart == 6)
						} else {
							//End-of-uri
							NBUriParser_depthStackPop(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					case 4: //query
						if(!state->curNode->curPartErr){
							//Expecting hierpart
							NBUriParser_depthStackPush(state, ENNBUriParserType_query);
							r = NBUriParser_feedChar(state, c);
						} else {
							//Error found
							NBUriParser_depthStackRemove(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					case 5: //"#"
						if(state->curNode->type == ENNBUriParserType_absoluteUri){
							//Completed all parts
							NBUriParser_depthStackPop(state);
							r = NBUriParser_feedChar(state, c);
						} else {
							if(c == '#'){
								//Expecting fragment
								NBString_concatByte(&state->strAcum, c); r = TRUE;
								state->curNode->iCurPart = iCurPart + 1;
							} else {
								//End-of-uri
								NBUriParser_depthStackPop(state);
								r = NBUriParser_feedChar(state, c);
							}
						}
						break;
					case 6: //fragment
						if(!state->curNode->curPartErr){
							//Expecting hierpart
							NBUriParser_depthStackPush(state, ENNBUriParserType_fragment);
							r = NBUriParser_feedChar(state, c);
						} else {
							//Error found
							NBUriParser_depthStackRemove(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					default: //Completed all parts
						NBUriParser_depthStackPop(state);
						r = NBUriParser_feedChar(state, c);
						break;
				}
				break;
			//-------------
			//- Scheme
			//-------------
			case ENNBUriParserType_scheme: //ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
				if(state->curNode->rng.iStart == state->strAcum.length){
					//First char must be alpha
					if(NBURI_IS_ALPHA(c)){
						NBString_concatByte(&state->strAcum, c); r = TRUE;
					} else {
						//Failed
						NBUriParser_depthStackRemove(state);
						r = NBUriParser_feedChar(state, c);
					}
				} else {
					//Any other char
					if(NBURI_IS_ALPHA(c) || NBURI_IS_DIGIT(c) || c == '+' || c == '-' || c == '.'){
						NBString_concatByte(&state->strAcum, c); r = TRUE;
					} else{
						//End of scheme
						NBUriParser_depthStackPop(state);
						r = NBUriParser_feedChar(state, c);
					}
				}
				break;
			//-------------
			//- Pct-encoded
			//-------------
			case ENNBUriParserType_pctEncoded: //'%' HEXDIG HEXDIG
				if(NBURI_IS_HEXDIG(c)){
					NBString_concatByte(&state->strAcum, c); r = TRUE;
					const UI32 cAdded = (state->strAcum.length - state->curNode->rng.iStart);
					if(cAdded == 3){
						NBASSERT(NBURI_IS_HEXDIG(state->strAcum.str[state->strAcum.length - 2]))
						NBASSERT(NBURI_IS_HEXDIG(state->strAcum.str[state->strAcum.length - 1]))
						/*{
							const char hex0 = state->strAcum.str[state->strAcum.length - 2]; NBASSERT(NBURI_IS_HEXDIG(hex0))
							const char hex1 = state->strAcum.str[state->strAcum.length - 1]; NBASSERT(NBURI_IS_HEXDIG(hex1))
							//Ascii order (0-9), (A-F), (a-f)
							const unsigned char utf8Octet = ((hex0 < 'A' ? hex0 - '0' : hex0 < 'a' ? 10 + (hex0 - 'A') : 10 + (hex0 - 'a')) << 4) + (hex1 < 'A' ? hex1 - '0' : hex1 < 'a' ? 10 + (hex1 - 'A') : 10 + (hex1 - 'a'));
							//PRINTF_INFO("UriParser, '%s' converted to %d (%d signed).\n", &state->strAcum.str[state->curNode->rng.iStart], utf8Octet, (char)utf8Octet);
							NBString_removeLastBytes(&state->strAcum, 3); //'%' + HEXDIG
							NBString_concatByte(&state->strAcum, (char)utf8Octet);
						}*/
						NBUriParser_depthStackPop(state);
					}
				} else {
					PRINTF_ERROR("UriParser, expected hexdigit after '%%', not '%c'.\n", c);
					state->fmtLogicError = TRUE; NBASSERT(FALSE)
				}
				break;
				//-------------
				//- Hier part
				//-------------
			case ENNBUriParserType_hierPart: //"//" authority path-abempty / path-absolute / path-noscheme / path-rootless / path-empty
				switch(iCurPosib){
					case 0: //"//" authority path-abempty
						switch(iCurPart){
							case 0: //"//"
								if(c == '/'){
									NBString_concatByte(&state->strAcum, c); r = TRUE;
									const UI32 cAdded = (state->strAcum.length - state->curNode->rng.iStart);
									if(cAdded == 2){
										state->curNode->iCurPart = iCurPart + 1;
									}
								} else {
									//Error found (next posibility)
									NBUriParser_depthStackReset(state);
									r = NBUriParser_feedChar(state, c);
								}
								break;
							case 1: //authority
								if(!state->curNode->curPartErr){
									//Expecting authority
									NBUriParser_depthStackPush(state, ENNBUriParserType_authority);
									r = NBUriParser_feedChar(state, c);
								} else {
									//Error found (next posibility)
									NBUriParser_depthStackReset(state);
									r = NBUriParser_feedChar(state, c);
								}
								break;
							case 2: //path-abempty
								if(!state->curNode->curPartErr){
									//Expecting path-abempty
									NBUriParser_depthStackPush(state, ENNBUriParserType_pathAbEmpty);
									r = NBUriParser_feedChar(state, c);
								} else {
									//Error found (next posibility)
									NBUriParser_depthStackReset(state);
									r = NBUriParser_feedChar(state, c);
								}
								break;
							default: //Completed all parts
								NBUriParser_depthStackPop(state);
								r = NBUriParser_feedChar(state, c);
								break;
						}
						break;
					case 1: //path-absolute
						if(iCurPart == 0){
							if(!state->curNode->curPartErr){
								//Expecting path-absolute
								NBUriParser_depthStackPush(state, ENNBUriParserType_pathAbsolute);
								r = NBUriParser_feedChar(state, c);
							} else {
								//Error found (next posibility)
								NBUriParser_depthStackReset(state);
								r = NBUriParser_feedChar(state, c);
							}
						} else {
							NBUriParser_depthStackPop(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					case 2: //path-noscheme
						if(iCurPart == 0){
							if(!state->curNode->curPartErr){
								//Expecting path-noscheme
								NBUriParser_depthStackPush(state, ENNBUriParserType_pathNoScheme);
								r = NBUriParser_feedChar(state, c);
							} else {
								//Error found (next posibility)
								NBUriParser_depthStackReset(state);
								r = NBUriParser_feedChar(state, c);
							}
						} else {
							NBUriParser_depthStackPop(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					case 3: //path-rootless
						if(iCurPart == 0){
							if(!state->curNode->curPartErr){
								//Expecting path-rootless
								NBUriParser_depthStackPush(state, ENNBUriParserType_pathRootless);
								r = NBUriParser_feedChar(state, c);
							} else {
								//Error found (next posibility)
								NBUriParser_depthStackReset(state);
								r = NBUriParser_feedChar(state, c);
							}
						} else {
							NBUriParser_depthStackPop(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					case 4: //path-empty
						if(iCurPart == 0){
							if(!state->curNode->curPartErr){
								//Expecting path-empty
								NBUriParser_depthStackPush(state, ENNBUriParserType_pathEmpty);
								r = NBUriParser_feedChar(state, c);
							} else {
								//Error found (next posibility)
								NBUriParser_depthStackReset(state);
								r = NBUriParser_feedChar(state, c);
							}
						} else {
							NBUriParser_depthStackPop(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					default: //All posibilities tried
						//Failed
						NBUriParser_depthStackRemove(state);
						r = NBUriParser_feedChar(state, c);
						break;
				}
				break;
				//Note: future parsing of path-rule, should considere the literal definition:
				//      "The path is terminated by the first question mark ("?")
				//      or number sign ("#") character, or by the end of the URI."
				//      That way the end-of-the-URI will be determinated by the "feed" caller (to consider the container delimiters only).
			case ENNBUriParserType_pathAbEmpty: //*( "/" segment )
				if(!state->curNode->curPartErr){
					if(c == '/'){
						NBString_concatByte(&state->strAcum, c); r = TRUE;
						NBUriParser_depthStackPush(state, ENNBUriParserType_segment);
					} else {
						NBUriParser_depthStackPop(state);
						r = NBUriParser_feedChar(state, c);
					}
				} else {
					NBUriParser_depthStackRemove(state);
					r = NBUriParser_feedChar(state, c);
				}
				break;
			case ENNBUriParserType_pathAbsolute: //"/" [ segment-nz *( "/" segment ) ]
				switch(iCurPart){
					case 0: //"/"
						if(c == '/'){
							//Expecting pathRootless
							NBString_concatByte(&state->strAcum, c); r = TRUE;
							state->curNode->iCurPart = iCurPart + 1;
						} else {
							//Error
							NBUriParser_depthStackRemove(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					case 1: //segment-nz (optional)
						if(!state->curNode->curPartErr){
							NBUriParser_depthStackPush(state, ENNBUriParserType_segmentNz);
							r = NBUriParser_feedChar(state, c);
						} else {
							//Error
							NBUriParser_depthStackPop(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					case 2: // "/" (optional)
						if(c == '/'){
							NBString_concatByte(&state->strAcum, c); r = TRUE;
							state->curNode->iCurPart = iCurPart + 1;
						} else {
							//Completed
							NBUriParser_depthStackPop(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					case 3: // segment
						if(!state->curNode->curPartErr){
							NBUriParser_depthStackPush(state, ENNBUriParserType_segment);
							r = NBUriParser_feedChar(state, c);
						} else {
							//Error
							NBUriParser_depthStackRemove(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					default: //Return to 2
						state->curNode->iCurPart = 2;
						r = NBUriParser_feedChar(state, c);
						break;
				}
				break;
			case ENNBUriParserType_pathNoScheme: //segment-nz-nc *( "/" segment )
				switch (iCurPart) {
					case 0: //segment-nz-nc
						if(!state->curNode->curPartErr){
							NBUriParser_depthStackPush(state, ENNBUriParserType_segmentNzNc);
							r = NBUriParser_feedChar(state, c);
						} else {
							//Error
							NBUriParser_depthStackRemove(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					case 1: // "/" (optional)
						if(c == '/'){
							NBString_concatByte(&state->strAcum, c); r = TRUE;
							state->curNode->iCurPart = iCurPart + 1;
						} else {
							//Completed
							NBUriParser_depthStackPop(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					case 2: // segment
						if(!state->curNode->curPartErr){
							NBUriParser_depthStackPush(state, ENNBUriParserType_segment);
							r = NBUriParser_feedChar(state, c);
						} else {
							//Error
							NBUriParser_depthStackRemove(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					default: //Return to 1
						state->curNode->iCurPart = 1;
						r = NBUriParser_feedChar(state, c);
						break;
				}
				break;
			case ENNBUriParserType_pathRootless: //segment-nz *( "/" segment )
				switch (iCurPart) {
					case 0: //segment-nz
						if(!state->curNode->curPartErr){
							NBUriParser_depthStackPush(state, ENNBUriParserType_segmentNz);
							r = NBUriParser_feedChar(state, c);
						} else {
							//Error
							NBUriParser_depthStackRemove(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					case 1: // "/" (optional)
						if(c == '/'){
							NBString_concatByte(&state->strAcum, c); r = TRUE;
							state->curNode->iCurPart = iCurPart + 1;
						} else {
							//Completed
							NBUriParser_depthStackPop(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					case 2: // segment
						if(!state->curNode->curPartErr){
							NBUriParser_depthStackPush(state, ENNBUriParserType_segment);
							r = NBUriParser_feedChar(state, c);
						} else {
							//Error
							NBUriParser_depthStackRemove(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					default: //Return to 1
						state->curNode->iCurPart = 1;
						r = NBUriParser_feedChar(state, c);
						break;
				}
				break;
			case ENNBUriParserType_pathEmpty: //""
				//Pop to anything
				NBUriParser_depthStackPop(state);
				r = NBUriParser_feedChar(state, c);
				break;
			case ENNBUriParserType_authority: //[ userinfo "@" ] host [ ":" port ]
				//Authority
				//Note: future parsing of auth-rule, should considere the literal definition:
				//      "The authority component is preceded by a double slash ("//") and is
				//      terminated by the next slash ("/"), question mark ("?"), or number
				//      sign ("#") character, or by the end of the URI."
				//      That way the end-of-the-URI will be determinated by the "feed" caller (to consider the container delimiters only).
				switch (iCurPart) {
					case 0: //userinfo (optional)
						if(!state->curNode->curPartErr){
							NBUriParser_depthStackPush(state, ENNBUriParserType_userInfo);
							r = NBUriParser_feedChar(state, c);
						} else {
							//Error (optional)
							NBUriParser_depthStackReset(state);
							state->curNode->iCurPosib	= iCurPosib;	//Mantain posib
							state->curNode->iCurPart	= iCurPart + 2;	//Move to 'host'part
							r = NBUriParser_feedChar(state, c);
						}
						break;
					case 1: //"@" (optional)
						if(c == '@'){
							NBString_concatByte(&state->strAcum, c); r = TRUE;
							state->curNode->iCurPart	= iCurPart + 1;	//Move to 'host'part
						} else {
							NBUriParser_depthStackReset(state);
							state->curNode->iCurPosib	= iCurPosib;	//Mantain posib
							state->curNode->iCurPart	= iCurPart + 1;	//Move to 'host'part
							r = NBUriParser_feedChar(state, c);
						}
						break;
					case 2: //host
						if(!state->curNode->curPartErr){
							NBUriParser_depthStackPush(state, ENNBUriParserType_host);
							r = NBUriParser_feedChar(state, c);
						} else {
							//Error
							NBUriParser_depthStackRemove(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					case 3: //":" (optional)
						if(c == ':'){
							NBString_concatByte(&state->strAcum, c); r = TRUE;
							state->curNode->iCurPart = iCurPart + 1;
						} else {
							//Completed
							NBUriParser_depthStackPop(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					case 4: //port
						if(!state->curNode->curPartErr){
							NBUriParser_depthStackPush(state, ENNBUriParserType_port);
							r = NBUriParser_feedChar(state, c);
						} else {
							//Error
							NBUriParser_depthStackRemove(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					default: //Completed
						NBUriParser_depthStackPop(state);
						r = NBUriParser_feedChar(state, c);
						break;
				}
				break;
			//-------------
			//- User info
			//-------------
			case ENNBUriParserType_userInfo: //*( unreserved / pct-encoded / sub-delims / ":" )
				if(c == '%'){
					NBUriParser_depthStackPush(state, ENNBUriParserType_pctEncoded);
					NBString_concatByte(&state->strAcum, c); r = TRUE;
				} else if(NBURI_IS_UNRESERVED(c) || NBURI_IS_SUB_DELIMS(c) || c == ':'){
					NBString_concatByte(&state->strAcum, c); r = TRUE;
				} else {
					//Pop and try to consume at parent
					NBUriParser_depthStackPop(state);
					r = NBUriParser_feedChar(state, c);
				}
				break;
			//-------------
			//- Host
			//-------------
			case ENNBUriParserType_port: //*DIGIT
				if(NBURI_IS_DIGIT(c)){
					NBString_concatByte(&state->strAcum, c); r = TRUE;
				} else {
					//Pop and try to consume at parent
					NBUriParser_depthStackPop(state);
					r = NBUriParser_feedChar(state, c);
				}
				break;
			case ENNBUriParserType_host: //IP-literal / IPv4address / reg-name
				switch(iCurPosib) {
					case 0: //IP-literal
						if(iCurPart == 0){
							if(!state->curNode->curPartErr){
								NBUriParser_depthStackPush(state, ENNBUriParserType_IPLiteral);
								r = NBUriParser_feedChar(state, c);
							} else {
								NBUriParser_depthStackReset(state);
								r = NBUriParser_feedChar(state, c);
							}
						} else {
							//Completed
							NBUriParser_depthStackPop(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					case 1: //IPv4address
						if(iCurPart == 0){
							if(!state->curNode->curPartErr){
								NBUriParser_depthStackPush(state, ENNBUriParserType_IPV4Address);
								r = NBUriParser_feedChar(state, c);
							} else {
								NBUriParser_depthStackReset(state);
								r = NBUriParser_feedChar(state, c);
							}
						} else {
							//Completed
							NBUriParser_depthStackPop(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					case 2: //reg-name
						if(iCurPart == 0){
							if(!state->curNode->curPartErr){
								NBUriParser_depthStackPush(state, ENNBUriParserType_regName);
								r = NBUriParser_feedChar(state, c);
							} else {
								NBUriParser_depthStackReset(state);
								r = NBUriParser_feedChar(state, c);
							}
						} else {
							//Completed
							NBUriParser_depthStackPop(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					default: //Tried all psoibilities
						NBUriParser_depthStackRemove(state);
						r = NBUriParser_feedChar(state, c);
						break;
				}
				break;
			case ENNBUriParserType_IPLiteral:	//"[" ( IPv6address / IPvFuture  ) "]"
				if(state->curNode->rng.iStart == state->strAcum.length){
					//First char must be '['
					if(c == '['){
						NBString_concatByte(&state->strAcum, c); r = TRUE;
					} else {
						NBUriParser_depthStackRemove(state);
						r = NBUriParser_feedChar(state, c);
					}
				} else {
					NBString_concatByte(&state->strAcum, c); r = TRUE;
					if(c == ']'){
						NBUriParser_depthStackPop(state);
					}
				}
				break;
			case ENNBUriParserType_ls32: //( h16 ":" h16 ) / IPv4address
				switch (iCurPosib) {
					case 0: //h16 ":" h16
						switch (iCurPart) {
							case 0: //h16
								if(!state->curNode->curPartErr){
									NBUriParser_depthStackPush(state, ENNBUriParserType_h16);
									r = NBUriParser_feedChar(state, c);
								} else {
									NBUriParser_depthStackReset(state);
									r = NBUriParser_feedChar(state, c);
								}
								break;
							case 1: //":"
								if(c == ':'){
									NBString_concatByte(&state->strAcum, c); r = TRUE;
									state->curNode->iCurPart = iCurPart + 1;
								} else {
									NBUriParser_depthStackReset(state);
									r = NBUriParser_feedChar(state, c);
								}
								break;
							case 2: //h16
								if(!state->curNode->curPartErr){
									NBUriParser_depthStackPush(state, ENNBUriParserType_h16);
									r = NBUriParser_feedChar(state, c);
								} else {
									NBUriParser_depthStackReset(state);
									r = NBUriParser_feedChar(state, c);
								}
								break;
							default: //Completed
								NBUriParser_depthStackPop(state);
								r = NBUriParser_feedChar(state, c);
								break;
						}
						break;
					case 1: //IPv4address
						if(iCurPart == 0){
							if(!state->curNode->curPartErr){
								NBUriParser_depthStackPush(state, ENNBUriParserType_IPV4Address);
								r = NBUriParser_feedChar(state, c);
							} else {
								NBUriParser_depthStackReset(state);
								r = NBUriParser_feedChar(state, c);
							}
						} else { //Completed
							NBUriParser_depthStackPop(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					default: //Tried all posibilities
						NBUriParser_depthStackRemove(state);
						r = NBUriParser_feedChar(state, c);
						break;
				}
				break;
			case ENNBUriParserType_h16: //1*4HEXDIG
				if(NBURI_IS_HEXDIG(c)){
					const UI32 dgtsAdded = (state->strAcum.length - state->curNode->rng.iStart);
					NBString_concatByte(&state->strAcum, c); r = TRUE;
					if(dgtsAdded == 3){
						//Pop
						NBUriParser_depthStackPop(state);
					}
					NBASSERT(dgtsAdded < 4) //Program logic error, should never enter here
				} else {
					const UI32 dgtsAdded = (state->strAcum.length - state->curNode->rng.iStart);
					if(dgtsAdded > 0){
						//Pop and try to consume at parent
						NBUriParser_depthStackPop(state);
						r = NBUriParser_feedChar(state, c);
					} else {
						//Empty h16
						//Remove node and feed to parent
						NBUriParser_depthStackRemove(state);
						r = NBUriParser_feedChar(state, c);
					}
				}
				break;
			case ENNBUriParserType_IPV4Address: //dec-octet "." dec-octet "." dec-octet "." dec-octet
				switch (iCurPart) {
					case 0: //dec-octet
					case 2: //dec-octet
					case 4: //dec-octet
					case 6: //dec-octet
						if(!state->curNode->curPartErr){
							NBUriParser_depthStackPush(state, ENNBUriParserType_decOctet);
							r = NBUriParser_feedChar(state, c);
						} else {
							NBUriParser_depthStackRemove(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					case 1: //"."
					case 3: //"."
					case 5: //"."
						if(c == '.'){
							NBString_concatByte(&state->strAcum, c); r = TRUE;
							state->curNode->iCurPart = iCurPart + 1;
						} else {
							NBUriParser_depthStackRemove(state);
							r = NBUriParser_feedChar(state, c);
						}
						break;
					default: //completed
						NBUriParser_depthStackPop(state);
						r = NBUriParser_feedChar(state, c);
						break;
				}
				break;
			case ENNBUriParserType_decOctet: //0-9, 10-99, 100-199, 200-249, 250-255
				if(NBURI_IS_DIGIT(c)){
					const UI32 dgtsAdded = (state->strAcum.length - state->curNode->rng.iStart);
					if(dgtsAdded == 0){
						NBString_concatByte(&state->strAcum, c); r = TRUE;
					} else if(dgtsAdded == 1){
						const char d0 = state->strAcum.str[state->curNode->rng.iStart];
						if(d0 != '0'){
							NBString_concatByte(&state->strAcum, c); r = TRUE;
						} else {
							//PRINTF_ERROR("UriParser, found ditig('%c') after '0' in dec-octets.\n", c);
							//Remove node and feed to parent
							NBUriParser_depthStackRemove(state);
							r = NBUriParser_feedChar(state, c);
						}
					} else if(dgtsAdded == 2){
						const char d0 = state->strAcum.str[state->curNode->rng.iStart];
						const char d1 = state->strAcum.str[state->curNode->rng.iStart + 1];
						if(d0 == '1' || (d0 == '2' && (d1 == '0' || d1 == '1' || d1 == '2' || d1 == '3' || d1 == '4' || (d1 == '5' && c == '0' && c == '1' && c == '2' && c == '3' && c == '4' && c == '5')))){
							//concat last digit and pop
							NBString_concatByte(&state->strAcum, c); r = TRUE;
							NBUriParser_depthStackPop(state);
						} else {
							//PRINTF_ERROR("UriParser, 3 digits dec-octets must be (100-199), (200-249) and (250-255).\n");
							//Remove node and feed to parent
							NBUriParser_depthStackRemove(state);
							r = NBUriParser_feedChar(state, c);
						}
					} else {
						NBASSERT(FALSE) //Program logic error, should never enter here
					}
				} else {
					const UI32 dgtsAdded = (state->strAcum.length - state->curNode->rng.iStart);
					if(dgtsAdded > 0){
						//Pop and try to consume at parent
						NBUriParser_depthStackPop(state);
					} else {
						//Empty octet
						//Remove node and feed to parent
						NBUriParser_depthStackRemove(state);
					}
					r = NBUriParser_feedChar(state, c);
				}
				break;
			case ENNBUriParserType_regName:		//*( unreserved / pct-encoded / "-" / ".")
				if(c == '%'){
					NBUriParser_depthStackPush(state, ENNBUriParserType_pctEncoded);
					NBString_concatByte(&state->strAcum, c); r = TRUE;
				} else if(NBURI_IS_UNRESERVED(c) || c == '-' || c == '.'){
					NBString_concatByte(&state->strAcum, c); r = TRUE;
				} else {
					//Pop and try to consume at parent
					NBUriParser_depthStackPop(state);
					r = NBUriParser_feedChar(state, c);
				}
				break;
				//-------------
				//- Path components
				//-------------
			case ENNBUriParserType_segment: //*(pchar)
				if(c == '%'){
					NBUriParser_depthStackPush(state, ENNBUriParserType_pctEncoded);
					NBString_concatByte(&state->strAcum, c); r = TRUE;
				} else if(NBURI_IS_PCHAR(c)){
					NBString_concatByte(&state->strAcum, c); r = TRUE;
				} else {
					//Pop and try to consume at parent
					NBUriParser_depthStackPop(state);
					r = NBUriParser_feedChar(state, c);
				}
				break;
			case ENNBUriParserType_segmentNz: //1*(pchar)
				if(c == '%'){
					NBUriParser_depthStackPush(state, ENNBUriParserType_pctEncoded);
					NBString_concatByte(&state->strAcum, c); r = TRUE;
				} else if(NBURI_IS_PCHAR(c)){
					NBString_concatByte(&state->strAcum, c); r = TRUE;
				} else {
					const UI32 cAdded = (state->strAcum.length - state->curNode->rng.iStart);
					if(cAdded > 0){
						//Sucess (not empty)
						NBUriParser_depthStackPop(state);
					} else {
						//Failed (empty)
						NBUriParser_depthStackRemove(state);
					}
					r = NBUriParser_feedChar(state, c);
				}
				break;
			case ENNBUriParserType_segmentNzNc: //1*(unreserved / pct-encoded / sub-delims / "@")
				if(c == '%'){
					NBUriParser_depthStackPush(state, ENNBUriParserType_pctEncoded);
					NBString_concatByte(&state->strAcum, c); r = TRUE;
				} else if(NBURI_IS_UNRESERVED(c) || NBURI_IS_SUB_DELIMS(c) || c == '@'){
					NBString_concatByte(&state->strAcum, c); r = TRUE;
				} else {
					const UI32 cAdded = (state->strAcum.length - state->curNode->rng.iStart);
					if(cAdded > 0){
						//Sucess (not empty)
						NBUriParser_depthStackPop(state);
					} else {
						//Failed (empty)
						NBUriParser_depthStackRemove(state);
					}
					r = NBUriParser_feedChar(state, c);
				}
				break;
			//-------------
			//- Query and frangment
			//-------------
			case ENNBUriParserType_query: //*( pchar / "/" / "?" )
				//Note: parsing will ignore this spec and use this:
				//      "The query component is indicated by the first question mark ("?") character
				//      and terminated by a number sign ("#") character or by the end of the URI."
				//      That way the end-of-the-URI will be determinated by the "feed" caller (to consider the container delimiters only).
				if(c != '#' && c != '\0'){
					NBString_concatByte(&state->strAcum, c); r = TRUE;
				} else {
					//Pop and try to consume at parent
					NBUriParser_depthStackPop(state);
					r = NBUriParser_feedChar(state, c);
				}
				/*
				Note:do not use the spec-def for parsing, only for encoding
				if(c == '%'){
					NBUriParser_depthStackPush(state, ENNBUriParserType_pctEncoded);
					NBString_concatByte(&state->strAcum, c); r = TRUE;
				} else if(NBURI_IS_PCHAR(c) || c == '/' || c == '?'){
					NBString_concatByte(&state->strAcum, c); r = TRUE;
				} else {
					//Pop and try to consume at parent
					NBUriParser_depthStackPop(state);
					r = NBUriParser_feedChar(state, c);
				}*/
				break;
			case ENNBUriParserType_fragment: //*( pchar / "/" / "?" / "#" )
				//Note: parsing will ignore this spec and use this:
				//      "Afragment identifier component is indicated by the presence of a
				//      number sign ("#") character and terminated by the end of the URI."
				//      That way the end-of-the-URI will be determinated by the "feed" caller (to consider the container delimiters only).
				if(c != '\0'){
					NBString_concatByte(&state->strAcum, c); r = TRUE;
				} else {
					//Pop and try to consume at parent
					NBUriParser_depthStackPop(state);
					r = NBUriParser_feedChar(state, c);
				}
				/*
				Note:do not use the spec-def for parsing, only for encoding
				if(c == '%'){
					NBUriParser_depthStackPush(state, ENNBUriParserType_pctEncoded);
					NBString_concatByte(&state->strAcum, c); r = TRUE;
				} else if(NBURI_IS_PCHAR(c) || c == '/' || c == '?' || c == '#'){
					NBString_concatByte(&state->strAcum, c); r = TRUE;
				} else {
					//Pop and try to consume at parent
					NBUriParser_depthStackPop(state);
					r = NBUriParser_feedChar(state, c);
				}
				*/
				break;
				//-------------
				//- Unepected
				//-------------
			default:
				PRINTF_ERROR("UriParser, program logic error, unexpected state-type.\n");
				state->fmtLogicError = TRUE; NBASSERT(FALSE) //Program  logic error
				break;
		}
		//Evaluate forced end of stream
		if(c == '\0'){
			if(r == FALSE){ //Only evaluate the first feed of '\0' (ignore others posible parent/recursive calls)
				//No active node, no format error and something in the stack
				r = (state->curNode == NULL && !state->fmtLogicError && state->nodesStack.use > 0);
			}
		}
	}
	return r; //(state->curNode != NULL && !state->fmtLogicError);
}
	
BOOL NBUriParser_feed(STNBUriParser* state, const void* pData, const SI32 dataSz){
	BOOL r = TRUE;
	const char* data = (const char*)pData;
	UI32 iData = 0;
	while(iData < dataSz && !state->fmtLogicError){
		const char c = data[iData++];
		if(!NBUriParser_feedChar(state, c)){
			r = FALSE; NBASSERT(FALSE)
			break;
		}
	}
	//
	return r;
}

//

#ifdef NB_CONFIG_INCLUDE_ASSERTS
BOOL NBUriParser_dbgTestUris(void){
	BOOL r = TRUE;
	const char* uris[] = {
		"https://tools.ietf.org/html/rfc3986#page-17"
		, "foo://example.com:8042/over/there?name=ferret#nose"
		, "urn:example:animal:ferret:nose"
		, "http://lists.w3.org/Archives/Public/www-qa/2001Aug/0000.html"
		, "http://a/b/c/d;p?q"
		//
		, "g:h"
		, "http://a/b/c/g"
		, "http://a/b/c/g"
		, "http://a/b/c/g/"
		, "http://a/g"
		, "http://g"
		, "http://a/b/c/?y"
		, "http://a/b/c/g?y"
		, "http://a/b/c/d;p?q#s"
		, "http://a/b/c/g#s"
		, "http://a/b/c/g?y#s"
		, "http://a/b/c/;x"
		, "http://a/b/c/g;x"
		, "http://a/b/c/g;x?y#s"
		, "http://a/b/c/"
		, "http://a/b/c/"
		, "http://a/b/"
		, "http://a/b/"
		, "http://a/b/g"
		, "http://a/"
		, "http://a/"
		, "http://a/g"
		//
		, "http://a/../g"
		, "http://a/../../g"
		//
		, "http://a/./g"
		, "http://a/../g"
		, "http://a/b/c/g."
		, "http://a/b/c/.g"
		, "http://a/b/c/g.."
		, "http://a/b/c/..g"
		//
		, "http://a/b/g"
		, "http://a/b/c/g/"
		, "http://a/b/c/g/h"
		, "http://a/b/c/h"
		//
		, "http://a/b/c/y"
		//
		, "http://a/b/c/g?y/./x"
		, "http://a/b/c/g?y/../x"
		, "http://a/b/c/g#s/./x"
		, "http://a/b/c/g#s/../x"
		//
		, "https://www.w3.org/2004/04/uri-rel-test.html"
		, "http:g" //for strict parsers
		, "http://a/b/c/g" //for backward compatibility
		//
		, "http://www.w%33.org"
		, "http://www.w3.org"
		, "http://r%C3%A4ksm%C3%B6rg%C3%A5s.josefsson.org"
		, "http://xn--rksmrgs-5wao1o.josefsson.org"
		, "http://%E7%B4%8D%E8%B1%86.w3.mag.keio.ac.jp"
		, "http://xn--99zt52a.w3.mag.keio.ac.jp"
		, "http://www.%E3%81%BB%E3%82%93%E3%81%A8%E3%81%86%E3%81%AB%E3%81%AA%E3%81%8C%E3%81%84%E3%82%8F%E3%81%91%E3%81%AE%E3%82%8F%E3%81%8B%E3%82%89%E3%81%AA%E3%81%84%E3%81%A9%E3%82%81%E3%81%84%E3%82%93%E3%82%81%E3%81%84%E3%81%AE%E3%82%89%E3%81%B9%E3%82%8B%E3%81%BE%E3%81%A0%E3%81%AA%E3%81%8C%E3%81%8F%E3%81%97%E3%81%AA%E3%81%84%E3%81%A8%E3%81%9F%E3%82%8A%E3%81%AA%E3%81%84.w3.mag.keio.ac.jp/"
		, "http://www.xn--n8jaaaaai5bhf7as8fsfk3jnknefdde3fg11amb5gzdb4wi9bya3kc6lra.w3.mag.keio.ac.jp/"
		, "http://%E3%81%BB%E3%82%93%E3%81%A8%E3%81%86%E3%81%AB%E3%81%AA%E3%81%8C%E3%81%84%E3%82%8F%E3%81%91%E3%81%AE%E3%82%8F%E3%81%8B%E3%82%89%E3%81%AA%E3%81%84%E3%81%A9%E3%82%81%E3%81%84%E3%82%93%E3%82%81%E3%81%84%E3%81%AE%E3%82%89%E3%81%B9%E3%82%8B%E3%81%BE%E3%81%A0%E3%81%AA%E3%81%8C%E3%81%8F%E3%81%97%E3%81%AA%E3%81%84%E3%81%A8%E3%81%9F%E3%82%8A%E3%81%AA%E3%81%84.%E3%81%BB%E3%82%93%E3%81%A8%E3%81%86%E3%81%AB%E3%81%AA%E3%81%8C%E3%81%84%E3%82%8F%E3%81%91%E3%81%AE%E3%82%8F%E3%81%8B%E3%82%89%E3%81%AA%E3%81%84%E3%81%A9%E3%82%81%E3%81%84%E3%82%93%E3%82%81%E3%81%84%E3%81%AE%E3%82%89%E3%81%B9%E3%82%8B%E3%81%BE%E3%81%A0%E3%81%AA%E3%81%8C%E3%81%8F%E3%81%97%E3%81%AA%E3%81%84%E3%81%A8%E3%81%9F%E3%82%8A%E3%81%AA%E3%81%84.%E3%81%BB%E3%82%93%E3%81%A8%E3%81%86%E3%81%AB%E3%81%AA%E3%81%8C%E3%81%84%E3%82%8F%E3%81%91%E3%81%AE%E3%82%8F%E3%81%8B%E3%82%89%E3%81%AA%E3%81%84%E3%81%A9%E3%82%81%E3%81%84%E3%82%93%E3%82%81%E3%81%84%E3%81%AE%E3%82%89%E3%81%B9%E3%82%8B%E3%81%BE%E3%81%A0%E3%81%AA%E3%81%8C%E3%81%8F%E3%81%97%E3%81%AA%E3%81%84%E3%81%A8%E3%81%9F%E3%82%8A%E3%81%AA%E3%81%84.w3.mag.keio.ac.jp/"
		, "http://xn--n8jaaaaai5bhf7as8fsfk3jnknefdde3fg11amb5gzdb4wi9bya3kc6lra.xn--n8jaaaaai5bhf7as8fsfk3jnknefdde3fg11amb5gzdb4wi9bya3kc6lra.xn--n8jaaaaai5bhf7as8fsfk3jnknefdde3fg11amb5gzdb4wi9bya3kc6lra.w3.mag.keio.ac.jp/"
		//URI examples (https://www.freeformatter.com/url-parser-query-string-splitter.html)
		, "https://www.freeformatter.com/url-parser-query-string-splitter.html"
		, "ftp://ftp.is.co.za/rfc/rfc1808.txt"
		, "http://www.ietf.org/rfc/rfc2396.txt"
		, "ldap://[2001:db8::7]/c=GB?objectClass?one"
		, "news:comp.infosystems.www.servers.unix"
		, "tel:+1-816-555-1212"
		, "telnet://192.0.2.16:80/"
		, "urn:oasis:names:specification:docbook:dtd:xml:4.1.2"
		//URL examples (https://www.freeformatter.com/url-parser-query-string-splitter.html)
		, "http://www.google.com"
		, "http://foo:bar@w1.superman.com/very/long/path.html?p1=v1&p2=v2#more-details"
		, "https://secured.com:443"
		, "ftp://ftp.bogus.com/~some/path/to/a/file.txt"
		//URN examples (https://www.freeformatter.com/url-parser-query-string-splitter.html)
		, "urn:isbn:0451450523"
		, "urn:ietf:rfc:2648"
		, "urn:uuid:6e8bc430-9c3a-11d9-9669-0800200c9a66"
		//Userinfo examples (https://www.freeformatter.com/url-parser-query-string-splitter.html)
		, "ftp://username:password@host.com/"
		, "ftp://username@host.com/"
		//Other examples (https://www.freeformatter.com/url-parser-query-string-splitter.html)
		, "http://www.foo.bar/?listings.html#section-2"
		, "http://www.foo.bar/segment1/segment2/some-resource.html"
		, "http://www.foo.bar/image-2.html?w=100&h=50"
		, "ftp://ftp.foo.bar/~john/doe?w=100&h=50"
		, "http://www.foo.bar/image.jpg?height=150&width=100"
		, "https://www.secured.com:443/resource.html?id=6e8bc430-9c3a-11d9-9669-0800200c9a66#some-header"
		//Testing path-absolute
		, "ftp:/~john/doe/larry/go/?w=100&h=50"
		//Malformed
		, "google"
		, "google.com"
		, "http://google"
	};
	const UI32 count = (sizeof(uris) / sizeof(uris[0]));
	UI32 i; for(i = 0; i < count; i++){
		if(!NBUriParser_dbgTestUri(uris[i])){
			PRINTF_ERROR("--------------------\n");
			PRINTF_ERROR("MALFORMED URI: '%s'.\n", uris[i]);
			PRINTF_ERROR("--------------------\n");
			r = FALSE;
		}
	}
	return r;
}
#endif

#ifdef NB_CONFIG_INCLUDE_ASSERTS
BOOL NBUriParser_dbgTestUri(const char* uri){
	BOOL r = FALSE;
	STNBUriParser par;
	NBUriParser_init(&par);
	PRINTF_INFO("UriParser, ------------.\n");
	PRINTF_INFO("UriParser, testing '%s'.\n", uri);
	if(NBUriParser_feedStart(&par)){
		if(NBUriParser_feed(&par, uri, NBString_strLenBytes(uri))){
			if(NBUriParser_feedEnd(&par)){
				r = TRUE;
				//Print nodes
				{
					STNBString tabs, data;
					NBString_init(&tabs);
					NBString_init(&data);
					UI32 i; for(i = 0; i < par.nodesStack.use; i++){
						const STNBUriParserNode* node = NBArray_itmPtrAtIndex(&par.nodesStack, STNBUriParserNode, i);
						NBString_empty(&tabs);
						NBString_empty(&data);
						NBString_concatRepeatedByte(&tabs, '\t', node->iDeepLvl);
						if(node->type == ENNBUriParserType_pctEncoded){
							NBString_concatBytes(&data, &par.strAcum.str[node->rng.iStart], node->rng.count);
						} else {
							NBUriParser_concatUnencodedBytes(&data, &par.strAcum.str[node->rng.iStart], node->rng.count);
						}
						PRINTF_INFO("%s%s = '%s'.\n", tabs.str, strENNBUriParserType[node->type], data.str);
					}
					NBString_release(&data);
					NBString_release(&tabs);
				}
			}
		}
	}
	NBUriParser_release(&par);
	return r;
}
#endif



