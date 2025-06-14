
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBOAuthClient.h"
#include "nb/net/NBHttpClient.h"
#include "nb/core/NBJson.h"

//Factory
void NBOAuthClient_init(STNBOAuthClient* obj){
	obj->clientId.clientId		= 0;
	obj->clientId.secret		= 0;
	//
	obj->authServer.server		= 0;
	obj->authServer.port		= 0;
	obj->authServer.useSSL		= FALSE;
	obj->authServer.redirUri	= 0;
	obj->authServerCmdAuth		= 0;
	obj->authServerCmdRevoke	= 0;
	//
	obj->toknServer.server		= 0;
	obj->toknServer.port		= 0;
	obj->toknServer.useSSL		= FALSE;
	obj->toknServer.redirUri 	= 0;
	obj->toknServerCmdToken		= 0;
	//
	obj->auth.code				= 0;
	obj->token.tokenType		= 0;
	obj->token.accessToken		= 0;
	obj->token.refreshToken		= 0;
	//
    NBString_init(&obj->strs);
    NBString_concatByte(&obj->strs, '\0'); //Index zero must be empty string
	obj->retainCount	= 1;
}

void NBOAuthClient_initFrom(STNBOAuthClient* obj, const STNBOAuthClient* other){
	obj->clientId.clientId		= other->clientId.clientId;
	obj->clientId.secret		= other->clientId.secret;
	//
	obj->authServer.server		= other->authServer.server;
	obj->authServer.port		= other->authServer.port;
	obj->authServer.useSSL		= other->authServer.useSSL;
	obj->authServer.redirUri	= other->authServer.redirUri;
	obj->authServerCmdAuth		= other->authServerCmdAuth;
	obj->authServerCmdRevoke	= other->authServerCmdRevoke;
	//
	obj->toknServer.server		= other->toknServer.server;
	obj->toknServer.port		= other->toknServer.port;
	obj->toknServer.useSSL		= other->toknServer.useSSL;
	obj->toknServer.redirUri	= other->toknServer.redirUri;
	obj->toknServerCmdToken		= other->toknServerCmdToken;
	//
	obj->auth.code				= other->auth.code;
	obj->token.tokenType		= other->token.tokenType;
	obj->token.accessToken		= other->token.accessToken;
	obj->token.refreshToken		= other->token.refreshToken;
	//
    NBString_initWithOther(&obj->strs, &other->strs);
	obj->retainCount	= 1;
}

void NBOAuthClient_retain(STNBOAuthClient* obj){
	//
	obj->retainCount++;
}

void NBOAuthClient_release(STNBOAuthClient* obj){
    NBASSERT(obj->retainCount > 0)
	obj->retainCount--;
	if(obj->retainCount == 0){
		obj->clientId.clientId		= 0;
		obj->clientId.secret		= 0;
		//
		obj->authServer.server		= 0;
		obj->authServer.port		= 0;
		obj->authServer.useSSL		= FALSE;
		obj->authServer.redirUri 	= 0;
		obj->authServerCmdAuth		= 0;
		obj->authServerCmdRevoke	= 0;
		//
		obj->toknServer.server		= 0;
		obj->toknServer.port		= 0;
		obj->toknServer.useSSL		= FALSE;
		obj->toknServer.redirUri 	= 0;
		obj->toknServerCmdToken		= 0;
		//
		obj->auth.code				= 0;
		obj->token.tokenType		= 0;
		obj->token.accessToken		= 0;
		obj->token.refreshToken		= 0;
	    NBString_release(&obj->strs);
	}
}

