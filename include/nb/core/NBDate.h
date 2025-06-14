//
//  NBDate.h
//  nbframework
//
//  Created by Marcos Ortega on 8/31/18.
//

#ifndef NB_DATE_H
#define NB_DATE_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBStructMap.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct STNBDate_ {
		UI16	year;
		UI8		month;
		UI8		day;
	} STNBDate;

	const STNBStructMap* NBDate_getSharedStructMap(void);
	
	STNBDate	NBDate_getCurUTC(void);
	STNBDate	NBDate_getCurLocal(void);
	UI64		NBDate_getUTCTimestampFromDate(const STNBDate date);
	UI64		NBDate_getLocalTimestampFromDate(const STNBDate date);

	UI8			NBDate_daysInMonth(const UI32 year, const UI8 month);
	BOOL		NBDate_isValid(const STNBDate* obj);
	BOOL		NBDate_isLower(const STNBDate* obj, const STNBDate* agaisnt);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
