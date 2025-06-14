
#include "nb/NBFrameworkPch.h"
//Complementary PCH
#ifdef _WIN32
#	include <winsock2.h>	//Include before windows.h
//#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#else
#	include <poll.h>
//#	include <sys/socket.h>	//for socklen_t
#	include <errno.h>		//for errno
#	include <string.h>		//for strerror()
#endif
#include "nb/core/NBIOPollster.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBThread.h"
#include "nb/core/NBThreadCond.h"

#ifdef _WIN32
#	define NB_POLL_METHOD	WSAPoll
#	define NB_POLL_READ		POLLRDNORM
#	define NB_POLL_WRITE	POLLWRNORM
#	define NB_POLL_ERR		POLLERR
#   define NB_POLL_HUP      POLLHUP //peer closed its end of the channel (read is allowed but will return zero once all pending buffer is consumed)
#   define NB_POLL_NVAL     POLLNVAL
#   define NB_POLL_FD_INVERT_SIGN(FD)               //in Windows fd is always positive
#   define NB_POLL_FD_IS_ENABLED_VALID(FD)   TRUE   //in Windows fd is always positive
#   define NB_POLL_FD_IS_DISABLED_VALID(FD)  TRUE   //in Windows fd is always positive
#else
#	define NB_POLL_METHOD	poll
#	define NB_POLL_READ		POLLIN
#	define NB_POLL_WRITE	POLLOUT
#	define NB_POLL_ERR		POLLERR
#   define NB_POLL_HUP      POLLHUP //peer closed its end of the channel (read is allowed but will return zero once all pending buffer is consumed)
#   define NB_POLL_NVAL     POLLNVAL
#   define NB_POLL_FD_INVERT_SIGN(FD)        ((FD) = (FD) == 0 ? (FD) : -(FD))
#   define NB_POLL_FD_IS_ENABLED_VALID(FD)   ((FD) >= 0)    //stdout is zero
#   define NB_POLL_FD_IS_DISABLED_VALID(FD)  ((FD) <= 0)    //stdout is zero
#endif

// ENNBIOPollsterUpdActionBit

typedef enum ENNBIOPollsterUpdActionBit_ {
	//base
	ENNBIOPollsterUpdActionBit_None				= 0x0
	, ENNBIOPollsterUpdActionBit_Add		    = (0x1 << 0)
	, ENNBIOPollsterUpdActionBit_Remove         = (0x1 << 1)
	, ENNBIOPollsterUpdActionBit_ApplyParams	= (0x1 << 2)
	//combinations
	, ENNBIOPollsterUpdActionBits_All			= (ENNBIOPollsterUpdActionBit_None | ENNBIOPollsterUpdActionBit_Add | ENNBIOPollsterUpdActionBit_Remove | ENNBIOPollsterUpdActionBit_ApplyParams)
} ENNBIOPollsterUpdActionBit;

// NBIOPollsterUpdPend (add/remove socket pending actions)

typedef struct STNBIOPollsterUpdPend_ {
    STNBIOLnk               link;
	UI8						actionsMask; //ENNBIOPollsterUpdActionBit*
	//updates
	UI8						addOpsMask;	//ENNBIOPollsterOpBit*
	UI8						rmvOpsMask;	//ENNBIOPollsterOpBit*
	BOOL					syncFd;
	//params
	ENNBIOPollsterPollMode	pollMode;
	STNBIOPollsterLstrnItf	itf;
	void*					usrData;
} STNBIOPollsterUpdPend;

void NBIOPollsterUpdPend_init(STNBIOPollsterUpdPend* obj);
void NBIOPollsterUpdPend_release(STNBIOPollsterUpdPend* obj);

//NBIOPollsterFd

typedef struct STNBIOPollsterFd_ {
    STNBIOLnk               link;
	BOOL					isInvalid;	//socket or file is invalid (negative fd)
	BOOL					isEnabled;
	UI8						opsMask;
    STNBIOPollsterUpd       reqUpd;
	STNBIOPollsterLstrnItf	itf;
	ENNBIOPollsterPollMode	pollMode;
	void*					usrData;
#   ifdef NB_CONFIG_INCLUDE_ASSERTS
    //dbg
    struct {
        UI64                syncsCount; //times this IO has been touched by an NBIOPollsterSync action.
        UI64                ticksCount; //times this IO has seen a enginePoll tick call.
        UI64                ticksEnabledCount;  //times this IO has seen a enginePoll tick call in enabled state.
        UI64                ticksDisabledCount; //times this IO has seen a enginePoll tick call in disabled state.
        UI64                ticksOpCount; //times this IO has seen a enginePoll tick call with ops.
        UI64                ticksNopCount; //times this IO has seen a enginePoll tick call without ops.
    } dbg;
#   endif
} STNBIOPollsterFd;

void NBIOPollsterFd_init(STNBIOPollsterFd* obj);
void NBIOPollsterFd_release(STNBIOPollsterFd* obj);

//NBIOPollsterLstnr

typedef struct STNBIOPollsterLstnr_ {
	STNBIOPollsterLstnrItf  itf;
	void*				    usrData;
} STNBIOPollsterLstnr;

//NBIOPollster

typedef struct STNBIOPollsterOpq_ {
	STNBObject			prnt;
	STNBIOPollsterLstnr	lstnr;
	STNBIOPollsterSyncRef modsPend;
	//sockets
	struct {
		BOOL			isPolling;	//currently calling 'poll(...)'
		STNBArray		arr;		//STNBIOPollsterFd (max RLIMIT_NOFILE)
		STNBArray		fds;		//struct pollfd (max RLIMIT_NOFILE)
		UI32			enabledCount;
	} fds;
	//unsafe mode (optimization)
	struct {
		BOOL 			isEnabled;
		UI32			depth;
	} unsafe;
	//engine
	struct {
		BOOL			isRunning;	//'NBIOPollster_engineStart()' called, and 'NBIOPollster_engineStop()' not-called.
	} engine;
} STNBIOPollsterOpq;

NB_OBJREF_BODY(NBIOPollster, STNBIOPollsterOpq, NBObject)

#ifdef NB_CONFIG_INCLUDE_ASSERTS
BOOL NBIOPollster_dbgValidateLockedOpq_(STNBIOPollsterOpq* opq);
#endif

//NBIOPollsterSync

typedef struct STNBIOPollsterSyncOpq_ {
	STNBObject			prnt; 
	STNBIOPollsterOpq*	pollOpq;	//pollster (only defined in internal NBIOPollsterSync)
	//modsPend
	struct {
		STNBArray		arr;		//STNBIOPollsterUpdPend
	} modsPend;
} STNBIOPollsterSyncOpq;

NB_OBJREF_BODY(NBIOPollsterSync, STNBIOPollsterSyncOpq, NBObject)

#ifdef NB_CONFIG_INCLUDE_ASSERTS
BOOL NBIOPollsterSync_dbgValidateLockedOpq_(STNBIOPollsterSyncOpq* opq);
#endif

void NBIOPollsterSync_initZeroed(STNBObject* obj) {
	STNBIOPollsterSyncOpq* opq	= (STNBIOPollsterSyncOpq*)obj;
	//modsPend
	{
		NBArray_init(&opq->modsPend.arr, sizeof(STNBIOPollsterUpdPend), NULL);
	}
}

void NBIOPollsterSync_uninitLocked(STNBObject* obj){
	STNBIOPollsterSyncOpq* opq = (STNBIOPollsterSyncOpq*)obj;
	//modsPend
	{
		SI32 i; for(i = 0; i < opq->modsPend.arr.use; i++){
			STNBIOPollsterUpdPend* upd = NBArray_itmPtrAtIndex(&opq->modsPend.arr, STNBIOPollsterUpdPend, i);
			NBIOPollsterUpdPend_release(upd);
		}
		NBArray_empty(&opq->modsPend.arr);
		NBArray_release(&opq->modsPend.arr);
	}
}

//sockets

BOOL NBIOPollsterSync_isEmpty(STNBIOPollsterSyncRef ref){
	STNBIOPollsterSyncOpq* opq = (STNBIOPollsterSyncOpq*)ref.opaque;
	return (opq->modsPend.arr.use == 0);
}

//fd (sockets and files)

BOOL NBIOPollsterSync_addFdOpq_(STNBIOPollsterSyncOpq* opq, const STNBIOLnk* ioLnk, STNBSocketRef socket, STNBFileRef file, const STNBIOPollsterLstrnItf* itf, ENNBIOPollsterOpBit opsMask, const ENNBIOPollsterPollMode pollMode, void* usrData){
	BOOL r = FALSE;
    if(ioLnk != NULL || NBSocket_isSet(socket) || NBFile_isSet(file)){
		//add socket
		NBObject_lock(opq);
		{
			BOOL fnd = FALSE; SI32 i;
			//Add to modsPend array
			for(i = 0; i < opq->modsPend.arr.use; i++){
				STNBIOPollsterUpdPend* s = NBArray_itmPtrAtIndex(&opq->modsPend.arr, STNBIOPollsterUpdPend, i);
				if(NBIOLnk_isSame(&s->link, ioLnk) || NBIOLnk_isObjRef(&s->link, socket) || NBIOLnk_isObjRef(&s->link, file)){
					if(s->actionsMask & ENNBIOPollsterUpdActionBit_Remove){
						//change remove-action to apply-action 
						s->actionsMask	&= ~ENNBIOPollsterUpdActionBit_Remove;
						s->actionsMask	|= ENNBIOPollsterUpdActionBit_ApplyParams;
						s->addOpsMask	= opsMask;
						s->rmvOpsMask	= 0;
						if(itf == NULL){
							NBMemory_setZeroSt(s->itf, STNBIOPollsterLstrnItf);
						} else {
							s->itf		= * itf; 
						}
						s->pollMode		= pollMode;
						s->usrData		= usrData;
						r				= TRUE;
					}
#					ifdef NB_CONFIG_INCLUDE_ASSERTS
					else {
						NBASSERT(FALSE) //user-logic error, IO already added
					} 
#					endif
					fnd = TRUE;
					break;
				}
			}
			//add new pend-record
			if(!fnd){
				STNBIOPollsterUpdPend s;
				NBIOPollsterUpdPend_init(&s);
				s.actionsMask	|= (ENNBIOPollsterUpdActionBit_Add | ENNBIOPollsterUpdActionBit_ApplyParams);
				s.addOpsMask	= opsMask;
				{
					if(itf == NULL){
						NBMemory_setZeroSt(s.itf, STNBIOPollsterLstrnItf);
					} else {
						s.itf	= *itf; 
					}
					s.usrData	= usrData;
                    //getIOLnk
                    if(ioLnk != NULL){
                        NBIOLnk_set(&s.link, ioLnk);
                    } else if(NBSocket_isSet(socket)){
                        NBSocket_getIOLnk(socket, &s.link);
                    } else if(NBFile_isSet(file)){
                        NBFile_getIOLnk(file, &s.link);
                    }
				}
				s.pollMode		= pollMode;
				NBArray_addValue(&opq->modsPend.arr, s);
				r				= TRUE;
			}
			NBASSERT(NBIOPollsterSync_dbgValidateLockedOpq_(opq)) //validation fail
		}
		NBObject_unlock(opq);
	}
	return r;
}

BOOL NBIOPollsterSync_enableFdOpsOpq_(STNBIOPollsterSyncOpq* opq, const STNBIOLnk* ioLnk, STNBSocketRef socket, STNBFileRef file, const ENNBIOPollsterOpBit opsMask){
    BOOL r = FALSE;
    if(ioLnk != NULL || NBSocket_isSet(socket) || NBFile_isSet(file)){
        NBObject_lock(opq);
        {
            //Add to modsPend array
            BOOL fnd = FALSE;
            SI32 i; for(i = 0; i < opq->modsPend.arr.use; i++){
                STNBIOPollsterUpdPend* s = NBArray_itmPtrAtIndex(&opq->modsPend.arr, STNBIOPollsterUpdPend, i);
                if(NBIOLnk_isSame(&s->link, ioLnk) || NBIOLnk_isObjRef(&s->link, socket) || NBIOLnk_isObjRef(&s->link, file)){
                    if(!(s->actionsMask & ENNBIOPollsterUpdActionBit_Remove)){
                        ENNBIOPollsterOpBit remainMask = opsMask;
                        const ENNBIOPollsterOpBit common = (s->rmvOpsMask & remainMask);
                        if(common){
                            s->rmvOpsMask    &= ~common;
                            remainMask        &= ~common;
                        }
                        if(remainMask){
                            s->addOpsMask    |= remainMask;
                        }
                        //Remove pend-record if no-action remain
                        if(!s->actionsMask && !s->addOpsMask && !s->rmvOpsMask && !s->syncFd){
                            NBIOPollsterUpdPend_release(s);
                            NBArray_removeItemAtIndex(&opq->modsPend.arr, i);
                        }
                        r = TRUE;
                    }
#                   ifdef NB_CONFIG_INCLUDE_ASSERTS
                    else {
                        NBASSERT(FALSE) //user-logic error, IO not added
                    }
#                   endif
                    fnd = TRUE;
                    break;
                }
            }
            if(!fnd){
                BOOL srchd = FALSE, fnd = FALSE;
                if(opq->pollOpq != NULL){
                    //Search in current array
                    SI32 i2; for(i2 = 0; i2 < opq->pollOpq->fds.arr.use; i2++){
                        STNBIOPollsterFd* s2 = NBArray_itmPtrAtIndex(&opq->pollOpq->fds.arr, STNBIOPollsterFd, i2);
                        if(NBIOLnk_isSame(&s2->link, ioLnk) || NBIOLnk_isObjRef(&s2->link, socket) || NBIOLnk_isObjRef(&s2->link, file)){
                            fnd = TRUE;
                            break;
                        }
                    } NBASSERT(fnd) //user-logic error (socket not added)
                    srchd = TRUE;
                }
                if(!srchd || fnd){
                    STNBIOPollsterUpdPend s;
                    NBIOPollsterUpdPend_init(&s);
                    s.addOpsMask    = opsMask;
                    //getIOLnk and retain
                    if(ioLnk != NULL){
                        NBIOLnk_set(&s.link, ioLnk);
                    } else if(NBSocket_isSet(socket)){
                        NBSocket_getIOLnk(socket, &s.link);
                    } else if(NBFile_isSet(file)){
                        NBFile_getIOLnk(file, &s.link);
                    }
                    NBArray_addValue(&opq->modsPend.arr, s);
                    r = TRUE;
                }
            }
            NBASSERT(NBIOPollsterSync_dbgValidateLockedOpq_(opq)) //validation fail
        }
        NBObject_unlock(opq);
    }
    return r;
}

