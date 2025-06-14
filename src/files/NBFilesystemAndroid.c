//
//  NBFilesystemAndroid.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/25/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/files/NBFilesystemAndroid.h"
//
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
//
#include <stdio.h>						//for SEEK_SET, SEEK_CUR, SEEK_END
//Android and JNI headers
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h> //for AAssetManager_fromJava

typedef struct STNBFilesystemAndroidOpq_ {
	//JNI
	STNBAndroidJniItf	jniItf;
	void*				jniObj;
	AAssetManager*		assetsMngr;
	//Values
	STNBString			roots[ENNBFilesystemRoot_Count];
	UI32				retainCount;
} STNBFilesystemAndroidOpq;

//Filesystem itf
void NBFilesystemAndroid_retain_(void* obj);
void NBFilesystemAndroid_release_(void** obj);
//
BOOL NBFilesystemAndroid_concatRootPath_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* fileName, STNBString* dst);
//
BOOL NBFilesystemAndroid_getFolders_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
BOOL NBFilesystemAndroid_folderExists_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath);
BOOL NBFilesystemAndroid_createFolder_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath);
BOOL NBFilesystemAndroid_deleteFolder_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath);
//
BOOL NBFilesystemAndroid_open_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* filepath, const ENNBFileMode mode, STNBFileRef dst);
BOOL NBFilesystemAndroid_getFiles_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath, const BOOL includeStats, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/);
BOOL NBFilesystemAndroid_moveFile_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot srcRoot, const char* srcFilepath, const ENNBFilesystemRoot dstRoot, const char* dstFilepath);
BOOL NBFilesystemAndroid_deleteFile_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* filepath);

//
void NBFileApk_close_(void** obj);
//void NBFileApk_lock_(void* obj);
//void NBFileApk_unlock_(void* obj);
BOOL NBFileApk_isOpen_(const void* obj);
UI32 NBFileApk_read_(void* obj, void* dst, const UI32 blockSize, const UI32 blocks, const SI64* curPos);
UI32 NBFileApk_write_(void* obj, const void* src, const UI32 blockSize, const UI32 blocks, const SI64* curPos);
BOOL NBFileApk_seek_(void* obj, const SI64 pos, const ENNBFileRelative relativeTo, const SI64* curPos);
SI64 NBFileApk_curPos_(const void* obj);
BOOL NBFileApk_flush_(void* obj);
//

