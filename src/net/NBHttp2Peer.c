
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBHttp2Peer.h"
//
#include "nb/core/NBStruct.h"
#include "nb/net/NBHttpBuilder.h"

void NBHttp2Peer_init(STNBHttp2Peer* obj){
	NBHttp2Parser_settingsSetDefaults(&obj->settings);
	NBHttp2Hpack_init(&obj->hpackLocal);
	NBHttp2Hpack_init(&obj->hpackRemote);
}

void NBHttp2Peer_release(STNBHttp2Peer* obj){
	NBHttp2Hpack_release(&obj->hpackLocal);
	NBHttp2Hpack_release(&obj->hpackRemote);
}

BOOL NBHttp2Peer_sendSettings(STNBHttp2Peer* obj, STNBSslRef ssl, const UI32 settingsMask){
	BOOL r = FALSE;
	STNBString payBuff, frameBuff;
	NBString_initWithSz(&payBuff, 4096, 4096, 0.10f);
	NBString_initWithSz(&frameBuff, 4096, 4096, 0.10f);
	if(!NBHttp2Parser_settingsToBuff(&obj->settings, settingsMask, &payBuff)){
		PRINTF_ERROR("NBHttp2Peer, NBHttp2Parser_settingsToBuff failed.\n");
	} else {
		BYTE buff9[9];
		STHttp2FrameHead frame;
		NBMemory_setZeroSt(frame, STHttp2FrameHead);
		frame.len		= payBuff.length;	//3 bytes
		frame.type		= ENNBHttp2FrameType_SETTINGS;	//ENNBHttp2FrameType
		frame.flag		= 0;
		frame.streamId	= 0; //allways zero
		NBHttp2Parser_frameHeadToBuff(&frame, buff9);
		//Send
		NBString_empty(&frameBuff);
		NBString_concatBytes(&frameBuff, (const char*)buff9, sizeof(buff9));
		NBString_concatBytes(&frameBuff, payBuff.str, payBuff.length);
		//Send
		if(NBSsl_write(ssl, frameBuff.str, frameBuff.length) != frameBuff.length){
			PRINTF_ERROR("NBHttp2Peer, NBHttp2Peer_sendSettings failed (%d bytes payload, %d bytes total).\n", payBuff.length, frameBuff.length);
		} else {
			//PRINTF_INFO("NBHttp2Peer, NBHttp2Peer_sendSettings success (%d bytes payload, %d bytes total).\n", payBuff.length, frameBuff.length);
			r = TRUE;
		}
	}
	NBString_release(&frameBuff);
	NBString_release(&payBuff);
	return r;
}

BOOL NBHttp2Peer_sendSettingsAck(STNBHttp2Peer* obj, STNBSslRef ssl){
	BOOL r = FALSE;
	STNBString frameBuff;
	NBString_initWithSz(&frameBuff, 9, 4096, 0.10f);
	{
		BYTE buff9[9];
		STHttp2FrameHead frame;
		NBMemory_setZeroSt(frame, STHttp2FrameHead);
		frame.len		= 0;	//3 bytes
		frame.type		= ENNBHttp2FrameType_SETTINGS;	//ENNBHttp2FrameType
		frame.flag		= ENNBHttp2FrameFlag_ACK;
		frame.streamId	= 0; //allways zero
		NBHttp2Parser_frameHeadToBuff(&frame, buff9);
		//Send
		NBString_empty(&frameBuff);
		NBString_concatBytes(&frameBuff, (const char*)buff9, sizeof(buff9));
		//Send
		if(NBSsl_write(ssl, frameBuff.str, frameBuff.length) != frameBuff.length){
			PRINTF_ERROR("NBHttp2Peer_sendSettings failed (%d bytes payload, %d bytes total).\n", 0, frameBuff.length);
		} else {
			//PRINTF_INFO("NBHttp2Peer_sendSettings success (%d bytes payload, %d bytes total).\n", 0, frameBuff.length);
			r = TRUE;
		}
	}
	NBString_release(&frameBuff);
	return r;
}