BOOL NBIOPollsterSync_disableFdOpsOpq_(STNBIOPollsterSyncOpq* opq, const STNBIOLnk* ioLnk, STNBSocketRef socket, STNBFileRef file, const ENNBIOPollsterOpBit opsMask){
    BOOL r = FALSE;
    if(ioLnk != NULL || NBSocket_isSet(socket) || NBFile_isSet(file)){
        NBObject_lock(opq);
        {
            //Add to modsPend array
            BOOL fnd = FALSE;
            SI32 i; for(i = 0; i < opq->modsPend.arr.use; i++){
                STNBIOPollsterUpdPend* s = NBArray_itmPtrAtIndex(&opq->modsPend.arr, STNBIOPollsterUpdPend, i);
                if(NBIOLnk_isSame(&s->link, ioLnk) || NBIOLnk_isObjRef(&s->link, socket) || NBIOLnk_isObjRef(&s->link, file)){
                    if(!(s->actionsMask & ENNBIOPollsterUpdActionBit_Remove)){
                        ENNBIOPollsterOpBit remainMask = opsMask;
                        const ENNBIOPollsterOpBit common = (s->addOpsMask & remainMask);
                        if(common){
                            s->addOpsMask    &= ~common;
                            remainMask       &= ~common;
                        }
                        if(remainMask){
                            s->rmvOpsMask    |= remainMask;
                        }
                        //Remove pend-record if no-action remain
                        if(!s->actionsMask && !s->addOpsMask && !s->rmvOpsMask && !s->syncFd){
                            NBIOPollsterUpdPend_release(s);
                            NBArray_removeItemAtIndex(&opq->modsPend.arr, i);
                        }
                        r = TRUE;
                    }
#                   ifdef NB_CONFIG_INCLUDE_ASSERTS
                    else {
                        NBASSERT(FALSE) //user-logic error, IO not added
                    }
#                   endif
                    fnd = TRUE;
                    break;
                }
            }
            if(!fnd){
                BOOL srchd = FALSE, fnd = FALSE;
                if(opq->pollOpq != NULL){
                    //Search in current array
                    SI32 i2; for(i2 = 0; i2 < opq->pollOpq->fds.arr.use; i2++){
                        STNBIOPollsterFd* s2 = NBArray_itmPtrAtIndex(&opq->pollOpq->fds.arr, STNBIOPollsterFd, i2);
                        if(NBIOLnk_isSame(&s2->link, ioLnk) || NBIOLnk_isObjRef(&s2->link, socket) || NBIOLnk_isObjRef(&s2->link, file)){
                            fnd = TRUE;
                            break;
                        }
                    } NBASSERT(fnd)
                    srchd = TRUE;
                }
                if(!srchd || fnd){
                    STNBIOPollsterUpdPend s;
                    NBIOPollsterUpdPend_init(&s);
                    s.rmvOpsMask    = opsMask;
                    //getIOLnk and retain
                    if(ioLnk != NULL){
                        NBIOLnk_set(&s.link, ioLnk);
                    } else if(NBSocket_isSet(socket)){
                        NBSocket_getIOLnk(socket, &s.link);
                    } else if(NBFile_isSet(file)){
                        NBFile_getIOLnk(file, &s.link);
                    }
                    NBArray_addValue(&opq->modsPend.arr, s);
                    r = TRUE;
                }
            }
            NBASSERT(NBIOPollsterSync_dbgValidateLockedOpq_(opq)) //validation fail
        }
        NBObject_unlock(opq);
    }
    return r;
}

BOOL NBIOPollsterSync_updateFdOpq_(STNBIOPollsterSyncOpq* opq, const STNBIOLnk* ioLnk, STNBSocketRef socket, STNBFileRef file){
    BOOL r = FALSE;
    if(ioLnk != NULL || NBSocket_isSet(socket) || NBFile_isSet(file)){
        NBObject_lock(opq);
        {
            //Add to modsPend array
            BOOL fnd = FALSE;
            SI32 i; for(i = 0; i < opq->modsPend.arr.use; i++){
                STNBIOPollsterUpdPend* s = NBArray_itmPtrAtIndex(&opq->modsPend.arr, STNBIOPollsterUpdPend, i);
                if(NBIOLnk_isSame(&s->link, ioLnk) || NBIOLnk_isObjRef(&s->link, socket) || NBIOLnk_isObjRef(&s->link, file)){
                    if(!(s->actionsMask & ENNBIOPollsterUpdActionBit_Remove)){
                        s->syncFd = TRUE;
                        //Remove pend-record if no-action remain
                        /*if(!s->actionsMask && !s->addOpsMask && !s->rmvOpsMask && !s->syncFd){
                         NBIOPollsterUpdPend_release(s);
                         NBArray_removeItemAtIndex(&opq->modsPend.arr, i);
                         }*/
                        r = TRUE;
                    }
#                   ifdef NB_CONFIG_INCLUDE_ASSERTS
                    else {
                        NBASSERT(FALSE) //user-logic error, IO not added
                    }
#                   endif
                    fnd = TRUE;
                    break;
                }
            }
            if(!fnd){
                BOOL srchd = FALSE, fnd = FALSE;
                if(opq->pollOpq != NULL){
                    //Search in current array
                    SI32 i2; for(i2 = 0; i2 < opq->pollOpq->fds.arr.use; i2++){
                        STNBIOPollsterFd* s2 = NBArray_itmPtrAtIndex(&opq->pollOpq->fds.arr, STNBIOPollsterFd, i2);
                        if(NBIOLnk_isSame(&s2->link, ioLnk) || NBIOLnk_isObjRef(&s2->link, socket) || NBIOLnk_isObjRef(&s2->link, file)){
                            fnd = TRUE;
                            break;
                        }
                    } NBASSERT(fnd) //user-logic error (IO not added)
                    srchd = TRUE;
                }
                if(!srchd || fnd){
                    STNBIOPollsterUpdPend s;
                    NBIOPollsterUpdPend_init(&s);
                    s.syncFd        = TRUE;
                    //getIOLnk and retain
                    if(ioLnk != NULL){
                        NBIOLnk_set(&s.link, ioLnk);
                    } else if(NBSocket_isSet(socket)){
                        NBSocket_getIOLnk(socket, &s.link);
                    } else if(NBFile_isSet(file)){
                        NBFile_getIOLnk(file, &s.link);
                    }
                    NBArray_addValue(&opq->modsPend.arr, s);
                    r = TRUE;
                }
            }
            NBASSERT(NBIOPollsterSync_dbgValidateLockedOpq_(opq)) //validation fail
        }
        NBObject_unlock(opq);
    }
    return r;
}

BOOL NBIOPollsterSync_removeFdOpq_(STNBIOPollsterSyncOpq* opq, const STNBIOLnk* ioLnk, STNBSocketRef socket, STNBFileRef file){
    BOOL r = FALSE;
    if(ioLnk != NULL || NBSocket_isSet(socket) || NBFile_isSet(file)){
        NBObject_lock(opq);
        {
            //Add to modsPend array
            BOOL fnd = FALSE;
            SI32 i; for(i = 0; i < opq->modsPend.arr.use; i++){
                STNBIOPollsterUpdPend* s = NBArray_itmPtrAtIndex(&opq->modsPend.arr, STNBIOPollsterUpdPend, i);
                if(NBIOLnk_isSame(&s->link, ioLnk) || NBIOLnk_isObjRef(&s->link, socket) || NBIOLnk_isObjRef(&s->link, file)){
                    if(s->actionsMask & ENNBIOPollsterUpdActionBit_Add){
                        NBIOPollsterUpdPend_release(s);
                        NBArray_removeItemAtIndex(&opq->modsPend.arr, i);
                        r = TRUE;
                    } else if(!(s->actionsMask & ENNBIOPollsterUpdActionBit_Remove)){
                        s->actionsMask    = ENNBIOPollsterUpdActionBit_Remove;
                        s->addOpsMask    = 0;
                        s->rmvOpsMask    = 0;
                        s->syncFd        = FALSE;
                        s->pollMode        = 0;
                        s->usrData        = NULL;
                        r = TRUE;
                    }
#                   ifdef NB_CONFIG_INCLUDE_ASSERTS
                    else {
                        NBASSERT(FALSE) //user-logic error (IO not added, removing twice)
                    }
#                   endif
                    fnd = TRUE;
                    break;
                }
            }
            if(!fnd){
                BOOL srchd = FALSE, fnd = FALSE;
                if(opq->pollOpq != NULL){
                    //Search in current array
                    SI32 i2; for(i2 = 0; i2 < opq->pollOpq->fds.arr.use; i2++){
                        STNBIOPollsterFd* s2 = NBArray_itmPtrAtIndex(&opq->pollOpq->fds.arr, STNBIOPollsterFd, i2);
                        if(NBIOLnk_isSame(&s2->link, ioLnk) || NBIOLnk_isObjRef(&s2->link, socket) || NBIOLnk_isObjRef(&s2->link, file)){
                            fnd = TRUE;
                            break;
                        }
                    } NBASSERT(fnd) //user-logic error (IO not added)
                    srchd = TRUE;
                }
                if(!srchd || fnd){
                    STNBIOPollsterUpdPend s;
                    NBIOPollsterUpdPend_init(&s);
                    s.actionsMask   = ENNBIOPollsterUpdActionBit_Remove;
                    //getIOLnk and retain
                    if(ioLnk != NULL){
                        NBIOLnk_set(&s.link, ioLnk);
                    } else if(NBSocket_isSet(socket)){
                        NBSocket_getIOLnk(socket, &s.link);
                    } else if(NBFile_isSet(file)){
                        NBFile_getIOLnk(file, &s.link);
                    }
                    NBArray_addValue(&opq->modsPend.arr, s);
                    r = TRUE;
                }
            }
            NBASSERT(NBIOPollsterSync_dbgValidateLockedOpq_(opq)) //validation fail
        }
        NBObject_unlock(opq);
    }
    return r;
}

//ioLnk

BOOL NBIOPollsterSync_addIOLnk(STNBIOPollsterSyncRef ref, const STNBIOLnk* ioLnk, const ENNBIOPollsterOpBit opsMask, const ENNBIOPollsterPollMode pollMode, void* usrData){
    STNBIOPollsterSyncOpq* opq = (STNBIOPollsterSyncOpq*)ref.opaque; NBASSERT(NBIOPollsterSync_isClass(ref))
    return NBIOPollsterSync_addFdOpq_(opq, ioLnk, NB_OBJREF_NULL, NB_OBJREF_NULL, NULL, opsMask, pollMode, usrData);
}

BOOL NBIOPollsterSync_addIOLnkWithItf(STNBIOPollsterSyncRef ref, const STNBIOLnk* ioLnk, const ENNBIOPollsterOpBit opsMask, const STNBIOPollsterLstrnItf* itf, void* usrData){
    STNBIOPollsterSyncOpq* opq = (STNBIOPollsterSyncOpq*)ref.opaque; NBASSERT(NBIOPollsterSync_isClass(ref))
    return NBIOPollsterSync_addFdOpq_(opq, ioLnk, NB_OBJREF_NULL, NB_OBJREF_NULL, itf, opsMask, ENNBIOPollsterPollMode_DisableFoundOps, usrData);
}

BOOL NBIOPollsterSync_enableIOLnkOps(STNBIOPollsterSyncRef ref, const STNBIOLnk* ioLnk, const ENNBIOPollsterOpBit opsMask){
    STNBIOPollsterSyncOpq* opq = (STNBIOPollsterSyncOpq*)ref.opaque; NBASSERT(NBIOPollsterSync_isClass(ref))
    return NBIOPollsterSync_enableFdOpsOpq_(opq, ioLnk, NB_OBJREF_NULL, NB_OBJREF_NULL, opsMask);
}

