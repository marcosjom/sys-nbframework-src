//
//  NBJpeg.c
//  nbframework
//
//  Created by Marcos Ortega on 18/3/19.
//

#include "nb/NBFrameworkPch.h"
#include "nb/2d/NBJpeg.h"
//
#include "nb/core/NBMemory.h"
#include "nb/2d/NBJpegRead.h"
#include "nb/2d/NBJpegWrite.h"

//Read

BOOL NBJpeg_loadFromFile(STNBFileRef file, const BOOL cargarDatos, STNBBitmap* dstBitmap){
	BOOL r = FALSE;
	if(NBFile_isSet(file)){
		STNBJpegRead jpeg;
		NBJpegRead_init(&jpeg);
		if(NBJpegRead_feedStart(&jpeg, file)){
			ENJpegReadResult rr = ENJpegReadResult_partial;
			//Read header
			while((rr = NBJpegRead_feedRead(&jpeg, NULL, 0, NULL)) == ENJpegReadResult_partial){
				//
			}
			//Read data
			if(rr == ENJpegReadResult_end){
				const STNBBitmapProps p = NBJpegRead_feedGetProps(&jpeg);
				NBASSERT(p.size.width > 0 && p.size.height > 0 && p.bitsPerPx > 0 && p.bytesPerLine > 0)
				if(!cargarDatos){
					if(dstBitmap != NULL){
						NBBitmap_createWithoutData(dstBitmap, p.size.width, p.size.height, p.color);
					}
					r = TRUE;
				} else if(dstBitmap != NULL){
					NBBitmap_create(dstBitmap, p.size.width, p.size.height, p.color);
					UI32 linesReadd = 0;
					BYTE* data = NBBitmap_getData(dstBitmap);
					const STNBBitmapProps p = NBBitmap_getProps(dstBitmap);
					while((rr = NBJpegRead_feedRead(&jpeg, data, (p.bytesPerLine * p.size.height), &linesReadd)) == ENJpegReadResult_partial){
						//
					}
					if(rr == ENJpegReadResult_end){
						NBASSERT(linesReadd == p.size.height)
						r = TRUE;
					}
				}
			}
		}
		NBJpegRead_release(&jpeg);
	}
	return r;
}

BOOL NBJpeg_loadFromPath(const char* filepath, const BOOL cargarDatos, STNBBitmap* dstBitmap){
	BOOL r = FALSE;
	STNBFileRef file = NBFile_alloc(NULL);
	if(NBFile_open(file, filepath, ENNBFileMode_Read)){
		NBFile_lock(file);
		r = NBJpeg_loadFromFile(file, cargarDatos, dstBitmap);
		NBFile_unlock(file);
		NBFile_close(file);
	}
	NBFile_release(&file);
	return r;
}

BOOL NBJpeg_loadFromPathAtRoot(STNBFilesystem* fs, const ENNBFilesystemRoot root, const char* filepath, const BOOL cargarDatos, STNBBitmap* dstBitmap){
	BOOL r = FALSE;
	STNBFileRef file = NBFile_alloc(NULL);
	if(NBFilesystem_openAtRoot(fs, root, filepath, ENNBFileMode_Read, file)){
		NBFile_lock(file);
		r = NBJpeg_loadFromFile(file, cargarDatos, dstBitmap);
		NBFile_unlock(file);
		NBFile_close(file);
	}
	NBFile_release(&file);
	return r;
}

//

BOOL NBJpeg_dataStartsAsJpeg(const void* data, const UI32 dataSz){
	BOOL r = FALSE;
	if(data != NULL && dataSz >= 3){
		const BYTE* head = (const BYTE*)data;
		if(head[0] == 0xFF && head[1] == 0xD8 && head[2] == 0xFF){
			r = TRUE;
		}
	}
	return r;
}

//Write

BOOL NBJpeg_saveToFile(const STNBBitmap* bitmap, STNBFileRef file, const UI8 quality100, const UI8 smooth100){
	BOOL r = FALSE;
	if(bitmap != NULL && NBFile_isSet(file)){
		const BYTE* data = NBBitmap_getData(bitmap);
		const STNBBitmapProps props = NBBitmap_getProps(bitmap);
		if(data != NULL && props.bytesPerLine > 0 && props.size.width > 0 && props.size.height > 0){
			STNBJpegWrite jpeg;
			NBJpegWrite_init(&jpeg);
			if(NBJpegWrite_feedStart(&jpeg, data, (props.bytesPerLine * props.size.height), props, file, quality100, smooth100)){
				SI32 ciclesCount = 0;
				ENJpegWriteResult rr = ENJpegWriteResult_error;
				while((rr = NBJpegWrite_feedWrite(&jpeg)) == ENJpegWriteResult_partial){
					ciclesCount++;
				}
				if(rr == ENJpegWriteResult_end){
					r = TRUE;
				}
				NBJpegWrite_feedEnd(&jpeg);
			}
			NBJpegWrite_release(&jpeg);
		}
	}
	return r;
}

BOOL NBJpeg_saveToPath(const STNBBitmap* bitmap, const char* filePath, const UI8 quality100, const UI8 smooth100){
	BOOL r = FALSE;
	STNBFileRef file = NBFile_alloc(NULL);
	if(NBFile_open(file, filePath, ENNBFileMode_Write)){
		NBFile_lock(file);
		{
			r = NBJpeg_saveToFile(bitmap, file, quality100, smooth100);
		}
		NBFile_unlock(file);
		NBFile_close(file);
	}
	NBFile_release(&file);
	return r;
}

BOOL NBJpeg_saveToPathAtRoot(const STNBBitmap* bitmap, STNBFilesystem* fs, const ENNBFilesystemRoot root, const char* filePath, const UI8 quality100, const UI8 smooth100){
	BOOL r = FALSE;
	STNBFileRef file = NBFile_alloc(NULL);
	if(NBFilesystem_openAtRoot(fs, root, filePath, ENNBFileMode_Write, file)){
		NBFile_lock(file);
		{
			r = NBJpeg_saveToFile(bitmap, file, quality100, smooth100);
		}
		NBFile_unlock(file);
		NBFile_close(file);
	}
	NBFile_release(&file);
	return r;
}
