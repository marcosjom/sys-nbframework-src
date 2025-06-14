//
//  NBOcrMultiRot.h
//  nbframework
//
//  Created by Marcos Ortega on 11/2/19.
//

#ifndef NBOcrMultiRot_h
#define NBOcrMultiRot_h

#include "NBFrameworkDefs.h"
#include "ocr/NBOcr.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBOcrMultiRot_ {
		void* opaque;
	} STNBOcrMultiRot;
	
	void NBOcrMultiRot_init(STNBOcrMultiRot* obj);
	void NBOcrMultiRot_retain(STNBOcrMultiRot* obj);
	void NBOcrMultiRot_release(STNBOcrMultiRot* obj);
	
	//Config
	void NBOcrMultiRot_emptyCustomCfg(STNBOcrMultiRot* obj);
	void NBOcrMultiRot_setOnlyCustomDict(STNBOcrMultiRot* obj, const BOOL onlyCustomDict);
	void NBOcrMultiRot_setTmpFolderPath(STNBOcrMultiRot* obj, const char* path);
	void NBOcrMultiRot_addCustomCharsToWhitelist(STNBOcrMultiRot* obj, const char* str, const BOOL includeOtherCase);
	void NBOcrMultiRot_addCustomWords(STNBOcrMultiRot* obj, const char* words);
	void NBOcrMultiRot_addCustomPatterns(STNBOcrMultiRot* obj, const char* patterns);
	
	//Prepare bitmap data (invert words)
	BOOL NBOcrMultiRot_ocrPrepareBitmap(const STNBBitmap* bmp);
	BOOL NBOcrMultiRot_ocrPrepareBitmapData(const STNBBitmapProps bmpProps, BYTE* bmpData);
	
	//All-jobs
	ENOcrState NBOcrMultiRot_getState(const STNBOcrMultiRot* obj);
	SI32 NBOcrMultiRot_getRotsCount(const STNBOcrMultiRot* obj);
	SI32 NBOcrMultiRot_getProgress(const STNBOcrMultiRot* obj);
	SI32 NBOcrMultiRot_getFeedCount(const STNBOcrMultiRot* obj);	//How many time a bitmap has been feed
	
	//Specific-job
	ENOcrState NBOcrMultiRot_getRotState(const STNBOcrMultiRot* obj, const SI32 iRot);
	SI32 NBOcrMultiRot_getRotProgress(const STNBOcrMultiRot* obj, const SI32 iRot, STNBSizeI* dstCurSize, STNBAABoxI* dstCurArea);		//0-100
	SI32 NBOcrMultiRot_getRotFeedCount(const STNBOcrMultiRot* obj, const SI32 iRot);	//How many time a bitmap has been feed
	SI32 NBOcrMultiRot_getRotTextFound(const STNBOcrMultiRot* obj, const SI32 iRot, STNBString* dst);	//Accumlated text

	BOOL NBOcrMultiRot_addRotDeg(STNBOcrMultiRot* obj, const float deg);
	
	//Sync
	SI32 NBOcrMultiRot_feedBitmap(STNBOcrMultiRot* obj, const STNBBitmap* bmp, const ENOcrSourcePrep bmpPrep, const STNBSizeI ppi, const char* lang3, const char* dataPath);
	SI32 NBOcrMultiRot_feedBitmapData(STNBOcrMultiRot* obj, const STNBBitmapProps bmpProps, const BYTE* bmpData, const ENOcrSourcePrep bmpPrep, const STNBSizeI ppi, const char* lang3, const char* dataPath);
	
	//Async
	void NBOcrMultiRot_startCanceling(STNBOcrMultiRot* obj);
	void NBOcrMultiRot_waitForAll(STNBOcrMultiRot* obj);
	SI32 NBOcrMultiRot_feedBitmapAsync(STNBOcrMultiRot* obj, const STNBBitmap* bmp, const ENOcrSourcePrep bmpPrep, const ENOcrSourceAction bmpAction, const STNBSizeI ppi, const char* lang3, const char* dataPath);
	SI32 NBOcrMultiRot_feedBitmapDataAsync(STNBOcrMultiRot* obj, const STNBBitmapProps bmpProps, const BYTE* bmpData, const ENOcrSourcePrep bmpPrep, const ENOcrSourceAction bmpDataAction, const STNBSizeI ppi, const char* lang3, const char* dataPath);
	
#ifdef __cplusplus
} // extern "C"
#endif

#endif /* NBOcrMultiRot_h */
