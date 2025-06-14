

#include "nb/NBFrameworkPch.h"
//Complementary PCH
#ifdef _WIN32
//#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>		//for DWORD and more
#else
#	include <sys/stat.h>	//for directories and simlinks
#endif
#include <stdarg.h>			//for va_list
//
#include "nb/core/NBLog.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBDatetime.h"
#include "nb/core/NBCompare.h"

STNBLog* _globalDRLog = NULL; //Objeto log por defecto (debe inicializarse por lo mens un log)

//LogLevel

STNBEnumMapRecord NBLogLevel_sharedEnumMapRecs[] = {
	{ ENNBLogLevel_None, "ENNBLogLevel_None", "none" }
	, { ENNBLogLevel_Error, "ENNBLogLevel_Error", "error" }
	, { ENNBLogLevel_Warning, "ENNBLogLevel_Warning", "warning" }
	, { ENNBLogLevel_Info, "ENNBLogLevel_Info", "info" }
	, { ENNBLogLevel_Verbose, "ENNBLogLevel_Verbose", "verbose" }
};

STNBEnumMap NBLogLevel_sharedEnumMap = {
	"ENNBLogLevel"
	, NBLogLevel_sharedEnumMapRecs
	, (sizeof(NBLogLevel_sharedEnumMapRecs) / sizeof(NBLogLevel_sharedEnumMapRecs[0]))
};

const STNBEnumMap* NBLogLevel_getSharedEnumMap(void){
	return &NBLogLevel_sharedEnumMap;
}


void NBLog_init(STNBLog* obj){
    NBThreadMutex_init(&obj->outMutex);
    NBArray_init(&obj->outputs, sizeof(STNBLogOutput), NULL);
	{
		//Init
		SI32 i; for(i = 0; i < ENNBLogType_Count; i++){
		    NBArray_init(&obj->outIdxsByType[i], sizeof(UI32), NBCompareUI32);
		}
	}
	//
    NBString_init(&obj->tmpString);
	obj->isVerbose = FALSE;
	//
	if(_globalDRLog == NULL) _globalDRLog = obj;
}

void NBLog_release(STNBLog* obj){
	if(_globalDRLog == obj) _globalDRLog = NULL;
    NBThreadMutex_lock(&obj->outMutex);
	{
		SI32 i; for(i = 0; i < ENNBLogType_Count; i++){
		    NBArray_empty(&obj->outIdxsByType[i]);
		    NBArray_release(&obj->outIdxsByType[i]);
		}
	}
	//Close streams
	{
		SI32 i; for(i = 0; i < obj->outputs.use; i++){
			STNBLogOutput* out = (STNBLogOutput*)NBArray_itemAtIndex(&obj->outputs, i);
			if(out->file != NULL){
				fclose(out->file);
				out->file = NULL;
			}
			out->lastTime.year	= 0;
			out->lastTime.month	= 0;
			out->lastTime.day	= 0;
			out->lastTime.hour	= 0;
			out->lastTime.min	= 0;
			out->lastTime.sec	= 0;
			out->lastTime.ms	= 0;
		    NBString_release(&out->pathTemplate);
			out->pathTemplateChangeMask = 0;
		    NBString_release(&out->pathTemplateReplaceBase);
		    NBArray_release(&out->pathTemplateReplaceIdxs);
		}
	    NBArray_empty(&obj->outputs);
	    NBArray_release(&obj->outputs);
	}
    NBString_release(&obj->tmpString);
	obj->isVerbose			= FALSE;
    NBThreadMutex_unlock(&obj->outMutex);
    NBThreadMutex_release(&obj->outMutex);
}

STNBLog* NBLog_globalInstance(void){
	return _globalDRLog;
}

//Verbose
BOOL NBLog_isVerbose(STNBLog* obj){
	return obj->isVerbose;
}

void NBLog_setVerbose(STNBLog* obj, const BOOL isVerbose){
	obj->isVerbose = isVerbose;
}

//

