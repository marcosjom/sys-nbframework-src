
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBHttpClient.h"
#include "nb/net/NBSocket.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBNumParser.h"

//Factory
void NBHttpClient_init(STNBHttpClient* obj){
	NBMemory_setZeroSt(*obj, STNBHttpClient);
	obj->queryInited	= FALSE;
	//obj->query;
	obj->bytesSent		= 0;
	obj->bytesReceived	= 0;
	//
	obj->socket = NBSocket_alloc(NULL);
	NBSocket_createHnd(obj->socket);
	NBSocket_setNoSIGPIPE(obj->socket, TRUE);
	NBSocket_setCorkEnabled(obj->socket, FALSE);
	NBSocket_setDelayEnabled(obj->socket, FALSE);
    NBString_init(&obj->sckServer);
	obj->sckPort		= 0;
	obj->sckSSL			= FALSE;
	//
	obj->retainCount	= 1;
}

void NBHttpClient_retain(STNBHttpClient* obj){
	obj->retainCount++;
}

void NBHttpClient_release(STNBHttpClient* obj){
    NBASSERT(obj->retainCount > 0)
	obj->retainCount--;
	if(obj->retainCount == 0){
		if(obj->queryInited){
		    NBHttpClientQuery_release(&obj->query);
			obj->queryInited	= FALSE;
		}
		//obj->query;
		obj->bytesSent		= 0;
		obj->bytesReceived	= 0;
		//Disconnect
		NBSocket_shutdown(obj->socket, NB_IO_BITS_RDWR);
		NBSocket_close(obj->socket);
		//Release
		{
			if(NBSsl_isSet(obj->ssl)){
				NBSsl_shutdown(obj->ssl);
				NBSsl_release(&obj->ssl);
				NBSsl_null(&obj->ssl);
			}
			if(NBSslContext_isSet(obj->sslContext)){
				if(obj->sslContextMine){
					NBSslContext_release(&obj->sslContext);
				}
				NBSslContext_null(&obj->sslContext);
			}
			NBSocket_shutdown(obj->socket, NB_IO_BITS_RDWR);
			NBSocket_release(&obj->socket);
			NBSocket_null(&obj->socket);
		}
		//
	    NBString_release(&obj->sckServer);
		obj->sckPort		= 0;
		obj->sckSSL			= FALSE;
	}
}

UI32 NBHttpClient_receiveBytes_(STNBHttpClient* obj, char* buff, int buffSz){
	UI32 r = 0;
	if(NBSsl_isSet(obj->ssl)){
        const SI32 rr = NBSsl_read(obj->ssl, buff, buffSz);
        r = (rr >= 0 ? rr : 0);
	} else {
		const SI32 rr = NBSocket_recv(obj->socket, buff, buffSz); 
		r = (rr >= 0 ? rr : 0);
	}
	return r;
}

BOOL NBHttpClient_sendBytes_(STNBHttpClient* obj, const char* data, int dataSz){
	BOOL r = FALSE;
	if(NBSsl_isSet(obj->ssl)){
        const SI32 rr = NBSsl_write(obj->ssl, data, dataSz);
        r = (rr == dataSz);
	} else {
		const SI32 rr = NBSocket_send(obj->socket, data, dataSz);
		r = (rr == dataSz);
	}
	return r;
}

void NBHttpClientQuery_init(STNBHttpClientQuery* obj, const char* server, const SI32 serverPort, const char* serverResource, const BOOL useSSL){
	obj->status		= ENNBHttpReqStatus_Undef;
	NBString_init(&obj->protocol);
    NBString_init(&obj->server);
    NBString_concat(&obj->server, server);
	obj->serverPort	= serverPort;
	obj->useSSL = useSSL;
    NBString_init(&obj->serverResource);
    NBString_concat(&obj->serverResource, serverResource);
    NBHttpResponse_init(&obj->response);
}

void NBHttpClientQuery_release(STNBHttpClientQuery* obj){
    NBHttpResponse_release(&obj->response);
	//
	NBString_release(&obj->protocol);
    NBString_release(&obj->server);
    NBString_release(&obj->serverResource);
	obj->serverPort	= 0;
}

//Execute Http protocol

