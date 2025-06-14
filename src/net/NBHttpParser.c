
#include "nb/NBFrameworkPch.h"
#include "nb/core/NBEncoding.h"
#include "nb/core/NBNumParser.h"
#include "nb/net/NBHttpParser.h"

#ifdef NB_CONFIG_INCLUDE_ASSERTS
//#	define NBHTTP_PARSER_DBG_PRINT_STACK_CHANGES
#endif

//-----------------
//-- Syntax Notation; ABNF RFC5234 (Core Rules)
//-- [RFC5234], Appendix B.1: https://tools.ietf.org/html/rfc5234#appendix-B.1
//-----------------
#define NBHTTP_v11_IS_ALPHA(C)		((C >= 'A' && C <= 'Z') || (C >= 'a' && C <= 'z'))
#define NBHTTP_v11_IS_BIT(C)		(C == '0' || C == '1')
#define NBHTTP_v11_IS_CHAR(C)		(C >= 0x01 || C <= 0x7F)	//any 7-bit US-ASCII character excluding NULL
#define NBHTTP_v11_IS_CR(C)			(C == 0x0D)					//carriage return
#define NBHTTP_v11_IS_LF(C)			(C == 0x0A)					//linefeed
#define NBHTTP_v11_IS_CTL(C)		((C >= 0x00 && C <= 0x1F) || C == 0x7F)	//controls
#define NBHTTP_v11_IS_DIGIT(C)		(C >= '0' && C <= '9')		//0-9
#define NBHTTP_v11_IS_DQUOTE(C)		(C == '\"')					//Double Quote
#define NBHTTP_v11_IS_HEXDIG(C)		(NBHTTP_v11_IS_DIGIT(C) || (C >= 'A' && C <= 'F') || (C >= 'a' && C <= 'f'))
#define NBHTTP_v11_IS_HTAB(C)		(C == 0x09)					//horizontal tab
//define NBHTTP_v11_IS_LWSP			*(WSP / CRLF WSP)			//linear-white-space rule
#define NBHTTP_v11_IS_SP(C)			(C == 0x20)					//white space char
#define NBHTTP_v11_IS_VCHAR(C)		(C >= 0x21 && C <= 0x7E)	//visible (printing) characters
#define NBHTTP_v11_IS_WSP(C)		(NBHTTP_v11_IS_SP(C) || NBHTTP_v11_IS_HTAB(C)) //logic white space

//

#define NBHTTP_v11_IS_TCHAR(C)		(NBHTTP_v11_IS_ALPHA(C) || NBHTTP_v11_IS_DIGIT(C) || C=='!' || C == '#' || C == '$' || C == '%' || C == '&' || C == '\'' || C == '*' || C == '+' || C == '-' || C == '.' || C == '^' || C == '_' || C == '`' || C == '|' || C == '~')

//

static const char* strENNBHttpParserType[] = {
	//Message Format
	"httpMessageHeader"	//start-line *( header-field CRLF ) CRLF [ message-body ]
	, "startLine"		//request-line / status-line
	, "requestLine"		//method SP request-target SP HTTP-version CRLF
	, "requestTarget"	//origin-form / absolute-form / authority-form / asterisk-form
	, "method"			//token
	, "statusLine"		//HTTP-version SP status-code SP reason-phrase CRLF
	, "statusCode"		//3DIGIT
	, "reasonPhrase"	//*( HTAB / SP / VCHAR / obs-text )
	//Request-target
	, "originForm"		//absolute-path [ "?" query ]
	, "absoluteForm"	//absolute-URI
	, "authorityForm"	//authority
	, "asteriskForm"	//"*"
	, "absolutePath"	//1*( "/" segment )
	//Protocol Versioning
	, "httpVersion"		//"HTTP" "/" DIGIT "." DIGIT
	//Header fields
	, "headerField"		//field-name ":" OWS field-value OWS
	, "fieldName"		//token
	, "fieldValue"		//*( field-content / obs-fold )
	, "fieldContent"	//field-vchar [ 1*( SP / HTAB / field-vchar ) field-vchar ]
	, "fieldVChar"		//VCHAR / obs-text
	, "obsFold"			//OWS CRLF 1*( SP / HTAB ) ; the folding must be replaced by one-white-space
	//Fields values
	, "token"			//1*tchar
	, "quotedString"	//DQUOTE *( qdtext / quoted-pair ) DQUOTE
	, "qdText"			//HTAB / SP /%x21 / %x23-5B / %x5D-7E / obs-text
	, "obsText"			//%x80-FF
	, "comment"			//"(" *( ctext / quoted-pair / comment ) ")"
	, "ctext"			//HTAB / SP / %x21-27 / %x2A-5B / %x5D-7E / obs-text
	, "quotedPair"		//"\" ( HTAB / SP / VCHAR / obs-text )
	//Content length
	, "contentLength"	//1*DIGIT
	//Transfer encoding
	, "transferEncoding"	//1#transfer-coding == transfer-coding *( OWS """ OWS transfer-coding )
	, "transferCoding"		//"chunked" / "compress" / "deflate" / "gzip" / transfer-extension
	, "transferExtension"	//token *( OWS ";" OWS transfer-parameter )
	, "transferParameter"	//token / token BWS "=" BWS ( token / quoted-string )
	//Message body
	, "chunkedBody"		//*chunk last-chunk trailer-part CRLF
	, "chunk"			//chunk-size [ chunk-ext ] CRLF chunk-data CRLF
	, "chunkSize"		//1*HEXDIG
	, "chunkData"		//1*OCTET
	, "chunkExt"		//*( ";" chunk-ext-name [ "=" chunk-ext-val ] )
	, "chunkExtName"	//token
	, "chunkExtVal"		//token / quoted-string
	, "trailerPart"		//*( header-field CRLF )
};

//Nodes stack
void NBHttpParser_depthStackPush(STNBHttpParser* obj, const ENNBHttpParserType type);
void NBHttpParser_depthStackPop(STNBHttpParser* obj);
void NBHttpParser_depthStackRemove(STNBHttpParser* obj);
void NBHttpParser_depthStackIgnore(STNBHttpParser* obj);
void NBHttpParser_depthStackReset(STNBHttpParser* obj);

//

void NBHttpParser_init(STNBHttpParser* obj){
	NBHttpParser_initWithListener(obj, NULL, NULL);
}

void NBHttpParser_initWithListener(STNBHttpParser* obj, IHttpParserListener* listener, void* listenerParam){
	obj->mode			= ENNBHttpParserMode_lazy;
	obj->usrLogicError	= FALSE;
	obj->fmtLogicError	= FALSE;
	NBArray_init(&obj->nodesStack, sizeof(STNBHttpParserNode), NULL);
	obj->curNode		= NULL;
	NBString_init(&obj->strAcum);
	NBString_init(&obj->strPend);
	obj->bytesFeed		= 0;
	NBASSERT((sizeof(strENNBHttpParserType) / sizeof(strENNBHttpParserType[0])) == ENNBHttpParserType_count)
	NBUriParser_init(&obj->uriParser);
	//Chunk-data-buff
	obj->dataBuff		= NULL;	//Buffer for transfered data (not stored in http-buffer)
	obj->dataBuffPos	= 0;	//Current position in transfered data buffer
	obj->dataBuffSz		= 4096;	//Zero if the chunkData must be ignored
	//Chunk-data
	obj->curChunkNotif	= 0;
	obj->curChunkPos	= 0;
	obj->curChunkSz		= 0;
	obj->lastChunkFound	= FALSE;
	//
	if(listener != NULL){
		obj->listener	= *listener;
	} else {
		NBMemory_set(&obj->listener, 0, sizeof(obj->listener));
	}
	obj->listenerParam	= listenerParam;
}

void NBHttpParser_release(STNBHttpParser* obj){
	obj->mode			= ENNBHttpParserMode_lazy;
	obj->usrLogicError	= FALSE;
	obj->fmtLogicError	= FALSE;
	NBArray_release(&obj->nodesStack);
	obj->curNode		= NULL;
	NBString_release(&obj->strAcum);
	NBString_release(&obj->strPend);
	obj->bytesFeed		= 0;
	NBUriParser_release(&obj->uriParser);
	//Chunk-data-buff
	if(obj->dataBuff != NULL){
		NBMemory_free(obj->dataBuff);
		obj->dataBuff = NULL;
	}
	obj->dataBuffPos	= 0;	//Current position in transfered data buffer
	obj->dataBuffSz		= 0;	//Zero if the chunkData must be ignored
	//Chunk-data
	obj->curChunkNotif	= 0;
	obj->curChunkPos	= 0;
	obj->curChunkSz		= 0;
	obj->lastChunkFound	= FALSE;
	//
	NBMemory_set(&obj->listener, 0, sizeof(obj->listener));
	obj->listenerParam	= NULL;
}

//

ENNBHttpParserMode NBHttpParser_mode(STNBHttpParser* obj){
	return obj->mode;
}

void NBHttpParser_setMode(STNBHttpParser* obj, const ENNBHttpParserMode mode){
	obj->mode = mode;
}

BOOL NBHttpParser_setDataBuffSz(STNBHttpParser* obj, const UI32 sz){
	BOOL r = FALSE;
	if(obj->dataBuff == NULL){
		obj->dataBuffSz = sz; //Zero if the chunkData must be ignored
	}
	return r;
}

//

const STNBHttpParserNode* NBHttpParser_nextNode(const STNBHttpParser* obj, const STNBHttpParserNode* parent, const STNBHttpParserNode* node){
	const STNBHttpParserNode* r = NULL;
	NBASSERT(node != NULL)
	const UI32 afterRngParent			= parent->rng.iStart + parent->rng.count;
	const UI32 afterRng					= node->rng.iStart + node->rng.count; NBASSERT(afterRng <= afterRngParent) //If fails, is not the real parent
	const STNBHttpParserNode* afterEnd	= NBArray_dataPtr(&obj->nodesStack, STNBHttpParserNode) + obj->nodesStack.use;
	const STNBHttpParserNode* next		= node + 1;
	while(next < afterEnd){
		if(next->rng.iStart >= afterRng){
			break;
		}
		next++;
	}
	if(next < afterEnd){
		if((next->rng.iStart + next->rng.count) <= afterRngParent){
			r = next;
		}
	}
	return r;
}

const STNBHttpParserNode* NBHttpParser_nextNodeWithType(const STNBHttpParser* obj, const STNBHttpParserNode* parent, const STNBHttpParserNode* node, const ENNBHttpParserType nextType){
	const STNBHttpParserNode* r = NULL;
	NBASSERT(node != NULL)
	const UI32 afterRngParent			= parent->rng.iStart + parent->rng.count;
	const UI32 afterRng					= node->rng.iStart + node->rng.count; NBASSERT(afterRng <= afterRngParent) //If fails, is not the real parent
	const STNBHttpParserNode* afterEnd	= NBArray_dataPtr(&obj->nodesStack, STNBHttpParserNode) + obj->nodesStack.use;
	const STNBHttpParserNode* next		= node + 1;
	while(next < afterEnd){
		if(next->rng.iStart >= afterRng && next->type == nextType){
			break;
		}
		next++;
	}
	if(next < afterEnd){
		if(next->type == nextType){
			if((next->rng.iStart + next->rng.count) <= afterRngParent){
				r = next;
			}
		}
	}
	return r;
}

//

