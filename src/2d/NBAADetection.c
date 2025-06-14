//
//  NBAADetection.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/11/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/2d/NBAADetection.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBThread.h"
#include "nb/core/NBThreadCond.h"
#include "nb/2d/NBBitmap.h"

//#define NB_AADETECT_ASSERTS_ENABLED		//if defined, NBASSERTs in line detection code are activated

#ifdef NB_AADETECT_ASSERTS_ENABLED
#	define NB_AADETECT_ASSERT(EVAL)			NBASSERT(EVAL)
#	define NB_AADETECT_IF_ASSERTING(CODE)	CODE
#else
#	define NB_AADETECT_ASSERT(EVAL)
#	define NB_AADETECT_IF_ASSERTING(OP)
#endif

//Horizontal
#define NB_LINE_H_BIT_LEFT_LEFT		(0x1 << 0)	//inverse of NB_LINE_H_BIT_RIGHT_RIGHT
#define NB_LINE_H_BIT_LEFT_DOWN		(0x1 << 1)	//inverse of NB_LINE_H_BIT_RIGHT_UP
#define NB_LINE_H_BIT_LEFT_UP		(0x1 << 2)	//inverse of NB_LINE_H_BIT_RIGHT_DOWN
#define NB_LINE_H_BIT_LEFT			(0x1 << 3)	//inverse if NB_LINE_H_BIT_RIGHT
#define NB_LINE_H_BIT_LEFT_ANY		(NB_LINE_H_BIT_LEFT_LEFT | NB_LINE_H_BIT_LEFT_DOWN | NB_LINE_H_BIT_LEFT_UP | NB_LINE_H_BIT_LEFT)
//
#define NB_LINE_H_BIT_RIGHT_RIGHT	(0x1 << 4)	//Two pixels to the right
#define NB_LINE_H_BIT_RIGHT_DOWN	(0x1 << 5)	//Two pixels to the right, one down
#define NB_LINE_H_BIT_RIGHT_UP		(0x1 << 6)	//Two pixels to the right, one up
#define NB_LINE_H_BIT_RIGHT			(0x1 << 7)	//One pixels to the right
#define NB_LINE_H_BIT_RIGHT_ANY		(NB_LINE_H_BIT_RIGHT_RIGHT | NB_LINE_H_BIT_RIGHT_DOWN | NB_LINE_H_BIT_RIGHT_UP | NB_LINE_H_BIT_RIGHT)

//Vertical
#define NB_LINE_V_BIT_UP_UP			(0x1 << 0)	//inverse of NB_LINE_V_BIT_DOWN_DOWN
#define NB_LINE_V_BIT_UP_RIGHT		(0x1 << 1)	//inverse of NB_LINE_V_BIT_DOWN_LEFT
#define NB_LINE_V_BIT_UP_LEFT		(0x1 << 2)	//inverse of NB_LINE_V_BIT_DOWN_RIGHT
#define NB_LINE_V_BIT_UP			(0x1 << 3)	//inverse if NB_LINE_V_BIT_DOWN
#define NB_LINE_V_BIT_UP_ANY		(NB_LINE_V_BIT_UP_UP | NB_LINE_V_BIT_UP_RIGHT | NB_LINE_V_BIT_UP_LEFT | NB_LINE_V_BIT_UP)
//
#define NB_LINE_V_BIT_DOWN_DOWN		(0x1 << 4)	//Two pixels to the right
#define NB_LINE_V_BIT_DOWN_RIGHT	(0x1 << 5)	//Two pixels to the right, one down
#define NB_LINE_V_BIT_DOWN_LEFT		(0x1 << 6)	//Two pixels to the right, one up
#define NB_LINE_V_BIT_DOWN			(0x1 << 7)	//One pixels to the right
#define NB_LINE_V_BIT_DOWN_ANY		(NB_LINE_V_BIT_DOWN_DOWN | NB_LINE_V_BIT_DOWN_RIGHT | NB_LINE_V_BIT_DOWN_LEFT | NB_LINE_V_BIT_DOWN)

//Thread state

typedef enum ENNBAADetectThreadState_ {
	ENNBAADetectThreadState_Stopped = 0,	//the thread is not running
	ENNBAADetectThreadState_Iddle,			//the thread is running inactive
	ENNBAADetectThreadState_Filtering,		//the thread is producing the filtered image
	ENNBAADetectThreadState_Masking,		//the thread is producing the mask of the filtered image
	ENNBAADetectThreadState_Detecting,		//the thread is detecting lines in the filtered image
	ENNBAADetectThreadState_Count
} ENNBAADetectThreadState;

//Line movements (horizontal or vertical)

typedef enum ENNBAADetectPxDir_ {
	ENNBAADetectPxDir_Next = 0,
	ENNBAADetectPxDir_NextTwice,
	ENNBAADetectPxDir_NextTwiceFrwd,
	ENNBAADetectPxDir_NextTwiceBckwd,
	//
	ENNBAADetectPxDir_Count
} ENNBAADetectPxDir;

//Line map-pixel

typedef struct STNBAADetectPx_ {
	UI8		mask;		//pixel mask
	UI8		iLngstFwrd;	//ENNBAADetectPxDir
	UI8		iLngstBwrd;	//ENNBAADetectPxDir
	UI16	lenFwrd;	//pixel longest line from start
	UI16	lenBwrd;	//pixel longest line from end
} STNBAADetectPx;

//Lines detection group (horizontal or vertical)

typedef struct STNBAADetectGrp_ {
	//Mask buffer
	struct {
		UI32	locksWrite;	//currently in use
		UI32	locksRead;	//currently in use
		UI32	seq;		//data version
		//
		STNBAADetectPx* buff;
		UI32	buffSz;
		SI32	width;
		SI32	height;
		STNBArraySorted areas;	//STNBAADetectionArea, if empty, then processing full image
	} mask;
	//Lines
	struct {
		UI32	locksWrite;	//currently in use
		UI32	locksRead;	//currently in use
		UI32	seq;		//data version
		//
		BOOL		pendToConsume;
		STNBArray	shorts;		//STNBAADetectionLine, short lines
		STNBArray	merged;		//STNBAADetectionLine, merged lines
	} lines;
} STNBAADetectGrp;

//Pixel to mask tasks

typedef struct STNBAADetectJump_ {
	SI32	img;			//offset in the pixel image
	SI32	map;			//offset in the map array
} STNBAADetectJump;

typedef struct STNBAADetectOffset_ {
	SI32	img;			//offset in the pixel image
	SI32	map;			//offset in the map array
	UI8		bitFwrd;		//bit to activate on forward direction
	UI8		bitFwrdForbid;	//do not eval 'bitFwrd' if 'bitFwrdForbid' is present  
	UI8		bitBwrd;		//bit to activate on backward direction
	UI8		bitBwrdForbid;	//do not eval 'bitBwrd' if 'bitBwrdForbid' is present
	UI16	lenAdd;			//len to add
} STNBAADetectOffset;
	
typedef struct STNBAADetectTask_ {
	//image
	struct {
		UI8		minBright;
	} img;
	//lines
	struct {
		UI16	minLen;
		UI16	minLenDelta;
	} lines;
	//offsets
	STNBAADetectOffset	offsetsFwrd[ENNBAADetectPxDir_Count]; //offsets to next pixel
	//cycles
	struct {
		//outer
		struct {
			BOOL				isHoriz; //moves are in the horizontal sense
			STNBAADetectJump	next;	//move to next pos, must be positive
		} outer;
		//inner
		struct {
			BOOL				isHoriz; //moves are in the horizontal sense
			STNBAADetectJump	next;	//move to next pos, must be positive
		} inner;
	} cycles;
} STNBAADetectTask;

//Mask to lines tasks

typedef struct STNBAADetectState_ {
	SI32					width;
	SI32					height;
	UI16					lineLenMin;		//min line-length of lines squared (discard shorter lines)
	const STNBAADetectPx	*lnFirst;
	const STNBAADetectPx	*lnAfterLast;
	STNBAADetectPx			*pxStart;
} STNBAADetectState;

typedef struct STNBAADetectPath_ {
	UI16		curLen;				//current length
	UI16		curLenUpLeft;		//current length moved upwards (curLenUpLeft <= curLen; curLenHorizOrVertOnly = curLen - curLenUpLeft - curLenDownRight)
	UI16		curLenDownRight;	//current length moved downwards (curLenDownRight <= curLen; curLenHorizOrVertOnly = curLen - curLenUpLeft - curLenDownRight) 
	//
	UI32*		longestAdded;		//shared var, so paths can ignore shorter paths
	UI32*		longestAddedIdx;	//shared var, idx of longest path found
} STNBAADetectPath;

typedef struct STNBAADetectStats_ {
	STNBArray*	dstArr;	//STNBAADetectionLine
	UI32		recursiveMaxDepth;
	UI32		recursiveCount;
	UI32		intersectsCount;
	UI32		ignoredCount;
	UI32		ignoredByLongerCount;
	UI32		addedCount;
} STNBAADetectStats;

//Thread

struct STNBAADetectionOpq_;

typedef struct STNBAADetectionThread_ {
	struct STNBAADetectionOpq_*	opq;
	ENNBAADetectThreadState	state;
} STNBAADetectionThread;

//Lines detection opq

typedef struct STNBAADetectionOpq_ {
	STNBObject		prnt;		//parent
	//config
	struct {
		UI32				iSeq;	//if zero, no validated config was set
		STNBAADetectionCfg	cur;	//current config
	} cfg;
	//org
	struct {
		UI32		locksWrite;	//currently in use
		UI32		locksRead;	//currently in use
		UI32		seq;		//current version
		STNBBitmap	bmp;		//copy of the original image
		STNBArray	areas;		//STNBAADetectionArea, if empty, then processing full image
	} org;
	//filtered
	struct {
		UI32		locksWrite;	//currently in use
		UI32		locksRead;	//currently in use
		UI32		seq;		//current version
		STNBBitmap	bmpH;		//filtered hortizontal lines image
		STNBBitmap	bmpV;		//filtered vertical lines image
		STNBBitmap	bmp;		//filtered for visual representation (only if cfg.dbg.enableFilteredBmp is true)
		STNBArray	areas;		//STNBAADetectionArea, if empty, then processing full image
		//prev-line (buffer)
		struct {
			BYTE*	data;
			UI32	dataSz;
		} prevLine;
	} filtered;
	//Lines
	struct {
		STNBAADetectGrp h;	//horizontal
		STNBAADetectGrp v;	//vertical
	} lines;
	//threads
	struct {
		UI32		amm;	//amount to active
		STNBAADetectionThread d[1];
	} threads;
	//
	BOOL			stopFlag;
	STNBThreadCond	cond;
} STNBAADetectionOpq;

#define NB_AADETECT_ACTION_IN_PROGRESS(OPQ)	( \
											(OPQ)->org.locksWrite > 0 || (OPQ)->org.locksRead > 0 \
											|| (OPQ)->filtered.locksWrite > 0 || (OPQ)->filtered.locksRead > 0 \
											|| (OPQ)->lines.h.mask.locksWrite > 0 || (OPQ)->lines.h.mask.locksRead > 0 || (OPQ)->lines.h.lines.locksWrite > 0 || (OPQ)->lines.h.lines.locksRead > 0 \
											|| (OPQ)->lines.v.mask.locksWrite > 0 || (OPQ)->lines.v.mask.locksRead > 0 || (OPQ)->lines.v.lines.locksWrite > 0 || (OPQ)->lines.v.lines.locksRead > 0 \
											) 
typedef struct STNBAADetectAlgorithmDef_ {
	BOOL dummy;
} STNBAADetectAlgorithmDef;


//Tasks
STNBAADetectTask NBAADetection_getScanTaskRowsPerCol_(const STNBAADetectionCfg* cfg, STNBBitmap* filtered);
STNBAADetectTask NBAADetection_getScanTaskColsPerRow_(const STNBAADetectionCfg* cfg, STNBBitmap* filtered);

//Actions
BOOL NBAADetection_doOneActionLocked_(STNBAADetectionOpq* opq, const SI32 iThread, ENNBAADetectThreadState* dstState);
BOOL NBAADetection_buildFilteredBmp_(const STNBAADetectionCfg* cfg, STNBBitmap* org, STNBBitmap* dstH, STNBBitmap* dstV, STNBBitmap* dstVisual, const STNBAADetectionArea* areas, const UI32 areasSz, STNBArray* dstAreasScld, BYTE** prevLineData, UI32* prevLineSz);
BOOL NBAADetection_buildMaskH_(const STNBAADetectionCfg* cfg, STNBBitmap* filtered, const STNBAADetectionArea* areas, const UI32 areasSz, STNBAADetectGrp* dst);
BOOL NBAADetection_buildMaskV_(const STNBAADetectionCfg* cfg, STNBBitmap* filtered, const STNBAADetectionArea* areas, const UI32 areasSz, STNBAADetectGrp* dst);
BOOL NBAADetection_buildMask_(STNBBitmap* filtered, const ENNBAADetectionBmp version, STNBAADetectGrp* dst, const STNBAADetectionArea* areas, const UI32 areasSz, const STNBAADetectTask* tsk);
BOOL NBAADetection_buildLinesH_(const STNBAADetectionCfg* cfg, STNBBitmap* filtered, STNBAADetectGrp* dst); 
BOOL NBAADetection_buildLinesV_(const STNBAADetectionCfg* cfg, STNBBitmap* filtered, STNBAADetectGrp* dst);
SI64 NBAADetection_threadMethod_(STNBThread* t, void* param);

NB_OBJREF_BODY(NBAADetection, STNBAADetectionOpq, NBObject)

