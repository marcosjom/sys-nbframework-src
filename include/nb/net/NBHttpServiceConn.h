#ifndef NBHttpServiceConn_h
#define NBHttpServiceConn_h

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


#   define NB_HTTP_SERVICE_REQ_MS_NEXT_TICK_UNCHANGED    0xFFFFFFFF

    //NBHttpServiceRespCtx; Response context

    typedef struct STNBHttpServiceRespCtx_ {
        //req
        struct {
            //header
            struct {
                BOOL    isCompleted;
                const STNBHttpHeader* ref;
            } header;
            //body
            struct {
                BOOL    isCompleted;
                const STNBHttpBody* ref;
            } body;
        } req;
        //resp
        struct {
            STNBHttpServiceRespLnk  lnk;
        } resp;
        //ticks
        struct {
            UI32        msPerTick;      //default milliseconds per call to tick, the real time elapsed can be modified on each tick-call (from the parameters).
            UI32        usAccum;        //usecs accumulated for next tick-call.
            STNBTimestampMicro last;    //time last tick-call.
            BOOL        isLastSet;      //last timestamp populated
        } ticks;
    } STNBHttpServiceRespCtx;

    //NBHttpServiceReqLine; Request line

    typedef struct STNBHttpServiceReqDesc_ {
        //first line (allways available)
        struct {
            const char*         method;
            const char*         target;
            UI8                 majorVer;
            UI8                 minorVer;
        } firstLine;
        //header (NULL unless completed)
        const STNBHttpHeader*   header;
        //body (NULL unless completed)
        const STNBHttpBody*     body;
    } STNBHttpServiceReqDesc;

    //NBHttpServiceReqLstnrItf
    //Note: the parameter 'const STNBHttpServiceRespCtx respContext' is
    //intentionaly passed as a const-copy instead of pointer to reduce
    //chance of external manipulation.

