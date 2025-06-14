//
//  test_textMetrics.c
//  nbframework-test-osx
//
//  Created by Marcos Ortega on 9/10/18.
//

#include "nb/NBFrameworkPch.h"
#include "test_textMetrics.h"
//
#include "nb/NBFrameworkDefs.h"
#include "nb/files/NBFilesystem.h"
#include "nb/fonts/NBFontsGlyphsStore.h"
#include "nb/fonts/NBFontsLinesStore.h"
#include "nb/fonts/NBFontsBitmapsStore.h"
#include "nb/fonts/NBText.h"
#include "nb/2d/NBPng.h"

//void testTextMetrics_drawBitmap(const STNBTextMetrics* m, const char* filename);

/*
void testTextMetrics(void){
	STNBFilesystem			fs;
	STNBFontsGlyphsStore	glyphsS;
	STNBFontsLinesStore		linesS;
	STNBFontsBitmapsStore	bitmS;
	NBFilesystem_init(&fs);
	NBFontsGlyphsStore_init(&glyphsS, &fs);
	NBFontsGlyphsStore_addSrchPath(&glyphsS, ".");
	NBFontsGlyphsStore_addSrchPath(&glyphsS, "Fuentes");
	NBFontsLinesStore_init(&linesS, &glyphsS);
	NBFontsBitmapsStore_init(&bitmS, &linesS);
	{
		//ENNBTextLineAlignH alineacionV = ENNBTextLineAlignH_Left;
		STNBFontBitmaps* font = NBFontsBitmapsStore_getFont(&bitmS, "Tangerine", "Regular", 0, 32, 1, ENNBFontSrchMode_flexible);
		STNBFontBitmaps* font2 = NBFontsBitmapsStore_getFont(&bitmS, "Roboto", "Regular", 0, 32, 1, ENNBFontSrchMode_flexible);
		STNBFontBitmaps* font3 = NBFontsBitmapsStore_getFont(&bitmS, "Homemade Apple", "Regular", 0, 32, 1, ENNBFontSrchMode_flexible);
		{
			STNBText text;
			NBText_init(&text);
			NBText_setTextAlign(&text, ENNBTextAlignV_FromTop);
			//Feed formats
			{
				NBText_setFont(&text, NBFontBitmaps_getMetricsItfRef(font2, 0));
				NBText_setCharAlign(&text, ENNBTextCharAlignV_Base);
				NBText_setColor(&text, NBST_P(STNBColor8, 0, 0, 0, 255 ));
				if(FALSE){
					//Mono-format
					NBText_setLineAlign(&text, ENNBTextLineAlignH_Left);
					NBText_setText(&text, NBST_P(STNBRect, 0, 0, 512, 512), TRUE, "Palabra1 Palabra2 Palabra3 Palabra4 Palabra5 Palabra6 Palabra7 Palabra8");
				} else {
					//Multi-format
					NBText_setLineAlign(&text, ENNBTextLineAlignH_Adjust);
					NBText_pushFormatWithFont(&text, NBFontBitmaps_getMetricsItfRef(font, 0), 0, NBST_P(STNBColor8, 0, 0, 0, 255 ), ENNBTextLineAlignH_Left, ENNBTextCharAlignV_Base);
					{
						//NBText_appendText(&text, "Hola");
						NBText_appendText(&text, "ÁéÍóÚ.");
						NBText_appendText(&text, " Este es el texto de HOLA!");
						{
							NBText_pushFormatWithFont(&text, NBFontBitmaps_getMetricsItfRef(font2, 0), 0, NBST_P(STNBColor8, 0, 0, 0, 255 ), ENNBTextLineAlignH_Left, ENNBTextCharAlignV_Base);
							{
								NBText_appendText(&text, " No se hace nada con      ver       las    cosas de esa ");
								NBText_appendTextWithFont(&text, "forma", NBFontBitmaps_getMetricsItfRef(font3, 0), 0);
								NBText_appendText(&text, "!");
								NBText_appendTextWithFont(&text, " Como has estado?", NBFontBitmaps_getMetricsItfRef(font3, 0), 0);
							}
							NBText_popFormat(&text);
						}
						NBText_appendText(&text, " Hakuna matata, claro... ");
						NBText_appendText(&text, "Pa");
						NBText_appendTextWithFont(&text, "la", NBFontBitmaps_getMetricsItfRef(font2, 0), 0);
						NBText_appendTextWithFont(&text, "bra.", NBFontBitmaps_getMetricsItfRef(font3, 0), 0);
					}
					NBText_popFormat(&text);
				}
				//Calculate metrics
				NBText_organizeText(&text, NBST_P(STNBRect, 0, 0, 512, 512 ), TRUE);
			}
			//Test metrics methods
			{
				UI32 txtLen = 0;
				const char* txt = NBText_getText(&text, &txtLen);
				NBTextMetricsBuilder_dbgTestAllSeqs(&text.builder, txtLen, 0);
				NBTextMetrics_dbgTestRangesQueries(&text.metrics, txt, txtLen, 0);
			}
			//Draw bitmap
			testTextMetrics_drawBitmap(&text.metrics, "testText_0_orig.png");
			//---------------
			//Remove chars and draw results
			//---------------
			{
				STNBString str01;
				NBString_init(&str01);
				{
					UI32 txtSz = 0;
					const char* txt = NBText_getText(&text, &txtSz);
					NBString_concatBytes(&str01, txt, txtSz);
				}
				const UI32 totalChars = text.metrics.chars.use;
				const STNBRangeI bytesRng = NBText_removeChars(&text, (totalChars / 3), (totalChars / 3), NULL);
				{
					//Print removed text
					{
						STNBString strRmvd;
						NBString_init(&strRmvd);
						NBString_concatBytes(&strRmvd, &str01.str[bytesRng.start], bytesRng.size);
						PRINTF_INFO("Removed text (%d, +%d): '%s'.\n", bytesRng.start, bytesRng.size, strRmvd.str);
						NBString_release(&strRmvd);
					}
					//Print new text and test metrics methods
					{
						STNBString strNew;
						NBString_init(&strNew);
						NBString_concatBytes(&strNew, str01.str, bytesRng.start);
						NBString_concatBytes(&strNew, &str01.str[bytesRng.start + bytesRng.size], str01.length - bytesRng.start - bytesRng.size);
						NBASSERT((strNew.length + bytesRng.size) == str01.length)
						PRINTF_INFO("New text: '%s'.\n", strNew.str);
						NBTextMetrics_dbgTestRangesQueries(&text.metrics, strNew.str, strNew.length, 0);
						NBString_release(&strNew);
					}
				}
				testTextMetrics_drawBitmap(&text.metrics, "testText_1_removed.png");
				//---------------
				//Rebuild metrics
				//---------------
				{
					//Rebuild metrics
					NBText_organizeText(&text, NBST_P(STNBRect, 0, 0, 512, 512 ), TRUE);
					//Test metrics methods
					{
						UI32 txtLen = 0;
						const char* txt = NBText_getText(&text, &txtLen);
						NBTextMetrics_dbgTestRangesQueries(&text.metrics, txt, txtLen, 0);
					}
					//Draw bitmap
					testTextMetrics_drawBitmap(&text.metrics, "testText_2_recalculated.png");
				}
				NBString_release(&str01);
			}
			//Release
			NBText_release(&text);
		}
	}
	NBFontsBitmapsStore_release(&bitmS);
	NBFontsLinesStore_release(&linesS);
	NBFontsGlyphsStore_release(&glyphsS);
	NBFilesystem_release(&fs);
}
*/

