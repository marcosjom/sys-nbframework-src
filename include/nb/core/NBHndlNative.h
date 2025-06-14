#ifndef NB_FD_NATIVE_H
#define NB_FD_NATIVE_H

#include "nb/NBFrameworkDefs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ENNBHndlNativeType_ {
    ENNBHndlNativeType_Undef = 0,
    ENNBHndlNativeType_Socket,     //socket(), closesocket() close()
    ENNBHndlNativeType_FileWin,    //CreateFile(), CloseHandle()
    ENNBHndlNativeType_FileStd,    //fopen(), fclose()
    ENNBHndlNativeType_FilePosix,  //open(), close()
    //
    ENNBHndlNativeType_Count
} ENNBHndlNativeType;

typedef struct STNBHndlNative_ {
    ENNBHndlNativeType  type;   //
    SI32                vUse;   //
    BYTE                v[16];  //muts fit 'FILE*', 'SOCKET', 'int', Handle, and all expected types
} STNBHndlNative;

BOOL NBHndlNative_set(STNBHndlNative* obj, const ENNBHndlNativeType type, void* src, const SI32 srcSz);
BOOL NBHndlNative_get(STNBHndlNative* obj, void* dst, const SI32 dstSz);

#ifdef __cplusplus
} //extern "C"
#endif


#endif
