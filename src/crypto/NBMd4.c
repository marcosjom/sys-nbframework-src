//
//  NBSha1.cpp
//  AUFramework
//
//  Created by Marcos Ortega Morales on 11/12/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
//Complementary PCH
#ifdef _WIN32
//
#else
#	include <string.h>		//for memset()
#endif
//
#include "nb/crypto/NBMd4.h"
#include "nb/core/NBMemory.h"

/*
* This is an OpenSSL-compatible implementation of the RSA Data Security, Inc.
* MD4 Message-Digest Algorithm (RFC 1320).
*
* Homepage:
* http://openwall.info/wiki/people/solar/software/public-domain-source-code/md4
*
* Author:
* Alexander Peslyak, better known as Solar Designer <solar at openwall.com>
*
* This software was written by Alexander Peslyak in 2001.  No copyright is
* claimed, and the software is hereby placed in the public domain.
* In case this attempt to disclaim copyright and place the software in the
* public domain is deemed null and void, then the software is
* Copyright (c) 2001 Alexander Peslyak and it is hereby released to the
* general public under the following terms:
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted.
*
* There's ABSOLUTELY NO WARRANTY, express or implied.
*
* (This is a heavily cut-down "BSD license".)
*
* This differs from Colin Plumb's older public domain implementation in that
* no exactly 32-bit integer data type is required (any 32-bit or wider
* unsigned integer data type will do), there's no compile-time endianness
* configuration, and the function prototypes match OpenSSL's.  No code from
* Colin Plumb's implementation has been reused; this comment merely compares
* the properties of the two independent implementations.
*
* The primary goals of this implementation are portability and ease of use.
* It is meant to be fast, but not as fast as possible.  Some known
* optimizations are not included to reduce source code size and avoid
* compile-time configuration.
*/

#include <string.h>
 
#include "nb/crypto/NBMd4.h"
 
/*
 * The basic MD4 functions.
 *
 * F and G are optimized compared to their RFC 1320 definitions, with the
 * optimization for F borrowed from Colin Plumb's Md4 implementation.
 */
#define F(x, y, z)			((z) ^ ((x) & ((y) ^ (z))))
#define G(x, y, z)			(((x) & ((y) | (z))) | ((y) & (z)))
#define H(x, y, z)			((x) ^ (y) ^ (z))
 
/*
 * The MD4 transformation for all three rounds.
 */
#define STEP(f, a, b, c, d, x, s) \
	(a) += f((b), (c), (d)) + (x); \
	(a) = (((a) << (s)) | (((a) & 0xffffffff) >> (32 - (s))));
 
/*
 * SET reads 4 input bytes in little-endian byte order and stores them in a
 * properly aligned word in host byte order.
 *
 * The check for little-endian architectures that tolerate unaligned memory
 * accesses is just an optimization.  Nothing will break if it fails to detect
 * a suitable architecture.
 *
 * Unfortunately, this optimization may be a C strict aliasing rules violation
 * if the caller's data buffer has effective type that cannot be aliased by
 * UI32.  In practice, this problem may occur if these MD4 routines are
 * inlined into a calling function, or with future and dangerously advanced
 * link-time optimizations.  For the time being, keeping these MD4 routines in
 * their own translation unit avoids the problem.
 */
#if defined(__i386__) || defined(__x86_64__) || defined(__vax__)
#define SET(n) \
	(*(UI32 *)&ptr[(n) * 4])
#define GET(n) \
	SET(n)
#else
#define SET(n) \
	(ctx->block[(n)] = \
	(UI32)ptr[(n) * 4] | \
	((UI32)ptr[(n) * 4 + 1] << 8) | \
	((UI32)ptr[(n) * 4 + 2] << 16) | \
	((UI32)ptr[(n) * 4 + 3] << 24))
#define GET(n) \
	(ctx->block[(n)])
#endif
 
/*
 * This processes one or more 64-byte data blocks, but does NOT update the bit
 * counters.  There are no alignment requirements.
 */
