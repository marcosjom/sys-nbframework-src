//
//  NBText.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 16/11/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/fonts/NBText.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBEncoding.h"

void NBText_init(STNBText* obj){
	NBMemory_setZeroSt(*obj, STNBText);
	NBTextMetricsBuilder_init(&obj->builder);
	NBTextMetrics_init(&obj->metrics);
	obj->alignV	= ENNBTextAlignV_FromTop;
	NBString_init(&obj->text);
	obj->defaults.charsSepFactor	= 1.0f;
	obj->defaults.linesSepFactor	= 1.0f;
	obj->defaults.fontColor.r		= 255; //make visible
	obj->defaults.fontColor.g		= 255; //make visible
	obj->defaults.fontColor.b		= 255; //make visible
	obj->defaults.fontColor.a		= 255; //make visible
}

void NBText_initWithOther(STNBText* obj, const STNBText* other){
	NBTextMetricsBuilder_initWithOther(&obj->builder, &other->builder);
	NBTextMetrics_initWithOther(&obj->metrics, &other->metrics);
	obj->alignV		= other->alignV;
	NBString_initWithOther(&obj->text, &other->text);
	obj->defaults	= other->defaults;
	//Retain font
	if(obj->defaults.fontItf.retain != NULL){
		(*obj->defaults.fontItf.retain)(obj->defaults.fontItfParam);
	}
}

void NBText_release(STNBText* obj){
	//Release font
	if(obj->defaults.fontItf.release != NULL){
		(*obj->defaults.fontItf.release)(obj->defaults.fontItfParam);
	}
	//
	NBString_release(&obj->text);
	NBTextMetricsBuilder_release(&obj->builder);
	NBTextMetrics_release(&obj->metrics);
}

//Sync

void NBText_syncDataWithOther(STNBText* obj, const STNBText* other){
	if(obj != other){
		NBTextMetricsBuilder_syncDataWithOther(&obj->builder, &other->builder);
		NBTextMetrics_syncDataWithOther(&obj->metrics, &other->metrics);
		obj->alignV		= other->alignV;
		NBString_setBytes(&obj->text, other->text.str, other->text.length);
		//Defaults
		{
			if(other->defaults.fontItf.retain != NULL){
				(*other->defaults.fontItf.retain)(other->defaults.fontItfParam);
			}
			if(obj->defaults.fontItf.release != NULL){
				(*obj->defaults.fontItf.release)(obj->defaults.fontItfParam);
			}
			obj->defaults = other->defaults;
		}
	}
}

//Defaults (are important for cursor data when text is empty)

void NBText_setDefaults(STNBText* obj, const STNBFontMetrics* defFontMetrics, const STNBPoint* defStartPos){
	NBTextMetrics_setDefaults(&obj->metrics, defFontMetrics, defStartPos);
}

void NBText_setDefaultFontMetrics(STNBText* obj, const STNBFontMetrics* defFontMetrics){
	NBTextMetrics_setDefaultFontMetrics(&obj->metrics, defFontMetrics);
}

void NBText_setDefaultStartPos(STNBText* obj, const STNBPoint* defStartPos){
	NBTextMetrics_setDefaultStartPos(&obj->metrics, defStartPos);
}

//Format stacks

void NBText_pushFormat(STNBText* obj){
	NBTextMetricsBuilder_pushFormat(&obj->builder);
}

void NBText_pushFormatWithFont(STNBText* obj, const STNBFontMetricsIRef fontItfRef, const float defFontSz, const STNBColor8 color, const ENNBTextLineAlignH lineAlignH, const ENNBTextCharAlignV charAlignV){
	NBText_pushFormatWithFontItf(obj, &fontItfRef.itf, fontItfRef.itfParam, (fontItfRef.fontSz > 0 ? fontItfRef.fontSz : defFontSz), color, lineAlignH, charAlignV);
}

void NBText_pushFormatWithFontItf(STNBText* obj, const STNBFontMetricsI* itf, void* itfParam, const float fontSz, const STNBColor8 color, const ENNBTextLineAlignH lineAlignH, const ENNBTextCharAlignV charAlignV){
	NBTextMetricsBuilder_pushFormatWithFontItf(&obj->builder, itf, itfParam, fontSz, color, lineAlignH, charAlignV);
	//Save the default font
	if(1 == NBTextMetricsBuilder_getActiveStackSz(&obj->builder)){
		STNBTextDefFmt* def = &obj->defaults;
		if(itf != NULL){
			if(itf->retain != NULL){
				(*itf->retain)(itfParam);
			}
		}
		if(def->fontItf.release != NULL){
			(*def->fontItf.release)(def->fontItfParam);
		}
		def->alignH			= lineAlignH;
		def->alignV			= charAlignV;
		def->charsSepFactor	= 1.0f;
		def->linesSepFactor	= 1.0f;
		def->fontColor		= color;
		def->fontSz			= fontSz;
		if(itf != NULL){
			def->fontItf	= *itf;
		} else {
			NBMemory_setZeroSt(def->fontItf, STNBFontMetricsI);
		}
		def->fontItfParam	= itfParam;
		//Pass defaults to metics (for cursor data when text is empty)
		if(def->fontItf.getFontMetricsForSz != NULL){
			const STNBFontMetrics metrics = (*def->fontItf.getFontMetricsForSz)(def->fontItfParam, def->fontSz);
			NBTextMetrics_setDefaultFontMetrics(&obj->metrics, &metrics);
		}
	}
}

void NBText_popFormat(STNBText* obj){
	NBTextMetricsBuilder_popFormat(&obj->builder);
}

