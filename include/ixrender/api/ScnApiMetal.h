//
//  ScnApiMetal.h
//  ixtli-render
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnApiMetal_h
#define ScnApiMetal_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/gpu/ScnGpuDevice.h"
#include "ixrender/ScnRender.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Retrieves the interface for Apple's Metal render API.
 * @param dst Destination for the interface.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnApiMetal_getApiItf(STScnApiItf* dst);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnApiMetal_h */
