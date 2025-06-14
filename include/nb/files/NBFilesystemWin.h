//
//  NBFilesystemWin.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/25/18.
//

#ifndef NBFilesystemWin_h
#define NBFilesystemWin_h

#include "nb/NBFrameworkDefs.h"
#include "nb/files/NBFilesystem.h"

#ifdef __cplusplus
extern "C" {
#endif
	
typedef struct STNBFilesystemWin_ {
	void* opaque;
} STNBFilesystemWin;

void NBFilesystemWin_init(STNBFilesystemWin* obj);
void NBFilesystemWin_retain(STNBFilesystemWin* obj);
void NBFilesystemWin_release(STNBFilesystemWin* obj);

//
void NBFilesystemWin_createItf(void* obj, IFilesystemItf* dst);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBFilesystemWin_h */
