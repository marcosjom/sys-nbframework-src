//
//  NBPoint.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/6/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/2d/NBPoint.h"
//
#include "math.h"	//for sqrt
#include "nb/core/NBMemory.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBMngrStructMaps.h"

STNBStructMapsRec NBPoint_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBPoint_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBPoint_sharedStructMap);
	if(NBPoint_sharedStructMap.map == NULL){
		STNBPoint s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBPoint);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addFloatM(map, s, x);
		NBStructMap_addFloatM(map, s, y);
		NBPoint_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBPoint_sharedStructMap);
	return NBPoint_sharedStructMap.map;
}

STNBStructMapsRec NBPointI_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBPointI_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBPointI_sharedStructMap);
	if(NBPointI_sharedStructMap.map == NULL){
		STNBPointI s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBPointI);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addIntM(map, s, x);
		NBStructMap_addIntM(map, s, y);
		NBPointI_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBPointI_sharedStructMap);
	return NBPointI_sharedStructMap.map;
}

BOOL NBCompare_NBPoint(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBPoint))
	if(dataSz == sizeof(STNBPoint)){
		const STNBPoint* o1 = (const STNBPoint*)data1;
		const STNBPoint* o2 = (const STNBPoint*)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return (o1->x == o2->x && o1->y == o2->y) ? TRUE : FALSE;
			case ENCompareMode_Lower:
				return (o1->x < o2->x || (o1->x == o2->x && o1->y < o2->y)) ? TRUE : FALSE;
			case ENCompareMode_LowerOrEqual:
				return (o1->x < o2->x || (o1->x == o2->x && o1->y <= o2->y)) ? TRUE : FALSE;
			case ENCompareMode_Greater:
				return (o1->x > o2->x || (o1->x == o2->x && o1->y > o2->y)) ? TRUE : FALSE;
			case ENCompareMode_GreaterOrEqual:
				return (o1->x > o2->x || (o1->x == o2->x && o1->y >= o2->y)) ? TRUE : FALSE;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}

BOOL NBCompare_NBPointI(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBPointI))
	if(dataSz == sizeof(STNBPointI)){
		const STNBPointI* o1 = (const STNBPointI*)data1;
		const STNBPointI* o2 = (const STNBPointI*)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return (o1->x == o2->x && o1->y == o2->y) ? TRUE : FALSE;
			case ENCompareMode_Lower:
				return (o1->x < o2->x || (o1->x == o2->x && o1->y < o2->y)) ? TRUE : FALSE;
			case ENCompareMode_LowerOrEqual:
				return (o1->x < o2->x || (o1->x == o2->x && o1->y <= o2->y)) ? TRUE : FALSE;
			case ENCompareMode_Greater:
				return (o1->x > o2->x || (o1->x == o2->x && o1->y > o2->y)) ? TRUE : FALSE;
			case ENCompareMode_GreaterOrEqual:
				return (o1->x > o2->x || (o1->x == o2->x && o1->y >= o2->y)) ? TRUE : FALSE;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}

//

