
#include "nb/NBFrameworkPch.h"
//Complementary PCH
#ifdef _WIN32
#	include <winsock2.h>	//Include before windows.h
//#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	include <ws2tcpip.h>	//for sockaddr_in6
#	pragma comment(lib, "Ws2_32.lib")
//	Validation
//#	if !defined(_WINSOCK2API_)
//#	if !defined(_WINSOCKAPI_)
//#		error No WINSOCK API was loaded by windows.h
//#	else
//#		warning WINSOCK_1 API was loaded by windows.h (not WINSOCK_2)
//#	endif
//#	endif
#else
#	ifdef __linux__
#		define _GNU_SOURCE	//for linux-sepecific "struct mmsghdr" inside <sys/socket.h>
#		if !defined(__ANDROID__) || (defined(__ANDROID__) && __ANDROID_API__ >= 21)
#			define NB_SOCKET_RECVMMSG_ENABLED
#		endif
#	endif
#	include <unistd.h>		//for close() de sockets
#	include <sys/socket.h>	//for socklen_t
#	include <netinet/in.h>	//for sockaddr_in
#	include <netinet/tcp.h>	//for TCP_NODELAY (also in /usr/include/linux/tcp.h or /usr/include/netinet/tcp.h)
#	include <netdb.h>		//for hostent
#	include <errno.h>		//for errno
#	include <string.h>		//for strerror()
#	include <fcntl.h>		//for O_NONBLOCK
#	include <poll.h>		//for poll()
#endif
//
#include "nb/net/NBSocket.h"
#include "nb/core/NBMngrStructMaps.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBLog.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBHndl.h"

#ifdef SOCKET
#	define NB_SOCKET	SOCKET
#else
#	define NB_SOCKET	int
#endif

#ifndef INVALID_SOCKET
#	define INVALID_SOCKET	(NB_SOCKET)(~0)	//SOCKET is unsigned in windows, signoed in others
#endif

#ifdef _WIN32
#	define NB_SOCKLEN	int
#else
#	define NB_SOCKLEN	socklen_t
#endif

//

#define NB_SOCKET_VERIF_VALUE	9812

//--------------
//- NBSocketAddr
//--------------

NBCompareFunc NBCompare_NBSocketAddr_addrOnlyByFamily[AF_MAX];
NBCompareFunc NBCompare_NBSocketAddr_addrAndPortByFamily[AF_MAX];

BOOL NBCompare_NBSocketAddr_addrOnlyINET(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);
BOOL NBCompare_NBSocketAddr_addrOnlyINET6(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);

const char* NBSocket_getErrorName(const int errCode);

//ENNBSocketAsyncAction

typedef enum ENNBSocketAsyncAction_ {
	ENNBSocketAsyncAction_Accept = 0,
	ENNBSocketAsyncAction_Connect,
	ENNBSocketAsyncAction_Receive,
	ENNBSocketAsyncAction_Send,
	//Count
	ENNBSocketAsyncAction_Count
} ENNBSocketAsyncAction;

//ENNBSocketAsyncActionBit

typedef enum ENNBSocketAsyncActionBit_ {
	ENNBSocketAsyncActionBit_Accept		= (0x1 << ENNBSocketAsyncAction_Accept),
	ENNBSocketAsyncActionBit_Connect	= (0x1 << ENNBSocketAsyncAction_Connect),
	ENNBSocketAsyncActionBit_Receive	= (0x1 << ENNBSocketAsyncAction_Receive),
	ENNBSocketAsyncActionBit_Send		= (0x1 << ENNBSocketAsyncAction_Send),
	//All
	ENNBSocketAsyncActionBits_All		= (ENNBSocketAsyncActionBit_Accept | ENNBSocketAsyncActionBit_Connect | ENNBSocketAsyncActionBit_Receive | ENNBSocketAsyncActionBit_Send)
} ENNBSocketAsyncActionBit;

#define NB_SOCKET_READ_ACTIONS_MASK		(ENNBSocketAsyncActionBit_Accept | ENNBSocketAsyncActionBit_Receive)
#define NB_SOCKET_WRITE_ACTIONS_MASK	(ENNBSocketAsyncActionBit_Connect | ENNBSocketAsyncActionBit_Send)

//

#ifdef NB_CONFIG_INCLUDE_ASSERTS
#	define NBSOCKET_DBG_WAIT(NAME)	//{ UI32 ms = (1 + (rand() % 50)); NBThread_mSleep(ms); if(ms > 35){ PRINTF_INFO("NBSOCKET_DBG_WAIT(%d ms at '%s').\n", ms, NAME); } }
#	else
#	define NBSOCKET_DBG_WAIT(NAME)
#endif

//

BOOL __globalSocketMutexInited = FALSE;
STNBThreadMutex __globalSocketMutex;

void NBSocket_initEngine(void){
    NBASSERT(!__globalSocketMutexInited)
	NBThreadMutex_init(&__globalSocketMutex);
	__globalSocketMutexInited = TRUE;
	//NBCompare_NBSocketAddr
	{
		NBMemory_setZero(NBCompare_NBSocketAddr_addrOnlyByFamily);
		NBMemory_setZero(NBCompare_NBSocketAddr_addrAndPortByFamily);
		{
			NBCompare_NBSocketAddr_addrOnlyByFamily[AF_INET]	= NBCompare_NBSocketAddr_addrOnlyINET;
			NBCompare_NBSocketAddr_addrOnlyByFamily[AF_INET6]	= NBCompare_NBSocketAddr_addrOnlyINET6;
		}
	}
}

void NBSocket_releaseEngine(void){
	NBASSERT(__globalSocketMutexInited)
	NBThreadMutex_release(&__globalSocketMutex);
	__globalSocketMutexInited = FALSE;
}

//


UI32 NBSocket_initWSA(void){
#	ifdef _WIN32
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	NBASSERT(__globalSocketMutexInited)
	if (iResult != NO_ERROR) return 0;
#	else
	NBASSERT(__globalSocketMutexInited)
#	endif
	//PRINTF_INFO("sizeof(struct sockaddr_in6) = %d.\n", (UI32)sizeof(struct sockaddr_in6));
	return 1;
}

UI32 NBSocket_finishWSA(void){
#	ifdef _WIN32
	NBASSERT(__globalSocketMutexInited)
	WSACleanup();
#	else
	NBASSERT(__globalSocketMutexInited)
#	endif
	return 1;
}

//opaque

typedef struct STNBSocketOpq_ {
	STNBObject			prnt;
	SI32				isClass;			//always NB_SOCKET_VERIF_VALUE
	ENNBSocketHndType	type;               //TCP, UDP
	BOOL				isShutedForRcv;		//Cannot receive anymore
	BOOL				isShutedForSend;	//Cannot send anymore
    STNBHndlRef         hndl;               //handle/file-descriptor
	NB_SOCKET			hndlNative;			//optimization, copy of handle/file-descriptor (this source-file should keep 'hndl' and 'hndlNative' in sync)
	struct sockaddr		my_addr;
	struct sockaddr		their_addr;
	//err
	struct {
		SI32			num;
		char			str[64];
	} err;
	//async actions
	struct {
		UI8				activeMask;
	} async;
	//unsafe mode (optimization)
	struct {
		BOOL 			isEnabled;
		UI32			depth;
	} unsafe;
	//dbg
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	struct {
		BOOL			connectFailed;		//POSIX 2001: if connect() fails, the state of the socket is unspecified. Conforming applications should close the file descriptor and create a new socket before attempting to reconnect.
	} dbg;
#	endif
} STNBSocketOpq;

BOOL NBSocket_createHndlLockedOpq_(STNBSocketOpq* opq);
int NBSocket_getFDOpq_(STNBSocketOpq* opq);
SI32 NBSocket_recvOpq_(STNBSocketOpq* opq, void* buff, int buffSz);
SI32 NBSocket_sendOpq_(STNBSocketOpq* opq, const void* data, int dataSz);
void NBSocket_shutdownOpq_(STNBSocketOpq* opq, const UI8 mask); //NB_IO_BIT_READ | NB_IO_BIT_WRITE
void NBSocket_closeOpq_(STNBSocketOpq* opq);

//IOLnk
int  NBSocket_ioGetFD_(void* usrData);
BOOL NBSocket_ioIsObjRef_(STNBObjRef objRef, void* usrData); //to determine if the io is an specific socket
SI32 NBSocket_ioRead_(void* dst, const SI32 dstSz, void* usrData); //read data to destination buffer, returns the ammount of bytes read, negative in case of error
SI32 NBSocket_ioWrite_(const void* src, const SI32 srcSz, void* usrData); //write data from source buffer, returns the ammount of bytes written, negative in case of error
void NBSocket_ioFlush_(void* usrData);
void NBSocket_ioShutdown_(const UI8 mask, void* usrData); //NB_IO_BIT_READ | NB_IO_BIT_WRITE
void NBSocket_ioClose_(void* usrData);

//

NB_OBJREF_BODY(NBSocket, STNBSocketOpq, NBObject)

//-----------------
//NBSocket MACROS
//-----------------

//Auto-safe action (depends of 'unsafe.isEnabled')

#define NB_SOCKET_AUTOSAFE_ACTION_START(OPQ) \
	BOOL _locked_ = FALSE; \
	NBASSERT(__globalSocketMutexInited) \
	NBASSERT((OPQ) != NULL) \
	NBASSERT((OPQ)->isClass == NB_SOCKET_VERIF_VALUE); \
	NBASSERT((OPQ)->unsafe.depth == 0) /*user logic error, unsafe mode must be enabled and only serial calls are allowed*/ \
	if((OPQ)->unsafe.isEnabled) {\
		(OPQ)->unsafe.depth++; \
	} else { \
		NBObject_lock((OPQ)); \
		_locked_ = TRUE; \
	}

#define NB_SOCKET_AUTOSAFE_ACTION_END(OPQ) \
	if(_locked_){ \
		NBObject_unlock((OPQ)); \
	} else { \
		NBASSERT((OPQ)->unsafe.depth > 0) /*program logic error*/ \
		(OPQ)->unsafe.depth--; \
	}

//Unsafe action (depends of 'unsafe.isEnabled' but wont touch 'unsafe.depth' value)

#define NB_SOCKET_UNSAFE_ACTION_START(OPQ) \
	BOOL _locked_ = FALSE; \
	NBASSERT(__globalSocketMutexInited) \
	NBASSERT((OPQ) != NULL) \
	NBASSERT((OPQ)->isClass == NB_SOCKET_VERIF_VALUE); \
	if(!(OPQ)->unsafe.isEnabled) {\
		NBObject_lock((OPQ)); \
		_locked_ = TRUE; \
	}

