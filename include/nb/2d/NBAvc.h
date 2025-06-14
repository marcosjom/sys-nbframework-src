//
//  NBAvc.h
//  nbframework
//
//  Created by Marcos Ortega on 9/21/20.
//

#ifndef NBAvc_h
#define NBAvc_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBArray.h"
#include "nb/2d/NBAvcBitstream.h"

//NALUnit defs

typedef struct STNBAvcNaluDef_ {
	UI8				type;
	const char*		group;	//"VCL", "non-VCL", "Stap-A", "Stap-B", "MTAP16", "MTAP24", "FU-A", "FU-B", ... 
	const char*		desc;
} STNBAvcNaluDef;

const STNBAvcNaluDef* NBAvc_getNaluDef(const UI8 type);

//nalu header (first byte)

typedef struct STNBAvcNaluHdr_ {
	BOOL	forbiddenZeroBit;	//f(1)
	UI8		refIdc;				//u(2)
	UI8		type;				//u(5)
} STNBAvcNaluHdr;

STNBAvcNaluHdr NBAvc_getNaluHdr(const BYTE firstByte);

//Structs predefinition

struct STNBAvc_rbsp_trailing_bits_;
struct STNBAvc_hrd_parameters_;
struct STNBAvc_vui_parameters_;
struct STNBAvc_ScalingList4x4_;
struct STNBAvc_ScalingList8x8_;
struct STNBAvc_seq_parameter_set_data_;
struct STNBAvc_seq_parameter_set_rbsp_;
struct STNBAvc_pic_parameter_set_rbsp_;
struct STNBAvc_ref_pic_list_mvc_modification_;
struct STNBAvc_ref_pic_list_modification_;
struct STNBAvc_pred_weight_table_;
struct STNBAvc_dec_ref_pic_marking_;
struct STNBAvc_slice_header_;
struct STNBAvc_slice_data_;
struct STNBAvc_slice_layer_without_partitioning_rbsp_;

//Params

typedef struct STNBAvcStateParams_ {
	STNBAvcNaluHdr hdr;				//f(1)+u(2)+u(5)
	BOOL	svc_extension_flag;		//u(1)
	BOOL	avc_3d_extension_flag;	//u(1)
	UI32	NumBytesInRBSP;
	UI32	nalUnitHeaderBytes;
	struct STNBAvc_seq_parameter_set_rbsp_*	seq_parameter_set_rbsp;
	struct STNBAvc_pic_parameter_set_rbsp_* pic_parameter_set_rbsp;
	struct STNBAvc_slice_layer_without_partitioning_rbsp_* slice_layer_without_partitioning_rbsp;
} STNBAvcStateParams;

void NBAvcStateParams_init(STNBAvcStateParams* obj);
void NBAvcStateParams_release(STNBAvcStateParams* obj);
//ToDo: remove
//void NBAvcStateParams_setPtr_seq_parameter_set_rbsp(STNBAvcStateParams* obj, struct STNBAvc_seq_parameter_set_rbsp_* seq_parameter_set_rbsp);
//void NBAvcStateParams_setPtr_pic_parameter_set_rbsp(STNBAvcStateParams* obj, struct STNBAvc_pic_parameter_set_rbsp_* pic_parameter_set_rbsp);

//NAL-Unit definition

typedef struct STNBAvcDefNalu_ {
	UI8				uid;
	const char*		name;
	const char*		func;
} STNBAvcDefNalu;

const STNBAvcDefNalu* NBAvc_getDefNalu(const UI8 uid);

//Maps

typedef void* (*NBAvctructCreateStMethod)(void);	//Create a new structure
typedef void (*NBAvctructReleaseStMethod)(void* obj); //Release structure
typedef BOOL (*NBAvcDefLoadBitsMethod)(STNBAvcBitstream* bs, const STNBAvcStateParams* params, void* obj, const BOOL picDescBaseOnly); //Load bits

typedef struct STNBAvcNaluMap_ {
	const char*					name;
	//Methods
	NBAvctructCreateStMethod	createSt;
	NBAvctructReleaseStMethod	releaseSt;
	NBAvcDefLoadBitsMethod		loadBits;
} STNBAvcNaluMap;

const STNBAvcNaluMap* NBAvc_getNaluMap(const UI8 uid);

//aspect_ratio_idc