void NBAADetection_initZeroed(STNBObject* obj){
	STNBAADetectionOpq* opq = (STNBAADetectionOpq*)obj;
	//org
	{
		NBBitmap_init(&opq->org.bmp);
		NBArray_init(&opq->org.areas, sizeof(STNBAADetectionArea), NULL);
	}
	//filtered
	{
		NBBitmap_init(&opq->filtered.bmpH);
		NBBitmap_init(&opq->filtered.bmpV);
		NBBitmap_init(&opq->filtered.bmp);		//filtered for visual representation (only if cfg.dbg.enableFilteredBmp is true)
		NBArray_init(&opq->filtered.areas, sizeof(STNBAADetectionArea), NULL);
	}
	//threads
	{
		//
	}
	//lines
	{
		//horizontal
		{
			NBArraySorted_init(&opq->lines.h.mask.areas, sizeof(STNBAADetectionArea), NBCompare_NBAADetectionAreaByX);
			NBArray_init(&opq->lines.h.lines.shorts, sizeof(STNBAADetectionLine), NULL);
			NBArray_init(&opq->lines.h.lines.merged, sizeof(STNBAADetectionLine), NULL);
		}
		//vertical
		{
			NBArraySorted_init(&opq->lines.v.mask.areas, sizeof(STNBAADetectionArea), NBCompare_NBAADetectionAreaByY);
			NBArray_init(&opq->lines.v.lines.shorts, sizeof(STNBAADetectionLine), NULL);
			NBArray_init(&opq->lines.v.lines.merged, sizeof(STNBAADetectionLine), NULL);
		}
	}
	NBThreadCond_init(&opq->cond);
}

void NBAADetection_uninitLocked(STNBObject* obj){
	STNBAADetectionOpq* opq = (STNBAADetectionOpq*)obj;
	{
		//wait for threads
		BOOL anyActive = TRUE;
		while(anyActive){
			anyActive = FALSE;
			{
				UI32 i; const UI32 count = (sizeof(opq->threads.d) / sizeof(opq->threads.d[0])); 
				for(i = 0; i < count; i++){
					if(opq->threads.d[i].state != ENNBAADetectThreadState_Stopped){
						anyActive = TRUE;
						opq->stopFlag = TRUE;
						NBThreadCond_broadcast(&opq->cond);
						NBThreadCond_waitObj(&opq->cond, obj);
						break;
					}
				}
			}
		}
		//org
		{
			NBBitmap_release(&opq->org.bmp);
			NBArray_release(&opq->org.areas);
		}
		//filtered
		{
			if(opq->filtered.prevLine.data != NULL){
				NBMemory_free(opq->filtered.prevLine.data);
				opq->filtered.prevLine.data = NULL;
				opq->filtered.prevLine.dataSz = 0;
			}
			NBBitmap_release(&opq->filtered.bmpH);
			NBBitmap_release(&opq->filtered.bmpV);
			NBBitmap_release(&opq->filtered.bmp);
			NBArray_release(&opq->filtered.areas);
		}
		//lines
		{
			//horizontal
			{
				NBArraySorted_release(&opq->lines.h.mask.areas);
				NBArray_release(&opq->lines.h.lines.shorts);
				NBArray_release(&opq->lines.h.lines.merged);
				if(opq->lines.h.mask.buff != NULL){
					NBMemory_free(opq->lines.h.mask.buff);
					opq->lines.h.mask.buff	= NULL;
					opq->lines.h.mask.buffSz	= 0;
				}
			}
			//vertical
			{
				NBArraySorted_release(&opq->lines.v.mask.areas);
				NBArray_release(&opq->lines.v.lines.shorts);
				NBArray_release(&opq->lines.v.lines.merged);
				if(opq->lines.v.mask.buff != NULL){
					NBMemory_free(opq->lines.v.mask.buff);
					opq->lines.v.mask.buff	= NULL;
					opq->lines.v.mask.buffSz	= 0;
				}
			}
		}
		//threads
		{
			//
		}
		NBThreadCond_release(&opq->cond);
	}
}

//cfg

STNBAADetectionCfg NBAADetection_getCfg(STNBAADetectionRef ref){
	STNBAADetectionOpq* opq = (STNBAADetectionOpq*)ref.opaque;
	return opq->cfg.cur;
}

BOOL NBAADetection_setCfg(STNBAADetectionRef ref, const STNBAADetectionCfg* cfg){
	BOOL r = FALSE;
	STNBAADetectionOpq* opq = (STNBAADetectionOpq*)ref.opaque;
	NBObject_lock(opq);
	if(cfg != NULL && !NB_AADETECT_ACTION_IN_PROGRESS(opq)){
		if(cfg->filter.bmpMaxSz <= 0 || cfg->filter.posterize.divider <= 0){
			//
		} else if(cfg->lines.minRelLen <= 0 || cfg->lines.lenScale <= 0 || cfg->lines.deltas.minRelLen <= 0){
			//
		} else {
			opq->cfg.cur = *cfg;
			opq->cfg.iSeq++;
		}
	}
	NBObject_unlock(opq);
	return r;
	
}

//Lines detection

BOOL NBAADetection_startThreads(STNBAADetectionRef ref){
	BOOL r = FALSE;
	STNBAADetectionOpq* opq = (STNBAADetectionOpq*)ref.opaque;
	NBObject_lock(opq);
	if(opq->cfg.iSeq > 0){ //set config before starting threads
		UI32 i; const UI32 count = (sizeof(opq->threads.d) / sizeof(opq->threads.d[0]));
		BOOL allStopped = TRUE;
		{
			for(i = 0; i < count; i++){
				if(opq->threads.d[i].state != ENNBAADetectThreadState_Stopped){
					allStopped = FALSE;
				}
			}
		}
		if(allStopped){
			r = TRUE;
			opq->stopFlag = FALSE;
			for(i = 0; i < count; i++){
				if(opq->threads.d[i].state == ENNBAADetectThreadState_Stopped){
					//Start thread
					STNBThread* t = NBMemory_allocType(STNBThread);
					NBThread_init(t);
					NBThread_setIsJoinable(t, FALSE);
					//Start thread
					opq->threads.d[i].opq	= opq;
					opq->threads.d[i].state	= ENNBAADetectThreadState_Iddle;
					if(!NBThread_start(t, NBAADetection_threadMethod_, &opq->threads.d[i], NULL)){
						opq->threads.d[i].state = ENNBAADetectThreadState_Stopped;
					} else {
						t = NULL; 
					}
					//Release (if not consumed)
					if(t != NULL){
						NBThread_release(t);
						NBMemory_free(t);
						t = NULL;
					}
				}
			}
		}
	}
	NBObject_unlock(opq);
	return r;
}

void NBAADetection_stopFlag(STNBAADetectionRef ref){
	STNBAADetectionOpq* opq = (STNBAADetectionOpq*)ref.opaque;
	NBObject_lock(opq);
	{
		opq->stopFlag = TRUE;
		NBThreadCond_broadcast(&opq->cond);
	}
	NBObject_unlock(opq);
}

SI64 NBAADetection_threadMethod_(STNBThread* t, void* param){
	SI64 r = 0;
	STNBAADetectionThread* wt	= (STNBAADetectionThread*)param;
	STNBAADetectionOpq* opq		= wt->opq;
	IF_NBASSERT(const SI32 wtIdx= (SI32)(wt - opq->threads.d);) NBASSERT(wtIdx >= 0 && wtIdx <= 2)
	{
		NBObject_lock(opq);
		{
			//Set as iddle
			{
				NBASSERT(wt->state == ENNBAADetectThreadState_Iddle)
				wt->state = ENNBAADetectThreadState_Iddle;
				NBThreadCond_broadcast(&opq->cond);
			}
			//Work cycle
			while(!opq->stopFlag){
				NBASSERT(wt->state == ENNBAADetectThreadState_Iddle)
				//Wait
				if(!NBAADetection_doOneActionLocked_(opq, -1, &wt->state)){
					NBThreadCond_waitObj(&opq->cond, opq);
				}
			}
			//Set as stopped
			{
				NBASSERT(wt->state == ENNBAADetectThreadState_Iddle)
				wt->state = ENNBAADetectThreadState_Stopped;
				NBThreadCond_broadcast(&opq->cond);
			}
		}
		NBObject_unlock(opq);
	}
	//Release thread
	{
		NBThread_release(t);
		NBMemory_free(t);
		t = NULL;
	}
	return r;
}

BOOL NBAADetection_doOneActionLocked_(STNBAADetectionOpq* opq, const SI32 iThread, ENNBAADetectThreadState* dstState){
	BOOL r = FALSE;
	if(opq->cfg.iSeq > 0){ //set config before actions
		//Analyze filtered production (only first thread)
		if(
		   (iThread < 0 || iThread == 0)
		   && opq->org.locksWrite == 0 //src not locked
		   && opq->filtered.seq != opq->org.seq && opq->filtered.locksWrite == 0 && opq->filtered.locksRead == 0 //work required
		   && opq->lines.h.mask.seq == opq->filtered.seq && opq->lines.v.mask.seq == opq->filtered.seq //next step completed
		   )
		{
			if(dstState != NULL) *dstState = ENNBAADetectThreadState_Filtering;
			opq->org.locksRead++;
			opq->filtered.locksWrite++;
			NBThreadCond_broadcast(&opq->cond);
			NBObject_unlock(opq);
			{
				STNBBitmap* visualBmp = (opq->cfg.cur.dbg.enableFilteredBmp ? &opq->filtered.bmp : NULL);
				NBArray_empty(&opq->filtered.areas);
				if(NBAADetection_buildFilteredBmp_(&opq->cfg.cur, &opq->org.bmp, &opq->filtered.bmpH, &opq->filtered.bmpV, visualBmp, NBArray_dataPtr(&opq->org.areas, STNBAADetectionArea), opq->org.areas.use, &opq->filtered.areas, &opq->filtered.prevLine.data, &opq->filtered.prevLine.dataSz)){
					opq->filtered.seq = opq->org.seq;
					r = TRUE;
				}
			}
			NBObject_lock(opq);
			NBASSERT(opq->org.locksRead > 0)
			NBASSERT(opq->filtered.locksWrite > 0)
			opq->org.locksRead--;
			opq->filtered.locksWrite--;
			if(dstState != NULL) *dstState = ENNBAADetectThreadState_Iddle;
			NBThreadCond_broadcast(&opq->cond);
		}
		//Lines-H detection
		{
			const UI32 threadIdx = 1;
			STNBAADetectGrp* lines = &opq->lines.h;
			//Build detection mask
			if(
			   (iThread < 0 || iThread == threadIdx) //allowed thread
			   && opq->filtered.locksWrite == 0 //src not locked
			   && lines->mask.seq != opq->filtered.seq && lines->mask.locksWrite == 0 && lines->mask.locksRead == 0 //work required
			   && lines->lines.seq == lines->mask.seq //next step completed
			   )
			{
				if(dstState != NULL) *dstState = ENNBAADetectThreadState_Masking;
				opq->filtered.locksRead++;
				lines->mask.locksWrite++;
				NBThreadCond_broadcast(&opq->cond);
				NBObject_unlock(opq);
				{
					if(NBAADetection_buildMaskH_(&opq->cfg.cur, &opq->filtered.bmpH, NBArray_dataPtr(&opq->filtered.areas, STNBAADetectionArea), opq->filtered.areas.use, lines)){
						//PRINTF_INFO("NBAADetection, %d short, %d long H-lines detected.\n", lines->lines.shorts.use, lines->lines.merged.use);
					}
					lines->mask.seq = opq->filtered.seq;
					r = TRUE;
				}
				NBObject_lock(opq);
				NBASSERT(opq->filtered.locksRead > 0)
				NBASSERT(lines->mask.locksWrite > 0)
				opq->filtered.locksRead--;
				lines->mask.locksWrite--;
				if(dstState != NULL) *dstState = ENNBAADetectThreadState_Iddle;
				NBThreadCond_broadcast(&opq->cond);
			}
			//Build lines array
			if(
			   (iThread < 0 || iThread == threadIdx) //allowed thread 
			   && lines->mask.locksWrite == 0 //src not locked
			   && lines->lines.seq != lines->mask.seq && lines->lines.locksWrite == 0 && lines->lines.locksRead == 0 //work required
			   && !lines->lines.pendToConsume //next step completed
			   )
			{
				if(dstState != NULL) *dstState = ENNBAADetectThreadState_Detecting;
				lines->mask.locksRead++;
				lines->lines.locksWrite++;
				NBThreadCond_broadcast(&opq->cond);
				NBObject_unlock(opq);
				{
					if(NBAADetection_buildLinesH_(&opq->cfg.cur, &opq->filtered.bmpH, lines)){
						//PRINTF_INFO("NBAADetection, %d short, %d long H-lines detected.\n", lines->lines.shorts.use, lines->lines.merged.use);
					}
					lines->lines.seq = lines->mask.seq;
					lines->lines.pendToConsume = TRUE;
					r = TRUE;
				}
				NBObject_lock(opq);
				NBASSERT(lines->mask.locksRead > 0)
				NBASSERT(lines->lines.locksWrite > 0)
				lines->mask.locksRead--;
				lines->lines.locksWrite--;
				if(dstState != NULL) *dstState = ENNBAADetectThreadState_Iddle;
				NBThreadCond_broadcast(&opq->cond);
			}
		}
		//Lines-V detection
		{
			const UI32 threadIdx = 2;
			STNBAADetectGrp* lines = &opq->lines.v;
			//Build detection mask
			if(
			   (iThread < 0 || iThread == threadIdx) //allowed thread
			   && opq->filtered.locksWrite == 0 //src not locked
			   && lines->mask.seq != opq->filtered.seq && lines->mask.locksWrite == 0 && lines->mask.locksRead == 0 //work required
			   && lines->lines.seq == lines->mask.seq //next step completed
			   )
			{
				if(dstState != NULL) *dstState = ENNBAADetectThreadState_Masking;
				opq->filtered.locksRead++;
				lines->mask.locksWrite++;
				NBThreadCond_broadcast(&opq->cond);
				NBObject_unlock(opq);
				{
					if(NBAADetection_buildMaskV_(&opq->cfg.cur, &opq->filtered.bmpV, NBArray_dataPtr(&opq->filtered.areas, STNBAADetectionArea), opq->filtered.areas.use, lines)){
						//PRINTF_INFO("NBAADetection, %d short, %d long H-lines detected.\n", lines->lines.shorts.use, lines->lines.merged.use);
					}
					lines->mask.seq = opq->filtered.seq;
					r = TRUE;
				}
				NBObject_lock(opq);
				NBASSERT(opq->filtered.locksRead > 0)
				NBASSERT(lines->mask.locksWrite > 0)
				opq->filtered.locksRead--;
				lines->mask.locksWrite--;
				if(dstState != NULL) *dstState = ENNBAADetectThreadState_Iddle;
				NBThreadCond_broadcast(&opq->cond);
			}
			//Build lines array
			if(
			   (iThread < 0 || iThread == threadIdx) //allowed thread 
			   && lines->mask.locksWrite == 0 //src not locked
			   && lines->lines.seq != lines->mask.seq && lines->lines.locksWrite == 0 && lines->lines.locksRead == 0 //work required
			   && !lines->lines.pendToConsume //next step completed
			   )
			{
				if(dstState != NULL) *dstState = ENNBAADetectThreadState_Detecting;
				lines->mask.locksRead++;
				lines->lines.locksWrite++;
				NBThreadCond_broadcast(&opq->cond);
				NBObject_unlock(opq);
				{
					if(NBAADetection_buildLinesV_(&opq->cfg.cur, &opq->filtered.bmpV, lines)){
						//PRINTF_INFO("NBAADetection, %d short, %d long H-lines detected.\n", lines->lines.shorts.use, lines->lines.merged.use);
					}
					lines->lines.seq = lines->mask.seq;
					lines->lines.pendToConsume = TRUE;
					r = TRUE;
				}
				NBObject_lock(opq);
				NBASSERT(lines->mask.locksRead > 0)
				NBASSERT(lines->lines.locksWrite > 0)
				lines->mask.locksRead--;
				lines->lines.locksWrite--;
				if(dstState != NULL) *dstState = ENNBAADetectThreadState_Iddle;
				NBThreadCond_broadcast(&opq->cond);
			}
		}
	}
	return r;
}

