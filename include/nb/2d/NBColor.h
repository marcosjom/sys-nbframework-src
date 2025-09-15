//
//  NBColor.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/12/18.
//

#ifndef NBColor_h
#define NBColor_h

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBCompare.h"
#include "nb/core/NBStructMap.h"

#ifdef __cplusplus
extern "C" {
#endif

//Keeping compatyibility with AUFramework::NBColor
//ToDo: remove once AUFramework support ends.
#define NBColor     STNBColor
#define NBColorF    STNBColor
#define NBColor8    STNBColor8

#define NBColor_areEqual(ONE, OTHER)    ((ONE).r == (OTHER).r && (ONE).g == (OTHER).g && (ONE).b == (OTHER).b && (ONE).a == (OTHER).a)

#define STNBColor_Zero  { 0.f, 0.f, 0.f, 0.f }

	typedef struct STNBColor_ {
		union {
			struct {
				float r, g, b, a;
			};
			float c[4];
		};
	} STNBColor;
	
	const STNBStructMap* NBColor_getSharedStructMap(void);

#define NB_COLOR8_R_MSK             0xFF000000u    //1111-1111 0000-0000 0000-0000 0000-0000b
#define NB_COLOR8_R_BIT_FIRST       0x1000000u    //0000-0001 0000-0000 0000-0000 0000-0000b
#define NB_COLOR8_R_BIT_LAST        0x80000000u    //1000-0000 0000-0000 0000-0000 0000-0000b
#define NB_COLOR8_R_MSK_MAX         0xFFu        //
//
#define NB_COLOR8_G_MSK             0xFF0000u    //0000-0000 1111-1111 0000-0000 0000-0000b
#define NB_COLOR8_G_BIT_FIRST       0x10000u    //0000-0000 0000-0001 0000-0000 0000-0000b
#define NB_COLOR8_G_BIT_LAST        0x800000u    //0000-0000 1000-0000 0000-0000 0000-0000b
#define NB_COLOR8_G_MSK_MAX         0xFFu        //
//
#define NB_COLOR8_B_MSK             0xFFFF00u    //0000-0000 0000-0000 1111-1111 0000-0000b
#define NB_COLOR8_B_BIT_FIRST       0x100u        //0000-0000 0000-0000 0000-0001 0000-0000b
#define NB_COLOR8_B_BIT_LAST        0x8000u        //0000-0000 0000-0000 1000-0000 0000-0000b
#define NB_COLOR8_B_MSK_MAX         0xFFu        //
//
#define NB_COLOR8_A_MSK             0xFFu        //0000-0000 0000-0000 0000-0000 1111-1111b
#define NB_COLOR8_A_BIT_FIRST       0x1u        //0000-0000 0000-0000 0000-0000 0000-0001b
#define NB_COLOR8_A_BIT_LAST        0x80u        //0000-0000 0000-0000 0000-0000 1000-0000b
#define NB_COLOR8_A_MSK_MAX         0xFFu        //

#define NBColor8_getR(V32)          (((V32) / NB_COLOR8_R_BIT_FIRST) % (NB_COLOR8_R_MSK_MAX + 1u))
#define NBColor8_getG(V32)          (((V32) / NB_COLOR8_G_BIT_FIRST) % (NB_COLOR8_G_MSK_MAX + 1u))
#define NBColor8_getB(V32)          (((V32) / NB_COLOR8_B_BIT_FIRST) % (NB_COLOR8_B_MSK_MAX + 1u))
#define NBColor8_getA(V32)          (((V32) / NB_COLOR8_A_BIT_FIRST) % (NB_COLOR8_A_MSK_MAX + 1u))


#define STNBColor8_Zero  { 0u, 0u, 0u, 0u }

	typedef struct STNBColor8_ {
		union {
			struct {
				UI8 r, g, b, a;
			};
			UI8 c[4];
            UI32 v32;
		};
	} STNBColor8;
	
	const STNBStructMap* NBColor8_getSharedStructMap(void);


#define STNBColorI_Zero  { 0, 0, 0, 0 }

	typedef struct STNBColorI_ {
		union {
			struct {
				SI32 r, g, b, a;
			};
			SI32 c[4];
		};
	} STNBColorI;
	
	const STNBStructMap* NBColorI_getSharedStructMap(void);
	
	BOOL NBCompare_NBColor8(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif /* NBColor_h */