#define NB_SOCKET_UNSAFE_ACTION_END(OPQ) \
	if(_locked_){ \
		NBObject_unlock((OPQ)); \
	}

//Safe action (forbids 'unsafe.isEnabled' and 'unsafe.depth')

#define NB_SOCKET_SAFE_ACTION_START(OPQ) \
	NBASSERT(__globalSocketMutexInited) \
	NBASSERT((OPQ) != NULL) \
	NBASSERT((OPQ)->isClass == NB_SOCKET_VERIF_VALUE); \
	NBASSERT((OPQ)->unsafe.depth == 0) /*user logic error, unsafe mode must be disabled and not action should be in progress*/ \
	NBASSERT(!(OPQ)->unsafe.isEnabled) /*user logic error, unsafe mode must be disabled and not action should be in progress*/ \
	NBObject_lock((OPQ));

#define NB_SOCKET_SAFE_ACTION_END(OPQ) \
	NBObject_unlock((OPQ));

//

void NBSocket_initZeroed(STNBObject* obj) {
	STNBSocketOpq* opq	= (STNBSocketOpq*)obj;
	opq->type			= ENNBSocketHndType_TCP;
	opq->isClass		= NB_SOCKET_VERIF_VALUE;
	//
	if(!NBSocket_initWSA()){
		NBASSERT(FALSE);
	}
}

void NBSocket_uninitLocked(STNBObject* obj){
	STNBSocketOpq* opq = (STNBSocketOpq*)obj;
    //handle/file-descriptor
    if(NBHndl_isSet(opq->hndl)){
        NBHndl_setOrphan(opq->hndl); //flag as orphan for every external consumer (like pollster).
        NBHndl_release(&opq->hndl);
        NBHndl_null(&opq->hndl);
    }
    opq->hndlNative = 0;
	NBSocket_finishWSA();
}

//

//when enabled, internal locks are disabled for efficiency but non-parallels calls must be ensured by user
void NBSocket_setUnsafeMode(STNBSocketRef ref, const BOOL unsafeEnabled){
	STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque; NBASSERT(NBSocket_isClass(ref));
	NB_SOCKET_AUTOSAFE_ACTION_START(opq)
	NBASSERT(!opq->dbg.connectFailed) //socket must be closed after a 'connect' fail.
	{
		opq->unsafe.isEnabled = unsafeEnabled;
	}
	NB_SOCKET_AUTOSAFE_ACTION_END(opq)
}

ENNBSocketHndType NBSocket_getType(STNBSocketRef ref){
	STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque;
	return opq->type;
}

BOOL NBSocket_setType(STNBSocketRef ref, const ENNBSocketHndType type){
	BOOL r = FALSE;
	STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque; NBASSERT(NBSocket_isClass(ref));
	NB_SOCKET_AUTOSAFE_ACTION_START(opq)
	NBASSERT(!opq->dbg.connectFailed) //socket must be closed after a 'connect' fail.
	{
		if(opq->type == type){
			r = TRUE;
		} else if(!NBHndl_isSet(opq->hndl)){
			opq->type = type;
			r = TRUE;
		}
	}
	NB_SOCKET_AUTOSAFE_ACTION_END(opq)
	return r;
}

//NBHndlCloseFnc

BOOL NBSocket_NBHndlCloseFnc_(STNBHndlNative* obj){
    BOOL r = FALSE;
    NB_SOCKET hndl;
    if(NBHndlNative_get(obj, &hndl, sizeof(hndl))){
#       ifdef _WIN32
        if(0 == closesocket(hndl)){
            hndl = 0;
            r = TRUE;
        } else {
            const int errCode = WSAGetLastError();
            PRINTF_ERROR("NBSocket, close failed with error %d '%s'.\n", errCode, NBSocket_getErrorName(errCode));
        }
#       else
        if(0 == close(hndl)){
            hndl = 0;
            r = TRUE;
        } else {
            PRINTF_ERROR("NBSocket, close failed with error %d '%s'.\n", errno, NBSocket_getErrorName(errno));
        }
#       endif
    }
    return r;
}

//handle/file-descriptor

BOOL NBSocket_createHndlLockedOpq_(STNBSocketOpq* opq){
	BOOL r = FALSE;
	NBASSERT(!NBHndl_isSet(opq->hndl))
	if(!NBHndl_isSet(opq->hndl)){
		NB_SOCKET hndl = INVALID_SOCKET;
		switch(opq->type) {
			case ENNBSocketHndType_TCP:
                hndl = (NB_SOCKET)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				break;
			case ENNBSocketHndType_UDP:
                hndl = (NB_SOCKET)socket(AF_INET, SOCK_DGRAM, 0);
				break;
			default:
				break;
		}
		//
		if(hndl == INVALID_SOCKET) {
			opq->err.num	= errno;
			__nb_strerror_r(opq->err.str, sizeof(opq->err.str), errno);
		} else {
            //release
            {
                if(NBHndl_isSet(opq->hndl)){
                    NBHndl_setOrphan(opq->hndl); //flag as orphan for every external consumer (like pollster).
                    NBHndl_release(&opq->hndl);
                    NBHndl_null(&opq->hndl);
                }
                opq->hndlNative = 0;
            }
            //allocate
            opq->hndl = NBHndl_alloc(NULL);
            if(!NBHndl_setNative(opq->hndl, ENNBHndlNativeType_Socket, &hndl, sizeof(hndl), NBSocket_NBHndlCloseFnc_)){
                NBASSERT(FALSE) //error
            } else {
                opq->err.num    = 0;
                opq->err.str[0] = '\0';
                opq->hndlNative = hndl;
                opq->isShutedForRcv = opq->isShutedForSend = FALSE;
                r = TRUE;
            }
		}
	}
	return r;
}
	
BOOL NBSocket_createHnd(STNBSocketRef ref){
	BOOL r = FALSE;
	STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque; NBASSERT(NBSocket_isClass(ref));
	NB_SOCKET_AUTOSAFE_ACTION_START(opq)
	NBASSERT(!opq->dbg.connectFailed) //socket must be closed after a 'connect' fail.
	if(!NBHndl_isSet(opq->hndl)){
		r = NBSocket_createHndlLockedOpq_(opq);
	}
	NB_SOCKET_AUTOSAFE_ACTION_END(opq)
	return r;
}

//Configure

//non-blocking mode

BOOL NBSocket_setNonBlocking(STNBSocketRef ref, const BOOL nonBlocking){
	BOOL r = FALSE;
	STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque; NBASSERT(NBSocket_isClass(ref));
	NB_SOCKET_AUTOSAFE_ACTION_START(opq)
	NBASSERT(!opq->dbg.connectFailed) //socket must be closed after a 'connect' fail.
	{
#		ifdef _WIN32
		//Create hndl (if necesary)
		if(!NBHndl_isSet(opq->hndl)){
			NBSocket_createHndlLockedOpq_(opq);
		}
		//Action
		if(NBHndl_isSet(opq->hndl)){
			//WindowsVista+
			// If iMode = 0, blocking is enabled; 
			// If iMode != 0, non-blocking mode is enabled.
			u_long iMode = (nonBlocking ? 1 : 0);
			if (ioctlsocket(opq->hndlNative, FIONBIO, &iMode) != NO_ERROR){
				//PRINTF_ERROR("NBSocket, setsockopt(%d, %d) failed.\n", b, c);
			} else {
				r = TRUE;
			}
		}
#		else //if defined(O_NONBLOCK) || defined(O_NDELAY) || defined(FNDELAY)
		//Create hndl (if necesary)
		if(!NBHndl_isSet(opq->hndl)){
			NBSocket_createHndlLockedOpq_(opq);
		}
		//Action
		if(NBHndl_isSet(opq->hndl)){
			int flags;
			if ((flags = fcntl(opq->hndlNative, F_GETFL, 0)) == -1){
				//PRINTF_ERROR("NBSocket, setsockopt(%d, %d) failed.\n", b, c);
			} else {
#				ifdef O_NONBLOCK
				if(nonBlocking) flags |= O_NONBLOCK;
				else flags &= ~O_NONBLOCK;
#				endif
#				ifdef O_NDELAY
				if(nonBlocking) flags |= O_NDELAY;
				else flags &= ~O_NDELAY;
#				endif
#				ifdef FNDELAY
				if(nonBlocking) flags |= FNDELAY;
				else flags &= ~FNDELAY;
#				endif
				if(fcntl(opq->hndlNative, F_SETFL, flags) == -1){
					//PRINTF_ERROR("NBSocket, setsockopt(%d, %d) failed.\n", b, c);
				} else {
					r = TRUE;
				}
			}
		}
#		endif
	}
	NB_SOCKET_AUTOSAFE_ACTION_END(opq)
	return r;
}

//Recommended to avoid SIGPIPE signals
BOOL NBSocket_setNoSIGPIPE(STNBSocketRef ref, const BOOL noSIGPIPE){
	BOOL r = FALSE;
	STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque; NBASSERT(NBSocket_isClass(ref));
	NB_SOCKET_AUTOSAFE_ACTION_START(opq)
	NBASSERT(!opq->dbg.connectFailed) //socket must be closed after a 'connect' fail.
	{
#		ifdef SO_NOSIGPIPE
		//Create hndl (if necesary)
		if(!NBHndl_isSet(opq->hndl)){
			NBSocket_createHndlLockedOpq_(opq);
		}
		//Action
		if(NBHndl_isSet(opq->hndl)){
			int v = (noSIGPIPE ? 1 : 0);
			if(setsockopt(opq->hndlNative, SOL_SOCKET, SO_NOSIGPIPE, (const char*)&v, sizeof(v)) < 0){
				//PRINTF_ERROR("NBSocket, setsockopt(%d, %d) failed.\n", b, c);
			} else {
				r = TRUE;
			}
		}
#		endif
	}
	NB_SOCKET_AUTOSAFE_ACTION_END(opq)
	return r;
}

//Recommended before bind;
//if not, the addr/port binded could still be reserver after closing the binded-socket.
BOOL NBSocket_setReuseAddr(STNBSocketRef ref, const BOOL reuse){
	BOOL r = FALSE;
	STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque; NBASSERT(NBSocket_isClass(ref));
	NB_SOCKET_AUTOSAFE_ACTION_START(opq)
	NBASSERT(!opq->dbg.connectFailed) //socket must be closed after a 'connect' fail.
	{
#		ifdef SO_REUSEADDR
		//Create hndl (if necesary)
		if(!NBHndl_isSet(opq->hndl)){
			NBSocket_createHndlLockedOpq_(opq);
		}
		//Action
		if(NBHndl_isSet(opq->hndl)){
			int v = (reuse ? 1 : 0);
			if(setsockopt(opq->hndlNative, SOL_SOCKET, SO_REUSEADDR, (const char*)&v, sizeof(v)) < 0){
				//PRINTF_ERROR("NBSocket, setsockopt(%d, %d) failed.\n", b, c);
			} else {
				r = TRUE;
			}
		}
#		endif
	}
	NB_SOCKET_AUTOSAFE_ACTION_END(opq)
	return r;
}

