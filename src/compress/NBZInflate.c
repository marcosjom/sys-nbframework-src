//
//  NBZlib.c
//  nbframework
//
//  Created by Marcos Ortega on 19/3/19.
//

#include "nb/NBFrameworkPch.h"
#include "nb/compress/NBZInflate.h"
//
#include "nb/core/NBMemory.h"
#include "zlib.h"

typedef struct STNBZInflateOpq_ {
	BOOL		zsInited;
	z_stream	zs;
} STNBZInflateOpq;

void NBZInflate_init(STNBZInflate* obj){
	STNBZInflateOpq* opq = obj->opaque = NBMemory_allocType(STNBZInflateOpq);
	NBMemory_setZeroSt(*opq, STNBZInflateOpq);
	opq->zsInited	= FALSE;
}

void NBZInflate_release(STNBZInflate* obj){
	STNBZInflateOpq* opq = (STNBZInflateOpq*)obj->opaque;
	if(opq != NULL){
		if(opq->zsInited){
			inflateEnd(&opq->zs);
			opq->zsInited = FALSE;
		}
		NBMemory_free(obj->opaque);
		obj->opaque = NULL;
	}
}


BOOL NBZInflate_feedStart(STNBZInflate* obj){
	BOOL r = FALSE;
	STNBZInflateOpq* opq = (STNBZInflateOpq*)obj->opaque;
	if(opq != NULL){
		opq->zs.next_in		= NULL;
		opq->zs.avail_in	= 0;
		opq->zs.zalloc		= (alloc_func)0;
		opq->zs.zfree		= (free_func)0;
		opq->zs.opaque		= (voidpf)0;
		if(opq->zsInited){
			if(inflateReset(&opq->zs) == Z_OK){
				r = TRUE;
			}
			NBASSERT(r)
		} else {
			if(inflateInit(&opq->zs) == Z_OK){
				opq->zsInited = TRUE;
				r = TRUE;
			}
			NBASSERT(r)
		}
	}
	return r;
}

ENZInfResult NBZInflate_feed(STNBZInflate* obj, void* dst, const UI32 dstSz, const void* src, const UI32 srcSz, const ENZInfBlckType blckType){
	ENZInfResult r;
	r.resultCode		= Z_BUF_ERROR;
	r.inBytesProcessed	= 0;
	r.outBytesProcessed	= 0;
	r.inBytesAvailable	= 0;
	r.outBytesAvailable	= 0;
	{
		STNBZInflateOpq* opq = (STNBZInflateOpq*)obj->opaque;
		if(opq != NULL){
			SI32 avail_inIni	= 0;
			if(src != NULL){
				opq->zs.next_in	= (Bytef*)src;
				opq->zs.avail_in = (uInt)srcSz;
			}
			avail_inIni			= opq->zs.avail_in;
			opq->zs.next_out	= (Bytef*)dst;
			opq->zs.avail_out	= (uInt)dstSz;
			r.resultCode		= inflate(&opq->zs, blckType == ENZInfBlckType_Final ? Z_FINISH : Z_NO_FLUSH /*Z_NO_FLUSH Z_SYNC_FLUSH, Z_BLOCK*/);
			r.inBytesAvailable	= opq->zs.avail_in;
			r.outBytesAvailable	= opq->zs.avail_out;
			r.inBytesProcessed	= avail_inIni - opq->zs.avail_in;
			r.outBytesProcessed	= dstSz - opq->zs.avail_out;
			/*{
				switch(r.resultCode){
						//Error codes
					case Z_VERSION_ERROR: PRINTF_ERROR("NBZInflate_feed: Z_VERSION_ERROR(%d).\n", r.resultCode); break;
					case Z_BUF_ERROR: PRINTF_ERROR("NBZInflate_feed: Z_BUF_ERROR(%d).\n", r.resultCode); break;
					case Z_MEM_ERROR: PRINTF_ERROR("NBZInflate_feed: Z_MEM_ERROR(%d).\n", r.resultCode); break;
					case Z_DATA_ERROR: PRINTF_ERROR("NBZInflate_feed: Z_DATA_ERROR(%d).\n", r.resultCode); break;
					case Z_STREAM_ERROR: PRINTF_ERROR("NBZInflate_feed: Z_STREAM_ERROR(%d).\n", r.resultCode); break;
					case Z_ERRNO: PRINTF_ERROR("NBZInflate_feed: Z_ERRNO(%d).\n", r.resultCode); break;
						//Retun codes
					case Z_NEED_DICT: PRINTF_INFO("NBZInflate_feed: Z_NEED_DICT(%d).\n", r.resultCode); break;
					case Z_STREAM_END: PRINTF_INFO("NBZInflate_feed: Z_STREAM_END(%d).\n", r.resultCode); break;
					case Z_OK: PRINTF_INFO("NBZInflate_feed: Z_OK(%d).\n", r.resultCode); break;
					default: PRINTF_INFO("NBZInflate_feed: ??? (%d).\n", r.resultCode); break;
				}
			}*/
		}
	}
	return r;
}
