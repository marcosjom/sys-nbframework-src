//
//Using raw-buffers instead of StructuredBuffer
//  to increase DX10 compatibility.
//
//using MACROS for GLSl and HLSL, which lack ~, <<, >>, &, | operator.

#define NB_APPLY_BIT(BASE, BVAL, MASK_BIT) \
           ((((BASE) / 2u) / MASK_BIT * MASK_BIT * 2u) + (((BVAL) % 2u) * MASK_BIT)  + ((BASE) % MASK_BIT))

#define NB_APPLY_BITS(BASE, VAL, MASK_BITS, MASK_BIT_FRST, MASK_BIT_LST, MASK_MAX_VAL) \
           (((BASE / 2u) / MASK_BIT_LST * MASK_BIT_LST * 2u) + ((VAL % (MASK_MAX_VAL + 1u)) * MASK_BIT_FRST) + ((BASE) % MASK_BIT_FRST))

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

#define NBScnTreeNode_getIsHidden(O)        (((O) / NB_SCN_TREE_NODE_ISHIDDEN_BIT) % 2)
#define NBScnTreeNode_getIsDisabled(O)      (((O) / NB_SCN_TREE_NODE_ISDISABLED_BIT) % 2)
#define NBScnTreeNode_getVertsType(O)       (((O) / NB_SCN_TREE_NODE_VERTS_TYPE_BIT_FIRST) % (NB_SCN_TREE_NODE_VERTS_TYPE_MSK_MAX + 1))
#define NBScnTreeNode_getChildCount(O)      (((O) / NB_SCN_TREE_NODE_CHLD_COUNT_BIT_FIRST) % (NB_SCN_TREE_NODE_CHLD_COUNT_MSK_MAX + 1))
#define NBColor8_getR(O)		   (((O) / NB_COLOR8_R_BIT_FIRST) % (NB_COLOR8_R_MSK_MAX + 1))
#define NBColor8_getG(O)          (((O) / NB_COLOR8_G_BIT_FIRST) % (NB_COLOR8_G_MSK_MAX + 1))
#define NBColor8_getB(O)          (((O) / NB_COLOR8_B_BIT_FIRST) % (NB_COLOR8_B_MSK_MAX + 1))
#define NBColor8_getA(O)          (((O) / NB_COLOR8_A_BIT_FIRST) % (NB_COLOR8_A_MSK_MAX + 1))

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

#define NBScnFlatNode_getIsHidden(O)        (((O) / NB_SCN_FLAT_NODE_ISHIDDEN_BIT) % 2u)
#define NBScnFlatNode_getIsDisabled(O)      (((O) / NB_SCN_FLAT_NODE_ISDISABLED_BIT) % 2u)
#define NBScnFlatNode_getDeepLvl(O)         (((O) / NB_SCN_FLAT_NODE_DEEP_LVL_BIT_FIRST) % (NB_SCN_FLAT_NODE_DEEP_LVL_MSK_MAX + 1u))

