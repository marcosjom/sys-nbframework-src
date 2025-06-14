#ifndef NB_RC4_ALLEGED_H
#define NB_RC4_ALLEGED_H

#include "nb/NBFrameworkDefs.h"
#include "nb/core/NBString.h"

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

	//

	typedef struct STNBRc4A_ {
		BYTE	S[256];
		UI32	i, j;
	} STNBRc4A;

	//
	void			NBRc4A_init(STNBRc4A* obj);
	void			NBRc4A_release(STNBRc4A* obj);

	//
	void			NBRc4A_feedStart(STNBRc4A* obj, const void* key, const UI32 keySz);
	void			NBRc4A_feed(STNBRc4A* obj, const void* data, const UI32 dataSz, STNBString* dst);

	//One step calculation
	void			NBRc4A_concat(const void* key, const UI32 keySz, const void* data, const UI32 dataSz, STNBString* dst);
	void			NBRc4A_concatHex(const void* key, const UI32 keySz, const void* data, const UI32 dataSz, STNBString* dst);
	
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif

