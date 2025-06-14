//
//  NBOcrMultiRot.c
//  nbframework
//
//  Created by Marcos Ortega on 11/2/19.
//

#include "NBFrameworkPch.h"
#include "ocr/NBOcrMultiRot.h"
//
#include "NBMemory.h"
#include "NBArray.h"
#include "NBThread.h"
#include "NBEncoding.h"

typedef struct STNBOcrRot_ {
	float		deg;
	STNBOcr		ocr;
	STNBBitmap	bmp;
} STNBOcrRot;

typedef struct STNBOcrMultiRotOpq_ {
	STNBArray		rots;			//STNBOcrRot
	ENOcrState		curState;
	SI32			feedCount;		//How many time a bitmap has been feed
	BOOL			canceling;
	//Config
	struct {
		BOOL			onlyCustomDict;
		STNBString		tmpFolderPath;			//for tmp files
		STNBString		customCharsWhiteList;	//chars to search
		STNBString		customWords;			//word to search
		STNBString		customPatterns;			//patterns to search
	} cfg;
	STNBThreadMutex	mutex;
	SI32			retainCount;
} STNBOcrMultiRotOpq;

void NBOcrMultiRot_init(STNBOcrMultiRot* obj){
	STNBOcrMultiRotOpq* opq 	= NBMemory_allocType(STNBOcrMultiRotOpq);
	obj->opaque			= opq;
	//
	NBArray_init(&opq->rots, sizeof(STNBOcrRot), NULL);
	opq->curState		= ENOcrState_Iddle;
	opq->feedCount		= 0;
	opq->canceling		= FALSE;
	//Config
	{
		opq->cfg.onlyCustomDict	= FALSE;
		NBString_init(&opq->cfg.tmpFolderPath);			//for tmp files
		NBString_init(&opq->cfg.customCharsWhiteList);	//chars to search
		NBString_init(&opq->cfg.customWords);			//word to search
		NBString_init(&opq->cfg.customPatterns);		//patterns to search
	}
	NBThreadMutex_init(&opq->mutex);
	opq->retainCount = 1;
}

void NBOcrMultiRot_retain(STNBOcrMultiRot* obj){
	STNBOcrMultiRotOpq* opq = (STNBOcrMultiRotOpq*)obj->opaque; NBASSERT(opq != NULL)
	NBThreadMutex_lock(&opq->mutex);
	{
		NBASSERT(opq->retainCount > 0)
		opq->retainCount++;
	}
	NBThreadMutex_unlock(&opq->mutex);
}

void NBOcrMultiRot_startCancelingLockedPriv(STNBOcrMultiRotOpq* opq){
	opq->canceling = TRUE;
	SI32 i; for(i = 0 ; i < opq->rots.use; i++){
		STNBOcrRot* rot = NBArray_itmPtrAtIndex(&opq->rots, STNBOcrRot, i);
		NBOcr_startCanceling(&rot->ocr);
	}
}

void NBOcrMultiRot_waitForAllLockedPriv(STNBOcrMultiRotOpq* opq, const BOOL includeMySelf){
	//Wait untill all finished
	{
		SI32 qActive;
		do {
			qActive = 0;
			{
				SI32 i; for(i = 0 ; i < opq->rots.use; i++){
					STNBOcrRot* rot = NBArray_itmPtrAtIndex(&opq->rots, STNBOcrRot, i);
					if(NBOcr_getState(&rot->ocr) == ENOcrState_Processing){
						qActive++;
					}
				}
				if(includeMySelf && opq->curState == ENOcrState_Processing){
					qActive++;
				}
				//To allow changes in variables/states
				NBThreadMutex_unlock(&opq->mutex);
				NBThreadMutex_lock(&opq->mutex);
			}
		} while(qActive != 0);
	}
}

