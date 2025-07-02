#ifndef NB_GPU_TEXTURE_H
#define NB_GPU_TEXTURE_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBMemoryBlocks.h"
#include "nb/core/NBStructMap.h"
#include "nb/2d/NBBitmap.h"
#include "nb/2d/NBRect.h"

#ifdef __cplusplus
extern "C" {
#endif

//ENNBGpuTextureIdx

typedef enum ENNBGpuTextureIdx_ {
    ENNBGpuTextureIdx_0 = 0,
    ENNBGpuTextureIdx_1,
    ENNBGpuTextureIdx_2,
    ENNBGpuTextureIdx_3,
    //Count
    ENNBGpuTextureIdx_Count
} ENNBGpuTextureIdx;

const STNBEnumMap* NBGpuTextureIdx_getSharedEnumMap(void);

//ENNBGpuTextureCoordMode

typedef enum ENNBGpuTextureCoordMode_ {
    ENNBGpuTextureCoordMode_Repeat = 0, //pattern
    ENNBGpuTextureCoordMode_Clamp,      //single-image
    //
    ENNBGpuTextureCoordMode_Count
} ENNBGpuTextureCoordMode;

const STNBEnumMap* NBGpuTextureCoordMode_getSharedEnumMap(void);

//ENNBGpuTexturePixelMode

typedef enum ENNBGpuTexturePixelMode_ {
    ENNBGpuTexturePixelMode_Nearest = 0, //fast selection of nearest color
    ENNBGpuTexturePixelMode_Linear,      //calculation of merged color
    //
    ENNBGpuTexturePixelMode_Count
} ENNBGpuTexturePixelMode;

const STNBEnumMap* NBGpuTexturePixelMode_getSharedEnumMap(void);


//STNBGpuTextureCfg

#define STNBGpuTextureCfg_Zero   { ENNBBitmapColor_undef, 0, 0, FALSE }

typedef struct STNBGpuTextureCfg_ {
    ENNBBitmapColor color;
    UI32            width;
    UI32            height;
    BOOL            mipmapEnabled;
    ENNBGpuTextureCoordMode coordMode;
    ENNBGpuTexturePixelMode pixelMode;
} STNBGpuTextureCfg;

const STNBStructMap* NBGpuTextureCfg_getSharedStructMap(void);

//STNBGpuTextureChanges

typedef struct STNBGpuTextureChanges_ {
    BOOL            whole;  //all the content is new
    STNBRectI*      rects;  //subimages areas
    UI32            recsUse;
} STNBGpuTextureChanges;

const STNBStructMap* NBGpuTextureChanges_getSharedStructMap(void);

//STNBGpuTextureApiItf

typedef struct STNBGpuTextureApiItf_ {
    void* (*create)(const STNBGpuTextureCfg* cfg, void* usrData);
    void  (*destroy)(void* data, void* usrData);
    //
    BOOL  (*sync)(void* data, const STNBGpuTextureCfg* cfg, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBGpuTextureChanges* changes, void* usrData);
} STNBGpuTextureApiItf;

//

NB_OBJREF_HEADER(NBGpuTexture)

//

BOOL NBGpuTexture_prepare(STNBGpuTextureRef ref, const STNBGpuTextureCfg* cfg, const STNBGpuTextureApiItf* itf, void* itfParam);
BOOL NBGpuTexture_setImage(STNBGpuTextureRef ref, const STNBBitmapProps srcProps, const BYTE* srcData);
BOOL NBGpuTexture_setSubimage(STNBGpuTextureRef ref, const STNBPointI pos, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBRectI srcRect);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