/*#if defined(WIN32) || defined(_WIN32)
DWORD WINAPI NBHttpClient_execute(LPVOID param)
#else
void *NBHttpClient_execute(void* param)
#endif
{
	BOOL transfSucess = FALSE;
	//
	STNBHttpClient* client; 
	STNBHttpClientQuery* cltQuery;
	STNBHttpRequest* request;	
	STNBHttpResponse* response;	
	//
	client	= (STNBHttpClient*)param; NBASSERT(client != NULL)
	data	= &obj->query;	    NBASSERT(data != NULL)
	request	= data->request;	    NBASSERT(request != NULL)
	response= &data->response;	    NBASSERT(response != NULL)
	//
	data->status = ENNBHttpReqStatus_Processing;
	//
	BOOL connected = FALSE;
	//PRINTF_INFO("Connecting to '%s':%d (ssl: %s).\n", data->server.str, data->serverPort, (data->useSSL ? "yes" : "no"));
	if(NBString_strIsEqual(obj->sckServer.str, data->server.str) && obj->sckPort == data->serverPort && obj->sckSSL == data->useSSL){
		connected = TRUE;
	} else {
 		NBSocket_shutdown(obj->socket, NB_IO_BITS_RDWR);
	    NBSocket_release(&obj->socket);
		NBSocket_null(&obj->socket);
		obj->socket = NBSocket_alloc(NULL);
 		NBSocket_createHnd(obj->socket);
 		NBSocket_setNoSIGPIPE(obj->socket, TRUE);
 		NBSocket_setCorkEnabled(obj->socket, FALSE);
 		NBSocket_setDelayEnabled(obj->socket, FALSE);
		if(!NBSocket_connect(obj->socket, data->server.str, data->serverPort)){
			PRINTF_ERROR("Couldnt connect to '%s':%d (ssl: %s), errorno(%d) error: '%s'.\n", data->server.str, data->serverPort, (data->useSSL ? "yes" : "no"), obj->socket.errorno, obj->socket.errorstr);
		} else {
		    NBString_empty(&obj->sckServer);
		    NBString_concat(&obj->sckServer, data->server.str);
			obj->sckPort		= data->serverPort;
			obj->sckSSL		= data->useSSL;
			connected			= TRUE;
		}
	}
	//
	if(connected){
		const char* httpBoundary = "aaabbbcccdddMyDBSync";
		SI32 httpBoundaryLen = NBString_strLenBytes(httpBoundary);
		SI32 bodyBytesCount = 0;
		STNBString strREQUEST;
		//PRINTF_INFO("Connected to '%s':%d (ssl: %s).\n", data->server.str, data->serverPort, (data->useSSL ? "yes" : "no"));
	    NBString_init(&strREQUEST);
		//Calcular body
		if(request->content.length > 0){
			bodyBytesCount = request->content.length;
		} else if(request->paramsPOST.use > 0){
			if(request->postFormat == ENNBHttpPOSTFormat_urlencoded){
				SI32 i; const SI32 conteo = request->paramsPOST.use;
				for(i = 0; i < conteo; i++){
					const STNBHttpParam* param = (const STNBHttpParam*)NBArray_itemAtIndex(&request->paramsPOST, i);
					if(i != 0) bodyBytesCount++; //"&"
					bodyBytesCount += param->name.length;
					bodyBytesCount++; //"="
					const char* valStr = param->value->str;
					SI32 i; for(i = 0; i < param->value->length; i++){
						switch(valStr[i]){
							case ':': bodyBytesCount += 3; break; //":" %3A
							case ' ': bodyBytesCount += 3; break; //" " %20
							case '/': bodyBytesCount += 3; break; //"/" %2F
							case '&': bodyBytesCount += 3; break; //"&" %26
							case '=': bodyBytesCount += 3; break; //"&" %3D
							case '\n': bodyBytesCount += 3; break; //"\n" %0A
							default: bodyBytesCount++; break;
						}
					}
				}
			} else {
			    NBASSERT(request->postFormat == ENNBHttpPOSTFormat_multipart)
				//POST params
				SI32 i; const SI32 conteo = request->paramsPOST.use;
				for(i = 0; i < conteo; i++){
					const STNBHttpParam* param = (const STNBHttpParam*)NBArray_itemAtIndex(&request->paramsPOST, i);
					bodyBytesCount += 2 + httpBoundaryLen + 2; //--[boundary]\r\n
					bodyBytesCount += NBString_strLenBytes("Content-Disposition: form-data; name=\"");
					bodyBytesCount += param->name.length;
					bodyBytesCount += NBString_strLenBytes("\"\r\n\r\n");
					bodyBytesCount += param->value->length;
					bodyBytesCount += 2; //\r\n
				}
				//Final boundary
				bodyBytesCount += 2 + httpBoundaryLen + 2; //--[boundary]--
			}
		}
		//Construir solicitud HTTP
		{
			ENNBHttpMethod method = request->httpMethod;
			if(method == ENNBHttpMethod_AUTO){
				method = (request->paramsPOST.use > 0 || request->content.length > 0 ? ENNBHttpMethod_POST : ENNBHttpMethod_GET);
			}
			switch (method) {
				case ENNBHttpMethod_GET: NBString_concat(&strREQUEST, "GET"); break;
				case ENNBHttpMethod_POST: NBString_concat(&strREQUEST, "POST"); break;
				case ENNBHttpMethod_PUT: NBString_concat(&strREQUEST, "PUT"); break;
				case ENNBHttpMethod_DELETE: NBString_concat(&strREQUEST, "DELETE"); break;
#				ifdef NB_CONFIG_INCLUDE_ASSERTS
				default:
					NBASSERT(FALSE); 
					break;
#				endif
			}
		    NBString_concatByte(&strREQUEST, ' '); NBString_concat(&strREQUEST, data->serverResource.str);
			//Agregar parametros GET
			{
				SI32 i; const SI32 conteo = request->paramsGET.use;
				for(i = 0; i < conteo; i++){
					const STNBHttpParam* param = (const STNBHttpParam*)NBArray_itemAtIndex(&request->paramsGET, i);
				    NBString_concatByte(&strREQUEST, (i == 0 ? '?' : '&'));
				    NBString_concat(&strREQUEST, param->name.str);
				    NBString_concatByte(&strREQUEST, '=');
					{
						const char* valStr = param->value->str;
						SI32 i; for(i = 0; i < param->value->length; i++){
							switch(valStr[i]){
								case ':': NBString_concat(&strREQUEST, "%3A"); break; //":" %3A
								case ' ': NBString_concat(&strREQUEST, "%20"); break; //" " %20
								case '/': NBString_concat(&strREQUEST, "%2F"); break; //"/" %2F
								case '&': NBString_concat(&strREQUEST, "%26"); break; //"&" %26
								case '=': NBString_concat(&strREQUEST, "%3D"); break; //"&" %3D
								case '\n': NBString_concat(&strREQUEST, "%0A"); break; //"\n" %0A
								default: NBString_concatByte(&strREQUEST, valStr[i]); break;
							}
						}
					}
				}
			}
		    NBString_concat(&strREQUEST, " HTTP/");
			switch (request->httpVersion) {
				case ENNBHttpVer_1_0: NBString_concat(&strREQUEST, "1.0"); break;
				case ENNBHttpVer_1_1: NBString_concat(&strREQUEST, "1.1"); break;
#				ifdef NB_CONFIG_INCLUDE_ASSERTS
				default:
					NBASSERT(FALSE); 
					break;
#				endif
			}
		    NBString_concat(&strREQUEST, "\r\n");
		    NBString_concat(&strREQUEST, "Host: ");
		    NBString_concat(&strREQUEST, data->server.str);
			//DBString_concat(&strREQUEST, ":");
			//DBString_concatSI32(&strREQUEST, data->serverPort);
		    NBString_concat(&strREQUEST, "\r\n");
			//Other headers
			{
				SI32 i; const SI32 conteo = request->headers.use;
				for(i = 0; i < conteo; i++){
					const STNBHttpParam* param = (const STNBHttpParam*)NBArray_itemAtIndex(&request->headers, i);
				    NBString_concat(&strREQUEST, param->name.str);
				    NBString_concatBytes(&strREQUEST, ": ", 2);
				    NBString_concat(&strREQUEST, param->value->str);
				    NBString_concatBytes(&strREQUEST, "\r\n", 2);
				}
			}
			if(request->content.length > 0){
				if(request->contentType.length == 0){
				    NBString_concat(&strREQUEST, "Content-Type: application/octet-stream\r\n");
				} else {
				    NBString_concat(&strREQUEST, "Content-Type: ");
				    NBString_concat(&strREQUEST, request->contentType.str);
				    NBString_concat(&strREQUEST, "\r\n");
				}
			    NBString_concat(&strREQUEST, "Content-Length: ");
			    NBString_concatSI32(&strREQUEST, bodyBytesCount);
			    NBString_concat(&strREQUEST, "\r\n");
			    NBASSERT(request->content.length == bodyBytesCount)
			} else if(request->paramsPOST.use > 0){
				if(request->postFormat == ENNBHttpPOSTFormat_urlencoded){
				    NBString_concat(&strREQUEST, "Content-Type: application/x-www-form-urlencoded\r\n");
				} else {
				    NBASSERT(request->postFormat == ENNBHttpPOSTFormat_multipart)
				    NBString_concat(&strREQUEST, "Content-Type: multipart/form-data; boundary=");
				    NBString_concat(&strREQUEST, httpBoundary);
				    NBString_concat(&strREQUEST, "\r\n");
				}
			    NBString_concat(&strREQUEST, "Content-Length: ");
			    NBString_concatSI32(&strREQUEST, bodyBytesCount);
			    NBString_concat(&strREQUEST, "\r\n");
			} else if(method == ENNBHttpMethod_POST){
			    NBASSERT(bodyBytesCount == 0)
				if(request->contentType.length == 0){
				    NBString_concat(&strREQUEST, "Content-Type: application/octet-stream\r\n");
				} else {
				    NBString_concat(&strREQUEST, "Content-Type: ");
				    NBString_concat(&strREQUEST, request->contentType.str);
				    NBString_concat(&strREQUEST, "\r\n");
				}
			    NBString_concat(&strREQUEST, "Content-Length: ");
			    NBString_concatSI32(&strREQUEST, bodyBytesCount);
			    NBString_concat(&strREQUEST, "\r\n");
			}
		    NBString_concat(&strREQUEST, "User-Agent: Nibsa/HttpClient\r\n");
		    NBString_concat(&strREQUEST, "\r\n");
			//PRINTF_INFO("Sending http request:\n%s\n", strREQUEST.str);
			if(NBSocket_send(obj->socket, strREQUEST.str) != strREQUEST.length){
				PRINTF_ERROR("Conectado, pero no se pudo enviar solicitud http a servidor.\n");
			} else {
				//PRINTF_INFO("ReqSent:\n------------\n%s\n------------\n", strREQUEST.str);
				char buff[1024];
				//Send POST data
				//STNBString dbgStrPOST;
				//DBString_init(&dbgStrPOST, 1024, 1024);
				if(request->content.length > 0){
				    NBHttpClient_sendBytes_(obj, request->content.str, request->content.length); //DBString_concatBytes(&dbgStrPOST, request->content.str, request->content.length);
				    NBASSERT(request->content.length == bodyBytesCount);
				} else if(request->paramsPOST.use > 0){
					//POST params
					if(request->paramsPOST.use > 0){
						if(request->postFormat == ENNBHttpPOSTFormat_urlencoded){
							STNBString strBuff;
						    NBString_init(&strBuff);
							SI32 i; const SI32 conteo = request->paramsPOST.use;
							for(i = 0; i < conteo; i++){
								const STNBHttpParam* param = (const STNBHttpParam*)NBArray_itemAtIndex(&request->paramsPOST, i);
							    NBString_empty(&strBuff);
								if(i != 0) NBString_concatByte(&strBuff, '&');
							    NBString_concat(&strBuff, param->name.str);
							    NBString_concatByte(&strBuff, '=');
								const char* valStr = param->value->str;
								SI32 i; for(i = 0; i < param->value->length; i++){
									switch(valStr[i]){
										case ':': NBString_concatBytes(&strBuff, "%3A", 3); break; //":" %3A
										case ' ': NBString_concatBytes(&strBuff, "%20", 3); break; //" " %20
										case '/': NBString_concatBytes(&strBuff, "%2F", 3); break; //"/" %2F
										case '&': NBString_concatBytes(&strBuff, "%26", 3); break; //"&" %26
										case '=': NBString_concatBytes(&strBuff, "%3D", 3); break; //"=" %3D
										case '\n': NBString_concatBytes(&strBuff, "%0A", 3); break; //"\n" %0A
										default: NBString_concatByte(&strBuff, valStr[i]); break;
									}
								}
								//Send param
							    NBHttpClient_sendBytes_(obj, strBuff.str, strBuff.length); //DBString_concatBytes(&dbgStrPOST, strBuff.str, strBuff.length);
							}
						    NBString_release(&strBuff);
						} else {
						    NBASSERT(request->postFormat == ENNBHttpPOSTFormat_multipart)
							SI32 i; const SI32 conteo = request->paramsPOST.use;
							for(i = 0; i < conteo; i++){
								const STNBHttpParam* param = (const STNBHttpParam*)NBArray_itemAtIndex(&request->paramsPOST, i);
							    NBHttpClient_sendBytes_(obj, "--", 2); //DBString_concatBytes(&dbgStrPOST, "--", 2);
							    NBHttpClient_sendBytes_(obj, httpBoundary, httpBoundaryLen); //DBString_concatBytes(&dbgStrPOST, httpBoundary, httpBoundaryLen);
							    NBHttpClient_sendBytes_(obj, "\r\n", 2); //DBString_concatBytes(&dbgStrPOST, "\r\n", 2);
								//
							    NBHttpClient_sendBytes_(obj, "Content-Disposition: form-data; name=\"", NBString_strLenBytes("Content-Disposition: form-data; name=\"")); //DBString_concatBytes(&dbgStrPOST, "Content-Disposition: form-data; name=\"", NBString_strLenBytes("Content-Disposition: form-data; name=\""));
							    NBHttpClient_sendBytes_(obj, param->name.str, param->name.length); //DBString_concatBytes(&dbgStrPOST, param->name.str, param->name.length);
							    NBHttpClient_sendBytes_(obj, "\"\r\n\r\n", 5); //DBString_concatBytes(&dbgStrPOST, "\"\r\n\r\n", 5);
							    NBHttpClient_sendBytes_(obj, param->value->str, param->value->length); //DBString_concatBytes(&dbgStrPOST, param->value->str, param->value->length);
							    NBHttpClient_sendBytes_(obj, "\r\n", 2); //DBString_concatBytes(&dbgStrPOST, "\r\n", 2);
							}
							//Final boundary
						    NBHttpClient_sendBytes_(obj, "--", 2); //DBString_concatBytes(&dbgStrPOST, "--", 2);
						    NBHttpClient_sendBytes_(obj, httpBoundary, httpBoundaryLen); //DBString_concatBytes(&dbgStrPOST, httpBoundary, httpBoundaryLen);
						    NBHttpClient_sendBytes_(obj, "--", 2); //DBString_concatBytes(&dbgStrPOST, "--", 2);
						}
					}
					//NBASSERT(dbgStrPOST.length == bodyBytesCount);
					//DBSocket_sendBytes(obj->socket, dbgStrPOST.str, dbgStrPOST.length);
				}
				//NBASSERT(dbgStrPOST.length == bodyBytesCount)
				//if(dbgStrPOST.length > 0){
				//	PRINTF_INFO("POSTSent:\n------------\n%s\n------------\n", dbgStrPOST.str);
				//}
				//DBString_release(&dbgStrPOST);
				//Receive response
				obj->bytesSent += strREQUEST.length;
				//PRINTF_INFO("Solicitud http enviada (%d bytes), esperando respuesta...\n", strREQUEST->tamano());
				do {
					const SI32 rcvd = NBHttpClient_receiveBytes_(obj, buff, 1024);
					if(rcvd <= 0){
						PRINTF_WARNING("Stream closed by server.\n");
						break;
					} else {
						obj->bytesReceived += (UI32)rcvd;
						if(!NBHttpResponse_consumeBytes(response, buff, rcvd)){
							break;
						} else if(response->_transfEnded){
							break;
						}
					}
				} while(1);
				//
				/ *PRINTF_INFO("Response HTTP: %d (%f)\n", response->code, response->httpVersion);
				PRINTF_INFO("\n-------Header-------:\n%s\n", response->header.str);
				PRINTF_INFO("\n-------Content-------:\n%s\n", response->body.str);* /
				if(!response->_transfEnded){
					/ *if(response->code != 200){
						PRINTF_ERROR("Response HTTP: %d (%f)\n", response->code, response->httpVersion);
						PRINTF_ERROR("\n-------Header-------:\n%s\n", response->header.str);
						PRINTF_ERROR("\n-------Content-------:\n%s\n", response->body.str);
					}* /
				} else {
					/ *if(response->code != 200){
						PRINTF_INFO("Response HTTP: %d (%f)\n", response->code, response->httpVersion);
						PRINTF_INFO("\n-------Header-------:\n%s\n", response->header.str);
						PRINTF_INFO("\n-------Content-------:\n%s\n", response->body.str);
					}* /
					transfSucess = TRUE;
				}
			}
		}
		//
	    NBString_release(&strREQUEST);
	}
	//
	data->status = (transfSucess ? ENNBHttpReqStatus_Success : ENNBHttpReqStatus_Error);
	//
    NBHttpClient_release(client); //Release myself
	//
	return 0;
}*/