void NBFilesystemAndroid_init(STNBFilesystemAndroid* obj, const STNBAndroidJniItf* jniItf, void* jniObj){
	STNBFilesystemAndroidOpq* opq = obj->opaque = NBMemory_allocType(STNBFilesystemAndroidOpq);
	NBMemory_setZeroSt(*opq, STNBFilesystemAndroidOpq);
	//JniItf
	{
		if(jniItf == NULL){
			NBMemory_setZeroSt(opq->jniItf, STNBAndroidJniItf);
		} else {
			opq->jniItf = *jniItf;
		}
		opq->jniObj	= jniObj;
		opq->assetsMngr	= NULL;
	}
	{
		if(opq->jniItf.curEnv == NULL || opq->jniItf.jActivity == NULL){
			PRINTF_ERROR("NBFilesystemAndroid_init fail, jniItf.curEnv or jniItf.jActivity are/is NULL.\n");
		} else {
			JNIEnv* jEnv = (JNIEnv*)(*opq->jniItf.curEnv)(opq->jniObj); NBASSERT(jEnv != NULL)
			jobject jContext = (jobject)(*opq->jniItf.jActivity)(opq->jniObj); NBASSERT(jContext != NULL)
			if(jEnv == NULL || jContext == NULL){
				PRINTF_ERROR("NBFilesystemAndroid_init fail, jEnv or jContext are/is NULL.\n");
			} else {
				jclass clsContext = (*jEnv)->FindClass(jEnv, "android/content/Context");
				if(clsContext == NULL){
					PRINTF_ERROR("NBFilesystemAndroid_init fail, clsContext is NULL.\n");
				} else {
					//Get Context's jni assets object
					{
						jmethodID mid = (*jEnv)->GetMethodID(jEnv, clsContext, "getResources", "()Landroid/content/res/Resources;");
						if (mid != NULL) {
							jobject jResObj = (*jEnv)->CallObjectMethod(jEnv, jContext, mid);
							if (jResObj != NULL) {
								jclass clsRes = (*jEnv)->FindClass(jEnv, "android/content/res/Resources");
								if(clsRes != NULL){
									jmethodID mid = (*jEnv)->GetMethodID(jEnv, clsRes, "getAssets", "()Landroid/content/res/AssetManager;");
									if (mid != NULL) {
										jobject jAssMngrObj = (*jEnv)->CallObjectMethod(jEnv, jResObj, mid);
										if (jAssMngrObj != NULL) {
											opq->assetsMngr = AAssetManager_fromJava(jEnv, jAssMngrObj);
											if(opq->assetsMngr == NULL){
												PRINTF_ERROR("NBFilesystemAndroid_init fail, AAssetManager_fromJava returned NULL.\n");
											}
										}
									}
									(*jEnv)->DeleteLocalRef(jEnv, clsRes);
								}
								(*jEnv)->DeleteLocalRef(jEnv, jResObj);
							}
						}
					}
					UI32 i; for(i = 0; i < ENNBFilesystemRoot_Count; i++){
						STNBString* str = &opq->roots[i];
						//Load value
						switch(i) {
							case ENNBFilesystemRoot_AppBundle:
								PRINTF_INFO("appBundle: '%s'.\n", "apk:/");
								NBString_initWithStr(str, "apk:/");
								break;
							case ENNBFilesystemRoot_Docs:
							case ENNBFilesystemRoot_Lib:
								//Get Context's docs path
								{
									NBString_init(str);
									jmethodID mid = (*jEnv)->GetMethodID(jEnv, clsContext, "getFilesDir", "()Ljava/io/File;");
									if (mid != NULL) {
										jobject jDirObj = (*jEnv)->CallObjectMethod(jEnv, jContext, mid);
										if(jDirObj != NULL){
											jclass clsFile = (*jEnv)->FindClass(jEnv, "java/io/File");
											if(clsFile != NULL){
												jmethodID mid = (*jEnv)->GetMethodID(jEnv, clsFile, "getAbsolutePath", "()Ljava/lang/String;");
												if (mid != NULL) {
													jstring jDocPath = (jstring)(*jEnv)->CallObjectMethod(jEnv, jDirObj, mid);
													const char* utfStr = (*jEnv)->GetStringUTFChars(jEnv, jDocPath, 0);
													PRINTF_INFO("getFilesDir: '%s'.\n", utfStr);
													NBString_set(str, utfStr);
													(*jEnv)->ReleaseStringUTFChars(jEnv, jDocPath, utfStr);
													(*jEnv)->DeleteLocalRef(jEnv, jDocPath);
												}
												(*jEnv)->DeleteLocalRef(jEnv, clsFile);
											}
											(*jEnv)->DeleteLocalRef(jEnv, jDirObj);
										}
									}
								}
								break;
							case ENNBFilesystemRoot_Cache:
								//Get Context's cache path
								{
									NBString_init(str);
									jmethodID mid = (*jEnv)->GetMethodID(jEnv, clsContext, "getCacheDir", "()Ljava/io/File;");
									if (mid != NULL) {
										jobject jDirObj = (*jEnv)->CallObjectMethod(jEnv, jContext, mid);
										if(jDirObj != NULL){
											jclass clsFile = (*jEnv)->FindClass(jEnv, "java/io/File");
											if(clsFile != NULL){
												jmethodID mid = (*jEnv)->GetMethodID(jEnv, clsFile, "getAbsolutePath", "()Ljava/lang/String;");
												if (mid != NULL) {
													jstring jCachePath = (jstring)(*jEnv)->CallObjectMethod(jEnv, jDirObj, mid);
													const char* utfStr = (*jEnv)->GetStringUTFChars(jEnv, jCachePath, 0);
													PRINTF_INFO("getCacheDir: '%s'.\n", utfStr);
													NBString_set(str, utfStr);
													(*jEnv)->ReleaseStringUTFChars(jEnv, jCachePath, utfStr);
													(*jEnv)->DeleteLocalRef(jEnv, jCachePath);
												}
												(*jEnv)->DeleteLocalRef(jEnv, clsFile);
											}
											(*jEnv)->DeleteLocalRef(jEnv, jDirObj);
										}
									}
								}
								break;
							default:
								NBString_init(str);
								break;
						}
						//Set value
						if(str->length > 0){
							if(str->str[str->length - 1] != '/'){
								NBString_concatByte(str, '/');
							}
						}
					}
					//Release
					(*jEnv)->DeleteLocalRef(jEnv, clsContext);
				}
			}
		}
	}
	opq->retainCount = 1;
}

