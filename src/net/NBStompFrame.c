
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBStompFrame.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBString.h"
#include "nb/core/NBNumParser.h"
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

//NBStompFrameParseState

struct STNBStompFrameParseState_;

typedef UI32 (*NBStompFrameParseConsumeMethod)(STNBStompFrame* obj, struct STNBStompFrameParseState_* state, const char* str, const UI32 iPos, const UI32 strSz);

typedef struct STNBStompFrameParseStateCur_ {
	NBStompFrameParseConsumeMethod	method;		//current method (NULL if completed)
	ENNBStompFrameAllocMode			allocMode;	//values allocation mode
	ENNBStompFrameParseMode			parseMode;	//values parsing mode
	BOOL							errFound;	//
	BOOL							charCRFound; //CR char found, expecting LF char afterwards
	//content-length
	struct {
		BOOL						isHdrFound;	//'content-length' header found
		UI32						value;		//value parsed
	} contentLength;
} STNBStompFrameParseStateCur;

typedef struct STNBStompFrameParseState_ {
	STNBStompStr					curStr;		//current str
	STNBStompFrameParseStateCur		cur;		//cur (zeroable)
	UI32							bytesFedCount;
} STNBStompFrameParseState;

void NBStompFrameParseState_init(STNBStompFrameParseState* obj);
void NBStompFrameParseState_release(STNBStompFrameParseState* obj);
void NBStompFrameParseState_reset(STNBStompFrameParseState* obj);
void NBStompFrameParseState_startNextPart(STNBStompFrameParseState* obj, NBStompFrameParseConsumeMethod csmMethod, const UI32 buffCurPos);

//parser methods
UI32 NBStompFrameParse_commandLn_(STNBStompFrame* obj, STNBStompFrameParseState* state, const char* str, const UI32 iPos, const UI32 strSz);
UI32 NBStompFrameParse_headerNameOrEmpty_(STNBStompFrame* obj, STNBStompFrameParseState* state, const char* str, const UI32 iPos, const UI32 strSz);
UI32 NBStompFrameParse_headerValue_(STNBStompFrame* obj, STNBStompFrameParseState* state, const char* str, const UI32 iPos, const UI32 strSz);
UI32 NBStompFrameParse_body_(STNBStompFrame* obj, STNBStompFrameParseState* state, const char* str, const UI32 iPos, const UI32 strSz);
UI32 NBStompFrameParse_trailingEOL_(STNBStompFrame* obj, STNBStompFrameParseState* state, const char* str, const UI32 iPos, const UI32 strSz);

//parse

void NBStompFrame_feedStart(STNBStompFrame* obj, const ENNBStompFrameAllocMode allocMode, const ENNBStompFrameParseMode parseMode){
	STNBStompFrameParseState* state = NULL;
	//reset
	{
		NBStompFrame_reset(obj);
	}
	//parser
	if(obj->parser == NULL){
		state = NBMemory_allocType(STNBStompFrameParseState);
		NBStompFrameParseState_init(state);
		obj->parser = state;
	} else {
		state = (STNBStompFrameParseState*)obj->parser;
		NBStompFrameParseState_reset(state);
	}
	//init parse-state
	{
		state->bytesFedCount	= 0;
		state->cur.allocMode	= allocMode;
		state->cur.parseMode	= parseMode;
		state->cur.errFound		= FALSE;
		NBMemory_setZero(state->cur.contentLength);
		NBStompFrameParseState_startNextPart(state, NBStompFrameParse_commandLn_, 0);
	}
}

UI32 NBStompFrame_feed(STNBStompFrame* obj, const void* pData, const UI32 dataSz){
	UI32 r = 0;
	STNBStompFrameParseState* state = (STNBStompFrameParseState*)obj->parser;
	NBASSERT(state != NULL)
	if(state != NULL){
		const char* data = (const char*)pData;
		while(r < dataSz && !state->cur.errFound && state->cur.method != NULL){
			r += (*state->cur.method)(obj, state, data, r, dataSz);
		}
		state->bytesFedCount += r;
	}
	return r;
}

BOOL NBStompFrame_feedIsCompleted(STNBStompFrame* obj){
	STNBStompFrameParseState* state = (STNBStompFrameParseState*)obj->parser;
	return (state != NULL && (state->cur.method == NULL || (state->cur.method == NBStompFrameParse_trailingEOL_ && !state->cur.charCRFound)));
}

BOOL NBStompFrame_feedError(STNBStompFrame* obj){
	STNBStompFrameParseState* state = (STNBStompFrameParseState*)obj->parser;
	return (state != NULL && state->cur.errFound);
}

UI32 NBStompFrame_getBytesFedCount(const STNBStompFrame* obj){
	STNBStompFrameParseState* state = (STNBStompFrameParseState*)obj->parser;
	return (state == NULL ? 0 : state->bytesFedCount);
}

//concat

BOOL NBStompFrame_concat(STNBStompFrame* obj, const BOOL includeCarriageReturnChar, STNBString* dst){
	return NBStompFrame_concatWithExternalBuffer(obj, obj->buff.str, obj->buff.length, includeCarriageReturnChar, dst);
}

BOOL NBStompFrame_concatWithExternalBuffer(STNBStompFrame* obj, const char* buff, const UI32 buffSz, const BOOL includeCarriageReturnChar, STNBString* dst){
	BOOL r = TRUE;
	//command
	if(obj->cmd.str.len <= 0 || obj->cmd.str.iStart < 0 || obj->cmd.str.iStart >= buffSz || (obj->cmd.str.iStart + obj->cmd.str.len) > buffSz){
		//no-command or str-out-of-bouds
		r = FALSE;
	} else {
		NBString_concatBytes(dst, &buff[obj->cmd.str.iStart], obj->cmd.str.len);
		if(includeCarriageReturnChar){
			NBString_concatByte(dst, '\r');
		}
		NBString_concatByte(dst, '\n');
	}
	//headers
	if(r && obj->hdrs.use > 0){
		SI32 i; for(i = 0; i < obj->hdrs.use; i++){
			STNBStompHeader* hdr = NBArray_itmPtrAtIndex(&obj->hdrs, STNBStompHeader, i);
			if(hdr->name.iStart < 0 || hdr->name.iStart >= buffSz || (hdr->name.iStart + hdr->name.len) > buffSz
			   || hdr->value.iStart < 0 || hdr->value.iStart >= buffSz || (hdr->value.iStart + hdr->value.len) > buffSz){
				//str-out-of-bouds
				r = FALSE;
				break;
			} else if(hdr->name.len > 0){
				NBString_concatBytes(dst, &buff[hdr->name.iStart], hdr->name.len);
				NBString_concatByte(dst, ':');
				if(hdr->value.len > 0){
					NBString_concatBytes(dst, &buff[hdr->value.iStart], hdr->value.len);
				}
				if(includeCarriageReturnChar){
					NBString_concatByte(dst, '\r');
				}
				NBString_concatByte(dst, '\n');
			}
		}
	}
	//empty line
	if(r){
		if(includeCarriageReturnChar){
			NBString_concatByte(dst, '\r');
		}
		NBString_concatByte(dst, '\n');
	}
	//body
	if(r){
		if(obj->body.iStart < 0 || obj->body.iStart >= buffSz || (obj->body.iStart + obj->body.len) > buffSz){
			//no-command or str-out-of-bouds
			r = FALSE;
		} else {
			NBString_concatBytes(dst, &buff[obj->body.iStart], obj->body.len);
		}
	}
	//NULL
	NBString_concatByte(dst, '\0');
	return r;
}

