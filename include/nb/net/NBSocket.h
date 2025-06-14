#ifndef NB_SOCKET_H
#define NB_SOCKET_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBString.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBDataPtr.h"
#include "nb/core/NBIOLnk.h"

#define NB_SOCKET_BIT_DONT_WAIT	(0x1)		//non-blocking
#define NB_SOCKET_BIT_PEEK		(0x1 << 1)	//keep content in buffer
							
#ifdef __cplusplus
extern "C" {
#endif
	
	//-----------------
	//- ENNBSocketResult
	//-----------------

	typedef enum ENNBSocketResult_ {
		ENNBSocketResult_Error = 0,		//error
		ENNBSocketResult_Success,		//success
		ENNBSocketResult_WouldBlock,	//non-blocking socket, try again latter
		//
		ENNBSocketResult_Count
	} ENNBSocketResult;

	//--------------
	//- NBSocketAddr
	//--------------

	#define NB_SOCKET_ADDR_STR_MAX_SZ	16	//size for a string buffer that can store any of the supported addresses in string representation

	typedef struct STNBSocketAddr_ {
		BYTE		opaque[32];	//enough for sizeof(struct sockaddr_in6) = ~24bytes
	} STNBSocketAddr;

	typedef struct STNBSocketAddrIP6_ {
		union {
			UI8		v8[16];	//128 bits
			UI16	v16[8];	//128 bits
			UI32	v32[4];	//128 bits
			UI64	v64[2];	//128 bits
		};
	} STNBSocketAddrIP6;

	BOOL NBCompare_NBSocketAddr_addrOnly(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);
	extern NBCompareFunc NBCompare_NBSocketAddr_addrOnlyByFamily[];
	extern NBCompareFunc NBCompare_NBSocketAddr_addrAndPortByFamily[];
		
	UI16	NBSocketAddr_getPort(const STNBSocketAddr* obj);
	UI32	NBSocketAddr_getIPv4(const STNBSocketAddr* obj);
	BOOL	NBSocketAddr_concatAddrOnly(const STNBSocketAddr* obj, char* dst, const UI32 dstSz);
	BOOL	NBSocketAddr_concatIPv4(const UI32 netOrderIP, char* dst16, const UI32 dstSz);
	UI32	NBSocketAddr_parseIPv4(const char* ip4);
	STNBSocketAddrIP6	NBSocketAddr_getIPv6(const STNBSocketAddr* obj);
	BOOL	NBSocketAddr_isSamePort(const STNBSocketAddr* obj, const STNBSocketAddr* other);
	BOOL	NBSocketAddr_isSameIP(const STNBSocketAddr* obj, const STNBSocketAddr* other);

	//--------------------------
	// OS specific optimization:
	// for reading/writting multiple
	// messages inone system call.
	//--------------------------

	typedef enum ENNBSocketPacketsBuffsType_ {
		ENNBSocketPacketsBuffsType_Internal = 0,	//allocated by the obj
		ENNBSocketPacketsBuffsType_External,		//allocated by the user and synced with "NBSocketPackets_syncInternalRng" 
		ENNBSocketPacketsBuffsType_Count
	} ENNBSocketPacketsBuffsType;

	typedef struct STNBSocketPackets_ {
		STNBSocketAddr*				addrs;		//packet addresses
		STNBDataPtr*				buffs;		//packet buffers
		UI32						use;		//packets populated
		UI32						size;		//size of headers
		void*						opaque;		//opaque data (os specific)
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		struct {
			UI32					bytesPerBuff;
		} dbg;
#		endif
	} STNBSocketPackets;
	//
	void	NBSocketPackets_init(STNBSocketPackets* obj);
	void	NBSocketPackets_release(STNBSocketPackets* obj);
	void	NBSocketPackets_deallocBuffers(STNBSocketPackets* obj);
	void	NBSocketPackets_syncInternalRng(STNBSocketPackets* obj, const UI32 iFirst, const UI32 count);
	void	NBSocketPackets_empty(STNBSocketPackets* obj);
	void	NBSocketPackets_allocBuffersSameSizes(STNBSocketPackets* obj, const UI32 buffsAmm, const UI32 bytesPerBuff);
	void	NBSocketPackets_useExternalBuffers(STNBSocketPackets* obj, STNBDataPtr* ptrs, const UI32 ptrsSz);

	//-----------------
	//- ENNBSocketHndType
	//-----------------

	typedef enum ENNBSocketHndType_ {
		ENNBSocketHndType_TCP = 0,
		ENNBSocketHndType_UDP,
		//
		ENNBSocketHndType_Count
	} ENNBSocketHndType;

	//----------
	//- NBSocket
	//----------
	
	NB_OBJREF_HEADER(NBSocket)

	void			NBSocket_initEngine(void);
	void			NBSocket_releaseEngine(void);
	//
	UI32			NBSocket_initWSA(void);
	UI32			NBSocket_finishWSA(void);
	//
	void			NBSocket_setUnsafeMode(STNBSocketRef ref, const BOOL unsafeEnabled);	//when enabled, internal locks are disabled for efficiency but non-parallels calls must be ensured by user
	ENNBSocketHndType NBSocket_getType(STNBSocketRef ref);
	BOOL			NBSocket_setType(STNBSocketRef ref, const ENNBSocketHndType type);
	BOOL			NBSocket_createHnd(STNBSocketRef ref);
	//Configure
	BOOL			NBSocket_setNonBlocking(STNBSocketRef ref, const BOOL nonBlocking);	//non-blocking mode
	BOOL			NBSocket_setNoSIGPIPE(STNBSocketRef ref, const BOOL noSIGPIPE);		//Recommended to avoid SIGPIPE signals
	BOOL			NBSocket_setReuseAddr(STNBSocketRef ref, const BOOL reuse);			//Recommended before bind
	BOOL			NBSocket_setReusePort(STNBSocketRef ref, const BOOL reuse);			//Recommended before bind
	BOOL			NBSocket_setDelayEnabled(STNBSocketRef ref, const BOOL delayEnabled);	//If TRUE, the socket will send data inmediatly (warning:avoid sending data in small sizes; use your own buffer).
	BOOL			NBSocket_setCorkEnabled(STNBSocketRef ref, const BOOL corkEnabled);	//If FALSE, the socket will send data inmediatly (warning:avoid sending data in small sizes; use your own buffer).
	//Server
	BOOL			NBSocket_bind(STNBSocketRef ref, int port);
	BOOL			NBSocket_listen(STNBSocketRef ref);
	BOOL			NBSocket_accept(STNBSocketRef ref, STNBSocketRef dst);
	//Stream
	ENNBSocketResult NBSocket_connect(STNBSocketRef ref, const char* server, const int port);
	SI32			NBSocket_recv(STNBSocketRef ref, void* buff, int buffSz);
	SI32			NBSocket_recvFrom(STNBSocketRef ref, void* buff, int buffSz, STNBSocketAddr* addr);
	SI32			NBSocket_recvMany(STNBSocketRef ref, STNBSocketPackets* pckts);
	SI32			NBSocket_send(STNBSocketRef ref, const void* data, int dataSz);
	BOOL			NBSocket_isShutedForRcv(STNBSocketRef ref);
	BOOL			NBSocket_isShutedForSend(STNBSocketRef ref);
	void			NBSocket_shutdown(STNBSocketRef ref, const UI8 mask); //NB_IO_BIT_READ | NB_IO_BIT_WRITE
	void			NBSocket_close(STNBSocketRef ref);
	BOOL			NBSocket_isClosed(STNBSocketRef ref);
    //Lnk
    BOOL            NBSocket_getIOLnk(STNBSocketRef ref, STNBIOLnk* dst);
	//Address
	BOOL			NBSocket_getAddressLocal(STNBSocketRef ref, STNBSocketAddr* dst);	
	BOOL			NBSocket_getAddressRemote(STNBSocketRef ref, STNBSocketAddr* dst);
	
	//Use less as posible
	int				NBSocket_getFD_(STNBSocketRef ref);
	
#ifdef __cplusplus
} //extern "C"
#endif


#endif
