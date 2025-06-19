
#include <metal_stdlib>
using namespace metal;

#define radians(DEG)    ( (DEG) * M_PI_F / 180.0f )

//using MACROS for GLSl and HLSL, which lack ~, <<, >>, &, | operator.

#define NB_APPLY_BIT(BASE, BVAL, MASK_BIT) \
           ((((BASE) / 2u) / MASK_BIT * MASK_BIT * 2u) + (((BVAL) % 2u) * MASK_BIT)  + ((BASE) % MASK_BIT))

#define NB_APPLY_BITS(BASE, VAL, MASK_BITS, MASK_BIT_FRST, MASK_BIT_LST, MASK_MAX_VAL) \
           (((BASE / 2u) / MASK_BIT_LST * MASK_BIT_LST * 2u) + ((VAL % (MASK_MAX_VAL + 1u)) * MASK_BIT_FRST) + ((BASE) % MASK_BIT_FRST))

//-----------------------
//-- STNBScnRenderBuffRng
//-----------------------

#define NBScnRenderBuffRng_IDX_offset   0u
#define NBScnRenderBuffRng_IDX_use      4u
#define NBScnRenderBuffRng_IDX_sz       8u
#define NBScnRenderBuffRng_SZ           12u

//------------------------
//-- STNBScnRenderBuffRngs
//------------------------

//These MACROs are useful for shader code.
#define NBScnRenderBuffRngs_IDX_nodes   0u
#define NBScnRenderBuffRngs_IDX_v       (NBScnRenderBuffRng_SZ * 1u)
#define NBScnRenderBuffRngs_IDX_v1      (NBScnRenderBuffRng_SZ * 2u)
#define NBScnRenderBuffRngs_IDX_v2      (NBScnRenderBuffRng_SZ * 3u)
#define NBScnRenderBuffRngs_IDX_v3      (NBScnRenderBuffRng_SZ * 4u)
#define NBScnRenderBuffRngs_SZ          (NBScnRenderBuffRng_SZ * 5u)

//------------------------------
//-- STNBScnRenderDispatchHeader
//------------------------------

//These MACROs are useful for shader code.
#define NBScnRenderDispatchHeader_IDX_iNodeOffset   0u
#define NBScnRenderDispatchHeader_IDX_src           4u
#define NBScnRenderDispatchHeader_IDX_dst           (4u + (NBScnRenderBuffRngs_SZ * 1u))
#define NBScnRenderDispatchHeader_SZ                (4u + (NBScnRenderBuffRngs_SZ * 2u))

//-------------------
//-- STNBScnVertex
//-------------------

//These MACROs are useful for shader code.
#define NBScnVertex_IDX_x            0u
#define NBScnVertex_IDX_y            4u
#define NBScnVertex_IDX_color        8u
#define NBScnVertex_SZ               12u


//-------------------
//-- STNBScnVertexF
//-------------------

//These MACROs are useful for shader code.
#define NBScnVertexF_IDX_x          0u
#define NBScnVertexF_IDX_y          4u
#define NBScnVertexF_IDX_color_r    8u
#define NBScnVertexF_IDX_color_g    12u
#define NBScnVertexF_IDX_color_b    16u
#define NBScnVertexF_IDX_color_a    20u
#define NBScnVertexF_SZ             24u

//--------------------
//-- STNBScnVertexTex
//--------------------

//These MACROs are useful for shader code.
#define NBScnVertexTex_IDX_x        0u
#define NBScnVertexTex_IDX_y        4u
#define NBScnVertexTex_IDX_color    8u
#define NBScnVertexTex_IDX_tex_x    12u
#define NBScnVertexTex_IDX_tex_y    16u
#define NBScnVertexTex_SZ           20u

//--------------------
//-- STNBScnVertexTexF
//--------------------
//
//These MACROs are useful for shader code.
#define NBScnVertexTexF_IDX_x        0u
#define NBScnVertexTexF_IDX_y        4u
#define NBScnVertexTexF_IDX_color_r  8u
#define NBScnVertexTexF_IDX_color_g  12u
#define NBScnVertexTexF_IDX_color_b  16u
#define NBScnVertexTexF_IDX_color_a  20u
#define NBScnVertexTexF_IDX_tex_x    24u
#define NBScnVertexTexF_IDX_tex_y    28u
#define NBScnVertexTexF_SZ           32u

//--------------------
//-- STNBScnVertexTex2
//--------------------

//These MACROs are useful for shader code.
#define NBScnVertexTex2_IDX_x      0u
#define NBScnVertexTex2_IDX_y      4u
#define NBScnVertexTex2_IDX_color  8u
#define NBScnVertexTex2_IDX_tex_x  12u
#define NBScnVertexTex2_IDX_tex_y  16u
#define NBScnVertexTex2_IDX_tex2_x 20u
#define NBScnVertexTex2_IDX_tex2_y 24u
#define NBScnVertexTex2_SZ         28u

