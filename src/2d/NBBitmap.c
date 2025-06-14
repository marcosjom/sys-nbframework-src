//
//  NBBitmap.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/11/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/2d/NBBitmap.h"
//
#include "nb/core/NBMemory.h"

typedef struct STNBBitmapOpq_ {
	STNBBitmapProps props;
	BYTE*			data;
	UI32			retainCount;
} STNBBitmapOpq;

//Color functions pointers (optimizations)

typedef struct STNBBitmapColorDesc_ {
	UI8				bitsPerPx;
	const char*		name;
	NBBitmapGetPixFunc	getPixFunc;
	NBBitmapSetPixFunc	setPixFunc;
} STNBBitmapColorDesc;

void NBBitmap_getPixelAlpha8_(const STNBBitmapProps* props, const BYTE* data, const UI32 x, const UI32 y, STNBColor8* dstColor);
void NBBitmap_getPixelGray8_(const STNBBitmapProps* props, const BYTE* data, const UI32 x, const UI32 y, STNBColor8* dstColor);
void NBBitmap_getPixelGrayAlpha8_(const STNBBitmapProps* props, const BYTE* data, const UI32 x, const UI32 y, STNBColor8* dstColor);
void NBBitmap_getPixelRGB8_(const STNBBitmapProps* props, const BYTE* data, const UI32 x, const UI32 y, STNBColor8* dstColor);
void NBBitmap_getPixelRGBA8_(const STNBBitmapProps* props, const BYTE* data, const UI32 x, const UI32 y, STNBColor8* dstColor);
void NBBitmap_getPixelARGB8_(const STNBBitmapProps* props, const BYTE* data, const UI32 x, const UI32 y, STNBColor8* dstColor);
void NBBitmap_getPixelBGRA8_(const STNBBitmapProps* props, const BYTE* data, const UI32 x, const UI32 y, STNBColor8* dstColor);

void NBBitmap_setPixelAlpha8_(const STNBBitmapProps* props, BYTE* data, const UI32 x, const UI32 y, const STNBColor8 color);
void NBBitmap_setPixelGray8_(const STNBBitmapProps* props, BYTE* data, const UI32 x, const UI32 y, const STNBColor8 color);
void NBBitmap_setPixelGrayAlpha8_(const STNBBitmapProps* props, BYTE* data, const UI32 x, const UI32 y, const STNBColor8 color);
void NBBitmap_setPixelRGB8_(const STNBBitmapProps* props, BYTE* data, const UI32 x, const UI32 y, const STNBColor8 color);
void NBBitmap_setPixelRGBA8_(const STNBBitmapProps* props, BYTE* data, const UI32 x, const UI32 y, const STNBColor8 color);
void NBBitmap_setPixelARGB8_(const STNBBitmapProps* props, BYTE* data, const UI32 x, const UI32 y, const STNBColor8 color);
void NBBitmap_setPixelBGRA8_(const STNBBitmapProps* props, BYTE* data, const UI32 x, const UI32 y, const STNBColor8 color);
	
static const STNBBitmapColorDesc NBBitmap_colorMap[] = {
	{0, "undef", NULL, NULL},
	{8, "ALPHA8", NBBitmap_getPixelAlpha8_, NBBitmap_setPixelAlpha8_},
	{8, "GRIS8", NBBitmap_getPixelGray8_, NBBitmap_setPixelGray8_},
	{16, "GRISALPHA8", NBBitmap_getPixelGrayAlpha8_, NBBitmap_setPixelGrayAlpha8_},
	{12, "RGB4", NULL, NULL},
	{24, "RGB8", NBBitmap_getPixelRGB8_, NBBitmap_setPixelRGB8_},
	{16, "RGBA4", NULL, NULL},
	{32, "RGBA8", NBBitmap_getPixelRGBA8_, NBBitmap_setPixelRGBA8_},
	{16, "ARGB4", NULL, NULL},
	{32, "ARGB8", NBBitmap_getPixelARGB8_, NBBitmap_setPixelARGB8_},
	{32, "BGRA8", NBBitmap_getPixelBGRA8_, NBBitmap_setPixelBGRA8_},
	{16, "SWF_PIX15", NULL, NULL},
	{32, "SWF_PIX24", NULL, NULL},
};

void NBBitmap_destroyBuffer(STNBBitmapOpq* opq);
BOOL NBBitmap_createBuffer(STNBBitmapOpq* opq, const UI32 width, const UI32 height, const ENNBBitmapColor color);

void NBBitmap_init(STNBBitmap* obj){
	STNBBitmapOpq* opq	= obj->opaque = NBMemory_allocType(STNBBitmapOpq);
	NBMemory_setZeroSt(*opq, STNBBitmapOpq);
	NBMemory_set(&opq->props, 0, sizeof(opq->props));
	opq->data			= NULL;
	opq->retainCount	= 1;
	//Validation
	NBASSERT((sizeof(NBBitmap_colorMap) / sizeof(NBBitmap_colorMap[0])) == ENNBBitmapColor_Count)
}

void NBBitmap_retain(STNBBitmap* obj){
	STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount++;
}

void NBBitmap_release(STNBBitmap* obj){
	STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount--;
	if(opq->retainCount == 0){
		NBMemory_set(&opq->props, 0, sizeof(opq->props));
		if(opq->data != NULL){
			NBMemory_free(opq->data);
			opq->data = NULL;
		}
		NBMemory_free(obj->opaque);
		obj->opaque = NULL;
	}
}

//Create/destroy bitmap
void NBBitmap_destroyBuffer(STNBBitmapOpq* opq){
	NBMemory_setZeroSt(opq->props, STNBBitmapProps);
	if(opq->data != NULL){
		NBMemory_free(opq->data);
		opq->data = NULL;
	}
}

BOOL NBBitmap_createBuffer(STNBBitmapOpq* opq, const UI32 width, const UI32 height, const ENNBBitmapColor color){
	BOOL r = FALSE;
	if(width > 0 && height > 0){
		const STNBBitmapProps props = NBBitmap_propsForBitmap(width, height, color);
		if(props.bitsPerPx > 0){
			const UI64 totalBytes = (props.bytesPerLine * props.size.height); NBASSERT(totalBytes > 0)
			UI8* data = NBMemory_alloc(totalBytes);
			if(data != NULL){
				NBBitmap_destroyBuffer(opq);
				NBASSERT(opq->data == NULL);
				opq->props	= props;
				opq->data	= data;
				r = TRUE;
			}
		}
	}
	return r;
}
//Get

STNBBitmapProps NBBitmap_propsForBitmap(const UI32 width, const UI32 height, const ENNBBitmapColor color){
	STNBBitmapProps r;
	if(color >= (sizeof(NBBitmap_colorMap) / sizeof(NBBitmap_colorMap[0]))){
		NBMemory_setZeroSt(r, STNBBitmapProps);
	} else {
		const STNBBitmapColorDesc* d = &NBBitmap_colorMap[color];
		if(d->bitsPerPx == 0){
			NBMemory_setZeroSt(r, STNBBitmapProps);
		} else {
			r.bitsPerPx		= d->bitsPerPx;
			{
			const UI32 bitsPerLine = (width * d->bitsPerPx);
				//align to byte
				r.bytesPerLine	= (bitsPerLine / 8) + ((bitsPerLine % 8) != 0 ? 1 : 0);
				//align to word (4 bytes)
				while((r.bytesPerLine % 4) != 0) r.bytesPerLine++;
			}
			r.color			= color;
			r.size.width	= width;
			r.size.height	= height;
		}
	}
	return r;
}

STNBBitmapProps NBBitmap_getProps(const STNBBitmap* obj){
	STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	return opq->props;
}

BYTE* NBBitmap_getData(const STNBBitmap* obj){
	STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	return opq->data;
}

BYTE* NBBitmap_getDataLine(const STNBBitmap* obj, const UI32 iLine){
	BYTE* r = NULL;
	STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	if(opq->data != NULL && opq->props.bytesPerLine > 0 && iLine < opq->props.size.height){
		r = &opq->data[opq->props.bytesPerLine * iLine];
	}
	return r;
}

UI32 NBBitmap_getPalette(const STNBBitmap* obj, STNBArraySorted* dstColors, const SI32 szLimit){ //STNBArraySorted<NBColor8>
	UI32 r = 0;
	STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	if(dstColors != NULL && opq->data != NULL){
		NBArraySorted_empty(dstColors);
		if(szLimit != 0){
			//Analyze all pixels
			UI8* rowFirstByte;
			UI16 curByteAtRow;
			UI8 curBitAtByte;
			SI32 iRow, iCol;
			STNBColor8 c; c.a = 255;
			if(opq->props.color == ENNBBitmapColor_GRIS8){
				for(iRow = 0; iRow < opq->props.size.height && dstColors->use < szLimit; iRow++){
					rowFirstByte	= &(opq->data[iRow *  opq->props.bytesPerLine]);
					curByteAtRow	= 0;
					curBitAtByte	= 0;
					for(iCol = 0; iCol < opq->props.size.width; iCol++){
						c.r = c.g = c.b = rowFirstByte[curByteAtRow++];
						if(NBArraySorted_indexOfValue(dstColors, c) == -1){
							NBArraySorted_addValue(dstColors, c);
							//Limite reached?
							if(szLimit > 0 && dstColors->use >= szLimit){
								break;
							}
						}
					}
					//Limite reached
					if(szLimit > 0 && dstColors->use >= szLimit){
						break;
					}
				}
			} else if(opq->props.color == ENNBBitmapColor_GRISALPHA8){
				for(iRow = 0; iRow < opq->props.size.height; iRow++){
					rowFirstByte	= &(opq->data[iRow *  opq->props.bytesPerLine]);
					curByteAtRow	= 0;
					curBitAtByte	= 0;
					for(iCol = 0; iCol < opq->props.size.width; iCol++){
						c.r = c.g = c.b	= rowFirstByte[curByteAtRow++];
						c.a = rowFirstByte[curByteAtRow++];
						if(NBArraySorted_indexOfValue(dstColors, c) == -1){
							NBArraySorted_addValue(dstColors, c);
							//Limite reached?
							if(szLimit > 0 && dstColors->use >= szLimit){
								break;
							}
						}
					}
					//Limite reached
					if(szLimit > 0 && dstColors->use >= szLimit){
						break;
					}
				}
			} else if(opq->props.color == ENNBBitmapColor_RGB8){
				c.a = 255;
				for(iRow = 0; iRow < opq->props.size.height; iRow++){
					rowFirstByte	= &(opq->data[iRow *  opq->props.bytesPerLine]);
					curByteAtRow	= 0;
					curBitAtByte	= 0;
					for(iCol = 0; iCol < opq->props.size.width; iCol++){
						c.r = rowFirstByte[curByteAtRow++];
						c.g = rowFirstByte[curByteAtRow++];
						c.b = rowFirstByte[curByteAtRow++];
						if(NBArraySorted_indexOfValue(dstColors, c) == -1){
							NBArraySorted_addValue(dstColors, c);
							//Limite reached?
							if(szLimit > 0 && dstColors->use >= szLimit){
								break;
							}
						}
					}
					//Limite reached?
					if(szLimit > 0 && dstColors->use >= szLimit){
						break;
					}
				}
			} else if(opq->props.color == ENNBBitmapColor_RGBA8){
				for(iRow = 0; iRow < opq->props.size.height; iRow++){
					rowFirstByte	= &(opq->data[iRow *  opq->props.bytesPerLine]);
					curByteAtRow	= 0;
					curBitAtByte	= 0;
					for(iCol = 0; iCol < opq->props.size.width; iCol++){
						c.r = rowFirstByte[curByteAtRow++];
						c.g = rowFirstByte[curByteAtRow++];
						c.b = rowFirstByte[curByteAtRow++];
						c.a = rowFirstByte[curByteAtRow++];
						if(NBArraySorted_indexOfValue(dstColors, c) == -1){
							NBArraySorted_addValue(dstColors, c);
							//Limite reached?
							if(szLimit > 0 && dstColors->use >= szLimit){
								break;
							}
						}
					}
					//Limite reached
					if(szLimit > 0 && dstColors->use >= szLimit){
						break;
					}
				}
			} else {
				//--------------------------------
				//-- Warning: usign "privInlineLeerPixel" proved to be expensive.
				//--------------------------------
				for(iRow = 0; iRow < opq->props.size.height; iRow++){
					rowFirstByte	= &(opq->data[iRow *  opq->props.bytesPerLine]);
					curByteAtRow	= 0;
					curBitAtByte	= 0;
					for(iCol = 0; iCol < opq->props.size.width; iCol++){
						NBBitmap_getPixelByCursor(opq->props.color, rowFirstByte, &curByteAtRow, &curBitAtByte, &c);
						if(NBArraySorted_indexOfValue(dstColors, c) == -1){
							NBArraySorted_addValue(dstColors, c);
							//Limite reached?
							if(szLimit > 0 && dstColors->use >= szLimit){
								break;
							}
						}
					}
					//Limite reached
					if(szLimit > 0 && dstColors->use >= szLimit){
						break;
					}
				}
			}
		}
		r = dstColors->use;
	}
	return r;
}

//Build

void NBBitmap_empty(STNBBitmap* obj){
	STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	NBBitmap_destroyBuffer(opq);
}

void NBBitmap_resignToData(STNBBitmap* obj){
	if(obj != NULL){
		STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
		NBMemory_setZeroSt(opq->props, STNBBitmapProps);
		opq->data = NULL;
	}
}

BOOL NBBitmap_swapData(STNBBitmap* obj, STNBBitmap* other){
	BOOL r = FALSE;
	if(obj != NULL && other != NULL && obj != other){
		STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
		STNBBitmapOpq* opq2 = (STNBBitmapOpq*)other->opaque;
		if(opq != NULL && opq2 != NULL){
			//Swap
			const STNBBitmapProps propsTmp = opq->props;
			BYTE* dataTmp	= opq->data;
			opq->props		= opq2->props;
			opq->data		= opq2->data;
			opq2->props		= propsTmp;
			opq2->data		= dataTmp;
		}
		r = TRUE;
	}
	return r;
}

BOOL NBBitmap_create(STNBBitmap* obj, const UI32 width, const UI32 height, const ENNBBitmapColor color){
	STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	return NBBitmap_createBuffer(opq, width, height, color);
}

BOOL NBBitmap_createWithoutData(STNBBitmap* obj, const UI32 width, const UI32 height, const ENNBBitmapColor color){
	STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	NBBitmap_destroyBuffer(opq);
	opq->props = NBBitmap_propsForBitmap(width, height, color);
	return TRUE;
}

BOOL NBBitmap_createAndSet(STNBBitmap* obj, const UI32 width, const UI32 height, const ENNBBitmapColor color, const UI8 bytesValue){
	BOOL r = FALSE;
	STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	//Create buffer
	if(NBBitmap_createBuffer(opq, width, height, color)){
		//Format buffer
		NBMemory_set(opq->data, bytesValue, opq->props.bytesPerLine * opq->props.size.height);
		r = TRUE;
	}
	return r;
}

BOOL NBBitmap_createWithBitmap(STNBBitmap* obj, const ENNBBitmapColor newColor, const STNBBitmap* src, const STNBColor8 color){
	BOOL r = FALSE;
	const STNBBitmapProps srcProps = NBBitmap_getProps(src);
	const BYTE* srcData = NBBitmap_getData(src);
	r = NBBitmap_createWithBitmapData(obj, newColor, srcProps, srcData, color);
	return r;
}

BOOL NBBitmap_createWithBitmapData(STNBBitmap* obj, const ENNBBitmapColor newColor, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBColor8 color){
	BOOL r = FALSE;
	if(!NBBitmap_createAndSet(obj, srcProps.size.width, srcProps.size.height, newColor, 0)){
		//PRINTF_ERROR("NBBitmap, could not create bitmap.\n");
	} else {
		if(!NBBitmap_pasteBitmapData(obj, NBST_P(STNBPointI, 0, 0 ), srcProps, srcData, color)){
			//PRINTF_ERROR("NBBitmap, could not paste bitmap.\n");
		} else {
			//PRINTF_INFO("NBBitmap, bitmap pasted.\n");
			r = TRUE;
		}
	}
	return r;
}

BOOL NBBitmap_createWithBitmapRotatedRight90(STNBBitmap* obj, const ENNBBitmapColor newColor, const STNBBitmap* src, const STNBColor8 color, const UI32 timesRotated90){
	BOOL r = FALSE;
	const STNBBitmapProps srcProps = NBBitmap_getProps(src);
	const BYTE* srcData = NBBitmap_getData(src);
	r = NBBitmap_createWithBitmapDataRotatedRight90(obj, newColor, srcProps, srcData, color, timesRotated90);
	return r;
}

BOOL NBBitmap_createWithBitmapDataRotatedRight90(STNBBitmap* obj, const ENNBBitmapColor newColor, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBColor8 color, const UI32 pTimesRotated90){
	BOOL r = FALSE;
	if(srcProps.size.width > 0 && srcProps.size.height > 0){
		STNBSizeI rotSize;
		STNBPointI rotStart;
		const UI8 timesRotated90 = (pTimesRotated90 % 4);
		switch (timesRotated90) {
			case 0:
				rotSize.width	= srcProps.size.width;
				rotSize.height	= srcProps.size.height;
				rotStart.x		= 0;
				rotStart.y		= 0;
				break;
			case 1: //Rotated once to the right
				rotSize.width	= srcProps.size.height;
				rotSize.height	= srcProps.size.width;
				rotStart.x		= srcProps.size.height - 1;
				rotStart.y		= 0;
				break;
			case 2: //Rotated twice to the right
				rotSize.width	= srcProps.size.width;
				rotSize.height	= srcProps.size.height;
				rotStart.x		= srcProps.size.width - 1;
				rotStart.y		= srcProps.size.height - 1;
				break;
			default: //Rotated three-times to the right
				NBASSERT(timesRotated90 == 3)
				rotSize.width	= srcProps.size.height;
				rotSize.height	= srcProps.size.width;
				rotStart.x		= 0;
				rotStart.y		= srcProps.size.width - 1;
				break;
		}
		if(!NBBitmap_create(obj, rotSize.width, rotSize.height, newColor)){
			//PRINTF_ERROR("NBBitmap, could not create rotated-%d bitmap.\n", (timesRotated90 * 90));
		} else {
			if(!NBBitmap_pasteBitmapDataRotatedRight90(obj, rotStart, srcProps, srcData, color, timesRotated90)){
				//PRINTF_ERROR("NBBitmap, could not paste rotated-%d bitmap.\n", (timesRotated90 * 90));
			} else {
				//PRINTF_INFO("NBBitmap, rotated-%d bitmap pasted.\n", (rot90Right * 90));
				r = TRUE;
			}
		}
	}
	return r;
}