//Compare
BOOL NBOAuthClient_areEquivalent(const STNBOAuthClient* obj, const STNBOAuthClient* other){
	BOOL r = TRUE;
	if(!NBString_strIsEqual(&obj->strs.str[obj->clientId.clientId], &other->strs.str[other->clientId.clientId])){
		r = FALSE;
	} else if(!NBString_strIsEqual(&obj->strs.str[obj->clientId.secret], &other->strs.str[other->clientId.secret])){
		r = FALSE;
	} else if(!NBString_strIsEqual(&obj->strs.str[obj->authServer.server], &other->strs.str[other->authServer.server])){
		r = FALSE;
	} else if(obj->authServer.port != other->authServer.port){
		r = FALSE;
	} else if(obj->authServer.useSSL != other->authServer.useSSL){
		r = FALSE;
	} else if(obj->authServerCmdAuth != other->authServerCmdAuth || obj->authServerCmdRevoke != other->authServerCmdRevoke){
		r = FALSE;
	} else if(!NBString_strIsEqual(&obj->strs.str[obj->authServer.redirUri], &other->strs.str[other->authServer.redirUri])){
		r = FALSE;
	} else if(!NBString_strIsEqual(&obj->strs.str[obj->toknServer.server], &other->strs.str[other->toknServer.server])){
		r = FALSE;
	} else if(obj->toknServer.port != other->toknServer.port){
		r = FALSE;
	} else if(obj->toknServer.useSSL != other->toknServer.useSSL){
		r = FALSE;
	} else if(!NBString_strIsEqual(&obj->strs.str[obj->toknServer.redirUri], &other->strs.str[other->toknServer.redirUri])){
		r = FALSE;
	} else if(obj->toknServerCmdToken != other->toknServerCmdToken){
		r = FALSE;
	} else if(!NBString_strIsEqual(&obj->strs.str[obj->auth.code], &other->strs.str[other->auth.code])){
		r = FALSE;
	} else if(!NBString_strIsEqual(&obj->strs.str[obj->token.refreshToken], &other->strs.str[other->token.refreshToken])){
		r = FALSE;
	}
	return r;
}

// Config

void NBOAuthClient_setClient(STNBOAuthClient* obj, const char* clientId, const char* clientSecret){
	obj->clientId.clientId	= 0;
	obj->clientId.secret	= 0;
	if(clientId != NULL){
		if(clientId[0] != '\0'){
			obj->clientId.clientId = obj->strs.length; NBString_concat(&obj->strs, clientId); NBString_concatByte(&obj->strs, '\0');
		}
	}
	if(clientSecret != NULL){
		if(clientSecret[0] != '\0'){
			obj->clientId.secret = obj->strs.length; NBString_concat(&obj->strs, clientSecret); NBString_concatByte(&obj->strs, '\0');
		}
	}
}

void NBOAuthClient_setAuthServer(STNBOAuthClient* obj, const char* server, const UI32 port, const BOOL useSSL, const char* redirURI){
	obj->authServer.server		= 0;
	obj->authServer.port		= port;
	obj->authServer.useSSL		= useSSL;
	obj->authServer.redirUri	= 0;
	if(server != NULL){
		if(server[0] != '\0'){
			obj->authServer.server = obj->strs.length; NBString_concat(&obj->strs, server); NBString_concatByte(&obj->strs, '\0');
		}
	}
	if(redirURI != NULL){
		if(redirURI[0] != '\0'){
			obj->authServer.redirUri = obj->strs.length; NBString_concat(&obj->strs, redirURI); NBString_concatByte(&obj->strs, '\0');
		}
	}
}

void NBOAuthClient_setAuthServerCmdAuth(STNBOAuthClient* obj, const char* cmdLogin){
	obj->authServerCmdAuth = 0;
	if(cmdLogin != NULL){
		if(cmdLogin[0] != '\0'){
			obj->authServerCmdAuth = obj->strs.length; NBString_concat(&obj->strs, cmdLogin); NBString_concatByte(&obj->strs, '\0');
		}
	}
}

void NBOAuthClient_setAuthServerCmdRevoke(STNBOAuthClient* obj, const char* cmdRevoke){
	obj->authServerCmdRevoke = 0;
	if(cmdRevoke != NULL){
		if(cmdRevoke[0] != '\0'){
			obj->authServerCmdRevoke = obj->strs.length; NBString_concat(&obj->strs, cmdRevoke); NBString_concatByte(&obj->strs, '\0');
		}
	}
}

void NBOAuthClient_setTokenServer(STNBOAuthClient* obj, const char* server, const UI32 port, const BOOL useSSL, const char* redirURI){
	obj->toknServer.server		= 0;
	obj->toknServer.port		= port;
	obj->toknServer.useSSL		= useSSL;
	obj->toknServer.redirUri	= 0;
	if(server != NULL){
		if(server[0] != '\0'){
			obj->toknServer.server = obj->strs.length; NBString_concat(&obj->strs, server); NBString_concatByte(&obj->strs, '\0');
		}
	}
	if(redirURI != NULL){
		if(redirURI[0] != '\0'){
			obj->toknServer.redirUri = obj->strs.length; NBString_concat(&obj->strs, redirURI); NBString_concatByte(&obj->strs, '\0');
		}
	}
}