/*float NBPoint_vectorLen(const STNBPoint* obj){
#	ifdef sqrtf
	return sqrtf(((obj->x) * (obj->x)) + ((obj->y) * (obj->y)));
#	else
	return sqrt(((obj->x) * (obj->x)) + ((obj->y) * (obj->y)));
#	endif
}

float NBPoint_vectorLenSqrd(const STNBPoint* obj){
#	ifdef sqrtf
	return ((obj->x) * (obj->x)) + ((obj->y) * (obj->y));
#	else
	return ((obj->x) * (obj->x)) + ((obj->y) * (obj->y));
#	endif
}

float NBPoint_ptVectorLen(const float x, const float y){
#	ifdef sqrtf
	return sqrtf(((x) * (x)) + ((y) * (y)));
#	else
	return sqrt(((x) * (x)) + ((y) * (y)));
#	endif
}

float NBPoint_ptVectorLenSqrd(const float x, const float y){
#	ifdef sqrtf
	return ((x) * (x)) + ((y) * (y));
#	else
	return ((x) * (x)) + ((y) * (y));
#	endif
}

float NBPoint_distance(const STNBPoint* obj, const float x, const float y){
	return NBPoint_ptVectorLen(x - obj->x, y - obj->y);
}

float NBPoint_distanceSqrd(const STNBPoint* obj, const float x, const float y){
	return NBPoint_ptVectorLenSqrd(x - obj->x, y - obj->y);
}

float NBPoint_ptDistance(const float x, const float y, const float x2, const float y2){
	return NBPoint_ptVectorLen(x2 - x, y2 - y);
}

float NBPoint_ptDistanceSqrd(const float x, const float y, const float x2, const float y2){
	return NBPoint_ptVectorLenSqrd(x2 - x, y2 - y);
}

//we have a vector for the line: V(a; b)
//we have a point on the line (the centre of the sprite): P(x1, y1)
//we have another point somewhere else: B(xB, yB)
//float _numerator = abs((b * xB) - (a * yB) - (b * x1) + (a * y1));
//float _denomimator = sqrt((a * a) + (b * b));
//float _distance = _numerator / _denominator;
//NOTA: el producto cruz de un vectorUnitario con otroVector retorna la distancia perpendicular con signo.
//#define NBPUNTO_DISTANCIA_PERPERNDICULAR_HACIA_VECTOR(VAR_DESTINO, VX, VY, PX, PY)

//Ver detalles sobre el calculo de distancia entre un punto y linea: http://mathworld.wolfram.com/Point-LineDistance2-Dimensional.html
//#define NBPUNTO_DISTANCIA_CON_SIGNO_HACIA_LINEA(PX, PY, lx1, ly1, lx2, ly2)

float NBPoint_signedDistanceToLine(const STNBPoint* obj, const float lx1, const float ly1, const float lx2, const float ly2){
	return ((((lx2) - (lx1)) * ((ly1) - (obj->y))) - (((lx1) - (obj->x)) * ((ly2) - (ly1)))) / sqrtf((((lx2) - (lx1)) * ((lx2) - (lx1)))+(((ly2) - (ly1)) * ((ly2) - (ly1))));
}

float NBPoint_ptSignedDistanceToLine(const float x, const float y, const float lx1, const float ly1, const float lx2, const float ly2){
	return ((((lx2) - (lx1)) * ((ly1) - (y))) - (((lx1) - (x)) * ((ly2) - (ly1)))) / sqrtf((((lx2) - (lx1)) * ((lx2) - (lx1)))+(((ly2) - (ly1)) * ((ly2) - (ly1))));
}

//  PRODUCTO PUNTO (nota: el producto punto con un vectorUnitario determina la distancia-hacia-adelante del otro vector sobre el unitario)
//                   0
//                   |
// -(modA*modB) -----+-----> +(modA*modB) VECTOR_A
//                   |
//                   0
//#define NBPUNTO_PRODUCTO_PUNTO(V1_X, V1_Y, V2_X, V2_Y)				(( (V1_X) * (V2_X) ) + ( (V1_Y) * (V2_Y) ))

float NBPoint_dotProduct(const STNBPoint* obj, const float x, const float y){
	return (( (obj->x) * (x) ) + ( (obj->y) * (y) ));
}

float NBPoint_ptDotProduct(const float x, const float y, const float x2, const float y2){
	return (( (x) * (x2) ) + ( (y) * (y2) ));
}

//  PRODUCTO CRUZ (nota: el producto cruz con un vectorUnitario determina la distancia-perpendicular-izquierda del otro vector hacia el unitario)
//  +(modA*modB)
//        |
// 0 -----+-----> 0  VECTOR_A
//        |
//  -(modA*modB)
//#define NBPUNTO_PRODUCTO_VECTORIAL(V1_X, V1_Y, V2_X, V2_Y)			(( (V1_X) * (V2_Y) ) - ( (V2_X) * (V1_Y) ))

float NBPoint_vectorialProduct(const STNBPoint* obj, const float x, const float y){
	return (( (obj->x) * (y) ) + ( (obj->y) * (x) ));
}

float NBPoint_ptVectorialProduct(const float x, const float y, const float x2, const float y2){
	return (( (x) * (y2) ) + ( (y) * (x2) ));
}

//Valores POSITIVOS escalan el vector y lo rotan perpendicularmente hacia la DERECHA.
//Valores NEGATIVOS escalan el vector y lo rotan perpendicularmente hacia la IZQUIERDA.
/ *#define NBPUNTO_PRODUCTO_CRUZ_VECTOR_X_ESCALAR(P_DEST, XV, YV, ESCALAR) NBASSERT(&(XV)!=&(P_DEST.x) && &(YV)!=&(P_DEST.y))
P_DEST.x = (ESCALAR) * (YV);
P_DEST.y = -(ESCALAR) * (XV);*/

