//
//  NBFontMetricsResults.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/12/18.
//

#ifndef NBFontMetricsResults_h
#define NBFontMetricsResults_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBRange.h"
#include "nb/2d/NBColor.h"
#include "nb/2d/NBPoint.h"
#include "nb/2d/NBRect.h"
#include "nb/2d/NBAABox.h"
#include "nb/fonts/NBFontMetrics.h"

/*
#define NB_TEXT_METRICS_LINES_INDEXES_BY_RANGE(OBJ, RNG, DST_IDX_FIRST, DST_IDX_AFTER_END)	\
	DST_IDX_FIRST		= RNG.start; \
	DST_IDX_AFTER_END	= RNG.start + RNG.size; \
	if(DST_IDX_FIRST < 0) DST_IDX_FIRST = 0; \
	if(DST_IDX_FIRST > (OBJ)->boxes.use) DST_IDX_FIRST = (OBJ)->boxes.use; \
	if(DST_IDX_AFTER_END > (OBJ)->boxes.use) DST_IDX_AFTER_END = (OBJ)->boxes.use;
*/

#ifdef __cplusplus
extern "C" {
#endif
	
	//Char action (for metrics organization)
	typedef enum ENTextMetricsCharMode_ {
		ENTextMetricsCharMode_ignore = 0,	//ignore the char (only for '\0' and processed chars)
		ENTextMetricsCharMode_noVisual,		//add control char (no-width, no-advance, will be added to the current line)
		ENTextMetricsCharMode_add			//add char and advance (can be added to the next word)
	} ENTextMetricsCharMode;
	
	//Char action (for metrics organization)
	typedef struct STNBTextMetricsCharAction_ {
		BOOL					isSpace;
		BOOL					endWord;
		BOOL					endLine;
		ENTextMetricsCharMode	charMode;
	} STNBTextMetricsCharAction;
	
	//Line horizontal alignment
	typedef enum ENNBTextLineAlignH_ {
		ENNBTextLineAlignH_Base = 0,	//Each line extends x and y font's default.
		ENNBTextLineAlignH_Left,		//Each line extends to x-negative-axe, and y font's default.
		ENNBTextLineAlignH_Center,		//Each line extends the same amount at both sides of the x-axe.
		ENNBTextLineAlignH_Right,		//Each line extends to x-positive-axe, and y font's default.
		ENNBTextLineAlignH_Adjust,		//Spaces between characters are extended.
		ENNBTextLineAlignH_Count
	} ENNBTextLineAlignH;
	
	//Text vertical alignment
	typedef enum ENNBTextAlignV_ {
		ENNBTextAlignV_Base = 0,
		ENNBTextAlignV_FromBottom,
		ENNBTextAlignV_Center,
		ENNBTextAlignV_FromTop,
		ENNBTextAlignV_Count
	} ENNBTextAlignV;
	
	//Char vertical alignment
	typedef enum ENNBTextCharAlignV_ {
		ENNBTextCharAlignV_Base = 0,
		ENNBTextCharAlignV_SuperScript,
		ENNBTextCharAlignV_SubScript,
		ENNBTextCharAlignV_Count
	} ENNBTextCharAlignV;
	
	typedef enum ENNBTextSide_ {
		ENNBTextSide_Left = 0,
		ENNBTextSide_Right
	} ENNBTextSide;
	
	typedef struct STNBTextMetricsChar_ {
		void*				itfObj;
		UI32				iByte;			//Index of char's first byte
		UI8					bytesLen;
		UI8					vAlign;			//ENNBTextCharAlignV
		BOOL				isSpace;
		BOOL				isControl;
		UI32				shapeId;
		STNBPoint			pos;
		float				extendsLeft;	//visible extension
		float				extendsRight;	//visible extension
		STNBColor8			color;
	} STNBTextMetricsChar;
	
	typedef struct STNBTextMetricsCharsRng_ {
		UI32	start;		//first unicode-char
		UI32	afterEnd;	//unicode-char after end (basically, start + count)
	} STNBTextMetricsCharsRng;
	
	typedef struct STNBTextMetricsWord_ {
		STNBTextMetricsCharsRng rngChars;	//range of chars
		BOOL					isControl;	//is control char
		BOOL					isSpace;	//is space char
	} STNBTextMetricsWord;
	
	typedef struct STNBTextMetricsLine_ {
		STNBTextMetricsCharsRng	rngWords;
		float					yBase;
		float					lBearing;		//first char's hBearingX
		float					rBearing;		//last char's (hAdvance - width - hBearingX)
		float					visibleLeft;	//visible left (excludes the first hBearingX)
		float					visibleRight;	//visible right (excludes the last (hAdvance - width - hBearingX))
		ENNBTextLineAlignH		alignH;			//line align
		STNBFontMetrics			fontMetricsMax;
	} STNBTextMetricsLine;
	
	typedef struct STNBTextMetricsDefaults_ {
		STNBFontMetrics	fontMetrics;		//default text metrics (used to determine cursor's size when the metric is empty)
		STNBPoint		start;				//default start of cursor (used to determine cursor's pos when the metric is empty).
	} STNBTextMetricsDefaults;

	/*typedef struct STNBTextMetricsBox_ {
		STNBAABox		box;		//Box
		STNBRangeI		rng;		//Indexes of lines (for level 0) or groups (for levels > 0)
	} STNBTextMetricsBox;*/
		
	typedef struct STNBTextMetrics_ {
		STNBArray		chars;				//STNBTextMetricsChar
		STNBArray		words;				//STNBTextMetricsWord
		STNBArray		lines;				//STNBTextMetricsLine
		//Optimizations boxes
		//STNBArray		boxes;				//STNBTextMetricsBox
		//STNBArray		boxesLvls;			//STNBRangeI (indexes inside 'boxes' array)
		//UI32			boxesGrpsSz;		//Las known configuration
		//Defaults values (when empty text)
		STNBTextMetricsDefaults defaults;	//defaults (used to determine cursor's size when the metric is empty)
	} STNBTextMetrics;
	
	void			NBTextMetrics_init(STNBTextMetrics* obj);
	void			NBTextMetrics_initWithOther(STNBTextMetrics* obj, const STNBTextMetrics* other);
	void			NBTextMetrics_release(STNBTextMetrics* obj);
	
	//Sync
	void			NBTextMetrics_syncDataWithOther(STNBTextMetrics* obj, const STNBTextMetrics* other);
	
	//
	void			NBTextMetrics_empty(STNBTextMetrics* obj);
	void			NBTextMetrics_setDefaults(STNBTextMetrics* obj, const STNBFontMetrics* defFontMetrics, const STNBPoint* defStart);
	void			NBTextMetrics_setDefaultFontMetrics(STNBTextMetrics* obj, const STNBFontMetrics* defFontMetrics);
	void			NBTextMetrics_setDefaultStartPos(STNBTextMetrics* obj, const STNBPoint* defStartPos);
	
	//Box
	STNBAABox		NBTextMetrics_box(const STNBTextMetrics* obj, const BOOL includeAnyCursor);
	//void			NBTextMetrics_buildMultiLevelBoxes(STNBTextMetrics* obj, const UI32 sizePerGrp);
	
	//Searches
	STNBRangeI		NBTextMetrics_charsRangeToBytesRange(const STNBTextMetrics* obj, const UI32 start, const UI32 size);
	STNBRangeI		NBTextMetrics_bytesRangeToCharsRange(const STNBTextMetrics* obj, const UI32 start, const UI32 size);
	const STNBTextMetricsLine* NBTextMetrics_lineByCharIndex(const STNBTextMetrics* obj, const UI32 iChar);
	const STNBTextMetricsLine* NBTextMetrics_lineByCoord(const STNBTextMetrics* obj, const float y);
	const STNBTextMetricsWord* NBTextMetrics_wordByCharIndex(const STNBTextMetrics* obj, const UI32 iChar);
	const STNBTextMetricsChar* NBTextMetrics_charAtLineByCoord(const STNBTextMetrics* obj, const STNBTextMetricsLine* line, const float x);
	
	//Text geometry
	STNBRect		NBTextMetrics_rectForCursor(const STNBTextMetrics* obj, const UI32 iChar);
	STNBRect		NBTextMetrics_firstRectForCharsRange(const STNBTextMetrics* obj, const UI32 start, const UI32 size);
	void			NBTextMetrics_rectsForCharsRange(const STNBTextMetrics* obj, const UI32 start, const UI32 size, STNBArray* dstRects);
	UI32			NBTextMetrics_closestCharIdx(const STNBTextMetrics* obj, const float x, const float y, UI32* dstLineIdx);
	
	//Char action (for metrics organization)
	STNBTextMetricsCharAction NBTextMetrics_getCharAction(const UI32 unicode, const UI32 curLineWordsCount, const UI32 curWordCharsCount);
	
	//Editions
	STNBRangeI		NBTextMetrics_removeChars(STNBTextMetrics* obj, const UI32 start, const UI32 numChars, STNBRangeI* dstRemovedCharsRng);
	STNBRangeI		NBTextMetrics_replaceChars(STNBTextMetrics* obj, const UI32 start, const UI32 numChars, const char* newStr, const UI32 newStrSz, STNBRangeI* dstRemovedCharsRng, STNBRangeI* dstAddedCharsRng);
	STNBRangeI		NBTextMetrics_replaceCharsSimulation(STNBTextMetrics* obj, const UI32 start, const UI32 numChars, const char* newStr, const UI32 newStrSz, STNBRangeI* dstRemoveCharsRng, STNBRangeI* dstAddCharsRng);
	
	//dbg
	void			NBTextMetrics_dbgTestAllSeqs(const STNBTextMetrics* obj, const char* textStr, const UI32 textStrLen, const UI8 printLevel);
	void			NBTextMetrics_dbgTestCharsSeq(const STNBTextMetrics* obj, const char* textStr, const UI32 textStrLen, const UI8 printLevel);
	void			NBTextMetrics_dbgTestWordsSeq(const STNBTextMetrics* obj, const char* textStr, const UI32 textStrLen, const UI8 printLevel);
	void			NBTextMetrics_dbgTestLinesSeq(const STNBTextMetrics* obj, const char* textStr, const UI32 textStrLen, const UI8 printLevel);
	void			NBTextMetrics_dbgTestRangesQueries(const STNBTextMetrics* obj, const char* textStr, const UI32 textStrLen, const UI8 printLevel);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBFontMetricsResults_h */
