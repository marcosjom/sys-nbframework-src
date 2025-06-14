//
//  NBPng.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/11/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/2d/NBPng.h"
//
#include "nb/core/NBMemory.h"
#include "nb/2d/NBPngChunk.h"
#include "nb/crypto/NBCrc32.h"
#include "nb/files/NBFile.h"
#include "nb/compress/NBZInflate.h"
#include "nb/compress/NBZDeflate.h"

//Byte order

static inline void _invSI16(SI16* val){
	SI16 copy		= *val;
	BYTE* arrOrig 	= (BYTE*)&copy;
	BYTE* arrInvr 	= (BYTE*)val;
	arrInvr[0]		= arrOrig[1];
	arrInvr[1]		= arrOrig[0];
}

static inline void _invUI16(UI16* val){
	UI16 copy		= *val;
	BYTE* arrOrig 	= (BYTE*)&copy;
	BYTE* arrInvr 	= (BYTE*)val;
	arrInvr[0]		= arrOrig[1];
	arrInvr[1]		= arrOrig[0];
}

static inline void	_invUI32(UI32* val){
	UI32 copy		= *val;
	BYTE* arrOrig 	= (BYTE*)&copy;
	BYTE* arrInvr 	= (BYTE*)val;
	arrInvr[0]		= arrOrig[3];
	arrInvr[1]		= arrOrig[2];
	arrInvr[2]		= arrOrig[1];
	arrInvr[3]		= arrOrig[0];
}

//Adam 7 interlaced algorith

typedef struct STNBAdam7Sample_ {
	struct {
		UI32		start;
		UI32		inc;
		UI32		size;	//height
	} rows;
	struct {
		UI32		start;
		UI32		inc;
		UI32		size;	//width
	} cols;
} STNBAdam7Sample;

static const STNBAdam7Sample __adam7Defs[] = {
	{ { 0, 8, 8 }, { 0, 8, 8 } } //Pass 1
	, { { 0, 8, 8 }, { 4, 8, 4 } } //Pass 2
	, { { 4, 8, 4 }, { 0, 4, 4 } } //Pass 3
	, { { 0, 4, 4 }, { 2, 4, 2 } } //Pass 4
	, { { 2, 4, 2 }, { 0, 2, 2 } } //Pass 5
	, { { 0, 2, 2 }, { 1, 2, 1 } } //Pass 6
	, { { 1, 2, 1 }, { 0, 1, 1 } } //Pass 7
};

//Scanlines buffer

typedef struct STNBPngScanLinesBuffer_ {
	UI32	iLineStart;		//This buffer first line index
	UI32	width;			//Amount of pixels per line
	UI32	height;			//Amount of lines
	UI32	bytesPerLine;	//Bytes per line
	BYTE*	prevLine;		//If this is a continuation buffer, the last line of previous buffer
	UI32	prevLineSz;		//If this is a continuation buffer, the last line of previous buffer
	BYTE*	data;			//Each line starts with a 'filer' byte, the the pixel data
	UI32	dataToFill;		//Buffer size to fill
	UI32	dataSz;			//Buffer size reserved (last buffer can be filled partially)
} STNBPngScanLinesBuffer;

void NBPngScanLinesBuffer_init(STNBPngScanLinesBuffer* obj);
void NBPngScanLinesBuffer_create(STNBPngScanLinesBuffer* obj, const UI32 width, const UI32 height, const UI32 bitsPerPix);
void NBPngScanLinesBuffer_createNext(STNBPngScanLinesBuffer* obj, const STNBPngScanLinesBuffer* prev, const UI32 height);
void NBPngScanLinesBuffer_release(STNBPngScanLinesBuffer* obj);


// Analyze

//reads all pixels and returns an ordered list of unique pixels, returns error if colorsCountLimit is non-zero and more color are found
BOOL NBPng_calculatePalette(const STNBBitmap* bitmap, STNBColor8** dstColors, UI32* dstColorsSz, const UI32 colorsCountLimit){
    BOOL r = FALSE;
    if(bitmap != NULL){
        const STNBBitmapProps props = NBBitmap_getProps(bitmap);
        const BYTE* pixels          = NBBitmap_getData(bitmap);
        r = NBPng_calculatePaletteOnData(props, pixels, dstColors, dstColorsSz, colorsCountLimit);
    }
    return r;
}

BOOL NBPng_calculatePaletteOnData(const STNBBitmapProps props, const BYTE* pixels, STNBColor8** dstColors, UI32* dstColorsSz, const UI32 colorsCountLimit){
    BOOL r = FALSE;
    if(props.size.width > 0 && props.size.height > 0 && props.bytesPerLine > 0 && props.bitsPerPx > 0 && dstColorsSz != NULL && pixels != NULL){
        STNBArraySorted p;
        NBArraySorted_initWithSz(&p, sizeof(STNBColor8), NBCompare_NBColor8, colorsCountLimit, 1024, 0.1f);
        {
            r = NBPng_calculatePaletteOnDataTo(props, pixels, &p, colorsCountLimit);
        }
        //
        if(dstColorsSz != NULL){
            *dstColorsSz = p.use;
        }
        if(dstColors != NULL && p.use > 0){
            if(*dstColors != NULL){
                NBMemory_free(*dstColors);
                *dstColors = NULL;
            }
            //give buffer
            *dstColors = NBArraySorted_dataPtr(&p, STNBColor8);
            NBArraySorted_resignToBuffer(&p);
        }
        NBArraySorted_release(&p);
    }
    return r;
}

BOOL NBPng_calculatePaletteOnDataTo(const STNBBitmapProps props, const BYTE* pixels, STNBArraySorted* dst /*STNBColor8*/, const UI32 colorsCountLimit){
    BOOL r = FALSE;
    if(dst != NULL){
        //Analyze all pixels
        UI8* rowFirstByte = NULL;
        UI16 curByteAtRow = 0;
        UI8 curBitAtByte = 0;
        SI32 iRow = 0, iCol = 0;
        STNBColor8 c;
        NBMemory_setZeroSt(c, STNBColor8);
        //
        NBArraySorted_empty(dst);
        //Optimization: read pixels according color (usign "privInlineLeerPixel" proved to be expensive)
        /*
         ENNBBitmapColor_ALPHA8,        //only alpha (8 bits)
         ENNBBitmapColor_GRIS8,        //grayscale (8 bits)
         ENNBBitmapColor_GRISALPHA8,    //grayscale and alpha (8 bits each component)
         ENNBBitmapColor_RGB4,        //RGB (4 bits each component)
         ENNBBitmapColor_RGB8,        //RGB (8 bits each component)
         ENNBBitmapColor_RGBA4,        //RGBA (4 bits each component)
         ENNBBitmapColor_RGBA8,        //RGBA (8 bits each component)
         ENNBBitmapColor_ARGB4,        //ARGB (4 bits each component)
         ENNBBitmapColor_ARGB8,        //ARGB (8 bits each component)
         ENNBBitmapColor_BGRA8,        //BGRA (8 bits each component)
         ENNBBitmapColor_SWF_PIX15,    //
         ENNBBitmapColor_SWF_PIX24,    //reserved+R+G+B
         */
        switch (props.color) {
            case ENNBBitmapColor_ALPHA8:
                c.r = c.g = c.b = 255;
                for(iRow = 0; iRow < props.size.height && (colorsCountLimit == 0 || dst->use < colorsCountLimit); iRow++){
                    rowFirstByte    = &(pixels[iRow * props.bytesPerLine]);
                    curByteAtRow    = 0;
                    curBitAtByte    = 0;
                    for(iCol = 0; iCol < props.size.width && (colorsCountLimit == 0 || dst->use < colorsCountLimit); iCol++){
                        c.a = rowFirstByte[curByteAtRow++];
                        if(NBArraySorted_indexOf(dst, &c, sizeof(c), NULL) == -1){
                            NBArraySorted_addValue(dst, c);
                        }
                    }
                }
                r = (iRow == props.size.height && iCol == props.size.width);
                break;
            case ENNBBitmapColor_GRIS8:
                c.a = 255;
                for(iRow = 0; iRow < props.size.height && (colorsCountLimit == 0 || dst->use < colorsCountLimit); iRow++){
                    rowFirstByte    = &(pixels[iRow * props.bytesPerLine]);
                    curByteAtRow    = 0;
                    curBitAtByte    = 0;
                    for(iCol = 0; iCol < props.size.width && (colorsCountLimit == 0 || dst->use < colorsCountLimit); iCol++){
                        c.r = c.g = c.b = rowFirstByte[curByteAtRow++];
                        if(NBArraySorted_indexOf(dst, &c, sizeof(c), NULL) == -1){
                            NBArraySorted_addValue(dst, c);
                        }
                    }
                }
                r = (iRow == props.size.height && iCol == props.size.width);
                break;
            case ENNBBitmapColor_GRISALPHA8:
                for(iRow = 0; iRow < props.size.height && (colorsCountLimit == 0 || dst->use < colorsCountLimit); iRow++){
                    rowFirstByte        = &(pixels[iRow * props.bytesPerLine]);
                    curByteAtRow    = 0;
                    curBitAtByte        = 0;
                    for(iCol = 0; iCol < props.size.width && (colorsCountLimit == 0 || dst->use < colorsCountLimit); iCol++){
                        c.r = c.g = c.b = rowFirstByte[curByteAtRow++];
                        c.a = rowFirstByte[curByteAtRow++];
                        if(NBArraySorted_indexOf(dst, &c, sizeof(c), NULL) == -1){
                            NBArraySorted_addValue(dst, c);
                        }
                    }
                }
                r = (iRow == props.size.height && iCol == props.size.width);
                break;
            case ENNBBitmapColor_RGB8:
                c.a = 255;
                for(iRow = 0; iRow < props.size.height && (colorsCountLimit == 0 || dst->use < colorsCountLimit); iRow++){
                    rowFirstByte        = &(pixels[iRow * props.bytesPerLine]);
                    curByteAtRow    = 0;
                    curBitAtByte        = 0;
                    for(iCol = 0; iCol < props.size.width && (colorsCountLimit == 0 || dst->use < colorsCountLimit); iCol++){
                        c.r = rowFirstByte[curByteAtRow++];
                        c.g = rowFirstByte[curByteAtRow++];
                        c.b = rowFirstByte[curByteAtRow++];
                        if(NBArraySorted_indexOf(dst, &c, sizeof(c), NULL) == -1){
                            NBArraySorted_addValue(dst, c);
                        }
                    }
                }
                r = (iRow == props.size.height && iCol == props.size.width);
                break;
            case ENNBBitmapColor_RGBA8:
                for(iRow = 0; iRow < props.size.height && (colorsCountLimit == 0 || dst->use < colorsCountLimit); iRow++){
                    rowFirstByte        = &(pixels[iRow * props.bytesPerLine]);
                    curByteAtRow    = 0;
                    curBitAtByte        = 0;
                    for(iCol = 0; iCol < props.size.width && (colorsCountLimit == 0 || dst->use < colorsCountLimit); iCol++){
                        c.r = rowFirstByte[curByteAtRow++];
                        c.g = rowFirstByte[curByteAtRow++];
                        c.b = rowFirstByte[curByteAtRow++];
                        c.a = rowFirstByte[curByteAtRow++];
                        if(NBArraySorted_indexOf(dst, &c, sizeof(c), NULL) == -1){
                            NBArraySorted_addValue(dst, c);
                        }
                    }
                }
                r = (iRow == props.size.height && iCol == props.size.width);
                break;
            case ENNBBitmapColor_ARGB8:
                for(iRow = 0; iRow < props.size.height && (colorsCountLimit == 0 || dst->use < colorsCountLimit); iRow++){
                    rowFirstByte        = &(pixels[iRow * props.bytesPerLine]);
                    curByteAtRow    = 0;
                    curBitAtByte        = 0;
                    for(iCol = 0; iCol < props.size.width && (colorsCountLimit == 0 || dst->use < colorsCountLimit); iCol++){
                        c.a = rowFirstByte[curByteAtRow++];
                        c.r = rowFirstByte[curByteAtRow++];
                        c.g = rowFirstByte[curByteAtRow++];
                        c.b = rowFirstByte[curByteAtRow++];
                        if(NBArraySorted_indexOf(dst, &c, sizeof(c), NULL) == -1){
                            NBArraySorted_addValue(dst, c);
                        }
                    }
                }
                r = (iRow == props.size.height && iCol == props.size.width);
                break;
            case ENNBBitmapColor_BGRA8:
                for(iRow = 0; iRow < props.size.height && (colorsCountLimit == 0 || dst->use < colorsCountLimit); iRow++){
                    rowFirstByte        = &(pixels[iRow * props.bytesPerLine]);
                    curByteAtRow    = 0;
                    curBitAtByte        = 0;
                    for(iCol = 0; iCol < props.size.width && (colorsCountLimit == 0 || dst->use < colorsCountLimit); iCol++){
                        c.b = rowFirstByte[curByteAtRow++];
                        c.g = rowFirstByte[curByteAtRow++];
                        c.r = rowFirstByte[curByteAtRow++];
                        c.a = rowFirstByte[curByteAtRow++];
                        if(NBArraySorted_indexOf(dst, &c, sizeof(c), NULL) == -1){
                            NBArraySorted_addValue(dst, c);
                        }
                    }
                }
                r = (iRow == props.size.height && iCol == props.size.width);
                break;
            default:
                //Not implemented yet
                r = FALSE;
                //--------------------------------
                //-- Warning: usign "privInlineLeerPixel" proved to be expensive.
                //--------------------------------
                /*PRINTF_WARNING("Using expensive 'privInlineLeerPixel' method for pixelsPalette(color = %d).\n", props.color);
                 for(iRow = 0; iRow < props.size.height && (colorsCountLimit == 0 || p.use < colorsCountLimit); iRow++){
                 rowFirstByte    = &(pixels[iRow * props.bytesPerLine]);
                 curByteAtRow    = 0;
                 curBitAtByte    = 0;
                 for(iCol = 0; iCol < props.size.width && (colorsCountLimit == 0 || p.use < colorsCountLimit); iCol++){
                 this->privInlineLeerPixel((MapaBitsColor)_propiedades.color, rowFirstByte, curByteAtRow, curBitAtByte, c.r, c.g, c.b, c.a);
                 //
                 if(NBArraySorted_indexOf(&p, &c, sizeof(c), NULL) == -1){
                 NBArraySorted_addValue(&p, c);
                 }
                 }
                 }*/
                break;
        }
    }
    return r;
}

//Save state

typedef struct STNBPngSaveStateOpq_ {
	struct {
		BYTE* buff;
		UI32  buffSz;
	} interlaced;
	struct {
		BYTE* buff;
		UI32  buffSz;
	} compress;
} STNBPngSaveStateOpq;

void NBPngSaveState_init(STNBPngSaveState* obj, const UI32 compressBuffSz){
	STNBPngSaveStateOpq* opq = obj->opaque = NBMemory_allocType(STNBPngSaveStateOpq);
	NBMemory_setZeroSt(*opq, STNBPngSaveStateOpq);
	opq->interlaced.buff	= NULL;
	opq->interlaced.buffSz	= 0;
	opq->compress.buff		= NBMemory_alloc(compressBuffSz);
	opq->compress.buffSz	= compressBuffSz;
}

void NBPngSaveState_release(STNBPngSaveState* obj){
	STNBPngSaveStateOpq* opq = (STNBPngSaveStateOpq*)obj->opaque;
	if(opq != NULL){
		{
			if(opq->interlaced.buff != NULL){
				NBMemory_free(opq->interlaced.buff);
				opq->interlaced.buff = NULL;
			}
			opq->interlaced.buffSz = 0;
		}
		{
			if(opq->compress.buff != NULL){
				NBMemory_free(opq->compress.buff);
				opq->compress.buff = NULL;
			}
			opq->compress.buffSz = 0;
		}
		//
		NBMemory_free(obj->opaque);
		obj->opaque = NULL;
	}
}

//Load state