BOOL NBHttpParser_concatEncoded(STNBString* dst, const char* unenc, const ENNBHttpParserType type){
	BOOL r = FALSE;
	const UI32 iStart = dst->length;
	switch(type){
		case ENNBHttpParserType_method:
		case ENNBHttpParserType_fieldName:
		case ENNBHttpParserType_chunkExtName:
		case ENNBHttpParserType_token:
			//token: 1*tchar
			if(*unenc != '\0'){
				r = TRUE;
				do {
					const char c = *unenc;
					if(!NBHTTP_v11_IS_TCHAR(c)){
						r = FALSE; //NBASSERT(FALSE)
						break;
					}
					NBString_concatByte(dst, c);
					unenc++;
				} while(*unenc != '\0');
			}
			break;
		case ENNBHttpParserType_transferParameter:
		case ENNBHttpParserType_chunkExtVal:
			//token / quoted-string
			{
				if(NBHttpParser_concatEncoded(dst, unenc, ENNBHttpParserType_token)){
					r = TRUE;
				} else {
					NBString_removeLastBytes(dst, (dst->length - iStart));
					//quoted-string: DQUOTE *( qdtext / quoted-pair ) DQUOTE
					//qdText:		 HTAB / SP /%x21 / %x23-5B / %x5D-7E / obs-text
					//obsText:		%x80-FF
					//quotedPair:	"\" ( HTAB / SP / VCHAR / obs-text )
					r = TRUE;
					NBString_concatByte(dst, '\"');
					while(*unenc != '\0'){
						const char c = *unenc;
						if(NBHTTP_v11_IS_HTAB(c) || NBHTTP_v11_IS_SP(c) || c == 0x21 || (c >= 0x23 && c <= 0x5B) || (c >= 0x5D && c <= 0x7E) || (c & 0x80) == 0x80){
							NBString_concatByte(dst, c);
						} else if(NBHTTP_v11_IS_HTAB(c) || NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_VCHAR(c) || (c & 0x80) == 0x80){
							NBString_concatByte(dst, '\\');
							NBString_concatByte(dst, c);
						} else {
							r = FALSE; //NBASSERT(FALSE)
							break;
						}
						unenc++;
					}
					NBString_concatByte(dst, '\"');
				}
			}
			break;
		case ENNBHttpParserType_reasonPhrase:
			r = TRUE;
			while(*unenc != '\0'){
				const char c = *unenc;
				if(NBHTTP_v11_IS_HTAB(c) || NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_VCHAR(c) || (c & 0x80) == 0x80){
					NBString_concatByte(dst, c);
				} else {
					r = FALSE; //NBASSERT(FALSE)
					break;
				}
				unenc++;
			}
			break;
		case ENNBHttpParserType_fieldValue:
			r = TRUE; //empty is allowed
		case ENNBHttpParserType_fieldContent:
			//fieldValue:	*( field-content / obs-fold )
			//fieldContent:	field-vchar [ 1*( SP / HTAB / field-vchar ) field-vchar ]
			//fieldVChar:	VCHAR / obs-text
			//obsFold:		OWS CRLF 1*( SP / HTAB )
			//obsText:		%x80-FF
			{
				char c = *unenc;
				if(NBHTTP_v11_IS_VCHAR(c) || (c & 0x80) == 0x80){ //First char must be
					r = TRUE;
					//First char
					NBString_concatByte(dst, c);
					unenc++;
					//Extra chars
					while(*unenc != '\0'){
						c = *unenc;
						if(NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_HTAB(c) || NBHTTP_v11_IS_VCHAR(c) || (c & 0x80) == 0x80){
							NBString_concatByte(dst, c);
						} else {
							r = FALSE; //NBASSERT(FALSE)
							break;
						}
						unenc++;
					}
					//Last char
					if(!(NBHTTP_v11_IS_VCHAR(c) || (c & 0x80) == 0x80)){
						r = FALSE;
					}
				}
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

BOOL NBHttpParser_concatUnencodedNode(const STNBHttpParser* obj, const STNBHttpParserNode* node, STNBString* dst){
	BOOL r = FALSE;
	const UI32 iStart	= dst->length;
	if(node->type == ENNBHttpParserType_quotedString){
		if(node->rng.count > 1){ //At least the 2 dquotes
			if(obj->strAcum.str[node->rng.iStart] == '\"' && obj->strAcum.str[node->rng.iStart + node->rng.count - 1] == '\"'){
				UI32 iStart = node->rng.iStart + 1; //+1 to ignore the '\"'
				const UI32 afterLastChar = (node->rng.iStart + node->rng.count);
				const STNBHttpParserNode* afterLastNode = NBArray_dataPtr(&obj->nodesStack, STNBHttpParserNode) + obj->nodesStack.use;
				const STNBHttpParserNode* next = (node + 1);
				if(next < afterLastNode){
					if(next->rng.iStart < afterLastChar){
						do {
							//Add prev content
							NBASSERT(iStart <= next->rng.iStart)
							if(iStart < next->rng.iStart){
								NBString_concatBytes(dst, &obj->strAcum.str[iStart], (UI32)(next->rng.iStart - iStart));
								iStart = next->rng.iStart;
							}
							//Add quoted-pair
							if(next->type == ENNBHttpParserType_quotedPair){
								NBASSERT(iStart == next->rng.iStart)
								NBASSERT(next->rng.count == 2) //Must be size 2 ('\' + char)
								NBString_concatByte(dst, obj->strAcum.str[next->rng.iStart + 1]);
								iStart += 2;
							}
							//Next
							next = NBHttpParser_nextNode(obj, node, next);
						} while(next != NULL);
					}
				}
				//Add remaining
				NBASSERT(iStart < afterLastChar) //At least the last '\"' must remain
				if((iStart + 1) < afterLastChar){
					NBString_concatBytes(dst, &obj->strAcum.str[iStart], (UI32)(afterLastChar - iStart - 1)); //-1, ignore the last '\"'
				}
				
			}
		}
	} else {
		const char* str		= &obj->strAcum.str[node->rng.iStart];
		const UI32 strSz	= node->rng.count;
		NBString_concatBytes(dst, str, strSz);
		r = TRUE;
	}
	//Remove added content
	if(!r){
		//NBASSERT(FALSE)
		NBString_removeLastBytes(dst, (dst->length - iStart));
	}
	return r;
}

//Case insensitive comparison
//TCHAR(C)		(NBHTTP_v11_IS_ALPHA(C) || NBHTTP_v11_IS_DIGIT(C) || C=='!' || C == '#' || C == '$' || C == '%' || C == '&' || C == '\'' || C == '*' || C == '+' || C == '-' || C == '.' || C == '^' || C == '_' || C == '`' || C == '|' || C == '~')

BOOL NBHttpParser_isTChar(const char c){	//Token char
	return (NBHTTP_v11_IS_TCHAR(c) ? TRUE : FALSE);
}

//Nodes stack

void NBHttpParser_depthStackPush(STNBHttpParser* obj, const ENNBHttpParserType type){
	STNBHttpParserNode d;
	d.type			= type;
	d.popped		= FALSE;
	d.iCurPosib		= 0;
	d.iCurPart		= 0;
	d.curPartErr	= FALSE;
	d.iDeepLvl		= 0; if(obj->curNode != NULL) d.iDeepLvl = (obj->curNode->iDeepLvl + 1);
	d.rng.iStart	= obj->strAcum.length;
	d.rng.count		= 0;
	NBArray_addValue(&obj->nodesStack, d);
	obj->curNode		= NBArray_itmPtrAtIndex(&obj->nodesStack, STNBHttpParserNode, obj->nodesStack.use - 1);
#	ifdef NBHTTP_PARSER_DBG_PRINT_STACK_CHANGES
	{
		STNBString tabs;
		NBString_init(&tabs);
		NBString_concatRepeatedByte(&tabs, '\t', d.iDeepLvl);
		PRINTF_INFO("%sPUSHED %s.\n", tabs.str, strENNBHttpParserType[d.type]);
		NBString_release(&tabs);
	}
#	endif
}

//Pop moves to the next part of current posib.
void NBHttpParser_depthStackPop(STNBHttpParser* obj){
	const STNBHttpParserNode* arrStart = NBArray_dataPtr(&obj->nodesStack, STNBHttpParserNode);
	NBASSERT(obj->nodesStack.use > 0 && obj->curNode >= arrStart)
	if(obj->nodesStack.use > 0 && obj->curNode >= arrStart){
		NBASSERT(obj->curNode->popped == FALSE)
#		ifdef NBHTTP_PARSER_DBG_PRINT_STACK_CHANGES
		{
			STNBString tabs, data;
			NBString_init(&tabs);
			NBString_init(&data);
			NBString_concatRepeatedByte(&tabs, '\t', obj->curNode->iDeepLvl);
			NBString_concatBytes(&data, &obj->strAcum.str[obj->curNode->rng.iStart], obj->strAcum.length - obj->curNode->rng.iStart);
			if(obj->curNode->type == ENNBHttpParserType_httpMessageHeader || (obj->curNode->type == ENNBHttpParserType_chunkedBody || obj->curNode->type == ENNBHttpParserType_chunk || obj->curNode->type == ENNBHttpParserType_chunkData)){
				PRINTF_INFO("%sPOPPED %s = %d bytes.\n", tabs.str, strENNBHttpParserType[obj->curNode->type], (SI32)(obj->strAcum.length - obj->curNode->rng.iStart));
			} else {
				PRINTF_INFO("%sPOPPED %s = '%s'.\n", tabs.str, strENNBHttpParserType[obj->curNode->type], data.str);
			}
			NBString_release(&data);
			NBString_release(&tabs);
		}
#		endif
		//Acumm range
		obj->curNode->rng.count	= (obj->strAcum.length - obj->curNode->rng.iStart);
		obj->curNode->popped	= TRUE;
		//-----------------------
		//-- Notify popped node
		//-----------------------
		switch(obj->curNode->type){
			case ENNBHttpParserType_httpVersion:
				if(obj->listener.consumeMethod != NULL){
					const char* str		= &obj->strAcum.str[obj->curNode->rng.iStart];
					const UI32 strSz	= obj->curNode->rng.count;
					const UI32 majVer = (str[5] - '0');
					const UI32 minVer = (str[7] - '0');
					if(!(*obj->listener.consumeHttpVer)(obj, obj->curNode, str, strSz, majVer, minVer, obj->listenerParam)){
						obj->usrLogicError = TRUE;
					}
				}
				break;
			case ENNBHttpParserType_method:
				//The method is notified after the SP separator is detected.
				//If not, the parser will notify while testing "HTTP" (of "HTTP/1.1") as method.
				break;
			case ENNBHttpParserType_requestTarget:
				if(obj->listener.consumeRequestTarget != NULL){
					const char* str		= &obj->strAcum.str[obj->curNode->rng.iStart];
					const UI32 strSz	= obj->curNode->rng.count;
					if(!(*obj->listener.consumeRequestTarget)(obj, obj->curNode, str, strSz, obj->listenerParam)){
						obj->usrLogicError = TRUE;
					}
				}
				break;
			case ENNBHttpParserType_requestLine:
				if(obj->listener.consumeRequestLine != NULL){
					//Search nodes: method SP request-target SP HTTP-version CRLF
					const STNBHttpParserNode* nMethod	= (obj->curNode + 1);
					const STNBHttpParserNode* nTarget	= NBHttpParser_nextNodeWithType(obj, obj->curNode, nMethod, ENNBHttpParserType_requestTarget);
					const STNBHttpParserNode* nHttpVer	= NBHttpParser_nextNodeWithType(obj, obj->curNode, nTarget, ENNBHttpParserType_httpVersion);
					NBASSERT(nMethod->type == ENNBHttpParserType_method);
					NBASSERT(nTarget->type == ENNBHttpParserType_requestTarget);
					NBASSERT(nHttpVer->type == ENNBHttpParserType_httpVersion);
					//Analyze some necesary headers
					const char* method		= &obj->strAcum.str[nMethod->rng.iStart];
					const UI32 methodSz		= nMethod->rng.count;
					const char* target		= &obj->strAcum.str[nTarget->rng.iStart];
					const UI32 targetSz		= nTarget->rng.count;
					const char* httpVer		= &obj->strAcum.str[nHttpVer->rng.iStart];
					const UI32 httpVerSz	= nHttpVer->rng.count; NBASSERT(httpVerSz == 8)
					const UI32 majVer 		= (httpVer[5] - '0');
					const UI32 minVer 		= (httpVer[7] - '0');
					if(!(*obj->listener.consumeRequestLine)(obj, obj->curNode, method, methodSz, target, targetSz, httpVer, httpVerSz, majVer, minVer, obj->listenerParam)){
						obj->usrLogicError = TRUE;
					}
				}
				break;
			case ENNBHttpParserType_statusCode:
				if(obj->listener.consumeStatusCode != NULL){
					const char* str		= &obj->strAcum.str[obj->curNode->rng.iStart];
					IF_NBASSERT(const UI32 strSz = obj->curNode->rng.count;) NBASSERT(strSz == 3)
					const UI32 code = ((str[0] - '0') * 100) + ((str[1] - '0') * 10) + (str[2] - '0');
					if(!(*obj->listener.consumeStatusCode)(obj, obj->curNode, code, obj->listenerParam)){
						obj->usrLogicError = TRUE;
					}
				}
				break;
			case ENNBHttpParserType_reasonPhrase:
				if(obj->listener.consumeReasonPhrase != NULL){
					const char* str		= &obj->strAcum.str[obj->curNode->rng.iStart];
					const UI32 strSz	= obj->curNode->rng.count;
					if(!(*obj->listener.consumeReasonPhrase)(obj, obj->curNode, str, strSz, obj->listenerParam)){
						obj->usrLogicError = TRUE;
					}
				}
				break;
			case ENNBHttpParserType_statusLine:
				if(obj->listener.consumeStatusLine != NULL){
					//Search nodes: HTTP-version SP status-code SP reason-phrase CRLF
					const STNBHttpParserNode* nHttpVer	= (obj->curNode + 1);
					const STNBHttpParserNode* nStatus	= NBHttpParser_nextNodeWithType(obj, obj->curNode, nHttpVer, ENNBHttpParserType_statusCode);
					const STNBHttpParserNode* nPhrase	= NBHttpParser_nextNodeWithType(obj, obj->curNode, nStatus, ENNBHttpParserType_reasonPhrase);
					NBASSERT(nHttpVer->type == ENNBHttpParserType_httpVersion);
					NBASSERT(nStatus->type == ENNBHttpParserType_statusCode);
					NBASSERT(nPhrase->type == ENNBHttpParserType_reasonPhrase);
					//Analyze some necesary headers
					const char* httpVer		= &obj->strAcum.str[nHttpVer->rng.iStart];
					const UI32 httpVerSz	= nHttpVer->rng.count; NBASSERT(httpVerSz == 8)
					const UI32 majVer 		= (httpVer[5] - '0');
					const UI32 minVer 		= (httpVer[7] - '0');
					const char* status		= &obj->strAcum.str[nStatus->rng.iStart];
					IF_NBASSERT(const UI32 statusSz = nStatus->rng.count;) NBASSERT(statusSz == 3)
					const UI32 code			= ((status[0] - '0') * 100) + ((status[1] - '0') * 10) + (status[2] - '0');
					const char* phrase		= &obj->strAcum.str[nPhrase->rng.iStart];
					const UI32 phraseSz		= nPhrase->rng.count;
					//Notify
					if(!(*obj->listener.consumeStatusLine)(obj, obj->curNode, httpVer, httpVerSz, majVer, minVer, code, phrase, phraseSz, obj->listenerParam)){
						obj->usrLogicError = TRUE;
					}
				}
				break;
			case ENNBHttpParserType_fieldName:
				if(obj->listener.consumeFieldName != NULL){
					const char* str		= &obj->strAcum.str[obj->curNode->rng.iStart];
					const UI32 strSz	= obj->curNode->rng.count;
					if(!(*obj->listener.consumeFieldName)(obj, obj->curNode, str, strSz, obj->listenerParam)){
						obj->usrLogicError = TRUE;
					}
				}
				break;
			case ENNBHttpParserType_headerField:
				if(obj->listener.consumeFieldLine != NULL){
					//Search nodes
					const STNBHttpParserNode* nFieldName = (obj->curNode + 1);
					const STNBHttpParserNode* nFieldValue = NBHttpParser_nextNodeWithType(obj, obj->curNode, nFieldName, ENNBHttpParserType_fieldValue);
					NBASSERT(nFieldName->type == ENNBHttpParserType_fieldName);
					NBASSERT(nFieldValue->type == ENNBHttpParserType_fieldValue);
					//Analyze some necesary headers
					const char* nameStr	= &obj->strAcum.str[nFieldName->rng.iStart];
					const UI32 nameSz	= nFieldName->rng.count;
					const char* valStr	= &obj->strAcum.str[nFieldValue->rng.iStart];
					const UI32 valSz	= nFieldValue->rng.count;
					//Notify
					if(!(*obj->listener.consumeFieldLine)(obj, obj->curNode, nameStr, nameSz, valStr, valSz, obj->listenerParam)){
						obj->usrLogicError = TRUE;
					}
				}
				break;
			case ENNBHttpParserType_httpMessageHeader:
				if(obj->listener.consumeHeaderEnd != NULL){
					if(!(*obj->listener.consumeHeaderEnd)(obj, obj->listenerParam)){
						obj->usrLogicError = TRUE;
					}
				}
				break;
			//Chunk
			case ENNBHttpParserType_chunkSize:
				{
					//Compute chunk-size
					const char* str		= &obj->strAcum.str[obj->curNode->rng.iStart];
					const UI32 strSz	= obj->curNode->rng.count;
					BOOL succ			= FALSE;
					const UI64 chunkSz	= NBNumParser_hexToUI64Bytes(str, strSz, &succ);
					if(!succ){
						PRINTF_ERROR("HttpParser, chunck hex-size not valid: '%s' (%d-len).\n", str, strSz);
						obj->fmtLogicError = TRUE; NBASSERT(FALSE)
					} else {
						//PRINTF_INFO("Chunck size: %llu.\n", chunkSz);
						NBASSERT(obj->curChunkNotif	== 0)
						NBASSERT(obj->curChunkPos == 0)
						obj->curChunkNotif	= 0;
						obj->curChunkPos	= 0;
						obj->curChunkSz		= chunkSz;
						if(chunkSz == 0){
							obj->lastChunkFound = TRUE;
						}
						if(obj->listener.chunkStarted != NULL){
							if(!(*obj->listener.chunkStarted)(obj, obj->curNode, chunkSz, obj->listenerParam)){
								obj->usrLogicError = TRUE;
							}
						}
					}
				}
				break;
			case ENNBHttpParserType_chunkData:
				{
					const UI64 dataBuffPos		= obj->dataBuffPos;
					const UI64 curChunkPos		= obj->curChunkPos;
					const UI64 curChunkSz		= obj->curChunkSz;
					NBASSERT(obj->curChunkPos > 0 && obj->curChunkPos == obj->curChunkSz)
					//Notify-remaining-data
					if(obj->dataBuffSz > 0){
						NBASSERT(obj->dataBuff != NULL)
						NBASSERT((obj->curChunkNotif + obj->dataBuffPos) == obj->curChunkSz)
						if(!(*obj->listener.consumeChunkData)(obj, obj->curNode, obj->dataBuff, dataBuffPos, curChunkPos, curChunkSz, obj->listenerParam)){
							obj->usrLogicError = TRUE;
						}
					} else {
						NBASSERT(obj->dataBuffPos == 0 && obj->curChunkNotif == 0)
						if(!(*obj->listener.consumeChunkData)(obj, obj->curNode, NULL, curChunkPos, curChunkPos, curChunkSz, obj->listenerParam)){
							obj->usrLogicError = TRUE;
						}
					}
					//Notify-end
					if(obj->listener.chunkEnded != NULL){
						if(!(*obj->listener.chunkEnded)(obj, obj->curNode, curChunkSz, obj->listenerParam)){
							obj->usrLogicError = TRUE;
						}
					}
					//Reset
					obj->dataBuffPos	= 0;
					obj->curChunkNotif	= 0;
					obj->curChunkPos	= 0;
					obj->curChunkSz		= 0;
				}
				break;
			case ENNBHttpParserType_chunkedBody:
				if(obj->listener.consumeChunkedBodyEnd != NULL){
					if(!(*obj->listener.consumeChunkedBodyEnd)(obj, obj->listenerParam)){
						obj->usrLogicError = TRUE;
					}
				}
				break;
			default:
				//Nothing
				break;
		}
		//Define parent
		if(obj->curNode == arrStart){
			//End-of-stack
			obj->curNode = NULL;
		} else {
			//Move to first active parent
			do {
				obj->curNode--;
				if(!obj->curNode->popped) break;
			} while(obj->curNode > arrStart);
			//Move parent to next part
			obj->curNode->iCurPart++;
		}
	} else {
		obj->fmtLogicError = TRUE; NBASSERT(FALSE)
	}
}

//Remove activates the 'error' flag at parent.
void NBHttpParser_depthStackRemove(STNBHttpParser* obj){
	const STNBHttpParserNode* arrStart = NBArray_dataPtr(&obj->nodesStack, STNBHttpParserNode);
	const STNBHttpParserNode* arrAfterEnd = arrStart + obj->nodesStack.use;
	NBASSERT(obj->nodesStack.use > 0 && obj->curNode >= arrStart)
	if(obj->nodesStack.use <= 0 || obj->curNode < arrStart){
		PRINTF_ERROR("HttpParser, all nodes already consumed (removing).\n");
		obj->fmtLogicError = TRUE; NBASSERT(FALSE)
	} else {
		const STNBHttpParserNode attmpd = *obj->curNode;
		NBASSERT(obj->curNode->popped == FALSE)
#		ifdef NBHTTP_PARSER_DBG_PRINT_STACK_CHANGES
		const UI32 dbgRemoved = (UI32)(arrAfterEnd - obj->curNode);
#		endif
		//Remove from stack
		NBArray_removeItemsAtIndex(&obj->nodesStack, (SI32)(obj->curNode - arrStart), (SI32)(arrAfterEnd - obj->curNode)); NBASSERT((arrAfterEnd - obj->curNode) > 0)
		if(obj->curNode == arrStart){
			//End of stack
			obj->curNode = NULL;
		} else {
			//Move to first active parent
			do {
				obj->curNode--;
				if(!obj->curNode->popped) break;
			} while(obj->curNode > arrStart);
			//Set parent's error flag
			obj->curNode->curPartErr = TRUE;
		}
		//Recover unused chars
		{
			const UI32 iStart = attmpd.rng.iStart;
			NBASSERT(iStart <= obj->strAcum.length)
			if(iStart < obj->strAcum.length){
				NBString_concatBytes(&obj->strPend, &obj->strAcum.str[iStart], (obj->strAcum.length - iStart));
				NBString_removeLastBytes(&obj->strAcum, (obj->strAcum.length - iStart));
			}
		}
#		ifdef NBHTTP_PARSER_DBG_PRINT_STACK_CHANGES
		{
			STNBString tabs;
			NBString_init(&tabs);
			NBString_concatRepeatedByte(&tabs, '\t', attmpd.iDeepLvl);
			if(dbgRemoved > 1){
				if(obj->strPend.length == 0){
					PRINTF_INFO("%sFAILED %s (+%d).\n", tabs.str, strENNBHttpParserType[attmpd.type], dbgRemoved - 1);
				} else {
					PRINTF_INFO("%sFAILED %s (+%d), pending '%s'.\n", tabs.str, strENNBHttpParserType[attmpd.type], dbgRemoved - 1, obj->strPend.str);
				}
			} else {
				if(obj->strPend.length == 0){
					PRINTF_INFO("%sFAILED %s.\n", tabs.str, strENNBHttpParserType[attmpd.type]);
				} else {
					PRINTF_INFO("%sFAILED %s, pending '%s'.\n", tabs.str, strENNBHttpParserType[attmpd.type], obj->strPend.str);
				}
			}
			NBString_release(&tabs);
		}
#		endif
	}
}

//Ignore, removes the node and its content, the node is treated as suceess popped.
void NBHttpParser_depthStackIgnore(STNBHttpParser* obj){
	const STNBHttpParserNode* arrStart = NBArray_dataPtr(&obj->nodesStack, STNBHttpParserNode);
	const STNBHttpParserNode* arrAfterEnd = arrStart + obj->nodesStack.use;
	NBASSERT(obj->nodesStack.use > 0 && obj->curNode >= arrStart)
	if(obj->nodesStack.use <= 0 || obj->curNode < arrStart){
		PRINTF_ERROR("HttpParser, all nodes already consumed (ignore).\n");
		obj->fmtLogicError = TRUE; NBASSERT(FALSE)
	} else {
		const STNBHttpParserNode attmpd = *obj->curNode;
		NBASSERT(obj->curNode->popped == FALSE)
#		ifdef NBHTTP_PARSER_DBG_PRINT_STACK_CHANGES
		const UI32 dbgRemoved = (UI32)(arrAfterEnd - obj->curNode);
#		endif
		//Remove from stack
		NBArray_removeItemsAtIndex(&obj->nodesStack, (SI32)(obj->curNode - arrStart), (SI32)(arrAfterEnd - obj->curNode)); NBASSERT((arrAfterEnd - obj->curNode) > 0)
		if(obj->curNode == arrStart){
			//End of stack
			obj->curNode = NULL;
		} else {
			//Move to first active parent
			do {
				obj->curNode--;
				if(!obj->curNode->popped) break;
			} while(obj->curNode > arrStart);
			//Move parent to next part
			obj->curNode->iCurPart++;
		}
		//Remove unused chars
		{
			const UI32 iStart = attmpd.rng.iStart;
			NBASSERT(iStart <= obj->strAcum.length)
			if(iStart < obj->strAcum.length){
				NBString_removeLastBytes(&obj->strAcum, (obj->strAcum.length - iStart));
			}
		}
#		ifdef NBHTTP_PARSER_DBG_PRINT_STACK_CHANGES
		{
			STNBString tabs;
			NBString_init(&tabs);
			NBString_concatRepeatedByte(&tabs, '\t', attmpd.iDeepLvl);
			if(dbgRemoved > 1){
				if(obj->strPend.length == 0){
					PRINTF_INFO("%sIGNORED %s (+%d).\n", tabs.str, strENNBHttpParserType[attmpd.type], dbgRemoved - 1);
				} else {
					PRINTF_INFO("%sIGNORED %s (+%d), pending '%s'.\n", tabs.str, strENNBHttpParserType[attmpd.type], dbgRemoved - 1, obj->strPend.str);
				}
			} else {
				if(obj->strPend.length == 0){
					PRINTF_INFO("%sIGNORED %s.\n", tabs.str, strENNBHttpParserType[attmpd.type]);
				} else {
					PRINTF_INFO("%sIGNORED %s, pending '%s'.\n", tabs.str, strENNBHttpParserType[attmpd.type], obj->strPend.str);
				}
			}
			NBString_release(&tabs);
		}
#		endif
	}
}

//Reset should be like "remove + push" without affecting parent.
//Reset moves to the next posibility.
void NBHttpParser_depthStackReset(STNBHttpParser* obj){
	const STNBHttpParserNode* arrStart = NBArray_dataPtr(&obj->nodesStack, STNBHttpParserNode);
	const STNBHttpParserNode* arrAfterEnd = arrStart + obj->nodesStack.use;
	NBASSERT(obj->nodesStack.use > 0 && obj->curNode >= arrStart)
	if(obj->nodesStack.use <= 0 || obj->curNode < arrStart){
		PRINTF_ERROR("HttpParser, all nodes already consumed (reset).\n");
		obj->fmtLogicError = TRUE; NBASSERT(FALSE)
	} else {
		const STNBHttpParserNode attmpd = *obj->curNode;
		NBASSERT(obj->curNode->popped == FALSE)
#		ifdef NBHTTP_PARSER_DBG_PRINT_STACK_CHANGES
		const UI32 dbgRemoved = (UI32)(arrAfterEnd - obj->curNode - 1);
#		endif
		//Remove childs from stack
		NBArray_removeItemsAtIndex(&obj->nodesStack, (SI32)(obj->curNode - arrStart + 1), (SI32)(arrAfterEnd - obj->curNode - 1)); NBASSERT((arrAfterEnd - obj->curNode) > 0)
		//Recover unused chars
		{
			const UI32 iStart = attmpd.rng.iStart;
			NBASSERT(iStart <= obj->strAcum.length)
			if(iStart < obj->strAcum.length){
				NBString_concatBytes(&obj->strPend, &obj->strAcum.str[iStart], (obj->strAcum.length - iStart));
				NBString_removeLastBytes(&obj->strAcum, (obj->strAcum.length - iStart));
			}
		}
		//Reset values
		{
			STNBHttpParserNode* d = obj->curNode;
			d->iCurPosib		= d->iCurPosib + 1;
			d->iCurPart			= 0;
			d->curPartErr		= FALSE;
		}
#		ifdef NBHTTP_PARSER_DBG_PRINT_STACK_CHANGES
		{
			STNBString tabs;
			NBString_init(&tabs);
			NBString_concatRepeatedByte(&tabs, '\t', attmpd.iDeepLvl);
			if(dbgRemoved > 0){
				if(obj->strPend.length == 0){
					PRINTF_INFO("%sRESETING %s (+%d removed).\n", tabs.str, strENNBHttpParserType[attmpd.type], dbgRemoved);
				} else {
					PRINTF_INFO("%sRESETING %s (+%d removed), pending '%s'.\n", tabs.str, strENNBHttpParserType[attmpd.type], dbgRemoved, obj->strPend.str);
				}
			} else {
				if(obj->strPend.length == 0){
					PRINTF_INFO("%sRESETING %s.\n", tabs.str, strENNBHttpParserType[attmpd.type]);
				} else {
					PRINTF_INFO("%sRESETING %s, pending '%s'.\n", tabs.str, strENNBHttpParserType[attmpd.type], obj->strPend.str);
				}
			}
			NBString_release(&tabs);
		}
#		endif
	}
}

void NBHttpParser_feedStart(STNBHttpParser* obj){
	NBHttpParser_feedStartWithType(obj, ENNBHttpParserType_httpMessageHeader);
}

void NBHttpParser_feedStartWithType(STNBHttpParser* obj, const ENNBHttpParserType rootType){
	//Init values
	obj->usrLogicError	= FALSE;
	obj->fmtLogicError	= FALSE;
	NBArray_empty(&obj->nodesStack);
	NBString_empty(&obj->strAcum);
	NBString_empty(&obj->strPend);
	//Chunk-data
	obj->curChunkNotif	= 0;
	obj->curChunkPos	= 0;
	obj->curChunkSz		= 0;
	obj->lastChunkFound	= FALSE;
	//
	NBHttpParser_depthStackPush(obj, rootType);
}

BOOL NBHttpParser_feedIsComplete(STNBHttpParser* obj){
	BOOL r = FALSE;
	if(!obj->usrLogicError){ //User listener never returned FALSE
		if(!obj->fmtLogicError){ //Format must be right
			if(obj->nodesStack.use > 0){ //Must be not empty
				if(obj->curNode == NULL){ //Must have no active node
					if(obj->curChunkSz == 0){ //Must have no chunk open
						r = TRUE;
					}
				}
			}
		}
	}
	return r;
}

BOOL NBHttpParser_feedEnd(STNBHttpParser* obj){
	//Flush
	NBHttpParser_feedByte(obj, '\0');
	return NBHttpParser_feedIsComplete(obj);
}

BOOL NBHttpParser_feedByte(STNBHttpParser* obj, const char c){
	BOOL r = FALSE;
	//First, feed unused chars
	if(obj->curNode != NULL && !obj->usrLogicError && !obj->fmtLogicError && obj->strPend.length > 0){
		//Swap pending string
		STNBString str = obj->strPend;
		NBString_init(&obj->strPend);
		UI32 i; for(i = 0; i < str.length && obj->curNode != NULL && !obj->usrLogicError && !obj->fmtLogicError; i++){
			const char c = str.str[i];
			if(!NBHttpParser_feedByte(obj, c)){
				obj->fmtLogicError = TRUE; //NBASSERT(FALSE)
				break;
			}
		}
		NBString_release(&str);
	}
	//Now, feed char
	if(obj->curNode != NULL && !obj->usrLogicError && !obj->fmtLogicError){
#		ifdef NBHTTP_PARSER_DBG_PRINT_STACK_CHANGES
		/*{
			STNBString tabs;
			NBString_init(&tabs);
			NBString_concatRepeatedByte(&tabs, '\t', obj->curNode->iDeepLvl);
			switch(c) {
				case '\r': PRINTF_INFO("%sCHAR: '\r'.\n", tabs.str); break;
				case '\t': PRINTF_INFO("%sCHAR: '\t'.\n", tabs.str); break;
				case '\n': PRINTF_INFO("%sCHAR: '\n'.\n", tabs.str); break;
				default: PRINTF_INFO("%sCHAR: (%d) '%c'.\n", tabs.str, (SI32)c, c); break;
			}
			NBString_release(&tabs);
		}*/
#		endif
		NBASSERT(obj->curNode->rng.iStart <= obj->strAcum.length)
		const UI8 iCurPosib = obj->curNode->iCurPosib;
		const UI8 iCurPart = obj->curNode->iCurPart;
		switch(obj->curNode->type){
			//-------------
			//- Message Format
			//-------------
			case ENNBHttpParserType_httpMessageHeader:	//start-line *( header-field CRLF ) CRLF
				switch (iCurPart) {
					case 0: //start-line
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_startLine);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 1: //header-field (optional)
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_headerField);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Move to CRLF before body
							obj->curNode->iCurPart = iCurPart + 4;
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 2: //CRLF (CR)
						if(NBHTTP_v11_IS_CR(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 3: //CRLF (LF)
						if(NBHTTP_v11_IS_LF(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
							//End-of-header-field
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 4:
						//Try another header-field
						obj->curNode->iCurPart = iCurPart - 3;
						r = NBHttpParser_feedByte(obj, c);
						break;
					case 5: //CRLF (CR)
						if(NBHTTP_v11_IS_CR(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					default: //CRLF (LF)
						if(NBHTTP_v11_IS_LF(c)){
							//Completed
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							NBHttpParser_depthStackPop(obj);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
				}
				break;
			case ENNBHttpParserType_startLine:		//request-line / status-line
				switch (iCurPosib) {
					case 0: //request-line
						if(iCurPart == 0){
							if(!obj->curNode->curPartErr){
								NBHttpParser_depthStackPush(obj, ENNBHttpParserType_requestLine);
								r = NBHttpParser_feedByte(obj, c);
							} else {
								//Err, next posib
								NBHttpParser_depthStackReset(obj);
								r = NBHttpParser_feedByte(obj, c);
							}
						} else {
							//Completed
							NBHttpParser_depthStackPop(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 1: //status-line
						if(iCurPart == 0){
							if(!obj->curNode->curPartErr){
								NBHttpParser_depthStackPush(obj, ENNBHttpParserType_statusLine);
								r = NBHttpParser_feedByte(obj, c);
							} else {
								//Err, next posib
								NBHttpParser_depthStackReset(obj);
								r = NBHttpParser_feedByte(obj, c);
							}
						} else {
							//Completed
							NBHttpParser_depthStackPop(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					default:
						//All posib tested
						NBHttpParser_depthStackRemove(obj);
						r = NBHttpParser_feedByte(obj, c);
						break;
				}
				break;
			case ENNBHttpParserType_requestLine:	//method SP request-target SP HTTP-version CRLF
				switch (iCurPart) {
					case 0: //method
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_method);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 1: //SP
						if(NBHTTP_v11_IS_SP(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
							//The method is notified after the SP separator is detected.
							//If not, the parser will notify while testing "HTTP" (of "HTTP/1.1") as method.
							if(obj->listener.consumeMethod != NULL){
								const char* str		= &obj->strAcum.str[obj->curNode->rng.iStart];
								const UI32 strSz	= (obj->strAcum.length - obj->curNode->rng.iStart - 1); //-1 to not include this space
								if(!(*obj->listener.consumeMethod)(obj, obj->curNode, str, strSz, obj->listenerParam)){
									obj->usrLogicError = TRUE;
								}
							}
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 2: //request-target
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_requestTarget);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 3: //SP
						if(NBHTTP_v11_IS_SP(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 4: //HTTP-version
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_httpVersion);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 5: //CRLF (CR)
						if(NBHTTP_v11_IS_CR(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					default:
						//CRLF (LF)
						if(NBHTTP_v11_IS_LF(c)){
							//Completed
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							NBHttpParser_depthStackPop(obj);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
				}
				break;
			case ENNBHttpParserType_requestTarget:
				switch (obj->mode) {
					case ENNBHttpParserMode_lazy:
						//anything until SP
						if(!NBHTTP_v11_IS_CTL(c) && !NBHTTP_v11_IS_SP(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
						} else {
							if(obj->curNode->rng.iStart == obj->strAcum.length){
								//Error (at least one char required)
								NBHttpParser_depthStackRemove(obj);
							} else {
								//Completed
								NBHttpParser_depthStackPop(obj);
							}
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case ENNBHttpParserMode_soft:
					case ENNBHttpParserMode_strict:
						//origin-form / absolute-form / authority-form / asterisk-form
						switch (iCurPosib) {
							case 0: //origin-form
								if(iCurPart == 0){
									if(!obj->curNode->curPartErr){
										NBHttpParser_depthStackPush(obj, ENNBHttpParserType_originForm);
										r = NBHttpParser_feedByte(obj, c);
									} else {
										//Err, next posib
										NBHttpParser_depthStackReset(obj);
										r = NBHttpParser_feedByte(obj, c);
									}
								} else {
									//Completed
									NBHttpParser_depthStackPop(obj);
									r = NBHttpParser_feedByte(obj, c);
								}
								break;
							case 1: //absolute-form
								if(iCurPart == 0){
									if(!obj->curNode->curPartErr){
										NBHttpParser_depthStackPush(obj, ENNBHttpParserType_absoluteForm);
										r = NBHttpParser_feedByte(obj, c);
									} else {
										//Err, next posib
										NBHttpParser_depthStackReset(obj);
										r = NBHttpParser_feedByte(obj, c);
									}
								} else {
									//Completed
									NBHttpParser_depthStackPop(obj);
									r = NBHttpParser_feedByte(obj, c);
								}
								break;
							case 2: //authority-form
								if(iCurPart == 0){
									if(!obj->curNode->curPartErr){
										NBHttpParser_depthStackPush(obj, ENNBHttpParserType_authorityForm);
										r = NBHttpParser_feedByte(obj, c);
									} else {
										//Err, next posib
										NBHttpParser_depthStackReset(obj);
										r = NBHttpParser_feedByte(obj, c);
									}
								} else {
									//Completed
									NBHttpParser_depthStackPop(obj);
									r = NBHttpParser_feedByte(obj, c);
								}
								break;
							case 3: //asterisk-form
								if(iCurPart == 0){
									if(!obj->curNode->curPartErr){
										NBHttpParser_depthStackPush(obj, ENNBHttpParserType_asteriskForm);
										r = NBHttpParser_feedByte(obj, c);
									} else {
										//Err, next posib
										NBHttpParser_depthStackReset(obj);
										r = NBHttpParser_feedByte(obj, c);
									}
								} else {
									//Completed
									NBHttpParser_depthStackPop(obj);
									r = NBHttpParser_feedByte(obj, c);
								}
								break;
							default:
								//All posib tested
								NBHttpParser_depthStackRemove(obj);
								r = NBHttpParser_feedByte(obj, c);
								break;
						}
						break;
					default:
						NBASSERT(FALSE)
						break;
				}
				break;
			case ENNBHttpParserType_method:			//token
				if(iCurPart == 0){
					if(!obj->curNode->curPartErr){
						NBHttpParser_depthStackPush(obj, ENNBHttpParserType_token);
						r = NBHttpParser_feedByte(obj, c);
					} else {
						//Error
						NBHttpParser_depthStackRemove(obj);
						r = NBHttpParser_feedByte(obj, c);
					}
				} else {
					//Completed
					NBHttpParser_depthStackPop(obj);
					r = NBHttpParser_feedByte(obj, c);
				}
				break;
			case ENNBHttpParserType_statusLine:		//HTTP-version SP status-code SP reason-phrase CRLF
				switch (iCurPart) {
					case 0: //HTTP-version
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_httpVersion);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 1: //SP
					case 3: //SP
						if(NBHTTP_v11_IS_SP(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 2: //status-code
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_statusCode);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 4: //reason-phrase
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_reasonPhrase);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 5: //CRLF (CR)
						if(NBHTTP_v11_IS_CR(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					default: //CRLF (LF)
						if(NBHTTP_v11_IS_LF(c)){
							//Completed
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							NBHttpParser_depthStackPop(obj);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
				}
				break;
			case ENNBHttpParserType_statusCode:		//3DIGIT
				if(NBHTTP_v11_IS_DIGIT(c)){
					NBString_concatByte(&obj->strAcum, c); r = TRUE;
					const UI32 cAdded = (obj->strAcum.length - obj->curNode->rng.iStart);
					if(cAdded == 3){
						NBHttpParser_depthStackPop(obj);
					}
				} else {
					//Error
					NBHttpParser_depthStackRemove(obj);
					r = NBHttpParser_feedByte(obj, c);
				}
				break;
			case ENNBHttpParserType_reasonPhrase:	//*( HTAB / SP / VCHAR / obs-text )
				if(iCurPosib == 0){
					if(NBHTTP_v11_IS_HTAB(c) || NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_VCHAR(c)){
						NBString_concatByte(&obj->strAcum, c); r = TRUE;
					} else {
						obj->curNode->iCurPosib = iCurPosib + 1;
						NBHttpParser_depthStackPush(obj, ENNBHttpParserType_obsText);
						r = NBHttpParser_feedByte(obj, c);
					}
				} else {
					if(!obj->curNode->curPartErr){
						obj->curNode->curPartErr = FALSE;
						obj->curNode->iCurPosib = iCurPosib - 1;
						r = NBHttpParser_feedByte(obj, c);
					} else {
						//Completed
						NBHttpParser_depthStackPop(obj);
						r = NBHttpParser_feedByte(obj, c);
					}
				}
				break;
			//-------------
			//- Request-target
			//-------------
			case ENNBHttpParserType_originForm:		//absolute-path [ "?" query ]
				if(iCurPart == 0){ //absolute-path
					//Start URI Parser
					if(obj->strAcum.length == obj->curNode->rng.iStart){
						NBUriParser_feedStartWithType(&obj->uriParser, ENNBUriParserType_absoluteUri);
					}
					BOOL cConsumd = FALSE;
					if(c != '?' && !NBHTTP_v11_IS_SP(c)){
						if(NBUriParser_feedChar(&obj->uriParser, c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							cConsumd = TRUE;
						}
					}
					if(!cConsumd){
						if(NBUriParser_feedEnd(&obj->uriParser)){
							//Was completed (before this char)
							if(c == '?'){
								//Continue with 'query'
								NBString_concatByte(&obj->strAcum, c); r = TRUE;
								NBUriParser_feedStartWithType(&obj->uriParser, ENNBUriParserType_query);
								obj->curNode->iCurPart = iCurPart + 1;
								cConsumd = TRUE;
							} else {
								//Pop
								NBHttpParser_depthStackPop(obj);
							}
						} else {
							//Error, is not completed
							NBHttpParser_depthStackRemove(obj);
						}
						if(!cConsumd){
							r = NBHttpParser_feedByte(obj, c);
						}
					}
				} else { //query
					BOOL cConsumd = FALSE;
					if(!NBHTTP_v11_IS_SP(c)){
						if(NBUriParser_feedChar(&obj->uriParser, c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							cConsumd = TRUE;
						}
					}
					if(!cConsumd){
						if(NBUriParser_feedEnd(&obj->uriParser)){
							//Pop
							NBHttpParser_depthStackPop(obj);
						} else {
							//Error, is not completed
							NBHttpParser_depthStackRemove(obj);
						}
						r = NBHttpParser_feedByte(obj, c);
					}
				}
				break;
			case ENNBHttpParserType_absoluteForm:	//absolute-URI
				//Start URI Parser
				if(obj->strAcum.length == obj->curNode->rng.iStart){
					NBUriParser_feedStartWithType(&obj->uriParser, ENNBUriParserType_absoluteUri);
				}
				//Feed URI parser
				if(NBUriParser_feedChar(&obj->uriParser, c)){
					NBString_concatByte(&obj->strAcum, c); r = TRUE;
				} else {
					if(NBUriParser_feedEnd(&obj->uriParser)){
						//Was completed (before this char)
						NBHttpParser_depthStackPop(obj);
					} else {
						//Error, is not completed
						NBHttpParser_depthStackRemove(obj);
					}
					r = NBHttpParser_feedByte(obj, c);
				}
				break;
			case ENNBHttpParserType_authorityForm:	//authority
				//Start URI Parser
				if(obj->strAcum.length == obj->curNode->rng.iStart){
					NBUriParser_feedStartWithType(&obj->uriParser, ENNBUriParserType_authority);
				}
				//Feed URI parser
				if(NBUriParser_feedChar(&obj->uriParser, c)){
					NBString_concatByte(&obj->strAcum, c); r = TRUE;
				} else {
					if(NBUriParser_feedEnd(&obj->uriParser)){
						//Was completed (before this char)
						NBHttpParser_depthStackPop(obj);
					} else {
						//Error, is not completed
						NBHttpParser_depthStackRemove(obj);
					}
					r = NBHttpParser_feedByte(obj, c);
				}
				break;
			case ENNBHttpParserType_asteriskForm:	//"*"
				if(c == '*'){
					//Completed
					NBString_concatByte(&obj->strAcum, c); r = TRUE;
					NBHttpParser_depthStackPop(obj);
				} else {
					//Error
					NBHttpParser_depthStackRemove(obj);
					r = NBHttpParser_feedByte(obj, c);
				}
				break;
			case ENNBHttpParserType_absolutePath:	//1*( "/" segment )
				if(iCurPart == 0){
					if(c == '/'){
						NBString_concatByte(&obj->strAcum, c); r = TRUE;
						obj->curNode->iCurPart = iCurPart + 1;
						//Start URI Parser
						NBUriParser_feedStartWithType(&obj->uriParser, ENNBUriParserType_segment);
					} else {
						if(obj->curNode->rng.iStart == obj->strAcum.length){
							//Error (at least one required)
							NBHttpParser_depthStackRemove(obj);
						} else {
							//Completed
							NBHttpParser_depthStackPop(obj);
						}
						r = NBHttpParser_feedByte(obj, c);
					}
				} else {
					if(NBUriParser_feedChar(&obj->uriParser, c)){
						NBString_concatByte(&obj->strAcum, c); r = TRUE;
					} else {
						if(NBUriParser_feedEnd(&obj->uriParser)){
							//Was completed (before this char)
							//Return to 0
							obj->curNode->iCurPart = 0;
						} else {
							//Error, is not completed
							NBHttpParser_depthStackRemove(obj);
						}
						r = NBHttpParser_feedByte(obj, c);
					}
				}
				break;
			//-------------
			//- Protocol Versioning
			//-------------
			case ENNBHttpParserType_httpVersion:	//"HTTP" "/" DIGIT "." DIGIT
				{
					const UI32 cAdded = (obj->strAcum.length - obj->curNode->rng.iStart);
					BOOL isValid = FALSE;
					switch (cAdded) {
						case 0: isValid = (c == 'H'); break;
						case 1: isValid = (c == 'T'); break;
						case 2: isValid = (c == 'T'); break;
						case 3: isValid = (c == 'P'); break;
						case 4: isValid = (c == '/'); break;
						case 5: isValid = NBHTTP_v11_IS_DIGIT(c); break;
						case 6: isValid = (c == '.'); break;
						case 7: isValid = NBHTTP_v11_IS_DIGIT(c); break;
#						ifdef NB_CONFIG_INCLUDE_ASSERTS
						default:
							NBASSERT(FALSE) 
							break;
#						endif
					}
					if(isValid){
						NBString_concatByte(&obj->strAcum, c); r = TRUE;
						if(cAdded == 7){
							//Completed
							NBHttpParser_depthStackPop(obj);
						}
					} else {
						//Error
						NBHttpParser_depthStackRemove(obj);
						r = NBHttpParser_feedByte(obj, c);
					}
				}
				break;
			//-------------
			//- Header fields
			//-------------
			case ENNBHttpParserType_headerField:	//field-name ":" OWS field-value OWS
				switch (iCurPart) {
					case 0: //field-name
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_fieldName);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 1: //":"
						if(c == ':'){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 2: //OWS, OptionalWhiteSpace: *( SP / HTAB )
					case 4: //OWS, OptionalWhiteSpace: *( SP / HTAB )
						if(NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_HTAB(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
						} else {
							obj->curNode->iCurPart = iCurPart + 1;
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 3: //field-value
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_fieldValue);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					default: //Completed
						NBHttpParser_depthStackPop(obj);
						r = NBHttpParser_feedByte(obj, c);
						break;
				}
				break;
			case ENNBHttpParserType_fieldName:		//token
				if(iCurPart == 0){
					if(!obj->curNode->curPartErr){
						NBHttpParser_depthStackPush(obj, ENNBHttpParserType_token);
						r = NBHttpParser_feedByte(obj, c);
					} else {
						//Error
						NBHttpParser_depthStackRemove(obj);
						r = NBHttpParser_feedByte(obj, c);
					}
				} else {
					//Completed
					NBHttpParser_depthStackPop(obj);
					r = NBHttpParser_feedByte(obj, c);
				}
				break;
			case ENNBHttpParserType_fieldValue:		//*( field-content / obs-fold )
				if(iCurPosib == 0){
					if(iCurPart == 0){
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_fieldContent);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Try next posib
							obj->curNode->curPartErr	= FALSE;
							obj->curNode->iCurPosib	= iCurPosib + 1;
							obj->curNode->iCurPart	= 0;
							r = NBHttpParser_feedByte(obj, c);
						}
					} else {
						//Try again
						obj->curNode->curPartErr	= FALSE;
						obj->curNode->iCurPosib	= 0;
						obj->curNode->iCurPart	= 0;
						r = NBHttpParser_feedByte(obj, c);
					}
				} else {
					if(iCurPart == 0){
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_obsFold);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//All posibilities evaluated
							NBHttpParser_depthStackPop(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
					} else {
						//Try again 0
						obj->curNode->curPartErr	= FALSE;
						obj->curNode->iCurPosib	= 0;
						obj->curNode->iCurPart	= 0;
						r = NBHttpParser_feedByte(obj, c);
					}
				}
				break;
			case ENNBHttpParserType_fieldContent:	//field-vchar [ 1*( SP / HTAB / field-vchar ) field-vchar ]
				switch(obj->mode) {
					case ENNBHttpParserMode_lazy:
						if(!NBHTTP_v11_IS_CTL(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
						} else {
							//Cannot end in (SP / HTAB), if so, remove it so can be feed to parent.
							char lastC = '\0'; BOOL lastRemoved = FALSE;
							if(obj->curNode->rng.iStart < obj->strAcum.length){
								lastC = obj->strAcum.str[obj->strAcum.length - 1];
							}
							if(NBHTTP_v11_IS_SP(lastC) || NBHTTP_v11_IS_HTAB(lastC)){
								NBString_removeLastBytes(&obj->strAcum, 1);
								lastRemoved = TRUE;
							}
							//Process
							if(obj->curNode->rng.iStart == obj->strAcum.length){
								//Error (at least one required)
								NBHttpParser_depthStackRemove(obj);
							} else {
								//Completed
								NBHttpParser_depthStackPop(obj);
							}
							if(lastRemoved){
								r = NBHttpParser_feedByte(obj, lastC);
								if(r){
									r = NBHttpParser_feedByte(obj, c);
								}
							} else {
								r = NBHttpParser_feedByte(obj, c);
							}
						}
						break;
					case ENNBHttpParserMode_soft:
					case ENNBHttpParserMode_strict:
						//ToDo, this implementation does not works for obs-fold preceded by space.
						//Example: "Transfer-Encoding: gzip, deflate \r\n ,chunked\r\n"
						//                                          |- this space fails.
						//Example: "Transfer-Encoding: gzip, deflate\r\n ,chunked\r\n"
						//                                          |- this works.
						switch (iCurPart) {
							case 0: //field-vchar
								if(!obj->curNode->curPartErr){
									NBHttpParser_depthStackPush(obj, ENNBHttpParserType_fieldVChar);
									r = NBHttpParser_feedByte(obj, c);
								} else {
									//Error
									NBHttpParser_depthStackRemove(obj);
									r = NBHttpParser_feedByte(obj, c);
								}
								break;
								//--------------------
								//- 1*( SP / HTAB / field-vchar )
								//- First optional:
								//--------------------
							case 1: //(SP / HTAB)
								if(NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_HTAB(c)){
									NBString_concatByte(&obj->strAcum, c); r = TRUE;
									obj->curNode->iCurPart = 6; //Try extra optionals after an space
								} else {
									//Try first field-vchar
									obj->curNode->iCurPart = iCurPart + 1;
									r = NBHttpParser_feedByte(obj, c);
								}
								break;
							case 2: //field-vchar
								if(!obj->curNode->curPartErr){
									NBHttpParser_depthStackPush(obj, ENNBHttpParserType_fieldVChar);
									r = NBHttpParser_feedByte(obj, c);
								} else {
									//Completed (no optional content)
									NBHttpParser_depthStackPop(obj);
									r = NBHttpParser_feedByte(obj, c);
								}
								break;
								//--------------------
								//- Extra optionals after a field-vchar:
								//--------------------
							case 3: //(SP / HTAB)
								if(NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_HTAB(c)){
									NBString_concatByte(&obj->strAcum, c); r = TRUE;
									obj->curNode->iCurPart = 6; //Try extra optionals after an space
								} else {
									//Try extra optional field-vchar
									obj->curNode->iCurPart = iCurPart + 1;
									r = NBHttpParser_feedByte(obj, c);
								}
								break;
							case 4: //field-vchar
								if(!obj->curNode->curPartErr){
									NBHttpParser_depthStackPush(obj, ENNBHttpParserType_fieldVChar);
									r = NBHttpParser_feedByte(obj, c);
								} else {
									//Completed (last field-vchar was the last-one)
									NBHttpParser_depthStackPop(obj);
									r = NBHttpParser_feedByte(obj, c);
								}
								break;
							case 5:
								obj->curNode->iCurPart = 3; //Try extra optionals after an field-vchar
								r = NBHttpParser_feedByte(obj, c);
								break;
								//--------------------
								//- Extra optionals after an space:
								//--------------------
							case 6: //(SP / HTAB)
								if(NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_HTAB(c)){
									NBString_concatByte(&obj->strAcum, c); r = TRUE;
									obj->curNode->iCurPart = 6; //Try extra optionals after an space
								} else {
									//Try extra optional field-vchar
									obj->curNode->iCurPart = iCurPart + 1;
									r = NBHttpParser_feedByte(obj, c);
								}
								break;
							case 7: //field-vchar
								if(!obj->curNode->curPartErr){
									NBHttpParser_depthStackPush(obj, ENNBHttpParserType_fieldVChar);
									r = NBHttpParser_feedByte(obj, c);
								} else {
									//The preceding white-space was not part of this fieldContent (and can be part of the parent definition)
									const char lastC = obj->strAcum.str[obj->strAcum.length - 1];
									NBASSERT(NBHTTP_v11_IS_SP(lastC) || NBHTTP_v11_IS_HTAB(lastC))
									if(NBHTTP_v11_IS_SP(lastC) || NBHTTP_v11_IS_HTAB(lastC)){
										NBString_removeLastBytes(&obj->strAcum, 1);
										NBHttpParser_depthStackPop(obj);
										NBASSERT(obj->strPend.length == 0)
											//Send the prev whitespace to parent
											r = NBHttpParser_feedByte(obj, lastC);
										if(r){
											r = NBHttpParser_feedByte(obj, c); //Send current char to parent
										}
									} else {
										//Program logic error ()
										obj->fmtLogicError = TRUE; r = FALSE;
									}
								}
								break;
							case 8:
								obj->curNode->iCurPart = 3; //Try extra optionals after an field-vchar
								r = NBHttpParser_feedByte(obj, c);
								break;
#							ifdef NB_CONFIG_INCLUDE_ASSERTS
							default:
								NBASSERT(FALSE) //Should never reach here
								break;
#							endif
						}
						break;
					default:
						NBASSERT(FALSE)
						break;
				}
				break;
			case ENNBHttpParserType_fieldVChar:		//VCHAR / obs-text
				if(iCurPosib == 0){ //VCHAR
					if(NBHTTP_v11_IS_VCHAR(c)){
						NBString_concatByte(&obj->strAcum, c); r = TRUE;
						//Completed
						NBHttpParser_depthStackPop(obj);
					} else {
						//Try next posib
						obj->curNode->iCurPosib = iCurPosib + 1;
						r = NBHttpParser_feedByte(obj, c);
					}
				} else { //obs-text
					if(iCurPart == 0){
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_obsText);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//All posib tried
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
					} else {
						//Completed
						NBHttpParser_depthStackPop(obj);
						r = NBHttpParser_feedByte(obj, c);
					}
				}
				break;
			case ENNBHttpParserType_obsFold:		//OWS CRLF 1*( SP / HTAB )
				switch (iCurPart) {
					case 0: //OWS, OptionalWhiteSpace: *( SP / HTAB )
						if(NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_HTAB(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
						} else {
							obj->curNode->iCurPart = iCurPart + 1;
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 1: //CRLF (CR)
						if(NBHTTP_v11_IS_CR(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 2: //CRLF (LF)
						if(NBHTTP_v11_IS_LF(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 3: //"1*( SP / HTAB )" //First (SP / HTAB)
						if(NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_HTAB(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					default: //"1*( SP / HTAB )" //Extras (SP / HTAB)
						if(NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_HTAB(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
						} else {
							//Completed
							//Replace all this node for a single-white-space
							NBHttpParser_depthStackIgnore(obj);
							NBASSERT(obj->strPend.length == 0)
							if(NBHttpParser_feedByte(obj, ' ')){ //Feed the replacement single-white-space
								NBASSERT(obj->strPend.length == 0)
								r = NBHttpParser_feedByte(obj, c); //Feed the current char
							}
						}
						break;
				}
				break;
			//-------------
			//-Fields values
			//-------------
			case ENNBHttpParserType_token:			//1*tchar
				if(NBHTTP_v11_IS_TCHAR(c)){
					NBString_concatByte(&obj->strAcum, c); r = TRUE;
				} else {
					if(obj->curNode->rng.iStart == obj->strAcum.length){
						//Error
						NBHttpParser_depthStackRemove(obj);
						r = NBHttpParser_feedByte(obj, c);
					} else {
						//Completed
						NBHttpParser_depthStackPop(obj);
						r = NBHttpParser_feedByte(obj, c);
					}
				}
				break;
			case ENNBHttpParserType_quotedString:	//DQUOTE *( qdtext / quoted-pair ) DQUOTE
				switch (iCurPart) {
					case 0: //DQUOTE
						if(NBHTTP_v11_IS_DQUOTE(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 1: //qdtext (optional)
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_qdText);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Try next
							obj->curNode->curPartErr = FALSE;
							obj->curNode->iCurPart = iCurPart + 2;
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 2:
						//Retry qdtext
						obj->curNode->iCurPart = iCurPart - 1;
						r = NBHttpParser_feedByte(obj, c);
						break;
					case 3: //quoted-pair (optional)
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_quotedPair);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Move to last DQUOTE
							obj->curNode->curPartErr = FALSE;
							obj->curNode->iCurPart = iCurPart + 2;
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 4: //Go back to 1
						obj->curNode->iCurPart = 1;
						r = NBHttpParser_feedByte(obj, c);
						break;
					case 5: //DQUOTE
						if(NBHTTP_v11_IS_DQUOTE(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					default: //Completed
						NBHttpParser_depthStackPop(obj);
						r = NBHttpParser_feedByte(obj, c);
						break;
				}
				break;
			case ENNBHttpParserType_qdText:			//HTAB / SP /%x21 / %x23-5B / %x5D-7E / obs-text
				if(iCurPosib == 0){
					if(NBHTTP_v11_IS_HTAB(c) || NBHTTP_v11_IS_SP(c) || c == 0x21 || (c >= 0x23 && c <= 0x5B) || (c >= 0x5D && c <= 0x7E)){
						NBString_concatByte(&obj->strAcum, c); r = TRUE;
						NBHttpParser_depthStackPop(obj);
					} else {
						obj->curNode->iCurPosib = iCurPosib + 1;
						r = NBHttpParser_feedByte(obj, c);
					}
				} else if(iCurPosib == 1){
					if(iCurPart == 0){
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_obsText);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
					} else {
						//Completed
						NBHttpParser_depthStackPop(obj);
						r = NBHttpParser_feedByte(obj, c);
					}
				} else {
					NBASSERT(FALSE) //Never should happend
				}
				break;
			case ENNBHttpParserType_obsText:		//%x80-FF
				if((c & 0x80) == 0x80){ //c >= 0x80 && c <= 0xFF
					NBString_concatByte(&obj->strAcum, c); r = TRUE;
					//Completed
					NBHttpParser_depthStackPop(obj);
				} else {
					//Error
					NBHttpParser_depthStackRemove(obj);
					r = NBHttpParser_feedByte(obj, c);
				}
				break;
			case ENNBHttpParserType_comment:		//"(" *( ctext / quoted-pair / comment ) ")"
				switch(iCurPart) {
					case 0: // "("
						if(c == '('){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 1: //ctext (optional)
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_ctext);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Try next
							obj->curNode->iCurPart = iCurPart + 2;
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 2: //go back to 1
						obj->curNode->iCurPart = 1;
						r = NBHttpParser_feedByte(obj, c);
						break;
					case 3: //quoted-pair (optional)
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_quotedPair);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Try next
							obj->curNode->iCurPart = iCurPart + 2;
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 4: //go back to 1
						obj->curNode->iCurPart = 1;
						r = NBHttpParser_feedByte(obj, c);
						break;
					case 5: //comment (optional)
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_comment);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Try next
							obj->curNode->iCurPart = iCurPart + 2;
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 6: //go back to 1
						obj->curNode->iCurPart = 1;
						r = NBHttpParser_feedByte(obj, c);
						break;
					case 7: //")"
						if(c == ')'){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					default:
						//Completed
						NBHttpParser_depthStackPop(obj);
						r = NBHttpParser_feedByte(obj, c);
						break;
				}
				break;
			case ENNBHttpParserType_ctext:			//HTAB / SP / %x21-27 / %x2A-5B / %x5D-7E / obs-text
				if(iCurPosib == 0){
					if(NBHTTP_v11_IS_HTAB(c) || NBHTTP_v11_IS_SP(c) || (c == 0x21 && c == 0x27) || (c >= 0x2A && c <= 0x5B) || (c >= 0x5D && c <= 0x7E)){
						NBString_concatByte(&obj->strAcum, c); r = TRUE;
						NBHttpParser_depthStackPop(obj);
					} else {
						obj->curNode->iCurPosib = iCurPosib + 1;
						r = NBHttpParser_feedByte(obj, c);
					}
				} else if(iCurPosib == 1){
					if(iCurPart == 0){
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_obsText);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
					} else {
						//Completed
						NBHttpParser_depthStackPop(obj);
						r = NBHttpParser_feedByte(obj, c);
					}
				} else {
					NBASSERT(FALSE) //Never should happend
				}
				break;
			case ENNBHttpParserType_quotedPair:		//"\" ( HTAB / SP / VCHAR / obs-text )
				switch (iCurPart) {
					case 0:
						if(c == '\\'){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 1:
						if(NBHTTP_v11_IS_HTAB(c) || NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_VCHAR(c)){
							//Completed
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							NBHttpParser_depthStackPop(obj);
						} else {
							//Try next posib
							obj->curNode->iCurPart = iCurPart + 1;
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 2:
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_obsText);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					default:
						//Completed
						NBHttpParser_depthStackPop(obj);
						r = NBHttpParser_feedByte(obj, c);
						break;
				}
				break;
			//-------------
			//- Content length
			//-------------
			case ENNBHttpParserType_contentLength: //1*DIGIT
				if(NBHTTP_v11_IS_DIGIT(c)){
					NBString_concatByte(&obj->strAcum, c); r = TRUE;
				} else {
					if(obj->curNode->rng.iStart == obj->strAcum.length){
						//Error
						NBHttpParser_depthStackRemove(obj);
						r = NBHttpParser_feedByte(obj, c);
					} else {
						//Completed
						NBHttpParser_depthStackPop(obj);
						r = NBHttpParser_feedByte(obj, c);
					}
				}
				break;
			//-------------
			//- Transfer encoding
			//-------------
			case ENNBHttpParserType_transferEncoding:	//1#transfer-coding == transfer-coding *( OWS "," OWS transfer-coding )
				switch(iCurPart) {
					case 0: //transfer-coding
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_transferCoding);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 1: //OWS, OptionalWhiteSpace: *( SP / HTAB )
						if(NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_HTAB(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
						} else {
							obj->curNode->iCurPart = iCurPart + 2; //go to optional ','
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 2: //OWS (extras)
						if(NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_HTAB(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
						} else {
							obj->curNode->iCurPart = iCurPart + 2; //go to required ','
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					//-----------
					// Optional ',' (no OWS found before)
					//-----------
					case 3: //',' (optional, no OWS found)
						if(c == ','){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 2; //Continue-optional-seq
						} else {
							//Completed
							NBHttpParser_depthStackPop(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					//-----------
					// Required ',' (OWS found before)
					//-----------
					case 4: //','
						if(c == ','){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1; //Continue-optional-seq
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					//-----------
					// Rest of optional seq
					//-----------
					case 5: //OWS, OptionalWhiteSpace: *( SP / HTAB )
						if(NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_HTAB(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
						} else {
							obj->curNode->iCurPart = iCurPart + 1;
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 6: //transfer-coding
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_transferCoding);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					default: //Go back to 1
						obj->curNode->iCurPart = 1;
						r = NBHttpParser_feedByte(obj, c);
						break;
				}
				break;
			case ENNBHttpParserType_transferCoding:	//"chunked" / "compress" / "deflate" / "gzip" / transfer-extension
				switch(iCurPosib) {
					case 0: //"chunked"
					case 1: //"compress"
					case 2: //"deflate"
					case 3: //"gzip"
						{
							const char* base = (iCurPosib == 0 ? "chunked" : iCurPosib == 1 ? "compress" : iCurPosib == 2 ? "deflate" : iCurPosib == 3 ? "gzip" : ""); NBASSERT(base[0] != '\0')
							const UI32 cAdded = (obj->strAcum.length - obj->curNode->rng.iStart);
							if(c == base[cAdded]){
								NBString_concatByte(&obj->strAcum, c); r = TRUE;
								if((cAdded + 1) == NBString_strLenBytes(base)){
									//Completed
									NBHttpParser_depthStackPop(obj);
								}
							} else {
								//Try next posib
								NBHttpParser_depthStackReset(obj);
								r = NBHttpParser_feedByte(obj, c);
							}
						}
						break;
					case 4: //transfer-extension
						if(iCurPart == 0){
							if(!obj->curNode->curPartErr){
								NBHttpParser_depthStackPush(obj, ENNBHttpParserType_transferExtension);
								r = NBHttpParser_feedByte(obj, c);
							} else {
								//Error
								NBHttpParser_depthStackRemove(obj);
								r = NBHttpParser_feedByte(obj, c);
							}
						} else {
							//Completed
							NBHttpParser_depthStackPop(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					default:
						//All posib tried
						NBHttpParser_depthStackRemove(obj);
						r = NBHttpParser_feedByte(obj, c);
						break;
				}
				break;
			case ENNBHttpParserType_transferExtension:	//token *( OWS ";" OWS transfer-parameter )
				switch(iCurPart) {
					case 0: //token
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_token);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 1: //OWS, OptionalWhiteSpace: *( SP / HTAB )
						if(NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_HTAB(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
						} else {
							obj->curNode->iCurPart = iCurPart + 2; //go to optional ';'
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 2: //OWS (extras)
						if(NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_HTAB(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
						} else {
							obj->curNode->iCurPart = iCurPart + 2; //go to required ';'
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					//-----------
					// Optional ';' (no OWS found before)
					//-----------
					case 3: //';' (optional, no OWS found)
						if(c == ';'){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 2; //Continue-optional-seq
						} else {
							//Completed
							NBHttpParser_depthStackPop(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					//-----------
					// Required ';' (OWS found before)
					//-----------
					case 4: //';'
						if(c == ';'){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1; //Continue-optional-seq
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					//-----------
					// Rest of optional seq
					//-----------
					case 5: //OWS, OptionalWhiteSpace: *( SP / HTAB )
						if(NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_HTAB(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
						} else {
							obj->curNode->iCurPart = iCurPart + 1;
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 6: //transfer-parameter
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_transferParameter);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					default: //Go back to 1
						obj->curNode->iCurPart = 1;
						r = NBHttpParser_feedByte(obj, c);
						break;
				}
				break;
			case ENNBHttpParserType_transferParameter:	//token / token BWS "=" BWS ( token / quoted-string )
				switch(iCurPart) {
					case 0: //token
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_token);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 1: //First BWS, BadWhiteSpace: OWS = OptionalWhiteSpace: *( SP / HTAB )
						if(NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_HTAB(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
						} else {
							obj->curNode->iCurPart = iCurPart + 2; //go to optional '='
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 2: //BWS (extras)
						if(NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_HTAB(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
						} else {
							obj->curNode->iCurPart = iCurPart + 2; //go to required '='
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					//-----------
					// Optional '=' (no OWS found before)
					//-----------
					case 3: //'=' (optional, no BWS found)
						if(c == '='){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 2; //Continue-optional-seq
						} else {
							//Completed
							NBHttpParser_depthStackPop(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					//-----------
					// Required '=' (BWS found before)
					//-----------
					case 4: //'='
						if(c == '='){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1; //Continue-optional-seq
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					//-----------
					// Rest of optional seq
					//-----------
					case 5: //BWS, BadWhiteSpace: OWS = OptionalWhiteSpace: *( SP / HTAB )
						if(NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_HTAB(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
						} else {
							obj->curNode->iCurPart = iCurPart + 1;
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					//---
					//Last optional part
					//---
					case 6: //token
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_token);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							obj->curNode->curPartErr = FALSE;
							obj->curNode->iCurPart = iCurPart + 2; //try quoted-string
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 7:
						//Completed
						NBHttpParser_depthStackPop(obj);
						r = NBHttpParser_feedByte(obj, c);
						break;
					case 8: //quoted-string
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_quotedString);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					default: //Completed
						NBHttpParser_depthStackPop(obj);
						r = NBHttpParser_feedByte(obj, c);
						break;
				}
				break;
			case ENNBHttpParserType_chunkedBody:	//*chunk last-chunk trailer-part CRLF
				switch(iCurPart) {
					case 0: //chunk
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_chunk);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 1: //last-chunk?
						//Evaluate last chunk
						if(obj->lastChunkFound){
							//Move to next part
							obj->curNode->iCurPart = iCurPart + 1;
						} else {
							//Go back to 0
							obj->curNode->iCurPart = 0;
						}
						r = NBHttpParser_feedByte(obj, c);
						break;
					case 2: //trailer-part
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_trailerPart);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 3: //CRLF (CR)
						if(NBHTTP_v11_IS_CR(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					default: //CRLF (LF)
						if(NBHTTP_v11_IS_LF(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							NBHttpParser_depthStackPop(obj);
							//End-of-chunked-body
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
				}
				break;
			case ENNBHttpParserType_chunk:	//chunk-size [ chunk-ext ] CRLF chunk-data CRLF
											//last-chunk:	//1*("0") [ chunk-ext ] CRLF
				switch(iCurPart) {
					case 0:
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_chunkSize);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 1:
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_chunkExt);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Was optional
							obj->curNode->iCurPart = iCurPart + 1;
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 2: //CRLF (CR)
						if(NBHTTP_v11_IS_CR(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 3: //CRLF (LF)
						if(NBHTTP_v11_IS_LF(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							if(obj->lastChunkFound){
								//Completed
								NBHttpParser_depthStackPop(obj);
							} else {
								//Move to next part
								obj->curNode->iCurPart = iCurPart + 1;
							}
							
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 4: //chunk-data
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_chunkData);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 5: //CRLF (CR)
						if(NBHTTP_v11_IS_CR(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					default: //CRLF (LF)
						if(NBHTTP_v11_IS_LF(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							//Completed
							NBHttpParser_depthStackPop(obj);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
				}
				break;
			case ENNBHttpParserType_chunkSize:		//1*HEXDIG
				if(NBHTTP_v11_IS_HEXDIG(c)){
					NBString_concatByte(&obj->strAcum, c); r = TRUE;
					if(iCurPart == 0) obj->curNode->iCurPart = iCurPart + 1;
				} else {
					if(iCurPart == 0){
						//Error, at least one HEXDIG expected
						NBHttpParser_depthStackRemove(obj);
						r = NBHttpParser_feedByte(obj, c);
					} else {
						//Completed
						NBHttpParser_depthStackPop(obj);
						r = NBHttpParser_feedByte(obj, c);
					}
				}
				break;
			case ENNBHttpParserType_chunkData:		//1*OCTET
				if(obj->curChunkPos <= obj->curChunkSz){
					const BOOL added = (obj->curChunkPos < obj->curChunkSz ? TRUE : FALSE);
					if(added){
						if(obj->dataBuffSz > 0){
							if(obj->dataBuff == NULL){
								obj->dataBuff = NBMemory_alloc(sizeof(char) * obj->dataBuffSz);
								NBASSERT(obj->dataBuffPos == 0)
							}
							NBASSERT(obj->dataBuffPos < obj->dataBuffSz)
							obj->dataBuff[obj->dataBuffPos++] = c;
							//Flush buffer (and notify)
							if(obj->dataBuffPos >= obj->dataBuffSz){
								NBASSERT(obj->dataBuffPos == obj->dataBuffSz)
								if(obj->listener.consumeChunkData != NULL){
									if(!(*obj->listener.consumeChunkData)(obj, obj->curNode, obj->dataBuff, obj->dataBuffPos, (obj->curChunkPos + 1), obj->curChunkSz, obj->listenerParam)){
										obj->usrLogicError = TRUE;
									}
									obj->curChunkNotif	+= obj->dataBuffPos; NBASSERT(obj->curChunkNotif == (obj->curChunkPos + 1))
								}
								obj->dataBuffPos = 0;
							}
						}
						r = TRUE;
						obj->curChunkPos++;
					}
					//PRINTF_INFO("chunkData: %llu / %llu.\n", obj->curChunkPos, obj->curChunkSz);
					if(obj->curChunkPos == obj->curChunkSz){
						//Completed
						NBHttpParser_depthStackPop(obj);
						NBASSERT(obj->curChunkNotif == 0)
						NBASSERT(obj->curChunkPos == 0)
						NBASSERT(obj->curChunkSz == 0)
					}
					if(!added){
						r = NBHttpParser_feedByte(obj, c);
					}
				} else {
					//Error
					NBHttpParser_depthStackRemove(obj);
					r = NBHttpParser_feedByte(obj, c);
				}
				break;
			case ENNBHttpParserType_chunkExt:		//*( BWS  ";" BWS chunk-ext-name [ BWS  "=" BWS chunk-ext-val ] )
				switch(iCurPart) {
					case 0: //BWS, BadWhiteSpace: OWS = OptionalWhiteSpace: *( SP / HTAB )
						if(NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_HTAB(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
						} else {
							obj->curNode->iCurPart = iCurPart + 1;
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 1: //";"
						if(c == ';'){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
						} else {
							//Completed
							NBHttpParser_depthStackPop(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 2: //BWS, BadWhiteSpace: OWS = OptionalWhiteSpace: *( SP / HTAB )
						if(NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_HTAB(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
						} else {
							obj->curNode->iCurPart = iCurPart + 1;
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 3: //chunk-ext-name
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_chunkExtName);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 4: //BWS or "=", (first)
						if(NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_HTAB(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
						} else if(c == '='){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 2;
						} else {
							//Go back to 0
							obj->curNode->iCurPart = 0;
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 5: //BWS or "=", (extras)
						if(NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_HTAB(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
						} else if(c == '='){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
							obj->curNode->iCurPart = iCurPart + 1;
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 6: //BWS, BadWhiteSpace: OWS = OptionalWhiteSpace: *( SP / HTAB )
						if(NBHTTP_v11_IS_SP(c) || NBHTTP_v11_IS_HTAB(c)){
							NBString_concatByte(&obj->strAcum, c); r = TRUE;
						} else {
							obj->curNode->iCurPart = iCurPart + 1;
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					case 7: //chunk-ext-val
						if(!obj->curNode->curPartErr){
							NBHttpParser_depthStackPush(obj, ENNBHttpParserType_chunkExtVal);
							r = NBHttpParser_feedByte(obj, c);
						} else {
							//Error
							NBHttpParser_depthStackRemove(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					default:
						//Go back to 0
						obj->curNode->iCurPart = 0;
						r = NBHttpParser_feedByte(obj, c);
						break;
				}
				break;
			case ENNBHttpParserType_chunkExtName:		//token
				if(iCurPart == 0){
					if(!obj->curNode->curPartErr){
						NBHttpParser_depthStackPush(obj, ENNBHttpParserType_token);
						r = NBHttpParser_feedByte(obj, c);
					} else {
						//Error
						NBHttpParser_depthStackRemove(obj);
						r = NBHttpParser_feedByte(obj, c);
					}
				} else {
					//Completed
					NBHttpParser_depthStackPop(obj);
					r = NBHttpParser_feedByte(obj, c);
				}
				break;
			case ENNBHttpParserType_chunkExtVal:	//token / quoted-string
				switch(iCurPosib) {
					case 0: //token
						if(iCurPart == 0){
							if(!obj->curNode->curPartErr){
								NBHttpParser_depthStackPush(obj, ENNBHttpParserType_token);
								r = NBHttpParser_feedByte(obj, c);
							} else {
								//Next posib
								NBHttpParser_depthStackReset(obj);
								r = NBHttpParser_feedByte(obj, c);
							}
						} else {
							//Completed
							NBHttpParser_depthStackPop(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
					default: //quoted-string
						if(iCurPart == 0){
							if(!obj->curNode->curPartErr){
								NBHttpParser_depthStackPush(obj, ENNBHttpParserType_quotedString);
								r = NBHttpParser_feedByte(obj, c);
							} else {
								//All posibs tried
								NBHttpParser_depthStackRemove(obj);
								r = NBHttpParser_feedByte(obj, c);
							}
						} else {
							//Completed
							NBHttpParser_depthStackPop(obj);
							r = NBHttpParser_feedByte(obj, c);
						}
						break;
				}
				break;
			case ENNBHttpParserType_trailerPart: //*( header-field CRLF )
				if(iCurPart == 0){
					if(!obj->curNode->curPartErr){
						NBHttpParser_depthStackPush(obj, ENNBHttpParserType_headerField);
						r = NBHttpParser_feedByte(obj, c);
					} else {
						//Completed
						NBHttpParser_depthStackPop(obj);
						r = NBHttpParser_feedByte(obj, c);
					}
				} else {
					//go back to 0
					obj->curNode->iCurPart = 0;
					r = NBHttpParser_feedByte(obj, c);
				}
				break;
			//-------------
			//- Unepected
			//-------------
			default:
				PRINTF_ERROR("HttpParser, program logic error, unexpected obj-type.\n");
				obj->fmtLogicError = TRUE; NBASSERT(FALSE) //Program  logic error
				break;
		}
		//Evaluate forced end of stream
		if(c == '\0'){
			if(r == FALSE){ //Only evaluate the first feed of '\0' (ignore others posible parent/recursive calls)
				//No active node, no format error and something in the stack
				r = (obj->curNode == NULL && !obj->usrLogicError && !obj->fmtLogicError && obj->nodesStack.use > 0);
			}
		}
	}
	return r;
}

UI32 NBHttpParser_feed(STNBHttpParser* obj, const char* pData){
	const char* data = pData;
	while(*data != '\0' && !obj->usrLogicError && !obj->fmtLogicError){
		const char c = *data;
		if(!NBHttpParser_feedByte(obj, c)){
			break;
		}
		data++;
	}
	return (UI32)(data - pData); //Return consumed count
}

UI32 NBHttpParser_feedBytes(STNBHttpParser* obj, const void* pData, const UI32 dataSz){
	const char* data = (const char*)pData;
	const char* afterEnd = data + dataSz;
	while(data < afterEnd && !obj->usrLogicError && !obj->fmtLogicError){
		const char c = *data;
		if(!NBHttpParser_feedByte(obj, c)){
			break;
		}
		data++;
	}
	return (UI32)(data - (const char*)pData); //Return consumed count
}