UI32 NBText_getActiveFormatsStackSz(const STNBText* obj){
	return NBTextMetricsBuilder_getActiveStackSz(&obj->builder);
}

//Active format

void NBText_setFormatAndFont(STNBText* obj, const STNBFontMetricsIRef fontItfRef, const float defFontSz, const STNBColor8 color, const ENNBTextLineAlignH lineAlignH, const ENNBTextCharAlignV charAlignV){
	NBText_setFormatAndFontItf(obj, &fontItfRef.itf, fontItfRef.itfParam, (fontItfRef.fontSz > 0 ? fontItfRef.fontSz : defFontSz), color, lineAlignH, charAlignV);
}

void NBText_setFormatAndFontItf(STNBText* obj, const STNBFontMetricsI* itf, void* itfParam, const float fontSz, const STNBColor8 color, const ENNBTextLineAlignH lineAlignH, const ENNBTextCharAlignV charAlignV){
	NBTextMetricsBuilder_setFormatAndFontItf(&obj->builder, itf, itfParam, fontSz, color, lineAlignH, charAlignV);
	//Save the default font
	if(1 == NBTextMetricsBuilder_getActiveStackSz(&obj->builder)){
		STNBTextDefFmt* def = &obj->defaults;
		if(itf != NULL){
			if(itf->retain != NULL){
				(*itf->retain)(itfParam);
			}
		}
		if(def->fontItf.release != NULL){
			(*def->fontItf.release)(def->fontItfParam);
		}
		def->alignH			= lineAlignH;
		def->alignV			= charAlignV;
		def->charsSepFactor	= 1.0f;
		def->linesSepFactor	= 1.0f;
		def->fontColor		= color;
		def->fontSz			= fontSz;
		if(itf != NULL){
			def->fontItf	= *itf;
		} else {
			NBMemory_setZeroSt(def->fontItf, STNBFontMetricsI);
		}
		def->fontItfParam	= itfParam;
		//Pass defaults to metics (for cursor data when text is empty)
		if(def->fontItf.getFontMetricsForSz != NULL){
			const STNBFontMetrics metrics = (*def->fontItf.getFontMetricsForSz)(def->fontItfParam, def->fontSz);
			NBTextMetrics_setDefaultFontMetrics(&obj->metrics, &metrics);
		}
	}
}

void NBText_setFormat(STNBText* obj, const STNBColor8 color, const ENNBTextLineAlignH lineAlignH, const ENNBTextCharAlignV charAlignV){
	NBTextMetricsBuilder_setFormat(&obj->builder, color, lineAlignH, charAlignV);
	//Save the default font
	if(1 == NBTextMetricsBuilder_getActiveStackSz(&obj->builder)){
		STNBTextDefFmt* def = &obj->defaults;
		def->alignH			= lineAlignH;
		def->alignV			= charAlignV;
		def->fontColor		= color;
	}
}

void NBText_setFontSz(STNBText* obj, const float fontSz){
	NBTextMetricsBuilder_setFontSz(&obj->builder, fontSz);
	//Save the default font
	if(1 == NBTextMetricsBuilder_getActiveStackSz(&obj->builder)){
		STNBTextDefFmt* def = &obj->defaults;
		def->fontSz			= fontSz;
		//Pass defaults to metics (for cursor data when text is empty)
		if(def->fontItf.getFontMetricsForSz != NULL){
			const STNBFontMetrics metrics = (*def->fontItf.getFontMetricsForSz)(def->fontItfParam, def->fontSz);
			NBTextMetrics_setDefaultFontMetrics(&obj->metrics, &metrics);
		}
	}
}

void NBText_setColor(STNBText* obj, const STNBColor8 color){
	NBTextMetricsBuilder_setColor(&obj->builder, color);
	//Save the default font
	if(1 == NBTextMetricsBuilder_getActiveStackSz(&obj->builder)){
		STNBTextDefFmt* def = &obj->defaults;
		def->fontColor		= color;
	}
}

void NBText_setLineAlign(STNBText* obj, const ENNBTextLineAlignH align){
	NBTextMetricsBuilder_setLineAlign(&obj->builder, align);
	//Save the default font
	if(1 == NBTextMetricsBuilder_getActiveStackSz(&obj->builder)){
		STNBTextDefFmt* def = &obj->defaults;
		def->alignH			= align;
	}
}

void NBText_setCharAlign(STNBText* obj, const ENNBTextCharAlignV align){
	NBTextMetricsBuilder_setCharAlign(&obj->builder, align);
	//Save the default font
	if(1 == NBTextMetricsBuilder_getActiveStackSz(&obj->builder)){
		STNBTextDefFmt* def = &obj->defaults;
		def->alignV			= align;
	}
}

void NBText_setFont(STNBText* obj, const STNBFontMetricsIRef fontItfRef){
	NBText_setFontItf(obj, &fontItfRef.itf, fontItfRef.itfParam);
	//Save the default font
	if(fontItfRef.fontSz > 0){
		NBTextMetricsBuilder_setFontSz(&obj->builder, fontItfRef.fontSz);
		if(1 == NBTextMetricsBuilder_getActiveStackSz(&obj->builder)){
			STNBTextDefFmt* def = &obj->defaults;
			def->fontSz			= fontItfRef.fontSz;
			//Pass defaults to metics (for cursor data when text is empty)
			if(def->fontItf.getFontMetricsForSz != NULL){
				const STNBFontMetrics metrics = (*def->fontItf.getFontMetricsForSz)(def->fontItfParam, def->fontSz);
				NBTextMetrics_setDefaultFontMetrics(&obj->metrics, &metrics);
			}
		}
	}
}

