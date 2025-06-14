
#include "nb/NBFrameworkPch.h"
//Complementary PCH
#ifdef _WIN32
//#	define WIN32_LEAN_AND_MEAN
#	include <windows.h> //for SYSTEMTIME
#else
#	include <sys/time.h>	//for struct timeval, gettimeofday()
#endif
#include <time.h>			//for struct tm
//
#include "nb/core/NBDatetime.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBMngrStructMaps.h"

STNBStructMapsRec NBDatetime_sharedStructMap = STNBStructMapsRec_empty;

#ifdef _WIN32
//https://learn.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancefrequency
//The frequency of the performance counter is fixed at system boot and is consistent across all processors.
//Therefore, the frequency need only be queried upon application initialization, and the result can be cached.
static LARGE_INTEGER _winFreq = { 0 };
#endif


const STNBStructMap* NBDatetime_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBDatetime_sharedStructMap);
	if(NBDatetime_sharedStructMap.map == NULL){
		STNBDatetime s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBDatetime);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addUIntM(map, s, year);
		NBStructMap_addUIntM(map, s, month);
		NBStructMap_addUIntM(map, s, day);
		NBStructMap_addUIntM(map, s, hour);
		NBStructMap_addUIntM(map, s, min);
		NBStructMap_addUIntM(map, s, sec);
		NBStructMap_addUIntM(map, s, ms);
		NBDatetime_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBDatetime_sharedStructMap);
	return NBDatetime_sharedStructMap.map;
}


UI64 NBDatetime_getCurUTCTimestamp(void){
#	ifdef _WIN32
	/*SYSTEMTIME timeUTC; FILETIME timeFile;
	ULARGE_INTEGER ull;
	GetSystemTime(&timeUTC);
	SystemTimeToFileTime(&timeUTC, &timeFile);
	ull.LowPart = timeFile.dwLowDateTime;
	ull.HighPart = timeFile.dwHighDateTime;
	return (UI64)(ull.QuadPart / 10000000ULL - 11644473600ULL);*/
	return (UI64)time(NULL); //time() es UTC
#	else
	return (UI64)time(NULL); //time() es UTC
#	endif
}

/* FILETIME of Jan 1 1970 00:00:00. */
static const UI64 _nbEpoch = 116444736000000000LLU;

STNBTimestampMicro NBTimestampMicro_getUTC(void){
    STNBTimestampMicro r = NBTimestampMicro_Zero;
#	ifdef _WIN32
	FILETIME    file_time;
	SYSTEMTIME  system_time;
	ULARGE_INTEGER ularge;
	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	ularge.LowPart	= file_time.dwLowDateTime;
	ularge.HighPart	= file_time.dwHighDateTime;
	r.timestamp		= (UI64)((ularge.QuadPart - _nbEpoch) / 10000000L);
	r.usecs			= (UI32)(system_time.wMilliseconds * 1000);
    r.type          = STNBTimestampType_UTC;
#	else
	struct timeval tv;
	if(0 == gettimeofday(&tv, NULL)){
		r.timestamp	= tv.tv_sec;
		r.usecs		= tv.tv_usec;
        r.type      = STNBTimestampType_UTC;
	}
#	endif
    return r;
}


//monotonic clocks (never jumps, can wrap arround)
STNBTimestampMicro NBTimestampMicro_getMonotonic(void){
    STNBTimestampMicro r = NBTimestampMicro_Zero;
#   ifdef _WIN32
    LARGE_INTEGER now = { 0 };
    if(_winFreq.QuadPart == 0){
        //https://learn.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancefrequency
        //The frequency of the performance counter is fixed at system boot and is consistent across all processors.
        //Therefore, the frequency need only be queried upon application initialization, and the result can be cached.
        if(!QueryPerformanceFrequency(&_winFreq)){
            //error
        }
    }
    if(_winFreq.QuadPart > 0 && QueryPerformanceCounter(&now)){
        r.timestamp = now.QuadPart / _winFreq.QuadPart;
        r.usecs     = now.QuadPart % _winFreq.QuadPart;
        r.type      = STNBTimestampType_Monotonic;
        if(_winFreq.QuadPart >= 1000000){
            r.usecs /= (_winFreq.QuadPart / 1000000);
        } else {
            r.usecs *= (1000000 / _winFreq.QuadPart);
        }
    }
#   else
    struct timespec tv;
    if(0 == clock_gettime(CLOCK_MONOTONIC, &tv)){
        r.timestamp = tv.tv_sec;
        r.usecs     = (UI32)(tv.tv_nsec / 1000);
        r.type      = STNBTimestampType_Monotonic;
    }
#   endif
    return r;
}

