//
//  NBBitmap.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/11/18.
//

#ifndef NBBitmap_h
#define NBBitmap_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBArraySorted.h"
#include "nb/2d/NBColor.h"
#include "nb/2d/NBPoint.h"
#include "nb/2d/NBSize.h"
#include "nb/2d/NBRect.h"
#include "nb/2d/NBAABox.h"
#include "nb/2d/NBMatrix.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef enum ENNBBitmapColor_ {
		ENNBBitmapColor_undef = 0,
		ENNBBitmapColor_ALPHA8,		//only alpha (8 bits)
		ENNBBitmapColor_GRIS8,		//grayscale (8 bits)
		ENNBBitmapColor_GRISALPHA8,	//grayscale and alpha (8 bits each component)
		ENNBBitmapColor_RGB4,		//RGB (4 bits each component)
		ENNBBitmapColor_RGB8,		//RGB (8 bits each component)
		ENNBBitmapColor_RGBA4,		//RGBA (4 bits each component)
		ENNBBitmapColor_RGBA8,		//RGBA (8 bits each component)
		ENNBBitmapColor_ARGB4,		//ARGB (4 bits each component)
		ENNBBitmapColor_ARGB8,		//ARGB (8 bits each component)
		ENNBBitmapColor_BGRA8,		//BGRA (8 bits each component)
		ENNBBitmapColor_SWF_PIX15,	//
		ENNBBitmapColor_SWF_PIX24,	//reserved+R+G+B
		ENNBBitmapColor_Count
	} ENNBBitmapColor;
	
	typedef struct STNBBitmapProps_ {
		ENNBBitmapColor color;
		STNBSizeI		size;
		SI32			bitsPerPx;
		SI32			bytesPerLine;
	} STNBBitmapProps;
	
	typedef void (*NBBitmapGetPixFunc)(const STNBBitmapProps* props, const BYTE* data, const UI32 x, const UI32 y, STNBColor8* dstColor);
	typedef void (*NBBitmapSetPixFunc)(const STNBBitmapProps* props, BYTE* data, const UI32 x, const UI32 y, const STNBColor8 color);
	
	typedef struct STNBBitmap_ {
		void* opaque;
	} STNBBitmap;
	
	void NBBitmap_init(STNBBitmap* obj);
	void NBBitmap_retain(STNBBitmap* obj);
	void NBBitmap_release(STNBBitmap* obj);
	
	//Get
	
	STNBBitmapProps NBBitmap_propsForBitmap(const UI32 width, const UI32 height, const ENNBBitmapColor color);
	STNBBitmapProps NBBitmap_getProps(const STNBBitmap* obj);
	BYTE* NBBitmap_getData(const STNBBitmap* obj);
	BYTE* NBBitmap_getDataLine(const STNBBitmap* obj, const UI32 iLine);
	UI32 NBBitmap_getPalette(const STNBBitmap* obj, STNBArraySorted* dstColors, const SI32 szLimit); //STNBArraySorted<NBColor8>
	
	//Build
	void NBBitmap_empty(STNBBitmap* obj);
	void NBBitmap_resignToData(STNBBitmap* obj);
	BOOL NBBitmap_swapData(STNBBitmap* obj, STNBBitmap* other);
	BOOL NBBitmap_create(STNBBitmap* obj, const UI32 width, const UI32 height, const ENNBBitmapColor color);
	BOOL NBBitmap_createWithoutData(STNBBitmap* obj, const UI32 width, const UI32 height, const ENNBBitmapColor color);
	BOOL NBBitmap_createAndSet(STNBBitmap* obj, const UI32 width, const UI32 height, const ENNBBitmapColor color, const UI8 bytesValue);
	BOOL NBBitmap_createWithBitmap(STNBBitmap* obj, const ENNBBitmapColor newColor, const STNBBitmap* src, const STNBColor8 color);
	BOOL NBBitmap_createWithBitmapData(STNBBitmap* obj, const ENNBBitmapColor newColor, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBColor8 color);
	BOOL NBBitmap_createWithBitmapRotatedRight90(STNBBitmap* obj, const ENNBBitmapColor newColor, const STNBBitmap* src, const STNBColor8 color, const UI32 timesRotated90);
	BOOL NBBitmap_createWithBitmapDataRotatedRight90(STNBBitmap* obj, const ENNBBitmapColor newColor, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBColor8 color, const UI32 timesRotated90);
	
	//Pixel Funcs
	NBBitmapGetPixFunc NBBitmap_getGetPixFunc(const ENNBBitmapColor color);
	NBBitmapSetPixFunc NBBitmap_getSetPixFunc(const ENNBBitmapColor color);
	
	//Pixels
	void NBBitmap_getPixel(const STNBBitmap* obj, const UI32 x, const UI32 y, STNBColor8* dstColor);
	void NBBitmap_getPixelByCursor(const ENNBBitmapColor color, const UI8* lineStart, UI16* lineBytePos, UI8* byteBitPos, STNBColor8* dstColor); //ToDo: remove
	
	//Area's color
	//ToDo: review 'NBBitmap_getColorAtBoxOpq'
	//void NBBitmap_getColorAtRect(const STNBBitmap* obj, const STNBRect rect, STNBColor8* dstColor);
	//void NBBitmap_getColorAtBox(const STNBBitmap* obj, const STNBAABox box, STNBColor8* dstColor);
	
	//Actions
	BOOL NBBitmap_setGrayFromAlpha(STNBBitmap* obj);
	BOOL NBBitmap_setAlphaFromGray(STNBBitmap* obj);
	BOOL NBBitmap_clear(STNBBitmap* obj, const UI8 r, const UI8 g, const UI8 b, const UI8 a);
	BOOL NBBitmap_clearWithColor(STNBBitmap* obj, const STNBColor8 c);
	BOOL NBBitmap_clearArea(STNBBitmap* obj, const STNBColor8 c, const SI32 x, const SI32 y, const SI32 width, const SI32 height);
	BOOL NBBitmap_posterize(STNBBitmap* obj, const UI8 tonesDivider);
	
	//Paste of bitmap (lowest calculations)
	BOOL NBBitmap_pasteBitmap(STNBBitmap* obj, const STNBPointI pos, const STNBBitmap* src, const STNBColor8 color);
	BOOL NBBitmap_pasteBitmapRect(STNBBitmap* obj, const STNBPointI pos, const STNBBitmap* src, const STNBRectI srcRect, const STNBColor8 color);
	BOOL NBBitmap_pasteBitmapRectWithMask(STNBBitmap* obj, const STNBPointI pos, const STNBBitmap* src, const STNBRectI srcRect, const STNBBitmap* msk, const STNBPointI mskLeftTop, const STNBColor8 color);
	BOOL NBBitmap_pasteBitmapData(STNBBitmap* obj, const STNBPointI pos, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBColor8 color);
	BOOL NBBitmap_pasteBitmapDataRect(STNBBitmap* obj, const STNBPointI pos, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBRectI srcRect, const STNBColor8 color);
	BOOL NBBitmap_pasteBitmapDataRectWithMaskData(STNBBitmap* obj, const STNBPointI pos, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBRectI srcRect, const STNBBitmapProps mskProps, const BYTE* mskData, const STNBPointI mskLeftTop, const STNBColor8 color);
	
	//Paste of bitmap scaled (low calculation)
	BOOL NBBitmap_pasteBitmapScaledRect(STNBBitmap* obj, const STNBRectI dstRect, const STNBBitmap* src, const STNBRectI srcRect, const STNBColor8 color);
	BOOL NBBitmap_pasteBitmapScaledDataRect(STNBBitmap* obj, const STNBRectI dstRect, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBRectI srcRect, const STNBColor8 color);

	//Paste of bitmap scaled (high calculation)
	BOOL NBBitmap_drawBitmapSmoothScaledRect(STNBBitmap* obj, const STNBRectI dstRect, const STNBBitmap* src, const STNBRectI srcRect, const STNBColor8 color);
	BOOL NBBitmap_drawBitmapSmoothScaledDataRect(STNBBitmap* obj, const STNBRectI dstRect, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBRectI srcRect, const STNBColor8 color);
	
	//Paste of bitmap 90-rotated (low calculation)
	BOOL NBBitmap_pasteBitmapRotatedRight90(STNBBitmap* obj, const STNBPointI pos, const STNBBitmap* src, const STNBColor8 color, const UI32 timesRotated90);
	BOOL NBBitmap_pasteBitmapRectRotatedRight90(STNBBitmap* obj, const STNBPointI pos, const STNBBitmap* src, const STNBRectI srcRect, const STNBColor8 color, const UI32 timesRotated90);
	BOOL NBBitmap_pasteBitmapDataRotatedRight90(STNBBitmap* obj, const STNBPointI pos, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBColor8 color, const UI32 timesRotated90);
	BOOL NBBitmap_pasteBitmapDataRectRotatedRight90(STNBBitmap* obj, const STNBPointI pos, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBRectI srcRect, const STNBColor8 color, const UI32 timesRotated90);
	
	//Draw color
	BOOL NBBitmap_drawPixel(STNBBitmap* obj, const STNBPointI pos, const STNBColor8 color);
	BOOL NBBitmap_drawRect(STNBBitmap* obj, const STNBRect rect, const STNBColor8 color);
	
	//Draw bitmap (low calculations)
	BOOL NBBitmap_drawBitmap(STNBBitmap* obj, const STNBBitmap* src, const STNBPoint posCenter, const float rotRad, const STNBSize scaleRel, const STNBColor8 color);
	BOOL NBBitmap_drawBitmapData(STNBBitmap* obj, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBPoint posCenter, const float rotRad, const STNBSize scaleRel, const STNBColor8 color);
	BOOL NBBitmap_drawBitmapRect(STNBBitmap* obj, const STNBBitmap* src, const STNBRect srcRect, const STNBPoint posCenter, const float rotRad, const STNBSize scaleRel, const STNBColor8 color);
	BOOL NBBitmap_drawBitmapDataRect(STNBBitmap* obj, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBRect srcRect, const STNBPoint posCenter, const float rotRad, const STNBSize scaleRel, const STNBColor8 color);
	BOOL NBBitmap_drawBitmapRectWithMatrix(STNBBitmap* obj, const STNBBitmap* src, const STNBRect srcRect, const STNBMatrix matrix, const STNBColor8 color);
	BOOL NBBitmap_drawBitmapDataRectWithMatrix(STNBBitmap* obj, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBRect srcRect, const STNBMatrix matrix, const STNBColor8 color);
	//Draw bitmap from shape
	BOOL NBBitmap_drawBitmapWithShape(STNBBitmap* obj, const STNBRectI dstArea, const STNBBitmap* src, const STNBPoint startXstartY, const STNBPoint startXendY, const STNBPoint endXendY, const STNBPoint endXstartY);
	BOOL NBBitmap_drawBitmapWithShapeData(STNBBitmap* obj, const STNBRectI dstArea, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBPoint startXstartY, const STNBPoint startXendY, const STNBPoint endXendY, const STNBPoint endXstartY);
	
	//Draw bitmap smooth (higher calculations)
	BOOL NBBitmap_drawBitmapSmooth(STNBBitmap* obj, const STNBBitmap* src, const STNBPoint posCenter, const float rotRad, const STNBSize scaleRel, const STNBColor8 color);
	BOOL NBBitmap_drawBitmapSmoothRect(STNBBitmap* obj, const STNBBitmap* src, const STNBRect srcRect, const STNBPoint posCenter, const float rotRad, const STNBSize scaleRel, const STNBColor8 color);
	BOOL NBBitmap_drawBitmapSmoothRectWithMatrix(STNBBitmap* obj, const STNBBitmap* src, const STNBRect srcRect, const STNBMatrix matrix, const STNBColor8 color);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBBitmap_h */