void NBText_setFontItf(STNBText* obj, const STNBFontMetricsI* itf, void* itfParam){
	NBTextMetricsBuilder_setFontItf(&obj->builder, itf, itfParam);
	//Save the default font
	if(1 == NBTextMetricsBuilder_getActiveStackSz(&obj->builder)){
		STNBTextDefFmt* def = &obj->defaults;
		if(itf != NULL){
			if(itf->retain != NULL){
				(*itf->retain)(itfParam);
			}
		}
		if(def->fontItf.release != NULL){
			(*def->fontItf.release)(def->fontItfParam);
		}
		if(itf != NULL){
			def->fontItf	= *itf;
		} else {
			NBMemory_setZeroSt(def->fontItf, STNBFontMetricsI);
		}
		def->fontItfParam	= itfParam;
		//Pass defaults to metics (for cursor data when text is empty)
		if(def->fontItf.getFontMetricsForSz != NULL){
			const STNBFontMetrics metrics = (*def->fontItf.getFontMetricsForSz)(def->fontItfParam, def->fontSz);
			NBTextMetrics_setDefaultFontMetrics(&obj->metrics, &metrics);
		}
	}
}

//Global format

void NBText_setTextAlign(STNBText* obj, const ENNBTextAlignV align){
	obj->alignV = align;
}

ENNBTextAlignV NBText_getTextAlign(const STNBText* obj){
	return obj->alignV;
}

//Defaults

STNBTextDefFmt NBText_getDefaultFormat(const STNBText* obj){
	return obj->defaults;
}

ENNBTextLineAlignH NBText_getDefAlignH(const STNBText* obj){
	return obj->defaults.alignH;
}

ENNBTextCharAlignV NBText_getDefCharAlignV(const STNBText* obj){
	return obj->defaults.alignV;
}

float NBText_getDefCharsSeptFactor(const STNBText* obj){
	return obj->defaults.charsSepFactor;
}

float NBText_getDefLinesSeptFactor(const STNBText* obj){
	return obj->defaults.linesSepFactor;
}

STNBColor8 NBText_getDefFontColor(const STNBText* obj){
	return obj->defaults.fontColor;
}

float NBText_getDefFontSz(const STNBText* obj){
	return obj->defaults.fontSz;
}

STNBFontMetricsI NBText_getDefFont(const STNBText* obj, void** dstFontItfParam){
	if(dstFontItfParam != NULL) *dstFontItfParam = obj->defaults.fontItfParam;
	return obj->defaults.fontItf;
}



//Text

const char* NBText_getText(const STNBText* obj, UI32* dstBytesLen){
	if(dstBytesLen != NULL) *dstBytesLen = obj->text.length;
	return obj->text.str;
}

UI32 NBText_getTextLen(const STNBText* obj){
	return NBTextMetricsBuilder_getBytesLen(&obj->builder);
}

UI32 NBText_getCharsCount(const STNBText* obj){
	return obj->metrics.chars.use;
}

UI32 NBText_getWordsCount(const STNBText* obj){
	return obj->metrics.words.use;
}

UI32 NBText_getLinesCount(const STNBText* obj){
	return obj->metrics.lines.use;
}

STNBRangeI NBText_getRngOfLineByChar(const STNBText* obj, const SI32 charDefPos){
	STNBRangeI r;
	NBMemory_setZeroSt(r, STNBRangeI);
	if(obj->metrics.lines.use > 0){
		const STNBTextMetricsLine* ln = NULL;
		if(charDefPos <= 0){
			//Return first line
			ln = NBArray_itmPtrAtIndex(&obj->metrics.lines, STNBTextMetricsLine, 0);
		} else if(charDefPos >= obj->metrics.chars.use){
			//Return last line
			ln = NBArray_itmPtrAtIndex(&obj->metrics.lines, STNBTextMetricsLine, obj->metrics.lines.use - 1);
		} else {
			ln = NBTextMetrics_lineByCharIndex(&obj->metrics, charDefPos);
		}
		if(ln != NULL){
			const STNBTextMetricsWord* w0 = NBArray_itmPtrAtIndex(&obj->metrics.words, STNBTextMetricsWord, ln->rngWords.start);
			const STNBTextMetricsWord* w1 = NBArray_itmPtrAtIndex(&obj->metrics.words, STNBTextMetricsWord, ln->rngWords.afterEnd - 1);
			r.start = w0->rngChars.start;
			r.size	= w1->rngChars.afterEnd - r.start;
		}
	}
	return r;
}

//Monoformat

void NBText_setText(STNBText* obj, const STNBRect column, const BOOL allowMultiline, const char* text){
	UI32 strLen = 0;
	if(text != NULL){
		strLen = NBString_strLenBytes(text);
	}
	NBText_setTextBytes(obj, column, allowMultiline, text, strLen);
}
	
