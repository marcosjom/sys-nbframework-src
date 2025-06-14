//
//  NBFontMetricsResults.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/12/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/fonts/NBTextMetrics.h"
#include "nb/core/NBEncoding.h"

void NBTextMetrics_init(STNBTextMetrics* obj){
	NBMemory_setZeroSt(*obj, STNBTextMetrics);
	NBArray_init(&obj->chars, sizeof(STNBTextMetricsChar), NULL);
	NBArray_init(&obj->words, sizeof(STNBTextMetricsWord), NULL);
	NBArray_init(&obj->lines, sizeof(STNBTextMetricsLine), NULL);
	//Optimizations boxes
	/*NBArray_init(&obj->boxes, sizeof(STNBTextMetricsBox), NULL);
	NBArray_init(&obj->boxesLvls, sizeof(STNBRangeI), NULL);
	obj->boxesGrpsSz = 0;*/
}

void NBTextMetrics_initWithOther(STNBTextMetrics* obj, const STNBTextMetrics* other){
	NBMemory_setZeroSt(*obj, STNBTextMetrics);
	NBArray_initWithOther(&obj->chars, &other->chars);
	NBArray_initWithOther(&obj->words, &other->words);
	NBArray_initWithOther(&obj->lines, &other->lines);
	//Optimizations boxes
	/*NBArray_initWithOther(&obj->boxes, &other->boxes);
	NBArray_initWithOther(&obj->boxesLvls, &other->boxesLvls);
	obj->boxesGrpsSz = other->boxesGrpsSz;*/
	//Defaults values (when empty text)
	obj->defaults	= other->defaults;
}

void NBTextMetrics_release(STNBTextMetrics* obj){
	NBArray_release(&obj->chars);
	NBArray_release(&obj->words);
	NBArray_release(&obj->lines);
	//Optimizations boxes
	/*NBArray_release(&obj->boxes);
	NBArray_release(&obj->boxesLvls);
	obj->boxesGrpsSz = 0;*/
}

// Sync

void NBTextMetrics_syncDataWithOther(STNBTextMetrics* obj, const STNBTextMetrics* other){
	if(obj != other){
		//Empty
		{
			NBArray_empty(&obj->chars);
			NBArray_empty(&obj->words);
			NBArray_empty(&obj->lines);
			//Optimizations boxes
			/*NBArray_empty(&obj->boxes);
			NBArray_empty(&obj->boxesLvls);
			obj->boxesGrpsSz = 0;*/
		}
		//Clone
		{
			NBArray_addItems(&obj->chars, NBArray_data(&other->chars), sizeof(STNBTextMetricsChar), other->chars.use);
			NBArray_addItems(&obj->words, NBArray_data(&other->words), sizeof(STNBTextMetricsWord), other->words.use);
			NBArray_addItems(&obj->lines, NBArray_data(&other->lines), sizeof(STNBTextMetricsLine), other->lines.use);
			//Optimizations boxes
			/*NBArray_addItems(&obj->boxes, NBArray_data(&other->boxes), sizeof(STNBTextMetricsBox), other->boxes.use);
			NBArray_addItems(&obj->boxesLvls, NBArray_data(&other->boxesLvls), sizeof(STNBRangeI), other->boxesLvls.use);
			obj->boxesGrpsSz = other->boxesGrpsSz;*/
		}
		//
		obj->defaults = other->defaults;
	}
}

//

void NBTextMetrics_empty(STNBTextMetrics* obj){
	NBArray_empty(&obj->chars);
	NBArray_empty(&obj->words);
	NBArray_empty(&obj->lines);
	//Optimizations boxes
	/*NBArray_empty(&obj->boxes);
	NBArray_empty(&obj->boxesLvls);
	obj->boxesGrpsSz = 0;*/
}

void NBTextMetrics_setDefaults(STNBTextMetrics* obj, const STNBFontMetrics* defFontMetrics, const STNBPoint* defStartPos){
	NBTextMetrics_setDefaultFontMetrics(obj, defFontMetrics);
	NBTextMetrics_setDefaultStartPos(obj, defStartPos);
}

void NBTextMetrics_setDefaultFontMetrics(STNBTextMetrics* obj, const STNBFontMetrics* defFontMetrics){
	if(defFontMetrics == NULL){
		NBMemory_setZeroSt(obj->defaults.fontMetrics, STNBFontMetrics);
	} else {
		obj->defaults.fontMetrics = *defFontMetrics;
	}
}

void NBTextMetrics_setDefaultStartPos(STNBTextMetrics* obj, const STNBPoint* defStartPos){
	if(defStartPos == NULL){
		NBMemory_setZeroSt(obj->defaults.start, STNBPoint);
	} else {
		obj->defaults.start = *defStartPos;
	}
}

//Box

STNBAABox NBTextMetrics_box(const STNBTextMetrics* obj, const BOOL includeAnyCursor){
	STNBAABox r;
	NBAABox_init(&r);
	/*if(obj->boxes.use > 0 && obj->boxesLvls.use > 0){
		//Use boxes in the higher level (optimization)
		const STNBRangeI higherLvlRng = NBArray_itmValueAtIndex(&obj->boxesLvls, STNBRangeI, obj->boxesLvls.use - 1);
		SI32 iFirst, iEndAfter;
		NB_TEXT_METRICS_LINES_INDEXES_BY_RANGE(obj, higherLvlRng, iFirst, iEndAfter);
		{
			UI32 boxesAdded = 0;
			SI32 i; for(i = iFirst; i < iEndAfter; i++){
				const STNBTextMetricsBox box = NBArray_itmValueAtIndex(&obj->boxes, STNBTextMetricsBox, i);
				if(boxesAdded <= 0){
					NBAABox_wrapFirstPoint(&r, NBST_P(STNBPoint, box.box.xMin, box.box.yMin));
					NBAABox_wrapNextPoint(&r, NBST_P(STNBPoint, box.box.xMax, box.box.yMax));
				} else {
					NBAABox_wrapNextPoint(&r, NBST_P(STNBPoint, box.box.xMin, box.box.yMin));
					NBAABox_wrapNextPoint(&r, NBST_P(STNBPoint, box.box.xMax, box.box.yMax));
				}
				boxesAdded++;
			}
		}
		if(includeAnyCursor){
			r.xMax += 2.0f;
		}
	} else*/ if(obj->lines.use > 0){
		//Use lines boxes
		const STNBTextMetricsLine* line = NBArray_itmPtrAtIndex(&obj->lines, STNBTextMetricsLine, 0);
		NBAABox_wrapFirstPoint(&r, NBST_P(STNBPoint, line->visibleLeft, line->yBase - line->fontMetricsMax.ascender));
		NBAABox_wrapNextPoint(&r, NBST_P(STNBPoint, line->visibleRight, line->yBase - line->fontMetricsMax.descender)); //descender is neg
		UI32 i; for(i = 1; i < obj->lines.use; i++){
			line = NBArray_itmPtrAtIndex(&obj->lines, STNBTextMetricsLine, i);
			const float yTop = line->yBase - line->fontMetricsMax.ascender;
			const float yBtm = line->yBase - line->fontMetricsMax.descender; //descender is neg
			NBAABox_wrapNextPoint(&r, NBST_P(STNBPoint, line->visibleRight, yTop));
			NBAABox_wrapNextPoint(&r, NBST_P(STNBPoint, line->visibleLeft, yBtm));
		}
		if(includeAnyCursor){
			r.xMax += 2.0f;
		}
	} else if(includeAnyCursor){
		//Only cursor (no text)
		r.xMin		= obj->defaults.start.x;
		r.xMax		= obj->defaults.start.x + 2.0f;
		r.yMin		= obj->defaults.start.y;
		r.yMax		= obj->defaults.start.y + obj->defaults.fontMetrics.ascender - obj->defaults.fontMetrics.descender; //descender is negative
		NBASSERT(r.xMin < r.xMax) //Must include cursor
		NBASSERT(r.yMin < r.yMax) //Must include cursor (obj->defaults.fontMetrics were set?)
	}
	//Not valid when text are only white-spaces
	NBASSERT(obj->chars.use == 0 || (r.xMin < r.xMax && r.yMin < r.yMax))
	return r;
}

