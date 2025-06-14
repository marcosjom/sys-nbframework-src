//
//  NBStructMapMember.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/28/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBStructMapMember.h"
#include "nb/core/NBMemory.h"

//Map's member
void NBStructMapMember_init(STNBStructMapMember* obj, const char* name){
	NBMemory_set(obj, 0, sizeof(*obj));
	//copy name
	{
		const UI32 nameLen = NBStructMapMember_strLenBytes(name);
		obj->name = NBMemory_allocTypes(char, nameLen + 1);
		NBMemory_copy(obj->name, name, nameLen + 1);
		NBASSERT(obj->name[nameLen] == '\0')
	}
}

void NBStructMapMember_release(STNBStructMapMember* obj){
	if(obj->name != NULL){
		NBMemory_free(obj->name);
		obj->name = NULL;
	}
	if(obj->elem.vCount != NULL){
		NBStructMapMember_release(obj->elem.vCount);
		NBMemory_free(obj->elem.vCount);
		obj->elem.vCount = NULL;
	}
	NBMemory_set(obj, 0, sizeof(*obj));
}

UI32 NBStructMapMember_strLenBytes(const char* str){
	const char* c1 = str;
	while(*c1 != '\0') c1++;
	return (UI32)(c1 - str);
}

BOOL NBStructMapMember_strIsEqual(const char* c1, const char* c2){
	do {
		if(*c1 != *c2) return FALSE;
		if(*c1 == '\0') break;
		c1++; c2++;
	} while(TRUE);
	return TRUE;
}