//monotonic clocks (never jumps, can wrap arround)
STNBTimestampMicro NBTimestampMicro_getMonotonicFast(void){
    STNBTimestampMicro r = NBTimestampMicro_Zero;
#   ifdef _WIN32
    LARGE_INTEGER now = { 0 };
    if(_winFreq.QuadPart == 0){
        //https://learn.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancefrequency
        //The frequency of the performance counter is fixed at system boot and is consistent across all processors.
        //Therefore, the frequency need only be queried upon application initialization, and the result can be cached.
        if(!QueryPerformanceFrequency(&_winFreq)){
            //error
        }
    }
    if(_winFreq.QuadPart > 0 && QueryPerformanceCounter(&now)){
        r.timestamp = now.QuadPart / _winFreq.QuadPart;
        r.usecs     = now.QuadPart % _winFreq.QuadPart;
        r.type      = STNBTimestampType_MonotonicFast;
        if(_winFreq.QuadPart >= 1000000){
            r.usecs /= (_winFreq.QuadPart / 1000000);
        } else {
            r.usecs *= (1000000 / _winFreq.QuadPart);
        }
    }
#   else
    struct timespec tv;
#   ifdef CLOCK_MONOTONIC_COARSE
    if(0 == clock_gettime(CLOCK_MONOTONIC_COARSE, &tv)){
        r.timestamp = tv.tv_sec;
        r.usecs     = (UI32)(tv.tv_nsec / 1000);
        r.type      = STNBTimestampType_MonotonicFast;
    }
#   else
    if(0 == clock_gettime(CLOCK_MONOTONIC, &tv)){
        r.timestamp = tv.tv_sec;
        r.usecs     = (UI32)(tv.tv_nsec / 1000);
        r.type      = STNBTimestampType_MonotonicFast;
    }
#   endif
#   endif
    return r;
}

//Diff

#define NBTimestampMicro_isGreaterTo_(BASE, NXT) ((BASE)->timestamp > (NXT)->timestamp || ((BASE)->timestamp == (NXT)->timestamp && (BASE)->usecs > (NXT)->usecs))

BOOL NBTimestampMicro_isGreaterTo(const STNBTimestampMicro* base, const STNBTimestampMicro* next){
    return NBTimestampMicro_isGreaterTo_(base, next);
}

SI64 NBTimestampMicro_getDiffInSecs(const STNBTimestampMicro* base, const STNBTimestampMicro* next){
	if(base != next){
		if(next == NULL){
			return base->timestamp + (base->usecs / 1000000ULL);
		} else if(base == NULL){
			return -(next->timestamp + (next->usecs / 1000000ULL));
		} else if(base->timestamp < next->timestamp || (base->timestamp == next->timestamp && base->usecs <= next->usecs)){
			//ahead
			if(base->timestamp == next->timestamp){
				return (next->usecs - base->usecs) / 1000000ULL;
			} else {
				return (next->timestamp - base->timestamp - 1) + (((1000000ULL - base->usecs) + (next->usecs)) / 1000000ULL);
			}
		} else {
			//backwards
			if(base->timestamp == next->timestamp){
				return -((base->usecs - next->usecs) / 1000000ULL);
			} else {
				return -((base->timestamp - next->timestamp - 1) + (((1000000ULL - next->usecs) + (base->usecs)) / 1000000ULL));
			}
		}
	}
	return 0;
}

