//
//  NBFontBitmaps.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/6/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/fonts/NBFontBitmaps.h"
//
#include "nb/core/NBCompare.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBArraySorted.h"
#include "nb/fonts/NBFontCodesMap.h"
#include "nb/fonts/NBFontShapesMetricsMap.h"

typedef struct STNBFontBitmapsMapRec_ {
	UI32					shapeId;
	UI32					version;
	UI16					iAtlas;
	UI16					iArea;
	STNBFontShapeMetrics	metrics;
	STNBPoint				center;
} STNBFontBitmapsMapRec;

BOOL NBCompare_NBFontBitmapsMapRec(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBFontBitmapsMapRec))
	if(dataSz == sizeof(STNBFontBitmapsMapRec)){
		const STNBFontBitmapsMapRec* o1 = (const STNBFontBitmapsMapRec*)data1;
		const STNBFontBitmapsMapRec* o2 = (const STNBFontBitmapsMapRec*)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return o1->shapeId == o2->shapeId ? TRUE : FALSE;
			case ENCompareMode_Lower:
				return o1->shapeId < o2->shapeId ? TRUE : FALSE;
			case ENCompareMode_LowerOrEqual:
				return o1->shapeId <= o2->shapeId ? TRUE : FALSE;
			case ENCompareMode_Greater:
				return o1->shapeId > o2->shapeId ? TRUE : FALSE;
			case ENCompareMode_GreaterOrEqual:
				return o1->shapeId >= o2->shapeId ? TRUE : FALSE;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}

typedef struct STNBFontBitmapsAtlas_ {
	UI32			version;	//increments each time a shape is drawn in this atlas
	STNBAtlasMap	atlas;
	STNBBitmap		bitmap;
	STNBArray		shapeIds;	//UI32
} STNBFontBitmapsAtlas;

//Metrics Itf
void 					NBFontBitmaps_retain_(void* obj);
void 					NBFontBitmaps_release_(void* obj);
float 					NBFontBitmaps_getNativeSz_(void* obj);
UI32 					NBFontBitmaps_getShapeId_(void* obj, const UI32 unicode);
STNBFontMetrics 		NBFontBitmaps_getFontMetrics_(void* obj);
STNBFontMetrics 		NBFontBitmaps_getFontMetricsForSz_(void* obj, const float fontSz);
STNBFontShapeMetrics	NBFontBitmaps_getFontShapeMetrics_(void* obj, const UI32 shapeId);
STNBFontShapeMetrics	NBFontBitmaps_getFontShapeMetricsForSz_(void* obj, const UI32 shapeId, const float fontSz);

typedef struct STNBFontBitmapsOpq_ {
	UI32						version;	//increments each time a shape is drawn in any atlas
	STNBFontLines*				font;
	float						fontSz;
	UI32						family;
	UI32						subfamily;
	UI8							styleMask;	//ENNBFontStyleBit
	STNBFontMetrics				metrics;
	UI16						shapesMargin;
	UI32						atlasSz;
	STNBFontCodesMap			codesMap;
	STNBFontShapesMetricsMap	metricsMap;
	STNBArraySorted				map;		//STNBFontBitmapsMapRec
	STNBArray					atlases;	//STNBFontBitmapsAtlas*
	STNBString					strs;
	UI32						retrainCout;
} STNBFontBitmapsOpq;

//Rendering storing lines at NBFontLines object
BOOL NBFontBitmaps_syncShapeOpq(STNBFontBitmapsOpq* opq, const STNBFontLinesSize* szShapes, const STNBFontLinesShape* shape);
//Rendering without storing lines at NBFontLines object
BOOL NBFontBitmaps_syncShapeCalculatedOpq(STNBFontBitmapsOpq* opq, const UI32 shapeId, const STNBFontLinesData* data);
	
