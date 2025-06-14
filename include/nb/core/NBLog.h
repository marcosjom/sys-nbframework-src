
#ifndef NB_LOG_H
#define NB_LOG_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBThreadMutex.h"
#include <stdio.h>		//for FILE

#ifdef __cplusplus
extern "C" {
#endif
	
#define LOG(STR_FMT, ...)		{ PRINTF(STR_FMT, ##__VA_ARGS__); NBLog_verb(NBLog_globalInstance(), STR_FMT, ##__VA_ARGS__); }
#define LOG_INFO(STR_FMT, ...)	{ PRINTF_INFO(STR_FMT, ##__VA_ARGS__);  NBLog_info(NBLog_globalInstance(), STR_FMT, ##__VA_ARGS__); }
#define LOG_ERROR(STR_FMT, ...)	{ PRINTF_ERROR(STR_FMT, ##__VA_ARGS__);  NBLog_error(NBLog_globalInstance(), STR_FMT, ##__VA_ARGS__); }
#define LOG_WARN(STR_FMT, ...)	{ PRINTF_WARNING(STR_FMT, ##__VA_ARGS__);  NBLog_warn(NBLog_globalInstance(), STR_FMT, ##__VA_ARGS__); }
	
	typedef enum ENNBLogType_ {
		ENNBLogType_Info = 0,	//Info and verbose
		ENNBLogType_Warn,		//Warnings
		ENNBLogType_Err,		//Errors
		ENNBLogType_Count
	} ENNBLogType;

	typedef enum ENNBLogLevel_ {
		ENNBLogLevel_None = 0,	//no logs
		//
		ENNBLogLevel_Error,		//errors only
		ENNBLogLevel_Warning,	//errors and warnings
		ENNBLogLevel_Info,		//errors, warnings and infos
		ENNBLogLevel_Verbose,	//all
		//
		ENNBLogLevel_Count
	} ENNBLogLevel;

	const STNBEnumMap* NBLogLevel_getSharedEnumMap(void);
	
	typedef enum ENNBLogPathTemplatePathPartType_ {
		ENNBLogPathTemplatePathPartType_YYYY = 0,
		ENNBLogPathTemplatePathPartType_MM,
		ENNBLogPathTemplatePathPartType_DD,
		ENNBLogPathTemplatePathPartType_hh,
		ENNBLogPathTemplatePathPartType_mm,
		ENNBLogPathTemplatePathPartType_Count
	} ENNBLogPathTemplatePathPartType;
	
	typedef enum ENNBLogPathTemplatePathBit_ {
		ENNBLogPathTemplatePathBit_None	= 0,
		ENNBLogPathTemplatePathBit_YYYY = 1,	//Year
		ENNBLogPathTemplatePathBit_MM	= 2,	//Month
		ENNBLogPathTemplatePathBit_DD	= 4,	//Day
		ENNBLogPathTemplatePathBit_hh	= 8,	//hour
		ENNBLogPathTemplatePathBit_mm	= 16	//minute
	} ENNBLogPathTemplatePathBit;
	
	typedef struct STNBLogPathReplacePos_ {
		UI32	pos;
		ENNBLogPathTemplatePathPartType	type;
	} STNBLogPathReplacePos;
	
	typedef struct STNBLogOutput_ {
		FILE*			file;
		STNBDatetime	lastTime;
		STNBString		pathTemplate;				//like "folder/[YYYY]/[MM]/[DD]/[hh]-[mm]-dbsync.log" (without the replace elements)
		UI32			pathTemplateChangeMask;		//ENNBLogPathTemplatePathBit
		STNBString		pathTemplateReplaceBase;	//like "folder////--dbsync.log" (without the replace elements)
		STNBArray		pathTemplateReplaceIdxs;	//STNBLogPathReplacePos
	} STNBLogOutput;
	
	typedef struct STNBLog_ {
		STNBArray	outputs;	//STNBLogOutput
		STNBArray	outIdxsByType[ENNBLogType_Count];	//UI32
		STNBThreadMutex outMutex;
		//
		STNBString	tmpString;
		BOOL		isVerbose;
	} STNBLog;
	
	void NBLog_init(STNBLog* obj);
	void NBLog_release(STNBLog* obj);
	
	STNBLog* NBLog_globalInstance(void);
	
	BOOL NBLog_addPathTemplate(STNBLog* obj, const ENNBLogType type, const char* pathTemplate);
	
	BOOL NBLog_isVerbose(STNBLog* obj);
	void NBLog_setModeVerbose(STNBLog* obj, const BOOL activeVerbose);
	void NBLog_info(STNBLog* obj, const char* strFormat, ...);
	void NBLog_warn(STNBLog* obj, const char* strFormat, ...);
	void NBLog_error(STNBLog* obj, const char* strFormato, ...);
	
	//
	
	void NBLog_verb(STNBLog* obj, const char* strFormat, ...);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