//Recommended before bind;
//if not, the addr/port binded could still be reserver after closing the binded-socket.
BOOL NBSocket_setReusePort(STNBSocketRef ref, const BOOL reuse){	//Recommended before bind
	BOOL r = FALSE;
	STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque; NBASSERT(NBSocket_isClass(ref));
	NB_SOCKET_AUTOSAFE_ACTION_START(opq)
	NBASSERT(!opq->dbg.connectFailed) //socket must be closed after a 'connect' fail.
	{
#		ifdef SO_REUSEPORT
		//Create hndl (if necesary)
		if(!NBHndl_isSet(opq->hndl)){
			NBSocket_createHndlLockedOpq_(opq);
		}
		//Action
		if(NBHndl_isSet(opq->hndl)){
			int v = (reuse ? 1 : 0);
			if(setsockopt(opq->hndlNative, SOL_SOCKET, SO_REUSEPORT, (const char*)&v, sizeof(v)) < 0){
				//PRINTF_ERROR("NBSocket, setsockopt(%d, %d) failed.\n", b, c);
			} else {
				r = TRUE;
			}
		}
#		endif
	}
	NB_SOCKET_AUTOSAFE_ACTION_END(opq)
	return r;
}

//If TRUE, "don't delay send to coalesce packets" (TRUE is inmediate).
//TCP_NODELAY is used for disabling Nagle's algorithm (waiting or accumulating before sending the data).
//Warning:avoid sending data in small sizes; use your own buffer.
BOOL NBSocket_setDelayEnabled(STNBSocketRef ref, const BOOL delayEnabled){
	BOOL r = FALSE;
	STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque; NBASSERT(NBSocket_isClass(ref));
	NB_SOCKET_AUTOSAFE_ACTION_START(opq)
	NBASSERT(!opq->dbg.connectFailed) //socket must be closed after a 'connect' fail.
	{
#		ifdef TCP_NODELAY
		//Create hndl (if necesary)
		if(!NBHndl_isSet(opq->hndl)){
			NBSocket_createHndlLockedOpq_(opq);
		}
		//Action
		if(NBHndl_isSet(opq->hndl)){
			int v = (delayEnabled ? 0 : 1);
			if(setsockopt(opq->hndlNative, IPPROTO_TCP, TCP_NODELAY, (const char*)&v, sizeof(v)) < 0){
				//PRINTF_ERROR("NBSocket, setsockopt(%d, %d) failed.\n", b, c);
			} else {
				r = TRUE;
			}
		}
#		endif
	}
	NB_SOCKET_AUTOSAFE_ACTION_END(opq)
	return r;
}

//If TRUE, "don't push last block of write" (FALSE is insmediate).
//TCP_NOPUSH enable a delay the last block of data untill data is acumulated (TCP_CORK in BSD).
//Warning:avoid sending data in small sizes; use your own buffer.
BOOL NBSocket_setCorkEnabled(STNBSocketRef ref, const BOOL corkEnabled){
	BOOL r = FALSE;
	STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque; NBASSERT(NBSocket_isClass(ref));
	NB_SOCKET_AUTOSAFE_ACTION_START(opq)
	NBASSERT(!opq->dbg.connectFailed) //socket must be closed after a 'connect' fail.
	{
#		ifdef TCP_NOPUSH
		//Create hndl (if necesary)
		if(!NBHndl_isSet(opq->hndl)){
			NBSocket_createHndlLockedOpq_(opq);
		}
		//Action
		if(NBHndl_isSet(opq->hndl)){
			int v = (corkEnabled ? 1 : 0);
			if(setsockopt(opq->hndlNative, IPPROTO_TCP, TCP_NOPUSH, (const char*)&v, sizeof(v)) < 0){
				//PRINTF_ERROR("NBSocket, setsockopt(%d, %d) failed.\n", b, c);
			} else {
				r = TRUE;
			}
		}
#		endif
	}
	NB_SOCKET_AUTOSAFE_ACTION_END(opq)
	return r;
}

//Server

BOOL NBSocket_bind(STNBSocketRef ref, int port){
	BOOL r = FALSE;
	STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque; NBASSERT(NBSocket_isClass(ref));
	NB_SOCKET_AUTOSAFE_ACTION_START(opq)
	NBASSERT(!opq->dbg.connectFailed) //socket must be closed after a 'connect' fail.
	{
		//Create hndl (if necesary)
		if(!NBHndl_isSet(opq->hndl)){
			NBSocket_createHndlLockedOpq_(opq);
		}
		//Action
		if(NBHndl_isSet(opq->hndl)){
			NBMemory_setZeroSt(opq->my_addr, struct sockaddr);
			{
				struct sockaddr_in* myAddr4 = (struct sockaddr_in*)&opq->my_addr;
				myAddr4->sin_family			= AF_INET;		//kernel order
				myAddr4->sin_port			= (u_short)htons((u_short)port);	//network order
				myAddr4->sin_addr.s_addr	= htonl(INADDR_ANY);	//automaticlly uses the local IP
			}
			if(bind(opq->hndlNative, (struct sockaddr *)&opq->my_addr, sizeof(opq->my_addr)) == 0){
				r = TRUE;
			}
		}
	}
	NB_SOCKET_AUTOSAFE_ACTION_END(opq)
	return r;
}

BOOL NBSocket_listen(STNBSocketRef ref){
	BOOL r = FALSE;
	STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque; NBASSERT(NBSocket_isClass(ref));
	NB_SOCKET_AUTOSAFE_ACTION_START(opq)
	NBASSERT(!opq->dbg.connectFailed) //socket must be closed after a 'connect' fail.
	{
		//Create hndl (if necesary)
		if(!NBHndl_isSet(opq->hndl)){
			NBSocket_createHndlLockedOpq_(opq);
		}
		//Action
		if(NBHndl_isSet(opq->hndl)){
			if(listen(opq->hndlNative, SOMAXCONN) == 0){
				r = TRUE;
			}
		}
	}
	NB_SOCKET_AUTOSAFE_ACTION_END(opq)
	return r;
}

BOOL NBSocket_accept(STNBSocketRef ref, STNBSocketRef dst){
	BOOL r = FALSE;
	if(NBSocket_isSet(dst)){
		STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque;
		NBASSERT(NBSocket_isClass(ref));
		NB_SOCKET_AUTOSAFE_ACTION_START(opq)
		NBASSERT(!opq->dbg.connectFailed) //socket must be closed after a 'connect' fail.
		NBASSERT(!(opq->async.activeMask & ENNBSocketAsyncActionBit_Accept)) //user logic error, multiple async taks
		if(!NBHndl_isSet(opq->hndl) || (opq->async.activeMask & ENNBSocketAsyncActionBit_Accept)){
			NB_SOCKET_AUTOSAFE_ACTION_END(opq)
		} else {
			NB_SOCKET hndlNative		= opq->hndlNative;
			struct sockaddr myAddr	= opq->my_addr;
			opq->async.activeMask |= ENNBSocketAsyncActionBit_Accept;
			NB_SOCKET_AUTOSAFE_ACTION_END(opq)
			//waitForClient (unlocked)
			{
				struct sockaddr remoteAddr;
				NB_SOCKLEN addrSize	= sizeof(remoteAddr);
				NB_SOCKET hndlNative2 = (NB_SOCKET)accept(hndlNative, &remoteAddr, &addrSize);
				if(hndlNative2 != INVALID_SOCKET){
					STNBSocketOpq* opq2 = (STNBSocketOpq*)dst.opaque;
					NB_SOCKET_AUTOSAFE_ACTION_START(opq2)
					{
                        //release
                        {
                            if(NBHndl_isSet(opq2->hndl)){
                                NBHndl_setOrphan(opq2->hndl); //flag as orphan for every external consumer (like pollster).
                                NBHndl_release(&opq2->hndl);
                                NBHndl_null(&opq2->hndl);
                            }
                            opq2->hndlNative = 0;
                        }
                        //allocate
                        opq2->hndl = NBHndl_alloc(NULL);
                        if(!NBHndl_setNative(opq2->hndl, ENNBHndlNativeType_Socket, &hndlNative2, sizeof(hndlNative2), NBSocket_NBHndlCloseFnc_)){
                            NBASSERT(FALSE) //error
                        } else {
                            opq2->err.num       = 0;
                            opq2->err.str[0]    = '\0';
                            opq2->my_addr       = myAddr;
                            opq2->their_addr    = remoteAddr;
                            opq2->hndlNative    = hndlNative2;
                            opq2->isShutedForRcv = opq2->isShutedForSend = FALSE;
                            r = TRUE;
                        }
					}
					NB_SOCKET_AUTOSAFE_ACTION_END(opq2)
					NBSOCKET_DBG_WAIT("NBSocket_accept")
				}
			}
			opq->async.activeMask &= ~ENNBSocketAsyncActionBit_Accept;
		}
	}
	return r;
}

//Stream

