#ifndef NB_URI_BUILDER_H
#define NB_URI_BUILDER_H

#include "nb/NBFrameworkDefs.h"
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
	//-----------------
	
	typedef struct STNBUriBuilder_ {
		BOOL		schemeAdded;
		BOOL		hierPartStarted;
		BOOL		queryStarted;
		STNBString	buffer;
	} STNBUriBuilder;
	
	//
	
	void NBUriBuilder_init(STNBUriBuilder* obj);
	void NBUriBuilder_release(STNBUriBuilder* obj);
	
	void NBUriBuilder_empty(STNBUriBuilder* obj);
	
	//
	
	BOOL NBUriBuilder_addScheme(STNBUriBuilder* obj, const char* scheme);
	//Hier part
	BOOL NBUriBuilder_addUserInfo(STNBUriBuilder* obj, const char* userInfo);
	BOOL NBUriBuilder_addHostName(STNBUriBuilder* obj, const char* regname);
	BOOL NBUriBuilder_addHostIPv4(STNBUriBuilder* obj, const char* ipv4);
	BOOL NBUriBuilder_addHostIPv6(STNBUriBuilder* obj, const char* ipv6);
	BOOL NBUriBuilder_addHostIPvFuture(STNBUriBuilder* obj, const char* ipvFuture);
	BOOL NBUriBuilder_addPort(STNBUriBuilder* obj, const UI32 port);
	//Path
	BOOL NBUriBuilder_addSegment(STNBUriBuilder* obj, const char* segment);
	BOOL NBUriBuilder_addSegmentNz(STNBUriBuilder* obj, const char* segmentNz);
	BOOL NBUriBuilder_addSegmentNzNc(STNBUriBuilder* obj, const char* segmentNzNc);
	//Query/fragment
	BOOL NBUriBuilder_addQuery(STNBUriBuilder* obj, const char* query);
	BOOL NBUriBuilder_addQueryPair(STNBUriBuilder* obj, const char* name, const char* value);
	BOOL NBUriBuilder_addFragment(STNBUriBuilder* obj, const char* fragment);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
