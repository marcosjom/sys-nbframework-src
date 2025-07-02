#ifndef NB_SCN_TRANSFORM_H
#define NB_SCN_TRANSFORM_H

#include "nb/NBFrameworkDefs.h"
#include "nb/NBObject.h"
//
#include "nb/core/NBStructMap.h"

#ifdef __cplusplus
extern "C" {
#endif

//NBScnRenderApiItf

#define STNBScnTransform_Zero   { 0.f, 0.f, 0.f, 0.f, 0.f }

typedef struct STNBScnTransform_ {
    //Note: keeping temporary compatibility with 'NBTransformacionesF'
    union { float sx; float escalaX; };     //scale-x
    union { float sy; float escalaY; };     //scale-y
    union { float tx; float trasladoX; };   //traslate-x
    union { float ty; float trasladoY; };   //traslate-y
    union { float deg; float rotacion; };   //rotation in degrees
} STNBScnTransform;

const STNBStructMap* NBScnTransform_getSharedStructMap(void);

#define NBTransformacionesF STNBScnTransform

/*
//Note: keeping temporary compatibility with 'NBTransformacionesF'
template <class TIPODATO>
struct NBTransformacionesP {
    TIPODATO escalaX;
    TIPODATO escalaY;
    TIPODATO trasladoX;
    TIPODATO trasladoY;
    float rotacion;
    bool operator==(const NBTransformacionesP<TIPODATO> &otra) const {
        return (escalaX==otra.escalaX && escalaY==otra.escalaY && rotacion==otra.rotacion && trasladoX==otra.trasladoX && trasladoY==otra.trasladoY);
    }
    bool operator!=(const NBTransformacionesP<TIPODATO> &otra) const {
        return !(escalaX==otra.escalaX && escalaY==otra.escalaY && rotacion==otra.rotacion && trasladoX==otra.trasladoX && trasladoY==otra.trasladoY);
    }
};

typedef NBTransformacionesP<FLOAT>    NBTransformaciones;
typedef NBTransformacionesP<FLOAT>    NBTransformacionesF;
*/

#ifdef __cplusplus
} //extern "C"
#endif

#endif
