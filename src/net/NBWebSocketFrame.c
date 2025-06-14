
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBWebSocketFrame.h"
//
#include "nb/core/NBStruct.h"

//-----------------
//-- WebSocket - RFC6455
//-- https://datatracker.ietf.org/doc/html/rfc6455
//-----------------
/*
      0                   1                   2                   3
	  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	 +-+-+-+-+-------+-+-------------+-------------------------------+
	 |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
	 |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
	 |N|V|V|V|       |S|             |   (if payload len==126/127)   |
	 | |1|2|3|       |K|             |                               |
	 +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
	 |     Extended payload length continued, if payload len == 127  |
	 + - - - - - - - - - - - - - - - +-------------------------------+
	 |                               |Masking-key, if MASK set to 1  |
	 +-------------------------------+-------------------------------+
	 | Masking-key (continued)       |          Payload Data         |
	 +-------------------------------- - - - - - - - - - - - - - - - +
	 :                     Payload Data continued ...                :
	 + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
	 |                     Payload Data continued ...                |
	 +---------------------------------------------------------------+
*/

//NBWebSocketFrameLoadState

UI32 NBWebSocketFrame_feedByte0_(STNBWebSocketFrameLoadState* obj, const BYTE* data, const BYTE* dataAfterEnd, STNBWebSocketFrame* dst);
UI32 NBWebSocketFrame_feedByte1_(STNBWebSocketFrameLoadState* obj, const BYTE* data, const BYTE* dataAfterEnd, STNBWebSocketFrame* dst);
UI32 NBWebSocketFrame_feedPayLen_(STNBWebSocketFrameLoadState* obj, const BYTE* data, const BYTE* dataAfterEnd, STNBWebSocketFrame* dst);
UI32 NBWebSocketFrame_feedMask_(STNBWebSocketFrameLoadState* obj, const BYTE* data, const BYTE* dataAfterEnd, STNBWebSocketFrame* dst);
UI32 NBWebSocketFrame_feedPayData_(STNBWebSocketFrameLoadState* obj, const BYTE* data, const BYTE* dataAfterEnd, STNBWebSocketFrame* dst);

void NBWebSocketFrameLoadState_init(STNBWebSocketFrameLoadState* obj){
	NBMemory_setZeroSt(*obj, STNBWebSocketFrameLoadState);
	obj->method = NBWebSocketFrame_feedByte0_;
	//IF_NBASSERT(NBString_initWithSz(&obj->dbg.bytesFedStr, 0, 1024, 0.1f);)
}

void NBWebSocketFrameLoadState_release(STNBWebSocketFrameLoadState* obj){
	//IF_NBASSERT(NBString_release(&obj->dbg.bytesFedStr);)
}

void NBWebSocketFrameLoadState_reset(STNBWebSocketFrameLoadState* obj){
	//IF_NBASSERT(STNBString dbgBytesFedStrBkp = obj->dbg.bytesFedStr;)
	NBMemory_setZeroSt(*obj, STNBWebSocketFrameLoadState);
	//reinit
	obj->method = NBWebSocketFrame_feedByte0_;
	//{
		//IF_NBASSERT(obj->dbg.bytesFedStr = dbgBytesFedStrBkp;)
		//IF_NBASSERT(NBString_empty(&obj->dbg.bytesFedStr);)
	//}
}

UI32 NBWebSocketFrame_feedByte0_(STNBWebSocketFrameLoadState* obj, const BYTE* data, const BYTE* dataAfterEnd, STNBWebSocketFrame* dst){
	NBASSERT(data < dataAfterEnd) //optimize, should not be called if buffer is empty
	NBASSERT(dst != NULL)
	dst->isFin	= (*data & 0x80);
	dst->rsv1	= (*data & 0x40);
	dst->rsv2	= (*data & 0x20);
	dst->rsv3	= (*data & 0x10);
	dst->opCode = (ENNBWebSocketFrameOpCode)(*data & 0xF);
	obj->method = NBWebSocketFrame_feedByte1_;
	return 1;
}

UI32 NBWebSocketFrame_feedByte1_(STNBWebSocketFrameLoadState* obj, const BYTE* data, const BYTE* dataAfterEnd, STNBWebSocketFrame* dst){
	NBASSERT(data < dataAfterEnd) //optimize, should not be called if buffer is empty
	NBASSERT(dst != NULL)
	dst->mask.isPresent		= (*data & 0x80);
	dst->pay.length			= (*data & 0x7F);
	obj->payload.bytesFed	= 0;
	if(dst->pay.length == 127){
		//8 bytes len
		obj->payLen.bytesExpect = 8;
		obj->method			= NBWebSocketFrame_feedPayLen_;
	} else if(dst->pay.length == 126){
		//2 bytes len
		obj->payLen.bytesExpect = 2;
		obj->method			= NBWebSocketFrame_feedPayLen_;
	} else {
		if(dst->pay.data != NULL){
			NBMemory_free(dst->pay.data);
			dst->pay.data	= NULL;
		}
		if(dst->pay.length > 0){
			dst->pay.data	= NBMemory_alloc(dst->pay.length);
		}
		obj->payLen.bytesExpect = 0;
		obj->method			= (dst->mask.isPresent ? NBWebSocketFrame_feedMask_ : dst->pay.length  > 0 ? NBWebSocketFrame_feedPayData_ : NULL);
	}
	return 1;
}

