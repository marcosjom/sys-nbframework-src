
#include "nb/NBFrameworkPch.h"
#include "nb/net/NBRtcpParser.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBString.h"


//Result

void NBRtcpParserResult_init(STNBRtcpParserResult* obj){
	NBMemory_setZeroSt(*obj, STNBRtcpParserResult);
}

void NBRtcpParserResult_release(STNBRtcpParserResult* obj){
	if(obj->packets != NULL){
		UI32 i; for(i = 0; i < obj->packetsSz; i++){
			STNBRtcpParserResultPacket* pp = &obj->packets[i];
			if(pp->data != NULL){
				switch (pp->head.type) {
					case ENRtcpPacketType_SR: //Packet sender report (SR)
						{
							STNBRtcpPacketSR* p = (STNBRtcpPacketSR*)pp->data;
							if(p->reports != NULL){
								NBMemory_free(p->reports);
								p->reports		= NULL;
								p->reportsSz	= 0;
							}
						}
						break;
					case ENRtcpPacketType_RR: //Packet receiver report (RR)
						{
							STNBRtcpPacketRR* p = (STNBRtcpPacketRR*)pp->data;
							if(p->reports != NULL){
								NBMemory_free(p->reports);
								p->reports		= NULL;
								p->reportsSz	= 0;
							}
						}
						break;
					case ENRtcpPacketType_SDES: //Packet source description item (SDES)
						{
							STNBRtcpPacketSDES* p = (STNBRtcpPacketSDES*)pp->data;
							if(p->chunks != NULL){
								UI32 i; for(i = 0; i < p->chunksSz; i++){
									STNBRtcpSrcDescChunck* chk = &p->chunks[i];
									if(chk->itms != NULL){
										UI32 i; for(i = 0; i < chk->itmsSz; i++){
											STNBRtcpSrcDescItm* itm = &chk->itms[i];
											if(itm->str != NULL){
												NBMemory_free(itm->str);
												itm->str	= NULL;
												itm->length	= 0;
											}
										}
										NBMemory_free(chk->itms);
										chk->itms	= NULL;
										chk->itmsSz	= 0;
									}
								}
								NBMemory_free(p->chunks);
								p->chunks	= NULL;
								p->chunksSz	= 0;
							}
						}
						break;
					case ENRtcpPacketType_BYE: //Goodbye RTCP Packet
						{
							STNBRtcpPacketBYE* p = (STNBRtcpPacketBYE*)pp->data;
							if(p->ssrcs != NULL){
								NBMemory_free(p->ssrcs);
								p->ssrcsSz = 0;
							}
							if(p->reason.str != NULL){
								NBMemory_free(p->reason.str);
								p->reason.str = NULL;
								p->reason.length = 0;
							}
						}
						break;
					default:
						//Logic error?
						break;
				}
				NBMemory_free(pp->data);
				pp->data = NULL;
			}
		}
		NBMemory_free(obj->packets);
		obj->packets	= NULL;
		obj->packetsSz	= 0;
	}
}

