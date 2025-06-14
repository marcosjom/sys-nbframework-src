#ifndef NB_FD_H
#define NB_FD_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBHndlNative.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef BOOL (*NBHndlCloseFnc)(STNBHndlNative* obj);    //This method should be static and independent of the life of objects. Could be called on exit-cleanup.

NB_OBJREF_HEADER(NBHndl)

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
#define NBHndl_setNative(REF, TYPE, SRC, SRCSZ, CLOSEFUNC) NBHndl_setNative_(REF, TYPE, SRC, SRCSZ, CLOSEFUNC, __FILE__, (SI32)__LINE__, __func__)
BOOL NBHndl_setNative_(STNBHndlRef ref, const ENNBHndlNativeType type, void* src, const SI32 srcSz, NBHndlCloseFnc closeFunc, const char* fullpath, const SI32 line, const char* func);
#else
BOOL NBHndl_setNative(STNBHndlRef ref, const ENNBHndlNativeType type, void* src, const SI32 srcSz, NBHndlCloseFnc closeFunc);
#endif

void NBHndl_setOrphan(STNBHndlRef ref); //flags the fd as orphaned by its creator; the creator should call this before releasing.
BOOL NBHndl_isOrphan(STNBHndlRef ref);  //if orphan by creator; other consumers should release as soon as posible.

#ifdef __cplusplus
} //extern "C"
#endif


#endif