UI32 NBWebSocketFrame_feedPayLen_(STNBWebSocketFrameLoadState* obj, const BYTE* data, const BYTE* dataAfterEnd, STNBWebSocketFrame* dst){
	NBASSERT(data < dataAfterEnd) //optimize, should not be called if buffer is empty
	NBASSERT(dst != NULL)
	NBASSERT(obj->payLen.bytesExpect == 2 || obj->payLen.bytesExpect == 8) //should be one or other
	const BYTE* dataStart = data;
	if(obj->payLen.bytesExpect == 2){
		while(obj->payLen.bytesFed < obj->payLen.bytesExpect && data < dataAfterEnd){
			if(obj->payLen.bytesFed == 0){
				dst->pay.length = (UI64)*data << 8;
			} else {
				dst->pay.length |= (UI64)*data;
			}
			//next
			obj->payLen.bytesFed++;
			data++;
		}
	} else {
		NBASSERT(obj->payLen.bytesExpect == 8)
		while(obj->payLen.bytesFed < obj->payLen.bytesExpect && data < dataAfterEnd){
			switch (obj->payLen.bytesFed) {
				case 0: dst->pay.length = (UI64)*data << 56; break;
				case 1: dst->pay.length |= (UI64)*data << 48; break;
				case 2: dst->pay.length |= (UI64)*data << 40; break;
				case 3: dst->pay.length |= (UI64)*data << 32; break;
				case 4: dst->pay.length |= (UI64)*data << 24; break;
				case 5: dst->pay.length |= (UI64)*data << 16; break;
				case 6: dst->pay.length |= (UI64)*data << 8; break;
				case 7: dst->pay.length |= (UI64)*data; break;
#				ifdef NB_CONFIG_INCLUDE_ASSERTS
				default:
					NBASSERT(FALSE) 
					break;
#				endif
			}
			//next
			obj->payLen.bytesFed++;
			data++;
		}
	}
	if(obj->payLen.bytesFed == obj->payLen.bytesExpect){
		if(dst->pay.data != NULL){
			NBMemory_free(dst->pay.data);
			dst->pay.data = NULL;
		}
		if(dst->pay.length > 0){
			dst->pay.data = NBMemory_alloc(dst->pay.length);
		}
		obj->method = (dst->mask.isPresent ? NBWebSocketFrame_feedMask_ : dst->pay.length  > 0 ? NBWebSocketFrame_feedPayData_ : NULL);
	}
	return (UI32)(data - dataStart);
}

UI32 NBWebSocketFrame_feedMask_(STNBWebSocketFrameLoadState* obj, const BYTE* data, const BYTE* dataAfterEnd, STNBWebSocketFrame* dst){
	NBASSERT(data < dataAfterEnd) //optimize, should not be called if buffer is empty
	NBASSERT(dst != NULL)
	NBASSERT(dst->mask.isPresent) //optimize, should not be called only if necesary
	const BYTE* dataStart = data;
	while(obj->maskKey.bytesFed < 4 && data < dataAfterEnd){
		dst->mask.value.c[obj->maskKey.bytesFed++] = *(data++); 
	}
	if(obj->maskKey.bytesFed == 4){
		obj->method = (dst->pay.length  > 0 ? NBWebSocketFrame_feedPayData_ : NULL);
	}
	return (UI32)(data - dataStart);
}

UI32 NBWebSocketFrame_feedPayData_(STNBWebSocketFrameLoadState* obj, const BYTE* data, const BYTE* dataAfterEnd, STNBWebSocketFrame* dst){
	NBASSERT(data < dataAfterEnd) //optimize, should not be called if buffer is empty
	NBASSERT(dst != NULL)
	NBASSERT(obj->payload.bytesFed < dst->pay.length) //optimize, should not be called only if necesary
	NBASSERT(dst->pay.data != NULL) //buffer shoukld be created by previous feed-call
	const UI64 bytesRemain	= (UI64)(dst->pay.length - obj->payload.bytesFed);
	const UI64 bytesAvail	= (UI64)(dataAfterEnd - data);
	const UI64 bytesRead	= (bytesRemain < bytesAvail ? bytesRemain : bytesAvail);
	if(dst->mask.isPresent){
		//masked payload
		BYTE* d = (BYTE*)dst->pay.data;
		const UI64 fedAfterEnd = obj->payload.bytesFed + bytesRead;
		const UI8* m = dst->mask.value.c;
		while(obj->payload.bytesFed < fedAfterEnd){
			//j                   = i MOD 4
			//transformed-octet-i = original-octet-i XOR masking-key-octet-j
			d[obj->payload.bytesFed] = (*(data++) ^ m[obj->payload.bytesFed % 4]);
			obj->payload.bytesFed++;
		}
	} else {
		//unmasked payload
		NBMemory_copy((BYTE*)dst->pay.data + obj->payload.bytesFed, data, bytesRead);
		obj->payload.bytesFed += bytesRead;
	}
	NBASSERT(obj->payload.bytesFed <= dst->pay.length)
	if(obj->payload.bytesFed >= dst->pay.length){
		obj->method			= NULL;
	}
	return (UI32)bytesRead;
}