BOOL NBIOPollsterSync_disableIOLnkOps(STNBIOPollsterSyncRef ref, const STNBIOLnk* ioLnk, const ENNBIOPollsterOpBit opsMask){
    STNBIOPollsterSyncOpq* opq = (STNBIOPollsterSyncOpq*)ref.opaque; NBASSERT(NBIOPollsterSync_isClass(ref))
    return NBIOPollsterSync_disableFdOpsOpq_(opq, ioLnk, NB_OBJREF_NULL, NB_OBJREF_NULL, opsMask);
}

BOOL NBIOPollsterSync_updateIOLnkFD(STNBIOPollsterSyncRef ref, const STNBIOLnk* ioLnk){
    STNBIOPollsterSyncOpq* opq = (STNBIOPollsterSyncOpq*)ref.opaque; NBASSERT(NBIOPollsterSync_isClass(ref))
    return NBIOPollsterSync_updateFdOpq_(opq, ioLnk, NB_OBJREF_NULL, NB_OBJREF_NULL);
}

BOOL NBIOPollsterSync_removeIOLnk(STNBIOPollsterSyncRef ref, const STNBIOLnk* ioLnk){
    STNBIOPollsterSyncOpq* opq = (STNBIOPollsterSyncOpq*)ref.opaque; NBASSERT(NBIOPollsterSync_isClass(ref))
    return NBIOPollsterSync_removeFdOpq_(opq, ioLnk, NB_OBJREF_NULL, NB_OBJREF_NULL);
}

//sockets

BOOL NBIOPollsterSync_addSocket(STNBIOPollsterSyncRef ref, STNBSocketRef socket, const ENNBIOPollsterOpBit opsMask, const ENNBIOPollsterPollMode pollMode, void* usrData){
	STNBIOPollsterSyncOpq* opq = (STNBIOPollsterSyncOpq*)ref.opaque; NBASSERT(NBIOPollsterSync_isClass(ref))
	return NBIOPollsterSync_addFdOpq_(opq, NULL, socket, NB_OBJREF_NULL, NULL, opsMask, pollMode, usrData);
}

BOOL NBIOPollsterSync_addSocketWithItf(STNBIOPollsterSyncRef ref, STNBSocketRef socket, const ENNBIOPollsterOpBit opsMask, const STNBIOPollsterLstrnItf* itf, void* usrData){
	STNBIOPollsterSyncOpq* opq = (STNBIOPollsterSyncOpq*)ref.opaque; NBASSERT(NBIOPollsterSync_isClass(ref))
	return NBIOPollsterSync_addFdOpq_(opq, NULL, socket, NB_OBJREF_NULL, itf, opsMask, ENNBIOPollsterPollMode_DisableFoundOps, usrData);
}

BOOL NBIOPollsterSync_enableSocketOps(STNBIOPollsterSyncRef ref, STNBSocketRef socket, const ENNBIOPollsterOpBit opsMask){
    STNBIOPollsterSyncOpq* opq = (STNBIOPollsterSyncOpq*)ref.opaque; NBASSERT(NBIOPollsterSync_isClass(ref))
    return NBIOPollsterSync_enableFdOpsOpq_(opq, NULL, socket, NB_OBJREF_NULL, opsMask);
}

BOOL NBIOPollsterSync_disableSocketOps(STNBIOPollsterSyncRef ref, STNBSocketRef socket, const ENNBIOPollsterOpBit opsMask){
    STNBIOPollsterSyncOpq* opq = (STNBIOPollsterSyncOpq*)ref.opaque; NBASSERT(NBIOPollsterSync_isClass(ref))
    return NBIOPollsterSync_disableFdOpsOpq_(opq, NULL, socket, NB_OBJREF_NULL, opsMask);
}

BOOL NBIOPollsterSync_updateSocketFD(STNBIOPollsterSyncRef ref, STNBSocketRef socket){
    STNBIOPollsterSyncOpq* opq = (STNBIOPollsterSyncOpq*)ref.opaque; NBASSERT(NBIOPollsterSync_isClass(ref))
    return NBIOPollsterSync_updateFdOpq_(opq, NULL, socket, NB_OBJREF_NULL);
}

BOOL NBIOPollsterSync_removeSocket(STNBIOPollsterSyncRef ref, STNBSocketRef socket){
    STNBIOPollsterSyncOpq* opq = (STNBIOPollsterSyncOpq*)ref.opaque; NBASSERT(NBIOPollsterSync_isClass(ref))
    return NBIOPollsterSync_removeFdOpq_(opq, NULL, socket, NB_OBJREF_NULL);
}

//files

BOOL NBIOPollsterSync_addFile(STNBIOPollsterSyncRef ref, STNBFileRef file, const ENNBIOPollsterOpBit opsMask, const ENNBIOPollsterPollMode pollMode, void* usrData){
    STNBIOPollsterSyncOpq* opq = (STNBIOPollsterSyncOpq*)ref.opaque; NBASSERT(NBIOPollsterSync_isClass(ref))
    return NBIOPollsterSync_addFdOpq_(opq, NULL, NB_OBJREF_NULL, file, NULL, opsMask, pollMode, usrData);
}

BOOL NBIOPollsterSync_addFileWithItf(STNBIOPollsterSyncRef ref, STNBFileRef file, const ENNBIOPollsterOpBit opsMask, const STNBIOPollsterLstrnItf* itf, void* usrData){
    STNBIOPollsterSyncOpq* opq = (STNBIOPollsterSyncOpq*)ref.opaque; NBASSERT(NBIOPollsterSync_isClass(ref))
    return NBIOPollsterSync_addFdOpq_(opq, NULL, NB_OBJREF_NULL, file, itf, opsMask, ENNBIOPollsterPollMode_DisableFoundOps, usrData);
}

BOOL NBIOPollsterSync_enableFileOps(STNBIOPollsterSyncRef ref, STNBFileRef file, const ENNBIOPollsterOpBit opsMask){
    STNBIOPollsterSyncOpq* opq = (STNBIOPollsterSyncOpq*)ref.opaque; NBASSERT(NBIOPollsterSync_isClass(ref))
    return NBIOPollsterSync_enableFdOpsOpq_(opq, NULL, NB_OBJREF_NULL, file, opsMask);
}

BOOL NBIOPollsterSync_disableFileOps(STNBIOPollsterSyncRef ref, STNBFileRef file, const ENNBIOPollsterOpBit opsMask){
    STNBIOPollsterSyncOpq* opq = (STNBIOPollsterSyncOpq*)ref.opaque; NBASSERT(NBIOPollsterSync_isClass(ref))
    return NBIOPollsterSync_disableFdOpsOpq_(opq, NULL, NB_OBJREF_NULL, file, opsMask);
}

BOOL NBIOPollsterSync_updateFileFD(STNBIOPollsterSyncRef ref, STNBFileRef file){
    STNBIOPollsterSyncOpq* opq = (STNBIOPollsterSyncOpq*)ref.opaque; NBASSERT(NBIOPollsterSync_isClass(ref))
    return NBIOPollsterSync_updateFdOpq_(opq, NULL, NB_OBJREF_NULL, file);
}

BOOL NBIOPollsterSync_removeFile(STNBIOPollsterSyncRef ref, STNBFileRef file){
    STNBIOPollsterSyncOpq* opq = (STNBIOPollsterSyncOpq*)ref.opaque; NBASSERT(NBIOPollsterSync_isClass(ref))
    return NBIOPollsterSync_removeFdOpq_(opq, NULL, NB_OBJREF_NULL, file);
}

//

BOOL NBIOPollsterSync_sendTo(STNBIOPollsterSyncRef ref, STNBIOPollsterSyncRef other){
	BOOL r = FALSE;
	UI32 use = 0; STNBIOPollsterUpdPend* arr = NULL;
	//get array
	{
		STNBIOPollsterSyncOpq* opq = (STNBIOPollsterSyncOpq*)ref.opaque;
		NBASSERT(NBIOPollsterSync_isClass(ref))
		if(opq != NULL){
			NBObject_lock(opq);
			{
				use = opq->modsPend.arr.use;
				if(use > 0){
					STNBIOPollsterUpdPend* org = NBArray_dataPtr(&opq->modsPend.arr, STNBIOPollsterUpdPend);
					arr = NBMemory_allocTypes(STNBIOPollsterUpdPend, use);
					NBMemory_copy(arr, org, sizeof(arr[0]) * use); 
					NBArray_empty(&opq->modsPend.arr);
					//PRINTF_INFO("NBIOPollsterSync, %d syncTasks loaded from list.\n", use);
				}
			}
			NBObject_unlock(opq);
		}
	}
	//add to other
	{
		STNBIOPollsterSyncOpq* opq = (STNBIOPollsterSyncOpq*)other.opaque;
		NBASSERT(NBIOPollsterSync_isClass(other))
		if(opq != NULL){
			if(use > 0){
				NBObject_lock(opq);
				{
					NBArray_addItems(&opq->modsPend.arr, arr, sizeof(arr[0]), use);
					//PRINTF_INFO("NBIOPollsterSync, %d syncTasks moved to list.\n", use);
				}
				NBObject_unlock(opq);
			}
			r = TRUE;
		}
	}
	//free
	if(arr != NULL){
		//PRINTF_INFO("NBIOPollsterSync, %d syncTasks release.\n", use);
		NBMemory_free(arr);
		arr = NULL;
		use = 0;
	}
	return r;
}

#ifdef NB_CONFIG_INCLUDE_ASSERTS
BOOL NBIOPollsterSync_dbgValidateLockedOpq_(STNBIOPollsterSyncOpq* opq){
	BOOL r = TRUE;
	//Validate pend-array
	{
		SI32 i, i2; for(i = 0; i < opq->modsPend.arr.use; i++){
			STNBIOPollsterUpdPend* s = NBArray_itmPtrAtIndex(&opq->modsPend.arr, STNBIOPollsterUpdPend, i);
			if(s->link.usrData == NULL){
				NBASSERT(FALSE) //ioLnk should be set
				r = FALSE;
			}
			if(s->actionsMask & ~ENNBIOPollsterUpdActionBits_All){
				NBASSERT(FALSE) //unexpected extra pend-action-bits found
				r = FALSE;
			}
			if(s->actionsMask & ENNBIOPollsterUpdActionBit_Remove){
				if(s->addOpsMask || s->rmvOpsMask || s->syncFd || s->pollMode != 0 || s->usrData != NULL){
					NBASSERT(FALSE) //params should be empty at remove action
					r = FALSE;
				}
				if(s->actionsMask != ENNBIOPollsterUpdActionBit_Remove){
					NBASSERT(FALSE) //only remove-flag should be enabled
					r = FALSE;
				}
				if(opq->pollOpq){
					for(i2 = 0; i2 < opq->pollOpq->fds.arr.use; i2++){
						STNBIOPollsterFd* s2 = NBArray_itmPtrAtIndex(&opq->pollOpq->fds.arr, STNBIOPollsterFd, i2);
                        if(NBIOLnk_isSame(&s->link, &s2->link)){
							break;
						}
					}
					if(i2 == opq->pollOpq->fds.arr.use){
						NBASSERT(FALSE) //IO not found for remove-action
						r = FALSE;
					}
				}
			} else if(s->actionsMask & ENNBIOPollsterUpdActionBit_Add){
				if(!(s->actionsMask & ENNBIOPollsterUpdActionBit_ApplyParams)){
					NBASSERT(FALSE) //apply-flag must be set with add-flag
					r = FALSE;
				}
				if(s->pollMode < 0 || s->pollMode >= ENNBIOPollsterPollMode_Count){
					NBASSERT(FALSE) //invalid poll-mode value
					r = FALSE;
				}
				if(opq->pollOpq != NULL){
					for(i2 = 0; i2 < opq->pollOpq->fds.arr.use; i2++){
						STNBIOPollsterFd* s2 = NBArray_itmPtrAtIndex(&opq->pollOpq->fds.arr, STNBIOPollsterFd, i2);
                        if(NBIOLnk_isSame(&s->link, &s2->link)){
							break;
						}
					}
					if(i2 < opq->pollOpq->fds.arr.use){
						NBASSERT(FALSE) //IO already added for add-action
						r = FALSE;
					}
				}
			} else {
				if(!s->addOpsMask && !s->rmvOpsMask && !s->syncFd && !(s->actionsMask & ENNBIOPollsterUpdActionBit_ApplyParams)){
					NBASSERT(FALSE) //empty update-action should be removed
					r = FALSE;
				}
				if(opq->pollOpq != NULL){
					for(i2 = 0; i2 < opq->pollOpq->fds.arr.use; i2++){
						STNBIOPollsterFd* s2 = NBArray_itmPtrAtIndex(&opq->pollOpq->fds.arr, STNBIOPollsterFd, i2);
                        if(NBIOLnk_isSame(&s->link, &s2->link)){
							break;
						}
					}
					if(i2 == opq->pollOpq->fds.arr.use){
						NBASSERT(FALSE) //IO not found for update-action
						r = FALSE;
					}
				}
			}
		}
	}
	return r;
}
#endif

//-----------------
//NBPollster MACROS
//-----------------

//Auto-safe action (depends of 'unsafe.isEnabled')