typedef enum ENAvc_aspect_ratio_idc_ {
	ENAvc_aspect_ratio_idc_Unspecified = 0, //Unspecified
	ENAvc_aspect_ratio_idc_1_1_1_square,//1:1(“square”)
										// 1280x720 16:9 frame without overscan
										// 1920x1080 16:9 frame without overscan (cropped from 1920x1088)
										// 640x480 4:3 frame without overscan
	ENAvc_aspect_ratio_idc_2_12_11,		//12:11 720x576 4:3 frame with horizontal overscan
										// 352x288 4:3 frame without overscan
	ENAvc_aspect_ratio_idc_3_10_11,		//10:11 720x480 4:3 frame with horizontal overscan
										// 352x240 4:3 frame without overscan
	ENAvc_aspect_ratio_idc_4_16_11,		//16:11 720x576 16:9 frame with horizontal overscan
										// 540x576 4:3 frame with horizontal overscan
	ENAvc_aspect_ratio_idc_5_40_33,		//40:33 720x480 16:9 frame with horizontal overscan
										// 540x480 4:3 frame with horizontal overscan
	ENAvc_aspect_ratio_idc_6_24_11,		//24:11 352x576 4:3 frame without overscan
										// 540x576 16:9 frame with horizontal overscan
	ENAvc_aspect_ratio_idc_7_20_11,		//20:11 352x480 4:3 frame without overscan
										// 480x480 16:9 frame with horizontal overscan
	ENAvc_aspect_ratio_idc_8_32_11,		//32:11 352x576 16:9 frame without overscan
	ENAvc_aspect_ratio_idc_9_80_33,		//80:33 352x480 16:9 frame without overscan
	ENAvc_aspect_ratio_idc_10_18_11,	//18:11 480x576 4:3 frame with horizontal overscan
	ENAvc_aspect_ratio_idc_11_15_11,	//15:11 480x480 4:3 frame with horizontal overscan
	ENAvc_aspect_ratio_idc_12_64_33,	//64:33 540x576 16:9 frame with horizontal overscan
	ENAvc_aspect_ratio_idc_13_160_99,	//160:99 540x480 16:9 frame with horizontal overscan
	//14..254 Reserved
	//Extended_SAR
	ENAvc_aspect_ratio_idc_Extended_SAR = 255, //Extended_SAR 
} ENAvc_aspect_ratio_idc;

typedef struct STNBAvcAspectRatioIdcDef_ {
	const char*	aspect;
	UI8			aspectW;
	UI8			aspectH;
	const char*	desc;
} STNBAvcAspectRatioIdcDef;

const STNBAvcAspectRatioIdcDef* NBAvc_getAspectRatioIdcDef(const UI8 idc); //ENAvc_aspect_ratio_idc

//slice type

typedef enum ENAvc_slice_type_ {
	ENAvc_slice_type_0_P = 0,	//(P slice, Consists of P-macroblocks (each macro block is predicted using one reference frame) and / or I-macroblocks.)
	ENAvc_slice_type_1_B,		//(B slice, Consists of B-macroblocks (each macroblock is predicted using one or two reference frames) and / or I-macroblocks.)
	ENAvc_slice_type_2_I,		//(I slice, Contains only I-macroblocks. Each macroblock is predicted from previously coded blocks of the same slice.)
	ENAvc_slice_type_3_SP,		//(SP slice, Consists of P and / or I-macroblocks and lets you switch between encoded streams.)
	ENAvc_slice_type_4_SI,		//(SI slice, It consists of a special type of SI-macroblocks and lets you switch between encoded streams.)
	//
	ENAvc_slice_type_5_P,		//(P slice, Consists of P-macroblocks (each macro block is predicted using one reference frame) and / or I-macroblocks.)
	ENAvc_slice_type_6_B,		//(B slice, Consists of B-macroblocks (each macroblock is predicted using one or two reference frames) and / or I-macroblocks.)
	ENAvc_slice_type_7_I,		//(I slice, Contains only I-macroblocks. Each macroblock is predicted from previously coded blocks of the same slice.)
	ENAvc_slice_type_8_SP,		//(SP slice, Consists of P and / or I-macroblocks and lets you switch between encoded streams.)
	ENAvc_slice_type_9_SI,		//(SI slice, It consists of a special type of SI-macroblocks and lets you switch between encoded streams.)
	//Count
	ENAvc_slice_type_Count
} ENAvc_slice_type;

#define NBAvc_slice_type_is_P(TYPE)		(TYPE == ENAvc_slice_type_0_P || TYPE == ENAvc_slice_type_5_P)
#define NBAvc_slice_type_is_B(TYPE)		(TYPE == ENAvc_slice_type_1_B || TYPE == ENAvc_slice_type_6_B)
#define NBAvc_slice_type_is_I(TYPE)		(TYPE == ENAvc_slice_type_2_I || TYPE == ENAvc_slice_type_7_I)
#define NBAvc_slice_type_is_SP(TYPE)	(TYPE == ENAvc_slice_type_3_SP || TYPE == ENAvc_slice_type_8_SP)
#define NBAvc_slice_type_is_SI(TYPE)	(TYPE == ENAvc_slice_type_4_SI || TYPE == ENAvc_slice_type_9_SI)

typedef struct STNBAvcSliceTypeDef_ {
	UI8	type;
	const char* name;
	const char* desc;
} STNBAvcSliceTypeDef;

const STNBAvcSliceTypeDef* NBAvc_getSliceTypeDef(const UI8 type); //ENAvc_slice_type

//video_format

