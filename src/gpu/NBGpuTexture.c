
#include "nb/NBFrameworkPch.h"
#include "nb/gpu/NBGpuTexture.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBStructMaps.h"
#include "nb/core/NBMngrStructMaps.h"

//NBGpuTextureIdx

STNBEnumMapRecord NBGpuTextureIdx_sharedEnumMapRecs[] = {
    { ENNBGpuTextureIdx_0, "ENNBGpuTextureIdx_0", "0" }
    , { ENNBGpuTextureIdx_1, "ENNBGpuTextureIdx_1", "1" }
    , { ENNBGpuTextureIdx_2, "ENNBGpuTextureIdx_2", "2" }
    , { ENNBGpuTextureIdx_3, "ENNBGpuTextureIdx_3", "3" }
};

STNBEnumMap NBGpuTextureIdx_sharedEnumMap = {
    "ENNBGpuTextureIdx"
    , NBGpuTextureIdx_sharedEnumMapRecs
    , (sizeof(NBGpuTextureIdx_sharedEnumMapRecs) / sizeof(NBGpuTextureIdx_sharedEnumMapRecs[0]))
};

const STNBEnumMap* NBGpuTextureIdx_getSharedEnumMap(void){
    return &NBGpuTextureIdx_sharedEnumMap;
}

//NBGpuTextureCoordMode

STNBEnumMapRecord NBGpuTextureCoordMode_sharedEnumMapRecs[] = {
    { ENNBGpuTextureCoordMode_Repeat, "ENNBGpuTextureCoordMode_Repeat", "repeat" }
    , { ENNBGpuTextureCoordMode_Clamp, "ENNBGpuTextureCoordMode_Clamp", "clamp" }
};

STNBEnumMap NBGpuTextureCoordMode_sharedEnumMap = {
    "ENNBGpuTextureCoordMode"
    , NBGpuTextureCoordMode_sharedEnumMapRecs
    , (sizeof(NBGpuTextureCoordMode_sharedEnumMapRecs) / sizeof(NBGpuTextureCoordMode_sharedEnumMapRecs[0]))
};

const STNBEnumMap* NBGpuTextureCoordMode_getSharedEnumMap(void){
    return &NBGpuTextureCoordMode_sharedEnumMap;
}

//NBGpuTexturePixelMode

STNBEnumMapRecord NBGpuTexturePixelMode_sharedEnumMapRecs[] = {
    { ENNBGpuTexturePixelMode_Nearest, "ENNBGpuTexturePixelMode_Nearest", "nearest" }
    , { ENNBGpuTexturePixelMode_Linear, "ENNBGpuTexturePixelMode_Linear", "linear" }
};

STNBEnumMap NBGpuTexturePixelMode_sharedEnumMap = {
    "ENNBGpuTexturePixelMode"
    , NBGpuTexturePixelMode_sharedEnumMapRecs
    , (sizeof(NBGpuTexturePixelMode_sharedEnumMapRecs) / sizeof(NBGpuTexturePixelMode_sharedEnumMapRecs[0]))
};

const STNBEnumMap* NBGpuTexturePixelMode_getSharedEnumMap(void){
    return &NBGpuTexturePixelMode_sharedEnumMap;
}

// NBGpuTextureCfg

STNBStructMapsRec NBGpuTextureCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBGpuTextureCfg_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBGpuTextureCfg_sharedStructMap);
    if(NBGpuTextureCfg_sharedStructMap.map == NULL){
        STNBGpuTextureCfg s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBGpuTextureCfg);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addEnumM(map, s, color, NBBitmapColor_getSharedEnumMap());
        NBStructMap_addUIntM(map, s, width);
        NBStructMap_addUIntM(map, s, color);
        NBStructMap_addBoolM(map, s, mipmapEnabled);
        NBStructMap_addEnumM(map, s, coordMode, NBGpuTextureCoordMode_getSharedEnumMap());
        NBStructMap_addEnumM(map, s, pixelMode, NBGpuTexturePixelMode_getSharedEnumMap());
        NBGpuTextureCfg_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBGpuTextureCfg_sharedStructMap);
    return NBGpuTextureCfg_sharedStructMap.map;
}

// NBGpuTextureChanges

STNBStructMapsRec NBGpuTextureChanges_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBGpuTextureChanges_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBGpuTextureChanges_sharedStructMap);
    if(NBGpuTextureChanges_sharedStructMap.map == NULL){
        STNBGpuTextureChanges s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBGpuTextureChanges);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addBoolM(map, s, whole);
        NBStructMap_addPtrToArrayOfStructM(map, s, rects, recsUse, ENNBStructMapSign_Unsigned, NBRectI_getSharedStructMap());
        NBGpuTextureChanges_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBGpuTextureChanges_sharedStructMap);
    return NBGpuTextureChanges_sharedStructMap.map;
}

//STNBGpuTextureOpq

typedef struct STNBGpuTextureOpq_ {
    STNBObject prnt;
    //
    STNBGpuTextureCfg   cfg;    //config
    STNBBitmap          bmp;    //bitmap
    //changes
    struct {
        BOOL            whole;  //all the content is new
        STNBArray       rects;  //STNBRectI, subimages areas
    } changes;
    //api
    struct {
        STNBGpuTextureApiItf    itf;
        void*                   itfParam;
        void*                   data;
    } api;
} STNBGpuTextureOpq;

NB_OBJREF_BODY(NBGpuTexture, STNBGpuTextureOpq, NBObject)