#define NB_POLLSTER_AUTOSAFE_ACTION_START(OPQ) \
	BOOL _locked_ = FALSE; \
	NB_POLLSTER_AUTOSAFE_ACTION_START_AGAIN(OPQ)

#define NB_POLLSTER_AUTOSAFE_ACTION_END(OPQ) \
	if(_locked_){ \
		NBObject_unlock((OPQ)); \
		_locked_ = FALSE; \
	} else { \
		NBASSERT((OPQ)->unsafe.depth > 0) /*program logic error*/ \
		(OPQ)->unsafe.depth--; \
	}

#define NB_POLLSTER_AUTOSAFE_ACTION_START_AGAIN(OPQ) \
	NBASSERT((OPQ) != NULL) \
	NBASSERT((OPQ)->unsafe.depth == 0) /*user logic error, unsafe mode must be enabled and only serial calls are allowed*/ \
	if((OPQ)->unsafe.isEnabled) {\
		(OPQ)->unsafe.depth++; \
	} else { \
		NBObject_lock((OPQ)); \
		_locked_ = TRUE; \
	}

#define NB_POLLSTER_AUTOSAFE_IS_LOCKED		(_locked_)

void NBIOPollster_syncPollArrayLockedOpq_(STNBIOPollsterOpq* opq, STNBArray* dstRmvdToNotify /*STNBIOPollsterFd*/);
void NBIOPollster_removeAllLockedOpq_(STNBIOPollsterOpq* opq, STNBArray* dstRmvdToNotify /*STNBIOPollsterFd*/);

void NBIOPollster_initZeroed(STNBObject* obj) {
	STNBIOPollsterOpq* opq	= (STNBIOPollsterOpq*)obj;
	//modsPend
	{
		opq->modsPend = NBIOPollsterSync_alloc(NULL);
		{
			STNBIOPollsterSyncOpq* syncOpq = (STNBIOPollsterSyncOpq*)opq->modsPend.opaque;
			syncOpq->pollOpq = opq;
		}
	}
	//sockets
	{
		NBArray_init(&opq->fds.arr, sizeof(STNBIOPollsterFd), NULL);
		NBArray_init(&opq->fds.fds, sizeof(struct pollfd), NULL);
	}
}

void NBIOPollster_uninitLocked(STNBObject* obj){
	STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)obj;
	NBASSERT(!opq->fds.isPolling) //should not be polling
	//empty array
	{
        STNBArray rmvdToNotify; //STNBIOPollsterFd
        NBArray_initWithSz(&rmvdToNotify, sizeof(STNBIOPollsterFd), NULL, 0, 32, 0.1f);
        //remove
		NBIOPollster_removeAllLockedOpq_(opq, &rmvdToNotify);
        NBObject_unlock(opq);
        //notify removed ones (unlocked)
        {
            SI32 i; for(i = 0; i < rmvdToNotify.use; i++){
                STNBIOPollsterFd* s = NBArray_itmPtrAtIndex(&rmvdToNotify, STNBIOPollsterFd, i);
                //notify
                if(s->itf.pollRemoved != NULL){
                    (*s->itf.pollRemoved)(s->link, s->usrData);
                }
                //release
                NBIOPollsterFd_release(s);
            }
            NBArray_empty(&rmvdToNotify);
            NBArray_release(&rmvdToNotify);
        }
        NBObject_lock(opq);
	}
	//modsPend
	if(NBIOPollsterSync_isSet(opq->modsPend)){
		NBIOPollsterSync_release(&opq->modsPend);
		NBIOPollsterSync_null(&opq->modsPend);
	}
	//sockets
	{
		//arr
		{
			SI32 i; for(i = 0; i < opq->fds.arr.use; i++){
				STNBIOPollsterFd* s = NBArray_itmPtrAtIndex(&opq->fds.arr, STNBIOPollsterFd, i);
				NBIOPollsterFd_release(s);
			}
			NBArray_empty(&opq->fds.arr);
			NBArray_release(&opq->fds.arr);
		}
		//fds
		{
			NBArray_empty(&opq->fds.fds);
			NBArray_release(&opq->fds.fds);
		}
	}
}

#ifdef NB_CONFIG_INCLUDE_ASSERTS
BOOL NBIOPollster_dbgValidateLockedOpq_(STNBIOPollsterOpq* opq){
	BOOL r = TRUE;
	//Validate pend-array
	{
		STNBIOPollsterSyncOpq* syncOpq = (STNBIOPollsterSyncOpq*)opq->modsPend.opaque;
		if(syncOpq != NULL && syncOpq->modsPend.arr.use != 0){
			NBObject_lock(syncOpq);
			{
				r = NBIOPollsterSync_dbgValidateLockedOpq_(syncOpq);
				NBASSERT(r);
			}
			NBObject_unlock(syncOpq);
		}
	}
	//Validate live-array
	if(opq->fds.arr.use != opq->fds.fds.use){
		NBASSERT(FALSE) //arrays shoudl be same size
		r = FALSE;
	} else {
		UI32 enabledCount = 0;
		SI32 i; for(i = 0; i < opq->fds.arr.use; i++){
			STNBIOPollsterFd* s = NBArray_itmPtrAtIndex(&opq->fds.arr, STNBIOPollsterFd, i);
			struct pollfd* fd2 = NBArray_itmPtrAtIndex(&opq->fds.fds, struct pollfd, i);
			//validate flags
			if((s->opsMask & ~ENNBIOPollsterOpBits_All) != 0){
#               ifdef NB_CONFIG_INCLUDE_ASSERTS
                PRINTF_ERROR("NBIOPollster, IO has unsupported bit-flags; syncs(%llu) ticks{%llu, enabled(%llu), disabled(%llu), ops(%llu) nops(%llu)}.\n", s->dbg.syncsCount, s->dbg.ticksCount, s->dbg.ticksEnabledCount, s->dbg.ticksDisabledCount, s->dbg.ticksOpCount, s->dbg.ticksNopCount);
#               endif
				NBASSERT(FALSE) //unsupported extra bits enabled
				r = FALSE;
			}
			//validate enabledCount
			if(s->opsMask & ENNBIOPollsterOpBits_All){
				if(!s->isInvalid && !s->isEnabled){
#                   ifdef NB_CONFIG_INCLUDE_ASSERTS
                    PRINTF_ERROR("NBIOPollster, IO should be enabled; syncs(%llu) ticks{%llu, enabled(%llu), disabled(%llu), ops(%llu) nops(%llu)}.\n", s->dbg.syncsCount, s->dbg.ticksCount, s->dbg.ticksEnabledCount, s->dbg.ticksDisabledCount, s->dbg.ticksOpCount, s->dbg.ticksNopCount);
#                   endif
					NBASSERT(FALSE) //Must be enabled
					r = FALSE;
				}
			} else if(!s->isInvalid && s->isEnabled){
#               ifdef NB_CONFIG_INCLUDE_ASSERTS
                PRINTF_ERROR("NBIOPollster, IO should be disabled; syncs(%llu) ticks{%llu, enabled(%llu), disabled(%llu), ops(%llu) nops(%llu)}.\n", s->dbg.syncsCount, s->dbg.ticksCount, s->dbg.ticksEnabledCount, s->dbg.ticksDisabledCount, s->dbg.ticksOpCount, s->dbg.ticksNopCount);
#               endif
				NBASSERT(FALSE) //Must be disabled
				r = FALSE;
			}
			//validate fd
			if(s->isEnabled){
                if(!s->isInvalid && !NB_POLL_FD_IS_ENABLED_VALID(fd2->fd)){
#                   ifdef NB_CONFIG_INCLUDE_ASSERTS
                    PRINTF_ERROR("NBIOPollster, IO fd(%lld) should be positive; syncs(%llu) ticks{%llu, enabled(%llu), disabled(%llu), ops(%llu) nops(%llu)}.\n", (SI64)fd2->fd, s->dbg.syncsCount, s->dbg.ticksCount, s->dbg.ticksEnabledCount, s->dbg.ticksDisabledCount, s->dbg.ticksOpCount, s->dbg.ticksNopCount);
#                   endif
                    NBASSERT(NB_POLL_FD_IS_ENABLED_VALID(fd2->fd)) //fd must be positive when enabled
                    r = FALSE;
                }
				enabledCount++;
			} else if(!s->isInvalid && !NB_POLL_FD_IS_DISABLED_VALID(fd2->fd)){
#               ifdef NB_CONFIG_INCLUDE_ASSERTS
                PRINTF_ERROR("NBIOPollster, IO fd(%lld) should be negative; syncs(%llu) ticks{%llu, enabled(%llu), disabled(%llu), ops(%llu) nops(%llu)}.\n", (SI64)fd2->fd, s->dbg.syncsCount, s->dbg.ticksCount, s->dbg.ticksEnabledCount, s->dbg.ticksDisabledCount, s->dbg.ticksOpCount, s->dbg.ticksNopCount);
#               endif
                NBASSERT(NB_POLL_FD_IS_DISABLED_VALID(fd2->fd)) //fd must be negative when disabled
                r = FALSE;
			}
		}
		if(enabledCount != opq->fds.enabledCount){
#           ifdef NB_CONFIG_INCLUDE_ASSERTS
            PRINTF_ERROR("NBIOPollster, IO enabled count miss-match.\n");
#           endif
			NBASSERT(FALSE) //enabledCount does not match
			r = FALSE;
		}
	}
	return r;
}
#endif

//cfg

//when enabled, internal locks are disabled for efficiency but non-parallels calls must be ensured by user
void NBIOPollster_setUnsafeMode(STNBIOPollsterRef ref, const BOOL unsafeEnabled){
	STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref));
	NB_POLLSTER_AUTOSAFE_ACTION_START(opq)
	{
		opq->unsafe.isEnabled = unsafeEnabled;
	}
	NB_POLLSTER_AUTOSAFE_ACTION_END(opq)
}

void NBIOPollster_setLstnr(STNBIOPollsterRef ref, STNBIOPollsterLstnrItf* itf, void* usrData){
	STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref));
	NB_POLLSTER_AUTOSAFE_ACTION_START(opq)
	{
		if(itf == NULL){
			NBMemory_setZeroSt(opq->lstnr, STNBIOPollsterLstnr);
		} else {
			opq->lstnr.itf = *itf;
			opq->lstnr.usrData = usrData;
		}
	}
	NB_POLLSTER_AUTOSAFE_ACTION_END(opq)
}


//fds

BOOL NBIOPollster_addFdOpq_(STNBIOPollsterOpq* opq, const STNBIOLnk* ioLnk, STNBSocketRef socket, STNBFileRef file, const STNBIOPollsterLstrnItf* itf, ENNBIOPollsterOpBit opsMask, const ENNBIOPollsterPollMode pollMode, void* usrData){
	BOOL r = FALSE;
	NBASSERT(!opq->unsafe.isEnabled) //in unsafe mode, actions must be done at callback
	if(!opq->unsafe.isEnabled && NBSocket_isSet(socket)){
		//add IO
		NB_POLLSTER_AUTOSAFE_ACTION_START(opq)
		{
            r = NBIOPollsterSync_addFdOpq_((STNBIOPollsterSyncOpq*)opq->modsPend.opaque, ioLnk, socket, file, itf, opsMask, pollMode, usrData);
		}
		NB_POLLSTER_AUTOSAFE_ACTION_END(opq)
	}
	return r;
}

//ioLnk

BOOL NBIOPollster_addIOLnk(STNBIOPollsterRef ref, const STNBIOLnk* ioLnk, const ENNBIOPollsterOpBit opsMask, const ENNBIOPollsterPollMode pollMode, void* usrData){
    STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref))
    return NBIOPollster_addFdOpq_(opq, ioLnk, NB_OBJREF_NULL, NB_OBJREF_NULL, NULL, opsMask, pollMode, usrData);
}

BOOL NBIOPollster_addIOLnkWithItf(STNBIOPollsterRef ref, const STNBIOLnk* ioLnk, const ENNBIOPollsterOpBit opsMask, const STNBIOPollsterLstrnItf* itf, void* usrData){
    STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref))
    NBASSERT(!opq->unsafe.isEnabled) //in unsafe mode, actions must be done at callback
    return NBIOPollster_addFdOpq_(opq, ioLnk, NB_OBJREF_NULL, NB_OBJREF_NULL, itf, opsMask, ENNBIOPollsterPollMode_DisableFoundOps, usrData);
}

BOOL NBIOPollster_enableIOLnkOps(STNBIOPollsterRef ref, const STNBIOLnk* ioLnk, const ENNBIOPollsterOpBit opsMask){
    BOOL r = FALSE;
    STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref))
    NBASSERT(!opq->unsafe.isEnabled) //in unsafe mode, actions must be done at callback
    if(!opq->unsafe.isEnabled && ioLnk != NULL){
        NB_POLLSTER_AUTOSAFE_ACTION_START(opq)
        {
            r = NBIOPollsterSync_enableIOLnkOps(opq->modsPend, ioLnk, opsMask);
        }
        NB_POLLSTER_AUTOSAFE_ACTION_END(opq)
    }
    return r;
}