SI64 NBTimestampMicro_getDiffInMs(const STNBTimestampMicro* base, const STNBTimestampMicro* next){
	if(base != next){
		if(next == NULL){
			return (base->timestamp * 1000ULL) + (base->usecs / 1000ULL);
		} else if(base == NULL){
			return -((next->timestamp * 1000ULL) + (next->usecs / 1000ULL));
		} else if(base->timestamp < next->timestamp || (base->timestamp == next->timestamp && base->usecs <= next->usecs)){
			//ahead
			if(base->timestamp == next->timestamp){
				return (next->usecs - base->usecs) / 1000ULL;
			} else {
				return ((next->timestamp - base->timestamp - 1) * 1000ULL) + (((1000000ULL - base->usecs) + (next->usecs)) / 1000ULL);
			}
		} else {
			//backwards
			if(base->timestamp == next->timestamp){
				return -((base->usecs - next->usecs) / 1000ULL);
			} else {
				return -(((base->timestamp - next->timestamp - 1) * 1000ULL) + (((1000000ULL - next->usecs) + (base->usecs)) / 1000ULL));
			}
		}
	}
	return 0;
}

SI64 NBTimestampMicro_getDiffInUs(const STNBTimestampMicro* base, const STNBTimestampMicro* next){
	if(base != next){
		if(next == NULL){
			return (base->timestamp * 1000000ULL) + base->usecs;
		} else if(base == NULL){
			return -((next->timestamp * 1000000ULL) + next->usecs);
		} else if(base->timestamp < next->timestamp || (base->timestamp == next->timestamp && base->usecs <= next->usecs)){
			//ahead
			if(base->timestamp == next->timestamp){
				return next->usecs - base->usecs;
			} else {
				return ((next->timestamp - base->timestamp - 1) * 1000000ULL) + (1000000ULL - base->usecs) + (next->usecs);
			}
		} else {
			//backwards
			if(base->timestamp == next->timestamp){
				return -((base->usecs - next->usecs) / 1000ULL);
			} else {
				return -(((base->timestamp - next->timestamp - 1) * 1000000ULL) + (1000000ULL - next->usecs) + (base->usecs));
			}
		}
	}
	return 0;
}

//

#define NBTimestampMicro_getEquivalentNow(BASE, DST) \
    switch((BASE)->type) { \
        case STNBTimestampType_Monotonic: \
        (*DST) = NBTimestampMicro_getMonotonic(); \
        break; \
    case STNBTimestampType_MonotonicFast: \
        (*DST) = NBTimestampMicro_getMonotonicFast(); \
        break; \
    default: \
        (*DST) = NBTimestampMicro_getUTC(); \
        break; \
    }
    

BOOL NBTimestampMicro_isGreaterToNow(const STNBTimestampMicro* base){   //compares the timestamp with its equivalent 'now'
    STNBTimestampMicro now = NBTimestampMicro_Zero;
    NBTimestampMicro_getEquivalentNow(base, &now);
    return NBTimestampMicro_isGreaterTo_(base, &now);
}

SI64 NBTimestampMicro_getDiffNowInSecs(const STNBTimestampMicro* base){
    STNBTimestampMicro now = NBTimestampMicro_Zero;
    NBTimestampMicro_getEquivalentNow(base, &now);
    return NBTimestampMicro_getDiffInSecs(base, &now);
}

SI64 NBTimestampMicro_getDiffNowInMs(const STNBTimestampMicro* base){
    STNBTimestampMicro now = NBTimestampMicro_Zero;
    NBTimestampMicro_getEquivalentNow(base, &now);
    return NBTimestampMicro_getDiffInMs(base, &now);
}

SI64 NBTimestampMicro_getDiffNowInUs(const STNBTimestampMicro* base){
    STNBTimestampMicro now = NBTimestampMicro_Zero;
    NBTimestampMicro_getEquivalentNow(base, &now);
    return NBTimestampMicro_getDiffInUs(base, &now);
}

