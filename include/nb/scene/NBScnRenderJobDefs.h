#ifndef NB_SCN_RENDER_JOB_DEFS_H
#define NB_SCN_RENDER_JOB_DEFS_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBArray.h"
#include "nb/2d/NBMatrix.h"
#include "nb/2d/NBColor.h"
//
#include "nb/scene/NBScnRenderJobMacros.h"

#ifdef __cplusplus
extern "C" {
#endif

//
//DO NOT DELETE: these macros are used for GLSl and HLSL, which lack ~, <<, >>, &, | operator.
//
#define NB_APPLY_BIT(BASE, BVAL, MASK_BIT)	\
											((((BASE) / 2u) / MASK_BIT * MASK_BIT * 2u) + (((BVAL) % 2u) * MASK_BIT)  + ((BASE) % MASK_BIT))
//
#define NB_APPLY_BITS(BASE, VAL, MASK_BITS, MASK_BIT_FRST, MASK_BIT_LST, MASK_MAX_VAL) \
											(((BASE / 2u) / MASK_BIT_LST * MASK_BIT_LST * 2u) + ((VAL % (MASK_MAX_VAL + 1u)) * MASK_BIT_FRST) + ((BASE) % MASK_BIT_FRST))

	//-----------------------
	//-- STNBScnRenderBuffRng
	//-----------------------
	// 
	//These MACROs are useful for shader code.
#define NBScnRenderBuffRng_IDX_offset	0u
#define NBScnRenderBuffRng_IDX_use		4u
#define NBScnRenderBuffRng_IDX_sz		8u
#define NBScnRenderBuffRng_SZ			12u

	typedef struct STNBScnRenderBuffRng_ {
		UI32 offset;	//start of data in buffer
		UI32 use;		//after offset, valid data
		UI32 sz;		//after offset, allocated space
	} STNBScnRenderBuffRng;

	//------------------------
	//-- STNBScnRenderBuffRngs
	//------------------------

	//These MACROs are useful for shader code.
#define NBScnRenderBuffRngs_IDX_nodes	0u
#define NBScnRenderBuffRngs_IDX_v		(NBScnRenderBuffRng_SZ * 1u)
#define NBScnRenderBuffRngs_IDX_v1		(NBScnRenderBuffRng_SZ * 2u)
#define NBScnRenderBuffRngs_IDX_v2		(NBScnRenderBuffRng_SZ * 3u)
#define NBScnRenderBuffRngs_IDX_v3		(NBScnRenderBuffRng_SZ * 4u)
#define NBScnRenderBuffRngs_SZ			(NBScnRenderBuffRng_SZ * 5u)

	typedef struct STNBScnRenderBuffRngs_ {
		STNBScnRenderBuffRng nodes;	//nodes
		//verts
		struct {
			STNBScnRenderBuffRng v;		//vertices without textures
			STNBScnRenderBuffRng v1;	//vertices with one texture
			STNBScnRenderBuffRng v2;	//vertices with two textures
			STNBScnRenderBuffRng v3;	//vertices with three textures
		} verts;
	} STNBScnRenderBuffRngs;

	//------------------------------
	//-- STNBScnRenderDispatchHeader
	//------------------------------

	//These MACROs are useful for shader code.
#define NBScnRenderDispatchHeader_IDX_iNodeOffset	0u
#define NBScnRenderDispatchHeader_IDX_src			4u
#define NBScnRenderDispatchHeader_IDX_dst			(4u + (NBScnRenderBuffRngs_SZ * 1u))
#define NBScnRenderDispatchHeader_SZ				(4u + (NBScnRenderBuffRngs_SZ * 2u))

	typedef struct STNBScnRenderDispatchHeader_ {
		UI32					iNodeOffset;	//nodes already executed on previous Dispatch() calls
		STNBScnRenderBuffRngs	src;			//src-data map in buffer
		STNBScnRenderBuffRngs	dst;			//dst-data map in buffer
	} STNBScnRenderDispatchHeader;

	//-------------------------
	//-- STNBScnRenderJobLimits
	//-------------------------
	typedef struct STNBScnRenderJobLimits_ {
		//header (dispatch-params, uniform buffers)
		struct {
			UI32 alignment;		//offset and size for header (dispatch-params, uniform buffers) structures
		} header;
		//buffer
		struct {
			UI32 alignment;		//offset and size for data buffer
		} buffer;
		//dispatch
		struct {
			UI32 maxThreads;	//maximun threads per dispacth (this will define the ammount of headers to allocate)
		} dispatch;
	} STNBScnRenderJobLimits;

#ifdef __cplusplus
} //extern "C"
#endif

#endif