typedef struct STNBPngLoadStateOpq_ {
	//Estado de flujo de archivo
	STNBFileRef     archivo;
	STNBFileRef     lector;
	//PNG props
	BOOL			headerFound;
	BYTE			encabezado[8];
	BOOL			foundIHDR;
	BOOL			foundIEND;
	STNBPngIHDR		ihdr;
	ENNBBitmapColor	bmpColor;
	STNBBitmapProps	bmpProps;
	//Chunks
	struct {
		UI32		count;
		UI32		IDATCount;
		//Current loading
		struct {
			BOOL	isTypeFnd;
			BOOL	isLoaded;
			BOOL	isConsumed;
			char	type[5];
			struct {
				BYTE*	ptr;
				UI32	use;
				UI32	sz;
			} buff;
		} cur;
		//Buffs
		UI8*	PLTE;
		UI8*	PLTEData;
		UI32	PLTESz;
		UI8*	tRNS;
		UI8*	tRNSData;
		UI32	tRNSSz;
	} chunks;
	//Buffers
	struct {
		//Scanlines
		struct {
			UI32		bitsPerPix;			//Bits per pixel in sample buffer
			STNBSizeI	sampleSz;			//size of current sacanlines sample
			UI32 		sampleIdx;			//only 1 if non-interlaced, 7 if interlaced.
			UI32 		bytesLoaded;		//current byte filled from decompression
			STNBPngScanLinesBuffer* cur;	//current buffer
		} scanlines;
		//Pixels
		struct {
			BOOL		isCompleted;		//All scanlines samples completed
		} pixels;
	} buffs;
	//Zlib
	struct {
		STNBZInflate obj;			//
		BOOL		 isHungry;		//Need data, or still have data inside
	} zlibb;
} STNBPngLoadStateOpq;

void NBPngLoadState_init(STNBPngLoadState* obj){
	STNBPngLoadStateOpq* opq = obj->opaque = NBMemory_allocType(STNBPngLoadStateOpq);
	NBMemory_setZeroSt(*opq, STNBPngLoadStateOpq);
    opq->archivo                = NBFile_alloc(NULL);
    opq->lector                 = NBFile_alloc(NULL);
	opq->chunks.count		    = 0;
	opq->chunks.cur.isTypeFnd	= FALSE;
	opq->chunks.cur.isLoaded	= FALSE;
	opq->chunks.IDATCount	    = 0;
	//opq->chunks.cur.type;
	opq->chunks.cur.buff.ptr	= NULL;
	opq->chunks.cur.buff.sz		= 0;	//Tamano del buffer
	opq->chunks.cur.buff.use	= 0;	//Tamano de datos reales, siempre (chunks.cur.buff.use <= chunks.cur.buff.sz)
	opq->chunks.cur.isConsumed	= FALSE;
	opq->chunks.PLTE			= NULL;
	opq->chunks.PLTEData		= NULL;
	opq->chunks.PLTESz			= 0;
	opq->chunks.tRNS			= NULL;
	opq->chunks.tRNSData		= NULL;
	opq->chunks.tRNSSz			= 0;
	//Buffers
	{
		//Scanlines
		{
			opq->buffs.scanlines.bitsPerPix			= 0;
			opq->buffs.scanlines.sampleSz.width		= 0;
			opq->buffs.scanlines.sampleSz.height	= 0;
			opq->buffs.scanlines.sampleIdx			= 0;
			opq->buffs.scanlines.bytesLoaded		= 0;
			opq->buffs.scanlines.cur				= NULL;
		}
		//Pixels
		{
			opq->buffs.pixels.isCompleted			= FALSE;
		}
	}
	//Estado de descompresion
	NBZInflate_init(&opq->zlibb.obj);
	NBZInflate_feedStart(&opq->zlibb.obj);
	opq->zlibb.isHungry		= TRUE;
	//Propiedades del PNG
	opq->headerFound		= FALSE;
	//opq->encabezado		= {0, 0, 0, 0, 0, 0, 0, 0};
	opq->foundIHDR			= FALSE;
	opq->foundIEND			= FALSE;
	opq->ihdr.width			= 0;
	opq->ihdr.height		= 0;
	opq->ihdr.bitsDepth		= 0;
	opq->ihdr.pngColor		= 0;
	opq->ihdr.compMethd		= 0;
	opq->ihdr.filterMethd	= 0;
	opq->ihdr.interlMethd	= 0;
	opq->bmpColor			= ENNBBitmapColor_undef;
	opq->bmpProps.size.width	= 0;
	opq->bmpProps.size.height	= 0;
	opq->bmpProps.bitsPerPx 	= 0;
	opq->bmpProps.bytesPerLine = 0;
	opq->bmpProps.color = ENNBBitmapColor_undef;
}

void NBPngLoadState_release(STNBPngLoadState* obj){
	STNBPngLoadStateOpq* opq = (STNBPngLoadStateOpq*)obj->opaque;
	//Buffers
	{
		//Scanlines
		{
			if(opq->buffs.scanlines.cur != NULL){
				NBPngScanLinesBuffer_release(opq->buffs.scanlines.cur);
				NBMemory_free(opq->buffs.scanlines.cur);
				opq->buffs.scanlines.cur = NULL;
			}
		}
		//Pixels
		{
			//
		}
	}
	{
		if(opq->chunks.PLTE != NULL) NBMemory_free(opq->chunks.PLTE); opq->chunks.PLTE = NULL;
		if(opq->chunks.tRNS != NULL) NBMemory_free(opq->chunks.tRNS); opq->chunks.tRNS = NULL;
		NBZInflate_release(&opq->zlibb.obj);
		if(opq->chunks.cur.buff.ptr != NULL) NBMemory_free(opq->chunks.cur.buff.ptr); opq->chunks.cur.buff.ptr = NULL;
		/*if(opq->buffPrevRow != NULL) NBMemory_free(opq->buffPrevRow); opq->buffPrevRow = NULL;
		opq->buffPrevRow		= NULL;*/
		NBFile_release(&opq->lector);
		NBFile_release(&opq->archivo);
	}
	NBMemory_free(obj->opaque);
	obj->opaque = NULL;
}

//

STNBBitmapProps	NBPngLoadState_getProps(STNBPngLoadState* obj){
	STNBPngLoadStateOpq* opq = (STNBPngLoadStateOpq*)obj->opaque;
	return opq->bmpProps;
}

//

BOOL NBPng_dataStartsAsPng(const void* data, const UI32 dataSz){
	BOOL r = FALSE;
	if(data != NULL && dataSz >= 8){
		const BYTE* head = (const BYTE*)data;
		if(head[0] == 0x89 && head[1] == 0x50 && head[2] == 0x4E && head[3] == 0x47 && head[4] == 0x0D && head[5] == 0x0A && head[6] == 0x1A && head[7] == 0x0A){
			r = TRUE;
		}
	}
	return r;
}

// Save

BOOL NBPng_saveToFile(const STNBBitmap* bitmap, STNBFileRef file, const ENPngCompressLvl compresssLvl){
	BOOL r = FALSE;
	STNBPngSaveState state;
	NBPngSaveState_init(&state, 4 + (16 * 1024)); //+4 for the chunkID
	r = NBPng_saveToFileWithState(bitmap, file, compresssLvl, &state, 0);
	NBPngSaveState_release(&state);
	return r;
}

BOOL NBPng_saveToFileWithPalette(const STNBBitmap* bitmap, STNBFileRef file, const ENPngCompressLvl compresssLvl){
    BOOL r = FALSE;
    STNBPngSaveState state;
    NBPngSaveState_init(&state, 4 + (16 * 1024)); //+4 for the chunkID
    r = NBPng_saveToFileWithState(bitmap, file, compresssLvl, &state, 256);
    NBPngSaveState_release(&state);
    return r;
}

BOOL NBPng_saveDataToFile(const STNBBitmapProps* props, const void* pixels, STNBFileRef file, const ENPngCompressLvl compresssLvl){
	BOOL r = FALSE;
	STNBPngSaveState state;
	NBPngSaveState_init(&state, 4 + (16 * 1024)); //+4 for the chunkID
	r = NBPng_saveDataToFileWithState(props, pixels, file, compresssLvl, &state, 0);
	NBPngSaveState_release(&state);
	return r;
}

BOOL NBPng_saveToPath(const STNBBitmap* bitmap, const char* filePath, const ENPngCompressLvl compresssLvl){
	BOOL r = FALSE;
	STNBPngSaveState state;
	NBPngSaveState_init(&state, 4 + (16 * 1024)); //+4 for the chunkID
	r = NBPng_saveToPathWithState(bitmap, filePath, compresssLvl, &state);
	NBPngSaveState_release(&state);
	return r;
}

BOOL NBPng_saveToPathAtRoot(const STNBBitmap* bitmap, STNBFilesystem* fs, const ENNBFilesystemRoot root, const char* filePath, const ENPngCompressLvl compresssLvl){
	BOOL r = FALSE;
	STNBPngSaveState state;
	NBPngSaveState_init(&state, 4 + (16 * 1024)); //+4 for the chunkID
	r = NBPng_saveToPathAtRootWithState(bitmap, fs, root, filePath, compresssLvl, &state);
	NBPngSaveState_release(&state);
	return r;
}

UI32 NBPng_injectInterlaceByteAtLines(BYTE** bufferParaEntrelazado, UI32* tamBufferParaEntrelazado, const STNBBitmapProps* props, const UI8* pixels, ENNBPngColor pngColor, STNBArraySorted* plette) {
	UI8 bytesPerPx				= NBPNG_BYTES_PER_PIXEL(pngColor); NBASSERT(bytesPerPx != 0)
	UI32 tamanoDescomprimidoEntrelazado = (props->size.width * props->size.height * bytesPerPx) + props->size.height;
	if(*bufferParaEntrelazado == NULL || *tamBufferParaEntrelazado < tamanoDescomprimidoEntrelazado){
		if(*bufferParaEntrelazado != NULL) NBMemory_free(*bufferParaEntrelazado);
		*bufferParaEntrelazado 			= (UI8*)NBMemory_alloc(tamanoDescomprimidoEntrelazado);
		*tamBufferParaEntrelazado		= tamanoDescomprimidoEntrelazado;
	}
	UI8* datosBMPEntrelazado			= *bufferParaEntrelazado;
	UI32 posArrDest						= 0;
	//NBGestorMemoria::copiarMemoria(datosBMPEntrelazado, pixels, props->bytesPerLine * props->size.height);
	if((pngColor == ENNBPngColor_Gris && (props->color == ENNBBitmapColor_ALPHA8 || props->color == ENNBBitmapColor_GRIS8))
	   || (pngColor == ENNBPngColor_RGB && props->color == ENNBBitmapColor_RGB8)
	   || (pngColor == ENNBPngColor_GrisAlpha && props->color == ENNBBitmapColor_GRISALPHA8)
	   || (pngColor == ENNBPngColor_RGBA && props->color == ENNBBitmapColor_RGBA8)){
		//
		// Los formatos son compatibles.
		// Puede copiarse linea por linea (sin analizar los pixeles individuales).
		//
		UI32 fil, lineSz;
		for(fil = 0; fil < props->size.height; fil++){
			//byte de entrelazado (0 = none)
			datosBMPEntrelazado[posArrDest++] = 0; //0 == sin entrelazado
			//bytes de linea
			lineSz	= props->size.width * (props->bitsPerPx / 8);
			NBMemory_copy(&datosBMPEntrelazado[posArrDest], &pixels[props->bytesPerLine * fil], lineSz);
			posArrDest += lineSz;
		}
	} else {
		//
		// Los formatos son incompatibles.
		// Debe descomponerse cada pixel del formato origen y recomponerse en el formato destino.
		//
		UI32 fil, col/*, posArrOrig = 0*/;
		for(fil = 0; fil < props->size.height; fil++){
			//byte de entrelazado (0 = none)
			datosBMPEntrelazado[posArrDest++] = 0; //0 == sin entrelazado
			//mover el lector a la fila (en caso que deba)
			//posArrOrig = props->bytesPerLine * fil; //while(posArrOrig<(props->bytesPerLine * fil)) posArrOrig++;
			//
			const UI8* lineStart = &pixels[fil * props->bytesPerLine];
			UI16 byteAtLine = 0; UI8 bitAtByte = 0; STNBColor8 color;
			for(col = 0; col < props->size.width; col++){
                //
				NBBitmap_getPixelByCursor(props->color, lineStart, &byteAtLine, &bitAtByte, &color);
                //
				if(pngColor == ENNBPngColor_Gris){
					//GRIS
					datosBMPEntrelazado[posArrDest++] = (UI8)(((UI16)color.r + (UI16)color.g + (UI16)color.b) / 3);
				} else if(pngColor == ENNBPngColor_GrisAlpha) {
					//GRIS + ALPHA
					datosBMPEntrelazado[posArrDest++] = (UI8)(((UI16)color.r + (UI16)color.g + (UI16)color.b) / 3);
					datosBMPEntrelazado[posArrDest++] = color.a;
				} else if(pngColor == ENNBPngColor_RGB){
					//RGB
					datosBMPEntrelazado[posArrDest++] = color.r;
					datosBMPEntrelazado[posArrDest++] = color.g;
					datosBMPEntrelazado[posArrDest++] = color.b;
				} else if(pngColor == ENNBPngColor_RGBA){
					//RGBA
					datosBMPEntrelazado[posArrDest++] = color.r;
					datosBMPEntrelazado[posArrDest++] = color.g;
					datosBMPEntrelazado[posArrDest++] = color.b;
					datosBMPEntrelazado[posArrDest++] = color.a;
				} else if(pngColor == ENNBPngColor_RGBConPaleta){
					//PALETA RGB (el alpha puede ir en un chunck tRNS)
                    SI32 iColor = NBArraySorted_indexOf(plette, &color, sizeof(color), NULL);
                    if(iColor < 0){
                        //color not found, using idx-0
                        iColor = 0;
                    }
                    datosBMPEntrelazado[posArrDest++] = iColor;
				}
			}
		}
	}
	NBASSERT(tamanoDescomprimidoEntrelazado==posArrDest);
	return tamanoDescomprimidoEntrelazado;
}

BOOL NBPng_saveToPathWithState(const STNBBitmap* bitmap, const char* filePath, const ENPngCompressLvl compresssLvl, STNBPngSaveState* state){
	BOOL r = FALSE;
	STNBFileRef file = NBFile_alloc(NULL);
	if(!NBFile_open(file, filePath, ENNBFileMode_Write)){
		PRINTF_ERROR("Could not create file: '%s'\n", filePath);
	} else {
		r = NBPng_saveToFileWithState(bitmap, file, compresssLvl, state, 0);
		NBFile_close(file);
	}
	NBFile_release(&file);
	return r;
}

BOOL NBPng_saveToPathAtRootWithState(const STNBBitmap* bitmap, STNBFilesystem* fs, const ENNBFilesystemRoot root, const char* filePath, const ENPngCompressLvl compresssLvl, STNBPngSaveState* state){
	BOOL r = FALSE;
	if(fs != NULL && root < ENNBFilesystemRoot_Count){
		STNBFileRef file = NBFile_alloc(NULL);
		if(!NBFilesystem_openAtRoot(fs, root, filePath, ENNBFileMode_Write, file)){
			PRINTF_ERROR("Could not create file at root: '%s'\n", filePath);
		} else {
			r = NBPng_saveToFileWithState(bitmap, file, compresssLvl, state, 0);
			NBFile_close(file);
		}
		NBFile_release(&file);
	}
	return r;
}

BOOL NBPng_saveToFileWithState(const STNBBitmap* bitmap, STNBFileRef file, const ENPngCompressLvl compresssLvl, STNBPngSaveState* state, const UI32 paletteMaxSz){
	const STNBBitmapProps props = NBBitmap_getProps(bitmap);
	const BYTE* pixels			= NBBitmap_getData(bitmap);
	return NBPng_saveDataToFileWithState(&props, pixels, file, compresssLvl, state, paletteMaxSz);
}