ENNBSocketResult NBSocket_connect(STNBSocketRef ref, const char* server, const int port){
	ENNBSocketResult r = ENNBSocketResult_Error;
	if(server != NULL && server[0] != '\0' && port > 0){
		struct in_addr hostAddr; BOOL hostAddrFnd = FALSE;
		NBMemory_setZeroSt(hostAddr, struct in_addr);
		//Get adddress (global lock; only one parallel call to 'gethostbyname()' is allowed)
		{
			NBASSERT(__globalSocketMutexInited)
			NBThreadMutex_lock(&__globalSocketMutex);
			{
				struct addrinfo hints, * result;
				NBMemory_setZeroSt(hints, struct addrinfo);
				hints.ai_family		= PF_UNSPEC;
				hints.ai_socktype	= SOCK_STREAM;
				hints.ai_flags		|= AI_CANONNAME;
				if (0 == getaddrinfo(server, NULL, &hints, &result)){
					struct addrinfo* res = result;
					while (res){
						if (res->ai_family == AF_INET) {
							hostAddr = ((struct sockaddr_in*)res->ai_addr)->sin_addr;
							hostAddrFnd = TRUE;
							break;
						//} else if (res->ai_family == AF_INET6) {
							//ToDo: enable 'sockaddr_in6'
							//ptr = &((struct sockaddr_in6*)res->ai_addr)->sin6_addr;
							//hostAddrFnd = TRUE;
							//break;
						}
						//next
						res = res->ai_next;
					}
					freeaddrinfo(result);
				}
				/*
				//The functions gethostbyname() and gethostbyaddr()
				//may return pointers to static data,
				//which may be overwritten by later calls.
				//Copying the struct hostent does not suffice,
				//since it contains pointers; a deep copy is required.
				struct hostent* host = gethostbyname(server);
				if(host && host->h_addr_list != NULL && host->h_addr_list[0] != NULL) {
					hostAddr = *((struct in_addr*)host->h_addr_list[0]);
					hostAddrFnd = TRUE;
				}
				*/
			}
			NBThreadMutex_unlock(&__globalSocketMutex);
		} 
		//Connect
		{
			STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque;
			NBASSERT(NBSocket_isClass(ref));
			NB_SOCKET_AUTOSAFE_ACTION_START(opq)
			NBASSERT(!opq->dbg.connectFailed) //socket must be closed after a 'connect' fail.
			NBASSERT(!(opq->async.activeMask & ENNBSocketAsyncActionBit_Connect)) //user logic error, multiple async taks
			if((opq->async.activeMask & ENNBSocketAsyncActionBit_Connect)){
				NB_SOCKET_AUTOSAFE_ACTION_END(opq)
			} else {
				opq->async.activeMask |= ENNBSocketAsyncActionBit_Connect;
				//Connect
				if(!hostAddrFnd){
					opq->err.num = errno;
					__nb_strerror_r(opq->err.str, sizeof(opq->err.str), errno);
					NB_SOCKET_AUTOSAFE_ACTION_END(opq)
				} else {
					struct sockaddr remoteAddr;
					NBMemory_setZeroSt(remoteAddr, struct sockaddr)
					{
						struct sockaddr_in* addr4 = (struct sockaddr_in*)&remoteAddr;
						addr4->sin_family	= AF_INET;
						addr4->sin_port		= (u_short)htons((u_short)port);
						addr4->sin_addr		= hostAddr;
					}
					//Create hndl (if necesary)
					if(!NBHndl_isSet(opq->hndl)){
						NBSocket_createHndlLockedOpq_(opq);
					}
					//Action
					if(!NBHndl_isSet(opq->hndl)){
						NB_SOCKET_AUTOSAFE_ACTION_END(opq)
					} else {
						int nret;
						NB_SOCKET hndlNative = opq->hndlNative;
						opq->their_addr	= remoteAddr;
						NB_SOCKET_AUTOSAFE_ACTION_END(opq)
						//Connect (unlocked)
						{
							nret = connect(hndlNative, (struct sockaddr*)&remoteAddr, sizeof(remoteAddr));
							if (nret == 0){
								r = ENNBSocketResult_Success;
							} else {
#								ifdef _WIN32
								const int errCode = WSAGetLastError();
								if(errCode == WSAEWOULDBLOCK || errCode == WSAEINPROGRESS || errCode == WSAEALREADY){
									r = ENNBSocketResult_WouldBlock;
								} else {
									//PRINTF_INFO("NBSocket, connect returned %d: '%d'.\n", r, errCode);
									IF_NBASSERT(opq->dbg.connectFailed = TRUE;)
								}
#								else
								if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINPROGRESS || errno == EALREADY){ //EINPROGRESS: first call; EALREADY: subsequent calls
									r = ENNBSocketResult_WouldBlock;
								} else {
									//PRINTF_INFO("NBSocket, connect returned %d: %d '%s'.\n", r, errno, NBSocket_getErrorName(errno));
									IF_NBASSERT(opq->dbg.connectFailed = TRUE;)
								}
#								endif
							}
						}
						NBSOCKET_DBG_WAIT("NBSocket_connect")
					}
				}
				opq->async.activeMask &= ~ENNBSocketAsyncActionBit_Connect;
			}
		}
	}
	return r;
}

//

SI32 NBSocket_recvOpq_(STNBSocketOpq* opq, void* buff, int buffSz){
    SI32 r = NB_IO_ERROR;
    NB_SOCKET_AUTOSAFE_ACTION_START(opq)
    NBASSERT(!opq->dbg.connectFailed) //socket must be closed after a 'connect' fail.
    NBASSERT(!(opq->async.activeMask & ENNBSocketAsyncActionBit_Receive)) //user logic error, multiple async taks
    if(!NBHndl_isSet(opq->hndl) || (opq->async.activeMask & ENNBSocketAsyncActionBit_Receive)){
        NB_SOCKET_AUTOSAFE_ACTION_END(opq)
    } else {
        NB_SOCKET hndlNative = opq->hndlNative;
        opq->async.activeMask |= ENNBSocketAsyncActionBit_Receive;
        NB_SOCKET_AUTOSAFE_ACTION_END(opq)
        //Receive (unlocked)
        {
            int rcvd = (int)recv(hndlNative, buff, buffSz, 0); //devuelve -1 si el socket se ha cerrado...
            if(rcvd > 0){
                r = rcvd;
                NBSOCKET_DBG_WAIT("NBSocket_recv")
            } else if(rcvd != 0){ //zero = socket propperly shuteddown
#                ifdef _WIN32
                const int errCode = WSAGetLastError();
                if(errCode == WSAEWOULDBLOCK){
                    r = 0;
                } else {
                    PRINTF_INFO("NBSocket, recv returned %d: %d '%s'.\n", r, errCode, NBSocket_getErrorName(errCode));
                }
#                else
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    r = 0;
                } else {
                    PRINTF_INFO("NBSocket, recv returned %d: %d '%s'.\n", r, errno, NBSocket_getErrorName(errno));
                }
#                endif
            }
        }
        opq->async.activeMask &= ~ENNBSocketAsyncActionBit_Receive;
    }
    return r;
}
    
SI32 NBSocket_recv(STNBSocketRef ref, void* buff, int buffSz){
    STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque; NBASSERT(NBSocket_isClass(ref));
    return NBSocket_recvOpq_(opq, buff, buffSz);
}

SI32 NBSocket_recvFrom(STNBSocketRef ref, void* buff, int buffSz, STNBSocketAddr* addr){
	SI32 r = NB_IO_ERROR;
	STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque; NBASSERT(NBSocket_isClass(ref));
	NB_SOCKET_AUTOSAFE_ACTION_START(opq)
	NBASSERT(!opq->dbg.connectFailed) //socket must be closed after a 'connect' fail.
	NBASSERT(!(opq->async.activeMask & ENNBSocketAsyncActionBit_Receive)) //user logic error, multiple async taks
	if(!NBHndl_isSet(opq->hndl) || (opq->async.activeMask & ENNBSocketAsyncActionBit_Receive)){
		NB_SOCKET_AUTOSAFE_ACTION_END(opq)
	} else {
		NB_SOCKET hndlNative	= opq->hndlNative;
		NB_SOCKLEN addrLen	= (addr == 0 ? 0 : sizeof(*addr)); NBASSERT(sizeof(struct sockaddr) <= sizeof(*addr))
		opq->async.activeMask |= ENNBSocketAsyncActionBit_Receive;
		NB_SOCKET_AUTOSAFE_ACTION_END(opq)
		//Receive (unlocked)
		{
			const int rcvd = (int)recvfrom(hndlNative, buff, buffSz, 0, (struct sockaddr*)addr, &addrLen); //devuelve -1 si el socket se ha cerrado...
			if(rcvd > 0){
				r = rcvd;
				//NBSocketHnd, from len(16) fam(2) port(13351) addr(1090627776).
				NBSOCKET_DBG_WAIT("NBSocket_recvFrom")
			} else if(rcvd != 0){ //zero = socket propperly shuteddown
#				ifdef _WIN32
				const int errCode = WSAGetLastError();
				if(errCode == WSAEWOULDBLOCK){
					r = 0;
				} else {
					//PRINTF_INFO("NBSocket, recvFrom returned %d: %d '%s'.\n", r, errCode, NBSocket_getErrorName(errCode));
				}
#				else
				if(errno == EAGAIN || errno == EWOULDBLOCK){
					r = 0;
				} else {
					//PRINTF_INFO("NBSocket, recvFrom returned %d: %d '%s'.\n", r, errno, NBSocket_getErrorName(errno));
				}
#				endif
			}
		}
		opq->async.activeMask &= ~ENNBSocketAsyncActionBit_Receive;
	}
	return r;
}

//Messages header (for multiple-packets-read calls)

typedef struct STNBSocketPacketsOpq_ {
#	ifdef NB_SOCKET_RECVMMSG_ENABLED
	struct iovec*		iovs;	//data destinations
	struct mmsghdr*		hdrs;	//messages headers
#	else
	//ToDo: implement Windows equivalent
#	endif
	void**				buffsPtrs; //pointers to original buffers (for cross-platform coding vaidation)
	UI32				size;
	ENNBSocketPacketsBuffsType type;
} STNBSocketPacketsOpq;

void NBSocketPacketsOpq_init(STNBSocketPacketsOpq* opq){
	NBMemory_setZeroSt(*opq, STNBSocketPacketsOpq);
}

void NBSocketPacketsOpq_deallocBuffers(STNBSocketPacketsOpq* opq){
	NBASSERT(opq != NULL)
#	ifdef NB_SOCKET_RECVMMSG_ENABLED
	if(opq->iovs != NULL){
		NBMemory_free(opq->iovs);
		opq->iovs = NULL;
	}
	if(opq->hdrs != NULL){
		NBMemory_free(opq->hdrs);
		opq->hdrs = NULL;
	}
#	else
	//ToDo: implement Windows equivalent
#	endif
	if(opq->buffsPtrs != NULL){
		NBMemory_free(opq->buffsPtrs);
		opq->buffsPtrs = NULL;
	}
	opq->size = 0;
}

void NBSocketPacketsOpq_allocBuffers(STNBSocketPacketsOpq* opq, const UI32 size, const ENNBSocketPacketsBuffsType type){
	NBASSERT(opq != NULL)
	NBSocketPacketsOpq_deallocBuffers(opq);
#	ifdef NB_SOCKET_RECVMMSG_ENABLED
	{
		opq->iovs	= NBMemory_allocTypes(struct iovec, size);
		NBMemory_set(opq->iovs, 0, sizeof(opq->iovs[0]) * size);
	}
	{
		opq->hdrs	= NBMemory_allocTypes(struct mmsghdr, size);
		NBMemory_set(opq->hdrs, 0, sizeof(opq->hdrs[0]) * size);
	}
#	else
	//ToDo: implement Windows equivalent
#	endif
	{
		opq->buffsPtrs = NBMemory_allocTypes(void*, size);
		NBMemory_set(opq->buffsPtrs, 0, sizeof(opq->buffsPtrs[0]) * size);
	}
	opq->size = size;
	opq->type = type;
}