void NBFontBitmaps_initWithSz(STNBFontBitmaps* obj, STNBFontLines* font, const float fontSz, const UI16 shapesMargin){
	STNBFontBitmapsOpq* opq =  obj->opaque = NBMemory_allocType(STNBFontBitmapsOpq);
	NBMemory_setZeroSt(*opq, STNBFontBitmapsOpq);
	opq->version		= 0;
	opq->font			= font;
	opq->fontSz			= fontSz;
	opq->family			= 0;
	opq->subfamily		= 0;
	opq->styleMask		= 0;
	opq->shapesMargin	= shapesMargin;
	opq->atlasSz		= 64;
	NBFontCodesMap_init(&opq->codesMap);
	NBFontShapesMetricsMap_init(&opq->metricsMap);
	NBArraySorted_init(&opq->map, sizeof(STNBFontBitmapsMapRec), NBCompare_NBFontBitmapsMapRec);
	NBArray_init(&opq->atlases, sizeof(STNBFontBitmapsAtlas*), NULL);
	NBString_init(&opq->strs);
	NBString_concatByte(&opq->strs, '\0');
	if(opq->font != NULL){
		opq->metrics		= NBFontLines_getMetrics(opq->font);
		{
			const char* family		= NULL;
			const char* subfamily	= NULL;
			UI8 styleMask			= 0;
			NBFontLines_getStyle(opq->font, &family, &subfamily, &styleMask);
			opq->family		= opq->strs.length; NBString_concat(&opq->strs, family); NBString_concatByte(&opq->strs, '\0');
			opq->subfamily	= opq->strs.length; NBString_concat(&opq->strs, subfamily); NBString_concatByte(&opq->strs, '\0');
			opq->styleMask	= styleMask;
		}
		//calculate bitmaps size
		{
			const STNBFontMetrics m		= NBFontLines_getMetricsForSz(opq->font, fontSz);
			const UI32 linesExpected	= 16;
			const UI32 linesHeight		= (m.ascender + m.descender + shapesMargin) * linesExpected;
			const UI32 maxSz			= 1024;
			while(opq->atlasSz < linesHeight && opq->atlasSz < maxSz){
				opq->atlasSz *= 2;
			}
		}
		NBFontLines_retain(opq->font);
	}
	opq->retrainCout = 1;
}

void NBFontBitmaps_retain(STNBFontBitmaps* obj){
	STNBFontBitmapsOpq* opq = (STNBFontBitmapsOpq*)obj->opaque;
	NBASSERT(opq->retrainCout > 0)
	opq->retrainCout++;
}

void NBFontBitmaps_release(STNBFontBitmaps* obj){
	STNBFontBitmapsOpq* opq = (STNBFontBitmapsOpq*)obj->opaque;
	NBASSERT(opq->retrainCout > 0)
	opq->retrainCout--;
	if(opq->retrainCout == 0){
		if(opq->font != NULL){
			NBFontLines_release(opq->font);
			opq->font = NULL;
		}
		{
			UI32 i; for(i = 0 ; i < opq->atlases.use; i++){
				STNBFontBitmapsAtlas* a = NBArray_itmValueAtIndex(&opq->atlases, STNBFontBitmapsAtlas*, i);
				NBAtlasMap_release(&a->atlas);
				NBBitmap_release(&a->bitmap);
				NBArray_release(&a->shapeIds);
				NBMemory_free(a);
			}
			NBArray_empty(&opq->atlases);
			NBArray_release(&opq->atlases);
		}
		NBFontShapesMetricsMap_release(&opq->metricsMap);
		NBFontCodesMap_release(&opq->codesMap);
		NBArraySorted_release(&opq->map);
		NBString_release(&opq->strs);
		//
		NBMemory_free(obj->opaque);
		obj->opaque = NULL;
	}
}

//Version

UI32 NBFontBitmaps_getVersion(const STNBFontBitmaps* obj){
	STNBFontBitmapsOpq* opq = (STNBFontBitmapsOpq*)obj->opaque;
	return opq->version;
}

//Style

void NBFontBitmaps_getStyle(const STNBFontBitmaps* obj, const char** dstFamily, const char** dstSubfamily, UI8* dstStyleMask){
	const STNBFontBitmapsOpq* opq = (STNBFontBitmapsOpq*)obj->opaque;
	if(dstFamily != NULL) *dstFamily		= &opq->strs.str[opq->family];
	if(dstSubfamily != NULL) *dstSubfamily	= &opq->strs.str[opq->subfamily];
	if(dstStyleMask != NULL) *dstStyleMask	= opq->styleMask;
}

float NBFontBitmaps_getSize(const STNBFontBitmaps* obj){
	const STNBFontBitmapsOpq* opq = (STNBFontBitmapsOpq*)obj->opaque;
	return opq->fontSz;
}

// Metrics

STNBFontMetricsIRef NBFontBitmaps_getMetricsItfRef(STNBFontBitmaps* obj, const float defFontSize){
	STNBFontBitmapsOpq* opq = (STNBFontBitmapsOpq*)obj->opaque;
	STNBFontMetricsIRef r;
	NBFontBitmaps_getMetricsItf(obj, &r.itf, &r.itfParam);
	r.fontSz = (defFontSize <= 0 ? opq->fontSz : defFontSize);
	return r;
}