BOOL NBLog_addPathTemplate(STNBLog* obj, const ENNBLogType type, const char* pathTemplate){
	BOOL r = FALSE;
	if(pathTemplate != NULL && type < ENNBLogType_Count){
		if(pathTemplate[0] != '\0'){
			SI32 iOut = -1;
			//Search at current path
			{
				SI32 i; for(i = 0; i < obj->outputs.use; i++){
					const STNBLogOutput* out = (const STNBLogOutput*)NBArray_itemAtIndex(&obj->outputs, i);
					if(NBString_strIsEqual(out->pathTemplate.str, pathTemplate)){
						iOut = i;
						//Search and link to output (if necesary)
						{
							STNBArray* rels = &obj->outIdxsByType[type];
							SI32 i; for(i = 0; i < rels->use; i++){
								const UI32* idx = (const UI32*)NBArray_itemAtIndex(rels, i);
								if(*idx == iOut) break;
							}
							//Add link
							if(i == rels->use){
								const UI32 val = iOut;
							    NBArray_addValue(rels, val);
							}
						}
						break;
					}
				}
			}
			//PRINTF_INFO("Adding log(%d): '%s'.\n", type, pathTemplate);
			//Add new output def
			if(iOut == -1){
				STNBLogOutput out;
				out.file			= NULL;
				out.lastTime.year	= 0;
				out.lastTime.month	= 0;
				out.lastTime.day	= 0;
				out.lastTime.hour	= 0;
				out.lastTime.min	= 0;
				out.lastTime.sec	= 0;
				out.lastTime.ms		= 0;
			    NBString_init(&out.pathTemplate);
				out.pathTemplateChangeMask	= 0;
			    NBString_init(&out.pathTemplateReplaceBase);
			    NBArray_init(&out.pathTemplateReplaceIdxs, sizeof(STNBLogPathReplacePos), NULL);
				//Config
				{
				    NBString_concat(&out.pathTemplate, pathTemplate);
					SI32 lastPos = 0;
					while(TRUE){
						SI32 leftPos = -1, leftLen = 0;
						ENNBLogPathTemplatePathBit leftTypeBit = ENNBLogPathTemplatePathBit_None;
						ENNBLogPathTemplatePathPartType leftType = ENNBLogPathTemplatePathPartType_Count;
						//Search parts
						{
							const SI32 pos = NBString_strIndexOf(pathTemplate, "[YYYY]", lastPos);
							if(leftPos == -1 || (pos != -1 && pos < leftPos)){
								leftPos = pos; leftLen = 6; leftTypeBit = ENNBLogPathTemplatePathBit_YYYY; leftType = ENNBLogPathTemplatePathPartType_YYYY;
							}
						}
						{
							const SI32 pos = NBString_strIndexOf(pathTemplate, "[MM]", lastPos);
							if(leftPos == -1 || (pos != -1 && pos < leftPos)){
								leftPos = pos; leftLen = 4; leftTypeBit = ENNBLogPathTemplatePathBit_MM; leftType = ENNBLogPathTemplatePathPartType_MM;
							}
						}
						{
							const SI32 pos = NBString_strIndexOf(pathTemplate, "[DD]", lastPos);
							if(leftPos == -1 || (pos != -1 && pos < leftPos)){
								leftPos = pos; leftLen = 4; leftTypeBit = ENNBLogPathTemplatePathBit_DD; leftType = ENNBLogPathTemplatePathPartType_DD;
							}
						}
						{
							const SI32 pos = NBString_strIndexOf(pathTemplate, "[hh]", lastPos);
							if(leftPos == -1 || (pos != -1 && pos < leftPos)){
								leftPos = pos; leftLen = 4; leftTypeBit = ENNBLogPathTemplatePathBit_hh; leftType = ENNBLogPathTemplatePathPartType_hh;
							}
						}
						{
							const SI32 pos = NBString_strIndexOf(pathTemplate, "[mm]", lastPos);
							if(leftPos == -1 || (pos != -1 && pos < leftPos)){
								leftPos = pos; leftLen = 4; leftTypeBit = ENNBLogPathTemplatePathBit_mm; leftType = ENNBLogPathTemplatePathPartType_mm;
							}
						}
						if(leftPos >= 0){
							//Add text
						    NBASSERT(lastPos <= leftPos)
						    NBASSERT(leftTypeBit != ENNBLogPathTemplatePathBit_None)
						    NBASSERT(leftType != ENNBLogPathTemplatePathPartType_Count)
						    NBString_concatBytes(&out.pathTemplateReplaceBase, &pathTemplate[lastPos], (leftPos - lastPos));
							//Add replace index
							STNBLogPathReplacePos repPos;
							repPos.pos	= out.pathTemplateReplaceBase.length;
							repPos.type	= leftType;
						    NBArray_addValue(&out.pathTemplateReplaceIdxs, repPos);
							//Add mask
							out.pathTemplateChangeMask |= leftTypeBit;
							//Continue
							lastPos = leftPos + leftLen;
						} else {
						    NBString_concatBytes(&out.pathTemplateReplaceBase, &pathTemplate[lastPos], (NBString_strLenBytes(pathTemplate) - lastPos));
							break;
						}
					}
				}
				//
			    NBArray_addValue(&obj->outputs, out);
				//Link to output
				{
					const UI32 val = (obj->outputs.use - 1);
				    NBArray_addValue(&obj->outIdxsByType[type], val);
				}
			}
			r = TRUE;
		}
	}
	return r;
}