STNBHttpResponse* NBHttpClient_executeSyncRetry(STNBHttpClient* obj, const char* server, const SI32 serverPort, const char* serverResource, const BOOL useSSL, const STNBHttpRequest* request, const STNBUrl* optUrlParamsAndFragments, const ENNBHttpRedirMode execMode, const UI32 retryCount){
	STNBHttpResponse* r = NULL;
	if(!NBHttpClient_execStart(obj, server, serverPort, serverResource, useSSL)){
		//
	} else if(!NBHttpClient_execSendHeader(obj, request, optUrlParamsAndFragments)){
		//Retry
		if(retryCount == 0){
			PRINTF_WARNING("NBHttpClient failed at 'execSendHeader', retrying.\n");
			r = NBHttpClient_executeSyncRetry(obj, server, serverPort, serverResource, useSSL, request, optUrlParamsAndFragments, execMode, (retryCount + 1));
		}
	} else {
		r = NBHttpClient_execRcvResponse(obj);
		//Retry
		if(retryCount == 0 && obj->queryInited){
			if(obj->query.status == ENNBHttpReqStatus_Error){
				PRINTF_WARNING("NBHttpClient failed at 'execRcvResponse', retrying.\n");
				r = NBHttpClient_executeSyncRetry(obj, server, serverPort, serverResource, useSSL, request, optUrlParamsAndFragments, execMode, (retryCount + 1));
			}
		}
		//Redirect
		if(r != NULL){
			//301 Moved Permanently
			//302 Found
			//307 Temporary Redirect (since HTTP/1.1)
			//308 Permanent Redirect
			if(r->code == 301 || r->code == 302 || r->code == 307 || r->code == 308){
				if(execMode == ENNBHttpRedirMode_FollowAllways){
					const char* loc = NBHttpResponse_headerValue(r, "Location", NULL);
					if(loc == NULL){
						PRINTF_ERROR("NBHttpClient returned redirect code(%d) but no location was provided.\n", r->code);
					} else {
						//PRINTF_INFO("NBHttpClient redirect code(%d) to location: '%s'.\n", r->code, loc);
						STNBUrl url;
						NBUrl_init(&url);
						if(NBUrl_parse(&url, loc)){
							const char* scheme = &url.str.str[url.iScheme];
							const BOOL isHttp = (NBString_strIsEqual(scheme, "http") || NBString_strIsEqual(scheme, "HTTP") || NBString_strIsEqual(scheme, "Http")) ? TRUE : FALSE;
							const BOOL isHttps = (NBString_strIsEqual(scheme, "https") || NBString_strIsEqual(scheme, "HTTPS") || NBString_strIsEqual(scheme, "Https")) ? TRUE : FALSE;
							//UI32 port = 0;
							//UI32 NBNumParser_toUI32(const char* str, BOOL* dstSuccess)
							const char* host = &url.str.str[url.iHost];
							const char* port = &url.str.str[url.iPort];
							const char* path = &url.str.str[url.iPath]; if(path[0] == '\0') path = "/";
							if(!isHttp && !isHttps){
								PRINTF_ERROR("NBHttpClient redirect code(%d) to location scheme is not soported: '%s'.\n", r->code, loc);
							} else {
								BOOL portOk = TRUE;
								UI32 portI = 0;
								if(port[0] != '\0'){
									portI = NBNumParser_toUI32(port, &portOk);
								} else {
									portI = (isHttps ? 443 : 80);
								}
								if(!portOk){
									PRINTF_ERROR("NBHttpClient redirect code(%d) to location port is not soported: '%s'.\n", r->code, loc);
								} else {
									//PRINTF_INFO("NBHttpClient redirecting code(%d) to location: '%s'.\n", r->code, loc);
									r = NBHttpClient_executeSyncRetry(obj, host, (const SI32)portI, path, isHttps, request, &url, execMode, 0);
								}
							}
						}
						NBUrl_release(&url);
					}
				}
			}
		}
	}
	return r;
}

