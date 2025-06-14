

#include "nb/NBFrameworkPch.h"
#include "nb/files/NBMp4.h"
#include "nb/crypto/NBCrc32.h"
#ifdef NB_CONFIG_INCLUDE_ASSERTS
#	include "nb/2d/NBAvc.h"
#endif
//FULL-BOXES FUNCTIONS

#define NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(BOX_NAME) \
	void NBMp4 ## BOX_NAME ## _init_(void* obj){\
		NBMp4 ## BOX_NAME ## _init((STNBMp4 ## BOX_NAME*)obj);\
	}\
	void NBMp4 ## BOX_NAME ## _release_(void* obj){\
		NBMp4 ## BOX_NAME ## _release((STNBMp4 ## BOX_NAME*)obj);\
	}\
	BOOL NBMp4 ## BOX_NAME ## _loadBits_(void* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){\
		return NBMp4 ## BOX_NAME ## _loadFromBuff((STNBMp4 ## BOX_NAME*) obj, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd);\
	}\
	void NBMp4 ## BOX_NAME ## _concat_(const void* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){\
		NBMp4 ## BOX_NAME ## _concat((const STNBMp4 ## BOX_NAME*)obj, depthLvl, optSrcFile, dst);\
	}\
	BOOL NBMp4 ## BOX_NAME ## _writeBits_(void* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){\
		return NBMp4 ## BOX_NAME ## _writeBits((STNBMp4 ## BOX_NAME*)obj, depthLvl, optSrcFile, dst);\
	}\
	BOOL NBMp4 ## BOX_NAME ## _allocRef(STNBMp4BoxRef* dst){\
		const STNBMp4BoxStDef def = NBMP4_BOX_PARSEABLE_ST_DEF( BOX_NAME );\
		if(dst != NULL){\
			STNBMp4 ## BOX_NAME * box = NBMemory_allocType(STNBMp4 ## BOX_NAME);\
			dst->stDef	= def;\
			dst->box	= (STNBMp4Box*)box;\
			NBMp4 ## BOX_NAME ##_init(box);\
		}\
		return TRUE;\
	}

//CONTAINERS FUNCTIONS

#define NB_MP4_CONTAINER_FUNCS_BODY(C0, C1, C2, C3, BOX_NAME) \
	void NBMp4 ## BOX_NAME ## _init(STNBMp4 ## BOX_NAME* obj){\
		NBMemory_setZeroSt(*obj, STNBMp4 ## BOX_NAME);\
		NBMp4Box_init(&obj->prntBox);\
		obj->prntBox.hdr.type.str[0] = C0; \
		obj->prntBox.hdr.type.str[1] = C1; \
		obj->prntBox.hdr.type.str[2] = C2; \
		obj->prntBox.hdr.type.str[3] = C3; \
	}\
	void NBMp4 ## BOX_NAME ## _release(STNBMp4 ## BOX_NAME* obj){\
		NBMp4Box_releaseChildren(obj, sizeof(*obj), NBMp4 ## BOX_NAME ## Chldrn_, (sizeof(NBMp4 ## BOX_NAME ## Chldrn_) / sizeof(NBMp4 ## BOX_NAME ## Chldrn_[0])));\
		NBMp4Box_release(&obj->prntBox);\
	}\
	BOOL NBMp4 ## BOX_NAME ## _loadFromBuff(STNBMp4 ## BOX_NAME* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){\
		NBMp4Box_cloneHdr(&obj->prntBox, prnt, boxFileStart, hdrFileSz, (UI32)(buffAfterEnd - *buff), hdr);\
		return (\
				NBMp4Box_loadChildren(&obj->prntBox, sizeof(*obj), NBMp4 ## BOX_NAME ## Chldrn_, (sizeof(NBMp4 ## BOX_NAME ## Chldrn_) / sizeof(NBMp4 ## BOX_NAME ## Chldrn_[0])), depthLvl, (boxFileStart + hdrFileSz), buff, buffAfterEnd)\
				&& *buff == buffAfterEnd\
				);\
	}\
	void NBMp4 ## BOX_NAME ## _concat(const STNBMp4 ## BOX_NAME* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){\
		if(dst != NULL){\
			NBMp4Box_concatHdr(&obj->prntBox, depthLvl, optSrcFile, dst);\
			NBMp4Box_concatChildren(&obj->prntBox, depthLvl + 1, optSrcFile, dst, sizeof(*obj), NBMp4 ## BOX_NAME ## Chldrn_, (sizeof(NBMp4 ## BOX_NAME ## Chldrn_) / sizeof(NBMp4 ## BOX_NAME ## Chldrn_[0])));\
		}\
	} \
	BOOL NBMp4 ## BOX_NAME ## _writeBits(STNBMp4 ## BOX_NAME* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){\
		if(!NBMp4Box_writeHdrBits(&obj->prntBox, depthLvl, optSrcFile, dst)){\
			return FALSE;\
		}\
		if(!NBMp4Box_writeChildren(&obj->prntBox, sizeof(*obj), NBMp4 ## BOX_NAME ## Chldrn_, (sizeof(NBMp4 ## BOX_NAME ## Chldrn_) / sizeof(NBMp4 ## BOX_NAME ## Chldrn_[0])), depthLvl, optSrcFile, dst)){\
			return FALSE;\
		}\
		if(!NBMp4Box_writeHdrFinalSize(&obj->prntBox, depthLvl, optSrcFile, dst)){\
			return FALSE;\
		}\
		return TRUE;\
	}

#define NB_MP4_CONTAINER_FUNCS_AND_ITFS_BODY(C0, C1, C2, C3, BOX_NAME) \
	NB_MP4_CONTAINER_FUNCS_BODY(C0, C1, C2, C3, BOX_NAME) \
	NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(BOX_NAME)

//General use

BOOL NBMp4_readUI32File(UI32* dst, STNBFileRef f){
	UI32 v = 0;
	if(NBFile_read(f, &v, sizeof(v)) != sizeof(v)){
		return FALSE;
	}
	if(dst != NULL){
		*dst = v >> 24 | v << 24 | (v >> 8 & 0xFF00) | (v << 8 & 0xFF0000);
	}
	return TRUE;
}

BOOL NBMp4_readBytesFile(void* dst, const UI32 bytes, STNBFileRef f){
	if(NBFile_read(f, dst, bytes) != bytes){
		return FALSE;
	}
	return TRUE;
}

UI32 NBMp4_crcFile(const UI32 iStart, const UI32 count, STNBFileRef f){
	UI32 r = 0;
	if(NBFile_seek(f, iStart, ENNBFileRelative_Start)){
		BYTE buff[4096]; UI32 toRead, remain = count;
		STNBCrc32 crc;
		NBCrc32_init(&crc);
		while(remain > 0){
			toRead = (remain > sizeof(buff) ? sizeof(buff) : remain);
			if(NBFile_read(f, buff, toRead) != toRead){
				break;
			} else {
				NBCrc32_feed(&crc, buff, toRead);
				remain -= toRead;
			}
		}
		if(remain == 0){
			NBCrc32_finish(&crc);
			r = crc.hash; 
		}
		NBCrc32_release(&crc);
	}
	return r;
}

//buff-read

BOOL NBMp4_readSI16(SI16* dst, const BYTE** buff, const BYTE* buffAfterEnd){
	if((*buff + 2) > buffAfterEnd){
		return FALSE;
	}
	if(dst != NULL){
		//as unsigned for zero-filled bitwise operations.
		UI16 v = *((const UI16*)*buff);
		v = v >> 8 | v << 8;
		*dst = *((const SI16*)&v); 
	}
	*buff += 2;
	return TRUE;
}

BOOL NBMp4_readUI16(UI16* dst, const BYTE** buff, const BYTE* buffAfterEnd){
	if((*buff + 2) > buffAfterEnd){
		return FALSE;
	}
	if(dst != NULL){
		const UI16 v = *((const UI16*)*buff);
		*dst = v >> 8 | v << 8;
	}
	*buff += 2;
	return TRUE;
}

BOOL NBMp4_readSI32(SI32* dst, const BYTE** buff, const BYTE* buffAfterEnd){
	if((*buff + 4) > buffAfterEnd){
		return FALSE;
	}
	if(dst != NULL){
		//as unsigned for zero-filled bitwise operations.
		UI32 v = *((const UI32*)*buff);
		// 0123
		// |||0 = v >> 24
		// 3||| = v << 24
		// ||1_ = (v >> 8 & 0xFF00)
		// |2__ = (v << 8 & 0xFF0000)
		v = (v >> 24 /*& 0xFF*/) | (v << 24 /*& 0xFF000000*/) | (v >> 8 & 0xFF00) | (v << 8 & 0xFF0000);
		*dst = *((const SI32*)&v);
	}
	*buff += 4;
	return TRUE;
}

BOOL NBMp4_readUI32(UI32* dst, const BYTE** buff, const BYTE* buffAfterEnd){ 
	if((*buff + 4) > buffAfterEnd){
		return FALSE;
	}
	if(dst != NULL){
		const UI32 v = *((const UI32*)*buff);
		// 0123
		// |||0 = v >> 24
		// 3||| = v << 24
		// ||1_ = (v >> 8 & 0xFF00)
		// |2__ = (v << 8 & 0xFF0000)
		*dst = v >> 24 | v << 24 | (v >> 8 & 0xFF00) | (v << 8 & 0xFF0000);
	}
	*buff += 4;
	return TRUE;
}

BOOL NBMp4_readSI64(SI64* dst, const BYTE** buff, const BYTE* buffAfterEnd){
	if((*buff + 8) > buffAfterEnd){
		return FALSE;
	}
	if(dst != NULL){
		//as unsigned for zero-filled bitwise operations.
		UI64 v = *((const UI64*)*buff);
		// 01234567
		// X||||||0 = v >> 56
		// 7||||||X = v << 56
		// |X||||1_ = (v >> 40 & 0xFF00)
		// |6____X_ = (v << 40 & 0xFF000000000000)
		// ||X||2__ = (v >> 24 & 0xFF0000)
		// ||5__X__ = (v << 24 & 0xFF0000000000)
		// |||X3___ = (v >> 8 & 0xFF000000)
		// |||4X___ = (v << 8 & 0xFF00000000)
		v = v >> 56 | v << 56 | (v >> 40 & 0xFF00) | (v << 40 & 0xFF000000000000) | (v >> 24 & 0xFF0000) | (v << 24 & 0xFF0000000000) | (v >> 8 & 0xFF000000) | (v << 8 & 0xFF00000000);
		*dst = *((const SI64*)&v);
	}
	*buff += 8;
	return TRUE;
}

BOOL NBMp4_readUI64(UI64* dst, const BYTE** buff, const BYTE* buffAfterEnd){ 
	if((*buff + 8) > buffAfterEnd){
		return FALSE;
	}
	if(dst != NULL){
		const UI64 v = *((const UI64*)*buff);
		// 01234567
		// X||||||0 = v >> 56
		// 7||||||X = v << 56
		// |X||||1_ = (v >> 40 & 0xFF00)
		// |6____X_ = (v << 40 & 0xFF000000000000)
		// ||X||2__ = (v >> 24 & 0xFF0000)
		// ||5__X__ = (v << 24 & 0xFF0000000000)
		// |||X3___ = (v >> 8 & 0xFF000000)
		// |||4X___ = (v << 8 & 0xFF00000000)
		*dst = v >> 56 | v << 56 | (v >> 40 & 0xFF00) | (v << 40 & 0xFF000000000000) | (v >> 24 & 0xFF0000) | (v << 24 & 0xFF0000000000) | (v >> 8 & 0xFF000000) | (v << 8 & 0xFF00000000);
	}
	*buff += 8;
	return TRUE;
}

BOOL NBMp4_readSI8s(SI8* dst, const UI32 amm, const BYTE** buff, const BYTE* buffAfterEnd){
	return NBMp4_readBytes(dst, amm, buff, buffAfterEnd);;
}

BOOL NBMp4_readUI8s(UI8* dst, const UI32 amm, const BYTE** buff, const BYTE* buffAfterEnd){
	return NBMp4_readBytes(dst, amm, buff, buffAfterEnd);;
}

BOOL NBMp4_readSI16s(SI16* dst, const UI32 amm, const BYTE** buff, const BYTE* buffAfterEnd){
	const SI16* dstAfterEnd = dst + amm;
	while(dst < dstAfterEnd){
		if(!NBMp4_readSI16(dst, buff, buffAfterEnd)){
			return FALSE;
		}
		dst++;
	}
	return TRUE;
}

BOOL NBMp4_readUI16s(UI16* dst, const UI32 amm, const BYTE** buff, const BYTE* buffAfterEnd){
	const UI16* dstAfterEnd = dst + amm;
	while(dst < dstAfterEnd){
		if(!NBMp4_readUI16(dst, buff, buffAfterEnd)){
			return FALSE;
		}
		dst++;
	}
	return TRUE;
}

BOOL NBMp4_readSI32s(SI32* dst, const UI32 amm, const BYTE** buff, const BYTE* buffAfterEnd){
	const SI32* dstAfterEnd = dst + amm;
	while(dst < dstAfterEnd){
		if(!NBMp4_readSI32(dst, buff, buffAfterEnd)){
			return FALSE;
		}
		dst++;
	}
	return TRUE;
}

BOOL NBMp4_readUI32s(UI32* dst, const UI32 amm, const BYTE** buff, const BYTE* buffAfterEnd){
	const UI32* dstAfterEnd = dst + amm;
	while(dst < dstAfterEnd){
		if(!NBMp4_readUI32(dst, buff, buffAfterEnd)){
			return FALSE;
		}
		dst++;
	}
	return TRUE;
}

BOOL NBMp4_readBytes(void* dst, const UI32 bytes, const BYTE** buff, const BYTE* buffAfterEnd){
	if((*buff + bytes) > buffAfterEnd){
		return FALSE;
	}
	if(dst != NULL){
		switch(bytes) {
			case 1:
				*((BYTE*)dst) = *((BYTE*)*buff);
				break;
			case 2:
				*((UI16*)dst) = *((UI16*)*buff);
				break;
			case 4:
				*((UI32*)dst) = *((UI32*)*buff);
				break;
			case 8:
				*((UI64*)dst) = *((UI64*)*buff);
				break;
			case 16:
				((UI64*)dst)[0] = ((UI64*)*buff)[0];
				((UI64*)dst)[1] = ((UI64*)*buff)[1];
				break;
			case 24:
				((UI64*)dst)[0] = ((UI64*)*buff)[0];
				((UI64*)dst)[1] = ((UI64*)*buff)[1];
				((UI64*)dst)[2] = ((UI64*)*buff)[2];
				break;
			case 32:
				((UI64*)dst)[0] = ((UI64*)*buff)[0];
				((UI64*)dst)[1] = ((UI64*)*buff)[1];
				((UI64*)dst)[2] = ((UI64*)*buff)[2];
				((UI64*)dst)[3] = ((UI64*)*buff)[3];
				break;
			default:
				NBMemory_copy(dst, *buff, bytes);
				break;
		}
	}
	*buff += bytes;
	return TRUE;
}

BOOL NBMp4_readString(char** dst, const BYTE** buff, const BYTE* buffAfterEnd){
	const BYTE* strStart = *buff;
	while(*buff < buffAfterEnd && **buff != '\0'){
		//next
		(*buff)++;
	}
	if(*buff >= buffAfterEnd){
		return FALSE;
	} else {
		NBASSERT(**buff == '\0')
		const UI32 len = (UI32)(*buff - strStart);
		if(dst != NULL){
			if(*dst != NULL){
				NBMemory_free(*dst);
			}
			*dst = NBMemory_alloc(len + 1);
			NBMemory_copy(*dst, strStart, len + 1);
		}
		//move from '\0'
		(*buff)++;
	}
	return TRUE;
}

//buff-write

BOOL NBMp4_writeSI16(const SI16* src, STNBString* dst){
	//as unsigned for zero-filled bitwise operations.
	UI16 v = *((const UI16*)src);
	v = v >> 8 | v << 8;
	NBString_concatBytes(dst, (const char*)&v, sizeof(v));
	return TRUE;
}

BOOL NBMp4_writeUI16(const UI16* src, STNBString* dst){
	const UI16 v = *src >> 8 | *src << 8;
	NBString_concatBytes(dst, (const char*)&v, sizeof(v));
	return TRUE;
}

BOOL NBMp4_writeSI32(const SI32* src, STNBString* dst){
	//as unsigned for zero-filled bitwise operations.
	UI32 v = *((const UI32*)src);
	v = (v >> 24) | (v << 24) | (v >> 8 & 0xFF00) | (v << 8 & 0xFF0000);
	NBString_concatBytes(dst, (const char*)&v, sizeof(v));
	return TRUE;
}

BOOL NBMp4_writeUI32(const UI32* src, STNBString* dst){
	const UI32 v = *src >> 24 | *src << 24 | (*src >> 8 & 0xFF00) | (*src << 8 & 0xFF0000);
	NBString_concatBytes(dst, (const char*)&v, sizeof(v));
	return TRUE;
}

BOOL NBMp4_writeUI32At(const UI32* src, void* dst){
	*((UI32*)dst) = *src >> 24 | *src << 24 | (*src >> 8 & 0xFF00) | (*src << 8 & 0xFF0000);
	return TRUE;
}

BOOL NBMp4_writeSI64(const SI64* src, STNBString* dst){
	//as unsigned for zero-filled bitwise operations.
	UI64 v = *((const UI64*)src);
	v = v >> 56 | v << 56 | (v >> 40 & 0xFF00) | (v << 40 & 0xFF000000000000) | (v >> 24 & 0xFF0000) | (v << 24 & 0xFF0000000000) | (v >> 8 & 0xFF000000) | (v << 8 & 0xFF00000000);
	NBString_concatBytes(dst, (const char*)&v, sizeof(v));
	return TRUE;
}

BOOL NBMp4_writeUI64(const UI64* src, STNBString* dst){ 
	const UI64 v = *src >> 56 | *src << 56 | (*src >> 40 & 0xFF00) | (*src << 40 & 0xFF000000000000) | (*src >> 24 & 0xFF0000) | (*src << 24 & 0xFF0000000000) | (*src >> 8 & 0xFF000000) | (*src << 8 & 0xFF00000000);
	NBString_concatBytes(dst, (const char*)&v, sizeof(v));
	return TRUE;
}

BOOL NBMp4_writeSI8s(const SI8* src, const UI32 amm, STNBString* dst){
	NBString_concatBytes(dst, (const char*)src, amm);
	return TRUE;
}

BOOL NBMp4_writeUI8s(const UI8* src, const UI32 amm, STNBString* dst){
	NBString_concatBytes(dst, (const char*)src, amm);
	return TRUE;
}

BOOL NBMp4_writeSI16s(const SI16* src, const UI32 amm, STNBString* dst){
	const SI16* srcAfterEnd = src + amm;
	while(src < srcAfterEnd){
		if(!NBMp4_writeSI16(src, dst)){
			return FALSE;
		}
		src++;
	}
	return TRUE;
}

BOOL NBMp4_writeUI16s(const UI16* src, const UI32 amm, STNBString* dst){
	const UI16* srcAfterEnd = src + amm;
	while(src < srcAfterEnd){
		if(!NBMp4_writeUI16(src, dst)){
			return FALSE;
		}
		src++;
	}
	return TRUE;
}

BOOL NBMp4_writeSI32s(const SI32* src, const UI32 amm, STNBString* dst){
	const SI32* srcAfterEnd = src + amm;
	while(src < srcAfterEnd){
		if(!NBMp4_writeSI32(src, dst)){
			return FALSE;
		}
		src++;
	}
	return TRUE;
}

BOOL NBMp4_writeUI32s(const UI32* src, const UI32 amm, STNBString* dst){
	const UI32* srcAfterEnd = src + amm;
	while(src < srcAfterEnd){
		if(!NBMp4_writeUI32(src, dst)){
			return FALSE;
		}
		src++;
	}
	return TRUE;
}

BOOL NBMp4_writeBytes(const void* src, const UI32 bytes, STNBString* dst){
	NBString_concatBytes(dst, (const char*)src, bytes);
	return TRUE;
}

BOOL NBMp4_writeString(const char* src, STNBString* dst){
	const UI32 len = NBString_strLenBytes(src);
	if(len == 0){
		char c0 = '\0';
		NBString_concatBytes(dst, &c0, 1);
	} else {
		NBString_concatBytes(dst, (const char*)src, len + 1); //+1 to include the '\0'
	}
	return TRUE;
}

//NBMp4Data

void NBMp4Data_init(STNBMp4Data* obj){
	NBMemory_setZeroSt(*obj, STNBMp4Data);
}

void NBMp4Data_release(STNBMp4Data* obj){
	if(obj->pay != NULL){
		NBMemory_free(obj->pay);
		obj->pay = NULL;
	}
	obj->use = obj->allocSz = 0;
}

//NBMp4Box

void NBMp4Box_init_(void* obj);
void NBMp4Box_release_(void* obj);
BOOL NBMp4Box_loadBits_(void* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd);
void NBMp4Box_concat_(const void* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst);
BOOL NBMp4Box_writeBits_(void* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst);

STNBMp4BoxStDef NBMp4BoxStDef_ = NBMP4_BOX_PARSEABLE_ST_DEF(Box);

void NBMp4BoxHdr_init(STNBMp4BoxHdr* obj){
	NBMemory_setZeroSt(*obj, STNBMp4BoxHdr);
}

void NBMp4BoxHdr_release(STNBMp4BoxHdr* obj){
	//nothing
}

void NBMp4_concatChar(STNBString* dst, const char v){
	if(v >= 32 && v <= 127){
		NBString_concatByte(dst, v);
	} else {
		NBString_concatByte(dst, '#');
		NBString_concatUI32(dst, v);
	}
}

void NBMp4_concatFixed8x8(STNBString* dst, const SI16 v){
	NBString_concatSI32(dst, v >> 8);
	if(v & 0xFF){
		NBString_concat(dst, ".");
		NBString_concatUI32(dst, (UI32)(v & 0xFF) * 100 / 0xFF);
	}
}

void NBMp4_concatFixed16x16(STNBString* dst, const SI32 v){
	NBString_concatSI32(dst, v >> 16);
	if(v & 0xFFFF){
		NBString_concat(dst, ".");
		NBString_concatUI32(dst, (UI32)(v & 0xFFFF) * 100 / 0xFFFF);
	}
}

void NBMp4_concatFixed2x30(STNBString* dst, const SI32 v){
	NBString_concatSI32(dst, v >> 30);
	if(v & 0x3FFFFFFF){
		NBString_concat(dst, ".");
		NBString_concatUI32(dst, (UI32)(((UI64)(v & 0x3FFFFFFF) * 100) / 0x3FFFFFFF));
	}
}

void NBMp4_concatMatrix9(STNBString* dst, const SI32* v){
	//All the values in a matrix are stored as 16.16 fixed‐point values, except for u, v and w, which are stored as 2.30 fixed‐point values.
	//The values in the matrix are stored in the order {a,b,u, c,d,v, x,y,w}.
	NBString_concat(dst, "{");
	//
	NBMp4_concatFixed16x16(dst, v[0]);
	NBString_concat(dst, ","); NBMp4_concatFixed16x16(dst, v[1]);
	NBString_concat(dst, ","); NBMp4_concatFixed2x30(dst, v[2]);
	//
	NBString_concat(dst, "}-{");
	//
	NBMp4_concatFixed16x16(dst, v[3]);
	NBString_concat(dst, ","); NBMp4_concatFixed16x16(dst, v[4]);
	NBString_concat(dst, ","); NBMp4_concatFixed2x30(dst, v[5]);
	//
	NBString_concat(dst, "}-{");
	//
	NBMp4_concatFixed16x16(dst, v[6]);
	NBString_concat(dst, ","); NBMp4_concatFixed16x16(dst, v[7]);
	NBString_concat(dst, ","); NBMp4_concatFixed2x30(dst, v[8]);
	//
	NBString_concat(dst, "}");
}

void NBMp4_concatTimestamp(STNBString* dst, const UI64 t){
	if(t >= NBMP4_SECS_FROM_1901_TO_UTC1970){
		const UI64 t1970 = t - NBMP4_SECS_FROM_1901_TO_UTC1970;
		const STNBDatetime tt = NBDatetime_getUTCFromTimestamp(t1970);
		NBString_concatSqlDatetime(dst, tt);
	} else {
		NBString_concatUI64(dst, t);
	}
}

void NBMp4BoxHdr_concat(const STNBMp4BoxHdr* obj, const UI32 depthLvl, STNBString* dst){
	if(dst != NULL){
		NBString_concatRepeatedByte(dst, '\t', depthLvl);
		NBString_concat(dst, "Box '");
		if(obj->type.str[0] == 'u' && obj->type.str[1] == 'u' && obj->type.str[2] == 'i' && obj->type.str[3] == 'd'){
			NBString_concat(dst, "uuid+");
			NBMp4_concatChar(dst, obj->userType[0]);
			NBMp4_concatChar(dst, obj->userType[1]);
			NBMp4_concatChar(dst, obj->userType[2]);
			NBMp4_concatChar(dst, obj->userType[3]);
			NBMp4_concatChar(dst, obj->userType[4]);
			NBMp4_concatChar(dst, obj->userType[5]);
			NBMp4_concatChar(dst, obj->userType[6]);
			NBMp4_concatChar(dst, obj->userType[7]);
			NBMp4_concatChar(dst, obj->userType[8]);
			NBMp4_concatChar(dst, obj->userType[9]);
			NBMp4_concatChar(dst, obj->userType[10]);
			NBMp4_concatChar(dst, obj->userType[11]);
			NBMp4_concatChar(dst, obj->userType[12]);
			NBMp4_concatChar(dst, obj->userType[13]);
			NBMp4_concatChar(dst, obj->userType[14]);
			NBMp4_concatChar(dst, obj->userType[15]);
			NBString_concat(dst, "'");
		} else {
			NBMp4_concatChar(dst, obj->type.str[0]);
			NBMp4_concatChar(dst, obj->type.str[1]);
			NBMp4_concatChar(dst, obj->type.str[2]);
			NBMp4_concatChar(dst, obj->type.str[3]);
			NBString_concat(dst, "'");
		}
		if(obj->size == 1){
			NBString_concat(dst, " large-size("); NBString_concatUI64(dst, obj->largeSize); NBString_concat(dst, ")");
		} else {
			NBString_concat(dst, " size("); NBString_concatUI32(dst, obj->size); NBString_concat(dst, ")");
		}
		NBString_concatByte(dst, '\n');
	}
}

BOOL NBMp4BoxHdr_loadBitsFromFile(STNBMp4BoxHdr* obj, STNBFileRef f){
	if(!NBMp4_readUI32File(&obj->size, f)){
		return FALSE;
	}
	if(!NBMp4_readBytesFile(&obj->type, sizeof(obj->type), f)){
		return FALSE;
	}
	/*if(obj->size == 1){
		if(!NBMp4_loadUI64FromFile(&obj->largeSize, f)){
			return FALSE;
		}
	}
	if(NBMp4_isChar4(obj->type.str, "uuid")){
		if(!NBMp4_readBytesFile(&obj->userType, sizeof(obj->userType), f)){
			return FALSE;
		}
	}*/
	return TRUE;
}

