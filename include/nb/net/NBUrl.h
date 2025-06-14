//
//  NBArray.h
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#ifndef NB_URL_H
#define NB_URL_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBUrlPair_ {
		SI32	iName;
		SI32	iValue;
	} STNBUrlPair;
	
	//STNBArray
	typedef struct STNBUrl_ {
		//Example: https://user:mypass@www.nibsa.com.ni:443/path/res.html;p=v&p=v?q=v&q=v#f=v&f=v
		UI16		iScheme;	//'https'
		UI16		iUser;		//'user'
		UI16		iPasswd;	//'mypass'
		UI16		iHost;		//'www.nibsa.com.ni'
		UI16		iPort;		//'443'
		UI16		iPath;		//'/path/res.html'
		STNBArray	params;		//[{'p', 'v', 'p', 'v'}]
		STNBArray	queries;	//[{'q', 'v', 'q', 'v'}]
		STNBArray	fragments;	//[{'f', 'v', 'f', 'v'}]
		STNBString	str;
	} STNBUrl;
	
	void NBUrl_init(STNBUrl* obj);
	void NBUrl_release(STNBUrl* obj);
	//Parse
	BOOL NBUrl_parse(STNBUrl* obj, const char* url);
	//Get
	const char* NBUrl_getScheme(const STNBUrl* obj, const char* defValue);
	const char* NBUrl_getUser(const STNBUrl* obj, const char* defValue);
	const char* NBUrl_getPasswd(const STNBUrl* obj, const char* defValue);
	const char* NBUrl_getHost(const STNBUrl* obj, const char* defValue);
	const char* NBUrl_getPort(const STNBUrl* obj, const char* defValue);
	const char* NBUrl_getPath(const STNBUrl* obj, const char* defValue);
	const char* NBUrl_getParamValue(const STNBUrl* obj, const char* name, const char* defValue);
	const char* NBUrl_getQueryValue(const STNBUrl* obj, const char* name, const char* defValue);
	const char* NBUrl_getFragmentValue(const STNBUrl* obj, const char* name, const char* defValue);
	//
	BOOL NBUrl_getParamAtIndex(const STNBUrl* obj, const SI32 idx, const char** dstName, const char** dstValue);
	BOOL NBUrl_getQueryAtIndex(const STNBUrl* obj, const SI32 idx, const char** dstName, const char** dstValue);
	//Encode
	UI32 NBUrl_encodedLen(const char* plain);
	UI32 NBUrl_encodedBytesLen(const char* plain, const UI32 plainSz);
	void NBUrl_concatEncoded(STNBString* dst, const char* plain);
	void NBUrl_concatEncodedBytes(STNBString* dst, const char* plain, const UI32 plainSz);
	//Decode
	BOOL NBUrl_concatDecoded(STNBString* dst, const char* encoded);
	BOOL NBUrl_concatDecodedBytes(STNBString* dst, const char* encoded, const UI32 encodedSz);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
