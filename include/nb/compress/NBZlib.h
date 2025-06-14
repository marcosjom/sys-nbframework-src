//
//  NBJsonParser.h
//  nbframework
//
//  Created by Marcos Ortega on 19/3/19.
//

#ifndef NBZlib_h
#define NBZlib_h

#include "nb/NBFrameworkDefs.h"
#include "nb/files/NBFile.h"
#include "nb/files/NBFilesystem.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	//Read
	BOOL NBZlib_inflateToStr(const void* data, const UI32 dataSz, STNBString* dst);
	BOOL NBZlib_inflateFromFile(STNBFileRef file, STNBString* dst);
	BOOL NBZlib_inflateFromPath(const char* filepath, STNBString* dst);
	BOOL NBZlib_inflateFromPathAtRoot(STNBFilesystem* fs, const ENNBFilesystemRoot root, const char* filepath, STNBString* dst);
	
	//Write
	BOOL NBZlib_deflateToStr(STNBString* dst, const void* data, const UI32 dataSz, const SI8 compLv9);
	BOOL NBZlib_deflateToFile(STNBFileRef file, const void* data, const UI32 dataSz, const SI8 compLv9);
	BOOL NBZlib_deflateToPath(const char* filePath, const void* data, const UI32 dataSz, const SI8 compLv9);
	BOOL NBZlib_deflateToPathAtRoot(STNBFilesystem* fs, const ENNBFilesystemRoot root, const char* filePath, const void* data, const UI32 dataSz, const SI8 compLv9);
	
#ifdef __cplusplus
}	//extern "C"
#endif

#endif /* NBZlib_h */
