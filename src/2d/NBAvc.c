//
//  NBAvc.c
//  nbframework
//
//  Created by Marcos Ortega on 9/21/20.
//

#include "nb/NBFrameworkPch.h"
#include "nb/2d/NBAvc.h"
//
#include "nb/core/NBMemory.h"
#include "nb/core/NBString.h"
//
#include <math.h>

//#define NB_AVC_PRINT_DBG

#if defined(CONFIG_NB_DESHABILITAR_IMPRESIONES_PRINTF) || !defined(NB_AVC_PRINT_DBG)
//U/F
#	define NBAvcBitstream_readU8_(ST_PTR, FIELD, BS, BITS)		(ST_PTR)->FIELD = NBAvcBitstream_readU8(BS, BITS);
#	define NBAvcBitstream_readU16_(ST_PTR, FIELD, BS, BITS)		(ST_PTR)->FIELD = NBAvcBitstream_readU16(BS, BITS);
#	define NBAvcBitstream_readU24_(ST_PTR, FIELD, BS, BITS)		(ST_PTR)->FIELD = NBAvcBitstream_readU24(BS, BITS);
#	define NBAvcBitstream_readU32_(ST_PTR, FIELD, BS, BITS)		(ST_PTR)->FIELD = NBAvcBitstream_readU32(BS, BITS);
//u(v)
#	define NBAvcBitstream_readU16v_(ST_PTR, FIELD, BS, BITS)	(ST_PTR)->FIELD = NBAvcBitstream_readU16v(BS, BITS);
#	define NBAvcBitstream_readU24v_(ST_PTR, FIELD, BS, BITS)	(ST_PTR)->FIELD = NBAvcBitstream_readU24v(BS, BITS);
#	define NBAvcBitstream_readU32v_(ST_PTR, FIELD, BS, BITS)	(ST_PTR)->FIELD = NBAvcBitstream_readU32v(BS, BITS);
//Golomb
#	define NBAvcBitstream_readGolombUE64_(ST_PTR, FIELD, BS)	(ST_PTR)->FIELD = NBAvcBitstream_readGolombUE64(BS);
#	define NBAvcBitstream_readGolombSE64_(ST_PTR, FIELD, BS)	(ST_PTR)->FIELD = NBAvcBitstream_readGolombSE64(BS);
#else
//U/F
#	define NBAvcBitstream_readU8_(ST_PTR, FIELD, BS, BITS)		{ (ST_PTR)->FIELD = NBAvcBitstream_readU8(BS, BITS); PRINTF_INFO("NBAvcBitstream, %s%s%s%u (%d bits, '%s').\n", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_OVERFLOWED_READ ? "[overflowed-read] " : "", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_OVERFLOWED_VALUE ? "[overflowed-value] " : "", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_VALUE_LOGIC_ERROR ? "[value-logic-err] " : "", (ST_PTR)->FIELD, BITS, #FIELD); }
#	define NBAvcBitstream_readU16_(ST_PTR, FIELD, BS, BITS)		{ (ST_PTR)->FIELD = NBAvcBitstream_readU16(BS, BITS); PRINTF_INFO("NBAvcBitstream, %s%s%s%u (%d bits, '%s').\n", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_OVERFLOWED_READ ? "[overflowed-read] " : "", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_OVERFLOWED_VALUE ? "[overflowed-value] " : "", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_VALUE_LOGIC_ERROR ? "[value-logic-err] " : "", (ST_PTR)->FIELD, BITS, #FIELD); }
#	define NBAvcBitstream_readU24_(ST_PTR, FIELD, BS, BITS)		{ (ST_PTR)->FIELD = NBAvcBitstream_readU24(BS, BITS); PRINTF_INFO("NBAvcBitstream, %s%s%s%u (%d bits, '%s').\n", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_OVERFLOWED_READ ? "[overflowed-read] " : "", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_OVERFLOWED_VALUE ? "[overflowed-value] " : "", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_VALUE_LOGIC_ERROR ? "[value-logic-err] " : "", (ST_PTR)->FIELD, BITS, #FIELD); }
#	define NBAvcBitstream_readU32_(ST_PTR, FIELD, BS, BITS)		{ (ST_PTR)->FIELD = NBAvcBitstream_readU32(BS, BITS); PRINTF_INFO("NBAvcBitstream, %s%s%s%u (%d bits, '%s').\n", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_OVERFLOWED_READ ? "[overflowed-read] " : "", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_OVERFLOWED_VALUE ? "[overflowed-value] " : "", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_VALUE_LOGIC_ERROR ? "[value-logic-err] " : "", (ST_PTR)->FIELD, BITS, #FIELD); }
//u(v)
#	define NBAvcBitstream_readU16v_(ST_PTR, FIELD, BS, BITS)	{ (ST_PTR)->FIELD = NBAvcBitstream_readU16v(BS, BITS); PRINTF_INFO("NBAvcBitstream, %s%s%s%u (uv, %d bits, '%s').\n", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_OVERFLOWED_READ ? "[overflowed-read] " : "", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_OVERFLOWED_VALUE ? "[overflowed-value] " : "", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_VALUE_LOGIC_ERROR ? "[value-logic-err] " : "", (ST_PTR)->FIELD, BITS, #FIELD); }
#	define NBAvcBitstream_readU24v_(ST_PTR, FIELD, BS, BITS)	{ (ST_PTR)->FIELD = NBAvcBitstream_readU24v(BS, BITS); PRINTF_INFO("NBAvcBitstream, %s%s%s%u (uv, %d bits, '%s').\n", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_OVERFLOWED_READ ? "[overflowed-read] " : "", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_OVERFLOWED_VALUE ? "[overflowed-value] " : "", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_VALUE_LOGIC_ERROR ? "[value-logic-err] " : "", (ST_PTR)->FIELD, BITS, #FIELD); }
#	define NBAvcBitstream_readU32v_(ST_PTR, FIELD, BS, BITS)	{ (ST_PTR)->FIELD = NBAvcBitstream_readU32v(BS, BITS); PRINTF_INFO("NBAvcBitstream, %s%s%s%u (uv, %d bits, '%s').\n", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_OVERFLOWED_READ ? "[overflowed-read] " : "", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_OVERFLOWED_VALUE ? "[overflowed-value] " : "", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_VALUE_LOGIC_ERROR ? "[value-logic-err] " : "", (ST_PTR)->FIELD, BITS, #FIELD); }
//Golomb
#	define NBAvcBitstream_readGolombUE64_(ST_PTR, FIELD, BS)	{ (ST_PTR)->FIELD = NBAvcBitstream_readGolombUE64(BS); PRINTF_INFO("NBAvcBitstream, %s%s%s%llu (ue, '%s').\n", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_OVERFLOWED_READ ? "[overflowed-read] " : "", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_OVERFLOWED_VALUE ? "[overflowed-value] " : "", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_VALUE_LOGIC_ERROR ? "[value-logic-err] " : "", (ST_PTR)->FIELD, #FIELD); }
#	define NBAvcBitstream_readGolombSE64_(ST_PTR, FIELD, BS)	{ (ST_PTR)->FIELD = NBAvcBitstream_readGolombSE64(BS); PRINTF_INFO("NBAvcBitstream, %s%s%s%lld (se, '%s').\n", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_OVERFLOWED_READ ? "[overflowed-read] " : "", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_OVERFLOWED_VALUE ? "[overflowed-value] " : "", (BS)->stopFlags & NB_AVC_BITSTREAM_STOP_VALUE_LOGIC_ERROR ? "[value-logic-err] " : "", (ST_PTR)->FIELD, #FIELD); }
#endif

//NALUnit defs

const static STNBAvcNaluDef _naluDefs[] = {
	{ 0, "non-VCL", "Unspecified" },
	{ 1, "VCL", "Coded slice of a non-IDR picture slice_layer_without_partitioning_rbsp( )" },
	{ 2, "VCL", "Coded slice data partition A slice_data_partition_a_layer_rbsp( )" },
	{ 3, "VCL", "Coded slice data partition B slice_data_partition_b_layer_rbsp( )" },
	{ 4, "VCL", "Coded slice data partition C slice_data_partition_c_layer_rbsp( )" },
	{ 5, "VCL", "Coded slice of an IDR picture slice_layer_without_partitioning_rbsp( )" },
	{ 6, "non-VCL", "Supplemental enhancement information (SEI) sei_rbsp( )" },
	{ 7, "non-VCL", "Sequence parameter set seq_parameter_set_rbsp( )" },
	{ 8, "non-VCL", "Picture parameter set pic_parameter_set_rbsp( )" },
	{ 9, "non-VCL", "Access unit delimiter access_unit_delimiter_rbsp( )" },
	{ 10, "non-VCL", "End of sequence end_of_seq_rbsp( )" },
	{ 11, "non-VCL", "End of stream end_of_stream_rbsp( )" },
	{ 12, "non-VCL", "Filler data filler_data_rbsp( )" },
	{ 13, "non-VCL", "Sequence parameter set extension seq_parameter_set_extension_rbsp( )" },
	{ 14, "non-VCL", "Prefix NAL unit prefix_nal_unit_rbsp( )" },
	{ 15, "non-VCL", "Subset sequence parameter set subset_seq_parameter_set_rbsp( )" },
	{ 16, "non-VCL", "Depth parameter set depth_parameter_set_rbsp( )" },
	{ 17, "non-VCL", "Reserved" },
	{ 18, "non-VCL", "Reserved" },
	{ 19, "non-VCL", "Coded slice of an auxiliary coded picture without partitioning slice_layer_without_partitioning_rbsp( )" },
	{ 20, "non-VCL", "Coded slice extension slice_layer_extension_rbsp( )" },
	{ 21, "non-VCL", "Coded slice extension for depth view components slice_layer_extension_rbsp( )" },
	{ 22, "non-VCL", "Reserved" },
	{ 23, "non-VCL", "Reserved" },
	{ 24, "Stap-A", "Single-time aggregation packet" },
	{ 25, "Stap-B", "Single-time aggregation packet" },	
	{ 26, "MTAP16", "Multi-time aggregation packet" },
	{ 27, "MTAP24", "Multi-time aggregation packet" },
	{ 28, "FU-A", "Fragmentation unit" },
	{ 29, "FU-B", "Fragmentation unit" },
	{ 30, "non-VCL", "Unspecified" },
	{ 31, "non-VCL", "Unspecified" },
};

const STNBAvcNaluDef* NBAvc_getNaluDef(const UI8 type){
	if(type >= 0 && type < 32){
		return &_naluDefs[type];
	}
	return NULL;
}

//First byte

STNBAvcNaluHdr NBAvc_getNaluHdr(const BYTE firstByte){
	STNBAvcNaluHdr r;
	NBMemory_setZeroSt(r, STNBAvcNaluHdr);
	r.forbiddenZeroBit	= ((firstByte >> 7) & 0x1);
	r.refIdc			= ((firstByte >> 5) & 0x2); //zero means 'not-image-related' NAL package
	r.type				= (firstByte & 0x1F);
	return r;
}

//

BOOL NBAvc_more_rbsp_data(STNBAvcBitstream* bs){
	BOOL r = FALSE;
	if(NBAvcBitstream_getBitsRemainCount(bs) > 0){
		const UI32 bsPos = NBAvcBitstream_getCurGlobalPos(bs);
		const UI32 bsBitsRemain = NBAvcBitstream_getBitsRemainCount(bs);
		UI8 cur = 0; UI32 bs2Pos = 0, bs2BitsRemain = 0;
		STNBAvcBitstream bs2;
		NBAvcBitstream_init(&bs2);
		NBAvcBitstream_setAsOther(&bs2, bs);
		NBAvcBitstream_moveToEnd(&bs2);
		//Searched for the last (least significant, right-most) bit equal to 1 that is present in the RBSP
		r = TRUE;
		//
		while(NBAvcBitstream_moveToPrevByte(&bs2) && bsPos <= NBAvcBitstream_getCurGlobalPos(&bs2) && !bs->stopFlags){
			cur = NBAvcBitstream_getCurByte(&bs2);
			if((cur & 0x1) == 0x1){
				//xxxx-xxx1 found
				bs2BitsRemain = 1; //trailBits
				break;
			} else if((cur & 0x2) == 0x2){
				//xxxx-xx10 found
				bs2BitsRemain = 2; //trailBits
				break;
			} else if((cur & 0x4) == 0x4){
				//xxxx-x100 found
				bs2BitsRemain = 3; //trailBits
				break;
			} else if((cur & 0x8) == 0x8){
				//xxxx-1000 found
				bs2BitsRemain = 4; //trailBits
				break;
			} else if((cur & 0x10) == 0x10){
				//xxx1-0000 found
				bs2BitsRemain = 5; //trailBits
				break;
			} else if((cur & 0x20) == 0x20){
				//xx10-0000 found
				bs2BitsRemain = 6; //trailBits
				break;
			} else if((cur & 0x40) == 0x40){
				//x100-0000 found
				bs2BitsRemain = 7; //trailBits
				break;
			} else if((cur & 0x80) == 0x80){
				//1000-0000 found
				bs2BitsRemain = 8; //trailBits
				break;
			} else if(cur != 0x0){
				NBASSERT(FALSE) //Should never reach this point
				r = FALSE;
				break;
			}
		}
		//Remain
		bs2Pos = NBAvcBitstream_getCurGlobalPos(&bs2);
		r = (r && ((bsPos < bs2Pos) || (bsPos == bs2Pos && bsBitsRemain > bs2BitsRemain)));
		//
		NBAvcBitstream_release(&bs2);
	}
	return r;
}

//NAL-Unit definition

//ISO-14496-10
static const STNBAvcDefNalu _nalUnitTypesStr[] = {
	{ 0, "Unspecified", "Unspecified" }
	, { 1, "Coded slice of a non-IDR picture", "slice_layer_without_partitioning_rbsp( )" }
	, { 2, "Coded slice data partition A", "slice_data_partition_a_layer_rbsp( )" }
	, { 3, "Coded slice data partition B", "slice_data_partition_b_layer_rbsp( )" }
	, { 4, "Coded slice data partition C", "slice_data_partition_c_layer_rbsp( )" }
	, { 5, "Coded slice of an IDR picture", "slice_layer_without_partitioning_rbsp( )" }
	, { 6, "Supplemental enhancement information (SEI)", "sei_rbsp( )" }
	, { 7, "Sequence parameter set", "seq_parameter_set_rbsp( )" }
	, { 8, "Picture parameter set", "pic_parameter_set_rbsp( )" }
	, { 9, "Access unit delimiter", "access_unit_delimiter_rbsp( )" }
	, { 10, "End of sequence", "end_of_seq_rbsp( )" }
	, { 11, "End of stream", "end_of_stream_rbsp( )" }
	, { 12, "Filler data", "filler_data_rbsp( )" }
	//Reserved 13-23 (ISO-14496-10)
	, { 13, "Reserved", "Reserved" }
	, { 14, "Reserved", "Reserved" }
	, { 15, "Reserved", "Reserved" }
	, { 16, "Reserved", "Reserved" }
	, { 17, "Reserved", "Reserved" }
	, { 18, "Reserved", "Reserved" }
	, { 19, "Reserved", "Reserved" }
	, { 20, "Reserved", "Reserved" }
	, { 21, "Reserved", "Reserved" }
	, { 22, "Reserved", "Reserved" }
	, { 23, "Reserved", "Reserved" }
	//Unspecified 24-31 (ISO-14496-10)
	, { 24, "Unspecified", "Unspecified" }
	, { 25, "Unspecified", "Unspecified" }
	, { 26, "Unspecified", "Unspecified" }
	, { 27, "Unspecified", "Unspecified" }
	, { 28, "Unspecified", "Unspecified" }
	, { 29, "Unspecified", "Unspecified" }
	, { 30, "Unspecified", "Unspecified" }
	, { 31, "Unspecified", "Unspecified" }
	//end
};

const STNBAvcDefNalu* NBAvc_getDefNalu(const UI8 uid){
	const STNBAvcDefNalu* r = NULL;
	NBASSERT((sizeof(_nalUnitTypesStr) / sizeof(_nalUnitTypesStr[0])) == 32)
	if(uid < 32){
		r = &_nalUnitTypesStr[uid];
	}
	return r;
}

//Aspect ratio

static const STNBAvcAspectRatioIdcDef _avcAspectRatioIdcDefs[] = {
	{ "Unspecified", 0, 0, "Unspecified" },
	{ "1:1", 1, 1, "1280x720 16:9 frame without overscan\n1920x1080 16:9 frame without overscan (cropped from 1920x1088)\n640x480 4:3 frame without overscan" },
	{ "12:11", 12, 11, "720x576 4:3 frame with horizontal overscan\n352x288 4:3 frame without overscan" },
	{ "10:11", 10, 11, "720x480 4:3 frame with horizontal overscan\n352x240 4:3 frame without overscan" },
	{ "16:11", 16, 11, "720x576 16:9 frame with horizontal overscan\n540x576 4:3 frame with horizontal overscan" },
	{ "40:33", 40, 33, "720x480 16:9 frame with horizontal overscan\n540x480 4:3 frame with horizontal overscan" },
	{ "24:11", 24, 11, "352x576 4:3 frame without overscan\n540x576 16:9 frame with horizontal overscan" },
	{ "20:11", 20, 11, "352x480 4:3 frame without overscan\n480x480 16:9 frame with horizontal overscan" },
	{ "32:11", 32, 11, "352x576 16:9 frame without overscan" },
	{ "80:33", 80, 33, "352x480 16:9 frame without overscan" },
	{ "18:11", 18, 11, "480x576 4:3 frame with horizontal overscan" },
	{ "15:11", 15, 11, "480x480 4:3 frame with horizontal overscan" },
	{ "64:33", 64, 33, "540x576 16:9 frame with horizontal overscan" },
	{ "160:99", 160, 99, "540x480 16:9 frame with horizontal overscan" },
	//14..254 Reserved
	{ "Reserved", 0, 0, "Reserved" },
	//Extended_SAR
	{ "Extended_SAR", 0, 0, "Extended_SAR" }
};

