//
//  NBMatrix.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/6/18.
//

#include "nb/NBFrameworkPch.h"
#include "nb/2d/NBMatrix.h"
//
#include <math.h> 	//for sqrt(), sin(), cos(), etc...

//Transform

void NBMatrix_setIdentity(STNBMatrix* obj){
	NBMATRIXP_E00(obj) = 1; NBMATRIXP_E01(obj) = 0; NBMATRIXP_E02(obj) = 0;
	NBMATRIXP_E10(obj) = 0; NBMATRIXP_E11(obj) = 1; NBMATRIXP_E12(obj) = 0;
	/*NBMATRIXP_E20(obj) = 0; NBMATRIXP_E21(obj) = 0; NBMATRIXP_E22(obj) = 1;*/
}

void NBMatrix_translate(STNBMatrix* obj, const float tx, const float ty){
	NBMATRIXP_E02(obj) = (NBMATRIXP_E00(obj) * tx) + (NBMATRIXP_E01(obj) * ty) + NBMATRIXP_E02(obj);
	NBMATRIXP_E12(obj) = (NBMATRIXP_E10(obj) * tx) + (NBMATRIXP_E11(obj) * ty) + NBMATRIXP_E12(obj);
}

void NBMatrix_translateWithPoint(STNBMatrix* obj, const STNBPoint t){
	NBMATRIXP_E02(obj) = (NBMATRIXP_E00(obj) * t.x) + (NBMATRIXP_E01(obj) * t.y) + NBMATRIXP_E02(obj);
	NBMATRIXP_E12(obj) = (NBMATRIXP_E10(obj) * t.x) + (NBMATRIXP_E11(obj) * t.y) + NBMATRIXP_E12(obj);
}

void NBMatrix_scale(STNBMatrix* obj, const float sx, const float sy){
	NBMATRIXP_E00(obj) *= (sx);
	NBMATRIXP_E10(obj) *= (sx);
	NBMATRIXP_E01(obj) *= (sy);
	NBMATRIXP_E11(obj) *= (sy);
}

void NBMatrix_scaleWithSize(STNBMatrix* obj, const STNBSize s){
	NBMATRIXP_E00(obj) *= (s.width);
	NBMATRIXP_E10(obj) *= (s.width);
	NBMATRIXP_E01(obj) *= (s.height);
	NBMATRIXP_E11(obj) *= (s.height);
}

void NBMatrix_rotate(STNBMatrix* obj, const float rad){
	const float vSin	= sin(rad);
	const float vCos	= cos(rad);
	const float e00		= NBMATRIXP_E00(obj);
	const float e10		= NBMATRIXP_E10(obj);
	NBMATRIXP_E00(obj)	= (e00 * vCos)	+ (NBMATRIXP_E01(obj) * vSin);
	NBMATRIXP_E01(obj)	= (e00 * -vSin) + (NBMATRIXP_E01(obj) * vCos);
	NBMATRIXP_E10(obj)	= (e10 * vCos)	+ (NBMATRIXP_E11(obj) * vSin);
	NBMATRIXP_E11(obj)	= (e10 * -vSin) + (NBMATRIXP_E11(obj) * vCos);
}

void NBMatrix_rotateDeg(STNBMatrix* obj, const float deg){
	const float	rad		= DEG_2_RAD(deg);
	const float vSin	= sin(rad);
	const float vCos	= cos(rad);
	const float e00		= NBMATRIXP_E00(obj);
	const float e10		= NBMATRIXP_E10(obj);
	NBMATRIXP_E00(obj)	= (e00 * vCos)	+ (NBMATRIXP_E01(obj) * vSin);
	NBMATRIXP_E01(obj)	= (e00 * -vSin) + (NBMATRIXP_E01(obj) * vCos);
	NBMATRIXP_E10(obj)	= (e10 * vCos)	+ (NBMATRIXP_E11(obj) * vSin);
	NBMATRIXP_E11(obj)	= (e10 * -vSin) + (NBMATRIXP_E11(obj) * vCos);
}

