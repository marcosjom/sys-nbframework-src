
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
#include "nb/core/NBIOPollstersProvider.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBThreadCond.h"

//NBIOPollstersProvider

typedef struct STNBIOPollstersProviderOpq_ {
	STNBObject			prnt;
	//provider
	struct {
		STNBIOPollstersProviderItf	itf;
		void*						usrData;
	} provider;
} STNBIOPollstersProviderOpq;

NB_OBJREF_BODY(NBIOPollstersProvider, STNBIOPollstersProviderOpq, NBObject)

void NBIOPollstersProvider_initZeroed(STNBObject* obj) {
	//STNBIOPollstersProviderOpq* opq = (STNBIOPollstersProviderOpq*)obj;
}

void NBIOPollstersProvider_uninitLocked(STNBObject* obj){
	//STNBIOPollstersProviderOpq* opq = (STNBIOPollstersProviderOpq*)obj;
}

//cfg

BOOL NBIOPollstersProvider_setItf(STNBIOPollstersProviderRef ref, const STNBIOPollstersProviderItf* itf, void* usrData){
	BOOL r = FALSE;
	STNBIOPollstersProviderOpq* opq = (STNBIOPollstersProviderOpq*)ref.opaque;
	NBObject_lock(opq);
	{
		if(itf == NULL){
			NBMemory_setZeroSt(opq->provider.itf, STNBIOPollstersProviderItf);
		} else {
			opq->provider.itf = *itf;
		}
		opq->provider.usrData = usrData;
		r = TRUE;
	}
	NBObject_unlock(opq);
	return r;
}

//pollsters

STNBIOPollsterRef NBIOPollstersProvider_getPollster(STNBIOPollstersProviderRef ref){
	STNBIOPollsterRef r = NB_OBJREF_NULL;
	STNBIOPollstersProviderOpq* opq = (STNBIOPollstersProviderOpq*)ref.opaque;
	NBObject_lock(opq);
	{
		if(opq->provider.itf.getPollster != NULL){
			r = (*opq->provider.itf.getPollster)(opq->provider.usrData);
		}
	}
	NBObject_unlock(opq);
	return r;
}

STNBIOPollsterSyncRef NBIOPollstersProvider_getPollsterSync(STNBIOPollstersProviderRef ref){
	STNBIOPollsterSyncRef r = NB_OBJREF_NULL;
	STNBIOPollstersProviderOpq* opq = (STNBIOPollstersProviderOpq*)ref.opaque;
	NBObject_lock(opq);
	{
		if(opq->provider.itf.getPollsterSync != NULL){
			r = (*opq->provider.itf.getPollsterSync)(opq->provider.usrData);
		}
	}
	NBObject_unlock(opq);
	return r;
}