const STNBAvcAspectRatioIdcDef* NBAvc_getAspectRatioIdcDef(const UI8 idc){ //ENAvc_aspect_ratio_idc
	const STNBAvcAspectRatioIdcDef* r = NULL;
	if(idc < ((sizeof(_avcAspectRatioIdcDefs) / sizeof(_avcAspectRatioIdcDefs[0])) - 2)){
		//Explicit
		r = &_avcAspectRatioIdcDefs[idc];
	} else if(idc == 255){
		//Extended_SAR
		r = &_avcAspectRatioIdcDefs[(sizeof(_avcAspectRatioIdcDefs) / sizeof(_avcAspectRatioIdcDefs[0])) - 1];
	} else {
		//Reserved
		r = &_avcAspectRatioIdcDefs[(sizeof(_avcAspectRatioIdcDefs) / sizeof(_avcAspectRatioIdcDefs[0])) - 2];
	}
	return r;
}

//slice type

static const STNBAvcSliceTypeDef _avcSliceTypeDefs[] = {
	{ ENAvc_slice_type_0_P, "P", "P slice" }, 
	{ ENAvc_slice_type_1_B, "B", "B slice" },
	{ ENAvc_slice_type_2_I, "I", "I slice" },
	{ ENAvc_slice_type_3_SP, "SP", "SP slice" },
	{ ENAvc_slice_type_4_SI, "SI", "SI slice" },
	{ ENAvc_slice_type_5_P, "P", "P slice" },
	{ ENAvc_slice_type_6_B, "B", "B slice" },
	{ ENAvc_slice_type_7_I, "I", "I slice" },
	{ ENAvc_slice_type_8_SP, "SP", "SP slice" },
	{ ENAvc_slice_type_9_SI, "SI" "SI slice" }
};

const STNBAvcSliceTypeDef* NBAvc_getSliceTypeDef(const UI8 type){ //ENAvc_slice_type
	const STNBAvcSliceTypeDef* r = NULL;
	if(type >= 0 && type < (sizeof(_avcSliceTypeDefs) / sizeof(_avcSliceTypeDefs[0]))){
		r = &_avcSliceTypeDefs[type];
	}
	return r;
}

//video_format

static const STNBAvcVideoFormatDef _avcVideoFormatsDefs[] = {
	{ ENAvc_video_format_0_Component, "Component" },
	{ ENAvc_video_format_1_PAL, "PAL" },
	{ ENAvc_video_format_2_NTSC, "NTSC" },
	{ ENAvc_video_format_3_SECAM, "SECAM" },
	{ ENAvc_video_format_4_MAC, "MAC" },
	{ ENAvc_video_format_5_Unspecified, "Unspecified video format" },
	{ ENAvc_video_format_6, "Reserved" },
	{ ENAvc_video_format_7, "Reserved" }
};

const STNBAvcVideoFormatDef* NBAvc_getVideoFormatDef(const UI8 videoFormat){ //ENAvc_video_format
	const STNBAvcVideoFormatDef* r = NULL;
	if(videoFormat < ((sizeof(_avcVideoFormatsDefs) / sizeof(_avcVideoFormatsDefs[0])))){
		//Explicit
		r = &_avcVideoFormatsDefs[videoFormat];
	}
	return r;
}

//rbsp_trailing_bits

void* NBAvc_rbsp_trailing_bits_createSt(void);
void NBAvc_rbsp_trailing_bits_releaseSt(void* obj);
BOOL NBAvc_rbsp_trailing_bits_loadBits(STNBAvcBitstream* bs, const STNBAvcStateParams* params, void* pData, const BOOL picDescBaseOnly);

static const STNBAvcNaluMap NBAvcMap_rbsp_trailing_bits = {
	"rbsp_trailing_bits"
	//Methods
	, NBAvc_rbsp_trailing_bits_createSt
	, NBAvc_rbsp_trailing_bits_releaseSt
	, NBAvc_rbsp_trailing_bits_loadBits
};

void* NBAvc_rbsp_trailing_bits_createSt(void){
	STNBAvc_rbsp_trailing_bits* r = NBMemory_allocType(STNBAvc_rbsp_trailing_bits);
	NBMemory_setZeroSt(*r, STNBAvc_rbsp_trailing_bits);
	return r;
}

void NBAvc_rbsp_trailing_bits_releaseSt(void* pObj){
	STNBAvc_rbsp_trailing_bits* obj = (STNBAvc_rbsp_trailing_bits*)pObj;
	if(obj != NULL){
		NBMemory_free(obj);
		obj = NULL;
	}
}

BOOL NBAvc_rbsp_trailing_bits_loadBits(STNBAvcBitstream* bs, const STNBAvcStateParams* params, void* pData, const BOOL picDescBaseOnly){
	BOOL r = FALSE;
	if(bs != NULL && !bs->stopFlags){
		const UI8 bistRemainCount = NBAvcBitstream_getBitsRemainCount(bs);
		const UI8 curByte = NBAvcBitstream_getCurByte(bs);
		STNBAvc_rbsp_trailing_bits data;
		NBMemory_setZeroSt(data, STNBAvc_rbsp_trailing_bits);
		/*{
			NBAvcBitstream_readU8_(&data, rbsp_stop_one_bit, bs, 1);
			data.rbsp_alignment_zero_bit = 0;
			if(data.rbsp_stop_one_bit){
				while(bs->bitsRemain < 8 && !data.rbsp_alignment_zero_bit && !bs->stopFlags){
					NBAvcBitstream_readU8_(&data, rbsp_alignment_zero_bit, bs, 1);
				}
				r = (data.rbsp_stop_one_bit == 1 && data.rbsp_alignment_zero_bit == 0 && bs->bitsRemain == 8 && !bs->overflowed);
			}
		}*/
		switch(bistRemainCount) {
			case 8:
				r = ((data.trailing_bits = (curByte & 0xFF)) == 0x80); //1000-0000 
				break;
			case 7:
				r = ((data.trailing_bits = (curByte & 0x7F)) == 0x40); //x100-0000 
				break;
			case 6:
				r = ((data.trailing_bits = (curByte & 0x3F)) == 0x20); //xx10-0000
				break;
			case 5:
				r = ((data.trailing_bits = (curByte & 0x1F)) == 0x10); //xxx1-0000
				break;
			case 4:
				r = ((data.trailing_bits = (curByte & 0xF)) == 0x8); //xxxx-1000
				break;
			case 3:
				r = ((data.trailing_bits = (curByte & 0x7)) == 0x4); //xxxx-x100
				break;
			case 2:
				r = ((data.trailing_bits = (curByte & 0x3)) == 0x2); //xxxx-xx10
				break;
			case 1:
				r = ((data.trailing_bits = (curByte & 0x1)) == 0x1); //xxxx-xxx1
				break;
			default:
				NBASSERT(FALSE)
				break;
		}
		//move to next byte
		if(r){
			NBAvcBitstream_moveToNextByte(bs);
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		} else {
			PRINTF_INFO("NBAvc_rbsp_trailing_bits_loadBits failed with %d bits and extra bytes remainig.\n", (SI32)bistRemainCount);
			//print remain
			//Bytes format
			/*{ 
				const char* hex = "0123456789abcdef";
				const UI32 dataSz = (UI32)(bs->srcAfterEnd - bs->src);
				UI32 i, zSeq = 0; const BYTE* data = (const BYTE*)bs->src;
				STNBString str;
				NBString_init(&str);
				for(i = 0; i < dataSz; i++){
					const BYTE b = data[i];
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
				}
				if((i % 64) != 0){
					NBString_concat(&str, " //");
					NBString_concatUI32(&str, i);
					NBString_concatByte(&str, '\n');
				}
				PRINTF_INFO("NBAvc_rbsp_trailing_bits_loadBits, remaining %d bytes:\n%s\n\n", dataSz, str.str);
				NBString_release(&str);
			}*/
			//Bits format
			/*{
				const UI32 dataSz = (UI32)(bs->srcAfterEnd - bs->src);
				UI32 i; const BYTE* data = (const BYTE*)bs->src;
				STNBString str;
				NBString_init(&str);
				for(i = 0; i < dataSz; i++){
					const BYTE b = data[i];
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
				}
				if((i % 8) != 0){
					NBString_concat(&str, " //");
					NBString_concatUI32(&str, i);
					NBString_concatByte(&str, '\n');
				}
				PRINTF_INFO("NBAvc_rbsp_trailing_bits_loadBits, remaining %d bits:\n%s\n\n", dataSz, str.str);
				NBString_release(&str);
			}*/
#		endif
		}
		//Set data
		if(pData != NULL){
			*((STNBAvc_rbsp_trailing_bits*)pData) = data; 
		}
	}
	return r;
}

//hrd_parameters

void* NBAvc_hrd_parameters_createSt(void);
void NBAvc_hrd_parameters_releaseSt(void* obj);
BOOL NBAvc_hrd_parameters_loadBits(STNBAvcBitstream* bs, const STNBAvcStateParams* params, void* pData, const BOOL picDescBaseOnly);

static const STNBAvcNaluMap NBAvcMap_hrd_parameters = {
	"hrd_parameters"
	//Methods
	, NBAvc_hrd_parameters_createSt
	, NBAvc_hrd_parameters_releaseSt
	, NBAvc_hrd_parameters_loadBits
};

void* NBAvc_hrd_parameters_createSt(void){
	STNBAvc_hrd_parameters* r = NBMemory_allocType(STNBAvc_hrd_parameters);
	NBMemory_setZeroSt(*r, STNBAvc_hrd_parameters);
	return r;
}

void NBAvc_hrd_parameters_releaseSt(void* pObj){
	STNBAvc_hrd_parameters* obj = (STNBAvc_hrd_parameters*)pObj;
	if(obj != NULL){
		if(obj->bit_rate_value_minus1 != NULL){
			NBMemory_free(obj->bit_rate_value_minus1);
			obj->bit_rate_value_minus1 = NULL;
		}
		if(obj->cpb_size_value_minus1 != NULL){
			NBMemory_free(obj->cpb_size_value_minus1);
			obj->cpb_size_value_minus1 = NULL;
		}
		if(obj->cbr_flag != NULL){
			NBMemory_free(obj->cbr_flag);
			obj->cbr_flag = NULL;
		}
		NBMemory_free(obj);
		obj = NULL;
	}
}

BOOL NBAvc_hrd_parameters_loadBits(STNBAvcBitstream* bs, const STNBAvcStateParams* params, void* pData, const BOOL picDescBaseOnly){
	BOOL r = FALSE;
	if(bs != NULL && !bs->stopFlags && pData != NULL){
		STNBAvc_hrd_parameters* data = (STNBAvc_hrd_parameters*)pData;
		r = TRUE;
		NBAvcBitstream_readGolombUE64_(data, cpb_cnt_minus1, bs); // 0 ue(v)
		NBAvcBitstream_readU8_(data, bit_rate_scale, bs, 4); //0 u(4)
		NBAvcBitstream_readU8_(data, cpb_size_scale, bs, 4); //0 u(4)
		{
			UI64 i, size = data->cpb_cnt_minus1 + 1;
			//reuse or allocate
			{
				//bit_rate
				data->bit_rate_value_minus1Use		= (UI16)size;
				if(data->bit_rate_value_minus1Use > data->bit_rate_value_minus1Sz){
					if(data->bit_rate_value_minus1 != NULL){
						NBMemory_free(data->bit_rate_value_minus1);
						data->bit_rate_value_minus1 = NULL;
					}
					data->bit_rate_value_minus1		= NBMemory_allocTypes(UI64, data->bit_rate_value_minus1Use);
					data->bit_rate_value_minus1Sz	= data->bit_rate_value_minus1Use;
				}
				//cpb_size
				data->cpb_size_value_minus1Use		= (UI16)size;
				if(data->cpb_size_value_minus1Use > data->cpb_size_value_minus1Sz){
					if(data->cpb_size_value_minus1 != NULL){
						NBMemory_free(data->cpb_size_value_minus1);
						data->cpb_size_value_minus1 = NULL;
					}
					data->cpb_size_value_minus1		= NBMemory_allocTypes(UI64, data->cpb_size_value_minus1Use);
					data->cpb_size_value_minus1Sz	= data->cpb_size_value_minus1Use;
				}
				//cbr_flag
				data->cbr_flagUse					= (UI16)size;
				if(data->cbr_flagUse > data->cbr_flagSz){
					if(data->cbr_flag != NULL){
						NBMemory_free(data->cbr_flag);
						data->cbr_flag = NULL;
					}
					data->cbr_flag					= NBMemory_allocTypes(BOOL, data->cbr_flagUse);
					data->cbr_flagSz				= data->cbr_flagUse;
				}
			}
			//read
			for( i = 0; i < size; i++ ) {
				NBAvcBitstream_readGolombUE64_(data, bit_rate_value_minus1[i], bs); //[ SchedSelIdx ] 0 ue(v)
				NBAvcBitstream_readGolombUE64_(data, cpb_size_value_minus1[i], bs); //[ SchedSelIdx ] 0 ue(v)
				NBAvcBitstream_readU8_(data, cbr_flag[i], bs, 1); //[ SchedSelIdx ] 0 u(1)
			}
		}
		NBAvcBitstream_readU8_(data, initial_cpb_removal_delay_length_minus1, bs, 5); //0 u(5)
		NBAvcBitstream_readU8_(data, cpb_removal_delay_length_minus1, bs, 5); //0 u(5)
		NBAvcBitstream_readU8_(data, dpb_output_delay_length_minus1, bs, 5); //0 u(5)
		NBAvcBitstream_readU8_(data, time_offset_length, bs, 5); //0 u(5)
		//
		if(bs->stopFlags){
			r = FALSE;
		}
	}
	return r;
}

//vui_parameters

void* NBAvc_vui_parameters_createSt(void);
void NBAvc_vui_parameters_releaseSt(void* obj);
BOOL NBAvc_vui_parameters_loadBits(STNBAvcBitstream* bs, const STNBAvcStateParams* params, void* pData, const BOOL picDescBaseOnly);

static const STNBAvcNaluMap NBAvcMap_vui_parameters = {
	"vui_parameters"
	//Methods
	, NBAvc_vui_parameters_createSt
	, NBAvc_vui_parameters_releaseSt
	, NBAvc_vui_parameters_loadBits
};

void* NBAvc_vui_parameters_createSt(void){
	STNBAvc_vui_parameters* r = NBMemory_allocType(STNBAvc_vui_parameters);
	NBMemory_setZeroSt(*r, STNBAvc_vui_parameters);
	return r;
}

void NBAvc_vui_parameters_releaseSt(void* pObj){
	STNBAvc_vui_parameters* obj = (STNBAvc_vui_parameters*)pObj;
	if(obj != NULL){
		if(obj->nal_hrd_parameters != NULL){
			NBAvc_hrd_parameters_releaseSt(obj->nal_hrd_parameters);
			obj->nal_hrd_parameters = NULL;
		}
		if(obj->vcl_hrd_parameters != NULL){
			NBAvc_hrd_parameters_releaseSt(obj->vcl_hrd_parameters);
			obj->vcl_hrd_parameters = NULL;
		}
		NBMemory_free(obj);
		obj = NULL;
	}
}

