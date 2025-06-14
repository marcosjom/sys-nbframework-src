
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBSdp.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBNumParser.h"
#include "nb/core/NBString.h"

//-----------------
//-- RTP - RFC2327
//-- https://datatracker.ietf.org/doc/html/rfc2327
//--
//-- MIME: "application/sdp"
//--
//-- An SDP session description consists of a number of lines of text of
//-- the form <type>=<value> <type> is always exactly one character and is
//-- case-significant.  <value> is a structured text string whose format
//-- depends on <type>.  It also will be case-significant unless a
//-- specific field defines otherwise.  Whitespace is not permitted either
//-- side of the `=' sign. In general <value> is either a number of fields
//-- delimited by a single space character or a free format string.
//--
//-- Session description
//-- v=  (protocol version)
//-- o=  (owner/creator and session identifier).
//-- s=  (session name)
//-- i=* (session information)
//-- u=* (URI of description)
//-- e=* (email address)
//-- p=* (phone number)
//-- c=* (connection information - not required if included in all media)
//-- b=* (bandwidth information)
//-- One or more time descriptions (see below)
//-- z=* (time zone adjustments)
//-- k=* (encryption key)
//-- a=* (zero or more session attribute lines)
//-- Zero or more media descriptions (see below)
//-- 
//-- Time description
//-- t=  (time the session is active)
//-- r=* (zero or more repeat times)
//-- 
//-- Media description
//-- m=  (media name and transport address)
//-- i=* (media title)
//-- c=* (connection information - optional if included at session-level)
//-- b=* (bandwidth information)
//-- k=* (encryption key)
//-- a=* (zero or more media attribute lines)

static STNBSdpTypeDef _typesDefs[] = {
	//-- Session description
	{ ENNBSdpType_v, 'v', FALSE, "protocol version" }
	, { ENNBSdpType_o, 'o', FALSE, "owner/creator and session identifier" }
	, { ENNBSdpType_s, 's', FALSE, "session name" }
	, { ENNBSdpType_i, 'i', TRUE, "session/media information" }
	, { ENNBSdpType_u, 'u', TRUE, "URI of description" }
	, { ENNBSdpType_e, 'e', TRUE, "email address" }
	, { ENNBSdpType_p, 'p', TRUE, "phone number" }
	, { ENNBSdpType_c, 'c', TRUE, "connection information" }
	, { ENNBSdpType_b, 'b', TRUE, "bandwidth information" }
	, { ENNBSdpType_z, 'z', TRUE, "time zone adjustments" }
	, { ENNBSdpType_k, 'k', TRUE, "encryption key" }
	, { ENNBSdpType_a, 'a', TRUE, "session/media attribute line" }
	//
	, { ENNBSdpType_t, 't', FALSE, "time session active" }
	, { ENNBSdpType_r, 'r', TRUE, "repeat time" }
	//
	, { ENNBSdpType_m, 'm', FALSE, "media name and transport address" }
};

STNBSdpTypeDef* STNBSdp_getTypeDefById(const ENNBSdpType type){
	STNBSdpTypeDef* r = NULL;
	if(type > ENNBSdpType_undef && type < ENNBSdpType_Count){
		r = &_typesDefs[type];
	} NBASSERT(r != NULL) //ToDo: remove after testing
	return r;
}

STNBSdpTypeDef* STNBSdp_getTypeDefByChar(const char letter){
	STNBSdpTypeDef* r = NULL;
	switch (letter) {
		case 'v': r = &_typesDefs[ENNBSdpType_v - 1]; break;
		case 'o': r = &_typesDefs[ENNBSdpType_o - 1]; break;
		case 's': r = &_typesDefs[ENNBSdpType_s - 1]; break;
		case 'i': r = &_typesDefs[ENNBSdpType_i - 1]; break;
		case 'u': r = &_typesDefs[ENNBSdpType_u - 1]; break;
		case 'e': r = &_typesDefs[ENNBSdpType_e - 1]; break;
		case 'p': r = &_typesDefs[ENNBSdpType_p - 1]; break;
		case 'c': r = &_typesDefs[ENNBSdpType_c - 1]; break;
		case 'b': r = &_typesDefs[ENNBSdpType_b - 1]; break;
		case 'z': r = &_typesDefs[ENNBSdpType_z - 1]; break;
		case 'k': r = &_typesDefs[ENNBSdpType_k - 1]; break;
		case 'a': r = &_typesDefs[ENNBSdpType_a - 1]; break;
		//
		case 't': r = &_typesDefs[ENNBSdpType_t - 1]; break;
		case 'r': r = &_typesDefs[ENNBSdpType_r - 1]; break;
		//
		case 'm': r = &_typesDefs[ENNBSdpType_m - 1]; break;
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		default:
			NBASSERT(FALSE) 
			break;
#		endif
	} NBASSERT(r != NULL && r->letter == letter)
	return r;
}

//Line

UI32 NBSdpLine_appendText(STNBSdpLine* obj, const char expFirstLetter, const ENNBSdpType expType, const UI32 curLen, const char* str, const UI32 strLen){
	UI32 r = 0;
	//First letter
	if(r < strLen && obj->type == ENNBSdpType_undef){
		if(str[r] == expFirstLetter){
			obj->type = expType;
			r++;
		}
	}
	//Content
	if(r < strLen && obj->type == expType){
		//Equal
		if(!obj->eqFound){
			if(str[r] == '='){
				obj->eqFound = TRUE;
				r++;
			}
		}
		//First char after '='
		if(r < strLen && obj->eqFound && !obj->isOpen){
			obj->iStart = curLen + r;
			obj->isOpen = TRUE;
		}
		//text
		{
			char c;
			while(r < strLen && obj->isOpen && !obj->isClosed){
				c = str[r];
				if(c == '\n' || c == '\0'){
					r++; obj->isClosed = TRUE;
					break;
				} else if(c == '\r'){
					if(obj->mustClose){
						//multiple '\r' found
						break;
					} else {
						//'\n' must follow
						r++; obj->mustClose = TRUE;
					}
				} else if(obj->mustClose){
					//'\n' must follow
					break;
				} else {
					obj->len++;
					r++;
				}
			}
		}
	}
	return r;
}

//TimeDesc

void NBSdpTimeDesc_init(STNBSdpTimeDesc* obj){
	NBMemory_setZeroSt(*obj, STNBSdpTimeDesc);
}

void NBSdpTimeDesc_release(STNBSdpTimeDesc* obj){
	if(obj->repeat != NULL){
		NBMemory_free(obj->repeat);
		obj->repeat = NULL;
		obj->repeatSz = 0;
	}
}

//-- Time description
//-- t=  (time the session is active)
//-- r=* (zero or more repeat times)

UI32 NBSdpTimeDesc_appendText(STNBSdpTimeDesc* obj, const UI32 curLen, const char* str, const UI32 strLen){
	UI32 r = 0;
	//t
	if(r < strLen && !obj->time.line.isClosed){
		r += NBSdpLine_appendText(&obj->time.line, 't', ENNBSdpType_t, curLen + r, &str[r], strLen - r);
		//Parse
		if(obj->time.line.isClosed){
			//t=<start time>  <stop time>
			if(!NBSdpTimes_parseInternalLine(&obj->time, str - curLen)){
				obj->errFnd = TRUE;
			}
		}
	}
	//r
	if(r < strLen && obj->time.line.isClosed){
		while(r < strLen){
			//Start new 'r'
			if(str[r] == 'r' && (obj->repeat == NULL || obj->repeat[obj->repeatSz - 1].isClosed)){
				const UI32 nSz	= obj->repeatSz + 1;
				STNBSdpLine* nArr = NBMemory_allocTypes(STNBSdpLine, nSz);
				if(obj->repeat != NULL){
					if(obj->repeatSz > 0){
						NBMemory_copy(nArr, obj->repeat, sizeof(obj->repeat[0]) * obj->repeatSz);
					}
					NBMemory_free(obj->repeat);
				}
				NBMemory_setZeroSt(nArr[obj->repeatSz], STNBSdpLine);
				obj->repeat		= nArr;
				obj->repeatSz	= nSz;
			}
			//Concat to last 'r'
			if(obj->repeat != NULL && !obj->repeat[obj->repeatSz - 1].isClosed){
				STNBSdpLine* ln = &obj->repeat[obj->repeatSz -1];
				const UI32 csmd = NBSdpLine_appendText(ln, 'r', ENNBSdpType_r, curLen + r, &str[r], strLen - r); 
				if(csmd == 0){
					break;
				}
				r += csmd;
			} else {
				break;
			}
		}
	}
	//
	return r;
}