void NBOcrMultiRot_release(STNBOcrMultiRot* obj){
	STNBOcrMultiRotOpq* opq = (STNBOcrMultiRotOpq*)obj->opaque; NBASSERT(opq != NULL)
	if(opq != NULL){
		NBThreadMutex_lock(&opq->mutex);
		NBASSERT(opq->retainCount > 0)
		opq->retainCount--;
		if(opq->retainCount > 0){
			NBThreadMutex_unlock(&opq->mutex);
		} else {
			//Wait untill all finished
			{
				NBOcrMultiRot_startCancelingLockedPriv(opq);
				NBOcrMultiRot_waitForAllLockedPriv(opq, TRUE); //include myseld
				NBASSERT(opq->curState == ENOcrState_Iddle)
			}
			//Rots
			{
				SI32 i; for(i = 0 ; i < opq->rots.use; i++){
					STNBOcrRot* rot = NBArray_itmPtrAtIndex(&opq->rots, STNBOcrRot, i);
					NBASSERT(NBOcr_getState(&rot->ocr) == ENOcrState_Iddle)
					NBOcr_release(&rot->ocr);
					NBBitmap_release(&rot->bmp);
				}
				NBArray_empty(&opq->rots);
				NBArray_release(&opq->rots);
			}
			//Config
			{
				opq->cfg.onlyCustomDict	= FALSE;
				NBString_release(&opq->cfg.tmpFolderPath);			//for tmp files
				NBString_release(&opq->cfg.customCharsWhiteList);	//chars to search
				NBString_release(&opq->cfg.customWords);			//word to search
				NBString_release(&opq->cfg.customPatterns);		//patterns to search
			}
			NBThreadMutex_unlock(&opq->mutex);
			NBThreadMutex_release(&opq->mutex);
			NBMemory_free(obj->opaque);
			obj->opaque		= NULL;
		}
	}
}

//Config

