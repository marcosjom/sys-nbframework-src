
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBHttp2Parser.h"
//
#include "nb/core/NBStruct.h"

//Frame header

void NBHttp2Parser_frameHeadToBuff(const STHttp2FrameHead* frame, BYTE* buff9){
	const BYTE* src = (const BYTE*)frame;
	buff9[0] = src[2]; buff9[1] = src[1]; buff9[2] = src[0]; //len
	buff9[3] = src[4]; //type
	buff9[4] = src[5]; //flag
	buff9[5] = src[11]; buff9[6] = src[10]; buff9[7] = src[9]; buff9[8] = src[8]; //streamId
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	//Validate
	{
		STHttp2FrameHead f;
		NBMemory_setZeroSt(f, STHttp2FrameHead);
		NBHttp2Parser_buffToFrameHead(buff9, &f);
		NBASSERT(frame->len == f.len)
		NBASSERT(frame->type == f.type)
		NBASSERT(frame->flag == f.flag)
		NBASSERT(frame->streamId == f.streamId)
	}
#	endif
}

void NBHttp2Parser_buffToFrameHead(const BYTE* buff9, STHttp2FrameHead* pDst){
	BYTE* dst = (BYTE*)pDst;
	NBMemory_setZeroSt(*pDst, STHttp2FrameHead);
	dst[2] = buff9[0]; dst[1] = buff9[1]; dst[0] = buff9[2]; //len
	dst[4] = buff9[3]; //type
	dst[5] = buff9[4]; //flag
	dst[11] = buff9[5]; dst[10] = buff9[6]; dst[9] = buff9[7]; dst[8] = buff9[8];
	//Do not validate, to avoid recursivity
}

//Settings frame

void NBHttp2Parser_settingsSetDefaults(STNBHttp2Settings* sett){
	NBMemory_setZeroSt(*sett, STNBHttp2Settings);
	sett->headerTableSz			= 4096;			//4096
	sett->enablePush			= 0;			//1
	sett->maxConcurrentStreams 	= 0xFFFFFFFF;	//(infinite)
	sett->initialWindowSz		= 65525;		//65525
	sett->maxFrameSz			= 16384;		//16384
	sett->maxHeaderListSz   	= 0xFFFFFFFF;	//(infinite)
}

void NBHttp2Parser_settingToBuff(const UI16 settId, const UI32 settValue, STNBString* dst){
	const BYTE* srcId = (const BYTE*)&settId;
	const BYTE* srcVal = (const BYTE*)&settValue;
	//Id
	NBString_concatByte(dst, srcId[1]);
	NBString_concatByte(dst, srcId[0]);
	//Value
	NBString_concatByte(dst, srcVal[3]);
	NBString_concatByte(dst, srcVal[2]);
	NBString_concatByte(dst, srcVal[1]);
	NBString_concatByte(dst, srcVal[0]);
}

BOOL NBHttp2Parser_settingsToBuff(const STNBHttp2Settings* sett, const UI32 explicitMask, STNBString* dst){
	BOOL r = TRUE;
	IF_NBASSERT(const UI32 lenStart = dst->length;)
	if((explicitMask & (0x01 << (ENNBHttp2Setting_HEADER_TABLE_SIZE - 1))) != 0){
		NBHttp2Parser_settingToBuff(ENNBHttp2Setting_HEADER_TABLE_SIZE, sett->headerTableSz, dst);
	}
	if((explicitMask & (0x01 << (ENNBHttp2Setting_ENABLE_PUSH - 1))) != 0){
		NBHttp2Parser_settingToBuff(ENNBHttp2Setting_ENABLE_PUSH, sett->enablePush, dst);
	}
	if((explicitMask & (0x01 << (ENNBHttp2Setting_MAX_CONCURRENT_STREAMS - 1))) != 0){
		NBHttp2Parser_settingToBuff(ENNBHttp2Setting_MAX_CONCURRENT_STREAMS, sett->maxConcurrentStreams, dst);
	}
	if((explicitMask & (0x01 << (ENNBHttp2Setting_INITIAL_WINDOW_SIZE - 1))) != 0){
		NBHttp2Parser_settingToBuff(ENNBHttp2Setting_INITIAL_WINDOW_SIZE, sett->initialWindowSz, dst);
	}
	if((explicitMask & (0x01 << (ENNBHttp2Setting_MAX_FRAME_SIZE - 1))) != 0){
		NBHttp2Parser_settingToBuff(ENNBHttp2Setting_MAX_FRAME_SIZE, sett->maxFrameSz, dst);
	}
	if((explicitMask & (0x01 << (ENNBHttp2Setting_MAX_HEADER_LIST_SIZE - 1))) != 0){
		NBHttp2Parser_settingToBuff(ENNBHttp2Setting_MAX_HEADER_LIST_SIZE, sett->maxHeaderListSz, dst);
	}
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	//Validate
	{
		STNBHttp2Settings f;
		NBMemory_setZeroSt(f, STNBHttp2Settings);
		NBHttp2Parser_buffToSettings(&dst->str[lenStart], dst->length - lenStart, &f, NULL);
		if((explicitMask & (0x01 << (ENNBHttp2Setting_HEADER_TABLE_SIZE - 1))) != 0){
			NBASSERT(sett->headerTableSz == f.headerTableSz)
		}
		if((explicitMask & (0x01 << (ENNBHttp2Setting_ENABLE_PUSH - 1))) != 0){
			NBASSERT(sett->enablePush == f.enablePush)
		}
		if((explicitMask & (0x01 << (ENNBHttp2Setting_MAX_CONCURRENT_STREAMS - 1))) != 0){
			NBASSERT(sett->maxConcurrentStreams == f.maxConcurrentStreams)
		}
		if((explicitMask & (0x01 << (ENNBHttp2Setting_INITIAL_WINDOW_SIZE - 1))) != 0){
			NBASSERT(sett->initialWindowSz == f.initialWindowSz)
		}
		if((explicitMask & (0x01 << (ENNBHttp2Setting_MAX_FRAME_SIZE - 1))) != 0){
			NBASSERT(sett->maxFrameSz == f.maxFrameSz)
		}
		if((explicitMask & (0x01 << (ENNBHttp2Setting_MAX_HEADER_LIST_SIZE - 1))) != 0){
			NBASSERT(sett->maxHeaderListSz == f.maxHeaderListSz)
		}
	}
#	endif
	return r;
}

