#ifndef NB_HTTP_SERVICE_STATS_H
#define NB_HTTP_SERVICE_STATS_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBStructMap.h"
#include "nb/net/NBHttpHeader.h"
#include "nb/net/NBHttpBody.h"
#include "nb/net/NBHttpMessage.h"
#include "nb/net/NBHttpCfg.h"
#include "nb/crypto/NBX509.h"
#include "nb/ssl/NBSslContext.h"
//

#ifdef __cplusplus
extern "C" {
#endif

	//Traffic

	typedef struct STNBHttpServiceTraffic_ {
		UI64	in;
		UI64	out;
	} STNBHttpServiceTraffic;
	
	const STNBStructMap* NBHttpServiceTraffic_getSharedStructMap(void);
	
	//Flow

	typedef struct STNBHttpServiceFlow_ {
		UI64					connsIn;		//Incoming conections
		UI64					connsRejects;	//Connections rejected
		UI64					requests;		//Requests
		UI64					secsIdle;		//Secs iddle
		STNBHttpServiceTraffic	bytes;			//Bytes sent/received
	} STNBHttpServiceFlow;
	
	const STNBStructMap* NBHttpServiceFlow_getSharedStructMap(void);
	
	//Responses

	typedef struct STNBHttpStatsResp_ {
		SI32	code;		//-1 for reject/disconnection
		char*	reason;
		UI64	count;
	} STNBHttpStatsResp;

	const STNBStructMap* NBHttpStatsResp_getSharedStructMap(void);

	//Requests

	typedef struct STNBHttpStatsReq_ {
		char*				target;
		UI64				count;
		STNBHttpStatsResp*	resps;
		UI32				respsSz;
	} STNBHttpStatsReq;

	const STNBStructMap* NBHttpStatsReq_getSharedStructMap(void);

	//Stats data
	
	typedef struct STNBHttpStatsData_ {
		STNBHttpServiceFlow	flow;
		STNBHttpStatsReq*	reqs;
		UI32				reqsSz;
	} STNBHttpStatsData;

	const STNBStructMap* NBHttpStatsData_getSharedStructMap(void);

	//Stats data parts

	typedef struct STNBHttpStatsDataParts_ {
		BOOL		conns;		//connections in
		BOOL		bytesIn;	//bytes in and out
		BOOL		bytesOut;	//bytes in and out
		BOOL		requests;	//requests count
	} STNBHttpStatsDataParts;

	const STNBStructMap* NBHttpStatsDataParts_getSharedStructMap(void);

	//

	void NBHttpStatsData_accumConnIn(STNBHttpStatsData* obj, const BOOL wasRejected);
	void NBHttpStatsData_accumBytes(STNBHttpStatsData* obj, const UI64 in, const UI64 out);
	void NBHttpStatsData_accumRequest(STNBHttpStatsData* obj, const char* target);
	void NBHttpStatsData_accumResponse(STNBHttpStatsData* obj, const char* target, const SI32 code, const char* reason);
	void NBHttpStatsData_accumData(STNBHttpStatsData* obj, const STNBHttpStatsData* other);
	STNBHttpStatsReq* NBHttpStatsData_getTargetData(STNBHttpStatsData* obj, const char* pTarget, const BOOL createIfNecesary);
	//Service stats

	NB_OBJREF_HEADER(NBHttpStats)
	
	//
	BOOL NBHttpStats_setCfg(STNBHttpStatsRef ref, const STNBHttpLimitsCfg* limitsConns, const STNBHttpStatsDataParts* ignoreParts, STNBHttpStatsRef prntStats);
	BOOL NBHttpStats_setLimits(STNBHttpStatsRef ref, const STNBHttpLimitsCfg* limitsConns);
	BOOL NBHttpStats_setIgnoreParts(STNBHttpStatsRef ref, const STNBHttpStatsDataParts* ignoreParts);
	BOOL NBHttpStats_setParentStats(STNBHttpStatsRef ref, STNBHttpStatsRef prntStats);
	BOOL NBHttpStats_getPartsCfg(STNBHttpStatsRef ref, STNBHttpStatsDataParts* dstIgnoreParts, STNBHttpStatsDataParts* dstAtomicParts);
	//
	void NBHttpStats_setData(STNBHttpStatsRef ref, const STNBHttpStatsData* src);
	void NBHttpStats_getData(STNBHttpStatsRef ref, STNBHttpStatsData* dst, const BOOL reset);
	void NBHttpStats_flush(STNBHttpStatsRef ref);
	void NBHttpStats_empty(STNBHttpStatsRef ref);
	
	//
	void NBHttpStats_accumConnIn(STNBHttpStatsRef ref, const BOOL wasRejected);
	void NBHttpStats_accumBytes(STNBHttpStatsRef ref, const UI64 in, const UI64 out);
	void NBHttpStats_accumRequest(STNBHttpStatsRef ref, const char* target);
	void NBHttpStats_accumResponse(STNBHttpStatsRef ref, const char* target, const SI32 code, const char* reason);
	void NBHttpStats_accumData(STNBHttpStatsRef ref, STNBHttpStatsData* data);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