void NBFilesystemAndroid_retain(STNBFilesystemAndroid* obj){
	NBFilesystemAndroid_retain_(obj->opaque);
}

void NBFilesystemAndroid_release(STNBFilesystemAndroid* obj){
	NBFilesystemAndroid_release_(&obj->opaque);
}

//

void NBFilesystemAndroid_createItf(void* pObj, IFilesystemItf* dst){
	STNBFilesystemAndroid* obj = (STNBFilesystemAndroid*)pObj;
	STNBFilesystemAndroidOpq* opq = (STNBFilesystemAndroidOpq*)obj->opaque;
	//Set iterface
	NBMemory_set(dst, 0, sizeof(*dst));
	//
	dst->obj			= opq;
	//
	dst->retain			= NBFilesystemAndroid_retain_;
	dst->release		= NBFilesystemAndroid_release_;
	//
	dst->concatRootPath	= NBFilesystemAndroid_concatRootPath_;
	//
	dst->getFolders		= NBFilesystemAndroid_getFolders_;
	dst->folderExists	= NBFilesystemAndroid_folderExists_;
	dst->createFolder	= NBFilesystemAndroid_createFolder_;
	dst->deleteFolder	= NBFilesystemAndroid_deleteFolder_;
	//
	dst->open			= NBFilesystemAndroid_open_;
	dst->getFiles		= NBFilesystemAndroid_getFiles_;
	dst->moveFile		= NBFilesystemAndroid_moveFile_;
	dst->deleteFile		= NBFilesystemAndroid_deleteFile_;
}

//Filesystem itf

void NBFilesystemAndroid_retain_(void* obj){
	STNBFilesystemAndroidOpq* opq = (STNBFilesystemAndroidOpq*)obj;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount++;
}

void NBFilesystemAndroid_release_(void** obj){
	STNBFilesystemAndroidOpq* opq = (STNBFilesystemAndroidOpq*)*obj;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount--;
	if(opq->retainCount == 0){
		//Paths
		{
			UI32 i; for(i = 0; i < ENNBFilesystemRoot_Count; i++){
				STNBString* str = &opq->roots[i];
				NBString_release(str);
			}
		}
		//Memory
		NBMemory_free(*obj);
		*obj = NULL;
	}
}

//

BOOL NBFilesystemAndroid_concatRootPath_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* fileName, STNBString* dst){
	BOOL r = FALSE;
	STNBFilesystemAndroidOpq* opq = (STNBFilesystemAndroidOpq*)obj;
	if(root >= 0 && root < ENNBFilesystemRoot_Count){
		if(dst != NULL){
			BOOL removeSlashes = FALSE;
			const char* rootStr = opq->roots[root].str;
			if(*rootStr != '\0'){
				NBString_concat(dst, rootStr);
				removeSlashes = TRUE;
			}
			if(fileName != NULL){
				if(removeSlashes){
					while(*fileName == '/' || *fileName == '\\'){
						fileName++;
					}
				}
				NBString_concat(dst, fileName);
			}
		}
		r = TRUE;
	}
	return r;
}

//

BOOL NBFilesystemAndroid_getFolders_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/){
	BOOL r = FALSE;
	if(parents != NULL && parentsSz > 0){
		STNBFilesystemAndroidOpq* opq = (STNBFilesystemAndroidOpq*)obj;
		const IFilesystemItf* pItf = &parents[0];
		if(pItf->getFolders != NULL){
			const STNBString* strRoot = &opq->roots[root];
			if(strRoot->length > 0){
				STNBString tmpPath;
				NBString_initWithStringAndStr(&tmpPath, strRoot, folderpath);
				r = (*pItf->getFolders)(pItf->obj, &parents[1], (parentsSz - 1), ENNBFilesystemRoot_WorkDir, tmpPath.str, dstStrs, dstFiles);
				NBString_release(&tmpPath);
			} else {
				r = (*pItf->getFolders)(pItf->obj, &parents[1], (parentsSz - 1), root, folderpath, dstStrs, dstFiles);
			}
		}
	}
	return r;
}

