//
//  NBAvcParser.c
//  nbframework
//
//  Created by Marcos Ortega on 9/21/20.
//

#include "nb/NBFrameworkPch.h"
#include "nb/2d/NBAvcParser.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBArray.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBMngrStructMaps.h"

STNBStructMapsRec NBAvcPicDescBase_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBAvcPicDescBase_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBAvcPicDescBase_sharedStructMap);
	if(NBAvcPicDescBase_sharedStructMap.map == NULL){
		STNBAvcPicDescBase s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBAvcPicDescBase);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addUIntM(map, s, w);			//pixels = luma-samples = (w-mbs * 16)
		NBStructMap_addUIntM(map, s, h);			//pixels = luma-samples = ((h-units to h-mbs formula) * 16)
		NBStructMap_addUIntM(map, s, fpsMax);		//frames per sec
		NBStructMap_addBoolM(map, s, isFixedFps);	//frames per sec is fixed
		//sps
		NBStructMap_addUIntM(map, s, avcProfile);		//sps first byte, determines the bitstream format (higher profiles are backward compatible with lower profiles)
		NBStructMap_addUIntM(map, s, profConstraints);	//sps second byte, determines the profile's contrains flags, each active flag represents
		NBStructMap_addUIntM(map, s, profLevel);			//sps third byte, determines the profile's maximun values of certain fields (required maximun decoding power)
		NBStructMap_addUIntM(map, s, chromaFmt);		//analyzed
		NBStructMap_addUIntM(map, s, bitDepthLuma);		//analyzed (not-found if lower than 8)
		NBStructMap_addUIntM(map, s, bitDepthChroma);	//analyzed (not-found if lower than 8)
		//
		NBAvcPicDescBase_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBAvcPicDescBase_sharedStructMap);
	return NBAvcPicDescBase_sharedStructMap.map;
}

//Returns true if the avcProfile of 'other' is lower or equals.

BOOL NBAvcPicDescBase_isCompatibleProfile(const STNBAvcPicDescBase* obj, const STNBAvcPicDescBase* other){
    return (obj->avcProfile == other->avcProfile && obj->profConstraints == other->profConstraints && obj->profLevel == other->profLevel);
    /*
    //Note: this seems not to work; higher profile cannot be simply mixed with lower ones.
    return (
            obj->avcProfile > other->avcProfile //other is lower profile
            || (obj->avcProfile == other->avcProfile && (~obj->profConstraints & other->profConstraints) == 0 && obj->profLevel >= other->profLevel) //other is same profile with non-higher contraints and non-higher level.
            );
     */
}

//Returns true if the image size are the same and the profile of 'other' is lower or equals.

BOOL NBAvcPicDescBase_isCompatible(const STNBAvcPicDescBase* obj, const STNBAvcPicDescBase* other){
    return (obj != NULL && other != NULL
            && obj->w == other->w //same width
            && obj->h == other->h //same height, NOTE: luma height could include few extra pixels that will be cropped on the final image
            && NBAvcPicDescBase_isCompatibleProfile(obj, other)
            && obj->chromaFmt == other->chromaFmt
            && obj->bitDepthLuma == other->bitDepthLuma
            && obj->bitDepthChroma == other->bitDepthChroma
            )
    ;
}

//

void NBAvcParser_init(STNBAvcParser* obj){
	NBMemory_setZeroSt(*obj, STNBAvcParser);
	NBAvcStateParams_init(&obj->params.data);
}

void NBAvcParser_release(STNBAvcParser* obj){
	NBAvcStateParams_release(&obj->params.data);
}

void NBAvcParser_syncPicDescBase_(STNBAvcParser* obj);

//Feed
	
