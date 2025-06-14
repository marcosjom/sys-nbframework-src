
#include "nb/NBFrameworkPch.h"
#include "nb/core/NBEncoding.h"
#include "nb/core/NBNumParser.h"
#include "nb/net/NBUriBuilder.h"
#include "nb/net/NBUriParser.h"

void NBUriBuilder_init(STNBUriBuilder* obj){
	obj->schemeAdded		= FALSE;
	obj->hierPartStarted	= FALSE;
	obj->queryStarted		= FALSE;
	NBString_init(&obj->buffer);
}

void NBUriBuilder_release(STNBUriBuilder* obj){
	obj->schemeAdded		= FALSE;
	obj->hierPartStarted	= FALSE;
	obj->queryStarted		= FALSE;
	NBString_release(&obj->buffer);
}

//

void NBUriBuilder_empty(STNBUriBuilder* obj){
	obj->schemeAdded		= FALSE;
	obj->hierPartStarted	= FALSE;
	obj->queryStarted		= FALSE;
	NBString_empty(&obj->buffer);
}

BOOL NBUriBuilder_addScheme(STNBUriBuilder* obj, const char* scheme){
	BOOL r = FALSE;
	const UI32 iStart = obj->buffer.length;
	if(NBUriParser_concatEncoded(&obj->buffer, scheme, ENNBUriParserType_scheme)){
		NBString_concatByte(&obj->buffer, ':');
		r = TRUE; obj->schemeAdded = TRUE;
	}
	//Remove added content
	if(!r){
		NBString_removeLastBytes(&obj->buffer, (obj->buffer.length - iStart));
	}
	return r;
}

//Hier part

BOOL NBUriBuilder_addUserInfo(STNBUriBuilder* obj, const char* userInfo){
	BOOL r = FALSE;
	const UI32 iStart = obj->buffer.length;
	if(obj->schemeAdded && !obj->hierPartStarted){
		NBString_concatByte(&obj->buffer, '/');
		NBString_concatByte(&obj->buffer, '/');
	}
	if(NBUriParser_concatEncoded(&obj->buffer, userInfo, ENNBUriParserType_userInfo)){
		NBString_concatByte(&obj->buffer, '@');
		r = TRUE; obj->hierPartStarted = TRUE;
	}
	//Remove added content
	if(!r){
		NBString_removeLastBytes(&obj->buffer, (obj->buffer.length - iStart));
	}
	return r;
}

BOOL NBUriBuilder_addHostName(STNBUriBuilder* obj, const char* regname){
	BOOL r = FALSE;
	const UI32 iStart = obj->buffer.length;
	if(obj->schemeAdded && !obj->hierPartStarted){
		NBString_concatByte(&obj->buffer, '/');
		NBString_concatByte(&obj->buffer, '/');
	}
	if(NBUriParser_concatEncoded(&obj->buffer, regname, ENNBUriParserType_regName)){
		r = TRUE; obj->hierPartStarted = TRUE;
	}
	//Remove added content
	if(!r){
		NBString_removeLastBytes(&obj->buffer, (obj->buffer.length - iStart));
	}
	return r;
}

BOOL NBUriBuilder_addHostIPv4(STNBUriBuilder* obj, const char* ipv4){
	BOOL r = FALSE;
	const UI32 iStart = obj->buffer.length;
	if(obj->schemeAdded && !obj->hierPartStarted){
		NBString_concatByte(&obj->buffer, '/');
		NBString_concatByte(&obj->buffer, '/');
	}
	if(NBUriParser_concatEncoded(&obj->buffer, ipv4, ENNBUriParserType_IPV4Address)){
		r = TRUE; obj->hierPartStarted = TRUE;
	}
	//Remove added content
	if(!r){
		NBString_removeLastBytes(&obj->buffer, (obj->buffer.length - iStart));
	}
	return r;
}

BOOL NBUriBuilder_addHostIPv6(STNBUriBuilder* obj, const char* ipv6){
	BOOL r = FALSE;
	const UI32 iStart = obj->buffer.length;
	if(obj->schemeAdded && !obj->hierPartStarted){
		NBString_concatByte(&obj->buffer, '/');
		NBString_concatByte(&obj->buffer, '/');
	}
	NBString_concatByte(&obj->buffer, '[');
	if(NBUriParser_concatEncoded(&obj->buffer, ipv6, ENNBUriParserType_IPv6Address)){
		NBString_concatByte(&obj->buffer, ']');
		r = TRUE; obj->hierPartStarted = TRUE;
	}
	//Remove added content
	if(!r){
		NBString_removeLastBytes(&obj->buffer, (obj->buffer.length - iStart));
	}
	return r;
}

