#ifndef NB_GPU_DATA_TYPES_H
#define NB_GPU_DATA_TYPES_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
#include "nb/core/NBStructMap.h"

#ifdef __cplusplus
extern "C" {
#endif

//ENNBGpuTextureIdx

typedef enum ENNBGpuDataType_ {
    ENNBGpuDataType_SI8 = 0,
    ENNBGpuDataType_UI8,
    ENNBGpuDataType_SI16,
    ENNBGpuDataType_UI16,
    ENNBGpuDataType_SI32,
    ENNBGpuDataType_UI32,
    ENNBGpuDataType_FLOAT32,
    ENNBGpuDataType_DOUBLE64,
    //Count
    ENNBGpuDataType_Count
} ENNBGpuDataType;

const STNBEnumMap* NBGpuDataType_getSharedEnumMap(void);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