STNBMatrix NBMatrix_multiply(const STNBMatrix* one, const STNBMatrix* other){
	STNBMatrix r;
	NBMATRIX_E00(r) = (NBMATRIXP_E00(one) * NBMATRIXP_E00(other)) + (NBMATRIXP_E01(one) * NBMATRIXP_E10(other)) /*siempre cero: + (NBMATRIXP_E02(one) * NBMATRIXP_E20(other))*/;
	NBMATRIX_E01(r) = (NBMATRIXP_E00(one) * NBMATRIXP_E01(other)) + (NBMATRIXP_E01(one) * NBMATRIXP_E11(other)) /*siempre cero: + (NBMATRIXP_E02(one) * NBMATRIXP_E21(other))*/;
	NBMATRIX_E02(r) = (NBMATRIXP_E00(one) * NBMATRIXP_E02(other)) + (NBMATRIXP_E01(one) * NBMATRIXP_E12(other)) + (NBMATRIXP_E02(one) /*siempre 1: * NBMATRIXP_E22(other)*/);
	NBMATRIX_E10(r) = (NBMATRIXP_E10(one) * NBMATRIXP_E00(other)) + (NBMATRIXP_E11(one) * NBMATRIXP_E10(other)) /*siempre cero: + (NBMATRIXP_E12(one) * NBMATRIXP_E20(other))*/;
	NBMATRIX_E11(r) = (NBMATRIXP_E10(one) * NBMATRIXP_E01(other)) + (NBMATRIXP_E11(one) * NBMATRIXP_E11(other)) /*siempre cero: + (NBMATRIXP_E12(one) * NBMATRIXP_E21(other))*/;
	NBMATRIX_E12(r) = (NBMATRIXP_E10(one) * NBMATRIXP_E02(other)) + (NBMATRIXP_E11(one) * NBMATRIXP_E12(other)) + (NBMATRIXP_E12(one) /*siempre 1: * NBMATRIXP_E22(other)*/);
	return r;
}

//Calculate

float NBMatrix_determinant(const STNBMatrix* obj){
	return ((NBMATRIXP_E00(obj) * NBMATRIXP_E11(obj)) - (NBMATRIXP_E01(obj) * NBMATRIXP_E10(obj))); /*only 6 elems matrix*/
}

STNBMatrix NBMatrix_inverse(const STNBMatrix* obj){
	STNBMatrix r;
	const float vDet	= ((NBMATRIXP_E00(obj) * NBMATRIXP_E11(obj)) - (NBMATRIXP_E01(obj) * NBMATRIXP_E10(obj)));
	//NBASSERT(vDet != 0.0f && vDet != -0.0f) NBASSERT(vDet == vDet)
	if (vDet != 0.0f && vDet != -0.0f && vDet == vDet) {
		NBMATRIX_E00(r) = ((NBMATRIXP_E11(obj) /** NBMATRIXP_E22(obj)*/) /*- (NBMATRIXP_E21(obj) * NBMATRIXP_E12(obj)) always zero*/) / vDet;
		NBMATRIX_E01(r) = (/*(NBMATRIXP_E02(obj) * NBMATRIXP_E21(obj)) always zero*/0.0f - (/*NBMATRIXP_E22(obj) **/ NBMATRIXP_E01(obj))) / vDet;
		NBMATRIX_E02(r) = ((NBMATRIXP_E01(obj) * NBMATRIXP_E12(obj)) - (NBMATRIXP_E11(obj) * NBMATRIXP_E02(obj))) / vDet;
		NBMATRIX_E10(r) = (/*(NBMATRIXP_E12(obj) * NBMATRIXP_E20(obj)) always zero*/0.0f - (/*NBMATRIXP_E22(obj) **/ NBMATRIXP_E10(obj))) / vDet;
		NBMATRIX_E11(r) = ((NBMATRIXP_E00(obj) /** NBMATRIXP_E22(obj)*/) /*- (NBMATRIXP_E20(obj) * NBMATRIXP_E02(obj)) always zero*/) / vDet;
		NBMATRIX_E12(r) = ((NBMATRIXP_E02(obj) * NBMATRIXP_E10(obj)) - (NBMATRIXP_E12(obj) * NBMATRIXP_E00(obj))) / vDet;
		/*NBMATRIXP_E20(r)	= ((NBMATRIXP_E10(obj) * NBMATRIXP_E21(obj)) - (NBMATRIXP_E20(obj) * NBMATRIXP_E11(obj))) / vDet;*/
		/*NBMATRIXP_E21(r)	= ((NBMATRIXP_E01(obj) * NBMATRIXP_E20(obj)) - (NBMATRIXP_E21(obj) * NBMATRIXP_E00(obj))) / vDet;*/
		/*NBMATRIXP_E22(r)	= ((NBMATRIXP_E00(obj) * NBMATRIXP_E11(obj)) - (NBMATRIXP_E10(obj) * NBMATRIXP_E01(obj))) / vDet;*/
	} else {
		NBMemory_setZeroSt(r, STNBMatrix);
	}
	return r;
}

STNBMatrix NBMatrix_fromTransforms(const STNBPoint traslation, const float radRot, const STNBSize scale){
	STNBMatrix r;
	const float vSin	= sin(radRot);
	const float vCos	= cos(radRot);
	NBMATRIX_E00(r)		= vCos * scale.width;
	NBMATRIX_E01(r)		= -vSin * scale.height;
	NBMATRIX_E02(r)		= traslation.x;
	NBMATRIX_E10(r)		= vSin * scale.width;
	NBMATRIX_E11(r)		= vCos * scale.height;
	NBMATRIX_E12(r)		= traslation.y;
	return r;
}