void NBSocketPacketsOpq_release(STNBSocketPacketsOpq* opq){
	NBASSERT(opq != NULL)
	NBSocketPacketsOpq_deallocBuffers(opq);
}

//

void NBSocketPackets_init(STNBSocketPackets* obj){
	NBMemory_setZeroSt(*obj, STNBSocketPackets);
	//opaque
	obj->opaque = NBMemory_allocType(STNBSocketPacketsOpq);
	NBSocketPacketsOpq_init((STNBSocketPacketsOpq*)obj->opaque);
}

void NBSocketPackets_deallocBuffers(STNBSocketPackets* obj){
	STNBSocketPacketsOpq* opq = (STNBSocketPacketsOpq*)obj->opaque;
	NBASSERT(obj != NULL)
	if(obj->addrs != NULL){
		NBMemory_free(obj->addrs);
		obj->addrs = NULL;
	}
	if(obj->buffs != NULL){
		if(opq->type == ENNBSocketPacketsBuffsType_Internal){
			if(obj->size > 0){
				NBDataPtr_ptrsReleaseGrouped(obj->buffs, obj->size);
			}
			NBMemory_free(obj->buffs);
		}
		obj->buffs = NULL;
	}
	NBASSERT(obj->opaque != NULL)
	if(obj->opaque != NULL){
		NBSocketPacketsOpq_deallocBuffers((STNBSocketPacketsOpq*)obj->opaque);
	}
	obj->size = 0;
}

void NBSocketPackets_syncInternalRng(STNBSocketPackets* obj, const UI32 iFirst, const UI32 count){
	NBASSERT(obj->opaque != NULL)
	if(obj->opaque != NULL){
		STNBSocketPacketsOpq* opq = (STNBSocketPacketsOpq*)obj->opaque;
		NBASSERT(obj->size == opq->size)
		UI32 iAfterEnd = iFirst + count;
		if(iAfterEnd > opq->size) iAfterEnd = opq->size;
#		ifdef NB_SOCKET_RECVMMSG_ENABLED
		{
			UI32 i; for(i = iFirst; i < iAfterEnd; i++){
				struct iovec* iov	= &opq->iovs[i];
				struct mmsghdr* hdr	= &opq->hdrs[i];
				iov->iov_base		= obj->buffs[i].def.alloc.ptr;
				iov->iov_len		= obj->buffs[i].def.alloc.size;
				if(obj->addrs == NULL){
					hdr->msg_hdr.msg_name = NULL;
					hdr->msg_hdr.msg_namelen = 0;
				} else {
					hdr->msg_hdr.msg_name = &obj->addrs[i];
					hdr->msg_hdr.msg_namelen = sizeof(obj->addrs[0]);
				}
				hdr->msg_hdr.msg_iov = iov;
				hdr->msg_hdr.msg_iovlen = 1;
			}
		}
#		else
		//ToDo: implement Windows equivalent
#		endif
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		//pointers to original buffers (for cross-platform coding vaidation)
		{
			UI32 i; for(i = iFirst; i < iAfterEnd; i++){
				opq->buffsPtrs[i]	= obj->buffs[i].def.alloc.ptr;
			}
		}
#		endif
	}
}

void NBSocketPackets_empty(STNBSocketPackets* obj){
	NBASSERT(obj != NULL)
	obj->use = 0;
}

void NBSocketPackets_allocBuffersSameSizes(STNBSocketPackets* obj, const UI32 buffsAmm, const UI32 bytesPerBuff){
	//empty
	NBSocketPackets_deallocBuffers(obj);
	//allocate
	if(buffsAmm > 0){
		obj->addrs		= NBMemory_allocTypes(STNBSocketAddr, buffsAmm);
		obj->buffs		= NBMemory_allocTypes(STNBDataPtr, buffsAmm);
		{
			UI32 i; for(i = 0; i < buffsAmm; i++){
				STNBDataPtr* buff = &obj->buffs[i];
				NBDataPtr_init(buff);
				if(bytesPerBuff > 0){
					NBDataPtr_allocEmptyPtr(buff, bytesPerBuff);
				}
			}
		}
		obj->size	= buffsAmm;
		//
		NBASSERT(obj->opaque != NULL)
		if(obj->opaque != NULL){
			STNBSocketPacketsOpq* opq = (STNBSocketPacketsOpq*)obj->opaque;
			NBSocketPacketsOpq_allocBuffers(opq, buffsAmm, ENNBSocketPacketsBuffsType_Internal);
			NBASSERT(obj->size == opq->size)
#			ifdef NB_SOCKET_RECVMMSG_ENABLED
			{
				UI32 i; for(i = 0; i < buffsAmm; i++){
					struct iovec* iov	= &opq->iovs[i];
					struct mmsghdr* hdr	= &opq->hdrs[i];
					iov->iov_base		= obj->buffs[i].def.alloc.ptr;
					iov->iov_len		= obj->buffs[i].def.alloc.size;
					if(obj->addrs == NULL){
						hdr->msg_hdr.msg_name = NULL;
						hdr->msg_hdr.msg_namelen = 0;
					} else {
						hdr->msg_hdr.msg_name = &obj->addrs[i];
						hdr->msg_hdr.msg_namelen = sizeof(obj->addrs[i]);
					}
					hdr->msg_hdr.msg_iov = iov;
					hdr->msg_hdr.msg_iovlen = 1;
				}
			}
#			else
			//ToDo: implement Windows equivalent
#			endif
#			ifdef NB_CONFIG_INCLUDE_ASSERTS
			//pointers to original buffers (for cross-platform coding vaidation)
			{
				UI32 i; for(i = 0; i < buffsAmm; i++){
					opq->buffsPtrs[i]	= obj->buffs[i].def.alloc.ptr;
				}
			}
#			endif
		}
		{
			IF_NBASSERT(obj->dbg.bytesPerBuff = bytesPerBuff;) 
		}
	}
}

void NBSocketPackets_useExternalBuffers(STNBSocketPackets* obj, STNBDataPtr* ptrs, const UI32 ptrsSz){
	ENNBSocketPacketsBuffsType type = ENNBSocketPacketsBuffsType_Internal;
	STNBSocketPacketsOpq* opq = (STNBSocketPacketsOpq*)obj->opaque;
	if(opq != NULL){
		type = opq->type;
	}
	if(type == ENNBSocketPacketsBuffsType_External && obj->size == ptrsSz){
		//just sync ptrs
		obj->buffs		= ptrs;
		obj->size		= ptrsSz;
		NBSocketPackets_syncInternalRng(obj, 0, ptrsSz);
	} else {
		//rebuild
		//empty
		NBSocketPackets_deallocBuffers(obj);
		//allocate
		if(ptrsSz > 0){
			obj->addrs		= NBMemory_allocTypes(STNBSocketAddr, ptrsSz);
			obj->buffs		= ptrs;
			obj->size		= ptrsSz;
			//
			NBASSERT(obj->opaque != NULL)
			if(obj->opaque != NULL){
				STNBSocketPacketsOpq* opq = (STNBSocketPacketsOpq*)obj->opaque;
				NBSocketPacketsOpq_allocBuffers(opq, ptrsSz, ENNBSocketPacketsBuffsType_External);
				NBASSERT(obj->size == opq->size)
#				ifdef NB_SOCKET_RECVMMSG_ENABLED
				{
					UI32 i; for(i = 0; i < ptrsSz; i++){
						struct iovec* iov	= &opq->iovs[i];
						struct mmsghdr* hdr	= &opq->hdrs[i];
						iov->iov_base		= obj->buffs[i].def.alloc.ptr;
						iov->iov_len		= obj->buffs[i].def.alloc.size;
						if(obj->addrs == NULL){
							hdr->msg_hdr.msg_name = NULL;
							hdr->msg_hdr.msg_namelen = 0;
						} else {
							hdr->msg_hdr.msg_name = &obj->addrs[i];
							hdr->msg_hdr.msg_namelen = sizeof(obj->addrs[i]);
						}
						hdr->msg_hdr.msg_iov = iov;
						hdr->msg_hdr.msg_iovlen = 1;
					}
				}
#				else
				//ToDo: implement Windows equivalent
#				endif
#				ifdef NB_CONFIG_INCLUDE_ASSERTS
				//pointers to original buffers (for cross-platform coding vaidation)
				{
					UI32 i; for(i = 0; i < ptrsSz; i++){
						opq->buffsPtrs[i]	= obj->buffs[i].def.alloc.ptr;
					}
				}
#				endif
			}
			{
				IF_NBASSERT(obj->dbg.bytesPerBuff = 0;) //External 
			}
		}
	}
}

void NBSocketPackets_release(STNBSocketPackets* obj){
	NBASSERT(obj != NULL)
	NBSocketPackets_deallocBuffers(obj);
	//opaque
	NBASSERT(obj->opaque != NULL)
	if(obj->opaque != NULL){
		NBSocketPacketsOpq_release((STNBSocketPacketsOpq*)obj->opaque);
		NBMemory_free(obj->opaque);
		obj->opaque = NULL;
	}
}

//

