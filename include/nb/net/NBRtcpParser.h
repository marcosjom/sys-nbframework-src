#ifndef NB_RTCP_PARSER_H
#define NB_RTCP_PARSER_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"
#include "nb/net/NBRtcp.h"

//

#ifdef __cplusplus
extern "C" {
#endif
	
	//-----------------
	//-- RTP - RFC3550
	//-- https://tools.ietf.org/html/rfc3550
	//-----------------

	//Packet

	typedef struct STNBRtcpParserResultPacket_ {
		STNBRtcpPacketHead	head;
		void*				data;
	} STNBRtcpParserResultPacket;

	//Result

	typedef struct STNBRtcpParserResult_ {
		STNBRtcpParserResultPacket* packets;
		UI32						packetsSz;
	} STNBRtcpParserResult;

	//Result

	void NBRtcpParserResult_init(STNBRtcpParserResult* obj);
	void NBRtcpParserResult_release(STNBRtcpParserResult* obj);
	void NBRtcpParserResult_concat(const STNBRtcpParserResult* obj, STNBString* dst);

	//Parser

	BOOL NBRtcpParser_translatePacketBytes(void* data, const UI32 dataSz, STNBRtcpParserResult* dst);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
