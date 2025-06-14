//
//  NBColor.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/12/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/pdf/NBPdfRenderPageItm.h"
//
#include "nb/core/NBMemory.h"

void NBPdfRenderPageItm_init(STNBPdfRenderPageItm* obj){
	NBMemory_setZeroSt(*obj, STNBPdfRenderPageItm);
	obj->type	= ENNBPdfRenderPageItmType_Count;
}

void NBPdfRenderPageItm_release(STNBPdfRenderPageItm* obj){
	switch(obj->type) {
		case ENNBPdfRenderPageItmType_Text:
			{
				NBString_release(&obj->text.fontName);
				NBString_release(&obj->text.text);
			}
			break;
		case ENNBPdfRenderPageItmType_Image:
			{
				NBString_release(&obj->image.data);
			}
			break;
		case ENNBPdfRenderPageItmType_PdfPage:
			{
				if(obj->pdfPage.releaseMethod != NULL){
					(*obj->pdfPage.releaseMethod)(obj->pdfPage.pageRef);
				}
			}
			break;
		default:
			//
			break;
	}
	obj->type = ENNBPdfRenderPageItmType_Count;
}

void NBPdfRenderPageItm_setTransform(STNBPdfRenderPageItm* obj, const STNBPoint traslation, const float rotDeg){
	obj->matrix.traslation	= traslation;
	obj->matrix.rotDeg		= rotDeg;
}

void NBPdfRenderPageItm_setAsText(STNBPdfRenderPageItm* obj, const char* fontName, const float fontSz, const STNBColor8 fontColor, const char* text){
	NBASSERT(obj->type == ENNBPdfRenderPageItmType_Count)
	if(obj->type == ENNBPdfRenderPageItmType_Count){
		obj->type = ENNBPdfRenderPageItmType_Text;
		{
			NBString_initWithStr(&obj->text.fontName, fontName);
			NBString_initWithStr(&obj->text.text, text);
			obj->text.fontSize	= fontSz;
			obj->text.fontColor	= fontColor;
		}
	}
}

void NBPdfRenderPageItm_setAsImage(STNBPdfRenderPageItm* obj, const STNBRect rect, const ENNBPdfRenderPageItmImgType imgType, const void* imgData, const UI32 imgDataSz){
	NBASSERT(obj->type == ENNBPdfRenderPageItmType_Count)
	if(obj->type == ENNBPdfRenderPageItmType_Count){
		obj->type = ENNBPdfRenderPageItmType_Image;
		{
			obj->image.type = imgType;
			obj->image.rect	= rect;
			NBString_initWithStrBytes(&obj->image.data, (const char*) imgData, imgDataSz);
		}
	}
}

void NBPdfRenderPageItm_setAsPdfPage(STNBPdfRenderPageItm* obj, void* pageRef, NBPdfRenderPageItmPdfPageRetainMethd retainMethod, NBPdfRenderPageItmPdfPageReleaseMethd releaseMethod){
	NBASSERT(obj->type == ENNBPdfRenderPageItmType_Count)
	if(obj->type == ENNBPdfRenderPageItmType_Count){
		obj->type = ENNBPdfRenderPageItmType_PdfPage;
		{
			obj->pdfPage.pageRef		= pageRef;
			obj->pdfPage.retainMethod	= retainMethod;
			obj->pdfPage.releaseMethod	= releaseMethod;
			//Retain
			if(obj->pdfPage.retainMethod != NULL){
				(*obj->pdfPage.retainMethod)(obj->pdfPage.pageRef);
			}
		}
	}
}
