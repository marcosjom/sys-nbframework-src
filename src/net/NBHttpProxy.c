
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBHttpProxy.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBNumParser.h"
#include "nb/net/NBSocket.h"
#include "nb/ssl/NBSsl.h"
#include "nb/core/NBThread.h"
#include "nb/net/NBHttpBuilder.h"

//
/*#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBThread.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBThreadCond.h"
#include "nb/net/NBSocket.h"
#include "nb/net/NBHttpHeader.h"
#include "nb/net/NBHttpBody.h"
#include "nb/net/NBHttpMessage.h"
#include "nb/net/NBHttpBuilder.h"*/

/*struct STNBHttpProxyOpaque_;

typedef struct STNBHttpProxyClt_ {
	struct STNBHttpProxyOpaque_* opq;
	const STNBHttpServiceConn* cltRef;
	//Destination
	STNBSocketRef	dstSocket;
	STNBString		dstServer;
	UI32			dstPort;
	//Service
	const IHttpServicePortSender* sendrItf;
	void*			sendrParam;
	STNBString		tmpStr;
	//Current response
	UI32			respCode;
	STNBString		respReason;
	BOOL			internalErr;	//Proxy internal error (program logic or parameters)
	BOOL			connOrigErr;	//Origin closed the connection (or error)
	BOOL			connDstErr;		//Destination close the connection (or error)
	BOOL			connClose;		//End client connection after the request
	//Request/response properties
	BOOL			reqIsTunnel;	//is a Tunnel request
	BOOL			reqIsChunkd;	//origin request is chunked
	BOOL			respIsChunkd;	//remote response is chunked
	BOOL			isTunneling;	//currently tunneling (working)
} STNBHttpProxyClt;

typedef struct STNBHttpProxyOpaque_ {
	STNBHttpProxyCfg		cfg;
	STNBHttpServicePortRef	service;
	UI32					netBuffSize;	//Read net-buffer size
	UI32 					httpBuffSz;		//Read http-buffer size (zero if data must be ignored)
	UI32 					storgBuffSz;	//Storage body-buffer sizes (zero if must not be stored, only passed to listener)
} STNBHttpProxyOpaque;

//HttpService interface

BOOL NBHttpProxy_cltConnected_(STNBHttpServicePortRef ref, const STNBHttpServiceConn* cltRef, IHttpMessageListener* dstMsgListener, void** dstMsgListenerParam, void* lparam);
BOOL NBHttpProxy_cltDisconnected_(STNBHttpServicePortRef ref, void* msgListenerParam, void* lparam);
void NBHttpProxy_cltWillDisconnect_(STNBHttpServicePortRef ref, void* msgListenerParam, void* lparam);
BOOL NBHttpProxy_cltRequested_(STNBHttpServicePortRef ref, const STNBHttpMessage* msg, const IHttpServicePortSender* sendrItf, void* sendrParam, void* msgListenerParam, void* lparam);

//

void NBHttpProxy_init(STNBHttpProxy* obj){
	STNBHttpProxyOpaque* opq = obj->opaque = NBMemory_allocType(STNBHttpProxyOpaque);
	NBMemory_setZeroSt(*opq, STNBHttpProxyOpaque);
	//
	NBMemory_set(&opq->cfg, 0, sizeof(opq->cfg));
	opq->service		= NBHttpServicePort_alloc(NULL);
	opq->netBuffSize	= 0;	//Read net-buffer size
	opq->httpBuffSz		= 0;	//Read http-buffer size (zero if data must be ignored)
	opq->storgBuffSz	= 0;	//Storage body-buffer sizes (zero if must not be stored, only passed to listener)
	//
	STNBHttpServicePortLstnr listnr;
	NBMemory_set(&listnr, 0, sizeof(listnr));
	listnr.obj = opq;
	listnr.cltConnected		= NBHttpProxy_cltConnected_;
	listnr.cltDisconnected	= NBHttpProxy_cltDisconnected_;
	listnr.cltWillDisconnect	= NBHttpProxy_cltWillDisconnect_;
	listnr.cltRequested		= NBHttpProxy_cltRequested_;
	NBHttpServicePort_setListener(opq->service, &listnr);
}

void NBHttpProxy_release(STNBHttpProxy* obj){
	STNBHttpProxyOpaque* opq = (STNBHttpProxyOpaque*)obj->opaque;
	NBMemory_set(&opq->cfg, 0, sizeof(opq->cfg));
	NBHttpServicePort_release(&opq->service);
	opq->netBuffSize	= 0;	//Read net-buffer size
	opq->httpBuffSz		= 0;	//Read http-buffer size (zero if data must be ignored)
	opq->storgBuffSz	= 0;	//Storage body-buffer sizes (zero if must not be stored, only passed to listener)
}

//

BOOL NBHttpProxy_setConfig(STNBHttpProxy* obj, const STNBHttpProxyCfg* cfg){
	BOOL r = FALSE;
	STNBHttpProxyOpaque* opq = (STNBHttpProxyOpaque*)obj->opaque;
	if(cfg != NULL){
		if(NBHttpServicePort_loadFromConfig(opq->service, &cfg->service)){
			opq->cfg	= *cfg;
			r = TRUE;
		}
	} else {
		if(NBHttpServicePort_loadFromConfig(opq->service, NULL)){
			NBMemory_set(&opq->cfg, 0, sizeof(opq->cfg));
			r = TRUE;
		}
	}
	return r;
}

BOOL NBHttpProxy_setBuffersSzs(STNBHttpProxy* obj, const UI32 netReadBuffSz, const UI32 httpBodyReadSz, const UI32 httpBodyStorgChunksSz){
	BOOL r = FALSE;
	STNBHttpProxyOpaque* opq = (STNBHttpProxyOpaque*)obj->opaque;
	if(NBHttpServicePort_setBuffersSzs(opq->service, netReadBuffSz, httpBodyReadSz, httpBodyStorgChunksSz)){
		opq->netBuffSize	= netReadBuffSz;			//Read net-buffer size
		opq->httpBuffSz		= httpBodyReadSz;			//Read http-buffer size (zero if data must be ignored)
		opq->storgBuffSz	= httpBodyStorgChunksSz;	//Storage body-buffer sizes (zero if must not be stored, only passed to listener)
	}
	return r;
}

BOOL NBHttpProxy_bind(STNBHttpProxy* obj, const UI32 port){
	STNBHttpProxyOpaque* opq = (STNBHttpProxyOpaque*)obj->opaque;
	return NBHttpServicePort_bind(opq->service, port);
}

BOOL NBHttpProxy_listen(STNBHttpProxy* obj){
	STNBHttpProxyOpaque* opq = (STNBHttpProxyOpaque*)obj->opaque;
	return NBHttpServicePort_listen(opq->service);
}

BOOL NBHttpProxy_execute(STNBHttpProxy* obj){
	STNBHttpProxyOpaque* opq = (STNBHttpProxyOpaque*)obj->opaque;
	return NBHttpServicePort_execute(opq->service);
}

void NBHttpProxy_stopFlag(STNBHttpProxy* obj){
	STNBHttpProxyOpaque* opq = (STNBHttpProxyOpaque*)obj->opaque;
	NBHttpServicePort_stopFlag(opq->service);
}

void NBHttpProxy_waitForAll(STNBHttpProxy* obj){
	STNBHttpProxyOpaque* opq = (STNBHttpProxyOpaque*)obj->opaque;
	NBHttpServicePort_stop(opq->service);
}

//

void NBHttpProxyClt_disconnect(STNBHttpProxyClt* clt){
	clt->dstPort = 0;
	NBString_empty(&clt->dstServer);
	NBSocket_shutdown(clt->dstSocket, NB_IO_BITS_RDWR);
	NBSocket_close(clt->dstSocket);
}

BOOL NBHttpProxyClt_readHttpMessage(STNBHttpProxyClt* clt, STNBHttpMessage* dstMsg, const char* optReqMethod, UI32* dstBytesRead, STNBSocketRef* optCloneSocket) {
	BOOL r = TRUE;
	NBASSERT(clt->respCode == 0)
	STNBHttpProxyOpaque* opq = clt->opq;
	NBHttpMessage_feedStartResponseOf(dstMsg, opq->httpBuffSz, opq->storgBuffSz, optReqMethod);
	char* buff = (char*)NBMemory_alloc(opq->netBuffSize);
	{
		UI32 totalFeed = 0;
		do {
			//Wait for limit
			NBHttpServicePort_linkToLimitBeforeReceiving(clt->opq->service, clt->cltRef->clientItf);
			//Receive
			const int rcvd = NBSocket_recv(clt->dstSocket, buff, opq->netBuffSize);
			if (rcvd <= 0) {
				break;
			}
			else {
				//Inform to limit
				NBHttpServicePort_bytesRecvd(clt->opq->service, clt->cltRef->clientItf, rcvd);
				//Process
				const UI32 rr = NBHttpMessage_feedBytes(dstMsg, buff, rcvd);
				if (rr != rcvd) {
					PRINTF_ERROR("Client %llu, httpMesage has extra bytes %d of %d consumed.\n", (UI64)clt, rr, rcvd);
					r = FALSE;
					break;
				}
				else {
					totalFeed += rr;
					if (NBHttpMessage_feedIsComplete(dstMsg)) {
						break;
					}
				}
			}
		} while (r);
		if (r) {
			if (!NBHttpMessage_feedEnd(dstMsg)) {
				r = FALSE;
			}
		}
		if (dstBytesRead != NULL) *dstBytesRead = totalFeed;
	}
	NBMemory_free(buff);
	return r;
}

//---------------------------
//- Reading incoming request
//---------------------------

BOOL NBHttpProxyClt_consumeHeadStartLine_(const struct STNBHttpMessage_* obj, void* lparam){
	return TRUE;
}

BOOL NBHttpProxyClt_consumeHeadFieldLine_(const struct STNBHttpMessage_* obj, const STNBHttpHeaderField* field, void* lparam){
	return TRUE;
}

BOOL NBHttpProxyClt_consumeHeadEnd_(const struct STNBHttpMessage_* obj, UI32* dstBodyBuffSz, void* lparam){
	BOOL r = TRUE;
	{
		STNBHttpProxyClt* clt = (STNBHttpProxyClt*)lparam;
		//Reset request values
		{
			//Service
			clt->sendrItf		= NULL;
			clt->sendrParam		= NULL;
			NBString_empty(&clt->tmpStr);
			//Current response
			clt->respCode		= 0;
			NBString_empty(&clt->respReason);
			clt->internalErr	= FALSE; //Proxy internal error (program logic or parameters)
			clt->connOrigErr	= FALSE; //Origin closed the connection (or error)
			clt->connDstErr		= FALSE; //Destination close the connection (or error)
			clt->connClose		= FALSE; //End client connection after the request
			//Request/response properties
			clt->reqIsTunnel	= FALSE; //is a Tunnel request
			clt->reqIsChunkd	= FALSE; //origin request is chunked
			clt->respIsChunkd	= FALSE; //remote response is chunked
			clt->isTunneling	= FALSE; //currently tunneling (working)
		}
		//Connect to remote (if necesary)
		const char* hostStr = NBHttpHeader_getField(&obj->header, "Host");
		if(hostStr[0] == '\0'){
			clt->respCode	= 400;
			NBString_set(&clt->respReason, "Malformed request");
			PRINTF_INFO("Client %llu, received a request without 'Host' header.\n", (UI64)clt);
		} else {
			UI32 port = 80;
			STNBString host;
			NBString_init(&host);
			const SI32 portPos = NBString_strIndexOf(hostStr, ":", 0);
			if(portPos >= 0){
				NBString_setBytes(&host, hostStr, portPos);
				BOOL succ	= FALSE;
				port		= (UI32)NBNumParser_toUI32(&hostStr[portPos + 1], &succ);
				if(!succ) port = 80;
				//PRINTF_INFO("Client %llu, host: '%s':%d.\n", (UI64)clt, host.str, port);
			} else {
				NBString_set(&host, hostStr);
				//PRINTF_INFO("Client %llu, host: '%s'.\n", (UI64)clt, host.str);
			}
			const char* reqMethod	= NBHttpHeader_getRequestMethod(&obj->header);
			{
				if(!NBString_isEqual(&clt->dstServer, host.str) || clt->dstPort != port){
					NBHttpProxyClt_disconnect(clt);
					NBSocket_setNoSIGPIPE(clt->dstSocket, TRUE);
					NBSocket_setDelayEnabled(clt->dstSocket, FALSE);
					NBSocket_setCorkEnabled(clt->dstSocket, FALSE);
					if(!NBSocket_connect(clt->dstSocket, host.str, port)){
						clt->respCode	= 503;
						NBString_set(&clt->respReason, "Service Unavailable");
						PRINTF_ERROR("Client %llu, could not connect to destination '%s':%d.\n", (UI64)clt, host.str, port);
					} else {
						//PRINTF_INFO("Client %llu, connected to destination '%s':%d.\n", (UI64)clt, host.str, port);
						NBString_set(&clt->dstServer, host.str);
						clt->dstPort = port;
					}
				} else {
					//PRINTF_INFO("Client %llu, reusing connection to destination '%s':%d.\n", (UI64)clt, host.str, port);
				}
				//Send copy-of-header to destination
				if(clt->respCode == 0){
					if(NBString_strIsEqual(reqMethod, "CONNECT")){
						NBASSERT(!clt->isTunneling)
						clt->reqIsTunnel = TRUE;
					} else {
						NBString_empty(&clt->tmpStr);
						STNBHttpBuilder bldr;
						NBHttpBuilder_init(&bldr);
						if(!NBHttpBuilder_addHeader(&bldr, &clt->tmpStr, &obj->header)){
							//PRINTF_ERROR("Could not build request from header.\n");
							clt->respCode	= 500;
							NBString_set(&clt->respReason, "Internal Server Error");
						} else {
							//PRINTF_INFO("Client %llu, sent header (+%llu body).\n", (UI64)clt, body->dataTotalSz);
							//PRINTF_INFO("Header (+%llu body):\n%s\n", body->dataTotalSz, httpStr.str);
							NBHttpBuilder_addHeaderEnd(&bldr, &clt->tmpStr);
							//Wait for limit
							NBHttpServicePort_linkToLimitBeforeSending(clt->opq->service, clt->cltRef->clientItf);
							//Send header
							if(NBSocket_send(clt->dstSocket, clt->tmpStr.str, clt->tmpStr.length) != clt->tmpStr.length){
								//PRINTF_ERROR("Could not send %d bytes header.\n", httpStr.length);
								clt->connDstErr	= TRUE;
								clt->respCode	= 503;
								NBString_set(&clt->respReason, "Upstream error");
							} else {
								//PRINTF_INFO("Header sent to destination (%d bytes):\n%s\n.\n", httpStr.length, httpStr.str);
								//Inform to limit
								NBHttpServicePort_bytesSent(clt->opq->service, clt->cltRef->clientItf, clt->tmpStr.length);
								//Header sent to destination
								clt->reqIsChunkd = NBHttpHeader_isBodyChunked(&obj->header);
							}
						}
						NBHttpBuilder_release(&bldr);
					}
				}
			}
			NBString_release(&host);
		}
	}
	return r;
}

BOOL NBHttpProxyClt_consumeBodyData_(const struct STNBHttpMessage_* obj, const void* data, const UI64 dataSz, void* lparam){
	BOOL r = TRUE;
	{
		STNBHttpProxyClt* clt = (STNBHttpProxyClt*)lparam;
		//PRINTF_INFO("Consuming %llu bytes from origin-body.\n", dataSz);
		if(clt->respCode == 0){
			if(clt->reqIsTunnel){
				//Ignore
				NBASSERT(FALSE) //a CONNECT request should have body?
			} else {
				if(!clt->reqIsChunkd){
					//Wait for limit
					NBHttpServicePort_linkToLimitBeforeSending(clt->opq->service, clt->cltRef->clientItf);
					//Just send
					if(NBSocket_send(clt->dstSocket, data, (UI32)dataSz) != (UI32)dataSz){
						//PRINTF_ERROR("Could not send %d bytes body.\n", (UI32)dataSz);
						clt->connDstErr	= TRUE;
						clt->respCode	= 503;
						NBString_set(&clt->respReason, "Upstream error");
					} else {
						//Header send to destination
						//PRINTF_INFO("Client %llu, body sent to destination (%llu bytes).\n", (UI64)clt, dataSz);
						//Inform to limit
						NBHttpServicePort_bytesSent(clt->opq->service, clt->cltRef->clientItf, (UI32)dataSz);
					}
				} else {
					//Send chunked
					NBString_empty(&clt->tmpStr);
					STNBHttpBuilder bldr;
					NBHttpBuilder_init(&bldr);
					//Add chunk-header
					if(!NBHttpBuilder_addChunkHeader(&bldr, &clt->tmpStr, dataSz)){
						//PRINTF_ERROR("Could not build request from header.\n");
						clt->internalErr	= TRUE;
						clt->respCode		= 500;
						NBString_set(&clt->respReason, "Internal Server Error");
					} else {
						//Add chunk data
						NBString_concatBytes(&clt->tmpStr, data, (UI32)dataSz);
						//Add chunk-end
						if(!NBHttpBuilder_addChunkDataEnd(&bldr, &clt->tmpStr)){
							//PRINTF_ERROR("Could not build request from header.\n");
							clt->internalErr	= TRUE;
							clt->respCode		= 500;
							NBString_set(&clt->respReason, "Internal Server Error");
						} else {
							//Wait for limit
							NBHttpServicePort_linkToLimitBeforeSending(clt->opq->service, clt->cltRef->clientItf);
							//Send
							if(NBSocket_send(clt->dstSocket, clt->tmpStr.str, clt->tmpStr.length) != clt->tmpStr.length){
								//PRINTF_ERROR("Could not send %d bytes header.\n", httpStr.length);
								clt->connDstErr	= TRUE;
								clt->respCode	= 503;
								NBString_set(&clt->respReason, "Upstream error");
							} else {
								//Chunk send to destination
								//PRINTF_INFO("Body-chunk sent to destination (%llu bytes).\n", dataSz);
								//Inform to limit
								NBHttpServicePort_bytesSent(clt->opq->service, clt->cltRef->clientItf, clt->tmpStr.length);
							}
						}
					}
					NBHttpBuilder_release(&bldr);
				}
			}
		}
	}
	return r;
}

BOOL NBHttpProxyClt_consumeBodyTrailerField_(const struct STNBHttpMessage_* obj, const STNBHttpBodyField* field, void* lparam){
	{
		STNBHttpProxyClt* clt = (STNBHttpProxyClt*)lparam;
		if(clt->respCode == 0){
			if(clt->reqIsTunnel){
				//Ignore
				NBASSERT(FALSE) //a CONNECT request should have body?
				clt->internalErr	= TRUE;
				clt->respCode		= 500;
				NBString_set(&clt->respReason, "Internal Server Error");
			} else {
				NBASSERT(FALSE) //ToDo: implement
				clt->internalErr	= TRUE;
				clt->respCode		= 500;
				NBString_set(&clt->respReason, "Internal Server Error");
			}
		}
	}
	return TRUE;
}

BOOL NBHttpProxyClt_consumeBodyEnd_(const struct STNBHttpMessage_* obj, void* lparam){
	BOOL r = TRUE;
	{
		STNBHttpProxyClt* clt = (STNBHttpProxyClt*)lparam;
		if(clt->respCode == 0){
			if(clt->reqIsTunnel){
				//Ignore
			} else {
				if(clt->reqIsChunkd){
					//Send last chunk to remote
					NBString_empty(&clt->tmpStr);
					STNBHttpBuilder bldr;
					NBHttpBuilder_init(&bldr);
					if(!NBHttpBuilder_addChunkHeader(&bldr, &clt->tmpStr, 0)){
						//PRINTF_ERROR("Could not build request from header.\n");
						clt->internalErr	= TRUE;
						clt->respCode		= 500;
						NBString_set(&clt->respReason, "Internal Server Error");
					} else {
						//ToDo: allow send of the trailer fields.
						NBHttpBuilder_addChunkBodyEnd(&bldr, &clt->tmpStr);
						//Wait for limit
						NBHttpServicePort_linkToLimitBeforeSending(clt->opq->service, clt->cltRef->clientItf);
						if(NBSocket_send(clt->dstSocket, clt->tmpStr.str, clt->tmpStr.length) != clt->tmpStr.length){
							//PRINTF_ERROR("Could not send %d bytes header.\n", httpStr.length);
							clt->connDstErr	= TRUE;
							clt->respCode	= 503;
							NBString_set(&clt->respReason, "Upstream error");
						} else {
							//Last chunk sent
							//PRINTF_INFO("Body-last-chunk sent to destination.\n");
							//Inform to limit
							NBHttpServicePort_bytesSent(clt->opq->service, clt->cltRef->clientItf, clt->tmpStr.length);
						}
					}
					NBHttpBuilder_release(&bldr);
				}
			}
		}
	}
	return r;
}

//---------------------------
//- Reading remote response
//---------------------------

BOOL NBHttpProxyClt_consumeHeadStartLine2_(const struct STNBHttpMessage_* obj, void* lparam){
	return TRUE;
}

BOOL NBHttpProxyClt_consumeHeadFieldLine2_(const struct STNBHttpMessage_* obj, const STNBHttpHeaderField* field, void* lparam){
	return TRUE;
}

BOOL NBHttpProxyClt_consumeHeadEnd2_(const struct STNBHttpMessage_* obj, UI32* dstBodyBuffSz, void* lparam){
	BOOL r = TRUE;
	{
		STNBHttpProxyClt* clt = (STNBHttpProxyClt*)lparam;
		//Send copy-of-header to origin
		clt->respIsChunkd	= FALSE;
		NBASSERT(clt->respCode == 0) //No error should exists if reached this point
		if(clt->respCode == 0){
			NBString_empty(&clt->tmpStr);
			STNBHttpBuilder bldr;
			NBHttpBuilder_init(&bldr);
			if(!NBHttpBuilder_addHeader(&bldr, &clt->tmpStr, &obj->header)){
				//PRINTF_ERROR("Could not build request from header.\n");
				clt->internalErr	= TRUE;
				clt->respCode		= 500;
				NBString_set(&clt->respReason, "Internal Server Error");
			} else {
				//PRINTF_INFO("Client %llu, sent header (+%llu body).\n", (UI64)clt, body->dataTotalSz);
				//PRINTF_INFO("Header (+%llu body):\n%s\n", body->dataTotalSz, httpStr.str);
				NBHttpBuilder_addHeaderEnd(&bldr, &clt->tmpStr);
				//Send header
				if(!(*clt->sendrItf->sendBytes)(clt->sendrParam, clt->tmpStr.str, clt->tmpStr.length)){
					//PRINTF_ERROR("Could not send %d bytes header.\n", httpStr.length);
					clt->connOrigErr	= TRUE;
					clt->respCode		= 503;
					NBString_set(&clt->respReason, "Origin disconnected");
				} else {
					//Header send to destination
					//PRINTF_INFO("Header sent to origin (%d bytes):\n%s\n.\n", httpStr.length, httpStr.str);
					clt->respIsChunkd = NBHttpHeader_isBodyChunked(&obj->header);
				}
			}
			NBHttpBuilder_release(&bldr);
			//Close destination connection (to stop traffic)
			if(clt->respCode != 0){
				PRINTF_INFO("Client %llu, closing destination-conn because %d '%s'.\n", (UI64)clt, clt->respCode, clt->respReason.str);
				NBHttpProxyClt_disconnect(clt);
			}
		}
	}
	return r;
}

BOOL NBHttpProxyClt_consumeBodyData2_(const struct STNBHttpMessage_* obj, const void* data, const UI64 dataSz, void* lparam){
	BOOL r = TRUE;
	{
		STNBHttpProxyClt* clt = (STNBHttpProxyClt*)lparam;
		//PRINTF_INFO("Consuming %llu bytes from destine-body.\n", dataSz);
		//Send copy-of-data to origin
		if(clt->respCode == 0){
			if(!clt->respIsChunkd){
				//Just send
				if(!(*clt->sendrItf->sendBytes)(clt->sendrParam, data, (UI32)dataSz)){
					//PRINTF_ERROR("Could not send %d bytes header.\n", httpStr.length);
					clt->connOrigErr	= TRUE;
					clt->respCode		= 503;
					NBString_set(&clt->respReason, "Origin disconnected");
				} else {
					//Header send to origin
					//PRINTF_INFO("Body-data sent to origin (%llu bytes).\n", dataSz);
				}
			} else {
				//Send chunked
				NBString_empty(&clt->tmpStr);
				STNBHttpBuilder bldr;
				NBHttpBuilder_init(&bldr);
				//Add chunk-header
				if(!NBHttpBuilder_addChunkHeader(&bldr, &clt->tmpStr, dataSz)){
					//PRINTF_ERROR("Could not build request from header.\n");
					clt->internalErr	= TRUE;
					clt->respCode		= 500;
					NBString_set(&clt->respReason, "Internal Server Error");
				} else {
					//Add data
					NBString_concatBytes(&clt->tmpStr, data, (UI32)dataSz);
					//Add chunk-end
					if(!NBHttpBuilder_addChunkDataEnd(&bldr, &clt->tmpStr)){
						//PRINTF_ERROR("Could not build request from header.\n");
						clt->internalErr	= TRUE;
						clt->respCode		= 500;
						NBString_set(&clt->respReason, "Internal Server Error");
					} else {
						//Send
						if(!(*clt->sendrItf->sendBytes)(clt->sendrParam, clt->tmpStr.str, clt->tmpStr.length)){
							//PRINTF_ERROR("Could not send %d bytes header.\n", httpStr.length);
							clt->connOrigErr	= TRUE;
							clt->respCode		= 503;
							NBString_set(&clt->respReason, "Origin disconnected");
						} else {
							//Chunk send to origin
							//PRINTF_INFO("Body-chunk sent to origin (%llu bytes).\n", dataSz);
						}
					}
				}
				NBHttpBuilder_release(&bldr);
			}
			//Close destination connection (to stop traffic)
			if(clt->respCode != 0){
				PRINTF_INFO("Client %llu, closing destination-conn because %d '%s'.\n", (UI64)clt, clt->respCode, clt->respReason.str);
				NBHttpProxyClt_disconnect(clt);
			}
		}
	}
	return r;
}

BOOL NBHttpProxyClt_consumeBodyTrailerField2_(const struct STNBHttpMessage_* obj, const STNBHttpBodyField* field, void* lparam){
	{
		STNBHttpProxyClt* clt = (STNBHttpProxyClt*)lparam;
		NBASSERT(FALSE) //ToDo: implement
		NBASSERT(!clt->reqIsTunnel)
		if(clt->respCode == 0){
			clt->internalErr	= TRUE;
			clt->respCode		= 500;
			NBString_set(&clt->respReason, "Internal Server Error");
			//Close destination connection (to stop that traffic)
			if(clt->respCode != 0){
				PRINTF_INFO("Client %llu, closing destination-conn because %d '%s'.\n", (UI64)clt, clt->respCode, clt->respReason.str);
				NBHttpProxyClt_disconnect(clt);
			}
		}
	}
	return TRUE;
}

BOOL NBHttpProxyClt_consumeBodyEnd2_(const struct STNBHttpMessage_* obj, void* lparam){
	BOOL r = TRUE;
	{
		STNBHttpProxyClt* clt = (STNBHttpProxyClt*)lparam;
		if(clt->respCode == 0){
			if(!clt->respIsChunkd){
				//PRINTF_INFO("Body ended from remote.\n");
			} else {
				//Send last chunk to origin
				NBString_empty(&clt->tmpStr);
				STNBHttpBuilder bldr;
				NBHttpBuilder_init(&bldr);
				if(!NBHttpBuilder_addChunkHeader(&bldr, &clt->tmpStr, 0)){
					//PRINTF_ERROR("Could not build request from header.\n");
					clt->internalErr	= TRUE;
					clt->respCode		= 500;
					NBString_set(&clt->respReason, "Internal Server Error");
				} else {
					//ToDo: allow send of the trailer fields.
					NBHttpBuilder_addChunkBodyEnd(&bldr, &clt->tmpStr);
					if(!(*clt->sendrItf->sendBytes)(clt->sendrParam, clt->tmpStr.str, clt->tmpStr.length)){
						//PRINTF_ERROR("Could not send %d bytes header.\n", httpStr.length);
						clt->connOrigErr	= TRUE;
						clt->respCode		= 503;
						NBString_set(&clt->respReason, "Origin disconnected");
					} else {
						//Last chunk sent
						//PRINTF_INFO("Body-last-chunk sent to origin.\n");
					}
				}
				NBHttpBuilder_release(&bldr);
			}
			//Close destination connection (to stop that traffic)
			if(clt->respCode != 0){
				PRINTF_INFO("Client %llu, closing destination-conn because %d '%s'.\n", (UI64)clt, clt->respCode, clt->respReason.str);
				NBHttpProxyClt_disconnect(clt);
			}
		}
	}
	return r;
}


BOOL NBHttpProxy_cltConnected_(STNBHttpServicePortRef ref, const STNBHttpServiceConn* cltRef, IHttpMessageListener* dstMsgListener, void** dstMsgListenerParam, void* lparam){
	BOOL r = FALSE;
	{
		STNBHttpProxyClt* clt = NBMemory_allocType(STNBHttpProxyClt);
		//Init client
		{
			clt->opq			= (STNBHttpProxyOpaque*)lparam;
			clt->cltRef			= cltRef;
			clt->dstSocket		= NBSocket_alloc(NULL);
			NBSocket_createHnd(clt->dstSocket);
			NBSocket_setNoSIGPIPE(clt->dstSocket, TRUE);
			NBSocket_setCorkEnabled(clt->dstSocket, FALSE);
			NBSocket_setDelayEnabled(clt->dstSocket, FALSE);
			NBString_init(&clt->dstServer);
			clt->dstPort = 0;
			//Service
			clt->sendrItf		= NULL;
			clt->sendrParam		= NULL;
			NBString_initWithSz(&clt->tmpStr, 1024, 1024, 1.5f);
			//Current response
			clt->respCode		= 0;
			NBString_init(&clt->respReason);
			clt->internalErr	= FALSE; //Proxy internal error (program logic or parameters)
			clt->connOrigErr	= FALSE; //Origin closed the connection (or error)
			clt->connDstErr		= FALSE; //Destination close the connection (or error)
			clt->connClose		= FALSE; //End client connection after the request
			//Request/response properties
			clt->reqIsTunnel	= FALSE; //is a Tunnel request
			clt->reqIsChunkd	= FALSE; //origin request is chunked
			clt->respIsChunkd	= FALSE; //remote response is chunked
			clt->isTunneling	= FALSE; //currently tunneling (working)
		}
		//
		dstMsgListener->consumeHeadStartLine	= NBHttpProxyClt_consumeHeadStartLine_;
		dstMsgListener->consumeHeadFieldLine	= NBHttpProxyClt_consumeHeadFieldLine_;
		dstMsgListener->consumeHeadEnd			= NBHttpProxyClt_consumeHeadEnd_;
		dstMsgListener->consumeBodyData			= NBHttpProxyClt_consumeBodyData_;
		dstMsgListener->consumeBodyTrailerField	= NBHttpProxyClt_consumeBodyTrailerField_;
		dstMsgListener->consumeBodyEnd			= NBHttpProxyClt_consumeBodyEnd_;
		*dstMsgListenerParam = clt;
		//
		r = TRUE;
	}
	return r;
}

BOOL NBHttpProxy_cltDisconnected_(STNBHttpServicePortRef ref, void* msgListenerParam, void* lparam){
	BOOL r = FALSE;
	STNBHttpProxyClt* clt = (STNBHttpProxyClt*)msgListenerParam;
	{
		NBHttpProxyClt_disconnect(clt);
		//
		NBSocket_shutdown(clt->dstSocket, NB_IO_BITS_RDWR);
		NBSocket_release(&clt->dstSocket);
		NBSocket_null(&clt->dstSocket);
		NBString_release(&clt->dstServer);
		clt->dstPort = 0;
		//Service
		clt->sendrItf		= NULL;
		clt->sendrParam		= NULL;
		NBString_release(&clt->tmpStr);
		//Current response
		clt->respCode		= 0;
		NBString_release(&clt->respReason);
		clt->internalErr	= FALSE; //Proxy internal error (program logic or parameters)
		clt->connOrigErr	= FALSE; //Origin closed the connection (or error)
		clt->connDstErr		= FALSE; //Destination close the connection (or error)
		clt->connClose		= FALSE; //End client connection after the request
		//Request/response properties
		clt->reqIsTunnel	= FALSE; //is a Tunnel request
		clt->reqIsChunkd	= FALSE; //origin request is chunked
		clt->respIsChunkd	= FALSE; //remote response is chunked
		clt->isTunneling	= FALSE; //currently tunneling (working)
		//
		NBMemory_free(clt);
		r = TRUE;
	}
	return r;
}

SI64 NBHttpNBHttpProxy_runTunnelFromRemoteAndRelease(STNBThread* thread, void* param){
	STNBHttpProxyClt* clt		= (STNBHttpProxyClt*)param;
	STNBHttpProxyOpaque* opq	= clt->opq;
	clt->isTunneling			= TRUE;
	char* buff = (char*)NBMemory_alloc(opq->netBuffSize);
	{
		UI64 totalRcvd = 0;
		do {
			//Wait for limit
			NBHttpServicePort_linkToLimitBeforeReceiving(clt->opq->service, clt->cltRef->clientItf);
			//Receive
			const UI32 rcvd = NBSocket_recv(clt->dstSocket, buff, opq->netBuffSize);
			if (rcvd <= 0) {
				break;
			}
			else {
				totalRcvd += rcvd;
				//Inform to limit
				NBHttpServicePort_bytesRecvd(clt->opq->service, clt->cltRef->clientItf, rcvd);
				//PRINTF_INFO("Tunnel received %d/%llu from '%s':%d.\n", rcvd, totalRcvd, clt->dstServer.str, clt->dstPort);
				if (!(clt->sendrItf->sendBytes)(clt->sendrParam, buff, rcvd)) {
					break;
				}
			}
		} while (TRUE);
		//PRINTF_INFO("Client %llu, closing tunnel after receiving %llu bytes from '%s':%d.\n", (UI64)clt, totalRcvd, clt->dstServer.str, clt->dstPort);
		NBHttpProxyClt_disconnect(clt);
		clt->isTunneling = FALSE;
		//Destroy thread
		NBThread_release(thread);
		NBMemory_free(thread);
		thread = NULL;
	}
	NBMemory_free(buff);
	return 0;
}

void NBHttpProxy_cltWillDisconnect_(STNBHttpServicePortRef ref, void* msgListenerParam, void* lparam){
	STNBHttpProxyClt* clt = (STNBHttpProxyClt*)msgListenerParam;
	//Disconnect remote socket
	NBHttpProxyClt_disconnect(clt);
}

BOOL NBHttpProxy_cltRequested_(STNBHttpServicePortRef ref, const STNBHttpMessage* msg, const IHttpServicePortSender* sendrItf, void* sendrParam, void* msgListenerParam, void* lparam){
	BOOL r = FALSE;
	STNBHttpProxyClt* clt		= (STNBHttpProxyClt*)msgListenerParam;
	STNBHttpProxyOpaque* opq	= clt->opq;
	{
		clt->sendrItf	= sendrItf;
		clt->sendrParam	= sendrParam;
		//Open the tunnel
		if(clt->respCode == 0 && clt->reqIsTunnel){
			NBASSERT(!clt->internalErr && !clt->connOrigErr && !clt->connDstErr)
			//Start thread
			if(sendrItf->receiveBytes == NULL || sendrItf->disconnect == NULL){
				clt->respCode	= 500;
				NBString_set(&clt->respReason, "Internal Server Error");
				NBASSERT(FALSE)
			} else {
				STNBThread* thread	= NBMemory_allocType(STNBThread);
				NBThread_init(thread);
				NBThread_setIsJoinable(thread, FALSE); //Release thread resources after exit
				clt->isTunneling = TRUE;
				//PRINTF_INFO("Client %llu, starting tunnel to '%s':%d.\n", (UI64)clt, clt->dstServer.str, clt->dstPort);
				if(!NBThread_start(thread, NBHttpNBHttpProxy_runTunnelFromRemoteAndRelease, clt, NULL)){
					PRINTF_ERROR("Client %llu, could not create thread for new http-tunnel.\n", (UI64)clt);
					clt->isTunneling = FALSE;
					//Destroy thread
					NBThread_release(thread);
					NBMemory_free(thread);
					thread = NULL;
					clt->internalErr	= TRUE;
					clt->respCode		= 500;
					NBString_set(&clt->respReason, "Internal Server Error");
				} else {
					clt->respCode		= 200;
					NBString_set(&clt->respReason, "OK");
				}
			}
		}
		//Respond explicit code
		if(clt->respCode != 0){
			//Http-sync, send explicit error
			STNBHttpHeader header;
			NBHttpHeader_init(&header);
			NBHttpHeader_setStatusLine(&header, 1, 1, clt->respCode, clt->respReason.str);
			/ *if(clt->closeConn){ //ToDo: implement
			 NBHttpHeader_addField(&header, "Connection", "close");
			 }* /
			NBHttpHeader_addField(&header, "Content-Length", "0");
			if(!(*sendrItf->sendHeader)(sendrParam, &header)){
				//PRINTF_ERROR("Client %llu, could not send %d '%s' response to request with no 'Host' header.\n", (UI64)clt, clt->respCode, clt->respReason.str);
				clt->connOrigErr	= TRUE;
				clt->respCode		= 503;
				NBString_set(&clt->respReason, "Origin disconnected");
				r = FALSE;
			} else {
				r = TRUE; //Response sent
				//Tunnel to destiny
				if(clt->reqIsTunnel && clt->isTunneling){
					char* buff = (char*)NBMemory_alloc(opq->netBuffSize);
					{
						UI64 totalRcvd = 0;
						do {
							const UI32 rcvd = (*sendrItf->receiveBytes)(sendrParam, buff, opq->netBuffSize);
							if (rcvd <= 0) {
								break;
							}
							else {
								totalRcvd += rcvd;
								//PRINTF_INFO("Tunnel received %d/%llu to '%s':%d.\n", rcvd, totalRcvd, clt->dstServer.str, clt->dstPort);
								//Wait for limit
								NBHttpServicePort_linkToLimitBeforeSending(clt->opq->service, clt->cltRef->clientItf);
								//Send
								if (NBSocket_send(clt->dstSocket, buff, rcvd) != rcvd) {
									break;
								}
								else {
									//Inform to limit
									NBHttpServicePort_bytesSent(clt->opq->service, clt->cltRef->clientItf, rcvd);
								}
							}
						} while (TRUE);
						//PRINTF_INFO("Client %llu, closing tunnel after receiving %llu bytes from origin to '%s':%d.\n", (UI64)clt, totalRcvd, clt->dstServer.str, clt->dstPort);
						//Close and wait
						(*sendrItf->disconnect)(sendrParam);
						while (clt->isTunneling) {
							//Wait
						}
					}
					NBMemory_free(buff);
				}
			}
			NBHttpHeader_release(&header);
		} else {
			NBASSERT(!clt->reqIsTunnel)
			const char* method		= &msg->header.strs.str[msg->header.requestLine->method];
			const char* target		= &msg->header.strs.str[msg->header.requestLine->target];
			//Request was already sent to destination, read response
			STNBHttpMessage msg2;
			NBHttpMessage_init(&msg2);
			{
				IHttpMessageListener listnr;
				NBMemory_set(&listnr, 0, sizeof(listnr));
				listnr.consumeHeadStartLine		= NBHttpProxyClt_consumeHeadStartLine2_;
				listnr.consumeHeadFieldLine		= NBHttpProxyClt_consumeHeadFieldLine2_;
				listnr.consumeHeadEnd			= NBHttpProxyClt_consumeHeadEnd2_;
				listnr.consumeBodyData			= NBHttpProxyClt_consumeBodyData2_;
				listnr.consumeBodyTrailerField	= NBHttpProxyClt_consumeBodyTrailerField2_;
				listnr.consumeBodyEnd			= NBHttpProxyClt_consumeBodyEnd2_;
				NBHttpMessage_setListener(&msg2, &listnr, clt);
			}
			UI32 bytesCsmd = 0;
			if(!NBHttpProxyClt_readHttpMessage(clt, &msg2, method, &bytesCsmd, NULL)){
				clt->connDstErr	= TRUE;
				clt->respCode	= 503;
				NBString_set(&clt->respReason, "Upstream error");
				if(msg2.header.statusLine != NULL){
					PRINTF_ERROR("Client %llu, incomplete response status(%d) reading %s after %d bytes for '%s'.\n", (UI64)clt, msg2.header.statusLine->statusCode, (msg2.header.isCompleted ? "body" : "header"), bytesCsmd, target);
				} else {
					PRINTF_ERROR("Client %llu, incomplete response after %d bytes for '%s'.\n", (UI64)clt, bytesCsmd, target);
				}
				if(bytesCsmd > 0){
					r = FALSE; //At this point, disconnect (http sync lost)
				}
			} else {
				const UI32 statusCode	= msg2.header.statusLine->statusCode;
				const char* reason		= &msg2.header.strs.str[msg2.header.statusLine->reasonPhrase];
				const UI32 targetSz		= NBString_strLenBytes(target);
				const UI32 maxTargetComplete = 64;
				const UI32 maxTargetShow = 60; NBASSERT(maxTargetShow <= maxTargetComplete)
				if(targetSz > maxTargetComplete){
					STNBString sShort;
					NBString_init(&sShort);
					NBString_concatBytes(&sShort, target, maxTargetShow);
					//PRINTF_INFO("Client %llu, %s %d '%s' for '%s'... (+%d chars).\n", (UI64)clt, method, statusCode, reason, sShort.str, (targetSz - maxTargetShow));
					NBString_release(&sShort);
				} else {
					//PRINTF_INFO("Client %llu, %s %d '%s' for '%s'.\n", (UI64)clt, method, statusCode, reason, target);
				}
				r = TRUE; //Response sent to origin
			}
			NBHttpMessage_release(&msg2);
		}
	}
	return r;
}
*/
