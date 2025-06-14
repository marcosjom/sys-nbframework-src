#ifndef NBIOPollstersProvider_h
#define NBIOPollstersProvider_h

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBIOPollster.h"

#ifdef __cplusplus
extern "C" {
#endif

//NBIOPollstersProviderItf

typedef struct STNBIOPollstersProviderItf_ {
	STNBIOPollsterRef		(*getPollster)(void* usrData); //returns a pollster to use
	STNBIOPollsterSyncRef	(*getPollsterSync)(void* usrData); //returns a pollster to use
} STNBIOPollstersProviderItf;

//NBIOPollstersProvider

NB_OBJREF_HEADER(NBIOPollstersProvider)

//cfg
BOOL NBIOPollstersProvider_setItf(STNBIOPollstersProviderRef ref, const STNBIOPollstersProviderItf* itf, void* usrData);

//pollsters
STNBIOPollsterRef		NBIOPollstersProvider_getPollster(STNBIOPollstersProviderRef ref);
STNBIOPollsterSyncRef	NBIOPollstersProvider_getPollsterSync(STNBIOPollstersProviderRef ref);

#ifdef __cplusplus
} //extern "C"
#endif


#endif
