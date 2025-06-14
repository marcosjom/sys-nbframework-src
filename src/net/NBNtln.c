
#include "nb/NBFrameworkPch.h"
//Complementary PCH
#include <stdlib.h>		//for rand()
//
#include "nb/net/NBNtln.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBEncoding.h"
#include "nb/crypto/NBMd4.h"
#include "nb/crypto/NBHmac.h"
#include "nb/crypto/NBRc4A.h"

static const char* ___NTLM_NegFlagsNames[] = {
	"NTLMSSP_NEGOTIATE_UNICODE",
	"NTLM_NEGOTIATE_OEM",
	"NTLMSSP_REQUEST_TARGET",
	"NTLN_R10",
	"NTLMSSP_NEGOTIATE_SIGN",
	"NTLMSSP_NEGOTIATE_SEAL",
	"NTLMSSP_NEGOTIATE_DATAGRAM",
	"NTLMSSP_NEGOTIATE_LM_KEY",
	"NTLN_R9",
	"NTLMSSP_NEGOTIATE_NTLM",
	"NTLN_R8",
	"NTLN_CONN_ANONYMOUS",
	"NTLMSSP_NEGOTIATE_OEM_DOMAIN_SUPPLIED",
	"NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED",
	"NTLN_R7",
	"NTLMSSP_NEGOTIATE_ALWAYS_SIGN",
	"NTLMSSP_TARGET_TYPE_DOMAIN",
	"NTLMSSP_TARGET_TYPE_SERVER",
	"NTLN_R6",
	"NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY",
	"NTLMSSP_NEGOTIATE_IDENTIFY",
	"NTLN_R5",
	"NTLMSSP_REQUEST_NON_NT_SESSION_KEY",
	"NTLMSSP_NEGOTIATE_TARGET_INFO",
	"NTLN_R4",
	"NTLMSSP_NEGOTIATE_VERSION",
	"NTLN_R3",
	"NTLN_R2",
	"NTLN_R1",
	"NTLMSSP_NEGOTIATE_128",
	"NTLMSSP_NEGOTIATE_KEY_EXCH",
	"NTLMSSP_NEGOTIATE_56",
};

/*
Define NTOWFv2(Passwd, User, UserDom) as
	HMAC_MD5( MD4(UNICODE(Passwd)), UNICODE(ConcatenationOf( Uppercase(User), UserDom ) ) )
EndDefine
*/
void NBNtln_concatNTOWFv2(STNBString* dst, const char* passUtf8, const char* userUtf8, const char* domainUtf8){
	STNBString pass16, usrDom16;
	NBString_init(&pass16);
	NBString_init(&usrDom16);
	//Build pass16
	{
		const char* v = passUtf8;
		while(*v != '\0'){
			NBString_concatByte(&pass16, *v);
			NBString_concatByte(&pass16, '\0');
			v++;
		}
	}
	//Build usrDom16
	{
		const char* v;
		v = userUtf8;
		while(*v != '\0'){
			NBString_concatUpperBytes(&usrDom16, v, 1);
			NBString_concatByte(&usrDom16, '\0');
			v++;
		}
		v = domainUtf8;
		while(*v != '\0'){
			NBString_concatByte(&usrDom16, *v);
			NBString_concatByte(&usrDom16, '\0');
			v++;
		}
	}
	//Calculate
	{
		STNBMd4Hash key = NBMd4_getHashBytes(pass16.str, pass16.length);
		STNBMd5Hash hash = NBHmacMd5_getHashBytes(key.v, sizeof(key.v), usrDom16.str, usrDom16.length);
		if(dst != NULL){
			NBString_concatBytes(dst, (const char*)hash.v, sizeof(hash.v));
		}
		/*{
			STNBMd5HashHex hash = NBHmacMd5_getHashHexBytes(key.v, sizeof(key.v), usrDom16.str, usrDom16.length);
			PRINTF_INFO("NBNtln_concatNTOWFv2('%s', '%s', '%s') = '%s'.\n", passUtf8, userUtf8, domainUtf8, hash.v);
		}*/
	}
	NBString_release(&pass16);
	NBString_release(&usrDom16);
}