BOOL NBFilesystemAndroid_folderExists_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath){
	BOOL r = FALSE;
	if(parents != NULL && parentsSz > 0){
		STNBFilesystemAndroidOpq* opq = (STNBFilesystemAndroidOpq*)obj;
		const IFilesystemItf* pItf = &parents[0];
		if(pItf->folderExists != NULL){
			const STNBString* strRoot = &opq->roots[root];
			if(strRoot->length > 0){
				STNBString tmpPath;
				NBString_initWithStringAndStr(&tmpPath, strRoot, folderpath);
				r = (*pItf->folderExists)(pItf->obj, &parents[1], (parentsSz - 1), ENNBFilesystemRoot_WorkDir, tmpPath.str);
				NBString_release(&tmpPath);
			} else {
				r = (*pItf->folderExists)(pItf->obj, &parents[1], (parentsSz - 1), root, folderpath);
			}
		}
	}
	return r;
}

BOOL NBFilesystemAndroid_createFolder_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath){
	BOOL r = FALSE;
	if(parents != NULL && parentsSz > 0){
		STNBFilesystemAndroidOpq* opq = (STNBFilesystemAndroidOpq*)obj;
		const IFilesystemItf* pItf = &parents[0];
		if(pItf->createFolder != NULL){
			const STNBString* strRoot = &opq->roots[root];
			if(strRoot->length > 0){
				STNBString tmpPath;
				NBString_initWithStringAndStr(&tmpPath, strRoot, folderpath);
				r = (*pItf->createFolder)(pItf->obj, &parents[1], (parentsSz - 1), ENNBFilesystemRoot_WorkDir, tmpPath.str);
				NBString_release(&tmpPath);
			} else {
				r = (*pItf->createFolder)(pItf->obj, &parents[1], (parentsSz - 1), root, folderpath);
			}
		}
	}
	return r;
}

BOOL NBFilesystemAndroid_deleteFolder_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath){
	BOOL r = FALSE;
	if(parents != NULL && parentsSz > 0){
		STNBFilesystemAndroidOpq* opq = (STNBFilesystemAndroidOpq*)obj;
		const IFilesystemItf* pItf = &parents[0];
		if(pItf->deleteFolder != NULL){
			const STNBString* strRoot = &opq->roots[root];
			if(strRoot->length > 0){
				STNBString tmpPath;
				NBString_initWithStringAndStr(&tmpPath, strRoot, folderpath);
				r = (*pItf->deleteFolder)(pItf->obj, &parents[1], (parentsSz - 1), ENNBFilesystemRoot_WorkDir, tmpPath.str);
				NBString_release(&tmpPath);
			} else {
				r = (*pItf->deleteFolder)(pItf->obj, &parents[1], (parentsSz - 1), root, folderpath);
			}
		}
	}
	return r;
}

//

BOOL NBFilesystemAndroid_open_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* filepath, const ENNBFileMode mode, STNBFileRef dst){
	BOOL r = FALSE;
	if(parents != NULL && parentsSz > 0){
		STNBFilesystemAndroidOpq* opq = (STNBFilesystemAndroidOpq*)obj;
		const IFilesystemItf* pItf = &parents[0];
		BOOL isApkFile = (root == ENNBFilesystemRoot_AppBundle);
		SI32 iStartPath = 0;
		//Detect path
		{
			if(filepath[0] != '\0'){
				if(filepath[1] != '\0'){
					if(filepath[2] != '\0'){
						if(filepath[3] != '\0'){
							if(filepath[4] != '\0'){
								if(filepath[0] == 'a' && filepath[1] == 'p' && filepath[2] == 'k' && filepath[3] == ':' && filepath[4] == '/'){
									isApkFile = TRUE;
									iStartPath = 5;
								}
							}
						}
					}
				}
			}
		}
		if(isApkFile){
			if(opq->assetsMngr != NULL){
				AAsset* asset = AAssetManager_open(opq->assetsMngr, &filepath[iStartPath], AASSET_MODE_RANDOM);
				if(asset == NULL){
					PRINTF_ERROR("AUAppGlueAndroidFiles, could not open android-asset '%s' (starting at char %d)\n", filepath, iStartPath);
				} else {
					IFileItf itf;
					NBMemory_setZeroSt(itf, IFileItf);
					itf.close		= NBFileApk_close_;
					//itf.lock		= NBFileApk_lock_;
					//itf.unlock	= NBFileApk_unlock_;
					itf.isOpen		= NBFileApk_isOpen_;
					itf.read		= NBFileApk_read_;
					itf.write		= NBFileApk_write_;
					itf.seek		= NBFileApk_seek_;
					itf.curPos		= NBFileApk_curPos_;
					itf.flush		= NBFileApk_flush_;
					NBFile_openAsItf(dst, &itf, asset);
					PRINTF_INFO("AUAppGlueAndroidFiles, AAsset from APK opened for '%s'.\n", filepath);
					r = TRUE;
				}
			}
		} else if(pItf->open != NULL){
			//Search with parent
			const STNBString* strRoot = &opq->roots[root];
			if(strRoot->length > 0){
				STNBString tmpPath;
				NBString_initWithStringAndStr(&tmpPath, strRoot, filepath);
				r = (*pItf->open)(pItf->obj, &parents[1], (parentsSz - 1), ENNBFilesystemRoot_WorkDir, tmpPath.str, mode, dst);
				NBString_release(&tmpPath);
			} else {
				r = (*pItf->open)(pItf->obj, &parents[1], (parentsSz - 1), root, filepath, mode, dst);
			}
		}
	}
	return r;
}

