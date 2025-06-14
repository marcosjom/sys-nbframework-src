//
//  NBPng.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/11/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/2d/NBPngChunk.h"
//
#include "nb/core/NBMemory.h"

void NBPngChunk_init(STNBPngChunk* obj){
	NBMemory_setZeroSt(*obj, STNBPngChunk);
}

void NBPngChunk_release(STNBPngChunk* obj){
	if(obj->data != NULL){
		NBMemory_free(obj->data);
		obj->data = NULL;
	}
}
