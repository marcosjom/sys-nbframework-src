//
//  NBFontsGlyphsStore.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/8/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/fonts/NBFontsGlyphsStore.h"
#include "nb/files/NBFilesPkgIndex.h"

//Freetype2
#ifdef NB_LIB_FREETYPE_SYSTEM
#   include <freetype/ftoutln.h>    //for "FT_Outline_Render"
#   include <freetype/ftadvanc.h>    //for "FT_Get_Advance"
#else
#   include <ft2build.h>
#   include FT_FREETYPE_H
#   include <ftoutln.h>    //for "FT_Outline_Render"
#   include <ftadvanc.h>    //for "FT_Get_Advance"
#endif

typedef struct STNBFontsGlyphsStoreFound_ {
	UI32	family;			//in strs
	UI32	subfamily;		//in strs
	UI32	filename;		//in strs
	UI8		iFont;			//index inside the file
	UI8		styleMask;		//ENNBFontStyleBit_*
} STNBFontsGlyphsStoreFound;

typedef struct STNBFontsGlyphsStoreMap_ {
	STNBFilesPkgIndex	files;	//files explored
	STNBArray			fonts;	//STNBFontsGlyphsStoreFound, fonts found
	STNBString			strs;
} STNBFontsGlyphsStoreMap;

typedef struct STNBFontsGlyphsStoreFont_ {
	STNBFontGlyphs	font;
	FT_Library*		ftLib;
	FT_Face			ftFont;
} STNBFontsGlyphsStoreFont;

typedef struct STNBFontsGlyphsStoreOpq_ {
	STNBFilesystem*			filesystem;
	//Search paths
	STNBArray				srchPaths;	//UI32
	//Files explored
	STNBFontsGlyphsStoreMap fontsMap;
	//Fonts loaded
	STNBArray				loaded;		//STNBFontsGlyphsStoreFont*
	//Freetype
	FT_Library				ftLib;
	//
	STNBString				strs;
	UI32					retainCount;
} STNBFontsGlyphsStoreOpq;

void NBFontsGlyphsStore_init(STNBFontsGlyphsStore* obj, STNBFilesystem* filesystem){
	STNBFontsGlyphsStoreOpq* opq = obj->opaque = NBMemory_allocType(STNBFontsGlyphsStoreOpq);
	NBMemory_setZeroSt(*opq, STNBFontsGlyphsStoreOpq);
	//
	opq->filesystem	= filesystem;
	//Search paths
	{
		NBArray_init(&opq->srchPaths, sizeof(UI32), NULL);
	}
	//Files explored
	{
		STNBFontsGlyphsStoreMap* map = &opq->fontsMap;
		NBFilesPkgIndex_init(&map->files);
		NBArray_init(&map->fonts, sizeof(STNBFontsGlyphsStoreFound), NULL);
		NBString_init(&map->strs);
	}
	//Fonts loaded
	{
		NBArray_init(&opq->loaded, sizeof(STNBFontsGlyphsStoreFont*), NULL);
	}
	//Freetype
	{
		opq->ftLib	= NULL;
		const FT_Error ftErr = FT_Init_FreeType(&opq->ftLib);
		if(ftErr != 0){
			PRINTF_ERROR("Could not init FtLibrary, returned %d.\n", ftErr);
		}
	}
	NBString_init(&opq->strs);
	opq->retainCount = 1;
}