BOOL NBStompFrame_concatWithHeaders(const char* command, const STNBStompHeaderInline* hdrs, const UI32 hdrsSz, const char* body, const UI32 bodyLen, const BOOL includeCarriageReturnChar, STNBString* dst){
	BOOL r = FALSE;
	if(command != NULL && command[0] != '\0' && dst != NULL){
		r = TRUE;
		//command
		{
			const char* c = command;
			while(*c != '\0'){
				switch (*c) {
				case '\r': NBString_concatByte(dst, '\\'); NBString_concatByte(dst, 'r'); break;
				case '\n': NBString_concatByte(dst, '\\'); NBString_concatByte(dst, 'n'); break;
				case ':': NBString_concatByte(dst, '\\'); NBString_concatByte(dst, 'c'); break;
				case '\\': NBString_concatByte(dst, '\\'); NBString_concatByte(dst, '\\'); break;
				default: NBString_concatByte(dst, *c); break;
				}
				//next
				c++;
			}
			if(includeCarriageReturnChar){
				NBString_concatByte(dst, '\r');
			}
			NBString_concatByte(dst, '\n');
		}
		//content-length header
		if(body != NULL && bodyLen > 0){
			NBString_concat(dst, "content-length:");
			NBString_concatUI32(dst, bodyLen);
			if(includeCarriageReturnChar){
				NBString_concatByte(dst, '\r');
			}
			NBString_concatByte(dst, '\n');
		}
		//headers
		if(hdrs != NULL && hdrsSz > 0){
			UI32 i; for(i = 0; i < hdrsSz; i++){
				const STNBStompHeaderInline* hdr = &hdrs[i];
				if(hdr->name != NULL && hdr->name[0] != '\0'){
					{
						const char* c = hdr->name;
						while(*c != '\0'){
							switch (*c) {
							case '\r': NBString_concatByte(dst, '\\'); NBString_concatByte(dst, 'r'); break;
							case '\n': NBString_concatByte(dst, '\\'); NBString_concatByte(dst, 'n'); break;
							case ':': NBString_concatByte(dst, '\\'); NBString_concatByte(dst, 'c'); break;
							case '\\': NBString_concatByte(dst, '\\'); NBString_concatByte(dst, '\\'); break;
							default: NBString_concatByte(dst, *c); break;
							}
							//next
							c++;
						}
					}
					//separator
					NBString_concatByte(dst, ':');
					//value
					if(hdr->value != NULL && hdr->value[0] != '\0'){
						const char* c = hdr->value;
						while(*c != '\0'){
							switch (*c) {
							case '\r': NBString_concatByte(dst, '\\'); NBString_concatByte(dst, 'r'); break;
							case '\n': NBString_concatByte(dst, '\\'); NBString_concatByte(dst, 'n'); break;
							case ':': NBString_concatByte(dst, '\\'); NBString_concatByte(dst, 'c'); break;
							case '\\': NBString_concatByte(dst, '\\'); NBString_concatByte(dst, '\\'); break;
							default: NBString_concatByte(dst, *c); break;
							}
							//next
							c++;
						}
					}
					if(includeCarriageReturnChar){
						NBString_concatByte(dst, '\r');
					}
					NBString_concatByte(dst, '\n');
				}
			}
		}
		//empty line
		{
			if(includeCarriageReturnChar){
				NBString_concatByte(dst, '\r');
			}
			NBString_concatByte(dst, '\n');
		}
		//body
		if(body != NULL && bodyLen > 0){
			NBString_concatBytes(dst, body, bodyLen);
		}
		//NULL
		NBString_concatByte(dst, '\0');
	}
	return r;
}

STNBStompStrPtr NBStompFrame_getCommandUnscaped(STNBStompFrame* obj, STNBString* tmpBuff){
	return NBStompFrame_getStrUnscapedWithExternalBuffer(&obj->cmd.str, obj->buff.str, obj->buff.length, tmpBuff);
}

STNBStompStrPtr NBStompFrame_getCommandUnscapedWithExternalBuffer(STNBStompFrame* obj, const char* buff, const UI32 buffSz, STNBString* tmpBuff){
	return NBStompFrame_getStrUnscapedWithExternalBuffer(&obj->cmd.str, buff, buffSz, tmpBuff);
}

STNBStompStrPtr NBStompFrame_getHdrValueUnscaped(STNBStompFrame* obj, const char* paramName, STNBString* tmpBuff){
	return NBStompFrame_getHdrValueUnscapedWithExternalBuffer(obj, paramName, obj->buff.str, obj->buff.length, tmpBuff);
}

STNBStompStrPtr NBStompFrame_getHdrValueUnscapedWithExternalBuffer(STNBStompFrame* obj, const char* paramName, const char* buff, const UI32 buffSz, STNBString* tmpBuff){
	STNBStompStrPtr r;
	NBMemory_setZeroSt(r, STNBStompStrPtr);
	//search param
	SI32 i; for(i = 0; i < obj->hdrs.use; i++){
		STNBStompHeader* hdr = NBArray_itmPtrAtIndex(&obj->hdrs, STNBStompHeader, i);
		NBASSERT(hdr->name.iStart >= 0 && (hdr->name.iStart + hdr->name.len) <= buffSz)
		if(hdr->name.iStart >= 0 && (hdr->name.iStart + hdr->name.len) <= buffSz){
			//compare unscaped name
			const char* n0 = paramName;
			const char* n1 = &buff[hdr->name.iStart];
			const char* n1AfterEnd = n1 + hdr->name.len;
			while(*n0 != '\0' && n1 < n1AfterEnd){
				if(*n1 == '\\'){
					if((n1 + 1) >= n1AfterEnd){
						//expected another char
						break;
					} else {
						n1++;
						if(*n1 == 'r'){
							if(*n0 != '\r') break;
							n0++; n1++;
						} else if(*n1 == 'n'){
							if(*n0 != '\n') break;
							n0++; n1++;
						} else if(*n1 == 'c'){
							if(*n0 != ':') break;
							n0++; n1++;
						} else if(*n1 == '\\'){
							if(*n0 != '\\') break;
							n0++; n1++;
						} else {
							//unsuported scaping-seq
							break;
						}
					}
				} else if(*n0 == *n1){
					n0++; n1++;
				} else {
					break;
				}
			}
			if(*n0 == '\0' && n1 == n1AfterEnd){
				r = NBStompFrame_getStrUnscapedWithExternalBuffer(&hdr->value, buff, buffSz, tmpBuff);
				break;
			}
		}
	}
	return r;
}