/*
Define LMOWFv2(Passwd, User, UserDom) as
	NTOWFv2(Passwd, User, UserDom)
EndDefine
*/
void NBNtln_concatLMOWFv2(STNBString* dst, const char* passUtf8, const char* userUtf8, const char* domainUtf8){
	NBNtln_concatNTOWFv2(dst, passUtf8, userUtf8, domainUtf8);
}

/*
Define SIGNKEY(NegFlg, ExportedSessionKey, Mode) as
	If (NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY flag is set in NegFlg)
		If (Mode equals "Client")
			Set SignKey to MD5(ConcatenationOf(ExportedSessionKey, "session key to client-to-server signing key magic constant"))
		Else
			Set SignKey to MD5(ConcatenationOf(ExportedSessionKey, "session key to server-to-client signing key magic constant"))
		Endif
	Else
		Set  SignKey to NIL
	Endif
EndDefine
*/
BOOL NBNtln_concatSIGNKEY(STNBString* dst, const UI32 negFlg, const void* exportedSessionKey, const UI32 exportedSessionKeySz, const char* mode){
	BOOL r = FALSE;
	if((negFlg & NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY) != 0){
		if(NBString_strIsEqual(mode, "Client")){
			STNBString tmp;
			NBString_init(&tmp);
			NBString_concatBytes(&tmp, exportedSessionKey, exportedSessionKeySz);
			NBString_concat(&tmp, "session key to client-to-server signing key magic constant");
			{
				const STNBMd5Hash h = NBMd5_getHashBytes(tmp.str, tmp.length);
				if(dst != NULL){
					NBString_concatBytes(dst, (const char*)h.v, sizeof(h.v));
				}
				r = TRUE;
			}
			NBString_release(&tmp);
		} else if(NBString_strIsEqual(mode, "Server")){
			STNBString tmp;
			NBString_init(&tmp);
			NBString_concatBytes(&tmp, exportedSessionKey, exportedSessionKeySz);
			NBString_concat(&tmp, "session key to server-to-client signing key magic constant");
			{
				const STNBMd5Hash h = NBMd5_getHashBytes(tmp.str, tmp.length);
				if(dst != NULL){
					NBString_concatBytes(dst, (const char*)h.v, sizeof(h.v));
				}
				r = TRUE;
			}
			NBString_release(&tmp);
		} else {
			PRINTF_ERROR("NBNtln_concatSIGNKEY, expected mode 'Client' or 'Server'.\n");
		}
	} else {
		r = TRUE;
	}
	return r;
}

