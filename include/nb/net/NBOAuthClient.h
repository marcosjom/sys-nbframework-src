
#ifndef NB_OAUTHCLIENT_H
#define NB_OAUTHCLIENT_H

#include "nb/NBFrameworkDefs.h"
#include "nb/net/NBHttpRequest.h"
#include "nb/net/NBHttpResponse.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct STNBOAuthClientId_ {
	UI32	clientId;	//clientId
	UI32	secret;		//clientSecret
} STNBOAuthClientId;

typedef struct STNBOAuthClientDst_ {
	UI32	server;		//server
	UI32	port;		//port
	BOOL	useSSL;		//SSL enabled
	UI32	redirUri;	//Redirect URI
} STNBOAuthClientDst;

typedef struct STNBOAuthClientAuth_ {
	UI32	code;		//auth code
} STNBOAuthClientAuth;

typedef struct STNBOAuthClientTkn_ {
	UI32	tokenType;
	UI32	accessToken;
	UI32	refreshToken;
} STNBOAuthClientTkn;

typedef struct STNBOAuthClient_ {
	STNBOAuthClientId	clientId;
	STNBOAuthClientDst	authServer;
	UI32				authServerCmdAuth;
	UI32				authServerCmdRevoke;
	STNBOAuthClientDst	toknServer;
	UI32				toknServerCmdToken;
	STNBOAuthClientAuth	auth;
	STNBOAuthClientTkn	token;
	//
	STNBString			strs;
	UI32				retainCount;
} STNBOAuthClient;

	
//Factory
void NBOAuthClient_init(STNBOAuthClient* obj);
void NBOAuthClient_initFrom(STNBOAuthClient* obj, const STNBOAuthClient* other);
void NBOAuthClient_retain(STNBOAuthClient* obj);
void NBOAuthClient_release(STNBOAuthClient* obj);

//Compare
BOOL NBOAuthClient_areEquivalent(const STNBOAuthClient* obj, const STNBOAuthClient* other);

//Config
void NBOAuthClient_setClient(STNBOAuthClient* obj, const char* clientId, const char* clientSecret);
void NBOAuthClient_setAuthServer(STNBOAuthClient* obj, const char* server, const UI32 port, const BOOL useSSL, const char* redirURI);
void NBOAuthClient_setAuthServerCmdAuth(STNBOAuthClient* obj, const char* cmdLogin);
void NBOAuthClient_setAuthServerCmdRevoke(STNBOAuthClient* obj, const char* cmdRevoke);
void NBOAuthClient_setTokenServer(STNBOAuthClient* obj, const char* server, const UI32 port, const BOOL useSSL, const char* redirURI);
void NBOAuthClient_setTokenServerCmdToken(STNBOAuthClient* obj, const char* cmdToken);
void NBOAuthClient_setAuth(STNBOAuthClient* obj, const char* code);
void NBOAuthClient_setToken(STNBOAuthClient* obj, const char* tokenType, const char* accessToken);
void NBOAuthClient_setTokenAndRefreshToken(STNBOAuthClient* obj, const char* tokenType, const char* accessToken, const char* refreshToken);

//Actions
BOOL NBOAuthClient_getAuthURI(STNBOAuthClient* obj, const char* scopesList, STNBString* dst);
BOOL NBOAuthClient_getToken(STNBOAuthClient* obj);
BOOL NBOAuthClient_refreshToken(STNBOAuthClient* obj);
BOOL NBOAuthClient_revokeToken(STNBOAuthClient* obj);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