void NBFontBitmaps_getMetricsItf(STNBFontBitmaps* obj, STNBFontMetricsI* dstItf, void** dstItfParam){
	dstItf->retain						= NBFontBitmaps_retain_;
	dstItf->release						= NBFontBitmaps_release_;
	dstItf->getNativeSz					= NBFontBitmaps_getNativeSz_;
	dstItf->getShapeId					= NBFontBitmaps_getShapeId_;
	dstItf->getFontMetrics				= NBFontBitmaps_getFontMetrics_;
	dstItf->getFontMetricsForSz			= NBFontBitmaps_getFontMetricsForSz_;
	dstItf->getFontShapeMetrics			= NBFontBitmaps_getFontShapeMetrics_;
	dstItf->getFontShapeMetricsForSz	= NBFontBitmaps_getFontShapeMetricsForSz_;
	*dstItfParam						= obj;
}

STNBFontMetrics NBFontBitmaps_getMetrics(const STNBFontBitmaps* obj){
	STNBFontBitmapsOpq* opq = (STNBFontBitmapsOpq*)obj->opaque;
	//PRINTF_INFO("FontBitmaps, emBoxSz(%f) ascender(%f) descender(%f) height(%f).\n", opq->metrics.emBoxSz, opq->metrics.ascender, opq->metrics.descender, opq->metrics.height);
	return opq->metrics;
}

STNBFontMetrics NBFontBitmaps_getMetricsForSz(const STNBFontBitmaps* obj, const float fontSz){
	STNBFontMetrics r;
	const STNBFontBitmapsOpq* opq = (STNBFontBitmapsOpq*)obj->opaque;
	const float emScale = fontSz / opq->metrics.emBoxSz;
	r.emBoxSz	= opq->metrics.emBoxSz * emScale;
	r.ascender	= opq->metrics.ascender * emScale;
	r.descender	= opq->metrics.descender * emScale;
	r.height	= opq->metrics.height * emScale;
	//PRINTF_INFO("FontBitmaps, size(%f) emBoxSz(%f) ascender(%f) descender(%f) height(%f).\n", fontSz, r.emBoxSz, r.ascender, r.descender, r.height);
	return r;
}

//Shapes

UI32 NBFontBitmaps_getShapeIdOpq(STNBFontBitmapsOpq* opq, const UI32 unicode, BOOL* dstIsNewShapeId){
	UI32 r = 0;
	if(dstIsNewShapeId != NULL) *dstIsNewShapeId  = FALSE;
	if(!NBFontCodesMap_getShapeId(&opq->codesMap, unicode, &r)){
		//Search in parent font
		if(opq->font != NULL){
			r = NBFontLines_getShapeId(opq->font, unicode);
			//Add to codesMap
			NBFontCodesMap_addRecord(&opq->codesMap, unicode, r);
			if(dstIsNewShapeId != NULL) *dstIsNewShapeId  = TRUE;
		}
	}
	return r;
}

UI32 NBFontBitmaps_getShapeId(STNBFontBitmaps* obj, const UI32 unicode){
	STNBFontBitmapsOpq* opq = (STNBFontBitmapsOpq*)obj->opaque;
	return NBFontBitmaps_getShapeIdOpq(opq, unicode, NULL);
}

STNBFontShapeMetrics NBFontBitmaps_getShapeMetrics(STNBFontBitmaps* obj, const UI32 shapeId){
	STNBFontShapeMetrics r;
	STNBFontBitmapsOpq* opq = (STNBFontBitmapsOpq*)obj->opaque;
	if(!NBFontShapesMetricsMap_getMetrics(&opq->metricsMap, shapeId, &r)){
		//Search in parent font
		if(opq->font != NULL){
			r = NBFontLines_getShapeMetrics(opq->font, shapeId);
			//PRINTF_INFO("FontBitmaps, shapeId(%d) emBoxSz(%f) advance(%f) hBearing(%f).\n", shapeId, opq->metrics.emBoxSz, r.hAdvance, r.hBearingX);
			//Add to metrics map
			NBFontShapesMetricsMap_addRecord(&opq->metricsMap, shapeId, &r);
		}
	}
	return r;
}

