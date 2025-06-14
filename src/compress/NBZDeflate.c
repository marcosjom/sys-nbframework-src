//
//  NBZlib.c
//  nbframework
//
//  Created by Marcos Ortega on 19/3/19.
//

#include "nb/NBFrameworkPch.h"
#include "nb/compress/NBZDeflate.h"
//
#include "nb/core/NBMemory.h"
#include "zlib.h"

typedef struct STNBZDeflateOpq_ {
	SI8			compLv9;
	BOOL		zsInited;
	z_stream	zs;
} STNBZDeflateOpq;

void NBZDeflate_init(STNBZDeflate* obj){
	STNBZDeflateOpq* opq = obj->opaque = NBMemory_allocType(STNBZDeflateOpq);
	NBMemory_setZeroSt(*opq, STNBZDeflateOpq);
	opq->compLv9	= -1;
	opq->zsInited	= FALSE;
}

void NBZDeflate_release(STNBZDeflate* obj){
	STNBZDeflateOpq* opq = (STNBZDeflateOpq*)obj->opaque;
	if(opq != NULL){
		if(opq->zsInited){
			deflateEnd(&opq->zs);
			opq->zsInited = FALSE;
		}
		NBMemory_free(obj->opaque);
		obj->opaque = NULL;
	}
}


BOOL NBZDeflate_feedStart(STNBZDeflate* obj, const SI8 pCompLv9){
	BOOL r = FALSE;
	STNBZDeflateOpq* opq = (STNBZDeflateOpq*)obj->opaque;
	if(opq != NULL){
		const SI8 compLv9 = (pCompLv9 < 0 ? 0 : pCompLv9 > 9 ? 9 : pCompLv9);
		if(opq->zsInited && opq->compLv9 == compLv9){
			//Reset current
			opq->zs.next_in		= NULL;
			opq->zs.avail_in	= 0;
			opq->zs.zalloc		= (alloc_func)0;
			opq->zs.zfree		= (free_func)0;
			opq->zs.opaque		= (voidpf)0;
			if(deflateReset(&opq->zs) == Z_OK){
				r = TRUE;
			} NBASSERT(r)
		} else {
			//Release previous
			if(opq->zsInited){
				deflateEnd(&opq->zs);
				opq->zsInited = FALSE;
			}
			//New stream
			{
				opq->zs.next_in		= NULL;
				opq->zs.avail_in	= 0;
				opq->zs.zalloc		= (alloc_func)0;
				opq->zs.zfree		= (free_func)0;
				opq->zs.opaque		= (voidpf)0;
				if(deflateInit(&opq->zs, compLv9) == Z_OK){
					opq->zsInited	= TRUE;
					opq->compLv9	= compLv9;
					r = TRUE;
				} NBASSERT(r)
			}
		}
	}
	return r;
}

ENZDefResult NBZDeflate_feed(STNBZDeflate* obj, void* dst, const UI32 dstSz, const void* src, const UI32 srcSz, const ENZDefBlckType blckType){
	ENZDefResult r;
	r.resultCode		= Z_BUF_ERROR;
	r.inBytesProcessed	= 0;
	r.outBytesProcessed	= 0;
	r.inBytesAvailable	= 0;
	r.outBytesAvailable	= 0;
	STNBZDeflateOpq* opq = (STNBZDeflateOpq*)obj->opaque;
	if(opq != NULL){
		SI32 avail_inIni	= 0;
		if(src != NULL){
			opq->zs.next_in	= (Bytef*)src;
			opq->zs.avail_in= (uInt)srcSz;
		}
		avail_inIni			= opq->zs.avail_in;
		opq->zs.next_out	= (Bytef*)dst;
		opq->zs.avail_out	= (uInt)dstSz;
		r.resultCode		= deflate(&opq->zs, blckType == ENZDefBlckType_Final ? Z_FINISH : Z_NO_FLUSH /*Z_NO_FLUSH Z_SYNC_FLUSH, Z_BLOCK*/);
		r.inBytesAvailable	= opq->zs.avail_in;
		r.outBytesAvailable	= opq->zs.avail_out;
		r.inBytesProcessed	= avail_inIni - opq->zs.avail_in;
		r.outBytesProcessed	= dstSz - opq->zs.avail_out;
		/*{
			switch(r.resultCode){
					//Error codes
				case Z_VERSION_ERROR: PRINTF_ERROR("NBZDeflate_feed: Z_VERSION_ERROR(%d).\n", r.resultCode); break;
				case Z_BUF_ERROR: PRINTF_ERROR("NBZDeflate_feed: Z_BUF_ERROR(%d).\n", r.resultCode); break;
				case Z_MEM_ERROR: PRINTF_ERROR("NBZDeflate_feed: Z_MEM_ERROR(%d).\n", r.resultCode); break;
				case Z_DATA_ERROR: PRINTF_ERROR("NBZDeflate_feed: Z_DATA_ERROR(%d).\n", r.resultCode); break;
				case Z_STREAM_ERROR: PRINTF_ERROR("NBZDeflate_feed: Z_STREAM_ERROR(%d).\n", r.resultCode); break;
				case Z_ERRNO: PRINTF_ERROR("NBZDeflate_feed: Z_ERRNO(%d).\n", r.resultCode); break;
					//Retun codes
				case Z_NEED_DICT: PRINTF_INFO("NBZDeflate_feed: Z_NEED_DICT(%d).\n", r.resultCode); break;
				case Z_STREAM_END: PRINTF_INFO("NBZDeflate_feed: Z_STREAM_END(%d).\n", r.resultCode); break;
				case Z_OK: PRINTF_INFO("NBZDeflate_feed: Z_OK(%d).\n", r.resultCode); break;
				default: PRINTF_INFO("NBZDeflate_feed: ??? (%d).\n", r.resultCode); break;
			}
		}*/
	}
	return r;
}