//Actions

BOOL NBAADetection_isBussy(STNBAADetectionRef ref){
	STNBAADetectionOpq* opq = (STNBAADetectionOpq*)ref.opaque;
	return NB_AADETECT_ACTION_IN_PROGRESS(opq);
}

BOOL NBAADetection_isCurOrgBmpProducedAndConsumed(STNBAADetectionRef ref){
	STNBAADetectionOpq* opq = (STNBAADetectionOpq*)ref.opaque;
	return (
			opq->org.seq == 0 //Nothing fed
			|| (
				opq->lines.h.lines.seq == opq->org.seq && opq->lines.v.lines.seq == opq->org.seq //current lines match org version
				&& !opq->lines.h.lines.pendToConsume && !opq->lines.v.lines.pendToConsume) //current lines were consumed
			);
}

BOOL NBAADetection_doOneAction(STNBAADetectionRef ref){
	BOOL r = FALSE;
	STNBAADetectionOpq* opq = (STNBAADetectionOpq*)ref.opaque;
	NBObject_lock(opq);
	{
		r = NBAADetection_doOneActionLocked_(opq, -1, NULL);
	}
	NBObject_unlock(opq);
	return r;
}

//Data

STNBBitmap* NBAADetection_getOrgBmpForWritting(STNBAADetectionRef ref){
	STNBBitmap* r = NULL;
	STNBAADetectionOpq* opq = (STNBAADetectionOpq*)ref.opaque;
	NBObject_lock(opq);
	if(opq->org.locksWrite == 0 && opq->org.locksRead == 0){
		opq->org.locksWrite++;
		r = &opq->org.bmp;
	}
	NBObject_unlock(opq);
	return r;
}

void NBAADetection_returnOrgBmpFromWrite(STNBAADetectionRef ref, STNBBitmap* bmp, const BOOL wasUpdated, const STNBAADetectionAreas* optAreasToConsume){
	STNBAADetectionOpq* opq = (STNBAADetectionOpq*)ref.opaque;
	NBObject_lock(opq);
	{
		NBASSERT(bmp == &opq->org.bmp)
		NBASSERT(opq->org.locksWrite > 0)
		NBASSERT(opq->org.locksRead == 0)
		if(bmp == &opq->org.bmp && opq->org.locksWrite > 0 && opq->org.locksRead == 0){
			//Areas
			{
				NBArray_empty(&opq->org.areas);
				if(optAreasToConsume != NULL && optAreasToConsume->areas != NULL && optAreasToConsume->areasSz > 0){
					NBArray_addItems(&opq->org.areas, optAreasToConsume->areas, sizeof(optAreasToConsume->areas[0]), optAreasToConsume->areasSz);
				}
			}
			if(wasUpdated){
				opq->org.seq++;
				NBThreadCond_broadcast(&opq->cond); //trigger action
			}
			opq->org.locksWrite--;
		}
	}
	NBObject_unlock(opq);
}

const STNBBitmap* NBAADetection_getFilteredBmpAndLock(STNBAADetectionRef ref, const ENNBAADetectionBmp version, UI32* seqFilter){
	STNBBitmap* r = NULL;
	STNBAADetectionOpq* opq = (STNBAADetectionOpq*)ref.opaque;
	NBObject_lock(opq);
	if(opq->filtered.locksWrite == 0 && (seqFilter == NULL || *seqFilter != opq->filtered.seq)){
		opq->filtered.locksRead++;
		if(seqFilter != NULL) *seqFilter = opq->filtered.seq;
		switch(version) {
		case ENNBAADetectionBmp_HLines:
			r = &opq->filtered.bmpH;
			break;
		case ENNBAADetectionBmp_VLines:
			r = &opq->filtered.bmpV;
			break;
		case ENNBAADetectionBmp_Visual:
			r = &opq->filtered.bmp;
			break;
		default:
			break;
		}
	}
	NBObject_unlock(opq);
	return r;
}

void NBAADetection_returnFilteredBmpLock(STNBAADetectionRef ref, const STNBBitmap* bmp){
	STNBAADetectionOpq* opq = (STNBAADetectionOpq*)ref.opaque;
	NBObject_lock(opq);
	{
		NBASSERT(bmp == &opq->filtered.bmp || bmp == &opq->filtered.bmpH || bmp == &opq->filtered.bmpV)
		NBASSERT(opq->filtered.locksWrite == 0)
		NBASSERT(opq->filtered.locksRead > 0)
		if((bmp == &opq->filtered.bmp || bmp == &opq->filtered.bmpH || bmp == &opq->filtered.bmpV) && opq->filtered.locksWrite == 0 && opq->filtered.locksRead > 0){
			opq->filtered.locksRead--;
		}
	}
	NBObject_unlock(opq);
}

BOOL NBAADetection_getLinesAndLock(STNBAADetectionRef ref, STNBAADetectionLines* dst, UI32* seqFilter){
	BOOL r = FALSE;
	STNBAADetectionOpq* opq = (STNBAADetectionOpq*)ref.opaque;
	NBObject_lock(opq);
	{
		STNBAADetectGrp* grpH = &opq->lines.h;
		STNBAADetectGrp* grpV = &opq->lines.v;
		if(
		   grpH->lines.locksWrite == 0 && (seqFilter == NULL || *seqFilter != grpH->lines.seq)
		   && grpV->lines.locksWrite == 0 && (seqFilter == NULL || *seqFilter != grpV->lines.seq)
		   && grpH->lines.seq == grpV->lines.seq
		   )
		{
			grpH->lines.locksRead++;
			grpV->lines.locksRead++;
			if(seqFilter != NULL) *seqFilter = grpH->lines.seq;
			if(dst != NULL){
				NBMemory_setZeroSt(*dst, STNBAADetectionLines);
				dst->width		= grpH->mask.width;
				dst->height		= grpH->mask.height;
				dst->h.lines	= NBArray_dataPtr(&grpH->lines.shorts, STNBAADetectionLine);
				dst->h.linesSz	= grpH->lines.shorts.use;
				dst->v.lines	= NBArray_dataPtr(&grpV->lines.shorts, STNBAADetectionLine);
				dst->v.linesSz	= grpV->lines.shorts.use;
			}
			r = TRUE;
		}
	}
	NBObject_unlock(opq);
	return r;
}

void NBAADetection_returnLinesLock(STNBAADetectionRef ref, STNBAADetectionLines* lines){
	STNBAADetectionOpq* opq = (STNBAADetectionOpq*)ref.opaque;
	NBObject_lock(opq);
	{
		STNBAADetectGrp* grpH = &opq->lines.h;
		STNBAADetectGrp* grpV = &opq->lines.v;
		NBASSERT(lines->h.lines == NBArray_dataPtr(&grpH->lines.shorts, STNBAADetectionLine))
		NBASSERT(lines->v.lines == NBArray_dataPtr(&grpV->lines.shorts, STNBAADetectionLine))
		NBASSERT(grpH->lines.locksWrite == 0)
		NBASSERT(grpV->lines.locksWrite == 0)
		NBASSERT(grpH->lines.locksRead > 0)
		NBASSERT(grpV->lines.locksRead > 0)
		if(lines->h.lines == NBArray_dataPtr(&grpH->lines.shorts, STNBAADetectionLine)
		   && lines->v.lines == NBArray_dataPtr(&grpV->lines.shorts, STNBAADetectionLine)
		   && grpH->lines.locksWrite == 0 && grpV->lines.locksWrite == 0
		   && grpH->lines.locksRead > 0 && grpV->lines.locksRead > 0
		   )
		{
			grpH->lines.locksRead--;
			grpV->lines.locksRead--;
			grpH->lines.pendToConsume = FALSE;
			grpV->lines.pendToConsume = FALSE;
			NBThreadCond_broadcast(&opq->cond); //trigger action
		}
	}
	NBObject_unlock(opq);
}

//

typedef struct STNBAADetectionFilteresTsk_ {
	//org
	struct {
		STNBBitmapProps props;
		BYTE*			data;
		SI32			bytesPerPx;
	} org;
	//scaled
	struct {
		SI32			scale;	//scale down
		ENNBAADetectAlgthm algorithm;
		STNBSizeI		size;
		//prev line buffer
		struct {
			BYTE*		data;
			UI32		sz;
		} prevLine;
		//horizontal
		struct {
			STNBBitmapProps	props;
			BYTE*			data;
			SI32			bytesPerPx;
		} h;
		//vertical
		struct {
			STNBBitmapProps	props;
			BYTE*			data;
			SI32			bytesPerPx;
		} v;
		//visual (optional)
		struct {
			STNBBitmapProps	props;
			BYTE*			data;
			SI32			bytesPerPx;
		} visual;
	} scaled;
} STNBAADetectionFilteresTsk;