STNBStompStrPtr NBStompFrame_getBody(STNBStompFrame* obj){
	return NBStompFrame_getBodyWithExternalBuffer(obj, obj->buff.str, obj->buff.length);
}

STNBStompStrPtr NBStompFrame_getBodyWithExternalBuffer(STNBStompFrame* obj, const char* buff, const UI32 buffSz){
	STNBStompStrPtr r;
	NBMemory_setZeroSt(r, STNBStompStrPtr);
	if(obj->body.iStart >= 0 && (obj->body.iStart + obj->body.len) <= buffSz){
		r.ptr = &buff[obj->body.iStart];
		r.len = buffSz;
	}
	return r;
}

STNBStompStrPtr NBStompFrame_getStrUnscaped(STNBStompFrame* obj, const STNBStompStr* str, STNBString* tmpBuff){
	return NBStompFrame_getStrUnscapedWithExternalBuffer(str, obj->buff.str, obj->buff.length, tmpBuff);
}

STNBStompStrPtr NBStompFrame_getStrUnscapedWithExternalBuffer(const STNBStompStr* str, const char* buff, const UI32 buffSz, STNBString* tmpBuff){
	STNBStompStrPtr r;
	NBMemory_setZeroSt(r, STNBStompStrPtr);
	if(str != NULL && str->iStart >= 0 && (str->iStart + str->len) <= buffSz && tmpBuff != NULL){
		//process
		UI32 tmpBuffStartLen = tmpBuff->length;
		BOOL errFnd = FALSE, usingTmpBuff = FALSE; 
		const char* c = &buff[str->iStart];
		const char* cAfterEnd = c + str->len;
		const char* cSeqStart = c;
		while(c < cAfterEnd){
			if(*c == '\\'){
				if((c + 1) >= cAfterEnd){
					//expected another char
					errFnd = TRUE;
					break;
				} else {
					c++;
					switch (*c) {
						case 'r':
						case 'n':
						case 'c':
						case '\\':
							NBASSERT((cSeqStart + 1) <= c)
							NBString_concatBytes(tmpBuff, cSeqStart, (UI32)(c - cSeqStart - 1));
							NBString_concatByte(tmpBuff, (*c == 'r' ? '\r' : *c == 'n' ? '\n' : *c == 'c' ? ':' : *c == '\\' ? '\\' : '\0'));
							usingTmpBuff = TRUE;
							cSeqStart = (c + 1);
							break;
						default:
							//unsuported scaping-seq
							errFnd = TRUE;
							break;
					}
				}
			}
			//next
			c++;
		}
		//complete
		if(errFnd){
			NBMemory_setZeroSt(r, STNBStompStrPtr);
		} else {
			NBASSERT(r.ptr == NULL && r.len == 0)
			if(!usingTmpBuff){
				r.ptr	= &buff[str->iStart];
				r.len	= str->len;
			} else {
				NBString_concatBytes(tmpBuff, cSeqStart, (UI32)(c - cSeqStart));
				r.ptr	= &tmpBuff->str[tmpBuffStartLen];
				r.len	= tmpBuff->length - tmpBuffStartLen;
				NBASSERT(r.len < str->len)
			}
		}
	}
	return r;
}

BOOL NBStompFrame_getHeaderAtIndex(const STNBStompFrame* obj, const SI32 idx, STNBStompStrPtr* dstName, STNBStompStrPtr* dstValue){
	return NBStompFrame_getHeaderAtIndexWithExternalBuffer(obj, idx, obj->buff.str, obj->buff.length, dstName, dstValue);
}

BOOL NBStompFrame_getHeaderAtIndexWithExternalBuffer(const STNBStompFrame* obj, const SI32 idx,  const char* buff, const UI32 buffSz, STNBStompStrPtr* dstName, STNBStompStrPtr* dstValue){
	BOOL r = FALSE;
	if(idx >= 0 && idx < obj->hdrs.use){
		const STNBStompHeader* h = NBArray_itmPtrAtIndex(&obj->hdrs, const STNBStompHeader, idx);
		if(h->name.iStart >= 0 && (h->name.iStart + h->name.len) <= buffSz && h->value.iStart >= 0 && (h->value.iStart + h->value.len) <= buffSz){
			if(dstName != NULL){
				dstName->ptr	= &buff[h->name.iStart];
				dstName->len	= h->name.len;
			}
			if(dstValue != NULL){
				dstValue->ptr	= &buff[h->value.iStart];
				dstValue->len	= h->value.len;
			}
			r = TRUE;
		}
	}
	return r;
}