//
STNBDatetime NBDatetime_getCurUTC(void){
	STNBDatetime r;
#	ifdef _WIN32
	time_t now = time(NULL); //es UTC
	struct tm t_st; gmtime_s(&t_st, &now); //es UTC
	r.year	= t_st.tm_year + 1900;
	r.month	= t_st.tm_mon + 1;
	r.day	= (UI8)t_st.tm_mday;
	r.hour	= (UI8)t_st.tm_hour;
	r.min	= (UI8)t_st.tm_min;
	r.sec	= (UI8)t_st.tm_sec;
	r.ms	= 0;
	/*SYSTEMTIME t_st; GetSystemTime(&t_st);
	r.year	= (UI16)t_st.wYear;
	r.month	= (UI8)t_st.wMonth;
	r.day	= (UI8)t_st.wDay;
	r.hour	= (UI8)t_st.wHour;
	r.min	= (UI8)t_st.wMinute;
	r.sec	= (UI8)t_st.wSecond;
	r.ms	= (UI32)t_st.wMilliseconds;*/
	PRINTF_INFO("NBDatetime_getCurUTC: %d-%d-%d %d:%d:%d.%d.\n", r.year, r.month, r.day, r.hour, r.min, r.sec, r.ms);
#	else
	time_t now = time(NULL); //es UTC
	struct tm t_st; gmtime_r(&now, &t_st); //es UTC
	r.year	= t_st.tm_year + 1900;
	r.month	= t_st.tm_mon + 1;
	r.day	= t_st.tm_mday;
	r.hour	= t_st.tm_hour;
	r.min	= t_st.tm_min;
	r.sec	= t_st.tm_sec;
	r.ms	= 0;
#	endif
	return r;
}


STNBDatetime NBDatetime_getUTCFromTimestamp(UI64 timestamp){
	STNBDatetime r;
#	ifdef _WIN32
	time_t tmstp = (time_t)timestamp;
	struct tm t_st; gmtime_s(&t_st, &tmstp);
	r.year	= t_st.tm_year + 1900;
	r.month	= t_st.tm_mon + 1;
	r.day	= (UI8)t_st.tm_mday;
	r.hour	= (UI8)t_st.tm_hour;
	r.min	= (UI8)t_st.tm_min;
	r.sec	= (UI8)t_st.tm_sec;
	r.ms	= 0;
	PRINTF_INFO("NBDatetime_getUTCFromTimestamp: %d-%d-%d %d:%d:%d.%d.\n", r.year, r.month, r.day, r.hour, r.min, r.sec, r.ms);
#	else
	time_t tmstp = (time_t)timestamp;
	struct tm t_st; gmtime_r(&tmstp, &t_st);
	r.year	= t_st.tm_year + 1900;
	r.month	= t_st.tm_mon + 1;
	r.day	= t_st.tm_mday;
	r.hour	= t_st.tm_hour;
	r.min	= t_st.tm_min;
	r.sec	= t_st.tm_sec;
	r.ms	= 0;
#	endif
	return r;
}

STNBDatetime NBDatetime_getLocalFromTimestamp(UI64 timestamp){
	STNBDatetime r;
#	ifdef _WIN32
	struct tm t_st;
	const time_t tmstp = (time_t)timestamp;
	localtime_s(&t_st, &tmstp);
	r.year	= t_st.tm_year + 1900;
	r.month	= t_st.tm_mon + 1;
	r.day	= (UI8)t_st.tm_mday;
	r.hour	= (UI8)t_st.tm_hour;
	r.min	= (UI8)t_st.tm_min;
	r.sec	= (UI8)t_st.tm_sec;
	r.ms	= 0;
	PRINTF_INFO("NBDatetime_getLocalFromTimestamp: %d-%d-%d %d:%d:%d.%d.\n", r.year, r.month, r.day, r.hour, r.min, r.sec, r.ms);
	/*NBASSERT(FALSE)
	SYSTEMTIME t_st; GetLocalTime(&t_st);
	r.year	= (UI16)t_st.wYear;
	r.month	= (UI8)t_st.wMonth;
	r.day	= (UI8)t_st.wDay;
	r.hour	= (UI8)t_st.wHour;
	r.min	= (UI8)t_st.wMinute;
	r.sec	= (UI8)t_st.wSecond;
	r.ms	= (UI32)t_st.wMilliseconds;*/
#	else
	time_t tmstp = (time_t)timestamp;
	struct tm* t_st = localtime(&tmstp);
	r.year	= t_st->tm_year + 1900;
	r.month	= t_st->tm_mon + 1;
	r.day	= t_st->tm_mday;
	r.hour	= t_st->tm_hour;
	r.min	= t_st->tm_min;
	r.sec	= t_st->tm_sec;
	r.ms	= 0;
#	endif
	return r;
}

