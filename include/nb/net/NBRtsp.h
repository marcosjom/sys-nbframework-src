
#ifndef NB_RTSP_H
#define NB_RTSP_H

#include "nb/NBFrameworkDefs.h"
//
#include "nb/core/NBArray.h"

#ifdef __cplusplus
extern "C" {
#endif

	//---------------
	//- RTSP v1
	//- RFC: https://tools.ietf.org/html/rfc2326
	//---------------

	//RTSP Methods

	typedef enum ENNBRtspMethod_ {
		ENNBRtspMethod_OPTIONS = 0,
		ENNBRtspMethod_DESCRIBE,		//https://datatracker.ietf.org/doc/html/rfc2326#appendix-C.1
		//								//SDP: Session Description Protocol, https://datatracker.ietf.org/doc/html/rfc2327
		ENNBRtspMethod_SETUP,
		ENNBRtspMethod_PLAY,
		ENNBRtspMethod_PAUSE,
		ENNBRtspMethod_TEARDOWN,
		ENNBRtspMethod_ANNOUNCE,
		ENNBRtspMethod_SET_PARAMETER,
		ENNBRtspMethod_GET_PARAMETER,
		//Count
		ENNBRtspMethod_Count
	} ENNBRtspMethod;

	typedef struct STNBRtspMethodDef_ {
		ENNBRtspMethod		methodId;
		const char*			method;				
	} STNBRtspMethodDef;

	const STNBRtspMethodDef* NBRtsp_getMethodDefById(const ENNBRtspMethod methodId);
	const STNBRtspMethodDef* NBRtsp_getMethodDefByName(const char* name);
	const STNBRtspMethodDef* NBRtsp_getMethodDefByNameBytes(const char* name, const UI32 nameLen);
	
	//Param "name=value;"

	typedef struct STNBRtspParam_ {
		char*		name;
		char*		value;
	} STNBRtspParam;

	void NBRtspParam_init(STNBRtspParam* obj);
	void NBRtspParam_release(STNBRtspParam* obj);

	//Delivery-mode

	typedef enum ENRtspDeliveryMode_ {
		ENRtspDeliveryMode_Unknown = 0,
		ENRtspDeliveryMode_Unicast,
		ENRtspDeliveryMode_Multicast
	} ENRtspDeliveryMode;

	//Channels pair

	typedef struct STNBRtspChannelsPair_ {
		UI32	c0;
		UI32	c1;
	} STNBRtspChannelsPair;

	//Ports pair (usually RTP-RTCP)

	typedef struct STNBRtspPortsPair_ {
		UI32	rtp;
		UI32	rtcp;
	} STNBRtspPortsPair;

	//Transport-spec

	typedef struct STNBRtspTransportSpec_ {
		char*		protocol;		//"RTP"
		char*		profile;		//"AVP"
		char*		lowerTransport;	//"TCP" | "UDP" (For RTP/AVP, the default is UDP.)
		STNBArray	params;			//STNBRtspParam
		//Parsed
		struct {
			ENRtspDeliveryMode		deliveryMode;
			const char*				destination;	//string in params.value.
			STNBRtspChannelsPair	interleaved;
			BOOL					append;
			SI32					layers;
			const char*				mode;			//string in params.value.
			//Multicst specific
			struct {
				SI32				ttl;
			} multicast;
			//RTP specific
			struct {
				STNBRtspPortsPair	port;
				STNBRtspPortsPair	client_port;
				STNBRtspPortsPair	server_port;
				UI32				ssrc;
			} rtp;
		} parsed;
	} STNBRtspTransportSpec;

	void NBRtspTransportSpec_init(STNBRtspTransportSpec* obj);
	void NBRtspTransportSpec_release(STNBRtspTransportSpec* obj);

	//OPTIONS

	typedef struct STNBRtspOptions_ {
		//"Public" header
		struct {
			BOOL	methods[ENNBRtspMethod_Count];
		} pub;
	} STNBRtspOptions;

	void NBRtspOptions_init(STNBRtspOptions* obj);
	void NBRtspOptions_release(STNBRtspOptions* obj);

	//Transport spec
	
	/*
	Transport           =    "Transport" ":"
							1\#transport-spec
	transport-spec      =    transport-protocol/profile[/lower-transport]
							*parameter
	transport-protocol  =    "RTP"
	profile             =    "AVP"
	lower-transport     =    "TCP" | "UDP"
	parameter           =    ( "unicast" | "multicast" )
					   |    ";" "destination" [ "=" address ]
					   |    ";" "interleaved" "=" channel [ "-" channel ]
					   |    ";" "append"
					   |    ";" "ttl" "=" ttl
					   |    ";" "layers" "=" 1*DIGIT
					   |    ";" "port" "=" port [ "-" port ]		//multicast
					   |    ";" "client_port" "=" port [ "-" port ]	//unicast
					   |    ";" "server_port" "=" port [ "-" port ]
					   |    ";" "ssrc" "=" ssrc
					   |    ";" "mode" = <"> 1\#mode <">
	ttl                 =    1*3(DIGIT)
	port                =    1*5(DIGIT)
	ssrc                =    8*8(HEX)
	channel             =    1*3(DIGIT)
	address             =    host
	mode                =    <"> *Method <"> | Method
	*/

	//SETUP

	typedef struct STNBRtspSetup_ {
		//Transport
		struct {
			STNBArray	specs;	//STNBRtspTransportSpec
		} transport;
		//Session (per-resource)
		struct {
			char*		sessionId;
			STNBArray	params;		//STNBRtspParam
			//parsed
			struct {
				UI32	timeout;
			} parsed;
		} session;
	} STNBRtspSetup;

	void NBRtspSetup_init(STNBRtspSetup* obj);
	void NBRtspSetup_release(STNBRtspSetup* obj);

	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