BOOL NBHttp2Peer_sendHeaders(STNBHttp2Peer* obj, const STNBHttpHeader* headers, STNBSslRef ssl, const UI32 streamId){
	BOOL r = FALSE;
	STNBString headBlock, payBuff, frameBuff;
	NBString_initWithSz(&headBlock, 4096, 4096, 0.10f);
	NBString_initWithSz(&payBuff, 4096, 4096, 0.10f);
	NBString_initWithSz(&frameBuff, 4096, 4096, 0.10f);
	if(!NBHttp2Hpack_encodeBlock(&obj->hpackRemote, headers, &headBlock)){
		PRINTF_ERROR("NBHttp2Peer, NBHttp2Hpack_encodeBlock failed.\n");
	} else {
		STHttp2FrameHead head;
		NBMemory_setZeroSt(head, STHttp2FrameHead);
		head.streamId	= streamId;
		if(!NBHttp2Parser_headersToBuff(headBlock.str, headBlock.length, 0, TRUE /*isEndOfHeaders*/, FALSE /*isEndOfStream*/, NULL, &head, &payBuff)){
			PRINTF_ERROR("NBHttp2Peer, NBHttp2Parser_headersToBuff failed.\n");
		} else {
			BYTE buff9[9];
			NBHttp2Parser_frameHeadToBuff(&head, buff9);
			//Send
			NBString_empty(&frameBuff);
			NBString_concatBytes(&frameBuff, (const char*)buff9, sizeof(buff9));
			NBString_concatBytes(&frameBuff, payBuff.str, payBuff.length);
			//Send
			if(NBSsl_write(ssl, frameBuff.str, frameBuff.length) != frameBuff.length){
				PRINTF_ERROR("NBHttp2Peer, send HEADERS failed (%d bytes payload, %d bytes total).\n", payBuff.length, frameBuff.length);
			} else {
				PRINTF_INFO("NBHttp2Peer, HEADERS sent (%d bytes payload, %d bytes total).\n", payBuff.length, frameBuff.length);
				r = TRUE;
			}
		}
	}
	NBString_release(&frameBuff);
	NBString_release(&payBuff);
	NBString_release(&headBlock);
	return r;
}

BOOL NBHttp2Peer_sendData(STNBHttp2Peer* obj, const void* data, const UI32 dataSz, STNBSslRef ssl, const UI32 streamId){
	BOOL r = FALSE;
	STNBString payBuff, frameBuff;
	NBString_initWithSz(&payBuff, 4096, 4096, 0.10f);
	NBString_initWithSz(&frameBuff, 4096, 4096, 0.10f);
	{
		STHttp2FrameHead head;
		NBMemory_setZeroSt(head, STHttp2FrameHead);
		head.streamId	= streamId;
		if(!NBHttp2Parser_dataToBuff(data, dataSz, 0, TRUE /*isEndOfStream*/, &head, &payBuff)){
			PRINTF_ERROR("NBHttp2Peer, NBHttp2Parser_dataToBuff failed.\n");
		} else {
			BYTE buff9[9];
			NBHttp2Parser_frameHeadToBuff(&head, buff9);
			//Send
			NBString_concatBytes(&frameBuff, (const char*)buff9, sizeof(buff9));
			NBString_concatBytes(&frameBuff, payBuff.str, payBuff.length);
			//Send
			if(NBSsl_write(ssl, frameBuff.str, frameBuff.length) != frameBuff.length){
				PRINTF_ERROR("NBHttp2Peer, send DATA failed (%d bytes payload, %d bytes total).\n", payBuff.length, frameBuff.length);
			} else {
				PRINTF_INFO("NBHttp2Peer, DATA sent (%d bytes payload, %d bytes total).\n", payBuff.length, frameBuff.length);
				r = TRUE;
			}
		}
	}
	NBString_release(&frameBuff);
	NBString_release(&payBuff);
	return r;
}

BOOL NBHttp2Peer_receiveSettings(STNBHttp2Peer* obj, STNBSslRef ssl){
	BOOL r = FALSE;
	STNBString payload;
	NBString_initWithSz(&payload, 0, 4096, 0.10f);
	{
		STHttp2FrameHead head;
		NBMemory_setZeroSt(head, STHttp2FrameHead);
		if(!NBHttp2Peer_receiveFrame(obj, ssl, &head, &payload)){
			PRINTF_ERROR("NBHttp2Peer, NBHttp2Peer_receiveFrame(SettingsAck) failed.\n");
		} else if(head.type != ENNBHttp2FrameType_SETTINGS){
			PRINTF_ERROR("NBHttp2Peer, expected SETTINGS frameType(%d) received(%d, %d bytes).\n", ENNBHttp2FrameType_SETTINGS, head.type, payload.length);
		} else if((head.flag & ENNBHttp2FrameFlag_ACK) != 0){
			PRINTF_ERROR("NBHttp2Peer, expected SETTINGS without ACK flag flags(%d, %d bytes).\n", head.flag, payload.length);
		} else if(!NBHttp2Peer_processFrame(obj, &head, payload.str, payload.length)){
			PRINTF_ERROR("NBHttp2Peer, could not process SETTINGS frame flags(%d bytes).\n", payload.length);
		} else {
			r = TRUE;
		}
	}
	NBString_release(&payload);
	return r;
}

