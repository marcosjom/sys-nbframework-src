#ifndef NBStompFrame_H
#define NBStompFrame_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBArray.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
STOMP Protocol Specification, Version 1.2
https://stomp.github.io/stomp-specification-1.2.html#STOMP_Frames
//
NULL                = <US-ASCII null (octet 0)>
LF                  = <US-ASCII line feed (aka newline) (octet 10)>
CR                  = <US-ASCII carriage return (octet 13)>
EOL                 = [CR] LF 
OCTET               = <any 8-bit sequence of data>

frame-stream        = 1*frame

frame               = command EOL
					  *( header EOL )
					  EOL
					  *OCTET
					  NULL
					  *( EOL )

command             = client-command | server-command

client-command      = "SEND"
					  | "SUBSCRIBE"
					  | "UNSUBSCRIBE"
					  | "BEGIN"
					  | "COMMIT"
					  | "ABORT"
					  | "ACK"
					  | "NACK"
					  | "DISCONNECT"
					  | "CONNECT"
					  | "STOMP"

server-command      = "CONNECTED"
					  | "MESSAGE"
					  | "RECEIPT"
					  | "ERROR"

header              = header-name ":" header-value
header-name         = 1*<any OCTET except CR or LF or ":">
header-value        = *<any OCTET except CR or LF or ":">
*/

//ToDo: implement LIMITS,
/*
To prevent malicious clients from exploiting memory allocation in a server, servers MAY place maximum limits on:

	the number of frame headers allowed in a single frame
	the maximum length of header lines
	the maximum size of a frame body

If these limits are exceeded the server SHOULD send the client an ERROR frame and then close the connection.
*/

//ENNBStompCommand

typedef enum ENNBStompCommand_ {
	//client
	ENNBStompCommand_SEND
	, ENNBStompCommand_SUBSCRIBE
	, ENNBStompCommand_UNSUBSCRIBE
	, ENNBStompCommand_BEGIN
	, ENNBStompCommand_COMMIT
	, ENNBStompCommand_ABORT
	, ENNBStompCommand_ACK
	, ENNBStompCommand_NACK
	, ENNBStompCommand_DISCONNECT
	, ENNBStompCommand_CONNECT
	, ENNBStompCommand_STOMP
	//server
	, ENNBStompCommand_CONNECTED
	, ENNBStompCommand_MESSAGE
	, ENNBStompCommand_RECEIPT
	, ENNBStompCommand_ERROR
	//count
	,ENNBStompCommand_Count
} ENNBStompCommand;

//NBStompCommandDef

typedef struct STNBStompCommandDef_ {
	ENNBStompCommand	uid;		//unique id
	const char*			str;		//string-value
	BOOL				fromClt;	//allowed from client
	BOOL				froSrvr;	//allowed from server
} STNBStompCommandDef;

const STNBStompCommandDef* NBStompCommandDef_getByUid(const ENNBStompCommand uid);
const STNBStompCommandDef* NBStompCommandDef_getByStr(const char* strCommand);
const STNBStompCommandDef* NBStompCommandDef_getByStrBytes(const char* strCommand, const UI32 strCommandSz);

//ENNBStompFrameAllocMode

typedef enum ENNBStompFrameAllocMode_ {
	ENNBStompFrameAllocMode_Internal = 0,	//allocate internal strings for values
	ENNBStompFrameAllocMode_External,		//populated with pointers to provided buffers (requires the frame be fed in one call)
	//Count
	ENNBStompFrameAllocMode_Count
} ENNBStompFrameAllocMode;

//ENNBStompFrameAllocMode

typedef enum ENNBStompFrameParseMode_ {
	ENNBStompFrameParseMode_Strict = 0,	//values must comply with stomp-definitions
	ENNBStompFrameParseMode_Flexible,	//flexible implementation is allowed (':' char is allowed inside a header-value)
	//Count
	ENNBStompFrameParseMode_Count
} ENNBStompFrameParseMode;

//NBStompStr

typedef struct STNBStompStr_ {
	UI32		iStart;
	UI32		len;
} STNBStompStr;

void NBStompStr_init(STNBStompStr* obj); 
void NBStompStr_release(STNBStompStr* obj);
void NBStompStr_reset(STNBStompStr* obj);
void NBStompStr_swap(STNBStompStr* obj, STNBStompStr* other);