//Pixel Funcs
NBBitmapGetPixFunc NBBitmap_getGetPixFunc(const ENNBBitmapColor color){
	NBBitmapGetPixFunc r = NULL;
	if(color >= 0 && color < ENNBBitmapColor_Count){
		r = NBBitmap_colorMap[color].getPixFunc;
	}
	return r;
}

NBBitmapSetPixFunc NBBitmap_getSetPixFunc(const ENNBBitmapColor color){
	NBBitmapSetPixFunc r = NULL;
	if(color >= 0 && color < ENNBBitmapColor_Count){
		r = NBBitmap_colorMap[color].setPixFunc;
	}
	return r;
}

//Pixels

void NBBitmap_getPixel(const STNBBitmap* obj, const UI32 x, const UI32 y, STNBColor8* dstColor){
	const STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	(*NBBitmap_colorMap[opq->props.color].getPixFunc)(&opq->props, opq->data, x, y, dstColor);
}

//Used by PNG load/save
void NBBitmap_getPixelByCursor(const ENNBBitmapColor color, const UI8* lineStart, UI16* lineBytePos, UI8* byteBitPos, STNBColor8* dstColor){
	//Importante: este metodo esta optimizado para se llamado repetitivamente a leer pixeles del mismo ENNBBitmapColor.
	//Si se manda a leer un pixel de un ENNBBitmapColor y luego otro de diferente ENNBBitmapColor el resultado es impredecible.
	switch(color){
		case ENNBBitmapColor_ALPHA8:
			dstColor->r			= 255;
			dstColor->g			= 255;
			dstColor->b			= 255;
			dstColor->a			= lineStart[(*lineBytePos)++];
			return;
			break;
		case ENNBBitmapColor_GRIS8:
			dstColor->r			= lineStart[(*lineBytePos)++];
			dstColor->g			= dstColor->r;
			dstColor->b			= dstColor->r;
			dstColor->a			= 255;
			return;
			break;
		case ENNBBitmapColor_GRISALPHA8:
			dstColor->r			= lineStart[(*lineBytePos)++];
			dstColor->g			= dstColor->r;
			dstColor->b			= dstColor->r;
			dstColor->a			= lineStart[(*lineBytePos)++];
			return;
			break;
		case ENNBBitmapColor_RGB4:
            {
                const UI8 firstByte	= lineStart[*lineBytePos];
                const UI8 scndByte	= lineStart[*lineBytePos+1];
                if(*byteBitPos == 0){
                    dstColor->r		= ((firstByte>>4) * 17);		//x17 para convertirlo a base 255
                    dstColor->g		= ((firstByte & 0xF) * 17);	//x17 para convertirlo a base 255
                    dstColor->b		= ((scndByte>>4) * 17);		//x17 para convertirlo a base 255
                    dstColor->a		= 255;
                    *lineBytePos += 1;
                    *byteBitPos	= 4;
                } else { //*byteBitPos==4
                    dstColor->r		= ((firstByte & 0xF) * 17);		//x17 para convertirlo a base 255
                    dstColor->g		= ((scndByte >> 4) * 17);	//x17 para convertirlo a base 255
                    dstColor->b		= ((scndByte & 0xF) * 17);		//x17 para convertirlo a base 255
                    dstColor->a		= 255;
                    *lineBytePos += 2;
                    *byteBitPos	= 0;
                }
            }
			return;
			break;
		case ENNBBitmapColor_RGB8:
			dstColor->r			= lineStart[(*lineBytePos)++];
			dstColor->g			= lineStart[(*lineBytePos)++];
			dstColor->b			= lineStart[(*lineBytePos)++];
			dstColor->a			= 255;
			return;
			break;
		case ENNBBitmapColor_RGBA4:
            {
                const UI8 firstByte	= lineStart[(*lineBytePos)++];
                const UI8 scndByte	= lineStart[(*lineBytePos)++];
                dstColor->r			= ((firstByte>>4) * 17);		//x17 para convertirlo a base 255
                dstColor->g			= ((firstByte & 0xF) * 17);	//x17 para convertirlo a base 255
                dstColor->b			= ((scndByte>>4) * 17);		//x17 para convertirlo a base 255
                dstColor->a			= ((scndByte & 0xF) * 17);	//x17 para convertirlo a base 255
            }
			return;
			break;
		case ENNBBitmapColor_RGBA8:
			dstColor->r			= lineStart[(*lineBytePos)++];
			dstColor->g			= lineStart[(*lineBytePos)++];
			dstColor->b			= lineStart[(*lineBytePos)++];
			dstColor->a			= lineStart[(*lineBytePos)++];
			return;
			break;
		case ENNBBitmapColor_ARGB4:
            {
                const UI8 firstByte	= lineStart[(*lineBytePos)++];
                const UI8 scndByte	= lineStart[(*lineBytePos)++];
                dstColor->a			= ((firstByte>>4) * 17);		//x17 para convertirlo a base 255
                dstColor->r			= ((firstByte & 0xF) * 17);	//x17 para convertirlo a base 255
                dstColor->g			= ((scndByte>>4) * 17);		//x17 para convertirlo a base 255
                dstColor->b			= ((scndByte & 0xF) * 17);	//x17 para convertirlo a base 255
            }
			return;
			break;
		case ENNBBitmapColor_ARGB8:
			dstColor->a			= lineStart[(*lineBytePos)++];
			dstColor->r			= lineStart[(*lineBytePos)++];
			dstColor->g			= lineStart[(*lineBytePos)++];
			dstColor->b			= lineStart[(*lineBytePos)++];
			return;
			break;
		case ENNBBitmapColor_BGRA8:
			dstColor->b			= lineStart[(*lineBytePos)++];
			dstColor->g			= lineStart[(*lineBytePos)++];
			dstColor->r			= lineStart[(*lineBytePos)++];
			dstColor->a			= lineStart[(*lineBytePos)++];
			return;
			break;
		case ENNBBitmapColor_SWF_PIX15:
			dstColor->r = 0; dstColor->g = 0; dstColor->b = 0; dstColor->a = 0;
			NBASSERT(FALSE) //ToDo
			break;
		case ENNBBitmapColor_SWF_PIX24:
			dstColor->r = 0; dstColor->g = 0; dstColor->b = 0; dstColor->a = 0;
			NBASSERT(FALSE) //ToDo
			break;
		default:
			dstColor->r = 0; dstColor->g = 0; dstColor->b = 0; dstColor->a = 0;
			NBASSERT(FALSE)
			break;
	}
}

void NBBitmap_getColorAtInternalScaledBoxOpq(const STNBBitmapProps* srcPrps, const BYTE* srcData, const STNBAABoxI scaldBox, const SI32 scale, STNBColor8* dstColor){
	NBASSERT(scaldBox.xMin >= 0 && scaldBox.xMin <= (srcPrps->size.width * scale))
	NBASSERT(scaldBox.yMin >= 0 && scaldBox.yMin <= (srcPrps->size.height * scale))
	NBASSERT(scaldBox.xMax >= 0 && scaldBox.xMax <= (srcPrps->size.width * scale))
	NBASSERT(scaldBox.yMax >= 0 && scaldBox.yMax <= (srcPrps->size.height * scale))
	NBASSERT(scaldBox.xMin <= scaldBox.xMax)
	NBASSERT(scaldBox.yMin <= scaldBox.yMax)
	NBASSERT(scale > 0)
	NBBitmapGetPixFunc getPixFunc = NBBitmap_colorMap[srcPrps->color].getPixFunc;
	STNBColor8 tmpPx; SI32 iX, iY, infX, infY, infPx; //indexes and influences
	STNBColorI pixAccum	= {0, 0, 0, 0};	//Accumulation of components
	SI32 totAccum		= 0;					//Accumulation of distributions
	const SI32 rLeft	= ((scaldBox.xMin / scale) * scale); //Right-left-part
	const SI32 rRight	= ((scaldBox.xMax / scale) * scale); //Right-rounded-part
	const SI32 rTop		= ((scaldBox.yMin / scale) * scale);
	const SI32 rBottm	= ((scaldBox.yMax / scale) * scale); //Right-rounded-part
	SI32 earLeft, earRight, lastRight;
	SI32 earTop, earBottm, lastBottm;
	if(rLeft <= (rRight - scale)){
		earLeft			= ((rRight - scaldBox.xMin) % scale); NBASSERT(earLeft < scale)
		earRight		= scaldBox.xMax - ((scaldBox.xMax / scale) * scale); NBASSERT(earRight < scale)
		lastRight		= rRight - (earRight == 0 ? 1 : 0);
	} else {
		earLeft			= earRight = 0;
		lastRight		= rRight;
	}
	if(rTop <= (rBottm - scale)){
		earTop			= ((rBottm - scaldBox.yMin) % scale); NBASSERT(earTop < scale)
		earBottm		= scaldBox.yMax - ((scaldBox.yMax / scale) * scale); NBASSERT(earBottm < scale)
		lastBottm		= rBottm - (earBottm == 0 ? 1 : 0);
	} else {
		earTop			= earBottm = 0;
		lastBottm		= rBottm;
	}
	NBASSERT(earLeft >= 0 && earLeft < scale && earRight >= 0 && earRight < scale && rLeft <= lastRight)
	NBASSERT(earTop >= 0 && earTop < scale && earBottm >= 0 && earBottm < scale && rTop <= lastBottm)
	for(iX = rLeft; iX <= lastRight; iX += scale){
		infX = (iX == rLeft && earLeft > 0 ? earLeft : (iX == lastRight && earRight > 0) ? earRight : scale);
		for(iY = rTop; iY <= lastBottm; iY += scale){
			infY		= (iY == rTop && earTop > 0 ? earTop : (iY == lastBottm && earBottm > 0) ? earBottm : scale);
			infPx		= (infX * infY); NBASSERT(infPx > 0 && infPx <= (scale * scale))
			(*getPixFunc)(srcPrps, srcData, (iX / scale), (iY / scale), &tmpPx);
			pixAccum.r	+= (tmpPx.r * infPx);
			pixAccum.g	+= (tmpPx.g * infPx);
			pixAccum.b	+= (tmpPx.b * infPx);
			pixAccum.a	+= (tmpPx.a * infPx);
			totAccum	+= infPx;
		}
	}
	//Apply color
	NBASSERT(pixAccum.r >= 0 && pixAccum.r <= (totAccum * 255))
	NBASSERT(pixAccum.g >= 0 && pixAccum.g <= (totAccum * 255))
	NBASSERT(pixAccum.b >= 0 && pixAccum.b <= (totAccum * 255))
	NBASSERT(pixAccum.a >= 0 && pixAccum.a <= (totAccum * 255))
	if(totAccum > 0){
		dstColor->r	= pixAccum.r / totAccum; NBASSERT(dstColor->r >= 0 && dstColor->r <= 255)
		dstColor->g	= pixAccum.g / totAccum; NBASSERT(dstColor->g >= 0 && dstColor->g <= 255)
		dstColor->b	= pixAccum.b / totAccum; NBASSERT(dstColor->b >= 0 && dstColor->b <= 255)
		dstColor->a	= pixAccum.a / totAccum; NBASSERT(dstColor->a >= 0 && dstColor->a <= 255)
	} else {
		dstColor->r	= dstColor->g = dstColor->b = 255;
		dstColor->a	= 0;
	}
}

void NBBitmap_getColorAtRepeatScaledBoxOpq(const STNBBitmapProps* srcPrps, const BYTE* srcData, const STNBAABoxI scaldBox, const STNBAABoxI scaldRepeatBox, const SI32 scale, STNBColor8* dstColor){
	NBASSERT(scaldRepeatBox.xMin >= 0 && scaldRepeatBox.xMax >= 0 && scaldRepeatBox.yMin >= 0 && scaldRepeatBox.yMax >= 0) //Must be an internal area
	NBASSERT(scaldRepeatBox.xMin <= (srcPrps->size.width * scale) && scaldRepeatBox.xMax <= (srcPrps->size.width * scale) && scaldRepeatBox.yMin <= (srcPrps->size.height * scale) && scaldRepeatBox.yMax <= (srcPrps->size.height * scale)) //Must be an internal area
	const SI32 scaldWidth = srcPrps->size.width * scale;
	const SI32 scaldHeight = srcPrps->size.height * scale;
	const SI32 scaldRepeatWidth = (scaldRepeatBox.xMax - scaldRepeatBox.xMin);
	const SI32 scaldRepeatHeight = (scaldRepeatBox.yMax - scaldRepeatBox.yMin);
	const SI32 adjtdX	= (scaldBox.xMin < 0 ? scaldWidth : 0) + (scaldBox.xMin % scaldWidth);
	const SI32 adjtdY	= (scaldBox.yMin < 0 ? scaldHeight : 0) + (scaldBox.yMin % scaldHeight);
	//Repeat box start
	STNBAABoxI scaldBox2;
	scaldBox2.xMax		= adjtdX + (scaldBox.xMax - scaldBox.xMin);
	scaldBox2.xMin		= adjtdX;
	scaldBox2.yMax		= adjtdY + (scaldBox.yMax - scaldBox.yMin);
	scaldBox2.yMin		= adjtdY;
	NBASSERT(scaldBox2.xMin >= 0 && scaldBox2.xMax >= 0 && scaldBox2.yMin >= 0 && scaldBox2.yMax >= 0)
	NBASSERT(scaldBox2.xMin <= scaldRepeatWidth && scaldBox2.yMin <= scaldRepeatHeight)
	//
	STNBColorI acumm = { 0, 0, 0, 0 }; STNBColor8 color;
	SI32 iY = scaldBox2.yMin, nxtY, iX, nxtX, accumCount = 0;
	while(iY < scaldBox2.yMax){
		nxtY	= iY + (scaldRepeatHeight - (iY % scaldRepeatHeight)); NBASSERT(iY <= nxtY)
		if(nxtY > scaldBox2.yMax) nxtY = scaldBox2.yMax;
		iX		= scaldBox2.xMin;
		while(iX < scaldBox2.xMax){
			nxtX	= iX + (scaldRepeatWidth - (iX % scaldRepeatWidth)); NBASSERT(iX <= nxtX)
			if(nxtX > scaldBox2.xMax) nxtX = scaldBox2.xMax;
			const STNBAABoxI scaldInt = {
				scaldRepeatBox.xMin + (iX - (iX * scaldRepeatWidth / scaldRepeatWidth))
				, scaldRepeatBox.xMin + (nxtX - (iX * scaldRepeatWidth / scaldRepeatWidth))
				, scaldRepeatBox.yMin + (iY - (iY * scaldRepeatHeight / scaldRepeatHeight))
				, scaldRepeatBox.yMin + (nxtY - (iY * scaldRepeatHeight / scaldRepeatHeight))
			};
			NBBitmap_getColorAtInternalScaledBoxOpq(srcPrps, srcData, scaldInt, scale, &color);
			acumm.r += color.r;
			acumm.g += color.g;
			acumm.b += color.b;
			acumm.a += color.a;
			accumCount++;
			//Next
			iX = nxtX;
		}
		//Next
		iY = nxtY;
	}
	if(accumCount <= 0){
		NBMemory_setZeroSt(*dstColor, STNBColor8);
	} else {
		dstColor->r = acumm.r / accumCount; NBASSERT(acumm.r >= 0 && (acumm.r / accumCount) <= 255)
		dstColor->g = acumm.g / accumCount; NBASSERT(acumm.g >= 0 && (acumm.g / accumCount) <= 255)
		dstColor->b = acumm.b / accumCount; NBASSERT(acumm.b >= 0 && (acumm.b / accumCount) <= 255)
		dstColor->a = acumm.a / accumCount; NBASSERT(acumm.a >= 0 && (acumm.a / accumCount) <= 255)
	}
}

//ToDo: review 'NBBitmap_getColorAtBoxOpq'
/*void NBBitmap_getColorAtBoxOpq(const STNBBitmapOpq* opq, const STNBAABox box, const SI32 scale, STNBColor8* dstColor){
	const STNBBitmapProps* srcPrps = &opq->props;
	const BYTE* srcData		= opq->data;
	const SI32 scaldWidth	= (srcPrps->size.width * scale);
	const SI32 scaldHeight	= (srcPrps->size.height * scale);
	STNBAABoxI scaldBox		= {
		box.xMin * (float)scale,
		box.xMax * (float)scale,
		box.yMin * (float)scale,
		box.yMax * (float)scale
	};
	//Invert (if necesary)
	if(scaldBox.xMin > scaldBox.xMax){
		const SI32 tmp = scaldBox.xMin;
		scaldBox.xMin = scaldBox.xMax;
		scaldBox.xMax = tmp;
	}
	if(scaldBox.yMin > scaldBox.yMax){
		const SI32 tmp = scaldBox.yMin;
		scaldBox.yMin = scaldBox.yMax;
		scaldBox.yMax = tmp;
	} NBASSERT(scaldBox.xMin <= scaldBox.xMax && scaldBox.yMin <= scaldBox.yMax)
	//Move to positive range
	const SI32 adjtdX	= (scaldBox.xMin < 0 ? scaldWidth : 0) + (scaldBox.xMin % scaldWidth);
	const SI32 adjtdY	= (scaldBox.yMin < 0 ? scaldHeight : 0) + (scaldBox.yMin % scaldHeight);
	scaldBox.xMax		= adjtdX + (scaldBox.xMax - scaldBox.xMin);
	scaldBox.xMin		= adjtdX;
	scaldBox.yMax		= adjtdY + (scaldBox.yMax - scaldBox.yMin);
	scaldBox.yMin		= adjtdY;
	NBASSERT(scaldBox.xMin >= 0 && scaldBox.xMax >= 0 && scaldBox.yMin >= 0 && scaldBox.yMax >= 0)
	if(scaldBox.xMax <= scaldWidth && scaldBox.yMax <= scaldHeight){
		NBBitmap_getColorAtInternalScaledBoxOpq(srcPrps, srcData, scaldBox, 4, dstColor); //4 scale fixed operations
	} else {
		STNBAABoxI scldSrcRepeatBox;
		scldSrcRepeatBox.xMin = 0;
		scldSrcRepeatBox.yMin = 0;
		scldSrcRepeatBox.xMax = srcPrps->size.width * scale;
		scldSrcRepeatBox.yMax = srcPrps->size.height * scale;
		NBBitmap_getColorAtRepeatScaledBoxOpq(srcPrps, srcData, scaldBox, scldSrcRepeatBox, 4, dstColor); //4 scale fixed operations
	}
}*/

