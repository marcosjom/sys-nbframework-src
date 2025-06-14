//
//  XUCadena.c
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
#include "nb/net/NBUrl.h"

//Factory
void NBUrl_init(STNBUrl* obj){
	obj->iScheme	= 0;	//'https'
	obj->iUser		= 0;	//'marcos'
	obj->iPasswd	= 0;	//'mypass'
	obj->iHost		= 0;	//'www.nibsa.com.ni'
	obj->iPort		= 0;	//'443'
	obj->iPath		= 0;	//'/path/res.html'
	NBArray_init(&obj->params, sizeof(STNBUrlPair), NULL);		//[{'p', 'v', 'p', 'v'}]
	NBArray_init(&obj->queries, sizeof(STNBUrlPair), NULL);		//[{'q', 'v', 'q', 'v'}]
	NBArray_init(&obj->fragments, sizeof(STNBUrlPair), NULL);	//[{'f', 'v', 'f', 'v'}]
	NBString_init(&obj->str);
	NBString_concatByte(&obj->str, '\0');
}

void NBUrl_release(STNBUrl* obj){
	obj->iScheme	= 0;			//'https'
	obj->iUser		= 0;			//'marcos'
	obj->iPasswd	= 0;			//'mypass'
	obj->iHost		= 0;			//'www.nibsa.com.ni'
	obj->iPort		= 0;			//'443'
	obj->iPath		= 0;			//'/path/res.html'
	NBArray_release(&obj->params);	//[{'p', 'v', 'p', 'v'}]
	NBArray_release(&obj->queries); //[{'q', 'v', 'q', 'v'}]
	NBArray_release(&obj->fragments); //[{'f', 'v', 'f', 'v'}]
	NBString_release(&obj->str);
}

//Parse

//Example: https://marcos:mypass@www.nibsa.com.ni:443/path/res.html;p=v&p=v?q=v&q=v#ref

BOOL NBUrl_parsePairs(STNBUrl* obj, const char* url, const UI16 start, const UI16 end, STNBArray* dst){
	BOOL r = TRUE;
	SI32 pos = start + 1;
	while(pos < end){
		SI32 eqPos = NBString_strIndexOf(url, "=", pos); if(eqPos == -1 || eqPos > end) eqPos = end;
		SI32 sepPos = NBString_strIndexOf(url, "&", pos); if(sepPos == -1 || sepPos > end) sepPos = end;
		if(eqPos >= sepPos){
			STNBUrlPair pair;
			pair.iName	= obj->str.length;
			if(!NBUrl_concatDecodedBytes(&obj->str, &url[pos], (sepPos - pos))){
				r = FALSE; NBASSERT(FALSE)
				break;
			}
			NBString_concatByte(&obj->str, '\0');
			pair.iValue	= 0;
			NBArray_addValue(dst, pair);
		} else {
			STNBUrlPair pair;
			pair.iName	= obj->str.length;
			if(!NBUrl_concatDecodedBytes(&obj->str, &url[pos], (eqPos - pos))){
				r = FALSE; NBASSERT(FALSE)
				break;
			}
			NBString_concatByte(&obj->str, '\0');
			pair.iValue	= obj->str.length;
			if(!NBUrl_concatDecodedBytes(&obj->str, &url[eqPos + 1], (sepPos - eqPos - 1))){
				r = FALSE; NBASSERT(FALSE)
				break;
			}
			NBString_concatByte(&obj->str, '\0');
			NBArray_addValue(dst, pair);
		}
		pos = sepPos + 1;
	}
	return r;
}