//Output

void NBLog_verifyOutputStream(STNBLogOutput* out, const STNBDatetime now){
	if(out->pathTemplateChangeMask != 0 || out->file == NULL){
		//Current time
		UI32 timeChangedBits = 0;
		if(out->lastTime.year != now.year) timeChangedBits |= ENNBLogPathTemplatePathBit_YYYY;
		if(out->lastTime.month != now.month) timeChangedBits |= ENNBLogPathTemplatePathBit_MM;
		if(out->lastTime.day != now.day) timeChangedBits |= ENNBLogPathTemplatePathBit_DD;
		if(out->lastTime.hour != now.hour) timeChangedBits |= ENNBLogPathTemplatePathBit_hh;
		if(out->lastTime.min != now.min) timeChangedBits |= ENNBLogPathTemplatePathBit_mm;
		if((timeChangedBits & out->pathTemplateChangeMask) != 0 || out->file == NULL){
			//Close prev stream
			if(out->file != NULL){
				fclose(out->file);
				out->file = NULL;
			}
			//Open new stream
			STNBString strPath;
		    NBString_init(&strPath);
			//Build path
			{
				UI32 lastPos = 0;
				UI32 i; for(i = 0; i < out->pathTemplateReplaceIdxs.use; i++){
					const STNBLogPathReplacePos* repPos = (const STNBLogPathReplacePos*)NBArray_itemAtIndex(&out->pathTemplateReplaceIdxs, i);
					//Concat left
				    NBString_concatBytes(&strPath, &out->pathTemplateReplaceBase.str[lastPos], (repPos->pos - lastPos));
					//Concat replacement
					switch (repPos->type) {
						case ENNBLogPathTemplatePathPartType_YYYY:
							if(now.year < 1000) NBString_concatByte(&strPath, '0');
							if(now.year < 100) NBString_concatByte(&strPath, '0');
							if(now.year < 10) NBString_concatByte(&strPath, '0');
						    NBString_concatUI32(&strPath, now.year);
							break;
						case ENNBLogPathTemplatePathPartType_MM:
							if(now.month < 10) NBString_concatByte(&strPath, '0');
						    NBString_concatUI32(&strPath, now.month);
							break;
						case ENNBLogPathTemplatePathPartType_DD:
							if(now.day < 10) NBString_concatByte(&strPath, '0');
						    NBString_concatUI32(&strPath, now.day);
							break;
						case ENNBLogPathTemplatePathPartType_hh:
							if(now.hour < 10) NBString_concatByte(&strPath, '0');
						    NBString_concatUI32(&strPath, now.hour);
							break;
						case ENNBLogPathTemplatePathPartType_mm:
							if(now.min < 10) NBString_concatByte(&strPath, '0');
						    NBString_concatUI32(&strPath, now.min);
							break;
						default:
							NBASSERT(FALSE)
							break;
					}
					//
					lastPos = repPos->pos;
				}
				//Add remaining
			    NBString_concatBytes(&strPath, &out->pathTemplateReplaceBase.str[lastPos], (out->pathTemplateReplaceBase.length - lastPos));
			}
			//Open path
			{
				//Create folders
				{
					STNBString partialPath;
				    NBString_init(&partialPath);
					UI32 iPos = 0;
					while(TRUE){
						const SI32 iPosSlash = NBString_strIndexOf(strPath.str, "/", iPos);
						if(iPosSlash == -1){
							break;
						} else {
						    NBString_concatBytes(&partialPath, &strPath.str[iPos], (iPosSlash - iPos));
#							ifdef _WIN32
							const DWORD dwAttrib = GetFileAttributes(partialPath.str);
							if((dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))) {
								//Folder exists
							} else {
								//Create folder
								if (!CreateDirectory(partialPath.str, NULL)) {
									//r = FALSE; NBASSERT(FALSE)
									break;
								}
							}
#							else
							struct stat sb;
							if (stat(partialPath.str, &sb) == 0 && S_ISDIR(sb.st_mode)){
								//Folder exists
							} else {
								//Create folder
								if(mkdir(partialPath.str, 0777) != 0){
									//r = FALSE; NBASSERT(FALSE)
									break;
								}
							}
#							endif
						    NBString_concat(&partialPath, "/");
							iPos = (iPosSlash + 1);
						}
					}
				    NBString_release(&partialPath);
				}
				//
				FILE* f = NULL;
#				ifdef _WIN32
				fopen_s(&f, strPath.str, "ab+");
#				else
				f = fopen(strPath.str, "ab+");
#				endif
				if(f == NULL){
					PRINTF_ERROR("LOG ERROR, could not open output file '%s'.\n", strPath.str);
				} else {
					//NBPRINTF_INFO("LOG, new stream opened '%s'.\n", strPath.str);
					out->file = f;
				}
			}
		    NBString_release(&strPath);
		}
		//
		out->lastTime = now;
	}
}

