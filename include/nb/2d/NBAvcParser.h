//
//  NBAvcParser.h
//  nbframework
//
//  Created by Marcos Ortega on 9/21/20.
//

#ifndef NBAvcParser_h
#define NBAvcParser_h

#include "nb/NBFrameworkDefs.h"
#include "nb/2d/NBSize.h"
#include "nb/2d/NBAvc.h"
#include "nb/core/NBStructMap.h"

//NBAvcPicDescBase

typedef struct STNBAvcPicDescBase_ {
	UI16	w;			//pixels = luma-samples = (w-mbs * 16)
	UI16	h;			//pixels = samples = ((h-units to h-mbs formula) * 16)
	UI16	fpsMax;		//frames per sec
	BOOL	isFixedFps;	//frames per sec is fixed
	//sps
	UI8		avcProfile;		//sps first byte, determines the bitstream format (higher profiles are backward compatible with lower profiles)
	UI8		profConstraints; //sps second byte, determines the profile's contrains flags, each active flag represents
	UI8		profLevel;		//sps third byte, determines the profile's maximun values of certain fields (required maximun decoding power)
	UI8		chromaFmt;		//analyzed
	UI8		bitDepthLuma;	//analyzed (not-found if lower than 8)
	UI8		bitDepthChroma;	//analyzed (not-found if lower than 8)
} STNBAvcPicDescBase;

const STNBStructMap* NBAvcPicDescBase_getSharedStructMap(void);

#define NBAvcPicDescBase_init(OBJ)			NBMemory_setZeroSt(*(OBJ), STNBAvcPicDescBase)
#define NBAvcPicDescBase_release(OBJ)		//nothing
#define NBAvcPicDescBase_setAsOther(OBJ, OTHER)	*OBJ = *OTHER;

//Returns true if the avcProfile of 'other' is lower or equals.
BOOL NBAvcPicDescBase_isCompatibleProfile(const STNBAvcPicDescBase* obj, const STNBAvcPicDescBase* other);

//Returns true if the image size are the same and the profile of 'other' is lower or equals.
BOOL NBAvcPicDescBase_isCompatible(const STNBAvcPicDescBase* obj, const STNBAvcPicDescBase* other);

//parser

typedef struct STNBAvcParser_ {
	//Current NALU
	struct {
		const STNBAvcNaluMap* map;
		//
		struct {
			UI32 bytesCount;
			UI32 bitsCount;
		} remain;
	} curNALU;
	//Current params
	struct {
		STNBAvcStateParams data;
	} params;
	//picDescBase
	struct {
		//params (required to calculate)
		struct {
			//seq_parameter_set
			struct {
				UI32 iSeq;								//zero mean no-loaded yet
				UI8 profile_idc;						//required for STNBAvcPicDescBase
				UI8 constraint_set;						//required for STNBAvcPicDescBase
				UI8 level_idc;							//required for STNBAvcPicDescBase
				UI8 chroma_format_idc;					//required for 'w' and 'h' (cropping) and STNBAvcPicDescBase
				UI8 bit_depth_luma_minus8;				//required for STNBAvcPicDescBase
				UI8 bit_depth_chroma_minus8;			//required for STNBAvcPicDescBase
				BOOL separate_colour_plane_flag;		//required for 'w' and 'h' (cropping)
				UI64 pic_width_in_mbs_minus1;			//required for 'w'
				UI64 pic_height_in_map_units_minus1;	//required for 'h'
				UI64 frame_mbs_only_flag;				//required for 'h'
				UI64 frame_crop_left_offset;			//required for 'w' and 'h' (cropping)
				UI64 frame_crop_right_offset;			//required for 'w' and 'h' (cropping)
				UI64 frame_crop_top_offset;				//required for 'w' and 'h' (cropping)
				UI64 frame_crop_bottom_offset;			//required for 'w' and 'h' (cropping)
				BOOL vui_parameters_present_flag;		//required for 'fps'
				//vui_parameters
				struct {
					UI32 iSeq;							//zero mean no-loaded yet
					UI32 num_units_in_tick;				//required for 'fps'
					UI32 time_scale;					//required for 'fps'
					BOOL fixed_frame_rate_flag;			//required for 'fps'
				} vui_parameters;
			} sps;
			//slice_header
			struct {
				UI32 iSeq;								//zero mean no-loaded yet
				BOOL field_pic_flag;					//required fo 'h'
			} slice_header;
		} params;
		//current (calculated value)
		struct {
			UI32				iSeq;					//zero mean no-calculated yet
			STNBAvcPicDescBase	data;
		} cur;
	} picDescBase;
} STNBAvcParser;

void NBAvcParser_init(STNBAvcParser* obj);
void NBAvcParser_release(STNBAvcParser* obj);

//Feed

BOOL NBAvcParser_feedStart(STNBAvcParser* obj, const UI8 nal_ref_idc, const UI8 nal_unit_type);
//UI32 NBAvcParser_feedBytes(STNBAvcParser* obj, const void* data, const UI32 dataSz); //Partial NALU (slow)
BOOL NBAvcParser_feedNALU(STNBAvcParser* obj, const void* prefix, const UI32 prefixSz, const void* pay, const UI32 paySz, const BOOL picDescBaseOnly); //'picDescBaseOnly' = quick-parse only data necesary for 'NBAvcParser_getPicDescBase()' (no image data)
BOOL NBAvcParser_feedNALUChunked(STNBAvcParser* obj, const void* prefix, const UI32 prefixSz, const STNBDataPtr* chunks, const SI32 chunksSz, const BOOL picDescBaseOnly); //'picDescBaseOnly' = quick-parse only data necesary for 'NBAvcParser_getPicDescBase()' (no image data)
BOOL NBAvcParser_feedEnd(STNBAvcParser* obj);

//Cfg

BOOL NBAvcParser_getPicDescBase(STNBAvcParser* obj, const UI32 iSeqFilter, STNBAvcPicDescBase* dst, UI32* dstSeqIdx);

#endif /* NBAvcParser_h */
