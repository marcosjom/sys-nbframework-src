//
//  NBSmtpHeader.h
//  nbframework
//
//  Created by Marcos Ortega on 28/2/19.
//

#ifndef NBSmtpHeader_h
#define NBSmtpHeader_h

#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"

typedef struct STNBSmtpHeader_ {
	UI32		FROM;	//UI32, idx in "strs"
	UI32		SUBJECT; //UI32, idx in "strs"
	STNBArray	TOs;	//UI32, idx in "strs"
	STNBArray	CCs;	//UI32, idx in "strs"
	STNBArray	CCOs;	//UI32, idx in "strs"
	STNBString	strs;
} STNBSmtpHeader;

void NBSmtpHeader_init(STNBSmtpHeader* obj);
void NBSmtpHeader_release(STNBSmtpHeader* obj);

const char* NBSmtpHeader_getFROM(const STNBSmtpHeader* obj);
const char* NBSmtpHeader_getSUBJECT(const STNBSmtpHeader* obj);
const char* NBSmtpHeader_getTO(const STNBSmtpHeader* obj, const SI32 i);
const char* NBSmtpHeader_getCC(const STNBSmtpHeader* obj, const SI32 i);
const char* NBSmtpHeader_getCCO(const STNBSmtpHeader* obj, const SI32 i);

BOOL NBSmtpHeader_setFROM(STNBSmtpHeader* obj, const char* from);
BOOL NBSmtpHeader_setSUBJECT(STNBSmtpHeader* obj, const char* subject);
BOOL NBSmtpHeader_addTO(STNBSmtpHeader* obj, const char* to);
BOOL NBSmtpHeader_addCC(STNBSmtpHeader* obj, const char* cc);
BOOL NBSmtpHeader_addCCO(STNBSmtpHeader* obj, const char* cco);

#endif /* NBSmtpHeader_h */