/*
void NBTextMetrics_buildMultiLevelBoxes(STNBTextMetrics* obj, const UI32 sizePerGrp){
	//Empty
	{
		NBArray_empty(&obj->boxes);
		NBArray_empty(&obj->boxesLvls);
		obj->boxesGrpsSz = sizePerGrp;
	}
	//Create groups
	if(obj->lines.use > 0){
		//Add grouped lines
		{
			STNBTextMetricsBox curGrp;
			NBMemory_setZeroSt(curGrp, STNBTextMetricsBox);
			NBAABox_init(&curGrp.box);
			curGrp.rng.start	= 0;
			curGrp.rng.size		= 0;
			UI32 i; for(i = 0; i < obj->lines.use; i++){
				const STNBTextMetricsLine* line = NBArray_itmPtrAtIndex(&obj->lines, STNBTextMetricsLine, i);
				const float yTop = line->yBase - line->fontMetricsMax.ascender;
				const float yBtm = line->yBase - line->fontMetricsMax.descender; //descender is neg
				//PRINTF_INFO("Line box: (%f, %f)-(+%f, +%f).\n", line->visibleLeft, yTop, (line->visibleRight - line->visibleLeft), (yBtm - yTop));
				//Inflate current group box
				if(curGrp.rng.size <= 0){
					NBAABox_wrapFirstPoint(&curGrp.box, NBST_P(STNBPoint, line->visibleLeft, yTop));
					NBAABox_wrapNextPoint(&curGrp.box, NBST_P(STNBPoint, line->visibleRight, yBtm));
				} else {
					NBAABox_wrapNextPoint(&curGrp.box, NBST_P(STNBPoint, line->visibleLeft, yTop));
					NBAABox_wrapNextPoint(&curGrp.box, NBST_P(STNBPoint, line->visibleRight, yBtm));
				}
				curGrp.rng.size++;
				//Start new group
				if(curGrp.rng.size >= sizePerGrp){
					NBArray_addValue(&obj->boxes, curGrp);
					//PRINTF_INFO("NBTextMetrics closed group (%d lines)-(%f, %f)-(+%f, +%f).\n", curGrp.rng.size, curGrp.box.xMin, curGrp.box.yMin, (curGrp.box.xMax - curGrp.box.xMin), (curGrp.box.yMax - curGrp.box.yMin));
					//Reset
					NBMemory_setZeroSt(curGrp, STNBTextMetricsBox);
					NBAABox_init(&curGrp.box);
					curGrp.rng.start	= i;
					curGrp.rng.size		= 0;
				}
			}
			//Add remaining group
			if(curGrp.rng.size > 0){
				NBArray_addValue(&obj->boxes, curGrp);
				//PRINTF_INFO("NBTextMetrics closed final group (%d lines)-(%f, %f)-(+%f, +%f).\n", curGrp.rng.size, curGrp.box.xMin, curGrp.box.yMin, (curGrp.box.xMax - curGrp.box.xMin), (curGrp.box.yMax - curGrp.box.yMin));
			}
		}
		//Add level zero (grouped-lines boxes)
		if(obj->boxes.use > 0){
			STNBRangeI lvlRng;
			lvlRng.start	= 0;
			lvlRng.size		= (obj->boxes.use - lvlRng.start);
			NBArray_addValue(&obj->boxesLvls, lvlRng);
			//PRINTF_INFO("NBTextMetrics added level-#%d (%d groups).\n", obj->boxesLvls.use, lvlRng.size);
			//Add sub-levels
			if(sizePerGrp > 1){
				while(lvlRng.size > sizePerGrp){
					//Build new level grpup
					STNBTextMetricsBox curGrp;
					NBMemory_setZeroSt(curGrp, STNBTextMetricsBox);
					NBAABox_init(&curGrp.box);
					curGrp.rng.start	= lvlRng.start;
					curGrp.rng.size		= 0;
					{
						const SI32 iEndAfter = lvlRng.start + lvlRng.size;
						SI32 i; for(i = lvlRng.start; i < iEndAfter; i++){
							const STNBTextMetricsBox grpBox = NBArray_itmValueAtIndex(&obj->boxes, STNBTextMetricsBox, i);
							//Inflate current group box
							if(curGrp.rng.size <= 0){
								NBAABox_wrapFirstPoint(&curGrp.box, NBST_P(STNBPoint, grpBox.box.xMin, grpBox.box.yMin));
								NBAABox_wrapNextPoint(&curGrp.box, NBST_P(STNBPoint, grpBox.box.xMax, grpBox.box.yMax));
							} else {
								NBAABox_wrapNextPoint(&curGrp.box, NBST_P(STNBPoint, grpBox.box.xMin, grpBox.box.yMin));
								NBAABox_wrapNextPoint(&curGrp.box, NBST_P(STNBPoint, grpBox.box.xMax, grpBox.box.yMax));
							}
							curGrp.rng.size++;
							//Start new group
							if(curGrp.rng.size >= sizePerGrp){
								NBArray_addValue(&obj->boxes, curGrp);
								//PRINTF_INFO("NBTextMetrics closed group (%d subgroups, level-#%d).\n", curGrp.rng.size, (obj->boxesLvls.use + 1));
								//Reset
								NBMemory_setZeroSt(curGrp, STNBTextMetricsBox);
								NBAABox_init(&curGrp.box);
								curGrp.rng.start	= i;
								curGrp.rng.size		= 0;
							}
						}
						//Add remaining group
						if(curGrp.rng.size > 0){
							NBArray_addValue(&obj->boxes, curGrp);
							//PRINTF_INFO("NBTextMetrics closed final group (%d subgroups, level-#%d).\n", curGrp.rng.size, (obj->boxesLvls.use + 1));
						}
					}
					//Add sublevel
					lvlRng.start	= (lvlRng.start + lvlRng.size);
					lvlRng.size		= (obj->boxes.use - lvlRng.start);
					NBArray_addValue(&obj->boxesLvls, lvlRng);
					//PRINTF_INFO("NBTextMetrics added level-#%d (%d groups).\n", obj->boxesLvls.use, lvlRng.size);
				}
			}
		}
	}
}
*/

//Searches

STNBRangeI NBTextMetrics_charsRangeToBytesRange(const STNBTextMetrics* obj, const UI32 start, const UI32 size){
	STNBRangeI r; r.size = 0;
	if(obj->chars.use <= 0){
		r.start = 0;
	} else if(start < obj->chars.use){
		const UI32 iCharAfter = (start + size);
		const STNBTextMetricsChar* c0 = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, start);
		r.start = c0->iByte;
		if(iCharAfter < obj->chars.use){
			const STNBTextMetricsChar* c1 = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, iCharAfter);
			r.size = (c1->iByte - r.start);
		} else {
			const STNBTextMetricsChar* cLast = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, obj->chars.use - 1);
			r.size = (cLast->iByte + cLast->bytesLen - r.start);
		}
	} else {
		NBASSERT(obj->words.use > 0)
		const STNBTextMetricsChar* cLast = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, obj->chars.use - 1);
		r.start = (cLast->iByte + cLast->bytesLen);
	}
	return r;
}

STNBRangeI NBTextMetrics_bytesRangeToCharsRange(const STNBTextMetrics* obj, const UI32 start, const UI32 size){
	STNBRangeI r; r.size = 0;
	if(obj->chars.use <= 0){
		r.start = 0;
	} else {
		const STNBTextMetricsChar* cLast = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, obj->chars.use - 1);
		const UI32 bytesTotal = (cLast->iByte + cLast->bytesLen);
		if(start < bytesTotal){
			const UI32 iByteAfter = (start + size);
			//Binary search for start-char
			{
				UI32 posStart	= 0;
				UI32 posMidd	= 0;
				UI32 posEnd		= (obj->chars.use - 1);
				const STNBTextMetricsChar* cMidd = NULL;
				while(posStart <= posEnd){
					posMidd		= posStart + ((posEnd - posStart)/2);
					cMidd		= NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, posMidd);
					if(cMidd->iByte == start){
						break; //cMidd is the start-char
					} else {
						if(start < cMidd->iByte){
							posEnd		= posMidd - 1;
						} else {
							posStart	= posMidd + 1;
						}
					}
				}
				//Move to the prev char (if necesary)
				if(posMidd > 0){
					while(cMidd->iByte > start){
						posMidd--; NBASSERT(cMidd != NULL)
						cMidd = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, posMidd);
						if(posMidd == 0){
							break;
						}
					}
				}
				NBASSERT(cMidd->iByte <= start)
				//Move to the leftest char (in case mutiples chars starts from the same byte-pos)
				while(posMidd > 0) {
					const STNBTextMetricsChar* c = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, posMidd - 1);
					if(c->iByte == cMidd->iByte){
						cMidd = c;
						posMidd--;
					} else {
						break;
					}
				} NBASSERT(cMidd->iByte <= start)
				r.start = posMidd;
			}
			//Define size
			if(size <= 0){
				r.size = 0;
			} else if(iByteAfter >= bytesTotal){
				r.size = (obj->chars.use - r.start);
			} else {
				NBASSERT(size > 0)
				//Binary search for after-char
				{
					UI32 posStart	= r.start;
					UI32 posMidd	= 0;
					UI32 posEnd		= (obj->chars.use - 1);
					const STNBTextMetricsChar* cMidd = NULL;
					while(posStart <= posEnd){
						posMidd		= posStart + ((posEnd - posStart)/2);
						cMidd		= NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, posMidd);
						if(cMidd->iByte == iByteAfter){
							break; //cMidd is the start-char
						} else {
							if(iByteAfter < cMidd->iByte){
								posEnd		= posMidd - 1;
							} else {
								posStart	= posMidd + 1;
							}
						}
					}
					//Move to the prev char (if necesary)
					if(posMidd > 0){
						while(cMidd->iByte > iByteAfter){
							posMidd--; NBASSERT(cMidd != NULL)
							cMidd = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, posMidd);
							if(posMidd == 0){
								break;
							}
						}
					}
					//Move to the next char (if necesary)
					if (cMidd != NULL) {
						while (cMidd->iByte < iByteAfter) {
							posMidd++;
							NBASSERT(cMidd != NULL)
								if (posMidd < obj->chars.use) {
									cMidd = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, posMidd);
								} else {
									cMidd = NULL;
									break;
								}
						}
					}
					//Move to the leftest char (in case mutiples chars starts from the same byte-pos)
					if(posMidd < obj->chars.use){
						NBASSERT(cMidd != NULL)
						if (cMidd != NULL) {
							while (posMidd > 0) {
								const STNBTextMetricsChar* c = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, posMidd - 1);
								if (c->iByte == cMidd->iByte) {
									cMidd = c;
									posMidd--;
								} else {
									break;
								}
							}
						}
					}
					NBASSERT(posMidd >= r.start)
					NBASSERT(posMidd <= obj->chars.use)
					r.size = (posMidd - r.start);
					NBASSERT(r.size != 0 || size == 0)
				}
			}
		} else {
			r.start = obj->chars.use;
		}
	}
	NBASSERT(r.start <= obj->chars.use)
	NBASSERT((r.start + r.size) <= obj->chars.use)
	return r;
}

const STNBTextMetricsLine* NBTextMetrics_lineByCharIndex(const STNBTextMetrics* obj, const UI32 iChar){
	const STNBTextMetricsLine* r = NULL;
	if(obj->lines.use > 0 && iChar < obj->chars.use){
		//Binary search (lines are ordered)
		SI32 posEnd		= (obj->lines.use - 1);
		SI32 posStart	= 0;
		SI32 posMidd;
		const STNBTextMetricsLine* dataMidd = NULL;
		const STNBTextMetricsWord* w0 = NULL;
		const STNBTextMetricsWord* w1 = NULL;
		while(posStart <= posEnd){
			posMidd		= posStart + ((posEnd - posStart)/2);
			dataMidd	= NBArray_itmPtrAtIndex(&obj->lines, STNBTextMetricsLine, posMidd);
			w0			= NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, dataMidd->rngWords.start);
			w1			= NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, dataMidd->rngWords.afterEnd - 1);
			//Into range
			if(w0->rngChars.start <= iChar && iChar < w1->rngChars.afterEnd){
				r = dataMidd;
				break;
			} else {
				if(iChar < w0->rngChars.start){
					posEnd		= posMidd - 1;
				} else {
					posStart	= posMidd + 1;
				}
			}
		}
		NBASSERT(r != NULL) //Must exists
	}
	return r;
}

