//
//  NBFontsGlyphsStore.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/8/18.
//

#ifndef NBFontsGlyphsStore_h
#define NBFontsGlyphsStore_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/files/NBFilesystem.h"
#include "nb/fonts/NBFontGlyphs.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef enum ENNBFontSrchMode_ {
		ENNBFontSrchMode_flexible = 0,
		ENNBFontSrchMode_exact
	} ENNBFontSrchMode;
	
	typedef struct STNBFontsGlyphsStore_ {
		void* opaque;
	} STNBFontsGlyphsStore;
	
	void NBFontsGlyphsStore_init(STNBFontsGlyphsStore* obj, STNBFilesystem* filesystem);
	void NBFontsGlyphsStore_retain(STNBFontsGlyphsStore* obj);
	void NBFontsGlyphsStore_release(STNBFontsGlyphsStore* obj);
	
	//Search paths
	void NBFontsGlyphsStore_addSrchPath(STNBFontsGlyphsStore* obj, const char* srchPath);
	
	//Fonts
	STNBFontGlyphs* NBFontsGlyphsStore_getFont(STNBFontsGlyphsStore* obj, const char* family, const char* subfamily, const UI8 styleMask, const ENNBFontSrchMode srchMode);
	
#ifdef __cplusplus
} //extern "C"
#endif
	
#endif /* NBFontsGlyphsStore_h */