BOOL NBSdpTimeDesc_canBeClosed(STNBSdpTimeDesc* obj){
	BOOL r = FALSE;
	if(obj->time.line.isClosed && (obj->repeat == NULL || obj->repeat[obj->repeatSz - 1].isClosed)){
		r = TRUE;
	}
	return r;
}

//MediaDesc

void NBSdpMediaDesc_init(STNBSdpMediaDesc* obj){
	NBMemory_setZeroSt(*obj, STNBSdpMediaDesc);
}

void NBSdpMediaDesc_release(STNBSdpMediaDesc* obj){
	NBMemory_setZeroSt(obj->nameAddr, STNBSdpLine);
	if(obj->title != NULL){
		NBMemory_free(obj->title);
		obj->title = NULL;
	}
	if(obj->connInfo != NULL){
		NBMemory_free(obj->connInfo);
		obj->connInfo = NULL;
	}
	if(obj->bandInfo != NULL){
		NBMemory_free(obj->bandInfo);
		obj->bandInfo = NULL;
	}
	if(obj->encKey != NULL){
		NBMemory_free(obj->encKey);
		obj->encKey = NULL;
	}
	if(obj->attribs != NULL){
		NBMemory_free(obj->attribs);
		obj->attribs = NULL;
		obj->attribsSz = 0;
	}
}

//-- Media description
//-- m=  (media name and transport address)
//-- i=* (media title)
//-- c=* (connection information - optional if included at session-level)
//-- b=* (bandwidth information)
//-- k=* (encryption key)
//-- a=* (zero or more media attribute lines)

UI32 NBSdpMediaDesc_appendText(STNBSdpMediaDesc* obj, const UI32 curLen, const char* str, const UI32 strLen){
	UI32 r = 0;
	//m
	if(r < strLen && !obj->nameAddr.isClosed){
		r += NBSdpLine_appendText(&obj->nameAddr, 'm', ENNBSdpType_m, curLen + r, &str[r], strLen - r);
	}
	//i
	if(r < strLen && obj->nameAddr.isClosed){
		//'i'
		if(obj->connInfo == NULL && obj->bandInfo == NULL && obj->encKey == NULL && obj->attribs == NULL){
			//Start new 'i'
			if(str[r] == 'i' && obj->title == NULL){
				obj->title = NBMemory_allocType(STNBSdpLine);
				NBMemory_setZeroSt(*obj->title, STNBSdpLine);
			}
			//Concat to 'i'
			if(obj->title != NULL && !obj->title->isClosed){
				r += NBSdpLine_appendText(obj->title, 'i', ENNBSdpType_i, curLen + r, &str[r], strLen - r); 
			}
		}
		//'c'
		if(r < strLen && (obj->title == NULL || obj->title->isClosed) && obj->bandInfo == NULL && obj->encKey == NULL && obj->attribs == NULL){
			//Start new 'c'
			if(str[r] == 'c' && obj->connInfo == NULL){
				obj->connInfo = NBMemory_allocType(STNBSdpConnData);
				NBMemory_setZeroSt(*obj->connInfo, STNBSdpConnData);
			}
			//Concat to 'c'
			if(obj->connInfo != NULL && !obj->connInfo->line.isClosed){
				r += NBSdpLine_appendText(&obj->connInfo->line, 'c', ENNBSdpType_c, curLen + r, &str[r], strLen - r);
				//Parse line
				if(obj->connInfo->line.isClosed){
					//c=<network type> <address type> <connection address>
					if(!NBSdpConnData_parseInternalLine(obj->connInfo, str - curLen)){
						obj->errFnd = TRUE;
					}
				}
			}
		}
		//'b'
		if(r < strLen && (obj->title == NULL || obj->title->isClosed) && (obj->connInfo == NULL || obj->connInfo->line.isClosed) && obj->encKey == NULL && obj->attribs == NULL){
			//Start new 'b'
			if(str[r] == 'b' && obj->bandInfo == NULL){
				obj->bandInfo = NBMemory_allocType(STNBSdpBandwidth);
				NBMemory_setZeroSt(*obj->bandInfo, STNBSdpBandwidth);
			}
			//Concat to 'b'
			if(obj->bandInfo != NULL && !obj->bandInfo->line.isClosed){
				r += NBSdpLine_appendText(&obj->bandInfo->line, 'b', ENNBSdpType_b, curLen + r, &str[r], strLen - r);
				//Parse
				if(obj->bandInfo->line.isClosed){
					//b=<modifier>:<bandwidth-value>
					if(!NBSdpBandwidth_parseInternalLine(obj->bandInfo, str - curLen)){
						obj->errFnd = TRUE;
					}
				}
			}
		}
		//'k'
		if(r < strLen && (obj->title == NULL || obj->title->isClosed) && (obj->connInfo == NULL || obj->connInfo->line.isClosed) && (obj->bandInfo == NULL || obj->bandInfo->line.isClosed) && obj->attribs == NULL){
			//Start new 'k'
			if(str[r] == 'k' && obj->encKey == NULL){
				obj->encKey = NBMemory_allocType(STNBSdpLine);
				NBMemory_setZeroSt(*obj->encKey, STNBSdpLine);
			}
			//Concat to 'k'
			if(obj->encKey != NULL && !obj->encKey->isClosed){
				r += NBSdpLine_appendText(obj->encKey, 'k', ENNBSdpType_k, curLen + r, &str[r], strLen - r); 
			}
		}
		//a
		if(r < strLen && (obj->title == NULL || obj->title->isClosed) && (obj->connInfo == NULL || obj->connInfo->line.isClosed) && (obj->bandInfo == NULL || obj->bandInfo->line.isClosed) && (obj->encKey == NULL || obj->encKey->isClosed)){
			while(r < strLen){
				//Start new 'a'
				if(str[r] == 'a' && (obj->attribs == NULL || obj->attribs[obj->attribsSz - 1].isClosed)){
					const UI32 nSz	= obj->attribsSz + 1;
					STNBSdpLine* nArr = NBMemory_allocTypes(STNBSdpLine, nSz);
					if(obj->attribs != NULL){
						if(obj->attribsSz > 0){
							NBMemory_copy(nArr, obj->attribs, sizeof(obj->attribs[0]) * obj->attribsSz);
						}
						NBMemory_free(obj->attribs);
					}
					NBMemory_setZeroSt(nArr[obj->attribsSz], STNBSdpLine);
					obj->attribs	= nArr;
					obj->attribsSz	= nSz;
				}
				//Concat to last 'a'
				if(obj->attribs != NULL && !obj->attribs[obj->attribsSz - 1].isClosed){
					STNBSdpLine* ln = &obj->attribs[obj->attribsSz -1];
					const UI32 csmd = NBSdpLine_appendText(ln, 'a', ENNBSdpType_a, curLen + r, &str[r], strLen - r); 
					if(csmd == 0){
						break;
					}
					r += csmd;
				} else {
					break;
				}
			}
		}
	}
	//
	return r;
}