const STNBTextMetricsLine* NBTextMetrics_lineByCoord(const STNBTextMetrics* obj, const float y){
	const STNBTextMetricsLine* r = NULL;
	if(obj->lines.use > 0){
		SI32 i = 0; STNBTextMetricsLine* ln = NULL;
		//Set first not-empty-line
		for(; i < obj->lines.use; i++){
			ln = NBArray_itmPtrAtIndex(&obj->lines, STNBTextMetricsLine, i);
			if(ln->rngWords.start < ln->rngWords.afterEnd){ //not-empty
				r = ln;
				break;
			}
		}
		//Seq-search
		for(; i < obj->lines.use; i++){
			ln = NBArray_itmPtrAtIndex(&obj->lines, STNBTextMetricsLine, i);
			if(ln->rngWords.start < ln->rngWords.afterEnd){ //not-empty
				if(y < (ln->yBase - ln->fontMetricsMax.ascender)){
					break;
				}
				r = ln;
			}
		}
	}
	return r;
}

const STNBTextMetricsWord* NBTextMetrics_wordByCharIndex(const STNBTextMetrics* obj, const UI32 iChar){
	const STNBTextMetricsWord* r = NULL;
	if(obj->words.use > 0 && iChar < obj->chars.use){
		//Binary search (words are ordered)
		SI32 posEnd		= (obj->words.use - 1);
		SI32 posStart	= 0;
		SI32 posMidd;
		const STNBTextMetricsWord* dataMidd = NULL;
		while(posStart <= posEnd){
			posMidd		= posStart + ((posEnd - posStart)/2);
			dataMidd	= NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, posMidd);
			//Into range
			if(dataMidd->rngChars.start <= iChar && iChar < dataMidd->rngChars.afterEnd){
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
	return r;
}

const STNBTextMetricsChar* NBTextMetrics_charAtLineByCoord(const STNBTextMetrics* obj, const STNBTextMetricsLine* ln, const float x){
	const STNBTextMetricsChar* r = NULL;
	const STNBTextMetricsLine* first = NBArray_dataPtr(&obj->lines, STNBTextMetricsLine);
	if(ln >= first && ln < (first + obj->lines.use)){
		if(ln->rngWords.start < ln->rngWords.afterEnd){
			STNBTextMetricsWord* w0 = NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, ln->rngWords.start);
			STNBTextMetricsWord* w1 = NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, ln->rngWords.afterEnd - 1);
			STNBTextMetricsChar* c = NULL;
			SI32 i = w0->rngChars.start;
			const SI32 iAfterEnd = (w1->rngChars.afterEnd < obj->chars.use ? w1->rngChars.afterEnd : obj->chars.use);
			//Set first char
			r = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, i++);
			//Seq-search
			for(; i < iAfterEnd; i++){
				c = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, i);
				if(x < (c->pos.x - c->extendsLeft)){
					break;
				}
				r = c;
			}
		}
	}
	return r;
}

//Text geometry

STNBRect NBTextMetrics_rectForCursor(const STNBTextMetrics* obj, const UI32 iChar){
	STNBRect r;
	NBMemory_setZeroSt(r, STNBRect);
	if(iChar >= obj->chars.use){
		NBASSERT((obj->lines.use <= 0 && obj->words.use <= 0 && obj->chars.use <= 0) || (obj->lines.use > 0 && obj->words.use > 0 && obj->chars.use > 0))
		if(obj->chars.use <= 0 || obj->lines.use <= 0){
			//Empty cursor
			r.x			= obj->defaults.start.x;
			r.y			= obj->defaults.start.y;
			r.width 	= 2.0f;
			r.height	= (obj->defaults.fontMetrics.ascender - obj->defaults.fontMetrics.descender); //descender is negative
		} else {
			//Default cursor (after last char)
			const STNBTextMetricsLine* ln = NBArray_itmPtrAtIndex(&obj->lines, STNBTextMetricsLine, obj->lines.use - 1);
			r.x			= ln->visibleRight;
			r.y			= ln->yBase - ln->fontMetricsMax.ascender;
			r.width 	= 2.0f;
			r.height	= (ln->fontMetricsMax.ascender - ln->fontMetricsMax.descender); //descender is negative
		}
	} else {
		//Determine char's line
		const STNBTextMetricsChar* c = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, iChar);
		const STNBTextMetricsLine* ln = NULL;
		{
			UI32 i; for(i = 0; i < obj->lines.use; i++){
				const STNBTextMetricsLine* ln2 = NBArray_itmPtrAtIndex(&obj->lines, STNBTextMetricsLine, i);
				if(ln2->rngWords.start < ln2->rngWords.afterEnd){ //Not empty-line
					const STNBTextMetricsWord* w0 = NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, ln2->rngWords.start);
					const STNBTextMetricsWord* w1 = NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, ln2->rngWords.afterEnd - 1);
					//PRINTF_INFO("Comparing with line #%d (%d vs %d -> %d).\n", (i + 1), iChar, w0->rngChars.start, (w1->rngChars.afterEnd - 1));
					if(iChar >= w0->rngChars.start && iChar < w1->rngChars.afterEnd){
						ln = ln2;
						break;
					}
				}
			} NBASSERT(i < obj->lines.use) //A line must be found
		}
		NBASSERT(ln != NULL)
		if (ln != NULL) {
			r.x = c->pos.x - c->extendsLeft;
			r.y = ln->yBase - ln->fontMetricsMax.ascender;
			r.width = 2.0f;
			r.height = (ln->fontMetricsMax.ascender - ln->fontMetricsMax.descender); //descender is negative
		}
	}
	return r;
}

STNBRect NBTextMetrics_firstRectForCharsRange(const STNBTextMetrics* obj, const UI32 start, const UI32 size){
	STNBRect r = { 0, 0, 0, 0 };
	if(obj->chars.use <= 0){
		//Empty cursor
		r.x			= obj->defaults.start.x;
		r.y			= obj->defaults.start.y;
		r.width 	= 2.0f;
		r.height	= (obj->defaults.fontMetrics.ascender - obj->defaults.fontMetrics.descender); //descender is negative
	} else if(start >= obj->chars.use){
		//Default cursor (after last char)
		NBASSERT(obj->lines.use > 0 && obj->words.use > 0 && obj->chars.use > 0)
		const STNBTextMetricsLine* ln = NBArray_itmPtrAtIndex(&obj->lines, STNBTextMetricsLine, obj->lines.use - 1);
		r.x			= ln->visibleRight;
		r.y			= ln->yBase - ln->fontMetricsMax.ascender;
		r.width 	= 2.0f;
		r.height	= (ln->fontMetricsMax.ascender - ln->fontMetricsMax.descender); //descender is negative
	} else {
		const STNBTextMetricsLine* ln = NBTextMetrics_lineByCharIndex(obj, start);
		NBASSERT(ln != NULL)
		if(ln != NULL){
			const UI32 afterEnd = start + size;
			const STNBTextMetricsLine* lnFirst = NBArray_dataPtr(&obj->lines, STNBTextMetricsLine);
			const STNBTextMetricsLine* lnAfterEnd = lnFirst + obj->lines.use;
			//PRINTF_INFO("First rect is at line #%d of %d.\n", (UI32)(ln - lnFirst + 1), obj->lines.use);
			NBASSERT(NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, ln->rngWords.start)->rngChars.start <= start)
			while(ln < lnAfterEnd){
				if(ln->rngWords.start < ln->rngWords.afterEnd){ //not-empty
					const STNBTextMetricsWord* w0 = NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, ln->rngWords.start);
					if(afterEnd <= w0->rngChars.start){
						//PRINTF_INFO("Stopping at line #%d of %d.\n", (UI32)(ln - lnFirst + 1), obj->lines.use);
						//Range ends before this line
						break;
					} else {
						//Range contains this line
						r.y			= ln->yBase - ln->fontMetricsMax.ascender;
						r.height	= (ln->fontMetricsMax.ascender - ln->fontMetricsMax.descender); //descender is negative
						//Set rect's left
						{
							if(start <= w0->rngChars.start){
								//Line's range starts AT line
								r.x = ln->visibleLeft;
							} else {
								//Line's range starts INSIDE line
								const STNBTextMetricsChar* c = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, start);
								r.x = c->pos.x - c->extendsLeft;
							}
						}
						//Set rect's right
						{
							const STNBTextMetricsWord* w1 = NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, ln->rngWords.afterEnd - 1);
							NBASSERT(start < w1->rngChars.afterEnd)
							if(afterEnd >= w1->rngChars.afterEnd){
								//Line's range ends AT line
								r.width = ln->visibleRight - r.x;
							} else {
								//Line's range ends INSIDE line
								const STNBTextMetricsChar* c = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, afterEnd);
								r.width = (c->pos.x - c->extendsLeft) - r.x;
							}
						}
						//Only one rect
						break;
					}
				}
				ln++;
			}
		}
		NBASSERT(r.height > 0)
	}
	return r;
}