BOOL NBAvc_vui_parameters_loadBits(STNBAvcBitstream* bs, const STNBAvcStateParams* params, void* pData, const BOOL picDescBaseOnly){
	BOOL r = FALSE;
	if(bs != NULL && !bs->stopFlags && pData != NULL){
		STNBAvc_vui_parameters* data = (STNBAvc_vui_parameters*)pData;
		r = TRUE;
		NBAvcBitstream_readU8_(data, aspect_ratio_info_present_flag, bs, 1); //0 u(1)
		if( data->aspect_ratio_info_present_flag ) {
			NBAvcBitstream_readU8_(data, aspect_ratio_idc, bs, 8); //0 u(8)
			if( data->aspect_ratio_idc == ENAvc_aspect_ratio_idc_Extended_SAR ) {
				NBAvcBitstream_readU16_(data, sar_width, bs, 16); // 0 u(16)
				NBAvcBitstream_readU16_(data, sar_height, bs, 16); // 0 u(16)
			}
		}
		NBAvcBitstream_readU8_(data, overscan_info_present_flag, bs, 1); //0 u(1)
		if( data->overscan_info_present_flag ){
			NBAvcBitstream_readU8_(data, overscan_appropriate_flag, bs, 1); //0 u(1)
		}
		NBAvcBitstream_readU8_(data, video_signal_type_present_flag, bs, 1); //0 u(1)
		if( data->video_signal_type_present_flag ) {
			NBAvcBitstream_readU8_(data, video_format, bs, 3); //0 u(3)
			NBAvcBitstream_readU8_(data, video_full_range_flag, bs, 1); //0 u(1)
			NBAvcBitstream_readU8_(data, colour_description_present_flag, bs, 1); //0 u(1)
			if( data->colour_description_present_flag ) {
				NBAvcBitstream_readU8_(data, colour_primaries, bs, 8); //0 u(8)
				NBAvcBitstream_readU8_(data, transfer_characteristics, bs, 8); //0 u(8)
				NBAvcBitstream_readU8_(data, matrix_coefficients, bs, 8); //0 u(8)
			}
		}
		NBAvcBitstream_readU8_(data, chroma_loc_info_present_flag, bs, 1); //0 u(1)
		if( data->chroma_loc_info_present_flag ) {
			NBAvcBitstream_readGolombUE64_(data, chroma_sample_loc_type_top_field, bs); //0 ue(v)
			NBAvcBitstream_readGolombUE64_(data, chroma_sample_loc_type_bottom_field, bs); //0 ue(v)
		} 
		NBAvcBitstream_readU8_(data, timing_info_present_flag, bs, 1); //0 u(1)
		if( data->timing_info_present_flag ) {
			NBAvcBitstream_readU32_(data, num_units_in_tick, bs, 32); //0 u(32)
			NBAvcBitstream_readU32_(data, time_scale, bs, 32); //0 u(32)
			NBAvcBitstream_readU8_(data, fixed_frame_rate_flag, bs, 1); //0 u(1)
		}
		NBAvcBitstream_readU8_(data, nal_hrd_parameters_present_flag, bs, 1); //0 u(1)
		if( data->nal_hrd_parameters_present_flag ){
			if(data->nal_hrd_parameters == NULL){
				data->nal_hrd_parameters = NBAvc_hrd_parameters_createSt();
			}
			if(data->nal_hrd_parameters == NULL){
				r = FALSE;
			} else if(!NBAvc_hrd_parameters_loadBits(bs, params, data->nal_hrd_parameters, picDescBaseOnly)){
				r = FALSE;
			}
		}
		NBAvcBitstream_readU8_(data, vcl_hrd_parameters_present_flag, bs, 1); //0 u(1)
		if(data->vcl_hrd_parameters_present_flag ){
			if(data->vcl_hrd_parameters == NULL){
				data->vcl_hrd_parameters = NBAvc_hrd_parameters_createSt();
			}
			if(data->vcl_hrd_parameters == NULL){
				r = FALSE;
			} else if(!NBAvc_hrd_parameters_loadBits(bs, params, data->vcl_hrd_parameters, picDescBaseOnly)){
				r = FALSE;
			}
		}
		if( data->nal_hrd_parameters_present_flag || data->vcl_hrd_parameters_present_flag ){
			NBAvcBitstream_readU8_(data, low_delay_hrd_flag, bs, 1); //0 u(1)
		}
		NBAvcBitstream_readU8_(data, pic_struct_present_flag, bs, 1); //0 u(1)
		NBAvcBitstream_readU8_(data, bitstream_restriction_flag, bs, 1); //0 u(1)
		if( data->bitstream_restriction_flag ) {
			NBAvcBitstream_readU8_(data, motion_vectors_over_pic_boundaries_flag, bs, 1); //0 u(1)
			NBAvcBitstream_readGolombUE64_(data, max_bytes_per_pic_denom, bs); //0 ue(v)
			NBAvcBitstream_readGolombUE64_(data, max_bits_per_mb_denom, bs); //0 ue(v)
			NBAvcBitstream_readGolombUE64_(data, log2_max_mv_length_horizontal, bs); //0 ue(v)
			NBAvcBitstream_readGolombUE64_(data, log2_max_mv_length_vertical, bs); //0 ue(v)
			NBAvcBitstream_readGolombUE64_(data, num_reorder_frames, bs); //0 ue(v)
			NBAvcBitstream_readGolombUE64_(data, max_dec_frame_buffering, bs); //0 ue(v)
		}
		//
		if(bs->stopFlags){
			r = FALSE;
		}
	}
	return r;
}

//seq_parameter_set_data

void* NBAvc_seq_parameter_set_data_createSt(void);
void NBAvc_seq_parameter_set_data_releaseSt(void* obj);
BOOL NBAvc_seq_parameter_set_data_loadBits(STNBAvcBitstream* bs, const STNBAvcStateParams* params, void* data, const BOOL picDescBaseOnly);

void* NBAvc_seq_parameter_set_data_createSt(void){
	STNBAvc_seq_parameter_set_data* r = NBMemory_allocType(STNBAvc_seq_parameter_set_data);
	NBMemory_setZeroSt(*r, STNBAvc_seq_parameter_set_data);
	return r;
}

void NBAvc_seq_parameter_set_data_releaseSt(void* pObj){
	STNBAvc_seq_parameter_set_data* obj = (STNBAvc_seq_parameter_set_data*)pObj;
	if(obj != NULL){
		if(obj->seq_scaling_list_present_flag != NULL){
			NBMemory_free(obj->seq_scaling_list_present_flag);
			obj->seq_scaling_list_present_flag		= NULL;
			obj->seq_scaling_list_present_flagUse	= 0;
			obj->seq_scaling_list_present_flagSz	= 0;
		}
		if(obj->ScalingList4x4 != NULL){
			NBMemory_free(obj->ScalingList4x4);
			obj->ScalingList4x4		= NULL;
			obj->ScalingList4x4Use	= 0;
			obj->ScalingList4x4Sz	= 0;
		}
		if(obj->ScalingList8x8 != NULL){
			NBMemory_free(obj->ScalingList8x8);
			obj->ScalingList8x8		= NULL;
			obj->ScalingList8x8Use	= 0;
			obj->ScalingList8x8Sz	= 0;
		}
		if(obj->offset_for_ref_frame != NULL){
			NBMemory_free(obj->offset_for_ref_frame);
			obj->offset_for_ref_frame		= NULL;
			obj->offset_for_ref_frameUse	= 0;
			obj->offset_for_ref_frameSz		= 0;
		}
		if(obj->vui_parameters != NULL){
			NBAvc_vui_parameters_releaseSt(obj->vui_parameters);
			obj->vui_parameters = NULL;
		}
		NBMemory_free(obj);
		obj = NULL;
	}
}

BOOL NBAvc_scaling_list_loadBits(STNBAvcBitstream* bs, UI8* elems, const UI8 elemsSz, BOOL* useDefaultFlag){
	BOOL r = FALSE;
	if(bs != NULL && !bs->stopFlags){
		r = TRUE;
		//
		{
			//Struct just to be able to use read MACROS
			typedef struct NBAvc_scaling_list_loadBits_st_ {
				SI64 delta_scale;
			} NBAvc_scaling_list_loadBits_st;
			//
			NBAvc_scaling_list_loadBits_st delta_scale;
			UI8 j, curScale = 8; SI64 lastScale = 8, nextScale = 8;
			for( j = 0; j < elemsSz; j++ ) {
				if( nextScale != 0 ) {
					NBAvcBitstream_readGolombSE64_(&delta_scale, delta_scale, bs); //se(v) 
					nextScale = ( lastScale + delta_scale.delta_scale + 256 ) % 256;
					if(useDefaultFlag != NULL){
						*useDefaultFlag = ( j == 0 && nextScale == 0 );
					}
				}
				curScale = (UI8)(( nextScale == 0 ) ? lastScale : nextScale);
				if(elems != NULL){
					elems[j] = curScale;
				} 
				lastScale = curScale;
			}
		}
		//
		if(bs->stopFlags){
			r = FALSE;
		}
	}
	return r;
}

BOOL NBAvc_seq_parameter_set_data_loadBits(STNBAvcBitstream* bs, const STNBAvcStateParams* params, void* pData, const BOOL picDescBaseOnly){
	BOOL r = FALSE;
	if(bs != NULL && !bs->stopFlags && pData != NULL){
		STNBAvc_seq_parameter_set_data* data = (STNBAvc_seq_parameter_set_data*)pData;
		r = TRUE;
		NBAvcBitstream_readU8_(data, profile_idc, bs, 8); //0 u(8)
		NBAvcBitstream_readU8_(data, constraint_set0_flag, bs, 1); //0 u(1)
		NBAvcBitstream_readU8_(data, constraint_set1_flag, bs, 1); //0 u(1)
		NBAvcBitstream_readU8_(data, constraint_set2_flag, bs, 1); //0 u(1)
		NBAvcBitstream_readU8_(data, constraint_set3_flag, bs, 1); //0 u(1)
		NBAvcBitstream_readU8_(data, constraint_set4_flag, bs, 1); //0 u(1)
		NBAvcBitstream_readU8_(data, constraint_set5_flag, bs, 1); //0 u(1)
		NBAvcBitstream_readU8_(data, reserved_zero_2bits, bs, 2); //0 u(2)
		NBAvcBitstream_readU8_(data, level_idc, bs, 8); //0 u(8)
		NBAvcBitstream_readGolombUE64_(data, seq_parameter_set_id, bs); //0 ue(v)
		if(data->profile_idc == 100 || data->profile_idc == 110 ||
		   data->profile_idc == 122 || data->profile_idc == 244 || data->profile_idc == 44 ||
		   data->profile_idc == 83 || data->profile_idc == 86 || data->profile_idc == 118 ||
		   data->profile_idc == 128 || data->profile_idc == 138 || data->profile_idc == 139 ||
		   data->profile_idc == 134 || data->profile_idc == 135)
		{
			NBAvcBitstream_readGolombUE64_(data, chroma_format_idc, bs); //0 ue(v) //0 to 3 inclusive
            if(data->chroma_format_idc < 0 || data->chroma_format_idc > 3){
                bs->stopFlags |= NB_AVC_BITSTREAM_STOP_VALUE_LOGIC_ERROR;
                //NBASSERT(data->chroma_format_idc >= 0 && data->chroma_format_idc <= 3)
                r = FALSE;
            }
			if(data->chroma_format_idc == 3 ){
				NBAvcBitstream_readU8_(data, separate_colour_plane_flag, bs, 1); //0 u(1)
			}
			NBAvcBitstream_readGolombUE64_(data, bit_depth_luma_minus8, bs); //ue(v)
			NBAvcBitstream_readGolombUE64_(data, bit_depth_chroma_minus8, bs); //ue(v)
			NBAvcBitstream_readU8_(data, qpprime_y_zero_transform_bypass_flag, bs, 1); //u(1)
			NBAvcBitstream_readU8_(data, seq_scaling_matrix_present_flag, bs, 1); //u(1)
			if(r && data->seq_scaling_matrix_present_flag ){
				UI8 i; const UI8 size = ((data->chroma_format_idc != 3) ? 8 : 12 ); NBASSERT(size > 0)
				//allocate or reuse
				{
					//flag
					data->seq_scaling_list_present_flagUse = size;
					if(data->seq_scaling_list_present_flagUse > data->seq_scaling_list_present_flagSz){
						if(data->seq_scaling_list_present_flag != NULL){
							NBMemory_free(data->seq_scaling_list_present_flag);
							data->seq_scaling_list_present_flag = NULL;
						}
						data->seq_scaling_list_present_flag = NBMemory_allocTypes(BOOL, data->seq_scaling_list_present_flagUse);
						data->seq_scaling_list_present_flagSz = data->seq_scaling_list_present_flagUse;
					}
					if(data->seq_scaling_list_present_flagSz > 0){
						NBMemory_set(data->seq_scaling_list_present_flag, 0, sizeof(data->seq_scaling_list_present_flag[0]) * data->seq_scaling_list_present_flagSz);
					}
					//4x4
					data->ScalingList4x4Use	= (size < 6 ? size : 6);
					if(data->ScalingList4x4Use > data->ScalingList4x4Sz){
						if(data->ScalingList4x4 != NULL){
							NBMemory_free(data->ScalingList4x4);
							data->ScalingList4x4 = NULL;
						}
						data->ScalingList4x4	= NBMemory_allocTypes(STNBAvc_ScalingList4x4, data->ScalingList4x4Use);
						data->ScalingList4x4Sz	= data->ScalingList4x4Use;
					}
					if(data->ScalingList4x4Sz > 0){
						NBMemory_set(data->ScalingList4x4, 0, sizeof(data->ScalingList4x4[0]) * data->ScalingList4x4Sz);
					}
					//8x8
					data->ScalingList8x8Use	= (size > 6 ? size - 6: 0);
					if(data->ScalingList8x8Use > data->ScalingList8x8Sz){
						if(data->ScalingList8x8 != NULL){
							NBMemory_free(data->ScalingList8x8);
							data->ScalingList8x8 = NULL;
						}
						data->ScalingList8x8	= NBMemory_allocTypes(STNBAvc_ScalingList8x8, data->ScalingList8x8Use);
						data->ScalingList8x8Sz	= data->ScalingList8x8Use;
					}
					if(data->ScalingList8x8Sz > 0){
						NBMemory_set(data->ScalingList8x8, 0, sizeof(data->ScalingList8x8[0]) * data->ScalingList8x8Sz);
					}
				}
				//load
				for(i = 0; r && i < size; i++){
					NBAvcBitstream_readU8_(data, seq_scaling_list_present_flag[i], bs, 1); //[ i ] u(1)
					if( data->seq_scaling_list_present_flag[ i ] ){
						if( i < 6 ){
							STNBAvc_ScalingList4x4* lst = &data->ScalingList4x4[i];
							if(!NBAvc_scaling_list_loadBits(bs, lst->elems, sizeof(lst->elems), &lst->UseDefaultScalingMatrix4x4Flag)){
								PRINTF_ERROR("NBAvc_seq_parameter_set_data_loadBits, NBAvc_scaling_list_loadBits failed.\n");
								r = FALSE;
								break;
							}
							//scaling_list( ScalingList4x4[ i ], 16, UseDefaultScalingMatrix4x4Flag[ i ])
						} else {
							STNBAvc_ScalingList8x8* lst = &data->ScalingList8x8[i - 6];
							if(!NBAvc_scaling_list_loadBits(bs, lst->elems, sizeof(lst->elems), &lst->UseDefaultScalingMatrix8x8Flag)){
								PRINTF_ERROR("NBAvc_seq_parameter_set_data_loadBits, NBAvc_scaling_list_loadBits failed.\n");
								r = FALSE;
								break;
							}
							//scaling_list( ScalingList8x8[ i - 6 ], 64, UseDefaultScalingMatrix8x8Flag[ i - 6 ] )
						}
					}
				}
				//Print scaling lists
#				ifdef NB_CONFIG_INCLUDE_ASSERTS
				/*if(data->ScalingList4x4Sz > 0){
					UI8 i;
					STNBString str;
					NBString_init(&str);
					for(i = 0; i < data->ScalingList4x4Sz; i++){
						const STNBAvc_ScalingList4x4* lst = &data->ScalingList4x4[i];
						NBString_empty(&str);
						NBAvc_ScalingList4x4_concat(lst, &str);
						PRINTF_INFO("NBAvc_seq_parameter_set_data_loadBits, ScalingList4x4[%d] = \n%s\n", i, str.str);
					}
					NBString_release(&str);
				}
				if(data->ScalingList8x8Sz > 0){
					UI8 i;
					STNBString str;
					NBString_init(&str);
					for(i = 0; i < data->ScalingList8x8Sz; i++){
						const STNBAvc_ScalingList8x8* lst = &data->ScalingList8x8[i];
						NBString_empty(&str);
						NBAvc_ScalingList8x8_concat(lst, &str);
						PRINTF_INFO("NBAvc_seq_parameter_set_data_loadBits, ScalingList8x8[%d] = \n%s\n", i, str.str);
					}
					NBString_release(&str);
				}*/
#				endif
			}
		}
		NBAvcBitstream_readGolombUE64_(data, log2_max_frame_num_minus4, bs); //0 ue(v)
		//shall be in the range of 0 to 12, inclusive
		if(data->log2_max_frame_num_minus4 > 12){
			PRINTF_ERROR("NBAvcBitstream, log2_max_frame_num_minus4(%llu) shall be in the range of 0 to 12, inclusive.\n", data->log2_max_frame_num_minus4);
			r = FALSE;
		}
		NBAvcBitstream_readGolombUE64_(data, pic_order_cnt_type, bs); //0 ue(v)
		//shall be in the range of 0 to 2, inclusive
		if(data->pic_order_cnt_type > 2){
			PRINTF_ERROR("NBAvcBitstream, pic_order_cnt_type(%llu) shall be in the range of 0 to 2, inclusive.\n", data->pic_order_cnt_type);
			r = FALSE;
		}
		if( data->pic_order_cnt_type == 0 ){
			NBAvcBitstream_readGolombUE64_(data, log2_max_pic_order_cnt_lsb_minus4, bs); //0 ue(v)
			//shall be in the range of 0 to 12, inclusive.
			if(data->log2_max_pic_order_cnt_lsb_minus4 > 12){
				PRINTF_ERROR("NBAvcBitstream, log2_max_pic_order_cnt_lsb_minus4(%llu) shall be in the range of 0 to 12, inclusive.\n", data->log2_max_pic_order_cnt_lsb_minus4);
				r = FALSE;
			}
		} else if( data->pic_order_cnt_type == 1 ) {
			NBAvcBitstream_readU8_(data, delta_pic_order_always_zero_flag, bs, 1); //0 u(1)
			NBAvcBitstream_readGolombSE64_(data, offset_for_non_ref_pic, bs); //0 se(v)
			NBAvcBitstream_readGolombSE64_(data, offset_for_top_to_bottom_field, bs); //0 se(v)
			NBAvcBitstream_readGolombUE64_(data, num_ref_frames_in_pic_order_cnt_cycle, bs); //0 ue(v)
			data->offset_for_ref_frameUse = (UI16)data->num_ref_frames_in_pic_order_cnt_cycle;
			if(data->offset_for_ref_frameUse > data->offset_for_ref_frameSz){
				if(data->offset_for_ref_frame != NULL){
					NBMemory_free(data->offset_for_ref_frame);
					data->offset_for_ref_frame = NULL;
				}
				data->offset_for_ref_frame		= NBMemory_allocTypes(SI64, data->offset_for_ref_frameUse);
				data->offset_for_ref_frameSz	= data->offset_for_ref_frameUse;
			}
			if(r && data->num_ref_frames_in_pic_order_cnt_cycle > 0){
				UI64 i; const UI64 size = data->num_ref_frames_in_pic_order_cnt_cycle;
				for( i = 0; i < size; i++ ){
					NBAvcBitstream_readGolombSE64_(data, offset_for_ref_frame[i], bs);//[ i ] 0 se(v)
				}
			}
		}
		NBAvcBitstream_readGolombUE64_(data, max_num_ref_frames, bs); //0 ue(v)
		NBAvcBitstream_readU8_(data, gaps_in_frame_num_value_allowed_flag, bs, 1); //0 u(1)
		NBAvcBitstream_readGolombUE64_(data, pic_width_in_mbs_minus1, bs); //0 ue(v)
		NBAvcBitstream_readGolombUE64_(data, pic_height_in_map_units_minus1, bs); //0 ue(v)
		NBAvcBitstream_readU8_(data, frame_mbs_only_flag, bs, 1); //0 u(1)
		if( !data->frame_mbs_only_flag ){
			NBAvcBitstream_readU8_(data, mb_adaptive_frame_field_flag, bs, 1); //0 u(1)
		}
		NBAvcBitstream_readU8_(data, direct_8x8_inference_flag, bs, 1); //0 u(1)
		NBAvcBitstream_readU8_(data, frame_cropping_flag, bs, 1); //0 u(1)
		if( data->frame_cropping_flag ) {
			NBAvcBitstream_readGolombUE64_(data, frame_crop_left_offset, bs); //0 ue(v)
			NBAvcBitstream_readGolombUE64_(data, frame_crop_right_offset, bs); //0 ue(v)
			NBAvcBitstream_readGolombUE64_(data, frame_crop_top_offset, bs); //0 ue(v)
			NBAvcBitstream_readGolombUE64_(data, frame_crop_bottom_offset, bs); //0 ue(v)
		}
		NBAvcBitstream_readU8_(data, vui_parameters_present_flag, bs, 1); //0 u(1)
		if( r && data->vui_parameters_present_flag ){
			//	vui_parameters( ) 0
			if(data->vui_parameters == NULL){
				data->vui_parameters = NBAvc_vui_parameters_createSt();
			}
			if(data->vui_parameters == NULL){
				PRINTF_ERROR("NBAvc_seq_parameter_set_data_loadBits, NBAvc_vui_parameters_createSt failed.\n");
				r = FALSE;
			} else if(!NBAvc_vui_parameters_loadBits(bs, params, data->vui_parameters, picDescBaseOnly)){
				PRINTF_ERROR("NBAvc_seq_parameter_set_data_loadBits, NBAvc_vui_parameters_loadBits failed.\n");
				r = FALSE;
			}
		}
		//
		if(bs->stopFlags){
			r = FALSE;
		}
	}
	return r;
}

