#ifndef NB_SOCKET_POLLSTER_H
#define NB_SOCKET_POLLSTER_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/net/NBSocket.h"

#ifdef __cplusplus
extern "C" {
#endif

//

NB_OBJREF_HEADER(NBIOPollsterSync)
NB_OBJREF_HEADER(NBIOPollster)

//ENNBIOPollsterOpBit

typedef enum ENNBIOPollsterOpBit_ {
	//base
	ENNBIOPollsterOpBit_None			= 0
	, ENNBIOPollsterOpBit_Read			= (0x1 << 0)	//Read OP
	, ENNBIOPollsterOpBit_Write			= (0x1 << 1)	//Write OP
	, ENNBIOPollsterOpBit_Error			= (0x1 << 2)	//Error-state
    , ENNBIOPollsterOpBit_HUP           = (0x1 << 3)    //Closed by remote-end
    , ENNBIOPollsterOpBit_NVAL          = (0x1 << 4)    //Invalid descriptor
	//combinations
    , ENNBIOPollsterOpBits_ReadWrite    = (ENNBIOPollsterOpBit_Read | ENNBIOPollsterOpBit_Write)
    , ENNBIOPollsterOpBits_ErrOrGone    = (ENNBIOPollsterOpBit_Error | ENNBIOPollsterOpBit_HUP | ENNBIOPollsterOpBit_NVAL)
	, ENNBIOPollsterOpBits_All			= (ENNBIOPollsterOpBit_Read | ENNBIOPollsterOpBit_Write | ENNBIOPollsterOpBit_Error | ENNBIOPollsterOpBit_HUP | ENNBIOPollsterOpBit_NVAL)
} ENNBIOPollsterOpBit;

//ENNBIOPollsterPollMode

typedef enum ENNBIOPollsterPollMode_ {
	ENNBIOPollsterPollMode_KeepFoundOps = 0	//ops-flags are kept after poll ready for poll again 
	, ENNBIOPollsterPollMode_DisableFoundOps	//found ops-flags are disabled after poll and must be manually re-enabled for next poll
	//Count
	, ENNBIOPollsterPollMode_Count
} ENNBIOPollsterPollMode;

//NBIOPollsterLstrnItf

typedef struct STNBIOPollsterUpd_ {
	UI8		opsMasks;	//ops mask to apply
	BOOL	syncFd;		//update fd
} STNBIOPollsterUpd;

typedef struct STNBIOPollsterLstrnItf_ {
	void	(*pollConsumeMask)(STNBIOLnk ioLnk, const UI8 pollMask, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData); //consume operations and return the ones that are required to poll.
	void	(*pollConsumeNoOp)(STNBIOLnk ioLnk, STNBIOPollsterUpd* dstUpd, STNBIOPollsterSyncRef dstSync, void* usrData); //consume non-ops and return the ones that are required to poll.
    void    (*pollGetReqUpd)(STNBIOLnk ioLnk, STNBIOPollsterUpd* dstUpd, void* usrData);    //optional, if the client expects to change the requested 'opsMasks' outside the 'pollConsumeMask' or 'pollConsumeNoOp' calls.
	void	(*pollRemoved)(STNBIOLnk ioLnk, void* usrData);
} STNBIOPollsterLstrnItf;

//NBIOPollsterRecord

typedef struct STNBIOPollsterRecord_ {
    STNBIOLnk           link;
	//opsMasks
	struct {
		UI8				req;		//ENNBIOPollsterOpBit*, requested-ops mask
		UI8				rslt;		//ENNBIOPollsterOpBit*, result-ops mask
		UI8				kept;		//ENNBIOPollsterOpBit*, kept-ops mask for next call
	} opsMasks;
	STNBIOPollsterLstrnItf itf;
	void*				usrData;	//userData
} STNBIOPollsterRecord;

//NBIOPollsterResult

typedef struct STNBIOPollsterResult_ {
	STNBIOPollsterRecord*	records;
	UI32					recordsUse;
	UI32					recordsSz;
} STNBIOPollsterResult;

void NBIOPollsterResult_init(STNBIOPollsterResult* obj);
void NBIOPollsterResult_release(STNBIOPollsterResult* obj);
void NBIOPollsterResult_resize(STNBIOPollsterResult* obj, const UI32 sz);

//NBIOPollsterSync

BOOL NBIOPollsterSync_isEmpty(STNBIOPollsterSyncRef ref);

//ioLnk
BOOL NBIOPollsterSync_addIOLnk(STNBIOPollsterSyncRef ref, const STNBIOLnk* ioLnk, const ENNBIOPollsterOpBit opsMask, const ENNBIOPollsterPollMode pollMode, void* usrData);
BOOL NBIOPollsterSync_addIOLnkWithItf(STNBIOPollsterSyncRef ref, const STNBIOLnk* ioLnk, const ENNBIOPollsterOpBit opsMask, const STNBIOPollsterLstrnItf* itf, void* usrData);
BOOL NBIOPollsterSync_enableIOLnkOps(STNBIOPollsterSyncRef ref, const STNBIOLnk* ioLnk, const ENNBIOPollsterOpBit opsMask);
BOOL NBIOPollsterSync_disableIOLnkOps(STNBIOPollsterSyncRef ref, const STNBIOLnk* ioLnk, const ENNBIOPollsterOpBit opsMask);
BOOL NBIOPollsterSync_updateIOLnkFD(STNBIOPollsterSyncRef ref, const STNBIOLnk* ioLnk);
BOOL NBIOPollsterSync_removeIOLnk(STNBIOPollsterSyncRef ref, const STNBIOLnk* ioLnk);

//sockets
BOOL NBIOPollsterSync_addSocket(STNBIOPollsterSyncRef ref, STNBSocketRef socket, const ENNBIOPollsterOpBit opsMask, const ENNBIOPollsterPollMode pollMode, void* usrData);
BOOL NBIOPollsterSync_addSocketWithItf(STNBIOPollsterSyncRef ref, STNBSocketRef socket, const ENNBIOPollsterOpBit opsMask, const STNBIOPollsterLstrnItf* itf, void* usrData);
BOOL NBIOPollsterSync_enableSocketOps(STNBIOPollsterSyncRef ref, STNBSocketRef socket, const ENNBIOPollsterOpBit opsMask);
BOOL NBIOPollsterSync_disableSocketOps(STNBIOPollsterSyncRef ref, STNBSocketRef socket, const ENNBIOPollsterOpBit opsMask);
BOOL NBIOPollsterSync_updateSocketFD(STNBIOPollsterSyncRef ref, STNBSocketRef socket);
BOOL NBIOPollsterSync_removeSocket(STNBIOPollsterSyncRef ref, STNBSocketRef socket);

//files
BOOL NBIOPollsterSync_addFile(STNBIOPollsterSyncRef ref, STNBFileRef file, const ENNBIOPollsterOpBit opsMask, const ENNBIOPollsterPollMode pollMode, void* usrData);
BOOL NBIOPollsterSync_addFileWithItf(STNBIOPollsterSyncRef ref, STNBFileRef file, const ENNBIOPollsterOpBit opsMask, const STNBIOPollsterLstrnItf* itf, void* usrData);
BOOL NBIOPollsterSync_enableFileOps(STNBIOPollsterSyncRef ref, STNBFileRef file, const ENNBIOPollsterOpBit opsMask);
BOOL NBIOPollsterSync_disableFileOps(STNBIOPollsterSyncRef ref, STNBFileRef file, const ENNBIOPollsterOpBit opsMask);
BOOL NBIOPollsterSync_updateFileFD(STNBIOPollsterSyncRef ref, STNBFileRef file);
BOOL NBIOPollsterSync_removeFile(STNBIOPollsterSyncRef ref, STNBFileRef file);

BOOL NBIOPollsterSync_sendTo(STNBIOPollsterSyncRef ref, STNBIOPollsterSyncRef other);

//NBIOPollster

//NBIOPollsterLstnrItf

typedef struct STNBIOPollsterLstnrItf_ {
	void	(*pollReturned)(STNBIOPollsterRef ref, const SI32 amm, void* usrData);	//called after poll and before notifying individual objects
} STNBIOPollsterLstnrItf;

//cfg
void NBIOPollster_setUnsafeMode(STNBIOPollsterRef ref, const BOOL unsafeEnabled);	//when enabled, internal locks are disabled for efficiency but non-parallels calls must be ensured by user
void NBIOPollster_setLstnr(STNBIOPollsterRef ref, STNBIOPollsterLstnrItf* itf, void* usrData);

//ioLnk
BOOL NBIOPollster_addIOLnk(STNBIOPollsterRef ref, const STNBIOLnk* ioLnk, const ENNBIOPollsterOpBit opsMask, const ENNBIOPollsterPollMode pollMode, void* usrData);
BOOL NBIOPollster_addIOLnkWithItf(STNBIOPollsterRef ref, const STNBIOLnk* ioLnk, const ENNBIOPollsterOpBit opsMask, const STNBIOPollsterLstrnItf* itf, void* usrData);
BOOL NBIOPollster_enableIOLnkOps(STNBIOPollsterRef ref, const STNBIOLnk* ioLnk, const ENNBIOPollsterOpBit opsMask);
BOOL NBIOPollster_disableIOLnkOps(STNBIOPollsterRef ref, const STNBIOLnk* ioLnk, const ENNBIOPollsterOpBit opsMask);
BOOL NBIOPollster_updateIOLnkFD(STNBIOPollsterRef ref, const STNBIOLnk* ioLnk);
BOOL NBIOPollster_removeIOLnk(STNBIOPollsterRef ref, const STNBIOLnk* ioLnk);

//sockets
BOOL NBIOPollster_addSocket(STNBIOPollsterRef ref, STNBSocketRef socket, const ENNBIOPollsterOpBit opsMask, const ENNBIOPollsterPollMode pollMode, void* usrData);
BOOL NBIOPollster_addSocketWithItf(STNBIOPollsterRef ref, STNBSocketRef socket, const ENNBIOPollsterOpBit opsMask, const STNBIOPollsterLstrnItf* itf, void* usrData);
BOOL NBIOPollster_removeSocket(STNBIOPollsterRef ref, STNBSocketRef socket);
BOOL NBIOPollster_enableSocketOps(STNBIOPollsterRef ref, STNBSocketRef socket, const ENNBIOPollsterOpBit opsMask);
BOOL NBIOPollster_disableSocketOps(STNBIOPollsterRef ref, STNBSocketRef socket, const ENNBIOPollsterOpBit opsMask);
BOOL NBIOPollster_updateSocketFD(STNBIOPollsterRef ref, STNBSocketRef socket);

//poll (without engine)
UI8  NBIOPollster_socketPoll(STNBSocketRef socket, const ENNBIOPollsterOpBit opsMask, const SI32 msTimeout);
UI8  NBIOPollster_filePoll(STNBFileRef file, const ENNBIOPollsterOpBit opsMask, const SI32 msTimeout);
UI32 NBIOPollster_pollTo(STNBIOPollsterRef ref, const SI32 msTimeout, STNBIOPollsterSyncRef syncTasks, STNBIOPollsterResult* dst);
BOOL NBIOPollster_pollAnyReady(STNBIOPollsterRef ref, const SI32 msTimeout, STNBIOPollsterSyncRef syncTasks);

//engine (does internal notifications)
BOOL NBIOPollster_engineStart(STNBIOPollsterRef ref);
UI32 NBIOPollster_enginePoll(STNBIOPollsterRef ref, const SI32 msTimeout, STNBIOPollsterSyncRef syncTasks);
BOOL NBIOPollster_engineStop(STNBIOPollsterRef ref);

#ifdef __cplusplus
} //extern "C"
#endif


#endif
