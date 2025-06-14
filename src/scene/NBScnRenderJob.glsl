//#version 330
//#version 300 es

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

#define NBScnTreeNode_getIsHidden(O)        (((O) / NB_SCN_TREE_NODE_ISHIDDEN_BIT) % 2u)
#define NBScnTreeNode_getIsDisabled(O)      (((O) / NB_SCN_TREE_NODE_ISDISABLED_BIT) % 2u)
#define NBScnTreeNode_getVertsType(O)       (((O) / NB_SCN_TREE_NODE_VERTS_TYPE_BIT_FIRST) % (NB_SCN_TREE_NODE_VERTS_TYPE_MSK_MAX + 1u))
#define NBScnTreeNode_getChildCount(O)      (((O) / NB_SCN_TREE_NODE_CHLD_COUNT_BIT_FIRST) % (NB_SCN_TREE_NODE_CHLD_COUNT_MSK_MAX + 1u))
#define NBColor8_getR(O)		   (((O) / NB_COLOR8_R_BIT_FIRST) % (NB_COLOR8_R_MSK_MAX + 1u))
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

//NOTE: std140, each struct and member of an array is aligned to 16-bytes (vec4)
//NOTE: std430, structs and array member are aligned to 4-bytes (uint)

layout(binding = 0) uniform STParams {
	uint iNodeOffset; //nodes already executed on previous Dispatch() calls
} params; 

//inputs (scene tree)
layout(std430, binding = 1) readonly buffer STNBScnTreeNode {
	uint arr[]; 
} trNds; 
layout(std430, binding = 2) readonly buffer STNBScnVertex {
	uint arr[]; 
} trV0; 
layout(std430, binding = 3) readonly buffer STNBScnVertexTex {
	uint arr[]; 
} trV1; 
layout(std430, binding = 4) readonly buffer STNBScnVertexTex2 {
	uint arr[]; 
} trV2; 
layout(std430, binding = 5) readonly buffer STNBScnVertexTex3 {
	uint arr[]; 
} trV3; 