BOOL NBHttp2Parser_buffToSettings(const void* pBuff, const UI32 buffSz, STNBHttp2Settings* dst, UI32 *dstExplicitMask){
	BOOL r = TRUE;
	if((buffSz % 6) != 0){
		PRINTF_ERROR("NBHttp2Parser, settings-frame must be multiple of 6 bytes.\n");
		r = FALSE;
	} else {
		UI32 i = 0;
		UI16 settId = 0;
		UI32 settVal = 0;
		BYTE* settIdPtr = (BYTE*)&settId;
		BYTE* settValPtr = (BYTE*)&settVal;
		const BYTE* buff = (const BYTE*)pBuff;
		while(i < buffSz && r){
			//Id
			settIdPtr[1] = buff[i];
			settIdPtr[0] = buff[i + 1];
			//Value
			settValPtr[3] = buff[i + 2];
			settValPtr[2] = buff[i + 3];
			settValPtr[1] = buff[i + 4];
			settValPtr[0] = buff[i + 5];
			//Process
			switch(settId) {
				case ENNBHttp2Setting_HEADER_TABLE_SIZE:
					if(dstExplicitMask != NULL) *dstExplicitMask = *dstExplicitMask | (0x01 << (settId - 1));
					dst->headerTableSz = settVal;
					break;
				case ENNBHttp2Setting_ENABLE_PUSH:
					if(settVal != 0 && settVal != 1){
						PRINTF_ERROR("NBHttp2Parser, ENABLE_PUSH setting must be 0 or 1.\n");
						r = FALSE;
					} else {
						if(dstExplicitMask != NULL) *dstExplicitMask = *dstExplicitMask | (0x01 << (settId - 1));
						dst->enablePush = settVal;
					}
					break;
				case ENNBHttp2Setting_MAX_CONCURRENT_STREAMS:
					if(dstExplicitMask != NULL) *dstExplicitMask = *dstExplicitMask | (0x01 << (settId - 1));
					dst->maxConcurrentStreams = settVal;
					break;
				case ENNBHttp2Setting_INITIAL_WINDOW_SIZE:
					if(settVal > 2147483647){ //2^31-1
						PRINTF_ERROR("NBHttp2Parser, INITIAL_WINDOW_SIZE setting must be not-greather than 2^31-1 (found %u).\n", settVal);
						r = FALSE;
					} else {
						if(dstExplicitMask != NULL) *dstExplicitMask = *dstExplicitMask | (0x01 << (settId - 1));
						dst->initialWindowSz = settVal;
					}
					break;
				case ENNBHttp2Setting_MAX_FRAME_SIZE:
					if(settVal < 16384 || settVal > 16777215){ //2^14, 2^24-1
						PRINTF_ERROR("NBHttp2Parser, MAX_FRAME_SIZE setting must be from 2^14 to 2^24-1 (found %u).\n", settVal);
						r = FALSE;
					} else {
						if(dstExplicitMask != NULL) *dstExplicitMask = *dstExplicitMask | (0x01 << (settId - 1));
						dst->maxFrameSz = settVal;
					}
					break;
				case ENNBHttp2Setting_MAX_HEADER_LIST_SIZE:
					if(dstExplicitMask != NULL) *dstExplicitMask = *dstExplicitMask | (0x01 << (settId - 1));
					dst->maxHeaderListSz = settVal;
					break;
				default:
					//Just ignore (RFC defined this behavior)
					PRINTF_INFO("NBHttp2Parser, ignoring unexpected setting id(%u)-val(%u).\n", settId, settVal);
					break;
			}
			i += 6;
		}
	}
	return r;
}

