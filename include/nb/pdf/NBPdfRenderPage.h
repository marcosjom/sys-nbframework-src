//
//  NBColor.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/12/18.
//

#ifndef NBPdfRenderPage_h
#define NBPdfRenderPage_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBCompare.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/2d/NBPoint.h"
#include "nb/2d/NBSize.h"
#include "nb/2d/NBColor.h"
#include "nb/pdf/NBPdfRenderPageItm.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBPdfRenderPage_ {
		STNBSize	size;
		STNBArray	itms;	//STNBPdfRenderPageItm
		//
		struct {
			void*	pageRef;
			NBPdfRenderPageItmPdfPageRetainMethd retainMethod;
			NBPdfRenderPageItmPdfPageReleaseMethd releaseMethod;
		} sizeTemplate;
	} STNBPdfRenderPage;
	
	
	void NBPdfRenderPage_init(STNBPdfRenderPage* obj);
	void NBPdfRenderPage_release(STNBPdfRenderPage* obj);
	
	void NBPdfRenderPage_setSize(STNBPdfRenderPage* obj, const STNBSize size);
	void NBPdfRenderPage_setSizeFromPagePdfTemplate(STNBPdfRenderPage* obj, void* pageRef, NBPdfRenderPageItmPdfPageRetainMethd retainMethod, NBPdfRenderPageItmPdfPageReleaseMethd releaseMethod);
	void NBPdfRenderPage_addText(STNBPdfRenderPage* obj, const STNBPoint traslation, const float rotDeg, const char* fontName, const float fontSz, const STNBColor8 fontColor, const char* text);
	void NBPdfRenderPage_addImage(STNBPdfRenderPage* obj, const STNBPoint traslation, const float rotDeg, const STNBRect rect, const ENNBPdfRenderPageItmImgType imgType, const void* imgData, const UI32 imgDataSz);
	void NBPdfRenderPage_addPdfPage(STNBPdfRenderPage* obj, const STNBPoint traslation, const float rotDeg, void* pageRef, NBPdfRenderPageItmPdfPageRetainMethd retainMethod, NBPdfRenderPageItmPdfPageReleaseMethd releaseMethod);
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBPdfRenderPage_h */
