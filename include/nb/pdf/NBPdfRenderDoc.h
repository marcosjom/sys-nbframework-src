//
//  NBColor.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/12/18.
//

#ifndef NBPdfRenderDoc_h
#define NBPdfRenderDoc_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBCompare.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/2d/NBPoint.h"
#include "nb/2d/NBColor.h"
#include "nb/pdf/NBPdfRenderPage.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBPdfRenderDoc_ {
		STNBArray pages;	//STNBPdfRenderPage*
	} STNBPdfRenderDoc;
	
	void NBPdfRenderDoc_init(STNBPdfRenderDoc* obj);
	void NBPdfRenderDoc_release(STNBPdfRenderDoc* obj);
	
	STNBPdfRenderPage* NBPdfRenderDoc_addPage(STNBPdfRenderDoc* obj, const STNBSize size);
	STNBPdfRenderPage* NBPdfRenderDoc_addPagePdfTemplate(STNBPdfRenderDoc* obj, void* pageRef, NBPdfRenderPageItmPdfPageRetainMethd retainMethod, NBPdfRenderPageItmPdfPageReleaseMethd releaseMethod);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBPdfRenderDoc_h */