#ifdef NB_CONFIG_INCLUDE_ASSERTS
BOOL NBStompFrame_dbgTest(void){
	BOOL r = TRUE;
	const char* samples[] = {
		"SEND\r\ndestination:/queue/a\r\nreceipt:message-12345\r\n\r\nhello queue a"
		, "ERROR\nreceipt-id:message-12345\ncontent-type:text/plain\ncontent-length:170\nmessage:malformed frame received\n\nThe message:\n-----\nMESSAGE\ndestined:/queue/a\nreceipt:message-12345\n\nHello queue a!\n-----\nDid not contain a destination header, which is REQUIRED\nfor message propagation.\n"
		, "RECEIPT\r\nreceipt-id:message-12345\r\n\n"
		, "MESSAGE\r\nsubscription:0\r\nmessage-id:007\r\ndestination:/queue/a\r\ncontent-type:text/plain\r\n\r\nhello queue a"
		, "RECEIPT\nreceipt-id:77\n\n"
		, "CONNECTED\nheart-beat:10000,10000\nsession:ID:ftp-af.artp-dakar.org-38997-1637628311267-18:8\nserver:ActiveMQ/5.10.0\nversion:1.2\nsubscription:1\n\n"
	};
	STNBString str, tmpBuff;
	STNBStompFrame frm;
	NBStompFrame_init(&frm);
	NBString_init(&str);
	NBString_init(&tmpBuff);
	{
		//test parsing message
		SI32 i; for(i = 0; i < (sizeof(samples) / sizeof(samples[0])); i++){
			UI32 iPos = 0, cmsd = 0;
			const char* sample = samples[i];
			const UI32 sampleLen = NBString_strLenBytes(sample);
			NBStompFrame_feedStart(&frm, ENNBStompFrameAllocMode_External, ENNBStompFrameParseMode_Flexible);
			//
			do {
				iPos += (cmsd = NBStompFrame_feed(&frm, &sample[iPos], sampleLen - iPos + 1)); // +1 to include the '\0'
				if(cmsd < (sampleLen + 1)){
					PRINTF_INFO("NBStompFrame_feed step consumed %d of %d bytes.\n", cmsd, sampleLen + 1);
				}
			} while(cmsd > 0 && !NBStompFrame_feedIsCompleted(&frm) && !NBStompFrame_feedError(&frm));
			//
			if(!NBStompFrame_feedIsCompleted(&frm)){
				PRINTF_ERROR("NBStompFrame_feedIsCompleted failed with %d of %d bytes consumed in total.\n", cmsd, sampleLen + 1);
			} else {
				if(cmsd < (sampleLen + 1)){
					PRINTF_INFO("NBStompFrame_feed %d of %d bytes successfully consumed in total.\n", cmsd, sampleLen + 1);
				}
				NBString_empty(&str);
				NBStompFrame_concatWithExternalBuffer(&frm, sample, sampleLen, FALSE, &str);
				PRINTF_INFO("NBStompFrame-parsed:\n---->\n%s\n<----\n", str.str);
				//print unscaped
				{
					UI32 i;
					STNBStompStrPtr cmd = NBStompFrame_getCommandUnscapedWithExternalBuffer(&frm, sample, sampleLen, &tmpBuff);
					NBString_empty(&str);
					NBString_concat(&str, "unscaped cmd: '"); NBString_concatBytes(&str, cmd.ptr, cmd.len); NBString_concat(&str, "'\n");
					for(i = 0; i < frm.hdrs.use; i++){
						STNBStompHeader* hdr = NBArray_itmPtrAtIndex(&frm.hdrs, STNBStompHeader, i);
						STNBStompStrPtr name, value;
						name = NBStompFrame_getStrUnscapedWithExternalBuffer(&hdr->name, sample, sampleLen, &tmpBuff);
						NBString_concat(&str, "param-"); NBString_concatUI32(&str, i); 
						NBString_concat(&str, ", '"); NBString_concatBytes(&str, name.ptr, name.len); NBString_concat(&str, "'");
						value = NBStompFrame_getStrUnscapedWithExternalBuffer(&hdr->value, sample, sampleLen, &tmpBuff);
						NBString_concat(&str, " = '"); NBString_concatBytes(&str, value.ptr, value.len); NBString_concat(&str, "'");
						NBString_concat(&str, "\n");
					}
				}
				PRINTF_INFO("NBStompFrame-details:\n---->\n%s\n<----\n\n\n\n\n", str.str);
				//test subscription search
				{
					const char* hdrName = "subscription";
					STNBStompStrPtr value = NBStompFrame_getHdrValueUnscapedWithExternalBuffer(&frm, hdrName, sample, sampleLen, &tmpBuff);
					NBString_empty(&str);
					if(value.len <= 0){
						NBString_concat(&str, "header('");
						NBString_concat(&str, hdrName);
						NBString_concat(&str, "') = NOT-FOUND\n");
					} else {
						NBString_concat(&str, "header('");
						NBString_concat(&str, hdrName);
						NBString_concat(&str, "') = '");
						NBString_concatBytes(&str, value.ptr, value.len);
						NBString_concat(&str, "'");
						NBString_concat(&str, "\n");
					}
					PRINTF_INFO("%s\n", str.str);
				}
			}
		}
		//test building message
		{
			const char* body = "This is the message.";
			STNBStompHeaderInline hdrs[] = {
				{ "param1", "value1" }
				, { "param:2", "value\r\n:\\2" }
				, { "param3", "value3" }
			};
			STNBString str2;
			NBString_init(&str2);
			if(!NBStompFrame_concatWithHeaders("HOLA", hdrs, (sizeof(hdrs) / sizeof(hdrs[0])), body, NBString_strLenBytes(body), FALSE, &str2)){
				PRINTF_ERROR("NBStompFrame_concatWithHeaders failed.\n");
			} else {
				UI32 iPos = 0, cmsd = 0;
				const char* sample = str2.str;
				const UI32 sampleLen = str2.length;
				NBStompFrame_feedStart(&frm, ENNBStompFrameAllocMode_External, ENNBStompFrameParseMode_Flexible);
				PRINTF_INFO("NBStompFrame-built:\n---->\n%s\n<----\n\n\n\n\n", str2.str);
				//
				do {
					iPos += (cmsd = NBStompFrame_feed(&frm, &sample[iPos], sampleLen - iPos + 1)); // +1 to include the '\0'
					if(cmsd < (sampleLen + 1)){
						PRINTF_INFO("NBStompFrame_feed step consumed %d of %d bytes.\n", cmsd, sampleLen + 1);
					}
				} while(cmsd > 0 && !NBStompFrame_feedIsCompleted(&frm) && !NBStompFrame_feedError(&frm));
				//
				if(!NBStompFrame_feedIsCompleted(&frm)){
					PRINTF_ERROR("NBStompFrame_feedIsCompleted failed with %d of %d bytes consumed in total.\n", cmsd, sampleLen + 1);
				} else {
					if(cmsd < (sampleLen + 1)){
						PRINTF_INFO("NBStompFrame_feed %d of %d bytes successfully consumed in total.\n", cmsd, sampleLen + 1);
					}
					NBString_empty(&str);
					NBStompFrame_concatWithExternalBuffer(&frm, sample, sampleLen, FALSE, &str);
					PRINTF_INFO("NBStompFrame-built:\n---->\n%s\n<----\n", str.str);
					//print unscaped
					{
						UI32 i;
						STNBStompStrPtr cmd = NBStompFrame_getCommandUnscapedWithExternalBuffer(&frm, sample, sampleLen, &tmpBuff);
						NBString_empty(&str);
						NBString_concat(&str, "unscaped cmd: '"); NBString_concatBytes(&str, cmd.ptr, cmd.len); NBString_concat(&str, "'\n");
						for(i = 0; i < frm.hdrs.use; i++){
							STNBStompHeader* hdr = NBArray_itmPtrAtIndex(&frm.hdrs, STNBStompHeader, i);
							STNBStompStrPtr name, value;
							name = NBStompFrame_getStrUnscapedWithExternalBuffer(&hdr->name, sample, sampleLen, &tmpBuff);
							NBString_concat(&str, "param-"); NBString_concatUI32(&str, i); 
							NBString_concat(&str, ", '"); NBString_concatBytes(&str, name.ptr, name.len); NBString_concat(&str, "'");
							value = NBStompFrame_getStrUnscapedWithExternalBuffer(&hdr->value, sample, sampleLen, &tmpBuff);
							NBString_concat(&str, " = '"); NBString_concatBytes(&str, value.ptr, value.len); NBString_concat(&str, "'");
							NBString_concat(&str, "\n");
						}
						//test unscaped-hdr-name search
						{
							const char* hdrName = "param1";
							STNBStompStrPtr value = NBStompFrame_getHdrValueUnscapedWithExternalBuffer(&frm, hdrName, sample, sampleLen, &tmpBuff);
							NBASSERT(value.len > 0)
							NBString_concat(&str, "non-scaped-hdr-search('");
							NBString_concat(&str, hdrName);
							NBString_concat(&str, "') = '");
							NBString_concatBytes(&str, value.ptr, value.len);
							NBString_concat(&str, "'");
							NBString_concat(&str, "\n");
						}
						//test scaped-hdr-name search
						{
							const char* hdrName = "param:2";
							STNBStompStrPtr value = NBStompFrame_getHdrValueUnscapedWithExternalBuffer(&frm, hdrName, sample, sampleLen, &tmpBuff);
							NBASSERT(value.len > 0)
							NBString_concat(&str, "scaped-hdr-search('");
							NBString_concat(&str, hdrName);
							NBString_concat(&str, "') = '");
							NBString_concatBytes(&str, value.ptr, value.len);
							NBString_concat(&str, "'");
							NBString_concat(&str, "\n");
						}
						//test subscription search
						{
							const char* hdrName = "subscription";
							STNBStompStrPtr value = NBStompFrame_getHdrValueUnscapedWithExternalBuffer(&frm, hdrName, sample, sampleLen, &tmpBuff);
							if(value.len <= 0){
								NBString_concat(&str, "header('");
								NBString_concat(&str, hdrName);
								NBString_concat(&str, "') = NOT-FOUND\n");
							} else {
								NBString_concat(&str, "header('");
								NBString_concat(&str, hdrName);
								NBString_concat(&str, "') = '");
								NBString_concatBytes(&str, value.ptr, value.len);
								NBString_concat(&str, "'");
								NBString_concat(&str, "\n");
							}
						}
					}
					PRINTF_INFO("NBStompFrame-details:\n---->\n%s\n<----\n\n\n\n\n", str.str);
				}
			}
			NBString_release(&str2);
		}
	}
	NBStompFrame_release(&frm);
	NBString_release(&str);
	NBString_release(&tmpBuff);
	return r;
}
#endif

