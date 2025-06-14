//
//  NBAvcDecoder.h
//  nbframework
//
//  Created by Marcos Ortega on 9/21/20.
//

#ifndef NBAvcDecoder_h
#define NBAvcDecoder_h

#include "nb/NBFrameworkDefs.h"

typedef struct STNBAVDecompItf_ {
	//
	void*	param;
	void	(*create)(void** obj);
	void	(*destroy)(void** obj);
	//Context
	void*	(*ctxCreate)(void* obj);
	void	(*ctxDestroy)(void* obj, void* ctx);
	BOOL	(*ctxFeedUnit)(void* obj, void* ctx, void* data, const UI32 dataSz);
} STNBAVDecompItf;

//BOOL NBAvcDecoder_getMN(const SI16 ctxIdx, const ENNBCabacInitGrp iGrp, SI8* dstM, SI8* dstN);

#endif /* NBAvcDecoder_h */