void NBOcrMultiRot_emptyCustomCfg(STNBOcrMultiRot* obj){
	if(obj != NULL){
		STNBOcrMultiRotOpq* opq = (STNBOcrMultiRotOpq*)obj->opaque; NBASSERT(opq != NULL)
		NBThreadMutex_lock(&opq->mutex);
		{
			opq->cfg.onlyCustomDict	= FALSE;
			NBString_empty(&opq->cfg.customCharsWhiteList);	//chars to search
			NBString_empty(&opq->cfg.customWords);			//word to search
			NBString_empty(&opq->cfg.customPatterns);		//patterns to search
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
}

void NBOcrMultiRot_setOnlyCustomDict(STNBOcrMultiRot* obj, const BOOL onlyCustomDict){
	if(obj != NULL){
		STNBOcrMultiRotOpq* opq = (STNBOcrMultiRotOpq*)obj->opaque; NBASSERT(opq != NULL)
		NBThreadMutex_lock(&opq->mutex);
		{
			opq->cfg.onlyCustomDict	= onlyCustomDict;
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
}

void NBOcrMultiRot_setTmpFolderPath(STNBOcrMultiRot* obj, const char* path){
	if(obj != NULL){
		STNBOcrMultiRotOpq* opq = (STNBOcrMultiRotOpq*)obj->opaque; NBASSERT(opq != NULL)
		NBThreadMutex_lock(&opq->mutex);
		{
			NBString_set(&opq->cfg.tmpFolderPath, path); //for tmp files
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
}

void NBOcrMultiRot_addCustomCharsToWhitelist(STNBOcrMultiRot* obj, const char* str, const BOOL includeOtherCase){
	if(obj != NULL){
		STNBOcrMultiRotOpq* opq = (STNBOcrMultiRotOpq*)obj->opaque; NBASSERT(opq != NULL)
		NBThreadMutex_lock(&opq->mutex);
		if(str != NULL){
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
void NBOcrMultiRot_addCustomWords(STNBOcrMultiRot* obj, const char* words){
	if(obj != NULL){
		STNBOcrMultiRotOpq* opq = (STNBOcrMultiRotOpq*)obj->opaque; NBASSERT(opq != NULL)
		NBThreadMutex_lock(&opq->mutex);
		{
			NBString_concat(&opq->cfg.customWords, words);			//word to search
			//Add '\n' if necesary
			if(opq->cfg.customWords.lenght > 0){
				if(opq->cfg.customWords.str[opq->cfg.customWords.lenght - 1] != '\n'){
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
void NBOcrMultiRot_addCustomPatterns(STNBOcrMultiRot* obj, const char* patterns){
	if(obj != NULL){
		STNBOcrMultiRotOpq* opq = (STNBOcrMultiRotOpq*)obj->opaque; NBASSERT(opq != NULL)
		NBThreadMutex_lock(&opq->mutex);
		{
			NBString_concat(&opq->cfg.customPatterns, patterns);			//word to search
			//Add '\n' if necesary
			if(opq->cfg.customPatterns.lenght > 0){
				if(opq->cfg.customPatterns.str[opq->cfg.customPatterns.lenght - 1] != '\n'){
					NBString_concatByte(&opq->cfg.customPatterns, '\n');
				}
			}
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
}

//Prepare bitmap data (invert words)
BOOL NBOcrMultiRot_ocrPrepareBitmap(const STNBBitmap* bmp){
	return NBOcr_ocrPrepareBitmap(bmp);
}

BOOL NBOcrMultiRot_ocrPrepareBitmapData(const STNBBitmapProps bmpProps, BYTE* bmpData){
	return NBOcr_ocrPrepareBitmapData(bmpProps, bmpData);
}

//All-jobs

ENOcrState NBOcrMultiRot_getState(const STNBOcrMultiRot* obj){
	ENOcrState r = ENOcrState_Iddle;
	STNBOcrMultiRotOpq* opq = (STNBOcrMultiRotOpq*)obj->opaque; NBASSERT(opq != NULL)
	if(opq != NULL){
		NBThreadMutex_lock(&opq->mutex);
		NBASSERT(opq->retainCount > 0)
		{
			r = opq->curState;
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}

SI32 NBOcrMultiRot_getRotsCount(const STNBOcrMultiRot* obj){
	SI32 r = 0;
	STNBOcrMultiRotOpq* opq = (STNBOcrMultiRotOpq*)obj->opaque; NBASSERT(opq != NULL)
	if(opq != NULL){
		NBThreadMutex_lock(&opq->mutex);
		NBASSERT(opq->retainCount > 0)
		{
			r = opq->rots.use;
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}

SI32 NBOcrMultiRot_getProgress(const STNBOcrMultiRot* obj){
	SI32 r = 0;
	STNBOcrMultiRotOpq* opq = (STNBOcrMultiRotOpq*)obj->opaque; NBASSERT(opq != NULL)
	if(opq != NULL){
		NBThreadMutex_lock(&opq->mutex);
		NBASSERT(opq->retainCount > 0)
		{
			SI32 i, total = 0, count = 0;
			for(i = 0 ; i < opq->rots.use; i++){
				const STNBOcrRot* rot = NBArray_itmPtrAtIndex(&opq->rots, STNBOcrRot, i);
				total += NBOcr_getProgress(&rot->ocr, NULL, NULL);
				count++;
			}
			if(count != 0){
				r = total / count;
			}
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}

SI32 NBOcrMultiRot_getFeedCount(const STNBOcrMultiRot* obj){	//How many time a bitmap has been feed
	SI32 r = 0;
	STNBOcrMultiRotOpq* opq = (STNBOcrMultiRotOpq*)obj->opaque; NBASSERT(opq != NULL)
	if(opq != NULL){
		NBThreadMutex_lock(&opq->mutex);
		NBASSERT(opq->retainCount > 0)
		{
			r = opq->feedCount;
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}

//Specific-job

ENOcrState NBOcrMultiRot_getRotState(const STNBOcrMultiRot* obj, const SI32 iRot){
	ENOcrState r = ENOcrState_Iddle;
	STNBOcrMultiRotOpq* opq = (STNBOcrMultiRotOpq*)obj->opaque; NBASSERT(opq != NULL)
	if(opq != NULL){
		NBThreadMutex_lock(&opq->mutex);
		NBASSERT(opq->retainCount > 0)
		if(iRot >= 0 && iRot < opq->rots.use){
			const STNBOcrRot* rot = NBArray_itmPtrAtIndex(&opq->rots, STNBOcrRot, iRot);
			r = NBOcr_getState(&rot->ocr);
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}

SI32 NBOcrMultiRot_getRotProgress(const STNBOcrMultiRot* obj, const SI32 iRot, STNBSizeI* dstCurSize, STNBAABoxI* dstCurArea){
	SI32 r = 0;
	STNBOcrMultiRotOpq* opq = (STNBOcrMultiRotOpq*)obj->opaque; NBASSERT(opq != NULL)
	if(opq != NULL){
		NBThreadMutex_lock(&opq->mutex);
		NBASSERT(opq->retainCount > 0)
		if(iRot >= 0 && iRot < opq->rots.use){
			const STNBOcrRot* rot = NBArray_itmPtrAtIndex(&opq->rots, STNBOcrRot, iRot);
			r = NBOcr_getProgress(&rot->ocr, dstCurSize, dstCurArea);
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}

SI32 NBOcrMultiRot_getRotFeedCount(const STNBOcrMultiRot* obj, const SI32 iRot){	//How many time a bitmap has been feed
	SI32 r = 0;
	STNBOcrMultiRotOpq* opq = (STNBOcrMultiRotOpq*)obj->opaque; NBASSERT(opq != NULL)
	if(opq != NULL){
		NBThreadMutex_lock(&opq->mutex);
		NBASSERT(opq->retainCount > 0)
		if(iRot >= 0 && iRot < opq->rots.use){
			const STNBOcrRot* rot = NBArray_itmPtrAtIndex(&opq->rots, STNBOcrRot, iRot);
			r = NBOcr_getFeedCount(&rot->ocr);
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}
	
SI32 NBOcrMultiRot_getRotTextFound(const STNBOcrMultiRot* obj, const SI32 iRot, STNBString* dst){
	SI32 r = 0;
	STNBOcrMultiRotOpq* opq = (STNBOcrMultiRotOpq*)obj->opaque; NBASSERT(opq != NULL)
	if(opq != NULL){
		NBThreadMutex_lock(&opq->mutex);
		NBASSERT(opq->retainCount > 0)
		if(iRot >= 0 && iRot < opq->rots.use){
			const STNBOcrRot* rot = NBArray_itmPtrAtIndex(&opq->rots, STNBOcrRot, iRot);
			r = NBOcr_getTextFound(&rot->ocr, dst);
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}

//

BOOL NBOcrMultiRot_addRotDeg(STNBOcrMultiRot* obj, const float deg){
	BOOL r = FALSE;
	STNBOcrMultiRotOpq* opq = (STNBOcrMultiRotOpq*)obj->opaque; NBASSERT(opq != NULL)
	if(opq != NULL){
		NBThreadMutex_lock(&opq->mutex);
		NBASSERT(opq->retainCount > 0)
		if(opq->curState == ENOcrState_Iddle){
			STNBOcrRot rot;
			rot.deg	= deg;
			NBOcr_init(&rot.ocr);
			NBBitmap_init(&rot.bmp);
			NBArray_addValue(&opq->rots, rot);
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}

//Sync-exec

SI32 NBOcrMultiRot_feedBitmapDataLockedPriv(STNBOcrMultiRotOpq* opq, const STNBBitmapProps bmpProps, const BYTE* bmpData, const ENOcrSourcePrep bmpPrep, const STNBSizeI ppi, const char* lang3, const char* dataPath){
	BOOL r = FALSE;
	NBASSERT(opq != NULL)
	if(opq != NULL){
		NBASSERT(opq->retainCount > 0)
		NBASSERT(opq->curState == ENOcrState_Processing) //Must be set by caller
		if(bmpProps.size.width > 0 && bmpProps.size.height > 0 && bmpData != NULL){
			//Do all rots async
			SI32 i; for(i = 0 ; i < opq->rots.use && !opq->canceling; i++){
				STNBOcrRot* rot = NBArray_itmPtrAtIndex(&opq->rots, STNBOcrRot, i);
				NBASSERT(NBOcr_getState(&rot->ocr) == ENOcrState_Iddle)
				//Create rotated bitmap
				const SI32 rotI		= (SI32)rot->deg;
				const SI32 after90	= (rotI % 90);
				if(after90 == 0){
					//Is a perfect rotation
					SI32 rotP			= rotI;
					while(rotP > 0)		rotP -= 360;
					while(rotP <= -360)	rotP += 360.0f;
					NBASSERT(rotP > -360 && rotP <= 0);
					const UI8 rotsRight	= (-rotP / 90);
					if(!NBBitmap_createWithBitmapDataRotatedRight90(&rot->bmp, ENNBBitmapColor_GRIS8, bmpProps, bmpData, (const STNBColor8){ 255, 255, 255, 255 }, rotsRight)){
						NBASSERT(FALSE)
					} else {
						//Done
						PRINTF_INFO("Created bitmap (%.2f deg) rotating to right %d times.\n", rot->deg, rotsRight);
					}
				} else {
					//Is a not-90 rotation
					//ToDo: calculate best size to all rotated content
					if(!NBBitmap_createAndSet(&rot->bmp, bmpProps.size.width, bmpProps.size.height, ENNBBitmapColor_GRIS8, 255)){
						NBASSERT(FALSE)
					} else {
						if(!NBBitmap_drawBitmapData(&rot->bmp, bmpProps, bmpData, (const STNBPoint){ bmpProps.size.width / 2, bmpProps.size.height  / 2}, ((rot->deg) * PI / 180.0f), (const STNBSize){ 1, 1 }, (const STNBColor8){ 255, 255, 255, 255})){
							NBASSERT(FALSE)
						} else {
							//Done
							PRINTF_INFO("Created bitmap (%.2f deg) drawing rotated.\n", rot->deg);
						}
					}
				}
				//Config
				{
					NBOcr_emptyCustomCfg(&rot->ocr);
					NBOcr_setOnlyCustomDict(&rot->ocr, opq->cfg.onlyCustomDict);
					NBOcr_setTmpFolderPath(&rot->ocr, opq->cfg.tmpFolderPath.str);
					NBOcr_addCustomCharsToWhitelist(&rot->ocr, opq->cfg.customCharsWhiteList.str, FALSE);
					NBOcr_addCustomWords(&rot->ocr, opq->cfg.customWords.str);
					NBOcr_addCustomPatterns(&rot->ocr, opq->cfg.customPatterns.str);
				}
				//Start processing async
				NBOcr_feedBitmapAsync(&rot->ocr, &rot->bmp, bmpPrep, ENOcrSourceAction_UseAndTrust, ppi, lang3, dataPath);
				NBASSERT(NBOcr_getState(&rot->ocr) == ENOcrState_Processing)
				PRINTF_INFO("Started job #%d / %d; rot(%f).\n", (i + 1), opq->rots.use, rot->deg);
				//Unlock-lock (to allow change of 'canceling' value)
				NBThreadMutex_unlock(&opq->mutex);
				NBThreadMutex_lock(&opq->mutex);
			}
			//Wait for all children
			NBOcrMultiRot_waitForAllLockedPriv(opq, FALSE); //do not include myself
			//Feed-count
			opq->feedCount++;
		}
	}
	return r;
}

SI32 NBOcrMultiRot_feedBitmap(STNBOcrMultiRot* obj, const STNBBitmap* bmp, const ENOcrSourcePrep bmpPrep, const STNBSizeI ppi, const char* lang3, const char* dataPath){
	BOOL r = FALSE;
	if(bmp != NULL){
		const STNBBitmapProps bmpProps = NBBitmap_getProps(bmp);
		const BYTE* bmpData = NBBitmap_getData(bmp);
		r = NBOcrMultiRot_feedBitmapData(obj, bmpProps, bmpData, bmpPrep, ppi, lang3, dataPath);
	}
	return r;
}

SI32 NBOcrMultiRot_feedBitmapData(STNBOcrMultiRot* obj, const STNBBitmapProps bmpProps, const BYTE* bmpData, const ENOcrSourcePrep bmpPrep, const STNBSizeI ppi, const char* lang3, const char* dataPath){
	BOOL r = FALSE;
	STNBOcrMultiRotOpq* opq = (STNBOcrMultiRotOpq*)obj->opaque; NBASSERT(opq != NULL)
	if(opq != NULL){
		NBThreadMutex_lock(&opq->mutex);
		NBASSERT(opq->retainCount > 0)
		if(opq->curState == ENOcrState_Iddle){
			opq->curState = ENOcrState_Processing;
			r = NBOcrMultiRot_feedBitmapDataLockedPriv(opq, bmpProps, bmpData, bmpPrep, ppi, lang3, dataPath);
			opq->curState = ENOcrState_Iddle;
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}



//Async-exec

void NBOcrMultiRot_startCanceling(STNBOcrMultiRot* obj){
	STNBOcrMultiRotOpq* opq = (STNBOcrMultiRotOpq*)obj->opaque; NBASSERT(opq != NULL)
	if(opq != NULL){
		NBThreadMutex_lock(&opq->mutex);
		NBASSERT(opq->retainCount > 0)
		{
			NBOcrMultiRot_startCancelingLockedPriv(opq);
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
}

void NBOcrMultiRot_waitForAll(STNBOcrMultiRot* obj){
	STNBOcrMultiRotOpq* opq = (STNBOcrMultiRotOpq*)obj->opaque; NBASSERT(opq != NULL)
	if(opq != NULL){
		NBThreadMutex_lock(&opq->mutex);
		NBASSERT(opq->retainCount > 0)
		{
			NBOcrMultiRot_waitForAllLockedPriv(opq, TRUE); //including myself
			NBASSERT(opq->curState == ENOcrState_Iddle)
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
}

typedef struct STNBOcrMultiRotJobParam_ {
	STNBOcrMultiRotOpq*		opq;
	STNBBitmap*				cloneBmp;		//if bitmap was cloned
	STNBBitmapProps			srcBmpProps;
	const BYTE*				srcBmpData;
	ENOcrSourcePrep			srcBmpPrep;	//already inverted
	STNBSizeI				ppi;
	STNBString				lang3;
	STNBString				dataPath;
} STNBOcrMultiRotJobParam;

SI64 NBOcrMultiRot_feedBitmapAndRelease(STNBThread* thread, void* pParam){
	SI64 r = 0;
	STNBOcrMultiRotJobParam* param = (STNBOcrMultiRotJobParam*)pParam;
	if(param != NULL){
		//Process
		NBThreadMutex_lock(&param->opq->mutex);
		{
			NBASSERT(param->opq->curState == ENOcrState_Processing) //Must be already set by parent
			param->opq->curState = ENOcrState_Processing;
			NBOcrMultiRot_feedBitmapDataLockedPriv(param->opq, param->srcBmpProps, param->srcBmpData, param->srcBmpPrep, param->ppi, param->lang3.str, param->dataPath.str);
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

SI32 NBOcrMultiRot_feedBitmapAsync(STNBOcrMultiRot* obj, const STNBBitmap* bmp, const ENOcrSourcePrep bmpPrep, const ENOcrSourceAction bmpAction, const STNBSizeI ppi, const char* lang3, const char* dataPath){
	BOOL r = FALSE;
	if(bmp != NULL){
		const STNBBitmapProps bmpProps = NBBitmap_getProps(bmp);
		const BYTE* bmpData = NBBitmap_getData(bmp);
		r = NBOcrMultiRot_feedBitmapDataAsync(obj, bmpProps, bmpData, bmpPrep, bmpAction, ppi, lang3, dataPath);
	}
	return r;
}

SI32 NBOcrMultiRot_feedBitmapDataAsync(STNBOcrMultiRot* obj, const STNBBitmapProps bmpProps, const BYTE* bmpData, const ENOcrSourcePrep bmpPrep, const ENOcrSourceAction bmpDataAction, const STNBSizeI ppi, const char* lang3, const char* dataPath){
	BOOL r = FALSE;
	if(obj != NULL && bmpData != NULL){
		STNBOcrMultiRotOpq* opq = (STNBOcrMultiRotOpq*)obj->opaque; NBASSERT(opq != NULL)
		NBThreadMutex_lock(&opq->mutex);
		NBASSERT(opq->curState == ENOcrState_Iddle) //Calling to work while already bussy
		if(opq->curState == ENOcrState_Iddle){
			STNBOcrMultiRotJobParam* param = NBMemory_allocType(STNBOcrMultiRotJobParam);
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
					if(NBBitmap_pasteBitmapData(param->cloneBmp, (const STNBPointI){ 0, 0 }, bmpProps, bmpData, (const STNBColor8){ 255, 255, 255, 255 })){
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
				if(!NBThread_start(thread, NBOcrMultiRot_feedBitmapAndRelease, param, NULL)){
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
		NBThreadMutex_unlock(&opq->mutex);
	}
	return r;
}