BOOL NBIOPollster_disableIOLnkOps(STNBIOPollsterRef ref, const STNBIOLnk* ioLnk, const ENNBIOPollsterOpBit opsMask){
    BOOL r = FALSE;
    STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref))
    NBASSERT(!opq->unsafe.isEnabled) //in unsafe mode, actions must be done at callback
    if(!opq->unsafe.isEnabled && ioLnk != NULL){
        NB_POLLSTER_AUTOSAFE_ACTION_START(opq)
        {
            r = NBIOPollsterSync_disableIOLnkOps(opq->modsPend, ioLnk, opsMask);
        }
        NB_POLLSTER_AUTOSAFE_ACTION_END(opq)
    }
    return r;
}

BOOL NBIOPollster_updateIOLnkFD(STNBIOPollsterRef ref, const STNBIOLnk* ioLnk){
    BOOL r = FALSE;
    STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref))
    NBASSERT(!opq->unsafe.isEnabled) //in unsafe mode, actions must be done at callback
    if(!opq->unsafe.isEnabled && ioLnk != NULL){
        NB_POLLSTER_AUTOSAFE_ACTION_START(opq)
        {
            r = NBIOPollsterSync_updateIOLnkFD(opq->modsPend, ioLnk);
        }
        NB_POLLSTER_AUTOSAFE_ACTION_END(opq)
    }
    return r;
}

BOOL NBIOPollster_removeIOLnk(STNBIOPollsterRef ref, const STNBIOLnk* ioLnk){
    BOOL r = FALSE;
    STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref))
    NBASSERT(!opq->unsafe.isEnabled) //in unsafe mode, actions must be done at callback
    if(!opq->unsafe.isEnabled && ioLnk != NULL){
        NB_POLLSTER_AUTOSAFE_ACTION_START(opq)
        {
            r = NBIOPollsterSync_removeIOLnk(opq->modsPend, ioLnk);
        }
        NB_POLLSTER_AUTOSAFE_ACTION_END(opq)
    }
    return r;
}

//sockets

BOOL NBIOPollster_addSocket(STNBIOPollsterRef ref, STNBSocketRef socket, const ENNBIOPollsterOpBit opsMask, const ENNBIOPollsterPollMode pollMode, void* usrData){
	STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref))
	return NBIOPollster_addFdOpq_(opq, NULL, socket, NB_OBJREF_NULL, NULL, opsMask, pollMode, usrData);
}

BOOL NBIOPollster_addSocketWithItf(STNBIOPollsterRef ref, STNBSocketRef socket, const ENNBIOPollsterOpBit opsMask, const STNBIOPollsterLstrnItf* itf, void* usrData){
	STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref))
	NBASSERT(!opq->unsafe.isEnabled) //in unsafe mode, actions must be done at callback
	return NBIOPollster_addFdOpq_(opq, NULL, socket, NB_OBJREF_NULL, itf, opsMask, ENNBIOPollsterPollMode_DisableFoundOps, usrData);
}

BOOL NBIOPollster_removeSocket(STNBIOPollsterRef ref, STNBSocketRef socket){
    BOOL r = FALSE;
    STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref))
    NBASSERT(!opq->unsafe.isEnabled) //in unsafe mode, actions must be done at callback
    if(!opq->unsafe.isEnabled && NBSocket_isSet(socket)){
        NB_POLLSTER_AUTOSAFE_ACTION_START(opq)
        {
            r = NBIOPollsterSync_removeSocket(opq->modsPend, socket);
        }
        NB_POLLSTER_AUTOSAFE_ACTION_END(opq)
    }
    return r;
}

BOOL NBIOPollster_enableSocketOps(STNBIOPollsterRef ref, STNBSocketRef socket, const ENNBIOPollsterOpBit opsMask){
	BOOL r = FALSE;
	STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref))
	NBASSERT(!opq->unsafe.isEnabled) //in unsafe mode, actions must be done at callback
	if(!opq->unsafe.isEnabled && NBSocket_isSet(socket)){
		NB_POLLSTER_AUTOSAFE_ACTION_START(opq)
		{
            r = NBIOPollsterSync_enableSocketOps(opq->modsPend, socket, opsMask);
		}
		NB_POLLSTER_AUTOSAFE_ACTION_END(opq)
	}
	return r;
}

BOOL NBIOPollster_disableSocketOps(STNBIOPollsterRef ref, STNBSocketRef socket, const ENNBIOPollsterOpBit opsMask){
	BOOL r = FALSE;
	STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref))
	NBASSERT(!opq->unsafe.isEnabled) //in unsafe mode, actions must be done at callback
	if(!opq->unsafe.isEnabled && NBSocket_isSet(socket)){
		NB_POLLSTER_AUTOSAFE_ACTION_START(opq)
		{
            r = NBIOPollsterSync_disableSocketOps(opq->modsPend, socket, opsMask);
		}
		NB_POLLSTER_AUTOSAFE_ACTION_END(opq)
	}
	return r;
}

BOOL NBIOPollster_updateSocketFD(STNBIOPollsterRef ref, STNBSocketRef socket){
	BOOL r = FALSE;
	STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref))
	NBASSERT(!opq->unsafe.isEnabled) //in unsafe mode, actions must be done at callback
	if(!opq->unsafe.isEnabled && NBSocket_isSet(socket)){
		NB_POLLSTER_AUTOSAFE_ACTION_START(opq)
		{
            r = NBIOPollsterSync_updateSocketFD(opq->modsPend, socket);
		}
		NB_POLLSTER_AUTOSAFE_ACTION_END(opq)
	}
	return r;
}

//files

BOOL NBIOPollster_addFile(STNBIOPollsterRef ref, STNBFileRef file, const ENNBIOPollsterOpBit opsMask, const ENNBIOPollsterPollMode pollMode, void* usrData){
    STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref))
    return NBIOPollster_addFdOpq_(opq, NULL, NB_OBJREF_NULL, file, NULL, opsMask, pollMode, usrData);
}

BOOL NBIOPollster_addFileWithItf(STNBIOPollsterRef ref, STNBFileRef file, const ENNBIOPollsterOpBit opsMask, const STNBIOPollsterLstrnItf* itf, void* usrData){
    STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref))
    NBASSERT(!opq->unsafe.isEnabled) //in unsafe mode, actions must be done at callback
    return NBIOPollster_addFdOpq_(opq, NULL, NB_OBJREF_NULL, file, itf, opsMask, ENNBIOPollsterPollMode_DisableFoundOps, usrData);
}

BOOL NBIOPollster_removeFile(STNBIOPollsterRef ref, STNBFileRef file){
    BOOL r = FALSE;
    STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref))
    NBASSERT(!opq->unsafe.isEnabled) //in unsafe mode, actions must be done at callback
    if(!opq->unsafe.isEnabled && NBFile_isSet(file)){
        NB_POLLSTER_AUTOSAFE_ACTION_START(opq)
        {
            r = NBIOPollsterSync_removeFile(opq->modsPend, file);
        }
        NB_POLLSTER_AUTOSAFE_ACTION_END(opq)
    }
    return r;
}

BOOL NBIOPollster_enableFileOps(STNBIOPollsterRef ref, STNBFileRef file, const ENNBIOPollsterOpBit opsMask){
    BOOL r = FALSE;
    STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref))
    NBASSERT(!opq->unsafe.isEnabled) //in unsafe mode, actions must be done at callback
    if(!opq->unsafe.isEnabled && NBFile_isSet(file)){
        NB_POLLSTER_AUTOSAFE_ACTION_START(opq)
        {
            r = NBIOPollsterSync_enableFileOps(opq->modsPend, file, opsMask);
        }
        NB_POLLSTER_AUTOSAFE_ACTION_END(opq)
    }
    return r;
}

BOOL NBIOPollster_disableFileOps(STNBIOPollsterRef ref, STNBFileRef file, const ENNBIOPollsterOpBit opsMask){
    BOOL r = FALSE;
    STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref))
    NBASSERT(!opq->unsafe.isEnabled) //in unsafe mode, actions must be done at callback
    if(!opq->unsafe.isEnabled && NBFile_isSet(file)){
        NB_POLLSTER_AUTOSAFE_ACTION_START(opq)
        {
            r = NBIOPollsterSync_disableFileOps(opq->modsPend, file, opsMask);
        }
        NB_POLLSTER_AUTOSAFE_ACTION_END(opq)
    }
    return r;
}

BOOL NBIOPollster_updateFileFD(STNBIOPollsterRef ref, STNBFileRef file){
    BOOL r = FALSE;
    STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref))
    NBASSERT(!opq->unsafe.isEnabled) //in unsafe mode, actions must be done at callback
    if(!opq->unsafe.isEnabled && NBFile_isSet(file)){
        NB_POLLSTER_AUTOSAFE_ACTION_START(opq)
        {
            r = NBIOPollsterSync_updateFileFD(opq->modsPend, file);
        }
        NB_POLLSTER_AUTOSAFE_ACTION_END(opq)
    }
    return r;
}

//poll

void NBIOPollster_syncPollArrayLockedOpq_(STNBIOPollsterOpq* opq, STNBArray* dstRmvdToNotify /*STNBIOPollsterFd*/){
    NBASSERT(dstRmvdToNotify != NULL)
	//consume pend actions
	{
		STNBIOPollsterSyncOpq* syncOpq = (STNBIOPollsterSyncOpq*)opq->modsPend.opaque;
		if(syncOpq->modsPend.arr.use > 0){
			NBObject_lock(syncOpq);
			{
				SI32 i, i2; BOOL fnd;
				for(i = 0; i < syncOpq->modsPend.arr.use; i++){
					STNBIOPollsterUpdPend* s = NBArray_itmPtrAtIndex(&syncOpq->modsPend.arr, STNBIOPollsterUpdPend, i);
					NBASSERT((s->actionsMask & ENNBIOPollsterUpdActionBits_All) || s->addOpsMask != 0 || s->rmvOpsMask != 0) //program-logic error
					fnd = FALSE;
					if(s->actionsMask & ENNBIOPollsterUpdActionBit_Remove){
						//remove-action
						for(i2 = 0; i2 < opq->fds.arr.use; i2++){
							STNBIOPollsterFd* s2 = NBArray_itmPtrAtIndex(&opq->fds.arr, STNBIOPollsterFd, i2);
                            if(NBIOLnk_isSame(&s->link, &s2->link)){
                                STNBIOPollsterFd sFnd = *s2; //use local variable because array will be re-arranged after 'NBArray_removeItemAtIndex'
#                               ifdef NB_CONFIG_INCLUDE_ASSERTS
                                //dbg
                                {
                                    s2->dbg.syncsCount++;
                                }
#                               endif
								//remove
								{
									if(sFnd.isEnabled){
										NBASSERT(opq->fds.enabledCount > 0)
										opq->fds.enabledCount--;
									}
									NBArray_removeItemAtIndex(&opq->fds.arr, i2);
									NBArray_removeItemAtIndex(&opq->fds.fds, i2);
								}
                                //to notify
                                NBASSERT(dstRmvdToNotify != NULL)
                                if(dstRmvdToNotify != NULL){
                                    PRINTF_INFO("NBIOPollster_syncPollArrayLockedOpq_ adding removed IO to usrData '%lld'.\n", (SI64)sFnd.usrData);
                                    NBArray_addValue(dstRmvdToNotify, sFnd);
                                }
								fnd = TRUE;
								break;
							}
						} NBASSERT(fnd) //must be found
					} else if(s->actionsMask & ENNBIOPollsterUpdActionBit_Add){
						//add-action
						STNBIOPollsterFd s2;
						struct pollfd fd2;
						NBIOPollsterFd_init(&s2);
						NBMemory_setZeroSt(fd2, struct pollfd);
                        NBIOLnk_set(&s2.link, &s->link);
						s2.opsMask		= s->addOpsMask & ~s->rmvOpsMask;
						s2.isEnabled	= (s2.opsMask & ENNBIOPollsterOpBits_All);
						s2.pollMode		= s->pollMode;
						s2.itf			= s->itf;
						s2.usrData		= s->usrData;
                        {
                            fd2.events  = (s2.opsMask & ENNBIOPollsterOpBit_Read ? NB_POLL_READ : 0) | (s2.opsMask & ENNBIOPollsterOpBit_Write ? NB_POLL_WRITE : 0);
                            fd2.fd      = NBIOLnk_getFD(&s->link);
                        }
						if (fd2.fd < 0) {
							s2.isInvalid = TRUE;
							s2.isEnabled = FALSE;
						} else {
							s2.isInvalid = FALSE;
							if (!s2.isEnabled) {
								NB_POLL_FD_INVERT_SIGN(fd2.fd);
								NBASSERT(NB_POLL_FD_IS_DISABLED_VALID(fd2.fd));
							} else {
								NBASSERT(NB_POLL_FD_IS_ENABLED_VALID(fd2.fd));
							}
						}
						NBArray_addValue(&opq->fds.arr, s2);
						NBArray_addValue(&opq->fds.fds, fd2);
						//update 'enabledCount'
						if(s2.isEnabled){
							opq->fds.enabledCount++;
						}
					} else {
						//update-action
						for(i2 = 0; i2 < opq->fds.arr.use; i2++){
							STNBIOPollsterFd* s2 = NBArray_itmPtrAtIndex(&opq->fds.arr, STNBIOPollsterFd, i2);
                            if(NBIOLnk_isSame(&s->link, &s2->link)){
								struct pollfd* fd2 = NBArray_itmPtrAtIndex(&opq->fds.fds, struct pollfd, i2);
								s2->opsMask		= (s2->opsMask | s->addOpsMask) & ~s->rmvOpsMask;
								fd2->events		= (s2->opsMask & ENNBIOPollsterOpBit_Read ? NB_POLL_READ : 0) | (s2->opsMask & ENNBIOPollsterOpBit_Write ? NB_POLL_WRITE : 0);
#                               ifdef NB_CONFIG_INCLUDE_ASSERTS
                                //dbg
                                {
                                    s2->dbg.syncsCount++;
                                }
#                               endif
                                //update isEnabled status
								if(s2->opsMask & ENNBIOPollsterOpBits_All){
									if(!s2->isInvalid && !s2->isEnabled){
                                        NB_POLL_FD_INVERT_SIGN(fd2->fd);
										s2->isEnabled = TRUE;
										opq->fds.enabledCount++;
                                        NBASSERT(NB_POLL_FD_IS_ENABLED_VALID(fd2->fd));
									}
								} else if(!s2->isInvalid && s2->isEnabled){
									s2->isEnabled = FALSE;
									NBASSERT(opq->fds.enabledCount > 0)
									opq->fds.enabledCount--;
                                    NB_POLL_FD_INVERT_SIGN(fd2->fd);
                                    NBASSERT(NB_POLL_FD_IS_DISABLED_VALID(fd2->fd));
								}
								//sync fd
								if(s->syncFd){
                                    fd2->fd = NBIOLnk_getFD(&s2->link);
									if (fd2->fd < 0) {
										s2->isInvalid = TRUE;
										s2->isEnabled = FALSE;
									} else {
										s2->isInvalid = FALSE;
										if (!s2->isEnabled) {
											NB_POLL_FD_INVERT_SIGN(fd2->fd);
											NBASSERT(NB_POLL_FD_IS_DISABLED_VALID(fd2->fd));
										} else {
											NBASSERT(NB_POLL_FD_IS_ENABLED_VALID(fd2->fd));
										}
									}
								}
								//Apply usrData
								if(s->actionsMask & ENNBIOPollsterUpdActionBit_ApplyParams){
									s2->pollMode	= s->pollMode;
									s2->itf			= s->itf;
									s2->usrData		= s->usrData;
								}
								fnd = TRUE;
								break;
							}
						} NBASSERT(fnd) //must be found
					}
					//release
					NBIOPollsterUpdPend_release(s);
				}
				NBArray_empty(&syncOpq->modsPend.arr);
			}
			NBObject_unlock(syncOpq);
		}
	}
	NBASSERT(NBIOPollster_dbgValidateLockedOpq_(opq)) //validation fail
	//sync FDs (ToDo: remove)
	{
		SI32 i; for(i = 0; i < opq->fds.arr.use; i++){
			STNBIOPollsterFd* s = NBArray_itmPtrAtIndex(&opq->fds.arr, STNBIOPollsterFd, i);
			if(!s->isInvalid && s->isEnabled){
				struct pollfd* fd = NBArray_itmPtrAtIndex(&opq->fds.fds, struct pollfd, i);
                fd->fd = NBIOLnk_getFD(&s->link);
				if (fd->fd < 0) {
					s->isInvalid = TRUE;
					s->isEnabled = FALSE;
				} else {
					s->isInvalid = FALSE;
					s->isEnabled = TRUE;
					NBASSERT(NB_POLL_FD_IS_ENABLED_VALID(fd->fd));
				}
			}
		}
	}
	NBASSERT(NBIOPollster_dbgValidateLockedOpq_(opq)) //validation fail
}