STNBHttpResponse* NBHttpClient_executeSync(STNBHttpClient* obj, const char* server, const SI32 serverPort, const char* serverResource, const BOOL useSSL, const STNBHttpRequest* request, const STNBUrl* optUrlParamsAndFragments, const ENNBHttpRedirMode execMode){
	return NBHttpClient_executeSyncRetry(obj, server, serverPort, serverResource, useSSL, request, optUrlParamsAndFragments, execMode, 0 /*retryCount*/);
}

//Execute Http protocol in stages

BOOL NBHttpClient_execStart(STNBHttpClient* obj, const char* server, const SI32 serverPort, const char* serverResource, const BOOL useSSL){
	BOOL r = FALSE;
	//It is posible to init a new connection?
	BOOL connAllowed = TRUE;
	if(obj->queryInited){
		if(obj->query.status == ENNBHttpReqStatus_Processing){
			connAllowed = FALSE; NBASSERT(FALSE);
		}
	}
	//Execute
	if(connAllowed){
		if(server != NULL && serverPort > 0 && serverResource != NULL){
			if(server[0] != '\0' && serverResource[0] != '\0'){
				{
					if(obj->queryInited){
					    NBHttpClientQuery_release(&obj->query);
						obj->queryInited = FALSE;
					}
				    NBHttpClientQuery_init(&obj->query, server, serverPort, serverResource, useSSL);
					obj->queryInited = TRUE;
				}
				//
				obj->query.status = ENNBHttpReqStatus_Processing;
			    NBHttpClient_retain(obj); //Retain myself
				//
				r = TRUE;
			}
		}
	}
	return r;
}