void NBTextMetrics_rectsForCharsRange(const STNBTextMetrics* obj, const UI32 start, const UI32 size, STNBArray* dstRects){
	if(obj->chars.use <= 0){
		//Empty cursor
		if(dstRects != NULL){
			STNBRect r;
			r.x			= obj->defaults.start.x;
			r.y			= obj->defaults.start.y;
			r.width 	= 2.0f;
			r.height	= (obj->defaults.fontMetrics.ascender - obj->defaults.fontMetrics.descender); //descender is negative
			NBArray_addValue(dstRects, r);
		}
	} else if(start >= obj->chars.use){
		//Default cursor (after last char)
		NBASSERT(obj->lines.use > 0 && obj->words.use > 0 && obj->chars.use > 0)
		const STNBTextMetricsLine* ln = NBArray_itmPtrAtIndex(&obj->lines, STNBTextMetricsLine, obj->lines.use - 1);
		if(dstRects != NULL){
			STNBRect r;
			r.x			= ln->visibleRight;
			r.y			= ln->yBase - ln->fontMetricsMax.ascender;
			r.width 	= 2.0f;
			r.height	= (ln->fontMetricsMax.ascender - ln->fontMetricsMax.descender); //descender is negative
			NBArray_addValue(dstRects, r);
		}
	} else {
		const STNBTextMetricsLine* ln = NBTextMetrics_lineByCharIndex(obj, start);
		NBASSERT(ln != NULL)
		if(ln != NULL){
			const UI32 afterEnd = start + size;
			const STNBTextMetricsLine* lnFirst = NBArray_dataPtr(&obj->lines, STNBTextMetricsLine);
			const STNBTextMetricsLine* lnAfterEnd = lnFirst + obj->lines.use;
			//PRINTF_INFO("First rect is at line #%d of %d.\n", (UI32)(ln - lnFirst + 1), obj->lines.use);
			NBASSERT(NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, ln->rngWords.start)->rngChars.start <= start)
			while(ln < lnAfterEnd){
				if(ln->rngWords.start < ln->rngWords.afterEnd){ //not-empty
					const STNBTextMetricsWord* w0 = NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, ln->rngWords.start);
					if(afterEnd <= w0->rngChars.start){
						//PRINTF_INFO("Stopping at line #%d of %d.\n", (UI32)(ln - lnFirst + 1), obj->lines.use);
						//Range ends before this line
						break;
					} else {
						//Range contains this line
						STNBRect r;
						r.y			= ln->yBase - ln->fontMetricsMax.ascender;
						r.height	= (ln->fontMetricsMax.ascender - ln->fontMetricsMax.descender); //descender is negative
						//Set rect's left
						{
							if(start <= w0->rngChars.start){
								//Line's range starts AT line
								r.x = ln->visibleLeft;
							} else {
								//Line's range starts INSIDE line
								const STNBTextMetricsChar* c = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, start);
								r.x = c->pos.x - c->extendsLeft;
							}
						}
						//Set rect's right
						{
							const STNBTextMetricsWord* w1 = NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, ln->rngWords.afterEnd - 1);
							NBASSERT(start < w1->rngChars.afterEnd)
							if(afterEnd >= w1->rngChars.afterEnd){
								//Line's range ends AT line
								r.width = ln->visibleRight - r.x;
							} else {
								//Line's range ends INSIDE line
								const STNBTextMetricsChar* c = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, afterEnd);
								r.width = (c->pos.x - c->extendsLeft) - r.x;
							}
						}
						//Add
						NBArray_addValue(dstRects, r);
					}
				}
				ln++;
			}
		}
	}
}

UI32 NBTextMetrics_closestCharIdx(const STNBTextMetrics* obj, const float x, const float y, UI32* dstLineIdx){
	UI32 r = 0;
	if(obj->lines.use > 0){
		const STNBTextMetricsLine* ln = NBTextMetrics_lineByCoord(obj, y);
		if(ln != NULL){
			const STNBTextMetricsChar* c = NBTextMetrics_charAtLineByCoord(obj, ln, x);
			NBASSERT(c != NULL)
			if(c != NULL){
				const STNBTextMetricsChar* first = NBArray_dataPtr(&obj->chars, STNBTextMetricsChar);
				const STNBTextMetricsChar* lastC = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, obj->chars.use - 1);
				//Move to next char if beyond last-char or below last-line
				if(c == lastC){
					IF_NBASSERT(const STNBTextMetricsLine* lastLn = NBArray_itmPtrAtIndex(&obj->lines, STNBTextMetricsLine, obj->lines.use - 1);)
					NBASSERT(ln == lastLn)
					if((c->pos.x + c->extendsRight) < x || (ln->yBase + ln->fontMetricsMax.descender) < y){ //descender is negative
						c++;
					}
				}
				r = (UI32)(c - first);
			}
			//Save line
			if(dstLineIdx != NULL){
				const STNBTextMetricsLine* first = NBArray_dataPtr(&obj->lines, STNBTextMetricsLine);
				*dstLineIdx = (UI32)(ln - first);
			}
		} else {
			if(dstLineIdx != NULL){
				*dstLineIdx = 0;
			}
		}
	}
	return r;
}

STNBTextMetricsCharAction NBTextMetrics_getCharAction(const UI32 unicode, const UI32 curLineWordsCount, const UI32 curWordCharsCount){
	STNBTextMetricsCharAction r;
	NBMemory_setZeroSt(r, STNBTextMetricsCharAction);
	switch (unicode) {
		case '\r':
		case '\n':
			r.endLine	= r.endWord	= (unicode == '\n');
			r.isSpace	= TRUE;
			r.charMode	= ENTextMetricsCharMode_noVisual; //will be added at the same line
			break;
		case '\0':
		case '\t':
		case ' ':
		case 0x2003: //Width-space
			r.endLine	= FALSE;
			r.endWord	= r.isSpace = TRUE;
			r.charMode	= ENTextMetricsCharMode_add;
			break;
		default:
			//letters
			r.endLine	= r.endWord = r.isSpace = FALSE;
			r.charMode	= ENTextMetricsCharMode_add;
			//ToDo: remove. Only spaces should be explicit separators.
			/*if((unicode >= ' ' && unicode < '0') || (unicode > '9' && unicode < 'A') || (unicode > 'Z' && unicode < 'a') || (unicode > 'z' && unicode <= 127)){
				//Codes that can separate words
				r.endWord	= TRUE;
				r.endLine	= r.isSpace = FALSE;
				r.charMode	= ENTextMetricsCharMode_add;
			} else {
				//letters
				r.endLine	= r.endWord = r.isSpace = FALSE;
				r.charMode	= ENTextMetricsCharMode_add;
			}*/
			break;
	}
	//PRINTF_INFO("NBTextMetrics_getCharAction, curLineWordsCount(%u) curWordCharsCount(%u).\n", curLineWordsCount, curWordCharsCount);
	return r;
}

//Edits

STNBRangeI NBTextMetrics_removeChars(STNBTextMetrics* obj, const UI32 start, const UI32 numChars, STNBRangeI* dstRemovedCharsRng){
	return NBTextMetrics_replaceChars(obj, start, numChars, NULL, 0, dstRemovedCharsRng, NULL);
}

