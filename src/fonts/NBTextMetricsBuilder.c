//
//  NBTextMetricsBuilder.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/12/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/fonts/NBTextMetricsBuilder.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBEncoding.h"
#include "nb/2d/NBColor.h"
#include "nb/2d/NBPoint.h"
#include "nb/fonts/NBFontMetrics.h"

//Feed (in)

typedef struct STNBTextMetricsBuilderFmt_ {
	float				fontSz;
	STNBColor8			color;
	ENNBTextLineAlignH	lineAlignH;
	ENNBTextCharAlignV	charAlignV;
	STNBFontMetricsI	itf;
	void*				itfParam;
	void*				usrObject;
} STNBTextMetricsBuilderFmt;

typedef struct STNBTextMetricsBuilderFmtRng_ {
	STNBTextMetricsCharsRng		rng;
	STNBTextMetricsBuilderFmt	format;
} STNBTextMetricsBuilderFmtRng;

//Process state

typedef struct STNBTextMetricsBuilderWord_ {
	UI32				iByte;			//Start of current char in str
	float				curX;			//Cur relative-s inside word
	STNBTextMetricsWord	wd;
	//Extra-data
	STNBFontMetrics		fontMetricsMax;
	float				rBearing;		//last char's (hAdvance - width - hBearingX)
	float				lBearing;		//first char's hBearingX
	ENNBTextLineAlignH	alignH;
} STNBTextMetricsBuilderWord;

typedef struct STNBTextMetricsBuilderLine_ {
	BOOL				isExplicit;		//Line started explicit (by new-line character)
	UI32				iByte;			//Start of current char in str
	float				curX;			//Cur relative-s inside word
	STNBTextMetricsLine	ln;
	//Extra-data
	struct {
		UI32			words;			//visible words at line
		struct {
			float		xPos;			//left pos
			float		lBearing;		//first visible char's hBearingX
			struct {
				UI32	count;
				float	width;
			} separators;				//continuous spaces-words at the left of the line
		} left;
		struct {
			float		xPos;			//right pos (including advance)
			float		rBearing;		//last visible char's (hAdvance - width - hBearingX)
			struct {
				UI32	count;
				float	width;
			} separators;				//continuous spaces-words at the right of the line
		} right;
	} visible;
} STNBTextMetricsBuilderLine;

typedef struct STNBTextMetricsBuilderState_ {
	UI32								iByte;
	UI32								iNextFmtRng;
	STNBArray							fmtStack; //UI32 (used when feeding columns)
	STNBPoint							curPos;
	STNBRect							curCol;
	STNBTextMetricsBuilderWord			curWord;
	STNBTextMetricsBuilderLine			curLine;
	struct {
		STNBArray						idxs;			//UI32 (Line's all spaces/separators)
		float							width;			//Line's all spaces/separators width
	} curLineSpaces;
	const STNBTextMetricsBuilderFmtRng*	curRng;
	const STNBTextMetricsBuilderFmt*	curFmt;
	float								curFontSz;
	STNBFontMetrics						curFontMetrics;
} STNBTextMetricsBuilderState;

void NBTextMetricsBuilderState_init(STNBTextMetricsBuilderState* obj);
void NBTextMetricsBuilderState_release(STNBTextMetricsBuilderState* obj);
void NBTextMetricsBuilderState_reset(STNBTextMetricsBuilderState* obj);

//

typedef struct STNBTextMetricsBuilderOpq_ {
	STNBArray					fmtStack;		//UI32 (used when feeding format-blocks)
	STNBArray					fmtRanges;		//STNBTextMetricsBuilderFmtRng
	UI32						bytesSynced;	//Opened-rngs were synced to this size
	UI32						bytesAdded;		//Current size of the content
	STNBTextMetricsBuilderState	state;
	UI32						retainCount;
} STNBTextMetricsBuilderOpq;

//const STNBTextMetricsBuilderFmtRng* NBTextMetricsBuilder_getFormatForByteIdxOpq(const STNBTextMetricsBuilderOpq* opq, const UI32 byteIdx);

void NBTextMetricsBuilder_init(STNBTextMetricsBuilder* obj){
	STNBTextMetricsBuilderOpq* opq = obj->opaque = NBMemory_allocType(STNBTextMetricsBuilderOpq);
	NBMemory_setZeroSt(*opq, STNBTextMetricsBuilderOpq);
	NBArray_init(&opq->fmtStack, sizeof(UI32), NULL);
	NBArray_init(&opq->fmtRanges, sizeof(STNBTextMetricsBuilderFmtRng), NULL);
	opq->bytesSynced	= 0;
	opq->bytesAdded		= 0;
	NBTextMetricsBuilderState_init(&opq->state);
	opq->retainCount	= 1;
}

void NBTextMetricsBuilder_initWithOther(STNBTextMetricsBuilder* obj, const STNBTextMetricsBuilder* other){
	STNBTextMetricsBuilderOpq* opq = obj->opaque = NBMemory_allocType(STNBTextMetricsBuilderOpq);
	STNBTextMetricsBuilderOpq* opq2 = (STNBTextMetricsBuilderOpq*)other->opaque;
	NBMemory_setZeroSt(*opq, STNBTextMetricsBuilderOpq);
	NBArray_initWithOther(&opq->fmtStack, &opq2->fmtStack);
	NBArray_initWithOther(&opq->fmtRanges, &opq2->fmtRanges);
	opq->bytesSynced	= opq2->bytesSynced;
	opq->bytesAdded		= opq2->bytesAdded;
	NBTextMetricsBuilderState_init(&opq->state);
	opq->retainCount	= 1;
	//Retain fonts
	{
		UI32 i; for(i = 0; i < opq->fmtRanges.use; i++){
			STNBTextMetricsBuilderFmtRng* rng = NBArray_itmPtrAtIndex(&opq->fmtRanges, STNBTextMetricsBuilderFmtRng, i);
			if(rng->format.itf.retain != NULL){
				(*rng->format.itf.retain)(rng->format.itfParam);
			}
		}
	}
}