BOOL NBFilesystemAndroid_getFiles_(const void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* folderpath, const BOOL includeStats, STNBString* dstStrs, STNBArray* dstFiles /*STNBFilesystemFile*/){
	BOOL r = FALSE;
	if(parents != NULL && parentsSz > 0){
		STNBFilesystemAndroidOpq* opq = (STNBFilesystemAndroidOpq*)obj;
		const IFilesystemItf* pItf = &parents[0];
		if(pItf->getFiles != NULL){
			const STNBString* strRoot = &opq->roots[root];
			if(strRoot->length > 0){
				STNBString tmpPath;
				NBString_initWithStringAndStr(&tmpPath, strRoot, folderpath);
				r = (*pItf->getFiles)(pItf->obj, &parents[1], (parentsSz - 1), ENNBFilesystemRoot_WorkDir, tmpPath.str, includeStats, dstStrs, dstFiles);
				NBString_release(&tmpPath);
			} else {
				r = (*pItf->getFiles)(pItf->obj, &parents[1], (parentsSz - 1), root, folderpath, includeStats, dstStrs, dstFiles);
			}
		}
	}
	return r;
}

BOOL NBFilesystemAndroid_moveFile_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot srcRoot, const char* srcFilepath, const ENNBFilesystemRoot dstRoot, const char* dstFilepath){
	BOOL r = FALSE;
	if(parents != NULL && parentsSz > 0){
		STNBFilesystemAndroidOpq* opq = (STNBFilesystemAndroidOpq*)obj;
		const IFilesystemItf* pItf = &parents[0];
		if(pItf->moveFile != NULL){
			const STNBString* strRoot1 = &opq->roots[srcRoot];
			const STNBString* strRoot2 = &opq->roots[dstRoot];
			if(strRoot1->length > 0 || strRoot2->length > 0){
				STNBString tmpPath, tmpPath2;
				NBString_initWithStringAndStr(&tmpPath, strRoot1, srcFilepath);
				NBString_initWithStringAndStr(&tmpPath2, strRoot2, dstFilepath);
				r = (*pItf->moveFile)(pItf->obj, &parents[1], (parentsSz - 1), (strRoot1->length > 0 ? ENNBFilesystemRoot_WorkDir : srcRoot), tmpPath.str, (strRoot2->length > 0 ? ENNBFilesystemRoot_WorkDir : dstRoot), tmpPath2.str);
				NBString_release(&tmpPath);
				NBString_release(&tmpPath2);
			} else {
				r = (*pItf->moveFile)(pItf->obj, &parents[1], (parentsSz - 1), srcRoot, srcFilepath, dstRoot, dstFilepath);
			}
		}
	}
	return r;
}

BOOL NBFilesystemAndroid_deleteFile_(void* obj, const struct IFilesystemItf_* parents, const UI32 parentsSz, const ENNBFilesystemRoot root, const char* filepath){
	BOOL r = FALSE;
	if(parents != NULL && parentsSz > 0){
		STNBFilesystemAndroidOpq* opq = (STNBFilesystemAndroidOpq*)obj;
		const IFilesystemItf* pItf = &parents[0];
		if(pItf->deleteFile != NULL){
			const STNBString* strRoot = &opq->roots[root];
			if(strRoot->length > 0){
				STNBString tmpPath;
				NBString_initWithStringAndStr(&tmpPath, strRoot, filepath);
				r = (*pItf->deleteFile)(pItf->obj, &parents[1], (parentsSz - 1), ENNBFilesystemRoot_WorkDir, tmpPath.str);
				NBString_release(&tmpPath);
			} else {
				r = (*pItf->deleteFile)(pItf->obj, &parents[1], (parentsSz - 1), root, filepath);
			}
		}
	}
	return r;
}

