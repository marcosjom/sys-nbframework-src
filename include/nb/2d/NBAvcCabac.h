//
//  NBAvcCabac.h
//  nbframework
//
//  Created by Marcos Ortega on 9/21/20.
//

#ifndef NBAvcCabac_h
#define NBAvcCabac_h

#include "nb/NBFrameworkDefs.h"

typedef enum ENNBCabacInitGrp_ {
	ENNBCabacInitGrp_I_SI_slice = 0,
	ENNBCabacInitGrp_init_idc_0,
	ENNBCabacInitGrp_init_idc_1,
	ENNBCabacInitGrp_init_idc_2,
	//
	ENNBCabacInitGrp_Count
} ENNBCabacInitGrp;

BOOL NBAvcCabac_getMN(const SI16 ctxIdx, const ENNBCabacInitGrp iGrp, SI8* dstM, SI8* dstN);

#endif /* NBAvcCabac_h */
