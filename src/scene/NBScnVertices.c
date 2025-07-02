
#include "nb/NBFrameworkPch.h"
#include "nb/scene/NBScnVertices.h"
//

//NBScnVertexType

STNBEnumMapRecord NBScnVertexType_sharedEnumMapRecs[] = {
    { ENNBScnVertexType_Color, "ENNBScnVertexType_Color", "color" }
    , { ENNBScnVertexType_Tex, "ENNBScnVertexType_Tex", "tex" }
    , { ENNBScnVertexType_Tex2, "ENNBScnVertexType_Tex2", "tex2" }
    , { ENNBScnVertexType_Tex3, "ENNBScnVertexType_Tex3", "tex3" }
};

STNBEnumMap NBScnVertexType_sharedEnumMap = {
    "ENNBScnVertexType"
    , NBScnVertexType_sharedEnumMapRecs
    , (sizeof(NBScnVertexType_sharedEnumMapRecs) / sizeof(NBScnVertexType_sharedEnumMapRecs[0]))
};

const STNBEnumMap* NBScnVertexType_getSharedEnumMap(void){
    return &NBScnVertexType_sharedEnumMap;
}