STNBFontShapeMetrics NBFontBitmaps_getShapeMetricsForSz(STNBFontBitmaps* obj, const UI32 shapeId, const float fontSz){
	STNBFontShapeMetrics r = NBFontBitmaps_getShapeMetrics(obj, shapeId);
	STNBFontBitmapsOpq* opq = (STNBFontBitmapsOpq*)obj->opaque;
	const float emScale = fontSz / opq->metrics.emBoxSz;
	r.hBearingX *= emScale;
	r.hBearingY *= emScale;
	r.hAdvance	*= emScale;
	r.vBearingX *= emScale;
	r.vBearingY *= emScale;
	r.vAdvance	*= emScale;
	r.width 	*= emScale;
	r.height	*= emScale;
	//PRINTF_INFO("FontBitmaps, shapeId(%d) size(%f) emBoxSz(%f) advance(%f) hBearing(%f).\n", shapeId, fontSz, opq->metrics.emBoxSz, r.hAdvance, r.hBearingX);
	return r;
}

//

STNBFontBitmap NBFontBitmaps_getBitmapAtIndex(const STNBFontBitmaps* obj, const UI32 index){
	STNBFontBitmap r;
	STNBFontBitmapsOpq* opq = (STNBFontBitmapsOpq*)obj->opaque;
	if(index >= opq->atlases.use){
		NBMemory_setZeroSt(r, STNBFontBitmap);
	} else {
		const STNBFontBitmapsAtlas* atlas = NBArray_itmValueAtIndex(&opq->atlases, STNBFontBitmapsAtlas*, index);
		r.version	= atlas->version;
		r.atlas		= &atlas->atlas;
		r.bitmap	= &atlas->bitmap;
		r.shapeIds	= &atlas->shapeIds;
	}
	return r;
}

STNBFontBitmapShape NBFontBitmaps_getBitmapShape(STNBFontBitmaps* obj, const UI32 shapeId, const BOOL syncIfNecesary){
	STNBFontBitmapShape r;
	STNBFontBitmapsOpq* opq = (STNBFontBitmapsOpq*)obj->opaque;
	STNBFontBitmapsMapRec srch;
	srch.shapeId	= shapeId;
	const SI32 iFnd = NBArraySorted_indexOf(&opq->map, &srch, sizeof(srch), NULL);
	if(iFnd == -1){
		NBMemory_setZeroSt(r, STNBFontBitmapShape);
		if(syncIfNecesary){
			//Rendering without storing lines at NBFontLines object
			{
				STNBMatrix matrix;
				STNBFontLinesData data;
				NBMemory_setZeroSt(data, STNBFontLinesData);
				NBArray_init(&data.vPoints, sizeof(STNBFontLineVertex), NULL);
				NBArray_init(&data.vLines, sizeof(STNBFontLineVertex), NULL);
				NBMatrix_setIdentity(&matrix);
				if(!NBFontLines_getLinesShapeCalculated(opq->font, opq->fontSz, matrix, shapeId, &data)){
					NBASSERT(FALSE)
				} else {
					if(!NBFontBitmaps_syncShapeCalculatedOpq(opq, shapeId, &data)){
						NBASSERT(FALSE)
					} else {
						r = NBFontBitmaps_getBitmapShape(obj, shapeId, FALSE);
						NBASSERT(r.bitmap != NULL);
					}
				}
				NBArray_release(&data.vPoints);
				NBArray_release(&data.vLines);
			}
			//Rendering storing lines at NBFontLines object
			/*{
				const STNBFontLinesSizeShape shape = NBFontLines_getLinesShape(opq->font, opq->fontSz, 0, shapeId);
				NBASSERT(shape.grpSize != NULL && shape.shape != NULL)
				if(shape.grpSize != NULL && shape.shape != NULL){
					if(!NBFontBitmaps_syncShapeOpq(opq, shape.grpSize, shape.shape)){
						NBASSERT(FALSE)
					} else {
						r = NBFontBitmaps_getBitmapShape(obj, shapeId, FALSE);
						NBASSERT(r.bitmap != NULL);
					}
				}
			}*/
		}
	} else {
		const STNBFontBitmapsMapRec* idx = NBArraySorted_itmPtrAtIndex(&opq->map, STNBFontBitmapsMapRec, iFnd);
		const STNBFontBitmapsAtlas* atlas = NBArray_itmValueAtIndex(&opq->atlases, STNBFontBitmapsAtlas*, idx->iAtlas);
		r.shapeId	= idx->shapeId;
		r.version	= idx->version;
		r.metrics	= idx->metrics;
		r.iBitmap	= idx->iAtlas;
		r.iArea		= idx->iArea;
		r.bitmap	= &atlas->bitmap;
		r.area		= NBAtlasMap_getAreaAtIndex(&atlas->atlas, idx->iArea);
		r.center	= idx->center;
	}
	return r;
}

