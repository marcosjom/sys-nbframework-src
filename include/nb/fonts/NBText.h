//
//  NBText.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 16/11/18.
//

#ifndef NBText_h
#define NBText_h

#include "nb/NBFrameworkDefs.h"
#include "nb/fonts/NBTextMetricsBuilder.h"
#include "nb/fonts/NBTextMetrics.h"
#include "nb/2d/NBAABox.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBTextDefFmt_ {
		ENNBTextLineAlignH	alignH;
		ENNBTextCharAlignV	alignV;
		float				charsSepFactor;
		float				linesSepFactor;
		STNBColor8			fontColor;
		float				fontSz;
		STNBFontMetricsI	fontItf;
		void*				fontItfParam;
	} STNBTextDefFmt;
	
	typedef struct STNBText_ {
		STNBTextMetricsBuilder	builder;	//formats and text
		STNBTextMetrics			metrics;	//resulting metrics
		ENNBTextAlignV			alignV;
		STNBString				text;
		//Defaults
		STNBTextDefFmt			defaults;
	} STNBText;
	
	void				NBText_init(STNBText* obj);
	void				NBText_initWithOther(STNBText* obj, const STNBText* other);
	void				NBText_release(STNBText* obj);
	
	//Sync
	void				NBText_syncDataWithOther(STNBText* obj, const STNBText* other);
	
	//Global format
	void				NBText_setTextAlign(STNBText* obj, const ENNBTextAlignV align);
	ENNBTextAlignV		NBText_getTextAlign(const STNBText* obj);
	
	//Defaults (are important for cursor data when text is empty)
	void				NBText_setDefaults(STNBText* obj, const STNBFontMetrics* defFontMetrics, const STNBPoint* defStartPos);
	void				NBText_setDefaultFontMetrics(STNBText* obj, const STNBFontMetrics* defFontMetrics);
	void				NBText_setDefaultStartPos(STNBText* obj, const STNBPoint* defStartPos);
	
	//Formats stack
	void				NBText_pushFormat(STNBText* obj);
	void				NBText_pushFormatWithFont(STNBText* obj, const STNBFontMetricsIRef fontItfRef, const float defFontSz, const STNBColor8 color, const ENNBTextLineAlignH lineAlignH, const ENNBTextCharAlignV charAlignV);
	void				NBText_pushFormatWithFontItf(STNBText* obj, const STNBFontMetricsI* itf, void* itfParam, const float fontSz, const STNBColor8 color, const ENNBTextLineAlignH lineAlignH, const ENNBTextCharAlignV charAlignV);
	void				NBText_popFormat(STNBText* obj);
	UI32				NBText_getActiveFormatsStackSz(const STNBText* obj);
	
	//Active format
	void				NBText_setFormatAndFont(STNBText* obj, const STNBFontMetricsIRef fontItfRef, const float defFontSz, const STNBColor8 color, const ENNBTextLineAlignH lineAlignH, const ENNBTextCharAlignV charAlignV);
	void				NBText_setFormatAndFontItf(STNBText* obj, const STNBFontMetricsI* itf, void* itfParam, const float fontSz, const STNBColor8 color, const ENNBTextLineAlignH lineAlignH, const ENNBTextCharAlignV charAlignV);
	void				NBText_setFormat(STNBText* obj, const STNBColor8 color, const ENNBTextLineAlignH lineAlignH, const ENNBTextCharAlignV charAlignV);
	void				NBText_setFontSz(STNBText* obj, const float fontSz);
	void				NBText_setColor(STNBText* obj, const STNBColor8 color);
	void				NBText_setLineAlign(STNBText* obj, const ENNBTextLineAlignH align);
	void				NBText_setCharAlign(STNBText* obj, const ENNBTextCharAlignV align);
	void				NBText_setFont(STNBText* obj, const STNBFontMetricsIRef fontItfRef);
	void				NBText_setFontItf(STNBText* obj, const STNBFontMetricsI* itf, void* itfParam);
	
	//Default format (from first format block defined)
	STNBTextDefFmt		NBText_getDefaultFormat(const STNBText* obj);
	ENNBTextLineAlignH	NBText_getDefAlignH(const STNBText* obj);
	ENNBTextCharAlignV	NBText_getDefCharAlignV(const STNBText* obj);
	float				NBText_getDefCharsSeptFactor(const STNBText* obj);
	float				NBText_getDefLinesSeptFactor(const STNBText* obj);
	STNBColor8			NBText_getDefFontColor(const STNBText* obj);
	float				NBText_getDefFontSz(const STNBText* obj);
	STNBFontMetricsI	NBText_getDefFontItf(const STNBText* obj, void** dstFontItfParam);
	
	//Text
	const char*			NBText_getText(const STNBText* obj, UI32* dstBytesLen);
	UI32				NBText_getTextLen(const STNBText* obj);		//bytes-len
	UI32				NBText_getCharsCount(const STNBText* obj);	//logic-chars-len
	UI32				NBText_getWordsCount(const STNBText* obj);	//chars-groups-len
	UI32				NBText_getLinesCount(const STNBText* obj);	//lines-len
	STNBRangeI			NBText_getRngOfLineByChar(const STNBText* obj, const SI32 charDefPos);
	
	//Monoformat
	void				NBText_setText(STNBText* obj, const STNBRect column, const BOOL allowMultiline, const char* text);
	void				NBText_setTextBytes(STNBText* obj, const STNBRect column, const BOOL allowMultiline, const char* text, const UI32 textSz);
	void				NBText_setTextRepeating(STNBText* obj, const STNBRect column, const BOOL allowMultiline, const char* text, const UI32 textSz, const UI32 timesRepeat);
	
	//Metrics calculation (monoformat)
	STNBAABox			NBText_boxForText(const STNBText* obj, const STNBRect column, const BOOL allowMultiline, const char* text, const UI32 textSz);
	STNBAABox			NBText_boxForTextWithFont(const STNBRect column, const BOOL allowMultiline, const char* text, const UI32 textSz, STNBFontMetricsIRef fontItfRef, const float defFontSz, const ENNBTextLineAlignH alignH, const ENNBTextAlignV alignV);
	STNBAABox			NBText_boxForTextWithFontItf(const STNBRect column, const BOOL allowMultiline, const char* text, const UI32 textSz, const STNBFontMetricsI* fontItf, void* fontItfParam, const float fontSz, const ENNBTextLineAlignH alignH, const ENNBTextAlignV alignV);
	
	//Multiformat
	void				NBText_appendTextWithFont(STNBText* obj, const char* text, const STNBFontMetricsIRef fontItfRef, const float defFontSz);
	void				NBText_appendTextWithFontItf(STNBText* obj, const char* text, const STNBFontMetricsI* fontItf, void* fontItfParam, const float fontSz);
	void				NBText_appendText(STNBText* obj, const char* str);
	void				NBText_appendTextBytes(STNBText* obj, const char* str, const UI32 strSz);
	void				NBText_syncOpenFormatRanges(STNBText* obj);
	//
	STNBAABox			NBText_metricsBox(const STNBText* obj, const BOOL includeAnyCursor);
	void				NBText_organizeText(STNBText* obj, const STNBRect column, const BOOL allowMultiline);
	
	//Edit
	void				NBText_empty(STNBText* obj);
	STNBRangeI			NBText_removeChars(STNBText* obj, const UI32 start, const UI32 size, STNBRangeI* dstRemovedCharsRng);
	STNBRangeI			NBText_removeCharsWithoutIntegrity(STNBText* obj, const UI32 start, const UI32 size, STNBRangeI* dstRemovedCharsRng); //"NBText_organizeText" must be called before a method that requires integrity
	STNBRangeI			NBText_replaceChars(STNBText* obj, const UI32 start, const UI32 size, const char* newStr, const UI32 newSrtSz, STNBRangeI* dstRemovedCharsRng, STNBRangeI* dstAddedCharsRng);
	STNBRangeI			NBText_replaceCharsWithoutIntegrity(STNBText* obj, const UI32 start, const UI32 size, const char* newStr, const UI32 newSrtSz, STNBRangeI* dstRemovedCharsRng, STNBRangeI* dstAddedCharsRng); //"NBText_organizeText" must be called before a method that requires integrity
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	void				NBText_dbgTestSequences(const STNBText* obj, const UI8 printLevel);
	void				NBText_dbgTestRemoveAndReplace(const UI32 totalTests, const UI32 ciclesPerTest, STNBFontMetricsIRef* fonts, const UI32 fontsSz, const UI8 printLevel);
#	endif
	
#ifdef __cplusplus
}
#endif

#endif /* NBText_h */