BOOL NBSdpMediaDesc_canBeClosed(STNBSdpMediaDesc* obj){
	BOOL r = FALSE;
	if(obj->nameAddr.isClosed
	   && (obj->title == NULL || obj->title->isClosed)
	   && (obj->connInfo == NULL || obj->connInfo->line.isClosed)
	   && (obj->bandInfo == NULL || obj->bandInfo->line.isClosed)
	   && (obj->encKey == NULL || obj->encKey->isClosed)
	   && (obj->attribs == NULL || obj->attribs[obj->attribsSz - 1].isClosed)
	   )
	{
		r = TRUE;
	}
	return r;
}

//SessionDesc

void NBSdpSessionDesc_init(STNBSdpSessionDesc* obj){
	NBMemory_setZeroSt(*obj, STNBSdpSessionDesc);
}

void NBSdpSessionDesc_release(STNBSdpSessionDesc* obj){
	NBMemory_setZeroSt(obj->version, STNBSdpVersion);
	NBMemory_setZeroSt(obj->origin, STNBSdpOrigin);
	NBMemory_setZeroSt(obj->name, STNBSdpLine);
	if(obj->info != NULL){
		NBMemory_free(obj->info);
		obj->info = NULL;
	}
	if(obj->uri != NULL){
		NBMemory_free(obj->uri);
		obj->uri = NULL;
	}
	if(obj->email != NULL){
		NBMemory_free(obj->email);
		obj->email = NULL;
	}
	if(obj->phone != NULL){
		NBMemory_free(obj->phone);
		obj->phone = NULL;
	}
	if(obj->connInfo != NULL){
		NBMemory_free(obj->connInfo);
		obj->connInfo = NULL;
	}
	if(obj->bandInfo != NULL){
		NBMemory_free(obj->bandInfo);
		obj->bandInfo = NULL;
	}
	if(obj->timeDescs != NULL){
		UI32 i; for(i = 0; i < obj->timeDescsSz; i++){
			STNBSdpTimeDesc* td = &obj->timeDescs[i];
			NBSdpTimeDesc_release(td);
		}
		NBMemory_free(obj->timeDescs);
		obj->timeDescs = NULL;
		obj->timeDescsSz = 0;
	}
	if(obj->timeZone != NULL){
		NBMemory_free(obj->timeZone);
		obj->timeZone = NULL;
	}
	if(obj->encKey != NULL){
		NBMemory_free(obj->encKey);
		obj->encKey = NULL;
	}
	if(obj->attribs != NULL){
		NBMemory_free(obj->attribs);
		obj->attribs = NULL;
		obj->attribsSz = 0;
	}
	if(obj->mediaDescs != NULL){
		UI32 i; for(i = 0; i < obj->mediaDescsSz; i++){
			STNBSdpMediaDesc* md = &obj->mediaDescs[i];
			NBSdpMediaDesc_release(md);
		}
		NBMemory_free(obj->mediaDescs);
		obj->mediaDescs = NULL;
		obj->mediaDescsSz = 0;
	}
}

//Times

BOOL NBSdpTimes_parseInternalLine(STNBSdpTimes* obj, const char* strBuff){
	BOOL r = FALSE;
	const char* c = &strBuff[obj->line.iStart];
	const char* cPos = c;
	const char* cAfterEnd = c + obj->line.len;
	UI32 iSpcs[2], iSpcFnd = 0;
	while(cPos < cAfterEnd && iSpcFnd < 3){
		if(*cPos == ' '){
			iSpcs[iSpcFnd++] = (UI32)(cPos - c);
		}
		//next char
		cPos++;
	}
	if(iSpcFnd == 1){
		BOOL success0			= FALSE;
		BOOL success1			= FALSE;
		const char* start		= c;
		const UI32 startLen		= iSpcs[0];
		const char* stop		= c + iSpcs[0] + 1;
		const UI32 stopLen		= obj->line.len - iSpcs[0] - 1;
		//
		obj->start.time			= NBNumParser_toUI32Bytes(start, startLen, &success0);
		obj->stop.time			= NBNumParser_toUI32Bytes(stop, stopLen, &success1);
		//Result
		if(success0 && success1){
			r = TRUE;
		}
	}
	return r;
}

//Connection Data

BOOL NBSdpConnData_parseInternalLine(STNBSdpConnData* obj, const char* strBuff){
	//c=<network type> <address type> <connection address>
	// <connection address> can be: <base multicast address>/<ttl>/<number of addresses>
	BOOL r = FALSE;
	const char* c = &strBuff[obj->line.iStart];
	const char* cPos = c;
	const char* cAfterEnd = c + obj->line.len;
	UI32 iSpcs[3], iSpcFnd = 0;
	while(cPos < cAfterEnd && iSpcFnd < 3){
		if(*cPos == ' '){
			iSpcs[iSpcFnd++] = (UI32)(cPos - c);
		}
		//next char
		cPos++;
	}
	if(iSpcFnd == 2){
		const char* netType		= c;
		const UI32 netTypeLen	= iSpcs[0];
		const char* addrType	= c + iSpcs[0] + 1;
		const UI32 addrTypeLen	= iSpcs[1] - iSpcs[0] - 1;
		const char* address		= c + iSpcs[1] + 1;
		const UI32 addressLen	= obj->line.len - iSpcs[1] - 1;
		//netType
		obj->netType.iStart		= obj->line.iStart + (UI32)(netType - c);
		obj->netType.len		= (UI16)netTypeLen;
		//addrType
		obj->addrType.iStart	= obj->line.iStart + (UI32)(addrType - c);
		obj->addrType.len		= (UI16)addrTypeLen;
		//address
		obj->address.iStart		= obj->line.iStart + (UI32)(address - c);
		obj->address.len		= (UI16)addressLen;
		//
		r = TRUE;
	}
	return r;
}

//Bandwidth

BOOL NBSdpBandwidth_parseInternalLine(STNBSdpBandwidth* obj, const char* strBuff){
	//b=<modifier>:<bandwidth-value>
	BOOL r = FALSE;
	const char* c = &strBuff[obj->line.iStart];
	const char* cPos = c;
	const char* cAfterEnd = c + obj->line.len;
	UI32 iSpcs[2], iSpcFnd = 0;
	while(cPos < cAfterEnd && iSpcFnd < 3){
		if(*cPos == ':'){
			iSpcs[iSpcFnd++] = (UI32)(cPos - c);
		}
		//next char
		cPos++;
	}
	if(iSpcFnd == 1){
		BOOL success			= FALSE;
		const char* modif		= c;
		const UI32 modifLen		= iSpcs[0];
		const char* bval		= c + iSpcs[0] + 1;
		const UI32 bvalLen		= obj->line.len - iSpcs[0] - 1;
		if(modif[0] == 'X' && modif[1] == '-'){
			obj->value.type		= ENNBSdpBandwidthType_Extension;
		} else if(modif[0] == 'C' && modif[1] == 'T' && modifLen == 2){
			obj->value.type		= ENNBSdpBandwidthType_ConferenceTotal;
		} else if(modif[0] == 'A' && modif[1] == 'S' && modifLen == 2){
			obj->value.type		= ENNBSdpBandwidthType_ApplicationMax;
		} else {
			obj->value.type		= ENNBSdpBandwidthType_Unknown;
		}
		//
		obj->value.kbps			= NBNumParser_toUI32Bytes(bval, bvalLen, &success);
		//Result
		if(obj->value.type != ENNBSdpBandwidthType_Unknown && success){
			r = TRUE;
		}
	}
	return r;
}

