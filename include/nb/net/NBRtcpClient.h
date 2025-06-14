#ifndef NB_RTCP_SERVER_H
#define NB_RTCP_SERVER_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBIOPollster.h"
#include "nb/core/NBStopFlag.h"
//

#ifdef __cplusplus
extern "C" {
#endif
	
typedef struct STNBRtcpClientListener_ {
	void*	obj;
} STNBRtcpClientListener;

typedef struct STNBRtcpClient_ {
	void* opaque;
} STNBRtcpClient;

void NBRtcpClient_init(STNBRtcpClient* obj);
void NBRtcpClient_release(STNBRtcpClient* obj);

//cfg
BOOL NBRtcpClient_setParentStopFlag(STNBRtcpClient* obj, STNBStopFlagRef* stopFlag);
BOOL NBRtcpClient_setPollster(STNBRtcpClient* obj, STNBIOPollsterRef pollster, STNBIOPollsterSyncRef pollSync);

//Listeners
BOOL NBRtcpClient_addListener(STNBRtcpClient* obj, STNBRtcpClientListener listener);
BOOL NBRtcpClient_removeListener(STNBRtcpClient* obj, void* lstnrObj);

//Actions
BOOL NBRtcpClient_bind(STNBRtcpClient* obj, const SI32 port);
BOOL NBRtcpClient_startListening(STNBRtcpClient* obj);
BOOL NBRtcpClient_isBusy(STNBRtcpClient* obj);
void NBRtcpClient_stopFlag(STNBRtcpClient* obj);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
