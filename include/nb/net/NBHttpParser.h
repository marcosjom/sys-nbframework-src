#ifndef NB_HTTP_PARSER_H
#define NB_HTTP_PARSER_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBString.h"
#include "nb/net/NBUriParser.h"

//

#ifdef __cplusplus
extern "C" {
#endif
	
	//-----------------
	//-- HTTP 1.1 - RFC7230
	//-- https://tools.ietf.org/html/rfc7230
	//-----------------
	//-- ERRATA
	//--- https://www.rfc-editor.org/errata_search.php?rfc=7230
	//http-URI				= "http:" "//" authority path-abempty [ "?" query ]
	//https-URI				= "https:" "//" authority path-abempty [ "?" query ]
	//chunk-ext     		= *( BWS  ";" BWS chunk-ext-name [ BWS  "=" BWS chunk-ext-val ] )
	//field-content			= field-vchar [ 1*( SP / HTAB / field-vchar ) field-vchar ]
	//obs-fold				= OWS CRLF 1*( SP / HTAB )
	//transfer-parameter	= token / token BWS "=" BWS ( token / quoted-string )
	//-----------------
	//White spaces
	//CRLF, CR LF
	//OWS, OptionalWhiteSpace: *( SP / HTAB )
	//RWS, RequiredWhiteSpace: 1*( SP / HTAB )
	//BWS, BadWhiteSpace: OWS
	
	typedef enum ENNBHttpParserType_ {
		//Message Format
		ENNBHttpParserType_httpMessageHeader = 0 //start-line *( header-field CRLF ) CRLF [ message-body ]
		, ENNBHttpParserType_startLine		//request-line / status-line
		, ENNBHttpParserType_requestLine	//method SP request-target SP HTTP-version CRLF
		, ENNBHttpParserType_requestTarget	//origin-form / absolute-form / authority-form / asterisk-form
		, ENNBHttpParserType_method			//token
		, ENNBHttpParserType_statusLine		//HTTP-version SP status-code SP reason-phrase CRLF
		, ENNBHttpParserType_statusCode		//3DIGIT
		, ENNBHttpParserType_reasonPhrase	//*( HTAB / SP / VCHAR / obs-text )
		//Request-target
		, ENNBHttpParserType_originForm		//absolute-path [ "?" query ]
		, ENNBHttpParserType_absoluteForm	//absolute-URI
		, ENNBHttpParserType_authorityForm	//authority
		, ENNBHttpParserType_asteriskForm	//"*"
		, ENNBHttpParserType_absolutePath	//1*( "/" segment )
		//Protocol Versioning
		, ENNBHttpParserType_httpVersion	//"HTTP" "/" DIGIT "." DIGIT
		//Header fields
		, ENNBHttpParserType_headerField	//field-name ":" OWS field-value OWS
		, ENNBHttpParserType_fieldName		//token
		, ENNBHttpParserType_fieldValue		//*( field-content / obs-fold )
		, ENNBHttpParserType_fieldContent	//field-vchar [ 1*( SP / HTAB / field-vchar ) field-vchar ]
		, ENNBHttpParserType_fieldVChar		//VCHAR / obs-text
		, ENNBHttpParserType_obsFold		//OWS CRLF 1*( SP / HTAB )
		//Fields values
		, ENNBHttpParserType_token			//1*tchar
		, ENNBHttpParserType_quotedString	//DQUOTE *( qdtext / quoted-pair ) DQUOTE
		, ENNBHttpParserType_qdText			//HTAB / SP / %x21 / %x23-5B / %x5D-7E / obs-text
		, ENNBHttpParserType_obsText		//%x80-FF
		, ENNBHttpParserType_comment		//"(" *( ctext / quoted-pair / comment ) ")"
		, ENNBHttpParserType_ctext			//HTAB / SP / %x21-27 / %x2A-5B / %x5D-7E / obs-text
		, ENNBHttpParserType_quotedPair		//"\" ( HTAB / SP / VCHAR / obs-text )
		//Content length
		, ENNBHttpParserType_contentLength	//1*DIGIT
		//Transfer encoding
		, ENNBHttpParserType_transferEncoding	//1#transfer-coding == transfer-coding *( OWS "," OWS transfer-coding )
		, ENNBHttpParserType_transferCoding		//"chunked" / "compress" / "deflate" / "gzip" / transfer-extension
		, ENNBHttpParserType_transferExtension	//token *( OWS ";" OWS transfer-parameter )
		, ENNBHttpParserType_transferParameter	//token / token BWS "=" BWS ( token / quoted-string )
		//Message body
		, ENNBHttpParserType_chunkedBody	//*chunk last-chunk trailer-part CRLF
		, ENNBHttpParserType_chunk			//chunk-size [ chunk-ext ] CRLF chunk-data CRLF //last-chunk = 1*("0") [ chunk-ext ] CRLF
		, ENNBHttpParserType_chunkSize		//1*HEXDIG
		, ENNBHttpParserType_chunkData		//1*OCTET
		, ENNBHttpParserType_chunkExt		//*( BWS  ";" BWS chunk-ext-name [ BWS  "=" BWS chunk-ext-val ] )
		, ENNBHttpParserType_chunkExtName	//token
		, ENNBHttpParserType_chunkExtVal	//token / quoted-string
		, ENNBHttpParserType_trailerPart	//*( header-field CRLF )
		//
		, ENNBHttpParserType_count
	} ENNBHttpParserType;
	
	
	
	typedef struct STNBHttpParserRng_ {
		UI32		iStart;
		UI32		count;
	} STNBHttpParserRng;
	
	typedef struct STNBHttpParserNode_ {
		ENNBHttpParserType	type;		//Depth type
		BOOL				popped;		//Still alive?
		UI8					iCurPosib;	//Currently testing posibility
		UI8					iCurPart;	//Currently testing part in posibility
		BOOL				curPartErr;	//Error ocurred in current evaluated part
		UI32				iDeepLvl;	//deep level (reference)
		STNBHttpParserRng	rng;		//Range of content in not-ignored seqs (the ones added to the strAccum).
	} STNBHttpParserNode;
	
	struct STNBHttpParser_;
	
	typedef struct IHttpParserListener_ {
		//Header
		BOOL (*consumeHttpVer)(const struct STNBHttpParser_* obj, const STNBHttpParserNode* node, const char* httpVer, const UI32 httpVerSz, const UI32 majorVer, const UI32 minorVer, void* listenerParam);
		//Header request-start-line
		BOOL (*consumeMethod)(const struct STNBHttpParser_* obj, const STNBHttpParserNode* node, const char* method, const UI32 methodSz, void* listenerParam);
		BOOL (*consumeRequestTarget)(const struct STNBHttpParser_* obj, const STNBHttpParserNode* node, const char* target, const UI32 targetSz, void* listenerParam);
		BOOL (*consumeRequestLine)(const struct STNBHttpParser_* obj, const STNBHttpParserNode* node, const char* method, const UI32 methodSz, const char* target, const UI32 targetSz, const char* httpVer, const UI32 httpVerSz, const UI32 majorVer, const UI32 minorVer, void* listenerParam);
		//Header status-start-line
		BOOL (*consumeStatusCode)(const struct STNBHttpParser_* obj, const STNBHttpParserNode* node, const UI32 statusCode, void* listenerParam);
		BOOL (*consumeReasonPhrase)(const struct STNBHttpParser_* obj, const STNBHttpParserNode* node, const char* reasonPhrase, const UI32 reasonPhraseSz, void* listenerParam);
		BOOL (*consumeStatusLine)(const struct STNBHttpParser_* obj, const STNBHttpParserNode* node, const char* httpVer, const UI32 httpVerSz, const UI32 majorVer, const UI32 minorVer, const UI32 statusCode, const char* reasonPhrase, const UI32 reasonPhraseSz, void* listenerParam);
		//Header generic-fields
		BOOL (*consumeFieldName)(const struct STNBHttpParser_* obj, const STNBHttpParserNode* node, const char* name, const UI32 nameSz, void* listenerParam);
		BOOL (*consumeFieldLine)(const struct STNBHttpParser_* obj, const STNBHttpParserNode* node, const char* name, const UI32 nameSz, const char* value, const UI32 valueSz, void* listenerParam);
		//Header end
		BOOL (*consumeHeaderEnd)(const struct STNBHttpParser_* obj, void* listenerParam);
		//Chunked-body
		BOOL (*chunkStarted)(const struct STNBHttpParser_* obj, const STNBHttpParserNode* node, const UI64 chunkSz, void* listenerParam);
		BOOL (*consumeChunkData)(const struct STNBHttpParser_* obj, const STNBHttpParserNode* node, const char* data, const UI64 dataSz, const UI64 chunkReadPos, const UI64 chunkTotalSz, void* listenerParam);
		BOOL (*chunkEnded)(const struct STNBHttpParser_* obj, const STNBHttpParserNode* node, const UI64 chunkTotalSz, void* listenerParam);
		BOOL (*consumeChunkedBodyEnd)(const struct STNBHttpParser_* obj, void* listenerParam);
	} IHttpParserListener;
	
	typedef enum ENNBHttpParserMode_ {
		ENNBHttpParserMode_lazy	= 0,
		ENNBHttpParserMode_soft,
		ENNBHttpParserMode_strict
	} ENNBHttpParserMode;
	
	typedef struct STNBHttpParser_ {
		ENNBHttpParserMode	mode;
		BOOL				usrLogicError;	//Listener returned FALSE
		BOOL				fmtLogicError;	//Sintax error
		STNBArray			nodesStack;		//STNBHttpParserNode
		STNBHttpParserNode*	curNode;		//Current active node
		STNBString			strAcum;		//Accumulated chars
		STNBString			strPend;		//Pending chars
		UI32				bytesFeed;		//total bytes feed to the parser
		STNBUriParser		uriParser;		//Helper for URI verification
		//Chunk-data-buff
		char*				dataBuff;		//Buffer for transfered data (not stored in http-buffer)
		UI32				dataBuffPos;	//Current position in transfered data buffer
		UI32				dataBuffSz;		//Zero if the chunkData must be ignored
		//Current-chunk
		UI64				curChunkNotif;	//Current chunk position notification
		UI64				curChunkPos;
		UI64				curChunkSz;
		BOOL				lastChunkFound;
		//Listener
		IHttpParserListener listener;
		void*				listenerParam;
	} STNBHttpParser;
	
	//
	
	void NBHttpParser_init(STNBHttpParser* obj);
	void NBHttpParser_initWithListener(STNBHttpParser* obj, IHttpParserListener* listener, void* listenerParam);
	void NBHttpParser_release(STNBHttpParser* obj);
	
	//
	
	ENNBHttpParserMode NBHttpParser_mode(STNBHttpParser* obj);
	void NBHttpParser_setMode(STNBHttpParser* obj, const ENNBHttpParserMode mode);
	
	//
	BOOL NBHttpParser_setDataBuffSz(STNBHttpParser* obj, const UI32 sz); //Zero if the chunkData must be ignored
	
	//
	const STNBHttpParserNode* NBHttpParser_nextNode(const STNBHttpParser* obj, const STNBHttpParserNode* parent, const STNBHttpParserNode* node);
	const STNBHttpParserNode* NBHttpParser_nextNodeWithType(const STNBHttpParser* obj, const STNBHttpParserNode* parent, const STNBHttpParserNode* node, const ENNBHttpParserType nextType);
	
	//scape
	BOOL NBHttpParser_concatEncoded(STNBString* dst, const char* unenc, const ENNBHttpParserType type);
	BOOL NBHttpParser_concatUnencodedNode(const STNBHttpParser* obj, const STNBHttpParserNode* node, STNBString* dst);
	BOOL NBHttpParser_isTChar(const char c);	//Token char
	
	//Feed
	void NBHttpParser_feedStart(STNBHttpParser* obj);
	void NBHttpParser_feedStartWithType(STNBHttpParser* obj, const ENNBHttpParserType rootType);
	BOOL NBHttpParser_feedByte(STNBHttpParser* obj, const char c);
	UI32 NBHttpParser_feed(STNBHttpParser* obj, const char* data);
	UI32 NBHttpParser_feedBytes(STNBHttpParser* obj, const void* data, const UI32 dataSz);
	BOOL NBHttpParser_feedIsComplete(STNBHttpParser* obj);
	BOOL NBHttpParser_feedEnd(STNBHttpParser* obj);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