//--------------------
//-- STNBScnVertexTex2F
//--------------------

//These MACROs are useful for shader code.
#define NBScnVertexTex2F_IDX_x        0u
#define NBScnVertexTex2F_IDX_y        4u
#define NBScnVertexTex2F_IDX_color_r  8u
#define NBScnVertexTex2F_IDX_color_g  12u
#define NBScnVertexTex2F_IDX_color_b  16u
#define NBScnVertexTex2F_IDX_color_a  20u
#define NBScnVertexTex2F_IDX_tex_x    24u
#define NBScnVertexTex2F_IDX_tex_y    28u
#define NBScnVertexTex2F_IDX_tex2_x   32u
#define NBScnVertexTex2F_IDX_tex2_y   36u
#define NBScnVertexTex2F_SZ           40u

//--------------------
//-- STNBScnVertexTex3
//--------------------

//These MACROs are useful for shader code.
#define NBScnVertexTex3_IDX_x        0u
#define NBScnVertexTex3_IDX_y        4u
#define NBScnVertexTex3_IDX_color    8u
#define NBScnVertexTex3_IDX_tex_x    12u
#define NBScnVertexTex3_IDX_tex_y    16u
#define NBScnVertexTex3_IDX_tex2_x   20u
#define NBScnVertexTex3_IDX_tex2_y   24u
#define NBScnVertexTex3_IDX_tex3_x   28u
#define NBScnVertexTex3_IDX_tex3_y   32u
#define NBScnVertexTex3_SZ           36u


//--------------------
//-- STNBScnVertexTex3F
//--------------------

//These MACROs are useful for shader code.
#define NBScnVertexTex3F_IDX_x          0u
#define NBScnVertexTex3F_IDX_y          4u
#define NBScnVertexTex3F_IDX_color_r    8u
#define NBScnVertexTex3F_IDX_color_g    12u
#define NBScnVertexTex3F_IDX_color_b    16u
#define NBScnVertexTex3F_IDX_color_a    20u
#define NBScnVertexTex3F_IDX_tex_x      24u
#define NBScnVertexTex3F_IDX_tex_y      28u
#define NBScnVertexTex3F_IDX_tex2_x     32u
#define NBScnVertexTex3F_IDX_tex2_y     36u
#define NBScnVertexTex3F_IDX_tex3_x     40u
#define NBScnVertexTex3F_IDX_tex3_y     44u
#define NBScnVertexTex3F_SZ             48u


//---------------------
//-- STNBScnTreeNode
//---------------------

//These MACROs are useful for shader code.
#define NBScnTreeNode_IDX_iParent     0u
#define NBScnTreeNode_IDX_pck         4u
#define NBScnTreeNode_IDX_t_x         8u
#define NBScnTreeNode_IDX_t_y         12u
#define NBScnTreeNode_IDX_t_deg       16u
#define NBScnTreeNode_IDX_t_sX        20u
#define NBScnTreeNode_IDX_t_sY        24u
#define NBScnTreeNode_IDX_c           28u
#define NBScnTreeNode_IDX_vs_iFirst   32u
#define NBScnTreeNode_IDX_vs_count    36u
#define NBScnTreeNode_SZ              40u

#define NB_SCN_TREE_NODE_ISHIDDEN_BIT          0x1u      //0000-0000 0000-0000 0000-0000 0000-0001b
#define NB_SCN_TREE_NODE_ISDISABLED_BIT        0x2u      //0000-0000 0000-0000 0000-0000 0000-0010b
//
#define NB_SCN_TREE_NODE_VERTS_TYPE_MSK        0x300u    //0000-0000 0000-0000 0000-0011 0000-0000b
#define NB_SCN_TREE_NODE_VERTS_TYPE_BIT_FIRST  0x100u    //0000-0000 0000-0000 0000-0001 0000-0000b
#define NB_SCN_TREE_NODE_VERTS_TYPE_BIT_LAST   0x200u    //0000-0000 0000-0000 0000-0001 0000-0000b
#define NB_SCN_TREE_NODE_VERTS_TYPE_MSK_MAX    0x3u      //
//
#define NB_SCN_TREE_NODE_CHLD_COUNT_MSK        0xFFFF0000u //1111-1111 1111-1111 0000-0000 0000-0000b
#define NB_SCN_TREE_NODE_CHLD_COUNT_BIT_FIRST  0x10000u    //0000-0000 0000-0001 0000-0000 0000-0000b
#define NB_SCN_TREE_NODE_CHLD_COUNT_BIT_LAST   0x80000000u //1000-0000 0000-0000 0000-0000 0000-0000b
#define NB_SCN_TREE_NODE_CHLD_COUNT_MSK_MAX    0xFFFFu     //