//ToDo: review 'NBBitmap_getColorAtBoxOpq'
/*void NBBitmap_getColorAtRect(const STNBBitmap* obj, const STNBRect rect, STNBColor8* dstColor){
	const STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	STNBAABox box = {
		rect.x, (rect.x + rect.width),
		rect.y, (rect.y + rect.height)
	};
	NBBitmap_getColorAtBoxOpq(opq, box, 4, dstColor);
}*/

//ToDo: review 'NBBitmap_getColorAtBoxOpq'
/*void NBBitmap_getColorAtBox(const STNBBitmap* obj, const STNBAABox box, STNBColor8* dstColor){
	const STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	NBBitmap_getColorAtBoxOpq(opq, box, 4, dstColor);
}*/

//Actions

BOOL NBBitmap_setGrayFromAlpha(STNBBitmap* obj){
	BOOL r = FALSE;
	STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	if(opq->props.color == ENNBBitmapColor_ALPHA8){
		opq->props.color = ENNBBitmapColor_GRIS8;
		r = TRUE;
	}
	return r;
}

BOOL NBBitmap_setAlphaFromGray(STNBBitmap* obj){
	BOOL r = FALSE;
	STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	if(opq->props.color == ENNBBitmapColor_GRIS8){
		opq->props.color = ENNBBitmapColor_ALPHA8;
		r = TRUE;
	}
	return r;
}

BOOL NBBitmap_clear(STNBBitmap* obj, const UI8 r, const UI8 g, const UI8 b, const UI8 a){
	STNBColor8 c; c.r = r; c.g = g; c.b = b; c.a = a;
	return NBBitmap_clearWithColor(obj, c);
}

BOOL NBBitmap_clearWithColor(STNBBitmap* obj, const STNBColor8 c){
	BOOL r = FALSE;
	STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	if(opq->data != NULL){
		switch (opq->props.color) {
			case ENNBBitmapColor_RGBA8:
				{
					STNBColor8* pixs = (STNBColor8*)opq->data;
					STNBColor8* pixAfterLast = pixs + (opq->props.size.width * opq->props.size.height);
					while(pixs < pixAfterLast){
						*pixs = c; pixs++;
					}
					r = TRUE;
				}
				break;
			case ENNBBitmapColor_ARGB8:
				{
					STNBColor8* pixs = (STNBColor8*)opq->data;
					STNBColor8* pixAfterLast = pixs + (opq->props.size.width * opq->props.size.height);
					STNBColor8 cc; cc.r = c.a; cc.g = c.r; cc.b = c.g; cc.a = c.b;
					while(pixs < pixAfterLast){
						*pixs = cc; pixs++;
					}
					r = TRUE;
				}
				break;
			case ENNBBitmapColor_BGRA8:
				{
					STNBColor8* pixs = (STNBColor8*)opq->data;
					STNBColor8* pixAfterLast = pixs + (opq->props.size.width * opq->props.size.height);
					STNBColor8 cc; cc.r = c.b; cc.g = c.g; cc.b = c.r; cc.a = c.a;
					while(pixs < pixAfterLast){
						*pixs = cc; pixs++;
					}
					r = TRUE;
				}
				break;
			default:
				break;
		}
	}
	return r;
}

BOOL NBBitmap_clearArea(STNBBitmap* obj, const STNBColor8 c, const SI32 x, const SI32 y, const SI32 width, const SI32 height){
	BOOL r = FALSE;
	STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	if(opq->data != NULL){
		switch (opq->props.color) {
			case ENNBBitmapColor_RGBA8:
				{
					SI32 colAftrLst = x + width, colAftrLst2 = opq->props.size.width;
					const SI32 iCol = (x >= 0 ? x : 0);
					const SI32 countCols = (colAftrLst < colAftrLst2 ? colAftrLst : colAftrLst2) - iCol;
					SI32 iFil, filMaxSig = y + height, filMaxSig2 = opq->props.size.height;
					for(iFil = (y >= 0 ? y : 0); iFil < filMaxSig && iFil < filMaxSig2; iFil++){
						STNBColor8* pix = (STNBColor8*)&(opq->data[(iFil * opq->props.bytesPerLine) + (iCol * (opq->props.bitsPerPx / 8))]);
						const STNBColor8* pixAftrLst = &pix[countCols];
						while(pix < pixAftrLst){
							*pix = c;
							pix++;
						}
					}
					r = TRUE;
				}
				break;
			case ENNBBitmapColor_ARGB8:
				{
					SI32 colAftrLst = x + width, colAftrLst2 = opq->props.size.width;
					const SI32 iCol = (x >= 0 ? x : 0);
					const SI32 countCols = (colAftrLst < colAftrLst2 ? colAftrLst : colAftrLst2) - iCol;
					SI32 iFil, filMaxSig = y + height, filMaxSig2 = opq->props.size.height;
					STNBColor8 cc; cc.r = c.a; cc.g = c.r; cc.b = c.g; cc.a = c.b;
					for(iFil = (y >= 0 ? y : 0); iFil < filMaxSig && iFil < filMaxSig2; iFil++){
						STNBColor8* pix = (STNBColor8*)&(opq->data[(iFil * opq->props.bytesPerLine) + (iCol * (opq->props.bitsPerPx / 8))]);
						const STNBColor8* pixAftrLst = &pix[countCols];
						while(pix < pixAftrLst){
							*pix = cc;
							pix++;
						}
					}
					r = TRUE;
				}
				break;
			case ENNBBitmapColor_BGRA8:
				{
					SI32 colAftrLst = x + width, colAftrLst2 = opq->props.size.width;
					const SI32 iCol = (x >= 0 ? x : 0);
					const SI32 countCols = (colAftrLst < colAftrLst2 ? colAftrLst : colAftrLst2) - iCol;
					SI32 iFil, filMaxSig = y + height, filMaxSig2 = opq->props.size.height;
					STNBColor8 cc; cc.r = c.b; cc.g = c.g; cc.b = c.r; cc.a = c.a;
					for(iFil = (y >= 0 ? y : 0); iFil < filMaxSig && iFil < filMaxSig2; iFil++){
						STNBColor8* pix = (STNBColor8*)&(opq->data[(iFil * opq->props.bytesPerLine) + (iCol * (opq->props.bitsPerPx / 8))]);
						const STNBColor8* pixAftrLst = &pix[countCols];
						while(pix < pixAftrLst){
							*pix = cc;
							pix++;
						}
					}
					r = TRUE;
				}
				break;
			default:
				break;
		}
	}
	return r;
}

BOOL NBBitmap_posterize(STNBBitmap* obj, const UI8 tonesDivider){
	BOOL r = FALSE;
	STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	if(opq->data != NULL && tonesDivider > 1 && tonesDivider <= 128){
		switch (opq->props.color) {
			case ENNBBitmapColor_ALPHA8:
			case ENNBBitmapColor_GRIS8:
			case ENNBBitmapColor_GRISALPHA8:
			case ENNBBitmapColor_RGB8:
			case ENNBBitmapColor_RGBA8:
			case ENNBBitmapColor_ARGB8:
			case ENNBBitmapColor_BGRA8:
				{
					//Apply posterization:
					//Move values to closest divider.
					const SI32 div32	= (SI32)tonesDivider;
					const UI8 divMid	= tonesDivider / 2;
					const UI8* ptrStop	= opq->data + (opq->props.bytesPerLine * opq->props.size.height);
					UI8* ptr = opq->data;
					while(ptr < ptrStop){
						//NBASSERT((((((SI32)*ptr + div32) / div32) * div32) - 1) >= 0 && (((((SI32)*ptr + div32) / div32) * div32) - 1) < 256)
						//*ptr = (((((SI32)*ptr + div32) / div32) * div32) - 1);
						if((*ptr % tonesDivider) < divMid){
							//align to left
							NBASSERT((((SI32)*ptr / (SI32)tonesDivider) * (SI32)tonesDivider) >= 0 && (((SI32)*ptr / (SI32)tonesDivider) * (SI32)tonesDivider) < 256)
							*ptr = ((*ptr / tonesDivider) * tonesDivider);
						} else {
							//align to right
							NBASSERT((((((SI32)*ptr + div32) / div32) * div32) - 1) >= 0 && (((((SI32)*ptr + div32) / div32) * div32) - 1) < 256)
							*ptr = (UI8)(((((SI32)*ptr + div32) / div32) * div32) - 1);
						}
						ptr++;
					}
				}
				r = TRUE;
				break;
			default:
				break;
		}
	}
	return r;
}

//Paste pf bitmap (lower calculations)

BOOL NBBitmap_pasteBitmap(STNBBitmap* obj, const STNBPointI pos, const STNBBitmap* src, const STNBColor8 color){
	STNBBitmapOpq* opq2 = (STNBBitmapOpq*)src->opaque;
	return NBBitmap_pasteBitmapDataRect(obj, pos, opq2->props, opq2->data, NBST_P(STNBRectI, 0, 0, opq2->props.size.width, opq2->props.size.height), color);
}

BOOL NBBitmap_pasteBitmapRect(STNBBitmap* obj, const STNBPointI pos, const STNBBitmap* src, const STNBRectI srcRect, const STNBColor8 color){
	BOOL r = FALSE;
	if(src != NULL){
		STNBBitmapOpq* opq2 = (STNBBitmapOpq*)src->opaque;
		r = NBBitmap_pasteBitmapDataRect(obj, pos, opq2->props, opq2->data, srcRect, color);
	}
	return r;
}

BOOL NBBitmap_pasteBitmapRectWithMask(STNBBitmap* obj, const STNBPointI pos, const STNBBitmap* src, const STNBRectI srcRect, const STNBBitmap* msk, const STNBPointI mskLeftTop, const STNBColor8 color){
	BOOL r = FALSE;
	if(src != NULL && msk != NULL){
		STNBBitmapOpq* opq2 = (STNBBitmapOpq*)src->opaque;
		STNBBitmapOpq* opq3 = (STNBBitmapOpq*)msk->opaque;
		r = NBBitmap_pasteBitmapDataRectWithMaskData(obj, pos, opq2->props, opq2->data, srcRect, opq3->props, opq3->data, mskLeftTop, color);
	}
	return r;
}

BOOL NBBitmap_pasteBitmapData(STNBBitmap* obj, const STNBPointI pos, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBColor8 color){
	return NBBitmap_pasteBitmapDataRect(obj, pos, srcProps, srcData, NBST_P(STNBRectI, 0, 0, srcProps.size.width, srcProps.size.height), color);
}

BOOL NBBitmap_pasteBitmapDataRect(STNBBitmap* obj, const STNBPointI pPos, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBRectI pSrcRect, const STNBColor8 color){
	BOOL r = FALSE;
	{
		STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
		if(opq->data != NULL && srcData != NULL && pSrcRect.width != 0 && pSrcRect.height != 0){
			if(color.a > 0){
				const STNBBitmapProps* dstPrps = &opq->props;
				const STNBBitmapProps* srcPrps = &srcProps;
				{
					STNBPointI pos = pPos;
					STNBRectI srcRect = pSrcRect;
					//Validate negative rect
					{
						if(srcRect.width < 0){
							srcRect.x		+= srcRect.width;
							srcRect.width	= -srcRect.width;
						}
						if(srcRect.height < 0){
							srcRect.y		+= srcRect.height;
							srcRect.height	= -srcRect.height;
						}
					} NBASSERT(srcRect.width >= 0 && srcRect.height >= 0)
					//Validate negative start
					{
						if(pos.x < 0){
							srcRect.x		-= pos.x;
							srcRect.width	+= pos.x;
							pos.x			= 0;
						}
						if(pos.y < 0){
							srcRect.y		-= pos.y;
							srcRect.height	+= pos.y;
							pos.y			= 0;
						}
					} NBASSERT(srcRect.x >= 0 && srcRect.y >= 0 && srcRect.width >= 0 && srcRect.height >= 0)
					//Validate against dst-rect
					{
						if((pos.x + srcRect.width) > dstPrps->size.width){
							srcRect.width = (dstPrps->size.width - pos.x);
						}
						if((pos.y + srcRect.height) > dstPrps->size.height){
							srcRect.height = (dstPrps->size.height - pos.y);
						}
					}
					//Copy content
					if(srcRect.width <= 0 || srcRect.height <= 0){
						//PRINTF_INFO("Bitmaqp omited.\n");
					} else {
						//Validate, inside-dstRange
						NBASSERT(pos.x >= 0 && pos.x < dstPrps->size.width)
						NBASSERT(pos.y >= 0 && pos.y < dstPrps->size.height)
						NBASSERT((pos.x + srcRect.width) > 0 && (pos.x + srcRect.width) <= dstPrps->size.width)
						NBASSERT((pos.y + srcRect.height) > 0 && (pos.y + srcRect.height) <= dstPrps->size.height)
						//
						const BOOL keepColor		= (color.r == 255 && color.g == 255 && color.b == 255 && color.a == 255);
						const BOOL isSameFormat		= (opq->props.color == srcProps.color && opq->props.bytesPerLine == srcProps.bytesPerLine); // && opq->props.bitsPerPx == srcProps.bitsPerPx
						const BOOL canDirectCopy	= (keepColor && isSameFormat && (opq->props.bitsPerPx % 8) == 0);
						if(canDirectCopy){
							const BOOL isFullCopy	= (pos.x == 0 && pos.y == 0 && srcRect.x == 0 && srcRect.y == 0 && srcPrps->bytesPerLine == dstPrps->bytesPerLine && srcRect.width == dstPrps->size.width && srcRect.height == dstPrps->size.height && srcPrps->size.width == dstPrps->size.width && srcPrps->size.height == dstPrps->size.height);
							if(isFullCopy){
								//Full-data-copy (optimization)
								NBMemory_copy(opq->data, srcData, (srcPrps->bytesPerLine * srcPrps->size.height));
							} else {
								//Copy partial lines
								const BYTE* lineOrg; BYTE* lineDst;
								const BYTE* pxsOrg; BYTE* pxsDst;
								SI32 y; for(y = 0; y < srcRect.height; y++){
									NBASSERT((srcRect.y + y) >= 0 && (srcRect.y + y) < srcPrps->size.height)
									NBASSERT((pos.y + y) >= 0 && (pos.y + y) < dstPrps->size.height)
									lineOrg = &srcData[srcPrps->bytesPerLine * (srcRect.y + y)];
									lineDst = &opq->data[dstPrps->bytesPerLine * (pos.y + y)];
									//Copy line
									{
										NBASSERT(srcRect.x >= 0 && srcRect.x < srcPrps->size.width)
										NBASSERT((srcRect.x + srcRect.width) >= 0 && (srcRect.x + srcRect.width) < srcPrps->size.width)
										//
										NBASSERT(pos.x >= 0 && pos.x < dstPrps->size.width)
										NBASSERT((pos.x + srcRect.width) >= 0 && (pos.x + srcRect.width) < dstPrps->size.width)
										//
										pxsOrg	= &lineOrg[srcRect.x * (srcPrps->bitsPerPx / 8)];
										pxsDst	= &lineDst[pos.x * (dstPrps->bitsPerPx / 8)];
										//
										NBMemory_copy(pxsDst, pxsOrg, srcRect.width * (srcPrps->bitsPerPx / 8));
									}
								}
							}
							
						} else {
							//Per-pixel copy
							NBBitmapGetPixFunc getSrcPixFunc = NBBitmap_colorMap[srcPrps->color].getPixFunc;
							NBBitmapGetPixFunc getDstPixFunc = NBBitmap_colorMap[dstPrps->color].getPixFunc;
							NBBitmapSetPixFunc setDstPixFunc = NBBitmap_colorMap[dstPrps->color].setPixFunc;
							STNBPointI dstPosI, srcPosI, srcPosI2;
							STNBColor8 srcPx, dstPx; STNBColorI vPx;
							SI32 iY, iX, srcAlpha, srcAlphaInv;
							for(iY = 0; iY < srcRect.height; iY++){
								for(iX = 0; iX < srcRect.width; iX++){
									dstPosI.x	= pos.x + iX; NBASSERT(dstPosI.x >= 0 && dstPosI.x < dstPrps->size.width)
									dstPosI.y	= pos.y + iY; NBASSERT(dstPosI.y >= 0 && dstPosI.y < dstPrps->size.height)
									srcPosI.x	= srcRect.x + iX;
									srcPosI.y	= srcRect.y + iY;
									srcPosI2.x	= (srcPosI.x < 0 ? srcPrps->size.width : 0) + (srcPosI.x % srcPrps->size.width);
									srcPosI2.y	= (srcPosI.y < 0 ? srcPrps->size.height : 0) + (srcPosI.y % srcPrps->size.height);
									//Get colors
									(*getSrcPixFunc)(srcPrps, srcData, srcPosI2.x, srcPosI2.y, &srcPx);
									(*getDstPixFunc)(&opq->props, opq->data, dstPosI.x, dstPosI.y, &dstPx);
									//Calculate
									srcAlpha 	= (srcPx.a * color.a) / 255;
									srcAlphaInv	= (255 - srcAlpha);
									vPx.r		= ((srcPx.r * color.r * srcAlpha) / 65025) + ((dstPx.r * srcAlphaInv) / 255);	dstPx.r = (vPx.r > 255 ? 255 : vPx.r);
									vPx.g		= ((srcPx.g * color.g * srcAlpha) / 65025) + ((dstPx.g * srcAlphaInv) / 255);	dstPx.g = (vPx.g > 255 ? 255 : vPx.g);
									vPx.b		= ((srcPx.b * color.b * srcAlpha) / 65025) + ((dstPx.b * srcAlphaInv) / 255);	dstPx.b = (vPx.b > 255 ? 255 : vPx.b);
									vPx.a		= dstPx.a + ((srcAlpha * (255 - dstPx.a)) / 255);								dstPx.a = (vPx.a > 255 ? 255 : vPx.a);
									//vPx.a		= ((srcAlpha * srcAlpha) / 255) + ((dstPx.a * srcAlphaInv) / 255);				dstPx.a = (vPx.a > 255 ? 255 : vPx.a);
									//Assign
									(*setDstPixFunc)(&opq->props, opq->data, dstPosI.x, dstPosI.y, dstPx);
								}
							}
						}
					}
				}
			}
			r = TRUE;
		}
	}
	return r;
}