STNBFontBitmapShape NBFontBitmaps_getBitmapShapeByUnicode(STNBFontBitmaps* obj, const UI32 unicode, const BOOL syncIfNecesary){
	STNBFontBitmapsOpq* opq = (STNBFontBitmapsOpq*)obj->opaque;
	return NBFontBitmaps_getBitmapShape(obj, NBFontBitmaps_getShapeIdOpq(opq, unicode, NULL), syncIfNecesary);
}

//

BOOL NBFontBitmaps_syncShapeOpq(STNBFontBitmapsOpq* opq, const STNBFontLinesSize* szShapes, const STNBFontLinesShape* shape){
	BOOL r = FALSE;
	SI32 iFnd;
	STNBFontBitmapsMapRec srch;
	NBMemory_setZeroSt(srch, STNBFontBitmapsMapRec);
	srch.shapeId	= shape->shapeId;
	iFnd			= NBArraySorted_indexOf(&opq->map, &srch, sizeof(srch), NULL);
	if(iFnd == -1){
		const float width	= shape->box.xMax - shape->box.xMin; NBASSERT((float)((SI32)width) == width) NBASSERT(width >= 0)
		const float height	= shape->box.yMax - shape->box.yMin; NBASSERT((float)((SI32)height) == height) NBASSERT(height >= 0)
		STNBFontBitmapsAtlas* atlas = NULL;
		STNBRectI rec;
		NBMemory_setZeroSt(rec, STNBRectI);
		//Search space in last atlas
		if(opq->atlases.use > 0){
			STNBFontBitmapsAtlas* lastAtlas = NBArray_itmValueAtIndex(&opq->atlases, STNBFontBitmapsAtlas*, opq->atlases.use - 1);
			rec = NBAtlasMap_reserveArea(&lastAtlas->atlas, width, height, opq->shapesMargin, opq->shapesMargin);
			NBASSERT(rec.x >= 0 && rec.y >= 0)
			if(rec.width >= width || rec.height >= height){
				atlas		= lastAtlas;
				srch.iAtlas	= opq->atlases.use - 1;
				srch.iArea	= NBAtlasMap_getAreasCount(&atlas->atlas) - 1;
			}
		}
		//Create new atlas
		if(atlas == NULL){
			STNBFontBitmapsAtlas* new = NBMemory_allocType(STNBFontBitmapsAtlas);
			new->version = 0;
			NBAtlasMap_initWithSz(&new->atlas, 1, opq->atlasSz, opq->atlasSz);
			rec = NBAtlasMap_reserveArea(&new->atlas, width, height, opq->shapesMargin, opq->shapesMargin);
			NBASSERT(rec.x >= 0 && rec.y >= 0)
			if(rec.width < width || rec.height < height){
				NBAtlasMap_release(&new->atlas);
				NBMemory_free(new);
				new = NULL;
			} else {
				{
					NBBitmap_init(&new->bitmap);
					NBBitmap_createAndSet(&new->bitmap, opq->atlasSz, opq->atlasSz, ENNBBitmapColor_ALPHA8, 0);
				}
				{
					NBArray_init(&new->shapeIds, sizeof(UI32), NULL);
				}
				NBArray_addValue(&opq->atlases, new);
				atlas		= new;
				srch.iAtlas	= opq->atlases.use - 1;
				srch.iArea	= NBAtlasMap_getAreasCount(&atlas->atlas) - 1;
			}
		}
		//Render
		NBASSERT(atlas != NULL)
		if(atlas != NULL){
			const STNBBitmapProps bmpProps = NBBitmap_getProps(&atlas->bitmap);
			BYTE* pixels = NBBitmap_getData(&atlas->bitmap);
			NBASSERT(rec.x >= 0 && rec.y >= 0)
			//Render points
			if(shape->points.count != 0){
				const STNBFontLineVertex* vrtx = NBArray_itmPtrAtIndex(&szShapes->vertexs, STNBFontLineVertex, shape->points.first);
				const STNBFontLineVertex* vrtxAfterLast = vrtx + shape->points.count;
				while(vrtx < vrtxAfterLast){
					const SI32 x = rec.x - shape->box.xMin + vrtx->pos;
					const SI32 y = rec.y + (shape->box.yMax - vrtx->line) - 1; //invertred-y-axis
					NBASSERT(x >= 0 && x < opq->atlasSz)
					NBASSERT(y >= 0 && y < opq->atlasSz)
					pixels[(y * bmpProps.bytesPerLine) + x] = vrtx->intensity;
					//Next
					vrtx++;
				}
			}
			//Render  lines
			if(shape->lines.count != 0){
				NBASSERT(srch.iAtlas < opq->atlases.use)
				NBASSERT(srch.iArea	< NBAtlasMap_getAreasCount(&atlas->atlas))
				const STNBFontLineVertex* vrtx = NBArray_itmPtrAtIndex(&szShapes->vertexs, STNBFontLineVertex, shape->lines.first);
				const STNBFontLineVertex* vrtxAfterLast = vrtx + (shape->lines.count * 2);
				while(vrtx < vrtxAfterLast){
					NBASSERT(vrtx->line == (vrtx + 1)->line) //Deben pertenecer a la misma linea
					const SI32 y 	= rec.y + (shape->box.yMax - vrtx->line) - 1; //eje-y-invertido, -1 porque los xMax/yMax apuntan al siguiente pixel
					const SI32 xEnd = rec.x - shape->box.xMin + (vrtx + 1)->pos;
					SI32 xIni		= rec.x - shape->box.xMin + vrtx->pos;
					NBASSERT(y >= 0 && y < opq->atlasSz)
					NBASSERT(xEnd >= 0 && xEnd < opq->atlasSz)
					NBASSERT(xIni >= 0 && xIni < opq->atlasSz)
					NBASSERT(xIni <= xEnd)
					//Pintar pixeles
					BYTE* pBuff = &pixels[(y * bmpProps.bytesPerLine) + xIni];
					const BYTE* pBuffAfLst = pBuff + (xEnd - xIni) + 1;
					while(pBuff < pBuffAfLst){
						*pBuff = vrtx->intensity;
						pBuff++;
					}
					//Siguiente
					vrtx += 2;
				}
			}
			//Register
			srch.version	= shape->version;
			srch.metrics	= shape->metrics;
			srch.center.x	= shape->box.xMin;
			srch.center.y	= -shape->box.yMax;
			NBArraySorted_add(&opq->map, &srch, sizeof(srch));
			{
				const UI32 shapeId = shape->shapeId;
				NBASSERT(atlas->shapeIds.use == srch.iArea)
				NBArray_addValue(&atlas->shapeIds, shapeId);
			}
			atlas->version++;
			opq->version++;
			r = TRUE;
		}
	}
	return r;
}