SI32 NBSocket_recvMany(STNBSocketRef ref, STNBSocketPackets* pckts){
	SI32 r = NB_IO_ERROR;
	STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque; NBASSERT(NBSocket_isClass(ref));
	NB_SOCKET_AUTOSAFE_ACTION_START(opq)
	NBASSERT(!opq->dbg.connectFailed) //socket must be closed after a 'connect' fail.
	NBASSERT(!(opq->async.activeMask & ENNBSocketAsyncActionBit_Receive)) //user logic error, multiple async taks
	if(!NBHndl_isSet(opq->hndl) || (opq->async.activeMask & ENNBSocketAsyncActionBit_Receive)){
		NB_SOCKET_AUTOSAFE_ACTION_END(opq)
	} else {
		NB_SOCKET hndlNative	= opq->hndlNative;
		opq->async.activeMask |= ENNBSocketAsyncActionBit_Receive;
		NB_SOCKET_AUTOSAFE_ACTION_END(opq)
		//Receive (unlocked)
		{
#			ifdef NB_SOCKET_RECVMMSG_ENABLED
			//optimized call
			if(pckts != NULL && pckts->opaque != NULL && pckts->use < pckts->size){
				STNBSocketPacketsOpq* pcktsOpq = (STNBSocketPacketsOpq*)pckts->opaque;
				NBASSERT(pcktsOpq->buffsPtrs != NULL)
				NBASSERT(pckts->size == pcktsOpq->size)
				//Validate integrity (user shoudl use 'NBSocketPackets_syncInternal*' internal pointers where manipulated)
#				ifdef NB_CONFIG_INCLUDE_ASSERTS
				{
					const UI32 idx				= pckts->use;
					const struct iovec* iov		= &pcktsOpq->iovs[idx];
					const struct mmsghdr* hdr	= &pcktsOpq->hdrs[idx];
					NBASSERT(iov->iov_base == pckts->buffs[idx].def.alloc.ptr)
					NBASSERT(iov->iov_len > 0 && iov->iov_len == pckts->buffs[idx].def.alloc.size)
					NBASSERT((pckts->addrs == NULL && hdr->msg_hdr.msg_name == NULL) || (pckts->addrs != NULL && hdr->msg_hdr.msg_name == &pckts->addrs[idx]))
					NBASSERT((pckts->addrs == NULL && hdr->msg_hdr.msg_namelen == 0) || (pckts->addrs != NULL && hdr->msg_hdr.msg_namelen > 0))
					NBASSERT(hdr->msg_hdr.msg_iov == iov)
					NBASSERT(hdr->msg_hdr.msg_iovlen == 1)
				}
#				endif
				NBASSERT(pcktsOpq->buffsPtrs[pckts->use] == pckts->buffs[pckts->use].def.alloc.ptr)
				r = recvmmsg(hndlNative, &pcktsOpq->hdrs[pckts->use], (pckts->size - pckts->use), 0, NULL);
				if (r > 0) {
					//apply results
					int i; for(i = 0; i < r; i++){
						//Validate integrity (user should call 'NBSocketPackets_syncInternal*' internal pointers where manipulated)
#						ifdef NB_CONFIG_INCLUDE_ASSERTS
						{
							const UI32 idx				= pckts->use;
							const struct iovec* iov		= &pcktsOpq->iovs[idx];
							const struct mmsghdr* hdr	= &pcktsOpq->hdrs[idx];
							NBASSERT(iov->iov_base == pckts->buffs[idx].def.alloc.ptr)
							NBASSERT(iov->iov_len > 0 && iov->iov_len == pckts->buffs[idx].def.alloc.size)
							NBASSERT((pckts->addrs == NULL && hdr->msg_hdr.msg_name == NULL) || (pckts->addrs != NULL && hdr->msg_hdr.msg_name == &pckts->addrs[idx]))
							NBASSERT((pckts->addrs == NULL && hdr->msg_hdr.msg_namelen == 0) || (pckts->addrs != NULL && hdr->msg_hdr.msg_namelen > 0))
							NBASSERT(hdr->msg_hdr.msg_iov == iov)
							NBASSERT(hdr->msg_hdr.msg_iovlen == 1)
						}
#						endif
						NBASSERT(pcktsOpq->buffsPtrs[pckts->use] == pckts->buffs[pckts->use].def.alloc.ptr) //buffer manipulated outside
						//set
						pckts->buffs[pckts->use].ptr	= pckts->buffs[pckts->use].def.alloc.ptr;
						pckts->buffs[pckts->use].use	= pcktsOpq->hdrs[pckts->use].msg_len;
						pckts->use++;
					}
				} else if(r != 0){
					if(errno == EAGAIN || errno == EWOULDBLOCK){
						r = 0;
					} else {
						//PRINTF_INFO("NBSocket, recvmmsg returned %d: %d '%s'.\n", r, errno, NBSocket_getErrorName(errno));
					}
				}
			}
#			else
			//normal crcv all
			if(pckts != NULL && pckts->use < pckts->size){
				IF_NBASSERT(STNBSocketPacketsOpq* pcktsOpq = (STNBSocketPacketsOpq*)pckts->opaque;)
				NB_SOCKLEN addrLen	= (pckts->addrs != NULL ? sizeof(pckts->addrs[0]) : 0); NBASSERT(pckts->addrs == NULL || sizeof(struct sockaddr) <= sizeof(pckts->addrs[0]))
				//
				NBASSERT(pcktsOpq->buffsPtrs != NULL)
				NBASSERT(pckts->size == pcktsOpq->size)
				//Validate integrity (user shoudl use 'NBSocketPackets_syncInternal*' internal pointers where manipulated)
				NBASSERT(pcktsOpq->buffsPtrs[pckts->use] == pckts->buffs[pckts->use].def.alloc.ptr)
				const int rcvd = (int)recvfrom(hndlNative, pckts->buffs[pckts->use].def.alloc.ptr, pckts->buffs[pckts->use].def.alloc.size, 0, (pckts->addrs != NULL ? (struct sockaddr*)&pckts->addrs[pckts->use]: NULL), (pckts->addrs != NULL ? &addrLen : NULL)); //devuelve -1 si el socket se ha cerrado...
				if(rcvd > 0){
					//Validate integrity (user should call 'NBSocketPackets_syncInternal*' internal pointers where manipulated)
					NBASSERT(pcktsOpq->buffsPtrs[pckts->use] == pckts->buffs[pckts->use].def.alloc.ptr)
					//set
					pckts->buffs[pckts->use].ptr	= pckts->buffs[pckts->use].def.alloc.ptr;
					pckts->buffs[pckts->use].use	= rcvd;
					pckts->use++;
					r = 1;
					//NBSocketHnd, from len(16) fam(2) port(13351) addr(1090627776).
					NBSOCKET_DBG_WAIT("NBSocket_recvFrom")
				} else if(rcvd == 0){
					r = 0;
				} else {
#					ifdef _WIN32
					const int errCode = WSAGetLastError();
					if(errCode == WSAEWOULDBLOCK){
						r = 0;
					} else {
						PRINTF_INFO("NBSocket, recvfrom returned %d: %d '%s'.\n", r, errCode, NBSocket_getErrorName(errCode));
					}
#					else
					if(errno == EAGAIN || errno == EWOULDBLOCK){
						r = 0;
					} else {
						PRINTF_INFO("NBSocket, recvfrom returned %d: %d '%s'.\n", r, errno, NBSocket_getErrorName(errno));
					}
#					endif
				}
			}
#			endif
		}
		opq->async.activeMask &= ~ENNBSocketAsyncActionBit_Receive;
	}
	return r;
}

SI32 NBSocket_sendOpq_(STNBSocketOpq* opq, const void* data, int dataSz){
    SI32 r = NB_IO_ERROR;
    NB_SOCKET_AUTOSAFE_ACTION_START(opq)
    NBASSERT(!opq->dbg.connectFailed) //socket must be closed after a 'connect' fail.
    NBASSERT(!(opq->async.activeMask & ENNBSocketAsyncActionBit_Send)) //user logic error, multiple async taks
    if(!NBHndl_isSet(opq->hndl) || (opq->async.activeMask & ENNBSocketAsyncActionBit_Send)){
        NB_SOCKET_AUTOSAFE_ACTION_END(opq)
    } else {
        NB_SOCKET hndlNative = opq->hndlNative;
        opq->async.activeMask |= ENNBSocketAsyncActionBit_Send;
        NB_SOCKET_AUTOSAFE_ACTION_END(opq)
        //Send (unlocked)
        {
            const int sent = (int)send(hndlNative, data, dataSz, 0);
            if(sent > 0){
                r = sent;
                NBSOCKET_DBG_WAIT("NBSocket_send")
            } else if(sent != 0){ //zero = socket propperly shuteddown
#                ifdef _WIN32
                const int errCode = WSAGetLastError();
                if(errCode == WSAEWOULDBLOCK){
                    r = 0;
                } else {
                    //PRINTF_INFO("NBSocket, send returned %d: '%d'.\n", r, errCode);
                    //If send fails, better release socket because the second-call could rise SIGPIPE.
                    //NBSocket_shutdown(obj, NB_IO_BITS_RDWR);
                    
                }
#                else
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    r = 0;
                } else {
                    //PRINTF_INFO("NBSocket, send returned %d: '%s'.\n", r, NBSocket_getErrorName(errno));
                    //If send fails, better release socket because the second-call could rise SIGPIPE.
                    //NBSocket_shutdown(obj, NB_IO_BITS_RDWR);
                }
#                endif
            }
        }
        opq->async.activeMask &= ~ENNBSocketAsyncActionBit_Send;
    }
    return r;
}
    
SI32 NBSocket_send(STNBSocketRef ref, const void* data, int dataSz){
	STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque; NBASSERT(NBSocket_isClass(ref));
    return NBSocket_sendOpq_(opq, data, dataSz);
}

BOOL NBSocket_isShutedForRcv(STNBSocketRef ref){
	BOOL r = FALSE;
	STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque; NBASSERT(NBSocket_isClass(ref));
	NB_SOCKET_AUTOSAFE_ACTION_START(opq)
	{
		r = opq->isShutedForRcv;
	}
	NB_SOCKET_AUTOSAFE_ACTION_END(opq)
	return r;
}

BOOL NBSocket_isShutedForSend(STNBSocketRef ref){
	BOOL r = FALSE;
	STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque; NBASSERT(NBSocket_isClass(ref));
	NB_SOCKET_AUTOSAFE_ACTION_START(opq)
	{
		r = opq->isShutedForSend;
	}
	NB_SOCKET_AUTOSAFE_ACTION_END(opq)
	return r;
}

void NBSocket_shutdownOpq_(STNBSocketOpq* opq, const UI8 mask){ //NB_IO_BIT_READ | NB_IO_BIT_WRITE
    NB_SOCKET_AUTOSAFE_ACTION_START(opq)
    if(NBHndl_isSet(opq->hndl)){
#       ifdef _WIN32
        if((mask & NB_IO_BIT_READ) && (mask & NB_IO_BIT_WRITE)){
            opq->isShutedForRcv     = TRUE; //Cannot receive anymore
            opq->isShutedForSend    = TRUE; //Cannot send anymore
            shutdown(opq->hndlNative, SD_BOTH);
        } else if(mask & NB_IO_BIT_READ){
            opq->isShutedForRcv     = TRUE; //Cannot receive anymore
            shutdown(opq->hndlNative, SD_RECEIVE);
        } else if(mask & NB_IO_BIT_WRITE){
            opq->isShutedForSend    = TRUE; //Cannot send anymore
            shutdown(opq->hndlNative, SD_SEND);
        }
#       else
        if((mask & NB_IO_BIT_READ) && (mask & NB_IO_BIT_WRITE)){
            opq->isShutedForRcv     = TRUE; //Cannot receive anymore
            opq->isShutedForSend    = TRUE; //Cannot send anymore
            shutdown(opq->hndlNative, SHUT_RDWR);
        } else if(mask & NB_IO_BIT_READ){
            opq->isShutedForRcv     = TRUE; //Cannot receive anymore
            shutdown(opq->hndlNative, SHUT_RD);
        } else if(mask & NB_IO_BIT_WRITE){
            opq->isShutedForSend    = TRUE; //Cannot send anymore
            shutdown(opq->hndlNative, SHUT_WR);
        }
#       endif
    }
    NB_SOCKET_AUTOSAFE_ACTION_END(opq)
}