typedef enum ENAvc_video_format_ {
	ENAvc_video_format_0_Component = 0,	//Component
	ENAvc_video_format_1_PAL,			//PAL
	ENAvc_video_format_2_NTSC,			//NTSC
	ENAvc_video_format_3_SECAM,			//SECAM
	ENAvc_video_format_4_MAC,			//MAC
	ENAvc_video_format_5_Unspecified,	//Unspecified video format
	ENAvc_video_format_6,				//Reserved
	ENAvc_video_format_7,				//Reserved
	//Count
	ENAvc_video_format_Count
} ENAvc_video_format;

typedef struct STNBAvcVideoFormatDef_ {
	UI8 value;
	const char* name;
} STNBAvcVideoFormatDef;

const STNBAvcVideoFormatDef* NBAvc_getVideoFormatDef(const UI8 videoFormat); //ENAvc_video_format

//rbsp_trailing_bits

typedef struct STNBAvc_rbsp_trailing_bits_ {
	UI8 trailing_bits;
	//UI8 rbsp_stop_one_bit; // 0 f(1) //equal to 1
	//while( !byte_aligned( ) ){
	//	UI8 rbsp_alignment_zero_bit; //0 f(1) //equal to 0
	//}
} STNBAvc_rbsp_trailing_bits;

//hrd_parameters

typedef struct STNBAvc_hrd_parameters_ {
	UI64 cpb_cnt_minus1; // 0 ue(v)
	UI8 bit_rate_scale; //0 u(4)
	UI8 cpb_size_scale; //0 u(4)
	//for( SchedSelIdx = 0; SchedSelIdx <= cpb_cnt_minus1; SchedSelIdx++ ) {
		UI64* bit_rate_value_minus1; //[ SchedSelIdx ] 0 ue(v)
		UI16 bit_rate_value_minus1Use;
		UI16 bit_rate_value_minus1Sz;
		UI64* cpb_size_value_minus1; //[ SchedSelIdx ] 0 ue(v)
		UI16 cpb_size_value_minus1Use;
		UI16 cpb_size_value_minus1Sz;
		BOOL* cbr_flag; //[ SchedSelIdx ] 0 u(1)
		UI16 cbr_flagUse;
		UI16 cbr_flagSz;
	//}
	UI8 initial_cpb_removal_delay_length_minus1; //0 u(5)
	UI8 cpb_removal_delay_length_minus1; //0 u(5)
	UI8 dpb_output_delay_length_minus1; //0 u(5)
	UI8 time_offset_length; //0 u(5)
} STNBAvc_hrd_parameters;

//vui_parameters

typedef struct STNBAvc_vui_parameters_ {
	BOOL aspect_ratio_info_present_flag; //0 u(1)
	//if( aspect_ratio_info_present_flag ) {
		UI8 aspect_ratio_idc; //0 u(8)
		//if( aspect_ratio_idc == Extended_SAR ) {
			UI16 sar_width; // 0 u(16)
			UI16 sar_height; // 0 u(16)
		//}
	//}
	BOOL overscan_info_present_flag; //0 u(1)
	//if( overscan_info_present_flag ){
		BOOL overscan_appropriate_flag; //0 u(1)
	//}
	BOOL video_signal_type_present_flag; //0 u(1)
	//if( video_signal_type_present_flag ) {
		UI8 video_format; //0 u(3)
		BOOL video_full_range_flag; //0 u(1)
		BOOL colour_description_present_flag; //0 u(1)
		//if( colour_description_present_flag ) {
			UI8 colour_primaries; //0 u(8)
			UI8 transfer_characteristics; //0 u(8)
			UI8 matrix_coefficients; //0 u(8)
		//}
	//}
	BOOL chroma_loc_info_present_flag; //0 u(1)
	//if( chroma_loc_info_present_flag ) {
		UI64 chroma_sample_loc_type_top_field; //0 ue(v)
		UI64 chroma_sample_loc_type_bottom_field; //0 ue(v)
	//} 
	BOOL timing_info_present_flag; //0 u(1)
	//if( timing_info_present_flag ) {
		UI32 num_units_in_tick; //0 u(32)
		UI32 time_scale; //0 u(32)
		BOOL fixed_frame_rate_flag; //0 u(1)
	//}
	BOOL nal_hrd_parameters_present_flag; //0 u(1)
	//if( nal_hrd_parameters_present_flag ){
		//hrd_parameters( )
		STNBAvc_hrd_parameters* nal_hrd_parameters;
	//}
	BOOL vcl_hrd_parameters_present_flag; //0 u(1)
	//if( vcl_hrd_parameters_present_flag ){
		//hrd_parameters( )
		STNBAvc_hrd_parameters* vcl_hrd_parameters;
	//}
	//if( nal_hrd_parameters_present_flag || vcl_hrd_parameters_present_flag ){
		BOOL low_delay_hrd_flag; //0 u(1)
	//}
	BOOL pic_struct_present_flag; //0 u(1)
	BOOL bitstream_restriction_flag; //0 u(1)
	//if( bitstream_restriction_flag ) {
		BOOL motion_vectors_over_pic_boundaries_flag; //0 u(1)
		UI64 max_bytes_per_pic_denom; //0 ue(v)
		UI64 max_bits_per_mb_denom; //0 ue(v)
		UI64 log2_max_mv_length_horizontal; //0 ue(v)
		UI64 log2_max_mv_length_vertical; //0 ue(v)
		UI64 num_reorder_frames; //0 ue(v)
		UI64 max_dec_frame_buffering; //0 ue(v)
	//}
} STNBAvc_vui_parameters;