BOOL NBHttpClient_execSendHeader(STNBHttpClient* obj, const STNBHttpRequest* request, const STNBUrl* optUrlParamsAndFragments){
	BOOL r = FALSE;
	if(obj->queryInited){
		if(obj->query.status == ENNBHttpReqStatus_Processing){
			//
			BOOL connected	= FALSE;
			STNBHttpClientQuery* cltQuery	= &obj->query; NBASSERT(cltQuery != NULL)
			IF_NBASSERT(STNBHttpResponse* response = &cltQuery->response;) NBASSERT(response != NULL)
			//
			cltQuery->status	= ENNBHttpReqStatus_Processing;
			NBString_set(&obj->query.protocol, request->protocol.str);
			//
			//PRINTF_INFO("Connecting to '%s':%d (ssl: %s).\n", cltQuery->server.str, cltQuery->serverPort, (cltQuery->useSSL ? "yes" : "no"));
			if(NBString_strIsEqual(obj->sckServer.str, cltQuery->server.str) && obj->sckPort == cltQuery->serverPort && obj->sckSSL == cltQuery->useSSL){
				connected = TRUE;
			} else {
				//Disconnect
				NBSocket_shutdown(obj->socket, NB_IO_BITS_RDWR);
				NBSocket_close(obj->socket);
				//Reset SSL
				{
					if(NBSsl_isSet(obj->ssl)){
						NBSsl_shutdown(obj->ssl);
						NBSsl_release(&obj->ssl);
						NBSsl_null(&obj->ssl);
					}
					if(NBSslContext_isSet(obj->sslContext)){
						if(obj->sslContextMine){
							NBSslContext_release(&obj->sslContext);
						}
						NBSslContext_null(&obj->sslContext);
					}
				}
				//Reset socket
			    NBSocket_shutdown(obj->socket, NB_IO_BITS_RDWR);
			    NBSocket_release(&obj->socket);
				NBSocket_null(&obj->socket);
				//
				obj->socket = NBSocket_alloc(NULL);
				NBSocket_createHnd(obj->socket);
				NBSocket_setNoSIGPIPE(obj->socket, TRUE);
				NBSocket_setCorkEnabled(obj->socket, FALSE);
				NBSocket_setDelayEnabled(obj->socket, FALSE);
			    NBString_empty(&obj->sckServer);
				obj->sckPort	= 0;
				obj->sckSSL		= 0;
				//Connect
				if(!NBSocket_connect(obj->socket, cltQuery->server.str, cltQuery->serverPort)){
					PRINTF_ERROR("Couldnt connect to '%s':%d (ssl: %s).\n", cltQuery->server.str, cltQuery->serverPort, (cltQuery->useSSL ? "yes" : "no"));
				} else {
					connected = TRUE;
					//Reset SSL
					{
						if(NBSsl_isSet(obj->ssl)){
							NBSsl_shutdown(obj->ssl);
							NBSsl_release(&obj->ssl);
							NBSsl_null(&obj->ssl);
						}
						if(NBSslContext_isSet(obj->sslContext)){
							if(obj->sslContextMine){
								NBSslContext_release(&obj->sslContext);
							}
							NBSslContext_null(&obj->sslContext);
						}
					}
					//Init SSL
					if(cltQuery->useSSL){
						obj->sslContextMine	= TRUE;
						obj->sslContext		= NBSslContext_alloc(NULL);
						if(!NBSslContext_create(obj->sslContext, NBSslContext_getClientMode)){
							PRINTF_ERROR("Couldnt create SSLConrtext for '%s':%d (ssl: %s).\n", cltQuery->server.str, cltQuery->serverPort, (cltQuery->useSSL ? "yes" : "no"));
							NBSslContext_release(&obj->sslContext);
							NBSslContext_null(&obj->sslContext);
							connected = FALSE;
						} else {
							obj->ssl = NBSsl_alloc(NULL);
							if(!NBSsl_connect(obj->ssl, obj->sslContext, obj->socket)){
								PRINTF_ERROR("Couldnt SSL to '%s':%d (ssl: %s).\n", cltQuery->server.str, cltQuery->serverPort, (cltQuery->useSSL ? "yes" : "no"));
								connected = FALSE;
							}
						}
					}
					if(connected){
						NBString_concat(&obj->sckServer, cltQuery->server.str);
						obj->sckPort	= cltQuery->serverPort;
						obj->sckSSL		= cltQuery->useSSL;
					}
				}
			}
			//
			if(connected){
				const char* httpBoundTag	= "aaabbbcccdddMyDBSync";
				STNBString buff;
				//PRINTF_INFO("Connected to '%s':%d (ssl: %s).\n", data->server.str, data->serverPort, (data->useSSL ? "yes" : "no"));
			    NBString_initWithSz(&buff, 4096, 4096, 0.1f);
				//Concat http header
				NBHttpRequest_concatHeaderStart(request, &buff, cltQuery->server.str, cltQuery->serverResource.str, optUrlParamsAndFragments, httpBoundTag);
				//PRINTF_INFO("Sending http request:\n%s\n", strREQUEST.str);
				if(!NBHttpClient_sendBytes_(obj, buff.str, buff.length)){
					//PRINTF_ERROR("Conectado, pero no se pudo enviar solicitud http a servidor.\n");
				} else {
					r = TRUE;
					obj->bytesSent += buff.length;
					NBString_empty(&buff);
					//Send BODY
					{
						void* dataToSend = NULL;
						UI32 dataToSendSz = 0;
						UI32 iStep = 0;
						while(NBHttpRequest_concatBodyContinue(request, &buff, httpBoundTag, &dataToSend, &dataToSendSz, iStep++)){
							if(dataToSendSz > 0){
								if(!NBHttpClient_sendBytes_(obj, dataToSend, dataToSendSz)){
									r = FALSE;
									break;
								}
							}
							NBString_empty(&buff);
						}
					}
				}
			    NBString_release(&buff);
			}
			//Release query (if not successfuly)
			if(!r){
			    NBASSERT(obj->query.status == ENNBHttpReqStatus_Processing)
			    NBASSERT(cltQuery->status == ENNBHttpReqStatus_Processing)
				obj->query.status = ENNBHttpReqStatus_Error;
				cltQuery->status = ENNBHttpReqStatus_Error;
				//Disconnect
				NBSocket_shutdown(obj->socket, NB_IO_BITS_RDWR);
				NBSocket_close(obj->socket);
				//Reset SSL
				{
					if(NBSsl_isSet(obj->ssl)){
						NBSsl_shutdown(obj->ssl);
						NBSsl_release(&obj->ssl);
						NBSsl_null(&obj->ssl);
					}
					if(NBSslContext_isSet(obj->sslContext)){
						if(obj->sslContextMine){
							NBSslContext_release(&obj->sslContext);
						}
						NBSslContext_null(&obj->sslContext);
					}
				}
				//Reset socket
			    NBSocket_shutdown(obj->socket, NB_IO_BITS_RDWR);
			    NBSocket_release(&obj->socket);
				NBSocket_null(&obj->socket);
				//
				obj->socket = NBSocket_alloc(NULL);
				NBSocket_createHnd(obj->socket);
				NBSocket_setNoSIGPIPE(obj->socket, TRUE);
				NBSocket_setCorkEnabled(obj->socket, FALSE);
				NBSocket_setDelayEnabled(obj->socket, FALSE);
			    NBString_empty(&obj->sckServer);
				obj->sckPort	= 0;
				obj->sckSSL		= 0;
				//Release myself
			    NBHttpClient_release(obj);
			}
		}
	}
	return r;
}

