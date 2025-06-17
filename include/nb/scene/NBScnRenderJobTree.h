#ifndef NB_SCN_RENDER_JOB_TREE_H
#define NB_SCN_RENDER_JOB_TREE_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBArray.h"
#include "nb/2d/NBMatrix.h"
#include "nb/2d/NBColor.h"
//
#include "nb/scene/NBScnRenderJobDefs.h"

#ifdef __cplusplus
extern "C" {
#endif

	//-------------------
	//-- STNBScnVertex
	//-------------------

	//These MACROs are useful for shader code.
#define NBScnVertex_IDX_x			0u
#define NBScnVertex_IDX_y			4u
#define NBScnVertex_IDX_color		8u
#define NBScnVertex_SZ				12u

	//Vertex without texture
	typedef struct STNBScnVertex_ {
		float		x;
		float		y;
		STNBColor8	color;
	} STNBScnVertex;

	//--------------------
	//-- STNBScnVertexTex
	//--------------------

	//These MACROs are useful for shader code.
#define NBScnVertexTex_IDX_x		0u
#define NBScnVertexTex_IDX_y		4u
#define NBScnVertexTex_IDX_color	8u
#define NBScnVertexTex_IDX_tex_x	12u
#define NBScnVertexTex_IDX_tex_y	16u
#define NBScnVertexTex_SZ			20u

	//Vertex with one texture
	typedef struct STNBScnVertexTex_ {
		float		x;
		float		y;
		STNBColor8	color;
		STNBPoint	tex;
	} STNBScnVertexTex;

	
	//--------------------
	//-- STNBScnVertexTex2
	//--------------------

	//These MACROs are useful for shader code.
#define NBScnVertexTex2_IDX_x		0u
#define NBScnVertexTex2_IDX_y		4u
#define NBScnVertexTex2_IDX_color	8u
#define NBScnVertexTex2_IDX_tex_x	12u
#define NBScnVertexTex2_IDX_tex_y	16u
#define NBScnVertexTex2_IDX_tex2_x	20u
#define NBScnVertexTex2_IDX_tex2_y	24u
#define NBScnVertexTex2_SZ			28u

	//Vertex with 2 textures
	typedef struct STNBScnVertexTex2_ {
		float		x;
		float		y;
		STNBColor8	color;
		STNBPoint	tex;
		STNBPoint	tex2;
	} STNBScnVertexTex2;

	//--------------------
	//-- STNBScnVertexTex3
	//--------------------

	//These MACROs are useful for shader code.
#define NBScnVertexTex3_IDX_x		0u
#define NBScnVertexTex3_IDX_y		4u
#define NBScnVertexTex3_IDX_color	8u
#define NBScnVertexTex3_IDX_tex_x	12u
#define NBScnVertexTex3_IDX_tex_y	16u
#define NBScnVertexTex3_IDX_tex2_x	20u
#define NBScnVertexTex3_IDX_tex2_y	24u
#define NBScnVertexTex3_IDX_tex3_x	28u
#define NBScnVertexTex3_IDX_tex3_y	32u
#define NBScnVertexTex3_SZ			36u

	//Vertex with 3 textures
	typedef struct STNBScnVertexTex3_ {
		float		x;
		float		y;
		STNBColor8	color;
		STNBPoint	tex;
		STNBPoint	tex2;
		STNBPoint	tex3;
	} STNBScnVertexTex3;

	//---------------------
	//-- STNBScnTreeNode
	//---------------------

	//These MACROs are useful for shader code.
#define NBScnTreeNode_IDX_iParent		0u
#define NBScnTreeNode_IDX_pck			4u
#define NBScnTreeNode_IDX_t_x			8u
#define NBScnTreeNode_IDX_t_y			12u
#define NBScnTreeNode_IDX_t_deg			16u
#define NBScnTreeNode_IDX_t_sX			20u
#define NBScnTreeNode_IDX_t_sY			24u
#define NBScnTreeNode_IDX_c				28u
#define NBScnTreeNode_IDX_vs_iFirst		32u
#define NBScnTreeNode_IDX_vs_count		36u
#define NBScnTreeNode_SZ				40u

#define NB_SCN_TREE_NODE_ISHIDDEN_BIT			0x1u		//0000-0000 0000-0000 0000-0000 0000-0001b
#define NB_SCN_TREE_NODE_ISDISABLED_BIT			0x2u		//0000-0000 0000-0000 0000-0000 0000-0010b
//
#define NB_SCN_TREE_NODE_VERTS_TYPE_MSK			0x300u		//0000-0000 0000-0000 0000-0011 0000-0000b
#define NB_SCN_TREE_NODE_VERTS_TYPE_BIT_FIRST	0x100u		//0000-0000 0000-0000 0000-0001 0000-0000b
#define NB_SCN_TREE_NODE_VERTS_TYPE_BIT_LAST	0x200u		//0000-0000 0000-0000 0000-0001 0000-0000b
#define NB_SCN_TREE_NODE_VERTS_TYPE_MSK_MAX		0x3u		//
//
#define NB_SCN_TREE_NODE_CHLD_COUNT_MSK			0xFFFF0000u	//1111-1111 1111-1111 0000-0000 0000-0000b
#define NB_SCN_TREE_NODE_CHLD_COUNT_BIT_FIRST	0x10000u	//0000-0000 0000-0001 0000-0000 0000-0000b
#define NB_SCN_TREE_NODE_CHLD_COUNT_BIT_LAST	0x80000000u	//1000-0000 0000-0000 0000-0000 0000-0000b
#define NB_SCN_TREE_NODE_CHLD_COUNT_MSK_MAX		0xFFFFu		//
//
#define NB_COLOR8_R_MSK				0xFF000000u	//1111-1111 0000-0000 0000-0000 0000-0000b
#define NB_COLOR8_R_BIT_FIRST		0x1000000u	//0000-0001 0000-0000 0000-0000 0000-0000b
#define NB_COLOR8_R_BIT_LAST		0x80000000u	//1000-0000 0000-0000 0000-0000 0000-0000b
#define NB_COLOR8_R_MSK_MAX			0xFFu		//
//
#define NB_COLOR8_G_MSK				0xFF0000u	//0000-0000 1111-1111 0000-0000 0000-0000b
#define NB_COLOR8_G_BIT_FIRST		0x10000u	//0000-0000 0000-0001 0000-0000 0000-0000b
#define NB_COLOR8_G_BIT_LAST		0x800000u	//0000-0000 1000-0000 0000-0000 0000-0000b
#define NB_COLOR8_G_MSK_MAX			0xFFu		//
//
#define NB_COLOR8_B_MSK				0xFFFF00u	//0000-0000 0000-0000 1111-1111 0000-0000b
#define NB_COLOR8_B_BIT_FIRST		0x100u		//0000-0000 0000-0000 0000-0001 0000-0000b
#define NB_COLOR8_B_BIT_LAST		0x8000u		//0000-0000 0000-0000 1000-0000 0000-0000b
#define NB_COLOR8_B_MSK_MAX			0xFFu		//
//
#define NB_COLOR8_A_MSK				0xFFu		//0000-0000 0000-0000 0000-0000 1111-1111b
#define NB_COLOR8_A_BIT_FIRST		0x1u		//0000-0000 0000-0000 0000-0000 0000-0001b
#define NB_COLOR8_A_BIT_LAST		0x80u		//0000-0000 0000-0000 0000-0000 1000-0000b
#define NB_COLOR8_A_MSK_MAX			0xFFu		//

//
//DO NOT DELETE: these macros are used for GLSl and HLSL, which lack ~, <<, >>, &, | operator.
//
#define NBScnTreeNode_getIsHiddenV(O)	(((O) / NB_SCN_TREE_NODE_ISHIDDEN_BIT) % 2u)
#define NBScnTreeNode_getIsDisabledV(O)	(((O) / NB_SCN_TREE_NODE_ISDISABLED_BIT) % 2u)
#define NBScnTreeNode_getVertsTypeV(O)	(((O) / NB_SCN_TREE_NODE_VERTS_TYPE_BIT_FIRST) % (NB_SCN_TREE_NODE_VERTS_TYPE_MSK_MAX + 1u))
#define NBScnTreeNode_getChildCountV(O)	(((O) / NB_SCN_TREE_NODE_CHLD_COUNT_BIT_FIRST) % (NB_SCN_TREE_NODE_CHLD_COUNT_MSK_MAX + 1u))
#define NBColor8_getRV(O)				(((O) / NB_COLOR8_R_BIT_FIRST) % (NB_COLOR8_R_MSK_MAX + 1u))
#define NBColor8_getGV(O)				(((O) / NB_COLOR8_G_BIT_FIRST) % (NB_COLOR8_G_MSK_MAX + 1u))
#define NBColor8_getBV(O)				(((O) / NB_COLOR8_B_BIT_FIRST) % (NB_COLOR8_B_MSK_MAX + 1u))
#define NBColor8_getAV(O)				(((O) / NB_COLOR8_A_BIT_FIRST) % (NB_COLOR8_A_MSK_MAX + 1u))

#define NBScnTreeNode_getIsHidden(O)	NBScnTreeNode_getIsHiddenV((O)->pck)
#define NBScnTreeNode_getIsDisabled(O)	NBScnTreeNode_getIsDisabledV((O)->pck)
#define NBScnTreeNode_getVertsType(O)	NBScnTreeNode_getVertsTypeV((O)->pck)
#define NBScnTreeNode_getChildCount(O)	NBScnTreeNode_getChildCountV((O)->pck)
#define NBColor8_getR(O)				NBColor8_getRV((O)->pck)
#define NBColor8_getG(O)				NBColor8_getGV((O)->pck)
#define NBColor8_getB(O)				NBColor8_getBV((O)->pck)
#define NBColor8_getA(O)				NBColor8_getAV((O)->pck)

#define NBScnTreeNode_withIsHidden(B, V)	NB_APPLY_BIT(B, V, NB_SCN_TREE_NODE_ISHIDDEN_BIT)
#define NBScnTreeNode_withIsDisabled(B, V)	NB_APPLY_BIT(B, V, NB_SCN_TREE_NODE_ISDISABLED_BIT)
#define NBScnTreeNode_withVertsType(B, V)	NB_APPLY_BITS(B, V, NB_SCN_TREE_NODE_VERTS_TYPE_MSK, NB_SCN_TREE_NODE_VERTS_TYPE_BIT_FIRST, NB_SCN_TREE_NODE_VERTS_TYPE_BIT_LAST, NB_SCN_TREE_NODE_VERTS_TYPE_MSK_MAX)
#define NBScnTreeNode_withChildCount(B, V)	NB_APPLY_BITS(B, V, NB_SCN_TREE_NODE_CHLD_COUNT_MSK, NB_SCN_TREE_NODE_CHLD_COUNT_BIT_FIRST, NB_SCN_TREE_NODE_CHLD_COUNT_BIT_LAST, NB_SCN_TREE_NODE_CHLD_COUNT_MSK_MAX)

#define NBScnTreeNode_setIsHidden(O, V)		(O)->pck = NBScnTreeNode_withIsHidden((O)->pck, V)
#define NBScnTreeNode_setIsDisabled(O, V)	(O)->pck = NBScnTreeNode_withIsDisabled((O)->pck, V)
#define NBScnTreeNode_setVertsType(O, V)	(O)->pck = NBScnTreeNode_withVertsType((O)->pck, V)
#define NBScnTreeNode_setChildCount(O, V)	(O)->pck = NBScnTreeNode_withChildCount((O)->pck, V)

//Note: for hardware-compute all members must be 32-bits-aligned
	typedef struct STNBScnTreeNode_ {
		unsigned int iParent;	//root nodes point to themself
		unsigned int pck;		//packed (chldCount, verts.type, isDisabled, isHidden)
		//transform (local)
		struct {
			float	x;		//traslationX
			float	y;		//traslationY
			float	deg;	//rotation
			float	sX;		//scaleX
			float	sY;		//scaleY
		} transform;
		STNBColor8	color8;
		//verts (local vertex)
		struct {
			//vertices buffer type (0 = no tex coords, 1 = tex1, 2 = tex2, 3 = tex3)
			unsigned int iFirst;
			unsigned int count;
		} verts;
	} STNBScnTreeNode;


	//-------------------------
	//-- STNBScnRenderJobFlatMap
	//-------------------------
	typedef struct STNBScnRenderJobTreeMap_ {
		//nodes
		struct {
			STNBScnTreeNode*		arr;	//pointer to buff.data[], do not deallocate
			UI32					use;
		} nodes;
		//verts (vertices)
		struct {
			//v
			struct {
				STNBScnVertex*		arr;	//pointer to buff.data[], do not deallocate
				UI32				use;
			} v;
			//v1
			struct {
				STNBScnVertexTex*	arr; //pointer to buff.data[], do not deallocate
				UI32				use;
			} v1;
			//v2
			struct {
				STNBScnVertexTex2*	arr;	//pointer to buff.data[], do not deallocate
				UI32				use;
			} v2;
			//v3
			struct {
				STNBScnVertexTex3*	arr; //pointer to buff.data[], do not deallocate
				UI32				use;
			} v3;
		} verts;
	} STNBScnRenderJobTreeMap;

	//-----------------------
	//-- STNBScnRenderJobTree
	//-----------------------
	typedef struct STNBScnRenderJobTree_ {
		STNBArray nodes;		//STNBScnTreeNode
		//verts (vertices)
		struct {
			STNBArray v;		//STNBScnVertex
			STNBArray v1;		//STNBScnVertexTex
			STNBArray v2;		//STNBScnVertexTex2
			STNBArray v3;		//STNBScnVertexTex3
		} verts;
	} STNBScnRenderJobTree;

	void NBScnRenderJobTree_init(STNBScnRenderJobTree* obj);
	void NBScnRenderJobTree_release(STNBScnRenderJobTree* obj);
	void NBScnRenderJobTree_empty(STNBScnRenderJobTree* obj);

	UI32 NBScnRenderJobTree_getDispatchBufferRngs(const STNBScnRenderJobTree* src, const STNBScnRenderJobLimits* limits, const UI32 offset, STNBScnRenderBuffRngs* dstRngs);

#ifdef __cplusplus
} //extern "C"
#endif

#endif