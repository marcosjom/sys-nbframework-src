
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBRtcpClient.h"
//
#include "nb/core/NBArray.h"
#include "nb/core/NBThread.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBThreadCond.h"
#include "nb/net/NBRtcpParser.h"
#include "nb/net/NBSocket.h"

typedef struct STNBRtcpClientOpq_ {
	SI32				port;
	//pollster
	struct {
        BOOL            isListening;    //socket added to pollster
        STNBStopFlagRef stopFlag;
		STNBIOPollsterRef ref;
	} pollster;
	char				buff[4096];
	STNBSocketRef		socket;
	STNBThreadMutex		mutex;
	STNBArray			listeners;	//STNBRtcpClientListener
} STNBRtcpClientOpq;

//

void NBRtcpClient_init(STNBRtcpClient* obj){
	STNBRtcpClientOpq* opq = obj->opaque = NBMemory_allocType(STNBRtcpClientOpq);
	NBMemory_setZeroSt(*opq, STNBRtcpClientOpq);
    {
        opq->pollster.stopFlag = NBStopFlag_alloc(NULL);
    }
	{
		opq->socket = NBSocket_alloc(NULL);
		NBSocket_setType(opq->socket, ENNBSocketHndType_UDP);
		NBSocket_setCorkEnabled(opq->socket, FALSE);
		NBSocket_setDelayEnabled(opq->socket, FALSE);
		NBSocket_setNoSIGPIPE(opq->socket, TRUE);
		NBSocket_setNonBlocking(opq->socket, TRUE);
		NBSocket_setUnsafeMode(opq->socket, TRUE);
	}
	NBThreadMutex_init(&opq->mutex);
	NBArray_init(&opq->listeners, sizeof(STNBRtcpClientListener), NULL);
}

void NBRtcpClient_release(STNBRtcpClient* obj){
	STNBRtcpClientOpq* opq = (STNBRtcpClientOpq*)obj->opaque; 
	//Release
	{
		//pollster
        {
            if(NBStopFlag_isSet(opq->pollster.stopFlag)){
                NBStopFlag_release(&opq->pollster.stopFlag);
                NBStopFlag_null(&opq->pollster.stopFlag);
            }
            if(NBIOPollster_isSet(opq->pollster.ref)){
                if(opq->pollster.isListening){
                    NBIOPollster_removeSocket(opq->pollster.ref, opq->socket);
                    opq->pollster.isListening = FALSE;
                }
                NBIOPollster_release(&opq->pollster.ref);
                NBIOPollster_null(&opq->pollster.ref);
            }
        }
		//listeners
		{
			//SI32 i; for(i = 0; i < opq->listeners.use; i++){
			//	
			//}
			NBArray_empty(&opq->listeners);
			NBArray_release(&opq->listeners);
		}
		//Release
		NBASSERT(!opq->pollster.isListening)
        //socket
        {
            NBSocket_release(&opq->socket);
            NBSocket_null(&opq->socket);
        }
		NBThreadMutex_release(&opq->mutex);
	}
	NBMemory_free(opq);
	obj->opaque = NULL;
}

//cfg

BOOL NBRtcpClient_setParentStopFlag(STNBRtcpClient* obj, STNBStopFlagRef* stopFlag){
    BOOL r = FALSE;
    STNBRtcpClientOpq* opq = (STNBRtcpClientOpq*)obj->opaque;
    NBThreadMutex_lock(&opq->mutex);
    if(!opq->pollster.isListening && stopFlag != NULL){
        NBStopFlag_setParentFlag(opq->pollster.stopFlag, stopFlag);
        r = TRUE;
    }
    NBThreadMutex_unlock(&opq->mutex);
    return r;
}