void NBIOPollster_removeAllLockedOpq_(STNBIOPollsterOpq* opq, STNBArray* dstRmvdToNotify /*STNBIOPollsterFd*/){
	//sync
	//if(syncOpq->modsPend.arr.use > 0) //ToDo: call only if pend task is not empty ()
	{
		NBIOPollster_syncPollArrayLockedOpq_(opq, dstRmvdToNotify);
	}
	//remove all IOs
	{
		SI32 i; for(i = (SI32)opq->fds.arr.use - 1; i >= 0; i--){
			STNBIOPollsterFd s = NBArray_itmValueAtIndex(&opq->fds.arr, STNBIOPollsterFd, i);
			//remove
			{
				if(s.isEnabled){
					NBASSERT(opq->fds.enabledCount > 0)
					opq->fds.enabledCount--;
				}
				NBArray_removeItemAtIndex(&opq->fds.arr, i);
				NBArray_removeItemAtIndex(&opq->fds.fds, i);
			}
            //to notify
            NBASSERT(dstRmvdToNotify != NULL)
            if(dstRmvdToNotify != NULL){
                PRINTF_INFO("NBIOPollster_removeAllLockedOpq_ adding removed IO to usrData '%lld'.\n", (SI64)s.usrData);
                NBArray_addValue(dstRmvdToNotify, s);
            }
		}
	}
	NBASSERT(opq->fds.arr.use == 0 && opq->fds.fds.use == 0)
}

UI8 NBIOPollster_socketPoll(STNBSocketRef socket, const ENNBIOPollsterOpBit opsMask, const SI32 msTimeout){
	UI8 r = 0;
	if(NBSocket_isSet(socket)){
		const int sd = NBSocket_getFD_(socket);
		if (sd < 0) {
			r = ENNBIOPollsterOpBit_Error;
		} else {
			const BOOL isRead = (opsMask & ENNBIOPollsterOpBit_Read);
			const BOOL isWrite = (opsMask & ENNBIOPollsterOpBit_Write);
			if(isRead || isWrite){
				//using 'poll()'
				struct pollfd fd;
				NBMemory_setZeroSt(fd, struct pollfd);
				fd.fd = sd;
				fd.events = (isRead ? NB_POLL_READ : 0) | (isWrite ? NB_POLL_WRITE : 0);
				if (NB_POLL_METHOD(&fd, 1, msTimeout) > 0){
					r = ((fd.revents & NB_POLL_READ ? (opsMask & ENNBIOPollsterOpBit_Read) : 0) | (fd.revents & NB_POLL_WRITE ? (opsMask & ENNBIOPollsterOpBit_Write) : 0) | (fd.revents & NB_POLL_ERR ? ENNBIOPollsterOpBit_Error : 0) | (fd.revents & NB_POLL_HUP ? ENNBIOPollsterOpBit_HUP : 0) | (fd.revents & NB_POLL_NVAL ? ENNBIOPollsterOpBit_NVAL : 0));
				}
			}
		}
	}
	return r;
}

UI8 NBIOPollster_filePoll(STNBFileRef file, const ENNBIOPollsterOpBit opsMask, const SI32 msTimeout){
    UI8 r = 0;
    if(NBFile_isSet(file)){
        const int sd = NBFile_getFD(file);
		if (sd < 0) {
			r = ENNBIOPollsterOpBit_Error;
		} else {
            const BOOL isRead = (opsMask & ENNBIOPollsterOpBit_Read);
            const BOOL isWrite = (opsMask & ENNBIOPollsterOpBit_Write);
            if(isRead || isWrite){
                //using 'poll()'
                struct pollfd fd;
                NBMemory_setZeroSt(fd, struct pollfd);
                fd.fd = sd;
                fd.events = (isRead ? NB_POLL_READ : 0) | (isWrite ? NB_POLL_WRITE : 0);
                if (NB_POLL_METHOD(&fd, 1, msTimeout) > 0){
                    r = ((fd.revents & NB_POLL_READ ? (opsMask & ENNBIOPollsterOpBit_Read) : 0) | (fd.revents & NB_POLL_WRITE ? (opsMask & ENNBIOPollsterOpBit_Write) : 0) | (fd.revents & NB_POLL_ERR ? ENNBIOPollsterOpBit_Error : 0) | (fd.revents & NB_POLL_HUP ? ENNBIOPollsterOpBit_HUP : 0) | (fd.revents & NB_POLL_NVAL ? ENNBIOPollsterOpBit_NVAL : 0));
                }
            }
        }
    }
    return r;
}

UI32 NBIOPollster_pollTo(STNBIOPollsterRef ref, const SI32 msTimeout, STNBIOPollsterSyncRef syncTasks, STNBIOPollsterResult* dst){
	UI32 r = 0;
	STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref))
	if(dst != NULL){
        STNBArray rmvdToNotify; //STNBIOPollsterFd
        NBArray_initWithSz(&rmvdToNotify, sizeof(STNBIOPollsterFd), NULL, 0, 32, 0.1f);
		NB_POLLSTER_AUTOSAFE_ACTION_START(opq)
		//sync
		//if(syncOpq->modsPend.arr.use > 0) //ToDo: call only if pend task is not empty ()
		{
			NBIOPollster_syncPollArrayLockedOpq_(opq, &rmvdToNotify);
		}
		if(opq->fds.enabledCount <= 0){
			//sleep
			if(msTimeout > 0){
				NBThread_mSleep(msTimeout);
			}
		} else {
			//poll
			SI32 rr; BOOL anyEnabledObj = FALSE;
			STNBIOPollsterFd* sckt;
			struct pollfd* fd; const struct pollfd* fdAfterEnd;
			opq->fds.isPolling = TRUE;
			sckt		= NBArray_dataPtr(&opq->fds.arr, STNBIOPollsterFd);
			fd			= NBArray_dataPtr(&opq->fds.fds, struct pollfd);
			fdAfterEnd	= fd + opq->fds.fds.use;
			NBASSERT(opq->fds.arr.use == opq->fds.fds.use)
			NB_POLLSTER_AUTOSAFE_ACTION_END(opq)
			{
				dst->recordsUse = 0; NBASSERT(r == 0)
				//resize dst (if necesary)
				if(dst->recordsSz < opq->fds.arr.use){
					NBIOPollsterResult_resize(dst, opq->fds.arr.use);
				}
				//pool
				NBASSERT(opq->fds.arr.use == opq->fds.fds.use)
				rr = NB_POLL_METHOD(fd, opq->fds.fds.use, msTimeout);
				//notify
				if(opq->lstnr.itf.pollReturned != NULL){
					(*opq->lstnr.itf.pollReturned)(ref, rr, opq->lstnr.usrData);
				}
				//results
				if(rr >= 0){
					//analize results
					STNBIOPollsterRecord* rDst; 
					STNBIOPollsterUpd reqUpd;
					BOOL isRead, isWrite, isError, isHUP, isNVAL;
					//add poll results
					NBASSERT(r == 0)
					while(fd < fdAfterEnd && r < rr){
						isRead	= ((fd->events & fd->revents) & NB_POLL_READ);
						isWrite	= ((fd->events & fd->revents) & NB_POLL_WRITE);
						isError = (fd->revents & NB_POLL_ERR);
                        isHUP   = (fd->revents & NB_POLL_HUP);
                        isNVAL  = (fd->revents & NB_POLL_NVAL);
						NBMemory_setZeroSt(reqUpd, STNBIOPollsterUpd);
						reqUpd.opsMasks = sckt->opsMask;
						if(isRead || isWrite || isError || isHUP || isNVAL){
							NBASSERT(r < dst->recordsSz)
							rDst				= dst->records + r;
							NBMemory_setZeroSt(*rDst, STNBIOPollsterRecord);
							rDst->link		    = sckt->link;
							rDst->itf			= sckt->itf;
							rDst->usrData		= sckt->usrData;
							rDst->opsMasks.req	= sckt->opsMask;
							rDst->opsMasks.rslt	= (isRead ? (sckt->opsMask & ENNBIOPollsterOpBit_Read) : 0) | (isWrite ? (sckt->opsMask & ENNBIOPollsterOpBit_Write) : 0) | (isError ? ENNBIOPollsterOpBit_Error : 0) | (isHUP ? ENNBIOPollsterOpBit_HUP : 0) | (isNVAL ? ENNBIOPollsterOpBit_NVAL : 0);
							//autodisable flags
							if(sckt->pollMode == ENNBIOPollsterPollMode_DisableFoundOps){
								if(isRead){
									reqUpd.opsMasks &= ~ENNBIOPollsterOpBit_Read;
								}
								if(isWrite){
									reqUpd.opsMasks &= ~ENNBIOPollsterOpBit_Write;
								}
								if(isError){
									reqUpd.opsMasks &= ~ENNBIOPollsterOpBit_Error;
								}
                                if(isHUP){
                                    reqUpd.opsMasks &= ~ENNBIOPollsterOpBit_HUP;
                                }
                                if(isNVAL){
                                    reqUpd.opsMasks &= ~ENNBIOPollsterOpBit_NVAL;
                                }
							}
							rDst->opsMasks.kept = reqUpd.opsMasks;
							NBASSERT(rDst->opsMasks.rslt) //should be non-zero
							//next
							r++;
						}
						//apply mask-change
						if(sckt->opsMask != reqUpd.opsMasks){
							sckt->opsMask	= reqUpd.opsMasks;
							fd->events		= (sckt->opsMask & ENNBIOPollsterOpBit_Read ? NB_POLL_READ : 0) | (sckt->opsMask & ENNBIOPollsterOpBit_Write ? NB_POLL_WRITE : 0);
							if(sckt->opsMask & ENNBIOPollsterOpBits_All){
								if(!sckt->isInvalid && !sckt->isEnabled){
                                    NB_POLL_FD_INVERT_SIGN(fd->fd);
									sckt->isEnabled = TRUE;
									opq->fds.enabledCount++;
									anyEnabledObj = TRUE;
                                    NBASSERT(NB_POLL_FD_IS_ENABLED_VALID(fd->fd))
								}
							} else if(!sckt->isInvalid && sckt->isEnabled){
								sckt->isEnabled = FALSE;
                                NB_POLL_FD_INVERT_SIGN(fd->fd);
								NBASSERT(opq->fds.enabledCount > 0)
								opq->fds.enabledCount--;
                                NBASSERT(NB_POLL_FD_IS_ENABLED_VALID(fd->fd))
							}
						}
						//sync FD
						if(reqUpd.syncFd){
                            fd->fd = NBIOLnk_getFD(&sckt->link);
							if (fd->fd < 0) {
								sckt->isInvalid = TRUE;
								sckt->isEnabled = FALSE;
							} else {
								sckt->isInvalid = FALSE;
								if (!sckt->isEnabled) {
									NB_POLL_FD_INVERT_SIGN(fd->fd);
									NBASSERT(NB_POLL_FD_IS_ENABLED_VALID(fd->fd))
								} else {
									NBASSERT(NB_POLL_FD_IS_ENABLED_VALID(fd->fd))
								}
							}
						}
						//validate FD
						//NBASSERT(sckt->isInvalid || fd->fd == NBIOLnk_getFD(&sckt->link) || fd->fd == -(NBIOLnk_getFD(&sckt->link))) //user logic error, fd changed but not synced
						//next
						fd++;
						sckt++;
					}
					NBASSERT(r <= opq->fds.arr.use)
					dst->recordsUse = r;
					NBASSERT(opq->fds.arr.use == opq->fds.fds.use)
					NBASSERT(NBIOPollster_dbgValidateLockedOpq_(opq)) //validation fail
#				ifdef NB_CONFIG_INCLUDE_ASSERTS
				} else {
					switch (errno) {
						case EFAULT:
							PRINTF_ERROR("NBIOPollster_pollTo, 'Fds points outside the process's accessible address space.'.\n");
							break;
						case EINTR:
							PRINTF_ERROR("NBIOPollster_pollTo, 'A signal occurred before any requested event.'.\n");
							break;
						case EINVAL:
							PRINTF_ERROR("NBIOPollster_pollTo, 'Size exceeds limit or timeout value is invalid.'.\n");
							break;
						case ENOMEM:
							PRINTF_ERROR("NBIOPollster_pollTo, 'Unable to allocate memory.'.\n");
							break;
						default:
							PRINTF_ERROR("NBIOPollster_pollTo, errno(%d) '%s'.\n", errno, strerror(errno));
							break;
					}
#				endif
				}
			}
			NB_POLLSTER_AUTOSAFE_ACTION_START_AGAIN(opq)
			opq->fds.isPolling = FALSE;
		}
		//move sync tasks (will be applied on next call)
		if(NBIOPollsterSync_isSet(syncTasks) && !NBIOPollsterSync_isEmpty(syncTasks)){
			NBIOPollsterSync_sendTo(syncTasks, opq->modsPend);
		}
		NB_POLLSTER_AUTOSAFE_ACTION_END(opq)
        //notify removed ones (unlocked)
        {
            SI32 i; for(i = 0; i < rmvdToNotify.use; i++){
                STNBIOPollsterFd* s = NBArray_itmPtrAtIndex(&rmvdToNotify, STNBIOPollsterFd, i);
                //notify
                if(s->itf.pollRemoved != NULL){
                    (*s->itf.pollRemoved)(s->link, s->usrData);
                }
                //release
                NBIOPollsterFd_release(s);
            }
            NBArray_empty(&rmvdToNotify);
            NBArray_release(&rmvdToNotify);
        }
	}
	return r;
}

