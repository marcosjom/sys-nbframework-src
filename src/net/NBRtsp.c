
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBRtsp.h"
//
#include "nb/core/NBString.h"
//#include "nb/core/NBMemory.h"
//#include "nb/core/NBNumParser.h"

//RTSP Methods

const static STNBRtspMethodDef _methodsDefs[] = {
	{ ENNBRtspMethod_OPTIONS, "OPTIONS" },
	{ ENNBRtspMethod_DESCRIBE, "DESCRIBE" },	//SDP: Session Description Protocol, https://datatracker.ietf.org/doc/html/rfc2327
	{ ENNBRtspMethod_SETUP, "SETUP" },
	{ ENNBRtspMethod_PLAY, "PLAY" },
	{ ENNBRtspMethod_PAUSE, "PAUSE" },
	{ ENNBRtspMethod_TEARDOWN, "TEARDOWN" },
	{ ENNBRtspMethod_ANNOUNCE, "ANNOUNCE" },
	{ ENNBRtspMethod_SET_PARAMETER, "SET_PARAMETER" },
	{ ENNBRtspMethod_GET_PARAMETER, "GET_PARAMETER" },
};

const STNBRtspMethodDef* NBRtsp_getMethodDefById(const ENNBRtspMethod methodId){
	const STNBRtspMethodDef* r = NULL;
	if(methodId >= 0 && methodId < ENNBRtspMethod_Count){
		r = &_methodsDefs[methodId];
	}
	return r;
}

const STNBRtspMethodDef* NBRtsp_getMethodDefByName(const char* name){
	const STNBRtspMethodDef* r = NULL;
	UI32 i; const UI32 count = (sizeof(_methodsDefs) / sizeof(_methodsDefs[0]));
	for(i = 0; i < count; i++){
		const STNBRtspMethodDef* def = &_methodsDefs[i]; 
		if(NBString_strIsLike(def->method, name)){
			r = def;
			break;
		}
	}
	return r;
}

const STNBRtspMethodDef* NBRtsp_getMethodDefByNameBytes(const char* name, const UI32 nameLen){
	const STNBRtspMethodDef* r = NULL;
	UI32 i; const UI32 count = (sizeof(_methodsDefs) / sizeof(_methodsDefs[0]));
	for(i = 0; i < count; i++){
		const STNBRtspMethodDef* def = &_methodsDefs[i]; 
		if(NBString_strIsLikeStrBytes(def->method, name, nameLen)){
			r = def;
			break;
		}
	}
	return r;
}

//Param "name=value;"

void NBRtspParam_init(STNBRtspParam* obj){
	NBMemory_setZeroSt(*obj, STNBRtspParam);
}

void NBRtspParam_release(STNBRtspParam* obj){
	if(obj->name != NULL){
		NBMemory_free(obj->name);
		obj->name = NULL;
	}
	if(obj->value != NULL){
		NBMemory_free(obj->value);
		obj->value = NULL;
	}
}

//Transport-spec

void NBRtspTransportSpec_init(STNBRtspTransportSpec* obj){
	NBMemory_setZeroSt(*obj, STNBRtspTransportSpec);
	NBArray_init(&obj->params, sizeof(STNBRtspParam), NULL);
}

void NBRtspTransportSpec_release(STNBRtspTransportSpec* obj){
	if(obj->protocol != NULL){
		NBMemory_free(obj->protocol);
		obj->protocol = NULL;
	}
	if(obj->profile != NULL){
		NBMemory_free(obj->profile);
		obj->profile = NULL;
	}
	if(obj->lowerTransport != NULL){
		NBMemory_free(obj->lowerTransport);
		obj->lowerTransport = NULL;
	}
	{
		SI32 i; for(i = 0; i < obj->params.use; i++){
			STNBRtspParam* p = NBArray_itmPtrAtIndex(&obj->params, STNBRtspParam, i);
			NBRtspParam_release(p);
		}
		NBArray_empty(&obj->params);
		NBArray_release(&obj->params);
	}
}

//OPTIONS

void NBRtspOptions_init(STNBRtspOptions* obj){
	NBMemory_setZeroSt(*obj, STNBRtspOptions);
}

void NBRtspOptions_release(STNBRtspOptions* obj){
	//Nothing
}

//SETUP

void NBRtspSetup_init(STNBRtspSetup* obj){
	NBMemory_setZeroSt(*obj, STNBRtspSetup);
	//Transport
	{
		//Specs
		{
			NBArray_init(&obj->transport.specs, sizeof(STNBRtspTransportSpec), NULL);
		}
	}
	//Session
	{
		//Params
		{
			NBArray_init(&obj->session.params, sizeof(STNBRtspParam), NULL);
		}
	}
}

void NBRtspSetup_release(STNBRtspSetup* obj){
	//Transport
	{
		//specs
		{
			SI32 i; for(i = 0; i < obj->transport.specs.use; i++){
				STNBRtspTransportSpec* spec = NBArray_itmPtrAtIndex(&obj->transport.specs, STNBRtspTransportSpec, i);
				NBRtspTransportSpec_release(spec);
			}
			NBArray_empty(&obj->transport.specs);
			NBArray_release(&obj->transport.specs);
		}
	}
	//Session
	{
		//sessionId
		if(obj->session.sessionId != NULL){
			NBMemory_free(obj->session.sessionId);
			obj->session.sessionId = NULL;
		}
		//Params
		{
			SI32 i; for(i = 0; i < obj->session.params.use; i++){
				STNBRtspParam* p = NBArray_itmPtrAtIndex(&obj->session.params, STNBRtspParam, i);
				NBRtspParam_release(p);
			}
			NBArray_empty(&obj->session.params);
			NBArray_release(&obj->session.params);
		}
	}
}