static const void *body(STNBMd4 *ctx, const void *data, unsigned long size)
{
	const unsigned char *ptr;
	UI32 a, b, c, d;
	UI32 saved_a, saved_b, saved_c, saved_d;
	const UI32 ac1 = 0x5a827999, ac2 = 0x6ed9eba1;
	
	ptr = (const unsigned char *)data;
 
	a = ctx->a;
	b = ctx->b;
	c = ctx->c;
	d = ctx->d;
 
	do {
		saved_a = a;
		saved_b = b;
		saved_c = c;
		saved_d = d;
 
/* Round 1 */
		STEP(F, a, b, c, d, SET(0), 3)
		STEP(F, d, a, b, c, SET(1), 7)
		STEP(F, c, d, a, b, SET(2), 11)
		STEP(F, b, c, d, a, SET(3), 19)
		STEP(F, a, b, c, d, SET(4), 3)
		STEP(F, d, a, b, c, SET(5), 7)
		STEP(F, c, d, a, b, SET(6), 11)
		STEP(F, b, c, d, a, SET(7), 19)
		STEP(F, a, b, c, d, SET(8), 3)
		STEP(F, d, a, b, c, SET(9), 7)
		STEP(F, c, d, a, b, SET(10), 11)
		STEP(F, b, c, d, a, SET(11), 19)
		STEP(F, a, b, c, d, SET(12), 3)
		STEP(F, d, a, b, c, SET(13), 7)
		STEP(F, c, d, a, b, SET(14), 11)
		STEP(F, b, c, d, a, SET(15), 19)
 
/* Round 2 */
		STEP(G, a, b, c, d, GET(0) + ac1, 3)
		STEP(G, d, a, b, c, GET(4) + ac1, 5)
		STEP(G, c, d, a, b, GET(8) + ac1, 9)
		STEP(G, b, c, d, a, GET(12) + ac1, 13)
		STEP(G, a, b, c, d, GET(1) + ac1, 3)
		STEP(G, d, a, b, c, GET(5) + ac1, 5)
		STEP(G, c, d, a, b, GET(9) + ac1, 9)
		STEP(G, b, c, d, a, GET(13) + ac1, 13)
		STEP(G, a, b, c, d, GET(2) + ac1, 3)
		STEP(G, d, a, b, c, GET(6) + ac1, 5)
		STEP(G, c, d, a, b, GET(10) + ac1, 9)
		STEP(G, b, c, d, a, GET(14) + ac1, 13)
		STEP(G, a, b, c, d, GET(3) + ac1, 3)
		STEP(G, d, a, b, c, GET(7) + ac1, 5)
		STEP(G, c, d, a, b, GET(11) + ac1, 9)
		STEP(G, b, c, d, a, GET(15) + ac1, 13)
 
/* Round 3 */
		STEP(H, a, b, c, d, GET(0) + ac2, 3)
		STEP(H, d, a, b, c, GET(8) + ac2, 9)
		STEP(H, c, d, a, b, GET(4) + ac2, 11)
		STEP(H, b, c, d, a, GET(12) + ac2, 15)
		STEP(H, a, b, c, d, GET(2) + ac2, 3)
		STEP(H, d, a, b, c, GET(10) + ac2, 9)
		STEP(H, c, d, a, b, GET(6) + ac2, 11)
		STEP(H, b, c, d, a, GET(14) + ac2, 15)
		STEP(H, a, b, c, d, GET(1) + ac2, 3)
		STEP(H, d, a, b, c, GET(9) + ac2, 9)
		STEP(H, c, d, a, b, GET(5) + ac2, 11)
		STEP(H, b, c, d, a, GET(13) + ac2, 15)
		STEP(H, a, b, c, d, GET(3) + ac2, 3)
		STEP(H, d, a, b, c, GET(11) + ac2, 9)
		STEP(H, c, d, a, b, GET(7) + ac2, 11)
		STEP(H, b, c, d, a, GET(15) + ac2, 15)
 
		a += saved_a;
		b += saved_b;
		c += saved_c;
		d += saved_d;
 
		ptr += 64;
	} while (size -= 64);
 
	ctx->a = a;
	ctx->b = b;
	ctx->c = c;
	ctx->d = d;
 
	return ptr;
}
 