BOOL NBAvcParser_feedStart(STNBAvcParser* obj, const UI8 nal_ref_idc, const UI8 nal_unit_type){
	BOOL r = FALSE;
	const STNBAvcNaluMap* map = NBAvc_getNaluMap(nal_unit_type);
	NBMemory_setZero(obj->curNALU);
	if(map == NULL){
		//PRINTF_ERROR("NBAvcParser, no map for unitType(%d).\n", nal_unit_type);
	}
	if(map != NULL){
		obj->params.data.hdr.refIdc				= nal_ref_idc; //u(2)
		obj->params.data.hdr.type				= nal_unit_type; //u(5)
		obj->params.data.svc_extension_flag		= FALSE; //u(1)
		obj->params.data.avc_3d_extension_flag	= FALSE; //u(1)
		obj->params.data.NumBytesInRBSP			= 0;
		obj->params.data.nalUnitHeaderBytes		= 1;
		obj->curNALU.map						= map;
		//Remain
		obj->curNALU.remain.bytesCount			= 0;
		obj->curNALU.remain.bitsCount			= 0;
		//
		r = TRUE;
	}
	return r;
}

/*void NBAvcParser_feedBytesStart_(STNBAvcParserBitsReader* rdr, const void* pData, const UI32 dataSz){
	rdr->cur.src			= (const BYTE*)pData;
	rdr->cur.srcAfterEnd	= rdr->cur.src + dataSz;
	rdr->cur.bitsRemain		= 8;
}*/

/*void NBAvcParser_feedBytesEnd_(STNBAvcParserBitsReader* rdr, const void* pData, const UI32 dataSz){
	if(rdr->pend.bitsRemain > 0){
		NBASSERT(rdr->cur.mPartial == NBAvcParser_readBitsPartialCheckingPend_) // 'check pend' method should remain in use
		NBASSERT(rdr->cur.mFull == NBAvcParser_readBitsFullCheckingPend_) // 'check pend' method should remain in use
		NBASSERT(rdr->cur.bitsRemain == 8)	//no bits should be consumed
		NBASSERT(rdr->cur.src == (const BYTE*)pData)	//no bytes should be consumed						//no bytes should be consumed
	} else {
		NBASSERT(rdr->cur.mPartial == NBAvcParser_readBitsPartialIgnoringPend_) // 'ignore pend' method should remain in use
		NBASSERT(rdr->cur.mFull == NBAvcParser_readBitsFullIgnoringPend_) // 'ignore pend' method should remain in use
		if(rdr->cur.bitsRemain < 8){
			//Keep last byte
			NBASSERT(rdr->cur.src < rdr->cur.srcAfterEnd) //should be inside the buffer
			NBASSERT(rdr->cur.bitsRemain > 0)
			rdr->pend.val			= *(rdr->cur.src++);
			rdr->pend.bitsRemain	= rdr->cur.bitsRemain;
			rdr->cur.mPartial		= NBAvcParser_readBitsPartialCheckingPend_;
			rdr->cur.mFull			= NBAvcParser_readBitsFullCheckingPend_;
		}
	}
}*/

UI32 NBAvcParser_feedBytes(STNBAvcParser* obj, const void* pData, const UI32 dataSz){
	//Start
	//NBAvcParser_feedBytesStart_(rdr, pData, dataSz);
	//Read
	//Result
	//NBAvcParser_feedBytesEnd_(rdr, pData, dataSz);
	//return (UI32)(rdr->cur.srcAfterEnd - rdr->cur.src);
	return 0;
}

