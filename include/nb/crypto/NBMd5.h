#ifndef NB_MD5_H
#define NB_MD5_H

#include "nb/NBFrameworkDefs.h"

#ifdef __cplusplus
extern "C" {
#endif

	//Hash binary
	typedef struct STNBMd5Hash_ {
		UI8 v[16];
	} STNBMd5Hash;
	//Hash hexadecimal string
	typedef struct STNBMd5HashHex_ {
		char v[33];
	} STNBMd5HashHex;
	//
	typedef struct STNBMd5_ {
		UI32		i[2];          /* number of _bits_ handled mod 2^64 */
		UI32		buf[4];        /* scratch buffer */
		UI8			in[64];        /* input buffer */
		STNBMd5Hash	hash;
	} STNBMd5;
	//Factory
	void			NBMd5_init(STNBMd5* obj);
	void			NBMd5_release(STNBMd5* obj);
	//Feed
	void			NBMd5_feed(STNBMd5* obj, const void* inBuf, UI32 inLen);
	void			NBMd5_finish(STNBMd5* obj);
	STNBMd5HashHex	NBMd5_getHashHex(const STNBMd5* v);
	void			NBMd5_reset(STNBMd5* obj);
	//One step calculation
	STNBMd5Hash		NBMd5_getHashBytes(const void* data, UI32 dataSz);
	STNBMd5HashHex	NBMd5_getHashBytesHex(const void* data, UI32 dataSz);
	//Helpers
	void			NBMd5_hashBinToHexChars(const void* hash16Bytes, char* dst33Chars);
	/*
	 **********************************************************************
	 ** End of md5.h                                                     **
	 ******************************* (cut) ********************************
	 */
	
#ifdef __cplusplus
} //extern "C"
#endif

#endif