BOOL NBUrl_parse(STNBUrl* obj, const char* url){
	BOOL r = TRUE;
	//Reset
	{
		obj->iScheme	= 0;			//'https'
		obj->iUser		= 0;			//'marcos'
		obj->iPasswd	= 0;			//'mypass'
		obj->iHost		= 0;			//'www.nibsa.com.ni'
		obj->iPort		= 0;			//'443'
		obj->iPath		= 0;			//'/path/res.html'
		NBArray_empty(&obj->params);	//[{'p', 'v', 'p', 'v'}]
		NBArray_empty(&obj->queries);	//[{'q', 'v', 'q', 'v'}]
		NBArray_empty(&obj->fragments);	//[{'f', 'v', 'f', 'v'}]
		NBString_empty(&obj->str);
		NBString_concatByte(&obj->str, '\0');
	}
	//Parse
	//Example: https://user:mypass@www.nibsa.com.ni:443/path/res.html;p=v&p=v?q=v&q=v#ref
	{
		const UI16 urlLen = (UI16)NBString_strLenBytes(url);
		UI16 curLeft = 0;
		//Get scheme and host
		{
			const SI32 sepScheme = NBString_strIndexOf(url, "://", curLeft);
			if(sepScheme != -1){
				obj->iScheme = (UI16)obj->str.length;
				if(!NBUrl_concatDecodedBytes(&obj->str, &url[curLeft], (sepScheme - curLeft))){
					r = FALSE; NBASSERT(FALSE)
				} else {
					NBString_concatByte(&obj->str, '\0');
					curLeft = sepScheme + 3;
					//Get user, password and host
					SI32 sepRes = urlLen;
					const SI32 end1 = NBString_strIndexOf(url, "/", curLeft);
					const SI32 end2 = NBString_strIndexOf(url, ";", curLeft);
					const SI32 end3 = NBString_strIndexOf(url, "?", curLeft);
					const SI32 end4 = NBString_strIndexOf(url, "#", curLeft);
					if(end1 != -1 && end1 < sepRes) sepRes = end1;
					if(end2 != -1 && end2 < sepRes) sepRes = end2;
					if(end3 != -1 && end3 < sepRes) sepRes = end3;
					if(end4 != -1 && end4 < sepRes) sepRes = end4;
					if(curLeft < sepRes){
						//User and password
						if(r){
							const SI32 sepUsr = NBString_strIndexOf(url, "@", curLeft);
							if(sepUsr >= 0 && sepUsr < sepRes){
								//User data
								const SI32 sepPass	= NBString_strIndexOf(url, ":", curLeft);
								if(sepPass >=0 && sepPass < sepUsr){
									//User:pass
									obj->iUser		= (UI16)obj->str.length;
									if(!NBUrl_concatDecodedBytes(&obj->str, &url[curLeft], (sepPass - curLeft))){
										r = FALSE; NBASSERT(FALSE)
									}
									NBString_concatByte(&obj->str, '\0');
									obj->iPasswd	= (UI16)obj->str.length;
									if(!NBUrl_concatDecodedBytes(&obj->str, &url[sepPass + 1], (sepUsr - sepPass - 1))){
										r = FALSE; NBASSERT(FALSE)
									}
									NBString_concatByte(&obj->str, '\0');
								} else {
									//Only user
									obj->iUser		= (UI16)obj->str.length;
									if(!NBUrl_concatDecodedBytes(&obj->str, &url[curLeft], (sepUsr - curLeft))){
										r = FALSE; NBASSERT(FALSE)
									}
									NBString_concatByte(&obj->str, '\0');
								}
								//
								curLeft = sepUsr + 1;
							}
						}
						//Server and port
						if(r){
							//User data
							const SI32 sepPort = NBString_strIndexOf(url, ":", curLeft);
							if(sepPort >=0 && sepPort < sepRes){
								//host:port
								obj->iHost	= (UI16)obj->str.length;
								if(!NBUrl_concatDecodedBytes(&obj->str, &url[curLeft], (sepPort - curLeft))){
									r = FALSE; NBASSERT(FALSE)
								}
								NBString_concatByte(&obj->str, '\0');
								obj->iPort	= (UI16)obj->str.length;
								if(!NBUrl_concatDecodedBytes(&obj->str, &url[sepPort + 1], (sepRes - sepPort - 1))){
									r = FALSE; NBASSERT(FALSE)
								}
								NBString_concatByte(&obj->str, '\0');
							} else {
								//Only host
								obj->iHost	= (UI16)obj->str.length;
								if(!NBUrl_concatDecodedBytes(&obj->str, &url[curLeft], (sepRes - curLeft))){
									r = FALSE; NBASSERT(FALSE)
								}
								NBString_concatByte(&obj->str, '\0');
							}
						}
					}
					curLeft = (UI16)sepRes;
				}
			}
		}
		//Get resource
		if(curLeft < urlLen){
			SI32 sepRes = urlLen;
			const SI32 end1 = NBString_strIndexOf(url, ";", curLeft);
			const SI32 end2 = NBString_strIndexOf(url, "?", curLeft);
			const SI32 end3 = NBString_strIndexOf(url, "#", curLeft);
			if(end1 != -1 && end1 < sepRes) sepRes = end1;
			if(end2 != -1 && end2 < sepRes) sepRes = end2;
			if(end3 != -1 && end3 < sepRes) sepRes = end3;
			if(curLeft < sepRes){
				obj->iPath	= (UI16)obj->str.length;
				if(!NBUrl_concatDecodedBytes(&obj->str, &url[curLeft], (sepRes - curLeft))){
					r = FALSE; NBASSERT(FALSE)
				}
				NBString_concatByte(&obj->str, '\0');
				curLeft		= (UI16)sepRes;
			}
		}
		//Get params (after ';')
		if(r && curLeft < urlLen){
			SI32 start = NBString_strIndexOf(url, ";", curLeft);
			if(start != -1){
				SI32 end = urlLen;
				const SI32 end1 = NBString_strIndexOf(url, "?", start + 1);
				const SI32 end2 = NBString_strIndexOf(url, "#", start + 1);
				if(end1 != -1 && end1 < end) end = end1;
				if(end2 != -1 && end2 < end) end = end2;
				if(start < end) r = NBUrl_parsePairs(obj, url, (UI16)start, (UI16)end, &obj->params);
			}
		}
		//Get queries (after '?')
		if(r && curLeft < urlLen){
			SI32 start = NBString_strIndexOf(url, "?", curLeft);
			if(start != -1){
				SI32 end = urlLen;
				const SI32 end1 = NBString_strIndexOf(url, ";", start + 1);
				const SI32 end2 = NBString_strIndexOf(url, "#", start + 1);
				if(end1 != -1 && end1 < end) end = end1;
				if(end2 != -1 && end2 < end) end = end2;
				if(start < end) r = NBUrl_parsePairs(obj, url, (UI16)start, (UI16)end, &obj->queries);
			}
		}
		//Get fragments (after '#')
		if(r && curLeft < urlLen){
			SI32 start = NBString_strIndexOf(url, "#", curLeft);
			if(start != -1){
				SI32 end = urlLen;
				const SI32 end1 = NBString_strIndexOf(url, ";", start + 1);
				const SI32 end2 = NBString_strIndexOf(url, "?", start + 1);
				if(end1 != -1 && end1 < end) end = end1;
				if(end2 != -1 && end2 < end) end = end2;
				if(start < end) r = NBUrl_parsePairs(obj, url, (UI16)start, (UI16)end, &obj->fragments);
			}
		}
	}
/*#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	//Debug: print parts
	if(!r){
		PRINTF_ERROR("Url could not be parse (malformed?): '%s'.\n", url);
	} else {
		PRINTF_INFO("----------------------\n");
		PRINTF_INFO("Url: '%s'.\n", url);
		PRINTF_INFO("Scheme: '%s'.\n", (obj->iScheme == 0 ? "(none)" : &obj->str.str[obj->iScheme]));
		PRINTF_INFO("User: '%s'.\n", (obj->iUser == 0 ? "(none)" : &obj->str.str[obj->iUser]));
		PRINTF_INFO("Pass: '%s'.\n", (obj->iPasswd == 0 ? "(none)" : &obj->str.str[obj->iPasswd]));
		PRINTF_INFO("Host: '%s'.\n", (obj->iHost == 0 ? "(none)" : &obj->str.str[obj->iHost]));
		PRINTF_INFO("Port: '%s'.\n", (obj->iPort == 0 ? "(none)" : &obj->str.str[obj->iPort]));
		PRINTF_INFO("Path: '%s'.\n", (obj->iPath == 0 ? "(none)" : &obj->str.str[obj->iPath]));
		{
			SI32 i; for(i = 0; i < obj->params.use; i++){
				const STNBUrlPair* pair = (const STNBUrlPair*)NBArray_itemAtIndex(&obj->params, i);
				PRINTF_INFO("Param#%d: '%s' = '%s'.\n", (i + 1), (pair->iName == 0 ? "(none)" : &obj->str.str[pair->iName]), (pair->iValue == 0 ? "(none)" : &obj->str.str[pair->iValue]));
			}
		}
		{
			SI32 i; for(i = 0; i < obj->queries.use; i++){
				const STNBUrlPair* pair = (const STNBUrlPair*)NBArray_itemAtIndex(&obj->queries, i);
				PRINTF_INFO("Query#%d: '%s' = '%s'.\n", (i + 1), (pair->iName == 0 ? "(none)" : &obj->str.str[pair->iName]), (pair->iValue == 0 ? "(none)" : &obj->str.str[pair->iValue]));
			}
		}
		{
			SI32 i; for(i = 0; i < obj->fragments.use; i++){
				const STNBUrlPair* pair = (const STNBUrlPair*)NBArray_itemAtIndex(&obj->fragments, i);
				PRINTF_INFO("Fragment#%d: '%s' = '%s'.\n", (i + 1), (pair->iName == 0 ? "(none)" : &obj->str.str[pair->iName]), (pair->iValue == 0 ? "(none)" : &obj->str.str[pair->iValue]));
			}
		}
	}
#	endif*/
	//
	return r;
}

