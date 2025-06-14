#ifndef NB_RTP_H
#define NB_RTP_H

#include "nb/NBFrameworkDefs.h"
//

#ifdef __cplusplus
extern "C" {
#endif

//-----------------
//-- RTP - RFC3550
//-- https://tools.ietf.org/html/rfc3550
//-----------------

#define NB_RTP_TIMESTAMP_16_DELTA(T0, T1)	(T0 <= T1 ? (T1 - T0) : (0xFFFFU - T0) + 1 + T1)
#define NB_RTP_TIMESTAMP_32_DELTA(T0, T1)	(T0 <= T1 ? (T1 - T0) : (0xFFFFFFFFU - T0) + 1 + T1)
#define NB_RTP_TIMESTAMP_64_DELTA(T0, T1)	(T0 <= T1 ? (T1 - T0) : (0xFFFFFFFFFFFFFFFFU - T0) + 1 + T1)

//Packet header

typedef struct STNBRtpPacketHead_ {
	UI8		version; 		//2 bits
	BOOL	havePadding;	//1 bit
	BOOL	haveExtension;	//1 bit
	UI8		csrcsCount;		//4 bits
	//
	BOOL	isMarker;		//1 bit
	UI8		payloadType;	//7 bits
	//
	UI16	seqNum;			//16 bits, sequence number (random start)
	UI32	timestamp;		//32 bits (16 bits integer, 16 bits fraction: middle 32 bits of a NTP 64 bits timetamp)
	UI32	ssrc;			//32 bits, synchronization source
	//Followed by an 'UI32 csrcs' array, contributing sources ('csrcsCount')
} STNBRtpPacketHead;

//Packet header Extension
typedef struct STNBRtpPacketHeadExt_ {
	UI16	val0;
	UI16	length;
	//Followed by the data
} STNBRtpPacketHeadExt;

#ifdef __cplusplus
} //extern "C"
#endif

#endif