UI64 NBDatetime_getUTCTimestampFromDate(const STNBDatetime datetime){
	UI64 r = 0;
	#	ifdef _WIN32
		struct tm t_st;
		NBMemory_setZeroSt(t_st, struct tm);
		t_st.tm_year 	= datetime.year - 1900;
		t_st.tm_mon		= datetime.month - 1;
		t_st.tm_mday	= datetime.day;
		t_st.tm_hour	= datetime.hour;
		t_st.tm_min		= datetime.min;
		t_st.tm_sec		= datetime.sec;
		r				= _mkgmtime64(&t_st);
	#	else
		struct tm t_st;
		NBMemory_setZeroSt(t_st, struct tm);
		t_st.tm_year 	= datetime.year - 1900;
		t_st.tm_mon		= datetime.month - 1;
		t_st.tm_mday	= datetime.day;
		t_st.tm_hour	= datetime.hour;
		t_st.tm_min		= datetime.min;
		t_st.tm_sec		= datetime.sec;
		r				= timegm(&t_st);
	#	endif
	return r;
}

UI64 NBDatetime_getLocalTimestampFromDate(const STNBDatetime datetime){
	UI64 r = 0;
#	ifdef _WIN32
	struct tm t_st;
	NBMemory_setZeroSt(t_st, struct tm);
	t_st.tm_year 	= datetime.year - 1900;
	t_st.tm_mon		= datetime.month - 1;
	t_st.tm_mday	= datetime.day;
	t_st.tm_hour	= datetime.hour;
	t_st.tm_min		= datetime.min;
	t_st.tm_sec		= datetime.sec;
	r				= mktime(&t_st);
#	else
	struct tm t_st;
	NBMemory_setZeroSt(t_st, struct tm);
	t_st.tm_year 	= datetime.year - 1900;
	t_st.tm_mon		= datetime.month - 1;
	t_st.tm_mday	= datetime.day;
	t_st.tm_hour	= datetime.hour;
	t_st.tm_min		= datetime.min;
	t_st.tm_sec		= datetime.sec;
	r				= mktime(&t_st);
#	endif
	return r;
}


STNBDatetime NBDatetime_getCurLocal(void){
	STNBDatetime r;
#	ifdef _WIN32
	struct tm t_st;
	const time_t now = time(NULL);
	localtime_s(&t_st , &now);
	r.year	= t_st.tm_year + 1900;
	r.month	= t_st.tm_mon + 1;
	r.day	= (UI8)t_st.tm_mday;
	r.hour	= (UI8)t_st.tm_hour;
	r.min	= (UI8)t_st.tm_min;
	r.sec	= (UI8)t_st.tm_sec;
	r.ms	= 0;
	/*SYSTEMTIME t_st; GetLocalTime(&t_st);
	r.year	= (UI16)t_st.wYear;
	r.month	= (UI8)t_st.wMonth;
	r.day	= (UI8)t_st.wDay;
	r.hour	= (UI8)t_st.wHour;
	r.min	= (UI8)t_st.wMinute;
	r.sec	= (UI8)t_st.wSecond;
	r.ms	= (UI32)t_st.wMilliseconds;*/
#	else
	time_t now = time(NULL);
	struct tm* t_st = localtime(&now);
	r.year	= t_st->tm_year + 1900;
	r.month	= t_st->tm_mon + 1;
	r.day	= t_st->tm_mday;
	r.hour	= t_st->tm_hour;
	r.min	= t_st->tm_min;
	r.sec	= t_st->tm_sec;
	r.ms	= 0;
#	endif
	return r;
}

//

