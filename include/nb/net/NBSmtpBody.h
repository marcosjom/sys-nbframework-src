//
//  NBSmtpBody.h
//  nbframework
//
//  Created by Marcos Ortega on 28/2/19.
//

#ifndef NBSmtpBody_h
#define NBSmtpBody_h

#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"

typedef struct STNBSmtpBodyAttach_ {
	UI32		mimeType;		//UI32, idx in "strs" (content-type)
	UI32		contentDisp;	//UI32, idx in "strs" (content-disposition)
	UI32		contentId;		//UI32, idx in "strs" (content-id, for internal references)
	UI32		filename;		//UI32, idx in "strs" (content disposition)
	UI32		dataBin;		//Data binary
	UI32		dataBinSz;		//Data binary size
} STNBSmtpBodyAttach;

typedef struct STNBSmtpBody_ {
	UI32		boundary;	//UI32, idx in "strs"
	UI32		mimeType;	//UI32, idx in "strs"
	UI32		content;	//UI32, idx in "strs"
	STNBArray	attachs;	//STNBSmtpBodyAttach
	STNBString	strs;
} STNBSmtpBody;

void NBSmtpBody_init(STNBSmtpBody* obj);
void NBSmtpBody_release(STNBSmtpBody* obj);

const char* NBSmtpBody_getBoundary(const STNBSmtpBody* obj);
const char* NBSmtpBody_getMimeType(const STNBSmtpBody* obj);
const char* NBSmtpBody_getContent(const STNBSmtpBody* obj);

BOOL NBSmtpBody_setBoundary(STNBSmtpBody* obj, const char* boundary);
BOOL NBSmtpBody_setMimeType(STNBSmtpBody* obj, const char* mimeType);
BOOL NBSmtpBody_setContent(STNBSmtpBody* obj, const char* content);

BOOL NBSmtpBody_addAttach(STNBSmtpBody* obj, const char* mimeType, const char* contentDisp, const char* contentId, const char* filename, const void* dataBin, const UI32 dataBinSz);

#endif /* NBSmtpBody_h */
