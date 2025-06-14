//
//  NBSmtpHeader.c
//  nbframework
//
//  Created by Marcos Ortega on 28/2/19.
//

#include "nb/NBFrameworkPch.h"
#include "nb/net/NBSmtpHeader.h"
//

void NBSmtpHeader_init(STNBSmtpHeader* obj){
	obj->FROM		= 0;
	obj->SUBJECT	= 0;
	NBArray_init(&obj->TOs, sizeof(UI32), NULL);
	NBArray_init(&obj->CCs, sizeof(UI32), NULL);
	NBArray_init(&obj->CCOs, sizeof(UI32), NULL);
	NBString_init(&obj->strs);
	//Index zero is empty string
	NBString_concatByte(&obj->strs, '\0');
}

void NBSmtpHeader_release(STNBSmtpHeader* obj){
	obj->FROM		= 0;
	obj->SUBJECT	= 0;
	NBArray_release(&obj->TOs);
	NBArray_release(&obj->CCs);
	NBArray_release(&obj->CCOs);
	NBString_release(&obj->strs);
}

//

const char* NBSmtpHeader_getFROM(const STNBSmtpHeader* obj){
	return &obj->strs.str[obj->FROM];
}

const char* NBSmtpHeader_getSUBJECT(const STNBSmtpHeader* obj){
	return &obj->strs.str[obj->SUBJECT];
}

const char* NBSmtpHeader_getTO(const STNBSmtpHeader* obj, const SI32 i){
	const char* r = NULL;
	if(i >= 0 && i < obj->TOs.use){
		const UI32 idx = NBArray_itmValueAtIndex(&obj->TOs, UI32, i);
		r = &obj->strs.str[idx];
	}
	return r;
}

const char* NBSmtpHeader_getCC(const STNBSmtpHeader* obj, const SI32 i){
	const char* r = NULL;
	if(i >= 0 && i < obj->CCs.use){
		const UI32 idx = NBArray_itmValueAtIndex(&obj->CCs, UI32, i);
		r = &obj->strs.str[idx];
	}
	return r;
}

const char* NBSmtpHeader_getCCO(const STNBSmtpHeader* obj, const SI32 i){
	const char* r = NULL;
	if(i >= 0 && i < obj->CCOs.use){
		const UI32 idx = NBArray_itmValueAtIndex(&obj->CCOs, UI32, i);
		r = &obj->strs.str[idx];
	}
	return r;
}

//

BOOL NBSmtpHeader_setFROM(STNBSmtpHeader* obj, const char* from){
	obj->FROM = obj->strs.length;
	NBString_concat(&obj->strs, from);
	NBString_concatByte(&obj->strs, '\0');
	return TRUE;
}

BOOL NBSmtpHeader_setSUBJECT(STNBSmtpHeader* obj, const char* subject){
	obj->SUBJECT = obj->strs.length;
	NBString_concat(&obj->strs, subject);
	NBString_concatByte(&obj->strs, '\0');
	return TRUE;
}

BOOL NBSmtpHeader_addTO(STNBSmtpHeader* obj, const char* to){
	const UI32 idx = obj->strs.length;
	NBString_concat(&obj->strs, to);
	NBString_concatByte(&obj->strs, '\0');
	NBArray_addValue(&obj->TOs, idx);
	return TRUE;
}

BOOL NBSmtpHeader_addCC(STNBSmtpHeader* obj, const char* cc){
	const UI32 idx = obj->strs.length;
	NBString_concat(&obj->strs, cc);
	NBString_concatByte(&obj->strs, '\0');
	NBArray_addValue(&obj->CCs, idx);
	return TRUE;
}

BOOL NBSmtpHeader_addCCO(STNBSmtpHeader* obj, const char* cco){
	const UI32 idx = obj->strs.length;
	NBString_concat(&obj->strs, cco);
	NBString_concatByte(&obj->strs, '\0');
	NBArray_addValue(&obj->CCOs, idx);
	return TRUE;
}