void NBLog_info(STNBLog* obj, const char* str, ...){
	if(obj != NULL){
	    NBThreadMutex_lock(&obj->outMutex);
		const STNBArray* outs = &obj->outIdxsByType[ENNBLogType_Info];
		if(outs->use > 0){
			BOOL outVerified = FALSE;
			const STNBDatetime now = NBDatetime_getCurLocal();
			//Date
			{
			    NBString_empty(&obj->tmpString);
			    NBString_concatUI32(&obj->tmpString, now.year);
			    NBString_concatByte(&obj->tmpString, '-');
				if(now.month < 10) NBString_concatByte(&obj->tmpString, '0');
			    NBString_concatUI32(&obj->tmpString, now.month);
			    NBString_concatByte(&obj->tmpString, '-');
				if(now.day < 10) NBString_concatByte(&obj->tmpString, '0');
			    NBString_concatUI32(&obj->tmpString, now.day);
			    NBString_concatByte(&obj->tmpString, ' ');
				if(now.hour < 10) NBString_concatByte(&obj->tmpString, '0');
			    NBString_concatUI32(&obj->tmpString, now.hour);
			    NBString_concatByte(&obj->tmpString, ':');
				if(now.min < 10) NBString_concatByte(&obj->tmpString, '0');
			    NBString_concatUI32(&obj->tmpString, now.min);
			    NBString_concatByte(&obj->tmpString, ':');
				if(now.sec < 10) NBString_concatByte(&obj->tmpString, '0');
			    NBString_concatUI32(&obj->tmpString, now.sec);
			    NBString_concatByte(&obj->tmpString, ' ');
				SI32 i; for(i = 0; i < outs->use; i++){
					const UI32* outIdx = (const UI32*)NBArray_itemAtIndex(outs, i);
					STNBLogOutput* out = (STNBLogOutput*)NBArray_itemAtIndex(&obj->outputs, *outIdx);
					if(!outVerified){
					    NBLog_verifyOutputStream(out, now);
					}
					if(out->file != NULL){
						fprintf(out->file, "%s", obj->tmpString.str);
					}
				}
				outVerified = TRUE;
			}
			//Message
			{
				SI32 i; for(i = 0; i < outs->use; i++){
					const UI32* outIdx = (const UI32*)NBArray_itemAtIndex(outs, i);
					STNBLogOutput* out = (STNBLogOutput*)NBArray_itemAtIndex(&obj->outputs, *outIdx);
					if(!outVerified){
					    NBLog_verifyOutputStream(out, now);
					}
					if(out->file != NULL){
						va_list vaArgs;
						va_start(vaArgs, str);
						vfprintf(out->file, str, vaArgs);
						//New line (if necesary)
						{
							const UI32 len = NBString_strLenBytes(str);
							if(len <= 0){
								fprintf(out->file, "\n");
							} else if(str[len - 1] != '\n'){
								fprintf(out->file, "\n");
							}
						}
						//Flush
						fflush(out->file);
						va_end(vaArgs);
					}
				}
				outVerified = TRUE;
			}
		}
	    NBThreadMutex_unlock(&obj->outMutex);
	}
}