//-- Session description
//-- v=  (protocol version)
//-- o=  (owner/creator and session identifier).
//-- s=  (session name)
//-- i=* (session information)
//-- u=* (URI of description)
//-- e=* (email address)
//-- p=* (phone number)
//-- c=* (connection information - not required if included in all media)
//-- b=* (bandwidth information)
//-- One or more time descriptions (see below)
//-- z=* (time zone adjustments)
//-- k=* (encryption key)
//-- a=* (zero or more session attribute lines)
//-- Zero or more media descriptions (see below)

UI32 NBSdpSessionDesc_appendText(STNBSdpSessionDesc* obj, const UI32 curLen, const char* str, const UI32 strLen){
	UI32 r = 0;
	//v
	if(!obj->errFnd && r < strLen && !obj->version.line.isClosed){
		r += NBSdpLine_appendText(&obj->version.line, 'v', ENNBSdpType_v, curLen + r, &str[r], strLen - r);
		//Parse line
		if(obj->version.line.isClosed){
			//v=0
			BOOL success = FALSE;
			const char* v = &str[(SI32)obj->version.line.iStart - (SI32)curLen];
			obj->version.major = NBNumParser_toUI32Bytes(v, obj->version.line.len, &success);
			if(!success){
				obj->errFnd = TRUE;
			}
			//PRINTF_INFO("v=%d.\n", obj->version.major);
		}
	}
	//o
	if(!obj->errFnd && r < strLen && obj->version.line.isClosed && !obj->origin.line.isClosed){
		r += NBSdpLine_appendText(&obj->origin.line, 'o', ENNBSdpType_o, curLen + r, &str[r], strLen - r);
		//Parse line
		if(obj->origin.line.isClosed){
			//o=<username> <session id> <version> <network type> <address type> <address>
			const char* o = &str[(SI32)obj->origin.line.iStart - (SI32)curLen];
			const char* oPos = o;
			const char* oAfterEnd = o + obj->origin.line.len;
			UI32 iSpcs[6], iSpcFnd = 0;
			while(oPos < oAfterEnd && iSpcFnd < 6){
				if(*oPos == ' '){
					iSpcs[iSpcFnd++] = (UI32)(oPos - o);
				}
				//next char
				oPos++;
			}
			if(iSpcFnd == 5){
				BOOL success = FALSE;
				const char* userName	= o;
				const char* sessionId	= o + iSpcs[0] + 1;
				const UI32 sessionIdLen = iSpcs[1] - iSpcs[0] - 1;
				const char* version		= o + iSpcs[1] + 1;
				const UI32 versionLen	= iSpcs[2] - iSpcs[1] - 1;
				const char* netType		= o + iSpcs[2] + 1;
				const UI32 netTypeLen	= iSpcs[3] - iSpcs[2] - 1;
				const char* addrType	= o + iSpcs[3] + 1;
				const UI32 addrTypeLen	= iSpcs[4] - iSpcs[3] - 1;
				const char* address		= o + iSpcs[4] + 1;
				const UI32 addressLen	= obj->origin.line.len - iSpcs[4] - 1;
				//userid
				obj->origin.userid.iStart	= obj->origin.line.iStart + (UI32)(userName - o);
				obj->origin.userid.len		= (UI16)iSpcs[0];
				obj->origin.userid.supported = (obj->origin.userid.len != 1 || *userName != '-');
				//sessionId
				obj->origin.sessionId.number = NBNumParser_toUI32Bytes(sessionId, sessionIdLen, &success);
				if(!success){
					obj->errFnd = TRUE;
				}
				//version
				obj->origin.version.number = NBNumParser_toUI32Bytes(version, versionLen, &success);
				if(!success){
					obj->errFnd = TRUE;
				}
				//netType
				obj->origin.netType.iStart	= obj->origin.line.iStart + (UI32)(netType - o);
				obj->origin.netType.len		= (UI16)netTypeLen;
				//addrType
				obj->origin.addrType.iStart	= obj->origin.line.iStart + (UI32)(addrType - o);
				obj->origin.addrType.len	= (UI16)addrTypeLen;
				//address
				obj->origin.address.iStart	= obj->origin.line.iStart + (UI32)(address - o);
				obj->origin.address.len		= (UI16)addressLen;
			}
		}
	}
	//s
	if(!obj->errFnd && r < strLen && obj->version.line.isClosed && obj->origin.line.isClosed && !obj->name.isClosed){
		r += NBSdpLine_appendText(&obj->name, 's', ENNBSdpType_s, curLen + r, &str[r], strLen - r);
	}
	//i
	if(!obj->errFnd && r < strLen && obj->name.isClosed){
		//'i'
		if(obj->uri == NULL && obj->email == NULL && obj->phone == NULL && obj->connInfo == NULL && obj->bandInfo == NULL && obj->timeDescs == NULL && obj->timeZone == NULL && obj->encKey == NULL && obj->attribs == NULL && obj->mediaDescs == NULL){
			//Start new 'i'
			if(str[r] == 'i' && obj->info == NULL){
				obj->info = NBMemory_allocType(STNBSdpLine);
				NBMemory_setZeroSt(*obj->info, STNBSdpLine);
			}
			//Concat to 'i'
			if(obj->info != NULL && !obj->info->isClosed){
				r += NBSdpLine_appendText(obj->info, 'i', ENNBSdpType_i, curLen + r, &str[r], strLen - r); 
			}
		}
		//'u'
		if(!obj->errFnd && r < strLen && (obj->info == NULL || obj->info->isClosed) && obj->email == NULL && obj->phone == NULL && obj->connInfo == NULL && obj->bandInfo == NULL && obj->timeDescs == NULL && obj->timeZone == NULL && obj->encKey == NULL && obj->attribs == NULL && obj->mediaDescs == NULL){
			//Start new 'u'
			if(str[r] == 'u' && obj->uri == NULL){
				obj->uri = NBMemory_allocType(STNBSdpLine);
				NBMemory_setZeroSt(*obj->uri, STNBSdpLine);
			}
			//Concat to 'u'
			if(obj->uri != NULL && !obj->uri->isClosed){
				r += NBSdpLine_appendText(obj->uri, 'u', ENNBSdpType_u, curLen + r, &str[r], strLen - r); 
			}
		}
		//'e'
		if(!obj->errFnd && r < strLen && (obj->info == NULL || obj->info->isClosed) && (obj->uri == NULL || obj->uri->isClosed) && obj->phone == NULL && obj->connInfo == NULL && obj->bandInfo == NULL && obj->timeDescs == NULL && obj->timeZone == NULL && obj->encKey == NULL && obj->attribs == NULL && obj->mediaDescs == NULL){
			//Start new 'e'
			if(str[r] == 'e' && obj->email == NULL){
				obj->email = NBMemory_allocType(STNBSdpLine);
				NBMemory_setZeroSt(*obj->email, STNBSdpLine);
			}
			//Concat to 'e'
			if(obj->email != NULL && !obj->email->isClosed){
				r += NBSdpLine_appendText(obj->email, 'e', ENNBSdpType_e, curLen + r, &str[r], strLen - r); 
			}
		}
		//'p'
		if(!obj->errFnd && r < strLen && (obj->info == NULL || obj->info->isClosed) && (obj->uri == NULL || obj->uri->isClosed) && (obj->email == NULL || obj->email->isClosed) && obj->connInfo == NULL && obj->bandInfo == NULL && obj->timeDescs == NULL && obj->timeZone == NULL && obj->encKey == NULL && obj->attribs == NULL && obj->mediaDescs == NULL){
			//Start new 'p'
			if(str[r] == 'p' && obj->phone == NULL){
				obj->phone = NBMemory_allocType(STNBSdpLine);
				NBMemory_setZeroSt(*obj->phone, STNBSdpLine);
			}
			//Concat to 'p'
			if(obj->phone != NULL && !obj->phone->isClosed){
				r += NBSdpLine_appendText(obj->phone, 'p', ENNBSdpType_p, curLen + r, &str[r], strLen - r); 
			}
		}
		//'c'
		if(!obj->errFnd && r < strLen && (obj->info == NULL || obj->info->isClosed) && (obj->uri == NULL || obj->uri->isClosed) && (obj->email == NULL || obj->email->isClosed) && (obj->phone == NULL || obj->phone->isClosed) && obj->bandInfo == NULL && obj->timeDescs == NULL && obj->timeZone == NULL && obj->encKey == NULL && obj->attribs == NULL && obj->mediaDescs == NULL){
			//Start new 'c'
			if(str[r] == 'c' && obj->connInfo == NULL){
				obj->connInfo = NBMemory_allocType(STNBSdpConnData);
				NBMemory_setZeroSt(*obj->connInfo, STNBSdpConnData);
			}
			//Concat to 'c'
			if(obj->connInfo != NULL && !obj->connInfo->line.isClosed){
				r += NBSdpLine_appendText(&obj->connInfo->line, 'c', ENNBSdpType_c, curLen + r, &str[r], strLen - r);
				//Parse line
				if(obj->connInfo->line.isClosed){
					//c=<network type> <address type> <connection address>
					if(!NBSdpConnData_parseInternalLine(obj->connInfo, str - curLen)){
						obj->errFnd = TRUE;
					}
				}
			}
		}
		//'b'
		if(!obj->errFnd && r < strLen && (obj->info == NULL || obj->info->isClosed) && (obj->uri == NULL || obj->uri->isClosed) && (obj->email == NULL || obj->email->isClosed) && (obj->phone == NULL || obj->phone->isClosed) && (obj->connInfo == NULL || obj->connInfo->line.isClosed) && obj->timeDescs == NULL && obj->timeZone == NULL && obj->encKey == NULL && obj->attribs == NULL && obj->mediaDescs == NULL){
			//Start new 'b'
			if(str[r] == 'b' && obj->bandInfo == NULL){
				obj->bandInfo = NBMemory_allocType(STNBSdpBandwidth);
				NBMemory_setZeroSt(*obj->bandInfo, STNBSdpBandwidth);
			}
			//Concat to 'b'
			if(obj->bandInfo != NULL && !obj->bandInfo->line.isClosed){
				r += NBSdpLine_appendText(&obj->bandInfo->line, 'b', ENNBSdpType_b, curLen + r, &str[r], strLen - r);
				//Parse
				if(obj->bandInfo->line.isClosed){
					//b=<modifier>:<bandwidth-value>
					if(!NBSdpBandwidth_parseInternalLine(obj->bandInfo, str - curLen)){
						obj->errFnd = TRUE;
					}
				}
			}
		}
		//time-descs
		if(!obj->errFnd && r < strLen && (obj->info == NULL || obj->info->isClosed) && (obj->uri == NULL || obj->uri->isClosed) && (obj->email == NULL || obj->email->isClosed) && (obj->phone == NULL || obj->phone->isClosed) && (obj->connInfo == NULL || obj->connInfo->line.isClosed) && (obj->bandInfo == NULL || obj->bandInfo->line.isClosed) && obj->timeZone == NULL && obj->encKey == NULL && obj->attribs == NULL && obj->mediaDescs == NULL){
			while(!obj->errFnd && r < strLen){
				//Start new time-desc
				if(str[r] == 't' && (obj->timeDescs == NULL || NBSdpTimeDesc_canBeClosed(&obj->timeDescs[obj->timeDescsSz - 1]))){
					const UI32 nSz	= obj->timeDescsSz + 1;
					STNBSdpTimeDesc* nArr = NBMemory_allocTypes(STNBSdpTimeDesc, nSz);
					if(obj->timeDescs != NULL){
						if(obj->timeDescsSz > 0){
							NBMemory_copy(nArr, obj->timeDescs, sizeof(obj->timeDescs[0]) * obj->timeDescsSz);
						}
						NBMemory_free(obj->timeDescs);
					}
					NBSdpTimeDesc_init(&nArr[obj->timeDescsSz]);
					obj->timeDescs		= nArr;
					obj->timeDescsSz	= nSz;
				}
				//Concat to last 'a'
				if(obj->timeDescs != NULL){
					STNBSdpTimeDesc* ln = &obj->timeDescs[obj->timeDescsSz -1];
					const UI32 csmd = NBSdpTimeDesc_appendText(ln, curLen + r, &str[r], strLen - r); 
					if(csmd == 0){
						break;
					}
					r += csmd;
				} else {
					break;
				}
			}
		}
		//'z'
		if(!obj->errFnd && r < strLen && (obj->info == NULL || obj->info->isClosed) && (obj->uri == NULL || obj->uri->isClosed) && (obj->email == NULL || obj->email->isClosed) && (obj->phone == NULL || obj->phone->isClosed) && (obj->connInfo == NULL || obj->connInfo->line.isClosed) && (obj->timeDescs == NULL || NBSdpTimeDesc_canBeClosed(&obj->timeDescs[obj->timeDescsSz - 1])) && obj->encKey == NULL && obj->attribs == NULL && obj->mediaDescs == NULL){
			//Start new 'z'
			if(str[r] == 'z' && obj->timeZone == NULL){
				obj->timeZone = NBMemory_allocType(STNBSdpLine);
				NBMemory_setZeroSt(*obj->timeZone, STNBSdpLine);
			}
			//Concat to 'z'
			if(obj->timeZone != NULL && !obj->timeZone->isClosed){
				r += NBSdpLine_appendText(obj->timeZone, 'z', ENNBSdpType_z, curLen + r, &str[r], strLen - r); 
			}
		}
		//'k'
		if(!obj->errFnd && r < strLen && (obj->info == NULL || obj->info->isClosed) && (obj->uri == NULL || obj->uri->isClosed) && (obj->email == NULL || obj->email->isClosed) && (obj->phone == NULL || obj->phone->isClosed) && (obj->connInfo == NULL || obj->connInfo->line.isClosed) && (obj->timeDescs == NULL || NBSdpTimeDesc_canBeClosed(&obj->timeDescs[obj->timeDescsSz - 1])) && (obj->timeZone == NULL || obj->timeZone->isClosed) && obj->attribs == NULL && obj->mediaDescs == NULL){
			//Start new 'k'
			if(str[r] == 'k' && obj->encKey == NULL){
				obj->encKey = NBMemory_allocType(STNBSdpLine);
				NBMemory_setZeroSt(*obj->encKey, STNBSdpLine);
			}
			//Concat to 'k'
			if(obj->encKey != NULL && !obj->encKey->isClosed){
				r += NBSdpLine_appendText(obj->encKey, 'k', ENNBSdpType_k, curLen + r, &str[r], strLen - r); 
			}
		}
		//'a'
		if(!obj->errFnd && r < strLen && (obj->info == NULL || obj->info->isClosed) && (obj->uri == NULL || obj->uri->isClosed) && (obj->email == NULL || obj->email->isClosed) && (obj->phone == NULL || obj->phone->isClosed) && (obj->connInfo == NULL || obj->connInfo->line.isClosed) && (obj->timeDescs == NULL || NBSdpTimeDesc_canBeClosed(&obj->timeDescs[obj->timeDescsSz - 1])) && (obj->timeZone == NULL || obj->timeZone->isClosed) && (obj->encKey == NULL || obj->encKey->isClosed) && obj->mediaDescs == NULL){
			while(r < strLen){
				//Start new 'a'
				if(str[r] == 'a' && (obj->attribs == NULL || obj->attribs[obj->attribsSz - 1].isClosed)){
					const UI32 nSz	= obj->attribsSz + 1;
					STNBSdpLine* nArr = NBMemory_allocTypes(STNBSdpLine, nSz);
					if(obj->attribs != NULL){
						if(obj->attribsSz > 0){
							NBMemory_copy(nArr, obj->attribs, sizeof(obj->attribs[0]) * obj->attribsSz);
						}
						NBMemory_free(obj->attribs);
					}
					NBMemory_setZeroSt(nArr[obj->attribsSz], STNBSdpLine);
					obj->attribs	= nArr;
					obj->attribsSz	= nSz;
				}
				//Concat to last 'a'
				if(obj->attribs != NULL && !obj->attribs[obj->attribsSz - 1].isClosed){
					STNBSdpLine* ln = &obj->attribs[obj->attribsSz -1];
					const UI32 csmd = NBSdpLine_appendText(ln, 'a', ENNBSdpType_a, curLen + r, &str[r], strLen - r); 
					if(csmd == 0){
						break;
					}
					r += csmd;
				} else {
					break;
				}
			}
		}
		//media-descs
		if(!obj->errFnd && r < strLen && (obj->info == NULL || obj->info->isClosed) && (obj->uri == NULL || obj->uri->isClosed) && (obj->email == NULL || obj->email->isClosed) && (obj->phone == NULL || obj->phone->isClosed) && (obj->connInfo == NULL || obj->connInfo->line.isClosed) && (obj->timeDescs == NULL || NBSdpTimeDesc_canBeClosed(&obj->timeDescs[obj->timeDescsSz - 1])) && (obj->timeZone == NULL || obj->timeZone->isClosed) && (obj->encKey == NULL || obj->encKey->isClosed) && (obj->attribs == NULL || obj->attribs[obj->attribsSz - 1].isClosed)){
			while(r < strLen){
				//Start new time-desc
				if(str[r] == 'm' && (obj->mediaDescs == NULL || NBSdpMediaDesc_canBeClosed(&obj->mediaDescs[obj->mediaDescsSz - 1]))){
					const UI32 nSz	= obj->mediaDescsSz + 1;
					STNBSdpMediaDesc* nArr = NBMemory_allocTypes(STNBSdpMediaDesc, nSz);
					if(obj->mediaDescs != NULL){
						if(obj->mediaDescsSz > 0){
							NBMemory_copy(nArr, obj->mediaDescs, sizeof(obj->mediaDescs[0]) * obj->mediaDescsSz);
						}
						NBMemory_free(obj->mediaDescs);
					}
					NBSdpMediaDesc_init(&nArr[obj->mediaDescsSz]);
					obj->mediaDescs		= nArr;
					obj->mediaDescsSz	= nSz;
				}
				//Concat to last 'a'
				if(obj->mediaDescs != NULL){
					STNBSdpMediaDesc* ln = &obj->mediaDescs[obj->mediaDescsSz -1];
					const UI32 csmd = NBSdpMediaDesc_appendText(ln, curLen + r, &str[r], strLen - r); 
					if(csmd == 0){
						break;
					}
					r += csmd;
				} else {
					break;
				}
			}
		}
	}
	//
	return r;
}

