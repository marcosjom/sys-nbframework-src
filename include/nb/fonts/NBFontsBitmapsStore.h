//
//  NBFontsBitmapsStore.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/11/18.
//

#ifndef NBFontsBitmapsStore_h
#define NBFontsBitmapsStore_h

#include "nb/NBFrameworkDefs.h"
#include "nb/fonts/NBFontsLinesStore.h"
#include "nb/fonts/NBFontBitmaps.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBFontsBitmapsStore_ {
		void* opaque;
	} STNBFontsBitmapsStore;
	
	void NBFontsBitmapsStore_init(STNBFontsBitmapsStore* obj, STNBFontsLinesStore* linesStore);
	void NBFontsBitmapsStore_retain(STNBFontsBitmapsStore* obj);
	void NBFontsBitmapsStore_release(STNBFontsBitmapsStore* obj);
	
	//Fonts
	STNBFontBitmaps* NBFontsBitmapsStore_getFont(STNBFontsBitmapsStore* obj, const char* family, const char* subfamily, const UI8 styleMask, const float fontSz, const UI16 shapesMargin, const ENNBFontSrchMode srchMode);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBFontsBitmapsStore_h */