void NBText_setTextBytes(STNBText* obj, const STNBRect column, const BOOL allowMultiline, const char* text, const UI32 textSz){
	//Empty
	NBTextMetricsBuilder_empty(&obj->builder);
	NBTextMetrics_empty(&obj->metrics);
	NBString_setBytes(&obj->text, text, textSz);
	//Feed formats and text
	{
		NBTextMetricsBuilder_pushFormatWithFontItf(&obj->builder, &obj->defaults.fontItf, obj->defaults.fontItfParam, obj->defaults.fontSz, obj->defaults.fontColor, obj->defaults.alignH, ENNBTextCharAlignV_Base);
		NBTextMetricsBuilder_appendBytes(&obj->builder, obj->text.str, obj->text.length); NBASSERT(obj->text.length == NBTextMetricsBuilder_getBytesLen(&obj->builder))
		NBTextMetricsBuilder_popFormat(&obj->builder);
		NBASSERT(obj->text.length == NBTextMetricsBuilder_getBytesLen(&obj->builder))
	}
	//Feed columns
	{
		NBASSERT(column.height >= 0) //Convert scene-coords (y+ up) to bitmap-coords (y+ down).
		NBTextMetricsBuilder_feedStart(&obj->builder, &obj->metrics);
		NBTextMetricsBuilder_feed(&obj->builder, &obj->metrics, column, allowMultiline, obj->text.str, obj->text.length);
		NBTextMetricsBuilder_feedEnd(&obj->builder, &obj->metrics);
	}
	//Apply vertical align
	{
		const SI32 cntLinea = obj->metrics.lines.use;
		if(cntLinea > 0){
			//Alineacion vertical
			const STNBTextMetricsLine* firstLn = NBArray_itmPtrAtIndex(&obj->metrics.lines, STNBTextMetricsLine, 0);
			const STNBTextMetricsLine* lastLn = NBArray_itmPtrAtIndex(&obj->metrics.lines, STNBTextMetricsLine, cntLinea - 1);
			float deltaY   = 0.0f;
			switch(obj->alignV) {
				case ENNBTextAlignV_Base:
					deltaY = -firstLn->fontMetricsMax.ascender;
					break;
				case ENNBTextAlignV_FromBottom:
					deltaY = column.y + column.height - lastLn->yBase + lastLn->fontMetricsMax.descender - 1; //descender is neg
					break;
				case ENNBTextAlignV_Center:
					deltaY = column.y + ((column.height - (lastLn->yBase + lastLn->fontMetricsMax.descender)) * 0.5f);; //descender is neg
					break;
				default: /*ENNBTextAlignV_FromTop*/
					break;
			}
			//Move lines and chars
			if(deltaY != 0.0f){
				UI32 i;
				//Move base lines
				for(i = 0; i < obj->metrics.lines.use; i++){
					STNBTextMetricsLine* l = NBArray_itmPtrAtIndex(&obj->metrics.lines, STNBTextMetricsLine, i);
					l->yBase += deltaY;
				}
				//Move chars
				for(i = 0; i < obj->metrics.chars.use; i++){
					STNBTextMetricsChar* c = NBArray_itmPtrAtIndex(&obj->metrics.chars, STNBTextMetricsChar, i);
					c->pos.y += deltaY;
				}
			}
		}
	}
	//Calculate boxes
	//NBTextMetrics_buildMultiLevelBoxes(&obj->metrics, NB_TEXT_METRICS_BUILDER_LEVELS_GROUPS_SIZE);
}

void NBText_setTextRepeating(STNBText* obj, const STNBRect column, const BOOL allowMultiline, const char* text, const UI32 textSz, const UI32 timesRepeat){
	//Empty
	NBTextMetricsBuilder_empty(&obj->builder);
	NBTextMetrics_empty(&obj->metrics);
	NBString_empty(&obj->text);
	//Feed formats and text
	{
		NBTextMetricsBuilder_pushFormatWithFontItf(&obj->builder, &obj->defaults.fontItf, obj->defaults.fontItfParam, obj->defaults.fontSz, obj->defaults.fontColor, obj->defaults.alignH, ENNBTextCharAlignV_Base);
		{
			UI32 i; for(i = 0; i < timesRepeat; i++){
				NBString_concatBytes(&obj->text, text, textSz);
			}
			if(obj->text.length > 0){
				NBTextMetricsBuilder_appendBytes(&obj->builder, obj->text.str, obj->text.length);
			}
			
		}
		NBTextMetricsBuilder_popFormat(&obj->builder);
		NBASSERT(obj->text.length == NBTextMetricsBuilder_getBytesLen(&obj->builder))
	}
	//Feed columns
	{
		NBASSERT(column.height >= 0) //Convert scene-coords (y+ up) to bitmap-coords (y+ down).
		NBTextMetricsBuilder_feedStart(&obj->builder, &obj->metrics);
		NBTextMetricsBuilder_feed(&obj->builder, &obj->metrics, column, allowMultiline, obj->text.str, obj->text.length);
		NBTextMetricsBuilder_feedEnd(&obj->builder, &obj->metrics);
	}
	//Apply vertical align
	{
		const SI32 cntLinea = obj->metrics.lines.use;
		if(cntLinea > 0){
			//Alineacion vertical
			const STNBTextMetricsLine* firstLn = NBArray_itmPtrAtIndex(&obj->metrics.lines, STNBTextMetricsLine, 0);
			const STNBTextMetricsLine* lastLn = NBArray_itmPtrAtIndex(&obj->metrics.lines, STNBTextMetricsLine, cntLinea - 1);
			float deltaY   = 0.0f;
			switch(obj->alignV) {
				case ENNBTextAlignV_Base:
					deltaY = -firstLn->fontMetricsMax.ascender;
					break;
				case ENNBTextAlignV_FromBottom:
					deltaY = column.y + column.height - lastLn->yBase + lastLn->fontMetricsMax.descender - 1; //descender is neg
					break;
				case ENNBTextAlignV_Center:
					deltaY = column.y + ((column.height - (lastLn->yBase + lastLn->fontMetricsMax.descender)) * 0.5f);; //descender is neg
					break;
				default: /*ENNBTextAlignV_FromTop*/
					break;
			}
			//Move lines and chars
			if(deltaY != 0.0f){
				UI32 i;
				//Move base lines
				for(i = 0; i < obj->metrics.lines.use; i++){
					STNBTextMetricsLine* l = NBArray_itmPtrAtIndex(&obj->metrics.lines, STNBTextMetricsLine, i);
					l->yBase += deltaY;
				}
				//Move chars
				for(i = 0; i < obj->metrics.chars.use; i++){
					STNBTextMetricsChar* c = NBArray_itmPtrAtIndex(&obj->metrics.chars, STNBTextMetricsChar, i);
					c->pos.y += deltaY;
				}
			}
		}
	}
	//Calculate boxes
	//NBTextMetrics_buildMultiLevelBoxes(&obj->metrics, NB_TEXT_METRICS_BUILDER_LEVELS_GROUPS_SIZE);
}