STNBRangeI NBTextMetrics_replaceWords(STNBTextMetrics* obj, const STNBRangeI charsReplaced, const UI32 newCharsCount, const STNBTextMetricsWord* newWords, const UI32 newWordsSz, UI32* dstNewWordsCount){
	STNBRangeI r; NBMemory_setZeroSt(r, STNBRangeI);
	const UI32 endAfter = (charsReplaced.start + charsReplaced.size);
	//-------------------------
	//-- Modifications sequences are:
	//-- 1: truncate-left-word
	//-- 2: add new words
	//---- 2.1: concatenate-first
	//---- 2.2: add-new-words
	//-- 3: update-words-affected-by-removed-chars-range
	//---- 3.1: remove-deleted-words
	//---- 3.2: truncate-last (move the start to right)
	//-- 4: add or concatenate orphans (one word)
	//-- 5: move remainig words to deltaChars (+ added - removed)
	//-------------------------
	//
	/*PRINTF_INFO("----------------\n");
	 PRINTF_INFO("PROCESSING WORDS (%d deltaChars)\n", (SI32)newCharsCount - (SI32)numChars);
	 PRINTF_INFO("----------------\n");*/
	STNBTextMetricsWord wOrphans, wPrev; UI32 newIdx = 0;
	NBMemory_setZeroSt(wPrev, STNBTextMetricsWord);
	wPrev.isSpace			= TRUE;
	wPrev.isControl			= TRUE;
	wPrev.rngChars.start	= wPrev.rngChars.afterEnd = charsReplaced.start;
	wOrphans				= wPrev;
	//Identify left-word (the one before the replaced range)
	if(charsReplaced.start > 0){
		//Search word by charIdx
		const STNBTextMetricsWord* w = NBTextMetrics_wordByCharIndex(obj, charsReplaced.start - 1);
		NBASSERT(w != NULL)
		if(w != NULL){
			STNBTextMetricsWord* wordsPtr = NBArray_dataPtr(&obj->words, STNBTextMetricsWord);
			wOrphans	= *w;
			newIdx		= (UI32)(w - wordsPtr) + 1; NBASSERT(newIdx <= obj->words.use)
			NBASSERT(w->rngChars.start < charsReplaced.start && w->rngChars.afterEnd >= charsReplaced.start)
			//PRINTF_INFO("Found left-word(#%d of %d) truncating from (%d, %d) to (%d, %d).\n", newIdx, obj->words.use, w->rngChars.start, w->rngChars.afterEnd, w->rngChars.start, start);
			//-- 1: truncate-left-word
			{
				STNBTextMetricsWord* w	= &wordsPtr[newIdx - 1];
				w->rngChars.afterEnd	= charsReplaced.start;
				wPrev	= *w;
			}
		}
	}
	//------------------
	//-- Precalculate modifications to array.
	//-- From "2: add new words" to "4: add or concatenate orphans (one word)"
	//------------------
	SI32 wAdds = 0, wRemoves = 0;
	BOOL wConcatFirst = FALSE;
	BOOL wTruncs = FALSE; STNBTextMetricsWord wTruncsData;
	BOOL concatLast = FALSE; STNBTextMetricsWord wLastNextData;
	UI32 orphansNextStart = 0;
	NBMemory_setZeroSt(wTruncsData, STNBTextMetricsWord);
	NBMemory_setZeroSt(wLastNextData, STNBTextMetricsWord);
	{
		//Backup state
		const STNBTextMetricsWord wPrev2 = wPrev;
		const UI32 newIdx2 = newIdx;
		//-- 2: add new words (char-indexes already calculated)
		NBASSERT(wPrev.rngChars.afterEnd == charsReplaced.start)
		if(newWordsSz > 0){
			//Try to concat-first-word
			{
				const STNBTextMetricsWord* newWord = &newWords[0];
				NBASSERT(wPrev.rngChars.afterEnd == newWord->rngChars.start)
				NBASSERT(charsReplaced.start <= newWord->rngChars.start)
				const BOOL canConcat = (!wPrev.isControl && !wPrev.isSpace && !newWord->isSpace && !newWord->isControl);
				if(canConcat){
					//---- 2.1: concatenate-first
					NBASSERT(wPrev.rngChars.afterEnd == newWord->rngChars.start)
					//PRINTF_INFO("New word #%d/%d concatenated to previous (%d -> %d) + (%d -> %d) = (%d -> %d) at idx(%d)\n", (i + 1), newWords.use, prev->rngChars.start, prev->rngChars.afterEnd, newWord->rngChars.start, newWord->rngChars.afterEnd, prev->rngChars.start, newWord->rngChars.afterEnd, (newIdx - 1));
					wConcatFirst = TRUE;
				}
			}
			//Count words to add
			{
				const STNBTextMetricsWord* lastNewWord = &newWords[newWordsSz - 1];
				wPrev = *lastNewWord;
				wAdds += (newWordsSz - (wConcatFirst ? 1 : 0)); //excludes first if was concatenated
			}
		}
		//-- 3: update-words-affected-by-removed-chars-range
		while(newIdx < obj->words.use){
			const STNBTextMetricsWord* w = NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, newIdx);
			NBASSERT(charsReplaced.start <= w->rngChars.start) //This word must start at-or-after the remplacement-range's start
			if(endAfter <= w->rngChars.start){
				break;
			} else if(w->rngChars.afterEnd <= endAfter){
				//---- 3.1: remove-deleted-words
				NBASSERT((charsReplaced.start <= w->rngChars.start && w->rngChars.afterEnd <= endAfter))
				//PRINTF_INFO("Removed word #%d/%d (%d -> %d)\n", newIdx, obj->words.use, w->rngChars.start, w->rngChars.afterEnd);
				//Simulation
				newIdx++;
				wRemoves++;
			} else {
				//---- 3.2: truncate-last (move the start to right)
				NBASSERT(wPrev.rngChars.afterEnd == (charsReplaced.start + newCharsCount))
				//PRINTF_INFO("Word #%d/%d truncated  from(%d -> %d) to (%d -> %d)\n", newIdx, obj->words.use, w->rngChars.start, w->rngChars.afterEnd, wPrev.rngChars.afterEnd, (w->rngChars.afterEnd + newCharsCount - numChars));
				//Simulation
				wTruncs		= TRUE;
				wTruncsData	= *w;
				wPrev		= *w;
				wPrev.rngChars.start 	= wPrev.rngChars.afterEnd;
				wPrev.rngChars.afterEnd	= w->rngChars.afterEnd + newCharsCount - charsReplaced.size;
				newIdx++;
				break;
			}
		}
		//-- 4: add or concatenate orphans (one word)
		{
			NBASSERT(newIdx <= obj->words.use)
			if(newIdx >= obj->words.use){
				orphansNextStart		= obj->chars.use;
			} else {
				const STNBTextMetricsWord* w = NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, newIdx);
				orphansNextStart		= w->rngChars.start + newCharsCount - charsReplaced.size;
			}
			NBASSERT((endAfter + newCharsCount - charsReplaced.size) <= orphansNextStart)
			NBASSERT(wPrev.rngChars.afterEnd <= orphansNextStart)
			if(wPrev.rngChars.afterEnd < orphansNextStart){
				const BOOL canConcat		= (!wPrev.isSpace && !wPrev.isControl && !wOrphans.isControl && !wOrphans.isSpace);
				wOrphans.rngChars.start		= wPrev.rngChars.afterEnd;
				wOrphans.rngChars.afterEnd	= orphansNextStart;
				if(canConcat){
					//PRINTF_INFO("Orphan-chars concatenated to previous (%d -> %d) + (%d -> %d) = (%d -> %d) at idx(%d)\n", prev->rngChars.start, prev->rngChars.afterEnd, wOrphans.rngChars.start, wOrphans.rngChars.afterEnd, prev->rngChars.start, wOrphans.rngChars.afterEnd, (newIdx - 1));
				} else {
					//PRINTF_INFO("Orphan-chars added (%d -> %d chars) at idx(%d)\n", wOrphans.rngChars.start, wOrphans.rngChars.afterEnd, newIdx);
					wAdds++; //Orphans will be added
				}
				wPrev = wOrphans;
			} else {
				//No orphans, can next-word be concatenated to prev?
				if(newIdx < obj->words.use){
					STNBTextMetricsWord* w	= NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, newIdx);
					NBASSERT(endAfter <= w->rngChars.start)
					NBASSERT(wPrev.rngChars.afterEnd == (w->rngChars.start + newCharsCount - charsReplaced.size))
					const BOOL canConcat = (!wPrev.isSpace && !wPrev.isControl && !w->isControl && !w->isSpace);
					if(canConcat){
						concatLast		= TRUE;
						wLastNextData	= *w;
						wPrev			= *w;
						wPrev.rngChars.start	= wPrev.rngChars.start + newCharsCount - charsReplaced.size;
						wPrev.rngChars.afterEnd	= wPrev.rngChars.afterEnd + newCharsCount - charsReplaced.size;
						wRemoves++;
						newIdx++;
					}
				}
			}
		}
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		//[[[Verification only]]]
		//-- 5: move remainig words to deltaChars (+ added - removed)
		{
			for(; newIdx < obj->words.use; newIdx++){
				const STNBTextMetricsWord* w	= NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, newIdx);
				NBASSERT(endAfter <= w->rngChars.start)
				NBASSERT(wPrev.rngChars.afterEnd == (w->rngChars.start + newCharsCount - charsReplaced.size))
				wPrev = *w;
				wPrev.rngChars.start	= wPrev.rngChars.start + newCharsCount - charsReplaced.size;
				wPrev.rngChars.afterEnd	= wPrev.rngChars.afterEnd + newCharsCount - charsReplaced.size;
			}
			NBASSERT(wPrev.rngChars.afterEnd == obj->chars.use)
		}
#		endif
		//Restore state
		wPrev 	= wPrev2;
		newIdx	= newIdx2;
	}
	//------------------
	//-- Resize array
	//------------------
	{
		NBASSERT(wAdds >= 0 && wRemoves >= 0)
		//Update results to return
		{
			r.start		= newIdx;
			r.size		= wRemoves;
			if(dstNewWordsCount != NULL){
				*dstNewWordsCount = wAdds;
			}
		}
		//
		if(wAdds < wRemoves){
			//Array will shrink
			NBArray_removeItemsAtIndex(&obj->words, newIdx, (wRemoves - wAdds));
		} else if(wRemoves < wAdds){
			//Array will grow
			NBArray_addItemsAtIndex(&obj->words, newIdx, NULL, sizeof(STNBTextMetricsWord), (wAdds - wRemoves));
		}
	}
	//------------------
	//-- Update array
	//------------------
	//-- 2: add new words (char-indexes already calculated)
	NBASSERT(wPrev.rngChars.afterEnd == charsReplaced.start)
	if(newWordsSz > 0){
		UI32 i = 0;
		//Concat first
		if(wConcatFirst){
			const STNBTextMetricsWord* newWord = &newWords[0];
			NBASSERT(wPrev.rngChars.afterEnd == newWord->rngChars.start)
			NBASSERT(charsReplaced.start <= newWord->rngChars.start)
			//---- 2.1: concatenate-first
			STNBTextMetricsWord* prev = NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, newIdx - 1);
			NBASSERT(wPrev.rngChars.afterEnd == newWord->rngChars.start)
			//PRINTF_INFO("New word #%d/%d concatenated to previous (%d -> %d) + (%d -> %d) = (%d -> %d) at idx(%d)\n", (i + 1), newWords.use, prev->rngChars.start, prev->rngChars.afterEnd, newWord->rngChars.start, newWord->rngChars.afterEnd, prev->rngChars.start, newWord->rngChars.afterEnd, (newIdx - 1));
			//Action
			NBASSERT(prev->rngChars.afterEnd == newWord->rngChars.start)
			prev->rngChars.afterEnd = newWord->rngChars.afterEnd;
			wPrev = *newWord;
			i++;
		}
		for(; i < newWordsSz; i++){
			const STNBTextMetricsWord* newWord = &newWords[i];
			NBASSERT(wPrev.rngChars.afterEnd == newWord->rngChars.start)
			NBASSERT(charsReplaced.start <= newWord->rngChars.start)
			//---- 2.2: add-new-words
			//PRINTF_INFO("New word #%d/%d added (%d -> %d) at idx(%d)\n", (i + 1), newWords.use, newWord->rngChars.start, newWord->rngChars.afterEnd, newIdx);
			{
				//Apply only (array was modified)
				STNBTextMetricsWord* dst = NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, newIdx);
				*dst = *newWord;
				newIdx++;
			}
			wPrev = *newWord;
		}
	}
	//-- 3: update-words-affected-by-removed-chars-range
	//-- Records already deleted, only truncate last.
	if(wTruncs){
		//---- 3.2: truncate-last (move the start to right)
		STNBTextMetricsWord* w = NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, newIdx);
		*w = wTruncsData;
		NBASSERT(wPrev.rngChars.afterEnd == (charsReplaced.start + newCharsCount))
		//PRINTF_INFO("Word #%d/%d truncated  from(%d -> %d) to (%d -> %d)\n", newIdx, obj->words.use, w->rngChars.start, w->rngChars.afterEnd, wPrev.rngChars.afterEnd, (w->rngChars.afterEnd + newCharsCount - numChars));
		//Action
		w->rngChars.start 		= wPrev.rngChars.afterEnd;
		w->rngChars.afterEnd	= w->rngChars.afterEnd + newCharsCount - charsReplaced.size;
		wPrev = *w;
		newIdx++;
	}
	//-- 4: add or concatenate orphans (one word)
	{
		NBASSERT((endAfter + newCharsCount - charsReplaced.size) <= orphansNextStart)
		NBASSERT(wPrev.rngChars.afterEnd <= orphansNextStart)
		if(wPrev.rngChars.afterEnd < orphansNextStart){
			const BOOL canConcat		= (!wPrev.isSpace && !wPrev.isControl && !wOrphans.isControl && !wOrphans.isSpace);
			wOrphans.rngChars.start		= wPrev.rngChars.afterEnd;
			wOrphans.rngChars.afterEnd	= orphansNextStart;
			if(canConcat){
				STNBTextMetricsWord* prev = NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, newIdx - 1);
				//PRINTF_INFO("Orphan-chars concatenated to previous (%d -> %d) + (%d -> %d) = (%d -> %d) at idx(%d)\n", prev->rngChars.start, prev->rngChars.afterEnd, wOrphans.rngChars.start, wOrphans.rngChars.afterEnd, prev->rngChars.start, wOrphans.rngChars.afterEnd, (newIdx - 1));
				//Action
				NBASSERT(prev->rngChars.afterEnd == wPrev.rngChars.afterEnd)
				prev->rngChars.afterEnd = wOrphans.rngChars.afterEnd;
			} else {
				//PRINTF_INFO("Orphan-chars added (%d -> %d chars) at idx(%d)\n", wOrphans.rngChars.start, wOrphans.rngChars.afterEnd, newIdx);
				/*if(mode == 1)*/{
					//Apply only (array was modified)
					STNBTextMetricsWord* dst = NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, newIdx);
					*dst = wOrphans;
					newIdx++;
				}/* else if(mode == 2){
					//Modify array
					NBArray_addItemsAtIndex(&obj->words, newIdx, &wOrphans, sizeof(STNBTextMetricsWord), 1);
					newIdx++;
				}*/
			}
			wPrev = wOrphans;
		} else {
			if(concatLast){
				NBASSERT(newIdx > 0)
				STNBTextMetricsWord* prev = NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, newIdx - 1);
				NBASSERT(prev->rngChars.afterEnd == wPrev.rngChars.afterEnd)
				prev->rngChars.afterEnd = (wLastNextData.rngChars.afterEnd + newCharsCount - charsReplaced.size);
				wPrev = *prev;
			}
			/*concatLast		= TRUE;
			wLastNextData	= *w;
			//No orphans, can next-word be concatenated to previous?
			if(newIdx < obj->words.use){
				STNBTextMetricsWord* w	= NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, newIdx);
				NBASSERT(endAfter <= w->rngChars.start)
				NBASSERT(wPrev.rngChars.afterEnd == (w->rngChars.start + newCharsCount - charsReplaced.size))
				const BOOL canConcat = (!wPrev.isSpace && !wPrev.isControl && !w->isControl && !w->isSpace);
				if(canConcat){
					STNBTextMetricsWord* prev = NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, newIdx - 1);
					NBASSERT(prev->rngChars.afterEnd == wPrev.rngChars.afterEnd)
					prev->rngChars.afterEnd = (w->rngChars.start + newCharsCount - charsReplaced.size);
				}
			}*/
		}
	}
	//-- 5: move remainig words to deltaChars (+ added - removed)
	{
		for(; newIdx < obj->words.use; newIdx++){
			STNBTextMetricsWord* w	= NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, newIdx);
			NBASSERT(endAfter <= w->rngChars.start)
			w->rngChars.start		= w->rngChars.start + newCharsCount - charsReplaced.size;
			w->rngChars.afterEnd	= w->rngChars.afterEnd + newCharsCount - charsReplaced.size;
			//
			NBASSERT(wPrev.rngChars.afterEnd == w->rngChars.start)
			IF_NBASSERT(wPrev = *w;)
		}
		NBASSERT(wPrev.rngChars.afterEnd == obj->chars.use)
	}
	//
	return r;
}