void NBLog_warn(STNBLog* obj, const char* str, ...){
	if(obj != NULL){
	    NBThreadMutex_lock(&obj->outMutex);
		const STNBArray* outs = &obj->outIdxsByType[ENNBLogType_Warn];
		if(outs->use > 0){
			BOOL outVerified = FALSE;
			const STNBDatetime now = NBDatetime_getCurLocal();
			//Date
			{
			    NBString_empty(&obj->tmpString);
			    NBString_concatUI32(&obj->tmpString, now.year);
			    NBString_concatByte(&obj->tmpString, '-');
				if(now.month < 10) NBString_concatByte(&obj->tmpString, '0');
			    NBString_concatUI32(&obj->tmpString, now.month);
			    NBString_concatByte(&obj->tmpString, '-');
				if(now.day < 10) NBString_concatByte(&obj->tmpString, '0');
			    NBString_concatUI32(&obj->tmpString, now.day);
			    NBString_concatByte(&obj->tmpString, ' ');
				if(now.hour < 10) NBString_concatByte(&obj->tmpString, '0');
			    NBString_concatUI32(&obj->tmpString, now.hour);
			    NBString_concatByte(&obj->tmpString, ':');
				if(now.min < 10) NBString_concatByte(&obj->tmpString, '0');
			    NBString_concatUI32(&obj->tmpString, now.min);
			    NBString_concatByte(&obj->tmpString, ':');
				if(now.sec < 10) NBString_concatByte(&obj->tmpString, '0');
			    NBString_concatUI32(&obj->tmpString, now.sec);
			    NBString_concatByte(&obj->tmpString, ' ');
				SI32 i; for(i = 0; i < outs->use; i++){
					const UI32* outIdx = (const UI32*)NBArray_itemAtIndex(outs, i);
					STNBLogOutput* out = (STNBLogOutput*)NBArray_itemAtIndex(&obj->outputs, *outIdx);
					if(!outVerified){
					    NBLog_verifyOutputStream(out, now);
					}
					if(out->file != NULL){
						fprintf(out->file, "%s", obj->tmpString.str);
					}
				}
				outVerified = TRUE;
			}
			//Message
			{
				SI32 i; for(i = 0; i < outs->use; i++){
					const UI32* outIdx = (const UI32*)NBArray_itemAtIndex(outs, i);
					STNBLogOutput* out = (STNBLogOutput*)NBArray_itemAtIndex(&obj->outputs, *outIdx);
					if(!outVerified){
					    NBLog_verifyOutputStream(out, now);
					}
					if(out->file != NULL){
						va_list vaArgs;
						va_start(vaArgs, str);
						vfprintf(out->file, str, vaArgs);
						//New line (if necesary)
						{
							const UI32 len = NBString_strLenBytes(str);
							if(len <= 0){
								fprintf(out->file, "\n");
							} else if(str[len - 1] != '\n'){
								fprintf(out->file, "\n");
							}
						}
						//Flush
						fflush(out->file);
						va_end(vaArgs);
					}
				}
				outVerified = TRUE;
			}
		}
	    NBThreadMutex_unlock(&obj->outMutex);
	}
}

void NBLog_error(STNBLog* obj, const char* str, ...){
	if(obj != NULL){
	    NBThreadMutex_lock(&obj->outMutex);
		const STNBArray* outs = &obj->outIdxsByType[ENNBLogType_Err];
		if(outs->use > 0){
			BOOL outVerified = FALSE;
			const STNBDatetime now = NBDatetime_getCurLocal();
			//Date
			{
			    NBString_empty(&obj->tmpString);
			    NBString_concatUI32(&obj->tmpString, now.year);
			    NBString_concatByte(&obj->tmpString, '-');
				if(now.month < 10) NBString_concatByte(&obj->tmpString, '0');
			    NBString_concatUI32(&obj->tmpString, now.month);
			    NBString_concatByte(&obj->tmpString, '-');
				if(now.day < 10) NBString_concatByte(&obj->tmpString, '0');
			    NBString_concatUI32(&obj->tmpString, now.day);
			    NBString_concatByte(&obj->tmpString, ' ');
				if(now.hour < 10) NBString_concatByte(&obj->tmpString, '0');
			    NBString_concatUI32(&obj->tmpString, now.hour);
			    NBString_concatByte(&obj->tmpString, ':');
				if(now.min < 10) NBString_concatByte(&obj->tmpString, '0');
			    NBString_concatUI32(&obj->tmpString, now.min);
			    NBString_concatByte(&obj->tmpString, ':');
				if(now.sec < 10) NBString_concatByte(&obj->tmpString, '0');
			    NBString_concatUI32(&obj->tmpString, now.sec);
			    NBString_concatByte(&obj->tmpString, ' ');
				SI32 i; for(i = 0; i < outs->use; i++){
					const UI32* outIdx = (const UI32*)NBArray_itemAtIndex(outs, i);
					STNBLogOutput* out = (STNBLogOutput*)NBArray_itemAtIndex(&obj->outputs, *outIdx);
					if(!outVerified){
					    NBLog_verifyOutputStream(out, now);
					}
					if(out->file != NULL){
						fprintf(out->file, "%s", obj->tmpString.str);
					}
				}
				outVerified = TRUE;
			}
			//Message
			{
				SI32 i; for(i = 0; i < outs->use; i++){
					const UI32* outIdx = (const UI32*)NBArray_itemAtIndex(outs, i);
					STNBLogOutput* out = (STNBLogOutput*)NBArray_itemAtIndex(&obj->outputs, *outIdx);
					if(!outVerified){
					    NBLog_verifyOutputStream(out, now);
					}
					if(out->file != NULL){
						va_list vaArgs;
						va_start(vaArgs, str);
						vfprintf(out->file, str, vaArgs);
						//New line (if necesary)
						{
							const UI32 len = NBString_strLenBytes(str);
							if(len <= 0){
								fprintf(out->file, "\n");
							} else if(str[len - 1] != '\n'){
								fprintf(out->file, "\n");
							}
						}
						//Flush
						fflush(out->file);
						va_end(vaArgs);
					}
				}
				outVerified = TRUE;
			}
		}
	    NBThreadMutex_unlock(&obj->outMutex);
	}
}