void NBAADetection_buildFilteredPosterizedBmpArea_(const STNBAADetectionCfg* cfg, const STNBAADetectionFilteresTsk* tsk, const STNBAADetectionArea sclArea){
	NB_AADETECT_ASSERT(cfg != NULL)
	NB_AADETECT_ASSERT(tsk != NULL)
	//
	const SI32 yStart		= sclArea.rect.y;
	const SI32 xStart		= sclArea.rect.x;
	const SI32 yAfterEnd	= sclArea.rect.y + sclArea.rect.height; 
	const SI32 xAfterEnd	= sclArea.rect.x + sclArea.rect.width;
	const SI32 xStart2		= xStart - (xStart > 0 ? 1 : 0);
	const SI32 yStart2		= yStart - (yStart > 0 ? 1 : 0);
	NBASSERT(xStart >= 0 && xStart <= tsk->scaled.size.width)
	NBASSERT(yStart >= 0 && yStart <= tsk->scaled.size.height)
	NBASSERT(xAfterEnd >= 0 && xAfterEnd <= tsk->scaled.size.width)
	NBASSERT(yAfterEnd >= 0 && yAfterEnd <= tsk->scaled.size.height)
	//PRINTF_INFO("NBAADetection, building filtered area rec(%d, %d)-(%d, %d).\n", xStart, yStart, xAfterEnd - 1, yAfterEnd - 1);
	//Calculate deltas
	{
		const SI32 scale2 = tsk->scaled.scale * tsk->scaled.scale; 
		BYTE* prevLine = tsk->scaled.prevLine.data;
		SI32 x, y, gry, accum[3], comps[3], deltaH, deltaV, deltasH[3], deltasV[3], deltaInv; BYTE *dstPxH, *dstPxV;
		const BYTE* srcLn; const BYTE* srcLnAfterEnd; const BYTE* srcPx; const BYTE* srcPxAfterEnd;
		if(tsk->scaled.algorithm == ENNBAADetectAlgthm_AvgComps){
			for(y = yStart2; y < yAfterEnd; y++){
				for(x = xStart2; x < xAfterEnd; x++){
					accum[0] = accum[1] = accum[2] = 0;
					//Calculate color
					{
						srcLn = &tsk->org.data[(tsk->org.props.bytesPerLine * y * tsk->scaled.scale) + (tsk->org.bytesPerPx * x * tsk->scaled.scale) + (tsk->org.props.color == ENNBBitmapColor_ARGB8 ? 1 : 0)];
						srcLnAfterEnd = srcLn + ((UI64)tsk->org.props.bytesPerLine * (UI64)tsk->scaled.scale);
						while(srcLn < srcLnAfterEnd){
							srcPx = srcLn;
							srcPxAfterEnd = srcPx + ((UI64)tsk->org.bytesPerPx * (UI64)tsk->scaled.scale);
							while(srcPx < srcPxAfterEnd){
								accum[0] += srcPx[0]; //RGB
								accum[1] += srcPx[1]; //RGB
								accum[2] += srcPx[2]; //RGB
								//Next px
								srcPx += tsk->org.bytesPerPx;
							}
							//Next line
							srcLn += tsk->org.props.bytesPerLine;
						}
					}
					//Calculate by component (not gray-only)
					{
						comps[0] = ((accum[0] / scale2) / cfg->filter.posterize.divider) * cfg->filter.posterize.divider;
						comps[1] = ((accum[1] / scale2) / cfg->filter.posterize.divider) * cfg->filter.posterize.divider;
						comps[2] = ((accum[2] / scale2) / cfg->filter.posterize.divider) * cfg->filter.posterize.divider;
						if(x >= xStart && y >= yStart){
							dstPxH = &tsk->scaled.h.data[(tsk->scaled.h.props.bytesPerLine * y) + (tsk->scaled.h.bytesPerPx * x)];
							dstPxV = &tsk->scaled.v.data[(tsk->scaled.v.props.bytesPerLine * y) + (tsk->scaled.v.bytesPerPx * x)];
							//NB_AADETECT_ASSERT(*dstPxH == 0) //detecting twice passes (bad filter-rect caculation)
							//NB_AADETECT_ASSERT(*dstPxV == 0) //detecting twice passes (bad filter-rect caculation)
							if(x == 0 || y == 0){
								*dstPxH = *dstPxV = 0;
							} else {
								//H
								if(sclArea.anlzV){
									deltasV[0]	= (SI32)comps[0] - (SI32)prevLine[(x * 3) + 0];
									deltasV[1]	= (SI32)comps[1] - (SI32)prevLine[(x * 3) + 1];
									deltasV[2]	= (SI32)comps[2] - (SI32)prevLine[(x * 3) + 2];
									if(deltasV[0] < 0) deltasV[0] = -deltasV[0];
									if(deltasV[1] < 0) deltasV[1] = -deltasV[1];
									if(deltasV[2] < 0) deltasV[2] = -deltasV[2];
									deltaV = (deltasV[0] > deltasV[1] && deltasV[0] > deltasV[2] ? deltasV[0] : deltasV[1] > deltasV[2] ? deltasV[1] : deltasV[2]);
									NB_AADETECT_ASSERT(deltaV >= -255 && deltaV <= 255)
									deltaInv	= 255 - deltaV;
									*dstPxH		= ((255 * 255 * 255) - (deltaInv * deltaInv * deltaInv)) / (255 * 255);
								}
								//V
								if(sclArea.anlzH){
									deltasH[0]	= (SI32)comps[0] - (SI32)prevLine[(x * 3) - 3]; //prev-pix was already fed to prevLine
									deltasH[1]	= (SI32)comps[1] - (SI32)prevLine[(x * 3) - 2]; //prev-pix was already fed to prevLine
									deltasH[2]	= (SI32)comps[2] - (SI32)prevLine[(x * 3) - 1]; //prev-pix was already fed to prevLine
									if(deltasH[0] < 0) deltasH[0] = -deltasH[0];
									if(deltasH[1] < 0) deltasH[1] = -deltasH[1];
									if(deltasH[2] < 0) deltasH[2] = -deltasH[2];
									deltaH = (deltasH[0] > deltasH[1] && deltasH[0] > deltasH[2] ? deltasH[0] : deltasH[1] > deltasH[2] ? deltasH[1] : deltasH[2]);
									NB_AADETECT_ASSERT(deltaH >= -255 && deltaH <= 255)
									deltaInv	= 255 - deltaH;
									*dstPxV		= ((255 * 255 * 255) - (deltaInv * deltaInv * deltaInv)) / (255 * 255);
								}
							}
							//Visual
							if(tsk->scaled.visual.data != NULL){
								tsk->scaled.visual.data[(tsk->scaled.visual.props.bytesPerLine * y) + (tsk->scaled.visual.bytesPerPx * x)] = (comps[0] + comps[1] + comps[2]) / (255 + 255 + 255);
							}
						}
						prevLine[(x * 3) + 0] = (BYTE)comps[0];
						prevLine[(x * 3) + 1] = (BYTE)comps[1];
						prevLine[(x * 3) + 2] = (BYTE)comps[2];
					}
				}
			}
		} else { //ENNBAADetectAlgthm_AvgGray
			for(y = yStart2; y < yAfterEnd; y++){
				for(x = xStart2; x < xAfterEnd; x++){
					accum[0] = accum[1] = accum[2] = 0;
					//Calculate color
					{
						srcLn = &tsk->org.data[(tsk->org.props.bytesPerLine * y * tsk->scaled.scale) + (tsk->org.bytesPerPx * x * tsk->scaled.scale) + (tsk->org.props.color == ENNBBitmapColor_ARGB8 ? 1 : 0)];
						srcLnAfterEnd = srcLn + ((UI64)tsk->org.props.bytesPerLine * (UI64)tsk->scaled.scale);
						while(srcLn < srcLnAfterEnd){
							srcPx = srcLn;
							srcPxAfterEnd = srcPx + ((UI64)tsk->org.bytesPerPx * (UI64)tsk->scaled.scale);
							while(srcPx < srcPxAfterEnd){
								accum[0] += srcPx[0]; //RGB
								accum[1] += srcPx[1]; //RGB
								accum[2] += srcPx[2]; //RGB
								//Next px
								srcPx += tsk->org.bytesPerPx;
							}
							//Next line
							srcLn += tsk->org.props.bytesPerLine;
						}
					}
					//Apply scaled-gray-postetrized color
					{
						//NBASSERT((255 % cfg->filter.posterize.divider) == 0)
						gry = ((SI32)((accum[0] + accum[1] + accum[2]) / (scale2 + scale2 + scale2)) / cfg->filter.posterize.divider) * cfg->filter.posterize.divider;
						if(x >= xStart && y >= yStart){
							dstPxH = &tsk->scaled.h.data[(tsk->scaled.h.props.bytesPerLine * y) + (tsk->scaled.h.bytesPerPx * x)];
							dstPxV = &tsk->scaled.v.data[(tsk->scaled.v.props.bytesPerLine * y) + (tsk->scaled.v.bytesPerPx * x)];
							//NB_AADETECT_ASSERT(*dstPxH == 0) //detecting twice passes (bad filter-rect caculation)
							//NB_AADETECT_ASSERT(*dstPxV == 0) //detecting twice passes (bad filter-rect caculation)
							if(x == 0 || y == 0){
								*dstPxH = *dstPxV = 0;
							} else {
								//H
								if(sclArea.anlzV){
									deltaV		= (SI32)gry - (SI32)prevLine[x];
									NB_AADETECT_ASSERT(deltaV >= -255 && deltaV <= 255)
									if(deltaV < 0) deltaV = -deltaV;
									deltaInv	= 255 - deltaV;
									*dstPxH		= ((255 * 255 * 255) - (deltaInv * deltaInv * deltaInv)) / (255 * 255);
								}
								//V
								if(sclArea.anlzH){
									deltaH		= (SI32)gry - (SI32)prevLine[x - 1]; //prev-pix was already fed to prevLine
									NB_AADETECT_ASSERT(deltaH >= -255 && deltaH <= 255)
									if(deltaH < 0) deltaH = -deltaH;
									deltaInv	= 255 - deltaH;
									*dstPxV		= ((255 * 255 * 255) - (deltaInv * deltaInv * deltaInv)) / (255 * 255);
								}
							}
							//Visual
							if(tsk->scaled.visual.data != NULL){
								tsk->scaled.visual.data[(tsk->scaled.visual.props.bytesPerLine * y) + (tsk->scaled.visual.bytesPerPx * x)] = (BYTE)gry;
							}
						}
						//Save value
						prevLine[x] = (BYTE)gry;
					}
				}
			}
		}
	}
	//Saturate-H (the brightest absorbs half of its neighborg)
	if(sclArea.anlzH){
		SI32 x, orgPrev, org, nVal; UI8 *pxPrev, *px;
		UI8* lnStart = tsk->scaled.h.data + ((UI64)tsk->scaled.h.props.bytesPerLine * (UI64)yStart2);
		const UI8* lnAfterEnd = tsk->scaled.h.data + ((UI64)tsk->scaled.h.props.bytesPerLine * (UI64)yAfterEnd);
		SI32 iPass; for(iPass = 0 ; iPass < cfg->filter.saturation.passes; iPass++){
			for(x = xStart2; x < xAfterEnd; x++){
				//first direction
				pxPrev		= lnStart + ((UI64)x * (UI64)tsk->scaled.h.bytesPerPx);
				px			= pxPrev + tsk->scaled.h.props.bytesPerLine;
				orgPrev		= *pxPrev;	//original value
				while(px < lnAfterEnd){
					org		= *px;
					if(orgPrev > org){
						nVal	= *pxPrev + (org / 2); //(((255 - orgPrev) * (org / 2)) / 255); NBASSERT(nVal >= 0 && nVal <= 255)
						*pxPrev	= (nVal > 255 ? 255 : nVal); //(UI8)nVal; //
						*px		= *px - (org / 2);
					} else {
						nVal	= *px + (orgPrev / 2); //(((255 - org) * (orgPrev / 2)) / 255); NBASSERT(nVal >= 0 && nVal <= 255)
						*px		= (nVal > 255 ? 255 : nVal); //(UI8)nVal; //
						*pxPrev	= *pxPrev - (orgPrev / 2);
					}
					//Next px
					orgPrev	= org;
					pxPrev = px; 
					px += tsk->scaled.h.props.bytesPerLine;
				}
			}
		}
	}
	//Saturate-V (the brightest absorbs half of its neighborg)
	if(sclArea.anlzV){
		SI32 y, orgPrev, org, nVal; UI8 *pxPrev, *px;
		const UI8* pxAfterEnd;
		SI32 iPass; for(iPass = 0 ; iPass < cfg->filter.saturation.passes; iPass++){
			for(y = yStart2; y < yAfterEnd; y++){
				//first direction
				pxPrev		= tsk->scaled.v.data + ((UI64)y * (UI64)tsk->scaled.v.props.bytesPerLine) + ((UI64)xStart2 * (UI64)tsk->scaled.v.bytesPerPx);
				px			= pxPrev + 1;
				orgPrev		= *pxPrev;	//original value
				pxAfterEnd	= pxPrev + (((UI64)xAfterEnd - (UI64)xStart2) * (UI64)tsk->scaled.v.bytesPerPx);
				while(px < pxAfterEnd){
					org		= *px;
					if(orgPrev > org){
						nVal	= *pxPrev + (org / 2); //(((255 - orgPrev) * (org / 2)) / 255); NBASSERT(nVal >= 0 && nVal <= 255)
						*pxPrev	= (nVal > 255 ? 255 : nVal); //(UI8)nVal; //
						*px		= *px - (org / 2);
					} else {
						nVal	= *px + (orgPrev / 2); //(((255 - org) * (orgPrev / 2)) / 255); NBASSERT(nVal >= 0 && nVal <= 255)
						*px		= (nVal > 255 ? 255 : nVal); //(UI8)nVal; //
						*pxPrev	= *pxPrev - (orgPrev / 2);
					}
					//Next px
					orgPrev	= org;
					pxPrev = px; 
					px += tsk->scaled.v.bytesPerPx;
				}
			}
		}
	}
}