BOOL NBSdpSessionDesc_canBeClosed(STNBSdpSessionDesc* obj){
	BOOL r = FALSE;
	if(
	   obj->version.line.isClosed && obj->origin.line.isClosed && obj->name.isClosed
	   && (obj->info == NULL || obj->info->isClosed)
	   && (obj->uri == NULL || obj->uri->isClosed)
	   && (obj->email == NULL || obj->email->isClosed)
	   && (obj->phone == NULL || obj->phone->isClosed)
	   && (obj->connInfo == NULL || obj->connInfo->line.isClosed)
	   && (obj->bandInfo == NULL || obj->bandInfo->line.isClosed)
	   && (obj->timeDescs == NULL || NBSdpTimeDesc_canBeClosed(&obj->timeDescs[obj->timeDescsSz - 1]))
	   && (obj->timeZone == NULL || obj->timeZone->isClosed)
	   && (obj->encKey == NULL || obj->encKey->isClosed)
	   && (obj->attribs == NULL || obj->attribs[obj->attribsSz - 1].isClosed)
	   && (obj->mediaDescs == NULL || NBSdpMediaDesc_canBeClosed(&obj->mediaDescs[obj->mediaDescsSz - 1]))
	   )
	{
		r = TRUE;
	}
	return r;
}

//Desc