//seq_parameter_set_rbsp (nal_unit_type = 7)

void* NBAvc_seq_parameter_set_rbsp_createSt(void);
void NBAvc_seq_parameter_set_rbsp_releaseSt(void* obj);
BOOL NBAvc_seq_parameter_set_rbsp_loadBits(STNBAvcBitstream* bs, const STNBAvcStateParams* params, void* data, const BOOL picDescBaseOnly);

static const STNBAvcNaluMap NBAvcMap_seq_parameter_set_rbsp = {
	"seq_parameter_set_rbsp"
	//Methods
	, NBAvc_seq_parameter_set_rbsp_createSt
	, NBAvc_seq_parameter_set_rbsp_releaseSt
	, NBAvc_seq_parameter_set_rbsp_loadBits
};

void* NBAvc_seq_parameter_set_rbsp_createSt(void){
	STNBAvc_seq_parameter_set_rbsp* r = NBMemory_allocType(STNBAvc_seq_parameter_set_rbsp);
	NBMemory_setZeroSt(*r, STNBAvc_seq_parameter_set_rbsp);
	return r;
}

void NBAvc_seq_parameter_set_rbsp_releaseSt(void* pObj){
	STNBAvc_seq_parameter_set_rbsp* obj = (STNBAvc_seq_parameter_set_rbsp*)pObj;
	if(obj != NULL){
		if(obj->data != NULL){
			NBAvc_seq_parameter_set_data_releaseSt(obj->data);
			obj->data = NULL;
		}
		if(obj->rbsp_trailing_bits != NULL){
			NBAvc_rbsp_trailing_bits_releaseSt(obj->rbsp_trailing_bits);
			obj->rbsp_trailing_bits = NULL;
		}
		NBMemory_free(obj);
		obj = NULL;
	}
}

BOOL NBAvc_seq_parameter_set_rbsp_loadBits(STNBAvcBitstream* bs, const STNBAvcStateParams* params, void* pData, const BOOL picDescBaseOnly){
	BOOL r = FALSE;
	if(bs != NULL && !bs->stopFlags && pData != NULL){
		STNBAvc_seq_parameter_set_rbsp* data = (STNBAvc_seq_parameter_set_rbsp*)pData;
		r = TRUE;
		//data
		{
			if(data->data == NULL){
				data->data = NBAvc_seq_parameter_set_data_createSt();
			}
			if(data->data == NULL){
				PRINTF_ERROR("NBAvc_seq_parameter_set_rbsp_loadBits, NBAvc_seq_parameter_set_data_createSt failed.\n");
				r = FALSE;
			} else if(!NBAvc_seq_parameter_set_data_loadBits(bs, params, data->data, picDescBaseOnly)){
				PRINTF_ERROR("NBAvc_seq_parameter_set_rbsp_loadBits, NBAvc_seq_parameter_set_data_loadBits failed.\n");
				r = FALSE;
			}
		}
		//trailing_bits
		if(r){
			if(!NBAvc_rbsp_trailing_bits_loadBits(bs, params, NULL, picDescBaseOnly)){ //rbsp_trailing_bits( )
				PRINTF_ERROR("NBAvc_seq_parameter_set_rbsp_loadBits, NBAvc_rbsp_trailing_bits_loadBits failed.\n");
				r = FALSE;
			}
		}
		//
		if(bs->stopFlags){
			r = FALSE;
		}
	}
	return r;
}

//pic_parameter_set_rbsp (nal_unit_type = 8)

void* NBAvc_pic_parameter_set_rbsp_createSt(void);
void NBAvc_pic_parameter_set_rbsp_releaseSt(void* obj);
BOOL NBAvc_pic_parameter_set_rbsp_loadBits(STNBAvcBitstream* bs, const STNBAvcStateParams* params, void* data, const BOOL picDescBaseOnly);

static const STNBAvcNaluMap NBAvcMap_pic_parameter_set_rbsp = {
	"pic_parameter_set_rbsp"
	//Methods
	, NBAvc_pic_parameter_set_rbsp_createSt
	, NBAvc_pic_parameter_set_rbsp_releaseSt
	, NBAvc_pic_parameter_set_rbsp_loadBits
};

void* NBAvc_pic_parameter_set_rbsp_createSt(void){
	STNBAvc_pic_parameter_set_rbsp* r = NBMemory_allocType(STNBAvc_pic_parameter_set_rbsp);
	NBMemory_setZeroSt(*r, STNBAvc_pic_parameter_set_rbsp);
	return r;
}

void NBAvc_pic_parameter_set_rbsp_releaseSt(void* pObj){
	STNBAvc_pic_parameter_set_rbsp* obj = (STNBAvc_pic_parameter_set_rbsp*)pObj;
	if(obj != NULL){
		if(obj->run_length_minus1 != NULL){
			NBMemory_free(obj->run_length_minus1);
			obj->run_length_minus1 = NULL;
		}
		if(obj->top_left != NULL){
            NBMemory_free(obj->top_left);
			obj->top_left = NULL;
		}
		if(obj->bottom_right != NULL){
            NBMemory_free(obj->bottom_right);
			obj->bottom_right = NULL;
		}
		if(obj->slice_group_id != NULL){
            NBMemory_free(obj->slice_group_id);
			obj->slice_group_id = NULL;
		}
		if(obj->rbsp_trailing_bits != NULL){
			NBAvc_rbsp_trailing_bits_releaseSt(obj->rbsp_trailing_bits);
			obj->rbsp_trailing_bits = NULL;
		}
		NBMemory_free(obj);
		obj = NULL;
	}
}

BOOL NBAvc_pic_parameter_set_rbsp_loadBits(STNBAvcBitstream* bs, const STNBAvcStateParams* params, void* pData, const BOOL picDescBaseOnly){
	BOOL r = FALSE;
	if(bs != NULL && !bs->stopFlags && pData != NULL){
		STNBAvc_pic_parameter_set_rbsp* data = (STNBAvc_pic_parameter_set_rbsp*)pData;
		r = TRUE;
		NBAvcBitstream_readGolombUE64_(data, pic_parameter_set_id, bs); //1 ue(v)
		NBAvcBitstream_readGolombUE64_(data, seq_parameter_set_id, bs); // 1 ue(v)
		NBAvcBitstream_readU8_(data, entropy_coding_mode_flag, bs, 1); // 1 u(1)
		NBAvcBitstream_readU8_(data, bottom_field_pic_order_in_frame_present_flag, bs, 1); // 1 u(1)
		NBAvcBitstream_readGolombUE64_(data, num_slice_groups_minus1, bs); // 1 ue(v)
		if( data->num_slice_groups_minus1 > 0 ) {
			NBAvcBitstream_readGolombUE64_(data, slice_group_map_type, bs); // 1 ue(v)
			if( data->slice_group_map_type == 0 ){
				UI64 i;
				//run_length
				data->run_length_minus1Use = (UI16)data->num_slice_groups_minus1 + 1;
				if(data->run_length_minus1Use > data->run_length_minus1Sz){
					if(data->run_length_minus1 != NULL){
						NBMemory_free(data->run_length_minus1);
						data->run_length_minus1 = NULL;
					}
					data->run_length_minus1		= NBMemory_allocTypes(UI64, data->run_length_minus1Use);
					data->run_length_minus1Sz	= data->run_length_minus1Use;
				}
				//load
				for( i = 0; i <= data->num_slice_groups_minus1; i++ ){
					NBAvcBitstream_readGolombUE64_(data, run_length_minus1[i], bs); //[ iGroup ] 1 ue(v)
				}
			} else if( data->slice_group_map_type == 2 ){
				//top_left
				data->top_leftUse		= (UI16)data->num_slice_groups_minus1;
				if(data->top_leftUse > data->top_leftSz){
					if(data->top_left != NULL){
                        NBMemory_free(data->top_left);
						data->top_left = NULL;
					}
					data->top_left		= NBMemory_allocTypes(UI64, data->top_leftUse);
					data->top_leftSz	= data->top_leftUse;
				}
				//bottom_right
				data->bottom_rightUse	= (UI16)data->num_slice_groups_minus1;
				if(data->bottom_rightUse > data->bottom_rightSz){
					if(data->bottom_right != NULL){
                        NBMemory_free(data->bottom_right);
						data->bottom_right = NULL;
					}
					data->bottom_right		= NBMemory_allocTypes(UI64, data->bottom_rightUse);
					data->bottom_rightSz	= data->bottom_rightUse;
				}
				//load
				if(data->num_slice_groups_minus1 > 0){
					UI64 i; for( i = 0; i < data->num_slice_groups_minus1; i++ ) {
						NBAvcBitstream_readGolombUE64_(data, top_left[i], bs); //[ iGroup ] 1 ue(v)
						NBAvcBitstream_readGolombUE64_(data, bottom_right[i], bs); //[ iGroup ] 1 ue(v)
					}
				}
			} else if( data->slice_group_map_type == 3 || data->slice_group_map_type == 4 || data->slice_group_map_type == 5 ) {
				NBAvcBitstream_readU8_(data, slice_group_change_direction_flag, bs, 1); // 1 u(1)
				NBAvcBitstream_readGolombUE64_(data, slice_group_change_rate_minus1, bs); // 1 ue(v)
			} else if( data->slice_group_map_type == 6 ) {
				UI64 i; const UI8 ammBits = (UI8)ceil( log2( (double)data->num_slice_groups_minus1 + 1.0 ) );
				NBAvcBitstream_readGolombUE64_(data, pic_size_in_map_units_minus1, bs); // 1 ue(v)
				//slice_group_id
				data->slice_group_idUse = (UI16)data->pic_size_in_map_units_minus1 + 1;
				if(data->slice_group_idUse > data->slice_group_idSz){
					if(data->slice_group_id != NULL){
						NBMemory_free(data->slice_group_id);
						data->slice_group_id = NULL;
					}
					data->slice_group_id	= NBMemory_allocTypes(UI32, data->slice_group_idUse);
					data->slice_group_idSz	= data->slice_group_idUse;
				}
				//load
				for( i = 0; i <= data->pic_size_in_map_units_minus1; i++ ){
					NBAvcBitstream_readU32v_(data, slice_group_id[i], bs, ammBits); //[ i ] 1 u(v)
				}
			}
		}
		NBAvcBitstream_readGolombUE64_(data, num_ref_idx_l0_active_minus1, bs); // 1 ue(v)
		NBAvcBitstream_readGolombUE64_(data, num_ref_idx_l1_active_minus1, bs); // 1 ue(v)
		NBAvcBitstream_readU8_(data, weighted_pred_flag, bs, 1); // 1 u(1)
		NBAvcBitstream_readU8_(data, weighted_bipred_idc, bs, 2); // 1 u(2)
		NBAvcBitstream_readGolombSE64_(data, pic_init_qp_minus26, bs); // relative to 26 // 1 se(v)
		NBAvcBitstream_readGolombSE64_(data, pic_init_qs_minus26, bs); // relative to 26 // 1 se(v)
		NBAvcBitstream_readGolombSE64_(data, chroma_qp_index_offset, bs); // 1 se(v)
		NBAvcBitstream_readU8_(data, deblocking_filter_control_present_flag, bs, 1); // 1 u(1)
		NBAvcBitstream_readU8_(data, constrained_intra_pred_flag, bs, 1); // 1 u(1)
		NBAvcBitstream_readU8_(data, redundant_pic_cnt_present_flag, bs, 1); // 1 u(1)
		if( r && NBAvc_more_rbsp_data(bs) ) {
			NBAvcBitstream_readU8_(data, transform_8x8_mode_flag, bs, 1); //1 u(1)
			NBAvcBitstream_readU8_(data, pic_scaling_matrix_present_flag, bs, 1); //1 u(1)
			if( data->pic_scaling_matrix_present_flag ){
				if(params == NULL){
					r = FALSE;
				} else if(params->seq_parameter_set_rbsp == NULL){
					r = FALSE;
				} else if(params->seq_parameter_set_rbsp->data == NULL){
					r = FALSE;
				} else {
					UI32 i; const UI32 size = 6 + ( ( params->seq_parameter_set_rbsp->data->chroma_format_idc != 3 ) ? 2 : 6 ) * data->transform_8x8_mode_flag;
					//pic_scaling_list_present_flag
					data->pic_scaling_list_present_flagUse = (UI16)size;
					if(data->pic_scaling_list_present_flagUse > data->pic_scaling_list_present_flagSz){
						if(data->pic_scaling_list_present_flag != NULL){
							NBMemory_free(data->pic_scaling_list_present_flag);
							data->pic_scaling_list_present_flag = NULL;
						}
						data->pic_scaling_list_present_flag		= NBMemory_allocTypes(BOOL, data->pic_scaling_list_present_flagUse);
						data->pic_scaling_list_present_flagSz	= data->pic_scaling_list_present_flagUse;
					}
					//4x4
					data->ScalingList4x4Use		= (size < 6 ? size : 6);
					if(data->ScalingList4x4Use > data->ScalingList4x4Sz){
						if(data->ScalingList4x4 != NULL){
							NBMemory_free(data->ScalingList4x4);
							data->ScalingList4x4 = NULL;
						}
						data->ScalingList4x4		= NBMemory_allocTypes(STNBAvc_ScalingList4x4, data->ScalingList4x4Use);
						data->ScalingList4x4Sz		= data->ScalingList4x4Sz;
					}
					if(data->ScalingList4x4Sz > 0){
						NBMemory_set(data->ScalingList4x4, 0, sizeof(data->ScalingList4x4[0]) * data->ScalingList4x4Sz);
					}
					//8x8
					data->ScalingList8x8Use		= (size > 6 ? size - 6 : 0); 
					if(data->ScalingList8x8Use > data->ScalingList8x8Sz){
						if(data->ScalingList8x8 != NULL){
							NBMemory_free(data->ScalingList8x8);
							data->ScalingList8x8 = NULL;
						}
						data->ScalingList8x8	= NBMemory_allocTypes(STNBAvc_ScalingList8x8, data->ScalingList8x8Use);
						data->ScalingList8x8Sz	= data->ScalingList8x8Use;
					}
					if(data->ScalingList8x8Sz > 0){
						NBMemory_set(data->ScalingList8x8, 0, sizeof(data->ScalingList8x8[0]) * data->ScalingList8x8Sz);
					}
					//load
					for( i = 0; i < size; i++ ) {
						NBAvcBitstream_readU8_(data, pic_scaling_list_present_flag[i], bs, 1); //[ i ]; //1 u(1)
						if(data->pic_scaling_list_present_flag[i]){
							if(i < 6){
								STNBAvc_ScalingList4x4* lst = &data->ScalingList4x4[i];
								if(!NBAvc_scaling_list_loadBits(bs, lst->elems, sizeof(lst->elems), &lst->UseDefaultScalingMatrix4x4Flag)){
									PRINTF_ERROR("NBAvc_pic_parameter_set_rbsp_loadBits, NBAvc_scaling_list_loadBits failed.\n");
									r = FALSE;
									break;
								}
							} else {
								STNBAvc_ScalingList8x8* lst = &data->ScalingList8x8[i - 6];
								if(!NBAvc_scaling_list_loadBits(bs, lst->elems, sizeof(lst->elems), &lst->UseDefaultScalingMatrix8x8Flag)){
									PRINTF_ERROR("NBAvc_pic_parameter_set_rbsp_loadBits, NBAvc_scaling_list_loadBits failed.\n");
									r = FALSE;
									break;
								}
							}
						}
					}
					//Print scaling lists
#					ifdef NB_CONFIG_INCLUDE_ASSERTS
					if(data->ScalingList4x4Sz > 0){
						UI8 i;
						STNBString str;
						NBString_init(&str);
						for(i = 0; i < data->ScalingList4x4Sz; i++){
							const STNBAvc_ScalingList4x4* lst = &data->ScalingList4x4[i];
							NBString_empty(&str);
							NBAvc_ScalingList4x4_concat(lst, &str);
							PRINTF_INFO("NBAvc_pic_parameter_set_rbsp_loadBits, ScalingList4x4[%d] = \n%s\n", i, str.str);
						}
						NBString_release(&str);
					}
					if(data->ScalingList8x8Sz > 0){
						UI8 i;
						STNBString str;
						NBString_init(&str);
						for(i = 0; i < data->ScalingList8x8Sz; i++){
							const STNBAvc_ScalingList8x8* lst = &data->ScalingList8x8[i];
							NBString_empty(&str);
							NBAvc_ScalingList8x8_concat(lst, &str);
							PRINTF_INFO("NBAvc_pic_parameter_set_rbsp_loadBits, ScalingList8x8[%d] = \n%s\n", i, str.str);
						}
						NBString_release(&str);
					}
#					endif
				}
			}
			NBAvcBitstream_readGolombSE64_(data, second_chroma_qp_index_offset, bs); //1 se(v)
		}
		//
		if(r){
			if(!NBAvc_rbsp_trailing_bits_loadBits(bs, params, NULL, picDescBaseOnly)){ //rbsp_trailing_bits( )
				PRINTF_ERROR("NBAvc_pic_parameter_set_rbsp_loadBits, NBAvc_rbsp_trailing_bits_loadBits failed.\n");
				r = FALSE;
			}
		}
		//
		if(bs->stopFlags){
			r = FALSE;
		}
	}
	return r;
}