//NBStompFrame

void NBStompFrame_init(STNBStompFrame* obj){
	NBMemory_setZeroSt(*obj, STNBStompFrame);
	//command
	{
		obj->cmd.knownId = ENNBStompCommand_Count;
		NBStompStr_init(&obj->cmd.str);
	}
	//headers
	{
		NBArray_init(&obj->hdrs, sizeof(STNBStompHeader), NULL);
	}
	//body
	{
		NBStompStr_init(&obj->body);
	}
	//buffer
	{
		NBString_initWithSz(&obj->buff, 0, 1024, 0.1f);
	}
}

void NBStompFrame_release(STNBStompFrame* obj){
	//command
	{
		obj->cmd.knownId = ENNBStompCommand_Count;
		NBStompStr_release(&obj->cmd.str);
	}
	//headers
	{
		SI32 i; for(i = 0; i < obj->hdrs.use; i++){
			STNBStompHeader* hdr = NBArray_itmPtrAtIndex(&obj->hdrs, STNBStompHeader, i);
			NBStompHeader_release(hdr);
		}
		NBArray_empty(&obj->hdrs);
		NBArray_release(&obj->hdrs);
	}
	//body
	{
		NBStompStr_release(&obj->body);
	}
	//buffer
	{
		NBString_release(&obj->buff);
	}
	//parser state
	if(obj->parser != NULL){
		NBStompFrameParseState_release((STNBStompFrameParseState*)obj->parser);
		NBMemory_free(obj->parser);
		obj->parser=  NULL;
	}
}

void NBStompFrame_reset(STNBStompFrame* obj){
	//command
	{
		obj->cmd.knownId = ENNBStompCommand_Count;
		NBStompStr_reset(&obj->cmd.str);
	}
	//headers
	{
		SI32 i; for(i = 0; i < obj->hdrs.use; i++){
			STNBStompHeader* hdr = NBArray_itmPtrAtIndex(&obj->hdrs, STNBStompHeader, i);
			NBStompHeader_release(hdr);
		}
		NBArray_empty(&obj->hdrs);
	}
	//body
	{
		NBStompStr_reset(&obj->body);
	}
	//buffer
	{
		NBString_empty(&obj->buff);
	}
}

//NBStompCommandDef

static const STNBStompCommandDef __cmdDefs[] = {
	{
		ENNBStompCommand_SEND 		//uid
		, "SEND"					//str
		, TRUE						//fromClt
		, FALSE						//froSrvr
	}
	, {
		ENNBStompCommand_SUBSCRIBE	//uid
		, "SUBSCRIBE"				//str
		, TRUE						//fromClt
		, FALSE						//froSrvr
	}
	, {
		ENNBStompCommand_UNSUBSCRIBE //uid
		, "UNSUBSCRIBE"				//str
		, TRUE						//fromClt
		, FALSE						//froSrvr
	}
	, {
		ENNBStompCommand_BEGIN		//uid
		, "BEGIN"					//str
		, TRUE						//fromClt
		, FALSE						//froSrvr
	}
	, {
		ENNBStompCommand_COMMIT		//uid
		, "COMMIT"					//str
		, TRUE						//fromClt
		, FALSE						//froSrvr
	}
	, {
		ENNBStompCommand_ABORT		//uid
		, "ABORT"					//str
		, TRUE						//fromClt
		, FALSE						//froSrvr
	}
	, { 
		ENNBStompCommand_ACK		//uid
		, "ACK"						//str
		, TRUE						//fromClt
		, FALSE						//froSrvr
	}
	, {
		ENNBStompCommand_NACK		//uid
		, "NACK"					//str
		, TRUE						//fromClt
		, FALSE						//froSrvr
	}
	, {
		ENNBStompCommand_DISCONNECT	//uid
		, "DISCONNECT"				//str
		, TRUE						//fromClt
		, FALSE						//froSrvr
	}
	, {
		ENNBStompCommand_CONNECT	//uid
		, "CONNECT"					//str
		, TRUE						//fromClt
		, FALSE						//froSrvr
	}
	, {
		ENNBStompCommand_STOMP		//uid
		, "STOMP"					//str
		, TRUE						//fromClt
		, FALSE						//froSrvr
	}
	//server
	, {
		ENNBStompCommand_CONNECTED	//uid
		, "CONNECTED"				//str
		, FALSE						//fromClt
		, TRUE						//froSrvr
	}
	, {
		ENNBStompCommand_MESSAGE	//uid
		, "MESSAGE"					//dtr
		, FALSE						//fromClt
		, TRUE						//froSrvr
	}
	, {
		ENNBStompCommand_RECEIPT	//uid
		, "RECEIPT"					//str
		, FALSE						//fromClt
		, TRUE						//froSrvr
	}
	, {
		ENNBStompCommand_ERROR		//uid
		, "ERROR"					//str
		, FALSE						//fromClt
		, TRUE						//froSrvr
	}
};

