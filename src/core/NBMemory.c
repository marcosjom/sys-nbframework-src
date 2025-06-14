//
//  XUCadena.c
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBMemory.h"

//Complementary PCH
#ifdef _WIN32
#	ifdef NB_COMPILE_DRIVER_MODE
//#		include <wdm.h> //for 'ExAllocatePoolWithTag', 'ExFreePoolWithTag'
#		include <ntddk.h> //for 'ExAllocatePoolWithTag', 'ExFreePoolWithTag'
#	else
#		include <stdlib.h>	//for "malloc", "free", "size_t" y rand()
#	endif
#else
#	include <stdlib.h>	//for "malloc", "free", "size_t" y rand()
#	include <string.h>	//for memset(), memcpy()
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
#	include "nb/core/NBMngrProcess.h"
#endif

const UI32 _nbZeroesArr[128] = {
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 	//8
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 	//16
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 	//24
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 	//32
	//
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 	//40
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 	//48
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 	//56
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 	//64
	//
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 	//72
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 	//80
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 	//88
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 	//96
	//
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 	//104
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 	//112
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 	//120
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 	//128
};

void* NBMemory_allocUnmanaged(const UI32 sz){
#	ifdef NB_COMPILE_DRIVER_MODE
#		ifdef _WIN32
		return ExAllocatePoolWithTag(0 /*(NonPagedPool*/, sz, NB_COMPILE_DRIVER_MODE_ALLOC_TAG);
#		else
		return malloc(sz);
#		endif
#	else
	return malloc(sz);
#	endif
}

void  NBMemory_freeUnmanaged(void* ptr){
#	ifdef NB_COMPILE_DRIVER_MODE
#		ifdef _WIN32
		ExFreePoolWithTag(ptr, NB_COMPILE_DRIVER_MODE_ALLOC_TAG); //Win2000+
#		else
		free(ptr);
#		endif
#	else
	free(ptr);
#	endif
}

#ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
void* NBMemory_alloc_(const UI32 sz, const char* fullpath, const SI32 line, const char* func){
	NBMngrProcess_memAllocated((UI64)sz, fullpath, line, func);
#	ifdef NB_COMPILE_DRIVER_MODE
#		ifdef _WIN32
		return ExAllocatePoolWithTag(0 /*(NonPagedPool*/, sz, NB_COMPILE_DRIVER_MODE_ALLOC_TAG); //Win2000+
#		else
		return malloc(sz);
#		endif
#	else
	return malloc(sz);
#	endif
}
#else
void* NBMemory_alloc_(const UI32 sz){
#	ifdef NB_COMPILE_DRIVER_MODE
#		ifdef _WIN32
		return ExAllocatePoolWithTag(0 /*(NonPagedPool*/, sz, NB_COMPILE_DRIVER_MODE_ALLOC_TAG);
#		else
		return malloc(sz);
#		endif
#	else
	return malloc(sz);
#	endif
}
#endif

#ifdef CONFIG_NB_INCLUDE_THREADS_METRICS
void NBMemory_free_(void* ptr, const char* fullpath, const SI32 line, const char* func){
	NBMngrProcess_memFreed(fullpath, line, func);
#	ifdef NB_COMPILE_DRIVER_MODE
#		ifdef _WIN32
		ExFreePoolWithTag(ptr, NB_COMPILE_DRIVER_MODE_ALLOC_TAG); //Win2000+
#		else
		free(ptr);
#		endif
#	else
	free(ptr);
#	endif
}
#else
void NBMemory_free_(void* ptr){
#	ifdef NB_COMPILE_DRIVER_MODE
#		ifdef _WIN32
		ExFreePoolWithTag(ptr, NB_COMPILE_DRIVER_MODE_ALLOC_TAG); //Win2000+
#		else
		free(ptr);
#		endif
#	else
	free(ptr);
#	endif
}
#endif

void NBMemory_set_(void* ptr, SI32 v, UI32 sz){
	memset(ptr, v, (size_t)sz);
}

void NBMemory_cpy_(void* dst, const void* src, UI32 sz){ //memory can't overlap (user responsability)
	memcpy(dst, src, (size_t)sz);
}

void NBMemory_move_(void* dst, const void* src, UI32 sz){   //memory can overlap
    memmove(dst, src, (size_t)sz);
}