BOOL NBFontBitmaps_syncShapeCalculatedOpq(STNBFontBitmapsOpq* opq, const UI32 shapeId, const STNBFontLinesData* data){
	BOOL r = FALSE;
	SI32 iFnd;
	STNBFontBitmapsMapRec srch;
	NBMemory_setZeroSt(srch, STNBFontBitmapsMapRec);
	srch.shapeId	= shapeId;
	iFnd			= NBArraySorted_indexOf(&opq->map, &srch, sizeof(srch), NULL);
	if(iFnd == -1){
		const float width	= data->box.xMax - data->box.xMin; NBASSERT((float)((SI32)width) == width) NBASSERT(width >= 0)
		const float height	= data->box.yMax - data->box.yMin; NBASSERT((float)((SI32)height) == height) NBASSERT(height >= 0)
		STNBFontBitmapsAtlas* atlas = NULL;
		STNBRectI rec;
		NBMemory_setZeroSt(rec, STNBRectI);
		//Search space in last atlas
		if(opq->atlases.use > 0){
			STNBFontBitmapsAtlas* lastAtlas = NBArray_itmValueAtIndex(&opq->atlases, STNBFontBitmapsAtlas*, opq->atlases.use - 1);
			rec = NBAtlasMap_reserveArea(&lastAtlas->atlas, width, height, opq->shapesMargin, opq->shapesMargin);
			NBASSERT(rec.x >= 0 && rec.y >= 0)
			if(rec.width >= width || rec.height >= height){
				atlas		= lastAtlas;
				srch.iAtlas	= opq->atlases.use - 1;
				srch.iArea	= NBAtlasMap_getAreasCount(&atlas->atlas) - 1;
			}
		}
		//Create new atlas
		if(atlas == NULL){
			STNBFontBitmapsAtlas* new = NBMemory_allocType(STNBFontBitmapsAtlas);
			new->version = 0;
			NBAtlasMap_initWithSz(&new->atlas, 1, opq->atlasSz, opq->atlasSz);
			rec = NBAtlasMap_reserveArea(&new->atlas, width, height, opq->shapesMargin, opq->shapesMargin);
			NBASSERT(rec.x >= 0 && rec.y >= 0)
			if(rec.width < width || rec.height < height){
				NBAtlasMap_release(&new->atlas);
				NBMemory_free(new);
				new = NULL;
			} else {
				{
					NBBitmap_init(&new->bitmap);
					NBBitmap_createAndSet(&new->bitmap, opq->atlasSz, opq->atlasSz, ENNBBitmapColor_ALPHA8, 0);
				}
				{
					NBArray_init(&new->shapeIds, sizeof(UI32), NULL);
				}
				NBArray_addValue(&opq->atlases, new);
				atlas		= new;
				srch.iAtlas	= opq->atlases.use - 1;
				srch.iArea	= NBAtlasMap_getAreasCount(&atlas->atlas) - 1;
			}
		}
		//Render
		NBASSERT(atlas != NULL)
		if(atlas != NULL){
			const STNBBitmapProps bmpProps = NBBitmap_getProps(&atlas->bitmap);
			BYTE* pixels = NBBitmap_getData(&atlas->bitmap);
			NBASSERT(rec.x >= 0 && rec.y >= 0)
			//Render points
			if(data->vPoints.use > 0){
				const STNBFontLineVertex* vrtx = NBArray_itmPtrAtIndex(&data->vPoints, STNBFontLineVertex, 0);
				const STNBFontLineVertex* vrtxAfterLast = vrtx + data->vPoints.use;
				while(vrtx < vrtxAfterLast){
					const SI32 x = rec.x - data->box.xMin + vrtx->pos;
					const SI32 y = rec.y + (data->box.yMax - vrtx->line) - 1; //invertred-y-axis
					NBASSERT(x >= 0 && x < opq->atlasSz)
					NBASSERT(y >= 0 && y < opq->atlasSz)
					pixels[(y * bmpProps.bytesPerLine) + x] = vrtx->intensity;
					//Next
					vrtx++;
				}
			}
			//Render  lines
			if(data->vLines.use != 0){
				NBASSERT(srch.iAtlas < opq->atlases.use)
				NBASSERT(srch.iArea	< NBAtlasMap_getAreasCount(&atlas->atlas))
				const STNBFontLineVertex* vrtx = NBArray_itmPtrAtIndex(&data->vLines, STNBFontLineVertex, 0);
				const STNBFontLineVertex* vrtxAfterLast = vrtx + data->vLines.use;
				while(vrtx < vrtxAfterLast){
					NBASSERT(vrtx->line == (vrtx + 1)->line) //Deben pertenecer a la misma linea
					const SI32 y 	= rec.y + (data->box.yMax - vrtx->line) - 1; //eje-y-invertido, -1 porque los xMax/yMax apuntan al siguiente pixel
					const SI32 xEnd = rec.x - data->box.xMin + (vrtx + 1)->pos;
					SI32 xIni		= rec.x - data->box.xMin + vrtx->pos;
					NBASSERT(y >= 0 && y < opq->atlasSz)
					NBASSERT(xEnd >= 0 && xEnd < opq->atlasSz)
					NBASSERT(xIni >= 0 && xIni < opq->atlasSz)
					NBASSERT(xIni <= xEnd)
					//Pintar pixeles
					BYTE* pBuff = &pixels[(y * bmpProps.bytesPerLine) + xIni];
					const BYTE* pBuffAfLst = pBuff + (xEnd - xIni) + 1;
					while(pBuff < pBuffAfLst){
						*pBuff = vrtx->intensity;
						pBuff++;
					}
					//Siguiente
					vrtx += 2;
				}
			}
			//Register
			srch.version	= 0;
			srch.metrics	= data->metrics;
			srch.center.x	= data->box.xMin;
			srch.center.y	= -data->box.yMax;
			NBArraySorted_add(&opq->map, &srch, sizeof(srch));
			{
				NBASSERT(atlas->shapeIds.use == srch.iArea)
				NBArray_addValue(&atlas->shapeIds, shapeId);
			}
			atlas->version++;
			opq->version++;
			r = TRUE;
		}
	}
	return r;
}

