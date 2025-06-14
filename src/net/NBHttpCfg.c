
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBHttpCfg.h"
//
#include "nb/core/NBStruct.h"
#include "nb/core/NBMngrStructMaps.h"

//Http traffic cfg

STNBStructMapsRec NBHttpTrafficCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBHttpTrafficCfg_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBHttpTrafficCfg_sharedStructMap);
	if(NBHttpTrafficCfg_sharedStructMap.map == NULL){
		STNBHttpTrafficCfg s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBHttpTrafficCfg);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addUIntM(map, s, in);
		NBStructMap_addUIntM(map, s, out);
		NBHttpTrafficCfg_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBHttpTrafficCfg_sharedStructMap);
	return NBHttpTrafficCfg_sharedStructMap.map;
}


//Http limit conns cfg

STNBStructMapsRec NBHttpLimitsCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBHttpLimitsCfg_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBHttpLimitsCfg_sharedStructMap);
    if(NBHttpLimitsCfg_sharedStructMap.map == NULL){
        STNBHttpLimitsCfg s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBHttpLimitsCfg);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, conns);
        NBStructMap_addStructM(map, s, bps, NBHttpTrafficCfg_getSharedStructMap());
        NBHttpLimitsCfg_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBHttpLimitsCfg_sharedStructMap);
    return NBHttpLimitsCfg_sharedStructMap.map;
}

//Http limits cfg

STNBStructMapsRec NBHttpConnLimitsCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBHttpConnLimitsCfg_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBHttpConnLimitsCfg_sharedStructMap);
    if(NBHttpConnLimitsCfg_sharedStructMap.map == NULL){
        STNBHttpConnLimitsCfg s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBHttpConnLimitsCfg);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, secsIdle);             //close conn after secs not read or writting
        NBStructMap_addUIntM(map, s, secsOverqueueing);     //close conn after total balance of secs where more data is queued by the processing code than sent.
        NBStructMap_addStructM(map, s, bps, NBHttpTrafficCfg_getSharedStructMap());
        NBHttpConnLimitsCfg_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBHttpConnLimitsCfg_sharedStructMap);
    return NBHttpConnLimitsCfg_sharedStructMap.map;
}

//Http limits cfg

STNBStructMapsRec NBHttpConnRecvCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBHttpConnRecvCfg_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBHttpConnRecvCfg_sharedStructMap);
    if(NBHttpConnRecvCfg_sharedStructMap.map == NULL){
        STNBHttpConnRecvCfg s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBHttpConnRecvCfg);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, buffSz); //0 = use-default, size of buffer for each recv-call (two buffers are allocated)
        NBStructMap_addUIntM(map, s, bodyChunkSz); //0 = use-default, size of body chunks allocations (allocated while receiving)
        NBHttpConnRecvCfg_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBHttpConnRecvCfg_sharedStructMap);
    return NBHttpConnRecvCfg_sharedStructMap.map;
}

//Http limits cfg

STNBStructMapsRec NBHttpConnSendQueueCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBHttpConnSendQueueCfg_getSharedStructMap(void){
    NBMngrStructMaps_lock(&NBHttpConnSendQueueCfg_sharedStructMap);
    if(NBHttpConnSendQueueCfg_sharedStructMap.map == NULL){
        STNBHttpConnSendQueueCfg s;
        STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBHttpConnSendQueueCfg);
        NBStructMap_init(map, sizeof(s));
        NBStructMap_addUIntM(map, s, initialSz);   //0 = use-default, initial size-in-bytes of queue buffer
        NBStructMap_addUIntM(map, s, growthSz);    //0 = use-default, minimal growth-in-bytes of queue buffer
        NBHttpConnSendQueueCfg_sharedStructMap.map = map;
    }
    NBMngrStructMaps_unlock(&NBHttpConnSendQueueCfg_sharedStructMap);
    return NBHttpConnSendQueueCfg_sharedStructMap.map;
}

//Http limits cfg

STNBStructMapsRec NBHttpConnCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBHttpConnCfg_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBHttpConnCfg_sharedStructMap);
	if(NBHttpConnCfg_sharedStructMap.map == NULL){
		STNBHttpConnCfg s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBHttpConnCfg);
		NBStructMap_init(map, sizeof(s));
        NBStructMap_addStructM(map, s, recv, NBHttpConnRecvCfg_getSharedStructMap());
        NBStructMap_addStructM(map, s, sendQueue, NBHttpConnSendQueueCfg_getSharedStructMap());
		NBStructMap_addStructM(map, s, limits, NBHttpConnLimitsCfg_getSharedStructMap());
		NBHttpConnCfg_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBHttpConnCfg_sharedStructMap);
	return NBHttpConnCfg_sharedStructMap.map;
}