//Scaling list

typedef struct STNBAvc_ScalingList4x4_ {
	UI8 elems[16];
	BOOL UseDefaultScalingMatrix4x4Flag;
} STNBAvc_ScalingList4x4;

void NBAvc_ScalingList4x4_concat(const STNBAvc_ScalingList4x4* obj, STNBString* dst);

typedef struct STNBAvc_ScalingList8x8_ {
	UI8 elems[64];
	BOOL UseDefaultScalingMatrix8x8Flag;
} STNBAvc_ScalingList8x8;

void NBAvc_ScalingList8x8_concat(const STNBAvc_ScalingList8x8* obj, STNBString* dst);

//seq_parameter_set_rbsp (nal_unit_type = 7)

typedef struct STNBAvc_seq_parameter_set_data_ {
	UI8 profile_idc; //0 u(8)
	BOOL constraint_set0_flag; //0 u(1)
	BOOL constraint_set1_flag; //0 u(1)
	BOOL constraint_set2_flag; //0 u(1)
	BOOL constraint_set3_flag; //0 u(1)
	BOOL constraint_set4_flag; //0 u(1)
	BOOL constraint_set5_flag; //0 u(1)
	UI8	reserved_zero_2bits; //0 u(2)
	UI8 level_idc; //0 u(8)
	UI64 seq_parameter_set_id; //0 ue(v)
	//if( profile_idc == 100 || profile_idc == 110 ||
	//	profile_idc == 122 || profile_idc == 244 || profile_idc == 44 || 
	//	profile_idc == 83 || profile_idc == 86 || profile_idc == 118 ||
	//	profile_idc == 128 || profile_idc == 138 || profile_idc == 139 ||
	//	profile_idc == 134 || profile_idc == 134 ){
		UI64 chroma_format_idc; //0 ue(v) //expected 0 to 3 inclusive
		//if( chroma_format_idc == 3 ){
			BOOL separate_colour_plane_flag; //0 u(1)
		//}
		UI64 bit_depth_luma_minus8; //ue(v)
		UI64 bit_depth_chroma_minus8; //ue(v)
		BOOL qpprime_y_zero_transform_bypass_flag; //u(1)
		BOOL seq_scaling_matrix_present_flag; //u(1)
		//if( seq_scaling_matrix_present_flag ){
			//for(i = 0; i < ((chroma_format_idc != 3) ? 8 : 12 ); i++){
				BOOL* seq_scaling_list_present_flag; //[ i ] u(1)
				UI8 seq_scaling_list_present_flagUse;
				UI8 seq_scaling_list_present_flagSz;
				STNBAvc_ScalingList4x4* ScalingList4x4;
				UI8						ScalingList4x4Use;
				UI8						ScalingList4x4Sz;
				STNBAvc_ScalingList8x8* ScalingList8x8;
				UI8						ScalingList8x8Use;
				UI8						ScalingList8x8Sz;
				//if( seq_scaling_list_present_flag[ i ] ){
					//if( i < 6 ){
						//scaling_list( ScalingList4x4[ i ], 16, UseDefaultScalingMatrix4x4Flag[ i ])
					//} else {
						//scaling_list( ScalingList8x8[ i − 6 ], 64, UseDefaultScalingMatrix8x8Flag[ i − 6 ] )
					//}
				//}
			//}
		//}
	//}
	UI64 log2_max_frame_num_minus4; //0 ue(v) //shall be in the range of 0 to 12, inclusive
	UI64 pic_order_cnt_type; //0 ue(v) //shall be in the range of 0 to 2, inclusive
	//if( pic_order_cnt_type == 0 ){
		UI64 log2_max_pic_order_cnt_lsb_minus4; //0 ue(v) //shall be in the range of 0 to 12, inclusive.
	//} else if( pic_order_cnt_type == 1 ) {
		BOOL delta_pic_order_always_zero_flag; //0 u(1)
		SI64 offset_for_non_ref_pic; //0 se(v)
		SI64 offset_for_top_to_bottom_field; //0 se(v)
		UI64 num_ref_frames_in_pic_order_cnt_cycle; //0 ue(v)
		//for( i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ ){
			SI64* offset_for_ref_frame;//[ i ] 0 se(v)
			UI16 offset_for_ref_frameUse;
			UI16 offset_for_ref_frameSz;
		//}
	//}
	UI64 max_num_ref_frames; //0 ue(v)
	BOOL gaps_in_frame_num_value_allowed_flag; //0 u(1)
	UI64 pic_width_in_mbs_minus1; //0 ue(v)
	UI64 pic_height_in_map_units_minus1; //0 ue(v)
	BOOL frame_mbs_only_flag; //0 u(1)
	//if( !frame_mbs_only_flag ){
		BOOL mb_adaptive_frame_field_flag; //0 u(1)
	//}
	BOOL direct_8x8_inference_flag; //0 u(1)
	BOOL frame_cropping_flag; //0 u(1)
	//if( frame_cropping_flag ) {
		UI64 frame_crop_left_offset; //0 ue(v)
		UI64 frame_crop_right_offset; //0 ue(v)
		UI64 frame_crop_top_offset; //0 ue(v)
		UI64 frame_crop_bottom_offset; //0 ue(v)
	//}
	BOOL vui_parameters_present_flag; //0 u(1)
	//if( vui_parameters_present_flag ){
		STNBAvc_vui_parameters*  vui_parameters; //	vui_parameters( ) 0
	//}
} STNBAvc_seq_parameter_set_data;