/*STNBPoint NBPoint_crossProductVectorByScalar(const STNBPoint* obj, const float scale){
	STNBPoint r;
	r.x = (scale) * (obj->y);
	r.y = -(scale) * (obj->x);
	return r;
}

void NBPoint_crossProductVectorByScalarTo(const STNBPoint* obj, const float scale, STNBPoint* dst){
	dst->x = (scale) * (obj->y);
	dst->y = -(scale) * (obj->x);
}

STNBPoint NBPoint_ptCrossProductVectorByScalar(const float x, const float y, const float scale){
	STNBPoint r;
	r.x = (scale) * (y);
	r.y = -(scale) * (x);
	return r;
}

void NBPoint_ptCrossProductVectorByScalarTo(const float x, const float y, const float scale, STNBPoint* dst){
	dst->x = (scale) * (y);
	dst->y = -(scale) * (x);
}*/

//Valores POSITIVOS escalan el vector y lo rotan perpendicularmente hacia la IZQUIERDA.
//Valores NEGATIVOS escalan el vector y lo rotan perpendicularmente hacia la DERECHA.
/*#define NBPUNTO_PRODUCTO_CRUZ_ESCALAR_X_VECTOR(P_DEST, ESCALAR, XV, YV) NBASSERT(&(XV)!=&(P_DEST.x) && &(YV)!=&(P_DEST.y))
P_DEST.x = -(ESCALAR) * (YV);
P_DEST.y = (ESCALAR) * (XV);* /

STNBPoint NBPoint_crossProductScalarByVector(const STNBPoint* obj, const float scale){
	STNBPoint r;
	r.x = -(scale) * (obj->y);
	r.y = (scale) * (obj->x);
	return r;
}

void NBPoint_crossProductScalarByVectorTo(const STNBPoint* obj, const float scale, STNBPoint* dst){
	dst->x = -(scale) * (obj->y);
	dst->y = (scale) * (obj->x);
}

STNBPoint NBPoint_ptCrossProductScalarByVector(const float x, const float y, const float scale){
	STNBPoint r;
	r.x = -(scale) * (y);
	r.y = (scale) * (x);
	return r;
}

void NBPoint_ptCrossProductScalarByVectorTo(const float x, const float y, const float scale, STNBPoint* dst){
	dst->x = -(scale) * (y);
	dst->y = (scale) * (x);
}

/ *#define NBPUNTO_VECTOR_IZQUIERDA(P_DEST, XV, YV)					P_DEST.x = -(YV);
P_DEST.y = (XV);* /

STNBPoint NBPoint_vectorLeft(const STNBPoint* obj){
	STNBPoint r;
	r.x = -(obj->y);
	r.y = (obj->x);
	return r;
}

void NBPoint_vectorLeftTo(const STNBPoint* obj, STNBPoint* dst){
	dst->x = -(obj->y);
	dst->y = (obj->x);
}

STNBPoint NBPoint_ptVectorLeft(const float x, const float y){
	STNBPoint r;
	r.x = -(y);
	r.y = (x);
	return r;
}

void NBPoint_ptVectorLeftTo(const float x, const float y, STNBPoint* dst){
	dst->x = -(y);
	dst->y = (x);
}

/ *#define NBPUNTO_VECTOR_REFLEJADO(P_DEST, V_UNIT_ESPEJO, V_REFLEJAR, GUARDAR_DIST_IZQ_EN, GUARDAR_DIST_DELANTE_EN)
GUARDAR_DIST_DELANTE_EN	= NBPUNTO_PRODUCTO_PUNTO(vUnitMirror->x, vUnitMirror->y, V_REFLEJAR.x, V_REFLEJAR.y);
GUARDAR_DIST_IZQ_EN		= NBPUNTO_PRODUCTO_VECTORIAL(vUnitMirror->x, vUnitMirror->y, V_REFLEJAR.x, V_REFLEJAR.y);
P_DEST.x				= (vUnitMirror->x * GUARDAR_DIST_DELANTE_EN) + (vUnitMirror->y * GUARDAR_DIST_IZQ_EN);
P_DEST.y				= (vUnitMirror->y * GUARDAR_DIST_DELANTE_EN) - (vUnitMirror->x * GUARDAR_DIST_IZQ_EN);* /

STNBPoint NBPoint_vectorReflected(const STNBPoint* obj, const STNBPoint* vUnitMirror, float* dstLeftDist, float* dstFrontDist){
	STNBPoint r;
	const float	distFront	= NBPoint_dotProduct(vUnitMirror, obj->x, obj->y); 			//NBPUNTO_PRODUCTO_PUNTO(vUnitMirror->x, vUnitMirror->y, V_REFLEJAR.x, V_REFLEJAR.y);
	const float distLeft	= NBPoint_vectorialProduct(vUnitMirror, obj->x, obj->y);	//NBPUNTO_PRODUCTO_VECTORIAL(vUnitMirror->x, vUnitMirror->y, V_REFLEJAR.x, V_REFLEJAR.y);
	r.x						= (vUnitMirror->x * distFront) + (vUnitMirror->y * distLeft);
	r.y						= (vUnitMirror->y * distFront) - (vUnitMirror->x * distLeft);
	if(dstLeftDist != NULL) *dstLeftDist = distLeft;
	if(dstFrontDist != NULL) *dstFrontDist = distFront;
	return r;
}

void NBPoint_vectorReflectedTo(const STNBPoint* obj, const STNBPoint* vUnitMirror, float* dstLeftDist, float* dstFrontDist, STNBPoint* dst){
	const float	distFront	= NBPoint_dotProduct(vUnitMirror, obj->x, obj->y); 			//NBPUNTO_PRODUCTO_PUNTO(vUnitMirror->x, vUnitMirror->y, V_REFLEJAR.x, V_REFLEJAR.y);
	const float distLeft	= NBPoint_vectorialProduct(vUnitMirror, obj->x, obj->y);	//NBPUNTO_PRODUCTO_VECTORIAL(vUnitMirror->x, vUnitMirror->y, V_REFLEJAR.x, V_REFLEJAR.y);
	dst->x					= (vUnitMirror->x * distFront) + (vUnitMirror->y * distLeft);
	dst->y					= (vUnitMirror->y * distFront) - (vUnitMirror->x * distLeft);
	if(dstLeftDist != NULL) *dstLeftDist = distLeft;
	if(dstFrontDist != NULL) *dstFrontDist = distFront;
}

STNBPoint NBPoint_ptVectorReflected(const float x, const float y, const float vUnitMirrorX, const float vUnitMirrorY, float* dstLeftDist, float* dstFrontDist){
	STNBPoint r;
	const float	distFront	= NBPoint_ptDotProduct(vUnitMirrorX, vUnitMirrorY, x, y); 		//NBPUNTO_PRODUCTO_PUNTO(vUnitMirror->x, vUnitMirror->y, V_REFLEJAR.x, V_REFLEJAR.y);
	const float distLeft	= NBPoint_ptVectorialProduct(vUnitMirrorX, vUnitMirrorY, x, y);	//NBPUNTO_PRODUCTO_VECTORIAL(vUnitMirror->x, vUnitMirror->y, V_REFLEJAR.x, V_REFLEJAR.y);
	r.x						= (vUnitMirrorX * distFront) + (vUnitMirrorY * distLeft);
	r.y						= (vUnitMirrorY * distFront) - (vUnitMirrorX * distLeft);
	if(dstLeftDist != NULL) *dstLeftDist = distLeft;
	if(dstFrontDist != NULL) *dstFrontDist = distFront;
	return r;
}

void NBPoint_ptVectorReflectedTo(const float x, const float y, const float vUnitMirrorX, const float vUnitMirrorY, float* dstLeftDist, float* dstFrontDist, STNBPoint* dst){
	const float	distFront	= NBPoint_ptDotProduct(vUnitMirrorX, vUnitMirrorY, x, y); 		//NBPUNTO_PRODUCTO_PUNTO(vUnitMirror->x, vUnitMirror->y, V_REFLEJAR.x, V_REFLEJAR.y);
	const float distLeft	= NBPoint_ptVectorialProduct(vUnitMirrorX, vUnitMirrorY, x, y);	//NBPUNTO_PRODUCTO_VECTORIAL(vUnitMirror->x, vUnitMirror->y, V_REFLEJAR.x, V_REFLEJAR.y);
	dst->x					= (vUnitMirrorX * distFront) + (vUnitMirrorY * distLeft);
	dst->y					= (vUnitMirrorY * distFront) - (vUnitMirrorX * distLeft);
	if(dstLeftDist != NULL) *dstLeftDist = distLeft;
	if(dstFrontDist != NULL) *dstFrontDist = distFront;
}*/