/*
void testTextMetrics_drawBitmap(const STNBTextMetrics* m, const char* filename){
	STNBBitmap bmp;
	NBBitmap_init(&bmp);
	NBBitmap_create(&bmp, 512, 512, ENNBBitmapColor_RGBA8);
	NBBitmap_clear(&bmp, 0, 0, 0, 255);
	//Draw chars area
	{
		UI32 iCurline = 0, iByte = 0;
		const STNBTextMetricsLine* curLine = NBArray_itmPtrAtIndex(&m->lines, STNBTextMetricsLine, iCurline);
		UI32 curLineAfterEnd = NBArray_itmPtrAtIndex(&m->words, STNBTextMetricsWord, curLine->rngWords.afterEnd - 1)->rngChars.afterEnd;
		UI32 i; for(i = 0; i < m->chars.use; i++){
			const STNBTextMetricsChar* c = NBArray_itmPtrAtIndex(&m->chars, STNBTextMetricsChar, i);
			const SI32 thisLeft = c->pos.x - c->extendsLeft;
			const SI32 thisRight = c->pos.x + c->extendsRight;
			NBASSERT(c->iByte <= iByte && c->bytesLen <= 8)
			//Change line if necesary
			while(i >= curLineAfterEnd){
				curLine = NBArray_itmPtrAtIndex(&m->lines, STNBTextMetricsLine, iCurline++);
				curLineAfterEnd = NBArray_itmPtrAtIndex(&m->words, STNBTextMetricsWord, curLine->rngWords.afterEnd - 1)->rngChars.afterEnd;
			}
			NBBitmap_clearArea(&bmp, NBST_P(STNBColor8, 0, 50, 0, 150), thisLeft, curLine->yBase - curLine->fontMetricsMax.ascender, (thisRight - thisLeft - 1), (curLine->fontMetricsMax.ascender - curLine->fontMetricsMax.descender - 1));
			iByte += c->bytesLen;
		}
	}
	//Draw base lines
	{
		UI32 i; for(i = 0; i < m->lines.use; i++){
			const STNBTextMetricsLine* l = NBArray_itmPtrAtIndex(&m->lines, STNBTextMetricsLine, i);
			NBBitmap_drawRect(&bmp, NBST_P(STNBRect,0, l->yBase, 512, 1), NBST_P(STNBColor8, 255, 0, 0, 255));
		}
	}
	//Draw cursors
	{
		UI32 i; for(i = 0; i <= m->chars.use; i++){
			const STNBRect r = NBTextMetrics_rectForCursor(m, i);
			NBBitmap_drawRect(&bmp, r, NBST_P(STNBColor8, 0, 255, 0, 255));
		}
	}
	//Draw chars
	{
		UI32 iByte = 0;
		UI32 i; for(i = 0; i < m->chars.use; i++){
			const STNBTextMetricsChar* c = NBArray_itmPtrAtIndex(&m->chars, STNBTextMetricsChar, i);
			NBASSERT(c->iByte <= iByte && c->bytesLen <= 8)
			STNBFontBitmapShape shape = NBFontBitmaps_getBitmapShape((STNBFontBitmaps*)c->itfObj, c->shapeId, TRUE);
			NBBitmap_pasteBitmapRect(&bmp, NBST_P(STNBPointI, c->pos.x + shape.center.x, c->pos.y + shape.center.y ), shape.bitmap, shape.area, NBST_P(STNBColor8, 255, 255, 255, 255 ));
			PRINTF_INFO("Printig char #%d/%d pos(%f, %f).\n", (i + 1), m->chars.use, c->pos.x, c->pos.y);
			iByte += c->bytesLen;
		}
	}
	NBPng_saveToPath(&bmp, filename, ENPngCompressLvl_3);
	NBBitmap_release(&bmp);
}
*/