BOOL NBIOPollster_pollAnyReady(STNBIOPollsterRef ref, const SI32 msTimeout, STNBIOPollsterSyncRef syncTasks){
	BOOL r = FALSE;
	STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref))
    STNBArray rmvdToNotify; //STNBIOPollsterFd
    NBArray_initWithSz(&rmvdToNotify, sizeof(STNBIOPollsterFd), NULL, 0, 32, 0.1f);
	NB_POLLSTER_AUTOSAFE_ACTION_START(opq)
	//sync
	//if(syncOpq->modsPend.arr.use > 0) //ToDo: call only if pend task is not empty ()
	{
		NBIOPollster_syncPollArrayLockedOpq_(opq, &rmvdToNotify);
	}
	if(opq->fds.fds.use <= 0){
		//sleep
		if(msTimeout > 0){
			NBThread_mSleep(msTimeout);
		}
	} else {
		//poll
		int rr; struct pollfd* fds; const struct pollfd* fdsAfterEnd; 
		opq->fds.isPolling = TRUE;
		fds = NBArray_dataPtr(&opq->fds.fds, struct pollfd);
		fdsAfterEnd = fds + opq->fds.fds.use;
		NBASSERT(opq->fds.arr.use == opq->fds.fds.use)
		NB_POLLSTER_AUTOSAFE_ACTION_END(opq)
		{
			//pool
			NBASSERT(opq->fds.arr.use == opq->fds.fds.use)
			rr = NB_POLL_METHOD(fds, opq->fds.fds.use, msTimeout);
			//notify
			if(opq->lstnr.itf.pollReturned != NULL){
				(*opq->lstnr.itf.pollReturned)(ref, rr, opq->lstnr.usrData);
			}
			//results
			if(rr > 0){
				//analize results
				int opsCount = 0;
				while(fds < fdsAfterEnd && opsCount < rr){
					if(fds->revents != 0){
						if(((fds->events & fds->revents) & NB_POLL_READ) || ((fds->events & fds->revents) & NB_POLL_WRITE)){
							r = TRUE;
							break;
						}
						opsCount++;
					}
					fds++;
				}
				NBASSERT(opq->fds.arr.use == opq->fds.fds.use)
#			ifdef NB_CONFIG_INCLUDE_ASSERTS
			} else if(rr != 0){
				switch (errno) {
					case EFAULT:
						PRINTF_ERROR("NBIOPollster_pollAnyReady, 'Fds points outside the process's accessible address space.'.\n");
						break;
					case EINTR:
						PRINTF_ERROR("NBIOPollster_pollAnyReady, 'A signal occurred before any requested event.'.\n");
						break;
					case EINVAL:
						PRINTF_ERROR("NBIOPollster_pollAnyReady, 'Size exceeds limit or timeout value is invalid.'.\n");
						break;
					case ENOMEM:
						PRINTF_ERROR("NBIOPollster_pollAnyReady, 'Unable to allocate memory.'.\n");
						break;
					default:
						PRINTF_ERROR("NBIOPollster_pollAnyReady, errno(%d) '%s'.\n", errno, strerror(errno));
						break;
				}
#			endif
			}
		}
		NB_POLLSTER_AUTOSAFE_ACTION_START_AGAIN(opq)
		opq->fds.isPolling = FALSE;
	}
	//move sync tasks (will be applied on next call)
	if(NBIOPollsterSync_isSet(syncTasks) && !NBIOPollsterSync_isEmpty(syncTasks)){
		NBIOPollsterSync_sendTo(syncTasks, opq->modsPend);
	}
	NB_POLLSTER_AUTOSAFE_ACTION_END(opq)
    //notify removed ones (unlocked)
    {
        SI32 i; for(i = 0; i < rmvdToNotify.use; i++){
            STNBIOPollsterFd* s = NBArray_itmPtrAtIndex(&rmvdToNotify, STNBIOPollsterFd, i);
            //notify
            if(s->itf.pollRemoved != NULL){
                (*s->itf.pollRemoved)(s->link, s->usrData);
            }
            //release
            NBIOPollsterFd_release(s);
        }
        NBArray_empty(&rmvdToNotify);
        NBArray_release(&rmvdToNotify);
    }
	return r;
}

//engine

BOOL NBIOPollster_engineStart(STNBIOPollsterRef ref){
	BOOL r = FALSE;
	STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref))
	NB_POLLSTER_AUTOSAFE_ACTION_START(opq)
	NBASSERT(!opq->engine.isRunning && !opq->fds.isPolling)
	if(!opq->engine.isRunning && !opq->fds.isPolling){
		opq->engine.isRunning = TRUE;
		r = TRUE;
	}
	NB_POLLSTER_AUTOSAFE_ACTION_END(opq)
	return r;
}

BOOL NBIOPollster_engineStop(STNBIOPollsterRef ref){
	BOOL r = FALSE;
	STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref))
    STNBArray rmvdToNotify; //STNBIOPollsterFd
    NBArray_initWithSz(&rmvdToNotify, sizeof(STNBIOPollsterFd), NULL, 0, 32, 0.1f);
	NB_POLLSTER_AUTOSAFE_ACTION_START(opq)
	NBASSERT(opq->engine.isRunning && !opq->fds.isPolling)
	if(opq->engine.isRunning && !opq->fds.isPolling){
		opq->engine.isRunning = FALSE;
		//empty array
		{
			NBIOPollster_removeAllLockedOpq_(opq, &rmvdToNotify);
		}
		r = TRUE;
	}
	NB_POLLSTER_AUTOSAFE_ACTION_END(opq)
    //notify removed ones (unlocked)
    {
        SI32 i; for(i = 0; i < rmvdToNotify.use; i++){
            STNBIOPollsterFd* s = NBArray_itmPtrAtIndex(&rmvdToNotify, STNBIOPollsterFd, i);
            //notify
            if(s->itf.pollRemoved != NULL){
                (*s->itf.pollRemoved)(s->link, s->usrData);
            }
            //release
            NBIOPollsterFd_release(s);
        }
        NBArray_empty(&rmvdToNotify);
        NBArray_release(&rmvdToNotify);
    }
	return r;
}