BOOL NBHttpClient_execSendData(STNBHttpClient* obj, const char* data, const UI32 dataSz){
	BOOL r = FALSE;
	if(obj->queryInited){
		if(obj->query.status == ENNBHttpReqStatus_Processing){
			STNBHttpClientQuery* cltQuery = &obj->query; NBASSERT(cltQuery != NULL)
			//Print
			/*{
				STNBString dbgStr;
			    NBString_init(&dbgStr);
				SI32 i; for(i = 0; i < dataSz; i++){
					switch(data[i]) {
						case '\r': NBString_concatBytes(&dbgStr, "\\r", 2); break;
						case '\n': NBString_concatBytes(&dbgStr, "\\n", 2); break;
						case '\t': NBString_concatBytes(&dbgStr, "\\t", 2); break;
						case '\\': NBString_concatBytes(&dbgStr, "\\\\", 2); break;
						case '\0': NBString_concatBytes(&dbgStr, "\\0", 2); break;
						case '\'': NBString_concatBytes(&dbgStr, "\\\'", 2); break;
						case '\"': NBString_concatBytes(&dbgStr, "\\\"", 2); break;
						default: NBString_concatByte(&dbgStr, data[i]); break;
					}
				}
				PRINTF_INFO("Sending %d bytes: '%s'.\n", dataSz, dbgStr.str);
			    NBString_release(&dbgStr);
			}*/
			//Send
			if(NBHttpClient_sendBytes_(obj, data, dataSz)){
				obj->bytesSent += dataSz;
				r = TRUE;
			} else {
			    NBASSERT(obj->query.status == ENNBHttpReqStatus_Processing)
			    NBASSERT(cltQuery->status == ENNBHttpReqStatus_Processing)
				obj->query.status = ENNBHttpReqStatus_Error;
				cltQuery->status = ENNBHttpReqStatus_Error;
				//Disconnect
				NBSocket_shutdown(obj->socket, NB_IO_BITS_RDWR);
				NBSocket_close(obj->socket);
				//Reset SSL
				{
					if(NBSsl_isSet(obj->ssl)){
						NBSsl_shutdown(obj->ssl);
						NBSsl_release(&obj->ssl);
						NBSsl_null(&obj->ssl);
					}
					if(NBSslContext_isSet(obj->sslContext)){
						if(obj->sslContextMine){
							NBSslContext_release(&obj->sslContext);
						}
						NBSslContext_null(&obj->sslContext);
					}
				}
				//Reset socket
			    NBSocket_shutdown(obj->socket, NB_IO_BITS_RDWR);
			    NBSocket_release(&obj->socket);
				NBSocket_null(&obj->socket);
				//
				obj->socket = NBSocket_alloc(NULL);
				NBSocket_createHnd(obj->socket);
				NBSocket_setNoSIGPIPE(obj->socket, TRUE);
				NBSocket_setCorkEnabled(obj->socket, FALSE);
				NBSocket_setDelayEnabled(obj->socket, FALSE);
			    NBString_empty(&obj->sckServer);
				obj->sckPort	= 0;
				obj->sckSSL		= 0;
				//Release myself
			    NBHttpClient_release(obj);
			}
		}
	}
	return r;
}

