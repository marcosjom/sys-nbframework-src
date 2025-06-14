//
//  NBSha1.cpp
//  AUFramework
//
//  Created by Marcos Ortega Morales on 11/12/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
#include "nb/crypto/NBRc4A.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBString.h"

void NBRc4A_init(STNBRc4A* obj){
	//Nothing
}

void NBRc4A_release(STNBRc4A* obj){
	//Nothing
}

//
#define NBRC4A_SWAP(OBJ_PTR, TMP_VAR) TMP_VAR = OBJ_PTR->S[OBJ_PTR->i]; OBJ_PTR->S[OBJ_PTR->i] = OBJ_PTR->S[OBJ_PTR->j]; OBJ_PTR->S[OBJ_PTR->j] = TMP_VAR;
/*void NBRc4A_swap_(BYTE *S, UI32 i, UI32 j){
    BYTE temp = S[i];
    S[i] = S[j];
    S[j] = temp;
}*/

/* PRGA */
BYTE NBRc4A_output_(STNBRc4A* obj) {
	BYTE tmp;
    obj->i = (obj->i + 1) & 255;
    obj->j = (obj->j + obj->S[obj->i]) & 255;
    NBRC4A_SWAP(obj, tmp);
    return obj->S[(obj->S[obj->i] + obj->S[obj->j]) & 255];
}

//

/* KSA */
void NBRc4A_feedStart(STNBRc4A* obj, const void* pKey, const UI32 keySz){
	BYTE tmp;
	const BYTE* key = (const BYTE*)pKey;
	for(obj->i = 0; obj->i < 256; obj->i++){
		obj->S[obj->i] = (BYTE)obj->i;
	}
	for (obj->i = obj->j = 0; obj->i < 256; obj->i++) {
		obj->j = (obj->j + key[obj->i % keySz] + obj->S[obj->i]) & 255;
		NBRC4A_SWAP(obj, tmp);
	}
	obj->i = obj->j = 0;
}

void NBRc4A_feed(STNBRc4A* obj, const void* pData, const UI32 dataSz, STNBString* dst){
	if(dst != NULL){
		const BYTE* data = (const BYTE*)pData;
		SI32 i; for(i = 0; i < dataSz; i++){
			NBString_concatByte(dst, data[i] ^ NBRc4A_output_(obj));
		}
	}
}

//

void NBRc4A_concat(const void* key, const UI32 keySz, const void* data, const UI32 dataSz, STNBString* dst){
	STNBRc4A rc;
	NBRc4A_init(&rc);
	NBRc4A_feedStart(&rc, key, keySz);
	NBRc4A_feed(&rc, data, dataSz, dst);
	NBRc4A_release(&rc);
}

void NBRc4A_concatHex(const void* key, const UI32 keySz, const void* data, const UI32 dataSz, STNBString* dst){
	STNBString tmp;
	NBString_init(&tmp);
	{
		STNBRc4A rc;
		NBRc4A_init(&rc);
		NBRc4A_feedStart(&rc, key, keySz);
		NBRc4A_feed(&rc, data, dataSz, &tmp);
		if(dst != NULL){
			NBString_concatBytesHex(dst, tmp.str, tmp.length);
		}
		NBRc4A_release(&rc);
	}
	NBString_release(&tmp);
}