const STNBStompCommandDef* NBStompCommandDef_getByUid(const ENNBStompCommand uid){
	return (uid >= 0 && uid < ENNBStompCommand_Count ? &__cmdDefs[uid] : NULL);
}

const STNBStompCommandDef* NBStompCommandDef_getByStr(const char* strCommand){
	const STNBStompCommandDef* r = NULL;
	SI32 i; for(i = (SI32)(sizeof(__cmdDefs) / sizeof(__cmdDefs[0])) - 1; i >= 0; i--){
		if(NBString_strIsEqual(__cmdDefs[i].str, strCommand)){
			r = &__cmdDefs[i];
			break;
		}
	}
	return r;
}

const STNBStompCommandDef* NBStompCommandDef_getByStrBytes(const char* strCommand, const UI32 strCommandSz){
	const STNBStompCommandDef* r = NULL;
	SI32 i; for(i = (SI32)(sizeof(__cmdDefs) / sizeof(__cmdDefs[0])) - 1; i >= 0; i--){
		if(NBString_strIsEqualStrBytes(__cmdDefs[i].str, strCommand, strCommandSz)){
			r = &__cmdDefs[i];
			break;
		}
	}
	return r;
}

//NBStompStr

void NBStompStr_init(STNBStompStr* obj){
	NBMemory_setZeroSt(*obj, STNBStompStr);
} 

void NBStompStr_release(STNBStompStr* obj){
	//ToDo: implement
}

void NBStompStr_reset(STNBStompStr* obj){
	NBMemory_setZeroSt(*obj, STNBStompStr);
}

void NBStompStr_swap(STNBStompStr* obj, STNBStompStr* other){
	STNBStompStr tmp = *obj;
	*obj		= *other;
	*other		= tmp;
}

//NBStompHeader

void NBStompHeader_init(STNBStompHeader* obj){
	NBMemory_setZeroSt(*obj, STNBStompHeader);
	NBStompStr_init(&obj->name);
	NBStompStr_init(&obj->value);
}

void NBStompHeader_release(STNBStompHeader* obj){
	NBStompStr_release(&obj->name);
	NBStompStr_release(&obj->value);
}

//NBStompFrameParseState

void NBStompFrameParseState_init(STNBStompFrameParseState* obj){
	NBStompStr_init(&obj->curStr);
	NBMemory_setZeroSt(obj->cur, STNBStompFrameParseStateCur);
}

void NBStompFrameParseState_release(STNBStompFrameParseState* obj){
	NBStompStr_release(&obj->curStr);
}

void NBStompFrameParseState_reset(STNBStompFrameParseState* obj){
	NBStompStr_reset(&obj->curStr);
	NBMemory_setZeroSt(obj->cur, STNBStompFrameParseStateCur);
}

void NBStompFrameParseState_startNextPart(STNBStompFrameParseState* obj, NBStompFrameParseConsumeMethod csmMethod, const UI32 buffCurPos){
	NBStompStr_reset(&obj->curStr);
	obj->curStr.iStart	= buffCurPos;
	//
	obj->cur.charCRFound = FALSE;
	obj->cur.method		= csmMethod;
}

//parser methods

UI32 NBStompFrameParse_commandLn_(STNBStompFrame* obj, STNBStompFrameParseState* state, const char* str, const UI32 iPos, const UI32 strSz){
	/*
	frame               = command EOL ...
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
	*/
	UI32 ignoreCount = 0;
	const char* cur = str + iPos;
	const char* curAfterEnd = str + strSz;
	while(cur < curAfterEnd){
		if(state->cur.charCRFound && *cur != '\n'){
			PRINTF_ERROR("NBStompFrameParse, expected '\n' after '\r'.\n");
			state->cur.errFound = TRUE;
			break;
		} else if((*cur > 47 && *cur < 58) || (*cur > 64 && *cur < 91) || (*cur > 96 && *cur < 123) ){ //48-57 (digits), 65-90 (uppercase), 97-122 (lowercase)
			//valid char
			cur++;
		} else if(*cur == '\r'){
			NBASSERT(!state->cur.charCRFound)
			state->cur.charCRFound = TRUE;
			cur++; ignoreCount++;
		} else if(*cur == '\n'){
			//consume pending
			{
				const UI32 cPend = (UI32)(cur - ignoreCount - iPos - str); NBASSERT((str + iPos + ignoreCount) <= cur)
				if(cPend > 0){
					const STNBStompCommandDef* def = NULL;
					const char* cmdStr = NULL;
					state->curStr.len += cPend;
					if(state->cur.allocMode == ENNBStompFrameAllocMode_External){
						cmdStr = &str[state->curStr.iStart];
						NBASSERT((state->curStr.iStart + state->curStr.len) <= (UI32)(cur - str))
					} else {
						NBString_concatBytes(&obj->buff, str, cPend); NBASSERT((state->curStr.iStart + state->curStr.len) <= obj->buff.length)
						cmdStr = &obj->buff.str[state->curStr.iStart];
					}
					def = NBStompCommandDef_getByStrBytes(cmdStr, state->curStr.len);
					if(def == NULL){
						PRINTF_WARNING("NBStompFrameParse, unexpected command-name.\n")
						obj->cmd.knownId = ENNBStompCommand_Count;
					} else {
						//PRINTF_INFO("NBStompFrameParse, command-def '%s'.\n", def->str);
						obj->cmd.knownId = def->uid;
					}
					ignoreCount += cPend; 
				}
			}
			//end of name
			cur++; ignoreCount++;
			//move to next part
			NBStompStr_swap(&obj->cmd.str, &state->curStr);
			NBStompFrameParseState_startNextPart(state, NBStompFrameParse_headerNameOrEmpty_, (state->cur.allocMode == ENNBStompFrameAllocMode_External ? (UI32)(cur - str) : obj->buff.length));
			break;
		} else {
			//unexpected char
			PRINTF_INFO("NBStompFrameParse, unexpected char '%c' at command.\n", *cur);
			state->cur.errFound = TRUE;
			break;
		}
	}
	//consume pending
	{
		const UI32 cPend = (UI32)(cur - ignoreCount - iPos - str); NBASSERT((str + iPos + ignoreCount) <= cur)
		if(cPend > 0){
			state->curStr.len += cPend;
			if(state->cur.allocMode == ENNBStompFrameAllocMode_External){
				//external buffer must be provided as complete frames
				PRINTF_ERROR("NBStompFrameParse, expected full-frame when using an external-buffer.\n");
				state->cur.errFound = TRUE;
				NBASSERT((state->curStr.iStart + state->curStr.len) <= (UI32)(cur - str))
			} else {
				NBString_concatBytes(&obj->buff, str, cPend); NBASSERT((state->curStr.iStart + state->curStr.len) <= obj->buff.length)
			}
		}
	}
	//return 
	return (UI32)(cur - iPos - str);
}

