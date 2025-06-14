// NBBase64.c: implementation of the NBBase64 class.
//
//////////////////////////////////////////////////////////////////////

#include "nb/NBFrameworkPch.h"
#include "nb/compress/NBLz4.h"
#include "nb/core/NBMemory.h"

#include "lz4.h"

//Read
BOOL NBLz4_inflate(const void* data, const UI32 dataSz, const UI32 uncompressSz, STNBString* dst){
	BOOL r = FALSE;
	if(dataSz <= 0){
		r = TRUE;
	} else if(uncompressSz > 0){
		const int buffSz	= (const int)uncompressSz;
		char* buff			= (char*)NBMemory_alloc(uncompressSz);
		const int written	= LZ4_decompress_safe((const char*)data, buff, (int)dataSz, buffSz);
		if(written <= 0){
			r = FALSE;
		} else {
			if(dst != NULL){
				if(dst->length <= 0){
					//Swap buffer
					NBString_swapContentBytes(dst, buff, written, (UI32)buffSz);
				} else {
					//Concat buffer
					NBString_concatBytes(dst, buff, written);
				}
			}
			r = TRUE;
		}
		NBMemory_free(buff);
		buff = NULL;
	}
	return r;
}

//Write
BOOL NBLz4_deflate(const void* data, const UI32 dataSz, STNBString* dst){
	BOOL r = FALSE;
	if(dataSz <= 0){
		r = TRUE;
	} else {
		const int buffSz	= LZ4_compressBound(dataSz);
		char* buff			= (char*)NBMemory_alloc(buffSz);
		const int written	= LZ4_compress_default((const char*)data, buff, (int)dataSz, buffSz);
		if(written <= 0){
			r = FALSE;
		} else {
			if(dst != NULL){
				if(dst->length <= 0){
					//Swap buffer
					NBString_swapContentBytes(dst, buff, written, (UI32)buffSz);
				} else {
					//Concat buffer
					NBString_concatBytes(dst, buff, written);
				}
			}
			r = TRUE;
		}
		NBMemory_free(buff);
		buff = NULL;
	}
	return r;
}