UI32 NBFontBitmaps_syncBitmapShapes(STNBFontBitmaps* obj, const UI32* shapeIds, const UI32 shapeIdsSz){
	UI32 r = 0;
	STNBFontBitmapsOpq* opq = (STNBFontBitmapsOpq*)obj->opaque;
	if(opq->font != NULL){
		UI32 i; for(i = 0; i < shapeIdsSz; i++){
			const UI32 shapeId = shapeIds[i];
			const STNBFontLinesSizeShape shape = NBFontLines_getLinesShape(opq->font, opq->fontSz, shapeId);
			if(shape.grpSize != NULL && shape.shape != NULL){
				if(NBFontBitmaps_syncShapeOpq(opq, shape.grpSize, shape.shape)){
					r++;
				}
			}
		}
	}
	return r;
}

UI32 NBFontBitmaps_syncBitmapShapesOfUnicodes(STNBFontBitmaps* obj, const UI32* unicodes, const UI32 unicodesSz){
	UI32 r = 0;
	STNBFontBitmapsOpq* opq = (STNBFontBitmapsOpq*)obj->opaque;
	if(opq->font != NULL){
		UI32 i; for(i = 0; i < unicodesSz; i++){
			BOOL isNewShapeId = FALSE;
			const UI32 shapeId = NBFontBitmaps_getShapeIdOpq(opq, unicodes[i], &isNewShapeId);
			if(isNewShapeId){
				const STNBFontLinesSizeShape shape = NBFontLines_getLinesShape(opq->font, opq->fontSz, shapeId);
				if(shape.grpSize != NULL && shape.shape != NULL){
					if(NBFontBitmaps_syncShapeOpq(opq, shape.grpSize, shape.shape)){
						r++;
					}
				}
			}
		}
	}
	return r;
}