void NBOAuthClient_setTokenServerCmdToken(STNBOAuthClient* obj, const char* cmdToken){
	obj->toknServerCmdToken	= 0;
	if(cmdToken != NULL){
		if(cmdToken[0] != '\0'){
			obj->toknServerCmdToken = obj->strs.length; NBString_concat(&obj->strs, cmdToken); NBString_concatByte(&obj->strs, '\0');
		}
	}
}

void NBOAuthClient_setAuth(STNBOAuthClient* obj, const char* code){
	obj->auth.code			= 0;
	if(code != NULL){
		if(code[0] != '\0'){
			obj->auth.code = obj->strs.length; NBString_concat(&obj->strs, code); NBString_concatByte(&obj->strs, '\0');
		}
	}
}

void NBOAuthClient_setToken(STNBOAuthClient* obj, const char* tokenType, const char* accessToken){
	obj->token.tokenType		= 0;
	obj->token.accessToken		= 0;
	if(tokenType != NULL){
		if(tokenType[0] != '\0'){
			obj->token.tokenType = obj->strs.length; NBString_concat(&obj->strs, tokenType); NBString_concatByte(&obj->strs, '\0');
		}
	}
	if(accessToken != NULL){
		if(accessToken[0] != '\0'){
			obj->token.accessToken = obj->strs.length; NBString_concat(&obj->strs, accessToken); NBString_concatByte(&obj->strs, '\0');
		}
	}
}

void NBOAuthClient_setTokenAndRefreshToken(STNBOAuthClient* obj, const char* tokenType, const char* accessToken, const char* refreshToken){
	obj->token.tokenType		= 0;
	obj->token.accessToken		= 0;
	obj->token.refreshToken		= 0;
	if(tokenType != NULL){
		if(tokenType[0] != '\0'){
			obj->token.tokenType = obj->strs.length; NBString_concat(&obj->strs, tokenType); NBString_concatByte(&obj->strs, '\0');
		}
	}
	if(accessToken != NULL){
		if(accessToken[0] != '\0'){
			obj->token.accessToken = obj->strs.length; NBString_concat(&obj->strs, accessToken); NBString_concatByte(&obj->strs, '\0');
		}
	}
	if(refreshToken != NULL){
		if(refreshToken[0] != '\0'){
			obj->token.refreshToken = obj->strs.length; NBString_concat(&obj->strs, refreshToken); NBString_concatByte(&obj->strs, '\0');
		}
	}
}

//Actions

BOOL NBOAuthClient_getAuthURI(STNBOAuthClient* obj, const char* scopesList, STNBString* dst){
	BOOL r = FALSE;
	//Built URI
	NBASSERT(obj->authServerCmdAuth != 0)
	if(obj->authServer.server == 0 || obj->authServerCmdAuth == 0){
		PRINTF_ERROR("Missing oauth::server info.\n");
	} else if(obj->clientId.clientId == 0){
		PRINTF_ERROR("Missing oauth::client info.\n");
	} else {
		NBString_concat(dst, (obj->authServer.useSSL ? "https://" : "http://"));
		NBString_concat(dst,  &obj->strs.str[obj->authServer.server]);
		NBString_concat(dst,  ":");
		NBString_concatUI32(dst, obj->authServer.port);
		NBString_concat(dst,  &obj->strs.str[obj->authServerCmdAuth]); NBASSERT(obj->authServerCmdAuth != 0)
		NBString_concat(dst, "?response_type=code");
		NBString_concat(dst, "&access_type=offline");
		NBString_concat(dst, "&client_id="); NBString_concat(dst, &obj->strs.str[obj->clientId.clientId]);
		if(obj->authServer.redirUri != 0){
			NBString_concat(dst, "&redirect_uri=");
			const char* str = &obj->strs.str[obj->authServer.redirUri];
			while(*str != '\0'){
				switch(*str){
					case ':': NBString_concatBytes(dst, "%3A", 3); break;
					case ' ': NBString_concatBytes(dst, "%20", 3); break;
					case '/': NBString_concatBytes(dst, "%2F", 3); break;
					case '&': NBString_concatBytes(dst, "%26", 3); break;
					case '=': NBString_concatBytes(dst, "%3D", 3); break;
					case '\n': NBString_concatBytes(dst, "%0A", 3); break;
					default: NBString_concatByte(dst, *str); break;
				}
				str++;
			}
		}
		if(scopesList != NULL){
			if(scopesList[0] != '\0'){
				NBString_concat(dst, "&scope=");
				const char* str = scopesList;
				while(*str != '\0'){
					switch(*str){
						case ':': NBString_concatBytes(dst, "%3A", 3); break;
						case ' ': NBString_concatBytes(dst, "%20", 3); break;
						case '/': NBString_concatBytes(dst, "%2F", 3); break;
						case '&': NBString_concatBytes(dst, "%26", 3); break;
						case '=': NBString_concatBytes(dst, "%3D", 3); break;
						case '\n': NBString_concatBytes(dst, "%0A", 3); break;
						default: NBString_concatByte(dst, *str); break;
					}
					str++;
				}
			}
		}
		r = TRUE;
	}
	return r;
}

