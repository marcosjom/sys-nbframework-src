
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBRtcp.h"

//Packet type

static const STNBRtcpPacketTypeDef _packetTypesDefs[] = {
	{ ENRtcpPacketType_SR, "SR", "Sender Report" },
	{ ENRtcpPacketType_RR, "RR", "Receiver Report" },
	{ ENRtcpPacketType_SDES, "SDES", "Source Description" },
	{ ENRtcpPacketType_BYE, "BYE", "End Of Participation" },
	{ ENRtcpPacketType_APP, "APP", "Application-specific function" }
};

const STNBRtcpPacketTypeDef* NBRtcp_getPacketTypeDef(const UI8 type){
	const STNBRtcpPacketTypeDef* r = NULL;
	switch (type) {
		case ENRtcpPacketType_SR:
		case ENRtcpPacketType_RR:
		case ENRtcpPacketType_SDES:
		case ENRtcpPacketType_BYE:
		case ENRtcpPacketType_APP:
			r = &_packetTypesDefs[type - ENRtcpPacketType_SR];
			break;
		default:
			break;
	}
	return r;
}

//SDES (Source Description) type

static const STNBRtcpSDESTypeDef _sdesTypesDefs[] = {
	{ ENRtcpPacketSDESType_END, "END", "End-of-chunk" },
	{ ENRtcpPacketSDESType_CNAME, "CNAME", "Canonical End-Point Identifier" },
	{ ENRtcpPacketSDESType_NAME, "NAME", "User Name" },
	{ ENRtcpPacketSDESType_EMAIL, "EMAIL", "Electronic Mail Address" },
	{ ENRtcpPacketSDESType_PHONE, "PHONE", "Phone Number" },
	{ ENRtcpPacketSDESType_LOC, "LOC", "Geographic User Location" },
	{ ENRtcpPacketSDESType_TOOL, "TOOL", "Application or Tool Name" },
	{ ENRtcpPacketSDESType_NOTE, "NOTE", "Notice/Status" },
	{ ENRtcpPacketSDESType_PRIV, "PRIV", "Private Extensions" }
};

const STNBRtcpSDESTypeDef* NBRtcp_getSDESTypeDef(const UI8 type){
	const STNBRtcpSDESTypeDef* r = NULL;
	switch(type){
		case ENRtcpPacketSDESType_END:
		case ENRtcpPacketSDESType_CNAME:
		case ENRtcpPacketSDESType_NAME:
		case ENRtcpPacketSDESType_EMAIL:
		case ENRtcpPacketSDESType_PHONE:
		case ENRtcpPacketSDESType_LOC:
		case ENRtcpPacketSDESType_TOOL:
		case ENRtcpPacketSDESType_NOTE:
		case ENRtcpPacketSDESType_PRIV:
			r = &_sdesTypesDefs[type - ENRtcpPacketSDESType_END];
			break;
		default:
			break;
	}
	return r;
}
