//
//  NBFontsLinesStore.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/10/18.
//

#ifndef NBFontsLinesStore_h
#define NBFontsLinesStore_h

#include "nb/NBFrameworkDefs.h"
#include "nb/fonts/NBFontsGlyphsStore.h"
#include "nb/fonts/NBFontGlyphs.h"
#include "nb/fonts/NBFontLines.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBFontsLinesStore_ {
		void* opaque;
	} STNBFontsLinesStore;
	
	void NBFontsLinesStore_init(STNBFontsLinesStore* obj, STNBFontsGlyphsStore* glyphsStore);
	void NBFontsLinesStore_retain(STNBFontsLinesStore* obj);
	void NBFontsLinesStore_release(STNBFontsLinesStore* obj);
	
	//Fonts
	STNBFontLines* NBFontsLinesStore_getFont(STNBFontsLinesStore* obj, const char* family, const char* subfamily, const UI8 styleMask, const ENNBFontSrchMode srchMode);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBFontsLinesStore_h */