STNBAABox NBText_boxForText(const STNBText* obj, const STNBRect column, const BOOL allowMultiline, const char* text, const UI32 textSz){
	return NBText_boxForTextWithFontItf(column, allowMultiline, text, textSz, &obj->defaults.fontItf, obj->defaults.fontItfParam, obj->defaults.fontSz, obj->defaults.alignH, obj->alignV);
}

STNBAABox NBText_boxForTextWithFont(const STNBRect column, const BOOL allowMultiline, const char* text, const UI32 textSz, STNBFontMetricsIRef fontItfRef, const float defFontSz, const ENNBTextLineAlignH alignH, const ENNBTextAlignV alignV){
	return NBText_boxForTextWithFontItf(column, allowMultiline, text, textSz, &fontItfRef.itf, fontItfRef.itfParam, (fontItfRef.fontSz > 0 ? fontItfRef.fontSz : defFontSz), alignH, alignV);
}

STNBAABox NBText_boxForTextWithFontItf(const STNBRect column, const BOOL allowMultiline, const char* text, const UI32 textSz, const STNBFontMetricsI* fontItf, void* fontItfParam, const float fontSz, const ENNBTextLineAlignH alignH, const ENNBTextAlignV alignV){
	NBASSERT(fontItf != NULL)
	STNBAABox box; NBAABox_init(&box);
	STNBTextMetrics m;
	STNBTextMetricsBuilder mBldr;
	NBTextMetrics_init(&m);
	NBTextMetricsBuilder_init(&mBldr);
	//Feed formats and text
	{
		const UI32 textSz = NBString_strLenBytes(text);
		NBTextMetricsBuilder_pushFormat(&mBldr);
		NBTextMetricsBuilder_setFormatAndFontItf(&mBldr, fontItf, fontItfParam, fontSz, NBST_P(STNBColor8, 255, 255, 255, 255 ), alignH, ENNBTextCharAlignV_Base);
		if(textSz > 0){
			NBTextMetricsBuilder_appendBytes(&mBldr, text, textSz);
		} NBASSERT(textSz == NBTextMetricsBuilder_getBytesLen(&mBldr))
		NBTextMetricsBuilder_popFormat(&mBldr);
	}
	//Feed columns
	{
		NBASSERT(column.height >= 0) //Convert scene-coords (y+ up) to bitmap-coords (y+ down).
		NBTextMetrics_empty(&m);
		NBTextMetricsBuilder_feedStart(&mBldr, &m);
		NBTextMetricsBuilder_feed(&mBldr, &m, column, allowMultiline, text, textSz);
		NBTextMetricsBuilder_feedEnd(&mBldr, &m);
	}
	//Calculations
	{
		SI32 iLinea; const SI32 cntLinea = m.lines.use;
		if(cntLinea > 0){
			//Alineacion vertical
			const STNBTextMetricsLine* firstLn = NBArray_itmPtrAtIndex(&m.lines, STNBTextMetricsLine, 0);
			const STNBTextMetricsLine* lastLn = NBArray_itmPtrAtIndex(&m.lines, STNBTextMetricsLine, cntLinea - 1);
			float deltaY   = 0.0f;
			switch (alignV) {
				case ENNBTextAlignV_Base:
					deltaY = -firstLn->fontMetricsMax.ascender;
					break;
				case ENNBTextAlignV_FromBottom:
					deltaY = column.y + column.height - lastLn->yBase + lastLn->fontMetricsMax.descender - 1; //descender is neg
					break;
				case ENNBTextAlignV_Center:
					deltaY = column.y + ((column.height - (lastLn->yBase + lastLn->fontMetricsMax.descender)) * 0.5f);; //descender is neg
					break;
				default: /*ENNBTextAlignV_FromTop*/
					break;
			}
			//Lines max left/right
			float maxLeft = firstLn->visibleLeft, maxRight = firstLn->visibleRight;
			for(iLinea = 1; iLinea < cntLinea; iLinea++){
				const STNBTextMetricsLine* line = NBArray_itmPtrAtIndex(&m.lines, STNBTextMetricsLine, iLinea);
				if(maxLeft > line->visibleLeft) maxLeft = line->visibleLeft;
				if(maxRight < line->visibleRight) maxRight = line->visibleRight;
			}
			//
			float yMax	= deltaY + (lastLn->yBase - lastLn->fontMetricsMax.descender); //descender is negative
			float yMin	= deltaY;
			//
			NBAABox_wrapFirstPoint(&box, NBST_P(STNBPoint, maxLeft, yMin));
			NBAABox_wrapNextPoint(&box, NBST_P(STNBPoint, maxRight, yMax));
		}
	}
	NBTextMetrics_release(&m);
	NBTextMetricsBuilder_release(&mBldr);
	return box;
}

void NBText_appendTextWithFont(STNBText* obj, const char* text, const STNBFontMetricsIRef fontItfRef, const float defFontSz){
	NBText_appendTextWithFontItf(obj, text, &fontItfRef.itf, fontItfRef.itfParam, (fontItfRef.fontSz > 0 ? fontItfRef.fontSz : defFontSz));
}
	
