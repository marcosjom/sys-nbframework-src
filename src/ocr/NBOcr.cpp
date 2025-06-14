//
//  NBOcr.c
//  nbframework
//
//  Created by Marcos Ortega on 11/2/19.
//

#ifdef _WIN32
//#	define WIN32_LEAN_AND_MEAN
#include <windows.h> 
#endif
#include "nb/NBFrameworkPch.h"
#include "nb/NBFrameworkDefs.h"

#ifndef NB_COMPILE_DRIVER_MODE
#include "nb/ocr/NBOcr.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBThread.h"
#include "nb/files/NBFile.h"
#include "nb/files/NBFilesystem.h"
#include "nb/core/NBEncoding.h"
//
#ifdef NB_LIB_LEPTONICA
#	ifdef NB_LIB_LEPTONICA_SYSTEM
#		include "leptonica/allheaders.h"					//leptonica (image processing lib)
#	else
#		include "../src/ext/leptonica/src/allheaders.h"	//leptonica (image processing lib)
#	endif
#endif

#ifdef NB_LIB_TESSERACT
#   define TESS_CAPI_INCLUDE_BASEAPI
#   include "tesseract/capi.h"            //tesseract (ocr lib)
#   include "tesseract/ocrclass.h"        // ETEXT_DESC
#   ifndef NB_LIB_LEPTONICA
#       error NB_LIB_LEPTONICA code-or-lib is required when NB_LIB_TESSERACT is enabled.
#   endif
#endif

typedef struct STNBOcrOpq_ {
#	ifdef NB_LIB_TESSERACT
	TessBaseAPI*	tessApi;
#	endif
	ENOcrState		curState;
	SI32			feedCount;		//How many time a bitmap has been feed
	STNBOcrOSD		osdFound;		//Orientation and script (ex: Latin) found
	STNBString		textFound;		//Text found
	//Config
	struct {
		ENOcrSourceMode	mode;
		BOOL			onlyCustomDict;
		STNBString		tmpFolderPath;			//for tmp files
		STNBString		customCharsWhiteList;	//chars to search
		STNBString		customWords;			//word to search
		STNBString		customPatterns;			//patterns to search
	} cfg;
	//Job
	struct {
		SI32			progress;		//0-100
		STNBSizeI		srcSize;		//cur img size
		STNBAABoxI		curArea;		//cur area analyzing
		BOOL			canceling;
	} curJob;
	STNBThreadMutex	mutex;
	SI32			retainCount;
} STNBOcrOpq;

void NBOcr_init(STNBOcr* obj){
	STNBOcrOpq* opq 	= NBMemory_allocType(STNBOcrOpq);
	obj->opaque			= opq;
	NBMemory_setZeroSt(*opq, STNBOcrOpq);
	//
#	ifdef NB_LIB_TESSERACT
	opq->tessApi		= TessBaseAPICreate();
#	endif
	opq->curState		= ENOcrState_Iddle;
	opq->feedCount		= 0;
	NBString_init(&opq->textFound);
	//Config
	{
		opq->cfg.mode			= ENOcrSourceMode_TextAuto;
		opq->cfg.onlyCustomDict	= FALSE;
		NBString_init(&opq->cfg.tmpFolderPath);			//for tmp files
		NBString_init(&opq->cfg.customCharsWhiteList);	//chars to search
		NBString_init(&opq->cfg.customWords);			//word to search
		NBString_init(&opq->cfg.customPatterns);		//patterns to search
	}
	//Job
	{
		opq->curJob.progress	= 0; //0-100
		opq->curJob.srcSize		= NBST_P(STNBSizeI, 0, 0);
		opq->curJob.curArea 	= NBST_P(STNBAABoxI, 0, 0, 0, 0);
		opq->curJob.canceling	= FALSE;
	}
	NBThreadMutex_init(&opq->mutex);
	opq->retainCount = 1;
}

void NBOcr_retain(STNBOcr* obj){
	STNBOcrOpq* opq = (STNBOcrOpq*)obj->opaque; NBASSERT(opq != NULL)
	NBThreadMutex_lock(&opq->mutex);
	{
		NBASSERT(opq->retainCount > 0)
		opq->retainCount++;
	}
	NBThreadMutex_unlock(&opq->mutex);
}

void NBOcr_release(STNBOcr* obj){
	STNBOcrOpq* opq = (STNBOcrOpq*)obj->opaque; NBASSERT(opq != NULL)
	if(opq != NULL){
		NBThreadMutex_lock(&opq->mutex);
		NBASSERT(opq->retainCount > 0)
		opq->retainCount--;
		if(opq->retainCount > 0){
			NBThreadMutex_unlock(&opq->mutex);
		} else {
			//Wait untill not-processing
			{
				while (opq->curState == ENOcrState_Processing) {
					opq->curJob.canceling = TRUE;
					NBThreadMutex_unlock(&opq->mutex);
					NBThreadMutex_lock(&opq->mutex);
				}
			}
#			ifdef NB_LIB_TESSERACT
			if(opq->tessApi != NULL){
				TessBaseAPIDelete(opq->tessApi);
				opq->tessApi = NULL;
			}
#			endif
			opq->feedCount	= 0;
			NBString_release(&opq->textFound);
			//Config
			{
				opq->cfg.onlyCustomDict	= FALSE;
				NBString_release(&opq->cfg.tmpFolderPath);			//for tmp files
				NBString_release(&opq->cfg.customCharsWhiteList);	//chars to search
				NBString_release(&opq->cfg.customWords);			//word to search
				NBString_release(&opq->cfg.customPatterns);		//patterns to search
			}
			//Job
			{
				opq->curJob.progress	= 0; //0-100
				opq->curJob.srcSize		= NBST_P(STNBSizeI, 0, 0);
				opq->curJob.curArea		= NBST_P(STNBAABoxI, 0, 0, 0, 0);
				opq->curJob.canceling	= FALSE;
			}
			NBThreadMutex_unlock(&opq->mutex);
			NBThreadMutex_release(&opq->mutex);
			NBMemory_free(obj->opaque);
			obj->opaque		= NULL;
		}
	}
}

