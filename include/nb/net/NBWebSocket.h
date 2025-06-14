#ifndef NB_WEBSOCKET_H
#define NB_WEBSOCKET_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBStructMap.h"

#include "nb/net/NBHttpHeader.h"
#include "nb/net/NBWebSocketMessage.h"
//

#ifdef __cplusplus
extern "C" {
#endif
	
	//-----------------
	//-- WebSocket - RFC6455
	//-- https://datatracker.ietf.org/doc/html/rfc6455
	//-----------------

	NB_OBJREF_HEADER(NBWebSocket)
	
	BOOL NBWebSocket_concatResponseHeader(STNBWebSocketRef ref, const STNBHttpHeader* req, STNBHttpHeader* dst);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