//Get

const char* NBUrl_getScheme(const STNBUrl* obj, const char* defValue){
	const char* r = defValue;
	if(obj->iScheme != 0){
		r = &obj->str.str[obj->iScheme];
	}
	return r;
}

const char* NBUrl_getUser(const STNBUrl* obj, const char* defValue){
	const char* r = defValue;
	if(obj->iUser != 0){
		r = &obj->str.str[obj->iUser];
	}
	return r;
}

const char* NBUrl_getPasswd(const STNBUrl* obj, const char* defValue){
	const char* r = defValue;
	if(obj->iPasswd != 0){
		r = &obj->str.str[obj->iPasswd];
	}
	return r;
}

const char* NBUrl_getHost(const STNBUrl* obj, const char* defValue){
	const char* r = defValue;
	if(obj->iHost != 0){
		r = &obj->str.str[obj->iHost];
	}
	return r;
}

const char* NBUrl_getPort(const STNBUrl* obj, const char* defValue){
	const char* r = defValue;
	if(obj->iPort != 0){
		r = &obj->str.str[obj->iPort];
	}
	return r;
}

const char* NBUrl_getPath(const STNBUrl* obj, const char* defValue){
	const char* r = defValue;
	if(obj->iPath != 0){
		r = &obj->str.str[obj->iPath];
	}
	return r;
}

