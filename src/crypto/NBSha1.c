//
//  NBSha1.cpp
//  AUFramework
//
//  Created by Marcos Ortega Morales on 11/12/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
#include "nb/crypto/NBSha1.h"

void NBSha1_init(STNBSha1* v){
	v->Length_Low			= 0;
	v->Length_High			= 0;
	v->Message_Block_Index	= 0;
	v->hash.v[0]			= 0x67452301;
	v->hash.v[1]			= 0xEFCDAB89;
	v->hash.v[2]			= 0x98BADCFE;
	v->hash.v[3]			= 0x10325476;
	v->hash.v[4]			= 0xC3D2E1F0;
	v->Computed				= 0;
	v->Corrupted			= 0;
}

void NBSha1_release(STNBSha1* v){
	//Nothing
}
	
void NBSha1_feedStart(STNBSha1* v){
    NBSha1_init(v);
}

SI32 NBSha1_feedEnd(STNBSha1* v){
	if (v->Corrupted){
		return 0;
	}
	if (!v->Computed){
	    NBSha1_PadMessage(v);
		v->Computed = 1;
		//Invert bytes order
		{
			const STNBSha1Hash cpy = v->hash;
			const BYTE* src = (const BYTE*)cpy.v;
			BYTE* dst = (BYTE*)v->hash.v;
			dst[3] = src[0];
			dst[2] = src[1];
			dst[1] = src[2];
			dst[0] = src[3];
			//
			dst[7] = src[4];
			dst[6] = src[5];
			dst[5] = src[6];
			dst[4] = src[7];
			//
			dst[11] = src[8];
			dst[10] = src[9];
			dst[9] = src[10];
			dst[8] = src[11];
			//
			dst[15] = src[12];
			dst[14] = src[13];
			dst[13] = src[14];
			dst[12] = src[15];
			//
			dst[19] = src[16];
			dst[18] = src[17];
			dst[17] = src[18];
			dst[16] = src[19];
		}
	}
	return 1;
}

STNBSha1HashHex NBSha1_getHashHex(const STNBSha1* v){
	STNBSha1HashHex r;
	if(v->Computed){
	    NBSha1_hashBinToHexChars((const BYTE*)&v->hash.v[0], r.v);
	} else {
		r.v[0] = '\0';
	}
	return r;
}

//

STNBSha1Hash NBSha1_getHashBytes(const void* data, const UI32 dataSz){
	STNBSha1Hash r;
	STNBSha1 sha1;
	NBSha1_init(&sha1);
	NBSha1_feed(&sha1, data, dataSz);
	NBSha1_feedEnd(&sha1);
	r = sha1.hash;
	NBSha1_release(&sha1);
	return r;
}

STNBSha1HashHex NBSha1_getHashHexBytes(const void* data, const UI32 dataSz){
	STNBSha1HashHex r;
	STNBSha1 sha1;
	NBSha1_init(&sha1);
	NBSha1_feed(&sha1, data, dataSz);
	NBSha1_feedEnd(&sha1);
	r = NBSha1_getHashHex(&sha1);
	NBSha1_release(&sha1);
	return r;
}

STNBSha1HashHex NBSha1_getHashHexStr(const char* str){
	STNBSha1HashHex r;
	STNBSha1 sha1;
	NBSha1_init(&sha1);
	{
		UI32 strSz = 0;
		if(str != NULL){
			while(str[strSz] != '\0'){
				strSz++;
			}
			NBSha1_feed(&sha1, str, strSz);
		}
	}
	NBSha1_feedEnd(&sha1);
	r = NBSha1_getHashHex(&sha1);
	NBSha1_release(&sha1);
	return r;
}