BOOL NBPng_saveDataToFileWithState(const STNBBitmapProps* props, const void* pPixels, STNBFileRef file, const ENPngCompressLvl compresssLvl, STNBPngSaveState* state, const UI32 paletteMaxSz){
	BOOL r = FALSE;
	if(props != NULL && pPixels != NULL){
		//--------------------
		// PNG SPECS
		//--------------------
		// COLOR   BITS_DEPTH   DESCRIPTION
		// 0       1, 2, 4, 8, 16     Gris
		// 2       8, 16              RGB
		// 3       1, 2, 4, 8         Paleta RGB (el alpha puede incluirse en un chunck tRNS)
		// 4       8, 16              Gris con alpha
		// 6       8, 16              RGBA
		//--------------------
		const BYTE* pixels			= (const BYTE*)pPixels;
		//
		BOOL noError				= TRUE;
		BOOL colorRGB				= (props->color == ENNBBitmapColor_RGB4 || props->color == ENNBBitmapColor_RGB8 || props->color == ENNBBitmapColor_RGBA4 || props->color == ENNBBitmapColor_RGBA8 || props->color == ENNBBitmapColor_ARGB4 || props->color == ENNBBitmapColor_ARGB8 || props->color == ENNBBitmapColor_BGRA8 || props->color == ENNBBitmapColor_SWF_PIX15 || props->color == ENNBBitmapColor_SWF_PIX24)  ? TRUE : FALSE;
		BOOL incluirAlpha			= (props->color == ENNBBitmapColor_GRISALPHA8 || props->color == ENNBBitmapColor_RGBA4 || props->color == ENNBBitmapColor_RGBA8 || props->color == ENNBBitmapColor_ARGB4 || props->color == ENNBBitmapColor_ARGB8 || props->color == ENNBBitmapColor_BGRA8) ? TRUE : FALSE;
        //
        STNBArraySorted plette;
        NBArraySorted_initWithSz(&plette, sizeof(STNBColor8), NBCompare_NBColor8, 0, 256, 0.1f);
        if(paletteMaxSz > 0 && paletteMaxSz <= 256 && !NBPng_calculatePaletteOnDataTo(*props, pixels, &plette, paletteMaxSz)){
            //error, using palette and does not fit
            r = FALSE;
        } else {
            /*if(pixsDesc != NULL){
             colorRGB				= ((pixsDesc->mask & NB_BITMAP_PIXS_DESC_BIT_RGB) != 0);
             incluirAlpha			= ((pixsDesc->mask & NB_BITMAP_PIXS_DESC_BIT_TRANSP) != 0);
             }*/
            //
            STNBPngSaveStateOpq* opq = (STNBPngSaveStateOpq*)state->opaque;
            ENNBPngColor pngColor   = ENNBPngColor_Error; //Se asume: bitsDepthPNG = 8
            if(colorRGB){
                if(plette.use > 0){
                    pngColor        = ENNBPngColor_RGBConPaleta;
                } else {
                    pngColor        = (incluirAlpha ? ENNBPngColor_RGBA : ENNBPngColor_RGB);
                }
            } else {
                pngColor			= (incluirAlpha ? ENNBPngColor_GrisAlpha : ENNBPngColor_Gris);
            }
            //
            if(pngColor == ENNBPngColor_Error){
                PRINTF_ERROR("Could not determine the PngColor(%d).\n", pngColor);
            } else {
                //PRINTF_INFO("Guardando PNG: pngColor(%d) bitsDepthPNG(%d) bytesPerPx(%d)\n", pngColor, bitsDepthPNG, bytesPerPx);
                //VOLCAR A ARCHIVO PNG
                if(noError){
                    if(NBFile_isOpen(file)){
                        NBFile_lock(file);
                        //NBFile_write(file, &datosBMPEntrelazadoComprimido[-4], (4 + tamanoEntrelazadoComprimido));
                        //PRINTF_INFO("Archivo creado: '%s'\n", nombreArchivo);
                        //ENCABEZADO DEL ARCHIVO PNG
                        UI8 encabezadoPNG[8];
                        encabezadoPNG[0] = 0x89; encabezadoPNG[1] = 0x50; encabezadoPNG[2] = 0x4E; encabezadoPNG[3] = 0x47;
                        encabezadoPNG[4] = 0x0D; encabezadoPNG[5] = 0x0A; encabezadoPNG[6] = 0x1A; encabezadoPNG[7] = 0x0A;
                        NBFile_write(file, encabezadoPNG, 8);
                        //CHUNCK - HEADER DEL PNG
                        UI8 header[17];
                        header[0]				= 'I';
                        header[1]				= 'H';
                        header[2]				= 'D';
                        header[3]				= 'R';
                        UI32 invWidth			= props->size.width; _invUI32(&invWidth);
                        UI32 invHeight			= props->size.height; _invUI32(&invHeight);
                        *((UI32*)(&header[4]))	= invWidth;
                        *((UI32*)(&header[8]))	= invHeight;
                        header[12]				= 8; //bitsDepth (4 u 8)
                        header[13]				= pngColor; //pngColor
                        header[14]				= 0; //compMethd
                        header[15]				= 0; //filterMethd
                        header[16]				= 0; //interlMethd
                        UI32 tamanoDatos		= 13; _invUI32(&tamanoDatos);
                        UI32 esteCRC			= NBCrc32_getHashBytes(header, 17); _invUI32(&esteCRC);
                        NBFile_write(file, &tamanoDatos, sizeof(tamanoDatos));
                        NBFile_write(file, &header, 17);
                        NBFile_write(file, &esteCRC, sizeof(esteCRC));
                        //CHUNCK - PALETA DEL PNG
                        if(plette.use > 0 && pngColor == ENNBPngColor_RGBConPaleta){
                            BYTE* chunckPaleta = (BYTE*)NBMemory_alloc(4 + (plette.use * 3));
                            chunckPaleta[0] = 'P';
                            chunckPaleta[1] = 'L';
                            chunckPaleta[2] = 'T';
                            chunckPaleta[3] = 'E';
                            {
                                int i; for(i = 0; i < plette.use; i++){
                                    STNBColor8* c         = NBArraySorted_itmPtrAtIndex(&plette, STNBColor8, i);
                                    chunckPaleta[4 + (i * 3)]        = c->r;
                                    chunckPaleta[4 + (i * 3) + 1]    = c->g;
                                    chunckPaleta[4 + (i * 3) + 2]    = c->b;
                                }
                            }
                            tamanoDatos = plette.use * 3; _invUI32(&tamanoDatos);
                            esteCRC		= NBCrc32_getHashBytes(chunckPaleta, (int)(4 + (plette.use * 3))); _invUI32(&esteCRC);
                            NBFile_write(file, &tamanoDatos, sizeof(tamanoDatos));
                            NBFile_write(file, chunckPaleta, (4 + (plette.use * 3)));
                            NBFile_write(file, &esteCRC, sizeof(esteCRC));
                            NBMemory_free(chunckPaleta);
                        }
                        //CHUNCK 'tRNS'
                        if(plette.use > 0 && pngColor == ENNBPngColor_RGBConPaleta && incluirAlpha){
                            BYTE* chunckTRNS = (BYTE*)NBMemory_alloc(4 + (plette.use));
                            chunckTRNS[0] = 't';
                            chunckTRNS[1] = 'R';
                            chunckTRNS[2] = 'N';
                            chunckTRNS[3] = 'S';
                            {
                                int i; for(i = 0; i < plette.use; i++){
                                    STNBColor8* c       = NBArraySorted_itmPtrAtIndex(&plette, STNBColor8, i);
                                    chunckTRNS[4 + i]   = c->a;
                                }
                            }
                            tamanoDatos = plette.use; _invUI32(&tamanoDatos);
                            esteCRC		= NBCrc32_getHashBytes(chunckTRNS, (int)(4 + (plette.use))); _invUI32(&esteCRC);
                            NBFile_write(file, &tamanoDatos, sizeof(tamanoDatos));
                            NBFile_write(file, chunckTRNS, (4 + (plette.use)));
                            NBFile_write(file, &esteCRC, sizeof(esteCRC));
                            NBMemory_free(chunckTRNS);
                        }
                        //CHUNCK - FIRMA SHA1 DEL MAPA DE BITS
                        /*if(incluirFirmaSHA1){
                         BYTE chunckSHA1[24];
                         chunckSHA1[0]			= 's';	//0 (uppercase) = critical, 1 (lowercase) = ancillary.
                         chunckSHA1[1]			= 'h';	//0 (uppercase) = public, 1 (lowercase) = private.
                         chunckSHA1[2]			= 'A';	//Must be 0 (uppercase) in files conforming to this version of PNG.
                         chunckSHA1[3]			= 'A';	//0 (uppercase) = unsafe to copy, 1 (lowercase) = safe to copy
                         STNBSha1Hash firmaSHA1	= this->firmaSHA1();
                         BYTE* punteroSHA1		= (BYTE*)&firmaSHA1;
                         SI32 iByteSha1;
                         for(iByteSha1 = 0; iByteSha1<20; iByteSha1++){
                         chunckSHA1[4+iByteSha1] = punteroSHA1[iByteSha1];
                         }
                         tamanoDatos				= 20; _invUI32(&tamanoDatos);
                         esteCRC					= NBCrc32_getHashBytes(chunckSHA1, 24); _invUI32(&esteCRC);
                         NBFile_write(file, &tamanoDatos, sizeof(tamanoDatos));
                         NBFile_write(file, chunckSHA1, 24);
                         NBFile_write(file, &esteCRC, sizeof(esteCRC));
                         }*/
                        //CHUNCKS - extras (before image data?)
                        /*if(extraChuncks != NULL){
                         SI32 i; const SI32 count = extraChuncks->countOfChuncks();
                         for(i = 0; i < count; i++){
                         const AUCadenaLarga8* chunck = extraChuncks->getChunckAtIndex(i);
                         if(chunck->tamano() > 4){
                         tamanoDatos				= chunck->tamano() - 4; _invUI32(&tamanoDatos);
                         esteCRC					= NBCrc32_getHashBytes((const BYTE*)chunck->str(), chunck->tamano()); _invUI32(&esteCRC);
                         NBFile_write(file, &tamanoDatos, sizeof(tamanoDatos));
                         NBFile_write(file, chunck->str(), chunck->tamano());
                         NBFile_write(file, &esteCRC, sizeof(esteCRC));
                         }
                         }
                         }*/
                        //CHUNCK - DATOS DEL PNG (en bloques de 16KB cada uno)
                        /*
                         datosBMPEntrelazadoComprimido[-4] = 'I';
                         datosBMPEntrelazadoComprimido[-3] = 'D';
                         datosBMPEntrelazadoComprimido[-2] = 'A';
                         datosBMPEntrelazadoComprimido[-1] = 'T';
                         tamanoDatos = tamanoEntrelazadoComprimido; _invUI32(&tamanoDatos);
                         esteCRC		= NBCrc32_getHashBytes(&datosBMPEntrelazadoComprimido[-4], (int)(4 + tamanoEntrelazadoComprimido)) ; _invUI32(&esteCRC);
                         NBFile_write(file, &tamanoDatos, sizeof(tamanoDatos));
                         NBFile_write(file, &datosBMPEntrelazadoComprimido[-4], (4 + tamanoEntrelazadoComprimido));
                         NBFile_write(file, &esteCRC, sizeof(esteCRC));*/
                        {
                            //se ocupan los primeros 4 bytes para el ID de chunck (recomiendo tamano + 4)
                            NBASSERT(opq->compress.buffSz > 4)
                            opq->compress.buff[0]			= 'I'; opq->compress.buff[1] = 'D'; opq->compress.buff[2] = 'A'; opq->compress.buff[3] = 'T';
                            //Definir el subespacio del buffer que se utilizará
                            BYTE* destinoBloqueZ		= &opq->compress.buff[4];
                            const UI32 tamBloqueZ		= opq->compress.buffSz - 4; NBASSERT((tamBloqueZ % 1024) == 0) //Debe ser multiplo de KB
                            UI32 posBloqueZ				= 0;
                            //
                            UI32 compressTotalProcesado = 0;
                            UI32 compressTotalResultado = 0;
                            //
                            STNBZDeflate zlib;
                            NBZDeflate_init(&zlib);
                            if(!NBZDeflate_feedStart(&zlib, compresssLvl)){
                                PRINTF_ERROR("Png, NBZDeflate_feedStart returned FALSE.\n");
                            } else {
                                if((pngColor == ENNBPngColor_Gris && (props->color == ENNBBitmapColor_ALPHA8 || props->color == ENNBBitmapColor_GRIS8))
                                   || (pngColor == ENNBPngColor_RGB && props->color == ENNBBitmapColor_RGB8)
                                   || (pngColor == ENNBPngColor_GrisAlpha && props->color == ENNBBitmapColor_GRISALPHA8)
                                   || (pngColor == ENNBPngColor_RGBA && props->color == ENNBBitmapColor_RGBA8)){
                                    //---------------------------------
                                    // Los formatos PNG-BMP son compatibles.
                                    // Puede procesar linea por linea
                                    // (sin analizar los pixeles individuales).
                                    //---------------------------------
                                    BOOL lineHeadAdded	= FALSE;
                                    SI32 lineCurByte	= 0;
                                    const SI32 lineSz	= props->size.width * (props->bitsPerPx / 8);
                                    //
                                    UI32 fil;
                                    for(fil = 0; fil < props->size.height; fil++){
                                        //-------------------------------
                                        // Process interlaced line header (1 byte)
                                        //-------------------------------
                                        if(!lineHeadAdded){
                                            const BYTE lineHead		= 0;
                                            const ENZDefResult zr	= NBZDeflate_feed(&zlib, &destinoBloqueZ[posBloqueZ], (tamBloqueZ - posBloqueZ), &lineHead, sizeof(lineHead), ENZDefBlckType_Partial);
                                            if(zr.resultCode >= 0){
                                                //PRINTF_INFO("LINE-head-ezresult: OK (%d) in(%d of 1) out(%d of %d).\n", zr.resultCode, zr.inBytesProcessed, zr.outBytesProcessed, (tamBloqueZ - posBloqueZ));
                                            } else {
                                                PRINTF_ERROR("LINE-head-ezresult: %s(%d) in(%d of 1) out(%d of %d).\n", "ZLIB_ERROR", zr.resultCode, zr.inBytesProcessed, zr.outBytesProcessed, (tamBloqueZ - posBloqueZ));
                                                NBASSERT(zr.resultCode >= 0)
                                                noError = FALSE;
                                                break;
                                            }
                                            //PRINTF_INFO("No-interlaced compress (result: %d, in: %d, out: %d) [header].\n", zr.resultCode, zr.inBytesProcessed, zr.outBytesProcessed);
                                            NBASSERT(zr.inBytesProcessed <= 1)
                                            lineHeadAdded			= (zr.inBytesProcessed == 1);
                                            compressTotalProcesado	+= zr.inBytesProcessed;
                                            posBloqueZ				+= zr.outBytesProcessed; NBASSERT(posBloqueZ <= tamBloqueZ)
                                            //Flush compressed data
                                            if(posBloqueZ == tamBloqueZ){
                                                NBASSERT(opq->compress.buff[0] == 'I' && opq->compress.buff[1] == 'D' && opq->compress.buff[2] == 'A' && opq->compress.buff[3] == 'T')
                                                tamanoDatos = posBloqueZ; _invUI32(&tamanoDatos);
                                                esteCRC		= NBCrc32_getHashBytes(opq->compress.buff, (int)(4 + posBloqueZ)); _invUI32(&esteCRC);
                                                NBFile_write(file, &tamanoDatos, sizeof(tamanoDatos));
                                                NBFile_write(file, opq->compress.buff, (4 + posBloqueZ));
                                                NBFile_write(file, &esteCRC, sizeof(esteCRC));
                                                compressTotalResultado += posBloqueZ;
                                                posBloqueZ = 0;
                                            }
                                        }
                                        //-------------------------------
                                        // Process line data
                                        //-------------------------------
                                        if(lineHeadAdded){
                                            const BYTE* line		= &pixels[props->bytesPerLine * fil];
                                            const ENZDefResult zr	= NBZDeflate_feed(&zlib, &destinoBloqueZ[posBloqueZ], (tamBloqueZ - posBloqueZ), &line[lineCurByte], (lineSz - lineCurByte), ENZDefBlckType_Partial);
                                            if(zr.resultCode >= 0){
                                                //PRINTF_INFO("LINE-data-ezresult: OK (%d) in(%d of %d) out(%d of %d).\n", zr.resultCode, zr.inBytesProcessed, (lineSz - lineCurByte), zr.outBytesProcessed, (tamBloqueZ - posBloqueZ));
                                            } else {
                                                PRINTF_ERROR("LINE-data-ezresult: %s(%d) in(%d of %d) out(%d of %d).\n", "ZLIB_ERROR", zr.resultCode, zr.inBytesProcessed, (lineSz - lineCurByte), zr.outBytesProcessed, (tamBloqueZ - posBloqueZ));
                                                NBASSERT(zr.resultCode >= 0)
                                                noError = FALSE;
                                                break;
                                            }
                                            //PRINTF_INFO("No-interlaced compress (result: %d, in: %d, out: %d).\n", zr.resultCode, zr.inBytesProcessed, zr.outBytesProcessed);
                                            lineCurByte				+= zr.inBytesProcessed; NBASSERT(lineCurByte <= lineSz)
                                            compressTotalProcesado	+= zr.inBytesProcessed; NBASSERT(compressTotalProcesado <= (props->size.width * props->size.height *  NBPNG_BYTES_PER_PIXEL(pngColor)) + props->size.height)
                                            posBloqueZ				+= zr.outBytesProcessed; NBASSERT(posBloqueZ <= tamBloqueZ)
                                            //Flush compressed data
                                            if(posBloqueZ == tamBloqueZ){
                                                NBASSERT(opq->compress.buff[0] == 'I' && opq->compress.buff[1] == 'D' && opq->compress.buff[2] == 'A' && opq->compress.buff[3] == 'T')
                                                tamanoDatos = posBloqueZ; _invUI32(&tamanoDatos);
                                                esteCRC		= NBCrc32_getHashBytes(opq->compress.buff, (int)(4 + posBloqueZ)); _invUI32(&esteCRC);
                                                NBFile_write(file, &tamanoDatos, sizeof(tamanoDatos));
                                                NBFile_write(file, opq->compress.buff, (4 + posBloqueZ));
                                                NBFile_write(file, &esteCRC, sizeof(esteCRC));
                                                compressTotalResultado += posBloqueZ;
                                                posBloqueZ = 0;
                                            }
                                        }
                                        //Next step
                                        NBASSERT(lineCurByte <= lineSz)
                                        if(lineCurByte < lineSz){
                                            //Repeat line
                                            fil--;
                                        } else {
                                            //Next line
                                            lineHeadAdded = FALSE;
                                            lineCurByte = 0;
                                        }
                                    }
                                    NBASSERT(compressTotalProcesado <= (props->size.width * props->size.height * NBPNG_BYTES_PER_PIXEL(pngColor)) + props->size.height)
                                    //PRINTF_INFO("FIN DE CICLO!\n");
                                } else {
                                    //---------------------------------
                                    // Los formatos PNG-BMP son incompatibles.
                                    // Se deben procesar los pixeles individualmente.
                                    //---------------------------------
                                    //ENTRELAZAR DATOS
                                    UI32 tamanoDescomprimidoEntrelazado = 0;
                                    UI8* datosBMPEntrelazado 			= NULL;
                                    if(noError){
                                        tamanoDescomprimidoEntrelazado 	= NBPng_injectInterlaceByteAtLines(&opq->interlaced.buff, &opq->interlaced.buffSz, props, pixels, pngColor, &plette);
                                        datosBMPEntrelazado				= opq->interlaced.buff;
                                    }
                                    //Comprimir en bloques de tamano fijos
                                    //Compresion de contenido (ezBlockType_NoFinal)
                                    while(compressTotalProcesado < tamanoDescomprimidoEntrelazado) {
                                        NBASSERT(posBloqueZ < tamBloqueZ)
                                        const ENZDefResult zr = NBZDeflate_feed(&zlib, &destinoBloqueZ[posBloqueZ], (tamBloqueZ - posBloqueZ), &datosBMPEntrelazado[compressTotalProcesado], (tamanoDescomprimidoEntrelazado - compressTotalProcesado), ENZDefBlckType_Partial);
                                        if(zr.resultCode < 0){
                                            PRINTF_ERROR("ezStreamCompressProcess_ retorno (%d).\n", zr.resultCode);
                                            noError = FALSE;
                                            NBASSERT(zr.resultCode >= 0)
                                            break;
                                        } else {
                                            //PRINTF_INFO("Preinterlaced compress (result: %d, in: %d, out: %d).\n", zr.resultCode, zr.inBytesProcessed, zr.outBytesProcessed);
                                            posBloqueZ				+= zr.outBytesProcessed; NBASSERT(posBloqueZ <= tamBloqueZ)
                                            compressTotalProcesado	+= zr.inBytesProcessed; NBASSERT(compressTotalProcesado <= tamanoDescomprimidoEntrelazado)
                                            if(posBloqueZ == tamBloqueZ){
                                                NBASSERT(opq->compress.buff[0] == 'I' && opq->compress.buff[1] == 'D' && opq->compress.buff[2] == 'A' && opq->compress.buff[3] == 'T')
                                                tamanoDatos = posBloqueZ; _invUI32(&tamanoDatos);
                                                esteCRC		= NBCrc32_getHashBytes(opq->compress.buff, (int)(4 + posBloqueZ)); _invUI32(&esteCRC);
                                                NBFile_write(file, &tamanoDatos, sizeof(tamanoDatos));
                                                NBFile_write(file, opq->compress.buff, (4 + posBloqueZ));
                                                NBFile_write(file, &esteCRC, sizeof(esteCRC));
                                                compressTotalResultado	+= posBloqueZ;
                                                posBloqueZ	= 0;
                                            }
                                        }
                                    }
                                }
                                //Flush compression buffer (ezBlockType_Final until 'Z_STREAM_END' is returned)
                                if(noError){
                                    do {
                                        NBASSERT(posBloqueZ < tamBloqueZ)
                                        const ENZDefResult zr = NBZDeflate_feed(&zlib, &destinoBloqueZ[posBloqueZ], (tamBloqueZ - posBloqueZ), NULL, 0, ENZDefBlckType_Final);
                                        if(zr.resultCode < 0){
                                            PRINTF_ERROR("ezStreamCompressProcess_(Final) returned: %d.\n", zr.resultCode);
                                            noError = FALSE;
                                            NBASSERT(zr.resultCode >= 0)
                                            break;
                                        } else {
                                            //PRINTF_INFO("Final compress (result: %d, in: %d, out: %d).\n", zr.resultCode, zr.inBytesProcessed, zr.outBytesProcessed);
                                            //Some compressed output
                                            posBloqueZ				+= zr.outBytesProcessed; NBASSERT(posBloqueZ <= tamBloqueZ)
                                            compressTotalProcesado	+= zr.inBytesProcessed; NBASSERT(compressTotalProcesado <= (props->size.width * props->size.height * NBPNG_BYTES_PER_PIXEL(pngColor)) + props->size.height)
                                            if(posBloqueZ != 0){
                                                NBASSERT(opq->compress.buff[0] == 'I' && opq->compress.buff[1] == 'D' && opq->compress.buff[2] == 'A' && opq->compress.buff[3] == 'T')
                                                tamanoDatos = posBloqueZ; _invUI32(&tamanoDatos);
                                                esteCRC		= NBCrc32_getHashBytes(opq->compress.buff, (int)(4 + posBloqueZ)); _invUI32(&esteCRC);
                                                NBFile_write(file, &tamanoDatos, sizeof(tamanoDatos));
                                                NBFile_write(file, opq->compress.buff, (4 + posBloqueZ));
                                                NBFile_write(file, &esteCRC, sizeof(esteCRC));
                                                compressTotalResultado += posBloqueZ;
                                                posBloqueZ	= 0;
                                            }
                                            //
                                            if(zr.resultCode == 1 /*Z_STREAM_END*/){
                                                break;
                                            }
                                        }
                                    } while(TRUE);
                                    //PRINTF_INFO("%d bytes comprimidos a %d bytes (%d%% en %.4f bloques de %dKB): '%s'.\n", compressTotalProcesado, compressTotalResultado, (100 * compressTotalResultado / compressTotalProcesado), ((float)compressTotalResultado / (float)tamPorBloque), (tamPorBloque / 1024), rutaArchivo);
                                    NBASSERT(compressTotalProcesado == (props->size.width * props->size.height * NBPNG_BYTES_PER_PIXEL(pngColor)) + props->size.height)
                                    //if(guardarCantidadBytesDatosComprimidosEn != NULL) *guardarCantidadBytesDatosComprimidosEn = *guardarCantidadBytesDatosComprimidosEn + compressTotalResultado;
                                }
                            }
                            NBZDeflate_release(&zlib);
                        }
                        //ULTIMO HEADER/CHUNCK PNG
                        if(noError){
                            BYTE chunckIEND[4];
                            chunckIEND[0] = 'I'; chunckIEND[1] = 'E'; chunckIEND[2] = 'N'; chunckIEND[3] = 'D';
                            tamanoDatos		= 0; _invUI32(&tamanoDatos);
                            esteCRC			= NBCrc32_getHashBytes(chunckIEND, 4); _invUI32(&esteCRC);
                            NBFile_write(file, &tamanoDatos, sizeof(tamanoDatos));
                            NBFile_write(file, chunckIEND, 4);
                            NBFile_write(file, &esteCRC, sizeof(esteCRC));
                            //
                            r = TRUE;
                        }
                        //
                        NBFile_unlock(file);
                    }
                }
            }
        }
        NBArraySorted_release(&plette);
	}
	return r;
}

