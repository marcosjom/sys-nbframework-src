//
//  NBFilesystemAndroid.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/25/18.
//

#ifndef NBFilesystemAndroid_h
#define NBFilesystemAndroid_h

#include "nb/NBFrameworkDefs.h"
#include "nb/files/NBFilesystem.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct STNBAndroidJniItf_ {
	SI32				(*jniVersion)(void* obj);
	void* /*jobject*/	(*jActivity)(void* obj);
	void* /*JNIEnv*/	(*curEnv)(void* obj);
} STNBAndroidJniItf;
	
typedef struct STNBFilesystemAndroid_ {
	void* opaque;
} STNBFilesystemAndroid;

void NBFilesystemAndroid_init(STNBFilesystemAndroid* obj, const STNBAndroidJniItf* jniItf, void* jniObj);
void NBFilesystemAndroid_retain(STNBFilesystemAndroid* obj);
void NBFilesystemAndroid_release(STNBFilesystemAndroid* obj);

//
void NBFilesystemAndroid_createItf(void* obj, IFilesystemItf* dst);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBFilesystemAndroid_h */