#define NB_COLOR8_R_MSK               0xFF000000u //1111-1111 0000-0000 0000-0000 0000-0000b
#define NB_COLOR8_R_BIT_FIRST         0x1000000u  //0000-0001 0000-0000 0000-0000 0000-0000b
#define NB_COLOR8_R_BIT_LAST          0x80000000u //1000-0000 0000-0000 0000-0000 0000-0000b
#define NB_COLOR8_R_MSK_MAX           0xFFu       //
//
#define NB_COLOR8_G_MSK               0xFF0000u   //0000-0000 1111-1111 0000-0000 0000-0000b
#define NB_COLOR8_G_BIT_FIRST         0x10000u    //0000-0000 0000-0001 0000-0000 0000-0000b
#define NB_COLOR8_G_BIT_LAST          0x800000u   //0000-0000 1000-0000 0000-0000 0000-0000b
#define NB_COLOR8_G_MSK_MAX           0xFFu       //
//
#define NB_COLOR8_B_MSK               0xFFFF00u   //0000-0000 0000-0000 1111-1111 0000-0000b
#define NB_COLOR8_B_BIT_FIRST         0x100u      //0000-0000 0000-0000 0000-0001 0000-0000b
#define NB_COLOR8_B_BIT_LAST          0x8000u     //0000-0000 0000-0000 1000-0000 0000-0000b
#define NB_COLOR8_B_MSK_MAX           0xFFu       //
//
#define NB_COLOR8_A_MSK               0xFFu       //0000-0000 0000-0000 0000-0000 1111-1111b
#define NB_COLOR8_A_BIT_FIRST         0x1u        //0000-0000 0000-0000 0000-0000 0000-0001b
#define NB_COLOR8_A_BIT_LAST          0x80u       //0000-0000 0000-0000 0000-0000 1000-0000b
#define NB_COLOR8_A_MSK_MAX           0xFFu       //

//using MACROS for GLSl and HLSL, which lack ~, <<, >>, &, | operator.

#define NBScnTreeNode_getIsHidden(O)        (((O) / NB_SCN_TREE_NODE_ISHIDDEN_BIT) % 2u)
#define NBScnTreeNode_getIsDisabled(O)      (((O) / NB_SCN_TREE_NODE_ISDISABLED_BIT) % 2u)
#define NBScnTreeNode_getVertsType(O)       (((O) / NB_SCN_TREE_NODE_VERTS_TYPE_BIT_FIRST) % (NB_SCN_TREE_NODE_VERTS_TYPE_MSK_MAX + 1u))
#define NBScnTreeNode_getChildCount(O)      (((O) / NB_SCN_TREE_NODE_CHLD_COUNT_BIT_FIRST) % (NB_SCN_TREE_NODE_CHLD_COUNT_MSK_MAX + 1u))
#define NBColor8_getR(O)           (((O) / NB_COLOR8_R_BIT_FIRST) % (NB_COLOR8_R_MSK_MAX + 1u))
#define NBColor8_getG(O)          (((O) / NB_COLOR8_G_BIT_FIRST) % (NB_COLOR8_G_MSK_MAX + 1u))
#define NBColor8_getB(O)          (((O) / NB_COLOR8_B_BIT_FIRST) % (NB_COLOR8_B_MSK_MAX + 1u))
#define NBColor8_getA(O)          (((O) / NB_COLOR8_A_BIT_FIRST) % (NB_COLOR8_A_MSK_MAX + 1u))

#define NBScnTreeNode_withIsHidden(B, V)    NB_APPLY_BIT(B, V, NB_SCN_TREE_NODE_ISHIDDEN_BIT)
#define NBScnTreeNode_withIsDisabled(B, V)  NB_APPLY_BIT(B, V, NB_SCN_TREE_NODE_ISDISABLED_BIT)
#define NBScnTreeNode_withVertsType(B, V)   NB_APPLY_BITS(B, V, NB_SCN_TREE_NODE_VERTS_TYPE_MSK, NB_SCN_TREE_NODE_VERTS_TYPE_BIT_FIRST, NB_SCN_TREE_NODE_VERTS_TYPE_BIT_LAST, NB_SCN_TREE_NODE_VERTS_TYPE_MSK_MAX)
#define NBScnTreeNode_withChildCount(B, V)  NB_APPLY_BITS(B, V, NB_SCN_TREE_NODE_CHLD_COUNT_MSK, NB_SCN_TREE_NODE_CHLD_COUNT_BIT_FIRST, NB_SCN_TREE_NODE_CHLD_COUNT_BIT_LAST, NB_SCN_TREE_NODE_CHLD_COUNT_MSK_MAX)

