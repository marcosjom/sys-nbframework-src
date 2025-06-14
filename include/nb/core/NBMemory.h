//
//  NBMemory.h
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#ifndef NB_MEMORY_H
#define NB_MEMORY_H

#include "nb/NBFrameworkDefs.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const UI32 _nbZeroesArr[128];

#ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
#	define NBMemory_alloc(BYTES)			NBMemory_alloc_((UI32)(BYTES), __FILE__, __LINE__, __func__)
#	define NBMemory_allocType(TYPE)			(TYPE*)NBMemory_alloc_((UI32)sizeof(TYPE), __FILE__, __LINE__, __func__)
#	define NBMemory_allocTypes(TYPE, AMM)	(TYPE*)NBMemory_alloc_((UI32)(sizeof(TYPE) * (AMM)), __FILE__, __LINE__, __func__)
#	define NBMemory_free(PTR)				NBMemory_free_(PTR, __FILE__, __LINE__, __func__)
#else
#	define NBMemory_alloc(BYTES)			NBMemory_alloc_((UI32)(BYTES))
#	define NBMemory_allocType(TYPE)			(TYPE*)NBMemory_alloc_((UI32)sizeof(TYPE))
#	define NBMemory_allocTypes(TYPE, AMM)	(TYPE*)NBMemory_alloc_((UI32)(sizeof(TYPE) * (UI64)(AMM)))
#	define NBMemory_free(PTR)				NBMemory_free_(PTR)
#endif

#define NBMemory_set(PTR, V, SZ)			NBMemory_set_(PTR, V, (UI32)(SZ))
#define NBMemory_setZero(VAR)				NBMemory_set_(&(VAR), 0, sizeof(VAR))
#define NBMemory_copy(DST, SRC, SZ)			NBMemory_cpy_(DST, SRC, (UI32)(SZ))     //memory can't overlap (user responsability)
#define NBMemory_move(DST, SRC, SZ)         NBMemory_move_(DST, SRC, (UI32)(SZ))    //memory can overlap
#define NBMemory_setZeroSt(VAR, TYPE)		{ \
												/*Note: a compiler optimization should and remove the conditional*/ \
												NBASSERT(sizeof(TYPE) == sizeof(VAR)) \
												if(sizeof(TYPE) == sizeof(VAR) && sizeof(VAR) <= sizeof(_nbZeroesArr)){ \
													VAR = *((TYPE*)_nbZeroesArr); \
												} else { \
													NBMemory_setZero(VAR); \
												} \
											}

#ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
void* NBMemory_alloc_(const UI32 sz, const char* fullpath, const SI32 line, const char* func);
void  NBMemory_free_(void* ptr, const char* fullpath, const SI32 line, const char* func);
#else
void* NBMemory_alloc_(const UI32 sz);
void  NBMemory_free_(void* ptr);
#endif

void* NBMemory_allocUnmanaged(const UI32 sz);
void  NBMemory_freeUnmanaged(void* ptr);

void NBMemory_set_(void* ptr, SI32 v, UI32 sz);
void NBMemory_cpy_(void* dst, const void* src, UI32 sz);    //memory can't overlap (user responsability)
void NBMemory_move_(void* dst, const void* src, UI32 sz);   //memory can overlap

#ifdef __cplusplus
} //extern "C"
#endif

#endif
