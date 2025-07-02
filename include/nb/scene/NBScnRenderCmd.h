#ifndef NB_SCN_RENDER_JOB_CMD_H
#define NB_SCN_RENDER_JOB_CMD_H

#include "nb/NBFrameworkDefs.h"
#include "nb/scene/NBScnRenderJobDefs.h"
#include "nb/scene/NBScnVertices.h"
#include "nb/gpu/NBGpuTexture.h"

#ifdef __cplusplus
extern "C" {
#endif

//ENNBScnRenderShape

typedef enum ENNBScnRenderShape_ {
    ENNBScnRenderShape_Compute = 0, //compute vertices but not drawn
    //
    ENNBScnRenderShape_Texture,     //same as 'ENNBScnRenderShape_TriangStrip' with possible bitblit-optimization if matrix has no rotation.
    ENNBScnRenderShape_TriangStrip, //triangles-strip, most common shape
    ENNBScnRenderShape_TriangFan,   //triangles-fan
    //
    ENNBScnRenderShape_LineStrip,   //lines-strip
    ENNBScnRenderShape_LineLoop,    //lines-loop
    ENNBScnRenderShape_Lines,       //lines
    ENNBScnRenderShape_Points,      //points
    //Count
    ENNBScnRenderShape_Count
} ENNBScnRenderShape;

//ENNBScnRenderCmd

typedef enum ENNBScnRenderCmd_ {
    ENNBScnRenderCmd_None = 0,       //do nothing
    //modes
    ENNBScnRenderCmd_MaskModePush,   //pushes drawing-mask mode, where only the alpha value is affected
    ENNBScnRenderCmd_MaskModePop,    //pop
    //drawing
    ENNBScnRenderCmd_SetTexture,     //activates the texture in a specific slot-index
    ENNBScnRenderCmd_SetVertsType,   //activates the type of vertices to be used (indexes and vertices)
    ENNBScnRenderCmd_DrawVerts,      //draws something using the vertices
    ENNBScnRenderCmd_DrawIndexes,    //draws something using the vertices indexes
    //Count
    ENNBScnRenderCmd_Count
} ENNBScnRenderCmd;

//ENNBScnRenderCmd

typedef struct STNBScnRenderCmd_ {
    ENNBScnRenderCmd cmdId;          //id of the command
    union {
        //ENNBScnRenderCmd_MaskModePush
        //  nothing
        //ENNBScnRenderCmd_MaskModePop
        //  nothing
        //ENNBScnRenderCmd_SetTexture
        struct {
            UI32    index;  //slot-index
            UI32    tex; //STNBGpuTextureRef  tex;  //texture-id
        } setTexture;
        //ENNBScnRenderCmd_SetVertsType
        struct {
            ENNBScnVertexType type;
        } setVertsType;
        //ENNBScnRenderCmd_DrawVerts
        struct {
            ENNBScnRenderShape mode;
            UI32    iFirst;
            UI32    count;
        } drawVerts;
        //ENNBScnRenderCmd_DrawIndexes
        struct {
            ENNBScnRenderShape mode;
            UI32    iFirst;
            UI32    count;
        } drawIndexes;
    };
} STNBScnRenderCmd;

#ifdef __cplusplus
} //extern "C"
#endif

#endif