BOOL NBOAuthClient_getToken(STNBOAuthClient* obj){
	BOOL r = FALSE;
	NBASSERT(obj->toknServerCmdToken != 0)
	if(obj->clientId.clientId == 0 || obj->clientId.secret == 0){
		PRINTF_ERROR("Missing oauth::client info.\n");
	} else if(obj->toknServer.server == 0 || obj->toknServer.port == 0 || obj->toknServerCmdToken == 0){
		PRINTF_ERROR("Missing oauth::server info.\n");
	} else if(obj->auth.code == 0){
		PRINTF_ERROR("Missing oauth::authorization info.\n");
	} else {
		const char* resPath = &obj->strs.str[obj->toknServerCmdToken];
		STNBHttpClient client;
		NBHttpClient_init(&client);
		STNBHttpRequest req;
		NBHttpRequest_init(&req);
		NBHttpRequest_addParamPOST(&req, "client_id", &obj->strs.str[obj->clientId.clientId]);
		NBHttpRequest_addParamPOST(&req, "client_secret", &obj->strs.str[obj->clientId.secret]);
		NBHttpRequest_addParamPOST(&req, "grant_type", "authorization_code");
		NBHttpRequest_addParamPOST(&req, "code", &obj->strs.str[obj->auth.code]);
		if(obj->toknServer.redirUri != 0){
			NBHttpRequest_addParamPOST(&req, "redirect_uri", &obj->strs.str[obj->toknServer.redirUri]);
		}
		const STNBHttpResponse* resp = NBHttpClient_executeSync(&client, &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, obj->toknServer.useSSL, &req, NULL, ENNBHttpRedirMode_None);
		if(resp == NULL){
			NBASSERT(FALSE)
		} else {
			STNBJson json;
			NBJson_init(&json);
			if(resp->code != 200){
				if(resp->code == 400){
					PRINTF_ERROR("OAuth::tokenRequest returned code %d (BAD REQUEST), for '%s:%d%s' %s, unparseable %d bytes body: %s.\n", resp->code, &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, (obj->toknServer.useSSL ? "WITH_SSL" : "NO_SSL"), resp->body.length, resp->body.str);
				} else if(resp->code == 401){
					PRINTF_ERROR("OAuth::tokenRequest returned code %d (UNAUTHORIZED), for '%s:%d%s' %s, unparseable %d bytes body: %s.\n", resp->code, &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, (obj->toknServer.useSSL ? "WITH_SSL" : "NO_SSL"), resp->body.length, resp->body.str);
				} else if(resp->code == 404){
					PRINTF_ERROR("OAuth::tokenRequest returned code %d (NOT FOUND), for '%s:%d%s' %s, unparseable %d bytes body: %s.\n", resp->code, &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, (obj->toknServer.useSSL ? "WITH_SSL" : "NO_SSL"), resp->body.length, resp->body.str);
				} else {
					if(!NBJson_loadFromStr(&json, resp->body.str)){
						PRINTF_ERROR("OAuth::tokenRequest returned code %d, for '%s:%d%s' %s, unparseable %d bytes body: %s.\n", resp->code, &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, (obj->toknServer.useSSL ? "WITH_SSL" : "NO_SSL"), resp->body.length, resp->body.str);
					} else {
						const STNBJsonNode*	rootObj = NBJson_rootMember(&json);
						if(rootObj == NULL){
							PRINTF_ERROR("OAuth::tokenRequest returned code %d, for '%s:%d%s' %s, unparseable %d bytes body: %s.\n", resp->code, &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, (obj->toknServer.useSSL ? "WITH_SSL" : "NO_SSL"), resp->body.length, resp->body.str);
						} else {
							PRINTF_ERROR("OAuth::tokenRequest returned error('%s') desc('%s'), code %d, for '%s:%d%s' %s.\n", NBJson_childStr(&json, "error", "", rootObj, NULL), NBJson_childStr(&json, "error_description", "", rootObj, NULL), resp->code, &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, (obj->toknServer.useSSL ? "WITH_SSL" : "NO_SSL"));
						}
					}
				}
			} else {
				if(!NBJson_loadFromStr(&json, resp->body.str)){
					PRINTF_ERROR("OAuth::tokenRequest returned for '%s:%d%s' %s, unparseable %d bytes body: %s.\n", &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, (obj->toknServer.useSSL ? "WITH_SSL" : "NO_SSL"), resp->body.length, resp->body.str);
				} else {
					const STNBJsonNode*	rootObj		= NBJson_rootMember(&json);
					const char* access_token		= NBJson_childStr(&json, "access_token", NULL, rootObj, NULL);
					const char* token_type			= NBJson_childStr(&json, "token_type", NULL, rootObj, NULL);
					IF_PRINTF(const SI32 expires_in	= NBJson_childSI32(&json, "expires_in", 0, rootObj, NULL);)
					const char* refresh_token		= NBJson_childStr(&json, "refresh_token", NULL, rootObj, NULL);
					if(access_token == NULL){
						PRINTF_ERROR("OAuth::tokenRequest 'access_token' expected as return, returned error('%s') desc('%s'), for '%s:%d%s' %s.\n", NBJson_childStr(&json, "error", "", rootObj, NULL), NBJson_childStr(&json, "error_description", "", rootObj, NULL), &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, (obj->toknServer.useSSL ? "WITH_SSL" : "NO_SSL"));
					} else {
						PRINTF_INFO("OAuth::tokenRequest returned for '%s:%d%s' %s, access_token('%s') token_type('%s') expires_in(%d) refresh_token('%s'):\n'%s'.\n", &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, (obj->toknServer.useSSL ? "WITH_SSL" : "NO_SSL"), access_token, token_type, expires_in, refresh_token, resp->body.str);
						NBOAuthClient_setTokenAndRefreshToken(obj, token_type, access_token, refresh_token);
						r = TRUE;
					}
				}
			}
			NBJson_release(&json);
		}
		NBHttpRequest_release(&req);
		NBHttpClient_release(&client);
	}
	return r;
}

