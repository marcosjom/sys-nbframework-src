#ifndef NB_IO_LNK_H
#define NB_IO_LNK_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBIO.h"

#ifdef __cplusplus
extern "C" {
#endif

struct STNBSocketRef_;

//NBIOLnkItf

typedef struct STNBIOLnkItf_ {
    //refs
    void    (*ioRetain)(void* usrData);
    void    (*ioRelease)(void* usrData);
    //
    int     (*ioGetFD)(void* usrData);
    BOOL    (*ioIsObjRef)(STNBObjRef objRef, void* usrData);
    //io
    SI32    (*ioRead)(void* dst, const SI32 dstSz, void* usrData); //read data to destination buffer, returns the ammount of bytes read, negative in case of error
    SI32    (*ioWrite)(const void* src, const SI32 srcSz, void* usrData); //write data from source buffer, returns the ammount of bytes written, negative in case of error
    void    (*ioFlush)(void* usrData);      //flush write-data
    void    (*ioShutdown)(const UI8 mask, void* usrData);   //NB_IO_BIT_READ | NB_IO_BIT_WRITE
    void    (*ioClose)(void* usrData);      //close ungracefully
} STNBIOLnkItf;

//NBIOLnk

typedef struct STNBIOLnk_ {
    STNBIOLnkItf    itf;
    void*           usrData;
} STNBIOLnk;

void NBIOLnk_init(STNBIOLnk* obj);
void NBIOLnk_retain(STNBIOLnk* obj);
void NBIOLnk_release(STNBIOLnk* obj);
void NBIOLnk_null(STNBIOLnk* obj);

//cfg
BOOL NBIOLnk_isSame(STNBIOLnk* obj, const STNBIOLnk* other);
BOOL NBIOLnk_isSet(STNBIOLnk* obj);
void NBIOLnk_set(STNBIOLnk* obj, const STNBIOLnk* other);
void NBIOLnk_setItf(STNBIOLnk* obj, const STNBIOLnkItf* itf, void* usrData);

//
int  NBIOLnk_getFD(STNBIOLnk* obj);
BOOL NBIOLnk_isObjRef(STNBIOLnk* obj, STNBObjRef objRef);

//io
SI32 NBIOLnk_read(STNBIOLnk* obj, void* dst, const SI32 dstSz);
SI32 NBIOLnk_write(STNBIOLnk* obj, const void* src, const SI32 srcSz);
void NBIOLnk_flush(STNBIOLnk* obj); //flush write-data
void NBIOLnk_shutdown(STNBIOLnk* obj, const UI8 mask); //NB_IO_BIT_READ | NB_IO_BIT_WRITE
void NBIOLnk_close(STNBIOLnk* obj); //close ungracefully

#ifdef __cplusplus
} //extern "C"
#endif


#endif