UI32 NBDatetime_setDigitStr(const UI32 value, const UI32 digitsMin, char* dst, const UI32 dstSz){
	UI32 r = 0, m10 = 1, digits = 1, v = value, vd = 0;
	//Calculate digits
	while(v >= (m10 * 10) || digits < digitsMin){
		m10 *= 10; digits++;
	}
	//Add digits
	while((r + 2) < dstSz && digits > 0){
		vd			= (v / m10);	NBASSERT(vd >= 0 && vd <= 9)
		v			-= (vd * m10);	NBASSERT(v >= 0 && v <= value)
		dst[r++]	= ('0' + vd);
		m10 /= 10; digits--;
	}
	//
	return r;
}
	
void NBDatetime_setSqlStr(const STNBDatetime t, char* dst, const UI32 dstSz){
	BOOL keepAdd = TRUE;
	UI32 i = 0;
	//Year
	if((i + 4) < dstSz && keepAdd){ i += NBDatetime_setDigitStr(t.year, 4, &dst[i], (dstSz - i)); } else { keepAdd = FALSE; }
	//Separator
	if((i + 1) < dstSz && keepAdd){ dst[i++] = '-'; } else { keepAdd = FALSE; }
	//Month
	if((i + 2) < dstSz && keepAdd){ i += NBDatetime_setDigitStr(t.month, 2, &dst[i], (dstSz - i)); } else { keepAdd = FALSE; }
	//Separator
	if((i + 1) < dstSz && keepAdd){ dst[i++] = '-'; } else { keepAdd = FALSE; }
	//Day
	if((i + 2) < dstSz && keepAdd){ i += NBDatetime_setDigitStr(t.day, 2, &dst[i], (dstSz - i)); } else { keepAdd = FALSE; }
	//Separator
	if((i + 1) < dstSz && keepAdd){ dst[i++] = ' '; } else { keepAdd = FALSE; }
	//Hour
	if((i + 2) < dstSz && keepAdd){ i += NBDatetime_setDigitStr(t.hour, 2, &dst[i], (dstSz - i)); } else { keepAdd = FALSE; }
	//Separator
	if((i + 1) < dstSz && keepAdd){ dst[i++] = ':'; } else { keepAdd = FALSE; }
	//Minute
	if((i + 2) < dstSz && keepAdd){ i += NBDatetime_setDigitStr(t.min, 2, &dst[i], (dstSz - i)); } else { keepAdd = FALSE; }
	//Separator
	if((i + 1) < dstSz && keepAdd){ dst[i++] = ':'; } else { keepAdd = FALSE; }
	//Secs
	if((i + 2) < dstSz && keepAdd){ i += NBDatetime_setDigitStr(t.sec, 2, &dst[i], (dstSz - i)); } else { keepAdd = FALSE; }
	//Millisecs
	if(t.ms > 0){
		//Separator
		if((i + 1) < dstSz && keepAdd){ dst[i++] = '.'; } else { keepAdd = FALSE; }
		//Secs
		if((i + 3) < dstSz && keepAdd){ i += NBDatetime_setDigitStr(t.ms, 3, &dst[i], (dstSz - i)); } else { keepAdd = FALSE; }
	}
	//End of string
	if(dstSz > 0){
		dst[(i < dstSz ? i : (dstSz - 1))] = '\0';
	}
}

void NBDatetime_getCurLocalSqlStr(char* dst, const UI32 dstSz){
	NBDatetime_setSqlStr(NBDatetime_getCurLocal(), dst, dstSz);
}

UI8 NBDatetime_daysInMonth(const UI32 year, const UI8 month){
	UI8 r = 0;
	switch(month) {
		case 1: r = 31; break;
		case 2: r = (((year % 400) == 0 || ((year % 4) == 0 && (year % 100) != 0)) ? 29 : 28); break;
		case 3: r = 31; break;
		case 4: r = 30; break;
		case 5: r = 31; break;
		case 6: r = 30; break;
		case 7: r = 31; break;
		case 8: r = 31; break;
		case 9: r = 30; break;
		case 10: r = 31; break;
		case 11: r = 30; break;
		case 12: r = 31; break;
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		default:
			NBASSERT(FALSE) 
			break;
#		endif
	}
	return r;
}