STNBRangeI NBTextMetrics_replaceChars(STNBTextMetrics* obj, const UI32 pStart, const UI32 pNumChars, const char* newStr, const UI32 pNewStrSz, STNBRangeI* dstRemovedCharsRng, STNBRangeI* dstAddedCharsRng){
	STNBRangeI cRmvRng		= { pStart, pNumChars };
	STNBRangeI cAddRng		= { pStart, 0 };
	const STNBRangeI bRmv	= NBTextMetrics_replaceCharsSimulation(obj, pStart, pNumChars, newStr, pNewStrSz, &cRmvRng, &cAddRng);
	//const SI32 endAfter	= cRmvRng.start + cRmvRng.size;
	//const SI32 bAfterEnd	= bRmv.start + bRmv.size;
	//SI32 endAfter = 0, newCharsCount = 0, bStart, bAfterEnd, numBytes;
	IF_NBASSERT(const UI32 prevLen = obj->chars.use;)
	STNBArray newWords;
	NBArray_initWithSz(&newWords, sizeof(STNBTextMetricsWord), NULL, (pNewStrSz > 0 ? 1 : 0), 8, 1.5f);
	NBASSERT(newStr != NULL || pNewStrSz == 0)
	//--------------------
	//- Update chars array
	//--------------------
	{
		//Make room for chars
		if(cAddRng.size < cRmvRng.size){
			//Length will decrease
			const SI32 delta = (cRmvRng.size - cAddRng.size);
			NBArray_removeItemsAtIndex(&obj->chars, cRmvRng.start, delta);
		} else if(cRmvRng.size < cAddRng.size){
			//Length will increase
			const UI32 delta = (cAddRng.size - cRmvRng.size);
			NBArray_addItemsAtIndex(&obj->chars, cRmvRng.start, NULL, sizeof(STNBTextMetricsChar), delta);
		}
		//Copy new-content
		{
			NBASSERT((cRmvRng.start + cAddRng.size) <= obj->chars.use)
			if(newStr != NULL && cAddRng.size > 0){
				STNBTextMetricsWord* newWord = NULL;
				UI32 iByte = 0, charSz = 0, unicode = 0, iChar = cRmvRng.start;
				while(iByte < pNewStrSz){
					charSz		= NBEncoding_utf8BytesExpected(newStr[iByte]);
					unicode		= NBEncoding_unicodeFromUtf8s(&newStr[iByte], (UI8)charSz, 0);
					//Update logic-char (already reserved)
					{
						STNBTextMetricsChar* c = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, iChar);
						NBMemory_setZeroSt(*c, STNBTextMetricsChar);
						c->iByte 	= bRmv.start + iByte;
						c->bytesLen	= (UI8)charSz;
					}
					//Analyze new-words
					{
						STNBTextMetricsCharAction action = NBTextMetrics_getCharAction(unicode, 0, 0);
						//Close prev word
						if(action.endWord && newWord != NULL){
							//PRINTF_INFO("Char '%c' CLOSED prev word.\n", (char)unicode);
							newWord = NULL;
						}
						//Process char
						if(action.charMode != ENTextMetricsCharMode_ignore){
							NBASSERT(action.charMode == ENTextMetricsCharMode_noVisual || action.charMode == ENTextMetricsCharMode_add)
							if(newWord == NULL){
								//Start new word
								newWord					= NBArray_add(&newWords, NULL, sizeof(STNBTextMetricsWord));
								NBMemory_setZeroSt(*newWord, STNBTextMetricsWord);
								newWord->isControl		= (action.charMode == ENTextMetricsCharMode_noVisual);
								newWord->isSpace		= action.isSpace;
								newWord->rngChars.start	= iChar;
								newWord->rngChars.afterEnd = iChar + 1;
								//PRINTF_INFO("Char '%c' STARTED current word.\n", (char)unicode);
							} else {
								//Continue word
								newWord->rngChars.afterEnd++;
								//PRINTF_INFO("Char '%c' CONITNUED current word.\n", (char)unicode);
								NBASSERT(newWord->rngChars.start < newWord->rngChars.afterEnd)
							}
						}
						//Close current word
						if(action.endWord && newWord != NULL){
							//PRINTF_INFO("Char '%c' CLOSED current word.\n", (char)unicode);
							newWord = NULL;
						}
					}
					iByte		+= charSz;
					iChar++;
				}
				NBASSERT(iChar == (cRmvRng.start + cAddRng.size))
			}
			NBASSERT(obj->chars.use == (prevLen + cAddRng.size - cRmvRng.size))
		}
		//Update right-chars bytes-definitions
		{
			UI32 i; for(i = cRmvRng.start + cAddRng.size; i < obj->chars.use; i++){
				STNBTextMetricsChar* c = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, i);
				c->iByte = c->iByte + pNewStrSz - bRmv.size;
			}
		}
	}
	//--------------------
	//- Update words array
	//--------------------
	{
		UI32 linesAddedOrRemoved = 0;
		IF_NBASSERT(const UI32 wCountOrig = obj->words.use;)
		UI32 newWordsCount = 0;
		const STNBTextMetricsWord* nWords = NBArray_dataPtr(&newWords, STNBTextMetricsWord);
		const STNBRangeI rngRemoved =  NBTextMetrics_replaceWords(obj, cRmvRng, cAddRng.size, nWords, newWords.use, &newWordsCount);
		NBASSERT(rngRemoved.start >= 0 && rngRemoved.start <= wCountOrig)
		NBASSERT(rngRemoved.size >= 0 && (rngRemoved.start + rngRemoved.size) <= wCountOrig)
		NBASSERT(obj->words.use == (wCountOrig + newWordsCount - rngRemoved.size))
		//--------------------
		//- Update lines array
		//--------------------
		if(obj->lines.use == 0){
			if(obj->words.use != 0){
				//Add first-line (take everything)
				STNBTextMetricsLine ln;
				NBMemory_setZeroSt(ln, STNBTextMetricsLine);
				ln.rngWords.start		= 0;
				ln.rngWords.afterEnd	= obj->words.use;
				NBArray_addValue(&obj->lines, ln);
				linesAddedOrRemoved++;
			}
		} else {
			//Remove or truncate lines
			const SI32 rngAfterEnd = (rngRemoved.start + rngRemoved.size);
			STNBTextMetricsLine* ln = NBArray_itmPtrAtIndex(&obj->lines, STNBTextMetricsLine, (obj->lines.use - 1));
			if(ln->rngWords.afterEnd == rngRemoved.start){
				//Special case: words added after the last line.
				ln->rngWords.afterEnd = rngRemoved.start + newWordsCount;
			} else {
				//Common case: update lines-ranges inverse order
				SI32 i = (obj->lines.use - 1);
				while(i >= 0){
					STNBTextMetricsLine* ln = NBArray_itmPtrAtIndex(&obj->lines, STNBTextMetricsLine, i);
					if(ln->rngWords.afterEnd <= rngRemoved.start){
						//End-of-cicle (this and prev-lines should not be affected)
						break;
					} else if(ln->rngWords.start <= rngRemoved.start && rngRemoved.start < ln->rngWords.afterEnd){
						//The new content will be added to this line
						if(ln->rngWords.afterEnd <= rngAfterEnd){
							//The line ended inside the replaced range
							ln->rngWords.afterEnd = rngRemoved.start + newWordsCount;
						} else {
							//The line continued after the replaced range
							ln->rngWords.afterEnd = ln->rngWords.afterEnd + newWordsCount - rngRemoved.size;
						}
						NBASSERT(ln->rngWords.start <= ln->rngWords.afterEnd)
						if(ln->rngWords.afterEnd <= ln->rngWords.start){
							//Line resulted empty, remove-it
							NBArray_removeItemAtIndex(&obj->lines, i);
							linesAddedOrRemoved++;
						}
						//End-of-cicle (prev-lines should not be affected)
						break;
					} else {
						//This line is after the replacement start (is moved or tunked)
						if(rngAfterEnd <= ln->rngWords.start){
							//Move delta-words to the right
							ln->rngWords.start = ln->rngWords.start + newWordsCount - rngRemoved.size;
						} else {
							//Move to resulting right limit
							ln->rngWords.start = rngRemoved.start + newWordsCount;
						}
						if(rngAfterEnd < ln->rngWords.afterEnd){
							//Move delta-words to the right
							ln->rngWords.afterEnd = ln->rngWords.afterEnd + newWordsCount - rngRemoved.size;
						} else {
							//Move to resulting right limit
							ln->rngWords.afterEnd = rngRemoved.start + newWordsCount;
						}
						NBASSERT(ln->rngWords.start <= ln->rngWords.afterEnd)
						if(ln->rngWords.afterEnd <= ln->rngWords.start){
							//Line resulted empty, remove-it
							NBArray_removeItemAtIndex(&obj->lines, i);
							linesAddedOrRemoved++;
						}
					}
					//Next line
					i--;
				}
			}
		}
		//Update boxes
		/*if(linesAddedOrRemoved > 0){
			NBTextMetrics_buildMultiLevelBoxes(obj, obj->boxesGrpsSz);
		}*/
	}
	if(dstRemovedCharsRng != NULL){
		*dstRemovedCharsRng = cRmvRng;
	}
	if(dstAddedCharsRng != NULL){
		*dstAddedCharsRng = cAddRng;
	}
	NBArray_release(&newWords);
	return bRmv;
}

