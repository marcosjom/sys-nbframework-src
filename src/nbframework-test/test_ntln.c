//
//  test_ntln.c
//  nbframework-test-osx
//
//  Created by Marcos Ortega on 19/12/21.
//

#include "test_ntln.h"

//Test HMAC
/*{
	const char* key = "key";
	const char* data = "The quick brown fox jumps over the lazy dog";
	const STNBMd5HashHex hash = NBHmacMd5_getHashHexBytes(key, NBString_strLenBytes(key), data, NBString_strLenBytes(data));
	PRINTF_INFO("HMAX_MD5: key('%s') data('%s'): '%s'.\n", key, data, hash.v);
}*/

//
//Test STNTLM_NEGOTIATE_MESSAGE
/*{
	//const char* b64Sample = "yNY3lb6a0L6vVQEZNqwQn0s8UNew33KdKZvG+Onv";
	//const char* b64Sample = "tESsBmE/yNY3lb6a0L6vVQEZNqwQn0s8Unew";
	//const char* b64Sample = "TlRMTVNTUAABAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==";
	const char* b64Sample = "TlRMTVNTUAABAAAABYIIAAAAAAAAAAAAAAAAAAAAAAAAAAAAMAAAAAAAAAAwAAAA"; //from github
	STNBString bin;
	NBString_init(&bin);
	if(!NBBase64_decode(&bin, b64Sample)){
		PRINTF_ERROR("NBBase64_decode failed.\n");
	} else {
		const STNTLM_NEGOTIATE_MESSAGE* v = (const STNTLM_NEGOTIATE_MESSAGE*)bin.str;
		PRINTF_INFO("NBBase64_decode success.\n");
	}
	NBString_release(&bin);
}*/

//Test
/*{
	const char* b64Sample = "TlRMTVNTUAACAAAADgAOADgAAAAFAoECwKNboeSzt2cAAAAAAAAAAJgAmABGAAAABgOAJQAAAA9PAE0AQwBPAEwATwAwAAIADgBPAE0AQwBPAEwATwAwAAEAFABFAFgAQwBIAEEATgBHAEUAMgAyAAQAFABvAG0AYwBvAGwAbwAuAG4AZQB0AAMAKgBlAHgAYwBoAGEAbgBnAGUAMgAyAC4AbwBtAGMAbwBsAG8ALgBuAGUAdAAFABQAbwBtAGMAbwBsAG8ALgBuAGUAdAAHAAgAekubKwCE1QEAAAAA";
	STNBString bin;
	NBString_init(&bin);
	if(!NBBase64_decode(&bin, b64Sample)){
		PRINTF_ERROR("NBBase64_decode failed.\n");
	} else {
		const STNTLM_CHALLENGE_MESSAGE* v = (const STNTLM_CHALLENGE_MESSAGE*)bin.str;
		char bytes[4096]; NBMemory_copy(&bytes[0], bin.str, bin.length);
		if(bin.length > sizeof(STNTLM_CHALLENGE_MESSAGE)){
			STNBString tgName, tgInfo;
			NBString_init(&tgName);
			NBString_init(&tgInfo);
			if(v->targetNameLen > 0){
				NBString_concatUtf16Chars(&tgName, (const UI16*)&bin.str[v->targetNameBufferOffset], v->targetNameLen / 2);
				PRINTF_INFO("tgName: '%s'.\n", tgName.str);
			}
			if(v->targetInfoLen > 0){
				NBString_concatUtf16Chars(&tgInfo, (const UI16*)&bin.str[v->targetInfoBufferOffset], v->targetInfoLen / 2);
				PRINTF_INFO("tgInfo: '%s'.\n", tgInfo.str);
			}
			NBString_release(&tgInfo);
			NBString_release(&tgName);
		}
		PRINTF_INFO("NBBase64_decode success.\n");
	}
	NBString_release(&bin);
}*/