void NBText_appendTextWithFontItf(STNBText* obj, const char* text, const STNBFontMetricsI* fontItf, void* fontItfParam, const float fontSz){
	const UI32 prevLen = obj->text.length;
	NBString_concat(&obj->text, text);
	//Feed formats and text
	NBTextMetricsBuilder_pushFormat(&obj->builder);
	NBTextMetricsBuilder_setFormatAndFontItf(&obj->builder, fontItf, fontItfParam, fontSz, obj->defaults.fontColor, obj->defaults.alignH, ENNBTextCharAlignV_Base);
	if(prevLen < obj->text.length){
		NBTextMetricsBuilder_appendBytes(&obj->builder, &obj->text.str[prevLen], (obj->text.length - prevLen));
	}
	NBASSERT(obj->text.length == NBTextMetricsBuilder_getBytesLen(&obj->builder))
	NBTextMetricsBuilder_popFormat(&obj->builder);
}

void NBText_appendText(STNBText* obj, const char* str){
	const UI32 prevLen = obj->text.length;
	NBString_concat(&obj->text, str);
	if(prevLen < obj->text.length){
		NBTextMetricsBuilder_appendBytes(&obj->builder, &obj->text.str[prevLen], (obj->text.length - prevLen));
	}
	NBASSERT(obj->text.length == NBTextMetricsBuilder_getBytesLen(&obj->builder))
}

void NBText_appendTextBytes(STNBText* obj, const char* str, const UI32 strSz){
	NBString_concatBytes(&obj->text, str, strSz);
	NBTextMetricsBuilder_appendBytes(&obj->builder, str, strSz);
	NBASSERT(obj->text.length == NBTextMetricsBuilder_getBytesLen(&obj->builder))
}

void NBText_syncOpenFormatRanges(STNBText* obj){
	NBASSERT(obj->text.length == NBTextMetricsBuilder_getBytesLen(&obj->builder))
	NBTextMetricsBuilder_syncOpenFormatRanges(&obj->builder);
}

//

STNBAABox NBText_metricsBox(const STNBText* obj, const BOOL includeAnyCursor){
	return NBTextMetrics_box(&obj->metrics, includeAnyCursor);
}

void NBText_organizeText(STNBText* obj, const STNBRect column, const BOOL allowMultiline){
	NBASSERT(obj->text.length == NBTextMetricsBuilder_getBytesLen(&obj->builder))
	//Feed columns
	{
		NBASSERT(column.height >= 0) //Convert scene-coords (y+ up) to bitmap-coords (y+ down).
		NBTextMetrics_empty(&obj->metrics);
		NBTextMetricsBuilder_feedStart(&obj->builder, &obj->metrics);
		NBTextMetricsBuilder_feed(&obj->builder, &obj->metrics, column, allowMultiline, obj->text.str, obj->text.length);
		NBTextMetricsBuilder_feedEnd(&obj->builder, &obj->metrics);
		//PRINTF_INFO("Metrics produced: %d chars, %d words, %d lines.\n", obj->metrics.chars.use, obj->metrics.words.use, obj->metrics.lines.use);
	}
	//Vertical align
	if(obj->metrics.lines.use > 0){
		//Alineacion vertical
		const STNBTextMetricsLine* firstLn = NBArray_itmPtrAtIndex(&obj->metrics.lines, STNBTextMetricsLine, 0);
		const STNBTextMetricsLine* lastLn = NBArray_itmPtrAtIndex(&obj->metrics.lines, STNBTextMetricsLine, obj->metrics.lines.use - 1);
		float deltaY   = 0.0f;
		switch(obj->alignV) {
			case ENNBTextAlignV_Base:
				deltaY = -firstLn->fontMetricsMax.ascender;
				break;
			case ENNBTextAlignV_FromBottom:
				deltaY = column.y + column.height - lastLn->yBase + lastLn->fontMetricsMax.descender - 1; //descender is neg
				break;
			case ENNBTextAlignV_Center:
				deltaY = column.y + ((column.height - (lastLn->yBase + lastLn->fontMetricsMax.descender)) * 0.5f);; //descender is neg
				break;
			default: /*ENNBTextAlignV_FromTop*/
				break;
		}
		//Move lines and chars
		if(deltaY != 0.0f){
			UI32 i;
			//Move base lines
			for(i = 0; i < obj->metrics.lines.use; i++){
				STNBTextMetricsLine* l = NBArray_itmPtrAtIndex(&obj->metrics.lines, STNBTextMetricsLine, i);
				l->yBase += deltaY;
			}
			//Move chars
			for(i = 0; i < obj->metrics.chars.use; i++){
				STNBTextMetricsChar* c = NBArray_itmPtrAtIndex(&obj->metrics.chars, STNBTextMetricsChar, i);
				c->pos.y += deltaY;
			}
		}
	}
	//Calculate boxes
	//NBTextMetrics_buildMultiLevelBoxes(&obj->metrics, NB_TEXT_METRICS_BUILDER_LEVELS_GROUPS_SIZE);
}

//Edit

void NBText_empty(STNBText* obj){
	//Empty
	NBTextMetricsBuilder_empty(&obj->builder);
	NBTextMetrics_empty(&obj->metrics);
	NBString_empty(&obj->text);
	NBASSERT(obj->text.length == NBTextMetricsBuilder_getBytesLen(&obj->builder))
	//Push default format
	{
		STNBTextDefFmt* def = &obj->defaults;
		NBASSERT(0 == NBTextMetricsBuilder_getActiveStackSz(&obj->builder))
		NBTextMetricsBuilder_pushFormatWithFontItf(&obj->builder, &def->fontItf, def->fontItfParam, def->fontSz, def->fontColor, def->alignH, def->alignV);
	}
}

