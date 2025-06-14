//
//  NBPoint.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/6/18.
//

#ifndef NBPoint_h
#define NBPoint_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBCompare.h"
#include "nb/core/NBStructMap.h"
//
#include <math.h> 	//for sqrt(), sin(), cos(), etc...

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBPoint_ {
		float	x;
		float	y;
	} STNBPoint;
	
	const STNBStructMap* NBPoint_getSharedStructMap(void);
	
	typedef struct STNBPointI_ {
		SI32	x;
		SI32	y;
	} STNBPointI;
	
	const STNBStructMap* NBPointI_getSharedStructMap(void);
	
	//
	
	BOOL NBCompare_NBPoint(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);
	BOOL NBCompare_NBPointI(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);
	
	//  PRODUCTO CRUZ (nota: el producto cruz con un vectorUnitario determina la distancia-perpendicular-izquierda del otro vector hacia el unitario)
	//  +(modA*modB)
	//        |
	// 0 -----+-----> 0  VECTOR_A
	//        |
	//  -(modA*modB)
#	define NBPOINT_CROSS_PRODUCT(V1_X, V1_Y, V2_X, V2_Y) (( (V1_X) * (V2_Y) ) - ( (V2_X) * (V1_Y) ))

#	define NBVECTOR_RADIANS(VAR_DEST, INI_X, INI_Y, FIN_X, FIN_Y)	{ \
float __xVectBsqAngulo = ((FIN_X)-(INI_X)); \
float __yVectBsqAngulo = ((FIN_Y)-(INI_Y)); \
VAR_DEST = 0.0f; \
if(__xVectBsqAngulo==0.0f){ \
	if(__yVectBsqAngulo<0.0f) VAR_DEST = 3.0f*PI/2.0f; \
	if(__yVectBsqAngulo>0.0f) VAR_DEST = PI/2.0f; \
} else { \
	VAR_DEST = atan(__yVectBsqAngulo/__xVectBsqAngulo); \
	if(__xVectBsqAngulo<0.0f) VAR_DEST += PI; \
} \
while(VAR_DEST<0.0f) VAR_DEST += PIx2; \
while(VAR_DEST>=PIx2) VAR_DEST -= PIx2; \
/*if((VAR_DEST>PI && VAR_DEST<3.0f*PI/2.0f) || (VAR_DEST>0.0f && VAR_DEST<PI/2.0f)) PRINTF_INFO("Angulo: %f\n", VAR_DEST);*/ \
}

#ifdef sqrtf
#	define NBVECTOR_DISTANCE_SQRD(X_FIN, Y_FIN)		(((X_FIN)*(X_FIN)) + ((Y_FIN)*(Y_FIN)))
#else
#	define NBVECTOR_DISTANCE_SQRD(X_FIN, Y_FIN)		(((X_FIN)*(X_FIN)) + ((Y_FIN)*(Y_FIN)))
#endif

#ifdef sqrtf
#	define NBVECTOR_DISTANCE(X_FIN, Y_FIN)		(sqrtf((NBVECTOR_DISTANCE_SQRD(X_FIN, Y_FIN))))
#else
#	define NBVECTOR_DISTANCE(X_FIN, Y_FIN)		(sqrt((NBVECTOR_DISTANCE_SQRD(X_FIN, Y_FIN))))
#endif

#	define NBVECTOR_ROTATE_RADIANS(P_DESTINO, P_X, P_Y, RADIANES)		{ \
float __senRotPto	= sin(RADIANES); \
float __cosRotPto	= cos(RADIANES); \
float __xRotPto 	= (P_X * __cosRotPto) - (P_Y * __senRotPto); \
float __yRotPto		= (P_X * __senRotPto) + (P_Y * __cosRotPto); \
P_DESTINO.x			= __xRotPto; \
P_DESTINO.y			= __yRotPto; \
}
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBPoint_h */
