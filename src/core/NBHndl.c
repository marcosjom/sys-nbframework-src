
#include "nb/NBFrameworkPch.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBHndl.h"
#include "nb/core/NBMngrProcess.h"


typedef struct STNBHndlOpq_ {
    STNBObject      prnt;
    STNBHndlNative  fd;
    BOOL            isOrphan;
    NBHndlCloseFnc  closeFunc;
#   ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
    UI64            hndlId; //provided by NBMngrProcess for dbg
#   endif
} STNBHndlOpq;

NB_OBJREF_BODY(NBHndl, STNBHndlOpq, NBObject)

void NBHndl_initZeroed(STNBObject* obj) {
    //STNBHndlOpq* opq = (STNBHndlOpq*)obj;
}

void NBHndl_uninitLocked(STNBObject* obj){
    STNBHndlOpq* opq = (STNBHndlOpq*)obj;
    if(opq->closeFunc != NULL){
        if(!(*opq->closeFunc)(&opq->fd)){
            //error
        } else {
            //success
#           ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
            if(opq->hndlId > 0){
                NBMngrProcess_hndlClosed(opq->hndlId);
                opq->hndlId = 0;
            }
#           endif
        }
        opq->closeFunc = NULL;
    }
    NBHndlNative_set(&opq->fd, ENNBHndlNativeType_Undef, NULL, 0);
}

#ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
BOOL NBHndl_setNative_(STNBHndlRef ref, const ENNBHndlNativeType type, void* src, const SI32 srcSz, NBHndlCloseFnc closeFunc, const char* fullpath, const SI32 line, const char* func)
#else
BOOL NBHndl_setNative(STNBHndlRef ref, const ENNBHndlNativeType type, void* src, const SI32 srcSz, NBHndlCloseFnc closeFunc)
#endif
{
    BOOL r = FALSE;
    STNBHndlOpq* opq = (STNBHndlOpq*)ref.opaque;
    if(closeFunc != NULL){
        r = TRUE;
        //close previous
        if(opq->closeFunc != NULL){
            if(!(*opq->closeFunc)(&opq->fd)){
                //error
                r = FALSE;
            } else {
#               ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
                if(opq->hndlId > 0){
                    NBMngrProcess_hndlClosed(opq->hndlId);
                    opq->hndlId = 0;
                }
#               endif
                //success
                opq->closeFunc = NULL;
                NBHndlNative_set(&opq->fd, ENNBHndlNativeType_Undef, NULL, 0);
            }
        }
        //set
        if(r && !NBHndlNative_set(&opq->fd, type, src, srcSz)){
            r = FALSE;
        } else {
            opq->closeFunc = closeFunc;
#           ifdef CONFIG_NB_INCLUDE_THREADS_ENABLED
            opq->hndlId = NBMngrProcess_hndlCreated(type, fullpath,line, func);
#           endif
        }
    }
    return r;
}

void NBHndl_setOrphan(STNBHndlRef ref){    //flags the fd as orphaned by its creator
    STNBHndlOpq* opq = (STNBHndlOpq*)ref.opaque;
    opq->isOrphan = TRUE;
}

BOOL NBHndl_isOrphan(STNBHndlRef ref){  //
    STNBHndlOpq* opq = (STNBHndlOpq*)ref.opaque;
    return opq->isOrphan;
}