//Testing 'NTOWFv2'
//{
//	NBNtln_concatNTOWFv2(NULL, "Password", "User", "Domain");
//}
//Testing RC4
/*{
	const char* keyDataPairs[] = {
		"Key", "Plaintext",
		"Wiki", "pedia",
		"Secret", "Attack at dawn"
	};
	STNBString tmp;
	NBString_init(&tmp);
	{
		SI32 i; const SI32 count = (sizeof(keyDataPairs) / sizeof(keyDataPairs[0]));
		for(i = 0; i < count; i += 2){
			const char* key = keyDataPairs[i];
			const char* data = keyDataPairs[i + 1];
			NBString_empty(&tmp);
			NBRc4A_concatHex(key, NBString_strLenBytes(key), data, NBString_strLenBytes(data), &tmp);
			PRINTF_INFO("NBRc4A('%s', '%s'): '%s'.\n", key, data, tmp.str);
		}
	}
	NBString_release(&tmp);
}*/
//Test challenge response
/*{
	const BYTE negBin[] = {
		0x4e, 0x54, 0x4c, 0x4d, 0x53, 0x53, 0x50, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x82, 0x80, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	const BYTE chBin[] = {
		0x4e, 0x54, 0x4c, 0x4d, 0x53, 0x53, 0x50, 0x00, 0x02, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x0c, 0x00,
		0x38, 0x00, 0x00, 0x00, 0x33, 0x82, 0x8a, 0xe2, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x24, 0x00, 0x44, 0x00, 0x00, 0x00,
		0x06, 0x00, 0x70, 0x17, 0x00, 0x00, 0x00, 0x0f, 0x53, 0x00, 0x65, 0x00, 0x72, 0x00, 0x76, 0x00,
		0x65, 0x00, 0x72, 0x00, 0x02, 0x00, 0x0c, 0x00, 0x44, 0x00, 0x6f, 0x00, 0x6d, 0x00, 0x61, 0x00,
		0x69, 0x00, 0x6e, 0x00, 0x01, 0x00, 0x0c, 0x00, 0x53, 0x00, 0x65, 0x00, 0x72, 0x00, 0x76, 0x00,
		0x65, 0x00, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	const BYTE authBin[] = {
		0x4e, 0x54, 0x4c, 0x4d, 0x53, 0x53, 0x50, 0x00, 0x03, 0x00, 0x00, 0x00, 0x18, 0x00, 0x18, 0x00,
		0x6c, 0x00, 0x00, 0x00, 0x54, 0x00, 0x54, 0x00, 0x84, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x0c, 0x00,
		0x48, 0x00, 0x00, 0x00, 0x08, 0x00, 0x08, 0x00, 0x54, 0x00, 0x00, 0x00, 0x10, 0x00, 0x10, 0x00,
		0x5c, 0x00, 0x00, 0x00, 0x10, 0x00, 0x10, 0x00, 0xd8, 0x00, 0x00, 0x00, 0x35, 0x82, 0x88, 0xe2,
		0x05, 0x01, 0x28, 0x0a, 0x00, 0x00, 0x00, 0x0f, 0x44, 0x00, 0x6f, 0x00, 0x6d, 0x00, 0x61, 0x00,
		0x69, 0x00, 0x6e, 0x00, 0x55, 0x00, 0x73, 0x00, 0x65, 0x00, 0x72, 0x00, 0x43, 0x00, 0x4f, 0x00,
		0x4d, 0x00, 0x50, 0x00, 0x55, 0x00, 0x54, 0x00, 0x45, 0x00, 0x52, 0x00, 0x86, 0xc3, 0x50, 0x97,
		0xac, 0x9c, 0xec, 0x10, 0x25, 0x54, 0x76, 0x4a, 0x57, 0xcc, 0xcc, 0x19, 0xaa, 0xaa, 0xaa, 0xaa,
		0xaa, 0xaa, 0xaa, 0xaa, 0x68, 0xcd, 0x0a, 0xb8, 0x51, 0xe5, 0x1c, 0x96, 0xaa, 0xbc, 0x92, 0x7b,
		0xeb, 0xef, 0x6a, 0x1c, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x00, 0x00, 0x00, 0x00,
		0x02, 0x00, 0x0c, 0x00, 0x44, 0x00, 0x6f, 0x00, 0x6d, 0x00, 0x61, 0x00, 0x69, 0x00, 0x6e, 0x00,
		0x01, 0x00, 0x0c, 0x00, 0x53, 0x00, 0x65, 0x00, 0x72, 0x00, 0x76, 0x00, 0x65, 0x00, 0x72, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc5, 0xda, 0xd2, 0x54, 0x4f, 0xc9, 0x79, 0x90,
		0x94, 0xce, 0x1c, 0xe9, 0x0b, 0xc9, 0xd0, 0x3e
	};
	const STNTLM_NEGOTIATE_MESSAGE* negSample = (const STNTLM_NEGOTIATE_MESSAGE*)negBin;
	const STNTLM_CHALLENGE_MESSAGE* chSample  = (const STNTLM_CHALLENGE_MESSAGE*)chBin;
	const STNTLM_AUTHENTICATE_MESSAGE* authSample  = (const STNTLM_AUTHENTICATE_MESSAGE*)authBin;
	{
		PRINTF_INFO("Negotiate flags: ....\n");
		NBNtln_printFlags(negSample->negotiateFlags);
		PRINTF_INFO("Challenge flags: ....\n");
		NBNtln_printFlags(chSample->negotiateFlags);
		PRINTF_INFO("Authenticate flags: ....\n");
		NBNtln_printFlags(authSample->negotiateFlags);
		PRINTF_INFO("Template auth: ....\n");
		{
			STNBString ntHex, lmHex, domainHex, userHex, workstationHex, encKeyHex;
			NBString_init(&ntHex);
			NBString_init(&lmHex);
			NBString_init(&domainHex);
			NBString_init(&userHex);
			NBString_init(&workstationHex);
			NBString_init(&encKeyHex);
			const char* nt = (const char*)&authBin[authSample->ntChallengeResponseBufferOffset];
			const char* lm = (const char*)&authBin[authSample->lmChallengeResponseBufferOffset];
			const char* domain = (const char*)&authBin[authSample->domainNameBufferOffset];
			const char* user = (const char*)&authBin[authSample->userNameBufferOffset];
			const char* workstation = (const char*)&authBin[authSample->workstationNameBufferOffset];
			const char* encKey = (const char*)&authBin[authSample->encRandSessionKeyBufferOffset];
			NBString_concatBytesHex(&ntHex, nt, authSample->ntChallengeResponseLen);
			NBString_concatBytesHex(&lmHex, lm, authSample->lmChallengeResponseLen);
			NBString_concatBytesHex(&domainHex, domain, authSample->domainNameLen);
			NBString_concatBytesHex(&userHex, user, authSample->userNameLen);
			NBString_concatBytesHex(&workstationHex, workstation, authSample->workstationNameLen);
			NBString_concatBytesHex(&encKeyHex, encKey, authSample->encRandSessionKeyLen);
			PRINTF_INFO("ntHex: '%s'.\n", ntHex.str);
			PRINTF_INFO("lmHex: '%s'.\n", lmHex.str);
			PRINTF_INFO("domainHex: '%s'.\n", domainHex.str);
			PRINTF_INFO("userHex: '%s'.\n", userHex.str);
			PRINTF_INFO("workstationHex: '%s'.\n", workstationHex.str);
			PRINTF_INFO("encKeyHex: '%s'.\n", encKeyHex.str);
			NBString_release(&ntHex);
			NBString_release(&lmHex);
			NBString_release(&domainHex);
			NBString_release(&userHex);
			NBString_release(&workstationHex);
			NBString_release(&encKeyHex);
		}
	}
	{
		const char* user	= "User";
		const char* pass	= "Password";
		const char* domain	= "Domain";
		const char* workstation	= "COMPUTER";
		const UI64 clientChallenge = 0Xaaaaaaaaaaaaaaaa;
		STNBString authBin;
		NBString_init(&authBin);
		if(!NBNtln_concatAuthMessageBin(negBin, sizeof(negBin), chBin, sizeof(chBin), user, pass, domain, workstation, clientChallenge, &authBin)){
			PRINTF_ERROR("NBNtln_concatAuthMessageBin failed.\n");
		} else {
			PRINTF_INFO("NBNtln_concatAuthMessageBin success.\n");
			const UI32 sz = (UI32)sizeof(STNTLM_AUTHENTICATE_MESSAGE);
			const STNTLM_AUTHENTICATE_MESSAGE* authResult  = (const STNTLM_AUTHENTICATE_MESSAGE*)authBin.str;
			PRINTF_INFO("Authenticate flags: ....\n");
			NBNtln_printFlags(authResult->negotiateFlags);
			{
				STNBString ntHex, lmHex, domainHex, userHex, workstationHex, encKeyHex;
				NBString_init(&ntHex);
				NBString_init(&lmHex);
				NBString_init(&domainHex);
				NBString_init(&userHex);
				NBString_init(&workstationHex);
				NBString_init(&encKeyHex);
				const char* nt = (const char*)&authBin.str[authResult->ntChallengeResponseBufferOffset];
				const char* lm = (const char*)&authBin.str[authResult->lmChallengeResponseBufferOffset];
				const char* domain = (const char*)&authBin.str[authResult->domainNameBufferOffset];
				const char* user = (const char*)&authBin.str[authResult->userNameBufferOffset];
				const char* workstation = (const char*)&authBin.str[authResult->workstationNameBufferOffset];
				const char* encKey = (const char*)&authBin.str[authResult->encRandSessionKeyBufferOffset];
				NBString_concatBytesHex(&ntHex, nt, authResult->ntChallengeResponseLen);
				NBString_concatBytesHex(&lmHex, lm, authResult->lmChallengeResponseLen);
				NBString_concatBytesHex(&domainHex, domain, authResult->domainNameLen);
				NBString_concatBytesHex(&userHex, user, authResult->userNameLen);
				NBString_concatBytesHex(&workstationHex, workstation, authResult->workstationNameLen);
				NBString_concatBytesHex(&encKeyHex, encKey, authResult->encRandSessionKeyLen);
				PRINTF_INFO("ntHex: '%s'.\n", ntHex.str);
				PRINTF_INFO("lmHex: '%s'.\n", lmHex.str);
				PRINTF_INFO("domainHex: '%s'.\n", domainHex.str);
				PRINTF_INFO("userHex: '%s'.\n", userHex.str);
				PRINTF_INFO("workstationHex: '%s'.\n", workstationHex.str);
				PRINTF_INFO("encKeyHex: '%s'.\n", encKeyHex.str);
				NBString_release(&ntHex);
				NBString_release(&lmHex);
				NBString_release(&domainHex);
				NBString_release(&userHex);
				NBString_release(&workstationHex);
				NBString_release(&encKeyHex);
			}
			PRINTF_INFO("Template auth: ....\n");
		}
		NBString_release(&authBin);
	}
}*/