void NBGpuTexture_initZeroed(STNBObject* obj) {
    STNBGpuTextureOpq* opq = (STNBGpuTextureOpq*)obj;
    //
    NBBitmap_init(&opq->bmp);
    //changes
    {
        NBArray_initWithSz(&opq->changes.rects, sizeof(STNBRectI), NULL, 0, 32, 0.1f);
    }
}

void NBGpuTexture_uninitLocked(STNBObject* obj){
    STNBGpuTextureOpq* opq = (STNBGpuTextureOpq*)obj;
    //api
    {
        if(opq->api.data != NULL){
            if(opq->api.itf.destroy != NULL){
                (*opq->api.itf.destroy)(opq->api.data, opq->api.itfParam);
            }
            opq->api.data = NULL;
        }
        NBMemory_setZeroSt(opq->api.itf, STNBGpuTextureApiItf);
        opq->api.itfParam = NULL;
    }
    //
    NBStruct_stRelease(NBGpuTextureCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
    //
    NBBitmap_release(&opq->bmp);
    //changes
    {
        NBArray_release(&opq->changes.rects);
    }
}

//

BOOL NBGpuTexture_prepare(STNBGpuTextureRef ref, const STNBGpuTextureCfg* cfg, const STNBGpuTextureApiItf* itf, void* itfParam) {
    BOOL r = FALSE;
    STNBGpuTextureOpq* opq = (STNBGpuTextureOpq*)ref.opaque;
    NBASSERT(NBGpuTexture_isClass(ref))
    NBObject_lock(opq);
    if(cfg != NULL && cfg->width > 0 && cfg->height > 0 && opq->cfg.width == 0 && itf != NULL && itf->create != NULL && itf->destroy != NULL){
        void* data = (*itf->create)(cfg, itfParam);
        if(data != NULL){
            STNBBitmap bmp;
            NBBitmap_init(&bmp);
            if(!NBBitmap_create(&bmp, cfg->width, cfg->height, cfg->color)){
                //error
            } else {
                //swap
                STNBBitmap cpy = bmp;
                bmp         = opq->bmp;
                opq->bmp    = cpy;
                //cfg
                NBStruct_stRelease(NBGpuTextureCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
                NBStruct_stClone(NBGpuTextureCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
                //changes
                opq->changes.whole = TRUE;
                NBArray_empty(&opq->changes.rects);
                //api
                {
                    if(opq->api.data != NULL){
                        if(opq->api.itf.destroy != NULL){
                            (*opq->api.itf.destroy)(opq->api.data, opq->api.itfParam);
                        }
                        opq->api.data = NULL;
                    }
                    NBMemory_setZeroSt(opq->api.itf, STNBGpuTextureApiItf);
                    opq->api.itfParam = NULL;
                    //
                    if(itf != NULL){
                        opq->api.itf = *itf;
                        opq->api.itfParam = itfParam;
                    }
                    //data
                    opq->api.data = data; data = NULL; //consume
                }
                //
                r = TRUE;
            }
            NBBitmap_release(&bmp);
        }
        //destroy (if not consumed)
        if(data != NULL && itf->destroy != NULL){
            (*itf->destroy)(data, itfParam);
        }
    }
    NBObject_unlock(opq);
    return r;
}


BOOL NBGpuTexture_setImage(STNBGpuTextureRef ref, const STNBBitmapProps srcProps, const BYTE* srcData){
    BOOL r = FALSE;
    STNBGpuTextureOpq* opq = (STNBGpuTextureOpq*)ref.opaque;
    NBASSERT(NBGpuTexture_isClass(ref))
    NBObject_lock(opq);
    if(opq->cfg.width == srcProps.size.width && opq->cfg.height == srcProps.size.height){
        const STNBPointI pstPos = { 0, 0 };
        const STNBColor8 pstColor = { 255, 255, 255, 255 };
        if(!NBBitmap_pasteBitmapData(&opq->bmp, pstPos, srcProps, srcData, pstColor)){
            //error
        } else {
            //changes
            opq->changes.whole = TRUE;
            NBArray_empty(&opq->changes.rects);
            //
            r = TRUE;
        }
    }
    NBObject_unlock(opq);
    return r;
}

BOOL NBGpuTexture_setSubimage(STNBGpuTextureRef ref, const STNBPointI pos, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBRectI pSrcRect){
    BOOL r = FALSE;
    STNBGpuTextureOpq* opq = (STNBGpuTextureOpq*)ref.opaque;
    NBASSERT(NBGpuTexture_isClass(ref))
    NBObject_lock(opq);
    if(opq->cfg.width >0 && opq->cfg.height > 0){
        STNBRectI srcRect = pSrcRect;
        const STNBColor8 pstColor = { 255, 255, 255, 255 };
        if(!NBBitmap_pasteValidatedSrcRect(&opq->bmp, pos, srcProps, &srcRect)){
            //error
        } else if(!NBBitmap_pasteBitmapDataRect(&opq->bmp, pos, srcProps, srcData, srcRect, pstColor)){
            //error
        } else {
            //changes
            if(opq->changes.whole){
                //already flagged
            } else if(opq->cfg.width == srcRect.width && opq->cfg.height == srcRect.height){
                //whole image changed
                opq->changes.whole = TRUE;
                NBArray_empty(&opq->changes.rects);
            } else {
                //add rect
                NBArray_addValue(&opq->changes.rects, srcRect);
            }
            //
            r = TRUE;
        }
    }
    NBObject_unlock(opq);
    return r;
}
