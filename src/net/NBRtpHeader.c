
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBRtpHeader.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBString.h"


//

void NBRtpHeader_ntoh16(void* ptr16){
	BYTE* data = (BYTE*)ptr16;
	const BYTE b = data[0];
	data[0] = data[1];
	data[1] = b;
}

void NBRtpHeader_ntoh32(void* ptr32){
	BYTE* data = (BYTE*)ptr32;
	BYTE b;
	//
	b		= data[0];
	data[0] = data[3];
	data[3] = b;
	//
	b		= data[1];
	data[1] = data[2];
	data[2] = b;
}

//

BOOL NBRtpHdrBasic_parse(STNBRtpHdrBasic* obj, const void* pData, const UI32 dataSz){
	BOOL r = FALSE;
	if(pData != NULL && dataSz >= 12){
		BYTE c;
		const BYTE* data = (const BYTE*)pData;
		const BYTE* dataAfterEnd = data + dataSz;
		STNBRtpHdrBasic localResult;
		if(obj == NULL) obj = &localResult;
		//Header - byte1
		c = *(data++);
		obj->head.version		= ((c >> 6) & 0x3);
		obj->head.havePadding	= ((c >> 5) & 0x1);
		obj->head.haveExtension = ((c >> 4) & 0x1);
		obj->head.csrcsCount	= (c & 0xF);
		if(obj->head.version == 2){
			//Header - byte2
			c = *(data++);
			obj->head.isMarker		= ((c >> 7) & 0x1);
			obj->head.payloadType	= (c & 0x7F);
			//Header - bytes3-12
			NBMemory_copy(&obj->head.seqNum, data, 10);
			NBRtpHeader_ntoh16(&obj->head.seqNum);
			NBRtpHeader_ntoh32(&obj->head.timestamp);
			NBRtpHeader_ntoh32(&obj->head.ssrc);
			data += 10;
			r = TRUE;
			//Header csrcs
			if(r && obj->head.csrcsCount > 0){
				if((sizeof(UI32) * obj->head.csrcsCount) > (dataAfterEnd - data)){
					r = FALSE;
				} else {
					data += (sizeof(UI32) * obj->head.csrcsCount);
				}
			}
			//Header extension
			if(r && obj->head.haveExtension){
				if(4 > (dataAfterEnd - data)){
					r = FALSE;
				} else {
					STNBRtpPacketHeadExt extDef;
					NBMemory_copy(&extDef, data, 4);
					NBRtpHeader_ntoh16(&extDef.val0);
					NBRtpHeader_ntoh16(&extDef.length);
					data += 4;
					//Header extension data
					if((extDef.length * sizeof(UI32)) > (dataAfterEnd - data)){
						r = FALSE;
					} else {
						data += extDef.length * sizeof(UI32);
					}
				}
			}
			//Payload
			if(r){
				obj->payload.iStart	= (UI32)(data - (const BYTE*)pData);
				obj->payload.sz		= (UI32)(dataAfterEnd - data);
				NBASSERT((data + obj->payload.sz) == dataAfterEnd)
				//Remove pading
				if(obj->payload.sz > 0 && obj->head.havePadding){
					const UI8 paddingCount = data[obj->payload.sz - 1];
					if(paddingCount > obj->payload.sz){
						r = FALSE;
					} else {
						obj->payload.sz -= paddingCount;
						NBASSERT((data + obj->payload.sz + paddingCount) == dataAfterEnd)
					}
				}
				//PRINTF_INFO("RTP, header version(%u) havePadding(%s) haveExtension(%s, %d bytes) csrcsCount(%u) isMarker(%s) payloadType(%u) seqNum(%u) timestamp(%u) ssrc(%u) payload(%d bytes).\n", obj->head.def.version, obj->head.def.havePadding ? "YES" : "NO", obj->head.def.haveExtension ? "YES" : "NO", obj->headExt.def.length, obj->head.def.csrcsCount, obj->head.def.isMarker ? "YES" : "NO", obj->head.def.payloadType, obj->head.def.seqNum, obj->head.def.timestamp, obj->head.def.ssrc, obj->payload.sz);
			}
		}
	}
	return r;
}

//

void NBRtpHeader_init(STNBRtpHeader* obj){
	NBMemory_setZeroSt(*obj, STNBRtpHeader);
}

void NBRtpHeader_release(STNBRtpHeader* obj){
	//header
	{
		if(obj->head.csrcs != NULL){
			if(!(obj->preResigned && obj->preResignTaken)){
				NBMemory_free(obj->head.csrcs);
			}
			obj->head.csrcs = NULL;
		}
		obj->head.csrcsSz = 0;
	}
	//Pre-resign
	obj->preResigned = obj->preResignTaken = FALSE;
}

//Optimization (reduces locks and internal data cloning)

void NBRtpHeader_preResign(STNBRtpHeader* obj){
	obj->preResigned = TRUE;
}

//