typedef struct STNBAvc_seq_parameter_set_rbsp_ {
	STNBAvc_seq_parameter_set_data*	data;
	STNBAvc_rbsp_trailing_bits*		rbsp_trailing_bits; //rbsp_trailing_bits( )
} STNBAvc_seq_parameter_set_rbsp;

//pic_parameter_set_rbsp (nal_unit_type = 8)

typedef struct STNBAvc_pic_parameter_set_rbsp_ {
	UI64 pic_parameter_set_id; //1 ue(v)
	UI64 seq_parameter_set_id; // 1 ue(v)
	BOOL entropy_coding_mode_flag; // 1 u(1)
	BOOL bottom_field_pic_order_in_frame_present_flag; // 1 u(1)
	UI64 num_slice_groups_minus1; // 1 ue(v)
	//if( num_slice_groups_minus1 > 0 ) {
		UI64 slice_group_map_type; // 1 ue(v)
		//if( slice_group_map_type == 0 ){
			//for( iGroup = 0; iGroup <= num_slice_groups_minus1; iGroup++ ){
				UI64* run_length_minus1; //[ iGroup ] 1 ue(v)
				UI16 run_length_minus1Use;
				UI16 run_length_minus1Sz;
			//}
		//} else if( slice_group_map_type == 2 ){
			//for( iGroup = 0; iGroup < num_slice_groups_minus1; iGroup++ ) {
				UI64* top_left; //[ iGroup ] 1 ue(v)
				UI16  top_leftUse;
				UI16  top_leftSz;
				UI64* bottom_right; //[ iGroup ] 1 ue(v)
				UI16  bottom_rightUse;
				UI16  bottom_rightSz;
			//}
		//} else if( slice_group_map_type == 3 ||
		//	slice_group_map_type == 4 ||
		//	slice_group_map_type == 5 ) {
			BOOL slice_group_change_direction_flag; // 1 u(1)
			UI64 slice_group_change_rate_minus1; // 1 ue(v)
		//} else if( slice_group_map_type == 6 ) {
			UI64 pic_size_in_map_units_minus1; // 1 ue(v)
			//for( i = 0; i <= pic_size_in_map_units_minus1; i++ ){
				UI32* slice_group_id; //[ i ] 1 u(v) //#bits = ceil( Log2( num_slice_groups_minus1 + 1 ) )
				UI16 slice_group_idUse;
				UI16 slice_group_idSz;
			//}
		//}
	//}
	UI64 num_ref_idx_l0_active_minus1; // 1 ue(v)
	UI64 num_ref_idx_l1_active_minus1; // 1 ue(v)
	BOOL weighted_pred_flag; // 1 u(1)
	UI8 weighted_bipred_idc; // 1 u(2)
	SI64 pic_init_qp_minus26; // relative to 26 // 1 se(v)
	SI64 pic_init_qs_minus26; // relative to 26 // 1 se(v)
	SI64 chroma_qp_index_offset; // 1 se(v)
	BOOL deblocking_filter_control_present_flag; // 1 u(1)
	BOOL constrained_intra_pred_flag; // 1 u(1)
	BOOL redundant_pic_cnt_present_flag; // 1 u(1)
	//if( more_rbsp_data( ) ) {
		BOOL transform_8x8_mode_flag ; //1 u(1)
		BOOL pic_scaling_matrix_present_flag; //1 u(1)
		//if( pic_scaling_matrix_present_flag ){
			//for( i = 0; i < 6 + ( ( chroma_format_idc != 3 ) ? 2 : 6 ) * transform_8x8_mode_flag; i++ ) {
				BOOL* pic_scaling_list_present_flag; //[ i ]; //1 u(1)
				UI16 pic_scaling_list_present_flagUse;
				UI16 pic_scaling_list_present_flagSz;
				STNBAvc_ScalingList4x4* ScalingList4x4;
				UI32					ScalingList4x4Use;
				UI32					ScalingList4x4Sz;
				STNBAvc_ScalingList8x8* ScalingList8x8;
				UI32					ScalingList8x8Use;
				UI32					ScalingList8x8Sz;
				//if( pic_scaling_list_present_flag[ i ] ){
					//if( i < 6 ){
						//scaling_list( ScalingList4x4[ i ], 16, UseDefaultScalingMatrix4x4Flag[ i ] )
					//} else {
						//scaling_list( ScalingList8x8[ i − 6 ], 64, UseDefaultScalingMatrix8x8Flag[ i − 6 ] )
					//}
				//}
			//}
		//}
		SI64 second_chroma_qp_index_offset; //1 se(v)
	//}
	STNBAvc_rbsp_trailing_bits* rbsp_trailing_bits; //rbsp_trailing_bits( ) 1 
} STNBAvc_pic_parameter_set_rbsp;

