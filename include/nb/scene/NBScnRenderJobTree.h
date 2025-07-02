#ifndef NB_SCN_RENDER_JOB_TREE_H
#define NB_SCN_RENDER_JOB_TREE_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBArray.h"
#include "nb/2d/NBMatrix.h"
//
#include "nb/scene/NBScnRenderJobDefs.h"
#include "nb/scene/NBScnVertices.h"

#ifdef __cplusplus
extern "C" {
#endif

//---------------------
//-- STNBScnTreeNode
//---------------------

//These MACROs are useful for shader code.
#define NBScnTreeNode_IDX_iParent       0u
#define NBScnTreeNode_IDX_pck           4u
#define NBScnTreeNode_IDX_t_x           8u
#define NBScnTreeNode_IDX_t_y           12u
#define NBScnTreeNode_IDX_t_deg         16u
#define NBScnTreeNode_IDX_t_sX          20u
#define NBScnTreeNode_IDX_t_sY          24u
#define NBScnTreeNode_IDX_c             28u
#define NBScnTreeNode_IDX_vs_iFirst     32u
#define NBScnTreeNode_IDX_vs_count      36u
#define NBScnTreeNode_SZ                40u

#define NB_SCN_TREE_NODE_ISHIDDEN_BIT           0x1u        //0000-0000 0000-0000 0000-0000 0000-0001b
#define NB_SCN_TREE_NODE_ISDISABLED_BIT         0x2u        //0000-0000 0000-0000 0000-0000 0000-0010b
//
#define NB_SCN_TREE_NODE_VERTS_TYPE_MSK         0x300u        //0000-0000 0000-0000 0000-0011 0000-0000b
#define NB_SCN_TREE_NODE_VERTS_TYPE_BIT_FIRST   0x100u        //0000-0000 0000-0000 0000-0001 0000-0000b
#define NB_SCN_TREE_NODE_VERTS_TYPE_BIT_LAST    0x200u        //0000-0000 0000-0000 0000-0001 0000-0000b
#define NB_SCN_TREE_NODE_VERTS_TYPE_MSK_MAX     0x3u        //
//
#define NB_SCN_TREE_NODE_CHLD_COUNT_MSK         0xFFFF0000u    //1111-1111 1111-1111 0000-0000 0000-0000b
#define NB_SCN_TREE_NODE_CHLD_COUNT_BIT_FIRST   0x10000u    //0000-0000 0000-0001 0000-0000 0000-0000b
#define NB_SCN_TREE_NODE_CHLD_COUNT_BIT_LAST    0x80000000u    //1000-0000 0000-0000 0000-0000 0000-0000b
#define NB_SCN_TREE_NODE_CHLD_COUNT_MSK_MAX     0xFFFFu        //

//
//DO NOT DELETE: these macros are used for GLSl and HLSL, which lack ~, <<, >>, &, | operator.
//
#define NBScnTreeNode_getIsHiddenV(O)	    (((O) / NB_SCN_TREE_NODE_ISHIDDEN_BIT) % 2u)
#define NBScnTreeNode_getIsDisabledV(O)	    (((O) / NB_SCN_TREE_NODE_ISDISABLED_BIT) % 2u)
#define NBScnTreeNode_getVertsTypeV(O)	    (((O) / NB_SCN_TREE_NODE_VERTS_TYPE_BIT_FIRST) % (NB_SCN_TREE_NODE_VERTS_TYPE_MSK_MAX + 1u))
#define NBScnTreeNode_getChildCountV(O)	    (((O) / NB_SCN_TREE_NODE_CHLD_COUNT_BIT_FIRST) % (NB_SCN_TREE_NODE_CHLD_COUNT_MSK_MAX + 1u))

#define NBScnTreeNode_getIsHidden(O)	    NBScnTreeNode_getIsHiddenV((O)->pck)
#define NBScnTreeNode_getIsDisabled(O)	    NBScnTreeNode_getIsDisabledV((O)->pck)
#define NBScnTreeNode_getVertsType(O)	    NBScnTreeNode_getVertsTypeV((O)->pck)
#define NBScnTreeNode_getChildCount(O)	    NBScnTreeNode_getChildCountV((O)->pck)

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
		UI32 iParent;	//root nodes point to themself
        UI32 pck;		//packed (chldCount, verts.type, isDisabled, isHidden)
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
            UI32 iFirst;
            UI32 count;
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