//Apply

STNBPoint NBMatrix_applyToPoint(const STNBMatrix* obj, const STNBPoint p){
	STNBPoint r;
	r.x = (NBMATRIXP_E00(obj) * (p.x)) + (NBMATRIXP_E01(obj) * (p.y)) + NBMATRIXP_E02(obj);
	r.y = (NBMATRIXP_E10(obj) * (p.x)) + (NBMATRIXP_E11(obj) * (p.y)) + NBMATRIXP_E12(obj);
	return r;
}

//
/*
float NBMatrix_translationX(const STNBMatrix* obj){
	return NBMATRIXP_E02(obj);
}

float NBMatrix_translationY(const STNBMatrix* obj){
	return NBMATRIXP_E12(obj);
}

STNBPoint NBMatrix_translation(const STNBMatrix* obj){
	STNBPoint r;
	r.x = NBMATRIXP_E02(obj);
	r.y	= NBMATRIXP_E12(obj);
	return r;
}

/ *#define NBMATRIZ_IMPRIMIR_PRINTF(obj)						PRINTF_INFO("F0 [%.6f] [%.6f] [%.6f]\n", NBMATRIXP_E00(obj), NBMATRIXP_E01(obj), NBMATRIXP_E02(obj));
PRINTF_INFO("F1 [%.6f] [%.6f] [%.6f]\n", NBMATRIXP_E10(obj), NBMATRIXP_E11(obj), NBMATRIXP_E12(obj));
PRINTF_INFO("F2 [%.6f] [%.6f] [%.6f]\n", NBMATRIXP_E20(obj), NBMATRIXP_E21(obj), NBMATRIXP_E22(obj));* /

void NBMatrix_setIdentity(STNBMatrix* obj){
	NBMATRIXP_E00(obj) = 1; NBMATRIXP_E01(obj) = 0; NBMATRIXP_E02(obj) = 0;
	NBMATRIXP_E10(obj) = 0; NBMATRIXP_E11(obj) = 1; NBMATRIXP_E12(obj) = 0;
	/ *NBMATRIXP_E20(obj) = 0; NBMATRIXP_E21(obj) = 0; NBMATRIXP_E22(obj) = 1;* /
}

void NBMatrix_setZeros(STNBMatrix* obj){
	NBMATRIXP_E00(obj) = 0; NBMATRIXP_E01(obj) = 0; NBMATRIXP_E02(obj) = 0;
	NBMATRIXP_E10(obj) = 0; NBMATRIXP_E11(obj) = 0; NBMATRIXP_E12(obj) = 0;
	/ *NBMATRIXP_E20(obj) = 0; NBMATRIXP_E21(obj) = 0; NBMATRIXP_E22(obj) = 0;* /
}

BOOL NBMatrix_wontDeform(const STNBMatrix* obj){
	return ((NBMATRIXP_E00(obj) == 1 || NBMATRIXP_E00(obj) == -1) && NBMATRIXP_E01(obj) == 0 && NBMATRIXP_E10(obj) == 0 && (NBMATRIXP_E11(obj) == 1 || NBMATRIXP_E11(obj) == -1)) ? TRUE : FALSE;
}

float NBMatrix_determinant(const STNBMatrix* obj){
	return ((NBMATRIXP_E00(obj)*NBMATRIXP_E11(obj))-(NBMATRIXP_E01(obj)*NBMATRIXP_E10(obj))); / *solo implementado para la matriz de seis elementos* /
}

STNBMatrix NBMatrix_inverse(const STNBMatrix* obj){
	STNBMatrix r;
	const float det = NBMatrix_determinant(obj); NBASSERT(det != 0.0f && det != -0.0f); NBASSERT(det == det);
	NBMATRIX_E00(r) = ((NBMATRIXP_E11(obj) / ** NBMATRIXP_E22(obj)* /) / *- (NBMATRIXP_E21(obj) * NBMATRIXP_E12(obj)) always zero* /) / det;
	NBMATRIX_E01(r) = (/ *(NBMATRIXP_E02(obj) * NBMATRIXP_E21(obj)) always zero* /0.0f - (/ *NBMATRIXP_E22(obj) ** / NBMATRIXP_E01(obj))) / det;
	NBMATRIX_E02(r) = ((NBMATRIXP_E01(obj) * NBMATRIXP_E12(obj)) - (NBMATRIXP_E11(obj) * NBMATRIXP_E02(obj))) / det;
	NBMATRIX_E10(r) = (/ *(NBMATRIXP_E12(obj) * NBMATRIXP_E20(obj)) always zero* /0.0f - (/ *NBMATRIXP_E22(obj) ** / NBMATRIXP_E10(obj))) / det;
	NBMATRIX_E11(r) = ((NBMATRIXP_E00(obj) / ** NBMATRIXP_E22(obj)* /) / *- (NBMATRIXP_E20(obj) * NBMATRIXP_E02(obj)) always zero* /) / det;
	NBMATRIX_E12(r) = ((NBMATRIXP_E02(obj) * NBMATRIXP_E10(obj)) - (NBMATRIXP_E12(obj) * NBMATRIXP_E00(obj))) / det;
	/ *NBMATRIXP_E20(dst) = ((NBMATRIXP_E10(obj) * NBMATRIXP_E21(obj)) - (NBMATRIXP_E20(obj) * NBMATRIXP_E11(obj))) / det;* /
	/ *NBMATRIXP_E21(dst) = ((NBMATRIXP_E01(obj) * NBMATRIXP_E20(obj)) - (NBMATRIXP_E21(obj) * NBMATRIXP_E00(obj))) / det;* /
	/ *NBMATRIXP_E22(dst) = ((NBMATRIXP_E00(obj) * NBMATRIXP_E11(obj)) - (NBMATRIXP_E10(obj) * NBMATRIXP_E01(obj))) / det;* /
	return r;
}

void NBMatrix_inverseTo(const STNBMatrix* obj, STNBMatrix* dst){
	const float det = NBMatrix_determinant(obj); NBASSERT(det != 0.0f && det != -0.0f); NBASSERT(det == det);
	NBMATRIXP_E00(dst) = ((NBMATRIXP_E11(obj) / ** NBMATRIXP_E22(obj)* /) / *- (NBMATRIXP_E21(obj) * NBMATRIXP_E12(obj)) always zero* /) / det;
	NBMATRIXP_E01(dst) = (/ *(NBMATRIXP_E02(obj) * NBMATRIXP_E21(obj)) always zero* /0.0f - (/ *NBMATRIXP_E22(obj) ** / NBMATRIXP_E01(obj))) / det;
	NBMATRIXP_E02(dst) = ((NBMATRIXP_E01(obj) * NBMATRIXP_E12(obj)) - (NBMATRIXP_E11(obj) * NBMATRIXP_E02(obj))) / det;
	NBMATRIXP_E10(dst) = (/ *(NBMATRIXP_E12(obj) * NBMATRIXP_E20(obj)) always zero* /0.0f - (/ *NBMATRIXP_E22(obj) ** / NBMATRIXP_E10(obj))) / det;
	NBMATRIXP_E11(dst) = ((NBMATRIXP_E00(obj) / ** NBMATRIXP_E22(obj)* /) / *- (NBMATRIXP_E20(obj) * NBMATRIXP_E02(obj)) always zero* /) / det;
	NBMATRIXP_E12(dst) = ((NBMATRIXP_E02(obj) * NBMATRIXP_E10(obj)) - (NBMATRIXP_E12(obj) * NBMATRIXP_E00(obj))) / det;
	/ *NBMATRIXP_E20(dst) = ((NBMATRIXP_E10(obj) * NBMATRIXP_E21(obj)) - (NBMATRIXP_E20(obj) * NBMATRIXP_E11(obj))) / det;* /
	/ *NBMATRIXP_E21(dst) = ((NBMATRIXP_E01(obj) * NBMATRIXP_E20(obj)) - (NBMATRIXP_E21(obj) * NBMATRIXP_E00(obj))) / det;* /
	/ *NBMATRIXP_E22(dst) = ((NBMATRIXP_E00(obj) * NBMATRIXP_E11(obj)) - (NBMATRIXP_E10(obj) * NBMATRIXP_E01(obj))) / det;* /
}

STNBPoint NBMatrix_multiplyPoint(const STNBMatrix* obj, const float x, const float y){
	STNBPoint r;
	r.x = (NBMATRIXP_E00(obj) * (x)) + (NBMATRIXP_E01(obj) * (y)) + NBMATRIXP_E02(obj);
	r.y = (NBMATRIXP_E10(obj) * (x)) + (NBMATRIXP_E11(obj) * (y)) + NBMATRIXP_E12(obj);
	return r;
}

void NBMatrix_multiplyPointTo(const STNBMatrix* obj, const float x, const float y, STNBPoint* dst){
	dst->x = (NBMATRIXP_E00(obj) * (x)) + (NBMATRIXP_E01(obj) * (y)) + NBMATRIXP_E02(obj);
	dst->y = (NBMATRIXP_E10(obj) * (x)) + (NBMATRIXP_E11(obj) * (y)) + NBMATRIXP_E12(obj);
}

STNBPoint NBMatrix_multiplyPointWithoutTranlate(const STNBMatrix* obj, const float x, const float y){
	STNBPoint r;
	r.x = (NBMATRIXP_E00(obj) * (x)) + (NBMATRIXP_E01(obj) * (y)) / *+ NBMATRIXP_E02(obj) sin trasladar* /;
	r.y = (NBMATRIXP_E10(obj) * (x)) + (NBMATRIXP_E11(obj) * (y)) / *+ NBMATRIXP_E12(obj) sin trasladar* /;
	return r;
}

void NBMatrix_multiplyPointWithoutTranlateTo(const STNBMatrix* obj, const float x, const float y, STNBPoint* dst){
	dst->x = (NBMATRIXP_E00(obj) * (x)) + (NBMATRIXP_E01(obj) * (y)) / *+ NBMATRIXP_E02(obj) sin trasladar* /;
	dst->y = (NBMATRIXP_E10(obj) * (x)) + (NBMATRIXP_E11(obj) * (y)) / *+ NBMATRIXP_E12(obj) sin trasladar* /;
}

STNBPoint NBMatrix_onlyTranslatePoint(const STNBMatrix* obj, const float x, const float y){
	STNBPoint r;
	r.x += / *(NBMATRIXP_E00(obj) * (P_X)) + (NBMATRIXP_E01(obj) * (P_Y)) +* / NBMATRIXP_E02(obj);
	r.y += / *(NBMATRIXP_E10(obj) * (P_X)) + (NBMATRIXP_E11(obj) * (P_Y)) +* / NBMATRIXP_E12(obj);
	return r;
}

void NBMatrix_onlyTranslatePointTo(const STNBMatrix* obj, const float x, const float y, STNBPoint* dst){
	dst->x += / *(NBMATRIXP_E00(obj) * (P_X)) + (NBMATRIXP_E01(obj) * (P_Y)) +* / NBMATRIXP_E02(obj);
	dst->y += / *(NBMATRIXP_E10(obj) * (P_X)) + (NBMATRIXP_E11(obj) * (P_Y)) +* / NBMATRIXP_E12(obj);
}

/ *STNBPoint NBMatrix_multiplyPoint(const STNBMatrix* obj, const float x, const float y){
	STNBPoint r;
	r.x = (NBMATRIXP_E00(obj) * (x)) + (NBMATRIXP_E01(obj) * (y)) + NBMATRIXP_E02(obj);
	r.y = (NBMATRIXP_E10(obj) * (x)) + (NBMATRIXP_E11(obj) * (y)) + NBMATRIXP_E12(obj);
	return r;
}

void NBMatrix_multiplyPointTo(const STNBMatrix* obj, const float x, const float y, STNBPoint* dst){
	dst->x = (NBMATRIXP_E00(obj) * (x)) + (NBMATRIXP_E01(obj) * (y)) + NBMATRIXP_E02(obj);
	dst->y = (NBMATRIXP_E10(obj) * (x)) + (NBMATRIXP_E11(obj) * (y)) + NBMATRIXP_E12(obj);
}* /

STNBAABox NBMatrix_multiplyBox(const STNBMatrix* obj, const STNBAABox* box){
	STNBAABox r;
	STNBPoint p;
	NBMatrix_multiplyPointTo(obj, box->xMin, box->yMin, &p); NBAABox_initWithPoint(&r, p.x, p.y);
	NBMatrix_multiplyPointTo(obj, box->xMax, box->yMin, &p); NBAABox_wrapNextPoint(&r, p.x, p.y);
	NBMatrix_multiplyPointTo(obj, box->xMax, box->yMax, &p); NBAABox_wrapNextPoint(&r, p.x, p.y);
	NBMatrix_multiplyPointTo(obj, box->xMin, box->yMax, &p); NBAABox_wrapNextPoint(&r, p.x, p.y);
	return r;
}

void NBMatrix_multiplyBoxTo(const STNBMatrix* obj, const STNBAABox* box, STNBAABox* dst){
	STNBPoint p;
	NBMatrix_multiplyPointTo(obj, box->xMin, box->yMin, &p); NBAABox_initWithPoint(dst, p.x, p.y);
	NBMatrix_multiplyPointTo(obj, box->xMax, box->yMin, &p); NBAABox_wrapNextPoint(dst, p.x, p.y);
	NBMatrix_multiplyPointTo(obj, box->xMax, box->yMax, &p); NBAABox_wrapNextPoint(dst, p.x, p.y);
	NBMatrix_multiplyPointTo(obj, box->xMin, box->yMax, &p); NBAABox_wrapNextPoint(dst, p.x, p.y);
}

#define NBMATRIZ_MULTIPLICAR_CAJAAABB(CAJA_DEST, MATRIZ, CAJA)	{
NBCajaAABBP<float> __rMutlCajaAABB; NBCAJAAABB_INICIALIZAR(__rMutlCajaAABB);
NBPuntoP<float> __puntoMultCajaAABB;
NBMATRIZ_MULTIPLICAR_PUNTO(__puntoMultCajaAABB, MATRIZ, CAJA.xMin, CAJA.yMin); NBCAJAAABB_ENVOLVER_PUNTO(__rMutlCajaAABB, __puntoMultCajaAABB.x, __puntoMultCajaAABB.y);
NBMATRIZ_MULTIPLICAR_PUNTO(__puntoMultCajaAABB, MATRIZ, CAJA.xMax, CAJA.yMin); NBCAJAAABB_ENVOLVER_PUNTO(__rMutlCajaAABB, __puntoMultCajaAABB.x, __puntoMultCajaAABB.y);
NBMATRIZ_MULTIPLICAR_PUNTO(__puntoMultCajaAABB, MATRIZ, CAJA.xMax, CAJA.yMax); NBCAJAAABB_ENVOLVER_PUNTO(__rMutlCajaAABB, __puntoMultCajaAABB.x, __puntoMultCajaAABB.y);
NBMATRIZ_MULTIPLICAR_PUNTO(__puntoMultCajaAABB, MATRIZ, CAJA.xMin, CAJA.yMax); NBCAJAAABB_ENVOLVER_PUNTO(__rMutlCajaAABB, __puntoMultCajaAABB.x, __puntoMultCajaAABB.y);
CAJA_DEST = __rMutlCajaAABB;
}

#define NBMATRIZ_MULTIPLICAR_CON_MATRIZ(M_DEST, cpy, other)		{
/ * 22/10/2011 - Pruebas de rendimiento con 10,000,000 multiplicaciones de matrices * /
/ * Preprogramado: promedio 639,000 ciclos * /
/ * Ciclo FOR:     promedio 3,956,000 ciclos * /
/ * Optimizacion: en las matrices 2D los elementos [2][0] y [2][1] siempre almacena 'cero', el elemento [2][2] siempre almacena 'uno'. * /
NBMatrizP<float> __matrizMultiplicada; / *Asumiento que todas las matrices son FLOAT* /
NBMATRIXP_E00(__matrizMultiplicada) = (NBMATRIXP_E00(cpy) * NBMATRIXP_E00(other)) + (NBMATRIXP_E01(cpy) * NBMATRIXP_E10(other)) / *always zero: + (NBMATRIXP_E02(cpy) * NBMATRIXP_E20(other))* /;
NBMATRIXP_E01(__matrizMultiplicada) = (NBMATRIXP_E00(cpy) * NBMATRIXP_E01(other)) + (NBMATRIXP_E01(cpy) * NBMATRIXP_E11(other)) / *always zero: + (NBMATRIXP_E02(cpy) * NBMATRIXP_E21(other))* /;
NBMATRIXP_E02(__matrizMultiplicada) = (NBMATRIXP_E00(cpy) * NBMATRIXP_E02(other)) + (NBMATRIXP_E01(cpy) * NBMATRIXP_E12(other)) + (NBMATRIXP_E02(cpy) / *siempre 1: * NBMATRIXP_E22(other)* /);
NBMATRIXP_E10(__matrizMultiplicada) = (NBMATRIXP_E10(cpy) * NBMATRIXP_E00(other)) + (NBMATRIXP_E11(cpy) * NBMATRIXP_E10(other)) / *always zero: + (NBMATRIXP_E12(cpy) * NBMATRIXP_E20(other))* /;
NBMATRIXP_E11(__matrizMultiplicada) = (NBMATRIXP_E10(cpy) * NBMATRIXP_E01(other)) + (NBMATRIXP_E11(cpy) * NBMATRIXP_E11(other)) / *always zero: + (NBMATRIXP_E12(cpy) * NBMATRIXP_E21(other))* /;
NBMATRIXP_E12(__matrizMultiplicada) = (NBMATRIXP_E10(cpy) * NBMATRIXP_E02(other)) + (NBMATRIXP_E11(cpy) * NBMATRIXP_E12(other)) + (NBMATRIXP_E12(cpy) / *siempre 1: * NBMATRIXP_E22(other)* /);
/ *always zero: NBMATRIXP_E20(__matrizMultiplicada) = (NBMATRIXP_E20(cpy) * NBMATRIXP_E00(other)) + (NBMATRIXP_E21(cpy) * NBMATRIXP_E10(other))+ (NBMATRIXP_E22(cpy) * NBMATRIXP_E20(other));* /
/ *always zero: NBMATRIXP_E21(__matrizMultiplicada) = (NBMATRIXP_E20(cpy) * NBMATRIXP_E01(other)) + (NBMATRIXP_E21(cpy) * NBMATRIXP_E11(other)) + (NBMATRIXP_E22(cpy) * NBMATRIXP_E21(other));* /
/ *siempre uno: NBMATRIXP_E22(__matrizMultiplicada) = (NBMATRIXP_E20(cpy) * NBMATRIXP_E02(other)) + (NBMATRIXP_E21(cpy) * NBMATRIXP_E12(other)) + (NBMATRIXP_E22(cpy) * NBMATRIXP_E22(other));* /
M_DEST = __matrizMultiplicada;
}

//Aplicar transformaciones a matrices.
//A continuacion las versiones "puras", son costosas pero realizan la operacion matematica acorde al libro.
//Se les puede pasar cualquier tipo de parametros tranformacion (incluyendo funciones) debido a que utilizan variables locales temporales propias.
/ *#define NBMATRIZ_TRASLADAR(M_BASE_Y_DEST, TRAS_X, TRAS_Y)		{
 NBMatrizP<float> __matrizTraslacion;
 NBMATRIXP_E00(__matrizTraslacion) = 1;	NBMATRIXP_E01(__matrizTraslacion) = 0;	NBMATRIXP_E02(__matrizTraslacion) = (TRAS_X);
 NBMATRIXP_E10(__matrizTraslacion) = 0;	NBMATRIXP_E11(__matrizTraslacion) = 1;	NBMATRIXP_E12(__matrizTraslacion) = (TRAS_Y);
 NBMATRIZ_MULTIPLICAR_CON_MATRIZ(M_BASE_Y_DEST, M_BASE_Y_DEST, __matrizTraslacion);
 }
 
 #define NBMATRIZ_ESCALAR(M_BASE_Y_DEST, ESC_X, ESC_Y)			{
 NBMatrizP<float> __matrizEscalacion;
 NBMATRIXP_E00(__matrizEscalacion) = (ESC_X);	NBMATRIXP_E01(__matrizEscalacion) = 0;		NBMATRIXP_E02(__matrizEscalacion) = 0;
 NBMATRIXP_E10(__matrizEscalacion) = 0;		NBMATRIXP_E11(__matrizEscalacion) = (ESC_Y);	NBMATRIXP_E12(__matrizEscalacion) = 0;
 NBMATRIZ_MULTIPLICAR_CON_MATRIZ(M_BASE_Y_DEST, M_BASE_Y_DEST, __matrizEscalacion);
 }
 
 
 #define NBMATRIZ_ROTAR_RADIANES(M_BASE_Y_DEST, RADIANES)		{
 float vSin	= sin(RADIANES);
 float vCos	= cos(RADIANES);
 NBMatrizP<float> __matrizRotacionRadianes;
 NBMATRIXP_E00(__matrizRotacionRadianes) = vCos;	NBMATRIXP_E01(__matrizRotacionRadianes) = -vSin;	NBMATRIXP_E02(__matrizRotacionRadianes) = 0;
 NBMATRIXP_E10(__matrizRotacionRadianes) = vSin;	NBMATRIXP_E11(__matrizRotacionRadianes) = vCos;	NBMATRIXP_E12(__matrizRotacionRadianes) = 0;
 NBMATRIZ_MULTIPLICAR_CON_MATRIZ(M_BASE_Y_DEST, M_BASE_Y_DEST, __matrizRotacionRadianes);
 }
 
 #define NBMATRIZ_ROTAR_GRADOS(M_BASE_Y_DEST, GRADOS)			NBMATRIZ_ROTAR_RADIANES(M_BASE_Y_DEST, DEG_2_RAD(GRADOS))* /

//Aplicar transformaciones a matrices.
//A continuacion las versiones "rapidas", son eficientes al realizar solo las operaciones que afectan a la matriz.
//Se les deben pasar parametros de tranformacion en forma de variables. No utilizan variables temporales (excepto al rotar), por lo que si se le pasan valores en forma de funciones esta sera ejecuta mas de una vez.
#define NBMATRIZ_TRASLADAR(M_BASE_Y_DEST, VAR_TRAS_X, VAR_TRAS_Y)
NBMATRIXP_E02(M_BASE_Y_DEST) = (NBMATRIXP_E00(M_BASE_Y_DEST) * (VAR_TRAS_X)) + (NBMATRIXP_E01(M_BASE_Y_DEST) * (VAR_TRAS_Y)) + NBMATRIXP_E02(M_BASE_Y_DEST);
NBMATRIXP_E12(M_BASE_Y_DEST) = (NBMATRIXP_E10(M_BASE_Y_DEST) * (VAR_TRAS_X)) + (NBMATRIXP_E11(M_BASE_Y_DEST) * (VAR_TRAS_Y)) + NBMATRIXP_E12(M_BASE_Y_DEST);


#define NBMATRIZ_ESCALAR(M_BASE_Y_DEST, VAR_ESC_X, VAR_ESC_Y)
NBMATRIXP_E00(M_BASE_Y_DEST) *= (VAR_ESC_X);
NBMATRIXP_E10(M_BASE_Y_DEST) *= (VAR_ESC_X);
NBMATRIXP_E01(M_BASE_Y_DEST) *= (VAR_ESC_Y);
NBMATRIXP_E11(M_BASE_Y_DEST) *= (VAR_ESC_Y);

#define NBMATRIZ_ROTAR_RADIANES(M_BASE_Y_DEST, VAR_RADIANES)	{
float vSin				= sin(VAR_RADIANES);
float vCos				= cos(VAR_RADIANES);
float e00			= NBMATRIXP_E00(M_BASE_Y_DEST);
float e10			= NBMATRIXP_E10(M_BASE_Y_DEST);
NBMATRIXP_E00(M_BASE_Y_DEST)	= (e00 * vCos)	+ (NBMATRIXP_E01(M_BASE_Y_DEST) * vSin);
NBMATRIXP_E01(M_BASE_Y_DEST)	= (e00 * -vSin) + (NBMATRIXP_E01(M_BASE_Y_DEST) * vCos);
NBMATRIXP_E10(M_BASE_Y_DEST)	= (e10 * vCos)	+ (NBMATRIXP_E11(M_BASE_Y_DEST) * vSin);
NBMATRIXP_E11(M_BASE_Y_DEST)	= (e10 * -vSin) + (NBMATRIXP_E11(M_BASE_Y_DEST) * vCos);
}

#define NBMATRIZ_ROTAR_GRADOS(M_BASE_Y_DEST, VAR_GRADOS)		NBMATRIZ_ROTAR_RADIANES(M_BASE_Y_DEST, DEG_2_RAD(VAR_GRADOS))

/ *#define NBMATRIZ_ESTABLECER_SEGUN_TRANSFORMACIONES_GRADOS(MATRIZ_DESTINO, VAR_TRAS_X, VAR_TRAS_Y, VAR_ROT_GRADOS, VAR_ESCALA_X, VAR_ESCALA_Y)
 NBMATRIZ_ESTABLECER_IDENTIDAD(MATRIZ_DESTINO)
 NBMATRIZ_TRASLADAR(MATRIZ_DESTINO, VAR_TRAS_X, VAR_TRAS_Y);
 NBMATRIZ_ROTAR_GRADOS(MATRIZ_DESTINO, VAR_ROT_GRADOS);
 NBMATRIZ_ESCALAR(MATRIZ_DESTINO, VAR_ESCALA_X, VAR_ESCALA_Y);* /

#define NBMATRIZ_ESTABLECER_SEGUN_TRANSFORMACIONES_RADIANES(MATRIZ_DESTINO, VAR_TRAS_X, VAR_TRAS_Y, VAR_ROT_RADIANES, VAR_ESC_X, VAR_ESC_Y)
{
float vSin				= sin(VAR_ROT_RADIANES);
float vCos				= cos(VAR_ROT_RADIANES);
NBMATRIXP_E00(MATRIZ_DESTINO) = vCos * (VAR_ESC_X);
NBMATRIXP_E01(MATRIZ_DESTINO) = -vSin * (VAR_ESC_Y);
NBMATRIXP_E02(MATRIZ_DESTINO) = (VAR_TRAS_X);
NBMATRIXP_E10(MATRIZ_DESTINO) = vSin * (VAR_ESC_X);
NBMATRIXP_E11(MATRIZ_DESTINO) = vCos * (VAR_ESC_Y);
NBMATRIXP_E12(MATRIZ_DESTINO) = (VAR_TRAS_Y);
}

#define NBMATRIZ_ESTABLECER_SEGUN_TRANSFORMACIONES_GRADOS(MATRIZ_DESTINO, VAR_TRAS_X, VAR_TRAS_Y, VAR_ROT_GRADOS, VAR_ESC_X, VAR_ESC_Y)
NBMATRIZ_ESTABLECER_SEGUN_TRANSFORMACIONES_RADIANES(MATRIZ_DESTINO, VAR_TRAS_X, VAR_TRAS_Y, DEG_2_RAD(VAR_ROT_GRADOS), VAR_ESC_X, VAR_ESC_Y)

#define NBMATRIZ_ESTABLECER_SEGUN_TRANSFORMACIONES_ESCENA_OBJETO(MATRIZ_DESTINO, VAR_TRANSFORMACIONES_ESCENA_OBJ)
NBMATRIZ_ESTABLECER_SEGUN_TRANSFORMACIONES_GRADOS(MATRIZ_DESTINO, VAR_TRANSFORMACIONES_ESCENA_OBJ.trasladoX, VAR_TRANSFORMACIONES_ESCENA_OBJ.trasladoY, VAR_TRANSFORMACIONES_ESCENA_OBJ.rotacion, VAR_TRANSFORMACIONES_ESCENA_OBJ.escalaX, VAR_TRANSFORMACIONES_ESCENA_OBJ.escalaY)


*/