//Config

SI32 NBOcr_isSupported(void){ //determines if the current compilation supoports NBOCR, if not all methods will return error or do-nothing.
#   ifdef NB_LIB_TESSERACT
    return TRUE;
#   else
    return FALSE;
#   endif
}

void NBOcr_setMode(STNBOcr* obj, const ENOcrSourceMode mode){
	if(obj != NULL){
		STNBOcrOpq* opq = (STNBOcrOpq*)obj->opaque; NBASSERT(opq != NULL)
		NBThreadMutex_lock(&opq->mutex);
		if(opq->curState == ENOcrState_Iddle){
			opq->cfg.mode = mode;
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
}

void NBOcr_emptyCustomCfg(STNBOcr* obj){
	if(obj != NULL){
		STNBOcrOpq* opq = (STNBOcrOpq*)obj->opaque; NBASSERT(opq != NULL)
		NBThreadMutex_lock(&opq->mutex);
		if(opq->curState == ENOcrState_Iddle){
			opq->cfg.onlyCustomDict	= FALSE;
			NBString_empty(&opq->cfg.customCharsWhiteList);	//chars to search
			NBString_empty(&opq->cfg.customWords);			//word to search
			NBString_empty(&opq->cfg.customPatterns);		//patterns to search
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
}

void NBOcr_setOnlyCustomDict(STNBOcr* obj, const SI32 onlyCustomDict){
	if(obj != NULL){
		STNBOcrOpq* opq = (STNBOcrOpq*)obj->opaque; NBASSERT(opq != NULL)
		NBThreadMutex_lock(&opq->mutex);
		if(opq->curState == ENOcrState_Iddle){
			opq->cfg.onlyCustomDict	= onlyCustomDict;
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
}

void NBOcr_setTmpFolderPath(STNBOcr* obj, const char* path){
	if(obj != NULL){
		STNBOcrOpq* opq = (STNBOcrOpq*)obj->opaque; NBASSERT(opq != NULL)
		NBThreadMutex_lock(&opq->mutex);
		if(opq->curState == ENOcrState_Iddle){
			NBString_set(&opq->cfg.tmpFolderPath, path); //for tmp files
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
}

void NBOcr_addCustomCharsToWhitelist(STNBOcr* obj, const char* str, const SI32 includeOtherCase){
	if(obj != NULL && str != NULL){
		STNBOcrOpq* opq = (STNBOcrOpq*)obj->opaque; NBASSERT(opq != NULL)
		NBThreadMutex_lock(&opq->mutex);
		if(opq->curState == ENOcrState_Iddle){
			UI32 pos = 0; UI8 i; char cBuff[12];
			while(str[pos] != '\0'){
				const UI32 bytes = NBEncoding_utf8BytesExpected(str[pos]);
				if(str[pos] > 32){ //exclude control-chars and space(32)
					//Copy
					{
						for(i = 0; i < bytes; i++) cBuff[i] = str[pos + i];
						cBuff[i] = '\0';
					}
					//Search and add
					if(NBString_indexOf(&opq->cfg.customCharsWhiteList, cBuff, 0) == -1){
						NBString_concatBytes(&opq->cfg.customCharsWhiteList, cBuff, bytes);
					}
					//Add other case
					if(includeOtherCase){
						//ToDo: implement unicode support
						if(bytes == 1){
							cBuff[0] = NBEncoding_asciiOtherCase(cBuff[0]);
							if(NBString_indexOf(&opq->cfg.customCharsWhiteList, cBuff, 0) == -1){
								NBString_concatBytes(&opq->cfg.customCharsWhiteList, cBuff, 1);
							}
						}
					}
				}
				//Move to next
				pos += bytes;
			}
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
}

//One word per line.
void NBOcr_addCustomWords(STNBOcr* obj, const char* words){
	if(obj != NULL){
		STNBOcrOpq* opq = (STNBOcrOpq*)obj->opaque; NBASSERT(opq != NULL)
		NBThreadMutex_lock(&opq->mutex);
		if(opq->curState == ENOcrState_Iddle){
			NBString_concat(&opq->cfg.customWords, words); //word(s) to search
			//Add '\n' if necesary
			if(opq->cfg.customWords.length > 0){
				if(opq->cfg.customWords.str[opq->cfg.customWords.length - 1] != '\n'){
					NBString_concatByte(&opq->cfg.customWords, '\n');
				}
			}
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
}

//----------------------
//One pattern per line:
// \c - unichar for which UNICHARSET::get_isalpha() is true (character)
// \d - unichar for which UNICHARSET::get_isdigit() is true
// \n - unichar for which UNICHARSET::get_isdigit() or UNICHARSET::isalpha() are true
// \p - unichar for which UNICHARSET::get_ispunct() is true
// \a - unichar for which UNICHARSET::get_islower() is true
// \A - unichar for which UNICHARSET::get_isupper() is true
// \* - specified after each character or pattern to indicate that the character/pattern can be repeated any number of times
// Example:
// 1-8\d\d-GOOG-411 will be expanded to strings:
// 1-800-GOOG-411, 1-801-GOOG-411, ... 1-899-GOOG-411.
// Example:
// http://www.\n\*.com will be expanded to strings like:
// http://www.a.com http://www.a123.com ... http://www.ABCDefgHIJKLMNop.com
//----------------------
void NBOcr_addCustomPatterns(STNBOcr* obj, const char* patterns){
	if(obj != NULL){
		STNBOcrOpq* opq = (STNBOcrOpq*)obj->opaque; NBASSERT(opq != NULL)
		NBThreadMutex_lock(&opq->mutex);
		if(opq->curState == ENOcrState_Iddle){
			NBString_concat(&opq->cfg.customPatterns, patterns);			//word to search
			//Add '\n' if necesary
			if(opq->cfg.customPatterns.length > 0){
				if(opq->cfg.customPatterns.str[opq->cfg.customPatterns.length - 1] != '\n'){
					NBString_concatByte(&opq->cfg.customPatterns, '\n');
				}
			}
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
}

void NBOcr_empty(const STNBOcr* obj){
	if(obj != NULL){
		STNBOcrOpq* opq = (STNBOcrOpq*)obj->opaque; NBASSERT(opq != NULL)
		NBThreadMutex_lock(&opq->mutex);
		if(opq->curState == ENOcrState_Iddle){
			NBString_empty(&opq->textFound);
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
}

//

ENOcrState NBOcr_getState(const STNBOcr* obj){
	ENOcrState r = ENOcrState_Iddle;
	if(obj != NULL){
		STNBOcrOpq* opq = (STNBOcrOpq*)obj->opaque; NBASSERT(opq != NULL)
		NBThreadMutex_lock(&opq->mutex);
		{
			r = opq->curState;
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}

SI32 NBOcr_getProgress(const STNBOcr* obj, STNBSizeI* dstCurSize, STNBAABoxI* dstCurArea){
	SI32 r = 0;
	if(obj != NULL){
		STNBOcrOpq* opq = (STNBOcrOpq*)obj->opaque; NBASSERT(opq != NULL)
		NBThreadMutex_lock(&opq->mutex);
		{
			r = opq->curJob.progress;
			if(dstCurSize != NULL){
				*dstCurSize = opq->curJob.srcSize;
			}
			if(dstCurArea != NULL){
				*dstCurArea = opq->curJob.curArea;
			}
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}

SI32 NBOcr_getFeedCount(const STNBOcr* obj){
	SI32 r = 0;
	if(obj != NULL){
		STNBOcrOpq* opq = (STNBOcrOpq*)obj->opaque; NBASSERT(opq != NULL)
		NBThreadMutex_lock(&opq->mutex);
		{
			r = opq->feedCount;
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}

STNBOcrOSD NBOcr_getOSDFound(const STNBOcr* obj){ //Orientation and Scripting (ex: Latin) Detection
	STNBOcrOSD r;
	NBMemory_setZeroSt(r, STNBOcrOSD);
	if(obj != NULL){
		STNBOcrOpq* opq = (STNBOcrOpq*)obj->opaque; NBASSERT(opq != NULL)
		NBThreadMutex_lock(&opq->mutex);
		{
			r = opq->osdFound;
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}

SI32 NBOcr_getTextFound(const STNBOcr* obj, STNBString* dst){
	SI32 r = 0;
	if(obj != NULL && dst != NULL){
		STNBOcrOpq* opq = (STNBOcrOpq*)obj->opaque; NBASSERT(opq != NULL)
		NBThreadMutex_lock(&opq->mutex);
		{
			NBString_concatBytes(dst, opq->textFound.str, opq->textFound.length);
			r = opq->textFound.length;
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}

//Prepare bitmap data
SI32 NBOcr_ocrPrepareBitmap(const STNBBitmap* bmp){
	BOOL r = FALSE;
	if(bmp != NULL){
		const STNBBitmapProps bmpProps = NBBitmap_getProps(bmp);
		BYTE* bmpData = NBBitmap_getData(bmp);
		r = NBOcr_ocrPrepareBitmapData(bmpProps, bmpData);
	}
	return r;
}

SI32 NBOcr_ocrPrepareBitmapData(const STNBBitmapProps bmpProps, BYTE* bmpData){
	BOOL r = FALSE;
	if(bmpData != NULL){
		//Copy each word-4-bytes inverted
		BYTE tmp0, tmp1;
		SI32 y; for(y = 0 ; y < bmpProps.size.height; y++){
			BYTE* srcLine = &bmpData[y * bmpProps.bytesPerLine];
			const BYTE* srcLineNxt = srcLine + bmpProps.bytesPerLine;
			while((srcLine + 4) <= srcLineNxt){
				tmp0		= srcLine[0];
				tmp1		= srcLine[1];
				srcLine[0]	= srcLine[3];
				srcLine[1]	= srcLine[2];
				srcLine[2]	= tmp1;
				srcLine[3]	= tmp0;
				srcLine += 4;
			}
		}
	}
	return r;
}


//Sync - Callback

bool NBOcr_funcCancel_(void* cancel_this, int words){
	bool r = false;
	//PRINTF_INFO("NBOcr_funcCancel_: %d words.\n", words);
	STNBOcrOpq* opq = (STNBOcrOpq*)cancel_this;
	if(opq != NULL){
		NBThreadMutex_lock(&opq->mutex);
		{
			r = (opq->curJob.canceling ? true : false);
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}

//Sync - Callback


//ToDo: delete, used before Tesseract implemented 'progress_callback2'
/*bool NBOcr_funcProgressMod_(void* progress_this, int progress, int left, int right, int top, int bottom){
	bool r = false;
	//PRINTF_INFO("NBOcr_progress_: progress(%d), left(%d), right(%d), top(%d), bottom(%d).\n", progress, left, right, top, bottom);
	STNBOcrOpq* opq = (STNBOcrOpq*)progress_this;
	if(opq != NULL){
		NBThreadMutex_lock(&opq->mutex);
		{
			opq->curJob.progress		= progress;
			opq->curJob.curArea.xMin	= left;
			opq->curJob.curArea.xMax	= right;
			opq->curJob.curArea.yMin	= top;
			opq->curJob.curArea.yMax	= bottom;
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}*/

bool NBOcr_funcProgress2_(ETEXT_DESC* desc, int left, int right, int top, int bottom) {
	bool r = false;
	//PRINTF_INFO("NBOcr_progress_: progress(%d), left(%d), right(%d), top(%d), bottom(%d).\n", progress, left, right, top, bottom);
	STNBOcrOpq* opq = (STNBOcrOpq*)desc->cancel_this;
	if (opq != NULL) {
		NBThreadMutex_lock(&opq->mutex);
		{
			opq->curJob.progress = desc->progress;
			opq->curJob.curArea.xMin = left;
			opq->curJob.curArea.xMax = right;
			opq->curJob.curArea.yMin = top;
			opq->curJob.curArea.yMax = bottom;
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}

//Sync-exec

SI32 NBOcr_feedBitmapDataUnlockedPriv(STNBOcrOpq* opq, const STNBBitmapProps bmpProps, const BYTE* bmpData, const ENOcrSourcePrep bmpPrep, const STNBSizeI ppi, const char* lang3, const char* dataPath){
	BOOL r = FALSE;
	NBASSERT(opq != NULL)
	{
		NBASSERT(opq->retainCount > 0)
		NBASSERT(opq->curState == ENOcrState_Processing) //Must be set by caller
		if(bmpData != NULL && lang3 != NULL){
			BOOL inited = FALSE;
			STNBString wordsFilepath, patternsFilepath;
			//Init
			{
				NBString_init(&wordsFilepath);
				NBString_init(&patternsFilepath);
				//
				if(opq->cfg.onlyCustomDict){
					//Build WORDS paths and write files
					{
						NBString_concat(&wordsFilepath, opq->cfg.tmpFolderPath.str);
						if(wordsFilepath.length > 0){
							if(wordsFilepath.str[wordsFilepath.length - 1] != '/'){
								NBString_concatByte(&wordsFilepath, '/');
							}
						}
						NBString_concat(&wordsFilepath, "._tmp_");
						NBString_concatUI64(&wordsFilepath, (UI64)opq);
						NBString_concat(&wordsFilepath, "_words.txt");
						//Write file
						{
							STNBFileRef f = NBFile_alloc(NULL);
							if(!NBFile_open(f, wordsFilepath.str, ENNBFileMode_Write)){
								NBASSERT(FALSE)
							} else {
								NBFile_lock(f);
								NBFile_write(f, opq->cfg.customWords.str, opq->cfg.customWords.length);
								NBFile_unlock(f);
							}
							NBFile_release(&f);
						}
					}
					//Build PATTERNS paths and write files
					{
						NBString_concat(&patternsFilepath, opq->cfg.tmpFolderPath.str);
						if(patternsFilepath.length > 0){
							if(patternsFilepath.str[patternsFilepath.length - 1] != '/'){
								NBString_concatByte(&patternsFilepath, '/');
							}
						}
						NBString_concat(&patternsFilepath, "._tmp_");
						NBString_concatUI64(&patternsFilepath, (UI64)opq);
						NBString_concat(&patternsFilepath, "_patterns.txt");
						//Write file
						{
							STNBFileRef f = NBFile_alloc(NULL);
							if(!NBFile_open(f, patternsFilepath.str, ENNBFileMode_Write)){
								NBASSERT(FALSE)
							} else {
								NBFile_lock(f);
								NBFile_write(f, opq->cfg.customPatterns.str, opq->cfg.customPatterns.length);
								NBFile_unlock(f);
							}
							NBFile_release(&f);
						}
					}
				}
			}
			//Init
#			ifdef NB_LIB_TESSERACT
			{
				STNBString pageSegMode;
				NBString_init(&pageSegMode);
				switch (opq->cfg.mode) {
					case ENOcrSourceMode_OSD:
						NBString_concatSI32(&pageSegMode, tesseract::PSM_OSD_ONLY);
						break;
					case ENOcrSourceMode_SingleColumn:
						NBString_concatSI32(&pageSegMode, tesseract::PSM_SINGLE_COLUMN);
						break;
					case ENOcrSourceMode_SingleLine:
						NBString_concatSI32(&pageSegMode, tesseract::PSM_SINGLE_LINE);
						break;
					case ENOcrSourceMode_TextAuto:
						NBString_concatSI32(&pageSegMode, tesseract::PSM_SPARSE_TEXT);
						break;
					default:
						NBASSERT(FALSE)
						NBString_concatSI32(&pageSegMode, tesseract::PSM_SPARSE_TEXT);
						break;
				}
				//PSM_OSD_ONLY = 0,       ///< Orientation and script detection only.
				//PSM_AUTO_OSD = 1,       ///< Automatic page segmentation with orientation and script detection. (OSD)
				//PSM_AUTO_ONLY = 2,      ///< Automatic page segmentation, but no OSD, or OCR.
				//PSM_AUTO,           ///< Fully automatic page segmentation, but no OSD.
				//PSM_SINGLE_COLUMN,  ///< Assume a single column of text of variable sizes.
				//PSM_SINGLE_BLOCK_VERT_TEXT,  ///< Assume a single uniform block of vertically aligned text.
				//PSM_SINGLE_BLOCK,   ///< Assume a single uniform block of text. (Default.)
				//PSM_SINGLE_LINE,    ///< Treat the image as a single text line.
				//PSM_SINGLE_WORD,    ///< Treat the image as a single word.
				//PSM_CIRCLE_WORD,    ///< Treat the image as a single word in a circle.
				//PSM_SINGLE_CHAR,    ///< Treat the image as a single character.
				//PSM_SPARSE_TEXT,    ///< Find as much text as possible in no particular order.
				//PSM_SPARSE_TEXT_OSD,  ///< Sparse text with orientation and script det.
				//PSM_RAW_LINE,       ///< Treat the image as a single text line, bypassing hacks that are Tesseract-specific.
				{
					TessOcrEngineMode mode = tesseract::OEM_TESSERACT_ONLY;
					char** configs = NULL;
					int configs_size = 0;
					const char* vars_vec[] = {
						"load_system_dawg"			//"Load system word dawg."
						, "load_freq_dawg"			//"Load frequent word dawg."
						, "tessedit_dump_choices"	//"Dump char choices"
						, "tessedit_timing_debug"	//"Print timing stats"
						, "tessedit_char_blacklist"	//Blacklist of chars not to recognize"
						, "tessedit_char_whitelist"	//"Whitelist of chars to recognize"
						, "user_words_file"			//"A filename of user-provided words."
						, "user_patterns_file"		//"A filename of user-provided patterns."
						, "tessedit_pageseg_mode"	//PSM_SINGLE_BLOCK //"Page seg mode: 0=osd only, 1=auto+osd, 2=auto, 3=col, 4=block, 5=line, 6=word, 7=char (Values from PageSegMode enum in publictypes.h)"
					};
					const char* vars_values[] = {
						(opq->cfg.onlyCustomDict ? "0" : "1") 		//"load_system_dawg" //"Load system word dawg."
						, (opq->cfg.onlyCustomDict ? "0" : "1")		//"load_freq_dawg" //"Load frequent word dawg."
						, "0"		//"tessedit_dump_choices" //"Dump char choices"
						, "0"		//"tessedit_timing_debug" //"Print timing stats"
						, ""		//"tessedit_char_blacklist"	//Blacklist of chars not to recognize"
						, opq->cfg.customCharsWhiteList.str	//"tessedit_char_whitelist"	//"Whitelist of chars to recognize"
						, wordsFilepath.str			//"user_words_file" //"A filename of user-provided words."
						, patternsFilepath.str		//"user_patterns_file" //"A filename of user-provided patterns."
						, pageSegMode.str
					};
					size_t vars_vec_size = (sizeof(vars_vec) / sizeof(vars_vec[0])); NBASSERT((sizeof(vars_vec) / sizeof(vars_vec[0])) == (sizeof(vars_values) / sizeof(vars_values[0])))
					BOOL set_only_non_debug_params = TRUE;
					if(TessBaseAPIInit4(opq->tessApi, dataPath, lang3, mode, configs, configs_size, (char**)vars_vec, (char**)vars_values, vars_vec_size, set_only_non_debug_params)){
						PRINTF_ERROR("NBOcr, could not load lang data: '%s' at '%s'.\n", lang3, dataPath);
					} else {
						inited = TRUE;
					}
				}
				NBString_release(&pageSegMode);
			}
#			endif
			//Execute
			if(inited){
				STNBBitmap bmpTmp;
				NBBitmap_init(&bmpTmp); //only used if data is not 8-bits already
#				ifdef NB_LIB_TESSERACT
				{
					PIX* img = pixCreateNoInit(bmpProps.size.width, bmpProps.size.height, 8); NBASSERT(img != NULL)
					NBASSERT(img->w == bmpProps.size.width)
					NBASSERT(img->h == bmpProps.size.height)
					NBASSERT(img->d == 8) //8 bits
					NBASSERT(img->spp == 1) //1 sample-per-pixel
					NBASSERT(img->data != NULL)
					if(bmpProps.color == ENNBBitmapColor_GRIS8 || bmpProps.color == ENNBBitmapColor_ALPHA8){
						//Already 8-bits image
						NBASSERT((bmpProps.bytesPerLine % 4) == 0)
						NBASSERT(img->wpl == (bmpProps.bytesPerLine / 4)) //bytes-per-line
						if(bmpPrep == ENOcrSourcePrep_Prepared){
							//Direct copy (non-inverted)
							NBMemory_copy(img->data, bmpData, bmpProps.bytesPerLine * bmpProps.size.height);
						} else {
							//Copy each word-4-bytes inverted
							SI32 y; for(y = 0 ; y < bmpProps.size.height; y++){
								const BYTE* srcLine = &bmpData[y * bmpProps.bytesPerLine];
								const BYTE* srcLineNxt = &bmpData[(y + 1) * bmpProps.bytesPerLine];
								BYTE* dstLine = (BYTE*)&img->data[y * img->wpl];
								while(srcLine < srcLineNxt){
									dstLine[0] = srcLine[3];
									dstLine[1] = srcLine[2];
									dstLine[2] = srcLine[1];
									dstLine[3] = srcLine[0];
									srcLine += 4;
									dstLine += 4;
								}
							}
						}
						//PRINTF_INFO("NBOcr, using original 8-bit bitmap (%d x %d).\n", bmpProps.size.width, bmpProps.size.height);
					} else {
						//Creating a 8-bits copy
						if(!NBBitmap_create(&bmpTmp, bmpProps.size.width, bmpProps.size.height, ENNBBitmapColor_GRIS8)){
							PRINTF_ERROR("NBOcr, could not create 8-bit copy of data: '%s' at '%s'.\n", lang3, dataPath);
						} else {
							if(!NBBitmap_pasteBitmapData(&bmpTmp, NBST_P(STNBPointI, 0, 0 ), bmpProps, bmpData, NBST_P(STNBColor8, 255, 255, 255, 255))){
								PRINTF_ERROR("NBOcr, could not paste 8-bit copy of data: '%s' at '%s'.\n", lang3, dataPath);
							} else {
								const STNBBitmapProps bmpProps = NBBitmap_getProps(&bmpTmp);
								const BYTE* bmpData = NBBitmap_getData(&bmpTmp);
								NBASSERT((bmpProps.bytesPerLine % 4) == 0)
								NBASSERT(img->wpl == (bmpProps.bytesPerLine / 4)) //bytes-per-line
								if(bmpPrep == ENOcrSourcePrep_Prepared){
									//Direct copy (non-inverted)
									NBMemory_copy(img->data, bmpData, bmpProps.bytesPerLine * bmpProps.size.height);
								} else {
									//Copy each word-4-bytes inverted
									SI32 y; for(y = 0 ; y < bmpProps.size.height; y++){
										const BYTE* srcLine = &bmpData[y * bmpProps.bytesPerLine];
										const BYTE* srcLineNxt = &bmpData[(y + 1) * bmpProps.bytesPerLine];
										BYTE* dstLine = (BYTE*)&img->data[y * img->wpl];
										while(srcLine < srcLineNxt){
											dstLine[0] = srcLine[3];
											dstLine[1] = srcLine[2];
											dstLine[2] = srcLine[1];
											dstLine[3] = srcLine[0];
											srcLine += 4;
											dstLine += 4;
										}
									}
								}
								PRINTF_INFO("NBOcr, tmp 8-bit bitmap created (%d x %d).\n", bmpProps.size.width, bmpProps.size.height);
							}
						}
					}
					if(img->data != NULL){
						{
							NBThreadMutex_lock(&opq->mutex);
							//Set props
							img->xres = ppi.width;
							img->yres = ppi.height;
							//Set image
							opq->curJob.srcSize.width	= img->w;
							opq->curJob.srcSize.height	= img->h;
							NBThreadMutex_unlock(&opq->mutex);
						}
						TessBaseAPISetImage2(opq->tessApi, img);
						//Process
						{
							{
								ETEXT_DESC mon;
								mon.cancel				= NBOcr_funcCancel_;
								mon.cancel_this			= opq;
								mon.progress_callback2	= NBOcr_funcProgress2_;
								if(TessBaseAPIRecognize(opq->tessApi, &mon) != 0){
									//PRINTF_ERROR("NBOcr, could not process image (recognize): '%s' at '%s'.\n", lang3, dataPath);
								} else {
									NBThreadMutex_lock(&opq->mutex);
									if(opq->cfg.mode == ENOcrSourceMode_OSD){
										//Orientation
										int orient_deg = 0; float orient_conf = 0.0f; /*15 is reasonable*/ const char* script_name = NULL; float script_conf = 0.0f;
										NBMemory_setZeroSt(opq->osdFound, STNBOcrOSD);
										if(!TessBaseAPIDetectOrientationScript(opq->tessApi, &orient_deg, &orient_conf, &script_name, &script_conf)){
											PRINTF_ERROR("NBOcr, could not obtain script's orientation.\n");
										} else {
											opq->osdFound.textDetected = TRUE;
											opq->osdFound.rot 		= (orient_deg == 0 ? 0 : orient_deg - 360); //invert rotation
											opq->osdFound.rotConf	= orient_conf;
											opq->osdFound.script	= script_name;
											opq->osdFound.scriptConf = script_conf;
											PRINTF_INFO("NBOcr, script's orientation detectd: orient_deg(%d) orient_conf(%f/15) script_name('%s') script_conf(%f/15).\n", orient_deg, orient_conf, script_name, script_conf);
											r = TRUE;
										}
									} else {
										//Text
										char* text = TessBaseAPIGetUTF8Text(opq->tessApi);
										if(text == NULL){
											PRINTF_ERROR("NBOcr, could get text after processing image (recognize): '%s' at '%s'.\n", lang3, dataPath);
										} else {
											//PRINTF_INFO("NBOcr, found: '%s'.\n", text);
											NBString_concat(&opq->textFound, text);
											TessDeleteText(text);
											r = TRUE;
										}
									}
									NBThreadMutex_unlock(&opq->mutex);
								}
							}
						}
						TessBaseAPIEnd(opq->tessApi);
					}
					pixDestroy(&img);
					img = NULL;
				}
#				endif
				NBBitmap_release(&bmpTmp);
			}
			//Finish
			{
				//Delete tmp files
				if(wordsFilepath.length > 0 || patternsFilepath.length > 0){
					STNBFilesystem fs;
					NBFilesystem_init(&fs);
					if(wordsFilepath.length > 0){
						NBFilesystem_deleteFile(&fs, wordsFilepath.str);
					}
					if(patternsFilepath.length > 0){
						NBFilesystem_deleteFile(&fs, wordsFilepath.str);
					}
					NBFilesystem_release(&fs);
				}
				NBString_release(&wordsFilepath);
				NBString_release(&patternsFilepath);
			}
		}
		//
		{
			NBThreadMutex_lock(&opq->mutex);
			opq->curJob.progress = 100;
			opq->feedCount++;
			NBThreadMutex_unlock(&opq->mutex);
		}
	}
	return r;
}

SI32 NBOcr_feedBitmap(STNBOcr* obj, const STNBBitmap* bmp, const ENOcrSourcePrep bmpPrep, const STNBSizeI ppi, const char* lang3, const char* dataPath){
	BOOL r = FALSE;
	if(bmp != NULL){
		const STNBBitmapProps bmpProps = NBBitmap_getProps(bmp);
		const BYTE* bmpData = NBBitmap_getData(bmp);
		r = NBOcr_feedBitmapData(obj, bmpProps, bmpData, bmpPrep, ppi, lang3, dataPath);
	}
	return r;
}

SI32 NBOcr_feedBitmapData(STNBOcr* obj, const STNBBitmapProps bmpProps, const BYTE* bmpData, const ENOcrSourcePrep bmpPrep, const STNBSizeI ppi, const char* lang3, const char* dataPath){
	BOOL r = FALSE;
	if(obj != NULL){
		STNBOcrOpq* opq = (STNBOcrOpq*)obj->opaque; NBASSERT(opq != NULL)
		NBThreadMutex_lock(&opq->mutex);
		NBASSERT(opq->curState == ENOcrState_Iddle) //Calling to work while already bussy
		if(opq->curState == ENOcrState_Iddle){
			opq->curState = ENOcrState_Processing;
			{
				NBThreadMutex_unlock(&opq->mutex);
				r = NBOcr_feedBitmapDataUnlockedPriv((STNBOcrOpq*)obj->opaque, bmpProps, bmpData, bmpPrep, ppi, lang3, dataPath);
				NBThreadMutex_lock(&opq->mutex);
			}
			opq->curState = ENOcrState_Iddle;
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}


//Async-exec

void NBOcr_startCanceling(STNBOcr* obj){
	if(obj != NULL){
		STNBOcrOpq* opq = (STNBOcrOpq*)obj->opaque; NBASSERT(opq != NULL)
		//Do not lock, for smoothness
		//NBThreadMutex_lock(&opq->mutex);
		{
			opq->curJob.canceling = TRUE;
		}
		//NBThreadMutex_unlock(&opq->mutex);
	}
}

typedef struct STNBOcrJobParam_ {
	STNBOcrOpq*			opq;
	STNBBitmap*			cloneBmp;		//if bitmap was cloned
	STNBBitmapProps		srcBmpProps;
	const BYTE*			srcBmpData;
	ENOcrSourcePrep		srcBmpPrep;		//already inverted
	STNBSizeI			ppi;
	STNBString			lang3;
	STNBString			dataPath;
} STNBOcrJobParam;

SI64 NBOcr_feedBitmapAndRelease(STNBThread* thread, void* pParam){
	SI64 r = 0;
	STNBOcrJobParam* param = (STNBOcrJobParam*)pParam;
	if(param != NULL){
		//Process
		NBThreadMutex_lock(&param->opq->mutex);
		{
			NBASSERT(param->opq->curState == ENOcrState_Processing) //Must be already set by parent
			param->opq->curState = ENOcrState_Processing;
			{
				NBThreadMutex_unlock(&param->opq->mutex);
				NBOcr_feedBitmapDataUnlockedPriv(param->opq, param->srcBmpProps, param->srcBmpData, param->srcBmpPrep, param->ppi, param->lang3.str, param->dataPath.str);
				NBThreadMutex_lock(&param->opq->mutex);
			}
			//Release data
			{
				NBString_release(&param->lang3);
				NBString_release(&param->dataPath);
				//Cloned bitmap
				if(param->cloneBmp != NULL){
					NBBitmap_release(param->cloneBmp);
					NBMemory_free(param->cloneBmp);
					param->cloneBmp = NULL;
				}
			}
			param->opq->curState = ENOcrState_Iddle;
		}
		NBThreadMutex_unlock(&param->opq->mutex);
		//Release param
		{
			NBASSERT(param->cloneBmp == NULL)
			NBMemory_free(param);
			param = NULL;
		}
	}
	//Release thread
	if(thread != NULL){
		NBThread_release(thread);
		NBMemory_free(thread);
		thread = NULL;
	}
	return r;
}

SI32 NBOcr_feedBitmapAsync(STNBOcr* obj, const STNBBitmap* bmp, const ENOcrSourcePrep bmpPrep, const ENOcrSourceAction bmpAction, const STNBSizeI ppi, const char* lang3, const char* dataPath){
	BOOL r = FALSE;
	if(bmp != NULL){
		const STNBBitmapProps bmpProps = NBBitmap_getProps(bmp);
		const BYTE* bmpData = NBBitmap_getData(bmp);
		r = NBOcr_feedBitmapDataAsync(obj, bmpProps, bmpData, bmpPrep, bmpAction, ppi, lang3, dataPath);
	}
	return r;
}

SI32 NBOcr_feedBitmapDataAsyncLockedPriv(STNBOcrOpq* opq, const STNBBitmapProps bmpProps, const BYTE* bmpData, const ENOcrSourcePrep bmpPrep, const ENOcrSourceAction bmpDataAction, const STNBSizeI ppi, const char* lang3, const char* dataPath){
	BOOL r = FALSE;
	NBASSERT(opq != NULL)
	if(opq != NULL && bmpData != NULL){
		NBASSERT(opq->curState == ENOcrState_Iddle) //Calling to work while already bussy
		{
			STNBOcrJobParam* param = NBMemory_allocType(STNBOcrJobParam);
			param->opq			= opq;
			param->cloneBmp 	= NULL;
			param->srcBmpData	= NULL;
			param->ppi			= ppi;
			NBString_initWithStr(&param->lang3, lang3);
			NBString_initWithStr(&param->dataPath, dataPath);
			if(bmpDataAction == ENOcrSourceAction_UseAndTrust){
				//Use and trust data for async operation
				param->srcBmpProps	= bmpProps;
				param->srcBmpData	= bmpData;
				param->srcBmpPrep	= bmpPrep;
			} else {
				//Clone
				param->cloneBmp = NBMemory_allocType(STNBBitmap);
				NBBitmap_init(param->cloneBmp);
				if(NBBitmap_create(param->cloneBmp, bmpProps.size.width, bmpProps.size.height, bmpProps.color)){
					if(NBBitmap_pasteBitmapData(param->cloneBmp, NBST_P(STNBPointI, 0, 0 ), bmpProps, bmpData, NBST_P(STNBColor8, 255, 255, 255, 255 ))){
						param->srcBmpProps	= NBBitmap_getProps(param->cloneBmp);
						param->srcBmpData	= NBBitmap_getData(param->cloneBmp);
						param->srcBmpPrep	= bmpPrep;
					}
				}
			}
			//Process
			if(param->srcBmpData != NULL){
				STNBThread* thread = NBMemory_allocType(STNBThread);
				NBThread_init(thread);
				NBThread_setIsJoinable(thread, FALSE);
				param->opq->curState = ENOcrState_Processing;
				if(!NBThread_start(thread, NBOcr_feedBitmapAndRelease, param, NULL)){
					param->opq->curState = ENOcrState_Iddle;
				} else {
					r = TRUE;
				}
			}
			//Release orphans
			if(!r){
				NBString_release(&param->lang3);
				NBString_release(&param->dataPath);
				if(param->cloneBmp != NULL){
					NBBitmap_release(param->cloneBmp);
					NBMemory_free(param->cloneBmp);
					param->cloneBmp = NULL;
				}
				if(param != NULL){
					NBMemory_free(param);
					param = NULL;
				}
			}
		}
	}
	return r;
}

SI32 NBOcr_feedBitmapDataAsync(STNBOcr* obj, const STNBBitmapProps bmpProps, const BYTE* bmpData, const ENOcrSourcePrep bmpPrep, const ENOcrSourceAction bmpDataAction, const STNBSizeI ppi, const char* lang3, const char* dataPath){
	BOOL r = FALSE;
	if(obj != NULL && bmpData != NULL){
		STNBOcrOpq* opq = (STNBOcrOpq*)obj->opaque; NBASSERT(opq != NULL)
		NBThreadMutex_lock(&opq->mutex);
		NBASSERT(opq->curState == ENOcrState_Iddle) //Calling to work while already bussy
		if(opq->curState == ENOcrState_Iddle){
			r = NBOcr_feedBitmapDataAsyncLockedPriv(opq, bmpProps, bmpData, bmpPrep, bmpDataAction, ppi, lang3, dataPath);
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}

#endif //#ifndef NB_COMPILE_DRIVER_MODE
