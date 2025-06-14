#ifndef NB_HTTP_CFG_H
#define NB_HTTP_CFG_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBStructMap.h"
#include "nb/net/NBHttpReqRule.h"
#include "nb/crypto/NBX509Req.h"

#ifdef __cplusplus
extern "C" {
#endif

//Http traffic cfg

typedef struct STNBHttpTrafficCfg_ {
	UI64	in;
	UI64	out;
} STNBHttpTrafficCfg;

const STNBStructMap* NBHttpTrafficCfg_getSharedStructMap(void);

//Http limit conns cfg

typedef struct STNBHttpLimitsCfg_ {
    UI32                conns;    //stablished conns
    STNBHttpTrafficCfg  bps;      //traffic
} STNBHttpLimitsCfg;

const STNBStructMap* NBHttpLimitsCfg_getSharedStructMap(void);

//Http conn limits cfg

typedef struct STNBHttpConnLimitsCfg_ {
    UI64                secsIdle;           //close conn after secs not read or writting
    UI64                secsOverqueueing;   //close conn after total balance of secs where more data is queued by the processing code than sent.
    STNBHttpTrafficCfg  bps;                //traffic limits
} STNBHttpConnLimitsCfg;

const STNBStructMap* NBHttpConnLimitsCfg_getSharedStructMap(void);

//Http conn cfg

typedef struct STNBHttpConnRecvCfg_ {
    UI32                    buffSz;         //0 = use-default, size of buffer for each recv-call (two buffers are allocated)
    UI32                    bodyChunkSz;    //0 = use-default, size of body chunks allocations (allocated while receiving)
} STNBHttpConnRecvCfg;

const STNBStructMap* NBHttpConnRecvCfg_getSharedStructMap(void);

typedef struct STNBHttpConnSendQueueCfg_ {
    UI32                    initialSz;  //0 = use-default, initial size-in-bytes of queue buffer
    UI32                    growthSz;   //0 = use-default, minimal growth-in-bytes of queue buffer
} STNBHttpConnSendQueueCfg;

const STNBStructMap* NBHttpConnSendQueueCfg_getSharedStructMap(void);

//Http conn cfg

typedef struct STNBHttpConnCfg_ {
    STNBHttpConnRecvCfg         recv;
    STNBHttpConnSendQueueCfg    sendQueue;
    STNBHttpConnLimitsCfg       limits;
} STNBHttpConnCfg;

const STNBStructMap* NBHttpConnCfg_getSharedStructMap(void);

//Http ssl-cert-src-key cfg

typedef struct STNBHttpCertSrcCfgKeyCfg_ {
	char*		pass;	//p12 password
	char*		path;	//p12 file (my-private-key + ca-certificate + my-certificate)
	char*		name;	//friendly name at p12
} STNBHttpCertSrcCfgKeyCfg;

const STNBStructMap* NBHttpCertSrcCfgKeyCfg_getSharedStructMap(void);

//Http ssl-cert-src cfg

typedef struct STNBHttpCertSrcCfg_ {
	char*					    path;	//other's certificate
	STNBX509Def				    def;
	STNBHttpCertSrcCfgKeyCfg    key;
} STNBHttpCertSrcCfg;

const STNBStructMap* NBHttpCertSrcCfg_getSharedStructMap(void);

//Http ssl-cert cfg

typedef struct STNBHttpSslCertCfg_ {
	BOOL					isRequested;
	BOOL					isRequired;
	STNBHttpCertSrcCfg		source;
} STNBHttpSslCertCfg;

const STNBStructMap* NBHttpSslCertCfg_getSharedStructMap(void);

//Http ssl cfg

typedef struct STNBHttpSslCfg_ {
	BOOL				isDisabled;
	STNBHttpSslCertCfg	cert;
} STNBHttpSslCfg;

const STNBStructMap* NBHttpSslCfg_getSharedStructMap(void);
	
//http port cfg

typedef struct STNBHttpPortCfg_ {
	BOOL					isDisabled;
	char*					server;
	UI32					port;
    STNBHttpLimitsCfg       conns;
    STNBHttpConnCfg         perConn;
	STNBHttpSslCfg			ssl;
    STNBHttpReqRulesDef*    reqsRules;   //dummy struct to warn about old design, and to move this code to user's side.
} STNBHttpPortCfg;

const STNBStructMap* NBHttpPortCfg_getSharedStructMap(void);

//http service cfg

typedef struct STNBHttpServiceCfg_ {
    STNBHttpLimitsCfg		conns;
	STNBHttpPortCfg*		ports;
	UI32					portsSz;
} STNBHttpServiceCfg;

const STNBStructMap* NBHttpServiceCfg_getSharedStructMap(void);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
