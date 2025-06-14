
#ifndef NB_RTSPCLIENTSTATS_H
#define NB_RTSPCLIENTSTATS_H

#include "nb/NBFrameworkDefs.h"
//
#include "nb/net/NBRtpClientStats.h"
#include "nb/net/NBRtpQueue.h"
#include "nb/core/NBLog.h"
#include "nb/core/NBStructMap.h"

#ifdef __cplusplus
extern "C" {
#endif

	//Rtcp

	typedef struct STNBRtcpStatsCfg_ {
		ENNBLogLevel	statsLevel;
	} STNBRtcpStatsCfg;

	const STNBStructMap* NBRtcpStatsCfg_getSharedStructMap(void);

	//Rtsp

	typedef struct STNBRtspStatsCfg_ {
		ENNBLogLevel		statsLevel;
		STNBRtpStatsCfg		rtp;
		STNBRtcpStatsCfg	rtcp;
	} STNBRtspStatsCfg;

	const STNBStructMap* NBRtspStatsCfg_getSharedStructMap(void);

    // NBRtspClientStatsBuffer

    typedef struct STNBRtspClientStatsBuffer_ {
        UI32    count;
        UI64    bytesCount;
    } STNBRtspClientStatsBuffer;

    const STNBStructMap* NBRtspClientStatsBuffer_getSharedStructMap(void);

	// NBRtspClientStatsBuffers

	typedef struct STNBRtspClientStatsBuffers_ {
		UI32	count;
		UI64	bytesCount;
        STNBRtspClientStatsBuffer wired;
	} STNBRtspClientStatsBuffers;

    const STNBStructMap* NBRtspClientStatsBuffers_getSharedStructMap(void);

    //NBRtspClientStatsState

	typedef struct STNBRtspClientStatsState_ {
		STNBRtspClientStatsBuffers		buffs;
		STNBRtpClientStatsState			rtp;
		UI64							updCalls;
	} STNBRtspClientStatsState;

    const STNBStructMap* NBRtspClientStatsState_getSharedStructMap(void);

    //NBRtspClientStatsData

	typedef struct STNBRtspClientStatsData_ {
		STNBRtspClientStatsState	loaded;		//loaded
		STNBRtspClientStatsState	accum;		//accum
		STNBRtspClientStatsState	total;		//total
	} STNBRtspClientStatsData;

    const STNBStructMap* NBRtspClientStatsData_getSharedStructMap(void);

	//

	typedef struct STNBRtspClientStats_ {
		void*		opaque;
	} STNBRtspClientStats;
	
	//Factory
	void NBRtspClientStats_init(STNBRtspClientStats* obj);
	void NBRtspClientStats_retain(STNBRtspClientStats* obj);
	void NBRtspClientStats_release(STNBRtspClientStats* obj);

	//SubRtp
	STNBRtpClientStats*			NBRtspClientStats_getRtpStats(STNBRtspClientStats* obj);
	STNBRtpClientStatsData		NBRtspClientStats_getRtpData(STNBRtspClientStats* obj, const BOOL resetAccum);
	//Data
	STNBRtspClientStatsData		NBRtspClientStats_getData(STNBRtspClientStats* obj, const BOOL resetAccum);
	void NBRtspClientStats_concat(STNBRtspClientStats* obj, const STNBRtspStatsCfg* cfg, const BOOL loaded, const BOOL accum, const BOOL total, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, STNBString* dst, const BOOL resetAccum);
	void NBRtspClientStats_concatData(const STNBRtspClientStatsData* obj, const STNBRtspStatsCfg* cfg, const BOOL loaded, const BOOL accum, const BOOL total, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, STNBString* dst);
	void NBRtspClientStats_concatState(const STNBRtspClientStatsState* obj, const STNBRtspStatsCfg* cfg, const char* prefixFirst, const char* prefixOthers, const BOOL ignoreEmpties, STNBString* dst);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
