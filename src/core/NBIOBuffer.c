
#include "nb/NBFrameworkPch.h"
#include "nb/core/NBIOBuffer.h"
//
#include "nb/core/NBStruct.h"
#include "nb/core/NBMngrStructMaps.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBThread.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBThreadCond.h"
#include "nb/net/NBSocket.h"
//

#define NB_IO_BUFFER_BUFF_READ_SZ               (1024 * 16)    //http parser read-buffer size, and socket read-buffer-size
#define NB_IO_BUFFER_BUFF_BODY_STRG_SEGS_SZ     (1024 * 16)    //http body storage segments size

//pollster callbacks

void NBIOBuffer_pollConsumeMask_(STNBIOLnk ioLnk, const UI8 pollMask, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData);
void NBIOBuffer_pollConsumeNoOp_(STNBIOLnk ioLnk, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData);
void NBIOBuffer_pollRemoved_(STNBIOLnk ioLnk, void* usrData);

//io (for child consumer/producer)

BOOL NBIOBuffer_ioIsObjRef_(STNBObjRef objRef, void* usrData);
SI32 NBIOBuffer_ioRead_(void* dst, const SI32 dstSz, void* usrData);
SI32 NBIOBuffer_ioWrite_(const void* src, const SI32 srcSz, void* usrData);
void NBIOBuffer_ioFlush_(void* usrData);
void NBIOBuffer_ioShutdown_(const UI8 mask, void* usrData); //NB_IO_BIT_READ | NB_IO_BIT_WRITE
void NBIOBuffer_ioClose_(void* usrData);

//

//NBIOBufferBuff

typedef struct STNBIOBufferBuff_ {
    BYTE*   data;
    UI32    size;    //right bo
    UI32    csmd;    //left border
    UI32    filled;  //right border
} STNBIOBufferBuff;

//NBIOBufferBuffs

//Note: these buffers are designed for one-fill-action per buffer.
//Their purpose is to provide 'big-enough' buffers for every low-level-call, like 'rcv' or 'read'.
//If an empty buffer is available the next low-level-call will be made to that empty buffer,
//else, the next low-level-call will be made to the remain space of the last buffer available.

typedef struct STNBIOBufferBuffs_ {
    STNBIOBufferBuff* fill;  //current filling buffer
    STNBIOBufferBuff* read;  //current read buffer
    //allocs
    struct {
        STNBIOBufferBuff buff0;      //final-buff if no-ssl, encrypted-buff if ssl
        STNBIOBufferBuff buff1;      //final-buff if no-ssl, encrypted-buff if ssl
    } allocs;
    //totals
    struct {
        UI32    csmd;      //total filled-bytes consumed from buffer
        UI64    filled;    //total filled-bytes populated on buffer
    } totals;
} STNBIOBufferBuffs;

void NBIOBufferBuffs_init(STNBIOBufferBuffs* obj);
void NBIOBufferBuffs_release(STNBIOBufferBuffs* obj);
//
void NBIOBufferBuffs_create(STNBIOBufferBuffs* obj, const SI32 buffSize);
void NBIOBufferBuffs_moveReadCursor(STNBIOBufferBuffs* obj, const SI32 moveSz);
void NBIOBufferBuffs_moveFillCursor(STNBIOBufferBuffs* obj, const SI32 moveSz);

//NBIOBuffer

typedef struct STNBIOBufferOpq_ {
	STNBObject				prnt;
    STNBStopFlagRef			stopFlag;
	//pollster
	struct {
		STNBIOPollsterSyncRef sync;
		BOOL				isListening;
	} pollster;
	//io
	struct {
        STNBIOLnk           lnk;
        STNBIOBufferBuffs   buffs;          //final-buff if no-ssl, encrypted-buff if ssl
        BOOL                isRecvBlocked;  //explicit blocking, last 'recv' action returned would-block
        BOOL                isSendBlocked;  //explicit blocking, last 'send' action returned would-block
        UI64                bytesSent;      //global socket-sent-counter
	} io;
} STNBIOBufferOpq;

//