UI32 NBIOPollster_enginePoll(STNBIOPollsterRef ref, const SI32 msTimeout, STNBIOPollsterSyncRef syncTasks){
	UI32 r = 0;
	STNBIOPollsterOpq* opq = (STNBIOPollsterOpq*)ref.opaque; NBASSERT(NBIOPollster_isClass(ref))
    STNBArray rmvdToNotify; //STNBIOPollsterFd
    NBArray_initWithSz(&rmvdToNotify, sizeof(STNBIOPollsterFd), NULL, 0, 32, 0.1f);
	NB_POLLSTER_AUTOSAFE_ACTION_START(opq)
	NBASSERT(opq->engine.isRunning && !opq->fds.isPolling) //'NBIOPollster_engineStart()' should be called first
	if(opq->engine.isRunning && !opq->fds.isPolling){
		SI32 rr = 0; BOOL isRealPollCall = FALSE, anyEnabledObj = FALSE;
		//sync
		//if(syncOpq->modsPend.arr.use > 0) //ToDo: call only if pend task is not empty ()
		{
			NBIOPollster_syncPollArrayLockedOpq_(opq, &rmvdToNotify);
		}
		opq->fds.isPolling = TRUE;
		NBASSERT(opq->fds.arr.use == opq->fds.fds.use)
		NB_POLLSTER_AUTOSAFE_ACTION_END(opq)
		{
			IF_NBASSERT(STNBTimestampMicro timeLastAction = NBTimestampMicro_getMonotonicFast(), timeCur; SI64 usDiff;)
			//pool
			NBASSERT(opq->fds.arr.use == opq->fds.fds.use)
			if(opq->fds.enabledCount <= 0){
				NBASSERT(rr == 0)
				//sleep
				if(msTimeout > 0){
					NBThread_mSleep(msTimeout);
                    //PRINTF_INFO("NBIOPollster, slept %d ms (%d enabled).\n", (SI32)msTimeout, opq->fds.enabledCount);
				}
			} else {
                struct pollfd* fd = NBArray_dataPtr(&opq->fds.fds, struct pollfd);
				rr = NB_POLL_METHOD(fd, opq->fds.fds.use, msTimeout);
				isRealPollCall = TRUE;
                //PRINTF_INFO("NBIOPollster, polled %d fds (%d enabled).\n", opq->fds.fds.use, opq->fds.enabledCount);
			}
#			ifdef NB_CONFIG_INCLUDE_ASSERTS
			if(msTimeout > 0){
				timeCur	= NBTimestampMicro_getMonotonicFast();
				usDiff	= NBTimestampMicro_getDiffInUs(&timeLastAction, &timeCur);
				/*if(usDiff > ((msTimeout + 1) * 1000ULL)){
					PRINTF_INFO("NBIOPollster, poll%s took %llu.%llu%llu/%dms with %d results.\n", (isRealPollCall ? "" : "-sleep"), (usDiff / 1000ULL), (usDiff % 1000ULL) % 100ULL, (usDiff % 100ULL) % 10ULL, msTimeout, rr);
				}*/
				timeLastAction = timeCur;
			}
#			endif
			//notify
			if(opq->lstnr.itf.pollReturned != NULL){
				(*opq->lstnr.itf.pollReturned)(ref, rr, opq->lstnr.usrData);
#				ifdef NB_CONFIG_INCLUDE_ASSERTS
				{
					timeCur	= NBTimestampMicro_getMonotonicFast();
					usDiff	= NBTimestampMicro_getDiffInUs(&timeLastAction, &timeCur);
					/*if(usDiff >= 1000ULL){
						PRINTF_INFO("NBIOPollster, pollReturned-notification took %llu.%llu%llums.\n", (usDiff / 1000ULL), (usDiff % 1000ULL) % 100ULL, (usDiff % 100ULL) % 10ULL);
					}*/
					timeLastAction = timeCur;
				}
#				endif
			}
			//results
			if(rr >= 0){
				//add poll results
                {
                    UI8 opsFnd = 0; BOOL isRead = FALSE, isWrite = FALSE, isError = FALSE, isHUP = FALSE, isNVAL = FALSE;
                    STNBIOPollsterFd* sckt; struct pollfd* fd; const struct pollfd* fdAfterEnd;
                    sckt        = NBArray_dataPtr(&opq->fds.arr, STNBIOPollsterFd);
                    fd          = NBArray_dataPtr(&opq->fds.fds, struct pollfd);
                    fdAfterEnd  = fd + opq->fds.fds.use;
                    while(fd < fdAfterEnd /*&& opsCount < rr*/){ //ToDo: remove.
#                       ifdef NB_CONFIG_INCLUDE_ASSERTS
                        //dbg
                        {
                            sckt->dbg.ticksCount++;
                            if(sckt->isEnabled){
                                sckt->dbg.ticksEnabledCount++;
                            } else {
                                sckt->dbg.ticksDisabledCount++;
                            }
                        }
#                       endif
						if (sckt->isInvalid) {
							isError = TRUE;
						} else if(isRealPollCall){ //get mask (only if was a real-poll-call)
                            isRead	= ((fd->events & fd->revents) & NB_POLL_READ);
                            isWrite	= ((fd->events & fd->revents) & NB_POLL_WRITE);
                            isError = (fd->revents & NB_POLL_ERR);
                            isHUP   = (fd->revents & NB_POLL_HUP);
                            isNVAL  = (fd->revents & NB_POLL_NVAL);
                        }
                        NBMemory_setZeroSt(sckt->reqUpd, STNBIOPollsterUpd);
                        sckt->reqUpd.opsMasks = sckt->opsMask;
                        if(isRead || isWrite || isError || isHUP || isNVAL){
#                           ifdef NB_CONFIG_INCLUDE_ASSERTS
                            //dbg
                            {
                                sckt->dbg.ticksOpCount++;
                            }
#                           endif
                            opsFnd = (isRead ? (sckt->opsMask & ENNBIOPollsterOpBit_Read) : 0) | (isWrite ? (sckt->opsMask & ENNBIOPollsterOpBit_Write) : 0) | (isError ? ENNBIOPollsterOpBit_Error : 0) | (isHUP ? ENNBIOPollsterOpBit_HUP : 0) | (isNVAL ? ENNBIOPollsterOpBit_NVAL : 0);
                            //autodisable flags
                            if(!sckt->isInvalid && sckt->pollMode == ENNBIOPollsterPollMode_DisableFoundOps){
                                if(isRead)  sckt->reqUpd.opsMasks &= ~ENNBIOPollsterOpBit_Read;
                                if(isWrite) sckt->reqUpd.opsMasks &= ~ENNBIOPollsterOpBit_Write;
                                if(isError) sckt->reqUpd.opsMasks &= ~ENNBIOPollsterOpBit_Error;
                                if(isHUP)   sckt->reqUpd.opsMasks &= ~ENNBIOPollsterOpBit_HUP;
                                if(isNVAL)  sckt->reqUpd.opsMasks &= ~ENNBIOPollsterOpBit_NVAL;
                            }
                            //notify
                            if(sckt->itf.pollConsumeMask != NULL){
                                (*sckt->itf.pollConsumeMask)(sckt->link, opsFnd, &sckt->reqUpd, opq->modsPend, sckt->usrData);
                            }
                            //result
                            r++;
                        } else {
#                           ifdef NB_CONFIG_INCLUDE_ASSERTS
                            //dbg
                            {
                                sckt->dbg.ticksNopCount++;
                            }
#                           endif
                            //notify
                            if(sckt->itf.pollConsumeNoOp != NULL){
                                (*sckt->itf.pollConsumeNoOp)(sckt->link, &sckt->reqUpd, opq->modsPend, sckt->usrData);
                            }
                        }
                        //calculate call-time (for debugging)
#					    ifdef NB_CONFIG_INCLUDE_ASSERTS
                        {
                            timeCur	= NBTimestampMicro_getMonotonicFast();
                            usDiff	= NBTimestampMicro_getDiffInUs(&timeLastAction, &timeCur);
                            //if(usDiff >= 1000ULL){
                            //	PRINTF_INFO("NBIOPollster, pollConsume-notification took %llu.%llu%llums.\n", (usDiff / 1000ULL), (usDiff % 1000ULL) % 100ULL, (usDiff % 100ULL) % 10ULL);
                            //}
                            timeLastAction = timeCur;
                        }
#					    endif
                        //next
                        fd++;
                        sckt++;
                    }
                }
                //apply post-mask-changes
                {
                    STNBIOPollsterFd* sckt; struct pollfd* fd; const struct pollfd* fdAfterEnd;
                    sckt        = NBArray_dataPtr(&opq->fds.arr, STNBIOPollsterFd);
                    fd          = NBArray_dataPtr(&opq->fds.fds, struct pollfd);
                    fdAfterEnd  = fd + opq->fds.fds.use;
                    while(fd < fdAfterEnd /*&& opsCount < rr*/){ //ToDo: remove.
                        //upd (optional)
                        if(sckt->itf.pollGetReqUpd != NULL){
                            (*sckt->itf.pollGetReqUpd)(sckt->link, &sckt->reqUpd, sckt->usrData);
                        }
                        //apply mask-change
                        if(sckt->opsMask != sckt->reqUpd.opsMasks){
                            sckt->opsMask   = sckt->reqUpd.opsMasks;
                            fd->events      = (sckt->opsMask & ENNBIOPollsterOpBit_Read ? NB_POLL_READ : 0) | (sckt->opsMask & ENNBIOPollsterOpBit_Write ? NB_POLL_WRITE : 0);
                            if(sckt->opsMask & ENNBIOPollsterOpBits_All){
                                if(!sckt->isInvalid && !sckt->isEnabled){
                                    NB_POLL_FD_INVERT_SIGN(fd->fd);
                                    sckt->isEnabled = TRUE;
                                    opq->fds.enabledCount++;
                                    anyEnabledObj = TRUE;
                                    NBASSERT(NB_POLL_FD_IS_ENABLED_VALID(fd->fd));
                                }
                            } else if(!sckt->isInvalid && sckt->isEnabled){
                                sckt->isEnabled = FALSE;
                                NB_POLL_FD_INVERT_SIGN(fd->fd);
                                NBASSERT(opq->fds.enabledCount > 0)
                                opq->fds.enabledCount--;
                                NBASSERT(NB_POLL_FD_IS_DISABLED_VALID(fd->fd));
                            }
                        }
                        //sync FD
                        if(sckt->reqUpd.syncFd){
                            fd->fd = NBIOLnk_getFD(&sckt->link);
							if (fd->fd < 0) {
								sckt->isInvalid = TRUE;
								sckt->isEnabled = FALSE;
							} else {
								sckt->isInvalid = FALSE;
								if (!sckt->isEnabled) {
									NB_POLL_FD_INVERT_SIGN(fd->fd);
									NBASSERT(NB_POLL_FD_IS_DISABLED_VALID(fd->fd));
								} else {
									NBASSERT(NB_POLL_FD_IS_ENABLED_VALID(fd->fd));
								}
							}
                        }
                        //validate FD
                        //NBASSERT(sckt->isInvalid || fd->fd == NBIOLnk_getFD(&sckt->link) || fd->fd == -(NBIOLnk_getFD(&sckt->link))) //user logic error, fd changed but not synced
                        //calculate call-time (for debugging)
#                       ifdef NB_CONFIG_INCLUDE_ASSERTS
                        {
                            timeCur = NBTimestampMicro_getMonotonicFast();
                            usDiff  = NBTimestampMicro_getDiffInUs(&timeLastAction, &timeCur);
                            //if(usDiff >= 1000ULL){
                            //    PRINTF_INFO("NBIOPollster, pollGetReqUpd-notification took %llu.%llu%llums.\n", (usDiff / 1000ULL), (usDiff % 1000ULL) % 100ULL, (usDiff % 100ULL) % 10ULL);
                            //}
                            timeLastAction = timeCur;
                        }
#                       endif
                        //next
                        fd++;
                        sckt++;
                    }
                }
				NBASSERT(opq->fds.arr.use == opq->fds.fds.use)
				NBASSERT(NBIOPollster_dbgValidateLockedOpq_(opq)) //validation fail
#				ifdef NB_CONFIG_INCLUDE_ASSERTS
			} else {
				switch (errno) {
				case EFAULT:
					PRINTF_ERROR("NBIOPollster_poll, 'Fds points outside the process's accessible address space.'.\n");
					break;
				case EINTR:
					PRINTF_ERROR("NBIOPollster_poll, 'A signal occurred before any requested event.'.\n");
					break;
				case EINVAL:
					PRINTF_ERROR("NBIOPollster_poll, 'Size exceeds limit or timeout value is invalid.'.\n");
					break;
				case ENOMEM:
					PRINTF_ERROR("NBIOPollster_poll, 'Unable to allocate memory.'.\n");
					break;
				default:
					PRINTF_ERROR("NBIOPollster_poll, errno(%d) '%s'.\n", errno, strerror(errno));
					break;
				}
#				endif
			}
		}
		NB_POLLSTER_AUTOSAFE_ACTION_START_AGAIN(opq)
		//poll-end
		opq->fds.isPolling = FALSE;
		//accumulate sockets-sync-tasks (will be applied on next call)
		if(NBIOPollsterSync_isSet(syncTasks) && !NBIOPollsterSync_isEmpty(syncTasks)){
			NBIOPollsterSync_sendTo(syncTasks, opq->modsPend);
		}
	}
	NB_POLLSTER_AUTOSAFE_ACTION_END(opq)
    //notify removed ones (unlocked)
    {
        SI32 i; for(i = 0; i < rmvdToNotify.use; i++){
            STNBIOPollsterFd* s = NBArray_itmPtrAtIndex(&rmvdToNotify, STNBIOPollsterFd, i);
            //notify
            if(s->itf.pollRemoved != NULL){
                (*s->itf.pollRemoved)(s->link, s->usrData);
            }
            //release
            NBIOPollsterFd_release(s);
        }
        NBArray_empty(&rmvdToNotify);
        NBArray_release(&rmvdToNotify);
    }
	return r;
}

//NBIOPollsterUpdPend

void NBIOPollsterUpdPend_init(STNBIOPollsterUpdPend* obj){
	NBMemory_setZeroSt(*obj, STNBIOPollsterUpdPend);
    NBIOLnk_init(&obj->link);
}

void NBIOPollsterUpdPend_release(STNBIOPollsterUpdPend* obj){
    NBASSERT(NBIOLnk_isSet(&obj->link))
    if(NBIOLnk_isSet(&obj->link)){
        NBIOLnk_release(&obj->link);
        NBIOLnk_null(&obj->link);
    }
}

//NBIOPollsterFd

void NBIOPollsterFd_init(STNBIOPollsterFd* obj){
	NBMemory_setZeroSt(*obj, STNBIOPollsterFd);
    NBIOLnk_init(&obj->link);
}

void NBIOPollsterFd_release(STNBIOPollsterFd* obj){
	//release
    {
        if(NBIOLnk_isSet(&obj->link)){
            NBIOLnk_release(&obj->link);
            NBIOLnk_null(&obj->link);
        }
        NBMemory_setZeroSt(obj->itf, STNBIOPollsterLstrnItf);
    }
}

//NBIOPollsterResult

void NBIOPollsterResult_init(STNBIOPollsterResult* obj){
	NBMemory_setZeroSt(*obj, STNBIOPollsterResult);
}

void NBIOPollsterResult_release(STNBIOPollsterResult* obj){
	if(obj->records != NULL){
		NBMemory_free(obj->records);
		obj->records = NULL;
	}
	obj->recordsUse = obj->recordsSz = 0;
}

void NBIOPollsterResult_resize(STNBIOPollsterResult* obj, const UI32 sz){
	if(obj->recordsSz != sz){
		//release
		{
			if(obj->records != NULL){
				NBMemory_free(obj->records);
				obj->records = NULL;
			}
			obj->recordsUse = obj->recordsSz = 0;
		}
		//allocate
		if(sz > 0){
			obj->records	= NBMemory_allocTypes(STNBIOPollsterRecord, sz);
			obj->recordsUse	= 0;
			obj->recordsSz	= sz;
		}
	}
}