void NBSha1_feed(STNBSha1* v, const void* pData, UI32 dataSz){
	const BYTE* data = (const BYTE*)pData;
	if (!dataSz){
		return;
	}
	if (v->Computed || v->Corrupted){
		v->Corrupted = 1;
		return;
	}
	while(dataSz-- && !v->Corrupted){
		v->Message_Block[v->Message_Block_Index++] = (*data & 0xFF);
		v->Length_Low += 8;
		v->Length_Low &= 0xFFFFFFFF;               // Force it to 32 bits
		if (v->Length_Low == 0){
			v->Length_High++;
			v->Length_High &= 0xFFFFFFFF;          // Force it to 32 bits
			if (v->Length_High == 0){
				v->Corrupted = 1;              	 // Message is too long
			}
		}
		if (v->Message_Block_Index == 64){
		    NBSha1_ProcessMessageBlock(v);
		}
		data++;
	}
}

void NBSha1_ProcessMessageBlock(STNBSha1* v){
	const UI32 K[] = {0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6};
	SI32    t;                          // Loop counter
	UI32    temp;                       // Temporary word value
	UI32    W[80];                      // Word sequence
	UI32    A, B, C, D, E;              // Word buffers
	//inicializar
	for(t = 0; t < 16; t++){
		W[t] = ((UI32) v->Message_Block[t * 4]) << 24;
		W[t] |= ((UI32) v->Message_Block[t * 4 + 1]) << 16;
		W[t] |= ((UI32) v->Message_Block[t * 4 + 2]) << 8;
		W[t] |= ((UI32) v->Message_Block[t * 4 + 3]);
	}
	for(t = 16; t < 80; t++){
		W[t] = NBSha1_CircularShift(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
	}
	A = v->hash.v[0];
	B = v->hash.v[1];
	C = v->hash.v[2];
	D = v->hash.v[3];
	E = v->hash.v[4];
	for(t = 0; t < 20; t++){
		temp = NBSha1_CircularShift(5,A) + ((B & C) | ((~B) & D)) + E + W[t] + K[0];
		temp &= 0xFFFFFFFF;
		E = D;
		D = C;
		C = NBSha1_CircularShift(30,B);
		B = A;
		A = temp;
	}
	for(t = 20; t < 40; t++){
		temp = NBSha1_CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[1];
		temp &= 0xFFFFFFFF;
		E = D;
		D = C;
		C = NBSha1_CircularShift(30,B);
		B = A;
		A = temp;
	}
	for(t = 40; t < 60; t++){
		temp = NBSha1_CircularShift(5,A) + ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
		temp &= 0xFFFFFFFF;
		E = D;
		D = C;
		C = NBSha1_CircularShift(30,B);
		B = A;
		A = temp;
	}
	for(t = 60; t < 80; t++){
		temp = NBSha1_CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[3];
		temp &= 0xFFFFFFFF;
		E = D;
		D = C;
		C = NBSha1_CircularShift(30,B);
		B = A;
		A = temp;
	}
	v->hash.v[0] = (v->hash.v[0] + A) & 0xFFFFFFFF;
	v->hash.v[1] = (v->hash.v[1] + B) & 0xFFFFFFFF;
	v->hash.v[2] = (v->hash.v[2] + C) & 0xFFFFFFFF;
	v->hash.v[3] = (v->hash.v[3] + D) & 0xFFFFFFFF;
	v->hash.v[4] = (v->hash.v[4] + E) & 0xFFFFFFFF;
	v->Message_Block_Index = 0;
}

void NBSha1_PadMessage(STNBSha1* v){
	if (v->Message_Block_Index > 55){
		v->Message_Block[v->Message_Block_Index++] = 0x80;
		while(v->Message_Block_Index < 64){
			v->Message_Block[v->Message_Block_Index++] = 0;
		}
	    NBSha1_ProcessMessageBlock(v);
		while(v->Message_Block_Index < 56){
			v->Message_Block[v->Message_Block_Index++] = 0;
		}
	} else {
		v->Message_Block[v->Message_Block_Index++] = 0x80;
		while(v->Message_Block_Index < 56){
			v->Message_Block[v->Message_Block_Index++] = 0;
		}
	}
	v->Message_Block[56] = (v->Length_High >> 24) & 0xFF;
	v->Message_Block[57] = (v->Length_High >> 16) & 0xFF;
	v->Message_Block[58] = (v->Length_High >> 8) & 0xFF;
	v->Message_Block[59] = (v->Length_High) & 0xFF;
	v->Message_Block[60] = (v->Length_Low >> 24) & 0xFF;
	v->Message_Block[61] = (v->Length_Low >> 16) & 0xFF;
	v->Message_Block[62] = (v->Length_Low >> 8) & 0xFF;
	v->Message_Block[63] = (v->Length_Low) & 0xFF;
    NBSha1_ProcessMessageBlock(v);
}

UI32 NBSha1_CircularShift(SI32 bits, UI32 word){
	return ((word << bits) & 0xFFFFFFFF) | ((word & 0xFFFFFFFF) >> (32-bits));
}

void NBSha1_hashBinToHexChars(const BYTE* hash20Bytes, char* dst41Bytes){
	static const char* hexChars = "0123456789abcdef"; //"0123456789ABCDEF"
	//
	dst41Bytes[0] = hexChars[hash20Bytes[0] >> 4]; dst41Bytes[1] = hexChars[hash20Bytes[0] & 15];
	dst41Bytes[2] = hexChars[hash20Bytes[1] >> 4]; dst41Bytes[3] = hexChars[hash20Bytes[1] & 15];
	dst41Bytes[4] = hexChars[hash20Bytes[2] >> 4]; dst41Bytes[5] = hexChars[hash20Bytes[2] & 15];
	dst41Bytes[6] = hexChars[hash20Bytes[3] >> 4]; dst41Bytes[7] = hexChars[hash20Bytes[3] & 15];
	//
	dst41Bytes[8] = hexChars[hash20Bytes[4] >> 4]; dst41Bytes[9] = hexChars[hash20Bytes[4] & 15];
	dst41Bytes[10] = hexChars[hash20Bytes[5] >> 4]; dst41Bytes[11] = hexChars[hash20Bytes[5] & 15];
	dst41Bytes[12] = hexChars[hash20Bytes[6] >> 4]; dst41Bytes[13] = hexChars[hash20Bytes[6] & 15];
	dst41Bytes[14] = hexChars[hash20Bytes[7] >> 4]; dst41Bytes[15] = hexChars[hash20Bytes[7] & 15];
	//
	dst41Bytes[16] = hexChars[hash20Bytes[8] >> 4]; dst41Bytes[17] = hexChars[hash20Bytes[8] & 15];
	dst41Bytes[18] = hexChars[hash20Bytes[9] >> 4]; dst41Bytes[19] = hexChars[hash20Bytes[9] & 15];
	dst41Bytes[20] = hexChars[hash20Bytes[10] >> 4]; dst41Bytes[21] = hexChars[hash20Bytes[10] & 15];
	dst41Bytes[22] = hexChars[hash20Bytes[11] >> 4]; dst41Bytes[23] = hexChars[hash20Bytes[11] & 15];
	//
	dst41Bytes[24] = hexChars[hash20Bytes[12] >> 4]; dst41Bytes[25] = hexChars[hash20Bytes[12] & 15];
	dst41Bytes[26] = hexChars[hash20Bytes[13] >> 4]; dst41Bytes[27] = hexChars[hash20Bytes[13] & 15];
	dst41Bytes[28] = hexChars[hash20Bytes[14] >> 4]; dst41Bytes[29] = hexChars[hash20Bytes[14] & 15];
	dst41Bytes[30] = hexChars[hash20Bytes[15] >> 4]; dst41Bytes[31] = hexChars[hash20Bytes[15] & 15];
	//
	dst41Bytes[32] = hexChars[hash20Bytes[16] >> 4]; dst41Bytes[33] = hexChars[hash20Bytes[16] & 15];
	dst41Bytes[34] = hexChars[hash20Bytes[17] >> 4]; dst41Bytes[35] = hexChars[hash20Bytes[17] & 15];
	dst41Bytes[36] = hexChars[hash20Bytes[18] >> 4]; dst41Bytes[37] = hexChars[hash20Bytes[18] & 15];
	dst41Bytes[38] = hexChars[hash20Bytes[19] >> 4]; dst41Bytes[39] = hexChars[hash20Bytes[19] & 15];
	//
	dst41Bytes[40] = '\0';
}
