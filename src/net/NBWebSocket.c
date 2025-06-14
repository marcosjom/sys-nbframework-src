
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBWebSocket.h"
//
#include "nb/core/NBStruct.h"
#include "nb/core/NBNumParser.h"
#include "nb/net/NBHttpParser.h"
#include "nb/crypto/NBBase64.h"
#include "nb/crypto/NBSha1.h"

//-----------------
//-- WebSocket - RFC6455
//-- https://datatracker.ietf.org/doc/html/rfc6455
//-----------------

#define NB_WEBSOCKET_GUI		"258EAFA5-E914-47DA-95CA-C5AB0DC85B11"	//Globally Unique Identifier, defined at RFC
#define NB_WEBSOCKET_VERSION	13

//opaque

typedef struct STNBWebSocketOpq_ {
	STNBObject			prnt;
} STNBWebSocketOpq;

NB_OBJREF_BODY(NBWebSocket, STNBWebSocketOpq, NBObject)


void NBWebSocket_initZeroed(STNBObject* obj) {
	//STNBWebSocketOpq* opq	= (STNBWebSocketOpq*)obj;
	//
}

void NBWebSocket_uninitLocked(STNBObject* obj){
	//STNBWebSocketOpq* opq = (STNBWebSocketOpq*)obj;
}

BOOL NBWebSocket_concatResponseHeader(STNBWebSocketRef ref, const STNBHttpHeader* req, STNBHttpHeader* dst){
	BOOL r = FALSE;
	//STNBWebSocketOpq* opq	= (STNBWebSocketOpq*)ref.opaque;
	const STNBHttpHeaderRequestLine* reqLn = req->requestLine;
	const char* host		= NBHttpHeader_getField(req, "host");
	const BOOL isWebSocket	= NBHttpHeader_hasFieldValue(req, "Upgrade", "websocket", FALSE);
	const BOOL isUpgrade	= NBHttpHeader_hasFieldValue(req, "Connection", "Upgrade", FALSE);
	const char*	key			= NBHttpHeader_getField(req, "sec-webSocket-key"); //16-bytes-random-key in base64 format
	const char*	version		= NBHttpHeader_getField(req, "sec-webSocket-version"); //13
	//ToDo: validate 'Sec-WebSocket-Protocol' field.
	//ToDo: validate 'Sec-WebSocket-Extensions' field.
	BOOL versionIsNum		= FALSE;
	const UI32 versionI		= NBNumParser_toUI32(version, &versionIsNum);
	if(reqLn == NULL){
		PRINTF_ERROR("NBWebSocket, request is NULL.\n");
		r = FALSE;
	} else if(!NBString_strIsLike(NBHttpHeader_getRequestMethod(req), "GET")){
		PRINTF_ERROR("NBWebSocket, request method must be 'GET'.\n");
		r = FALSE;
	} else if(reqLn->majorVer <= 1 && reqLn->minorVer <= 0){
		PRINTF_ERROR("NBWebSocket, request must be '1.1' or higher.\n");
		r = FALSE;
	} else if(host == NULL){
		PRINTF_ERROR("NBWebSocket, request 'Host' field is required.\n");
		r = FALSE;
	} else if(!isWebSocket){
		PRINTF_ERROR("NBWebSocket, request must include 'Upgrade: websocket'.\n");
		r = FALSE;
	} else if(!isUpgrade){
		PRINTF_ERROR("NBWebSocket, request must include 'Connection: upgrade'.\n");
		r = FALSE;
	} else if(NBString_strIsEmpty(key)){
		PRINTF_ERROR("NBWebSocket, request must include 'Sec-webSocket-key: ...'.\n");
		r = FALSE;
	} else if(version == NULL || !versionIsNum){
		PRINTF_ERROR("NBWebSocket, request must include 'sec-webSocket-version: ...'.\n");
		r = FALSE;
	} else if(versionI != NB_WEBSOCKET_VERSION){
		PRINTF_ERROR("NBWebSocket, request must include 'sec-webSocket-version: %d' (%d).\n", NB_WEBSOCKET_VERSION, versionI);
		r = FALSE;
	} else {
		if(dst != NULL){
			NBHttpHeader_setStatusLine(dst, 1, 1, 101, "Switching Protocols");
			NBHttpHeader_addField(dst, "Upgrade", "websocket");
			NBHttpHeader_addField(dst, "Connection", "Upgrade");
			{
				STNBSha1Hash hash;
				STNBString str, str2;
				NBString_init(&str);
				NBString_init(&str2);
				NBString_concat(&str, key);
				NBString_concat(&str, NB_WEBSOCKET_GUI);
				hash = NBSha1_getHashBytes(str.str, str.length);
				NBBase64_codeBytes(&str2, (const char*)hash.v, sizeof(hash.v));
				NBHttpHeader_addField(dst, "Sec-WebSocket-Accept", str2.str);
				NBString_release(&str);
				NBString_release(&str2);
			}
		}
		r = TRUE;  
	}
	return r;
}