void NBFontsGlyphsStore_retain(STNBFontsGlyphsStore* obj){
	STNBFontsGlyphsStoreOpq* opq = (STNBFontsGlyphsStoreOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount++;
}

void NBFontsGlyphsStore_release(STNBFontsGlyphsStore* obj){
	NBASSERT(obj->opaque != NULL)
	STNBFontsGlyphsStoreOpq* opq = (STNBFontsGlyphsStoreOpq*)obj->opaque;
	NBASSERT(opq->retainCount > 0)
	opq->retainCount--;
	if(opq->retainCount == 0){
		//
		opq->filesystem	= NULL;
		//
		//Search paths
		{
			NBArray_release(&opq->srchPaths);
		}
		//Files explored
		{
			STNBFontsGlyphsStoreMap* map = &opq->fontsMap;
			NBFilesPkgIndex_release(&map->files);
			NBArray_release(&map->fonts);
			NBString_release(&map->strs);
		}
		//Fonts loaded
		{
			UI32 i;
			for(i = 0; i < opq->loaded.use; i++){
				STNBFontsGlyphsStoreFont* f = NBArray_itmValueAtIndex(&opq->loaded, STNBFontsGlyphsStoreFont*, i);
				NBFontGlyphs_release(&f->font);
				FT_Done_Face(f->ftFont);
				NBMemory_free(f);
			}
			NBArray_empty(&opq->loaded);
			NBArray_release(&opq->loaded);
		}
		//Freetype
		{
			if(opq->ftLib != NULL){
				FT_Done_FreeType(opq->ftLib);
				opq->ftLib = NULL;
			}
			opq->ftLib	= NULL;
		}
		NBString_release(&opq->strs);
		//
		NBMemory_free(obj->opaque);
		obj->opaque = NULL;
	}
}

//

void NBFontsGlyphsStore_addSrchPath(STNBFontsGlyphsStore* obj, const char* srchPath){
	STNBFontsGlyphsStoreOpq* opq = (STNBFontsGlyphsStoreOpq*)obj->opaque;
	const UI32 path = opq->strs.length;
	NBString_concat(&opq->strs, srchPath);
	NBString_concatByte(&opq->strs, '\0');
	NBArray_addValue(&opq->srchPaths, path);
}

UI8 NBFontsGlyphsStore_styleMaskOptsCount(const UI8 styleMask){
	UI8 r = 0;
	UI8 cpy = styleMask;
	while(cpy != 0){
		if((cpy & 1) != 0) r++;
		cpy = (cpy & 254) >> 1;
	}
	return r;
}

UI8 NBFontsGlyphsStore_styleMaskMatchOptsCount(const UI8 styleMask, const UI8 otherStyleMask){
	UI8 r = 0;
	UI8 cpy2 = styleMask;
	UI8 cpy = otherStyleMask;
	while(cpy2 != 0 && cpy != 0){
		if((cpy2 & 1) != 0 && (cpy & 1) != 0) r++;
		cpy2 = (cpy2 & 254) >> 1;
		cpy = (cpy & 254) >> 1;
	}
	return r;
}

BOOL NBFontsGlyphsStore_isBetterMatch(const char* srchFamily, const char* srchSubfamily, const UI8 srchStyleMask, const char* othrFamily, const char* othrSubfamily, const UI8 othrStyleMask, BOOL* rIsExact, UI8* rMaskMatchs){
	BOOL r = FALSE;
	if(NBString_strIsEqual(srchFamily, othrFamily)){
		if(srchSubfamily != NULL && othrSubfamily != NULL){
			r			= TRUE;
			*rIsExact	= NBString_strIsEqual(srchSubfamily, othrSubfamily);
		} else {
			if(srchStyleMask == othrStyleMask){
				r			= TRUE;
				*rIsExact	= TRUE;
			} else {
				const UI8 maskMatchs = NBFontsGlyphsStore_styleMaskMatchOptsCount(srchStyleMask, othrStyleMask);
				if(*rMaskMatchs == 0 || *rMaskMatchs < maskMatchs){
					r			= TRUE;
					*rMaskMatchs = maskMatchs;
				}
			}
		}
	}
	return r;
}

//Read font-file
typedef struct STNBFTReadr_ {
	STNBFileRef				file;
	BOOL					lockAtRead;
	BOOL					freeAtClose;
	//
	struct FT_MemoryRec_	ftMem;
	FT_StreamRec			ftStream;
} STNBFTReadr;

unsigned long NBFTReadr_read(FT_Stream stream, unsigned long offset, unsigned char*  buffer, unsigned long count);
void NBFTReadr_close(FT_Stream stream);
void* NBFTReadr_mAlloc(FT_Memory memory, long size);
void NBFTReadr_mFree(FT_Memory memory, void* block);
void* NBFTReadr_mRealloc(FT_Memory memory, long cur_size, long new_size, void* block);

//Peek fonts
BOOL NBFontsGlyphsStore_peekFontsInFolder(STNBFontsGlyphsStoreOpq* opq, const char* folderpath, const char* family, const char* subfamily, const UI8 styleMask, STNBString* dstFileExactFound, SI32* dstFontIdx);
BOOL NBFontsGlyphsStore_peekFontsInFilepath(STNBFontsGlyphsStoreOpq* opq, const char* filepath, const char* family, const char* subfamily, const UI8 styleMask, SI32* dstFontIdx);
BOOL NBFontsGlyphsStore_peekFontsInFilePos(STNBFontsGlyphsStoreOpq* opq, STNBFileRef file, STNBString* dstFamily, STNBString* dstSubfamily, UI8* dstStyleMask);

//Query fonts
UI32					NBFontsGlyphsStore_getShapeId_(void* obj, const UI32 unicode);
STNBFontShapeMetrics	NBFontsGlyphsStore_getShapeMetrics_(void* obj, const UI32 shapeId);
BOOL					NBFontsGlyphsStore_getShapeLines_(void* obj, const UI32 shapeId, const UI32 fontSize, const STNBMatrix matrix, STNBFontLinesData* dstData);

STNBFontGlyphs* NBFontsGlyphsStore_loadFontFromPath(STNBFontsGlyphsStore* obj, const char* filepath, const UI8 iFont, const UI8 styleMask){
	STNBFontGlyphs* r   = NULL;
	STNBFontsGlyphsStoreOpq* opq = (STNBFontsGlyphsStoreOpq*)obj->opaque;
	STNBFileRef file	= NBFile_alloc(NULL);
	if(!NBFilesystem_open(opq->filesystem, filepath, ENNBFileMode_Read, file)){
		PRINTF_ERROR("Could not open font file: '%s'.\n", filepath);
		NBFile_release(&file);
        NBFile_null(&file);
	} else {
		NBFile_lock(file);
		STNBFTReadr* rdr	= NBMemory_allocType(STNBFTReadr);
		rdr->file			= file;
		rdr->lockAtRead		= FALSE;
		rdr->freeAtClose	= TRUE;
		{
			NBMemory_set(&rdr->ftMem, 0, sizeof(rdr->ftMem));
			rdr->ftMem.user		= rdr;
			rdr->ftMem.alloc	= NBFTReadr_mAlloc;
			rdr->ftMem.free		= NBFTReadr_mFree;
			rdr->ftMem.realloc	= NBFTReadr_mRealloc;
		}
		{
			NBFile_seek(file, 0, ENNBFileRelative_End);
			const SI64 fileSz	= NBFile_curPos(file); NBASSERT(fileSz > 0)
			NBMemory_set(&rdr->ftStream, 0, sizeof(rdr->ftStream));
			rdr->ftStream.base					= NULL;
			rdr->ftStream.size					= (unsigned long)fileSz;
			rdr->ftStream.pos					= 0;
			rdr->ftStream.descriptor.pointer	= rdr;
			rdr->ftStream.pathname.pointer		= NULL; //filepath;
			rdr->ftStream.read					= NBFTReadr_read;
			rdr->ftStream.close					= NBFTReadr_close;
			rdr->ftStream.memory				= &rdr->ftMem;
		}
		//Start stream
		STNBFontsGlyphsStoreFont* font =  NBMemory_allocType(STNBFontsGlyphsStoreFont);
		NBMemory_set(font, 0, sizeof(*font));
		FT_Error ftError = 0;
		FT_Open_Args  args;
		args.flags    = FT_OPEN_STREAM;
		args.stream   = &rdr->ftStream;
		if((ftError = FT_Open_Face(opq->ftLib, &args, iFont, &font->ftFont)) != 0){
			PRINTF_ERROR("Could not open font file: '%s' (subfont %d).\n", filepath, iFont);
			NBFile_release(&file);
            NBFile_null(&file);
			NBMemory_free(font); font = NULL;
			NBMemory_free(rdr); rdr = NULL;
		} else {
			PRINTF_INFO("Font loaded: '%s' (subfont %d).\n", filepath, iFont);
			{
				STNBFontNativeI itf;
				NBMemory_set(&itf, 0, sizeof(itf));
				itf.getShapeId		= NBFontsGlyphsStore_getShapeId_;
				itf.getShapeMetrics	= NBFontsGlyphsStore_getShapeMetrics_;
				itf.getShapeLines	= NBFontsGlyphsStore_getShapeLines_;
				NBFontGlyphs_initWithProps(&font->font, font->ftFont->units_per_EM, font->ftFont->family_name, font->ftFont->style_name, styleMask, font->ftFont->ascender, font->ftFont->descender, font->ftFont->height, &itf, font);
			}
			font->ftLib = &opq->ftLib;
			NBArray_addValue(&opq->loaded, font);
			r = &font->font;
			rdr->lockAtRead = TRUE;
		}
		NBFile_unlock(file);
	}
	return r;
}
	
STNBFontGlyphs* NBFontsGlyphsStore_getFont(STNBFontsGlyphsStore* obj, const char* family, const char* subfamily, const UI8 styleMask, const ENNBFontSrchMode srchMode){
	STNBFontGlyphs* r	= NULL;		//Current best found font
	STNBFontsGlyphsStoreOpq* opq = (STNBFontsGlyphsStoreOpq*)obj->opaque;
	STNBFontsGlyphsStoreMap* map = &opq->fontsMap;
	UI32 fontsLoaded	= 0;
	BOOL rIsExact		= FALSE;	//Current font is exact
	//Best already-loaded font
	STNBFontsGlyphsStoreFont* bestLoaded = NULL;
	UI8 bestLoadedMaskMatchs	= 0;
	//Best already-mapped font
	STNBFontsGlyphsStoreFound* bestMapped = NULL;
	UI8 bestMappedMaskMatchs	= 0;
	//Search in loaded fonts
	if(r == NULL || !rIsExact){
		UI32 i;
		for(i = 0; i < opq->loaded.use && !rIsExact; i++){
			STNBFontsGlyphsStoreFont* f = NBArray_itmValueAtIndex(&opq->loaded, STNBFontsGlyphsStoreFont*, i);
			const char* fFamily		= NULL;
			const char* fSubfamily	= NULL;
			UI8 fStyleMask			= 0;
			NBFontGlyphs_getStyle(&f->font, &fFamily, &fSubfamily, &fStyleMask);
			if(NBFontsGlyphsStore_isBetterMatch(family, subfamily, styleMask, fFamily, fSubfamily, fStyleMask, &rIsExact, &bestLoadedMaskMatchs)){
				if(rIsExact){
					//PRINTF_INFO("FontsGlyphsStore, exact font '%s'/'%s'/%d found at already-loaded-list.\n", family, subfamily, styleMask);
					r = &f->font;
				} else {
					//PRINTF_INFO("FontsGlyphsStore, candidate font '%s'/'%s'/%d found at already-loaded-list for '%s'/'%s'/%d.\n", fFamily, fSubfamily, fStyleMask, family, subfamily, styleMask);
					bestLoaded = f;
				}
			}
		}
	}
	//Search exact-match in mapped fonts
	UI32 iMapped = 0;
	if(r == NULL || !rIsExact){
		for(iMapped = 0; iMapped < map->fonts.use && !rIsExact; iMapped++){
			STNBFontsGlyphsStoreFound* f = NBArray_itmPtrAtIndex(&map->fonts, STNBFontsGlyphsStoreFound, iMapped);
			const char* fFamily			= &map->strs.str[f->family];
			const char* fSubfamily		= &map->strs.str[f->subfamily];
			const UI8 fStyleMask		= f->styleMask;
			if(NBFontsGlyphsStore_isBetterMatch(family, subfamily, styleMask, fFamily, fSubfamily, fStyleMask, &rIsExact, &bestMappedMaskMatchs)){
				if(rIsExact){
					//PRINTF_INFO("FontsGlyphsStore, exact font '%s'/'%s'/%d found at already-mapped-list (loading).\n", family, subfamily, styleMask);
					r = NBFontsGlyphsStore_loadFontFromPath(obj, &map->strs.str[f->filename], f->iFont, fStyleMask);
					if(r != NULL) fontsLoaded++;
				} else {
					//PRINTF_INFO("FontsGlyphsStore, candidate font '%s'/'%s'/%d found at already-mapped-list for '%s'/'%s'/%d (loading).\n", fFamily, fSubfamily, fStyleMask, family, subfamily, styleMask);
					bestMapped = f;
				}
			}
		}
	}
	//Continue mapping
	if(r == NULL || !rIsExact){
		STNBString fileFound; SI32 iFont = -1;
		NBString_init(&fileFound);
		UI32 i; for(i = 0; i < opq->srchPaths.use && fileFound.length == 0 && iFont < 0; i++){
			const UI32 p = NBArray_itmValueAtIndex(&opq->srchPaths, UI32, i);
			const char* path = &opq->strs.str[p];
			//PRINTF_INFO("FontsGlyphsStore, peeking fonts at folder '%s'.\n", path);
			NBFontsGlyphsStore_peekFontsInFolder(opq, path, family, subfamily, styleMask, &fileFound, &iFont);
			if(fileFound.length > 0 && iFont >= 0){
				rIsExact = TRUE;
				r = NBFontsGlyphsStore_loadFontFromPath(obj, fileFound.str, (UI8)iFont, styleMask);
				if(r != NULL) fontsLoaded++;
			}
		}
		NBString_release(&fileFound);
	}
	//Continue search of better match (only if map changed)
	if((r == NULL || !rIsExact) && srchMode != ENNBFontSrchMode_exact){
		NBASSERT(iMapped <= map->fonts.use)
		for(; iMapped < map->fonts.use && !rIsExact; iMapped++){
			STNBFontsGlyphsStoreFound* f	= NBArray_itmPtrAtIndex(&map->fonts, STNBFontsGlyphsStoreFound, iMapped);
			const char* fFamily				= &map->strs.str[f->family];
			const char* fSubfamily			= &map->strs.str[f->subfamily];
			const UI8 fStyleMask			= f->styleMask;
			if(NBFontsGlyphsStore_isBetterMatch(family, subfamily, styleMask, fFamily, fSubfamily, fStyleMask, &rIsExact, &bestMappedMaskMatchs)){
				NBASSERT(!rIsExact) //If this fails, "NBFontsGlyphsStore_peekFontsInFolder" failed to correctly identify the font at peek.
				//PRINTF_INFO("FontsGlyphsStore, candidate font '%s'/'%s'/%d found at new-mapped-list (loading).\n", family, subfamily, styleMask);
				bestMapped = f;
			}
		}
	}
	//Return
	if(srchMode == ENNBFontSrchMode_exact){
		if(!rIsExact){
			r = NULL;
		}
	} else if(r == NULL || !rIsExact){
		if(bestLoaded != NULL && bestLoadedMaskMatchs >= bestMappedMaskMatchs){
			//Use already-loaded (priorize)
			const char* fFamily		= NULL;
			const char* fSubfamily	= NULL;
			UI8 fStyleMask			= 0;
			NBFontGlyphs_getStyle(&bestLoaded->font, &fFamily, &fSubfamily, &fStyleMask);
			//PRINTF_INFO("FontsGlyphsStore, font not-found, using already-loaded best match: '%s' '%s' styleMsk(%d%s%s%s%s%s%s%s%s) for '%s' '%s' styleMsk(%d%s%s%s%s%s%s%s%s).\n", fFamily, fSubfamily, fStyleMask, (fStyleMask != 0 ? ":" : ""), (fStyleMask & ENNBFontStyleBit_Bold) != 0 ? " bold" : "", (fStyleMask & ENNBFontStyleBit_Italic) != 0 ? " italic" : "", (fStyleMask & ENNBFontStyleBit_Undelined) != 0 ? " underlined" : "", (fStyleMask & ENNBFontStyleBit_Striked) != 0 ? " striked" : "", (fStyleMask & ENNBFontStyleBit_Shadowed) != 0 ? " shadowed" : "", (fStyleMask & ENNBFontStyleBit_Condensed) != 0 ? " condensed" : "", (fStyleMask & ENNBFontStyleBit_Extended) != 0 ? " extended" : "", family, subfamily, styleMask, (styleMask != 0 ? ":" : ""), (styleMask & ENNBFontStyleBit_Bold) != 0 ? " bold" : "", (styleMask & ENNBFontStyleBit_Italic) != 0 ? " italic" : "", (styleMask & ENNBFontStyleBit_Undelined) != 0 ? " underlined" : "", (styleMask & ENNBFontStyleBit_Striked) != 0 ? " striked" : "", (styleMask & ENNBFontStyleBit_Shadowed) != 0 ? " shadowed" : "", (styleMask & ENNBFontStyleBit_Condensed) != 0 ? " condensed" : "", (styleMask & ENNBFontStyleBit_Extended) != 0 ? " extended" : "");
			r = &bestLoaded->font;
		} else if(bestMapped != NULL){
			//Load new font
			STNBFontsGlyphsStoreMap* map = &opq->fontsMap;
			const UI8 fStyleMask		= bestMapped->styleMask;
			//const char* fFamily		= &map->strs.str[bestMapped->family];
			//const char* fSubfamily	= &map->strs.str[bestMapped->subfamily];
			//PRINTF_INFO("FontsGlyphsStore, font not-found, loading best match: '%s' '%s' styleMsk(%d%s%s%s%s%s%s%s%s) for '%s' '%s' styleMsk(%d%s%s%s%s%s%s%s%s).\n", fFamily, fSubfamily, fStyleMask, (fStyleMask != 0 ? ":" : ""), (fStyleMask & ENNBFontStyleBit_Bold) != 0 ? " bold" : "", (fStyleMask & ENNBFontStyleBit_Italic) != 0 ? " italic" : "", (fStyleMask & ENNBFontStyleBit_Undelined) != 0 ? " underlined" : "", (fStyleMask & ENNBFontStyleBit_Striked) != 0 ? " striked" : "", (fStyleMask & ENNBFontStyleBit_Shadowed) != 0 ? " shadowed" : "", (fStyleMask & ENNBFontStyleBit_Condensed) != 0 ? " condensed" : "", (fStyleMask & ENNBFontStyleBit_Extended) != 0 ? " extended" : "", family, subfamily, styleMask, (styleMask != 0 ? ":" : ""), (styleMask & ENNBFontStyleBit_Bold) != 0 ? " bold" : "", (styleMask & ENNBFontStyleBit_Italic) != 0 ? " italic" : "", (styleMask & ENNBFontStyleBit_Undelined) != 0 ? " underlined" : "", (styleMask & ENNBFontStyleBit_Striked) != 0 ? " striked" : "", (styleMask & ENNBFontStyleBit_Shadowed) != 0 ? " shadowed" : "", (styleMask & ENNBFontStyleBit_Condensed) != 0 ? " condensed" : "", (styleMask & ENNBFontStyleBit_Extended) != 0 ? " extended" : "");
			r = NBFontsGlyphsStore_loadFontFromPath(obj, &map->strs.str[bestMapped->filename], bestMapped->iFont, fStyleMask);
			if(r != NULL){
				fontsLoaded++;
			}
		}
	}
	NBASSERT(fontsLoaded <= 1) //Optimize; only one font should be loaded (the exact or best match)
	return r;
}

//Peek into font files

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

BOOL NBFontsGlyphsStore_peekFontsInFolder(STNBFontsGlyphsStoreOpq* opq, const char* folderpath, const char* family, const char* subfamily, const UI8 styleMask, STNBString* dstFileExactFound, SI32* dstFontIdx){
	BOOL r = FALSE;
	STNBString filesStrs, filepath;
	STNBArray filesIdxs;
	NBString_init(&filesStrs);
	NBString_init(&filepath);
	NBArray_init(&filesIdxs, sizeof(STNBFilesystemFile), NULL);
	//PRINTF_INFO("FontsGlyphsStore, mapping folder: '%s'.\n", folderpath);
	if(!NBFilesystem_getFiles(opq->filesystem, folderpath, FALSE, &filesStrs, &filesIdxs)){
		//PRINTF_ERROR("FontsGlyphsStore, could not get files-list at: '%s'.\n", folderpath);
	} else {
		UI32 i; for(i = 0; i < filesIdxs.use; i++){
			const STNBFilesystemFile fp = NBArray_itmValueAtIndex(&filesIdxs, STNBFilesystemFile, i);
			const char* fpath = &filesStrs.str[fp.name];
			NBString_empty(&filepath);
			NBString_concat(&filepath, folderpath);
			if(filepath.str[filepath.length - 1] != '/'){
				NBString_concatByte(&filepath, '/');
			}
			NBString_concat(&filepath, fpath);
			BOOL isFont  = FALSE;
			if(filepath.length > 3){
				const char* ext4 = &filepath.str[filepath.length - 4];
				if(ext4[0] == '.'){
					if((ext4[1] == 't' || ext4[1] == 'T') && (ext4[2] == 't' || ext4[2] == 'T') && (ext4[3] == 'f' || ext4[3] == 'F')){ //.ttf
						isFont = TRUE;
					} else if((ext4[1] == 'o' || ext4[1] == 'O') && (ext4[2] == 't' || ext4[2] == 'T') && (ext4[3] == 'f' || ext4[3] == 'F')){ //.otf
						isFont = TRUE;
					} else if((ext4[1] == 't' || ext4[1] == 'T') && (ext4[2] == 't' || ext4[2] == 'T') && (ext4[3] == 'c' || ext4[3] == 'C')){ //.ttc
						isFont = TRUE;
					}
				}
			}
			if(isFont){
				if(NBFilesPkgIndex_fileExists(&opq->fontsMap.files, filepath.str)){
					//PRINTF_INFO("FontsGlyphsStore, file already peeked: '%s'.\n", filepath.str);
				} else {
					//Peek
					//PRINTF_INFO("FontsGlyphsStore, peeking file: '%s'.\n", filepath.str);
					SI32 fontIdx = -1;
					if(!NBFontsGlyphsStore_peekFontsInFilepath(opq, filepath.str, family, subfamily, styleMask, &fontIdx)){
						//PRINTF_ERROR("FontsGlyphsStore, could not peek font file: '%s'.\n", filepath.str);
					} else {
						if(fontIdx >= 0){
							//PRINTF_INFO("FontsGlyphsStore, exact font found at file-idx(%d): '%s'.\n", fontIdx, filepath.str);
							NBString_set(dstFileExactFound, filepath.str);
							*dstFontIdx = fontIdx;
							break;
						}
					}
				}
			}
		}
	}
	NBArray_release(&filesIdxs);
	NBString_release(&filepath);
	NBString_release(&filesStrs);
	return r;
}

BOOL NBFontsGlyphsStore_peekFontsInFilepath(STNBFontsGlyphsStoreOpq* opq, const char* filepath, const char* pFamily, const char* pSubfamily, const UI8 pStyleMask, SI32* dstFontIdx){
	BOOL r = FALSE;
	STNBFileRef file = NBFile_alloc(NULL);
	if(!NBFilesystem_open(opq->filesystem, filepath, ENNBFileMode_Read, file)){
		//PRINTF_ERROR("NBFontsGlyphsStore, Could not open file '%s'.\n", filepath);
	} else {
		NBFile_lock(file);
		UI8 first32[4];
		if(NBFile_read(file, first32, 4) != 4){
			//PRINTF_ERROR("NBFontsGlyphsStore, Could not read first 4 bytes.\n");
		} else {
			if(!(first32[0] == 't' && first32[1] == 't' && first32[2] == 'c' && first32[3] == 'f')){
				//Not 'ttcf', asume the file has only one font
				if(!NBFile_seek(file, 0, ENNBFileRelative_Start)){
					NBASSERT(FALSE)
				} else {
					STNBString family, subfamily; UI8 styleMask = 0;
					NBString_init(&family);
					NBString_init(&subfamily);
					if(!NBFontsGlyphsStore_peekFontsInFilePos(opq, file, &family, &subfamily, &styleMask)){
						//PRINTF_ERROR("NBFontsGlyphsStore, Could not peek fonts in filepath: '%s'.\n", filepath);
						//NBASSERT(FALSE)
					} else {
						PRINTF_INFO("FontsGlyphsStore, found font: '%s' '%s' styleMsk(%d%s%s%s%s%s%s%s%s).\n", family.str, subfamily.str, styleMask, (styleMask != 0 ? ":" : ""), (styleMask & ENNBFontStyleBit_Bold) != 0 ? " bold" : "", (styleMask & ENNBFontStyleBit_Italic) != 0 ? " italic" : "", (styleMask & ENNBFontStyleBit_Undelined) != 0 ? " underlined" : "", (styleMask & ENNBFontStyleBit_Striked) != 0 ? " striked" : "", (styleMask & ENNBFontStyleBit_Shadowed) != 0 ? " shadowed" : "", (styleMask & ENNBFontStyleBit_Condensed) != 0 ? " condensed" : "", (styleMask & ENNBFontStyleBit_Extended) != 0 ? " extended" : "");
						//Add to found
						{
							STNBFontsGlyphsStoreFound f;
							f.family	= opq->fontsMap.strs.length; NBString_concat(&opq->fontsMap.strs, family.str); NBString_concatByte(&opq->fontsMap.strs, '\0');
							f.subfamily	= opq->fontsMap.strs.length; NBString_concat(&opq->fontsMap.strs, subfamily.str); NBString_concatByte(&opq->fontsMap.strs, '\0');
							f.styleMask	= styleMask;
							f.filename	= opq->fontsMap.strs.length; NBString_concat(&opq->fontsMap.strs, filepath); NBString_concatByte(&opq->fontsMap.strs, '\0');
							f.iFont		= 0;
							NBArray_addValue(&opq->fontsMap.fonts, f);
						}
						//Compare with search
						if(NBString_isEqual(&family, pFamily)){
							if(pSubfamily != NULL){
								if(NBString_isEqual(&subfamily, pSubfamily)){
									*dstFontIdx = 0;
								}
							} else if(pStyleMask == styleMask){
								*dstFontIdx = 0;
							}
						}
						r = TRUE;
					}
					NBString_release(&subfamily);
					NBString_release(&family);
				}
			} else {
				//TTCF, the file could have multiple fonts
				UI16 bigVer = 0, lilVer = 0;
                NBFile_read(file, &bigVer, sizeof(bigVer)); _invUI16(&bigVer);
				NBFile_read(file, &lilVer, sizeof(lilVer)); _invUI16(&lilVer);
				if(!((bigVer == 1 || bigVer == 2) && lilVer == 0)){
					//PRINTF_ERROR("NBFontsGlyphsStore, Unexpected font collection version.\n");
				} else {
					UI32 fontsCount	= 0; NBFile_read(file, &fontsCount, sizeof(fontsCount)); _invUI32(&fontsCount);
					if(fontsCount == 0){
						//PRINTF_WARNING("NBFontsGlyphsStore, Fonts collection is empty.\n");
					} else {
						//Load fonts indexes
						UI32* fontsPos = NBMemory_allocTypes(UI32, fontsCount);
						if(NBFile_read(file, fontsPos, sizeof(UI32) * fontsCount) != (sizeof(UI32) * fontsCount)){
							//PRINTF_ERROR("NBFontsGlyphsStore, Could not read all fonts positions.\n");
						} else {
							r = TRUE;
							UI32 i; for(i = 0; i < fontsCount; i++){
								UI32 fontPos = fontsPos[i]; _invUI32(&fontPos);
								if(!NBFile_seek(file, fontPos, ENNBFileRelative_Start)){
									r = FALSE; NBASSERT(FALSE)
								} else {
									STNBString family, subfamily; UI8 styleMask = 0;
									NBString_init(&family);
									NBString_init(&subfamily);
									if(!NBFontsGlyphsStore_peekFontsInFilePos(opq, file, &family, &subfamily, &styleMask)){
										r = FALSE; NBASSERT(FALSE)
									} else {
										PRINTF_INFO("NBFontsGlyphsStore, Found font: '%s' '%s' styleMsk(%d%s%s%s%s%s%s%s%s).\n", family.str, subfamily.str, styleMask, (styleMask != 0 ? ":" : ""), (styleMask & ENNBFontStyleBit_Bold) != 0 ? " bold" : "", (styleMask & ENNBFontStyleBit_Italic) != 0 ? " italic" : "", (styleMask & ENNBFontStyleBit_Undelined) != 0 ? " underlined" : "", (styleMask & ENNBFontStyleBit_Striked) != 0 ? " striked" : "", (styleMask & ENNBFontStyleBit_Shadowed) != 0 ? " shadowed" : "", (styleMask & ENNBFontStyleBit_Condensed) != 0 ? " condensed" : "", (styleMask & ENNBFontStyleBit_Extended) != 0 ? " extended" : "");
										//Add to found
										{
											STNBFontsGlyphsStoreFound f;
											f.family	= opq->fontsMap.strs.length; NBString_concat(&opq->fontsMap.strs, family.str); NBString_concatByte(&opq->fontsMap.strs, '\0');
											f.subfamily	= opq->fontsMap.strs.length; NBString_concat(&opq->fontsMap.strs, subfamily.str); NBString_concatByte(&opq->fontsMap.strs, '\0');
											f.styleMask	= styleMask;
											f.filename	= opq->fontsMap.strs.length; NBString_concat(&opq->fontsMap.strs, filepath); NBString_concatByte(&opq->fontsMap.strs, '\0');
											f.iFont		= (UI8)i;
											NBArray_addValue(&opq->fontsMap.fonts, f);
										}
										//Compare with search
										if(NBString_isEqual(&family, pFamily)){
											if(pSubfamily != NULL){
												if(NBString_isEqual(&subfamily, pSubfamily)){
													*dstFontIdx = i;
												}
											} else if(pStyleMask == styleMask){
												*dstFontIdx = i;
											}
										}
									}
									NBString_release(&subfamily);
									NBString_release(&family);
								}
							}
						}
						NBMemory_free(fontsPos);
					}
				}
			}
		}
		NBFile_unlock(file);
		//Add file to peeked
		{
			NBFilesPkgIndex_addFile(&opq->fontsMap.files, filepath, 0, 0);
		}
	}
	NBFile_release(&file);
	return r;
}

BOOL NBFontsGlyphsStore_peekFontsInFilePos(STNBFontsGlyphsStoreOpq* opq, STNBFileRef file, STNBString* dstFamily, STNBString* dstSubfamily, UI8* dstStyleMask){
	BOOL r = FALSE;
	UI16 bigVer			= 0; NBFile_read(file, &bigVer, sizeof(bigVer));			_invUI16(&bigVer);
	UI16 lilVer			= 0; NBFile_read(file, &lilVer, sizeof(lilVer));			_invUI16(&lilVer);
	UI16 tablesCount	= 0; NBFile_read(file, &tablesCount, sizeof(tablesCount));	_invUI16(&tablesCount);
	UI16 srchRng		= 0; NBFile_read(file, &srchRng, sizeof(srchRng));			_invUI16(&srchRng);
	UI16 inSelector		= 0; NBFile_read(file, &inSelector, sizeof(inSelector));	_invUI16(&inSelector);
	UI16 changeRng		= 0; NBFile_read(file, &changeRng, sizeof(changeRng));		_invUI16(&changeRng);
	if(tablesCount == 0){
		r = TRUE;
	} else {
		typedef struct STNBFontOTEncTable_ {
			UI8 tag[4];
			UI32 checksum, start, size;
		} STNBFontOTEncTable;
		STNBFontOTEncTable* tbls = NBMemory_allocTypes(STNBFontOTEncTable, tablesCount);
		if(NBFile_read(file, tbls, sizeof(STNBFontOTEncTable) * tablesCount) != (sizeof(STNBFontOTEncTable) * tablesCount)){
			//PRINTF_ERROR("NBFontsGlyphsStore, Could not read OT-encoding table.\n");
		} else {
			r = TRUE;
			UI32 i; for(i = 0; i < tablesCount; i++){
				STNBFontOTEncTable* tbl = &tbls[i];
				NBASSERT(tbl->tag[0] >= 32 && tbl->tag[0] < 127) //Printable rng
				NBASSERT(tbl->tag[1] >= 32 && tbl->tag[1] < 127) //Printable rng
				NBASSERT(tbl->tag[2] >= 32 && tbl->tag[2] < 127) //Printable rng
				NBASSERT(tbl->tag[3] >= 32 && tbl->tag[3] < 127) //Printable rng
				//PRINTF_INFO("TAG '%c%c%c%c'.\n", tbl->tag[0], tbl->tag[1], tbl->tag[2], tbl->tag[3]);
				if(tbl->tag[0] == 'h' && tbl->tag[1] == 'e' && tbl->tag[2] == 'a' && tbl->tag[3] == 'd'){
					_invUI32(&tbl->checksum);
					_invUI32(&tbl->start);
					_invUI32(&tbl->size);
					//Read font 'head' block
					if(!NBFile_seek(file, tbl->start, ENNBFileRelative_Start)){
						//PRINTF_ERROR("NBFontsGlyphsStore, Could not jump to OT header's position.\n");
						r = FALSE; NBASSERT(FALSE)
						break;
					} else {
						STNBFileRef subfile = NBFile_alloc(NULL);
						if(!NBFile_openAsFileRng(subfile, file, tbl->start, tbl->size)){
							//PRINTF_ERROR("NBFontsGlyphsStore, Could not open a sub-file to OT header's range.\n");
							r = FALSE; NBASSERT(FALSE)
							break;
						} else {
							NBFile_unlock(file);
							NBFile_lock(subfile);
							{
								typedef struct STFuenteOTTablaHead_ {
									UI16 bigVer, lilVer;
									UI16 bigRev, lilRev;
									UI32 checksumAdj, magicNum;
									UI16 flags, unitsPerEM;
									UI64 dateCreated;		//Here 2 bytes of padding are added
									UI64 dateModified;
									SI16 gblBoxXmin, gblBoxYmin, gblBoxXmax, gblBoxYmax;
									UI16 macStyle, minPixSz;
									SI16 dirHint, locaIdxFmt, glyphDataFmt;
								} STFuenteOTTablaHead;
								STFuenteOTTablaHead head;
								NBFile_read(subfile, &head.bigVer, sizeof(head.bigVer));				_invUI16(&head.bigVer);
								NBFile_read(subfile, &head.lilVer, sizeof(head.lilVer));				_invUI16(&head.lilVer);
								NBFile_read(subfile, &head.bigRev, sizeof(head.bigRev));				_invUI16(&head.bigRev);
								NBFile_read(subfile, &head.lilRev, sizeof(head.lilRev));				_invUI16(&head.lilRev);
								NBFile_read(subfile, &head.checksumAdj, sizeof(head.checksumAdj));		_invUI32(&head.checksumAdj);
								NBFile_read(subfile, &head.magicNum, sizeof(head.magicNum));			_invUI32(&head.magicNum);
								NBFile_read(subfile, &head.flags, sizeof(head.flags));					_invUI16(&head.flags);
								NBFile_read(subfile, &head.unitsPerEM, sizeof(head.unitsPerEM));		_invUI16(&head.unitsPerEM);
								NBFile_read(subfile, &head.dateCreated, sizeof(head.dateCreated));		//_invUI16(&head.dateCreated);
								NBFile_read(subfile, &head.dateModified, sizeof(head.dateModified));	//_invUI16(&head.dateModified);
								NBFile_read(subfile, &head.gblBoxXmin, sizeof(head.gblBoxXmin));		_invSI16(&head.gblBoxXmin);
								NBFile_read(subfile, &head.gblBoxYmin, sizeof(head.gblBoxYmin));		_invSI16(&head.gblBoxYmin);
								NBFile_read(subfile, &head.gblBoxXmax, sizeof(head.gblBoxXmax));		_invSI16(&head.gblBoxXmax);
								NBFile_read(subfile, &head.gblBoxYmax, sizeof(head.gblBoxYmax));		_invSI16(&head.gblBoxYmax);
								NBFile_read(subfile, &head.macStyle, sizeof(head.macStyle));			_invUI16(&head.macStyle);
								NBFile_read(subfile, &head.minPixSz, sizeof(head.minPixSz)); 			_invUI16(&head.minPixSz);
								NBFile_read(subfile, &head.dirHint, sizeof(head.dirHint));				_invSI16(&head.dirHint);
								NBFile_read(subfile, &head.locaIdxFmt, sizeof(head.locaIdxFmt));		_invSI16(&head.locaIdxFmt);
								NBFile_read(subfile, &head.glyphDataFmt, sizeof(head.glyphDataFmt));	_invSI16(&head.glyphDataFmt);
								NBASSERT(head.locaIdxFmt==0 || head.locaIdxFmt==1)
								//
								*dstStyleMask = 0;
								if((head.macStyle & 1) != 0) *dstStyleMask |= ENNBFontStyleBit_Bold;
								if((head.macStyle & 2) != 0) *dstStyleMask |= ENNBFontStyleBit_Italic;
								if((head.macStyle & 4) != 0) *dstStyleMask |= ENNBFontStyleBit_Undelined;
								if((head.macStyle & 8) != 0) *dstStyleMask |= ENNBFontStyleBit_Striked;
								if((head.macStyle & 16) != 0) *dstStyleMask |= ENNBFontStyleBit_Shadowed;
								if((head.macStyle & 32) != 0) *dstStyleMask |= ENNBFontStyleBit_Condensed;
								if((head.macStyle & 64) != 0) *dstStyleMask |= ENNBFontStyleBit_Extended;
							}
							NBFile_unlock(subfile);
							NBFile_close(subfile);
							NBFile_lock(file);
						}
						NBFile_release(&subfile);
					}
				} else if(tbl->tag[0] == 'n' && tbl->tag[1] == 'a' && tbl->tag[2] == 'm' && tbl->tag[3] == 'e'){
					_invUI32(&tbl->checksum);
					_invUI32(&tbl->start);
					_invUI32(&tbl->size);
					//Read font 'name' block
					if(!NBFile_seek(file, tbl->start, ENNBFileRelative_Start)){
						//PRINTF_ERROR("NBFontsGlyphsStore, Could not jump to OT header's position.\n");
						r = FALSE; NBASSERT(FALSE)
						break;
					} else {
						STNBFileRef subfile = NBFile_alloc(NULL);
						if(!NBFile_openAsFileRng(subfile, file, tbl->start, tbl->size)){
							//PRINTF_ERROR("NBFontsGlyphsStore, Could not open a sub-file to OT header's range.\n");
							r = FALSE; NBASSERT(FALSE)
							break;
						} else {
							NBFile_unlock(file);
							NBFile_lock(subfile);
							//Read format
							UI16 format = 0; NBFile_read(subfile, &format, sizeof(format)); _invUI16(&format);
							if(!(format == 0 || format == 1)){
								//PRINTF_ERROR("NBFontsGlyphsStore, Unexpected format value %d.\n", format);
								r = FALSE; NBASSERT(FALSE)
								break;
							} else {
								//Read records
								UI16 recsCount	= 0; NBFile_read(subfile, &recsCount, sizeof(recsCount));	_invUI16(&recsCount);
								UI16 strsPos	= 0; NBFile_read(subfile, &strsPos, sizeof(strsPos));		_invUI16(&strsPos);
								typedef struct STFontOTNameRecord_ {
									UI16 idPlatf, idCodif, idLang;
									UI16 idName, length, start;
								} STFontOTNameRecord;
								NBASSERT(recsCount > 0)
								STFontOTNameRecord* recs = NBMemory_allocTypes(STFontOTNameRecord, recsCount);
								if(NBFile_read(subfile, recs, sizeof(STFontOTNameRecord)* recsCount) != (sizeof(STFontOTNameRecord)* recsCount)){
									//PRINTF_ERROR("NBFontsGlyphsStore, Could not read 'name' records.\n");
									r = FALSE; NBASSERT(FALSE)
									break;
								} else {
									SI32 iFamUnicode	= -1, iSubfamUnicode = -1;
									SI32 iFamMac		= -1, iSubfamMac = -1;
									SI32 iFamMacEng		= -1, iSubfamMacEng = -1;
									SI32 iFamMacRoman	= -1, iSubfamMacRoman = -1;
									SI32 iFamWin		= -1, iSubfamWin = -1;
									BOOL iFamWinEsEng	= FALSE, iSubfamWinEsEng = FALSE;
									//Get all posibles names (decide the best after)
									{
										UI32 i; for(i = 0; i < recsCount; i++){
											STFontOTNameRecord* rec = &recs[i];
											_invUI16(&rec->idPlatf);
											_invUI16(&rec->idCodif);
											_invUI16(&rec->idLang);
											_invUI16(&rec->idName);
											_invUI16(&rec->length);
											_invUI16(&rec->start);
											//Search names
											if(rec->idName == 1){ //IDNAME_FAMILY
												switch(rec->idPlatf){
													case 0: //IDPLATAF_UNICODE:
													case 2: //IDPLATAF_ISO:
														iFamUnicode = i;
														break;
													case 1: //IDPLATAF_MAC:
														if(rec->idLang == 0 /*MAC_LANGID_ENGLISH*/) iFamMacEng = i;
														else if(rec->idCodif == 0 /*MAC_ID_ROMAN*/) iFamMacRoman = i;
														break;
													case 3: //IDPLATAF_WINDOWS:
														if(iFamWin == -1 || (rec->idLang & 0x3FF ) == 0x009 /*WIN_ENGLISH*/) {
															switch (rec->idCodif) {
																case 0: //WINDOWS_SYMBOL
																case 1: //WINDOWS_UNICODE_BMP:
																case 10: //WINDOWS_UNICODE_UCS4:
																	iFamWin			= i;
																	iFamWinEsEng	= ((rec->idLang & 0x3FF ) == 0x009);
																	break;
																default:
																	break;
															}
														}
														break;
													default:
														break;
												}
											} else if(rec->idName == 2){ //IDNAME_SUBFAMILY
												switch(rec->idPlatf){
													case 0: //IDPLATAF_UNICODE:
													case 2: //IDPLATAF_ISO:
														iSubfamUnicode = i;
														break;
													case 1: //IDPLATAF_MAC:
														if(rec->idLang == 0 /*MAC_LANGID_ENGLISH*/) iSubfamMacEng = i;
														else if(rec->idCodif == 0 /*MAC_ID_ROMAN*/) iSubfamMacRoman = i;
														break;
													case 3: //IDPLATAF_WINDOWS:
														if(iSubfamWin==-1 || (rec->idLang & 0x3FF ) == 0x009 /*WIN_ENGLISH*/) {
															switch (rec->idCodif) {
																case 0: //WINDOWS_SYMBOL
																case 1: //WINDOWS_UNICODE_BMP:
																case 10: //WINDOWS_UNICODE_UCS4:
																	iSubfamWin		= i;
																	iSubfamWinEsEng	= ((rec->idLang & 0x3FF ) == 0x009);
																	break;
																default:
																	break;
															}
														}
														break;
													default:
														break;
												}
											}
										}
									}
									//Load family and subfamily names
									iFamMac = iFamMacRoman; if(iFamMacEng>=0) iFamMac = iFamMacEng;
									iSubfamMac = iSubfamMacRoman; if(iSubfamMacEng>=0) iSubfamMac = iSubfamMacEng;
									//We will priorize Windows/English font names.
									//OpenTrueType establish that some fonts contains Mac and/or Unicode not-valid names.
									//Font-name
									{
										SI32 iRec = -1; BOOL isUtf16 = FALSE;
										if(iFamWin != -1 && (iFamMac == -1 || iFamWinEsEng)){
											//Only use non-english if english is not available.
											//ToDo: WINDOWS_UNICODE_UCS4 se debe procesar como 32-bits?
											iRec = iFamWin; isUtf16 = TRUE; //UTF16
										} else if(iFamMac != -1){
											iRec = iFamMac; isUtf16 = FALSE; //UTF8
										} else if(iFamUnicode != -1){
											iRec = iFamUnicode; isUtf16 = TRUE; //UTF16
										}
										if(iRec >= 0){
											NBASSERT(iRec < recsCount)
											STFontOTNameRecord* rec = &recs[iRec];
											char* strData = NBMemory_allocTypes(char, rec->length);
											if(!NBFile_seek(subfile, strsPos + rec->start, ENNBFileRelative_Start)){
												NBASSERT(FALSE)
											} else {
												NBString_empty(dstFamily);
												if(NBFile_read(subfile, strData, rec->length) != rec->length){
													NBASSERT(FALSE)
												} else {
													if(isUtf16){
														UI16* str16			= (UI16*)strData;
														const UI32 chars16	= (rec->length / 2);
														UI32 i; for(i = 0; i < chars16; i++) _invUI16(&str16[i]);
														NBString_concatUtf16Chars(dstFamily, str16, chars16);
													} else {
														NBString_concatBytes(dstFamily, strData, rec->length);
													}
												}
											}
											NBMemory_free(strData);
										}
									}
									//Subfont-name
									{
										SI32 iRec	= -1; BOOL isUtf16 = FALSE;
										if(iSubfamWin != -1 && (iSubfamMac == -1 || iSubfamWinEsEng)){
											//Only use non-english if english is not available.
											//ToDo: WINDOWS_UNICODE_UCS4 se debe procesar como 32-bits?
											iRec = iSubfamWin; isUtf16 = TRUE; //UTF16
										} else if(iSubfamMac!=-1){
											iRec = iSubfamMac; isUtf16 = FALSE; //UTF8
										} else if(iSubfamUnicode!=-1){
											iRec = iSubfamUnicode; isUtf16 = TRUE; //UTF16
										}
										if(iRec >= 0){
											NBASSERT(iRec < recsCount)
											STFontOTNameRecord* rec = &recs[iRec];
											char* strData = NBMemory_allocTypes(char, rec->length);
											if(!NBFile_seek(subfile, strsPos + rec->start, ENNBFileRelative_Start)){
												NBASSERT(FALSE)
											} else {
												NBString_empty(dstSubfamily);
												if(NBFile_read(subfile, strData, rec->length) != rec->length){
													NBASSERT(FALSE)
												} else {
													if(isUtf16){
														UI16* str16			= (UI16*)strData;
														const UI32 chars16	= (rec->length / 2);
														UI32 i; for(i = 0; i < chars16; i++) _invUI16(&str16[i]);
														NBString_concatUtf16Chars(dstSubfamily, str16, chars16);
													} else {
														NBString_concatBytes(dstSubfamily, strData, rec->length);
													}
												}
											}
											NBMemory_free(strData);
										}
									}
								}
								NBMemory_free(recs);
							}
							NBFile_unlock(subfile);
							NBFile_close(subfile);
							NBFile_lock(file);
						}
						NBFile_release(&subfile);
					}
				}
			}
		}
		NBMemory_free(tbls);
		tbls = NULL;
	}
	return r;
}

//Freetype reader

unsigned long NBFTReadr_read(FT_Stream stream, unsigned long offset, unsigned char*  buffer, unsigned long count){
	unsigned long r = 0;
	STNBFTReadr* rdr = (STNBFTReadr*) stream->descriptor.pointer; NBASSERT(rdr != NULL)
	NBASSERT(NBFile_isSet(rdr->file))
	if(NBFile_isSet(rdr->file)){
		if(rdr->lockAtRead){
			NBFile_lock(rdr->file);
		}
		if(!NBFile_seek(rdr->file, offset, ENNBFileRelative_Start)){
			NBASSERT(FALSE)
		} else {
            const int rd = NBFile_read(rdr->file, buffer, (UI32)count);
            if(rd >= 0){
                r = (unsigned long)rd;
            }
		}
		if(rdr->lockAtRead){
			NBFile_unlock(rdr->file);
		}
	}
	//PRINTF_INFO("NBFTReadr_read offset(%lu) count(%lu).\n", offset, count);
	return r;
}

void NBFTReadr_close(FT_Stream stream){
	STNBFTReadr* readr = (STNBFTReadr*) stream->descriptor.pointer; NBASSERT(readr != NULL)
	//Close file
	if(NBFile_isSet(readr->file)){
		NBFile_close(readr->file);
		//Free memory
		if(readr->freeAtClose){
			NBFile_release(&readr->file);
            NBFile_null(&readr->file);
		}
	}
	//Free memory
	if(readr->freeAtClose){
		NBMemory_free(readr);
		stream->descriptor.pointer = NULL;
	}
	//PRINTF_INFO("NBFTReadr_close.\n");
}

void* NBFTReadr_mAlloc(FT_Memory memory, long size){
	return NBMemory_alloc(size);
}

void NBFTReadr_mFree(FT_Memory memory, void* block){
	NBMemory_free(block);
}

void* NBFTReadr_mRealloc(FT_Memory memory, long cur_size, long new_size, void* block){
	NBMemory_free(block);
	return NBMemory_alloc(new_size);
}

//Query fonts

UI32 NBFontsGlyphsStore_getShapeId_(void* obj, const UI32 unicode){
	STNBFontsGlyphsStoreFont* font = (STNBFontsGlyphsStoreFont*)obj;
	return FT_Get_Char_Index(font->ftFont, unicode );
}

STNBFontShapeMetrics NBFontsGlyphsStore_getShapeMetrics_(void* obj, const UI32 shapeId){
	STNBFontShapeMetrics r;
	NBMemory_setZeroSt(r, STNBFontShapeMetrics);
	{
		STNBFontsGlyphsStoreFont* font = (STNBFontsGlyphsStoreFont*)obj;
		FT_Set_Char_Size( font->ftFont, font->ftFont->units_per_EM, font->ftFont->units_per_EM, 0, 0);
		FT_Error error	= FT_Load_Glyph( font->ftFont, shapeId, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_IGNORE_TRANSFORM | FT_LOAD_NO_BITMAP );
		
		if(error != 0){
			error = FT_Load_Glyph( font->ftFont, 0, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_IGNORE_TRANSFORM | FT_LOAD_NO_BITMAP);
		}
		if(error != 0){
			FT_Fixed advance;
			if(FT_Get_Advance(font->ftFont, shapeId, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_IGNORE_TRANSFORM, &advance) == 0){
				r.hAdvance	= advance;
			}
		} else {
			FT_GlyphSlot slot = font->ftFont->glyph;
			r.hBearingX = slot->metrics.horiBearingX;
			r.hBearingY = slot->metrics.horiBearingY;
			r.hAdvance	= slot->metrics.horiAdvance;
			r.vBearingX = slot->metrics.vertBearingX;
			r.vBearingY = slot->metrics.vertBearingY;
			r.vAdvance	= slot->metrics.vertAdvance;
			r.width 	= slot->metrics.width;
			r.height	= slot->metrics.height;
		}
	}
	return r;
}

void NBFontsGlyphsStore_rasterCallback(const int y, const int count, const FT_Span* const spans, void * const user){
	STNBFontLinesData* dstData = (STNBFontLinesData*) user;
	SI32 i, i2, spanLen, spanEnd; STNBFontLineVertex v; v.line = (SI16)y; //params->yMax - y; //(y - params->alto); //- (y - params->alto); // + (y - params->alto);
	for(i = 0; i < count; i++){
		const FT_Span* const span = &spans[i]; //PRINTF_INFO("Span yOrig(%d) y(%d) x(%d) len(%d) intensity(%d).\n", y, vertice.linea, span->x, span->len, span->coverage);
		v.intensity	= span->coverage;
		spanLen		= (SI32)span->len;
		if(v.intensity != 0 && spanLen != 0){
			spanEnd = span->x + spanLen;
			//Envolver caja
			if(dstData->spansCount == 0){
				dstData->box.xMin = span->x;
				dstData->box.xMax = spanEnd;
				dstData->box.yMin = y;
				dstData->box.yMax = y + 1;
			} else {
				if(dstData->box.xMin > span->x)	dstData->box.xMin = span->x;
				if(dstData->box.yMin > y)		dstData->box.yMin = y;
				if(dstData->box.xMax < spanEnd)	dstData->box.xMax = spanEnd;
				if(dstData->box.yMax < (y + 1))	dstData->box.yMax = (y + 1);
			}
			dstData->spansCount++;
			//Crear rasterizado
			if(spanLen < 4){
				//Add points
				for(i2 = 0; i2 < spanLen; i2++){
					v.pos = span->x + i2;
					NBArray_addValue(&dstData->vPoints, v); NBASSERT(v.intensity > 0)
				}
			} else {
				//Add line
				v.pos = span->x;
				NBArray_addValue(&dstData->vLines, v); NBASSERT(v.intensity > 0)
				v.pos = span->x + spanLen - 1;
				NBArray_addValue(&dstData->vLines, v); NBASSERT(v.intensity > 0)
			}
		}
	}
	
}

BOOL NBFontsGlyphsStore_getShapeLines_(void* obj, const UI32 shapeId, const UI32 fontSize, const STNBMatrix pMatrix, STNBFontLinesData* dstData){
	BOOL r = FALSE;
	STNBFontsGlyphsStoreFont* font = (STNBFontsGlyphsStoreFont*)obj;
	FT_GlyphSlot slot = font->ftFont->glyph;
	FT_Matrix matrix;
	{
		//only scale and rotation
		matrix.xx			= NBMATRIX_E00(pMatrix) * (float)0x10000L;	//(FT_Fixed)( cos( angle ) * 0x10000L );
		matrix.xy			= NBMATRIX_E01(pMatrix) * (float)0x10000L;	//(FT_Fixed)(-sin( angle ) * 0x10000L );
		matrix.yx			= NBMATRIX_E10(pMatrix) * (float)0x10000L;	//(FT_Fixed)( sin( angle ) * 0x10000L );
		matrix.yy			= NBMATRIX_E11(pMatrix) * (float)0x10000L;	//(FT_Fixed)( cos( angle ) * 0x10000L );
	}
	FT_Vector pen;
	{
		pen.x = 0 * 64; pen.y = 0 * 64;
	}
	FT_Set_Char_Size( font->ftFont, fontSize * 64, fontSize * 64, 64, 64);
	FT_Set_Transform( font->ftFont, &matrix, &pen );
	//
	FT_Error error = FT_Load_Glyph( font->ftFont, shapeId, FT_LOAD_NO_BITMAP /*| FT_LOAD_LINEAR_DESIGN*/ /*FT_LOAD_RENDER*/);
	if(error != 0){
		//Load default glyph
		error = FT_Load_Glyph( font->ftFont, 0, FT_LOAD_NO_BITMAP /*| FT_LOAD_LINEAR_DESIGN*/ /*FT_LOAD_RENDER*/); //Cargar caracter cero (glyphNoDef)
	}
	if(error != 0){
		PRINTF_ERROR("FT_Load_Glyph(%d shapeId) returned error '%d'.\n", shapeId, error);
		NBASSERT(FALSE)
	} else {
		NBASSERT(slot->bitmap.buffer == NULL || slot->bitmap.width == 0 || slot->bitmap.rows == 0)
		if(font->ftFont->glyph->format == FT_GLYPH_FORMAT_OUTLINE){
			//FT_Bitmap*  bitmap = &slot->bitmap;
			//PRINTF_INFO("Tam(%.2f) char(%d, '%c'): lbear(%d) lbearDelta(%ld) rbearDelta(%ld) w(%d) avLinH(%.2f) avLinV(%.2f) ... avHlinear(%.2f) ... w(%.2f) h(%.2f) ... av(%.2f) avx(%.2f) avy(%.2f) ... lsb_del(%ld) rsb_del(%ld)\n", tamanoFuente, caracter, (char)caracter, slot->bitmap_left, slot->lsb_delta, slot->rsb_delta, bitmap->width, (float)slot->advance.x / 64.0f, (float)slot->linearHoriAdvance / (float)face->units_per_EM * tamanoFuente /*65536.0f*/, (float)slot->linearVertAdvance / (float)face->units_per_EM * tamanoFuente /*65536.0f*/, (float)slot->metrics.width / 64.0f, (float)slot->metrics.height / 64.0f, (float)slot->metrics.horiAdvance / 64.0f, (float)slot->metrics.horiBearingX / 64.0f, (float)slot->metrics.horiBearingY / 64.0f, slot->lsb_delta, slot->rsb_delta);
			//PRINTF_INFO("                    w(%.2f) h(%.2f) ... ah(%.2f) ahx(%.2f) ahy(%.2f) ... av(%.2f) avx(%.2f) avy(%.2f) ... EM(%.2f).\n", (float)slot->metrics.width / (float)face->units_per_EM * tamano, (float)slot->metrics.height / (float)face->units_per_EM * tamano, (float)slot->metrics.horiAdvance / (float)face->units_per_EM * tamano, (float)slot->metrics.horiBearingX / (float)face->units_per_EM * tamano, (float)slot->metrics.horiBearingY / (float)face->units_per_EM * tamano, (float)slot->metrics.vertAdvance / (float)face->units_per_EM * tamano, (float)slot->metrics.vertBearingX / (float)face->units_per_EM * tamano, (float)slot->metrics.vertBearingY / (float)face->units_per_EM * tamano, (float)face->units_per_EM);
			dstData->metrics.hBearingX	= (float)slot->metrics.horiBearingX / 64.0f;
			dstData->metrics.hBearingY	= (float)slot->metrics.horiBearingY / 64.0f;
			dstData->metrics.hAdvance	= (float)slot->metrics.horiAdvance / 64.0f;
			dstData->metrics.vBearingX	= (float)slot->metrics.vertBearingX / 64.0f;
			dstData->metrics.vBearingY	= (float)slot->metrics.vertBearingY / 64.0f;
			dstData->metrics.vAdvance	= (float)slot->metrics.vertAdvance / 64.0f;
			dstData->metrics.width 		= (float)slot->metrics.width / 64.0f;
			dstData->metrics.height		= (float)slot->metrics.height / 64.0f;
			dstData->spansCount			= 0;
			dstData->box.xMin			= dstData->box.yMin = dstData->box.xMax = dstData->box.yMax = 0;
			//
			FT_Raster_Params params;
			NBMemory_set(&params, 0, sizeof(params));
			params.flags		= FT_RASTER_FLAG_AA | FT_RASTER_FLAG_DIRECT;
			params.gray_spans	= NBFontsGlyphsStore_rasterCallback;
			params.user			= dstData;
			FT_Outline_Render(*font->ftLib, &font->ftFont->glyph->outline, &params);
			//
			NBASSERT((float)((SI32)dstData->box.xMin) == dstData->box.xMin && (float)((SI32)dstData->box.xMax) == dstData->box.xMax && (float)((SI32)dstData->box.yMin) == dstData->box.yMin && (float)((SI32)dstData->box.yMax) == dstData->box.yMax)
			NBASSERT((slot->metrics.horiBearingX % 64) == 0) //If fails, not integer
			//NBASSERT((slot->advance.x % 64) == 0) //If fails, not integer
			r = TRUE;
		}
	}
	return r;
}