NB_OBJREF_BODY(NBIOBuffer, STNBIOBufferOpq, NBObject)

//

void NBIOBuffer_initZeroed(STNBObject* obj){
	STNBIOBufferOpq* opq = (STNBIOBufferOpq*)obj;
    opq->stopFlag = NBStopFlag_alloc(NULL);
    //pollster
    {
        //
    }
	//io
	{
        //
	}
}

void NBIOBuffer_uninitLocked(STNBObject* obj){
	STNBIOBufferOpq* opq = (STNBIOBufferOpq*)obj;
    NBStopFlag_activate(opq->stopFlag);
	//io
	{
        if(NBIOLnk_isSet(&opq->io.lnk)){
            NBIOLnk_release(&opq->io.lnk);
            NBIOLnk_null(&opq->io.lnk);
        }
        NBIOBufferBuffs_release(&opq->io.buffs);
	}
	//pollster
	{
		if(NBIOPollsterSync_isSet(opq->pollster.sync)){
			NBIOPollsterSync_release(&opq->pollster.sync);
			NBIOPollsterSync_null(&opq->pollster.sync);
		}
	}
    if(NBStopFlag_isSet(opq->stopFlag)){
        NBStopFlag_release(&opq->stopFlag);
        NBStopFlag_null(&opq->stopFlag);
    }
}

//

BOOL NBIOBuffer_isObjRef(STNBIOBufferRef ref, STNBObjRef objRef){
	STNBIOBufferOpq* opq = (STNBIOBufferOpq*)ref.opaque; NBASSERT(NBIOBuffer_isClass(ref))
    return NBIOLnk_isObjRef(&opq->io.lnk, objRef);
}

BOOL NBIOBuffer_startListeningOwningSocket(STNBIOBufferRef ref, STNBIOPollsterSyncRef pollSync, STNBStopFlagRef* parentStopFlag, STNBSocketRef socket){
	BOOL r = FALSE;
	STNBIOBufferOpq* opq = (STNBIOBufferOpq*)ref.opaque; NBASSERT(NBIOBuffer_isClass(ref))
	if(!opq->pollster.isListening && !NBIOPollsterSync_isSet(opq->pollster.sync) && NBIOPollsterSync_isSet(pollSync) && !NBIOLnk_isSet(&opq->io.lnk) && NBSocket_isSet(socket)){
        r = TRUE;
		//set-vars
		{
            //parent stop flag
            if(r){
                NBStopFlag_setParentFlag(opq->stopFlag, parentStopFlag);
            }
			//socket
            if(r){
                //cfg
				NBSocket_setNoSIGPIPE(socket, TRUE);
				NBSocket_setCorkEnabled(socket, FALSE);
				NBSocket_setDelayEnabled(socket, FALSE);
				NBSocket_setNonBlocking(socket, TRUE);
				NBSocket_setUnsafeMode(socket, TRUE);
                //get lnk
                if(!NBSocket_getIOLnk(socket, &opq->io.lnk)){
                    r = FALSE;
                }
			}
			//pollster
            if(r){
				opq->pollster.isListening = TRUE;
				NBIOPollsterSync_set(&opq->pollster.sync, &pollSync);
			}
		}
		//start listenting
        if(r){
            STNBIOPollsterLstrnItf itf;
            NBMemory_setZeroSt(itf, STNBIOPollsterLstrnItf);
            itf.pollConsumeMask = NBIOBuffer_pollConsumeMask_;
            itf.pollConsumeNoOp = NBIOBuffer_pollConsumeNoOp_;
            itf.pollRemoved     = NBIOBuffer_pollRemoved_;
            if(!NBIOPollsterSync_addSocketWithItf(pollSync, socket, ENNBIOPollsterOpBit_Read, &itf, opq)){
                PRINTF_ERROR("NBIOBuffer, initial NBIOPollsterSync_addSocketWithItf failed.\n");
                NBStopFlag_activate(opq->stopFlag);
                //unset-vars
                opq->pollster.isListening = FALSE;
                if(NBIOPollsterSync_isSet(opq->pollster.sync)){
                    NBIOPollsterSync_release(&opq->pollster.sync);
                    NBIOPollsterSync_null(&opq->pollster.sync);
                }
                if(NBIOLnk_isSet(&opq->io.lnk)){
                    NBIOLnk_release(&opq->io.lnk);
                    NBIOLnk_null(&opq->io.lnk);
                }
                r = FALSE;
            }
        }
	}
	return r;
}