BOOL NBMp4BoxHdr_loadBits(STNBMp4BoxHdr* obj, const BYTE** buff, const BYTE* buffAfterEnd){
	if(!NBMp4_readUI32(&obj->size, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readBytes(&obj->type, sizeof(obj->type), buff, buffAfterEnd)){
		return FALSE;
	}
	/*if(obj->size == 1){
		if(!NBMp4_loadUI64FromBuff(&obj->largeSize, buff, buffAfterEnd)){
			return FALSE;
		}
	}
	if(NBMp4_isChar4(obj->type.str, "uuid")){
		if(!NBMp4_readBytes(&obj->userType, sizeof(obj->userType), buff, buffAfterEnd)){
			return FALSE;
		}
	}*/
	return TRUE;
}

UI32 NBMp4BoxHdr_getHdrBytesLen(const STNBMp4BoxHdr* obj){
	return 8 //size32 + type
	+ (obj->size == 1 ? 8 : 0) //size64
	+ (obj->type.str[0] == 'u' && obj->type.str[1] == 'u' && obj->type.str[2] == 'i' && obj->type.str[3] == 'd' ? sizeof(obj->userType) : 0) //userType
	;
}

BOOL NBMp4BoxHdr_writeBits(STNBMp4BoxHdr* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	const UI32 infiniteSize = 0;
	obj->lastWritePos = dst->length;
	//Write infinite size, untill the final-size is known
	if(!NBMp4_writeUI32(&infiniteSize, dst)){
		return FALSE;
	}
	if(!NBMp4_writeBytes(&obj->type, sizeof(obj->type), dst)){
		return FALSE;
	}
	/*if(obj->size == 1){
		if(!NBMp4_writeUI64(&obj->largeSize, dst)){
			return FALSE;
		}
	}
	if(NBMp4_isChar4(obj->type.str, "uuid")){
		if(!NBMp4_writeBytes(&obj->userType, sizeof(obj->userType), dst)){
			return FALSE;
		}
	}*/
	return TRUE;
}

BOOL NBMp4BoxHdr_writeHdrFinalSize(const STNBMp4BoxHdr* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if((obj->lastWritePos + 4) >= dst->length){
		return FALSE;
	}
	{
		const UI32 finalSize = (UI32)(dst->length - obj->lastWritePos);
		const UI32 v = finalSize >> 24 | finalSize << 24 | (finalSize >> 8 & 0xFF00) | (finalSize << 8 & 0xFF0000);
		*((UI32*)&dst->str[obj->lastWritePos]) = v;
	}
	return TRUE;
}

BOOL NBMp4BoxHdr_writeHdrFinalSizeExplicit(const STNBMp4BoxHdr* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst, const UI32 dstFinalSize){
	if((obj->lastWritePos + 4) >= dst->length || dstFinalSize < dst->length){
		return FALSE;
	}
	{
		const UI32 finalSize = (UI32)(dstFinalSize - obj->lastWritePos);
		const UI32 v = finalSize >> 24 | finalSize << 24 | (finalSize >> 8 & 0xFF00) | (finalSize << 8 & 0xFF0000);
		*((UI32*)&dst->str[obj->lastWritePos]) = v;
	}
	return TRUE;
}

void NBMp4BoxStDef_init(STNBMp4BoxStDef* obj){
	NBMemory_setZeroSt(*obj, STNBMp4BoxStDef);
}

void NBMp4BoxStDef_release(STNBMp4BoxStDef* obj){
	//nothing
}

void NBMp4BoxRef_init(STNBMp4BoxRef* obj){
	NBMemory_setZeroSt(*obj, STNBMp4BoxRef);
	NBMp4BoxStDef_init(&obj->stDef);
}

void NBMp4BoxRef_release(STNBMp4BoxRef* obj){
	if(obj->box != NULL){
		if(obj->stDef.itf.release != NULL){
			(*obj->stDef.itf.release)(obj->box);
		}
		NBMp4BoxStDef_release(&obj->stDef);
		NBMemory_free(obj->box);
		obj->box = NULL;
	}
}

void NBMp4BoxRef_concat(const STNBMp4BoxRef* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(obj->stDef.itf.concat != NULL){
		(*obj->stDef.itf.concat)(obj->box, depthLvl, optSrcFile, dst);
	}
}

BOOL NBMp4BoxRef_writeBits(STNBMp4BoxRef* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(obj->stDef.itf.writeBits != NULL){
		return (*obj->stDef.itf.writeBits)(obj->box, depthLvl, optSrcFile, dst);
	}
	return FALSE;
}

#ifdef NB_CONFIG_INCLUDE_ASSERTS
void NBMp4BoxRef_printLoad_(const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BOOL isFromFile, const UI32 paySz, const UI32 payCrc, const BOOL isGenericBox){
	STNBString str;
	NBString_init(&str);
	NBString_concatRepeatedByte(&str, '\t', depthLvl);
	NBString_concatByte(&str, '\'');
	NBMp4_concatChar(&str, hdr->type.str[0]);
	NBMp4_concatChar(&str, hdr->type.str[1]);
	NBMp4_concatChar(&str, hdr->type.str[2]);
	NBMp4_concatChar(&str, hdr->type.str[3]);
	NBString_concatByte(&str, '\'');
	NBString_concat(&str, " (");
	NBString_concatUI64(&str, boxFileStart);
	NBString_concat(&str, " + 8 + ");
	NBString_concatUI32(&str, paySz);
	NBString_concat(&str, " bytes");
	NBString_concat(&str, "; crc-");
	NBString_concatUI32(&str, payCrc);
	if(hdr->size == 0){
		NBString_concat(&str, "; untill-end-of-container");
	}
	NBString_concat(&str, isFromFile ? ") [from-file]" : ") [from-memory]");
	NBString_concat(&str, isGenericBox ? " [generic]" : " [parsed]");
	PRINTF_INFO("NBMp4BoxRef, box-parsing: %s\n", str.str);
	NBString_release(&str);
}
#endif

BOOL NBMp4BoxRef_allocBoxFromFile(STNBMp4BoxRef* dst, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, STNBFileRef f){
	BOOL r = FALSE; UI32 paySz = 0;
	const STNBMp4BoxDef* def = NBMp4_getBoxDef(hdr->type.str);
	const BOOL isGenericBox = (def == NULL || def->readHint == ENNBMp4BoxReadHint_File || def->stDef.size == 0 || def->stDef.itf.init == NULL || def->stDef.itf.loadBits == NULL);
	if(isGenericBox){
		//Just skip
		if(hdr->size == 0){
			const SI64 startPos = NBFile_curPos(f);
			if(NBFile_seek(f, 0, ENNBFileRelative_End)){
				const SI64 endPos = NBFile_curPos(f);
				paySz = (UI32)(endPos - startPos);
				//print
				IF_NBASSERT(NBMp4BoxRef_printLoad_(depthLvl, boxFileStart, hdrFileSz, hdr, TRUE, paySz, NBMp4_crcFile((UI32)startPos, paySz, f), isGenericBox);)
				//
				if(dst != NULL){
					dst->stDef	= NBMp4BoxStDef_;
					dst->box	= NBMemory_allocType(STNBMp4Box);
					NBMp4Box_init(dst->box);
					NBMp4Box_cloneHdr(dst->box, prnt, boxFileStart, hdrFileSz, paySz, hdr);
					NBASSERT(dst->box->filePos.size == (UI64)(endPos - boxFileStart))
				}
				r = TRUE;
			}
		} else if(hdr->size >= 8){
#			ifdef NB_CONFIG_INCLUDE_ASSERTS
			const SI64 startPos = NBFile_curPos(f);
#			endif
			paySz = (hdr->size - 8);
			if(NBFile_seek(f, paySz, ENNBFileRelative_CurPos)){
				//print
				IF_NBASSERT(NBMp4BoxRef_printLoad_(depthLvl, boxFileStart, hdrFileSz, hdr, TRUE, paySz, NBMp4_crcFile((UI32)startPos, paySz, f), isGenericBox);)
				//
				if(dst != NULL){
					dst->stDef	= NBMp4BoxStDef_;
					dst->box	= NBMemory_allocType(STNBMp4Box);
					NBMp4Box_init(dst->box);
					NBMp4Box_cloneHdr(dst->box, prnt, boxFileStart, hdrFileSz, paySz, hdr);
					NBASSERT((UI64)(NBFile_curPos(f) - boxFileStart) == dst->box->filePos.size)
				}
				r = TRUE;
			}
		}
	} else {
		//Load to memory
		NBASSERT(def != NULL && def->stDef.size > 0 && def->stDef.itf.init != NULL && def->stDef.itf.loadBits != NULL)
		if(hdr->size == 0){
			const SI64 startPos = NBFile_curPos(f);
			if(NBFile_seek(f, 0, ENNBFileRelative_End)){
				const SI64 endPos = NBFile_curPos(f);
				paySz = (UI32)(endPos - startPos);
				if(paySz > 0){
					if(NBFile_seek(f, startPos, ENNBFileRelative_Start)){
						BYTE* buff = NBMemory_alloc(paySz);
						if(NBFile_read(f, buff, (UI32)paySz) == paySz){
							const BYTE* buffPos = buff;
							void* st = NBMemory_alloc(def->stDef.size);
							//print
							IF_NBASSERT(NBMp4BoxRef_printLoad_(depthLvl, boxFileStart, hdrFileSz, hdr, TRUE, paySz, NBCrc32_getHashBytes(buff, paySz), isGenericBox);)
							//
							(*def->stDef.itf.init)(st);
							if((*def->stDef.itf.loadBits)(st, prnt, depthLvl + 1, boxFileStart, hdrFileSz, hdr, &buffPos, buff + paySz)){
								if(dst != NULL){
									dst->stDef	= def->stDef;
									dst->box	= st;
									st			= NULL;
								}
								r = TRUE;
							}
							//release (if not consumed)
							if(st != NULL){
								if(def->stDef.itf.release != NULL){
									(*def->stDef.itf.release)(st);
								}
								NBMemory_free(st);
								st = NULL;
							}
						}
						NBMemory_free(buff);
					}
				}
			}
		} else if(hdr->size > 8){
			BYTE* buff;
			paySz	= (hdr->size - 8);
			buff	= NBMemory_alloc(paySz);
			if(NBFile_read(f, buff, (UI32)paySz) == paySz){
				const BYTE* buffPos = buff;
				void* st = NBMemory_alloc(def->stDef.size);
				//print
				IF_NBASSERT(NBMp4BoxRef_printLoad_(depthLvl, boxFileStart, hdrFileSz, hdr, TRUE, paySz, NBCrc32_getHashBytes(buff, paySz), isGenericBox);)
				//
				(*def->stDef.itf.init)(st);
				if((*def->stDef.itf.loadBits)(st, prnt, depthLvl + 1, boxFileStart, hdrFileSz, hdr, &buffPos, buff + paySz)){
					if(dst != NULL){
						dst->stDef	= def->stDef;
						dst->box	= st;
						st			= NULL;
					}
					r = TRUE;
				}
				//release (if not consumed)
				if(st != NULL){
					if(def->stDef.itf.release != NULL){
						(*def->stDef.itf.release)(st);
					}
					NBMemory_free(st);
					st = NULL;
				}
			}
			NBMemory_free(buff);
		}
	}
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	if(!r){
		STNBString str;
		NBString_init(&str);
		NBString_concatRepeatedByte(&str, '\t', depthLvl);
		NBString_concatByte(&str, '\'');
		NBMp4_concatChar(&str, hdr->type.str[0]);
		NBMp4_concatChar(&str, hdr->type.str[1]);
		NBMp4_concatChar(&str, hdr->type.str[2]);
		NBMp4_concatChar(&str, hdr->type.str[3]);
		NBString_concatByte(&str, '\'');
		NBString_concat(&str, " ( 8 + ");
		NBString_concatUI32(&str, paySz);
		NBString_concat(&str, " bytes");
		if(hdr->size == 0){
			NBString_concat(&str, "; untill-end-of-container");
		}
		NBString_concat(&str, ") [from-file]");
		NBString_concat(&str, isGenericBox ? " [generic]" : " [parsed]");
		PRINTF_INFO("NBMp4BoxRef, box-parsing failed: %s\n", str.str);
		NBString_release(&str);
	}
#	endif
	return r;
}

BOOL NBMp4BoxRef_allocBoxFromBuff(STNBMp4BoxRef* dst, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	const STNBMp4BoxDef* def = NBMp4_getBoxDef(hdr->type.str);
	return NBMp4Box_allocBoxWithDef(&dst->stDef, &dst->box, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd, def);
}

/*
unsigned int(32) size;
unsigned int(32) type = boxtype;
if (size==1) {
	unsigned int(64) largesize;
} else if (size==0) {
	// box extends to end of file
}
if (boxtype==‘uuid’) {
	unsigned int(8)[16] usertype = extended_type;
}
*/

void NBMp4Box_init(STNBMp4Box* obj){
	NBMemory_setZeroSt(*obj, STNBMp4Box);
	NBMp4BoxHdr_init(&obj->hdr);
	NBArray_init(&obj->subBoxes, sizeof(STNBMp4BoxRef), NULL);
}

void NBMp4Box_release(STNBMp4Box* obj){
	//subBoxes
	{
		STNBMp4BoxRef* sub = NBArray_dataPtr(&obj->subBoxes, STNBMp4BoxRef);
		const STNBMp4BoxRef* subAfterEnd = sub + obj->subBoxes.use;
		while(sub < subAfterEnd){
			NBMp4BoxRef_release(sub);
			//next
			sub++;
		}
		NBArray_empty(&obj->subBoxes);
		NBArray_release(&obj->subBoxes);
	}
	NBMp4BoxHdr_release(&obj->hdr);
}

void NBMp4Box_concat(const STNBMp4Box* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	//hdr
	NBMp4Box_concatHdr(obj, depthLvl, optSrcFile, dst);
	//subBoxes
	NBMp4Box_concatChildren(obj, depthLvl, optSrcFile, dst, sizeof(*obj), NULL, 0);
}

void NBMp4Box_concatHdr(const STNBMp4Box* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	//hdr
	NBMp4BoxHdr_concat(&obj->hdr, depthLvl, dst);
	//file-pos
	if(obj->filePos.iStart != 0 || obj->filePos.size != 0){
		NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
		NBString_concat(dst, "filePos("); NBString_concatUI64(dst, obj->filePos.iStart); NBString_concat(dst, " + "); NBString_concatUI64(dst, obj->filePos.size); NBString_concat(dst, ")");
		NBString_concatByte(dst, '\n');
	}
}

BOOL NBMp4Box_writeBits(STNBMp4Box* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4BoxHdr_writeBits(&obj->hdr, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	//payload
	if(!NBFile_isSet(optSrcFile) || obj->filePos.hdrSz < 8 || obj->filePos.size == 0 || obj->filePos.hdrSz > obj->filePos.size){
		return FALSE;
	} else {
		BOOL errFnd = FALSE;
		NBFile_lock(optSrcFile);
		{
			if(!NBFile_seek(optSrcFile, obj->filePos.iStart, ENNBFileRelative_Start)){
				errFnd = TRUE;
			} else {
				BYTE buff[4096];
				//verify type
				if(NBFile_read(optSrcFile, buff, 8) != 8){
					errFnd = TRUE;
				} else if(buff[4] != obj->hdr.type.str[0] || buff[5] != obj->hdr.type.str[1] || buff[6] != obj->hdr.type.str[2] || buff[7] != obj->hdr.type.str[3]){
					errFnd = TRUE;
				} else if(!NBFile_seek(optSrcFile, obj->filePos.hdrSz - 8, ENNBFileRelative_CurPos)){
					errFnd = TRUE;
				} else {
					UI32 toRead, remain = obj->filePos.size - obj->filePos.hdrSz;
					while(remain > 0){
						toRead = (remain > sizeof(buff) ? sizeof(buff) : remain);
						NBASSERT(toRead > 0 && toRead <= sizeof(buff))
						if(NBFile_read(optSrcFile, buff, toRead) != toRead){
							errFnd = TRUE;
							break;
						} else {
							NBString_concatBytes(dst, (const char*)buff, toRead);
							remain -= toRead;
						}
					}
				}
			}
		}
		NBFile_unlock(optSrcFile);
		if(errFnd){
			return FALSE;
		}
	}
	if(!NBMp4BoxHdr_writeHdrFinalSize(&obj->hdr, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

BOOL NBMp4Box_writeHdrBits(STNBMp4Box* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	return NBMp4BoxHdr_writeBits(&obj->hdr, depthLvl, optSrcFile, dst);
}

BOOL NBMp4Box_writeHdrFinalSize(const STNBMp4Box* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	return NBMp4BoxHdr_writeHdrFinalSize(&obj->hdr, depthLvl, optSrcFile, dst);
}

void NBMp4Box_cloneHdr(STNBMp4Box* obj, STNBMp4Box* prnt, const UI64 boxFileStart, const UI8 hdrFileSz, const UI32 payFileSz, const STNBMp4BoxHdr* hdr){
	if(hdr == NULL){
		NBMemory_setZeroSt(obj->hdr, STNBMp4BoxHdr);
	} else {
		obj->hdr = *hdr;
	}
	//
	obj->prnt			= prnt;
	//
	obj->filePos.iStart	= boxFileStart;
	obj->filePos.hdrSz	= hdrFileSz;
	obj->filePos.size	= hdrFileSz + payFileSz;
}

void NBMp4Box_addChildBoxByOwning(STNBMp4Box* obj, STNBMp4BoxRef* refToOwn){
	if(refToOwn != NULL && refToOwn->box != NULL){
		STNBMp4BoxRef ref;
		NBMp4BoxRef_init(&ref);
		ref.stDef		= refToOwn->stDef;
		ref.box			= refToOwn->box;
		refToOwn->box	= NULL;
		NBArray_addValue(&obj->subBoxes, ref);
	}
}


void NBMp4Box_init_(void* obj){
	NBMp4Box_init((STNBMp4Box*)obj);
}

void NBMp4Box_release_(void* obj){
	NBMp4Box_init((STNBMp4Box*)obj);
}

BOOL NBMp4Box_loadBits_(void* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	NBASSERT(FALSE)
	return FALSE;
}

void NBMp4Box_concat_(const void* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	NBMp4Box_concat((STNBMp4Box*)obj, depthLvl, optSrcFile, dst);
}

BOOL NBMp4Box_writeBits_(void* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	return NBMp4Box_writeBits((STNBMp4Box*)obj, depthLvl, optSrcFile, dst);
}

void NBMp4Box_concatChildren(const STNBMp4Box* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst, const UI32 stSz, const STNBMp4BoxChildDef* cDefs, const UI32 cDefsSz){
	//subBoxes
	{
		STNBMp4BoxRef* sub = NBArray_dataPtr(&obj->subBoxes, STNBMp4BoxRef);
		const STNBMp4BoxRef* subAfterEnd = sub + obj->subBoxes.use;
		while(sub < subAfterEnd){
			NBMp4BoxRef_concat(sub, depthLvl + 1, optSrcFile, dst);
			//next
			sub++;
		}
	}
}

BOOL NBMp4Box_loadChildren(STNBMp4Box* obj, const UI32 stSz, const STNBMp4BoxChildDef* cDefs, const UI32 cDefsSz, const UI32 depthLvl, const UI64 filePos, const BYTE** buff, const BYTE* buffAfterEnd){
	BOOL errFnd = FALSE;
	UI32 paySz = 0; const BYTE* buffStart = *buff;
	while(*buff < buffAfterEnd && !errFnd){
		const BYTE* boxStart = *buff;
		STNBMp4BoxHdr hdr;
		NBMp4BoxHdr_init(&hdr);
		if(!NBMp4BoxHdr_loadBits(&hdr, buff, buffAfterEnd)){
			PRINTF_INFO("NBMp4Box, end-of-container-box.\n");
			errFnd = TRUE;
		} else if(hdr.size < 8){
			PRINTF_ERROR("NBMp4Box, sz(%d bytes) must be 8+.\n", hdr.size);
			errFnd = TRUE;
		} else {
			paySz = (hdr.size - 8);
			if(paySz <= 0){
				PRINTF_INFO("NBMp4Box, type('%c%c%c%c') %d bytes.\n", hdr.type.str[0], hdr.type.str[1], hdr.type.str[2], hdr.type.str[3], hdr.size);
				errFnd = TRUE;
			} else {
				const BYTE* payStart = *buff;
				STNBMp4BoxRef ref;
				NBMp4BoxRef_init(&ref);
				if(!NBMp4BoxRef_allocBoxFromBuff(&ref, obj, depthLvl + 1, filePos + (boxStart - buffStart), (UI8)(payStart - boxStart), &hdr, buff, *buff + paySz)){
					PRINTF_ERROR("NBMp4Box, failed('%c%c%c%c') %d bytes.\n", hdr.type.str[0], hdr.type.str[1], hdr.type.str[2], hdr.type.str[3], hdr.size);
					NBMp4BoxRef_release(&ref);
					errFnd = TRUE;
				} else {
					NBArray_addValue(&obj->subBoxes, ref);
					//ToDo: remove
					//buff += paySz;
				}
			}
		}
		NBMp4BoxHdr_release(&hdr);
	}
	return (!errFnd && *buff == buffAfterEnd);
	/*if(cDefs != NULL && cDefsSz > 0){
		UI32 i; for(i = 0; i < cDefsSz; i++){
			const STNBMp4BoxChildDef* cDef = &cDefs[i];
			const STNBMp4BoxDef* def = NBMp4_getBoxDef(cDef->boxId.str);
			NBASSERT(def != NULL)
			if(def != NULL){
				NBASSERT(cDef->stOffset >= 0 && (cDef->stOffset + sizeof(void*)) <= stSz)
				void** ptr = (void**)((BYTE*)st + cDef->stOffset);
				if(*ptr != NULL){
					if(def->stDef.itf.release != NULL){
						(*def->stDef.itf.release)(*ptr);
					}
					NBMemory_free(*ptr);
					*ptr = NULL;
				}
			}
		}
	}*/
}

BOOL NBMp4Box_writeChildren(STNBMp4Box* obj, const UI32 stSz, const STNBMp4BoxChildDef* cDefs, const UI32 cDefsSz, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	//subBoxes
	{
		STNBMp4BoxRef* sub = NBArray_dataPtr(&obj->subBoxes, STNBMp4BoxRef);
		const STNBMp4BoxRef* subAfterEnd = sub + obj->subBoxes.use;
		while(sub < subAfterEnd){
			if(!NBMp4BoxRef_writeBits(sub, depthLvl + 1, optSrcFile, dst)){
				return FALSE;
			}
			//next
			sub++;
		}
	}
	return TRUE;
}

void NBMp4Box_releaseChildren(void* st, const UI32 stSz, const STNBMp4BoxChildDef* cDefs, const UI32 cDefsSz){
	if(cDefs != NULL && cDefsSz > 0){
		UI32 i; for(i = 0; i < cDefsSz; i++){
			const STNBMp4BoxChildDef* cDef = &cDefs[i];
			const STNBMp4BoxDef* def = NBMp4_getBoxDef(cDef->boxId.str);
			NBASSERT(def != NULL)
			if(def != NULL){
				NBASSERT(cDef->stPos >= cDef->stBase && ((cDef->stPos - cDef->stBase) + sizeof(void*)) <= stSz)
				void** ptr = (void**)((BYTE*)st + (cDef->stPos - cDef->stBase));
				if(*ptr != NULL){
					if(def->stDef.itf.release != NULL){
						(*def->stDef.itf.release)(*ptr);
					}
					NBMemory_free(*ptr);
					*ptr = NULL;
				}
			}
		}
	}
}

BOOL NBMp4Box_allocBoxWithDef(STNBMp4BoxStDef* dstStDef, STNBMp4Box** dstBox, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd, const STNBMp4BoxDef* def){
	BOOL r = FALSE; UI32 paySz = 0;
	const BOOL isGenericBox = (def == NULL || def->readHint == ENNBMp4BoxReadHint_File || def->stDef.size == 0 || def->stDef.itf.init == NULL || def->stDef.itf.loadBits == NULL); 
	if(isGenericBox){
		//Just skip
		if(hdr->size == 0){
			paySz = (UI32)(buffAfterEnd - *buff);
			//print
			IF_NBASSERT(NBMp4BoxRef_printLoad_(depthLvl, boxFileStart, hdrFileSz, hdr, FALSE, paySz, NBCrc32_getHashBytes(*buff, paySz), isGenericBox);)
			//
			*buff = buffAfterEnd;
			if(dstStDef != NULL){
				*dstStDef = NBMp4BoxStDef_;
			}
			if(dstBox != NULL){
				*dstBox = NBMemory_allocType(STNBMp4Box);
				NBMp4Box_init(*dstBox);
				NBMp4Box_cloneHdr(*dstBox, prnt, boxFileStart, hdrFileSz, paySz, hdr);
			}
			r = TRUE;
		} else if(hdr->size >= hdrFileSz){
			paySz = (hdr->size - hdrFileSz);
			//print
			IF_NBASSERT(NBMp4BoxRef_printLoad_(depthLvl, boxFileStart, hdrFileSz, hdr, FALSE, paySz, NBCrc32_getHashBytes(*buff, paySz), isGenericBox);)
			//
			*buff = *buff + paySz;
			if(dstStDef != NULL){
				*dstStDef = NBMp4BoxStDef_;
			}
			if(dstBox != NULL){
				*dstBox = NBMemory_allocType(STNBMp4Box);
				NBMp4Box_init(*dstBox);
				NBMp4Box_cloneHdr(*dstBox, prnt, boxFileStart, hdrFileSz, paySz, hdr);
			}
			r = TRUE;
		}
	} else {
		//Load to memory
		NBASSERT(def != NULL && def->stDef.size > 0 && def->stDef.itf.init != NULL && def->stDef.itf.loadBits != NULL)
		if(hdr->size == 0){
			if(*buff < buffAfterEnd){
				void* st;
				paySz	= (UI32)(buffAfterEnd - *buff);
				st		= NBMemory_alloc(def->stDef.size);
				//print
				IF_NBASSERT(NBMp4BoxRef_printLoad_(depthLvl, boxFileStart, hdrFileSz, hdr, FALSE, paySz, NBCrc32_getHashBytes(*buff, paySz), isGenericBox);)
				//
				(*def->stDef.itf.init)(st);
				if((*def->stDef.itf.loadBits)(st, prnt, depthLvl + 1, boxFileStart, hdrFileSz, hdr, buff, *buff + paySz)){
					if(dstStDef != NULL){
						*dstStDef = def->stDef;
					}
					if(dstBox != NULL){
						*dstBox		= st;
						st			= NULL;
					}
					r = TRUE;
				}	
				//release (if not consumed)
				if(st != NULL){
					if(def->stDef.itf.release != NULL){
						(*def->stDef.itf.release)(st);
					}
					NBMemory_free(st);
					st = NULL;
				}
			}
		} else if(hdr->size > hdrFileSz){
			void* st;
			paySz	= (hdr->size - hdrFileSz);
			st		= NBMemory_alloc(def->stDef.size);
			//print
			IF_NBASSERT(NBMp4BoxRef_printLoad_(depthLvl, boxFileStart, hdrFileSz, hdr, FALSE, paySz, NBCrc32_getHashBytes(*buff, paySz), isGenericBox);)
			//
			(*def->stDef.itf.init)(st);
			if((*def->stDef.itf.loadBits)(st, prnt, depthLvl + 1, boxFileStart, hdrFileSz, hdr, buff, *buff + paySz)){
				if(dstStDef != NULL){
					*dstStDef = def->stDef;
				}
				if(dstBox != NULL){
					*dstBox		= st;
					st			= NULL;
				}
				r = TRUE;
			}
			//release (if not consumed)
			if(st != NULL){
				if(def->stDef.itf.release != NULL){
					(*def->stDef.itf.release)(st);
				}
				NBMemory_free(st);
				st = NULL;
			}
		}
	}
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	if(!r){
		STNBString str;
		NBString_init(&str);
		NBString_concatRepeatedByte(&str, '\t', depthLvl);
		NBString_concatByte(&str, '\'');
		NBMp4_concatChar(&str, hdr->type.str[0]);
		NBMp4_concatChar(&str, hdr->type.str[1]);
		NBMp4_concatChar(&str, hdr->type.str[2]);
		NBMp4_concatChar(&str, hdr->type.str[3]);
		NBString_concatByte(&str, '\'');
		NBString_concat(&str, " ( 8 + ");
		NBString_concatUI32(&str, paySz);
		NBString_concat(&str, " bytes");
		if(hdr->size == 0){
			NBString_concat(&str, "; untill-end-of-container");
		}
		NBString_concat(&str, ") [from-memory]");
		NBString_concat(&str, isGenericBox ? " [generic]" : " [parsed]");
		PRINTF_ERROR("NBMp4BoxRef, box-parsing failed: %s\n", str.str);
		NBString_release(&str);
	}
#	endif
	return r;
}

STNBMp4Box* NBMp4Box_alloctIfMatchDef(const STNBMp4BoxDef* def, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const BYTE** buff, const BYTE* buffAfterEnd){
	STNBMp4Box* r = NULL;
	const BYTE* buffStart = *buff;
	STNBMp4BoxHdr hdr;
	NBMp4BoxHdr_init(&hdr);
	if(!NBMp4BoxHdr_loadBits(&hdr, buff, buffAfterEnd)){
		*buff = buffStart;
	} else if(hdr.type.str[0] != def->boxId.str[0] || hdr.type.str[1] != def->boxId.str[1] || hdr.type.str[2] != def->boxId.str[2] || hdr.type.str[3] != def->boxId.str[3]){
		*buff = buffStart;
	} else {
		const BYTE* payStart = *buff;
		const UI32 hdrSz = (UI32)(payStart - buffStart);
		const UI32 paySz = (UI32)hdr.size - hdrSz;
		if(!NBMp4Box_allocBoxWithDef(NULL, &r, prnt, depthLvl, boxFileStart, (UI8)hdrSz, &hdr, buff, *buff + paySz, def)){
			*buff = buffStart;
		}
	}
	NBMp4BoxHdr_release(&hdr);
	return r;
}

//STNBMp4FullBox

/*
aligned(8) class FullBox(unsigned int(32) boxtype, unsigned int(8) v, bit(24) f)
   extends Box(boxtype) {
   unsigned int(8)   version = v;
   bit(24)           flags = f;
}
*/

void NBMp4FullBox_init(STNBMp4FullBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4FullBox);
	NBMp4Box_init(&obj->prntBox);
}

void NBMp4FullBox_release(STNBMp4FullBox* obj){
	NBMp4Box_release(&obj->prntBox);
}

void NBMp4FullBox_concat(const STNBMp4FullBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4Box_concatHdr(&obj->prntBox, depthLvl, optSrcFile, dst);
		{
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "version("); NBString_concatUI32(dst, obj->version); NBString_concat(dst, ")");
			NBString_concat(dst, " flags("); NBString_concatUI32(dst, obj->flags); NBString_concat(dst, ")");
			NBString_concatByte(dst, '\n');
		}
	}
}

BOOL NBMp4FullBox_loadBits(STNBMp4FullBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	BYTE flags[3]; const BYTE* buffStart = *buff;
	if(!NBMp4_readBytes(&obj->version, sizeof(obj->version), buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readBytes(flags, 3, buff, buffAfterEnd)){
		return FALSE;
	}
	{
		obj->flags = ((UI32)flags[0] << 16) | ((UI32)flags[1] << 8) | flags[2];
	}
	//
	NBMp4Box_cloneHdr(&obj->prntBox, prnt, boxFileStart, hdrFileSz, (UI32)(buffAfterEnd - buffStart), hdr);
	//
	return TRUE;
}

UI32 NBMp4FullBox_getHdrBytesLen(const STNBMp4FullBox* obj){
	return
	NBMp4BoxHdr_getHdrBytesLen(&obj->prntBox.hdr)
	+ 4 //version+flags3
	;
}

BOOL NBMp4FullBox_writeHdrBits(STNBMp4FullBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4Box_writeHdrBits(&obj->prntBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(!NBMp4_writeBytes(&obj->version, sizeof(obj->version), dst)){
		return FALSE;
	}
	{
		const BYTE* flags = (const BYTE*)&obj->flags;
		if(!NBMp4_writeBytes(&flags[2], 1, dst)){
			return FALSE;
		}
		if(!NBMp4_writeBytes(&flags[1], 1, dst)){
			return FALSE;
		}
		if(!NBMp4_writeBytes(&flags[0], 1, dst)){
			return FALSE;
		}
	}
	if(!NBMp4Box_writeHdrFinalSize(&obj->prntBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

BOOL NBMp4FullBox_writeHdrFinalSize(const STNBMp4FullBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	return NBMp4Box_writeHdrFinalSize(&obj->prntBox, depthLvl, optSrcFile, dst);
}

//NBMp4SampleFlags

void NBMp4SampleFlags_init(STNBMp4SampleFlags* obj){
	NBMemory_setZeroSt(*obj, STNBMp4SampleFlags);
}

void NBMp4SampleFlags_release(STNBMp4SampleFlags* obj){
	//nothing
}

BOOL NBMp4SampleFlags_loadFromBuff(STNBMp4SampleFlags* obj, const BYTE** buff, const BYTE* buffAfterEnd){
	if(!NBMp4_readUI32(&obj->flags, buff, buffAfterEnd)){
		return FALSE;
	}
	return TRUE;
}

BOOL NBMp4SampleFlags_writeBits(STNBMp4SampleFlags* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4_writeUI32(&obj->flags, dst)){
		return FALSE;
	}
	return TRUE;
}

void NBMp4SampleFlags_concat(const STNBMp4SampleFlags* obj, STNBString* dst, const char* prefix){
	//prefix
	if(!NBString_strIsEmpty(prefix)){
		NBString_concat(dst, prefix);
	}
	NBString_concat(dst, "(");
	//NBString_concat(dst, "reserved("); NBString_concatUI32(dst, ((obj->flags >> 28) & 0xF)); NBString_concat(dst, ")");
	{
		const ENNBMp4SampleFlagsLeading v = ((obj->flags >> 26) & 0x3);
		switch(v) {
			case ENNBMp4SampleFlagsLeading_Unknown: /*NBString_concat(dst, " leading-unknown");*/ break;
			case ENNBMp4SampleFlagsLeading_Dependant: NBString_concat(dst, " leading-dependant"); break;
			case ENNBMp4SampleFlagsLeading_NonLeading: NBString_concat(dst, " nonLeading"); break;
			case ENNBMp4SampleFlagsLeading_NonDependent: NBString_concat(dst, "leading-nonDependent"); break;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	{
		const ENNBMp4SampleFlagsDependedsOn v = ((obj->flags >> 24) & 0x3);
		switch(v) {
			case ENNBMp4SampleFlagsDependedsOn_Unknown: /*NBString_concat(dst, " depends-unknown");*/ break;
			case ENNBMp4SampleFlagsDependedsOn_Depends: NBString_concat(dst, " dependant"); break;
			case ENNBMp4SampleFlagsDependedsOn_Independent: NBString_concat(dst, " independent"); break;
			case ENNBMp4SampleFlagsDependedsOn_Count: NBString_concat(dst, " depends-reserved"); break;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	{
		const ENNBMp4SampleFlagsIsDependedOn v = ((obj->flags >> 22) & 0x3);
		switch(v) {
			case ENNBMp4SampleFlagsIsDependedOn_Unknown: /*NBString_concat(dst, " disposable-unknown");*/ break;
			case ENNBMp4SampleFlagsIsDependedOn_NonDisposable: NBString_concat(dst, " nonDisposable"); break;
			case ENNBMp4SampleFlagsIsDependedOn_Disposable: NBString_concat(dst, " disposable"); break;
			case ENNBMp4SampleFlagsIsDependedOn_Count: NBString_concat(dst, " disposable-reserved"); break;
			default:
				NBASSERT(FALSE) 
				break;
		}
	}
	{
		const ENNBMp4SampleFlagsHasRedundancy v = ((obj->flags >> 20) & 0x3); 
		switch(v) {
			case ENNBMp4SampleFlagsHasRedundancy_Unknown: /*NBString_concat(dst, " redundancy-unknown");*/ break;
			case ENNBMp4SampleFlagsHasRedundancy_Redundant: NBString_concat(dst, " redundant"); break;
			case ENNBMp4SampleFlagsHasRedundancy_NonRedundant: NBString_concat(dst, " nonRedundant"); break;
			case ENNBMp4SampleFlagsHasRedundancy_Count: NBString_concat(dst, " redundancy-reserved"); break;
			default:
				NBASSERT(FALSE) 
				break;
		}
	}
	//NBString_concat(dst, " sample_padding_value("); NBString_concatUI32(dst, ((obj->flags >> 17) & 0x7)); NBString_concat(dst, ")");
	NBString_concat(dst, ((obj->flags >> 16) & 0x1) ? " non_sync_sample" : " sync_sample");
	if((obj->flags & 0xFF) != 0){
		NBString_concat(dst, " degradation_priority("); NBString_concatUI32(dst, (obj->flags & 0xFF)); NBString_concat(dst, ")");
	}
	NBString_concat(dst, " )");
}

void NBMp4SampleFlags_setFromOther(STNBMp4SampleFlags* obj, const STNBMp4SampleFlags* other){
	obj->flags = other->flags;
}

BOOL NBMp4SampleFlags_setIsLeading(STNBMp4SampleFlags* obj, const ENNBMp4SampleFlagsLeading v){ //unsigned int(2)
	BOOL r = FALSE;
	if(v <= 0x3){ //2 bits only
		obj->flags |= ((UI32)v << 26);
	}
	return r;
}

BOOL NBMp4SampleFlags_setSampleDependsOn(STNBMp4SampleFlags* obj, const ENNBMp4SampleFlagsDependedsOn v){ //unsigned int(2)
	BOOL r = FALSE;
	if(v <= 0x3){ //2 bits only
		obj->flags |= ((UI32)v << 24);
	}
	return r;
}

BOOL NBMp4SampleFlags_setSampleIsDependsOn(STNBMp4SampleFlags* obj, const ENNBMp4SampleFlagsIsDependedOn v){ //unsigned int(2)
	BOOL r = FALSE;
	if(v <= 0x3){ //2 bits only
		obj->flags |= ((UI32)v << 22);
	}
	return r;
}

BOOL NBMp4SampleFlags_setSampleHasRedundancy(STNBMp4SampleFlags* obj, const ENNBMp4SampleFlagsHasRedundancy v){ //unsigned int(2)
	BOOL r = FALSE;
	if(v <= 0x3){ //2 bits only
		obj->flags |= ((UI32)v << 20);
	}
	return r;
}

BOOL NBMp4SampleFlags_setSampleIsNonSyncSample(STNBMp4SampleFlags* obj, const BOOL v){ //bit(1)
	BOOL r = FALSE;
	if(v){
		obj->flags |= ((UI32)0x1 << 16);
	} else {
		obj->flags &= ~((UI32)0x1 << 16);
	}
	return r;
}

void NBMp4SampleFlags_setSampleDegradationPriority(STNBMp4SampleFlags* obj, const UI16 v){ //unsigned int(16)
	obj->flags |= (UI32)v;
}

//NBMp4FileTypeBox

/*
aligned(8) class FileTypeBox extends Box(‘ftyp’) {
 unsigned int(32) major_brand;
 unsigned int(32) minor_version;
 unsigned int(32) compatible_brands[]; // to end of the box
}*/

void NBMp4FileTypeBox_init(STNBMp4FileTypeBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4FileTypeBox);
	NBMp4Box_init(&obj->prntBox);
	obj->prntBox.hdr.type.str[0] = 'f';
	obj->prntBox.hdr.type.str[1] = 't';
	obj->prntBox.hdr.type.str[2] = 'y';
	obj->prntBox.hdr.type.str[3] = 'p';
}

void NBMp4FileTypeBox_release(STNBMp4FileTypeBox* obj){
	//compatibleBrands
	{
		if(obj->compatibleBrands != NULL){
			NBMemory_free(obj->compatibleBrands);
			obj->compatibleBrands	= NULL;
		}
		obj->compatibleBrandsSz = 0;
	}
	NBMp4Box_release(&obj->prntBox);
}
	
BOOL NBMp4FileTypeBox_loadFromBuff(STNBMp4FileTypeBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	const BYTE* buffStart = *buff;
	if(!NBMp4_readBytes(&obj->majorBrand, 4, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->minorVersion, buff, buffAfterEnd)){
		return FALSE;
	}
	//compatibleBrands
	{
		UI32 compatibleBrandsSz = 0;
		STNBMp4Id4* compatibleBrands = NULL;
		if(((buffAfterEnd - *buff) % 4) == 0){
			compatibleBrandsSz = (UI32)((buffAfterEnd - *buff) / 4);
			if(compatibleBrandsSz > 0){
				UI32 i;
				compatibleBrands			= NBMemory_allocTypes(STNBMp4Id4, compatibleBrandsSz);
				for(i = 0; i < compatibleBrandsSz; i++){
					STNBMp4Id4* idd = &compatibleBrands[i];
					if(!NBMp4_readBytes(idd, 4, buff, buffAfterEnd)){
						break;
					}
				}
				//failed
				if(i < compatibleBrandsSz){
					if(compatibleBrands != NULL){
						NBMemory_free(compatibleBrands);
						compatibleBrands	= NULL;
						compatibleBrandsSz	= 0;
					}
					return FALSE;
				}
			}
		}
		{
			if(obj->compatibleBrands != NULL){
				NBMemory_free(obj->compatibleBrands);
				obj->compatibleBrands	= NULL;
			}
			obj->compatibleBrands	= compatibleBrands;
			obj->compatibleBrandsSz	= compatibleBrandsSz;
		}
	}
	//
	NBMp4Box_cloneHdr(&obj->prntBox, prnt, boxFileStart, hdrFileSz, (UI32)(buffAfterEnd - buffStart), hdr);
	//Apply
	return (*buff == buffAfterEnd);
}

void NBMp4FileTypeBox_concat(const STNBMp4FileTypeBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4Box_concatHdr(&obj->prntBox, depthLvl, optSrcFile, dst);
		{
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "majorBrand('"); NBMp4_concatChar(dst, obj->majorBrand.str[0]); NBMp4_concatChar(dst, obj->majorBrand.str[1]); NBMp4_concatChar(dst, obj->majorBrand.str[2]); NBMp4_concatChar(dst, obj->majorBrand.str[3]); NBString_concat(dst, "')");
			NBString_concat(dst, " minorVersion("); NBString_concatUI32(dst, obj->minorVersion); NBString_concat(dst, ")");
			NBString_concatByte(dst, '\n');
		}
		if(obj->compatibleBrandsSz > 0){
			UI32 i;
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "compatibleBrands('");
			for(i = 0; i < obj->compatibleBrandsSz; i++){
				const STNBMp4Id4* b = &obj->compatibleBrands[i];
				if(i != 0){ NBString_concat(dst, "', '"); } 
				 NBMp4_concatChar(dst, b->str[0]); NBMp4_concatChar(dst, b->str[1]); NBMp4_concatChar(dst, b->str[2]); NBMp4_concatChar(dst, b->str[3]);
			}
			NBString_concat(dst, "')");
			NBString_concatByte(dst, '\n');
		}
	}
}

BOOL NBMp4FileTypeBox_writeBits(STNBMp4FileTypeBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4Box_writeHdrBits(&obj->prntBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(!NBMp4_writeBytes(&obj->majorBrand, 4, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->minorVersion, dst)){
		return FALSE;
	}
	//compatibleBrands
	if(obj->compatibleBrandsSz > 0){
		UI32 i; for(i = 0; i < obj->compatibleBrandsSz; i++){
			const STNBMp4Id4* b = &obj->compatibleBrands[i];
			if(!NBMp4_writeBytes(b->str, 4, dst)){
				return FALSE;
			}
		}
	}
	if(!NBMp4Box_writeHdrFinalSize(&obj->prntBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

BOOL NBMp4FileTypeBox_setMajorBrand(STNBMp4FileTypeBox* obj, const char* majorBrand, const UI32 minorVersion){
	BOOL r = FALSE;
	if(majorBrand != NULL && majorBrand[0] != '\0' && majorBrand[1] != '\0' && majorBrand[2] != '\0' && majorBrand[3] != '\0'){
		obj->majorBrand.str[0] = majorBrand[0];
		obj->majorBrand.str[1] = majorBrand[1];
		obj->majorBrand.str[2] = majorBrand[2];
		obj->majorBrand.str[3] = majorBrand[3];
		obj->minorVersion = minorVersion;
		r = TRUE;
	}
	return r;
}

BOOL NBMp4FileTypeBox_addCompatibleBrand(STNBMp4FileTypeBox* obj, const char* brand){
	BOOL r = FALSE;
	//search
	if(brand != NULL && brand[0] != '\0' && brand[1] != '\0' && brand[2] != '\0' && brand[3] != '\0'){
		UI32 i; for(i = 0; i < obj->compatibleBrandsSz; i++){
			const STNBMp4Id4* b = &obj->compatibleBrands[i];
			if(b->str[0] == brand[0] && b->str[1] == brand[1] && b->str[2] == brand[2] && b->str[3] == brand[3]){
				break;
			}
		}
		//Add
		if(i == obj->compatibleBrandsSz){
			STNBMp4Id4* nArr = NBMemory_allocTypes(STNBMp4Id4, obj->compatibleBrandsSz + 1);
			if(obj->compatibleBrands != NULL){
				if(obj->compatibleBrandsSz > 0){
					NBMemory_copy(nArr, obj->compatibleBrands, sizeof(obj->compatibleBrands[0]) * obj->compatibleBrandsSz);
				}
				NBMemory_free(obj->compatibleBrands);
			}
			obj->compatibleBrands = nArr;
			{
				STNBMp4Id4* b = &obj->compatibleBrands[obj->compatibleBrandsSz++];
				NBMemory_setZeroSt(*b, STNBMp4Id4);
				b->str[0] = brand[0];
				b->str[1] = brand[1];
				b->str[2] = brand[2];
				b->str[3] = brand[3];
			}
		}
		r = TRUE;
	}
	return r;
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(FileTypeBox)

//NBMp4MovieHeaderBox

/*aligned(8) class MovieHeaderBox extends FullBox(‘mvhd’, version, 0) {
   if (version==1) {
	  unsigned int(64)  creation_time;
	  unsigned int(64)  modification_time;
	  unsigned int(32)  timescale;
	  unsigned int(64)  duration;
   } else { // version==0
	  unsigned int(32)  creation_time;
	  unsigned int(32)  modification_time;
	  unsigned int(32)  timescale;
	  unsigned int(32)  duration;
   }
   template int(32)  rate = 0x00010000; // typically 1.0
   template int(16)  volume = 0x0100;   // typically, full volume
   const bit(16)  reserved = 0;
   const unsigned int(32)[2]  reserved = 0;
   template int(32)[9]  matrix = { 0x00010000,0,0,0,0x00010000,0,0,0,0x40000000 }; // Unity matrix
   bit(32)[6]  pre_defined = 0;
   unsigned int(32)  next_track_ID;
}*/

void NBMp4MovieHeaderBox_init(STNBMp4MovieHeaderBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4MovieHeaderBox);
	NBMp4FullBox_init(&obj->prntFullBox);
	//
	obj->prntFullBox.prntBox.hdr.type.str[0] = 'm';
	obj->prntFullBox.prntBox.hdr.type.str[1] = 'v';
	obj->prntFullBox.prntBox.hdr.type.str[2] = 'h';
	obj->prntFullBox.prntBox.hdr.type.str[3] = 'd';
	//cur-time
	{
		const UI64 t1901 = NBMP4_SECS_FROM_1901_TO_UTC1970 + NBDatetime_getCurUTCTimestamp();
		if(t1901 > 0xFFFFFFFFU){
			obj->prntFullBox.version = 1;
			obj->hdr.v1.creationTime = obj->hdr.v1.modificationTime = t1901;
		} else {
			obj->prntFullBox.version = 0;
			obj->hdr.v0.creationTime = obj->hdr.v0.modificationTime = (UI32)t1901;
		}
	}
	//
	obj->rate		= 0x00010000;	// typically 1.0
	obj->volume		= 0x0100;		// typically, full volume
	// Unity matrix
	obj->matrix[0]	= 0x00010000;	obj->matrix[1]	= 0;			obj->matrix[2]	= 0;
	obj->matrix[3]	= 0;			obj->matrix[4]	= 0x00010000;	obj->matrix[5]	= 0;
	obj->matrix[6]	= 0;			obj->matrix[7]	= 0;			obj->matrix[8]	= 0x40000000;
}

void NBMp4MovieHeaderBox_release(STNBMp4MovieHeaderBox* obj){
	NBMp4FullBox_release(&obj->prntFullBox);
}

BOOL NBMp4MovieHeaderBox_loadFromBuff(STNBMp4MovieHeaderBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	if(!NBMp4FullBox_loadBits(&obj->prntFullBox, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd)){
		return FALSE;
	} else if(obj->prntFullBox.flags != 0){
		return FALSE;
	}
	if(obj->prntFullBox.version == 1){
		if(!NBMp4_readUI64(&obj->hdr.v1.creationTime, buff, buffAfterEnd)){
			return FALSE;
		}
		if(!NBMp4_readUI64(&obj->hdr.v1.modificationTime, buff, buffAfterEnd)){
			return FALSE;
		}
		if(!NBMp4_readUI64(&obj->hdr.v1.timescale, buff, buffAfterEnd)){
			return FALSE;
		}
		if(!NBMp4_readUI64(&obj->hdr.v1.duration, buff, buffAfterEnd)){
			return FALSE;
		}
	} else if(obj->prntFullBox.version == 0){
		if(!NBMp4_readUI32(&obj->hdr.v0.creationTime, buff, buffAfterEnd)){
			return FALSE;
		}
		if(!NBMp4_readUI32(&obj->hdr.v0.modificationTime, buff, buffAfterEnd)){
			return FALSE;
		}
		if(!NBMp4_readUI32(&obj->hdr.v0.timescale, buff, buffAfterEnd)){
			return FALSE;
		}
		if(!NBMp4_readUI32(&obj->hdr.v0.duration, buff, buffAfterEnd)){
			return FALSE;
		}
	} else {
		return FALSE;
	}
	//
	if(!NBMp4_readSI32(&obj->rate, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readSI16(&obj->volume, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readSI16(&obj->reserved, buff, buffAfterEnd)){
		return FALSE;
	} else if(obj->reserved != 0){
		return FALSE;
	}
	if(!NBMp4_readUI32s(&obj->reserved2[0], 2, buff, buffAfterEnd)){
		return FALSE;
	//} else if(obj->reserved2[0] != 0 || obj->reserved2[1] != 0){
	//	return FALSE;
	}
	if(!NBMp4_readSI32s(&obj->matrix[0], 9, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readSI32s(&obj->preDefined[0], 6, buff, buffAfterEnd)){
		return FALSE;
	//} else if(obj->preDefined[0] != 0 || obj->preDefined[1] != 0 || obj->preDefined[2] != 0 || obj->preDefined[3] != 0 || obj->preDefined[4] != 0 || obj->preDefined[5] != 0){
	//	return FALSE;
	}
	if(!NBMp4_readUI32(&obj->nextTrackId, buff, buffAfterEnd)){
		return FALSE;
	}
	return (*buff == buffAfterEnd);
}

void NBMp4MovieHeaderBox_concat(const STNBMp4MovieHeaderBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4FullBox_concat(&obj->prntFullBox, depthLvl, optSrcFile, dst);
		//
		if(obj->prntFullBox.version == 1){
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "creation("); NBMp4_concatTimestamp(dst, obj->hdr.v1.creationTime); NBString_concat(dst, ")");
			NBString_concat(dst, " mod("); NBMp4_concatTimestamp(dst, obj->hdr.v1.modificationTime); NBString_concat(dst, ")");
			NBString_concat(dst, " timeScale("); NBString_concatUI64(dst, obj->hdr.v1.timescale); NBString_concat(dst, ")");
			NBString_concat(dst, " duration("); NBString_concatUI64(dst, obj->hdr.v1.duration); NBString_concat(dst, ")");
			NBString_concatByte(dst, '\n');
		} else {
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "creation("); NBMp4_concatTimestamp(dst, obj->hdr.v0.creationTime); NBString_concat(dst, ")");
			NBString_concat(dst, " mod("); NBMp4_concatTimestamp(dst, obj->hdr.v0.modificationTime); NBString_concat(dst, ")");
			NBString_concat(dst, " timeScale("); NBString_concatUI32(dst, obj->hdr.v0.timescale); NBString_concat(dst, ")");
			NBString_concat(dst, " duration("); NBString_concatUI32(dst, obj->hdr.v0.duration); NBString_concat(dst, ")");
			NBString_concatByte(dst, '\n');
		}
		{
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "rate("); NBMp4_concatFixed16x16(dst, obj->rate); NBString_concat(dst, ")");
			NBString_concat(dst, " volume("); NBMp4_concatFixed8x8(dst, obj->volume); NBString_concat(dst, ")");
			//SI16	reserved;		//0
			//UI32	reserved2[2];	//[0, 0]
			NBString_concat(dst, " matrix("); NBMp4_concatMatrix9(dst, obj->matrix); NBString_concat(dst, ")");
			//SI32	preDefined[6];	//0
			NBString_concat(dst, " nxtTrackId("); NBString_concatUI32(dst, obj->nextTrackId); NBString_concat(dst, ")");
			NBString_concatByte(dst, '\n');
		}
	}
}

BOOL NBMp4MovieHeaderBox_writeBits(STNBMp4MovieHeaderBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4FullBox_writeHdrBits(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(obj->prntFullBox.version == 1){
		if(!NBMp4_writeUI64(&obj->hdr.v1.creationTime, dst)){
			return FALSE;
		}
		if(!NBMp4_writeUI64(&obj->hdr.v1.modificationTime, dst)){
			return FALSE;
		}
		if(!NBMp4_writeUI64(&obj->hdr.v1.timescale, dst)){
			return FALSE;
		}
		if(!NBMp4_writeUI64(&obj->hdr.v1.duration, dst)){
			return FALSE;
		}
	} else if(obj->prntFullBox.version == 0){
		if(!NBMp4_writeUI32(&obj->hdr.v0.creationTime, dst)){
			return FALSE;
		}
		if(!NBMp4_writeUI32(&obj->hdr.v0.modificationTime, dst)){
			return FALSE;
		}
		if(!NBMp4_writeUI32(&obj->hdr.v0.timescale, dst)){
			return FALSE;
		}
		if(!NBMp4_writeUI32(&obj->hdr.v0.duration, dst)){
			return FALSE;
		}
	} else {
		return FALSE;
	}
	//
	if(!NBMp4_writeSI32(&obj->rate, dst)){
		return FALSE;
	}
	if(!NBMp4_writeSI16(&obj->volume, dst)){
		return FALSE;
	}
	if(!NBMp4_writeSI16(&obj->reserved, dst)){
		return FALSE;
	} else if(obj->reserved != 0){
		return FALSE;
	}
	if(!NBMp4_writeUI32s(&obj->reserved2[0], 2, dst)){
		return FALSE;
	//} else if(obj->reserved2[0] != 0 || obj->reserved2[1] != 0){
		//	return FALSE;
	}
	if(!NBMp4_writeSI32s(&obj->matrix[0], 9, dst)){
		return FALSE;
	}
	if(!NBMp4_writeSI32s(&obj->preDefined[0], 6, dst)){
		return FALSE;
	//} else if(obj->preDefined[0] != 0 || obj->preDefined[1] != 0 || obj->preDefined[2] != 0 || obj->preDefined[3] != 0 || obj->preDefined[4] != 0 || obj->preDefined[5] != 0){
		//	return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->nextTrackId, dst)){
		return FALSE;
	}
	if(!NBMp4FullBox_writeHdrFinalSize(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

void NBMp4MovieHeaderBox_setTimescale(STNBMp4MovieHeaderBox* obj, const UI64 timescale){
	if(obj->prntFullBox.version == 0 && timescale > 0xFFFFFFFFU){
		//change to version 1
		const UI64 creationTime			= obj->hdr.v0.creationTime;
		const UI64 modificationTime		= obj->hdr.v0.modificationTime;
		const UI64 duration				= obj->hdr.v0.duration;
		obj->prntFullBox.version		= 1;
		obj->hdr.v1.creationTime		= creationTime;
		obj->hdr.v1.modificationTime	= modificationTime;
		obj->hdr.v1.duration			= duration;
	}
	if(obj->prntFullBox.version == 1){
		obj->hdr.v1.timescale	= timescale;
	} else {
		obj->hdr.v0.timescale	= (UI32)timescale;
	}
}

void NBMp4MovieHeaderBox_setDuration(STNBMp4MovieHeaderBox* obj, const UI64 duration){
	if(obj->prntFullBox.version == 0 && duration > 0xFFFFFFFFU){
		//change to version 1
		const UI64 creationTime			= obj->hdr.v0.creationTime;
		const UI64 modificationTime		= obj->hdr.v0.modificationTime;
		const UI64 timescale			= obj->hdr.v0.timescale;
		obj->prntFullBox.version		= 1;
		obj->hdr.v1.creationTime		= creationTime;
		obj->hdr.v1.modificationTime	= modificationTime;
		obj->hdr.v1.timescale			= timescale;
	}
	if(obj->prntFullBox.version == 1){
		obj->hdr.v1.duration	= duration;
	} else {
		obj->hdr.v0.duration	= (UI32)duration;
	}
}

void NBMp4MovieHeaderBox_setNextTrackId(STNBMp4MovieHeaderBox* obj, const UI32 nextTrackId){
	obj->nextTrackId = nextTrackId;
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(MovieHeaderBox)

//NBMp4MediaHeaderBox

/*aligned(8) class MediaHeaderBox extends FullBox(‘mdhd’, version, 0) {
   if (version==1) {
	  unsigned int(64)  creation_time;
	  unsigned int(64)  modification_time;
	  unsigned int(32)  timescale;
	  unsigned int(64)  duration;
   } else { // version==0
	  unsigned int(32)  creation_time;
	  unsigned int(32)  modification_time;
	  unsigned int(32)  timescale;
	  unsigned int(32)  duration;
   }
   bit(1)   pad = 0;
   unsigned int(5)[3]   language;   // ISO-639-2/T language code
   unsigned int(16)  pre_defined = 0;
}*/

void NBMp4MediaHeaderBox_init(STNBMp4MediaHeaderBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4MediaHeaderBox);
	NBMp4FullBox_init(&obj->prntFullBox);
	//
	obj->prntFullBox.prntBox.hdr.type.str[0] = 'm';
	obj->prntFullBox.prntBox.hdr.type.str[1] = 'd';
	obj->prntFullBox.prntBox.hdr.type.str[2] = 'h';
	obj->prntFullBox.prntBox.hdr.type.str[3] = 'd';
	//cur-time
	{
		const UI64 t1901 = NBMP4_SECS_FROM_1901_TO_UTC1970 + NBDatetime_getCurUTCTimestamp();
		if(t1901 > 0xFFFFFFFFU){
			obj->prntFullBox.version = 1;
			obj->hdr.v1.creationTime = obj->hdr.v1.modificationTime = t1901;
		} else {
			obj->prntFullBox.version = 0;
			obj->hdr.v0.creationTime = obj->hdr.v0.modificationTime = (UI32)t1901;
		}
	}
	obj->language[0] = 'u'; obj->language[1] = 'n'; obj->language[2] = 'd';
}

void NBMp4MediaHeaderBox_release(STNBMp4MediaHeaderBox* obj){
	NBMp4FullBox_release(&obj->prntFullBox);
}

BOOL NBMp4MediaHeaderBox_loadFromBuff(STNBMp4MediaHeaderBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	if(!NBMp4FullBox_loadBits(&obj->prntFullBox, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd)){
		return FALSE;
	} else if(obj->prntFullBox.flags != 0){
		return FALSE;
	}
	if(obj->prntFullBox.version == 1){
		if(!NBMp4_readUI64(&obj->hdr.v1.creationTime, buff, buffAfterEnd)){
			return FALSE;
		}
		if(!NBMp4_readUI64(&obj->hdr.v1.modificationTime, buff, buffAfterEnd)){
			return FALSE;
		}
		if(!NBMp4_readUI64(&obj->hdr.v1.timescale, buff, buffAfterEnd)){
			return FALSE;
		}
		if(!NBMp4_readUI64(&obj->hdr.v1.duration, buff, buffAfterEnd)){
			return FALSE;
		}
	} else if(obj->prntFullBox.version == 0){
		if(!NBMp4_readUI32(&obj->hdr.v0.creationTime, buff, buffAfterEnd)){
			return FALSE;
		}
		if(!NBMp4_readUI32(&obj->hdr.v0.modificationTime, buff, buffAfterEnd)){
			return FALSE;
		}
		if(!NBMp4_readUI32(&obj->hdr.v0.timescale, buff, buffAfterEnd)){
			return FALSE;
		}
		if(!NBMp4_readUI32(&obj->hdr.v0.duration, buff, buffAfterEnd)){
			return FALSE;
		}
	} else {
		return FALSE;
	}
	{
		UI16 language = 0;
		if(!NBMp4_readUI16(&language, buff, buffAfterEnd)){
			return FALSE;
		} else {
			//Each character is packed as the difference between its ASCII value and 0x60.
			//Since the code is confined to being three lower‐case letters,
			//these values are strictly positive.
			obj->language[0] = 0x60 + (language >> 10 & 0x1F);
			obj->language[1] = 0x60 + (language >> 5 & 0x1F);
			obj->language[2] = 0x60 + (language & 0x1F);
		}
	}
	if(!NBMp4_readSI16(&obj->preDefined, buff, buffAfterEnd)){
		return FALSE;
	} else if(obj->preDefined != 0){
		return FALSE;
	}
	return (*buff == buffAfterEnd);
}

void NBMp4MediaHeaderBox_concat(const STNBMp4MediaHeaderBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4FullBox_concat(&obj->prntFullBox, depthLvl, optSrcFile, dst);
		//
		if(obj->prntFullBox.version == 1){
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "creation("); NBMp4_concatTimestamp(dst, obj->hdr.v1.creationTime); NBString_concat(dst, ")");
			NBString_concat(dst, " mod("); NBMp4_concatTimestamp(dst, obj->hdr.v1.modificationTime); NBString_concat(dst, ")");
			NBString_concat(dst, " timeScale("); NBString_concatUI64(dst, obj->hdr.v1.timescale); NBString_concat(dst, ")");
			NBString_concat(dst, " duration("); NBString_concatUI64(dst, obj->hdr.v1.duration); NBString_concat(dst, ")");
			NBString_concatByte(dst, '\n');
		} else {
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "creation("); NBMp4_concatTimestamp(dst, obj->hdr.v0.creationTime); NBString_concat(dst, ")");
			NBString_concat(dst, " mod("); NBMp4_concatTimestamp(dst, obj->hdr.v0.modificationTime); NBString_concat(dst, ")");
			NBString_concat(dst, " timeScale("); NBString_concatUI32(dst, obj->hdr.v0.timescale); NBString_concat(dst, ")");
			NBString_concat(dst, " duration("); NBString_concatUI32(dst, obj->hdr.v0.duration); NBString_concat(dst, ")");
			NBString_concatByte(dst, '\n');
		}
		{
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "lang('"); NBMp4_concatChar(dst, obj->language[0]); NBMp4_concatChar(dst, obj->language[1]); NBMp4_concatChar(dst, obj->language[2]); NBString_concat(dst, "')");
			//SI16	preDefined;		//0
			NBString_concatByte(dst, '\n');
		}
	}
}

BOOL NBMp4MediaHeaderBox_writeBits(STNBMp4MediaHeaderBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4FullBox_writeHdrBits(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(obj->prntFullBox.version == 1){
		if(!NBMp4_writeUI64(&obj->hdr.v1.creationTime, dst)){
			return FALSE;
		}
		if(!NBMp4_writeUI64(&obj->hdr.v1.modificationTime, dst)){
			return FALSE;
		}
		if(!NBMp4_writeUI64(&obj->hdr.v1.timescale, dst)){
			return FALSE;
		}
		if(!NBMp4_writeUI64(&obj->hdr.v1.duration, dst)){
			return FALSE;
		}
	} else if(obj->prntFullBox.version == 0){
		if(!NBMp4_writeUI32(&obj->hdr.v0.creationTime, dst)){
			return FALSE;
		}
		if(!NBMp4_writeUI32(&obj->hdr.v0.modificationTime, dst)){
			return FALSE;
		}
		if(!NBMp4_writeUI32(&obj->hdr.v0.timescale, dst)){
			return FALSE;
		}
		if(!NBMp4_writeUI32(&obj->hdr.v0.duration, dst)){
			return FALSE;
		}
	} else {
		return FALSE;
	}
	{
		//Each character is packed as the difference between its ASCII value and 0x60.
		//Since the code is confined to being three lower‐case letters,
		//these values are strictly positive.
		const UI16 language = (((obj->language[0] - 0x60) & 0x1F) << 10) | (((obj->language[1] - 0x60) & 0x1F) << 5) | ((obj->language[2] - 0x60) & 0x1F);
		if(!NBMp4_writeUI16(&language, dst)){
			return FALSE;
		}
	}
	if(!NBMp4_writeSI16(&obj->preDefined, dst)){
		return FALSE;
	} else if(obj->preDefined != 0){
		return FALSE;
	}
	if(!NBMp4FullBox_writeHdrFinalSize(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

void NBMp4MediaHeaderBox_setTimescale(STNBMp4MediaHeaderBox* obj, const UI64 timescale){
	if(obj->prntFullBox.version == 0 && timescale > 0xFFFFFFFFU){
		//change to version 1
		const UI64 creationTime			= obj->hdr.v0.creationTime;
		const UI64 modificationTime		= obj->hdr.v0.modificationTime;
		const UI64 duration				= obj->hdr.v0.duration;
		obj->prntFullBox.version		= 1;
		obj->hdr.v1.creationTime		= creationTime;
		obj->hdr.v1.modificationTime	= modificationTime;
		obj->hdr.v1.duration			= duration;
	}
	if(obj->prntFullBox.version == 1){
		obj->hdr.v1.timescale	= timescale;
	} else {
		obj->hdr.v0.timescale	= (UI32)timescale;
	}
}

void NBMp4MediaHeaderBox_setDuration(STNBMp4MediaHeaderBox* obj, const UI64 duration){
	if(obj->prntFullBox.version == 0 && duration > 0xFFFFFFFFU){
		//change to version 1
		const UI64 creationTime			= obj->hdr.v0.creationTime;
		const UI64 modificationTime		= obj->hdr.v0.modificationTime;
		const UI64 timescale			= obj->hdr.v0.timescale;
		obj->prntFullBox.version		= 1;
		obj->hdr.v1.creationTime		= creationTime;
		obj->hdr.v1.modificationTime	= modificationTime;
		obj->hdr.v1.timescale			= timescale;
	}
	if(obj->prntFullBox.version == 1){
		obj->hdr.v1.duration	= duration;
	} else {
		obj->hdr.v0.duration	= (UI32)duration;
	}
}

BOOL NBMp4MediaHeaderBox_setLanguage(STNBMp4MediaHeaderBox* obj, const char* lang3){
	BOOL r = FALSE;
	if(lang3 != NULL && lang3[0] != '\0' && lang3[1] != '\0' && lang3[2] != '\0'){
		obj->language[0] = lang3[0];
		obj->language[1] = lang3[1];
		obj->language[2] = lang3[2];
		r = TRUE;
	}
	return r;
}


NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(MediaHeaderBox)

//NBMp4TrackHeaderBox

/*
aligned(8) class TrackHeaderBox extends FullBox(‘tkhd’, version, flags){
   if (version==1) {
	  unsigned int(64)  creation_time;
	  unsigned int(64)  modification_time;
	  unsigned int(32)  track_ID;
	  const unsigned int(32)  reserved = 0;
	  unsigned int(64)  duration;
   } else { // version==0
	  unsigned int(32)  creation_time;
	  unsigned int(32)  modification_time;
	  unsigned int(32)  track_ID;
	  const unsigned int(32)  reserved = 0;
	  unsigned int(32)  duration;
   }
   const unsigned int(32)[2]  reserved = 0;
   template int(16) layer = 0;
   template int(16) alternate_group = 0;
   template int(16)  volume = {if track_is_audio 0x0100 else 0};
   const unsigned int(16)  reserved = 0;
   template int(32)[9]  matrix=
	  { 0x00010000,0,0,0,0x00010000,0,0,0,0x40000000 };
	  // unity matrix
   unsigned int(32) width;	//fixed 16.16
   unsigned int(32) height;	//fixed 16.16
}*/

#define NBMp4TrackHeaderBoxFlag_Track_enabled		0x000001
#define NBMp4TrackHeaderBoxFlag_Track_in_movie		0x000002
#define NBMp4TrackHeaderBoxFlag_Track_in_preview	0x000004

void NBMp4TrackHeaderBox_init(STNBMp4TrackHeaderBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4TrackHeaderBox);
	NBMp4FullBox_init(&obj->prntFullBox);
	//
	obj->prntFullBox.prntBox.hdr.type.str[0] = 't';
	obj->prntFullBox.prntBox.hdr.type.str[1] = 'k';
	obj->prntFullBox.prntBox.hdr.type.str[2] = 'h';
	obj->prntFullBox.prntBox.hdr.type.str[3] = 'd';
	//cur-time
	{
		const UI64 t1901 = NBMP4_SECS_FROM_1901_TO_UTC1970 + NBDatetime_getCurUTCTimestamp();
		if(t1901 > 0xFFFFFFFFU){
			obj->prntFullBox.version = 1;
			obj->hdr.v1.creationTime = obj->hdr.v1.modificationTime = t1901;
		} else {
			obj->prntFullBox.version = 0;
			obj->hdr.v0.creationTime = obj->hdr.v0.modificationTime = (UI32)t1901;
		}
	}
	//
	obj->volume = 0x0100;
	// Unity matrix
	obj->matrix[0]	= 0x00010000;	obj->matrix[1]	= 0;			obj->matrix[2]	= 0;
	obj->matrix[3]	= 0;			obj->matrix[4]	= 0x00010000;	obj->matrix[5]	= 0;
	obj->matrix[6]	= 0;			obj->matrix[7]	= 0;			obj->matrix[8]	= 0x40000000;
}

void NBMp4TrackHeaderBox_release(STNBMp4TrackHeaderBox* obj){
	NBMp4FullBox_release(&obj->prntFullBox);
}

BOOL NBMp4TrackHeaderBox_loadFromBuff(STNBMp4TrackHeaderBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	if(!NBMp4FullBox_loadBits(&obj->prntFullBox, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd)){
		return FALSE;
	}
	if(obj->prntFullBox.version == 1){
		if(!NBMp4_readUI64(&obj->hdr.v1.creationTime, buff, buffAfterEnd)){
			return FALSE;
		}
		if(!NBMp4_readUI64(&obj->hdr.v1.modificationTime, buff, buffAfterEnd)){
			return FALSE;
		}
		if(!NBMp4_readUI32(&obj->hdr.v1.trackId, buff, buffAfterEnd)){
			return FALSE;
		}
		if(!NBMp4_readUI32(&obj->hdr.v1.reserved, buff, buffAfterEnd)){
			return FALSE;
		}
		if(!NBMp4_readUI64(&obj->hdr.v1.duration, buff, buffAfterEnd)){
			return FALSE;
		}
	} else if(obj->prntFullBox.version == 0){
		if(!NBMp4_readUI32(&obj->hdr.v0.creationTime, buff, buffAfterEnd)){
			return FALSE;
		}
		if(!NBMp4_readUI32(&obj->hdr.v0.modificationTime, buff, buffAfterEnd)){
			return FALSE;
		}
		if(!NBMp4_readUI32(&obj->hdr.v0.trackId, buff, buffAfterEnd)){
			return FALSE;
		}
		if(!NBMp4_readUI32(&obj->hdr.v0.reserved, buff, buffAfterEnd)){
			return FALSE;
		}
		if(!NBMp4_readUI32(&obj->hdr.v0.duration, buff, buffAfterEnd)){
			return FALSE;
		}
	} else {
		return FALSE;
	}
	if(!NBMp4_readUI32s(&obj->reserved2[0], 2, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readSI16(&obj->layer, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readSI16(&obj->alternateGroup, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readSI16(&obj->volume, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI16(&obj->reserved, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readSI32s(&obj->matrix[0], 9, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->width, buff, buffAfterEnd)){ //fixed 16.16
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->height, buff, buffAfterEnd)){ //fixed 16.16
		return FALSE;
	}
	return (*buff == buffAfterEnd);
}

void NBMp4TrackHeaderBox_concat(const STNBMp4TrackHeaderBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4FullBox_concat(&obj->prntFullBox, depthLvl, optSrcFile, dst);
		//
		if(obj->prntFullBox.version == 1){
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "creation("); NBMp4_concatTimestamp(dst, obj->hdr.v1.creationTime); NBString_concat(dst, ")");
			NBString_concat(dst, " mod("); NBMp4_concatTimestamp(dst, obj->hdr.v1.modificationTime); NBString_concat(dst, ")");
			NBString_concat(dst, " trackId("); NBString_concatUI32(dst, obj->hdr.v1.trackId); NBString_concat(dst, ")");
			//UI32	reserved;
			NBString_concat(dst, " duration("); NBString_concatUI64(dst, obj->hdr.v1.duration); NBString_concat(dst, ")");
			NBString_concatByte(dst, '\n');
		} else {
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "creation("); NBMp4_concatTimestamp(dst, obj->hdr.v0.creationTime); NBString_concat(dst, ")");
			NBString_concat(dst, " mod("); NBMp4_concatTimestamp(dst, obj->hdr.v0.modificationTime); NBString_concat(dst, ")");
			NBString_concat(dst, " trackId("); NBString_concatUI32(dst, obj->hdr.v0.trackId); NBString_concat(dst, ")");
			//UI32	reserved;
			NBString_concat(dst, " duration("); NBString_concatUI32(dst, obj->hdr.v0.duration); NBString_concat(dst, ")");
			NBString_concatByte(dst, '\n');
		}
		{
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			//UI32	reserved2[2];
			NBString_concat(dst, "layer("); NBString_concatSI32(dst, obj->layer); NBString_concat(dst, ")");
			NBString_concat(dst, " alternateGrp("); NBString_concatSI32(dst, obj->alternateGroup); NBString_concat(dst, ")");
			NBString_concat(dst, " volume("); NBMp4_concatFixed8x8(dst, obj->volume); NBString_concat(dst, ")");
			//SI16	reserved;		//0
			NBString_concat(dst, " matrix("); NBMp4_concatMatrix9(dst, obj->matrix); NBString_concat(dst, ")");
			NBString_concat(dst, " width("); NBMp4_concatFixed16x16(dst, obj->width); NBString_concat(dst, ")");
			NBString_concat(dst, " height("); NBMp4_concatFixed16x16(dst, obj->height); NBString_concat(dst, ")");
			NBString_concatByte(dst, '\n');
		}
	}
}

BOOL NBMp4TrackHeaderBox_writeBits(STNBMp4TrackHeaderBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4FullBox_writeHdrBits(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(obj->prntFullBox.version == 1){
		if(!NBMp4_writeUI64(&obj->hdr.v1.creationTime, dst)){
			return FALSE;
		}
		if(!NBMp4_writeUI64(&obj->hdr.v1.modificationTime, dst)){
			return FALSE;
		}
		if(!NBMp4_writeUI32(&obj->hdr.v1.trackId, dst)){
			return FALSE;
		}
		if(!NBMp4_writeUI32(&obj->hdr.v1.reserved, dst)){
			return FALSE;
		}
		if(!NBMp4_writeUI64(&obj->hdr.v1.duration, dst)){
			return FALSE;
		}
	} else if(obj->prntFullBox.version == 0){
		if(!NBMp4_writeUI32(&obj->hdr.v0.creationTime, dst)){
			return FALSE;
		}
		if(!NBMp4_writeUI32(&obj->hdr.v0.modificationTime, dst)){
			return FALSE;
		}
		if(!NBMp4_writeUI32(&obj->hdr.v0.trackId, dst)){
			return FALSE;
		}
		if(!NBMp4_writeUI32(&obj->hdr.v0.reserved, dst)){
			return FALSE;
		}
		if(!NBMp4_writeUI32(&obj->hdr.v0.duration, dst)){
			return FALSE;
		}
	} else {
		return FALSE;
	}
	if(!NBMp4_writeUI32s(&obj->reserved2[0], 2, dst)){
		return FALSE;
	}
	if(!NBMp4_writeSI16(&obj->layer, dst)){
		return FALSE;
	}
	if(!NBMp4_writeSI16(&obj->alternateGroup, dst)){
		return FALSE;
	}
	if(!NBMp4_writeSI16(&obj->volume, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI16(&obj->reserved, dst)){
		return FALSE;
	}
	if(!NBMp4_writeSI32s(&obj->matrix[0], 9, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->width, dst)){ //fixed 16.16
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->height, dst)){ //fixed 16.16
		return FALSE;
	}
	if(!NBMp4FullBox_writeHdrFinalSize(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

void NBMp4TrackHeaderBox_setIsEnabled(STNBMp4TrackHeaderBox* obj, const BOOL isEnabled){ //that the track is enabled
	if(isEnabled){
		obj->prntFullBox.flags |= NBMp4TrackHeaderBoxFlag_Track_enabled;
	} else {
		obj->prntFullBox.flags &= ~NBMp4TrackHeaderBoxFlag_Track_enabled;
	}
}

void NBMp4TrackHeaderBox_setIsInMovie(STNBMp4TrackHeaderBox* obj, const BOOL isInMovie){ //the track is used in the presentation.
	if(isInMovie){
		obj->prntFullBox.flags |= NBMp4TrackHeaderBoxFlag_Track_in_movie;
	} else {
		obj->prntFullBox.flags &= ~NBMp4TrackHeaderBoxFlag_Track_in_movie;
	}
}

void NBMp4TrackHeaderBox_setIsInPreview(STNBMp4TrackHeaderBox* obj, const BOOL isInPreview){ //the track is used when previewing the presentation.
	if(isInPreview){
		obj->prntFullBox.flags |= NBMp4TrackHeaderBoxFlag_Track_in_preview;
	} else {
		obj->prntFullBox.flags &= ~NBMp4TrackHeaderBoxFlag_Track_in_preview;
	}
}

void NBMp4TrackHeaderBox_setTrackId(STNBMp4TrackHeaderBox* obj, const UI32 trackId){
	if(obj->prntFullBox.version == 1){
		obj->hdr.v1.trackId	= trackId;
	} else {
		obj->hdr.v0.trackId	= trackId;
	}
}

void NBMp4TrackHeaderBox_setDuration(STNBMp4TrackHeaderBox* obj, const UI64 duration){
	if(obj->prntFullBox.version == 0 && duration > 0xFFFFFFFFU){
		//change to version 1
		const UI64 creationTime			= obj->hdr.v0.creationTime;
		const UI64 modificationTime		= obj->hdr.v0.modificationTime;
		const UI32 trackId				= obj->hdr.v0.trackId;
		obj->prntFullBox.version		= 1;
		obj->hdr.v1.creationTime		= creationTime;
		obj->hdr.v1.modificationTime	= modificationTime;
		obj->hdr.v1.trackId				= trackId;
		obj->hdr.v1.reserved			= 0;
	}
	if(obj->prntFullBox.version == 1){
		obj->hdr.v1.duration	= duration;
	} else {
		obj->hdr.v0.duration	= (UI32)duration;
	}
}

void NBMp4TrackHeaderBox_setLayer(STNBMp4TrackHeaderBox* obj, const SI16 layer){
	obj->layer = layer;
}

void NBMp4TrackHeaderBox_setAlternateGroup(STNBMp4TrackHeaderBox* obj, const SI16 alternateGroup){
	obj->alternateGroup = alternateGroup;
}

void NBMp4TrackHeaderBox_setWidth(STNBMp4TrackHeaderBox* obj, const UI16 width){
	obj->width = ((UI16)width << 16);
}

void NBMp4TrackHeaderBox_setHeight(STNBMp4TrackHeaderBox* obj, const UI16 height){
	obj->height = ((UI16)height << 16);
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(TrackHeaderBox)

//NBMp4HandlerBox

/*aligned(8) class HandlerBox extends FullBox(‘hdlr’, version = 0, 0) {
   unsigned int(32)  pre_defined = 0;
   unsigned int(32)  handler_type;
   const unsigned int(32)[3]  reserved = 0;
   string   name;
}*/

void NBMp4HandlerBox_init(STNBMp4HandlerBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4HandlerBox);
	NBMp4FullBox_init(&obj->prntFullBox);
	//
	obj->prntFullBox.prntBox.hdr.type.str[0] = 'h';
	obj->prntFullBox.prntBox.hdr.type.str[1] = 'd';
	obj->prntFullBox.prntBox.hdr.type.str[2] = 'l';
	obj->prntFullBox.prntBox.hdr.type.str[3] = 'r';
}

void NBMp4HandlerBox_release(STNBMp4HandlerBox* obj){
	if(obj->name != NULL){
		NBMemory_free(obj->name);
		obj->name = NULL;
	}
	NBMp4FullBox_release(&obj->prntFullBox);
}

BOOL NBMp4HandlerBox_loadFromBuff(STNBMp4HandlerBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	if(!NBMp4FullBox_loadBits(&obj->prntFullBox, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->preDefined, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readBytes(&obj->handlerType, sizeof(obj->handlerType), buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI32s(&obj->reserved[0], 3, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readString(&obj->name, buff, buffAfterEnd)){
		return FALSE;
	}
	return (*buff == buffAfterEnd);
}

void NBMp4HandlerBox_concat(const STNBMp4HandlerBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4FullBox_concat(&obj->prntFullBox, depthLvl, optSrcFile, dst);
		//
		{
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			//UI32			preDefined;
			NBString_concat(dst, "handlerType('"); NBMp4_concatChar(dst, obj->handlerType.str[0]); NBMp4_concatChar(dst, obj->handlerType.str[1]); NBMp4_concatChar(dst, obj->handlerType.str[2]); NBMp4_concatChar(dst, obj->handlerType.str[3]); NBString_concat(dst, "')");
			NBString_concat(dst, " name('"); NBString_concat(dst, obj->name); NBString_concat(dst, "')");
			NBString_concatByte(dst, '\n');
		}
	}
}

BOOL NBMp4HandlerBox_writeBits(STNBMp4HandlerBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4FullBox_writeHdrBits(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->preDefined, dst)){
		return FALSE;
	}
	if(!NBMp4_writeBytes(&obj->handlerType, sizeof(obj->handlerType), dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32s(&obj->reserved[0], 3, dst)){
		return FALSE;
	}
	if(!NBMp4_writeString(obj->name, dst)){
		return FALSE;
	}
	if(!NBMp4FullBox_writeHdrFinalSize(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

BOOL NBMp4HandlerBox_setHandlerType(STNBMp4HandlerBox* obj, const char* type4, const char* name){
	BOOL r = FALSE;
	if(type4 != NULL && type4[0] != '\0' && type4[1] != '\0' && type4[2] != '\0' && type4[3] != '\0'){
		obj->handlerType.str[0] = type4[0];
		obj->handlerType.str[1] = type4[1];
		obj->handlerType.str[2] = type4[2];
		obj->handlerType.str[3] = type4[3];
		if(obj->name != NULL){
			NBMemory_free(obj->name);
			obj->name = NULL;
		}
		if(name != NULL){
			obj->name = NBString_strNewBuffer(name);
		}
	}
	return r;
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(HandlerBox)

//NBMp4DataEntryUrlBox

/*
aligned(8) class DataEntryUrlBox (bit(24) flags) extends FullBox(‘url ’, version = 0, flags) {
   string   location;
}
*/

void NBMp4DataEntryUrlBox_init(STNBMp4DataEntryUrlBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4DataEntryUrlBox);
	NBMp4FullBox_init(&obj->prntFullBox);
	//
	obj->prntFullBox.prntBox.hdr.type.str[0] = 'u';
	obj->prntFullBox.prntBox.hdr.type.str[1] = 'r';
	obj->prntFullBox.prntBox.hdr.type.str[2] = 'l';
	obj->prntFullBox.prntBox.hdr.type.str[3] = ' ';
	//
	obj->prntFullBox.flags |= 0x1;	//currently empty location
}

void NBMp4DataEntryUrlBox_release(STNBMp4DataEntryUrlBox* obj){
	if(obj->location != NULL){
		NBMemory_free(obj->location);
		obj->location = NULL;
	}
	NBMp4FullBox_release(&obj->prntFullBox);
}

BOOL NBMp4DataEntryUrlBox_loadFromBuff(STNBMp4DataEntryUrlBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	if(!NBMp4FullBox_loadBits(&obj->prntFullBox, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd)){
		return FALSE;
	} if(obj->prntFullBox.version != 0){
		return FALSE;
	}
	if((obj->prntFullBox.flags & 0x1)){
		if(obj->location != NULL){
			NBMemory_free(obj->location);
			obj->location = NULL;
		}
	} else if(!NBMp4_readString(&obj->location, buff, buffAfterEnd)){
		return FALSE;
	}
	return (*buff == buffAfterEnd);
}

void NBMp4DataEntryUrlBox_concat(const STNBMp4DataEntryUrlBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4FullBox_concat(&obj->prntFullBox, depthLvl, optSrcFile, dst);
		//
		{
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "location('"); NBString_concat(dst, obj->location); NBString_concat(dst, "')");
			NBString_concatByte(dst, '\n');
		}
	}
}

BOOL NBMp4DataEntryUrlBox_writeBits(STNBMp4DataEntryUrlBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4FullBox_writeHdrBits(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if((obj->prntFullBox.flags & 0x1)){
		//
	} else if(!NBMp4_writeString(obj->location, dst)){
		return FALSE;
	}
	if(!NBMp4FullBox_writeHdrFinalSize(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

void NBMp4DataEntryUrlBox_setLocation(STNBMp4DataEntryUrlBox* obj, const char* location){
	if(obj->location != NULL){
		NBMemory_free(obj->location);
		obj->location = NULL;
	}
	if(location == NULL || location[0] == '\0'){
		obj->prntFullBox.flags |= 0x1;	//empty location
	} else {
		obj->prntFullBox.flags &= ~0x1; //non-empty location
		obj->location = NBString_strNewBuffer(location);
	}
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(DataEntryUrlBox)

//NBMp4DataEntryUrnBox

/*aligned(8) class DataEntryUrnBox (bit(24) flags) extends FullBox(‘urn ’, version = 0, flags) {
   string   name;
   string   location;
}*/

void NBMp4DataEntryUrnBox_init(STNBMp4DataEntryUrnBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4DataEntryUrnBox);
	NBMp4FullBox_init(&obj->prntFullBox);
	//
	obj->prntFullBox.prntBox.hdr.type.str[0] = 'u';
	obj->prntFullBox.prntBox.hdr.type.str[1] = 'r';
	obj->prntFullBox.prntBox.hdr.type.str[2] = 'n';
	obj->prntFullBox.prntBox.hdr.type.str[3] = ' ';
}

void NBMp4DataEntryUrnBox_release(STNBMp4DataEntryUrnBox* obj){
	if(obj->name != NULL){
		NBMemory_free(obj->name);
		obj->name = NULL;
	}
	if(obj->location != NULL){
		NBMemory_free(obj->location);
		obj->location = NULL;
	}
	NBMp4FullBox_release(&obj->prntFullBox);
}

BOOL NBMp4DataEntryUrnBox_loadFromBuff(STNBMp4DataEntryUrnBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	if(!NBMp4FullBox_loadBits(&obj->prntFullBox, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd)){
		return FALSE;
	} if(obj->prntFullBox.version != 0){
		return FALSE;
	}
	if(!NBMp4_readString(&obj->name, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readString(&obj->location, buff, buffAfterEnd)){
		return FALSE;
	}
	return (*buff == buffAfterEnd);
}

void NBMp4DataEntryUrnBox_concat(const STNBMp4DataEntryUrnBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4FullBox_concat(&obj->prntFullBox, depthLvl, optSrcFile, dst);
		//
		{
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "name('"); NBString_concat(dst, obj->name); NBString_concat(dst, "')");
			NBString_concat(dst, " location('"); NBString_concat(dst, obj->location); NBString_concat(dst, "')");
			NBString_concatByte(dst, '\n');
		}
	}
}

BOOL NBMp4DataEntryUrnBox_writeBits(STNBMp4DataEntryUrnBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4FullBox_writeHdrBits(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(!NBMp4_writeString(obj->name, dst)){
		return FALSE;
	}
	if(!NBMp4_writeString(obj->location, dst)){
		return FALSE;
	}
	if(!NBMp4FullBox_writeHdrFinalSize(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

void NBMp4DataEntryUrnBox_setLocation(STNBMp4DataEntryUrnBox* obj, const char* name, const char* location){
	if(obj->name != NULL){
		NBMemory_free(obj->name);
		obj->name = NULL;
	}
	if(obj->location != NULL){
		NBMemory_free(obj->location);
		obj->location = NULL;
	}
	if(name != NULL){
		obj->name = NBString_strNewBuffer(name);
	}
	if(location != NULL){
		obj->location = NBString_strNewBuffer(location);
	}
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(DataEntryUrnBox)

//NBMp4DataEntryBox

void NBMp4DataEntryBox_init(STNBMp4DataEntryBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4DataEntryBox);
	//obj->entry.url.prntFullBox.prntBox.hdr.type.str[0] = 'u';	//redundant
	//obj->entry.url.prntFullBox.prntBox.hdr.type.str[1] = 'r';	//redundant
	//obj->entry.url.prntFullBox.prntBox.hdr.type.str[2] = 'l';	//redundant
	//obj->entry.url.prntFullBox.prntBox.hdr.type.str[3] = ' ';	//redundant
	NBMp4DataEntryUrlBox_init(&obj->entry.url);
}

void NBMp4DataEntryBox_release(STNBMp4DataEntryBox* obj){
	if(
	   obj->entry.url.prntFullBox.prntBox.hdr.type.str[0] == 'u'
	   && obj->entry.url.prntFullBox.prntBox.hdr.type.str[1] == 'r'
	   && obj->entry.url.prntFullBox.prntBox.hdr.type.str[2] == 'l'
	   && obj->entry.url.prntFullBox.prntBox.hdr.type.str[3] == ' '
	   )
	{
		NBMp4DataEntryUrlBox_release(&obj->entry.url);
	} else if(
			  obj->entry.url.prntFullBox.prntBox.hdr.type.str[0] == 'u'
		&& obj->entry.url.prntFullBox.prntBox.hdr.type.str[1] == 'r'
		&& obj->entry.url.prntFullBox.prntBox.hdr.type.str[2] == 'n'
		&& obj->entry.url.prntFullBox.prntBox.hdr.type.str[3] == ' '
		)
	 {
		 NBMp4DataEntryUrnBox_release(&obj->entry.urn);
	 }
}

BOOL NBMp4DataEntryBox_loadFromBuff(STNBMp4DataEntryBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	if(
	   hdr->type.str[0] == 'u'
	   && hdr->type.str[1] == 'r'
	   && hdr->type.str[2] == 'l'
	   && hdr->type.str[3] == ' '
	   )
	{
		return NBMp4DataEntryUrlBox_loadFromBuff(&obj->entry.url, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd);
	} else if(
		hdr->type.str[0] == 'u'
		&& hdr->type.str[1] == 'r'
		&& hdr->type.str[2] == 'n'
		&& hdr->type.str[3] == ' '
		)
	 {
		 return NBMp4DataEntryUrnBox_loadFromBuff(&obj->entry.urn, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd);
	 }
	return FALSE;
}

void NBMp4DataEntryBox_concat(const STNBMp4DataEntryBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		if(
		   obj->entry.url.prntFullBox.prntBox.hdr.type.str[0] == 'u'
		   && obj->entry.url.prntFullBox.prntBox.hdr.type.str[1] == 'r'
		   && obj->entry.url.prntFullBox.prntBox.hdr.type.str[2] == 'l'
		   && obj->entry.url.prntFullBox.prntBox.hdr.type.str[3] == ' '
		   )
		{
			NBMp4DataEntryUrlBox_concat(&obj->entry.url, depthLvl, optSrcFile, dst);
		} else if(
				  obj->entry.url.prntFullBox.prntBox.hdr.type.str[0] == 'u'
			&& obj->entry.url.prntFullBox.prntBox.hdr.type.str[1] == 'r'
			&& obj->entry.url.prntFullBox.prntBox.hdr.type.str[2] == 'n'
			&& obj->entry.url.prntFullBox.prntBox.hdr.type.str[3] == ' '
			)
		 {
			 NBMp4DataEntryUrnBox_concat(&obj->entry.urn, depthLvl, optSrcFile, dst);
		 }
	}
}

BOOL NBMp4DataEntryBox_writeBits(STNBMp4DataEntryBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(
	   obj->entry.url.prntFullBox.prntBox.hdr.type.str[0] == 'u'
	   && obj->entry.url.prntFullBox.prntBox.hdr.type.str[1] == 'r'
	   && obj->entry.url.prntFullBox.prntBox.hdr.type.str[2] == 'l'
	   && obj->entry.url.prntFullBox.prntBox.hdr.type.str[3] == ' '
	   )
	{
		return NBMp4DataEntryUrlBox_writeBits(&obj->entry.url, depthLvl, optSrcFile, dst);
	} else if(
			  obj->entry.url.prntFullBox.prntBox.hdr.type.str[0] == 'u'
		&& obj->entry.url.prntFullBox.prntBox.hdr.type.str[1] == 'r'
		&& obj->entry.url.prntFullBox.prntBox.hdr.type.str[2] == 'n'
		&& obj->entry.url.prntFullBox.prntBox.hdr.type.str[3] == ' '
		)
	 {
		 return NBMp4DataEntryUrnBox_writeBits(&obj->entry.urn, depthLvl, optSrcFile, dst);
	 }
	return FALSE;
}

void NBMp4DataEntryBox_setEntry(STNBMp4DataEntryBox* obj, const char* name, const char* location){
	if(
	   obj->entry.url.prntFullBox.prntBox.hdr.type.str[0] == 'u'
	   && obj->entry.url.prntFullBox.prntBox.hdr.type.str[1] == 'r'
	   && obj->entry.url.prntFullBox.prntBox.hdr.type.str[2] == 'l'
	   && obj->entry.url.prntFullBox.prntBox.hdr.type.str[3] == ' '
	   )
	{
		NBMp4DataEntryUrlBox_release(&obj->entry.url);
	} else if(
			  obj->entry.url.prntFullBox.prntBox.hdr.type.str[0] == 'u'
		&& obj->entry.url.prntFullBox.prntBox.hdr.type.str[1] == 'r'
		&& obj->entry.url.prntFullBox.prntBox.hdr.type.str[2] == 'n'
		&& obj->entry.url.prntFullBox.prntBox.hdr.type.str[3] == ' '
		)
	 {
		 NBMp4DataEntryUrnBox_release(&obj->entry.urn);
	 }
	if(name == NULL || name[0] == '\0'){
		//url
		obj->entry.url.prntFullBox.prntBox.hdr.type.str[0] = 'u';
		obj->entry.url.prntFullBox.prntBox.hdr.type.str[1] = 'r';
		obj->entry.url.prntFullBox.prntBox.hdr.type.str[2] = 'l';
		obj->entry.url.prntFullBox.prntBox.hdr.type.str[3] = ' ';
		NBMp4DataEntryUrlBox_init(&obj->entry.url);
		NBMp4DataEntryUrlBox_setLocation(&obj->entry.url, location);
	} else {
		//urn
		obj->entry.url.prntFullBox.prntBox.hdr.type.str[0] = 'u';
		obj->entry.url.prntFullBox.prntBox.hdr.type.str[1] = 'r';
		obj->entry.url.prntFullBox.prntBox.hdr.type.str[2] = 'n';
		obj->entry.url.prntFullBox.prntBox.hdr.type.str[3] = ' ';
		NBMp4DataEntryUrnBox_init(&obj->entry.urn);
		NBMp4DataEntryUrnBox_setLocation(&obj->entry.urn, name, location);
	}
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(DataEntryBox)

//NBMp4DataReferenceBox

/*
aligned(8) class DataReferenceBox extends FullBox(‘dref’, version = 0, 0) {
   unsigned int(32)  entry_count;
   for (i=1; i <= entry_count; i++) {
	   DataEntryBox(entry_version, entry_flags) data_entry;
   }
}*/

void NBMp4DataReferenceBox_init(STNBMp4DataReferenceBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4DataReferenceBox);
	NBMp4FullBox_init(&obj->prntFullBox);
	//
	obj->prntFullBox.prntBox.hdr.type.str[0] = 'd';
	obj->prntFullBox.prntBox.hdr.type.str[1] = 'r';
	obj->prntFullBox.prntBox.hdr.type.str[2] = 'e';
	obj->prntFullBox.prntBox.hdr.type.str[3] = 'f';
}

void NBMp4DataReferenceBox_release(STNBMp4DataReferenceBox* obj){
	if(obj->dataEntries != NULL){
		UI32 i; for(i = 0; i < obj->entryCount; i++){
			STNBMp4DataEntryBox* e = &obj->dataEntries[i];
			NBMp4DataEntryBox_release(e);
		}
		NBMemory_free(obj->dataEntries);
		obj->dataEntries = NULL;
		obj->entryCount = 0;
	}
	NBMp4FullBox_release(&obj->prntFullBox);
}

BOOL NBMp4DataReferenceBox_loadFromBuff(STNBMp4DataReferenceBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	const BYTE* buffStart = *buff;
	if(!NBMp4FullBox_loadBits(&obj->prntFullBox, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd)){
		return FALSE;
	} if(obj->prntFullBox.version != 0 || obj->prntFullBox.flags != 0){
		return FALSE;
	}
	//release entries
	if(obj->dataEntries != NULL){
		UI32 i; for(i = 0; i < obj->entryCount; i++){
			STNBMp4DataEntryBox* e = &obj->dataEntries[i];
			NBMp4DataEntryBox_release(e);
		}
		NBMemory_free(obj->dataEntries);
		obj->dataEntries = NULL;
		obj->entryCount = 0;
	}
	if(!NBMp4_readUI32(&obj->entryCount, buff, buffAfterEnd)){
		return FALSE;
	} else if(obj->entryCount < 1){ //must be at least one
		obj->entryCount = 0;
		return FALSE;
	}
	//entries
	{
		UI32 i; BOOL errFnd = FALSE;
		obj->dataEntries = NBMemory_allocTypes(STNBMp4DataEntryBox, obj->entryCount);
		for(i = 0; i < obj->entryCount && !errFnd; i++){
			const BYTE* boxStart = *buff;
			STNBMp4DataEntryBox* e = &obj->dataEntries[i];
			STNBMp4BoxHdr hdr2;
			NBMp4BoxHdr_init(&hdr2);
			errFnd = TRUE;
			if(NBMp4BoxHdr_loadBits(&hdr2, buff, buffAfterEnd)){
				const BYTE* payStart = *buff;
				NBMp4DataEntryBox_init(e);
				if(!NBMp4DataEntryBox_loadFromBuff(e, &obj->prntFullBox.prntBox, depthLvl + 1, boxFileStart + hdrFileSz + (boxStart - buffStart), (UI8)(payStart - boxStart), &hdr2, buff, *buff + hdr2.size - 8)){
					NBMp4DataEntryBox_release(e);
				} else {
					errFnd = FALSE;
				}
			}
			NBMp4BoxHdr_release(&hdr2);
		}
		obj->entryCount = i;
		if(errFnd){
			return FALSE;
		}
	}
	return (*buff == buffAfterEnd);
}

void NBMp4DataReferenceBox_concat(const STNBMp4DataReferenceBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4FullBox_concat(&obj->prntFullBox, depthLvl, optSrcFile, dst);
		//
		{
			UI32 i; for(i = 0; i < obj->entryCount; i++){
				STNBMp4DataEntryBox* e = &obj->dataEntries[i];
				NBMp4DataEntryBox_concat(e, depthLvl + 1, optSrcFile, dst);
			}
		}
	}
}

BOOL NBMp4DataReferenceBox_writeBits(STNBMp4DataReferenceBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4FullBox_writeHdrBits(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->entryCount, dst)){
		return FALSE;
	}
	//entries
	{
		if(obj->dataEntries != NULL){
			UI32 i; for(i = 0; i < obj->entryCount; i++){
				STNBMp4DataEntryBox* e = &obj->dataEntries[i];
				if(!NBMp4DataEntryBox_writeBits(e, depthLvl + 1, optSrcFile, dst)){
					return FALSE;
				}
			}
		}
	}
	if(!NBMp4FullBox_writeHdrFinalSize(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

void NBMp4DataReferenceBox_addEntry(STNBMp4DataReferenceBox* obj, const char* name, const char* location){
	STNBMp4DataEntryBox* nArr = NBMemory_allocTypes(STNBMp4DataEntryBox, obj->entryCount + 1);
	if(obj->dataEntries != NULL){
		if(obj->entryCount > 0){
			NBMemory_copy(nArr, obj->dataEntries, sizeof(obj->dataEntries[0]) * obj->entryCount);
		}
		NBMemory_free(obj->dataEntries);
	}
	obj->dataEntries = nArr;
	{
		STNBMp4DataEntryBox* e = &obj->dataEntries[obj->entryCount++];
		NBMp4DataEntryBox_init(e);
		NBMp4DataEntryBox_setEntry(e, name, location);
	}
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(DataReferenceBox)

//NBMp4SampleSizeBox

/*
 aligned(8) class SampleSizeBox extends FullBox(‘stsz’, version = 0, 0) {
	unsigned int(32)  sample_size;
	unsigned int(32)  sample_count;
	if (sample_size==0) {
	   for (i=1; i <= sample_count; i++) {
	       unsigned int(32)  entry_size;
	   }
 }
}
*/

void NBMp4SampleSizeBox_init(STNBMp4SampleSizeBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4SampleSizeBox);
	NBMp4FullBox_init(&obj->prntFullBox);
	//
	obj->prntFullBox.prntBox.hdr.type.str[0] = 's';
	obj->prntFullBox.prntBox.hdr.type.str[1] = 't';
	obj->prntFullBox.prntBox.hdr.type.str[2] = 's';
	obj->prntFullBox.prntBox.hdr.type.str[3] = 'z';
}

void NBMp4SampleSizeBox_release(STNBMp4SampleSizeBox* obj){
	if(obj->entrySize != NULL){
		NBMemory_free(obj->entrySize);
		obj->entrySize = NULL;
		obj->entrySizeSz = 0;
	}
	NBMp4FullBox_release(&obj->prntFullBox);
}

BOOL NBMp4SampleSizeBox_loadFromBuff(STNBMp4SampleSizeBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	if(!NBMp4FullBox_loadBits(&obj->prntFullBox, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd)){
		return FALSE;
	} if(obj->prntFullBox.version != 0 || obj->prntFullBox.flags != 0){
		return FALSE;
	}
	//release entries
	if(obj->entrySize != NULL){
		NBMemory_free(obj->entrySize);
		obj->entrySize = NULL;
		obj->entrySizeSz = 0;
	}
	if(!NBMp4_readUI32(&obj->sampleSize, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->sampleCount, buff, buffAfterEnd)){
		return FALSE;
	}
	//entries
	if(obj->sampleSize == 0){
		UI32 i; BOOL errFnd = FALSE;
		obj->entrySizeSz	= 0;
		obj->entrySize		= NBMemory_allocTypes(UI32, obj->sampleCount);
		for(i = 0; i < obj->entrySizeSz && !errFnd; i++){
			if(!NBMp4_readUI32(&obj->entrySize[obj->entrySizeSz], buff, buffAfterEnd)){
				errFnd = TRUE;
				break;
			}
			obj->entrySizeSz++;
		}
		if(errFnd){
			return FALSE;
		}
	}
	return (*buff == buffAfterEnd);
}

void NBMp4SampleSizeBox_concat(const STNBMp4SampleSizeBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4FullBox_concat(&obj->prntFullBox, depthLvl, optSrcFile, dst);
		//
		{
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "sampleSize('"); NBString_concatUI32(dst, obj->sampleSize); NBString_concat(dst, "')");
			NBString_concat(dst, " sampleCount('"); NBString_concatUI32(dst, obj->sampleCount); NBString_concat(dst, "')");
			NBString_concatByte(dst, '\n');
		}
		//
		if(obj->entrySize != NULL && obj->entrySizeSz > 0){
			UI32 i;
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "entrySize: ");
			for(i = 0; i < obj->entrySizeSz; i++){
				const UI32* e = &obj->entrySize[i];
				NBString_concatUI32(dst, *e);
				if((i + 1) % 8){
					NBString_concatByte(dst, '\n');
					NBString_concat(dst, "           ");
				}
			}
			if(i % 8){
				NBString_concatByte(dst, '\n');
			}
		}
	}
}

BOOL NBMp4SampleSizeBox_writeBits(STNBMp4SampleSizeBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4FullBox_writeHdrBits(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->sampleSize, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->sampleCount, dst)){
		return FALSE;
	}
	//entries
	if(obj->sampleSize == 0 && obj->entrySize != NULL && obj->entrySizeSz > 0){
		if(!NBMp4_writeUI32s(obj->entrySize, obj->entrySizeSz, dst)){
			return FALSE;
		}
	}
	if(!NBMp4FullBox_writeHdrFinalSize(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(SampleSizeBox)

//NBMp4VideoMediaHeaderBox

/*aligned(8) class VideoMediaHeaderBox extends FullBox(‘vmhd’, version = 0, 1) {
   template unsigned int(16)  graphicsmode = 0;   // copy, see below
   template unsigned int(16)[3]  opColor = {0, 0, 0};
}*/

void NBMp4VideoMediaHeaderBox_init(STNBMp4VideoMediaHeaderBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4VideoMediaHeaderBox);
	NBMp4FullBox_init(&obj->prntFullBox);
	//
	obj->prntFullBox.prntBox.hdr.type.str[0] = 'v';
	obj->prntFullBox.prntBox.hdr.type.str[1] = 'm';
	obj->prntFullBox.prntBox.hdr.type.str[2] = 'h';
	obj->prntFullBox.prntBox.hdr.type.str[3] = 'd';
	//
	obj->prntFullBox.flags = 0x1;
}

void NBMp4VideoMediaHeaderBox_release(STNBMp4VideoMediaHeaderBox* obj){
	NBMp4FullBox_release(&obj->prntFullBox);
}

BOOL NBMp4VideoMediaHeaderBox_loadFromBuff(STNBMp4VideoMediaHeaderBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	if(!NBMp4FullBox_loadBits(&obj->prntFullBox, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd)){
		return FALSE;
	} if(obj->prntFullBox.version != 0 || obj->prntFullBox.flags != 0x1){
		return FALSE;
	}
	if(!NBMp4_readUI16(&obj->graphicsMode, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI16s(&obj->opColor[0], 3, buff, buffAfterEnd)){
		return FALSE;
	}
	return (*buff == buffAfterEnd);
}

void NBMp4VideoMediaHeaderBox_concat(const STNBMp4VideoMediaHeaderBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4FullBox_concat(&obj->prntFullBox, depthLvl, optSrcFile, dst);
		//
		{
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "graphicsMode('"); NBString_concatUI32(dst, obj->graphicsMode); NBString_concat(dst, "')");
			NBString_concat(dst, " opColor("); NBString_concatUI32(dst, obj->opColor[0]); NBString_concat(dst, ", "); NBString_concatUI32(dst, obj->opColor[1]); NBString_concat(dst, ", "); NBString_concatUI32(dst, obj->opColor[2]); NBString_concat(dst, ")");
			NBString_concatByte(dst, '\n');
		}
	}
}

BOOL NBMp4VideoMediaHeaderBox_writeBits(STNBMp4VideoMediaHeaderBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4FullBox_writeHdrBits(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI16(&obj->graphicsMode, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI16s(&obj->opColor[0], 3, dst)){
		return FALSE;
	}
	if(!NBMp4FullBox_writeHdrFinalSize(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(VideoMediaHeaderBox)

//NBMp4BitRateBox

/*class BitRateBox extends Box(‘btrt’){
   unsigned int(32) bufferSizeDB;
   unsigned int(32) maxBitrate;
   unsigned int(32) avgBitrate;
}*/

void NBMp4BitRateBox_init(STNBMp4BitRateBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4BitRateBox);
	NBMp4Box_init(&obj->prntBox);
	//
	obj->prntBox.hdr.type.str[0] = 'b';
	obj->prntBox.hdr.type.str[1] = 't';
	obj->prntBox.hdr.type.str[2] = 'r';
	obj->prntBox.hdr.type.str[3] = 't';
}

void NBMp4BitRateBox_release(STNBMp4BitRateBox* obj){
	NBMp4Box_release(&obj->prntBox);
}

BOOL NBMp4BitRateBox_loadFromBuff(STNBMp4BitRateBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	const BYTE* buffStart = *buff;
	if(!NBMp4_readUI32(&obj->bufferSizeDB, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->maxBitrate, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->avgBitrate, buff, buffAfterEnd)){
		return FALSE;
	}
	//
	NBMp4Box_cloneHdr(&obj->prntBox, prnt, boxFileStart, hdrFileSz, (UI32)(buffAfterEnd - buffStart), hdr);
	//
	return (*buff == buffAfterEnd);
}

void NBMp4BitRateBox_concat(const STNBMp4BitRateBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4Box_concatHdr(&obj->prntBox, depthLvl, optSrcFile, dst);
		//
		{
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "bufferSizeDB("); NBString_concatUI32(dst, obj->bufferSizeDB); NBString_concat(dst, ")");
			NBString_concat(dst, " maxBitrate("); NBString_concatUI32(dst, obj->maxBitrate); NBString_concat(dst, ")");
			NBString_concat(dst, " avgBitrate("); NBString_concatUI32(dst, obj->avgBitrate); NBString_concat(dst, ")");
			NBString_concatByte(dst, '\n');
		}
	}
}

BOOL NBMp4BitRateBox_writeBits(STNBMp4BitRateBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4Box_writeHdrBits(&obj->prntBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->bufferSizeDB, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->maxBitrate, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->avgBitrate, dst)){
		return FALSE;
	}
	if(!NBMp4Box_writeHdrFinalSize(&obj->prntBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(BitRateBox)

//NBMp4PixelAspectRatioBox

/*class PixelAspectRatioBox extends Box(‘pasp’){
	unsigned int(32) hSpacing;
	unsigned int(32) vSpacing;
}*/

void NBMp4PixelAspectRatioBox_init(STNBMp4PixelAspectRatioBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4PixelAspectRatioBox);
	NBMp4Box_init(&obj->prntBox);
	//
	obj->prntBox.hdr.type.str[0] = 'p';
	obj->prntBox.hdr.type.str[1] = 'a';
	obj->prntBox.hdr.type.str[2] = 's';
	obj->prntBox.hdr.type.str[3] = 'p';
}

void NBMp4PixelAspectRatioBox_release(STNBMp4PixelAspectRatioBox* obj){
	NBMp4Box_release(&obj->prntBox);
}

BOOL NBMp4PixelAspectRatioBox_loadFromBuff(STNBMp4PixelAspectRatioBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	const BYTE* buffStart = *buff;
	if(!NBMp4_readUI32(&obj->hSpacing, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->vSpacing, buff, buffAfterEnd)){
		return FALSE;
	}
	//
	NBMp4Box_cloneHdr(&obj->prntBox, prnt, boxFileStart, hdrFileSz, (UI32)(buffAfterEnd - buffStart), hdr);
	//
	return (*buff == buffAfterEnd);
}

void NBMp4PixelAspectRatioBox_concat(const STNBMp4PixelAspectRatioBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4Box_concatHdr(&obj->prntBox, depthLvl, optSrcFile, dst);
		//
		{
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "hSpacing("); NBString_concatUI32(dst, obj->hSpacing); NBString_concat(dst, ")");
			NBString_concat(dst, " vSpacing("); NBString_concatUI32(dst, obj->vSpacing); NBString_concat(dst, ")");
			NBString_concatByte(dst, '\n');
		}
	}
}

BOOL NBMp4PixelAspectRatioBox_writeBits(STNBMp4PixelAspectRatioBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4Box_writeHdrBits(&obj->prntBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->hSpacing, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->vSpacing, dst)){
		return FALSE;
	}
	if(!NBMp4Box_writeHdrFinalSize(&obj->prntBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

void NBMp4PixelAspectRatioBox_setHSpacing(STNBMp4PixelAspectRatioBox* obj, const UI32 hSpacing){
	obj->hSpacing = hSpacing;
}

void NBMp4PixelAspectRatioBox_setVSpacing(STNBMp4PixelAspectRatioBox* obj, const UI32 vSpacing){
	obj->vSpacing = vSpacing;
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(PixelAspectRatioBox)

//NBMp4CleanApertureBox

/*class CleanApertureBox extends Box(‘clap’){
	unsigned int(32) cleanApertureWidthN;
	unsigned int(32) cleanApertureWidthD;
	unsigned int(32) cleanApertureHeightN;
	unsigned int(32) cleanApertureHeightD;
	unsigned int(32) horizOffN;
	unsigned int(32) horizOffD;
	unsigned int(32) vertOffN;
	unsigned int(32) vertOffD;
}*/

void NBMp4CleanApertureBox_init(STNBMp4CleanApertureBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4CleanApertureBox);
	NBMp4Box_init(&obj->prntBox);
	//
	obj->prntBox.hdr.type.str[0] = 'c';
	obj->prntBox.hdr.type.str[1] = 'l';
	obj->prntBox.hdr.type.str[2] = 'a';
	obj->prntBox.hdr.type.str[3] = 'p';
}

void NBMp4CleanApertureBox_release(STNBMp4CleanApertureBox* obj){
	NBMp4Box_release(&obj->prntBox);
}

BOOL NBMp4CleanApertureBox_loadFromBuff(STNBMp4CleanApertureBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	const BYTE* buffStart = *buff;
	if(!NBMp4_readUI32(&obj->cleanApertureWidthN, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->cleanApertureWidthD, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->cleanApertureHeightN, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->cleanApertureHeightD, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->horizOffN, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->horizOffD, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->vertOffN, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->vertOffD, buff, buffAfterEnd)){
		return FALSE;
	}
	//
	NBMp4Box_cloneHdr(&obj->prntBox, prnt, boxFileStart, hdrFileSz, (UI32)(buffAfterEnd - buffStart), hdr);
	//
	return (*buff == buffAfterEnd);
}

void NBMp4CleanApertureBox_concat(const STNBMp4CleanApertureBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4Box_concatHdr(&obj->prntBox, depthLvl, optSrcFile, dst);
		//
		{
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "cleanApertureWidthN("); NBString_concatUI32(dst, obj->cleanApertureWidthN); NBString_concat(dst, ")");
			NBString_concat(dst, " cleanApertureWidthD("); NBString_concatUI32(dst, obj->cleanApertureWidthD); NBString_concat(dst, ")");
			NBString_concat(dst, " cleanApertureHeightN("); NBString_concatUI32(dst, obj->cleanApertureHeightN); NBString_concat(dst, ")");
			NBString_concat(dst, " cleanApertureHeightD("); NBString_concatUI32(dst, obj->cleanApertureHeightD); NBString_concat(dst, ")");
			NBString_concat(dst, " horizOffN("); NBString_concatUI32(dst, obj->horizOffN); NBString_concat(dst, ")");
			NBString_concat(dst, " horizOffD("); NBString_concatUI32(dst, obj->horizOffD); NBString_concat(dst, ")");
			NBString_concat(dst, " vertOffN("); NBString_concatUI32(dst, obj->vertOffN); NBString_concat(dst, ")");
			NBString_concat(dst, " vertOffD("); NBString_concatUI32(dst, obj->vertOffD); NBString_concat(dst, ")");
			NBString_concatByte(dst, '\n');
		}
	}
}

BOOL NBMp4CleanApertureBox_writeBits(STNBMp4CleanApertureBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4Box_writeHdrBits(&obj->prntBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->cleanApertureWidthN, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->cleanApertureWidthD, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->cleanApertureHeightN, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->cleanApertureHeightD, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->horizOffN, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->horizOffD, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->vertOffN, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->vertOffD, dst)){
		return FALSE;
	}
	if(!NBMp4Box_writeHdrFinalSize(&obj->prntBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(CleanApertureBox)

//NBMp4SampleEntry

/*aligned(8) abstract class SampleEntry (unsigned int(32) format) extends Box(format){
	const unsigned int(8)[6] reserved = 0;
	unsigned int(16) data_reference_index;
}*/

void NBMp4SampleEntry_init(STNBMp4SampleEntry* obj){
	NBMemory_setZeroSt(*obj, STNBMp4SampleEntry);
	NBMp4Box_init(&obj->prntBox);
}

void NBMp4SampleEntry_release(STNBMp4SampleEntry* obj){
	if(obj->data != NULL){
		NBMemory_free(obj->data);
		obj->data = NULL;
		obj->dataSz = 0;
	}
	NBMp4Box_release(&obj->prntBox);
}

BOOL NBMp4SampleEntry_loadFromBuff(STNBMp4SampleEntry* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	const BYTE* buffStart = *buff;
	if(!NBMp4_readUI8s(&obj->reserved[0], 6, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI16(&obj->dataReferenceIndex, buff, buffAfterEnd)){
		return FALSE;
	}
	//
	NBMp4Box_cloneHdr(&obj->prntBox, prnt, boxFileStart, hdrFileSz, (UI32)(buffAfterEnd - buffStart), hdr);
	//
	return TRUE; //abstract
}

void NBMp4SampleEntry_concat(const STNBMp4SampleEntry* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4Box_concatHdr(&obj->prntBox, depthLvl, optSrcFile, dst);
		//
		{
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			//UI8			reserved[6];	//reserved
			NBString_concat(dst, "dataRefIndex('"); NBString_concatUI32(dst, obj->dataReferenceIndex); NBString_concat(dst, "')");
			NBString_concatByte(dst, '\n');
		}
		//
		{
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "data("); NBString_concatUI32(dst, obj->dataSz); NBString_concat(dst, " bytes)");
			NBString_concatByte(dst, '\n');
		}
	}
}
	
BOOL NBMp4SampleEntry_writeBits(STNBMp4SampleEntry* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4Box_writeHdrBits(&obj->prntBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI8s(&obj->reserved[0], 6, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI16(&obj->dataReferenceIndex, dst)){
		return FALSE;
	}
	//trailing-data
	if(obj->data != NULL && obj->dataSz > 0){
		if(!NBMp4_writeBytes(obj->data, obj->dataSz, dst)){
			return FALSE;
		}
	}
	if(!NBMp4Box_writeHdrFinalSize(&obj->prntBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

void NBMp4SampleEntry_setDataRefIndex(STNBMp4SampleEntry* obj, const UI16 idx){
	obj->dataReferenceIndex = idx;
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(SampleEntry)

//NBMp4VisualSampleEntry

/*class VisualSampleEntry(codingname) extends SampleEntry (codingname){
	unsigned int(16) pre_defined = 0;
	const unsigned int(16) reserved = 0;
	unsigned int(32)[3]  pre_defined = 0;
	unsigned int(16)  width;
	unsigned int(16)  height;
	template unsigned int(32)  horizresolution = 0x00480000; // 72 dpi
	template unsigned int(32)  vertresolution  = 0x00480000; // 72 dpi
	const unsigned int(32)  reserved = 0;
	template unsigned int(16)  frame_count = 1;
	string[32]  compressorname;
	template unsigned int(16)  depth = 0x0018;
	int(16)  pre_defined = -1;
	// other boxes from derived specifications
	CleanApertureBox     clap;    // optional
	PixelAspectRatioBox  pasp;    // optional
}*/

void NBMp4VisualSampleEntry_init(STNBMp4VisualSampleEntry* obj){
	NBMemory_setZeroSt(*obj, STNBMp4VisualSampleEntry);
	NBMp4SampleEntry_init(&obj->prntEntry);
	//default values
	obj->horizResolution	= 0x00480000; // 72 dpi
	obj->vertResolution		= 0x00480000; // 72 dpi
	obj->frameCount			= 1;
	obj->depth				= 0x0018;
	obj->preDefined3		= -1;
}

void NBMp4VisualSampleEntry_release(STNBMp4VisualSampleEntry* obj){
	NBMp4SampleEntry_release(&obj->prntEntry);
}

BOOL NBMp4VisualSampleEntry_loadFromBuff(STNBMp4VisualSampleEntry* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	const BYTE* buffStart = *buff;
	if(!NBMp4SampleEntry_loadFromBuff(&obj->prntEntry, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI16(&obj->preDefined, buff, buffAfterEnd)){
		return FALSE;
	} else if(obj->preDefined != 0){
		return FALSE;
	}
	if(!NBMp4_readUI16(&obj->reserved, buff, buffAfterEnd)){
		return FALSE;
	} else if(obj->reserved != 0){
		return FALSE;
	}
	if(!NBMp4_readUI32s(obj->preDefined2, 3, buff, buffAfterEnd)){
		return FALSE;
	} else if(obj->preDefined2[0] != 0 || obj->preDefined2[1] != 0 || obj->preDefined2[2] != 0){
		return FALSE;
	}
	if(!NBMp4_readUI16(&obj->width, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI16(&obj->height, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->horizResolution, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->vertResolution, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->reserved2, buff, buffAfterEnd)){
		return FALSE;
	} else if(obj->reserved2 != 0){
		return FALSE;
	}
	if(!NBMp4_readUI16(&obj->frameCount, buff, buffAfterEnd)){
		return FALSE;
	}
	{
		char compressorName[32];
		if(!NBMp4_readBytes(compressorName, 32, buff, buffAfterEnd)){
			return FALSE;
		} else if(compressorName[0] > 31){
			return FALSE;
		} else {
			obj->compressorName.len = compressorName[0];
			NBMemory_copy(obj->compressorName.value, &compressorName[1], obj->compressorName.len);
			obj->compressorName.value[obj->compressorName.len] = '\0';
		}
	}
	if(!NBMp4_readUI16(&obj->depth, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readSI16(&obj->preDefined3, buff, buffAfterEnd)){
		return FALSE;
	} else if(obj->preDefined3 != -1){
		return FALSE;
	}
	//clap (optional)
	{
		const STNBMp4BoxDef def = NBMP4_BOX_PARSEABLE_DEF( 'c', 'l', 'a', 'p', CleanApertureBox );
		STNBMp4Box* clap = NBMp4Box_alloctIfMatchDef(&def, &obj->prntEntry.prntBox, depthLvl + 1, boxFileStart + hdrFileSz + (*buff - buffStart), buff, buffAfterEnd);
		if(clap != NULL){
			if(obj->clap != NULL){
				NBMp4CleanApertureBox_release(obj->clap);
				NBMemory_free(obj->clap);
			}
			obj->clap = (STNBMp4CleanApertureBox*)clap;
		}
	}
	//pasp (optional)
	{
		const STNBMp4BoxDef def = NBMP4_BOX_PARSEABLE_DEF( 'p', 'a', 's', 'p', PixelAspectRatioBox );
		STNBMp4Box* pasp = NBMp4Box_alloctIfMatchDef(&def, &obj->prntEntry.prntBox, depthLvl + 1, boxFileStart + hdrFileSz + (*buff - buffStart), buff, buffAfterEnd);
		if(pasp != NULL){
			if(obj->pasp != NULL){
				NBMp4PixelAspectRatioBox_release(obj->pasp);
				NBMemory_free(obj->pasp);
			}
			obj->pasp = (STNBMp4PixelAspectRatioBox*)pasp;
		}
	}
	return TRUE; //abstract
}

void NBMp4VisualSampleEntry_concat(const STNBMp4VisualSampleEntry* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4SampleEntry_concat(&obj->prntEntry, depthLvl, optSrcFile, dst);
		//
		{
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "width('"); NBString_concatUI32(dst, obj->width); NBString_concat(dst, "')");
			NBString_concat(dst, " height("); NBString_concatUI32(dst, obj->height); NBString_concat(dst, ")");
			NBString_concat(dst, " hRes("); NBMp4_concatFixed16x16(dst, obj->horizResolution); NBString_concat(dst, ")");
			NBString_concat(dst, " vRes("); NBMp4_concatFixed16x16(dst, obj->vertResolution); NBString_concat(dst, ")");
			NBString_concatByte(dst, '\n');
			//
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "frameCount("); NBString_concatUI32(dst, obj->frameCount); NBString_concat(dst, ")");
			NBString_concat(dst, " compressorName('"); NBString_concatBytes(dst, obj->compressorName.value, obj->compressorName.len); NBString_concat(dst, "')");
			NBString_concat(dst, " depth("); NBString_concatUI32(dst, obj->depth); NBString_concat(dst, ")");
			NBString_concatByte(dst, '\n');
		}
		//clap (optional)
		if(obj->clap != NULL){
			NBMp4CleanApertureBox_concat(obj->clap, depthLvl + 1, optSrcFile, dst);
		}
		//pasp (optional)
		if(obj->pasp != NULL){
			NBMp4PixelAspectRatioBox_concat(obj->pasp, depthLvl + 1, optSrcFile, dst);
		}
	}
}

BOOL NBMp4VisualSampleEntry_writeBits(STNBMp4VisualSampleEntry* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4SampleEntry_writeBits(&obj->prntEntry, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI16(&obj->preDefined, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI16(&obj->reserved, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32s(obj->preDefined2, 3, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI16(&obj->width, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI16(&obj->height, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->horizResolution, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->vertResolution, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->reserved2, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI16(&obj->frameCount, dst)){
		return FALSE;
	}
	{
		if(!NBMp4_writeBytes(&obj->compressorName.len, 1, dst)){
			return FALSE;
		}
		if(!NBMp4_writeBytes(&obj->compressorName.value, 31, dst)){
			return FALSE;
		}
	}
	if(!NBMp4_writeUI16(&obj->depth, dst)){
		return FALSE;
	}
	if(!NBMp4_writeSI16(&obj->preDefined3, dst)){
		return FALSE;
	}
	//clap (optional)
	if(obj->clap != NULL){
		NBMp4CleanApertureBox_writeBits(obj->clap, depthLvl + 1, optSrcFile, dst);
	}
	//pasp (optional)
	if(obj->pasp != NULL){
		NBMp4PixelAspectRatioBox_writeBits(obj->pasp, depthLvl + 1, optSrcFile, dst);
	}
	if(!NBMp4Box_writeHdrFinalSize(&obj->prntEntry.prntBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

void NBMp4VisualSampleEntry_setWidth(STNBMp4VisualSampleEntry* obj, const UI16 width){
	obj->width = width;
}

void NBMp4VisualSampleEntry_setHeight(STNBMp4VisualSampleEntry* obj, const UI16 height){
	obj->height = height;
}

void NBMp4VisualSampleEntry_setFrameCount(STNBMp4VisualSampleEntry* obj, const UI16 frameCount){
	obj->frameCount = frameCount;
}

void NBMp4VisualSampleEntry_setCompressorName(STNBMp4VisualSampleEntry* obj, const char* name31Max){
	if(name31Max == NULL || name31Max[0] == '\0'){
		obj->compressorName.len = 0;
	} else {
		const UI32 len = NBString_strLenBytes(name31Max);
		obj->compressorName.len = (len > 31 ? 31 : len);
		NBMemory_copy(obj->compressorName.value, name31Max, obj->compressorName.len);
		obj->compressorName.value[obj->compressorName.len] = '\0';
	}
}

void NBMp4VisualSampleEntry_setDepth(STNBMp4VisualSampleEntry* obj, const UI16 depth){
	obj->depth = depth;
}

BOOL NBMp4VisualSampleEntry_setClapByOwning(STNBMp4VisualSampleEntry* obj, STNBMp4BoxRef* ref){
	BOOL r = FALSE;
	if(ref == NULL || ref->box == NULL){
		if(obj->clap != NULL){
			NBMp4CleanApertureBox_release(obj->clap);
			NBMemory_free(obj->clap);
			obj->clap = NULL;
		}
		r = TRUE;
	} else if(ref->box->hdr.type.str[0] == 'c' && ref->box->hdr.type.str[1] == 'l' && ref->box->hdr.type.str[2] == 'a' && ref->box->hdr.type.str[3] == 'p'){
		if(obj->clap != NULL){
			NBMp4CleanApertureBox_release(obj->clap);
			NBMemory_free(obj->clap);
			obj->clap = NULL;
		}
		obj->clap = (STNBMp4CleanApertureBox*)ref->box;
		ref->box = NULL;
		r = TRUE;
	}
	return r;
}

BOOL NBMp4VisualSampleEntry_setPaspByOwning(STNBMp4VisualSampleEntry* obj, STNBMp4BoxRef* ref){
	BOOL r = FALSE;
	if(ref == NULL || ref->box == NULL){
		if(obj->pasp != NULL){
			NBMp4PixelAspectRatioBox_release(obj->pasp);
			NBMemory_free(obj->pasp);
			obj->pasp = NULL;
		}
		r = TRUE;
	} else if(ref->box->hdr.type.str[0] == 'p' && ref->box->hdr.type.str[1] == 'a' && ref->box->hdr.type.str[2] == 's' && ref->box->hdr.type.str[3] == 'p'){
		if(obj->pasp != NULL){
			NBMp4PixelAspectRatioBox_release(obj->pasp);
			NBMemory_free(obj->pasp);
			obj->pasp = NULL;
		}
		obj->pasp = (STNBMp4PixelAspectRatioBox*)ref->box;
		ref->box = NULL;
		r = TRUE;
	}
	return r;
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(VisualSampleEntry)

//NBMp4SampleDescriptionBox

/*aligned(8) class SampleDescriptionBox (unsigned int(32) handler_type) extends FullBox('stsd', version, 0){
   int i ;
   unsigned int(32) entry_count;
   for (i = 1 ; i <= entry_count ; i++){
	  SampleEntry();    // an instance of a class derived from SampleEntry
   } 
}*/

void NBMp4SampleDescriptionBox_init(STNBMp4SampleDescriptionBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4SampleDescriptionBox);
	NBMp4FullBox_init(&obj->prntFullBox);
	//
	obj->prntFullBox.prntBox.hdr.type.str[0] = 's';
	obj->prntFullBox.prntBox.hdr.type.str[1] = 't';
	obj->prntFullBox.prntBox.hdr.type.str[2] = 's';
	obj->prntFullBox.prntBox.hdr.type.str[3] = 'd';
}

void NBMp4SampleDescriptionBox_release(STNBMp4SampleDescriptionBox* obj){
	if(obj->entries != NULL){
		UI32 i; for(i = 0; i < obj->entryCount; i++){
			NBMp4BoxRef_release(&obj->entries[i]);
		}
		NBMemory_free(obj->entries);
		obj->entries = NULL;
		obj->entryCount = 0;
	}
	NBMp4FullBox_release(&obj->prntFullBox);
}

BOOL NBMp4SampleDescriptionBox_loadFromBuff(STNBMp4SampleDescriptionBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	const BYTE* buffStart = *buff;
	if(!NBMp4FullBox_loadBits(&obj->prntFullBox, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd)){
		return FALSE;
	} if(obj->prntFullBox.version != 0 || obj->prntFullBox.flags != 0){
		return FALSE;
	}
	{
		UI32 i, entryCount;
		if(!NBMp4_readUI32(&entryCount, buff, buffAfterEnd)){
			return FALSE;
		}
		if(entryCount > 0){
			BOOL errFnd = FALSE;
			STNBMp4BoxRef* entries = NBMemory_allocTypes(STNBMp4BoxRef, entryCount);
			for(i = 0; i < entryCount && !errFnd; i++){
				const BYTE* boxStart = *buff;
				STNBMp4BoxHdr hdr2;
				NBMp4BoxHdr_init(&hdr2);
				if(!NBMp4BoxHdr_loadBits(&hdr2, buff, buffAfterEnd)){
					errFnd = TRUE;
				} else {
					const BYTE* payStart = *buff;
					const UI32 hdr2Sz = (UI32)(payStart - boxStart);
					const UI32 paySz = (UI32)hdr2.size - hdr2Sz;
					STNBMp4BoxRef* e = &entries[i];
					NBMp4BoxRef_init(e);
					if(!NBMp4BoxRef_allocBoxFromBuff(e, &obj->prntFullBox.prntBox, depthLvl + 1, boxFileStart + hdrFileSz + (boxStart - buffStart), (UI8)hdr2Sz, &hdr2, buff, *buff + paySz)){
						PRINTF_ERROR("NBMp4SampleDescriptionBox, failed('%c%c%c%c') %d bytes.\n", hdr2.type.str[0], hdr2.type.str[1], hdr2.type.str[2], hdr2.type.str[3], hdr2.size);
						errFnd = TRUE;
					} 
				}
				NBMp4BoxHdr_release(&hdr2);
			}
			//Release if not success
			if(errFnd){
				UI32 i2; for(i2 = 0; i2 < i; i2++){
					NBMp4BoxRef_release(&entries[i2]);
				}
				NBMemory_free(entries);
				entries = NULL;
				entryCount = 0;
				return FALSE;
			}
			//apply
			{
				//Release current
				if(obj->entries != NULL){
					UI32 i; for(i = 0; i < obj->entryCount; i++){
						NBMp4BoxRef_release(&obj->entries[i]);
					}
					NBMemory_free(obj->entries);
					obj->entries = NULL;
					obj->entryCount = 0;
				}
				obj->entries = entries;
				obj->entryCount = entryCount;
			}
		}
	}
	return (*buff == buffAfterEnd);
}

void NBMp4SampleDescriptionBox_concat(const STNBMp4SampleDescriptionBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4FullBox_concat(&obj->prntFullBox, depthLvl, optSrcFile, dst);
		//
		{
			UI32 i; for(i = 0; i < obj->entryCount; i++){
				STNBMp4BoxRef* e = &obj->entries[i];
				NBMp4BoxRef_concat(e, depthLvl + 1, optSrcFile, dst);
			}
		}
	}
}

BOOL NBMp4SampleDescriptionBox_writeBits(STNBMp4SampleDescriptionBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4FullBox_writeHdrBits(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->entryCount, dst)){
		return FALSE;
	}
	if(obj->entries != NULL && obj->entryCount > 0){
		UI32 i; for(i = 0; i < obj->entryCount; i++){
			if(!NBMp4BoxRef_writeBits(&obj->entries[i], depthLvl, optSrcFile, dst)){
				return FALSE;
			}
		}
	}
	if(!NBMp4FullBox_writeHdrFinalSize(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

BOOL NBMp4SampleDescriptionBox_addEntryByOwning(STNBMp4SampleDescriptionBox* obj, STNBMp4BoxRef* boxRef){
	BOOL r = FALSE;
	if(boxRef != NULL && boxRef->box != NULL && obj->entryCount < 0xFFFFFFFFU){
		STNBMp4BoxRef* nArr = NBMemory_allocTypes(STNBMp4BoxRef, obj->entryCount + 1);
		if(obj->entries != NULL){
			if(obj->entryCount > 0){
				NBMemory_copy(nArr, obj->entries, sizeof(obj->entries[0]) * obj->entryCount);
			}
			NBMemory_free(obj->entries);
		}
		obj->entries = nArr;
		{
			STNBMp4BoxRef* e = &obj->entries[obj->entryCount++];
			NBMp4BoxRef_init(e);
			e->stDef = boxRef->stDef;
			e->box = boxRef->box; boxRef->box = NULL;
		}
		r = TRUE;
	}
	return r;
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(SampleDescriptionBox)

//NBMp4TimeToSampleBox

/*aligned(8) class TimeToSampleBox extends FullBox(’stts’, version = 0, 0) {
   unsigned int(32)  entry_count;
   int i;
   for (i=0; i < entry_count; i++) {
	  unsigned int(32)  sample_count;
	  unsigned int(32)  sample_delta;
   }
}*/

void NBMp4TimeToSampleBox_init(STNBMp4TimeToSampleBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4TimeToSampleBox);
	NBMp4FullBox_init(&obj->prntFullBox);
	//
	obj->prntFullBox.prntBox.hdr.type.str[0] = 's';
	obj->prntFullBox.prntBox.hdr.type.str[1] = 't';
	obj->prntFullBox.prntBox.hdr.type.str[2] = 't';
	obj->prntFullBox.prntBox.hdr.type.str[3] = 's';
}

void NBMp4TimeToSampleBox_release(STNBMp4TimeToSampleBox* obj){
	if(obj->entries != NULL){
		NBMemory_free(obj->entries);
		obj->entries = NULL;
		obj->entryCount = 0;
	}
	NBMp4FullBox_release(&obj->prntFullBox);
}

BOOL NBMp4TimeToSampleBox_loadFromBuff(STNBMp4TimeToSampleBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	if(!NBMp4FullBox_loadBits(&obj->prntFullBox, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd)){
		return FALSE;
	} if(obj->prntFullBox.version != 0 || obj->prntFullBox.flags != 0){
		return FALSE;
	}
	{
		UI32 i, entryCount; STNBMp4TimeToSampleEntry* entries = NULL;
		if(!NBMp4_readUI32(&entryCount, buff, buffAfterEnd)){
			return FALSE;
		}
		if(entryCount > 0){
			entries = NBMemory_allocTypes(STNBMp4TimeToSampleEntry, entryCount);
			for(i = 0; i < entryCount; i++){
				STNBMp4TimeToSampleEntry* e = &entries[i];
				if(!NBMp4_readUI32(&e->sampleCount, buff, buffAfterEnd)){
					break;
				}
				if(!NBMp4_readUI32(&e->sampleDelta, buff, buffAfterEnd)){
					break;
				}
			}
			//Release if not success
			if(i < entryCount){
				NBMemory_free(entries);
				entries = NULL;
				entryCount = 0;
				return FALSE;
			}
			//apply
			{
				//Release current
				if(obj->entries != NULL){
					NBMemory_free(obj->entries);
					obj->entries = NULL;
					obj->entryCount = 0;
				}
				obj->entries = entries;
				obj->entryCount = entryCount;
			}
		}
	}
	return (*buff == buffAfterEnd);
}

void NBMp4TimeToSampleBox_concat(const STNBMp4TimeToSampleBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4FullBox_concat(&obj->prntFullBox, depthLvl, optSrcFile, dst);
		//
		{
			UI32 i; for(i = 0; i < obj->entryCount; i++){
				STNBMp4TimeToSampleEntry* e = &obj->entries[i];
				NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
				NBString_concat(dst, "sampleCount("); NBString_concatUI32(dst, e->sampleCount); NBString_concat(dst, ")");
				NBString_concat(dst, " sampleDelta("); NBString_concatUI32(dst, e->sampleDelta); NBString_concat(dst, ")");
				NBString_concatRepeatedByte(dst, '\n', depthLvl + 1);
			}
		}
	}
}

BOOL NBMp4TimeToSampleBox_writeBits(STNBMp4TimeToSampleBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4FullBox_writeHdrBits(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->entryCount, dst)){
		return FALSE;
	}
	if(obj->entries != NULL && obj->entryCount > 0){
		UI32 i; for(i = 0; i < obj->entryCount; i++){
			STNBMp4TimeToSampleEntry* e = &obj->entries[i];
			if(!NBMp4_writeUI32(&e->sampleCount, dst)){
				return FALSE;
			}
			if(!NBMp4_writeUI32(&e->sampleDelta, dst)){
				return FALSE;
			}
		}
	}
	if(!NBMp4FullBox_writeHdrFinalSize(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(TimeToSampleBox)

//NBMp4SampleToChunkBox

/*aligned(8) class SampleToChunkBox extends FullBox(‘stsc’, version = 0, 0) {
   unsigned int(32)  entry_count;
   for (i=1; i <= entry_count; i++) {
      unsigned int(32)  first_chunk;
      unsigned int(32)  samples_per_chunk;
      unsigned int(32)  sample_description_index;
   }
}*/

void NBMp4SampleToChunkBox_init(STNBMp4SampleToChunkBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4SampleToChunkBox);
	NBMp4FullBox_init(&obj->prntFullBox);
	//
	obj->prntFullBox.prntBox.hdr.type.str[0] = 's';
	obj->prntFullBox.prntBox.hdr.type.str[1] = 't';
	obj->prntFullBox.prntBox.hdr.type.str[2] = 's';
	obj->prntFullBox.prntBox.hdr.type.str[3] = 'c';
}

void NBMp4SampleToChunkBox_release(STNBMp4SampleToChunkBox* obj){
	if(obj->entries != NULL){
		NBMemory_free(obj->entries);
		obj->entries = NULL;
		obj->entryCount = 0;
	}
	NBMp4FullBox_release(&obj->prntFullBox);
}

BOOL NBMp4SampleToChunkBox_loadFromBuff(STNBMp4SampleToChunkBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	if(!NBMp4FullBox_loadBits(&obj->prntFullBox, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd)){
		return FALSE;
	} if(obj->prntFullBox.version != 0 || obj->prntFullBox.flags != 0){
		return FALSE;
	}
	{
		UI32 i, entryCount; STNBMp4SampleToChunkEntry* entries = NULL;
		if(!NBMp4_readUI32(&entryCount, buff, buffAfterEnd)){
			return FALSE;
		}
		if(entryCount > 0){
			entries = NBMemory_allocTypes(STNBMp4SampleToChunkEntry, entryCount);
			for(i = 0; i < entryCount; i++){
				STNBMp4SampleToChunkEntry* e = &entries[i];
				if(!NBMp4_readUI32(&e->firstChunk, buff, buffAfterEnd)){
					break;
				}
				if(!NBMp4_readUI32(&e->samplesPerChunk, buff, buffAfterEnd)){
					break;
				}
				if(!NBMp4_readUI32(&e->sampleDescriptionIndex, buff, buffAfterEnd)){
					break;
				}
			}
			//Release if not success
			if(i < entryCount){
				NBMemory_free(entries);
				entries = NULL;
				entryCount = 0;
				return FALSE;
			}
			//apply
			{
				//Release current
				if(obj->entries != NULL){
					NBMemory_free(obj->entries);
					obj->entries = NULL;
					obj->entryCount = 0;
				}
				obj->entries = entries;
				obj->entryCount = entryCount;
			}
		}
	}
	return (*buff == buffAfterEnd);
}

void NBMp4SampleToChunkBox_concat(const STNBMp4SampleToChunkBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4FullBox_concat(&obj->prntFullBox, depthLvl, optSrcFile, dst);
		//
		{
			UI32 i; for(i = 0; i < obj->entryCount; i++){
				STNBMp4SampleToChunkEntry* e = &obj->entries[i];
				NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
				NBString_concat(dst, "firstChunk("); NBString_concatUI32(dst, e->firstChunk); NBString_concat(dst, ")");
				NBString_concat(dst, " samplesPerChunk("); NBString_concatUI32(dst, e->samplesPerChunk); NBString_concat(dst, ")");
				NBString_concat(dst, " sampleDescriptionIndex("); NBString_concatUI32(dst, e->sampleDescriptionIndex); NBString_concat(dst, ")");
				NBString_concatRepeatedByte(dst, '\n', depthLvl + 1);
			}
		}
	}
}

BOOL NBMp4SampleToChunkBox_writeBits(STNBMp4SampleToChunkBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4FullBox_writeHdrBits(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->entryCount, dst)){
		return FALSE;
	}
	if(obj->entries != NULL && obj->entryCount > 0){
		UI32 i; for(i = 0; i < obj->entryCount; i++){
			const STNBMp4SampleToChunkEntry* e = &obj->entries[i];
			if(!NBMp4_writeUI32(&e->firstChunk, dst)){
				return FALSE;
			}
			if(!NBMp4_writeUI32(&e->samplesPerChunk, dst)){
				return FALSE;
			}
			if(!NBMp4_writeUI32(&e->sampleDescriptionIndex, dst)){
				return FALSE;
			}
		}
	}
	if(!NBMp4FullBox_writeHdrFinalSize(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(SampleToChunkBox)

//NBMp4ChunkOffsetBox

/*aligned(8) class ChunkOffsetBox extends FullBox(‘stco’, version = 0, 0) {
   unsigned int(32)  entry_count;
   for (i=1; i <= entry_count; i++) {
	  unsigned int(32)  chunk_offset;
   }
}*/

void NBMp4ChunkOffsetBox_init(STNBMp4ChunkOffsetBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4ChunkOffsetBox);
	NBMp4FullBox_init(&obj->prntFullBox);
	//
	obj->prntFullBox.prntBox.hdr.type.str[0] = 's';
	obj->prntFullBox.prntBox.hdr.type.str[1] = 't';
	obj->prntFullBox.prntBox.hdr.type.str[2] = 'c';
	obj->prntFullBox.prntBox.hdr.type.str[3] = 'o';
}

void NBMp4ChunkOffsetBox_release(STNBMp4ChunkOffsetBox* obj){
	if(obj->entries != NULL){
		NBMemory_free(obj->entries);
		obj->entries = NULL;
		obj->entryCount = 0;
	}
	NBMp4FullBox_release(&obj->prntFullBox);
}

BOOL NBMp4ChunkOffsetBox_loadFromBuff(STNBMp4ChunkOffsetBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	if(!NBMp4FullBox_loadBits(&obj->prntFullBox, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd)){
		return FALSE;
	} if(obj->prntFullBox.version != 0 || obj->prntFullBox.flags != 0){
		return FALSE;
	}
	{
		UI32 i, entryCount; UI32* entries = NULL;
		if(!NBMp4_readUI32(&entryCount, buff, buffAfterEnd)){
			return FALSE;
		}
		if(entryCount > 0){
			entries = NBMemory_allocTypes(UI32, entryCount);
			for(i = 0; i < entryCount; i++){
				if(!NBMp4_readUI32(&entries[i], buff, buffAfterEnd)){
					break;
				}
			}
			//Release if not success
			if(i < entryCount){
				NBMemory_free(entries);
				entries = NULL;
				entryCount = 0;
				return FALSE;
			}
			//apply
			{
				//Release current
				if(obj->entries != NULL){
					NBMemory_free(obj->entries);
					obj->entries = NULL;
					obj->entryCount = 0;
				}
				obj->entries = entries;
				obj->entryCount = entryCount;
			}
		}
	}
	return (*buff == buffAfterEnd);
}

void NBMp4ChunkOffsetBox_concat(const STNBMp4ChunkOffsetBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4FullBox_concat(&obj->prntFullBox, depthLvl, optSrcFile, dst);
		//
		if(obj->entries != NULL && obj->entryCount > 0){
			UI32 i;
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "entries: ");
			for(i = 0; i < obj->entryCount; i++){
				const UI32* e = &obj->entries[i];
				NBString_concatUI32(dst, *e);
				if((i + 1) % 8){
					NBString_concatByte(dst, '\n');
					NBString_concat(dst, "           ");
				}
			}
			if(i % 8){
				NBString_concatByte(dst, '\n');
			}
		}
	}
}

BOOL NBMp4ChunkOffsetBox_writeBits(STNBMp4ChunkOffsetBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4FullBox_writeHdrBits(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->entryCount, dst)){
		return FALSE;
	}
	if(obj->entries != NULL && obj->entryCount > 0){
		UI32 i;  for(i = 0; i < obj->entryCount; i++){
			if(!NBMp4_writeUI32(&obj->entries[i], dst)){
				return FALSE;
			}
		}
	}
	if(!NBMp4FullBox_writeHdrFinalSize(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(ChunkOffsetBox)

//NBMp4ChunkOffsetBox64

/*aligned(8) class ChunkLargeOffsetBox extends FullBox(‘co64’, version = 0, 0) {
  unsigned int(32)  entry_count;
  for (i=1; i <= entry_count; i++) {
	 unsigned int(64)  chunk_offset;
  }
}*/

void NBMp4ChunkOffsetBox64_init(STNBMp4ChunkOffsetBox64* obj){
	NBMemory_setZeroSt(*obj, STNBMp4ChunkOffsetBox64);
	NBMp4FullBox_init(&obj->prntFullBox);
	//
	obj->prntFullBox.prntBox.hdr.type.str[0] = 'c';
	obj->prntFullBox.prntBox.hdr.type.str[1] = 'o';
	obj->prntFullBox.prntBox.hdr.type.str[2] = '6';
	obj->prntFullBox.prntBox.hdr.type.str[3] = '4';
}

void NBMp4ChunkOffsetBox64_release(STNBMp4ChunkOffsetBox64* obj){
	if(obj->entries != NULL){
		NBMemory_free(obj->entries);
		obj->entries = NULL;
		obj->entryCount = 0;
	}
	NBMp4FullBox_release(&obj->prntFullBox);
}

BOOL NBMp4ChunkOffsetBox64_loadFromBuff(STNBMp4ChunkOffsetBox64* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	if(!NBMp4FullBox_loadBits(&obj->prntFullBox, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd)){
		return FALSE;
	} if(obj->prntFullBox.version != 0 || obj->prntFullBox.flags != 0){
		return FALSE;
	}
	{
		UI32 i, entryCount; UI64* entries = NULL;
		if(!NBMp4_readUI32(&entryCount, buff, buffAfterEnd)){
			return FALSE;
		}
		if(entryCount > 0){
			entries = NBMemory_allocTypes(UI64, entryCount);
			for(i = 0; i < entryCount; i++){
				if(!NBMp4_readUI64(&entries[i], buff, buffAfterEnd)){
					break;
				}
			}
			//Release if not success
			if(i < entryCount){
				NBMemory_free(entries);
				entries = NULL;
				entryCount = 0;
				return FALSE;
			}
			//apply
			{
				//Release current
				if(obj->entries != NULL){
					NBMemory_free(obj->entries);
					obj->entries = NULL;
					obj->entryCount = 0;
				}
				obj->entries = entries;
				obj->entryCount = entryCount;
			}
		}
	}
	return (*buff == buffAfterEnd);
}

void NBMp4ChunkOffsetBox64_concat(const STNBMp4ChunkOffsetBox64* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4FullBox_concat(&obj->prntFullBox, depthLvl, optSrcFile, dst);
		//
		if(obj->entries != NULL && obj->entryCount > 0){
			UI32 i;
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "entries: ");
			for(i = 0; i < obj->entryCount; i++){
				const UI64* e = &obj->entries[i];
				NBString_concatUI64(dst, *e);
				if((i + 1) % 8){
					NBString_concatByte(dst, '\n');
					NBString_concat(dst, "           ");
				}
			}
			if(i % 8){
				NBString_concatByte(dst, '\n');
			}
		}
	}
}

BOOL NBMp4ChunkOffsetBox64_writeBits(STNBMp4ChunkOffsetBox64* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4FullBox_writeHdrBits(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->entryCount, dst)){
		return FALSE;
	}
	if(obj->entries != NULL && obj->entryCount > 0){
		UI32 i; for(i = 0; i < obj->entryCount; i++){
			if(!NBMp4_writeUI64(&obj->entries[i], dst)){
				return FALSE;
			}
		}
	}
	if(!NBMp4FullBox_writeHdrFinalSize(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(ChunkOffsetBox64)

//NBMp4TrackExtendsBox

/*aligned(8) class TrackExtendsBox extends FullBox(‘trex’, 0, 0){
   unsigned int(32)  track_ID;
   unsigned int(32)  default_sample_description_index;
   unsigned int(32)  default_sample_duration;
   unsigned int(32)  default_sample_size;
   unsigned int(32)  default_sample_flags;
}*/

void NBMp4TrackExtendsBox_init(STNBMp4TrackExtendsBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4TrackExtendsBox);
	NBMp4FullBox_init(&obj->prntFullBox);
	//
	obj->prntFullBox.prntBox.hdr.type.str[0] = 't';
	obj->prntFullBox.prntBox.hdr.type.str[1] = 'r';
	obj->prntFullBox.prntBox.hdr.type.str[2] = 'e';
	obj->prntFullBox.prntBox.hdr.type.str[3] = 'x';
	//
	NBMp4SampleFlags_init(&obj->defaultSampleFlags);
}

void NBMp4TrackExtendsBox_release(STNBMp4TrackExtendsBox* obj){
	NBMp4SampleFlags_release(&obj->defaultSampleFlags);
	NBMp4FullBox_release(&obj->prntFullBox);
}

BOOL NBMp4TrackExtendsBox_loadFromBuff(STNBMp4TrackExtendsBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	if(!NBMp4FullBox_loadBits(&obj->prntFullBox, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd)){
		return FALSE;
	} if(obj->prntFullBox.version != 0 || obj->prntFullBox.flags != 0){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->trackId, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->defaultSampleDescriptionIndex, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->defaultSampleDuration, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->defaultSampleSize, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4SampleFlags_loadFromBuff(&obj->defaultSampleFlags, buff, buffAfterEnd)){
		return FALSE;
	}
	return (*buff == buffAfterEnd);
}

void NBMp4TrackExtendsBox_concat(const STNBMp4TrackExtendsBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4FullBox_concat(&obj->prntFullBox, depthLvl, optSrcFile, dst);
		//
		{
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "trackId("); NBString_concatUI32(dst, obj->trackId); NBString_concat(dst, ")");
			NBString_concat(dst, " defaultSampleDescriptionIndex("); NBString_concatUI32(dst, obj->defaultSampleDescriptionIndex); NBString_concat(dst, ")");
			NBString_concat(dst, " defaultSampleDuration("); NBString_concatUI32(dst, obj->defaultSampleDuration); NBString_concat(dst, ")");
			NBString_concat(dst, " defaultSampleSize("); NBString_concatUI32(dst, obj->defaultSampleSize); NBString_concat(dst, ")");
			NBMp4SampleFlags_concat(&obj->defaultSampleFlags, dst, " defaultSampleFlags");
			NBString_concatByte(dst, '\n');
		}
	}
}

BOOL NBMp4TrackExtendsBox_writeBits(STNBMp4TrackExtendsBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4FullBox_writeHdrBits(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->trackId, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->defaultSampleDescriptionIndex, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->defaultSampleDuration, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->defaultSampleSize, dst)){
		return FALSE;
	}
	if(!NBMp4SampleFlags_writeBits(&obj->defaultSampleFlags, depthLvl, NB_OBJREF_NULL, dst)){
		return FALSE;
	}
	if(!NBMp4FullBox_writeHdrFinalSize(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

void NBMp4TrackExtendsBox_setTrackId(STNBMp4TrackExtendsBox* obj, const UI32 trackId){
	obj->trackId = trackId;
}

void NBMp4TrackExtendsBox_setDefaultSampleDescriptionIndex(STNBMp4TrackExtendsBox* obj, const UI32 defaultSampleDescriptionIndex){
	obj->defaultSampleDescriptionIndex = defaultSampleDescriptionIndex;
}

void NBMp4TrackExtendsBox_setDefaultSampleDuration(STNBMp4TrackExtendsBox* obj, const UI32 defaultSampleDuration){
	obj->defaultSampleDuration = defaultSampleDuration;
}

void NBMp4TrackExtendsBox_setDefaultSampleSize(STNBMp4TrackExtendsBox* obj, const UI32 defaultSampleSize){
	obj->defaultSampleSize = defaultSampleSize;
}

void NBMp4TrackExtendsBox_setDefaultSampleFlags(STNBMp4TrackExtendsBox* obj, const STNBMp4SampleFlags* flags){
	NBMp4SampleFlags_setFromOther(&obj->defaultSampleFlags, flags);
} 

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(TrackExtendsBox)

//NBMp4MovieFragmentHeaderBox

/*aligned(8) class MovieFragmentHeaderBox extends FullBox(‘mfhd’, 0, 0){
  unsigned int(32)  sequence_number;
}
*/

void NBMp4MovieFragmentHeaderBox_init(STNBMp4MovieFragmentHeaderBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4MovieFragmentHeaderBox);
	NBMp4FullBox_init(&obj->prntFullBox);
	//
	obj->prntFullBox.prntBox.hdr.type.str[0] = 'm';
	obj->prntFullBox.prntBox.hdr.type.str[1] = 'f';
	obj->prntFullBox.prntBox.hdr.type.str[2] = 'h';
	obj->prntFullBox.prntBox.hdr.type.str[3] = 'd';
}

void NBMp4MovieFragmentHeaderBox_release(STNBMp4MovieFragmentHeaderBox* obj){
	NBMp4FullBox_release(&obj->prntFullBox);
}

BOOL NBMp4MovieFragmentHeaderBox_loadFromBuff(STNBMp4MovieFragmentHeaderBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	if(!NBMp4FullBox_loadBits(&obj->prntFullBox, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd)){
		return FALSE;
	} if(obj->prntFullBox.version != 0 || obj->prntFullBox.flags != 0){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->sequenceNumber, buff, buffAfterEnd)){
		return FALSE;
	}
	return (*buff == buffAfterEnd);
}

void NBMp4MovieFragmentHeaderBox_concat(const STNBMp4MovieFragmentHeaderBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4FullBox_concat(&obj->prntFullBox, depthLvl, optSrcFile, dst);
		//
		{
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "sequenceNumber("); NBString_concatUI32(dst, obj->sequenceNumber); NBString_concat(dst, ")");
			NBString_concatByte(dst, '\n');
		}
	}
}

BOOL NBMp4MovieFragmentHeaderBox_writeBits(STNBMp4MovieFragmentHeaderBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4FullBox_writeHdrBits(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->sequenceNumber, dst)){
		return FALSE;
	}
	if(!NBMp4FullBox_writeHdrFinalSize(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

void NBMp4MovieFragmentHeaderBox_setSeqNumber(STNBMp4MovieFragmentHeaderBox* obj, const UI32 seqNum){
	obj->sequenceNumber = seqNum;
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(MovieFragmentHeaderBox)

//NBMp4TrackFragmentHeaderBox

/*aligned(8) class TrackFragmentHeaderBox extends FullBox(‘tfhd’, 0, tf_flags){
	unsigned int(32)  track_ID;
	// all the following are optional fields
	unsigned int(64)  base_data_offset;
	unsigned int(32)  sample_description_index;
	unsigned int(32)  default_sample_duration;
	unsigned int(32)  default_sample_size;
	unsigned int(32)  default_sample_flags
}*/

void NBMp4TrackFragmentHeaderBox_init(STNBMp4TrackFragmentHeaderBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4TrackFragmentHeaderBox);
	NBMp4FullBox_init(&obj->prntFullBox);
	//
	obj->prntFullBox.prntBox.hdr.type.str[0] = 't';
	obj->prntFullBox.prntBox.hdr.type.str[1] = 'f';
	obj->prntFullBox.prntBox.hdr.type.str[2] = 'h';
	obj->prntFullBox.prntBox.hdr.type.str[3] = 'd';
	//
	NBMp4SampleFlags_init(&obj->defaultSampleFlags);
}

void NBMp4TrackFragmentHeaderBox_release(STNBMp4TrackFragmentHeaderBox* obj){
	NBMp4SampleFlags_release(&obj->defaultSampleFlags);
	NBMp4FullBox_release(&obj->prntFullBox);
}

BOOL NBMp4TrackFragmentHeaderBox_loadFromBuff(STNBMp4TrackFragmentHeaderBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	if(!NBMp4FullBox_loadBits(&obj->prntFullBox, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd)){
		return FALSE;
	} if(obj->prntFullBox.version != 0){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->trackId, buff, buffAfterEnd)){
		return FALSE;
	}
	if(obj->prntFullBox.flags & NBMp4TrackFragmentHeaderBoxFlag_base_data_offset_present){
		if(!NBMp4_readUI64(&obj->baseDataOffset, buff, buffAfterEnd)){
			return FALSE;
		}
	}
	if(obj->prntFullBox.flags & NBMp4TrackFragmentHeaderBoxFlag_sample_description_index_present){
		if(!NBMp4_readUI32(&obj->sampleDescriptionIndex, buff, buffAfterEnd)){
			return FALSE;
		}
	}
	if(obj->prntFullBox.flags & NBMp4TrackFragmentHeaderBoxFlag_default_sample_duration_present){
		if(!NBMp4_readUI32(&obj->defaultSampleDuration, buff, buffAfterEnd)){
			return FALSE;
		}
	}
	if(obj->prntFullBox.flags & NBMp4TrackFragmentHeaderBoxFlag_default_sample_size_present){
		if(!NBMp4_readUI32(&obj->defaultSampleSize, buff, buffAfterEnd)){
			return FALSE;
		}
	}
	if(obj->prntFullBox.flags & NBMp4TrackFragmentHeaderBoxFlag_default_sample_flags_present){
		if(!NBMp4SampleFlags_loadFromBuff(&obj->defaultSampleFlags, buff, buffAfterEnd)){
			return FALSE;
		}
	}
	return (*buff == buffAfterEnd);
}

void NBMp4TrackFragmentHeaderBox_concat(const STNBMp4TrackFragmentHeaderBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4FullBox_concat(&obj->prntFullBox, depthLvl, optSrcFile, dst);
		//
		{
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "trackId("); NBString_concatUI32(dst, obj->trackId); NBString_concat(dst, ")");
			if(obj->prntFullBox.flags & NBMp4TrackFragmentHeaderBoxFlag_base_data_offset_present){
				NBString_concat(dst, " baseDataOffset("); NBString_concatUI64(dst, obj->baseDataOffset); NBString_concat(dst, ")");
			}
			if(obj->prntFullBox.flags & NBMp4TrackFragmentHeaderBoxFlag_sample_description_index_present){
				NBString_concat(dst, " sampleDescriptionIndex("); NBString_concatUI32(dst, obj->sampleDescriptionIndex); NBString_concat(dst, ")");
			}
			if(obj->prntFullBox.flags & NBMp4TrackFragmentHeaderBoxFlag_default_sample_duration_present){
				NBString_concat(dst, " defaultSampleDuration("); NBString_concatUI32(dst, obj->defaultSampleDuration); NBString_concat(dst, ")");
			}
			if(obj->prntFullBox.flags & NBMp4TrackFragmentHeaderBoxFlag_default_sample_size_present){
				NBString_concat(dst, " defaultSampleSize("); NBString_concatUI32(dst, obj->defaultSampleSize); NBString_concat(dst, ")");
			}
			if(obj->prntFullBox.flags & NBMp4TrackFragmentHeaderBoxFlag_default_sample_flags_present){
				NBMp4SampleFlags_concat(&obj->defaultSampleFlags, dst, " defaultSampleFlags");
			}
			NBString_concatByte(dst, '\n');
		}
	}
}

BOOL NBMp4TrackFragmentHeaderBox_writeBits(STNBMp4TrackFragmentHeaderBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4FullBox_writeHdrBits(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->trackId, dst)){
		return FALSE;
	}
	if(obj->prntFullBox.flags & NBMp4TrackFragmentHeaderBoxFlag_base_data_offset_present){
		if(!NBMp4_writeUI64(&obj->baseDataOffset, dst)){
			return FALSE;
		}
	}
	if(obj->prntFullBox.flags & NBMp4TrackFragmentHeaderBoxFlag_sample_description_index_present){
		if(!NBMp4_writeUI32(&obj->sampleDescriptionIndex, dst)){
			return FALSE;
		}
	}
	if(obj->prntFullBox.flags & NBMp4TrackFragmentHeaderBoxFlag_default_sample_duration_present){
		if(!NBMp4_writeUI32(&obj->defaultSampleDuration, dst)){
			return FALSE;
		}
	}
	if(obj->prntFullBox.flags & NBMp4TrackFragmentHeaderBoxFlag_default_sample_size_present){
		if(!NBMp4_writeUI32(&obj->defaultSampleSize, dst)){
			return FALSE;
		}
	}
	if(obj->prntFullBox.flags & NBMp4TrackFragmentHeaderBoxFlag_default_sample_flags_present){
		if(!NBMp4SampleFlags_writeBits(&obj->defaultSampleFlags, depthLvl, optSrcFile, dst)){
			return FALSE;
		}
	}
	if(!NBMp4FullBox_writeHdrFinalSize(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

UI64 NBMp4TrackFragmentHeaderBox_getBaseDataOffset(const STNBMp4TrackFragmentHeaderBox* obj){
	UI64 r = 0;
	if(obj->prntFullBox.flags & NBMp4TrackFragmentHeaderBoxFlag_base_data_offset_present){
		r = obj->baseDataOffset;
	}
	return r;
}

UI32 NBMp4TrackFragmentHeaderBox_getDefaultSampleSize(const STNBMp4TrackFragmentHeaderBox* obj){
	UI32 r = 0;
	if(obj->prntFullBox.flags & NBMp4TrackFragmentHeaderBoxFlag_default_sample_size_present){
		r = obj->defaultSampleSize;
	}
	return r;
}

//

void NBMp4TrackFragmentHeaderBox_setTrackId(STNBMp4TrackFragmentHeaderBox* obj, const UI32 trackId){
	obj->trackId = trackId;
}

void NBMp4TrackFragmentHeaderBox_setBaseDataOffset(STNBMp4TrackFragmentHeaderBox* obj, const UI64 baseDataOffset){
	if(baseDataOffset == 0){
		obj->prntFullBox.flags &= ~NBMp4TrackFragmentHeaderBoxFlag_base_data_offset_present;
	} else {
		obj->prntFullBox.flags |= NBMp4TrackFragmentHeaderBoxFlag_base_data_offset_present;
	}
	obj->baseDataOffset = baseDataOffset;
}

void NBMp4TrackFragmentHeaderBox_setSampleDescriptionIndex(STNBMp4TrackFragmentHeaderBox* obj, const UI32 sampleDescriptionIndex){
	if(sampleDescriptionIndex == 0){
		obj->prntFullBox.flags &= ~NBMp4TrackFragmentHeaderBoxFlag_sample_description_index_present;
	} else {
		obj->prntFullBox.flags |= NBMp4TrackFragmentHeaderBoxFlag_sample_description_index_present;
	}
	obj->sampleDescriptionIndex = sampleDescriptionIndex;
}

void NBMp4TrackFragmentHeaderBox_setDefaultSampleDuration(STNBMp4TrackFragmentHeaderBox* obj, const UI32 defaultSampleDuration){
	if(defaultSampleDuration == 0){
		obj->prntFullBox.flags &= ~NBMp4TrackFragmentHeaderBoxFlag_default_sample_duration_present;
	} else {
		obj->prntFullBox.flags |= NBMp4TrackFragmentHeaderBoxFlag_default_sample_duration_present;
	}
	obj->defaultSampleDuration = defaultSampleDuration;
}

void NBMp4TrackFragmentHeaderBox_setDefaultSampleSize(STNBMp4TrackFragmentHeaderBox* obj, const UI32 defaultSampleSize){
	if(defaultSampleSize == 0){
		obj->prntFullBox.flags &= ~NBMp4TrackFragmentHeaderBoxFlag_default_sample_size_present;
	} else {
		obj->prntFullBox.flags |= NBMp4TrackFragmentHeaderBoxFlag_default_sample_size_present;
	}
	obj->defaultSampleSize = defaultSampleSize;
}

void NBMp4TrackFragmentHeaderBox_setDefaultSampleFlags(STNBMp4TrackFragmentHeaderBox* obj, const STNBMp4SampleFlags* defaultSampleFlags){
	if(defaultSampleFlags == NULL){
		obj->prntFullBox.flags &= ~NBMp4TrackFragmentHeaderBoxFlag_default_sample_flags_present;
	} else {
		obj->prntFullBox.flags |= NBMp4TrackFragmentHeaderBoxFlag_default_sample_flags_present;
		NBMp4SampleFlags_setFromOther(&obj->defaultSampleFlags, defaultSampleFlags);
	}
}

void NBMp4TrackFragmentHeaderBox_setDurationIsEmpty(STNBMp4TrackFragmentHeaderBox* obj, const BOOL durIsEmpty){
	if(durIsEmpty){
		obj->prntFullBox.flags |= NBMp4TrackFragmentHeaderBoxFlag_duration_is_empty;
	} else {
		obj->prntFullBox.flags &= ~NBMp4TrackFragmentHeaderBoxFlag_duration_is_empty;
	}
}

void NBMp4TrackFragmentHeaderBox_setDefaultBaseIsMoof(STNBMp4TrackFragmentHeaderBox* obj, const BOOL defBaseIsMoof){
	if(defBaseIsMoof){
		obj->prntFullBox.flags |= NBMp4TrackFragmentHeaderBoxFlag_default_base_is_moof;
	} else {
		obj->prntFullBox.flags &= ~NBMp4TrackFragmentHeaderBoxFlag_default_base_is_moof;
	}
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(TrackFragmentHeaderBox)

//NBMp4TrackFragmentBaseMediaDecodeTimeBox

/*aligned(8) class TrackFragmentBaseMediaDecodeTimeBox extends FullBox(‘tfdt’, version, 0) {
	if (version==1) {
		unsigned int(64) baseMediaDecodeTime;
	} else { // version==0
		unsigned int(32) baseMediaDecodeTime;
	}
}*/

void NBMp4TrackFragmentBaseMediaDecodeTimeBox_init(STNBMp4TrackFragmentBaseMediaDecodeTimeBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4TrackFragmentBaseMediaDecodeTimeBox);
	NBMp4FullBox_init(&obj->prntFullBox);
	//
	obj->prntFullBox.prntBox.hdr.type.str[0] = 't';
	obj->prntFullBox.prntBox.hdr.type.str[1] = 'f';
	obj->prntFullBox.prntBox.hdr.type.str[2] = 'd';
	obj->prntFullBox.prntBox.hdr.type.str[3] = 't';
}

void NBMp4TrackFragmentBaseMediaDecodeTimeBox_release(STNBMp4TrackFragmentBaseMediaDecodeTimeBox* obj){
	NBMp4FullBox_release(&obj->prntFullBox);
}

BOOL NBMp4TrackFragmentBaseMediaDecodeTimeBox_loadFromBuff(STNBMp4TrackFragmentBaseMediaDecodeTimeBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	if(!NBMp4FullBox_loadBits(&obj->prntFullBox, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd)){
		return FALSE;
	} if(obj->prntFullBox.flags != 0){
		return FALSE;
	}
	if(obj->prntFullBox.version == 0){
		if(!NBMp4_readUI32(&obj->baseMediaDecodeTime.v0, buff, buffAfterEnd)){
			return FALSE;
		}
	} else if(obj->prntFullBox.version == 1){
		if(!NBMp4_readUI64(&obj->baseMediaDecodeTime.v1, buff, buffAfterEnd)){
			return FALSE;
		}
	} else {
		return FALSE;
	}
	return (*buff == buffAfterEnd);
}

void NBMp4TrackFragmentBaseMediaDecodeTimeBox_concat(const STNBMp4TrackFragmentBaseMediaDecodeTimeBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4FullBox_concat(&obj->prntFullBox, depthLvl, optSrcFile, dst);
		//
		{
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			if(obj->prntFullBox.version == 0){
				NBString_concat(dst, "baseMediaDecodeTime("); NBString_concatUI32(dst, obj->baseMediaDecodeTime.v0); NBString_concat(dst, ")");
			} else {
				NBString_concat(dst, "baseMediaDecodeTime("); NBString_concatUI64(dst, obj->baseMediaDecodeTime.v1); NBString_concat(dst, ")");
			}
			NBString_concatByte(dst, '\n');
		}
	}
}

BOOL NBMp4TrackFragmentBaseMediaDecodeTimeBox_writeBits(STNBMp4TrackFragmentBaseMediaDecodeTimeBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4FullBox_writeHdrBits(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(obj->prntFullBox.version == 0){
		if(!NBMp4_writeUI32(&obj->baseMediaDecodeTime.v0, dst)){
			return FALSE;
		}
	} else if(obj->prntFullBox.version == 1){
		if(!NBMp4_writeUI64(&obj->baseMediaDecodeTime.v1, dst)){
			return FALSE;
		}
	} else {
		return FALSE;
	}
	if(!NBMp4FullBox_writeHdrFinalSize(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

void NBMp4TrackFragmentBaseMediaDecodeTimeBox_setBaseMediaDecodeTime(STNBMp4TrackFragmentBaseMediaDecodeTimeBox* obj, const UI64 baseMediaDecodeTime){
	if(baseMediaDecodeTime <= 0xFFFFFFFFU){
		obj->prntFullBox.version = 0;
		obj->baseMediaDecodeTime.v0 = (UI32)baseMediaDecodeTime;
	} else {
		obj->prntFullBox.version = 1;
		obj->baseMediaDecodeTime.v1 = baseMediaDecodeTime;
	}
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(TrackFragmentBaseMediaDecodeTimeBox)

//NBMp4TrackRunBox

/*aligned(8) class TrackRunBox extends FullBox(‘trun’, version, tr_flags) {
   unsigned int(32)  sample_count;
   // the following are optional fields
   signed int(32) data_offset;
   unsigned int(32)  first_sample_flags;
   // all fields in the following array are optional
   {
      unsigned int(32)  sample_duration;
      unsigned int(32)  sample_size;
      unsigned int(32)  sample_flags
      if (version == 0){
         unsigned int(32) sample_composition_time_offset;
      } else {
         signed int(32) sample_composition_time_offset;
       }
   }[ sample_count ];
}*/
//sample-composition-time-offset = presentationTime - decodingTime.

void NBMp4TrackRunBox_init(STNBMp4TrackRunBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4TrackRunBox);
	NBMp4FullBox_init(&obj->prntFullBox);
	//
	obj->prntFullBox.prntBox.hdr.type.str[0] = 't';
	obj->prntFullBox.prntBox.hdr.type.str[1] = 'r';
	obj->prntFullBox.prntBox.hdr.type.str[2] = 'u';
	obj->prntFullBox.prntBox.hdr.type.str[3] = 'n';
	//
	NBMp4SampleFlags_init(&obj->firstSampleFlags);
}

void NBMp4TrackRunBox_release(STNBMp4TrackRunBox* obj){
	if(obj->entries != NULL){
		UI32 i; for(i = 0; i < obj->sampleCount; i++){
			STNBMp4TrackRunEntry* e = &obj->entries[i];
			NBMp4SampleFlags_release(&e->sampleFlags);
		}
		NBMemory_free(obj->entries);
		obj->entries = NULL;
		obj->entriesAlloc = 0;
		obj->sampleCount = 0;
	}
	NBMp4SampleFlags_release(&obj->firstSampleFlags);
	NBMp4FullBox_release(&obj->prntFullBox);
}

BOOL NBMp4TrackRunBox_loadFromBuff(STNBMp4TrackRunBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	UI32 sampleCount = 0;
	if(!NBMp4FullBox_loadBits(&obj->prntFullBox, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI32(&sampleCount, buff, buffAfterEnd)){
		return FALSE;
	}
	if(obj->prntFullBox.flags & NBMp4TrackRunBoxFlag_data_offset_present){
		if(!NBMp4_readSI32(&obj->dataOffset, buff, buffAfterEnd)){
			return FALSE;
		}
	}
	if(obj->prntFullBox.flags & NBMp4TrackRunBoxFlag_first_sample_flags_present){
		if(!NBMp4SampleFlags_loadFromBuff(&obj->firstSampleFlags, buff, buffAfterEnd)){
			return FALSE;
		}
	}
	{
		UI32 i; STNBMp4TrackRunEntry* entries = NULL;
		if(sampleCount > 0){
			entries = NBMemory_allocTypes(STNBMp4TrackRunEntry, sampleCount);
			for(i = 0; i < sampleCount; i++){ 
				STNBMp4TrackRunEntry* e = &entries[i];
				NBMemory_setZeroSt(*e, STNBMp4TrackRunEntry);
				NBMp4SampleFlags_init(&e->sampleFlags); 
				if(obj->prntFullBox.flags & NBMp4TrackRunBoxFlag_sample_duration_present){
					if(!NBMp4_readUI32(&e->sampleDuration, buff, buffAfterEnd)){
						break;
					}
				}
				if(obj->prntFullBox.flags & NBMp4TrackRunBoxFlag_sample_size_present){
					if(!NBMp4_readUI32(&e->sampleSize, buff, buffAfterEnd)){
						break;
					}
				}
				if(i == 0 && (obj->prntFullBox.flags & NBMp4TrackRunBoxFlag_first_sample_flags_present)){
					//using first sampleFlags
					NBMp4SampleFlags_setFromOther(&e->sampleFlags, &obj->firstSampleFlags);
				} else if(obj->prntFullBox.flags & NBMp4TrackRunBoxFlag_sample_flags_present){
					if(!NBMp4SampleFlags_loadFromBuff(&e->sampleFlags, buff, buffAfterEnd)){
						break;
					}
				}
				if(obj->prntFullBox.flags & NBMp4TrackRunBoxFlag_sample_composition_time_offsets_present){
					if(obj->prntFullBox.version == 0){
						if(!NBMp4_readUI32(&e->sampleCompositionTimeOffset.v0, buff, buffAfterEnd)){
							break;
						}
					} else {
						if(!NBMp4_readSI32(&e->sampleCompositionTimeOffset.vAny, buff, buffAfterEnd)){
							break;
						}
					}
				}
			}
			//Release if not success
			if(i < sampleCount){
				UI32 i2; for(i2 = 0; i2 < i; i2++){
					STNBMp4TrackRunEntry* e = &obj->entries[i2];
					NBMp4SampleFlags_release(&e->sampleFlags);
				}
				NBMemory_free(entries);
				entries = NULL;
				sampleCount = 0;
				return FALSE;
			}
		}
		//apply
		{
			//Release current
			if(obj->entries != NULL){
				UI32 i; for(i = 0; i < obj->sampleCount; i++){
					STNBMp4TrackRunEntry* e = &obj->entries[i];
					NBMp4SampleFlags_release(&e->sampleFlags);
				}
				NBMemory_free(obj->entries);
				obj->entries = NULL;
				obj->sampleCount = 0;
			}
			obj->entries		= entries;
			obj->entriesAlloc	= sampleCount;
			obj->sampleCount	= sampleCount;
		}
	}
	return (*buff == buffAfterEnd);
}

void NBMp4TrackRunBox_concat(const STNBMp4TrackRunBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		UI64 fileBase = obj->prntFullBox.prntBox.filePos.iStart;
		UI64 offBase = 0; SI32 offData = 0; UI32 defSampleSize = 0;
		{
			const STNBMp4Box* prnt = obj->prntFullBox.prntBox.prnt;
			while(prnt != NULL && prnt->hdr.type.str[0] != '\0' && prnt->hdr.type.str[1] != '\0' && prnt->hdr.type.str[2] != '\0' && prnt->hdr.type.str[3] != '\0'){ //type '\0\0\0\0' is root-box
				//'traf', seach for base_offset
				if(prnt->hdr.type.str[0] == 't' && prnt->hdr.type.str[1] == 'r' && prnt->hdr.type.str[2] == 'a' && prnt->hdr.type.str[3] == 'f'){
					//'traf', seach for base_offset
					const STNBMp4BoxRef* sub = NBArray_dataPtr(&prnt->subBoxes, STNBMp4BoxRef);
					const STNBMp4BoxRef* subAfterEnd = sub + prnt->subBoxes.use;
					while(sub < subAfterEnd){
						if(sub->box->hdr.type.str[0] == 't' && sub->box->hdr.type.str[1] == 'f' && sub->box->hdr.type.str[2] == 'h' && sub->box->hdr.type.str[3] == 'd'){
							const STNBMp4TrackFragmentHeaderBox* subb = (const STNBMp4TrackFragmentHeaderBox*)sub->box;
							offBase			= NBMp4TrackFragmentHeaderBox_getBaseDataOffset(subb);
							defSampleSize	= NBMp4TrackFragmentHeaderBox_getDefaultSampleSize(subb);
						}
						//next
						sub++;
					}
				}
				//fileBase
				fileBase = prnt->filePos.iStart;
				//parent
				prnt = prnt->prnt;
			}
		}
#		endif
		NBMp4FullBox_concat(&obj->prntFullBox, depthLvl, optSrcFile, dst);
		//
		{
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			if(obj->prntFullBox.flags & NBMp4TrackRunBoxFlag_data_offset_present){
				NBString_concat(dst, "dataOffset("); NBString_concatSI32(dst, obj->dataOffset); NBString_concat(dst, ")");
				IF_NBASSERT(offData = obj->dataOffset;)
			}
			if(obj->prntFullBox.flags & NBMp4TrackRunBoxFlag_first_sample_flags_present){
				NBMp4SampleFlags_concat(&obj->firstSampleFlags, dst, "firstSampleFlags");
			}
			NBString_concatByte(dst, '\n');
		}
		//
		if(obj->entries != NULL && obj->sampleCount > 0){
			UI32 totalDuration = 0, totalSize = 0; SI32 totalCompositionTime = 0;
			IF_NBASSERT(SI64 offCur = offBase + offData;)
			IF_NBASSERT(SI64 offPrev = offCur;)
			UI32 i; for(i = 0; i < obj->sampleCount; i++){
				STNBMp4TrackRunEntry* e = &obj->entries[i];
				BOOL lineStarted = FALSE; 
				//flags = (i == 0 && () ? &obj->firstSampleFlags : &);
				if(obj->prntFullBox.flags & NBMp4TrackRunBoxFlag_sample_duration_present){
					if(lineStarted){
						NBString_concat(dst, " ");
					} else {
						NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
						lineStarted = TRUE;
					}
					NBString_concat(dst, "sampleDuration("); NBString_concatUI32(dst, e->sampleDuration); NBString_concat(dst, ")");
					totalDuration += e->sampleDuration;
				}
				if(obj->prntFullBox.flags & NBMp4TrackRunBoxFlag_sample_size_present){
					if(lineStarted){
						NBString_concat(dst, " ");
					} else {
						NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
						lineStarted = TRUE;
					}
					NBString_concat(dst, "sampleSize("); NBString_concatUI32(dst, e->sampleSize); NBString_concat(dst, ")");
					totalSize += e->sampleSize;
					IF_NBASSERT(offCur += e->sampleSize;)
				} else {
					IF_NBASSERT(offCur += defSampleSize;)
				}
				if(i == 0 && (obj->prntFullBox.flags & NBMp4TrackRunBoxFlag_first_sample_flags_present)){
					//nothing
				} else if(obj->prntFullBox.flags & NBMp4TrackRunBoxFlag_sample_flags_present){
					if(lineStarted){
						NBString_concat(dst, " ");
					} else {
						NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
						lineStarted = TRUE;
					}
					NBMp4SampleFlags_concat(&e->sampleFlags, dst, "sampleFlags");
				}
				if(obj->prntFullBox.flags & NBMp4TrackRunBoxFlag_sample_composition_time_offsets_present){
					if(lineStarted){
						NBString_concat(dst, " ");
					} else {
						NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
						lineStarted = TRUE;
					}
					if(obj->prntFullBox.version == 0){
						NBString_concat(dst, "sampleCompositionTimeOffset("); NBString_concatUI32(dst, e->sampleCompositionTimeOffset.v0); NBString_concat(dst, ")");
						totalCompositionTime += (SI32)e->sampleCompositionTimeOffset.v0;
					} else {
						NBString_concat(dst, "sampleCompositionTimeOffset("); NBString_concatSI32(dst, e->sampleCompositionTimeOffset.vAny); NBString_concat(dst, ")");
						totalCompositionTime += e->sampleCompositionTimeOffset.vAny;
					}
				}
				if(lineStarted){
					NBString_concatByte(dst, '\n');
					lineStarted = FALSE;
				}
#				ifdef NB_CONFIG_INCLUDE_ASSERTS
				//search for content
				if(NBFile_isSet(optSrcFile)){
					NBFile_lock(optSrcFile);
					if(offPrev == offCur){
						NBString_concatRepeatedByte(dst, '\t', depthLvl + 2);
						NBString_concat(dst, "empty-sample\n");
					} else if(offPrev > offCur){
						NBString_concatRepeatedByte(dst, '\t', depthLvl + 2);
						NBString_concat(dst, "invalid-offset\n");
					} else {
						UI64 filePos = fileBase + offPrev;
						if(!NBFile_seek(optSrcFile, filePos, ENNBFileRelative_Start)){
							NBString_concatRepeatedByte(dst, '\t', depthLvl + 2);
							NBString_concat(dst, "first-seek-error\n");
						} else {
							const UI64 filePosAfterEnd = fileBase + offCur; 
							BYTE data[4]; UI32 nalSize = 0, toRead = 0; UI64 fileRemain = 0;
							while(filePos < filePosAfterEnd){
								if(!NBMp4_readUI32File(&nalSize, optSrcFile)){
									NBString_concatRepeatedByte(dst, '\t', depthLvl + 2);
									NBString_concat(dst, "nalSize-read-error\n");
									break;
								} else {
									filePos		+= sizeof(nalSize);
									fileRemain	= (filePosAfterEnd - filePos);
									if(fileRemain < nalSize){
										NBString_concatRepeatedByte(dst, '\t', depthLvl + 2);
										NBString_concat(dst, "nalSize-value-error(");
										NBString_concatUI32(dst, nalSize);
										NBString_concat(dst, " of ");
										NBString_concatUI32(dst, (UI32)fileRemain);
										NBString_concat(dst, " bytes)\n");
										break;
									} else if(nalSize > 0){
										toRead = (nalSize > sizeof(data) ? sizeof(data) : nalSize);
										if(NBFile_read(optSrcFile, data, toRead) != toRead){
											NBString_concatRepeatedByte(dst, '\t', depthLvl + 2);
											NBString_concat(dst, "nalPay-read-error\n");
											break;
										} else {
											//NBAvc_getNaluDef
											const STNBAvcNaluHdr avcHdr = NBAvc_getNaluHdr(data[0]);
											NBString_concatRepeatedByte(dst, '\t', depthLvl + 2);
											//NBString_concat(dst, "; pos("); NBString_concatUI64(dst, fileBase + offPrev); NBString_concat(dst, ", +"); NBString_concatUI64(dst, offCur - offPrev); NBString_concat(dst, ")");
											NBString_concat(dst, "nalSize("); NBString_concatUI32(dst, nalSize); NBString_concat(dst, ")");
											NBString_concat(dst, " type("); NBString_concatUI32(dst, avcHdr.type); NBString_concat(dst, ")");
											NBString_concat(dst, " refIdc("); NBString_concatUI32(dst, avcHdr.refIdc); NBString_concat(dst, ")");
											NBString_concat(dst, " pay(0x"); NBString_concatBytesHex(dst, data, toRead); if(toRead != nalSize){ NBString_concat(dst, "..."); } NBString_concat(dst, ")");
											NBString_concatByte(dst, '\n');
											//
											filePos += toRead;
											if(toRead < nalSize){
												if(!NBFile_seek(optSrcFile, (nalSize - toRead), ENNBFileRelative_CurPos)){
													NBString_concatRepeatedByte(dst, '\t', depthLvl + 2);
													NBString_concat(dst, "nalPay-seek-error\n");
													break;
												} else {
													filePos += (nalSize - toRead);
												}
											}
										}
									}
								}
							} //while
						}
					}
					NBFile_unlock(optSrcFile);
				}
				//next
				offPrev = offCur;
#				endif
			}
			if(totalDuration != 0 || totalSize != 0 || totalCompositionTime != 0){
				BOOL opened = FALSE;
				NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
				if(totalDuration != 0){
					if(opened) NBString_concat(dst, " "); opened = TRUE;
					NBString_concat(dst, "totalDuration("); NBString_concatUI32(dst, totalDuration); NBString_concat(dst, ")");
				}
				if(totalSize != 0){
					if(opened) NBString_concat(dst, " "); opened = TRUE;
					NBString_concat(dst, "totalSize("); NBString_concatUI32(dst, totalSize); NBString_concat(dst, ")");
				}
				if(totalCompositionTime != 0){
					if(opened) NBString_concat(dst, " "); opened = TRUE;
					NBString_concat(dst, "totalCompositionTime("); NBString_concatSI32(dst, totalCompositionTime); NBString_concat(dst, ")");
				}
				NBString_concatByte(dst, '\n');
			}
		}
	}
}

BOOL NBMp4TrackRunBox_writeBits(STNBMp4TrackRunBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4FullBox_writeHdrBits(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->sampleCount, dst)){
		return FALSE;
	}
	if(obj->prntFullBox.flags & NBMp4TrackRunBoxFlag_data_offset_present){
		if(!NBMp4_writeSI32(&obj->dataOffset, dst)){
			return FALSE;
		}
	}
	if(obj->prntFullBox.flags & NBMp4TrackRunBoxFlag_first_sample_flags_present){
		if(!NBMp4SampleFlags_writeBits(&obj->firstSampleFlags, depthLvl, optSrcFile, dst)){
			return FALSE;
		}
	}
	if(obj->entries != NULL && obj->sampleCount > 0){
		UI32 i; for(i = 0; i < obj->sampleCount; i++){ 
			STNBMp4TrackRunEntry* e = &obj->entries[i]; 
			if(obj->prntFullBox.flags & NBMp4TrackRunBoxFlag_sample_duration_present){
				if(!NBMp4_writeUI32(&e->sampleDuration, dst)){
					return FALSE;
				}
			}
			if(obj->prntFullBox.flags & NBMp4TrackRunBoxFlag_sample_size_present){
				if(!NBMp4_writeUI32(&e->sampleSize, dst)){
					return FALSE;
				}
			}
			if(i == 0 && (obj->prntFullBox.flags & NBMp4TrackRunBoxFlag_first_sample_flags_present)){
				//ignore
			} else if(obj->prntFullBox.flags & NBMp4TrackRunBoxFlag_sample_flags_present){
				if(!NBMp4SampleFlags_writeBits(&e->sampleFlags, depthLvl, optSrcFile, dst)){
					return FALSE;
				}
			}
			if(obj->prntFullBox.flags & NBMp4TrackRunBoxFlag_sample_composition_time_offsets_present){
				if(obj->prntFullBox.version == 0){
					if(!NBMp4_writeUI32(&e->sampleCompositionTimeOffset.v0, dst)){
						return FALSE;
					}
				} else {
					if(!NBMp4_writeSI32(&e->sampleCompositionTimeOffset.vAny, dst)){
						return FALSE;
					}
				}
			}
		}
	}
	if(!NBMp4FullBox_writeHdrFinalSize(&obj->prntFullBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

void NBMp4TrackRunBox_setDataOffset(STNBMp4TrackRunBox* obj, const SI32 dataOffset){
	if(dataOffset == 0){
		obj->prntFullBox.flags &= ~NBMp4TrackRunBoxFlag_data_offset_present;
	} else {
		obj->prntFullBox.flags |= NBMp4TrackRunBoxFlag_data_offset_present;
	}
	obj->dataOffset = dataOffset;
}

BOOL NBMp4TrackRunBox_overwriteDataOffset(STNBMp4TrackRunBox* obj, const SI32 dataOffset, STNBString* dst){
	BOOL r = FALSE;
	if(obj->prntFullBox.flags & NBMp4TrackRunBoxFlag_data_offset_present){
		const UI32 iPos = obj->prntFullBox.prntBox.hdr.lastWritePos
			+ NBMp4FullBox_getHdrBytesLen(&obj->prntFullBox) //prntFullBox
			+ 4 //sampleCount
			;
		if((iPos + 4) <= dst->length){
			const BYTE* bSrc = (const BYTE*)&dataOffset;
			BYTE* bDst = (BYTE*)&dst->str[iPos];
			bDst[0] = bSrc[3];
			bDst[1] = bSrc[2];
			bDst[2] = bSrc[1];
			bDst[3] = bSrc[0];
			r = TRUE;
		}
	}
	return r;
}

void NBMp4TrackRunBox_entriesEmpty(STNBMp4TrackRunBox* obj){
	if(obj->entries != NULL){
		UI32 i; for(i = 0; i < obj->sampleCount; i++){
			STNBMp4TrackRunEntry* e = &obj->entries[i];
			NBMp4SampleFlags_release(&e->sampleFlags);
		}
	}
	obj->sampleCount = 0;
}

//flags are NBMp4TrackFragmentHeaderBoxFlag_*
BOOL NBMp4TrackRunBox_entriesAdd(STNBMp4TrackRunBox* obj, const UI32 sampleDuration, const UI32 sampleSize, const STNBMp4SampleFlags* sampleFlags, const UI32 sampleCompositionTimeOffset){
	BOOL r = FALSE;
	//v0 is UI32, v1 is SI32
	if(obj->prntFullBox.version == 0 && obj->sampleCount < 0xFFFFFFFFU){
		//increase array (if necesary)
		while(obj->sampleCount >= obj->entriesAlloc){
			STNBMp4TrackRunEntry* nArr = NBMemory_allocTypes(STNBMp4TrackRunEntry, obj->sampleCount + 1);
			if(obj->entries != NULL){
				if(obj->sampleCount > 0){
					NBMemory_copy(nArr, obj->entries, sizeof(obj->entries[0]) * obj->sampleCount);
				}
				NBMemory_free(obj->entries);
			}
			obj->entries		= nArr;
			obj->entriesAlloc	= obj->sampleCount + 1;
		}
		//populate entry
		{
			STNBMp4TrackRunEntry* e = &obj->entries[obj->sampleCount++]; NBASSERT(obj->sampleCount <= obj->entriesAlloc)
			NBMemory_setZeroSt(*e, STNBMp4TrackRunEntry);
			e->sampleDuration	= sampleDuration;
			e->sampleSize		= sampleSize;
			e->sampleCompositionTimeOffset.v0 = sampleCompositionTimeOffset;
			if(sampleDuration != 0){
				obj->prntFullBox.flags |= NBMp4TrackRunBoxFlag_sample_duration_present;
			}
			if(sampleSize != 0){
				obj->prntFullBox.flags |= NBMp4TrackRunBoxFlag_sample_size_present;
			}
			if(sampleFlags != NULL){
				obj->prntFullBox.flags |= NBMp4TrackRunBoxFlag_sample_flags_present;
				NBMp4SampleFlags_setFromOther(&e->sampleFlags, sampleFlags);
			}
			if(sampleCompositionTimeOffset != 0){
				//sample-composition-time-offset = presentationTime - decodingTime.
				obj->prntFullBox.flags |= NBMp4TrackRunBoxFlag_sample_composition_time_offsets_present;
			}
		}
		r = TRUE;
	}
	return r;
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(TrackRunBox)

//---------------------
// Containers
//---------------------

//NBMp4SampleTableBox

/*aligned(8) class SampleTableBox extends Box(‘stbl’) {
	//
}*/

static STNBMp4SampleTableBox NBMp4SampleTableBoxTplt_;

const static STNBMp4BoxChildDef NBMp4SampleTableBoxChldrn_[] = {
	{ .boxId.str = { 's', 't', 's', 'd' }, (const BYTE*)&NBMp4SampleTableBoxTplt_.stsd, (const BYTE*)&NBMp4SampleTableBoxTplt_ }
	, { .boxId.str = { 's', 't', 't', 's' }, (const BYTE*)&NBMp4SampleTableBoxTplt_.stts, (const BYTE*)&NBMp4SampleTableBoxTplt_ }
	//, { .boxId.str = { 'c', 't', 't', 's' }, (const BYTE*)&NBMp4SampleTableBoxTplt_.ctts, (const BYTE*)&NBMp4SampleTableBoxTplt_ }
	//, { .boxId.str = { 'c', 's', 'l', 'g' }, (const BYTE*)&NBMp4SampleTableBoxTplt_.cslg, (const BYTE*)&NBMp4SampleTableBoxTplt_ }
	, { .boxId.str = { 's', 't', 's', 'c' }, (const BYTE*)&NBMp4SampleTableBoxTplt_.stsc, (const BYTE*)&NBMp4SampleTableBoxTplt_ }
	, { .boxId.str = { 's', 't', 's', 'z' }, (const BYTE*)&NBMp4SampleTableBoxTplt_.stsz, (const BYTE*)&NBMp4SampleTableBoxTplt_ }
	//, { .boxId.str = { 's', 't', 'z', '2' }, (const BYTE*)&NBMp4SampleTableBoxTplt_.stz2, (const BYTE*)&NBMp4SampleTableBoxTplt_ }
	, { .boxId.str = { 's', 't', 'c', 'o' }, (const BYTE*)&NBMp4SampleTableBoxTplt_.stco, (const BYTE*)&NBMp4SampleTableBoxTplt_ }
	//, { .boxId.str = { 'c', 'o', '6', '4' }, (const BYTE*)&NBMp4SampleTableBoxTplt_.co64, (const BYTE*)&NBMp4SampleTableBoxTplt_ }
	//, { .boxId.str = { 's', 't', 's', 's' }, (const BYTE*)&NBMp4SampleTableBoxTplt_.stss, (const BYTE*)&NBMp4SampleTableBoxTplt_ }
	//, { .boxId.str = { 's', 't', 's', 'h' }, (const BYTE*)&NBMp4SampleTableBoxTplt_.stsh, (const BYTE*)&NBMp4SampleTableBoxTplt_ }
	//, { .boxId.str = { 'p', 'a', 'd', 'b' }, (const BYTE*)&NBMp4SampleTableBoxTplt_.padb, (const BYTE*)&NBMp4SampleTableBoxTplt_ }
	//, { .boxId.str = { 's', 't', 'd', 'p' }, (const BYTE*)&NBMp4SampleTableBoxTplt_.stdp, (const BYTE*)&NBMp4SampleTableBoxTplt_ }
	//, { .boxId.str = { 's', 'd', 't', 'p' }, (const BYTE*)&NBMp4SampleTableBoxTplt_.sdtp, (const BYTE*)&NBMp4SampleTableBoxTplt_ }
	//, { .boxId.str = { 's', 'b', 'g', 'p' }, (const BYTE*)&NBMp4SampleTableBoxTplt_.sbgp, (const BYTE*)&NBMp4SampleTableBoxTplt_ }
	//, { .boxId.str = { 's', 'g', 'p', 'd' }, (const BYTE*)&NBMp4SampleTableBoxTplt_.sgpd, (const BYTE*)&NBMp4SampleTableBoxTplt_ }
	//, { .boxId.str = { 's', 'u', 'b', 's' }, (const BYTE*)&NBMp4SampleTableBoxTplt_.subs, (const BYTE*)&NBMp4SampleTableBoxTplt_ }
	//, { .boxId.str = { 's', 'a', 'i', 'z' }, (const BYTE*)&NBMp4SampleTableBoxTplt_.saiz, (const BYTE*)&NBMp4SampleTableBoxTplt_ }
	//, { .boxId.str = { 's', 'a', 'i', 'o' }, (const BYTE*)&NBMp4SampleTableBoxTplt_.saio, (const BYTE*)&NBMp4SampleTableBoxTplt_ }
};

NB_MP4_CONTAINER_FUNCS_AND_ITFS_BODY('s', 't', 'b', 'l', SampleTableBox);

//NBMp4DataInformationBox

/*aligned(8) class DataInformationBox extends Box(‘dinf’) {
	//
}*/

static STNBMp4DataInformationBox NBMp4DataInformationBoxTplt_;

const static STNBMp4BoxChildDef NBMp4DataInformationBoxChldrn_[] = {
	{ .boxId.str = { 'd', 'r', 'e', 'f' }, (const BYTE*)&NBMp4DataInformationBoxTplt_.dref, (const BYTE*)&NBMp4DataInformationBoxTplt_ }
};

NB_MP4_CONTAINER_FUNCS_AND_ITFS_BODY('d', 'i', 'n', 'f', DataInformationBox);

//NBMp4MediaInformationBox

/*aligned(8) class MediaInformationBox extends Box(‘minf’) {
	//
}*/

static STNBMp4MediaInformationBox NBMp4MediaInformationBoxTplt_;

const static STNBMp4BoxChildDef NBMp4MediaInformationBoxChldrn_[] = {
	{ .boxId.str = { 'v', 'm', 'h', 'd' }, (const BYTE*)&NBMp4MediaInformationBoxTplt_.vmhd, (const BYTE*)&NBMp4MediaInformationBoxTplt_ }
	//, { .boxId.str = { 's', 'm', 'h', 'd' }, (const BYTE*)&NBMp4MediaInformationBoxTplt_.smhd, (const BYTE*)&NBMp4MediaInformationBoxTplt_ }
	//, { .boxId.str = { 'h', 'm', 'h', 'd' }, (const BYTE*)&NBMp4MediaInformationBoxTplt_.hmhd, (const BYTE*)&NBMp4MediaInformationBoxTplt_ }
	//, { .boxId.str = { 's', 't', 'h', 'd' }, (const BYTE*)&NBMp4MediaInformationBoxTplt_.sthd, (const BYTE*)&NBMp4MediaInformationBoxTplt_ }
	//, { .boxId.str = { 'n', 'm', 'h', 'd' }, (const BYTE*)&NBMp4MediaInformationBoxTplt_.nmhd, (const BYTE*)&NBMp4MediaInformationBoxTplt_ }
	, { .boxId.str = { 'd', 'i', 'n', 'f' }, (const BYTE*)&NBMp4MediaInformationBoxTplt_.dinf, (const BYTE*)&NBMp4MediaInformationBoxTplt_ }
	, { .boxId.str = { 's', 't', 'b', 'l' }, (const BYTE*)&NBMp4MediaInformationBoxTplt_.stbl, (const BYTE*)&NBMp4MediaInformationBoxTplt_ }
};

NB_MP4_CONTAINER_FUNCS_AND_ITFS_BODY('m', 'i', 'n', 'f', MediaInformationBox);

//NBMp4MediaBox

/*aligned(8) class MediaBox extends Box(‘mdia’) {
	//
}*/

static STNBMp4MediaBox NBMp4MediaBoxTplt_;

const static STNBMp4BoxChildDef NBMp4MediaBoxChldrn_[] = {
	{ .boxId.str = { 'm', 'd', 'h', 'd' }, (const BYTE*)&NBMp4MediaBoxTplt_.mdhd, (const BYTE*)&NBMp4MediaBoxTplt_ }
	, { .boxId.str = { 'h', 'd', 'l', 'r' }, (const BYTE*)&NBMp4MediaBoxTplt_.hdlr, (const BYTE*)&NBMp4MediaBoxTplt_ }
	//, { .boxId.str = { 'e', 'l', 'n', 'g' }, (const BYTE*)&NBMp4MediaBoxTplt_.elng, (const BYTE*)&NBMp4MediaBoxTplt_ }
	, { .boxId.str = { 'm', 'i', 'n', 'f' }, (const BYTE*)&NBMp4MediaBoxTplt_.minf, (const BYTE*)&NBMp4MediaBoxTplt_ }
};

NB_MP4_CONTAINER_FUNCS_AND_ITFS_BODY('m', 'd', 'i', 'a', MediaBox);

//NBMp4TrackBox

/*aligned(8) class TrackBox extends Box(‘trak’) {
	//
}*/

static STNBMp4TrackBox NBMp4TrackBoxTplt_;

const static STNBMp4BoxChildDef NBMp4TrackBoxChldrn_[] = {
	{ .boxId.str = { 't', 'k', 'h', 'd' }, (const BYTE*)&NBMp4TrackBoxTplt_.tkhd, (const BYTE*)&NBMp4TrackBoxTplt_ }
	//, { .boxId.str = { 't', 'r', 'e', 'f' }, (const BYTE*)&NBMp4TrackBoxTplt_.tref, (const BYTE*)&NBMp4TrackBoxTplt_ }
	//, { .boxId.str = { 't', 'r', 'g', 'r' }, (const BYTE*)&NBMp4TrackBoxTplt_.trgr, (const BYTE*)&NBMp4TrackBoxTplt_ }
	//, { .boxId.str = { 'e', 'd', 't', 's' }, (const BYTE*)&NBMp4TrackBoxTplt_.edts, (const BYTE*)&NBMp4TrackBoxTplt_ }
	//, { .boxId.str = { 'm', 'e', 't', 'a' }, (const BYTE*)&NBMp4TrackBoxTplt_.meta, (const BYTE*)&NBMp4TrackBoxTplt_ }
	, { .boxId.str = { 'm', 'd', 'i', 'a' }, (const BYTE*)&NBMp4TrackBoxTplt_.mdia, (const BYTE*)&NBMp4TrackBoxTplt_ }
	//, { .boxId.str = { 'u', 'd', 't', 'a' }, (const BYTE*)&NBMp4TrackBoxTplt_.udta, (const BYTE*)&NBMp4TrackBoxTplt_ }
};

NB_MP4_CONTAINER_FUNCS_AND_ITFS_BODY('t', 'r', 'a', 'k', TrackBox);

//NBMp4MovieExtendsBox

/*aligned(8) class MovieExtendsBox extends Box(‘mvex’){
	//
}*/

static STNBMp4MovieExtendsBox NBMp4MovieExtendsBoxTplt_;

const static STNBMp4BoxChildDef NBMp4MovieExtendsBoxChldrn_[] = {
	//{ .boxId.str = { 'm', 'e', 'h', 'd' }, (const BYTE*)&NBMp4MovieExtendsBoxTplt_.mehd, (const BYTE*)&NBMp4MovieExtendsBoxTplt_ }
	{ .boxId.str = { 't', 'r', 'e', 'x' }, (const BYTE*)&NBMp4MovieExtendsBoxTplt_.trex, (const BYTE*)&NBMp4MovieExtendsBoxTplt_ }
	//, { .boxId.str = { 'l', 'e', 'v', 'a' }, (const BYTE*)&NBMp4MovieExtendsBoxTplt_.leva, (const BYTE*)&NBMp4MovieExtendsBoxTplt_ }
};

NB_MP4_CONTAINER_FUNCS_AND_ITFS_BODY('m', 'v', 'e', 'x', MovieExtendsBox);

//NBMp4MovieBox

/*aligned(8) class MovieBox extends Box(‘moov’){
	//
}*/

static STNBMp4MovieBox NBMp4MovieBoxTplt_;

const static STNBMp4BoxChildDef NBMp4MovieBoxChldrn_[] = {
	{ .boxId.str = { 'm', 'v', 'h', 'd' }, (const BYTE*)&NBMp4MovieBoxTplt_.mvhd, (const BYTE*)&NBMp4MovieBoxTplt_ }
	//, { .boxId.str = { 'm', 'e', 't', 'a' }, (const BYTE*)&NBMp4MovieBoxTplt_.meta, (const BYTE*)&NBMp4MovieBoxTplt_ }
	, { .boxId.str = { 't', 'r', 'a', 'k' }, (const BYTE*)&NBMp4MovieBoxTplt_.trak, (const BYTE*)&NBMp4MovieBoxTplt_ }
	//, { .boxId.str = { 'm', 'v', 'e', 'x' }, (const BYTE*)&NBMp4MovieBoxTplt_.mvex, (const BYTE*)&NBMp4MovieBoxTplt_ }
};

NB_MP4_CONTAINER_FUNCS_AND_ITFS_BODY('m', 'o', 'o', 'v', MovieBox);

//NBMp4TrackFragmentBox

/*aligned(8) class TrackFragmentBox extends Box(‘traf’){
	//
}*/

static STNBMp4TrackFragmentBox NBMp4TrackFragmentBoxTplt_;

const static STNBMp4BoxChildDef NBMp4TrackFragmentBoxChldrn_[] = {
	{ .boxId.str = { 't', 'f', 'h', 'd' }, (const BYTE*)&NBMp4TrackFragmentBoxTplt_.tfhd, (const BYTE*)&NBMp4TrackFragmentBoxTplt_ }
	, { .boxId.str = { 't', 'r', 'u', 'n' }, (const BYTE*)&NBMp4TrackFragmentBoxTplt_.trun, (const BYTE*)&NBMp4TrackFragmentBoxTplt_ }
	//, { .boxId.str = { 's', 'b', 'g', 'p' }, (const BYTE*)&NBMp4TrackFragmentBoxTplt_.sbgp, (const BYTE*)&NBMp4TrackFragmentBoxTplt_ }
	//, { .boxId.str = { 's', 'g', 'p', 'd' }, (const BYTE*)&NBMp4TrackFragmentBoxTplt_.sgpd, (const BYTE*)&NBMp4TrackFragmentBoxTplt_ }
	//, { .boxId.str = { 's', 'u', 'b', 's' }, (const BYTE*)&NBMp4TrackFragmentBoxTplt_.subs, (const BYTE*)&NBMp4TrackFragmentBoxTplt_ }
	//, { .boxId.str = { 's', 'a', 'i', 'z' }, (const BYTE*)&NBMp4TrackFragmentBoxTplt_.saiz, (const BYTE*)&NBMp4TrackFragmentBoxTplt_ }
	//, { .boxId.str = { 's', 'a', 'i', 'o' }, (const BYTE*)&NBMp4TrackFragmentBoxTplt_.saio, (const BYTE*)&NBMp4TrackFragmentBoxTplt_ }
	, { .boxId.str = { 't', 'f', 'd', 't' }, (const BYTE*)&NBMp4TrackFragmentBoxTplt_.tfdt, (const BYTE*)&NBMp4TrackFragmentBoxTplt_ }
	//, { .boxId.str = { 'm', 'e', 't', 'a' }, (const BYTE*)&NBMp4TrackFragmentBoxTplt_.meta, (const BYTE*)&NBMp4TrackFragmentBoxTplt_ }
};

NB_MP4_CONTAINER_FUNCS_AND_ITFS_BODY('t', 'r', 'a', 'f', TrackFragmentBox);

//NBMp4MovieFragmentBox

/*aligned(8) class MovieFragmentBox extends Box(‘moof’){
	//
}*/

static STNBMp4MovieFragmentBox NBMp4MovieFragmentBoxTplt_;

const static STNBMp4BoxChildDef NBMp4MovieFragmentBoxChldrn_[] = {
	{ .boxId.str = { 'm', 'f', 'h', 'd' }, (const BYTE*)&NBMp4MovieFragmentBoxTplt_.mfhd, (const BYTE*)&NBMp4MovieFragmentBoxTplt_ }
	//, { .boxId.str = { 'm', 'e', 't', 'a' }, (const BYTE*)&NBMp4MovieFragmentBoxTplt_.meta, (const BYTE*)&NBMp4MovieFragmentBoxTplt_ }
	, { .boxId.str = { 't', 'r', 'a', 'f' }, (const BYTE*)&NBMp4MovieFragmentBoxTplt_.traf, (const BYTE*)&NBMp4MovieFragmentBoxTplt_ }
};

NB_MP4_CONTAINER_FUNCS_AND_ITFS_BODY('m', 'o', 'o', 'f', MovieFragmentBox);

//ISO-IEC-14496-15

/*aligned(8) class AVCDecoderConfigurationRecord {
	unsigned int(8) configurationVersion = 1;
	unsigned int(8) AVCProfileIndication;
	unsigned int(8) profile_compatibility;
	unsigned int(8) AVCLevelIndication;
	bit(6) reserved = ‘111111’b;
	unsigned int(2) lengthSizeMinusOne;
	bit(3) reserved = ‘111’b;
	unsigned int(5) numOfSequenceParameterSets;
	for (i=0; i< numOfSequenceParameterSets; i++) {
		unsigned int(16) sequenceParameterSetLength ;
		bit(8*sequenceParameterSetLength) sequenceParameterSetNALUnit;
	}
	unsigned int(8) numOfPictureParameterSets;
	for (i=0; i< numOfPictureParameterSets; i++) {
		unsigned int(16) pictureParameterSetLength;
		bit(8*pictureParameterSetLength) pictureParameterSetNALUnit;
	}
	if( profile_idc == 100  ||  profile_idc == 110  || profile_idc == 122  ||  profile_idc == 144 ){
		bit(6) reserved = ‘111111’b;
		unsigned int(2) chroma_format;
		bit(5) reserved = ‘11111’b;
		unsigned int(3) bit_depth_luma_minus8;
		bit(5) reserved = ‘11111’b;
		unsigned int(3) bit_depth_chroma_minus8;
		unsigned int(8) numOfSequenceParameterSetExt;
		for (i=0; i< numOfSequenceParameterSetExt; i++) {
			unsigned int(16) sequenceParameterSetExtLength;
			bit(8*sequenceParameterSetExtLength) sequenceParameterSetExtNALUnit;
		}
	}
}*/

void NBMp4AVCDecoderConfigurationRecord_init(STNBMp4AVCDecoderConfigurationRecord* obj){
	NBMemory_setZeroSt(*obj, STNBMp4AVCDecoderConfigurationRecord);
	obj->configurationVersion = 1;
	obj->reserved = obj->reserved1 = obj->reserved2 = obj->reserved3 = obj->reserved4 = 0xFF;
}

void NBMp4AVCDecoderConfigurationRecord_release(STNBMp4AVCDecoderConfigurationRecord* obj){
	//sps(s)
	if(obj->sequenceParameterSetNALUnits != NULL){
		UI8 i; for(i = 0; i < obj->numOfSequenceParameterSets; i++){
			STNBMp4Data* d = &obj->sequenceParameterSetNALUnits[i];
			NBMp4Data_release(d);
		}
		NBMemory_free(obj->sequenceParameterSetNALUnits);
		obj->sequenceParameterSetNALUnits = NULL;
		obj->numOfSequenceParameterSets = 0;
	}
	//pps(s)
	if(obj->pictureParameterSetNALUnits != NULL){
		UI8 i; for(i = 0; i < obj->numOfPictureParameterSets; i++){
			STNBMp4Data* d = &obj->pictureParameterSetNALUnits[i];
			NBMp4Data_release(d);
		}
		NBMemory_free(obj->pictureParameterSetNALUnits);
		obj->pictureParameterSetNALUnits = NULL;
		obj->numOfPictureParameterSets = 0;
	}
	//sps-ext(s)
	if(obj->sequenceParameterSetExtNALUnits != NULL){
		UI8 i; for(i = 0; i < obj->numOfSequenceParameterSetExt; i++){
			STNBMp4Data* d = &obj->sequenceParameterSetExtNALUnits[i];
			NBMp4Data_release(d);
		}
		NBMemory_free(obj->sequenceParameterSetExtNALUnits);
		obj->sequenceParameterSetExtNALUnits = NULL;
		obj->numOfSequenceParameterSetExt = 0;
	}
}

BOOL NBMp4AVCDecoderConfigurationRecord_loadFromBuff(STNBMp4AVCDecoderConfigurationRecord* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	if(!NBMp4_readBytes(&obj->configurationVersion, 1, buff, buffAfterEnd)){
		return FALSE;
	} else if(obj->configurationVersion != 1){
		return FALSE;
	}
	if(!NBMp4_readBytes(&obj->AVCProfileIndication, 1, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readBytes(&obj->profileCompatibility, 1, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readBytes(&obj->AVCLevelIndication, 1, buff, buffAfterEnd)){
		return FALSE;
	}
	{
		BYTE b = 0;
		if(!NBMp4_readBytes(&b, 1, buff, buffAfterEnd)){ //bit(6) ‘111111’b +  bit(2)
			return FALSE;
		} else {
			obj->reserved = (b & 0xFC);
			obj->lengthSizeMinusOne = (b & 0x3);
			NBASSERT(0xFC + 0x3 == 0xFF)
		}
	}
	//sps(s)
	{
		BYTE b = 0; UI8 arrSz = 0, reserved = 0;
		STNBMp4Data* arr = NULL;
		if(!NBMp4_readBytes(&b, 1, buff, buffAfterEnd)){ //bit(3) ‘111’b + bit(5)
			return FALSE;
		} else {
			UI8 i = 0; UI16 dataSz = 0;
			reserved = (b & 0xE0);
			arrSz = (b & 0x1F);
			NBASSERT(0xE0 + 0x1F == 0xFF)
			if(arrSz > 0){
				//load
				arr = NBMemory_allocType(STNBMp4Data);
				for(i = 0; i < arrSz; i++){
					if(!NBMp4_readUI16(&dataSz, buff, buffAfterEnd)){
						break;
					} else {
						STNBMp4Data* dd = &arr[i];
						NBMp4Data_init(dd);
						dd->use = dd->allocSz = dataSz;
						if(dd->allocSz > 0){
							dd->pay		= NBMemory_alloc(dd->allocSz);
							if(!NBMp4_readBytes(dd->pay, dd->allocSz, buff, buffAfterEnd)){
								NBMp4Data_release(dd);
								break;
							}
						}
					}
				}
			}
			//result
			if(i < arrSz){
				//error
				UI32 i2; for(i2 = 0; i2 < i; i2++){
					NBMp4Data_release(&arr[i2]);
				}
				NBMemory_free(arr);
				arr = NULL;
				return FALSE;
			}
		}
		//set sps(s)
		{
			if(obj->sequenceParameterSetNALUnits != NULL){
				UI8 i; for(i = 0; i < obj->numOfSequenceParameterSets; i++){
					STNBMp4Data* d = &obj->sequenceParameterSetNALUnits[i];
					NBMp4Data_release(d);
				}
				NBMemory_free(obj->sequenceParameterSetNALUnits);
			}
			obj->sequenceParameterSetNALUnits = arr;
			obj->numOfSequenceParameterSets = arrSz;
			obj->reserved1 = reserved;
		}
	}
	//pps(s)
	{
		UI8 arrSz = 0;
		STNBMp4Data* arr = NULL;
		if(!NBMp4_readBytes(&arrSz, 1, buff, buffAfterEnd)){
			return FALSE;
		} else {
			UI8 i = 0; UI16 dataSz = 0; 
			if(arrSz > 0){
				//load
				arr = NBMemory_allocType(STNBMp4Data);
				for(i = 0; i < arrSz; i++){
					if(!NBMp4_readUI16(&dataSz, buff, buffAfterEnd)){
						break;
					} else {
						STNBMp4Data* dd = &arr[i];
						NBMp4Data_init(dd);
						dd->use = dd->allocSz = dataSz;
						if(dd->allocSz > 0){
							dd->pay		= NBMemory_alloc(dd->allocSz);
							if(!NBMp4_readBytes(dd->pay, dd->allocSz, buff, buffAfterEnd)){
								NBMp4Data_release(dd);
								break;
							}
						}
					}
				}
			}
			//result
			if(i < arrSz){
				//error
				UI32 i2; for(i2 = 0; i2 < i; i2++){
					NBMp4Data_release(&arr[i2]);
				}
				NBMemory_free(arr);
				arr = NULL;
				return FALSE;
			}
		}
		//set sps(s)
		{
			if(obj->pictureParameterSetNALUnits != NULL){
				UI8 i; for(i = 0; i < obj->numOfPictureParameterSets; i++){
					STNBMp4Data* d = &obj->pictureParameterSetNALUnits[i];
					NBMp4Data_release(d);
				}
				NBMemory_free(obj->pictureParameterSetNALUnits);
			}
			obj->pictureParameterSetNALUnits = arr;
			obj->numOfPictureParameterSets = arrSz;
		}
	}
	//sps-ext(s)
	{
		UI8 arrSz = 0;
		STNBMp4Data* arr = NULL;
		if( obj->AVCProfileIndication == 100  ||  obj->AVCProfileIndication == 110  || obj->AVCProfileIndication == 122  ||  obj->AVCProfileIndication == 144 ){
			{
				BYTE b = 0;
				if(!NBMp4_readBytes(&b, 1, buff, buffAfterEnd)){ //bit(6) ‘111111’b +  bit(2)
					return FALSE;
				} else {
					obj->reserved2 = (b & 0xFC);
					obj->chromaFormat = (b & 0x3); 
					NBASSERT(0xFC + 0x3 == 0xFF)
				}
			}
			{
				BYTE b = 0;
				if(!NBMp4_readBytes(&b, 1, buff, buffAfterEnd)){ //bit(5) ‘111111’b +  bit(3)
					return FALSE;
				} else {
					obj->reserved3 = (b & 0xF8);
					obj->bitDepthLumaMinus8 = (b & 0x7);
					NBASSERT(0xF8 + 0x7 == 0xFF)
				}
			}
			{
				BYTE b = 0;
				if(!NBMp4_readBytes(&b, 1, buff, buffAfterEnd)){
					return FALSE;
				} else {
					obj->reserved4 = (b & 0xF8);
					obj->bitDepthChromaMinus8 = (b & 0x7); 
					NBASSERT(0xF8 + 0x7 == 0xFF)
				}
			}
			//sps-ext(s)
			if(!NBMp4_readBytes(&arrSz, 1, buff, buffAfterEnd)){
				return FALSE;
			} else {
				UI8 i = 0; UI16 dataSz = 0; 
				if(arrSz > 0){
					//load
					arr = NBMemory_allocType(STNBMp4Data);
					for(i = 0; i < arrSz; i++){
						if(!NBMp4_readUI16(&dataSz, buff, buffAfterEnd)){
							break;
						} else {
							STNBMp4Data* dd = &arr[i];
							NBMp4Data_init(dd);
							dd->use = dd->allocSz = dataSz;
							if(dd->allocSz > 0){
								dd->pay = NBMemory_alloc(dd->allocSz);
								if(!NBMp4_readBytes(dd->pay, dd->allocSz, buff, buffAfterEnd)){
									NBMp4Data_release(dd);
									break;
								}
							}
						}
					}
				}
				//result
				if(i < arrSz){
					//error
					UI32 i2; for(i2 = 0; i2 < i; i2++){
						NBMp4Data_release(&arr[i2]);
					}
					NBMemory_free(arr);
					arr = NULL;
					return FALSE;
				}
			}
		}
		//set sps-ext(s)
		{
			if(obj->sequenceParameterSetExtNALUnits != NULL){
				UI8 i; for(i = 0; i < obj->numOfSequenceParameterSetExt; i++){
					STNBMp4Data* d = &obj->sequenceParameterSetExtNALUnits[i];
					NBMp4Data_release(d);
				}
				NBMemory_free(obj->sequenceParameterSetExtNALUnits);
			}
			obj->sequenceParameterSetExtNALUnits = arr;
			obj->numOfSequenceParameterSetExt = arrSz;
		}
	}
	//
	return (*buff == buffAfterEnd);
}

void NBMp4AVCDecoderConfigurationRecord_concat(const STNBMp4AVCDecoderConfigurationRecord* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		{
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "configurationVersion("); NBString_concatUI32(dst, obj->configurationVersion); NBString_concat(dst, ")");
			NBString_concat(dst, " AVCProfileIndication("); NBString_concatUI32(dst, obj->AVCProfileIndication); NBString_concat(dst, ")");
			NBString_concat(dst, " profileCompatibility("); NBString_concatUI32(dst, obj->profileCompatibility); NBString_concat(dst, ")");
			NBString_concat(dst, " AVCLevelIndication("); NBString_concatUI32(dst, obj->AVCLevelIndication); NBString_concat(dst, ")");
			NBString_concat(dst, " lengthSizeMinusOne("); NBString_concatUI32(dst, obj->lengthSizeMinusOne); NBString_concat(dst, ")");
			NBString_concatByte(dst, '\n');
		}
		//sps(s)
		{
			UI8 i; for(i = 0; i < obj->numOfSequenceParameterSets; i++){
				const STNBMp4Data* d = &obj->sequenceParameterSetNALUnits[i];
				NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
				NBString_concat(dst, "spsNALU(0x"); NBString_concatBytesHex(dst, d->pay, d->use > 1024 ? 1024 : d->use); NBString_concat(dst, (d->use > 1024 ? "..., " : ", ")); NBString_concatUI32(dst, d->use); NBString_concat(dst, " bytes)");
				NBString_concatByte(dst, '\n');
			}
		}
		//pps(s)
		{
			UI8 i; for(i = 0; i < obj->numOfPictureParameterSets; i++){
				const STNBMp4Data* d = &obj->pictureParameterSetNALUnits[i];
				NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
				NBString_concat(dst, "ppsNALU(0x"); NBString_concatBytesHex(dst, d->pay, d->use > 1024 ? 1024 : d->use); NBString_concat(dst, (d->use > 1024 ? "..., " : ", ")); NBString_concatUI32(dst, d->use); NBString_concat(dst, " bytes)");
				NBString_concatByte(dst, '\n');
			}
		}
		if( obj->AVCProfileIndication == 100  ||  obj->AVCProfileIndication == 110  || obj->AVCProfileIndication == 122  ||  obj->AVCProfileIndication == 144 ){
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "chromaFormat("); NBString_concatUI32(dst, obj->chromaFormat); NBString_concat(dst, ")");
			NBString_concat(dst, " bitDepthLumaMinus8("); NBString_concatUI32(dst, obj->bitDepthLumaMinus8); NBString_concat(dst, ")");
			NBString_concat(dst, " bitDepthChromaMinus8("); NBString_concatUI32(dst, obj->bitDepthChromaMinus8); NBString_concat(dst, ")");
			NBString_concatByte(dst, '\n');
		}
		//sps-ext(s)
		{
			UI8 i; for(i = 0; i < obj->numOfSequenceParameterSetExt; i++){
				const STNBMp4Data* d = &obj->sequenceParameterSetExtNALUnits[i];
				NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
				NBString_concat(dst, "spsNALUExt(0x"); NBString_concatBytesHex(dst, d->pay, d->use > 1024 ? 1024 : d->use); NBString_concat(dst, (d->use > 1024 ? "..., " : ", ")); NBString_concatUI32(dst, d->use); NBString_concat(dst, " bytes)");
				NBString_concatByte(dst, '\n');
			}
		}
	}
}

BOOL NBMp4AVCDecoderConfigurationRecord_writeBits(STNBMp4AVCDecoderConfigurationRecord* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4_writeBytes(&obj->configurationVersion, 1, dst)){
		return FALSE;
	} else if(obj->configurationVersion != 1){
		return FALSE;
	}
	if(!NBMp4_writeBytes(&obj->AVCProfileIndication, 1, dst)){
		return FALSE;
	}
	if(!NBMp4_writeBytes(&obj->profileCompatibility, 1, dst)){
		return FALSE;
	}
	if(!NBMp4_writeBytes(&obj->AVCLevelIndication, 1, dst)){
		return FALSE;
	}
	{
		const BYTE b = (obj->reserved & 0xFC) | (obj->lengthSizeMinusOne & 0x3);
		if(!NBMp4_writeBytes(&b, 1, dst)){ //bit(6) ‘111111’b +  bit(2)
			return FALSE;
		}
	}
	//sps(s)
	{
		const BYTE b = (obj->reserved1 & 0xE0) | (obj->numOfSequenceParameterSets & 0x1F);
		if(!NBMp4_writeBytes(&b, 1, dst)){ //bit(3) ‘111’b + bit(5)
			return FALSE;
		} else if(obj->numOfSequenceParameterSets > 0 && obj->sequenceParameterSetNALUnits == NULL){
			return FALSE;
		} else if(obj->sequenceParameterSetNALUnits != NULL){
			UI8 i; for(i = 0; i < obj->numOfSequenceParameterSets; i++){
				const STNBMp4Data* d = &obj->sequenceParameterSetNALUnits[i];
				if(d->use > 0xFFFF){
					return FALSE;
				} else {
					const UI16 sz = (UI16)d->use;
					if(!NBMp4_writeUI16(&sz, dst)){
						return FALSE;
					} else if(!NBMp4_writeBytes(d->pay, d->use, dst)){
						return FALSE;
					}
				}
			}
		}
	}
	//pps(s)
	{
		const BYTE b = obj->numOfPictureParameterSets;
		if(!NBMp4_writeBytes(&b, 1, dst)){
			return FALSE;
		} else if(obj->numOfPictureParameterSets > 0 && obj->pictureParameterSetNALUnits == NULL){
			return FALSE;
		} else if(obj->pictureParameterSetNALUnits != NULL){
			UI8 i; for(i = 0; i < obj->numOfPictureParameterSets; i++){
				const STNBMp4Data* d = &obj->pictureParameterSetNALUnits[i];
				if(d->use > 0xFFFF){
					return FALSE;
				} else {
					const UI16 sz = (UI16)d->use;
					if(!NBMp4_writeUI16(&sz, dst)){
						return FALSE;
					} else if(!NBMp4_writeBytes(d->pay, d->use, dst)){
						return FALSE;
					}
				}
			}
		}
	}
	//sps-ext(s)
	{
		if( obj->AVCProfileIndication == 100  ||  obj->AVCProfileIndication == 110  || obj->AVCProfileIndication == 122  ||  obj->AVCProfileIndication == 144 ){
			{
				const BYTE b = (obj->reserved2 & 0xFC) | (obj->chromaFormat & 0x3);
				if(!NBMp4_writeBytes(&b, 1, dst)){ //bit(6) ‘111111’b +  bit(2)
					return FALSE;
				}
			}
			{
				const  BYTE b = (obj->reserved3 & 0xF8) | (obj->bitDepthLumaMinus8 & 0x7);
				if(!NBMp4_writeBytes(&b, 1, dst)){ //bit(5) ‘111111’b +  bit(3)
					return FALSE;
				}
			}
			{
				const BYTE b = (obj->reserved4 & 0xF8) | (obj->bitDepthChromaMinus8 & 0x7);
				if(!NBMp4_writeBytes(&b, 1, dst)){
					return FALSE;
				}
			}
			//sps-ext(s)
			if(!NBMp4_writeBytes(&obj->numOfSequenceParameterSetExt, 1, dst)){
				return FALSE;
			} else if(obj->numOfSequenceParameterSetExt > 0 && obj->sequenceParameterSetExtNALUnits == NULL){
				return FALSE;
			} else if(obj->sequenceParameterSetExtNALUnits != NULL){
				UI8 i; for(i = 0; i < obj->numOfSequenceParameterSetExt; i++){
					const STNBMp4Data* d = &obj->sequenceParameterSetExtNALUnits[i];
					if(d->use > 0xFFFF){
						return FALSE;
					} else {
						const UI16 sz = (UI16)d->use;
						if(!NBMp4_writeUI16(&sz, dst)){
							return FALSE;
						} else if(!NBMp4_writeBytes(d->pay, d->use, dst)){
							return FALSE;
						}
					}
				}
			}
		}
	}
	return TRUE;
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(AVCDecoderConfigurationRecord)

//NBMp4AVCConfigurationBox

/*class AVCConfigurationBox extends Box(‘avcC’) {
	AVCDecoderConfigurationRecord() AVCConfig;
}*/

void NBMp4AVCConfigurationBox_init(STNBMp4AVCConfigurationBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4AVCConfigurationBox);
	NBMp4Box_init(&obj->prntBox);
	//
	obj->prntBox.hdr.type.str[0] = 'a';
	obj->prntBox.hdr.type.str[1] = 'v';
	obj->prntBox.hdr.type.str[2] = 'c';
	obj->prntBox.hdr.type.str[3] = 'C';
	//
	NBMp4AVCDecoderConfigurationRecord_init(&obj->avcCfg);
}

void NBMp4AVCConfigurationBox_release(STNBMp4AVCConfigurationBox* obj){
	NBMp4AVCDecoderConfigurationRecord_release(&obj->avcCfg);
	NBMp4Box_release(&obj->prntBox);
}

BOOL NBMp4AVCConfigurationBox_loadFromBuff(STNBMp4AVCConfigurationBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	const BYTE* buffStart = *buff;
	if(!NBMp4AVCDecoderConfigurationRecord_loadFromBuff(&obj->avcCfg, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd)){
		return FALSE;
	}
	//
	NBMp4Box_cloneHdr(&obj->prntBox, prnt, boxFileStart, hdrFileSz, (UI32)(buffAfterEnd - buffStart), hdr);
	//
	return (*buff == buffAfterEnd);
}

void NBMp4AVCConfigurationBox_concat(const STNBMp4AVCConfigurationBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4Box_concatHdr(&obj->prntBox, depthLvl, optSrcFile, dst);
		//
		NBMp4AVCDecoderConfigurationRecord_concat(&obj->avcCfg, depthLvl + 1, optSrcFile, dst);
	}
}

BOOL NBMp4AVCConfigurationBox_writeBits(STNBMp4AVCConfigurationBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4Box_writeHdrBits(&obj->prntBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(!NBMp4AVCDecoderConfigurationRecord_writeBits(&obj->avcCfg, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(!NBMp4Box_writeHdrFinalSize(&obj->prntBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

void NBMp4AVCConfigurationBox_setAVCProfileIndication(STNBMp4AVCConfigurationBox* obj, const UI8 avcProfileIndication){
	obj->avcCfg.AVCProfileIndication = avcProfileIndication;
	
}

void NBMp4AVCConfigurationBox_setProfileCompatibility(STNBMp4AVCConfigurationBox* obj, const UI8 profileCompatibility){
	obj->avcCfg.profileCompatibility = profileCompatibility;
}

void NBMp4AVCConfigurationBox_setProfileLevelIndication(STNBMp4AVCConfigurationBox* obj, const UI8 profLevelIndication){
	obj->avcCfg.AVCLevelIndication = profLevelIndication;
}

BOOL NBMp4AVCConfigurationBox_setLengthSize(STNBMp4AVCConfigurationBox* obj, const UI8 lengthSize){ //bit(2) + 1
	BOOL r = FALSE;
	if(lengthSize >= 1 && lengthSize <= 0x3 + 1){
		obj->avcCfg.lengthSizeMinusOne = lengthSize - 1;
		r = TRUE;
	}
	return r;
}

void NBMp4AVCConfigurationBox_emptySps(STNBMp4AVCConfigurationBox* obj){
	if(obj->avcCfg.sequenceParameterSetNALUnits != NULL){
		UI16 i; for(i = 0; i < obj->avcCfg.numOfSequenceParameterSets; i++){
			STNBMp4Data* e = &obj->avcCfg.sequenceParameterSetNALUnits[i];
			NBMp4Data_release(e);
		}
		NBMemory_free(obj->avcCfg.sequenceParameterSetNALUnits);
		obj->avcCfg.sequenceParameterSetNALUnits = NULL;
	}
	obj->avcCfg.numOfSequenceParameterSets = 0;
}

BOOL NBMp4AVCConfigurationBox_addSps(STNBMp4AVCConfigurationBox* obj, const void* data, const UI16 size){
	BOOL r = FALSE;
	if(data != NULL && size > 0 && obj->avcCfg.numOfSequenceParameterSets < 0x1F){
		STNBMp4Data* nArr = NBMemory_allocTypes(STNBMp4Data, obj->avcCfg.numOfSequenceParameterSets + 1);
		if(obj->avcCfg.sequenceParameterSetNALUnits != NULL){
			if(obj->avcCfg.numOfSequenceParameterSets > 0){
				NBMemory_copy(nArr, obj->avcCfg.sequenceParameterSetNALUnits, sizeof(obj->avcCfg.sequenceParameterSetNALUnits[0]) * obj->avcCfg.numOfSequenceParameterSets);
			}
			NBMemory_free(obj->avcCfg.sequenceParameterSetNALUnits);
		}
		obj->avcCfg.sequenceParameterSetNALUnits = nArr;
		{
			STNBMp4Data* e = &obj->avcCfg.sequenceParameterSetNALUnits[obj->avcCfg.numOfSequenceParameterSets++];
			NBMp4Data_init(e);
			e->use	= e->allocSz = size;
			e->pay	= NBMemory_alloc(size);
			NBMemory_copy(e->pay, data, size);
		}
		r = TRUE;
	}
	return r;
}

BOOL NBMp4AVCConfigurationBox_updateSps(STNBMp4AVCConfigurationBox* obj, const UI16 idx, const void* data, const UI16 size){
	BOOL r = FALSE;
	if(data != NULL && size > 0 && idx < obj->avcCfg.numOfSequenceParameterSets){
		STNBMp4Data* e = &obj->avcCfg.sequenceParameterSetNALUnits[idx];
		//alloc (if necesary)
		if(size > e->allocSz){
			if(e->pay != NULL){
				NBMemory_free(e->pay);
			}
			e->pay = NBMemory_alloc(size);
			e->allocSz = size;
		}
		//populate
		NBMemory_copy(e->pay, data, size);
		e->use = size;
		//result
		r = TRUE;
	}
	return r;
}

void NBMp4AVCConfigurationBox_emptyPps(STNBMp4AVCConfigurationBox* obj){
	if(obj->avcCfg.pictureParameterSetNALUnits != NULL){
		UI16 i; for(i = 0; i < obj->avcCfg.numOfPictureParameterSets; i++){
			STNBMp4Data* e = &obj->avcCfg.pictureParameterSetNALUnits[i];
			NBMp4Data_release(e);
		}
		NBMemory_free(obj->avcCfg.pictureParameterSetNALUnits);
		obj->avcCfg.pictureParameterSetNALUnits = NULL;
	}
	obj->avcCfg.numOfPictureParameterSets = 0;
}

BOOL NBMp4AVCConfigurationBox_addPps(STNBMp4AVCConfigurationBox* obj, const void* data, const UI16 size){
	BOOL r = FALSE;
	if(data != NULL && size > 0 && obj->avcCfg.numOfPictureParameterSets < 0xFF){
		STNBMp4Data* nArr = NBMemory_allocTypes(STNBMp4Data, obj->avcCfg.numOfPictureParameterSets + 1);
		if(obj->avcCfg.pictureParameterSetNALUnits != NULL){
			if(obj->avcCfg.numOfPictureParameterSets > 0){
				NBMemory_copy(nArr, obj->avcCfg.pictureParameterSetNALUnits, sizeof(obj->avcCfg.pictureParameterSetNALUnits[0]) * obj->avcCfg.numOfPictureParameterSets);
			}
			NBMemory_free(obj->avcCfg.pictureParameterSetNALUnits);
		}
		obj->avcCfg.pictureParameterSetNALUnits = nArr;
		{
			STNBMp4Data* e = &obj->avcCfg.pictureParameterSetNALUnits[obj->avcCfg.numOfPictureParameterSets++];
			NBMp4Data_init(e);
			e->use	= e->allocSz = size;
			e->pay	= NBMemory_alloc(size);
			NBMemory_copy(e->pay, data, size);
		}
		r = TRUE;
	}
	return r;
}

BOOL NBMp4AVCConfigurationBox_updatePps(STNBMp4AVCConfigurationBox* obj, const UI16 idx, const void* data, const UI16 size){
	BOOL r = FALSE;
	if(data != NULL && size > 0 && idx < obj->avcCfg.numOfPictureParameterSets){
		STNBMp4Data* e = &obj->avcCfg.pictureParameterSetNALUnits[idx];
		//alloc (if necesary)
		if(size > e->allocSz){
			if(e->pay != NULL){
				NBMemory_free(e->pay);
			}
			e->pay = NBMemory_alloc(size);
			e->allocSz = size;
		}
		//populate
		NBMemory_copy(e->pay, data, size);
		e->use = size;
		//result
		r = TRUE;
	}
	return r;
}

BOOL NBMp4AVCConfigurationBox_setChromaFormat(STNBMp4AVCConfigurationBox* obj, const UI8 fmt){ //bit(2)
	BOOL r = FALSE;
	if(fmt <= 0x3){
		obj->avcCfg.chromaFormat = fmt;
		r = TRUE;
	}
	return r;
}

BOOL NBMp4AVCConfigurationBox_setBitDepthLuma(STNBMp4AVCConfigurationBox* obj, const UI8 bitDepth){ //bit(3) + 8
	BOOL r = FALSE;
	if(bitDepth >= 8 && bitDepth <= 0x7 + 8){
		obj->avcCfg.bitDepthLumaMinus8 = bitDepth - 8;
		r = TRUE;
	}
	return r;
}

BOOL NBMp4AVCConfigurationBox_setBitDepthChroma(STNBMp4AVCConfigurationBox* obj, const UI8 bitDepth){ //bit(3) + 8
	BOOL r = FALSE;
	if(bitDepth >= 8 && bitDepth <= 0x7 + 8){
		obj->avcCfg.bitDepthChromaMinus8 = bitDepth - 8;
		r = TRUE;
	}
	return r; 
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(AVCConfigurationBox)

//NBMp4MPEG4BitRateBox

/*class MPEG4BitRateBox extends Box(‘btrt’){
	unsigned int(32) bufferSizeDB;
	unsigned int(32) maxBitrate;
	unsigned int(32) avgBitrate;
}*/

void NBMp4MPEG4BitRateBox_init(STNBMp4MPEG4BitRateBox* obj){
	NBMemory_setZeroSt(*obj, STNBMp4MPEG4BitRateBox);
	NBMp4Box_init(&obj->prntBox);
	//
	obj->prntBox.hdr.type.str[0] = 'b';
	obj->prntBox.hdr.type.str[1] = 't';
	obj->prntBox.hdr.type.str[2] = 'r';
	obj->prntBox.hdr.type.str[3] = 't';
}

void NBMp4MPEG4BitRateBox_release(STNBMp4MPEG4BitRateBox* obj){
	NBMp4Box_release(&obj->prntBox);
}

BOOL NBMp4MPEG4BitRateBox_loadFromBuff(STNBMp4MPEG4BitRateBox* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	const BYTE* buffStart = *buff;
	if(!NBMp4_readUI32(&obj->bufferSizeDB, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->maxBitrate, buff, buffAfterEnd)){
		return FALSE;
	}
	if(!NBMp4_readUI32(&obj->avgBitrate, buff, buffAfterEnd)){
		return FALSE;
	}
	//
	NBMp4Box_cloneHdr(&obj->prntBox, prnt, boxFileStart, hdrFileSz, (UI32)(buffAfterEnd - buffStart), hdr);
	//
	return (*buff == buffAfterEnd);
}

void NBMp4MPEG4BitRateBox_concat(const STNBMp4MPEG4BitRateBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4Box_concatHdr(&obj->prntBox, depthLvl, optSrcFile, dst);
		//
		{
			NBString_concatRepeatedByte(dst, '\t', depthLvl + 1);
			NBString_concat(dst, "bufferSizeDB("); NBString_concatUI32(dst, obj->bufferSizeDB); NBString_concat(dst, ")");
			NBString_concat(dst, " maxBitrate("); NBString_concatUI32(dst, obj->maxBitrate); NBString_concat(dst, ")");
			NBString_concat(dst, " avgBitrate("); NBString_concatUI32(dst, obj->avgBitrate); NBString_concat(dst, ")");
			NBString_concatByte(dst, '\n');
		}
	}
}

BOOL NBMp4MPEG4BitRateBox_writeBits(STNBMp4MPEG4BitRateBox* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4Box_writeHdrBits(&obj->prntBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->bufferSizeDB, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->maxBitrate, dst)){
		return FALSE;
	}
	if(!NBMp4_writeUI32(&obj->avgBitrate, dst)){
		return FALSE;
	}
	if(!NBMp4Box_writeHdrFinalSize(&obj->prntBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(MPEG4BitRateBox)

/*class MPEG4ExtensionDescriptorsBox extends Box(‘m4ds’) {
	Descriptor Descr[0 .. 255];
}*/

//NBMp4AVCSampleEntry

/*class AVCSampleEntry() extends VisualSampleEntry (‘avc1’){
	AVCConfigurationBox config;
	MPEG4BitRateBox (); // optional
	MPEG4ExtensionDescriptorsBox (); // optional
}*/

void NBMp4AVCSampleEntry_init(STNBMp4AVCSampleEntry* obj){
	NBMemory_setZeroSt(*obj, STNBMp4AVCSampleEntry);
	NBMp4VisualSampleEntry_init(&obj->prntVisualEntry);
	//
	obj->prntVisualEntry.prntEntry.prntBox.hdr.type.str[0] = 'a';
	obj->prntVisualEntry.prntEntry.prntBox.hdr.type.str[1] = 'v';
	obj->prntVisualEntry.prntEntry.prntBox.hdr.type.str[2] = 'c';
	obj->prntVisualEntry.prntEntry.prntBox.hdr.type.str[3] = '1';
}

void NBMp4AVCSampleEntry_release(STNBMp4AVCSampleEntry* obj){
	if(obj->avcC != NULL){
		NBMp4AVCConfigurationBox_release(obj->avcC);
		NBMemory_free(obj->avcC);
		obj->avcC = NULL;
	}
	if(obj->btrt != NULL){
		NBMp4MPEG4BitRateBox_release(obj->btrt);
		NBMemory_free(obj->btrt);
		obj->btrt = NULL;
	}
	if(obj->m4ds != NULL){
		NBMp4Box_release(obj->m4ds);
		NBMemory_free(obj->m4ds);
		obj->m4ds = NULL;
	}
	NBMp4VisualSampleEntry_release(&obj->prntVisualEntry);
}

BOOL NBMp4AVCSampleEntry_loadFromBuff(STNBMp4AVCSampleEntry* obj, STNBMp4Box* prnt, const UI32 depthLvl, const UI64 boxFileStart, const UI8 hdrFileSz, const STNBMp4BoxHdr* hdr, const BYTE** buff, const BYTE* buffAfterEnd){
	const BYTE* buffStart = *buff;
	if(!NBMp4VisualSampleEntry_loadFromBuff(&obj->prntVisualEntry, prnt, depthLvl, boxFileStart, hdrFileSz, hdr, buff, buffAfterEnd)){
		return FALSE;
	}
	//avcC (required)
	{
		const STNBMp4BoxDef def = NBMP4_BOX_PARSEABLE_DEF( 'a', 'v', 'c', 'C', AVCConfigurationBox );
		STNBMp4Box* avcC = NBMp4Box_alloctIfMatchDef(&def, &obj->prntVisualEntry.prntEntry.prntBox, depthLvl + 1, boxFileStart + hdrFileSz + (*buff - buffStart), buff, buffAfterEnd);
		if(avcC == NULL){
			return FALSE; //required
		} else {
			if(obj->avcC != NULL){
				NBMp4AVCConfigurationBox_release(obj->avcC);
				NBMemory_free(obj->avcC);
			}
			obj->avcC = (STNBMp4AVCConfigurationBox*)avcC;
		}
	}
	//btrt (optional)
	{
		const STNBMp4BoxDef def = NBMP4_BOX_PARSEABLE_DEF( 'b', 't', 'r', 't', MPEG4BitRateBox );
		STNBMp4Box* btrt = NBMp4Box_alloctIfMatchDef(&def, &obj->prntVisualEntry.prntEntry.prntBox, depthLvl + 1, boxFileStart + hdrFileSz + (*buff - buffStart), buff, buffAfterEnd);
		if(btrt != NULL){
			if(obj->btrt != NULL){
				NBMp4MPEG4BitRateBox_release(obj->btrt);
				NBMemory_free(obj->btrt);
			}
			obj->btrt = (STNBMp4MPEG4BitRateBox*)btrt;
		}
	}
	//m4ds (optional)
	{
		const STNBMp4BoxDef def = NBMP4_BOX_PARSEABLE_FILE_PREFERED_DEF( 'm', '4', 'd', 's' );
		STNBMp4Box* m4ds = NBMp4Box_alloctIfMatchDef(&def, &obj->prntVisualEntry.prntEntry.prntBox, depthLvl + 1, boxFileStart + hdrFileSz + (*buff - buffStart), buff, buffAfterEnd);
		if(m4ds != NULL){
			if(obj->m4ds != NULL){
				NBMp4Box_release(obj->m4ds);
				NBMemory_free(obj->m4ds);
			}
			obj->m4ds = m4ds;
		}
	}
	return TRUE; //abstract
}

void NBMp4AVCSampleEntry_concat(const STNBMp4AVCSampleEntry* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(dst != NULL){
		NBMp4VisualSampleEntry_concat(&obj->prntVisualEntry, depthLvl, optSrcFile, dst);
		//avcC (required)
		if(obj->avcC != NULL){
			NBMp4AVCConfigurationBox_concat(obj->avcC, depthLvl + 1, optSrcFile, dst);
		}
		//btrt (optional)
		if(obj->btrt != NULL){
			NBMp4MPEG4BitRateBox_concat(obj->btrt, depthLvl + 1, optSrcFile, dst);
		}
		//m4ds (optional)
		if(obj->m4ds != NULL){
			NBMp4Box_concat(obj->m4ds, depthLvl + 1, optSrcFile, dst);
		}
	}
}

BOOL NBMp4AVCSampleEntry_writeBits(STNBMp4AVCSampleEntry* obj, const UI32 depthLvl, STNBFileRef optSrcFile, STNBString* dst){
	if(!NBMp4VisualSampleEntry_writeBits(&obj->prntVisualEntry, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	//avcC (required)
	if(obj->avcC != NULL){
		NBMp4AVCConfigurationBox_writeBits(obj->avcC, depthLvl + 1, optSrcFile, dst);
	}
	//btrt (optional)
	if(obj->btrt != NULL){
		NBMp4MPEG4BitRateBox_writeBits(obj->btrt, depthLvl + 1, optSrcFile, dst);
	}
	//m4ds (optional)
	if(obj->m4ds != NULL){
		NBMp4Box_writeBits(obj->m4ds, depthLvl + 1, optSrcFile, dst);
	}
	//
	if(!NBMp4Box_writeHdrFinalSize(&obj->prntVisualEntry.prntEntry.prntBox, depthLvl, optSrcFile, dst)){
		return FALSE;
	}
	return TRUE;
}

void NBMp4AVCSampleEntry_setDataRefIndex(STNBMp4AVCSampleEntry* obj, const UI16 idx){
	NBMp4SampleEntry_setDataRefIndex(&obj->prntVisualEntry.prntEntry, idx);
}

void NBMp4AVCSampleEntry_setWidth(STNBMp4AVCSampleEntry* obj, const UI16 width){
	NBMp4VisualSampleEntry_setWidth(&obj->prntVisualEntry, width);
}

void NBMp4AVCSampleEntry_setHeight(STNBMp4AVCSampleEntry* obj, const UI16 height){
	NBMp4VisualSampleEntry_setHeight(&obj->prntVisualEntry, height);
}

void NBMp4AVCSampleEntry_setFrameCount(STNBMp4AVCSampleEntry* obj, const UI16 frameCount){
	NBMp4VisualSampleEntry_setFrameCount(&obj->prntVisualEntry, frameCount);
}

void NBMp4AVCSampleEntry_setCompressorName(STNBMp4AVCSampleEntry* obj, const char* name31Max){
	NBMp4VisualSampleEntry_setCompressorName(&obj->prntVisualEntry, name31Max);
}

void NBMp4AVCSampleEntry_setDepth(STNBMp4AVCSampleEntry* obj, const UI16 depth){
	NBMp4VisualSampleEntry_setDepth(&obj->prntVisualEntry, depth);
}

BOOL NBMp4AVCSampleEntry_setClapByOwning(STNBMp4AVCSampleEntry* obj, STNBMp4BoxRef* ref){
	return NBMp4VisualSampleEntry_setClapByOwning(&obj->prntVisualEntry, ref);
}

BOOL NBMp4AVCSampleEntry_setPaspByOwning(STNBMp4AVCSampleEntry* obj, STNBMp4BoxRef* ref){
	return NBMp4VisualSampleEntry_setPaspByOwning(&obj->prntVisualEntry, ref);
}

BOOL NBMp4AVCSampleEntry_setAvcCByOwning(STNBMp4AVCSampleEntry* obj, STNBMp4BoxRef* ref){
	BOOL r = FALSE;
	if(ref == NULL || ref->box == NULL){
		if(obj->avcC != NULL){
			NBMp4AVCConfigurationBox_release(obj->avcC);
			NBMemory_free(obj->avcC);
			obj->avcC = NULL;
		}
		r = TRUE;
	} else if(ref->box->hdr.type.str[0] == 'a' && ref->box->hdr.type.str[1] == 'v' && ref->box->hdr.type.str[2] == 'c' && ref->box->hdr.type.str[3] == 'C'){
		if(obj->avcC != NULL){
			NBMp4AVCConfigurationBox_release(obj->avcC);
			NBMemory_free(obj->avcC);
			obj->avcC = NULL;
		}
		obj->avcC = (STNBMp4AVCConfigurationBox*)ref->box;
		ref->box = NULL;
		r = TRUE;
	}
	return r;
}

BOOL NBMp4AVCSampleEntry_setBtrtByOwning(STNBMp4AVCSampleEntry* obj, STNBMp4BoxRef* ref){
	BOOL r = FALSE;
	if(ref == NULL || ref->box == NULL){
		if(obj->btrt != NULL){
			NBMp4MPEG4BitRateBox_release(obj->btrt);
			NBMemory_free(obj->btrt);
			obj->btrt = NULL;
		}
		r = TRUE;
	} else if(ref->box->hdr.type.str[0] == 'b' && ref->box->hdr.type.str[1] == 't' && ref->box->hdr.type.str[2] == 'r' && ref->box->hdr.type.str[3] == 't'){
		if(obj->btrt != NULL){
			NBMp4MPEG4BitRateBox_release(obj->btrt);
			NBMemory_free(obj->btrt);
			obj->btrt = NULL;
		}
		obj->btrt = (STNBMp4MPEG4BitRateBox*)ref->box;
		ref->box = NULL;
		r = TRUE;
	}
	return r;
}

NB_MP4_FULL_BOX_FUNCS_ITFS_BODY(AVCSampleEntry)

//------------
// Definitions
//------------
STNBMp4BoxDef __boxesDefs[] = {
	//--------------------
	//parseable full-boxes
	//--------------------
	NBMP4_BOX_PARSEABLE_DEF( 'f', 't', 'y', 'p' , FileTypeBox )
	, NBMP4_BOX_PARSEABLE_DEF( 'm', 'v', 'h', 'd', MovieHeaderBox )
	, NBMP4_BOX_PARSEABLE_DEF( 'm', 'd', 'h', 'd', MediaHeaderBox )
	, NBMP4_BOX_PARSEABLE_DEF( 't', 'k', 'h', 'd', TrackHeaderBox )
	, NBMP4_BOX_PARSEABLE_DEF( 'h', 'd', 'l', 'r', HandlerBox )
	, NBMP4_BOX_PARSEABLE_DEF( 'd', 'r', 'e', 'f', DataReferenceBox )
	, NBMP4_BOX_PARSEABLE_DEF( 's', 't', 's', 'z', SampleSizeBox )
	, NBMP4_BOX_PARSEABLE_DEF( 'v', 'm', 'h', 'd', VideoMediaHeaderBox )
	, NBMP4_BOX_PARSEABLE_DEF( 's', 't', 's', 'd', SampleDescriptionBox )
	, NBMP4_BOX_PARSEABLE_DEF( 's', 't', 't', 's', TimeToSampleBox )
	, NBMP4_BOX_PARSEABLE_DEF( 's', 't', 's', 'c', SampleToChunkBox )
	, NBMP4_BOX_PARSEABLE_DEF( 's', 't', 'c', 'o', ChunkOffsetBox )
	, NBMP4_BOX_PARSEABLE_DEF( 'c', 'o', '6', '4', ChunkOffsetBox64 )
	, NBMP4_BOX_PARSEABLE_DEF( 't', 'r', 'e', 'x', TrackExtendsBox )
	, NBMP4_BOX_PARSEABLE_DEF( 'm', 'f', 'h', 'd', MovieFragmentHeaderBox )
	, NBMP4_BOX_PARSEABLE_DEF( 't', 'f', 'h', 'd', TrackFragmentHeaderBox )
	, NBMP4_BOX_PARSEABLE_DEF( 't', 'f', 'd', 't', TrackFragmentBaseMediaDecodeTimeBox )
	, NBMP4_BOX_PARSEABLE_DEF( 't', 'r', 'u', 'n', TrackRunBox )
	, NBMP4_BOX_PARSEABLE_DEF( 'b', 't', 'r', 't', BitRateBox )
	, NBMP4_BOX_PARSEABLE_DEF( 'p', 'a', 's', 'p', PixelAspectRatioBox )
	, NBMP4_BOX_PARSEABLE_DEF( 'c', 'l', 'a', 'p', CleanApertureBox )
	//ISO-IEC-14496-15
	, NBMP4_BOX_PARSEABLE_DEF( 'b', 't', 'r', 't', MPEG4BitRateBox )
	, NBMP4_BOX_PARSEABLE_DEF( 'a', 'v', 'c', 'C', AVCConfigurationBox )
	, NBMP4_BOX_PARSEABLE_DEF( 'a', 'v', 'c', '1', AVCSampleEntry )
	//-------------
	//file-prefered full-boxes (avoid memory loading if posible)
	//-------------
	, NBMP4_BOX_PARSEABLE_FILE_PREFERED_DEF( 'm', 'd', 'a', 't' )
	//----------
	//containers
	//----------
	, NBMP4_BOX_CONTAINER_DEF( 's', 't', 'b', 'l', SampleTableBox )
	, NBMP4_BOX_CONTAINER_DEF( 'm', 'i', 'n', 'f', MediaInformationBox )
	, NBMP4_BOX_CONTAINER_DEF( 'm', 'd', 'i', 'a', MediaBox )
	, NBMP4_BOX_CONTAINER_DEF( 't', 'r', 'a', 'k', TrackBox )
	, NBMP4_BOX_CONTAINER_DEF( 'm', 'o', 'o', 'v', MovieBox )
	, NBMP4_BOX_CONTAINER_DEF( 't', 'r', 'a', 'f', TrackFragmentBox )
	, NBMP4_BOX_CONTAINER_DEF( 'm', 'v', 'e', 'x', MovieExtendsBox )
	, NBMP4_BOX_CONTAINER_DEF( 'm', 'o', 'o', 'f', MovieFragmentBox )
	, NBMP4_BOX_CONTAINER_DEF( 'd', 'i', 'n', 'f', DataInformationBox )
};

const STNBMp4BoxDef* NBMp4_getBoxDef(const BYTE id4[4]){
	const STNBMp4BoxDef* r = NULL;
	const UI32 idUi32 = *((const UI32*)id4);
	const UI32 count = (sizeof(__boxesDefs) / sizeof(__boxesDefs[0]));
	const STNBMp4BoxDef* d = __boxesDefs;
	const STNBMp4BoxDef* dAfterEnd = __boxesDefs + count;
	while(d < dAfterEnd){
		if(d->boxId.ui32 == idUi32){
			r = d;
			break;
		}
		d++;
	}
	return r;
}
