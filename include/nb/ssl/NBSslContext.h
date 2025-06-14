#ifndef NB_SSL_CONTEXT_H
#define NB_SSL_CONTEXT_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/crypto/NBX509.h"
#include "nb/crypto/NBPKey.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef enum ENNBSslContextMode_ {
		ENNBSslContextMode_none = 0,
		ENNBSslContextMode_client,
		ENNBSslContextMode_server,
		ENNBSslContextMode_both
	} ENNBSslContextMode;
	
	typedef void* (NBSsslContextModeFunc)(ENNBSslContextMode* dstMode);
	
	NB_OBJREF_HEADER(NBSslContext)

    //Error
    void    NBSslContext_concatLastErrStack(STNBString* dst);
    void    NBSslContext_printLastErrStack(const char* lastMethodCalled);
	
	//Create context
	void*	NBSslContext_getClientMode(ENNBSslContextMode* dstMode);
	void*	NBSslContext_getServerMode(ENNBSslContextMode* dstMode);
	void*	NBSslContext_getBothMode(ENNBSslContextMode* dstMode);
	BOOL	NBSslContext_isCreated(STNBSslContextRef ref);
	BOOL	NBSslContext_create(STNBSslContextRef ref, NBSsslContextModeFunc getModeFunc);
	//BOOL	NBSslContext_createAsRefTo(STNBSslContextRef ref, STNBSslContextRef other); //ToDo: remove
	
    //Security level
    //https://www.openssl.org/docs/man1.1.1/man3/SSL_set_security_level.html
    BOOL    NBSslContext_setSecurityLevel(STNBSslContextRef ref, int lvl);

	//Attach certificate and key to context
	BOOL	NBSslContext_attachCertAndkey(STNBSslContextRef ref, const STNBX509* cert, const STNBPKey* pkey);
	BOOL	NBSslContext_setVerifyPeerCert(STNBSslContextRef ref, const BOOL certIsRequested, const BOOL certIsRequired);
	BOOL	NBSslContext_addCAToRequestList(STNBSslContextRef ref, const STNBX509* cert);
	BOOL	NBSslContext_addCAToStore(STNBSslContextRef ref, const STNBX509* cert);
	//Handle
	void*	NBSslContext_createHandle(STNBSslContextRef ref);
	void	NBSslContext_destroyHandle(STNBSslContextRef ref, void* sslHandle);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
