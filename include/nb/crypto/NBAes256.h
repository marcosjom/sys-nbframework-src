//
//  NBCryptAES.h
//  AUFramework
//
//  Created by Marcos Ortega Morales on 11/12/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef NB_AES_256_H
#define NB_AES_256_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef enum ENAes256Op_ {
		ENAes256Op_Encrypt = 0,
		ENAes256Op_Decrypt,
	} ENAes256Op;
	
	typedef struct STNBAes256_ {
		void* opaque;
	} STNBAes256;
	
	void NBAes256_init(STNBAes256* obj);
	void NBAes256_release(STNBAes256* obj);
	//
	UI32 NBAes256_blockSize(void);
	UI32 NBAes256_encryptedSize(const UI32 plainSize);
	BOOL NBAes256_feedStart(STNBAes256* obj, const ENAes256Op op, const char* pass, const UI32 passSz, const BYTE* salt, const UI8 saltSz, const UI16 iterations);
	SI32 NBAes256_feed(STNBAes256* obj, BYTE* outBuff, const SI32 outBuffSz, const BYTE* plainData, SI32 plainDataSz);
	SI32 NBAes256_feedEnd(STNBAes256* obj, BYTE* outBuff, const SI32 outBuffSz);
	//
	BOOL NBAes256_aesEncrypt(const void* plainData, const UI32 plainDataSz, const char* pass, const UI32 passSz, const void* salt, const UI8 saltSz, const UI16 iterations, STNBString* dst);
	BOOL NBAes256_aesDecrypt(const void* cryptdData, const UI32 cryptdDataSz, const char* pass, const UI32 passSz, const void* salt, const UI8 saltSz, const UI16 iterations, STNBString* dst);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