BOOL NBAADetection_buildFilteredBmp_(const STNBAADetectionCfg* cfg, STNBBitmap* org, STNBBitmap* dstH, STNBBitmap* dstV, STNBBitmap* dstVisual, const STNBAADetectionArea* areas, const UI32 areasSz, STNBArray* dstAreasScld, BYTE** prevLineData, UI32* prevLineSz){
	BOOL r = FALSE;
	if(cfg != NULL && org != NULL && dstH != NULL && dstV != NULL && prevLineData != NULL && prevLineSz != NULL){
		STNBAADetectionFilteresTsk tsk;
		NBMemory_setZeroSt(tsk, STNBAADetectionFilteresTsk);
		tsk.org.props		= NBBitmap_getProps(org);
		tsk.org.data		= NBBitmap_getData(org);
		tsk.scaled.scale	= (tsk.org.props.size.width > tsk.org.props.size.height ? tsk.org.props.size.width : tsk.org.props.size.height) / cfg->filter.bmpMaxSz;
		tsk.scaled.size		= NBST_P(STNBSizeI, (tsk.org.props.size.width + (tsk.scaled.scale - 1)) / tsk.scaled.scale, (tsk.org.props.size.height + (tsk.scaled.scale - 1)) / tsk.scaled.scale);
		if(tsk.org.data == NULL || tsk.org.props.size.width <= 0 || tsk.org.props.size.height <= 0 || tsk.scaled.scale <= 0){
			PRINTF_ERROR("AUScenwIntro, camera, no image.\n");
		} else if(tsk.org.props.color != ENNBBitmapColor_RGB8 && tsk.org.props.color != ENNBBitmapColor_RGBA8 && tsk.org.props.color != ENNBBitmapColor_BGRA8 && tsk.org.props.color != ENNBBitmapColor_ARGB8){
			PRINTF_ERROR("AUScenwIntro, camera, expected RGB8, RGBA8, BGRA8 or ARGB8 image.\n");
		} else {
			//Resize dst if necesary
			{
				//H
				{
					const STNBBitmapProps scldProps = NBBitmap_getProps(dstH);
					if(scldProps.size.width != tsk.scaled.size.width || scldProps.size.height != tsk.scaled.size.height){
						if(!NBBitmap_create(dstH, tsk.scaled.size.width, tsk.scaled.size.height, ENNBBitmapColor_GRIS8)){
							PRINTF_ERROR("AUScenwIntro, camera, could not create scaled-H (%d x %d).\n", tsk.scaled.size.width, tsk.scaled.size.height);
						} else {
							PRINTF_INFO("AUScenwIntro, camera, created scaled-H (%d x %d) with scale %d.\n", tsk.scaled.size.width, tsk.scaled.size.height, tsk.scaled.scale);
						}
					}
				}
				//V
				{
					const STNBBitmapProps scldProps = NBBitmap_getProps(dstV);
					if(scldProps.size.width != tsk.scaled.size.width || scldProps.size.height != tsk.scaled.size.height){
						if(!NBBitmap_create(dstV, tsk.scaled.size.width, tsk.scaled.size.height, ENNBBitmapColor_GRIS8)){
							PRINTF_ERROR("AUScenwIntro, camera, could not create scaled-V (%d x %d).\n", tsk.scaled.size.width, tsk.scaled.size.height);
						} else {
							PRINTF_INFO("AUScenwIntro, camera, created scaled-V (%d x %d) with scale %d.\n", tsk.scaled.size.width, tsk.scaled.size.height, tsk.scaled.scale);
						}
					}
				}
				//Visual
				if(dstVisual != NULL){
					const STNBBitmapProps scldProps = NBBitmap_getProps(dstVisual);
					if(scldProps.size.width != tsk.scaled.size.width || scldProps.size.height != tsk.scaled.size.height){
						if(!NBBitmap_create(dstVisual, tsk.scaled.size.width, tsk.scaled.size.height, ENNBBitmapColor_GRIS8)){
							PRINTF_ERROR("AUScenwIntro, camera, could not create scaled-visual (%d x %d).\n", tsk.scaled.size.width, tsk.scaled.size.height);
						} else {
							PRINTF_INFO("AUScenwIntro, camera, created scaled-visual (%d x %d) with scale %d.\n", tsk.scaled.size.width, tsk.scaled.size.height, tsk.scaled.scale);
						}
					}
				}
			}
			//Fill dst
			{
				tsk.scaled.h.props	= NBBitmap_getProps(dstH);
				tsk.scaled.h.data	= NBBitmap_getData(dstH);
				tsk.scaled.v.props	= NBBitmap_getProps(dstV);
				tsk.scaled.v.data	= NBBitmap_getData(dstV);
				if(dstVisual != NULL){
					tsk.scaled.visual.props	= NBBitmap_getProps(dstVisual);
					tsk.scaled.visual.data	= NBBitmap_getData(dstVisual);
				}
				if(
				   tsk.scaled.h.props.size.width == tsk.scaled.size.width
				   && tsk.scaled.h.props.size.height == tsk.scaled.size.height
				   && tsk.scaled.v.props.size.width == tsk.scaled.size.width
				   && tsk.scaled.v.props.size.height == tsk.scaled.size.height
				   )
				{
					//scale down
					tsk.scaled.algorithm		= TRUE;
					tsk.org.bytesPerPx			= (tsk.org.props.bitsPerPx / 8);
					tsk.scaled.h.bytesPerPx		= (tsk.scaled.h.props.bitsPerPx / 8);
					tsk.scaled.v.bytesPerPx		= (tsk.scaled.v.props.bitsPerPx / 8);
					tsk.scaled.visual.bytesPerPx = (tsk.scaled.visual.props.bitsPerPx / 8);
					//Resize prev line (if necesary)
					
					if(*prevLineSz < (tsk.scaled.h.props.bytesPerLine * (tsk.scaled.algorithm == ENNBAADetectAlgthm_AvgComps ? 3 : 1))){
						//Release previous
						if(*prevLineData != NULL){
							NBMemory_free(*prevLineData);
							*prevLineData = NULL;
						}
						//Allocate new
						*prevLineSz				= tsk.scaled.h.props.bytesPerLine * (tsk.scaled.algorithm == ENNBAADetectAlgthm_AvgComps ? 3 : 1);
						*prevLineData			= (BYTE*)NBMemory_alloc(*prevLineSz);
					}
					tsk.scaled.prevLine.data	= *prevLineData;
					tsk.scaled.prevLine.sz		= *prevLineSz;
					NBASSERT(dstAreasScld->use == 0)
					//Process areas
					if(areas == NULL || areasSz <= 0){
						//Full image
						STNBAADetectionArea sclArea;
						NBMemory_setZeroSt(sclArea, STNBAADetectionArea);
						sclArea.rect.x		= 0;
						sclArea.rect.y		= 0;
						sclArea.rect.width	= tsk.org.props.size.width / tsk.scaled.scale;
						sclArea.rect.height = tsk.org.props.size.height / tsk.scaled.scale;
						sclArea.anlzH		= sclArea.anlzV = TRUE;
						NBAADetection_buildFilteredPosterizedBmpArea_(cfg, &tsk, sclArea);
						//Add to array
						NBArray_addValue(dstAreasScld, sclArea);
					} else {
						//build filtered and posterized
						{
							UI32 i; SI32 remain;
							for(i = 0; i < areasSz; i++){
								STNBAADetectionArea sclArea = areas[i];
								if(dstVisual != NULL || sclArea.anlzH || sclArea.anlzV){
									//filter
									{
										//force positive size
										if(sclArea.rect.width < 0){ sclArea.rect.x += sclArea.rect.width; sclArea.rect.width = -sclArea.rect.width; }; 
										if(sclArea.rect.height < 0){ sclArea.rect.y += sclArea.rect.height; sclArea.rect.height = -sclArea.rect.height; };
										//crop by position
										if(sclArea.rect.x < 0){ sclArea.rect.width += sclArea.rect.x; sclArea.rect.x = 0; }
										if(sclArea.rect.y < 0){ sclArea.rect.height += sclArea.rect.y; sclArea.rect.y = 0; }
										//crop by size
										if((sclArea.rect.x + sclArea.rect.width) > tsk.org.props.size.width) sclArea.rect.width -= ((sclArea.rect.x + sclArea.rect.width) - tsk.org.props.size.width);
										if((sclArea.rect.y + sclArea.rect.height) > tsk.org.props.size.height) sclArea.rect.height -= ((sclArea.rect.y + sclArea.rect.height) - tsk.org.props.size.height);
									}
									//align to scale to the left
									{
										remain = (sclArea.rect.x % tsk.scaled.scale);
										if(remain != 0){ sclArea.rect.x -= remain; sclArea.rect.width += remain; };
										//
										remain = (sclArea.rect.y % tsk.scaled.scale);
										if(remain != 0){ sclArea.rect.y -= remain; sclArea.rect.height += remain; };
										//
										sclArea.rect.x		/= tsk.scaled.scale;
										sclArea.rect.y		/= tsk.scaled.scale;
										sclArea.rect.width	/= tsk.scaled.scale; 
										sclArea.rect.height	/= tsk.scaled.scale;
									}
									//Execute
									NBAADetection_buildFilteredPosterizedBmpArea_(cfg, &tsk, sclArea);
									//Add to array
									NBArray_addValue(dstAreasScld, sclArea);
								}
							}
						}
					}
					//
					r = TRUE;
				}
			}
		}
	}
	return r;
}

//Pixel to mas task

#ifdef NB_AADETECT_ASSERTS_ENABLED
void NBAADetection_validateMaskLinks_(const STNBAADetectTask* tsk, const STNBAADetectPx* pxs, const UI32 pxsSz){
	const STNBAADetectOffset* nxtOff = NULL;
	const STNBAADetectPx *dstCur = pxs;
	const STNBAADetectPx* mapAfterEnd = pxs + pxsSz;
	UI32 dbgCount = 0, pathsCount = 0, multiPathsNodes = 0;
	while(dstCur < mapAfterEnd){
		pathsCount = 0;
		//Forward
		if(dstCur->mask & NB_LINE_H_BIT_RIGHT){
			pathsCount++;
			nxtOff = &tsk->offsetsFwrd[ENNBAADetectPxDir_Next];
			NB_AADETECT_ASSERT((dstCur + nxtOff->map)->mask & NB_LINE_H_BIT_LEFT)
		}
		if(dstCur->mask & NB_LINE_H_BIT_RIGHT_RIGHT){
			pathsCount++;
			nxtOff = &tsk->offsetsFwrd[ENNBAADetectPxDir_NextTwice];
			NB_AADETECT_ASSERT((dstCur + nxtOff->map)->mask & NB_LINE_H_BIT_LEFT_LEFT)
		}
		if(dstCur->mask & NB_LINE_H_BIT_RIGHT_UP){
			pathsCount++;
			nxtOff = &tsk->offsetsFwrd[ENNBAADetectPxDir_NextTwiceBckwd];
			NB_AADETECT_ASSERT((dstCur + nxtOff->map)->mask & NB_LINE_H_BIT_LEFT_DOWN)
		}
		if(dstCur->mask & NB_LINE_H_BIT_RIGHT_DOWN){
			pathsCount++;
			nxtOff = &tsk->offsetsFwrd[ENNBAADetectPxDir_NextTwiceFrwd];
			NB_AADETECT_ASSERT((dstCur + nxtOff->map)->mask & NB_LINE_H_BIT_LEFT_UP)
		}
		//Backwards
		if(dstCur->mask & NB_LINE_H_BIT_LEFT){
			nxtOff = &tsk->offsetsFwrd[ENNBAADetectPxDir_Next];
			NB_AADETECT_ASSERT((dstCur - nxtOff->map)->mask & NB_LINE_H_BIT_RIGHT)
		}
		if(dstCur->mask & NB_LINE_H_BIT_LEFT_LEFT){
			nxtOff = &tsk->offsetsFwrd[ENNBAADetectPxDir_NextTwice];
			NB_AADETECT_ASSERT((dstCur - nxtOff->map)->mask & NB_LINE_H_BIT_RIGHT_RIGHT)
		}
		if(dstCur->mask & NB_LINE_H_BIT_LEFT_DOWN){
			nxtOff = &tsk->offsetsFwrd[ENNBAADetectPxDir_NextTwiceBckwd];
			NB_AADETECT_ASSERT((dstCur - nxtOff->map)->mask & NB_LINE_H_BIT_RIGHT_UP)
		}
		if(dstCur->mask & NB_LINE_H_BIT_LEFT_UP){
			nxtOff = &tsk->offsetsFwrd[ENNBAADetectPxDir_NextTwiceFrwd];
			NB_AADETECT_ASSERT((dstCur - nxtOff->map)->mask & NB_LINE_H_BIT_RIGHT_DOWN)
		}
		if(pathsCount > 1) multiPathsNodes++;
		//next inner
		dstCur++;
		dbgCount++;
	}
	PRINTF_INFO("Lines, %u nodes with multiple posible paths.\n", multiPathsNodes);
	NB_AADETECT_ASSERT(dbgCount == pxsSz)
}
#endif