BOOL NBHttp2Peer_receiveSettingsAck(STNBHttp2Peer* obj, STNBSslRef ssl){
	BOOL r = FALSE;
	STNBString payload;
	NBString_initWithSz(&payload, 0, 4096, 0.10f);
	{
		STHttp2FrameHead head;
		NBMemory_setZeroSt(head, STHttp2FrameHead);
		if(!NBHttp2Peer_receiveFrame(obj, ssl, &head, NULL)){
			PRINTF_ERROR("NBHttp2Peer, NBHttp2Peer_receiveFrame(SettingsAck) failed.\n");
		} else if(head.type != ENNBHttp2FrameType_SETTINGS){
			PRINTF_ERROR("NBHttp2Peer, expected SETTINGS frameType(%d) received(%d, %d bytes).\n", ENNBHttp2FrameType_SETTINGS, head.type, payload.length);
		} else if(head.len != 0){
			PRINTF_ERROR("NBHttp2Peer, expected empty SETTINGS frame received(%d bytes, %d bytes).\n", head.len, payload.length);
		} else if((head.flag & ENNBHttp2FrameFlag_ACK) == 0){
			PRINTF_ERROR("NBHttp2Peer, expected SETTINGS ACK flag flags(%d, %d bytes).\n", head.flag, payload.length);
		} else {
			r = TRUE;
		}
	}
	NBString_release(&payload);
	return r;
}

BOOL NBHttp2Peer_receiveFrame(STNBHttp2Peer* obj, STNBSslRef ssl, STHttp2FrameHead* dstHead, STNBString* dstPayload){
	BOOL r = FALSE;
	//Read frame
	{
		BYTE buff9[9];
		if(NBSsl_read(ssl, (char*)buff9, sizeof(buff9)) != sizeof(buff9)){
			PRINTF_ERROR("NBHttp2Peer, NBSsl_read(header) failed.\n");
		} else {
			STHttp2FrameHead head;
			NBMemory_setZeroSt(head, STHttp2FrameHead);
			NBHttp2Parser_buffToFrameHead(buff9, &head);
			if(dstHead != NULL){
				NBMemory_copy(dstHead, &head, sizeof(head));
			}
			if(head.len <= 0){
				r = TRUE;
			} else {
				BYTE* buff = (BYTE*)NBMemory_alloc(head.len);
				if(NBSsl_read(ssl, (char*)buff, head.len) != head.len){
					PRINTF_ERROR("NBHttp2Peer, NBSsl_read(payload) failed.\n");
				} else {
					if(dstPayload != NULL){
						NBString_concatBytes(dstPayload, (const char*)buff, head.len);
					}
					r = TRUE;
				}
				NBMemory_free(buff);
			}
		}
	}
	return r;
}

