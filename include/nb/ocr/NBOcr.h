//
//  NBOcr.h
//  nbframework
//
//  Created by Marcos Ortega on 11/2/19.
//

#ifndef NBOcr_h
#define NBOcr_h

#include "nb/NBFrameworkDefs.h"

#ifndef NB_COMPILE_DRIVER_MODE

#include "nb/core/NBString.h"
#include "nb/2d/NBBitmap.h"

#ifdef __cplusplus
#define NB_EXTERNC extern "C"
#else
#define NB_EXTERNC
#endif

typedef enum ENOcrState_ {
	ENOcrState_Iddle = 0,	//Not working
	ENOcrState_Processing	//Working
} ENOcrState;

typedef enum ENOcrSourcePrep_ {
	ENOcrSourcePrep_Unprepared = 0,	//Data was not prepared yet (4-bytes-words inverted)
	ENOcrSourcePrep_Prepared,		//Data was prepared yet (4-bytes-words inverted)
} ENOcrSourcePrep;

typedef enum ENOcrSourceAction_ {
	ENOcrSourceAction_Clone = 0,	//Clone the bitmap, dont trust the parameter with persist.
	ENOcrSourceAction_UseAndTrust,	//Use the given data, trust it will exist and wont be modified during the analysis (this avoids making a copy).
} ENOcrSourceAction;

typedef enum ENOcrSourceMode_ {
	ENOcrSourceMode_OSD = 0,		//Orientation and Scripting (ex: Latin) Detection
	ENOcrSourceMode_TextAuto,		//Search of text and organization
	ENOcrSourceMode_SingleColumn,	//Single column of text of variable sizes.
	ENOcrSourceMode_SingleLine		//Single text line.
} ENOcrSourceMode;

typedef struct STNBOcrOSD_ {
	BOOL		textDetected;
	SI32		rot;
	float		rotConf;	//15 is trusty
	const char*	script;		//Ex: "latin"
	float		scriptConf;	//15 is trusty
} STNBOcrOSD;

typedef struct STNBOcr_ {
	void* opaque;
} STNBOcr;
		
NB_EXTERNC void NBOcr_init(STNBOcr* obj);
NB_EXTERNC void NBOcr_retain(STNBOcr* obj);
NB_EXTERNC void NBOcr_release(STNBOcr* obj);

//Config
NB_EXTERNC SI32 NBOcr_isSupported(void);    //determines if the current compilation supoports NBOCR, if not all methods will return error or do-nothing.
NB_EXTERNC void NBOcr_setMode(STNBOcr* obj, const ENOcrSourceMode mode);
NB_EXTERNC void NBOcr_emptyCustomCfg(STNBOcr* obj);
NB_EXTERNC void NBOcr_setOnlyCustomDict(STNBOcr* obj, const SI32 onlyCustomDict);
NB_EXTERNC void NBOcr_setTmpFolderPath(STNBOcr* obj, const char* path);
NB_EXTERNC void NBOcr_addCustomCharsToWhitelist(STNBOcr* obj, const char* str, const SI32 includeOtherCase);
NB_EXTERNC void NBOcr_addCustomWords(STNBOcr* obj, const char* words);
NB_EXTERNC void NBOcr_addCustomPatterns(STNBOcr* obj, const char* patterns);
NB_EXTERNC void NBOcr_empty(const STNBOcr* obj); //Accumlated text

//
NB_EXTERNC ENOcrState NBOcr_getState(const STNBOcr* obj);	//
NB_EXTERNC SI32	NBOcr_getProgress(const STNBOcr* obj, STNBSizeI* dstCurSize, STNBAABoxI* dstCurArea);		//0-100
NB_EXTERNC SI32	NBOcr_getFeedCount(const STNBOcr* obj);						//How many time a bitmap has been feed
NB_EXTERNC STNBOcrOSD NBOcr_getOSDFound(const STNBOcr* obj);				//Orientation and Scripting (ex: Latin) Detection
NB_EXTERNC SI32 NBOcr_getTextFound(const STNBOcr* obj, STNBString* dst);	//Accumlated text

//Prepare bitmap data (invert words)
NB_EXTERNC SI32 NBOcr_ocrPrepareBitmap(const STNBBitmap* bmp);
NB_EXTERNC SI32 NBOcr_ocrPrepareBitmapData(const STNBBitmapProps bmpProps, BYTE* bmpData);

//Sync-exec
NB_EXTERNC SI32 NBOcr_feedBitmap(STNBOcr* obj, const STNBBitmap* bmp, const ENOcrSourcePrep bmpPrep, const STNBSizeI ppi, const char* lang3, const char* dataPath);
NB_EXTERNC SI32 NBOcr_feedBitmapData(STNBOcr* obj, const STNBBitmapProps bmpProps, const BYTE* bmpData, const ENOcrSourcePrep bmpPrep, const STNBSizeI ppi, const char* lang3, const char* dataPath);

//Async-exec
NB_EXTERNC void NBOcr_startCanceling(STNBOcr* obj);
NB_EXTERNC SI32 NBOcr_feedBitmapAsync(STNBOcr* obj, const STNBBitmap* bmp, const ENOcrSourcePrep bmpPrep, const ENOcrSourceAction bmpAction, const STNBSizeI ppi, const char* lang3, const char* dataPath);
NB_EXTERNC SI32 NBOcr_feedBitmapDataAsync(STNBOcr* obj, const STNBBitmapProps bmpProps, const BYTE* bmpData, const ENOcrSourcePrep bmpPrep, const ENOcrSourceAction bmpDataAction, const STNBSizeI ppi, const char* lang3, const char* dataPath);

	
#undef NB_EXTERNC

#endif //#ifndef NB_COMPILE_DRIVER_MODE

#endif /* NBOcr_h */