#define NBScnFlatNode_withIsHidden(B, V)    NB_APPLY_BIT(B, V, NB_SCN_FLAT_NODE_ISHIDDEN_BIT)
#define NBScnFlatNode_withIsDisabled(B, V)  NB_APPLY_BIT(B, V, NB_SCN_FLAT_NODE_ISDISABLED_BIT)
#define NBScnFlatNode_withDeepLvl(B, V)     NB_APPLY_BITS(B, V, NB_SCN_FLAT_NODE_DEEP_LVL_MSK, NB_SCN_FLAT_NODE_DEEP_LVL_BIT_FIRST, NB_SCN_FLAT_NODE_DEEP_LVL_BIT_LAST, NB_SCN_FLAT_NODE_DEEP_LVL_MSK_MAX)
//
cbuffer ExecConstantBuffer : register(b0)
{
   uint iNodeOffset; //nodes already executed on previous Dispatch() calls
};
//
//inputs (scene tree)
ByteAddressBuffer trNds : register(t0);
ByteAddressBuffer trV0 : register(t1);
ByteAddressBuffer trV1 : register(t2);
ByteAddressBuffer trV2 : register(t3);
ByteAddressBuffer trV3 : register(t4);
//
//outputs (scene flatten)
RWByteAddressBuffer flNds : register(u0);
RWByteAddressBuffer flV0 : register(u1);
RWByteAddressBuffer flV1 : register(u2);
RWByteAddressBuffer flV2 : register(u3);
RWByteAddressBuffer flV3 : register(u4);
//
[numthreads(1, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
    uint iNodeSrc   = iNodeOffset + DTid.x;
    uint iNode      = iNodeSrc;
    uint iDeepLvl   = 0u;
    uint iParent    = asuint( trNds.Load( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_iParent ) );
    uint pck        = asuint( trNds.Load( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_pck ) );
    uint chldCountSrc = NBScnTreeNode_getChildCount(pck);
    uint isHidden   = NBScnTreeNode_getIsHidden(pck);
    uint isDisabled = NBScnTreeNode_getIsDisabled(pck);
    float rad       = radians( asfloat( trNds.Load( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_deg ) ) );
    float radSin    = sin( rad );
    float radCos    = cos( rad );
    float sx        = asfloat( trNds.Load( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_sX ) );
    float sy        = asfloat( trNds.Load( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_sY ) );
    uint c8         = asuint( trNds.Load( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_c ) );
    float r         = (float)NBColor8_getR(c8) / 255.f;
    float g         = (float)NBColor8_getG(c8) / 255.f;
    float b         = (float)NBColor8_getB(c8) / 255.f;
    float a         = (float)NBColor8_getA(c8) / 255.f;
    uint vsType     = NBScnTreeNode_getVertsType(pck);
    uint vsFirstIdx = asuint( trNds.Load( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_vs_iFirst ) );
    uint vsCount    = asuint( trNds.Load( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_vs_count ) );
    float mDet;
    float2x3 mP, mC, m;
    m._m00 = radCos * sx; 
    m._m01 = -radSin * sy; 
    m._m02 = asfloat( trNds.Load( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_x ) ); 
    m._m10 = radSin * sx; 
    m._m11 = radCos * sy; 
    m._m12 = asfloat( trNds.Load( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_y ) ); 
    while(iNode != iParent){
        iNode = iParent;
        iParent = asuint( trNds.Load( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_iParent ) );
        iDeepLvl++;
        pck = asuint( trNds.Load( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_pck ) );
        if(NBScnTreeNode_getIsHidden(pck) != 0u){
            isHidden = 1u;
        }
        if(NBScnTreeNode_getIsDisabled(pck) != 0u){
            isDisabled = 1u;
        }
        rad = radians( asfloat( trNds.Load( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_deg ) ) );
        radSin = sin( rad );
        radCos = cos( rad );
        sx = asfloat( trNds.Load( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_sX ) );
        sy = asfloat( trNds.Load( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_sY ) );
        mC = m;
        mP._m00 = radCos * sx; 
        mP._m01 = -radSin * sy; 
        mP._m02 = asfloat( trNds.Load( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_x ) ); 
        mP._m10 = radSin * sx; 
        mP._m11 = radCos * sy; 
        mP._m12 = asfloat( trNds.Load( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_y ) ); 
        m._m00 = (mP._m00 * mC._m00) + (mP._m01 * mC._m10) /*always-zero: + ( mP._m02 * mC._m20)*/; 
        m._m01 = (mP._m00 * mC._m01) + (mP._m01 * mC._m11) /*always-zero: + ( mP._m02 * mC._m21)*/; 
        m._m02 = (mP._m00 * mC._m02) + (mP._m01 * mC._m12) + (mP._m02 /*always-one: * mC._m22*/); 
        m._m10 = (mP._m10 * mC._m00) + (mP._m11 * mC._m10) /*always-zero: + ( mP._m12 * mC._m20)*/; 
        m._m11 = (mP._m10 * mC._m01) + (mP._m11 * mC._m11) /*always-zero: + ( mP._m12 * mC._m21)*/; 
        m._m12 = (mP._m10 * mC._m02) + (mP._m11 * mC._m12) + (mP._m12 /*always-one: * mC._m22*/); 
        c8 = asuint( trNds.Load( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_c ) ); 
        r *= (float)NBColor8_getR(c8) / 255.f;
        g *= (float)NBColor8_getG(c8) / 255.f;
        b *= (float)NBColor8_getB(c8) / 255.f;
        a *= (float)NBColor8_getA(c8) / 255.f;
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
                    vX = asfloat( trV0.Load( vIdx * NBScnVertex_SZ + NBScnVertex_IDX_x ) ); 
                    vY = asfloat( trV0.Load( vIdx * NBScnVertex_SZ + NBScnVertex_IDX_y ) ); 
                    //
                    vXP = (m._m00 * vX) + (m._m01 * vY) + m._m02;
                    vYP = (m._m10 * vX) + (m._m11 * vY) + m._m12;
                    //
                    c8 = asuint( trV0.Load( vIdx * NBScnVertex_SZ + NBScnVertex_IDX_color ) ); 
                    vCR = r * ((float)NBColor8_getR(c8) / 255.f); 
                    vCG = g * ((float)NBColor8_getG(c8) / 255.f); 
                    vCB = b * ((float)NBColor8_getB(c8) / 255.f); 
                    vCA = a * ((float)NBColor8_getA(c8) / 255.f); 
                    //
                    flV0.Store( vIdx * NBScnVertexF_SZ + NBScnVertexF_IDX_x, asuint(vXP) );
                    flV0.Store( vIdx * NBScnVertexF_SZ + NBScnVertexF_IDX_y, asuint(vYP) );
                    flV0.Store( vIdx * NBScnVertexF_SZ + NBScnVertexF_IDX_color_r, asuint(vCR) );
                    flV0.Store( vIdx * NBScnVertexF_SZ + NBScnVertexF_IDX_color_g, asuint(vCG) );
                    flV0.Store( vIdx * NBScnVertexF_SZ + NBScnVertexF_IDX_color_b, asuint(vCB) );
                    flV0.Store( vIdx * NBScnVertexF_SZ + NBScnVertexF_IDX_color_a, asuint(vCA) );
                }
                break; 
            case 1:
                for (vIdx = vsFirstIdx; vIdx < vIdxAfterLast; vIdx++) {
                    vX = asfloat( trV1.Load( vIdx * NBScnVertexTex_SZ + NBScnVertexTex_IDX_x ) ); 
                    vY = asfloat( trV1.Load( vIdx * NBScnVertexTex_SZ + NBScnVertexTex_IDX_y ) ); 
                    //
                    vXP = (m._m00 * (vX)) + (m._m01 * (vY)) + m._m02;
                    vYP = (m._m10 * (vX)) + (m._m11 * (vY)) + m._m12;
                    //
                    c8 = asuint( trV1.Load( vIdx * NBScnVertexTex_SZ + NBScnVertexTex_IDX_color ) ); 
                    vCR = r * ((float)NBColor8_getR(c8) / 255.f); 
                    vCG = g * ((float)NBColor8_getG(c8) / 255.f); 
                    vCB = b * ((float)NBColor8_getB(c8) / 255.f); 
                    vCA = a * ((float)NBColor8_getA(c8) / 255.f); 
                    //
                    t1x = asfloat( trV1.Load( vIdx * NBScnVertexTex_SZ + NBScnVertexTex_IDX_tex_x ) ); 
                    t1y = asfloat( trV1.Load( vIdx * NBScnVertexTex_SZ + NBScnVertexTex_IDX_tex_y ) ); 
                    //
                    flV1.Store( vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_x, asuint(vXP) );
                    flV1.Store( vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_y, asuint(vYP) );
                    flV1.Store( vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_color_r, asuint(vCR) );
                    flV1.Store( vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_color_g, asuint(vCG) );
                    flV1.Store( vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_color_b, asuint(vCB) );
                    flV1.Store( vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_color_a, asuint(vCA) );
                    flV1.Store( vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_tex_x, asuint(t1x) );
                    flV1.Store( vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_tex_y, asuint(t1y) );
                }
                break; 
            case 2:
                for (vIdx = vsFirstIdx; vIdx < vIdxAfterLast; vIdx++) {
                    vX = asfloat( trV2.Load( vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_x ) ); 
                    vY = asfloat( trV2.Load( vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_y ) ); 
                    //
                    vXP = (m._m00 * (vX)) + (m._m01 * (vY)) + m._m02;
                    vYP = (m._m10 * (vX)) + (m._m11 * (vY)) + m._m12;
                    //
                    c8 = asuint( trV2.Load( vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_color ) ); 
                    vCR = r * ((float)NBColor8_getR(c8) / 255.f); 
                    vCG = g * ((float)NBColor8_getG(c8) / 255.f); 
                    vCB = b * ((float)NBColor8_getB(c8) / 255.f); 
                    vCA = a * ((float)NBColor8_getA(c8) / 255.f); 
                    //
                    t1x = asfloat( trV2.Load(vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_tex_x ) ); 
                    t1y = asfloat( trV2.Load(vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_tex_y ) ); 
                    t2x = asfloat( trV2.Load(vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_tex2_x ) ); 
                    t2y = asfloat( trV2.Load(vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_tex2_y ) ); 
                    //
                    flV2.Store( vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_x, asuint(vXP) );
                    flV2.Store( vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_y, asuint(vYP) );
                    flV2.Store( vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_color_r, asuint(vCR) );
                    flV2.Store( vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_color_g, asuint(vCG) );
                    flV2.Store( vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_color_b, asuint(vCB) );
                    flV2.Store( vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_color_a, asuint(vCA) );
                    flV2.Store( vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_tex_x, asuint(t1x) );
                    flV2.Store( vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_tex_y, asuint(t1y) );
                    flV2.Store( vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_tex2_x, asuint(t2x) );
                    flV2.Store( vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_tex2_y, asuint(t2y) );
                }
                break; 
            case 3:
                for (vIdx = vsFirstIdx; vIdx < vIdxAfterLast; vIdx++) {
                    vX = asfloat( trV3.Load( vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_x ) ); 
                    vY = asfloat( trV3.Load( vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_y ) ); 
                    //
                    vXP = (m._m00 * (vX)) + (m._m01 * (vY)) + m._m02;
                    vYP = (m._m10 * (vX)) + (m._m11 * (vY)) + m._m12;
                    //
                    c8 = asuint( trV3.Load( vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_color ) ); 
                    vCR = r * ((float)NBColor8_getR(c8) / 255.f); 
                    vCG = g * ((float)NBColor8_getG(c8) / 255.f); 
                    vCB = b * ((float)NBColor8_getB(c8) / 255.f); 
                    vCA = a * ((float)NBColor8_getA(c8) / 255.f); 
                    //
                    t1x = asfloat( trV3.Load( vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_tex_x ) ); 
                    t1y = asfloat( trV3.Load( vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_tex_y ) ); 
                    t2x = asfloat( trV3.Load( vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_tex2_x ) ); 
                    t2y = asfloat( trV3.Load( vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_tex2_y ) ); 
                    t3x = asfloat( trV3.Load( vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_tex3_x ) ); 
                    t3y = asfloat( trV3.Load( vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_tex3_y ) ); 
                    //
                    flV3.Store( vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_x, asuint(vXP) );
                    flV3.Store( vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_y, asuint(vYP) );
                    flV3.Store( vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_color_r, asuint(vCR) );
                    flV3.Store( vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_color_g, asuint(vCG) );
                    flV3.Store( vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_color_b, asuint(vCB) );
                    flV3.Store( vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_color_a, asuint(vCA) );
                    flV3.Store( vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_tex_x, asuint(t1x) );
                    flV3.Store( vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_tex_y, asuint(t1y) );
                    flV3.Store( vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_tex2_x, asuint(t2x) );
                    flV3.Store( vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_tex2_y, asuint(t2y) );
                    flV3.Store( vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_tex3_x, asuint(t3x) );
                    flV3.Store( vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_tex3_y, asuint(t3y) );
                }
                break; 
            default:
                break; 
        }
    }
    //save
    pck = NBScnFlatNode_withDeepLvl(0u, iDeepLvl) + NBScnFlatNode_withIsHidden(0u, isHidden) + NBScnFlatNode_withIsDisabled(0u, isDisabled);
    flNds.Store( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_pck, asuint(pck) );
    flNds.Store( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_m_0_0, asuint(m._m00) );
    flNds.Store( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_m_0_1, asuint(m._m01) );
    flNds.Store( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_m_0_2, asuint(m._m02) );
    flNds.Store( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_m_1_0, asuint(m._m10) );
    flNds.Store( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_m_1_1, asuint(m._m11) );
    flNds.Store( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_m_1_2, asuint(m._m12) );
    flNds.Store( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_c_r, asuint(r) );
    flNds.Store( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_c_g, asuint(g) );
    flNds.Store( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_c_b, asuint(b) );
    flNds.Store( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_c_a, asuint(a) );
    //calculate inverse matrix (last action as div-by-zero-err is posible)
    mDet = (m._m00 * m._m11) - (m._m01 * m._m10);
    if(mDet != 0.0f){
        mC._m00 = ((m._m11 /** mInv._m22*/) /*- (mInv._m21 * m._m12) allways-zero*/) / mDet;
        mC._m01 = (/*(m._m02 * mInv._m21) allways-zero*/0.0f - (/*mInv._m22 **/ m._m01)) / mDet;
        mC._m02 = ((m._m01 * m._m12) - (m._m11 * m._m02)) / mDet;
        mC._m10 = (/*(m._m12 * mInv._m20) allways-zero*/0.0f - (/*mInv._m22 **/ m._m10)) / mDet;
        mC._m11 = ((m._m00 /** mInv._m22*/) /*- (mInv._m20 * m._m02) allways-zero*/) / mDet;
        mC._m12 = ((m._m02 * m._m10) - (m._m12 * m._m00)) / mDet;
        flNds.Store( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_mInv_0_0, asuint(mC._m00) );
        flNds.Store( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_mInv_0_1, asuint(mC._m01) );
        flNds.Store( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_mInv_0_2, asuint(mC._m02) );
        flNds.Store( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_mInv_1_0, asuint(mC._m10) ); 
        flNds.Store( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_mInv_1_1, asuint(mC._m11) ); 
        flNds.Store( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_mInv_1_2, asuint(mC._m12) ); 
    }
};