//Slice

//ref_pic_list_mvc_modification

void* NBAvc_ref_pic_list_mvc_modification_createSt(void){
	STNBAvc_ref_pic_list_mvc_modification* r = NBMemory_allocType(STNBAvc_ref_pic_list_mvc_modification);
	NBMemory_setZeroSt(*r, STNBAvc_ref_pic_list_mvc_modification);
	return r;
}

void NBAvc_ref_pic_list_mvc_modification_releaseSt(void* pData){
	STNBAvc_ref_pic_list_mvc_modification* data = (STNBAvc_ref_pic_list_mvc_modification*)pData;
	NBMemory_free(data);
	data = NULL;
}
	
BOOL NBAvc_ref_pic_list_mvc_modification_loadBits(STNBAvcBitstream* bs, const STNBAvcStateParams* params, void* pData, const UI64 slice_type){
	BOOL r = FALSE;
	if(bs != NULL && !bs->stopFlags && pData != NULL){
		STNBAvc_ref_pic_list_mvc_modification* data = (STNBAvc_ref_pic_list_mvc_modification*)pData;
		r = TRUE;
		if((slice_type % 5) != 2 && (slice_type % 5) != 4){
			NBAvcBitstream_readU8_(data, ref_pic_list_modification_flag_l0, bs, 1); //2 u(1)
			if( data->ref_pic_list_modification_flag_l0 ){
				do {
					NBAvcBitstream_readGolombUE64_(data, modification_of_pic_nums_idc, bs); //2 ue(v)
					if( data->modification_of_pic_nums_idc == 0 || data->modification_of_pic_nums_idc == 1 ){
						NBAvcBitstream_readGolombUE64_(data, abs_diff_pic_num_minus1, bs); //2 ue(v)
					} else if( data->modification_of_pic_nums_idc == 2 ){
						NBAvcBitstream_readGolombUE64_(data, long_term_pic_num, bs); //2 ue(v)
					} else if ( data->modification_of_pic_nums_idc == 4 || data->modification_of_pic_nums_idc == 5 ){
						NBAvcBitstream_readGolombUE64_(data, abs_diff_view_idx_minus1, bs); //2 ue(v)
					}
				} while( data->modification_of_pic_nums_idc != 3 && !bs->stopFlags);
			}
		}
		if((slice_type % 5) == 1){
			NBAvcBitstream_readU8_(data, ref_pic_list_modification_flag_l1, bs, 1); //2 u(1)
			if( data->ref_pic_list_modification_flag_l1 ){
				do {
					NBAvcBitstream_readGolombUE64_(data, modification_of_pic_nums_idc, bs); //2 ue(v)
					if( data->modification_of_pic_nums_idc == 0 || data->modification_of_pic_nums_idc == 1 ){
						NBAvcBitstream_readGolombUE64_(data, abs_diff_pic_num_minus1, bs); //2 ue(v)
					} else if( data->modification_of_pic_nums_idc == 2 ){
						NBAvcBitstream_readGolombUE64_(data, long_term_pic_num, bs); //2 ue(v)
					} else if ( data->modification_of_pic_nums_idc == 4 || data->modification_of_pic_nums_idc == 5 ){
						NBAvcBitstream_readGolombUE64_(data, abs_diff_view_idx_minus1, bs); //2 ue(v)
					}
				} while( data->modification_of_pic_nums_idc != 3 && !bs->stopFlags);
			}
		}
		//
		if(bs->stopFlags){
			r = FALSE;
		}
	}
	return r;
}

//ref_pic_list_modification

void* NBAvc_ref_pic_list_modification_createSt(void){
	STNBAvc_ref_pic_list_modification* r = NBMemory_allocType(STNBAvc_ref_pic_list_modification);
	NBMemory_setZeroSt(*r, STNBAvc_ref_pic_list_modification);
	return r;
}

void NBAvc_ref_pic_list_modification_releaseSt(void* pData){
	STNBAvc_ref_pic_list_modification* data = (STNBAvc_ref_pic_list_modification*)pData;
	NBMemory_free(data);
	data = NULL;
}

BOOL NBAvc_ref_pic_list_modification_loadBits(STNBAvcBitstream* bs, const STNBAvcStateParams* params, void* pData, const UI64 slice_type){
	BOOL r = FALSE;
	if(bs != NULL && !bs->stopFlags && pData != NULL){
		STNBAvc_ref_pic_list_modification* data = (STNBAvc_ref_pic_list_modification*)pData;
		r = TRUE;
		if((slice_type % 5) != 2 && (slice_type % 5) != 4){
			NBAvcBitstream_readU8_(data, ref_pic_list_modification_flag_l0, bs, 1); //2 u(1)
			if( data->ref_pic_list_modification_flag_l0 ){
				do {
					NBAvcBitstream_readGolombUE64_(data, modification_of_pic_nums_idc, bs); //2 ue(v)
					if( data->modification_of_pic_nums_idc == 0 || data->modification_of_pic_nums_idc == 1 ){
						NBAvcBitstream_readGolombUE64_(data, abs_diff_pic_num_minus1, bs); //2 ue(v)
					} else if( data->modification_of_pic_nums_idc == 2 ){
						NBAvcBitstream_readGolombUE64_(data, long_term_pic_num, bs); //2 ue(v)
					}
				} while( data->modification_of_pic_nums_idc != 3 && !bs->stopFlags);
			}
		}
		if((slice_type % 5) == 1){
			NBAvcBitstream_readU8_(data, ref_pic_list_modification_flag_l1, bs, 1); //2 u(1)
			if( data->ref_pic_list_modification_flag_l1 ){
				do {
					NBAvcBitstream_readGolombUE64_(data, modification_of_pic_nums_idc, bs); //2 ue(v)
					if( data->modification_of_pic_nums_idc == 0 || data->modification_of_pic_nums_idc == 1 ){
						NBAvcBitstream_readGolombUE64_(data, abs_diff_pic_num_minus1, bs); //2 ue(v)
					} else if( data->modification_of_pic_nums_idc == 2 ){
						NBAvcBitstream_readGolombUE64_(data, long_term_pic_num, bs); //2 ue(v)
					}
				} while( data->modification_of_pic_nums_idc != 3 && !bs->stopFlags);
			}
		}
		if(bs->stopFlags){
			r = FALSE;
		}
	}
	return r;
}

//pred_weight_table

void* NBAvc_pred_weight_table_createSt(void){
	STNBAvc_pred_weight_table* r = NBMemory_allocType(STNBAvc_pred_weight_table);
	NBMemory_setZeroSt(*r, STNBAvc_pred_weight_table);
	return r;
}

void NBAvc_pred_weight_table_releaseSt(void* pData){
	STNBAvc_pred_weight_table* data = (STNBAvc_pred_weight_table*)pData;
	//
	if(data->luma_weight_l0 != NULL){
		NBMemory_free(data->luma_weight_l0);
		data->luma_weight_l0 = NULL;
	}
	if(data->luma_offset_l0 != NULL){
		NBMemory_free(data->luma_offset_l0);
		data->luma_offset_l0 = NULL;
	}
	if(data->chroma_weight_l0 != NULL){
		NBMemory_free(data->chroma_weight_l0);
		data->chroma_weight_l0 = NULL;
	}
	if(data->chroma_offset_l0 != NULL){
		NBMemory_free(data->chroma_offset_l0);
		data->chroma_offset_l0 = NULL;
	}
	//
	if(data->luma_weight_l1 != NULL){
		NBMemory_free(data->luma_weight_l1);
		data->luma_weight_l1 = NULL;
	}
	if(data->luma_offset_l1 != NULL){
		NBMemory_free(data->luma_offset_l1);
		data->luma_offset_l1 = NULL;
	}
	if(data->chroma_weight_l1 != NULL){
		NBMemory_free(data->chroma_weight_l1);
		data->chroma_weight_l1 = NULL;
	}
	if(data->chroma_offset_l1 != NULL){
		NBMemory_free(data->chroma_offset_l1);
		data->chroma_offset_l1 = NULL;
	}
	//
	NBMemory_free(data);
	data = NULL;
}

BOOL NBAvc_pred_weight_table_loadBits(STNBAvcBitstream* bs, const STNBAvcStateParams* params, void* pData, const STNBAvc_slice_header* slice_header){
	BOOL r = FALSE;
	if(bs != NULL && !bs->stopFlags && pData != NULL){
		STNBAvc_pred_weight_table* data = (STNBAvc_pred_weight_table*)pData;
		if(params != NULL){
			if(params->seq_parameter_set_rbsp != NULL){
				const UI8 ChromaArrayType = (params->seq_parameter_set_rbsp->data->separate_colour_plane_flag ? 0 : (UI8)params->seq_parameter_set_rbsp->data->chroma_format_idc);
				r = TRUE;
				NBAvcBitstream_readGolombUE64_(data, luma_log2_weight_denom, bs); //2 ue(v)
				if( ChromaArrayType != 0 ){
					NBAvcBitstream_readGolombUE64_(data, chroma_log2_weight_denom, bs); //2 ue(v)
				}
				{
					UI32 i;
					const UI32 luma_l0Sz	= (UI32)slice_header->num_ref_idx_l0_active_minus1 + 1;
					const UI32 chroma_l0Sz	= (ChromaArrayType != 0 ? (UI32)(slice_header->num_ref_idx_l0_active_minus1 + 1) * 2 : 0);
					//luma
					data->lumal0Use = (UI16)luma_l0Sz;
					if(data->lumal0Use > data->lumal0Sz){
						if(data->luma_weight_l0 != NULL){
							NBMemory_free(data->luma_weight_l0);
							data->luma_weight_l0 = NULL;
						}
						if(data->luma_offset_l0 != NULL){
							NBMemory_free(data->luma_offset_l0);
							data->luma_offset_l0 = NULL;
						}
						data->luma_weight_l0	= NBMemory_allocTypes(SI64, data->lumal0Use);
						data->luma_offset_l0	= NBMemory_allocTypes(SI64, data->lumal0Use);
						data->lumal0Sz			= data->lumal0Use;
					}
					if(data->lumal0Sz > 0){
						NBMemory_set(data->luma_weight_l0, 0, sizeof(data->luma_weight_l0[0]) * data->lumal0Sz);
						NBMemory_set(data->luma_offset_l0, 0, sizeof(data->luma_offset_l0[0]) * data->lumal0Sz);
					}
					//chroma
					data->chromal0Use = (UI16)chroma_l0Sz;
					if(data->chromal0Use > data->chromal0Sz){
						if(data->chroma_weight_l0 != NULL){
							NBMemory_free(data->chroma_weight_l0);
							data->chroma_weight_l0 = NULL;
						}
						if(data->chroma_offset_l0 != NULL){
							NBMemory_free(data->chroma_offset_l0);
							data->chroma_offset_l0 = NULL;
						}
						data->chroma_weight_l0	= NBMemory_allocTypes(SI64, data->chromal0Use);
						data->chroma_offset_l0	= NBMemory_allocTypes(SI64, data->chromal0Use);
						data->chromal0Sz			= data->chromal0Use;
					}
					if(data->chromal0Sz > 0){
						NBMemory_set(data->chroma_weight_l0, 0, sizeof(data->chroma_weight_l0[0]) * data->chromal0Sz);
						NBMemory_set(data->chroma_offset_l0, 0, sizeof(data->chroma_offset_l0[0]) * data->chromal0Sz);
					}
					//load
					for( i = 0; i <= slice_header->num_ref_idx_l0_active_minus1; i++ ) {
						NBAvcBitstream_readU8_(data, luma_weight_l0_flag, bs, 1); //2 u(1)
						if( data->luma_weight_l0_flag ) {
							NBAvcBitstream_readGolombSE64_(data, luma_weight_l0[i], bs); //[ i ] //2 se(v)
							NBAvcBitstream_readGolombSE64_(data, luma_offset_l0[i], bs); //[ i ] //2 se(v)
						}
						if ( ChromaArrayType != 0 ) {
							NBAvcBitstream_readU8_(data, chroma_weight_l0_flag, bs, 1); //2 u(1)
							if( data->chroma_weight_l0_flag ) {
								UI8 j; for( j = 0; j < 2; j++ ) {
									NBAvcBitstream_readGolombSE64_(data, chroma_weight_l0[(i * 2) + j], bs); //[ i ][ j ]; //2 se(v)
									NBAvcBitstream_readGolombSE64_(data, chroma_offset_l0[(i * 2) + j], bs); //[ i ][ j ]; //2 se(v) 
								}
							}
						}
					}
					//Print
#					ifdef NB_CONFIG_INCLUDE_ASSERTS
					{
						STNBString str;
						NBString_init(&str);
						NBAvc_pred_weight_concat(luma_l0Sz, data->luma_weight_l0, data->luma_offset_l0, data->chroma_weight_l0, data->chroma_offset_l0, &str);
						PRINTF_INFO("NBAvc_pred_weight_table_loadBits, luma_l0 table:\n%s\n", str.str);
						NBString_release(&str);
					}
#					endif
				}
				if((slice_header->slice_type % 5) == 1){
					UI32 i;
					const UI32 luma_l1Sz	= (UI32)slice_header->num_ref_idx_l1_active_minus1 + 1;
					const UI32 chroma_l1Sz	= (ChromaArrayType != 0 ? (UI32)(slice_header->num_ref_idx_l1_active_minus1 + 1) * 2 : 0);
					data->lumal1Use = (UI16)luma_l1Sz;
					if(data->lumal1Use > data->lumal1Sz){
						if(data->luma_weight_l1 != NULL){
							NBMemory_free(data->luma_weight_l1);
							data->luma_weight_l1 = NULL;
						}
						if(data->luma_offset_l1 != NULL){
							NBMemory_free(data->luma_offset_l1);
							data->luma_offset_l1 = NULL;
						}
						data->luma_weight_l1	= NBMemory_allocTypes(SI64, data->lumal1Use);
						data->luma_offset_l1	= NBMemory_allocTypes(SI64, data->lumal1Use);
						data->lumal1Sz			= data->lumal1Use;
					}
					if(data->lumal1Sz > 0){
						NBMemory_set(data->luma_weight_l1, 0, sizeof(data->luma_weight_l1[0]) * data->lumal1Sz);
						NBMemory_set(data->luma_offset_l1, 0, sizeof(data->luma_offset_l1[0]) * data->lumal1Sz);
					}
					//chroma
					data->chromal1Use = (UI16)chroma_l1Sz;
					if(data->chromal1Use > data->chromal1Sz){
						if(data->chroma_weight_l1 != NULL){
							NBMemory_free(data->chroma_weight_l1);
							data->chroma_weight_l1 = NULL;
						}
						if(data->chroma_offset_l1 != NULL){
							NBMemory_free(data->chroma_offset_l1);
							data->chroma_offset_l1 = NULL;
						}
						data->chroma_weight_l1		= NBMemory_allocTypes(SI64, data->chromal1Use);
						data->chroma_offset_l1		= NBMemory_allocTypes(SI64, data->chromal1Use);
						data->chromal1Sz			= data->chromal1Use;
					}
					if(data->chromal1Sz > 0){
						NBMemory_set(data->chroma_weight_l1, 0, sizeof(data->chroma_weight_l1[0]) * data->chromal1Sz);
						NBMemory_set(data->chroma_offset_l1, 0, sizeof(data->chroma_offset_l1[0]) * data->chromal1Sz);
					}
					//load
					for( i = 0; i <= slice_header->num_ref_idx_l1_active_minus1; i++ ) {
						NBAvcBitstream_readU8_(data, luma_weight_l1_flag, bs, 1); //2 u(1)
						if( data->luma_weight_l1_flag ) {
							NBAvcBitstream_readGolombSE64_(data, luma_weight_l1[i], bs); //[ i ]; //2 se(v)
							NBAvcBitstream_readGolombSE64_(data, luma_offset_l1[i], bs); //[ i ]; //2 se(v)
						}
						if( ChromaArrayType != 0 ) {
							NBAvcBitstream_readU8_(data, chroma_weight_l1_flag, bs, 1); //2 u(1)
							if( data->chroma_weight_l1_flag ){
								UI8 j; for( j = 0; j < 2; j++ ) {
									NBAvcBitstream_readGolombSE64_(data, chroma_weight_l1[(i * 2) + j], bs); //[ i ][ j ] //2 se(v)
									NBAvcBitstream_readGolombSE64_(data, chroma_offset_l1[(i * 2) + j], bs); //[ i ][ j ] //2 se(v)
								}
							}
						}
					}
					//Print
#					ifdef NB_CONFIG_INCLUDE_ASSERTS
					{
						STNBString str;
						NBString_init(&str);
						NBAvc_pred_weight_concat(luma_l1Sz, data->luma_weight_l1, data->luma_offset_l1, data->chroma_weight_l1, data->chroma_offset_l1, &str);
						PRINTF_INFO("NBAvc_pred_weight_table_loadBits, luma_l1 table:\n%s\n", str.str);
						NBString_release(&str);
					}
#					endif
				}
				//
				if(bs->stopFlags){
					r = FALSE;
				}
			}
		}
	}
	return r;
}