STNBRangeI NBTextMetrics_replaceCharsSimulation(STNBTextMetrics* obj, const UI32 pStart, const UI32 pNumChars, const char* newStr, const UI32 pNewStrSz, STNBRangeI* dstRemoveCharsRng, STNBRangeI* dstAddCharsRng){
	STNBRangeI r; NBMemory_setZeroSt(r, STNBRangeI);
	STNBRangeI cReplcd = { pStart, pNumChars };
	SI32 endAfter = 0, newCharsCount = 0, bStart, bAfterEnd;
	NBASSERT(newStr != NULL || pNewStrSz == 0)
	PRINTF_INFO("NBTextMetrics_replaceCharsSimulation(%u, +%u) / %d, with strlen(%d).\n", pStart, pNumChars, obj->chars.use, pNewStrSz);
	//Validate-range
	{
		if(cReplcd.start > obj->chars.use){
			cReplcd.start = obj->chars.use;
		}
		if((cReplcd.start + cReplcd.size) > obj->chars.use){
			NBASSERT(cReplcd.start <= obj->chars.use)
			cReplcd.size = obj->chars.use - cReplcd.start;
		}
		endAfter = cReplcd.start + cReplcd.size;
	}
	//Count new-logic-chars
	{
		UI32 iByte = 0, charSz = 0;
		while(iByte < pNewStrSz){
			charSz	= NBEncoding_utf8BytesExpected(newStr[iByte]);
			iByte	+= charSz;
			newCharsCount++;
		}
	}
	//Identify orig-bytes-range
	{
		NBASSERT(cReplcd.start <= obj->chars.use)
		NBASSERT((cReplcd.start + cReplcd.size) <= obj->chars.use)
		//Start-of-bytes
		if(cReplcd.start >= obj->chars.use){
			if(obj->chars.use <= 0){
				//The string is empty
				bStart	= 0;
				PRINTF_INFO("NBTextMetrics_replaceCharsSimulation: The string is empty.\n");
			} else {
				//Use last char next-byte.
				PRINTF_INFO("NBTextMetrics_replaceCharsSimulation: Use last char next-byte.\n");
				const STNBTextMetricsChar* lChar = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, obj->chars.use - 1);
				bStart	= (lChar->iByte + lChar->bytesLen);
			}
		} else {
			//Find the replacement start-char
			PRINTF_INFO("NBTextMetrics_replaceCharsSimulation: Find the replacement start-char(#%d) / %d.\n", (cReplcd.start + 1), obj->chars.use);
			const STNBTextMetricsChar* cStart = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, cReplcd.start);
			bStart		= cStart->iByte;
		}
		//End-of-bytes
		if(cReplcd.size <= 0){
			//Empty replaced
			PRINTF_INFO("NBTextMetrics_replaceCharsSimulation: Empty replaced.\n");
			bAfterEnd	= bStart;
		} else {
			//Replacing chars
			PRINTF_INFO("NBTextMetrics_replaceCharsSimulation: Replacing %d chars [%d, %d] / %d.\n", cReplcd.size, cReplcd.start, cReplcd.start + cReplcd.size - 1, obj->chars.use);
			NBASSERT((cReplcd.start + cReplcd.size - 1) < obj->chars.use)
			const STNBTextMetricsChar* cEnd = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, cReplcd.start + cReplcd.size - 1);
			bAfterEnd	= cEnd->iByte + cEnd->bytesLen;
		}
		//Number of bytes
		NBASSERT(bStart <= bAfterEnd)
	}
	//Return
	if(dstRemoveCharsRng != NULL){
		*dstRemoveCharsRng = cReplcd;
	}
	if(dstAddCharsRng != NULL){
		dstAddCharsRng->start	= cReplcd.start;
		dstAddCharsRng->size	= newCharsCount;
	}
	r.start = bStart;
	r.size	= (bAfterEnd - bStart);
	return r;
}

//Test

void NBTextMetrics_dbgTestAllSeqs(const STNBTextMetrics* obj, const char* textStr, const UI32 textStrLen, const UI8 printLevel){
	if(printLevel > 1){
		PRINTF_INFO("------------------------------------\n");
	}
	if(printLevel > 0){
		PRINTF_INFO("Testing metrics data sequences.\n");
	}
	if(printLevel > 1){
		PRINTF_INFO("------------------------------------\n");
	}
	NBASSERT((obj->chars.use == 0 && obj->words.use == 0 && obj->lines.use == 0) || (obj->chars.use > 0 && obj->words.use > 0 && obj->lines.use > 0))
	NBTextMetrics_dbgTestCharsSeq(obj, textStr, textStrLen, printLevel);
	NBTextMetrics_dbgTestWordsSeq(obj, textStr, textStrLen, printLevel);
	NBTextMetrics_dbgTestLinesSeq(obj, textStr, textStrLen, printLevel);
}

void NBTextMetrics_dbgTestCharsSeq(const STNBTextMetrics* obj, const char* textStr, const UI32 textStrLen, const UI8 printLevel){
	NBASSERT(obj->chars.use > 0 || textStrLen == 0) //All bytes must be mapped
	//Verify chars's bytes range (holes are not allowed)
	if(obj->chars.use > 0){
		const STNBTextMetricsChar* first = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, 0);
		UI32 iByte = first->iByte + first->bytesLen;
		const STNBTextMetricsChar* c = first;
		UI32 i; for(i = 1; i < obj->chars.use; i++){
			c = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, i);
			if(iByte != c->iByte && textStr != NULL){
				IF_PRINTF(const char* txt = &textStr[iByte];)
				PRINTF_INFO("Logic-char #%d, hole of %d bytes detected at: '%s'.\n", (i + 1), (c->iByte - iByte ), txt);
			}
			NBASSERT(iByte == c->iByte) //forced integrity (continuity)
			NBASSERT(c->bytesLen > 0) //forced integrity (no-empty-ranges)
			iByte += c->bytesLen;
		}
		//All bytes must be mapped
		NBASSERT((c->iByte + c->bytesLen) == textStrLen)
	}
}

