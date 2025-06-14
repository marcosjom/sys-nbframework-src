//
//  NBFilesystemApple.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/25/18.
//

#ifndef NBFilesystemApple_h
#define NBFilesystemApple_h

#include "nb/NBFrameworkDefs.h"
#include "nb/files/NBFilesystem.h"

#ifdef __cplusplus
extern "C" {
#endif
	
typedef struct STNBFilesystemApple_ {
	void* opaque;
} STNBFilesystemApple;

void NBFilesystemApple_init(STNBFilesystemApple* obj);
void NBFilesystemApple_retain(STNBFilesystemApple* obj);
void NBFilesystemApple_release(STNBFilesystemApple* obj);

//
void NBFilesystemApple_createItf(void* obj, IFilesystemItf* dst);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBFilesystemApple_h */