void NBAvc_pred_weight_concatValue_(const SI64 val, const UI8 maxLen, STNBString* dst){
	UI8 lenVal = 1;
	SI64 val2;
	if(val < 0){
		val2 = val;
		while(val2 < 0){
			val2 /= 10;
			lenVal++;
		}
	} else {
		val2 = val / 10;
		while(val2 > 0){
			val2 /= 10;
			lenVal++;
		}
	}
	if(lenVal < maxLen){
		NBString_concatRepeatedByte(dst, ' ', maxLen - lenVal);
	}
	NBString_concatSI64(dst, val);
}
							  
void NBAvc_pred_weight_concat(const UI32 lumaSz, const SI64* luma_weight, const SI64* luma_offset, const SI64* chroma_weight, const SI64* chroma_offset, STNBString* dst){
	//luma_weight offset (chroma_weight offset)
	//                   (chroma_weight offset)
	const BOOL isLumma	= (luma_weight != NULL && luma_offset != NULL);
	const BOOL isChroma	= (chroma_weight != NULL && chroma_offset != NULL); 
	if((isLumma || isChroma) && dst != NULL){
		UI32 i; SI64 val, colsMinNeg[4] = { 0, 0, 0, 0 }, colsMaxPos[4] = { 0, 0, 0, 0 };
		UI8 colsLen[4] = { 0, 0, 0, 0 };
		//Accumulate values
		if(!isLumma){
			if(isChroma){
				for(i = 0; i < lumaSz; i++){
					val = chroma_weight[i * 2];
					if(val < 0){
						if(colsMinNeg[0] > val) colsMinNeg[0] = val;
					} else {
						if(colsMaxPos[0] < val) colsMaxPos[0] = val;
					}
					val = chroma_weight[(i * 2) + 1];
					if(val < 0){
						if(colsMinNeg[0] > val) colsMinNeg[0] = val;
					} else {
						if(colsMaxPos[0] < val) colsMaxPos[0] = val;
					}
					val = chroma_offset[i * 2];
					if(val < 0){
						if(colsMinNeg[1] > val) colsMinNeg[1] = val;
					} else {
						if(colsMaxPos[1] < val) colsMaxPos[1] = val;
					}
					val = chroma_offset[(i * 2) + 1];
					if(val < 0){
						if(colsMinNeg[1] > val) colsMinNeg[1] = val;
					} else {
						if(colsMaxPos[1] < val) colsMaxPos[1] = val;
					}
				}
			}
		} else if(!isChroma){
			for(i = 0; i < lumaSz; i++){
				val = luma_weight[i];
				if(val < 0){
					if(colsMinNeg[0] > val) colsMinNeg[0] = val;
				} else {
					if(colsMaxPos[0] < val) colsMaxPos[0] = val;
				}
				val = luma_offset[i];
				if(val < 0){
					if(colsMinNeg[1] > val) colsMinNeg[1] = val;
				} else {
					if(colsMaxPos[1] < val) colsMaxPos[1] = val;
				}
			}
		} else {
			for(i = 0; i < lumaSz; i++){
				val = luma_weight[i];
				if(val < 0){
					if(colsMinNeg[0] > val) colsMinNeg[0] = val;
				} else {
					if(colsMaxPos[0] < val) colsMaxPos[0] = val;
				}
				val = luma_offset[i];
				if(val < 0){
					if(colsMinNeg[1] > val) colsMinNeg[1] = val;
				} else {
					if(colsMaxPos[1] < val) colsMaxPos[1] = val;
				}
				//chroma
				val = chroma_weight[i * 2];
				if(val < 0){
					if(colsMinNeg[2] > val) colsMinNeg[2] = val;
				} else {
					if(colsMaxPos[2] < val) colsMaxPos[2] = val;
				}
				val = chroma_weight[(i * 2) + 1];
				if(val < 0){
					if(colsMinNeg[2] > val) colsMinNeg[2] = val;
				} else {
					if(colsMaxPos[2] < val) colsMaxPos[2] = val;
				}
				val = chroma_offset[i * 2];
				if(val < 0){
					if(colsMinNeg[3] > val) colsMinNeg[3] = val;
				} else {
					if(colsMaxPos[3] < val) colsMaxPos[3] = val;
				}
				val = chroma_offset[(i * 2) + 1];
				if(val < 0){
					if(colsMinNeg[3] > val) colsMinNeg[3] = val;
				} else {
					if(colsMaxPos[3] < val) colsMaxPos[3] = val;
				}
			}
		}
		//Calculate cols len
		{
			UI8 lenNeg = 0, lenPos = 0;
			for(i = 0; i < 4; i++){
				//
				lenNeg = 1; //negative sign
				val = colsMinNeg[i];
				while(val < 0){
					val /= 10;
					lenNeg++;
				}
				//
				lenPos = 1; 
				val = colsMaxPos[i] / 10;
				while(val > 0){
					val /= 10;
					lenPos++;
				}
				//
				colsLen[i] = ( lenNeg > lenPos ? lenNeg : lenPos );
			}
		}
		//concat
		if(!isLumma){
			if(isChroma){
				for(i = 0; i < lumaSz; i++){
					NBString_concatByte(dst, '#');
					NBString_concatUI32(dst, i);
					NBString_concatByte(dst, ')');
					NBString_concatByte(dst, ' ');
					//
					NBAvc_pred_weight_concatValue_(chroma_weight[i * 2], colsLen[0], dst);
					NBString_concatByte(dst, ' ');
					//
					NBAvc_pred_weight_concatValue_(chroma_weight[(i * 2) + 1], colsLen[1], dst);
					NBString_concatByte(dst, ' ');
					//
					NBAvc_pred_weight_concatValue_(chroma_offset[i * 2], colsLen[2], dst);
					NBString_concatByte(dst, ' ');
					//
					NBAvc_pred_weight_concatValue_(chroma_offset[(i * 2) + 1], colsLen[3], dst);
					NBString_concatByte(dst, '\n');
				}
			}
		} else if(!isChroma){
			for(i = 0; i < lumaSz; i++){
				NBString_concatByte(dst, '#');
				NBString_concatUI32(dst, i);
				NBString_concatByte(dst, ')');
				NBString_concatByte(dst, ' ');
				//
				NBAvc_pred_weight_concatValue_(luma_weight[i], colsLen[0], dst);
				NBString_concatByte(dst, ' ');
				//
				NBAvc_pred_weight_concatValue_(luma_offset[i], colsLen[0], dst);
				NBString_concatByte(dst, '\n');
			}
		} else {
			for(i = 0; i < lumaSz; i++){
				NBString_concatByte(dst, '#');
				NBString_concatUI32(dst, i);
				NBString_concatByte(dst, ')');
				NBString_concatByte(dst, ' ');
				//
				val = luma_weight[i];
				NBAvc_pred_weight_concatValue_(luma_weight[i], colsLen[0], dst);
				NBString_concatByte(dst, ' ');
				//
				val = luma_offset[i];
				NBAvc_pred_weight_concatValue_(luma_weight[i], colsLen[1], dst);
				NBString_concatByte(dst, ' ');
				//
				//chroma
				val = chroma_weight[i * 2];
				NBAvc_pred_weight_concatValue_(luma_weight[i], colsLen[2], dst);
				NBString_concatByte(dst, ' ');
				//
				val = chroma_offset[i * 2];
				NBAvc_pred_weight_concatValue_(luma_weight[i], colsLen[3], dst);
				NBString_concatByte(dst, '\n');
				//
				//space
				NBString_concatRepeatedByte(dst, ' ', colsLen[0] + 1 + colsLen[1] + 1);
				//
				//chroma
				val = chroma_weight[(i * 2) + 1];
				NBAvc_pred_weight_concatValue_(luma_weight[i], colsLen[2], dst);
				NBString_concatByte(dst, ' ');
				//
				val = chroma_offset[(i * 2) + 1];
				NBAvc_pred_weight_concatValue_(luma_weight[i], colsLen[3], dst);
				NBString_concatByte(dst, '\n');
				//
			}
		}
	}
}

//dec_ref_pic_marking

void* NBAvc_dec_ref_pic_marking_createSt(void){
	STNBAvc_dec_ref_pic_marking* r = NBMemory_allocType(STNBAvc_dec_ref_pic_marking);
	NBMemory_setZeroSt(*r, STNBAvc_dec_ref_pic_marking);
	return r;
}

void NBAvc_dec_ref_pic_marking_releaseSt(void* pData){
	STNBAvc_dec_ref_pic_marking* data = (STNBAvc_dec_ref_pic_marking*)pData;
	NBMemory_free(data);
	data = NULL;
}

BOOL NBAvc_dec_ref_pic_marking_loadBits(STNBAvcBitstream* bs, const STNBAvcStateParams* params, void* pData, const BOOL picDescBaseOnly){
	BOOL r = FALSE;
	if(bs != NULL && !bs->stopFlags && pData != NULL){
		STNBAvc_dec_ref_pic_marking* data = (STNBAvc_dec_ref_pic_marking*)pData;
		const BOOL IdrPicFlag = ((params->hdr.type == 5) ? 1 : 0);
		r = TRUE;
		if( IdrPicFlag ) {
			NBAvcBitstream_readU8_(data, no_output_of_prior_pics_flag, bs, 1); //2|5 u(1)
			NBAvcBitstream_readU8_(data, long_term_reference_flag, bs, 1); //2|5 u(1)
		} else {
			NBAvcBitstream_readU8_(data, adaptive_ref_pic_marking_mode_flag, bs, 1); //2|5 u(1)
			if( data->adaptive_ref_pic_marking_mode_flag ){
				do {
					NBAvcBitstream_readGolombUE64_(data, memory_management_control_operation, bs); //2|5 ue(v)
					if(data->memory_management_control_operation == 1 || data->memory_management_control_operation == 3){
						NBAvcBitstream_readGolombUE64_(data, difference_of_pic_nums_minus1, bs); //2|5 ue(v)
					} 
					if(data->memory_management_control_operation == 2 ){
						NBAvcBitstream_readGolombUE64_(data, long_term_pic_num, bs); //2|5 ue(v)
					}
					if(data->memory_management_control_operation == 3 || data->memory_management_control_operation == 6){
						NBAvcBitstream_readGolombUE64_(data, long_term_frame_idx, bs); //2|5 ue(v)
					}
					if(data->memory_management_control_operation == 4){
						NBAvcBitstream_readGolombUE64_(data, max_long_term_frame_idx_plus1, bs); //2|5 ue(v)
					}
				} while( data->memory_management_control_operation != 0 && !bs->stopFlags);
			}
		}
		//
		if(bs->stopFlags){
			r = FALSE;
		}
	}
	return r;
}

//slice_header

void* NBAvc_slice_header_createSt(void){
	STNBAvc_slice_header* r = NBMemory_allocType(STNBAvc_slice_header);
	NBMemory_setZeroSt(*r, STNBAvc_slice_header);
	return r;
}

void NBAvc_slice_header_releaseSt(void* pData){
	STNBAvc_slice_header* data = (STNBAvc_slice_header*)pData;
	//
	if(data->ref_pic_list_mvc_modification != NULL){
		NBAvc_ref_pic_list_mvc_modification_releaseSt(data->ref_pic_list_mvc_modification);
		data->ref_pic_list_mvc_modification = NULL;
	}
	if(data->ref_pic_list_modification != NULL){
		NBAvc_ref_pic_list_modification_releaseSt(data->ref_pic_list_modification);
		data->ref_pic_list_modification = NULL;
	}
	if(data->pred_weight_table != NULL){
		NBAvc_pred_weight_table_releaseSt(data->pred_weight_table);
		data->pred_weight_table = NULL;
	}
	if(data->dec_ref_pic_marking != NULL){
		NBAvc_dec_ref_pic_marking_releaseSt(data->dec_ref_pic_marking);
		data->dec_ref_pic_marking = NULL;
	}
	//
	NBMemory_free(data);
	data = NULL;
}

BOOL NBAvc_slice_header_loadBits(STNBAvcBitstream* bs, const STNBAvcStateParams* params, void* pData, const BOOL picDescBaseOnly){
	BOOL r = FALSE;
	if(bs != NULL && !bs->stopFlags && pData != NULL){
		STNBAvc_slice_header* data = (STNBAvc_slice_header*)pData;
		r = TRUE;
		NBAvcBitstream_readGolombUE64_(data, first_mb_in_slice, bs);	//2 ue(v)
		NBAvcBitstream_readGolombUE64_(data, slice_type, bs); //2 ue(v)
		NBAvcBitstream_readGolombUE64_(data, pic_parameter_set_id, bs); //2 ue(v)
		if(params == NULL || params->seq_parameter_set_rbsp == NULL || params->seq_parameter_set_rbsp->data == NULL){
			r = FALSE;
		} else if(data->slice_type > 9){ //slice_type should be between 0 and 9
			r = FALSE;
		} else {
			STNBAvc_seq_parameter_set_data*	spsData = params->seq_parameter_set_rbsp->data;
			if( spsData->separate_colour_plane_flag == 1 ){
				NBAvcBitstream_readU8_(data, colour_plane_id, bs, 2); //2 u(2)
			}
			NBAvcBitstream_readU32v_(data, frame_num, bs, (UI8)(spsData->log2_max_frame_num_minus4 + 4)); //2 u(v)
			if( !spsData->frame_mbs_only_flag ) {
				NBAvcBitstream_readU8_(data, field_pic_flag, bs, 1); //2 u(1)
				if( data->field_pic_flag ){
					NBAvcBitstream_readU8_(data, bottom_field_flag, bs, 1); //2 u(1)
				}
			}
			//stop after 'field_pic_flag' (if picDescBaseOnly)
			if(params->pic_parameter_set_rbsp == NULL){
				r = FALSE;
			} else {
				const BOOL IdrPicFlag = ((params->hdr.type == 5) ? 1 : 0);
				if( IdrPicFlag ){
					NBAvcBitstream_readGolombUE64_(data, idr_pic_id, bs); //2 ue(v)
				}
				if( spsData->pic_order_cnt_type == 0 ) {
					NBAvcBitstream_readU32v_(data, pic_order_cnt_lsb, bs, (UI8)(spsData->log2_max_pic_order_cnt_lsb_minus4 + 4)); //2 u(v) 
					if( params->pic_parameter_set_rbsp->bottom_field_pic_order_in_frame_present_flag && !data->field_pic_flag ){
						NBAvcBitstream_readGolombSE64_(data, delta_pic_order_cnt_bottom, bs); //2 se(v)
					}
				}
				if( spsData->pic_order_cnt_type == 1 && !spsData->delta_pic_order_always_zero_flag ) {
					NBAvcBitstream_readGolombSE64_(data, delta_pic_order_cnt[ 0 ], bs); //2 se(v)
					if( params->pic_parameter_set_rbsp->bottom_field_pic_order_in_frame_present_flag && !data->field_pic_flag ){
						NBAvcBitstream_readGolombSE64_(data, delta_pic_order_cnt[ 1 ], bs); //2 se(v)
					}
				}
				if( params->pic_parameter_set_rbsp->redundant_pic_cnt_present_flag ){
					NBAvcBitstream_readGolombUE64_(data, redundant_pic_cnt, bs); //2 ue(v)
				}
				if( NBAvc_slice_type_is_B(data->slice_type) ){
					NBAvcBitstream_readU8_(data, direct_spatial_mv_pred_flag, bs, 1); //2 u(1)
				}
				if(NBAvc_slice_type_is_P(data->slice_type) || NBAvc_slice_type_is_SP(data->slice_type) || NBAvc_slice_type_is_B(data->slice_type)){
					NBAvcBitstream_readU8_(data, num_ref_idx_active_override_flag, bs, 1); //2 u(1)
					if( data->num_ref_idx_active_override_flag ) {
						NBAvcBitstream_readGolombUE64_(data, num_ref_idx_l0_active_minus1, bs); //2 ue(v)
						if( NBAvc_slice_type_is_B(data->slice_type) ){
							NBAvcBitstream_readGolombUE64_(data, num_ref_idx_l1_active_minus1, bs); //2 ue(v)
						}
					}
				}
				//
				data->ref_pic_list_mvc_modificationPresent	= FALSE;
				data->ref_pic_list_modificationPresent		= FALSE;
				data->pred_weight_tablePresent				= FALSE;
				data->dec_ref_pic_markingPresent			= FALSE;
				//
				if( params->hdr.type == 20 || params->hdr.type == 21 ){
					if(data->ref_pic_list_mvc_modification == NULL){
						data->ref_pic_list_mvc_modification = NBAvc_ref_pic_list_mvc_modification_createSt();
					}
					//ref_pic_list_mvc_modification( ) // specified in Annex H
					if(data->ref_pic_list_mvc_modification == NULL){
						PRINTF_ERROR("NBAvc_slice_header_loadBits, NBAvc_ref_pic_list_mvc_modification_createSt failed.\n");
						r = FALSE;
					} else if(!NBAvc_ref_pic_list_mvc_modification_loadBits(bs, params, data->ref_pic_list_mvc_modification, data->slice_type)){
						PRINTF_ERROR("NBAvc_slice_header_loadBits, NBAvc_ref_pic_list_mvc_modification_loadBits failed.\n");
						r = FALSE;
					} else {
						data->ref_pic_list_mvc_modificationPresent = TRUE;
					}
				} else {
					if(data->ref_pic_list_modification == NULL){
						data->ref_pic_list_modification = NBAvc_ref_pic_list_modification_createSt();
					}
					if(data->ref_pic_list_modification == NULL){
						PRINTF_ERROR("NBAvc_slice_header_loadBits, NBAvc_ref_pic_list_modification_createSt failed.\n");
						r = FALSE;
					} else if(!NBAvc_ref_pic_list_modification_loadBits(bs, params, data->ref_pic_list_modification, data->slice_type)){
						PRINTF_ERROR("NBAvc_slice_header_loadBits, NBAvc_ref_pic_list_modification_loadBits failed.\n");
						r = FALSE;
					} else {
						data->ref_pic_list_modificationPresent = TRUE;
					}
				}
				if( ( params->pic_parameter_set_rbsp->weighted_pred_flag && ( NBAvc_slice_type_is_P(data->slice_type) || NBAvc_slice_type_is_SP(data->slice_type) ) ) || ( params->pic_parameter_set_rbsp->weighted_bipred_idc == 1 && NBAvc_slice_type_is_B(data->slice_type) ) ){
					if(data->pred_weight_table == NULL){
						data->pred_weight_table = NBAvc_pred_weight_table_createSt();
					}
					if(data->pred_weight_table == NULL){
						r = FALSE;
					} else if(!NBAvc_pred_weight_table_loadBits(bs, params, data->pred_weight_table, data)){
						PRINTF_ERROR("NBAvc_slice_header_loadBits, NBAvc_pred_weight_table_loadBits failed.\n");
						r = FALSE;
					} else {
						data->pred_weight_tablePresent = TRUE;
					}
				}
				if( params->hdr.refIdc != 0 ){
					if(data->dec_ref_pic_marking == NULL){
						data->dec_ref_pic_marking = NBAvc_dec_ref_pic_marking_createSt();
					}
					if(data->dec_ref_pic_marking == NULL){
						r = FALSE;
					} else if(!NBAvc_dec_ref_pic_marking_loadBits(bs, params,  data->dec_ref_pic_marking, picDescBaseOnly)){
						r = FALSE;
					} else {
						data->dec_ref_pic_markingPresent = TRUE;
					}
				}
				if( params->pic_parameter_set_rbsp->entropy_coding_mode_flag && !NBAvc_slice_type_is_I(data->slice_type) && !NBAvc_slice_type_is_SI(data->slice_type) ){
					NBAvcBitstream_readGolombUE64_(data, cabac_init_idc, bs); //2 ue(v)
					//The value of cabac_init_idc shall be in the range of 0 to 2, inclusive.
					if(data->cabac_init_idc > 2){
                        bs->stopFlags |= NB_AVC_BITSTREAM_STOP_VALUE_LOGIC_ERROR;
                        //NBASSERT(data->cabac_init_idc >= 0 && data->cabac_init_idc <= 2)
						r = FALSE;
					}
					
				}
				NBAvcBitstream_readGolombSE64_(data, slice_qp_delta, bs); //2 se(v)
				if( NBAvc_slice_type_is_SP(data->slice_type) || NBAvc_slice_type_is_SI(data->slice_type) ){
					if( NBAvc_slice_type_is_SP(data->slice_type) ){
						NBAvcBitstream_readU8_(data, sp_for_switch_flag, bs, 1); //2 u(1)
					}
					NBAvcBitstream_readGolombSE64_(data, slice_qs_delta, bs); //2 se(v)
				}
				if( params->pic_parameter_set_rbsp->deblocking_filter_control_present_flag ) {
					NBAvcBitstream_readGolombUE64_(data, disable_deblocking_filter_idc, bs); //2 ue(v)
					if( data->disable_deblocking_filter_idc != 1 ) {
						NBAvcBitstream_readGolombSE64_(data, slice_alpha_c0_offset_div2, bs); //2 se(v)
						NBAvcBitstream_readGolombSE64_(data, slice_beta_offset_div2, bs); //2 se(v)
					}
				}
				if( params->pic_parameter_set_rbsp->num_slice_groups_minus1 > 0 && params->pic_parameter_set_rbsp->slice_group_map_type >= 3 && params->pic_parameter_set_rbsp->slice_group_map_type <= 5){
					const UI64 PicWidthInMbs		= spsData->pic_width_in_mbs_minus1 + 1;
					const UI64 PicHeightInMapUnits	= spsData->pic_height_in_map_units_minus1 + 1;
					const UI64 PicSizeInMapUnits	= PicWidthInMbs * PicHeightInMapUnits;
					const UI64 SliceGroupChangeRate	= params->pic_parameter_set_rbsp->slice_group_change_rate_minus1 + 1;
					const UI8 ammBits				= (UI8)ceil( log2( (double)PicSizeInMapUnits / (double)SliceGroupChangeRate + 1.0 ) );
					NBAvcBitstream_readU32v_(data, slice_group_change_cycle, bs, ammBits); //2 u(v)
				}
			}
		}
		//
		if(bs->stopFlags){
			r = FALSE;
		}
	}
	return r;
}