BOOL NBRtcpClient_setPollster(STNBRtcpClient* obj, STNBIOPollsterRef pollster, STNBIOPollsterSyncRef pollSync){
	BOOL r = FALSE;
	STNBRtcpClientOpq* opq = (STNBRtcpClientOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	if(!opq->pollster.isListening){
		//set
		if(!NBIOPollster_isSame(opq->pollster.ref, pollster)){
			//remove from previous pollster and release
			if(NBIOPollster_isSet(opq->pollster.ref)){
				if(opq->pollster.isListening){
                    NBIOPollster_removeSocket(opq->pollster.ref, opq->socket);
					opq->pollster.isListening = FALSE;
				}
			}
			//set
			NBIOPollster_set(&opq->pollster.ref, &pollster);
		}
		r = TRUE;
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

//Listeners

BOOL NBRtcpClient_addListener(STNBRtcpClient* obj, STNBRtcpClientListener listener){
	BOOL r = FALSE;
	STNBRtcpClientOpq* opq = (STNBRtcpClientOpq*)obj->opaque;
	if(listener.obj != NULL){
		NBThreadMutex_lock(&opq->mutex);
		{
			BOOL fnd = FALSE;
			SI32 i; for(i = 0; i < opq->listeners.use; i++){
				const STNBRtcpClientListener* lstnr = NBArray_itmPtrAtIndex(&opq->listeners, STNBRtcpClientListener, i);
				if(lstnr->obj == listener.obj){
					fnd = TRUE;
					break;
				}
			}
			if(!fnd){
				NBArray_addValue(&opq->listeners, listener);
				r = TRUE;
			}
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	//
	return r;
}

BOOL NBRtcpClient_removeListener(STNBRtcpClient* obj, void* lstnrObj){
	BOOL r = FALSE;
	STNBRtcpClientOpq* opq = (STNBRtcpClientOpq*)obj->opaque;
	if(lstnrObj != NULL){
		NBThreadMutex_lock(&opq->mutex);
		{
			SI32 i; for(i = (SI32)opq->listeners.use - 1; i >= 0; i--){
				const STNBRtcpClientListener* lstnr = NBArray_itmPtrAtIndex(&opq->listeners, STNBRtcpClientListener, i);
				if(lstnr->obj == lstnrObj){
					NBArray_removeItemAtIndex(&opq->listeners, i);
					r = TRUE;
				}
			}
		}
		NBThreadMutex_unlock(&opq->mutex);
	}
	//
	return r;
}

//Actions

BOOL NBRtcpClient_bind(STNBRtcpClient* obj, const SI32 port){
	BOOL r = FALSE;
	STNBRtcpClientOpq* opq = (STNBRtcpClientOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	if(!opq->pollster.isListening){
		NBSocket_setReuseAddr(opq->socket, TRUE); //Recommended before bind
		NBSocket_setReusePort(opq->socket, TRUE); //Recommended before bind
		if(NBSocket_bind(opq->socket, port)){
			opq->port = port;
			r = TRUE;
		}
	}
	NBThreadMutex_unlock(&opq->mutex);
	//
	return r;
}

//pollster callbacks
void NBRtcpClient_pollConsumeMask_(STNBIOLnk ioLnk, const UI8 pollMask, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData);
void NBRtcpClient_pollConsumeNoOp_(STNBIOLnk ioLnk, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData);
void NBRtcpClient_pollRemoved_(STNBIOLnk ioLnk, void* usrData);

BOOL NBRtcpClient_startListening(STNBRtcpClient* obj){
	BOOL r = FALSE;
	STNBRtcpClientOpq* opq = (STNBRtcpClientOpq*)obj->opaque;
	NBThreadMutex_lock(&opq->mutex);
	if(!opq->pollster.isListening && NBIOPollster_isSet(opq->pollster.ref)){
		//add socket to pollster
		{
			STNBIOPollsterLstrnItf itf;
			NBMemory_setZeroSt(itf, STNBIOPollsterLstrnItf);
			itf.pollConsumeMask = NBRtcpClient_pollConsumeMask_;
			itf.pollConsumeNoOp = NBRtcpClient_pollConsumeNoOp_;
			itf.pollRemoved		= NBRtcpClient_pollRemoved_;
			if(NBIOPollster_addSocketWithItf(opq->pollster.ref, opq->socket, ENNBIOPollsterOpBit_Read, &itf, opq)){
				opq->pollster.isListening = TRUE;
			}
		}
		r = TRUE;
	}
	NBThreadMutex_unlock(&opq->mutex);
	return r;
}

BOOL NBRtcpClient_isBusy(STNBRtcpClient* obj){
	STNBRtcpClientOpq* opq = (STNBRtcpClientOpq*)obj->opaque;
	return opq->pollster.isListening;
}

void NBRtcpClient_stopFlag(STNBRtcpClient* obj){
	STNBRtcpClientOpq* opq = (STNBRtcpClientOpq*)obj->opaque;
    NBStopFlag_activate(opq->pollster.stopFlag);
}

//

void NBRtcpClient_pollConsumeMask_(STNBIOLnk ioLnk, const UI8 pollMask, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData){
	STNBRtcpClientOpq* opq = (STNBRtcpClientOpq*)usrData;
	IF_NBASSERT(STNBTimestampMicro timeLastAction = NBTimestampMicro_getMonotonicFast(), timeCur; SI64 usDiff;)
	//read
	if(pollMask & ENNBIOPollsterOpBit_Read){
		STNBSocketAddr from;
		SI32 rcvd;
		do {
			rcvd = NBSocket_recvFrom(opq->socket, opq->buff, sizeof(opq->buff), &from);
			if(rcvd < 0){
				//Error
				PRINTF_ERROR("RTCP, port(%d) connection lost.\n", opq->port);
				break;
			} else if(rcvd == 0){
				//Nothing to consume
				break;
			} else {
				STNBRtcpParserResult rr;
				NBRtcpParserResult_init(&rr);
				if(!NBRtcpParser_translatePacketBytes(opq->buff, rcvd, &rr)){
					PRINTF_ERROR("RTCP, port(%d) %d bytes packet could not be parsed.\n", opq->port, rcvd);
				} else {
					PRINTF_INFO("RTCP, port(%d) %d bytes packet parsed.\n", opq->port, rcvd);
				}
				//Print
				{
					STNBString str;
					NBString_init(&str);
					NBRtcpParserResult_concat(&rr, &str);
					PRINTF_INFO("RTCP, %s\n", str.str);
					NBString_release(&str);
				}
				NBRtcpParserResult_release(&rr);
			}
		} while(TRUE);
	}
	//error
	if(pollMask & ENNBIOPollsterOpBits_ErrOrGone){
		//
	}
	dstUpd->opsMasks = ENNBIOPollsterOpBit_Read;
	//consume stopFlag
	if(NBStopFlag_isAnyActivated(opq->pollster.stopFlag) && NBIOPollsterSync_isSet(dstSync)){
		NBASSERT(opq->pollster.isListening)
        NBIOPollsterSync_removeIOLnk(dstSync, &ioLnk);
	}
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	{
		timeCur	= NBTimestampMicro_getMonotonicFast();
		usDiff	= NBTimestampMicro_getDiffInUs(&timeLastAction, &timeCur);
		/*if(usDiff >= 1000ULL){
			PRINTF_INFO("NBRtcpClient, pollConsumeMask(%s%s%s) took %llu.%llu%llums.\n", (pollMask & ENNBIOPollsterOpBit_Read ? "+read" : ""), (pollMask & ENNBIOPollsterOpBit_Write ? "+write" : ""), (pollMask & ENNBIOPollsterOpBits_ErrOrGone ? "+errOrGone" : ""), (usDiff / 1000ULL), (usDiff % 1000ULL) % 100ULL, (usDiff % 100ULL) % 10ULL);
		}*/
		timeLastAction = timeCur;
	}
#	endif
}

void NBRtcpClient_pollConsumeNoOp_(STNBIOLnk ioLnk, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData){
	NBRtcpClient_pollConsumeMask_(ioLnk, 0, dstUpd, dstSync, usrData);
}

void NBRtcpClient_pollRemoved_(STNBIOLnk ioLnk, void* usrData){
	STNBRtcpClientOpq* opq = (STNBRtcpClientOpq*)usrData;
	opq->pollster.isListening	= FALSE;
	//PRINTF_INFO("NBRtcpClient, port(%d) safetly-removed from pollster.\n", opq->port);
}

