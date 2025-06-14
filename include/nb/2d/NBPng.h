//
//  NBPng.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/11/18.
//

#ifndef NBPng_h
#define NBPng_h

#include "nb/NBFrameworkDefs.h"
#include "nb/2d/NBBitmap.h"
#include "nb/2d/NBColor.h"
#include "nb/files/NBFilesystem.h"

#define NBPNG_BYTES_PER_PIXEL(COLORPNG)	(COLORPNG == ENNBPngColor_RGBA ? 4 : COLORPNG == ENNBPngColor_Gris ? 1 : COLORPNG == ENNBPngColor_RGB ? 3 : COLORPNG == ENNBPngColor_RGBConPaleta ? 1 : COLORPNG == ENNBPngColor_GrisAlpha ? 2 : 0)

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef enum ENNBPngColor_ {
		ENNBPngColor_Gris			= 0,    //explicit value for compatibility with old-framework
		ENNBPngColor_RGB			= 2,    //explicit value for compatibility with old-framework
		ENNBPngColor_RGBConPaleta	= 3,    //explicit value for compatibility with old-framework
		ENNBPngColor_GrisAlpha		= 4,    //explicit value for compatibility with old-framework
		ENNBPngColor_RGBA			= 6,    //explicit value for compatibility with old-framework
		ENNBPngColor_Error			= 255
	} ENNBPngColor;
	
	typedef enum ENPngCompressLvl_ {
		ENPngCompressLvl_0          = 0,
		ENPngCompressLvl_1,
		ENPngCompressLvl_2,
		ENPngCompressLvl_3,
		ENPngCompressLvl_4,
		ENPngCompressLvl_5,
		ENPngCompressLvl_6,
		ENPngCompressLvl_7,
		ENPngCompressLvl_8,
		ENPngCompressLvl_9,
		ENPngCompressLvl_Count,
	} ENPngCompressLvl;

	typedef struct STNBPngIHDR_ {
		UI32	width;
		UI32	height;
		UI8		bitsDepth;
		UI8		pngColor;
		UI8		compMethd;
		UI8		filterMethd;
		UI8		interlMethd;
	} STNBPngIHDR;

	typedef struct STNBPngIHDRCustom_ {
		BOOL	isCgBI;		//Apple incompatible PNG format
	} STNBPngIHDRCustom;

    // Analyze

    BOOL NBPng_calculatePalette(const STNBBitmap* bitmap, STNBColor8** dstColors, UI32* dstColorsSz, const UI32 colorsCountLimit);    //reads all pixels and returns an ordered list of unique pixels, returns error if colorsCountLimit is non-zero and more color are found
    BOOL NBPng_calculatePaletteOnData(const STNBBitmapProps props, const BYTE* pixels, STNBColor8** dstColors, UI32* dstColorsSz, const UI32 colorsCountLimit);
    BOOL NBPng_calculatePaletteOnDataTo(const STNBBitmapProps props, const BYTE* pixels, STNBArraySorted* dst /*STNBColor8*/, const UI32 colorsCountLimit);

	//Save
	
	typedef struct STNBPngSaveState_ {
		void* opaque;
	} STNBPngSaveState;
	
	void NBPngSaveState_init(STNBPngSaveState* obj, const UI32 compressBuffSz);
	void NBPngSaveState_release(STNBPngSaveState* obj);
	
	BOOL NBPng_dataStartsAsPng(const void* data, const UI32 dataSz);
	
	BOOL NBPng_saveToFile(const STNBBitmap* bitmap, STNBFileRef file, const ENPngCompressLvl compresssLvl);
    BOOL NBPng_saveToFileWithPalette(const STNBBitmap* bitmap, STNBFileRef file, const ENPngCompressLvl compresssLvl);
	BOOL NBPng_saveToPath(const STNBBitmap* bitmap, const char* filePath, const ENPngCompressLvl compresssLvl);
	BOOL NBPng_saveToPathAtRoot(const STNBBitmap* bitmap, STNBFilesystem* fs, const ENNBFilesystemRoot root, const char* filePath, const ENPngCompressLvl compresssLvl);
	//
	BOOL NBPng_saveDataToFile(const STNBBitmapProps* props, const void* pixels, STNBFileRef file, const ENPngCompressLvl compresssLvl);
	
	BOOL NBPng_saveToFileWithState(const STNBBitmap* bitmap, STNBFileRef file, const ENPngCompressLvl compresssLvl, STNBPngSaveState* state, const UI32 paletteMaxSz);
	BOOL NBPng_saveToPathWithState(const STNBBitmap* bitmap, const char* filePath, const ENPngCompressLvl compresssLvl, STNBPngSaveState* state);
	BOOL NBPng_saveToPathAtRootWithState(const STNBBitmap* bitmap, STNBFilesystem* fs, const ENNBFilesystemRoot root, const char* filePath, const ENPngCompressLvl compresssLvl, STNBPngSaveState* state);
	//
	BOOL NBPng_saveDataToFileWithState(const STNBBitmapProps* props, const void* pixels, STNBFileRef file, const ENPngCompressLvl compresssLvl, STNBPngSaveState* state, const UI32 paletteMaxSz);
	
	//Load
	
	typedef struct STNBPngLoadState_ {
		void* opaque;
	} STNBPngLoadState;
	
	void NBPngLoadState_init(STNBPngLoadState* obj);
	void NBPngLoadState_release(STNBPngLoadState* obj);

	STNBBitmapProps	NBPngLoadState_getProps(STNBPngLoadState* obj);
	//
	BOOL NBPng_loadFromPath(const char* rutaArchivo, const BOOL cargarDatos, STNBBitmap* dstBitmap, STNBArray* dstExtraChunks/*, STNBSha1Hash* guardarFirmaSHA1En, PTRFuncGetPngBlock funcConsumeExtraBlocks, void* paramConsumeExtraBlocks*/);
	BOOL NBPng_loadFromPathWithIHDR(const char* rutaArchivo, const BOOL cargarDatos, STNBBitmap* dstBitmap, STNBArray* dstExtraChunks, STNBPngIHDR* dstIHDR, STNBPngIHDRCustom* dstHeadCustom/*, STNBSha1Hash* guardarFirmaSHA1En, PTRFuncGetPngBlock funcConsumeExtraBlocks, void* paramConsumeExtraBlocks*/);
	//
	BOOL NBPng_loadFromFile(STNBFileRef file, const BOOL cargarDatos, STNBBitmap* dstBitmap, STNBArray* dstExtraChunks/*, STNBSha1Hash* guardarFirmaSHA1En, PTRFuncGetPngBlock funcConsumeExtraBlocks, void* paramConsumeExtraBlocks*/);
	BOOL NBPng_loadFromFileWithIHDR(STNBFileRef file, const BOOL cargarDatos, STNBBitmap* dstBitmap, STNBArray* dstExtraChunks, STNBPngIHDR* dstIHDR, STNBPngIHDRCustom* dstHeadCustom/*, STNBSha1Hash* guardarFirmaSHA1En, PTRFuncGetPngBlock funcConsumeExtraBlocks, void* paramConsumeExtraBlocks*/);
	//
	BOOL NBPng_loadStreamSetItf(STNBPngLoadState* eLoad, IFileItf* itf, void* itfObj);
	void NBPng_loadStreamLock(STNBPngLoadState* eLoad);
	void NBPng_loadStreamUnlock(STNBPngLoadState* eLoad);
	BOOL NBPng_loadWithState(STNBPngLoadState* eLoad, BYTE* pixsBuff, const UI32 pixsBuffSz, STNBArray* dstExtraChunks, STNBPngIHDR* dstIHDR, STNBPngIHDRCustom* dstHeadCustom/*, STNBSha1Hash* guardarFirmaSHA1En, PTRFuncGetPngBlock funcConsumeExtraBlocks, void* paramConsumeExtraBlocks*/);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBPng_h */
