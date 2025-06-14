//
//  NBSha1.h
//  AUFramework
//
//  Created by Marcos Ortega Morales on 11/12/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef NB_SHA1_H
#define NB_SHA1_H

#include "nb/NBFrameworkDefs.h"

#define SHA1_HASH_INIT(OBJ)					(OBJ).v[0] = 0x67452301; (OBJ).v[1] = 0xEFCDAB89; (OBJ).v[2] = 0x98BADCFE; (OBJ).v[3] = 0x10325476; (OBJ).v[4] = 0xC3D2E1F0
#define SHA1_HASH_INIT_P(OBJ)				(OBJ)->v[0] = 0x67452301; (OBJ)->v[1] = 0xEFCDAB89; (OBJ)->v[2] = 0x98BADCFE; (OBJ)->v[3] = 0x10325476; (OBJ)->v[4] = 0xC3D2E1F0

#define SHA1_HASH_IS_EMPTY(OBJ)				((OBJ).v[0] == 0x67452301 && (OBJ).v[1] == 0xEFCDAB89 && (OBJ).v[2] == 0x98BADCFE && (OBJ).v[3] == 0x10325476 && (OBJ).v[4] == 0xC3D2E1F0)
#define SHA1_HASH_IS_EMPTY_P(OBJ)			((OBJ)->v[0] == 0x67452301 && (OBJ)->v[1] == 0xEFCDAB89 && (OBJ)->v[2] == 0x98BADCFE && (OBJ)->v[3] == 0x10325476 && (OBJ)->v[4] == 0xC3D2E1F0)

#define SHA1_HASH_ARE_EQUALS(OBJ, OBJ2)		((OBJ).v[0] == (OBJ2).v[0] && (OBJ).v[1] == (OBJ2).v[1] && (OBJ).v[2] == (OBJ2).v[2] && (OBJ).v[3] == (OBJ2).v[3] && (OBJ).v[4] == (OBJ2).v[4])
#define SHA1_HASH_ARE_EQUALS_P(OBJ, OBJ2)	((OBJ)->v[0] == (OBJ2)->v[0] && (OBJ)->v[1] == (OBJ2)->v[1] && (OBJ)->v[2] == (OBJ2)->v[2] && (OBJ)->v[3] == (OBJ2)->v[3] && (OBJ)->v[4] == (OBJ2)->v[4])

#ifdef __cplusplus
extern "C" {
#endif
	//Hash binary
	typedef struct STNBSha1Hash_ {
		UI32 v[5];
	} STNBSha1Hash;
	//Hash hexadecimal string
	typedef struct STNBSha1HashHex_ {
		char v[41];
	} STNBSha1HashHex;
	//
	typedef struct STNBSha1_ {
		STNBSha1Hash	hash;
		//
		UI32 	Length_Low;               	// Message length in bits
		UI32	Length_High;               	// Message length in bits
		BYTE 	Message_Block[64];    		// 512-bit message blocks
		SI32 	Message_Block_Index;        // Index into message block array
		BOOL 	Computed;                   // Is the digest computed?
		BOOL 	Corrupted;                  // Is the message digest corruped?
	} STNBSha1;
	//Factory
	void			NBSha1_init(STNBSha1* v);
	void			NBSha1_release(STNBSha1* v);
	//Feed
	void			NBSha1_feedStart(STNBSha1* v);
	void			NBSha1_feed(STNBSha1* v, const void* data, UI32 dataSz);
	SI32			NBSha1_feedEnd(STNBSha1* v);
	STNBSha1HashHex	NBSha1_getHashHex(const STNBSha1* v);
	//One step calculation
	STNBSha1Hash	NBSha1_getHashBytes(const void* data, const UI32 dataSz);
	STNBSha1HashHex	NBSha1_getHashHexBytes(const void* data, const UI32 dataSz);
	STNBSha1HashHex NBSha1_getHashHexStr(const char* str);
	//Helpers
	void			NBSha1_hashBinToHexChars(const BYTE* hash20Bytes, char* dst41Bytes);
	void			NBSha1_ProcessMessageBlock(STNBSha1* v);
	void			NBSha1_PadMessage(STNBSha1* v);
	UI32			NBSha1_CircularShift(SI32 bits, UI32 word);
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif
