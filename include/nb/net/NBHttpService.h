#ifndef NB_HTTP_SERVICE_H
#define NB_HTTP_SERVICE_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBStopFlag.h"
#include "nb/core/NBIOPollster.h"
#include "nb/core/NBIOPollstersProvider.h"
#include "nb/net/NBHttpCfg.h"
#include "nb/net/NBHttpStats.h"
#include "nb/net/NBHttpServicePort.h"
#include "nb/net/NBHttpServiceConn.h"
#include "nb/crypto/NBX509.h"
#include "nb/crypto/NBPKey.h"
//

#ifdef __cplusplus
extern "C" {
#endif

	//NBHttpServiceCmdState

	typedef struct STNBHttpServiceCmdState_ {
		UI32		seq;
		BOOL		isPend;		//zero means command was completed
	} STNBHttpServiceCmdState;

	void NBHttpServiceCmdState_init(STNBHttpServiceCmdState* obj);
	void NBHttpServiceCmdState_release(STNBHttpServiceCmdState* obj);

	//NBHttpService

	NB_OBJREF_HEADER(NBHttpService)
	
	//NBHttpServiceLstnrItf

	typedef struct STNBHttpServiceLstnrItf_ {
		BOOL (*httpCltConnected)(STNBHttpServiceRef srv, const UI32 port, STNBHttpServiceConnRef conn, void* usrData);			//new client connected
		void (*httpCltDisconnected)(STNBHttpServiceRef srv, const UI32 port, STNBHttpServiceConnRef conn, void* usrData);		//connection was closed or lost
        //request
        BOOL (*httpCltReqArrived)(STNBHttpServiceRef srv, const UI32 port, STNBHttpServiceConnRef conn, const STNBHttpServiceReqDesc reqDesc, STNBHttpServiceReqArrivalLnk reqLnk, void* usrData);    //called when header-frist-line arrived, when header completion arrived or when body completing arrived; first to populate required methods into 'dstLtnr' take ownership and stops further calls to this method.
	} STNBHttpServiceLstnrItf;

	//NBHttpService

	//Cfg
	BOOL NBHttpService_setCaCert(STNBHttpServiceRef ref, STNBX509* caCert);
	BOOL NBHttpService_setCaCertAndKey(STNBHttpServiceRef ref, STNBX509* caCert, STNBPKey* caKey);
	BOOL NBHttpService_setPollster(STNBHttpServiceRef ref, STNBIOPollsterRef pollster, STNBIOPollsterSyncRef pollSync);	//when one pollster only
	BOOL NBHttpService_setPollstersProvider(STNBHttpServiceRef ref, STNBIOPollstersProviderRef provider); //when multiple pollsters
    BOOL NBHttpService_setParentStopFlag(STNBHttpServiceRef ref, STNBStopFlagRef* parentStopFlag);
	//
	BOOL NBHttpService_prepare(STNBHttpServiceRef ref, const STNBHttpServiceCfg* cfg, const STNBHttpServiceLstnrItf* lstnrItf, void* lstnrUsrData);
	BOOL NBHttpService_startListening(STNBHttpServiceRef ref);
	BOOL NBHttpService_isBusy(STNBHttpServiceRef ref);
	void NBHttpService_stopFlag(STNBHttpServiceRef ref);

	//Commands
	BOOL NBHttpService_statsFlushStart(STNBHttpServiceRef ref, STNBHttpServiceCmdState* dst);
	BOOL NBHttpService_statsFlushIsPend(STNBHttpServiceRef ref, STNBHttpServiceCmdState* dst);
	void NBHttpService_statsGet(STNBHttpServiceRef ref, STNBHttpStatsData* dst, const BOOL resetAccum);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
