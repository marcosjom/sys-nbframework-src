// NBBase64.h: interface for the NBBase64 class.
//
//////////////////////////////////////////////////////////////////////

#ifndef NB_LZ4_H
#define NB_LZ4_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	//Read
	BOOL NBLz4_inflate(const void* data, const UI32 dataSz, const UI32 uncompressSz, STNBString* dst);

	//Write
	BOOL NBLz4_deflate(const void* data, const UI32 dataSz, STNBString* dst);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