BOOL NBAADetection_buildMask_(STNBBitmap* filtered, const ENNBAADetectionBmp version, STNBAADetectGrp* dst, const STNBAADetectionArea* areas, const UI32 areasSz, const STNBAADetectTask* tsk){
	BOOL r = FALSE;
	if(filtered != NULL && dst != NULL && areas != NULL && areasSz > 0){
		const STNBBitmapProps props = NBBitmap_getProps(filtered);
		BYTE* data = NBBitmap_getData(filtered);
		if(props.bitsPerPx == 8 && props.size.width > 0 && props.size.height > 0 && data != NULL){
			const SI32 pxTotal		= (props.size.width * props.size.height);
#			ifdef NB_AADETECT_ASSERTS_ENABLED
			UI16 dbgMaxLenFwrd = 0, dbgMaxLenBcwrd = 0;
			UI32 dbgCountDeltasIgnoredFwrd = 0, dbgCountDeltasRemovedFwrd = 0, dbgCountDeltasRemovedBwrd = 0;
#			endif
			//
			NB_AADETECT_ASSERT(tsk->img.minBright > 0)
			NB_AADETECT_ASSERT(tsk->lines.minLen > 0)
			NB_AADETECT_ASSERT(tsk->lines.minLenDelta > 0)
			NB_AADETECT_ASSERT(tsk->offsetsFwrd[ENNBAADetectPxDir_Next].img != 0)
			NB_AADETECT_ASSERT(tsk->offsetsFwrd[ENNBAADetectPxDir_Next].map != 0)
			NB_AADETECT_ASSERT(tsk->offsetsFwrd[ENNBAADetectPxDir_Next].bitFwrd != 0)
			NB_AADETECT_ASSERT(tsk->offsetsFwrd[ENNBAADetectPxDir_Next].bitBwrd != 0)
			NB_AADETECT_ASSERT(tsk->offsetsFwrd[ENNBAADetectPxDir_Next].lenAdd > 0)
			NB_AADETECT_ASSERT(tsk->offsetsFwrd[ENNBAADetectPxDir_NextTwice].img != 0)
			NB_AADETECT_ASSERT(tsk->offsetsFwrd[ENNBAADetectPxDir_NextTwice].map != 0)
			NB_AADETECT_ASSERT(tsk->offsetsFwrd[ENNBAADetectPxDir_NextTwice].lenAdd > 0)
			NB_AADETECT_ASSERT(tsk->offsetsFwrd[ENNBAADetectPxDir_NextTwice].bitFwrd != 0)
			NB_AADETECT_ASSERT(tsk->offsetsFwrd[ENNBAADetectPxDir_NextTwice].bitBwrd != 0)
			NB_AADETECT_ASSERT(tsk->offsetsFwrd[ENNBAADetectPxDir_NextTwiceFrwd].img != 0)
			NB_AADETECT_ASSERT(tsk->offsetsFwrd[ENNBAADetectPxDir_NextTwiceFrwd].map != 0)
			NB_AADETECT_ASSERT(tsk->offsetsFwrd[ENNBAADetectPxDir_NextTwiceFrwd].lenAdd > 0)
			NB_AADETECT_ASSERT(tsk->offsetsFwrd[ENNBAADetectPxDir_NextTwiceFrwd].bitFwrd != 0)
			NB_AADETECT_ASSERT(tsk->offsetsFwrd[ENNBAADetectPxDir_NextTwiceFrwd].bitBwrd != 0)
			NB_AADETECT_ASSERT(tsk->offsetsFwrd[ENNBAADetectPxDir_NextTwiceBckwd].img != 0)
			NB_AADETECT_ASSERT(tsk->offsetsFwrd[ENNBAADetectPxDir_NextTwiceBckwd].map != 0)
			NB_AADETECT_ASSERT(tsk->offsetsFwrd[ENNBAADetectPxDir_NextTwiceBckwd].lenAdd > 0)
			NB_AADETECT_ASSERT(tsk->offsetsFwrd[ENNBAADetectPxDir_NextTwiceBckwd].bitFwrd != 0)
			NB_AADETECT_ASSERT(tsk->offsetsFwrd[ENNBAADetectPxDir_NextTwiceBckwd].bitBwrd != 0)
			//
			//Note: when moving 1px in one direction and 2px in a perpendicular,
			//the distance is 2.236px ~(9/4) ~2.25.
			//
			//resize masks-buff (if necesary)
			if(dst->mask.buffSz < pxTotal){
				//release
				if(dst->mask.buff != NULL){
					NBMemory_free(dst->mask.buff);
					dst->mask.buff		= NULL;
					dst->mask.buffSz	= 0;
				}
				//create
				dst->mask.buffSz	= pxTotal;
				dst->mask.buff		= NBMemory_allocTypes(STNBAADetectPx, dst->mask.buffSz);
				dst->mask.width		= props.size.width;
				dst->mask.height	= props.size.height;
			}
			//clear masks-buff
			{
				NBMemory_set(dst->mask.buff, 0, sizeof(dst->mask.buff[0]) * pxTotal);
				NBArraySorted_empty(&dst->mask.areas);
				//Build sorted array
				{
					UI32 i; for(i = 0; i < areasSz; i++){
						const STNBAADetectionArea aa = areas[i];
						if((version == ENNBAADetectionBmp_HLines && aa.anlzV) || (version == ENNBAADetectionBmp_VLines && aa.anlzH)){
							NBArraySorted_addValue(&dst->mask.areas, aa);
						}
					}
				}
			}
			//Build mask of posible directions and calculate length forward
			{
				SI32 i; for(i = 0; i < dst->mask.areas.use; i++){
					const STNBAADetectionArea aa = NBArraySorted_itmValueAtIndex(&dst->mask.areas, STNBAADetectionArea, i);
					SI32 dbgCount = 0;
					//outer
					const BYTE *outImgStart		= data			+ ((UI64)(tsk->cycles.outer.isHoriz ? aa.rect.x : aa.rect.y) * (UI64)tsk->cycles.outer.next.img);
					const BYTE *outImgEnd		= outImgStart	+ ((UI64)(tsk->cycles.outer.isHoriz ? aa.rect.width : aa.rect.height) * (UI64)tsk->cycles.outer.next.img);
					STNBAADetectPx* outMapStart	= dst->mask.buff + ((UI64)(tsk->cycles.outer.isHoriz ? aa.rect.x : aa.rect.y) * (UI64)tsk->cycles.outer.next.map);
					//inner
					const BYTE *pxFirstt = data, *pxAfterEndd = data + ((UI64)props.bytesPerLine * (UI64)props.size.height);
					const BYTE *pxCur, *pxNxt, *pxCurEnd; const STNBAADetectOffset* nxtOff = NULL;
					STNBAADetectPx *dstCur, *dstNxt, *dstPrv; SI32 i; UI8 mskFnd = 0;
					//outer cicle
					NB_AADETECT_ASSERT(outImgStart < outImgEnd)
					while(outImgStart < outImgEnd){
						pxCur  		= outImgStart	+ ((UI64)(tsk->cycles.inner.isHoriz ? aa.rect.x : aa.rect.y) * (UI64)tsk->cycles.inner.next.img);
						pxCurEnd	= pxCur			+ ((UI64)(tsk->cycles.inner.isHoriz ? aa.rect.width : aa.rect.height) * (UI64)tsk->cycles.inner.next.img);
						dstCur		= outMapStart	+ ((UI64)(tsk->cycles.inner.isHoriz ? aa.rect.x : aa.rect.y) * (UI64)tsk->cycles.inner.next.map);
						//inner cicle
						NB_AADETECT_ASSERT(pxCur < pxCurEnd)
						while(pxCur < pxCurEnd){
							NB_AADETECT_ASSERT(dstCur >= dst->mask.buff && dstCur < (dst->mask.buff + pxTotal))
							NB_AADETECT_ASSERT(pxCur >= pxFirstt && pxCur < pxAfterEndd)
							NB_AADETECT_ASSERT(!(dstCur->mask & NB_LINE_H_BIT_RIGHT_ANY))
							//PRINTF_INFO("Forward(%d, %d).\n", (dstCur - dst->mask.buff) % props.size.width, (dstCur - dst->mask.buff) / props.size.width);
							if(*pxCur >= tsk->img.minBright){
								//Add links and lengths forward
								{
									mskFnd = 0;
									for(i = 0; i < ENNBAADetectPxDir_Count; i++){
										nxtOff	= &tsk->offsetsFwrd[i];
										pxNxt	= pxCur + nxtOff->img;
										if(!(mskFnd & nxtOff->bitFwrdForbid) && pxNxt >= pxFirstt && pxNxt < pxAfterEndd && *pxNxt >= tsk->img.minBright){
											mskFnd |= nxtOff->bitFwrd;
											dstNxt = dstCur + nxtOff->map;
											if(dstNxt->lenFwrd < (dstCur->lenFwrd + nxtOff->lenAdd)){
												//I'm the longest path (take ownership)
												dstNxt->lenFwrd = dstCur->lenFwrd + nxtOff->lenAdd;
												dstNxt->iLngstFwrd = (UI8)i;
												dstCur->mask |= nxtOff->bitFwrd;
												dstNxt->mask |= nxtOff->bitBwrd;
											} else /*if(dstCur->lenFwrd >= tsk->lines.minLenDelta){
													//I'm not the longest path, but long enough as a delta
													dstCur->mask |= nxtOff->bitFwrd;
													dstNxt->mask |= nxtOff->bitBwrd;
													} else*/ {
														NB_AADETECT_IF_ASSERTING(dbgCountDeltasIgnoredFwrd++;)
													}
										}
									}
								}
								//Remove short deltas backwards
								if(dstCur->mask & NB_LINE_H_BIT_LEFT_ANY & ~tsk->offsetsFwrd[dstCur->iLngstFwrd].bitBwrd){
									for(i = 0; i < ENNBAADetectPxDir_Count; i++){
										nxtOff	= &tsk->offsetsFwrd[i];
										if(dstCur->mask & nxtOff->bitBwrd & ~tsk->offsetsFwrd[dstCur->iLngstFwrd].bitBwrd){
											dstPrv = dstCur - nxtOff->map;
											//this is a delta (not the longest)
											if(dstPrv->lenFwrd < tsk->lines.minLenDelta){
												//disconnect
												NB_AADETECT_ASSERT(dstCur->mask & nxtOff->bitBwrd)
												NB_AADETECT_ASSERT(dstPrv->mask & nxtOff->bitFwrd)
												dstCur->mask &= ~nxtOff->bitBwrd;
												dstPrv->mask &= ~nxtOff->bitFwrd;
												NB_AADETECT_IF_ASSERTING(dbgCountDeltasRemovedFwrd++;)
											}
										}
									}
								}
							}
							//Reset bit for next ste
							NB_AADETECT_ASSERT(dstCur->lenBwrd == 0)
							NB_AADETECT_IF_ASSERTING(if(dbgMaxLenFwrd < dstCur->lenFwrd) dbgMaxLenFwrd = dstCur->lenFwrd;)
							//
							NB_AADETECT_ASSERT((((dstCur->mask & NB_LINE_H_BIT_RIGHT) ? 1 : 0) + ((dstCur->mask & NB_LINE_H_BIT_RIGHT_RIGHT) ? 1 : 0)) < 2) //only one of them
							NB_AADETECT_ASSERT((((dstCur->mask & NB_LINE_H_BIT_LEFT) ? 1 : 0) + ((dstCur->mask & NB_LINE_H_BIT_LEFT_LEFT) ? 1 : 0)) < 2) //only one of them
							//next inner
							pxCur	+= tsk->cycles.inner.next.img;
							dstCur	+= tsk->cycles.inner.next.map; 
							dbgCount++;
						}
						//next outer
						outImgStart	+= tsk->cycles.outer.next.img;
						outMapStart	+= tsk->cycles.outer.next.map;
					} NB_AADETECT_ASSERT(dbgCount == (aa.rect.width * aa.rect.height))
				}
			} 
			//Calculate length backwards (using previously calculated masks),
			//and remove joints to very short deltas.
			{
				SI32 i; for(i = (SI32)dst->mask.areas.use - 1; i >= 0; i--){
					const STNBAADetectionArea aa = NBArraySorted_itmValueAtIndex(&dst->mask.areas, STNBAADetectionArea, i);
					SI32 dbgCount = 0;
					//outer
					STNBAADetectPx* outMapEnd = dst->mask.buff + ((UI64)(tsk->cycles.outer.isHoriz ? aa.rect.x : aa.rect.y) * (UI64)tsk->cycles.outer.next.map);
					STNBAADetectPx* outMapStart = outMapEnd + ((UI64)(tsk->cycles.outer.isHoriz ? (aa.rect.width - 1) : (aa.rect.height - 1)) * (UI64)tsk->cycles.outer.next.map);
					//inner
					const STNBAADetectOffset* nxtOff = NULL;
					SI32 i; STNBAADetectPx *dstCur, *dstCurEnd, *dstNxt, *dstPrv;
					NB_AADETECT_ASSERT(outMapStart > outMapEnd)
					while(outMapStart >= outMapEnd){
						dstCur		= outMapStart + ((UI64)(tsk->cycles.inner.isHoriz ? aa.rect.x : aa.rect.y) * (UI64)tsk->cycles.inner.next.map);
						dstCurEnd	= dstCur + ((UI64)(tsk->cycles.inner.isHoriz ? aa.rect.width : aa.rect.height) * (UI64)tsk->cycles.inner.next.map);
						NB_AADETECT_ASSERT(dstCur < dstCurEnd)
						while(dstCur < dstCurEnd){
							NB_AADETECT_ASSERT(dstCur >= dst->mask.buff && dstCur < (dst->mask.buff + pxTotal))
							NB_AADETECT_ASSERT(dstCur >= dst->mask.buff && dstCur < (dst->mask.buff + pxTotal))
							//PRINTF_INFO("Backwards(%d, %d).\n", (dstCur - dst->mask.buff) % props.size.width, (dstCur - dst->mask.buff) / props.size.width);
							//Add lengths backwards
							if(dstCur->mask & NB_LINE_H_BIT_LEFT_ANY){
								for(i = 0; i < ENNBAADetectPxDir_Count; i++){
									nxtOff	= &tsk->offsetsFwrd[i];
									if(dstCur->mask & nxtOff->bitBwrd){
										dstPrv = dstCur - nxtOff->map;
										NB_AADETECT_ASSERT(dstPrv->mask & nxtOff->bitFwrd)
										if(dstPrv->lenBwrd < (dstCur->lenBwrd + nxtOff->lenAdd)){
											dstPrv->lenBwrd = dstCur->lenBwrd + nxtOff->lenAdd;
											dstPrv->iLngstBwrd = (UI8)i;
										}
									}
								}
							}
							//Remove short deltas forwards
							if(dstCur->mask & NB_LINE_H_BIT_RIGHT_ANY & ~tsk->offsetsFwrd[dstCur->iLngstBwrd].bitFwrd){
								for(i = 0; i < ENNBAADetectPxDir_Count; i++){
									nxtOff	= &tsk->offsetsFwrd[i];
									if(dstCur->mask & nxtOff->bitFwrd & ~tsk->offsetsFwrd[dstCur->iLngstBwrd].bitFwrd){
										dstNxt = dstCur + nxtOff->map;
										//this is a delta (not the longest)
										if(dstNxt->lenBwrd < tsk->lines.minLenDelta){
											//disconnect
											NB_AADETECT_ASSERT(dstCur->mask & nxtOff->bitFwrd)
											NB_AADETECT_ASSERT(dstNxt->mask & nxtOff->bitBwrd)
											dstCur->mask &= ~nxtOff->bitFwrd;
											dstNxt->mask &= ~nxtOff->bitBwrd;
											NB_AADETECT_IF_ASSERTING(dbgCountDeltasRemovedBwrd++;)
										}
									}
								}
							}
							NB_AADETECT_ASSERT((((dstCur->mask & NB_LINE_H_BIT_RIGHT) ? 1 : 0) + ((dstCur->mask & NB_LINE_H_BIT_RIGHT_RIGHT) ? 1 : 0)) < 2) //only one of them
							NB_AADETECT_ASSERT((((dstCur->mask & NB_LINE_H_BIT_LEFT) ? 1 : 0) + ((dstCur->mask & NB_LINE_H_BIT_LEFT_LEFT) ? 1 : 0)) < 2) //only one of them
							//Reset lenFwrd (for next step)
							dstCur->lenFwrd = 0;
							NB_AADETECT_IF_ASSERTING(if(dbgMaxLenBcwrd < dstCur->lenBwrd) dbgMaxLenBcwrd = dstCur->lenBwrd;)
							//next inner
							dstCur	+= tsk->cycles.inner.next.map; 
							dbgCount++;
						}
						//next outer
						outMapStart	-= tsk->cycles.outer.next.map;
					}
					NB_AADETECT_ASSERT(dbgCount == (aa.rect.width * aa.rect.height))
				}
			}
			//only valid when delta removal is disabled
			//NB_AADETECT_ASSERT(dbgMaxLenBcwrd == dbgMaxLenFwrd)
			//
			//NB_AADETECT_IF_ASSERTING(NBAADetection_validateMaskLinks_(tsk, dst->mask.buff, pxTotal);)
			//
			//PRINTF_INFO("Lines-%c, %u short-deltas rmvd, fwrd(%u ign, %d rmvd) bwrd(%d rmvd), dbgMaxLenFwrd(%d)-dbgMaxLenBcwrd(%d).\n", (version == ENNBAADetectionBmp_HLines ? 'H' : 'V'), (dbgCountDeltasIgnoredFwrd + dbgCountDeltasRemovedFwrd + dbgCountDeltasRemovedBwrd), dbgCountDeltasIgnoredFwrd, dbgCountDeltasRemovedFwrd, dbgCountDeltasRemovedBwrd, dbgMaxLenFwrd, dbgMaxLenBcwrd);
			//
			r = TRUE;
		}
	}
	return r;
}