//---------------------
//-- STNBScnFlatNode
//---------------------

//These MACROs are useful for shader code.
#define NBScnFlatNode_IDX_pck        0u
#define NBScnFlatNode_IDX_m_0_0      4u
#define NBScnFlatNode_IDX_m_0_1      8u
#define NBScnFlatNode_IDX_m_0_2      12u
#define NBScnFlatNode_IDX_m_1_0      16u
#define NBScnFlatNode_IDX_m_1_1      20u
#define NBScnFlatNode_IDX_m_1_2      24u
#define NBScnFlatNode_IDX_mInv_0_0   28u
#define NBScnFlatNode_IDX_mInv_0_1   32u
#define NBScnFlatNode_IDX_mInv_0_2   36u
#define NBScnFlatNode_IDX_mInv_1_0   40u
#define NBScnFlatNode_IDX_mInv_1_1   44u
#define NBScnFlatNode_IDX_mInv_1_2   48u
#define NBScnFlatNode_IDX_c_r        52u
#define NBScnFlatNode_IDX_c_g        56u
#define NBScnFlatNode_IDX_c_b        60u
#define NBScnFlatNode_IDX_c_a        64u
#define NBScnFlatNode_SZ             68u

#define NB_SCN_FLAT_NODE_ISHIDDEN_BIT        0x1u    //0000-0000 0000-0000 0000-0000 0000-0001b
#define NB_SCN_FLAT_NODE_ISDISABLED_BIT      0x2u    //0000-0000 0000-0000 0000-0000 0000-0010b
//
#define NB_SCN_FLAT_NODE_DEEP_LVL_MSK        0xFFFF0000u//1111-1111 1111-1111 0000-0000 0000-0000b
#define NB_SCN_FLAT_NODE_DEEP_LVL_BIT_FIRST  0x10000u//0000-0000 0000-0001 0000-0000 0000-0000b
#define NB_SCN_FLAT_NODE_DEEP_LVL_BIT_LAST   0x80000000u//1000-0000 0000-0000 0000-0000 0000-0000b
#define NB_SCN_FLAT_NODE_DEEP_LVL_MSK_MAX    0xFFFFu    //

//using MACROS for GLSl and HLSL, which lack ~, <<, >>, &, | operator.

#define NBScnFlatNode_getIsHidden(O)        (((O) / NB_SCN_FLAT_NODE_ISHIDDEN_BIT) % 2)
#define NBScnFlatNode_getIsDisabled(O)      (((O) / NB_SCN_FLAT_NODE_ISDISABLED_BIT) % 2)
#define NBScnFlatNode_getDeepLvl(O)         (((O) / NB_SCN_FLAT_NODE_DEEP_LVL_BIT_FIRST) % (NB_SCN_FLAT_NODE_DEEP_LVL_MSK_MAX + 1))

#define NBScnFlatNode_withIsHidden(B, V)    NB_APPLY_BIT(B, V, NB_SCN_FLAT_NODE_ISHIDDEN_BIT)
#define NBScnFlatNode_withIsDisabled(B, V)  NB_APPLY_BIT(B, V, NB_SCN_FLAT_NODE_ISDISABLED_BIT)
#define NBScnFlatNode_withDeepLvl(B, V)     NB_APPLY_BITS(B, V, NB_SCN_FLAT_NODE_DEEP_LVL_MSK, NB_SCN_FLAT_NODE_DEEP_LVL_BIT_FIRST, NB_SCN_FLAT_NODE_DEEP_LVL_BIT_LAST, NB_SCN_FLAT_NODE_DEEP_LVL_MSK_MAX)