void NBSdpDesc_init(STNBSdpDesc* obj){
	NBMemory_setZeroSt(*obj, STNBSdpDesc);
}

void NBSdpDesc_release(STNBSdpDesc* obj){
	if(obj->sessions != NULL){
		UI32 i; for(i = 0; i < obj->sessionsSz; i++){
			STNBSdpSessionDesc* sd = &obj->sessions[i];
			NBSdpSessionDesc_release(sd);
		}
		NBMemory_free(obj->sessions);
		obj->sessions = NULL;
		obj->sessionsSz = 0;
	}
	if(obj->str != NULL){
		NBMemory_free(obj->str);
		obj->str = NULL;
		obj->len = 0;
	}
}

UI32 NBSdpDesc_appendText(STNBSdpDesc* obj, const char* pStr, const UI32 pStrLen, const BOOL isFinal){
	UI32 r = 0;
	const UI32 lenBefore = obj->len;
	//Copy content
	if(pStrLen > 0){
		char* strN = NBMemory_alloc(obj->len + pStrLen + 1);
		if(obj->str != NULL){
			if(obj->len > 0){
				NBMemory_copy(strN, obj->str, obj->len);
			}
			NBMemory_free(obj->str);
			obj->str = NULL;
		}
		NBMemory_copy(&strN[obj->len], pStr, pStrLen);
		obj->str	= strN;
		obj->len	+= pStrLen;
		obj->str[obj->len] = '\0';
	}
	//Process
	while(r < pStrLen){
		//Start new time-desc
		if(obj->str[lenBefore + r] == 'v' && (obj->sessions == NULL || NBSdpSessionDesc_canBeClosed(&obj->sessions[obj->sessionsSz - 1]))){
			const UI32 nSz	= obj->sessionsSz + 1;
			STNBSdpSessionDesc* nArr = NBMemory_allocTypes(STNBSdpSessionDesc, nSz);
			if(obj->sessions != NULL){
				if(obj->sessionsSz > 0){
					NBMemory_copy(nArr, obj->sessions, sizeof(obj->sessions[0]) * obj->sessionsSz);
				}
				NBMemory_free(obj->sessions);
			}
			NBSdpSessionDesc_init(&nArr[obj->sessionsSz]);
			obj->sessions	= nArr;
			obj->sessionsSz	= nSz;
		}
		//Concat to last 'v'
		if(obj->sessions != NULL){
			STNBSdpSessionDesc* ln = &obj->sessions[obj->sessionsSz - 1];
			const UI32 csmd = NBSdpSessionDesc_appendText(ln, lenBefore + r, &obj->str[lenBefore + r], pStrLen - r); 
			if(csmd == 0){
				break;
			}
			r += csmd;
		} else {
			break;
		}
	}
	if(r == pStrLen && isFinal){
		char c = '\0';
		NBSdpDesc_appendText(obj, &c, 1, FALSE);
	}
	return r;
}

void NBSdpDesc_flush(STNBSdpDesc* obj){
	char c = '\0';
	NBSdpDesc_appendText(obj, &c, 1, FALSE);
}