BOOL NBAvcParser_feedNALUBitstream_(STNBAvcParser* obj, STNBAvcBitstream* bs, const BOOL picDescBaseOnly){
	BOOL r = FALSE;
	if(obj->curNALU.map == NULL){
		PRINTF_ERROR("NBAvcParser, no map for NALU(%d).\n", obj->params.data.hdr.type);
	} else if(obj->curNALU.map->createSt == NULL || obj->curNALU.map->releaseSt == NULL || obj->curNALU.map->loadBits == NULL){
		PRINTF_ERROR("NBAvcParser, incomplete map for NALU(%d).\n", obj->params.data.hdr.type);
	} else {
		BOOL releaseSt = TRUE;
		void* dataSt = NULL;
		//optimization, allocate-once-known-structs
		switch(obj->params.data.hdr.type) {
			case 7: //seq_parameter_set_rbsp (nal_unit_type = 7)
				if(obj->params.data.seq_parameter_set_rbsp == NULL){
					dataSt = (*obj->curNALU.map->createSt)();
					if(dataSt != NULL){
						obj->params.data.seq_parameter_set_rbsp = (STNBAvc_seq_parameter_set_rbsp*)dataSt;
						releaseSt = FALSE;
					}
				} else {
					dataSt = obj->params.data.seq_parameter_set_rbsp;
					releaseSt = FALSE;
				}
				break;
			case 8: //pic_parameter_set_rbsp (nal_unit_type = 8)
				if(obj->params.data.pic_parameter_set_rbsp == NULL){
					dataSt = (*obj->curNALU.map->createSt)();
					if(dataSt != NULL){
						obj->params.data.pic_parameter_set_rbsp = (STNBAvc_pic_parameter_set_rbsp*)dataSt;
						releaseSt = FALSE;
					}
				} else {
					dataSt = obj->params.data.pic_parameter_set_rbsp;
					releaseSt = FALSE;
				}
				break;
			case 1: //IDR
			case 5: //non-IDR
			case 19: //non-IDR
				if(obj->params.data.slice_layer_without_partitioning_rbsp == NULL){
					dataSt = (*obj->curNALU.map->createSt)();
					if(dataSt != NULL){
						obj->params.data.slice_layer_without_partitioning_rbsp = (STNBAvc_slice_layer_without_partitioning_rbsp*)dataSt;
						releaseSt = FALSE;
					}
				} else {
					dataSt = obj->params.data.slice_layer_without_partitioning_rbsp;
					releaseSt = FALSE;
				}
				break;
			default:
				dataSt = (*obj->curNALU.map->createSt)();
				break;
		}
		//parse
		if(dataSt == NULL){
			PRINTF_ERROR("NBAvcParser, 'createSt' failed for NALU(%d).\n", obj->params.data.hdr.type);
		} else {
            BYTE frst = 0; STNBAvcNaluHdr desc;
            //Print data
            /*{
                //Bytes format
                {
                    const char* hex = "0123456789abcdef";
                    UI32 i = 0, zSeq = 0;
                    STNBString str;
                    NBString_init(&str);
                    while(!NBAvcBitstream_isAtEnd(bs)){
                        const BYTE b = NBAvcBitstream_readU8(bs, 8);
                        const BYTE b0 = (b >> 4) & 0xF;
                        const BYTE b1 = b & 0xF;
                        NBString_concatByte(&str, hex[b0]);
                        NBString_concatByte(&str, hex[b1]);
                        if(b == 0x00){
                            if(zSeq >= 2) { NBString_concat(&str, "{forbid_0x000000}"); }
                            zSeq++;
                        } else{
                            if(b == 0x01 && zSeq >= 2){ NBString_concat(&str, "{forbid_0x000001}"); }
                            if(b == 0x02 && zSeq >= 2){ NBString_concat(&str, "{forbid_0x000002}"); }
                            if(b == 0x03 && zSeq >= 2){ NBString_concat(&str, "{em_prev_0x000003}"); }
                            zSeq = 0;
                        }
                        if(((i + 1) % 64) == 0){
                            NBString_concat(&str, " //");
                            NBString_concatUI32(&str, (i + 1));
                            NBString_concatByte(&str, '\n');
                        } else if(((i + 1) % 4) == 0){
                            NBString_concatByte(&str, ' ');
                        }
                        i++;
                    }
                    if((i % 64) != 0){
                        NBString_concat(&str, " //");
                        NBString_concatUI32(&str, i);
                        NBString_concatByte(&str, '\n');
                    }
                    PRINTF_INFO("NBAvcParser, NAL bytes:\n%s\n\n", str.str);
                    NBString_release(&str);
                    //reset bitstream
                    NBAvcBitstream_moveToStart(bs);
                }
                //Bits format
                {
                    UI32 i = 0;
                    STNBString str;
                    NBString_init(&str);
                    while(!NBAvcBitstream_isAtEnd(bs)){
                        const BYTE b = NBAvcBitstream_readU8(bs, 8);
                        NBString_concatByte(&str, (b & 0x80) ? '1' : '0');
                        NBString_concatByte(&str, (b & 0x40) ? '1' : '0');
                        NBString_concatByte(&str, (b & 0x20) ? '1' : '0');
                        NBString_concatByte(&str, (b & 0x10) ? '1' : '0');
                        NBString_concatByte(&str, '-');
                        NBString_concatByte(&str, (b & 0x8) ? '1' : '0');
                        NBString_concatByte(&str, (b & 0x4) ? '1' : '0');
                        NBString_concatByte(&str, (b & 0x2) ? '1' : '0');
                        NBString_concatByte(&str, (b & 0x1) ? '1' : '0');
                        if(((i + 1) % 8) == 0){
                            NBString_concat(&str, " //");
                            NBString_concatUI32(&str, (i + 1));
                            NBString_concatByte(&str, '\n');
                        } else {
                            NBString_concatByte(&str, ' ');
                        }
                        i++;
                    }
                    if((i % 8) != 0){
                        NBString_concat(&str, " //");
                        NBString_concatUI32(&str, i);
                        NBString_concatByte(&str, '\n');
                    }
                    PRINTF_INFO("NBAvcParser, NAL bits:\n%s\n\n", str.str);
                    NBString_release(&str);
                    //reset bitstream
                    NBAvcBitstream_moveToStart(bs);
                }
            }*/
			//First byte is (refIdc + type) header.
			frst = NBAvcBitstream_readU8(bs, 8);
			desc = NBAvc_getNaluHdr(frst);
			NBASSERT(desc.forbiddenZeroBit == 0)
			//NBASSERT(desc.refIdc == obj->params.data.hdr.refIdc) //Note: this fails in old cameras ("reseting params" implemented as result)
			NBASSERT(desc.type == obj->params.data.hdr.type)
            //reseting params
            obj->params.data.hdr.type = desc.type;
            obj->params.data.hdr.refIdc = desc.refIdc;
			//
			NBASSERT(!obj->params.data.svc_extension_flag)
			NBASSERT(!obj->params.data.avc_3d_extension_flag)
			if(obj->params.data.hdr.type == 14 || obj->params.data.hdr.type == 20 || obj->params.data.hdr.type == 21){
				if(obj->params.data.hdr.type != 21){
					obj->params.data.svc_extension_flag = NBAvcBitstream_readU8(bs, 1);
				} else {
					obj->params.data.avc_3d_extension_flag = NBAvcBitstream_readU8(bs, 1);
				}
				if(obj->params.data.svc_extension_flag){
					NBASSERT(FALSE) //ToDo: implement
					//nal_unit_header_svc_extension( ) /* specified in Annex G */
					obj->params.data.nalUnitHeaderBytes += 3;
				} else if(obj->params.data.avc_3d_extension_flag){
					NBASSERT(FALSE) //ToDo: implement
					//nal_unit_header_3davc_extension( ) /* specified in Annex J */
					obj->params.data.nalUnitHeaderBytes += 2;
				} else {
					NBASSERT(FALSE) //ToDo: implement
					//nal_unit_header_mvc_extension( ) /* specified in Annex H */
					obj->params.data.nalUnitHeaderBytes += 3;
				}
			}
			if(!(*obj->curNALU.map->loadBits)(bs, &obj->params.data, dataSt, picDescBaseOnly)){
				//PRINTF_ERROR("NBAvcParser, 'loadBits' failed for NALU(%d).\n", obj->params.data.hdr.type);
				r = FALSE;
			} else if(picDescBaseOnly){
				r = TRUE;
			} else {
				//Check if bitstream was consumed
				obj->curNALU.remain.bitsCount	= NBAvcBitstream_getBitsRemainCount(bs);
				obj->curNALU.remain.bytesCount	= 0;
				if(obj->curNALU.remain.bitsCount == 8){
					obj->curNALU.remain.bitsCount = 0;
					obj->curNALU.remain.bytesCount++;
				} else {
					NBAvcBitstream_moveToNextByte(bs);
				}
				obj->curNALU.remain.bytesCount	+= (UI32)NBAvcBitstream_moveToEnd(bs);
				r = (obj->curNALU.remain.bytesCount == 0 && obj->curNALU.remain.bitsCount == 0);
				if(!r){
					PRINTF_INFO("NBAvcParser, NALU(%d) remain(%d bytes and %d bits).\n", obj->params.data.hdr.type, obj->curNALU.remain.bytesCount, obj->curNALU.remain.bitsCount);
				}
			}
			//Keep struct if params
			if(r){
				switch(obj->params.data.hdr.type) {
					case 7: //seq_parameter_set_rbsp (nal_unit_type = 7)
						{
							STNBAvc_seq_parameter_set_rbsp* seq_parameter_set_rbsp = (STNBAvc_seq_parameter_set_rbsp*)dataSt;
							NBASSERT(obj->params.data.seq_parameter_set_rbsp == seq_parameter_set_rbsp)
							NBASSERT(!releaseSt)
							//set props param
							if(seq_parameter_set_rbsp != NULL && seq_parameter_set_rbsp->data != NULL){
								const STNBAvc_seq_parameter_set_data* data = seq_parameter_set_rbsp->data;
								obj->picDescBase.params.sps.iSeq++;
								obj->picDescBase.params.sps.profile_idc						= data->profile_idc;
								obj->picDescBase.params.sps.constraint_set					= ((data->constraint_set0_flag & 0x1) << 7) | ((data->constraint_set1_flag & 0x1) << 6) | ((data->constraint_set2_flag & 0x1) << 5) | ((data->constraint_set3_flag & 0x1) << 4) | ((data->constraint_set4_flag & 0x1) << 3) | ((data->constraint_set5_flag & 0x1) << 2) | (data->reserved_zero_2bits & 0x3);
								obj->picDescBase.params.sps.level_idc						= data->level_idc;
								obj->picDescBase.params.sps.chroma_format_idc				= (UI8)data->chroma_format_idc;
								obj->picDescBase.params.sps.bit_depth_luma_minus8			= (UI8)data->bit_depth_luma_minus8;
								obj->picDescBase.params.sps.bit_depth_chroma_minus8			= (UI8)data->bit_depth_chroma_minus8;
								obj->picDescBase.params.sps.separate_colour_plane_flag		= data->separate_colour_plane_flag;
								obj->picDescBase.params.sps.pic_width_in_mbs_minus1			= data->pic_width_in_mbs_minus1;
								obj->picDescBase.params.sps.pic_height_in_map_units_minus1	= data->pic_height_in_map_units_minus1;
								obj->picDescBase.params.sps.frame_mbs_only_flag				= data->frame_mbs_only_flag;
								obj->picDescBase.params.sps.vui_parameters_present_flag		= data->vui_parameters_present_flag;
								if(data->frame_cropping_flag){
									obj->picDescBase.params.sps.frame_crop_left_offset		= data->frame_crop_left_offset;
									obj->picDescBase.params.sps.frame_crop_right_offset		= data->frame_crop_right_offset;
									obj->picDescBase.params.sps.frame_crop_top_offset		= data->frame_crop_top_offset;
									obj->picDescBase.params.sps.frame_crop_bottom_offset	= data->frame_crop_bottom_offset;
								} else {
									obj->picDescBase.params.sps.frame_crop_left_offset		= 0;
									obj->picDescBase.params.sps.frame_crop_right_offset		= 0;
									obj->picDescBase.params.sps.frame_crop_top_offset		= 0;
									obj->picDescBase.params.sps.frame_crop_bottom_offset	= 0;
								}
								if(data->vui_parameters_present_flag && data->vui_parameters != NULL){
									const STNBAvc_vui_parameters* vui_parameters = data->vui_parameters; 
									obj->picDescBase.params.sps.vui_parameters.iSeq++;
									obj->picDescBase.params.sps.vui_parameters.time_scale				= vui_parameters->time_scale;
									obj->picDescBase.params.sps.vui_parameters.num_units_in_tick		= vui_parameters->num_units_in_tick;
									obj->picDescBase.params.sps.vui_parameters.fixed_frame_rate_flag	= vui_parameters->fixed_frame_rate_flag;
								}
								//sync picBaseDesc
								NBAvcParser_syncPicDescBase_(obj);
							}
						}
						break;
					case 8: //pic_parameter_set_rbsp (nal_unit_type = 8)
						{
							IF_NBASSERT(STNBAvc_pic_parameter_set_rbsp* pic_parameter_set_rbsp = (STNBAvc_pic_parameter_set_rbsp*)dataSt;)
							NBASSERT(obj->params.data.pic_parameter_set_rbsp == pic_parameter_set_rbsp)
							NBASSERT(!releaseSt)
						}
						break;
					case 1: //IDR
					case 5: //non-IDR
					case 19: //non-IDR
						{
							STNBAvc_slice_layer_without_partitioning_rbsp* slice_layer_without_partitioning_rbsp = (STNBAvc_slice_layer_without_partitioning_rbsp*)dataSt;
							NBASSERT(obj->params.data.slice_layer_without_partitioning_rbsp == slice_layer_without_partitioning_rbsp)
							NBASSERT(!releaseSt)
							//set props param
							if(slice_layer_without_partitioning_rbsp != NULL && slice_layer_without_partitioning_rbsp->slice_header != NULL){
								const STNBAvc_slice_header* hdr = slice_layer_without_partitioning_rbsp->slice_header;
								obj->picDescBase.params.slice_header.iSeq++;
								obj->picDescBase.params.slice_header.field_pic_flag = hdr->field_pic_flag; 
								//sync picBaseDesc
								NBAvcParser_syncPicDescBase_(obj);
							}
						}
						break;
					default:
						NBASSERT(releaseSt)
						break;
				}
			}
			//Release root
			if(dataSt != NULL && releaseSt){
				(*obj->curNALU.map->releaseSt)(dataSt);
				dataSt = NULL;
			}
		}
	}
	return r;
}

	
BOOL NBAvcParser_feedNALU(STNBAvcParser* obj, const void* prefix, const UI32 prefixSz, const void* pay, const UI32 paySz, const BOOL picDescBaseOnly){
	BOOL r = FALSE;
	STNBAvcBitstream bs;
	NBAvcBitstream_init(&bs);
	NBAvcBitstream_setAsPayload(&bs, prefix, prefixSz, pay, paySz);
	r = NBAvcParser_feedNALUBitstream_(obj, &bs, picDescBaseOnly);
	NBAvcBitstream_release(&bs);
	return r;
}

