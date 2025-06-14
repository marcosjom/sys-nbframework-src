#ifndef NB_RTCP_H
#define NB_RTCP_H

#include "nb/NBFrameworkDefs.h"
//

#ifdef __cplusplus
extern "C" {
#endif
	
//-----------------
//-- RTP - RFC3550
//-- https://tools.ietf.org/html/rfc3550
//-----------------

//Packet type

typedef enum ENRtcpPacketType_ {
	ENRtcpPacketType_SR   = 200,	//SenderReport
	ENRtcpPacketType_RR   = 201,	//ReceiverReport
	ENRtcpPacketType_SDES = 202,	//SourceDescription items
	ENRtcpPacketType_BYE  = 203,	//EndOfParticipation
	ENRtcpPacketType_APP  = 204		//Application-specific functions
} ENRtcpPacketType;

typedef struct STNBRtcpPacketTypeDef_ {
	ENRtcpPacketType	type;
	const char*			acronim;
	const char*			desc;
} STNBRtcpPacketTypeDef;

//SDES (Source Description) type

typedef enum ENRtcpPacketSDESType_ {
	ENRtcpPacketSDESType_END   = 0,		//End-of-chunk
	ENRtcpPacketSDESType_CNAME = 1,		//Canonical End-Point Identifier
	ENRtcpPacketSDESType_NAME  = 2,		//User Name
	ENRtcpPacketSDESType_EMAIL = 3,		//Electronic Mail Address
	ENRtcpPacketSDESType_PHONE = 4,		//Phone Number
	ENRtcpPacketSDESType_LOC   = 5,		//Geographic User Location
	ENRtcpPacketSDESType_TOOL  = 6,		//Application or Tool Name
	ENRtcpPacketSDESType_NOTE  = 7,		//Notice/Status
	ENRtcpPacketSDESType_PRIV  = 8		//Private Extensions
} ENRtcpPacketSDESType;

typedef struct STNBRtcpSDESTypeDef_ {
	ENRtcpPacketSDESType	type;
	const char*				acronim;
	const char*				desc;
} STNBRtcpSDESTypeDef;

//Packet header

typedef struct STNBRtcpPacketHead_ {
	UI8		version; 		//2 bits
	BOOL	havePadding;	//1 bit
	union {
		UI8		count;		//5 bits (reception report count)
		UI8		subtype;	//5 bits (for APP: Application-Defined RTCP Packet)
	};
	UI8		type;			//8 bits, ENRtcpPacketType
	UI16	length;			//16 bits, pkt len in words, w/o this word
} STNBRtcpPacketHead;

//Packet reception report

typedef struct STNBRtcpRcptRprt_ {
	UI32	ssrc;				//32 bits, data source being reported
	UI8		fractionLost;		//8 bits, fraction lost
	UI32	accumPacktLost;		//24 bits, cumulative number of packets lost
	UI32	highestSeqNum;		//32 bits, extended highest sequence number received
	UI32	interarrivalJitter;	//32 bits, interarrival jitter
	UI32	lastSenderReport;	//32 bits, last SR (LSR)
	UI32	delaySinceLastSR;	//32 bits, delay since last SR (DLSR)
} STNBRtcpRcptRprt;

//Packet sender report (SR)

typedef struct STNBRtcpPacketSRHead_ {
	UI32	ssrc;			//32 bits, SSRC of sender 
	UI32	ntpWordHigh;	//32 bits, NTP timestamp, most significant word 
	UI32	ntpWordLow;		//32 bits, NTP timestamp, least significant word
	UI32	rtpTimestamp;	//32 bits, RTP timestamp
	UI32	packtCount;		//32 bits, sender's packet count
	UI32	octetCount;		//32 bits, sender's octet count
} STNBRtcpPacketSRHead;

typedef struct STNBRtcpPacketSR_ {
	STNBRtcpPacketSRHead	head;
	STNBRtcpRcptRprt*		reports;
	UI32					reportsSz;
} STNBRtcpPacketSR;

//Packet receiver report (RR)

typedef struct STNBRtcpPacketRRHead_ {
	UI32	ssrc;			//32 bits, SSRC of sender 
} STNBRtcpPacketRRHead;

typedef struct STNBRtcpPacketRR_ {
	STNBRtcpPacketRRHead	head;
	STNBRtcpRcptRprt*		reports;
	UI32					reportsSz;
} STNBRtcpPacketRR;

//Packet source description item (SDES)

typedef struct STNBRtcpSrcDescItm_ {
	UI8		type;		//type of item (rtcp_sdes_type_t)
	UI8		length;		//length of item (in octets)
	char*	str;		//text, not null-terminated
} STNBRtcpSrcDescItm;

typedef struct STNBRtcpSrcDescChunck_ {
	union {
		UI32	ssrc;
		UI32	csrc;
	};
	STNBRtcpSrcDescItm*	itms;	//list of SDES items
	UI32				itmsSz;	//list of SDES items
} STNBRtcpSrcDescChunck;

//Packet source description item (SDES)

typedef struct STNBRtcpPacketSDES_ {
	STNBRtcpSrcDescChunck*	chunks;
	UI32					chunksSz;
} STNBRtcpPacketSDES;

//Packet reception report (BYE)

typedef struct STNBRtcpPacketBYE_ {
	UI32*		ssrcs;		//list of sources
	UI32		ssrcsSz;	//list of sources
	//reason for leaving (optional)
	struct {
		UI8		length;		//length of item (in octets)
		char*	str;		//text, not null-terminated
	} reason;
} STNBRtcpPacketBYE;

//

const STNBRtcpPacketTypeDef*	NBRtcp_getPacketTypeDef(const UI8 type);
const STNBRtcpSDESTypeDef*		NBRtcp_getSDESTypeDef(const UI8 type);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