#   define NBHttpServiceReqLstnrItf_isSet(PTR)  ((PTR)->httpReqOwnershipEnded != NULL)

    typedef struct STNBHttpServiceReqLstnrItf_ {
        //required
        void (*httpReqOwnershipEnded)       (const STNBHttpServiceRespCtx ctx, void* usrData);    //request ended and none of these callbacks will be called again, release resources (not necesary to call 'httpReqRespClose')
        //Reading header (optional, if request real-time processing is implement for the request)
        BOOL (*httpReqConsumeHeadFieldLine) (const STNBHttpServiceRespCtx ctx, const STNBHttpHeaderField* field, void* lparam);
        BOOL (*httpReqConsumeHeadEnd)       (const STNBHttpServiceRespCtx ctx, UI32* dstBodyBuffSz, void* lparam);
        //Reading body (optional, if request real-time processing is implement for the request)
        BOOL (*httpReqConsumeBodyData)      (const STNBHttpServiceRespCtx ctx, const void* data, const UI64 dataSz, void* lparam);
        BOOL (*httpReqConsumeBodyTrailerField)(const STNBHttpServiceRespCtx ctx, const STNBHttpBodyField* field, void* lparam);
        BOOL (*httpReqConsumeBodyEnd)       (const STNBHttpServiceRespCtx ctx, void* lparam);
        //Reading raw-bytes
        SI32 (*httpReqRcvd)                 (const STNBHttpServiceRespCtx ctx, const void* data, const UI32 dataSz, void* usrData); //request rcvd data
        //Ticks
        void (*httpReqTick)                 (const STNBHttpServiceRespCtx ctx, const STNBTimestampMicro tickTime, const UI64 msCurTick, const UI32 msNextTick, UI32* dstMsNextTick, void* usrData);        //request poll-tick
        //Flush
        void (*httpSendFlushed)             (const STNBHttpServiceRespCtx ctx, void* usrData); //the write buffer was fully consumed after populated at least once
    } STNBHttpServiceReqLstnrItf;

    //NBHttpServiceReqArrivalLnkItf
    
    typedef struct STNBHttpServiceReqArrivalLnkItf_ {
        UI32 (*httpReqGetDefaultResponseCode) (STNBString* dstReason, void* usrData);   //get request's default response code
        BOOL (*httpReqSetDefaultResponseCode) (const UI32 code, const char* reason, void* usrData);   //set request's default response code
        BOOL (*httpReqAddDefaultResponseField) (const char* name, const char* value, void* usrData);   //adds a header-field's for the default response without checking (allows duplicated fields)
        BOOL (*httpReqSetDefaultResponseField) (const char* name, const char* value, void* usrData);   //adds or updates a header-field's for the default response without checking (wont allow duplicated fields)
        BOOL (*httpReqSetDefaultResponseBodyStr) (const char* body, void* usrData);   //set request's default response code
        BOOL (*httpReqSetDefaultResponseBodyData) (const void* data, const UI32 dataSz, void* usrData);   //set request's default response code
        BOOL (*httpReqConcatDefaultResponseBodyStruct) (const STNBStructMap* structMap, const void* src, const UI32 srcSz, void* usrData);   //set request's default response code
        BOOL (*httpReqSetOwner)             (const STNBHttpServiceReqLstnrItf* itf, void* itfParam, const UI32 msPerTick, void* usrData);   //set request owner
    } STNBHttpServiceReqArrivalLnkItf;

    //NBHttpServiceReqArrivalLnk

    typedef struct STNBHttpServiceReqArrivalLnk_ {
        STNBHttpServiceReqArrivalLnkItf     itf;
        void*                               usrParam;
    } STNBHttpServiceReqArrivalLnk;

    UI32 NBHttpServiceReqArrivalLnk_getDefaultResponseCode(STNBHttpServiceReqArrivalLnk* obj);
    UI32 NBHttpServiceReqArrivalLnk_getDefaultResponseCodeAndReason(STNBHttpServiceReqArrivalLnk* obj, STNBString* optDstReason);
    BOOL NBHttpServiceReqArrivalLnk_setDefaultResponseCode(STNBHttpServiceReqArrivalLnk* obj, const UI32 code, const char* reason);
    BOOL NBHttpServiceReqArrivalLnk_addDefaultResponseField(STNBHttpServiceReqArrivalLnk* obj, const char* name, const char* value);
    BOOL NBHttpServiceReqArrivalLnk_setDefaultResponseField(STNBHttpServiceReqArrivalLnk* obj, const char* name, const char* value);
    BOOL NBHttpServiceReqArrivalLnk_setDefaultResponseBodyStr(STNBHttpServiceReqArrivalLnk* obj, const char* body);
    BOOL NBHttpServiceReqArrivalLnk_setDefaultResponseBodyData(STNBHttpServiceReqArrivalLnk* obj, const void* data, const UI32 dataSz);
    BOOL NBHttpServiceReqArrivalLnk_concatDefaultResponseBodyStruct(STNBHttpServiceReqArrivalLnk* obj, const STNBStructMap* structMap, const void* src, const UI32 srcSz);
    BOOL NBHttpServiceReqArrivalLnk_setOwner(STNBHttpServiceReqArrivalLnk* obj, const STNBHttpServiceReqLstnrItf* itf, void* itfParam, const UI32 msPerTick);

	//NBHttpServiceConn

	NB_OBJREF_HEADER(NBHttpServiceConn)	//client connected

	typedef struct STNBHttpServiceConnLstnrItf_ {
        //connection ended
        void (*httpConnStopped)(STNBHttpServiceConnRef conn, void* usrData);    //when socket-error or stopFlags is consumed
        BOOL (*httpConnReqArrived)(STNBHttpServiceConnRef conn, const STNBHttpServiceReqDesc reqDesc, STNBHttpServiceReqArrivalLnk reqLnk, void* usrData);    //called when header-frist-line arrived, when header completion arrived or when body completing arrived; first to populate required methods into 'dstLtnr' take ownership and stops further calls to this method.
	} STNBHttpServiceConnLstnrItf;

	//cfg
	BOOL NBHttpServiceConn_isCertPresent(STNBHttpServiceConnRef ref);
	BOOL NBHttpServiceConn_isCertSignedByCA(STNBHttpServiceConnRef ref);
	BOOL NBHttpServiceConn_getCert(STNBHttpServiceConnRef ref, STNBX509* dst, BOOL* dstIsSignedByCA);
	BOOL NBHttpServiceConn_isSocket(STNBHttpServiceConnRef ref, STNBSocketRef socket);

	//

	BOOL NBHttpServiceConn_startListeningOwningSocket(STNBHttpServiceConnRef ref, const STNBHttpConnCfg* limits, STNBIOPollsterSyncRef pollSync, STNBStopFlagRef* parentStopFlag, STNBSocketRef socket, STNBX509* optCACert, STNBSslContextRef optSslContext, STNBHttpStatsRef parentProvider, STNBHttpServiceConnLstnrItf* itf, void* usrData);
	void NBHttpServiceConn_stopFlag(STNBHttpServiceConnRef ref);
	BOOL NBHttpServiceConn_isBusy(STNBHttpServiceConnRef ref);

	//Commands
	BOOL NBHttpServiceConn_statsFlushStart(STNBHttpServiceConnRef ref, const UI32 iSeq);
	BOOL NBHttpServiceConn_statsFlushIsPend(STNBHttpServiceConnRef ref, const UI32 iSeq);
	void NBHttpServiceConn_statsGet(STNBHttpServiceConnRef ref, STNBHttpStatsData* dst, const BOOL resetAccum);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