BOOL NBAADetection_buildMaskH_(const STNBAADetectionCfg* cfg, STNBBitmap* filtered, const STNBAADetectionArea* areas, const UI32 areasSz, STNBAADetectGrp* dst){
	BOOL r = FALSE;
	if(cfg != NULL && filtered != NULL && areas != NULL && areasSz > 0 && dst != NULL){
		const STNBBitmapProps props = NBBitmap_getProps(filtered);
		BYTE* data = NBBitmap_getData(filtered);
		if(props.bitsPerPx == 8 && props.size.width > 0 && props.size.height > 0 && data != NULL){
			const STNBAADetectTask tsk = NBAADetection_getScanTaskRowsPerCol_(cfg, filtered);
			r = NBAADetection_buildMask_(filtered, ENNBAADetectionBmp_HLines, dst, areas, areasSz, &tsk);
		}
	}
	return r;
}

BOOL NBAADetection_buildMaskV_(const STNBAADetectionCfg* cfg, STNBBitmap* filtered, const STNBAADetectionArea* areas, const UI32 areasSz, STNBAADetectGrp* dst){
	BOOL r = FALSE;
	if(cfg != NULL && filtered != NULL && areas != NULL && areasSz > 0 && dst != NULL){
		const STNBBitmapProps props = NBBitmap_getProps(filtered);
		BYTE* data = NBBitmap_getData(filtered);
		if(props.bitsPerPx == 8 && props.size.width > 0 && props.size.height > 0 && data != NULL){
			const STNBAADetectTask tsk = NBAADetection_getScanTaskColsPerRow_(cfg, filtered);
			r = NBAADetection_buildMask_(filtered, ENNBAADetectionBmp_VLines, dst, areas, areasSz, &tsk);
		}
	}
	return r;
}

void NBAADetection_buildLines_(const STNBAADetectState* state, const STNBAADetectTask* tsk, const STNBAADetectPx *pxCur, STNBAADetectPath* path, const UI32 depth, STNBAADetectStats* dstStats){
	BOOL advanced = FALSE; const STNBAADetectPx *pxCurBase = NULL;
	//Advance on this call and start recursive calls
	const STNBAADetectOffset* nxtOff = NULL;
#	ifdef NB_AADETECT_ASSERTS_ENABLED
	const STNBAADetectPx *pxCurLst = NULL;
#	endif
	do {
#		ifdef NB_AADETECT_ASSERTS_ENABLED
		const SI32 x = (SI32)((pxCur - state->lnFirst) % state->width);
		const SI32 y = (SI32)((pxCur - state->lnFirst) / state->width);
		NB_AADETECT_ASSERT(x >= 0 && x < state->width)
		NB_AADETECT_ASSERT(y >= 0 && y < state->height)
#		endif
		NB_AADETECT_ASSERT(pxCur >= state->lnFirst && pxCur < state->lnAfterLast)
		NB_AADETECT_ASSERT(pxCur != pxCurLst)
		NB_AADETECT_IF_ASSERTING(pxCurLst = pxCur;)
		pxCurBase = pxCur;
		advanced = FALSE;
		//NB_AADETECT_ASSERT(validMask != 0)
		//Right
		if(pxCurBase->mask & NB_LINE_H_BIT_RIGHT){
			nxtOff = &tsk->offsetsFwrd[ENNBAADetectPxDir_Next];
			path->curLen += nxtOff->lenAdd;
			NB_AADETECT_ASSERT(nxtOff->img != 0 && nxtOff->map != 0 && nxtOff->lenAdd > 0)
			NB_AADETECT_ASSERT((pxCurBase + nxtOff->map) >= state->lnFirst && (pxCurBase + nxtOff->map) < state->lnAfterLast)
			NB_AADETECT_ASSERT((pxCurBase + nxtOff->map)->mask & NB_LINE_H_BIT_LEFT)
			if(advanced){
				//Continue in recursive call
				STNBAADetectPath path2 = *path;
				dstStats->recursiveCount++;
				NBAADetection_buildLines_(state, tsk, pxCurBase + nxtOff->map, &path2, depth + 1, dstStats);
			} else {
				//Continue in this call
				advanced = TRUE;
				pxCur += nxtOff->map;
				//NB_AADETECT_ASSERT(pxCur >= state->lnFirst && pxCur < state->lnAfterLast)
			}
			//pxCurBase->mask &= ~(NB_LINE_H_BIT_RIGHT | NB_LINE_H_BIT_RIGHT_RIGHT);
		} else if(pxCurBase->mask & NB_LINE_H_BIT_RIGHT_RIGHT){
			nxtOff = &tsk->offsetsFwrd[ENNBAADetectPxDir_NextTwice];
			path->curLen += nxtOff->lenAdd;
			NB_AADETECT_ASSERT(nxtOff->img != 0 && nxtOff->map != 0 && nxtOff->lenAdd > 0)
			NB_AADETECT_ASSERT((pxCurBase + nxtOff->map) >= state->lnFirst && (pxCurBase + nxtOff->map) < state->lnAfterLast)
			NB_AADETECT_ASSERT((pxCurBase + nxtOff->map)->mask & NB_LINE_H_BIT_LEFT_LEFT)
			if(advanced){
				//Continue in recursive call
				STNBAADetectPath path2 = *path;
				dstStats->recursiveCount++;
				NBAADetection_buildLines_(state, tsk, pxCurBase + nxtOff->map, &path2, depth + 1, dstStats);
			} else {
				//Continue in this call
				advanced = TRUE;
				pxCur += nxtOff->map;
				//NB_AADETECT_ASSERT(pxCur >= state->lnFirst && pxCur < state->lnAfterLast)
			}
			//pxCurBase->mask &= ~(NB_LINE_H_BIT_RIGHT | NB_LINE_H_BIT_RIGHT_RIGHT);
		}
		//RighUp
		if(pxCurBase->mask & NB_LINE_H_BIT_RIGHT_UP){
			nxtOff = &tsk->offsetsFwrd[ENNBAADetectPxDir_NextTwiceBckwd];
			path->curLen += nxtOff->lenAdd;
			path->curLenUpLeft += nxtOff->lenAdd;
			NB_AADETECT_ASSERT(nxtOff->img != 0 && nxtOff->map != 0 && nxtOff->lenAdd > 0)
			NB_AADETECT_ASSERT((pxCurBase + nxtOff->map) >= state->lnFirst && (pxCurBase + nxtOff->map) < state->lnAfterLast)
			NB_AADETECT_ASSERT((pxCurBase + nxtOff->map)->mask & NB_LINE_H_BIT_LEFT_DOWN)
			if(advanced){
				//Continue in recursive call
				STNBAADetectPath path2 = *path;
				dstStats->recursiveCount++;
				NBAADetection_buildLines_(state, tsk, pxCurBase + nxtOff->map, &path2, depth + 1, dstStats);
			} else {
				//Continue in this call
				advanced = TRUE;
				pxCur += nxtOff->map;
				//NB_AADETECT_ASSERT(pxCur >= state->lnFirst && pxCur < state->lnAfterLast)
			}
			//pxCurBase->mask &= ~NB_LINE_H_BIT_RIGHT_UP;
		}
		//RightDown
		if(pxCurBase->mask & NB_LINE_H_BIT_RIGHT_DOWN){
			nxtOff = &tsk->offsetsFwrd[ENNBAADetectPxDir_NextTwiceFrwd];
			path->curLen += nxtOff->lenAdd;
			path->curLenDownRight += nxtOff->lenAdd;
			NB_AADETECT_ASSERT(nxtOff->img != 0 && nxtOff->map != 0 && nxtOff->lenAdd > 0)
			NB_AADETECT_ASSERT((pxCurBase + nxtOff->map) >= state->lnFirst && (pxCurBase + nxtOff->map) < state->lnAfterLast)
			NB_AADETECT_ASSERT((pxCurBase + nxtOff->map)->mask & NB_LINE_H_BIT_LEFT_UP)
			if(advanced){
				//Continue in recursive call
				STNBAADetectPath path2 = *path;
				dstStats->recursiveCount++;
				NBAADetection_buildLines_(state, tsk, pxCurBase + nxtOff->map, &path2, depth + 1, dstStats);
			} else {
				//Continue in this call
				advanced = TRUE;
				pxCur += nxtOff->map;
				//NB_AADETECT_ASSERT(pxCur >= state->lnFirst && pxCur < state->lnAfterLast)
			}
			//pxCurBase->mask &= ~NB_LINE_H_BIT_RIGHT_DOWN;
		}
	} while(advanced);
	//End-of-line
	//if(path->curLen < *path->longestAdded){
	//	//Another longer was already added
	//	dstStats->ignoredByLongerCount++;
	//	//PRINTF_INFO("Ignoring H-line len2(%u) from (%d, %d)-(%d, %d).\n", lenSqr, start.x, start.y, end.x, end.y);
	//} else {	
		IF_NBASSERT(const SI32 iStart		= (SI32)(state->pxStart - state->lnFirst);)
		IF_NBASSERT(const SI32 iEnd			= (SI32)(pxCur - state->lnFirst);)
		IF_NBASSERT(const STNBPointI start	= NBST_P(STNBPointI, (iStart % state->width), (iStart / state->width));) 
		IF_NBASSERT(const STNBPointI end	= NBST_P(STNBPointI, (iEnd % state->width), (iEnd / state->width));)
		//const UI32 lenSqr		= ((end.x - start.x) * (end.x - start.x)) + ((end.y - start.y) * (end.y - start.y));
		NB_AADETECT_ASSERT(start.x >= 0 && start.x < state->width && start.y >= 0 && start.y < state->height)
		NB_AADETECT_ASSERT(end.x >= 0 && end.x < state->width && end.y >= 0 && end.y < state->height)
		if(path->curLen < state->lineLenMin){
			dstStats->ignoredCount++;
			PRINTF_INFO("state(%llu) Ignoring H-line %d of %d minLen (%d %%); from(%d, %d)-to(%d, %d).\n", (UI64)state, path->curLen, state->lineLenMin, path->curLen * 100 / state->lineLenMin, start.x, start.y, end.x, end.y);
		} else {
			STNBAADetectionLine ln;
			NBMemory_setZeroSt(ln, STNBAADetectionLine);
			ln.iPxStart 			= (UI32)(state->pxStart - state->lnFirst);
			ln.iPxEnd				= (UI32)(pxCur - state->lnFirst);
			*path->longestAdded		= path->curLen;
			*path->longestAddedIdx	= dstStats->dstArr->use;
			NBArray_addValue(dstStats->dstArr, ln);
			dstStats->addedCount++;
			//PRINTF_INFO("state(%llu) Added H-line %d len (%d %%); from(%d, %d)-to(%d, %d).\n", (UI64)state, path->curLen, path->curLen * 100 / state->lineLenMin, start.x, start.y, end.x, end.y);
		}
	//}
	//Recursive depth
	if(dstStats->recursiveMaxDepth < depth){
		dstStats->recursiveMaxDepth = depth;
	}
}