void NBIOBuffer_stopFlag(STNBIOBufferRef ref){
	STNBIOBufferOpq* opq = (STNBIOBufferOpq*)ref.opaque; NBASSERT(NBIOBuffer_isClass(ref))
    NBStopFlag_activate(opq->stopFlag);
}

BOOL NBIOBuffer_isBusy(STNBIOBufferRef ref){
	STNBIOBufferOpq* opq = (STNBIOBufferOpq*)ref.opaque; NBASSERT(NBIOBuffer_isClass(ref))
	return opq->pollster.isListening;
}
    
//pollster callbacks
	
void NBIOBuffer_pollConsumeMask_(STNBIOLnk ioLnk, const UI8 pollMask, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData){
	STNBIOBufferOpq* opq = (STNBIOBufferOpq*)usrData;
	IF_NBASSERT(STNBTimestampMicro timeLastAction = NBTimestampMicro_getMonotonicFast(), timeCur; SI64 usDiff;)
    //
    //unset blocking-flags
    STNBIOBufferBuffs* buffs = &opq->io.buffs;
    opq->io.isRecvBlocked = ((pollMask & ENNBIOPollsterOpBit_Read) == 0); //read only when explicit available.
    if(pollMask & ENNBIOPollsterOpBit_Write){ //write is implicit available untill explicit unlocked.
        opq->io.isSendBlocked = FALSE;
    }
	//read
    {
        //ToDo: voluntarily stop reading after a limit (avoid chance of hijacking the thread on fast network action)
        //read and consume (while not explicit-blocked and buffer-has-space)
        while(!NBStopFlag_isMineActivated(opq->stopFlag) && !opq->io.isRecvBlocked && buffs->fill->filled < buffs->fill->size){
            //recv
            const SI32 rcvd = NBIOLnk_read(&ioLnk, &buffs->fill->data[buffs->fill->filled], buffs->fill->size - buffs->fill->filled);
            //process
            if(rcvd < 0){
                //stop (socket error)
                PRINTF_ERROR("NBIOBuffer, NBIOLnk_read failed with (%d).\n", rcvd);
                NBStopFlag_activate(opq->stopFlag);
                opq->io.isRecvBlocked = TRUE;
                break;
            } else if(rcvd > 0){
                //PRINTF_INFO("NBIOBuffer, NBIOLnk_read received %d bytes.\n", rcvd);
                {
                    STNBString str;
                    NBString_initWithStrBytes(&str, (const char*)&buffs->fill->data[buffs->fill->filled], rcvd);
                    PRINTF_INFO("NBIOBuffer, NBIOLnk_read received %d bytes:--->\n%s<---.\n", rcvd, str.str);
                    NBString_release(&str);
                }
                NBIOBufferBuffs_moveFillCursor(buffs, rcvd);
            } else {
                //wait for flag to be cleaned
                opq->io.isRecvBlocked = TRUE;
            }
        }
    }
	//write
	{
        //ToDo: voluntarily stop sending after a limit (avoid chance of hijacking the thread on fast network action)
        /*UI64 sentBefore = opq->io.bytesSent + 1; //+1 to force first action.
        while(!NBStopFlag_isMineActivated(opq->stopFlag) && !opq->io.isSendBlocked && sentBefore != opq->io.bytesSent){
            sentBefore = opq->io.bytesSent;
            NBIOBuffer_consumeWriteOportunityOpq_(opq);
        }*/
	}
	//error
	if(pollMask & ENNBIOPollsterOpBits_ErrOrGone){
		//stop
		PRINTF_ERROR("NBIOBuffer, conn-error-or-gone.\n");
        NBStopFlag_activate(opq->stopFlag);
	}
	//return
    dstUpd->opsMasks = ENNBIOPollsterOpBit_Read; // | ((opq->io.ssl.cmd.cur != ENNBHttpSslCmd_None && opq->io.ssl.cmd.result == ENNBSslResult_ErrWantWrite) || (opq->io.ssl.cmd.cur == ENNBHttpSslCmd_None && opq->req.ctx.req.body.isCompleted && NBHttpServiceResp_isSet(opq->resp.obj) && NBHttpServiceResp_isWritePend(opq->resp.obj)) ? ENNBIOPollsterOpBit_Write : 0);
    if(dstUpd->opsMasks & ENNBIOPollsterOpBit_Write){
        //PRINTF_INFO("NBIOBuffer, waiting for write.\n");
    } else {
        //PRINTF_INFO("NBIOBuffer, not-waiting for write.\n");
    }
	//consume stopFlag
	if(NBStopFlag_isAnyActivated(opq->stopFlag) && NBIOPollsterSync_isSet(dstSync)){
		NBASSERT(opq->pollster.isListening)
        PRINTF_INFO("NBIOBuffer, stopping after %d bytes rcvd and %d bytes sent.\n", opq->io.buffs.totals.filled, opq->io.bytesSent);
        NBIOPollsterSync_removeIOLnk(dstSync, &ioLnk);
	}
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	{
		timeCur	= NBTimestampMicro_getMonotonicFast();
		usDiff	= NBTimestampMicro_getDiffInUs(&timeLastAction, &timeCur);
		/*if(usDiff >= 1000ULL){
			PRINTF_INFO("NBIOBuffer, pollConsumeMask(%s%s%s) took %llu.%llu%llums.\n", (pollMask & ENNBIOPollsterOpBit_Read ? "+read" : ""), (pollMask & ENNBIOPollsterOpBit_Write ? "+write" : ""), (pollMask & ENNBIOPollsterOpBits_ErrOrGone ? "+errOrGone" : ""), (usDiff / 1000ULL), (usDiff % 1000ULL) % 100ULL, (usDiff % 100ULL) % 10ULL);
		}*/
		timeLastAction = timeCur;
	}
#	endif
}