BOOL NBUriBuilder_addHostIPvFuture(STNBUriBuilder* obj, const char* ipvFuture){
	BOOL r = FALSE;
	const UI32 iStart = obj->buffer.length;
	if(obj->schemeAdded && !obj->hierPartStarted){
		NBString_concatByte(&obj->buffer, '/');
		NBString_concatByte(&obj->buffer, '/');
	}
	NBString_concatByte(&obj->buffer, '[');
	if(NBUriParser_concatEncoded(&obj->buffer, ipvFuture, ENNBUriParserType_IPvFuture)){
		NBString_concatByte(&obj->buffer, ']');
		r = TRUE; obj->hierPartStarted = TRUE;
	}
	//Remove added content
	if(!r){
		NBString_removeLastBytes(&obj->buffer, (obj->buffer.length - iStart));
	}
	return r;
}

BOOL NBUriBuilder_addPort(STNBUriBuilder* obj, const UI32 port){
	if(obj->schemeAdded && !obj->hierPartStarted){
		NBString_concatByte(&obj->buffer, '/');
		NBString_concatByte(&obj->buffer, '/');
	}
	NBString_concatByte(&obj->buffer, ':');
	NBString_concatUI32(&obj->buffer, port);
	obj->hierPartStarted = TRUE;
	return TRUE;
}

//Path

BOOL NBUriBuilder_addSegment(STNBUriBuilder* obj, const char* segment){
	BOOL r = FALSE;
	const UI32 iStart = obj->buffer.length;
	NBString_concatByte(&obj->buffer, '/');
	if(NBUriParser_concatEncoded(&obj->buffer, segment, ENNBUriParserType_segment)){
		r = TRUE; obj->hierPartStarted = TRUE;
	}
	//Remove added content
	if(!r){
		NBString_removeLastBytes(&obj->buffer, (obj->buffer.length - iStart));
	}
	return r;
}

BOOL NBUriBuilder_addSegmentNz(STNBUriBuilder* obj, const char* segmentNz){
	BOOL r = FALSE;
	const UI32 iStart = obj->buffer.length;
	NBString_concatByte(&obj->buffer, '/');
	if(NBUriParser_concatEncoded(&obj->buffer, segmentNz, ENNBUriParserType_segmentNz)){
		r = TRUE; obj->hierPartStarted = TRUE;
	}
	//Remove added content
	if(!r){
		NBString_removeLastBytes(&obj->buffer, (obj->buffer.length - iStart));
	}
	return r;
}

BOOL NBUriBuilder_addSegmentNzNc(STNBUriBuilder* obj, const char* segmentNzNc){
	BOOL r = FALSE;
	const UI32 iStart = obj->buffer.length;
	NBString_concatByte(&obj->buffer, '/');
	if(NBUriParser_concatEncoded(&obj->buffer, segmentNzNc, ENNBUriParserType_segmentNzNc)){
		r = TRUE; obj->hierPartStarted = TRUE;
	}
	//Remove added content
	if(!r){
		NBString_removeLastBytes(&obj->buffer, (obj->buffer.length - iStart));
	}
	return r;
}

//Query/fragment

BOOL NBUriBuilder_addQuery(STNBUriBuilder* obj, const char* query){
	BOOL r = FALSE;
	const UI32 iStart = obj->buffer.length;
	if(!obj->queryStarted){
		NBString_concatByte(&obj->buffer, '?');
	}
	if(NBUriParser_concatEncoded(&obj->buffer, query, ENNBUriParserType_query)){
		r = TRUE; obj->queryStarted = TRUE;
	}
	//Remove added content
	if(!r){
		NBString_removeLastBytes(&obj->buffer, (obj->buffer.length - iStart));
	}
	return r;
}

BOOL NBUriBuilder_addQueryPair(STNBUriBuilder* obj, const char* name, const char* value){
	BOOL r = FALSE;
	const UI32 iStart = obj->buffer.length;
	NBString_concatByte(&obj->buffer, (obj->queryStarted ? '&' : '?'));
	if(NBUriParser_concatEncoded(&obj->buffer, name, ENNBUriParserType_unreserved)){
		if(value != NULL){
			if(*value != '\0'){
				NBString_concatByte(&obj->buffer, '=');
				if(NBUriParser_concatEncoded(&obj->buffer, value, ENNBUriParserType_unreserved)){
					r = TRUE; obj->queryStarted = TRUE;
				}
			}
		}
	}
	//Remove added content
	if(!r){
		NBString_removeLastBytes(&obj->buffer, (obj->buffer.length - iStart));
	}
	return r;
}

BOOL NBUriBuilder_addFragment(STNBUriBuilder* obj, const char* fragment){
	BOOL r = FALSE;
	const UI32 iStart = obj->buffer.length;
	NBString_concatByte(&obj->buffer, '#');
	if(NBUriParser_concatEncoded(&obj->buffer, fragment, ENNBUriParserType_fragment)){
		r = TRUE;
	}
	//Remove added content
	if(!r){
		NBString_removeLastBytes(&obj->buffer, (obj->buffer.length - iStart));
	}
	return r;
}
