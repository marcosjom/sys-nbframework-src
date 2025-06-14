//
//  NBPng.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/11/18.
//

#ifndef NBPngChunk_h
#define NBPngChunk_h

#include "nb/NBFrameworkDefs.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBPngChunk_ {
		char	name[5];
		void*	data;
		UI32	dataSz;
	} STNBPngChunk;
	
	void NBPngChunk_init(STNBPngChunk* obj);
	void NBPngChunk_release(STNBPngChunk* obj);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBPng_h */
