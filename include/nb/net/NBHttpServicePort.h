#ifndef NBHttpServicePort_h
#define NBHttpServicePort_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBStopFlag.h"
#include "nb/core/NBIOPollster.h"
#include "nb/core/NBIOPollstersProvider.h"
#include "nb/net/NBHttpHeader.h"
#include "nb/net/NBHttpBody.h"
#include "nb/net/NBHttpMessage.h"
#include "nb/net/NBHttpCfg.h"
#include "nb/net/NBHttpServiceConn.h"
#include "nb/crypto/NBX509.h"
#include "nb/ssl/NBSslContext.h"
//
#include "nb/net/NBHttpStats.h"

#ifdef __cplusplus
extern "C" {
#endif

	//NBHttpServicePort

	NB_OBJREF_HEADER(NBHttpServicePort)
	
	//Listener
	
	typedef struct STNBHttpServicePortLstnrItf_ {
        void (*httpPortStopped)(STNBHttpServicePortRef port, void* usrData);
		BOOL (*httpPortCltConnected)(STNBHttpServicePortRef port, STNBHttpServiceConnRef conn, void* usrData);
		void (*httpPortCltDisconnected)(STNBHttpServicePortRef port, STNBHttpServiceConnRef conn, void* usrData);
        BOOL (*httpPortCltReqArrived)(STNBHttpServicePortRef port, STNBHttpServiceConnRef conn, const STNBHttpServiceReqDesc reqDesc, STNBHttpServiceReqArrivalLnk reqLnk, void* usrData);    //called when header-frist-line arrived, when header completion arrived or when body completing arrived; first to populate required methods into 'dstLtnr' take ownership and stops further calls to this method.
	} STNBHttpServicePortLstnrItf;

	typedef struct STNBHttpServicePortLstnr_ {
		STNBHttpServicePortLstnrItf itf;
		void*						usrData;
	} STNBHttpServicePortLstnr;

	//Service
	BOOL NBHttpServicePort_loadFromConfig(STNBHttpServicePortRef ref, STNBX509* caCert, const STNBHttpPortCfg* cfg, STNBHttpStatsRef parentProvider, const STNBHttpServicePortLstnrItf* lstnrItf, void* lstnrUsrData);
	BOOL NBHttpServicePort_setBuffersSzs(STNBHttpServicePortRef ref, const UI32 netReadBuffSz, const UI32 httpBodyReadSz, const UI32 httpBodyStorgChunksSz);
	BOOL NBHttpServicePort_setListener(STNBHttpServicePortRef ref, const STNBHttpServicePortLstnr* lstnr);
	BOOL NBHttpServicePort_setSslContext(STNBHttpServicePortRef ref, STNBSslContextRef sslContext);
	BOOL NBHttpServicePort_setPollster(STNBHttpServicePortRef ref, STNBIOPollsterRef pollster, STNBIOPollsterSyncRef pollSync);	//when one pollster only
	BOOL NBHttpServicePort_setPollstersProvider(STNBHttpServicePortRef ref, STNBIOPollstersProviderRef provider); //when multiple pollsters
    BOOL NBHttpServicePort_setParentStopFlag(STNBHttpServicePortRef ref, STNBStopFlagRef* parentStopFlag);
	UI32 NBHttpServicePort_getPort(STNBHttpServicePortRef ref);
	//
	BOOL NBHttpServicePort_prepare(STNBHttpServicePortRef ref);
	BOOL NBHttpServicePort_startListening(STNBHttpServicePortRef ref);
	BOOL NBHttpServicePort_isBusy(STNBHttpServicePortRef ref);
	void NBHttpServicePort_stopFlag(STNBHttpServicePortRef ref);

	//Commands
	BOOL NBHttpServicePort_statsFlushStart(STNBHttpServicePortRef ref, const UI32 iSeq);
	BOOL NBHttpServicePort_statsFlushIsPend(STNBHttpServicePortRef ref, const UI32 iSeq);
	void NBHttpServicePort_statsGet(STNBHttpServicePortRef ref, STNBHttpStatsData* dst, const BOOL resetAccum);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