//slice

typedef struct STNBAvc_ref_pic_list_mvc_modification_ {
	//if((slice_type % 5) != 2 && (slice_type % 5) != 4){
		UI8 ref_pic_list_modification_flag_l0; //2 u(1)
		//if( ref_pic_list_modification_flag_l0 ){
			//do {
				UI64 modification_of_pic_nums_idc; //2 ue(v)
				//if( modification_of_pic_nums_idc == 0 || modification_of_pic_nums_idc == 1 ){
					UI64 abs_diff_pic_num_minus1; //2 ue(v)
				//} else if( modification_of_pic_nums_idc == 2 ){
					UI64 long_term_pic_num; //2 ue(v)
				//} else if ( modification_of_pic_nums_idc == 4 || modification_of_pic_nums_idc == 5 ){
					UI64 abs_diff_view_idx_minus1; //2 ue(v)
				//}
			//} while( modification_of_pic_nums_idc != 3 );
		//}
	//}
	//if((slice_type % 5) == 1){
		UI8 ref_pic_list_modification_flag_l1; //2 u(1)
		//if( ref_pic_list_modification_flag_l1 ){
			//do {
				//UI64 modification_of_pic_nums_idc; //2 ue(v)
				//if( modification_of_pic_nums_idc == 0 || modification_of_pic_nums_idc == 1 ){
					//UI64 abs_diff_pic_num_minus1; //2 ue(v)
				//} else if( modification_of_pic_nums_idc == 2 ){
					//UI64 long_term_pic_num; //2 ue(v)
				//} else if ( modification_of_pic_nums_idc == 4 || modification_of_pic_nums_idc == 5 ){
					//UI64 abs_diff_view_idx_minus1; //2 ue(v)
				//}
			//} while( modification_of_pic_nums_idc != 3 )
		//}
	//}
} STNBAvc_ref_pic_list_mvc_modification;

typedef struct STNBAvc_ref_pic_list_modification_ {
	//if((slice_type % 5) != 2 && (slice_type % 5) != 4){
		UI8 ref_pic_list_modification_flag_l0; //2 u(1)
		//if( ref_pic_list_modification_flag_l0 ){
			//do {
				UI64 modification_of_pic_nums_idc; //2 ue(v)
				//if( modification_of_pic_nums_idc == 0 || modification_of_pic_nums_idc == 1 ){
					UI64 abs_diff_pic_num_minus1; //2 ue(v)
				//} else if( modification_of_pic_nums_idc == 2 ){
					UI64 long_term_pic_num; //2 ue(v)
				//}
			//} while( modification_of_pic_nums_idc != 3 );
		//}
	//}
	//if((slice_type % 5) == 1){
		UI8 ref_pic_list_modification_flag_l1; //2 u(1)
		//if( ref_pic_list_modification_flag_l1 ){
			//do {
				//UI64 modification_of_pic_nums_idc; //2 ue(v)
				//if( modification_of_pic_nums_idc == 0 || modification_of_pic_nums_idc == 1 ){
					//UI64 abs_diff_pic_num_minus1; //2 ue(v)
				//} else if( modification_of_pic_nums_idc == 2 ){
					//UI64 long_term_pic_num; //2 ue(v)
				//}
			//} while( modification_of_pic_nums_idc != 3 );
		//}
	//}
} STNBAvc_ref_pic_list_modification;