BOOL NBAADetection_buildLinesTask_(STNBBitmap* filtered, STNBAADetectGrp* dst, const STNBAADetectTask* tsk){
	BOOL r = FALSE;
	if(filtered != NULL && dst != NULL){
		if(dst->mask.width > 0 && dst->mask.height > 0 && dst->mask.buff != NULL){
			const SI32 pxTotal = (dst->mask.width * dst->mask.height);
			//Find short lines
			{
				STNBAADetectStats stats;
				STNBAADetectState state;
				NBMemory_setZeroSt(stats, STNBAADetectStats);
				state.width			= dst->mask.width;
				state.height		= dst->mask.height;
				state.lineLenMin	= tsk->lines.minLen;
				state.lnFirst		= dst->mask.buff;
				state.lnAfterLast	= dst->mask.buff + pxTotal;
				state.pxStart		= dst->mask.buff;
				//
				stats.dstArr		= &dst->lines.shorts;
				//clear mask
				NBArray_empty(&dst->lines.shorts);
				//Populate short lines
				{
					SI32 i; for(i = 0; i < dst->mask.areas.use; i++){
						const STNBAADetectionArea aa = NBArraySorted_itmValueAtIndex(&dst->mask.areas, STNBAADetectionArea, i);
						STNBAADetectPx* ln = dst->mask.buff + ((UI64)aa.rect.y * (UI64)dst->mask.width);
						const STNBAADetectPx* lnAftrEnd = ln + ((UI64)aa.rect.height * (UI64)dst->mask.width);
						while(ln < lnAftrEnd){
							STNBAADetectPx* px = ln + aa.rect.x;
							const STNBAADetectPx* pxAftrEnd = px + aa.rect.width;
							while(px < pxAftrEnd){
								//const SI32 x = (SI32)((px - state.lnFirst) % props.size.width);
								//const SI32 y = (SI32)((px - state.lnFirst) / props.size.width);
								if(
								   (px->mask & NB_LINE_H_BIT_LEFT_ANY) == 0 && (px->mask & NB_LINE_H_BIT_RIGHT_ANY) != 0 //is a start
								   && px->lenBwrd >= tsk->lines.minLen //is long enough
								   )
								{
									UI32 longestAdded = 0, longestAddedIdx = 0, iFirst = dst->lines.shorts.use;
									STNBAADetectPath path;
									NBMemory_setZeroSt(path, STNBAADetectPath);
									path.longestAdded = &longestAdded;
									path.longestAddedIdx = &longestAddedIdx;
									//Add
									state.pxStart = px;
									NBAADetection_buildLines_(&state, tsk, px, &path, 1, &stats);
									//Process result
									if(longestAdded > 0){
										//Remove all except the longest added
										NB_AADETECT_ASSERT(longestAddedIdx < dst->lines.shorts.use)
										if(iFirst < longestAddedIdx){
											NBArray_removeItemsAtIndex(&dst->lines.shorts, iFirst, (longestAddedIdx - iFirst));
										}
										if((iFirst + 1) < dst->lines.shorts.use){
											NBArray_removeItemsAtIndex(&dst->lines.shorts, iFirst + 1, (dst->lines.shorts.use - iFirst - 1));
										}
										NB_AADETECT_ASSERT((iFirst + 1) == dst->lines.shorts.use) //only one should be kept
									} else {
										NB_AADETECT_ASSERT(iFirst == dst->lines.shorts.use) //none should be added
									}
								}
								//next pixel
								px++;
							}
							//next line
							ln += dst->mask.width;
						}
					}
				}
				//PRINTF_INFO("Lines %d kept, %d added, %d ignored, %d ignoredByLonger, %d recursive (%d max depth), %d intersections.\n", dst->lines.shorts.use, stats.addedCount, stats.ignoredCount, stats.ignoredByLongerCount, stats.recursiveCount, stats.recursiveMaxDepth, stats.intersectsCount);
			}
			//
			r = TRUE;
		}
	}
	return r;
}
	
BOOL NBAADetection_buildLinesH_(const STNBAADetectionCfg* cfg, STNBBitmap* filtered, STNBAADetectGrp* dst){
	const STNBAADetectTask tsk = NBAADetection_getScanTaskRowsPerCol_(cfg, filtered);
	return NBAADetection_buildLinesTask_(filtered, dst, &tsk);
} 

BOOL NBAADetection_buildLinesV_(const STNBAADetectionCfg* cfg, STNBBitmap* filtered, STNBAADetectGrp* dst){
	const STNBAADetectTask tsk = NBAADetection_getScanTaskColsPerRow_(cfg, filtered);
	return NBAADetection_buildLinesTask_(filtered, dst, &tsk);
}

//Tasks defs

STNBAADetectTask NBAADetection_getScanTaskRowsPerCol_(const STNBAADetectionCfg* cfg, STNBBitmap* filtered){
	STNBAADetectTask tsk;
	NBMemory_setZeroSt(tsk, STNBAADetectTask);
	if(cfg != NULL && filtered != NULL){
		const STNBBitmapProps props = NBBitmap_getProps(filtered);
		BYTE* data = NBBitmap_getData(filtered);
		if(props.bitsPerPx == 8 && props.size.width > 0 && props.size.height > 0 && data != NULL){
			//img
			tsk.img.minBright	= cfg->mask.minValue;
			//lines
			tsk.lines.minLen	= (UI16)((props.size.width < props.size.height ? props.size.width : props.size.height) * cfg->lines.lenScale *  cfg->lines.minRelLen / 100);
			tsk.lines.minLenDelta = (UI16)((props.size.width < props.size.height ? props.size.width : props.size.height) * cfg->lines.lenScale *  cfg->lines.deltas.minRelLen / 100);
			//offsets forward
			{
				//Right
				{
					STNBAADetectOffset* nxt = &tsk.offsetsFwrd[ENNBAADetectPxDir_Next];
					nxt->img		= 1;
					nxt->map		= 1;
					nxt->bitFwrd	= NB_LINE_H_BIT_RIGHT;
					nxt->bitBwrd	= NB_LINE_H_BIT_LEFT;
					nxt->lenAdd		= cfg->lines.lenScale;
				}
				//Right-right
				{
					STNBAADetectOffset* nxt = &tsk.offsetsFwrd[ENNBAADetectPxDir_NextTwice];
					nxt->img		= 2;
					nxt->map		= 2;
					nxt->bitFwrd		= NB_LINE_H_BIT_RIGHT_RIGHT;
					nxt->bitFwrdForbid	= NB_LINE_H_BIT_RIGHT;
					nxt->bitBwrd		= NB_LINE_H_BIT_LEFT_LEFT;
					nxt->bitBwrdForbid	= NB_LINE_H_BIT_LEFT;
					nxt->lenAdd		= cfg->lines.lenScale * 2;
				}
				//
				//Note: when moving 1px in one direction and 2px in a perpendicular,
				//the distance is 2.236px ~(9/4) ~2.25.
				//
				//Right-up
				{
					STNBAADetectOffset* nxt = &tsk.offsetsFwrd[ENNBAADetectPxDir_NextTwiceBckwd];
					nxt->img		= 2 - props.bytesPerLine;
					nxt->map		= 2 - props.size.width;
					nxt->bitFwrd	= NB_LINE_H_BIT_RIGHT_UP;
					nxt->bitBwrd	= NB_LINE_H_BIT_LEFT_DOWN;
					nxt->lenAdd		= cfg->lines.lenScale * 9 / 4; //~2.236px
				}
				//Right-down
				{
					STNBAADetectOffset* nxt = &tsk.offsetsFwrd[ENNBAADetectPxDir_NextTwiceFrwd];
					nxt->img		= 2 + props.bytesPerLine;
					nxt->map		= 2 + props.size.width;
					nxt->bitFwrd	= NB_LINE_H_BIT_RIGHT_DOWN;
					nxt->bitBwrd	= NB_LINE_H_BIT_LEFT_UP;
					nxt->lenAdd		= cfg->lines.lenScale * 9 / 4; //~2.236px
				}
			}
			//cycles
			{
				//outer (column by column)
				{
					tsk.cycles.outer.isHoriz	= TRUE;
					//
					tsk.cycles.outer.next.img	= 1;
					tsk.cycles.outer.next.map	= 1;
				}
				//inner (row by row)
				{
					tsk.cycles.inner.isHoriz	= FALSE;
					//
					tsk.cycles.inner.next.img	= props.bytesPerLine;
					tsk.cycles.inner.next.map	= props.size.width;
				}
			}
		}
	}
	return tsk;
}

STNBAADetectTask NBAADetection_getScanTaskColsPerRow_(const STNBAADetectionCfg* cfg, STNBBitmap* filtered){
	STNBAADetectTask tsk;
	NBMemory_setZeroSt(tsk, STNBAADetectTask);
	if(cfg != NULL && filtered != NULL){
		const STNBBitmapProps props = NBBitmap_getProps(filtered);
		BYTE* data = NBBitmap_getData(filtered);
		if(props.bitsPerPx == 8 && props.size.width > 0 && props.size.height > 0 && data != NULL){
			//img
			tsk.img.minBright	= cfg->mask.minValue;
			//lines
			tsk.lines.minLen	= (UI16)((props.size.width < props.size.height ? props.size.width : props.size.height) * cfg->lines.lenScale * cfg->lines.minRelLen / 100);
			tsk.lines.minLenDelta = (UI16)((props.size.width < props.size.height ? props.size.width : props.size.height) * cfg->lines.lenScale * cfg->lines.deltas.minRelLen / 100);
			//offsets forward
			{
				//Down
				{
					STNBAADetectOffset* nxt = &tsk.offsetsFwrd[ENNBAADetectPxDir_Next];
					nxt->img		= props.bytesPerLine;
					nxt->map		= props.size.width;
					nxt->bitFwrd	= NB_LINE_V_BIT_DOWN; NB_AADETECT_ASSERT(nxt->bitFwrd == NB_LINE_H_BIT_RIGHT)
					nxt->bitBwrd	= NB_LINE_V_BIT_UP; NB_AADETECT_ASSERT(nxt->bitBwrd == NB_LINE_H_BIT_LEFT)
					nxt->lenAdd		= cfg->lines.lenScale;
				}
				//Down-Down
				{
					STNBAADetectOffset* nxt = &tsk.offsetsFwrd[ENNBAADetectPxDir_NextTwice];
					nxt->img		= 2 * props.bytesPerLine;
					nxt->map		= 2 * props.size.width;
					nxt->bitFwrd	= NB_LINE_V_BIT_DOWN_DOWN; NB_AADETECT_ASSERT(nxt->bitFwrd == NB_LINE_H_BIT_RIGHT_RIGHT)
					nxt->bitFwrdForbid	= NB_LINE_V_BIT_DOWN;
					nxt->bitBwrd	= NB_LINE_V_BIT_UP_UP; NB_AADETECT_ASSERT(nxt->bitBwrd == NB_LINE_H_BIT_LEFT_LEFT)
					nxt->bitBwrdForbid	= NB_LINE_V_BIT_UP;
					nxt->lenAdd		= cfg->lines.lenScale * 2;
				}
				//
				//Note: when moving 1px in one direction and 2px in a perpendicular,
				//the distance is 2.236px ~(9/4) ~2.25.
				//
				//Down-left
				{
					STNBAADetectOffset* nxt = &tsk.offsetsFwrd[ENNBAADetectPxDir_NextTwiceBckwd];
					nxt->img		= (2 * props.bytesPerLine) - 1;
					nxt->map		= (2 * props.size.width) - 1;
					nxt->bitFwrd	= NB_LINE_V_BIT_DOWN_LEFT; NB_AADETECT_ASSERT(nxt->bitFwrd == NB_LINE_H_BIT_RIGHT_UP)
					nxt->bitBwrd	= NB_LINE_V_BIT_UP_RIGHT; NB_AADETECT_ASSERT(nxt->bitBwrd == NB_LINE_H_BIT_LEFT_DOWN)
					nxt->lenAdd		= cfg->lines.lenScale * 9 / 4; //~2.236px
				}
				//Down-right
				{
					STNBAADetectOffset* nxt = &tsk.offsetsFwrd[ENNBAADetectPxDir_NextTwiceFrwd];
					nxt->img		= (2 * props.bytesPerLine) + 1;
					nxt->map		= (2 * props.size.width) + 1;
					nxt->bitFwrd	= NB_LINE_V_BIT_DOWN_RIGHT; NB_AADETECT_ASSERT(nxt->bitFwrd == NB_LINE_H_BIT_RIGHT_DOWN)
					nxt->bitBwrd	= NB_LINE_V_BIT_UP_LEFT; NB_AADETECT_ASSERT(nxt->bitBwrd == NB_LINE_H_BIT_LEFT_UP)
					nxt->lenAdd		= cfg->lines.lenScale * 9 / 4; //~2.236px
				}
			}
			//cycles
			{
				//outer (row by row)
				{
					tsk.cycles.outer.isHoriz	= FALSE;
					//
					tsk.cycles.outer.next.img	= props.bytesPerLine;
					tsk.cycles.outer.next.map	= props.size.width;
				}
				//inner (column by column)
				{
					tsk.cycles.inner.isHoriz	= TRUE;
					//
					tsk.cycles.inner.next.img	= 1;
					tsk.cycles.inner.next.map	= 1;
				}
			}
		}
	}
	return tsk;
}

//Compare

BOOL NBCompare_NBAADetectionAreaByX(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	const STNBAADetectionArea* d1 = (STNBAADetectionArea*)data1;
	const STNBAADetectionArea* d2 = (STNBAADetectionArea*)data2;
	NBASSERT(dataSz == sizeof(*d1))
	if(dataSz == sizeof(*d1)){
		switch (mode) {
			case ENCompareMode_Equal:
				return d1->rect.x == d2->rect.x;
			case ENCompareMode_Lower:
				return d1->rect.x < d2->rect.x;
			case ENCompareMode_LowerOrEqual:
				return d1->rect.x <= d2->rect.x;
			case ENCompareMode_Greater:
				return d1->rect.x > d2->rect.x;
			case ENCompareMode_GreaterOrEqual:
				return d1->rect.x >= d2->rect.x;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	NBASSERT(FALSE)
	return FALSE;
}

BOOL NBCompare_NBAADetectionAreaByY(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	const STNBAADetectionArea* d1 = (STNBAADetectionArea*)data1;
	const STNBAADetectionArea* d2 = (STNBAADetectionArea*)data2;
	NBASSERT(dataSz == sizeof(*d1))
	if(dataSz == sizeof(*d1)){
		switch (mode) {
			case ENCompareMode_Equal:
				return d1->rect.y == d2->rect.y;
			case ENCompareMode_Lower:
				return d1->rect.y < d2->rect.y;
			case ENCompareMode_LowerOrEqual:
				return d1->rect.y <= d2->rect.y;
			case ENCompareMode_Greater:
				return d1->rect.y > d2->rect.y;
			case ENCompareMode_GreaterOrEqual:
				return d1->rect.y >= d2->rect.y;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	NBASSERT(FALSE)
	return FALSE;
}