//Data frame

BOOL NBHttp2Parser_dataToBuff(const void* data, const UI32 dataSz, const UI8 paddingLen, const BOOL isEndOfStream, STHttp2FrameHead* dstHead, STNBString* dst){
	BOOL r = TRUE;
	const UI32 lenStart	= dst->length;
	dstHead->type		= ENNBHttp2FrameType_DATA;
	dstHead->flag		= (paddingLen > 0 ? ENNBHttp2FrameFlag_PADDED : 0) | (isEndOfStream ? ENNBHttp2FrameFlag_END_STREAM : 0);
	//Padding header
	if(paddingLen > 0){
		NBString_concatBytes(dst, (char*)&paddingLen, 1);
	}
	//Data
	{
		NBString_concatBytes(dst, (char*)data, dataSz);
	}
	//Padding
	if(paddingLen > 0){
		NBString_concatRepeatedByte(dst, 0, paddingLen);
	}
	dstHead->len = (dst->length - lenStart);
	return r;
}

BOOL NBHttp2Parser_buffToData(const STHttp2FrameHead* head, const void* pBuff, const UI32 buffSz, STNBString* dst){
	BOOL r = TRUE;
	const BYTE* buff = (const BYTE*)pBuff;
	UI32 iPos = 0;
	UI8 paddingLen = 0;
	//Padding header
	if(r){
		if((head->flag & ENNBHttp2FrameFlag_PADDED) != 0){
			if(iPos >= buffSz){
				PRINTF_ERROR("NBHttp2Parser, expected more octets for DATA padding-hearder.\n");
				r = FALSE;
			} else {
				paddingLen = buff[iPos++];
			}
		}
	}
	//Data
	if(r){
		if((iPos + paddingLen) > buffSz){
			PRINTF_ERROR("NBHttp2Parser, DATA padding is greather than buffSize.\n");
			r = FALSE;
		} else {
			const UI32 dataSz = (buffSz - iPos - paddingLen);
			NBString_concatBytes(dst, (const char*)&buff[iPos], dataSz);
			iPos += dataSz; NBASSERT(iPos <= buffSz)
		}
	}
	//Validate padding (must be zero)
	if(r){
		while(iPos < buffSz){
			if(buff[iPos] != 0){
				PRINTF_ERROR("NBHttp2Parser, DATA padding must be zeros.\n");
				r = FALSE;
				break;
			}
			iPos++;
		}
	}
	return r;
}

//Headers frame

BOOL NBHttp2Parser_headersToBuff(const void* data, const UI32 dataSz, const UI8 paddingLen, const BOOL isEndOfHeaders, const BOOL isEndOfStream, const STNBHttp2FrameHeadersPriority* optPriorityParam, STHttp2FrameHead* dstHead, STNBString* dst){
	BOOL r = TRUE;
	const UI32 lenStart	= dst->length;
	BOOL havePriority	= FALSE;
	if(optPriorityParam != NULL){
		if(optPriorityParam->dependIsExclusive != 0 || optPriorityParam->dependStreamId != 0 || optPriorityParam->priorityWeight != 0){
			havePriority = TRUE;
		}
	}
	dstHead->type		= ENNBHttp2FrameType_HEADERS;
	dstHead->flag		= (paddingLen > 0 ? ENNBHttp2FrameFlag_PADDED : 0) | (isEndOfHeaders ? ENNBHttp2FrameFlag_END_HEADERS : 0) | (isEndOfStream ? ENNBHttp2FrameFlag_END_STREAM : 0) | (havePriority ? ENNBHttp2FrameFlag_PRIORITY : 0);
	//Padding header
	if(paddingLen > 0){
		NBString_concatBytes(dst, (char*)&paddingLen, 1);
	}
	//Priority
	if(havePriority){
		const BYTE* srcId = (const BYTE*)&optPriorityParam->dependStreamId;
		const BYTE first = (srcId[3] & ~0x80) | (optPriorityParam->dependIsExclusive ? 0x80 : 0x00);
		//E-Flag and streamId
		NBString_concatBytes(dst, (const char*)&first, 1);
		NBString_concatBytes(dst, (const char*)&srcId[2], 1);
		NBString_concatBytes(dst, (const char*)&srcId[1], 1);
		NBString_concatBytes(dst, (const char*)&srcId[0], 1);
		//Weight
		NBString_concatBytes(dst, (const char*)&optPriorityParam->priorityWeight, 1);
	}
	//Headers
	{
		NBString_concatBytes(dst, (char*)data, dataSz);
	}
	//Padding
	if(paddingLen > 0){
		NBString_concatRepeatedByte(dst, 0, paddingLen);
	}
	dstHead->len = (dst->length - lenStart);
	return r;
}