//APK file iterface

void NBFileApk_close_(void** obj){
	if(obj != NULL){
		AAsset* asset = (AAsset*)*obj;
		PRINTF_INFO("NBFileApk_close_.\n");
		if(asset != NULL){
			AAsset_close(asset);
			//Set NULL
			*obj = NULL;
		}
	}
}

/*void NBFileApk_lock_(void* obj){
	AAsset* asset = (AAsset*)obj;
}*/

/*void NBFileApk_unlock_(void* obj){
	AAsset* asset = (AAsset*)obj;
}*/

BOOL NBFileApk_isOpen_(const void* obj){
	AAsset* asset = (AAsset*)obj;
	PRINTF_INFO("NBFileApk_isOpen_.\n");
	return (asset != NULL);
}

UI32 NBFileApk_read_(void* obj, void* dst, const UI32 blockSize, const UI32 blocks, const SI64* curPos){
	UI32 r = 0;
	AAsset* asset = (AAsset*)obj;
	if(asset != NULL && blockSize > 0){
		r = AAsset_read(asset, dst, blockSize * blocks);
		NBASSERT(r >= 0);
		NBASSERT((r % blockSize) == 0);
		r /= blockSize;
	}
	//PRINTF_INFO("NBFileApk_read_ => r(%d).\n", r);
	return r;
}

UI32 NBFileApk_write_(void* obj, const void* src, const UI32 blockSize, const UI32 blocks, const SI64* curPos){
	NBASSERT(FALSE) //Not allowed
	PRINTF_INFO("NBFileApk_write_.\n");
	return 0;
}

BOOL NBFileApk_seek_(void* obj, const SI64 pos, const ENNBFileRelative relativeTo, const SI64* curPos){
	BOOL r = FALSE;
	NBASSERT(relativeTo == ENNBFileRelative_Start || relativeTo == ENNBFileRelative_End || relativeTo == ENNBFileRelative_CurPos)
	//PRINTF_INFO("NBFileApk_seek_ pos(%lld) relativeTo(%d) curPos(%lld).\n", pos, relativeTo, curPos);
	AAsset* asset = (AAsset*)obj;
	if(asset != NULL){
		switch (relativeTo) {
			case ENNBFileRelative_Start:
				{
					const SI32 rSeek = AAsset_seek(asset, pos, SEEK_SET);
					if(rSeek < 0){
						NBASSERT(FALSE)
					} else {
						NBASSERT(rSeek == pos)
						NBASSERT(rSeek == (AAsset_getLength(asset) - AAsset_getRemainingLength(asset)))
						r = TRUE;
					}
				}
				break;
			case ENNBFileRelative_CurPos:
				{
					const SI32 rSeek = AAsset_seek(asset, pos, SEEK_CUR);
					if(rSeek < 0){
						NBASSERT(FALSE)
					} else {
						NBASSERT(curPos != NULL || (rSeek == (*curPos + pos)))
						NBASSERT(rSeek == (AAsset_getLength(asset) - AAsset_getRemainingLength(asset)))
						r = TRUE;
					}
				}
				break;
			case ENNBFileRelative_End:
				{
					const SI32 rSeek = AAsset_seek(asset, pos, SEEK_END);
					if(rSeek < 0){
						NBASSERT(FALSE)
					} else {
						NBASSERT(rSeek == AAsset_getRemainingLength(asset))
						r = TRUE;
					}
				}
				break;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return r;
}

SI64 NBFileApk_curPos_(const void* obj){
	SI64 r = 0;
	AAsset* asset = (AAsset*)obj;
	if(asset != NULL){
		r = (AAsset_getLength(asset) - AAsset_getRemainingLength(asset));
	}
	PRINTF_INFO("NBFileApk_curPos_(%llu).\n", r);
	return r;
}

BOOL NBFileApk_flush_(void* obj){
	AAsset* asset = (AAsset*)obj;
	PRINTF_INFO("NBFileApk_flush_.\n");
	return TRUE;
}