void NBTextMetricsBuilder_retain(STNBTextMetricsBuilder* obj){
	STNBTextMetricsBuilderOpq* opq = (STNBTextMetricsBuilderOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount++;
}

void NBTextMetricsBuilder_release(STNBTextMetricsBuilder* obj){
	STNBTextMetricsBuilderOpq* opq = (STNBTextMetricsBuilderOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount--;
	if(opq->retainCount == 0){
		NBTextMetricsBuilder_empty(obj);
		NBTextMetricsBuilderState_release(&opq->state);
		NBArray_release(&opq->fmtStack);
		NBArray_release(&opq->fmtRanges);
		opq->bytesSynced	= 0;
		opq->bytesAdded		= 0;
		NBMemory_free(obj->opaque);
		obj->opaque = NULL;
	}
}

//Sync

void NBTextMetricsBuilder_syncDataWithOther(STNBTextMetricsBuilder* obj, const STNBTextMetricsBuilder* other){
	if(obj != other){
		STNBTextMetricsBuilderOpq* opq = (STNBTextMetricsBuilderOpq*)obj->opaque;
		const STNBTextMetricsBuilderOpq* opq2 = (const STNBTextMetricsBuilderOpq*)other->opaque;
		//Retain fonts (before releasing)
		{
			UI32 i; for(i = 0; i < opq2->fmtRanges.use; i++){
				STNBTextMetricsBuilderFmtRng* rng = NBArray_itmPtrAtIndex(&opq2->fmtRanges, STNBTextMetricsBuilderFmtRng, i);
				if(rng->format.itf.retain != NULL){
					(*rng->format.itf.retain)(rng->format.itfParam);
				}
			}
		}
		//Release fonts and formats
		NBTextMetricsBuilder_empty(obj);
		//State
		NBTextMetricsBuilderState_reset(&opq->state);
		//Formats
		NBArray_empty(&opq->fmtStack);
		NBArray_empty(&opq->fmtRanges);
		NBArray_addItems(&opq->fmtStack, NBArray_data(&opq2->fmtStack), sizeof(UI32), opq2->fmtStack.use);
		NBArray_addItems(&opq->fmtRanges, NBArray_data(&opq2->fmtRanges), sizeof(STNBTextMetricsBuilderFmtRng), opq2->fmtRanges.use);
		//
		opq->bytesSynced	= opq2->bytesSynced;
		opq->bytesAdded		= opq2->bytesAdded;
	}
}

//State

void NBTextMetricsBuilderState_init(STNBTextMetricsBuilderState* obj){
	NBMemory_setZeroSt(*obj, STNBTextMetricsBuilderState);
	NBArray_init(&obj->fmtStack, sizeof(UI32), NULL);
	NBArray_init(&obj->curLineSpaces.idxs, sizeof(UI32), NULL);
}

void NBTextMetricsBuilderState_release(STNBTextMetricsBuilderState* obj){
	NBArray_release(&obj->fmtStack);
	NBArray_release(&obj->curLineSpaces.idxs);
}

void NBTextMetricsBuilderState_reset(STNBTextMetricsBuilderState* obj){
	//Backup arrays
	const STNBArray fmtStack = obj->fmtStack;
	const STNBArray curLineSpacesIdxs = obj->curLineSpaces.idxs;
	//Set zero
	NBMemory_setZeroSt(*obj, STNBTextMetricsBuilderState);
	//Restore arays and empty
	obj->fmtStack			= fmtStack;
	obj->curLineSpaces.idxs	= curLineSpacesIdxs;
	NBArray_empty(&obj->fmtStack);
	NBArray_empty(&obj->curLineSpaces.idxs);
}


//Format stack

void NBTextMetricsBuilder_pushFormat(STNBTextMetricsBuilder* obj){
	STNBTextMetricsBuilderOpq* opq = (STNBTextMetricsBuilderOpq*)obj->opaque;
	if(opq->fmtStack.use > 0){
		const UI32 idx	= NBArray_itmValueAtIndex(&opq->fmtStack, UI32, opq->fmtStack.use - 1);
		STNBTextMetricsBuilderFmtRng* last = NBArray_itmPtrAtIndex(&opq->fmtRanges, STNBTextMetricsBuilderFmtRng, idx);
		//Clone prev rang and set value
		STNBTextMetricsBuilderFmtRng cpy = *last;
		cpy.rng.start		= opq->bytesAdded;
		cpy.rng.afterEnd	= cpy.rng.start;
		if(cpy.format.itf.retain != NULL){
			(*cpy.format.itf.retain)(cpy.format.itfParam);
		}
		NBArray_addValue(&opq->fmtRanges, cpy);
	} else {
		//Create first rng
		STNBTextMetricsBuilderFmtRng rng;
		NBMemory_setZeroSt(rng, STNBTextMetricsBuilderFmtRng);
		rng.format.color.r	= rng.format.color.g = rng.format.color.b = rng.format.color.a = 255;
		rng.rng.start		= opq->bytesAdded;
		rng.rng.afterEnd	= rng.rng.start;
		NBArray_addValue(&opq->fmtRanges, rng);
	}
	const UI32 idx = opq->fmtRanges.use - 1;
	NBArray_addValue(&opq->fmtStack, idx);
}

void NBTextMetricsBuilder_pushFormatWithFont(STNBTextMetricsBuilder* obj, const STNBFontMetricsIRef fontItfRef, const float defFontSz, const STNBColor8 color, const ENNBTextLineAlignH lineAlignH, const ENNBTextCharAlignV charAlignV){
	NBTextMetricsBuilder_pushFormat(obj);
	NBTextMetricsBuilder_setFormatAndFontItf(obj, &fontItfRef.itf, fontItfRef.itfParam, (fontItfRef.fontSz > 0 ? fontItfRef.fontSz : defFontSz), color, lineAlignH, charAlignV);
}

void NBTextMetricsBuilder_pushFormatWithFontItf(STNBTextMetricsBuilder* obj, const STNBFontMetricsI* itf, void* itfParam, const float fontSz, const STNBColor8 color, const ENNBTextLineAlignH lineAlignH, const ENNBTextCharAlignV charAlignV){
	NBTextMetricsBuilder_pushFormat(obj);
	NBTextMetricsBuilder_setFormatAndFontItf(obj, itf, itfParam, fontSz, color, lineAlignH, charAlignV);
}

void NBTextMetricsBuilder_popFormat(STNBTextMetricsBuilder* obj){
	STNBTextMetricsBuilderOpq* opq = (STNBTextMetricsBuilderOpq*)obj->opaque;
	if(opq->fmtStack.use > 1){ //The root-format-rng cannot be popped
		const UI32 lastIdx = NBArray_itmValueAtIndex(&opq->fmtStack, UI32, opq->fmtStack.use - 1);
		STNBTextMetricsBuilderFmtRng* rng = NBArray_itmPtrAtIndex(&opq->fmtRanges, STNBTextMetricsBuilderFmtRng, lastIdx);
		rng->rng.afterEnd = opq->bytesAdded;
		NBArray_removeItemAtIndex(&opq->fmtStack, opq->fmtStack.use - 1);
	}
}

void NBTextMetricsBuilder_empty(STNBTextMetricsBuilder* obj){
	STNBTextMetricsBuilderOpq* opq = (STNBTextMetricsBuilderOpq*)obj->opaque;
	//Release fonts
	UI32 i; for(i = 0; i < opq->fmtRanges.use; i++){
		STNBTextMetricsBuilderFmtRng* rng = NBArray_itmPtrAtIndex(&opq->fmtRanges, STNBTextMetricsBuilderFmtRng, i);
		if(rng->format.itf.release != NULL){
			(*rng->format.itf.release)(rng->format.itfParam);
		}
	}
	//Empty formats and string
	NBArray_empty(&opq->fmtRanges);
	NBArray_empty(&opq->fmtStack);
	opq->bytesSynced	= 0;
	opq->bytesAdded		= 0;
}

void NBTextMetricsBuilder_syncOpenFormatRangesOpq(STNBTextMetricsBuilderOpq* opq){
	NBASSERT(opq->bytesSynced <= opq->bytesAdded)
	if(opq->bytesSynced != opq->bytesAdded){
		SI32 i; for(i = (SI32)opq->fmtStack.use - 1; i >= 0; i--){
			const UI32 idx = NBArray_itmValueAtIndex(&opq->fmtStack, UI32, i);
			STNBTextMetricsBuilderFmtRng* rng = NBArray_itmPtrAtIndex(&opq->fmtRanges, STNBTextMetricsBuilderFmtRng, idx);
			rng->rng.afterEnd = opq->bytesAdded;
		}
		//PRINTF_INFO("Synced %d ranges still-open at the stack-of-formats.\n", opq->fmtStack.use);
		opq->bytesSynced = opq->bytesAdded;
	}
}

void NBTextMetricsBuilder_syncOpenFormatRanges(STNBTextMetricsBuilder* obj){
	STNBTextMetricsBuilderOpq* opq = (STNBTextMetricsBuilderOpq*)obj->opaque;
	NBTextMetricsBuilder_syncOpenFormatRangesOpq(opq);
}

UI32 NBTextMetricsBuilder_getActiveStackSz(const STNBTextMetricsBuilder* obj){
	const STNBTextMetricsBuilderOpq* opq = (const STNBTextMetricsBuilderOpq*)obj->opaque;
	return opq->fmtStack.use;
}

UI32 NBTextMetricsBuilder_getFormatsCount(const STNBTextMetricsBuilder* obj){
	const STNBTextMetricsBuilderOpq* opq = (const STNBTextMetricsBuilderOpq*)obj->opaque;
	return opq->fmtRanges.use;
}

//Active format

void NBTextMetricsBuilder_setFormatAndFont(STNBTextMetricsBuilder* obj, const STNBFontMetricsIRef fontItfRef, const float defFontSz, const STNBColor8 color, const ENNBTextLineAlignH lineAlignH, const ENNBTextCharAlignV charAlignV){
	NBTextMetricsBuilder_setFormatAndFontItf(obj, &fontItfRef.itf, fontItfRef.itfParam, (fontItfRef.fontSz > 0 ? fontItfRef.fontSz : defFontSz), color, lineAlignH, charAlignV);
}
	
void NBTextMetricsBuilder_setFormatAndFontItf(STNBTextMetricsBuilder* obj, const STNBFontMetricsI* itf, void* itfParam, const float fontSz, const STNBColor8 color, const ENNBTextLineAlignH lineAlignH, const ENNBTextCharAlignV charAlignV){
	STNBTextMetricsBuilderOpq* opq = (STNBTextMetricsBuilderOpq*)obj->opaque;
	//Add first format
	if(opq->fmtStack.use == 0){
		NBTextMetricsBuilder_pushFormat(obj);
	}
	//Set
	{
		const UI32 lastIdx = NBArray_itmValueAtIndex(&opq->fmtStack, UI32, opq->fmtStack.use - 1);
		STNBTextMetricsBuilderFmtRng* rng = NBArray_itmPtrAtIndex(&opq->fmtRanges, STNBTextMetricsBuilderFmtRng, lastIdx);
		rng->format.fontSz		= fontSz;
		rng->format.color		= color;
		rng->format.lineAlignH	= lineAlignH;
		rng->format.charAlignV	= charAlignV;
		if(itf != NULL){
			//Retain new (first)
			if(itf->retain != NULL){
				(*itf->retain)(itfParam);
			}
			//Release current (then)
			if(rng->format.itf.release != NULL){
				(*rng->format.itf.release)(rng->format.itfParam);
			}
			//Set
			rng->format.itf = *itf;
		} else {
			//Release current
			if(rng->format.itf.release != NULL){
				(*rng->format.itf.release)(rng->format.itfParam);
			}
			NBMemory_setZeroSt(rng->format.itf, STNBFontMetricsI);
		}
		rng->format.itfParam = itfParam;
	}
}

void NBTextMetricsBuilder_setFormat(STNBTextMetricsBuilder* obj, const STNBColor8 color, const ENNBTextLineAlignH lineAlignH, const ENNBTextCharAlignV charAlignV){
	STNBTextMetricsBuilderOpq* opq = (STNBTextMetricsBuilderOpq*)obj->opaque;
	//Add first format
	if(opq->fmtStack.use == 0){
		NBTextMetricsBuilder_pushFormat(obj);
	}
	//Set
	{
		const UI32 lastIdx = NBArray_itmValueAtIndex(&opq->fmtStack, UI32, opq->fmtStack.use - 1);
		STNBTextMetricsBuilderFmtRng* rng = NBArray_itmPtrAtIndex(&opq->fmtRanges, STNBTextMetricsBuilderFmtRng, lastIdx);
		rng->format.color		= color;
		rng->format.lineAlignH	= lineAlignH;
		rng->format.charAlignV	= charAlignV;
	}
}

void NBTextMetricsBuilder_setFontSz(STNBTextMetricsBuilder* obj, const float fontSz){
	STNBTextMetricsBuilderOpq* opq = (STNBTextMetricsBuilderOpq*)obj->opaque;
	//Add first format
	if(opq->fmtStack.use == 0){
		NBTextMetricsBuilder_pushFormat(obj);
	}
	//Set
	{
		const UI32 lastIdx = NBArray_itmValueAtIndex(&opq->fmtStack, UI32, opq->fmtStack.use - 1);
		STNBTextMetricsBuilderFmtRng* rng = NBArray_itmPtrAtIndex(&opq->fmtRanges, STNBTextMetricsBuilderFmtRng, lastIdx);
		rng->format.fontSz = fontSz;
	}
}

void NBTextMetricsBuilder_setColor(STNBTextMetricsBuilder* obj, const STNBColor8 color){
	STNBTextMetricsBuilderOpq* opq = (STNBTextMetricsBuilderOpq*)obj->opaque;
	//Add first format
	if(opq->fmtStack.use == 0){
		NBTextMetricsBuilder_pushFormat(obj);
	}
	//Set
	{
		const UI32 lastIdx = NBArray_itmValueAtIndex(&opq->fmtStack, UI32, opq->fmtStack.use - 1);
		STNBTextMetricsBuilderFmtRng* rng = NBArray_itmPtrAtIndex(&opq->fmtRanges, STNBTextMetricsBuilderFmtRng, lastIdx);
		rng->format.color = color;
	}
}

void NBTextMetricsBuilder_setLineAlign(STNBTextMetricsBuilder* obj, const ENNBTextLineAlignH align){
	STNBTextMetricsBuilderOpq* opq = (STNBTextMetricsBuilderOpq*)obj->opaque;
	//Add first format
	if(opq->fmtStack.use == 0){
		NBTextMetricsBuilder_pushFormat(obj);
	}
	//Set
	{
		const UI32 lastIdx = NBArray_itmValueAtIndex(&opq->fmtStack, UI32, opq->fmtStack.use - 1);
		STNBTextMetricsBuilderFmtRng* rng = NBArray_itmPtrAtIndex(&opq->fmtRanges, STNBTextMetricsBuilderFmtRng, lastIdx);
		rng->format.lineAlignH = align;
	}
}

void NBTextMetricsBuilder_setCharAlign(STNBTextMetricsBuilder* obj, const ENNBTextCharAlignV align){
	STNBTextMetricsBuilderOpq* opq = (STNBTextMetricsBuilderOpq*)obj->opaque;
	//Add first format
	if(opq->fmtStack.use == 0){
		NBTextMetricsBuilder_pushFormat(obj);
	}
	//Set
	{
		const UI32 lastIdx = NBArray_itmValueAtIndex(&opq->fmtStack, UI32, opq->fmtStack.use - 1);
		STNBTextMetricsBuilderFmtRng* rng = NBArray_itmPtrAtIndex(&opq->fmtRanges, STNBTextMetricsBuilderFmtRng, lastIdx);
		rng->format.charAlignV = align;
	}
}

void NBTextMetricsBuilder_setFont(STNBTextMetricsBuilder* obj, const STNBFontMetricsIRef fontItfRef){
	NBTextMetricsBuilder_setFontItf(obj, &fontItfRef.itf, fontItfRef.itfParam);
	if(fontItfRef.fontSz > 0){
		NBTextMetricsBuilder_setFontSz(obj, fontItfRef.fontSz);
	}
}

void NBTextMetricsBuilder_setFontItf(STNBTextMetricsBuilder* obj, const STNBFontMetricsI* itf, void* itfParam){
	STNBTextMetricsBuilderOpq* opq = (STNBTextMetricsBuilderOpq*)obj->opaque;
	//Add first format
	if(opq->fmtStack.use == 0){
		NBTextMetricsBuilder_pushFormat(obj);
	}
	//Set
	{
		const UI32 lastIdx = NBArray_itmValueAtIndex(&opq->fmtStack, UI32, opq->fmtStack.use - 1);
		STNBTextMetricsBuilderFmtRng* rng = NBArray_itmPtrAtIndex(&opq->fmtRanges, STNBTextMetricsBuilderFmtRng, lastIdx);
		if(itf != NULL){
			//Retain new (first)
			if(itf->retain != NULL){
				(*itf->retain)(itfParam);
			}
			//Release current (then)
			if(rng->format.itf.release != NULL){
				(*rng->format.itf.release)(rng->format.itfParam);
			}
			//Set
			rng->format.itf = *itf;
		} else {
			//Release current
			if(rng->format.itf.release != NULL){
				(*rng->format.itf.release)(rng->format.itfParam);
			}
			NBMemory_setZeroSt(rng->format.itf, STNBFontMetricsI);
		}
		rng->format.itfParam = itfParam;
	}
}

//Text

UI32 NBTextMetricsBuilder_getBytesLen(const STNBTextMetricsBuilder* obj){
	STNBTextMetricsBuilderOpq* opq = (STNBTextMetricsBuilderOpq*)obj->opaque;
	return opq->bytesAdded;
}

void NBTextMetricsBuilder_appendBytes(STNBTextMetricsBuilder* obj, const char* str, const UI32 strSz){
	STNBTextMetricsBuilderOpq* opq = (STNBTextMetricsBuilderOpq*)obj->opaque;
	opq->bytesAdded += strSz;
}

void NBTextMetricsBuilder_removeBytes(STNBTextMetricsBuilder* obj, const UI32 start, const UI32 pBytesLen){
	NBTextMetricsBuilder_replaceBytes(obj, start, pBytesLen, NULL, 0);
}

void NBTextMetricsBuilder_replaceBytes(STNBTextMetricsBuilder* obj, const UI32 pStart, const UI32 pBytesLen, const char* newStr, const UI32 newStrSz){
	STNBTextMetricsBuilderOpq* opq = (STNBTextMetricsBuilderOpq*)obj->opaque;
	UI32 start = pStart, bytesLen = pBytesLen;
	//Validate-range
	{
		if(start > opq->bytesAdded){
			start = opq->bytesAdded;
		}
		if((start + bytesLen) > opq->bytesAdded){
			NBASSERT(start <= opq->bytesAdded)
			bytesLen = opq->bytesAdded - start;
		}
	}
	//Adjust
	if(opq->fmtRanges.use > 0){
		const UI32 rngAfterEnd = (start + bytesLen);
		SI32 i = (opq->fmtRanges.use - 1);
		if(start == opq->bytesAdded){
			//Special case: adding at the end (stretch all ranges that end at right limit)
			STNBTextMetricsBuilderFmtRng* rng = NULL;
			while(i >= 0){
				rng = NBArray_itmPtrAtIndex(&opq->fmtRanges, STNBTextMetricsBuilderFmtRng, i);
				if(rng->rng.afterEnd == rngAfterEnd){
					rng->rng.afterEnd = start + newStrSz;
				}
				i--;
			}
		} else {
			//Remove or truncate ranges
			BOOL remove = FALSE;
			STNBTextMetricsBuilderFmtRng* rng = NULL;
			while(i >= 0){
				rng = NBArray_itmPtrAtIndex(&opq->fmtRanges, STNBTextMetricsBuilderFmtRng, i);
				if(rng->rng.afterEnd <= start){
					//This range is not affected
				} else if(rng->rng.start <= start && start < rng->rng.afterEnd){
					//The new content will be added to this line
					if(rng->rng.afterEnd <= rngAfterEnd){
						//The line ended inside the replaced range
						rng->rng.afterEnd = start + newStrSz;
					} else {
						//The line continued after the replaced range
						rng->rng.afterEnd = rng->rng.afterEnd + newStrSz - bytesLen;
					}
					NBASSERT(rng->rng.start <= rng->rng.afterEnd)
					if(rng->rng.afterEnd <= rng->rng.start){
						//Range resulted empty, remove-it
						remove = TRUE;
					}
				} else {
					//This line is after the replacement start (is moved or tunked)
					if(rngAfterEnd <= rng->rng.start){
						//Move delta-words to the right
						rng->rng.start = rng->rng.start + newStrSz - bytesLen;
					} else {
						//Move to resulting right limit
						rng->rng.start = start + newStrSz;
					}
					if(rngAfterEnd < rng->rng.afterEnd){
						//Move delta-words to the right
						rng->rng.afterEnd = rng->rng.afterEnd + newStrSz - bytesLen;
					} else {
						//Move to resulting right limit
						rng->rng.afterEnd = start + newStrSz;
					}
					NBASSERT(rng->rng.start <= rng->rng.afterEnd)
					if(rng->rng.afterEnd <= rng->rng.start){
						//Range resulted empty, remove-it
						remove = TRUE;
					}
				}
				//Remove range
				if(remove && i >0){ //Do not remove root-range
					NBArray_removeItemAtIndex(&opq->fmtRanges, i);
					//Update format stack
					if(opq->fmtStack.use > 0){
						SI32 j; for(j = (opq->fmtStack.use - 1); j >= 0; j--){
							UI32* idx = NBArray_itmPtrAtIndex(&opq->fmtStack, UI32, j);
							if(i == *idx){
								//Remove from stack
								NBArray_removeItemAtIndex(&opq->fmtStack, j);
							} else if(i < *idx){
								//Update rng-index
								*idx = *idx - 1;
							} else {
								//Stop cicle
								break;
							}
						}
					}
					//Reset flag
					remove = FALSE;
				}
				//Next range
				i--;
			}
		}
	}
	//
	opq->bytesAdded 	= opq->bytesAdded + newStrSz - bytesLen;
	opq->bytesSynced	= 0;
}

//Metrics calculation

void NBTextMetricsBuilder_feedEndLineOpq(STNBTextMetricsBuilderOpq* opq, STNBTextMetrics* dst, const STNBRect column, const BOOL isExplicitLineEnd, const BOOL isLastLine){
	//PRINTF_INFO("Ending line.\n");
	//End current line
	STNBTextMetricsBuilderState* s		= &opq->state;
	STNBTextMetricsBuilderLine* curLine	= &s->curLine;
	if(curLine->ln.rngWords.start >= curLine->ln.rngWords.afterEnd){
		//Empty line
		//Add if was explicit
		if(curLine->isExplicit){
			curLine->ln.fontMetricsMax = s->curFontMetrics;
			curLine->ln.yBase = s->curPos.y + curLine->ln.fontMetricsMax.ascender;
			{
				curLine->ln.lBearing = 0.0f;
				curLine->ln.rBearing = 0.0f;
				switch(curLine->ln.alignH){
					case ENNBTextLineAlignH_Right:
						curLine->ln.visibleLeft = curLine->ln.visibleRight = column.x + column.width;
						break;
					case ENNBTextLineAlignH_Center:
						curLine->ln.visibleLeft = curLine->ln.visibleRight = column.x + (column.width / 2.0f);
						break;
					default:
						//ENNBTextLineAlignH_Base
						//ENNBTextLineAlignH_Left
						//ENNBTextLineAlignH_Adjust
						curLine->ln.visibleLeft = curLine->ln.visibleRight = column.x;
						break;
				}
			}
			NBArray_addValue(&dst->lines, curLine->ln);
		}
		//Move
		s->curPos.y += s->curFontMetrics.height;
	} else {
		//Set visible range (remove bearings)
		curLine->ln.visibleLeft		+= curLine->ln.lBearing;
		curLine->ln.visibleRight	-= curLine->ln.rBearing;
		NBASSERT(curLine->ln.visibleLeft <= curLine->ln.visibleRight)
		//H-align line
		float deltaX = 0.0f;
		switch(curLine->ln.alignH){
			case ENNBTextLineAlignH_Left:
				deltaX = column.x - curLine->ln.lBearing;
				break;
			case ENNBTextLineAlignH_Right:
				deltaX = ((column.x + column.width) - curLine->ln.visibleRight);
				break;
			case ENNBTextLineAlignH_Center:
				deltaX = column.x - curLine->ln.visibleLeft + ((column.width - (curLine->ln.visibleRight - curLine->ln.visibleLeft)) * 0.5f);
				break;
			case ENNBTextLineAlignH_Adjust:
				NBASSERT((curLine->visible.left.separators.count + curLine->visible.right.separators.count) <= s->curLineSpaces.idxs.use)
				if(curLine->visible.words == 0){
					//No visible words
					deltaX = column.x - curLine->ln.lBearing; //ENNBTextLineAlignH_Left
				} else if(s->curLineSpaces.idxs.use <= 0 || s->curLineSpaces.width <= 0){
					//No spaces to adjust
					deltaX = column.x - curLine->ln.lBearing; //ENNBTextLineAlignH_Left
				} else if((curLine->visible.left.separators.count + curLine->visible.right.separators.count) == s->curLineSpaces.idxs.use){
					//No internal spaces to adjust
					deltaX = column.x - curLine->ln.lBearing; //ENNBTextLineAlignH_Left
				} else if(isExplicitLineEnd || isLastLine){
					//Explicit lines-change and last-line must not be adjusted.
					deltaX = column.x - curLine->ln.lBearing; //ENNBTextLineAlignH_Left
				} else {
					//Adjust words by expanding spaces
					BOOL lastWasInternalSpace	= FALSE;
					float extraWidthRemain		= (column.width - (curLine->ln.visibleRight - curLine->ln.visibleLeft));
					SI32 spacesRemain			= (s->curLineSpaces.idxs.use - curLine->visible.left.separators.count - curLine->visible.right.separators.count); NBASSERT(spacesRemain > 0)
					float deltaX				= 0.0f;
					//PRINTF_INFO("extraWidthRemain(%f) in %d spaces (START).\n", extraWidthRemain, spacesRemain);
					UI32 iSpace = curLine->visible.left.separators.count, nxtSpace = NBArray_itmValueAtIndex(&s->curLineSpaces.idxs, UI32, iSpace);
					UI32 i; for(i = curLine->ln.rngWords.start; i < curLine->ln.rngWords.afterEnd; i++){
						STNBTextMetricsWord* w = NBArray_itmPtrAtIndex(&dst->words, STNBTextMetricsWord, i);
						if(i == nxtSpace){
							const float thisExtra = (extraWidthRemain / spacesRemain);
							UI32 i; for(i = w->rngChars.start; i < w->rngChars.afterEnd; i++){
								STNBTextMetricsChar* c = NBArray_itmPtrAtIndex(&dst->chars, STNBTextMetricsChar, i);
								c->pos.x += deltaX;
							}
							deltaX				+= thisExtra;
							extraWidthRemain	-= thisExtra;
							spacesRemain--;
							//Next space
							iSpace++;
							if(iSpace < (s->curLineSpaces.idxs.use - curLine->visible.right.separators.count)){
								nxtSpace = NBArray_itmValueAtIndex(&s->curLineSpaces.idxs, UI32, iSpace);
							}
							lastWasInternalSpace = TRUE;
						} else if(deltaX != 0.0f){
							UI32 i; for(i = w->rngChars.start; i < w->rngChars.afterEnd; i++){
								STNBTextMetricsChar* c = NBArray_itmPtrAtIndex(&dst->chars, STNBTextMetricsChar, i);
								c->pos.x += deltaX;
							}
							lastWasInternalSpace = FALSE;
						}
					}
					//Add total-inflation to line
					curLine->ln.visibleRight += deltaX;
					NBASSERT(!lastWasInternalSpace)
					//PRINTF_INFO("extraWidthRemain(%f) in %f spaces (END).\n", extraWidthRemain, spacesRemain);
				}
				break;
			default: //ENNBTextLineAlignH_Base
				deltaX = column.x;
				break;
		}
		if(deltaX != 0.0f){
			NBASSERT(deltaX != -0.0f)
			NBASSERT(curLine->ln.rngWords.start < curLine->ln.rngWords.afterEnd)
			NBASSERT(curLine->ln.rngWords.start < dst->chars.use && curLine->ln.rngWords.afterEnd <= dst->chars.use)
			const STNBTextMetricsWord* fW = NBArray_itmPtrAtIndex(&dst->words, STNBTextMetricsWord, curLine->ln.rngWords.start);
			const STNBTextMetricsWord* lW = NBArray_itmPtrAtIndex(&dst->words, STNBTextMetricsWord, curLine->ln.rngWords.afterEnd - 1);
			UI32 i; for(i = fW->rngChars.start; i < lW->rngChars.afterEnd; i++){
				STNBTextMetricsChar* c = NBArray_itmPtrAtIndex(&dst->chars, STNBTextMetricsChar, i);
				c->pos.x += deltaX;
			}
			curLine->ln.visibleLeft		+= deltaX;
			curLine->ln.visibleRight	+= deltaX;
		}
		//Add line
		curLine->ln.yBase	= s->curPos.y + curLine->ln.fontMetricsMax.ascender;
		NBArray_addValue(&dst->lines, curLine->ln);
		s->curPos.y += curLine->ln.fontMetricsMax.height;
	}
	s->curPos.x = column.x;
	//Set empty-line
	curLine->ln.rngWords.start = curLine->ln.rngWords.afterEnd = dst->words.use;
	curLine->isExplicit = isExplicitLineEnd;
	NBArray_empty(&s->curLineSpaces.idxs); //UI32
}

void NBTextMetricsBuilder_feedEndWordOpq(STNBTextMetricsBuilderOpq* opq, STNBTextMetrics* dst){
	//PRINTF_INFO("Ending word.\n");
	STNBTextMetricsBuilderState* s		= &opq->state;
	STNBTextMetricsBuilderWord* curWord	= &s->curWord;
	if(curWord->wd.rngChars.start < curWord->wd.rngChars.afterEnd){
		STNBTextMetricsBuilderLine* curLine	= &s->curLine;
		//Word is not empty
		if(curLine->ln.rngWords.start < curLine->ln.rngWords.afterEnd){
			//Continue line
			//Add word to line
			curLine->ln.rngWords.afterEnd	= dst->words.use + 1; NBASSERT(curLine->ln.rngWords.start < curLine->ln.rngWords.afterEnd)
			curLine->ln.rBearing			= curWord->rBearing;
			if(curLine->ln.fontMetricsMax.emBoxSz < curWord->fontMetricsMax.emBoxSz){
				curLine->ln.fontMetricsMax.emBoxSz = curWord->fontMetricsMax.emBoxSz;
			}
			if(curLine->ln.fontMetricsMax.ascender < curWord->fontMetricsMax.ascender){
				UI32 i, j; const float delta = (curWord->fontMetricsMax.ascender - curLine->ln.fontMetricsMax.ascender); NBASSERT(delta > 0)
				for(i = curLine->ln.rngWords.start; i < dst->words.use; i++){
					STNBTextMetricsWord* w = NBArray_itmPtrAtIndex(&dst->words, STNBTextMetricsWord, i);
					for(j = w->rngChars.start; j < w->rngChars.afterEnd; j++){
						STNBTextMetricsChar* c = NBArray_itmPtrAtIndex(&dst->chars, STNBTextMetricsChar, j);
						c->pos.y += delta;
					}
				}
				curLine->ln.yBase += delta;
				curLine->ln.fontMetricsMax.ascender = curWord->fontMetricsMax.ascender;
			}
			if(curLine->ln.fontMetricsMax.descender > curWord->fontMetricsMax.descender){ //is negative
				curLine->ln.fontMetricsMax.descender = curWord->fontMetricsMax.descender;
			}
			if(curLine->ln.fontMetricsMax.height < curWord->fontMetricsMax.height){
				curLine->ln.fontMetricsMax.height = curWord->fontMetricsMax.height;
			}
		} else {
			//Start line
			curLine->iByte					= curWord->iByte;
			curLine->curX					= 0.0f;
			curLine->ln.rngWords.start		= dst->words.use;
			curLine->ln.rngWords.afterEnd	= dst->words.use + 1;
			curLine->ln.yBase				= s->curPos.y + curWord->fontMetricsMax.ascender;
			curLine->ln.lBearing			= curWord->lBearing;
			curLine->ln.rBearing			= curWord->rBearing;
			curLine->ln.visibleLeft			= 0.0f;
			curLine->ln.visibleRight		= 0.0f;
			curLine->ln.alignH				= curWord->alignH;
			curLine->ln.fontMetricsMax		= curWord->fontMetricsMax;
			//Visible area
			NBMemory_setZero(curLine->visible);
			//Spaces
			NBArray_empty(&s->curLineSpaces.idxs);
			s->curLineSpaces.width			= 0;
		}
		//Acumulate internal spaces (for adjusted-alignment)
		if(curWord->wd.isSpace){
			const UI32 wIdx = dst->words.use;
			s->curLineSpaces.width += (curWord->curX - curWord->lBearing - curWord->rBearing);
			NBArray_addValue(&s->curLineSpaces.idxs, wIdx);
			if(curLine->visible.words == 0){
				//Move left visible
				curLine->visible.left.separators.count++;
				curLine->visible.left.separators.width += curWord->curX;
				curLine->visible.left.xPos = curLine->curX + curWord->curX;
			} else {
				curLine->visible.right.separators.count++;
				curLine->visible.right.separators.width += curWord->curX;
			}
		} else {
			if(curLine->visible.words == 0){
				//This will be the first visible
				curLine->visible.left.xPos			= curLine->curX;
				curLine->visible.left.lBearing		= curWord->lBearing;
			}
			//Disable right spaces
			curLine->visible.right.xPos				= curLine->curX + curWord->curX;
			curLine->visible.right.rBearing			= curWord->rBearing;
			curLine->visible.right.separators.count = 0;
			curLine->visible.right.separators.width = 0.0f;
			//Count visible words
			curLine->visible.words++;
		}
		//Adjust current word chars coords
		{
			NBASSERT(curWord->fontMetricsMax.ascender <= curLine->ln.fontMetricsMax.ascender)
			const float delta = (curLine->ln.fontMetricsMax.ascender - curWord->fontMetricsMax.ascender); NBASSERT(delta >= 0)
			UI32 i; for(i = curWord->wd.rngChars.start; i < curWord->wd.rngChars.afterEnd; i++){
				STNBTextMetricsChar* c = NBArray_itmPtrAtIndex(&dst->chars, STNBTextMetricsChar, i);
				c->pos.x += curLine->curX;
				c->pos.y += s->curPos.y + delta;
			}
		}
		//Add word to array
		/*{
		 STNBString str;
		 NBString_init(&str);
		 NBString_concatBytes(&str, &opq->str.str[curWord->iByte], iByte - curWord->iByte + (iByte == curWord->iByte ? charSz : 0));
		 PRINTF_INFO("Adding word to line: '%s'.\n", str.str);
		 NBString_release(&str);
		 }*/
		NBArray_addValue(&dst->words, curWord->wd);
		//Advance-x
		curLine->curX += curWord->curX;
		curLine->ln.visibleRight += curWord->curX;
		//Clear word
		curWord->wd.rngChars.start = curWord->wd.rngChars.afterEnd = dst->chars.use;
		//PRINTF_INFO("Word added to line.\n");
	}
}

void NBTextMetricsBuilder_feedUnicodeAccumCharOpq(STNBTextMetricsBuilderOpq* opq, STNBTextMetrics* dst, const UI32 unicode, const UI32 iByte, const UI32 charSz, const BOOL isControlChar, const BOOL isSpace){
	STNBTextMetricsBuilderState* s		= &opq->state;
	STNBTextMetricsBuilderWord* curWord	= &s->curWord;
	BOOL metricsLoaded = FALSE;
	UI32 shapeId = 0;
	STNBFontShapeMetrics metrics;
	NBMemory_setZeroSt(metrics, STNBFontShapeMetrics);
	if(s->curFmt != NULL){
		//Note: do-not force a line-change after lineAlignH-changed
		//Force line change after lineAlignH-changed
		/*if(s->curLine.alignH != s->curFmt->lineAlignH && s->curLine.rngWords.start < s->curLine.rngWords.afterEnd){
		 PRINTF_INFO("Forcing line change due to horiz-align change.\n");
		 cAction.endLine = TRUE;
		 }*/
		//Load char metrics
		if(isControlChar){
			shapeId = 0;
			NBMemory_setZeroSt(metrics, STNBFontShapeMetrics);
			metricsLoaded = TRUE;
		} else {
			if(s->curFontSz > 0 && s->curFmt->itf.getShapeId != NULL && s->curFmt->itf.getFontShapeMetricsForSz != NULL){
				shapeId = (*s->curFmt->itf.getShapeId)(s->curFmt->itfParam, unicode);
				metrics = (*s->curFmt->itf.getFontShapeMetricsForSz)(s->curFmt->itfParam, shapeId, s->curFontSz);
				metricsLoaded = TRUE;
				/*if(unicode == '\r' || unicode == '\n'){
				 PRINTF_INFO("Metrics for char(%d) = shapeId(%d):\n", (UI32)unicode, shapeId);
				 PRINTF_INFO("width     = %f\n", metrics.width);
				 PRINTF_INFO("height    = %f\n", metrics.height);
				 PRINTF_INFO("hAdvance  = %f\n", metrics.hAdvance);
				 PRINTF_INFO("vAdvance  = %f\n", metrics.vAdvance);
				 PRINTF_INFO("hBearingX = %f\n", metrics.hBearingX);
				 PRINTF_INFO("hBearingY = %f\n", metrics.hBearingY);
				 PRINTF_INFO("vBearingX = %f\n", metrics.vBearingX);
				 PRINTF_INFO("vBearingY = %f\n", metrics.vBearingY);
				 }*/
			}
		}
	}
	if(!metricsLoaded){
		NBASSERT(FALSE)
	} else {
		//Add char
		if(curWord->wd.rngChars.start >= curWord->wd.rngChars.afterEnd){
			//Start word
			//PRINTF_INFO("Starting new word.\n");
			curWord->curX					= 0.0f;
			curWord->iByte					= iByte;
			curWord->alignH					= s->curFmt->lineAlignH;
			curWord->wd.rngChars.start		= dst->chars.use;
			curWord->wd.rngChars.afterEnd	= dst->chars.use + 1;
			curWord->wd.isControl			= isControlChar;
			curWord->wd.isSpace				= isSpace;
			curWord->lBearing				= (metrics.width == 0 ? 0 : metrics.hBearingX); //special case, spaces have zero-width
			curWord->rBearing				= (metrics.width == 0 ? 0 : (metrics.hAdvance - metrics.width - metrics.hBearingX)); //special case, spaces have zero-width
			curWord->fontMetricsMax			= s->curFontMetrics;
		} else {
			//Continue word
			curWord->wd.rngChars.afterEnd	= dst->chars.use + 1;
			curWord->rBearing				= (metrics.width == 0 ? 0 : (metrics.hAdvance - metrics.width - metrics.hBearingX)); //special case, spaces have zero-width
			//
			if(curWord->fontMetricsMax.emBoxSz < s->curFontMetrics.emBoxSz){
				curWord->fontMetricsMax.emBoxSz = s->curFontMetrics.emBoxSz;
			}
			//Adjust prev-chars yBase
			if(curWord->fontMetricsMax.ascender < s->curFontMetrics.ascender){
				const float delta = (s->curFontMetrics.ascender - curWord->fontMetricsMax.ascender); NBASSERT(delta > 0)
				UI32 i; for(i = curWord->wd.rngChars.start; i < dst->chars.use; i++){
					STNBTextMetricsChar* c = NBArray_itmPtrAtIndex(&dst->chars, STNBTextMetricsChar, i);
					c->pos.y += delta;
				}
				curWord->fontMetricsMax.ascender = s->curFontMetrics.ascender;
			}
			if(curWord->fontMetricsMax.descender > s->curFontMetrics.descender){ //is negative
				curWord->fontMetricsMax.descender = s->curFontMetrics.descender;
			}
			if(curWord->fontMetricsMax.height < s->curFontMetrics.height){
				curWord->fontMetricsMax.height = s->curFontMetrics.height;
			}
		}
		//Add char
		STNBTextMetricsChar c;
		NBMemory_setZeroSt(c, STNBTextMetricsChar);
		c.itfObj		= s->curFmt->itfParam;
		c.iByte			= iByte;
		c.bytesLen		= (UI8)charSz;
		c.vAlign		= s->curFmt->charAlignV;
		c.isSpace		= isSpace;
		c.isControl		= isControlChar;
		c.shapeId		= shapeId;
		c.pos.x			= curWord->curX;
		c.pos.y			= s->curFontMetrics.ascender + (curWord->fontMetricsMax.ascender - s->curFontMetrics.ascender);
		c.extendsLeft	= -metrics.hBearingX;
		c.extendsRight	= (metrics.width > 0 ? (metrics.hBearingX + metrics.width) : metrics.hAdvance);
		c.color			= s->curFmt->color;
		NBArray_addValue(&dst->chars, c);
		//PRINTF_INFO("Added #%d-char: '%c' pos(%f, %f).\n", dst->chars.use, (char)unicode, c.pos.x, c.pos.y);
		//Add Advance
		curWord->curX	+= metrics.hAdvance;
		//PRINTF_INFO("Char('%c') size(%f) advance(%f) hBearingX(%f) width(%f).\n", (char)unicode, s->curFontSz, metrics.hAdvance, metrics.hBearingX, metrics.width);
	}
}

BOOL NBTextMetricsBuilder_feedUnicode(STNBTextMetricsBuilder* obj, STNBTextMetrics* dst, const STNBRect column, const UI32 unicode, const UI32 iByte, const UI32 charSz, const BOOL endFeedStream){
	BOOL r = TRUE;
	STNBTextMetricsBuilderOpq* opq	= (STNBTextMetricsBuilderOpq*)obj->opaque;
	STNBTextMetricsBuilderState* s	= &opq->state; NBASSERT(iByte == s->iByte)
	//Determine char action
	STNBTextMetricsCharAction cAction;
	if(endFeedStream) {
		//Set last action
		NBMemory_setZeroSt(cAction, STNBTextMetricsCharAction);
		cAction.endLine		= TRUE;
		cAction.endWord		= TRUE;
		cAction.isSpace		= TRUE;
		cAction.charMode	= ENTextMetricsCharMode_ignore;
	} else {
		cAction				= NBTextMetrics_getCharAction(unicode, (s->curLine.ln.rngWords.afterEnd - s->curLine.ln.rngWords.start), (s->curWord.wd.rngChars.afterEnd - s->curWord.wd.rngChars.start));
	}
	//Feed word
	if(cAction.endWord){
		STNBTextMetricsBuilderWord* curWord	= &s->curWord;
		STNBTextMetricsBuilderLine* curLine	= &s->curLine;
		if(curWord->wd.rngChars.start < curWord->wd.rngChars.afterEnd){
			//Word is not empty
			if(curLine->ln.rngWords.start == dst->words.use || curWord->wd.isControl || column.width == 0 || (curLine->curX - curLine->ln.lBearing + curWord->curX - curWord->rBearing) <= column.width){
				//It fits: [Line is empty], [Curword is control-char], [Column is empty], [Word fits]
				//PRINTF_INFO("Word fits in the line.\n");
				NBTextMetricsBuilder_feedEndWordOpq(opq, dst);
			} else {
				//Word wont fit in line
				NBASSERT(curLine->ln.rngWords.start < curLine->ln.rngWords.afterEnd) //Line must not be empty
				if(curWord->wd.isSpace){
					//Space-word wont-fit in line
					//curWord->lBearing		= 0;
					//curWord->rBearing		= 0;
					//curWord->visibleRight	= curWord->visibleLeft;
					//Add space-word to current line and change line
					//PRINTF_INFO("Word (space) wont fit on line: adding to cur-line and forcing new-line.\n");
					NBTextMetricsBuilder_feedEndWordOpq(opq, dst);
					NBTextMetricsBuilder_feedEndLineOpq(opq, dst, column, cAction.endLine, (endFeedStream && unicode == '\0'));
				} else {
					//Content-word wont-fit in line
					//Change line and add word to new line
					//PRINTF_INFO("Word (not-space) wont fit on line: forcing new-line and adding word.\n");
					NBTextMetricsBuilder_feedEndLineOpq(opq, dst, column, cAction.endLine, FALSE);
					NBTextMetricsBuilder_feedEndWordOpq(opq, dst);
				}
			}
		}
	}
	//Process char
	if(cAction.charMode > ENTextMetricsCharMode_ignore){
		NBTextMetricsBuilder_feedUnicodeAccumCharOpq(opq, dst, unicode, iByte, charSz, (cAction.charMode == ENTextMetricsCharMode_noVisual ? TRUE : FALSE), cAction.isSpace);
		if(cAction.endWord){
			NBTextMetricsBuilder_feedEndWordOpq(opq, dst);
		}
	}
	//Process line
	if(cAction.endLine){
		NBTextMetricsBuilder_feedEndLineOpq(opq, dst, column, cAction.endLine, (endFeedStream && unicode == '\0'));
	}
	return r;
}
	
BOOL NBTextMetricsBuilder_feedStart(STNBTextMetricsBuilder* obj, STNBTextMetrics* dst){
	STNBTextMetricsBuilderOpq* opq = (STNBTextMetricsBuilderOpq*)obj->opaque;
	//Clear current results
	NBTextMetrics_empty(dst);
	NBTextMetricsBuilderState_reset(&opq->state);
	//Sync-open-ranges
	if(opq->bytesSynced != opq->bytesAdded){
		NBTextMetricsBuilder_syncOpenFormatRangesOpq(opq);
		NBASSERT(opq->bytesSynced == opq->bytesAdded)
	}
	//Load first (and default) format
	{
		STNBTextMetricsBuilderState* s = &opq->state;
		NBASSERT(s->fmtStack.use == 0)
		NBASSERT(s->iByte == 0)
		UI32 iRng = 0;
		while(iRng < opq->fmtRanges.use){
			const STNBTextMetricsBuilderFmtRng* rng	= NBArray_itmPtrAtIndex(&opq->fmtRanges, STNBTextMetricsBuilderFmtRng, iRng);
			if(s->iByte < rng->rng.start){
				//Next-ranges will not cover this-char
				break;
			} else {
				const STNBTextMetricsBuilderFmt* fmt	= &rng->format;
				float fontSz							= fmt->fontSz;
				if(fontSz <= 0 && fmt->itf.getNativeSz != NULL){
					fontSz = (*fmt->itf.getNativeSz)(fmt->itfParam);
				}
				if(fontSz > 0.0f && fmt->itf.getFontMetricsForSz != NULL){
					//Set current range
					s->curRng			= rng;
					s->curFmt			= fmt;
					s->curFontSz		= fontSz;
					s->curFontMetrics	= (*fmt->itf.getFontMetricsForSz)(fmt->itfParam, fontSz);
					NBArray_addValue(&s->fmtStack, iRng);
					s->iNextFmtRng		= iRng + 1;
					break;
				} else {
					//Try next range
					iRng++;
				}
			}
		}
		//PRINTF_INFO("First range will be idx(%d).\n", iRng);
	}
	return TRUE;
}

UI32 NBTextMetricsBuilder_feed(STNBTextMetricsBuilder* obj, STNBTextMetrics* dst, const STNBRect column, const BOOL allowMultiline, const char* text, const UI32 textSz){
	UI32 iByte = 0;
	STNBTextMetricsBuilderOpq* opq = (STNBTextMetricsBuilderOpq*)obj->opaque;
	//Process
	NBASSERT(opq->fmtRanges.use > 0 || opq->bytesAdded == 0) //If fails, there's text with no-range-defined (default font defined?)
	if(opq->fmtRanges.use > 0 && opq->bytesAdded > 0){
		//Load first (and default) format
		STNBTextMetricsBuilderState* s = &opq->state;
		s->curCol = column;
		//Process
		if(s->curFontSz <= 0){
			//Could not determine the size of the default rng/font
			NBASSERT(FALSE)
		} else {
			//Set defaults
			{
				if(s->curFontMetrics.ascender == 0 || s->curFontMetrics.descender == 0 || s->curFontMetrics.emBoxSz == 0 || s->curFontMetrics.height == 0){
					NBASSERT(s->curFontMetrics.ascender != 0 || s->curFontMetrics.descender != 0 || s->curFontMetrics.emBoxSz != 0 || s->curFontMetrics.height != 0)
					NBTextMetrics_setDefaultFontMetrics(dst, &s->curFontMetrics);
				}
				NBTextMetrics_setDefaultStartPos(dst, &s->curPos);
			}
			//Process bytes/chars
			UI32 charSz = 0, unicode = 0;
			while(iByte < textSz){
				NBASSERT(s->iByte < opq->bytesAdded)
				//Detect push of format
				while(s->iNextFmtRng < opq->fmtRanges.use){
					const STNBTextMetricsBuilderFmtRng* rng	= NBArray_itmPtrAtIndex(&opq->fmtRanges, STNBTextMetricsBuilderFmtRng, s->iNextFmtRng);
					if(s->iByte < rng->rng.start){
						break; //stop pushing
					} else {
						//Set current range (push)
						const STNBTextMetricsBuilderFmt* fmt	= &rng->format;
						float fontSz							= fmt->fontSz;
						if(fontSz <= 0 && fmt->itf.getNativeSz != NULL){
							fontSz = (*fmt->itf.getNativeSz)(fmt->itfParam);
						}
						if(fontSz > 0.0f && fmt->itf.getFontMetricsForSz != NULL){
							s->curRng			= rng;
							s->curFmt			= fmt;
							s->curFontSz		= fontSz;
							s->curFontMetrics	= (*fmt->itf.getFontMetricsForSz)(fmt->itfParam, fontSz);
							NBArray_addValue(&s->fmtStack, s->iNextFmtRng);
						}
						//PRINTF_INFO("Pushed format %d (%d -> %d) at char(%d / %d).\n", s->iNextFmtRng, rng->rng.start, rng->rng.afterEnd, s->iByte, opq->bytesAdded);
						s->iNextFmtRng++;
					}
				}
				//Detect pop of format
				while(s->fmtStack.use > 1){
					const UI32 idx = NBArray_itmValueAtIndex(&s->fmtStack, UI32, s->fmtStack.use - 1);
					const STNBTextMetricsBuilderFmtRng* rng	= NBArray_itmPtrAtIndex(&opq->fmtRanges, STNBTextMetricsBuilderFmtRng, idx);
					if(rng->rng.afterEnd > s->iByte){
						break; //stop popping
					} else {
						//Set current range (pop)
						//PRINTF_INFO("Popped format %d (%d -> %d) at char(%d / %d).\n", idx, rng->rng.start, rng->rng.afterEnd, s->iByte, opq->bytesAdded);
						NBArray_removeItemAtIndex(&s->fmtStack, s->fmtStack.use - 1);
						const UI32 idx = NBArray_itmValueAtIndex(&s->fmtStack, UI32, s->fmtStack.use - 1);
						const STNBTextMetricsBuilderFmtRng* rng	= NBArray_itmPtrAtIndex(&opq->fmtRanges, STNBTextMetricsBuilderFmtRng, idx);
						const STNBTextMetricsBuilderFmt* fmt	= &rng->format;
						float fontSz							= fmt->fontSz;
						if(fontSz <= 0 && fmt->itf.getNativeSz != NULL){
							fontSz = (*fmt->itf.getNativeSz)(fmt->itfParam);
						}
						NBASSERT(fontSz > 0.0f && fmt->itf.getFontMetricsForSz != NULL) //Program logic error (should never be pushed)
						if(fontSz > 0.0f && fmt->itf.getFontMetricsForSz != NULL){
							//Set current range
							s->curRng			= rng;
							s->curFmt			= fmt;
							s->curFontSz		= fontSz;
							s->curFontMetrics	= (*fmt->itf.getFontMetricsForSz)(fmt->itfParam, fontSz);
						}
					}
				}
				//Process char
				charSz	= NBEncoding_utf8BytesExpected(text[iByte]);
				unicode	= NBEncoding_unicodeFromUtf8s(&text[iByte], (UI8)charSz, 0);
				if(!NBTextMetricsBuilder_feedUnicode(obj, dst, column, unicode, s->iByte, charSz, FALSE)){
					NBASSERT(FALSE)
					break;
				} else {
					iByte		+= charSz;
					s->iByte	+= charSz;
				}
			}
			NBASSERT(s->iByte == opq->bytesAdded)
		}
	}
	return iByte;
}

BOOL NBTextMetricsBuilder_feedEnd(STNBTextMetricsBuilder* obj, STNBTextMetrics* dst){
	BOOL r = FALSE;
	STNBTextMetricsBuilderOpq* opq = (STNBTextMetricsBuilderOpq*)obj->opaque;
	STNBTextMetricsBuilderState* s = &opq->state;
	if(NBTextMetricsBuilder_feedUnicode(obj, dst, s->curCol, '\0', s->iByte, 1, TRUE)){
		r = TRUE;
	}
	/*if(buildMultiLevelBoxes){
		NBTextMetrics_buildMultiLevelBoxes(dst, NB_TEXT_METRICS_BUILDER_LEVELS_GROUPS_SIZE);
	}*/
	return r;
}


//

/*const STNBTextMetricsBuilderFmtRng* NBTextMetricsBuilder_getFormatForByteIdxOpq(const STNBTextMetricsBuilderOpq* opq, const UI32 byteIdx){
	const STNBTextMetricsBuilderFmtRng* r = NULL;
	if(opq->fmtRanges.use > 0){
		//Binary search (starts are ordered)
		SI32 posEnd		= (opq->fmtRanges.use - 1);
		SI32 posStart	= 0;
		SI32 posMidd;
		const STNBTextMetricsBuilderFmtRng* dataMidd = NULL;
		while(posStart <= posEnd){
			posMidd		= posStart + ((posEnd - posStart)/2);
			dataMidd	= NBArray_itmPtrAtIndex(&opq->fmtRanges, STNBTextMetricsBuilderFmtRng, posMidd);
			//Into range
			if(rng->rng.start <= byteIdx && byteIdx < rng->rng.afterEnd){
				r = dataMidd;
				break;
			} else {
				if(iChar < dataMidd->rngChars.start){
					posEnd		= posMidd - 1;
				} else {
					posStart	= posMidd + 1;
				}
			}
		}
		NBASSERT(r != NULL) //Must exists
	}
	/ *UI32 i; for(i = 0; i < opq->fmtRanges.use; i++){
		const STNBTextMetricsBuilderFmtRng* rng = NBArray_itmPtrAtIndex(&opq->fmtRanges, STNBTextMetricsBuilderFmtRng, i);
		//All the ranges are ordered by their starts
		if(byteIdx < rng->rng.start){
			break; //end-of-cicle
		} else if(rng->rng.start <= byteIdx && byteIdx < rng->rng.afterEnd){
			r = rng;
		}
	}* /
	return r;
}*/

//dbg

void NBTextMetricsBuilder_dbgTestAllSeqs(const STNBTextMetricsBuilder* obj, const UI32 textStrLen, const UI8 printLevel){
	const STNBTextMetricsBuilderOpq* opq = (STNBTextMetricsBuilderOpq*)obj->opaque;
	//Test format stack
	if(opq->fmtRanges.use > 0){
		STNBArray parents;
		NBArray_init(&parents, sizeof(STNBTextMetricsBuilderFmtRng*), NULL);
		{
			//Full-root range
			const STNBTextMetricsBuilderFmtRng* root = NBArray_itmPtrAtIndex(&opq->fmtRanges, STNBTextMetricsBuilderFmtRng, 0);
			NBASSERT(root->rng.start == 0) //forced integrity (continuity)
			NBASSERT(root->rng.afterEnd == textStrLen) //forced integrity (full-root-range)
			UI32 prevStart = root->rng.start;
			NBArray_addValue(&parents, root);
			//Other ranges
			UI32 i; for(i = 1; i < opq->fmtRanges.use; i++){
				const STNBTextMetricsBuilderFmtRng* rng = NBArray_itmPtrAtIndex(&opq->fmtRanges, STNBTextMetricsBuilderFmtRng, i);
				NBASSERT(prevStart <= rng->rng.start) //forced integrity (continuity)
				NBASSERT(parents.use > 0) //forced integrity (must-have a full-root-range)
				//Analyze against parents
				{
					SI32 j; for(j = (parents.use - 1); j >= 0; j--){
						const STNBTextMetricsBuilderFmtRng* prnt = NBArray_itmValueAtIndex(&parents, STNBTextMetricsBuilderFmtRng*, j);
						if(prnt->rng.afterEnd <= rng->rng.start){
							//Pop parent
							NBASSERT(j > 0) //Must-not-pop the root format
							NBArray_removeItemAtIndex(&parents, j);
						} else {
							//This must be inside parent's range
							NBASSERT(prnt->rng.start <= rng->rng.start && rng->rng.afterEnd <= prnt->rng.afterEnd)
						}
					}
				}
				//Push this range
				NBASSERT(parents.use > 0) //forced integrity (must-have a full-root-range)
				NBArray_addValue(&parents, rng);
				//Next
				prevStart = rng->rng.start;
			}
		}
		NBArray_release(&parents);
	} else {
		//All bytes must be mapped
		NBASSERT(textStrLen == 0)
	}
	//
	NBTextMetricsBuilder_dbgTestFmtSearchByByteIdx(obj, textStrLen, printLevel);
}

void NBTextMetricsBuilder_dbgTestFmtSearchByByteIdx(const STNBTextMetricsBuilder* obj, const UI32 textStrLen, const UI8 printLevel){
	/*const STNBTextMetricsBuilderOpq* opq = (STNBTextMetricsBuilderOpq*)obj->opaque;
	UI32 i; for(i = 0; i < textStrLen; i++){
		const STNBTextMetricsBuilderFmtRng* fmt = NBTextMetricsBuilder_getFormatForByteIdxOpq(opq, i);
		NBASSERT(fmt != NULL)
	}*/
}
