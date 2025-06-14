//
//  XUCadena.c
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
//Complementary PCH
#ifdef _WIN32
//#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>		//for SYSTEMTIME
#else
//#	include <sys/time.h>	//for struct timeval, gettimeofday()
#endif
#include <time.h>			//for struct tm
//
#include "nb/core/NBDate.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBMngrStructMaps.h"

STNBStructMapsRec NBDate_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBDate_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBDate_sharedStructMap);
	if(NBDate_sharedStructMap.map == NULL){
		STNBDate s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBDate);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addUIntM(map, s, year);
		NBStructMap_addUIntM(map, s, month);
		NBStructMap_addUIntM(map, s, day);
		NBDate_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBDate_sharedStructMap);
	return NBDate_sharedStructMap.map;
}

STNBDate NBDate_getCurUTC(void){
	STNBDate r;
#	ifdef _WIN32
	SYSTEMTIME t_st; GetSystemTime(&t_st);
	r.year	= (UI16)t_st.wYear;
	r.month	= (UI8)t_st.wMonth;
	r.day	= (UI8)t_st.wDay;
#	else
	time_t now = time(NULL); //es UTC
	struct tm t_st; gmtime_r(&now, &t_st); //es UTC
	r.year	= t_st.tm_year + 1900;
	r.month	= t_st.tm_mon + 1;
	r.day	= t_st.tm_mday;
#	endif
	return r;
}

STNBDate NBDate_getCurLocal(void){
	STNBDate r;
#	ifdef _WIN32
	SYSTEMTIME t_st; GetLocalTime(&t_st);
	r.year	= (UI16)t_st.wYear;
	r.month	= (UI8)t_st.wMonth;
	r.day	= (UI8)t_st.wDay;
#	else
	time_t now = time(NULL);
	struct tm* t_st = localtime(&now);
	r.year	= t_st->tm_year + 1900;
	r.month	= t_st->tm_mon + 1;
	r.day	= t_st->tm_mday;
#	endif
	return r;
}

UI64 NBDate_getUTCTimestampFromDate(const STNBDate date){
	UI64 r = 0;
#	ifdef _WIN32
	struct tm t_st;
	NBMemory_setZeroSt(t_st, struct tm);
	t_st.tm_year = date.year - 1900;
	t_st.tm_mon = date.month - 1;
	t_st.tm_mday = date.day;
	r = _mkgmtime64(&t_st);
#	else
	struct tm t_st;
	NBMemory_setZeroSt(t_st, struct tm);
	t_st.tm_year 	= date.year - 1900;
	t_st.tm_mon		= date.month - 1;
	t_st.tm_mday	= date.day;
	r				= timegm(&t_st);
#	endif
	return r;
}


UI64 NBDate_getLocalTimestampFromDate(const STNBDate date){
	UI64 r = 0;
#	ifdef _WIN32
	struct tm t_st;
	NBMemory_setZeroSt(t_st, struct tm);
	t_st.tm_year 	= date.year - 1900;
	t_st.tm_mon		= date.month - 1;
	t_st.tm_mday	= date.day;
	r				= mktime(&t_st);
#	else
	struct tm t_st;
	NBMemory_setZeroSt(t_st, struct tm);
	t_st.tm_year 	= date.year - 1900;
	t_st.tm_mon		= date.month - 1;
	t_st.tm_mday	= date.day;
	r				= mktime(&t_st);
#	endif
	return r;
}

UI8 NBDate_daysInMonth(const UI32 year, const UI8 month){
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

BOOL NBDate_isValid(const STNBDate* obj){
	BOOL r = FALSE;
	if(obj != NULL){
		if(obj->day >= 1 && obj->day <= 31 && obj->month >= 1 && obj->month <= 12 && obj->year > 0){
			const UI8 days = NBDate_daysInMonth(obj->year, obj->month);
			if(obj->day <= days){
				r = TRUE;
			}
		}
		if(!r){
			PRINTF_INFO("Date is not valid: %d-%d-%d.\n", obj->year, obj->month, obj->day);
		}
	}
	return r;
}

BOOL NBDate_isLower(const STNBDate* obj, const STNBDate* agaisnt){
	BOOL r = FALSE;
	if(obj != NULL && agaisnt != NULL){
		r = (obj->year < agaisnt->year || (obj->year == agaisnt->year && obj->month < agaisnt->month) || (obj->year == agaisnt->year && obj->month == agaisnt->month && obj->day < agaisnt->day));
	}
	return r;
}