//NBStompStrPtr

typedef struct STNBStompStrPtr_ {
	const char* ptr;
	UI32		len;
} STNBStompStrPtr;

//NBStompHeader

typedef struct STNBStompHeader_ {
	STNBStompStr	name;
	STNBStompStr	value;
} STNBStompHeader;

void NBStompHeader_init(STNBStompHeader* obj); 
void NBStompHeader_release(STNBStompHeader* obj);

typedef struct STNBStompHeaderInline_ {
	const char* name;
	const char* value;
} STNBStompHeaderInline;

//NBStompFrame

typedef struct STNBStompFrame_ {
	//cmd
	struct {
		ENNBStompCommand knownId;	//ENNBStompCommand_Count, if 'command.str' is an unexpected value
		STNBStompStr	str;		//str value
	} cmd;
	//hdrs
	STNBArray			hdrs;		//STNBStompHeader
	STNBStompStr		body;		//body
	STNBString			buff;		//allocation buffer
	//parser
	void*				parser;		//opaque active parser
} STNBStompFrame;

void NBStompFrame_init(STNBStompFrame* obj);
void NBStompFrame_release(STNBStompFrame* obj);

void NBStompFrame_reset(STNBStompFrame* obj);

//parse
void NBStompFrame_feedStart(STNBStompFrame* obj, const ENNBStompFrameAllocMode allocMode, const ENNBStompFrameParseMode parseMode);
UI32 NBStompFrame_feed(STNBStompFrame* obj, const void* data, const UI32 dataSz);
BOOL NBStompFrame_feedIsCompleted(STNBStompFrame* obj);
BOOL NBStompFrame_feedError(STNBStompFrame* obj);
UI32 NBStompFrame_getBytesFedCount(const STNBStompFrame* obj);

//concat
BOOL NBStompFrame_concat(STNBStompFrame* obj, const BOOL includeCarriageReturnChar, STNBString* dst);
BOOL NBStompFrame_concatWithExternalBuffer(STNBStompFrame* obj, const char* buff, const UI32 buffSz, const BOOL includeCarriageReturnChar, STNBString* dst);
BOOL NBStompFrame_concatWithHeaders(const char* command, const STNBStompHeaderInline* hdrs, const UI32 hdrsSz, const char* body, const UI32 bodyLen, const BOOL includeCarriageReturnChar, STNBString* dst);
STNBStompStrPtr NBStompFrame_getCommandUnscaped(STNBStompFrame* obj, STNBString* tmpBuff);
STNBStompStrPtr NBStompFrame_getCommandUnscapedWithExternalBuffer(STNBStompFrame* obj, const char* buff, const UI32 buffSz, STNBString* tmpBuff);
STNBStompStrPtr NBStompFrame_getHdrValueUnscaped(STNBStompFrame* obj, const char* paramName, STNBString* tmpBuff);
STNBStompStrPtr NBStompFrame_getHdrValueUnscapedWithExternalBuffer(STNBStompFrame* obj, const char* paramName, const char* buff, const UI32 buffSz, STNBString* tmpBuff);
STNBStompStrPtr NBStompFrame_getBody(STNBStompFrame* obj);
STNBStompStrPtr NBStompFrame_getBodyWithExternalBuffer(STNBStompFrame* obj, const char* buff, const UI32 buffSz);
STNBStompStrPtr NBStompFrame_getStrUnscaped(STNBStompFrame* obj, const STNBStompStr* str, STNBString* buff);
STNBStompStrPtr NBStompFrame_getStrUnscapedWithExternalBuffer(const STNBStompStr* str, const char* buff, const UI32 buffSz, STNBString* tmpBuff);
BOOL NBStompFrame_getHeaderAtIndex(const STNBStompFrame* obj, const SI32 idx, STNBStompStrPtr* dstName, STNBStompStrPtr* dstValue);  
BOOL NBStompFrame_getHeaderAtIndexWithExternalBuffer(const STNBStompFrame* obj, const SI32 idx,  const char* buff, const UI32 buffSz, STNBStompStrPtr* dstName, STNBStompStrPtr* dstValue);

#ifdef NB_CONFIG_INCLUDE_ASSERTS
BOOL NBStompFrame_dbgTest(void);
#endif

#ifdef __cplusplus
} //extern "C"
#endif


#endif