STNBRangeI NBText_removeChars(STNBText* obj, const UI32 start, const UI32 size, STNBRangeI* dstRemovedCharsRng){
	return NBText_replaceChars(obj, start, size, NULL, 0, dstRemovedCharsRng, NULL);
}

//"NBText_organizeText" must be called before a method that requires integrity
STNBRangeI NBText_removeCharsWithoutIntegrity(STNBText* obj, const UI32 start, const UI32 size, STNBRangeI* dstRemovedCharsRng){
	return NBText_replaceCharsWithoutIntegrity(obj, start, size, NULL, 0, dstRemovedCharsRng, NULL);
}

STNBRangeI NBText_replaceChars(STNBText* obj, const UI32 start, const UI32 size, const char* newStr, const UI32 newSrtSz, STNBRangeI* dstRemovedCharsRng, STNBRangeI* dstAddedCharsRng){
	//Replace chars
	const STNBRangeI bytesRng = NBTextMetrics_replaceChars(&obj->metrics, start, size, newStr, newSrtSz, dstRemovedCharsRng, dstAddedCharsRng);
	//Remove blocks affected
	NBTextMetricsBuilder_replaceBytes(&obj->builder, bytesRng.start, bytesRng.size, newStr, newSrtSz);
	//Remove text affected
	NBString_replaceBytes(&obj->text, bytesRng.start, bytesRng.size, newStr, newSrtSz);
	NBASSERT(obj->text.length == NBTextMetricsBuilder_getBytesLen(&obj->builder))
	//Return
	return bytesRng;
}

//"NBText_organizeText" must be called before a method that requires integrity
STNBRangeI NBText_replaceCharsWithoutIntegrity(STNBText* obj, const UI32 start, const UI32 size, const char* newStr, const UI32 newSrtSz, STNBRangeI* dstRemovedCharsRng, STNBRangeI* dstAddedCharsRng){
	//Replace chars
	const STNBRangeI bytesRng = NBTextMetrics_replaceCharsSimulation(&obj->metrics, start, size, newStr, newSrtSz, dstRemovedCharsRng, dstAddedCharsRng);
	//Remove blocks affected
	NBTextMetricsBuilder_replaceBytes(&obj->builder, bytesRng.start, bytesRng.size, newStr, newSrtSz);
	//Remove text affected
	NBString_replaceBytes(&obj->text, bytesRng.start, bytesRng.size, newStr, newSrtSz);
	NBASSERT(obj->text.length == NBTextMetricsBuilder_getBytesLen(&obj->builder))
	//Return
	return bytesRng;
}

//Debug

#ifdef NB_CONFIG_INCLUDE_ASSERTS
#include <stdlib.h>		//for rand()
#endif

#ifdef NB_CONFIG_INCLUDE_ASSERTS
void NBText_dbgTestSequences(const STNBText* obj, const UI8 printLevel){
	NBTextMetrics_dbgTestAllSeqs(&obj->metrics, obj->text.str, obj->text.length, printLevel);
	NBTextMetrics_dbgTestRangesQueries(&obj->metrics, obj->text.str, obj->text.length, printLevel);
	NBTextMetricsBuilder_dbgTestAllSeqs(&obj->builder, obj->text.length, printLevel);
}
#endif