void NBSocket_shutdown(STNBSocketRef ref, const UI8 mask){ //NB_IO_BIT_READ | NB_IO_BIT_WRITE
	STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque; NBASSERT(NBSocket_isClass(ref));
    NBSocket_shutdownOpq_(opq, mask);
}

void NBSocket_closeOpq_(STNBSocketOpq* opq){
    NB_SOCKET_UNSAFE_ACTION_START(opq)
    //handle/file-descriptor
    if(NBHndl_isSet(opq->hndl)){
        NBHndl_setOrphan(opq->hndl); //flag as orphan for every external consumer (like pollster).
        NBHndl_release(&opq->hndl);
        NBHndl_null(&opq->hndl);
    }
    opq->hndlNative = 0;
    IF_NBASSERT(opq->dbg.connectFailed = FALSE;)
    NB_SOCKET_UNSAFE_ACTION_END(opq)
}

void NBSocket_close(STNBSocketRef ref){
	STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque; NBASSERT(NBSocket_isClass(ref));
    NBSocket_closeOpq_(opq);
}

BOOL NBSocket_isClosed(STNBSocketRef ref){
	STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque; NBASSERT(NBSocket_isClass(ref));
	return (!NBHndl_isSet(opq->hndl));
}

//Lnk

BOOL NBSocket_getIOLnk(STNBSocketRef ref, STNBIOLnk* dst){
    BOOL r = FALSE;
    STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque; NBASSERT(NBSocket_isClass(ref));
    if(opq != NULL && dst != NULL){
        STNBIOLnkItf itf;
        NBMemory_setZeroSt(itf, STNBIOLnkItf);
        //no-lock required
        itf.ioRetain   = NBObjRef_retainOpq;
        itf.ioRelease  = NBObjRef_releaseOpq;
        //
        itf.ioGetFD    = NBSocket_ioGetFD_;
        itf.ioIsObjRef = NBSocket_ioIsObjRef_;
        //
        itf.ioRead     = NBSocket_ioRead_;
        itf.ioWrite    = NBSocket_ioWrite_;
        itf.ioFlush    = NBSocket_ioFlush_;
        itf.ioShutdown = NBSocket_ioShutdown_;
        itf.ioClose    = NBSocket_ioClose_;
        //
        NBIOLnk_setItf(dst, &itf, opq);
        //
        r = TRUE;
    }
    return r;
}

int NBSocket_ioGetFD_(void* usrData){
    STNBSocketOpq* opq = (STNBSocketOpq*)usrData; NBASSERT(NBSocket_isClass(NBObjRef_fromOpqPtr(opq)));
    return NBSocket_getFDOpq_(opq);
}

BOOL NBSocket_ioIsObjRef_(STNBObjRef objRef, void* usrData){
    STNBSocketOpq* opq = (STNBSocketOpq*)usrData; NBASSERT(NBSocket_isClass(NBObjRef_fromOpqPtr(opq)));
    //NBASSERT(NBSocket_isClass(*objRef)); //allowed to compare NonSocket objects
    return (opq == objRef.opaque);
}

SI32 NBSocket_ioRead_(void* dst, const SI32 dstSz, void* usrData){ //read data to destination buffer, returns the ammount of bytes read, negative in case of error
    SI32 r = NB_IO_ERROR;
    STNBSocketOpq* opq = (STNBSocketOpq*)usrData; NBASSERT(NBSocket_isClass(NBObjRef_fromOpqPtr(opq)));
    if(opq != NULL){
        r = NBSocket_recvOpq_(opq, dst, dstSz);
    }
    return r;
}

SI32 NBSocket_ioWrite_(const void* src, const SI32 srcSz, void* usrData){ //write data from source buffer, returns the ammount of bytes written, negative in case of error
    SI32 r = NB_IO_ERROR;
    STNBSocketOpq* opq = (STNBSocketOpq*)usrData; NBASSERT(NBSocket_isClass(NBObjRef_fromOpqPtr(opq)));
    if(opq != NULL){
        r = NBSocket_sendOpq_(opq, src, srcSz);
    }
    return r;
}

void NBSocket_ioFlush_(void* usrData){
    STNBSocketOpq* opq = (STNBSocketOpq*)usrData; NBASSERT(NBSocket_isClass(NBObjRef_fromOpqPtr(opq)));
    if(opq != NULL){
        NBASSERT(FALSE) //ToDo: implement.
    }
}

void NBSocket_ioShutdown_(const UI8 mask, void* usrData){ //NB_IO_BIT_READ | NB_IO_BIT_WRITE
    STNBSocketOpq* opq = (STNBSocketOpq*)usrData; NBASSERT(NBSocket_isClass(NBObjRef_fromOpqPtr(opq)));
    if(opq != NULL){
        NBSocket_shutdownOpq_(opq, mask);
    }
}

void NBSocket_ioClose_(void* usrData){
    STNBSocketOpq* opq = (STNBSocketOpq*)usrData; NBASSERT(NBSocket_isClass(NBObjRef_fromOpqPtr(opq)));
    if(opq != NULL){
        NBSocket_closeOpq_(opq);
    }
}

//Address

BOOL NBSocket_getAddressLocal(STNBSocketRef ref, STNBSocketAddr* dst){
	BOOL r = FALSE;
	STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque; NBASSERT(NBSocket_isClass(ref));
	NB_SOCKET_AUTOSAFE_ACTION_START(opq)
	if(NBHndl_isSet(opq->hndl)){
		if(dst != NULL){
			NBASSERT(sizeof(opq->my_addr) < sizeof(*dst))
			*dst = *((STNBSocketAddr*)&opq->my_addr);
		} 
		r = TRUE;
	}
	NB_SOCKET_AUTOSAFE_ACTION_END(opq)
	return r;
}	

BOOL NBSocket_getAddressRemote(STNBSocketRef ref, STNBSocketAddr* dst){
	BOOL r = FALSE;
	STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque; NBASSERT(NBSocket_isClass(ref));
	NB_SOCKET_AUTOSAFE_ACTION_START(opq)
	if(NBHndl_isSet(opq->hndl)){
		if(dst != NULL){
			NBASSERT(sizeof(opq->their_addr) < sizeof(*dst))
			*dst = *((STNBSocketAddr*)&opq->their_addr);
		} 
		r = TRUE;
	}
	NB_SOCKET_AUTOSAFE_ACTION_END(opq)
	return r;
}

//Use less as posible

int NBSocket_getFDOpq_(STNBSocketOpq* opq){
    return (opq->hndlNative != 0 ? opq->hndlNative : -1);
}

int NBSocket_getFD_(STNBSocketRef ref){
	STNBSocketOpq* opq = (STNBSocketOpq*)ref.opaque; NBASSERT(NBSocket_isClass(ref));
	return NBSocket_getFDOpq_(opq);
}

//--------------
//- NBSocketAddr
//--------------

BOOL NBCompare_NBSocketAddr_addrOnly(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBSocketAddr))
	NBASSERT(sizeof(struct sockaddr) <= sizeof(STNBSocketAddr))
	struct sockaddr* o1 = (struct sockaddr*)data1;
	struct sockaddr* o2 = (struct sockaddr*)data2;
	{
		NBCompareFunc cmp = NULL;
		NBASSERT(o1->sa_family >= 0 && o1->sa_family < AF_MAX)
		NBASSERT(o2->sa_family >= 0 && o2->sa_family < AF_MAX)
		if(o1->sa_family >= 0 && o1->sa_family < AF_MAX){
			cmp = NBCompare_NBSocketAddr_addrOnlyByFamily[o1->sa_family];
			NBASSERT(cmp != NULL)
		}
		switch (mode) {
			case ENCompareMode_Equal:
				return o1->sa_family == o2->sa_family && (cmp == NULL || (*cmp)(mode, data1, data2, dataSz));
			case ENCompareMode_Lower:
				return o1->sa_family < o2->sa_family || (o1->sa_family == o2->sa_family && (cmp == NULL || (*cmp)(mode, data1, data2, dataSz)));
			case ENCompareMode_LowerOrEqual:
				return o1->sa_family < o2->sa_family || (o1->sa_family == o2->sa_family && (cmp == NULL || (*cmp)(mode, data1, data2, dataSz)));
			case ENCompareMode_Greater:
				return o1->sa_family > o2->sa_family || (o1->sa_family == o2->sa_family && (cmp == NULL || (*cmp)(mode, data1, data2, dataSz)));
			case ENCompareMode_GreaterOrEqual:
				return o1->sa_family > o2->sa_family || (o1->sa_family == o2->sa_family && (cmp == NULL || (*cmp)(mode, data1, data2, dataSz)));
#			ifdef NB_CONFIG_INCLUDE_ASSERTS
			default:
				NBASSERT(FALSE)
				break;
#			endif
		}
	}
	NBASSERT(FALSE)
	return FALSE;
}

BOOL NBCompare_NBSocketAddr_addrOnlyINET(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBSocketAddr))
	NBASSERT(sizeof(struct sockaddr_in) <= sizeof(STNBSocketAddr))
	struct sockaddr_in* o1 = (struct sockaddr_in*)data1;
	struct sockaddr_in* o2 = (struct sockaddr_in*)data2;
	NBASSERT(o1->sin_family == AF_INET && o2->sin_family == AF_INET)
	NBASSERT(sizeof(o1->sin_addr) == 4) //32 bits
	if(o1->sin_family == AF_INET && o2->sin_family == AF_INET){
		switch (mode) {
			case ENCompareMode_Equal:
				return *((UI32*)&o1->sin_addr) == *((UI32*)&o2->sin_addr);
			case ENCompareMode_Lower:
				return *((UI32*)&o1->sin_addr) < *((UI32*)&o2->sin_addr);
			case ENCompareMode_LowerOrEqual:
				return *((UI32*)&o1->sin_addr) <= *((UI32*)&o2->sin_addr);
			case ENCompareMode_Greater:
				return *((UI32*)&o1->sin_addr) > *((UI32*)&o2->sin_addr);
			case ENCompareMode_GreaterOrEqual:
				return *((UI32*)&o1->sin_addr) >= *((UI32*)&o2->sin_addr);
#			ifdef NB_CONFIG_INCLUDE_ASSERTS
			default:
				NBASSERT(FALSE)
				break;
#			endif
		}
	}
	NBASSERT(FALSE)
	return FALSE;
}