STNBDatetime NBDatetime_addYears(const STNBDatetime* ref, const SI32 amm){
	STNBDatetime r = *ref;
	if(amm >= 0){
		r.year += (UI16)amm;
	} else {
		NBASSERT(FALSE) //Pending to test with Internet
	}
	return r;
}

STNBDatetime NBDatetime_addMonths(const STNBDatetime* ref, const SI32 amm){
	STNBDatetime r = *ref;
	if(amm >= 0){
		SI32 months = amm;
		//Full years
		{
			const SI32 fullYears = (months / 12);
			if(fullYears > 0){
				r.year += (UI16)fullYears;
				months -= (fullYears * 12);
			} NBASSERT(months < 12)
		}
		//Remaining
		{
			r.month += (UI8)months;
			if(r.month > 12){
				r.year++;
				r.month -= 12;
			}
			//Truncate day
			{
				const SI32 curMonthDays = NBDatetime_daysInMonth(r.year, r.month);
				if(r.day > curMonthDays) r.day = (UI8)curMonthDays;
			}
		}
	} else {
		NBASSERT(FALSE) //Pending to test with Internet
	}
	return r;
}

STNBDatetime NBDatetime_addDays(const STNBDatetime* ref, const SI32 amm){
	STNBDatetime r = *ref;
	if(amm >= 0){
		const SI32 curMonthDays = NBDatetime_daysInMonth(r.year, r.month);
		const SI32 curMonthDaysRem = (curMonthDays - r.day);
		if(amm <= curMonthDaysRem){
			//PRINTF_INFO("(curMonthDaysRem <= amm): r.day += amm = %d.\n", (r.day + amm));
			r.day += (UI8)amm;
		} else {
			const SI32 willRemain = (amm - curMonthDaysRem - 1);
			if(r.month >= 12){
				r.year++;
				r.month = 1;
				//PRINTF_INFO("Moving to next year january and restoring day; year(%d) month(%d).\n", r.year, r.month);
			} else {
				r.month++;
				//PRINTF_INFO("Moving to next month and restoring day; year(%d) month(%d).\n", r.year, r.month);
			}
			r.day = 1;
			if(willRemain > 0){
				//PRINTF_INFO("Adding %d remaining days.\n", willRemain);
				r = NBDatetime_addDays(&r, willRemain);
			}
		}
	} else {
		NBASSERT(FALSE) //Pending to test with Internet
	}
	return r;
}

STNBDatetime NBDatetime_addHours(const STNBDatetime* ref, const SI32 amm){
	STNBDatetime r = *ref;
	if(amm >= 0){
		SI32 hours = amm;
		//Days
		{
			const SI32 fullDays = (hours / 24);
			if(fullDays > 0){
				r = NBDatetime_addDays(&r, fullDays);
				hours -= (fullDays * 24);
			} NBASSERT(hours < 24)
		}
		//Remain
		{
			r.hour += (UI8)hours;
			if(r.hour > 23){
				r.hour -= 24;
				r = NBDatetime_addDays(&r, 1);
			}
		}
	} else {
		NBASSERT(FALSE) //Pending to test with Internet
	}
	return r;
}

STNBDatetime NBDatetime_addMinutes(const STNBDatetime* ref, const SI32 amm){
	STNBDatetime r = *ref;
	if(amm >= 0){
		SI32 mins = amm;
		//Days
		{
			const SI32 fullDays = (mins / (24 * 60));
			if(fullDays > 0){
				r = NBDatetime_addDays(&r, fullDays);
				mins -= (fullDays * (24 * 60));
			} NBASSERT(mins < (24 * 60))
		}
		//Hours
		{
			const SI32 fullHours = (mins / 60);
			if(fullHours > 0){
				r = NBDatetime_addHours(&r, fullHours);
				mins -= (fullHours * 60);
			} NBASSERT(mins < 60)
		}
		//Remain
		{
			r.min += (UI8)mins;
			if(r.min > 59){
				r.min -= 60;
				r = NBDatetime_addHours(&r, 1);
			}
		}
	} else {
		NBASSERT(FALSE) //Pending to test with Internet
	}
	return r;
}