BOOL NBOAuthClient_refreshToken(STNBOAuthClient* obj){
	BOOL r = FALSE;
	NBASSERT(obj->toknServerCmdToken != 0)
	if(obj->clientId.clientId == 0 || obj->clientId.secret == 0){
		PRINTF_ERROR("Missing oauth::client info.\n");
	} else if(obj->toknServer.server == 0 || obj->toknServer.port == 0 || obj->toknServerCmdToken == 0){
		PRINTF_ERROR("Missing oauth::server info.\n");
	} else if(obj->token.refreshToken == 0){
		PRINTF_ERROR("Missing oauth::refreshToken info.\n");
	} else {
		const char* resPath = &obj->strs.str[obj->toknServerCmdToken];
		STNBHttpClient client;
		NBHttpClient_init(&client);
		STNBHttpRequest req;
		NBHttpRequest_init(&req);
		NBHttpRequest_addParamPOST(&req, "client_id", &obj->strs.str[obj->clientId.clientId]);
		NBHttpRequest_addParamPOST(&req, "client_secret", &obj->strs.str[obj->clientId.secret]);
		NBHttpRequest_addParamPOST(&req, "grant_type", "refresh_token");
		NBHttpRequest_addParamPOST(&req, "refresh_token", &obj->strs.str[obj->token.refreshToken]);
		const STNBHttpResponse* resp = NBHttpClient_executeSync(&client, &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, obj->toknServer.useSSL, &req, NULL, ENNBHttpRedirMode_None);
		if(resp == NULL){
			NBASSERT(FALSE)
		} else {
			STNBJson json;
			NBJson_init(&json);
			if(resp->code != 200){
				if(resp->code == 400){
					PRINTF_ERROR("OAuth::tokenRefresh returned code %d (BAD REQUEST), for '%s:%d%s' %s, unparseable %d bytes body: %s.\n", resp->code, &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, (obj->toknServer.useSSL ? "WITH_SSL" : "NO_SSL"), resp->body.length, resp->body.str);
				} else if(resp->code == 401){
					PRINTF_ERROR("OAuth::tokenRefresh returned code %d (UNAUTHORIZED), for '%s:%d%s' %s, unparseable %d bytes body: %s.\n", resp->code, &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, (obj->toknServer.useSSL ? "WITH_SSL" : "NO_SSL"), resp->body.length, resp->body.str);
				} else if(resp->code == 404){
					PRINTF_ERROR("OAuth::tokenRefresh returned code %d (NOT FOUND), for '%s:%d%s' %s, unparseable %d bytes body: %s.\n", resp->code, &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, (obj->toknServer.useSSL ? "WITH_SSL" : "NO_SSL"), resp->body.length, resp->body.str);
				} else {
					if(!NBJson_loadFromStr(&json, resp->body.str)){
						PRINTF_ERROR("OAuth::tokenRefresh returned code %d, for '%s:%d%s' %s, unparseable %d bytes body: %s.\n", resp->code, &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, (obj->toknServer.useSSL ? "WITH_SSL" : "NO_SSL"), resp->body.length, resp->body.str);
					} else {
						const STNBJsonNode*	rootObj		= NBJson_rootMember(&json);
						if(rootObj == NULL){
							PRINTF_ERROR("OAuth::tokenRefresh returned code %d, for '%s:%d%s' %s, unparseable %d bytes body: %s.\n", resp->code, &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, (obj->toknServer.useSSL ? "WITH_SSL" : "NO_SSL"), resp->body.length, resp->body.str);
						} else {
							PRINTF_ERROR("OAuth::tokenRefresh returned error('%s') desc('%s'), code %d, for '%s:%d%s' %s.\n", NBJson_childStr(&json, "error", "", rootObj, NULL), NBJson_childStr(&json, "error_description", "", rootObj, NULL), resp->code, &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, (obj->toknServer.useSSL ? "WITH_SSL" : "NO_SSL"));
						}
					}
				}
			} else {
				if(!NBJson_loadFromStr(&json, resp->body.str)){
					PRINTF_ERROR("OAuth::tokenRefresh returned for '%s:%d%s' %s, unparseable %d bytes body: %s.\n", &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, (obj->toknServer.useSSL ? "WITH_SSL" : "NO_SSL"), resp->body.length, resp->body.str);
				} else {
					const STNBJsonNode*	rootObj		= NBJson_rootMember(&json);
					const char* access_token		= NBJson_childStr(&json, "access_token", NULL, rootObj, NULL);
					const char* token_type			= NBJson_childStr(&json, "token_type", NULL, rootObj, NULL);
					IF_PRINTF(const SI32 expires_in	= NBJson_childSI32(&json, "expires_in", 0, rootObj, NULL);)
					if(token_type == NULL || access_token == NULL){
						PRINTF_ERROR("OAuth::tokenRefresh 'access_token' expected as return, returned error('%s') desc('%s'), for '%s:%d%s' %s.\n", NBJson_childStr(&json, "error", "", rootObj, NULL), NBJson_childStr(&json, "error_description", "", rootObj, NULL), &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, (obj->toknServer.useSSL ? "WITH_SSL" : "NO_SSL"));
					} else {
						PRINTF_INFO("OAuth::tokenRefresh returned for '%s:%d%s' %s, access_token('%s') token_type('%s') expires_in(%d).\n", &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, (obj->toknServer.useSSL ? "WITH_SSL" : "NO_SSL"), access_token, token_type, expires_in);
						NBOAuthClient_setToken(obj, token_type, access_token);
						r = TRUE;
					}
				}
			}
			NBJson_release(&json);
		}
		NBHttpRequest_release(&req);
		NBHttpClient_release(&client);
	}
	return r;
}