const char* NBUrl_getParamValue(const STNBUrl* obj, const char* name, const char* defValue){
	const char* r = defValue;
	SI32 i; for(i = 0; i < obj->params.use; i++){
		const STNBUrlPair* pair = (const STNBUrlPair*)NBArray_itemAtIndex(&obj->params, i);
		if(NBString_strIsEqual(&obj->str.str[pair->iName], name)){
			r = &obj->str.str[pair->iValue];
			break;
		}
	}
	return r;
}

BOOL NBUrl_getParamAtIndex(const STNBUrl* obj, const SI32 idx, const char** dstName, const char** dstValue){
	BOOL r = FALSE;
	if(idx >= 0 && idx < obj->params.use){
		const STNBUrlPair* pair = (const STNBUrlPair*)NBArray_itemAtIndex(&obj->params, idx);
		if(dstName != NULL){
			*dstName = &obj->str.str[pair->iName];
		}
		if(dstValue != NULL){
			*dstValue = &obj->str.str[pair->iValue];
		}
		r = TRUE;
	}
	return r;
}

BOOL NBUrl_getQueryAtIndex(const STNBUrl* obj, const SI32 idx, const char** dstName, const char** dstValue){
	BOOL r = FALSE;
	if(idx >= 0 && idx < obj->queries.use){
		const STNBUrlPair* pair = (const STNBUrlPair*)NBArray_itemAtIndex(&obj->queries, idx);
		if(dstName != NULL){
			*dstName = &obj->str.str[pair->iName];
		}
		if(dstValue != NULL){
			*dstValue = &obj->str.str[pair->iValue];
		}
		r = TRUE;
	}
	return r;
}