//slice data

void* NBAvc_slice_data_createSt(void){
	STNBAvc_slice_data* r = NBMemory_allocType(STNBAvc_slice_data);
	NBMemory_setZeroSt(*r, STNBAvc_slice_data);
	return r;
}

void NBAvc_slice_data_releaseSt(void* pObj){
	STNBAvc_slice_data* obj = (STNBAvc_slice_data*)pObj;
	if(obj != NULL){
		NBMemory_free(obj);
		obj = NULL;
	}
}

//NBAvc_more_rbsp_data 

UI32 NBAvc_NextMbAddress(const UI32 n, const UI32 PicSizeInMbs, const UI32* MbToSliceGroupMap){
	UI32 i = n + 1;
	while( i < PicSizeInMbs && MbToSliceGroupMap[ i ] != MbToSliceGroupMap[ n ] ){
		i++;
	}
	return i;
}

BOOL NBAvc_slice_data_loadBits(STNBAvcBitstream* bs, const STNBAvcStateParams* params, void* pData, const STNBAvc_slice_header* slice_header){
	BOOL r = FALSE;
	if(bs != NULL && !bs->stopFlags && pData != NULL){
		STNBAvc_slice_data* data = (STNBAvc_slice_data*)pData;
		if(params != NULL && slice_header != NULL){
			if(params->seq_parameter_set_rbsp != NULL && params->pic_parameter_set_rbsp != NULL){
				if(params->seq_parameter_set_rbsp->data != NULL){
					const UI32 PicWidthInMbs			= (UI32)params->seq_parameter_set_rbsp->data->pic_width_in_mbs_minus1 + 1;
					const UI32 PicHeightInMapUnits		= (UI32)params->seq_parameter_set_rbsp->data->pic_height_in_map_units_minus1 + 1;
					const UI32 FrameHeightInMbs			= ( 2 - params->seq_parameter_set_rbsp->data->frame_mbs_only_flag ) * PicHeightInMapUnits;
					const UI32 PicHeightInMbs			= FrameHeightInMbs / ( 1 + slice_header->field_pic_flag );
					const UI32 PicSizeInMbs				= PicWidthInMbs * PicHeightInMbs;
					const UI32 PicSizeInMapUnits		= PicWidthInMbs * PicHeightInMapUnits; NBASSERT(PicSizeInMapUnits > 0)
					const UI32 SliceGroupChangeRate		= (UI32)params->pic_parameter_set_rbsp->slice_group_change_rate_minus1 + 1;
					const BOOL MbaffFrameFlag			= ( params->seq_parameter_set_rbsp->data->mb_adaptive_frame_field_flag && !slice_header->field_pic_flag );
					const UI32 MapUnitsInSliceGroup0	= (((UI32)slice_header->slice_group_change_cycle * SliceGroupChangeRate) < PicSizeInMapUnits ? ((UI32)slice_header->slice_group_change_cycle * SliceGroupChangeRate) : PicSizeInMapUnits);
					const UI32 sizeOfUpperLeftGroup		= ( params->pic_parameter_set_rbsp->slice_group_change_direction_flag ? ( PicSizeInMapUnits - MapUnitsInSliceGroup0 ) : MapUnitsInSliceGroup0 );
					BOOL moreDataFlag					= 1;
					BOOL prevMbSkipped					= 0;
					UI32 CurrMbAddr						= (UI32)slice_header->first_mb_in_slice * ( 1 + MbaffFrameFlag );
					//ToDo: build only once
					UI32* MbToSliceGroupMap				= NBMemory_allocTypes(UI32, PicSizeInMbs);
					UI32* mapUnitToSliceGroupMap		= NBMemory_allocTypes(UI32, PicSizeInMapUnits);
					NBMemory_set(MbToSliceGroupMap, 0, sizeof(MbToSliceGroupMap[0]) * PicSizeInMbs);
					NBMemory_set(mapUnitToSliceGroupMap, 0, sizeof(mapUnitToSliceGroupMap[0]) * PicSizeInMapUnits);
					r = TRUE;
					//Build mapUnitToSliceGroupMap
					if(params->pic_parameter_set_rbsp->num_slice_groups_minus1 > 0){
						switch(params->pic_parameter_set_rbsp->slice_group_map_type) {
							case 0:
								if(params->pic_parameter_set_rbsp->run_length_minus1 == NULL || params->pic_parameter_set_rbsp->run_length_minus1 == NULL){
									r = FALSE;
								} else {
									UI32 i = 0, iGroup, j;
									do {
										for( iGroup = 0; iGroup <= params->pic_parameter_set_rbsp->num_slice_groups_minus1 && i < PicSizeInMapUnits; i += (UI32)params->pic_parameter_set_rbsp->run_length_minus1[ iGroup++ ] + 1 ){
											for( j = 0; j <= params->pic_parameter_set_rbsp->run_length_minus1[ iGroup ] && i + j < PicSizeInMapUnits; j++ ) {
												mapUnitToSliceGroupMap[ i + j ] = iGroup;
											}
										}	
									} while( i < PicSizeInMapUnits && !bs->stopFlags);
								}
								break;
							case 1:
								{
									UI32 i;
									for( i = 0; i < PicSizeInMapUnits; i++ ){
										mapUnitToSliceGroupMap[ i ] = ( ( i % PicWidthInMbs ) + ( ( ( i / PicWidthInMbs ) * ( (UI32)params->pic_parameter_set_rbsp->num_slice_groups_minus1 + 1 ) ) / 2 ) ) % ( (UI32)params->pic_parameter_set_rbsp->num_slice_groups_minus1 + 1 );
									}
								}
								break;
							case 2:
								{
									SI32 iGroup; UI32 i, x, y, yTopLeft, xTopLeft, yBottomRight, xBottomRight;
									for( i = 0; i < PicSizeInMapUnits; i++ ){
										mapUnitToSliceGroupMap[ i ] = (UI32)params->pic_parameter_set_rbsp->num_slice_groups_minus1;
									}
									for( iGroup = (SI32)params->pic_parameter_set_rbsp->num_slice_groups_minus1 - 1; iGroup >= 0; iGroup--) {
										yTopLeft		= (UI32)params->pic_parameter_set_rbsp->top_left[ iGroup ] / PicWidthInMbs;
										xTopLeft		= (UI32)params->pic_parameter_set_rbsp->top_left[ iGroup ] % PicWidthInMbs;
										yBottomRight	= (UI32)params->pic_parameter_set_rbsp->bottom_right[ iGroup ] / PicWidthInMbs;
										xBottomRight	= (UI32)params->pic_parameter_set_rbsp->bottom_right[ iGroup ] % PicWidthInMbs;
										for( y = yTopLeft; y <= yBottomRight; y++ ){
											for( x = xTopLeft; x <= xBottomRight; x++ ){
												mapUnitToSliceGroupMap[ y * PicWidthInMbs + x ] = iGroup;
											}
										}
									}
								}
								break;
							case 3:
								{
									UI32 i; SI32 x, y, k, leftBound, topBound, rightBound, bottomBound, xDir, yDir;
									BOOL mapUnitVacant = FALSE;
									for( i = 0; i < PicSizeInMapUnits; i++ ){
										mapUnitToSliceGroupMap[ i ] = 1;
									}
									x = ( (SI32)PicWidthInMbs - (SI32)params->pic_parameter_set_rbsp->slice_group_change_direction_flag ) / 2;
									y = ( (SI32)PicHeightInMapUnits - (SI32)params->pic_parameter_set_rbsp->slice_group_change_direction_flag ) / 2;
									leftBound	= x;
									topBound	= y;
									rightBound	= x;
									bottomBound	= y;
									xDir		= params->pic_parameter_set_rbsp->slice_group_change_direction_flag - 1;
									yDir		= params->pic_parameter_set_rbsp->slice_group_change_direction_flag;
									for( k = 0; k < MapUnitsInSliceGroup0; k += mapUnitVacant ) {
										mapUnitVacant = ( mapUnitToSliceGroupMap[ y * PicWidthInMbs + x ] == 1 );
										if( mapUnitVacant ){
											mapUnitToSliceGroupMap[ y * PicWidthInMbs + x ] = 0;
										}
										if(xDir == -1 && x == leftBound){
											leftBound	= ((leftBound  - 1) > 0 ? (leftBound - 1) : 0); //Max(leftBound - 1 , 0 );
											x			= leftBound;
											xDir		= 0;
											yDir		= 2 * params->pic_parameter_set_rbsp->slice_group_change_direction_flag - 1;
										} else if( xDir == 1 && x == rightBound ) {
											rightBound	= ((rightBound + 1) < ((SI32)PicWidthInMbs - 1) ? (rightBound + 1) : ((SI32)PicWidthInMbs - 1)); //Min( rightBound + 1, PicWidthInMbs - 1 );
											x			= rightBound;
											xDir		= 0;
											yDir		=  1 - 2 * params->pic_parameter_set_rbsp->slice_group_change_direction_flag;
										} else if( yDir == -1 && y == topBound ) {
											topBound	= ((topBound - 1) > 0 ? (topBound - 1) : 0); //Max(topBound - 1 , 0 );
											y			= topBound;
											xDir		= 1 - 2 * params->pic_parameter_set_rbsp->slice_group_change_direction_flag;
											yDir		= 0;
										} else if( yDir == 1 && y == bottomBound ) {
											bottomBound = ((bottomBound + 1) < ((SI32)PicHeightInMapUnits - 1) ? (bottomBound + 1) : ((SI32)PicHeightInMapUnits - 1)); //Min( bottomBound + 1, PicHeightInMapUnits - 1 );
											y			= bottomBound;
											xDir		= 2 * params->pic_parameter_set_rbsp->slice_group_change_direction_flag - 1;
											yDir		= 0;
										} else {
											x			= x + xDir;
											y			= y + yDir;
										}
									}
								}
								break;
							case 4:
								{
									UI32 i;
									for( i = 0; i < PicSizeInMapUnits; i++ ){
										if( i < sizeOfUpperLeftGroup ){
											mapUnitToSliceGroupMap[ i ] = params->pic_parameter_set_rbsp->slice_group_change_direction_flag;
										} else {
											mapUnitToSliceGroupMap[ i ] = 1 - params->pic_parameter_set_rbsp->slice_group_change_direction_flag;
										}
									}
								}
								break;
							case 5:
								{
									UI32 i, j, k = 0;
									for( j = 0; j < PicWidthInMbs; j++ ){
										for( i = 0; i < PicHeightInMapUnits; i++ ) {
											if( k++ < sizeOfUpperLeftGroup ) {
												mapUnitToSliceGroupMap[ i * PicWidthInMbs + j ] = params->pic_parameter_set_rbsp->slice_group_change_direction_flag;
											} else {
												mapUnitToSliceGroupMap[ i * PicWidthInMbs + j ] = 1 - params->pic_parameter_set_rbsp->slice_group_change_direction_flag;
											}
										}
									}
								}
								break;
							case 6:
								{
									UI32 i; for(i = 0; i < PicSizeInMapUnits; i++){
										mapUnitToSliceGroupMap[ i ] = params->pic_parameter_set_rbsp->slice_group_id[ i ];
									}
								}
								break;
							default:
								NBASSERT(FALSE)
								break;
						}
#						ifdef NB_CONFIG_INCLUDE_ASSERTS
						/*{
							STNBString str;
							NBString_init(&str);
							NBAvc_tableUI32_concat(mapUnitToSliceGroupMap, PicWidthInMbs, PicHeightInMapUnits, &str);
							PRINTF_INFO("NBAvc_slice_data_loadBits, mapUnitToSliceGroupMap:\n%s\n", str.str);
							NBString_release(&str);
						}*/
#						endif
					}
					//Build MbToSliceGroupMap
					{
						if(params->seq_parameter_set_rbsp->data->frame_mbs_only_flag || slice_header->field_pic_flag){
							UI32 i; for(i = 0; i < PicSizeInMbs; i++){
								MbToSliceGroupMap[ i ] = mapUnitToSliceGroupMap[ i ];
							}
						} else if(MbaffFrameFlag){
							UI32 i; for(i = 0; i < PicSizeInMbs; i++){
								MbToSliceGroupMap[ i ] = mapUnitToSliceGroupMap[ i / 2 ];
							}
						} else {
							UI32 i; for(i = 0; i < PicSizeInMbs; i++){
								MbToSliceGroupMap[ i ] = mapUnitToSliceGroupMap[ ( i / ( 2 * PicWidthInMbs ) ) * PicWidthInMbs + ( i % PicWidthInMbs ) ];
							}
						}
#						ifdef NB_CONFIG_INCLUDE_ASSERTS
						/*{
							STNBString str;
							NBString_init(&str);
							NBAvc_tableUI32_concat(MbToSliceGroupMap, PicWidthInMbs, PicHeightInMbs, &str);
							PRINTF_INFO("NBAvc_slice_data_loadBits, MbToSliceGroupMap:\n%s\n", str.str);
							NBString_release(&str);
						}*/
#						endif
					}
					//
					if( params->pic_parameter_set_rbsp->entropy_coding_mode_flag ){
						while( bs->bitsRemain != 8 && !bs->stopFlags){ //byte_aligned()
							NBAvcBitstream_readU8_(data, cabac_alignment_one_bit, bs, 1); //2 f(1)
							if(!data->cabac_alignment_one_bit){
								r = FALSE;
								break;
							}
						}
					}
					//
					//r = FALSE; //force bad result
					if(r){
						//9.3 CABAC parsing process for slice data
						do {
							if( !NBAvc_slice_type_is_I(slice_header->slice_type) && !NBAvc_slice_type_is_SI(slice_header->slice_type) ){
								if( !params->pic_parameter_set_rbsp->entropy_coding_mode_flag ) {
									UI64 i;
									NBAvcBitstream_readGolombUE64_(data, mb_skip_run, bs); //2 ue(v)
									//The value of mb_skip_run shall be in the range of 0 to PicSizeInMbs − CurrMbAddr, inclusive.
                                    if(data->mb_skip_run > (PicSizeInMbs - CurrMbAddr)){
                                        bs->stopFlags |= NB_AVC_BITSTREAM_STOP_VALUE_LOGIC_ERROR;
                                        //NBASSERT(data->mb_skip_run >= 0 && data->mb_skip_run <= (PicSizeInMbs - CurrMbAddr))
                                        r = FALSE;
                                    } else {
                                        prevMbSkipped = ( data->mb_skip_run > 0 );
                                        for( i = 0; i < data->mb_skip_run; i++ ){
                                            CurrMbAddr = NBAvc_NextMbAddress(CurrMbAddr, PicSizeInMbs, MbToSliceGroupMap);
                                        }
                                        if( data->mb_skip_run > 0 ){
                                            moreDataFlag = NBAvc_more_rbsp_data( bs );
                                        }
                                    }
								} else {
									//mb_skip_flag //2 ae(v)
									moreDataFlag = !data->mb_skip_flag;
								}
							}
							if( moreDataFlag ) {
								if( MbaffFrameFlag && ( CurrMbAddr % 2 == 0 || ( CurrMbAddr % 2 == 1 && prevMbSkipped ) ) ){ 
									//mb_field_decoding_flag; //2 u(1) | ae(v)
								}
								//macroblock_layer( );
							}
							if( !params->pic_parameter_set_rbsp->entropy_coding_mode_flag ){
								moreDataFlag = NBAvc_more_rbsp_data( bs );
							} else {
								if( !NBAvc_slice_type_is_I(slice_header->slice_type) && !NBAvc_slice_type_is_SI(slice_header->slice_type) ){
									prevMbSkipped = (BOOL)data->mb_skip_flag;
								}
								if( MbaffFrameFlag && CurrMbAddr % 2 == 0 ) {
									moreDataFlag = 1;
								} else {
									//end_of_slice_flag; //2 ae(v)
									moreDataFlag = !data->end_of_slice_flag;
								}
							}
							CurrMbAddr = NBAvc_NextMbAddress(CurrMbAddr, PicSizeInMbs, MbToSliceGroupMap);
						} while( moreDataFlag && !bs->stopFlags);
					}
					//Release
					{
						if(MbToSliceGroupMap != NULL){
							NBMemory_free(MbToSliceGroupMap);
							MbToSliceGroupMap = NULL;
						}
						if(mapUnitToSliceGroupMap != NULL){
							NBMemory_free(mapUnitToSliceGroupMap);
							mapUnitToSliceGroupMap = NULL;
						}
					}
					//
					if(bs->stopFlags){
						r = FALSE;
					}
				}
			}
		}
	}
	return r;
}