STNBHttpResponse* NBHttpClient_execRcvResponse(STNBHttpClient* obj){
	STNBHttpResponse* r = NULL;
	if(obj->queryInited){
		if(obj->query.status == ENNBHttpReqStatus_Processing){
			STNBHttpClientQuery* cltQuery	= &obj->query; NBASSERT(cltQuery != NULL)
			STNBHttpResponse* response		= &cltQuery->response; NBASSERT(response != NULL)
			//PRINTF_INFO("Solicitud http enviada, esperando respuesta...\n");
			char buff[1024];
			do {
				const SI32 rcvd = NBHttpClient_receiveBytes_(obj, buff, 1024);
				if(rcvd <= 0){
					PRINTF_WARNING("Stream closed by server after (%d bytes received).\n", obj->bytesReceived);
					break;
				} else {
					obj->bytesReceived += (UI32)rcvd;
					if(!NBHttpResponse_consumeBytes(response, buff, rcvd, obj->query.protocol.str)){
						break;
					} else if(response->_transfEnded){
						break;
					}
				}
			} while(1);
			//
			/*PRINTF_INFO("Response HTTP: %d (%f)\n", response->code, response->httpVersion);
			PRINTF_INFO("\n-------Header-------:\n%s\n", response->header.str);
			PRINTF_INFO("\n-------Content-------:\n%s\n", response->body.str);*/
			if(!response->_transfEnded){
				/*if(response->code != 200){
				 PRINTF_ERROR("Response HTTP: %d (%f)\n", response->code, response->httpVersion);
				 PRINTF_ERROR("\n-------Header-------:\n%s\n", response->header.str);
				 PRINTF_ERROR("\n-------Content-------:\n%s\n", response->body.str);
				 }*/
			} else {
				/*if(response->code != 200){
				 PRINTF_INFO("Response HTTP: %d (%f)\n", response->code, response->httpVersion);
				 PRINTF_INFO("\n-------Header-------:\n%s\n", response->header.str);
				 PRINTF_INFO("\n-------Content-------:\n%s\n", response->body.str);
				 }*/
			}
			if(response->_transfEnded){
				cltQuery->status	= ENNBHttpReqStatus_Success;
			} else {
				cltQuery->status	= ENNBHttpReqStatus_Error;
				//Reset socket
				NBSocket_shutdown(obj->socket, NB_IO_BITS_RDWR);
			    NBSocket_release(&obj->socket);
				NBSocket_null(&obj->socket);
				//
				obj->socket = NBSocket_alloc(NULL);
				NBSocket_createHnd(obj->socket);
				NBSocket_setNoSIGPIPE(obj->socket, TRUE);
				NBSocket_setCorkEnabled(obj->socket, FALSE);
				NBSocket_setDelayEnabled(obj->socket, FALSE);
			    NBString_empty(&obj->sckServer);
				obj->sckPort	= 0;
				obj->sckSSL		= 0;
			}
			r = &cltQuery->response;
			//Release myself
		    NBHttpClient_release(obj); //Release myself
		}
	}
	return r;
}