//Load

BOOL NBPng_consumeInterlacedOpq(STNBPngLoadStateOpq* opq, BYTE* pixsBuff, const UI32 pixsBuffSz);
	
BOOL NBPng_loadFromPath(const char* rutaArchivo, const BOOL cargarDatos, STNBBitmap* dstBitmap, STNBArray* dstExtraChunks/*, STNBSha1Hash* guardarFirmaSHA1En, PTRFuncGetPngBlock funcConsumeExtraBlocks, void* paramConsumeExtraBlocks*/){
	BOOL r = FALSE;
	STNBFileRef file = NBFile_alloc(NULL);
	if(NBFile_open(file, rutaArchivo, ENNBFileMode_Read)){
		NBFile_lock(file);
		r = NBPng_loadFromFileWithIHDR(file, cargarDatos, dstBitmap, dstExtraChunks, NULL, NULL);
		NBFile_unlock(file);
		NBFile_close(file);
	}
	NBFile_release(&file);
	return r;
}

BOOL NBPng_loadFromPathWithIHDR(const char* rutaArchivo, const BOOL cargarDatos, STNBBitmap* dstBitmap, STNBArray* dstExtraChunks, STNBPngIHDR* dstIHDR, STNBPngIHDRCustom* dstHeadCustom/*, STNBSha1Hash* guardarFirmaSHA1En, PTRFuncGetPngBlock funcConsumeExtraBlocks, void* paramConsumeExtraBlocks*/){
	BOOL r = FALSE;
	STNBFileRef file = NBFile_alloc(NULL);
	if(NBFile_open(file, rutaArchivo, ENNBFileMode_Read)){
		NBFile_lock(file);
		r = NBPng_loadFromFileWithIHDR(file, cargarDatos, dstBitmap, dstExtraChunks, dstIHDR, dstHeadCustom);
		NBFile_unlock(file);
		NBFile_close(file);
	}
	NBFile_release(&file);
	return r;
}

BOOL NBPng_loadFromFile(STNBFileRef file, const BOOL cargarDatos, STNBBitmap* dstBitmap, STNBArray* dstExtraChunks/*, STNBSha1Hash* guardarFirmaSHA1En, PTRFuncGetPngBlock funcConsumeExtraBlocks, void* paramConsumeExtraBlocks*/){
	return NBPng_loadFromFileWithIHDR(file, cargarDatos, dstBitmap, dstExtraChunks, NULL, NULL);
}

BOOL NBPng_loadFromFileWithIHDR(STNBFileRef file, const BOOL cargarDatos, STNBBitmap* dstBitmap, STNBArray* dstExtraChunks, STNBPngIHDR* dstIHDR, STNBPngIHDRCustom* dstHeadCustom/*, STNBSha1Hash* guardarFirmaSHA1En, PTRFuncGetPngBlock funcConsumeExtraBlocks, void* paramConsumeExtraBlocks*/){
	BOOL r = FALSE;
	STNBPngLoadState eLoad;
	NBPngLoadState_init(&eLoad);
	{
		STNBPngLoadStateOpq* opq = (STNBPngLoadStateOpq*)eLoad.opaque;
		//Swap state's file obj for param-file
		STNBFileRef stateFile = opq->archivo;
		opq->archivo = file;
		//Load
		{
			//First load withouf data buffers (will only read the header)
			if(!NBPng_loadWithState(&eLoad, NULL, 0, dstExtraChunks, dstIHDR, dstHeadCustom/*, guardarFirmaSHA1En, funcConsumeExtraBlocks, paramConsumeExtraBlocks*/)){
				//PRINTF_ERROR("Could not load the PNG's header.\n"); //NBASSERT(FALSE)
			} else {
				if(!cargarDatos){
					NBBitmap_createWithoutData(dstBitmap, opq->bmpProps.size.width, opq->bmpProps.size.height, opq->bmpProps.color);
					//this->privCrearMapaBits(opq->width, opq->height, opq->bmpColor, FALSE, FALSE, 0);
					r = TRUE;
				} else {
					NBBitmap_create(dstBitmap, opq->bmpProps.size.width, opq->bmpProps.size.height, opq->bmpProps.color);
					//Load pixels
					{
						const STNBBitmapProps bmpProps	= NBBitmap_getProps(dstBitmap);
						const UI32 bmpDatosSz	= (bmpProps.bytesPerLine * bmpProps.size.height);
						BYTE* bmpData			= NBBitmap_getData(dstBitmap);
						while(!opq->foundIEND){
							if(!NBPng_loadWithState(&eLoad, bmpData, bmpDatosSz, dstExtraChunks, dstIHDR, dstHeadCustom/*, NULL, funcConsumeExtraBlocks, paramConsumeExtraBlocks*/)){
								break;
							}
						}
					}
					//Result
					if(opq->buffs.pixels.isCompleted){
						r = TRUE;
					}
				}
			}
		}
		//Return state's file obj from param-file (before release)
		opq->archivo = stateFile;
	}
	NBPngLoadState_release(&eLoad);
	return r;
}

BOOL NBPng_loadStreamSetItf(STNBPngLoadState* eLoad, IFileItf* itf, void* itfObj){
	BOOL r = FALSE;
	STNBPngLoadStateOpq* opq = (STNBPngLoadStateOpq*)eLoad->opaque;
	if(!NBFile_openAsItf(opq->archivo, itf, itfObj)){
		//
	} else {
		r = TRUE;
	}
	return r;
}

void NBPng_loadStreamLock(STNBPngLoadState* eLoad){
	STNBPngLoadStateOpq* opq = (STNBPngLoadStateOpq*)eLoad->opaque;
	NBFile_lock(opq->archivo);
}

void NBPng_loadStreamUnlock(STNBPngLoadState* eLoad){
	STNBPngLoadStateOpq* opq = (STNBPngLoadStateOpq*)eLoad->opaque;
	NBFile_unlock(opq->archivo);
}