void NBRtcpParserResult_concat(const STNBRtcpParserResult* obj, STNBString* dst){
	if(obj != NULL && dst != NULL){
		if(obj->packetsSz <= 0){
			NBString_concatUI32(dst, obj->packetsSz); NBString_concat(dst, " RTCP packets.");
		} else {
			NBString_concatUI32(dst, obj->packetsSz); NBString_concat(dst, " RTCP packets:");
			if(obj->packets == NULL){
				NBString_concat(dst, "\n  NULL array.");
			} else {
				UI32 i; for(i = 0; i < obj->packetsSz; i++){
					STNBRtcpParserResultPacket* pp = &obj->packets[i];
					if(pp->data != NULL){
						//Header
						{
							const STNBRtcpPacketTypeDef* packTypeDef = NBRtcp_getPacketTypeDef(pp->head.type);
							NBString_concat(dst, "\n  Packet ");
							NBString_concatUI32(dst, i + 1);
							NBString_concat(dst, "/");
							NBString_concatUI32(dst, obj->packetsSz);
							NBString_concat(dst, " (");
							if(packTypeDef == NULL){
								NBString_concat(dst, "unexpected type");
							} else {
								
								NBString_concat(dst, packTypeDef->acronim);
								NBString_concat(dst, ", ");
								NBString_concat(dst, packTypeDef->desc);
							}
							{
								NBString_concat(dst, ", ver=");
								NBString_concatUI32(dst, pp->head.version);
								NBString_concat(dst, ", count=");
								NBString_concatUI32(dst, pp->head.count);
							}
							NBString_concat(dst, ")");
						}
						//Body
						switch (pp->head.type) {
							case ENRtcpPacketType_SR: //Packet sender report (SR)
								{
									const STNBRtcpPacketSR* p = (const STNBRtcpPacketSR*)pp->data;
									NBString_concat(dst, "\n    ssrc:         "); NBString_concatUI32(dst, p->head.ssrc); 
									NBString_concat(dst, "\n    ntpWordHigh:  "); NBString_concatUI32(dst, p->head.ntpWordHigh);
									NBString_concat(dst, "\n    ntpWordLow:   "); NBString_concatUI32(dst, p->head.ntpWordLow);
									NBString_concat(dst, "\n    rtpTimestamp: "); NBString_concatUI32(dst, p->head.rtpTimestamp);
									NBString_concat(dst, "\n    packtCount:   "); NBString_concatUI32(dst, p->head.packtCount);
									NBString_concat(dst, "\n    octetCount:   "); NBString_concatUI32(dst, p->head.octetCount);
									if(p->reports != NULL){
										UI32 i; for(i = 0; i < p->reportsSz; i++){
											const STNBRtcpRcptRprt* rpt = &p->reports[i];
											NBString_concat(dst, "\n    Report ");
											NBString_concatUI32(dst, i + 1);
											NBString_concat(dst, "/");
											NBString_concatUI32(dst, p->reportsSz);
											//
											NBString_concat(dst, "\n      ssrc:               "); NBString_concatUI32(dst, rpt->ssrc);
											NBString_concat(dst, "\n      fractionLost:       "); NBString_concatUI32(dst, rpt->fractionLost);
											NBString_concat(dst, "\n      accumPacktLost:     "); NBString_concatUI32(dst, rpt->accumPacktLost);
											NBString_concat(dst, "\n      highestSeqNum:      "); NBString_concatUI32(dst, rpt->highestSeqNum);
											NBString_concat(dst, "\n      interarrivalJitter: "); NBString_concatUI32(dst, rpt->interarrivalJitter);
											NBString_concat(dst, "\n      lastSenderReport:   "); NBString_concatUI32(dst, rpt->lastSenderReport);
											NBString_concat(dst, "\n      delaySinceLastSR:   "); NBString_concatUI32(dst, rpt->delaySinceLastSR);
										}
									}
								}
								break;
							case ENRtcpPacketType_RR: //Packet receiver report (RR)
								{
									const STNBRtcpPacketRR* p = (const STNBRtcpPacketRR*)pp->data;
									NBString_concat(dst, "\n    ssrc:         "); NBString_concatUI32(dst, p->head.ssrc); 
									if(p->reports != NULL){
										UI32 i; for(i = 0; i < p->reportsSz; i++){
											const STNBRtcpRcptRprt* rpt = &p->reports[i];
											NBString_concat(dst, "\n    Report ");
											NBString_concatUI32(dst, i + 1);
											NBString_concat(dst, "/");
											NBString_concatUI32(dst, p->reportsSz);
											//
											NBString_concat(dst, "\n      ssrc:               "); NBString_concatUI32(dst, rpt->ssrc);
											NBString_concat(dst, "\n      fractionLost:       "); NBString_concatUI32(dst, rpt->fractionLost);
											NBString_concat(dst, "\n      accumPacktLost:     "); NBString_concatUI32(dst, rpt->accumPacktLost);
											NBString_concat(dst, "\n      highestSeqNum:      "); NBString_concatUI32(dst, rpt->highestSeqNum);
											NBString_concat(dst, "\n      interarrivalJitter: "); NBString_concatUI32(dst, rpt->interarrivalJitter);
											NBString_concat(dst, "\n      lastSenderReport:   "); NBString_concatUI32(dst, rpt->lastSenderReport);
											NBString_concat(dst, "\n      delaySinceLastSR:   "); NBString_concatUI32(dst, rpt->delaySinceLastSR);
										}
									}
								}
								break;
							case ENRtcpPacketType_SDES: //Packet source description item (SDES)
								{
									const STNBRtcpPacketSDES* p = (const STNBRtcpPacketSDES*)pp->data;
									if(p->chunks == NULL || p->chunksSz <= 0){
										NBString_concat(dst, "\n    zero-chunks.");
									} else {
										UI32 i; for(i = 0; i < p->chunksSz; i++){
											const STNBRtcpSrcDescChunck* chk = &p->chunks[i];
											NBString_concat(dst, "\n    Chunk ");
											NBString_concatUI32(dst, i + 1);
											NBString_concat(dst, "/");
											NBString_concatUI32(dst, p->chunksSz);
											//
											NBString_concat(dst, "\n      ssrc/csrc:          "); NBString_concatUI32(dst, chk->ssrc);
											//
											if(chk->itms != NULL && chk->itmsSz > 0){
												UI32 i; for(i = 0; i < chk->itmsSz; i++){
													const STNBRtcpSrcDescItm* itm = &chk->itms[i];
													const STNBRtcpSDESTypeDef* sTypeDef = NBRtcp_getSDESTypeDef(itm->type);
													NBString_concat(dst, "\n      Itm ");
													NBString_concatUI32(dst, i + 1);
													NBString_concat(dst, "/");
													NBString_concatUI32(dst, chk->itmsSz);
													if(sTypeDef == NULL){
														NBString_concat(dst, " unexpected-type(");
														NBString_concatUI32(dst, itm->type);
														NBString_concat(dst, ")");
													} else {
														NBString_concat(dst, " ");
														NBString_concat(dst, sTypeDef->acronim);
													}
													NBString_concat(dst, " len(");
													NBString_concatUI32(dst, itm->length);
													NBString_concat(dst, "): '");
													{
														const char* c = itm->str;
														const char* cAfterEnd = c + itm->length;
														while(c < cAfterEnd){
															if(*c == '\0'){
																NBString_concat(dst, "\\0");
															} else {
																NBString_concatByte(dst, *c);
															}
															c++;
														}
													}
													NBString_concat(dst, "'");
												}
											}
										}
									}
								}
								break;
							case ENRtcpPacketType_BYE: //Goodbye RTCP Packet
								{
									const STNBRtcpPacketBYE* p = (const STNBRtcpPacketBYE*)pp->data;
									if(p->ssrcs == NULL || p->ssrcsSz <= 0){
										NBString_concat(dst, "\n    zero-ssrcs.");
									} else {
										NBString_concat(dst, "\n    ssrcs:   ");
										{
											UI32 i; for(i = 0; i < p->ssrcsSz; i++){
												const UI32 ssrc = p->ssrcs[i];
												if(i != 0) NBString_concat(dst, ", ");
												NBString_concatUI32(dst, ssrc);
											}
										}
									}
									if(p->reason.str == NULL || p->reason.length <= 0){
										NBString_concat(dst, "\n    no-explicit-reason.");
									} else {
										NBString_concat(dst, "\n    reason:  '");
										{
											const char* c = p->reason.str;
											const char* cAfterEnd = c + p->reason.length;
											while(c < cAfterEnd){
												if(*c == '\0'){
													NBString_concat(dst, "\\0");
												} else {
													NBString_concatByte(dst, *c);
												}
												c++;
											}
										}
										NBString_concat(dst, "'");
									}
								}
								break;
							default:
								NBString_concat(dst, "\n    Unimplemented type.");
								break;
						}
					}
				}
			}
		}
	}
}