BOOL NBBitmap_pasteBitmapDataRectWithMaskData(STNBBitmap* obj, const STNBPointI pPos, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBRectI pSrcRect, const STNBBitmapProps mskProps, const BYTE* mskData, const STNBPointI mskLeftTop, const STNBColor8 color){
	BOOL r = FALSE;
	{
		STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
		if(opq->data != NULL && srcData != NULL && mskData != NULL && pSrcRect.width != 0 && pSrcRect.height != 0 && color.a > 0){
			const STNBBitmapProps* dstPrps = &opq->props;
			const STNBBitmapProps* srcPrps = &srcProps;
			const STNBBitmapProps* mskPrps = &mskProps;
			{
				STNBPointI pos = pPos;
				STNBRectI srcRect = pSrcRect;
				//Validate negative rect
				{
					if(srcRect.width < 0){
						srcRect.x		+= srcRect.width;
						srcRect.width	= -srcRect.width;
					}
					if(srcRect.height < 0){
						srcRect.y		+= srcRect.height;
						srcRect.height	= -srcRect.height;
					}
				} NBASSERT(srcRect.width >= 0 && srcRect.height >= 0)
				//Validate negative start
				{
					if(pos.x < 0){
						srcRect.x		-= pos.x;
						srcRect.width	+= pos.x;
						pos.x			= 0;
					}
					if(pos.y < 0){
						srcRect.y		-= pos.y;
						srcRect.height	+= pos.y;
						pos.y			= 0;
					}
				} NBASSERT(srcRect.x >= 0 && srcRect.y >= 0 && srcRect.width >= 0 && srcRect.height >= 0)
				//Validate against dst-rect
				{
					if((pos.x + srcRect.width) > dstPrps->size.width){
						srcRect.width = (dstPrps->size.width - pos.x);
					}
					if((pos.y + srcRect.height) > dstPrps->size.height){
						srcRect.height = (dstPrps->size.height - pos.y);
					}
				}
				//Copy content
				if(srcRect.width <= 0 || srcRect.height <= 0){
					//PRINTF_INFO("Bitmaqp omited.\n");
				} else {
					//Inside dstRange
					NBASSERT(pos.x >= 0 && pos.x < dstPrps->size.width)
					NBASSERT(pos.y >= 0 && pos.y < dstPrps->size.height)
					NBASSERT((pos.x + srcRect.width) > 0 && (pos.x + srcRect.width) <= dstPrps->size.width)
					NBASSERT((pos.y + srcRect.height) > 0 && (pos.y + srcRect.height) <= dstPrps->size.height)
					//
					NBBitmapGetPixFunc getSrcPixFunc = NBBitmap_colorMap[srcPrps->color].getPixFunc;
					NBBitmapGetPixFunc getMskPixFunc = NBBitmap_colorMap[mskPrps->color].getPixFunc;
					NBBitmapGetPixFunc getDstPixFunc = NBBitmap_colorMap[dstPrps->color].getPixFunc;
					NBBitmapSetPixFunc setDstPixFunc = NBBitmap_colorMap[dstPrps->color].setPixFunc;
					STNBPointI dstPosI, srcPosI, srcPosI2, mskPosI, mskPosI2;
					STNBColor8 srcPx, mskPx, dstPx; STNBColorI vPx;
					SI32 iY, iX, srcAlpha, srcAlphaInv;
					for(iY = 0; iY < srcRect.height; iY++){
						for(iX = 0; iX < srcRect.width; iX++){
							dstPosI.x	= pos.x + iX; NBASSERT(dstPosI.x >= 0 && dstPosI.x < dstPrps->size.width)
							dstPosI.y	= pos.y + iY; NBASSERT(dstPosI.y >= 0 && dstPosI.y < dstPrps->size.height)
							srcPosI.x	= srcRect.x + iX;
							srcPosI.y	= srcRect.y + iY;
							srcPosI2.x	= (srcPosI.x < 0 ? srcPrps->size.width : 0) + (srcPosI.x % srcPrps->size.width);
							srcPosI2.y	= (srcPosI.y < 0 ? srcPrps->size.height : 0) + (srcPosI.y % srcPrps->size.height);
							mskPosI.x	= mskLeftTop.x + iX;
							mskPosI.y	= mskLeftTop.y + iY;
							mskPosI2.x	= (mskPosI.x < 0 ? mskPrps->size.width : 0) + (mskPosI.x % mskPrps->size.width);
							mskPosI2.y	= (mskPosI.y < 0 ? mskPrps->size.height : 0) + (mskPosI.y % mskPrps->size.height);
							//Get colors
							(*getMskPixFunc)(mskPrps, mskData, mskPosI2.x, mskPosI2.y, &mskPx);
							(*getSrcPixFunc)(srcPrps, srcData, srcPosI2.x, srcPosI2.y, &srcPx);
							(*getDstPixFunc)(&opq->props, opq->data, dstPosI.x, dstPosI.y, &dstPx);
							//Calculate
							srcAlpha 	= (mskPx.a * srcPx.a * color.a) / 65025;
							srcAlphaInv	= (255 - srcAlpha);
							vPx.r		= ((srcPx.r * color.r * srcAlpha) / 65025) + ((dstPx.r * srcAlphaInv) / 255);	dstPx.r = (vPx.r > 255 ? 255 : vPx.r);
							vPx.g		= ((srcPx.g * color.g * srcAlpha) / 65025) + ((dstPx.g * srcAlphaInv) / 255);	dstPx.g = (vPx.g > 255 ? 255 : vPx.g);
							vPx.b		= ((srcPx.b * color.b * srcAlpha) / 65025) + ((dstPx.b * srcAlphaInv) / 255);	dstPx.b = (vPx.b > 255 ? 255 : vPx.b);
							vPx.a		= dstPx.a + ((srcAlpha * (255 - dstPx.a)) / 255);								dstPx.a = (vPx.a > 255 ? 255 : vPx.a);
							//vPx.a		= ((srcAlpha * srcAlpha) / 255) + ((dstPx.a * srcAlphaInv) / 255);				dstPx.a = (vPx.a > 255 ? 255 : vPx.a);
							//Assign
							(*setDstPixFunc)(&opq->props, opq->data, dstPosI.x, dstPosI.y, dstPx);
						}
					}
				}
			}
		}
	}
	return r;
}

//Paste of bitmap 90-rotated (low calculation)

BOOL NBBitmap_pasteBitmapRotatedRight90(STNBBitmap* obj, const STNBPointI pos, const STNBBitmap* src, const STNBColor8 color, const UI32 pTimesRotated90){
	BOOL r = FALSE;
	if(obj != NULL && src != NULL){
		const BYTE* srcData = NBBitmap_getData(src);
		if(srcData != NULL){
			const UI8 timesRotated90 = (pTimesRotated90 % 4);
			const STNBBitmapProps srcProps = NBBitmap_getProps(src);
			if(timesRotated90 == 0){
				//Not rotation
				r = NBBitmap_pasteBitmapDataRect(obj, pos, srcProps, srcData, NBST_P(STNBRectI, 0, 0, srcProps.size.width, srcProps.size.height ), color);
			} else {
				//Rotated
				NBASSERT(timesRotated90 == 1 || timesRotated90 == 2 || timesRotated90 == 3)
				r = NBBitmap_pasteBitmapDataRectRotatedRight90(obj, pos, srcProps, srcData, NBST_P(STNBRectI, 0, 0, srcProps.size.width, srcProps.size.height ), color, timesRotated90);
			}
		}
	}
	return r;
}

BOOL NBBitmap_pasteBitmapRectRotatedRight90(STNBBitmap* obj, const STNBPointI pos, const STNBBitmap* src, const STNBRectI srcRect, const STNBColor8 color, const UI32 pTimesRotated90){
	BOOL r = FALSE;
	if(obj != NULL && src != NULL){
		const BYTE* srcData = NBBitmap_getData(src);
		if(srcData != NULL){
			const UI8 timesRotated90 = (pTimesRotated90 % 4);
			const STNBBitmapProps srcProps = NBBitmap_getProps(src);
			if(timesRotated90 == 0){
				//Not rotation
				r = NBBitmap_pasteBitmapDataRect(obj, pos, srcProps, srcData, srcRect, color);
			} else {
				//Rotated
				NBASSERT(timesRotated90 == 1 || timesRotated90 == 2 || timesRotated90 == 3)
				r = NBBitmap_pasteBitmapDataRectRotatedRight90(obj, pos, srcProps, srcData, srcRect, color, timesRotated90);
			}
		}
	}
	return r;
}

BOOL NBBitmap_pasteBitmapDataRotatedRight90(STNBBitmap* obj, const STNBPointI pos, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBColor8 color, const UI32 pTimesRotated90){
	BOOL r = FALSE;
	if(obj != NULL && srcData != NULL){
		const UI8 timesRotated90 = (pTimesRotated90 % 4);
		if(timesRotated90 == 0){
			//Not rotation
			r = NBBitmap_pasteBitmapDataRect(obj, pos, srcProps, srcData, NBST_P(STNBRectI, 0, 0, srcProps.size.width, srcProps.size.height ), color);
		} else {
			//Rotated
			NBASSERT(timesRotated90 == 1 || timesRotated90 == 2 || timesRotated90 == 3)
			r = NBBitmap_pasteBitmapDataRectRotatedRight90(obj, pos, srcProps, srcData, NBST_P(STNBRectI, 0, 0, srcProps.size.width, srcProps.size.height ), color, timesRotated90);
		}
	}
	return r;
}

BOOL NBBitmap_pasteBitmapDataRectRotatedRight90(STNBBitmap* obj, const STNBPointI pPos, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBRectI pSrcRect, const STNBColor8 color, const UI32 pTimesRotated90){
	BOOL r = FALSE;
	if(obj != NULL && srcData != NULL && pSrcRect.width != 0 && pSrcRect.height != 0){
		const UI8 timesRotated90 = (pTimesRotated90 % 4);
		if(timesRotated90 == 0){
			//Not rotation
			r = NBBitmap_pasteBitmapDataRect(obj, pPos, srcProps, srcData, pSrcRect, color);
		} else {
			STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
			if(opq != NULL){
				//Rotated
				NBASSERT(timesRotated90 == 1 || timesRotated90 == 2 || timesRotated90 == 3)
				const STNBBitmapProps* dstPrps = &opq->props;
				const STNBBitmapProps* srcPrps = &srcProps;
				STNBPointI pos = pPos;
				STNBRectI srcRect = pSrcRect;
				//Validate negative rect
				{
					if(srcRect.width < 0){
						srcRect.x		+= srcRect.width;
						srcRect.width	= -srcRect.width;
					}
					if(srcRect.height < 0){
						srcRect.y		+= srcRect.height;
						srcRect.height	= -srcRect.height;
					}
				} NBASSERT(srcRect.width >= 0 && srcRect.height >= 0)
				//Validate negative start
				{
					if(pos.x < 0){
						srcRect.x		-= pos.x;
						srcRect.width	+= pos.x;
						pos.x			= 0;
					}
					if(pos.y < 0){
						srcRect.y		-= pos.y;
						srcRect.height	+= pos.y;
						pos.y			= 0;
					}
				} NBASSERT(srcRect.x >= 0 && srcRect.y >= 0 && srcRect.width >= 0 && srcRect.height >= 0)
				//Validate against dst-rect
				{
					switch (timesRotated90) {
						case 1:
							//---------------------------------
							//x = y, y becomes x-negative
							//---------------------------------
							if((pos.x - srcRect.height) < 0){
								srcRect.height = pos.x;
							}
							if((pos.y + srcRect.width) > dstPrps->size.height){
								srcRect.width = (dstPrps->size.height - pos.y);
							}
							break;
						case 2:
							//---------------------------------
							//x = x-negative, y becomes y-negative
							//---------------------------------
							if((pos.x - srcRect.width) < 0){
								srcRect.width = pos.x;
							}
							if((pos.y - srcRect.height) < 0){
								srcRect.height = pos.y;
							}
							break;
						default:
							NBASSERT(timesRotated90 == 3)
							//---------------------------------
							//x = y-negative, y becomes x
							//---------------------------------
							if((pos.x + srcRect.height) > dstPrps->size.width){
								srcRect.height = (dstPrps->size.width - pos.x);
							}
							if((pos.y - srcRect.width) < 0){
								srcRect.width = pos.y;
							}
							break;
					}
				}
				//Copy data
				if(srcRect.width <= 0 || srcRect.height <= 0){
					//PRINTF_INFO("Bitmaqp omited.\n");
				} else {
					//Validate, inside-dstRange
					NBASSERT(pos.x >= 0 && pos.x < dstPrps->size.width)
					NBASSERT(pos.y >= 0 && pos.y < dstPrps->size.height)
					NBBitmapGetPixFunc getSrcPixFunc = NBBitmap_colorMap[srcPrps->color].getPixFunc;
					NBBitmapGetPixFunc getDstPixFunc = NBBitmap_colorMap[dstPrps->color].getPixFunc;
					NBBitmapSetPixFunc setDstPixFunc = NBBitmap_colorMap[dstPrps->color].setPixFunc;
					switch (timesRotated90) {
						case 1:
							//---------------------------------
							//x = y, y becomes x-negative
							//---------------------------------
							{
								//Validate, inside-dstRange
								NBASSERT((pos.x - srcRect.height) >= 0 && (pos.x - srcRect.height) < dstPrps->size.width)
								NBASSERT((pos.y + srcRect.width) > 0 && (pos.y + srcRect.width) <= dstPrps->size.height)
								//Per-pixel copy
								STNBPointI dstPosI, srcPosI, srcPosI2;
								STNBColor8 srcPx, dstPx; STNBColorI vPx;
								SI32 iY, iX, srcAlpha, srcAlphaInv;
								for(iY = 0; iY < srcRect.height; iY++){
									for(iX = 0; iX < srcRect.width; iX++){
										dstPosI.x	= pos.x - iY; NBASSERT(dstPosI.x >= 0 && dstPosI.x < dstPrps->size.width)
										dstPosI.y	= pos.y + iX; NBASSERT(dstPosI.y >= 0 && dstPosI.y < dstPrps->size.height)
										srcPosI.x	= srcRect.x + iX;
										srcPosI.y	= srcRect.y + iY;
										srcPosI2.x	= (srcPosI.x < 0 ? srcPrps->size.width : 0) + (srcPosI.x % srcPrps->size.width);
										srcPosI2.y	= (srcPosI.y < 0 ? srcPrps->size.height : 0) + (srcPosI.y % srcPrps->size.height);
										//Get colors
										(*getSrcPixFunc)(srcPrps, srcData, srcPosI2.x, srcPosI2.y, &srcPx);
										(*getDstPixFunc)(&opq->props, opq->data, dstPosI.x, dstPosI.y, &dstPx);
										//Calculate
										srcAlpha 	= (srcPx.a * color.a) / 255;
										srcAlphaInv	= (255 - srcAlpha);
										vPx.r		= ((srcPx.r * color.r * srcAlpha) / 65025) + ((dstPx.r * srcAlphaInv) / 255);	dstPx.r = (vPx.r > 255 ? 255 : vPx.r);
										vPx.g		= ((srcPx.g * color.g * srcAlpha) / 65025) + ((dstPx.g * srcAlphaInv) / 255);	dstPx.g = (vPx.g > 255 ? 255 : vPx.g);
										vPx.b		= ((srcPx.b * color.b * srcAlpha) / 65025) + ((dstPx.b * srcAlphaInv) / 255);	dstPx.b = (vPx.b > 255 ? 255 : vPx.b);
										vPx.a		= dstPx.a + ((srcAlpha * (255 - dstPx.a)) / 255);								dstPx.a = (vPx.a > 255 ? 255 : vPx.a);
										//vPx.a		= ((srcAlpha * srcAlpha) / 255) + ((dstPx.a * srcAlphaInv) / 255);				dstPx.a = (vPx.a > 255 ? 255 : vPx.a);
										//Assign
										(*setDstPixFunc)(&opq->props, opq->data, dstPosI.x, dstPosI.y, dstPx);
									}
									r = TRUE;
								}
							}
							break;
						case 2:
							//---------------------------------
							//x = x-negative, y becomes y-negative
							//---------------------------------
							{
								//Validate, inside-dstRange
								NBASSERT((pos.x - srcRect.width) >= 0 && (pos.x - srcRect.width) < dstPrps->size.width)
								NBASSERT((pos.y - srcRect.height) >= 0 && (pos.y - srcRect.height) < dstPrps->size.height)
								//Per-pixel copy
								STNBPointI dstPosI, srcPosI, srcPosI2;
								STNBColor8 srcPx, dstPx; STNBColorI vPx;
								SI32 iY, iX, srcAlpha, srcAlphaInv;
								for(iY = 0; iY < srcRect.height; iY++){
									for(iX = 0; iX < srcRect.width; iX++){
										dstPosI.x	= pos.x - iX; NBASSERT(dstPosI.x >= 0 && dstPosI.x < dstPrps->size.width)
										dstPosI.y	= pos.y - iY; NBASSERT(dstPosI.y >= 0 && dstPosI.y < dstPrps->size.height)
										srcPosI.x	= srcRect.x + iX;
										srcPosI.y	= srcRect.y + iY;
										srcPosI2.x	= (srcPosI.x < 0 ? srcPrps->size.width : 0) + (srcPosI.x % srcPrps->size.width);
										srcPosI2.y	= (srcPosI.y < 0 ? srcPrps->size.height : 0) + (srcPosI.y % srcPrps->size.height);
										//Get colors
										(*getSrcPixFunc)(srcPrps, srcData, srcPosI2.x, srcPosI2.y, &srcPx);
										(*getDstPixFunc)(&opq->props, opq->data, dstPosI.x, dstPosI.y, &dstPx);
										//Calculate
										srcAlpha 	= (srcPx.a * color.a) / 255;
										srcAlphaInv	= (255 - srcAlpha);
										vPx.r		= ((srcPx.r * color.r * srcAlpha) / 65025) + ((dstPx.r * srcAlphaInv) / 255);	dstPx.r = (vPx.r > 255 ? 255 : vPx.r);
										vPx.g		= ((srcPx.g * color.g * srcAlpha) / 65025) + ((dstPx.g * srcAlphaInv) / 255);	dstPx.g = (vPx.g > 255 ? 255 : vPx.g);
										vPx.b		= ((srcPx.b * color.b * srcAlpha) / 65025) + ((dstPx.b * srcAlphaInv) / 255);	dstPx.b = (vPx.b > 255 ? 255 : vPx.b);
										vPx.a		= dstPx.a + ((srcAlpha * (255 - dstPx.a)) / 255);								dstPx.a = (vPx.a > 255 ? 255 : vPx.a);
										//vPx.a		= ((srcAlpha * srcAlpha) / 255) + ((dstPx.a * srcAlphaInv) / 255);				dstPx.a = (vPx.a > 255 ? 255 : vPx.a);
										//Assign
										(*setDstPixFunc)(&opq->props, opq->data, dstPosI.x, dstPosI.y, dstPx);
									}
									r = TRUE;
								}
							}
							break;
						default:
							NBASSERT(timesRotated90 == 3)
							//---------------------------------
							//x = y-negative, y becomes x
							//---------------------------------
							{
								//Validate, inside-dstRange
								NBASSERT((pos.x + srcRect.height) > 0 && (pos.x + srcRect.height) <= dstPrps->size.width)
								NBASSERT((pos.y - srcRect.width) >= 0 && (pos.y - srcRect.width) < dstPrps->size.height)
								//Per-pixel copy
								STNBPointI dstPosI, srcPosI, srcPosI2;
								STNBColor8 srcPx, dstPx; STNBColorI vPx;
								SI32 iY, iX, srcAlpha, srcAlphaInv;
								for(iY = 0; iY < srcRect.height; iY++){
									for(iX = 0; iX < srcRect.width; iX++){
										dstPosI.x	= pos.x + iY; NBASSERT(dstPosI.x >= 0 && dstPosI.x < dstPrps->size.width)
										dstPosI.y	= pos.y - iX; NBASSERT(dstPosI.y >= 0 && dstPosI.y < dstPrps->size.height)
										srcPosI.x	= srcRect.x + iX;
										srcPosI.y	= srcRect.y + iY;
										srcPosI2.x	= (srcPosI.x < 0 ? srcPrps->size.width : 0) + (srcPosI.x % srcPrps->size.width);
										srcPosI2.y	= (srcPosI.y < 0 ? srcPrps->size.height : 0) + (srcPosI.y % srcPrps->size.height);
										//Get colors
										(*getSrcPixFunc)(srcPrps, srcData, srcPosI2.x, srcPosI2.y, &srcPx);
										(*getDstPixFunc)(&opq->props, opq->data, dstPosI.x, dstPosI.y, &dstPx);
										//Calculate
										srcAlpha 	= (srcPx.a * color.a) / 255;
										srcAlphaInv	= (255 - srcAlpha);
										vPx.r		= ((srcPx.r * color.r * srcAlpha) / 65025) + ((dstPx.r * srcAlphaInv) / 255);	dstPx.r = (vPx.r > 255 ? 255 : vPx.r);
										vPx.g		= ((srcPx.g * color.g * srcAlpha) / 65025) + ((dstPx.g * srcAlphaInv) / 255);	dstPx.g = (vPx.g > 255 ? 255 : vPx.g);
										vPx.b		= ((srcPx.b * color.b * srcAlpha) / 65025) + ((dstPx.b * srcAlphaInv) / 255);	dstPx.b = (vPx.b > 255 ? 255 : vPx.b);
										vPx.a		= dstPx.a + ((srcAlpha * (255 - dstPx.a)) / 255);								dstPx.a = (vPx.a > 255 ? 255 : vPx.a);
										//vPx.a		= ((srcAlpha * srcAlpha) / 255) + ((dstPx.a * srcAlphaInv) / 255);				dstPx.a = (vPx.a > 255 ? 255 : vPx.a);
										//Assign
										(*setDstPixFunc)(&opq->props, opq->data, dstPosI.x, dstPosI.y, dstPx);
									}
									r = TRUE;
								}
							}
							break;
					}
				}
			}
		}
	}
	return r;
}


