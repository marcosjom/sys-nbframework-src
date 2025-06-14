#ifndef NB_SDP_H
#define NB_SDP_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBRange.h"
//

#ifdef __cplusplus
extern "C" {
#endif

//-----------------
//-- RTP - RFC2327
//-- https://datatracker.ietf.org/doc/html/rfc2327
//--
//-- MIME: "application/sdp"
//--
//-- An SDP session description consists of a number of lines of text of
//-- the form <type>=<value> <type> is always exactly one character and is
//-- case-significant.  <value> is a structured text string whose format
//-- depends on <type>.  It also will be case-significant unless a
//-- specific field defines otherwise.  Whitespace is not permitted either
//-- side of the `=' sign. In general <value> is either a number of fields
//-- delimited by a single space character or a free format string.
//--
//-- Session description
//-- v=  (protocol version)
//-- o=  (owner/creator and session identifier).
//-- s=  (session name)
//-- i=* (session information)
//-- u=* (URI of description)
//-- e=* (email address)
//-- p=* (phone number)
//-- c=* (connection information - not required if included in all media)
//-- b=* (bandwidth information)
//-- One or more time descriptions (see below)
//-- z=* (time zone adjustments)
//-- k=* (encryption key)
//-- a=* (zero or more session attribute lines)
//-- Zero or more media descriptions (see below)
//-- 
//-- Time description
//-- t=  (time the session is active)
//-- r=* (zero or more repeat times)
//-- 
//-- Media description
//-- m=  (media name and transport address)
//-- i=* (media title)
//-- c=* (connection information - optional if included at session-level)
//-- b=* (bandwidth information)
//-- k=* (encryption key)
//-- a=* (zero or more media attribute lines)
//-----------------

//-----------------
//-- The set of `type' letters is deliberately small and not intended to be extensible
//-----------------

typedef enum ENNBSdpType_ {
	ENNBSdpType_undef = 0,
	//-- Session description
	ENNBSdpType_v,
	ENNBSdpType_o,
	ENNBSdpType_s,
	ENNBSdpType_i,
	ENNBSdpType_u,
	ENNBSdpType_e,
	ENNBSdpType_p,
	ENNBSdpType_c,
	ENNBSdpType_b,
	ENNBSdpType_z,
	ENNBSdpType_k,
	ENNBSdpType_a,
	//
	ENNBSdpType_t,
	ENNBSdpType_r,
	//
	ENNBSdpType_m,
	//
	ENNBSdpType_Count
} ENNBSdpType;

typedef struct STNBSdpTypeDef_ {
	ENNBSdpType	type;
	char		letter;
	BOOL		isOptional;
	const char*	desc;
} STNBSdpTypeDef;

//Line 

typedef struct STNBSdpLine_ {
	ENNBSdpType	type;
	UI32		iStart;
	UI16		len;
	BOOL		eqFound;	//found '='
	BOOL		isOpen;		//found first char after '='
	BOOL		mustClose;	//found '\r' and '\n' must follow
	BOOL		isClosed;	//end-of-line found
} STNBSdpLine;

UI32 NBSdpLine_appendText(STNBSdpLine* obj, const char expFirstLetter, const ENNBSdpType expType, const UI32 curLen, const char* str, const UI32 strLen);

//Times

typedef struct STNBSdpTimes_ {
	STNBSdpLine		line;
	//start
	struct {
		UI64		time;
	} start;
	//stop
	struct {
		UI64		time;
	} stop;
} STNBSdpTimes;

BOOL NBSdpTimes_parseInternalLine(STNBSdpTimes* obj, const char* strBuff);

//Repeat

typedef struct STNBSdpRepeat_ {
	STNBSdpLine		line;
	//interval
	struct {
		UI64		time;
	} interval;
	//duration
	struct {
		UI64		time;
	} duration;
	//offsets
	struct {
		UI64*		list;
		UI32		listSz;
	} offsets;
} STNBSdpRepeat;

BOOL NBSdpRepeat_parseInternalLine(STNBSdpRepeat* obj, const char* strBuff);

//TimeDesc

typedef struct STNBSdpTimeDesc_ {
	BOOL			errFnd;
	STNBSdpTimes	time;
	STNBSdpLine*	repeat;
	UI32			repeatSz;
} STNBSdpTimeDesc;

void NBSdpTimeDesc_init(STNBSdpTimeDesc* obj);
void NBSdpTimeDesc_release(STNBSdpTimeDesc* obj);
UI32 NBSdpTimeDesc_appendText(STNBSdpTimeDesc* obj, const UI32 curLen, const char* str, const UI32 strLen);
BOOL NBSdpTimeDesc_canBeClosed(STNBSdpTimeDesc* obj);

//Connection Data

typedef struct STNBSdpConnData_ {
	STNBSdpLine		line;
	//netType
	struct {
		UI32		iStart;	//'IN' means 'Internet', ...
		UI16		len;
	} netType;
	//addrType
	struct {
		UI32		iStart;	//'IP4', 'IP6', ...
		UI16		len;
	} addrType;
	//address
	struct {
		UI32		iStart;	//fully-qualified domain or IP form, ...
		UI16		len;
	} address;
} STNBSdpConnData;

BOOL NBSdpConnData_parseInternalLine(STNBSdpConnData* obj, const char* strBuff);

//Bandwidth

typedef enum ENNBSdpBandwidthType_ {
	ENNBSdpBandwidthType_Unknown = 0,
	ENNBSdpBandwidthType_Extension,		// 'X-...' Extension Mechanism
	ENNBSdpBandwidthType_ConferenceTotal,	//'CT', 'Conference Total'
	ENNBSdpBandwidthType_ApplicationMax		//'AS', 'Application-Specific Maximum'
} ENNBSdpBandwidthType;

typedef struct STNBSdpBandwidth_ {
	STNBSdpLine		line;
	//netType
	struct {
		UI32		iStart;	//'CT' Conference Total, 'AS' Application-Specific Maximum, 'X-...' Extension Mechanism
		UI16		len;
	} modifier;
	//address
	struct {
		ENNBSdpBandwidthType type;
		UI32		kbps;
	} value;
} STNBSdpBandwidth;

BOOL NBSdpBandwidth_parseInternalLine(STNBSdpBandwidth* obj, const char* strBuff);

//MediaDesc

typedef struct STNBSdpMediaDesc_ {
	BOOL			errFnd;
	STNBSdpLine		nameAddr;	//m
	STNBSdpLine*	title;		//i
	STNBSdpConnData* connInfo;	//c
	STNBSdpBandwidth* bandInfo;	//b
	STNBSdpLine*	encKey;		//k
	STNBSdpLine*	attribs;	//a
	UI32			attribsSz;
} STNBSdpMediaDesc;

void NBSdpMediaDesc_init(STNBSdpMediaDesc* obj);
void NBSdpMediaDesc_release(STNBSdpMediaDesc* obj);
UI32 NBSdpMediaDesc_appendText(STNBSdpMediaDesc* obj, const UI32 curLen, const char* str, const UI32 strLen);
BOOL NBSdpMediaDesc_canBeClosed(STNBSdpMediaDesc* obj);

//Version

typedef struct STNBSdpVersion_ {
	STNBSdpLine		line;
	UI32			major;
} STNBSdpVersion;

//Origin

typedef struct STNBSdpOrigin_ {
	STNBSdpLine		line;
	//userid
	struct {
		UI32		iStart;
		UI16		len;
		BOOL		supported;	//username not '-'
	} userid;
	//sessionId
	struct {
		UI64		number;
	} sessionId;
	//version
	struct {
		UI64		number;
	} version;
	//netType
	struct {
		UI32		iStart;	//'IN' means 'Internet', ...
		UI16		len;
	} netType;
	//addrType
	struct {
		UI32		iStart;	//'IP4', 'IP6', ...
		UI16		len;
	} addrType;
	//address
	struct {
		UI32		iStart;	//fully-qualified domain or IP form, ...
		UI16		len;
	} address;
} STNBSdpOrigin;

//SessionDesc

typedef struct STNBSdpSessionDesc_ {
	BOOL			errFnd;
	STNBSdpVersion	version;		//v (required)
	STNBSdpOrigin	origin;			//o (required)
	STNBSdpLine		name;			//s (required)
	STNBSdpLine*	info;			//i (optional)
	STNBSdpLine*	uri;			//u (optional)
	STNBSdpLine*	email;			//e (optional)
	STNBSdpLine*	phone;			//p (optional)
	STNBSdpConnData* connInfo;		//c (optional)
	STNBSdpBandwidth* bandInfo;		//b (optional)
	STNBSdpTimeDesc* timeDescs;		//one or more
	UI32			timeDescsSz;	//
	STNBSdpLine*	timeZone;		//z (optional)
	STNBSdpLine*	encKey;			//k (optional)
	STNBSdpLine*	attribs;		//a (optional)
	UI32			attribsSz;		//
	STNBSdpMediaDesc* mediaDescs;	//zero or more
	UI32			mediaDescsSz;	//
} STNBSdpSessionDesc;

void NBSdpSessionDesc_init(STNBSdpSessionDesc* obj);
void NBSdpSessionDesc_release(STNBSdpSessionDesc* obj);
UI32 NBSdpSessionDesc_appendText(STNBSdpSessionDesc* obj, const UI32 curLen, const char* str, const UI32 strLen);
BOOL NBSdpSessionDesc_canBeClosed(STNBSdpSessionDesc* obj);

//Desc

typedef struct STNBSdpDesc_ {
	char*				str;
	UI32				len;
	BOOL				errFnd;
	STNBSdpSessionDesc*	sessions;
	UI32				sessionsSz;
} STNBSdpDesc;

void NBSdpDesc_init(STNBSdpDesc* obj);
void NBSdpDesc_release(STNBSdpDesc* obj);
UI32 NBSdpDesc_appendText(STNBSdpDesc* obj, const char* str, const UI32 strLen, const BOOL isFinal);
void NBSdpDesc_flush(STNBSdpDesc* obj);
BOOL NBSdpDesc_canBeClosed(STNBSdpDesc* obj);

//Known SDP attributes
//https://datatracker.ietf.org/doc/html/rfc4566#section-6
//6.  SDP Attributes

//a=cat:<category>
typedef struct STNBSdpAttribCat_ {
	STNBRangeI cat;
} STNBSdpAttribCat;

BOOL NBSdpDesc_parseAttribCat(const char* pay, const UI32 payLen, STNBSdpAttribCat* dst);

//a=keywds:<keywords>
typedef struct STNBSdpAttribKeywords_ {
	STNBRangeI keywords;
} STNBSdpAttribKeywords;

BOOL NBSdpDesc_parseAttribKeywords(const char* pay, const UI32 payLen, STNBSdpAttribKeywords* dst);

//a=tool:<name and version of tool>
typedef struct STNBSdpAttribTool_ {
	STNBRangeI nameAndVer;
} STNBSdpAttribTool;

BOOL NBSdpDesc_parseAttribTool(const char* pay, const UI32 payLen, STNBSdpAttribTool* dst);

//a=ptime:<packet time>
typedef struct STNBSdpAttribPTime_ {
	UI32 ms;
} STNBSdpAttribPTime;

BOOL NBSdpDesc_parseAttribPTime(const char* pay, const UI32 payLen, STNBSdpAttribPTime* dst);

//a=maxptime:<maximum packet time>
typedef struct STNBSdpAttribMaxPTime_ {
	UI32 ms;
} STNBSdpAttribMaxPTime;

BOOL NBSdpDesc_parseAttribMaxPTime(const char* pay, const UI32 payLen, STNBSdpAttribMaxPTime* dst);

//a=rtpmap:<payload type> <encoding name>/<clock rate> [/<encoding parameters>]
typedef struct STNBSdpAttribRtpMap_ {
	UI32		payType;
	STNBRangeI	encName;
	UI32		clockRate;
	STNBRangeI	encParams;
} STNBSdpAttribRtpMap;

BOOL NBSdpDesc_parseAttribRtpMap(const char* pay, const UI32 payLen, STNBSdpAttribRtpMap* dst);

//a=recvonly
typedef struct STNBSdpAttribRecvOnly_ {
	BOOL recvOnly;
} STNBSdpAttribRecvOnly;

BOOL NBSdpDesc_parseAttribRecvOnly(const char* pay, const UI32 payLen, STNBSdpAttribRecvOnly* dst);

//a=sendrecv
typedef struct STNBSdpAttribSendRecv_ {
	BOOL sendRecv;
} STNBSdpAttribSendRecv;

BOOL NBSdpDesc_parseAttribSendRecv(const char* pay, const UI32 payLen, STNBSdpAttribSendRecv* dst);

//a=sendonly
typedef struct STNBSdpAttribSendOnly_ {
	BOOL sendOnly;
} STNBSdpAttribSendOnly;

BOOL NBSdpDesc_parseAttribSendOnly(const char* pay, const UI32 payLen, STNBSdpAttribSendOnly* dst);

//a=inactive
typedef struct STNBSdpAttribInactive_ {
	BOOL inactive;
} STNBSdpAttribInactive;

BOOL NBSdpDesc_parseAttribInactive(const char* pay, const UI32 payLen, STNBSdpAttribInactive* dst);

//a=orient:<orientation>
typedef enum ENNBSdpAttribOrient_ {
	ENNBSdpAttribOrient_Portrait = 0,
	ENNBSdpAttribOrient_Landscape,
	ENNBSdpAttribOrient_Seascape, //(upside-down landscape)
	ENNBSdpAttribOrient_Count
} ENNBSdpAttribOrient;

typedef struct STNBSdpAttribOrient_ {
	ENNBSdpAttribOrient orient; //Permitted values are "portrait", "landscape", and "seascape" (upside-down landscape).
} STNBSdpAttribOrient;

BOOL NBSdpDesc_parseAttribOrient(const char* pay, const UI32 payLen, STNBSdpAttribOrient* dst);


//a=type:<conference type>
//Suggested values are "broadcast", "meeting", "moderated", "test", and "H332".
typedef struct STNBSdpAttribType_ {
	STNBRangeI type;
} STNBSdpAttribType;

BOOL NBSdpDesc_parseAttribType(const char* pay, const UI32 payLen, STNBSdpAttribType* dst);

//a=charset:<character set>
typedef struct STNBSdpAttribCharset_ {
	STNBRangeI charset;
} STNBSdpAttribCharset;

BOOL NBSdpDesc_parseAttribCharset(const char* pay, const UI32 payLen, STNBSdpAttribCharset* dst);

//a=sdplang:<language tag>
//language tag: https://datatracker.ietf.org/doc/html/rfc3066
typedef struct STNBSdpAttribSdpLang_ {
	STNBRangeI sdpLang;
} STNBSdpAttribSdpLang;

BOOL NBSdpDesc_parseAttribSdpLang(const char* pay, const UI32 payLen, STNBSdpAttribSdpLang* dst);

//a=lang:<language tag>
//language tag: https://datatracker.ietf.org/doc/html/rfc3066
typedef struct STNBSdpAttribLang_ {
	STNBRangeI lang;
} STNBSdpAttribLang;

BOOL NBSdpDesc_parseAttribLang(const char* pay, const UI32 payLen, STNBSdpAttribLang* dst);

//a=framerate:<frame rate>
//Decimal representations of fractional values using the notation "<integer>.<fraction>" are allowed
typedef struct STNBSdpAttribFramerate_ {
	UI32 integer;
	UI32 decimal;
} STNBSdpAttribFramerate;

BOOL NBSdpDesc_parseAttribFramerate(const char* pay, const UI32 payLen, STNBSdpAttribFramerate* dst);

//a=quality:<quality>
//For video, the value is in the range 0 to 10 (best).
typedef struct STNBSdpAttribQuality_ {
	UI32 quality;
} STNBSdpAttribQuality;

BOOL NBSdpDesc_parseAttribQuality(const char* pay, const UI32 payLen, STNBSdpAttribQuality* dst);

//a=fmtp:<format> <format specific parameters>
typedef struct STNBSdpAttribFmtp_ {
	UI32 format;
	STNBRangeI params;
} STNBSdpAttribFmtp;

BOOL NBSdpDesc_parseAttribFmtp(const char* pay, const UI32 payLen, STNBSdpAttribFmtp* dst);

#ifdef NB_CONFIG_INCLUDE_ASSERTS
BOOL NBSdpDesc_dbgTest(void);
#endif

#ifdef __cplusplus
} //extern "C"
#endif

#endif
