//
//  NBSha1.cpp
//  AUFramework
//
//  Created by Marcos Ortega Morales on 11/12/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
#include "nb/crypto/NBMd5.h"
#include "nb/core/NBMemory.h"

/*
 **********************************************************************
 ** md5.c                                                            **
 ** RSA Data Security, Inc. MD5 Message Digest Algorithm             **
 ** Created: 2/17/90 RLR                                             **
 ** Revised: 1/91 SRD,AJ,BSK,JT Reference C Version                  **
 **********************************************************************
 */

/*
 **********************************************************************
 ** Copyright (C) 1990, RSA Data Security, Inc. All rights reserved. **
 **                                                                  **
 ** License to copy and use this software is granted provided that   **
 ** it is identified as the "RSA Data Security, Inc. MD5 Message     **
 ** Digest Algorithm" in all material mentioning or referencing this **
 ** software or this function.                                       **
 **                                                                  **
 ** License is also granted to make and use derivative works         **
 ** provided that such works are identified as "derived from the RSA **
 ** Data Security, Inc. MD5 Message Digest Algorithm" in all         **
 ** material mentioning or referencing the derived work.             **
 **                                                                  **
 ** RSA Data Security, Inc. makes no representations concerning      **
 ** either the merchantability of this software or the suitability   **
 ** of this software for any particular purpose.  It is provided "as **
 ** is" without express or implied warranty of any kind.             **
 **                                                                  **
 ** These notices must be retained in any copies of any part of this **
 ** documentation and/or software.                                   **
 **********************************************************************
 */

/* -- include the following line if the md5.h header file is separate -- */
/* #include "md5.h" */

