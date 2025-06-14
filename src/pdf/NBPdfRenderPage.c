//
//  NBColor.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/12/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/pdf/NBPdfRenderPage.h"
//
#include "nb/core/NBMemory.h"

void NBPdfRenderPage_init(STNBPdfRenderPage* obj){
	NBMemory_setZeroSt(*obj, STNBPdfRenderPage);
	obj->size.width		= 612;
	obj->size.height	= 792;
	NBArray_init(&obj->itms, sizeof(STNBPdfRenderPageItm), NULL);
}

void NBPdfRenderPage_release(STNBPdfRenderPage* obj){
	{
		SI32 i; for(i = 0; i < obj->itms.use; i++){
			STNBPdfRenderPageItm* itm = NBArray_itmPtrAtIndex(&obj->itms, STNBPdfRenderPageItm, i);
			NBPdfRenderPageItm_release(itm);
		}
		NBArray_empty(&obj->itms);
		NBArray_release(&obj->itms);
	}
	//PdfPageTemplate
	{
		if(obj->sizeTemplate.releaseMethod != NULL){
			(*obj->sizeTemplate.releaseMethod)(obj->sizeTemplate.pageRef);
			obj->sizeTemplate.releaseMethod = NULL;
		}
	}
}

//

void NBPdfRenderPage_setSize(STNBPdfRenderPage* obj, const STNBSize size){
	obj->size = size;
}

void NBPdfRenderPage_setSizeFromPagePdfTemplate(STNBPdfRenderPage* obj, void* pageRef, NBPdfRenderPageItmPdfPageRetainMethd retainMethod, NBPdfRenderPageItmPdfPageReleaseMethd releaseMethod){
	//Release previous
	{
		if(obj->sizeTemplate.releaseMethod != NULL){
			(*obj->sizeTemplate.releaseMethod)(obj->sizeTemplate.pageRef);
		}
	}
	//Retain new
	{
		obj->sizeTemplate.pageRef		= pageRef;
		obj->sizeTemplate.retainMethod	= retainMethod;
		obj->sizeTemplate.releaseMethod	= releaseMethod;
		//Retain
		if(obj->sizeTemplate.retainMethod != NULL){
			(*obj->sizeTemplate.retainMethod)(obj->sizeTemplate.pageRef);
		}
	}
}

void NBPdfRenderPage_addText(STNBPdfRenderPage* obj, const STNBPoint traslation, const float rotDeg, const char* fontName, const float fontSz, const STNBColor8 fontColor, const char* text){
	STNBPdfRenderPageItm itm;
	NBPdfRenderPageItm_init(&itm);
	NBPdfRenderPageItm_setTransform(&itm, traslation, rotDeg);
	NBPdfRenderPageItm_setAsText(&itm, fontName, fontSz, fontColor, text);
	NBArray_addValue(&obj->itms, itm);
}

void NBPdfRenderPage_addImage(STNBPdfRenderPage* obj, const STNBPoint traslation, const float rotDeg, const STNBRect rect, const ENNBPdfRenderPageItmImgType imgType, const void* imgData, const UI32 imgDataSz){
	STNBPdfRenderPageItm itm;
	NBPdfRenderPageItm_init(&itm);
	NBPdfRenderPageItm_setTransform(&itm, traslation, rotDeg);
	NBPdfRenderPageItm_setAsImage(&itm, rect, imgType, imgData, imgDataSz);
	NBArray_addValue(&obj->itms, itm);
}

void NBPdfRenderPage_addPdfPage(STNBPdfRenderPage* obj, const STNBPoint traslation, const float rotDeg, void* pageRef, NBPdfRenderPageItmPdfPageRetainMethd retainMethod, NBPdfRenderPageItmPdfPageReleaseMethd releaseMethod){
	STNBPdfRenderPageItm itm;
	NBPdfRenderPageItm_init(&itm);
	NBPdfRenderPageItm_setTransform(&itm, traslation, rotDeg);
	NBPdfRenderPageItm_setAsPdfPage(&itm, pageRef, retainMethod, releaseMethod);
	NBArray_addValue(&obj->itms, itm);
}
