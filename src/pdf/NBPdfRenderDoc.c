//
//  NBColor.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/12/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/pdf/NBPdfRenderDoc.h"
//
#include "nb/core/NBMemory.h"

void NBPdfRenderDoc_init(STNBPdfRenderDoc* obj){
	NBArray_init(&obj->pages, sizeof(STNBPdfRenderPage*), NULL);
}

void NBPdfRenderDoc_release(STNBPdfRenderDoc* obj){
	{
		SI32 i; for(i = 0; i < obj->pages.use; i++){
			STNBPdfRenderPage* page = NBArray_itmValueAtIndex(&obj->pages, STNBPdfRenderPage*, i);
			if(page != NULL){
				NBPdfRenderPage_release(page);
				NBMemory_free(page);
				page = NULL;
			}
		}
		NBArray_empty(&obj->pages);
		NBArray_release(&obj->pages);
	}
}

//

STNBPdfRenderPage* NBPdfRenderDoc_addPage(STNBPdfRenderDoc* obj, const STNBSize size){
	STNBPdfRenderPage* r = NULL;
	{
		STNBPdfRenderPage* page = NBMemory_allocType(STNBPdfRenderPage);
		NBPdfRenderPage_init(page);
		NBPdfRenderPage_setSize(page, size);
		NBArray_addValue(&obj->pages, page);
		r = page;
	}
	return r;
}


STNBPdfRenderPage* NBPdfRenderDoc_addPagePdfTemplate(STNBPdfRenderDoc* obj, void* pageRef, NBPdfRenderPageItmPdfPageRetainMethd retainMethod, NBPdfRenderPageItmPdfPageReleaseMethd releaseMethod){
	STNBPdfRenderPage* r = NULL;
	{
		STNBPdfRenderPage* page = NBMemory_allocType(STNBPdfRenderPage);
		NBPdfRenderPage_init(page);
		NBPdfRenderPage_setSizeFromPagePdfTemplate(page, pageRef, retainMethod, releaseMethod);
		NBArray_addValue(&obj->pages, page);
		r = page;
	}
	return r;
}