const char* NBUrl_getQueryValue(const STNBUrl* obj, const char* name, const char* defValue){
	const char* r = defValue;
	SI32 i; for(i = 0; i < obj->queries.use; i++){
		const STNBUrlPair* pair = (const STNBUrlPair*)NBArray_itemAtIndex(&obj->queries, i);
		if(NBString_strIsEqual(&obj->str.str[pair->iName], name)){
			r = &obj->str.str[pair->iValue];
			break;
		}
	}
	return r;
}

const char* NBUrl_getFragmentValue(const STNBUrl* obj, const char* name, const char* defValue){
	const char* r = defValue;
	SI32 i; for(i = 0; i < obj->fragments.use; i++){
		const STNBUrlPair* pair = (const STNBUrlPair*)NBArray_itemAtIndex(&obj->fragments, i);
		if(NBString_strIsEqual(&obj->str.str[pair->iName], name)){
			r = &obj->str.str[pair->iValue];
			break;
		}
	}
	return r;
}

//Encode

UI32 NBUrl_encodedLen(const char* plain){
	UI32 r = 0;
	const char* c = plain; char b;
	while(*c != '\0'){
		b = *c;
		if((b > 47 && b < 58) /*0-9*/ || (b > 64 && b < 91) /*A-Z*/ || (b > 96 && b < 123) /*a-z*/){
			r++;
		} else {
			r += 3;
		}
		c++;
	}
	return r;
}

UI32 NBUrl_encodedBytesLen(const char* plain, const UI32 plainSz){
	UI32 r = 0;
	UI32 i; char b;
	for(i = 0; i < plainSz; i++){
		b = plain[i];
		if((b > 47 && b < 58) /*0-9*/ || (b > 64 && b < 91) /*A-Z*/ || (b > 96 && b < 123) /*a-z*/){
			r++;
		} else {
			r += 3;
		}
	}
	return r;
}

void NBUrl_concatEncoded(STNBString* dst, const char* plain){
	const char* upperHex = "0123456789ABCDEF";
	const UI8* c = (const UI8*)plain; UI8 b;
	while(*c != '\0'){
		b = *c;
		if((b > 47 && b < 58) /*0-9*/ || (b > 64 && b < 91) /*A-Z*/ || (b > 96 && b < 123) /*a-z*/){
			NBString_concatByte(dst, b);
		} else {
			NBString_concatByte(dst, '%');
			NBString_concatByte(dst, upperHex[b >> 4]);
			NBString_concatByte(dst, upperHex[b & 0xF]);
			NBASSERT((dst->str[dst->length - 2] > 47 && dst->str[dst->length - 2] < 58) /*0-9*/ || (dst->str[dst->length - 2] > 64 && dst->str[dst->length - 2] < 71) /*A-F*/ || (dst->str[dst->length - 2] > 96 && dst->str[dst->length - 2] < 103) /*a-f*/)
			NBASSERT((dst->str[dst->length - 1] > 47 && dst->str[dst->length - 1] < 58) /*0-9*/ || (dst->str[dst->length - 1] > 64 && dst->str[dst->length - 1] < 71) /*A-F*/ || (dst->str[dst->length - 1] > 96 && dst->str[dst->length - 1] < 103) /*a-f*/)
		}
		c++;
	}
}