kernel void compute_tree(
                       //params
                       device const uint* params
                       , device const uint* src
                       , device uint* dst
                       //
                       , uint index [[thread_position_in_grid]])
{
    uint iNodeSrc   = params[NBScnRenderDispatchHeader_IDX_iNodeOffset] /*iNodeOffset*/ + index;
    //
    uint trNdsOff   = params[NBScnRenderDispatchHeader_IDX_src + NBScnRenderBuffRngs_IDX_nodes + NBScnRenderBuffRng_IDX_offset];
    uint trV0Off    = params[NBScnRenderDispatchHeader_IDX_src + NBScnRenderBuffRngs_IDX_v + NBScnRenderBuffRng_IDX_offset];
    uint trV1Off    = params[NBScnRenderDispatchHeader_IDX_src + NBScnRenderBuffRngs_IDX_v1 + NBScnRenderBuffRng_IDX_offset];
    uint trV2Off    = params[NBScnRenderDispatchHeader_IDX_src + NBScnRenderBuffRngs_IDX_v2 + NBScnRenderBuffRng_IDX_offset];
    uint trV3Off    = params[NBScnRenderDispatchHeader_IDX_src + NBScnRenderBuffRngs_IDX_v3 + NBScnRenderBuffRng_IDX_offset];
    //
    uint flNdsOff   = params[NBScnRenderDispatchHeader_IDX_dst + NBScnRenderBuffRngs_IDX_nodes + NBScnRenderBuffRng_IDX_offset];
    uint flV0Off    = params[NBScnRenderDispatchHeader_IDX_dst + NBScnRenderBuffRngs_IDX_v + NBScnRenderBuffRng_IDX_offset];
    uint flV1Off    = params[NBScnRenderDispatchHeader_IDX_dst + NBScnRenderBuffRngs_IDX_v1 + NBScnRenderBuffRng_IDX_offset];
    uint flV2Off    = params[NBScnRenderDispatchHeader_IDX_dst + NBScnRenderBuffRngs_IDX_v2 + NBScnRenderBuffRng_IDX_offset];
    uint flV3Off    = params[NBScnRenderDispatchHeader_IDX_dst + NBScnRenderBuffRngs_IDX_v3 + NBScnRenderBuffRng_IDX_offset];
    //
    uint iNode      = iNodeSrc;
    uint iDeepLvl   = 0u;
    uint iParent    = src[ ( trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_iParent ) / 4u ];
    uint pck        = src[ ( trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_pck ) / 4u ];
    uint chldCountSrc = NBScnTreeNode_getChildCount(pck);
    uint isHidden   = NBScnTreeNode_getIsHidden(pck);
    uint isDisabled = NBScnTreeNode_getIsDisabled(pck);
    float rad       = radians( as_type<float>( src[ ( trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_deg ) / 4u ] ) );
    float radSin    = sin( rad );
    float radCos    = cos( rad );
    float sx        = as_type<float>( src[ ( trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_sX ) / 4u ] );
    float sy        = as_type<float>( src[ ( trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_sY ) / 4u ] );
    uint c8         = src[ ( trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_c ) / 4u ];
    float r         = float(NBColor8_getR(c8)) / 255.f;
    float g         = float(NBColor8_getG(c8)) / 255.f;
    float b         = float(NBColor8_getB(c8)) / 255.f;
    float a         = float(NBColor8_getA(c8)) / 255.f;
    uint vsType     = NBScnTreeNode_getVertsType(pck);
    uint vsFirstIdx = src[ ( trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_vs_iFirst ) / 4u ];
    uint vsCount    = src[ ( trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_vs_count ) / 4u ];
    float mDet;
    float2x3 mP, mC, m;
    m[0][0] = radCos * sx;
    m[0][1] = -radSin * sy;
    m[0][2] = as_type<float>( src[ ( trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_x ) / 4u ] );
    m[1][0] = radSin * sx;
    m[1][1] = radCos * sy;
    m[1][2] = as_type<float>( src[ ( trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_y ) / 4u ] );
    while(iNode != iParent){
        iNode = iParent;
        iParent = src[ ( trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_iParent ) / 4u ];
        iDeepLvl++;
        pck = src[ ( trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_pck ) / 4u ];
        if(NBScnTreeNode_getIsHidden(pck) != 0u){
            isHidden = 1u;
        }
        if(NBScnTreeNode_getIsDisabled(pck) != 0u){
            isDisabled = 1u;
        }
        rad = radians( as_type<float>( src[ ( trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_deg )  / 4u ] ) );
        radSin = sin( rad );
        radCos = cos( rad );
        sx = as_type<float>( src[ ( trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_sX ) / 4u ] );
        sy = as_type<float>( src[ ( trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_sY ) / 4u ] );
        mC = m;
        mP[0][0] = radCos * sx;
        mP[0][1] = -radSin * sy;
        mP[0][2] = as_type<float>( src[ ( trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_x ) / 4u ] );
        mP[1][0] = radSin * sx;
        mP[1][1] = radCos * sy;
        mP[1][2] = as_type<float>( src[ ( trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_y ) / 4u ] );
        m[0][0] = (mP[0][0] * mC[0][0]) + (mP[0][1] * mC[1][0]) /*always-zero: + ( mP[0][2] * mC[2][0])*/;
        m[0][1] = (mP[0][0] * mC[0][1]) + (mP[0][1] * mC[1][1]) /*always-zero: + ( mP[0][2] * mC[2][1])*/;
        m[0][2] = (mP[0][0] * mC[0][2]) + (mP[0][1] * mC[1][2]) + (mP[0][2] /*always-one: * mC[2][2]*/);
        m[1][0] = (mP[1][0] * mC[0][0]) + (mP[1][1] * mC[1][0]) /*always-zero: + ( mP[1][2] * mC[2][0])*/;
        m[1][1] = (mP[1][0] * mC[0][1]) + (mP[1][1] * mC[1][1]) /*always-zero: + ( mP[1][2] * mC[2][1])*/;
        m[1][2] = (mP[1][0] * mC[0][2]) + (mP[1][1] * mC[1][2]) + (mP[1][2] /*always-one: * mC[2][2]*/);
        c8 = src[ ( trNdsOff + iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_c ) / 4u ];
        r *= float(NBColor8_getR(c8)) / 255.f;
        g *= float(NBColor8_getG(c8)) / 255.f;
        b *= float(NBColor8_getB(c8)) / 255.f;
        a *= float(NBColor8_getA(c8)) / 255.f;
    }
    //transform vertices
    if (vsCount > 0u) {
        uint vIdx, vIdxAfterLast;
        float vX, vY, vXP, vYP, vCR, vCG, vCB, vCA;
        float t1x, t1y, t2x, t2y, t3x, t3y;
        vIdxAfterLast = vsFirstIdx + vsCount;
        switch (vsType) {
            case 0:
                for (vIdx = vsFirstIdx; vIdx < vIdxAfterLast; vIdx++) {
                    vX = as_type<float>( src[ ( trV0Off + vIdx * NBScnVertex_SZ + NBScnVertex_IDX_x ) / 4u ] );
                    vY = as_type<float>( src[ ( trV0Off + vIdx * NBScnVertex_SZ + NBScnVertex_IDX_y ) / 4u ] );
                    //
                    vXP = (m[0][0] * vX) + (m[0][1] * vY) + m[0][2];
                    vYP = (m[1][0] * vX) + (m[1][1] * vY) + m[1][2];
                    //
                    c8 = src[ ( trV0Off + vIdx * NBScnVertex_SZ + NBScnVertex_IDX_color ) / 4u ];
                    vCR = r * (float(NBColor8_getR(c8)) / 255.f);
                    vCG = g * (float(NBColor8_getG(c8)) / 255.f);
                    vCB = b * (float(NBColor8_getB(c8)) / 255.f);
                    vCA = a * (float(NBColor8_getA(c8)) / 255.f);
                    //
                    dst[ ( flV0Off + vIdx * NBScnVertexF_SZ + NBScnVertexF_IDX_x) / 4u ] = as_type<uint>(vXP);
                    dst[ ( flV0Off + vIdx * NBScnVertexF_SZ + NBScnVertexF_IDX_y) / 4u ] = as_type<uint>(vYP);
                    dst[ ( flV0Off + vIdx * NBScnVertexF_SZ + NBScnVertexF_IDX_color_r) / 4u ] = as_type<uint>(vCR);
                    dst[ ( flV0Off + vIdx * NBScnVertexF_SZ + NBScnVertexF_IDX_color_g) / 4u ] = as_type<uint>(vCG);
                    dst[ ( flV0Off + vIdx * NBScnVertexF_SZ + NBScnVertexF_IDX_color_b) / 4u ] = as_type<uint>(vCB);
                    dst[ ( flV0Off + vIdx * NBScnVertexF_SZ + NBScnVertexF_IDX_color_a) / 4u ] = as_type<uint>(vCA);
                }
                break;
            case 1:
                for (vIdx = vsFirstIdx; vIdx < vIdxAfterLast; vIdx++) {
                    vX = as_type<float>( src[ ( trV1Off + vIdx * NBScnVertexTex_SZ + NBScnVertexTex_IDX_x ) / 4u ] );
                    vY = as_type<float>( src[ ( trV1Off + vIdx * NBScnVertexTex_SZ + NBScnVertexTex_IDX_y ) / 4u ] );
                    //
                    vXP = (m[0][0] * (vX)) + (m[0][1] * (vY)) + m[0][2];
                    vYP = (m[1][0] * (vX)) + (m[1][1] * (vY)) + m[1][2];
                    //
                    c8 = src[ ( trV1Off + vIdx * NBScnVertexTex_SZ + NBScnVertexTex_IDX_color ) / 4u ];
                    vCR = r * (float(NBColor8_getR(c8)) / 255.f);
                    vCG = g * (float(NBColor8_getG(c8)) / 255.f);
                    vCB = b * (float(NBColor8_getB(c8)) / 255.f);
                    vCA = a * (float(NBColor8_getA(c8)) / 255.f);
                    //
                    t1x = as_type<float>( src[ ( trV1Off + vIdx * NBScnVertexTex_SZ + NBScnVertexTex_IDX_tex_x ) / 4u ] );
                    t1y = as_type<float>( src[ ( trV1Off + vIdx * NBScnVertexTex_SZ + NBScnVertexTex_IDX_tex_y ) / 4u ] );
                    //
                    dst[ ( flV1Off + vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_x) / 4u ] = as_type<uint>(vXP);
                    dst[ ( flV1Off + vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_y) / 4u ] = as_type<uint>(vYP);
                    dst[ ( flV1Off + vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_color_r) / 4u ] = as_type<uint>(vCR);
                    dst[ ( flV1Off + vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_color_g) / 4u ] = as_type<uint>(vCG);
                    dst[ ( flV1Off + vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_color_b) / 4u ] = as_type<uint>(vCB);
                    dst[ ( flV1Off + vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_color_a) / 4u ] = as_type<uint>(vCA);
                    dst[ ( flV1Off + vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_tex_x) / 4u ] = as_type<uint>(t1x);
                    dst[ ( flV1Off + vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_tex_y) / 4u ] = as_type<uint>(t1y);
                }
                break;
            case 2:
                for (vIdx = vsFirstIdx; vIdx < vIdxAfterLast; vIdx++) {
                    vX = as_type<float>( src[ ( trV2Off + vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_x ) / 4u ] );
                    vY = as_type<float>( src[ ( trV2Off + vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_y ) / 4u ] );
                    //
                    vXP = (m[0][0] * (vX)) + (m[0][1] * (vY)) + m[0][2];
                    vYP = (m[1][0] * (vX)) + (m[1][1] * (vY)) + m[1][2];
                    //
                    c8 = src[ ( trV2Off + vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_color ) / 4u ];
                    vCR = r * (float(NBColor8_getR(c8)) / 255.f);
                    vCG = g * (float(NBColor8_getG(c8)) / 255.f);
                    vCB = b * (float(NBColor8_getB(c8)) / 255.f);
                    vCA = a * (float(NBColor8_getA(c8)) / 255.f);
                    //
                    t1x = as_type<float>( src[ ( trV2Off +vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_tex_x ) / 4u ] );
                    t1y = as_type<float>( src[ ( trV2Off +vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_tex_y ) / 4u ] );
                    t2x = as_type<float>( src[ ( trV2Off +vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_tex2_x ) / 4u ] );
                    t2y = as_type<float>( src[ ( trV2Off +vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_tex2_y ) / 4u ] );
                    //
                    dst[ ( flV2Off + vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_x) / 4u ] = as_type<uint>(vXP);
                    dst[ ( flV2Off + vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_y) / 4u ] = as_type<uint>(vYP);
                    dst[ ( flV2Off + vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_color_r) / 4u ] = as_type<uint>(vCR);
                    dst[ ( flV2Off + vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_color_g) / 4u ] = as_type<uint>(vCG);
                    dst[ ( flV2Off + vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_color_b) / 4u ] = as_type<uint>(vCB);
                    dst[ ( flV2Off + vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_color_a) / 4u ] = as_type<uint>(vCA);
                    dst[ ( flV2Off + vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_tex_x) / 4u ] = as_type<uint>(t1x);
                    dst[ ( flV2Off + vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_tex_y) / 4u ] = as_type<uint>(t1y);
                    dst[ ( flV2Off + vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_tex2_x) / 4u ] = as_type<uint>(t2x);
                    dst[ ( flV2Off + vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_tex2_y) / 4u ] = as_type<uint>(t2y);
                }
                break;
            case 3:
                for (vIdx = vsFirstIdx; vIdx < vIdxAfterLast; vIdx++) {
                    vX = as_type<float>( src[ ( trV3Off + vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_x ) / 4u ] );
                    vY = as_type<float>( src[ ( trV3Off + vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_y ) / 4u ] );
                    //
                    vXP = (m[0][0] * (vX)) + (m[0][1] * (vY)) + m[0][2];
                    vYP = (m[1][0] * (vX)) + (m[1][1] * (vY)) + m[1][2];
                    //
                    c8 = src[ ( trV3Off + vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_color ) / 4u ];
                    vCR = r * (float(NBColor8_getR(c8)) / 255.f);
                    vCG = g * (float(NBColor8_getG(c8)) / 255.f);
                    vCB = b * (float(NBColor8_getB(c8)) / 255.f);
                    vCA = a * (float(NBColor8_getA(c8)) / 255.f);
                    //
                    t1x = as_type<float>( src[ ( trV3Off + vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_tex_x ) / 4u ] );
                    t1y = as_type<float>( src[ ( trV3Off + vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_tex_y ) / 4u ] );
                    t2x = as_type<float>( src[ ( trV3Off + vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_tex2_x ) / 4u ] );
                    t2y = as_type<float>( src[ ( trV3Off + vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_tex2_y ) / 4u ] );
                    t3x = as_type<float>( src[ ( trV3Off + vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_tex3_x ) / 4u ] );
                    t3y = as_type<float>( src[ ( trV3Off + vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_tex3_y ) / 4u ] );
                    //
                    dst[ ( flV3Off + vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_x) / 4u ] = as_type<uint>(vXP);
                    dst[ ( flV3Off + vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_y) / 4u ] = as_type<uint>(vYP);
                    dst[ ( flV3Off + vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_color_r) / 4u ] = as_type<uint>(vCR);
                    dst[ ( flV3Off + vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_color_g) / 4u ] = as_type<uint>(vCG);
                    dst[ ( flV3Off + vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_color_b) / 4u ] = as_type<uint>(vCB);
                    dst[ ( flV3Off + vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_color_a) / 4u ] = as_type<uint>(vCA);
                    dst[ ( flV3Off + vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_tex_x) / 4u ] = as_type<uint>(t1x);
                    dst[ ( flV3Off + vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_tex_y) / 4u ] = as_type<uint>(t1y);
                    dst[ ( flV3Off + vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_tex2_x) / 4u ] = as_type<uint>(t2x);
                    dst[ ( flV3Off + vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_tex2_y) / 4u ] = as_type<uint>(t2y);
                    dst[ ( flV3Off + vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_tex3_x) / 4u ] = as_type<uint>(t3x);
                    dst[ ( flV3Off + vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_tex3_y) / 4u ] = as_type<uint>(t3y);
                }
                break;
            default:
                break;
        }
    }
    //save
    pck = NBScnFlatNode_withDeepLvl(0u, iDeepLvl) + NBScnFlatNode_withIsHidden(0u, isHidden) + NBScnFlatNode_withIsDisabled(0u, isDisabled);
    dst[ ( flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_pck) / 4u ] = pck;
    dst[ ( flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_m_0_0) / 4u ] = as_type<uint>(m[0][0]);
    dst[ ( flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_m_0_1) / 4u ] = as_type<uint>(m[0][1]);
    dst[ ( flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_m_0_2) / 4u ] = as_type<uint>(m[0][2]);
    dst[ ( flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_m_1_0) / 4u ] = as_type<uint>(m[1][0]);
    dst[ ( flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_m_1_1) / 4u ] = as_type<uint>(m[1][1]);
    dst[ ( flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_m_1_2) / 4u ] = as_type<uint>(m[1][2]);
    dst[ ( flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_c_r) / 4u ] = as_type<uint>(r);
    dst[ ( flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_c_g) / 4u ] = as_type<uint>(g);
    dst[ ( flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_c_b) / 4u ] = as_type<uint>(b);
    dst[ ( flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_c_a) / 4u ] = as_type<uint>(a);
    //calculate inverse matrix (last action as div-by-zero-err is posible)
    mDet = (m[0][0] * m[1][1]) - (m[0][1] * m[1][0]);
    if(mDet != 0.0f){
        mC[0][0] = ((m[1][1] /** mInv[2][2]*/) /*- (mInv[2][1] * m[1][2]) allways-zero*/) / mDet;
        mC[0][1] = (/*(m[0][2] * mInv[2][1]) allways-zero*/0.0f - (/*mInv[2][2] **/ m[0][1])) / mDet;
        mC[0][2] = ((m[0][1] * m[1][2]) - (m[1][1] * m[0][2])) / mDet;
        mC[1][0] = (/*(m[1][2] * mInv[2][0]) allways-zero*/0.0f - (/*mInv[2][2] **/ m[1][0])) / mDet;
        mC[1][1] = ((m[0][0] /** mInv[2][2]*/) /*- (mInv[2][0] * m[0][2]) allways-zero*/) / mDet;
        mC[1][2] = ((m[0][2] * m[1][0]) - (m[1][2] * m[0][0])) / mDet;
        dst[ ( flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_mInv_0_0) / 4u ] = as_type<uint>(mC[0][0]);
        dst[ ( flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_mInv_0_1) / 4u ] = as_type<uint>(mC[0][1]);
        dst[ ( flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_mInv_0_2) / 4u ] = as_type<uint>(mC[0][2]);
        dst[ ( flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_mInv_1_0) / 4u ] = as_type<uint>(mC[1][0]);
        dst[ ( flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_mInv_1_1) / 4u ] = as_type<uint>(mC[1][1]);
        dst[ ( flNdsOff + iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_mInv_1_2) / 4u ] = as_type<uint>(mC[1][2]);
    }
}
