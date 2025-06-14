
#include "nb/NBFrameworkPch.h"
#include "nb/core/NBIOLnk.h"
//
#include "nb/core/NBMemory.h"

void NBIOLnk_init(STNBIOLnk* obj){
    NBMemory_setZeroSt(*obj, STNBIOLnk);
}

void NBIOLnk_retain(STNBIOLnk* obj){
    if(obj != NULL && obj->itf.ioRetain != NULL){
        (*obj->itf.ioRetain)(obj->usrData);
    }
}

void NBIOLnk_release(STNBIOLnk* obj){
    if(obj != NULL && obj->itf.ioRelease != NULL){
        (*obj->itf.ioRelease)(obj->usrData);
    }
}

void NBIOLnk_null(STNBIOLnk* obj){
    if(obj != NULL){
        NBMemory_setZeroSt(*obj, STNBIOLnk);
    }
}

//cfg

BOOL NBIOLnk_isSame(STNBIOLnk* obj, const STNBIOLnk* other){
    return (obj == other || (obj != NULL && other != NULL && obj->usrData == other->usrData));
}

BOOL NBIOLnk_isSet(STNBIOLnk* obj){
    return (obj != NULL && (obj->itf.ioRead != NULL || obj->itf.ioWrite != NULL));
}

void NBIOLnk_set(STNBIOLnk* obj, const STNBIOLnk* other){
    //retain new
    if(other != NULL && other->itf.ioRetain != NULL){
        (*other->itf.ioRetain)(other->usrData);
    }
    //release current
    if(obj != NULL && obj->itf.ioRelease != NULL){
        (*obj->itf.ioRelease)(obj->usrData);
    }
    //apply
    if(other == NULL){
        NBMemory_setZeroSt(obj->itf, STNBIOLnkItf);
        obj->usrData = NULL;
    } else {
        obj->itf = other->itf;
        obj->usrData = other->usrData;
    }
}

void NBIOLnk_setItf(STNBIOLnk* obj, const STNBIOLnkItf* itf, void* usrData){
    //retain new
    if(itf != NULL && itf->ioRetain != NULL){
        (*itf->ioRetain)(usrData);
    }
    //release current
    if(obj != NULL && obj->itf.ioRelease != NULL){
        (*obj->itf.ioRelease)(obj->usrData);
    }
    //apply
    if(itf == NULL){
        NBMemory_setZeroSt(obj->itf, STNBIOLnkItf);
        obj->usrData = NULL;
    } else {
        obj->itf = *itf;
        obj->usrData = usrData;
    }
}

//

int NBIOLnk_getFD(STNBIOLnk* obj){
    int r = 0;
    if(obj != NULL && obj->itf.ioGetFD != NULL){
        r = (*obj->itf.ioGetFD)(obj->usrData);
    }
    return r;
}

BOOL NBIOLnk_isObjRef(STNBIOLnk* obj, STNBObjRef objRef){
    BOOL r = FALSE;
    if(obj != NULL && obj->itf.ioIsObjRef != NULL){
        r = (*obj->itf.ioIsObjRef)(objRef, obj->usrData);
    }
    return r;
}

//io

SI32 NBIOLnk_read(STNBIOLnk* obj, void* dst, const SI32 dstSz){
    SI32 r = NB_IO_ERROR;
    if(obj != NULL && obj->itf.ioRead != NULL){
        r = (*obj->itf.ioRead)(dst, dstSz, obj->usrData);
    }
    return r;
}

SI32 NBIOLnk_write(STNBIOLnk* obj, const void* src, const SI32 srcSz){
    SI32 r = NB_IO_ERROR;
    if(obj != NULL && obj->itf.ioWrite != NULL){
        r = (*obj->itf.ioWrite)(src, srcSz, obj->usrData);
    }
    return r;
}

void NBIOLnk_flush(STNBIOLnk* obj){ //flush write-data
    if(obj != NULL && obj->itf.ioFlush != NULL){
        (*obj->itf.ioFlush)(obj->usrData);
    }
}

void NBIOLnk_shutdown(STNBIOLnk* obj, const UI8 mask){ //NB_IO_BIT_READ | NB_IO_BIT_WRITE
    if(obj != NULL && obj->itf.ioShutdown != NULL){
        (*obj->itf.ioShutdown)(mask, obj->usrData);
    }
}

void NBIOLnk_close(STNBIOLnk* obj){ //close ungracefully
    if(obj != NULL && obj->itf.ioClose != NULL){
        (*obj->itf.ioClose)(obj->usrData);
    }
}
