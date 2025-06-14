#ifndef NB_HTTP2_PEER_H
#define NB_HTTP2_PEER_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/net/NBHttp2Parser.h"
#include "nb/net/NBHttp2Hpack.h"
#include "nb/ssl/NBSsl.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBHttp2Peer_ {
		STNBHttp2Settings	settings;
		STNBHttp2Hpack		hpackLocal;
		STNBHttp2Hpack		hpackRemote;
	} STNBHttp2Peer;
	
	void NBHttp2Peer_init(STNBHttp2Peer* obj);
	void NBHttp2Peer_release(STNBHttp2Peer* obj);
	
	BOOL NBHttp2Peer_sendSettings(STNBHttp2Peer* obj, STNBSslRef ssl, const UI32 settingsMask);
	BOOL NBHttp2Peer_sendSettingsAck(STNBHttp2Peer* obj, STNBSslRef ssl);
	BOOL NBHttp2Peer_sendHeaders(STNBHttp2Peer* obj, const STNBHttpHeader* headers, STNBSslRef ssl, const UI32 streamId);
	BOOL NBHttp2Peer_sendData(STNBHttp2Peer* obj, const void* data, const UI32 dataSz, STNBSslRef ssl, const UI32 streamId);
	
	BOOL NBHttp2Peer_receiveFrame(STNBHttp2Peer* obj, STNBSslRef ssl, STHttp2FrameHead* dstHead, STNBString* dstPayload);
	BOOL NBHttp2Peer_processFrame(STNBHttp2Peer* obj, const STHttp2FrameHead* head, const void* pay, const UI32 paySz);
	
	BOOL NBHttp2Peer_receiveSettings(STNBHttp2Peer* obj, STNBSslRef ssl);
	BOOL NBHttp2Peer_receiveSettingsAck(STNBHttp2Peer* obj, STNBSslRef ssl);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
