//
//  NBDatetime.h
//  nbframework
//
//  Created by Marcos Ortega on 8/31/18.
//

#ifndef NB_DATETIME_H
#define NB_DATETIME_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBStructMap.h"

#ifdef __cplusplus
extern "C" {
#endif

#	define NB_DATE_TIME_STR_BUFF_SIZE	32 //YYYY-MM-DD hh:mm:ss.nnn\0
	
    typedef enum STNBTimestampType_ {
        STNBTimestampType_Undef = 0,
        STNBTimestampType_UTC,
        STNBTimestampType_Monotonic,
        STNBTimestampType_MonotonicFast,
        STNBTimestampType_LocalTime,
        //
        STNBTimestampType_Count
    } STNBTimestampType;

#   define NBTimestampMicro_Zero       { 0, 0, STNBTimestampType_Undef }

	typedef struct STNBTimestampMicro_ {
		UI64                timestamp;	//secs
		UI32                usecs;		//microseconds (1,000,000 per sec)
        STNBTimestampType   type;       //type
	} STNBTimestampMicro;

    //realtime clock, can jump when system-time is updated (by user, NTP, ...)
    STNBTimestampMicro  NBTimestampMicro_getUTC(void);
    //monotonic clocks (never jumps, can wrap arround)
    STNBTimestampMicro  NBTimestampMicro_getMonotonic(void);
    STNBTimestampMicro  NBTimestampMicro_getMonotonicFast(void);

	//Diff
    BOOL NBTimestampMicro_isGreaterTo(const STNBTimestampMicro* base, const STNBTimestampMicro* next);
	SI64 NBTimestampMicro_getDiffInSecs(const STNBTimestampMicro* base, const STNBTimestampMicro* next);
	SI64 NBTimestampMicro_getDiffInMs(const STNBTimestampMicro* base, const STNBTimestampMicro* next);
	SI64 NBTimestampMicro_getDiffInUs(const STNBTimestampMicro* base, const STNBTimestampMicro* next);
    //
    BOOL NBTimestampMicro_isGreaterToNow(const STNBTimestampMicro* base);   //compares the timestamp with its equivalent 'now'
    SI64 NBTimestampMicro_getDiffNowInSecs(const STNBTimestampMicro* base); //compares the timestamp with its equivalent 'now'
    SI64 NBTimestampMicro_getDiffNowInMs(const STNBTimestampMicro* base);   //compares the timestamp with its equivalent 'now'
    SI64 NBTimestampMicro_getDiffNowInUs(const STNBTimestampMicro* base);   //compares the timestamp with its equivalent 'now'
    //
    typedef struct STNBDatetime_ {
		UI16	year;
		UI8		month;
		UI8		day;
		UI8		hour;
		UI8		min;
		UI8		sec;
		UI32	ms;
	} STNBDatetime;
	
	const STNBStructMap* NBDatetime_getSharedStructMap(void);

	UI64				NBDatetime_getCurUTCTimestamp(void);
	UI64				NBDatetime_getUTCTimestampFromDate(const STNBDatetime datetime);
	UI64				NBDatetime_getLocalTimestampFromDate(const STNBDatetime datetime);
	STNBDatetime		NBDatetime_getCurUTC(void);
	STNBDatetime		NBDatetime_getUTCFromTimestamp(UI64 timestamp);
	STNBDatetime		NBDatetime_getLocalFromTimestamp(UI64 timestamp);
	STNBDatetime		NBDatetime_getCurLocal(void);

	//Format str
	void				NBDatetime_setSqlStr(const STNBDatetime t, char* dst, const UI32 dstSz);
	void				NBDatetime_getCurLocalSqlStr(char* dst, const UI32 dstSz);

	//validation
	UI8					NBDatetime_daysInMonth(const UI32 year, const UI8 month);

    //ops
	STNBDatetime		NBDatetime_addYears(const STNBDatetime* ref, const SI32 amm);
	STNBDatetime		NBDatetime_addMonths(const STNBDatetime* ref, const SI32 amm);
	STNBDatetime		NBDatetime_addDays(const STNBDatetime* ref, const SI32 amm);
	STNBDatetime		NBDatetime_addHours(const STNBDatetime* ref, const SI32 amm);
	STNBDatetime		NBDatetime_addMinutes(const STNBDatetime* ref, const SI32 amm);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