BOOL NBHttp2Peer_processFrame(STNBHttp2Peer* obj, const STHttp2FrameHead* head, const void* pay, const UI32 paySz){
	BOOL r = TRUE;
	NBASSERT(head->len == paySz)
	switch(head->type) {
		case ENNBHttp2FrameType_DATA:
			PRINTF_INFO("NBHttp2Peer, frame type(DATA) payload(%d bytes).\n", paySz);
#			ifdef NB_CONFIG_INCLUDE_ASSERTS
			{
				STNBString str;
				NBString_init(&str);
				NBString_concatBytes(&str, (const char*)pay, paySz);
				PRINTF_INFO("-->'%s'<--\n", str.str);
				NBString_release(&str);
			}
#			endif
			break;
		case ENNBHttp2FrameType_HEADERS:
			PRINTF_INFO("NBHttp2Peer, frame type(HEADERS) payload(%d bytes).\n", paySz);
#			ifdef NB_CONFIG_INCLUDE_ASSERTS
			{
				STNBHttp2FrameHeadersPriority pp;
				NBMemory_setZeroSt(pp, STNBHttp2FrameHeadersPriority);
				{
					STNBString headBlock;
					NBString_initWithSz(&headBlock, 4096, 4096, 0.10f);
					if(!NBHttp2Parser_buffToHeaders(head, pay, paySz, &pp, &headBlock)){
						PRINTF_ERROR("NBHttp2Peer, NBHttp2Parser_buffToHeaders failed.\n");
						r = FALSE;
					} else {
						STNBHttpHeader pairs;
						NBHttpHeader_init(&pairs);
						if(!NBHttp2Hpack_decodeBlock(&obj->hpackLocal, headBlock.str, headBlock.length, &pairs)){
							PRINTF_ERROR("NBHttp2Peer, NBHttp2Hpack_decodeBlock failed.\n");
							r = FALSE;
						} else {
							STNBHttpBuilder bldr;
							NBHttpBuilder_init(&bldr);
							{
								STNBString plain;
								NBString_initWithSz(&plain, 4096, 4096, 0.10f);
								NBHttpBuilder_addHeaderFieldsPlain(&bldr, &plain, &pairs);
								PRINTF_INFO("NBHttp2Peer, headers:\n-->%s<--.\n", plain.str);
								NBString_release(&plain);
							}
							NBHttpBuilder_release(&bldr);
						}
						NBHttpHeader_release(&pairs);
					}
					NBString_release(&headBlock);
				}
			}
#			endif
			break;
		case ENNBHttp2FrameType_PRIORITY:
			PRINTF_INFO("NBHttp2Peer, frame type(PRIORITY) payload(%d bytes).\n", paySz);
			break;
		case ENNBHttp2FrameType_RTS_STREAM:
			PRINTF_INFO("NBHttp2Peer, frame type(RTS_STREAM) payload(%d bytes).\n", paySz);
			break;
		case ENNBHttp2FrameType_SETTINGS:
			PRINTF_INFO("NBHttp2Peer, frame type(SETTINGS) payload(%d bytes).\n", paySz);
			if(head->streamId != 0){
				PRINTF_ERROR("NBHttp2Peer, setting streamId must be zero.\n");
				r = FALSE;
			} else {
				UI32 settMask = 0;
				STNBHttp2Settings sett;
				NBMemory_setZeroSt(sett, STNBHttp2Settings);
				NBHttp2Parser_settingsSetDefaults(&sett);
				if(!NBHttp2Parser_buffToSettings(pay, paySz, &sett, &settMask)){
					PRINTF_ERROR("NBHttp2Peer, NBHttp2Parser_buffToSettings failed.\n");
					r = FALSE;
				} else {
					if((settMask & ENNBHttp2Setting_HEADER_TABLE_SIZE) != 0){
						PRINTF_INFO("NBHttp2Peer, setting HEADER_TABLE_SIZE: %d.\n", sett.headerTableSz);
					}
					if((settMask & ENNBHttp2Setting_ENABLE_PUSH) != 0){
						PRINTF_INFO("NBHttp2Peer, setting ENABLE_PUSH: %d.\n", sett.enablePush);
					}
					if((settMask & ENNBHttp2Setting_MAX_CONCURRENT_STREAMS) != 0){
						PRINTF_INFO("NBHttp2Peer, setting MAX_CONCURRENT_STREAMS: %d.\n", sett.maxConcurrentStreams);
					}
					if((settMask & ENNBHttp2Setting_INITIAL_WINDOW_SIZE) != 0){
						PRINTF_INFO("NBHttp2Peer, setting INITIAL_WINDOW_SIZE: %d.\n", sett.initialWindowSz);
					}
					if((settMask & ENNBHttp2Setting_MAX_FRAME_SIZE) != 0){
						PRINTF_INFO("NBHttp2Peer, setting MAX_FRAME_SIZE: %d.\n", sett.maxFrameSz);
					}
					if((settMask & ENNBHttp2Setting_MAX_HEADER_LIST_SIZE) != 0){
						PRINTF_INFO("NBHttp2Peer, setting MAX_HEADER_LIST_SIZE: %d.\n", sett.maxHeaderListSz);
					}
				}
			}
			break;
		case ENNBHttp2FrameType_PUSH_PROMISE:
			PRINTF_INFO("NBHttp2Peer frame type(PUSH_PROMISE) payload(%d bytes).\n", paySz);
			break;
		case ENNBHttp2FrameType_PING:
			PRINTF_INFO("NBHttp2Peer, frame type(PING) payload(%d bytes).\n", paySz);
			break;
		case ENNBHttp2FrameType_GOAWAY:
			PRINTF_INFO("NBHttp2Peer, frame type(GOAWAY) payload(%d bytes).\n", paySz);
		{
			STNBString dbgData;
			STNBHttp2GoAway goAway;
			NBMemory_setZeroSt(goAway, STNBHttp2GoAway);
			NBString_init(&dbgData);
			if(!NBHttp2Parser_buffToGoAway(head, pay, paySz, &goAway, &dbgData)){
				PRINTF_ERROR("NBHttp2Peer, NBHttp2Parser_buffToGoAway failed.\n");
				r = FALSE;
			} else {
				PRINTF_INFO("NBHttp2Peer, goAway isR(%d) lstStreamId(%d) errCode(%d) dbgData(%d bytes, %s).\n", goAway.isR, goAway.lastStreamId, goAway.errorCode, dbgData.length, dbgData.str);
			}
			NBString_release(&dbgData);
		}
			break;
		case ENNBHttp2FrameType_WINDOW_UPDATE:
			PRINTF_INFO("NBHttp2Peer, frame type(WINDOW_UPDATE) payload(%d bytes).\n", paySz);
			break;
		case ENNBHttp2FrameType_CONTINUATION:
			PRINTF_INFO("NBHttp2Peer, frame type(CONTINUATION) payload(%d bytes).\n", paySz);
			break;
		default:
			PRINTF_INFO("NBHttp2Peer, frame type(UNKNOWN) payload(%d bytes).\n", paySz);
			break;
	}
	return r;
}