void NBIOBuffer_pollConsumeNoOp_(STNBIOLnk ioLnk, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData){
	NBIOBuffer_pollConsumeMask_(ioLnk, 0, dstUpd, dstSync, usrData);
}

void NBIOBuffer_pollRemoved_(STNBIOLnk ioLnk, void* usrData){
	STNBIOBufferOpq* opq = (STNBIOBufferOpq*)usrData;
	opq->pollster.isListening = FALSE;
	//PRINTF_INFO("NBIOBuffer, client safetly-removed from pollster.\n");
}

//NBIOLink

BOOL NBIOBuffer_ioIsObjRef_(STNBObjRef objRef, void* usrData){
    STNBIOBufferOpq* opq = (STNBIOBufferOpq*)usrData; NBASSERT(NBIOBuffer_isClass(NBObjRef_fromOpqPtr(opq)))
    return NBIOLnk_isObjRef(&opq->io.lnk, objRef);
}

SI32 NBIOBuffer_ioRead_(void* dst, const SI32 dstSz, void* usrData){ //read data to destination buffer, returns the ammount of bytes read, negative in case of error
    SI32 r = -1;
    STNBIOBufferOpq* opq = (STNBIOBufferOpq*)usrData; NBASSERT(NBIOBuffer_isClass(NBObjRef_fromOpqPtr(opq)))
    if(!NBStopFlag_isMineActivated(opq->stopFlag) && dst != NULL && dstSz >= 0){
        STNBIOBufferBuffs* buffs = &opq->io.buffs; //final-buff if no-ssl, encrypted-buff if ssl
        const SI32 avail = (buffs->read->filled - buffs->read->csmd);
        r = (avail <= 0 ? 0 : dstSz <= avail ? dstSz : avail);
        if(r > 0){
            NBMemory_copy(dst, &buffs->read->data[buffs->read->csmd], r);
            PRINTF_INFO("NBIOBuffer, encrypted-ioRead did (%d of %d bytes from buffer).\n", r, dstSz);
        } else {
            PRINTF_INFO("NBIOBuffer, encrypted-ioRead did (%d of %d bytes from buffer).\n", r, dstSz);
        }
        //consume buffer
        NBIOBufferBuffs_moveReadCursor(buffs, r);
    }
    return r;
}