//slice_layer_without_partitioning_rbsp (nal_unit_type = 1, 5 or 19; [Coded slice of a non-IDR picture], [Coded slice of an IDR picture] or [Coded slice of an auxiliary coded picture without partitioning])

void* NBAvc_slice_layer_without_partitioning_rbsp_createSt(void);
void NBAvc_slice_layer_without_partitioning_rbsp_releaseSt(void* obj);
BOOL NBAvc_slice_layer_without_partitioning_rbsp_loadBits(STNBAvcBitstream* bs, const STNBAvcStateParams* params, void* data, const BOOL picDescBaseOnly);

static const STNBAvcNaluMap NBAvcMap_slice_layer_without_partitioning_rbsp = {
	"slice_layer_without_partitioning_rbsp"
	//Methods
	, NBAvc_slice_layer_without_partitioning_rbsp_createSt
	, NBAvc_slice_layer_without_partitioning_rbsp_releaseSt
	, NBAvc_slice_layer_without_partitioning_rbsp_loadBits
};

void* NBAvc_slice_layer_without_partitioning_rbsp_createSt(void){
	STNBAvc_slice_layer_without_partitioning_rbsp* r = NBMemory_allocType(STNBAvc_slice_layer_without_partitioning_rbsp);
	NBMemory_setZeroSt(*r, STNBAvc_slice_layer_without_partitioning_rbsp);
	return r;
}

void NBAvc_slice_layer_without_partitioning_rbsp_releaseSt(void* pObj){
	STNBAvc_slice_layer_without_partitioning_rbsp* obj = (STNBAvc_slice_layer_without_partitioning_rbsp*)pObj;
	if(obj != NULL){
		if(obj->slice_header != NULL){
			NBAvc_slice_header_releaseSt(obj->slice_header);
			obj->slice_header = NULL;
		}
		if(obj->slice_data != NULL){
			NBAvc_slice_data_releaseSt(obj->slice_data);
			obj->slice_data = NULL;
		}
		NBMemory_free(obj);
		obj = NULL;
	}
}

BOOL NBAvc_slice_layer_without_partitioning_rbsp_loadBits(STNBAvcBitstream* bs, const STNBAvcStateParams* params, void* pData, const BOOL picDescBaseOnly){
	BOOL r = FALSE;
	if(bs != NULL && !bs->stopFlags && pData != NULL){
		STNBAvc_slice_layer_without_partitioning_rbsp* data = (STNBAvc_slice_layer_without_partitioning_rbsp*)pData;
		if(params != NULL){
			r = TRUE;
			{
				if(data->slice_header == NULL){
					data->slice_header = NBAvc_slice_header_createSt();
				}
				if(data->slice_header == NULL){
					PRINTF_ERROR("NBAvc_slice_layer_without_partitioning_rbsp_loadBits, NBAvc_slice_header_createSt failed.\n");
					r = FALSE;
				} else if(!NBAvc_slice_header_loadBits(bs, params, data->slice_header, picDescBaseOnly)){
					PRINTF_ERROR("NBAvc_slice_layer_without_partitioning_rbsp_loadBits, NBAvc_slice_header_loadBits failed.\n");
					r = FALSE;
				}
			}
			//stop after 'slice_header' (if picDescBaseOnly)
			if(!picDescBaseOnly){
				//slice_data( )
				if(r){
					if(data->slice_data == NULL){
						data->slice_data = NBAvc_slice_data_createSt();
					}
					if(data->slice_data == NULL){
						PRINTF_ERROR("NBAvc_slice_layer_without_partitioning_rbsp_loadBits, NBAvc_slice_data_createSt failed.\n");
						r = FALSE;
					} else if(!NBAvc_slice_data_loadBits(bs, params, data->slice_data, data->slice_header)){
						PRINTF_ERROR("NBAvc_slice_layer_without_partitioning_rbsp_loadBits, NBAvc_slice_data_loadBits failed.\n");
						r = FALSE;
					}
				}
				//
				if(r){
					if(!NBAvc_rbsp_trailing_bits_loadBits(bs, params, NULL, picDescBaseOnly)){ //rbsp_trailing_bits( )
						PRINTF_ERROR("NBAvc_slice_layer_without_partitioning_rbsp_loadBits, NBAvc_rbsp_trailing_bits_loadBits failed.\n");
						r = FALSE;
					}
				}
			}
			//
			if(bs->stopFlags){
				r = FALSE;
			}
		}
	}
	return r;
}

//Params

void NBAvcStateParams_init(STNBAvcStateParams* obj){
	NBMemory_setZeroSt(*obj, STNBAvcStateParams);
}

void NBAvcStateParams_release(STNBAvcStateParams* obj){
	if(obj != NULL){
		if(obj->seq_parameter_set_rbsp != NULL){
			NBAvc_seq_parameter_set_rbsp_releaseSt(obj->seq_parameter_set_rbsp);
			obj->seq_parameter_set_rbsp = NULL;
		}
		if(obj->pic_parameter_set_rbsp != NULL){
			NBAvc_pic_parameter_set_rbsp_releaseSt(obj->pic_parameter_set_rbsp);
			obj->pic_parameter_set_rbsp = NULL;
		}
		if(obj->slice_layer_without_partitioning_rbsp != NULL){
			NBAvc_slice_layer_without_partitioning_rbsp_releaseSt(obj->slice_layer_without_partitioning_rbsp);
			obj->slice_layer_without_partitioning_rbsp = NULL;
		}
	}
}

//ToDo: remove
/*void NBAvcStateParams_setPtr_seq_parameter_set_rbsp(STNBAvcStateParams* obj, struct STNBAvc_seq_parameter_set_rbsp_* seq_parameter_set_rbsp){
	if(obj->seq_parameter_set_rbsp != NULL){
		NBAvc_seq_parameter_set_rbsp_releaseSt(obj->seq_parameter_set_rbsp);
	}
	obj->seq_parameter_set_rbsp = seq_parameter_set_rbsp;
}*/

//ToDo: remove
/*void NBAvcStateParams_setPtr_pic_parameter_set_rbsp(STNBAvcStateParams* obj, struct STNBAvc_pic_parameter_set_rbsp_* pic_parameter_set_rbsp){
	if(obj->pic_parameter_set_rbsp != NULL){
		NBAvc_pic_parameter_set_rbsp_releaseSt(obj->pic_parameter_set_rbsp);
	}
	obj->pic_parameter_set_rbsp = pic_parameter_set_rbsp;
}*/

//Scaling list

void NBAvc_ScalingList4x4_concat(const STNBAvc_ScalingList4x4* obj, STNBString* dst){
	if(obj != NULL && dst != NULL){
		//Determine the max len per column
		UI8 iCol, iRow, iElem, colsMaxVal[4], colsMaxLen[4], val;
		for(iCol = 0; iCol < 4; iCol++){
			//Max value
			colsMaxVal[iCol] = 0;
			iElem = iCol; if(colsMaxVal[iCol] < obj->elems[iElem]) colsMaxVal[iCol] = obj->elems[iElem];
			iElem += 4; if(colsMaxVal[iCol] < obj->elems[iElem]) colsMaxVal[iCol] = obj->elems[iElem];
			iElem += 4; if(colsMaxVal[iCol] < obj->elems[iElem]) colsMaxVal[iCol] = obj->elems[iElem];
			iElem += 4; if(colsMaxVal[iCol] < obj->elems[iElem]) colsMaxVal[iCol] = obj->elems[iElem];
			//
			NBASSERT(iElem <= (sizeof(obj->elems) / sizeof(obj->elems[0])))
			//Max len
			colsMaxLen[iCol] = (colsMaxVal[iCol] < 10 ? 1 : colsMaxVal[iCol] < 100 ? 2 : 3);
		}
		//Concat
		NBString_concat(dst, "UseDefaultScalingMatrix4x4Flag = ");
		NBString_concat(dst, (obj->UseDefaultScalingMatrix4x4Flag ? "true" : "false"));
		NBString_concat(dst, "\n");
		for(iRow = 0; iRow < 4; iRow++){
			NBString_concatByte(dst, '|');
			NBString_concatByte(dst, ' ');
			for(iCol = 0; iCol < 4; iCol++){
				val = obj->elems[(iRow * 4) + iCol];
				switch(colsMaxLen[iCol]) {
					case 1:
						NBString_concatUI32(dst, val);
						break;
					case 2:
						if(val < 10) NBString_concatByte(dst, ' ');
						NBString_concatUI32(dst, val);
						break;
					case 3:
						if(val < 100) NBString_concatByte(dst, ' ');
						if(val < 10) NBString_concatByte(dst, ' ');
						NBString_concatUI32(dst, val);
						break;
					default:
						NBASSERT(FALSE)
						break;
				}
				NBString_concatByte(dst, ' ');
			}
			NBString_concatByte(dst, '|');
			NBString_concatByte(dst, '\n');
		}
	}
}

void NBAvc_ScalingList8x8_concat(const STNBAvc_ScalingList8x8* obj, STNBString* dst){
	if(obj != NULL && dst != NULL){
		//Determine the max len per column
		UI8 iCol, iRow, iElem, colsMaxVal[8], colsMaxLen[8], val;
		for(iCol = 0; iCol < 8; iCol++){
			//Max value
			colsMaxVal[iCol] = 0;
			iElem = iCol; if(colsMaxVal[iCol] < obj->elems[iElem]) colsMaxVal[iCol] = obj->elems[iElem];
			iElem += 8; if(colsMaxVal[iCol] < obj->elems[iElem]) colsMaxVal[iCol] = obj->elems[iElem];
			iElem += 8; if(colsMaxVal[iCol] < obj->elems[iElem]) colsMaxVal[iCol] = obj->elems[iElem];
			iElem += 8; if(colsMaxVal[iCol] < obj->elems[iElem]) colsMaxVal[iCol] = obj->elems[iElem];
			//
			iElem += 8; if(colsMaxVal[iCol] < obj->elems[iElem]) colsMaxVal[iCol] = obj->elems[iElem];
			iElem += 8; if(colsMaxVal[iCol] < obj->elems[iElem]) colsMaxVal[iCol] = obj->elems[iElem];
			iElem += 8; if(colsMaxVal[iCol] < obj->elems[iElem]) colsMaxVal[iCol] = obj->elems[iElem];
			iElem += 8; if(colsMaxVal[iCol] < obj->elems[iElem]) colsMaxVal[iCol] = obj->elems[iElem];
			//
			NBASSERT(iElem <= (sizeof(obj->elems) / sizeof(obj->elems[0])))
			//Max len
			colsMaxLen[iCol] = (colsMaxVal[iCol] < 10 ? 1 : colsMaxVal[iCol] < 100 ? 2 : 3);
		}
		//Concat
		NBString_concat(dst, "UseDefaultScalingMatrix8x8Flag = ");
		NBString_concat(dst, (obj->UseDefaultScalingMatrix8x8Flag ? "true" : "false"));
		NBString_concat(dst, "\n");
		//
		for(iRow = 0; iRow < 8; iRow++){
			NBString_concatByte(dst, '|');
			NBString_concatByte(dst, ' ');
			for(iCol = 0; iCol < 8; iCol++){
				val = obj->elems[(iRow * 8) + iCol];
				switch(colsMaxLen[iCol]) {
					case 1:
						NBString_concatUI32(dst, val);
						break;
					case 2:
						if(val < 10) NBString_concatByte(dst, ' ');
						NBString_concatUI32(dst, val);
						break;
					case 3:
						if(val < 100) NBString_concatByte(dst, ' ');
						if(val < 10) NBString_concatByte(dst, ' ');
						NBString_concatUI32(dst, val);
						break;
					default:
						NBASSERT(FALSE)
						break;
				}
				NBString_concatByte(dst, ' ');
			}
			NBString_concatByte(dst, '|');
			NBString_concatByte(dst, '\n');
		}
	}
}

//

const STNBAvcNaluMap* NBAvc_getNaluMap(const UI8 uid){
	const STNBAvcNaluMap* r = NULL;
	switch (uid) {
		case 1:
			r = &NBAvcMap_slice_layer_without_partitioning_rbsp;
			break;
		case 5:
			r = &NBAvcMap_slice_layer_without_partitioning_rbsp;
			break;
		case 7:
			r = &NBAvcMap_seq_parameter_set_rbsp;
			break;		
		case 8:
			r = &NBAvcMap_pic_parameter_set_rbsp;
			break;
		case 19:
			r = &NBAvcMap_slice_layer_without_partitioning_rbsp;
			break;
		default:
			break;
	}
	return r;
}

void NBAvc_tableUI32_concat(const UI32* tbl, const UI32 width, const UI32 height, STNBString* dst){
	if(tbl != NULL && width > 0 && height > 0 && dst != NULL){
		//Determine the max len per column
		const UI32 tblSz = width * height; NBASSERT(tblSz > 0)
		UI32 iCol, iRow, iElem, val, val2, valLen;
		UI32* colsMaxVal = NBMemory_allocTypes(UI32, width);
		UI32* colsMaxLen = NBMemory_allocTypes(UI32, width);
		for(iCol = 0; iCol < width; iCol++){
			//Max value
			val		= 0;
			iElem	= iCol;
			do {
				if(val < tbl[iElem]){
					val = tbl[iElem];
				}
				iElem += width;
			} while(iElem < tblSz);
			//Max len
			valLen	= 1;
			val2	= val / 10;
			while(val2 > 0){
				val2 /= 10;
				valLen++;
			}
			//Keep values
			colsMaxVal[iCol] = val;
			colsMaxLen[iCol] = valLen;
		}
		//Concat
		for(iRow = 0; iRow < height; iRow++){
			NBString_concatByte(dst, '|');
			NBString_concatByte(dst, ' ');
			for(iCol = 0; iCol < width; iCol++){
				val		= tbl[(iRow * width) + iCol];
				//Len
				valLen	= 1;
				val2	= val / 10;
				while(val2 > 0){
					val2 /= 10;
					valLen++;
				}
				//Spaces
				NBASSERT(colsMaxLen[iCol] >= valLen)
				if(colsMaxLen[iCol] > valLen){
					NBString_concatRepeatedByte(dst, ' ', colsMaxLen[iCol] - valLen);
				}
				//Value
				NBString_concatUI32(dst, val);
				//
				NBString_concatByte(dst, ' ');
			}
			NBString_concatByte(dst, '|');
			NBString_concatByte(dst, '\n');
		}
		NBMemory_free(colsMaxVal);
		NBMemory_free(colsMaxLen);
	}
}