BOOL NBPng_loadWithState(STNBPngLoadState* eLoad, BYTE* pixsBuff, const UI32 pixsBuffSz, STNBArray* dstExtraChunks, STNBPngIHDR* dstIHDR, STNBPngIHDRCustom* dstHeadCustom/*, STNBSha1Hash* guardarFirmaSHA1En, PTRFuncGetPngBlock funcConsumeExtraBlocks, void* paramConsumeExtraBlocks*/){
	BOOL exito = FALSE;
	STNBPngLoadStateOpq* opq = (STNBPngLoadStateOpq*)eLoad->opaque;
	NBFile_lock(opq->lector);
	{
		if(!opq->headerFound){
			NBASSERT(NBFile_curPos(opq->archivo) >= 0)
			if(NBFile_read(opq->archivo, opq->encabezado, 8) != 8){
				PRINTF_ERROR("PNG load, could not read the first 8 bytes.\n");
				NBMemory_setZero(opq->encabezado);
			} else {
				opq->headerFound = TRUE;
			}
		}
		if(!(opq->encabezado[0] == 0x89 && opq->encabezado[1] == 0x50 && opq->encabezado[2] == 0x4E && opq->encabezado[3] == 0x47 && opq->encabezado[4] == 0x0D && opq->encabezado[5] == 0x0A && opq->encabezado[6] == 0x1A && opq->encabezado[7] == 0x0A)){
			PRINTF_ERROR("PNG load, first 8 bytes does not match.\n");
		} else {
			//Lectura de chunks
			BOOL continuarCiclo		= TRUE;
			do {
				//---------------------------------
				//-- Leer tipo de chunck (primeros 4 bytes)
				//---------------------------------
				if(!opq->chunks.cur.isTypeFnd){
					NBASSERT(NBFile_curPos(opq->archivo) >= 8)
					//Read chunk size
					opq->chunks.cur.buff.use = 0;
					if(NBFile_read(opq->archivo, &opq->chunks.cur.buff.use, sizeof(opq->chunks.cur.buff.use)) != sizeof(opq->chunks.cur.buff.use)){
						PRINTF_ERROR("PNG load, could not read the chunk-sz.\n");
						continuarCiclo = FALSE;
					} else {
						_invUI32(&opq->chunks.cur.buff.use);
						//Read chunk type
						if(NBFile_read(opq->archivo, opq->chunks.cur.type, 4) != 4){
							PRINTF_ERROR("PNG load, could not read the chunk-type.\n");
							continuarCiclo = FALSE;
						} else {
							opq->chunks.count++;
							opq->chunks.cur.isLoaded = FALSE;
							opq->chunks.cur.isTypeFnd = TRUE;
							//PRINTF_INFO("Chunck %c%c%c%c + %d dataSz.\n", opq->chunks.cur.type[0], opq->chunks.cur.type[1], opq->chunks.cur.type[2], opq->chunks.cur.type[3], opq->chunks.cur.buff.use);
						}
					}
				}
				//---------------------------------
				//-- Cargar datos de chunck, si no hay datos cargados de un ciclo anterior
				//---------------------------------
				//Stop cicle if not destination specified yet (could be specified after header was loaded)
				if((pixsBuff == NULL || pixsBuffSz <= 0) && opq->chunks.cur.type[0] == 'I' && opq->chunks.cur.type[1] == 'D' && opq->chunks.cur.type[2] == 'A' && opq->chunks.cur.type[3] == 'T'){
					continuarCiclo	= FALSE;
					exito			= opq->foundIHDR;
				} else {
					if(!opq->chunks.cur.isLoaded){
						NBASSERT(opq->chunks.cur.isTypeFnd)
						//PENDIENTE: reutilizar la memoria del chunck anterior si es posible
						if(opq->chunks.cur.buff.ptr != NULL){
							if(opq->chunks.cur.buff.sz < (opq->chunks.cur.buff.use + 4)){
								//PRINTF_INFO("Liberando buffer anterior (muy pequeno, %d para %d bytes).\n", opq->chunks.cur.buff.sz, (opq->chunks.cur.buff.use + 4));
								NBMemory_free(opq->chunks.cur.buff.ptr);
								opq->chunks.cur.buff.sz	= 0;
								opq->chunks.cur.buff.ptr		= NULL;
							}
						}
						if(opq->chunks.cur.buff.ptr == NULL){
							//PRINTF_INFO("Creando nuevo buffer tam(%d).\n", opq->chunks.cur.buff.use + 4);
							opq->chunks.cur.buff.sz	= opq->chunks.cur.buff.use + 4;
							opq->chunks.cur.buff.ptr	= (BYTE*)NBMemory_alloc(opq->chunks.cur.buff.sz);
						}
						const SI32 bytesLeidos = NBFile_read(opq->archivo, &(opq->chunks.cur.buff.ptr[4]), opq->chunks.cur.buff.use);
						if(bytesLeidos != opq->chunks.cur.buff.use){
							PRINTF_ERROR("solo se leyeron %d bytes del chunck %c%c%c%c (4 + %d bytes) PNG.\n", bytesLeidos, opq->chunks.cur.type[0], opq->chunks.cur.type[1], opq->chunks.cur.type[2], opq->chunks.cur.type[3], opq->chunks.cur.buff.use);
							continuarCiclo = FALSE;
							//NBASSERT(FALSE)
						} else {
							opq->chunks.cur.buff.ptr[0]	= opq->chunks.cur.type[0];
							opq->chunks.cur.buff.ptr[1]	= opq->chunks.cur.type[1];
							opq->chunks.cur.buff.ptr[2]	= opq->chunks.cur.type[2];
							opq->chunks.cur.buff.ptr[3]	= opq->chunks.cur.type[3];
							//
							UI32 crcChunck = 0;		NBFile_read(opq->archivo, &crcChunck, sizeof(crcChunck)); _invUI32(&crcChunck);
							const UI32 calcCrc		= NBCrc32_getHashBytes((const BYTE*)opq->chunks.cur.buff.ptr, opq->chunks.cur.buff.use + 4);
							if(calcCrc != crcChunck){
								PRINTF_ERROR("el CRC del chunck %c%c%c%c (4 + %d bytes) en el PNG no corresponde.\n", opq->chunks.cur.buff.ptr[0], opq->chunks.cur.buff.ptr[1], opq->chunks.cur.buff.ptr[2], opq->chunks.cur.buff.ptr[3], opq->chunks.cur.buff.use);
								continuarCiclo 		= FALSE;
							} else {
								NBFile_unlock(opq->lector);
								NBFile_close(opq->lector);
								NBFile_release(&opq->lector);
                                NBFile_null(&opq->lector);
								//
                                opq->lector = NBFile_alloc(NULL);
								NBFile_openAsDataRng(opq->lector, opq->chunks.cur.buff.ptr, opq->chunks.cur.buff.use + 4);
								NBFile_lock(opq->lector);
								NBFile_seek(opq->lector, 4, ENNBFileRelative_CurPos); //ignore the chunkId
							}
						}
						opq->chunks.cur.isLoaded = TRUE;
					}
					//---------------------------------
					//-- Interpretar datos de chunck
					//---------------------------------
					if(opq->chunks.cur.buff.ptr != NULL && continuarCiclo){
						opq->chunks.cur.isConsumed = TRUE;
						//PRINTF_INFO("Tipo chunck: %c%c%c%c (4 + %d bytes)\n", opq->chunks.cur.buff.ptr[0], opq->chunks.cur.buff.ptr[1], opq->chunks.cur.buff.ptr[2], opq->chunks.cur.buff.ptr[3], opq->chunks.cur.buff.use);
						if(opq->chunks.cur.type[0] == 'I' && opq->chunks.cur.type[1] == 'H' && opq->chunks.cur.type[2] == 'D' && opq->chunks.cur.type[3] == 'R'){
							//IHDR
							if(opq->chunks.count != 1){
								PRINTF_ERROR("el IHDR no es el primer chunck del PNG.\n");
								continuarCiclo				= FALSE;
							} else {
								SI32 rd;
								opq->foundIHDR			= TRUE;
								rd = NBFile_read(opq->lector, &opq->ihdr.width, sizeof(opq->ihdr.width)); _invUI32(&opq->ihdr.width);		NBASSERT(rd == sizeof(opq->ihdr.width)) //UI32
								rd = NBFile_read(opq->lector, &opq->ihdr.height, sizeof(opq->ihdr.height)); _invUI32(&opq->ihdr.height);	NBASSERT(rd == sizeof(opq->ihdr.height)) //UI32
								rd = NBFile_read(opq->lector, &opq->ihdr.bitsDepth, sizeof(opq->ihdr.bitsDepth));		NBASSERT(rd == sizeof(opq->ihdr.bitsDepth))	//UI8
								rd = NBFile_read(opq->lector, &opq->ihdr.pngColor, sizeof(opq->ihdr.pngColor));			NBASSERT(rd == sizeof(opq->ihdr.pngColor))	//UI8
								rd = NBFile_read(opq->lector, &opq->ihdr.compMethd, sizeof(opq->ihdr.compMethd));		NBASSERT(rd == sizeof(opq->ihdr.compMethd))	//UI8
								rd = NBFile_read(opq->lector, &opq->ihdr.filterMethd, sizeof(opq->ihdr.filterMethd));	NBASSERT(rd == sizeof(opq->ihdr.filterMethd))	//UI8
								rd = NBFile_read(opq->lector, &opq->ihdr.interlMethd, sizeof(opq->ihdr.interlMethd));	NBASSERT(rd == sizeof(opq->ihdr.interlMethd))	//UI8
								//opq->width			= opq->lector->leerUI32Invertido();
								//opq->height			= opq->lector->leerUI32Invertido();
								//opq->bitsDepth		= opq->lector->leerUI8();
								//opq->pngColor			= opq->lector->leerUI8();
								//opq->compMethd		= opq->lector->leerUI8();
								//opq->filterMethd		= opq->lector->leerUI8();
								//opq->interlMethd		= opq->lector->leerUI8();
								//opq->lector->ignorarBytes(opq->chunks.cur.buff.use - 13); //13 bytes de los datos anteriores
								//bits de profundidad
								//if(opq->ihdr.bitsDepth != 8 && opq->ihdr.bitsDepth != 16){
								//	PRINTF_ERROR("PNG load, only PNGs 8 and 16 bits depth are supported (esta es de %d).\n", opq->ihdr.bitsDepth);
								//	continuarCiclo 	= FALSE;
								//}
								//definir compresion
								if(opq->ihdr.compMethd != 0){
									PRINTF_ERROR("PNG load,el metodo de compresion PNG %d no esta soportado.\n", opq->ihdr.compMethd);
									continuarCiclo 	= FALSE;
								}
								//definir filtrado
								if(opq->ihdr.filterMethd != 0){
									PRINTF_ERROR("PNG load,el metodo de filtrado PNG %d no esta soportado.\n", opq->ihdr.filterMethd);
									continuarCiclo 	= FALSE;
								}
								//definir el entrelazado
								if(opq->ihdr.interlMethd != 0 && opq->ihdr.interlMethd != 1){
									PRINTF_ERROR("el tipo de entrelazado PNG %d no esta soportado.\n", opq->ihdr.interlMethd);
									continuarCiclo 	= FALSE;
								}
								// Especificaciones PNG
								// TIPO_COLOR   BITS_PROFUNDIDAD   DESCRIPCION
								// 0            1, 2, 4, 8, 16     Gris
								// 2            8, 16              RGB
								// 3            1, 2, 4, 8         Paleta RGB (el alpha puede incluirse en un chunck tRNS)
								// 4            8, 16              Gris con alpha
								// 6            8, 16              RGBA
								switch(opq->ihdr.pngColor) {
									case 0: //Grayscale
										if(opq->ihdr.bitsDepth != 1 && opq->ihdr.bitsDepth != 2 && opq->ihdr.bitsDepth != 4 && opq->ihdr.bitsDepth != 8 && opq->ihdr.bitsDepth != 16){
											PRINTF_ERROR("PNG load, bitdepth(%d) not allowed for colorMode(%d) by specification.\n", opq->ihdr.bitsDepth, opq->ihdr.pngColor);
										} else {
											opq->buffs.scanlines.bitsPerPix = (opq->ihdr.bitsDepth * 1); //Color
											opq->bmpColor	= ENNBBitmapColor_GRIS8;
											opq->bmpProps	= NBBitmap_propsForBitmap(opq->ihdr.width, opq->ihdr.height, opq->bmpColor);
										}
										break;
									case 2: //Truecolor
										if(opq->ihdr.bitsDepth != 8 && opq->ihdr.bitsDepth != 16){
											PRINTF_ERROR("PNG load, bitdepth(%d) not allowed for colorMode(%d) by specification.\n", opq->ihdr.bitsDepth, opq->ihdr.pngColor);
										} else {
											opq->buffs.scanlines.bitsPerPix = (opq->ihdr.bitsDepth * 3); //Color
											opq->bmpColor	= ENNBBitmapColor_RGB8;
											opq->bmpProps	= NBBitmap_propsForBitmap(opq->ihdr.width, opq->ihdr.height, opq->bmpColor);
										}
										break;
									case 3: //Indexed-color
										if(opq->ihdr.bitsDepth != 1 && opq->ihdr.bitsDepth != 2 && opq->ihdr.bitsDepth != 4 && opq->ihdr.bitsDepth != 8){
											PRINTF_ERROR("PNG load, bitdepth(%d) not allowed for colorMode(%d) by specification.\n", opq->ihdr.bitsDepth, opq->ihdr.pngColor);
										} else {
											opq->buffs.scanlines.bitsPerPix = (opq->ihdr.bitsDepth * 1); //Index to 'PLTE'
											//Waiting for 'PTE' and 'tRNS' to detemrine bmpProps.
										}
										break;
									case 4: //Grayscale with alpha
										if(opq->ihdr.bitsDepth != 8 && opq->ihdr.bitsDepth != 16){
											PRINTF_ERROR("PNG load, bitdepth(%d) not allowed for colorMode(%d) by specification.\n", opq->ihdr.bitsDepth, opq->ihdr.pngColor);
										} else {
											opq->buffs.scanlines.bitsPerPix = (opq->ihdr.bitsDepth * 2); //Color
											opq->bmpColor	= ENNBBitmapColor_GRISALPHA8;
											opq->bmpProps	= NBBitmap_propsForBitmap(opq->ihdr.width, opq->ihdr.height, opq->bmpColor);
										}
										break;
									case 6: //Truecolor with alpha
										if(opq->ihdr.bitsDepth != 8 && opq->ihdr.bitsDepth != 16){
											PRINTF_ERROR("PNG load, bitdepth(%d) not allowed for colorMode(%d) by specification.\n", opq->ihdr.bitsDepth, opq->ihdr.pngColor);
										} else {
											opq->buffs.scanlines.bitsPerPix = (opq->ihdr.bitsDepth * 4); //Color
											opq->bmpColor	= ENNBBitmapColor_RGBA8;
											opq->bmpProps	= NBBitmap_propsForBitmap(opq->ihdr.width, opq->ihdr.height, opq->bmpColor);
										}
										break;
									default:
										PRINTF_ERROR("PNG load, colorMode(%d) not in specification.\n", opq->ihdr.pngColor);
										continuarCiclo = FALSE;
										break;
								}
								if(dstIHDR != NULL){
									*dstIHDR = opq->ihdr;
								}
							}
						} else if(opq->chunks.cur.type[0] == 'P' && opq->chunks.cur.type[1] == 'L' && opq->chunks.cur.type[2] == 'T' && opq->chunks.cur.type[3] == 'E') {
							//PLTE
							if(!opq->foundIHDR){
								PRINTF_ERROR("aparecio un chunck PLTE antes del IHDR en el PNG.\n");
								continuarCiclo			= FALSE;
							} else {
								//PRINTF_INFO("Tamano de PLTE: %d\n", opq->chunks.cur.buff.use);
								if((opq->chunks.cur.buff.use % 3) != 0){
									PRINTF_ERROR("PNG load, PLTE chunck size is not divisble by 3 (RGB).\n");
									continuarCiclo		= FALSE;
								} else {
									opq->chunks.PLTE		= opq->chunks.cur.buff.ptr;
									opq->chunks.PLTEData	= &opq->chunks.cur.buff.ptr[NBFile_curPos(opq->lector)];
									opq->chunks.PLTESz		= opq->chunks.cur.buff.use;
									//
									if(opq->ihdr.pngColor == 3){
										//RGB or RGBA depending of 'tRNS'
										opq->bmpColor	= (opq->chunks.tRNSData == NULL ? ENNBBitmapColor_RGB8 : ENNBBitmapColor_RGBA8);
										opq->bmpProps	= NBBitmap_propsForBitmap(opq->ihdr.width, opq->ihdr.height, opq->bmpColor);
									}
									//Heredar este chunck (permite que se conserven los datos)
									NBASSERT(opq->chunks.cur.buff.ptr != NULL)
									opq->chunks.cur.buff.ptr		= NULL;
									opq->chunks.cur.buff.sz		= 0;
									opq->chunks.cur.buff.use	= 0;
								}
							}
						} else if(opq->chunks.cur.type[0] == 't' && opq->chunks.cur.type[1] == 'R' && opq->chunks.cur.type[2] == 'N' && opq->chunks.cur.type[3] == 'S') {
							//tRNS
							if(!opq->foundIHDR){
								PRINTF_ERROR("aparecio un chunck tRNS antes del IHDR en el PNG.\n");
								continuarCiclo 		= FALSE;
							} else {
								//PRINTF_INFO("Tamano de tRNS: %d\n", opq->chunks.cur.buff.use);
								opq->chunks.tRNS		= opq->chunks.cur.buff.ptr;
								opq->chunks.tRNSData	= &opq->chunks.cur.buff.ptr[NBFile_curPos(opq->lector)];
								opq->chunks.tRNSSz		= opq->chunks.cur.buff.use;
								//
								if(opq->ihdr.pngColor == 3 && opq->chunks.PLTEData != NULL){
									opq->bmpColor	= ENNBBitmapColor_RGBA8;
									opq->bmpProps	= NBBitmap_propsForBitmap(opq->ihdr.width, opq->ihdr.height, opq->bmpColor);
								}
								//Heredar este chunck (permite que se conserven los datos)
								NBASSERT(opq->chunks.cur.buff.ptr != NULL)
								opq->chunks.cur.buff.ptr		= NULL;
								opq->chunks.cur.buff.sz		= 0;
								opq->chunks.cur.buff.use	= 0;
							}
							/*} else if(opq->chunks.cur.type[0] == 's' && opq->chunks.cur.type[1] == 'h' && opq->chunks.cur.type[2] == 'A' && opq->chunks.cur.type[3] == 'A') {
							 if(opq->chunks.cur.buff.use!=20){
							 PRINTF_ERROR("el chunck 'shAA' no guarda una firma SHA1 de 20 bytes (es de %d bytes)\n", opq->chunks.cur.buff.use);
							 } else {
							 STNBSha1Hash firmaSHA1;	SHA1_HASH_INIT(firmaSHA1);
							 BYTE* bytesFirmaSHA1 = (BYTE*)&firmaSHA1;
							 SI32 iByte; for(iByte = 0; iByte<20; iByte++){
							 bytesFirmaSHA1[iByte] = opq->lector->leerUI8();
							 }
							 //unsigned char firmaHex[41]; NBSHA1::dameHash20Hexadecimal((const unsigned char*)&firmaSHA1, firmaHex);
							 //PRINTF_INFO("Firma SHA1 aparecio en el PNG: '%s'\n", firmaHex);
							 if(guardarFirmaSHA1En != NULL) *guardarFirmaSHA1En = firmaSHA1;
							 }*/
						} else if(opq->chunks.cur.type[0] == 'I' && opq->chunks.cur.type[1] == 'D' && opq->chunks.cur.type[2] == 'A' && opq->chunks.cur.type[3] == 'T'){
							//IDAT (los datos en el PNG no estan paddeados, e incluyen un byte de entrelazado al principio)
							//NOTA: en experimentos he notado que en archivos PNGs con multiples bloques IDAT, son de 8 o 16 KBs por bloque
							if(!opq->foundIHDR){
								continuarCiclo = FALSE;
								PRINTF_ERROR("PNG load, IDAT before IHDR.\n");
							} else if(opq->chunks.PLTEData == NULL && (opq->ihdr.pngColor & 1) != 0){
								continuarCiclo = FALSE;
								PRINTF_ERROR("PNG load, IDAT before PLTE chunck.\n");
							} else if(opq->ihdr.width <= 0 || opq->ihdr.height <= 0 || opq->buffs.scanlines.bitsPerPix <= 0 || opq->bmpProps.bytesPerLine <= 0){
								continuarCiclo = FALSE;
								PRINTF_ERROR("PNG load, IDAT while not-valid values for width(%d), height(%d) y scanLineBitsPerPx(%d).\n", opq->ihdr.width, opq->ihdr.height, opq->buffs.scanlines.bitsPerPix);
							} else if(pixsBuffSz < (opq->ihdr.height * opq->bmpProps.bytesPerLine)){
								continuarCiclo = FALSE;
								PRINTF_ERROR("PNG load, IDAT while dstBuffer not big enought for all pixels.\n");
								NBASSERT(FALSE); //Testing validation
							} else {
								NBASSERT(NBFile_curPos(opq->lector) == 4)
								//Consume the full chunk
								if(opq->buffs.pixels.isCompleted){
									//Ignore extra data
								} else {
									SI32 chunkBuffPos = 0; //+4 for real chunk pos
									BOOL chunkConsumed = FALSE;
									while(continuarCiclo && !chunkConsumed){
										//Create scanlines buffer
										if(opq->ihdr.interlMethd == 0){
											//Not interlaced, only one scanLines buffer.
											if(opq->buffs.scanlines.sampleIdx > 0){
												if(opq->buffs.scanlines.cur != NULL){
													NBPngScanLinesBuffer_release(opq->buffs.scanlines.cur);
													NBMemory_free(opq->buffs.scanlines.cur);
													opq->buffs.scanlines.cur = NULL;
												}
												continuarCiclo = FALSE;
												PRINTF_ERROR("Interlaced-0 expected only 1 scanlines pass.\n");
											} else {
												if(opq->buffs.scanlines.cur == NULL){
													const UI32 sampleWidth	= opq->ihdr.width;
													const UI32 sampleHeight	= opq->ihdr.height;
													const UI32 bitsPerLine	= (opq->buffs.scanlines.bitsPerPix * sampleWidth);
													const UI32 bytesPerLine	= (bitsPerLine / 8) + ((bitsPerLine % 8) != 0 ? 1 : 0);
													const UI32 lines256KB	= ((1024 * 256) / bytesPerLine);
													const UI32 lines20P		= (sampleHeight / (100 / 20));
													const UI32 linesBuf		= (sampleHeight < lines256KB ? sampleHeight : lines20P > lines256KB ? lines20P : lines256KB);
													NBASSERT(linesBuf <= sampleHeight)
													//PRINTF_INFO("Scanlines buffer is %d%% of bitmap sample #%d.\n", linesBuf * 100 / sampleHeight, (opq->buffs.scanlines.sampleIdx + 1))
													NBASSERT(opq->buffs.scanlines.sampleIdx == 0)
													opq->buffs.scanlines.sampleSz.width		= sampleWidth;
													opq->buffs.scanlines.sampleSz.height	= sampleHeight;
													opq->buffs.scanlines.bytesLoaded	 	= 0;
													opq->buffs.scanlines.cur				= NBMemory_allocType(STNBPngScanLinesBuffer);
													NBPngScanLinesBuffer_init(opq->buffs.scanlines.cur);
													NBPngScanLinesBuffer_create(opq->buffs.scanlines.cur, opq->buffs.scanlines.sampleSz.width, linesBuf, opq->buffs.scanlines.bitsPerPix);
												}
											}
										} else if(opq->ihdr.interlMethd == 1){
											//Interlaced, expecting 7 scanLines buffer (some can be empty if image is smaller than 5 pixels width or height).
											if(opq->buffs.scanlines.sampleIdx > 7){
												if(opq->buffs.scanlines.cur != NULL){
													NBPngScanLinesBuffer_release(opq->buffs.scanlines.cur);
													NBMemory_free(opq->buffs.scanlines.cur);
													opq->buffs.scanlines.cur = NULL;
												}
												continuarCiclo = FALSE;
												PRINTF_ERROR("Interlaced-1 expected not more than 7 scanlines pass.\n");
											} else {
												//Start next sample
												if(opq->buffs.scanlines.cur == NULL){
													const STNBAdam7Sample* adamDef = &__adam7Defs[opq->buffs.scanlines.sampleIdx];
													const SI32 sampleWidthB		= ((SI32)opq->ihdr.width - (SI32)adamDef->cols.start);
													const SI32 sampleHeightB	= ((SI32)opq->ihdr.height - (SI32)adamDef->rows.start);
													const UI32 sampleWidth		= (sampleWidthB <= 0 ? 0 : (sampleWidthB / adamDef->cols.inc) + ((sampleWidthB % adamDef->cols.inc) != 0 ? 1 : 0));
													const UI32 sampleHeight		= (sampleHeightB <= 0 ? 0 : (sampleHeightB / adamDef->rows.inc) + ((sampleHeightB % adamDef->rows.inc) != 0 ? 1 : 0));
													if(sampleWidth <= 0 || sampleHeight <= 0){
														//PRINTF_INFO("Scanlines buffer is empty for sample #%d.\n", (opq->buffs.scanlines.sampleIdx + 1))
														opq->buffs.scanlines.sampleIdx++;
													} else {
														const UI32 bitsPerLine	= (opq->buffs.scanlines.bitsPerPix * sampleWidth);
														const UI32 bytesPerLine	= (bitsPerLine / 8) + ((bitsPerLine % 8) != 0 ? 1 : 0);
														const UI32 lines256KB	= ((1024 * 256) / bytesPerLine);
														const UI32 lines20P		= (sampleHeight / (100 / 20));
														const UI32 linesBuf		= (sampleHeight < lines256KB ? sampleHeight : lines20P > lines256KB ? lines20P : lines256KB);
														NBASSERT(linesBuf <= sampleHeight)
														//PRINTF_INFO("Scanlines buffer is %d%% of bitmap sample #%d of 7 (%d, %d) of (%d, %d).\n", linesBuf * 100 / sampleHeight, (opq->buffs.scanlines.sampleIdx + 1), sampleWidth, sampleHeight, opq->ihdr.width, opq->ihdr.height);
														NBASSERT(opq->buffs.scanlines.sampleIdx <= 7)
														opq->buffs.scanlines.sampleSz.width		= sampleWidth;
														opq->buffs.scanlines.sampleSz.height	= sampleHeight;
														opq->buffs.scanlines.bytesLoaded	 	= 0;
														opq->buffs.scanlines.cur				= NBMemory_allocType(STNBPngScanLinesBuffer);
														NBPngScanLinesBuffer_init(opq->buffs.scanlines.cur);
														NBPngScanLinesBuffer_create(opq->buffs.scanlines.cur, opq->buffs.scanlines.sampleSz.width, linesBuf, opq->buffs.scanlines.bitsPerPix);
													}
												}
											}
										}
										//Consume chunk data
										if(opq->buffs.scanlines.cur != NULL){
											NBASSERT(opq->buffs.scanlines.cur != NULL) //muts exists
											NBASSERT(opq->buffs.scanlines.cur->dataSz != 0) //must not be empty
											NBASSERT(opq->buffs.scanlines.cur->dataToFill != 0) //must not be empty
											NBASSERT((opq->buffs.scanlines.cur->dataSz % opq->buffs.scanlines.cur->bytesPerLine) == 0) //must be size of full scanlines
											NBASSERT((opq->buffs.scanlines.cur->dataToFill % opq->buffs.scanlines.cur->bytesPerLine) == 0) //must be size of full scanlines
											NBASSERT(pixsBuffSz != 0  && (pixsBuffSz % opq->bmpProps.bytesPerLine) == 0) //el buffer debe ser multiplo del tamano de una linea
											NBASSERT(opq->buffs.scanlines.bytesLoaded <= opq->buffs.scanlines.cur->dataToFill) //Must have space
											NBASSERT(chunkBuffPos >= 0 && chunkBuffPos <= opq->chunks.cur.buff.use)
											const SI32 tamanoIn		= opq->zlibb.isHungry ? (opq->chunks.cur.buff.use - chunkBuffPos) : 0; NBASSERT(chunkBuffPos <= opq->chunks.cur.buff.use)
											const BYTE* punteroIn	= opq->zlibb.isHungry ? &opq->chunks.cur.buff.ptr[4 + chunkBuffPos] : NULL;
											const SI32 tamanoOut	= (opq->buffs.scanlines.cur->dataToFill - opq->buffs.scanlines.bytesLoaded); NBASSERT(opq->buffs.scanlines.bytesLoaded <= opq->buffs.scanlines.cur->dataToFill)
											BYTE* punteroOut		= &opq->buffs.scanlines.cur->data[opq->buffs.scanlines.bytesLoaded];
											const ENZInfResult zr	= NBZInflate_feed(&opq->zlibb.obj, punteroOut, tamanoOut, punteroIn, tamanoIn, ENZInfBlckType_Partial);
											opq->zlibb.isHungry		= FALSE;
											if(zr.resultCode < 0){
												PRINTF_ERROR("PNG load, IDAT chunk not consumed(%d%% consumed, %d bytes remained) (inflate returned error).\n", ((opq->chunks.cur.buff.use - chunkBuffPos) * 100 / chunkBuffPos), (opq->chunks.cur.buff.use - chunkBuffPos));
												continuarCiclo = FALSE;
											} else if(zr.inBytesProcessed <= 0 && zr.outBytesProcessed <= 0){
												//None of the buffer was affected
												NBASSERT(chunkBuffPos <= opq->chunks.cur.buff.use)
												if(chunkBuffPos == opq->chunks.cur.buff.use){
													//PRINTF_INFO("PNG load, IDAT chunk consumed (inflate did not changed the buffers).\n");
													chunkConsumed = TRUE;
													opq->chunks.IDATCount++;
												} else {
													PRINTF_ERROR("PNG load, IDAT chunk not consumed(%d%% consumed, %d bytes remained) (inflate did not changed the buffers).\n", ((opq->chunks.cur.buff.use - chunkBuffPos) * 100 / chunkBuffPos), (opq->chunks.cur.buff.use - chunkBuffPos));
													continuarCiclo = FALSE;
												}
											} else {
												chunkBuffPos += zr.inBytesProcessed; NBASSERT(chunkBuffPos >= 0 && chunkBuffPos <= opq->chunks.cur.buff.use)
												opq->buffs.scanlines.bytesLoaded += zr.outBytesProcessed; NBASSERT(opq->buffs.scanlines.bytesLoaded >= 0 && opq->buffs.scanlines.bytesLoaded <= opq->buffs.scanlines.cur->dataToFill)
												//PRINTF_INFO("Zlib in-bytes-processed(%d) chunk-progress(%d%%).\n", zr.inBytesProcessed, chunkBuffPos * 100 / opq->chunks.cur.buff.use);
												//Determine if zlib is hungry
												{
													const BOOL inAgotado	= (zr.inBytesAvailable == 0);
													const BOOL outAgotado	= (zr.outBytesAvailable == 0);
													if(!outAgotado || inAgotado || zr.resultCode == 1/*Z_STREAM_END*/){
														opq->zlibb.isHungry	= TRUE;
														if(chunkBuffPos >= opq->chunks.cur.buff.use){
															chunkConsumed = TRUE;
														}
													}
												}
												//Process sncaLines buffers (if filled)
												{
													const SI32 linesFilled	= (opq->buffs.scanlines.bytesLoaded / opq->buffs.scanlines.cur->bytesPerLine);
													const SI32 linesRemain	= (opq->buffs.scanlines.sampleSz.height - (opq->buffs.scanlines.cur->iLineStart + linesFilled));
													const BOOL buffIsFull	= (opq->buffs.scanlines.bytesLoaded == opq->buffs.scanlines.cur->dataToFill);
													if(buffIsFull || linesRemain == 0){
														//Current scanlines buffer is full, or all lines of this sample were loaded (even with buffer not-full)
														if(!NBPng_consumeInterlacedOpq(opq, pixsBuff, pixsBuffSz)){
															continuarCiclo = FALSE;
															PRINTF_ERROR("PNG load, IDAT interlaced buffer could not consumed.\n");
														} else {
															if(linesRemain > 0){
																//Create new scanlines buffer by keeping last scanLine
																//PRINTF_INFO("PNG load, next scaneLinesBuffer created.\n");
																NBPngScanLinesBuffer_createNext(opq->buffs.scanlines.cur, opq->buffs.scanlines.cur, (linesRemain < opq->buffs.scanlines.cur->height ? linesRemain : opq->buffs.scanlines.cur->height));
																opq->buffs.scanlines.bytesLoaded = 0;
															} else {
																//Force next scanlines sample
																opq->buffs.scanlines.sampleIdx++;
																if(opq->buffs.scanlines.cur != NULL){
																	NBPngScanLinesBuffer_release(opq->buffs.scanlines.cur);
																	NBMemory_free(opq->buffs.scanlines.cur);
																	opq->buffs.scanlines.cur = NULL;
																}
																//Determine if data was completed
																{
																	if(opq->ihdr.interlMethd == 0){
																		if(opq->buffs.scanlines.sampleIdx == 1){
																			//PRINTF_INFO("PNG load, IDAT last scanLinesBuff(#%d) completed.\n", opq->buffs.scanlines.sampleIdx);
																			opq->buffs.pixels.isCompleted = TRUE;
																			continuarCiclo = FALSE;
																			exito = TRUE;
																		}
																	} else if(opq->ihdr.interlMethd == 1){
																		if(opq->buffs.scanlines.sampleIdx == 7){
																			//PRINTF_INFO("PNG load, IDAT last scanLinesBuff(#%d) completed.\n", opq->buffs.scanlines.sampleIdx);
																			opq->buffs.pixels.isCompleted = TRUE;
																			continuarCiclo = FALSE;
																			exito = TRUE;
																		}
																	}
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
						} else if(opq->chunks.cur.type[0] == 'I' && opq->chunks.cur.type[1] == 'E' && opq->chunks.cur.type[2] == 'N' && opq->chunks.cur.type[3] == 'D') {
							//IEND
							//PRINTF_INFO("IEND alcanzado!\n");
							continuarCiclo	= FALSE;
							exito			= (opq->foundIHDR && opq->buffs.scanlines.bitsPerPix != 0 && opq->bmpColor != ENNBBitmapColor_undef && opq->buffs.pixels.isCompleted);
							opq->foundIEND	= TRUE;
							NBASSERT(opq->foundIHDR)
							NBASSERT(opq->buffs.scanlines.bitsPerPix != 0)
							NBASSERT(opq->bmpColor != ENNBBitmapColor_undef)
							NBASSERT(opq->buffs.pixels.isCompleted)
						} else {
							if(!opq->foundIHDR){
								if(opq->chunks.cur.type[0] == 'C' && opq->chunks.cur.type[1] == 'g' && opq->chunks.cur.type[2] == 'B' && opq->chunks.cur.type[3] == 'I'){
									if(dstHeadCustom != NULL) dstHeadCustom->isCgBI = TRUE;
								} else {
									PRINTF_ERROR("PNG load, chunk('%c%c%c%c') before IHDR.\n", opq->chunks.cur.type[0], opq->chunks.cur.type[1], opq->chunks.cur.type[2], opq->chunks.cur.type[3]);
								}
								continuarCiclo 	= FALSE;
							} else {
								//Add extra chunk
								if(dstExtraChunks != NULL){
									if(opq->chunks.cur.buff.ptr != NULL && opq->chunks.cur.buff.use > 0){
										STNBPngChunk chnk;
										NBPngChunk_init(&chnk);
										chnk.name[0] = opq->chunks.cur.buff.ptr[0];
										chnk.name[1] = opq->chunks.cur.buff.ptr[1];
										chnk.name[2] = opq->chunks.cur.buff.ptr[2];
										chnk.name[3] = opq->chunks.cur.buff.ptr[3];
										chnk.name[4] = '\0';
										chnk.dataSz = opq->chunks.cur.buff.use;
										chnk.data	= NBMemory_alloc(chnk.dataSz);
										NBMemory_copy(chnk.data, &opq->chunks.cur.buff.ptr[4], chnk.dataSz);
										NBArray_addValue(dstExtraChunks, chnk);
									}
								}
							}
						}
						//Fin de lectura de CHUNCK
						if(opq->chunks.cur.isConsumed){
							opq->chunks.cur.isTypeFnd = FALSE;
						}
					}
				}
			} while(continuarCiclo);
		}
	}
	NBFile_unlock(opq->lector);
	return exito;
}

#define NBPNG_GET_BITS(ROW_PTR, BIT_IDX, BITS_WIDTH)		(((ROW_PTR[(BIT_IDX) / 8] << ((BIT_IDX) % 8)) & 0xFF) >> (8 - BITS_WIDTH))

BOOL NBPng_consumeInterlacedOpq(STNBPngLoadStateOpq* opq, BYTE* pixsBuff, const UI32 pixsBuffSz){
	BOOL r = FALSE;
	NBASSERT((opq->buffs.scanlines.bytesLoaded % opq->buffs.scanlines.cur->bytesPerLine) == 0) //La cantidad de bytes deben corresponder a filas completas
	if((opq->buffs.scanlines.bytesLoaded % opq->buffs.scanlines.cur->bytesPerLine) == 0){
		BOOL errFnd = FALSE;
		const UI32 bytesPerScanLn	= opq->buffs.scanlines.cur->bytesPerLine;
		const UI32 linesAtBuff		= (opq->buffs.scanlines.bytesLoaded / opq->buffs.scanlines.cur->bytesPerLine); NBASSERT(linesAtBuff > 0)
		//Unfilter data
		if(!(opq->ihdr.bitsDepth == 1 || opq->ihdr.bitsDepth == 2 || opq->ihdr.bitsDepth == 4 || opq->ihdr.bitsDepth == 8 || opq->ihdr.bitsDepth == 16)){
			errFnd = TRUE;
		} else {
			//---------------------------
			// Filters are applied to bytes, not to pixels,
			// regardless of the bit depth or colour type of the image.
			//---------------------------
			SI32 p, pa, pb, pc;
			UI8* rowPtr; UI8* rowPrevPtr;
			UI8 filterMthd, compCur, compLft, compTop, compLftTop, compNew;
			UI32 fil, iByte, bytesPerPix = (opq->buffs.scanlines.bitsPerPix / 8);
			if(bytesPerPix <= 0) bytesPerPix = 1;
			for(fil = 0; fil < linesAtBuff && !errFnd; fil++){
				rowPtr 		= &(opq->buffs.scanlines.cur->data[(fil * bytesPerScanLn)]);
				rowPrevPtr	= (fil == 0 ? opq->buffs.scanlines.cur->prevLine : &(opq->buffs.scanlines.cur->data[((fil - 1) * bytesPerScanLn)]));
				filterMthd	= rowPtr[0];
				//PRINTF_INFO("PNG load, line #%d/%d (#%d/%d) filterMthd(%d) value.\n", (fil + 1), linesAtBuff, (opq->buffs.scanlines.cur->iLineStart + fil + 1), opq->buffs.scanlines.sampleSz.height, filterMthd);
				switch (filterMthd) {
					case 0: //NO FILTER
						//Do nothing
						break;
					case 1: //FILTRADO-SUB
						for(iByte = 1; iByte < bytesPerScanLn; iByte++){
							compCur			= rowPtr[iByte];
							compLft			= 0; if(iByte > bytesPerPix) compLft = rowPtr[iByte - bytesPerPix];
							//Apply
							compNew			= compCur + compLft;
							rowPtr[iByte]	= compNew;
						}
						break;
					case 2: //FILTRADO-UP
						for(iByte = 1; iByte < bytesPerScanLn; iByte++){
							compCur			= rowPtr[iByte];
							compTop 		= 0; if(rowPrevPtr != NULL) compTop = rowPrevPtr[iByte];
							//Apply
							compNew			= compCur + compTop;
							rowPtr[iByte]	= compNew;
						}
						break;
					case 3: //FILTRADO-AVERAGE
						for(iByte = 1; iByte < bytesPerScanLn; iByte++){
							compCur			= rowPtr[iByte];
							compLft			= 0; if(iByte > bytesPerPix) compLft = rowPtr[iByte - bytesPerPix];
							compTop 		= 0; if(rowPrevPtr != NULL) compTop = rowPrevPtr[iByte];
							//Apply
							compNew			= compCur + ((compLft + compTop) / 2);
							rowPtr[iByte]	= compNew;
						}
						break;
					case 4: //FILTRADO-PAETH
						for(iByte = 1; iByte < bytesPerScanLn; iByte++){
							compCur			= rowPtr[iByte];
							compLft			= 0; if(iByte > bytesPerPix) compLft = rowPtr[iByte - bytesPerPix];
							compTop 		= 0; if(rowPrevPtr != NULL) compTop = rowPrevPtr[iByte];
							//Apply
							compLftTop		= 0;
							if(iByte > bytesPerPix && rowPrevPtr != NULL){
								compLftTop = rowPrevPtr[iByte - bytesPerPix];
							}
							{
								//Paeth
								p	= compLft + compTop - compLftTop;
								pa	= (p - compLft); 	if(pa<0) pa = -pa; //ABS
								pb	= (p - compTop);	if(pb<0) pb = -pb; //ABS
								pc	= (p - compLftTop);	if(pc<0) pc = -pc; //ABS
								if(pa <= pb && pa <= pc){
									compNew = compLft;
								} else if(pb <= pc){
									compNew = compTop;
								} else {
									compNew = compLftTop;
								}
							}
							compNew			= compCur + compNew;
							rowPtr[iByte]	= compNew;
						}
						break;
					default:
						PRINTF_ERROR("PNG load, unexpected filterMthd(%d) value.\n", filterMthd);
						errFnd = TRUE; NBASSERT(FALSE)
						break;
				}
			} //for(fil)
		}
		//Convertir los datos desentrelazados hacia datos de mapaBits
		// Especificaciones PNG
		// TIPO_COLOR   BITS_PROFUNDIDAD   DESCRIPCION
		// 0            1, 2, 4, 8, 16     Gris
		// 2            8, 16              RGB
		// 3            1, 2, 4, 8         Paleta RGB (el alpha puede incluirse en un chunck tRNS)
		// 4            8, 16              Gris con alpha
		// 6            8, 16              RGBA
		if(!errFnd){
			//---------------------------
			// 8 bits samples (copy pixels to bitmap)
			//---------------------------
			if(opq->ihdr.bitsDepth == 1 || opq->ihdr.bitsDepth == 2 || opq->ihdr.bitsDepth == 4 || opq->ihdr.bitsDepth == 8 || opq->ihdr.bitsDepth == 16){
				if(opq->ihdr.interlMethd == 0){
					if(opq->ihdr.pngColor == 0 || opq->ihdr.pngColor == 2 || opq->ihdr.pngColor == 4 || opq->ihdr.pngColor == 6){
						//Copy without PLTE
						const UI32 bytesSmpls = (bytesPerScanLn - 1);
						UI32 iFila; for(iFila = 0; iFila < linesAtBuff; iFila++){
							const UI8* lineSrc	= &opq->buffs.scanlines.cur->data[1 + (iFila * bytesPerScanLn)]; //+1 para excluir el byte de entrelazado
							UI8* lineDst		= &pixsBuff[(opq->buffs.scanlines.cur->iLineStart + iFila) * opq->bmpProps.bytesPerLine];
							if(opq->ihdr.bitsDepth == 8){
								NBMemory_copy(lineDst, lineSrc, bytesSmpls);
							} else if(opq->ihdr.bitsDepth == 16){
								UI32 i; for(i = 0; i < bytesSmpls; i += 2){
									//Simply ignore the lower byte order
									lineDst[i / 2] = lineSrc[i];
								}
							} else {
								NBASSERT(opq->ihdr.bitsDepth == 1 || opq->ihdr.bitsDepth == 2 || opq->ihdr.bitsDepth == 4)
								NBASSERT(opq->buffs.scanlines.sampleSz.width <= (opq->bmpProps.bytesPerLine / (opq->bmpProps.bitsPerPx / 8)))
								const UI32 bitsPerSmpl	= opq->ihdr.bitsDepth;
								const UI32 bitsPerLine	= (opq->buffs.scanlines.sampleSz.width * opq->ihdr.bitsDepth);
								const UI8 multBase		= (0xFF / (0xFF >> (8 - opq->ihdr.bitsDepth)));
								UI8 val;
								UI32 iBit; for(iBit = 0; iBit < bitsPerLine; iBit += bitsPerSmpl){
									val = NBPNG_GET_BITS(lineSrc, iBit, bitsPerSmpl);
									lineDst[iBit / bitsPerSmpl] = val * multBase;
								}
							}
						}
						r = TRUE;
					} else if(opq->ihdr.pngColor == 3){
						if(opq->chunks.PLTEData == NULL){
							PRINTF_ERROR("PNG load, missing PLTE.\n");
						} else {
							if(opq->ihdr.bitsDepth == 8){
								if(opq->chunks.tRNSData == NULL){
									//Ciclo de copiado con paleta RGB sin paleta alpha
									UI32 iFila, iPixel, iByteDestino, pixsCount = opq->buffs.scanlines.sampleSz.width;
									for(iFila = 0; iFila < linesAtBuff; iFila++){
										const  UI8* lineSrc = &opq->buffs.scanlines.cur->data[1 + (iFila * bytesPerScanLn)]; //+1 para excluir el byte de entrelazado
										UI8* lineDst		= &pixsBuff[(opq->buffs.scanlines.cur->iLineStart + iFila) * opq->bmpProps.bytesPerLine];
										iByteDestino		= 0;
										for(iPixel = 0; iPixel < pixsCount; iPixel++){
											UI8 indiceEnPaleta		= lineSrc[iPixel];
											UI8* pixelRGBAPaleta	= &(opq->chunks.PLTEData[indiceEnPaleta * 3]); //x3, la paleta es RGB
											lineDst[iByteDestino++] = pixelRGBAPaleta[0]; //R
											lineDst[iByteDestino++] = pixelRGBAPaleta[1]; //G
											lineDst[iByteDestino++] = pixelRGBAPaleta[2]; //B
										}
									}
									r = TRUE;
								} else {
									//Ciclo de copiado con paleta RGB y paleta alpha
									UI32 iFila, iPixel, iByteDestino, pixsCount = opq->buffs.scanlines.sampleSz.width;
									for(iFila = 0; iFila < linesAtBuff; iFila++){
										const UI8* lineSrc	= &opq->buffs.scanlines.cur->data[1 + (iFila * bytesPerScanLn)]; //+1 para excluir el byte de entrelazado
										UI8* lineDst		= &pixsBuff[(opq->buffs.scanlines.cur->iLineStart + iFila) * opq->bmpProps.bytesPerLine];
										iByteDestino		= 0;
										for(iPixel = 0; iPixel < pixsCount; iPixel++){
											UI8 indiceEnPaleta		= lineSrc[iPixel];
											UI8* pixelRGBAPaleta	= &(opq->chunks.PLTEData[indiceEnPaleta * 3]); //x3, la paleta es RGB
											UI8 alpha				= 255;
											if(indiceEnPaleta < opq->chunks.tRNSSz){
												alpha				= opq->chunks.tRNSData[indiceEnPaleta];
											}
											lineDst[iByteDestino++] = pixelRGBAPaleta[0]; //R
											lineDst[iByteDestino++] = pixelRGBAPaleta[1]; //G
											lineDst[iByteDestino++] = pixelRGBAPaleta[2]; //B
											lineDst[iByteDestino++] = alpha;				//A
										}
									}
									r = TRUE;
								}
							} else if(opq->ihdr.bitsDepth == 1 || opq->ihdr.bitsDepth == 2 || opq->ihdr.bitsDepth == 4){
								if(opq->chunks.tRNSData == NULL){
									//RGB
									UI32 iFila;
									for(iFila = 0; iFila < linesAtBuff && !errFnd; iFila++){
										const UI8* lineSrc	= &opq->buffs.scanlines.cur->data[1 + (iFila * bytesPerScanLn)]; //+1 para excluir el byte de entrelazado
										UI8* lineDst		= &pixsBuff[(opq->buffs.scanlines.cur->iLineStart + iFila) * opq->bmpProps.bytesPerLine];
										{
											NBASSERT(opq->buffs.scanlines.sampleSz.width <= (opq->bmpProps.bytesPerLine / (opq->bmpProps.bitsPerPx / 8)))
											const UI32 bitsPerSmpl	= opq->ihdr.bitsDepth;
											const UI32 bitsPerLine	= (opq->buffs.scanlines.sampleSz.width * opq->ihdr.bitsDepth);
											UI32 iByteDestino		= 0;
											UI32 iBit; for(iBit = 0; iBit < bitsPerLine; iBit += bitsPerSmpl){
												UI8 indiceEnPaleta		= NBPNG_GET_BITS(lineSrc, iBit, bitsPerSmpl);
												UI8* pixelRGBAPaleta	= &(opq->chunks.PLTEData[indiceEnPaleta * 3]); //x3, la paleta es RGB
												lineDst[iByteDestino++] = pixelRGBAPaleta[0]; //R
												lineDst[iByteDestino++] = pixelRGBAPaleta[1]; //G
												lineDst[iByteDestino++] = pixelRGBAPaleta[2]; //B
											}
										}
									}
									r = TRUE;
								} else {
									//RGB + Alpha
									UI32 iFila;
									for(iFila = 0; iFila < linesAtBuff && !errFnd; iFila++){
										const UI8* lineSrc	= &opq->buffs.scanlines.cur->data[1 + (iFila * bytesPerScanLn)]; //+1 para excluir el byte de entrelazado
										UI8* lineDst		= &pixsBuff[(opq->buffs.scanlines.cur->iLineStart + iFila) * opq->bmpProps.bytesPerLine];
										{
											NBASSERT(opq->buffs.scanlines.sampleSz.width <= (opq->bmpProps.bytesPerLine / (opq->bmpProps.bitsPerPx / 8)))
											const UI32 bitsPerSmpl	= opq->ihdr.bitsDepth;
											const UI32 bitsPerLine	= (opq->buffs.scanlines.sampleSz.width * opq->ihdr.bitsDepth);
											UI32 iByteDestino		= 0;
											UI32 iBit; for(iBit = 0; iBit < bitsPerLine; iBit += bitsPerSmpl){
												UI8 indiceEnPaleta		= NBPNG_GET_BITS(lineSrc, iBit, bitsPerSmpl);
												UI8* pixelRGBAPaleta	= &(opq->chunks.PLTEData[indiceEnPaleta * 3]); //x3, la paleta es RGB
												UI8 alpha				= 255;
												if(indiceEnPaleta < opq->chunks.tRNSSz){
													alpha				= opq->chunks.tRNSData[indiceEnPaleta];
												}
												lineDst[iByteDestino++] = pixelRGBAPaleta[0];	//R
												lineDst[iByteDestino++] = pixelRGBAPaleta[1];	//G
												lineDst[iByteDestino++] = pixelRGBAPaleta[2];	//B
												lineDst[iByteDestino++] = alpha;				//A
											}
										}
									}
									r = TRUE;
								}
							} else {
								errFnd = TRUE;
							}
						}
					} //Procesar desentrelazos a mapaBits
				} else if(opq->ihdr.interlMethd == 1){
					NBASSERT(opq->buffs.scanlines.sampleIdx >= 0 && opq->buffs.scanlines.sampleIdx <= 7)
					const STNBAdam7Sample* adamDef = &__adam7Defs[opq->buffs.scanlines.sampleIdx];
					const UI32 bytesPerPx = (opq->bmpProps.bitsPerPx / 8);
					if(opq->ihdr.pngColor == 0 || opq->ihdr.pngColor == 2 || opq->ihdr.pngColor == 4 || opq->ihdr.pngColor == 6){
						//Copy without PLTE
						const UI32 bytesSmpls = (bytesPerScanLn - 1);
						const UI32 compsPerPix = (opq->buffs.scanlines.bitsPerPix / opq->ihdr.bitsDepth); NBASSERT(compsPerPix > 0 && compsPerPix <= 4)
						UI32 iFila; for(iFila = 0; iFila < linesAtBuff; iFila++){
							const UI8* lineSrc	= &opq->buffs.scanlines.cur->data[1 + (iFila * bytesPerScanLn)]; //+1 para excluir el byte de entrelazado
							const UI32 lnDstIdx	= (adamDef->rows.start + ((opq->buffs.scanlines.cur->iLineStart + iFila) * adamDef->rows.inc)); NBASSERT(lnDstIdx >= 0 && lnDstIdx < opq->bmpProps.size.height)
							UI8* lineDst		= &pixsBuff[lnDstIdx * opq->bmpProps.bytesPerLine];
							if(opq->ihdr.bitsDepth == 8){
								UI32 i, divRem, pixIdx; UI8* pixDst = NULL;
								for(i = 0; i < bytesSmpls; i++){
									divRem = (i % compsPerPix);
									if(divRem == 0){
										pixIdx		= (adamDef->cols.start + ((i / compsPerPix) * adamDef->cols.inc)); NBASSERT(pixIdx >= 0 && pixIdx < opq->bmpProps.size.width)
										pixDst		= &lineDst[pixIdx * bytesPerPx];
									}
									pixDst[divRem]	= lineSrc[i];
								}
							} else if(opq->ihdr.bitsDepth == 16){
								UI32 i, divRem, pixIdx; UI8* pixDst = NULL;
								for(i = 0; i < bytesSmpls; i += 2){
									divRem = ((i / 2) % compsPerPix);
									if(divRem == 0){
										pixIdx		= (adamDef->cols.start + (((i / 2) / compsPerPix) * adamDef->cols.inc)); NBASSERT(pixIdx >= 0 && pixIdx < opq->bmpProps.size.width)
										pixDst		= &lineDst[pixIdx * bytesPerPx];
									}
									//Simply ignore the lower byte order
									pixDst[divRem]	= lineSrc[i];
								}
							} else {
								//PRINTF_INFO("Scanline-row(%d)-byte(%d), bmp-line(%d)-col(%d).\n", iFila, i, lnDstIdx, pixIdx);
								NBASSERT(opq->ihdr.bitsDepth == 1 || opq->ihdr.bitsDepth == 2 || opq->ihdr.bitsDepth == 4)
								const UI32 bitsPerSmpl	= opq->ihdr.bitsDepth;
								const UI32 bitsPerLine	= (opq->buffs.scanlines.sampleSz.width * opq->ihdr.bitsDepth);
								const UI8 multBase		= (0xFF / (0xFF >> (8 - opq->ihdr.bitsDepth)));
								UI8 val; UI32 iSmpl, iBit, divRem, pixIdx; UI8* pixDst = NULL;
								for(iBit = 0; iBit < bitsPerLine; iBit += bitsPerSmpl){
									iSmpl	= (iBit / bitsPerSmpl);
									divRem = (iSmpl % compsPerPix);
									if(divRem == 0){
										pixIdx		= (adamDef->cols.start + ((iSmpl / compsPerPix) * adamDef->cols.inc)); NBASSERT(pixIdx >= 0 && pixIdx < opq->bmpProps.size.width)
										pixDst		= &lineDst[pixIdx * bytesPerPx];
									}
									val				= NBPNG_GET_BITS(lineSrc, iBit, bitsPerSmpl);
									pixDst[divRem]	= val * multBase;
								}
							}
						}
						r = TRUE;
					} else if(opq->ihdr.pngColor == 3){
						if(opq->chunks.PLTEData == NULL){
							PRINTF_ERROR("PNG load, missing PLTE.\n");
						} else {
							if(opq->ihdr.bitsDepth == 8){
								if(opq->chunks.tRNSData == NULL){
									//Ciclo de copiado con paleta RGB sin paleta alpha
									UI32 iFila, iPix, pixsCount = opq->buffs.scanlines.sampleSz.width;
									for(iFila = 0; iFila < linesAtBuff; iFila++){
										const UI8* lineSrc	= &opq->buffs.scanlines.cur->data[1 + (iFila * bytesPerScanLn)]; //+1 para excluir el byte de entrelazado
										const UI32 lnDstIdx	= (adamDef->rows.start + ((opq->buffs.scanlines.cur->iLineStart + iFila) * adamDef->rows.inc)); NBASSERT(lnDstIdx >= 0 && lnDstIdx < opq->bmpProps.size.height)
										UI8* lineDst		= &pixsBuff[lnDstIdx * opq->bmpProps.bytesPerLine];
										for(iPix = 0; iPix < pixsCount; iPix++){
											UI8 indiceEnPaleta		= lineSrc[iPix];
											UI8* pixelRGBAPaleta	= &(opq->chunks.PLTEData[indiceEnPaleta * 3]); //x3, la paleta es RGB
											const UI32 pixIdx		= (adamDef->cols.start + (iPix * adamDef->cols.inc)); NBASSERT(pixIdx >= 0 && pixIdx < opq->bmpProps.size.width);
											UI8* pixDst				= &lineDst[pixIdx * bytesPerPx];
											pixDst[0]				= pixelRGBAPaleta[0]; //R
											pixDst[1] 				= pixelRGBAPaleta[1]; //G
											pixDst[2] 				= pixelRGBAPaleta[2]; //B
										}
									}
									r = TRUE;
								} else {
									//Ciclo de copiado con paleta RGB y paleta alpha
									UI32 iFila, iPix, pixsCount = opq->buffs.scanlines.sampleSz.width;
									for(iFila = 0; iFila < linesAtBuff; iFila++){
										const UI8* lineSrc	= &opq->buffs.scanlines.cur->data[1 + (iFila * bytesPerScanLn)]; //+1 para excluir el byte de entrelazado
										const UI32 lnDstIdx	= (adamDef->rows.start + ((opq->buffs.scanlines.cur->iLineStart + iFila) * adamDef->rows.inc)); NBASSERT(lnDstIdx >= 0 && lnDstIdx < opq->bmpProps.size.height)
										UI8* lineDst		= &pixsBuff[lnDstIdx * opq->bmpProps.bytesPerLine];
										for(iPix = 0; iPix < pixsCount; iPix++){
											UI8 indiceEnPaleta		= lineSrc[iPix];
											UI8* pixelRGBAPaleta	= &(opq->chunks.PLTEData[indiceEnPaleta * 3]); //x3, la paleta es RGB
											UI8 alpha				= 255;
											if(indiceEnPaleta < opq->chunks.tRNSSz){
												alpha				= opq->chunks.tRNSData[indiceEnPaleta];
											}
											const UI32 pixIdx		= (adamDef->cols.start + (iPix * adamDef->cols.inc)); NBASSERT(pixIdx >= 0 && pixIdx < opq->bmpProps.size.width);
											UI8* pixDst				= &lineDst[pixIdx * bytesPerPx];
											pixDst[0]				= pixelRGBAPaleta[0]; //R
											pixDst[1]				= pixelRGBAPaleta[1]; //G
											pixDst[2]				= pixelRGBAPaleta[2]; //B
											pixDst[3]				= alpha;				//A
										}
									}
									r = TRUE;
								}
							} else if(opq->ihdr.bitsDepth == 1 || opq->ihdr.bitsDepth == 2 || opq->ihdr.bitsDepth == 4){
								if(opq->chunks.tRNSData == NULL){
									//Ciclo de copiado con paleta RGB sin paleta alpha
									const UI32 bitsPerSmpl	= opq->ihdr.bitsDepth;
									const UI32 bitsPerLine	= (opq->buffs.scanlines.sampleSz.width * opq->ihdr.bitsDepth);
									UI32 iFila;
									for(iFila = 0; iFila < linesAtBuff; iFila++){
										const UI8* lineSrc	= &opq->buffs.scanlines.cur->data[1 + (iFila * bytesPerScanLn)]; //+1 para excluir el byte de entrelazado
										const UI32 lnDstIdx	= (adamDef->rows.start + ((opq->buffs.scanlines.cur->iLineStart + iFila) * adamDef->rows.inc)); NBASSERT(lnDstIdx >= 0 && lnDstIdx < opq->bmpProps.size.height)
										UI8* lineDst		= &pixsBuff[lnDstIdx * opq->bmpProps.bytesPerLine];
										UI32 iBit, pixIdx; UI8* pixDst = NULL; UI8 idxPLTE = 0; const UI8* rgbPLTE = NULL;
										for(iBit = 0; iBit < bitsPerLine; iBit += bitsPerSmpl){
											pixIdx		= (adamDef->cols.start + ((iBit / bitsPerSmpl) * adamDef->cols.inc)); NBASSERT(pixIdx >= 0 && pixIdx < opq->bmpProps.size.width)
											pixDst		= &lineDst[pixIdx * bytesPerPx];
											//
											idxPLTE		= NBPNG_GET_BITS(lineSrc, iBit, bitsPerSmpl);
											rgbPLTE		= &(opq->chunks.PLTEData[idxPLTE * 3]); //x3, la paleta es RGB
											pixDst[0]	= rgbPLTE[0]; //R
											pixDst[1] 	= rgbPLTE[1]; //G
											pixDst[2] 	= rgbPLTE[2]; //B
										}
									}
									r = TRUE;
								} else {
									//Ciclo de copiado con paleta RGB y paleta alpha
									const UI32 bitsPerSmpl	= opq->ihdr.bitsDepth;
									const UI32 bitsPerLine	= (opq->buffs.scanlines.sampleSz.width * opq->ihdr.bitsDepth);
									UI32 iFila;
									for(iFila = 0; iFila < linesAtBuff; iFila++){
										const UI8* lineSrc	= &opq->buffs.scanlines.cur->data[1 + (iFila * bytesPerScanLn)]; //+1 para excluir el byte de entrelazado
										const UI32 lnDstIdx	= (adamDef->rows.start + ((opq->buffs.scanlines.cur->iLineStart + iFila) * adamDef->rows.inc)); NBASSERT(lnDstIdx >= 0 && lnDstIdx < opq->bmpProps.size.height)
										UI8* lineDst		= &pixsBuff[lnDstIdx * opq->bmpProps.bytesPerLine];
										UI32 iBit, pixIdx; UI8* pixDst = NULL; UI8 alpha, idxPLTE = 0; const UI8* rgbPLTE = NULL;
										for(iBit = 0; iBit < bitsPerLine; iBit += bitsPerSmpl){
											pixIdx		= (adamDef->cols.start + ((iBit / bitsPerSmpl) * adamDef->cols.inc)); NBASSERT(pixIdx >= 0 && pixIdx < opq->bmpProps.size.width)
											pixDst		= &lineDst[pixIdx * bytesPerPx];
											//
											idxPLTE		= NBPNG_GET_BITS(lineSrc, iBit, bitsPerSmpl);
											rgbPLTE		= &(opq->chunks.PLTEData[idxPLTE * 3]); //x3, la paleta es RGB
											alpha		= 0;
											if(idxPLTE < opq->chunks.tRNSSz){
												alpha	= opq->chunks.tRNSData[idxPLTE];
											}
											pixDst[0]	= rgbPLTE[0]; //R
											pixDst[1] 	= rgbPLTE[1]; //G
											pixDst[2] 	= rgbPLTE[2]; //B
											pixDst[3]	= alpha;	  //A
										}
									}
									r = TRUE;
								}
							}
						}
					}
				}
			}
		}
	}
	return r;
}


//Scanlines buffer

void NBPngScanLinesBuffer_init(STNBPngScanLinesBuffer* obj){
	NBMemory_setZeroSt(*obj, STNBPngScanLinesBuffer);
}

void NBPngScanLinesBuffer_create(STNBPngScanLinesBuffer* obj, const UI32 width, const UI32 height, const UI32 bitsPerPix){
	const UI32 bitsPerLine = width * bitsPerPix;
	//Set props
	obj->iLineStart		= 0;
	obj->width			= width;
	obj->height			= height;
	obj->bytesPerLine	= 1 + (bitsPerLine / 8) + ((bitsPerLine % 8) != 0 ? 1 : 0);
	obj->dataToFill		= (obj->bytesPerLine * height);
	//Scanlines
	if(obj->dataSz != (obj->bytesPerLine * height)){
		//Release prev
		if(obj->data != NULL){
			NBMemory_free(obj->data);
			obj->data = NULL;
		}
		//Create new
		{
			obj->dataSz = (obj->bytesPerLine * height);
			if(obj->dataSz > 0){
				obj->data = NBMemory_allocTypes(BYTE, obj->dataSz);
			}
		}
	}
	//Prev scanline
	{
		if(obj->prevLine != NULL){
			NBMemory_free(obj->prevLine);
			obj->prevLine = NULL;
		}
		obj->prevLineSz	= 0;
	}
}

void NBPngScanLinesBuffer_createNext(STNBPngScanLinesBuffer* obj, const STNBPngScanLinesBuffer* prev, const UI32 height){
	//Copy last line
	{
		NBASSERT((prev->iLineStart == 0 && prev->prevLine == NULL && prev->prevLineSz == 0) || (prev->iLineStart != 0 && prev->prevLine != NULL && prev->prevLineSz != 0 && prev->prevLineSz == prev->bytesPerLine))
		//Copy last line
		if(!(prev->bytesPerLine > 0 && prev->data != NULL && prev->dataSz >= prev->bytesPerLine)){
			//Release prev buffer if incompatible
			if(obj->prevLine != NULL){
				NBMemory_free(obj->prevLine);
				obj->prevLine	= NULL;
				obj->prevLineSz	= 0;
			}
		} else {
			//Release prev buffer if incompatible
			if(obj->prevLine != NULL && obj->prevLineSz != prev->prevLineSz){
				NBMemory_free(obj->prevLine);
				obj->prevLine	= NULL;
				obj->prevLineSz	= 0;
			}
			//Ceate buffer
			if(obj->prevLine == NULL){
				obj->prevLineSz	= prev->bytesPerLine;
				obj->prevLine	= NBMemory_allocTypes(BYTE, obj->prevLineSz);
			}
			//Copy data
			NBASSERT(obj->prevLine != NULL && obj->prevLineSz > 0 && obj->prevLineSz == prev->bytesPerLine)
			if(obj->prevLine != NULL && obj->prevLineSz > 0 && obj->prevLineSz == prev->bytesPerLine){
				const BYTE* srcLn = &prev->data[(prev->height - 1) * prev->bytesPerLine];
				NBMemory_copy(obj->prevLine, srcLn, prev->bytesPerLine);
				obj->prevLineSz = prev->bytesPerLine;
			}
		}
	}
	//Scanlines
	if(obj->dataSz < (prev->bytesPerLine * height)){
		//Release prev
		if(obj->data != NULL){
			NBMemory_free(obj->data);
			obj->data = NULL;
		}
		//Create new
		{
			obj->dataSz = (prev->bytesPerLine * height);
			if(obj->dataSz > 0){
				obj->data = NBMemory_allocTypes(BYTE, obj->dataSz);
			}
		}
	}
	
	//Set at the end to allow call "createNext" over the same buffer (reuse)
	obj->iLineStart		= (prev->iLineStart + prev->height);
	obj->width			= prev->width;
	obj->height			= height;
	obj->bytesPerLine	= prev->bytesPerLine;
	obj->dataToFill		= (obj->bytesPerLine * height);
}

void NBPngScanLinesBuffer_release(STNBPngScanLinesBuffer* obj){
	if(obj->prevLine != NULL){
		NBMemory_free(obj->prevLine);
		obj->prevLine = NULL;
		obj->prevLineSz = 0;
	}
	if(obj->data != NULL){
		NBMemory_free(obj->data);
		obj->data		= NULL;
		obj->dataSz		= 0;
	}
}