SI32 NBIOBuffer_ioWrite_(const void* src, const SI32 srcSz, void* usrData){ //write data from source buffer, returns the ammount of bytes written, negative in case of error
    SI32 r = -1;
    STNBIOBufferOpq* opq = (STNBIOBufferOpq*)usrData; NBASSERT(NBIOBuffer_isClass(NBObjRef_fromOpqPtr(opq)))
    if(srcSz == 0 || opq->io.isSendBlocked){
        r = 0;
    } else if(srcSz > 0 && src != NULL){
        //Plain send
        r = NBIOLnk_write(&opq->io.lnk, src, srcSz);
        if(r < 0){
            PRINTF_ERROR("NBIOBuffer, encrypted-ioWrite did (%d of %d bytes).\n", r, srcSz);
            NBStopFlag_activate(opq->stopFlag);
        } else {
            opq->io.bytesSent += r;
            if(r < srcSz){
                opq->io.isSendBlocked = TRUE;
            }
            PRINTF_INFO("NBIOBuffer, encrypted-ioWrite did (%d of %d bytes).\n", r, srcSz);
        }
    }
    return r;
}

void NBIOBuffer_ioFlush_(void* usrData){ //flush write-data
    STNBIOBufferOpq* opq = (STNBIOBufferOpq*)usrData; NBASSERT(NBIOBuffer_isClass(NBObjRef_fromOpqPtr(opq)))
    {
        PRINTF_INFO("NBIOBuffer, ioFlush.\n");
        NBIOLnk_flush(&opq->io.lnk);
    }
}

void NBIOBuffer_ioShutdown_(const UI8 mask, void* usrData){ //NB_IO_BIT_READ | NB_IO_BIT_WRITE
    STNBIOBufferOpq* opq = (STNBIOBufferOpq*)usrData; NBASSERT(NBIOBuffer_isClass(NBObjRef_fromOpqPtr(opq)))
    {
        PRINTF_INFO("NBIOBuffer, ioShutdown.\n");
        NBIOLnk_shutdown(&opq->io.lnk, mask);
        NBStopFlag_activate(opq->stopFlag);
    }
}

void NBIOBuffer_ioClose_(void* usrData){ //close ungracefully
    STNBIOBufferOpq* opq = (STNBIOBufferOpq*)usrData; NBASSERT(NBIOBuffer_isClass(NBObjRef_fromOpqPtr(opq)))
    {
        PRINTF_INFO("NBIOBuffer, ioClose.\n");
        NBIOLnk_close(&opq->io.lnk);
        NBStopFlag_activate(opq->stopFlag);
    }
}


//NBIOBufferBuffs


void NBIOBufferBuffs_init(STNBIOBufferBuffs* obj){
    NBMemory_setZeroSt(*obj, STNBIOBufferBuffs);
}

void NBIOBufferBuffs_release(STNBIOBufferBuffs* obj){
    obj->fill = obj->read = NULL;
    //allocs
    {
        //buff0
        {
            STNBIOBufferBuff* buff = &obj->allocs.buff0;   //final-buff if no-ssl, encrypted-buff if ssl
            if(buff->data != NULL){
                NBMemory_free(buff->data);
                buff->data = NULL;
            }
            buff->filled = buff->csmd = buff->size = 0;
        }
        //buff1
        {
            STNBIOBufferBuff* buff = &obj->allocs.buff1;   //final-buff if no-ssl, encrypted-buff if ssl
            if(buff->data != NULL){
                NBMemory_free(buff->data);
                buff->data = NULL;
            }
            buff->filled = buff->csmd = buff->size = 0;
        }
    }
}