UI32 NBStompFrameParse_headerNameOrEmpty_(STNBStompFrame* obj, STNBStompFrameParseState* state, const char* str, const UI32 iPos, const UI32 strSz){
	/*
	 frame               = ...
						   *( header EOL )
						   EOL
	 ...
	 header              = header-name ":" header-value
	 header-name         = 1*<any OCTET except CR or LF or ":">
	 header-value        = *<any OCTET except CR or LF or ":">
	*/
	UI32 ignoreCount = 0;
	const char* cur = str + iPos;
	const char* curAfterEnd = str + strSz;
	while(cur < curAfterEnd){
		if(state->cur.charCRFound && *cur != '\n'){
			PRINTF_ERROR("NBStompFrameParse, expected '\n' after '\r'.\n");
			state->cur.errFound = TRUE;
			break;
		} else if(*cur != '\r' && *cur != '\n' && *cur != ':'){
			//valid char
			cur++;
		} else if(*cur == '\r'){
			NBASSERT(!state->cur.charCRFound)
			state->cur.charCRFound = TRUE;
			cur++; ignoreCount++;
		} else if(*cur == ':' || *cur == '\n'){
			const char c = *cur;
			//consume pending
			{
				const UI32 cPend = (UI32)(cur - ignoreCount - iPos - str); NBASSERT((str + iPos + ignoreCount) <= cur)
				if(cPend > 0){
					state->curStr.len += cPend;
					if(state->cur.allocMode == ENNBStompFrameAllocMode_External){
						NBASSERT((state->curStr.iStart + state->curStr.len) <= (UI32)(cur - str))
					} else {
						NBString_concatBytes(&obj->buff, str, cPend); NBASSERT((state->curStr.iStart + state->curStr.len) <= obj->buff.length)
					}
					ignoreCount += cPend; 
				}
			}
			//end of name
			cur++; ignoreCount++;
			//move to next part
			if(state->curStr.len == 0 && c == '\n'){
				//empty-line, move to body
				NBStompFrameParseState_startNextPart(state, NBStompFrameParse_body_, (state->cur.allocMode == ENNBStompFrameAllocMode_External ? (UI32)(cur - str) : obj->buff.length));
			} else if(state->curStr.len != 0 && c == ':'){
				//new named-header, move to value
				STNBStompHeader hdr;
				NBStompHeader_init(&hdr);
				NBStompStr_swap(&hdr.name, &state->curStr);
				NBArray_addValue(&obj->hdrs, hdr);
				NBStompFrameParseState_startNextPart(state, NBStompFrameParse_headerValue_, (state->cur.allocMode == ENNBStompFrameAllocMode_External ? (UI32)(cur - str) : obj->buff.length));
			} else {
				PRINTF_ERROR("NBStompFrameParse, empty header-name found.\n");
				state->cur.errFound = TRUE;
			}
			break;
		} else {
			//unexpected char
			PRINTF_INFO("NBStompFrameParse, unexpected char '%c' at header-name.\n", *cur);
			state->cur.errFound = TRUE;
			break;
		}
	}
	//consume pending
	{
		const UI32 cPend = (UI32)(cur - ignoreCount - iPos - str); NBASSERT((str + iPos + ignoreCount) <= cur)
		if(cPend > 0){
			state->curStr.len += cPend;
			if(state->cur.allocMode == ENNBStompFrameAllocMode_External){
				//external buffer must be provided as complete frames
				PRINTF_ERROR("NBStompFrameParse, expected full-frame when using an external-buffer.\n");
				state->cur.errFound = TRUE;
				NBASSERT((state->curStr.iStart + state->curStr.len) <= (UI32)(cur - str))
			} else {
				NBString_concatBytes(&obj->buff, str, cPend); NBASSERT((state->curStr.iStart + state->curStr.len) <= obj->buff.length)
			}
		}
	}
	//return 
	return (UI32)(cur - iPos - str);
}

UI32 NBStompFrameParse_headerValue_(STNBStompFrame* obj, STNBStompFrameParseState* state, const char* str, const UI32 iPos, const UI32 strSz){
	/*
	 frame               = ...
						   *( header EOL )
						   EOL
	 ...
	 header              = header-name ":" header-value
	 header-name         = 1*<any OCTET except CR or LF or ":">
	 header-value        = *<any OCTET except CR or LF or ":">
	*/
	UI32 ignoreCount = 0;
	const char* cur = str + iPos;
	const char* curAfterEnd = str + strSz;
	while(cur < curAfterEnd){
		if(state->cur.charCRFound && *cur != '\n'){
			PRINTF_ERROR("NBStompFrameParse, expected '\n' after '\r'.\n");
			state->cur.errFound = TRUE;
			break;
		} else if(*cur != '\r' && *cur != '\n' && (*cur != ':' || state->cur.parseMode == ENNBStompFrameParseMode_Flexible)){
			//valid char
			cur++;
		} else if(*cur == '\r'){
			NBASSERT(!state->cur.charCRFound)
			state->cur.charCRFound = TRUE;
			cur++; ignoreCount++;
		} else if(*cur == '\n'){
			//consume pending
			{
				const UI32 cPend = (UI32)(cur - ignoreCount - iPos - str); NBASSERT((str + iPos + ignoreCount) <= cur)
				if(cPend > 0){
					state->curStr.len += cPend;
					if(state->cur.allocMode == ENNBStompFrameAllocMode_External){
						NBASSERT((state->curStr.iStart + state->curStr.len) <= (UI32)(cur - str))
					} else {
						NBString_concatBytes(&obj->buff, str, cPend); NBASSERT((state->curStr.iStart + state->curStr.len) <= obj->buff.length)
					}
					ignoreCount += cPend; 
				}
			}
			//end of name
			cur++; ignoreCount++;
			//move to next part
			{
				STNBStompHeader* hdr = NBArray_itmPtrAtIndex(&obj->hdrs, STNBStompHeader, obj->hdrs.use - 1);
				NBStompStr_swap(&hdr->value, &state->curStr);
				//analize
				{
					const char* name = NULL;
					const char* value = NULL;
					if(state->cur.allocMode == ENNBStompFrameAllocMode_External){
						name	= &str[hdr->name.iStart];
						value	= &str[hdr->value.iStart];
					} else {
						name	= &obj->buff.str[hdr->name.iStart];
						value	= &obj->buff.str[hdr->value.iStart];
					}
					if(NBString_strIsLikeStrBytes("content-length", name, hdr->name.len)){
						if(!state->cur.contentLength.isHdrFound){ //only-once, the first
							if(hdr->value.len <= 0){
								PRINTF_ERROR("NBStompFrameParse, empty 'content-length' found.\n");
								state->cur.errFound = TRUE;
							} else {
								BOOL success = FALSE;
								const SI32 val = NBNumParser_toSI32Bytes(value, hdr->value.len, &success);
								if(!success){
									PRINTF_ERROR("NBStompFrameParse, non-numeric 'content-length' found.\n");
									state->cur.errFound = TRUE;
								} else if(val < 0){
									PRINTF_ERROR("NBStompFrameParse, negative 'content-length' found.\n");
									state->cur.errFound = TRUE;
								} else {
									state->cur.contentLength.isHdrFound = TRUE;
									state->cur.contentLength.value		= (UI32)val;
									//PRINTF_INFO("NBStompFrameParse, expecting %d bytes body ('content-length' found).\n", val);
								}
							}
						}
					}
				}
				NBStompFrameParseState_startNextPart(state, NBStompFrameParse_headerNameOrEmpty_, (state->cur.allocMode == ENNBStompFrameAllocMode_External ? (UI32)(cur - str) : obj->buff.length));
			}
			//
			break;
		} else {
			//unexpected char
			PRINTF_ERROR("NBStompFrameParse, unexpected char '%c' at header-value.\n", *cur);
			state->cur.errFound = TRUE;
			break;
		}
	}
	//consume pending
	{
		const UI32 cPend = (UI32)(cur - ignoreCount - iPos - str); NBASSERT((str + iPos + ignoreCount) <= cur)
		if(cPend > 0){
			state->curStr.len += cPend;
			if(state->cur.allocMode == ENNBStompFrameAllocMode_External){
				//external buffer must be provided as complete frames
				PRINTF_ERROR("NBStompFrameParse, expected full-frame when using an external-buffer.\n");
				state->cur.errFound = TRUE;
				NBASSERT((state->curStr.iStart + state->curStr.len) <= (UI32)(cur - str))
			} else {
				NBString_concatBytes(&obj->buff, str, cPend); NBASSERT((state->curStr.iStart + state->curStr.len) <= obj->buff.length)
			}
		}
	}
	//return 
	return (UI32)(cur - iPos - str);
}