#ifdef NB_CONFIG_INCLUDE_ASSERTS
void NBText_dbgTestRemoveAndReplace(const UI32 totalTests, const UI32 ciclesPerTest, STNBFontMetricsIRef* fonts, const UI32 fontsSz, const UI8 printLevel){
	NBASSERT(fonts != NULL && fontsSz > 0)
	if(fonts != NULL && fontsSz > 0){
		UI32 i, j;
		STNBText textTest;
		STNBString strTest, str2, str3;
		NBText_init(&textTest);
		NBString_init(&strTest);
		NBString_init(&str2);
		NBString_init(&str3);
		const UI32 origMaxLen = 512;
		const UI32 replcMaxLen = 512;
		const float colWidth = 256.0f, colHeight = 999999.0f;
		const char* charsPosibs = "0123456789 \n";
		const UI32 charsPosibsSz = NBString_strLenBytes(charsPosibs);
		const char* charsPosibs2 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ \n";
		const UI32 charsPosibs2Sz = NBString_strLenBytes(charsPosibs2);
		//Tests
		for(i = 0; i < totalTests; i++){
			//Random reset string (to reset buffer)
			if((rand() % 100) < 2){
				NBText_release(&textTest);
				NBText_init(&textTest);
			}
			//Set initial value
			{
				UI32 i; const UI32 charsCount = (rand() % origMaxLen);
				NBText_empty(&textTest);
				NBString_empty(&strTest);
				if(charsCount == 0){
					//Must have a root format
					NBText_pushFormatWithFont(&textTest, fonts[(rand() % fontsSz)], 0, NBST_P(STNBColor8, 255, 255, 255, 255 ), ENNBTextLineAlignH_Left, ENNBTextCharAlignV_Base);
				} else {
					//Generate content
					for(i = 0; i < charsCount; i++){
						const char newC = charsPosibs[(rand() % charsPosibsSz)];
						//Push format
						if(i == 0 || (rand() % 10) < 1){
							NBText_pushFormatWithFont(&textTest, fonts[(rand() % fontsSz)], 0, NBST_P(STNBColor8, 255, 255, 255, 255 ), ENNBTextLineAlignH_Left, ENNBTextCharAlignV_Base);
						}
						//Add char
						NBString_concatByte(&strTest, newC);
						NBText_appendTextBytes(&textTest, &newC, 1);
						//Pop format
						if((rand() % 10) < 1){
							NBText_popFormat(&textTest);
						}
					}
				}
				NBText_organizeText(&textTest, NBST_P(STNBRect, 0, 0, colWidth, colHeight), TRUE);
				//Verify initial integrity
				NBText_dbgTestSequences(&textTest, 0);
				//Compare
				{
					UI32 textLen = 0;
					const char* text = NBText_getText(&textTest, &textLen);
					NBASSERT(textLen == strTest.length)
					NBASSERT(NBString_strIsEqual(text, strTest.str));
				}
			}
			//ciclesPerTest
			for(j = 0; j < ciclesPerTest; j++){
				UI32 orgLen = 0;
				const char* orgText = NBText_getText(&textTest, & orgLen);
				if((rand() % 2) == 0){
					//Test remove
					UI32 start = 0, size = 0;
					if(orgLen > 0){
						start = (rand() % orgLen) + (rand() % ((orgLen < 5 ? 5 : orgLen) / 5));
						size = (rand() % (orgLen * 5) / 4);
					} else {
						start = (rand() % 3);
						size = (rand() % 3);
					}
					//Build compare
					{
						NBString_empty(&str2);
						if(start > 0){
							NBString_concatBytes(&str2, &orgText[0], (start < orgLen ? start : orgLen));
						}
						if((start + size) < orgLen){
							NBString_concatBytes(&str2, &orgText[start + size], orgLen - start - size);
						}
					}
					//Replace
					if(printLevel > 1){
						PRINTF_INFO("-------------\n");
						PRINTF_INFO("Orig: %s.\n", orgText);
						PRINTF_INFO("Epct: %s.\n", str2.str);
					}
					if(printLevel > 0){
						PRINTF_INFO("#%d-test-#%d-cicle REMOVE(%d, +%d) on %d bytes%s%s%s (%d chars, %d words, %d lines).\n", (i + 1), (j + 1), start, size, orgLen, (start >= orgLen ? " start-out-of-rng" : ""), (start + size) == orgLen ? " remove-at-end" : "", (start + size) > orgLen ? " size-out-of-rng" : "", textTest.metrics.chars.use, textTest.metrics.words.use, textTest.metrics.lines.use);
					}
					//----------------
					//- Action
					//----------------
					NBText_removeChars(&textTest, start, size, NULL); //assuming one-byte chars
					//----------------
					if(printLevel > 1){
						const char* result = NBText_getText(&textTest, NULL);
						PRINTF_INFO("Rslt: %s.\n", result);
					}
					//Compare
					{
						UI32 textLen = 0;
						const char* text = NBText_getText(&textTest, &textLen);
						NBASSERT(textLen == str2.length)
						NBASSERT(NBString_strIsEqual(text, str2.str));
					}
					//Verify integrity
					NBText_dbgTestSequences(&textTest, printLevel);
				} else {
					//Test replace
					UI32 start = 0, size = 0;
					if(orgLen > 0){
						start = (rand() % orgLen) + (rand() % ((orgLen < 5 ? 5 : orgLen) / 5));
						size = (rand() % (orgLen * 5) / 4);
					} else {
						start = (rand() % 3);
						size = (rand() % 3);
					}
					//Set replace value
					{
						UI32 i; const UI32 charsCount = (rand() % replcMaxLen);
						NBString_empty(&str3);
						for(i = 0; i < charsCount; i++){
							NBString_concatByte(&str3, charsPosibs2[(rand() % charsPosibs2Sz)]);
						}
					}
					//Build compare
					{
						NBString_empty(&str2);
						if(start > 0){
							NBString_concatBytes(&str2, &orgText[0], (start < orgLen ? start : orgLen));
						}
						NBString_concatBytes(&str2, str3.str, str3.length);
						if((start + size) < orgLen){
							NBString_concatBytes(&str2, &orgText[start + size], orgLen - start - size);
						}
					}
					//Replace
					if(printLevel > 1){
						PRINTF_INFO("-------------\n");
						PRINTF_INFO("Rplc: %s.\n", str3.str);
						PRINTF_INFO("Orig: %s.\n", orgText);
						PRINTF_INFO("Epct: %s.\n", str2.str);
					}
					if(printLevel > 0){
						PRINTF_INFO("#%d-test-#%d-cicle REPLACE(%d, +%d, with +%d [%s]) on %d bytes%s%s%s (%d chars, %d words, %d lines).\n", (i + 1), (j + 1), start, size, str3.length, (str3.length < size ? "reduce" : str3.length > size ? "growth" : "same"), orgLen, (start >= orgLen ? " start-out-of-rng" : ""), (start + size) == orgLen ? " remove-at-end" : "", (start + size) > orgLen ? " size-out-of-rng" : "", textTest.metrics.chars.use, textTest.metrics.words.use, textTest.metrics.lines.use);
					}
					//----------------
					//- Action
					//----------------
					NBText_replaceChars(&textTest, start, size, str3.str, str3.length, NULL, NULL);
					//----------------
					//
					if(printLevel > 1){
						const char* result = NBText_getText(&textTest, NULL);
						PRINTF_INFO("Rslt: %s.\n", result);
					}
					//Compare
					{
						UI32 textLen = 0;
						const char* text = NBText_getText(&textTest, &textLen);
						NBASSERT(textLen == str2.length)
						NBASSERT(NBString_strIsEqual(text, str2.str));
					}
					//Verify integrity
					NBText_dbgTestSequences(&textTest, printLevel);
				}
			}
		}
		NBString_release(&str3);
		NBString_release(&str2);
		NBString_release(&strTest);
		NBText_release(&textTest);
	}
}
#endif
