//
//  NBAADetection.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/11/18.
//

#ifndef NBAADetection_h
#define NBAADetection_h

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/2d/NBBitmap.h"

#ifdef __cplusplus
extern "C" {
#endif

//Line movements (horizontal or vertical)

typedef enum ENNBAADetectAlgthm_ {
	ENNBAADetectAlgthm_AvgGray = 0,	//gray scaled-down pixel is calculated and then compared with neighborgs (fastest, but can miss changes in color components)
	ENNBAADetectAlgthm_AvgComps,	//(RGB) color-components scaled-down pixel are calculated and then compared with neighborgs (fast, but can miss thin lines)
	//
	ENNBAADetectAlgthm_Count
} ENNBAADetectAlgthm;

//Filtered version

typedef enum ENNBAADetectionBmp_ {
	ENNBAADetectionBmp_HLines = 0,	//filtered image for horizontal lines detection
	ENNBAADetectionBmp_VLines,		//filtered image for vertical lines detection
	ENNBAADetectionBmp_Visual,		//filtered gray image
	ENNBAADetectionBmp_Count
} ENNBAADetectionBmp;

//Line detected

typedef struct STNBAADetectionLine_ {
	UI32		iPxStart;
	UI32		iPxEnd;
} STNBAADetectionLine;

typedef struct STNBAADetectionLines_ {
	SI32		width;
	SI32		height;
	struct {
		STNBAADetectionLine* lines;
		UI32	linesSz;
	} h;
	struct {
		STNBAADetectionLine* lines;
		UI32	linesSz;
	} v;
} STNBAADetectionLines;

typedef struct STNBAADetectionArea_ {
	STNBRectI	rect;
	BOOL		anlzH;
	BOOL		anlzV;
} STNBAADetectionArea;

BOOL NBCompare_NBAADetectionAreaByX(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);
BOOL NBCompare_NBAADetectionAreaByY(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);

typedef struct STNBAADetectionAreas_ {
	STNBAADetectionArea* areas;
	UI32				areasSz;
} STNBAADetectionAreas;

//Config

typedef struct STNBAADetectionCfg_ {
	//filter (org to filtered-bmp)
	struct {
		UI16		bmpMaxSz;//in relation to the longest side of the org
		ENNBAADetectAlgthm algorithm;
		//posterize
		struct {
			UI8		divider; //255 should be divisible by 'divider'
		} posterize;
		//saturation
		struct {
			UI8		passes;	//each pass makes stronger pixels to absobr half of softer neighbors
		} saturation;
	} filter;
	//mask (filtered-bmp to mask-of-lines)
	struct {
		UI8			minValue;	//over 255
	} mask;
	//lines (mask-of-lines to lines)
	struct {
		UI16		lenScale;		//multiplier/divider to keep lengths as integer (low to keep inside an UI16)
		UI32		minRelLen;		//(1-100) shorter lines will be discarted (relative to shorter side)
		//deltas
		struct {
			UI32	minRelLen;		//(1-100) shorter paths emergin from other path will be discarted (relative to 
		} deltas;
	} lines;
	//dbg
	struct {
		BOOL		enableFilteredBmp;	//filtered visual representation (not for internal use)
	} dbg;
} STNBAADetectionCfg;

//AxisAligned Detecton

NB_OBJREF_HEADER(NBAADetection)

//cfg
STNBAADetectionCfg NBAADetection_getCfg(STNBAADetectionRef ref);
BOOL NBAADetection_setCfg(STNBAADetectionRef ref, const STNBAADetectionCfg* cfg);

//Threads
BOOL NBAADetection_startThreads(STNBAADetectionRef ref);
void NBAADetection_stopFlag(STNBAADetectionRef ref);

//Detection steps
BOOL NBAADetection_isBussy(STNBAADetectionRef ref);
BOOL NBAADetection_isCurOrgBmpProducedAndConsumed(STNBAADetectionRef ref);
BOOL NBAADetection_doOneAction(STNBAADetectionRef ref);

//Data
STNBBitmap* NBAADetection_getOrgBmpForWritting(STNBAADetectionRef ref);
void NBAADetection_returnOrgBmpFromWrite(STNBAADetectionRef ref, STNBBitmap* bmp, const BOOL wasUpdated, const STNBAADetectionAreas* optAreasToConsume);

const STNBBitmap* NBAADetection_getFilteredBmpAndLock(STNBAADetectionRef ref, const ENNBAADetectionBmp version, UI32* seqFilter);
void NBAADetection_returnFilteredBmpLock(STNBAADetectionRef ref, const STNBBitmap* bmp);

BOOL NBAADetection_getLinesAndLock(STNBAADetectionRef ref, STNBAADetectionLines* dst, UI32* seqFilter);
void NBAADetection_returnLinesLock(STNBAADetectionRef ref, STNBAADetectionLines* lines);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBAADetection_h */