typedef struct STNBAvc_pred_weight_table_ {
	UI64 luma_log2_weight_denom; //2 ue(v)
	//if( ChromaArrayType != 0 ){
		UI64 chroma_log2_weight_denom; //2 ue(v)
	//}
	//for( i = 0; i <= num_ref_idx_l0_active_minus1; i++ ) {
		BOOL luma_weight_l0_flag; //2 u(1)
		UI16 lumal0Use;
		UI16 lumal0Sz;
		//if( luma_weight_l0_flag ) {
			SI64* luma_weight_l0; //[ i ] //2 se(v)
			SI64* luma_offset_l0; //[ i ] //2 se(v)
		//}
		//if ( ChromaArrayType != 0 ) {
			BOOL chroma_weight_l0_flag; //2 u(1)
			UI16 chromal0Use;
			UI16 chromal0Sz;
			//if( chroma_weight_l0_flag ) {
				//for( j =0; j < 2; j++ ) {
					SI64* chroma_weight_l0; //[ i ][ j ]; //2 se(v)
					SI64* chroma_offset_l0; //[ i ][ j ]; //2 se(v)
				//}
			//}
		//}
	//}
	//if((slice_type % 5) == 1){
		//for( i = 0; i <= num_ref_idx_l1_active_minus1; i++ ) {
			BOOL luma_weight_l1_flag; //2 u(1)
			UI16 lumal1Use;
			UI16 lumal1Sz;
			//if( luma_weight_l1_flag ) {
				SI64* luma_weight_l1; //[ i ]; //2 se(v)
				SI64* luma_offset_l1; //[ i ]; //2 se(v)
			//}
			//if( ChromaArrayType != 0 ) {
				BOOL chroma_weight_l1_flag; //2 u(1)
				UI16 chromal1Use;
				UI16 chromal1Sz;
				//if( chroma_weight_l1_flag ){
					//for( j = 0; j < 2; j++ ) {
						SI64* chroma_weight_l1; //[ i ][ j ] //2 se(v)
						SI64* chroma_offset_l1; //[ i ][ j ] //2 se(v)
					//}
				//}
			//}
		//}
	//}
} STNBAvc_pred_weight_table;

void NBAvc_pred_weight_concat(const UI32 lumaSz, const SI64* luma_weight, const SI64* luma_offset, const SI64* chroma_weight, const SI64* chroma_offset, STNBString* dst);

typedef struct STNBAvc_dec_ref_pic_marking_ {
	//if( IdrPicFlag ) {
		BOOL no_output_of_prior_pics_flag; //2|5 u(1)
		BOOL long_term_reference_flag; //2|5 u(1)
	//} else {
		BOOL adaptive_ref_pic_marking_mode_flag; //2|5 u(1)
		//if( adaptive_ref_pic_marking_mode_flag ){
			//do {
				UI64 memory_management_control_operation; //2|5 ue(v)
				//if(memory_management_control_operation == 1 || memory_management_control_operation == 3){
					UI64 difference_of_pic_nums_minus1; //2|5 ue(v)
				//} 
				//if(memory_management_control_operation == 2 ){
					UI64 long_term_pic_num; //2|5 ue(v)
				//}
				//if(memory_management_control_operation == 3 || memory_management_control_operation == 6){
					UI64 long_term_frame_idx; //2|5 ue(v)
				//}
				//if(memory_management_control_operation == 4){
					UI64 max_long_term_frame_idx_plus1; //2|5 ue(v)
				//}
			//} while( memory_management_control_operation != 0 );
		//}
	//}
} STNBAvc_dec_ref_pic_marking;

