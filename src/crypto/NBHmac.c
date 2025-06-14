//
//  NBHmac.cpp
//  AUFramework
//
//  Created by Marcos Ortega Morales on 11/12/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
#include "nb/crypto/NBHmac.h"
#include "nb/core/NBString.h"
//HMAC MD5

/*typedef struct STNBHmacMd5_ {
	STNBMd5		md5;
} STNBHmacMd5;*/

//

void NBHmacMd5_init(STNBHmacMd5* obj){
	obj->blockSz	= 64;
	NBMd5_init(&obj->hashIn);
	NBMd5_init(&obj->hashOut);
}

void NBHmacMd5_release(STNBHmacMd5* obj){
	NBMd5_release(&obj->hashIn);
	NBMd5_release(&obj->hashOut);
}

//Feed

void NBHmacMd5_feedStart(STNBHmacMd5* obj, const void* pKey, const UI32 keySz){
	NBMd5_release(&obj->hashIn);
	NBMd5_release(&obj->hashOut);
	//
	NBMd5_init(&obj->hashIn);
	NBMd5_init(&obj->hashOut);
	//
	NBASSERT(obj->blockSz == 64) //MD5 and SHA1 blocksSz are 64
	{
		STNBString key;
		NBString_initWithSz(&key, obj->blockSz + 8, 8, 0.10f);
		NBString_setBytes(&key, (const char*)pKey, keySz);
		//Short key (if largen than blockSz)
		if(key.length > obj->blockSz){
			const STNBMd5Hash kHash = NBMd5_getHashBytes(key.str, key.length);
			NBString_setBytes(&key, (const char*)&kHash.v, sizeof(kHash.v));
		}
		//Pad key (if shorter than blockSz)
		if(key.length < obj->blockSz){
			NBString_concatRepeatedByte(&key, '\0', (obj->blockSz - key.length));
		}
		NBASSERT(key.length == obj->blockSz)
		{
			const BYTE* keyB = (const BYTE*)key.str;
			BYTE* keyO = (BYTE*)NBMemory_alloc(obj->blockSz);
			BYTE* keyI = (BYTE*)NBMemory_alloc(obj->blockSz);
			SI32 i; for(i = 0; i < obj->blockSz; i++){
				keyO[i] = keyB[i] ^ 0x5c;
				keyI[i] = keyB[i] ^ 0x36;
			}
			NBMd5_feed(&obj->hashOut, keyO, obj->blockSz);
			NBMd5_feed(&obj->hashIn, keyI, obj->blockSz);
			NBMemory_free(keyO);
			NBMemory_free(keyI);
		}
		NBString_release(&key);
	}
}

void NBHmacMd5_feed(STNBHmacMd5* obj, const void* data, UI32 dataSz){
	NBASSERT(obj->blockSz == 64) //MD5 and SHA1 blocksSz are 64
	NBMd5_feed(&obj->hashIn, data, dataSz);
}

void NBHmacMd5_feedEnd(STNBHmacMd5* obj){
	NBASSERT(obj->blockSz == 64) //MD5 and SHA1 blocksSz are 64
	NBMd5_finish(&obj->hashIn);
	NBMd5_feed(&obj->hashOut, &obj->hashIn.hash.v, sizeof(obj->hashIn.hash.v));
	NBMd5_finish(&obj->hashOut);
}

STNBMd5Hash NBHmacMd5_getHash(const STNBHmacMd5* obj){
	NBASSERT(obj->blockSz == 64) //MD5 and SHA1 blocksSz are 64
	return obj->hashOut.hash;
}

STNBMd5HashHex NBHmacMd5_getHashHex(const STNBHmacMd5* obj){
	NBASSERT(obj->blockSz == 64) //MD5 and SHA1 blocksSz are 64
	return NBMd5_getHashHex(&obj->hashOut);
}

//One step calculation

STNBMd5Hash NBHmacMd5_getHashBytes(const void* key, const UI32 keySz, const void* data, const UI32 dataSz){
	STNBMd5Hash r;
	{
		STNBHmacMd5 hmac;
		NBHmacMd5_init(&hmac);
		NBHmacMd5_feedStart(&hmac, key, keySz);
		NBHmacMd5_feed(&hmac, data, dataSz);
		NBHmacMd5_feedEnd(&hmac);
		r = NBHmacMd5_getHash(&hmac);
		NBHmacMd5_release(&hmac);
	}
	return r;
}

STNBMd5HashHex NBHmacMd5_getHashHexBytes(const void* key, const UI32 keySz, const void* data, const UI32 dataSz){
	STNBMd5HashHex r;
	{
		STNBHmacMd5 hmac;
		NBHmacMd5_init(&hmac);
		NBHmacMd5_feedStart(&hmac, key, keySz);
		NBHmacMd5_feed(&hmac, data, dataSz);
		NBHmacMd5_feedEnd(&hmac);
		r = NBHmacMd5_getHashHex(&hmac);
		NBHmacMd5_release(&hmac);
	}
	return r;
}