BOOL NBAvcParser_feedNALUChunked(STNBAvcParser* obj, const void* prefix, const UI32 prefixSz, const STNBDataPtr* chunks, const SI32 chunksSz, const BOOL picDescBaseOnly){ //'picDescBaseOnly' = quick-parse only data necesary for 'NBAvcParser_getPicDescBase()' (no image data)
	BOOL r = FALSE;
	STNBAvcBitstream bs;
	NBAvcBitstream_init(&bs);
	NBAvcBitstream_setAsChunks(&bs, prefix, prefixSz, chunks, chunksSz);
	r = NBAvcParser_feedNALUBitstream_(obj, &bs, picDescBaseOnly);
	NBAvcBitstream_release(&bs);
	return r;
}

BOOL NBAvcParser_feedEnd(STNBAvcParser* obj){
	return (obj->curNALU.remain.bytesCount == 0 && obj->curNALU.remain.bitsCount == 0);
}

//Cfg

void NBAvcParser_syncPicDescBase_(STNBAvcParser* obj){
	BOOL changed = FALSE;
	STNBAvcPicDescBase data = obj->picDescBase.cur.data;
	if(obj->picDescBase.params.sps.iSeq != 0){
		const UI8 ChromaArrayType = (obj->picDescBase.params.sps.separate_colour_plane_flag ? 0 : obj->picDescBase.params.sps.chroma_format_idc);
		UI16 CropUnitX = 1, CropUnitY = 2 - (UI16)obj->picDescBase.params.sps.frame_mbs_only_flag; 
		//NBASSERT(ChromaArrayType >= 0 && ChromaArrayType <= 3) //ToDo: remove. Note: spec does not limit the value.
		if(ChromaArrayType != 0){
			//Table 6-1 – SubWidthC, and SubHeightC values derived from chroma_format_idc and separate_colour_plane_flag
			UI16 SubWidthC = 0, SubHeightC = 0;
			if(obj->picDescBase.params.sps.chroma_format_idc == 1 && obj->picDescBase.params.sps.separate_colour_plane_flag == 0){
				SubWidthC = SubHeightC = 2;
			} else if(obj->picDescBase.params.sps.chroma_format_idc == 2 && obj->picDescBase.params.sps.separate_colour_plane_flag == 0){
				SubWidthC = 2; SubHeightC = 1;
			} else if(obj->picDescBase.params.sps.chroma_format_idc == 3 && obj->picDescBase.params.sps.separate_colour_plane_flag == 0){
				SubWidthC = SubHeightC = 1;
			}
            if(SubWidthC > 0 && SubHeightC > 0){
                CropUnitX = SubWidthC;
                CropUnitY = SubHeightC * (UI16)( 2 - obj->picDescBase.params.sps.frame_mbs_only_flag );
            }
		}
		//width
		{
			const UI16 PicWidthInMbs			= (UI16)obj->picDescBase.params.sps.pic_width_in_mbs_minus1 + 1; NBASSERT(PicWidthInMbs <= (0xFFFF / 16)) //must fit in 16-bits
			const UI16 PicWidthInSamplesLuma	= (PicWidthInMbs * 16) - (UI16)((obj->picDescBase.params.sps.frame_crop_left_offset + obj->picDescBase.params.sps.frame_crop_right_offset) * CropUnitX);
			if(data.w != PicWidthInSamplesLuma){
				data.w = PicWidthInSamplesLuma;
				changed = TRUE;
			}
		}
		//fps
		if(obj->picDescBase.params.sps.vui_parameters_present_flag && obj->picDescBase.params.sps.vui_parameters.iSeq != 0){
			if(obj->picDescBase.params.sps.vui_parameters.num_units_in_tick > 0){
				const UI32 MaxFPS		= (obj->picDescBase.params.sps.vui_parameters.time_scale / (2 * obj->picDescBase.params.sps.vui_parameters.num_units_in_tick)) + ((obj->picDescBase.params.sps.vui_parameters.time_scale % (2 * obj->picDescBase.params.sps.vui_parameters.num_units_in_tick)) != 0 ? 1 : 0);
				const BOOL isFixedFps	= obj->picDescBase.params.sps.vui_parameters.fixed_frame_rate_flag;
				if(data.fpsMax != MaxFPS){
					data.fpsMax = (UI16)MaxFPS;
					changed = TRUE;
				}
				if(data.isFixedFps != isFixedFps){
					data.isFixedFps = isFixedFps;
					changed = TRUE;
				}
			}
		}
		//height
		if(obj->picDescBase.params.slice_header.iSeq != 0){
			const UI16 PicHeightInMapUnits		= (UI16)obj->picDescBase.params.sps.pic_height_in_map_units_minus1 + 1; NBASSERT(PicHeightInMapUnits <= (0xFFFF / 16)) //must fit in 16-bits
			const UI16 FrameHeightInMbs			= (UI16)( 2 - obj->picDescBase.params.sps.frame_mbs_only_flag ) * PicHeightInMapUnits; NBASSERT(FrameHeightInMbs <= (0xFFFF / 16)) //must fit in 16-bits
			const UI16 PicHeightInMbs			= FrameHeightInMbs / ( 1 + obj->picDescBase.params.slice_header.field_pic_flag ); NBASSERT(PicHeightInMbs <= (0xFFFF / 16)) //must fit in 16-bits
			const UI16 PicHeightInSamplesLuma	= (PicHeightInMbs * 16) - (UI16)((obj->picDescBase.params.sps.frame_crop_top_offset + obj->picDescBase.params.sps.frame_crop_bottom_offset) * CropUnitY);
			if(data.h != PicHeightInSamplesLuma){
				data.h = PicHeightInSamplesLuma;
				changed = TRUE;
			}
		}
		//avcProfile
		if(data.avcProfile != obj->picDescBase.params.sps.profile_idc){
			data.avcProfile = obj->picDescBase.params.sps.profile_idc;
			changed = TRUE;
		}
		//profConstraints
		if(data.profConstraints != obj->picDescBase.params.sps.constraint_set){
			data.profConstraints = obj->picDescBase.params.sps.constraint_set;
			changed = TRUE;
		}
		//profLevel
		if(data.profLevel != obj->picDescBase.params.sps.level_idc){
			data.profLevel = obj->picDescBase.params.sps.level_idc;
			changed = TRUE;
		}
		//chromaFmt
		if(data.chromaFmt != obj->picDescBase.params.sps.chroma_format_idc){
			data.chromaFmt = obj->picDescBase.params.sps.chroma_format_idc;
			changed = TRUE;
		}
		//bitDepthLuma
		if(data.bitDepthLuma != (obj->picDescBase.params.sps.bit_depth_luma_minus8 + 8)){
			data.bitDepthLuma = (obj->picDescBase.params.sps.bit_depth_luma_minus8 + 8);
			changed = TRUE;
		}
		//bitDepthChroma
		if(data.bitDepthChroma != (obj->picDescBase.params.sps.bit_depth_chroma_minus8 + 8)){
			data.bitDepthChroma = (obj->picDescBase.params.sps.bit_depth_chroma_minus8 + 8);
			changed = TRUE;
		}
	}
	if(changed){
		obj->picDescBase.cur.data = data;
		obj->picDescBase.cur.iSeq++;
	}
}

BOOL NBAvcParser_getPicDescBase(STNBAvcParser* obj, const UI32 iSeqFilter, STNBAvcPicDescBase* dst, UI32* dstSeqIdx){
	BOOL r = FALSE;
	if(obj->picDescBase.cur.iSeq != 0 && obj->picDescBase.cur.iSeq != iSeqFilter){
		if(dst != NULL){
            NBStruct_stRelease(NBAvcPicDescBase_getSharedStructMap(), dst, sizeof(*dst));
            NBStruct_stClone(NBAvcPicDescBase_getSharedStructMap(), &obj->picDescBase.cur.data, sizeof(obj->picDescBase.cur.data), dst, sizeof(*dst));
		}
		if(dstSeqIdx != NULL){
			*dstSeqIdx = obj->picDescBase.cur.iSeq;
		}
		r = TRUE;
	}
	return r;
}