//Paste of bitmap scaled (low calculation)

BOOL NBBitmap_pasteBitmapScaledRect(STNBBitmap* obj, const STNBRectI dstRect, const STNBBitmap* src, const STNBRectI srcRect, const STNBColor8 color){
	BOOL r = FALSE;
	if(src != NULL){
		STNBBitmapOpq* opq2 = (STNBBitmapOpq*)src->opaque;
		r = NBBitmap_pasteBitmapScaledDataRect(obj, dstRect, opq2->props, opq2->data, srcRect, color);
	}
	return r;
}

BOOL NBBitmap_pasteBitmapScaledDataRect(STNBBitmap* obj, const STNBRectI pDstRect, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBRectI pSrcRect, const STNBColor8 color){
	BOOL r = FALSE;
	STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	if(opq->data != NULL && srcData != NULL && pSrcRect.width != 0 && pSrcRect.height != 0){
		const STNBBitmapProps* dstPrps	= &opq->props;
		const STNBBitmapProps* srcPrps	= &srcProps;
		BYTE* dstData		= opq->data;
		STNBRectI dstRect	= pDstRect;
		STNBRectI srcRect	= pSrcRect;
		//Validate dstRect
		{
			while((dstRect.width < 0 || dstRect.height < 0 || dstRect.x < 0 || dstRect.y < 0 || (dstRect.x + dstRect.width) > dstPrps->size.width || (dstRect.y + dstRect.height) > dstPrps->size.height) && dstRect.width != 0 && dstRect.height != 0 && srcRect.width != 0 && srcRect.height != 0){
				//Validate negative rect
				{
					if(dstRect.width < 0){
						dstRect.x		+= dstRect.width;
						dstRect.width	= -dstRect.width;
					}
					if(dstRect.height < 0){
						dstRect.y		+= dstRect.height;
						dstRect.height	= -dstRect.height;
					}
				} NBASSERT(dstRect.width > 0 && dstRect.height > 0)
				//Validate negative start
				{
					if(dstRect.x < 0 && dstRect.width != 0){
						const float rel	= ((float)dstRect.x / (float)dstRect.width);
						dstRect.width	+= dstRect.x;
						dstRect.x		-= dstRect.x;
						srcRect.width	+= ((float)srcRect.width * rel);
						srcRect.x		-= ((float)srcRect.width * rel);
					}
					if(dstRect.y < 0 && dstRect.height != 0){
						const float rel	= ((float)dstRect.y / (float)dstRect.height);
						dstRect.height	+= dstRect.y;
						dstRect.y		-= dstRect.y;
						srcRect.height	+= ((float)srcRect.height * rel);
						srcRect.y		-= ((float)srcRect.height * rel);
					}
				} NBASSERT(dstRect.x >= 0 && dstRect.y >= 0 && dstRect.width >= 0 && dstRect.height >= 0)
				//Validate size
				{
					if((dstRect.x + dstRect.width) > dstPrps->size.width && dstRect.width != 0){
						const SI32 dlta	= dstRect.width - (dstPrps->size.width - dstRect.x);
						const float rel	= ((float)dlta / (float)dstRect.width);
						dstRect.width	-= dlta; NBASSERT(dstRect.width == (dstPrps->size.width - dstRect.x))
						srcRect.width	-= ((float)srcRect.width * rel);
					} NBASSERT((dstRect.x + dstRect.width) <= dstPrps->size.width)
					//
					if((dstRect.y + dstRect.height) > dstPrps->size.height && dstRect.height != 0){
						const SI32 dlta	= dstRect.height - (dstPrps->size.height - dstRect.y);
						const float rel	= ((float)dlta / (float)dstRect.height);
						dstRect.height	-= dlta; NBASSERT(dstRect.height == (dstPrps->size.height - dstRect.y))
						srcRect.height	-= ((float)srcRect.height * rel);
					} NBASSERT((dstRect.y + dstRect.height) <= dstPrps->size.height)
				}
			}
			NBASSERT(dstRect.x >= 0 && dstRect.y >= 0 && dstRect.width >= 0 && dstRect.height >= 0)
			NBASSERT((dstRect.x + dstRect.width) <= dstPrps->size.width)
			NBASSERT((dstRect.y + dstRect.height) <= dstPrps->size.height)
		}
		if(dstRect.width == 0 || dstRect.height == 0){
			r = TRUE; //Nothing to draw
		} else if(srcRect.width != 0 && srcRect.height != 0){
			//Validate negative rect
			{
				if(srcRect.width < 0){
					srcRect.x		+= srcRect.width;
					srcRect.width	= -srcRect.width;
				}
				if(srcRect.height < 0){
					srcRect.y		+= srcRect.height;
					srcRect.height	= -srcRect.height;
				}
			} NBASSERT(srcRect.width >= 0 && srcRect.height >= 0)
			//Determine drawing method
			if(dstPrps->color == srcPrps->color && dstPrps->bitsPerPx == srcPrps->bitsPerPx && (dstPrps->bitsPerPx % 8) == 0 && color.r == 255 && color.g == 255 && color.b == 255 && color.a == 255){
				//Direct bytes copy (optimized)
				BYTE* dstDataLine			= NULL;
				BYTE* dstDataPx				= NULL;
				const SI32 dstBytesPerPx	= (dstPrps->bitsPerPx / 8);
				const SI32 srcBytesPerPx	= (srcPrps->bitsPerPx / 8);
				const BYTE* srcDataLine		= NULL;
				const BYTE* srcDataPx		= NULL;
				SI32 iY, iX; STNBPointI srcPosI, srcPosI2;
				NBASSERT(dstBytesPerPx == srcBytesPerPx)
				switch(dstBytesPerPx) {
					case 1:
						for(iY = 0; iY < dstRect.height; iY++){
							srcPosI.y	= srcRect.y + ((float)srcRect.height * ((float)iY / (float)dstRect.height));
							srcPosI2.y	= (srcPosI.y < 0 ? srcPrps->size.height : 0) + (srcPosI.y % srcPrps->size.height);
							NBASSERT(srcPosI2.y >= 0 && srcPosI2.y < srcPrps->size.height)
							srcDataLine	= &srcData[srcPosI2.y * srcPrps->bytesPerLine];
							dstDataLine = &dstData[(dstRect.y + iY) * dstPrps->bytesPerLine];
							for(iX = 0; iX < dstRect.width; iX++){
								srcPosI.x	= srcRect.x + ((float)srcRect.width * ((float)iX / (float)dstRect.width));
								srcPosI2.x	= (srcPosI.x < 0 ? srcPrps->size.width : 0) + (srcPosI.x % srcPrps->size.width);
								NBASSERT(srcPosI2.x >= 0 && srcPosI2.x < srcPrps->size.width)
								srcDataPx	= &srcDataLine[srcPosI2.x * srcBytesPerPx];
								dstDataPx	= &dstDataLine[(dstRect.x + iX) * dstBytesPerPx];
								*dstDataPx	= *srcDataPx; //1-byte
							}
						}
						break;
					case 2:
						for(iY = 0; iY < dstRect.height; iY++){
							srcPosI.y	= srcRect.y + ((float)srcRect.height * ((float)iY / (float)dstRect.height));
							srcPosI2.y	= (srcPosI.y < 0 ? srcPrps->size.height : 0) + (srcPosI.y % srcPrps->size.height);
							NBASSERT(srcPosI2.y >= 0 && srcPosI2.y < srcPrps->size.height)
							srcDataLine	= &srcData[srcPosI2.y * srcPrps->bytesPerLine];
							dstDataLine = &dstData[(dstRect.y + iY) * dstPrps->bytesPerLine];
							for(iX = 0; iX < dstRect.width; iX++){
								srcPosI.x	= srcRect.x + ((float)srcRect.width * ((float)iX / (float)dstRect.width));
								srcPosI2.x	= (srcPosI.x < 0 ? srcPrps->size.width : 0) + (srcPosI.x % srcPrps->size.width);
								NBASSERT(srcPosI2.x >= 0 && srcPosI2.x < srcPrps->size.width)
								srcDataPx	= &srcDataLine[srcPosI2.x * srcBytesPerPx];
								dstDataPx	= &dstDataLine[(dstRect.x + iX) * dstBytesPerPx];
								*((UI16*)dstDataPx)	= *((UI16*)srcDataPx); //2-bytes
							}
						}
						break;
					case 3:
						for(iY = 0; iY < dstRect.height; iY++){
							srcPosI.y	= srcRect.y + ((float)srcRect.height * ((float)iY / (float)dstRect.height));
							srcPosI2.y	= (srcPosI.y < 0 ? srcPrps->size.height : 0) + (srcPosI.y % srcPrps->size.height);
							NBASSERT(srcPosI2.y >= 0 && srcPosI2.y < srcPrps->size.height)
							srcDataLine	= &srcData[srcPosI2.y * srcPrps->bytesPerLine];
							dstDataLine = &dstData[(dstRect.y + iY) * dstPrps->bytesPerLine];
							for(iX = 0; iX < dstRect.width; iX++){
								srcPosI.x	= srcRect.x + ((float)srcRect.width * ((float)iX / (float)dstRect.width));
								srcPosI2.x	= (srcPosI.x < 0 ? srcPrps->size.width : 0) + (srcPosI.x % srcPrps->size.width);
								NBASSERT(srcPosI2.x >= 0 && srcPosI2.x < srcPrps->size.width)
								srcDataPx	= &srcDataLine[srcPosI2.x * srcBytesPerPx];
								dstDataPx	= &dstDataLine[(dstRect.x + iX) * dstBytesPerPx];
								dstDataPx[0] = srcDataPx[0]; //1-byte
								dstDataPx[1] = srcDataPx[1]; //2-byte
								dstDataPx[2] = srcDataPx[2]; //3-byte
							}
						}
						break;
					case 4:
						for(iY = 0; iY < dstRect.height; iY++){
							srcPosI.y	= srcRect.y + ((float)srcRect.height * ((float)iY / (float)dstRect.height));
							srcPosI2.y	= (srcPosI.y < 0 ? srcPrps->size.height : 0) + (srcPosI.y % srcPrps->size.height);
							NBASSERT(srcPosI2.y >= 0 && srcPosI2.y < srcPrps->size.height)
							srcDataLine	= &srcData[srcPosI2.y * srcPrps->bytesPerLine];
							dstDataLine = &dstData[(dstRect.y + iY) * dstPrps->bytesPerLine];
							for(iX = 0; iX < dstRect.width; iX++){
								srcPosI.x	= srcRect.x + ((float)srcRect.width * ((float)iX / (float)dstRect.width));
								srcPosI2.x	= (srcPosI.x < 0 ? srcPrps->size.width : 0) + (srcPosI.x % srcPrps->size.width);
								NBASSERT(srcPosI2.x >= 0 && srcPosI2.x < srcPrps->size.width)
								srcDataPx	= &srcDataLine[srcPosI2.x * srcBytesPerPx];
								dstDataPx	= &dstDataLine[(dstRect.x + iX) * dstBytesPerPx];
								*((UI32*)dstDataPx)	= *((UI32*)srcDataPx); //4-bytes
							}
						}
						break;
					default:
						for(iY = 0; iY < dstRect.height; iY++){
							srcPosI.y	= srcRect.y + ((float)srcRect.height * ((float)iY / (float)dstRect.height));
							srcPosI2.y	= (srcPosI.y < 0 ? srcPrps->size.height : 0) + (srcPosI.y % srcPrps->size.height);
							NBASSERT(srcPosI2.y >= 0 && srcPosI2.y < srcPrps->size.height)
							srcDataLine	= &srcData[srcPosI2.y * srcPrps->bytesPerLine];
							dstDataLine = &dstData[(dstRect.y + iY) * dstPrps->bytesPerLine];
							for(iX = 0; iX < dstRect.width; iX++){
								srcPosI.x	= srcRect.x + ((float)srcRect.width * ((float)iX / (float)dstRect.width));
								srcPosI2.x	= (srcPosI.x < 0 ? srcPrps->size.width : 0) + (srcPosI.x % srcPrps->size.width);
								NBASSERT(srcPosI2.x >= 0 && srcPosI2.x < srcPrps->size.width)
								srcDataPx	= &srcDataLine[srcPosI2.x * srcBytesPerPx];
								dstDataPx	= &dstDataLine[(dstRect.x + iX) * dstBytesPerPx];
								NBMemory_copy(dstDataPx, srcDataPx, srcBytesPerPx);
							}
						}
						break;
				}
				r = TRUE;
			} else if(dstPrps->color == ENNBBitmapColor_RGBA8 && srcPrps->color == ENNBBitmapColor_RGB8 && dstPrps->bitsPerPx == 32 && srcPrps->bitsPerPx == 24 && color.r == 255 && color.g == 255 && color.b == 255 && color.a == 255){
				//Special case optimization: from RGB8 (with alpha 255) to RGBA8
				//Direct bytes copy (optimized)
				BYTE* dstDataLine			= NULL;
				BYTE* dstDataPx				= NULL;
				const SI32 dstBytesPerPx	= (dstPrps->bitsPerPx / 8);
				const SI32 srcBytesPerPx	= (srcPrps->bitsPerPx / 8);
				const BYTE* srcDataLine		= NULL;
				const BYTE* srcDataPx		= NULL;
				SI32 iY, iX; STNBPointI srcPosI, srcPosI2;
				NBASSERT(dstBytesPerPx == 4)
				NBASSERT(srcBytesPerPx == 3)
				for(iY = 0; iY < dstRect.height; iY++){
					srcPosI.y	= srcRect.y + ((float)srcRect.height * ((float)iY / (float)dstRect.height));
					srcPosI2.y	= (srcPosI.y < 0 ? srcPrps->size.height : 0) + (srcPosI.y % srcPrps->size.height);
					NBASSERT(srcPosI2.y >= 0 && srcPosI2.y < srcPrps->size.height)
					srcDataLine	= &srcData[srcPosI2.y * srcPrps->bytesPerLine];
					dstDataLine = &dstData[(dstRect.y + iY) * dstPrps->bytesPerLine];
					for(iX = 0; iX < dstRect.width; iX++){
						srcPosI.x	= srcRect.x + ((float)srcRect.width * ((float)iX / (float)dstRect.width));
						srcPosI2.x	= (srcPosI.x < 0 ? srcPrps->size.width : 0) + (srcPosI.x % srcPrps->size.width);
						NBASSERT(srcPosI2.x >= 0 && srcPosI2.x < srcPrps->size.width)
						srcDataPx	= &srcDataLine[srcPosI2.x * srcBytesPerPx];
						dstDataPx	= &dstDataLine[(dstRect.x + iX) * dstBytesPerPx];
						dstDataPx[0] = srcDataPx[0]; //1-byte
						dstDataPx[1] = srcDataPx[1]; //2-byte
						dstDataPx[2] = srcDataPx[2]; //3-byte
						dstDataPx[3] = 255;			 //alpha 255
					}
				}
				r = TRUE;
			} else {
				//Per-pixel draw (universal, but slower)
				NBBitmapGetPixFunc getSrcPixFunc = NBBitmap_colorMap[srcPrps->color].getPixFunc;
				NBBitmapGetPixFunc getDstPixFunc = NBBitmap_colorMap[dstPrps->color].getPixFunc;
				NBBitmapSetPixFunc setDstPixFunc = NBBitmap_colorMap[dstPrps->color].setPixFunc;
				STNBPointI dstPosI, srcPosI, srcPosI2;
				STNBColor8 srcPx, dstPx; STNBColorI vPx;
				SI32 iY, iX, srcAlpha, srcAlphaInv;
				for(iY = 0; iY < dstRect.height; iY++){
					dstPosI.y	= dstRect.y + iY;
					srcPosI.y	= srcRect.y + ((float)srcRect.height * ((float)iY / (float)dstRect.height));
					srcPosI2.y	= (srcPosI.y < 0 ? srcPrps->size.height : 0) + (srcPosI.y % srcPrps->size.height);
					for(iX = 0; iX < dstRect.width; iX++){
						dstPosI.x	= dstRect.x + iX;
						srcPosI.x	= srcRect.x + ((float)srcRect.width * ((float)iX / (float)dstRect.width));
						srcPosI2.x	= (srcPosI.x < 0 ? srcPrps->size.width : 0) + (srcPosI.x % srcPrps->size.width);
						NBASSERT(srcPosI2.x >= 0 && srcPosI2.x < srcPrps->size.width)
						//Get colors
						(*getSrcPixFunc)(srcPrps, srcData, srcPosI2.x, srcPosI2.y, &srcPx);
						(*getDstPixFunc)(&opq->props, opq->data, dstPosI.x, dstPosI.y, &dstPx);
						//Calculate
						srcAlpha 	= (srcPx.a * color.a) / 255;
						srcAlphaInv	= (255 - srcAlpha);
						vPx.r		= ((srcPx.r * color.r * srcAlpha) / 65025) + ((dstPx.r * srcAlphaInv) / 255);	dstPx.r = (vPx.r > 255 ? 255 : vPx.r);
						vPx.g		= ((srcPx.g * color.g * srcAlpha) / 65025) + ((dstPx.g * srcAlphaInv) / 255);	dstPx.g = (vPx.g > 255 ? 255 : vPx.g);
						vPx.b		= ((srcPx.b * color.b * srcAlpha) / 65025) + ((dstPx.b * srcAlphaInv) / 255);	dstPx.b = (vPx.b > 255 ? 255 : vPx.b);
						vPx.a		= dstPx.a + ((srcAlpha * (255 - dstPx.a)) / 255);								dstPx.a = (vPx.a > 255 ? 255 : vPx.a);
						//vPx.a		= ((srcAlpha * srcAlpha) / 255) + ((dstPx.a * srcAlphaInv) / 255);				dstPx.a = (vPx.a > 255 ? 255 : vPx.a);
						//Assign
						(*setDstPixFunc)(&opq->props, opq->data, dstPosI.x, dstPosI.y, dstPx);
					}
				}
				r = TRUE;
			}
		}
	}
	return r;
}