UI32 NBFontBitmaps_syncBitmapsShapes(STNBFontBitmaps* obj){
	STNBFontBitmapsOpq* opq = (STNBFontBitmapsOpq*)obj->opaque;
	return NBFontBitmaps_syncBitmapsShapesOfFont(obj, opq->font);
}

UI32 NBFontBitmaps_syncBitmapsShapesOfFont(STNBFontBitmaps* obj, STNBFontLines* font){
	UI32 r = 0;
	if(font != NULL){
		STNBFontBitmapsOpq* opq = (STNBFontBitmapsOpq*)obj->opaque;
		const STNBFontLinesSize* grpSize = NBFontLines_getLinesSize(font, opq->fontSz);
		if(grpSize != NULL){ //If null, the current grpSize has no shapes loaded
			UI32 i; for(i = 0 ; i < grpSize->shapes.use; i++){
				const STNBFontLinesShape* shape = NBArraySorted_itmPtrAtIndex(&grpSize->shapes, STNBFontLinesShape, i);
				if(NBFontBitmaps_syncShapeOpq(opq, grpSize, shape)){
					r++;
				}
			}
		}
	}
	return r;
}

//Metrics Itf
void NBFontBitmaps_retain_(void* obj){
	NBFontBitmaps_retain((STNBFontBitmaps*)obj);
}
void NBFontBitmaps_release_(void* obj){
	NBFontBitmaps_release((STNBFontBitmaps*)obj);
}
float NBFontBitmaps_getNativeSz_(void* obj){
	return NBFontBitmaps_getSize((STNBFontBitmaps*)obj);
}
UI32 NBFontBitmaps_getShapeId_(void* obj, const UI32 unicode){
	return NBFontBitmaps_getShapeId((STNBFontBitmaps*)obj, unicode);
}
STNBFontMetrics NBFontBitmaps_getFontMetrics_(void* obj){
	return NBFontBitmaps_getMetrics((STNBFontBitmaps*)obj);
}
STNBFontMetrics NBFontBitmaps_getFontMetricsForSz_(void* obj, const float fontSz){
	return NBFontBitmaps_getMetricsForSz((STNBFontBitmaps*)obj, fontSz);
}
STNBFontShapeMetrics NBFontBitmaps_getFontShapeMetrics_(void* obj, const UI32 shapeId){
	return NBFontBitmaps_getShapeMetrics((STNBFontBitmaps*)obj, shapeId);
}
STNBFontShapeMetrics NBFontBitmaps_getFontShapeMetricsForSz_(void* obj, const UI32 shapeId, const float fontSz){
	return NBFontBitmaps_getShapeMetricsForSz((STNBFontBitmaps*)obj, shapeId, fontSz);
}