static unsigned char PADDING[64] = {
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* F, G and H are basic MD5 functions: selection, majority, parity */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4 */
/* Rotation is separate from addition to prevent recomputation */
#define FF(a, b, c, d, x, s, ac) \
{(a) += F ((b), (c), (d)) + (x) + (UI32)(ac); \
(a) = ROTATE_LEFT ((a), (s)); \
(a) += (b); \
}
#define GG(a, b, c, d, x, s, ac) \
{(a) += G ((b), (c), (d)) + (x) + (UI32)(ac); \
(a) = ROTATE_LEFT ((a), (s)); \
(a) += (b); \
}
#define HH(a, b, c, d, x, s, ac) \
{(a) += H ((b), (c), (d)) + (x) + (UI32)(ac); \
(a) = ROTATE_LEFT ((a), (s)); \
(a) += (b); \
}
#define II(a, b, c, d, x, s, ac) \
{(a) += I ((b), (c), (d)) + (x) + (UI32)(ac); \
(a) = ROTATE_LEFT ((a), (s)); \
(a) += (b); \
}

//Basic MD5 step. Transform buf based on in.
static void NBMd5_transform(UI32 *buf, UI32 *in){
	UI32 a = buf[0], b = buf[1], c = buf[2], d = buf[3];
	/* Round 1 */
#	define S11 7
#	define S12 12
#	define S13 17
#	define S14 22
	FF ( a, b, c, d, in[ 0], S11, 3614090360u); /* 1 */
	FF ( d, a, b, c, in[ 1], S12, 3905402710u); /* 2 */
	FF ( c, d, a, b, in[ 2], S13,  606105819u); /* 3 */
	FF ( b, c, d, a, in[ 3], S14, 3250441966u); /* 4 */
	FF ( a, b, c, d, in[ 4], S11, 4118548399u); /* 5 */
	FF ( d, a, b, c, in[ 5], S12, 1200080426u); /* 6 */
	FF ( c, d, a, b, in[ 6], S13, 2821735955u); /* 7 */
	FF ( b, c, d, a, in[ 7], S14, 4249261313u); /* 8 */
	FF ( a, b, c, d, in[ 8], S11, 1770035416u); /* 9 */
	FF ( d, a, b, c, in[ 9], S12, 2336552879u); /* 10 */
	FF ( c, d, a, b, in[10], S13, 4294925233u); /* 11 */
	FF ( b, c, d, a, in[11], S14, 2304563134u); /* 12 */
	FF ( a, b, c, d, in[12], S11, 1804603682u); /* 13 */
	FF ( d, a, b, c, in[13], S12, 4254626195u); /* 14 */
	FF ( c, d, a, b, in[14], S13, 2792965006u); /* 15 */
	FF ( b, c, d, a, in[15], S14, 1236535329u); /* 16 */
	/* Round 2 */
#	define S21 5
#	define S22 9
#	define S23 14
#	define S24 20
	GG ( a, b, c, d, in[ 1], S21, 4129170786u); /* 17 */
	GG ( d, a, b, c, in[ 6], S22, 3225465664u); /* 18 */
	GG ( c, d, a, b, in[11], S23,  643717713u); /* 19 */
	GG ( b, c, d, a, in[ 0], S24, 3921069994u); /* 20 */
	GG ( a, b, c, d, in[ 5], S21, 3593408605u); /* 21 */
	GG ( d, a, b, c, in[10], S22,   38016083u); /* 22 */
	GG ( c, d, a, b, in[15], S23, 3634488961u); /* 23 */
	GG ( b, c, d, a, in[ 4], S24, 3889429448u); /* 24 */
	GG ( a, b, c, d, in[ 9], S21,  568446438u); /* 25 */
	GG ( d, a, b, c, in[14], S22, 3275163606u); /* 26 */
	GG ( c, d, a, b, in[ 3], S23, 4107603335u); /* 27 */
	GG ( b, c, d, a, in[ 8], S24, 1163531501u); /* 28 */
	GG ( a, b, c, d, in[13], S21, 2850285829u); /* 29 */
	GG ( d, a, b, c, in[ 2], S22, 4243563512u); /* 30 */
	GG ( c, d, a, b, in[ 7], S23, 1735328473u); /* 31 */
	GG ( b, c, d, a, in[12], S24, 2368359562u); /* 32 */
	/* Round 3 */
#	define S31 4
#	define S32 11
#	define S33 16
#	define S34 23
	HH ( a, b, c, d, in[ 5], S31, 4294588738u); /* 33 */
	HH ( d, a, b, c, in[ 8], S32, 2272392833u); /* 34 */
	HH ( c, d, a, b, in[11], S33, 1839030562u); /* 35 */
	HH ( b, c, d, a, in[14], S34, 4259657740u); /* 36 */
	HH ( a, b, c, d, in[ 1], S31, 2763975236u); /* 37 */
	HH ( d, a, b, c, in[ 4], S32, 1272893353u); /* 38 */
	HH ( c, d, a, b, in[ 7], S33, 4139469664u); /* 39 */
	HH ( b, c, d, a, in[10], S34, 3200236656u); /* 40 */
	HH ( a, b, c, d, in[13], S31,  681279174u); /* 41 */
	HH ( d, a, b, c, in[ 0], S32, 3936430074u); /* 42 */
	HH ( c, d, a, b, in[ 3], S33, 3572445317u); /* 43 */
	HH ( b, c, d, a, in[ 6], S34,   76029189u); /* 44 */
	HH ( a, b, c, d, in[ 9], S31, 3654602809u); /* 45 */
	HH ( d, a, b, c, in[12], S32, 3873151461u); /* 46 */
	HH ( c, d, a, b, in[15], S33,  530742520u); /* 47 */
	HH ( b, c, d, a, in[ 2], S34, 3299628645u); /* 48 */
	/* Round 4 */
#	define S41 6
#	define S42 10
#	define S43 15
#	define S44 21
	II ( a, b, c, d, in[ 0], S41, 4096336452u); /* 49 */
	II ( d, a, b, c, in[ 7], S42, 1126891415u); /* 50 */
	II ( c, d, a, b, in[14], S43, 2878612391u); /* 51 */
	II ( b, c, d, a, in[ 5], S44, 4237533241u); /* 52 */
	II ( a, b, c, d, in[12], S41, 1700485571u); /* 53 */
	II ( d, a, b, c, in[ 3], S42, 2399980690u); /* 54 */
	II ( c, d, a, b, in[10], S43, 4293915773u); /* 55 */
	II ( b, c, d, a, in[ 1], S44, 2240044497u); /* 56 */
	II ( a, b, c, d, in[ 8], S41, 1873313359u); /* 57 */
	II ( d, a, b, c, in[15], S42, 4264355552u); /* 58 */
	II ( c, d, a, b, in[ 6], S43, 2734768916u); /* 59 */
	II ( b, c, d, a, in[13], S44, 1309151649u); /* 60 */
	II ( a, b, c, d, in[ 4], S41, 4149444226u); /* 61 */
	II ( d, a, b, c, in[11], S42, 3174756917u); /* 62 */
	II ( c, d, a, b, in[ 2], S43,  718787259u); /* 63 */
	II ( b, c, d, a, in[ 9], S44, 3951481745u); /* 64 */
	//
	buf[0] += a;
	buf[1] += b;
	buf[2] += c;
	buf[3] += d;
}

//

void NBMd5_init(STNBMd5* obj){
	obj->i[0] = obj->i[1] = (UI32)0;
	//Load magic initialization constants.
	obj->buf[0] = (UI32)0x67452301;
	obj->buf[1] = (UI32)0xefcdab89;
	obj->buf[2] = (UI32)0x98badcfe;
	obj->buf[3] = (UI32)0x10325476;
}

void NBMd5_release(STNBMd5* obj){
	//Nothing
}

void NBMd5_feed(STNBMd5 *obj, const void* pInBuf, UI32 inLen){
	UI32 in[16];
	int mdi;
	unsigned int i, ii;
	
	const BYTE* inBuf = (const BYTE*)pInBuf;
	
	/* compute number of bytes mod 64 */
	mdi = (int)((obj->i[0] >> 3) & 0x3F);
	
	/* update number of bits */
	if ((obj->i[0] + ((UI32)inLen << 3)) < obj->i[0])
		obj->i[1]++;
	obj->i[0] += ((UI32)inLen << 3);
	obj->i[1] += ((UI32)inLen >> 29);
	
	while (inLen--) {
		/* add new character to buffer, increment mdi */
		obj->in[mdi++] = *inBuf++;
		
		/* transform if necessary */
		if (mdi == 0x40) {
			for (i = 0, ii = 0; i < 16; i++, ii += 4)
				in[i] = (((UI32)obj->in[ii+3]) << 24) |
				(((UI32)obj->in[ii+2]) << 16) |
				(((UI32)obj->in[ii+1]) << 8) |
				((UI32)obj->in[ii]);
		    NBMd5_transform(obj->buf, in);
			mdi = 0;
		}
	}
}

void NBMd5_finish(STNBMd5 *obj){
	UI32 in[16];
	int mdi;
	unsigned int i, ii;
	unsigned int padLen;
	
	/* save number of bits */
	in[14] = obj->i[0];
	in[15] = obj->i[1];
	
	/* compute number of bytes mod 64 */
	mdi = (int)((obj->i[0] >> 3) & 0x3F);
	
	/* pad out to 56 mod 64 */
	padLen = (mdi < 56) ? (56 - mdi) : (120 - mdi);
    NBMd5_feed (obj, PADDING, padLen);
	
	/* append length in bits and transform */
	for (i = 0, ii = 0; i < 14; i++, ii += 4)
		in[i] = (((UI32)obj->in[ii+3]) << 24) |
		(((UI32)obj->in[ii+2]) << 16) |
		(((UI32)obj->in[ii+1]) << 8) |
		((UI32)obj->in[ii]);
    NBMd5_transform(obj->buf, in);
	
	/* store buffer in digest */
	for (i = 0, ii = 0; i < 4; i++, ii += 4) {
		obj->hash.v[ii] = (unsigned char)(obj->buf[i] & 0xFF);
		obj->hash.v[ii+1] =
		(unsigned char)((obj->buf[i] >> 8) & 0xFF);
		obj->hash.v[ii+2] =
		(unsigned char)((obj->buf[i] >> 16) & 0xFF);
		obj->hash.v[ii+3] =
		(unsigned char)((obj->buf[i] >> 24) & 0xFF);
	}
}

STNBMd5HashHex NBMd5_getHashHex(const STNBMd5* obj){
	STNBMd5HashHex r;
	NBMd5_hashBinToHexChars(obj->hash.v, r.v);
	return r;
}

void NBMd5_reset(STNBMd5* obj){
    NBMd5_init(obj);
}

//

STNBMd5Hash NBMd5_getHashBytes(const void* data, UI32 dataSz){
	STNBMd5Hash r;
	STNBMd5 md5;
	NBMd5_init(&md5);
	NBMd5_feed(&md5, data, dataSz);
	NBMd5_finish(&md5);
	r = md5.hash;
	NBMd5_release(&md5);
	return r;
}

STNBMd5HashHex NBMd5_getHashBytesHex(const void* data, UI32 dataSz){
	STNBMd5HashHex r;
	STNBMd5 md5;
	NBMd5_init(&md5);
	NBMd5_feed(&md5, data, dataSz);
	NBMd5_finish(&md5);
	r = NBMd5_getHashHex(&md5);
	NBMd5_release(&md5);
	return r;
}

//

void NBMd5_hashBinToHexChars(const void* pHash16Bytes, char* dst33Chars){
	static const char* hexChars = "0123456789abcdef"; //"0123456789ABCDEF"
	const BYTE* hash16Bytes = (const BYTE*)pHash16Bytes;
	//
	dst33Chars[0] = hexChars[*hash16Bytes >> 4]; dst33Chars[1] = hexChars[*hash16Bytes & 15]; hash16Bytes++;
	dst33Chars[2] = hexChars[*hash16Bytes >> 4]; dst33Chars[3] = hexChars[*hash16Bytes & 15]; hash16Bytes++;
	dst33Chars[4] = hexChars[*hash16Bytes >> 4]; dst33Chars[5] = hexChars[*hash16Bytes & 15]; hash16Bytes++;
	dst33Chars[6] = hexChars[*hash16Bytes >> 4]; dst33Chars[7] = hexChars[*hash16Bytes & 15]; hash16Bytes++;
	//
	dst33Chars[8] = hexChars[*hash16Bytes >> 4]; dst33Chars[9] = hexChars[*hash16Bytes & 15]; hash16Bytes++;
	dst33Chars[10] = hexChars[*hash16Bytes >> 4]; dst33Chars[11] = hexChars[*hash16Bytes & 15]; hash16Bytes++;
	dst33Chars[12] = hexChars[*hash16Bytes >> 4]; dst33Chars[13] = hexChars[*hash16Bytes & 15]; hash16Bytes++;
	dst33Chars[14] = hexChars[*hash16Bytes >> 4]; dst33Chars[15] = hexChars[*hash16Bytes & 15]; hash16Bytes++;
	//
	dst33Chars[16] = hexChars[*hash16Bytes >> 4]; dst33Chars[17] = hexChars[*hash16Bytes & 15]; hash16Bytes++;
	dst33Chars[18] = hexChars[*hash16Bytes >> 4]; dst33Chars[19] = hexChars[*hash16Bytes & 15]; hash16Bytes++;
	dst33Chars[20] = hexChars[*hash16Bytes >> 4]; dst33Chars[21] = hexChars[*hash16Bytes & 15]; hash16Bytes++;
	dst33Chars[22] = hexChars[*hash16Bytes >> 4]; dst33Chars[23] = hexChars[*hash16Bytes & 15]; hash16Bytes++;
	//
	dst33Chars[24] = hexChars[*hash16Bytes >> 4]; dst33Chars[25] = hexChars[*hash16Bytes & 15]; hash16Bytes++;
	dst33Chars[26] = hexChars[*hash16Bytes >> 4]; dst33Chars[27] = hexChars[*hash16Bytes & 15]; hash16Bytes++;
	dst33Chars[28] = hexChars[*hash16Bytes >> 4]; dst33Chars[29] = hexChars[*hash16Bytes & 15]; hash16Bytes++;
	dst33Chars[30] = hexChars[*hash16Bytes >> 4]; dst33Chars[31] = hexChars[*hash16Bytes & 15]; hash16Bytes++;
	//
	dst33Chars[32] = '\0';
}

