//
//  NBSmtpBody.c
//  nbframework
//
//  Created by Marcos Ortega on 28/2/19.
//

#include "nb/NBFrameworkPch.h"
#include "nb/net/NBSmtpBody.h"
//

void NBSmtpBody_init(STNBSmtpBody* obj){
	obj->boundary	= 0;
	obj->mimeType	= 0;
	obj->content	= 0;
	NBArray_init(&obj->attachs, sizeof(STNBSmtpBodyAttach), NULL);
	NBString_init(&obj->strs);
	//Index zero is empty string
	NBString_concatByte(&obj->strs, '\0');
	//Set defaults
	NBSmtpBody_setBoundary(obj, "NBBoundaryMIME");
	NBSmtpBody_setMimeType(obj, "text/plain");
}

void NBSmtpBody_release(STNBSmtpBody* obj){
	obj->boundary	= 0;
	obj->mimeType	= 0;
	obj->content	= 0;
	NBArray_release(&obj->attachs);
	NBString_release(&obj->strs);
}

//

const char* NBSmtpBody_getBoundary(const STNBSmtpBody* obj){
	return &obj->strs.str[obj->boundary];
}

const char* NBSmtpBody_getMimeType(const STNBSmtpBody* obj){
	return &obj->strs.str[obj->mimeType];
}

const char* NBSmtpBody_getContent(const STNBSmtpBody* obj){
	return &obj->strs.str[obj->content];
}

//

BOOL NBSmtpBody_setBoundary(STNBSmtpBody* obj, const char* boundary){
	obj->boundary = obj->strs.length;
	NBString_concat(&obj->strs, boundary);
	NBString_concatByte(&obj->strs, '\0');
	return TRUE;
}

BOOL NBSmtpBody_setMimeType(STNBSmtpBody* obj, const char* mimeType){
	obj->mimeType = obj->strs.length;
	NBString_concat(&obj->strs, mimeType);
	NBString_concatByte(&obj->strs, '\0');
	return TRUE;
}

BOOL NBSmtpBody_setContent(STNBSmtpBody* obj, const char* content){
	obj->content = obj->strs.length;
	NBString_concat(&obj->strs, content);
	NBString_concatByte(&obj->strs, '\0');
	return TRUE;
}

//

BOOL NBSmtpBody_addAttach(STNBSmtpBody* obj, const char* mimeType, const char* contentDisp, const char* contentId, const char* filename, const void* dataBin, const UI32 dataBinSz){
	BOOL r = FALSE;
	if(dataBin != NULL && dataBinSz > 0){
		STNBSmtpBodyAttach aa;
		NBMemory_setZeroSt(aa, STNBSmtpBodyAttach);
		if(!NBString_strIsEmpty(mimeType)){
			aa.mimeType = obj->strs.length;
			NBString_concat(&obj->strs, mimeType);
			NBString_concatByte(&obj->strs, '\0');
		}
		if(!NBString_strIsEmpty(contentDisp)){
			aa.contentDisp = obj->strs.length;
			NBString_concat(&obj->strs, contentDisp);
			NBString_concatByte(&obj->strs, '\0');
		}
		if(!NBString_strIsEmpty(contentId)){
			aa.contentId = obj->strs.length;
			NBString_concat(&obj->strs, contentId);
			NBString_concatByte(&obj->strs, '\0');
		}
		if(!NBString_strIsEmpty(filename)){
			aa.filename = obj->strs.length;
			NBString_concat(&obj->strs, filename);
			NBString_concatByte(&obj->strs, '\0');
		}
		{
			aa.dataBin		= obj->strs.length;
			aa.dataBinSz	= dataBinSz;
			NBString_concatBytes(&obj->strs, dataBin, dataBinSz);
		}
		NBArray_addValue(&obj->attachs, aa);
	}
	return r;
}
