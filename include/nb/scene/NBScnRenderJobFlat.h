#ifndef NB_SCN_RENDER_JOB_FLAT_H
#define NB_SCN_RENDER_JOB_FLAT_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBArray.h"
#include "nb/2d/NBMatrix.h"
#include "nb/2d/NBColor.h"
//
#include "nb/scene/NBScnRenderJobDefs.h"
#include "nb/scene/NBScnRenderJobTree.h"

#ifdef __cplusplus
extern "C" {
#endif

	//-------------------
	//-- STNBScnVertexF
	//-------------------

	//These MACROs are useful for shader code.
#define NBScnVertexF_IDX_x			0u
#define NBScnVertexF_IDX_y			4u
#define NBScnVertexF_IDX_color_r	8u
#define NBScnVertexF_IDX_color_g	12u
#define NBScnVertexF_IDX_color_b	16u
#define NBScnVertexF_IDX_color_a	20u
#define NBScnVertexF_SZ				24u

	//Vertex without texture
	typedef struct STNBScnVertexF_ {
		float		x;
		float		y;
		STNBColor	color;
	} STNBScnVertexF;

	//--------------------
	//-- STNBScnVertexTexF
	//--------------------
// 
	//These MACROs are useful for shader code.
#define NBScnVertexTexF_IDX_x		0u
#define NBScnVertexTexF_IDX_y		4u
#define NBScnVertexTexF_IDX_color_r	8u
#define NBScnVertexTexF_IDX_color_g	12u
#define NBScnVertexTexF_IDX_color_b	16u
#define NBScnVertexTexF_IDX_color_a	20u
#define NBScnVertexTexF_IDX_tex_x	24u
#define NBScnVertexTexF_IDX_tex_y	28u
#define NBScnVertexTexF_SZ			32u

	//Vertex with one texture
	typedef struct STNBScnVertexTexF_ {
		float		x;
		float		y;
		STNBColor	color;
		STNBPoint	tex;
	} STNBScnVertexTexF;


//--------------------
//-- STNBScnVertexTex2F
//--------------------

//These MACROs are useful for shader code.
#define NBScnVertexTex2F_IDX_x			0u
#define NBScnVertexTex2F_IDX_y			4u
#define NBScnVertexTex2F_IDX_color_r	8u
#define NBScnVertexTex2F_IDX_color_g	12u
#define NBScnVertexTex2F_IDX_color_b	16u
#define NBScnVertexTex2F_IDX_color_a	20u
#define NBScnVertexTex2F_IDX_tex_x		24u
#define NBScnVertexTex2F_IDX_tex_y		28u
#define NBScnVertexTex2F_IDX_tex2_x		32u
#define NBScnVertexTex2F_IDX_tex2_y		36u
#define NBScnVertexTex2F_SZ				40u

	//Vertex with 2 textures
	typedef struct STNBScnVertexTex2F_ {
		float		x;
		float		y;
		STNBColor	color;
		STNBPoint	tex;
		STNBPoint	tex2;
	} STNBScnVertexTex2F;

	
//--------------------
//-- STNBScnVertexTex3F
//--------------------

//These MACROs are useful for shader code.
#define NBScnVertexTex3F_IDX_x			0u
#define NBScnVertexTex3F_IDX_y			4u
#define NBScnVertexTex3F_IDX_color_r	8u
#define NBScnVertexTex3F_IDX_color_g	12u
#define NBScnVertexTex3F_IDX_color_b	16u
#define NBScnVertexTex3F_IDX_color_a	20u
#define NBScnVertexTex3F_IDX_tex_x		24u
#define NBScnVertexTex3F_IDX_tex_y		28u
#define NBScnVertexTex3F_IDX_tex2_x		32u
#define NBScnVertexTex3F_IDX_tex2_y		36u
#define NBScnVertexTex3F_IDX_tex3_x		40u
#define NBScnVertexTex3F_IDX_tex3_y		44u
#define NBScnVertexTex3F_SZ				48u

	//Vertex with 3 textures
	typedef struct STNBScnVertexTex3F_ {
		float		x;
		float		y;
		STNBColor	color;
		STNBPoint	tex;
		STNBPoint	tex2;
		STNBPoint	tex3;
	} STNBScnVertexTex3F;

	//---------------------
	//-- STNBScnFlatNode
	//---------------------

	//These MACROs are useful for shader code.
#define NBScnFlatNode_IDX_pck		0u
#define NBScnFlatNode_IDX_m_0_0		4u
#define NBScnFlatNode_IDX_m_0_1		8u
#define NBScnFlatNode_IDX_m_0_2		12u
#define NBScnFlatNode_IDX_m_1_0		16u
#define NBScnFlatNode_IDX_m_1_1		20u
#define NBScnFlatNode_IDX_m_1_2		24u
#define NBScnFlatNode_IDX_mInv_0_0	28u
#define NBScnFlatNode_IDX_mInv_0_1	32u
#define NBScnFlatNode_IDX_mInv_0_2	36u
#define NBScnFlatNode_IDX_mInv_1_0	40u
#define NBScnFlatNode_IDX_mInv_1_1	44u
#define NBScnFlatNode_IDX_mInv_1_2	48u
#define NBScnFlatNode_IDX_c_r		52u
#define NBScnFlatNode_IDX_c_g		56u
#define NBScnFlatNode_IDX_c_b		60u
#define NBScnFlatNode_IDX_c_a		64u
#define NBScnFlatNode_SZ			68u

#define NB_SCN_FLAT_NODE_ISHIDDEN_BIT			0x1u		//0000-0000 0000-0000 0000-0000 0000-0001b
#define NB_SCN_FLAT_NODE_ISDISABLED_BIT			0x2u		//0000-0000 0000-0000 0000-0000 0000-0010b
//
#define NB_SCN_FLAT_NODE_DEEP_LVL_MSK			0xFFFF0000u	//1111-1111 1111-1111 0000-0000 0000-0000b
#define NB_SCN_FLAT_NODE_DEEP_LVL_BIT_FIRST		0x10000u	//0000-0000 0000-0001 0000-0000 0000-0000b
#define NB_SCN_FLAT_NODE_DEEP_LVL_BIT_LAST		0x80000000u	//1000-0000 0000-0000 0000-0000 0000-0000b
#define NB_SCN_FLAT_NODE_DEEP_LVL_MSK_MAX		0xFFFFu		//

//
//DO NOT DELETE: these macros are used for GLSl and HLSL, which lack ~, <<, >>, &, | operator.
//
#define NBScnFlatNode_getIsHidden(O)		(((O)->pck / NB_SCN_FLAT_NODE_ISHIDDEN_BIT) % 2u)
#define NBScnFlatNode_getIsDisabled(O)		(((O)->pck / NB_SCN_FLAT_NODE_ISDISABLED_BIT) % 2u)
#define NBScnFlatNode_getDeepLvl(O)			(((O)->pck / NB_SCN_FLAT_NODE_DEEP_LVL_BIT_FIRST) % (NB_SCN_FLAT_NODE_DEEP_LVL_MSK_MAX + 1u))

#define NBScnFlatNode_withIsHidden(B, V)	NB_APPLY_BIT(B, V, NB_SCN_FLAT_NODE_ISHIDDEN_BIT)
#define NBScnFlatNode_withIsDisabled(B, V)	NB_APPLY_BIT(B, V, NB_SCN_FLAT_NODE_ISDISABLED_BIT)
#define NBScnFlatNode_withDeepLvl(B, V)		NB_APPLY_BITS(B, V, NB_SCN_FLAT_NODE_DEEP_LVL_MSK, NB_SCN_FLAT_NODE_DEEP_LVL_BIT_FIRST, NB_SCN_FLAT_NODE_DEEP_LVL_BIT_LAST, NB_SCN_FLAT_NODE_DEEP_LVL_MSK_MAX)

#define NBScnFlatNode_setIsHidden(O, V)		(O)->pck = NBScnFlatNode_withIsHidden((O)->pck, V)
#define NBScnFlatNode_setIsDisabled(O, V)	(O)->pck = NBScnFlatNode_withIsDisabled((O)->pck, V)
#define NBScnFlatNode_setDeepLvl(O, V)		(O)->pck = NBScnFlatNode_withDeepLvl((O)->pck, V)

//Note: for hardware-compute all members must be 32-bits-aligned
	typedef struct STNBScnFlatNode_ {
		unsigned int	pck;		//packed (iDeepLvl, isDisabled, isHidden)
		STNBMatrix2D	matrix;		//matrix (render matrix)
		STNBMatrix2D	matrixInv;	//matrixInv (render inverse matrix, used to convert scene to local coordinates)
		STNBColor		color;
	} STNBScnFlatNode;

	//-------------------------
	//-- STNBScnRenderJobFlatMap
	//-------------------------
	typedef struct STNBScnRenderJobFlatMap_ {
		//nodes
		struct {
			STNBScnFlatNode*		arr;	//pointer to buff.data[], do not deallocate
			UI32					use;
		} nodes;
		//verts (vertices)
		struct {
			//v
			struct {
				STNBScnVertexF*		arr;	//pointer to buff.data[], do not deallocate
				UI32				use;
			} v;
			//v1
			struct {
				STNBScnVertexTexF*	arr; //pointer to buff.data[], do not deallocate
				UI32				use;
			} v1;
			//v2
			struct {
				STNBScnVertexTex2F*	arr;	//pointer to buff.data[], do not deallocate
				UI32				use;
			} v2;
			//v3
			struct {
				STNBScnVertexTex3F*	arr; //pointer to buff.data[], do not deallocate
				UI32				use;
			} v3;
		} verts;
	} STNBScnRenderJobFlatMap;

	void NBScnRenderJobFlatMap_init(STNBScnRenderJobFlatMap* obj);
	void NBScnRenderJobFlatMap_release(STNBScnRenderJobFlatMap* obj);
	//
	void NBScnRenderJobFlatMap_reset(STNBScnRenderJobFlatMap* obj);
	void NBScnRenderJobFlatMap_build(STNBScnRenderJobFlatMap* obj, void* ptr, const STNBScnRenderBuffRngs* rngs);

	//-------------------------
	//-- STNBScnRenderJobFlat
	//-------------------------
	typedef struct STNBScnRenderJobFlat_ {
		//buff
		struct {
			BYTE*	data;
			UI32	use;
			UI32	sz;
		} buff;
		STNBScnRenderJobFlatMap map;
	} STNBScnRenderJobFlat;

	void NBScnRenderJobFlat_init(STNBScnRenderJobFlat* obj);
	void NBScnRenderJobFlat_release(STNBScnRenderJobFlat* obj);
	UI32 NBScnRenderJobFlat_prepare(STNBScnRenderJobFlat* obj, const STNBScnRenderJobTree* src, const STNBScnRenderJobLimits* limits, const UI32 offset, STNBScnRenderBuffRngs* dstRngs);
	//this algorithm is runs over the tree,
	//and cannot be called in parallel;
	//each node is passed just once.
	UI32 NBScnRenderJobFlat_dispatchForwards(STNBScnRenderJobFlat* obj, const STNBScnRenderJobTree* src, const STNBScnRenderJobLimits* limits);
	//this algorithm can be called in parallel
	//but the same node is calculated multiple times.
	UI32 NBScnRenderJobFlat_dispatchBackwards(STNBScnRenderJobFlat* obj, const STNBScnRenderJobTree* src, const STNBScnRenderJobLimits* limits, const UI32 iNode, const UI32 ammNodes);
	//
	UI32 NBScnRenderJobFlat_getDispatchHeaderPaddedSz(const STNBScnRenderJobTree* src, const STNBScnRenderJobLimits* limits);
	UI32 NBScnRenderJobFlat_getDispatchHeadersBufferSz(const STNBScnRenderJobTree* src, const STNBScnRenderJobLimits* limits);
	UI32 NBScnRenderJobFlat_getDispatcCallsNeeded(const STNBScnRenderJobTree* src, const STNBScnRenderJobLimits* limits);
	UI32 NBScnRenderJobFlat_getDispatchBufferRngs(const STNBScnRenderJobTree* src, const STNBScnRenderJobLimits* limits, const UI32 offset, STNBScnRenderBuffRngs* dstRngs);
	

#ifdef __cplusplus
} //extern "C"
#endif

#endif