BOOL NBHttp2Parser_buffToHeaders(const STHttp2FrameHead* head, const void* pBuff, const UI32 buffSz, STNBHttp2FrameHeadersPriority* dstPriorityParam, STNBString* dst){
	BOOL r = TRUE;
	const BYTE* buff = (const BYTE*)pBuff;
	UI32 iPos = 0;
	UI8 paddingLen = 0;
	//Padding header
	if(r){
		if((head->flag & ENNBHttp2FrameFlag_PADDED) != 0){
			if(iPos >= buffSz){
				PRINTF_ERROR("NBHttp2Parser, expected more octets for HEADERS padding-hearder.\n");
				r = FALSE;
			} else {
				paddingLen = buff[iPos++];
			}
		}
	}
	//Priority
	if(r){
		if((head->flag & ENNBHttp2FrameFlag_PRIORITY) != 0){
			if((iPos + 5) > buffSz){
				PRINTF_ERROR("NBHttp2Parser, expected more octets for HEADERS priority-hearder.\n");
				r = FALSE;
			} else {
				STNBHttp2FrameHeadersPriority pp;
				BYTE* dstId = (BYTE*)&pp.dependStreamId;
				NBMemory_setZeroSt(pp, STNBHttp2FrameHeadersPriority);
				//E-Flag and streamId
				pp.dependIsExclusive = ((buff[iPos] & 0x80) != 0);
				dstId[3] = (buff[iPos++] & ~0x80);
				dstId[2] = buff[iPos++];
				dstId[1] = buff[iPos++];
				dstId[0] = buff[iPos++];
				//Weight
				pp.priorityWeight = buff[iPos++];
				if(dstPriorityParam != NULL){
					NBMemory_copy(dstPriorityParam, &pp, sizeof(pp));
				}
			}
		}
	}
	//Data
	if(r){
		if((iPos + paddingLen) > buffSz){
			PRINTF_ERROR("NBHttp2Parser, DATA padding is greather than buffSize.\n");
			r = FALSE;
		} else {
			const UI32 dataSz = (buffSz - iPos - paddingLen);
			NBString_concatBytes(dst, (const char*)&buff[iPos], dataSz);
			iPos += dataSz; NBASSERT(iPos <= buffSz)
		}
	}
	//Validate padding (must be zero)
	if(r){
		while(iPos < buffSz){
			if(buff[iPos] != 0){
				PRINTF_ERROR("NBHttp2Parser, DATA padding must be zeros.\n");
				r = FALSE;
				break;
			}
			iPos++;
		}
	}
	return r;
}


//Goaway frame

BOOL NBHttp2Parser_buffToGoAway(const STHttp2FrameHead* head, const void* pBuff, const UI32 buffSz, STNBHttp2GoAway* dstGoAway, STNBString* dstAditionalDebugData){
	BOOL r = TRUE;
	const BYTE* buff = (const BYTE*)pBuff;
	UI32 iPos = 0;
	//Last stream Id
	if(r){
		UI32 lstStreamId = 0;
		BYTE* ptr = (BYTE*)&lstStreamId;
		if((iPos + 4) > buffSz){
			PRINTF_ERROR("NBHttp2Parser, expected more octets for GOAWAY lastStreamId.\n");
			r = FALSE;
		} else {
			const BOOL isR = ((buff[iPos] & 0x80) != 0);
			ptr[3] = buff[iPos++] & ~0x80;
			ptr[2] = buff[iPos++];
			ptr[1] = buff[iPos++];
			ptr[0] = buff[iPos++];
			if(dstGoAway != NULL){
				dstGoAway->isR = isR;
				dstGoAway->lastStreamId = lstStreamId;
			}
		}
	}
	//Error code
	if(r){
		UI32 errCode = 0;
		BYTE* ptr = (BYTE*)&errCode;
		if((iPos + 4) > buffSz){
			PRINTF_ERROR("NBHttp2Parser, expected more octets for GOAWAY errCode.\n");
			r = FALSE;
		} else {
			ptr[3] = buff[iPos++];
			ptr[2] = buff[iPos++];
			ptr[1] = buff[iPos++];
			ptr[0] = buff[iPos++];
			if(dstGoAway != NULL){
				dstGoAway->errorCode = errCode;
			}
		}
	}
	//Aditional debug data
	if(iPos < buffSz){
		if(dstAditionalDebugData != NULL){
			NBString_concatBytes(dstAditionalDebugData, (const char*)&buff[iPos], (buffSz - iPos));
		}
	}
	return r;
}