//NBWebSocketFrame

void NBWebSocketFrame_init(STNBWebSocketFrame* obj){
	NBMemory_setZeroSt(*obj, STNBWebSocketFrame);
}

void NBWebSocketFrame_release(STNBWebSocketFrame* obj){
	if(obj->pay.data != NULL){
		NBMemory_free(obj->pay.data);
		obj->pay.data = NULL;
		obj->pay.length = 0;
	}
	if(obj->loadState != NULL){
		NBWebSocketFrameLoadState_release(obj->loadState);
		NBMemory_free(obj->loadState);
		obj->loadState = NULL;
	}
}

//

BOOL NBWebSocketFrame_isFinalFrame(const STNBWebSocketFrame* obj){
	return obj->isFin;
}

//

void NBWebSocketFrame_feedStart(STNBWebSocketFrame* obj){
	if(obj->loadState != NULL){
		NBWebSocketFrameLoadState_reset(obj->loadState);
	} else {
		obj->loadState = NBMemory_allocType(STNBWebSocketFrameLoadState);
		NBWebSocketFrameLoadState_init(obj->loadState);
	}
}

UI32 NBWebSocketFrame_feedBytes(STNBWebSocketFrame* obj, const void* pData, const UI32 dataSz){
	UI32 r = 0;
	if(obj->loadState != NULL && !obj->loadState->errFnd && obj->loadState->method != NULL){
		const BYTE* data = (const BYTE*)pData;
		const BYTE* dataStart = data;
		const BYTE* dataAfterEnd = data + dataSz;
		while(data < dataAfterEnd && obj->loadState->method != NULL){
			data += (*obj->loadState->method)(obj->loadState, data, dataAfterEnd, obj);
		}
		NBASSERT(dataStart <= data && data <= dataAfterEnd)
		if(!obj->loadState->errFnd){
			r = (UI32)(data - dataStart);
		}
		//IF_NBASSERT(NBString_concatBytes(&obj->loadState->dbg.bytesFedStr, dataStart, (UI32)(data - dataStart));)
	}
	return r;
}

BOOL NBWebSocketFrame_feedIsCompleted(const STNBWebSocketFrame* obj){
	return (obj->loadState != NULL && obj->loadState->method == NULL);
}

#ifdef NB_CONFIG_INCLUDE_ASSERTS
/*void NBWebSocketFrame_concatFedStrHex(const STNBWebSocketFrame* obj, const UI32 bytesPerGrp, const UI32 bytesPerLine, STNBString* dst){
	if(obj->loadState != NULL && bytesPerGrp > 0 && bytesPerLine > 0){
		STNBWebSocketFrameLoadState* loadState = obj->loadState;
		const char hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
		NBASSERT((sizeof(hex) / sizeof(hex[0])) == 16)
		const BYTE* b = (const BYTE*)loadState->dbg.bytesFedStr.str;
		UI32 i = 0; const UI32 len = loadState->dbg.bytesFedStr.length;
		while(i < len){
			if((i % bytesPerGrp) == 0 || (i % bytesPerLine) == 0){
				NBString_concatBytes(dst, "0x", 2);
			}
			NBString_concatByte(dst, hex[(*b >> 4) & 0xF]);
			NBString_concatByte(dst, hex[*b & 0xF]);
			//
			b++; i++;
			//
			if((i % bytesPerLine) == 0){
				NBString_concatByte(dst, '\n');
			} else if((i % bytesPerGrp) == 0){
				NBString_concatByte(dst, ' ');
			}
		}
		if((i % bytesPerLine) != 0){
			NBString_concatByte(dst, '\n');
		}
	}
}*/
#endif

//

