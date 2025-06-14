//
//  NBColor.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/12/18.
//

#ifndef NBPdfRenderPageItm_h
#define NBPdfRenderPageItm_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBCompare.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBString.h"
#include "nb/2d/NBPoint.h"
#include "nb/2d/NBRect.h"
#include "nb/2d/NBColor.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef enum ENNBPdfRenderPageItmType_ {
		ENNBPdfRenderPageItmType_Text = 0,	//Text
		ENNBPdfRenderPageItmType_Image,		//Image
		ENNBPdfRenderPageItmType_PdfPage,	//Other PDF page
		ENNBPdfRenderPageItmType_Count
	} ENNBPdfRenderPageItmType;
	
	typedef enum ENNBPdfRenderPageItmImgType_ {
		ENNBPdfRenderPageItmImgType_Png = 0,
		ENNBPdfRenderPageItmImgType_Jpeg,
		ENNBPdfRenderPageItmImgType_Count
	} ENNBPdfRenderPageItmImgType;
	
	typedef	void (*NBPdfRenderPageItmPdfPageRetainMethd)(void* pageRef);
	typedef	void (*NBPdfRenderPageItmPdfPageReleaseMethd)(void* pageRef);
	
	typedef struct STNBPdfRenderPageItm_ {
		ENNBPdfRenderPageItmType	type;
		struct {
			STNBPoint		traslation;
			float			rotDeg;
		} matrix;
		union {
			struct {
				STNBString	fontName;
				float		fontSize;
				STNBColor8	fontColor;
				STNBString	text;
			} text;
			struct {
				ENNBPdfRenderPageItmImgType type;
				STNBRect	rect;
				STNBString	data;
			} image;
			struct {
				void*		pageRef;
				NBPdfRenderPageItmPdfPageRetainMethd retainMethod;
				NBPdfRenderPageItmPdfPageReleaseMethd releaseMethod;
			} pdfPage;
		};
	} STNBPdfRenderPageItm;
	
	
	void NBPdfRenderPageItm_init(STNBPdfRenderPageItm* obj);
	void NBPdfRenderPageItm_release(STNBPdfRenderPageItm* obj);
	//
	void NBPdfRenderPageItm_setTransform(STNBPdfRenderPageItm* obj, const STNBPoint traslation, const float rotDeg);
	void NBPdfRenderPageItm_setAsText(STNBPdfRenderPageItm* obj, const char* fontName, const float fontSz, const STNBColor8 fontColor, const char* text);
	void NBPdfRenderPageItm_setAsImage(STNBPdfRenderPageItm* obj, const STNBRect rect, const ENNBPdfRenderPageItmImgType imgType, const void* imgData, const UI32 imgDataSz);
	void NBPdfRenderPageItm_setAsPdfPage(STNBPdfRenderPageItm* obj, void* pageRef, NBPdfRenderPageItmPdfPageRetainMethd retainMethod, NBPdfRenderPageItmPdfPageReleaseMethd releaseMethod);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBPdfRenderPageItm_h */