BOOL NBCompare_NBSocketAddr_addrOnlyINET6(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBSocketAddr))
	NBASSERT(sizeof(struct sockaddr_in6) <= sizeof(STNBSocketAddr))
	struct sockaddr_in6* o1 = (struct sockaddr_in6*)data1;
	struct sockaddr_in6* o2 = (struct sockaddr_in6*)data2;
	NBASSERT(o1->sin6_family == AF_INET6 && o2->sin6_family == AF_INET6)
	NBASSERT(sizeof(o1->sin6_addr) == 16) //128 bits
	if(o1->sin6_family == AF_INET6 && o2->sin6_family == AF_INET6){
		switch (mode) {
			case ENCompareMode_Equal:
				return ((UI64*)&o1->sin6_addr)[0] == ((UI64*)&o2->sin6_addr)[0] && ((UI64*)&o1->sin6_addr)[1] == ((UI64*)&o2->sin6_addr)[1];
			case ENCompareMode_Lower:
				return ((UI64*)&o1->sin6_addr)[0] < ((UI64*)&o2->sin6_addr)[0] || (((UI64*)&o1->sin6_addr)[0] == ((UI64*)&o2->sin6_addr)[0] && ((UI64*)&o1->sin6_addr)[1] < ((UI64*)&o2->sin6_addr)[1]);
			case ENCompareMode_LowerOrEqual:
				return ((UI64*)&o1->sin6_addr)[0] < ((UI64*)&o2->sin6_addr)[0] || (((UI64*)&o1->sin6_addr)[0] == ((UI64*)&o2->sin6_addr)[0] && ((UI64*)&o1->sin6_addr)[1] <= ((UI64*)&o2->sin6_addr)[1]);
			case ENCompareMode_Greater:
				return ((UI64*)&o1->sin6_addr)[0] > ((UI64*)&o2->sin6_addr)[0] || (((UI64*)&o1->sin6_addr)[0] == ((UI64*)&o2->sin6_addr)[0] && ((UI64*)&o1->sin6_addr)[1] > ((UI64*)&o2->sin6_addr)[1]);
			case ENCompareMode_GreaterOrEqual:
				return ((UI64*)&o1->sin6_addr)[0] > ((UI64*)&o2->sin6_addr)[0] || (((UI64*)&o1->sin6_addr)[0] == ((UI64*)&o2->sin6_addr)[0] && ((UI64*)&o1->sin6_addr)[1] >= ((UI64*)&o2->sin6_addr)[1]);
#			ifdef NB_CONFIG_INCLUDE_ASSERTS
			default:
				NBASSERT(FALSE)
				break;
#			endif
		}
	}
	NBASSERT(FALSE)
	return FALSE;
}

UI16 NBSocketAddr_getPort(const STNBSocketAddr* obj){
	UI16 r = 0;
	if(obj != NULL){
		struct sockaddr* addr = (struct sockaddr*)obj;
		switch(addr->sa_family) {
			case AF_INET: r = ((struct sockaddr_in*)addr)->sin_port; break;
			case AF_INET6: r = ((struct sockaddr_in6*)addr)->sin6_port; break;
			default: break;
		}
	}
	return r;
}

UI32 NBSocketAddr_getIPv4(const STNBSocketAddr* obj){
	UI32 r = 0;
	if(obj != NULL){
		struct sockaddr* addr = (struct sockaddr*)obj;
		if(addr->sa_family == AF_INET){
			r = ((const UI32*)&((struct sockaddr_in*)addr)->sin_addr)[0];
		}
	}
	return r;
}

BOOL NBSocketAddr_concatAddrOnly(const STNBSocketAddr* obj, char* dst, const UI32 dstSz){
	BOOL r = FALSE;
	if(obj != NULL){
		struct sockaddr* addr = (struct sockaddr*)obj;
		switch(addr->sa_family) {
			case AF_INET: r = NBSocketAddr_concatIPv4(((UI32*)&((struct sockaddr_in*)addr)->sin_addr)[0], dst, dstSz); break;
			default: break;
		}
	}
	return r;
}

BOOL NBSocketAddr_concatIPv4(const UI32 netOrderIP, char* dst16, const UI32 dstSz){
	BOOL r = FALSE;
	if(dst16 != NULL && dstSz >= 16){
		const UI8* src = (const UI8*)&netOrderIP;
		//
		if(src[0] > 99){
			*(dst16++) = '0' + (src[0] / 100);
			*(dst16++) = '0' + ((src[0] % 100) / 10);
		} else if(src[0] > 9){
			*(dst16++) = '0' + (src[0] / 10);
		}
		*(dst16++) = '0' + (src[0] % 10);
		//
		*(dst16++) = '.';
		//
		if(src[1] > 99){
			*(dst16++) = '0' + (src[1] / 100);
			*(dst16++) = '0' + ((src[1] % 100) / 10);
		} else if(src[1] > 9){
			*(dst16++) = '0' + (src[1] / 10);
		}
		*(dst16++) = '0' + (src[1] % 10);
		//
		*(dst16++) = '.';
		//
		if(src[2] > 99){
			*(dst16++) = '0' + (src[2] / 100);
			*(dst16++) = '0' + ((src[2] % 100) / 10);
		} else if(src[2] > 9){
			*(dst16++) = '0' + (src[2] / 10);
		}
		*(dst16++) = '0' + (src[2] % 10);
		//
		*(dst16++) = '.';
		//
		if(src[3] > 99){
			*(dst16++) = '0' + (src[3] / 100);
			*(dst16++) = '0' + ((src[3] % 100) / 10);
		} else if(src[3] > 9){
			*(dst16++) = '0' + (src[3] / 10);
		}
		*(dst16++) = '0' + (src[3] % 10);
		//
		*(dst16++) = '\0';
		//
		r = TRUE;
	}
	return r;
}

UI32 NBSocketAddr_parseIPv4(const char* ip4){
	UI32 r = 0;
	if(ip4 != NULL){
		UI8 iFnd = 0;
		UI8 octs[4] = { 0, 0, 0, 0 };
		const char* octStart = ip4;
		while(TRUE){
			if(*ip4 >= '0' && *ip4 <= '9'){
				//Stop if more than 4 octects found
				if(iFnd == 4){
					break;
				}
			} else if(*ip4 == '.' || *ip4 == '\0'){
				//consume
				NBASSERT(octStart < ip4)
				if((ip4 - octStart) <= 0 || (ip4 - octStart) > 3){
					//1 to 3 chars for the integer part
					break;
				} else {
					//Parse octect
					UI8 multiplier = 1;
					const char* octChar = ip4;
					do {
						octChar--;
						NBASSERT(*octChar >= '0' && *octChar <= '9')
						octs[iFnd] += (*octChar - '0') * multiplier;
						multiplier *= 10;
					} while(octStart < octChar);
					//Start nex
					iFnd++;
					octStart = ip4 + 1;
				}
				//stop
				if(*ip4 == '\0'){
					break;
				}
			} else {
				//invalid char
				break;
			}
			//next
			ip4++;
		}
		//
		if(iFnd == 4 && *ip4 == '\0'){
			r = *((UI32*)octs);
			/*UI8* dst = (UI8*)&r;
			dst[0] = octs[3];
			dst[1] = octs[2];
			dst[2] = octs[1];
			dst[3] = octs[0];*/
		}
	}
	return r;
}

STNBSocketAddrIP6 NBSocketAddr_getIPv6(const STNBSocketAddr* obj){
	STNBSocketAddrIP6 r;
	NBMemory_setZeroSt(r, STNBSocketAddrIP6);
	if(obj != NULL){
		struct sockaddr* addr = (struct sockaddr*)obj;
		if(addr->sa_family == AF_INET6){
			r.v64[0] = ((const UI64*)&((struct sockaddr_in6*)addr)->sin6_addr)[0];
			r.v64[1] = ((const UI64*)&((struct sockaddr_in6*)addr)->sin6_addr)[1];
		}
	}
	return r;
}

BOOL NBSocketAddr_isSamePort(const STNBSocketAddr* obj, const STNBSocketAddr* other){
	BOOL r = FALSE;
	if(obj == other){
		r = TRUE;
	} else if(obj != NULL && other != NULL){
		r = (NBSocketAddr_getPort(obj) == NBSocketAddr_getPort(other));
	}
	return r;
}

BOOL NBSocketAddr_isSameIP(const STNBSocketAddr* obj, const STNBSocketAddr* other){
	BOOL r = FALSE;
	if(obj == other){
		r = TRUE;
	} else if(obj != NULL && other != NULL){
		struct sockaddr* addr0 = (struct sockaddr*)obj;
		struct sockaddr* addr1 = (struct sockaddr*)other;
		NBASSERT(addr0->sa_family >= 0 && addr0->sa_family < AF_MAX)
		if(addr0->sa_family >= 0 && addr0->sa_family < AF_MAX && addr0->sa_family == addr1->sa_family){
			NBCompareFunc cmp = NBCompare_NBSocketAddr_addrOnlyByFamily[addr0->sa_family];
			NBASSERT(cmp != NULL)
			if(cmp != NULL){
				r = (*cmp)(ENCompareMode_Equal, obj, other, sizeof(*other));
			}
		}
	}
	return r;
}


const char* NBSocket_getErrorName(const int errCode){
#   ifdef _WIN32
    switch(errCode){
        case WSANOTINITIALISED: return "WSANOTINITIALISED";
        case WSAENETDOWN: return "WSAENETDOWN";
        case WSAEFAULT: return "WSAEFAULT";
        case WSAENOTCONN: return "WSAENOTCONN";
        case WSAEINTR: return "WSAEINTR";
        case WSAEINPROGRESS: return "WSAEINPROGRESS";
        case WSAENETRESET: return "WSAENETRESET";
        case WSAENOTSOCK: return "WSAENOTSOCK";
        case WSAEOPNOTSUPP: return "WSAEOPNOTSUPP";
        case WSAESHUTDOWN: return "WSAESHUTDOWN";
        case WSAEWOULDBLOCK: return "WSAEWOULDBLOCK";
        case WSAEMSGSIZE: return "WSAEMSGSIZE";
        case WSAEINVAL: return "WSAEINVAL";
        case WSAECONNABORTED: return "WSAECONNABORTED";
        case WSAETIMEDOUT: return "WSAETIMEDOUT";
        case WSAECONNRESET: return "WSAECONNRESET";
        default: break;
    }
#   else
    return strerror(errCode);
#   endif
    return "ERR_NAME_UNKNOWN";
}