BOOL NBOAuthClient_revokeToken(STNBOAuthClient* obj){
	BOOL r = FALSE;
	NBASSERT(obj->authServerCmdRevoke != 0)
	if(obj->authServer.server == 0 || obj->authServer.port == 0 || obj->authServerCmdRevoke == 0){
		PRINTF_ERROR("Missing oauth::server info.\n");
	} else if(obj->token.accessToken == 0){
		r = TRUE; //Nothing to revoke
	} else {
		const char* resPath = &obj->strs.str[obj->authServerCmdRevoke];
		STNBHttpClient client;
		NBHttpClient_init(&client);
		STNBHttpRequest req;
		NBHttpRequest_init(&req);
		NBHttpRequest_addParamGET(&req, "token", &obj->strs.str[obj->token.accessToken]);
		const STNBHttpResponse* resp = NBHttpClient_executeSync(&client, &obj->strs.str[obj->authServer.server], obj->authServer.port, resPath, obj->authServer.useSSL, &req, NULL, ENNBHttpRedirMode_None);
		if(resp == NULL){
			NBASSERT(FALSE)
		} else {
			STNBJson json;
			NBJson_init(&json);
			if(resp->code != 200){
				if(resp->code == 400){
					PRINTF_ERROR("OAuth::tokenRevoke returned code %d (BAD REQUEST), for '%s:%d%s' %s, unparseable %d bytes body: %s.\n", resp->code, &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, (obj->toknServer.useSSL ? "WITH_SSL" : "NO_SSL"), resp->body.length, resp->body.str);
				} else if(resp->code == 401){
					PRINTF_ERROR("OAuth::tokenRevoke returned code %d (UNAUTHORIZED), for '%s:%d%s' %s, unparseable %d bytes body: %s.\n", resp->code, &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, (obj->toknServer.useSSL ? "WITH_SSL" : "NO_SSL"), resp->body.length, resp->body.str);
				} else if(resp->code == 404){
					PRINTF_ERROR("OAuth::tokenRevoke returned code %d (NOT FOUND), for '%s:%d%s' %s, unparseable %d bytes body: %s.\n", resp->code, &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, (obj->toknServer.useSSL ? "WITH_SSL" : "NO_SSL"), resp->body.length, resp->body.str);
				} else {
					if(!NBJson_loadFromStr(&json, resp->body.str)){
						PRINTF_ERROR("OAuth::tokenRevoke returned code %d, for '%s:%d%s' %s, unparseable %d bytes body: %s.\n", resp->code, &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, (obj->toknServer.useSSL ? "WITH_SSL" : "NO_SSL"), resp->body.length, resp->body.str);
					} else {
						const STNBJsonNode*	rootObj = NBJson_rootMember(&json);
						if(rootObj == NULL){
							PRINTF_ERROR("OAuth::tokenRevoke returned code %d, for '%s:%d%s' %s, unparseable %d bytes body: %s.\n", resp->code, &obj->strs.str[obj->toknServer.server], obj->toknServer.port, resPath, (obj->toknServer.useSSL ? "WITH_SSL" : "NO_SSL"), resp->body.length, resp->body.str);
						} else {
							PRINTF_ERROR("OAuth::tokenRevoke returned error('%s') desc('%s'), code %d, for '%s:%d%s' %s.\n", NBJson_childStr(&json, "error", "", rootObj, NULL), NBJson_childStr(&json, "error_description", "", rootObj, NULL), resp->code, &obj->strs.str[obj->authServer.server], obj->authServer.port, resPath, (obj->authServer.useSSL ? "WITH_SSL" : "NO_SSL"));
						}
					}
				}
			} else {
				r = TRUE;
			}
			NBJson_release(&json);
		}
		NBHttpRequest_release(&req);
		NBHttpClient_release(&client);
	}
	return r;
}

