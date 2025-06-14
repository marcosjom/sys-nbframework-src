//
//  NBFontsBitmapsStore.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/11/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/fonts/NBFontsBitmapsStore.h"

typedef struct STNBFontsBitmapsStoreFont_ {
	STNBFontBitmaps	font;
	UI32			family;
	UI32			subfamily;
	UI8				styleMask;	//ENNBFontStyleBit
} STNBFontsBitmapsStoreFont;

typedef struct STNBFontsBitmapsStoreOpq_ {
	STNBFontsLinesStore* linesStore;
	STNBArray	fonts;	//STNBFontsBitmapsStoreFont*
	STNBString	strs;
	UI32		retainCount;
} STNBFontsBitmapsStoreOpq;

void NBFontsBitmapsStore_init(STNBFontsBitmapsStore* obj, STNBFontsLinesStore* linesStore){
	STNBFontsBitmapsStoreOpq* opq = obj->opaque = NBMemory_allocType(STNBFontsBitmapsStoreOpq);
	NBMemory_setZeroSt(*opq, STNBFontsBitmapsStoreOpq);
	opq->linesStore	= linesStore; if(linesStore != NULL) NBFontsLinesStore_retain(linesStore);
	NBArray_init(&opq->fonts, sizeof(STNBFontsBitmapsStoreFont*), NULL);
	NBString_init(&opq->strs);
	NBString_concatByte(&opq->strs, '\0');
	opq->retainCount	= 1;
}

void NBFontsBitmapsStore_retain(STNBFontsBitmapsStore* obj){
	STNBFontsBitmapsStoreOpq* opq = (STNBFontsBitmapsStoreOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount++;
}

void NBFontsBitmapsStore_release(STNBFontsBitmapsStore* obj){
	STNBFontsBitmapsStoreOpq* opq = (STNBFontsBitmapsStoreOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount--;
	if(opq->retainCount == 0){
		{
			UI32 i; for(i = 0; i < opq->fonts.use; i++){
				STNBFontsBitmapsStoreFont* font = NBArray_itmValueAtIndex(&opq->fonts, STNBFontsBitmapsStoreFont*, i);
				NBFontBitmaps_release(&font->font);
				NBMemory_free(font);
			}
			NBArray_empty(&opq->fonts);
			NBArray_release(&opq->fonts);
		}
		if(opq->linesStore != NULL){
			NBFontsLinesStore_release(opq->linesStore);
			opq->linesStore = NULL;
		}
		NBString_release(&opq->strs);
		//
		NBMemory_free(obj->opaque);
		obj->opaque = NULL;
	}
}

//Fonts

STNBFontBitmaps* NBFontsBitmapsStore_getFont(STNBFontsBitmapsStore* obj, const char* family, const char* subfamily, const UI8 styleMask, const float fontSz, const UI16 shapesMargin, const ENNBFontSrchMode srchMode){
	STNBFontBitmaps* r = NULL;
	STNBFontsBitmapsStoreOpq* opq = (STNBFontsBitmapsStoreOpq*)obj->opaque;
	//Search font
	{
		UI32 i; for(i = 0; i < opq->fonts.use; i++){
			STNBFontsBitmapsStoreFont* font = NBArray_itmValueAtIndex(&opq->fonts, STNBFontsBitmapsStoreFont*, i);
			const char* fFamily		= &opq->strs.str[font->family];
			const float fSize		= NBFontBitmaps_getSize(&font->font);
			if(fSize == fontSz){
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
	}
	//Load font
	if(r == NULL){
		STNBFontLines* lines = NBFontsLinesStore_getFont(opq->linesStore, family, subfamily, styleMask, srchMode);
		if(lines != NULL){
			STNBFontsBitmapsStoreFont* font = NBMemory_allocType(STNBFontsBitmapsStoreFont);
			font->family	= opq->strs.length; NBString_concat(&opq->strs, family); NBString_concatByte(&opq->strs, '\0');
			font->subfamily	= opq->strs.length; NBString_concat(&opq->strs, subfamily); NBString_concatByte(&opq->strs, '\0');
			font->styleMask	= styleMask;
			NBFontBitmaps_initWithSz(&font->font, lines, fontSz, shapesMargin);
			NBArray_addValue(&opq->fonts, font);
			r = &font->font;
		}
	}
	return r;
}


