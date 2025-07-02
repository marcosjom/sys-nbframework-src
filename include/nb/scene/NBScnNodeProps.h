#ifndef NB_SCN_OBJ_PROPS_H
#define NB_SCN_OBJ_PROPS_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
//
#include "nb/core/NBStructMap.h"
#include "nb/2d/NBColor.h"
#include "nb/scene/NBScnTransform.h"

#ifdef __cplusplus
extern "C" {
#endif

//STNBScnNodeProps

#define STNBScnNodeProps_Zero   { STNBColor8_Zero, STNBScnTransform_Zero }

typedef struct STNBScnNodeProps_ {
    //Note: keeping temporary compatibility with 'NBPropiedadesEnEscena'
    //union { BOOL isVisible; BOOL visible; };    //ToDo: invert to isHidden/isNotVisible
    union { STNBColor8 c8; STNBColor8 color8;};
    union { STNBScnTransform tform; STNBScnTransform transformaciones;};
} STNBScnNodeProps;

const STNBStructMap* NBScnNodeProps_getSharedStructMap(void);

#define NBPropiedadesEnEscena STNBScnNodeProps
/*
//Note: keeping temporary compatibility with 'NBPropiedadesEnEscena'
struct NBPropiedadesEnEscena {
    bool                visible;
    NBColor8            color8;
    NBTransformacionesF    transformaciones;
    bool operator==(const NBPropiedadesEnEscena &otro) const {
        return (visible==otro.visible && transformaciones==otro.transformaciones && color8==otro.color8);
    }
    bool operator!=(const NBPropiedadesEnEscena &otro) const {
        return !(visible==otro.visible && transformaciones==otro.transformaciones && color8==otro.color8);
    }
};
*/

#ifdef __cplusplus
} //extern "C"
#endif

#endif