BOOL NBRtpHeader_parse(STNBRtpHeader* obj, const void* pData, const UI32 dataSz){
	BOOL r = FALSE;
	if(pData != NULL && dataSz >= 12){
		BYTE c;
		const BYTE* data = (const BYTE*)pData;
		const BYTE* dataAfterEnd = data + dataSz;
		STNBRtpHeader localResult;
		if(obj == NULL) obj = &localResult;
		//reset
		{
			//Header
			{
				if(obj->head.csrcs != NULL){
					if(!(obj->preResigned && obj->preResignTaken)){
						NBMemory_free(obj->head.csrcs);
					}
					obj->head.csrcs = NULL;
				}
				obj->head.csrcsSz = 0;
			}
			//Header extension
			{
				obj->headExt.iStart = obj->headExt.sz = 0; 
			}
			//Payload
			{
				obj->payload.iStart = obj->payload.sz = 0;
			}
			//Pre-resign
			obj->preResigned = obj->preResignTaken = FALSE;
		}
		//Header - byte1
		c = *(data++);
		obj->head.def.version		= ((c >> 6) & 0x3);
		obj->head.def.havePadding	= ((c >> 5) & 0x1);
		obj->head.def.haveExtension = ((c >> 4) & 0x1);
		obj->head.def.csrcsCount	= (c & 0xF);
		if(obj->head.def.version == 2){
			//Header - byte2
			c = *(data++);
			obj->head.def.isMarker		= ((c >> 7) & 0x1);
			obj->head.def.payloadType	= (c & 0x7F);
			//Header - bytes3-12
			NBMemory_copy(&obj->head.def.seqNum, data, 10);
			NBRtpHeader_ntoh16(&obj->head.def.seqNum);
			NBRtpHeader_ntoh32(&obj->head.def.timestamp);
			NBRtpHeader_ntoh32(&obj->head.def.ssrc);
			data += 10;
			r = TRUE;
			//Header csrcs
			if(r && obj->head.def.csrcsCount > 0){
				if((sizeof(UI32) * obj->head.def.csrcsCount) > (dataAfterEnd - data)){
					r = FALSE;
				} else {
					UI8 i;
					//Release
					obj->head.csrcs = NBMemory_allocTypes(UI32, obj->head.def.csrcsCount);
					for(i = 0; i < obj->head.def.csrcsCount; i++){
						obj->head.csrcs[i] = *((const UI32*)data);
						NBRtpHeader_ntoh32(&obj->head.csrcs[i]);
						data += sizeof(UI32);
					}
				}
			}
			//Header extension
			if(r && obj->head.def.haveExtension){
				if(4 > (dataAfterEnd - data)){
					r = FALSE;
				} else {
					NBMemory_copy(&obj->headExt.def, data, 4);
					NBRtpHeader_ntoh16(&obj->headExt.def.val0);
					NBRtpHeader_ntoh16(&obj->headExt.def.length);
					data += 4;
					//Header extension data
					if((obj->headExt.def.length * sizeof(UI32)) > (dataAfterEnd - data)){
						r = FALSE;
					} else {
						obj->headExt.iStart = (UI32)(data - (const BYTE*)pData);
						obj->headExt.sz		= obj->headExt.def.length * sizeof(UI32);
						data += obj->headExt.sz;
					}
				}
			}
			//Payload
			if(r){
				obj->payload.iStart	= (UI32)(data - (const BYTE*)pData);
				obj->payload.sz		= (UI32)(dataAfterEnd - data);
				NBASSERT((data + obj->payload.sz) == dataAfterEnd)
				//Remove pading
				if(obj->payload.sz > 0 && obj->head.def.havePadding){
					const UI8 paddingCount = data[obj->payload.sz - 1];
					if(paddingCount > obj->payload.sz){
						r = FALSE;
					} else {
						obj->payload.sz -= paddingCount;
						NBASSERT((data + obj->payload.sz + paddingCount) == dataAfterEnd)
					}
				}
				//PRINTF_INFO("RTP, header version(%u) havePadding(%s) haveExtension(%s, %d bytes) csrcsCount(%u) isMarker(%s) payloadType(%u) seqNum(%u) timestamp(%u) ssrc(%u) payload(%d bytes).\n", obj->head.def.version, obj->head.def.havePadding ? "YES" : "NO", obj->head.def.haveExtension ? "YES" : "NO", obj->headExt.def.length, obj->head.def.csrcsCount, obj->head.def.isMarker ? "YES" : "NO", obj->head.def.payloadType, obj->head.def.seqNum, obj->head.def.timestamp, obj->head.def.ssrc, obj->payload.sz);
			}
		}
	}
	return r;
}

BOOL NBRtpHeader_copy(STNBRtpHeader* obj, const STNBRtpHeader* other){
	BOOL r = FALSE;
	if(obj != NULL && other != NULL){
		//reset
		{
			//Header
			{
				if(obj->head.csrcs != NULL){
					if(!(obj->preResigned && obj->preResignTaken)){
						NBMemory_free(obj->head.csrcs);
					}
					obj->head.csrcs = NULL;
				}
				obj->head.csrcsSz = 0;
			}
			//Pre-resign
			obj->preResigned = obj->preResignTaken = FALSE;
		}
		//apply
		*obj = *other;
		//clone data
		if(other->preResigned && !other->preResignTaken){
			//just keep resigned data
			((STNBRtpHeader*)other)->preResignTaken	= TRUE;
			obj->preResigned		= obj->preResignTaken = FALSE;
		} else {
			//clone data
			obj->head.csrcs			= NULL;
			obj->head.csrcsSz		= 0;
			if(other->head.csrcs != NULL && other->head.csrcsSz > 0){
				obj->head.csrcs	= NBMemory_allocTypes(UI32, other->head.csrcsSz);
				obj->head.csrcsSz = other->head.csrcsSz;
				NBMemory_copy(obj->head.csrcs, other->head.csrcs, sizeof(UI32) * other->head.csrcsSz);
			}
		}
		//
		r = TRUE;
	}
	return r;
}