typedef struct STNBAvc_slice_header_ {
	UI64 first_mb_in_slice;	//2 ue(v)
	UI64 slice_type; //2 ue(v)
	UI64 pic_parameter_set_id; //2 ue(v)
	//if( separate_colour_plane_flag == 1 ){
		UI8 colour_plane_id; //2 u(2)
	//}
	UI32 frame_num; //2 u(v)
	//if( !frame_mbs_only_flag ) {
		UI8 field_pic_flag; //2 u(1)
		//if( field_pic_flag ){
			UI8 bottom_field_flag; //2 u(1)
		//}
	//}
	//if( IdrPicFlag ){
		UI64 idr_pic_id; //2 ue(v)
	//}
	//if( pic_order_cnt_type == 0 ) {
		UI32 pic_order_cnt_lsb; //2 u(v)
		//if( bottom_field_pic_order_in_frame_present_flag && !field_pic_flag ){
			SI64 delta_pic_order_cnt_bottom; //2 se(v)
		//}
	//}
	//if( pic_order_cnt_type == 1 && !delta_pic_order_always_zero_flag ) {
		SI64 delta_pic_order_cnt[ 2 ]; //2 se(v)
		//delta_pic_order_cnt[ 0 ]; //2 se(v)
		//if( bottom_field_pic_order_in_frame_present_flag && !field_pic_flag ){
		//	delta_pic_order_cnt[ 1 ]; //2 se(v)
		//}
	//}
	//if( redundant_pic_cnt_present_flag ){
		UI64 redundant_pic_cnt; //2 ue(v)
	//}
	//if( slice_type == B ){
		UI8 direct_spatial_mv_pred_flag; //2 u(1)
	//}
	//if(slice_type == P || slice_type == SP || slice_type == B){
		UI8 num_ref_idx_active_override_flag; //2 u(1)
		//if( num_ref_idx_active_override_flag ) {
			UI64 num_ref_idx_l0_active_minus1; //2 ue(v)
			//if( slice_type == B ){
				UI64 num_ref_idx_l1_active_minus1; //2 ue(v)
			//}
		//}
	//}
	//if( nal_unit_type == 20 || nal_unit_type == 21 ){
		STNBAvc_ref_pic_list_mvc_modification* ref_pic_list_mvc_modification; //ref_pic_list_mvc_modification( ) /* specified in Annex H */
		BOOL ref_pic_list_mvc_modificationPresent;
	//} else {
		STNBAvc_ref_pic_list_modification* ref_pic_list_modification; //ref_pic_list_modification( )
		BOOL ref_pic_list_modificationPresent;
	//}
	//if( ( weighted_pred_flag && ( slice_type == P || slice_type == SP ) ) ||
	//	( weighted_bipred_idc == 1 && slice_type == B )
	//){
		STNBAvc_pred_weight_table* pred_weight_table; //pred_weight_table( )
		BOOL pred_weight_tablePresent;
	//}
	//if( nal_ref_idc != 0 ){
		STNBAvc_dec_ref_pic_marking* dec_ref_pic_marking; //dec_ref_pic_marking( )
		BOOL dec_ref_pic_markingPresent;
	//}
	//if( entropy_coding_mode_flag && slice_type != I && slice_type != SI ){
		UI64 cabac_init_idc; //2 ue(v) //The value of cabac_init_idc shall be in the range of 0 to 2, inclusive.
	//}
	SI64 slice_qp_delta; //2 se(v)
	//if(slice_type == SP || slice_type == SI){
		//if( slice_type == SP ){
			UI8 sp_for_switch_flag; //2 u(1)
		//}
		SI64 slice_qs_delta; //2 se(v)
	//}
	//if( deblocking_filter_control_present_flag ) {
		UI64 disable_deblocking_filter_idc; //2 ue(v)
		//if( disable_deblocking_filter_idc != 1 ) {
			SI64 slice_alpha_c0_offset_div2; //2 se(v)
			SI64 slice_beta_offset_div2; //2 se(v)
		//}
	//}
	//if( num_slice_groups_minus1 > 0 && slice_group_map_type >= 3 && slice_group_map_type <= 5){
		UI32 slice_group_change_cycle; //2 u(v)
	//}
} STNBAvc_slice_header;

typedef struct STNBAvc_slice_data_ {
	//if( entropy_coding_mode_flag ){
		//while( !byte_aligned() ){
			UI8 cabac_alignment_one_bit; //2 f(1)
		//}
	//}
	//CurrMbAddr = first_mb_in_slice * ( 1 + MbaffFrameFlag );
	//moreDataFlag = 1;
	//prevMbSkipped = 0;
	//do {
		//if( slice_type != I && slice_type != SI ){
			//if( !entropy_coding_mode_flag ) {
				UI64 mb_skip_run; //2 ue(v)
				//prevMbSkipped = ( mb_skip_run > 0 );
				//for( i=0; i<mb_skip_run; i++ ){
					//CurrMbAddr = NextMbAddress( CurrMbAddr );
				//}
				//if( mb_skip_run > 0 ){
					//moreDataFlag = more_rbsp_data( );
				//}
			//} else {
				UI64 mb_skip_flag; //2 ae(v)
				//moreDataFlag = !mb_skip_flag;
			//}
		//}
		//if( moreDataFlag ) {
			//if( MbaffFrameFlag && ( CurrMbAddr % 2 = = 0 || ( CurrMbAddr % 2 = = 1 && prevMbSkipped ) ) ){ 
				UI64 mb_field_decoding_flag; //2 u(1) | ae(v)
			//}
			//macroblock_layer( );
		//}
		//if( !entropy_coding_mode_flag ){
			//moreDataFlag = more_rbsp_data( ) 
		//} else {
			//if( slice_type != I && slice_type != SI ){
				//prevMbSkipped = mb_skip_flag;
			//}
			//if( MbaffFrameFlag && CurrMbAddr % 2 = = 0 ) {
				//moreDataFlag = 1
			//} else {
				UI64 end_of_slice_flag; //2 ae(v)
				//moreDataFlag = !end_of_slice_flag
			//}
		//}
		//CurrMbAddr = NextMbAddress( CurrMbAddr )
	//} while( moreDataFlag );
} STNBAvc_slice_data;

void NBAvc_tableUI32_concat(const UI32* tbl, const UI32 width, const UI32 height, STNBString* dst);

//slice_layer_without_partitioning_rbsp (nal_unit_type = 1, 5 or 19; [Coded slice of a non-IDR picture], [Coded slice of an IDR picture] or [Coded slice of an auxiliary coded picture without partitioning])

typedef struct STNBAvc_slice_layer_without_partitioning_rbsp_ {
	STNBAvc_slice_header*			slice_header; //slice_header ()
	STNBAvc_slice_data*				slice_data; //slice_data ()
	//STNBAvc_rbsp_trailing_bits*	rbsp_trailing_bits; //rbsp_trailing_bits( ) 1
} STNBAvc_slice_layer_without_partitioning_rbsp;

#endif /* NBAvc_h */
