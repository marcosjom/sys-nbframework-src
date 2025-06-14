//
//  NBHmac.h
//  AUFramework
//
//  Created by Marcos Ortega Morales on 11/12/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef NB_HMAC_H
#define NB_HMAC_H

#include "nb/NBFrameworkDefs.h"
#include "nb/crypto/NBMd5.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	//HMAC MD5

	typedef struct STNBHmacMd5_ {
		UI32		blockSz;
		STNBMd5		hashIn;
		STNBMd5		hashOut;
	} STNBHmacMd5;

	//
	void			NBHmacMd5_init(STNBHmacMd5* obj);
	void			NBHmacMd5_release(STNBHmacMd5* obj);

	//Feed
	void			NBHmacMd5_feedStart(STNBHmacMd5* obj, const void* key, const UI32 keySz);
	void			NBHmacMd5_feed(STNBHmacMd5* obj, const void* data, UI32 dataSz);
	void			NBHmacMd5_feedEnd(STNBHmacMd5* obj);
	STNBMd5Hash		NBHmacMd5_getHash(const STNBHmacMd5* obj);
	STNBMd5HashHex	NBHmacMd5_getHashHex(const STNBHmacMd5* obj);

	//One step calculation
	STNBMd5Hash		NBHmacMd5_getHashBytes(const void* key, const UI32 keySz, const void* data, const UI32 dataSz);
	STNBMd5HashHex	NBHmacMd5_getHashHexBytes(const void* key, const UI32 keySz, const void* data, const UI32 dataSz);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
