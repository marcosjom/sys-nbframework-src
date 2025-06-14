//
//  NBFontsLinesStore.c
//  
//
//  Created by Marcos Ortega on 8/10/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/fonts/NBFontsLinesStore.h"

typedef struct STNBFontsLinesStoreFont_ {
	STNBFontLines	font;
	UI32			family;
	UI32			subfamily;
	UI8				styleMask;	//ENNBFontStyleBit
} STNBFontsLinesStoreFont;

typedef struct STNBFontsLinesStoreOpq_ {
	STNBFontsGlyphsStore* glyphsStore;
	STNBArray	fonts;	//STNBFontsLinesStoreFont*
	STNBString	strs;
	UI32		retainCount;
} STNBFontsLinesStoreOpq;

void NBFontsLinesStore_init(STNBFontsLinesStore* obj, STNBFontsGlyphsStore* glyphsStore){
	STNBFontsLinesStoreOpq* opq = obj->opaque = NBMemory_allocType(STNBFontsLinesStoreOpq);
	NBMemory_setZeroSt(*opq, STNBFontsLinesStoreOpq);
	opq->glyphsStore	= glyphsStore; if(glyphsStore != NULL) NBFontsGlyphsStore_retain(glyphsStore);
	NBArray_init(&opq->fonts, sizeof(STNBFontsLinesStoreFont*), NULL);
	NBString_init(&opq->strs);
	NBString_concatByte(&opq->strs, '\0');
	opq->retainCount	= 1;
}

void NBFontsLinesStore_retain(STNBFontsLinesStore* obj){
	STNBFontsLinesStoreOpq* opq = (STNBFontsLinesStoreOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount++;
}

void NBFontsLinesStore_release(STNBFontsLinesStore* obj){
	STNBFontsLinesStoreOpq* opq = (STNBFontsLinesStoreOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount--;
	if(opq->retainCount == 0){
		{
			UI32 i; for(i = 0; i < opq->fonts.use; i++){
				STNBFontsLinesStoreFont* font = NBArray_itmValueAtIndex(&opq->fonts, STNBFontsLinesStoreFont*, i);
				NBFontLines_release(&font->font);
				NBMemory_free(font);
			}
			NBArray_empty(&opq->fonts);
			NBArray_release(&opq->fonts);
		}
		if(opq->glyphsStore != NULL){
			NBFontsGlyphsStore_release(opq->glyphsStore);
			opq->glyphsStore = NULL;
		}
		NBString_release(&opq->strs);
		//
		NBMemory_free(obj->opaque);
		obj->opaque = NULL;
	}
}

//Fonts

STNBFontLines* NBFontsLinesStore_getFont(STNBFontsLinesStore* obj, const char* family, const char* subfamily, const UI8 styleMask, const ENNBFontSrchMode srchMode){
	STNBFontLines* r = NULL;
	STNBFontsLinesStoreOpq* opq = (STNBFontsLinesStoreOpq*)obj->opaque;
	//Search font
	{
		UI32 i; for(i = 0; i < opq->fonts.use; i++){
			STNBFontsLinesStoreFont* font = NBArray_itmValueAtIndex(&opq->fonts, STNBFontsLinesStoreFont*, i);
			const char* fFamily		= &opq->strs.str[font->family];
			if(NBString_strIsEqual(fFamily, family)){
				if(subfamily != NULL){
					const char* fSubfamily	= &opq->strs.str[font->subfamily];
					if(NBString_strIsEqual(fSubfamily, subfamily)){
						r = &font->font;
						break;
					}
				} else if(font->styleMask == styleMask){
					r = &font->font;
					break;
				}
			}
		}
	}
	//Load font
	if(r == NULL){
		STNBFontGlyphs* glyphs = NBFontsGlyphsStore_getFont(opq->glyphsStore, family, subfamily, styleMask, srchMode);
		if(glyphs != NULL){
			STNBFontsLinesStoreFont* font = NBMemory_allocType(STNBFontsLinesStoreFont);
			font->family	= opq->strs.length; NBString_concat(&opq->strs, family); NBString_concatByte(&opq->strs, '\0');
			font->subfamily	= opq->strs.length; NBString_concat(&opq->strs, subfamily); NBString_concatByte(&opq->strs, '\0');
			font->styleMask	= styleMask;
			NBFontLines_init(&font->font, glyphs);
			NBArray_addValue(&opq->fonts, font);
			r = &font->font;
		}
	}
	return r;
}
