//
//  NBTextMetricsBuilder.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/12/18.
//

#ifndef NBTextMetricsBuilder_h
#define NBTextMetricsBuilder_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/2d/NBPoint.h"
#include "nb/2d/NBSize.h"
#include "nb/2d/NBRect.h"
#include "nb/fonts/NBFontMetrics.h"
#include "nb/fonts/NBTextMetrics.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBTextMetricsBuilder_ {
		void*	opaque;
	} STNBTextMetricsBuilder;
	
	void NBTextMetricsBuilder_init(STNBTextMetricsBuilder* obj);
	void NBTextMetricsBuilder_initWithOther(STNBTextMetricsBuilder* obj, const STNBTextMetricsBuilder* other);
	void NBTextMetricsBuilder_retain(STNBTextMetricsBuilder* obj);
	void NBTextMetricsBuilder_release(STNBTextMetricsBuilder* obj);
	
	//Sync
	void NBTextMetricsBuilder_syncDataWithOther(STNBTextMetricsBuilder* obj, const STNBTextMetricsBuilder* other);
	
	//Format stack
	void NBTextMetricsBuilder_pushFormat(STNBTextMetricsBuilder* obj);
	void NBTextMetricsBuilder_pushFormatWithFont(STNBTextMetricsBuilder* obj, const STNBFontMetricsIRef fontItfRef, const float defFontSz, const STNBColor8 color, const ENNBTextLineAlignH lineAlignH, const ENNBTextCharAlignV charAlignV);
	void NBTextMetricsBuilder_pushFormatWithFontItf(STNBTextMetricsBuilder* obj, const STNBFontMetricsI* itf, void* itfParam, const float fontSz, const STNBColor8 color, const ENNBTextLineAlignH lineAlignH, const ENNBTextCharAlignV charAlignV);
	void NBTextMetricsBuilder_popFormat(STNBTextMetricsBuilder* obj);
	void NBTextMetricsBuilder_empty(STNBTextMetricsBuilder* obj);
	void NBTextMetricsBuilder_syncOpenFormatRanges(STNBTextMetricsBuilder* obj);
	
	//
	UI32 NBTextMetricsBuilder_getActiveStackSz(const STNBTextMetricsBuilder* obj);
	UI32 NBTextMetricsBuilder_getFormatsCount(const STNBTextMetricsBuilder* obj);
	
	//Active format
	void NBTextMetricsBuilder_setFormatAndFont(STNBTextMetricsBuilder* obj, const STNBFontMetricsIRef fontItfRef, const float defFontSz, const STNBColor8 color, const ENNBTextLineAlignH lineAlignH, const ENNBTextCharAlignV charAlignV);
	void NBTextMetricsBuilder_setFormatAndFontItf(STNBTextMetricsBuilder* obj, const STNBFontMetricsI* itf, void* itfParam, const float fontSz, const STNBColor8 color, const ENNBTextLineAlignH lineAlignH, const ENNBTextCharAlignV charAlignV);
	void NBTextMetricsBuilder_setFormat(STNBTextMetricsBuilder* obj, const STNBColor8 color, const ENNBTextLineAlignH lineAlignH, const ENNBTextCharAlignV charAlignV);
	void NBTextMetricsBuilder_setFontSz(STNBTextMetricsBuilder* obj, const float fontSz);
	void NBTextMetricsBuilder_setColor(STNBTextMetricsBuilder* obj, const STNBColor8 color);
	void NBTextMetricsBuilder_setLineAlign(STNBTextMetricsBuilder* obj, const ENNBTextLineAlignH align);
	void NBTextMetricsBuilder_setCharAlign(STNBTextMetricsBuilder* obj, const ENNBTextCharAlignV align);
	void NBTextMetricsBuilder_setFont(STNBTextMetricsBuilder* obj, const STNBFontMetricsIRef fontItfRef);
	void NBTextMetricsBuilder_setFontItf(STNBTextMetricsBuilder* obj, const STNBFontMetricsI* itf, void* itfParam);
	
	//Text (active stack)
	UI32 NBTextMetricsBuilder_getBytesLen(const STNBTextMetricsBuilder* obj);
	void NBTextMetricsBuilder_appendBytes(STNBTextMetricsBuilder* obj, const char* str, const UI32 strSz);
	void NBTextMetricsBuilder_removeBytes(STNBTextMetricsBuilder* obj, const UI32 start, const UI32 bytesLen);
	void NBTextMetricsBuilder_replaceBytes(STNBTextMetricsBuilder* obj, const UI32 start, const UI32 bytesLen, const char* newStr, const UI32 newStrSz);
	
	//Calculation
	BOOL NBTextMetricsBuilder_feedStart(STNBTextMetricsBuilder* obj, STNBTextMetrics* dst);
	UI32 NBTextMetricsBuilder_feed(STNBTextMetricsBuilder* obj, STNBTextMetrics* dst, const STNBRect column, const BOOL allowMultiline, const char* text, const UI32 textSz);
	BOOL NBTextMetricsBuilder_feedEnd(STNBTextMetricsBuilder* obj, STNBTextMetrics* dst);
	
	//dbg
	void NBTextMetricsBuilder_dbgTestAllSeqs(const STNBTextMetricsBuilder* obj, const UI32 textStrLen, const UI8 printLevel);
	void NBTextMetricsBuilder_dbgTestFmtSearchByByteIdx(const STNBTextMetricsBuilder* obj, const UI32 textStrLen, const UI8 printLevel);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBTextMetricsBuilder_h */