//Http maxs cfg

STNBStructMapsRec NBHttpPortMaxsCfg_sharedStructMap = STNBStructMapsRec_empty;
//Http ssl-cert-src-key cfg

STNBStructMapsRec NBHttpCertSrcCfgKeyCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBHttpCertSrcCfgKeyCfg_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBHttpCertSrcCfgKeyCfg_sharedStructMap);
	if(NBHttpCertSrcCfgKeyCfg_sharedStructMap.map == NULL){
		STNBHttpCertSrcCfgKeyCfg s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBHttpCertSrcCfgKeyCfg);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addStrPtrM(map, s, pass);
		NBStructMap_addStrPtrM(map, s, path);
		NBStructMap_addStrPtrM(map, s, name);
		NBHttpCertSrcCfgKeyCfg_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBHttpCertSrcCfgKeyCfg_sharedStructMap);
	return NBHttpCertSrcCfgKeyCfg_sharedStructMap.map;
}

//Http ssl-cert-src cfg

STNBStructMapsRec NBHttpCertSrcCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBHttpCertSrcCfg_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBHttpCertSrcCfg_sharedStructMap);
	if(NBHttpCertSrcCfg_sharedStructMap.map == NULL){
		STNBHttpCertSrcCfg s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBHttpCertSrcCfg);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addStrPtrM(map, s, path);
		NBStructMap_addStructM(map, s, def, NBX509Def_getSharedStructMap());
		NBStructMap_addStructM(map, s, key, NBHttpCertSrcCfgKeyCfg_getSharedStructMap());
		NBHttpCertSrcCfg_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBHttpCertSrcCfg_sharedStructMap);
	return NBHttpCertSrcCfg_sharedStructMap.map;
}

//Http ssl-cert cfg

STNBStructMapsRec NBHttpSslCertCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBHttpSslCertCfg_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBHttpSslCertCfg_sharedStructMap);
	if(NBHttpSslCertCfg_sharedStructMap.map == NULL){
		STNBHttpSslCertCfg s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBHttpSslCertCfg);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addBoolM(map, s, isRequested);
		NBStructMap_addBoolM(map, s, isRequired);
		NBStructMap_addStructM(map, s, source, NBHttpCertSrcCfg_getSharedStructMap());
		NBHttpSslCertCfg_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBHttpSslCertCfg_sharedStructMap);
	return NBHttpSslCertCfg_sharedStructMap.map;
}

//Http ssl cfg

STNBStructMapsRec NBHttpSslCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBHttpSslCfg_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBHttpSslCfg_sharedStructMap);
	if(NBHttpSslCfg_sharedStructMap.map == NULL){
		STNBHttpSslCfg s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBHttpSslCfg);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addBoolM(map, s, isDisabled);
		NBStructMap_addStructM(map, s, cert, NBHttpSslCertCfg_getSharedStructMap());
		NBHttpSslCfg_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBHttpSslCfg_sharedStructMap);
	return NBHttpSslCfg_sharedStructMap.map;
}

//Http port cfg

STNBStructMapsRec NBHttpPortCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBHttpPortCfg_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBHttpPortCfg_sharedStructMap);
	if(NBHttpPortCfg_sharedStructMap.map == NULL){
		STNBHttpPortCfg s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBHttpPortCfg);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addBoolM(map, s, isDisabled);
		NBStructMap_addStrPtrM(map, s, server);
		NBStructMap_addUIntM(map, s, port);
        NBStructMap_addStructM(map, s, conns, NBHttpLimitsCfg_getSharedStructMap());
        NBStructMap_addStructM(map, s, perConn, NBHttpConnCfg_getSharedStructMap());
		NBStructMap_addStructM(map, s, ssl, NBHttpSslCfg_getSharedStructMap());
		NBStructMap_addStructPtrM(map, s, reqsRules, NBHttpReqRulesDef_getSharedStructMap());
		NBHttpPortCfg_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBHttpPortCfg_sharedStructMap);
	return NBHttpPortCfg_sharedStructMap.map;
}

//Http service cfg

STNBStructMapsRec NBHttpServiceCfg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBHttpServiceCfg_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBHttpServiceCfg_sharedStructMap);
	if(NBHttpServiceCfg_sharedStructMap.map == NULL){
		STNBHttpServiceCfg s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBHttpServiceCfg);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addStructM(map, s, conns, NBHttpLimitsCfg_getSharedStructMap());
		NBStructMap_addPtrToArrayOfStructM(map, s, ports, portsSz, ENNBStructMapSign_Unsigned, NBHttpPortCfg_getSharedStructMap());
		NBHttpServiceCfg_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBHttpServiceCfg_sharedStructMap);
	return NBHttpServiceCfg_sharedStructMap.map;
}
