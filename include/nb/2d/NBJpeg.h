//
//  NBJpeg.h
//  nbframework
//
//  Created by Marcos Ortega on 18/3/19.
//

#ifndef NBJpeg_h
#define NBJpeg_h

#include "nb/NBFrameworkDefs.h"
#include "nb/files/NBFile.h"
#include "nb/files/NBFilesystem.h"
#include "nb/2d/NBBitmap.h"

#ifdef __cplusplus
extern "C" {
#endif

	//Read
	BOOL NBJpeg_loadFromFile(STNBFileRef file, const BOOL cargarDatos, STNBBitmap* dstBitmap);
	BOOL NBJpeg_loadFromPath(const char* filepath, const BOOL cargarDatos, STNBBitmap* dstBitmap);
	BOOL NBJpeg_loadFromPathAtRoot(STNBFilesystem* fs, const ENNBFilesystemRoot root, const char* filepath, const BOOL cargarDatos, STNBBitmap* dstBitmap);
	
	BOOL NBJpeg_dataStartsAsJpeg(const void* data, const UI32 dataSz);
	
	//Write
	BOOL NBJpeg_saveToFile(const STNBBitmap* bitmap, STNBFileRef file, const UI8 quality100, const UI8 smooth100);
	BOOL NBJpeg_saveToPath(const STNBBitmap* bitmap, const char* filePath, const UI8 quality100, const UI8 smooth100);
	BOOL NBJpeg_saveToPathAtRoot(const STNBBitmap* bitmap, STNBFilesystem* fs, const ENNBFilesystemRoot root, const char* filePath, const UI8 quality100, const UI8 smooth100);
	
#ifdef __cplusplus
}	//extern "C"
#endif


#endif /* NBJpeg_h */
