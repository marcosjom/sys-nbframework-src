//
//  test_plistOld.c
//  nbframework-test-osx
//
//  Created by Marcos Ortega on 19/12/21.
//

#include "test_plistOld.h"

/*
const STNBPlistOldNode* getObjectNameAndType(STNBPlistOld* obj, const STNBPlistOldNode* objectsNode, const char* objId, const char* objIsa){
	const STNBPlistOldNode* r = NULL;
	if(obj != NULL && objectsNode != NULL && objId != NULL){
		const STNBPlistOldNode* nodeObj = NBPlistOld_childNode(obj, objId, objectsNode, NULL);
		if(nodeObj != NULL){
			r = nodeObj;
			if(objIsa != NULL){
				const char* isa = NBPlistOld_childStr(obj, "isa", NULL, nodeObj, NULL);
				if(!NBString_strIsEqual(isa, objIsa)){
					PRINTF_ERROR("Plist (old format), object('%s') expected to be a '%s' but found '%s'.\n", objId, objIsa, isa);
					NBASSERT(FALSE)
					r = NULL;
				}
			}
		}
	}
	return r;
}
*/

//Testing PlistOld
/*{
	STNBPlistOld plist;
	NBPlistOld_init(&plist);
	if(!NBPlistOld_loadFromFilePath(&plist, "/Users/mortegam/Desktop/NIBSA_proyectos/CltNicaraguaBinary/sys-nbframework/lib-nbframework-src/projects/xcode/lib-nbframework/lib-nbframework.xcodeproj/project.pbxproj")){
		PRINTF_ERROR("Could not load plist (old format).\n");
	} else {
		PRINTF_INFO("Plist (old format) loaded.\n");
		//
		const STNBPlistOldNode* rootNode = NBPlistOld_childNodeAfter(&plist, NBPlistOld_docNode(&plist), NULL);
		if(rootNode == NULL){
			PRINTF_ERROR("Plist (old format), no root node found.\n");
			NBASSERT(FALSE)
		} else {
			const STNBPlistOldNode* objects = NBPlistOld_childNode(&plist, "objects", rootNode, NULL);
			const char* rootObjectId = NBPlistOld_childStr(&plist, "rootObject", NULL, rootNode, NULL);
			if(objects == NULL){
				PRINTF_ERROR("Plist (old format), no 'objects' node found.\n");
				NBASSERT(FALSE)
			} else if(rootObjectId == NULL){
				PRINTF_ERROR("Plist (old format), no 'rootObject' node found.\n");
				NBASSERT(FALSE)
			} else {
				const STNBPlistOldNode* rootObject = getObjectNameAndType(&plist, objects, rootObjectId, "PBXProject");
				if(rootObject == NULL){
					PRINTF_ERROR("Plist (old format), rootObject('%s') node not found.\n", rootObjectId);
					NBASSERT(FALSE)
				} else {
					const char* projectDirPath = NBPlistOld_childStr(&plist, "projectDirPath", "", rootObject, NULL);
					const char* projectRoot = NBPlistOld_childStr(&plist, "projectRoot", "", rootObject, NULL);
					const STNBPlistOldNode* targets = NBPlistOld_childNode(&plist, "targets", rootObject, NULL);
					if(targets != NULL){
						const STNBPlistOldNode* target = NBPlistOld_childNode(&plist, "targets", targets, NULL);
						while(target != NULL){
							const char* targetId = NBPlistOld_nodeValue(&plist, target);
							PRINTF_INFO("Target: '%s'.\n", targetId);
							const STNBPlistOldNode* targetObj = getObjectNameAndType(&plist, objects, targetId, "PBXNativeTarget");
							if(targetObj == NULL){
								PRINTF_ERROR("Plist (old format), target('%s') node not found.\n", targetId);
								NBASSERT(FALSE)
							} else {
								const char* name = NBPlistOld_childStr(&plist, "name", "", targetObj, NULL);
								//const char* productName = NBPlistOld_childStr(&plist, "productName", "", targetObj, NULL);
								//const char* productReference = NBPlistOld_childStr(&plist, "productReference", "", targetObj, NULL);
								const char* productType = NBPlistOld_childStr(&plist, "productType", "", targetObj, NULL);
								PRINTF_INFO("Target name: '%s' ('%s').\n", name, productType);
								const STNBPlistOldNode* buildPhases = NBPlistOld_childNode(&plist, "buildPhases", targetObj, NULL);
								if(buildPhases != NULL){
									const STNBPlistOldNode* buildPhase = NBPlistOld_childNode(&plist, "buildPhases", buildPhases, NULL);
									while (buildPhase != NULL) {
										const char* buildPhaseId = NBPlistOld_nodeValue(&plist, buildPhase);
										PRINTF_INFO("BuildPhase: '%s'.\n", buildPhaseId);
										//PBXAppleScriptBuildPhase
										//PBXCopyFilesBuildPhase
										//PBXFrameworksBuildPhase
										//PBXHeadersBuildPhase
										//PBXResourcesBuildPhase
										//PBXShellScriptBuildPhase
										//PBXSourcesBuildPhase
										const STNBPlistOldNode* buildPhaseObj = NBPlistOld_childNode(&plist, buildPhaseId, objects, NULL);
										if(buildPhaseObj == NULL){
											PRINTF_ERROR("Plist (old format), buildPhase('%s') node not found.\n", buildPhaseId);
											NBASSERT(FALSE)
										} else {
											const char* isa = NBPlistOld_childStr(&plist, "isa", "", buildPhaseObj, NULL);
											if(NBString_strIsEqual(isa, "PBXSourcesBuildPhase")){
												const STNBPlistOldNode* files = NBPlistOld_childNode(&plist, "files", buildPhaseObj, NULL);
												if(files != NULL){
													const STNBPlistOldNode* file = NBPlistOld_childNode(&plist, "files", files, NULL);
													while(file != NULL){
														const char* buildFileId = NBPlistOld_nodeValue(&plist, file);
														const STNBPlistOldNode* buildFileObj = getObjectNameAndType(&plist, objects, buildFileId, "PBXBuildFile");
														if(buildFileObj == NULL){
															PRINTF_ERROR("Plist (old format), buildFile('%s') node not found.\n", buildFileId);
															NBASSERT(FALSE)
														} else {
															const char* fileRefId = NBPlistOld_childStr(&plist, "fileRef", NULL, buildFileObj, NULL);
															if(fileRefId == NULL){
																PRINTF_ERROR("Plist (old format), 'fileRef' ast buildFile('%s') not found.\n", buildFileId);
																NBASSERT(FALSE)
															} else {
																const STNBPlistOldNode* fileRefObj = getObjectNameAndType(&plist, objects, fileRefId, "PBXFileReference");
																if(fileRefObj == NULL){
																	PRINTF_ERROR("Plist (old format), fileRef('%s') node not found.\n", fileRefId);
																	NBASSERT(FALSE)
																} else {
																	const char* lastKnownFileType = NBPlistOld_childStr(&plist, "lastKnownFileType", NULL, fileRefObj, NULL);
																	const char* name = NBPlistOld_childStr(&plist, "name", NULL, fileRefObj, NULL);
																	const char* path = NBPlistOld_childStr(&plist, "path", NULL, fileRefObj, NULL);
																	const char* sourceTree = NBPlistOld_childStr(&plist, "sourceTree", NULL, fileRefObj, NULL);
																	//----------------------
																	//sourceTree:
																	//'<group>'				: '../../path/file.ext' (relative to group)
																	//'<absolute>'			: '/path/file.ext' (absolute path)
																	//'SOURCE_ROOT'			: '../../path/file.ext' (relativo to)
																	//'DEVELOPER_DIR'		: '../../path/file.ext' (relative to)
																	//'BUILT_PRODUCTS_DIR'	: '../../path/file.ext' (relative to)
																	//'SDKROOT'				: '../../path/file.ext' (relative to)
																	//----------------------
																	//'SOURCE_ROOT', 'DEVELOPER_DIR', 'BUILT_PRODUCTS_DIR' and 'SDKROOT'
																	//should be exported by the calling script, example:
																	//export SOURCE_ROOT=/Users/mortegam/Desktop/NIBSA_proyectos/CltNicaraguaBinary/sys-nbframework/lib-nbframework-src/projects/xcode/lib-nbframework
																	//export DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer
																	//export BUILT_PRODUCTS_DIR=/Users/mortegam/Library/Developer/Xcode/DerivedData/sis-refraneronica-ddkhvvldocqsmjcfsmxwiqrsvssg/Build/Products/Debug
																	//export SDKROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.13.sdk
																	//----------------------
																	PRINTF_INFO("FileRef: '%s' type('%s') path('%s') srcTree('%s').\n", name, lastKnownFileType, path, sourceTree);
																}
															}
														}
														//Next
														file = NBPlistOld_childNode(&plist, "files", files, file);
													}
												}
											}
										}
										//Next
										buildPhase = NBPlistOld_childNode(&plist, "buildPhases", buildPhases, buildPhase);
									}
								}
							}
							//Next
							target = NBPlistOld_childNode(&plist, "targets", targets, target);
						}
					}
				}
			}
		}
	}
	NBPlistOld_release(&plist);
}*/
