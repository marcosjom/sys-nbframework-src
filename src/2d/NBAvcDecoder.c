//
//  NBAvcDecoder.c
//  nbframework
//
//  Created by Marcos Ortega on 9/21/20.
//

#include "nb/NBFrameworkPch.h"
#include "nb/2d/NBAvcDecoder.h"

/*BOOL NBAvcDecoder_getMN(const SI16 ctxIdx, const ENNBCabacInitGrp iGrp, SI8* dstM, SI8* dstN){
	BOOL r = FALSE;
	NBASSERT(ctxIdx >= 0 && ctxIdx < WELS_CONTEXT_COUNT)
	NBASSERT(iGrp >= 0 && iGrp < ENNBCabacInitGrp_Count)
	if(ctxIdx >= 0 && ctxIdx < WELS_CONTEXT_COUNT && iGrp >= 0 && iGrp < ENNBCabacInitGrp_Count){
		const SI8* mPtr = &_cabacContextIdx[ctxIdx][iGrp][0];
		if(dstM != NULL) *dstM = *mPtr;
		if(dstN != NULL) *dstN = *(mPtr + 1);
		r = TRUE;
	}
	return r;
}*/