BOOL NBSdpDesc_canBeClosed(STNBSdpDesc* obj){
	BOOL r = FALSE;
	if(obj->sessions != NULL && NBSdpSessionDesc_canBeClosed(&obj->sessions[obj->sessionsSz - 1])){
		r = TRUE;
	}
	return r;
}

//Known SDP attributes
//https://datatracker.ietf.org/doc/html/rfc4566#section-6
//6.  SDP Attributes

//a=cat:<category>
BOOL NBSdpDesc_parseAttribCat(const char* pay, const UI32 payLen, STNBSdpAttribCat* dst){
	STNBSdpAttribCat rr;
	NBMemory_setZeroSt(rr, STNBSdpAttribCat);
	rr.cat.start	= 0;
	rr.cat.size		= payLen;
	if(dst != NULL){
		*dst = rr;
	}
	return TRUE;
}

//a=keywds:<keywords>
BOOL NBSdpDesc_parseAttribKeywords(const char* pay, const UI32 payLen, STNBSdpAttribKeywords* dst){
	STNBSdpAttribKeywords rr;
	NBMemory_setZeroSt(rr, STNBSdpAttribKeywords);
	rr.keywords.start	= 0;
	rr.keywords.size	= payLen;
	if(dst != NULL){
		*dst = rr;
	}
	return (payLen > 0);
}

//a=tool:<name and version of tool>
BOOL NBSdpDesc_parseAttribTool(const char* pay, const UI32 payLen, STNBSdpAttribTool* dst){
	STNBSdpAttribTool rr;
	NBMemory_setZeroSt(rr, STNBSdpAttribTool);
	rr.nameAndVer.start	= 0;
	rr.nameAndVer.size	= payLen;
	if(dst != NULL){
		*dst = rr;
	}
	return (payLen > 0);
}

//a=ptime:<packet time>
BOOL NBSdpDesc_parseAttribPTime(const char* pay, const UI32 payLen, STNBSdpAttribPTime* dst){
	BOOL r = FALSE;
	STNBSdpAttribPTime rr;
	NBMemory_setZeroSt(rr, STNBSdpAttribPTime);
	rr.ms = NBNumParser_toUI32Bytes(pay, payLen, &r);
	if(dst != NULL){
		*dst = rr;
	}
	return r;
}

//a=maxptime:<maximum packet time>
BOOL NBSdpDesc_parseAttribMaxPTime(const char* pay, const UI32 payLen, STNBSdpAttribMaxPTime* dst){
	BOOL r = FALSE;
	STNBSdpAttribMaxPTime rr;
	NBMemory_setZeroSt(rr, STNBSdpAttribMaxPTime);
	rr.ms = NBNumParser_toUI32Bytes(pay, payLen, &r);
	if(dst != NULL){
		*dst = rr;
	}
	return r;
}

//a=rtpmap:<payload type> <encoding name>/<clock rate> [/<encoding parameters>]
//
//m=audio 49232 RTP/AVP 98
//a=rtpmap:98 L16/16000/2
//
//m=audio 49230 RTP/AVP 96 97 98
//a=rtpmap:96 L8/8000
//a=rtpmap:97 L16/8000
//a=rtpmap:98 L16/11025/2
//
BOOL NBSdpDesc_parseAttribRtpMap(const char* pay, const UI32 payLen, STNBSdpAttribRtpMap* dst){
	BOOL r = FALSE;
	STNBSdpAttribRtpMap rr;
	NBMemory_setZeroSt(rr, STNBSdpAttribRtpMap);
	const SI32 sep0 = NBString_strIndexOfBytes(pay, payLen, " ", 0);
	if(sep0 > 0){
		BOOL rrr = FALSE;
		rr.payType = NBNumParser_toUI32Bytes(pay, sep0, &rrr);
		if(rrr){
			const SI32 sep1 = NBString_strIndexOfBytes(pay, payLen, "/", sep0);
			if(sep1 > sep0){
				SI32 sep22			= NBString_strIndexOfBytes(pay, payLen, " ", sep1 + 1);
				SI32 sep33			= NBString_strIndexOfBytes(pay, payLen, "/", sep1 + 1);
				SI32 sepMin, sepMax;
				rr.encName.start	= sep0 + 1;
				rr.encName.size		= sep1 - sep0 - 1;
				if(sep22 < 0) sep22	= payLen;
				if(sep33 < 0) sep33	= payLen;
				sepMin				= (sep22 < sep33 ? sep22 : sep33);
				sepMax				= (sep22 > sep33 ? sep22 : sep33);
				if(sepMin > sep1){
					rr.clockRate	= NBNumParser_toUI32Bytes(&pay[sep1 + 1], sepMin - sep1 - 1, &rrr);
					if(sepMax < payLen){
						rr.encParams.start	= sepMax + 1;
						rr.encParams.size	= (payLen - sepMax - 1);
					}
					r = TRUE;
				}
			}
		}
	}
	if(dst != NULL){
		*dst = rr;
	}
	return r;
}

//a=recvonly
BOOL NBSdpDesc_parseAttribRecvOnly(const char* pay, const UI32 payLen, STNBSdpAttribRecvOnly* dst){
	STNBSdpAttribRecvOnly rr;
	NBMemory_setZeroSt(rr, STNBSdpAttribRecvOnly);
	rr.recvOnly = TRUE;
	if(dst != NULL){
		*dst = rr;
	}
	return TRUE;
}

//a=sendrecv
BOOL NBSdpDesc_parseAttribSendRecv(const char* pay, const UI32 payLen, STNBSdpAttribSendRecv* dst){
	STNBSdpAttribSendRecv rr;
	NBMemory_setZeroSt(rr, STNBSdpAttribSendRecv);
	rr.sendRecv = TRUE;
	if(dst != NULL){
		*dst = rr;
	}
	return TRUE;
}

//a=sendonly
BOOL NBSdpDesc_parseAttribSendOnly(const char* pay, const UI32 payLen, STNBSdpAttribSendOnly* dst){
	STNBSdpAttribSendOnly rr;
	NBMemory_setZeroSt(rr, STNBSdpAttribSendOnly);
	rr.sendOnly = TRUE;
	if(dst != NULL){
		*dst = rr;
	}
	return TRUE;
}

//a=inactive
BOOL NBSdpDesc_parseAttribInactive(const char* pay, const UI32 payLen, STNBSdpAttribInactive* dst){
	STNBSdpAttribInactive rr;
	NBMemory_setZeroSt(rr, STNBSdpAttribInactive);
	rr.inactive = TRUE;
	if(dst != NULL){
		*dst = rr;
	}
	return TRUE;
}

//a=orient:<orientation>
//Permitted values are "portrait", "landscape", and "seascape" (upside-down landscape).

BOOL NBSdpDesc_parseAttribOrient(const char* pay, const UI32 payLen, STNBSdpAttribOrient* dst){
	BOOL r = FALSE;
	STNBSdpAttribOrient rr;
	NBMemory_setZeroSt(rr, STNBSdpAttribOrient);
	if(NBString_strIsEqualStrBytes("portrait", pay, payLen)){
		rr.orient = ENNBSdpAttribOrient_Portrait;
		r = TRUE;
	} else if(NBString_strIsEqualStrBytes("landscape", pay, payLen)){
		rr.orient = ENNBSdpAttribOrient_Landscape;
		r = TRUE;
	} else if(NBString_strIsEqualStrBytes("seascape", pay, payLen)){
		rr.orient = ENNBSdpAttribOrient_Seascape;
		r = TRUE;
	}
	if(dst != NULL){
		*dst = rr;
	}
	return r;
}