//outputs (scene flatten)
layout(std430, binding = 6) buffer STNBScnFlatNode {
	uint arr[]; 
} flNds; 
layout(std430, binding = 7) buffer STNBScnVertexF {
	uint arr[]; 
} flV0; 
layout(std430, binding = 8) buffer STNBScnVertexTexF {
	uint arr[]; 
} flV1; 
layout(std430, binding = 9) buffer STNBScnVertexTex2F {
	uint arr[]; 
} flV2; 
layout(std430, binding = 10) buffer STNBScnVertexTex3F {
	uint arr[]; 
} flV3; 

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main(){
	uint iNodeSrc   = params.iNodeOffset + gl_GlobalInvocationID.x;
    uint iNode      = iNodeSrc;
    uint iDeepLvl   = 0u;
    uint iParent    = trNds.arr[ ( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_iParent ) / 4u ];
    uint pck        = trNds.arr[ ( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_pck ) / 4u ];
    uint chldCountSrc = NBScnTreeNode_getChildCount(pck);
    uint isHidden   = NBScnTreeNode_getIsHidden(pck);
    uint isDisabled = NBScnTreeNode_getIsDisabled(pck);
    float rad       = radians( uintBitsToFloat( trNds.arr[ ( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_deg ) / 4u ] ) );
    float radSin    = sin( rad );
    float radCos    = cos( rad );
    float sx        = uintBitsToFloat( trNds.arr[ ( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_sX ) / 4u ] );
    float sy        = uintBitsToFloat( trNds.arr[ ( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_sY ) / 4u ] );
    uint c8         = trNds.arr[ ( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_c ) / 4u ];
    float r         = float(NBColor8_getR(c8)) / 255.f;
    float g         = float(NBColor8_getG(c8)) / 255.f;
    float b         = float(NBColor8_getB(c8)) / 255.f;
    float a         = float(NBColor8_getA(c8)) / 255.f;
    uint vsType     = NBScnTreeNode_getVertsType(pck);
    uint vsFirstIdx = trNds.arr[ ( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_vs_iFirst ) / 4u ];
    uint vsCount    = trNds.arr[ ( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_vs_count ) / 4u ];
    float mDet;
    mat2x3 mP, mC, m;
    m[0][0] = radCos * sx; 
    m[0][1] = -radSin * sy; 
    m[0][2] = uintBitsToFloat( trNds.arr[ ( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_x ) / 4u ] ); 
    m[1][0] = radSin * sx; 
    m[1][1] = radCos * sy; 
    m[1][2] = uintBitsToFloat( trNds.arr[ ( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_y ) / 4u ] ); 
    while(iNode != iParent){
        iNode = iParent;
        iParent = trNds.arr[ ( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_iParent ) / 4u ];
        iDeepLvl++;
        pck = trNds.arr[ ( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_pck ) / 4u ];
        if(NBScnTreeNode_getIsHidden(pck) != 0u){
            isHidden = 1u;
        }
        if(NBScnTreeNode_getIsDisabled(pck) != 0u){
            isDisabled = 1u;
        }
        rad = radians( uintBitsToFloat( trNds.arr[ ( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_deg )  / 4u ] ) );
        radSin = sin( rad );
        radCos = cos( rad );
        sx = uintBitsToFloat( trNds.arr[ ( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_sX ) / 4u ] );
        sy = uintBitsToFloat( trNds.arr[ ( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_sY ) / 4u ] );
        mC = m;
        mP[0][0] = radCos * sx; 
        mP[0][1] = -radSin * sy; 
        mP[0][2] = uintBitsToFloat( trNds.arr[ ( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_x ) / 4u ] ); 
        mP[1][0] = radSin * sx; 
        mP[1][1] = radCos * sy; 
        mP[1][2] = uintBitsToFloat( trNds.arr[ ( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_t_y ) / 4u ] ); 
        m[0][0] = (mP[0][0] * mC[0][0]) + (mP[0][1] * mC[1][0]) /*always-zero: + ( mP[0][2] * mC[2][0])*/; 
        m[0][1] = (mP[0][0] * mC[0][1]) + (mP[0][1] * mC[1][1]) /*always-zero: + ( mP[0][2] * mC[2][1])*/; 
        m[0][2] = (mP[0][0] * mC[0][2]) + (mP[0][1] * mC[1][2]) + (mP[0][2] /*always-one: * mC[2][2]*/); 
        m[1][0] = (mP[1][0] * mC[0][0]) + (mP[1][1] * mC[1][0]) /*always-zero: + ( mP[1][2] * mC[2][0])*/; 
        m[1][1] = (mP[1][0] * mC[0][1]) + (mP[1][1] * mC[1][1]) /*always-zero: + ( mP[1][2] * mC[2][1])*/; 
        m[1][2] = (mP[1][0] * mC[0][2]) + (mP[1][1] * mC[1][2]) + (mP[1][2] /*always-one: * mC[2][2]*/); 
        c8 = trNds.arr[ ( iNode * NBScnTreeNode_SZ + NBScnTreeNode_IDX_c ) / 4u ]; 
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
                    vX = uintBitsToFloat( trV0.arr[ ( vIdx * NBScnVertex_SZ + NBScnVertex_IDX_x ) / 4u ] ); 
                    vY = uintBitsToFloat( trV0.arr[ ( vIdx * NBScnVertex_SZ + NBScnVertex_IDX_y ) / 4u ] ); 
                    //
                    vXP = (m[0][0] * vX) + (m[0][1] * vY) + m[0][2];
                    vYP = (m[1][0] * vX) + (m[1][1] * vY) + m[1][2];
                    //
                    c8 = trV0.arr[ ( vIdx * NBScnVertex_SZ + NBScnVertex_IDX_color ) / 4u ]; 
                    vCR = r * (float(NBColor8_getR(c8)) / 255.f); 
                    vCG = g * (float(NBColor8_getG(c8)) / 255.f); 
                    vCB = b * (float(NBColor8_getB(c8)) / 255.f); 
                    vCA = a * (float(NBColor8_getA(c8)) / 255.f); 
                    //
                    flV0.arr[ ( vIdx * NBScnVertexF_SZ + NBScnVertexF_IDX_x) / 4u ] = floatBitsToUint(vXP);
                    flV0.arr[ ( vIdx * NBScnVertexF_SZ + NBScnVertexF_IDX_y) / 4u ] = floatBitsToUint(vYP);
                    flV0.arr[ ( vIdx * NBScnVertexF_SZ + NBScnVertexF_IDX_color_r) / 4u ] = floatBitsToUint(vCR);
                    flV0.arr[ ( vIdx * NBScnVertexF_SZ + NBScnVertexF_IDX_color_g) / 4u ] = floatBitsToUint(vCG);
                    flV0.arr[ ( vIdx * NBScnVertexF_SZ + NBScnVertexF_IDX_color_b) / 4u ] = floatBitsToUint(vCB);
                    flV0.arr[ ( vIdx * NBScnVertexF_SZ + NBScnVertexF_IDX_color_a) / 4u ] = floatBitsToUint(vCA);
                }
                break; 
            case 1:
                for (vIdx = vsFirstIdx; vIdx < vIdxAfterLast; vIdx++) {
                    vX = uintBitsToFloat( trV1.arr[ ( vIdx * NBScnVertexTex_SZ + NBScnVertexTex_IDX_x ) / 4u ] ); 
                    vY = uintBitsToFloat( trV1.arr[ ( vIdx * NBScnVertexTex_SZ + NBScnVertexTex_IDX_y ) / 4u ] ); 
                    //
                    vXP = (m[0][0] * (vX)) + (m[0][1] * (vY)) + m[0][2];
                    vYP = (m[1][0] * (vX)) + (m[1][1] * (vY)) + m[1][2];
                    //
                    c8 = trV1.arr[ ( vIdx * NBScnVertexTex_SZ + NBScnVertexTex_IDX_color ) / 4u ]; 
                    vCR = r * (float(NBColor8_getR(c8)) / 255.f); 
                    vCG = g * (float(NBColor8_getG(c8)) / 255.f); 
                    vCB = b * (float(NBColor8_getB(c8)) / 255.f); 
                    vCA = a * (float(NBColor8_getA(c8)) / 255.f); 
                    //
                    t1x = uintBitsToFloat( trV1.arr[ ( vIdx * NBScnVertexTex_SZ + NBScnVertexTex_IDX_tex_x ) / 4u ] ); 
                    t1y = uintBitsToFloat( trV1.arr[ ( vIdx * NBScnVertexTex_SZ + NBScnVertexTex_IDX_tex_y ) / 4u ] ); 
                    //
                    flV1.arr[ ( vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_x) / 4u ] = floatBitsToUint(vXP);
                    flV1.arr[ ( vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_y) / 4u ] = floatBitsToUint(vYP);
                    flV1.arr[ ( vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_color_r) / 4u ] = floatBitsToUint(vCR);
                    flV1.arr[ ( vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_color_g) / 4u ] = floatBitsToUint(vCG);
                    flV1.arr[ ( vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_color_b) / 4u ] = floatBitsToUint(vCB);
                    flV1.arr[ ( vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_color_a) / 4u ] = floatBitsToUint(vCA);
                    flV1.arr[ ( vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_tex_x) / 4u ] = floatBitsToUint(t1x);
                    flV1.arr[ ( vIdx * NBScnVertexTexF_SZ + NBScnVertexTexF_IDX_tex_y) / 4u ] = floatBitsToUint(t1y);
                }
                break; 
            case 2:
                for (vIdx = vsFirstIdx; vIdx < vIdxAfterLast; vIdx++) {
                    vX = uintBitsToFloat( trV2.arr[ ( vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_x ) / 4u ] ); 
                    vY = uintBitsToFloat( trV2.arr[ ( vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_y ) / 4u ] ); 
                    //
                    vXP = (m[0][0] * (vX)) + (m[0][1] * (vY)) + m[0][2];
                    vYP = (m[1][0] * (vX)) + (m[1][1] * (vY)) + m[1][2];
                    //
                    c8 = trV2.arr[ ( vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_color ) / 4u ]; 
                    vCR = r * (float(NBColor8_getR(c8)) / 255.f); 
                    vCG = g * (float(NBColor8_getG(c8)) / 255.f); 
                    vCB = b * (float(NBColor8_getB(c8)) / 255.f); 
                    vCA = a * (float(NBColor8_getA(c8)) / 255.f); 
                    //
                    t1x = uintBitsToFloat( trV2.arr[ (vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_tex_x ) / 4u ] ); 
                    t1y = uintBitsToFloat( trV2.arr[ (vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_tex_y ) / 4u ] ); 
                    t2x = uintBitsToFloat( trV2.arr[ (vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_tex2_x ) / 4u ] ); 
                    t2y = uintBitsToFloat( trV2.arr[ (vIdx * NBScnVertexTex2_SZ + NBScnVertexTex2_IDX_tex2_y ) / 4u ] ); 
                    //
                    flV2.arr[ ( vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_x) / 4u ] = floatBitsToUint(vXP);
                    flV2.arr[ ( vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_y) / 4u ] = floatBitsToUint(vYP);
                    flV2.arr[ ( vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_color_r) / 4u ] = floatBitsToUint(vCR);
                    flV2.arr[ ( vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_color_g) / 4u ] = floatBitsToUint(vCG);
                    flV2.arr[ ( vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_color_b) / 4u ] = floatBitsToUint(vCB);
                    flV2.arr[ ( vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_color_a) / 4u ] = floatBitsToUint(vCA);
                    flV2.arr[ ( vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_tex_x) / 4u ] = floatBitsToUint(t1x);
                    flV2.arr[ ( vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_tex_y) / 4u ] = floatBitsToUint(t1y);
                    flV2.arr[ ( vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_tex2_x) / 4u ] = floatBitsToUint(t2x);
                    flV2.arr[ ( vIdx * NBScnVertexTex2F_SZ + NBScnVertexTex2F_IDX_tex2_y) / 4u ] = floatBitsToUint(t2y);
                }
                break; 
            case 3:
                for (vIdx = vsFirstIdx; vIdx < vIdxAfterLast; vIdx++) {
                    vX = uintBitsToFloat( trV3.arr[ ( vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_x ) / 4u ] ); 
                    vY = uintBitsToFloat( trV3.arr[ ( vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_y ) / 4u ] ); 
                    //
                    vXP = (m[0][0] * (vX)) + (m[0][1] * (vY)) + m[0][2];
                    vYP = (m[1][0] * (vX)) + (m[1][1] * (vY)) + m[1][2];
                    //
                    c8 = trV3.arr[ ( vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_color ) / 4u ]; 
                    vCR = r * (float(NBColor8_getR(c8)) / 255.f); 
                    vCG = g * (float(NBColor8_getG(c8)) / 255.f); 
                    vCB = b * (float(NBColor8_getB(c8)) / 255.f); 
                    vCA = a * (float(NBColor8_getA(c8)) / 255.f); 
                    //
                    t1x = uintBitsToFloat( trV3.arr[ ( vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_tex_x ) / 4u ] ); 
                    t1y = uintBitsToFloat( trV3.arr[ ( vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_tex_y ) / 4u ] ); 
                    t2x = uintBitsToFloat( trV3.arr[ ( vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_tex2_x ) / 4u ] ); 
                    t2y = uintBitsToFloat( trV3.arr[ ( vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_tex2_y ) / 4u ] ); 
                    t3x = uintBitsToFloat( trV3.arr[ ( vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_tex3_x ) / 4u ] ); 
                    t3y = uintBitsToFloat( trV3.arr[ ( vIdx * NBScnVertexTex3_SZ + NBScnVertexTex3_IDX_tex3_y ) / 4u ] ); 
                    //
                    flV3.arr[ ( vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_x) / 4u ] = floatBitsToUint(vXP);
                    flV3.arr[ ( vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_y) / 4u ] = floatBitsToUint(vYP);
                    flV3.arr[ ( vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_color_r) / 4u ] = floatBitsToUint(vCR);
                    flV3.arr[ ( vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_color_g) / 4u ] = floatBitsToUint(vCG);
                    flV3.arr[ ( vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_color_b) / 4u ] = floatBitsToUint(vCB);
                    flV3.arr[ ( vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_color_a) / 4u ] = floatBitsToUint(vCA);
                    flV3.arr[ ( vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_tex_x) / 4u ] = floatBitsToUint(t1x);
                    flV3.arr[ ( vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_tex_y) / 4u ] = floatBitsToUint(t1y);
                    flV3.arr[ ( vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_tex2_x) / 4u ] = floatBitsToUint(t2x);
                    flV3.arr[ ( vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_tex2_y) / 4u ] = floatBitsToUint(t2y);
                    flV3.arr[ ( vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_tex3_x) / 4u ] = floatBitsToUint(t3x);
                    flV3.arr[ ( vIdx * NBScnVertexTex3F_SZ + NBScnVertexTex3F_IDX_tex3_y) / 4u ] = floatBitsToUint(t3y);
                }
                break; 
            default:
                break; 
        }
    }
    //save
    pck = NBScnFlatNode_withDeepLvl(0u, iDeepLvl) + NBScnFlatNode_withIsHidden(0u, isHidden) + NBScnFlatNode_withIsDisabled(0u, isDisabled);
    flNds.arr[ ( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_pck) / 4u ] = pck;
    flNds.arr[ ( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_m_0_0) / 4u ] = floatBitsToUint(m[0][0]);
    flNds.arr[ ( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_m_0_1) / 4u ] = floatBitsToUint(m[0][1]);
    flNds.arr[ ( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_m_0_2) / 4u ] = floatBitsToUint(m[0][2]);
    flNds.arr[ ( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_m_1_0) / 4u ] = floatBitsToUint(m[1][0]);
    flNds.arr[ ( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_m_1_1) / 4u ] = floatBitsToUint(m[1][1]);
    flNds.arr[ ( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_m_1_2) / 4u ] = floatBitsToUint(m[1][2]);
    flNds.arr[ ( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_c_r) / 4u ] = floatBitsToUint(r);
    flNds.arr[ ( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_c_g) / 4u ] = floatBitsToUint(g);
    flNds.arr[ ( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_c_b) / 4u ] = floatBitsToUint(b);
    flNds.arr[ ( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_c_a) / 4u ] = floatBitsToUint(a);
    //calculate inverse matrix (last action as div-by-zero-err is posible)
    mDet = (m[0][0] * m[1][1]) - (m[0][1] * m[1][0]);
    if(mDet != 0.0f){
        mC[0][0] = ((m[1][1] /** mInv[2][2]*/) /*- (mInv[2][1] * m[1][2]) allways-zero*/) / mDet;
        mC[0][1] = (/*(m[0][2] * mInv[2][1]) allways-zero*/0.0f - (/*mInv[2][2] **/ m[0][1])) / mDet;
        mC[0][2] = ((m[0][1] * m[1][2]) - (m[1][1] * m[0][2])) / mDet;
        mC[1][0] = (/*(m[1][2] * mInv[2][0]) allways-zero*/0.0f - (/*mInv[2][2] **/ m[1][0])) / mDet;
        mC[1][1] = ((m[0][0] /** mInv[2][2]*/) /*- (mInv[2][0] * m[0][2]) allways-zero*/) / mDet;
        mC[1][2] = ((m[0][2] * m[1][0]) - (m[1][2] * m[0][0])) / mDet;
        flNds.arr[ ( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_mInv_0_0) / 4u ] = floatBitsToUint(mC[0][0]);
        flNds.arr[ ( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_mInv_0_1) / 4u ] = floatBitsToUint(mC[0][1]);
        flNds.arr[ ( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_mInv_0_2) / 4u ] = floatBitsToUint(mC[0][2]);
        flNds.arr[ ( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_mInv_1_0) / 4u ] = floatBitsToUint(mC[1][0]); 
        flNds.arr[ ( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_mInv_1_1) / 4u ] = floatBitsToUint(mC[1][1]); 
        flNds.arr[ ( iNodeSrc * NBScnFlatNode_SZ + NBScnFlatNode_IDX_mInv_1_2) / 4u ] = floatBitsToUint(mC[1][2]); 
    }
}