/*
#define NBPUNTO_POSICION_REFLEJADA(P_DEST, P_INICIO_ESPEJO, V_UNIT_ESPEJO, P_REFLEJAR, GUARDAR_VECTOR_EN, GUARDAR_DIST_IZQ_EN, GUARDAR_DIST_DELANTE_EN)
NBPUNTO_ESTABLECER(GUARDAR_VECTOR_EN, P_REFLEJAR.x - P_INICIO_ESPEJO.x, P_REFLEJAR.y - P_INICIO_ESPEJO.y)
GUARDAR_DIST_DELANTE_EN	= NBPUNTO_PRODUCTO_PUNTO(vUnitMirror->x, vUnitMirror->y, GUARDAR_VECTOR_EN.x, GUARDAR_VECTOR_EN.y);
GUARDAR_DIST_IZQ_EN		= NBPUNTO_PRODUCTO_VECTORIAL(vUnitMirror->x, vUnitMirror->y, GUARDAR_VECTOR_EN.x, GUARDAR_VECTOR_EN.y);
P_DEST.x				= P_INICIO_ESPEJO.x + (vUnitMirror->x * GUARDAR_DIST_DELANTE_EN) + (vUnitMirror->y * GUARDAR_DIST_IZQ_EN);
P_DEST.y				= P_INICIO_ESPEJO.y + (vUnitMirror->y * GUARDAR_DIST_DELANTE_EN) - (vUnitMirror->x * GUARDAR_DIST_IZQ_EN);

#define NBPUNTO_NORMALIZAR_VECTOR(P_VECTOR_Y_DESTINO)				{\
float ___modulo = NBPUNTO_DISTANCIA_VECTOR(P_VECTOR_Y_DESTINO.x, P_VECTOR_Y_DESTINO.y);
P_VECTOR_Y_DESTINO.x /= ___modulo; P_VECTOR_Y_DESTINO.y /= ___modulo;
}

#define NBPUNTO_MOVER(PUNTO_MOVER, MOV_X, MOV_Y)					PUNTO_MOVER.x += (MOV_X); PUNTO_MOVER.y += (MOV_Y);

#define NBPUNTO_ROTAR(P_DESTINO, PUNTO, GRADOS)						{
float __radRotPto 	= GRADOS_A_RADIANES(GRADOS);
float __senRotPto	= sin(__radRotPto);
float __cosRotPto	= cos(__radRotPto);
float __xRotPto 	= (PUNTO.x * __cosRotPto) - (PUNTO.y * __senRotPto);
float __yRotPto		= (PUNTO.x * __senRotPto) + (PUNTO.y * __cosRotPto);
P_DESTINO.x			= __xRotPto;
P_DESTINO.y			= __yRotPto;
}

#define NBPUNTO_ROTAR_RADIANES(P_DESTINO, PUNTO, RADIANES)			{
float __senRotPto	= sin(RADIANES);
float __cosRotPto	= cos(RADIANES);
float __xRotPto 	= (PUNTO.x * __cosRotPto) - (PUNTO.y * __senRotPto);
float __yRotPto		= (PUNTO.x * __senRotPto) + (PUNTO.y * __cosRotPto);
P_DESTINO.x			= __xRotPto;
P_DESTINO.y			= __yRotPto;
}

#define NBPUNTO_OBTENER_VECTOR_ROTADO(P_DESTINO, MOD_VEC, RAD)		P_DESTINO.x			= (MOD_VEC) * cos(RAD);
P_DESTINO.y			= (MOD_VEC) * sin(RAD);

#define NBPUNTO_ROTAR_RADIANES_P(P_DESTINO, P_X, P_Y, RADIANES)		{
float __senRotPto	= sin(RADIANES);
float __cosRotPto	= cos(RADIANES);
float __xRotPto 	= (P_X * __cosRotPto) - (P_Y * __senRotPto);
float __yRotPto		= (P_X * __senRotPto) + (P_Y * __cosRotPto);
P_DESTINO.x			= __xRotPto;
P_DESTINO.y			= __yRotPto;
}


#define NBPUNTO_ROTAR_EN_EJE(P_DEST, PUNTO, GRADOS, EJE_X, EJE_Y)	{
float __radRotEje 	= GRADOS_A_RADIANES(GRADOS);
float __senRotEje	= sin(__radRotEje);
float __cosRotEje	= cos(__radRotEje);
float __xRotEje 	= ((PUNTO.x-(EJE_X)) * __cosRotEje) - ((PUNTO.y-(EJE_Y)) * __senRotEje);
float __yRotEje		= ((PUNTO.x-(EJE_X)) * __senRotEje) + ((PUNTO.y-(EJE_Y)) * __cosRotEje);
P_DEST.x			= __xRotEje + (EJE_X);
P_DEST.y			= __yRotEje + (EJE_Y);
}

#define NBPUNTO_RADIANES_VECTOR(VAR_DEST, INI_X, INI_Y, FIN_X, FIN_Y)	{
float __xVectBsqAngulo = ((FIN_X) - (INI_X));
float __yVectBsqAngulo = ((FIN_Y) - (INI_Y));
VAR_DEST = 0.0f;
if(__xVectBsqAngulo==0.0f){
if(__yVectBsqAngulo<0.0f) VAR_DEST = 3.0f*PI/2.0f;
if(__yVectBsqAngulo>0.0f) VAR_DEST = PI/2.0f;
} else {
VAR_DEST = atan(__yVectBsqAngulo/__xVectBsqAngulo);
if(__xVectBsqAngulo<0.0f) VAR_DEST += PI;
}
while(VAR_DEST<0.0f) VAR_DEST += PIx2;
while(VAR_DEST>=PIx2) VAR_DEST -= PIx2;
/ *if((VAR_DEST>PI && VAR_DEST<3.0f*PI/2.0f) || (VAR_DEST>0.0f && VAR_DEST<PI/2.0f)) PRINTF_INFO("Angulo: %f\n", VAR_DEST);* /
}

#define NBPUNTO_ANGULO_VECTOR(VAR_DEST, INI_X, INI_Y, FIN_X, FIN_Y)	NBPUNTO_RADIANES_VECTOR(VAR_DEST, INI_X, INI_Y, FIN_X, FIN_Y) VAR_DEST = RADIANES_A_GRADOS(VAR_DEST);

#define NBPUNTO_ESTA_IZQUIERDA_VECTOR_V(VBASE_X, VBASE_Y, VCOMP_X, VCOMP_Y)
(NBPUNTO_PRODUCTO_VECTORIAL(VCOMP_X, VCOMP_Y, VBASE_X, VBASE_Y)<0.0f)

#define NBPUNTO_ESTA_IZQUIERDA_VECTOR(INIV_X, INIV_Y, FINV_X, FINV_Y, PX, PY)
(NBPUNTO_ESTA_IZQUIERDA_VECTOR_V((FINV_X) - (INIV_X), (FINV_Y) - (INIV_Y), (PX) - (INIV_X), (PY) - (INIV_Y)))

#define NBPUNTO_ESTA_SOBRE_VECTOR(INIV_X, INIV_Y, FINV_X, FINV_Y, PX, PY)
(NBPUNTO_PRODUCTO_VECTORIAL((PX) - (INIV_X), (PY) - (INIV_Y), (FINV_X) - (INIV_X), (FINV_Y) - (INIV_Y))==0.0f)


#define NBPUNTO_ESTA_IZQUIERDA_O_SOBRE_VECTOR(INIV_X, INIV_Y, FINV_X, FINV_Y, PX, PY)
(NBPUNTO_PRODUCTO_VECTORIAL((PX) - (INIV_X), (PY) - (INIV_Y), (FINV_X) - (INIV_X), (FINV_Y) - (INIV_Y))<=0.0f)*/