/*#ifdef NB_HTTPCLIENT_USE_THREAD
BOOL NBHttpClient_executeAsync(STNBHttpClient* obj, const char* server, const SI32 serverPort, const char* serverResource, const BOOL useSSL, STNBHttpRequest* request){
	BOOL r = FALSE;
	//It is posible to init a new connection?
	BOOL connAllowed = TRUE;
	if(obj->queryInited){
		if(obj->query.status == ENNBHttpReqStatus_Processing){
			connAllowed = FALSE; NBASSERT(FALSE);
		}
	}
	//Iniciar conexion
	if(connAllowed){
		if(server != NULL && serverPort > 0 && serverResource != NULL && request != NULL){
			if(server[0] != '\0' && serverResource[0] != '\0'){
				if(obj->queryInited){
				    NBHttpClientQuery_release(&obj->query);
					obj->queryInited = FALSE;
				}
			    NBHttpClientQuery_init(&obj->query, server, serverPort, serverResource, useSSL, request);
				obj->queryInited = TRUE;
				obj->query.status = ENNBHttpReqStatus_Processing;
				obj->query.threadInited = TRUE;
			    NBHttpClient_retain(obj); //Retain myself
				//
				if(!NBThread_createAndExecute(&obj->query.thread, NBHttpClient_execute, obj)){
					PRINTF_ERROR("Couldnt create Threat for HttpClient async execution.\n");
				    NBHttpClient_release(obj); //Release myself
					obj->query.threadInited = FALSE;
					obj->query.status		= ENNBHttpReqStatus_Error;
					obj->queryInited		= FALSE;
				    NBHttpClientQuery_release(&obj->query);
				    NBASSERT(FALSE) //coludnt create thread
				} else {
					r = TRUE;
				}
			}
		}
	}
	return r;
}
#endif*/

void NBHttpClient_clear(STNBHttpClient* obj){
	if(obj->queryInited){
	    NBHttpClientQuery_release(&obj->query);
		obj->queryInited = FALSE;
	}
}