UI32 NBStompFrameParse_body_(STNBStompFrame* obj, STNBStompFrameParseState* state, const char* str, const UI32 iPos, const UI32 strSz){
	/*
	NULL                = <US-ASCII null (octet 0)>
	LF                  = <US-ASCII line feed (aka newline) (octet 10)>
	CR                  = <US-ASCII carriage return (octet 13)>
	EOL                 = [CR] LF 
	OCTET               = <any 8-bit sequence of data>

	frame               = ...
						  EOL
						  *OCTET
						  NULL
						  ...
	*/
	UI32 ignoreCount = 0;
	const char* cur = str + iPos;
	const char* curAfterEnd = str + strSz;
	while(cur < curAfterEnd){
		if(*cur != '\0' || (state->cur.contentLength.isHdrFound && (state->curStr.len + (UI32)(cur - ignoreCount - iPos - str)) < state->cur.contentLength.value)){
			//valid char
			cur++;
		} else {
			//consume pending
			{
				const UI32 cPend = (UI32)(cur - ignoreCount - iPos - str); NBASSERT((str + iPos + ignoreCount) <= cur)
				if(cPend > 0){
					state->curStr.len += cPend;
					if(state->cur.allocMode == ENNBStompFrameAllocMode_External){
						NBASSERT((state->curStr.iStart + state->curStr.len) <= (UI32)(cur - str))
					} else {
						NBString_concatBytes(&obj->buff, str, cPend); NBASSERT((state->curStr.iStart + state->curStr.len) <= obj->buff.length)
					}
					ignoreCount += cPend; 
				}
			}
			//end of name
			cur++; ignoreCount++;
			//if(state->curStr.len > 0){
			//	PRINTF_INFO("NBStompFrameParse, %d bytes body found.\n", state->curStr.len);
			//}
			//move to next part
			{
				NBStompStr_swap(&obj->body, &state->curStr);
				NBStompFrameParseState_startNextPart(state, NBStompFrameParse_trailingEOL_, (state->cur.allocMode == ENNBStompFrameAllocMode_External ? (UI32)(cur - str) : obj->buff.length));
			}
			//
			break;
		}
	}
	//consume pending
	{
		const UI32 cPend = (UI32)(cur - ignoreCount - iPos - str); NBASSERT((str + iPos + ignoreCount) <= cur)
		if(cPend > 0){
			state->curStr.len += cPend;
			if(state->cur.allocMode == ENNBStompFrameAllocMode_External){
				//external buffer must be provided as complete frames
				PRINTF_ERROR("NBStompFrameParse, expected full-frame when using an external-buffer.\n");
				state->cur.errFound = TRUE;
				NBASSERT((state->curStr.iStart + state->curStr.len) <= (UI32)(cur - str))
			} else {
				NBString_concatBytes(&obj->buff, str, cPend); NBASSERT((state->curStr.iStart + state->curStr.len) <= obj->buff.length)
			}
		}
	}
	//return 
	return (UI32)(cur - iPos - str);
}

UI32 NBStompFrameParse_trailingEOL_(STNBStompFrame* obj, STNBStompFrameParseState* state, const char* str, const UI32 iPos, const UI32 strSz){
	/*
	frame               = ...
						  *( EOL )
	 */
	UI32 ignoreCount = 0;
	const char* cur = str + iPos;
	const char* curAfterEnd = str + strSz;
	while(cur < curAfterEnd){
		if(state->cur.charCRFound && *cur != '\n'){
			PRINTF_ERROR("NBStompFrameParse, expected '\n' after '\r'.\n");
			state->cur.errFound = TRUE;
			break;
		} else if(*cur == '\r'){
			NBASSERT(!state->cur.charCRFound)
			state->cur.charCRFound = TRUE;
			cur++; ignoreCount++;
		} else if(*cur == '\n'){
			//end of name
			state->cur.charCRFound = FALSE;
			cur++; ignoreCount++;
		} else {
			//frame-completed
			NBStompFrameParseState_startNextPart(state, NULL, (state->cur.allocMode == ENNBStompFrameAllocMode_External ? (UI32)(cur - str) : obj->buff.length));
			break;
		}
	}
	//consume pending
	{
		const UI32 cPend = (UI32)(cur - ignoreCount - iPos - str); NBASSERT((str + iPos + ignoreCount) <= cur)
		if(cPend > 0){
			state->curStr.len += cPend;
			if(state->cur.allocMode == ENNBStompFrameAllocMode_External){
				//external buffer must be provided as complete frames
				PRINTF_ERROR("NBStompFrameParse, expected full-frame when using an external-buffer.\n");
				state->cur.errFound = TRUE;
				NBASSERT((state->curStr.iStart + state->curStr.len) <= (UI32)(cur - str))
			} else {
				NBString_concatBytes(&obj->buff, str, cPend); NBASSERT((state->curStr.iStart + state->curStr.len) <= obj->buff.length)
			}
		}
	}
	//return 
	return (UI32)(cur - iPos - str);
}
