#ifndef NB_HTTP_PROXY_H
#define NB_HTTP_PROXY_H

#include "nb/NBFrameworkDefs.h"
#include "nb/net/NBHttpHeader.h"
#include "nb/net/NBHttpBody.h"
#include "nb/net/NBHttpMessage.h"
#include "nb/net/NBHttpServicePort.h"

//

#ifdef __cplusplus
extern "C" {
#endif

	/*
	//Config
	typedef struct STNBHttpProxyCfg_ {
		STNBHttpPortCfg	service;
	} STNBHttpProxyCfg;
	
	typedef struct STNBHttpProxy_ {
		void*		opaque;
	} STNBHttpProxy;
	
	void NBHttpProxy_init(STNBHttpProxy* obj);
	void NBHttpProxy_release(STNBHttpProxy* obj);
	
	BOOL NBHttpProxy_setConfig(STNBHttpProxy* obj, const STNBHttpProxyCfg* cfg);
	BOOL NBHttpProxy_setBuffersSzs(STNBHttpProxy* obj, const UI32 netReadBuffSz, const UI32 httpBodyReadSz, const UI32 httpBodyStorgChunksSz);
	BOOL NBHttpProxy_bind(STNBHttpProxy* obj, const UI32 port);
	BOOL NBHttpProxy_listen(STNBHttpProxy* obj);
	BOOL NBHttpProxy_execute(STNBHttpProxy* obj);
	void NBHttpProxy_stopFlag(STNBHttpProxy* obj);
	void NBHttpProxy_waitForAll(STNBHttpProxy* obj);
	*/

#ifdef __cplusplus
} //extern "C"
#endif

#endif
