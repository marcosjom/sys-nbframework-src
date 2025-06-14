//
//  NBCrc32.h
//  SWF_SDK
//
//  Created by Marcos Ortega on 23/07/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef NB_CRC32_H
#define NB_CRC32_H

#include "nb/NBFrameworkDefs.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	//Hash hexadecimal string
	typedef struct STNBCrc32HashHex_ {
		char v[9];
	} STNBCrc32HashHex;
	//
	typedef struct STNBCrc32_ {
		UI32	hash;
		UI32	totalBytesFed;
	} STNBCrc32;
	//Factory
	void				NBCrc32_init(STNBCrc32* obj);
	void				NBCrc32_release(STNBCrc32* obj);
	//Feed
	void				NBCrc32_feed(STNBCrc32* obj, const void* data, const UI32 dataSz);
	void				NBCrc32_finish(STNBCrc32* obj);
	STNBCrc32HashHex	NBCrc32_getHashHex(const STNBCrc32* v);
	void				NBCrc32_reset(STNBCrc32* obj);
	//One step calculation
	UI32				NBCrc32_getHashBytes(const void* data, const UI32 dataSz);
	STNBCrc32HashHex	NBCrc32_getHashBytesHex(const void* data, const UI32 dataSz);
	//Helpers
	void				NBCrc32_hashBinToHexChars(const UI32 hash, char* dst9Chars);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