void NBLog_verb(STNBLog* obj, const char* str, ...){
	if(obj != NULL){
	    NBThreadMutex_lock(&obj->outMutex);
		const STNBArray* outs = &obj->outIdxsByType[ENNBLogType_Info];
		if(outs->use > 0){
			BOOL outVerified = FALSE;
			const STNBDatetime now = NBDatetime_getCurLocal();
			//Date
			{
			    NBString_empty(&obj->tmpString);
			    NBString_concatUI32(&obj->tmpString, now.year);
			    NBString_concatByte(&obj->tmpString, '-');
				if(now.month < 10) NBString_concatByte(&obj->tmpString, '0');
			    NBString_concatUI32(&obj->tmpString, now.month);
			    NBString_concatByte(&obj->tmpString, '-');
				if(now.day < 10) NBString_concatByte(&obj->tmpString, '0');
			    NBString_concatUI32(&obj->tmpString, now.day);
			    NBString_concatByte(&obj->tmpString, ' ');
				if(now.hour < 10) NBString_concatByte(&obj->tmpString, '0');
			    NBString_concatUI32(&obj->tmpString, now.hour);
			    NBString_concatByte(&obj->tmpString, ':');
				if(now.min < 10) NBString_concatByte(&obj->tmpString, '0');
			    NBString_concatUI32(&obj->tmpString, now.min);
			    NBString_concatByte(&obj->tmpString, ':');
				if(now.sec < 10) NBString_concatByte(&obj->tmpString, '0');
			    NBString_concatUI32(&obj->tmpString, now.sec);
			    NBString_concatByte(&obj->tmpString, ' ');
				SI32 i; for(i = 0; i < outs->use; i++){
					const UI32* outIdx = (const UI32*)NBArray_itemAtIndex(outs, i);
					STNBLogOutput* out = (STNBLogOutput*)NBArray_itemAtIndex(&obj->outputs, *outIdx);
					if(!outVerified){
					    NBLog_verifyOutputStream(out, now);
					}
					if(out->file != NULL){
						fprintf(out->file, "%s", obj->tmpString.str);
					}
				}
				outVerified = TRUE;
			}
			//Message
			{
				
				SI32 i; for(i = 0; i < outs->use; i++){
					const UI32* outIdx = (const UI32*)NBArray_itemAtIndex(outs, i);
					STNBLogOutput* out = (STNBLogOutput*)NBArray_itemAtIndex(&obj->outputs, *outIdx);
					if(!outVerified){
					    NBLog_verifyOutputStream(out, now);
					}
					if(out->file != NULL){
						va_list vaArgs;
						va_start(vaArgs, str);
						vfprintf(out->file, str, vaArgs);
						//New line (if necesary)
						{
							const UI32 len = NBString_strLenBytes(str);
							if(len <= 0){
								fprintf(out->file, "\n");
							} else if(str[len - 1] != '\n'){
								fprintf(out->file, "\n");
							}
						}
						//Flush
						fflush(out->file);
						va_end(vaArgs);
					}
				}
				outVerified = TRUE;
			}
		}
	    NBThreadMutex_unlock(&obj->outMutex);
	}
}
