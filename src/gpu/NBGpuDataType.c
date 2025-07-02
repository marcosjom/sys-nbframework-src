
#include "nb/NBFrameworkPch.h"
#include "nb/gpu/NBGpuDataType.h"
//
#include "nb/core/NBStruct.h"
#include "nb/core/NBStructMaps.h"
#include "nb/core/NBMngrStructMaps.h"

//NBGpuDataType

STNBEnumMapRecord NBGpuDataType_sharedEnumMapRecs[] = {
    { ENNBGpuDataType_SI8, "ENNBGpuDataType_SI8", "si8" }
    , { ENNBGpuDataType_UI8, "ENNBGpuDataType_UI8", "ui8" }
    , { ENNBGpuDataType_SI16, "ENNBGpuDataType_SI16", "si16" }
    , { ENNBGpuDataType_UI16, "ENNBGpuDataType_UI16", "ui16" }
    , { ENNBGpuDataType_SI32, "ENNBGpuDataType_SI32", "si32" }
    , { ENNBGpuDataType_UI32, "ENNBGpuDataType_UI32", "ui32" }
    , { ENNBGpuDataType_FLOAT32, "ENNBGpuDataType_FLOAT32", "f32" }
    , { ENNBGpuDataType_DOUBLE64, "ENNBGpuDataType_DOUBLE64", "d64" }
};

STNBEnumMap NBGpuDataType_sharedEnumMap = {
    "ENNBGpuDataType"
    , NBGpuDataType_sharedEnumMapRecs
    , (sizeof(NBGpuDataType_sharedEnumMapRecs) / sizeof(NBGpuDataType_sharedEnumMapRecs[0]))
};

const STNBEnumMap* NBGpuDataType_getSharedEnumMap(void){
    return &NBGpuDataType_sharedEnumMap;
}