//a=type:<conference type>
//Suggested values are "broadcast", "meeting", "moderated", "test", and "H332".
BOOL NBSdpDesc_parseAttribType(const char* pay, const UI32 payLen, STNBSdpAttribType* dst){
	STNBSdpAttribType rr;
	NBMemory_setZeroSt(rr, STNBSdpAttribType);
	rr.type.start	= 0;
	rr.type.size	= payLen;
	if(dst != NULL){
		*dst = rr;
	}
	return TRUE;
}

//a=charset:<character set>
BOOL NBSdpDesc_parseAttribCharset(const char* pay, const UI32 payLen, STNBSdpAttribCharset* dst){
	STNBSdpAttribCharset rr;
	NBMemory_setZeroSt(rr, STNBSdpAttribCharset);
	rr.charset.start = 0;
	rr.charset.size	= payLen;
	if(dst != NULL){
		*dst = rr;
	}
	return TRUE;
}

//a=sdplang:<language tag>
//language tag: https://datatracker.ietf.org/doc/html/rfc3066
BOOL NBSdpDesc_parseAttribSdpLang(const char* pay, const UI32 payLen, STNBSdpAttribSdpLang* dst){
	STNBSdpAttribSdpLang rr;
	NBMemory_setZeroSt(rr, STNBSdpAttribSdpLang);
	rr.sdpLang.start = 0;
	rr.sdpLang.size = payLen;
	if(dst != NULL){
		*dst = rr;
	}
	return TRUE;
}

//a=lang:<language tag>
//language tag: https://datatracker.ietf.org/doc/html/rfc3066
BOOL NBSdpDesc_parseAttribLang(const char* pay, const UI32 payLen, STNBSdpAttribLang* dst){
	STNBSdpAttribLang rr;
	NBMemory_setZeroSt(rr, STNBSdpAttribLang);
	rr.lang.start	= 0;
	rr.lang.size	= payLen;
	if(dst != NULL){
		*dst = rr;
	}
	return TRUE;
}

//a=framerate:<frame rate>
//Decimal representations of fractional values using the notation "<integer>.<fraction>" are allowed
BOOL NBSdpDesc_parseAttribFramerate(const char* pay, const UI32 payLen, STNBSdpAttribFramerate* dst){
	BOOL r = FALSE;
	STNBSdpAttribFramerate rr;
	NBMemory_setZeroSt(rr, STNBSdpAttribFramerate);
	const SI32 sep0 = NBString_strIndexOfBytes(pay, payLen, ".", 0);
	if(sep0 < 0){
		rr.integer = NBNumParser_toUI32Bytes(pay, payLen, &r);
	} else {
		rr.integer = NBNumParser_toUI32Bytes(pay, sep0, &r);
		if(r){
			rr.decimal = NBNumParser_toUI32Bytes(&pay[sep0 + 1], payLen - sep0 - 1, &r);
		}
	}
	if(dst != NULL){
		*dst = rr;
	}
	return r;
}

//a=quality:<quality>
//For video, the value is in the range 0 to 10 (best).
BOOL NBSdpDesc_parseAttribQuality(const char* pay, const UI32 payLen, STNBSdpAttribQuality* dst){
	BOOL r = FALSE;
	STNBSdpAttribQuality rr;
	NBMemory_setZeroSt(rr, STNBSdpAttribQuality);
	rr.quality = NBNumParser_toUI32Bytes(pay, payLen, &r);
	if(dst != NULL){
		*dst = rr;
	}
	return r;
}

//a=fmtp:<format> <format specific parameters>
/*typedef struct STNBSdpAttribFmtp_ {
	UI32 format;
	STNBRangeI params;
} STNBSdpAttribFmtp;*/

BOOL NBSdpDesc_parseAttribFmtp(const char* pay, const UI32 payLen, STNBSdpAttribFmtp* dst){
	BOOL r = FALSE;
	STNBSdpAttribFmtp rr;
	NBMemory_setZeroSt(rr, STNBSdpAttribFmtp);
	const SI32 sep0 = NBString_strIndexOfBytes(pay, payLen, " ", 0);
	if(sep0 >= 0){
		BOOL rrr = FALSE;
		rr.format = NBNumParser_toUI32Bytes(pay, sep0, &rrr);
		if(rrr){
			rr.params.start = sep0 + 1;
			rr.params.size	= (payLen - sep0 - 1);
			r = TRUE;
		}
	}
	if(dst != NULL){
		*dst = rr;
	}
	return r;
}

#ifdef NB_CONFIG_INCLUDE_ASSERTS
#	include "nb/core/NBString.h"
#endif

#ifdef NB_CONFIG_INCLUDE_ASSERTS
BOOL NBSdpDesc_dbgTest(void){
	BOOL r = TRUE;
	{
		const char* samples[] = { 
			"v=0\n\
o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\n\
s=SDP Seminar\n\
i=A Seminar on the session description protocol\n\
u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\n\
e=mjh@isi.edu (Mark Handley)\n\
c=IN IP4 224.2.17.12/127\n\
t=2873397496 2873404696\n\
a=recvonly\n\
m=audio 49170 RTP/AVP 0\n\
m=video 51372 RTP/AVP 31\n\
m=application 32416 udp wb\n\
a=orient:portrait"
			, "v=0\r\n\
o=- 1625504730476217 1625504730476217 IN IP4 192.168.0.221\r\n\
s=Media Presentation\r\n\
e=NONE\r\n\
b=AS:5050\r\n\
t=0 0\r\n\
a=control:rtsp://192.168.0.221/Streaming/channels/103/trackID=1/\r\n\
m=video 0 RTP/AVP 96\r\n\
c=IN IP4 0.0.0.0\r\n\
b=AS:5000\r\n\
a=recvonly\r\n\
a=x-dimensions:352,240\r\n\
a=control:rtsp://192.168.0.221/Streaming/channels/103/trackID=1/trackID=1\r\n\
a=rtpmap:96 H264/90000\r\n\
a=fmtp:96 profile-level-id=420029; packetization-mode=1; sprop-parameter-sets=Z01ADY2NQLD///gH4AaLcGBgZAAAD6AAAw1DoYB6QAHoSu8uNDAPSAA9CV3lwoA=,aO44gA==\r\n\
a=Media_header:MEDIAINFO=494D4B48010300000400000100000000000000000000000000000000000000000000000000000000;\r\n\
a=appversion:1.0\r\n"
		};
		const SI32 samplesCount = (sizeof(samples) / sizeof(samples[0]));
		SI32 i = 0, errCount = 0, successCount = 0;
		for(i = 0; i < samplesCount; i++){
			const char* sample = samples[i];
			const SI32 sampleLen = NBString_strLenBytes(sample);
			SI32 sampleConsumed = 0;
			STNBSdpDesc desc;
			NBSdpDesc_init(&desc);
			sampleConsumed = NBSdpDesc_appendText(&desc, sample, sampleLen, TRUE);
			if(sampleConsumed < sampleLen){
				PRINTF_ERROR("Sample:--->\n%s<---\n", sample);
				PRINTF_ERROR("Only %d of %d bytes of sample could be processed.\n", sampleConsumed, sampleLen);
				PRINTF_ERROR("Unconsumed:--->\n%s<---\n", &sample[sampleConsumed]);
				PRINTF_ERROR("%d unconsumed bytes.\n", sampleLen - sampleConsumed);
				errCount++;
			} else if(!NBSdpDesc_canBeClosed(&desc)){
				PRINTF_ERROR("Sample:--->\n%s<---\n", sample);
				PRINTF_ERROR("Sample cannto be closed (incomplete).\n");
				errCount++;
			} else {
				//success
				successCount++;
			}
			NBSdpDesc_release(&desc);
		}
		PRINTF_INFO("%d of %d samples success (%d errors).\n", successCount, samplesCount, errCount);
	}
	return r;
}
#endif