//Paste of bitmap scaled (high calculation)

BOOL NBBitmap_drawBitmapSmoothScaledRect(STNBBitmap* obj, const STNBRectI dstRect, const STNBBitmap* src, const STNBRectI srcRect, const STNBColor8 color){
	BOOL r = FALSE;
	if(src != NULL){
		STNBBitmapOpq* opq2 = (STNBBitmapOpq*)src->opaque;
		r = NBBitmap_drawBitmapSmoothScaledDataRect(obj, dstRect, opq2->props, opq2->data, srcRect, color);
	}
	return r;
}

BOOL NBBitmap_drawBitmapSmoothScaledDataRect(STNBBitmap* obj, const STNBRectI pDstRect, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBRectI pSrcRect, const STNBColor8 color){
	BOOL r = FALSE;
	STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	if(opq->data != NULL && srcData != NULL && pSrcRect.width != 0 && pSrcRect.height != 0){
		const STNBBitmapProps* dstPrps	= &opq->props;
		const STNBBitmapProps* srcPrps	= &srcProps;
		STNBRectI dstRect	= pDstRect;
		STNBRectI srcRect	= pSrcRect;
		//Validate dstRect
		{
			while((dstRect.width < 0 || dstRect.height < 0 || dstRect.x < 0 || dstRect.y < 0 || (dstRect.x + dstRect.width) > dstPrps->size.width || (dstRect.y + dstRect.height) > dstPrps->size.height) && dstRect.width != 0 && dstRect.height != 0 && srcRect.width != 0 && srcRect.height != 0){
				//Validate negative rect
				{
					if(dstRect.width < 0){
						dstRect.x		+= dstRect.width;
						dstRect.width	= -dstRect.width;
					}
					if(dstRect.height < 0){
						dstRect.y		+= dstRect.height;
						dstRect.height	= -dstRect.height;
					}
				} NBASSERT(dstRect.width > 0 && dstRect.height > 0)
				//Validate negative start
				{
					if(dstRect.x < 0 && dstRect.width != 0){
						const float rel	= ((float)dstRect.x / (float)dstRect.width);
						dstRect.width	+= dstRect.x;
						dstRect.x		-= dstRect.x;
						srcRect.width	+= ((float)srcRect.width * rel);
						srcRect.x		-= ((float)srcRect.width * rel);
					}
					if(dstRect.y < 0 && dstRect.height != 0){
						const float rel	= ((float)dstRect.y / (float)dstRect.height);
						dstRect.height	+= dstRect.y;
						dstRect.y		-= dstRect.y;
						srcRect.height	+= ((float)srcRect.height * rel);
						srcRect.y		-= ((float)srcRect.height * rel);
					}
				} NBASSERT(dstRect.x >= 0 && dstRect.y >= 0 && dstRect.width >= 0 && dstRect.height >= 0)
				//Validate size
				{
					if((dstRect.x + dstRect.width) > dstPrps->size.width && dstRect.width != 0){
						const SI32 dlta	= dstRect.width - (dstPrps->size.width - dstRect.x);
						const float rel	= ((float)dlta / (float)dstRect.width);
						dstRect.width	-= dlta; NBASSERT(dstRect.width == (dstPrps->size.width - dstRect.x))
						srcRect.width	-= ((float)srcRect.width * rel);
					} NBASSERT((dstRect.x + dstRect.width) <= dstPrps->size.width)
					//
					if((dstRect.y + dstRect.height) > dstPrps->size.height && dstRect.height != 0){
						const SI32 dlta	= dstRect.height - (dstPrps->size.height - dstRect.y);
						const float rel	= ((float)dlta / (float)dstRect.height);
						dstRect.height	-= dlta; NBASSERT(dstRect.height == (dstPrps->size.height - dstRect.y))
						srcRect.height	-= ((float)srcRect.height * rel);
					} NBASSERT((dstRect.y + dstRect.height) <= dstPrps->size.height)
				}
			}
			NBASSERT(dstRect.x >= 0 && dstRect.y >= 0 && dstRect.width >= 0 && dstRect.height >= 0)
			NBASSERT((dstRect.x + dstRect.width) <= dstPrps->size.width)
			NBASSERT((dstRect.y + dstRect.height) <= dstPrps->size.height)
		}
		if(dstRect.width == 0 || dstRect.height == 0){
			r = TRUE; //Nothing to draw
		} else if(srcRect.width != 0 && srcRect.height != 0){
			//Validate negative rect
			{
				if(srcRect.width < 0){
					srcRect.x		+= srcRect.width;
					srcRect.width	= -srcRect.width;
				}
				if(srcRect.height < 0){
					srcRect.y		+= srcRect.height;
					srcRect.height	= -srcRect.height;
				}
			} NBASSERT(srcRect.width >= 0 && srcRect.height >= 0)
			//Determine drawing method
			{
				const SI32 scale = 4;
				//Per-pixel draw (universal, but slower)
				NBBitmapGetPixFunc getDstPixFunc = NBBitmap_colorMap[dstPrps->color].getPixFunc;
				NBBitmapSetPixFunc setDstPixFunc = NBBitmap_colorMap[dstPrps->color].setPixFunc;
				STNBColor8 srcPx, dstPx; STNBColorI vPx; UI8 srcAlpha, srcAlphaInv;
				SI32 iX, iY, dstX, dstY; STNBAABoxI scaldSrcBox;
				//PRINTF_INFO("dstRect-x(%d, +%d)-y(%d, +%d), srcRect-x(%d, +%d)-y(%d, +%d).\n", dstRect.x, dstRect.width, dstRect.y, dstRect.height, srcRect.x, srcRect.width, srcRect.y, srcRect.height);
				for(iY = 0; iY < dstRect.height; iY++){
					scaldSrcBox.yMin = srcRect.y + (iY * srcRect.height / dstRect.height);
					scaldSrcBox.yMax = srcRect.y + ((iY + 1) * srcRect.height / dstRect.height);
					//PRINTF_INFO("srcBox-y(%d, +%d of %d).\n", scaldSrcBox.yMin, (scaldSrcBox.yMax - scaldSrcBox.yMin), srcRect.height);
					scaldSrcBox.yMin *= scale;
					scaldSrcBox.yMax *= scale;
					dstY = dstRect.y + iY;
					for(iX = 0; iX < dstRect.width; iX++){
						scaldSrcBox.xMin = srcRect.x + (iX * srcRect.width / dstRect.width);
						scaldSrcBox.xMax = srcRect.x + ((iX + 1) * srcRect.width / dstRect.width);
						dstX = dstRect.x + iX;
						//PRINTF_INFO("dstPx(%d, %d), srcBox-x(%d, +%d, of %d).\n", dstX, dstY, scaldSrcBox.xMin, (scaldSrcBox.xMax - scaldSrcBox.xMin), srcRect.width);
						//Accumulate color
						scaldSrcBox.xMin *= scale;
						scaldSrcBox.xMax *= scale;
						//Get colors
						(*getDstPixFunc)(&opq->props, opq->data, dstX, dstY, &dstPx);
						NBBitmap_getColorAtInternalScaledBoxOpq(srcPrps, srcData, scaldSrcBox, scale, &srcPx);
						//Calculate
						srcAlpha 	= (srcPx.a * color.a) / 255;
						srcAlphaInv	= (255 - srcAlpha);
						vPx.r		= ((srcPx.r * color.r * srcAlpha) / 65025) + ((dstPx.r * srcAlphaInv) / 255);	dstPx.r = (vPx.r > 255 ? 255 : vPx.r);
						vPx.g		= ((srcPx.g * color.g * srcAlpha) / 65025) + ((dstPx.g * srcAlphaInv) / 255);	dstPx.g = (vPx.g > 255 ? 255 : vPx.g);
						vPx.b		= ((srcPx.b * color.b * srcAlpha) / 65025) + ((dstPx.b * srcAlphaInv) / 255);	dstPx.b = (vPx.b > 255 ? 255 : vPx.b);
						vPx.a		= dstPx.a + ((srcAlpha * (255 - dstPx.a)) / 255);								dstPx.a = (vPx.a > 255 ? 255 : vPx.a);
						//vPx.a		= ((srcAlpha * srcAlpha) / 255) + ((dstPx.a * srcAlphaInv) / 255);				dstPx.a = (vPx.a > 255 ? 255 : vPx.a);
						//Assign
						(*setDstPixFunc)(&opq->props, opq->data, dstX, dstY, dstPx);
					}
				}
				r = TRUE;
			}
		}
	}
	return r;
}

//Draw color

BOOL NBBitmap_drawPixel(STNBBitmap* obj, const STNBPointI pos, const STNBColor8 color){
	BOOL r = FALSE;
	STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	if(opq->data != NULL && pos.x >= 0 && pos.x < opq->props.size.width && pos.y >= 0 && pos.y < opq->props.size.height && color.a != 0){
		SI32 srcAlpha, srcAlphaInv;
		STNBColor8 dstPx; STNBColorI v;
		NBBitmapGetPixFunc getPixFunc = NBBitmap_colorMap[opq->props.color].getPixFunc;
		NBBitmapSetPixFunc setPixFunc = NBBitmap_colorMap[opq->props.color].setPixFunc;
		//Get color
		(*getPixFunc)(&opq->props, opq->data, pos.x, pos.y, &dstPx);
		//Calculate
		srcAlpha 	= color.a;
		srcAlphaInv	= (255 - srcAlpha);
		v.r			= ((color.r * srcAlpha) / 255) + ((dstPx.r * srcAlphaInv) / 255); dstPx.r = (v.r > 255 ? 255 : v.r);
		v.g			= ((color.g * srcAlpha) / 255) + ((dstPx.g * srcAlphaInv) / 255); dstPx.g = (v.g > 255 ? 255 : v.g);
		v.b			= ((color.b * srcAlpha) / 255) + ((dstPx.b * srcAlphaInv) / 255); dstPx.b = (v.b > 255 ? 255 : v.b);
		v.a			= dstPx.a + ((srcAlpha * (255 - dstPx.a)) / 255);				  dstPx.a = (v.a > 255 ? 255 : v.a);
		//v.a		= ((srcAlpha * srcAlpha) / 255) + ((dstPx.a * srcAlphaInv) / 255); dstPx.a = (v.a > 255 ? 255 : v.a);
		//Set color
		(*setPixFunc)(&opq->props, opq->data, pos.x, pos.y, dstPx);
	}
	return r;
}

BOOL NBBitmap_drawRect(STNBBitmap* obj, const STNBRect rect, const STNBColor8 color){
	BOOL r = FALSE;
	STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	if(opq->data != NULL && rect.height != 0.0f && rect.width != 0.0f && color.a != 0){
		float xMin, xMax, yMin, yMax;
		if(rect.width > 0){
			xMin = rect.x;
			xMax = rect.x + rect.width;
		} else {
			xMin = rect.x + rect.width;
			xMax = rect.x;
		}
		if(rect.height > 0){
			yMin = rect.y;
			yMax = rect.y + rect.height;
		} else {
			yMin = rect.y + rect.height;
			yMax = rect.y;
		}
		NBASSERT(xMin < xMax && yMin < yMax)
		if(xMin < xMax && yMin < yMax){
			SI32 srcAlpha, srcAlphaInv;
			STNBColor8 dstPx; STNBColorI v;
			NBBitmapGetPixFunc getPixFunc = NBBitmap_colorMap[opq->props.color].getPixFunc;
			NBBitmapSetPixFunc setPixFunc = NBBitmap_colorMap[opq->props.color].setPixFunc;
			float xInt, yInt;
			for(yInt = (SI32)yMin; yInt <= yMax; yInt++){
				float rowRel = 1.0f;
				if(yMin > yInt){
					rowRel	-= (yMin - yInt);
				}
				if(yMax < (yInt+1.0f)){
					rowRel	-= ((yInt + 1.0f) - yMax);
				}
				for(xInt = (SI32)xMin; xInt <= xMax; xInt++){
					float colRel	= 1.0f;
					if(xMin > xInt){
						colRel 		-= (xMin - xInt);
					}
					if(xMax < (xInt + 1.0f)){
						colRel 		-= ((xInt + 1.0f) - xMax);
					}
					const SI32 pxRel = 255.0f * (colRel * rowRel);
					//Get color
					(*getPixFunc)(&opq->props, opq->data, xInt, yInt, &dstPx);
					//Calculate
					srcAlpha 	= (color.a * pxRel) / 255;
					srcAlphaInv	= (255 - srcAlpha);
					v.r			= ((color.r * srcAlpha) / 255) + ((dstPx.r * srcAlphaInv) / 255); dstPx.r = (v.r > 255 ? 255 : v.r);
					v.g			= ((color.g * srcAlpha) / 255) + ((dstPx.g * srcAlphaInv) / 255); dstPx.g = (v.g > 255 ? 255 : v.g);
					v.b			= ((color.b * srcAlpha) / 255) + ((dstPx.b * srcAlphaInv) / 255); dstPx.b = (v.b > 255 ? 255 : v.b);
					v.a			= dstPx.a + ((srcAlpha * (255 - dstPx.a)) / 255);				  dstPx.a = (v.a > 255 ? 255 : v.a);
					//v.a		= ((srcAlpha * srcAlpha) / 255) + ((dstPx.a * srcAlphaInv) / 255); dstPx.a = (v.a > 255 ? 255 : v.a);
					//Set color
					(*setPixFunc)(&opq->props, opq->data, xInt, yInt, dstPx);
				}
			}
		}
	}
	return r;
}

//Draw bitmap (low calculation)

BOOL NBBitmap_drawBitmap(STNBBitmap* obj, const STNBBitmap* src, const STNBPoint posCenter, const float rotRad, const STNBSize scaleRel, const STNBColor8 color){
	STNBBitmapOpq* srcOpq = (STNBBitmapOpq*)src->opaque;
	//const STNBMatrix m = NBMatrix_fromTransforms(const STNBPoint traslation, const float radRot, const STNBSize scale)
	STNBMatrix m;
	NBMatrix_setIdentity(&m);
	NBMatrix_translate(&m, posCenter.x, posCenter.y);
	NBMatrix_scale(&m, scaleRel.width, scaleRel.height);
	NBMatrix_rotate(&m, rotRad);
	NBMatrix_translate(&m, (float)srcOpq->props.size.width / -2.0f, (float)srcOpq->props.size.height / -2.0f);
	return NBBitmap_drawBitmapRectWithMatrix(obj, src, NBST_P(STNBRect, 0, 0, srcOpq->props.size.width, srcOpq->props.size.height), m, color);
}

BOOL NBBitmap_drawBitmapData(STNBBitmap* obj, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBPoint posCenter, const float rotRad, const STNBSize scaleRel, const STNBColor8 color){
	//const STNBMatrix m = NBMatrix_fromTransforms(const STNBPoint traslation, const float radRot, const STNBSize scale)
	STNBMatrix m;
	NBMatrix_setIdentity(&m);
	NBMatrix_translate(&m, posCenter.x, posCenter.y);
	NBMatrix_scale(&m, scaleRel.width, scaleRel.height);
	NBMatrix_rotate(&m, rotRad);
	NBMatrix_translate(&m, (float)srcProps.size.width / -2.0f, (float)srcProps.size.height / -2.0f);
	return NBBitmap_drawBitmapDataRectWithMatrix(obj, srcProps, srcData, NBST_P(STNBRect, 0, 0, srcProps.size.width, srcProps.size.height), m, color);
}

