
#ifndef NB_HTTPCLIENT_H
#define NB_HTTPCLIENT_H

#include "nb/NBFrameworkDefs.h"
#include "nb/net/NBHttpRequest.h"
#include "nb/net/NBHttpResponse.h"
#include "nb/net/NBSocket.h"
#include "nb/net/NBUrl.h"
//
#include "nb/ssl/NBSsl.h"
#include "nb/ssl/NBSslContext.h"

#ifdef NB_HTTPCLIENT_USE_THREAD
#	include "nb/core/NBThread.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef enum ENNBHttpRedirMode_ {
		ENNBHttpRedirMode_None = 0,			//Run only once
		ENNBHttpRedirMode_FollowAllways,	//Follow ALL redirects automatically
	} ENNBHttpRedirMode;
	
	typedef enum ENNBHttpReqStatus_ {
		ENNBHttpReqStatus_Undef = 0,
		ENNBHttpReqStatus_Processing,
		ENNBHttpReqStatus_Success,
		ENNBHttpReqStatus_Error
	} ENNBHttpReqStatus;
	
	typedef struct STNBHttpClientQuery_ {
		ENNBHttpReqStatus	status;
		STNBString			protocol;
		STNBString			server;
		SI32				serverPort;
		STNBString			serverResource;
		BOOL				useSSL;
		//
		STNBHttpResponse	response;
	} STNBHttpClientQuery;
	
	typedef struct STNBHttpClient_ {
		BOOL				queryInited;
		STNBHttpClientQuery	query;
		//
		UI32				bytesSent;
		UI32				bytesReceived;
		//
		STNBSocketRef		socket;
		STNBString			sckServer;
		UI32				sckPort;
		BOOL				sckSSL;
		//SSL
		BOOL				sslContextMine; //created and owned by me?
		STNBSslContextRef	sslContext;
		STNBSslRef			ssl;
		//
		SI32				retainCount;
	} STNBHttpClient;
	
	//Factory
	void NBHttpClient_init(STNBHttpClient* obj);
	void NBHttpClient_retain(STNBHttpClient* obj);
	void NBHttpClient_release(STNBHttpClient* obj);
	//
	void NBHttpClientQuery_init(STNBHttpClientQuery* obj, const char* server, const SI32 serverPort, const char* serverResource, const BOOL useSSL);
	void NBHttpClientQuery_release(STNBHttpClientQuery* obj);
	
	//Execute Http protocol
	STNBHttpResponse* NBHttpClient_executeSync(STNBHttpClient* obj, const char* server, const SI32 serverPort, const char* serverResource, const BOOL useSSL, const STNBHttpRequest* request, const STNBUrl* optUrlParamsAndFragments, const ENNBHttpRedirMode execMode);
	/*#ifdef NB_HTTPCLIENT_USE_THREAD
	 BOOL NBHttpClient_executeAsync(STNBHttpClient* obj, const char* server, const SI32 serverPort, const char* serverResource, const BOOL useSSL, STNBHttpRequest* request);
	 #endif*/
	
	//Execute Http protocol in stages
	BOOL NBHttpClient_execStart(STNBHttpClient* obj, const char* server, const SI32 serverPort, const char* serverResource, const BOOL useSSL);
	BOOL NBHttpClient_execSendHeader(STNBHttpClient* obj, const STNBHttpRequest* request, const STNBUrl* optUrlParamsAndFragments);
	BOOL NBHttpClient_execSendData(STNBHttpClient* obj, const char* data, const UI32 dataSz);
	STNBHttpResponse* NBHttpClient_execRcvResponse(STNBHttpClient* obj);
	
	//
	void NBHttpClient_clear(STNBHttpClient* obj);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