BOOL NBWebSocketFrame_concat(const STNBWebSocketFrame* obj, STNBString* dst){
	BOOL r = FALSE;
	if(dst != NULL && obj->opCode >= 0 && obj->opCode < ENNBWebSocketFrameOpCode_Count){
		BYTE hdr[14];
		const UI32 hdsSz = NBWebSocketFrame_writeHeader(obj->opCode, obj->mask.isPresent ? obj->mask.value.u32 : 0, (obj->pay.length != 0 && obj->pay.data != NULL ? obj->pay.length : 0), (obj->opCode != ENNBWebSocketFrameOpCode_Continuation), obj->isFin, hdr);
		if(hdsSz > 0){
			NBString_concatBytes(dst, hdr, hdsSz);
		}
		if(obj->pay.length != 0 && obj->pay.data != NULL){
			if(!obj->mask.isPresent){
				NBString_concatBytes(dst, (const char*)obj->pay.data, (UI32)obj->pay.length);
			} else {
				UI64 i = 0; const BYTE* d = (const BYTE*)obj->pay.data;
				const BYTE* m = obj->mask.value.c;
				NBString_increaseBuffer(dst, (UI32)obj->pay.length);
				for(i = 0; i < obj->pay.length; i++){
					//j                   = i MOD 4
					//transformed-octet-i = original-octet-i XOR masking-key-octet-j
					NBString_concatByte(dst, (d[i] ^ m[i % 4]));
				}
			}
			
		}
		r = TRUE;
	}
	return r;
}

void NBWebSocketFrame_concatUnmaskedData(const STNBWebSocketFrame* obj, STNBString* dst){
	if(obj->pay.length != 0 && obj->pay.data != NULL){
		NBString_concatBytes(dst, (const char*)obj->pay.data, (UI32)obj->pay.length);
	}
}

BOOL NBWebSocketFrame_concatWithData(const ENNBWebSocketFrameOpCode opCode, const UI32 maskKey, const void* data, const UI64 dataSz, const BOOL isMsgFirstFrame, const BOOL isMsgLastFrame, STNBString* dst){
	BYTE hdr[14];
	const UI32 hdsSz = NBWebSocketFrame_writeHeader(opCode, maskKey, (data != NULL && dataSz > 0 ? dataSz : 0), isMsgFirstFrame, isMsgLastFrame, hdr);
	if(hdsSz > 0){
		NBString_concatBytes(dst, hdr, hdsSz);
	}
	if(data != NULL && dataSz > 0){
		if(maskKey == 0){
			NBString_concatBytes(dst, (const char*)data, (UI32)dataSz);
		} else {
			UI64 i = 0; const BYTE* d = (const BYTE*)data;
			const BYTE* m = (const BYTE*)&maskKey;
			NBString_increaseBuffer(dst, (UI32)dataSz);
			for(i = 0; i < dataSz; i++){
				//j                   = i MOD 4
				//transformed-octet-i = original-octet-i XOR masking-key-octet-j
				NBString_concatByte(dst, (d[i] ^ m[i % 4]));
			}
		}
	}
	return TRUE;
}

UI32 NBWebSocketFrame_writeHeader(const ENNBWebSocketFrameOpCode opCode, const UI32 maskKey, const UI64 dataSz, const BOOL isMsgFirstFrame, const BOOL isMsgLastFrame, void* dst14){ //basic header is 14-bytes max
	BYTE* dst = (BYTE*)dst14;
	if(dst != NULL && opCode > 0 && opCode < ENNBWebSocketFrameOpCode_Count){
		dst[0] = (isMsgLastFrame ? 0x80 : 0x0) | (isMsgFirstFrame ? opCode : ENNBWebSocketFrameOpCode_Continuation);
		dst[1] = (maskKey != 0 ? 0x80 : 0x0) | (BYTE)(dataSz < 126 ? dataSz : dataSz <= 0xFFFF ? 126 : 127);
		dst += 2;
		if(dataSz > 125){
			if(dataSz <= 0xFFFF){
				dst[0] = ((dataSz >> 8) & 0xFF);
				dst[1] = (dataSz & 0xFF);
				dst += 2;
			} else {
				dst[0] = ((dataSz >> 56) & 0xFF);
				dst[1] = ((dataSz >> 48) & 0xFF);
				dst[2] = ((dataSz >> 40) & 0xFF);
				dst[3] = ((dataSz >> 32) & 0xFF);
				dst[4] = ((dataSz >> 24) & 0xFF);
				dst[5] = ((dataSz >> 16) & 0xFF);
				dst[6] = ((dataSz >> 8) & 0xFF);
				dst[7] = (dataSz & 0xFF);
				dst += 8;
			}
		}
		if(maskKey != 0){
			STNBWebSocketMask m;
			NBMemory_setZeroSt(m, STNBWebSocketMask);
			m.u32 = maskKey; dst[0] = m.c[0]; dst[1] = m.c[1]; dst[2] = m.c[2]; dst[3] = m.c[3];
			dst += 4;
		}
	}
	return (UI32)(dst - (BYTE*)dst14);
}