void NBIOBufferBuffs_create(STNBIOBufferBuffs* obj, const SI32 buffSize){
    //buff0
    {
        STNBIOBufferBuff* buff = &obj->allocs.buff0; //final-buff if no-ssl, encrypted-buff if ssl
        NBASSERT(buff->data == NULL && buff->size == 0)
        if(buff->data != NULL){
            NBMemory_free(buff->data);
            buff->data = NULL;
            buff->size = 0;
        }
        buff->size = buffSize;
#       ifdef NB_CONFIG_INCLUDE_ASSERTS
        buff->data = (BYTE*)NBMemory_alloc(buff->size + 1); //+1 for a '\0'
#       else
        buff->data = (BYTE*)NBMemory_alloc(buff->size);
#       endif
    }
    //buff1
    {
        STNBIOBufferBuff* buff = &obj->allocs.buff1; //final-buff if no-ssl, encrypted-buff if ssl
        NBASSERT(buff->data == NULL && buff->size == 0)
        if(buff->data != NULL){
            NBMemory_free(buff->data);
            buff->data = NULL;
            buff->size = 0;
        }
        buff->size = buffSize;
#       ifdef NB_CONFIG_INCLUDE_ASSERTS
        buff->data = (BYTE*)NBMemory_alloc(buff->size + 1); //+1 for a '\0'
#       else
        buff->data = (BYTE*)NBMemory_alloc(buff->size);
#       endif
    }
    //activate firtst buffer
    obj->fill = obj->read = &obj->allocs.buff0;
}

//Note: these buffers are designed for one-fill-action per buffer.
//Their purpose is to provide 'big-enough' buffers for every low-level-call, like 'rcv' or 'read'.
//If an empty buffer is available the next low-level-call will be made to that empty buffer,
//else, the next low-level-call will be made to the remain space of the last buffer available.

void NBIOBufferBuffs_moveFillCursor(STNBIOBufferBuffs* obj, const SI32 moveSz){
    if(moveSz > 0){
        obj->fill->filled   += moveSz;
        obj->totals.filled  += moveSz;
        //swap fill-buffer
        NBASSERT(obj->fill->filled <= obj->fill->size)
        if(
           obj->fill->filled == obj->fill->size //buffer is full
           || obj->fill == obj->read //next buffer is not in use
           )
        {
            STNBIOBufferBuff* nxt = (obj->fill == &obj->allocs.buff0 ? &obj->allocs.buff1 : &obj->allocs.buff0);
            if(nxt != obj->read){
                nxt->csmd = nxt->filled = 0;
                //also move 'read' buffer if consumed
                if(obj->read->csmd == obj->read->filled){
                    STNBIOBufferBuff* nxt2 = (obj->read == &obj->allocs.buff0 ? &obj->allocs.buff1 : &obj->allocs.buff0);
                    obj->read = nxt2; NBASSERT(obj->read->csmd == 0)
                }
                obj->fill = nxt; NBASSERT(obj->fill->csmd == 0 && obj->fill->filled == 0)
            }
        }
    }
}

//Note: these buffers are designed for one-fill-action per buffer.
//Their purpose is to provide 'big-enough' buffers for every low-level-call, like 'rcv' or 'read'.
//If an empty buffer is available the next low-level-call will be made to that empty buffer,
//else, the next low-level-call will be made to the remain space of the last buffer available.

void NBIOBufferBuffs_moveReadCursor(STNBIOBufferBuffs* obj, const SI32 moveSz){
    if(moveSz > 0){
        obj->read->csmd     += moveSz;
        obj->totals.csmd    += moveSz;
        //reset buffer
        NBASSERT(obj->read->csmd <= obj->read->filled)
        if(obj->read->csmd == obj->read->filled){
            if(obj->fill == obj->read){
                //reset buffer state
                obj->read->csmd = obj->fill->filled = 0;
            } else {
                //move to next buffer
                STNBIOBufferBuff* nxt = (obj->read == &obj->allocs.buff0 ? &obj->allocs.buff1 : &obj->allocs.buff0);
                obj->read = nxt; NBASSERT(obj->read->csmd == 0)
            }
        }
    }
}