BOOL NBBitmap_drawBitmapRect(STNBBitmap* obj, const STNBBitmap* src, const STNBRect srcRect, const STNBPoint posCenter, const float rotRad, const STNBSize scaleRel, const STNBColor8 color){
	STNBMatrix m;
	NBMatrix_setIdentity(&m);
	NBMatrix_translate(&m, posCenter.x, posCenter.y);
	NBMatrix_scale(&m, scaleRel.width, scaleRel.height);
	NBMatrix_rotate(&m, rotRad);
	NBMatrix_translate(&m, ((float)srcRect.width / -2.0f) - (float)srcRect.x, ((float)srcRect.height / -2.0f) - (float)srcRect.y);
	return NBBitmap_drawBitmapRectWithMatrix(obj, src, srcRect, m, color);
}

BOOL NBBitmap_drawBitmapDataRect(STNBBitmap* obj, const STNBBitmapProps srcProps, const BYTE* srcData, const STNBRect srcRect, const STNBPoint posCenter, const float rotRad, const STNBSize scaleRel, const STNBColor8 color){
	STNBMatrix m;
	NBMatrix_setIdentity(&m);
	NBMatrix_translate(&m, posCenter.x, posCenter.y);
	NBMatrix_scale(&m, scaleRel.width, scaleRel.height);
	NBMatrix_rotate(&m, rotRad);
	NBMatrix_translate(&m, ((float)srcRect.width / -2.0f) - (float)srcRect.x, ((float)srcRect.height / -2.0f) - (float)srcRect.y);
	return NBBitmap_drawBitmapDataRectWithMatrix(obj, srcProps, srcData, srcRect, m, color);
}

BOOL NBBitmap_drawBitmapRectWithMatrix(STNBBitmap* obj, const STNBBitmap* src, const STNBRect srcRect, const STNBMatrix m, const STNBColor8 color){
	BOOL r = FALSE;
	if(src != NULL){
		STNBBitmapOpq* srcOpq = (STNBBitmapOpq*)src->opaque;
		r = NBBitmap_drawBitmapDataRectWithMatrix(obj, srcOpq->props, srcOpq->data, srcRect, m, color);
	}
	return r;
}

	
BOOL NBBitmap_drawBitmapDataRectWithMatrix(STNBBitmap* obj, const STNBBitmapProps pSrcProps, const BYTE* srcData, const STNBRect srcRect, const STNBMatrix m, const STNBColor8 color){
	BOOL r = FALSE;
	STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	const STNBBitmapProps* dstPrps = &opq->props;
	const STNBBitmapProps* srcPrps = &pSrcProps;
	//Validate negative rect
	STNBRect srcRectt = srcRect;
	{
		if(srcRectt.width < 0){
			srcRectt.x		+= srcRectt.width;
			srcRectt.width	= -srcRectt.width;
		}
		if(srcRectt.height < 0){
			srcRectt.y		+= srcRectt.height;
			srcRectt.height	= -srcRectt.height;
		}
	} NBASSERT(srcRectt.width >= 0 && srcRectt.height >= 0)
	//Defining the source corners
	const STNBPoint srcPts[4] = {
		 NBST_P(STNBPoint, srcRectt.x, srcRectt.y) //left-top
		, NBST_P(STNBPoint, srcRectt.x + srcRectt.width, srcRectt.y) //right-top
		, NBST_P(STNBPoint, srcRectt.x, srcRectt.y + srcRectt.height) //left-bottom
		, NBST_P(STNBPoint, srcRectt.x + srcRectt.width, srcRectt.y + srcRectt.height) //right-bottom
	};
	//Defining the destination corners
	const STNBPoint dstPts[4] = {
		NBMatrix_applyToPoint(&m, srcPts[0]) //left-top
		, NBMatrix_applyToPoint(&m, srcPts[1]) //right-top
		, NBMatrix_applyToPoint(&m, srcPts[2]) //left-bottom
		, NBMatrix_applyToPoint(&m, srcPts[3]) //right-bottom
	};
	//Defining the destination box
	STNBAABox dstBox;
	NBAABox_wrapFirstPoint(&dstBox, dstPts[0]);
	NBAABox_wrapNextPoint(&dstBox, dstPts[1]);
	NBAABox_wrapNextPoint(&dstBox, dstPts[2]);
	NBAABox_wrapNextPoint(&dstBox, dstPts[3]);
	STNBAABoxI dstBoxI = {
		dstBox.xMin, dstBox.xMax,
		dstBox.yMin, dstBox.yMax
	};
	//Limit destination to bitmap size
	if(dstBoxI.xMin < 0) dstBoxI.xMin = 0;
	if(dstBoxI.xMax < 0) dstBoxI.xMax = 0;
	if(dstBoxI.yMin < 0) dstBoxI.yMin = 0;
	if(dstBoxI.yMax < 0) dstBoxI.yMax = 0;
	if(dstBoxI.xMin > dstPrps->size.width) dstBoxI.xMin = dstPrps->size.width;
	if(dstBoxI.xMax > dstPrps->size.width) dstBoxI.xMax = dstPrps->size.width;
	if(dstBoxI.yMin > dstPrps->size.height) dstBoxI.yMin = dstPrps->size.height;
	if(dstBoxI.yMax > dstPrps->size.height) dstBoxI.yMax = dstPrps->size.height;
	NBASSERT(dstBoxI.xMin <= dstBoxI.xMax && dstBoxI.yMin <= dstBoxI.yMax)
	//Draw hotizontal lines
	//PRINTF_INFO("drawing dstBoxI x(%d, %d) y(%d, %d).\n", dstBoxI.xMin, dstBoxI.xMax, dstBoxI.yMin, dstBoxI.yMax);
	if(dstBoxI.xMin < dstBoxI.xMax && dstBoxI.yMin < dstBoxI.yMax){
		const STNBMatrix mInv = NBMatrix_inverse(&m);
		//Validate dstRange
		NBASSERT(dstBoxI.xMin >= 0 && dstBoxI.xMin <= dstPrps->size.width)
		NBASSERT(dstBoxI.xMax >= 0 && dstBoxI.xMax <= dstPrps->size.width)
		NBASSERT(dstBoxI.yMin >= 0 && dstBoxI.yMin <= dstPrps->size.height)
		NBASSERT(dstBoxI.yMax >= 0 && dstBoxI.yMax <= dstPrps->size.height)
		//Load copy props
		const STNBSizeI dstBoxSz = { dstBoxI.xMax - dstBoxI.xMin , dstBoxI.yMax - dstBoxI.yMin };
		NBASSERT(dstBoxSz.width > 0 && dstBoxSz.width <= dstPrps->size.width && dstBoxSz.height > 0 && dstBoxSz.height <= dstPrps->size.height)
		NBBitmapGetPixFunc getSrcPixFunc = NBBitmap_colorMap[srcPrps->color].getPixFunc;
		NBBitmapGetPixFunc getDstPixFunc = NBBitmap_colorMap[dstPrps->color].getPixFunc;
		NBBitmapSetPixFunc setDstPixFunc = NBBitmap_colorMap[dstPrps->color].setPixFunc;
		const SI32 srcRectAftrX	= srcRectt.x + srcRectt.width;
		const SI32 srcRectAftrY	= srcRectt.y + srcRectt.height;
		//Copy data
		{
			BOOL outOfSrcRect;
			SI32 iY, iX, srcAlpha, srcAlphaInv;
			STNBColor8 srcPx, dstPx; STNBColorI v;
			STNBPoint srcPos; STNBPointI srcPosI, srcPosI2;
			for(iY = 0; iY < dstBoxSz.height; iY++){
				for(iX = 0; iX < dstBoxSz.width; iX++){
					srcPos		= NBMatrix_applyToPoint(&mInv, NBST_P(STNBPoint, dstBoxI.xMin + iX, dstBoxI.yMin + iY));
					srcPosI.x	= srcPos.x;
					srcPosI.y	= srcPos.y;
					outOfSrcRect = (srcPosI.x < srcRectt.x || srcPosI.y < srcRectt.y || srcPosI.x >= srcRectAftrX || srcPosI.y >= srcRectAftrY);
					//PRINTF_INFO(" dst(%d, %d) = src(%d, %d)%s .\n", dstX, dstRect.y + i - 1, srcPosI.x, srcPosI.y, (outOfSrcRng ? " out-of-rng" : ""));
					if(outOfSrcRect){
						//PRINTF_INFO("Ignoring (%d / %d, %d / %d).\n", srcPosI.x, srcPrps->size.width, srcPosI.y, srcPrps->size.height);
					} else {
						//Convert rect coordinate to bitmap coord
						srcPosI2.x	= (srcPosI.x < 0 ? srcPrps->size.width : 0) + (srcPosI.x % srcPrps->size.width);
						srcPosI2.y	= (srcPosI.y < 0 ? srcPrps->size.height : 0) + (srcPosI.y % srcPrps->size.height);
						//Read pixel
						NBASSERT(srcPosI2.x >= 0 && srcPosI2.x < srcPrps->size.width && srcPosI2.y >= 0 && srcPosI2.y < srcPrps->size.height)
						//Get colors
						(*getSrcPixFunc)(srcPrps, srcData, srcPosI2.x, srcPosI2.y, &srcPx);
						(*getDstPixFunc)(&opq->props, opq->data, dstBoxI.xMin + iX, dstBoxI.yMin + iY, &dstPx);
						//Calculate
						srcAlpha 	= (srcPx.a * color.a) / 255;
						srcAlphaInv	= (255 - srcAlpha);
						v.r			= ((srcPx.r * color.r * srcAlpha) / 65025) + ((dstPx.r * srcAlphaInv) / 255); dstPx.r = (v.r > 255 ? 255 : v.r);
						v.g			= ((srcPx.g * color.g * srcAlpha) / 65025) + ((dstPx.g * srcAlphaInv) / 255); dstPx.g = (v.g > 255 ? 255 : v.g);
						v.b			= ((srcPx.b * color.b * srcAlpha) / 65025) + ((dstPx.b * srcAlphaInv) / 255); dstPx.b = (v.b > 255 ? 255 : v.b);
						v.a			= dstPx.a + ((srcAlpha * (255 - dstPx.a)) / 255); dstPx.a = (v.a > 255 ? 255 : v.a);
						//v.a		= ((srcAlpha * srcAlpha) / 255) + ((dstPx.a * srcAlphaInv) / 255); dstPx.a = (v.a > 255 ? 255 : v.a);
						//Set
						(*setDstPixFunc)(&opq->props, opq->data, dstBoxI.xMin + iX, dstBoxI.yMin + iY, dstPx);
					}
				}
			}
		}
		r = TRUE;
	}
	return r;
}


BOOL NBBitmap_drawBitmapWithShape(STNBBitmap* obj, const STNBRectI dstArea, const STNBBitmap* src, const STNBPoint startXstartY, const STNBPoint startXendY, const STNBPoint endXendY, const STNBPoint endXstartY){
	BOOL r = FALSE;
	if(src != NULL && src->opaque != NULL){
		STNBBitmapOpq* opq2 = (STNBBitmapOpq*)src->opaque;
		r = NBBitmap_drawBitmapWithShapeData(obj, dstArea, opq2->props, opq2->data, startXstartY, startXendY, endXendY, endXstartY);
	}
	return r;
}

BOOL NBBitmap_drawBitmapWithShapeData(STNBBitmap* obj, const STNBRectI dstArea, const STNBBitmapProps pSrcProps, const BYTE* pSrcData, const STNBPoint startXstartY, const STNBPoint startXendY, const STNBPoint endXendY, const STNBPoint endXstartY){
	BOOL r = FALSE;
	STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	const STNBBitmapProps* dstPrps = &opq->props;
	BYTE* dstData = opq->data;
	if(dstData != NULL && dstPrps->size.width > 0 && dstPrps->size.height > 0 && dstArea.x >= 0 && dstArea.y >= 0 && dstArea.width >= 0 && dstArea.height >= 0 && (dstArea.x + dstArea.width) <= dstPrps->size.width && (dstArea.y + dstArea.height) <= dstPrps->size.height){
		const STNBBitmapProps* srcPrps = &pSrcProps;
		const BYTE* srcData = pSrcData;
		if(srcData != NULL && srcPrps->size.width > 0 && srcPrps->size.height > 0){
			const SI32 scale = 4;
			const float scaleF = (float)scale;
			const STNBAABoxI scldSrcRepeatBox = {
				0, srcPrps->size.width * scale
				, 0, srcPrps->size.height * scale
			};
			//
			const float startWidth	= (endXstartY.x - startXstartY.x);
			const float endWidth	= (endXendY.x - startXendY.x);
			const float widthDelta	= (endWidth - startWidth);
			const float startDeltaX	= (startXendY.x - startXstartY.x);
			//
			const float startHeight	= (startXendY.y - startXstartY.y);
			const float endHeight	= (endXendY.y - endXstartY.y);
			const float heightDelta	= (endHeight - startHeight);
			const float startDeltaY	= (endXstartY.y - startXstartY.y);
			//
			const float w = dstArea.width;
			const float h = dstArea.height;
			float x, y, x0, x1, y0, y1, xRel0, xRel1, yRel0, yRel1;
			STNBAABoxI scaldPxsBox; STNBColor8 srcColor;
			NBBitmapSetPixFunc setColorFnc = NBBitmap_getSetPixFunc(dstPrps->color);
			//
			{
				for(x = 0; x < w; x++){
					xRel0	= (x / w);
					xRel1	= ((x + 1.0f) / w);
					for(y = 0; y < h; y++){
						yRel0	= (y / h);
						yRel1	= ((y + 1.0f) / h);
						x0		= startXstartY.x + (startWidth * xRel0) + ((startDeltaX + widthDelta) * yRel0);
						x1		= startXstartY.x + (startWidth * xRel1) + ((startDeltaX + widthDelta) * yRel1);
						y0		= startXstartY.y + (startHeight * yRel0) + ((startDeltaY + heightDelta) * xRel0);
						y1		= startXstartY.y + (startHeight * yRel1) + ((startDeltaY + heightDelta) * xRel1);
						if(x0 < x1){
							scaldPxsBox.xMin = (SI32)(x0 * scaleF);
							scaldPxsBox.xMax = (SI32)(x1 * scaleF);
						} else {
							scaldPxsBox.xMin = (SI32)(x1 * scaleF);
							scaldPxsBox.xMax = (SI32)(x0 * scaleF);
						}
						if(y0 < y1){
							scaldPxsBox.yMin = (SI32)(y0 * scaleF);
							scaldPxsBox.yMax = (SI32)(y1 * scaleF);
						} else {
							scaldPxsBox.yMin = (SI32)(y1 * scaleF);
							scaldPxsBox.yMax = (SI32)(y0 * scaleF);
						}
						//Get color of area
						if(scaldPxsBox.xMax <= scldSrcRepeatBox.xMax && scaldPxsBox.yMax <= scldSrcRepeatBox.yMax){
							NBBitmap_getColorAtInternalScaledBoxOpq(srcPrps, srcData, scaldPxsBox, scale, &srcColor);
						} else {
							NBBitmap_getColorAtRepeatScaledBoxOpq(srcPrps, srcData, scaldPxsBox, scldSrcRepeatBox, scale, &srcColor);
						}
						//PRINTF_INFO("Px(%d, %d) filling-with(%.1f, %.1f)-(+%.1f, +%.1f) with color(%d, %d, %d, %d).\n", (SI32)(dstArea.x + x), (SI32)(dstArea.y + y), (float)scaldPxsBox.xMin / (float)scale, (float)scaldPxsBox.yMin / (float)scale, (float)(scaldPxsBox.xMax - scaldPxsBox.xMin) / (float)scale, (scaldPxsBox.yMax - scaldPxsBox.yMin) / (float)scale, srcColor.r, srcColor.g, srcColor.b, srcColor.a);
						//Set color of pixel
						(setColorFnc)(dstPrps, dstData, dstArea.x + (SI32)x, dstArea.y + (SI32)y, srcColor);
					}
				}
			}
			//
			r = TRUE;
		}
	}
	return r;
}

//Draw bitmap smooth (higher calculations)

BOOL NBBitmap_drawBitmapSmooth(STNBBitmap* obj, const STNBBitmap* src, const STNBPoint posCenter, const float rotRad, const STNBSize scaleRel, const STNBColor8 color){
	STNBBitmapOpq* srcOpq = (STNBBitmapOpq*)src->opaque;
	STNBMatrix m;
	NBMatrix_setIdentity(&m);
	NBMatrix_translate(&m, posCenter.x, posCenter.y);
	NBMatrix_scale(&m, scaleRel.width, scaleRel.height);
	NBMatrix_rotate(&m, rotRad);
	NBMatrix_translate(&m, (float)srcOpq->props.size.width / -2.0f, (float)srcOpq->props.size.height / -2.0f);
	return NBBitmap_drawBitmapSmoothRectWithMatrix(obj, src, NBST_P(STNBRect, 0, 0, srcOpq->props.size.width, srcOpq->props.size.height), m, color);
}

BOOL NBBitmap_drawBitmapSmoothRect(STNBBitmap* obj, const STNBBitmap* src, const STNBRect srcRect, const STNBPoint posCenter, const float rotRad, const STNBSize scaleRel, const STNBColor8 color){
	STNBMatrix m;
	NBMatrix_setIdentity(&m);
	NBMatrix_translate(&m, posCenter.x, posCenter.y);
	NBMatrix_scale(&m, scaleRel.width, scaleRel.height);
	NBMatrix_rotate(&m, rotRad);
	NBMatrix_translate(&m, ((float)srcRect.width / -2.0f) - (float)srcRect.x, ((float)srcRect.height / -2.0f) - (float)srcRect.y);
	return NBBitmap_drawBitmapSmoothRectWithMatrix(obj, src, srcRect, m, color);
}

