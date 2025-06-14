#ifndef NB_MD4_H
#define NB_MD4_H

#include "nb/NBFrameworkDefs.h"

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
* See md4.c for more information.
*/

#ifdef __cplusplus
extern "C" {
#endif

	//Hash binary
	typedef struct STNBMd4Hash_ {
		UI8 v[16];
	} STNBMd4Hash;

	//Hash hexadecimal string
	typedef struct STNBMd4HashHex_ {
		char v[33];
	} STNBMd4HashHex;

	//
	typedef struct STNBMd4_ {
		UI32 lo, hi;
		unsigned char buffer[64];
		UI32 block[16];
		UI32 a, b, c, d;
		STNBMd4Hash	hash;
	} STNBMd4;

	//
	void			NBMd4_init(STNBMd4* obj);
	void			NBMd4_release(STNBMd4* obj);

	//
	void			NBMd4_feed(STNBMd4* obj, const void* inBuf, const UI32 inLen);
	void			NBMd4_finish(STNBMd4* obj);
	STNBMd4HashHex	NBMd4_getHashHex(const STNBMd4* v);
	void			NBMd4_reset(STNBMd4* obj);

	//One step calculation
	STNBMd4Hash		NBMd4_getHashBytes(const void* data, UI32 dataSz);
	STNBMd4HashHex	NBMd4_getHashBytesHex(const void* data, UI32 dataSz);

	//Helpers
	void			NBMd4_hashBinToHexChars(const void* hash16Bytes, char* dst33Chars);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif

