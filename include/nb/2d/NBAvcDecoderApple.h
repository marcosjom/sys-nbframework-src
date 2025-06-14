//
//  NBAvcDecoderApple.h
//  nbframework
//
//  Created by Marcos Ortega on 9/21/20.
//

#ifndef NBAvcDecoderApple_h
#define NBAvcDecoderApple_h

#include "nb/NBFrameworkDefs.h"
#include "nb/2d/NBAvcDecoder.h"
#include "nb/2d/NBBitmap.h"

//-----
//- https://developer.apple.com/documentation/videotoolbox/vtdecompressionsession?language=objc
//-----

typedef struct STNBAvcDecoderApple_ {
	void* opaque;
} STNBAvcDecoderApple;

STNBAVDecompItf NBAvcDecoderApple_getItf(void);

void NBAvcDecoderApple_init(STNBAvcDecoderApple* obj);
void NBAvcDecoderApple_release(STNBAvcDecoderApple* obj);

//Context

void* NBAvcDecoderApple_ctxCreate(STNBAvcDecoderApple* obj);
void NBAvcDecoderApple_ctxDestroy(STNBAvcDecoderApple* obj, void* ctx);
BOOL NBAvcDecoderApple_ctxFeedUnit(STNBAvcDecoderApple* obj, void* ctx, void* data, const UI32 dataSz);
BOOL NBAvcDecoderApple_ctxGetBuffer(STNBAvcDecoderApple* obj, void* ctx, UI32* seqFilterAndDst, STNBBitmap* dst, const BOOL swapBuffersInsteadOfCloning);

#endif /* NBAvcDecoderApple_h */