//

void NBRtcpParser_ntoh16(const void* ptr16, void* dst16){
	const BYTE* src	= (BYTE*)ptr16;
	BYTE* dst		= (BYTE*)dst16;
	dst[0]	= src[1];
	dst[1]	= src[0];
}

void NBRtcpParser_ntoh24(const void* ptr24, void* dst32){
	const BYTE* src	= (BYTE*)ptr24;
	BYTE* dst		= (BYTE*)dst32;
	dst[1]	= src[2];
	dst[2]	= src[1];
	dst[3]	= src[0];
}

void NBRtcpParser_ntoh32(const void* ptr32, void* dst32){
	const BYTE* src	= (BYTE*)ptr32;
	BYTE* dst		= (BYTE*)dst32;
	dst[0]	= src[3];
	dst[1]	= src[2];
	dst[2]	= src[1];
	dst[3]	= src[0];
}

//

BOOL NBRtcpParser_translatePacketBytes(void* pData, const UI32 dataSz, STNBRtcpParserResult* dst){
	BOOL r = FALSE;
	if(pData != NULL && dataSz >= 4){
		BYTE c;
		const BYTE* data = (BYTE*)pData;
		const BYTE* dataAfterEnd = data + dataSz;
		const STNBRtcpPacketTypeDef* packTypeDef = NULL;
		STNBRtcpPacketHead head;
		r = TRUE;
		while(r && (data + 4) < dataAfterEnd){
			c = *(data++);
			head.version		= ((c >> 6) & 0x3);
			head.havePadding	= ((c >> 5) & 0x1);
			head.count			= (c & 0x1F);
			//
			head.type			= *(data++);
			//
			head.length			= ((data[0]) << 8) | data[1]; data += 2;
			//
			packTypeDef = NBRtcp_getPacketTypeDef(head.type);
			if(packTypeDef ==  NULL){
				PRINTF_ERROR("RTCPParser, unexpected packetType(%d).\n", head.type);
				r = FALSE;
			} else {
				//PRINTF_INFO("RTCPParser, packetType(%s, %s) v(%d) pading(%s) len(%d bytes).\n", packTypeDef->acronim, packTypeDef->desc, head.version, head.havePadding ? "yes" : "no", ((head.length + 1) * 4));
				if(data + (head.length * 4) > dataAfterEnd){
					PRINTF_ERROR("RTCPParser, incomplete packetType(%s, %s) missingBytes(%d).\n", packTypeDef->acronim, packTypeDef->desc, (SI32)(head.length * 4) - (SI32)(dataAfterEnd - data));
					r = FALSE;
				} else {
					void* parsedData = NULL;
					const BYTE* pcktAfterEnd = data + (head.length * 4);
					switch(head.type){
						case ENRtcpPacketType_SR: //Packet sender report (SR)
							if((pcktAfterEnd - data) < sizeof(STNBRtcpPacketSRHead)){
								PRINTF_ERROR("RTCPParser, incomplete packetType(%s, %s) missingBytes(%d).\n", packTypeDef->acronim, packTypeDef->desc, (SI32)(head.length * 4) - (SI32)(dataAfterEnd - data));
								r = FALSE;
							} else {
								STNBRtcpPacketSR* pkt = NBMemory_allocType(STNBRtcpPacketSR);
								NBMemory_setZeroSt(*pkt, STNBRtcpPacketSR);
								//
								NBRtcpParser_ntoh32(data, &pkt->head.ssrc); data += 4;
								NBRtcpParser_ntoh32(data, &pkt->head.ntpWordHigh); data += 4;
								NBRtcpParser_ntoh32(data, &pkt->head.ntpWordLow); data += 4;
								NBRtcpParser_ntoh32(data, &pkt->head.rtpTimestamp); data += 4;
								NBRtcpParser_ntoh32(data, &pkt->head.packtCount); data += 4;
								NBRtcpParser_ntoh32(data, &pkt->head.octetCount); data += 4;
								//
								if(pcktAfterEnd < (data + (24 * head.count))){
									PRINTF_ERROR("RTCPParser, incomplete packetType(%s, %s) missingBytes(%d).\n", packTypeDef->acronim, packTypeDef->desc, (UI32)((data + (24 * head.count)) - pcktAfterEnd));
									r = FALSE;
								} else if(head.count > 0){
									UI32 i;
									pkt->reports	= NBMemory_allocTypes(STNBRtcpRcptRprt, head.count);
									NBMemory_set(pkt->reports, 0, sizeof(pkt->reports[0]) * head.count);
									for(i = 0 ; i < head.count; i++){
										STNBRtcpRcptRprt* rpt = &pkt->reports[i];
										NBRtcpParser_ntoh32(data, &rpt->ssrc); data += 4;
										rpt->fractionLost	= *(data++);
										NBRtcpParser_ntoh24(data, &rpt->accumPacktLost); data += 3;
										NBRtcpParser_ntoh32(data, &rpt->highestSeqNum); data += 4;
										NBRtcpParser_ntoh32(data, &rpt->interarrivalJitter); data += 4;
										NBRtcpParser_ntoh32(data, &rpt->lastSenderReport); data += 4;
										NBRtcpParser_ntoh32(data, &rpt->delaySinceLastSR); data += 4;
									}
									pkt->reportsSz = head.count;
								}
								//Skip profile-specific extensions (remainig data)
								data = pcktAfterEnd;
								//
								parsedData = pkt;
							}
							break;
						case ENRtcpPacketType_RR: //Packet receiver report (RR)
							if((pcktAfterEnd - data) < sizeof(STNBRtcpPacketRRHead)){
								PRINTF_ERROR("RTCPParser, incomplete packetType(%s, %s) missingBytes(%d).\n", packTypeDef->acronim, packTypeDef->desc, (SI32)(head.length * 4) - (SI32)(dataAfterEnd - data));
								r = FALSE;
							} else {
								STNBRtcpPacketRR* pkt = NBMemory_allocType(STNBRtcpPacketRR);
								NBMemory_setZeroSt(*pkt, STNBRtcpPacketRR);
								//
								NBRtcpParser_ntoh32(data, &pkt->head.ssrc); data += 4;
								//
								if(pcktAfterEnd < (data + (24 * head.count))){
									PRINTF_ERROR("RTCPParser, incomplete packetType(%s, %s) missingBytes(%d).\n", packTypeDef->acronim, packTypeDef->desc, (UI32)((data + (24 * head.count)) - pcktAfterEnd));
									r = FALSE;
								} else if(head.count > 0){
									UI32 i;
									pkt->reports	= NBMemory_allocTypes(STNBRtcpRcptRprt, head.count);
									NBMemory_set(pkt->reports, 0, sizeof(pkt->reports[0]) * head.count);
									for(i = 0 ; i < head.count; i++){
										STNBRtcpRcptRprt* rpt = &pkt->reports[i];
										NBRtcpParser_ntoh32(data, &rpt->ssrc); data += 4;
										rpt->fractionLost	= *(data++);
										NBRtcpParser_ntoh24(data, &rpt->accumPacktLost); data += 3;
										NBRtcpParser_ntoh32(data, &rpt->highestSeqNum); data += 4;
										NBRtcpParser_ntoh32(data, &rpt->interarrivalJitter); data += 4;
										NBRtcpParser_ntoh32(data, &rpt->lastSenderReport); data += 4;
										NBRtcpParser_ntoh32(data, &rpt->delaySinceLastSR); data += 4;
									}
									pkt->reportsSz = head.count;
								}
								//Skip profile-specific extensions (remainig data)
								data = pcktAfterEnd;
								//
								parsedData = pkt;
							}
							break;
						case ENRtcpPacketType_SDES: //Packet source description item (SDES)
							{
								
								STNBRtcpPacketSDES* pkt = NBMemory_allocType(STNBRtcpPacketSDES);
								NBMemory_setZeroSt(*pkt, STNBRtcpPacketSDES);
								if(head.count > 0){
									UI8 i, use = 0;
									pkt->chunks		= NBMemory_allocTypes(STNBRtcpSrcDescChunck, head.count);
									NBMemory_set(pkt->chunks, 0, sizeof(pkt->chunks[0]) * head.count);
									for(i = 0; i < head.count && r; i++){
										STNBRtcpSrcDescChunck* chk = &pkt->chunks[use];
										//"Each chunk starts on a 32-bit boundary."
										while(((UI64)data % 4) != 0){
											data++;
										}
										//
										if((data + 4) > pcktAfterEnd){
											PRINTF_ERROR("RTCPParser, incomplete SDESChunk.\n");
											r = FALSE;
										} else {
											NBRtcpParser_ntoh32(data, &chk->ssrc); data += 4; //sscr/csrc
											while(data < pcktAfterEnd && r){
												const UI8 type = *data; data++;
												if(type == ENRtcpPacketSDESType_END){
													break;
												} else if(data >= pcktAfterEnd){
													PRINTF_ERROR("RTCPParser, incomplete SDESChunkItem (len is missing).\n");
													r = FALSE;
													break;
												} else {
													const UI8 len = *data; data++;
													if((data + len) > pcktAfterEnd){
														PRINTF_ERROR("RTCPParser, incomplete SDESChunkItem (str is missing).\n");
														r = FALSE;
														break;
													} else {
														//Increase buffer
														{
															STNBRtcpSrcDescItm* nArr = NBMemory_allocTypes(STNBRtcpSrcDescItm, chk->itmsSz + 1);
															if(chk->itms != NULL){
																if(chk->itmsSz > 0){
																	NBMemory_copy(nArr, chk->itms, sizeof(chk->itms[0]) * chk->itmsSz);
																}
																NBMemory_free(chk->itms);
															}
															chk->itms = nArr;
															chk->itmsSz++;
														}
														//Set
														{
															STNBRtcpSrcDescItm* nItm = &chk->itms[chk->itmsSz - 1];
															NBMemory_setZeroSt(*nItm, STNBRtcpSrcDescItm);
															nItm->type		= type;
															nItm->length	= len;
															nItm->str		= NBMemory_alloc(len + 1); 
															NBMemory_copy(nItm->str, data, len);
															nItm->str[len] = '\0';
														}
														data += len;
													}
												}
											}
											use++;
										}
									}
									//"Each chunk starts on a 32-bit boundary."
									if(r){
										while(((UI64)data % 4) != 0){
											data++;
										}
									}
									pkt->chunksSz = use;
								}
								//
								parsedData = pkt;
							}
							break;
						case ENRtcpPacketType_BYE: //Goodbye RTCP Packet
							if((pcktAfterEnd - data) < (sizeof(UI32) * head.count)){
								PRINTF_ERROR("RTCPParser, incomplete packetType(%s, %s) missingBytes(%d).\n", packTypeDef->acronim, packTypeDef->desc, (SI32)(sizeof(UI32) * head.count) - (SI32)(dataAfterEnd - data));
								r = FALSE;
							} else {
								STNBRtcpPacketBYE* pkt = NBMemory_allocType(STNBRtcpPacketBYE);
								NBMemory_setZeroSt(*pkt, STNBRtcpPacketBYE);
								//ssrcs
								if(head.count > 0){
									pkt->ssrcsSz	= head.count;
									pkt->ssrcs		= NBMemory_allocTypes(UI32, head.count);
									NBMemory_set(pkt->ssrcs, 0, sizeof(pkt->ssrcs[0]) * head.count);
									{
										UI32 i; for(i = 0; i < head.count; i++){
											NBRtcpParser_ntoh32(data, &pkt->ssrcs[i]); data += 4;
										}
									}
								}
								//reason for leaving (optional)
								if((pcktAfterEnd - data) > 0){
									const UI8 len = *data;
									if((pcktAfterEnd - data) < len){
										PRINTF_ERROR("RTCPParser, incomplete packetType(%s, %s) missingBytes(%d).\n", packTypeDef->acronim, packTypeDef->desc, (SI32)len - (SI32)(dataAfterEnd - data));
										r = FALSE;
									} else {
										pkt->reason.length	= len;
										pkt->reason.str		= NBMemory_alloc(len + 1); 
										NBMemory_copy(pkt->reason.str, data, len);
										pkt->reason.str[len] = '\0';
										data += len;
									}
								}
								//
								parsedData = pkt;
							}
							break;
						default:
							PRINTF_ERROR("RTCPParser, unimplemented packetType(%d).\n", head.type);
							r = FALSE;
						break;
					}
					//Append packet
					if(parsedData != NULL){
						//Increase buffer
						{
							STNBRtcpParserResultPacket* nArr = NBMemory_allocTypes(STNBRtcpParserResultPacket, dst->packetsSz + 1);
							if(dst->packets != NULL){
								if(dst->packetsSz > 0){
									NBMemory_copy(nArr, dst->packets, sizeof(dst->packets[0]) * dst->packetsSz);
								}
								NBMemory_free(dst->packets);
							}
							dst->packets = nArr;
							dst->packetsSz++;
						}
						//Set
						{
							STNBRtcpParserResultPacket* nPckt = &dst->packets[dst->packetsSz - 1];
							NBMemory_setZeroSt(*nPckt, STNBRtcpParserResultPacket);
							nPckt->head = head;
							nPckt->data	= parsedData;
						}
					}
					//Remaining bytes
					if(r && data < pcktAfterEnd){
						PRINTF_ERROR("RTCPParser, %d unconsumed bytes in packet.\n", (SI32)(pcktAfterEnd - data));
						r = FALSE;
					}
				}
			} 
		}
		//Remaining bytes
		if(r && data < dataAfterEnd){
			PRINTF_ERROR("RTCPParser, %d unconsumed bytes in feed.\n", (SI32)(dataAfterEnd - data));
			r = FALSE;
		}
	}
	return r;
}
