#ifndef NB_SSL_HND_H
#define NB_SSL_HND_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBString.h"
#include "nb/net/NBSocket.h"
#include "nb/crypto/NBX509.h"
#include "nb/ssl/NBSslContext.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	NB_OBJREF_HEADER(NBSsl)

    //-----------------
    //- ENNBSslResult
    //-----------------

    typedef enum ENNBSslResult_ {
        ENNBSslResult_ErrWantWrite   = -2, //non-blocking action, try again latter
        ENNBSslResult_ErrWantRead    = -1, //non-blocking action, try again latter
        ENNBSslResult_Error          = 0,  //error
        ENNBSslResult_Success        = 1,  //success
        //
        ENNBSslResult_Count
    } ENNBSslResult;
	
    //

    BOOL NBSsl_createWithSocket(STNBSslRef ref, STNBSslContextRef ctxt, STNBSocketRef socket); //Incoming connection
    BOOL NBSsl_createWithIOLnk(STNBSslRef ref, STNBSslContextRef ctxt, STNBIOLnk* ioLnk); //Incoming connection
    BOOL NBSsl_createWithIOLnkItf(STNBSslRef ref, STNBSslContextRef ctxt, STNBIOLnkItf* ioLnk, void* ioLnkUsrData); //Incoming connection

    //

    ENNBSslResult NBSsl_accept(STNBSslRef ref); //Incoming connection
    ENNBSslResult NBSsl_connectHandshake(STNBSslRef ref); //Outgoing connection
	BOOL NBSsl_connect(STNBSslRef ref, STNBSslContextRef ctxt, STNBSocketRef socket); //Outgoing connection
    BOOL NBSsl_hasPendingRead(STNBSslRef ref); //internal buffer containes posible 'read' data (note: sometimes a read can return zero data)
	SI32 NBSsl_read(STNBSslRef ref, void* buff, int buffSz); //bytes-recvd or ENNBSslResult value
	SI32 NBSsl_write(STNBSslRef ref, const void* data, int dataSz); //bytes-sent or ENNBSslResult value
	void NBSsl_shutdown(STNBSslRef ref);
    
    //Error
    void NBSsl_concatLastErrStack(STNBString* dst);
    void NBSsl_printLastErrStack(const char* lastMethodCalled);

	//

	BOOL NBSsl_getPeerCertificate(STNBSslRef ref, STNBX509* dst);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
