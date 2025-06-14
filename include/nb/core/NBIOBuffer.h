#ifndef NBIOBuffer_h
#define NBIOBuffer_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBIOPollster.h"
#include "nb/core/NBStopFlag.h"
#include "nb/net/NBHttpHeader.h"
#include "nb/net/NBHttpBody.h"
#include "nb/net/NBHttpMessage.h"
#include "nb/net/NBHttpCfg.h"
#include "nb/crypto/NBX509.h"
#include "nb/ssl/NBSslContext.h"
//
#include "nb/net/NBHttpStats.h"
#include "nb/net/NBHttpServiceResp.h"

#ifdef __cplusplus
extern "C" {
#endif

	//NBIOBuffer

	NB_OBJREF_HEADER(NBIOBuffer)	//client connected

    //cfg
	BOOL NBIOBuffer_isObjRef(STNBIOBufferRef ref, STNBObjRef objRef);

	//
	BOOL NBIOBuffer_startListeningOwningSocket(STNBIOBufferRef ref, STNBIOPollsterSyncRef pollSync, STNBStopFlagRef* parentStopFlag, STNBSocketRef socket);
	void NBIOBuffer_stopFlag(STNBIOBufferRef ref);
	BOOL NBIOBuffer_isBusy(STNBIOBufferRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