void NBUrl_concatEncodedBytes(STNBString* dst, const char* plain, const UI32 plainSz){
	const char* upperHex = "0123456789ABCDEF";
	UI32 i; const UI8* c = (const UI8*)plain; UI8 b;
	for(i = 0; i < plainSz; i++){
		b = c[i];
		if((b > 47 && b < 58) /*0-9*/ || (b > 64 && b < 91) /*A-Z*/ || (b > 96 && b < 123) /*a-z*/){
			NBString_concatByte(dst, b);
		} else {
			NBString_concatByte(dst, '%');
			NBString_concatByte(dst, upperHex[b >> 4]);
			NBString_concatByte(dst, upperHex[b & 0xF]);
			NBASSERT((dst->str[dst->length - 2] > 47 && dst->str[dst->length - 2] < 58) /*0-9*/ || (dst->str[dst->length - 2] > 64 && dst->str[dst->length - 2] < 71) /*A-F*/ || (dst->str[dst->length - 2] > 96 && dst->str[dst->length - 2] < 103) /*a-f*/)
			NBASSERT((dst->str[dst->length - 1] > 47 && dst->str[dst->length - 1] < 58) /*0-9*/ || (dst->str[dst->length - 1] > 64 && dst->str[dst->length - 1] < 71) /*A-F*/ || (dst->str[dst->length - 1] > 96 && dst->str[dst->length - 1] < 103) /*a-f*/)
		}
	}
}

//Decode

//Decode
BOOL NBUrl_concatDecoded(STNBString* dst, const char* encoded){
	BOOL r = TRUE;
	const UI8* c = (const UI8*)encoded; UI8 b, encSeq = 0, decVal = 0;
	while(*c != '\0'){
		b = *c;
		if(encSeq == 0){
			switch (b) {
				case '+':
					NBString_concatByte(dst, ' ');
					break;
				case '%':
					NBASSERT(encSeq == 0)
					NBASSERT(decVal == 0)
					encSeq++;
					break;
				default:
					NBString_concatByte(dst, b);
					break;
			}
		} else {
			if((b > 47 && b < 58) /*0-9*/ || (b > 64 && b < 71) /*A-F*/ || (b > 96 && b < 103) /*a-f*/){
				decVal += ((b < 58 ? 0 : 10) + (b - (b < 58 ? '0' : b < 71 ? 'A' : 'a'))) << (encSeq == 1 ? 4 : 0);
				encSeq++;
				if(encSeq > 2){
					NBString_concatByte(dst, decVal);
					encSeq = 0; decVal = 0;
				}
			} else {
				//Expected an hexadecimal value
				r = FALSE; NBASSERT(FALSE)
				break;
			}
		}
		c++;
	}
	//Last validation
	if(r){
		NBASSERT(encSeq == 0)
		r = (encSeq == 0); //No incomplete encode-sequence must remain.
	}
	return r;
}

BOOL NBUrl_concatDecodedBytes(STNBString* dst, const char* encoded, const UI32 encodedSz){
	BOOL r = TRUE;
	UI32 i; const UI8* c = (const UI8*)encoded; UI8 b, encSeq = 0, decVal = 0;
	for(i = 0; i < encodedSz; i++){
		b = c[i];
		if(encSeq == 0){
			switch (b) {
				case '+':
					NBString_concatByte(dst, ' ');
					break;
				case '%':
					NBASSERT(encSeq == 0)
					NBASSERT(decVal == 0)
					encSeq++;
					break;
				default:
					NBString_concatByte(dst, b);
					break;
			}
		} else {
			if((b > 47 && b < 58) /*0-9*/ || (b > 64 && b < 71) /*A-F*/ || (b > 96 && b < 103) /*a-f*/){
				decVal += ((b < 58 ? 0 : 10) + (b - (b < 58 ? '0' : b < 71 ? 'A' : 'a'))) << (encSeq == 1 ? 4 : 0);
				encSeq++;
				if(encSeq > 2){
					NBString_concatByte(dst, decVal);
					encSeq = 0; decVal = 0;
				}
			} else {
				//Expected an hexadecimal value
				r = FALSE; NBASSERT(FALSE)
				break;
			}
		}
	}
	return r;
}