void NBTextMetrics_dbgTestWordsSeq(const STNBTextMetrics* obj, const char* textStr, const UI32 textStrLen, const UI8 printLevel){
	//Verify words's chars ranges (holes are not allowed)
	if(obj->words.use > 0){
		const STNBTextMetricsWord* w = NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, 0);
		NBASSERT(w->rngChars.start == 0) //forced integrity (continuity)
		NBASSERT(w->rngChars.start < w->rngChars.afterEnd) //forced integrity (no-empty-ranges)
		UI32 iChar = w->rngChars.afterEnd;
		UI32 i; for(i = 1; i < obj->words.use; i++){
			w = NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, i);
			if(iChar != w->rngChars.start && textStr != NULL){
				IF_PRINTF(const STNBTextMetricsChar* c = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, iChar);)
				IF_PRINTF(const STNBTextMetricsChar* c2 = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, w->rngChars.start);)
				IF_PRINTF(const char* txt = &textStr[c->iByte];)
				IF_PRINTF(const char* txt2 = &textStr[c2->iByte];)
				PRINTF_INFO("Word #%d, hole of %d logic-chars detected at: '%s'.\n", (i + 1), (w->rngChars.start - iChar), txt);
				PRINTF_INFO("Continued at: '%s'.\n", txt2);
			}
			NBASSERT(iChar == w->rngChars.start) //forced integrity (continuity)
			NBASSERT(w->rngChars.start < w->rngChars.afterEnd) //forced integrity (no-empty-ranges)
			if(iChar != w->rngChars.start){
				if(printLevel > 0){
					PRINTF_INFO("%d orphan-chars found, before word #%d/%d.\n", (w->rngChars.start - iChar), (i + 1), obj->words.use);
				}
			}
			iChar = w->rngChars.afterEnd;
		}
		//All chars must be mapped
		NBASSERT(w->rngChars.afterEnd == obj->chars.use)
		NBASSERT(iChar == obj->chars.use)
	}
	//Query words for every char (holes are not allowed)
	if(obj->chars.use > 0){
		NBASSERT(obj->words.use > 0)
		const STNBTextMetricsWord* first = NBArray_dataPtr(&obj->words, STNBTextMetricsWord);
		const STNBTextMetricsWord* last = first;
		UI32 i; for(i = 0; i < obj->chars.use; i++){
			const STNBTextMetricsWord* word = NBTextMetrics_wordByCharIndex(obj, i);
			NBASSERT(word != NULL)
			NBASSERT(last <= word)
			if(last != word){
				const SI32 delta = (SI32)(word - last);
				NBASSERT(delta > 0)
				NBASSERT(delta == 1) //forced integrity (continuity)
				if(delta != 1){
					if(printLevel > 0){
						PRINTF_INFO("Jumped %d words (are empty?).\n", (delta - 1));
					}
				}
			}
			if(printLevel > 1){
				PRINTF_INFO("Char(#%d/%d) is at word(#%d/%d)%s.\n", (i+ 1), obj->chars.use, (SI32)(word - first + 1), obj->words.use, (last != word ? " => start-of-word" : ""));
			}
			last = word;
		}
	}
}

void NBTextMetrics_dbgTestLinesSeq(const STNBTextMetrics* obj, const char* textStr, const UI32 textStrLen, const UI8 printLevel){
	//Verify line's words ranges (holes are not allowed)
	if(obj->lines.use > 0){
		const STNBTextMetricsLine* ln = NBArray_itmPtrAtIndex(&obj->lines, STNBTextMetricsLine, 0);
		NBASSERT(ln->rngWords.start == 0) //forced integrity (continuity)
		NBASSERT(ln->rngWords.start < ln->rngWords.afterEnd) //forced integrity (no-empty-ranges)
		UI32 iWord = ln->rngWords.afterEnd;
		UI32 i; for(i = 1; i < obj->lines.use; i++){
			ln = NBArray_itmPtrAtIndex(&obj->lines, STNBTextMetricsLine, i);
			if(iWord != ln->rngWords.start && textStr != NULL){
				IF_PRINTF(const STNBTextMetricsWord* w = NBArray_itmPtrAtIndex(&obj->words, STNBTextMetricsWord, iWord);)
				IF_PRINTF(const STNBTextMetricsChar* c = NBArray_itmPtrAtIndex(&obj->chars, STNBTextMetricsChar, w->rngChars.start);)
				PRINTF_INFO("Line #%d, hole of %d words detected at: '%s'.\n", (i + 1), (ln->rngWords.start - iWord), &textStr[c->iByte]);
			}
			NBASSERT(iWord == ln->rngWords.start) //forced integrity (continuity)
			NBASSERT(ln->rngWords.start < ln->rngWords.afterEnd) //forced integrity (no-empty-ranges)
			if(iWord != ln->rngWords.start){
				if(printLevel > 0){
					PRINTF_INFO("%d orphan-words found, before line #%d/%d.\n", (ln->rngWords.start - iWord), (i + 1), obj->lines.use);
				}
			}
			iWord = ln->rngWords.afterEnd;
		}
		//All words must be mapped
		NBASSERT(ln->rngWords.afterEnd == obj->words.use)
		NBASSERT(iWord == obj->words.use)
	}
	//Query lines for every char (holes are not allowed)
	if(obj->chars.use > 0){
		NBASSERT(obj->lines.use > 0)
		const STNBTextMetricsLine* first = NBArray_dataPtr(&obj->lines, STNBTextMetricsLine);
		const STNBTextMetricsLine* last = first;
		UI32 i; for(i = 1; i < obj->chars.use; i++){
			const STNBTextMetricsLine* line = NBTextMetrics_lineByCharIndex(obj, i);
			NBASSERT(line != NULL)
			NBASSERT(last <= line)
			if(last != line){
				const SI32 delta = (SI32)(line - last);
				NBASSERT(delta > 0)
				NBASSERT(delta == 1) //forced integrity (continuity)
				if(delta != 1){
					if(printLevel > 0){
						PRINTF_INFO("Jumped %d lines (are empty?).\n", (delta - 1));
					}
				}
			}
			if(printLevel > 1){
				PRINTF_INFO("Char(#%d/%d) is at line(#%d/%d)%s.\n", (i+ 1), obj->chars.use, (SI32)(line - first + 1), obj->lines.use, (last != line ? " => start-of-line" : ""));
			}
			last = line;
		}
	}
}

void NBTextMetrics_dbgTestRangesQueries(const STNBTextMetrics* obj, const char* textStr, const UI32 textStrLen, const UI8 printLevel){
	//Test sequences first
	{
		NBTextMetrics_dbgTestAllSeqs(obj, textStr, textStrLen, printLevel);
	}
	//const UI32 textStrLen = NBString_strLenBytes(textStr);
	UI32 start = 0, size = 0, sizeBeforeRepeting = 0, lastAfter = 0, afterRepeated = 0;
	STNBString strTmp;
	NBString_init(&strTmp);
	if(printLevel > 1){
		PRINTF_INFO("------------------------------------\n");
	}
	if(printLevel > 0){
		PRINTF_INFO("Testing Ranges Queries for %d bytes: '%s'.\n", textStrLen, textStr);
	}
	if(printLevel > 1){
		PRINTF_INFO("------------------------------------\n");
	}
	//Query bytesRange from charsRanges
	{
		start = 0; size = 0; sizeBeforeRepeting = 0; lastAfter = 0; afterRepeated = 0;
		//Moving length
		while(afterRepeated < 3){ //any value greather than 1
			const STNBRangeI bRng = NBTextMetrics_charsRangeToBytesRange(obj, start, size);
			NBASSERT(bRng.start <= textStrLen)
			NBASSERT((bRng.start + bRng.size) <= textStrLen)
			if((bRng.start + bRng.size) == lastAfter){
				afterRepeated++;
			} else {
				sizeBeforeRepeting = size;
			}
			//Print
			{
				NBString_setBytes(&strTmp, &textStr[bRng.start], bRng.size);
				if(printLevel > 1){
					PRINTF_INFO("chars(%u->%u, of %u) = bytes(%u->%u, of %u) = '%s'.\n", start, size, obj->chars.use, bRng.start, bRng.size, textStrLen, strTmp.str);
				}
			}
			lastAfter = (bRng.start + bRng.size);
			size++;
		}
		NBASSERT(sizeBeforeRepeting == obj->chars.use)
		//Moving start
		while(start <= sizeBeforeRepeting){
			const STNBRangeI bRng = NBTextMetrics_charsRangeToBytesRange(obj, start, (sizeBeforeRepeting - start));
			NBASSERT(bRng.start <= textStrLen)
			NBASSERT((bRng.start + bRng.size) <= textStrLen)
			//Print
			{
				NBString_setBytes(&strTmp, &textStr[bRng.start], bRng.size);
				if(printLevel > 1){
					PRINTF_INFO("chars(%u->%u, of %u) = bytes(%u->%u, of %u) = '%s'.\n", start, size, obj->chars.use, bRng.start, bRng.size, textStrLen, strTmp.str);
				}
			}
			start++;
		}
	}
	//Query charsRanges from bytesRanges
	{
		start = 0; size = 0; sizeBeforeRepeting = 0; lastAfter = 0; afterRepeated = 0;
		//Moving length
		while(afterRepeated < 9){ //Any value greather than UTF8 bigest char
			const STNBRangeI cRng = NBTextMetrics_bytesRangeToCharsRange(obj, start, size);
			NBASSERT(cRng.start <= obj->chars.use)
			NBASSERT((cRng.start + cRng.size) <= obj->chars.use)
			if((cRng.start + cRng.size) == lastAfter){
				afterRepeated++;
			} else {
				sizeBeforeRepeting = size;
			}
			//Print
			{
				const STNBRangeI bRng = NBTextMetrics_charsRangeToBytesRange(obj, cRng.start, cRng.size);
				NBASSERT(bRng.start <= textStrLen)
				NBASSERT((bRng.start + bRng.size) <= textStrLen)
				NBString_setBytes(&strTmp, &textStr[bRng.start], bRng.size);
				if(printLevel > 1){
					PRINTF_INFO("bytes(%u->%u, of %u) = [chars(%u->%u, of %u) == bytes(%u->%u, of %u)] = '%s'.\n", start, size, textStrLen, cRng.start, cRng.size, obj->chars.use, bRng.start, bRng.size, textStrLen, strTmp.str);
				}
			}
			lastAfter = (cRng.start + cRng.size);
			size++;
		}
		NBASSERT(sizeBeforeRepeting == textStrLen)
		//Moving start
		while(start <= sizeBeforeRepeting){
			const STNBRangeI cRng = NBTextMetrics_bytesRangeToCharsRange(obj, start, (sizeBeforeRepeting - start));
			NBASSERT(cRng.start <= obj->chars.use)
			NBASSERT((cRng.start + cRng.size) <= obj->chars.use)
			//Print
			{
				const STNBRangeI bRng = NBTextMetrics_charsRangeToBytesRange(obj, cRng.start, cRng.size);
				NBASSERT(bRng.start <= textStrLen)
				NBASSERT((bRng.start + bRng.size) <= textStrLen)
				NBString_setBytes(&strTmp, &textStr[bRng.start], bRng.size);
				if(printLevel > 1){
					PRINTF_INFO("bytes(%u->%u, of %u) = [chars(%u->%u, of %u) == bytes(%u->%u, of %u)] = '%s'.\n", start, size, textStrLen, cRng.start, cRng.size, obj->chars.use, bRng.start, bRng.size, textStrLen, strTmp.str);
				}
			}
			start++;
		}
	}
	//
	NBString_release(&strTmp);
}