void NBMd4_init(STNBMd4 *ctx){
	NBMemory_setZeroSt(*ctx, STNBMd4);
	//
	ctx->a = 0x67452301;
	ctx->b = 0xefcdab89;
	ctx->c = 0x98badcfe;
	ctx->d = 0x10325476;
	//
	ctx->lo = 0;
	ctx->hi = 0;
}

void NBMd4_release(STNBMd4* obj){
	//Nothing
}
 
void NBMd4_feed(STNBMd4 *ctx, const void *data, const UI32 dataSz){
	UI32 saved_lo, size = dataSz;
	unsigned long used, available;
 
	saved_lo = ctx->lo;
	if ((ctx->lo = (saved_lo + size) & 0x1fffffff) < saved_lo)
		ctx->hi++;
	ctx->hi += size >> 29;
 
	used = saved_lo & 0x3f;
 
	if (used) {
		available = 64 - used;
 
		if (size < available) {
			memcpy(&ctx->buffer[used], data, size);
			return;
		}
 
		memcpy(&ctx->buffer[used], data, available);
		data = (const unsigned char *)data + available;
		size -= available;
		body(ctx, ctx->buffer, 64);
	}
 
	if (size >= 64) {
		data = body(ctx, data, size & ~(unsigned long)0x3f);
		size &= 0x3f;
	}
 
	memcpy(ctx->buffer, data, size);
}
 
#define OUT(dst, src) \
	(dst)[0] = (unsigned char)(src); \
	(dst)[1] = (unsigned char)((src) >> 8); \
	(dst)[2] = (unsigned char)((src) >> 16); \
	(dst)[3] = (unsigned char)((src) >> 24);
 
void NBMd4_finish(STNBMd4 *ctx){
	unsigned long used, available;
	
	used = ctx->lo & 0x3f;
	
	ctx->buffer[used++] = 0x80;
	
	available = 64 - used;
	
	if (available < 8) {
		memset(&ctx->buffer[used], 0, available);
		body(ctx, ctx->buffer, 64);
		used = 0;
		available = 64;
	}
	
	memset(&ctx->buffer[used], 0, available - 8);
	
	ctx->lo <<= 3;
	OUT(&ctx->buffer[56], ctx->lo)
	OUT(&ctx->buffer[60], ctx->hi)
	
	body(ctx, ctx->buffer, 64);
	
	OUT(&ctx->hash.v[0], ctx->a)
	OUT(&ctx->hash.v[4], ctx->b)
	OUT(&ctx->hash.v[8], ctx->c)
	OUT(&ctx->hash.v[12], ctx->d)
	
}

STNBMd4HashHex NBMd4_getHashHex(const STNBMd4* obj){
	STNBMd4HashHex r;
	NBMd4_hashBinToHexChars(obj->hash.v, r.v);
	return r;
}

void NBMd4_reset(STNBMd4* obj){
    NBMd4_init(obj);
}


//

STNBMd4Hash NBMd4_getHashBytes(const void* data, UI32 dataSz){
	STNBMd4Hash r;
	STNBMd4 Md4;
	NBMd4_init(&Md4);
	NBMd4_feed(&Md4, data, dataSz);
	NBMd4_finish(&Md4);
	r = Md4.hash;
	NBMd4_release(&Md4);
	return r;
}

STNBMd4HashHex NBMd4_getHashBytesHex(const void* data, UI32 dataSz){
	STNBMd4HashHex r;
	STNBMd4 Md4;
	NBMd4_init(&Md4);
	NBMd4_feed(&Md4, data, dataSz);
	NBMd4_finish(&Md4);
	r = NBMd4_getHashHex(&Md4);
	NBMd4_release(&Md4);
	return r;
}

//

void NBMd4_hashBinToHexChars(const void* pHash16Bytes, char* dst33Chars){
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