BOOL NBBitmap_drawBitmapSmoothRectWithMatrix(STNBBitmap* obj, const STNBBitmap* src, const STNBRect srcRect, const STNBMatrix m, const STNBColor8 color){
	BOOL r = FALSE;
	STNBBitmapOpq* opq = (STNBBitmapOpq*)obj->opaque;
	STNBBitmapOpq* srcOpq = (STNBBitmapOpq*)src->opaque;
	const STNBBitmapProps* dstPrps = &opq->props;
	const STNBBitmapProps* srcPrps = &srcOpq->props;
	const BYTE* srcData = srcOpq->data;
	//Validate negative rect
	STNBRect srcRectt = srcRect;
	{
		if(srcRectt.width < 0){
			srcRectt.x		+= srcRectt.width;
			srcRectt.width	= -srcRectt.width;
		}
		if(srcRectt.height < 0){
			srcRectt.y		+= srcRectt.height;
			srcRectt.height	= -srcRectt.height;
		}
	} NBASSERT(srcRectt.width >= 0 && srcRectt.height >= 0)
	//Defining the source corners
	const STNBPoint srcPts[4] = {
		 NBST_P(STNBPoint, srcRectt.x, srcRectt.y) //left-top
		, NBST_P(STNBPoint, srcRectt.x + srcRectt.width, srcRectt.y) //right-top
		, NBST_P(STNBPoint, srcRectt.x, srcRectt.y + srcRectt.height) //left-bottom
		, NBST_P(STNBPoint, srcRectt.x + srcRectt.width, srcRectt.y + srcRectt.height) //right-bottom
	};
	//Defining the destination corners
	const STNBPoint dstPts[4] = {
		NBMatrix_applyToPoint(&m, srcPts[0]) //left-top
		, NBMatrix_applyToPoint(&m, srcPts[1]) //right-top
		, NBMatrix_applyToPoint(&m, srcPts[2]) //left-bottom
		, NBMatrix_applyToPoint(&m, srcPts[3]) //right-bottom
	};
	//Defining the destination box
	STNBAABox dstBox;
	NBAABox_wrapFirstPoint(&dstBox, dstPts[0]);
	NBAABox_wrapNextPoint(&dstBox, dstPts[1]);
	NBAABox_wrapNextPoint(&dstBox, dstPts[2]);
	NBAABox_wrapNextPoint(&dstBox, dstPts[3]);
	STNBAABoxI dstBoxI = {
		dstBox.xMin, dstBox.xMax,
		dstBox.yMin, dstBox.yMax
	};
	//Limit destination to bitmap size
	if(dstBoxI.xMin < 0) dstBoxI.xMin = 0;
	if(dstBoxI.xMax < 0) dstBoxI.xMax = 0;
	if(dstBoxI.yMin < 0) dstBoxI.yMin = 0;
	if(dstBoxI.yMax < 0) dstBoxI.yMax = 0;
	if(dstBoxI.xMin > dstPrps->size.width) dstBoxI.xMin = dstPrps->size.width;
	if(dstBoxI.xMax > dstPrps->size.width) dstBoxI.xMax = dstPrps->size.width;
	if(dstBoxI.yMin > dstPrps->size.height) dstBoxI.yMin = dstPrps->size.height;
	if(dstBoxI.yMax > dstPrps->size.height) dstBoxI.yMax = dstPrps->size.height;
	NBASSERT(dstBoxI.xMin <= dstBoxI.xMax && dstBoxI.yMin <= dstBoxI.yMax)
	//Draw hotizontal lines
	if(dstBoxI.xMin < dstBoxI.xMax && dstBoxI.yMin < dstBoxI.yMax){
		const STNBMatrix mInv = NBMatrix_inverse(&m);
		//Validate dstRange
		NBASSERT(dstBoxI.xMin >= 0 && dstBoxI.xMin <= dstPrps->size.width)
		NBASSERT(dstBoxI.xMax >= 0 && dstBoxI.xMax <= dstPrps->size.width)
		NBASSERT(dstBoxI.yMin >= 0 && dstBoxI.yMin <= dstPrps->size.height)
		NBASSERT(dstBoxI.yMax >= 0 && dstBoxI.yMax <= dstPrps->size.height)
		//Load copy props
		const STNBSizeI dstBoxSz = { dstBoxI.xMax - dstBoxI.xMin, dstBoxI.yMax - dstBoxI.yMin };
		NBASSERT(dstBoxSz.width > 0 && dstBoxSz.width <= dstPrps->size.width && dstBoxSz.height > 0 && dstBoxSz.height <= dstPrps->size.height)
		NBBitmapGetPixFunc getDstPixFunc = NBBitmap_colorMap[dstPrps->color].getPixFunc;
		NBBitmapSetPixFunc setDstPixFunc = NBBitmap_colorMap[dstPrps->color].setPixFunc;
		STNBPoint* srcTopPoss	= NBMemory_allocTypes(STNBPoint, dstBoxSz.width + 1);
		STNBPoint* srcLowPoss	= NBMemory_allocTypes(STNBPoint, dstBoxSz.width + 1);
		//Calculate first 2d-topPoss of pixels
		{
			SI32 iX; for(iX = 0; iX <= dstBoxSz.width; iX++){
				srcLowPoss[iX] = NBMatrix_applyToPoint(&mInv, NBST_P(STNBPoint, dstBoxI.xMin + iX, dstBoxI.yMin));
				//PRINTF_INFO("first-pos(%d, %d) -> (%.2f, %.2f).\n", dstRect.x + xLeft, dstRect.y, srcLowPoss[xLeft].x, srcLowPoss[xLeft].y);
			}
		}
		//Copy data
		{
			//Copy to RGBA8 from RGBA8
			const SI32 scale = 4;
			const STNBRectI scaldSrcRectt = {
				srcRectt.x * scale,
				srcRectt.y * scale,
				srcRectt.width * scale,
				srcRectt.height * scale
			};
			const STNBAABoxI scldSrcRepeatBox = {
				0, srcPrps->size.width * scale
				, 0, srcPrps->size.height * scale
			};
			const SI32 scldSrcWidth		= srcPrps->size.width * scale;
			const SI32 scldSrcHeight	= srcPrps->size.height * scale;
			const SI32 scldSrcRectAftrX	= scaldSrcRectt.x + scaldSrcRectt.width;
			const SI32 scldSrcRectAftrY	= scaldSrcRectt.y + scaldSrcRectt.height;
			STNBColor8 dstPx, srcColor; STNBColorI v;
			STNBAABox srcPxsBox; STNBAABoxI scaldPxsBox, scaldPxsBox2;
			STNBPoint* tmp; BOOL outOfSrcRng;
			SI32 iY, iX, srcAlpha, srcAlphaInv;
			for(iY = 0; iY < dstBoxSz.height; iY++){
				//Calculate new 2d-lowPoss of pixels
				{
					tmp			= srcTopPoss;
					srcTopPoss	= srcLowPoss;
					srcLowPoss	= tmp;
					for(iX = 0; iX <= dstBoxSz.width; iX++){
						srcLowPoss[iX] = NBMatrix_applyToPoint(&mInv, NBST_P(STNBPoint, dstBoxI.xMin + iX, dstBoxI.yMin + iY ));
						//PRINTF_INFO("pos(%d, %d) -> (%.2f, %.2f).\n", dstRect.x + xLeft, dstRect.y + iY, srcLowPoss[xLeft].x, srcLowPoss[xLeft].y);
					}
				}
				//Process line
				for(iX = 0; iX < dstBoxSz.width; iX++){
					//Calculate srcPixsArea
					NBAABox_wrapFirstPoint(&srcPxsBox, srcTopPoss[iX]);
					NBAABox_wrapNextPoint(&srcPxsBox, srcTopPoss[iX + 1]);
					NBAABox_wrapNextPoint(&srcPxsBox, srcLowPoss[iX]);
					NBAABox_wrapNextPoint(&srcPxsBox, srcLowPoss[iX + 1]);
					//Scaled Pixels box
					scaldPxsBox.xMin	= (srcPxsBox.xMin * (float)scale) - scaldSrcRectt.x;
					scaldPxsBox.xMax	= (srcPxsBox.xMax * (float)scale) - scaldSrcRectt.x;
					scaldPxsBox.yMin	= (srcPxsBox.yMin * (float)scale) - scaldSrcRectt.y;
					scaldPxsBox.yMax	= (srcPxsBox.yMax * (float)scale) - scaldSrcRectt.y;
					NBASSERT(scaldPxsBox.xMin <= scaldPxsBox.xMax && scaldPxsBox.yMin <= scaldPxsBox.yMax)
					outOfSrcRng	= (scaldPxsBox.xMax < scaldSrcRectt.x || scaldPxsBox.yMax < scaldSrcRectt.y || scaldPxsBox.xMin >= scldSrcRectAftrX || scaldPxsBox.yMin >= scldSrcRectAftrY);
					if(outOfSrcRng){
						//PRINTF_INFO("Out of range for color x(%.2f-%.2f)+%.2f y(%.2f-%.2f)+%.2f.\n", srcBox.xMin, srcBox.xMax, (srcBox.xMax - srcBox.xMin), srcBox.yMin, srcBox.yMax, (srcBox.yMax - srcBox.yMin));
					} else if(scaldPxsBox.xMin < scaldPxsBox.xMax && scaldPxsBox.yMin < scaldPxsBox.yMax){
						//PRINTF_INFO("Looking for color x(%.2f-%.2f)+%.2f y(%.2f-%.2f)+%.2f.\n", srcBoxLmtd.xMin, srcBoxLmtd.xMax, (srcBoxLmtd.xMax - srcBoxLmtd.xMin), srcBoxLmtd.yMin, srcBoxLmtd.yMax, (srcBoxLmtd.yMax - srcBoxLmtd.yMin));
						//Move to positive range
						const SI32 adjtdX	= (scaldPxsBox.xMin < 0 ? scldSrcWidth : 0) + (scaldPxsBox.xMin % scldSrcWidth);
						const SI32 adjtdY	= (scaldPxsBox.yMin < 0 ? scldSrcHeight : 0) + (scaldPxsBox.yMin % scldSrcHeight);
						scaldPxsBox2.xMax	= adjtdX + (scaldPxsBox.xMax - scaldPxsBox.xMin);
						scaldPxsBox2.xMin	= adjtdX;
						scaldPxsBox2.yMax	= adjtdY + (scaldPxsBox.yMax - scaldPxsBox.yMin);
						scaldPxsBox2.yMin	= adjtdY;
						NBASSERT(scaldPxsBox2.xMin >= 0 && scaldPxsBox2.xMin < scldSrcWidth);
						NBASSERT(scaldPxsBox2.yMin >= 0 && scaldPxsBox2.yMin < scldSrcHeight);
						//Get colors
						if(scaldPxsBox2.xMax <= scldSrcWidth && scaldPxsBox2.yMax <= scldSrcHeight){
							NBBitmap_getColorAtInternalScaledBoxOpq(srcPrps, srcData, scaldPxsBox2, scale, &srcColor);
						} else {
							NBBitmap_getColorAtRepeatScaledBoxOpq(srcPrps, srcData, scaldPxsBox2, scldSrcRepeatBox, scale, &srcColor);
						}
						(*getDstPixFunc)(&opq->props, opq->data, dstBoxI.xMin + iX, dstBoxI.yMin + iY, &dstPx);
						//Calculate
						srcAlpha 	= (srcColor.a * color.a) / 255;
						srcAlphaInv	= (255 - srcAlpha);
						v.r			= ((srcColor.r * color.r * srcAlpha) / 65025) + ((dstPx.r * srcAlphaInv) / 255); dstPx.r = (v.r > 255 ? 255 : v.r);
						v.g			= ((srcColor.g * color.g * srcAlpha) / 65025) + ((dstPx.g * srcAlphaInv) / 255); dstPx.g = (v.g > 255 ? 255 : v.g);
						v.b			= ((srcColor.b * color.b * srcAlpha) / 65025) + ((dstPx.b * srcAlphaInv) / 255); dstPx.b = (v.b > 255 ? 255 : v.b);
						v.a			= dstPx.a + ((srcAlpha * (255 - dstPx.a)) / 255); dstPx.a = (v.a > 255 ? 255 : v.a);
						//v.a		= ((srcAlpha * srcAlpha) / 255) + ((dstPx.a * srcAlphaInv) / 255); dstPx.a = (v.a > 255 ? 255 : v.a);
						//Set
						(*setDstPixFunc)(&opq->props, opq->data, dstBoxI.xMin + iX, dstBoxI.yMin + iY, dstPx);
					}
				}
			}
		}
		NBMemory_free(srcTopPoss);
		NBMemory_free(srcLowPoss);
		r = TRUE;
	}
	return r;
}

//Get Pixel

void NBBitmap_getPixelAlpha8_(const STNBBitmapProps* props, const BYTE* data, const UI32 x, const UI32 y, STNBColor8* dstColor){
	NBASSERT(props->color == ENNBBitmapColor_ALPHA8)
	dstColor->r = dstColor->g = dstColor->b = 255;
	dstColor->a = data[(props->bytesPerLine * y) + (x * (props->bitsPerPx / 8))];
}

void NBBitmap_getPixelGray8_(const STNBBitmapProps* props, const BYTE* data, const UI32 x, const UI32 y, STNBColor8* dstColor){
	NBASSERT(props->color == ENNBBitmapColor_GRIS8)
	dstColor->r = dstColor->g = dstColor->b = data[(props->bytesPerLine * y) + (x * (props->bitsPerPx / 8))];
	dstColor->a = 255;
}

void NBBitmap_getPixelGrayAlpha8_(const STNBBitmapProps* props, const BYTE* data, const UI32 x, const UI32 y, STNBColor8* dstColor){
	NBASSERT(props->color == ENNBBitmapColor_GRISALPHA8)
	const BYTE* pix	= &data[(props->bytesPerLine * y) + (x * (props->bitsPerPx / 8))];
	dstColor->r		= pix[0];
	dstColor->g		= dstColor->r;
	dstColor->b		= dstColor->r;
	dstColor->a		= pix[1];
}

void NBBitmap_getPixelRGB8_(const STNBBitmapProps* props, const BYTE* data, const UI32 x, const UI32 y, STNBColor8* dstColor){
	NBASSERT(props->color == ENNBBitmapColor_RGB8)
	const BYTE* pix	= &data[(props->bytesPerLine * y) + (x * (props->bitsPerPx / 8))];
	dstColor->r		= pix[0];
	dstColor->g		= pix[1];
	dstColor->b		= pix[2];
	dstColor->a		= 255;
}

void NBBitmap_getPixelRGBA8_(const STNBBitmapProps* props, const BYTE* data, const UI32 x, const UI32 y, STNBColor8* dstColor){
	NBASSERT(props->color == ENNBBitmapColor_RGBA8)
	*dstColor		= *((STNBColor8*)&data[(props->bytesPerLine * y) + (x * (props->bitsPerPx / 8))]);
}

void NBBitmap_getPixelARGB8_(const STNBBitmapProps* props, const BYTE* data, const UI32 x, const UI32 y, STNBColor8* dstColor){
	NBASSERT(props->color == ENNBBitmapColor_ARGB8)
	const BYTE* pix	= &data[(props->bytesPerLine * y) + (x * (props->bitsPerPx / 8))];
	dstColor->a		= pix[0];
	dstColor->r		= pix[1];
	dstColor->g		= pix[2];
	dstColor->b		= pix[3];
}

void NBBitmap_getPixelBGRA8_(const STNBBitmapProps* props, const BYTE* data, const UI32 x, const UI32 y, STNBColor8* dstColor){
	NBASSERT(props->color == ENNBBitmapColor_BGRA8)
	const BYTE* pix	= &data[(props->bytesPerLine * y) + (x * (props->bitsPerPx / 8))];
	dstColor->b		= pix[0];
	dstColor->g		= pix[1];
	dstColor->r		= pix[2];
	dstColor->a		= pix[3];
}

//Set Pixel

void NBBitmap_setPixelAlpha8_(const STNBBitmapProps* props, BYTE* data, const UI32 x, const UI32 y, const STNBColor8 color){
	NBASSERT(props->color == ENNBBitmapColor_ALPHA8)
	data[(props->bytesPerLine * y) + (x * (props->bitsPerPx / 8))] = color.a;
}

void NBBitmap_setPixelGray8_(const STNBBitmapProps* props, BYTE* data, const UI32 x, const UI32 y, const STNBColor8 color){
	NBASSERT(props->color == ENNBBitmapColor_GRIS8)
	data[(props->bytesPerLine * y) + (x * (props->bitsPerPx / 8))] = (color. r + color.g + color.b) / 3;
}

void NBBitmap_setPixelGrayAlpha8_(const STNBBitmapProps* props, BYTE* data, const UI32 x, const UI32 y, const STNBColor8 color){
	NBASSERT(props->color == ENNBBitmapColor_GRISALPHA8)
	BYTE* pix = &data[(props->bytesPerLine * y) + (x * (props->bitsPerPx / 8))];
	pix[0]	= (color. r + color.g + color.b) / 3;
	pix[1]	= color.a;
}

void NBBitmap_setPixelRGB8_(const STNBBitmapProps* props, BYTE* data, const UI32 x, const UI32 y, const STNBColor8 color){
	NBASSERT(props->color == ENNBBitmapColor_RGB8)
	BYTE* pix = &data[(props->bytesPerLine * y) + (x * (props->bitsPerPx / 8))];
	pix[0] = color.r;
	pix[1] = color.g;
	pix[2] = color.b;
}

void NBBitmap_setPixelRGBA8_(const STNBBitmapProps* props, BYTE* data, const UI32 x, const UI32 y, const STNBColor8 color){
	NBASSERT(props->color == ENNBBitmapColor_RGBA8)
	*((STNBColor8*)&data[(props->bytesPerLine * y) + (x * (props->bitsPerPx / 8))]) = color;
}

void NBBitmap_setPixelARGB8_(const STNBBitmapProps* props, BYTE* data, const UI32 x, const UI32 y, const STNBColor8 color){
	NBASSERT(props->color == ENNBBitmapColor_ARGB8)
	BYTE* pix = &data[(props->bytesPerLine * y) + (x * (props->bitsPerPx / 8))];
	pix[0] = color.a;
	pix[1] = color.r;
	pix[2] = color.g;
	pix[3] = color.b;
}

void NBBitmap_setPixelBGRA8_(const STNBBitmapProps* props, BYTE* data, const UI32 x, const UI32 y, const STNBColor8 color){
	NBASSERT(props->color == ENNBBitmapColor_BGRA8)
	BYTE* pix = &data[(props->bytesPerLine * y) + (x * (props->bitsPerPx / 8))];
	pix[0] = color.b;
	pix[1] = color.g;
	pix[2] = color.r;
	pix[3] = color.a;
}


