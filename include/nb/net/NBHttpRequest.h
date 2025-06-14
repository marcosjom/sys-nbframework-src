
#ifndef NB_HTTPREQUEST_H
#define NB_HTTPREQUEST_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/net/NBHttpRequest.h"
#include "nb/net/NBUrl.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef enum ENNBHttpVer_ {
		ENNBHttpVer_1_0 = 0,
		ENNBHttpVer_1_1
	} ENNBHttpVer;
	
	typedef enum ENNBHttpMethod_ {
		ENNBHttpMethod_AUTO = 0,
		ENNBHttpMethod_GET,
		ENNBHttpMethod_POST,
		ENNBHttpMethod_PUT,
		ENNBHttpMethod_DELETE
	} ENNBHttpMethod;
	
	typedef enum ENNBHttpPOSTFormat_ {
		ENNBHttpPOSTFormat_urlencoded = 0,	// var1=val1&var2=val2... (standar)
		ENNBHttpPOSTFormat_multipart,		// ----boundary val1 ----boundary val2 (better for huge size content)
	} ENNBHttpPOSTFormat;
	
	typedef enum ENNBHttpParamStorage_ {
		ENNBHttpParamStorage_Own = 0,	//Param-value-string is stored by request and must be released
		ENNBHttpParamStorage_External	//Param-value-string is stores by external identity and must not be released
	} ENNBHttpParamStorage;
	
	typedef struct STNBHttpParam_ {
		STNBString				name;
		STNBString*				value;
		ENNBHttpParamStorage	valueStorageType;
	} STNBHttpParam;
	
	typedef struct STNBHttpRequest_ {
		STNBString			protocol;		//"HTTP" by default
		UI8					verMajor;
		UI8					verMinor;
		BOOL				addImplicitParams;
		BOOL				addParamUserAgent;
		BOOL				addParamHost;
		STNBString			method;
		ENNBHttpPOSTFormat	postFormat;
		STNBArray			headers;		//STNBHttpParam
		STNBArray			paramsGET;		//STNBHttpParam
		STNBArray			paramsPOST;		//STNBHttpParam
		STNBString			contentType;	//Explicit 'Content-type'
		STNBString			content;
		//
		SI32				retainCount;
	} STNBHttpRequest;
	
	//Factory
	void NBHttpRequest_init(STNBHttpRequest* obj);
	void NBHttpRequest_initWithOther(STNBHttpRequest* obj, const STNBHttpRequest* other);
	void NBHttpRequest_retain(STNBHttpRequest* obj);
	void NBHttpRequest_release(STNBHttpRequest* obj);
	//Build
	void NBHttpRequest_setProtocol(STNBHttpRequest* obj, const char* protocol);
	void NBHttpRequest_setProtocolVer(STNBHttpRequest* obj, const ENNBHttpVer httpVer);
	void NBHttpRequest_setProtocolVerX(STNBHttpRequest* obj, const UI8 majorVer, const UI8 minorVer);
	void NBHttpRequest_setAddImplicitParams(STNBHttpRequest* obj, const BOOL addImplicitParams);
	void NBHttpRequest_setAddParamUserAgent(STNBHttpRequest* obj, const BOOL addParamUserAgent);
	void NBHttpRequest_setAddParamHost(STNBHttpRequest* obj, const BOOL addParamHost);
	void NBHttpRequest_setMethod(STNBHttpRequest* obj, const ENNBHttpMethod httpMethod);
	void NBHttpRequest_setMethodX(STNBHttpRequest* obj, const char* method);
	void NBHttpRequest_setContentType(STNBHttpRequest* obj, const char* contentType);
	void NBHttpRequest_emptyContent(STNBHttpRequest* obj);
	void NBHttpRequest_setContent(STNBHttpRequest* obj, const char* body);
	void NBHttpRequest_concatContent(STNBHttpRequest* obj, const char* bodyPart);
	void NBHttpRequest_concatContentBytes(STNBHttpRequest* obj, const char* bodyPart, const UI32 bytes);
	void NBHttpRequest_concatContentUI64(STNBHttpRequest* obj, const UI64 val);
	void NBHttpRequest_addHeader(STNBHttpRequest* obj, const char* name, const char* value);
	BOOL NBHttpRequest_headerIsPresent(const STNBHttpRequest* obj, const char* name);
	void NBHttpRequest_addParamGET(STNBHttpRequest* obj, const char* name, const char* value);
	void NBHttpRequest_addParamPOST(STNBHttpRequest* obj, const char* name, const char* value);
	void NBHttpRequest_addHeaderStr(STNBHttpRequest* obj, const char* name, STNBString* value, const ENNBHttpParamStorage valueStorageType);
	void NBHttpRequest_addParamGETStr(STNBHttpRequest* obj, const char* name, STNBString* value, const ENNBHttpParamStorage valueStorageType);
	void NBHttpRequest_addParamPOSTStr(STNBHttpRequest* obj, const char* name, STNBString* value, const ENNBHttpParamStorage valueStorageType);
	//
	void NBHttpRequest_addParamsCopy(STNBArray* dst, const STNBArray* src); //STNBHttpParam
	//Build request content
	UI32 NBHttpRequest_concatHeaderStart(const STNBHttpRequest* obj, STNBString* dst, const char* server, const char* resource, const STNBUrl* optUrlParamsAndFragments, const char* httpBoundaryTag);
	BOOL NBHttpRequest_concatBodyContinue(const STNBHttpRequest* obj, STNBString* buff, const char* httpBoundaryTag, void** dstPtrDataToSend, UI32* dstPtrDataToSendSize, const UI32 iStep);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