//Test TextMetrics operations
/*{
	STNBFilesystem			fs;
	STNBFontsGlyphsStore	glyphsS;
	STNBFontsLinesStore		linesS;
	STNBFontsBitmapsStore	bitmS;
	NBFilesystem_init(&fs);
	NBFontsGlyphsStore_init(&glyphsS, &fs);
	NBFontsGlyphsStore_addSrchPath(&glyphsS, ".");
	NBFontsGlyphsStore_addSrchPath(&glyphsS, "Fuentes");
	NBFontsLinesStore_init(&linesS, &glyphsS);
	NBFontsBitmapsStore_init(&bitmS, &linesS);
	{
		//ENNBTextLineAlignH alineacionV = ENNBTextLineAlignH_Left;
		STNBFontBitmaps* font = NBFontsBitmapsStore_getFont(&bitmS, "Tangerine", "Regular", 0, 32, 1, ENNBFontSrchMode_flexible);
		STNBFontBitmaps* font2 = NBFontsBitmapsStore_getFont(&bitmS, "Roboto", "Regular", 0, 32, 1, ENNBFontSrchMode_flexible);
		STNBFontBitmaps* font3 = NBFontsBitmapsStore_getFont(&bitmS, "Homemade Apple", "Regular", 0, 32, 1, ENNBFontSrchMode_flexible);
		STNBFontMetricsIRef fonts[] = {
			NBFontBitmaps_getMetricsItfRef(font, 0)
			, NBFontBitmaps_getMetricsItfRef(font2, 0)
			, NBFontBitmaps_getMetricsItfRef(font3, 0)
		};
		const UI32 totalTests = 1000/ *0000* /, ciclesPerTest = 10, printLevel = 0;
		NBText_dbgTestRemoveAndReplace(totalTests, ciclesPerTest, fonts, (sizeof(fonts) / sizeof(fonts[0])), printLevel);
		PRINTF_INFO("NBText executed %d remove-replace-tests (%d cicles-per-test).\n", totalTests, ciclesPerTest);
	}
	NBFontsBitmapsStore_release(&bitmS);
	NBFontsLinesStore_release(&linesS);
	NBFontsGlyphsStore_release(&glyphsS);
	NBFilesystem_release(&fs);
}*/

