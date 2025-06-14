//
//  NBMatrix.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/6/18.
//

#ifndef NBMatrix_h
#define NBMatrix_h

#define NBMATRIX_E00(MATRIZ)	MATRIZ.e[0]
#define NBMATRIX_E01(MATRIZ)	MATRIZ.e[1]
#define NBMATRIX_E02(MATRIZ)	MATRIZ.e[2]
#define NBMATRIX_E10(MATRIZ)	MATRIZ.e[3]
#define NBMATRIX_E11(MATRIZ)	MATRIZ.e[4]
#define NBMATRIX_E12(MATRIZ)	MATRIZ.e[5]
#define NBMATRIX_E20(MATRIZ)	0.0f //MATRIZ.e[6] //0.0f
#define NBMATRIX_E21(MATRIZ)	0.0f //MATRIZ.e[7] //0.0f
#define NBMATRIX_E22(MATRIZ)	1.0f //MATRIZ.e[8] //1.0f

#define NBMATRIXP_E00(MATRIZ)	MATRIZ->e[0]
#define NBMATRIXP_E01(MATRIZ)	MATRIZ->e[1]
#define NBMATRIXP_E02(MATRIZ)	MATRIZ->e[2]
#define NBMATRIXP_E10(MATRIZ)	MATRIZ->e[3]
#define NBMATRIXP_E11(MATRIZ)	MATRIZ->e[4]
#define NBMATRIXP_E12(MATRIZ)	MATRIZ->e[5]
#define NBMATRIXP_E20(MATRIZ)	0.0f //MATRIZ.e[6] //0.0f
#define NBMATRIXP_E21(MATRIZ)	0.0f //MATRIZ.e[7] //0.0f
#define NBMATRIXP_E22(MATRIZ)	1.0f //MATRIZ.e[8] //1.0f

#include "nb/NBFrameworkDefs.h"
#include "nb/2d/NBPoint.h"
#include "nb/2d/NBSize.h"
#include "nb/2d/NBAABox.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct STNBMatrix_ {
		union {
			struct {
				//row 0
				float e00;
				float e01;
				float e02;
				//row 1
				float e10;
				float e11;
				float e12;
				//row 2
				//0.0f
				//0.0f
				//1.0f
			};
			//HLSL (DirectX Shader Language)
			struct {
				//row 0
				float _m00;
				float _m01;
				float _m02;
				//row 1
				float _m10;
				float _m11;
				float _m12;
				//row 2
				//0.0f
				//0.0f
				//1.0f
			};
			float r[3][2];	//for a 3x3 matrix, the last 3 elemments are allways asummed to be [0, 0, 1].
			float e[6];		//for a 3x3 matrix, the last 3 elemments are allways asummed to be [0, 0, 1].
		};
	} STNBMatrix, STNBMatrix2D;
	
	//Transform
	void NBMatrix_setIdentity(STNBMatrix* obj);
	void NBMatrix_translate(STNBMatrix* obj, const float tx, const float ty);
	void NBMatrix_translateWithPoint(STNBMatrix* obj, const STNBPoint t);
	void NBMatrix_scale(STNBMatrix* obj, const float sx, const float sy);
	void NBMatrix_scaleWithSize(STNBMatrix* obj, const STNBSize s);
	void NBMatrix_rotate(STNBMatrix* obj, const float rad);
	void NBMatrix_rotateDeg(STNBMatrix* obj, const float deg);
	STNBMatrix NBMatrix_multiply(const STNBMatrix* one, const STNBMatrix* other);
	
	//Calculate
	float NBMatrix_determinant(const STNBMatrix* obj);
	STNBMatrix NBMatrix_inverse(const STNBMatrix* obj);
	STNBMatrix NBMatrix_fromTransforms(const STNBPoint traslation, const float radRot, const STNBSize scale);
	
	//Apply
	STNBPoint NBMatrix_applyToPoint(const STNBMatrix* obj, const STNBPoint p);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBMatrix_h */
