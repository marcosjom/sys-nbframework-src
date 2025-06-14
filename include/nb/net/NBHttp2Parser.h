#ifndef NB_HTTP2_PARSER_H
#define NB_HTTP2_PARSER_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBString.h"
#include "nb/core/NBArray.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	//-----------------
	//-- HTTP 2.0 - RFC7540
	//-- https://tools.ietf.org/html/rfc7540
	//-----------------
	
	typedef enum ENNBHttp2FrameType_ {
		ENNBHttp2FrameType_DATA			= 0x00,
		ENNBHttp2FrameType_HEADERS		= 0x01,
		ENNBHttp2FrameType_PRIORITY		= 0x02,
		ENNBHttp2FrameType_RTS_STREAM	= 0x03,
		ENNBHttp2FrameType_SETTINGS		= 0x04,
		ENNBHttp2FrameType_PUSH_PROMISE	= 0x05,
		ENNBHttp2FrameType_PING			= 0x06,
		ENNBHttp2FrameType_GOAWAY		= 0x07,
		ENNBHttp2FrameType_WINDOW_UPDATE = 0x08,
		ENNBHttp2FrameType_CONTINUATION	= 0x09
	} ENNBHttp2FrameType;
	
#	define ENNBHttp2FrameFlag_ACK			0x01
#	define ENNBHttp2FrameFlag_END_STREAM	0x01
#	define ENNBHttp2FrameFlag_END_HEADERS	0x04
#	define ENNBHttp2FrameFlag_PADDED		0x08
#	define ENNBHttp2FrameFlag_PRIORITY		0x20
	
	typedef enum ENNBHttp2Setting_ {
		ENNBHttp2Setting_HEADER_TABLE_SIZE		= 0x01,	//4096
		ENNBHttp2Setting_ENABLE_PUSH			= 0x02, //1
		ENNBHttp2Setting_MAX_CONCURRENT_STREAMS	= 0x03, //(infinite)
		ENNBHttp2Setting_INITIAL_WINDOW_SIZE	= 0x04,	//65535
		ENNBHttp2Setting_MAX_FRAME_SIZE			= 0x05,	//16384
		ENNBHttp2Setting_MAX_HEADER_LIST_SIZE	= 0x06,	//(infinite)
	} ENNBHttp2Setting;
	
	typedef enum ENNBHttp2Err_ {
		ENNBHttp2Err_NO_ERROR			= 0x00, //Graceful shutdown
		ENNBHttp2Err_PROTOCOL_ERROR 	= 0x01, //Protocol error detected
		ENNBHttp2Err_INTERNAL_ERROR		= 0x02, //Implementation fault
		ENNBHttp2Err_FLOW_CONTROL_ERROR	= 0x03, //Flow-control limits exceeded
		ENNBHttp2Err_SETTINGS_TIMEOUT	= 0x04, //Settings not acknowledged
		ENNBHttp2Err_STREAM_CLOSED		= 0x05, //Frame received for closed stream
		ENNBHttp2Err_FRAME_SIZE_ERROR	= 0x06, //Frame size incorrect
		ENNBHttp2Err_REFUSED_STREAM		= 0x07, //Stream not processed
		ENNBHttp2Err_CANCEL				= 0x08, //Stream cancelled
		ENNBHttp2Err_COMPRESSION_ERROR	= 0x09, //Compression state not updated
		ENNBHttp2Err_CONNECT_ERROR		= 0x0a, //TCP connection error for CONNECT method
		ENNBHttp2Err_ENHANCE_YOUR_CALM	= 0x0b, //Processing capacity exceeded
		ENNBHttp2Err_INADEQUATE_SECURITY = 0x0c, //Negotiated TLS parameters not acceptable
		ENNBHttp2Err_HTTP_1_1_REQUIRED	= 0x0d, //Use HTTP/1.1 for the request
	} ENNBHttp2Err;
	
	typedef struct STHttp2FrameHead_ {
		UI32	len;	//3 bytes
		UI8		type;	//ENNBHttp2FrameType
		UI8		flag;
		UI32	streamId;
	} STHttp2FrameHead;
	
	typedef struct STNBHttp2Settings_ {
		UI32 headerTableSz;			// 4096
		UI32 enablePush;			//1
		UI32 maxConcurrentStreams;	//(infinite)
		UI32 initialWindowSz;		//65525
		UI32 maxFrameSz;			//16384
		UI32 maxHeaderListSz;		//(infinite)
	} STNBHttp2Settings;
	
	typedef struct STNBHttp2GoAway_ {
		BOOL isR;
		UI32 lastStreamId;
		UI32 errorCode;
	} STNBHttp2GoAway;
	
	typedef struct STNBHttp2FrameHeadersPriority_ {
		BOOL	dependIsExclusive;
		UI32	dependStreamId;
		UI8		priorityWeight;
	} STNBHttp2FrameHeadersPriority;
	
	//Frame header
	void NBHttp2Parser_frameHeadToBuff(const STHttp2FrameHead* frame, BYTE* buff9);
	void NBHttp2Parser_buffToFrameHead(const BYTE* buff9, STHttp2FrameHead* dst);
	
	//Settings frame
	void NBHttp2Parser_settingsSetDefaults(STNBHttp2Settings* sett);
	BOOL NBHttp2Parser_settingsToBuff(const STNBHttp2Settings* sett, const UI32 explicitMask, STNBString* dst);
	BOOL NBHttp2Parser_buffToSettings(const void* buff, const UI32 buffSz, STNBHttp2Settings* dst, UI32 *dstExplicitMask);
	
	//Data frame
	BOOL NBHttp2Parser_dataToBuff(const void* data, const UI32 dataSz, const UI8 paddingLen, const BOOL isEndOfStream, STHttp2FrameHead* dstHead, STNBString* dst);
	BOOL NBHttp2Parser_buffToData(const STHttp2FrameHead* head, const void* buff, const UI32 buffSz, STNBString* dst);
	
	//Headers frame
	BOOL NBHttp2Parser_headersToBuff(const void* data, const UI32 dataSz, const UI8 paddingLen, const BOOL isEndOfHeaders, const BOOL isEndOfStream, const STNBHttp2FrameHeadersPriority* optPriorityParam, STHttp2FrameHead* dstHead, STNBString* dst);
	BOOL NBHttp2Parser_buffToHeaders(const STHttp2FrameHead* head, const void* buff, const UI32 buffSz, STNBHttp2FrameHeadersPriority* dstPriorityParam, STNBString* dst);
	
	//Goaway frame
	BOOL NBHttp2Parser_buffToGoAway(const STHttp2FrameHead* head, const void* buff, const UI32 buffSz, STNBHttp2GoAway* dstGoAway, STNBString* dstAditionalDebugData);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
