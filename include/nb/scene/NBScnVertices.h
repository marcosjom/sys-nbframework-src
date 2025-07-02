#ifndef NB_SCN_VERTICES_H
#define NB_SCN_VERTICES_H

#include "nb/NBFrameworkDefs.h"
#include "nb/scene/NBScnRenderJobDefs.h"
#include "nb/2d/NBColor.h"
#include "nb/2d/NBPoint.h"

#ifdef __cplusplus
extern "C" {
#endif

//ENNBScnVertexType

typedef enum ENNBScnVertexType_ {
    ENNBScnVertexType_Color = 0,    //no texture
    ENNBScnVertexType_Tex,          //one texture
    ENNBScnVertexType_Tex2,         //two textures
    ENNBScnVertexType_Tex3,         //three textures
    //
    ENNBScnVertexType_Count
} ENNBScnVertexType;

const STNBEnumMap* NBScnVertexType_getSharedEnumMap(void);

//-------------------
//-- STNBScnVertexIdx
//-------------------

//These MACROs are useful for shader code.
#define NBScnVertexIdx_IDX_idx    0u
#define NBScnVertexIdx_SZ         4u

#define STNBScnVertexIdx_Zero     { 0 }

//Vertex without texture
typedef struct STNBScnVertexIdx_ {
    UI32        i;
} STNBScnVertexIdx;

//-------------------
//-- STNBScnVertex
//-------------------

//These MACROs are useful for shader code.
#define NBScnVertex_IDX_x           0u
#define NBScnVertex_IDX_y           4u
#define NBScnVertex_IDX_color       8u
#define NBScnVertex_SZ              12u

#define STNBScnVertex_Zero     { 0.f, 0.f, STNBColor8_Zero }

//Vertex without texture
typedef struct STNBScnVertex_ {
    float        x;
    float        y;
    STNBColor8   color;
} STNBScnVertex;

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

#define STNBScnVertexTex_Zero     { 0.f, 0.f, STNBColor8_Zero, STNBPoint_Zero }

//Vertex with one texture
typedef struct STNBScnVertexTex_ {
    float        x;
    float        y;
    STNBColor8   color;
    STNBPoint    tex;
} STNBScnVertexTex;


//--------------------
//-- STNBScnVertexTex2
//--------------------

//These MACROs are useful for shader code.
#define NBScnVertexTex2_IDX_x       0u
#define NBScnVertexTex2_IDX_y       4u
#define NBScnVertexTex2_IDX_color   8u
#define NBScnVertexTex2_IDX_tex_x   12u
#define NBScnVertexTex2_IDX_tex_y   16u
#define NBScnVertexTex2_IDX_tex2_x  20u
#define NBScnVertexTex2_IDX_tex2_y  24u
#define NBScnVertexTex2_SZ          28u

#define STNBScnVertexTex2_Zero     { 0.f, 0.f, STNBColor8_Zero, STNBPoint_Zero, STNBPoint_Zero }

//Vertex with 2 textures
typedef struct STNBScnVertexTex2_ {
    float        x;
    float        y;
    STNBColor8   color;
    STNBPoint    tex;
    STNBPoint    tex2;
} STNBScnVertexTex2;

//--------------------
//-- STNBScnVertexTex3
//--------------------

//These MACROs are useful for shader code.
#define NBScnVertexTex3_IDX_x       0u
#define NBScnVertexTex3_IDX_y       4u
#define NBScnVertexTex3_IDX_color   8u
#define NBScnVertexTex3_IDX_tex_x   12u
#define NBScnVertexTex3_IDX_tex_y   16u
#define NBScnVertexTex3_IDX_tex2_x  20u
#define NBScnVertexTex3_IDX_tex2_y  24u
#define NBScnVertexTex3_IDX_tex3_x  28u
#define NBScnVertexTex3_IDX_tex3_y  32u
#define NBScnVertexTex3_SZ          36u

#define STNBScnVertexTex3_Zero     { 0.f, 0.f, STNBColor8_Zero, STNBPoint_Zero, STNBPoint_Zero, STNBPoint_Zero }

//Vertex with 3 textures
typedef struct STNBScnVertexTex3_ {
    float        x;
    float        y;
    STNBColor8   color;
    STNBPoint    tex;
    STNBPoint    tex2;
    STNBPoint    tex3;
} STNBScnVertexTex3;


//

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

//Vertex without texture
typedef struct STNBScnVertexF_ {
    float        x;
    float        y;
    STNBColor    color;
} STNBScnVertexF;

//--------------------
//-- STNBScnVertexTexF
//--------------------
//
//These MACROs are useful for shader code.
#define NBScnVertexTexF_IDX_x           0u
#define NBScnVertexTexF_IDX_y           4u
#define NBScnVertexTexF_IDX_color_r     8u
#define NBScnVertexTexF_IDX_color_g     12u
#define NBScnVertexTexF_IDX_color_b     16u
#define NBScnVertexTexF_IDX_color_a     20u
#define NBScnVertexTexF_IDX_tex_x       24u
#define NBScnVertexTexF_IDX_tex_y       28u
#define NBScnVertexTexF_SZ              32u

//Vertex with one texture
typedef struct STNBScnVertexTexF_ {
    float        x;
    float        y;
    STNBColor    color;
    STNBPoint    tex;
} STNBScnVertexTexF;


//--------------------
//-- STNBScnVertexTex2F
//--------------------

//These MACROs are useful for shader code.
#define NBScnVertexTex2F_IDX_x          0u
#define NBScnVertexTex2F_IDX_y          4u
#define NBScnVertexTex2F_IDX_color_r    8u
#define NBScnVertexTex2F_IDX_color_g    12u
#define NBScnVertexTex2F_IDX_color_b    16u
#define NBScnVertexTex2F_IDX_color_a    20u
#define NBScnVertexTex2F_IDX_tex_x      24u
#define NBScnVertexTex2F_IDX_tex_y      28u
#define NBScnVertexTex2F_IDX_tex2_x     32u
#define NBScnVertexTex2F_IDX_tex2_y     36u
#define NBScnVertexTex2F_SZ             40u

//Vertex with 2 textures
typedef struct STNBScnVertexTex2F_ {
    float        x;
    float        y;
    STNBColor    color;
    STNBPoint    tex;
    STNBPoint    tex2;
} STNBScnVertexTex2F;


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

//Vertex with 3 textures
typedef struct STNBScnVertexTex3F_ {
    float        x;
    float        y;
    STNBColor    color;
    STNBPoint    tex;
    STNBPoint    tex2;
    STNBPoint    tex3;
} STNBScnVertexTex3F;


#ifdef __cplusplus
} //extern "C"
#endif

#endif