BOOL NBNtln_concatAuthMessageBin(const void* cltNegBin, const UI32 cltNegBinSz, const void* pSrvChallengeBin, const UI32 srvChallengeBinSz, const char* user, const char* pass, const char* domain, const char* workstation, const UI64 clientChallenge, STNBString* dstAuthBin){
	BOOL r = FALSE;
	if(srvChallengeBinSz < sizeof(STNTLM_CHALLENGE_MESSAGE)){
		PRINTF_ERROR("NTLM challenge corrupted, signature/header does not match.\n");
	} else {
		const BYTE* srvChallengeBin = (const BYTE*)pSrvChallengeBin;
		const STNTLM_CHALLENGE_MESSAGE* chg = (const STNTLM_CHALLENGE_MESSAGE*)srvChallengeBin;
		if(!NBString_strIsEqual((const char*)&chg->signature[0], "NTLMSSP")){
			PRINTF_ERROR("NTLM challenge corrupted, signature/header does not match.\n");
		} else if(chg->messageType != 0x00000002){
			PRINTF_ERROR("NTLM challenge corrupted, messageType does not match.\n");
		} else {
			BOOL timeIsSet = FALSE;
			STNBString compName, compName16, domainName, domainName16;
			STNBString dnsCompName, dnsCompName16, dnsDomainName, dnsDomainName16;
			STNTLM_FILETIME time;
			NBMemory_setZeroSt(time, STNTLM_FILETIME);
			NBString_init(&compName);
			NBString_init(&compName16);
			NBString_init(&domainName);
			NBString_init(&domainName16);
			NBString_init(&dnsCompName);
			NBString_init(&dnsCompName16);
			NBString_init(&dnsDomainName);
			NBString_init(&dnsDomainName16);
			//Analyze challenge
			{
				STNBString tgName;
				NBString_init(&tgName);
				if(chg->targetNameLen > 0){
					NBString_concatUtf16Chars(&tgName, (const UI16*)&srvChallengeBin[chg->targetNameBufferOffset], chg->targetNameLen / 2);
					//PRINTF_INFO("tgName: '%s'.\n", tgName.str);
				}
				//NegFlags
				/*{
					NBNtln_printFlags(chg->negotiateFlags);
				}*/
				if(chg->targetInfoLen > 0){
					BOOL doCicle = TRUE;
					const BYTE* v = (const BYTE*)&srvChallengeBin[chg->targetInfoBufferOffset];
					const BYTE* vLastAfter = v + chg->targetInfoLen;
					STNBString val;
					NBString_init(&val);
					while(v < vLastAfter && doCicle){
						const STNTLM_AVPAIR* head = (const STNTLM_AVPAIR*)v;
						switch(head->avId) {
							case 0x0000: //MsvAvEOL
								doCicle = FALSE;
								//PRINTF_INFO("tagInfo: MsvAvEOL.\n");
								break;
							case 0x0001: //MsvAvNbComputerName
								NBString_empty(&val);
								NBString_concatUtf16Chars(&val, (const UI16*)(head + 1), (head->avLen / 2));
								NBString_concatBytes(&compName16, (const char*)(head + 1), head->avLen);
								NBString_concatBytes(&compName, val.str, val.length);
								//PRINTF_INFO("tagInfo: MsvAvNbComputerName('%s').\n", val.str);
								break;
							case 0x0002: //MsvAvNbDomainName
								NBString_empty(&val);
								NBString_concatUtf16Chars(&val, (const UI16*)(head + 1), (head->avLen / 2));
								NBString_concatBytes(&domainName16, (const char*)(head + 1), head->avLen);
								NBString_concatBytes(&domainName, val.str, val.length);
								//PRINTF_INFO("tagInfo: MsvAvNbDomainName('%s').\n", val.str);
								break;
							case 0x0003: //MsvAvDnsComputerName
								NBString_empty(&val);
								NBString_concatUtf16Chars(&val, (const UI16*)(head + 1), (head->avLen / 2));
								NBString_concatBytes(&dnsCompName16, (const char*)(head + 1), head->avLen);
								NBString_concatBytes(&dnsCompName, val.str, val.length);
								//PRINTF_INFO("tagInfo: MsvAvDnsComputerName('%s').\n", val.str);
								break;
							case 0x0004: //MsvAvDnsDomainName
								NBString_empty(&val);
								NBString_concatUtf16Chars(&val, (const UI16*)(head + 1), (head->avLen / 2));
								NBString_concatBytes(&dnsDomainName16, (const char*)(head + 1), head->avLen);
								NBString_concatBytes(&dnsDomainName, val.str, val.length);
								//PRINTF_INFO("tagInfo: MsvAvDnsDomainName('%s').\n", val.str);
								break;
							case 0x0005: //MsvAvDnsTreeName
								NBString_empty(&val);
								NBString_concatUtf16Chars(&val, (const UI16*)(head + 1), (head->avLen / 2));
								//PRINTF_INFO("tagInfo: MsvAvDnsTreeName('%s').\n", val.str);
								break;
							case 0x0006: //MsvAvFlags
								if(head->avLen < 4){
									PRINTF_ERROR("tagInfo: MsvAvFlags expected 4 bytes (%d provided).\n", head->avLen);
								} else {
									//const UI32 flags = *((const UI32*)(head + 1));
									//if((flags & 0x00000001) != 0){ PRINTF_INFO("tagInfo: MsvAvFlags: 'account authentication is constrained'.\n"); }
									//if((flags & 0x00000002) != 0){ PRINTF_INFO("tagInfo: MsvAvFlags: 'client is providing message integrity in the MIC field'.\n"); }
									//if((flags & 0x00000004) != 0){ PRINTF_INFO("tagInfo: MsvAvFlags: 'client is providing a target SPN generated from an untrusted source'.\n"); }
								}
								break;
							case 0x0007: //MsvAvTimestamp
								if(head->avLen < sizeof(STNTLM_FILETIME)){
									PRINTF_ERROR("tagInfo: MsvAvTimestamp expected %d bytes (%d provided).\n", (SI32)sizeof(STNTLM_FILETIME), head->avLen);
								} else {
									const STNTLM_FILETIME ftime = *((const STNTLM_FILETIME*)(head + 1));
									time = ftime; timeIsSet = TRUE;
									//PRINTF_INFO("tagInfo: MsvAvTimestamp: %llu.\n", *((const UI64*)(head + 1)));
								}
								break;
							case 0x0008: //MsvAvSingleHost
								if(head->avLen < sizeof(STNTLM_Single_Host_Data)){
									PRINTF_ERROR("tagInfo: MsvAvSingleHost expected %d bytes (%d provided).\n", (SI32)sizeof(STNTLM_Single_Host_Data), head->avLen);
								} else {
									//const STNTLM_Single_Host_Data* sHost = (const STNTLM_Single_Host_Data*)(head + 1);
									//PRINTF_INFO("tagInfo: MsvAvSingleHost: size(%d) z4(%d).\n", sHost->size, sHost->z4);
								}
								break;
							case 0x0009: //MsvAvTargetName
								NBString_empty(&val);
								NBString_concatUtf16Chars(&val, (const UI16*)(head + 1), (head->avLen / 2));
								//PRINTF_INFO("tagInfo: MsvAvTargetName('%s').\n", val.str);
								break;
							case 0x000A: //MsvAvChannelBindings
								//PRINTF_INFO("tagInfo: MsvAvChannelBindings %d bytes gss_channel_bindings_struct.\n", head->avLen);
								break;
							default:
								//PRINTF_WARNING("tagInfo: STNTLM_AVPAIR with unexpected avId found (%d bytes).\n", head->avLen);
								break;
						}
						v += sizeof(STNTLM_AVPAIR) + head->avLen;
					}
					NBString_release(&val);
				}
				NBString_release(&tgName);
			}
			/*if(!timeIsSet){
				//No timestamp present means to send a 'LmChallengeResponse',
				//wich are not supported since WindowsNT/2000
				PRINTF_ERROR("MsvAvTimestamp was expected in serveChallenge.\n");
			} else*/ if(compName.length <= 0){
				PRINTF_ERROR("MsvAvNbComputerName was expected in serveChallenge.\n");
			} else if(domainName.length <= 0){
				PRINTF_ERROR("MsvAvNbDomainName was expected in serveChallenge.\n");
			} else {
				const BYTE responserVersion		= 1; //Defined by spec
				const BYTE hiResponserVersion	= 1; //Defined by spec
				STNBString responseKeyNT, responseKeyLM, nTProofStr, ntChallengeResponse, lmChallengeResponse;
				STNBString sessionBaseKey, keyExchangeKey, exportedSessionKey, encryptedRandomSessionKey;
				NBString_init(&responseKeyNT);
				NBString_init(&responseKeyLM);
				NBString_init(&nTProofStr);
				NBString_init(&ntChallengeResponse);
				NBString_init(&lmChallengeResponse);
				NBString_init(&sessionBaseKey);
				NBString_init(&keyExchangeKey);
				NBString_init(&exportedSessionKey);
				NBString_init(&encryptedRandomSessionKey);
				NBNtln_concatNTOWFv2(&responseKeyNT, pass, user, domain);
				NBNtln_concatLMOWFv2(&responseKeyLM, pass, user, domain);
				{
					STNBString tmp2;
					NBString_init(&tmp2);
					if(NBString_strIsEmpty(user) && NBString_strIsEmpty(pass)){
						//-- Special case for anonymous authentication
						NBString_empty(&ntChallengeResponse);
						NBString_empty(&lmChallengeResponse);
						NBString_concatRepeatedByte(&lmChallengeResponse, '\0', 1); //Z(1)
					} else {
						STNBString tmp;
						NBString_init(&tmp);
						NBString_concatByte(&tmp, responserVersion);
						NBString_concatByte(&tmp, hiResponserVersion);
						NBString_concatRepeatedByte(&tmp, '\0', 6); //Z(6)
						NBString_concatBytes(&tmp, (const char*)&time, sizeof(time));
						NBString_concatBytes(&tmp, (const char*)&clientChallenge, sizeof(clientChallenge));
						NBString_concatRepeatedByte(&tmp, '\0', 4); //Z(4)
						{
							NBString_concatBytes(&tmp, (const char*)&srvChallengeBin[chg->targetInfoBufferOffset], chg->targetInfoLen);
							//NBString_concatBytes(&tmp, compName16.str, compName16.length);
						}
						NBString_concatRepeatedByte(&tmp, '\0', 4); //Z(4)
						//nTProofStr
						{
							NBString_empty(&tmp2);
							NBString_concatBytes(&tmp2, (const char*)&chg->serverChallenge, sizeof(chg->serverChallenge));
							NBString_concatBytes(&tmp2, tmp.str, tmp.length);
							{
								const STNBMd5Hash hash = NBHmacMd5_getHashBytes(responseKeyNT.str, responseKeyNT.length, tmp2.str, tmp2.length);
								NBString_setBytes(&nTProofStr, (const char*)hash.v, sizeof(hash.v));
							}
						}
						//NtChallengeResponse
						{
							NBString_empty(&ntChallengeResponse);
							NBString_concatBytes(&ntChallengeResponse, nTProofStr.str, nTProofStr.length);
							NBString_concatBytes(&ntChallengeResponse, tmp.str, tmp.length);
						}
						//LmChallengeResponse
						{
							NBString_empty(&tmp2);
							NBString_concatBytes(&tmp2, (const char*)&chg->serverChallenge, sizeof(chg->serverChallenge));
							NBString_concatBytes(&tmp2, (const char*)&clientChallenge, sizeof(clientChallenge));
							{
								NBString_empty(&lmChallengeResponse);
								{
									const STNBMd5Hash hash = NBHmacMd5_getHashBytes(responseKeyLM.str, responseKeyLM.length, tmp2.str, tmp2.length);
									NBString_concatBytes(&lmChallengeResponse, (const char*)hash.v, sizeof(hash.v));
								}
								NBString_concatBytes(&lmChallengeResponse, (const char*)&clientChallenge, sizeof(clientChallenge));
							}
						}
						//Print values
						/*{
							NBString_empty(&tmp2);
							NBString_concatBytesHex(&tmp2, tmp.str, tmp.length);
							PRINTF_INFO("tmp: '%s'.\n", tmp2.str);
						}*/
						NBString_release(&tmp);
					}
					//Print values
					/*{
						NBString_empty(&tmp2);
						NBString_concatBytesHex(&tmp2, ntChallengeResponse.str, ntChallengeResponse.length);
						PRINTF_INFO("ntChallengeResponse: '%s'.\n", tmp2.str);
						//
						NBString_empty(&tmp2);
						NBString_concatBytesHex(&tmp2, lmChallengeResponse.str, lmChallengeResponse.length);
						PRINTF_INFO("lmChallengeResponse: '%s'.\n", tmp2.str);
					}*/
					//SessionBaseKey
					{
						const STNBMd5Hash hash = NBHmacMd5_getHashBytes(responseKeyNT.str, responseKeyNT.length, nTProofStr.str, nTProofStr.length);
						NBString_setBytes(&sessionBaseKey, (const char*)hash.v, sizeof(hash.v));
						/*{
							const STNBMd5HashHex hashHex = NBHmacMd5_getHashHexBytes(responseKeyNT.str, responseKeyNT.length, nTProofStr.str, nTProofStr.length);
							PRINTF_INFO("sessionBaseKey: '%s'.\n", hashHex.v);
						}*/
					}
					//
					if((chg->negotiateFlags & NTLMSSP_NEGOTIATE_LM_KEY) != 0){
						PRINTF_ERROR("Unsupported NTLMSSP_NEGOTIATE_LM_KEY in challenge.\n");
					} else if((chg->negotiateFlags & NTLMSSP_REQUEST_NON_NT_SESSION_KEY) != 0){
						PRINTF_ERROR("Unsupported NTLMSSP_REQUEST_NON_NT_SESSION_KEY in challenge.\n");
					} else {
						//set KeyExchangeKey to KXKEY(SessionBaseKey, LmChallengeResponse, CHALLENGE_MESSAGE.ServerChallenge)
						NBString_setBytes(&keyExchangeKey, sessionBaseKey.str, sessionBaseKey.length);
						//
						if((chg->negotiateFlags & NTLMSSP_NEGOTIATE_KEY_EXCH) != 0){
							//exportedSessionKey
							{
								//NONCE(16)
								NBString_empty(&exportedSessionKey);
								{
									SI32 i; for(i = 0; i < 16; i++){
										NBString_concatByte(&exportedSessionKey, rand() % 256);
									}
								}
								//Tmp
								/*{
									NBString_set(&exportedSessionKey, "UUUUUUUUUUUUUUUU");
									PRINTF_WARNING("DGB, forcing randomSessionKey to 'UUUUUUUUUUUUUUUU'.\n");
								}*/
								NBASSERT(exportedSessionKey.length == 16)
							}
							//encryptedRandomSessionKey
							{
								NBString_empty(&encryptedRandomSessionKey);
								NBRc4A_concat(keyExchangeKey.str, keyExchangeKey.length, exportedSessionKey.str, exportedSessionKey.length, &encryptedRandomSessionKey);
							}
							//Print values
							/*{
								NBString_empty(&tmp2);
								NBString_concatBytesHex(&tmp2, exportedSessionKey.str, exportedSessionKey.length);
								PRINTF_INFO("randomSessionKey: '%s'.\n", tmp2.str);
								//
								NBString_empty(&tmp2);
								NBString_concatBytesHex(&tmp2, encryptedRandomSessionKey.str, encryptedRandomSessionKey.length);
								PRINTF_INFO("encryptedRandomSessionKey: '%s'.\n", tmp2.str);
							}*/
						} else {
							//Set ExportedSessionKey to KeyExchangeKey
							NBString_setBytes(&exportedSessionKey, keyExchangeKey.str, keyExchangeKey.length);
							NBString_empty(&encryptedRandomSessionKey);
						}
						//
						//Populate result
						if(dstAuthBin != NULL){
							const UI32 headPos = dstAuthBin->length;
							STNTLM_AUTHENTICATE_MESSAGE msg;
							NBMemory_setZeroSt(msg, STNTLM_AUTHENTICATE_MESSAGE);
							msg.signature[0]	= 'N'; msg.signature[1] = 'T'; msg.signature[2] = 'L'; msg.signature[3] = 'M';
							msg.signature[4]	= 'S'; msg.signature[5] = 'S'; msg.signature[6] = 'P'; msg.signature[7] = '\0';
							msg.messageType		= 0x00000003; //must be  0x00000003
							msg.negotiateFlags	= NTLMSSP_NEGOTIATE_UNICODE | NTLMSSP_NEGOTIATE_NTLM | NTLMSSP_NEGOTIATE_TARGET_INFO;
							msg.version			= NBNtln_getDefaultVersionClt();
							//Reserve header
							NBString_concatBytes(dstAuthBin, (const char*)&msg, sizeof(msg));
							//Poppulate
							{
								//domainName
								if(!NBString_strIsEmpty(domain)){
									const char* str = domain;
									msg.domainNameBufferOffset = (dstAuthBin->length - headPos);
									while(*str != '\0'){
										NBString_concatByte(dstAuthBin, *str);
										NBString_concatByte(dstAuthBin, '\0');
										str++;
									}
									msg.domainNameLen = msg.domainNameMaxLen = (dstAuthBin->length - msg.domainNameBufferOffset);
								}
								//username
								if(!NBString_strIsEmpty(user)){
									const char* str = user;
									msg.userNameBufferOffset = (dstAuthBin->length - headPos);
									while(*str != '\0'){
										NBString_concatByte(dstAuthBin, *str);
										NBString_concatByte(dstAuthBin, '\0');
										str++;
									}
									msg.userNameLen = msg.userNameMaxLen = (dstAuthBin->length - msg.userNameBufferOffset);
								}
								//workstation
								if(!NBString_strIsEmpty(workstation)){
									const char* str = workstation;
									msg.workstationNameBufferOffset = (dstAuthBin->length - headPos);
									while(*str != '\0'){
										NBString_concatByte(dstAuthBin, *str);
										NBString_concatByte(dstAuthBin, '\0');
										str++;
									}
									msg.workstationNameLen = msg.workstationNameMaxLen = (dstAuthBin->length - msg.workstationNameBufferOffset);
								}
								//lmChallengeResponse
								if(lmChallengeResponse.length > 0){
									msg.lmChallengeResponseBufferOffset	= (dstAuthBin->length - headPos);
									NBString_concatBytes(dstAuthBin, lmChallengeResponse.str, lmChallengeResponse.length);
									msg.lmChallengeResponseLen = msg.lmChallengeResponseMaxLen = (dstAuthBin->length - msg.lmChallengeResponseBufferOffset);
								}
								//ntChallengeResponse
								if(ntChallengeResponse.length > 0){
									msg.ntChallengeResponseBufferOffset	= (dstAuthBin->length - headPos);
									NBString_concatBytes(dstAuthBin, ntChallengeResponse.str, ntChallengeResponse.length);
									msg.ntChallengeResponseLen = msg.ntChallengeResponseMaxLen = (dstAuthBin->length - msg.ntChallengeResponseBufferOffset);
								}
								//encryptedRandomSessionKey
								if(encryptedRandomSessionKey.length > 0){
									msg.encRandSessionKeyBufferOffset	= (dstAuthBin->length - headPos);
									NBString_concatBytes(dstAuthBin, encryptedRandomSessionKey.str, encryptedRandomSessionKey.length);
									msg.encRandSessionKeyLen = msg.encRandSessionKeyMaxLen = (dstAuthBin->length - msg.encRandSessionKeyBufferOffset);
								}
							}
							//Update head
							{
								NBMemory_copy(&dstAuthBin->str[headPos], &msg, sizeof(msg));
							}
							//Populate 'mic'
							{
								NBString_empty(&tmp2);
								NBString_concatBytes(&tmp2, (const char*)cltNegBin, cltNegBinSz);
								NBString_concatBytes(&tmp2, (const char*)srvChallengeBin, srvChallengeBinSz);
								NBString_concatBytes(&tmp2, (const char*)dstAuthBin->str, dstAuthBin->length);
								{
									const STNBMd5Hash h = NBHmacMd5_getHashBytes(exportedSessionKey.str, exportedSessionKey.length, tmp2.str, tmp2.length);
									NBASSERT(sizeof(msg.mic) == sizeof(h.v))
									NBMemory_copy(&msg.mic, h.v, sizeof(h.v));
								}
								//Update head with MIC
								{
									NBMemory_copy(&dstAuthBin->str[headPos], &msg, sizeof(msg));
								}
							}
						}
						r = TRUE;
					}
					NBString_release(&tmp2);
				}
				NBString_release(&encryptedRandomSessionKey);
				NBString_release(&exportedSessionKey);
				NBString_release(&keyExchangeKey);
				NBString_release(&sessionBaseKey);
				NBString_release(&ntChallengeResponse);
				NBString_release(&lmChallengeResponse);
				NBString_release(&nTProofStr);
				NBString_release(&responseKeyLM);
				NBString_release(&responseKeyNT);
			}
			NBString_release(&dnsCompName);
			NBString_release(&dnsCompName16);
			NBString_release(&dnsDomainName);
			NBString_release(&dnsDomainName16);
			NBString_release(&compName);
			NBString_release(&compName16);
			NBString_release(&domainName);
			NBString_release(&domainName16);
		}
	}
	return r;
}

STNTLM_VERSION NBNtln_getDefaultVersionClt(void){
	STNTLM_VERSION r;
	NBMemory_setZeroSt(r, STNTLM_VERSION);
	//Setting values from specs-doc samples
	r.productMajorVersion	= 0x05;
	r.productMinorVersion	= 0x01;
	r.productBuild			= 2600;
	r.nTLMRevisionCurrent	= 0x0f;
	//
	return r;
}

void NBNtln_printFlags(const UI32 negFlags){
	const SI32 count = (sizeof(___NTLM_NegFlagsNames) / sizeof(___NTLM_NegFlagsNames[0]));
	SI32 i; for(i = 0; i < count; i++){
		if((negFlags & (0x1 << i)) != 0){
			PRINTF_INFO("NegotiateFlag ON: '%s'.\n", ___NTLM_NegFlagsNames[i]);
		}
	}
}
